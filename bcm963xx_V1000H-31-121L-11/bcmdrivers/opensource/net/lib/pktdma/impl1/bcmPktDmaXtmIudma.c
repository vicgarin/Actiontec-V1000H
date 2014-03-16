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
 * File Name  : bcmPktDmaXtmIudma.c
 *
 * Description: This file contains the Packet DMA Implementation for the iuDMA
 *              channels of the XTM Controller.
 * Note       : This bcmPktDma code is tied to impl1 of the Xtm Driver
 *
 *******************************************************************************
 */

#ifndef CONFIG_BCM96816

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#include <bcm_intr.h>
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#ifdef FAP_4KE
#include "Fap4keOsDeps.h"
#include "fap4ke_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "bcmenet.h"
#include "fap4ke_mailBox.h"
#include "fap4ke_timers.h"
#include "bcmPktDmaHooks.h"
#include "bcmxtmrtimpl.h"
#else /* FAP_4KE */
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap4ke_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap4ke_mailBox.h"
#include "fap4ke_timers.h"
#include "bcmPktDmaHooks.h"
#endif /* FAP_4KE */
#endif

#include "bcmPktDma.h"

/* fap4ke_local redfines memset to the 4ke lib one - not what we want */
#if defined memset
#undef memset
#endif

#ifndef FAP_4KE
/* Binding with XTMRT */
PBCMXTMRT_GLOBAL_INFO g_pXtmGlobalInfo = (PBCMXTMRT_GLOBAL_INFO)NULL;
#endif /* FAP_4KE */

//#ifndef FAP_4KE
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
const int g_Xtm_rx_iudma_ownership[XTM_RX_CHANNELS_MAX] =
{
    PKTDMA_XTM_RX_OWNERSHIP   /* rx iudma channel ownership */
};

const int g_Xtm_tx_iudma_ownership[XTM_TX_CHANNELS_MAX] =
{
    PKTDMA_XTM_TX_OWNERSHIP   /* tx iudma channel ownership */
};
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */
//#endif /* !FAP_4KE */

int bcmPktDma_XtmInitRxChan_Iudma(uint32 bufDescrs,
                                  BcmPktDma_LocalXtmRxDma *pXtmRxDma)
{
    pXtmRxDma->numRxBds = bufDescrs;
    pXtmRxDma->rxAssignedBds = 0;
    pXtmRxDma->rxHeadIndex = 0;
    pXtmRxDma->rxTailIndex = 0;
    pXtmRxDma->xtmrxchannel_isr_enable = 1;
    pXtmRxDma->rxEnabled = 0;

    return 1;
}

int bcmPktDma_XtmInitTxChan_Iudma(uint32 bufDescrs,
                                  BcmPktDma_LocalXtmTxDma *pXtmTxDma,
                                  uint32 dmaType)
{
   //printk("bcmPktDma_XtmInitTxChan_Iudma ch: %ld bufs: %ld txdma: %p\n",
   //        pXtmTxDma->ulDmaIndex, bufDescrs, pXtmTxDma);


   pXtmTxDma->txHeadIndex = 0;
   pXtmTxDma->txTailIndex = 0;
   pXtmTxDma->txEnabled = 0;

   if (dmaType == XTM_SW_DMA)
      pXtmTxDma->txSchedHeadIndex = 0;

    return 1;
}

#ifndef FAP_4KE
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmSelectRxIrq
 Purpose: Return IRQ number to be used for bcmPkt Rx on a specific channel
