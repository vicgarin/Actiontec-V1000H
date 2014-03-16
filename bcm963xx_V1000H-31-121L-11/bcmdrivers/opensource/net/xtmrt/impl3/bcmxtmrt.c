/*
<:label-BRCM:2011:DUAL/GPL:standard

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
/**************************************************************************
 * File Name  : bcmxtmrt.c
 *
 * Description: This file implements BCM6368 ATM/PTM network device driver
 *              runtime processing - sending and receiving data.
 ***************************************************************************/

/* Defines. */
#define VERSION     "0.3"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__


/* Includes. */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/blog.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmnet.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"
#include "bcmPktDma.h"
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#include "fap_task.h"
#include "fap_dqm.h"
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include "ingqos.h"
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include <linux/gbpm.h>
#include "bpm.h"
#endif

static UINT32 gs_ulLogPorts[]  = {0, 3, 1, 2};

static UINT32 swSchedTimerUsecs = 0 ;

#define SW_SCHED_TIMER          TIMER1
#define SW_SCHED_TIMEREN        TIMER1EN
#define SW_SCHED_TIMERCTL       TIMER->TimerCtl1

#define PORT_PHYS_TO_LOG(PP) gs_ulLogPorts[PP]

/* Externs. */
extern unsigned long getMemorySize(void);
extern int kerSysGetMacAddress(UINT8 *pucaMacAddr, UINT32 ulId);

/* 32bit context is union of pointer to pdevCtrl and channel number */
#if (ENET_RX_CHANNELS_MAX > 4)
#error "Overlaying channel and pDevCtrl into context param needs rework"
#else
#define BUILD_CONTEXT(pGi,channel) \
            (uint32)((uint32)(pGi) | ((uint32)(channel) & 0x3u))
#define CONTEXT_TO_CHANNEL(context)  (int)((context) & 0x3u)
#endif

/* 32-bit recycle context definition */
typedef union {
    struct {
        /* fapQuickFree handling removed - Oct 2010 */
        uint32 reserved     : 30;
        uint32 channel      :  2;
    };
    uint32 u32;
} xtm_recycle_context_t;

#define RECYCLE_CONTEXT(_context)  ( (xtm_recycle_context_t *)(&(_context)) )
#define FKB_RECYCLE_CONTEXT(_pFkb) RECYCLE_CONTEXT((_pFkb)->recycle_context)

/* Prototypes. */
int __init bcmxtmrt_init( void );
static void bcmxtmrt_cleanup( void );
static int bcmxtmrt_open( struct net_device *dev );
       int bcmxtmrt_close( struct net_device *dev );
static void bcmxtmrt_timeout( struct net_device *dev );
static struct net_device_stats *bcmxtmrt_query(struct net_device *dev);
static int bcmxtmrt_change_mtu(struct net_device *dev, int new_mtu);
static void bcmxtmrt_clrStats(struct net_device *dev);
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd);
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx,void *useraddr);
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd,
    unsigned long arg);
static PBCMXTMRT_DEV_CONTEXT FindDevCtx( short vpi, int vci );
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc);
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc);
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb);
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan,struct sk_buff *skb);
static int bcmxtmrt_xmit( pNBuff_t pNBuff, struct net_device *dev);
static void AddRfc2684Hdr(pNBuff_t *ppNBuff, struct sk_buff **ppNbuffSkb,
    UINT8 **ppData, int * pLen, UINT32 ulHdrType);
static void bcmxtmrt_recycle_skb_or_data(struct sk_buff *skb, unsigned context,
    UINT32 nFlag );
static void bcmxtmrt_recycle(pNBuff_t pNBuff, unsigned context, UINT32 nFlag);
static FN_HANDLER_RT bcmxtmrt_rxisr(int nIrq, void *pRxDma);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static int bcmxtmrt_poll_napi(struct napi_struct *napi, int budget);
#else
static int bcmxtmrt_poll(struct net_device *dev, int *budget);
#endif
static UINT32 bcmxtmrt_rxtask( UINT32 ulBudget, UINT32 *pulMoreToDo );
static void ProcessRxCell(PBCMXTMRT_GLOBAL_INFO pGi, BcmXtm_RxDma *rxdma, UINT8 *pucData);
#if defined(AEI_VDSL_TOOLBOX)
static void AEI_UpdateMirrorFlag(UINT16 *mirFlags, char *intfName, UINT8 enable);
static void AEI_MultiMirrorPacket( struct sk_buff *skb, UINT16 mirFlags );
static void AEI_MirrorPacket( struct sk_buff *skb, char *intfName );
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
static UBOOL8 AEI_hasVlanTag(struct sk_buff *skb);
static UBOOL8 AEI_matchVlanId(struct sk_buff *skb, int vlanId);
#endif
#else
static void MirrorPacket(struct sk_buff *skb, char *intfName, int need_unshare);
#endif
static void bcmxtmrt_timer( PBCMXTMRT_GLOBAL_INFO pGi );
static void bcmxtmrt_sw_sched_initOrReset ( PBCMXTMRT_GLOBAL_INFO pGi ) ;
inline static UINT32 bcmxtmrt_swsched_getxmit_len (BcmPktDma_XtmTxDma *txSwdma);
static void bcmxtmrt_swsched_xmit (XtmRtTxSchedContext *pTxSchedCtxt,
         BcmPktDma_XtmTxDma *txSwdma, int channel, int txSwChannel) ;
static void bcmxtmrt_swsched_swdma_bd_free(XtmRtTxSchedContext *pTxSchedCtxt,
         UINT32 txSwdmaIndex, UINT32 txBdIndex);
static void bcmxtmrt_swsched_recycle (XtmRtTxSchedContext *pXtmTxSchedCtxt) ;
static void bcmxtmrt_swscheduler ( PBCMXTMRT_GLOBAL_INFO pGi ) ;
static int DoGlobInitReq( PXTMRT_GLOBAL_INIT_PARMS pGip );
static int DoGlobUninitReq( void );
static int DoCreateDeviceReq( PXTMRT_CREATE_NETWORK_DEVICE pCnd );
static int DoRegCellHdlrReq( PXTMRT_CELL_HDLR pCh );
static int DoUnregCellHdlrReq( PXTMRT_CELL_HDLR pCh );
static int DoLinkStsChangedReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoLinkUp( PBCMXTMRT_DEV_CONTEXT pDevCtx,
                     PXTMRT_LINK_STATUS_CHANGE pLsc,
                     UINT32 ulDevId);
static int DoLinkDownRx( UINT32 ulPortId );
static int DoLinkDownTx( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc );
static int DoSetTxHwQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static int DoSetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static int DoUnsetTxHwQueues ( void ) ;
static int DoUnsetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId );
static void ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx, volatile BcmPktDma_XtmTxDma *txdma,
    volatile DmaStateRam *pStRam) ;
static void freeXmitPkts (PBCMXTMRT_DEV_CONTEXT pDevCtx, volatile BcmPktDma_XtmTxDma *txdma,
                          int flushOrDisableSuccess);
static void FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx, volatile BcmPktDma_XtmTxDma *txdma) ;
static int DoSendCellReq( PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC );
static int DoDeleteDeviceReq( PBCMXTMRT_DEV_CONTEXT pDevCtx );
static int DoGetNetDevTxChannel( PXTMRT_NETDEV_TXCHANNEL pParm );
static int DoTogglePortDataStatusReq (PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE pParm) ;
static int bcmxtmrt_add_proc_files( void );
static int bcmxtmrt_del_proc_files( void );
static int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt,
    int *eof, void *data);
static int ProcDmaRxInfo(char *page, char **start, off_t off, int cnt,
    int *eof, void *data);
static int ProcTxSchedInfo(char *page, char **start, off_t off, int cnt,
    int *eof, void *data);
static void bcmxtmrt_swsched_set_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma);
static void bcmxtmrt_swsched_disable_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma);
static void bcmxtmrt_swsched_flush_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma);
static void bcmxtmrt_swsched_unset_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma);
static void (*bcmxtmrt_swsched_timer_int_cb) (PBCMXTMRT_GLOBAL_INFO data) ;
static PBCMXTMRT_GLOBAL_INFO bcmxtmrt_swsched_timer_int_cb_data ;
static unsigned int bcmxtmrt_swsched_timer_int(unsigned int i) ;
static int bcmxtmrt_compute_us_sched_timer_divpktsize (UINT32 usRate, UINT32 dsRate) ;
static int bcmxtmrt_init_swsched_timer (void (*cb)(PBCMXTMRT_GLOBAL_INFO cb_data), PBCMXTMRT_GLOBAL_INFO data,
                                         int timerInit) ;
static void bcmxtmrt_uninit_swsched_timer (void);


/* Globals. */
BCMXTMRT_GLOBAL_INFO g_GlobalInfo;
static struct atm_ioctl g_PppoAtmIoctlOps =
    {
        .ioctl    = bcmxtmrt_atm_ioctl,
    };
static struct ppp_channel_ops g_PppoAtmOps =
    {
        .start_xmit = bcmxtmrt_pppoatm_send
    };
static const struct atmdev_ops g_AtmDevOps =
    {
        .open       = bcmxtmrt_atmdev_open,
        .close      = bcmxtmrt_atmdev_close,
        .send       = bcmxtmrt_atmdev_send,
    };

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static const struct header_ops bcmXtmRt_headerOps = {
    .parse = NULL
};

static const struct net_device_ops bcmXtmRt_netdevops = {

    .ndo_open           = bcmxtmrt_open,
    .ndo_stop           = bcmxtmrt_close,
    .ndo_start_xmit     = (HardStartXmitFuncP)bcmxtmrt_xmit,
    .ndo_do_ioctl       = bcmxtmrt_ioctl,
    .ndo_tx_timeout     = bcmxtmrt_timeout,
    .ndo_get_stats      = bcmxtmrt_query,
    .ndo_change_mtu     = bcmxtmrt_change_mtu
 };
#endif

static int bcmxtmrt_in_init_dev = 0;

extern BcmPktDma_Bds *bcmPktDma_Bds_p;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
extern uint32_t iqos_enable_g;
extern uint32_t iqos_debug_g;
extern uint32_t iqos_cpu_cong_g;
extern iqos_status_hook_t iqos_xtm_status_hook_g;
static thresh_t xtm_rx_dma_iq_thresh[XTM_RX_CHANNELS_MAX];

static void xtm_rx_set_iq_thresh( int chnl );
static void xtm_rx_init_iq_thresh(int chnl);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static thresh_t xtm_rx_dqm_iq_thresh[XTM_RX_CHANNELS_MAX];

extern iqos_fap_xtmRxDqmQueue_hook_t iqos_fap_xtmRxDqmQueue_hook_g;
void xtm_set_iq_thresh_dqm(int channel, uint16 loThresh, uint16 hiThresh);
void xtm_iq_dqm_update_cong_status(int channel);
void xtm_iq_dqm_status(void);
#endif
void xtm_iq_update_cong_status( int chnl );
void xtm_iq_status(void);
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
extern uint32_t gbpm_enable_g;
static inline int xtm_bpm_free_buf( BcmXtm_RxDma *rxdma, uint8 *pData);
static int xtm_bpm_alloc_buf_ring( BcmXtm_RxDma *rxdma, UINT32 num );
static void xtm_bpm_free_buf_ring( BcmXtm_RxDma *rxdma );

/* Sanity checks */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#if (BPM_XTM_BULK_ALLOC_COUNT > FAP_BPM_XTM_BULK_ALLOC_MAX)
#error "ERROR - BPM_XTM_BULK_ALLOC_COUNT > FAP_BPM_XTM_BULK_ALLOC_MAX"
#endif
#else
/* BPM status and threshold dump handler hooks */
extern gbpm_status_hook_t gbpm_xtm_status_hook_g;
extern gbpm_thresh_hook_t gbpm_xtm_thresh_hook_g;

void xtm_bpm_status(void);
void xtm_bpm_dump_txq_thresh(void);
static inline int xtm_bpm_alloc_buf( BcmXtm_RxDma *rxdma );
#endif


static int xtm_bpm_txq_thresh( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId, int qId);
#endif


#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
/* Add code for buffer quick free between enet and xtm - June 2010 */
static RecycleFuncP enet_recycle_hook = NULL;
#endif

/***************************************************************************
 * Function Name: bcmxtmrt_init
 * Description  : Called when the driver is loaded.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int __init bcmxtmrt_init( void )
{
    UINT16 usChipId  = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
    UINT16 usChipRev = (PERF->RevID & REV_ID_MASK);

    printk(CARDNAME ": Broadcom BCM%X%X ATM/PTM Network Device ", usChipId,
        usChipRev);
    printk(VER_STR "\n");

    g_pXtmGlobalInfo = &g_GlobalInfo;

    memset(&g_GlobalInfo, 0x00, sizeof(g_GlobalInfo));

    g_GlobalInfo.ulChipRev = PERF->RevID;
    register_atm_ioctl(&g_PppoAtmIoctlOps);
    g_GlobalInfo.pAtmDev = atm_dev_register("bcmxtmrt_atmdev", &g_AtmDevOps,
        -1, NULL);
    if( g_GlobalInfo.pAtmDev )
    {
        g_GlobalInfo.pAtmDev->ci_range.vpi_bits = 12;
        g_GlobalInfo.pAtmDev->ci_range.vci_bits = 16;
    }
    bcmxtmrt_add_proc_files();

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Add code for buffer quick free between enet and xtm - June 2010 */
    bcmPktDma_set_xtm_recycle((RecycleFuncP)bcmxtmrt_recycle,
                              (RecycleFuncP)bcmxtmrt_recycle_skb_or_data);
#else
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    gbpm_xtm_status_hook_g = xtm_bpm_status;
    gbpm_xtm_thresh_hook_g = xtm_bpm_dump_txq_thresh;
#endif
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    iqos_xtm_status_hook_g = xtm_iq_dqm_status;
#else
    iqos_xtm_status_hook_g = xtm_iq_status;
#endif
#endif
    return 0;
} /* bcmxtmrt_init */


/***************************************************************************
 * Function Name: bcmxtmrt_cleanup
 * Description  : Called when the driver is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_cleanup( void )
{
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    iqos_xtm_status_hook_g = NULL;
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    gbpm_xtm_status_hook_g = NULL;
    gbpm_xtm_thresh_hook_g = NULL;
#endif
#endif

    bcmxtmrt_del_proc_files();
    deregister_atm_ioctl(&g_PppoAtmIoctlOps);
    if( g_GlobalInfo.pAtmDev )
    {
        atm_dev_deregister( g_GlobalInfo.pAtmDev );
        g_GlobalInfo.pAtmDev = NULL;
    }
} /* bcmxtmrt_cleanup */


/***************************************************************************
 * Function Name: bcmxtmrt_open
 * Description  : Called to make the device operational.  Called due to shell
 *                command, "ifconfig <device_name> up".
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_open( struct net_device *dev )
{
    int nRet = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
#else
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
#endif
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    BCM_XTM_DEBUG("bcmxtmrt_open\n");

    netif_start_queue(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    napi_enable(&pDevCtx->napi);
#endif

    if( pDevCtx->ulAdminStatus == ADMSTS_UP )
        pDevCtx->ulOpenState = XTMRT_DEV_OPENED;
    else
        nRet = -EIO;

/* FAP enables RX in open as packets can/will arrive between
   link up and open which cause the interrupt to not be reset  */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    {
        int i;
        BcmXtm_RxDma *rxdma;
        for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
        {
           rxdma = pGi->rxdma[i];
           rxdma->pktDmaRxInfo.rxDma->intMask =
                DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
           bcmPktDma_XtmClrRxIrq(&rxdma->pktDmaRxInfo);
           bcmPktDma_XtmRxEnable(&rxdma->pktDmaRxInfo);
        }
    }
#else
    {
        int i ;
        BcmXtm_RxDma *rxdma ;

        for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
        {
           rxdma = pGi->rxdma[i];
           if (rxdma->pktDmaRxInfo.rxBds)
           {
              rxdma->pktDmaRxInfo.rxDma->intMask =
                    DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
              bcmPktDma_XtmRxEnable(&rxdma->pktDmaRxInfo);
              BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);
           }
        }
    }
#endif

    return( nRet );
} /* bcmxtmrt_open */


/***************************************************************************
 * Function Name: bcmxtmrt_close
 * Description  : Called to stop the device.  Called due to shell command,
 *                "ifconfig <device_name> down".
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int bcmxtmrt_close( struct net_device *dev )
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
#else
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
#endif

    if( pDevCtx->ulOpenState != XTMRT_DEV_CLOSED )
    {
        BCM_XTM_DEBUG("bcmxtmrt_close\n");

        pDevCtx->ulOpenState = XTMRT_DEV_CLOSED;
        netif_stop_queue(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        napi_disable(&pDevCtx->napi);
#endif
    }

    return 0;
} /* bcmxtmrt_close */


/***************************************************************************
 * Function Name: bcmxtmrt_timeout
 * Description  : Called when there is a transmit timeout.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_timeout( struct net_device *dev )
{
    dev->trans_start = jiffies;
    netif_wake_queue(dev);
} /* bcmxtmrt_timeout */


/***************************************************************************
 * Function Name: bcmxtmrt_query
 * Description  : Called to return device statistics.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static struct net_device_stats *bcmxtmrt_query(struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
#else
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
#endif
    struct net_device_stats *pStats = &pDevCtx->DevStats;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;
    UINT32 found      = 0;
    UINT32 rx_bytes   = 0;
    UINT32 rx_packets = 0;

    if( pDevCtx->ucTxVcid != INVALID_VCID )
        pStats->tx_bytes += pGi->pulMibTxOctetCountBase[pDevCtx->ucTxVcid];

    if (pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
    {
       pStats->rx_bytes   = pGi->ptmBondInfo.bStats.rxo ;
       pStats->rx_packets = (pGi->ptmBondInfo.bStats.rxp +
                             pGi->ptmBondInfo.bStats.rxpl) ;
    }
    else
    {
       for( i = 0; i < MAX_DEFAULT_MATCH_IDS; i++ )
       {
          if( pGi->pDevCtxsByMatchId[i] == pDevCtx )
          {
             *pGi->pulMibRxMatch = i;
             rx_bytes   += *pGi->pulMibRxOctetCount;
             *pGi->pulMibRxMatch = i;
             rx_packets += *pGi->pulMibRxPacketCount;
             found = 1;

             if ((pGi->bondConfig.sConfig.ptmBond != BC_PTM_BONDING_ENABLE)
                   &&
                   (pGi->bondConfig.sConfig.atmBond != BC_ATM_BONDING_ENABLE))
             {
                /* By convension, VCID 0 collects statistics for packets that use
                 * Packet CMF rules.
                 */
                if( i == 0 )
                {
                   UINT32 j;
                   for( j = MAX_DEFAULT_MATCH_IDS + 1; j < MAX_MATCH_IDS; j++ )
                   {
                      *pGi->pulMibRxMatch = j;
                      rx_bytes   += *pGi->pulMibRxOctetCount;
                      *pGi->pulMibRxMatch = j;
                      rx_packets += *pGi->pulMibRxPacketCount;
                   }
                } /* if (i) */
             }
          }
       } /* for (i) */
       if( found )
       {
          if( *pGi->pulMibRxCtrl & pGi->ulMibRxClrOnRead )
          {
             pStats->rx_bytes   += rx_bytes;
             pStats->rx_packets += rx_packets;
          }
          else 
          {
             pStats->rx_bytes    = rx_bytes;
             pStats->rx_packets  = rx_packets;
          }
       }
    }

    return( pStats );
} /* bcmxtmrt_query */


/***************************************************************************
 * Function Name: bcmxtmrt_change_mtu
 * Description  : Called to change device mtu.
 * Returns      : None
 ***************************************************************************/
static int bcmxtmrt_change_mtu(struct net_device *dev, int new_mtu)
{
    //printk("[%s.%d] %s->mtu changing to %d (was %d)\n", __func__, __LINE__, dev->name, new_mtu, dev->mtu);
    if (new_mtu < ETH_ZLEN || new_mtu > ETH_DATA_LEN)
        return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}


/***************************************************************************
 * Function Name: bcmxtmrt_clrStats
 * Description  : Called to clear device statistics.
 * Returns      : None
 ***************************************************************************/
static void bcmxtmrt_clrStats(struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
#else
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
#endif
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    *pGi->pulMibRxCtrl |= pGi->ulMibRxClrOnRead;
    bcmxtmrt_query(dev);
    *pGi->pulMibRxCtrl &= ~pGi->ulMibRxClrOnRead;
    memset(&pDevCtx->DevStats, 0, sizeof(struct net_device_stats));
} /* bcmxtmrt_clrStats */

/***************************************************************************
 * Function Name: bcmxtmrt_ioctl
 * Description  : Driver IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_ioctl(struct net_device *dev, struct ifreq *Req, int nCmd)
{
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
 #else
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
 #endif
    int *data=(int*)Req->ifr_data;
    int status;
    MirrorCfg mirrorCfg;
    int nRet = 0;

    switch (nCmd)
    {
    case SIOCGLINKSTATE:
        if( pDevCtx->ulLinkState == LINK_UP )
            status = LINKSTATE_UP;
        else
            status = LINKSTATE_DOWN;
        if (copy_to_user((void*)data, (void*)&status, sizeof(int)))
            nRet = -EFAULT;
        break;

    case SIOCSCLEARMIBCNTR:
        bcmxtmrt_clrStats(dev);
        break;

    case SIOCMIBINFO:
        if (copy_to_user((void*)data, (void*)&pDevCtx->MibInfo,
            sizeof(pDevCtx->MibInfo)))
        {
            nRet = -EFAULT;
        }
        break;

    case SIOCPORTMIRROR:
        if(copy_from_user((void*)&mirrorCfg,data,sizeof(MirrorCfg)))
            nRet=-EFAULT;
        else
        {
#if defined(AEI_VDSL_TOOLBOX)
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
            pDevCtx->matchVlanId = mirrorCfg.vlanId;
#endif

            if( mirrorCfg.nDirection == MIRROR_DIR_IN )
            {
                AEI_UpdateMirrorFlag(&(pDevCtx->usMirrorInFlags),
                          mirrorCfg.szMirrorInterface, mirrorCfg.nStatus);
            }
            else /* MIRROR_DIR_OUT */
            {
                AEI_UpdateMirrorFlag(&(pDevCtx->usMirrorOutFlags),
                          mirrorCfg.szMirrorInterface, mirrorCfg.nStatus);
            }
#else
            if( mirrorCfg.nDirection == MIRROR_DIR_IN )
            {
                if( mirrorCfg.nStatus == MIRROR_ENABLED )
                    strcpy(pDevCtx->szMirrorIntfIn, mirrorCfg.szMirrorInterface);
                else
                    memset(pDevCtx->szMirrorIntfIn, 0x00, MIRROR_INTF_SIZE);
            }
            else /* MIRROR_DIR_OUT */
            {
                if( mirrorCfg.nStatus == MIRROR_ENABLED )
                    strcpy(pDevCtx->szMirrorIntfOut, mirrorCfg.szMirrorInterface);
                else
                    memset(pDevCtx->szMirrorIntfOut, 0x00, MIRROR_INTF_SIZE);
            }
#endif
        }
        break;

#if defined(AEI_VDSL_SMARTLED)
    case SIOCINETTRAFFICBLINK:
        if (copy_from_user((void*)&pDevCtx->inetTrafficBlinkEnable, data,
            sizeof(pDevCtx->inetTrafficBlinkEnable)))
        {
            nRet=-EFAULT;
        }
        break;
#endif

#if defined(AEI_VDSL_STATS_DIAG) && !defined(AEI_VDSL_CUSTOMER_NCS)
    case SIOCGETMULTICASTSTATS:
    {
        if (copy_to_user((void *)data, (void *)&pDevCtx->multi_stats,
                         sizeof(struct xtm_multicast_stats)))
        {
            nRet = -EFAULT;
        }
        break;
    }
#endif

    case SIOCETHTOOL:
        nRet = bcmxtmrt_ethtool_ioctl(pDevCtx, (void *) Req->ifr_data);
        break;

    default:
        nRet = -EOPNOTSUPP;
        break;
    }

    return( nRet );
} /* bcmxtmrt_ioctl */


/***************************************************************************
 * Function Name: bcmxtmrt_ethtool_ioctl
 * Description  : Driver ethtool IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_ethtool_ioctl(PBCMXTMRT_DEV_CONTEXT pDevCtx, void *useraddr)
{
    struct ethtool_drvinfo info;
    struct ethtool_cmd ecmd;
    unsigned long ethcmd;
    int nRet = 0;

    if( copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)) == 0 )
    {
        switch (ethcmd)
        {
        case ETHTOOL_GDRVINFO:
            info.cmd = ETHTOOL_GDRVINFO;
            strncpy(info.driver, CARDNAME, sizeof(info.driver)-1);
            strncpy(info.version, VERSION, sizeof(info.version)-1);
            if (copy_to_user(useraddr, &info, sizeof(info)))
                nRet = -EFAULT;
            break;

        case ETHTOOL_GSET:
            ecmd.cmd = ETHTOOL_GSET;
            ecmd.speed = pDevCtx->MibInfo.ulIfSpeed / (1024 * 1024);
            if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
                nRet = -EFAULT;
            break;

        default:
            nRet = -EOPNOTSUPP;
            break;
        }
    }
    else
       nRet = -EFAULT;

    return( nRet );
}

/***************************************************************************
 * Function Name: bcmxtmrt_atm_ioctl
 * Description  : Driver ethtool IOCTL entry point.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atm_ioctl(struct socket *sock, unsigned int cmd,
    unsigned long arg)
{
    struct atm_vcc *pAtmVcc = ATM_SD(sock);
    void __user *argp = (void __user *)arg;
    atm_backend_t b;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    int nRet = -ENOIOCTLCMD;

    switch( cmd )
    {
    case ATM_SETBACKEND:
        if( get_user(b, (atm_backend_t __user *) argp) == 0 )
        {
            switch (b)
            {
            case ATM_BACKEND_PPP_BCM:
                if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci))!=NULL &&
                    pDevCtx->Chan.private == NULL )
                {
                    pDevCtx->Chan.private = pDevCtx->pDev;
                    pDevCtx->Chan.ops = &g_PppoAtmOps;
                    pDevCtx->Chan.mtu = 1500; /* TBD. Calc value. */
                    pAtmVcc->user_back = pDevCtx;
                    if( ppp_register_channel(&pDevCtx->Chan) == 0 )
                        nRet = 0;
                    else
                        nRet = -EFAULT;
                }
                else
                    nRet = (pDevCtx) ? 0 : -EFAULT;
                break;

            case ATM_BACKEND_PPP_BCM_DISCONN:
                /* This is a patch for PPP reconnection.
                 * ppp daemon wants us to send out an LCP termination request
                 * to let the BRAS ppp server terminate the old ppp connection.
                 */
                if((pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
                {
                    struct sk_buff *skb;
                    int size = 6;
                    int eff  = (size+3) & ~3; /* align to word boundary */

                    while (!(skb = alloc_skb(eff, GFP_KERNEL)))
                        schedule();

                    skb->dev = NULL; /* for paths shared with net_device interfaces */
                    skb_put(skb, size);

                    skb->data[0] = 0xc0;  /* PPP_LCP == 0xc021 */
                    skb->data[1] = 0x21;
                    skb->data[2] = 0x05;  /* TERMREQ == 5 */
                    skb->data[3] = 0x02;  /* id == 2 */
                    skb->data[4] = 0x00;  /* HEADERLEN == 4 */
                    skb->data[5] = 0x04;

                    if (eff > size)
                        memset(skb->data+size,0,eff-size);

                    nRet = bcmxtmrt_xmit( SKBUFF_2_PNBUFF(skb), pDevCtx->pDev );
                }
                else
                    nRet = -EFAULT;
                break;

            case ATM_BACKEND_PPP_BCM_CLOSE_DEV:
                if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL)
                {
                    bcmxtmrt_pppoatm_send(&pDevCtx->Chan, NULL);
                    ppp_unregister_channel(&pDevCtx->Chan);
                    pDevCtx->Chan.private = NULL;
                }
                nRet = 0;
                break;

            default:
                break;
            }
        }
        else
            nRet = -EFAULT;
        break;

    case PPPIOCGCHAN:
        if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL )
        {
            nRet = put_user(ppp_channel_index(&pDevCtx->Chan),
                (int __user *) argp) ? -EFAULT : 0;
        }
        else
            nRet = -EFAULT;
        break;

    case PPPIOCGUNIT:
        if( (pDevCtx = FindDevCtx(pAtmVcc->vpi, pAtmVcc->vci)) != NULL )
        {
            nRet = put_user(ppp_unit_number(&pDevCtx->Chan),
                (int __user *) argp) ? -EFAULT : 0;
        }
        else
            nRet = -EFAULT;
        break;
    default:
        break;
    }

    return( nRet );
} /* bcmxtmrt_atm_ioctl */


