/*
<:copyright-BRCM:2002:GPL/GPL:standard

   Copyright (c) 2002 Broadcom Corporation
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/*
 * Generic setup routines for Broadcom 963xx MIPS boards
 */

//#include <linux/config.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/console.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/bootmem.h>

#include <asm/addrspace.h>
#include <asm/bcache.h>
#include <asm/irq.h>
#include <asm/time.h>
#include <asm/reboot.h>
//#include <asm/gdb-stub.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>

extern unsigned long getMemorySize(void);
extern irqreturn_t brcm_timer_interrupt(int irq, void *dev_id);

#include <bcm_map_part.h>
#include <bcm_cpu.h>
#include <bcm_intr.h>
#include <board.h>
#include <boardparms.h>

#if defined(CONFIG_PCI)
#include <linux/pci.h>
#include <bcmpci.h>
#endif

#if 1
#include "softdsl/AdslCoreDefs.h"
#endif

#if defined(CONFIG_BCM_ENDPOINT_MODULE)
#include <dsp_mod_size.h>
#endif

#if defined(CONFIG_BCM96816)
#include "bcmSpiRes.h"
#endif

#if 1

/***************************************************************************
 * C++ New and delete operator functions
 ***************************************************************************/

/* void *operator new(unsigned int sz) */
void *_Znwj(unsigned int sz)
{
    return( kmalloc(sz, GFP_KERNEL) );
}

/* void *operator new[](unsigned int sz)*/
void *_Znaj(unsigned int sz)
{
    return( kmalloc(sz, GFP_KERNEL) );
}

/* placement new operator */
/* void *operator new (unsigned int size, void *ptr) */
void *ZnwjPv(unsigned int size, void *ptr)
{
    return ptr;
}

/* void operator delete(void *m) */
void _ZdlPv(void *m)
{
    kfree(m);
}

/* void operator delete[](void *m) */
void _ZdaPv(void *m)
{
    kfree(m);
}

EXPORT_SYMBOL(_Znwj);
EXPORT_SYMBOL(_Znaj);
EXPORT_SYMBOL(ZnwjPv);
EXPORT_SYMBOL(_ZdlPv);
EXPORT_SYMBOL(_ZdaPv);

#endif



void __init plat_mem_setup(void)
{
#if defined(CONFIG_BCM96816)
    add_memory_region(0, (getMemorySize()), BOOT_MEM_RAM);
#else
    printk("DSL SDRAM reserved: 0x%x\n", ADSL_SDRAM_IMAGE_SIZE);
    add_memory_region(0, (getMemorySize() - ADSL_SDRAM_IMAGE_SIZE), BOOT_MEM_RAM);
#endif


    {
        unsigned long *cr;
	cr = (void *)(MIPS_BASE + MIPS_RAC_CR0);
    	*cr = *cr | RAC_D | RAC_PF_D;

	cr = (void *)(MIPS_BASE + MIPS_RAC_CR1);
    	*cr = *cr | RAC_D | RAC_PF_D;
    }
}


extern UINT32 __init calculateCpuSpeed(void);
#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS
extern void BcmPwrMngtInitC0Speed (void);
#endif


void __init plat_time_init(void)
{
    /* JU: TBD: there was some special SMP handling added here in original kernel */
    mips_hpt_frequency = calculateCpuSpeed() / 2;
#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS
    BcmPwrMngtInitC0Speed();
#else
    // Enable cp0 counter/compare interrupt when
    // not using workaround for clock divide
    write_c0_status(IE_IRQ5 | read_c0_status());
#endif
}


static void brcm_machine_restart(char *command)
{
    kerSysMipsSoftReset();
}

extern void stop_other_cpu(void);  // in arch/mips/kernel/smp.c

static void brcm_machine_halt(void)
{
    /*
     * we don't support power off yet.  This halt will cause both CPU's to
     * spin in a while(1) loop with interrupts disabled.  (Used for gathering
     * wlan debug dump via JTAG)
     */
#if defined(CONFIG_SMP)
    stop_other_cpu();
#endif
    printk("System halted\n");
    local_irq_disable();
    while (1);
}

#if defined(CONFIG_PCI) && (defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816))
static void mpi_SetLocalPciConfigReg(uint32 reg, uint32 value)
{
    /* write index then value */
    MPI->pcicfgcntrl = PCI_CFG_REG_WRITE_EN + reg;;
    MPI->pcicfgdata = value;
}

static uint32 mpi_GetLocalPciConfigReg(uint32 reg)
{
    /* write index then get value */
    MPI->pcicfgcntrl = PCI_CFG_REG_WRITE_EN + reg;;
    return MPI->pcicfgdata;
}

#if !defined(CONFIG_BCM96816)
/*
 * mpi_ResetPcCard: Set/Reset the PcCard
 */
static void mpi_ResetPcCard(int cardtype, BOOL bReset)
{
    if (cardtype == MPI_CARDTYPE_NONE) {
        return;
    }

    if (cardtype == MPI_CARDTYPE_CARDBUS) {
        bReset = ! bReset;
    }

    if (bReset) {
        MPI->pcmcia_cntl1 = (MPI->pcmcia_cntl1 & ~PCCARD_CARD_RESET);
    } else {
        MPI->pcmcia_cntl1 = (MPI->pcmcia_cntl1 | PCCARD_CARD_RESET);
    }
}

/*
 * mpi_ConfigCs: Configure an MPI/EBI chip select
 */
static void mpi_ConfigCs(uint32 cs, uint32 base, uint32 size, uint32 flags)
{
    MPI->cs[cs].base = ((base & 0x1FFFFFFF) | size);
    MPI->cs[cs].config = flags;
}

/*
 * mpi_InitPcmciaSpace
 */
static void mpi_InitPcmciaSpace(void)
{
    // ChipSelect 4 controls PCMCIA Memory accesses
    mpi_ConfigCs(PCMCIA_COMMON_BASE, pcmciaMem, EBI_SIZE_1M, (EBI_WORD_WIDE|EBI_ENABLE));
    // ChipSelect 5 controls PCMCIA Attribute accesses
    mpi_ConfigCs(PCMCIA_ATTRIBUTE_BASE, pcmciaAttr, EBI_SIZE_1M, (EBI_WORD_WIDE|EBI_ENABLE));
    // ChipSelect 6 controls PCMCIA I/O accesses
    mpi_ConfigCs(PCMCIA_IO_BASE, pcmciaIo, EBI_SIZE_64K, (EBI_WORD_WIDE|EBI_ENABLE));

    MPI->pcmcia_cntl2 = ((PCMCIA_ATTR_ACTIVE << RW_ACTIVE_CNT_BIT) |
                         (PCMCIA_ATTR_INACTIVE << INACTIVE_CNT_BIT) |
                         (PCMCIA_ATTR_CE_SETUP << CE_SETUP_CNT_BIT) |
                         (PCMCIA_ATTR_CE_HOLD << CE_HOLD_CNT_BIT));

    MPI->pcmcia_cntl2 |= (PCMCIA_HALFWORD_EN | PCMCIA_BYTESWAP_DIS);
}

/*
 * cardtype_vcc_detect: PC Card's card detect and voltage sense connection
 *
 *   CD1#/      CD2#/     VS1#/     VS2#/    Card       Initial Vcc
 *  CCD1#      CCD2#     CVS1      CVS2      Type
 *
 *   GND        GND       open      open     16-bit     5 vdc
 *
 *   GND        GND       GND       open     16-bit     3.3 vdc
 *
 *   GND        GND       open      GND      16-bit     x.x vdc
 *
 *   GND        GND       GND       GND      16-bit     3.3 & x.x vdc
 *
 *====================================================================
 *
 *   CVS1       GND       CCD1#     open     CardBus    3.3 vdc
 *
 *   GND        CVS2      open      CCD2#    CardBus    x.x vdc
 *
 *   GND        CVS1      CCD2#     open     CardBus    y.y vdc
 *
 *   GND        CVS2      GND       CCD2#    CardBus    3.3 & x.x vdc
 *
 *   CVS2       GND       open      CCD1#    CardBus    x.x & y.y vdc
 *
 *   GND        CVS1      CCD2#     open     CardBus    3.3, x.x & y.y vdc
 *
 */