-------------------------------------------------------------------------- */
int	bcmPktDma_XtmSelectRxIrq_Iudma(int channel)
{
    return (SAR_RX_INT_ID_BASE + channel);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecvRingSize
 Purpose: Receive ring size (Currently queued Rx Buffers) of the associated DMA desc.
-------------------------------------------------------------------------- */
uint32 bcmPktDma_XtmRecv_RingSize_Iudma(BcmPktDma_LocalXtmRxDma * rxdma)
{
    int ringSize ;

    if (rxdma->rxTailIndex < rxdma->rxHeadIndex) {
       ringSize = rxdma->numRxBds - rxdma->rxHeadIndex ;
       ringSize += rxdma->rxTailIndex ;
    }
    else {
       ringSize = rxdma->rxTailIndex - rxdma->rxHeadIndex ;
    }

    return (ringSize) ;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecv
 Purpose: Receive a packet on a specific channel,
          returning the associated DMA desc
-------------------------------------------------------------------------- */
uint32 bcmPktDma_XtmRecv_Iudma(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char **pBuf, int * pLen)
{
    DmaDesc                  dmaDesc;

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

    dmaDesc.word0 = 0;

    if (rxdma->rxAssignedBds != 0)
    {
        /* Get the status from Rx BD */
        dmaDesc.word0 = rxdma->rxBds[rxdma->rxHeadIndex].word0;

        /* If no more rx packets, we are done for this channel */
        if ((dmaDesc.status & DMA_OWN) == 0)
        {
            *pBuf = (unsigned char *)
                   (phys_to_virt(rxdma->rxBds[rxdma->rxHeadIndex].address));
            *pLen = (int) dmaDesc.length;

            /* Wrap around the rxHeadIndex */
            if (++rxdma->rxHeadIndex == rxdma->numRxBds)
            {
                rxdma->rxHeadIndex = 0;
            }
            rxdma->rxAssignedBds--;
        }
    }
    else   /* out of buffers! */
       return (uint32)0xFFFF;

    //printk("XtmRecv_Iudma end ch: %d head: %d tail: %d assigned: %d\n", rxdma->channel, rxdma->rxHeadIndex, rxdma->rxTailIndex, rxdma->rxAssignedBds);

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);

    return dmaDesc.word0;
}
#endif /* FAP_4KE */
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmXmitAvailable
 Purpose: Determine if there are free resources for the xmit
   Notes: channel in XTM mode refers to a specific TXQINFO struct of a
          specific XTM Context
-------------------------------------------------------------------------- */
int bcmPktDma_XtmXmitAvailable_Iudma(BcmPktDma_LocalXtmTxDma *txdma, uint32 dmaType)
{
    if (txdma->txFreeBds != 0)  return 1;

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmTxEnable_Iudma
 Purpose: Coordinate with FAP for tx enable
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_XtmTxEnable_Iudma( BcmPktDma_XtmTxDma * txdma, PDEV_PARAMS unused, uint32 dmaType )
{
    //printk("bcmPktDma_XtmTxEnable_Iudma ch: %d\n", txdma->ulDmaIndex);

    txdma->txEnabled = 1;

    /* The other SW entity which reads from this DMA in case of SW_DMA,
     * will always look at this bit to start processing anything from
     * the DMA queue.
     */
    if (dmaType == XTM_SW_DMA)
       txdma->txDma->cfg = DMA_ENABLE ;
    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmTxDisable_Iudma
 Purpose: Coordinate with FAP for tx disable
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_XtmTxDisable_Iudma( BcmPktDma_LocalXtmTxDma * txdma, uint32 dmaType, void (*func) (uint32 param1,
         BcmPktDma_XtmTxDma *txswdma), uint32 param1)
{
    int j;

    txdma->txEnabled = 0;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Request the iuDMA to disable at the end of the next tx pkt - Jan 2011 */
    txdma->txDma->cfg = DMA_PKT_HALT;
#endif

    /* Changing txEnabled to 0 prevents any more packets
     * from being queued on a transmit DMA channel.  Allow all currenlty
     * queued transmit packets to be transmitted before disabling the DMA.
     */

    if (dmaType == XTM_HW_DMA) {

    for (j = 0; j < 2000 && (txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE; j++)
    {
#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
        udelay(500);
#else
        {
            /* Increase wait from .1 sec to .5 sec to handle longer XTM packets - May 2010 */
            uint32 prevJiffies = fap4keTmr_jiffies + (FAPTMR_HZ) / 2; /* .5 sec */

            while(!fap4keTmr_isTimeAfter(fap4keTmr_jiffies, prevJiffies));

            if((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE)
            {
                return 0;    /* return so caller can handle the failure */
            }
        }
#endif
    }

    if ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE)
    {
        /* This should not happen. */
        txdma->txDma->cfg = DMA_PKT_HALT;
#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
        udelay(500);
#else
        /* This will not happen in the FAP/FAP_MODULE case */
#endif
        txdma->txDma->cfg = 0;
        if ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE)
            return 0;    /* return so caller can handle the failure */
    }
    } /* if DMA is HW Type */
    else {

       /* No blocking wait for SW DMAs */
       (*func)(param1, txdma) ;
    }

    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRxEnable
    Purpose: Enable rx DMA for the given channel.
    Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_XtmRxEnable_Iudma( BcmPktDma_LocalXtmRxDma * rxdma )
{

    //printk("bcmPktDma_XtmRxEnable_Iudma channel: %d\n", rxdma->channel);

    rxdma->rxDma->cfg |= DMA_ENABLE;
    rxdma->rxEnabled = 1;

    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRxDisable
 Purpose: Disable rx interrupts for the given channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_XtmRxDisable_Iudma( BcmPktDma_LocalXtmRxDma * rxdma )
{
    int                       i;

    //printk("bcmPktDma_XtmRxDisable_Iudma channel: %d\n", rxdma->channel);

    rxdma->rxEnabled = 0;

    rxdma->rxDma->cfg &= ~DMA_ENABLE;
    for (i = 0; rxdma->rxDma->cfg & DMA_ENABLE; i++)
    {
        rxdma->rxDma->cfg &= ~DMA_ENABLE;

        if (i >= 100)
        {
            //printk("Failed to disable RX DMA?\n");
            return 0;
        }

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        /* The delay only works when called from Linux */
        udelay(20);
#else
        {
            uint32 prevJiffies = fap4keTmr_jiffies;

            while(!fap4keTmr_isTimeAfter(fap4keTmr_jiffies, prevJiffies));

            if((rxdma->rxDma->cfg & DMA_ENABLE) == DMA_ENABLE)
            {
                return 0;    /* return so caller can handle the failure */
            }
        }
#endif
    }

    return 1;
}


/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmForceFreeXmitBufGet
 Purpose: Gets a TX buffer to free by caller, ignoring DMA_OWN status
-------------------------------------------------------------------------- */
BOOL bcmPktDma_XtmForceFreeXmitBufGet_Iudma(BcmPktDma_LocalXtmTxDma * txdma, uint32 *pKey,
                                            uint32 *pTxSource, uint32 *pTxAddr,
                                            uint32 *pRxChannel, uint32 dmaType,
                                            uint32 noGlobalBufAccount)
{
    BOOL ret = FALSE;
    int  bdIndex;

    bdIndex = txdma->txHeadIndex;
    *pKey = 0;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* TxSource & TxAddr not required in non-FAP applications */
    *pTxSource = 0;
    *pTxAddr   = 0;
    *pRxChannel = 0;
#endif

    /* Reclaim transmitted buffers */
    if (txdma->txFreeBds < txdma->ulQueueSize)
    {
        {
           *pKey = txdma->txRecycle[bdIndex].key;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
           *pTxSource = txdma->txRecycle[bdIndex].source;
           *pTxAddr = txdma->txRecycle[bdIndex].address;
           *pRxChannel = txdma->txRecycle[bdIndex].rxChannel;
#endif

           if (++txdma->txHeadIndex == txdma->ulQueueSize)
               txdma->txHeadIndex = 0;

           txdma->txFreeBds++;
           txdma->ulNumTxBufsQdOne--;
#if !defined(CONFIG_BCM96816) && !defined(CONFIG_BCM96362)  && !defined(CONFIG_BCM963268)
           // FIXME - Which chip uses more then one TX queue?
           if (!noGlobalBufAccount)
           g_pXtmGlobalInfo->ulNumTxBufsQdAll--;
#endif

           ret = TRUE;
        }
    }

    return ret;
}

#ifndef FAP_4KE

/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
DmaDesc *bcmPktDma_XtmAllocTxBds(int channel, int numBds)
{
    /* Allocate space for pKeyPtr, pTxSource and pTxAddress as well as BDs. */
    int size = sizeof(DmaDesc) + sizeof(BcmPktDma_txRecycle_t);

#if defined(XTM_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
{
    uint8 * pMem;
    static uint8 * txBdAllocation[MAX_TRANSMIT_QUEUES] = {NULL};
    static int txNumBds[MAX_TRANSMIT_QUEUES] = {0};
    uint32 fapIdx;

    /* channel is iudma in this instance */
    fapIdx = getFapIdxFromXtmTxIudma(channel);
    if (!isValidFapIdx(fapIdx))
    {
        printk("ERROR: bcmPktDma_XtmAllocTxBds: Tried to allocate using bad fapIdx (%d / %ld)\n",
                channel, fapIdx);
        return (NULL);
    }

    /* Restore previous BD allocation pointer if any */
    pMem = txBdAllocation[channel];

    if (pMem)
    {
        if(txNumBds[channel] != numBds)
        {
            printk("ERROR: Tried to allocate a different number of txBDs (was %d, attempted %d)\n",
                    txNumBds[channel], numBds);
            printk("       Xtm tx BD allocation rejected!!\n");
            return( NULL );
}
        memset(pMem, 0, numBds * size);
        return((DmaDesc *)pMem);   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
    }

    /* Try to allocate Tx Descriptors in PSM. Use Host-side addressing here. */
    /* fapDrv_psmAlloc guarantees 8 byte alignment. */
    pMem = bcmPktDma_psmAlloc(fapIdx, numBds * size);
    if(pMem != FAP4KE_OUT_OF_PSM)
    {
        memset(pMem, 0, numBds * size);
        txBdAllocation[channel] = pMem;
        txNumBds[channel] = numBds;
        return((DmaDesc *)pMem);   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
}

    printk("ERROR: Out of PSM. Xtm tx BD allocation rejected!!\n");
    return(NULL);
}
#else  /* !defined(XTM_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
{
    void * p;

    /* Allocate Tx Descriptors in DDR */
    /* Leave room for alignment by caller - Apr 2010 */
    p = kmalloc(numBds * size + 0x10, GFP_ATOMIC) ;
    if (p !=NULL) {
        memset(p, 0, numBds * size + 0x10);
        cache_flush_len(p, numBds * size + 0x10);
    }
    return( (DmaDesc *)p );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
}
#endif   /* defined(XTM_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
}

/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
DmaDesc *bcmPktDma_XtmAllocRxBds(int channel, int numBds)
{
#if defined(XTM_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
{
    uint8 * pMem;
    static uint8 * rxBdAllocation[MAX_RECEIVE_QUEUES] = {NULL};
    static int rxNumBds[MAX_RECEIVE_QUEUES] = {0};
    uint32 fapIdx;

    /* Restore previous BD allocation pointer if any */
    pMem = rxBdAllocation[channel];
    fapIdx = getFapIdxFromXtmRxIudma(channel);
    if (!isValidFapIdx(fapIdx))
    {
        printk("ERROR: bcmPktDma_XtmAllocRxBds: Invalid Fap Index (channel=%d, fapIdx=%ld)\n", channel, fapIdx);
    }

    if (pMem)
    {
        if(rxNumBds[channel] != numBds)
        {
            printk("ERROR: Tried to allocate a different number of rxBDs (was %d, attempted %d)\n",
                    rxNumBds[channel], numBds);
            printk("       Xtm rx BD allocation rejected!!\n");
            return( NULL );
        }
        memset(pMem, 0, numBds * sizeof(DmaDesc));
        return((DmaDesc *)pMem);   /* rx bd ring */
    }

    /* Try to allocate Rx Descriptors in PSM. Use Host-side addressing here. */
    /* fapDrv_psmAlloc guarantees 8 byte alignment. */
    pMem = bcmPktDma_psmAlloc(fapIdx, numBds * sizeof(DmaDesc));
    if(pMem != FAP4KE_OUT_OF_PSM)
    {
        memset(pMem, 0, numBds * sizeof(DmaDesc));
        rxBdAllocation[channel] = pMem;
        rxNumBds[channel] = numBds;
        return((DmaDesc *)pMem);   /* rx bd ring */
    }

    printk("ERROR: Out of PSM. Xtm rx BD allocation rejected!!\n");
    return( NULL );
}
#else   /* !defined(XTM_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
{
    void * p;

    /* Allocate Rx Descriptors in DDR */
    /* Leave room for alignment by caller - Apr 2010 */
    p = kmalloc(numBds * sizeof(DmaDesc) + 0x10, GFP_ATOMIC) ;
    if (p != NULL) {
        memset(p, 0, numBds * sizeof(DmaDesc) + 0x10);
        cache_flush_len(p, numBds * sizeof(DmaDesc) + 0x10);
    }
    return((DmaDesc *)p);   /* rx bd ring */
}
#endif   /* defined(XTM_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
}

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int bcmPktDma_XtmSetIqThresh_Iudma( BcmPktDma_LocalXtmRxDma * rxdma,
                                    uint16 loThresh,
                                    uint16 hiThresh)
{
    rxdma->iqLoThresh = loThresh;
    rxdma->iqHiThresh = hiThresh;
    rxdma->iqDropped  = 0;
    return 1;
}
#endif

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int bcmPktDma_XtmSetRxChanBpmThresh_Iudma( BcmPktDma_LocalXtmRxDma * rxdma,
                                           uint16 allocTrig,
                                           uint16 bulkAlloc )
{
   rxdma->allocTrig = allocTrig;
   rxdma->bulkAlloc = bulkAlloc;

    return 1;
}


int bcmPktDma_XtmSetTxChanBpmThresh_Iudma( BcmPktDma_LocalXtmTxDma * txdma,
                                           uint16 loThresh,
                                           uint16 hiThresh,
                                           uint32 dmaType)
{
   txdma->ulLoThresh = loThresh;
   txdma->ulHiThresh = hiThresh;
   txdma->ulDropped  = 0;

    return 1;
}
#endif


EXPORT_SYMBOL(g_pXtmGlobalInfo);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
EXPORT_SYMBOL(g_Xtm_rx_iudma_ownership);
EXPORT_SYMBOL(g_Xtm_tx_iudma_ownership);
#endif

EXPORT_SYMBOL(bcmPktDma_XtmInitRxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmInitTxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmSelectRxIrq_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmClrRxIrq_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRecv_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmXmitAvailable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmXmit_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmTxEnable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmTxDisable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRxEnable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRxDisable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmForceFreeXmitBufGet_Iudma);

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
EXPORT_SYMBOL(bcmPktDma_XtmSetIqThresh_Iudma);
#endif
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
EXPORT_SYMBOL(bcmPktDma_XtmSetRxChanBpmThresh_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmSetTxChanBpmThresh_Iudma);
#endif

#endif /* FAP_4KE */

#endif  /* #ifndef CONFIG_BCM96816 */