/***************************************************************************
 * Function Name: FindDevCtx
 * Description  : Finds a device context structure for a VCC.
 * Returns      : Pointer to a device context structure or NULL.
 ***************************************************************************/
static PBCMXTMRT_DEV_CONTEXT FindDevCtx( short vpi, int vci )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( (pDevCtx = pGi->pDevCtxs[i]) != NULL )
        {
            if( pDevCtx->Addr.u.Vcc.usVpi == vpi &&
                pDevCtx->Addr.u.Vcc.usVci == vci )
            {
                break;
            }

            pDevCtx = NULL;
        }
    }

    return( pDevCtx );
} /* FindDevCtx */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_open
 * Description  : ATM device open
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atmdev_open(struct atm_vcc *pVcc)
{
    set_bit(ATM_VF_READY,&pVcc->flags);
    return( 0 );
} /* bcmxtmrt_atmdev_open */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_close
 * Description  : ATM device open
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static void bcmxtmrt_atmdev_close(struct atm_vcc *pVcc)
{
    clear_bit(ATM_VF_READY,&pVcc->flags);
    clear_bit(ATM_VF_ADDR,&pVcc->flags);
} /* bcmxtmrt_atmdev_close */


/***************************************************************************
 * Function Name: bcmxtmrt_atmdev_send
 * Description  : send data
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_atmdev_send(struct atm_vcc *pVcc, struct sk_buff *skb)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = FindDevCtx( pVcc->vpi, pVcc->vci );
    int nRet;

    if( pDevCtx )
        nRet = bcmxtmrt_xmit( SKBUFF_2_PNBUFF(skb), pDevCtx->pDev );
    else
        nRet = -EIO;

    return( nRet );
} /* bcmxtmrt_atmdev_send */



/***************************************************************************
 * Function Name: bcmxtmrt_pppoatm_send
 * Description  : Called by the PPP driver to send data.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_pppoatm_send(struct ppp_channel *pChan, struct sk_buff *skb)
{
    if ( skb ) skb->dev = (struct net_device *) pChan->private;
    bcmxtmrt_xmit( SKBUFF_2_PNBUFF(skb), (struct net_device *) pChan->private );
    return(1);
} /* bcmxtmrt_pppoatm_send */


/***************************************************************************
 * Function Name: QueuePacket
 * Description  : Determines whether to queue a packet for transmission based
 *                on the number of total external (ie Ethernet) buffers and
 *                buffers already queued.
 *                For all ATM cells (ASM, OAM which are locally originated and
 *                mgmt based), we allow them to get queued as they are critical
 *                & low frequency based.
 *                For ex., if we drop sucessive ASM cels during congestion (the whole bonding
 *                layer will be reset end to end). So, the criteria here should
 *                be applied more for data packets than for mgmt cells.
 * Returns      : 1 to queue packet, 0 to drop packet
 ***************************************************************************/
inline int QueuePacket( PBCMXTMRT_GLOBAL_INFO pGi, PTXQINFO pTqi, UINT32 isAtmCell )
{
    int nRet = 0; /* default to drop packet */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if (gbpm_enable_g)
    {
        UINT32 thresh;

        if (gbpm_get_dyn_buf_level())
            thresh = pTqi->ulHiThresh;
        else
            thresh = pTqi->ulLoThresh;

        if ( ( pTqi->ulNumTxBufsQdOne < thresh) || (isAtmCell))
        {
            nRet = 1; /* queue packet */
            pGi->ulDbgQ1++;
        }
        else
        {
            pGi->ulDbgD1++;
            pTqi->ulDropped++;
        }
    }
    else
    {
        nRet = 1; /* queue packet */
        pGi->ulDbgQ1++;
    }
#else
    if( pGi->ulNumTxQs == 1 )
    {
        /* One total transmit queue.  Allow up to 90% of external buffers to
         * be queued on this transmit queue.
         */
        if(( pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs90Pct )
              ||
           (isAtmCell))
        {
            nRet = 1; /* queue packet */
            pGi->ulDbgQ1++;
        }
        else
            pGi->ulDbgD1++;
    }
    else
    {
        if(pGi->ulNumExtBufs - pGi->ulNumTxBufsQdAll > pGi->ulNumExtBufsRsrvd)
        {
            /* The available number of external buffers is greater than the
             * reserved value.  Allow up to 50% of external buffers to be
             * queued on this transmit queue.
             */
            if(( pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs50Pct )
                 ||
                (isAtmCell))
            {
                nRet = 1; /* queue packet */
                pGi->ulDbgQ2++;
            }
            else
                pGi->ulDbgD2++;
        }
        else
        {
            /* Divide the reserved number of external buffers evenly among all
             * of the transmit queues.
             */
            if((pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufsRsrvd / pGi->ulNumTxQs)
                 ||
                (isAtmCell))
            {
                nRet = 1; /* queue packet */
                pGi->ulDbgQ3++;
            }
            else
                pGi->ulDbgD3++;
        }
    }
#endif

    return( nRet );
} /* QueuePacket */

#ifdef AEI_VDSL_SMALL_PACKET_PRIORITY
 // 1: it is TCP ACK (no push)
// 0: NOT match
static int is_tcp_ack( const UINT8 * pData, int len )
{
#define AEI_ETH_HEAD_LEN       (14)
#define AEI_VLAN_HEAD_LEN       (4)     // VLAN is option
#define AEI_MIN_IP_HEAD_LEN     (20)
#define AEI_MIN_TCP_HEAD_LEN    (20)


#define AEI_L3_TYPE_OFFSET_H    (12)
#define AEI_L3_TYPE_OFFSET_L    (13)


#define AEI_IP_VER(p)           ((p[0] & 0xF0)>>4)
#define AEI_PROTOCOL(p)         (p[9])
#define AEI_IP_HDR_LEN(p)       ((p[0] & 0x0F)<<2)      // <<2, means *4

#define AEI_TCP_FLAGS(p)        (p[13])

        int bTcpAck = FALSE;
        //int ip_hdr_len;
        int bVlan = FALSE;

        do {
                if( len < (AEI_ETH_HEAD_LEN + AEI_MIN_IP_HEAD_LEN + AEI_MIN_TCP_HEAD_LEN))
                        // not long enough
                        break;


                // Init, Check vlan
                if( (pData[AEI_L3_TYPE_OFFSET_H]==0x81) && ((pData[AEI_L3_TYPE_OFFSET_L]==0x00)) )  {
                        bVlan = TRUE;
                        pData += AEI_VLAN_HEAD_LEN; // pData bypass vlan
                }       

                // 1. Check IP (0x0800)
                if( (pData[AEI_L3_TYPE_OFFSET_H]!=0x08) || (pData[AEI_L3_TYPE_OFFSET_L]!=0x00)) {
                        //printk( "not IP\n" );
                        break;  
                }       
                pData += AEI_ETH_HEAD_LEN; // pData ptr to IP header

                // 2. Check TCP (0x06)
                if( 0x04 != AEI_IP_VER(pData)) {
                        //printk( "NOT support IPv4 now\n" );
                        // To support IPv6, add code later.
                        break;
                }
                if( 0x06 != AEI_PROTOCOL(pData)) {
                        //printk( "NOT TCP\n" );
                        break;
                }
// Jean, 2011.5.9, Check TCP is enough
#if 0
                ip_hdr_len = AEI_IP_HDR_LEN(pData);
                if( len < (AEI_ETH_HEAD_LEN + (bVlan?AEI_VLAN_HEAD_LEN:0) + ip_hdr_len + AEI_MIN_TCP_HEAD_LEN)) {
                        //printk( "NOT long enough\n" );
                        break;
                }
                pData += ip_hdr_len; // pData ptr to TCP header

		// 3. Check Flags, ACK/SYN bit and No PSH bit, (0x10), NOT (0x18)
#define AEI_F_ACK 	(0x01<<4)
#define AEI_F_PSH 	(0x01<<3)
#define AEI_F_SYN 	(0x01<<1)

		UINT8 tcpFlag = AEI_TCP_FLAGS(pData);
		//if( tcpFlag & AEI_F_PSH ) {
		//	break;
		//}
		if( !((tcpFlag & AEI_F_ACK) || (tcpFlag & AEI_F_SYN))) {
			break;
		}
#endif
                bTcpAck = TRUE;
        } while(0);

        return bTcpAck;
}
#endif


/***************************************************************************
 * Function Name: bcmxtmrt_xmit
 * Description  : Check for transmitted packets to free and, if skb is
 *                non-NULL, transmit a packet. Transmit may be invoked for
 *                a packet originating from the network stack or from a
 *                packet received from another interface.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_xmit( pNBuff_t pNBuff, struct net_device *dev )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    PBCMXTMRT_DEV_CONTEXT pDevCtx = netdev_priv(dev);
#else
    PBCMXTMRT_DEV_CONTEXT pDevCtx = dev->priv;
#endif
    DmaDesc  dmaDesc;
#if defined(AEI_VDSL_STATS_DIAG)
    int is_multicast = 0;
#endif

    spin_lock_bh(&pGi->xtmlock_tx);

    if( pDevCtx->ulLinkState == LINK_UP )
    {
        BcmPktDma_XtmTxDma *txdma = NULL;
        UINT8 * pData;
        UINT32 i, channel;
        unsigned len, uMark, uPriority;
        struct sk_buff * pNBuffSkb; /* If pNBuff is sk_buff: protocol access */

        /* Free packets that have been transmitted. */
        for (i=0; i<pDevCtx->ulTxQInfosSize; i++)
        {
            uint32               txSource;
            uint32               txAddr;
            uint32               rxChannel;
            pNBuff_t             nbuff_reclaim_p;

            txdma = pDevCtx->txdma[i];
            while (bcmPktDma_XtmFreeXmitBufGet(txdma, (uint32 *)&nbuff_reclaim_p,
                                               &txSource, &txAddr, &rxChannel,
                                               TXDMATYPE(pDevCtx), 0x0) == TRUE)
            {
                if (nbuff_reclaim_p != PNBUFF_NULL)
                {
                    spin_unlock_bh(&pGi->xtmlock_tx);
                    BCM_XTM_TX_DEBUG("Host bcmPktDma_XtmFreeXmitBufGet TRUE! (xmit) key 0x%x\n", (int)nbuff_reclaim_p);
                    nbuff_free(nbuff_reclaim_p);
                    spin_lock_bh(&pGi->xtmlock_tx);
                }
            }
        }

        if( nbuff_get_params(pNBuff, &pData, &len, &uMark, &uPriority)
            == (void*)NULL )
        {
            goto unlock_done_xmit;
        }

#if defined(AEI_VDSL_STATS_DIAG)
        /* multicast packets (including broadcast packets) */
        if (pData && (*pData & 1))
        {
            is_multicast = 1;
        }
#endif

#ifdef AEI_VDSL_SMALL_PACKET_PRIORITY
        if (len<129 && uMark==0 && is_tcp_ack(pData,len)){
			
            uMark |= AEI_VDSL_SMALL_PACKET_PRIORITY;
        }
#endif
        if (IS_SKBUFF_PTR(pNBuff))
        {
            pNBuffSkb = PNBUFF_2_SKBUFF(pNBuff);

            /* Give the highest possible priority to ARP packets */
            if (pNBuffSkb->protocol == __constant_htons(ETH_P_ARP))
               uMark |= 0x7;
        }
        else
        {
            pNBuffSkb = NULL;
        }

//spin_unlock_bh(&pGi->xtmlock_tx);
//BCM_XTM_TX_DEBUG("XTM TX: pNBuff<0x%08x> pNBuffSkb<0x%08x> pData<0x%08x>\n", (int)pNBuff,(int)pNBuffSkb, (int)pData);
//DUMP_PKT(pData, 32);
//spin_lock_bh(&pGi->xtmlock_tx);

        if( pDevCtx->ulTxQInfosSize )
        {
            /* Find a transmit queue to send on. */
            UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;
            UINT32 isAtmCell ;
            UINT32 ulTxAvailable = 0;

            isAtmCell = ( pNBuffSkb &&
                  ((pDevCtx->Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)  &&
                  (pNBuffSkb->protocol & ~FSTAT_CT_MASK) == SKB_PROTO_ATM_CELL
                  );

            if (isAtmCell)       /* Necessary for ASM/OAM cells */
               uMark |= 0x7;

#ifdef CONFIG_NETFILTER
            /* See if this is a classified flow */
            if (SKBMARK_GET_FLOW_ID(uMark))
            {
               /* Non-zero flow id implies classified packet.
                * Find tx queue based on its qid.
                */
               /* For ATM classified packet,
                *   bit 3-0 of nfmark is the queue id (0 to 15).
                *   bit 4   of nfmark is the DSL latency, 0=PATH0, 1=PATH1
                *
                * For PTM classified packet,
                *   bit 2-0 of nfmark is the queue id (0 to 7).
                *   bit 3   of nfmark is the PTM priority, 0=LOW, 1=HIGH
                *   bit 4   of nfmark is the DSL latency, 0=PATH0, 1=PATH1
                */
               /* Classified packet. Find tx queue based on its queue id. */
               if ((pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM) ||
                   (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
               {
                  /* For PTM, bit 2-0 of the 32-bit nfmark is the queue id. */
                  txdma = pDevCtx->pTxQids[uMark & 0x7];

                  /* bit 3 of the 32-bit nfmark is the PTM priority, 0=LOW, 1=HIGH */
                  ulPtmPrioIdx = (uMark >> 3) & 0x1;
               }
               else
               {
                  /* For ATM, bit 3-0 of the 32-bit nfmark is the queue id. */
                  txdma = pDevCtx->pTxQids[uMark & 0xf];
               }
            }
            else
            {
               /* Flow id 0 implies unclassified packet.
                * Find tx queue based on its subpriority.
                */
               /* There are 2 types of unclassified packet flow.
                *   1) Protocol control packets originated locally from CPE.
                *      Such packets are marked the highest subpriority (7),
                *      and will be sent to the highest subpriority queue of
                *      the connection.
                *   2) Packets that do not match any QoS classification rule.
                *      Such packets do not have any subpriority, i.e. 0, and
                *      will be sent to the default (first) queue of the connection.
                */
               /* For unclassified packet,
                *   bit 2-0 of nfmark is the subpriority (0 to 7).
                *   bit 3   of nfmark is the PTM priority, 0=LOW, 1=HIGH
                *   bit 4   of nfmark is the DSL latency, 0=PATH0, 1=PATH1
                */

               /* If the subpriority is the highest (7), use the existing
                * highest priority queue.
                */
               if ((uMark & 0x7) == 0x7)
               {
                  txdma = pDevCtx->pHighestPrio;
               }
               else
                  txdma = NULL;
            }
#endif
            /* If a transmit queue was not found or the queue was disabled,
             * use the first (default) queue.
             */
            if (txdma == NULL || txdma->txEnabled == 0)
               txdma = pDevCtx->txdma[0]; /* the default queue */

            if (txdma && txdma->txEnabled == 1)
            {
                channel = txdma->ulDmaIndex;
                ulTxAvailable = bcmPktDma_XtmXmitAvailable(txdma, TXDMATYPE(pDevCtx));
            }
            else
                ulTxAvailable = 0;

            if (ulTxAvailable && QueuePacket(pGi, txdma, isAtmCell))
            {
                UINT32 ulRfc2684_type; /* Not needed as CMF "F in software" */
                UINT32 ulHdrType = pDevCtx->ulHdrType;
                UINT32 blogPhyType;

                if( (pDevCtx->ulFlags & CNI_HW_ADD_HEADER) == 0 &&
                     HT_LEN(ulHdrType) != 0 && !isAtmCell )
                {
                    ulRfc2684_type = HT_TYPE(ulHdrType);
                }
                else
                    ulRfc2684_type = RFC2684_NONE;

                blogPhyType = BLOG_SET_PHYHDR(ulRfc2684_type, BLOG_XTMPHY);
                spin_unlock_bh(&pGi->xtmlock_tx);
#ifdef CONFIG_BLOG
                blog_emit( pNBuff, dev, pDevCtx->ulEncapType,
                           channel, blogPhyType );
#endif
                spin_lock_bh(&pGi->xtmlock_tx);

#if defined(AEI_VDSL_TOOLBOX)
                if( pDevCtx->usMirrorOutFlags != 0 &&
                    pNBuffSkb && !isAtmCell &&
                    (ulHdrType ==  HT_PTM ||
                     ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
                     ulHdrType ==  HT_VC_MUX_ETHERNET) )
                {
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                    if (pDevCtx->matchVlanId >= 0)
                    {
                        if (pDevCtx->matchVlanId == 0)
                        {
                            if (!AEI_hasVlanTag(pNBuffSkb))
                            {
                                AEI_MultiMirrorPacket( pNBuffSkb, pDevCtx->usMirrorOutFlags );
                            }
                        }
                        else
                        {
                            if (AEI_matchVlanId(pNBuffSkb, pDevCtx->matchVlanId))
                            {
                                AEI_MultiMirrorPacket( pNBuffSkb, pDevCtx->usMirrorOutFlags );
                            }
                        }
                    }
                    else
                    {
                        AEI_MultiMirrorPacket( pNBuffSkb, pDevCtx->usMirrorOutFlags );
                    }
#else
                    AEI_MultiMirrorPacket( pNBuffSkb, pDevCtx->usMirrorOutFlags );
#endif
                }
#else
                if( pDevCtx->szMirrorIntfOut[0] != '\0' &&
                    !isAtmCell &&
                    (ulHdrType ==  HT_PTM ||
                     ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
                     ulHdrType ==  HT_VC_MUX_ETHERNET) )
                {
                    pNBuffSkb = nbuff_xlate(pNBuff);    /* translate to skb */
                    if (pNBuffSkb != (struct sk_buff *)NULL)
                    {
                        MirrorPacket( pNBuffSkb, pDevCtx->szMirrorIntfOut, 1 );
                        pNBuff = SKBUFF_2_PNBUFF( pNBuffSkb );
                        nbuff_get_context( pNBuff, &pData, &len );
                    }
                }
#endif

                if ( ulRfc2684_type )
                {
                    AddRfc2684Hdr(&pNBuff, &pNBuffSkb, &pData, &len, ulHdrType);
                }

                if( len < ETH_ZLEN && !isAtmCell &&
                    (ulHdrType == HT_PTM ||
                     ulHdrType == HT_LLC_SNAP_ETHERNET ||
                     ulHdrType == HT_VC_MUX_ETHERNET) )
                {
                    len = ETH_ZLEN;
                }

                if (pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {
						 if (pDevCtx->ulPortDataMask == 0 ||
							  !bcmxtmrt_ptmbond_add_hdr (pDevCtx, ulPtmPrioIdx, &pNBuff, &pNBuffSkb, &pData, &len)) {
							 nbuff_flushfree(pNBuff);
							 pDevCtx->DevStats.tx_dropped++;
							 goto unlock_done_xmit;
						 }
					 }

#if defined(CONFIG_BCM96368)
                /* Ported from xtmrt/impl1 Apr 2010 */
                if(kerSysGetSdramWidth() == MEMC_16BIT_BUS)
                {
                    pNBuff = nbuff_align_data(pNBuff, &pData, len, NBUFF_ALIGN_MASK_8);
                    if(pNBuff == NULL)
                    {
                        pDevCtx->DevStats.tx_errors++;
                        goto unlock_done_xmit;
                    }
                }
#endif
                nbuff_flush(pNBuff, pData, len);

                dmaDesc.status = DMA_SOP | DMA_EOP | pDevCtx->ucTxVcid;

                if (( pDevCtx->Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK ) == TRAFFIC_TYPE_ATM )
                {
#if defined(AEI_VDSL_SMARTLED)
                    if (pDevCtx->inetTrafficBlinkEnable)
                    {
                        GPIO->AuxLedCtrl |= AUX_STROBE_2;
                    }
#endif

                    if (isAtmCell)
                    {
                        dmaDesc.status |= pNBuffSkb->protocol & FSTAT_CT_MASK;
                        if( (pDevCtx->ulFlags & CNI_USE_ALT_FSTAT) != 0 )
                        {
                            dmaDesc.status |= FSTAT_MODE_COMMON;
                            dmaDesc.status &= ~(FSTAT_COMMON_INS_HDR_EN |
                                              FSTAT_COMMON_HDR_INDEX_MASK);
                        }
                    }
                    else
                    {
                        dmaDesc.status |= FSTAT_CT_AAL5;
                        if( (pDevCtx->ulFlags & CNI_USE_ALT_FSTAT) != 0 )
                        {
                            dmaDesc.status |= FSTAT_MODE_COMMON;
                            if(HT_LEN(ulHdrType) != 0 &&
                               (pDevCtx->ulFlags & CNI_HW_ADD_HEADER) != 0)
                            {
                                dmaDesc.status |= FSTAT_COMMON_INS_HDR_EN |
                                                  ((HT_TYPE(ulHdrType) - 1) <<
                                                  FSTAT_COMMON_HDR_INDEX_SHIFT);
                            }
                            else
                            {
                                dmaDesc.status &= ~(FSTAT_COMMON_INS_HDR_EN |
                                                   FSTAT_COMMON_HDR_INDEX_MASK);
                            }
                        }
                    }
                }
                else {
                    /* Manually strobe INET Activity LED in PTM mode due the bug in the chip */
#if defined(CONFIG_BCM96368)
#if defined(AEI_VDSL_SMARTLED)
                    if (pDevCtx->inetTrafficBlinkEnable)
                    {
                        GPIO->AuxLedCtrl |= AUX_STROBE_2;
                    }
#else
                    GPIO->AuxLedCtrl |= AUX_STROBE_2;
#endif /* AEI_VDSL_SMARTLED */
#endif
                    dmaDesc.status |= FSTAT_CT_PTM | FSTAT_PTM_ENET_FCS |
                                      FSTAT_PTM_CRC;
                }

                dmaDesc.status |= DMA_OWN;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
                if(IS_FKBUFF_PTR(pNBuff))
                {
                    FkBuff_t *pFkb = PNBUFF_2_FKBUFF(pNBuff);

                    /* We can only use the recycle context if this is an xtm or enet buffer */
                    if((pFkb->recycle_hook == (RecycleFuncP)bcmxtmrt_recycle) ||
                       (pFkb->recycle_hook == (RecycleFuncP)enet_recycle_hook))
                    {
                        uint8 *pDataXtm = PFKBUFF_TO_PDATA(pFkb, RXBUF_HEAD_RESERVE);
                        uint8 *pBuffer;
                        uint32 key;
                        uint32 bufSource;
                        uint8  rxChannel = FKB_RECYCLE_CONTEXT(pFkb)->channel;

                        if(pFkb->recycle_hook == (RecycleFuncP)bcmxtmrt_recycle)
                        {
                            pBuffer = pDataXtm;
                            key = (uint32)pNBuff;
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
                            /* TBD Laurie - XTM rx pktdma splitting not implemented yet */
                            bufSource = FAP_XTM_RX;
#elif NUM_FAPS > 1
                            /* This assumes that the XTM RX and XTM TX are split between two
                               different FAPs. */
                            bufSource = HOST_VIA_DQM;
#else
                            bufSource = FAP_XTM_RX;
#endif
                        }
                        else
                        {
                            pBuffer = pData;
                            key = (uint32)PFKBUFF_TO_PDATA(pFkb,RX_ENET_SKB_HEADROOM);
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
                            if(g_Eth_rx_iudma_ownership[rxChannel] == HOST_OWNED )
                            {
                                bufSource = HOST_ETH_RX;
                                key = (uint32)pNBuff;   /* modify key when free will be done by Host not FAP */
                            }
                            else
                            {
                                bufSource = FAP_ETH_RX;
                            }
#elif NUM_FAPS > 1
                            if(g_Eth_rx_iudma_ownership[rxChannel] != g_Xtm_tx_iudma_ownership[txdma->ulDmaIndex])
                            {   /* Return buffer to Host for recycling - Jan 24 */
                                bufSource = HOST_VIA_DQM;
                                key = (uint32)pNBuff;   /* modify key when free will be done by Host not FAP */
                            }
                            else
                            {
                                bufSource = FAP_ETH_RX;
                            }
#else
                            bufSource = FAP_ETH_RX;
#endif
                        }

                        bcmPktDma_XtmXmit(txdma, pData, len, bufSource,
                                          dmaDesc.status, (uint32)key, rxChannel,
                                          TXDMATYPE(pDevCtx), 0);

                        goto tx_continue;
                    }
                }

                bcmPktDma_XtmXmit(txdma, pData, len, HOST_VIA_DQM, dmaDesc.status, (uint32)pNBuff, 0,
                                  TXDMATYPE(pDevCtx), 0);

tx_continue:
#else
                bcmPktDma_XtmXmit(txdma, pData, len, HOST_VIA_LINUX,
                                  dmaDesc.status, (uint32)pNBuff, 0, TXDMATYPE(pDevCtx), 0);
#endif

                /* Transmitted bytes are counted in hardware. */
                pDevCtx->DevStats.tx_packets++;
                
                pDevCtx->pDev->trans_start = jiffies;

#if defined(AEI_VDSL_STATS_DIAG)
                if (is_multicast)
                {
                    pDevCtx->multi_stats.tx_multicast_bytes += len;
                }
#endif
            }
            else
            {
                /* Transmit queue is full.  Free the socket buffer.  Don't call
                 * netif_stop_queue because this device may use more than one
                 * queue.
                 */
                nbuff_flushfree(pNBuff);
                pDevCtx->DevStats.tx_errors++;
            }
        }
        else
        {
            nbuff_flushfree(pNBuff);
            pDevCtx->DevStats.tx_dropped++;
        }
    }
    else
    {
        if( pNBuff )
        {
            nbuff_flushfree(pNBuff);
            pDevCtx->DevStats.tx_dropped++;
        }
    }

unlock_done_xmit:
    spin_unlock_bh(&pGi->xtmlock_tx);

    return 0;
} /* bcmxtmrt_xmit */

/***************************************************************************
 * Function Name: AddRfc2684Hdr
 * Description  : Adds the RFC2684 header to an ATM packet before transmitting
 *                it.
 * Returns      : None.
 ***************************************************************************/
static void AddRfc2684Hdr(pNBuff_t *ppNBuff, struct sk_buff **ppNBuffSkb,
                          UINT8 **ppData, int * pLen, UINT32 ulHdrType)
{
    UINT8 ucHdrs[][16] =
        {{},
         {0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00},
         {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00},
         {0xFE, 0xFE, 0x03, 0xCF},
         {0x00, 0x00}};
    int minheadroom = HT_LEN(ulHdrType);

    if ( *ppNBuffSkb )
    {
        struct sk_buff *skb = *ppNBuffSkb;
        int headroom = skb_headroom(skb);

        if (headroom < minheadroom)
        {
            struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);

            dev_kfree_skb_any(skb);
            skb = (skb2 == NULL) ? NULL : skb2;
        }
        if( skb )
        {
            *ppData = skb_push(skb, minheadroom);
            *pLen = skb->len;
            u16cpy(*ppData, ucHdrs[HT_TYPE(ulHdrType)], minheadroom);
        }
        // else ?
        *ppNBuffSkb = skb;
        *ppNBuff = SKBUFF_2_PNBUFF(skb);
    }
    else // if( IS_FKBUFF_PTR(*ppNBuff) )
    {
        struct fkbuff *fkb = PNBUFF_2_FKBUFF(*ppNBuff);
        int headroom = fkb_headroom(fkb);
        if (headroom >= minheadroom)
        {
            *ppData = fkb_push(fkb, minheadroom);
            *pLen += minheadroom;
            u16cpy(*ppData, ucHdrs[HT_TYPE(ulHdrType)], minheadroom);
        }
        else
            printk(CARDNAME ": FKB not enough headroom.\n");
    }

} /* AddRfc2684Hdr */


/***************************************************************************
 * Function Name: AssignRxBuffer
 * Description  : Put a data buffer back on to the receive BD ring.
 * Returns      : None.
 ***************************************************************************/
void AssignRxBuffer(int channel, UINT8 *pucData)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmXtm_RxDma *rxdma = pGi->rxdma[channel];
    BcmPktDma_XtmRxDma *pktDmaRxInfo_p = &rxdma->pktDmaRxInfo;

    spin_lock_bh(&pGi->xtmlock_rx);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    {
        if (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds)
        {
            /* Free the data buffer to a specific Rx Queue (ie channel) */
            bcmPktDma_XtmFreeRecvBuf(pktDmaRxInfo_p, (unsigned char *)pucData);
        }
        else
        {
            xtm_bpm_free_buf(rxdma, pucData);
        }
    }
#else
    /* Free the data buffer to a specific Rx Queue (ie channel) */
    bcmPktDma_XtmFreeRecvBuf(pktDmaRxInfo_p, (unsigned char *)pucData);
#endif

#else
    /* Free the data buffer to a specific Rx Queue (ie channel) */
    bcmPktDma_XtmFreeRecvBuf(pktDmaRxInfo_p, (unsigned char *)pucData);
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    /* Update congestion status, once all the buffers have been recycled. */
    if (iqos_cpu_cong_g)
    {
        if (pktDmaRxInfo_p->numRxBds == pktDmaRxInfo_p->rxAssignedBds)
            iqos_set_cong_status(IQOS_IF_XTM, channel, IQOS_CONG_STATUS_LO);
    }
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Delay is only needed when the free is actually being done by the FAP */
    if(bcmxtmrt_in_init_dev)
       udelay(20);
#endif

    spin_unlock_bh(&pGi->xtmlock_rx);
}

