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
 * File Name  : bcmxtmrtimpl.h
 *
 * Description: This file contains constant definitions and structure
 *              definitions for the BCM6368 ATM/PTM network device driver.
 ***************************************************************************/

#if !defined(_BCMXTMRTIMPL_H)
#define _BCMXTMRTIMPL_H

#include <bcmtypes.h>
#include "bcmnet.h"
#include "bcmxtmrt.h"
#include "bcmPktDma_structs.h"

#define XTM_CACHE_SMARTFLUSH

#define TEQ_DATA_VCID    15

#define MAX_DEV_CTXS                16
#define MAX_MATCH_IDS               128
#define MAX_DEFAULT_MATCH_IDS       16
#define ENET_8021Q_SIZE             4
#define MAX_MTU_SIZE                (1500 + 14 + 4 + ENET_8021Q_SIZE)
#ifdef XTM_CACHE_SMARTFLUSH
#define MAX_RFC2684_HDR_SIZE        10
#else
#define MAX_RFC2684_HDR_SIZE        SKB_DATA_ALIGN(10)
#endif
#define SAR_DMA_MAX_BURST_LENGTH    8
#define RXBUF_FKB_INPLACE           ((sizeof(FkBuff_t) + 0x0f) & ~0x0f)
#define RXBUF_HEAD_RESERVE          ((176 + 0x3f) & ~0x3f)
#define RXBUF_SIZE                  ((MAX_MTU_SIZE +                    \
                                        MAX_RFC2684_HDR_SIZE +          \
                                      (SAR_DMA_MAX_BURST_LENGTH * 8)    \
                                      + 0x3f) & ~0x3f)
#define RXBUF_ALLOC_SIZE            SKB_DATA_ALIGN(RXBUF_FKB_INPLACE +  \
                                        RXBUF_HEAD_RESERVE +            \
                                        RXBUF_SIZE+                     \
                                        sizeof(struct skb_shared_info))
#define MAX_BUFMEM_BLOCKS           64
#define SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define RFC1626_MTU                 9180

#define SAR_RX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_0
#define SAR_TX_INT_ID_BASE          INTERRUPT_ID_ATM_DMA_4
#define SAR_RX_DMA_BASE_CHAN        0
#define NR_SAR_RX_DMA_CHANS         2
#define SAR_TX_DMA_BASE_CHAN        4
#define NR_SAR_TX_DMA_CHANS         16
#define SAR_TIMEOUT                 (HZ/20)
#define INVALID_VCID                0xff
#ifndef CARDNAME
#define CARDNAME                    "bcmxtmrt"
#endif

#define CACHE_TO_NONCACHE(x)        KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)        KSEG0ADDR(x)

#define XTMRT_UNINITIALIZED         0
#define XTMRT_INITIALIZED           1
#define XTMRT_RUNNING               2

#define SKB_PROTO_ATM_CELL          0xf000
#define XTM_POLL_DONE               0x80000000

/* Circuit types. */
#define XCT_TRANSPARENT             0x00000001
#define XCT_AAL0_PKT                0x00000002
#define XCT_AAL0_CELL               0x00000003
#define XCT_OAM_F5_SEG              0x00000004
#define XCT_OAM_F5_E2E              0x00000005
#define XCT_RM                      0x00000006
#define XCT_AAL5                    0x00000007
#define XCT_ASM_P0                  0x00000008
#define XCT_ASM_P1                  0x00000009
#define XCT_ASM_P2                  0x0000000a
#define XCT_ASM_P3                  0x0000000b
#define XCT_OAM_F4_SEG              0x0000000c
#define XCT_OAM_F4_E2E              0x0000000d
#define XCT_TEQ                     0x0000000e
#define XCT_PTM                     0x0000000f

