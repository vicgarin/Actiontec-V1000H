/*
 Copyright 2002-2010 Broadcom Corp. All Rights Reserved.

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
#ifndef _BCMENET_H_
#define _BCMENET_H_

#ifdef FAP_4KE
#include "Fap4keOsDeps.h"
#else
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <bcm_map.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include "boardparms.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <bcmnet.h>
#include <bcm/bcmswapitypes.h>
#include <linux/version.h>
#include "bcmPktDma_structs.h"
#endif

/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
/* In order for ATM shaping to work correctly,
 * the number of receive BDS/buffers = # tx queues * # buffers per tx queue
 * (80 ENET buffers = 8 tx queues * 10 buffers per tx queue)
 */
#define ENET_CACHE_SMARTFLUSH

/* misc. configuration */
#define DMA_FC_THRESH_LO        5
#define DMA_FC_THRESH_HI        (NR_RX_BDS_MIN / 2)
/* IEEE 802.3 Ethernet constant */
#define ETH_CRC_LEN             4
#define ETH_MULTICAST_BIT       0x01

#define CACHE_TO_NONCACHE(x)    KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)    KSEG0ADDR(x)

#define ERROR(x)        printk x
#ifndef ASSERT
#define ASSERT(x)       if (x); else ERROR(("assert: "__FILE__" line %d\n", __LINE__)); 
#endif

#if defined(DUMP_TRACE)
#define TRACE(x)        printk x
#else
#define TRACE(x)
#endif

#define NUM_PORTS                   1

#if defined(CONFIG_BCM963268) && (CONFIG_BCM_EXT_SWITCH)
#define IsExternalSwitchPort(port) (((port < MAX_EXT_SWITCH_PORTS) && (BpGetPortConnectedToExtSwitch()>=0))?1:0)
#else
#define IsExternalSwitchPort(port) ((port < MAX_EXT_SWITCH_PORTS)?1:0)
#endif
#define LOGICAL_PORT_TO_PHYSICAL_PORT(port) ( (port < MAX_EXT_SWITCH_PORTS) ? port : (port-MAX_EXT_SWITCH_PORTS))

typedef struct extsw_info_s {
    unsigned int switch_id;
    int brcm_tag_type;
    int accessType;
    int page;
    int bus_num;
    int spi_ss;
    int spi_cid;
    int present;
    int connected_to_internalPort;
} extsw_info_t;

typedef struct emac_pm_addr_t {
    BOOL                valid;          /* 1 indicates the corresponding address is valid */
    unsigned int        ref;            /* reference count */
    unsigned char       dAddr[ETH_ALEN];/* perfect match register's destination address */
    char                unused[2];      /* pad */
} emac_pm_addr_t;                    
#define MAX_PMADDR          4           /* # of perfect match address */
/*
 * device context
 */ 

typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint32 brcm_tag;
    uint16 encap_proto;
} __attribute__((packed)) BcmEnet_hdr;
typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint16 brcm_tag;
    uint16 encap_proto;
} __attribute__((packed)) BcmEnet_hdr2;

#if defined(VLAN_TAG_FFF_STRIP)
#define VLAN_TYPE           0x8100
typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint32 brcm_tag;
    uint16 vlan_proto;
    uint16 vlan_TCI;
    uint16 encap_proto;
} __attribute__((packed)) BcmVlan_ethhdr;
typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint16 brcm_tag;
    uint16 vlan_proto;
    uint16 vlan_TCI;
    uint16 encap_proto;
} __attribute__((packed)) BcmVlan_ethhdr2;
#endif

#define BRCM_TYPE2               0x888A
#define BRCM_TAG_TYPE2_LEN       4
#define BRCM_TAG2_EGRESS         0x2000
#define BRCM_TAG2_EGRESS_TC_MASK 0x1c00

