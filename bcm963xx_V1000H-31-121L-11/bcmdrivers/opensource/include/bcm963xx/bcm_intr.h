/*
<:copyright-gpl 
 Copyright 2003 Broadcom Corp. All Rights Reserved. 
 
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

#ifndef __BCM_INTR_H
#define __BCM_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

#if defined(CONFIG_BCM963268)
#include <63268_intr.h>
#endif
#if defined(CONFIG_BCM96328)
#include <6328_intr.h>
#endif
#if defined(CONFIG_BCM96368)
#include <6368_intr.h>
#endif
#if defined(CONFIG_BCM96816)
#include <6816_intr.h>
#endif
#if defined(CONFIG_BCM96362)
#include <6362_intr.h>
#endif

/* defines */
/* The following definitions must match the definitions in linux/interrupt.h for 
   irqreturn_t and irq_handler_t */

typedef enum {  
    BCM_IRQ_NONE = 0,
    BCM_IRQ_HANDLED = 1,
    BCM_IRQ_WAKE_THREAD = 2
} FN_HANDLER_RT;
typedef FN_HANDLER_RT (*FN_HANDLER) (int, void *);


/** used by BcmHalMapInterruptEx */
typedef enum {
   INTR_REARM_NO=0,
   INTR_REARM_YES=1
} INTR_REARM_MODE_ENUM;

/** used by BcmHalMapInterruptEx */
typedef enum {
   INTR_AFFINITY_DEFAULT=0,
   INTR_AFFINITY_TP1_ONLY=1,  /**< set affinity to TP1, complain if no TP1 */
   INTR_AFFINITY_TP1_IF_POSSIBLE=2, /**< set affinity to TP1, silently use
                                         TP0 if TP1 not available */
   INTR_AFFINITY_BOTH_IF_POSSIBLE=3 /**< set affinity to both TP, silently
                                         use TP0 if TP1 not available */
} INTR_AFFINITY_MODE_ENUM;


/* prototypes */
/* NOTE: The enable/disable functions do not appear symmetric, allowing enabling but not disabling of external interrupts. */
extern void enable_brcm_irq(unsigned int irq);
extern void disable_brcm_irq(unsigned int irq);
extern unsigned int BcmHalMapInterrupt(FN_HANDLER isr, unsigned int param, unsigned int interruptId);
extern unsigned int BcmHalMapInterruptVoip(FN_HANDLER isr, unsigned int param, unsigned int interruptId);
extern unsigned int BcmHalMapInterruptEx(FN_HANDLER isr,
                                         unsigned int param,
                                         unsigned int interruptNum,
                                         const char *interruptName,
                                         INTR_REARM_MODE_ENUM rearmMode,
                                         INTR_AFFINITY_MODE_ENUM affinMode);
extern void dump_intr_regs(void);

/* bill BCM interrupt control prototypes */
/* NOTE: These routines only operate on HW interrupts between INTERRUPT_ID_TIMER and INTERRUPT_ID_I2C. */
extern void disable_brcm_irqsave(unsigned int irq, unsigned long stateSaveArray[]);
extern void restore_brcm_irqsave(unsigned int irq, unsigned long stateSaveArray[]);

/* compatibility definitions */
#define BcmHalInterruptEnable(irq)      enable_brcm_irq( irq )
#define BcmHalInterruptDisable(irq)     disable_brcm_irq( irq )

#ifdef __cplusplus
    }
#endif

#endif
