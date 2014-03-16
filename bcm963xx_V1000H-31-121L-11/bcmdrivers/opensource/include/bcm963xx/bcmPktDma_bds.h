#ifndef __PKTDMA_BDS_H_INCLUDED__
#define __PKTDMA_BDS_H_INCLUDED__

#if defined(CONFIG_BCM96816)
// bill #define ENET_MAX_MTU_SIZE       (1528 + 4)    /* Body(1500) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) */
#define ENET_MAX_MTU_SIZE       (2076 + 4)    /* Body(2048) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) */
#else
#define ENET_MAX_MTU_SIZE       (1528 + 4)    /* Body(2048) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) */
#endif

#define ENET_MIN_MTU_SIZE       60            /* Without FCS */
#define ENET_MIN_MTU_SIZE_EXT_SWITCH       64            /* Without FCS */

#define DMA_MAX_BURST_LENGTH    8       /* in 64 bit words */
#define RX_BONDING_EXTRA        0
#define RX_ENET_FKB_INPLACE     sizeof(FkBuff_t)
#define RX_ENET_SKB_HEADROOM    176
#define SKB_ALIGNED_SIZE        ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define RX_BUF_LEN              ((ENET_MAX_MTU_SIZE + 63) & ~63)
#define RX_BUF_SIZE             (SKB_DATA_ALIGN(RX_ENET_FKB_INPLACE  + \
                                                RX_ENET_SKB_HEADROOM + \
                                                RX_BONDING_EXTRA     + \
                                                RX_BUF_LEN           + \
                                                sizeof(struct skb_shared_info)))

#define NON_JUMBO_MAX_MTU_SIZE  (1528 + 4)    /* Body(1500) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) */
#define NON_JUMBO_RX_BUF_LEN    ((NON_JUMBO_MAX_MTU_SIZE + 63) & ~63)

#define NON_JUMBO_RX_BUF_SIZE    (SKB_DATA_ALIGN(RX_ENET_FKB_INPLACE  + \
                                                RX_ENET_SKB_HEADROOM + \
                                                RX_BONDING_EXTRA     + \
                                                NON_JUMBO_RX_BUF_LEN + \
                                                sizeof(struct skb_shared_info)))

#endif /* __PKTDMA_BDS_H_INCLUDED__ */