static int cardtype_vcc_detect(void)
{
    uint32 data32;
    int cardtype;

    cardtype = MPI_CARDTYPE_NONE;
    MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE); // Turn on the output enables and drive
                                        // the CVS pins to 0.
    data32 = MPI->pcmcia_cntl1;
    switch (data32 & (CD2_IN|CD1_IN))  // Test CD1# and CD2#, see if card is plugged in.
    {
    case (CD2_IN|CD1_IN):  // No Card is in the slot.
        printk("MPI: No Card is in the PCMCIA slot\n");
        break;

    case CD2_IN:  // Partial insertion, No CD2#.
        printk("MPI: Card in the PCMCIA slot partial insertion, no CD2 signal\n");
        break;

    case CD1_IN:  // Partial insertion, No CD1#.
        printk("MPI: Card in the PCMCIA slot partial insertion, no CD1 signal\n");
        break;

    case 0x00000000:
        MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS2_OEN|VS1_OEN);
                                        // Turn off the CVS output enables and
                                        // float the CVS pins.
        mdelay(1);
        data32 = MPI->pcmcia_cntl1;
        // Read the Register.
        switch (data32 & (VS2_IN|VS1_IN))  // See what is on the CVS pins.
        {
        case 0x00000000: // CVS1 and CVS2 are tied to ground, only 1 option.
            printk("MPI: Detected 3.3 & x.x 16-bit PCMCIA card\n");
            cardtype = MPI_CARDTYPE_PCMCIA;
            break;

        case VS1_IN: // CVS1 is open or tied to CCD1/CCD2 and CVS2 is tied to ground.
                         // 2 valid voltage options.
        switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
        {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n");
                break;

            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS2_OEN); // Drive CVS1 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD2_IN) { // CCD2 is tied to CVS2, not valid.
                    printk("MPI: Unknown card plugged into slot\n");
                } else {                   // CCD2 is tied to CVS1.
                    printk("MPI: Detected 3.3, x.x and y.y Cardbus card\n");
                    cardtype = MPI_CARDTYPE_CARDBUS;
                }
                break;

            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n");
                break;

            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                printk("MPI: Detected x.x vdc 16-bit PCMCIA card\n");
                cardtype = MPI_CARDTYPE_PCMCIA;
                break;
            }
            break;

        case VS2_IN: // CVS2 is open or tied to CCD1/CCD2 and CVS1 is tied to ground.
                         // 2 valid voltage options.
            switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
            {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n");
                break;

            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD2_IN) { // CCD2 is tied to CVS1, not valid.
                    printk("MPI: Unknown card plugged into slot\n");
                } else {// CCD2 is tied to CVS2.
                    printk("MPI: Detected 3.3 and x.x Cardbus card\n");
                    cardtype = MPI_CARDTYPE_CARDBUS;
                }
                break;

            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n");
                break;

            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                cardtype = MPI_CARDTYPE_PCMCIA;
                printk("MPI: Detected 3.3 vdc 16-bit PCMCIA card\n");
                break;
            }
            break;

        case (VS2_IN|VS1_IN):  // CVS1 and CVS2 are open or tied to CCD1/CCD2.
                          // 5 valid voltage options.

            switch (data32 & (CD2_IN|CD1_IN))  // Test the values of CCD1 and CCD2.
            {
            case (CD2_IN|CD1_IN):  // CCD1 and CCD2 are tied to 1 of the CVS pins.
                              // This is not a valid combination.
                printk("MPI: Unknown card plugged into slot\n");
                break;

            case CD2_IN:  // CCD2 is tied to either CVS1 or CVS2.
                              // CCD1 is tied to ground.
                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD2_IN) {  // CCD2 is tied to CVS1.
                    printk("MPI: Detected y.y vdc Cardbus card\n");
                } else {                    // CCD2 is tied to CVS2.
                    printk("MPI: Detected x.x vdc Cardbus card\n");
                }
                cardtype = MPI_CARDTYPE_CARDBUS;
                break;

            case CD1_IN: // CCD1 is tied to either CVS1 or CVS2.
                             // CCD2 is tied to ground.

                MPI->pcmcia_cntl1 = (CARDBUS_ENABLE|PCMCIA_GPIO_ENABLE|VS1_OEN);// Drive CVS2 to a 0.
                mdelay(1);
                data32 = MPI->pcmcia_cntl1;
                if (data32 & CD1_IN) {// CCD1 is tied to CVS1.
                    printk("MPI: Detected 3.3 vdc Cardbus card\n");
                } else {                    // CCD1 is tied to CVS2.
                    printk("MPI: Detected x.x and y.y Cardbus card\n");
                }
                cardtype = MPI_CARDTYPE_CARDBUS;
                break;

            case 0x00000000:  // CCD1 and CCD2 are tied to ground.
                cardtype = MPI_CARDTYPE_PCMCIA;
                printk("MPI: Detected 5 vdc 16-bit PCMCIA card\n");
                break;
            }
            break;

        default:
            printk("MPI: Unknown card plugged into slot\n");
            break;

        }
    }
    return cardtype;
}

/*
 * mpi_DetectPcCard: Detect the plugged in PC-Card
 * Return: < 0 => Unknown card detected
 *         0 => No card detected
 *         1 => 16-bit card detected
 *         2 => 32-bit CardBus card detected
 */
static int mpi_DetectPcCard(void)
{
    int cardtype;

    cardtype = cardtype_vcc_detect();
    switch(cardtype) {
        case MPI_CARDTYPE_PCMCIA:
            MPI->pcmcia_cntl1 &= ~(CARDBUS_ENABLE|PCMCIA_ENABLE|PCMCIA_GPIO_ENABLE); // disable enable bits
            MPI->pcmcia_cntl1 |= (PCMCIA_ENABLE | PCMCIA_GPIO_ENABLE);
            mpi_InitPcmciaSpace();
            mpi_ResetPcCard(cardtype, FALSE);
            // Hold card in reset for 10ms
            mdelay(10);
            mpi_ResetPcCard(cardtype, TRUE);
            // Let card come out of reset
            mdelay(100);
            break;
        case MPI_CARDTYPE_CARDBUS:
            // 8 => CardBus Enable
            // 1 => PCI Slot Number
            // C => Float VS1 & VS2
            MPI->pcmcia_cntl1 = (MPI->pcmcia_cntl1 & 0xFFFF0000) |
                                CARDBUS_ENABLE |
                                (CARDBUS_SLOT << 8)|
                                VS2_OEN |
                                VS1_OEN | PCMCIA_GPIO_ENABLE;
            /* access to this memory window will be to/from CardBus */
            MPI->l2pmremap1 |= CARDBUS_MEM;

            // Need to reset the Cardbus Card. There's no CardManager to do this,
            // and we need to be ready for PCI configuration.
            mpi_ResetPcCard(cardtype, FALSE);
            // Hold card in reset for 10ms
            mdelay(10);
            mpi_ResetPcCard(cardtype, TRUE);
            // Let card come out of reset
            mdelay(100);
            break;
        default:
            break;
    }
    return cardtype;
}
#endif