/***************************************************************************
 * Function Name: FlushAssignRxBuffer
 * Description  : Flush then assign RxBdInfo to the receive BD ring.
 * Returns      : None.
 ***************************************************************************/
void FlushAssignRxBuffer(int channel, UINT8 *pucData, UINT8 *pucEnd)
{
    cache_flush_region(pucData, pucEnd);
    AssignRxBuffer(channel, pucData);
}

/***************************************************************************
 * Function Name: bcmxtmrt_recycle_skb_or_data
 * Description  : Put socket buffer header back onto the free list or a data
 *                buffer back on to the BD ring.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_recycle_skb_or_data(struct sk_buff *skb, unsigned context,
    UINT32 nFlag )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    int channel = RECYCLE_CONTEXT(context)->channel;

    if( nFlag & SKB_RECYCLE )
    {
        BcmXtm_RxDma *rxdma = pGi->rxdma[channel];

        spin_lock_bh(&pGi->xtmlock_rx);
        skb->next_free = rxdma->freeSkbList;
        rxdma->freeSkbList = skb;
        spin_unlock_bh(&pGi->xtmlock_rx);
    }
    else
    {

        UINT8 *pucData, *pucEnd;

        pucData = skb->head + RXBUF_HEAD_RESERVE;
#ifdef XTM_CACHE_SMARTFLUSH
        pucEnd = (UINT8*)(skb_shinfo(skb)) + sizeof(struct skb_shared_info);
#else
        pucEnd = pucData + RXBUF_SIZE;
#endif
        FlushAssignRxBuffer(channel, pucData, pucEnd);
    }
} /* bcmxtmrt_recycle_skb_or_data */

/***************************************************************************
 * Function Name: _bcmxtmrt_recycle_fkb
 * Description  : Put fkb buffer back on to the BD ring.
 * Returns      : None.
 ***************************************************************************/
static inline void _bcmxtmrt_recycle_fkb(struct fkbuff *pFkb,
                                              unsigned context)
{
    UINT8 *pucData = (UINT8*) PFKBUFF_TO_PDATA(pFkb, RXBUF_HEAD_RESERVE);
    int channel = FKB_RECYCLE_CONTEXT(pFkb)->channel;

    AssignRxBuffer(channel, pucData); /* No cache flush */
} /* _bcmxtmrt_recycle_fkb */

/***************************************************************************
 * Function Name: bcmxtmrt_recycle
 * Description  : Recycle a fkb or skb or skb->data
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_recycle(pNBuff_t pNBuff, unsigned context, UINT32 flags)
{
    if (IS_FKBUFF_PTR(pNBuff))
        _bcmxtmrt_recycle_fkb( PNBUFF_2_FKBUFF(pNBuff), context );
    else // if (IS_SKBUFF_PTR(pNBuff))
        bcmxtmrt_recycle_skb_or_data( PNBUFF_2_SKBUFF(pNBuff), context, flags );
}

/***************************************************************************
 * Function Name: bcmxtmrt_rxisr
 * Description  : Hardware interrupt that is called when a packet is received
 *                on one of the receive queues.
 * Returns      : IRQ_HANDLED
 ***************************************************************************/
static FN_HANDLER_RT bcmxtmrt_rxisr(int nIrq, void *pRxDma)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    UINT32 i;
    UINT32 ulScheduled = 0;
    int    channel;
    BcmXtm_RxDma *rxdma;

    channel = CONTEXT_TO_CHANNEL((uint32)pRxDma);
    rxdma = pGi->rxdma[channel];

    spin_lock(&pGi->xtmlock_rx_regs);

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        if( (pDevCtx = pGi->pDevCtxs[i]) != NULL &&
            pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
        {
            /* Device is open.  Schedule the poll function. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
            napi_schedule(&pDevCtx->napi);
#else
            netif_rx_schedule(pDevCtx->pDev);
#endif

            bcmPktDma_XtmClrRxIrq(&rxdma->pktDmaRxInfo);
            pGi->ulIntEnableMask |= 1 << channel;
            ulScheduled = 1;
        }
    }

    if( ulScheduled == 0 && pGi->ulDrvState == XTMRT_RUNNING )
    {
        /* Device is not open.  Reenable interrupt. */
        bcmPktDma_XtmClrRxIrq(&rxdma->pktDmaRxInfo);
#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
        BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + channel);
#endif
    }

    spin_unlock(&pGi->xtmlock_rx_regs);

    return( IRQ_HANDLED );
} /* bcmxtmrt_rxisr */

/***************************************************************************
 * Function Name: bcmxtmrt_poll
 * Description  : Hardware interrupt that is called when a packet is received
 *                on one of the receive queues.
 * Returns      : IRQ_HANDLED
 ***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

static int bcmxtmrt_poll_napi(struct napi_struct* napi, int budget)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 ulMask;
    UINT32 i;
    UINT32 work_done;
    UINT32 ret_done;
    UINT32 flags;
    UINT32 more_to_do;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    BcmXtm_RxDma *rxdma = pGi->rxdma[0];
#endif

    spin_lock_irqsave(&pGi->xtmlock_rx_regs, flags);
    ulMask = pGi->ulIntEnableMask;
    pGi->ulIntEnableMask = 0;
    spin_unlock_irqrestore(&pGi->xtmlock_rx_regs,flags);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    if (iqos_enable_g)
    {
        for(i=0; i < MAX_RECEIVE_QUEUES; i++)
        {
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
                xtm_iq_dqm_update_cong_status(i);
#endif
                xtm_iq_update_cong_status(i);
        }
    }
#endif

    work_done = bcmxtmrt_rxtask(budget, &more_to_do);
    ret_done = work_done & XTM_POLL_DONE;
    work_done &= ~XTM_POLL_DONE;

    /* JU: You may not call napi_complete if work_done == budget...
       this causes the framework to crash (specifically, you get
       napi->poll_list.next=0x00100100).  So, in this case
       you have to just return work_done.  */
    if (work_done == budget || ret_done != XTM_POLL_DONE)
    {
        /* We have either exhausted our budget or there are
           more packets on the DMA (or both) */
        spin_lock_irqsave(&pGi->xtmlock_rx_regs, flags);
        pGi->ulIntEnableMask |= ulMask;
        spin_unlock_irqrestore(&pGi->xtmlock_rx_regs,flags);
        return work_done;
    }

    /* We are done */
    napi_complete(napi);

    /* Renable interrupts. */
    if( pGi->ulDrvState == XTMRT_RUNNING )
    for( i = 0; ulMask && i < MAX_RECEIVE_QUEUES; i++, ulMask >>= 1 )
        if( (ulMask & 0x01) == 0x01 )
        {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
            bcmPktDma_XtmClrRxIrq(&rxdma->pktDmaRxInfo);
#else
            BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);
#endif
        }

    return work_done;
}
#else
static int bcmxtmrt_poll(struct net_device * dev, int * budget)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 ulMask;
    UINT32 i;
    UINT32 work_to_do = min(dev->quota, *budget);
    UINT32 work_done;
    UINT32 ret_done;
    UINT32 more_to_do = 0;
    UINT32 flags;
    BcmXtm_RxDma *rxdma = pGi->rxdma[0];

    spin_lock_irqsave(&pGi->xtmlock_rx_regs, flags);
    ulMask = pGi->ulIntEnableMask;
    pGi->ulIntEnableMask = 0;
    spin_unlock_irqrestore(&pGi->xtmlock_rx_regs,flags);

    work_done = bcmxtmrt_rxtask(work_to_do, &more_to_do);
    ret_done = work_done & XTM_POLL_DONE;
    work_done &= ~XTM_POLL_DONE;

    *budget -= work_done;
    dev->quota -= work_done;

    /* JU I think it should be (work_done >= work_to_do).... */
    if (work_done < work_to_do && ret_done != XTM_POLL_DONE)
    {
        /* Did as much as could, but we are not done yet */
        spin_lock_irqsave(&pGi->xtmlock_rx_regs, flags);
        pGi->ulIntEnableMask |= ulMask;
        spin_unlock_irqrestore(&pGi->xtmlock_rx_regs,flags);
        return 1;
    }

    /* We are done */
    netif_rx_complete(dev);

    /* Renable interrupts. */
    if( pGi->ulDrvState == XTMRT_RUNNING )
    for( i = 0; ulMask && i < MAX_RECEIVE_QUEUES; i++, ulMask >>= 1 )
        if( (ulMask & 0x01) == 0x01 )
        {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
            bcmPktDma_XtmClrRxIrq(&rxdma->pktDmaRxInfo);
#else
            BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);
#endif
        }

    return 0;
} /* bcmxtmrt_poll */
#endif

/***************************************************************************
 * Function Name: bcmxtmrt_rxtask
 * Description  : Linux Tasklet that processes received packets.
 * Returns      : None.
 ***************************************************************************/