/* Transmit Buffer Descriptor frame status word for ATM/PTM. */
#define FSTAT_MASK                  0x00000fff
#define FSTAT_ATM_VCID_MASK         0x0000000f
#define FSTAT_ATM_VCID_SHIFT        0
#define FSTAT_PTM_CRC               0x00000001
#define FSTAT_PTM_ENET_FCS          0x00000002
#define FSTAT_CT_MASK               0x000000f0
#define FSTAT_CT_SHIFT              4
#define FSTAT_CT_TRANSPARENT        (XCT_TRANSPARENT << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL0_PKT           (XCT_AAL0_PKT    << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL0_CELL          (XCT_AAL0_CELL   << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F5_SEG         (XCT_OAM_F5_SEG  << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F5_E2E         (XCT_OAM_F5_E2E  << FSTAT_CT_SHIFT)
#define FSTAT_CT_RM                 (XCT_RM          << FSTAT_CT_SHIFT)
#define FSTAT_CT_AAL5               (XCT_AAL5        << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P0             (XCT_ASM_P0      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P1             (XCT_ASM_P1      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P2             (XCT_ASM_P2      << FSTAT_CT_SHIFT)
#define FSTAT_CT_ASM_P3             (XCT_ASM_P3      << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F4_SEG         (XCT_OAM_F4_SEG  << FSTAT_CT_SHIFT)
#define FSTAT_CT_OAM_F4_E2E         (XCT_OAM_F4_E2E  << FSTAT_CT_SHIFT)
#define FSTAT_CT_TEQ                (XCT_TEQ         << FSTAT_CT_SHIFT)
#define FSTAT_CT_PTM                (XCT_PTM         << FSTAT_CT_SHIFT)
#define FSTAT_COMMON_INS_HDR_EN     0x00000100
#define FSTAT_COMMON_HDR_INDEX_MASK 0x00000600
#define FSTAT_COMMON_HDR_INDEX_SHIFT 9
#define FSTAT_INDEX_CI              0x00000100
#define FSTAT_INDEX_CLP             0x00000200
#define FSTAT_INDEX_USE_ALT_GFC     0x00000400
#define FSTAT_MODE_INDEX            0x00000000
#define FSTAT_MODE_COMMON           0x00000800

/* Receive Buffer Descriptor frame status word for ATM/PTM. */
#define FSTAT_MATCH_ID_MASK         0x0000007f             
#define FSTAT_MATCH_ID_SHIFT        0
#define FSTAT_PACKET_CELL_MASK      0x00000400
#define FSTAT_PACKET                0x00000000
#define FSTAT_CELL                  0x00000400
#define FSTAT_ERROR                 0x00000800

/* First byte of a received cell. */
#define CHDR_CT_MASK                0x0f
#define CHDR_CT_SHIFT               0
#define CHDR_CT_OAM_F5_SEG          (XCT_OAM_F5_SEG  << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F5_E2E          (XCT_OAM_F5_E2E  << CHDR_CT_SHIFT)
#define CHDR_CT_RM                  (XCT_RM          << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P0              (XCT_ASM_P0      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P1              (XCT_ASM_P1      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P2              (XCT_ASM_P2      << CHDR_CT_SHIFT)
#define CHDR_CT_ASM_P3              (XCT_ASM_P3      << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F4_SEG          (XCT_OAM_F4_SEG  << CHDR_CT_SHIFT)
#define CHDR_CT_OAM_F4_E2E          (XCT_OAM_F4_E2E  << CHDR_CT_SHIFT)
#define CHDR_PORT_MASK              0x60
#define CHDR_PORT_SHIFT             5
#define CHDR_ERROR                  0x80
#define CHDR_ERROR_MISC             0x01
#define CHDR_ERROR_CRC              0x02
#define CHDR_ERROR_CAM              0x04
#define CHDR_ERROR_HEC              0x08
#define CHDR_ERROR_PORT             0x10

/****************************************************************************
   Logging Defines
****************************************************************************/

/* #define BCM_XTM_LOG      
   #define BCM_XTM_RX_LOG  
   #define BCM_XTM_TX_LOG   
   #define BCM_XTM_LINK_LOG */

#if defined(BCM_XTM_LOG)
#define BCM_XTM_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_DEBUG(fmt, arg...)
#define BCM_XTM_INFO(fmt, arg...)
#define BCM_XTM_NOTICE(fmt, arg...)
#define BCM_XTM_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_RX_LOG)
#define BCM_XTM_RX_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_RX_DEBUG(fmt, arg...)
#define BCM_XTM_RX_INFO(fmt, arg...)
#define BCM_XTM_RX_NOTICE(fmt, arg...)
#define BCM_XTM_RX_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_TX_LOG)
#define BCM_XTM_TX_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_TX_DEBUG(fmt, arg...)
#define BCM_XTM_TX_INFO(fmt, arg...)
#define BCM_XTM_TX_NOTICE(fmt, arg...)
#define BCM_XTM_TX_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_LINK_LOG)
#define BCM_XTM_LINK_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_INFO(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_LINK_DEBUG(fmt, arg...)
#define BCM_XTM_LINK_INFO(fmt, arg...)
#define BCM_XTM_LINK_NOTICE(fmt, arg...)
#define BCM_XTM_LINK_ERROR(fmt, arg...)
#endif