#define MAX_NUM_OF_VPORTS   8
#define MAX_SWITCH_PORTS    8
#define MAX_EXT_SWITCH_PORTS 8
#define BRCM_TAG_LEN        6
#define BRCM_TYPE           0x8874
#define BRCM_TAG_UNICAST    0x00000000
#define BRCM_TAG_MULTICAST  0x20000000
#define BRCM_TAG_EGRESS     0x40000000
#define BRCM_TAG_INGRESS    0x60000000

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
#define MAX_MARK_VALUES   32
#define MAX_GEM_IDS       32
#define MAX_GPON_IFS      40
/* The bits[0:6] of status field in DmaDesc are Rx Gem ID. For now, we are
   using only 5 bits */
#define RX_GEM_ID_MASK    0x1F
#endif

#if defined(CONFIG_BCM96816)
#define MAX_6829_IFS      2
#define BCM6829LINKMASK   0xFF00

#endif

#ifndef FAP_4KE

/* Keep in sync with bcmPktDma_structs.h */
#define NUM_RXDMA_CHANNELS ENET_RX_CHANNELS_MAX
#define NUM_TXDMA_CHANNELS ENET_TX_CHANNELS_MAX

#define BcmPktDma_EthRxDma BcmPktDma_LocalEthRxDma
#define BcmPktDma_EthTxDma BcmPktDma_LocalEthTxDma

typedef struct BcmEnet_RxDma {

    BcmPktDma_EthRxDma pktDmaRxInfo;
    int      rxIrq;   /* rx dma irq */
    struct sk_buff *freeSkbList;
    uint32   channel;

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    volatile DmaDesc *rxBdsStdBy;
    unsigned char * StdByBuf;
#endif
    unsigned char   **buf_pool; //[NR_RX_BDS_MAX]; /* rx buffer pool */
    unsigned char *skbs_p;
} BcmEnet_RxDma;