static UINT32 bcmxtmrt_rxtask( UINT32 ulBudget, UINT32 *pulMoreToDo )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 ulMoreToReceive;
    UINT32 i;
    BcmXtm_RxDma       *rxdma;
    DmaDesc dmaDesc;
    UINT8 *pBuf, *pucData;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    int  len;
    struct sk_buff *skb ;
    unsigned long flags;

    UINT32 ulRxPktGood = 0;
    UINT32 ulRxPktProcessed = 0;
    UINT32 ulRxPktMax = ulBudget + (ulBudget / 2);

    /* Receive packets from every receive queue in a round robin order until
     * there are no more packets to receive.
     */
    do
    {
        ulMoreToReceive = 0;

        for  (i = 0;  i < MAX_RECEIVE_QUEUES; i++)
        {
            UINT32   ulCell;

            rxdma = pGi->rxdma[i];

            if( ulBudget == 0 )
            {
                *pulMoreToDo = 1;
                break;
            }

            spin_lock_bh(&pGi->xtmlock_rx);


            dmaDesc.word0 = bcmPktDma_XtmRecv(&rxdma->pktDmaRxInfo, (unsigned char **)&pucData, &len);
            if( dmaDesc.status & DMA_OWN )
            {
                ulRxPktGood |= XTM_POLL_DONE;
                spin_unlock_bh(&pGi->xtmlock_rx);

                continue;   /* next RxBdInfos */
            }
            pBuf = pucData ;

            ulRxPktProcessed++;
            pDevCtx = pGi->pDevCtxsByMatchId[dmaDesc.status & FSTAT_MATCH_ID_MASK];
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
            //alloc a new buf from bpm
            if (xtm_bpm_alloc_buf( rxdma ) == GBPM_ERROR)
            {
                spin_unlock_bh(&pGi->xtmlock_rx);
                FlushAssignRxBuffer(rxdma->channel, pBuf, pBuf);
                if( pDevCtx )
                    pDevCtx->DevStats.rx_errors++;
            };
#endif
#endif
            ulCell  = (dmaDesc.status & FSTAT_PACKET_CELL_MASK) == FSTAT_CELL;

            /* error status, or packet with no pDev */
            if(((dmaDesc.status & FSTAT_ERROR) != 0) ||
               ((dmaDesc.status & (DMA_SOP|DMA_EOP)) != (DMA_SOP|DMA_EOP)) ||
                ((!ulCell) && (pDevCtx == NULL)))   /* packet */
            {
                spin_unlock_bh(&pGi->xtmlock_rx);
                if( (dmaDesc.status & FSTAT_MATCH_ID_MASK) == TEQ_DATA_VCID &&
                    pGi->pTeqNetDev )
                {
#ifdef XTM_CACHE_SMARTFLUSH
                    int len = dmaDesc.length + SAR_DMA_MAX_BURST_LENGTH;
#else
                    int len = RXBUF_SIZE;
#endif
                    skb = rxdma->freeSkbList;
                    rxdma->freeSkbList = rxdma->freeSkbList->next_free;
                    skb_headerinit( RXBUF_HEAD_RESERVE, len, skb, pBuf,
                        (RecycleFuncP)bcmxtmrt_recycle_skb_or_data, 0, NULL);
                    __skb_trim(skb, dmaDesc.length);

                    // Sending TEQ data to interface told to us by DSL Diags
                    skb->dev = pGi->pTeqNetDev;
                    skb->protocol = htons(ETH_P_802_3);
                    local_irq_save(flags);
                    local_irq_enable();
                    dev_queue_xmit(skb);
		    printk ("bcmxtmrt : TEQ data \n") ;
                    local_irq_restore(flags);
                }
                else
                {
                    AssignRxBuffer(i, pBuf);
                    if( pDevCtx )
                        pDevCtx->DevStats.rx_errors++;
                }
            }
            else if ( !ulCell ) /* process packet, pDev != NULL */
            {
                FkBuff_t * pFkb;
                UINT16 usLength = dmaDesc.length;
                int delLen = 0, trailerDelLen = 0;

                ulRxPktGood++;
                ulBudget--;

                if( (pDevCtx->ulFlags & LSC_RAW_ENET_MODE) != 0 )
                    usLength -= 4; /* ETH CRC Len */

                if ( pDevCtx->ulHdrType == HT_PTM &&
                    (pDevCtx->ulFlags & CNI_HW_REMOVE_TRAILER) == 0 )
                {
                   if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {
                      if (usLength > XTMRT_PTM_CRC_SIZE) {
                        usLength -= XTMRT_PTM_CRC_SIZE;
                                trailerDelLen = XTMRT_PTM_CRC_SIZE ;
                             }
                   }
                   else {
                      if (usLength > (ETH_FCS_LEN+XTMRT_PTM_CRC_SIZE)) {
                        usLength -= (ETH_FCS_LEN+XTMRT_PTM_CRC_SIZE);
                                trailerDelLen = (ETH_FCS_LEN+XTMRT_PTM_CRC_SIZE) ;
                             }
                   }
                }

                if( (pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0 )
                {
                   delLen = HT_LEN(pDevCtx->ulHdrType);

                   /* For PTM flow, this will not take effect and hence so, for
                    * bonding flow as well. So we do not need checks here to not
                    * make it happen.
                    */
                    if( delLen > 0)
                    {
                        pucData += delLen;
                        usLength -= delLen;
                    }
                }

                if( usLength < ETH_ZLEN )
                    usLength = ETH_ZLEN;

                pFkb = fkb_qinit(pBuf, RXBUF_HEAD_RESERVE, pucData, usLength,
                                 (uint32_t)rxdma);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
                pFkb->recycle_hook = (RecycleFuncP)bcmxtmrt_recycle;
#endif

                if (pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {
                    spin_unlock_bh (&pGi->xtmlock_rx) ;
                    spin_lock_bh (&pGi->xtmlock_rx_bond) ;
                    bcmxtmrt_ptmbond_receive_rx_fragment (pDevCtx, pFkb, dmaDesc.status, (UINT16) i) ;
                    spin_unlock_bh (&pGi->xtmlock_rx_bond);
                }
                else {
                    bcmxtmrt_process_rx_pkt (pDevCtx, rxdma, pFkb, dmaDesc.status, delLen, trailerDelLen, 
                                             FALSE) ;
                    //bcmxtmrt_ptm_receive_and_drop (pDevCtx, pFkb, dmaDesc.status, (UINT16) i) ;
                                         /* Used for SAR-phy Lpbk high-rate test */
                }
            }
            else                /* process cell */
            {
                spin_unlock_bh(&pGi->xtmlock_rx);
                ProcessRxCell(pGi, rxdma, pucData);
            }

            if( ulRxPktProcessed >= ulRxPktMax )
                break;
            else
                ulMoreToReceive = 1; /* more packets to receive on Rx queue? */

        } /* For loop */

    } while( ulMoreToReceive );

    return( ulRxPktGood );

} /* bcmxtmrt_rxtask */


/***************************************************************************
 * Function Name: bcmxtmrt_process_rx_pkt
 * Description  : Processes a received packet.
 *                Responsible to send the packet up to the blog and network stack.
 * Returns      : Status as the packet thro BLOG/NORMAL path.
 ***************************************************************************/

UINT32 bcmxtmrt_process_rx_pkt ( PBCMXTMRT_DEV_CONTEXT pDevCtx, BcmXtm_RxDma *rxdma,
                                 FkBuff_t *pFkb, UINT16 bufStatus, int delLen, int trailerDelLen,
                                 BOOLEAN throttle)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   struct sk_buff *skb ;
   UINT8  *pucData  = pFkb->data ;
   UINT32 ulHdrType = pDevCtx->ulHdrType ;
   UINT32 ulRfc2684_type = RFC2684_NONE; /* blog.h: Rfc2684_t */
   UINT32 blogPhyType;
   UINT8  *pBuf = PFKBUFF_TO_PDATA(pFkb, RXBUF_HEAD_RESERVE) ;
   UINT32 retStatus;
#ifdef CONFIG_BLOG
   BlogAction_t blogAction;
#endif

   pDevCtx->pDev->last_rx = jiffies;

   pDevCtx->DevStats.rx_bytes += pFkb->len ;

   if( (ulHdrType ==  HT_PTM || ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
        ulHdrType ==  HT_VC_MUX_ETHERNET)
                     &&
                    ((pucData[0] & 0x01) == 0x01) )
                {
                    pDevCtx->DevStats.multicast++;
#if defined(AEI_VDSL_STATS_DIAG)
                    pDevCtx->multi_stats.rx_multicast_bytes += pFkb->len;
#endif
                }

   if( (pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0 )
   {  /* cannot be an AtmCell, also do not use delLen (bonding), recompute */
      if( HT_LEN(ulHdrType) > 0)
         ulRfc2684_type = HT_TYPE(ulHdrType); /* blog.h: Rfc2684_t */
   }

#if !defined(AEI_VDSL_TOOLBOX)
   if( pDevCtx->szMirrorIntfIn[0] != '\0' &&
         (ulHdrType ==  HT_PTM ||
          ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
          ulHdrType ==  HT_VC_MUX_ETHERNET) )
   {
      struct sk_buff *skb_m;
      FkBuff_t *fkbC_p;

      fkbC_p = fkb_clone( pFkb );
      skb_m = nbuff_xlate( FKBUFF_2_PNBUFF(fkbC_p) );    /* translate to skb */
      if (skb_m != (struct sk_buff *)NULL)
      {
         spin_unlock_bh(&pGi->xtmlock_rx);
         MirrorPacket(skb_m, pDevCtx->szMirrorIntfOut, 0);
         dev_kfree_skb_any( skb_m );
         spin_lock_bh(&pGi->xtmlock_rx);
      }
   }
#endif

   blogPhyType = BLOG_SET_PHYHDR(ulRfc2684_type, BLOG_XTMPHY);
#ifdef CONFIG_BLOG
   spin_unlock_bh(&pGi->xtmlock_rx);
   blogAction = blog_finit( pFkb, pDevCtx->pDev, pDevCtx->ulEncapType,
               (bufStatus & FSTAT_MATCH_ID_MASK), blogPhyType );

   if (blogAction == PKT_DONE )
   {
        retStatus = PACKET_BLOG ;
   }
   else {

   if ( blogAction == PKT_DROP)
   {
       FlushAssignRxBuffer(rxdma->channel, pBuf, pBuf + pFkb->len);
       pDevCtx->DevStats.rx_dropped++;
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CC_IQ_STATS)
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
       rxdma->pktDmaRxInfo.iqDroppedDqm++;
#else
       rxdma->pktDmaRxInfo.iqDropped++;
#endif
#endif
#endif
       return PACKET_NORMAL;
   }

   spin_lock_bh(&pGi->xtmlock_rx);
#else
   {
#endif

        pGi->noOfNonAccelPktsDSInSec++ ;

        if (((throttle == TRUE) && (pGi->noOfNonAccelPktsDSInSec > MAX_NON_ACCEL_PKTS_IN_SEC))
              ||
            ( rxdma->freeSkbList == NULL ))
        {
           spin_unlock_bh(&pGi->xtmlock_rx);

           fkb_release(pFkb);  /* releases allocated blog */
           FlushAssignRxBuffer(rxdma->channel, pBuf, pBuf);
           pDevCtx->DevStats.rx_dropped++;
           if (throttle == TRUE) 
               return PACKET_DROP;
           else
           return PACKET_NORMAL;
        }

        /* Get an skb to return to the network stack. */
        skb = rxdma->freeSkbList;
        rxdma->freeSkbList = rxdma->freeSkbList->next_free;

        spin_unlock_bh(&pGi->xtmlock_rx);
        BCM_XTM_RX_DEBUG("XTM RX SKB: skb<0x%08x> pBuf<0x%08x> len<%d>\n", (int)skb, (int)pBuf, pFkb->len);

        {
            uint32 recycle_context = 0;

            RECYCLE_CONTEXT(recycle_context)->channel = rxdma->channel;

            skb_headerinit( RXBUF_HEAD_RESERVE,
#ifdef XTM_CACHE_SMARTFLUSH
                            pFkb->len + delLen + trailerDelLen + SAR_DMA_MAX_BURST_LENGTH,
#else
                            RXBUF_SIZE,
#endif
                            skb, pBuf,
                            (RecycleFuncP)bcmxtmrt_recycle_skb_or_data,
                            recycle_context, (void*)fkb_blog(pFkb));
        }

        if ( delLen )
             __skb_pull(skb, delLen);

        __skb_trim(skb, pFkb->len);
        skb->dev = pDevCtx->pDev ;

#if defined(AEI_VDSL_TOOLBOX)
       if ( pDevCtx->usMirrorInFlags != 0 &&
            (ulHdrType ==  HT_PTM ||
             ulHdrType ==  HT_LLC_SNAP_ETHERNET ||
             ulHdrType ==  HT_VC_MUX_ETHERNET) )
       {
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
           if (pDevCtx->matchVlanId >= 0)
           {
               if (pDevCtx->matchVlanId == 0)
               {
                   if (!AEI_hasVlanTag(skb))
                   {
                       AEI_MultiMirrorPacket( skb, pDevCtx->usMirrorInFlags );
                   }
               }
               else
               {
                   if (AEI_matchVlanId(skb, pDevCtx->matchVlanId))
                   {
                       AEI_MultiMirrorPacket( skb, pDevCtx->usMirrorInFlags );
                   }
               }
           }
           else
           {
               AEI_MultiMirrorPacket( skb, pDevCtx->usMirrorInFlags );
           }
#else
           AEI_MultiMirrorPacket( skb, pDevCtx->usMirrorInFlags );
#endif
       }
#endif /* AEI_VDSL_TOOLBOX */

        switch( ulHdrType )
        {
            case HT_LLC_SNAP_ROUTE_IP:
            case HT_VC_MUX_IPOA:
                /* IPoA */
                skb->protocol = htons(ETH_P_IP);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
                skb_reset_mac_header(skb);
#else
                skb->mac.raw = skb->data;
#endif
                /* Give the received packet to the network stack. */
                netif_receive_skb(skb);
                break;

        case HT_LLC_ENCAPS_PPP:
        case HT_VC_MUX_PPPOA:
                /*PPPoA*/
                ppp_input(&pDevCtx->Chan, skb);
                break;

        default:
                /* bridge, MER, PPPoE */
                skb->protocol = eth_type_trans(skb,pDevCtx->pDev);

                /* Give the received packet to the network stack. */
                netif_receive_skb(skb);
                break;
        }

        retStatus = PACKET_NORMAL;
    }
    return (retStatus) ;
}

/***************************************************************************
 * Function Name: ProcessRxCell
 * Description  : Processes a received cell.
 * Returns      : None.
 ***************************************************************************/
static void ProcessRxCell(PBCMXTMRT_GLOBAL_INFO pGi, BcmXtm_RxDma *rxdma,
    UINT8 *pucData )
{
    const UINT16 usOamF4VciSeg = 3;
    const UINT16 usOamF4VciEnd = 4;
    UINT8 ucCts[] = {0, 0, 0, 0, CTYPE_OAM_F5_SEGMENT, CTYPE_OAM_F5_END_TO_END,
        0, 0, CTYPE_ASM_P0, CTYPE_ASM_P1, CTYPE_ASM_P2, CTYPE_ASM_P3,
        CTYPE_OAM_F4_SEGMENT, CTYPE_OAM_F4_END_TO_END};
    XTMRT_CELL Cell;
    UINT8 ucCHdr = *pucData;
    UINT8 *pucAtmHdr = pucData + sizeof(char);
    UINT8 ucLogPort;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;

    /* Fill in the XTMRT_CELL structure */
    Cell.ConnAddr.u.Vcc.usVpi = (((UINT16) pucAtmHdr[0] << 8) +
        ((UINT16) pucAtmHdr[1])) >> 4;
    Cell.ConnAddr.u.Vcc.usVci = (UINT16)
        (((UINT32) (pucAtmHdr[1] & 0x0f) << 16) +
         ((UINT32) pucAtmHdr[2] << 8) +
         ((UINT32) pucAtmHdr[3])) >> 4;

    if ((Cell.ConnAddr.u.Vcc.usVpi == XTMRT_ATM_BOND_ASM_VPI)
             &&
        (Cell.ConnAddr.u.Vcc.usVci == XTMRT_ATM_BOND_ASM_VCI)) {

       pDevCtx = pGi->pDevCtxs[0];
    }
    else {
       Cell.ConnAddr.ulTrafficType = TRAFFIC_TYPE_ATM;
       ucLogPort = PORT_PHYS_TO_LOG((ucCHdr & CHDR_PORT_MASK) >> CHDR_PORT_SHIFT);
       Cell.ConnAddr.u.Vcc.ulPortMask = PORT_TO_PORTID(ucLogPort);

    if( Cell.ConnAddr.u.Vcc.usVci == usOamF4VciSeg )
    {
        ucCHdr = CHDR_CT_OAM_F4_SEG;
        pDevCtx = pGi->pDevCtxs[0];
    }
    else
        if( Cell.ConnAddr.u.Vcc.usVci == usOamF4VciEnd )
        {
            ucCHdr = CHDR_CT_OAM_F4_E2E;
            pDevCtx = pGi->pDevCtxs[0];
        }
        else
        {
            pDevCtx = FindDevCtx( (short) Cell.ConnAddr.u.Vcc.usVpi,
                (int) Cell.ConnAddr.u.Vcc.usVci);
        }
    } /* End of else */

    Cell.ucCircuitType = ucCts[(ucCHdr & CHDR_CT_MASK) >> CHDR_CT_SHIFT];

    if( (ucCHdr & CHDR_ERROR) == 0 )
    {
        memcpy(Cell.ucData, pucData + sizeof(char), sizeof(Cell.ucData));

        /* Call the registered OAM or ASM callback function. */
        switch( ucCHdr & CHDR_CT_MASK )
        {
        case CHDR_CT_OAM_F5_SEG:
        case CHDR_CT_OAM_F5_E2E:
        case CHDR_CT_OAM_F4_SEG:
        case CHDR_CT_OAM_F4_E2E:
            if( pGi->pfnOamHandler && pDevCtx )
            {
                (*pGi->pfnOamHandler) ((XTMRT_HANDLE)pDevCtx,
                    XTMRTCB_CMD_CELL_RECEIVED, &Cell,
                    pGi->pOamContext);
            }
            break;

        case CHDR_CT_ASM_P0:
        case CHDR_CT_ASM_P1:
        case CHDR_CT_ASM_P2:
        case CHDR_CT_ASM_P3:
            if( pGi->pfnAsmHandler && pDevCtx )
            {
                (*pGi->pfnAsmHandler) ((XTMRT_HANDLE)pDevCtx,
                    XTMRTCB_CMD_CELL_RECEIVED, &Cell,
                    pGi->pAsmContext);
            }
            break;

        default:
            break;
        }
    }
    else
        if( pDevCtx )
            pDevCtx->DevStats.rx_errors++;

    /* Put the buffer back onto the BD ring. */
    FlushAssignRxBuffer(rxdma->channel, pucData, pucData + RXBUF_SIZE);

} /* ProcessRxCell */

#if defined(AEI_VDSL_TOOLBOX)
static char * pMirrorIntfNames[] =
{
    "eth0",
    "eth1",
    "eth2",
    "eth3",
    "eth4", /* for HPNA */
    "wl0",
    "wl0.1",
    "wl0.2",
    "wl0.3",
    NULL
};

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
#define AEI_VLAN_VID_MASK    0x0FFF

static UBOOL8 AEI_hasVlanTag(struct sk_buff *skb)
{
    UBOOL8 hasVlan = FALSE;
    struct vlan_ethhdr *veth = (struct vlan_ethhdr *)skb->data;

    if (veth && (veth->h_vlan_proto == htons(ETH_P_8021Q)))
    {
        hasVlan = TRUE;
    }

    return hasVlan;
}

static UBOOL8 AEI_matchVlanId(struct sk_buff *skb, int vlanId)
{
    struct vlan_ethhdr *veth;
    UBOOL8 match = FALSE;

    if (AEI_hasVlanTag(skb))
    {
        veth = (struct vlan_ethhdr *)skb->data;
        if (veth && ((veth->h_vlan_TCI & AEI_VLAN_VID_MASK) == vlanId))
        {
            match = TRUE;
        }
    }

    return match;
}
#endif

static void AEI_UpdateMirrorFlag(UINT16 *mirFlags, char *intfName, UINT8 enable)
{
    UINT8 i;
    UINT16 flag;

    for (i = 0; pMirrorIntfNames[i]; i++)
    {
        if (strcmp(pMirrorIntfNames[i], intfName) == 0)
        {
            flag = 0x0001 << i;

            if (enable)
                (*mirFlags) |= flag;
            else
                (*mirFlags) &= ~flag;

            break;
        }
    }
}

static void AEI_MirrorPacket( struct sk_buff *skb, char *intfName )
{
    struct sk_buff *skbCopy = NULL;
    struct net_device *netDev;
    int headroom, tailroom;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    if( (netDev = __dev_get_by_name(&init_net, intfName)) != NULL )
#else
    if( (netDev = __dev_get_by_name(intfName)) != NULL )
#endif
    {
        if (strstr(intfName, "eth"))
        {
            headroom = BRCM_TAG_TYPE2_LEN;
            tailroom = ETH_ZLEN - skb->len;

            if (tailroom < 0)
                tailroom = 0;

            if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
            {
                skbCopy = skb_copy_expand(skb, headroom, tailroom, GFP_ATOMIC);
            }
        }

        if (skbCopy == NULL)
            skbCopy = skb_copy(skb, GFP_ATOMIC);

        if (skbCopy)
        {
            unsigned long flags;

            skbCopy->dev = netDev;
            skbCopy->blog_p = BLOG_NULL;
            if (strstr(intfName, "wl"))
                skbCopy->protocol = htons(ETH_P_MIRROR_WLAN);
            else
                skbCopy->protocol = htons(ETH_P_MIRROR);
            skb_set_network_header(skbCopy, ETH_HLEN);
            skb_reset_mac_header(skbCopy);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(skbCopy) ;
            local_irq_restore(flags);
        }
    }
}

static void AEI_MultiMirrorPacket(struct sk_buff *skb, UINT16 mirFlags)
{
    int i;
    UINT32 flag;

    if (mirFlags == 0)
        return;

    for (i = 0; pMirrorIntfNames[i]; i++)
    {
        flag = 0x0001 << i;

        if (!(mirFlags & flag))
            continue;

        AEI_MirrorPacket( skb, pMirrorIntfNames[i] );
    }
}
#else
/***************************************************************************
 * Function Name: MirrorPacket
 * Description  : This function sends a sent or received packet to a LAN port.
 *                The purpose is to allow packets sent and received on the WAN
 *                to be captured by a protocol analyzer on the Lan for debugging
 *                purposes.
 * Returns      : None.
 ***************************************************************************/
static void MirrorPacket(struct sk_buff *skb, char *intfName, int need_unshare)
{
    struct sk_buff *skb2;
    struct net_device *netDev;

    if ( need_unshare )
        skb2 = skb_copy( skb, GFP_ATOMIC );
    else
        skb2 = skb_clone( skb, GFP_ATOMIC );

    if( skb2 != NULL )
    {
        blog_xfer(skb, skb2);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        if( (netDev = __dev_get_by_name(&init_net, intfName)) != NULL )
#else
        if( (netDev = __dev_get_by_name(intfName)) != NULL )
#endif
        {
            unsigned long flags;

            skb2->dev = netDev;
            skb2->protocol = htons(ETH_P_802_3);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(skb2) ;
            local_irq_restore(flags);
        }
        else
            dev_kfree_skb(skb2);
    }
} /* MirrorPacket */
#endif

/***************************************************************************
 * Function Name: bcmxtmrt_timer
 * Description  : Periodic timer that calls the send function to free packets
 *                that have been transmitted.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_timer( PBCMXTMRT_GLOBAL_INFO pGi )
{
    UINT32 i;
    UINT32 ulHdrTypeIsPtm = FALSE;
    PBCMXTMRT_DEV_CONTEXT pDevCtx, pPtmDevCtx=NULL;
    unsigned long newThrottleTimeout = pGi->prevTime + RX_THROTTLE_TIMEOUT ;

    if (time_after (jiffies, newThrottleTimeout)) {
       pGi->prevTime = jiffies ;
       pGi->noOfNonAccelPktsDSInSec = 0 ;
    }

    /* Free transmitted buffers. */
    for( i = 0; i < MAX_DEV_CTXS; i++ ) {
        if( (pDevCtx = pGi->pDevCtxs[i]) ) {
            if( pDevCtx->ulTxQInfosSize )
            {
                bcmxtmrt_xmit( PNBUFF_NULL, pGi->pDevCtxs[i]->pDev );
            }
            if (pGi->pDevCtxs[i]->ulHdrType == HT_PTM) {
               ulHdrTypeIsPtm = TRUE ;
               pPtmDevCtx = pGi->pDevCtxs[i] ;
        }
        } /* if (pDevCtx) */
    }

    if ((pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
          &&
        (ulHdrTypeIsPtm == TRUE)) {
       if (pGi->ptmBondInfo.tickCount >= MAX_TICK_COUNT) {
          spin_lock(&pGi->xtmlock_rx_bond);
          bcmxtmrt_ptmbond_tick (pPtmDevCtx, &pGi->ptmBondInfo, 1) ;
          spin_unlock(&pGi->xtmlock_rx_bond);
          pGi->ptmBondInfo.tickCount = 0 ;
       }
       else
          pGi->ptmBondInfo.tickCount++ ;
    }

    if( pGi->pTeqNetDev && ((void *)pGi->ulDevCtxMask == NULL) )
    {
        UINT32 ulNotUsed;
        bcmxtmrt_rxtask( 100, &ulNotUsed );
    }

    /* Restart the timer. */
    pGi->Timer.expires = jiffies + SAR_TIMEOUT;
    add_timer(&pGi->Timer);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Add code for buffer quick free between enet and xtm - June 2010 */
    if(enet_recycle_hook == NULL)
        enet_recycle_hook = bcmPktDma_get_enet_recycle();
#endif

} /* bcmxtmrt_timer */


/***************************************************************************
 * Function Name: bcmxtmrt_request
 * Description  : Request from the bcmxtmcfg driver.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int bcmxtmrt_request( XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParm )
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx = (PBCMXTMRT_DEV_CONTEXT) hDev;
    int nRet = 0;

    switch( ulCommand )
    {
    case XTMRT_CMD_GLOBAL_INITIALIZATION:
        nRet = DoGlobInitReq( (PXTMRT_GLOBAL_INIT_PARMS) pParm );
        break;

    case XTMRT_CMD_GLOBAL_UNINITIALIZATION:
        nRet = DoGlobUninitReq();
        break;

    case XTMRT_CMD_CREATE_DEVICE:
        nRet = DoCreateDeviceReq( (PXTMRT_CREATE_NETWORK_DEVICE) pParm );
        break;

    case XTMRT_CMD_GET_DEVICE_STATE:
        *(UINT32 *) pParm = pDevCtx->ulOpenState;
        break;

    case XTMRT_CMD_SET_ADMIN_STATUS:
        pDevCtx->ulAdminStatus = (UINT32) pParm;
        break;

    case XTMRT_CMD_REGISTER_CELL_HANDLER:
        nRet = DoRegCellHdlrReq( (PXTMRT_CELL_HDLR) pParm );
        break;

    case XTMRT_CMD_UNREGISTER_CELL_HANDLER:
        nRet = DoUnregCellHdlrReq( (PXTMRT_CELL_HDLR) pParm );
        break;

    case XTMRT_CMD_LINK_STATUS_CHANGED:
        nRet = DoLinkStsChangedReq(pDevCtx, (PXTMRT_LINK_STATUS_CHANGE)pParm);
        break;

    case XTMRT_CMD_SEND_CELL:
        nRet = DoSendCellReq( pDevCtx, (PXTMRT_CELL) pParm );
        break;

    case XTMRT_CMD_DELETE_DEVICE:
        nRet = DoDeleteDeviceReq( pDevCtx );
        break;

    case XTMRT_CMD_SET_TX_QUEUE:
        {

         PXTMRT_TRANSMIT_QUEUE_ID pTxQId  = (PXTMRT_TRANSMIT_QUEUE_ID) pParm ;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
         xtm_bpm_txq_thresh(pDevCtx, pTxQId, pTxQId->ulQueueIndex);
#endif

         if (pTxQId->ulBondingPortId != PORT_PHY_INVALID) {

            if (DoSetTxHwQueue (pDevCtx, pTxQId) != 0)
               nRet = -ENOMEM;
         }

         if (nRet == 0)
		      nRet = DoSetTxQueue( pDevCtx, pTxQId) ;

         if ((nRet == 0) && (pTxQId->ulBondingPortId != PORT_PHY_INVALID) &&
             (pDevCtx->ulTxQInfosSize == 0)) {

            DoUnsetTxHwQueues () ;
         }

        break;
       }
		 case XTMRT_CMD_UNSET_TX_QUEUE: {

          PXTMRT_TRANSMIT_QUEUE_ID pTxQId  = (PXTMRT_TRANSMIT_QUEUE_ID) pParm ;

			 nRet = DoUnsetTxQueue( pDevCtx, pTxQId) ;
          if ((nRet == 0) && (pTxQId->ulBondingPortId != PORT_PHY_INVALID) &&
                (pDevCtx->ulTxQInfosSize == 0)) {

             DoUnsetTxHwQueues () ;
          }

        break;
       }

    case XTMRT_CMD_SET_DS_SEQ_DEVIATION:
      if (g_GlobalInfo.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {
         g_GlobalInfo.ptmBondInfo.rxDsReducedDeviation = (UINT32) pParm ;
         printk ("bcmxtmrt: DS Reduced Deviation = %lu \n", g_GlobalInfo.ptmBondInfo.rxDsReducedDeviation) ;
      }
      break;

    case XTMRT_CMD_GET_NETDEV_TXCHANNEL:
        nRet = DoGetNetDevTxChannel( (PXTMRT_NETDEV_TXCHANNEL) pParm );
        break;

		 case XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE:
			 nRet = DoTogglePortDataStatusReq(pDevCtx, (PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE)pParm);
			 break;

    case XTMRT_CMD_SET_TEQ_DEVCTX:
        {
        PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
        BcmXtm_RxDma *rxdma;
        UINT i;

        pGi->pTeqNetDev = (struct net_device *) pParm;

        /* If receive interrupts are not enabled, enable them. */
        if (pGi->ulDrvState == XTMRT_INITIALIZED)
        {
            pGi->ulDrvState = XTMRT_RUNNING;

            /* Enable receive interrupts and start a timer. */
            for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
            {
                rxdma = pGi->rxdma[i];
                if (rxdma->pktDmaRxInfo.rxBds)
                {
                    /* fap RX interrupts are enabled in open */
#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
                     {
                         bcmPktDma_XtmRxEnable(&rxdma->pktDmaRxInfo);
                         BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);
                     }
#endif
                }
            }

            pGi->Timer.expires = jiffies + SAR_TIMEOUT;
            add_timer(&pGi->Timer);
        }
        }
        break;

    default:
        nRet = -EINVAL;
        break;
    }

    return( nRet );
} /* bcmxtmrt_request */


/***************************************************************************
 * Function Name: DoGlobInitReq
 * Description  : Processes an XTMRT_CMD_GLOBAL_INITIALIZATION command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoGlobInitReq( PXTMRT_GLOBAL_INIT_PARMS pGip )
{
    int ret ;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i, j = 0, k;
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    UINT32 ulSize;
#endif

    BcmXtm_RxDma         *rxdma;
    volatile DmaStateRam *StateRam;

    int nr_tx_bds = bcmPktDma_XtmGetTxBds(0);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Allocate smaller number of xtm tx BDs in FAP builds. BDs are stored in PSM - Apr 2010 */
    g_GlobalInfo.ulNumExtBufs = nr_tx_bds;   /* was NR_TX_BDS */
#else
    /* Allocate original number of xtm tx BDs in non-FAP builds. BDs are stored in DDR - Apr 2010 */
    g_GlobalInfo.ulNumExtBufs = nr_tx_bds;
#endif

    g_GlobalInfo.ulNumExtBufsRsrvd = g_GlobalInfo.ulNumExtBufs / 5;
    g_GlobalInfo.ulNumExtBufs90Pct = (g_GlobalInfo.ulNumExtBufs * 9) / 10;
    g_GlobalInfo.ulNumExtBufs50Pct = g_GlobalInfo.ulNumExtBufs / 2;


    if( pGi->ulDrvState != XTMRT_UNINITIALIZED )
    {
        return -EPERM;
    }

    bcmxtmrt_in_init_dev = 1;

    bcmLog_setLogLevel(BCM_LOG_ID_XTM, BCM_LOG_LEVEL_ERROR);

    spin_lock_init(&pGi->xtmlock_tx);
    spin_lock_init(&pGi->xtmlock_rx);
    spin_lock_init(&pGi->xtmlock_rx_regs);

    /* Save MIB counter/Cam registers. */
    pGi->pulMibTxOctetCountBase = pGip->pulMibTxOctetCountBase;
    pGi->ulMibRxClrOnRead = pGip->ulMibRxClrOnRead;
    pGi->pulMibRxCtrl = pGip->pulMibRxCtrl;
    pGi->pulMibRxMatch = pGip->pulMibRxMatch;
    pGi->pulMibRxOctetCount = pGip->pulMibRxOctetCount;
    pGi->pulMibRxPacketCount = pGip->pulMibRxPacketCount;
    pGi->pulRxCamBase = pGip->pulRxCamBase;

    /* allocate rxdma channel structures */
    for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
    {
        pGi->rxdma[i] = (BcmXtm_RxDma *) (kzalloc(
                               sizeof(BcmXtm_RxDma), GFP_KERNEL));

        if (pGi->rxdma[i] == NULL)
        {
            printk("Unable to allocate memory for rx dma channel structs \n");
            for(j = 0; j < i; j++)
                kfree(pGi->rxdma[j]);
            return -ENXIO;
        }
        pGi->rxdma[i]->pktDmaRxInfo.channel = i;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        pGi->rxdma[i]->pktDmaRxInfo.fapIdx = getFapIdxFromXtmRxIudma(i);
#endif
    }

    /* alloc space for the rx buffer descriptors */
    for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
    {
        rxdma = pGi->rxdma[i];

        pGip->ulReceiveQueueSizes[i] = bcmPktDma_XtmGetRxBds(i);
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
        xtm_rx_init_iq_thresh(i);
#endif

        if( pGip->ulReceiveQueueSizes[i] == 0 ) continue;

        rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_XtmAllocRxBds(i, pGip->ulReceiveQueueSizes[i]);

        if (rxdma->pktDmaRxInfo.rxBdsBase == NULL)
        {
            printk("Unable to allocate memory for Rx Descriptors \n");
            for(j = 0; j < MAX_RECEIVE_QUEUES; j++)
            {
#if !defined(XTM_RX_BDS_IN_PSM)
                if(pGi->rxdma[j]->pktDmaRxInfo.rxBdsBase)
                    kfree((void *)pGi->rxdma[j]->pktDmaRxInfo.rxBdsBase);
#endif
                kfree(pGi->rxdma[j]);
            }
            return -ENOMEM;
        }
#if defined(XTM_RX_BDS_IN_PSM)
        rxdma->pktDmaRxInfo.rxBds = rxdma->pktDmaRxInfo.rxBdsBase;
#else
        /* Align rx BDs on 16-byte boundary - Apr 2010 */
        rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)(((int)rxdma->pktDmaRxInfo.rxBdsBase + 0xF) & ~0xF);
        rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(rxdma->pktDmaRxInfo.rxBds);