int mpi_init(void)
{
    unsigned long data;
    unsigned long sdramsize;
    unsigned int modesel;

    sdramsize = getMemorySize();

    // UBUS to PCI address range
    // Memory Window 1. Used for devices in slot 0. Potentially can be CardBus
    MPI->l2pmrange1 = ~(BCM_PCI_MEM_SIZE-1);
    // UBUS to PCI Memory base address. This is akin to the ChipSelect base
    // register.
    MPI->l2pmbase1 = BCM_CB_MEM_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI Remap Address. Replaces the masked address bits in the
    // range register with this setting.
    // Also, enable direct I/O and direct Memory accesses
    MPI->l2pmremap1 = (BCM_PCI_MEM_BASE | MEM_WINDOW_EN);

    // Memory Window 2. Used for devices in other slots
    MPI->l2pmrange2 = ~(BCM_PCI_MEM_SIZE-1);
    // UBUS to PCI Memory base address.
    MPI->l2pmbase2 = BCM_PCI_MEM_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI Remap Address
    MPI->l2pmremap2 = (BCM_PCI_MEM_BASE | MEM_WINDOW_EN);

    // Setup PCI I/O Window range. Give 64K to PCI I/O
    MPI->l2piorange = ~(BCM_PCI_IO_SIZE-1);
    // UBUS to PCI I/O base address
    MPI->l2piobase = BCM_PCI_IO_BASE & BCM_PCI_ADDR_MASK;
    // UBUS to PCI I/O Window remap
    MPI->l2pioremap = (BCM_PCI_IO_BASE | MEM_WINDOW_EN);

    // enable PCI related GPIO pins and data swap between system and PCI bus
    MPI->locbuscntrl = (EN_PCI_GPIO | DIR_U2P_NOSWAP);

    /* Enable BusMaster and Memory access mode */
    data = mpi_GetLocalPciConfigReg(PCI_COMMAND);
    data |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
    mpi_SetLocalPciConfigReg(PCI_COMMAND, data);

    /* Configure two 16 MByte PCI to System memory regions. */
    /* These memory regions are used when PCI device is a bus master */
    /* Accesses to the SDRAM from PCI bus will be "byte swapped" for this region */
    mpi_SetLocalPciConfigReg(PCI_BASE_ADDRESS_3, BCM_HOST_MEM_SPACE1);

    MPI->sp0remap = MEM_WINDOW_EN;

    /* Accesses to the SDRAM from PCI bus will be "byte swapped" for this region */
    mpi_SetLocalPciConfigReg(PCI_BASE_ADDRESS_4, BCM_HOST_MEM_SPACE2);

    MPI->sp1remap = MEM_WINDOW_EN;

    modesel = MPI->pcimodesel;
    modesel &= ~PCI_INT_BUS_RD_PREFETCH;
    modesel |= 0x100;
    MPI->pcimodesel = modesel;

    MPI->sp0range = ~(sdramsize-1);
    MPI->sp1range = ~(sdramsize-1);
    /*
     * Change PCI Cfg Reg. offset 0x40 to PCI memory read retry count infinity
     * by set 0 in bit 8~15.  This resolve read Bcm4306 srom return 0xffff in
     * first read.
     */
    data = mpi_GetLocalPciConfigReg(BRCM_PCI_CONFIG_TIMER);
    data &= ~BRCM_PCI_CONFIG_TIMER_RETRY_MASK;
    data |= 0x00000080;
    mpi_SetLocalPciConfigReg(BRCM_PCI_CONFIG_TIMER, data);

    /* enable pci interrupt */
    MPI->locintstat |= (EXT_PCI_INT << 16);

    return 0;
}
EXPORT_SYMBOL(mpi_init);
#endif

