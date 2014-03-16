/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 

 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 

 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/*
 * Interrupt control functions for Broadcom 963xx MIPS boards
 */

#include <asm/atomic.h>

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/linkage.h>

#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/signal.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <linux/bcm_assert.h>


#if defined(CONFIG_SMP)
    #define AFFINITY_OF(d) (*(d)->affinity)
#else
    #define AFFINITY_OF(d) ((void)(d), CPU_MASK_CPU0)
#endif

#if IRQ_BITS == 64
    #define IRQ_TYPE uint64
#else
    #define IRQ_TYPE uint32
#endif

volatile IrqControl_t * brcm_irq_ctrl[NR_CPUS];
spinlock_t brcm_irqlock;

#if defined(CONFIG_SMP)
extern DEFINE_PER_CPU(unsigned int, ipi_pending);
#endif

static void irq_dispatch_int(void)
{
    int cpu = smp_processor_id();
    IRQ_TYPE pendingIrqs;
    static IRQ_TYPE irqBit[NR_CPUS];

    static uint32 isrNumber[NR_CPUS] = {[0 ... NR_CPUS-1] = (sizeof(IRQ_TYPE) * 8) - 1};

    spin_lock(&brcm_irqlock);
    pendingIrqs = brcm_irq_ctrl[cpu]->IrqStatus & brcm_irq_ctrl[cpu]->IrqMask;
    spin_unlock(&brcm_irqlock);

    if (!pendingIrqs) {
        return;
    }

    while (1) {
        irqBit[cpu] <<= 1;
        isrNumber[cpu]++;
        if (isrNumber[cpu] == (sizeof(IRQ_TYPE) * 8)) {
            isrNumber[cpu] = 0;
            irqBit[cpu] = 0x1;
        }
        if (pendingIrqs & irqBit[cpu]) {
            unsigned int irq = isrNumber[cpu] + INTERNAL_ISR_TABLE_OFFSET;
            if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) {
                spin_lock(&brcm_irqlock);
                PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
                spin_unlock(&brcm_irqlock);
            }
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
            else if (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_5) {
                spin_lock(&brcm_irqlock);
                PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_CLEAR_SHFT));      // Clear
                spin_unlock(&brcm_irqlock);
            }
#endif
            do_IRQ(irq);
            break;
        }
    }
}

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE
extern void BcmPwrMngtResumeFullSpeed (void);
#endif


asmlinkage void plat_irq_dispatch(void)
{
    u32 cause;

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE
    BcmPwrMngtResumeFullSpeed();
#endif

    while((cause = (read_c0_cause() & read_c0_status() & CAUSEF_IP))) {
        if (cause & CAUSEF_IP7)
            do_IRQ(MIPS_TIMER_INT);
        else if (cause & CAUSEF_IP0)
            do_IRQ(INTERRUPT_ID_SOFTWARE_0);
        else if (cause & CAUSEF_IP1)
            do_IRQ(INTERRUPT_ID_SOFTWARE_1);
#if defined (CONFIG_SMP)
        else if (cause & (CAUSEF_IP2 | CAUSEF_IP3))
#else 
        else if (cause & CAUSEF_IP2)
#endif
            irq_dispatch_int();
    }
}


// bill
void disable_brcm_irqsave(unsigned int irq, unsigned long stateSaveArray[])
{
    int cpu;
    struct irq_desc *desc = irq_desc + irq;
    unsigned long flags;

    // Test for valid interrupt.
    if ((irq >= INTERNAL_ISR_TABLE_OFFSET ) && (irq <= INTERRUPT_ID_LAST))
    {
        // Disable this processor's interrupts and acquire spinlock.
        spin_lock_irqsave(&brcm_irqlock, flags);

        // Loop thru each processor.
        for_each_cpu_mask(cpu, AFFINITY_OF(desc))
        {
            // Save original interrupt's enable state.
            stateSaveArray[cpu] = brcm_irq_ctrl[cpu]->IrqMask & (((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));

            // Clear each cpu's selected interrupt enable.
            brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));
        }

        // Release spinlock and enable this processor's interrupts.
        spin_unlock_irqrestore(&brcm_irqlock, flags);
    }
}


// bill
void restore_brcm_irqsave(unsigned int irq, unsigned long stateSaveArray[])
{
    int cpu;
    struct irq_desc *desc = irq_desc + irq;
    unsigned long flags;

    // Disable this processor's interrupts and acquire spinlock.
    spin_lock_irqsave(&brcm_irqlock, flags);

    // Loop thru each processor.
    for_each_cpu_mask(cpu, AFFINITY_OF(desc))
    {
        // Restore cpu's original interrupt enable (off or on).
        brcm_irq_ctrl[cpu]->IrqMask |= stateSaveArray[cpu];
    }

    // Release spinlock and enable this processor's interrupts.
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}