#endif

        rxdma->pktDmaRxInfo.numRxBds = pGip->ulReceiveQueueSizes[i];

        printk("XTM Init: %d rx BDs at 0x%x\n", rxdma->pktDmaRxInfo.numRxBds,
            (unsigned int)rxdma->pktDmaRxInfo.rxBds);

        rxdma->rxIrq = bcmPktDma_XtmSelectRxIrq(i);
        rxdma->channel = i;
    }

    /*
     * clear RXDMA state RAM
     */
    pGi->dmaCtrl = (DmaRegs *) SAR_DMA_BASE;
    StateRam = (DmaStateRam *)&pGi->dmaCtrl->stram.s;
    memset((char *) &StateRam[SAR_RX_DMA_BASE_CHAN], 0x00,
            sizeof(DmaStateRam) * NR_SAR_RX_DMA_CHANS);

    /* setup the RX DMA channels */
    for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
    {
        BcmXtm_RxDma *rxdma;

        rxdma = pGi->rxdma[i];

        rxdma->pktDmaRxInfo.rxDma = &pGi->dmaCtrl->chcfg[SAR_RX_DMA_BASE_CHAN + i];
        rxdma->pktDmaRxInfo.rxDma->cfg = 0;

        if( pGip->ulReceiveQueueSizes[i] == 0 ) continue;

        rxdma->pktDmaRxInfo.rxDma->maxBurst = SAR_DMA_MAX_BURST_LENGTH;
        rxdma->pktDmaRxInfo.rxDma->intMask = 0;   /* mask all ints */
        rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
        rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
        pGi->dmaCtrl->stram.s[SAR_RX_DMA_BASE_CHAN + i].baseDescPtr =
            (uint32)VIRT_TO_PHY((uint32 *)rxdma->pktDmaRxInfo.rxBds);

        /* register the RX ISR */
        if (rxdma->rxIrq)
        {
            BcmHalInterruptDisable(rxdma->rxIrq);
            BcmHalMapInterrupt(bcmxtmrt_rxisr,
                              BUILD_CONTEXT(pGi,i), rxdma->rxIrq);
        }

       bcmPktDma_XtmInitRxChan(rxdma->pktDmaRxInfo.numRxBds, &rxdma->pktDmaRxInfo);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
        xtm_rx_set_iq_thresh( i );
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   bcmPktDma_XtmSetRxChanBpmThresh(&rxdma->pktDmaRxInfo,
                (rxdma->pktDmaRxInfo.numRxBds * BPM_XTM_ALLOC_TRIG_PCT/100),
                BPM_XTM_BULK_ALLOC_COUNT);

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    BCM_XTM_DEBUG( "Xtm: BPM Rx allocTrig=%d bulkAlloc=%d\n",
        (int) (rxdma->pktDmaRxInfo.allocTrig),
        (int) rxdma->pktDmaRxInfo.bulkAlloc );
#endif
#endif
    }

    /* Allocate receive socket buffers and data buffers. */
    for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
    {
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        const UINT32 ulRxAllocSize = SKB_ALIGNED_SIZE + RXBUF_ALLOC_SIZE;
        const UINT32 ulBlockSize = (64 * 1024);
        const UINT32 ulBufsPerBlock = ulBlockSize / ulRxAllocSize;
        UINT32 ulAllocAmt;
        unsigned char *pFkBuf;
        unsigned char *data;
        struct sk_buff *pSkbuff;
#endif
        uint32_t  BufsToAlloc;
        uint32 context = 0;

        rxdma = pGi->rxdma[i];
        j = 0;

        rxdma->pktDmaRxInfo.rxAssignedBds = 0;
        rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;

        BufsToAlloc = rxdma->pktDmaRxInfo.numRxBds;

        RECYCLE_CONTEXT(context)->channel = rxdma->channel;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        if( (xtm_bpm_alloc_buf_ring(rxdma, BufsToAlloc)) == GBPM_ERROR )
        {
            /* release all allocated receive buffers */
            xtm_bpm_free_buf_ring(rxdma);
            kfree(pGi->rxdma[i]);
            return -ENOMEM;
        }

        {
            int s;
            unsigned char *pSkbuff;

                if( (rxdma->skbs_p = kmalloc(
                     (rxdma->pktDmaRxInfo.numRxBds * SKB_ALIGNED_SIZE) + 0x10,
                     GFP_KERNEL)) == NULL )
                {
                    return -ENOMEM;
                }

                memset(rxdma->skbs_p, 0,
                     (rxdma->pktDmaRxInfo.numRxBds * SKB_ALIGNED_SIZE) + 0x10);

            /* Chain socket skbs */
            for (s = 0, pSkbuff = (unsigned char *)
                (((unsigned long) rxdma->skbs_p + 0x0f) & ~0x0f); s < rxdma->pktDmaRxInfo.numRxBds;
                s++, pSkbuff += SKB_ALIGNED_SIZE)
            {
                ((struct sk_buff *) pSkbuff)->next_free = rxdma->freeSkbList;
                rxdma->freeSkbList = (struct sk_buff *) pSkbuff;
            }
        }

        gbpm_resv_rx_buf( GBPM_PORT_XTM, i, rxdma->pktDmaRxInfo.numRxBds,
                (rxdma->pktDmaRxInfo.numRxBds * BPM_XTM_ALLOC_TRIG_PCT/100) );
#else
        while (BufsToAlloc)
        {
            ulAllocAmt = (ulBufsPerBlock < BufsToAlloc) ? ulBufsPerBlock : BufsToAlloc;

            ulSize = ulAllocAmt * ulRxAllocSize;
            ulSize = (ulSize + 0x0f) & ~0x0f;

            if( (j >= MAX_BUFMEM_BLOCKS) ||
                ((data = kmalloc(ulSize, GFP_KERNEL)) == NULL) )
            {
                /* release all allocated receive buffers */
                printk(KERN_NOTICE CARDNAME": Low memory.\n");
                for (k = 0; k < MAX_BUFMEM_BLOCKS; k++)
                {
                    if (rxdma->buf_pool[k]) {
                        kfree(rxdma->buf_pool[k]);
                        rxdma->buf_pool[k] = NULL;
                    }
                }
                for(k = 0; k < MAX_RECEIVE_QUEUES; k++)
                {
#if !defined(XTM_RX_BDS_IN_PSM)
                    if(pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase)
                        kfree((void *)pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase);
#endif

                    kfree(pGi->rxdma[k]);
                }
                return -ENOMEM;
            }

            rxdma->buf_pool[j++] = data;
            memset(data, 0x00, ulSize);
            cache_flush_len(data, ulSize);

            data = (UINT8 *) (((UINT32) data + 0x0f) & ~0x0f);
            for (k = 0, pFkBuf = data; k < ulAllocAmt; k++)
            {
                /* Place a FkBuff_t object at the head of pFkBuf */
                fkb_preinit(pFkBuf, (RecycleFuncP)bcmxtmrt_recycle, context);
                FlushAssignRxBuffer(i,
                                 PFKBUFF_TO_PDATA(pFkBuf, RXBUF_HEAD_RESERVE),
                                   (uint8_t*)pFkBuf + RXBUF_ALLOC_SIZE);

                /* skbuff allocation as in impl1 - Apr 2010 */
                pSkbuff = (struct sk_buff *)(pFkBuf + RXBUF_ALLOC_SIZE);
                pSkbuff->next_free = rxdma->freeSkbList;
                rxdma->freeSkbList = pSkbuff;
                pFkBuf += ulRxAllocSize;
            }
            BufsToAlloc -= ulAllocAmt;
        }
#endif
    }
    pGi->bondConfig.uConfig = pGip->bondConfig.uConfig ;
    if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
    {
      /* Initialize the swDmaCtrl area */
      pGi->swDmaCtrlBase = (DmaRegs *) kzalloc(sizeof (DmaRegs)+0x40, GFP_KERNEL) ;

      //printk ("bcmxtmrt: sw dma ctrl area %x \n", (UINT32) pGi->swDmaCtrlBase) ;

      /* Initialize the SW Scheduler */
      bcmxtmrt_sw_sched_initOrReset (pGi) ;

		/* start the HW timer for the scheduler */
		ret = bcmxtmrt_init_swsched_timer (bcmxtmrt_swscheduler, pGi, XTM_SW_SCHED_TIMER_INIT) ;

      if ((pGi->swDmaCtrlBase == NULL) || (ret != 0)) {

         /* release all allocated receive buffers */
         printk(KERN_NOTICE CARDNAME": Low memory/Sw Scheduler Failure.\n");

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
         for(i = 0; i < MAX_RECEIVE_QUEUES; i++)
         {
            xtm_bpm_free_buf_ring(pGi->rxdma[i]);
         }
#else
         for (k = 0; k < MAX_BUFMEM_BLOCKS; k++)
         {
            if (rxdma->buf_pool[k]) {
               kfree(rxdma->buf_pool[k]);
               rxdma->buf_pool[k] = NULL;
            }
         }
#endif

         for(k = 0; k < MAX_RECEIVE_QUEUES; k++)
         {
            if(pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase)
               kfree((void *)pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase);

            kfree(pGi->rxdma[k]);
         }

         if (pGi->swDmaCtrlBase != NULL)
            kfree ((void *) pGi->swDmaCtrlBase) ;
         pGi->swDmaCtrlBase = NULL ;

         return -ENOMEM;
      }

      /* Alignment on 64-byte boundary */
      pGi->swDmaCtrl = (volatile DmaRegs *)(((int)pGi->swDmaCtrlBase + 0x3F) & ~0x3F);

       bcmxtmrt_ptmbond_initialize (PTM_BOND_INITIALIZE_GLOBAL) ;
       spin_lock_init(&pGi->xtmlock_rx_bond);
    }

    /* Initialize a timer function to free transmit buffers. */
    init_timer(&pGi->Timer);
    pGi->Timer.data = (unsigned long) pGi;
    pGi->Timer.function = (void *) bcmxtmrt_timer;

    /* This was not done before. Is done in impl1 - Apr 2010 */
    pGi->dmaCtrl->controller_cfg |= DMA_MASTER_EN;

    pGi->ulDrvState = XTMRT_INITIALIZED;

    bcmxtmrt_in_init_dev = 0;

    return 0;
} /* DoGlobInitReq */


/***************************************************************************
 * Function Name: DoGlobUninitReq
 * Description  : Processes an XTMRT_CMD_GLOBAL_UNINITIALIZATION command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoGlobUninitReq( void )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    UINT32 j;
#endif

    if( pGi->ulDrvState == XTMRT_UNINITIALIZED )
    {
        nRet = -EPERM;
    }
    else
    {
        pGi->ulDrvState = XTMRT_UNINITIALIZED;

        for( i = 0; i < MAX_RECEIVE_QUEUES; i++ )
        {
#if !(defined(CONFIG_BCM_FAP) || defined (CONFIG_BCM_FAP_MODULE))
            BcmHalInterruptDisable(SAR_RX_INT_ID_BASE + i);
#endif

#if !defined(XTM_RX_BDS_IN_PSM)
            if (pGi->rxdma[i]->pktDmaRxInfo.rxBdsBase)
                kfree((void *)pGi->rxdma[i]->pktDmaRxInfo.rxBdsBase);
#endif
            /* Free space for receive socket buffers and data buffers */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            xtm_bpm_free_buf_ring(pGi->rxdma[i]);
#else
            for (j = 0; j < MAX_BUFMEM_BLOCKS; j++)
            {
                if (pGi->rxdma[i]->buf_pool[j])
                {
                    kfree(pGi->rxdma[i]->buf_pool[j]);
                    pGi->rxdma[i]->buf_pool[j] = NULL;
                }
            }
#endif
            if (pGi->rxdma[i])
                kfree((void *)(pGi->rxdma[i]));

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            gbpm_unresv_rx_buf( GBPM_PORT_XTM, i );
#endif
        }

       if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
       {
          /* Uninitialize the SW Scheduler timer */
           bcmxtmrt_uninit_swsched_timer () ;

          /* Uninitialize the swDmaCtrl area */
          if (pGi->swDmaCtrlBase != NULL)
             kfree ((void *) pGi->swDmaCtrlBase) ;
          pGi->swDmaCtrlBase = pGi->swDmaCtrl = NULL ;
       }

        del_timer_sync(&pGi->Timer);
    }
    return( nRet );

} /* DoGlobUninitReq */


/***************************************************************************
 * Function Name: DoCreateDeviceReq
 * Description  : Processes an XTMRT_CMD_CREATE_DEVICE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoCreateDeviceReq( PXTMRT_CREATE_NETWORK_DEVICE pCnd )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
    struct net_device *dev = NULL;
    int i;
    UINT32 unit = 0;
    UINT32 macId = 0;
    UINT32 blogPhyType;
    UINT32 ulRfc2684_type = RFC2684_NONE;
    UINT32 hwAction;

    BCM_XTM_DEBUG("DoCreateDeviceReq\n");

    if( pGi->ulDrvState != XTMRT_UNINITIALIZED &&
        (dev = alloc_netdev( sizeof(BCMXTMRT_DEV_CONTEXT),
         pCnd->szNetworkDeviceName, ether_setup )) != NULL )
    {
        dev_alloc_name(dev, dev->name);
        SET_MODULE_OWNER(dev);

 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        pDevCtx = (PBCMXTMRT_DEV_CONTEXT) netdev_priv(dev);
 #else
        pDevCtx = (PBCMXTMRT_DEV_CONTEXT) dev->priv;
 #endif
        memset(pDevCtx, 0x00, sizeof(BCMXTMRT_DEV_CONTEXT));
        memcpy(&pDevCtx->Addr, &pCnd->ConnAddr, sizeof(XTM_ADDR));
        if(( pCnd->ConnAddr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK ) == TRAFFIC_TYPE_ATM )
            pDevCtx->ulHdrType = pCnd->ulHeaderType;
        else
            pDevCtx->ulHdrType = HT_PTM;

        if (pDevCtx->ulHdrType == HT_PTM) {
           if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
              pDevCtx->ulTrafficType = TRAFFIC_TYPE_PTM_BONDED ;
           else
              pDevCtx->ulTrafficType = TRAFFIC_TYPE_PTM ;
        }
        else {
           if (pGi->bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)
              pDevCtx->ulTrafficType = TRAFFIC_TYPE_ATM_BONDED ;
           else
              pDevCtx->ulTrafficType = TRAFFIC_TYPE_ATM ;
        }

        pDevCtx->ulFlags = pCnd->ulFlags;
        pDevCtx->pDev = dev;
        pDevCtx->ulAdminStatus = ADMSTS_UP;
        pDevCtx->ucTxVcid = INVALID_VCID;

        /* Read and display the MAC address. */
        dev->dev_addr[0] = 0xff;

        /* format the mac id */
        i = strcspn(dev->name, "0123456789");
        if (i > 0)
           unit = simple_strtoul(&(dev->name[i]), (char **)NULL, 10);

        if (pDevCtx->ulHdrType == HT_PTM)
           macId = MAC_ADDRESS_PTM;
        else
           macId = MAC_ADDRESS_ATM;
        /* set unit number to bit 20-27 */
        macId |= ((unit & 0xff) << 20);

#if defined(AEI_VDSL_CUSTOMER_QWEST)
        printk("KEN- devname=%s\n",dev->name);
	if(strstr(dev->name,"atm0")!=NULL || strstr(dev->name,"ptm0")!=NULL) 
        kerSysGetMacAddress( dev->dev_addr,  0x12ffffff);
        else
#endif
        kerSysGetMacAddress(dev->dev_addr, macId);

        if( (dev->dev_addr[0] & 0x01) == 0x01 )
        {
            printk( KERN_ERR CARDNAME": Unable to read MAC address from "
                "persistent storage.  Using default address.\n" );
            memcpy( dev->dev_addr, "\x02\x10\x18\x02\x00\x01", 6 );
        }

        printk( CARDNAME": MAC address: %2.2x %2.2x %2.2x %2.2x %2.2x "
            "%2.2x\n", dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
            dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5] );
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        dev->netdev_ops = &bcmXtmRt_netdevops;
#else
        /* Setup the callback functions. */
        dev->open               = bcmxtmrt_open;
        dev->stop               = bcmxtmrt_close;
        dev->hard_start_xmit    = (HardStartXmitFuncP) bcmxtmrt_xmit;
        dev->tx_timeout         = bcmxtmrt_timeout;
        dev->set_multicast_list = NULL;
        dev->do_ioctl           = &bcmxtmrt_ioctl;
        dev->poll               = bcmxtmrt_poll;
        dev->weight             = 32;
        dev->get_stats          = bcmxtmrt_query;
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
        dev->clr_stats          = bcmxtmrt_clrStats;
#endif
        dev->watchdog_timeo     = SAR_TIMEOUT;

        /* identify as a WAN interface to block WAN-WAN traffic */
        dev->priv_flags |= IFF_WANDEV;

        switch( pDevCtx->ulHdrType )
        {
        case HT_LLC_SNAP_ROUTE_IP:
        case HT_VC_MUX_IPOA:
            pDevCtx->ulEncapType = TYPE_IP;     /* IPoA */

            /* Since IPoA does not need a Ethernet header,
             * set the pointers below to NULL. Refer to kernel rt2684.c.
             */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
            dev->header_ops = &bcmXtmRt_headerOps;
#else
            dev->hard_header = NULL;
            dev->rebuild_header = NULL;
            dev->set_mac_address = NULL;
            dev->hard_header_parse = NULL;
            dev->hard_header_cache = NULL;
            dev->header_cache_update = NULL;
            dev->change_mtu = NULL;
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

            dev->type = ARPHRD_PPP;
            dev->hard_header_len = HT_LEN_LLC_SNAP_ROUTE_IP;
            dev->mtu = RFC1626_MTU;
            dev->addr_len = 0;
            dev->tx_queue_len = 100;
            dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
            break;

        case HT_LLC_ENCAPS_PPP:
        case HT_VC_MUX_PPPOA:
            pDevCtx->ulEncapType = TYPE_PPP;    /*PPPoA*/
            break;

        default:
            pDevCtx->ulEncapType = TYPE_ETH;    /* bridge, MER, PPPoE, PTM */
            dev->flags = IFF_BROADCAST | IFF_MULTICAST;
            break;
        }

        if( (pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0 )
        {
           if( HT_LEN(pDevCtx->ulHdrType) > 0)
              ulRfc2684_type = HT_TYPE(pDevCtx->ulHdrType);
        }

        hwAction = HT_TYPE(pDevCtx->ulHdrType);
        blogPhyType = BLOG_SET_PHYHDR(ulRfc2684_type, BLOG_XTMPHY);

        /* Embed HT_TYPE() info for provisioned mcast case */
        blogPhyType |= BLOG_SET_HW_ACT(hwAction);
        netdev_path_set_hw_port(dev, 0, blogPhyType);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#if defined(AEI_VDSL_CUSTOMER_NCS)
        netif_napi_add(dev, &pDevCtx->napi, bcmxtmrt_poll_napi, 32);
#else
        netif_napi_add(dev, &pDevCtx->napi, bcmxtmrt_poll_napi, 32);
#endif
#endif
        /* Don't reset or enable the device yet. "Open" does that. */
        printk("[%s.%d]: register_netdev\n", __func__, __LINE__);
        nRet = register_netdev(dev);
        printk("[%s.%d]: register_netdev done\n", __func__, __LINE__);
        if (nRet == 0)
        {
            UINT32 i;
            for( i = 0; i < MAX_DEV_CTXS; i++ )
                if( pGi->pDevCtxs[i] == NULL )
                {
                    UINT32 trailerDelLen = 0;
                    UINT32 delLen = 0;

                    pGi->pDevCtxs[i] = pDevCtx;

                    if ( pDevCtx->ulHdrType == HT_PTM &&
                         (pDevCtx->ulFlags & CNI_HW_REMOVE_TRAILER) == 0 )
                    {
                        if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
                        {
                            trailerDelLen = XTMRT_PTM_CRC_SIZE ;
                        }
                        else
                        {
                            trailerDelLen = (ETH_FCS_LEN+XTMRT_PTM_CRC_SIZE) ;
                        }
                    }

                    if( (pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0 )
                    {
                        delLen = HT_LEN(pDevCtx->ulHdrType);
                    }

                    bcmPktDma_XtmCreateDevice(i, pDevCtx->ulEncapType, delLen, trailerDelLen);

                    break;
                }

            pCnd->hDev = (XTMRT_HANDLE) pDevCtx;
        }
        else
        {
            printk(KERN_ERR CARDNAME": register_netdev failed\n");
            free_netdev(dev);
        }

        if( nRet != 0 )
            kfree(pDevCtx);
    }
    else
    {
        printk(KERN_ERR CARDNAME": alloc_netdev failed\n");
        nRet = -ENOMEM;
    }

    return( nRet );
} /* DoCreateDeviceReq */


/***************************************************************************
 * Function Name: DoRegCellHdlrReq
 * Description  : Processes an XTMRT_CMD_REGISTER_CELL_HANDLER command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoRegCellHdlrReq( PXTMRT_CELL_HDLR pCh )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    switch( pCh->ulCellHandlerType )
    {
    case CELL_HDLR_OAM:
        if( pGi->pfnOamHandler == NULL )
        {
            pGi->pfnOamHandler = pCh->pfnCellHandler;
            pGi->pOamContext = pCh->pContext;
        }
        else
            nRet = -EEXIST;
        break;

    case CELL_HDLR_ASM:
        if( pGi->pfnAsmHandler == NULL )
        {
            pGi->pfnAsmHandler = pCh->pfnCellHandler;
            pGi->pAsmContext = pCh->pContext;
        }
        else
            nRet = -EEXIST;
        break;
    }

    return( nRet );
} /* DoRegCellHdlrReq */


/***************************************************************************
 * Function Name: DoUnregCellHdlrReq
 * Description  : Processes an XTMRT_CMD_UNREGISTER_CELL_HANDLER command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnregCellHdlrReq( PXTMRT_CELL_HDLR pCh )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    switch( pCh->ulCellHandlerType )
    {
    case CELL_HDLR_OAM:
        if( pGi->pfnOamHandler == pCh->pfnCellHandler )
        {
            pGi->pfnOamHandler = NULL;
            pGi->pOamContext = NULL;
        }
        else
            nRet = -EPERM;
        break;

    case CELL_HDLR_ASM:
        if( pGi->pfnAsmHandler == pCh->pfnCellHandler )
        {
            pGi->pfnAsmHandler = NULL;
            pGi->pAsmContext = NULL;
        }
        else
            nRet = -EPERM;
        break;
    }

    return( nRet );
} /* DoUnregCellHdlrReq */


/***************************************************************************
 * Function Name: DoLinkStsChangedReq
 * Description  : Processes an XTMRT_CMD_LINK_STATUS_CHANGED command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkStsChangedReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_LINK_STATUS_CHANGE pLsc )
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    int nRet = -EPERM;
    UINT32 dmaSize ;

    local_bh_disable();

    if( pDevCtx )
    {
        UINT32 i;

#if 0  /* debug code */
    {
    int j;
    printk("ulLinkState: %ld ulLinkUsRate: %ld ulLinkDsRate: %ld ulLinkDataMask: %ld ulTransmitQueueIdsSize: %ld ucTxVcid: %d ulRxVcidsSize: %ld\n\n",
           pLsc->ulLinkState, pLsc->ulLinkUsRate, pLsc->ulLinkDsRate, pLsc->ulLinkDataMask, pLsc->ulTransmitQueueIdsSize, pLsc->ucTxVcid, pLsc->ulRxVcidsSize);

    for (j = 0; j < MAX_TRANSMIT_QUEUES; j++)
        printk("%d: ulPortId: %ld PtmPriority: %ld WeightAlg: %ld WeightValue: %ld SubPriority: %ld QueueSize: %ld QueueIndex: %ld BondingPortId: %ld\n",
        j, pLsc->TransmitQueueIds[j].ulPortId, pLsc->TransmitQueueIds[j].ulPtmPriority, pLsc->TransmitQueueIds[j].ulWeightAlg, pLsc->TransmitQueueIds[j].ulWeightValue,
        pLsc->TransmitQueueIds[j].ulSubPriority, pLsc->TransmitQueueIds[j].ulQueueSize, pLsc->TransmitQueueIds[j].ulQueueIndex, pLsc->TransmitQueueIds[j].ulBondingPortId);
    }
#endif

        for( i = 0; i < MAX_DEV_CTXS; i++ )
        {
           if( pGi->pDevCtxs[i] == pDevCtx )
           {
              UINT32 ulLinkUsRate[MAX_BOND_PORTS], ulLinkDsRate ;

              pDevCtx->ulFlags |= pLsc->ulLinkState & LSC_RAW_ENET_MODE;
              pLsc->ulLinkState &= ~LSC_RAW_ENET_MODE;
              pDevCtx->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;
              pDevCtx->MibInfo.ulIfSpeed = pLsc->ulLinkUsRate+pLsc->ulOtherLinkUsRate ;

              ulLinkUsRate[0] = pDevCtx->ulLinkUsRate[0] ;
              ulLinkUsRate[1] = pDevCtx->ulLinkUsRate[1] ;
              ulLinkDsRate    = pDevCtx->ulLinkDsRate ;
              pDevCtx->ulLinkUsRate[0] = pLsc->ulLinkUsRate ;
              pDevCtx->ulLinkUsRate[1] = pLsc->ulOtherLinkUsRate ;
              pDevCtx->ulLinkDsRate    = pLsc->ulLinkDsRate + pLsc->ulOtherLinkDsRate ;
               if (pLsc->ulDsSeqDeviation <= MAX_SEQ_NR_DEVIATION)
                  pDevCtx->ulDsSeqDeviation  = pLsc->ulDsSeqDeviation ;
               else {
                  printk ("bcmxtmrt: !!!Diff Seq Deviation Exceeds Support \n") ;
                  pDevCtx->ulDsSeqDeviation = MAX_SEQ_NR_DEVIATION ;
               }

               printk ("bcmxtmrt: DSD(%x) - %ld \n",
                       (unsigned int) &pDevCtx->ulDsSeqDeviation, pDevCtx->ulDsSeqDeviation) ;
               pDevCtx->ulDsOrigSeqDeviation = pDevCtx->ulDsSeqDeviation ;

              /* compute the weights */
               if (pLsc->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {
                  pDevCtx->ulPortDataMask = pLsc->ulLinkDataMask ;
			         //printk ("PDM1=%x\n", (unsigned int) pDevCtx->ulPortDataMask) ;
                  if ((nRet = bcmxtmrt_ptmbond_calculate_link_weights (pDevCtx)) < 0) {
                     pDevCtx->ulLinkUsRate[0] = ulLinkUsRate[0] ;
                     pDevCtx->ulLinkUsRate[1] = ulLinkUsRate[1] ;
                     pDevCtx->ulLinkDsRate    = ulLinkDsRate ;
                     bcmxtmrt_ptmbond_calculate_link_weights (pDevCtx) ;
                     break ;
                  }
               }

              if( pLsc->ulLinkState == LINK_UP )
              {
                 if (pLsc->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {
                    nRet = bcmxtmrt_init_swsched_timer (bcmxtmrt_swscheduler, pGi,
                          XTM_SW_SCHED_TIMER_REINIT) ;
                    if (nRet != 0)
                        break ;
                 }
                 nRet = DoLinkUp( pDevCtx, pLsc , i);
              }
              else
              {
                 spin_lock(&pGi->xtmlock_tx);
                 nRet = DoLinkDownTx( pDevCtx, pLsc );
                 spin_unlock(&pGi->xtmlock_tx);
                 if (nRet == 0 && pLsc->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {
                    nRet = bcmxtmrt_init_swsched_timer (bcmxtmrt_swscheduler, pGi, XTM_SW_SCHED_TIMER_REINIT) ;
						  if (pLsc->ulLinkDataMask == 0) { /* All the links down. PtmBond Service is down */
								bcmxtmrt_sw_sched_initOrReset (pGi) ;
						  }
                 }
              }

              break;
           }
        }

        if (nRet==0 && pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {

            spin_lock (&pGi->xtmlock_rx_bond) ;
            dmaSize = bcmPktDma_XtmRecv_RingSize (&pGi->rxdma[0]->pktDmaRxInfo) ; /* Rx channel 0 */
            bcmxtmrt_ptmbond_handle_port_status_change (pDevCtx, &pGi->ptmBondInfo, pLsc->ulLinkState,
                                                        dmaSize) ;
            spin_unlock (&pGi->xtmlock_rx_bond) ;
        }
    }
    else
    {
        /* No device context indicates that the link is down.  Do global link
         * down processing.  pLsc is really an unsigned long containing the
         * port id.
         */
      spin_lock(&pGi->xtmlock_rx);
      nRet = DoLinkDownRx( (UINT32) pLsc );
      spin_unlock(&pGi->xtmlock_rx);
    }

    local_bh_enable();


#if 0 /* debug code */
    printk("\n");
    printk("GLOBAL: ulNumTxQs %ld\n", pGi->ulNumTxQs);

    if (pDevCtx != NULL)
    {
        printk("DEV PTR: %p VPI: %d VCI: %d\n", pDevCtx, pDevCtx->Addr.u.Vcc.usVpi, pDevCtx->Addr.u.Vcc.usVci);
        printk("DEV ulLinkState: %ld ulPortDataMask: %ld ulOpenState: %ld ulAdminStatus: %ld \n",
                pDevCtx->ulLinkState, pDevCtx->ulPortDataMask, pDevCtx->ulOpenState, pDevCtx->ulAdminStatus);
        printk("DEV ulHdrType: %ld ulEncapType: %ld ulFlags: %ld ucTxVcid: %d ulTxQInfosSize: %ld\n",
                pDevCtx->ulHdrType, pDevCtx->ulEncapType, pDevCtx->ulFlags, pDevCtx->ucTxVcid, pDevCtx->ulTxQInfosSize);
    }
    else
        printk("StsChangedReq called with NULL pDevCtx\n");
#endif

    return( nRet );
} /* DoLinkStsChangedReq */


/***************************************************************************
 * Function Name: DoLinkUp
 * Description  : Processes a "link up" condition.
 *                In bonding case, successive links may be coming UP one after
 *                another, accordingly the processing differs.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkUp( PBCMXTMRT_DEV_CONTEXT pDevCtx,
                     PXTMRT_LINK_STATUS_CHANGE pLsc,
                     UINT32 ulDevId)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId;
    int i, j ;

    BCM_XTM_DEBUG("DoLinkUp\n");

    if (pDevCtx->ulLinkState != pLsc->ulLinkState)
    {
        /* Initialize transmit DMA channel information. */
        pDevCtx->ucTxVcid = pLsc->ucTxVcid;
        pDevCtx->ulLinkState = pLsc->ulLinkState;
        pDevCtx->ulTxQInfosSize = 0;

        netdev_path_set_hw_port_only(pDevCtx->pDev, ulDevId);

        /* Use each Rx vcid as an index into an array of bcmxtmrt devices
         * context structures.
         */
        for (i = 0; i < pLsc->ulRxVcidsSize ; i++) {
            pGi->pDevCtxsByMatchId[pLsc->ucRxVcids[i]] = pDevCtx;
            pGi->ulDevCtxMask |= (1 << pLsc->ucRxVcids[i]);
            bcmPktDma_XtmLinkUp(ulDevId, pLsc->ucRxVcids[i]);
        }

        for(i = 0, pTxQId = pLsc->TransmitQueueIds;
                   i < pLsc->ulTransmitQueueIdsSize; i++, pTxQId++)
        {
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            xtm_bpm_txq_thresh(pDevCtx, pTxQId, i);
#endif
            if (pTxQId->ulBondingPortId != PORT_PHY_INVALID) {

               if (DoSetTxHwQueue (pDevCtx, pTxQId) != 0)
                  return -ENOMEM;
            }

            if (DoSetTxQueue(pDevCtx, pTxQId) != 0)
            {
                pDevCtx->ulTxQInfosSize = 0;

                if (pTxQId->ulBondingPortId != PORT_PHY_INVALID) {

                   for(j = 0, pTxQId = pLsc->TransmitQueueIds;   /* Reusing pTxQId as we are exiting */
                         j < pLsc->ulTransmitQueueIdsSize; j++, pTxQId++) {
                      DoUnsetTxHwQueues () ;
                   }
                }

                return -ENOMEM;
            }
        } /* for i */

        /* If it is not already there, put the driver into a "ready to send and
         * receive state".
         */
        if (pGi->ulDrvState == XTMRT_INITIALIZED)
        {
             pGi->ulDrvState = XTMRT_RUNNING;

            pGi->prevTime = jiffies ;

            pGi->Timer.expires = jiffies + SAR_TIMEOUT;
            add_timer(&pGi->Timer);

            if (pDevCtx->ulOpenState == XTMRT_DEV_OPENED)
                netif_start_queue(pDevCtx->pDev);
        }
    }
    pDevCtx->ulPortDataMask = pLsc->ulLinkDataMask ;

    return 0;
} /* DoLinkUp */