#if defined(CONFIG_PCI)
#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
static void pcie_init(void)
{
    /* pcie clock enable*/
    PERF->blkEnables |= PCIE_CLK_EN;

    /* pcie serdes enable */
#if defined(CONFIG_BCM96816)   
    GPIO->SerdesCtl |= (SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
#endif
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    MISC->miscSerdesCtrl |= (SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
#endif    

    /* reset pcie and ext device */
    PERF->softResetB &= ~(SOFT_RST_PCIE|SOFT_RST_PCIE_EXT|SOFT_RST_PCIE_CORE);

#if defined(CONFIG_BCM96328)  || defined(CONFIG_BCM963268)
    PERF->softResetB &= ~SOFT_RST_PCIE_HARD;
#endif
   
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
    PERF->softResetB |= SOFT_RST_PCIE_HARD;
#endif
    mdelay(10);
    
    PERF->softResetB |= (SOFT_RST_PCIE|SOFT_RST_PCIE_CORE);
    mdelay(10);
    PERF->softResetB |= (SOFT_RST_PCIE_EXT);
    /* this is a critical delay */
    mdelay(200);
}
#endif
#endif

#if defined(CONFIG_BCM96368)

static int __init bcm6368_hw_init(void)
{
#if defined(CONFIG_PCI)
    unsigned long data;
#endif
    unsigned long GPIOOverlays;
    unsigned short gpio;

    /* Enable SPI interface */
    PERF->blkEnables |= SPI_CLK_EN;

    GPIO->GPIOMode = 0;

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {

        if (GPIOOverlays & BP_OVERLAY_PHY) {
            GPIO->GPIOMode |= (GPIO_MODE_ANALOG_AFE_0 | GPIO_MODE_ANALOG_AFE_1);
            GPIO->GPIODir |= (GPIO_MODE_ANALOG_AFE_0 | GPIO_MODE_ANALOG_AFE_1);
        }

        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            GPIO->GPIOMode |= GPIO_MODE_EPHY0_LED;
            GPIO->GPIODir |= GPIO_MODE_EPHY0_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            GPIO->GPIOMode |= GPIO_MODE_EPHY1_LED;
            GPIO->GPIODir |= GPIO_MODE_EPHY1_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            GPIO->GPIOMode |= GPIO_MODE_EPHY2_LED;
            GPIO->GPIODir |= GPIO_MODE_EPHY2_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_3) {
            GPIO->GPIOMode |= GPIO_MODE_EPHY3_LED;
            GPIO->GPIODir |= GPIO_MODE_EPHY3_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
            GPIO->GPIODir |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
        }

        if (GPIOOverlays & BP_OVERLAY_INET_LED) {
            /* Disable Activity indication until link is established */
            GPIO->AuxLedCtrl |= AUX_HW_DISAB_2;
            if ( BpGetWanDataLedGpio(&gpio) == BP_SUCCESS ) {
                if (gpio & BP_ACTIVE_LOW) {
                    /* Invert polarity of Inet activity LED */
                    GPIO->AuxLedCtrl |= (LED_STEADY_ON << AUX_MODE_SHFT_2);
                }
            }
            GPIO->GPIOMode |= GPIO_MODE_INET_LED;
            GPIO->GPIODir |= GPIO_MODE_INET_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_SPI_EXT_CS) {           
            /* Enable Overlay for SPI SS Pins */            
            GPIO->GPIOMode |= (GPIO_MODE_SPI_SSN2 | GPIO_MODE_SPI_SSN3);            
            /* Enable SPI Slave Select as Output Pins */            
            /* GPIO 28 is SS2, GPIO 29 is SS3 */            
            GPIO->GPIODir |= (GPIO_MODE_SPI_SSN2 | GPIO_MODE_SPI_SSN3);        
        }

#if defined(CONFIG_PCI)
        if (GPIOOverlays & BP_OVERLAY_PCI) {
            GPIO->GPIOMode |= (GPIO_MODE_PCI_GNT0 |
                GPIO_MODE_PCI_REQ0 |
                GPIO_MODE_PCI_INTB |
                GPIO_MODE_PCI_GNT1 |
                GPIO_MODE_PCI_REQ1);
            mpi_init();

            if (GPIOOverlays & BP_OVERLAY_CB) {
                GPIO->GPIOMode |= (GPIO_MODE_PCMCIA_VS2 |
                    GPIO_MODE_PCMCIA_VS1 |
                    GPIO_MODE_PCMCIA_CD2 |
                    GPIO_MODE_PCMCIA_CD1);
                mpi_DetectPcCard();
            }
            else {
                /*
                 * CardBus support is defaulted to Slot 0 because there is no external
                 * IDSEL for CardBus.  To disable the CardBus and allow a standard PCI
                 * card in Slot 0 set the cbus_idsel field to 0x1f.
                */
                data = MPI->pcmcia_cntl1;
                data |= CARDBUS_IDSEL;
                MPI->pcmcia_cntl1 = data;
            }
        }
#endif
    }

#if defined(CONFIG_USB)
    PERF->blkEnables |= USBH_CLK_EN;
    mdelay(100);
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->Setup |= USBH_IOC;
#else
    PERF->blkEnables &= ~(USBH_CLK_EN | USBH_IDDQ_EN);
#endif

    return 0;
}
#define bcm63xx_specific_hw_init() bcm6368_hw_init()

#elif defined(CONFIG_BCM96816)
/* FIXME: The following function is temporary and must be rewritten when the
   GPON specific parameters are supported in boardparms.c */
#if defined(CONFIG_BCM_GPON_FPGA)
static void reset_gpon_fpga(void)
{
    unsigned short fpgaResetGpio;
    int rc;
    int retryCnt = 10;
    unsigned int confDone;

#define GPON_FPGA_CONFIG_GPIO   37
#define GPON_FPGA_DONE_GPIO     38

#if defined(GPON_FPGA_PROGRAM)
    /* re-program FPGA */
    printk("Programming GPON FPGA...");

    GPIO->GPIODir |= GPIO_NUM_TO_MASK(GPON_FPGA_CONFIG_GPIO);

    GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(GPON_FPGA_CONFIG_GPIO);;

    msleep(1000);

    GPIO->GPIOio |= GPIO_NUM_TO_MASK(GPON_FPGA_CONFIG_GPIO);;

    printk("Done. GPIOio=0x%016llX\n", GPIO->GPIOio);
#endif

    /* check if FPGA was programmed successfully */
    do
    {
        confDone = (GPIO->GPIOio & GPIO_NUM_TO_MASK(GPON_FPGA_DONE_GPIO)) >> GPON_FPGA_DONE_GPIO;

        printk("GPON FPGA State (%d): 0x%X (0x%016llX, 0x%016llX / 0x%016llX)\n",
               retryCnt, confDone, GPIO->GPIODir, GPIO->GPIOio,
               GPIO_NUM_TO_MASK(GPON_FPGA_DONE_GPIO));

        if(!confDone)
        {
            msleep(500);
        }

    } while(!confDone && --retryCnt);

    if(!confDone)
    {
        printk("\nERROR: *** GPON FPGA is not programmed ***\n\n");
        return;
    }

    /* reset FPGA */
    printk("Resetting GPON FPGA...");

    rc = BpGetFpgaResetGpio(&fpgaResetGpio);
    if (rc != BP_SUCCESS) {
      printk("\nERROR: *** FPGA Reset GPIO not found **\n\n");
        return;
    }

    GPIO->GPIODir |= GPIO_NUM_TO_MASK(fpgaResetGpio);
    GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(fpgaResetGpio);

    msleep(1000);

    GPIO->GPIOio |= GPIO_NUM_TO_MASK(fpgaResetGpio);

    msleep(1000);

    printk("Done\n");
}
#endif /* CONFIG_BCM_GPON */

static int __init bcm6816_hw_init(void)
{
    unsigned long GPIOOverlays;

    /* Enable SPI interface and GPON MAC*/
    {
        /* Check if this is BHR */
        unsigned char portInfo6829;
        if ( (BP_SUCCESS == BpGet6829PortInfo(&portInfo6829)) &&
             (0 != portInfo6829))
        {
            /* BHR only needs SPI and GPON Serdes clock */
            PERF->blkEnables |= SPI_CLK_EN | GPON_SER_CLK_EN;
        }
        else
        {
            PERF->blkEnables |= SPI_CLK_EN | GPON_CLK_EN | GPON_SER_CLK_EN;
        }
    }

    udelay(500);
    /* Force the GPON serdes laser off so we don't introduce glitches on the fiber during init.*/
    GPON_SERDES->laserCfg = GPON_SERDES_LASERMODE_FORCE_OFF;
    GPON_SERDES->miscCfg = 5;

    GPIO->GPIOMode = 0;

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {

        if (GPIOOverlays & BP_OVERLAY_GPON_TX_EN_L) {
            GPIO->GPIOMode |= GPIO_MODE_GPON_TX_EN_L;
            GPIO->GPIODir |= GPIO_MODE_GPON_TX_EN_L;
        }
        else {
          GPIO->GPIODir |= GPIO_MODE_GPON_TX_EN_L;
          GPIO->GPIOio |= GPIO_MODE_GPON_TX_EN_L; /*Take optics out of reset*/
        }

        if (GPIOOverlays & BP_OVERLAY_GPHY_LED_0) {
            GPIO->GPIOMode |= GPIO_MODE_GPHY0_LED;
            GPIO->GPIODir |= GPIO_MODE_GPHY0_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_GPHY_LED_1) {
            GPIO->GPIOMode |= GPIO_MODE_GPHY1_LED;
            GPIO->GPIODir |= GPIO_MODE_GPHY1_LED;
        }

        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
            GPIO->GPIODir |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
        }

        if (GPIOOverlays & BP_OVERLAY_MOCA_LED) {
            GPIO->GPIOMode |= GPIO_MODE_MOCA_LED;
            GPIO->GPIODir |= GPIO_MODE_MOCA_LED;
        }

#if defined(CONFIG_BCM_GPON_FPGA)
        /* Initialize GPON FPGA (before PCI is initialized) */
        reset_gpon_fpga();
#endif /*CONFIG_BCM_GPON_FPGA*/

#if defined(CONFIG_PCI)
        if (GPIOOverlays & BP_OVERLAY_PCI) {
            GPIO->GPIOMode |= (
                GPIO_MODE_PCI_GNT0
               |GPIO_MODE_PCI_REQ0
               |GPIO_MODE_PCI_INTB
#if !defined(CONFIG_BCM_GPON_FPGA) /*Don't enable PCI1 on GPON FPGA board. GPON Fail LED is on that GPIO*/
               |GPIO_MODE_PCI_GNT1
               |GPIO_MODE_PCI_REQ1
#endif /*CONFIG_BCM_GPON_FPGA*/
            );

            mpi_init();
        }
        pcie_init();
#endif
    }

#if defined(CONFIG_USB)
    PERF->blkEnables |= USBH_CLK_EN;
    mdelay(100);
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->Setup |= USBH_IOC;
#else
    // Could also turn off PLL_IDDQ_PWRDN
    PERF->blkEnables &= ~USBH_CLK_EN;
#endif

    MISC->miscMoCARst &= ~(MISC_MOCA_RST_REF_DIV2RST | MISC_MOCA_RST_REF_FBDIVRST);
    MISC->miscMoCARst &= ~MISC_MOCA_RST_REF_VCRST;
    MISC->miscMoCARst &= ~(MISC_MOCA_RST_REF_OUTDIV_RESET_M_MASK | MISC_MOCA_RST_REF_MDIV2RST);
    MISC->miscMoCACtl |= (7 << MISC_MOCA_CTL_REF_QP_ICTRL_SHIFT);
    MISC->miscMoCARst &= ~MISC_MOCA_RST_REF_LD_RESET_STRT;
    /* Turn off MoCA to save power. It will be enabled if desired later. */
    PERF->softResetB &= ~(SOFT_RST_MOCA | SOFT_RST_MOCA_SYS | SOFT_RST_MOCA_CPU);


    // determine we need to setup MII over GPIO for 6818
    {
    unsigned long
        Flag = 0;
           
        BpGetMiiOverGpioFlag(&Flag);
    
        if (TRUE == Flag)
        {                    
            // set basemode for MII to robosw
            GPIO->GPIOBaseMode |= EN_MII_OVER_GPIO;
            
            //set pins for output direction 
            GPIO->GPIODir |= GPIO_MII_OVER_GPIO_OUTPUTS;

            //clear pins for input direction 
            GPIO->GPIODir &= ~(GPIO_MII_OVER_GPIO_INPUTS); 
        }   
    }

#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
    /* Enable power savings from DDR pads on this chip when DDR goes in Self-Refresh mode */
    DDR->PhyControl.IDLE_PAD_CONTROL = 0x00000172;
    DDR->PhyByteLane0Control.IDLE_PAD_CONTROL = 0x000fffff;
    DDR->PhyByteLane1Control.IDLE_PAD_CONTROL = 0x000fffff;
#endif

    return 0;
}
#define bcm63xx_specific_hw_init() bcm6816_hw_init()

