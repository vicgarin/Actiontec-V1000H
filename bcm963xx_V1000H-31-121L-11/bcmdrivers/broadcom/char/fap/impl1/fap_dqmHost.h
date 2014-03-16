/************************************************************
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

#ifndef __FAPDQMHOST_H_INCLUDED__ 
#define __FAPDQMHOST_H_INCLUDED__

/******************************************************************************
* File Name  : fap_dqmHost.h                                                  *
*                                                                             *
* Description: This is a support header file for the FAP on the host side.    *
******************************************************************************/

/* register access */
/*  The following two macros are never invoked
#define dqmWriteFifoDataWordHost(__fapIdx, __index, __wordNo, __data) *((unsigned int *)(((unsigned int)&hostDqmQDataReg->q[__index])+(__wordNo*4))) = __data
#define dqmReadFifoDataWordHost(__fapIdx,__index, __wordNo) *((unsigned int *)(((unsigned int)&hostDqmQDataReg->q[__index])+(__wordNo*4)))
*/
#define dqmReadNotEmptyIrqMskHost(fapIdx) FAP_HOST_REG_RD(hostDqmReg(fapIdx)->mips_not_empty_irq_msk)
#define dqmReadNotEmptyIrqStsHost(fapIdx) FAP_HOST_REG_RD(hostDqmReg(fapIdx)->not_empty_irq_sts)
#define dqmReadNotEmptyStsHost(fapIdx)    FAP_HOST_REG_RD(hostDqmReg(fapIdx)->not_empty_sts)
#define dqmReadLowWtmkIrqMskHost(fapIdx)  FAP_HOST_REG_RD(hostDqmReg(fapIdx)->mips_low_wtmk_irq_msk)
#define dqmReadLowWtmkIrqStsHost(fapIdx)  FAP_HOST_REG_RD(hostDqmReg(fapIdx)->low_wtmk_irq_sts)
#define dqmClearNotEmptyIrqStsHost(fapIdx, __qb) FAP_HOST_REG_WR(hostDqmReg(fapIdx)->not_empty_irq_sts, __qb)
#define dqmClearLowWtmkIrqStsHost(fapIdx, __qb)  FAP_HOST_REG_WR(hostDqmReg(fapIdx)->low_wtmk_irq_sts, __qb)
#define dqmEnableNotEmptyIrqMskHost(fapIdx, __qb) FAP_HOST_REG_WR(hostDqmReg(fapIdx)->mips_not_empty_irq_msk, dqmReadNotEmptyIrqMskHost(fapIdx) | __qb)
#define dqmEnableLowWtmkIrqMskHost(fapIdx, __qb) FAP_HOST_REG_WR(hostDqmReg(fapIdx)->mips_low_wtmk_irq_msk, dqmReadLowWtmkIrqMskHost(fapIdx) | __qb)
#define dqmRecvAvailableHost(fapIdx, __q) ((dqmReadNotEmptyStsHost(fapIdx)) & (1 << (__q)))
#define dqmXmitAvailableHost(fapIdx, __q) FAP_HOST_REG_RD(hostDqmQCntrlReg(fapIdx)->q[__q].sts)

/* type definitions */
typedef void(* dqmHandlerHost_t)(unsigned long arg);

/* prototypes */
int dqmHandlerRegisterHost(uint32 mask, dqmHandlerHost_t handler,
                           unsigned long arg);
void dqmIrqHandlerHost(uint32 fapIdx);

int _dqmHandlerEnableHost(uint32 mask, bool enable);

/******************************************************************************
* Function: dqmXmitMsgHost                                                    *
*                                                                             *
* Description: send a DQM token - host side                                   *
******************************************************************************/
static inline void dqmXmitMsgHost(
                    uint32 fapIdx,
                    uint32 queue, 
                    uint32 tokenSize, 
                    DQMQueueDataReg_S *t)
{
    volatile uint32 *wp = (volatile uint32 *)(&hostDqmQDataReg(fapIdx)->q[queue].word0);
    uint32 *rp = (uint32 *)(t);
    int i;

    for(i=0; i<tokenSize; ++i)
    {
        *wp++ = *rp++;
    }
}

/******************************************************************************
* Function: dqmRecvMsgHost                                                    *
*                                                                             *
* Description: get a DQM token - host side                                    *
******************************************************************************/
static inline void dqmRecvMsgHost(
                    uint32 fapIdx,
                    uint32 queue, 
                    uint32 tokenSize, 
                    DQMQueueDataReg_S *t)
{
    uint32 *wp = (uint32 *)(t);
    volatile uint32 *rp = (volatile uint32 *)(&hostDqmQDataReg(fapIdx)->q[queue].word0);
    int i;

    for(i=0; i<tokenSize; ++i)
    {
        *wp++ = *rp++;
    }
}

#endif /* __FAPDQMHOST_H_INCLUDED__ */