/***************************************************************************
 * Function Name: DoLinkDownRx
 * Description  : Processes a "link down" condition for receive only.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkDownRx( UINT32 ulPortId )
{
    int nRet = 0;
    BcmXtm_RxDma       *rxdma;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i, ulStopRunning;

    BCM_XTM_DEBUG("DoLinkDownRx\n");

    /* If all links are down, put the driver into an "initialized" state. */
    for( i = 0, ulStopRunning = 1; i < MAX_DEV_CTXS; i++ )
    {
        if( pGi->pDevCtxs[i] )
        {
            PBCMXTMRT_DEV_CONTEXT pDevCtx = pGi->pDevCtxs[i];
            UINT32 ulDevPortId = pDevCtx->ulPortDataMask ;
            if( (ulDevPortId & ~ulPortId) != 0 )
            {
                /* At least one link that uses a different port is up.
                 * For Ex., in bonding case, one of the links can be up
                 */
                ulStopRunning = 0;
                break;
            }
        }
    }

    if( ulStopRunning )
    {
        pGi->ulDrvState = XTMRT_INITIALIZED;

        /* Disable receive interrupts and stop the timer. */
        for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
        {
            rxdma = pGi->rxdma[i];
            if (rxdma->pktDmaRxInfo.rxBds)
            {
                bcmPktDma_XtmRxDisable(&rxdma->pktDmaRxInfo);
#if !(defined(CONFIG_BCM_FAP) || defined (CONFIG_BCM_FAP_MODULE))
                BcmHalInterruptDisable(SAR_RX_INT_ID_BASE + i);
#endif
            }
        }

        /* Stop the timer. */
        del_timer_sync(&pGi->Timer);
    }

    return( nRet );
} /* DoLinkDownRx */

/***************************************************************************
 * Function Name: DoTogglePortDataStatusReq
 * Description  : Processes an XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoTogglePortDataStatusReq( PBCMXTMRT_DEV_CONTEXT pDevCtx,
     PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE pTpdsc )
{
	UINT32 i ;
	PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   UINT32 dmaSize ;

	local_bh_disable();
	for( i = 0; i < MAX_DEV_CTXS; i++ )
	{
		pDevCtx  = pGi->pDevCtxs [i] ;

		if ((pDevCtx != NULL) && (pDevCtx->ulHdrType == HT_PTM)) {

			spin_lock(&pGi->xtmlock_tx);
         /* For the US direction */
			if (pTpdsc->ulPortDataUsStatus == XTMRT_CMD_PORT_DATA_STATUS_ENABLED)
				pDevCtx->ulPortDataMask |= (0x1 << pTpdsc->ulPortId) ;
			else
				pDevCtx->ulPortDataMask &= ~(0x1 << pTpdsc->ulPortId) ;
			spin_unlock(&pGi->xtmlock_tx);
			//printk ("PDM2=%x\n", (unsigned int) pDevCtx->ulPortDataMask) ;
			spin_lock (&pGi->xtmlock_rx_bond) ;
         dmaSize = bcmPktDma_XtmRecv_RingSize (&pGi->rxdma[0]->pktDmaRxInfo) ; /* Rx channel 0 */
         /* For the DS direction */
         if (pTpdsc->ulPortDataDsStatus == XTMRT_CMD_PORT_DATA_STATUS_DISABLED) {
             if (pTpdsc->ulPortDataUsStatus == XTMRT_CMD_PORT_DATA_STATUS_DISABLED) {
                bcmxtmrt_ptmbond_handle_port_status_change (pDevCtx, &pGi->ptmBondInfo,
                                  LINK_DOWN, dmaSize) ;
             }
             else {
                bcmxtmrt_ptmbond_handle_port_status_change (pDevCtx, &pGi->ptmBondInfo,
                                  LINK_DS_DOWN, dmaSize) ;
             }
         }
         else {
            bcmxtmrt_ptmbond_handle_port_status_change (pDevCtx, &pGi->ptmBondInfo,
                  ((pTpdsc->ulPortDataUsStatus == XTMRT_CMD_PORT_DATA_STATUS_ENABLED) ? LINK_UP :
                   LINK_DOWN), dmaSize) ;
         }

			spin_unlock (&pGi->xtmlock_rx_bond) ;

			break ;
		}
	}

	local_bh_enable();

	return( 0 );
} /* DoTogglePortDataStatusReq */

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
/******************************************************************************
* Function: xtmDmaStatus                                                      *
*                                                                             *
* Description: Dumps information about the status of the XTM IUDMA channel    *
******************************************************************************/
void xtmDmaStatus(int channel, BcmPktDma_XtmTxDma *txdma)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_LocalXtmRxDma *rxdma;

    rxdma = (BcmPktDma_LocalXtmRxDma *)&(pGi->rxdma[channel]->pktDmaRxInfo);

    printk("XTM IUDMA INFO CH %d\n", channel);

    printk("RXDMA STATUS: HeadIndex: %d TailIndex: %d numRxBds: %d rxAssignedBds: %d\n",
                  rxdma->rxHeadIndex, rxdma->rxTailIndex,
                  rxdma->numRxBds, rxdma->rxAssignedBds);

    printk("RXDMA CFG: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n",
                     rxdma->rxDma->cfg,
                     rxdma->rxDma->intStat,
                     rxdma->rxDma->intMask);

    printk("TXDMA STATUS: HeadIndex: %d TailIndex: %d txFreeBds: %d\n",
                  txdma->txHeadIndex,
                  txdma->txTailIndex,
                  txdma->txFreeBds);

    printk("TXDMA CFG: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n",
                     txdma->txDma->cfg,
                     txdma->txDma->intStat,
                     txdma->txDma->intMask);

}
#endif

/***************************************************************************
 * Function Name: ShutdownTxQueue
 * Description  : Shutdown and clean up a transmit queue by waiting for it to
 *                empty, clearing state ram and free memory allocated for it.
 * Returns      : None.
 ***************************************************************************/
static void ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx, volatile BcmPktDma_XtmTxDma *txdma,
    volatile DmaStateRam *pStRam)
{
#if !defined(AEI_VDSL_ADSL_BONDING_AUTODETECT)
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    UINT32 j;
#endif
#endif
    UINT32 ulIdx;
    UINT32 ulPtmPrioIdx ;
    int disableSuccess = 1 ;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
      /* Add a variable to confirm that the 4ke tx disable is complete - May 2010 */
    pHostPsmGbl(txdma->fapIdx)->XtmTxDownFlags[txdma->ulDmaIndex] = 0;
#endif

    ulPtmPrioIdx = txdma->ulPtmPriority ? txdma->ulPtmPriority - PTM_PRI_LOW : 0 ;
    bcmPktDma_XtmTxDisable((BcmPktDma_LocalXtmTxDma *)txdma, TXDMATYPE(pDevCtx),
          bcmxtmrt_swsched_disable_swdma, ulPtmPrioIdx);

#if !defined(AEI_VDSL_ADSL_BONDING_AUTODETECT)
    /* fix 6.5 min delay when switching from ADSL bonding to non-bonding for 6368 */
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Wait until iuDMA/SwScheduler is really disabled (esp. when the disable is done by FAP) */
    for (j = 0; (j < 20000) && ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE); j++) {
       /* Increase the wait of the outer loop to 20000 from 2000 as XTM tx of */
       /* large packets in CBR mode with peak cell rate of 48 is slow - Dec 2010 */
       udelay(10000);
    }
#endif
#endif

    if((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE) {
        printk("XTM Tx ch %d NOT disabled. Fatal\n", (int)txdma->ulDmaIndex);
       freeXmitPkts (pDevCtx, txdma, disableSuccess) ;
    }
    else
    {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        volatile fap4kePsm_global_t * pPsmGbl = (volatile fap4kePsm_global_t *)pHostPsmGbl(txdma->fapIdx);

        /* More delay required to ensure FAP has completed the XTM Force Free of the tx BDs */
        for (j = 0; (j < 2000) && (pPsmGbl->XtmTxDownFlags[txdma->ulDmaIndex] == 0); j++)
        {
            udelay(5000);
        }
        if(j < 2000)
#endif
        printk("HOST XTM Tx ch %d down success\n", (int)txdma->ulDmaIndex);
        freeXmitPkts (pDevCtx, txdma, disableSuccess) ;
    }

    ulIdx = SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex;

    pStRam[ulIdx].baseDescPtr = 0;
    pStRam[ulIdx].state_data = 0;
    pStRam[ulIdx].desc_len_status = 0;
    pStRam[ulIdx].desc_base_bufptr = 0;

    /* Uninitialize the SW Scheduler for this SW DMA fully */
    if (TXDMATYPE(pDevCtx)==XTM_SW_DMA)
       bcmxtmrt_swsched_unset_swdma (ulPtmPrioIdx, (BcmPktDma_XtmTxDma *) txdma) ;

    /* Added to match impl1 - May 2010 */
    txdma->txFreeBds = txdma->ulQueueSize = 0;

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    //xtmDmaStatus(txdma->ulDmaIndex, txdma);
#endif

#if !defined(XTM_TX_BDS_IN_PSM)
    /* remove the tx bd ring */
    if (txdma->txBdsBase)
    {
        kfree((void*)txdma->txBdsBase);
        txdma->txBdsBase = txdma->txBds = NULL;
    }
#endif
}

static void freeXmitPkts (PBCMXTMRT_DEV_CONTEXT pDevCtx, volatile BcmPktDma_XtmTxDma *txdma,
                          int flushOrDisableSuccess)
{
   int j, ret ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 txAddr;
   UINT32 txSource, rxChannel;
   pNBuff_t nbuff_reclaim_p;

   /* Free transmitted packets. */
   for (j = 0; j < txdma->ulQueueSize; j++)
   {
      if (flushOrDisableSuccess == 0x1) {
         ret = (bcmPktDma_XtmFreeXmitBufGet((BcmPktDma_LocalXtmTxDma *)txdma,
                  (uint32 *)&nbuff_reclaim_p, &txSource, &txAddr, &rxChannel,
                  TXDMATYPE(pDevCtx), 0x0)) ;
      }
      else {
         ret = (bcmPktDma_XtmForceFreeXmitBufGet((BcmPktDma_LocalXtmTxDma *)txdma,
                  (uint32 *)&nbuff_reclaim_p, &txSource, &txAddr, &rxChannel,
                  TXDMATYPE(pDevCtx), 0x0)) ;
      }

      if (ret)
      {
         if (nbuff_reclaim_p != PNBUFF_NULL) 
         {
            spin_unlock_bh(&pGi->xtmlock_tx);
            nbuff_free(nbuff_reclaim_p);
            spin_lock_bh(&pGi->xtmlock_tx);
         }
      }
   } /* for (j) */
}

/***************************************************************************
 * Function Name: FlushdownTxQueue
 * Description  : Flushes and clean up a transmit queue by waiting for it to
 *                empty,
 * Returns      : None.
 ***************************************************************************/
static void FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx, volatile BcmPktDma_XtmTxDma *txdma)
{
   UINT32 j;
   UINT32 ulPtmPrioIdx ;
   int flushSuccess = 1 ;

   ulPtmPrioIdx = txdma->ulPtmPriority ? txdma->ulPtmPriority - PTM_PRI_LOW : 0 ;
   bcmPktDma_XtmTxDisable((BcmPktDma_LocalXtmTxDma *)txdma, TXDMATYPE(pDevCtx),
         bcmxtmrt_swsched_flush_swdma, ulPtmPrioIdx);

   for (j = 0; (j < 5000) && ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE); j++) {
      udelay (10000) ;
   }

   if((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE) {
      printk("XTM Tx ch %d NOT flushed. Fatal\n", (int)txdma->ulDmaIndex);
      freeXmitPkts (pDevCtx, txdma, flushSuccess) ;
   }
   else
   {
      printk("HOST XTM Tx ch %d Flush success\n", (int)txdma->ulDmaIndex);
      freeXmitPkts (pDevCtx, txdma, flushSuccess) ;
   }

   bcmPktDma_XtmTxEnable((BcmPktDma_LocalXtmTxDma *)txdma, NULL, TXDMATYPE(pDevCtx)) ;
}

/***************************************************************************
 * Function Name: DoLinkDownTx
 * Description  : Processes a "link down" condition for transmit only.
 *                In bonding case, one of the links could still be UP, in which
 *                case only the link data status is updated.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoLinkDownTx( PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         PXTMRT_LINK_STATUS_CHANGE pLsc )
{
    int nRet = 0;
    volatile DmaRegs *pDmaCtrl = TXDMACTRL(pDevCtx);
    volatile DmaStateRam  *pStRam;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    volatile BcmPktDma_XtmTxDma  *txdma;
    UINT32 i, ulTxQs;

    BCM_XTM_DEBUG("DoLinkDownTx\n");

    if (pLsc->ulLinkDataMask == 0) {
       /* Disable transmit DMA. */
       pDevCtx->ulLinkState = LINK_DOWN;

       pStRam = pDmaCtrl->stram.s;
       for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
       {
          txdma = pDevCtx->txdma[i];
          ShutdownTxQueue(pDevCtx, txdma, pStRam);
       }

#if !defined(AEI_VDSL_CUSTOMER_NCS)
       /* Zero transmit related data structures. */
       pDevCtx->ulTxQInfosSize = 0;
#endif
       if (pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
          DoUnsetTxHwQueues () ;

       /* Free memory used for txdma info - Apr 2010 */
       for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
       {
           if(pDevCtx->txdma[i])
           {
              kfree((void*)pDevCtx->txdma[i]);
              pDevCtx->txdma[i] = NULL;
           }
       }
#if defined(AEI_VDSL_CUSTOMER_NCS)
       /* Zero transmit related data structures. */
       pDevCtx->ulTxQInfosSize = 0;
#endif

       /* Zero out list of priorities - Apr 2010 */
       memset(pDevCtx->pTxPriorities, 0x00, sizeof(pDevCtx->pTxPriorities));
       /* Zero out pTxQids pointer array */
       memset(pDevCtx->pTxQids, 0x00, sizeof(pDevCtx->pTxQids));

       pDevCtx->pHighestPrio = NULL;
       pDevCtx->ucTxVcid     = INVALID_VCID;
       pGi->ulNumTxBufsQdAll = 0;

       /* Zero receive vcids. */
       for( i = 0; i < MAX_MATCH_IDS; i++ )
          if( pGi->pDevCtxsByMatchId[i] == pDevCtx )
          {
             pGi->pDevCtxsByMatchId[i] = NULL;
             pGi->ulDevCtxMask &= ~(1 << i);
          }

       /* Count the total number of transmit queues used across all device
        * interfaces.
        */
       for( i = 0, ulTxQs = 0; i < MAX_DEV_CTXS; i++ )
           if( pGi->pDevCtxs[i] )
               ulTxQs += pGi->pDevCtxs[i]->ulTxQInfosSize;
       pGi->ulNumTxQs = ulTxQs;
    }
    else {
       /* flush out all the queues, as one of the ports, particularly in
        * bonding, could be down and all the data in the queues need to be
        * flushed out, as the data fragments might be desinted for this down
        * port.
        */
       for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
       {
          if (TXDMATYPE(pDevCtx)==XTM_SW_DMA) {
            txdma = pDevCtx->txdma[i];
            FlushdownTxQueue(pDevCtx, txdma) ;
          }
       }
    }

    pDevCtx->ulPortDataMask = pLsc->ulLinkDataMask;

    return( nRet );
} /* DoLinkDownTx */


/***************************************************************************
 * Function Name: DoSetTxQueue
 * Description  : Allocate memory for and initialize a transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    UINT32 ulQueueSize, ulPort;
    BcmPktDma_XtmTxDma  *txdma;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    BCM_XTM_DEBUG("DoSetTxQueue\n");

    local_bh_enable();  // needed to avoid kernel error
    pDevCtx->txdma[pDevCtx->ulTxQInfosSize] = txdma =
       (BcmPktDma_XtmTxDma *) (kzalloc(sizeof(BcmPktDma_XtmTxDma), GFP_KERNEL));
    local_bh_disable();

    if (pDevCtx->txdma[pDevCtx->ulTxQInfosSize] == NULL)
    {
        printk("Unable to allocate memory for tx dma info\n");
        return -ENOMEM;
    }

    /* Increment channels per dev context */
    pDevCtx->ulTxQInfosSize++;

    txdma->ulDmaIndex = pTxQId->ulQueueIndex;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    txdma->fapIdx = getFapIdxFromXtmTxIudma(txdma->ulDmaIndex);
#endif
    /* Set every transmit queue size to the number of external buffers.
     * The QueuePacket function will control how many packets are queued.
     */
    ulQueueSize = pTxQId->ulQueueSize; // pGi->ulNumExtBufs;

    local_bh_enable();  // needed to avoid kernel error - Apr 2010
    /* allocate and assign tx buffer descriptors */
    txdma->txBdsBase = bcmPktDma_XtmAllocTxBds(txdma->ulDmaIndex, ulQueueSize);
    local_bh_disable();

    if (txdma->txBdsBase == NULL)
    {
            printk("Unable to allocate memory for Tx Descriptors \n");
            return -ENOMEM;
    }
#if defined(XTM_TX_BDS_IN_PSM)
    txdma->txBds = txdma->txBdsBase;
#else
    /* Align to 16 byte boundary - Apr 2010 */
    txdma->txBds = (volatile DmaDesc *)(((int)txdma->txBdsBase + 0xF) & ~0xF);
    txdma->txBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(txdma->txBds);
#endif

    /* pKeyPtr, pTxSource, pTxAddress now relative to txBds - Apr 2010 */
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (pGi->ulNumExtBufs * sizeof(DmaDesc)));
#if !defined(XTM_TX_BDS_IN_PSM)
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);
#endif
    printk("XTM Init: %d tx BDs at 0x%x\n", (int)pGi->ulNumExtBufs, (unsigned int)txdma->txBds);

    ulPort = PORTID_TO_PORT(pTxQId->ulPortId);

    if ((ulPort < MAX_PHY_PORTS) && (pTxQId->ulSubPriority < MAX_SUB_PRIORITIES))
    {
        UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;
        volatile DmaRegs *pDmaCtrl = TXDMACTRL(pDevCtx);
        volatile DmaStateRam *pStRam = pDmaCtrl->stram.s;
        UINT32 i, ulTxQs;

        txdma->ulPort           = ulPort;
        txdma->ulPtmPriority    = pTxQId->ulPtmPriority;
        txdma->ulSubPriority    = pTxQId->ulSubPriority;
        txdma->ulAlg            = (UINT16) pTxQId->ulWeightAlg;
        txdma->ulWeightValue    = (UINT16) pTxQId->ulWeightValue;
        txdma->ulQueueSize      = ulQueueSize;
        txdma->ulDmaIndex       = pTxQId->ulQueueIndex;
        txdma->txEnabled        = 1;
        txdma->ulNumTxBufsQdOne = 0;

        txdma->txDma = &pDmaCtrl->chcfg[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex];
        txdma->txDma->cfg       = 0;
        txdma->txDma->maxBurst  = SAR_DMA_MAX_BURST_LENGTH;
        txdma->txDma->intMask   = 0;   /* mask all ints */

        memset((UINT8 *)&pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex],
                0x00, sizeof(DmaStateRam));
        pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex].baseDescPtr =
                (UINT32) VIRT_TO_PHY(txdma->txBds);

        txdma->txBds[txdma->ulQueueSize - 1].status |= DMA_WRAP;
        txdma->txFreeBds = txdma->ulQueueSize;
        txdma->txHeadIndex = txdma->txTailIndex ;

        if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM)
            ulPtmPrioIdx = (txdma->ulPtmPriority == PTM_PRI_HIGH)?
                              PTM_FLOW_PRI_HIGH : PTM_FLOW_PRI_LOW;

        pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][txdma->ulSubPriority] = txdma;
        pDevCtx->pTxQids[pTxQId->ulQosQId] = txdma;

        if (pDevCtx->pHighestPrio == NULL ||
            pDevCtx->pHighestPrio->ulSubPriority < txdma->ulSubPriority)
           pDevCtx->pHighestPrio = txdma;

        /* Set it for the scheduler context as well */
        if (TXDMATYPE(pDevCtx) == XTM_SW_DMA)
            bcmxtmrt_swsched_set_swdma (ulPtmPrioIdx, txdma) ;

        /* Count the total number of transmit queues used across all device
         * interfaces.
         */
        for( i = 0, ulTxQs = 0; i < MAX_DEV_CTXS; i++ )
        {
            if( pGi->pDevCtxs[i] )
                ulTxQs += pGi->pDevCtxs[i]->ulTxQInfosSize;
        }
        pGi->ulNumTxQs = ulTxQs;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        /* Shadow copies used by the 4ke for accelerated XTM - Apr 2010 */
        pDevCtx->devParams.ucTxVcid  = pDevCtx->ucTxVcid;
        pDevCtx->devParams.ulFlags   = pDevCtx->ulFlags;
        pDevCtx->devParams.ulHdrType = pDevCtx->ulHdrType;
#endif
    }

    bcmPktDma_XtmInitTxChan(txdma->ulQueueSize, txdma,
                            TXDMATYPE(pDevCtx));

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    bcmPktDma_XtmSetTxChanBpmThresh (txdma, pTxQId->ulLoThresh, pTxQId->ulHiThresh, XTM_HW_DMA);

    BCM_XTM_DEBUG("XTM TxQid ulLoThresh=%d, ulHiThresh=%d\n",
         (int) pTxQId->ulLoThresh, (int) pTxQId->ulHiThresh);
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    txdma->ulLoThresh = pTxQId->ulLoThresh;
    txdma->ulHiThresh = pTxQId->ulHiThresh;
#endif
#endif

#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
    bcmPktDma_XtmTxEnable(txdma, NULL, TXDMATYPE(pDevCtx));
#else
    /* For FAP builds, pass params needed for accelerated XTM - Apr 2010 */
    bcmPktDma_XtmTxEnable(txdma, &pDevCtx->devParams, TXDMATYPE(pDevCtx));
#endif

    return 0;
} /* DoSetTxQueue */


/***************************************************************************
 * Function Name: DoUnsetTxQueue
 * Description  : Frees memory for a transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnsetTxQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    int nRet = 0;
    UINT32 i, j;
    BcmPktDma_XtmTxDma  *txdma;
#if defined(AEI_VDSL_CUSTOMER_NCS)
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    spin_lock_bh(&pGi->xtmlock_tx);
#endif

    BCM_XTM_DEBUG("DoUnsetTxQueue\n");

    for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
    {
        txdma = pDevCtx->txdma[i];

        if( txdma && pTxQId->ulQueueIndex == txdma->ulDmaIndex )
        {
            UINT32 ulPort = PORTID_TO_PORT(pTxQId->ulPortId);
            PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
            volatile DmaRegs *pDmaCtrl = TXDMACTRL(pDevCtx);
            volatile DmaStateRam *pStRam = pDmaCtrl->stram.s;
            UINT32 ulTxQs;
            UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;

            ShutdownTxQueue(pDevCtx, txdma, pStRam);

            if ((pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM) ||
                (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
               ulPtmPrioIdx = (txdma->ulPtmPriority == PTM_PRI_HIGH)? PTM_FLOW_PRI_HIGH : PTM_FLOW_PRI_LOW;

            pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][txdma->ulSubPriority] = NULL;
            pDevCtx->pTxQids[pTxQId->ulQosQId] = NULL;

            if (pDevCtx->pHighestPrio == txdma)
               pDevCtx->pHighestPrio = NULL;

            /* Shift remaining array elements down by one element. */
            memmove(&pDevCtx->txdma[i], &pDevCtx->txdma[i + 1],
                (pDevCtx->ulTxQInfosSize - i - 1) * sizeof(txdma));
            pDevCtx->ulTxQInfosSize--;

            kfree((void*)txdma);

            /* Count the total number of transmit queues used across all device
             * interfaces.
             */
            for( j = 0, ulTxQs = 0; j < MAX_DEV_CTXS; j++ )
                if( pGi->pDevCtxs[j] )
                    ulTxQs += pGi->pDevCtxs[j]->ulTxQInfosSize;
            pGi->ulNumTxQs = ulTxQs;

            break;
        }
    }

    /* Find the highest subpriority dma */
    for (i = 0, txdma = pDevCtx->txdma[i];
         i < pDevCtx->ulTxQInfosSize;
         i++, txdma++)
    {
       if (pDevCtx->pHighestPrio == NULL ||
           pDevCtx->pHighestPrio->ulSubPriority < txdma->ulSubPriority)
          pDevCtx->pHighestPrio = txdma;
    }
#if defined(AEI_VDSL_CUSTOMER_NCS)
    spin_unlock_bh(&pGi->xtmlock_tx);