void enable_brcm_irq(unsigned int irq)
{
    int cpu;
    struct irq_desc *desc = irq_desc + irq;
    unsigned long flags;

    spin_lock_irqsave(&brcm_irqlock, flags);

    if( irq >= INTERNAL_ISR_TABLE_OFFSET ) {
        for_each_cpu_mask(cpu, AFFINITY_OF(desc)) {
            brcm_irq_ctrl[cpu]->IrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_ISR_TABLE_OFFSET));
        }
    }
    else if ((irq == INTERRUPT_ID_SOFTWARE_0) || (irq == INTERRUPT_ID_SOFTWARE_1)) {
        set_c0_status(0x1 << (STATUSB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    }

    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_3) {
        PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_INSENS_SHFT));    // Edge insesnsitive
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_LEVEL_SHFT));      // Level triggered
        PERF->ExtIrqCfg &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_SENSE_SHFT));     // Low level
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_CLEAR_SHFT));      // Clear
        PERF->ExtIrqCfg |= (1 << (irq - INTERRUPT_ID_EXTERNAL_0 + EI_MASK_SHFT));       // Unmask
    }
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    else if (irq >= INTERRUPT_ID_EXTERNAL_4 && irq <= INTERRUPT_ID_EXTERNAL_5) {
        PERF->ExtIrqCfg1 &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_INSENS_SHFT));    // Edge insesnsitive
        PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_LEVEL_SHFT));      // Level triggered
        PERF->ExtIrqCfg1 &= ~(1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_SENSE_SHFT));     // Low level
        PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_CLEAR_SHFT));      // Clear
        PERF->ExtIrqCfg1 |= (1 << (irq - INTERRUPT_ID_EXTERNAL_4 + EI_MASK_SHFT));       // Unmask
    }
#endif

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


void __disable_ack_brcm_irq(unsigned int irq)
{
    int cpu;
    struct irq_desc *desc = irq_desc + irq;

    if( irq >= INTERNAL_ISR_TABLE_OFFSET ) {
        for_each_cpu_mask(cpu, AFFINITY_OF(desc)) {
            brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));
        }
    }
}


void disable_brcm_irq(unsigned int irq)
{
    unsigned long flags;

    spin_lock_irqsave(&brcm_irqlock, flags);
    __disable_ack_brcm_irq(irq);
    if ((irq == INTERRUPT_ID_SOFTWARE_0) || (irq == INTERRUPT_ID_SOFTWARE_1)) {
        clear_c0_status(0x1 << (STATUSB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));
    }
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


void ack_brcm_irq(unsigned int irq)
{
    unsigned long flags;

    spin_lock_irqsave(&brcm_irqlock, flags);
    __disable_ack_brcm_irq(irq);

#if defined(CONFIG_SMP)
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        int this_cpu = smp_processor_id();
        int other_cpu = !this_cpu;
        per_cpu(ipi_pending, this_cpu) = 0;
        mb();
        clear_c0_cause(1<<CAUSEB_IP0);
        if (per_cpu(ipi_pending, other_cpu)) {
            set_c0_cause(1<<CAUSEB_IP0);
        }
    }
#else
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        clear_c0_cause(1<<CAUSEB_IP0);
    }
#endif

    if (irq == INTERRUPT_ID_SOFTWARE_1) {
        clear_c0_cause(1<<CAUSEB_IP1);
    }

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


void mask_ack_brcm_irq(unsigned int irq)
{
    unsigned long flags;

    spin_lock_irqsave(&brcm_irqlock, flags);
    __disable_ack_brcm_irq(irq);

#if defined(CONFIG_SMP)
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        int this_cpu = smp_processor_id();
        int other_cpu = !this_cpu;
        per_cpu(ipi_pending, this_cpu) = 0;
        mb();
        clear_c0_cause(1<<CAUSEB_IP0);
        if (per_cpu(ipi_pending, other_cpu)) {
            set_c0_cause(1<<CAUSEB_IP0);
        }
        clear_c0_status(1<<STATUSB_IP0);
    }
#else
    if (irq == INTERRUPT_ID_SOFTWARE_0) {
        clear_c0_status(1<<STATUSB_IP0);
        clear_c0_cause(1<<CAUSEB_IP0);
    }