#if defined(AEI_VDSL_STATS_DIAG)
struct enet_dev_stats {
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

typedef struct BcmEnet_devctrl {
    struct net_device *dev;             /* ptr to net_device */
    struct net_device *next_dev;        /* next device */
    struct net_device_stats stats;      /* statistics used by the kernel */
    volatile DmaRegs *dmaCtrl;          /* EMAC DMA register base address */
    struct tasklet_struct task;         /* tasklet */
    int             linkState;          /* link status */
    int             wanPort;            /* wan port selection */          
#if defined(CONFIG_BCM96816)
    int             softSwitchingMap;   /* software switching port map */
    int             softSwitchingMap6829; /* software switching port map for 6829 */
    int             mocaLinkState;      /* holds link state for moca phy */
    int             wanPort6829;        /* wan port selection on 6829 */
#endif
    int             unit;               /* device control index */
    unsigned int    vid;
    uint16          chipId;             /* chip's id */
    uint16          chipRev;            /* step */

    spinlock_t ethlock_tx;
    spinlock_t ethlock_moca_tx;
    spinlock_t ethlock_rx;
#ifdef CONFIG_SMP
    uint16 bulk_rx_lock_active[2];    /* optimization: hold rx lock for
                                         multiple pkts. */
#endif

    emac_pm_addr_t  pmAddr[MAX_PMADDR]; /* perfect match address */
    extsw_info_t  *extSwitch;          /* external switch */
    ETHERNET_MAC_INFO EnetInfo[2];
    IOCTL_MIB_INFO MibInfo;

#ifdef CONFIG_BCM96816
    /* For gpon virtual interfaces */
    int gem_count;                      /* Number of gem ids */
    int gponifid;   /* Unique ifindex in [0:31] for gpon virtual interface  */
#endif

    int sw_port_id; /* Physical port index of the Ethernet switch  */
    int vport_id;   /* Unique id of virtual eth interface */

    BcmPktDma_EthTxDma *txdma[NUM_TXDMA_CHANNELS];
    BcmEnet_RxDma *rxdma[NUM_RXDMA_CHANNELS];

    /* DmaKeys, DmaSources, DmaAddresses now allocated with txBds - Apr 2010 */

    int default_txq;
    int use_default_txq;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    struct napi_struct napi;
#endif

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* iuDMA channels can be owned by Host or FAP with TX_SPLITTING - Aug 2010 */
    int enetTxChannel;   /* default iuDMA channel to use for enet tx - Aug 2010 */
#endif

#if defined(GPHY_EEE_1000BASE_T_DEF)
    int eee_enable_request_flag;
#endif

#if defined(AEI_VDSL_TOOLBOX)
    UINT16 usMirrorInFlags;
    UINT16 usMirrorOutFlags;
#endif
#if defined(AEI_VDSL_STATS_DIAG)
    struct enet_dev_stats dev_stats;
#endif
} BcmEnet_devctrl;

#ifndef CARDNAME
#define CARDNAME    "BCM63xx_ENET"
#endif

int bcmenet_add_proc_files(struct net_device *dev);
int bcmenet_del_proc_files(struct net_device *dev);

typedef struct enet_xmit_params {
    unsigned int len;
    unsigned int mark;
    unsigned int priority; 
    unsigned int r_flags;
    int channel; 
    int egress_queue;
    uint16 port_id;
    uint8 * data;
    BcmEnet_devctrl *pDevPriv;
    struct net_device_stats *vstats;
} EnetXmitParams;

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define MOCA_TOTAL_QUEUED_PACKETS_MAX 4500
#else
#define MOCA_TOTAL_QUEUED_PACKETS_MAX 3000
#endif
#define NUM_MOCA_SW_QUEUES          4

#define MOCA_QUEUE_HAS_A_PACKET(moca, queue) \
   (((moca)->tail[queue] == (moca)->head[queue])?0:1)

#define MOCA_QUEUE_HEAD_INCREEMENT(moca, queue) \
    { \
        (moca)->head[queue]++; \
        (moca)->head[queue] %= MOCA_TXQ_DEPTH_MAX; \
    }

#define MOCA_QUEUE_TAIL_INCREEMENT(moca, queue) \
    { \
        (moca)->tail[queue]++; \
        (moca)->tail[queue] %= MOCA_TXQ_DEPTH_MAX; \
    }

#define MOCA_QUEUE_NUM_PACKETS(moca, queue) \
   (((moca)->tail[queue] - (moca)->head[queue] + MOCA_TXQ_DEPTH_MAX) \
     % MOCA_TXQ_DEPTH_MAX)

#endif /* CONFIG_BCM_MOCA_SOFT_SWITCHING */
#endif /* CONFIG_BCM96816 */

#define ENET_TX_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_tx)
#define ENET_TX_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_tx)
#define ENET_RX_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_rx)
#define ENET_RX_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_rx)
#define ENET_MOCA_TX_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_moca_tx)
#define ENET_MOCA_TX_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_moca_tx)
#ifdef AEI_ABBA_FIX
//we know the printk below could cause problem, but for make sure we experienced this problem,
//still leave there;
//NOTE: the production code should remove following printk.
#define MAX_LOCKUP_DETECT 10000
#define ENET_RX_LOCK_WITH_UNLOCK_TX() \
{ \
	unsigned int loop_counter=0; \
	while(!spin_trylock_bh(&global.pVnetDev0_g->ethlock_rx)){ \
		if(!spin_trylock_bh(&global.pVnetDev0_g->ethlock_tx)){ \
			if(loop_counter++>MAX_LOCKUP_DETECT){ \
				spin_unlock_bh(&global.pVnetDev0_g->ethlock_tx); \
				spin_lock_bh(&global.pVnetDev0_g->ethlock_tx); \
			} \
		} else {\
			spin_unlock_bh(&global.pVnetDev0_g->ethlock_tx); \
		} \
	} \
}
#endif

#endif /* !FAP_4KE */
unsigned int bcm63xx_enet_extSwId(void);
int bcm63xx_enet_isExtSwPresent(void);
#endif /* _BCMENET_H_ */