#endif

    return( nRet );
} /* DoUnsetTxQueue */


/***************************************************************************
 * Function Name: DoSendCellReq
 * Description  : Processes an XTMRT_CMD_SEND_CELL command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSendCellReq( PBCMXTMRT_DEV_CONTEXT pDevCtx, PXTMRT_CELL pC )
{
    int nRet = 0;

    if( pDevCtx->ulLinkState == LINK_UP )
    {
        struct sk_buff *skb = dev_alloc_skb(CELL_PAYLOAD_SIZE);

        if( skb )
        {
            UINT32 i;
            UINT32 ulPort = PORTID_TO_PORT(pC->ConnAddr.u.Conn.ulPortMask) ;
            UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;

            /* A network device instance can potentially have transmit queues
             * on different ports. Find a transmit queue for the port specified
             * in the cell structure.  The cell structure should only specify
             * one port.
             */
            for( i = 0; i < MAX_SUB_PRIORITIES; i++ )
            {
                if( pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][i] )
                {
                    skb->mark = i;
                    break;
                }
            }

            skb->dev = pDevCtx->pDev;
            __skb_put(skb, CELL_PAYLOAD_SIZE);
            memcpy(skb->data, pC->ucData, CELL_PAYLOAD_SIZE);

            switch( pC->ucCircuitType )
            {
            case CTYPE_OAM_F5_SEGMENT:
                skb->protocol = FSTAT_CT_OAM_F5_SEG;
                break;

            case CTYPE_OAM_F5_END_TO_END:
                skb->protocol = FSTAT_CT_OAM_F5_E2E;
                break;

            case CTYPE_OAM_F4_SEGMENT:
                skb->protocol = FSTAT_CT_OAM_F4_SEG;
                break;

            case CTYPE_OAM_F4_END_TO_END:
                skb->protocol = FSTAT_CT_OAM_F4_E2E;
                break;

            case CTYPE_ASM_P0:
                skb->protocol = FSTAT_CT_ASM_P0;
                break;

            case CTYPE_ASM_P1:
                skb->protocol = FSTAT_CT_ASM_P1;
                break;

            case CTYPE_ASM_P2:
                skb->protocol = FSTAT_CT_ASM_P2;
                break;

            case CTYPE_ASM_P3:
                skb->protocol = FSTAT_CT_ASM_P3;
                break;
            }

            skb->protocol |= SKB_PROTO_ATM_CELL;

            bcmxtmrt_xmit( SKBUFF_2_PNBUFF(skb), pDevCtx->pDev );
        }
        else
            nRet = -ENOMEM;
    }
    else
        nRet = -EPERM;

    return( nRet );
} /* DoSendCellReq */


/***************************************************************************
 * Function Name: DoDeleteDeviceReq
 * Description  : Processes an XTMRT_CMD_DELETE_DEVICE command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoDeleteDeviceReq( PBCMXTMRT_DEV_CONTEXT pDevCtx )
{
    int nRet = -EPERM;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    UINT32 i;

    BCM_XTM_DEBUG("DoDeleteDeviceReq\n");

    if ((pDevCtx != NULL) && (pDevCtx->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)) {
       spin_lock (&pGi->xtmlock_rx_bond);
       bcmxtmrt_ptmbond_tick (pDevCtx, &pGi->ptmBondInfo, 0) ;
       spin_unlock (&pGi->xtmlock_rx_bond);
    }

    for( i = 0; i < MAX_DEV_CTXS; i++ )
        if( pGi->pDevCtxs[i] == pDevCtx )
        {
            pGi->pDevCtxs[i] = NULL;

            kerSysReleaseMacAddress( pDevCtx->pDev->dev_addr );

            unregister_netdev( pDevCtx->pDev );
            free_netdev( pDevCtx->pDev );

            nRet = 0;
            break;
        }

    for( i = 0; i < MAX_MATCH_IDS; i++ )
        if( pGi->pDevCtxsByMatchId[i] == pDevCtx )
            pGi->pDevCtxsByMatchId[i] = NULL;

    return( nRet );
} /* DoDeleteDeviceReq */


/***************************************************************************
 * Function Name: DoGetNetDevTxChannel
 * Description  : Processes an XTMRT_CMD_GET_NETDEV_TXCHANNEL command.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoGetNetDevTxChannel( PXTMRT_NETDEV_TXCHANNEL pParm )
{
    int nRet = 0;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    BcmPktDma_XtmTxDma   *txdma;
    UINT32 i, j;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        pDevCtx = pGi->pDevCtxs[i];
        if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
        {
            if ( pDevCtx->ulOpenState == XTMRT_DEV_OPENED )
            {
                for (j = 0; j < pDevCtx->ulTxQInfosSize; j++)
                {
                    txdma = pDevCtx->txdma[j];

                    if ( txdma->ulDmaIndex == pParm->txChannel )
                    {
                        pParm->pDev = (void*)pDevCtx->pDev;
                        return nRet;
                    }
                }
            }
        }
    }

    return -EEXIST;
} /* DoGetNetDevTxChannel */


        /*********************************************************************
        * ALL SW Scheduler functions for BCM6368 PTM Bonding mode START here *
        *********************************************************************/

static void bcmxtmrt_swsched_set_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma)
{
   UINT8 *pu8SubPrioBitMap ;

   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   XtmRtTxSchedContext *pTxSchedCtxt = &pGi->txSchedCtxt ;

   txSwdma->txSchedHeadIndex = 0 ;
   memset (&pTxSchedCtxt->wfq[ulPtmPrioIdx] , 0, sizeof (SwSchedWfqInfo)) ;
   pTxSchedCtxt->uSwDma.pTxPrioSwDmaD [ulPtmPrioIdx][txSwdma->ulSubPriority] = txSwdma ;
   pTxSchedCtxt->ulSwTxQInfosSize++ ;
   pu8SubPrioBitMap = (UINT8 *) &pTxSchedCtxt->ulSubPrioBitMap[ulPtmPrioIdx] ;
   *pu8SubPrioBitMap |= 0x1 << (txSwdma->ulSubPriority) ;
}

static void bcmxtmrt_swsched_disable_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma)
{
   UINT8 *pu8SubPrioBitMap ;

   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   XtmRtTxSchedContext *pTxSchedCtxt = &pGi->txSchedCtxt ;

   pu8SubPrioBitMap = (UINT8 *) &pTxSchedCtxt->ulSubPrioBitMap[ulPtmPrioIdx] ;
   *pu8SubPrioBitMap &= ~(0x1 << (txSwdma->ulSubPriority)) ;

   /* post deletion command. Ordering here is important. */
   pTxSchedCtxt->ulCommandParam = (ulPtmPrioIdx*MAX_SUB_PRIORITIES) + txSwdma->ulSubPriority ;
   pTxSchedCtxt->ulCommand = XTM_SW_DMA_DELETE ;
}

static void bcmxtmrt_swsched_flush_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   XtmRtTxSchedContext *pTxSchedCtxt = &pGi->txSchedCtxt ;

   /* post deletion command. Ordering here is important. */
   pTxSchedCtxt->ulCommandParam = (ulPtmPrioIdx*MAX_SUB_PRIORITIES) + txSwdma->ulSubPriority ;
   pTxSchedCtxt->ulCommand = XTM_SW_DMA_FLUSH ;
}

static void bcmxtmrt_swsched_unset_swdma (UINT32 ulPtmPrioIdx, BcmPktDma_XtmTxDma  *txSwdma)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   XtmRtTxSchedContext *pTxSchedCtxt = &pGi->txSchedCtxt ;

   pTxSchedCtxt->uSwDma.pTxPrioSwDmaD [ulPtmPrioIdx][txSwdma->ulSubPriority] = NULL ;
   pTxSchedCtxt->ulSwTxQInfosSize-- ;
}

static unsigned int bcmxtmrt_swsched_timer_int(unsigned int i)
{
   if ((swSchedTimerUsecs != 0) && (TIMER->TimerInts && SW_SCHED_TIMER)) {
      TIMER->TimerInts = SW_SCHED_TIMER ;
      (*bcmxtmrt_swsched_timer_int_cb)(bcmxtmrt_swsched_timer_int_cb_data) ;
      SW_SCHED_TIMERCTL = (TIMERENABLE | swSchedTimerUsecs);
      BcmHalInterruptEnable(INTERRUPT_ID_TIMER);
   }
  return (IRQ_HANDLED) ;
}

static int bcmxtmrt_compute_us_sched_timer_divpktsize (UINT32 usRate, UINT32 dsRate)
{
   int divPktSize ;
   UINT32 oneMbps = 1024*1024 ;

#if 1
   if (dsRate <= 80*oneMbps) {

       if (usRate <= 8*oneMbps)
      divPktSize = 60 ;
       else if ((usRate > 8*oneMbps) && (usRate <= 20*oneMbps))
      divPktSize = 128 ;
   else if ((usRate > 20*oneMbps) && (usRate <= 30*oneMbps))
      divPktSize = 192 ;
   else
      divPktSize = 256 ;
   }
   else {
       if (usRate <= 15*oneMbps)
      divPktSize = 128 ;
       else if ((usRate > 15*oneMbps) && (usRate <= 30*oneMbps))
           divPktSize = 192 ;
       else
           divPktSize = 256 ;
   }
#else
       if (usRate <= 30*oneMbps)
           divPktSize = 192 ;
       else
           divPktSize = 256 ;
#endif

   return (divPktSize) ;
}

static int bcmxtmrt_init_swsched_timer (void (*cb)(PBCMXTMRT_GLOBAL_INFO cb_data),
       PBCMXTMRT_GLOBAL_INFO pGi, int timerInit)
{
  int k = 0, divPktSize ;
  UINT32 usTotalRate = 0, dsTotalRate = 0, timerValueinUSecs = 1000, prevUSecs ;
  PBCMXTMRT_DEV_CONTEXT pDevCtx = pGi->pDevCtxs[0] ;
  char devname[16];

  if (pDevCtx != NULL) {
     usTotalRate = pDevCtx->ulLinkUsRate[0] + pDevCtx->ulLinkUsRate[1] ;
     dsTotalRate = pDevCtx->ulLinkDsRate ;
  }

  if ((timerInit == XTM_SW_SCHED_TIMER_INIT) && (SW_SCHED_TIMERCTL & TIMERENABLE) == 0) {

     /* Registering IRQ */
     sprintf(devname, "bcm_bondschedT") ;
     k = BcmHalMapInterruptEx((FN_HANDLER) bcmxtmrt_swsched_timer_int, timerValueinUSecs,
                              INTERRUPT_ID_TIMER, devname, INTR_REARM_NO, INTR_AFFINITY_TP1_ONLY) ;
     if (k == -1)
        goto _End ;

     bcmxtmrt_swsched_timer_int_cb      = cb;
     bcmxtmrt_swsched_timer_int_cb_data = pGi;
  } /* First time, Init */
  else if (timerInit == XTM_SW_SCHED_TIMER_REINIT) {

     if (usTotalRate != 0) {

#if 0
        usTotalRate = usTotalRate+(usTotalRate*10/100) ;  /* extra credit of 10% times the total US rate, as the
                                                         scheduler has to work fast enough more than the avail
                                                         rate */
#endif
        divPktSize = bcmxtmrt_compute_us_sched_timer_divpktsize (usTotalRate, dsTotalRate) ;

        /* Enabling MIPS interrupt for sw sched timer and clearing any previous interrupt */
        TIMER->TimerMask |= SW_SCHED_TIMEREN ;
        TIMER->TimerInts = SW_SCHED_TIMER ;

        /* Calculate the timer value based on total US rates. if total rates are 0, then disable the timer */
        /* ((1000000*50 (UBUS freq))/((Total rate/bits in byte)/small(est) packet size (divPktSize bytes))) */
        timerValueinUSecs = (1000000*XTM_HW_TIMER_USEC/((usTotalRate/8)/divPktSize)) ;
        //timerValueinUSecs *= (MAX_HW_TRANSMIT_QUEUE_SIZE/2); /* schedule credit, experimenting */
        timerValueinUSecs *= MAX_HW_TRANSMIT_QUEUE_SIZE ;

        prevUSecs = swSchedTimerUsecs ;
        swSchedTimerUsecs = timerValueinUSecs ;
        if (prevUSecs == 0) {
           /* Setting the counter value and enabling SW Sched timer */
           SW_SCHED_TIMERCTL = TIMERENABLE | timerValueinUSecs ;
           BcmHalInterruptEnable(INTERRUPT_ID_TIMER);
        }
     }
     else {
        swSchedTimerUsecs = 0 ;
        BcmHalInterruptDisable(INTERRUPT_ID_TIMER);
        SW_SCHED_TIMERCTL &= ~TIMERENABLE;
     }
  } /* Reinit */
  else {
     printk ("bcmxtmrt: SW Sched Timer HW timer) is not available. Fatal !!! \n") ;
     k = -1 ;
  }

_End :
  return (k);
}

static void bcmxtmrt_uninit_swsched_timer ()
{
  BcmHalInterruptDisable(INTERRUPT_ID_TIMER);
  /* Stopping SW Sched Timer (HW Timer) */
  SW_SCHED_TIMERCTL &= ~TIMERENABLE;
  BcmHalInterruptEnable(INTERRUPT_ID_TIMER);
}

/***************************************************************************
 * Function Name: bcmxtmrt_sw_sched_initOrReset
 * Description  : Initializes the SW scheduler for BCM6368 PTM bonding mode.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static void bcmxtmrt_sw_sched_initOrReset ( PBCMXTMRT_GLOBAL_INFO pGi )
{
   XtmRtTxSchedContext *pTxSchedCtxt = &pGi->txSchedCtxt ;

   //printk ("bcmxtmrt: bcmxtmrt_sw_sched_initOrReset \n");
   memset (pTxSchedCtxt, 0, sizeof (XtmRtTxSchedContext)) ;
   pTxSchedCtxt->rrVisitCount[PTM_PRI_LOW-1] =
   pTxSchedCtxt->rrVisitCount[PTM_PRI_HIGH-1] = MAX_SUB_PRIORITIES ;
	return;
}

/***************************************************************************
 * Function Name: DoSetTxHwQueue
 * Description  : Allocate memory for and initialize a SAR HW transmit queue.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoSetTxHwQueue( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
   int nRet = 0 ;
#ifdef SW_SCHED
   UINT32 ulPort ;
   UINT32 ulQueueSize ;
   UINT32 ulQueueIndex, ulPtmPrioIdx ;
   BcmPktDma_XtmTxDma  *txdma ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   XtmRtTxSchedContext *pTxSchedCtxt = &g_GlobalInfo.txSchedCtxt ;

   BCM_XTM_DEBUG("DoSetTxHwQueue\n") ;

   ulPtmPrioIdx = pTxQId->ulPtmPriority ? pTxQId->ulPtmPriority - PTM_PRI_LOW : 0 ;
   ulQueueIndex = PORTID_TO_PORT (pTxQId->ulPortId) + ulPtmPrioIdx ;

   txdma = &pTxSchedCtxt->txHwdma [ulQueueIndex] ;

   if (txdma->txEnabled == 1) {
      //printk ("bcmxtmrt: Already active DMA \n") ;
      nRet = 0 ;
      goto _exit ;
   }

   /* Set every transmit queue size to the number of external buffers.
    * The QueuePacket function will control how many packets are queued.
    */
   ulQueueSize = MAX_HW_TRANSMIT_QUEUE_SIZE ;

   local_bh_enable();  // needed to avoid kernel error - Apr 2010
   /* allocate and assign tx buffer descriptors */
   txdma->txBdsBase = bcmPktDma_XtmAllocTxBds(ulQueueIndex, ulQueueSize);
   local_bh_disable();

   if (txdma->txBdsBase == NULL)
   {
      printk("Unable to allocate memory for Tx Descriptors \n");
      nRet=-ENOMEM;
      goto _exit ;
   }

   /* Align to 16 byte boundary - Apr 2010 */
   txdma->txBds = (volatile DmaDesc *)(((int)txdma->txBdsBase + 0xF) & ~0xF);
   txdma->txBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(txdma->txBds);

   /* pKeyPtr, pTxSource, pTxAddress now relative to txBds - Apr 2010 */
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (ulQueueSize * sizeof(DmaDesc)));
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);
   printk("XTM Init: %d tx BDs at 0x%x\n", (int)ulQueueSize, (unsigned int)txdma->txBds);

   ulPort = PORTID_TO_PORT(pTxQId->ulPortId);

   if (ulPort < MAX_PHY_PORTS) {

      volatile DmaStateRam *pStRam = pGi->dmaCtrl->stram.s;

      txdma->ulPort = ulPort;
      txdma->ulPtmPriority = pTxQId->ulPtmPriority;
      txdma->ulSubPriority = MAX_SUB_PRIORITIES-1;  /* Max Prio */
      txdma->ulAlg         = (UINT16) pTxQId->ulWeightAlg;
      txdma->ulQueueSize = ulQueueSize;
      txdma->ulDmaIndex = ulQueueIndex;
      txdma->txEnabled = 0;
      txdma->ulNumTxBufsQdOne = 0;
      txdma->txDma = &pGi->dmaCtrl->chcfg[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex];
      txdma->txDma->cfg = 0;
      txdma->txDma->maxBurst = SAR_DMA_MAX_BURST_LENGTH;
      txdma->txDma->intMask = 0;   /* mask all ints */

      memset((UINT8 *)&pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex],
            0x00, sizeof(DmaStateRam));

      pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex].baseDescPtr =
         (UINT32) VIRT_TO_PHY(txdma->txBds);

      txdma->txBds[txdma->ulQueueSize - 1].status |= DMA_WRAP;
      txdma->txFreeBds = txdma->ulQueueSize;
      txdma->txHeadIndex = txdma->txTailIndex = 0;

   } /* if (ulPort < MAX_PHY_PORTS) */

   bcmPktDma_XtmInitTxChan(txdma->ulQueueSize, txdma, XTM_HW_DMA);
   BCM_XTM_DEBUG("XTM ulQueueSize =%d\n", (int) txdma->ulQueueSize);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   bcmPktDma_XtmSetTxChanBpmThresh (txdma, pTxQId->ulLoThresh, pTxQId->ulHiThresh, XTM_HW_DMA);

    BCM_XTM_DEBUG("XTM TxQid ulLoThresh=%d, ulHiThresh=%d\n",
        (int) pTxQId->ulLoThresh, (int) pTxQId->ulHiThresh);
#endif

   bcmPktDma_XtmTxEnable(txdma, NULL, XTM_HW_DMA);

   /* Increment channels per dev context */
   pTxSchedCtxt->usHwTxQInfosSize++ ;

   //printk ("exiting DoSetTxHwQueue\n");

_exit :
#endif

   return (nRet);
} /* DoSetTxHwQueue */


/***************************************************************************
 * Function Name: DoUnsetTxHwQueues
 * Description  : Frees memory for all the available HW tx queues.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int DoUnsetTxHwQueues ( )
{
    int nRet = 0;
    UINT32 ulIdx, ulHwQueues ;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    XtmRtTxSchedContext   *pTxSchedCtxt = (XtmRtTxSchedContext *) &pGi->txSchedCtxt ;

#ifdef SW_SCHED
    UINT32 i ;
    BcmPktDma_XtmTxDma  *txdma;

    BCM_XTM_DEBUG("DoUnsetTxQueues\n");

    if (pTxSchedCtxt->ulSwTxQInfosSize == 0) {

       ulHwQueues = (UINT32) pTxSchedCtxt->usHwTxQInfosSize ;
       pTxSchedCtxt->usHwTxQInfosSize = 0 ;  /* Block the SW scheduler from doing anything */
       udelay (5000) ;

       for (i = 0; i < ulHwQueues; i++) {

          volatile DmaStateRam *pStRam = pGi->dmaCtrl->stram.s;
          txdma = &pTxSchedCtxt->txHwdma[i] ;

          /* ShutdownTxQueue limited functionality here */
          bcmPktDma_XtmTxDisable(txdma, XTM_HW_DMA, NULL, 0) ;

          if((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE) {
             printk("XTM HW Tx ch %d NOT disabled ALARM\n", (int)txdma->ulDmaIndex);
             printk("XTM HW Tx ch %d Force disabled \n", (int)txdma->ulDmaIndex);
          }
          else
             printk("HOST XTM HW Tx ch %d down success\n", (int)txdma->ulDmaIndex);

          ulIdx = SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex;

          pStRam[ulIdx].baseDescPtr = 0;
          pStRam[ulIdx].state_data = 0;
          pStRam[ulIdx].desc_len_status = 0;
          pStRam[ulIdx].desc_base_bufptr = 0;

          txdma->txFreeBds = txdma->ulQueueSize = 0;
          //txdma->ulNumTxBufsQdOne = 0;   Managed from within PktDma Library.

          /* remove the tx bd ring */
          if (txdma->txBdsBase)
          {
             kfree((void*)txdma->txBdsBase);
             txdma->txBdsBase = txdma->txBds = NULL;
          }
       }
    } /* No more SW queues */
#endif

    return( nRet );
} /* DoUnsetTxQueue */

inline static UINT32 bcmxtmrt_swsched_getxmit_len (BcmPktDma_XtmTxDma *txSwdma)
{
   volatile DmaDesc    *txSwBds ;
   txSwBds = &txSwdma->txBds[txSwdma->txSchedHeadIndex] ;
   return ((UINT32) (txSwBds->length)) ;
}

static void bcmxtmrt_swsched_xmit (XtmRtTxSchedContext *pTxSchedCtxt,
         BcmPktDma_XtmTxDma *txSwdma, int txHwchannel, int txSwChannel)
{
   UINT32              key ;
   DmaDesc             dmaDesc ;
   volatile DmaDesc    *txSwBds ;
#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
   BcmPktDma_XtmTxDma  *txdma = &pTxSchedCtxt->txHwdma [txHwchannel] ;
#endif

   txSwBds = &txSwdma->txBds[txSwdma->txSchedHeadIndex] ;
   memcpy (&dmaDesc, (DmaDesc *) txSwBds, sizeof (DmaDesc)) ;

   if (dmaDesc.status & DMA_OWN) {
      key = XTM_SW_SCHED_KEY(txSwChannel,txSwdma->txSchedHeadIndex) ;

#if !defined(CONFIG_BCM_FAP) && !defined(CONFIG_BCM_FAP_MODULE)
      bcmPktDma_XtmXmitSwdmaToIudma (txdma, dmaDesc, key) ;
#endif

      if (dmaDesc.status & DMA_WRAP)
         txSwdma->txSchedHeadIndex = 0 ;
      else
         txSwdma->txSchedHeadIndex++ ;
   }
}


static void bcmxtmrt_swsched_swdma_bd_free(XtmRtTxSchedContext *pTxSchedCtxt,
         UINT32 txSwdmaIndex, UINT32 txBdIndex)
{
   BcmPktDma_XtmTxDma  *txdma ;
   volatile DmaDesc    *txBds ;

   txdma = (BcmPktDma_XtmTxDma *) pTxSchedCtxt->uSwDma.pTxPrioSwDmaL[txSwdmaIndex] ;
   txBds = (DmaDesc *) &txdma->txBds[txBdIndex] ;
   txBds->status &= ~DMA_OWN ;
}

static void bcmxtmrt_swsched_recycle (XtmRtTxSchedContext *pXtmTxSchedCtxt)
{
   int    channel ;
   volatile BcmPktDma_XtmTxDma *txdma = NULL ;

   /* Free packets that have been transmitted. */
   for (channel=(int) pXtmTxSchedCtxt->usHwTxQInfosSize-1; channel >= 0; channel--) {

      uint16               txSwdmaIndex, txBdIndex ;
      uint32               txSource, txAddr, rxChannel, key;

      txdma = &pXtmTxSchedCtxt->txHwdma[channel] ;

      /* Recycle the channel buffers first */

      if (txdma->txFreeBds < txdma->ulQueueSize) {

      while (bcmPktDma_XtmFreeXmitBufGet((BcmPktDma_XtmTxDma *)txdma,(uint32 *)&key,
               &txSource, &txAddr, &rxChannel, XTM_HW_DMA, 0x1) == TRUE)
      {
         txSwdmaIndex = XTM_SW_CHANNEL(key) ;
         txBdIndex    = XTM_SW_CHANNEL_INDEX(key) ;
         bcmxtmrt_swsched_swdma_bd_free(pXtmTxSchedCtxt, txSwdmaIndex, txBdIndex) ;
      } /* Clean for channel */
      }
   } /*for channel */
}

/***************************************************************************
 * Function Name: bcmxtmrt_swscheduler
 * Description  : Periodic timer for SW scheduler when in BCM6368 PTM bonding
 *                mode.
 * Returns      : None.
 ***************************************************************************/