/* Information about a DMA transmit channel. A device instance may use more
 * than one transmit DMA channel. A DMA channel corresponds to a transmit queue.
 */
 
struct bcmxtmrt_dev_context;   /* forward reference */

#define BcmPktDma_XtmRxDma BcmPktDma_LocalXtmRxDma
#define RXBDINFO BcmPktDma_XtmRxDma
#define PRXBDINFO BcmPktDma_XtmRxDma *

typedef struct BcmXtm_RxDma {

    BcmPktDma_XtmRxDma pktDmaRxInfo;
    struct sk_buff  *freeSkbList;
    unsigned char   *buf_pool[MAX_BUFMEM_BLOCKS];
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#ifndef FAP_4KE
    /* SKB Pool now dynamically allocated - Apr 2010 */
    unsigned char    *skbs_p;
#endif
#endif
    int              rxIrq; 
    int              channel; 
} BcmXtm_RxDma;
              
#define BcmPktDma_XtmTxDma BcmPktDma_LocalXtmTxDma
#define TXQINFO BcmPktDma_XtmTxDma
#define PTXQINFO BcmPktDma_XtmTxDma *

/* Struct added for xtmrt dmaStatus field generation for xtm flows - Apr 2010 */
typedef struct dev_params
{
    UINT32 ulFlags;
    UINT32 ulHdrType;
    UINT8  ucTxVcid;
} DEV_PARAMS, *PDEV_PARAMS;


#define PACKET_BLOG        0
#define PACKET_NORMAL      1

#include "bcmxtmrtbond.h"

#ifndef FAP_4KE

#if defined(AEI_VDSL_STATS_DIAG)
struct xtm_dev_stats {
    unsigned long rx_packets;
    unsigned long long rx_bytes;
    unsigned long tx_packets;
    unsigned long long tx_bytes;
    unsigned long rx_multicast_packets;
    unsigned long long rx_multicast_bytes;
    unsigned long tx_multicast_packets;
    unsigned long long tx_multicast_bytes;
    unsigned long unicast_discarded_packets;
    unsigned long multicast_discarded_packets;
};
#endif

/* The definition of the driver control structure */
typedef struct bcmxtmrt_dev_context
{
    /* Linux structures. */
    struct net_device *pDev;        
    struct net_device_stats DevStats;
    IOCTL_MIB_INFO MibInfo;
    struct ppp_channel Chan;

    /* ATM/PTM fields. */
    XTM_ADDR Addr;
    UINT32 ulLinkState;
    UINT32 ulLinkUsRate[MAX_BOND_PORTS] ;
    UINT32 ulLinkDsRate ;
    UINT32 ulTrafficType ;
    UINT32 ulPortDataMask ;
    UINT32 ulOpenState;
    UINT32 ulAdminStatus;
    UINT32 ulHdrType;
    UINT32 ulEncapType; /* IPoA, PPPoA, or EoA[bridge,MER,PPPoE] */
    UINT32 ulFlags;

    /* Transmit fields. */
    UINT8 ucTxVcid;
    UINT32 ulTxQInfosSize;
    BcmPktDma_XtmTxDma *txdma[MAX_TRANSMIT_QUEUES];
    BcmPktDma_XtmTxDma *pTxQids[MAX_TRANSMIT_QUEUES];
    BcmPktDma_XtmTxDma *pHighestPrio;
    BcmPktDma_XtmTxDma *pTxPriorities[MAX_PTM_PRIORITIES][MAX_PHY_PORTS][MAX_SUB_PRIORITIES];

    /* DmaKeys, DmaSources, DmaAddresses now allocated with txBds - Apr 2010 */

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Device params to be passed to the FAP on tx enable - Apr 2010 */
    DEV_PARAMS devParams;
#endif

#if defined(AEI_VDSL_TOOLBOX)
    UINT16 usMirrorInFlags;
    UINT16 usMirrorOutFlags;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    int matchVlanId;
#endif
#else
    /*Port Mirroring fields*/
    char szMirrorIntfIn[MIRROR_INTF_SIZE];
    char szMirrorIntfOut[MIRROR_INTF_SIZE];
#endif

#if defined(AEI_VDSL_SMARTLED)
    UINT8 inetTrafficBlinkEnable;
    UINT8 inetAmberEnable;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    struct napi_struct napi;
#endif
#if defined(AEI_VDSL_STATS_DIAG)
    struct xtm_dev_stats dev_stats;
#endif
} BCMXTMRT_DEV_CONTEXT, *PBCMXTMRT_DEV_CONTEXT;

