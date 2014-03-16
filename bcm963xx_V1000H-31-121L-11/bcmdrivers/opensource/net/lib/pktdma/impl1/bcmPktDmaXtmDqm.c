/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom Corporation
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
 * File Name  : bcmPktDmaXtmDqm.c
 *
 * Description: This file contains the Packet DMA Implementation for the
 *              Forward Assist Processor (FAP) Dynamic Queues for use by the
 *              XTM Controller.
 *
 *******************************************************************************
 */

#ifndef CONFIG_BCM96816

#include <bcm_intr.h>
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap4ke_local.h"
#endif
#include "bcmPktDma.h"
#include "bcmPktDmaHooks.h"

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap4ke_dqm.h"
#include "fap_dqmHost.h"
#include "fap4ke_msg.h"
#include "fap4ke_irq.h"

#endif

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmSelectRxIrq
    Purpose: Return IRQ number to be used for bcmPkt Rx on a specific channel
    Notes: Interrupt ID returned is 0.
          Handling of INTERRUPT_ID_FAP is installed in fapDriver instead
-------------------------------------------------------------------------- */
int	bcmPktDma_XtmSelectRxIrq_Dqm(int channel)
{
    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmClrRxIrq
 Purpose: Clear the Rx interrupt for a specific channel
-------------------------------------------------------------------------- */
void    bcmPktDma_XtmClrRxIrq_Dqm(BcmPktDma_LocalXtmRxDma * rxdma)
{
    uint32 qbit = 1 << (DQM_FAP2HOST_XTM0_RX_Q + rxdma->channel);

    //printk("\nbcmPktDma_XtmClrRxIrq_Dqm\n");

    dqmClearNotEmptyIrqStsHost(rxdma->fapIdx, qbit);
    dqmEnableNotEmptyIrqMskHost(rxdma->fapIdx, qbit);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecv_RingSize
 Purpose: Return 0, not used.
   Notes: None
-------------------------------------------------------------------------- */
uint32 bcmPktDma_XtmRecv_RingSize_Dqm(BcmPktDma_LocalXtmRxDma * rxdma)
{
    return 0 ;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecv
 Purpose: Receive a packet on a channel
   Notes: In DQM msg, word0 = pBuf; word1 = dmaDesc.word0
-------------------------------------------------------------------------- */
uint32 bcmPktDma_XtmRecv_Dqm(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char **pBuf, int * pLen)
{
    uint32            dqm = DQM_FAP2HOST_XTM0_RX_Q + rxdma->channel;
    fapDqm_XtmRx_t    rx;
    DmaDesc           dmaDesc;

    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel: %d", rxdma->channel);

    dmaDesc.word0 = 0;

    if (dqmRecvAvailableHost(rxdma->fapIdx, dqm))
    {
        dqmRecvMsgHost(rxdma->fapIdx, dqm, DQM_FAP2HOST_XTM_RX_Q_SIZE,
                                      (DQMQueueDataReg_S *) &rx);

        *pBuf         = (unsigned char *) rx.pBuf;
        dmaDesc.word0 = rx.dmaWord0;
        *pLen         = dmaDesc.length;

        //printk("XtmRecv_Dqm pbuf: 0x%08lX len: %d dmaDesc.word0: 0x%08lX\n",
        //          (long unsigned int)*pBuf, *pLen, dmaDesc.word0);
    }
    else
    {
        *pBuf   = (unsigned char *) NULL;
        *pLen   = 0;
        dmaDesc.status = DMA_OWN;
    }

    return dmaDesc.word0;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmFreeRecvBuf
 Purpose: Free a single RX buffer
   Notes: In DQM msg, word0 = channel; word1 = pBuf
-------------------------------------------------------------------------- */
void bcmPktDma_XtmFreeRecvBuf_Dqm(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char * pBuf)
{
    DQMQueueDataReg_S msg;

    while(!dqmXmitAvailableHost(rxdma->fapIdx, DQM_HOST2FAP_XTM_FREE_RXBUF_Q));

    msg.word0 = (uint32) rxdma->channel;
    msg.word1 = (uint32) pBuf;
    (void) bcmPktDma_dqmXmitMsgHost(rxdma->fapIdx, DQM_HOST2FAP_XTM_FREE_RXBUF_Q,
                                    DQM_HOST2FAP_XTM_FREE_RXBUF_Q_SIZE, &msg);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmFreeXmitBufGet_Dqm
 Purpose: Free all possible TX buffers once transmission is done
   Notes: Params pTxSource and pTxAddr not used in DQM implementation
-------------------------------------------------------------------------- */
BOOL bcmPktDma_XtmFreeXmitBufGet_Dqm(BcmPktDma_LocalXtmTxDma *txdma, uint32 *pKey, uint32 *pTxSource,
                                     uint32 *pTxAddr, uint32 *rxChannel,
                                     uint32 dmaType, uint32 noGlobalBufAccount)
{
    BOOL ret = FALSE;
    DQMQueueDataReg_S msg;

    //BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "channel: %d", txdma->ulDmaIndex);

    /* Reclaim transmitted buffers for any queue */
    if (dqmRecvAvailableHost(txdma->fapIdx, DQM_FAP2HOST_XTM_FREE_TXBUF_Q))
    {
        /* Channel and index for bcmPktDma_EthFreeXmitBuf don't matter */
        /* Just take skb ptrs off the DQM queue til done */
        dqmRecvMsgHost(txdma->fapIdx, DQM_FAP2HOST_XTM_FREE_TXBUF_Q,
                          DQM_FAP2HOST_XTM_FREE_TXBUF_Q_SIZE, &msg);

         *pKey     = (uint32) msg.word0;
         ret = TRUE;
    }

    return ret;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmXmitAvailable
 Purpose: Determine if there are free resources for the xmit
-------------------------------------------------------------------------- */
int bcmPktDma_XtmXmitAvailable_Dqm(BcmPktDma_LocalXtmTxDma *txdma, uint32 dmaType)
{
    return (dqmXmitAvailableHost(txdma->fapIdx, DQM_HOST2FAP_XTM_XMIT_Q));
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmXmit
 Purpose: Xmit an skb
          param1, param2, param3 N/A
          In DQM msg, word0 = skb; word1 = channel & len
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_XtmXmit_Dqm(BcmPktDma_LocalXtmTxDma *txdma, uint8 *pBuf, uint16 len, int bufSource,
                          uint16 dmaStatus, uint32 key,int param1,
                          uint32 dmaType, uint32 noGlobalBufAccount)
{
    fapDqm_XtmTx_t tx;

    //printk("bcmPktDma_XtmXmit_Dqm ch: %d pBuf: %lx len: %d key: %lx dma: %x\n",
    //                txdma->ulDmaIndex, (uint32)pBuf, len, key, dmaStatus);

    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel %d", txdma->ulDmaIndex);

    /* If there is space in the ETH_XMIT_Q, send the xmit request to the FAP */
    if (dqmXmitAvailableHost(txdma->fapIdx, DQM_HOST2FAP_XTM_XMIT_Q))
    {
        tx.pBuf = pBuf;
        tx.source = bufSource;
        tx.channel = txdma->ulDmaIndex;
        tx.len = len;
        tx.key = key;
        tx.dmaStatus = dmaStatus;
        tx.param1 = param1;

        bcmPktDma_dqmXmitMsgHost(txdma->fapIdx, DQM_HOST2FAP_XTM_XMIT_Q,
                                 DQM_HOST2FAP_XTM_XMIT_Q_SIZE,
                                (DQMQueueDataReg_S *) &tx);

        return 1;
    }

/*     else  */
/*         BCM_LOG_ERROR(BCM_LOG_ID_FAP,  */
/*                   "bcmPktDma_EthXmit_Dqm to ETH_XMIT_Q FAILED! (drop)\n"); */

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmTxEnable_Dqm
 Purpose: Coordinate with FAP to enable tx channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_XtmTxEnable_Dqm( BcmPktDma_XtmTxDma * txdma, PDEV_PARAMS pDevParams, uint32 dmaTypeUnUsed )
{
    xmit2FapMsg_t fapMsg;
    int           retVal;

    //printk("bcmPktDma_XtmTxEnable_Dqm ch: %d\n", txdma->ulDmaIndex);

    fapMsg.drvCtl.cmd     = FAPMSG_CMD_TX_ENABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_XTM;
    fapMsg.drvCtl.channel = txdma->ulDmaIndex;

    /* Params added for xtmrt dmaStatus field generation for xtm flows - Apr 2010 */
    /* Convert pDevParams to a DDR address usable by FAP */
    cache_flush_len(pDevParams, sizeof(DEV_PARAMS));
    fapMsg.drvCtl.params  = (uint32)CACHE_TO_NONCACHE(pDevParams);

    retVal = bcmPktDma_xmit2Fap(txdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg);

    /* Wait a while for FAP receive */
    udelay(500);

    return( retVal );
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmTxDisable_Dqm
 Purpose: Coordinate with FAP to disable tx channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int  bcmPktDma_XtmTxDisable_Dqm(BcmPktDma_LocalXtmTxDma * txdma, uint32 dmaTypeUnused, void (*func)(uint32 param1,
            BcmPktDma_XtmTxDma *txdma), uint32 param1)
{
    xmit2FapMsg_t fapMsg;
    int           retVal;

    //printk("bcmPktDma_XtmTxDisable_Dqm ch: %d\n", txdma->ulDmaIndex);

    fapMsg.drvCtl.cmd     = FAPMSG_CMD_TX_DISABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_XTM;
    fapMsg.drvCtl.channel = txdma->ulDmaIndex;

    retVal = bcmPktDma_xmit2Fap(txdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg);

    /* Wait a while for FAP receive */
    udelay(500);

    return( retVal );

}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRxEnable_Dqm
 Purpose: Coordinate with FAP to enable rx
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_XtmRxEnable_Dqm( BcmPktDma_LocalXtmRxDma * rxdma )
{
    xmit2FapMsg_t fapMsg;
    int retVal;

    //printk("bcmPktDma_XtmRxEnable_Dqm ch: %d\n", rxdma->channel);

    fapMsg.drvCtl.cmd     = FAPMSG_CMD_RX_ENABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_XTM;
    fapMsg.drvCtl.channel = rxdma->channel;

    retVal = bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg);

    /* Wait a while for FAP receive */
    udelay(500);

    return retVal;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRxDisable_Dqm
 Purpose: Coordinate with FAP to disable rx
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_XtmRxDisable_Dqm( BcmPktDma_LocalXtmRxDma * rxdma )
{
    xmit2FapMsg_t fapMsg;
    int           retVal;

    //printk("bcmPktDma_XtmRxDisable_Dqm ch: %d\n", rxdma->channel);

    /* Disable rx pkt processing */
    fapMsg.drvCtl.cmd     = FAPMSG_CMD_RX_DISABLE;
    fapMsg.drvCtl.drv     = FAPMSG_DRV_XTM;
    fapMsg.drvCtl.channel = rxdma->channel;

    retVal = bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_DRV_CTL, &fapMsg);

    /* Wait a while for FAP receive */
    udelay(500);

    return( retVal );
}

int	bcmPktDma_XtmInitRxChan_Dqm( uint32 bufDescrs,
                                 BcmPktDma_LocalXtmRxDma *pXtmRxDma)
{
    xmit2FapMsg_t fapMsg;

    fapMsg.drvInit.cmd     = FAPMSG_CMD_INIT_RX;
    fapMsg.drvInit.channel = pXtmRxDma->channel;
    fapMsg.drvInit.drv     = FAPMSG_DRV_XTM;
    fapMsg.drvInit.numBds =  bufDescrs;

#if defined(XTM_RX_BDS_IN_PSM)
    /* If BDs in PSM, convert rxBds to FAP-based address - Apr 2010 */
    fapMsg.drvInit.Bds = (uint32)CONVERT_PSM_HOST2FAP(pXtmRxDma->rxBds);
#else
    /* Remap to uncached kseg */
    fapMsg.drvInit.Bds = KSEG1ADDR(pXtmRxDma->rxBds);
#endif

    /* Dma Ctrl registers require bit 29 to be set as well to read properly */
    fapMsg.drvInit.Dma = (uint32)(VIRT_TO_PHY(pXtmRxDma->rxDma)|0xA0000000);

    bcmPktDma_xmit2Fap(pXtmRxDma->fapIdx, FAP_MSG_DRV_XTM_INIT, &fapMsg);

    /* Wait a while for FAP receive */
    udelay(500);

    return 1;

}

int	bcmPktDma_XtmInitTxChan_Dqm( uint32 bufDescrs,
                                 BcmPktDma_LocalXtmTxDma *pXtmTxDma,
                                 uint32 dmaType)
{
    xmit2FapMsg_t fapMsg;

    fapMsg.drvInit.cmd     = FAPMSG_CMD_INIT_TX;
    fapMsg.drvInit.channel = pXtmTxDma->ulDmaIndex;
    fapMsg.drvInit.drv     = FAPMSG_DRV_XTM;
    fapMsg.drvInit.numBds =  bufDescrs;

#if defined(XTM_TX_BDS_IN_PSM)
    /* If BDs in PSM, convert txBds to FAP-based address - Apr 2010 */
    fapMsg.drvInit.Bds = (uint32)CONVERT_PSM_HOST2FAP(pXtmTxDma->txBds);
#else
    /* Remap to uncached kseg */
    fapMsg.drvInit.Bds = KSEG1ADDR(pXtmTxDma->txBds);
#endif

    /* Dma Ctrl registers require bit 29 to be set as well to read properly */
    fapMsg.drvInit.Dma = (uint32)(VIRT_TO_PHY(pXtmTxDma->txDma)|0xA0000000);

    bcmPktDma_xmit2Fap(pXtmTxDma->fapIdx, FAP_MSG_DRV_XTM_INIT, &fapMsg);

    /* Wait a while for FAP receive */
    udelay(500);

    return 1;

}

int bcmPktDma_XtmCreateDevice_Dqm(uint32 devId, uint32 encapType, uint32 headerLen, uint32 trailerLen)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;


    fapMsg.xtmCreateDevice.devId = devId;
    fapMsg.xtmCreateDevice.encapType = encapType;
    fapMsg.xtmCreateDevice.headerLen = headerLen;
    fapMsg.xtmCreateDevice.trailerLen = trailerLen;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        bcmPktDma_xmit2Fap(fapIdx, FAP_MSG_DRV_XTM_CREATE_DEVICE, &fapMsg);
    }

    /* Wait a while for FAP receive */
    udelay(500);

    return 1;
}

int bcmPktDma_XtmLinkUp_Dqm(uint32 devId, uint32 matchId)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    fapMsg.xtmLinkUp.devId = devId;
    fapMsg.xtmLinkUp.matchId = matchId;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        bcmPktDma_xmit2Fap(fapIdx, FAP_MSG_DRV_XTM_LINK_UP, &fapMsg);
    }

    /* Wait a while for FAP receive */
    udelay(500);

    return 1;
}

void bcm63xx_xtm_dqmhandler(unsigned long channel)
{
    int i;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;

    // BCM_LOG_INFO(BCM_LOG_ID_FAP, "channel %ld", channel);

    for (i = 0; i < MAX_DEV_CTXS; i++)
    {
        if ((pDevCtx = g_pXtmGlobalInfo->pDevCtxs[i]) != NULL &&
             pDevCtx->ulOpenState == XTMRT_DEV_OPENED)
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
            napi_schedule(&pDevCtx->napi);
#else
            netif_rx_schedule(pDevCtx->pDev);
#endif

            g_pXtmGlobalInfo->ulIntEnableMask |= 1 << channel;
        }
    }
}


#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int	bcmPktDma_XtmSetIqThresh_Dqm( BcmPktDma_LocalXtmRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh)
{
    xmit2FapMsg_t fapMsg;

    fapMsg.threshInit.cmd     = FAPMSG_CMD_SET_IQ_THRESH;
    fapMsg.threshInit.channel = rxdma->channel;
    fapMsg.threshInit.drv     = FAPMSG_DRV_XTM;
    fapMsg.threshInit.loThresh= loThresh;
    fapMsg.threshInit.hiThresh= hiThresh;

    bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_IQ, &fapMsg);

    return 1;
}

int	bcmPktDma_XtmSetIqDqmThresh_Dqm( BcmPktDma_LocalXtmRxDma * rxdma,
                                  uint16 loThresh,
                                  uint16 hiThresh)
{
    xmit2FapMsg_t fapMsg;

    fapMsg.threshInit.cmd     = FAPMSG_CMD_SET_IQ_DQM_THRESH;
    fapMsg.threshInit.channel = rxdma->channel;
    fapMsg.threshInit.drv     = FAPMSG_DRV_XTM;
    fapMsg.threshInit.loThresh= loThresh;
    fapMsg.threshInit.hiThresh= hiThresh;

    bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_IQ, &fapMsg);

    return 1;
}
#endif

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int	bcmPktDma_XtmSetRxChanBpmThresh_Dqm( BcmPktDma_LocalXtmRxDma * rxdma,
                                         uint16 allocTrig,
                                         uint16 bulkAlloc )
{
    xmit2FapMsg_t fapMsg;

    fapMsg.rxThresh.cmd     = FAPMSG_CMD_SET_RX_BPM_THRESH;
    fapMsg.rxThresh.channel = rxdma->channel;
    fapMsg.rxThresh.drv     = FAPMSG_DRV_XTM;
    fapMsg.rxThresh.allocTrig = allocTrig;
    fapMsg.rxThresh.bulkAlloc = bulkAlloc;

    bcmPktDma_xmit2Fap(rxdma->fapIdx, FAP_MSG_BPM, &fapMsg);

    return 1;
}


int	bcmPktDma_XtmSetTxChanBpmThresh_Dqm( BcmPktDma_LocalXtmTxDma * txdma,
                                         uint16 loThresh,
                                         uint16 hiThresh,
                                         uint32 dmaType)
{
    xmit2FapMsg_t fapMsg;

    fapMsg.threshInit.cmd     = FAPMSG_CMD_SET_TXQ_BPM_THRESH;
    fapMsg.threshInit.channel = txdma->ulDmaIndex;
    fapMsg.threshInit.drv     = FAPMSG_DRV_XTM;
    fapMsg.threshInit.loThresh= loThresh;
    fapMsg.threshInit.hiThresh= hiThresh;

    bcmPktDma_xmit2Fap(txdma->fapIdx, FAP_MSG_BPM, &fapMsg);

    return 1;
}
#endif

EXPORT_SYMBOL(bcm63xx_xtm_dqmhandler);

EXPORT_SYMBOL(bcmPktDma_XtmSelectRxIrq_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmClrRxIrq_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmRecv_RingSize_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmRecv_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmFreeRecvBuf_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmXmitAvailable_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmXmit_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmCreateDevice_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmLinkUp_Dqm);
#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
EXPORT_SYMBOL(bcmPktDma_XtmSetIqThresh_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmSetIqDqmThresh_Dqm);
#endif
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
EXPORT_SYMBOL(bcmPktDma_XtmSetRxChanBpmThresh_Dqm);
EXPORT_SYMBOL(bcmPktDma_XtmSetTxChanBpmThresh_Dqm);
#endif

#endif   /* #ifndef CONFIG_BCM96816 */

