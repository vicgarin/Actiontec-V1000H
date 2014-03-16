#ifndef __FAPIRQ_H_INCLUDED__
#define __FAPIRQ_H_INCLUDED__

/***********************************************************
 *
 * <:copyright-BRCM:2009:DUAL/GPL:standard
 * 
 *    Copyright (c) 2009 Broadcom Corporation
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 ************************************************************/

/*
 *******************************************************************************
 * File Name  : fap4ke_irq.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#define FAP4KE_IRQ_HANDLERS_MAX 8

#define fap4keIrq_enable(irq)    _fap4keIrq_enable(irq)

#define fap4keIrq_disable(irq)   _fap4keIrq_disable(irq)

#define fap4keIrq_register(_handler, _arg, irq) \
    __fap4keIrq_register(_handler, _arg, irq, #_handler)

typedef enum {                     /* Usage (see fap_hw.h for register definitions): */
    FAP4KE_IRQ_GROUP_FAP,          /* For interrupts in the irq_4ke_status register */
    FAP4KE_IRQ_GROUP_CHIP_EXTRA2,  /* For interrupts in the extra2ChipIrqStatus register */
    FAP4KE_IRQ_GROUP_CHIP_EXTRA,   /* For interrupts in the extraChipIrqStatus register 
                                     (or fap1IrqMaskLo register for 963268) */
    FAP4KE_IRQ_GROUP_CHIP,         /* For interrupts in the chipIrqStatus register */
    FAP4KE_IRQ_GROUP_MAX
} fap4keIrq_irqGroup_t;

typedef enum {
    FAP4KE_IRQ_ENET_RX_0 = 0,
    FAP4KE_IRQ_ENET_RX_1,
    FAP4KE_IRQ_ENET_RX_2,
    FAP4KE_IRQ_ENET_RX_3,
    FAP4KE_IRQ_ENET_RX_ALL,
    FAP4KE_IRQ_DQM,
    FAP4KE_IRQ_SAR_DMA_0,
    FAP4KE_IRQ_SAR_DMA_1,
    FAP4KE_IRQ_SAR_DMA_2,
    FAP4KE_IRQ_SAR_DMA_3,
    FAP4KE_IRQ_SAR_ALL,
    FAP4KE_IRQ_GENERAL_PURPOSE_INPUT,
    FAP4KE_IRQ_TIMER_0,
} fap4keIrqs;


typedef fapRet(*fap4keIrq_handler_t)(uint32);

typedef struct
{
    fap4keIrq_handler_t handler;
    uint32 arg;
    fap4keIrq_irqGroup_t irqGroup;
    volatile uint32 *irqAddr;
    uint32 irqMask;
    const char *name;
    uint32 count;
} fap4keIrq_handlerInfo_t;


void fap4keIrq_init(void);

fapRet __fap4keIrq_register(    fap4keIrq_handler_t handler, 
                                uint32 arg, 
                                fap4keIrqs irq, 
                                const char *name);

void _fap4keIrq_enable(fap4keIrqs irq);

void _fap4keIrq_disable(fap4keIrqs irq);

void fap4keIrq_mainHandler(void);

void printIrqStats(void);

#endif  /* defined(__FAPIRQ_H_INCLUDED__) */