#elif defined(CONFIG_BCM96362)

static int __init bcm6362_hw_init(void)
{
    unsigned long GPIOOverlays;
    unsigned short gpio;
    
    /* Set LED blink rate for activity LEDs to 80mS */
    LED->ledInit &= ~LED_FAST_INTV_MASK;
    LED->ledInit |= (LED_INTERVAL_20MS * 4) << LED_FAST_INTV_SHIFT;

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {
        /* Start with all HW LEDs disabled */
        LED->ledHWDis |= 0xFFFFFF;
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
            LED->ledInit |= LED_SERIAL_LED_EN;
        }

        if (GPIOOverlays & BP_OVERLAY_SPI_SSB2_EXT_CS) {           
            /* Enable Overlay for SPI SS2 Pin */            
             GPIO->GPIOMode |= GPIO_MODE_LS_SPIM_SSB2;                    
        }

        if (GPIOOverlays & BP_OVERLAY_SPI_SSB3_EXT_CS) {           
            /* Enable Overlay for SPI SS3 Pin */            
             GPIO->GPIOMode |= GPIO_MODE_LS_SPIM_SSB3;                    
        }

        /* Map HW LEDs to LED controller inputs and enable LED controller to drive GPIO */
        if (GPIOOverlays & BP_OVERLAY_USB_LED) {
            LED->ledLinkActSelLow |= ((1 << LED_USB_ACT) << LED_0_ACT_SHIFT);
            LED->ledLinkActSelLow |= ((1 << LED_USB_ACT) << LED_0_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_USB_ACT);
            LED->ledHWDis &= ~(1 << LED_USB_ACT);
        }
        if ( BpGetWanDataLedGpio(&gpio) == BP_SUCCESS ) {
            if ((gpio & BP_GPIO_NUM_MASK) == LED_INET_ACT) {
                /* WAN Data LED must be LED 1 */
                LED->ledLinkActSelLow |= ((1 << LED_INET_ACT) << LED_1_ACT_SHIFT);
                GPIO->LEDCtrl |= GPIO_NUM_TO_MASK(gpio);
            }
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET0 - 4)) << LED_4_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET0);
            LED->ledHWDis &= ~(1 << LED_ENET0);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET1 - 4)) << LED_5_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET1);
            LED->ledHWDis &= ~(1 << LED_ENET1);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET2 - 4)) << LED_6_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET2);
            LED->ledHWDis &= ~(1 << LED_ENET2);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_3) {
            LED->ledLinkActSelHigh |= ((1 << (LED_ENET3 - 4)) << LED_7_LINK_SHIFT);
            GPIO->LEDCtrl |= (1 << LED_ENET3);
            LED->ledHWDis &= ~(1 << LED_ENET3);
        }
    }

#if defined(CONFIG_USB)
    PERF->blkEnables |= USBH_CLK_EN;
    mdelay(100);
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->Setup |= USBH_IOC;
#else
    MISC->miscIddqCtrl |= MISC_IDDQ_CTRL_USBH;
    PERF->blkEnables &= ~USBH_CLK_EN;
#endif

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    PERF->blkEnables &= ~FAP_CLK_EN;
#endif

#if defined(CONFIG_PCI)
    /* Enable WOC */  
    PERF->blkEnables |=WLAN_OCP_CLK_EN;
    mdelay(10);
    PERF->softResetB &= ~(SOFT_RST_WLAN_SHIM_UBUS | SOFT_RST_WLAN_SHIM);
    mdelay(1);
    PERF->softResetB |= (SOFT_RST_WLAN_SHIM_UBUS | SOFT_RST_WLAN_SHIM);
    mdelay(1);

    WLAN_SHIM->ShimMisc = (WLAN_SHIM_FORCE_CLOCKS_ON|WLAN_SHIM_MACRO_SOFT_RESET);
    mdelay(1);
    WLAN_SHIM->MacControl = (SICF_FGC|SICF_CLOCK_EN);
    WLAN_SHIM->ShimMisc = WLAN_SHIM_FORCE_CLOCKS_ON;
    WLAN_SHIM->ShimMisc = 0;
    WLAN_SHIM->MacControl = SICF_CLOCK_EN;        	

    /* enable PCIE */
    pcie_init();
#endif    

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    // Turn off pll_use_lock to allow watchdog timer to reset the chip when
    // ephy_pwr_down_dll is set in ethernet sleep mode
    MISC->miscStrapBus &= ~MISC_STRAP_BUS_PLL_USE_LOCK;
    MISC->miscStrapOverride |= 1;
#endif

#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
    /* Enable power savings from DDR pads on this chip when DDR goes in Self-Refresh mode */
    DDR->PhyControl.IDLE_PAD_CONTROL = 0x00000172;
    DDR->PhyByteLane0Control.IDLE_PAD_CONTROL = 0x000fffff;
    DDR->PhyByteLane1Control.IDLE_PAD_CONTROL = 0x000fffff;
#endif

    if ( BpGetFemtoResetGpio(&gpio) == BP_SUCCESS ) {
        kerSysSetGpioState(gpio, kGpioActive);
    }

    return 0;
}
#define bcm63xx_specific_hw_init() bcm6362_hw_init()

#elif defined(CONFIG_BCM96328)

