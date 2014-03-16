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

#ifndef __FAP4KEDQM_H_INCLUDED__ 
#define __FAP4KEDQM_H_INCLUDED__


/******************************************************************************
* File Name  : fap4ke_dqm.h                                                   *
*                                                                             *
* Description: This is the 4ke header file for DQM implementation             * 
*              Not required for the Host MIPS.                                *
******************************************************************************/

#include "fap_hw.h"
#include "fap4ke_memory.h"

/* queue configuration structure */
typedef struct {
    uint8  queue;        // target queue (0-31)
    uint8  direction;    // 0 = host->4ke   1 = 4ke->host 
        #define HOST_2_4KE  0
        #define _4KE_2_HOST 1
        #define POLLED      2
    uint16 tokenSize;    // queue token size in words  (1-4)
    uint16 memSize;      // queue memory size in words (tokenSize * N)
    uint16 lowWaterMark; // queue low water mark in tokens
    uint16 enableIrq;    // 0 = disable irq   1 = enable irq
} dqmConfig_s;

/* irq handler access */
#define g_4keDqmHandlerInfo    (p4keDspramGbl->dqm.handlerInfo)
#define g_4keDqmHandlerCount   (p4keDspramGbl->dqm.handlerCount)

/* Queue Shared Memory (QSM) */
#define DQM_TOT_MEM_SZ (FAP_QSM_SIZE / 4) // Size in words
#define DQM_START_ADDR 0             // QSM Start Address 

/* register access */
#define dqmWriteFifoDataWord4ke(__index, __wordNo, __data) *((unsigned int *)(((unsigned int)&_4keDqmQDataReg->q[__index])+(__wordNo*4))) = __data
#define dqmReadFifoDataWord4ke(__index, __wordNo) *((unsigned int *)(((unsigned int)&_4keDqmQDataReg->q[__index])+(__wordNo*4)))
#define dqmReadNotEmptyIrqMsk4ke() FAP_4KE_REG_RD(_4keDqmReg->_4ke_not_empty_irq_msk)
#define dqmReadNotEmptyIrqSts4ke() FAP_4KE_REG_RD(_4keDqmReg->not_empty_irq_sts)
#define dqmReadNotEmptySts4ke()    FAP_4KE_REG_RD(_4keDqmReg->not_empty_sts)
#define dqmReadLowWtmkIrqMsk4ke()  FAP_4KE_REG_RD(_4keDqmReg->_4ke_low_wtmk_irq_msk)
#define dqmReadLowWtmkIrqSts4ke()  FAP_4KE_REG_RD(_4keDqmReg->low_wtmk_irq_sts)
#define dqmClearNotEmptyIrqSts4ke(__qb) FAP_4KE_REG_WR(_4keDqmReg->not_empty_irq_sts, __qb)
#define dqmClearLowWtmkIrqSts4ke(__qb) FAP_4KE_REG_WR(_4keDqmReg->low_wtmk_irq_sts, __qb)
#define dqmEnableNotEmptyIrqMsk4ke(__qb) FAP_4KE_REG_WR(_4keDqmReg->_4ke_not_empty_irq_msk, dqmReadNotEmptyIrqMsk4ke() | __qb)
#define dqmEnableLowWtmkIrqMsk4ke(__qb) FAP_4KE_REG_WR(_4keDqmReg->_4ke_low_wtmk_irq_msk, dqmReadLowWtmkIrqMsk4ke() | __qb)
#define dqmRecvAvailable4ke(__q) dqmReadNotEmptySts4ke() & (1 << __q)
#define dqmXmitAvailable4ke(__q) FAP_4KE_REG_RD(_4keDqmQCntrlReg->q[__q].sts)

typedef uint32 fap4keDqm_id_t;

/* prototypes */
int  dqmCreateQueue(dqmConfig_s *qCfg);
void dqmInit(void);
int dqmHandlerRegister4ke(uint32 mask, 
                          fap4keTsk_task_t *tsk,
                          fap4keTsk_taskPriority_t priority,
                          fap4keDqm_id_t *pDqmId);
fap4keTsk_task_t *dqmGetTask4ke(fap4keDqm_id_t dqmId);


/******************************************************************************
* Function: dqmRecvMsg4ke                                                     *
*                                                                             *
* Description: get a DQM token - 4ke side                                     *
******************************************************************************/
static inline void dqmRecvMsg4ke(uint32 queue, 
                                 uint32 tokenSize, 
                                 DQMQueueDataReg_S *t)
{  
    uint32 *pw = (uint32 *)(t);
    volatile uint32 *pr = (volatile uint32 *)(&_4keDqmQDataReg->q[queue].word0);
    int i;

    for (i=0; i<tokenSize; ++i)
    {
        *pw++ = *pr++;
    }
}

/******************************************************************************
* Function: dqmXmitMsg4ke                                                     *
*                                                                             *
* Description: send a DQM token - 4ke side                                    *
******************************************************************************/
static inline void dqmXmitMsg4ke(uint32 queue, 
                                 uint32 tokenSize, 
                                 DQMQueueDataReg_S *t)
{  
    volatile uint32 *pw = (volatile uint32 *)(&_4keDqmQDataReg->q[queue].word0);
    uint32 *pr = (uint32 *)(t);
    int i;

    for (i=0; i<tokenSize; ++i)
    {
        *pw++ = *pr++;
    }
}

/******************************************************************************
* Function: _dqmHandlerEnable4ke                                              *
*                                                                             *
* Description: enables or disables a 4ke side DQM handler                     *
*              called by dqmHandlerEnable4ke and dqmHandlerDisable4ke         *
******************************************************************************/
static inline int _dqmHandlerEnable4ke(uint32 mask, uint32 enable)
{
   int i, ret = 0;

   for (i = 0; i < g_4keDqmHandlerCount; i++)
   {
      if (mask == g_4keDqmHandlerInfo[i].mask)
      {
          g_4keDqmHandlerInfo[i].enable = enable;
          ret = 1;
      }
   }

   return ret;
}

#define dqmHandlerEnable4ke(_mask)  _dqmHandlerEnable4ke(_mask, TRUE)
#define dqmHandlerDisable4ke(_mask) _dqmHandlerEnable4ke(_mask, FALSE)

#endif /* __FAP4KEDQM_H_INCLUDED__ */