static void bcmxtmrt_swscheduler ( PBCMXTMRT_GLOBAL_INFO pGi )
{
   int    i, channel ;
   int    serviced  = 0 ;
   int    credit[MAX_PTM_PRIORITIES] ;
   UINT8  u8SubPrioBitMap ;
   UINT32 queueMask ;
   volatile BcmPktDma_XtmTxDma *txdma = NULL ;
   volatile BcmPktDma_XtmTxDma *txSwdma = NULL ;
   XtmRtTxSchedContext *pXtmTxSchedCtxt  = &pGi->txSchedCtxt ;
   volatile SwSchedWfqInfo *swSchedWfqInfo = NULL ;
   UINT32 swdmaChannel ;

   /* Check for transmitted packet to HW queue(s) to free and then go for
    * scheduling from SW DMAs based on the priority/Weights scheme.
    * Strict Priority/WFQ.
    */
   bcmxtmrt_swsched_recycle (pXtmTxSchedCtxt) ;

   if (pXtmTxSchedCtxt->ulCommand == 0) {
      /* Schedule now */
      for (channel=(int) pXtmTxSchedCtxt->usHwTxQInfosSize-1; channel >= 0; channel--) {

         txdma = &pXtmTxSchedCtxt->txHwdma[channel] ;
         credit [channel] = txdma->txFreeBds ;
         u8SubPrioBitMap = pXtmTxSchedCtxt->ulSubPrioBitMap[channel] ;
         swSchedWfqInfo = &pXtmTxSchedCtxt->wfq [channel] ;
         while (credit[channel] > 0) {

            for (i = pXtmTxSchedCtxt->rrVisitCount[channel]-1; i >= 0 ; i--) {
               queueMask = 0x1 << i ;
               if (queueMask & u8SubPrioBitMap) {

                  txSwdma = pXtmTxSchedCtxt->uSwDma.pTxPrioSwDmaD [channel][i] ;
                  if (txSwdma->txSchedHeadIndex != txSwdma->txTailIndex) {

                     if (txdma->ulAlg == WA_DISABLED) {
                        /* Strict Priority scheduling discipline */
                     bcmxtmrt_swsched_xmit (pXtmTxSchedCtxt, (BcmPktDma_XtmTxDma *) txSwdma,
                           channel, (i+(MAX_SUB_PRIORITIES*channel))) ;
                     }
                     else {
                        if (swSchedWfqInfo->allowanceQuantity[i] == 0) {
                           swSchedWfqInfo->allowanceQuantity[i] =
                              txSwdma->ulWeightValue * (MIN_WFQ_ALLOWANCE+
                                    swSchedWfqInfo->prevMaxSC) -
                              swSchedWfqInfo->SC[i] ;
                           swSchedWfqInfo->sentQuantity[i] = 0 ;
                        }

                        swSchedWfqInfo->sentQuantity[i] += bcmxtmrt_swsched_getxmit_len ((BcmPktDma_XtmTxDma *) txSwdma) ;
                        bcmxtmrt_swsched_xmit (pXtmTxSchedCtxt, (BcmPktDma_XtmTxDma *) txSwdma,
                              channel, (i+(MAX_SUB_PRIORITIES*channel))) ;

                        if (swSchedWfqInfo->sentQuantity[i] > swSchedWfqInfo->allowanceQuantity[i]) {
                           swSchedWfqInfo->SC[i] = swSchedWfqInfo->sentQuantity[i] -
                              swSchedWfqInfo->allowanceQuantity[i] ;
                           if (swSchedWfqInfo->SC[i] > swSchedWfqInfo->currMaxSC)
                              swSchedWfqInfo->currMaxSC = swSchedWfqInfo->SC[i] ;
                           swSchedWfqInfo->allowanceQuantity[i] =  0 ;
                           i -= 1 ;
                        }
                     } /* else part for WFQ */

                     serviced = 1 ;
                     credit[channel]-- ;
                     break ;
                  } /* if i/p exists in q */
                  else {
                     swSchedWfqInfo->SC[i] = 0 ;
                  }
               } /* if (Q) exists */
            } /* for i */

            if (txdma->ulAlg != WA_DISABLED) {
               if (i < 0) {
                  pXtmTxSchedCtxt->rrVisitCount[channel] = MAX_SUB_PRIORITIES ;
                  swSchedWfqInfo->prevMaxSC    = swSchedWfqInfo->currMaxSC ;
                  swSchedWfqInfo->currMaxSC    = 0 ;
               }
               else
                  pXtmTxSchedCtxt->rrVisitCount[channel] = i+1 ;
            }

            if (serviced > 0) {
               pXtmTxSchedCtxt->totalScheduled++ ;
               serviced = 0 ;
            }
            else /* Waste of cycles, scheduler was idle */
               break ;
         } /* while (credit for channel o/p exists) */
      } /* for all the tx channels available */
   }
   else {
      /* For now, If FLUSH/DELETE SW DMA command actions are same, so we do not
       * need to differentiate here.
       */
      swdmaChannel = pXtmTxSchedCtxt->ulCommandParam ;
      channel = swdmaChannel>>3 ;
      txSwdma = pXtmTxSchedCtxt->uSwDma.pTxPrioSwDmaL [swdmaChannel] ;
      txdma = &pXtmTxSchedCtxt->txHwdma[channel] ;

      if (txSwdma->txSchedHeadIndex == txSwdma->txTailIndex) {
         if (txdma->txHeadIndex == txdma->txTailIndex) {
            pXtmTxSchedCtxt->ulCommand = pXtmTxSchedCtxt->ulCommandParam = 0 ;
            txSwdma->txDma->cfg = 0 ; /* Simulate the real HW DMA mechanism */
         }
      }
      else {
         credit[channel] = txdma->txFreeBds ;
         while (credit[channel] > 0) {
            if (txSwdma->txSchedHeadIndex != txSwdma->txTailIndex) {

               bcmxtmrt_swsched_xmit (pXtmTxSchedCtxt, (BcmPktDma_XtmTxDma *) txSwdma,
                     channel, swdmaChannel) ;
               credit[channel]-- ;
               pXtmTxSchedCtxt->totalScheduled++ ;
            }
            else
               break ;
         } /* while (credit) */
      }
   }
}

        /*********************************************************************
        *  ALL SW Scheduler functions for BCM6368 PTM Bonding mode END here  *
        *********************************************************************/


/***************************************************************************
 * Function Name: bcmxtmrt_add_proc_files
 * Description  : Adds proc file system directories and entries.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_add_proc_files( void )
{
    proc_mkdir ("driver/xtm", NULL);

    create_proc_read_entry("driver/xtm/txdmainfo",  0, NULL, ProcDmaTxInfo,  0);
    create_proc_read_entry("driver/xtm/rxdmainfo",  0, NULL, ProcDmaRxInfo,  0);
    create_proc_read_entry("driver/xtm/rxbondctrs", 0, NULL, ProcRxBondCtrs, 0);
    create_proc_read_entry("driver/xtm/txbondinfo", 0, NULL, ProcTxBondInfo, 0);
    create_proc_read_entry("driver/xtm/txschedinfo", 0, NULL, ProcTxSchedInfo, 0);

#ifdef PTM_BONDING_DEBUG
    create_proc_read_entry("driver/xtm/rxbondseq0", 0, NULL, ProcRxBondSeq0, 0);
    create_proc_read_entry("driver/xtm/rxbondseq1", 0, NULL, ProcRxBondSeq1, 0);
#endif
    return(0);
} /* bcmxtmrt_add_proc_files */


/***************************************************************************
 * Function Name: bcmxtmrt_del_proc_files
 * Description  : Deletes proc file system directories and entries.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int bcmxtmrt_del_proc_files( void )
{
    remove_proc_entry("driver/xtm/txschedinfo", NULL);
    remove_proc_entry("driver/xtm/txbondinfo", NULL);
    remove_proc_entry("driver/xtm/rxbondctrs", NULL);
    remove_proc_entry("driver/xtm/txdmainfo", NULL);
    remove_proc_entry("driver/xtm", NULL);

    return(0);
} /* bcmxtmrt_del_proc_files */


/***************************************************************************
 * Function Name: ProcDmaTxInfo
 * Description  : Displays information about transmit DMA channels for all
 *                network interfaces.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt,
    int *eof, void *data)
{
    PBCMXTMRT_GLOBAL_INFO  pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT  pDevCtx;
    BcmPktDma_XtmTxDma    *txdma;
    volatile DmaStateRam  *pStRam = (DmaStateRam *)&pGi->dmaCtrl->stram.s;
    UINT32 i, j;
    int sz = 0;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        pDevCtx = pGi->pDevCtxs[i];
        if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
        {
            for (j = 0; j < pDevCtx->ulTxQInfosSize; j++)
            {
                txdma = pDevCtx->txdma[j];

                sz += sprintf(page + sz, "\nCh %lu, NumTxBds: %lu, HeadIdx: %lu, TailIdx: %lu, FreeBds: %lu\n",
                      txdma->ulDmaIndex, txdma->ulQueueSize, (UINT32)txdma->txHeadIndex,
                      (UINT32)txdma->txTailIndex, (UINT32)txdma->txFreeBds);

                sz += sprintf(page + sz, "BD RingOffset: 0x%08lx, Word1: 0x%08lx\n",                  
                      pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex].state_data & 0x1fff,
                      pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex].desc_len_status);

#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
                sz += sprintf(page + sz, "%s tx_chan_size: %lu, tx_chan_filled: %lu\n",
                      pDevCtx->pDev->name, txdma->ulQueueSize, txdma->ulNumTxBufsQdOne);
#endif                      
            }
        }
    }

#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    sz += sprintf(page + sz, "\next_buf_size: %lu, reserve_buf_size: %lu, tx_total_filled: %lu\n\n",
          pGi->ulNumExtBufs, pGi->ulNumExtBufsRsrvd, pGi->ulNumTxBufsQdAll);

    sz += sprintf(page + sz, "queue_condition: %lu %lu %lu, drop_condition: %lu %lu %lu\n\n",
          pGi->ulDbgQ1, pGi->ulDbgQ2, pGi->ulDbgQ3, pGi->ulDbgD1, pGi->ulDbgD2, pGi->ulDbgD3);
#endif

    *eof = 1;
    return( sz );
} /* ProcDmaTxInfo */

/***************************************************************************
 * Function Name: ProcDmaRxInfo
 * Description  : Displays information about receive DMA channels for all
 *                network interfaces.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int ProcDmaRxInfo(char *page, char **start, off_t off, int cnt,
    int *eof, void *data)
{
    PBCMXTMRT_GLOBAL_INFO  pGi = &g_GlobalInfo;
    BcmPktDma_LocalXtmRxDma *rxdma;
    UINT32 i;
    int sz = 0;

    for( i = 0; i < MAX_RECEIVE_QUEUES; i++ )
    {
        rxdma = (BcmPktDma_LocalXtmRxDma *)&pGi->rxdma[i]->pktDmaRxInfo; 

        if ( rxdma != NULL )
        {

           sz += sprintf(page + sz, "\nCh %lu, NumRxBds: %lu, HeadIdx: %lu, TailIdx: %lu, AssignedBds: %lu\n",
                 i, (UINT32)rxdma->numRxBds, (UINT32)rxdma->rxHeadIndex,
                 (UINT32)rxdma->rxTailIndex, (UINT32)rxdma->rxAssignedBds);

           sz += sprintf(page + sz, "DMA cfg: 0x%08lx, intstat: 0x%08lx, intmask: 0x%08lx\n",
                 rxdma->rxDma->cfg, rxdma->rxDma->intStat, rxdma->rxDma->intMask) ;
        }
    }

    *eof = 1;
    return( sz );
} /* ProcDmaRxInfo */

/***************************************************************************
 * Function Name: ProcTxSchedInfo
 * Description  : Displays information about transmit SW Scheduler.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
static int ProcTxSchedInfo(char *page, char **start, off_t off, int cnt,
    int *eof, void *data)
{
    int sz = 0;
    PBCMXTMRT_GLOBAL_INFO  pGi = &g_GlobalInfo;

    sz += sprintf(page + sz, "Total Scheduled-%ld \n", pGi->txSchedCtxt.totalScheduled) ;
    sz += sprintf(page + sz, "xtm tx sched: Hw Q -%d Hw Tx DM0 %x\n",
                  pGi->txSchedCtxt.usHwTxQInfosSize, (unsigned int) pGi->txSchedCtxt.txHwdma[0].txDma) ;
    sz += sprintf(page + sz, "xtm tx sched: Sw Q -%ld Sw Tx DMA0 %x\n",
                  pGi->txSchedCtxt.ulSwTxQInfosSize, (unsigned int) pGi->txSchedCtxt.uSwDma.pTxPrioSwDmaL[0]);
    sz += sprintf(page + sz, "xtm tx sched: Priomap - %x \n",
                  *((UINT16 *) pGi->txSchedCtxt.ulSubPrioBitMap));
    sz += sprintf(page + sz, "xtm tx sched: Sw dma Head %d Tail %d  \n",
                  pGi->txSchedCtxt.uSwDma.pTxPrioSwDmaL[0]->txSchedHeadIndex,
                  pGi->txSchedCtxt.uSwDma.pTxPrioSwDmaL[0]->txTailIndex);
    sz += sprintf(page + sz, "xtm tx sched: tx dma free Bds  %d QSize %d \n",
                  pGi->txSchedCtxt.txHwdma[0].txFreeBds,
                  (int) pGi->txSchedCtxt.txHwdma[0].ulQueueSize) ;
    *eof = 1;
    return( sz );
} /* ProcTxSchedInfo */

/***************************************************************************
 * Function Name: bcmxtmrt_update_hw_stats
 * Description  : Update the XTMRT transmit counters if appropriate for the
 *                flow. This flows through to the 'ifconfig' counts
 * Returns      : None
 ***************************************************************************/
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
void bcmxtmrt_update_hw_stats(struct net_device *txDev_p, unsigned int hits, unsigned int octets)
{
    PBCMXTMRT_DEV_CONTEXT pDevCtx;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    pDevCtx = netdev_priv(txDev_p);
#else
    pDevCtx = txDev_p->priv;
#endif
    /* Adjust xmit packet counts for flows that tx to XTM */
    pDevCtx->DevStats.tx_packets += hits;

    /* Transmitted bytes are counted in hardware. */

}
#endif

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init(bcmxtmrt_init);
module_exit(bcmxtmrt_cleanup);
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(bcmxtmrt_request);
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
EXPORT_SYMBOL(bcmxtmrt_update_hw_stats);
#endif


#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
/* init XTM IQ thresholds */
static void xtm_rx_init_iq_thresh(int chnl)
{
    int nr_rx_bds;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    {
        nr_rx_bds = bcmPktDma_Bds_p->fap.xtm_rxbds[chnl];

        xtm_rx_dma_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_XTM_LO_THRESH_PCT)/100;
        xtm_rx_dma_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_XTM_HI_THRESH_PCT)/100;
        BCM_XTM_RX_DEBUG("Xtm: rxbds=%u, iqLoThresh=%u, iqHiThresh=%u\n",
                    nr_rx_bds,
                    xtm_rx_dma_iq_thresh[chnl].loThresh,
                    xtm_rx_dma_iq_thresh[chnl].hiThresh);

    }

    {/* DQM */
        nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxdqm[chnl];

        xtm_rx_dqm_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_XTM_LO_THRESH_PCT)/100;
        xtm_rx_dqm_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_XTM_HI_THRESH_PCT)/100;

        BCM_XTM_RX_DEBUG("Xtm: dqm=%u, iqLoThresh=%u, iqHiThresh=%u\n",
                    nr_rx_bds,
                    xtm_rx_dqm_iq_thresh[chnl].loThresh,
                    xtm_rx_dqm_iq_thresh[chnl].hiThresh);
    }
#else
    {
        nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[chnl];

        xtm_rx_dma_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_XTM_LO_THRESH_PCT)/100;
        xtm_rx_dma_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_XTM_HI_THRESH_PCT)/100;
    }
#endif
}

static void xtm_rx_set_iq_thresh( int chnl )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    xtm_set_iq_thresh_dqm(chnl,
                xtm_rx_dqm_iq_thresh[chnl].loThresh,
                xtm_rx_dqm_iq_thresh[chnl].hiThresh);

        bcmPktDma_XtmSetIqDqmThresh(rxdma,
                    xtm_rx_dqm_iq_thresh[chnl].loThresh,
                    xtm_rx_dqm_iq_thresh[chnl].hiThresh);
#endif

    bcmPktDma_XtmSetIqThresh(rxdma,
                xtm_rx_dma_iq_thresh[chnl].loThresh,
                xtm_rx_dma_iq_thresh[chnl].hiThresh);

    BCM_XTM_DEBUG("Xtm: chan=%d iqLoThresh=%d iqHiThresh=%d\n",
        chnl, (int) rxdma->iqLoThresh, (int) rxdma->iqHiThresh );
}

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
/* configure XTM IQ thresholds for DQM */
void xtm_set_iq_thresh_dqm(int channel, uint16 loThresh, uint16 hiThresh)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[channel]->pktDmaRxInfo;

    rxdma->iqLoThreshDqm = loThresh;
    rxdma->iqHiThreshDqm = hiThresh;
    rxdma->iqDroppedDqm  = 0;
    BCM_XTM_DEBUG("XTM: DQM Chnl=%d IQ Thresh lo=%d hi=%d\n",
               channel, (int) rxdma->iqLoThreshDqm,
               (int) rxdma->iqHiThreshDqm );
}


/* Update CPU congestion status based on the DQM IQ thresholds */
void xtm_iq_dqm_update_cong_status( int chnl )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;
    int iqDepth;

    if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED) 
        return;

    if (iqos_fap_xtmRxDqmQueue_hook_g == NULL)
        return;

    /* get DQM queue length */
    iqDepth = iqos_fap_xtmRxDqmQueue_hook_g( chnl );

    if (iqDepth >= rxdma->iqHiThreshDqm)
    {/* high thresh crossed on upside, CPU congestion set */
        iqos_set_cong_status(IQOS_IF_XTM, chnl, IQOS_CONG_STATUS_HI );
    }
    else if (iqDepth <= rxdma->iqHiThreshDqm)
    {/* low thresh crossed on downside, CPU congestion abated */
        iqos_set_cong_status(IQOS_IF_XTM, chnl, IQOS_CONG_STATUS_LO );
    }
    /* donot change the congestion status */
}

/* print the IQ DQM status */
void xtm_iq_dqm_status(void)
{
    int chnl;
    int iqDepth = 0;

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
        BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;

        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
            continue;

        /* get DQM queue length */
        if (iqos_fap_xtmRxDqmQueue_hook_g == NULL)
            iqDepth = 0xFFFF;           /* Invalid value */
        else
            iqDepth = iqos_fap_xtmRxDqmQueue_hook_g( chnl );


        printk("    XTM  %4d %5d %5d %5d %10u %8x\n",
               chnl,
               (int) rxdma->iqLoThreshDqm,
               (int) rxdma->iqHiThreshDqm,
               (int) iqDepth,
               (uint32_t)
#if defined(CC_IQ_STATS)
               rxdma->iqDroppedDqm,
#else
               0,
#endif
               iqos_cpu_cong_g
        );
    }
}
#endif

void xtm_iq_update_cong_status( int chnl )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;
    int thrOfst;
    DmaDesc  dmaDesc;
    volatile DmaDesc *rxBd_pv;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    if (g_Xtm_rx_iudma_ownership[chnl] != HOST_OWNED) return;
#endif

    if (iqos_get_cong_status(IQOS_IF_XTM, chnl) == IQOS_CONG_STATUS_HI)
    {
        /* calculate low threshold ring offset */
        thrOfst = rxdma->rxTailIndex + rxdma->iqLoThresh;

        if (thrOfst >= rxdma->numRxBds)
            thrOfst %= rxdma->numRxBds;

        /* Get the status from Rx BD */
        rxBd_pv = &rxdma->rxBds[thrOfst];
        dmaDesc.word0 = rxBd_pv->word0;

        if ((dmaDesc.status & DMA_OWN) == DMA_OWN)
        { /* low thresh crossed on downside */
            iqos_set_cong_status( IQOS_IF_XTM, chnl, IQOS_CONG_STATUS_LO );
        }
    }
    else
    {
        /* calculate high threshold ring offset */
        thrOfst = rxdma->rxTailIndex + rxdma->iqHiThresh;

        if (thrOfst >= rxdma->numRxBds)
            thrOfst %= rxdma->numRxBds;

        /* Get the status from Rx BD */
        rxBd_pv = &rxdma->rxBds[thrOfst];
        dmaDesc.word0 = rxBd_pv->word0;

        if ((dmaDesc.status & DMA_OWN) == 0)
        {/* high thresh crossed on upside */
            iqos_set_cong_status( IQOS_IF_XTM, chnl, IQOS_CONG_STATUS_HI );
        }
    }
}


/* dump the IQ thresholds, stats and cong status */
void xtm_iq_status(void)
{
    int chnl;

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
        BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        if (g_Xtm_rx_iudma_ownership[chnl] != HOST_OWNED) continue;
#endif

        printk("    XTM  %4d %5d %5d %5d %10u %8x\n",
               chnl,
               (int) rxdma->iqLoThresh,
               (int) rxdma->iqHiThresh,
               (rxdma->numRxBds - rxdma->rxAssignedBds),
               (uint32_t)
#if defined(CC_IQ_STATS)
               rxdma->iqDropped,
#else
               0,
#endif
               iqos_cpu_cong_g
               );
    }
}
#endif


#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
uint32_t xtm_alloc_buf_addr[BPM_XTM_BULK_ALLOC_COUNT];

/* Allocates BPM_XTM_BULK_ALLOC_COUNT number of bufs and assigns to the
 * DMA ring of an XTM RX channel. The allocation is done in groups for
 * optimization.
 */
static inline int xtm_bpm_alloc_buf( BcmXtm_RxDma *rxdma )
{
    UINT8 *data, *pFkBuf;
    int buf_ix;
    uint32_t *pBuf = xtm_alloc_buf_addr;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *pktDmaRxInfo_p =
                                &pGi->rxdma[rxdma->channel]->pktDmaRxInfo;

    if (gbpm_enable_g)
    {
        if ( (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds)
                >= pktDmaRxInfo_p->allocTrig )
        { /* number of used buffers has crossed the trigger threshold */

            if (gbpm_alloc_mult_buf(pktDmaRxInfo_p->bulkAlloc, pBuf)
                    == GBPM_ERROR )
            {
                /* may be temporarily global buffer pool is depleted.
                 * Later try again */
                return GBPM_ERROR;
            }

            pktDmaRxInfo_p->alloc += pktDmaRxInfo_p->bulkAlloc;

            spin_unlock_bh(&pGi->xtmlock_rx);
            for (buf_ix=0; buf_ix < pktDmaRxInfo_p->bulkAlloc; buf_ix++, pBuf++)
            {
                pFkBuf = (UINT8 *) (*pBuf);

                /* Align data buffers on 16-byte boundary - Apr 2010 */
                data = PFKBUFF_TO_PDATA(pFkBuf, RXBUF_HEAD_RESERVE);
                FlushAssignRxBuffer(rxdma->channel, data,
                                     (uint8_t*) pFkBuf + RXBUF_ALLOC_SIZE);
            }
            spin_lock_bh(&pGi->xtmlock_rx);
        }
    }

    return GBPM_SUCCESS;
}


/* dump the BPM status, stats */
void xtm_bpm_status(void)
{
    int chnl;

    for(chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
        BcmPktDma_XtmRxDma *pktDmaRxInfo_p = &pGi->rxdma[chnl]->pktDmaRxInfo;

        printk("    XTM  %4d %10u %6d %10u %6d %7d %4d %4d\n",
               chnl, (uint32_t) pktDmaRxInfo_p->alloc,
               (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds),
               (uint32_t) pktDmaRxInfo_p->free, 0,
               (int) pktDmaRxInfo_p->numRxBds,
               (int) pktDmaRxInfo_p->allocTrig,
               (int) pktDmaRxInfo_p->bulkAlloc
               );
    }
}
#endif

static inline int xtm_bpm_free_buf(BcmXtm_RxDma *rxdma, uint8 *pData)
{
    if (gbpm_enable_g)
    {
        PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
        BcmPktDma_XtmRxDma *pktDmaRxInfo_p =
                                    &pGi->rxdma[rxdma->channel]->pktDmaRxInfo;
        gbpm_free_buf((uint32_t *) PDATA_TO_PFKBUFF(pData,RXBUF_HEAD_RESERVE));
        pktDmaRxInfo_p->free--;
    }

    return GBPM_SUCCESS;
}


/* Allocate the buffer ring for an XTM RX channel */
static int xtm_bpm_alloc_buf_ring(BcmXtm_RxDma *rxdma, UINT32 num)
{
    UINT8 *data, *pFkBuf;
    UINT32 context = 0;
    UINT32 buf_ix;

    RECYCLE_CONTEXT(context)->channel = rxdma->channel;

    for (buf_ix=0; buf_ix < num; buf_ix++)
    {
        if( (pFkBuf = (uint8_t *) gbpm_alloc_buf()) == NULL )
        {
            printk(KERN_NOTICE CARDNAME ": Low memory.\n");
            return GBPM_ERROR;
        }

        /* Align data buffers on 16-byte boundary - Apr 2010 */
        data = PFKBUFF_TO_PDATA(pFkBuf, RXBUF_HEAD_RESERVE);

        /* Place a FkBuff_t object at the head of pFkBuf */
        fkb_preinit(pFkBuf, (RecycleFuncP)bcmxtmrt_recycle, context);

        cache_flush_region(data, (uint8_t*)pFkBuf + RXBUF_ALLOC_SIZE);
        bcmPktDma_XtmFreeRecvBuf(&rxdma->pktDmaRxInfo, (unsigned char *)data);
    }

    return GBPM_SUCCESS;
}


/* Free the buffer ring for an XTM RX channel */
static void xtm_bpm_free_buf_ring( BcmXtm_RxDma *rxdma )
{
    uint32 rxAddr = 0;

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    while (bcmPktDma_XtmRecvBufGet(&rxdma->pktDmaRxInfo, &rxAddr) == TRUE)
#endif
    {
        if ((UINT8 *) rxAddr != NULL)
            xtm_bpm_free_buf(rxdma, (uint8 *) rxAddr);
    }
}


/*
 *-----------------------------------------------------------------------------
 * function   : xtm_bpm_txq_thresh
 * description: configures the queue thresholds
 *-----------------------------------------------------------------------------
 */
static int xtm_bpm_txq_thresh( PBCMXTMRT_DEV_CONTEXT pDevCtx,
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId, int qid)
{

    uint32 usSpeed;
    int nr_tx_bds;

    usSpeed = (pDevCtx->MibInfo.ulIfSpeed >> 20) + 1;   /* US in Mbps */
    pTxQId->ulLoThresh = XTM_BPM_TXQ_LO_THRESH(usSpeed);
    pTxQId->ulHiThresh = XTM_BPM_TXQ_HI_THRESH(usSpeed);
    pTxQId->ulDropped = 0;

    BCM_XTM_DEBUG("XTM Tx qId[%d] ulIfSpeed=%d, usSpeed=%d\n",
        qid, (int) pDevCtx->MibInfo.ulIfSpeed, (int) usSpeed);

    BCM_XTM_DEBUG("XTM Tx qId[%d] ulLoThresh=%d, ulHiThresh=%d\n",
        qid, (int) pTxQId->ulLoThresh, (int) pTxQId->ulHiThresh);


#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    nr_tx_bds = bcmPktDma_Bds_p->fap.xtm_txbds[qid];
#else
    nr_tx_bds = bcmPktDma_Bds_p->host.xtm_txbds[qid];
#endif

    pTxQId->ulQueueSize = nr_tx_bds;

    return GBPM_SUCCESS;
}


/* dumps the BPM TxQ thresholds */
void xtm_bpm_dump_txq_thresh(void)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    BcmPktDma_XtmTxDma   *txdma;
    UINT32 i, j;

    for( i = 0; i < MAX_DEV_CTXS; i++ )
    {
        pDevCtx = pGi->pDevCtxs[i];
        if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
        {
            if ( pDevCtx->ulLinkState == LINK_UP )
            {
                for (j = 0; j < pDevCtx->ulTxQInfosSize; j++)
                {
                    txdma = pDevCtx->txdma[j];

                    if (txdma->txEnabled == 1)
                    {
                        printk( "    XTM  %4d %5d %5d %10u\n",
                               (int) j, (int) txdma->ulLoThresh,
                               (int) txdma->ulHiThresh,
                               (unsigned int) txdma->ulDropped
                               );
                        printk( "\n" );
                    }
                }
            }
        }
    }
}
#endif