static int __init bcm6328_hw_init(void)
{
    unsigned long GPIOOverlays;
    unsigned short gpio;

    /* Set LED blink rate for activity LEDs to 80mS */
    LED->ledInit &= ~LED_FAST_INTV_MASK;
    LED->ledInit |= (LED_INTERVAL_20MS * 4) << LED_FAST_INTV_SHIFT;

    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {
        /* Start with all HW LEDs disabled */
        LED->ledHWDis |= 0xFFFFFF;
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->PinMuxSel |= PINMUX_SERIAL_LED_DATA;
            GPIO->PinMuxSel |= PINMUX_SERIAL_LED_CLK;
            LED->ledInit |= LED_SERIAL_LED_EN;
        }
        if ( BpGetWanDataLedGpio(&gpio) == BP_SUCCESS ) {
            if ((gpio & BP_GPIO_NUM_MASK) < 4) {
                /* WAN Data LED must be LED 0-3 */
                LED->ledLinkActSelLow |= ((1 << LED_INET_ACT) << ((gpio & BP_GPIO_NUM_MASK) * 4));
                LED->ledLinkActSelLow |= ((1 << LED_INET_ACT) << (((gpio & BP_GPIO_NUM_MASK) * 4) + LED_0_LINK_SHIFT));
                GPIO->GPIOMode |= GPIO_NUM_TO_MASK(gpio);

                /* The following two checks are for legacy schematics */
                if (gpio & BP_GPIO_SERIAL) {
                    /* If WAN Data LED is serial, then configure serial controller to shift it out */
                    LED->ledSerialMuxSelect |= GPIO_NUM_TO_MASK(gpio);
                }
                if ((gpio & BP_GPIO_NUM_MASK) == 0) {
                    /* In case INET_ACT LED is connected to GPIO_11 */
                    GPIO->PinMuxSel |= PINMUX_INET_ACT_LED;
                }
            }
        }
        /* Enable LED controller to drive GPIO */
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            GPIO->PinMuxSel |= PINMUX_EPHY0_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY0_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY0_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            GPIO->PinMuxSel |= PINMUX_EPHY1_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY1_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY1_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            GPIO->PinMuxSel |= PINMUX_EPHY2_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY2_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY2_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_3) {
            GPIO->PinMuxSel |= PINMUX_EPHY3_ACT_LED;
            GPIO->GPIOMode |= (1 << EPHY3_SPD_LED);
            LED->ledHWDis &= ~(1 << EPHY3_SPD_LED);
        }
        if (GPIOOverlays & BP_OVERLAY_SPI_EXT_CS) { 
            GPIO->PinMuxSelOther &= ~PINMUX_SEL_SPI2_MASK;
            GPIO->PinMuxSelOther |= PINMUX_SEL_SPI2;                            
        }

        /* Enable PCIe CLKREQ signal */
        if (GPIOOverlays & BP_OVERLAY_PCIE_CLKREQ) {
            GPIO->PinMuxSel |= PINMUX_PCIE_CLKREQ;
        }
    }

#if defined(CONFIG_USB)
    PERF->blkEnables |= USBH_CLK_EN;
    mdelay(100);
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->Setup |= USBH_IOC;
#else
    MISC->miscIddqCtrl |= MISC_IDDQ_CTRL_USBH;
    PERF->blkEnables &= ~USBH_CLK_EN;
#endif

#if !(defined(CONFIG_BCM_ADSL) || defined(CONFIG_BCM_ADSL_MODULE))
    MISC->miscIddqCtrl |= MISC_IDDQ_CTRL_SAR | MISC_IDDQ_CTRL_ADSL2_AFE | MISC_IDDQ_CTRL_ADSL2_PHY | MISC_IDDQ_CTRL_ADSL2_MIPS;
    PERF->blkEnables &= ~(SAR_CLK_EN | ADSL_CLK_EN | ADSL_AFE_EN | ADSL_QPROC_EN | PHYMIPS_CLK_EN);
    MISC->miscPllCtrlSysPll2 |= (1<<6); // Channel 5
    MISC->miscPllCtrlDdrPll |= 1; // Channel 5
#endif

#if defined(CONFIG_PCI)
    /* enable PCIE */
    pcie_init();
#endif    

#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
    /* Enable power savings from DDR pads on this chip when DDR goes in Self-Refresh mode */
    DDR->PhyControl.IDLE_PAD_CONTROL = 0x00000172;
    DDR->PhyByteLane0Control.IDLE_PAD_CONTROL = 0x000fffff;
    DDR->PhyByteLane1Control.IDLE_PAD_CONTROL = 0x000fffff;
#endif
    return 0;
}

#define bcm63xx_specific_hw_init() bcm6328_hw_init()

#elif defined(CONFIG_BCM963268)

int map_63268_vdsl_override(int val) {
    switch (val & BP_GPIO_NUM_MASK) {
        case (BP_GPIO_10_AH & BP_GPIO_NUM_MASK):
        case (BP_GPIO_11_AH & BP_GPIO_NUM_MASK):
	    return(GPIO_BASE_VDSL_PHY_OVERRIDE_0);
        case (BP_GPIO_12_AH & BP_GPIO_NUM_MASK):
        case (BP_GPIO_13_AH & BP_GPIO_NUM_MASK):
	    return(GPIO_BASE_VDSL_PHY_OVERRIDE_1);
        case (BP_GPIO_24_AH & BP_GPIO_NUM_MASK):
        case (BP_GPIO_25_AH & BP_GPIO_NUM_MASK):
	    return(GPIO_BASE_VDSL_PHY_OVERRIDE_2);
        case (BP_GPIO_26_AH & BP_GPIO_NUM_MASK):
        case (BP_GPIO_27_AH & BP_GPIO_NUM_MASK):
	    return(GPIO_BASE_VDSL_PHY_OVERRIDE_3);
        default:
            return(0);
    }
}