#endif

    if (irq == INTERRUPT_ID_SOFTWARE_1) {
        clear_c0_status(1<<STATUSB_IP1);
        clear_c0_cause(1<<CAUSEB_IP1);
    }

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


void unmask_brcm_irq_noop(unsigned int irq)
{
}

void set_brcm_affinity(unsigned int irq, const struct cpumask *mask)
{
    int cpu;
    struct irq_desc *desc = irq_desc + irq;
    unsigned long flags;

    spin_lock_irqsave(&brcm_irqlock, flags);

    if( irq >= INTERNAL_ISR_TABLE_OFFSET ) {
        for_each_online_cpu(cpu) {
            if (cpu_isset(cpu, *mask) && !(desc->status & IRQ_DISABLED)) {
                brcm_irq_ctrl[cpu]->IrqMask |= (((IRQ_TYPE)1)  << (irq - INTERNAL_ISR_TABLE_OFFSET));
            }
            else {
                brcm_irq_ctrl[cpu]->IrqMask &= ~(((IRQ_TYPE)1) << (irq - INTERNAL_ISR_TABLE_OFFSET));
            }
        }
    }

    spin_unlock_irqrestore(&brcm_irqlock, flags);
}


static struct irq_chip brcm_irq_chip = {
    .name = "BCM63xx",
    .enable = enable_brcm_irq,
    .disable = disable_brcm_irq,
    .ack = ack_brcm_irq,
    .mask = disable_brcm_irq,
    .mask_ack = mask_ack_brcm_irq,
    .unmask = enable_brcm_irq,
    .set_affinity = set_brcm_affinity
};

static struct irq_chip brcm_irq_chip_no_unmask = {
    .name = "BCM63xx_no_unmask",
    .enable = enable_brcm_irq,
    .disable = disable_brcm_irq,
    .ack = ack_brcm_irq,
    .mask = disable_brcm_irq,
    .mask_ack = mask_ack_brcm_irq,
    .unmask = unmask_brcm_irq_noop,
    .set_affinity = set_brcm_affinity
};


void __init arch_init_irq(void)
{
    int i;

    spin_lock_init(&brcm_irqlock);

    for (i = 0; i < NR_CPUS; i++) {
        brcm_irq_ctrl[i] = &PERF->IrqControl[i];
    }

    for (i = 0; i < NR_IRQS; i++) {
        set_irq_chip_and_handler(i, &brcm_irq_chip, handle_level_irq); 
    }

    clear_c0_status(ST0_BEV);
#if defined(CONFIG_SMP)
    // make interrupt mask same as TP1, miwang 6/14/10
    change_c0_status(ST0_IM, IE_IRQ0|IE_IRQ1);
#else
    change_c0_status(ST0_IM, IE_IRQ0);
#endif


#ifdef CONFIG_REMOTE_DEBUG
    rs_kgdb_hook(0);
#endif
}


#define INTR_NAME_MAX_LENGTH 16

// This is a wrapper to standand Linux request_irq
// Differences are:
//    - The irq won't be renabled after ISR is done and needs to be explicity re-enabled, which is good for NAPI drivers.
//      The change is implemented by filling in an no-op unmask function in brcm_irq_chip_no_unmask and set it as the irq_chip
//    - IRQ flags and interrupt names are automatically set
// Either request_irq or BcmHalMapInterrupt can be used. Just make sure re-enabling IRQ is handled correctly.

unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, unsigned int param, unsigned int irq)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptEx(pfunc, param, irq, devname,
                                INTR_REARM_NO, INTR_AFFINITY_DEFAULT);
}


// This is a wrapper to standand Linux request_irq for the VOIP driver
// Differences are:
// The irq is not automatically enabled when the ISR is registered.
// The irq is automatically re-enabled when the ISR is done.
// Interrupts are re-enabled when the ISR is invoked.
unsigned int BcmHalMapInterruptVoip(FN_HANDLER pfunc, unsigned int param, unsigned int irq)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptEx(pfunc, param, irq, devname,
                                INTR_REARM_YES, INTR_AFFINITY_DEFAULT);
}


/** Broadcom wrapper to linux request_irq.  This version does more stuff.
 *
 * @param pfunc (IN) interrupt handler function
 * @param param (IN) context/cookie that is passed to interrupt handler
 * @param irq   (IN) interrupt number
 * @param interruptName (IN) descriptive name for the interrupt.  15 chars
 *                           or less.  This function will make a copy of
 *                           the name.
 * @param INTR_REARM_MODE    (IN) See bcm_intr.h
 * @param INTR_AFFINITY_MODE (IN) See bcm_intr.h
 *
 * @return 0 on success.
 */
