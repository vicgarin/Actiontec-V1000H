#ifndef __PKTDMA_HOOKS_H_INCLUDED__
#define __PKTDMA_HOOKS_H_INCLUDED__

/*
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom Corporation
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
 *******************************************************************************
 * File Name  : bcmPktDmaHooks.h
 *
 * Description: This file contains the Packet DMA Host Hook macros and definitions.
 *              It may be included in Kernel space only.
 *
 *******************************************************************************
 */

#include "fap4ke_msg.h"
#include "fap_hw.h"

//#define CC_PKTDMA_HOOKS_DEBUG

/* Hook for: fapDrv_Xmit2Fap */
#define bcmPktDma_xmit2Fap(fapIdx, _msgType, _pMsg)                     \
    ({                                                                  \
        int __ret;                                                      \
        if(bcmPktDma_hostHooks_g.xmit2Fap != NULL) {                    \
            bcmPktDma_hostHooks_g.xmit2Fap(fapIdx, _msgType, _pMsg);    \
            __ret = FAP_SUCCESS;                                        \
        } else {                                                        \
            __ret = FAP_ERROR;                                          \
        }                                                               \
        __ret;                                                          \
    })

/* Hook for: fapDrv_psmAlloc */
/* FAP PSM Memory Allocation added Apr 2010 */
#define bcmPktDma_psmAlloc(fapIdx, _size)                                       \
    ({                                                                  \
        uint8 * __ret;                                                  \
        if(bcmPktDma_hostHooks_g.psmAlloc != NULL)                      \
            __ret = bcmPktDma_hostHooks_g.psmAlloc(fapIdx, _size);      \
        else                                                            \
            __ret = FAP4KE_OUT_OF_PSM;                                  \
        __ret;                                                          \
    })

/* Hook for: dqmXmitMsgHost */
#if defined(CC_PKTDMA_HOOKS_DEBUG)
#define bcmPktDma_dqmXmitMsgHost(fapIdx, _queue, _tokenSize, _t)        \
    ({                                                                  \
        if(bcmPktDma_hostHooks_g.dqmXmitMsgHost != NULL)                \
          bcmPktDma_hostHooks_g.dqmXmitMsgHost(fapIdx, _queue, _tokenSize, _t); \
    })
#else
#define bcmPktDma_dqmXmitMsgHost(fapIdx, _queue, _tokenSize, _t)        \
    bcmPktDma_hostHooks_g.dqmXmitMsgHost(fapIdx, _queue, _tokenSize, _t)
#endif

/* Hook for: dqmRecvMsgHost */
#if defined(CC_PKTDMA_HOOKS_DEBUG)
#define bcmPktDma_dqmRecvMsgHost(fapIdx, _queue, _tokenSize, _t)        \
    ({                                                                  \
        if(bcmPktDma_hostHooks_g.dqmRecvMsgHost != NULL)                \
          bcmPktDma_hostHooks_g.dqmRecvMsgHost(fapIdx, _queue, _tokenSize, _t); \
    })
#else
#define bcmPktDma_dqmRecvMsgHost(fapIdx, _queue, _tokenSize, _t)        \
    bcmPktDma_hostHooks_g.dqmRecvMsgHost(fapIdx, _queue, _tokenSize, _t)
#endif

/* Hook for: dqmXmitAvailableHost */
#if defined(CC_PKTDMA_HOOKS_DEBUG)
#define bcmPktDma_isDqmXmitAvailableHost(fapIdx, _queue)                \
    ({                                                                  \
        int __ret = 0;                                                  \
        if(bcmPktDma_hostHooks_g.isDqmXmitAvailableHost != NULL)        \
            __ret = bcmPktDma_hostHooks_g.isDqmXmitAvailableHost(fapIdx, _queue); \
        __ret;                                                          \
    })
#else
#define bcmPktDma_isDqmXmitAvailableHost(fapIdx, _queue)                \
    bcmPktDma_hostHooks_g.isDqmXmitAvailableHost(fapIdx, _queue)
#endif

/* Hook for: dqmRecvAvailableHost */
#if defined(CC_PKTDMA_HOOKS_DEBUG)
#define bcmPktDma_isDqmRecvAvailableHost(fapIdx, _queue)                \
    ({                                                                  \
        int __ret = 0;                                                  \
        if(bcmPktDma_hostHooks_g.isDqmRecvAvailableHost != NULL)        \
            __ret = bcmPktDma_hostHooks_g.isDqmRecvAvailableHost(fapIdx, _queue); \
        __ret;                                                          \
    })
#else
#define bcmPktDma_isDqmRecvAvailableHost(fapIdx, _queue)                \
    bcmPktDma_hostHooks_g.isDqmRecvAvailableHost(fapIdx, _queue)
#endif

/* Hooks for: dqmHandlerEnableHost, dqmHandlerDisableHost */
#define __bcmPktDma_dqmHandlerEnableHost(_mask, _enable)                \
    ({                                                                  \
        int __ret;                                                      \
        if(bcmPktDma_hostHooks_g.dqmEnableHost != NULL)                 \
            __ret = bcmPktDma_hostHooks_g.dqmEnableHost(_mask, _enable); \
        else                                                            \
            __ret = FAP_ERROR;                                          \
        __ret;                                                          \
    })
#define bcmPktDma_dqmHandlerEnableHost(_mask)  __bcmPktDma_dqmHandlerEnableHost(_mask, TRUE)
#define bcmPktDma_dqmHandlerDisableHost(_mask) __bcmPktDma_dqmHandlerEnableHost(_mask, FALSE)

/* The following are MASKS which describe why the FAP was asked to go to sleep.
   The fap will not wake up unless all triggers are cleared */
#define FAP_SLEEP_TRIGGER_UNPLUGGED_ETH     0x1


/* FAP Driver Hooks */
typedef struct {
    void  (* xmit2Fap)(uint32 fapIdx, fapMsgGroups_t msgType, xmit2FapMsg_t *pMsg);
    uint8 * (* psmAlloc)(uint32 fapIdx, int size);
    void (* dqmXmitMsgHost)(uint32 fapIdx, uint32 queue, uint32 tokenSize, DQMQueueDataReg_S *t);
    void (* dqmRecvMsgHost)(uint32 fapIdx, uint32 queue, uint32 tokenSize, DQMQueueDataReg_S *t);
    int  (* isDqmXmitAvailableHost)(uint32 fapIdx, uint32 queue);
    int  (* isDqmRecvAvailableHost)(uint32 fapIdx, uint32 queue);
    int  (* dqmEnableHost)(uint32 mask, bool enable);
} bcmPktDma_hostHooks_t;

extern bcmPktDma_hostHooks_t bcmPktDma_hostHooks_g;

int bcmPktDma_bind(bcmPktDma_hostHooks_t *hooks);
void bcmPktDma_unbind(void);

#endif  /* defined(__PKTDMA_HOOKS_H_INCLUDED__) */