static int __init bcm63268_hw_init(void)
{
    unsigned long GPIOOverlays;
    unsigned short gpio;
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
#if defined(CONFIG_BCM_1V2REG_AUTO_SHUTDOWN)
    uint32 startCount, endCount;
    int diff; 
#endif
    
    /* Turn off test bus */
    PERF->blkEnables &= ~TBUS_CLK_EN;


#if !(defined(CONFIG_BCM_XTMRT) || defined(CONFIG_BCM_XTMRT_MODULE))
    // Disable SAR if unused
    PERF->blkEnables &= ~( SAR_CLK_EN );
    MISC->miscIddqCtrl |= MISC_IDDQ_CTRL_SAR;
#endif

#if defined(CONFIG_BCM_XTMRT) || defined(CONFIG_BCM_XTMRT_MODULE) 
    // Phy should always be powered down if XTM is deselected
    if (kerSysGetDslPhyEnable()) {
#else
    if (0) {
#endif
        MISC->miscIddqCtrl &= ~(MISC_IDDQ_CTRL_VDSL_PHY 
				| MISC_IDDQ_CTRL_VDSL_MIPS
				| MISC_IDDQ_CTRL_SAR);
    } 
    else 
    {
        /* If there is no phy support, shut off power */
        PERF->blkEnables &= ~( PHYMIPS_CLK_EN
				| VDSL_CLK_EN 
				| VDSL_AFE_EN | VDSL_QPROC_EN );
        MISC->miscIddqCtrl |= (MISC_IDDQ_CTRL_VDSL_PHY 
				| MISC_IDDQ_CTRL_VDSL_MIPS);
    }

    /* Set LED blink rate for activity LEDs to 80mS */
    LED->ledInit &= ~LED_FAST_INTV_MASK;
    LED->ledInit |= (LED_INTERVAL_20MS * 4) << LED_FAST_INTV_SHIFT;

    /* Start with all HW LEDs disabled */
    LED->ledHWDis |= 0xFFFFFF;


    BpGetEthernetMacInfo(EnetInfo, BP_MAX_ENET_MACS);

    /* Enable HW to drive LEDs for Ethernet ports in use */
    if (EnetInfo[0].sw.port_map & (1 << 0)) {
        LED->ledHWDis &= ~(1 << LED_EPHY0_ACT);
        LED->ledHWDis &= ~(1 << LED_EPHY0_SPD);
    }
    if (EnetInfo[0].sw.port_map & (1 << 1)) {
        LED->ledHWDis &= ~(1 << LED_EPHY1_ACT);
        LED->ledHWDis &= ~(1 << LED_EPHY1_SPD);
    }
    if (EnetInfo[0].sw.port_map & (1 << 2)) {
        LED->ledHWDis &= ~(1 << LED_EPHY2_ACT);
        LED->ledHWDis &= ~(1 << LED_EPHY2_SPD);
    }
    if (EnetInfo[0].sw.port_map & (1 << 3)) {
        LED->ledHWDis &= ~(1 << LED_GPHY0_ACT);
/*In Actiontec 63268 Chip, GPIO 0/1 is used for Internet LED , not for GPHY speed LED*/
#ifndef AEI_63168_CHIP
        LED->ledHWDis &= ~(1 << LED_GPHY0_SPD0);
        LED->ledHWDis &= ~(1 << LED_GPHY0_SPD1);
#endif
        LED->ledLinkActSelLow |= ((1 << LED_GPHY0_SPD0) << LED_0_LINK_SHIFT);
        LED->ledLinkActSelLow |= ((1 << LED_GPHY0_SPD1) << LED_1_LINK_SHIFT);
        GPIO->RoboSWLEDControl |= LED_BICOLOR_SPD;
    }

    /* UART2 - SDIN and SDOUT are separate for flexibility */
    {
        unsigned short Uart2Sdin;
        unsigned short Uart2Sdout;
        if (BpGetUart2SdinGpio(&Uart2Sdin) == BP_SUCCESS) {
            switch (Uart2Sdin & BP_GPIO_NUM_MASK) {
            case (BP_GPIO_12_AH & BP_GPIO_NUM_MASK):
                GPIO->GPIOMode |= (GPIO_MODE_UART2_SDIN);
                break;
            case (BP_GPIO_26_AH & BP_GPIO_NUM_MASK):
                GPIO->GPIOMode |= (GPIO_MODE_UART2_SDIN2);
                break;
            }
        }
        if (BpGetUart2SdoutGpio(&Uart2Sdout) == BP_SUCCESS) {
            switch (Uart2Sdout & BP_GPIO_NUM_MASK) {
            case (BP_GPIO_13_AH & BP_GPIO_NUM_MASK):
                GPIO->GPIOMode |= (GPIO_MODE_UART2_SDOUT);
                break;
            case (BP_GPIO_27_AH & BP_GPIO_NUM_MASK):
                GPIO->GPIOMode |= (GPIO_MODE_UART2_SDOUT2);
                break;
            }
        }
    }


    if( BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS ) {
        if (GPIOOverlays & BP_OVERLAY_SERIAL_LEDS) {
            GPIO->GPIOMode |= (GPIO_MODE_SERIAL_LED_CLK | GPIO_MODE_SERIAL_LED_DATA);
            LED->ledInit |= LED_SERIAL_LED_EN;
        }
        if ( BpGetWanDataLedGpio(&gpio) == BP_SUCCESS ) {
            if ((gpio & BP_GPIO_NUM_MASK) == LED_INET_ACT) {
                /* WAN Data LED must be LED 8 */
                if (!(gpio & BP_GPIO_SERIAL)) {
                    /* If LED is not serial, enable corresponding GPIO */
                    GPIO->LEDCtrl |= GPIO_NUM_TO_MASK(gpio);
                }
            }
        }
        /* Enable LED controller to drive GPIO when LEDs are connected to GPIO pins */
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_0) {
            GPIO->LEDCtrl |= (1 << LED_EPHY0_ACT);
            GPIO->LEDCtrl |= (1 << LED_EPHY0_SPD);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_1) {
            GPIO->LEDCtrl |= (1 << LED_EPHY1_ACT);
            GPIO->LEDCtrl |= (1 << LED_EPHY1_SPD);
        }
        if (GPIOOverlays & BP_OVERLAY_EPHY_LED_2) {
            GPIO->LEDCtrl |= (1 << LED_EPHY2_ACT);
            GPIO->LEDCtrl |= (1 << LED_EPHY2_SPD);
        }
        if (GPIOOverlays & BP_OVERLAY_GPHY_LED_0) {
            GPIO->LEDCtrl |= (1 << LED_GPHY0_ACT);
            GPIO->LEDCtrl |= (1 << LED_GPHY0_SPD0);
            GPIO->LEDCtrl |= (1 << LED_GPHY0_SPD1);
        }
        if (GPIOOverlays & BP_OVERLAY_PHY) {
            unsigned short IntLdMode = 0xffff;
            unsigned short IntLdPwr = 0xffff;
            unsigned short ExtLdMode = 0xffff;
            unsigned short ExtLdPwr = 0xffff;
            int ExplicitLdControl ;
            ExplicitLdControl = (BpGetIntAFELDModeGpio(&IntLdMode) == BP_SUCCESS);
            ExplicitLdControl = ExplicitLdControl ||  (BpGetIntAFELDPwrGpio(&IntLdPwr) == BP_SUCCESS);
            ExplicitLdControl = ExplicitLdControl ||  (BpGetExtAFELDModeGpio(&ExtLdMode) == BP_SUCCESS);
            ExplicitLdControl = ExplicitLdControl ||  (BpGetExtAFELDPwrGpio(&ExtLdPwr) == BP_SUCCESS);

            if (!ExplicitLdControl) {
                /* default if boardparms doesn't specify a subset */
                GPIO->GPIOBaseMode |= GPIO_BASE_VDSL_PHY_OVERRIDE_0  | GPIO_BASE_VDSL_PHY_OVERRIDE_1;
            } else {
                GPIO->GPIOBaseMode |= map_63268_vdsl_override(IntLdMode) 
                    |  map_63268_vdsl_override(IntLdPwr) 
                    | map_63268_vdsl_override(ExtLdMode)
                    |  map_63268_vdsl_override(ExtLdPwr) ;
            } 
        }

        /* Enable PCIe CLKREQ signal */
        if (GPIOOverlays & BP_OVERLAY_PCIE_CLKREQ) {
            GPIO->GPIOMode |= GPIO_MODE_PCIE_CLKREQ_B;
        }

        if (GPIOOverlays & BP_OVERLAY_USB_LED) {
            LED->ledHWDis &= ~(1 << LED_USB_ACT);
        }
        /* Enable HS SPI SS Pins */
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB4_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_4;
        }
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB5_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_5;
        }
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB6_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_6;
        }
        if (GPIOOverlays & BP_OVERLAY_HS_SPI_SSB7_EXT_CS) {
             GPIO->GPIOMode |= GPIO_MODE_HS_SPI_SS_7;
        }
    }

    {
        unsigned short EphyBaseAddr;
        if( BpGetEphyBaseAddress(&EphyBaseAddr) == BP_SUCCESS ) {
            GPIO->RoboswEphyCtrl |= ((EphyBaseAddr >>3) & 0x3) << EPHY_PHYAD_BASE_ADDR_SHIFT;
        }
    }


#if defined(CONFIG_USB)
    PERF->blkEnables |= USBH_CLK_EN;
    PERF->softResetB |= SOFT_RST_USBH;
    TIMER->ClkRstCtl |= USB_REF_CLKEN;
    MISC->miscIddqCtrl &= ~MISC_IDDQ_CTRL_USBH;
    mdelay(100);
    USBH->SwapControl = EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP;
    USBH->Setup |= USBH_IOC;
    USBH->Setup &= ~USBH_IPP;
    USBH->PllControl1 &= ~(PLLC_PLL_IDDQ_PWRDN | PLLC_PLL_PWRDN_DELAY);
#else
    MISC->miscIddqCtrl |= MISC_IDDQ_CTRL_USBH;
    PERF->blkEnables &= ~USBH_CLK_EN;
#endif

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    PERF->blkEnables &= ~FAP0_CLK_EN;
    PERF->blkEnables &= ~FAP1_CLK_EN;
#endif

