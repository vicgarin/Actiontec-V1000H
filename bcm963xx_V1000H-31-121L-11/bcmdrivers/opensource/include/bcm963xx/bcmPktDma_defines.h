#ifndef __PKTDMA_DEFINES_H_INCLUDED__
#define __PKTDMA_DEFINES_H_INCLUDED__

#define XTMFREE_FORCE_FREE    1
#define XTMFREE_NO_FORCE_FREE 0

#if defined(CONFIG_BCM96816) || defined(CHIP_6816)
#define ENET_TX_EGRESS_QUEUES_MAX  8
#else
#define ENET_TX_EGRESS_QUEUES_MAX  4
#endif

#if defined(CONFIG_BCM96362) && defined(CONFIG_BCM_PKTDMA)
#define ENET_RX_CHANNELS_MAX CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS
#define ENET_TX_CHANNELS_MAX CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS
#elif defined(CONFIG_BCM96362) || (defined(CONFIG_BCM963268))
/* Increase these from 1 to 2 to support rx & tx splitting - Oct 2010 */
#define ENET_RX_CHANNELS_MAX  2
#define ENET_TX_CHANNELS_MAX  2
#elif defined(CONFIG_BCM96368)
#define ENET_RX_CHANNELS_MAX  2
#define ENET_TX_CHANNELS_MAX  2
#else
#define ENET_RX_CHANNELS_MAX  4
#define ENET_TX_CHANNELS_MAX  4
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define XTM_RX_CHANNELS_MAX   2
#define XTM_TX_CHANNELS_MAX   16
#else
#define XTM_RX_CHANNELS_MAX   4
#define XTM_TX_CHANNELS_MAX   16
#endif

/*
 * -----------------------------------------------------------------------------
 * 1. Added % memory used by BPM to menuconfig. Set default to 15% of memory.
 * 2. For 64MB  #of RXBDs and #of buffers doubles to that of 32MB
 * 3. BPM will be disabled on 6368 chip on 32MB boards.
 * 4. Mini-jumbo packets (size of 2K) support on 6816 only.
 * 5. 6819BHR will be treated similar to 6816.
 * 6. Number of buffers on 6816 is going to be less since jumbo packets will
 *    be enabled
 * 7. Number of RXBDs will be computed as % of total buffers. i.e. Ethernet
 *    host BDs on 40% of total buffers. Unless CMF is compiled in and then
 *    it is reduced to 20% of total
 * 8. A fixed number of 100 BDs for additional DMA channels on 6816 does not
 *    need change for different memory sizes i.e. don't double it to 200 on
 *    64MB board
 * 9. RX Multiple channels
 *    - supported for 6816 only.
 *    - Ingress Qos supported on default channel (Channel 0) only.
 *    - All other channels (n) are allocated 100 desc each, and are treated
 *      as high priority by IQ.
 *10. FAP has fixed number of BDs because of limited DSPRAM/PSRAM
 *    Eth Chnl: # of RXBDs = 600
 *    XTM Chnl: # of RXBDs = 200
 *    Eth (WoE) Chnl: # of RXBDs = 800 (min)
 *11. 6368 has 2 times the requirements for RXBDs because of CMF FWD driver.
 *    XTM bonding needs at least 512 RXBDs because of latency requirements.
 *
 *12. Table below gives rough estimate of RXBDs
 * -----------------------------------------------------------------------------
 * Chip | Mem |     # of RXBDs                | # of | Comments
 * -----------------------------------------------------------------------------
 *            | Host              | FAP/FWD   | buff |
 * -----------------------------------------------------------------------------
 *            | ETH         | XTM | ETH | XTM |      |
 * -----------------------------------------------------------------------------
 * 6328 | 32M | 800         | 200 |     |     | 2000 |
 * 6362 | 32M |             |     | 600 | 200 | 2000 | XTM WAN
 * 6362 | 32M | 800         |     | 600 |     | 2000 | WoE (iuDMA split)
 * 6816 | 32M | 800+(n*100) |     |     |     | 2000 | Default Chnl and n other
 *      |     |             |     |     |     |      | channels
 * 6368 | 64M | 800         | 600 | 800 | 600 | 4000 | 6368 requires atleast
 *      |     |             |     |     |     |      | 512 RXBDs for XTM bonding
 * -----------------------------------------------------------------------------
 */
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/* Channel-0 is default */
/* % of number of buffers assigned to RXBDs */
#define ENET_DEF_RXBDS_BUF_PRCNT        40
#define XTM_DEF_RXBDS_BUF_PRCNT         10

/* Fixed # of RXBDs for non-default channels */
#define HOST_ENET_NON_DEF_CHNL_NR_RXBDS 100
#define HOST_XTM_NON_DEF_CHNL_NR_RXBDS  16

#define HOST_ENET_NR_RXBDS              600

#if defined(CONFIG_BCM96338) || defined(CHIP_6338)
/* 38 needs a bigger cell queue for soft sar (64 bytes each) */
#define HOST_XTM_NR_RXBDS               500
#else
#define HOST_XTM_NR_RXBDS               HOST_ENET_NR_RXBDS
#endif