/* Information that is global to all network device instances. */
#define TXDMACTRL(pDevCtx)       g_GlobalInfo.dmaCtrl
#define TXDMATYPE(pDevCtx)       XTM_HW_DMA

typedef struct bcmxtmrt_global_info
{
    /* Linux structures. */
    PBCMXTMRT_DEV_CONTEXT pDevCtxs[MAX_DEV_CTXS];
    PBCMXTMRT_DEV_CONTEXT pDevCtxsByMatchId[MAX_MATCH_IDS];
    struct timer_list Timer;
    struct atm_dev *pAtmDev;
    struct net_device *pTeqNetDev;  
    UINT32 ulDevCtxMask;

    spinlock_t xtmlock_tx;
    spinlock_t xtmlock_rx;
    spinlock_t xtmlock_rx_regs;

    /* DMA, BD and buffer fields. */
    BcmXtm_RxDma       *rxdma[MAX_RECEIVE_QUEUES];
    volatile DmaRegs   *dmaCtrl;
    
    UINT32 ulIntEnableMask;

    /* Global transmit queue fields. */
    UINT32 ulNumExtBufs;
    UINT32 ulNumExtBufsRsrvd;
    UINT32 ulNumExtBufs90Pct;
    UINT32 ulNumExtBufs50Pct;
    UINT32 ulNumTxQs;
    UINT32 ulNumTxBufsQdAll;
    UINT32 ulDbgQ1;
    UINT32 ulDbgQ2;
    UINT32 ulDbgQ3;
    UINT32 ulDbgD1;
    UINT32 ulDbgD2;
    UINT32 ulDbgD3;

    /* Callback functions. */
    XTMRT_CALLBACK pfnOamHandler;
    void *pOamContext;
    XTMRT_CALLBACK pfnAsmHandler;
    void *pAsmContext;

    /* MIB counter registers. */
    UINT32 *pulMibTxOctetCountBase;
    UINT32 ulMibRxClrOnRead;
    UINT32 *pulMibRxCtrl;
    UINT32 *pulMibRxMatch;
    UINT32 *pulMibRxOctetCount;
    UINT32 *pulMibRxPacketCount;
    UINT32 *pulRxCamBase;

    /* Bonding information */
    XtmBondConfig          bondConfig ;
    XtmRtPtmTxBondHeader   ptmBondHdr [XTMRT_PTM_BOND_MAX_FRAG_PER_PKT] ;
    XtmRtPtmBondInfo       ptmBondInfo ;

    /* Everything else. */
    UINT32 ulChipRev ;
    UINT32 ulDrvState ;
} BCMXTMRT_GLOBAL_INFO, *PBCMXTMRT_GLOBAL_INFO;

extern BCMXTMRT_GLOBAL_INFO g_GlobalInfo;

/* Function Prototypes */
UINT32 bcmxtmrt_process_rx_pkt( PBCMXTMRT_DEV_CONTEXT pDevCtx, BcmXtm_RxDma *rxdma,
                              FkBuff_t *pFkb, UINT16 bufStatus, int delLen, int trailerDelLen);
int bcmxtmrt_ptmbond_add_hdr(pNBuff_t *ppNBuff, struct sk_buff **ppNBuffSkb,
                             UINT8 **ppData, int *pLen, UINT32 ulPtmPrioIdx);
int ProcTxBondInfo(char *page, char **start, off_t off, int cnt, int *eof, void *data);

#endif /* FAP_4KE */

#endif /* _BCMXTMRTIMPL_H */