#if defined(CONFIG_PCI)
    /* Enable WOC */  
    PERF->blkEnables |=WLAN_OCP_CLK_EN;
    mdelay(10);
    PERF->softResetB &= ~(SOFT_RST_WLAN_SHIM_UBUS | SOFT_RST_WLAN_SHIM);
    mdelay(1);
    PERF->softResetB |= (SOFT_RST_WLAN_SHIM_UBUS | SOFT_RST_WLAN_SHIM);
    mdelay(1);
 
    WLAN_SHIM->ShimMisc = (WLAN_SHIM_FORCE_CLOCKS_ON|WLAN_SHIM_MACRO_SOFT_RESET);
    mdelay(1);
    WLAN_SHIM->MacControl = (SICF_FGC|SICF_CLOCK_EN);
    WLAN_SHIM->ShimMisc = WLAN_SHIM_FORCE_CLOCKS_ON;
    WLAN_SHIM->ShimMisc = 0;
    WLAN_SHIM->MacControl = SICF_CLOCK_EN;      


    /* enable PCIE */
    pcie_init();
#endif    

#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
    /* Enable power savings from DDR pads on this chip when DDR goes in Self-Refresh mode */
    MEMC->PhyControl.IDLE_PAD_CONTROL = 0x00000172;
    MEMC->PhyByteLane0Control.IDLE_PAD_CONTROL = 0x000fffff;
    MEMC->PhyByteLane1Control.IDLE_PAD_CONTROL = 0x000fffff;
#endif

#if defined(CONFIG_BCM_1V2REG_AUTO_SHUTDOWN)
    /*
     * Determine if internal VREG is used.
     * If not, disable it to improve WLAN performance at 5GHz
     * The ring oscillators are affected when varying the 1V2 voltage
     * So take a measure of the ring osc count, then raise the internal regulator voltage and remeasure
     * If the ring osc count changed as expected than internal regulators are used
     */
    printk("Internal 1P2 VREG will be shutdown if unused...");

    /* Take the first ring osc measurement */
    GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK | RING_OSC_COUNT_RESET | RING_OSC_IRQ;
    GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK | (2 << RING_OSC_SELECT_SHIFT);
    GPIO->RingOscCtrl0 = RING_OSC_512_CYCLES;
    while (!(GPIO->RingOscCtrl1 & RING_OSC_IRQ));
    startCount = GPIO->RingOscCtrl1 & RING_OSC_COUNT_MASK;

    /* Increase internal 1V2 slightly and see if the ring osc is speeding up */
    MISC->miscVregCtrl1 += 8;
    MISC->miscVregCtrl0 |= MISC_VREG_CONTROL0_REG_RESET_B;

    /* Take the second ring osc measurement */
    GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK | RING_OSC_COUNT_RESET | RING_OSC_IRQ;
    GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK | (2 << RING_OSC_SELECT_SHIFT);
    GPIO->RingOscCtrl0 = RING_OSC_512_CYCLES;
    while (!(GPIO->RingOscCtrl1 & RING_OSC_IRQ));
    endCount = GPIO->RingOscCtrl1 & RING_OSC_COUNT_MASK;

    /* Reset the internal 1V2 to its original value */
    MISC->miscVregCtrl1 -= 8;

    /*
     * A negative difference or a small positive difference indicates that an external regulator is used
     * This code was calibrated by repeating the measurements thousands of times and looking for a safe value
     * Safe means avoiding at all costs being wrong by shutting down the internal regulator when it is in use
     * It is better to be wrong by leaving the internal regulator running when an external regulator is used
     */
    diff = startCount - endCount;
    if (diff < 300) {
        printk("Unused, turn it off (%08lx-%08lx=%d<300)\n", startCount, endCount,diff);
        /* Turn off internal 1P2 regulator */
        MISC->miscVregCtrl0 |= MISC_VREG_CONTROL0_REG_RESET_B | MISC_VREG_CONTROL0_POWER_DOWN_1;
    } else {
        printk("Used, leave it on (%08lx-%08lx=%d>=300)\n", startCount, endCount, diff);
    }
#elif defined(CONFIG_BCM_1V2REG_ALWAYS_SHUTDOWN)
    printk("Internal 1P2 VREG is forced to be shutdown\n");
    MISC->miscVregCtrl0 |= MISC_VREG_CONTROL0_REG_RESET_B | MISC_VREG_CONTROL0_POWER_DOWN_1;
#elif defined(CONFIG_BCM_1V2REG_NEVER_SHUTDOWN)
    printk("Internal 1P2 VREG is forced to remain enabled\n");
#endif

    if ( BpGetFemtoResetGpio(&gpio) == BP_SUCCESS ) {
        kerSysSetGpioState(gpio, kGpioActive);
    }

    return 0;
}

#define bcm63xx_specific_hw_init() bcm63268_hw_init()

#endif

static int __init bcm63xx_hw_init(void)
{
#if !defined(CONFIG_BRCM_IKOS)
    kerSysFlashInit();
#endif

    return bcm63xx_specific_hw_init();
}
arch_initcall(bcm63xx_hw_init);


static int __init brcm63xx_setup(void)
{
    extern int panic_timeout;

    _machine_restart = brcm_machine_restart;
    _machine_halt = brcm_machine_halt;
    pm_power_off = brcm_machine_halt;

    panic_timeout = 1;

    return 0;
}

arch_initcall(brcm63xx_setup);


unsigned long getMemorySize(void)
{
#if defined(CONFIG_BRCM_IKOS)
    return(31 * 1024 * 1024); /* voice DSP is loaded after this amount */
#elif defined(CONFIG_BRCM_MEMORY_RESTRICTION_16M)
    return(16 * 1024 * 1024); 
#elif defined(CONFIG_BRCM_MEMORY_RESTRICTION_32M)
    return(32 * 1024 * 1024); 
#elif defined(CONFIG_BRCM_MEMORY_RESTRICTION_64M)
    return(64 * 1024 * 1024); 
#elif defined(CONFIG_BCM96368)
    unsigned long size;
    unsigned long memCfg;

    size = 1;
    memCfg = MEMC->Config;
    /* Number of column bits */
    size <<= (((memCfg & MEMC_COL_MASK) >> MEMC_COL_SHFT) + 8);
    /* Plus number of row bits */
    size <<= (((memCfg & MEMC_ROW_MASK) >> MEMC_ROW_SHFT) + 11);
    /* Plus bus width */
    if (((memCfg & MEMC_WIDTH_MASK) >> MEMC_WIDTH_SHFT) == MEMC_32BIT_BUS)
        size <<= 2;
    else
        size <<= 1;

    /* Plus number of banks */
    size <<= 2;

    return( size );
#elif defined(CONFIG_BCM96816) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
    return (DDR->CSEND << 24);
#else
    return (((MEMC->CSEND > 16) ? 16 : MEMC->CSEND) << 24);
#endif
}


/* Pointers to memory buffers allocated for the DSP module */
void *dsp_core;
void *dsp_init;
EXPORT_SYMBOL(dsp_core);
EXPORT_SYMBOL(dsp_init);
void __init allocDspModBuffers(void);
/*
*****************************************************************************
** FUNCTION:   allocDspModBuffers
**
** PURPOSE:    Allocates buffers for the init and core sections of the DSP
**             module. This module is special since it has to be allocated
**             in the 0x800.. memory range which is not mapped by the TLB.
**
** PARAMETERS: None
** RETURNS:    Nothing
*****************************************************************************
*/
void __init allocDspModBuffers(void)
{
#if defined(CONFIG_BCM_ENDPOINT_MODULE)
    printk("Allocating memory for DSP module core and initialization code\n");

  dsp_core = (void*)((DSP_CORE_SIZE > 0) ? alloc_bootmem((unsigned long)DSP_CORE_SIZE) : NULL);
  dsp_init = (void*)((DSP_INIT_SIZE > 0) ? alloc_bootmem((unsigned long)DSP_INIT_SIZE) : NULL);

  printk("Allocated DSP module memory - CORE=0x%x SIZE=%d, INIT=0x%x SIZE=%d\n",
         (unsigned int)dsp_core, DSP_CORE_SIZE, (unsigned int)dsp_init , DSP_INIT_SIZE);
#endif
}