/* Host/MIPS: # of TXBDs for IuDMA managed by host */
#define HOST_ENET_NR_TXBDS              200
#define HOST_XTM_NR_TXBDS               400

#define MOCA_TXQ_DEPTH_MAX              3000

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
/* FAP: # of buffers assigned to RXBDs */
#define FAP_ENET_NR_RXBDS               600     /* FAP chnl */
#define FAP_XTM_NR_RXBDS                200
#define HOST_ENET_NR_RXBDS_MIN          800     /* Host chnl, WANoE case */
#define HOST_XTM_NR_RXBDS_MIN           512

/* FAP: # of RXBDs for non-default channels */
#define FAP_ENET_NON_DEF_CHNL_NR_RXBDS  HOST_ENET_NON_DEF_CHNL_NR_RXBDS
#define FAP_XTM_NON_DEF_CHNL_NR_RXBDS   HOST_XTM_NON_DEF_CHNL_NR_RXBDS

/* FAP: # of TXBDs for IuDMA managed by FAP */
#define FAP_ENET_NR_TXBDS               HOST_ENET_NR_TXBDS
#define FAP_XTM_NR_TXBDS                HOST_XTM_NR_TXBDS
#endif

#if defined(CONFIG_BCM96368)
/* FWD: # of buffers assigned to RXBDs
 * XTM should be atleast 512 because of bonding */
#define HOST_ENET_NR_RXBDS_MIN           600
#define HOST_XTM_NR_RXBDS_MIN            512

/* FWD: # of RXBDs for non-default channels */
#define FWD_ENET_NON_DEF_CHNL_NR_RXBDS  HOST_ENET_NON_DEF_CHNL_NR_RXBDS
#define FWD_XTM_NON_DEF_CHNL_NR_RXBDS   HOST_XTM_NON_DEF_CHNL_NR_RXBDS

/* FWD: # of RXBDs for IuDMA managed by FWD */
#define FWD_ENET_NR_TXBDS               HOST_ENET_NR_TXBDS
#define FWD_XTM_NR_TXBDS                HOST_XTM_NR_TXBDS
#endif

#else // (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))


/* Channel-0 is default */
/* Fixed # of RXBDs for non-default channels */
#define HOST_ENET_NON_DEF_CHNL_NR_RXBDS 100
#define HOST_XTM_NON_DEF_CHNL_NR_RXBDS  16

#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
#define HOST_ENET_NR_RXBDS              4000
#else
#define HOST_ENET_NR_RXBDS              400
#endif


#if defined(CONFIG_BCM96338) || defined(CHIP_6338)
/* 38 needs a bigger cell queue for soft sar (64 bytes each) */
#define HOST_XTM_NR_RXBDS               500
#else
#define HOST_XTM_NR_RXBDS               HOST_ENET_NR_RXBDS
#endif


/* Host/MIPS: # of TXBDs for IuDMA managed by host */
#define HOST_ENET_NR_TXBDS              200
#define HOST_XTM_NR_TXBDS               HOST_ENET_NR_RXBDS

#define MOCA_TXQ_DEPTH_MAX              3000

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
/* FAP: # of buffers assigned to RXBDs */
#define FAP_ENET_NR_RXBDS               HOST_ENET_NR_RXBDS
#define FAP_XTM_NR_RXBDS                HOST_XTM_NR_RXBDS

/* FAP: # of RXBDs for non-default channels */
#define FAP_ENET_NON_DEF_CHNL_NR_RXBDS  HOST_ENET_NON_DEF_CHNL_NR_RXBDS
#define FAP_XTM_NON_DEF_CHNL_NR_RXBDS   HOST_XTM_NON_DEF_CHNL_NR_RXBDS

/* FAP: # of TXBDs for IuDMA managed by FAP */
#define FAP_ENET_NR_TXBDS               HOST_ENET_NR_TXBDS
#define FAP_XTM_NR_TXBDS                HOST_XTM_NR_TXBDS
#endif

#if defined(CONFIG_BCM96368)
/* FWD: # of buffers assigned to RXBDs
 * XTM should be atleast 512 because of bonding */
#define FWD_ENET_NR_RXBDS               600
#define FWD_XTM_NR_RXBDS                512

/* FWD: # of RXBDs for non-default channels */
#define FWD_ENET_NON_DEF_CHNL_NR_RXBDS  HOST_ENET_NON_DEF_CHNL_NR_RXBDS
#define FWD_XTM_NON_DEF_CHNL_NR_RXBDS   HOST_XTM_NON_DEF_CHNL_NR_RXBDS

/* FWD: # of TXBDs for IuDMA managed by FWD */
#define FWD_ENET_NR_TXBDS               500
#define FWD_XTM_NR_TXBDS                400
#endif
#endif // (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))


#endif /* __PKTDMA_DEFINES_H_INCLUDED__ */