unsigned int BcmHalMapInterruptEx(FN_HANDLER pfunc,
                                  unsigned int param,
                                  unsigned int irq,
                                  const char *interruptName,
                                  INTR_REARM_MODE_ENUM rearmMode,
                                  INTR_AFFINITY_MODE_ENUM affinMode)
{
    char *devname;
    unsigned long irqflags;
    struct irq_chip *chip;
    unsigned int retval;

    BCM_ASSERT_R(interruptName != NULL, -1);
    BCM_ASSERT_R(strlen(interruptName) < INTR_NAME_MAX_LENGTH, -1);

    if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_KERNEL)) == NULL)
    {
        return -1;
    }
    sprintf( devname, "%s", interruptName );

    /* If this is for the timer interrupt, do not invoke the following code
       because doing so kills the timer interrupts that may already be running */
    if (irq != INTERRUPT_ID_TIMER) {
        chip = (rearmMode == INTR_REARM_NO) ? &brcm_irq_chip_no_unmask :
                                              &brcm_irq_chip;
        set_irq_chip_and_handler(irq, chip, handle_level_irq);
    }

    if (rearmMode == INTR_REARM_YES)
    {
        unsigned long flags;
        struct irq_desc *desc = irq_desc + irq;

        spin_lock_irqsave(&desc->lock, flags);
        desc->status |= IRQ_NOAUTOEN;
        desc->status &= ~IRQ_DISABLED;
        spin_unlock_irqrestore(&desc->lock, flags);
    }


    irqflags = IRQF_SAMPLE_RANDOM;
    irqflags |= (rearmMode == INTR_REARM_NO) ? IRQF_DISABLED : 0;
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    irqflags |= (irq == INTERRUPT_ID_MPI) ? IRQF_SHARED : 0;
#endif
    /* There are 3 timers with individual control, so the interrupt can be shared */
    irqflags |= (irq == INTERRUPT_ID_TIMER) ? IRQF_SHARED : 0;

    retval = request_irq(irq, (void*)pfunc, irqflags, devname, (void *) param);
    if (retval != 0)
    {
        printk(KERN_WARNING "request_irq failed for irq=%d (%s) retval=%d\n",
               irq, devname, retval);
        kfree(devname);
        return retval;
    }

    // now deal with interrupt affinity requests
    if (affinMode != INTR_AFFINITY_DEFAULT)
    {
        struct cpumask mask;

        cpus_clear(mask);

        if (affinMode == INTR_AFFINITY_TP1_ONLY ||
            affinMode == INTR_AFFINITY_TP1_IF_POSSIBLE)
        {
            if (cpu_online(1))
            {
                cpu_set(1, mask);
                irq_set_affinity(irq, &mask);
            }
            else
            {
                // TP1 is not on-line but caller insisted on it
                if (affinMode == INTR_AFFINITY_TP1_ONLY)
                {
                    printk(KERN_WARNING
                           "cannot assign intr %d to TP1, not online\n", irq);
                    retval = request_irq(irq, NULL, 0, NULL, NULL);
                    kfree(devname);
                    retval = -1;
                }
            }
        }
        else
        {
            // INTR_AFFINITY_BOTH_IF_POSSIBLE
            cpu_set(0, mask);
            if (cpu_online(1))
            {
                cpu_set(1, mask);
                irq_set_affinity(irq, &mask);
            }
        }
    }

    return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptEx);


//***************************************************************************
//  void  BcmHalGenerateSoftInterrupt
//
//   Triggers a software interrupt.
//
//***************************************************************************
void BcmHalGenerateSoftInterrupt( unsigned int irq )
{
    unsigned long flags;

    local_irq_save(flags);

    set_c0_cause(0x1 << (CAUSEB_IP0 + irq - INTERRUPT_ID_SOFTWARE_0));

    local_irq_restore(flags);
}


EXPORT_SYMBOL(enable_brcm_irq);
EXPORT_SYMBOL(disable_brcm_irq);
EXPORT_SYMBOL(BcmHalMapInterrupt);
EXPORT_SYMBOL(BcmHalMapInterruptVoip);
EXPORT_SYMBOL(BcmHalGenerateSoftInterrupt);

// bill
EXPORT_SYMBOL(disable_brcm_irqsave);
EXPORT_SYMBOL(restore_brcm_irqsave);

