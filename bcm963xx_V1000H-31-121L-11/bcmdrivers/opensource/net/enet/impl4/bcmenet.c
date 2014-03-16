
/*
<:copyright-BRCM:2010:DUAL/GPL:standard

   Copyright (c) 2010 Broadcom Corporation
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


//**************************************************************************
// File Name  : bcmenet.c
//
// Description: This is Linux network driver for Broadcom Ethernet controller
//
//**************************************************************************

#define VERSION     "0.1"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__


#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/skbuff.h>
#include <linux/kthread.h>
#include "kmap_skb.h"
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kmod.h>
#include <linux/rtnetlink.h>
#include <net/arp.h>
#include <board.h>
#include <spidevices.h>
#include <bcmnetlink.h>
#include <bcm_map.h>
#include <bcm_intr.h>
#include "linux/bcm_assert_locks.h"
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethsw.h"
#include "ethsw_phy.h"
#include "bcmsw.h"
#include <linux/stddef.h>
#include <asm/atomic.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include "pktCmf_public.h"
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#include "fap_hw.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap4ke_memory.h"
#include "fap4ke_local.h"
#endif
#include "bcmPktDma.h"
#include <linux/version.h>
#include "bcmsw_api.h"
#include "bcmswaccess.h"
#include "bcmSpiRes.h"
#include "bcmswshared.h"
#if defined (MOCA_HIGH_RES_TX)
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#endif

#if defined(AEI_VDSL_SW_WAN_ETH_LED)
int enetWanLedOn=0;
void AEI_poll_gphy_LED(void);
#define GPHY_MIBS_TX_REG     0xb0702300
#define GPHY_MIBS_RX_REG     0xb0702344
#define LED_CTRL_MODE1_REG   0xb0001908
#define LED_CTRL_INIT_REG    0xb0001900

#define UPDATE_GPHY_OCTET_COUNT(x) \
do { \
volatile uint32 *ghyMibsTxReg =  (volatile uint32 *) GPHY_MIBS_TX_REG; \
volatile uint32 *ghyMibsRxReg =  (volatile uint32 *) GPHY_MIBS_RX_REG; \
x = *ghyMibsTxReg + *ghyMibsRxReg; \
} while(0)

#define GPHY_LED_ON() \
do { \
volatile uint32 *ledCtrlMode1Reg = (volatile uint32 *) LED_CTRL_MODE1_REG; \
*ledCtrlMode1Reg |= 0x30000000; \
} while(0)

#define GPHY_LED_FAST_BLINK() \
do { \
volatile uint32 *ledCtrlInitReg = (volatile uint32 *) LED_CTRL_INIT_REG; \
volatile uint32 *ledCtrlMode1Reg = (volatile uint32 *) LED_CTRL_MODE1_REG; \
*ledCtrlInitReg &= 0xFFFFF03F; \
*ledCtrlInitReg |= 0x1C0; \
*ledCtrlMode1Reg &= 0xCFFFFFFF; \
*ledCtrlMode1Reg |= 0x20000000; \
} while (0)

#define GPHY_LED_SLOW_BLINK() \
do { \
volatile uint32 *ledCtrlInitReg = (volatile uint32 *) LED_CTRL_INIT_REG; \
volatile uint32 *ledCtrlMode1Reg = (volatile uint32 *) LED_CTRL_MODE1_REG; \
*ledCtrlInitReg &= 0xFFFFF03F; \
*ledCtrlInitReg |= 0x600; \
*ledCtrlMode1Reg &= 0xCFFFFFFF; \
*ledCtrlMode1Reg |= 0x20000000; \
} while (0)
#endif

 
#if defined(CONFIG_BCM96816) && defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
#ifdef BCM_FULL_SRC
#include "mocablock.h"
#endif
#include "bmoca.h"
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#include <net/net_namespace.h>
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include "ingqos.h"
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include <linux/gbpm.h>
#include "bpm.h"
#endif
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
#include <linux/blog_rule.h>
#endif


#define ENET_POLL_DONE        0x80000000
#define ENET_SKB_TAILROOM     16

#if defined(CONFIG_BCM_ENDPOINT) || defined(CONFIG_BCM_ENDPOINT_MODULE) || (defined(CONFIG_BCM96816) && defined(CONFIG_BCM_MOCA_SOFT_SWITCHING))
#define NETDEV_WEIGHT  16 // lower weight for less voice latency
#else
#if defined(AEI_VDSL_CUSTOMER_NCS)
#define NETDEV_WEIGHT  32
#else
#define NETDEV_WEIGHT  32
#endif
#endif

extern BcmPktDma_Bds *bcmPktDma_Bds_p;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
extern uint32_t iqos_enable_g;
extern uint32_t iqos_debug_g;
extern uint32_t iqos_cpu_cong_g;

/* IQ status dump handler hook */
extern iqos_status_hook_t iqos_enet_status_hook_g;

static thresh_t enet_rx_dma_iq_thresh[ENET_RX_CHANNELS_MAX];

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static thresh_t enet_rx_dqm_iq_thresh[ENET_RX_CHANNELS_MAX];

/* FAP get Eth DQM queue length handler hook */
extern iqos_fap_ethRxDqmQueue_hook_t iqos_fap_ethRxDqmQueue_hook_g;

static void enet_set_iq_thresh_dqm(BcmEnet_devctrl *pDevCtrl, int channel,
                                uint16 loThresh, uint16 hiThresh);
static void enet_iq_dqm_update_cong_status(BcmEnet_devctrl *pDevCtrl);
static void enet_iq_dqm_status(void);
#endif

static void enet_rx_set_iq_thresh( BcmEnet_devctrl *pDevCtrl, int chnl );
static void enet_rx_init_iq_thresh( BcmEnet_devctrl *pDevCtrl, int chnl );
static void enet_iq_update_cong_status(BcmEnet_devctrl *pDevCtrl);
static void enet_iq_dma_status(void);
static void enet_iq_status(void);
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))

#if (ENET_TX_EGRESS_QUEUES_MAX != NUM_EGRESS_QUEUES)
#error "ERROR - (ENET_TX_EGRESS_QUEUES_MAX != NUM_EGRESS_QUEUES)"
#endif

extern uint32_t gbpm_enable_g;
static inline int enet_bpm_alloc_buf(BcmEnet_devctrl *pDevCtrl, int channel);
static inline int enet_bpm_free_buf(BcmEnet_devctrl *pDevCtrl, int channel,
                uint8 *pData);
static int enet_bpm_alloc_buf_ring(BcmEnet_devctrl *pDevCtrl,
        int channel, uint32 num);
static void enet_bpm_free_buf_ring(BcmEnet_RxDma *rxdma, int channel);

static void enet_rx_set_bpm_alloc_trig( BcmEnet_devctrl *pDevCtrl, int chnl );

extern gbpm_status_hook_t gbpm_enet_status_hook_g;
static void enet_bpm_status(void);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
/* BPM status dump handler hook */
extern gbpm_thresh_hook_t gbpm_enet_thresh_hook_g;

static uint16_t
        enet_bpm_dma_tx_drop_thr[ENET_TX_CHANNELS_MAX][ENET_TX_EGRESS_QUEUES_MAX];


static void enet_bpm_init_tx_drop_thr(BcmEnet_devctrl *pDevCtrl, int chnl);
static void enet_bpm_set_tx_drop_thr( BcmEnet_devctrl *pDevCtrl, int chnl );
static void enet_bpm_dma_dump_tx_drop_thr(void);
static void enet_bpm_dump_tx_drop_thr(void);
/* Sanity checks */
#if (BPM_ENET_BULK_ALLOC_COUNT > FAP_BPM_ENET_BULK_ALLOC_MAX)
#error "ERROR - BPM_ENET_BULK_ALLOC_COUNT > FAP_BPM_ENET_BULK_ALLOC_MAX"
#endif
#endif

#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
/* BPM threshold dump handler hook */
extern gbpm_thresh_hook_t gbpm_moca_thresh_hook_g;

static int moca_lan_bpm_txq_thresh(int qid);
static int moca_wan_bpm_txq_thresh(int qid);

static void moca_lan_dump_txq_thresh( void );
static void moca_wan_dump_txq_thresh( void );
static void moca_bpm_dump_txq_thresh(void);
#endif
#endif

//extern int sched_setscheduler_export(struct task_struct *, int, struct sched_param *);

#define port_id_from_dev(dev) ((dev->base_addr == vnet_dev[0]->base_addr) ? 0 : \
                              ((BcmEnet_devctrl *)netdev_priv(dev))->sw_port_id)
#define port_from_flag(flag) ((flag >> 8) & 0x000f)
extern int kerSysGetMacAddress(unsigned char *pucaMacAddr, unsigned long ulId);
static int bcm63xx_enet_open(struct net_device * dev);
static int bcm63xx_enet_close(struct net_device * dev);
static void bcm63xx_enet_timeout(struct net_device * dev);
static void bcm63xx_enet_poll_timer(unsigned long arg);
static int bcm63xx_enet_xmit(pNBuff_t pNBuff, struct net_device * dev);
static inline int bcm63xx_enet_xmit2(pNBuff_t pNBuff, struct net_device *dev, EnetXmitParams *pParam);
static int bcm63xx_xmit_reclaim(void);
static struct net_device_stats * bcm63xx_enet_query(struct net_device * dev);
static int bcm63xx_enet_change_mtu(struct net_device *dev, int new_mtu);
static FN_HANDLER_RT bcm63xx_ephy_isr(int irq, void *);
#if defined(CONFIG_BCM963268)
static FN_HANDLER_RT bcm63xx_gphy_isr(int irq, void *);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static int bcm63xx_enet_poll_napi(struct napi_struct *napi, int budget);
#else
static int bcm63xx_enet_poll(struct net_device * dev, int *budget);
#endif
static uint32 bcm63xx_rx(void *ptr, uint32 budget);
static void bcm63xx_enet_recycle_skb_or_data(struct sk_buff *skb,
                                             uint32 context, uint32 free_flag);
static int bcm_set_mac_addr(struct net_device *dev, void *p);
static void flush_assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel,
                                   uint8 * pData, uint8 * pEnd);
static void init_dma(BcmEnet_devctrl *pDevCtrl);
static int init_buffers(BcmEnet_devctrl *pDevCtrl, int channel);
static void setup_txdma_channel(int channel);
static int bcm63xx_init_dev(BcmEnet_devctrl *pDevCtrl);
static int bcm63xx_uninit_dev(BcmEnet_devctrl *pDevCtrl);
static void __exit bcmenet_module_cleanup(void);
static int __init bcmenet_module_init(void);
static int bcm63xx_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
int __init bcm63xx_enet_probe(void);
static int set_cur_txdma_channels(int num_channels);
static int set_cur_rxdma_channels(int num_channels);
void uninit_buffers(BcmEnet_RxDma *rxdma);
static int init_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
static int init_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
void uninit_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
void uninit_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
#if defined(CONFIG_BCM963268)
static int bcm_strip_tag_type1(struct sk_buff *skb, bool strip_brcm_tag);
static int bcm_strip_tag_type2(struct sk_buff *skb, bool strip_brcm_tag);
#else
static int bcm_strip_tag_type1(struct sk_buff *skb);
static int bcm_strip_tag_type2(struct sk_buff *skb);
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS) && !defined(AEI_63168_CHIP)
UBOOL8 extSwitchExist = FALSE;
#endif
/* Sanity checks for user configured DMA parameters */
#if (CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS > ENET_RX_CHANNELS_MAX)
#error "ERROR - Defined RX DMA Channels greater than MAX"
#endif
#if (CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS > ENET_TX_CHANNELS_MAX)
#error "ERROR - Defined TX DMA Channels greater than MAX"
#endif
#if 0
#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
#if (CONFIG_BCM_NR_RX_BDS*CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS > 400)
#error "ERROR - Not enough memory for configured RX BDs"
#endif
#if (CONFIG_BCM_NR_TX_BDS*CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS > 180)
#error "ERROR - Not enough memory for configured TX BDs"
#endif
#endif
#endif

/* 32bit context is union of pointer to pdevCtrl and channel number */
#if (ENET_RX_CHANNELS_MAX > 4)
#error "Overlaying channel and pDevCtrl into context param needs rework"
#else
/*
 * IMPORTANT: The following 3 macros are only used for ISR context. The
 * recycling context is defined by enet_recycle_context_t
 */
#define BUILD_CONTEXT(pDevCtrl,channel) \
            (uint32)((uint32)(pDevCtrl) | ((uint32)(channel) & 0x3u))
#define CONTEXT_TO_PDEVCTRL(context)    (BcmEnet_devctrl*)((context) & ~0x3u)
#define CONTEXT_TO_CHANNEL(context)     (int)((context) & 0x3u)
#endif

/*
 * Recycling context definition
 */
typedef union {
    struct {
        /* fapQuickFree handling removed - Oct 2010 */
        uint32 reserved     : 30;
        uint32 channel      :  2;
    };
    uint32 u32;
} enet_recycle_context_t;

#define RECYCLE_CONTEXT(_context)  ( (enet_recycle_context_t *)(&(_context)) )
#define FKB_RECYCLE_CONTEXT(_pFkb) RECYCLE_CONTEXT((_pFkb)->recycle_context)

/* Following APIs use the 32bit context to pass pDevCtrl and channel */
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) || defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
static FN_HANDLER_RT bcm63xx_enet_isr(int irq, void * pContext);
#endif
static void bcm63xx_enet_recycle(pNBuff_t pNBuff, uint32 context, uint32 flags);

static DECLARE_COMPLETION(poll_done);
static atomic_t poll_lock = ATOMIC_INIT(1);
static int poll_pid = -1;
#if !defined(SUPPORT_SWMDK)
static int ephy_int_cnt = 1;
#endif
struct net_device* vnet_dev[MAX_NUM_OF_VPORTS+1] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int vport_to_phyport[MAX_NUM_OF_VPORTS] = {0, 1, 2, 3, 4, 5, 6, 7};
static int phyport_to_vport[MAX_SWITCH_PORTS*2] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int vport_cnt;  /* number of vports: bitcount of Enetinfo.sw.port_map */

#if defined(CONFIG_BCM963268)
static unsigned int consolidated_portmap;
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
static void switch_rx_ring(BcmEnet_devctrl *pDevCtrl, int channel, int toStdBy);
static DECLARE_COMPLETION(timer_done);
static atomic_t timer_lock = ATOMIC_INIT(1);
static int timer_pid = -1;
#define DMA_THROUGHPUT_TEST_EN  0x80000
atomic_t v = ATOMIC_INIT(0);
/* default pkt rate is 100 pkts/100ms */
static int rxchannel_rate_credit[ENET_RX_CHANNELS_MAX] = {100};
static int rxchannel_rate_limit_enable[ENET_RX_CHANNELS_MAX] = {0};
static int rxchannel_isr_enable[ENET_RX_CHANNELS_MAX] = {1};
static int rx_pkts_from_last_jiffies[ENET_RX_CHANNELS_MAX] = {0};
static int last_pkt_jiffies[ENET_RX_CHANNELS_MAX] = {0};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#else
static const softirq_prio_t poll_prio =  /* type, priority, nice */
  {SCHED_RR, CONFIG_BRCM_SOFTIRQ_BASE_RT_PRIO, 0};
#endif
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
#define gemid_from_dmaflag(dmaFlag) (dmaFlag & RX_GEM_ID_MASK)
#define UNASSIGED_IFIDX_VALUE (-1)
#define MAX_GPON_IFS_PER_GEM  (5)
static int gem_to_gponifid[MAX_GEM_IDS][MAX_GPON_IFS_PER_GEM];
static int freeid_map[MAX_GPON_IFS] = {0};
static int default_gemid[MAX_GPON_IFS] = {0};
struct net_device* gponifid_to_dev[MAX_GPON_IFS] =
                             {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int create_gpon_vport(char *name);
static int delete_gpon_vport(char *ifname);
static int delete_all_gpon_vports(void);
static int set_get_gem_map(int op, char *ifname, int ifnum, u32 *gem_map);
static int set_mcast_gem_id(u32 gem_map);
#endif

#if defined(CONFIG_BCM96816)
extern void create_6829_vport( int portMap );
extern void check_6829_vports( int portMap, int * newStat );
struct net_device* bcm6829_to_dev[MAX_6829_IFS] = {NULL, NULL};
#define BCM6829_MOCA_DEV 0
atomic_t bcm6829ActDevIdx = ATOMIC_INIT(0);
int phyport_to_vport_6829[MAX_SWITCH_PORTS] = {-1, -1, -1, -1, -1, -1, -1, -1};
MirrorCfg gemMirrorCfg[2]= { {"", "", 0, 0, 0},
                             {"", "", 0, 0, 0}};
static uint8 defaultIPG = 0; /* Read during init */
#endif

#if defined(AEI_VDSL_POWERSAVE)
static PowerSaveCfg pwsCfg = {0, -1};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static const struct net_device_ops bcm96xx_netdev_ops = {
  .ndo_open   = bcm63xx_enet_open,
  .ndo_stop   = bcm63xx_enet_close,
  .ndo_start_xmit   = (HardStartXmitFuncP)bcm63xx_enet_xmit,
  .ndo_set_mac_address  = bcm_set_mac_addr,
  .ndo_do_ioctl   = bcm63xx_enet_ioctl,
  .ndo_tx_timeout   = bcm63xx_enet_timeout,
  .ndo_get_stats      = bcm63xx_enet_query,
  .ndo_change_mtu     = bcm63xx_enet_change_mtu
};
#endif

/* The number of rx and tx dma channels currently used by enet driver */
   int cur_rxdma_channels = CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS;
   int cur_txdma_channels = CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS;

/* When TX iuDMA channel is used for determining the egress queue,
   this array provides the Tx iuDMA channel to egress queue mapping
   information */
int channel_for_queue[NUM_EGRESS_QUEUES] = {0};
int use_tx_dma_channel_for_priority = 0;
/* rx scheduling control and config variables */
static int scheduling = WRR_SCHEDULING;
static int max_pkts = 1280;
static int weights[ENET_RX_CHANNELS_MAX] = {[0 ... (ENET_RX_CHANNELS_MAX-1)] = 1};
static int weight_pkts[ENET_RX_CHANNELS_MAX] = {[0 ... (ENET_RX_CHANNELS_MAX-1)] = 320};
static int pending_weight_pkts[ENET_RX_CHANNELS_MAX] = {[0 ... (ENET_RX_CHANNELS_MAX-1)] = 320};
static int pending_channel[ENET_RX_CHANNELS_MAX] = {0}; /* Initialization is done during module init */
static int channel_ptr = 0;
static int loop_index = 0;
static int global_channel = 0;
static int pending_ch_tbd;
static int channels_mask;
static int pending_channels_mask;

#ifdef BCM_ENET_DEBUG_BUILD
/* Debug Variables */
/* Number of pkts received on each channel */
static int ch_pkts[ENET_RX_CHANNELS_MAX] = {0};
/* Number of times there are no rx pkts on each channel */
static int ch_no_pkts[ENET_RX_CHANNELS_MAX] = {0};
static int ch_no_bds[ENET_RX_CHANNELS_MAX] = {0};
/* Number of elements in ch_serviced debug array */
#define NUM_ELEMS 4000
/* -1 indicates beginning of an rx(). The bit31 indicates whether a pkt
   is received on that channel or not */
static unsigned int ch_serviced[NUM_ELEMS] = {0};
static int dbg_index;
#define NEXT_INDEX(index) ((++index) % NUM_ELEMS)
#define ISR_START 0xFF
#define WRR_RELOAD 0xEE
#endif

extsw_info_t extSwInfo = {
  .switch_id = 0,
  .brcm_tag_type = 0,
  .present = 0,
  .connected_to_internalPort = -1,
};

static int bcmenet_in_init_dev = 0;
static unsigned int bcmenet_rxToss = 0;

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
struct tasklet_struct mocaTasklet;
static inline int moca_send_packets(int isWan, int sendOne);
typedef struct {
    unsigned int tail[NUM_MOCA_SW_QUEUES];
    unsigned int head[NUM_MOCA_SW_QUEUES];
    int egressq_alloc_bufs;
    pNBuff_t queue[NUM_MOCA_SW_QUEUES][MOCA_TXQ_DEPTH_MAX];
    unsigned short usedBuffers[NUM_MOCA_SW_QUEUES];
    unsigned int   queuedPackets;
    unsigned int   mask;
    u64            lastWindowTime;
    unsigned int   packetCount;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    bpm_thresh_t   thresh[NUM_MOCA_SW_QUEUES];
#endif
} moca_queue_t;

#define MOCA_PORT_CHECK_DEBUG 1
typedef struct {
    int          enable;
    unsigned int interval_ns;
    unsigned int threshold;
    unsigned int match_count[NUM_MOCA_SW_QUEUES];
    unsigned short last_count[NUM_MOCA_SW_QUEUES];
    u64          last_check_time;
    unsigned long last_tx_count[NUM_MOCA_SW_QUEUES];
#ifdef MOCA_PORT_CHECK_DEBUG
    unsigned int run_count;
    unsigned int reset_count;
    unsigned int total_match_count;
    int          test;
#endif
} moca_port_check_t;
#endif /* CONFIG_BCM_MOCA_SOFT_SWITCHING */
#endif /* CONFIG_BCM96816 */

typedef struct {
    unsigned int extPhyMask;
    int dump_enable;
#if defined(CONFIG_BCM963268)
    int (*bcm_strip_tag) (struct sk_buff *skb, bool strip_brcm_tag);
#else
    int (*bcm_strip_tag) (struct sk_buff *skb);
#endif
#if defined(CONFIG_BCM96816)
    int Is6829;
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    int enet_softswitch_xmit_start_q;
    int moca_xmit_budget;
    int moca_queue_depth;
    moca_queue_t moca_lan;
    moca_queue_t moca_wan;
    moca_port_check_t moca_port_check[2]; /* LAN = 0; WAN = 1 */
#endif
#endif /* CONFIG_BCM96816 */
    struct net_device_stats net_device_stats_from_hw;
    BcmEnet_devctrl *pVnetDev0_g;
}enet_global_var_t;

static enet_global_var_t global = {
  .extPhyMask = 0,
  .dump_enable = 0,
  .bcm_strip_tag = bcm_strip_tag_type1,
#if defined(CONFIG_BCM96816)
  .Is6829 = 0,
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
  .enet_softswitch_xmit_start_q = 2,
#if defined (MOCA_HIGH_RES_TX)
  .moca_xmit_budget = 12,
#else
  .moca_xmit_budget = 20,
#endif
  .moca_queue_depth = MOCA_TXQ_DEPTH_MAX,
  .moca_port_check[0] =
   {
      .enable = 1,
      .interval_ns = 250000000,
      .threshold = 3,
      .match_count = {0, 0, 0, 0},
      .last_count = {0, 0, 0, 0},
      .last_tx_count = {0, 0, 0, 0},
#ifdef MOCA_PORT_CHECK_DEBUG
      .run_count = 0,
      .reset_count = 0,
      .total_match_count = 0
#endif
   },
  .moca_port_check[1] =
   {
      .enable = 1,
      .interval_ns = 1000000000,
      .threshold = 3,
      .match_count = {0, 0, 0, 0},
      .last_count = {0, 0, 0, 0},
      .last_tx_count = {0, 0, 0, 0},
#ifdef MOCA_PORT_CHECK_DEBUG
      .run_count = 0,
      .reset_count = 0,
      .total_match_count = 0
#endif
   },
#endif
#endif
  .net_device_stats_from_hw = {0},
  .pVnetDev0_g = NULL
};

DECLARE_MUTEX(bcm_ethlock_switch_config);

spinlock_t bcm_ethlock_phy_access;
spinlock_t bcm_extsw_access;
atomic_t phy_read_ref_cnt = ATOMIC_INIT(0);
atomic_t phy_write_ref_cnt = ATOMIC_INIT(0);

#define DELAYED_RECLAIM_ARRAY_LEN 8

/*
 * This macro can only be used inside enet_xmit2 because it uses the local
 * variables defined in that function.
 */
#define DO_DELAYED_RECLAIM() \
    do { \
        uint32 tmp_idx=0; \
        while (tmp_idx < reclaim_idx) { \
            nbuff_free((pNBuff_t) delayed_reclaim_array[tmp_idx]); \
            tmp_idx++; } \
    } while (0)

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
/* Add code for buffer quick free between enet and xtm - June 2010 */
static RecycleFuncP xtm_fkb_recycle_hook = NULL;
#if defined(CONFIG_BCM963268)
static RecycleFuncP xtm_skb_recycle_hook = NULL;
#endif
#endif

#if defined(CONFIG_BCM96816)
static unsigned short bcm_type_trans(struct sk_buff *skb, struct net_device *dev);
/***************************************************************************
 * Function Name: MirrorPacket
 * Description  : This function sends a sent or received packet to a LAN port.
 *                The purpose is to allow packets sent and received on the WAN
 *                to be captured by a protocol analyzer on the Lan for debugging
 *                purposes.
 * Returns      : None.
 ***************************************************************************/
static void MirrorPacket(struct sk_buff *skb, char *intfName, int stripTag)
{
    struct sk_buff *skb2;
    struct net_device *netDev;

    skb2 = skb_copy( skb, GFP_ATOMIC );
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

            if (stripTag)
            {
                skb_trim(skb2, skb2->len - ETH_CRC_LEN);
                skb2->dev = netDev;
                skb2->protocol = bcm_type_trans(skb2, netDev);
                skb_push(skb2, sizeof(struct ethhdr));
            }
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

#if defined(AEI_VDSL_HPNA)
enum {
HPNA_LAN,
HPNA_WAN_LAN
};

static int hpna_admin_state = 1;
static int hpna_mode = HPNA_LAN;
static int hpna_dev_index = 0;
static int hpna_port = 0;
int hpna_support = 0; // 0 : un-support, 1 : support
#endif

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

static void AEI_MirrorPacket( struct sk_buff *skb, char *intfName, UBOOL8 mirrorIn)
{
    struct sk_buff *skbCopy;
    struct net_device *netDev;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    if( (netDev = __dev_get_by_name(&init_net, intfName)) != NULL )
#else
    if( (netDev = __dev_get_by_name(intfName)) != NULL )
#endif
    {
        if( (skbCopy = skb_copy(skb, GFP_ATOMIC)) != NULL )
        {
            unsigned long flags;

            if (mirrorIn)
            {
                unsigned char tmp_addr[ETH_ALEN];
                struct ethhdr * eth_hdr;

                skb_push(skbCopy, sizeof(struct ethhdr));

                /* when transmiting packets using GW mac address as its ethernet dest mac address, 
                 * some version of the BCM53115 switch will drop these packets, so just swap
                 * the ethernet source and dest mac address for these packets.
                 */
                eth_hdr = (struct ethhdr *)skbCopy->data;
                if (eth_hdr)
                {
                   memcpy(tmp_addr, eth_hdr->h_source, ETH_ALEN);
                   memcpy(eth_hdr->h_source, eth_hdr->h_dest, ETH_ALEN);
                   memcpy(eth_hdr->h_dest, tmp_addr, ETH_ALEN);
                }
            }

            skbCopy->dev = netDev;
            skbCopy->blog_p = BLOG_NULL;
            if (strstr(intfName, "wl"))
                skbCopy->protocol = htons(ETH_P_MIRROR_WLAN);
            else
                skbCopy->protocol = htons(ETH_P_MIRROR);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(skbCopy) ;
            local_irq_restore(flags);
        }
    }
} /* MirrorPacket */

static void AEI_MultiMirrorPacket( struct sk_buff *skb, UINT16 mirFlags, UBOOL8 mirrorIn)
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

        AEI_MirrorPacket( skb, pMirrorIntfNames[i], mirrorIn );
    }
}
#endif /* AEI_VDSL_TOOLBOX */

/* _assign_rx_buffer: Reassigns a free data buffer to RxBD. No flushing !!! */
static inline void _assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel, uint8 * pData)
{
    BcmPktDma_EthRxDma *pktDmaRxInfo_p =
                                &pDevCtrl->rxdma[channel]->pktDmaRxInfo;

#ifdef CONFIG_SMP
    unsigned int cpuid;
    unsigned int is_bulk_rx_lock_active;

    /*
     * Disable preemption so that my cpuid will not change in this func.
     * Not possible for the state of bulk_rx_lock_active to change
     * underneath this function on the same cpu.
     */
    preempt_disable();
    cpuid = smp_processor_id();
    is_bulk_rx_lock_active = pDevCtrl->bulk_rx_lock_active[cpuid];

    if (0 == is_bulk_rx_lock_active)
        ENET_RX_LOCK();
#else
    ENET_RX_LOCK();
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    {
        if (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds)
        {
            bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
        }
        else
        {
            enet_bpm_free_buf(pDevCtrl, channel, pData);
        }
    }
#else
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
     if (pktDmaRxInfo_p->rxOwnership == HOST_OWNED)
     {
        if (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds)
        {
            bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
        }
        else
        {
            enet_bpm_free_buf(pDevCtrl, channel, pData);
        }
     }
     else
        bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
#else
    bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
#endif
#endif /* !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
#else
    bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
#endif /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    /* Update congestion status, once all the buffers have been recycled. */
    if (iqos_cpu_cong_g)
    {
        if (pktDmaRxInfo_p->numRxBds == pktDmaRxInfo_p->rxAssignedBds)
            iqos_set_cong_status(IQOS_IF_ENET, channel, IQOS_CONG_STATUS_LO);
    }
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Delay is only needed when the free is actually being done by the FAP */
    if(bcmenet_in_init_dev)
       udelay(20);
#endif

#ifdef CONFIG_SMP
    if (0 == is_bulk_rx_lock_active)
        ENET_RX_UNLOCK();
    preempt_enable();
#else
    ENET_RX_UNLOCK();
#endif
}

/*
 * flush_assign_rx_buffer: Cache invalidates before assigning buffer to RxBd
 * Subtle point: flush means write back and invalidate.  Doing invalidate
 * is not good enough because the dirty bit in the cache line tag can still
 * be set, and MIPS will still want to write back that line even though the
 * valid bit has been cleared.
 *   pData: Points to rx DMAed buffer
 *   pEnd : demarcates the end of the buffer that may have cache lines that
 *          need to be invalidated.
 *  if ( round_down_cacheline(pData) == round_up_cacheline(pEnd) ) no flush.
 */
static void flush_assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel,
                                   uint8 * pData, uint8 * pEnd)
{
    cache_flush_region(pData, pEnd);
    _assign_rx_buffer( pDevCtrl, channel, pData );
}

/* Callback: fkb and data recycling */
static inline void __bcm63xx_enet_recycle_fkb(struct fkbuff * pFkb,
                                              uint32 context)
{
    int channel = FKB_RECYCLE_CONTEXT(pFkb)->channel;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    uint8 *pData = PFKBUFF_TO_PDATA(pFkb,RX_ENET_SKB_HEADROOM);

    _assign_rx_buffer(pDevCtrl, channel, pData); /* No cache flush */
}


/* kernel skb free call back */
static void bcm63xx_enet_recycle_skb_or_data(struct sk_buff *skb,
                                             uint32 context, uint32 free_flag)
{
    int channel  = RECYCLE_CONTEXT(context)->channel;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    if( free_flag & SKB_RECYCLE ) {
        BcmEnet_RxDma * rxdma;
#ifdef CONFIG_SMP
    unsigned int cpuid;
    unsigned int is_bulk_rx_lock_active;

    /*
     * Disable preemption so that my cpuid will not change in this func.
     * Not possible for the state of bulk_rx_lock_active to change
     * underneath this function on the same cpu.
     */
    preempt_disable();
    cpuid = smp_processor_id();
    is_bulk_rx_lock_active = pDevCtrl->bulk_rx_lock_active[cpuid];

#ifdef AEI_ABBA_FIX
    if (0 == is_bulk_rx_lock_active){
	    ENET_RX_LOCK_WITH_UNLOCK_TX();
    }
#else
    if (0 == is_bulk_rx_lock_active)
        ENET_RX_LOCK();
#endif
#else
    ENET_RX_LOCK();
#endif

        rxdma = pDevCtrl->rxdma[channel];
        skb->next_free = rxdma->freeSkbList;
        rxdma->freeSkbList = skb;

#ifdef CONFIG_SMP
    if (0 == is_bulk_rx_lock_active)
        ENET_RX_UNLOCK();
    preempt_enable();
#else
    ENET_RX_UNLOCK();
#endif
    }
    else { // free data
            uint8 *pData = skb->head + RX_ENET_SKB_HEADROOM;
            uint8 *pEnd;
#if defined(ENET_CACHE_SMARTFLUSH)
            uint8 *dirty_p = skb_shinfo(skb)->dirty_p;
            uint8 *shinfoBegin = (uint8 *)skb_shinfo(skb);
            uint8 *shinfoEnd;
            if (skb_shinfo(skb)->nr_frags == 0) {
                // no frags was used on this skb, so can shorten amount of data
                // flushed on the skb_shared_info structure
                shinfoEnd = shinfoBegin + offsetof(struct skb_shared_info, frags);
            }
            else {
                shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
            }
            cache_flush_region(shinfoBegin, shinfoEnd);

            // If driver returned this buffer to us with a valid dirty_p,
            // then we can shorten the flush length.
            if (IS_SKBSHINFO_DIRTYP_ACK(dirty_p)) {
                CLR_SKBSHINFO_DIRTYP_ACK(dirty_p);
                if ((dirty_p < skb->head) || (dirty_p > shinfoBegin)) {
                    printk("invalid dirty_p detected: %p valid=[%p %p]\n",
                           dirty_p, skb->head, shinfoBegin);
                    pEnd = shinfoBegin;
                } else {
                    pEnd = (dirty_p < pData) ? pData : dirty_p;
                }
            } else {
                pEnd = shinfoBegin;
            }
#else
            pEnd = pData + RX_BUF_LEN;
#endif
            flush_assign_rx_buffer(pDevCtrl, channel, pData, pEnd);
    }
}

/* Common recycle callback for fkb, skb or data */
static void bcm63xx_enet_recycle(pNBuff_t pNBuff, uint32 context, uint32 flags)
{
    if ( IS_FKBUFF_PTR(pNBuff) ) {
        __bcm63xx_enet_recycle_fkb(PNBUFF_2_FKBUFF(pNBuff), context);
    } else { /* IS_SKBUFF_PTR(pNBuff) */
        bcm63xx_enet_recycle_skb_or_data(PNBUFF_2_SKBUFF(pNBuff),context,flags);
    }
}

/* Delete all the virtual eth ports */
static void delete_vport(void)
{
    int port;

    synchronize_net();

    for (port = 1; port <= vport_cnt; port++)
    {
        if (vnet_dev[port] != NULL)
        {
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
            if(memcmp(vnet_dev[0]->dev_addr, vnet_dev[port]->dev_addr, ETH_ALEN)) {
                kerSysReleaseMacAddress(vnet_dev[port]->dev_addr);
            }
#endif
            unregister_netdev(vnet_dev[port]);
            free_netdev(vnet_dev[port]);
            vnet_dev[port] = NULL;
        }
    }

#if defined(CONFIG_BCM96816)
    for (port = 0; port < MAX_6829_IFS; port++)
    {
        if (bcm6829_to_dev[port] != NULL)
        {
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
            if(memcmp(vnet_dev[0]->dev_addr, bcm6829_to_dev[port]->dev_addr, ETH_ALEN)) {
                kerSysReleaseMacAddress(bcm6829_to_dev[port]->dev_addr);
            }
#endif
            unregister_netdev(bcm6829_to_dev[port]);
            free_netdev(bcm6829_to_dev[port]);
            bcm6829_to_dev[port] = NULL;
        }
    }
#endif
}

#if defined(CONFIG_BCM96816)
static const char* mocaif_name = "moca%d";
#endif

#if defined(CONFIG_EPON_SDK)
static const char* eponif_name = "epon0";
#endif

#if defined (AEI_VDSL_WAN_ETH)
static const char* ewanif_name = "ewan%d";
#endif

/* Create virtual eth ports: one for each physical switch port except
   for the GPON port */
static int create_vport(void)
{
    struct net_device *dev;
    struct sockaddr sockaddr;
    int status, map, i, j;
    PHY_STAT phys;
    BcmEnet_devctrl *pDevCtrl = NULL;
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    int phy_id;
#if defined(CONFIG_BCM963268)
    int unit, port;
#endif   

#if defined(CONFIG_BCM963268)
    map = consolidated_portmap;
#else
    map = pVnetDev0->EnetInfo[pVnetDev0->unit].sw.port_map;
#endif

    if (vport_cnt > MAX_NUM_OF_VPORTS)
        return -1;

    phys.lnk = 0;
    phys.fdx = 0;
    phys.spd1000 = 0;
    phys.spd100 = 0;

    for (i = 1, j = 0; i < vport_cnt + 1; i++, j++, map /= 2)
    {
        /* Skip the switch ports which are not in the port_map */
        while ((map % 2) == 0)
        {
            map /= 2;
            j ++;
        }

        /* Initialize the vport <--> phyport mapping tables */
        vport_to_phyport[i] = j;
        phyport_to_vport[j] = i;

#if defined(CONFIG_BCM96816)
        /* Skip creating eth interface for GPON port */
        if (j == GPON_PORT_ID) {
            map /= 2;
            j ++;
            continue;
        }
#endif

#if defined(CONFIG_BCM963268)
        if (extSwInfo.present)
        {
            port = LOGICAL_PORT_TO_PHYSICAL_PORT(j);     
            unit = (j < MAX_EXT_SWITCH_PORTS) ? 1 : 0;
            phy_id = pVnetDev0->EnetInfo[unit].sw.phy_id[port];
        }
        else 
        {
             phy_id = pVnetDev0->EnetInfo[pVnetDev0->unit].sw.phy_id[j];
        }
#else
        phy_id = pVnetDev0->EnetInfo[pVnetDev0->unit].sw.phy_id[j];
#endif


#if defined(CONFIG_BCM96816)
        if ( (j == SERDES_PORT_ID) &&
             IsExt6829(phy_id) )
        {
           create_6829_vport(phy_id);
           continue;
        }
#endif

        dev = alloc_etherdev(sizeof(BcmEnet_devctrl));

        /* Set the pDevCtrl->dev to dev */
        pDevCtrl = netdev_priv(dev);
        pDevCtrl->dev = dev;

        if (dev == NULL) {
            delete_vport();
            return -ENOMEM;
        }

        memset(netdev_priv(dev), 0, sizeof(BcmEnet_devctrl));

#if defined(CONFIG_BCM96816)
        /* Name the MoCA interface as mocaX */
        if (j == MOCA_PORT_ID) {
            dev_alloc_name(dev, mocaif_name);
        } else
#endif
#if defined(CONFIG_EPON_SDK)
        if (phy_id & CONNECTED_TO_EPON_MAC) {
            dev_alloc_name(dev, eponif_name);
        } else
#endif
#if defined(AEI_VDSL_WAN_ETH)
#if defined(CONFIG_BCM963268)
        if (phy_id == EWAN_PORT_ID)
#else
        if (j == EWAN_PORT_ID)
#endif  
        {
            dev_alloc_name(dev, ewanif_name);
        } else
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
        if ((j >= LAN_PORT_ID_1) && (j <= LAN_PORT_ID_4))
        {
            char ifname[IFNAMSIZ];

            sprintf(ifname, "eth%d", 3 - j);
            dev_alloc_name(dev, ifname);
        }		
        else
#endif
        {
            dev_alloc_name(dev, dev->name);
        }


#if defined(AEI_VDSL_HPNA)
        //initialize the external switch port to get hpna running
        if ( hpna_support && (phy_id == HPNA_PORT_ID)) 
        {
#if defined(AEI_63168_CHIP)
            ;
#else
            uint8 v8 = REG_PORT_STATE_LNK | REG_PORT_STATE_FDX | REG_PORT_STATE_100 | REG_PORT_STATE_FLOWCTL | REG_PORT_STATE_OVERRIDE;

            extsw_wreg(PAGE_CONTROL, REG_PORT5_STATE, &v8, 1);
#endif
            hpna_port = LOGICAL_PORT_TO_PHYSICAL_PORT(j);
            hpna_dev_index = i;
        }
#endif

        SET_MODULE_OWNER(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        dev->netdev_ops             = vnet_dev[0]->netdev_ops;
#else
        dev->open                   = vnet_dev[0]->open;
        dev->stop                   = vnet_dev[0]->stop;
        dev->hard_start_xmit        = vnet_dev[0]->hard_start_xmit;
        dev->tx_timeout             = vnet_dev[0]->tx_timeout;
        dev->set_mac_address        = vnet_dev[0]->set_mac_address;
        dev->do_ioctl               = vnet_dev[0]->do_ioctl;
        dev->get_stats              = vnet_dev[0]->get_stats;
#endif
        dev->priv_flags             = vnet_dev[0]->priv_flags;
        dev->base_addr              = j;

        dev->features               = vnet_dev[0]->features;

        /* Switch port id of this interface */
        pDevCtrl->sw_port_id        = j;

        netdev_path_set_hw_port(dev, j, BLOG_ENETPHY);

        status = register_netdev(dev);

        if (status != 0)
        {
            unregister_netdev(dev);
            free_netdev(dev);
            return status;
        }

        vnet_dev[i] = dev; 
    
        /* The vport_id specifies the unique id of virtual eth interface */
        pDevCtrl->vport_id = i;

        /* Set the default tx queue to 0 */
        pDevCtrl->default_txq = 0;
        pDevCtrl->use_default_txq = 0;

        memmove(dev->dev_addr, vnet_dev[0]->dev_addr, ETH_ALEN);
        memmove(sockaddr.sa_data, vnet_dev[0]->dev_addr, ETH_ALEN);
        BCM_ENET_DEBUG("phy_id = %d", phy_id);
        if (IsWanPort(phy_id)) {
            pVnetDev0->wanPort |= 1 << j;
            BCM_ENET_DEBUG("Getting MAC for WAN port %d", j);
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
       #if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
        if(strstr(dev->name,"ewan0")!=NULL) 
            status=kerSysGetMacAddress( dev->dev_addr,  0x13ffffff);
        else
#endif
            status = kerSysGetMacAddress(dev->dev_addr, dev->ifindex);

            if (status == 0) {
                memmove(sockaddr.sa_data, dev->dev_addr, ETH_ALEN);
            }
#endif

          dev->priv_flags |= IFF_WANDEV;
          dev->priv_flags &= ~IFF_HW_SWITCH;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        dev_set_mac_address(dev, &sockaddr);
#else
        dev->set_mac_address(dev, &sockaddr);
#endif

        /* Note: The parameter i should be the vport_id-1. The ethsw_set_mac
           maps it to physical port id */
        if(pVnetDev0->unit == 0)
            ethsw_set_mac(i-1, phys);

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96828)
        if((pVnetDev0->unit == 1) && IsExternalSwitchPort(j)){
          dev->priv_flags |= IFF_EXT_SWITCH;
        }
#endif

        printk("%s: MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
            dev->name,
            dev->dev_addr[0],
            dev->dev_addr[1],
            dev->dev_addr[2],
            dev->dev_addr[3],
            dev->dev_addr[4],
            dev->dev_addr[5]);
    }

    return 0;
}

#undef OFFSETOF
#define OFFSETOF(STYPE, MEMBER)     ((size_t) &((STYPE *)0)->MEMBER)

#if defined(CONFIG_BCM963268)
static int bcm_strip_tag_type1(struct sk_buff *skb, bool strip_brcm_tag)
#else
static int bcm_strip_tag_type1(struct sk_buff *skb)
#endif
{
    unsigned int end_offset = 0;

#if defined(CONFIG_BCM963268)
    if (strip_brcm_tag && ((BcmEnet_hdr*)skb->data)->brcm_type == BRCM_TYPE)
#else
    if (((BcmEnet_hdr*)skb->data)->brcm_type == BRCM_TYPE)
#endif
    {
#ifdef VLAN_TAG_FFF_STRIP
        if ((((BcmVlan_ethhdr*)skb->data)->vlan_proto == VLAN_TYPE) &&
            ((((BcmVlan_ethhdr*)skb->data)->vlan_TCI & VLAN_VID_MASK) == 0xFFF))
        {
            /* Both BRCM TAG and VID_FFF tag present */
            end_offset = BRCM_TAG_LEN + VLAN_HLEN;
        } else
#endif
        {
            /* VID_FFF tag not present but BRCM Tag Present */
            end_offset = BRCM_TAG_LEN;
        }
    }
#ifdef VLAN_TAG_FFF_STRIP
    else if ((((struct vlan_ethhdr*)skb->data)->h_vlan_proto == VLAN_TYPE) &&
        ((((struct vlan_ethhdr*)skb->data)->h_vlan_TCI & VLAN_VID_MASK) == 0xFFF))
    {
        /* BRCM Tag not present; VLAN_FFF present */
        end_offset = VLAN_HLEN;
    }
#endif
    return end_offset;
}

#if defined(CONFIG_BCM963268)
static int bcm_strip_tag_type2(struct sk_buff *skb, bool strip_brcm_tag)
#else
static int bcm_strip_tag_type2(struct sk_buff *skb)
#endif
{
    unsigned int end_offset = 0;

#if defined(CONFIG_BCM963268)
    if (strip_brcm_tag && ((BcmEnet_hdr*)skb->data)->brcm_type == BRCM_TYPE2)
#else
    if (((BcmEnet_hdr*)skb->data)->brcm_type == BRCM_TYPE2)
#endif
    {
#ifdef VLAN_TAG_FFF_STRIP
        if ((((BcmVlan_ethhdr2*)skb->data)->vlan_proto == VLAN_TYPE) &&
             ((((BcmVlan_ethhdr2*)skb->data)->vlan_TCI & VLAN_VID_MASK) == 0xFFF))
        {
            /* Both BRCM TAG2 and VID_FFF tag present. */
            end_offset = BRCM_TAG_TYPE2_LEN + VLAN_HLEN;
        } else
#endif
        {
            /* BRCM Tag Present, So memmove by BRCM_TAG_LEN */
            end_offset = BRCM_TAG_TYPE2_LEN;
        }
    }
#ifdef VLAN_TAG_FFF_STRIP
     /* BRCM Tag not present; VLAN_FFF present */
    else if ((((struct vlan_ethhdr*)skb->data)->h_vlan_proto == VLAN_TYPE) &&
        ((((struct vlan_ethhdr*)skb->data)->h_vlan_TCI & VLAN_VID_MASK) == 0xFFF))
    {
        end_offset = VLAN_HLEN;
    }
#endif
    return end_offset;
}

/*
 *  This is a modified version of eth_type_trans(), for taking care of
 *  Broadcom Tag with Ethernet type BRCM_TYPE [0x8874].
 */

#if defined(CONFIG_BCM963268)
static unsigned short bcm_type_trans(struct sk_buff *skb, struct net_device *dev, int strip_tag)
#else
static unsigned short bcm_type_trans(struct sk_buff *skb, struct net_device *dev)
#endif
{
    struct ethhdr *eth;
    unsigned char *rawp;
    unsigned int end_offset = 0, from_offset = 0;
    uint16 *to, *end, *from;
    unsigned int hdrlen = sizeof(struct ethhdr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    skb_reset_mac_header(skb);
#else
    skb->mac.raw = skb->data;
#endif

#if defined(CONFIG_BCM963268)
    end_offset = global.bcm_strip_tag(skb, strip_tag);
#else
    end_offset = global.bcm_strip_tag(skb);
#endif
    if (end_offset)
    {
        from_offset = OFFSETOF(struct ethhdr, h_proto);
    
        to = (uint16*)(skb->data + from_offset + end_offset) - 1;
        end = (uint16*)(skb->data + end_offset) - 1;
        from = (uint16*)(skb->data + from_offset) - 1;

        while ( to != end )
            *to-- = *from--;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
     skb_set_mac_header(skb, end_offset);
#else
     skb->mac.raw += end_offset;
#endif

    hdrlen += end_offset;

    skb_pull(skb, hdrlen);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    eth = (struct ethhdr *)skb_mac_header(skb);
#else
    eth = (struct ethhdr *)skb->mac.raw;
#endif

    if(*eth->h_dest&1)
    {
        if(memcmp(eth->h_dest,dev->broadcast, ETH_ALEN)==0)
            skb->pkt_type=PACKET_BROADCAST;
        else
            skb->pkt_type=PACKET_MULTICAST;
    }

    /*
     *  This ALLMULTI check should be redundant by 1.4
     *  so don't forget to remove it.
     *
     *  Seems, you forgot to remove it. All silly devices
     *  seems to set IFF_PROMISC.
     */

    else if(1 /*dev->flags&IFF_PROMISC*/)
    {
        if(memcmp(eth->h_dest,dev->dev_addr, ETH_ALEN))
            skb->pkt_type=PACKET_OTHERHOST;
    }

    if (ntohs(eth->h_proto) >= 1536)
        return eth->h_proto;

    rawp = skb->data;

    /*
     *  This is a magic hack to spot IPX packets. Older Novell breaks
     *  the protocol design and runs IPX over 802.3 without an 802.2 LLC
     *  layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
     *  won't work for fault tolerant netware but does for the rest.
     */
    if (*(unsigned short *)rawp == 0xFFFF)
        return htons(ETH_P_802_3);

    /*
     *  Real 802.2 LLC
     */
    return htons(ETH_P_802_2);
}

/******************************************************************************
* Function: enetDmaStatus (for debug)                                         *
* Description: Dumps information about the status of the ENET IUDMA channel   *
******************************************************************************/
void enetDmaStatus(int channel)
{
    BcmPktDma_EthRxDma *rxdma;
    BcmPktDma_EthTxDma *txdma;

    rxdma = &g_pEnetDevCtrl->rxdma[channel]->pktDmaRxInfo;
    txdma = g_pEnetDevCtrl->txdma[channel];

    printk("ENET IUDMA INFO CH %d\n", channel);
    if(channel < cur_rxdma_channels)
    {
        printk("enet dmaStatus: rxdma 0x%x, cfg at 0x%x\n",
            (unsigned int)rxdma, (unsigned int)&rxdma->rxDma->cfg);


        printk("RXDMA STATUS: HeadIndex: %d TailIndex: %d numRxBds: %d rxAssignedBds: %d\n",
                  rxdma->rxHeadIndex, rxdma->rxTailIndex,
                  rxdma->numRxBds, rxdma->rxAssignedBds);

        printk("RXDMA CFG: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n",
                     rxdma->rxDma->cfg,
                     rxdma->rxDma->intStat,
                     rxdma->rxDma->intMask);
    }

    if(channel < cur_txdma_channels)
    {

        printk("TXDMA STATUS: HeadIndex: %d TailIndex: %d txFreeBds: %d\n",
                  txdma->txHeadIndex,
                  txdma->txTailIndex,
                  txdma->txFreeBds);

        printk("TXDMA CFG: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n",
                     txdma->txDma->cfg,
                     txdma->txDma->intStat,
                     txdma->txDma->intMask);
    }
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_open
 Purpose: Open and Initialize the EMAC on the chip
-------------------------------------------------------------------------- */
static int bcm63xx_enet_open(struct net_device * dev)
{
    int channel = 0;
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    BcmPktDma_EthTxDma *txdma;

    if (dev != vnet_dev[0])
    {
        if ((vnet_dev[0]->flags & IFF_UP) == 0)
            return -ENETDOWN;

        netif_start_queue(dev);
        return 0;
    }

    ASSERT(pDevCtrl != NULL);
    TRACE(("%s: bcm63xx_enet_open\n", dev->name));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    /* napi_enable must be called before the interrupts are enabled
       if an interrupt comes in before napi_enable is called the napi
       handler will not run and the interrupt will not be re-enabled */
    napi_enable(&pDevCtrl->napi);
#endif

    ENET_RX_LOCK();
    pDevCtrl->dmaCtrl->controller_cfg |= DMA_MASTER_EN;
    /*  Enable the Rx DMA channels and their interrupts  */
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
        rxchannel_isr_enable[channel] = 1;
#endif
        bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);
        bcmPktDma_BcmHalInterruptEnable(channel, rxdma->rxIrq);
    }
    ENET_RX_UNLOCK();

    ENET_TX_LOCK();
    /*  Enable the Tx DMA channels  */
    for (channel = 0; channel < cur_txdma_channels; channel++) {
        txdma = pDevCtrl->txdma[channel];
        bcmPktDma_EthTxEnable(txdma);
        txdma->txEnabled = 1;
    }
    ENET_TX_UNLOCK();

    netif_start_queue(dev);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#if defined(CONFIG_BCM963268)
{
    struct ethswctl_data e2;

    /* Needed to allow iuDMA split override to work properly - Feb 2011 */
    /* Set the Switch Control and QoS registers later than init for the 63268 */

    /* The equivalent of "ethswctl -c cosqsched -v BCM_COSQ_COMBO -q 2 -x 1 -y 1 -z 1 -w 1" */
    /* This assigns equal weight to each of the 4 egress queues */
    /* This means the rx splitting feature cannot co-exist with h/w QOS */
    e2.type = TYPE_SET;
    e2.queue = 2;   /* mode */
    e2.scheduling = BCM_COSQ_COMBO;
    e2.weights[0] = e2.weights[1] = e2.weights[2] = e2.weights[3] = 1;
    enet_ioctl_ethsw_cosq_sched(&e2);

    /* The equivalent of "ethswctl -c cosq -q 1 -v 1" */
    /* This associates egress queue 1 on the switch to iuDMA1 */
    e2.type = TYPE_SET;
    e2.queue = 1;
    e2.channel = 1;
    enet_ioctl_ethsw_cosq_rxchannel_mapping(&e2);

}
#endif
#endif

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_close
    Purpose: Stop communicating with the outside world
    Note: Caused by 'ifconfig ethX down'
-------------------------------------------------------------------------- */
static int bcm63xx_enet_close(struct net_device * dev)
{
    int channel = 0;
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    BcmPktDma_EthTxDma *txdma;

    if (dev != vnet_dev[0])
    {
        netif_stop_queue(dev);
        return 0;
    }

    ASSERT(pDevCtrl != NULL);
    TRACE(("%s: bcm63xx_enet_close\n", dev->name));

    netif_stop_queue(dev);

    ENET_RX_LOCK();
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
        bcmPktDma_BcmHalInterruptDisable(channel, rxdma->rxIrq);
        bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);
    }
    ENET_RX_UNLOCK();

    ENET_TX_LOCK();
    for (channel = 0; channel < cur_txdma_channels; channel++) {

        txdma = pDevCtrl->txdma[channel];
        txdma->txEnabled = 0;
        bcmPktDma_EthTxDisable(txdma);
    }
    ENET_TX_UNLOCK();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    napi_disable(&pDevCtrl->napi);
#endif

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_timeout
 Purpose:
-------------------------------------------------------------------------- */
static void bcm63xx_enet_timeout(struct net_device * dev)
{
    ASSERT(dev != NULL);
    TRACE(("%s: bcm63xx_enet_timeout\n", dev->name));

    dev->trans_start = jiffies;
    netif_wake_queue(dev);
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_query
 Purpose: Return the current statistics. This may be called with the card
          open or closed.
-------------------------------------------------------------------------- */
static struct net_device_stats *
bcm63xx_enet_query(struct net_device * dev)
{
#ifdef REPORT_HARDWARE_STATS 
    int port, extswitch = 0;

#if !defined(CONFIG_BCM963268)  
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
#endif

    port = port_id_from_dev(dev);
#if defined(CONFIG_BCM96816)
    if ((SERDES_PORT_ID == port) &&
        IsExt6829(pVnetDev0->EnetInfo[0].sw.phy_id[port]) )
    {
        extswitch = 1;
        port = dev->base_addr;
    }
#endif

#if defined(CONFIG_BCM963268)    
    if ( (extSwInfo.present == 1) && (port < MAX_EXT_SWITCH_PORTS)){    
        extswitch = 1;
    }    
    port = LOGICAL_PORT_TO_PHYSICAL_PORT(port);
#else
    if (pVnetDev0->extSwitch->brcm_tag_type == BRCM_TYPE2) {
        extswitch = 1;
    }
#endif

    if ((port < 0) || (port >= TOTAL_SWITCH_PORTS)) {
        printk("Invalid port, so stats will not be correct \n");
    } else {
        struct net_device_stats *stats = &global.net_device_stats_from_hw;
        BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
#if defined(AEI_VDSL_STATS_DIAG)
        ethsw_get_hw_stats(port, extswitch, stats, dev);
#else
        ethsw_get_hw_stats(port, extswitch, stats);
#endif
        /* Add the dropped packets in software */
        stats->rx_dropped += pDevCtrl->stats.rx_dropped;
        stats->tx_dropped += pDevCtrl->stats.tx_dropped;
        stats->multicast += pDevCtrl->stats.multicast;
    }
    return &global.net_device_stats_from_hw;
#else
    return &(((BcmEnet_devctrl *)netdev_priv(dev))->stats);
#endif
}

static int bcm63xx_enet_change_mtu(struct net_device *dev, int new_mtu)
{
    //printk("[%s.%d] %s->mtu changing to %d (was %d)\n", __func__, __LINE__, dev->name, new_mtu, dev->mtu);
    if (new_mtu < ETH_ZLEN || new_mtu > ETH_DATA_LEN)
        return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

#if defined(RXCHANNEL_BYTE_RATE_LIMIT) && defined(CONFIG_BCM96816)
static int channel_rx_rate_limit_enable[ENET_RX_CHANNELS_MAX] = {0};
static int rx_bytes_from_last_jiffies[ENET_RX_CHANNELS_MAX] = {0};
/* default rate in bytes/sec */
static int channel_rx_rate_credit[ENET_RX_CHANNELS_MAX] = {1000000};
static int last_byte_jiffies[ENET_RX_CHANNELS_MAX] = {0};
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
/*
 * bcm63xx_timer: 100ms timer for updating rx rate control credits
 */
static void bcm63xx_timer(unsigned long arg)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    BcmEnet_RxDma *rxdma;
    unsigned int elapsed_msecs;
    int channel;
    struct sched_param param;

    /* */
    daemonize("bcmsw_timer");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    param.sched_priority = CONFIG_BRCM_SOFTIRQ_BASE_RT_PRIO;
    sched_setscheduler(current, SCHED_RR, &param);
    set_user_nice(current, 0);
#else
    param.sched_priority = poll_prio.prio;
    sched_setscheduler(current, poll_prio.type, &param);
    set_user_nice(current, poll_prio.nice);
#endif

    /* */
    while (atomic_read(&timer_lock) > 0)
    {
        for (channel = 0; channel < cur_rxdma_channels; channel++) {
            ENET_RX_LOCK();
            if (rxchannel_rate_limit_enable[channel]) {
                elapsed_msecs = jiffies_to_msecs(jiffies -
                                  last_pkt_jiffies[channel]);
                if (elapsed_msecs >= 99) {
                    rxdma = priv->rxdma[channel];
                    BCM_ENET_INFO("pkts_from_last_jiffies = %d \n",
                                   rx_pkts_from_last_jiffies[channel]);
                    rx_pkts_from_last_jiffies[channel] = 0;
                    last_pkt_jiffies[channel] = jiffies;
                    if (rxchannel_isr_enable[channel] == 0) {
                        BCM_ENET_INFO("Enabling DMA Channel & Interrupt \n");
                        switch_rx_ring(priv, channel, 0);
                        bcmPktDma_BcmHalInterruptEnable(channel, rxdma->rxIrq);
                        rxchannel_isr_enable[channel] = 1;
                    }
                }
            }
            ENET_RX_UNLOCK();
        }

        /*  Sleep for HZ/10 jiffies (100ms)  */
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ/10);
    }

    complete_and_exit(&timer_done, 0);
    printk("bcm63xx_timer: thread exits!\n");
}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
struct task_struct *enet_softswitch_xmit_task = NULL;
/*
 * bcm63xx_softswitch_xmit_timer: timer for MoCA
 *   software switching
 */
static int bcm63xx_softswitch_xmit_timer(void * arg)
{
    struct sched_param param;
#if defined (MOCA_HIGH_RES_TX)
    ktime_t ktime  = ktime_set(0, 250000);
#endif

    daemonize("bcm63xx_softswitch_xmit_timer");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    param.sched_priority = CONFIG_BRCM_SOFTIRQ_BASE_RT_PRIO;
    sched_setscheduler(current, SCHED_RR, &param);
    set_user_nice(current, 0);
#else
    param.sched_priority = poll_prio.prio;
    sched_setscheduler(current, poll_prio.type, &param);
    set_user_nice(current, poll_prio.nice);
#endif

    while ( 1 )
    {
        if (kthread_should_stop())
           break;

        if ( global.moca_lan.queuedPackets ||
             (global.Is6829 && global.moca_wan.queuedPackets) )
        {
            tasklet_schedule(&mocaTasklet);
        }

        set_current_state(TASK_INTERRUPTIBLE);
#if defined (MOCA_HIGH_RES_TX)
        schedule_hrtimeout(&ktime, HRTIMER_MODE_REL);
#else
        /*  Sleep for HZ/1000 jiffies (1ms)  */
        schedule_timeout(HZ/1000);
#endif
    }

    printk("bcm63xx_softswitch_xmit_timer: thread exits!\n");
    return 0;
}

/*
 * bcm63xx_moca_xmit_tasklet: tasklet for moca tx
 */
static void bcm63xx_moca_xmit_tasklet(unsigned long arg)
{
    moca_send_packets(0, 0);
    if (global.Is6829)
    {
        int idx = atomic_read(&bcm6829ActDevIdx);
        if (idx == BCM6829_MOCA_DEV)
        {
            moca_send_packets(1, 0);
        }
    }
}
#endif /* defined(CONFIG_BCM_MOCA_SOFT_SWITCHING) */

static int bcmenet_set_spdled(int port, int speed)
{
   if ( IsExt6829(port) )
   {
      ETHERNET_MAC_INFO EnetInfo;

      if ( (port & ~BCM_EXT_6829) > 0 )
         return 0;

      /* toggle spd100 and spd1000 GPIOs for 6829 LEDs */
      if ( BpGetEthernetMacInfo( &EnetInfo, 1 ) == BP_SUCCESS )
      {
         unsigned int    tempReg;
         unsigned int    dirAddr;
         unsigned int    ioAddr;
         int             gpioData;
         int             ledGpio;
         unsigned short *p16;
         int             i;
         int             speedCheck = 100;

         p16 = &EnetInfo.sw.ledInfo[0].speedLed100;
         for ( i = 0; i < 2; i++ )
         {
            if ( p16[i] != BP_NOT_DEFINED )
            {
               ledGpio = p16[i];
               /* check 100 for 100 led and 1000 for 1000 led */
               if ( speed == speedCheck )
               {
                  if ( ledGpio & BP_ACTIVE_LOW )
                     gpioData = 0;
                  else
                     gpioData = 1;
               }
               else
               {
                  if ( ledGpio & BP_ACTIVE_LOW )
                     gpioData = 1;
                  else
                     gpioData = 0;
               }
               ledGpio &= BP_GPIO_NUM_MASK;
               if ( ledGpio > 31 )
               {
                  ledGpio  -= 32;

                  dirAddr  = (unsigned int)&GPIO->GPIODir;
                  ioAddr   = (unsigned int)&GPIO->GPIOio;
               }
               else
               {
                  dirAddr = (unsigned int)&GPIO->GPIODir + 4;
                  ioAddr  = (unsigned int)&GPIO->GPIOio + 4;
               }

               tempReg = kerSysBcmSpiSlaveReadReg32(dirAddr);
               tempReg |= (1 << ledGpio);
               kerSysBcmSpiSlaveWriteReg32(dirAddr, tempReg);

               tempReg = kerSysBcmSpiSlaveReadReg32(ioAddr);
               if ( gpioData )
                  tempReg |= (1 << ledGpio);
               else
                  tempReg &= ~(1 << ledGpio);
               kerSysBcmSpiSlaveWriteReg32(ioAddr, tempReg);
               speedCheck = 1000; /* check 1000 next time through */
            }
         }
      }
   }
   else
   {
      int led100;
      int led1000;

      if ( port > 1 )
         return 0;

      if( 0 == port )
      {
         led100  = kLedEth0Spd100;
         led1000 = kLedEth0Spd1000;
      }
      else
      {
         led100  = kLedEth1Spd100;
         led1000 = kLedEth1Spd1000;
      }

      if ( speed == 1000 )
      {
         kerSysLedCtrl(led100, kLedStateOff);
         kerSysLedCtrl(led1000, kLedStateOn);
      }
      else if (speed == 100)
      {
         kerSysLedCtrl(led100, kLedStateOn);
         kerSysLedCtrl(led1000, kLedStateOff);
      }
      else /* 10 or off */
      {
         kerSysLedCtrl(led100, kLedStateOff);
         kerSysLedCtrl(led1000, kLedStateOff);
      }
   }

   return 0;
}
void bcmsw_set_ae_ipg(void)
{
    uint8 v8 = (defaultIPG & 0xF8) | 0x3; /* Min IPG >= 8B */
    ethsw_wreg_ext(PAGE_CONTROL, 0x0A, (uint8 *)&v8, 1, global.Is6829);
}
void bcmsw_set_default_ipg(void)
{
    ethsw_wreg_ext(PAGE_CONTROL, 0x0A, (uint8 *)&defaultIPG, 1, global.Is6829);
}
#endif

struct semaphore bcm_link_handler_config;
DECLARE_MUTEX(bcm_link_handler_config);
/*
 * handle_link_status_change
 */
void link_change_handler(int port, int linkstatus, int speed, int duplex)
{
    IOCTL_MIB_INFO *mib;
    int mask, vport;
    struct net_device *dev = vnet_dev[0];
    struct net_device *pNetDev;
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    int linkMask;
    int sw_port;
#if defined(CONFIG_BCM_ETH_PWRSAVE)
    int phyId = 0; // 0 is a valid phy_id but not an external phy_id. So we are OK initializing it to 0.
    if (extSwInfo.present == 1) {
        if (!IsExternalSwitchPort(port)) {
            phyId = priv->EnetInfo[0].sw.phy_id[LOGICAL_PORT_TO_PHYSICAL_PORT(port)];
        }
    } else {
        phyId = priv->EnetInfo[0].sw.phy_id[port];
    }
#endif

    down(&bcm_link_handler_config);

#if defined(CONFIG_BCM96816)
    if ( IsExt6829(port) )
    {
       sw_port  = port & ~BCM_EXT_6829;
       mask     = 1 << (sw_port + MAX_SWITCH_PORTS);
       linkMask = linkstatus << (sw_port + MAX_SWITCH_PORTS);
       pNetDev  = bcm6829_to_dev[phyport_to_vport_6829[sw_port]];
       vport    = port; /* for ethsw_set_mac */
    }
    else
#endif
    {
       sw_port  = port;
       vport    = phyport_to_vport[port];
       mask     = 1 << port;
       pNetDev  = vnet_dev[vport];
       linkMask = linkstatus << port;
       vport   -= 1; /* -1 for ethsw_set_mac */
    }

    if ( NULL == pNetDev )
    {
        up(&bcm_link_handler_config);
        return;
    }

    if ((priv->linkState & mask) != linkMask) {
        BCM_ENET_DEBUG("port=%x; vport=%x", port, vport);

        mib = &((BcmEnet_devctrl *)netdev_priv(pNetDev))->MibInfo;
        if (linkstatus) {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
            /* Link is up, so de-isolate the Phy  */
            if (IsExtPhyId(phyId)) {
                ethsw_isolate_phy(phyId, 0);
            }
#if defined(GPHY_EEE_1000BASE_T_DEF)
            /* Just set the flag because a 1 second delay is required */
            priv->eee_enable_request_flag |= (1<<sw_port);
#endif
#endif
            if (netif_carrier_ok(pNetDev) == 0)
                netif_carrier_on(pNetDev);
            if (speed == 1000)
                mib->ulIfSpeed = SPEED_1000MBIT;
            else if (speed == 100)
                mib->ulIfSpeed = SPEED_100MBIT;
            else
                mib->ulIfSpeed = SPEED_10MBIT;
            mib->ulIfLastChange  = (jiffies * 100) / HZ;
            mib->ulIfDuplex = (unsigned long)duplex;
            priv->linkState |= mask;
#if defined(CONFIG_BCM96816)
            /* set spd led for the internal phy */
            bcmenet_set_spdled(port, speed);

#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
            if ( MOCA_PORT_ID == sw_port )
            {
                if (IsExt6829(vport))
                {
                    /* set active device to MoCA right away */
                    atomic_set(&bcm6829ActDevIdx, 0);
                }
            }
#endif
            if ( MOCA_PORT_ID != sw_port )
#endif
            printk((KERN_CRIT "%s (switch port: %d) Link UP %d mbps %s duplex\n"),
                    pNetDev->name, sw_port, speed, duplex?"full":"half");

#if defined(AEI_63168_CHIP)
            if (memcmp(pNetDev->name, "ewan", 4) == 0)
            {
                kerSysLedCtrl(kLedEnetWan, kLedStateOn);
#if defined(AEI_VDSL_SW_WAN_ETH_LED)
                enetWanLedOn=1;
#endif
            }
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS) && !defined(AEI_63168_CHIP)      
                if (extSwitchExist)
                {
                    uint8 v8=0;
                    bcmsw_pmdio_rreg(0x00, port, (uint8 *)&v8, sizeof(v8));
                    /* if tx/rx not enabled, then enable */
                    if (v8 & 0x3)
                    {
                       v8 &= ~(0x3); /* enable tx/rx */
                       bcmsw_pmdio_wreg(0x00, port, (uint8 *)&v8, sizeof(v8));
                    }
                }
#endif
            
        } else {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
            /* Link is down, so isolate the Phy. To prevent switch rx lockup 
            because of packets entering switch with DLL/Clock disabled */
            if (IsExtPhyId(phyId)) {
                ethsw_isolate_phy(phyId, 1);
            }
#if defined(GPHY_EEE_1000BASE_T_DEF)
            /* Clear any pending request to enable eee and disable it */
            priv->eee_enable_request_flag &= ~((1<<sw_port) | (1<<(sw_port+MAX_SWITCH_PORTS)));
            ethsw_eee_link_enable(sw_port, 0);
#endif
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
/* enable fast age will reduce the performance of HPNA<->LAN */
#if defined(CONFIG_BCM963268) || defined (CONFIG_BCM96828)
            if ((extSwInfo.present == 1) && IsExternalSwitchPort(sw_port)) {
                extsw_fast_age_port(sw_port, 0);
            } else {
                fast_age_port(LOGICAL_PORT_TO_PHYSICAL_PORT(sw_port), 0);
            }
#else
            if (extSwInfo.present == 1) {
                extsw_fast_age_port(sw_port, 0);
            } else {
                fast_age_port(sw_port, 0);
            }
#endif
#endif

            if (netif_carrier_ok(pNetDev) != 0)
                netif_carrier_off(pNetDev);
            mib->ulIfLastChange  = 0;
            mib->ulIfSpeed       = 0;
            mib->ulIfDuplex      = 0;
            priv->linkState &= ~mask;
#if defined(CONFIG_BCM96816)
            /* set spd led for the internal phy */
            bcmenet_set_spdled(vport, 0);

            if ( MOCA_PORT_ID != sw_port )
#endif
            printk((KERN_CRIT "%s (switch port: %d)  Link DOWN.\n"), pNetDev->name, sw_port);

#if defined(AEI_63168_CHIP)
            if (memcmp(pNetDev->name, "ewan", 4) == 0)
            {
               kerSysLedCtrl(kLedEnetWan, kLedStateOff);
#if defined(AEI_VDSL_SW_WAN_ETH_LED)
               enetWanLedOn=0;
#endif
            }
#endif
        }

        kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,NULL,0);

#if defined(CONFIG_BCM96816)
        if ( (MOCA_PORT_ID == sw_port) ||
             (IsExt6829(vport) ) )
        {
            PHY_STAT phys;

            phys.fdx = (duplex ? 1 : 0);
            phys.lnk = (linkstatus ? 1 : 0);
            if ( 1000 == speed )
            {
               phys.spd1000 = 1;
               phys.spd100  = 0;
            }
            else if ( 100 == speed )
            {
               phys.spd1000 = 0;
               phys.spd100  = 1;
            }
            else
            {
               phys.spd1000 = 0;
               phys.spd100  = 0;
            }
            ethsw_set_mac(vport, phys);
        }
#endif
    }

    up(&bcm_link_handler_config);
}



#if defined(SUPPORT_SWMDK)
static int link_change_handler_wrapper(void *ctxt)
{
  LinkChangeArgs *args = ctxt;

  BCM_ASSERT(args);
#if defined(CONFIG_BCM96816)
  if (args->activeELink)
  {
      if (args->linkstatus == 1)
      { /* AE link going up */
          bcmsw_set_ae_ipg();
      }
      else
      { /* AE link going down */
          bcmsw_set_default_ipg();
      }
  }
#endif /* BCM96816 */

  link_change_handler(args->port,
                      args->linkstatus,
                      args->speed,
                      args->duplex);
  return 0;
}

static void bcm63xx_enet_poll_timer(unsigned long arg)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    int i;
#if defined(CONFIG_BCM96816)
    int phyId;
#endif

    /* */
    daemonize("bcmsw");

    BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
    BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif

    /* Enable the Phy interrupts of internal Phys */
    for (i = 0; i < TOTAL_SWITCH_PORTS - 1; i++) {
        if ((priv->EnetInfo[0].sw.port_map) & (1<<i)) {
            if (!IsExtPhyId(priv->EnetInfo[0].sw.phy_id[i])) {
                ethsw_phy_intr_ctrl(i, 1);
            } else {
                global.extPhyMask |= (1 << i);
            }
        }
    }

    /* Start with virtual interfaces as down */
    for (i = 1; i <= vport_cnt; i++) {
        if ( vnet_dev[i] != NULL )
        {
           if (netif_carrier_ok(vnet_dev[i]) != 0)
               netif_carrier_off(vnet_dev[i]);
        }
    }

#if defined(CONFIG_BCM96816)
    for( i = 0; i < MAX_6829_IFS; i++ )
    {
        if ( bcm6829_to_dev[i] != NULL )
        {
           if (netif_carrier_ok(bcm6829_to_dev[i]) != 0)
               netif_carrier_off(bcm6829_to_dev[i]);
        }
    }
#endif

    /* */
    while (atomic_read(&poll_lock) > 0)
    {
        /* reclaim tx descriptors and buffers */
        bcm63xx_xmit_reclaim();

#if defined(CONFIG_BCM96816)
        phyId = priv->EnetInfo[0].sw.phy_id[SERDES_PORT_ID];
        if ( IsExt6829(phyId) )  {
            int newstat = 0;
            check_6829_vports(phyId, &newstat);
        }
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        /* Add code for buffer quick free between enet and xtm - June 2010 */
        if(xtm_fkb_recycle_hook == NULL)
            xtm_fkb_recycle_hook = bcmPktDma_get_xtm_fkb_recycle();
#if defined(CONFIG_BCM963268)
        if(xtm_skb_recycle_hook == NULL)
            xtm_skb_recycle_hook = bcmPktDma_get_xtm_skb_recycle();
#endif
#endif

        /*   */
        set_current_state(TASK_INTERRUPTIBLE);

        /* Sleep for HZ jiffies (1sec) */
        schedule_timeout(HZ);
    }

    complete_and_exit(&poll_done, 0);
    printk("bcm63xx_enet_poll_timer: thread exits!\n");
}


#if defined(AEI_VDSL_SW_WAN_ETH_LED)
void AEI_poll_gphy_LED(void)
{
    static uint32 previousCnt = 0;
    static uint8 blinkBlink = 0; 
    uint32 currentCnt = 0;

    if (enetWanLedOn)
    {
        /* could use EthSwMIBRegs *e, e->RxOctetsLo and e->TxOctetsLo to access but not to save some instructions */
        UPDATE_GPHY_OCTET_COUNT(currentCnt);

        /* not locking here as the only other LEDs that may access this register  
         * are POST and POWER_ON which are accessed during bootup.
         * use blinkBlink to take care of timer drift between to polling threads.  
         */
        if (previousCnt != currentCnt)
        {
            if (blinkBlink == 1)
                GPHY_LED_FAST_BLINK(); 	
            else 
            {  
                GPHY_LED_SLOW_BLINK();
                blinkBlink = 1;
            } 
            previousCnt = currentCnt;
        }
	else
        {
            blinkBlink = 0;
            GPHY_LED_ON();
        }  
    }     

    return;
}
#endif

static int enet_ioctl_kernel_poll(void)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    uint32_t port_map = (uint32_t) priv->EnetInfo[0].sw.port_map;
#if defined(AEI_VDSL_HPNA)
    PHY_STAT phys;
    static int hpna_link = 1;
#endif

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_ephy_auto_power_down_wakeup();
#endif

    /* Collect the statistics  */
    ethsw_counter_collect(port_map, 0);

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_ephy_auto_power_down_sleep();

    /* Check for delayed request to enable EEE */
    ethsw_eee_process_delayed_enable_requests();
#endif

#if defined(AEI_VDSL_HPNA)
    if (hpna_support) 
    {
        phys = AEI_ethsw_hpna_phy_stat(hpna_port);
        if (hpna_link != phys.lnk)
        {
            printk("%s Link %s\n",vnet_dev[hpna_dev_index]->name, phys.lnk?"UP":"DOWN");

            hpna_link = phys.lnk;

            if (phys.lnk)
            {
                if (netif_carrier_ok(vnet_dev[hpna_dev_index]) == 0)
                    netif_carrier_on(vnet_dev[hpna_dev_index]);
            }
            else
            {
                if (netif_carrier_ok(vnet_dev[hpna_dev_index]) != 0)
                    netif_carrier_off(vnet_dev[hpna_dev_index]);
            }
        }
    }
#endif

#if defined(AEI_VDSL_SW_WAN_ETH_LED)
    /* Do WAN eth LED here because not in datapath, alternate calling AEI_poll_gphy_LED
     * because swmdk fork and calls enet_ioctl_kernel_poll 2x in every second.
     */
    AEI_poll_gphy_LED();
#endif
 
    return 0;
}

#else
/*
 * bcm63xx_enet_poll_timer: reclaim transmit frames which have been sent out
 */
static void bcm63xx_enet_poll_timer(unsigned long arg)
{
    IOCTL_MIB_INFO *mib;
    PHY_STAT phys;
    int newstat, tmp, mask, i;
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    int ephy_sleep_delay = 0;
    uint32_t port_map = (uint32_t) priv->EnetInfo[0].sw.port_map;

    /* */
    daemonize("bcmsw");

    /* */
    BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
    BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif

    /* Enable the Phy interrupts of internal Phys */
    for (i = 0; i < EPHY_PORTS; i++) {
        if ((priv->EnetInfo[0].sw.port_map) & (1<<i)) {
            if (!IsExtPhyId(priv->EnetInfo[0].sw.phy_id[i])) {
                ethsw_phy_intr_ctrl(i, 1);
            }
        }
    }

    /* Start with virtual interfaces as down */
    for (i = 1; i <= vport_cnt; i++) {
        if ( vnet_dev[i] != NULL )
        {
            if (netif_carrier_ok(vnet_dev[i]) != 0) {
                netif_carrier_off(vnet_dev[i]);
            }
        }
    }

#if defined(CONFIG_BCM96816)
    for( i = 0; i < MAX_6829_IFS; i++ )
    {
        if ( bcm6829_to_dev[i] != NULL )
        {
           if (netif_carrier_ok(bcm6829_to_dev[i]) != 0)
               netif_carrier_off(bcm6829_to_dev[i]);
        }
    }
#endif

    /* */
    while (atomic_read(&poll_lock) > 0)
    {
        /* reclaim tx descriptors and buffers */
        bcm63xx_xmit_reclaim();

        /* Start with New link status of all vports as 0*/
        newstat = 0;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ephy_sleep_delay = ethsw_ephy_auto_power_down_wakeup();
#endif

        for (i = 1; i <= vport_cnt; i++)
        {
            int phyId = priv->EnetInfo[0].sw.phy_id[vport_to_phyport[i]];

#if defined(CONFIG_BCM96816)
            /* Skip GPON interface */
            if(vport_to_phyport[i] == GPON_PORT_ID) {
                continue;
            }
#endif

#if defined(CONFIG_BCM96816)
            if(vport_to_phyport[i] == SERDES_PORT_ID)
            {
                if ( IsExt6829(phyId) )
                {
                    check_6829_vports(phyId, &newstat);
                    continue;
                }
            }
#endif

            /* Mask for this port */
            mask = (1 << vport_to_phyport[i]);

            /* If internal phy and no interrupt, continue */
            if (!IsExtPhyId(phyId)
                && ephy_int_cnt == 0)
            {
                /* Set new status same as old status */
                newstat |= (priv->linkState & mask);
                continue;
            }

            /* Get the MIB of this vport */
            mib = &((BcmEnet_devctrl *)netdev_priv(vnet_dev[i]))->MibInfo;

            /* Get the status of Phy connected to switch port i */
#if defined(CONFIG_BCM96816)
            /* moca daemon notifies eth driver of link state
               this calls link_change_handler which sets the link */
            if ( MOCA_PORT_ID == vport_to_phyport[i] )
            {
               newstat |= (priv->linkState & mask);
               continue;
            }
            else
#endif
            phys = ethsw_phy_stat(i - 1);

            /* If link is up, set tmp with the mask of this port */
            tmp = (phys.lnk != 0) ? mask : 0;

            /* Update the new link status */
            newstat |= tmp;

            /* If link status has changed for this switch port i, update
               the interface status */
            if ((priv->linkState & mask) != tmp)
            {
                /* Set the MAC with link/speed/duplex status from Phy */
                /* Note: The parameter i should be the vport id. The
                   ethsw_set_mac maps it to physical port id */
                ethsw_set_mac(i - 1, phys);

                /* If Link has changed from down to up, indicate upper layers
                   and print the link status */
                if (phys.lnk)
                {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
                    /* Link is up, so de-isolate the Phy  */
                    if (IsExtPhyId(phyId)) {
                        ethsw_isolate_phy(phyId, 0);
                    }
#endif

                    if (netif_carrier_ok(vnet_dev[i]) == 0)
                        netif_carrier_on(vnet_dev[i]);

                    if (phys.spd100)
                      mib->ulIfSpeed = SPEED_100MBIT;
                    else if (!phys.spd1000)
                      mib->ulIfSpeed = SPEED_10MBIT;

                    mib->ulIfLastChange  = (jiffies * 100) / HZ;

#if defined(CONFIG_BCM96816)
                    if ( MOCA_PORT_ID != vport_to_phyport[i] )
#endif
                    {
                        int speed;

                        if (phys.spd1000)
                            speed=1000;
                        else if (phys.spd100)
                            speed=100;
                        else
                            speed=10;

                        printk((KERN_CRIT "%s Link UP %d mbps %s duplex\n"),
                                vnet_dev[i]->name, speed, phys.fdx?"full":"half");
                    }
                }
                else
                {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
                    /* Link is down, so isolate the Phy. To prevent switch rx lockup 
                    because of packets entering switch with DLL/Clock disabled */
                    if (IsExtPhyId(phyId)) {
                        ethsw_isolate_phy(phyId, 1);
                    }
#endif

                    /* If link has changed from up to down, indicate upper
                       layers and print the 'Link Down' message */
                    if (netif_carrier_ok(vnet_dev[i]) != 0)
                        netif_carrier_off(vnet_dev[i]);

                    mib->ulIfLastChange  = 0;
                    mib->ulIfSpeed       = 0;
#if defined(CONFIG_BCM96816)
                    if ( MOCA_PORT_ID != vport_to_phyport[i] )
#endif
                    {
                        printk((KERN_CRIT "%s Link DOWN.\n"), vnet_dev[i]->name);
                    }
                }
            }
        }

        /* If there was a link status change, update linkStatus to newstat */
        if (priv->linkState != newstat)
        {
          ephy_int_cnt = 0;
          BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
          BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif
          priv->linkState = newstat;
          kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_LINK_STATUS_CHANGED,NULL,0);
        }

        /* Collect the statistics  */
        ethsw_counter_collect(port_map, 0);


#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ephy_sleep_delay += ethsw_ephy_auto_power_down_sleep();

        /* Check for delayed request to enable EEE */
        ethsw_eee_process_delayed_enable_requests();
#endif

        /*   */
        set_current_state(TASK_INTERRUPTIBLE);

        /* Sleep for HZ jiffies (1sec), minus the time that was already */
        /* spent waiting for EPHY PLL  */
        schedule_timeout(HZ - ephy_sleep_delay);
    }

    complete_and_exit(&poll_done, 0);
    printk("bcm63xx_enet_poll_timer: thread exits!\n");
}
#endif

static int bcm63xx_xmit_reclaim(void)
{
    int i;
    pNBuff_t pNBuff;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    BcmPktDma_txRecycle_t txRecycle;
    BcmPktDma_txRecycle_t *txRecycle_p;

    /* Obtain exclusive access to transmitter.  This is necessary because
    * we might have more than one stack transmitting at once. */
    ENET_TX_LOCK();
    for (i = 0; i < cur_txdma_channels; i++)
    {
        while ((txRecycle_p = bcmPktDma_EthFreeXmitBufGet(pDevCtrl->txdma[i], &txRecycle)) != NULL)
        {
           pNBuff = (pNBuff_t)txRecycle_p->key;

           BCM_ENET_RX_DEBUG("bcmPktDma_EthFreeXmitBufGet TRUE! (reclaim) key 0x%x\n", (int)pNBuff);
           if (pNBuff != PNBUFF_NULL) {
               ENET_TX_UNLOCK();
               nbuff_free(pNBuff);
               ENET_TX_LOCK();
           }
        }   /* end while(...) */
    }   /* end for(...) */
    ENET_TX_UNLOCK();

    return 0;
}

static struct sk_buff *bcm63xx_skb_put_tag(struct sk_buff *skb,
    struct net_device *dev, unsigned int port_map)
{
    BcmEnet_hdr *pHdr = (BcmEnet_hdr *)skb->data;
    int i, headroom;
    int tailroom;

    if (pHdr->brcm_type == BRCM_TYPE2) {
        headroom = 0;
        tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - skb->len;
    } else {
        headroom = BRCM_TAG_TYPE2_LEN;
        tailroom = ETH_ZLEN - skb->len;
    }

    if (tailroom < 0) {
        tailroom = 0;
    }

    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom)) {
        struct sk_buff *oskb = skb;
        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);
        if (!skb)
            return NULL;
    } else if (headroom != 0) {
        skb = skb_unshare(skb, GFP_ATOMIC);
        if (!skb)
            return NULL;
    }

    if (tailroom > 0) {
        if (skb_is_nonlinear(skb)) {
            /* Non linear skb whose skb->len is < minimum Ethernet Packet Length 
                         (ETHZLEN or ETH_ZLEN + BroadcomMgmtTag Length) */
            if (skb_linearize(skb)) {
                return NULL;
            }
        }
        memset(skb->data + skb->len, 0, tailroom);  /* padding to 0 */
        skb_put(skb, tailroom);
    }

    if (headroom != 0) {
        uint16 *to, *from;
        BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)skb_push(skb, headroom);
        to = (uint16*)pHdr;
        from = (uint16*)(skb->data + headroom);
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set ingress brcm tag and TC bit */
        pHdr->brcm_type = BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(skb->mark) << 10);
        pHdr->brcm_tag = port_map;
    }
    return skb;
}

static inline void bcm63xx_fkb_put_tag(FkBuff_t * fkb_p,
    struct net_device * dev, unsigned int port_map)
{
    int i;
    int tailroom, crc_len = 0;
    uint16 *from = (uint16*)fkb_p->data;
    BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)from;

    if (pHdr->brcm_type != BRCM_TYPE2) {
        uint16 * to = (uint16*)fkb_push(fkb_p, BRCM_TAG_TYPE2_LEN);
        pHdr = (BcmEnet_hdr2 *)to;
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set port of ingress brcm tag */
        pHdr->brcm_tag = port_map;

    }
    /* set ingress brcm tag and TC bit */
    pHdr->brcm_type = BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(fkb_p->mark) << 10);
    tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - fkb_p->len;
    tailroom = (tailroom < 0) ? crc_len : crc_len + tailroom;
    fkb_put(fkb_p, tailroom);
}

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
static struct sk_buff *moca_skb_put_tag(struct sk_buff *skb,
    struct net_device *dev, unsigned int priority)
{
    int                 headroom;
    int                 tailroom;
    struct vlan_ethhdr *pHdr = (struct vlan_ethhdr *)skb->data;
    int                 i;

    /* if the VLAN tag with VID 0xFFF already exists then return */
    if ( (VLAN_TYPE == pHdr->h_vlan_proto) &&
         (0xFFF == (pHdr->h_vlan_TCI & 0xFFF)) )
    {
        return skb;
    }

    headroom = VLAN_HLEN;
    tailroom = ETH_ZLEN - skb->len;
    if (tailroom < 0) {
        tailroom = 0;
    }

    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
    {
        struct sk_buff *oskb = skb;
        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);
        if (!skb)
            return NULL;
    }
    else if (headroom != 0)
    {
        skb = skb_unshare(skb, GFP_ATOMIC);
        if (!skb)
            return NULL;
    }

    if (tailroom > 0) {
        if (skb_is_nonlinear(skb)) {
            /* Non linear skb whose skb->len is < minimum Ethernet Packet Length 
                         (ETHZLEN or ETH_ZLEN + BroadcomMgmtTag Length) */
            if (skb_linearize(skb)) {
                return NULL;
            }
        }
        memset(skb->data + skb->len, 0, tailroom);  /* padding to 0 */
        skb_put(skb, tailroom);
    }

    if (headroom != 0)
    {
        uint16 *to;
        uint16 *from;

        pHdr = (struct vlan_ethhdr *)skb_push(skb, headroom);
        to   = (uint16*)pHdr;
        from = (uint16*)(skb->data + headroom);
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set VLAN protocol, VID and priority*/
        pHdr->h_vlan_proto = VLAN_TYPE;
        pHdr->h_vlan_TCI = 0xFFF | (priority << 13);
    }

    return skb;

}

static inline void moca_fkb_put_tag(FkBuff_t * fkb_p,
    struct net_device * dev, unsigned int priority)
{
    int                 tailroom;
    int                 crc_len = 0;
    uint16             *from    = (uint16*)fkb_p->data;
    uint16             *to;
    struct vlan_ethhdr *pHdr    = (struct vlan_ethhdr *)from;
    int                 i;

    /* if the VLAN tag with VID 0xFFF already exists then return */
    if ( (VLAN_TYPE == pHdr->h_vlan_proto) &&
         (0xFFF == (pHdr->h_vlan_TCI & 0xFFF)) )
    {
        return;
    }

    to   = (uint16 *)fkb_push(fkb_p, VLAN_HLEN);
    pHdr = (struct vlan_ethhdr *)to;
    for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */

    /* set VLAN protocol VID and priority*/
    pHdr->h_vlan_proto = VLAN_TYPE;
    pHdr->h_vlan_TCI   = 0xFFF | (priority << 13);

    tailroom = ETH_ZLEN + VLAN_HLEN - fkb_p->len;
    tailroom = (tailroom < 0) ? crc_len : crc_len + tailroom;
    fkb_put(fkb_p, tailroom);

}



static inline int moca_check_port(moca_port_check_t * moca_port_check,
                                       int queue, int isWan,
                                       unsigned long check_port_stat)
{
    uint16 val16 = MOCA_PORT_ID;
    uint16 curr_count;
    uint16 max_count;
    unsigned long moca_fc_reg;
    int i;

#ifdef MOCA_PORT_CHECK_DEBUG
        moca_port_check->run_count++;

    if (moca_port_check->test == 1)
    {
        printk("%s: Resetting %s switch. \n",
           __FUNCTION__, (isWan ? "WAN" : "LAN") );
        moca_port_check->test = 0;
        reset_switch(isWan);
        return(1);
    }
#endif

    /* First check that no traffic is leaving the port */
    if (check_port_stat != moca_port_check->last_tx_count[queue])
    {
        /* Traffic is still being sent. The port is fine,
         * make sure the counts are clear and return. */
        moca_port_check->last_tx_count[queue] = check_port_stat;
        moca_port_check->match_count[queue] = 0;
        moca_port_check->last_count[queue] = 0;
        return 0;
    }

    /* If the byte count is stagnant, check whether or not the queue
     * is draining properly. Also make sure that the buffer level
     * isn't static because the MoCA firmware is asserting the flow
     * control signal
     */
    if ( !isWan )
        ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (uint8 *)&val16,
            2, isWan);

    ethsw_rreg_ext(PAGE_FLOW_CTRL,
        REG_FC_Q_MON_CNT + ((global.enet_softswitch_xmit_start_q + queue) * 2),
        (uint8 *)&curr_count, 2, isWan);
    ethsw_rreg_ext(PAGE_FLOW_CTRL,
        REG_FC_PEAK_Q_MON_CNT + ((global.enet_softswitch_xmit_start_q + queue) * 2),
         (uint8 *)&max_count, 2, isWan);

    moca_get_fc_bits(isWan, &moca_fc_reg);

    if ((curr_count != 0) &&
        (curr_count == max_count) &&
        (max_count == moca_port_check->last_count[queue]) &&
        (moca_fc_reg == 0))
    {
        moca_port_check->match_count[queue]++;
#ifdef MOCA_PORT_CHECK_DEBUG
        moca_port_check->total_match_count++;
#endif

        if (moca_port_check->match_count[queue] ==
            moca_port_check->threshold)
        {
            /* Read the peak queue count one last time before resetting
             * the switch */
            ethsw_rreg_ext(PAGE_FLOW_CTRL,
                REG_FC_PEAK_Q_MON_CNT + ((global.enet_softswitch_xmit_start_q + queue) * 2),
                (uint8 *)&max_count, 2, isWan);

            if (max_count == moca_port_check->last_count[queue])
            {

                printk("\n\n%s %d: Resetting switch. Q%d Curr %u Max %u\n\n\n",
                       __FUNCTION__, isWan, queue, curr_count, max_count );
#ifdef MOCA_PORT_CHECK_DEBUG
                if (moca_port_check->test != -1)
#endif
                reset_switch(isWan);

                for (i = 0; i < NUM_MOCA_SW_QUEUES; i++) {
                    moca_port_check->match_count[i] = 0;
                    moca_port_check->last_count[i] = 0;
                    moca_port_check->last_tx_count[i] = 0;
                }
#ifdef MOCA_PORT_CHECK_DEBUG
                moca_port_check->reset_count++;
#endif
                return(1);
            }
            else
            {
                moca_port_check->match_count[queue] = 0;
                moca_port_check->last_count[queue] = max_count;
                return 0;
            }
        }
    }
    else
    {
        moca_port_check->match_count[queue] = 0;
    }

    moca_port_check->last_count[queue] = max_count;

    return 0;
}

static inline int moca_queue_packet(moca_queue_t *moca, int queue,
    pNBuff_t pNBuff, struct net_device *dev, EnetXmitParams *pParam)
{
    pNBuff_t pDropBuf = NULL;
    int i;
    int queueThresh = global.moca_queue_depth;
    int totalThresh = MOCA_TOTAL_QUEUED_PACKETS_MAX;

    ENET_MOCA_TX_LOCK();

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if (gbpm_enable_g)
    {
        /* Get the global buffer pool level */
        if (gbpm_get_dyn_buf_level())
        {
            queueThresh = moca->thresh[queue].q_hi_thresh;
        }
        else
        {
            queueThresh = moca->thresh[queue].q_lo_thresh;
            totalThresh = MOCA_TXQ_DEPTH_MAX;
        }
    }
#endif

    /* if this packet exceeds the total allowable count for this queue
       then drop the oldest packet in the queue */
    if ( MOCA_QUEUE_NUM_PACKETS(moca, queue) >= (queueThresh-1) )
    {
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        moca->thresh[queue].q_dropped++;
#endif
        pDropBuf = moca->queue[queue][moca->head[queue]];
        moca->queuedPackets--;
        MOCA_QUEUE_HEAD_INCREEMENT(moca, queue);
        goto unlock_drop_exit;
    }

    /* if this packet gets us to the total allowable count then
       try to drop a lower priority packet or the oldest packet on
       this queue */
    if ( moca->queuedPackets >= totalThresh )
    {
        for ( i=0; i<=queue; i++ )
        {
            if ( MOCA_QUEUE_HAS_A_PACKET(moca, i) )
            {
                pDropBuf = moca->queue[i][moca->head[i]];
                moca->queuedPackets--;
                MOCA_QUEUE_HEAD_INCREEMENT(moca, i);
                break;
            }
        }
        /* did not find a lower priority or older same priority
           buffer to drop so drop current buffer */
        if (pDropBuf == NULL)
           pDropBuf = pNBuff;
    }

unlock_drop_exit:

    /* current buffer should be added to the queue */
    if ( pDropBuf != pNBuff )
    {
       moca->queue[queue][moca->tail[queue]] = pNBuff;
       moca->queuedPackets++;
       MOCA_QUEUE_TAIL_INCREEMENT(moca, queue);
    }

    /* check to see if we need to free a buffer */
    if ( pDropBuf )
    {
        global.pVnetDev0_g->stats.tx_dropped++;
        pParam->vstats->tx_dropped++;
        ENET_MOCA_TX_UNLOCK();
        nbuff_flushfree(pDropBuf);
    }
    else
        ENET_MOCA_TX_UNLOCK();

#if !defined(MOCA_HIGH_RES_TX)
    wake_up_process(enet_softswitch_xmit_task);
#endif

    return 0;
}

static inline int moca_send_packets(int isWan, int sendOne)
{
    struct net_device *pDev0 = vnet_dev[0];
    BcmEnet_devctrl   *pDev0Priv = (BcmEnet_devctrl *)netdev_priv(pDev0);
    EnetXmitParams param = {0};
    int                queue;
    int                avail_bufs;
    uint16             val16;
    uint16             used_bufs;
    pNBuff_t           pNBuff;
    moca_queue_t      *moca;
    moca_port_check_t *moca_port_check;
    struct net_device *dev;
    int                dropFrames = 0;
    u64                curTime = ktime_to_ns(ktime_get());
    int                check_port = 0;
    unsigned long      check_port_stat = 0;

    if (isWan) {
        param.port_id = SERDES_PORT_ID;
        moca = &global.moca_wan;
        moca_port_check = &global.moca_port_check[1];
        dev = bcm6829_to_dev[BCM6829_MOCA_DEV];
        param.pDevPriv = netdev_priv(dev);
        param.vstats = &param.pDevPriv->stats;
    } else {
        param.port_id = MOCA_PORT_ID;
        moca = &global.moca_lan;
        moca_port_check = &global.moca_port_check[0];
        dev = vnet_dev[phyport_to_vport[MOCA_PORT_ID]];
        param.pDevPriv = netdev_priv(dev);
        param.vstats = &param.pDevPriv->stats;
    }

    ENET_MOCA_TX_LOCK();
    if ( (curTime - moca->lastWindowTime) >= 250000 )
    {
        moca->packetCount = 0;
        moca->lastWindowTime = curTime;
    }
    else if ( moca->packetCount >= global.moca_xmit_budget)
    {
        ENET_MOCA_TX_UNLOCK();
        return 0;
    }

    /* drop frames if link is down */
    if ( 0 == (pDev0Priv->linkState & moca->mask))
        dropFrames = 1;

    if (moca_port_check->enable &&
        ((curTime - moca_port_check->last_check_time) >=
         moca_port_check->interval_ns))
    {
        moca_port_check->last_check_time = curTime;
        /* Use check_port as a bitfield to only check each queue once */
        check_port = (1 << NUM_MOCA_SW_QUEUES) - 1;

        /* Just do one read of the octet count per moca interface instead
         * of reading the count once per queue. */
        ethsw_rreg_ext(PAGE_MIB_P0 + MOCA_PORT_ID, REG_MIB_P0_TXOCTETS,
            (uint8 *)&check_port_stat, sizeof(check_port_stat), isWan);
    }

    for (queue = NUM_MOCA_SW_QUEUES-1; queue >= 0; queue--)
    {
        while (TRUE)
        {
            if (MOCA_QUEUE_HAS_A_PACKET(moca, queue))
            {
                if (check_port & (1 << queue))
                {
                    if (moca_check_port(moca_port_check, queue, isWan, check_port_stat))
                    {
                        ENET_MOCA_TX_UNLOCK();
                        return 0;
                    }
                    else
                    {
                        check_port &= ~(1 << queue);
                    }
                }

                BCM_ENET_TX_DEBUG("MoCA queue %d has a packet \n", queue);
                pNBuff = moca->queue[queue][moca->head[queue]];
                if ((1 == dropFrames) ||
                    (nbuff_get_params_ext(pNBuff, &param.data, &param.len, &param.mark,
                                          &param.priority, &param.r_flags) == NULL) )
                {
                    moca->queuedPackets--;
                    MOCA_QUEUE_HEAD_INCREEMENT(moca, queue);
                    ENET_MOCA_TX_UNLOCK();
                    BCM_ENET_TX_DEBUG("continue if there is no valid buff \n");
                    nbuff_flushfree(pNBuff);
                    ENET_MOCA_TX_LOCK();
                    continue;
                }

                if ( (sendOne) &&
                     (IS_SKBUFF_PTR(pNBuff)) )
                {
                    struct sk_buff * skb_p;

                    skb_p = PNBUFF_2_SKBUFF(pNBuff);
                    if ( skb_p->blog_p )
                    {
                        /* this path can lead to locking issues
                           return and schedule the MoCA tasklet to
                           send the next packet */
                        ENET_MOCA_TX_UNLOCK();
                        tasklet_schedule(&mocaTasklet);
                        return 0;
                    }
                }

                /* buffers are 256 bytes each */
                used_bufs   = ((param.len/256) + ((param.len % 256)?1:0));
                /* calculate buffer count after sending this packet */
                avail_bufs  = moca->egressq_alloc_bufs - moca->usedBuffers[queue] - used_bufs;
                BCM_ENET_TX_DEBUG("Available 256B buffers for the queue %d = %d \n",
                                  queue, (unsigned int)avail_bufs);

                if (avail_bufs >= 0)
                {
                    int leave = 0;

                    BCM_ENET_TX_DEBUG("Transmitting a packet on MoCA \n");
                    moca->queuedPackets--;
                    MOCA_QUEUE_HEAD_INCREEMENT(moca, queue);
                    moca->usedBuffers[queue] += used_bufs;
                    if ( ++moca->packetCount >= global.moca_xmit_budget )
                    {
                        leave = 1;
                    }
                    param.egress_queue = queue;
                    param.channel = SKBMARK_GET_Q_CHANNEL(param.mark) % cur_txdma_channels;

                    /* add VLAN tag for MoCA WAN interface to preserve switch prioprity mapping */
                    if ( 1 == isWan )
                    {
                       if ( IS_FKBUFF_PTR(pNBuff) )
                       {
                           FkBuff_t * fkb_p = PNBUFF_2_FKBUFF(pNBuff);
                           moca_fkb_put_tag(fkb_p, dev, (global.enet_softswitch_xmit_start_q + queue));
                           param.data = fkb_p->data;
                           param.len  = fkb_p->len;
                       }
                       else
                       {
                           struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
                           skb = moca_skb_put_tag(skb, dev, (global.enet_softswitch_xmit_start_q + queue));
                           if (skb == NULL)
                           {
                               ENET_MOCA_TX_UNLOCK();
                               nbuff_flushfree(pNBuff);
                               ENET_MOCA_TX_LOCK();
                               continue;
                           }
                           param.data = skb->data;
                           param.len  = skb->len;
                           pNBuff = PBUF_2_PNBUFF((void*)skb, SKBUFF_PTR);
                       }
                    }

                    ENET_MOCA_TX_UNLOCK();
                    bcm63xx_enet_xmit2(pNBuff, dev, &param);
                    if ( leave || sendOne )
                    {
                        return 0;
                    }
                    ENET_MOCA_TX_LOCK();
                }
                else
                {
                    val16 = MOCA_PORT_ID;

                    if ( !isWan ) {
                         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (uint8 *)&val16, 2, isWan);
                    }
                    ethsw_rreg_ext(PAGE_FLOW_CTRL, REG_FC_Q_MON_CNT + ((global.enet_softswitch_xmit_start_q + queue) * 2),
                                   (uint8 *)&moca->usedBuffers[queue], 2, isWan);

                    avail_bufs  = moca->egressq_alloc_bufs - moca->usedBuffers[queue] - used_bufs;
                    if ( avail_bufs < 0 )
                    {
                       break; /* process next queue */
                    }
                }
            }
            else
            {
                BCM_ENET_TX_DEBUG("No packets in MoCA sw queue %d \n", queue);
                break;
            }
        }
    }
    ENET_MOCA_TX_UNLOCK();
    return 0;
}
#endif
#endif

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))

#define get_first_gemid_to_ifIdx_mapping(gemId) gem_to_gponifid[gemId][0]

#if defined(DEFINE_ME_TO_USE)
static void print_gemid_to_ifIdx_map(const char *pMark, int gemId)
{
    int ifIdx;
    unsigned int map1 = 0;
    unsigned int map2 = 0;

    for (ifIdx = 0; ifIdx < MAX_GPON_IFS_PER_GEM; ++ifIdx)
    {
        if (gem_to_gponifid[gemId][ifIdx] != UNASSIGED_IFIDX_VALUE)
        { /* Occupied */
            if (ifIdx > 31)
            {
                map2 |= (1<< (ifIdx-32));
            }
            else
            {
                map1 |= (1<<ifIdx);
            }
        }
    }
    if (map1 || map2)
    {
        printk("%s : GEM = %02d Map1 = 0x%08X Map2 = 0x%08X\n",pMark,gemId,map1,map2);
    }
}

static int get_next_gemid_to_ifIdx_mapping(int gemId, int ifId)
{
    int ifIdx;
    if (ifId == UNASSIGED_IFIDX_VALUE)
    { /* get first */
        return get_first_gemid_to_ifIdx_mapping(gemId);
    }
    for (ifIdx = 0; ifIdx < MAX_GPON_IFS_PER_GEM; ++ifIdx)
    {
        if (gem_to_gponifid[gemId][ifIdx] == UNASSIGED_IFIDX_VALUE)
        {
            break;
        }
        if ((ifIdx+1 < MAX_GPON_IFS_PER_GEM) &&
            (gem_to_gponifid[gemId][ifIdx] == ifId))
        {
            return gem_to_gponifid[gemId][ifIdx+1];
        }
    }
    return UNASSIGED_IFIDX_VALUE;
}
#endif /* DEFINE_ME_TO_USE */

static void initialize_gemid_to_ifIdx_mapping(void)
{
    int gemId,ifIdx;
    for (gemId = 0; gemId < MAX_GEM_IDS; ++gemId)
    {
        for (ifIdx = 0; ifIdx < MAX_GPON_IFS_PER_GEM; ++ifIdx)
        {
            gem_to_gponifid[gemId][ifIdx] = UNASSIGED_IFIDX_VALUE;
        }
    }
}
static int add_gemid_to_gponif_mapping(int gemId, int ifId)
{
    int ifIdx;
    for (ifIdx = 0; ifIdx < MAX_GPON_IFS_PER_GEM; ++ifIdx)
    {
        if (gem_to_gponifid[gemId][ifIdx] == UNASSIGED_IFIDX_VALUE)
        { /* Empty */
            gem_to_gponifid[gemId][ifIdx] = ifId;
            return 0;
        }
    }
    printk("Out of resources !! No more ifs available for gem<%d>\n",gemId);
    return -1;
}
static void remove_gemid_to_gponif_mapping(int gemId, int ifId)
{
    int idx;
    int usedIdx;
    int moveIdx;
    for (idx = 0; idx < MAX_GPON_IFS_PER_GEM; ++idx)
    {
        if (gem_to_gponifid[gemId][idx] != UNASSIGED_IFIDX_VALUE)
        { /* Occupied */
            if (gem_to_gponifid[gemId][idx] == ifId)
            {
                /* Remove the mapping */
                gem_to_gponifid[gemId][idx] = UNASSIGED_IFIDX_VALUE;
                moveIdx = idx;
                for (usedIdx = idx+1; usedIdx < MAX_GPON_IFS_PER_GEM; ++usedIdx)
                {
                    if (gem_to_gponifid[gemId][usedIdx] != UNASSIGED_IFIDX_VALUE)
                    {
                        gem_to_gponifid[gemId][moveIdx] = gem_to_gponifid[gemId][usedIdx];
                        gem_to_gponifid[gemId][usedIdx] = UNASSIGED_IFIDX_VALUE;
                        moveIdx = usedIdx;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
}
static BOOL is_gemid_mapped_to_gponif(int gemId, int ifId)
{
    int ifIdx;
    for (ifIdx = 0; ifIdx < MAX_GPON_IFS_PER_GEM; ++ifIdx)
    {
        if (gem_to_gponifid[gemId][ifIdx] == UNASSIGED_IFIDX_VALUE)
        {
            break;
        }
        if (gem_to_gponifid[gemId][ifIdx] == ifId)
        {
            return TRUE;
        }
    }
    return FALSE;
}
#endif /* defined(CONFIG_BCM96816) && defined(DBL_DESC) */

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC) && defined(SUPPORT_HELLO))
static int dropped_xmit_port = 0;
/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_xmit_port
 Purpose: Send ethernet traffic given a port number
-------------------------------------------------------------------------- */
static int bcm63xx_enet_xmit_port(struct sk_buff * skb, int xPort)
{
    struct net_device * dev;
    BcmEnet_devctrl   *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);

    BCM_ENET_DEBUG("Send<0x%08x> len=%d xport<%d> "
                  "mark<0x%08x> destQ<%d> gemId<%d> channel<%d>\n",
                  (int)skb, skb->len, xPort, skb->mark,
                  SKBMARK_GET_Q_PRIO(skb->mark), SKBMARK_GET_PORT(skb->mark),
                  SKBMARK_GET_Q_CHANNEL(skb->mark));

     //DUMP_PKT(skb->data, skb->len);

    if (xPort == GPON_PORT_ID) {
        int gemid, gponifid;
        gemid = SKBMARK_GET_PORT(skb->mark);
        gponifid = get_first_gemid_to_ifIdx_mapping(gemid);
        BCM_ENET_INFO("gponifid<%d> \n ", gponifid);
        if ( gponifid < 0 )
        {
            BCM_ENET_INFO("-ve gponifid, dropping pkt \n");
            dropped_xmit_port++;
            return -1;
        }
        dev = gponifid_to_dev[gponifid];
        if ( dev == (struct net_device*)NULL )
        {
            dropped_xmit_port++;
            return -1;
        }
        BCM_ENET_INFO("dev<0x%08x:%s>\n", (int)dev, dev->name);
    }
    else if ((SERDES_PORT_ID == xPort) &&
             IsExt6829(pVnetDev0->EnetInfo[0].sw.phy_id[xPort]) )
    {
        int idx = atomic_read(&bcm6829ActDevIdx);
        dev     = bcm6829_to_dev[idx];
    }
    else
    {
        dev = vnet_dev[ phyport_to_vport[xPort] ];
    }

    return bcm63xx_enet_xmit( SKBUFF_2_PNBUFF(skb), dev );
}
#endif  /* defined(SUPPORT_HELLO) */

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_xmit
 Purpose: Send ethernet traffic
-------------------------------------------------------------------------- */
static int bcm63xx_enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    EnetXmitParams   param = {0};

    param.pDevPriv = netdev_priv(dev);
    param.vstats   = &param.pDevPriv->stats;
    param.port_id  = port_id_from_dev(dev);
    BCM_ENET_TX_DEBUG("The physical port_id is %d\n", param.port_id);

    if (nbuff_get_params_ext(pNBuff, &param.data, &param.len,
                             &param.mark, &param.priority, &param.r_flags) == NULL)
        return 0;

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    if ( dev->base_addr == MOCA_PORT_ID )
    {
        if (global.pVnetDev0_g->softSwitchingMap & (1<<param.port_id))
        {
             int            queue = SKBMARK_GET_Q_PRIO(param.mark);
             moca_queue_t * mocaQueue;
             int            isWan;

             if (param.port_id == SERDES_PORT_ID)
             {
                 mocaQueue = &global.moca_wan;
                 isWan = 1;
             }
             else
             {
                 mocaQueue = &global.moca_lan;
                 isWan = 0;
             }

             if (queue > 3)
                 queue = 2;

             BCM_ENET_TX_DEBUG("Queueing packet to %d software queue \n", dev->name);
             moca_queue_packet(mocaQueue, queue, pNBuff, dev, &param);
             return moca_send_packets(isWan, 1);
        }
    }

    /* if there are queued packets for MoCA then schedule the tasklet to run */
    if ( (global.moca_lan.queuedPackets) ||
         (global.Is6829 && (global.moca_wan.queuedPackets)) )
    {
        /* only schedule on CPU 0 */
        if ( 0 == smp_processor_id() )
           tasklet_schedule(&mocaTasklet);
    }
#endif
#endif

    if (global.dump_enable)
        DUMP_PKT(param.data, param.len);

#ifdef USE_DEFAULT_EGRESS_QUEUE
    if (param.pDevPriv->use_default_txq) {
        param.egress_queue = param.pDevPriv->default_txq;
        BCM_ENET_TX_DEBUG("Using default egress queue %d \n", param.egress_queue);
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        if (use_tx_dma_channel_for_priority) {
            param.channel = channel_for_queue[param.egress_queue] % cur_txdma_channels;
        } else {
            param.channel = SKBMARK_GET_Q_CHANNEL(param.mark) % cur_txdma_channels;
        }
#endif
    } else
#endif
    {
        BCM_ENET_TX_DEBUG("Using mark for channel and queue \n");
        param.egress_queue = SKBMARK_GET_Q_PRIO(param.mark);
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        param.channel = SKBMARK_GET_Q_CHANNEL(param.mark) % cur_txdma_channels;
#endif
    }

    BCM_ENET_TX_DEBUG("The Tx channel is %d \n", param.channel);
    BCM_ENET_TX_DEBUG("The egress queue is %d \n", param.egress_queue);

    return bcm63xx_enet_xmit2(pNBuff, dev, &param);
}

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
static inline int fapTxChannelFkb(FkBuff_t *pFkb)
{
    int txChannel;

#if defined(CONFIG_BCM963268)
    FkBuff_t *pFkbMaster = _get_fkb_master_ptr_(pFkb);

    if(pFkbMaster->recycle_hook == (RecycleFuncP)bcm63xx_enet_recycle)
    {
        /* from ETH (from LAN or WAN) */
        /* Always transmit an ethernet packet back to the same iuDMA channel it was
           received from. This is needed to avoid buffer recycling across FAPs */
        txChannel = FKB_RECYCLE_CONTEXT(pFkb)->channel;
    }
    else if(pFkbMaster->recycle_hook == xtm_fkb_recycle_hook)
    {
        /* from XTM (from WAN) */
        txChannel = PKTDMA_ETH_DS_IUDMA;
    }
    else
    {
        /* unknown source, e.g. USB (from LAN) */
        txChannel = PKTDMA_ETH_US_IUDMA;
    }

#elif defined(CONFIG_BCM96362)

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    /* Transmit to the Host owned iuDMA channel */
    txChannel = global.pVnetDev0_g->enetTxChannel;
#else
    /* Transmit to the FAP owned iuDMA channel */
    txChannel = PKTDMA_ETH_TX_FAP0_IUDMA;
#endif

#endif

    return txChannel;
}

static inline int fapTxChannelSkb(struct sk_buff *skb)
{
    int txChannel;

#if defined(CONFIG_BCM963268)

    if(skb->recycle_hook == (RecycleFuncP)bcm63xx_enet_recycle_skb_or_data)
    {
        /* from ETH (from LAN or WAN) */
        /* Always transmit an ethernet packet back to the same iuDMA channel it was
           received from. This is needed to avoid buffer recycling across FAPs */
        txChannel = RECYCLE_CONTEXT(skb->recycle_context)->channel;
    }
    else if(skb->recycle_hook == xtm_skb_recycle_hook)
    {
        /* from XTM (from WAN) */
        txChannel = PKTDMA_ETH_DS_IUDMA;
    }
    else
    {
        /* unknown source, e.g. USB (from LAN) */
        txChannel = PKTDMA_ETH_US_IUDMA;
    }

#elif defined(CONFIG_BCM96362)

    /* Transmit SKBs to the FAP owned iuDMA channel */
    txChannel = PKTDMA_ETH_TX_FAP0_IUDMA;

#endif

    return txChannel;
}
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */

int bcm63xx_enet_isExtSwPresent(void)
{
    return extSwInfo.present;
}

unsigned int bcm63xx_enet_extSwId(void)
{
    return extSwInfo.switch_id;
}


static inline int bcm63xx_enet_xmit2(pNBuff_t pNBuff,
    struct net_device *dev, EnetXmitParams *pParam)
{
#if defined(AEI_VDSL_TOOLBOX)
    BcmEnet_devctrl *pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
#endif
    uint32_t blog_chnl, blog_phy;       /* used if CONFIG_BLOG enabled */
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    int gemid = -1;
#endif
    uint32 reclaim_idx;
    uint32 delayed_reclaim_array[DELAYED_RECLAIM_ARRAY_LEN];
    BcmPktDma_EthTxDma *txdma;
    unsigned int port_map = (1 << pParam->port_id);
    DmaDesc  dmaDesc;
    FkBuff_t *pFkb = NULL;
    struct sk_buff *skb = NULL;

    /* tx request should never be on the bcmsw interface */
    BCM_ASSERT_R((dev != vnet_dev[0]), 0);

    if(IS_FKBUFF_PTR(pNBuff))
    {
        pFkb = PNBUFF_2_FKBUFF(pNBuff);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        pParam->channel = fapTxChannelFkb(pFkb);
#endif
    }
    else
    {
        skb = PNBUFF_2_SKBUFF(pNBuff);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        pParam->channel = fapTxChannelSkb(skb);
#endif
    }

#if defined(AEI_VDSL_TOOLBOX)
    if (pEthCtrl->usMirrorOutFlags != 0)
    {
        struct sk_buff *skb;

        skb = PNBUFF_2_SKBUFF(pNBuff);
        if (skb && (skb->protocol != 0))
        {
            AEI_MultiMirrorPacket( skb, pEthCtrl->usMirrorOutFlags, FALSE);
        }
    }
#endif

    txdma = global.pVnetDev0_g->txdma[pParam->channel];

    BCM_ENET_TX_DEBUG("Send len=%d \n", pParam->len);

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
    /* If GPON port, get the gemid from mark and put it in the BD */
    if (pParam->port_id == GPON_PORT_ID) {
        switch (pParam->pDevPriv->gem_count) {
            case 0:
                BCM_ENET_TX_DEBUG("No gem_ids, so dropping Tx \n");
                goto drop_exit;

            case 1:
                BCM_ENET_TX_DEBUG("Using the only gem_id\n");
                gemid = default_gemid[pParam->pDevPriv->gponifid];
                break;

            default:
                BCM_ENET_TX_DEBUG("2 or more gem_ids, get gem_id from mark\n");
                gemid = SKBMARK_GET_PORT(pParam->mark);
                if (get_first_gemid_to_ifIdx_mapping(gemid) != pParam->pDevPriv->gponifid) {
                    BCM_ENET_TX_DEBUG("The given gem_id is not associated with"
                                     " the given interface\n");
                    goto drop_exit;
                }
                break;
        }
        BCM_ENET_TX_DEBUG("port_id<%d> gem_count<%d> gemid<%d>\n", pParam->port_id,
                          pParam->pDevPriv->gem_count, gemid );
        /* gemid is set later in bcmPktDma implementation of enet driver */
        blog_chnl = gemid;
        blog_phy  = BLOG_GPONPHY;
    }
    else    /* ! GPON_PORT_ID */
#endif
#if defined(CONFIG_BCM96816)
    if ( dev->base_addr == MOCA_PORT_ID )
    {
        blog_chnl = pParam->port_id;
        blog_phy = BLOG_MOCAPHY;/* blog rx phy type is moca */
    } else
#endif
    {
        blog_chnl = pParam->port_id;
        blog_phy  = BLOG_ENETPHY;
    }

    txdma = global.pVnetDev0_g->txdma[pParam->channel];

#ifdef CONFIG_BLOG
    /*
     * Pass to blog->fcache, so it can construct the customized
     * fcache based execution stack.
     */
    blog_emit( pNBuff, dev, TYPE_ETH, blog_chnl, blog_phy ); /* CONFIG_BLOG */
#endif

    ENET_TX_LOCK();

    {
        BcmPktDma_txRecycle_t txRecycle;
        BcmPktDma_txRecycle_t *txRecycle_p;

        reclaim_idx = 0;

        while((txRecycle_p = bcmPktDma_EthFreeXmitBufGet(txdma, &txRecycle)) != NULL)
        {
            delayed_reclaim_array[reclaim_idx] = txRecycle_p->key;

            reclaim_idx++;
            /*
             * only unlock and do reclaim if we have collected many free
             * buffers, otherwise, wait until end of function when we have
             * already released the tx lock to do reclaim.
             */
            if (reclaim_idx >= DELAYED_RECLAIM_ARRAY_LEN) {
                ENET_TX_UNLOCK();
                DO_DELAYED_RECLAIM();
                reclaim_idx = 0;
                ENET_TX_LOCK();
            }
        }   /* end while(...) */
    }

    {
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        uint32_t q;
        uint32_t txDropThr;
        uint32_t qDepth;

        if (g_Eth_tx_iudma_ownership[pParam->channel] == HOST_OWNED)
        {
            q = (pParam->egress_queue > 3) ? 3 : pParam->egress_queue;
            txDropThr = txdma->txDropThr[q];
            qDepth = bcmPktDma_EthXmitBufCountGet_Iudma(txdma);
        }
#endif
#endif

        if ( 
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
            ( (g_Eth_tx_iudma_ownership[pParam->channel] == HOST_OWNED) &&
              (qDepth >= txDropThr) ) ||
#endif
#endif
            /* Check for tx slots available AFTER re-acquiring the tx lock */
            (!bcmPktDma_EthXmitAvailable(txdma)))
        {
#if defined(ENABLE_FAP_COMMS_DEBUG)
            /* Count # times eth tx send to FAP is rejected - May 2010 */
            pHostPsmGbl->debug_ctrs[ENET_TX_HOST_2_FAP_REJECT]++;
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
            txdma->txDropThrPkts[q]++;
#endif
#endif

            TRACE(("%s: bcm63xx_enet_xmit low on txFreeBds\n", dev->name));
            BCM_ENET_TX_DEBUG("No more Tx Free BDs\n");
            global.pVnetDev0_g->stats.tx_dropped++;
            pParam->vstats->tx_dropped++;

#if defined(AEI_VDSL_STATS_DIAG)
            if (pParam->data)
            {
                if (*(pParam->data) & 1)
                    pEthCtrl->dev_stats.multicast_discarded_packets++;
                else
                    pEthCtrl->dev_stats.unicast_discarded_packets++;
            }
#endif

            goto unlock_drop_exit;
        }
    }

#if defined(CONFIG_BCM96816)
    if (blog_phy == BLOG_GPONPHY && gemMirrorCfg[MIRROR_DIR_OUT].nStatus &&
        (gemMirrorCfg[MIRROR_DIR_OUT].nGemPortMask & (1 << blog_chnl)))
    {
        struct sk_buff * pNBuffSkb; /* If pNBuff is sk_buff: protocol access */
        pNBuffSkb = nbuff_xlate(pNBuff);    /* translate to skb */
        if (pNBuffSkb != (struct sk_buff *)NULL)
        {
            MirrorPacket( pNBuffSkb, gemMirrorCfg[MIRROR_DIR_OUT].szMirrorInterface, 0);
            pNBuff = SKBUFF_2_PNBUFF( pNBuffSkb );
            nbuff_get_context( pNBuff, &pParam->data, &pParam->len );
        }
    }
#endif

    if (global.pVnetDev0_g->extSwitch->present
#if defined(CONFIG_BCM963268)
        && (pParam->port_id < MAX_EXT_SWITCH_PORTS)
#endif
       ) 
    {
        if ( pFkb ) {
            FkBuff_t * pFkbOrig = pFkb;
            pFkb = fkb_unshare(pFkbOrig);

            if (pFkb == FKB_NULL)
            {
                fkb_free(pFkbOrig);
                goto unlock_exit;
            }
            bcm63xx_fkb_put_tag(pFkb, dev, port_map);
            pParam->data = pFkb->data;
            pParam->len  = pFkb->len;
            pNBuff = PBUF_2_PNBUFF((void*)pFkb,FKBUFF_PTR);
        } else {
            skb = bcm63xx_skb_put_tag(skb, dev, port_map);    /* also pads to 0 */
            if (skb == NULL) {
                goto unlock_exit;
            }
            pParam->data = skb->data;   /* Re-encode pNBuff for adjusted data and len */
            pParam->len  = skb->len;
            pNBuff = PBUF_2_PNBUFF((void*)skb,SKBUFF_PTR);
        }
#if defined(CONFIG_BCM963268)
        dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC | DMA_APPEND_BRCM_TAG | (1 << global.pVnetDev0_g->extSwitch->connected_to_internalPort);
#else
        dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC;
#endif
    } else {

        /* DMA priority is 2 bits - range 0-3, if the egress queue is greater than 3
           then use DMA priority 3 to transmit the packet */
        int dmaPriority = pParam->egress_queue;
#if defined(CONFIG_BCM963268)
       /* When external switch is present and when we are xmitting on an internal sw port */
       if (pParam->port_id >= MAX_EXT_SWITCH_PORTS)
       {
           port_map = port_map >> MAX_EXT_SWITCH_PORTS;
       }
#endif
        if (dmaPriority > 3)
            dmaPriority = 3;
        dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC |
                         DMA_APPEND_BRCM_TAG | (port_map) |
                         ((dmaPriority << 10) & DMA_PRIO);
    }

    if ( pParam->len < ETH_ZLEN )
        pParam->len = ETH_ZLEN;

#if defined(CONFIG_BCM96368)
    if(kerSysGetSdramWidth() == MEMC_16BIT_BUS)
    {
        pNBuff = nbuff_align_data(pNBuff, &pParam->data, pParam->len, NBUFF_ALIGN_MASK_8);
        if(pNBuff == NULL)
        {
            global.pVnetDev0_g->stats.tx_dropped++;
            pParam->vstats->tx_dropped++;
            goto unlock_exit; 
        }
    }
#endif

    {
        int bufSource;
        uint32 key;
        int param1;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        bufSource = HOST_VIA_DQM;
        key = (uint32)pNBuff;
        param1 = 0;

        if(pFkb)
        {
            uint8  rxChannel = FKB_RECYCLE_CONTEXT(pFkb)->channel;
            FkBuff_t *pFkbMaster = _get_fkb_master_ptr_(pFkb);

            if(pFkbMaster->recycle_hook == (RecycleFuncP)bcm63xx_enet_recycle)
            {
                /* received from FAP */
                if(_get_master_users_(pFkbMaster) == 1)
                {
                    /* allow local recycling in FAP */
                    bufSource = FAP_ETH_RX;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
                    if(txdma->txOwnership != HOST_OWNED)
#endif
                    {
                        key = (uint32)PFKBUFF_TO_PDATA(pFkb, RX_ENET_SKB_HEADROOM);
                    }
                }
                /*
                 * else, the FKB has been cloned so we need to send the nbuff back
                 * to the Host for recycling:
                 * bufSource = HOST_VIA_DQM;
                 * key = (uint32)pNBuff;
                 */

            param1 = rxChannel;
            }
            else if(pFkbMaster->recycle_hook == xtm_fkb_recycle_hook)
            {
                /* received from FAP */
                if(_get_master_users_(pFkbMaster) == 1)
                {
                    bufSource = FAP_XTM_RX;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
                    if(txdma->txOwnership != HOST_OWNED)
#endif
                    {
                        key = (uint32)PFKBUFF_TO_PDATA(pFkb, RXBUF_HEAD_RESERVE);
                    }
                }
                /*
                 * else, the FKB has been cloned so we need to send the nbuff back
                 * to the Host for recycling:
                 * bufSource = HOST_VIA_DQM;
                 * key = (uint32)pNBuff;
                 */

                param1 = rxChannel;
            }
        }
#if defined(CC_FAP4KE_PKT_GSO)
        else /* skb */
        {
            if(skb->ip_summed == CHECKSUM_PARTIAL)
            {
                bufSource = HOST_VIA_DQM_CSUM;
            }

            if(skb_is_gso(skb))
            {
                bufSource = HOST_VIA_DQM_GSO;
                /* pass TCP MSS via param1 */
                param1 = skb_shinfo(skb)->gso_size;

                if(skb_shinfo(skb)->nr_frags)
                {
                    /* SKB is fragmented */

#if defined(CC_FAP4KE_PKT_GSO_FRAG)
                    if(skb_shinfo(skb)->nr_frags == 1)
                    {
                        skb_frag_t *frag = &skb_shinfo(skb)->frags[0];
                        uint8 *vaddr = kmap_skb_frag(frag);
                        uint8 *payload_p = vaddr + frag->page_offset;
                        int payloadLen = frag->size;
                        int headerLen = pParam->len - payloadLen;


                        /* Transmit Payload (fragment) */
                        cache_flush_len(payload_p, payloadLen);
                        bcmPktDma_EthXmit(txdma, payload_p,
                                          payloadLen, HOST_VIA_DQM_FRAG, 0, 0, 0);

                        /* wait until space is avaialable, we cannot fail here */
                        while(!(bcmPktDma_EthXmitAvailable(txdma)));

                        /* Transmit Header */
                        cache_flush_len(pParam->data, headerLen);
                        bcmPktDma_EthXmit(txdma, pParam->data, pParam->len,
                                          bufSource, dmaDesc.status, (uint32)pNBuff, param1);

                        kunmap_skb_frag(vaddr);
                        goto tx_continue;
                    }
                    else
#endif /* CC_FAP4KE_PKT_GSO_FRAG */
                    {
                        if(__skb_linearize(skb))
                        {
                            goto unlock_drop_exit;
                        }

                        pParam->data = skb->data;
                        pParam->len  = skb->len;
                    }
                }
            }
            else
            {
                /* Not GSO */
                if(skb_shinfo(skb)->nr_frags)
                {
                    /* SKB is fragmented */
#if defined(CC_FAP4KE_PKT_GSO_FRAG)
                    bufSource = HOST_VIA_DQM_GSO;
                    /* pass MSS via param1
                     * set to 0, FAP will calculate based on headers len*/
                    param1 = 0;

                    if(skb_shinfo(skb)->nr_frags == 1)
                    {
                        skb_frag_t *frag = &skb_shinfo(skb)->frags[0];
                        uint8 *vaddr = kmap_skb_frag(frag);
                        uint8 *payload_p = vaddr + frag->page_offset;
                        int payloadLen = frag->size;
                        int headerLen;

                        pParam->len  = skb->len;
                        headerLen = pParam->len - payloadLen;


                        /* Transmit Payload (fragment) */
                        cache_flush_len(payload_p, payloadLen);
                        bcmPktDma_EthXmit(txdma, payload_p,
                                          payloadLen, HOST_VIA_DQM_FRAG, 0, 0, 0);

                        /* wait until space is avaialable, we cannot fail here */
                        while(!(bcmPktDma_EthXmitAvailable(txdma)));

                        /* Transmit Header */
                        cache_flush_len(pParam->data, headerLen);
                        bcmPktDma_EthXmit(txdma, pParam->data, pParam->len,
                                          bufSource, dmaDesc.status, (uint32)pNBuff, param1);

                        kunmap_skb_frag(vaddr);
                        goto tx_continue;
                    }
                    else
#endif /* CC_FAP4KE_PKT_GSO_FRAG */
                    {

                        if(__skb_linearize(skb))
                        {
                            goto unlock_drop_exit;
                        }

                        pParam->data = skb->data;
                        pParam->len  = skb->len;
                    }
                }
            }
        }
#endif /* CC_FAP4KE_PKT_GSO */

#else /* CONFIG_BCM_FAP */

        /* FAP is compiled out */
        bufSource = HOST_VIA_LINUX;
        key = (uint32)pNBuff;
        param1 = gemid;

#endif /* CONFIG_BCM_FAP */

        nbuff_flush(pNBuff, pParam->data, pParam->len);

        bcmPktDma_EthXmit(txdma,
                          pParam->data, pParam->len, bufSource,
                          dmaDesc.status, key, param1);
    }
#if defined(CC_FAP4KE_PKT_GSO_FRAG)
tx_continue:
#endif

    /* update stats */
    pParam->vstats->tx_bytes += pParam->len + ETH_CRC_LEN;
    pParam->vstats->tx_packets++;

    global.pVnetDev0_g->stats.tx_bytes += pParam->len + ETH_CRC_LEN;
    global.pVnetDev0_g->stats.tx_packets++;
    global.pVnetDev0_g->dev->trans_start = jiffies;

#if defined(AEI_VDSL_STATS_DIAG)
    /* because of some limitations, it's not able to support 
     * multicast rx/tx bytes counters in the hardware, except this,
     * all other counters will be directly gotten from the hardware */
    if (pParam->data && (*(pParam->data) & 1))
    {
        pEthCtrl->dev_stats.tx_multicast_bytes += pParam->len + ETH_CRC_LEN;
    }
#endif

#ifdef DEBUG_COUNTERS
    txdma->localstats.tx_pkts++;
#endif

unlock_exit:
    ENET_TX_UNLOCK();
    DO_DELAYED_RECLAIM();
    return 0;

unlock_drop_exit:
    ENET_TX_UNLOCK();
    DO_DELAYED_RECLAIM();
    nbuff_flushfree(pNBuff);
    return 0;

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
drop_exit:
    nbuff_flushfree(pNBuff);
    return 0;
#endif
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
/* New version of kernel uses the 'new api' or NAPI. */
static int bcm63xx_enet_poll_napi(struct napi_struct *napi, int budget)
{
    struct BcmEnet_devctrl *pDevCtrl = container_of(napi, struct BcmEnet_devctrl, napi);

    uint32 work_done;
    uint32 ret_done, channel;
    BcmEnet_RxDma *rxdma;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    if (iqos_enable_g)
    {
        /* update the CPU congestion status
         * FAP    : use the DQM queue length
         * non-FAP: use the RX DMA ring length
         */
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        enet_iq_dqm_update_cong_status(pDevCtrl);
#endif
        enet_iq_update_cong_status(pDevCtrl);
    }
#endif

    work_done = bcm63xx_rx(pDevCtrl, budget);
    ret_done = work_done & ENET_POLL_DONE;
    work_done &= ~ENET_POLL_DONE;

    BCM_ENET_DEBUG("Work Done: %d \n", (int)work_done);

    if (work_done == budget || ret_done != ENET_POLL_DONE)
    {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        ENET_RX_LOCK();

        /* Add check for 1 iuDMA channel limiting others by hitting the no BDs condition - Sept 2010 */
        for (channel = 0; channel < cur_rxdma_channels; channel++) {

            rxdma = pDevCtrl->rxdma[channel];

        }

        ENET_RX_UNLOCK();
#endif

        /* We have either exhausted our budget or there are
           more packets on the DMA (or both).  Simply
          return, and the framework will reschedule
          this function automatically */
        return work_done;
    }

    /* we are done processing packets */

    napi_complete(napi);

    /* Enable the interrupts from all RX DMA channels */
    ENET_RX_LOCK();
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
        if (rxchannel_isr_enable[channel])
#endif
        {
            bcmPktDma_BcmHalInterruptEnable(channel, rxdma->rxIrq);
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
            // This is needed in the FAP version of the driver to keep it from locking up
            bcmPktDma_EthClrRxIrq(&rxdma->pktDmaRxInfo);
#endif
        }
    }
    ENET_RX_UNLOCK();

    return work_done;
}

#else // LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
/* NOTE: any changes made to this version of the polling function
   must also be made to bcm63xx_enet_poll_napi */
static int bcm63xx_enet_poll(struct net_device * dev, int * budget)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    uint32 work_to_do = min(dev->quota, *budget);
    uint32 work_done;
    uint32 ret_done, channel;
    BcmEnet_RxDma *rxdma;

    work_done = bcm63xx_rx(pDevCtrl, work_to_do);

    ret_done = work_done & ENET_POLL_DONE;
    work_done &= ~ENET_POLL_DONE;
    BCM_ENET_DEBUG("Work Done: %d \n", (int)work_done);
    *budget -= work_done;
    dev->quota -= work_done;

    if (ret_done != ENET_POLL_DONE) {
        /* Did as much as could, but we are not done yet */
        return 1;
    }

    ENET_RX_LOCK();

    /* Reschedule if there are any rx packets in the ring */
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
        if (rxchannel_isr_enable[channel])
#endif
        {
            /* clear the interrupt */
            bcmPktDma_EthClrRxIrq(&rxdma->pktDmaRxInfo);
            if (bcmPktDma_EthRecvAvailable(&rxdma->pktDmaRxInfo))
            {
                ENET_RX_UNLOCK();
                return 1;
            }
        }
    }

    ENET_RX_UNLOCK();

    /* We are done */
    netif_rx_complete(dev);

    /* Enable the interrupts */
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
        if (rxchannel_isr_enable[channel])
#endif
        {
            bcmPktDma_BcmHalInterruptEnable(channel, rxdma->rxIrq);
        }
    }

    return 0;
}
#endif // else LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
/*
 * Switch the RX DMA channel b/w standby ring and main ring
 */
static void switch_rx_ring(BcmEnet_devctrl *pDevCtrl, int channel, int toStdBy)
{
    int i = 0, status = 0, index = 0;
    DmaStateRam *StateRam;
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[channel];

    BCM_ENET_RX_DEBUG("Head = %d; Assigned BDs = %d \n",
        rxdma->pktDmaRxInfo.rxHeadIndex, rxdma->pktDmaRxInfo.rxAssignedBds);

    /* Stop DMA channel */
    rxdma->pktDmaRxInfo.rxDma->cfg = DMA_PKT_HALT;
    while(rxdma->pktDmaRxInfo.rxDma->cfg & DMA_ENABLE) {
    }
    bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);

    /* Clear State RAM */
    StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[channel*2];
    memset(StateRam, 0, sizeof(DmaStateRam));

    /* Setup rx dma channel */
    if (toStdBy) {
        BCM_ENET_RX_DEBUG("switch_rx_ring: changing to stdby ring\n");
        rxdma->pktDmaRxInfo.rxDma->maxBurst |= DMA_THROUGHPUT_TEST_EN;
        pDevCtrl->dmaCtrl->stram.s[channel * 2].baseDescPtr =
            (uint32)VIRT_TO_PHY((uint32 *)rxdma->rxBdsStdBy);
    } else {
        BCM_ENET_RX_DEBUG("switch_rx_ring: changing to main ring\n");
        rxdma->pktDmaRxInfo.rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
        pDevCtrl->dmaCtrl->stram.s[channel * 2].baseDescPtr =
            (uint32)VIRT_TO_PHY((uint32 *)rxdma->pktDmaRxInfo.rxBds);
        /* The head*/

        for (i = 0; i < rxdma->pktDmaRxInfo.rxAssignedBds; i++) {
            index = (rxdma->pktDmaRxInfo.rxHeadIndex + i) % rxdma->pktDmaRxInfo.numRxBds;
            status = rxdma->pktDmaRxInfo.rxBds[index].status;
            if (!(status & DMA_OWN)) {
                rxdma->pktDmaRxInfo.rxBds[index].length  = RX_BUF_LEN;
                if (index == (rxdma->pktDmaRxInfo.numRxBds - 1)) {
                    rxdma->pktDmaRxInfo.rxBds[index].status = DMA_OWN | DMA_WRAP;
                } else {
                    rxdma->pktDmaRxInfo.rxBds[index].status = DMA_OWN;
                }
            } else {
                break;
            }
        }
        pDevCtrl->dmaCtrl->stram.s[channel * 2].state_data = rxdma->pktDmaRxInfo.rxHeadIndex;
    }
    rxdma->pktDmaRxInfo.rxDma->intMask = 0;   /* mask all ints */
    rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;

    /* Start DMA channel if BDs are available */
    if (toStdBy || rxdma->pktDmaRxInfo.rxAssignedBds)
    {
        rxdma->pktDmaRxInfo.rxDma->cfg = DMA_ENABLE;
        bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);
    }
}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */


/*
 * bcm63xx_ephy_isr: Acknowledge interrupt.
 */
static FN_HANDLER_RT bcm63xx_ephy_isr(int irq, void * dev_id)
{
    ethsw_set_mac_link_down();
#if !defined(SUPPORT_SWMDK)
    ephy_int_cnt++;
#else
    BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#endif
    return BCM_IRQ_HANDLED;
}

#if defined(CONFIG_BCM963268)
/*
 * bcm63xx_gphy_isr: Acknowledge Gphy interrupt.
 */
static FN_HANDLER_RT bcm63xx_gphy_isr(int irq, void * dev_id)
{
    ethsw_set_mac_link_down();
#if !defined(SUPPORT_SWMDK)
    ephy_int_cnt++;
#else
    BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif
    return BCM_IRQ_HANDLED;
}
#endif

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) || defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
/*
 * bcm63xx_enet_isr: Acknowledge interrupt and check if any packets have
 *                  arrived on Rx DMA channel 0..3
 */
static FN_HANDLER_RT bcm63xx_enet_isr(int irq, void * pContext)
{
    /* this code should not run in DQM operation !!! */

    int channel;
    BcmEnet_devctrl *pDevCtrl;

    channel = CONTEXT_TO_CHANNEL((uint32)pContext);
    pDevCtrl = CONTEXT_TO_PDEVCTRL((uint32)pContext);

    /* Only rx channels owned by the Host come through this ISR */
    bcmPktDma_EthClrRxIrq_Iudma(&pDevCtrl->rxdma[channel]->pktDmaRxInfo);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    napi_schedule(&pDevCtrl->napi);
#else
    netif_rx_schedule(pDevCtrl->dev);
#endif
    return BCM_IRQ_HANDLED;
}
#endif /* !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) || defined(CONFIG_BCM_PKTDMA_RX_SPLITTING) */

#define  SELECT_NEXT_CHANNEL()                                  \
{                                                               \
    if (scheduling == SP_SCHEDULING) {                          \
        /* Channel-X is done, so move on to channel X-1 */            \
        /* The strict priority is Ch3, Ch2, Ch1, and Ch0 */              \
        global_channel--;                                       \
        if (global_channel < 0) {                               \
            rxpktgood |= ENET_POLL_DONE;                        \
            break;                                              \
        }                                                       \
        continue;                                               \
    } else if (scheduling == FAP_SCHEDULING) {                  \
        /* we only look at one channel */                            \
        rxpktgood |= ENET_POLL_DONE;                            \
        break;                                                  \
    } else {                                                    \
        if ((--channels_tbd) <= 0) {                            \
            rxpktgood |= ENET_POLL_DONE;                        \
            break;                                              \
        }                                                       \
        /* Replace the channel done with the last channel in the                  \
                  channels to be done */                                   \
        next_channel[channel_ptr] = next_channel[channels_tbd]; \
        goto next_channel;                                      \
    }                                                           \
}

#ifdef CONFIG_SMP
#define BULK_RX_LOCK_ACTIVE() pDevCtrl->bulk_rx_lock_active[cpuid]
#define RECORD_BULK_RX_LOCK() pDevCtrl->bulk_rx_lock_active[cpuid] = 1
#define RECORD_BULK_RX_UNLOCK() pDevCtrl->bulk_rx_lock_active[cpuid] = 0
#else
#define RECORD_BULK_RX_UNLOCK()
#endif

/*
 *  bcm63xx_rx: Process all received packets.
 */
static uint32 bcm63xx_rx(void *ptr, uint32 budget)
{
    BcmEnet_devctrl *pDevCtrl = ptr;
    struct net_device *dev = NULL;
    unsigned char *pBuf = NULL;
    DmaDesc dmaDesc;
    int i, vport, len=0, phy_port_id, channels_tbd = 0;
    uint32 rxpktgood = 0, rxpktprocessed = 0;
    uint32 rxpktmax = budget + (budget / 2);
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
    int gemid = 0;
#endif
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[global_channel];
    int next_channel[ENET_RX_CHANNELS_MAX];
#ifdef CONFIG_BLOG
    BlogAction_t blogAction;
#endif
#ifdef CONFIG_SMP
    /* napi rx function will never migrate, so this is safe */
    uint32 cpuid = smp_processor_id();
    /* bulk blog locking optimization only used in SMP builds */
    int got_blog_lock=0;
#endif

#if defined(CONFIG_BCM963268) 
    bool strip_brcm_tag = TRUE;
#endif

#ifdef BCM_ENET_DEBUG_BUILD
    ch_serviced[NEXT_INDEX(dbg_index)] = ISR_START;
    ch_serviced[NEXT_INDEX(dbg_index)] = pending_weight_pkts[0];
    ch_serviced[NEXT_INDEX(dbg_index)] = pending_weight_pkts[1];
    ch_serviced[NEXT_INDEX(dbg_index)] = pending_weight_pkts[2];
    ch_serviced[NEXT_INDEX(dbg_index)] = pending_weight_pkts[3];
#endif

    if(scheduling == WRR_SCHEDULING) {
        BCM_ENET_RX_DEBUG("next_channel = ");
        for(i = 0; i < cur_rxdma_channels; i++) {
            next_channel[i] = pending_channel[i];
            BCM_ENET_RX_DEBUG("%d ", next_channel[i]);
        }
        channels_tbd = pending_ch_tbd;
        BCM_ENET_RX_DEBUG("\nchannels_tbd = %d ", channels_tbd);
    }

#if defined(CC_FAP4KE_ENET_STATS) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    pHostPsmGbl->stats.Q7budget = budget;
#endif

    // JU:TBD -- this can be looked into but is not being done for now
    /* When the Kernel is upgraded to 2.6.24 or above, the napi call will
       tell you the received queue to be serviced. So, loop across queues
       can be removed. */
    /* RR loop across channels until either no more packets in any channel or
       we have serviced budget number of packets. The logic is to keep the
       channels to be serviced in next_channel array with channels_tbd
       tracking the number of channels that still need to be serviced. */
    while (budget > 0) {

#ifdef BCM_ENET_DEBUG_BUILD
        ch_serviced[NEXT_INDEX(dbg_index)] = global_channel;
        BCM_ENET_RX_DEBUG("Total pkts received on this channel = %d",
                           ch_pkts[global_channel]);
#endif
        BCM_ENET_RX_DEBUG("channels_tbd = %d; channel = %d", channels_tbd, global_channel);

#ifdef CONFIG_SMP
        /* as optimization on SMP, hold blog lock across multiple pkts */
        /* must grab blog_lock before enet_rx_lock */
        if (!got_blog_lock)
        {
            blog_lock_bh();
            got_blog_lock=1;
        }

        /* as optimization on SMP, hold rx lock across multiple pkts */
        if (0 == BULK_RX_LOCK_ACTIVE())
        {
            ENET_RX_LOCK();
            RECORD_BULK_RX_LOCK();
        }
#else
        ENET_RX_LOCK();
#endif

        rxdma = pDevCtrl->rxdma[global_channel];

        /* process received packet */
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        /* rxAssignedBds is only local for non-FAP builds */
        if (rxdma->pktDmaRxInfo.rxAssignedBds != 0)
        {
#endif


#if defined(RXCHANNEL_PKT_RATE_LIMIT)
            if (!rxchannel_isr_enable[global_channel])  {
                RECORD_BULK_RX_UNLOCK();
                ENET_RX_UNLOCK();
                SELECT_NEXT_CHANNEL();
            }
#endif

            /* Read <status,length> from Rx BD at head of ring */
            dmaDesc.word0 = bcmPktDma_EthRecv(&rxdma->pktDmaRxInfo,
                                              &pBuf, &len);

            /* If no more rx packets, we are done for this channel */
            if (dmaDesc.status & DMA_OWN)
            {
                BCM_ENET_RX_DEBUG("No Rx Pkts on this channel");
                RECORD_BULK_RX_UNLOCK();
                ENET_RX_UNLOCK();
#ifdef BCM_ENET_DEBUG_BUILD
                ch_no_pkts[global_channel]++;
#endif
                SELECT_NEXT_CHANNEL();
            }

            BCM_ENET_RX_DEBUG("Processing Rx packet");
            rxpktprocessed++;

#if defined(CC_FAP4KE_ENET_STATS) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
            pHostPsmGbl->stats.Q7rxTotal++;
#endif

            if ((len < ENET_MIN_MTU_SIZE) ||
                (dmaDesc.status & (DMA_SOP | DMA_EOP)) != (DMA_SOP | DMA_EOP))
            {
                RECORD_BULK_RX_UNLOCK();
                ENET_RX_UNLOCK();
                flush_assign_rx_buffer(pDevCtrl, global_channel, pBuf, pBuf);
                pDevCtrl->stats.rx_dropped++;
                goto next_rx;
            }

#if defined(RXCHANNEL_BYTE_RATE_LIMIT) && defined(CONFIG_BCM96816)
            if (channel_rx_rate_limit_enable[global_channel]) {
                if (jiffies_to_msecs(jiffies - last_byte_jiffies[global_channel])
                    > 1000)
                {
                    rx_pkts_from_last_jiffies[global_channel] = 0;
                    last_byte_jiffies[channel] = jiffies;
                }
                if ((rx_bytes_from_last_jiffies[global_channel] + len) >
                    channel_rx_rate_credit[global_channel]) {
                    RECORD_BULK_RX_UNLOCK();
                    ENET_RX_UNLOCK();
                    flush_assign_rx_buffer(pDevCtrl, global_channel, pBuf,pBuf);
                    pDevCtrl->stats.rx_dropped++;
                    goto next_rx;
                }
                rx_bytes_from_last_jiffies[global_channel] += len;
            }
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
            if (rxchannel_rate_limit_enable[global_channel]) {
                rx_pkts_from_last_jiffies[global_channel]++;
                if ((rx_pkts_from_last_jiffies[global_channel] >=
                     rxchannel_rate_credit[global_channel]) &&
                    rxchannel_isr_enable[global_channel])
                {
                    bcmPktDma_BcmHalInterruptDisable(global_channel, rxdma->rxIrq);
                    rxchannel_isr_enable[global_channel] = 0;
                    switch_rx_ring(pDevCtrl, global_channel, 1);
                }
            }
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */


#if defined(CONFIG_BCM963268)
            phy_port_id = port_from_flag(dmaDesc.status);
            if (pDevCtrl->EnetInfo[0].sw.phy_id[phy_port_id] & EXTSW_CONNECTED) {
                ((BcmEnet_hdr2*)pBuf)->brcm_type = BRCM_TYPE2;
                phy_port_id = BCM_PKTDMA_PORT_FROM_TYPE2_TAG(((BcmEnet_hdr2*)pBuf)->brcm_tag);
                /* Need to strip the broadcom tag from external switch */
                strip_brcm_tag = TRUE;
            }
            else {
                if (extSwInfo.present == 1)
                {
                    phy_port_id += MAX_EXT_SWITCH_PORTS; /* Port numbers for internal switch ports(non external sw ports) are from 8..15 */
                }
                /* No need to strip broadcom tag as 63268 switch is configured to not to add the tag */
                strip_brcm_tag = FALSE;
            }
#else
            if (global.pVnetDev0_g->extSwitch->brcm_tag_type == BRCM_TYPE2) {
                ((BcmEnet_hdr2*)pBuf)->brcm_type = BRCM_TYPE2;
                phy_port_id = BCM_PKTDMA_PORT_FROM_TYPE2_TAG(((BcmEnet_hdr2*)pBuf)->brcm_tag);
            } else {
                phy_port_id = port_from_flag(dmaDesc.status);
            }
#endif

            vport = phyport_to_vport[phy_port_id];

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
            /* If packet is from GPON port, get the gemid and find the gpon virtual
               interface with which that gemid is associated */
            if ( phy_port_id == GPON_PORT_ID) {
                int gponifid;
                gemid = gemid_from_dmaflag(dmaDesc.status);

                gponifid = get_first_gemid_to_ifIdx_mapping(gemid);
                if (gponifid < 0) {
                    pDevCtrl->stats.rx_dropped++;
                    RECORD_BULK_RX_UNLOCK();
                    ENET_RX_UNLOCK();
                    flush_assign_rx_buffer(pDevCtrl, global_channel, pBuf,pBuf);
                    goto next_rx;
                }
                dev = gponifid_to_dev[gponifid];
            } else
#endif
#if defined(CONFIG_BCM96816)
            /* If packet is from SERDES port check for 6829 */
            if ( (phy_port_id == SERDES_PORT_ID) &&
                 IsExt6829(global.pVnetDev0_g->EnetInfo[0].sw.phy_id[phy_port_id]) )
            {
                int idx = atomic_read(&bcm6829ActDevIdx);
                dev     = bcm6829_to_dev[idx];
            } else
#endif
            /* possibility of corrupted source port in dmaFlag,
               validate presence of device */
            if ((vport > 0) && (vport <= vport_cnt)) {
                dev = vnet_dev[vport];
            }

            if (dev != NULL)  /* validate device */
            {
                struct net_device_stats *vstats;
                struct sk_buff *skb;
                FkBuff_t * pFkb;
                uint32_t blog_chnl, blog_phy; /* used if CONFIG_BLOG enabled */
#if defined(AEI_VDSL_TOOLBOX)
                BcmEnet_devctrl *pEthCtrl;
#endif
#if defined(AEI_VDSL_STATS_DIAG)
                struct enet_dev_stats *dev_stats;
#endif

                vstats = &(((BcmEnet_devctrl *) netdev_priv(dev))->stats);
                vstats->rx_packets ++;
                vstats->rx_bytes += len;

                pDevCtrl->stats.rx_packets++;
                pDevCtrl->stats.rx_bytes += len;

#if defined(AEI_VDSL_STATS_DIAG)
                /* because of some limitations, it's not able to support 
                 * multicast rx/tx bytes counters in the hardware, except this,
                 * all other counters will be directly gotten from the hardware */
                if (pBuf && (*pBuf & 1))
                {
                    dev_stats = &(((BcmEnet_devctrl *) netdev_priv(dev))->dev_stats);
                    dev_stats->rx_multicast_bytes += len;
                }
#endif

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
                if (phy_port_id == GPON_PORT_ID) {
                    blog_chnl = gemid;      /* blog rx channel is gemid */
                    blog_phy = BLOG_GPONPHY;/* blog rx phy type is GPON */
                }
                else    /* ! GPON_PORT_ID */
#endif
#if defined(CONFIG_BCM96816)
                if ( dev->base_addr == MOCA_PORT_ID )
                {
                    blog_chnl = phy_port_id;
                    blog_phy = BLOG_MOCAPHY;/* blog rx phy type is moca */
                } else
#endif
                {
                    blog_chnl = phy_port_id;/* blog rx channel is switch port */
                    blog_phy = BLOG_ENETPHY;/* blog rx phy type is ethernet */
                }

                /* FkBuff_t<data,len> in-placed leaving headroom */
                pFkb = fkb_init(pBuf, RX_ENET_SKB_HEADROOM,
                                pBuf, len - ETH_CRC_LEN );

#if defined(CONFIG_BCM96816)
                if (blog_phy == BLOG_GPONPHY && gemMirrorCfg[MIRROR_DIR_IN].nStatus &&
                    (gemMirrorCfg[MIRROR_DIR_IN].nGemPortMask & (1 << blog_chnl)))
                {
                    struct sk_buff *skb_m;
                    FkBuff_t *fkbC_p;

                    fkbC_p = fkb_clone( pFkb );
                    skb_m = nbuff_xlate( FKBUFF_2_PNBUFF(fkbC_p) );    /* translate to skb */
                    if (skb_m != (struct sk_buff *)NULL)
                    {
                        MirrorPacket(skb_m, gemMirrorCfg[MIRROR_DIR_IN].szMirrorInterface, 1);
                        dev_kfree_skb_any( skb_m );
                    }
                }
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) || (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
                {
                    uint32 context = 0;

                    RECYCLE_CONTEXT(context)->channel = global_channel;

                    pFkb->recycle_hook = (RecycleFuncP)bcm63xx_enet_recycle;
                    pFkb->recycle_context = context;
                }
#endif

#ifdef CONFIG_BLOG
#ifdef CONFIG_SMP
                /* SMP: bulk rx, bulk blog optimization */
                blogAction = blog_finit_locked( pFkb, dev, TYPE_ETH, blog_chnl, blog_phy );
#else
                /* UNI: unlock rx, call blog, which will lock+unlock blog */
                ENET_RX_UNLOCK();
                blogAction = blog_finit( pFkb, dev, TYPE_ETH, blog_chnl, blog_phy );
#endif
                if ( blogAction == PKT_DROP )
                {
#if defined(AEI_VDSL_STATS_DIAG)
                    if (pBuf)
                    {
                        dev_stats = &(((BcmEnet_devctrl *) netdev_priv(dev))->dev_stats);

                        if (*pBuf & 1)
                            dev_stats->multicast_discarded_packets++;
                        else
                            dev_stats->unicast_discarded_packets++;
                    }
#endif

                    /* CPU is congested and fcache has identified the packet
                     * as low prio, and needs to be dropped */
                    flush_assign_rx_buffer(pDevCtrl, global_channel, pBuf, pBuf + len);
                    pDevCtrl->stats.rx_dropped++;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CC_IQ_STATS)
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
                    if (rxdma->pktDmaRxInfo.rxOwnership == HOST_OWNED)
                        rxdma->pktDmaRxInfo.iqDropped++;
                    else
                        rxdma->pktDmaRxInfo.iqDroppedDqm++;
#else
                    rxdma->pktDmaRxInfo.iqDroppedDqm++;
#endif
#else
                    rxdma->pktDmaRxInfo.iqDropped++;
#endif
#endif
#endif
                    goto next_rx;
                }
                else
                {
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
                    //alloc a new buf from bpm
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
                    if (rxdma->pktDmaRxInfo.rxOwnership == HOST_OWNED)
                        enet_bpm_alloc_buf( pDevCtrl, global_channel );
#endif
#else
                    enet_bpm_alloc_buf( pDevCtrl, global_channel );
#endif
#endif
                }

                /* packet consumed, proceed to next packet*/
                if ( blogAction == PKT_DONE )
                    goto next_rx;

#ifndef CONFIG_SMP
                /* UNI */
                ENET_RX_LOCK();
#endif
#endif /* CONFIG_BLOG */

                if (rxdma->freeSkbList == NULL)
                {
                    /* Not necessary to flush cache as no access was done.  */
                    RECORD_BULK_RX_UNLOCK();
                    ENET_RX_UNLOCK();

#if defined(AEI_VDSL_STATS_DIAG)
                    if (pBuf)
                    {
                        dev_stats = &(((BcmEnet_devctrl *) netdev_priv(dev))->dev_stats);

                        if (*pBuf & 1)
                            dev_stats->multicast_discarded_packets++;
                        else
                            dev_stats->unicast_discarded_packets++;
                    }
#endif
                    fkb_release(pFkb);
                    pDevCtrl->stats.rx_dropped++;
                    flush_assign_rx_buffer(pDevCtrl, global_channel,
                                           pBuf, pBuf + len);
                    if( rxpktprocessed < rxpktmax ) {
                        continue;
                    }
                    break;
                }

                skb = rxdma->freeSkbList;
                rxdma->freeSkbList = rxdma->freeSkbList->next_free;

                /*
                 * We are outside of the fast path and not touching any
                 * critical variables, so release all locks.
                 */
                RECORD_BULK_RX_UNLOCK();
                ENET_RX_UNLOCK();

#ifdef CONFIG_SMP
                got_blog_lock=0;
                blog_unlock_bh();
#endif

                {
                    uint32 recycle_context = 0;

                    RECYCLE_CONTEXT(recycle_context)->channel = global_channel;

                skb_headerinit(RX_ENET_SKB_HEADROOM,
#if defined(ENET_CACHE_SMARTFLUSH)
                    SKB_DATA_ALIGN(len+ENET_SKB_TAILROOM),
#else
                    RX_BUF_LEN,
#endif
                    skb, pBuf, (RecycleFuncP)bcm63xx_enet_recycle_skb_or_data,
                                   recycle_context,(void*)pFkb->blog_p);

                }

                skb_trim(skb, len - ETH_CRC_LEN);
                skb->dev = dev;

#if defined(CONFIG_BCM96816) && defined(DBL_DESC)
                skb->mark = SKBMARK_SET_PORT(skb->mark, gemid);
#if defined(SUPPORT_HELLO)
                if ( pktCmfHelloRx( skb, phy_port_id, bcm63xx_enet_xmit_port )
                     != HELLO_PKT_FWD2MIPS ) {
                    goto next_rx;
                }
#endif  /* SUPPORT_HELLO */
#endif

#if defined(CONFIG_BCM963268)
                skb->protocol = bcm_type_trans(skb, dev, strip_brcm_tag);  
#else
                skb->protocol = bcm_type_trans(skb, dev);
#endif

#if defined(AEI_VDSL_TOOLBOX)
                pEthCtrl = (BcmEnet_devctrl *)netdev_priv(skb->dev);

                if (pEthCtrl->usMirrorInFlags != 0)
                {
                    AEI_MultiMirrorPacket( skb, pEthCtrl->usMirrorInFlags, TRUE);
                }
#endif

                if (global.dump_enable) {
                   DUMP_PKT(skb->data, skb->len);
                }
                rxpktgood++;
                netif_receive_skb(skb);
            }
            else    /* invalid vport, do not use vnet_dev[vport] */
            {
                RECORD_BULK_RX_UNLOCK();
                ENET_RX_UNLOCK();
                flush_assign_rx_buffer(pDevCtrl, global_channel, pBuf, pBuf);
                pDevCtrl->stats.rx_dropped++;

                printk(KERN_NOTICE "ETH Rcv: Pkt with invalid vport/phy_port_id(0x%x/0x%x),dmaDesc.word0=0x%x\n",
       vport,phy_port_id,(unsigned int)dmaDesc.word0);
            }

next_rx:
            budget--;
            BCM_ENET_RX_DEBUG("Received a good packet");
#ifdef BCM_ENET_DEBUG_BUILD
            ch_pkts[global_channel]++;
            ch_serviced[dbg_index % NUM_ELEMS] |= (1<<31);
#endif

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
            /* Replace removed code for non-FAP builds - Feb 2011 */
            /* Check for next packet, same channel - Oct 2010 */
            continue;
#endif

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        /* rxAssignedBds is only local for non-FAP builds */
        } else {
            RECORD_BULK_RX_UNLOCK();
            ENET_RX_UNLOCK();
            BCM_ENET_RX_DEBUG("No RxAssignedBDs for this channel");
#ifdef BCM_ENET_DEBUG_BUILD
            ch_no_bds[global_channel]++;
#endif
             SELECT_NEXT_CHANNEL();
        }
#endif

        /* Update the pending weights and channels_tbd */
        if (scheduling == WRR_SCHEDULING) {
            pending_weight_pkts[global_channel]--;
            if (pending_weight_pkts[global_channel] <= 0) {
                if (pending_channels_mask & (1<<global_channel)) {
                    pending_channels_mask &= (~(1<<global_channel));
                    BCM_ENET_RX_DEBUG("pending_channels_mask = %x",
                                       pending_channels_mask);
                    /* If channels_tbd is less than or equal to 0, we are done.
                       So, get out of here */
                    if ((--channels_tbd) <= 0) {
                        rxpktgood |= ENET_POLL_DONE;
                        break;
                    }
                    /* Replace the channel done with the last channel in the
                       channels to be done */
                    next_channel[channel_ptr] = next_channel[channels_tbd];

                    if ((--pending_ch_tbd) <= 0) {
                        rxpktgood |= ENET_POLL_DONE;
                        break;
                    }
                    BCM_ENET_RX_DEBUG("pending_ch_tbd = %d", pending_ch_tbd);
                    /* Replace the channel done with the last channel in the
                       channels to be done */
                    pending_channel[channel_ptr] = pending_channel[pending_ch_tbd];
                }
                else
                {
                    // Sampling the same ch again.
                    // Time to reset the pending variables - Mar 2011
                    rxpktgood |= ENET_POLL_DONE;
                }

                /* WRR - Check for next packet on new channel - Jan 2011 */
                goto next_channel;
            }
        } else {
            /* SP or FAP Scheduling. Continue servicing the same channel */
            BCM_ENET_RX_DEBUG("SP Sched: Continue Rx on the same channel");
        }
        /* WRR - Check for next packet, same channel. Line moved - Jan 2011 */
        continue;

next_channel:
        BCM_ENET_RX_DEBUG("Selecting next channel for WRR scheduling");
        /* Get the array index for next channel and remember it. We need this
           index for replacing the channel done in next_channel array. */
        channel_ptr = (++loop_index) % channels_tbd;
        /* Select the next channel to be serviced from the next_channel array
           using the loop_index. Any scheduling alogirthms across channels can
           be implemented by chaging the logic on selecting the next channel.*/
        global_channel = next_channel[channel_ptr];
    } /* end while (budget > 0) */


#ifdef CONFIG_SMP
    if (BULK_RX_LOCK_ACTIVE())
    {
        RECORD_BULK_RX_UNLOCK();
        ENET_RX_UNLOCK();
    }

    if (got_blog_lock)
    {
        got_blog_lock=0;
        blog_unlock_bh();
    }
#endif

    pDevCtrl->dev->last_rx = jiffies;

#if defined(CC_FAP4KE_ENET_STATS) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    pHostPsmGbl->stats.Q7rxCount = rxpktprocessed;
    if(rxpktprocessed > pHostPsmGbl->stats.Q7rxHighWm)
    {
        pHostPsmGbl->stats.Q7rxHighWm = rxpktprocessed;
    }
#endif

    if(scheduling == SP_SCHEDULING) {
        global_channel = cur_rxdma_channels - 1;
    } else if (scheduling == FAP_SCHEDULING) {
        global_channel = 0;
    } else { /* WRR_SCHEDULING */
        if(rxpktgood & ENET_POLL_DONE) {
#ifdef BCM_ENET_DEBUG_BUILD
            ch_serviced[NEXT_INDEX(dbg_index)] = WRR_RELOAD;
#endif
            /* reload the pending_weight_pkts[] array */
            /* reload the next_channel[] array */
            for(i = 0; i < cur_rxdma_channels; i++) {
                pending_weight_pkts[i] = weight_pkts[i];
                pending_channel[i] = i;
            }
            /* reset the other scheduling variables */
            global_channel = channel_ptr = loop_index = 0;
            pending_ch_tbd = cur_rxdma_channels;
            pending_channels_mask = channels_mask;
        }
    }

#ifdef CONFIG_SMP
    BCM_ASSERT_C(0 == got_blog_lock);
#endif
    BCM_ASSERT_NOT_HAS_SPINLOCK_C(&global.pVnetDev0_g->ethlock_rx);

    return rxpktgood;
}


/*
 * Set the hardware MAC address.
 */
static int bcm_set_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if(netif_running(dev))
        return -EBUSY;

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
    return 0;
}

/*
 * init_dma: Initialize DMA control register
 */
static void init_dma(BcmEnet_devctrl *pDevCtrl)
{
    int channel;
    DmaStateRam *StateRam;
    BcmEnet_RxDma *rxdma;

    TRACE(("bcm63xxenet: init_dma\n"));

    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[channel*2];
        memset(StateRam, 0, sizeof(DmaStateRam));
    }
    for (channel = 0; channel < cur_txdma_channels; channel++) {
        StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[(channel*2) + 1];
        memset(StateRam, 0, sizeof(DmaStateRam));
    }

    for (channel = 0; channel < cur_txdma_channels; channel++) {
        setup_txdma_channel(channel);
    }

    /* setup the RX DMA channels */
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
        rxdma->pktDmaRxInfo.rxDma->cfg = 0;
        rxdma->pktDmaRxInfo.rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
        rxdma->pktDmaRxInfo.rxDma->intMask = 0;
        rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
        rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
        pDevCtrl->dmaCtrl->stram.s[channel * 2].baseDescPtr =
            (uint32)VIRT_TO_PHY((uint32 *)rxdma->pktDmaRxInfo.rxBds);

    }
}

static void setup_txdma_channel(int channel)
{
    BcmPktDma_EthTxDma *txdma;
    txdma = global.pVnetDev0_g->txdma[channel];
    txdma->txDma->cfg = 0;
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH | DMA_DESCSIZE_SEL;
#else
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH;
#endif
    txdma->txDma->intMask = 0;
    global.pVnetDev0_g->dmaCtrl->stram.s[(channel * 2) + 1].baseDescPtr =
        (uint32)VIRT_TO_PHY((uint32 *)txdma->txBds);
}

/*
 *  init_buffers: initialize driver's pools of receive buffers
 */
static int init_buffers(BcmEnet_devctrl *pDevCtrl, int channel)
{
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    const unsigned long BlockSize = (64 * 1024);
    const unsigned long BufsPerBlock = BlockSize / RX_BUF_SIZE;
    unsigned long AllocAmt;
    unsigned char *pFkBuf;
    int j=0;
#endif
    int i;
    unsigned char *pSkbuff;
    unsigned long BufsToAlloc;
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    unsigned char *data;
#endif
    BcmEnet_RxDma *rxdma;
    uint32 context = 0;

    RECYCLE_CONTEXT(context)->channel = channel;

    TRACE(("bcm63xxenet: init_buffers\n"));

    /* allocate recieve buffer pool */
    rxdma = pDevCtrl->rxdma[channel];
    /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
    rxdma->pktDmaRxInfo.rxAssignedBds = 0;
    rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;
    BufsToAlloc = rxdma->pktDmaRxInfo.numRxBds;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if (enet_bpm_alloc_buf_ring(pDevCtrl, channel, BufsToAlloc) == GBPM_ERROR)
    {
        printk(KERN_NOTICE "Eth: Low memory.\n");

        /* release all allocated receive buffers */
        enet_bpm_free_buf_ring(rxdma, channel);
        return -ENOMEM;
    }
#else
    if( (rxdma->buf_pool = kzalloc(BufsToAlloc * sizeof(uint32_t) + 0x10,
        GFP_ATOMIC)) == NULL )
    {
        printk(KERN_NOTICE "Eth: Low memory.\n");
        return -ENOMEM;
    }

    while( BufsToAlloc ) {
        AllocAmt = (BufsPerBlock < BufsToAlloc) ? BufsPerBlock : BufsToAlloc;
        if( (data = kmalloc(AllocAmt * RX_BUF_SIZE + 0x10, GFP_ATOMIC)) == NULL )
        {
            /* release all allocated receive buffers */
            printk(KERN_NOTICE CARDNAME": Low memory.\n");
            for (i = 0; i < j; i++) {
                if (rxdma->buf_pool[i]) {
                    kfree(rxdma->buf_pool[i]);
                    rxdma->buf_pool[i] = NULL;
                }
            }
            return -ENOMEM;
        }

        rxdma->buf_pool[j++] = data;
        /* Align data buffers on 16-byte boundary - Apr 2010 */
        data = (unsigned char *) (((UINT32) data + 0x0f) & ~0x0f);
        for (i = 0, pFkBuf = data; i < AllocAmt; i++, pFkBuf += RX_BUF_SIZE) {
            /* Place a FkBuff_t object at the head of pFkBuf */
            fkb_preinit(pFkBuf, (RecycleFuncP)bcm63xx_enet_recycle, context);
            flush_assign_rx_buffer(pDevCtrl, channel, /* headroom not flushed */
                        PFKBUFF_TO_PDATA(pFkBuf,RX_ENET_SKB_HEADROOM),
                        (uint8_t*)pFkBuf + RX_BUF_SIZE);
        }
        BufsToAlloc -= AllocAmt;
    }
#endif


    if( (rxdma->skbs_p = kmalloc(
                    (rxdma->pktDmaRxInfo.numRxBds * SKB_ALIGNED_SIZE) + 0x10,
                    GFP_ATOMIC)) == NULL )
        return -ENOMEM;

    memset(rxdma->skbs_p, 0,
                (rxdma->pktDmaRxInfo.numRxBds * SKB_ALIGNED_SIZE) + 0x10);

    /* Chain socket skbs */
    for(i = 0, pSkbuff = (unsigned char *)
        (((unsigned long) rxdma->skbs_p + 0x0f) & ~0x0f);
            i < rxdma->pktDmaRxInfo.numRxBds; i++, pSkbuff += SKB_ALIGNED_SIZE)
    {
        ((struct sk_buff *) pSkbuff)->next_free = rxdma->freeSkbList;
        rxdma->freeSkbList = (struct sk_buff *) pSkbuff;
    }

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    /* Initialize the StdBy BD Ring */
    {
    if( (data = kmalloc(RX_BUF_SIZE, GFP_ATOMIC)) == NULL ) {
        /* release all allocated receive buffers */
        printk(KERN_NOTICE CARDNAME": Low memory.\n");
        return -ENOMEM;
    }
    rxdma->StdByBuf = data;
    rxdma->rxBdsStdBy[0].address =
             (uint32)VIRT_TO_PHY(data + RX_ENET_SKB_HEADROOM);
    rxdma->rxBdsStdBy[0].length  = RX_BUF_LEN;
    rxdma->rxBdsStdBy[0].status = DMA_OWN | DMA_WRAP;
    }
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

    return 0;
}

/*
 *  uninit_buffers: un-initialize driver's pools of receive buffers
 */
void uninit_buffers(BcmEnet_RxDma *rxdma)
{
    int i;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    int channel;
    uint32 context=0;
    uint32 rxAddr=0;

    channel  = RECYCLE_CONTEXT(context)->channel;

    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++)
    {
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        if (bcmPktDma_EthRecvBufGet(&rxdma->pktDmaRxInfo, &rxAddr) == TRUE)
#endif
        {
            if ((uint8 *) rxAddr != NULL)
            {
                gbpm_free_buf((uint32_t *) PDATA_TO_PFKBUFF(rxAddr,RX_ENET_SKB_HEADROOM));
            }
        }
    }
#else
    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++) {
        if (rxdma->buf_pool[i]) {
            kfree(rxdma->buf_pool[i]);
            rxdma->buf_pool[i] = NULL;
        }
    }
    kfree(rxdma->buf_pool);

    kfree(rxdma->skbs_p);
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    /* Free the buffer in StdBy Ring */
    kfree(rxdma->StdByBuf);
    rxdma->StdByBuf = NULL;
    /* BDs freed elsewhere - Apr 2010 */
#endif
}

/* Note: this may be called from an atomic context */
static int bcm63xx_alloc_rxdma_bds(int channel, BcmEnet_devctrl *pDevCtrl)
{
   BcmEnet_RxDma *rxdma;
   rxdma = pDevCtrl->rxdma[channel];

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
   /* Allocate 1 extra BD for rxBdsStdBy */
   rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_EthAllocRxBds(channel, rxdma->pktDmaRxInfo.numRxBds + 1);
#else
   rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_EthAllocRxBds(channel, rxdma->pktDmaRxInfo.numRxBds);
#endif
   if ( rxdma->pktDmaRxInfo.rxBdsBase == NULL )
   {
      printk("Unable to allocate memory for Rx Descriptors \n");
      return -ENOMEM;
   }
#if defined(ENET_RX_BDS_IN_PSM)
   rxdma->pktDmaRxInfo.rxBds = rxdma->pktDmaRxInfo.rxBdsBase;
#else
   /* Align BDs to a 16-byte boundary - Apr 2010 */
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)(((int)rxdma->pktDmaRxInfo.rxBdsBase + 0xF) & ~0xF);
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(rxdma->pktDmaRxInfo.rxBds);
#endif

   /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
   rxdma->pktDmaRxInfo.rxAssignedBds = 0;
   rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
   /* stand by bd ring with only one BD */
   rxdma->rxBdsStdBy = &rxdma->pktDmaRxInfo.rxBds[rxdma->pktDmaRxInfo.numRxBds];
#endif

   printk("ETH Init: Ch:%d - %d rx BDs at 0x%x\n",
          channel, rxdma->pktDmaRxInfo.numRxBds, (unsigned int)rxdma->pktDmaRxInfo.rxBds);
   return 0;
}

static int bcm63xx_alloc_txdma_bds(int channel, BcmEnet_devctrl *pDevCtrl)
{
   BcmPktDma_EthTxDma *txdma;
   int nr_tx_bds;

   txdma = pDevCtrl->txdma[channel];
   nr_tx_bds = txdma->numTxBds;

   /* BDs allocated in bcmPktDma lib in PSM or in DDR */
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
   txdma->txBdsBase = bcmPktDma_EthAllocTxBds(channel, nr_tx_bds );
   if ( txdma->txBdsBase == NULL )
   {
      printk("Unable to allocate memory for Tx Descriptors \n");
      return -ENOMEM;
   }
   /* Align BDs to a 16-byte boundary - Apr 2010 */
   txdma->txBds = (volatile DmaDesc16 *)(((int)txdma->txBdsBase + 0xF) & ~0xF);
   txdma->txBds = (volatile DmaDesc16 *)CACHE_TO_NONCACHE(txdma->txBds);
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDesc16)));
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);

#else /* !(defined(CONFIG_BCM96816) && defined(DBL_DESC)) */
   txdma->txBdsBase = bcmPktDma_EthAllocTxBds(channel, nr_tx_bds);
   if ( txdma->txBdsBase == NULL )
   {
      printk("Unable to allocate memory for Tx Descriptors \n");
      return -ENOMEM;
   }

  #if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(txdma->txOwnership == HOST_OWNED)
    {
        /* Align BDs to a 16-byte boundary - Apr 2010 */
        txdma->txBds = (volatile DmaDesc *)(((int)txdma->txBdsBase + 0xF) & ~0xF);
        txdma->txBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(txdma->txBds);
        txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDesc)));
        txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);
    }
    else
    {
  #endif
#if defined(ENET_TX_BDS_IN_PSM)
   txdma->txBds = txdma->txBdsBase;
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDesc)));
  #else   /* TX BDs in DDR */
   /* Align BDs to a 16-byte boundary - Apr 2010 */
   txdma->txBds = (volatile DmaDesc *)(((int)txdma->txBdsBase + 0xF) & ~0xF);
   txdma->txBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(txdma->txBds);
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDesc)));
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);
  #endif

  #if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    }
#endif

#endif

   printk("ETH Init: Ch:%d - %d tx BDs at 0x%x\n", channel, nr_tx_bds, (unsigned int)txdma->txBds);

   txdma->txFreeBds = nr_tx_bds;
   txdma->txHeadIndex = txdma->txTailIndex = 0;

   bcmPktDma_EthInitTxChan(nr_tx_bds, pDevCtrl->txdma[channel]);
   return 0;
}

static int bcm63xx_init_txdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;

    pDevCtrl->txdma[channel] = (BcmPktDma_EthTxDma *) (kzalloc(
                           sizeof(BcmPktDma_EthTxDma), GFP_KERNEL));
    if (pDevCtrl->txdma[channel] == NULL) {
        printk("Unable to allocate memory for tx dma rings \n");
        return -ENXIO;
    }

    BCM_ENET_DEBUG("The txdma is %p \n", pDevCtrl->txdma[channel]);

    txdma = pDevCtrl->txdma[channel];
    txdma->channel = channel;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    txdma->fapIdx = getFapIdxFromEthTxIudma(channel);
#endif

    txdma->txDma = &pDevCtrl->dmaCtrl->chcfg[(channel * 2) + 1];

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    txdma->txOwnership = g_Eth_tx_iudma_ownership[channel];
#endif

    /* init number of Tx BDs in each tx ring */
    txdma->numTxBds = bcmPktDma_EthGetTxBds( txdma, channel );

    BCM_ENET_DEBUG("Enet: txbds=%u \n", txdma->numTxBds);
    return 0;
}

static int bcm63xx_init_rxdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;

    /* init rx dma channel structures */
    pDevCtrl->rxdma[channel] = (BcmEnet_RxDma *) (kzalloc(
                           sizeof(BcmEnet_RxDma), GFP_KERNEL));
    if (pDevCtrl->rxdma[channel] == NULL) {
        printk("Unable to allocate memory for rx dma rings \n");
        return -ENXIO;
    }
    BCM_ENET_DEBUG("The rxdma is %p \n", pDevCtrl->rxdma[channel]);

    rxdma = pDevCtrl->rxdma[channel];
    rxdma->pktDmaRxInfo.channel = channel;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    rxdma->pktDmaRxInfo.fapIdx = getFapIdxFromEthRxIudma(channel);
#endif

#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
    /* FAP_TBD: Is this needed for 268 BPM or IQ? */
    rxdma->pktDmaRxInfo.rxOwnership = g_Eth_rx_iudma_ownership[channel];
#endif

    /* init number of Rx BDs in each rx ring */
    rxdma->pktDmaRxInfo.numRxBds =
                    bcmPktDma_EthGetRxBds( &rxdma->pktDmaRxInfo, channel );

    /* init rxdma structures */
    rxdma->rxIrq = bcmPktDma_EthSelectRxIrq(channel);
    rxdma->pktDmaRxInfo.rxDma = &pDevCtrl->dmaCtrl->chcfg[channel * 2];

    /* disable the interrupts from device */
    bcmPktDma_BcmHalInterruptDisable(channel, rxdma->rxIrq);

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) || defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
    /* register the interrupt service handler */
    if(rxdma->rxIrq)
    {   /* a Host owned channel */
        BcmHalMapInterrupt(bcm63xx_enet_isr,
            BUILD_CONTEXT(pDevCtrl,channel), rxdma->rxIrq);
    }
#endif
    return 0;
}

/*
 * bcm63xx_init_dev: initialize Ethernet MACs,
 * allocate Tx/Rx buffer descriptors pool, Tx header pool.
 * Note that freeing memory upon failure is handled by calling
 * bcm63xx_uninit_dev, so no need of explicit freeing.
 */
static int bcm63xx_init_dev(BcmEnet_devctrl *pDevCtrl)
{
    int i, rc = 0;
    BcmEnet_RxDma *rxdma;
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
    struct ethswctl_data e2;


#endif


    TRACE(("bcm63xxenet: bcm63xx_init_dev\n"));

    bcmenet_in_init_dev = 1;

    g_pEnetDevCtrl = pDevCtrl;   /* needs to be set before assign_rx_buffers is called */
    /* Handle pkt rate limiting independently in the FAP. No need for global array */

    /* Get the pointer to switch DMA registers */
    pDevCtrl->dmaCtrl = (DmaRegs *)(SWITCH_DMA_BASE);

    /* Initialize the Tx DMA software structures */
    for (i = 0; i < ENET_TX_CHANNELS_MAX; i++) {
        rc = bcm63xx_init_txdma_structures(i, pDevCtrl);
        if (rc < 0)
            return rc;
    }

    /* Set the default tx queue to 0 */
    pDevCtrl->default_txq = 0;
    pDevCtrl->use_default_txq = 0;

    /* Initialize the Rx DMA software structures */
    for (i = 0; i < ENET_RX_CHANNELS_MAX; i++) {
        rc = bcm63xx_init_rxdma_structures(i, pDevCtrl);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
        enet_rx_init_iq_thresh(pDevCtrl, i);
#endif
        if (rc < 0)
            return rc;
    }

    /* allocate and assign tx buffer descriptors */
    for (i=0; i < cur_txdma_channels; ++i)
    {
        rc = bcm63xx_alloc_txdma_bds(i,pDevCtrl);
        if (rc < 0)
        {
            return rc;
        }
        
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        enet_bpm_init_tx_drop_thr(pDevCtrl, i);
        enet_bpm_set_tx_drop_thr( pDevCtrl, i );
#endif
#endif
    }


    BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
    BcmHalInterruptDisable(INTERRUPT_ID_GPHY);
#endif

    pending_ch_tbd = cur_rxdma_channels;
    for (i = 0; i < cur_rxdma_channels; i++) {
        channels_mask |= (1 << i);
    }
    pending_channels_mask = channels_mask;

    /* alloc space for the rx buffer descriptors */
    for (i = 0; i < cur_rxdma_channels; i++)
    {
        rc = bcm63xx_alloc_rxdma_bds(i,pDevCtrl);
        if (rc < 0)
        {
            return rc;
        }
    }

    /* init dma registers */
    init_dma(pDevCtrl);

    for (i=0;i<cur_rxdma_channels;i++)
    {
        rxdma = pDevCtrl->rxdma[i];

        //printk("rxdma[0] is 0x%x, address of 0x%x\n", (unsigned int)pDevCtrl->rxdma[i], (unsigned int)&pDevCtrl->rxdma[i]);
        //printk("rxdma[%d] rxBds 0x%x, rxDma 0x%x\n", i, (unsigned int)pDevCtrl->rxdma[i]->pktDmaRxInfo.rxBds, (unsigned int)pDevCtrl->rxdma[i]->pktDmaRxInfo.rxDma);
        bcmPktDma_EthInitRxChan(rxdma->pktDmaRxInfo.numRxBds,
                                                        &rxdma->pktDmaRxInfo);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
        enet_rx_set_iq_thresh( pDevCtrl, i );
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        enet_rx_set_bpm_alloc_trig( pDevCtrl, i );
#endif

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#if defined(CONFIG_BCM963268) && (CONFIG_BCM_EXT_SWITCH)
    bcmPktDma_EthInitExtSw(extSwInfo.connected_to_internalPort);
#endif
#endif

        /* initialize the receive buffers */
        if (init_buffers(pDevCtrl, i)) {
            printk("Unable to allocate rx packet buffers \n");
            return -ENOMEM;
        }
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        gbpm_resv_rx_buf( GBPM_PORT_ETH, i, rxdma->pktDmaRxInfo.numRxBds,
            (rxdma->pktDmaRxInfo.numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100) );
#endif
    }

#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
    /* Needed to allow iuDMA split override to work properly - Feb 2011 */
    /* The equivalent of "ethswctl -c cosq -q 1 -v 1" */
    /* This associates egress queue 1 on the switch to iuDMA1 */
    e2.type = TYPE_SET;
    e2.queue = 1;
    e2.channel = 1;
    enet_ioctl_ethsw_cosq_rxchannel_mapping(&e2);
#endif

    /* Workaround for 4ke */
    cache_flush_len(pDevCtrl, sizeof(BcmEnet_devctrl));

    bcmenet_in_init_dev = 0;
    /* if we reach this point, we've init'ed successfully */
    return 0;
}

static void bcm63xx_uninit_txdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    int nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];
#endif

    txdma = pDevCtrl->txdma[channel];

    /* disable DMA */
    txdma->txEnabled = 0;
    txdma->txDma->cfg = 0;
    (void) bcmPktDma_EthTxDisable(txdma);

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* if any, free the tx skbs */
    while (txdma->txFreeBds < nr_tx_bds) {
        txdma->txFreeBds++;
                nbuff_free((void *)txdma->txRecycle[txdma->txHeadIndex++].key);
        if (txdma->txHeadIndex == nr_tx_bds)
            txdma->txHeadIndex = 0;
    }
#endif

    /* free the transmit buffer descriptor ring */
    txdma = pDevCtrl->txdma[channel];
#if !defined(ENET_TX_BDS_IN_PSM)
    /* remove the tx bd ring */
    if (txdma->txBdsBase) {
        kfree((void *)txdma->txBdsBase);
    }
#endif

    /* free the txdma channel structures */
    for (channel = 0; channel < ENET_TX_CHANNELS_MAX; channel++) {
        if (pDevCtrl->txdma[channel]) {
            kfree((void *)(pDevCtrl->txdma[channel]));
        }
   }
}

static void bcm63xx_uninit_rxdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;

    rxdma = pDevCtrl->rxdma[channel];
    rxdma->pktDmaRxInfo.rxDma->cfg = 0;
    (void) bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);
    /* free the irq */
    bcmPktDma_BcmHalInterruptDisable(channel, rxdma->rxIrq);
    if(rxdma->rxIrq)   /* a Host owned channel */
        free_irq(rxdma->rxIrq, (BcmEnet_devctrl *)BUILD_CONTEXT(pDevCtrl,channel));

    /* release allocated receive buffer memory */
    uninit_buffers(rxdma);

    /* free the receive buffer descriptor ring */
#if !defined(ENET_RX_BDS_IN_PSM)
    if (rxdma->pktDmaRxInfo.rxBdsBase) {
        kfree((void *)rxdma->pktDmaRxInfo.rxBdsBase);
    }
#endif

    /* free the rxdma channel structures */
    if (pDevCtrl->rxdma[channel]) {
        kfree((void *)(pDevCtrl->rxdma[channel]));
    }
}

/* Uninitialize tx/rx buffer descriptor pools */
static int bcm63xx_uninit_dev(BcmEnet_devctrl *pDevCtrl)
{
    int i;

    if (pDevCtrl) {

        /* Free the Tx DMA software structures */
        for (i = 0; i < ENET_TX_CHANNELS_MAX; i++) {
            bcm63xx_uninit_txdma_structures(i, pDevCtrl);
        }

        BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
        BcmHalInterruptDisable(INTERRUPT_ID_GPHY);
#endif
        free_irq(INTERRUPT_ID_EPHY, pDevCtrl);
#if defined(CONFIG_BCM963268)
        free_irq(INTERRUPT_ID_GPHY, pDevCtrl);
#endif

        /* Free the Rx DMA software structures and packet buffers*/
        for (i = 0; i < ENET_RX_CHANNELS_MAX; i++) {
            bcm63xx_uninit_rxdma_structures(i, pDevCtrl);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            gbpm_unresv_rx_buf( GBPM_PORT_ETH, i );
#endif
        }

        /* Deleate the proc files */
        bcmenet_del_proc_files(pDevCtrl->dev);
        ethsw_del_proc_files();

        /* unregister and free the net device */
        if (pDevCtrl->dev) {
            if (pDevCtrl->dev->reg_state != NETREG_UNINITIALIZED) {
                kerSysReleaseMacAddress(pDevCtrl->dev->dev_addr);
                unregister_netdev(pDevCtrl->dev);
            }
            free_netdev(pDevCtrl->dev);
        }
    }

    return 0;
}

/*
 *      bcm63xx_enet_probe: - Probe Ethernet switch and allocate device
 */
int __init bcm63xx_enet_probe(void)
{
    static int probed = 0;
    struct net_device *dev = NULL;
    BcmEnet_devctrl *pDevCtrl = NULL;
    unsigned int chipid;
    unsigned int chiprev;
    unsigned char macAddr[ETH_ALEN];
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
    int status = 0, unit;
#if defined(CONFIG_BCM96816)
    unsigned char portInfo6829;
#endif
#if defined(AEI_VDSL_HPNA)
    char board_id[32]={0};
#endif
    int is6829=0;
    BcmEnet_devctrl *pVnetDev0;

    TRACE(("bcm63xxenet: bcm63xx_enet_probe\n"));

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    /*set Power Led Green Blink */
    kerSysLedCtrl(kLedPower, kLedStatePowerOneSecondBlinkContinues);
#endif

#if defined(AEI_VDSL_HPNA)
    kerSysNvRamGetBoardId(board_id);

    if (strstr(board_id, "C1000") != NULL)
        hpna_support = 0;
    else 
        hpna_support = 1;
#endif

    if (probed == 0)
    {
        chipid  = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
        chiprev = (PERF->RevID & REV_ID_MASK);

        if(BpGetEthernetMacInfo(&EnetInfo[0], BP_MAX_ENET_MACS) != BP_SUCCESS)
        {
            printk(KERN_DEBUG CARDNAME" board id not set\n");
            return -ENODEV;
        }
        probed++;
    }
    else
    {
        /* device has already been initialized */
        return -ENXIO;
    }
#if defined(AEI_VDSL_CUSTOMER_NCS) && !defined(AEI_63168_CHIP)
    /* if external switch exists, then shut down all ports at first to prevent any packets from coming into driver.
     * Only when a port has linked up do we turn on tx/rx for that port
     */ 
    if (EnetInfo[1].ucPhyType == BP_ENET_EXTERNAL_SWITCH) {
        extSwitchExist=TRUE;
    }
#endif

    for (unit = 0; unit < BP_MAX_ENET_MACS; unit++)
    {
        if (EnetInfo[unit].ucPhyType == BP_ENET_EXTERNAL_SWITCH)
          break;
    }

    if (unit >= BP_MAX_ENET_MACS)
      return -ENODEV;

    if ((EnetInfo[1].ucPhyType == BP_ENET_EXTERNAL_SWITCH) ||
        (EnetInfo[1].ucPhyType == BP_ENET_SWITCH_VIA_INTERNAL_PHY)) {
        unit = 1;
    }

#if defined(CONFIG_BCM963268)
    // Create a port map with only end ports. A port connected to external switch is ignored.
    consolidated_portmap = EnetInfo[0].sw.port_map;  
    if (unit == 1){
        unsigned int port;
        consolidated_portmap = EnetInfo[1].sw.port_map;   
        for (port = 0; port < MAX_SWITCH_PORTS; port++) {
            unsigned int phycfg = EnetInfo[0].sw.phy_id[port];             
            if (IsPortConnectedToExternalSwitch(phycfg)) {               
                consolidated_portmap |= ( EnetInfo[0].sw.port_map & (~(1<<port)) ) << 8;
                extSwInfo.connected_to_internalPort = port;
                break;  
            }                             
        }        
    }  
#endif        
    

#ifdef CONFIG_BCM96816
    if ( BP_SUCCESS == BpGet6829PortInfo(&portInfo6829) )
      is6829 = (0 != portInfo6829);
#endif

#ifdef NO_CFE
    ethsw_reset(is6829);
    ethsw_configure_ports(EnetInfo[0].sw.port_map, &EnetInfo[0].sw.phy_id[0]);
#endif

    dev = alloc_etherdev(sizeof(*pDevCtrl));

    if (dev == NULL)
    {
        printk(KERN_ERR CARDNAME": Unable to allocate net_device!\n");
        return -ENOMEM;
    }

    pDevCtrl = netdev_priv(dev);
    pDevCtrl->dev = dev;
    pDevCtrl->unit = unit;
    pDevCtrl->chipId  = chipid;
    pDevCtrl->chipRev = chiprev;

    global.pVnetDev0_g = pDevCtrl;
    global.pVnetDev0_g->extSwitch = &extSwInfo;
#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    global.pVnetDev0_g->enetTxChannel = PKTDMA_ETH_TX_HOST_IUDMA;   /* default for enet tx on HOST */
#endif
    if (unit == 1) {
        int bus_type, spi_id;
        /* get external switch access details */
        get_ext_switch_access_info(EnetInfo[1].usConfigType, &bus_type, &spi_id);
        extSwInfo.accessType = bus_type;
        extSwInfo.bus_num = (bus_type == MBUS_SPI)?LEG_SPI_BUS_NUM:HS_SPI_BUS_NUM;
        extSwInfo.spi_ss = spi_id;
        extSwInfo.spi_cid = 0;
        extSwInfo.brcm_tag_type = BRCM_TYPE2;
        extSwInfo.present = 1;
        global.bcm_strip_tag = bcm_strip_tag_type2;
        if ((extSwInfo.accessType == MBUS_SPI) || (extSwInfo.accessType == MBUS_HS_SPI)) {
            status = BcmSpiReserveSlave2(extSwInfo.bus_num, spi_id, 781000, SPI_MODE_3, 
                                         SPI_CONTROLLER_STATE_GATE_CLK_SSOFF);
            if ( SPI_STATUS_OK != status ) {
                printk("Unable to reserve slave id for ethernet switch\n");
            }
        }
#if defined(CONFIG_BCM963268)
        {
            int port_connected_to_ext_switch = extSwInfo.connected_to_internalPort + MAX_EXT_SWITCH_PORTS;
            pDevCtrl->wanPort |= 1 << port_connected_to_ext_switch;
        }
#endif
    }

#if defined(CONFIG_BCM96816)
    global.Is6829 = IsExt6829(EnetInfo[0].sw.phy_id[SERDES_PORT_ID]);
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    memset(&global.moca_lan, 0, sizeof(moca_queue_t));
    global.moca_lan.egressq_alloc_bufs = 32;
    global.moca_lan.mask = 1<<MOCA_PORT_ID;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    {
        int qid;
        for (qid = 0; qid < NUM_MOCA_SW_QUEUES; qid++)
            moca_lan_bpm_txq_thresh(qid);
    }
#endif

    if (global.Is6829) {
        memset(&global.moca_wan, 0, sizeof(moca_queue_t));
        global.moca_wan.egressq_alloc_bufs = 32;
        global.moca_wan.mask = 1<<(MOCA_PORT_ID+MAX_SWITCH_PORTS);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        {
            int qid;

            for (qid = 0; qid < NUM_MOCA_SW_QUEUES; qid++)
                moca_wan_bpm_txq_thresh(qid);
        }
#endif
    }
#endif
#endif

    spin_lock_init(&pDevCtrl->ethlock_tx);
    spin_lock_init(&pDevCtrl->ethlock_moca_tx);
    spin_lock_init(&pDevCtrl->ethlock_rx);
    spin_lock_init(&bcm_ethlock_phy_access);
    spin_lock_init(&bcm_extsw_access);

    memcpy(&(pDevCtrl->EnetInfo[0]), &EnetInfo[0], sizeof(ETHERNET_MAC_INFO));
    if (unit == 1)
        memcpy(&(pDevCtrl->EnetInfo[1]), &EnetInfo[1], sizeof(ETHERNET_MAC_INFO));


    printk("Broadcom BCM%X%X Ethernet Network Device ", chipid, chiprev);
    printk(VER_STR);
    printk("\n");

    if ((status = bcm63xx_init_dev(pDevCtrl)))
    {
        printk((KERN_ERR CARDNAME ": device initialization error!\n"));
        bcm63xx_uninit_dev(pDevCtrl);
        return -ENXIO;
    }

    dev_alloc_name(dev, dev->name);
    SET_MODULE_OWNER(dev);
    sprintf(dev->name, "bcmsw");

    bcmenet_add_proc_files(dev);
    ethsw_add_proc_files(dev);
    vnet_dev[0] = dev;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    dev->netdev_ops = &bcm96xx_netdev_ops;
    netif_napi_add(dev, &pDevCtrl->napi, bcm63xx_enet_poll_napi, NETDEV_WEIGHT);

#else // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    /* TBD: check below initialization of base_addr and irq */
    dev->irq                = pDevCtrl->rxdma[0]->rxIrq;
    dev->open               = bcm63xx_enet_open;
    dev->stop               = bcm63xx_enet_close;
    dev->hard_start_xmit    = (HardStartXmitFuncP)bcm63xx_enet_xmit;
    dev->tx_timeout         = bcm63xx_enet_timeout;
    dev->get_stats          = bcm63xx_enet_query;
    dev->set_mac_address    = bcm_set_mac_addr;
    dev->do_ioctl           = &bcm63xx_enet_ioctl;
    dev->poll               = bcm63xx_enet_poll;
    dev->weight             = NETDEV_WEIGHT;

#endif //else LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

    netdev_path_set_hw_port(dev, 0, BLOG_ENETPHY);

    dev->base_addr          = (unsigned int)pDevCtrl->rxdma[0]->pktDmaRxInfo.rxDma;
    dev->watchdog_timeo     = 2 * HZ;
    /* setting this flag will cause the Linux bridge code to not forward
       broadcast packets back to other hardware ports */
    dev->priv_flags         = IFF_HW_SWITCH;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) && defined(CC_FAP4KE_PKT_GSO)
#if defined(AEI_VDSL_CUSTOMER_NCS)
    dev->features           = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_UFO;
#else
    dev->features           = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_UFO | NETIF_F_FRAGLIST;
#endif
#endif

#if defined(CONFIG_BCM963268)
    bitcount(vport_cnt, consolidated_portmap);
#else
    bitcount(vport_cnt, pDevCtrl->EnetInfo[unit].sw.port_map);
#endif
    ethsw_reset_ports(dev);

    status = register_netdev(dev);

    if (status != 0)
    {
        bcm63xx_uninit_dev(pDevCtrl);
        printk(KERN_ERR CARDNAME "bcm63xx_enet_probe failed, returns %d\n", status);
        return status;
    }

    ethsw_setup_led();
#if defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
#if !defined(AEI_VDSL_POWERSAVE)
    ethsw_setup_hw_apd(1);
#endif
#endif
#ifdef DYING_GASP_API
    kerSysRegisterDyingGaspHandler(pDevCtrl->dev->name, &ethsw_switch_power_off, dev);
#endif

#if defined(CONFIG_BCM96816)
    ethsw_setup_phys();
#endif

#if defined(CONFIG_BCM963268)
    ethsw_phy_advertise_all(GPHY_PORT_PHY_ID);
#endif

#if defined(CONFIG_BCM96368) && (defined(CONFIG_BCM_PKTCMF_MODULE) || defined(CONFIG_BCM_PKTCMF))
    pktCmfSarPortEnable  = ethsw_enable_sar_port;
    pktCmfSarPortDisable = ethsw_disable_sar_port;
    pktCmfSwcConfig();
#endif

#if defined(CONFIG_BCM96816) && (defined(CONFIG_BCM_PKTCMF_MODULE) || defined(CONFIG_BCM_PKTCMF))
    pktCmfSaveSwitchPortState    = ethsw_save_port_state;
    pktCmfRestoreSwitchPortState = ethsw_restore_port_state;
#endif

    macAddr[0] = 0xff;
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
        if(strstr(dev->name,"ewan0")!=NULL) 
            kerSysGetMacAddress( dev->dev_addr,  0x13ffffff);
        else
#endif
    kerSysGetMacAddress(macAddr, dev->ifindex);

    if((macAddr[0] & ETH_MULTICAST_BIT) == ETH_MULTICAST_BIT)
    {
        memcpy(macAddr, "\x00\x10\x18\x63\x00\x00", ETH_ALEN);
        printk((KERN_CRIT "%s: MAC address has not been initialized in NVRAM.\n"), dev->name);
    }

    memmove(dev->dev_addr, macAddr, ETH_ALEN);
    ethsw_set_multiport_address((uint8_t*)dev->dev_addr);

    status = create_vport();

    if (status != 0)
      return status;

    pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);

    ethsw_init_hw(pDevCtrl->unit, pDevCtrl->EnetInfo[0].sw.port_map, pVnetDev0->wanPort, is6829);

    ethsw_init_config();

#if defined(CONFIG_BCM_EXT_SWITCH)
    /* Retrieve external switch id - this can only be done after other globals have been initialized */
    if (extSwInfo.present) {
        uint8 val[4] = {0};

        extsw_rreg(PAGE_MANAGEMENT, REG_DEV_ID, (uint8 *)&val, 4);
        extSwInfo.switch_id = swab32(*(uint32 *)val);
        printk("switch_id=%x\n",extSwInfo.switch_id);
    }
#endif

    BcmHalMapInterrupt(bcm63xx_ephy_isr, (unsigned int)pDevCtrl, INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
    BcmHalMapInterrupt(bcm63xx_gphy_isr, (unsigned int)pDevCtrl, INTERRUPT_ID_GPHY);
#endif

    poll_pid = kernel_thread((int(*)(void *))bcm63xx_enet_poll_timer, 0, CLONE_KERNEL);
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    timer_pid = kernel_thread((int(*)(void *))bcm63xx_timer, 0, CLONE_KERNEL);
#endif

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    /* create a kernel thread for software switching and bind it to CPU 0 */
    enet_softswitch_xmit_task = kthread_create(bcm63xx_softswitch_xmit_timer, NULL, "bcm63xx_softswitch_xmit_timer");
    if (IS_ERR(enet_softswitch_xmit_task))
    {
        return -ENOMEM;
    }

    kthread_bind(enet_softswitch_xmit_task, 0);
    wake_up_process(enet_softswitch_xmit_task);

    tasklet_init( &mocaTasklet, bcm63xx_moca_xmit_tasklet, 0 );
#endif
#endif

    set_bit(__LINK_STATE_START, &dev->state);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    dev->netdev_ops->ndo_open(dev);
    dev->flags |= IFF_UP;
#else
    dev->open(dev);
    dev->flags |= IFF_UP;
    dev_mc_upload(dev);
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    if (timer_pid < 0)
        return -ENOMEM;
#endif
    return ((poll_pid < 0)? -ENOMEM: 0);
}

static void __exit bcmenet_module_cleanup(void)
{
    BcmEnet_devctrl *pDevCtrl;
    TRACE(("bcm63xxenet: bcmenet_module_cleanup\n"));

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
    bcm_arl_process_hook_g = NULL;
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    iqos_enet_status_hook_g = NULL;
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    gbpm_enet_status_hook_g = NULL;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    gbpm_enet_thresh_hook_g = NULL;
#endif
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    gbpm_moca_thresh_hook_g = NULL;
#endif
#endif

#if defined(CONFIG_BCM96368) && (defined(CONFIG_BCM_PKTCMF_MODULE) || defined(CONFIG_BCM_PKTCMF))
    pktCmfSarPortEnable  = (HOOKV)NULL;
    pktCmfSarPortDisable = (HOOKV)NULL;
#endif

#if defined(CONFIG_BCM96816) && (defined(CONFIG_BCM_PKTCMF_MODULE) || defined(CONFIG_BCM_PKTCMF))
    pktCmfSaveSwitchPortState    = (HOOKV)NULL;
    pktCmfRestoreSwitchPortState = (HOOKV)NULL;
#endif

    delete_vport();
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
    delete_all_gpon_vports();
#endif

    if (poll_pid >= 0)
    {
      atomic_dec(&poll_lock);
      wait_for_completion(&poll_done);
    }

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    if (timer_pid >= 0) {
      atomic_dec(&timer_lock);
      wait_for_completion(&timer_done);
    }
#endif

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    tasklet_kill(&mocaTasklet);
    if (enet_softswitch_xmit_task)
        kthread_stop(enet_softswitch_xmit_task);
#endif
#endif

    pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    if (pDevCtrl)
    {
#ifdef DYING_GASP_API
        if(pDevCtrl->EnetInfo[0].ucPhyType == BP_ENET_EXTERNAL_SWITCH)
            kerSysDeregisterDyingGaspHandler(pDevCtrl->dev->name);
#endif
        bcm63xx_uninit_dev(pDevCtrl);
    }

    bcmFun_dereg(BCM_FUN_ID_ENET_LINK_CHG);
    bcmFun_dereg(BCM_FUN_ID_RESET_SWITCH);
    bcmFun_dereg(BCM_FUN_ID_ENET_CHECK_SWITCH_LOCKUP);
    bcmFun_dereg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY);	
}

static int enet_ioctl_ethsw_rxscheduling(struct ethswctl_data *e)
{
    int i;

    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->scheduling), (void*)&scheduling,
            sizeof(int))) {
            return -EFAULT;
        }
    } else {
        if (e->scheduling == WRR_SCHEDULING) {
            scheduling = WRR_SCHEDULING;
            for(i=0; i < ENET_RX_CHANNELS_MAX; i++) {
                pending_weight_pkts[i] = weight_pkts[i];
                pending_channel[i] = i;
            }
            /* reset the other scheduling variables */
            global_channel = channel_ptr = loop_index = 0;
            pending_ch_tbd = cur_rxdma_channels;
        } else if (e->scheduling == SP_SCHEDULING) {
            global_channel = cur_rxdma_channels - 1;
            scheduling = SP_SCHEDULING;
        } else {
            return -EFAULT;
        }
    }
    return 0;
}

static int enet_ioctl_ethsw_wrrparam(struct ethswctl_data *e)
{
    int i;
    int total_of_weights = 0;

    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->max_pkts_per_iter), (void*)&max_pkts,
            sizeof(int))) {
            return -EFAULT;
        }
        if (copy_to_user((void*)(&e->weights), (void*)&weights,
            sizeof(int) * ENET_RX_CHANNELS_MAX)) {
            return -EFAULT;
        }
    } else {
        max_pkts = e->max_pkts_per_iter;
        for(i=0; i<ENET_RX_CHANNELS_MAX; i++) {
            weights[i] = e->weights[i];
        }

        total_of_weights = 0;
        for(i=0; i<cur_rxdma_channels; i++) {
            total_of_weights += weights[i];
        }

        for(i=0; i<cur_rxdma_channels; i++) {
           weight_pkts[i] = (max_pkts/total_of_weights) * weights[i];
           pending_weight_pkts[i] = weight_pkts[i];
           BCM_ENET_DEBUG("weight[%d]_pkts: %d \n", i, weight_pkts[i]);
           pending_channel[i] = i;
        }
        global_channel = channel_ptr = loop_index = 0;
        pending_ch_tbd = cur_rxdma_channels;
    }
    return 0;
}

static int enet_ioctl_use_default_txq_config(BcmEnet_devctrl *pDevCtrl,
                                             struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->ret_val),
            (void*)&pDevCtrl->use_default_txq, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given use_default_txq: 0x%02x \n ", e->val);
        pDevCtrl->use_default_txq = e->val;
    }

    return 0;
}

static int enet_ioctl_default_txq_config(BcmEnet_devctrl *pDevCtrl,
                                         struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->queue),
            (void*)&pDevCtrl->default_txq, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->queue: 0x%02x \n ", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
        if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
            printk("Invalid queue \n");
            return BCM_E_ERROR;
        }
        pDevCtrl->default_txq = e->queue;
    }

    return 0;
}

#if defined(RXCHANNEL_BYTE_RATE_LIMIT) && defined(CONFIG_BCM96816)
static int enet_ioctl_rx_rate_limit_config(struct ethswctl_data *e)
{
    BCM_ENET_DEBUG("Given channel: %d \n ", e->channel);
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->ret_val),
            (void*)&channel_rx_rate_limit_enable[e->channel], sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate_enable_cfg: %d \n ", e->val);
        channel_rx_rate_limit_enable[e->channel % ENET_RX_CHANNELS_MAX] = e->val;
    }

    return 0;
}

static int enet_ioctl_rx_rate_config(struct ethswctl_data *e)
{
    BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->ret_val),
            (void*)&channel_rx_rate_credit[e->channel], sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate: %d \n ", e->val);
        channel_rx_rate_credit[e->channel] = e->val;
    }

    return 0;
}
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
static int enet_ioctl_soft_switch_start_queue(struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->queue),
            (void*)&global.enet_softswitch_xmit_start_q, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->queue: 0x%02x \n ", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
        if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
            printk("Invalid queue \n");
            return BCM_E_ERROR;
        }
        global.enet_softswitch_xmit_start_q = e->queue;
    }

    return 0;
}

static int enet_ioctl_moca_soft_switch(struct ethswctl_data *e)
{
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    int              vport;
    int              port_map;

    /* port map uses bit 0 for MoCA LAN and bit 1 for MoCA WAN */
    port_map  = (e->port_map & 0x2) ? (1 << SERDES_PORT_ID) : 0;
    port_map |= (e->port_map & 0x1) ? (1 << MOCA_PORT_ID)   : 0;
    if (e->type == TYPE_ENABLE)
    {
        priv->softSwitchingMap |= port_map;

        /* check MoCA LAN */
        if ( port_map & (1<<MOCA_PORT_ID))
        {
            /* enable software switching for this port */
            vport = phyport_to_vport[MOCA_PORT_ID];
            vnet_dev[vport]->priv_flags &= ~IFF_HW_SWITCH;
        }

        /* check MoCA WAN */
        if ( port_map & (1<<SERDES_PORT_ID))
        {
            unsigned short val16 = MOCA_PORT_ID;

            /* make sure that REG_FC_DIAG_PORT_SEL is set to MoCA port
               we only want to set this once to avoid another
               SPI write access in the data path */
            ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (uint8 *)&val16, 2, 1);

            /* enable software switching for this port */
            vport = phyport_to_vport_6829[MOCA_PORT_ID];
            bcm6829_to_dev[vport]->priv_flags &= ~IFF_HW_SWITCH;
            priv->softSwitchingMap6829 |= (1 << MOCA_PORT_ID);
        }
        ethsw_port_based_vlan(priv->EnetInfo[0].sw.port_map,
                              priv->wanPort, priv->softSwitchingMap);

    }
    else if (e->type == TYPE_DISABLE)
    {
        /* check MoCA WAN */
        if ( port_map & (1<<SERDES_PORT_ID) )
        {
            /* enable software switching for this port */
            vport = phyport_to_vport_6829[MOCA_PORT_ID];
            bcm6829_to_dev[vport]->priv_flags |= IFF_HW_SWITCH;
            priv->softSwitchingMap6829 &= ~(1 << MOCA_PORT_ID);
            if (priv->softSwitchingMap6829)
               port_map &= ~(1<<SERDES_PORT_ID);
        }

        priv->softSwitchingMap &= ~port_map;

        /* check MoCA LAN */
        if ( port_map & (1<<MOCA_PORT_ID))
        {
            /* disable software switching for this port */
            vport = phyport_to_vport[MOCA_PORT_ID];
            vnet_dev[vport]->priv_flags |= IFF_HW_SWITCH;
        }

        ethsw_port_based_vlan(priv->EnetInfo[0].sw.port_map,
                              priv->wanPort, priv->softSwitchingMap);
    }
    else if (e->type == TYPE_GET)
    {
        port_map  = (priv->softSwitchingMap & (1<<MOCA_PORT_ID))   ? 0x1 : 0;
        port_map |= (priv->softSwitchingMap & (1<<SERDES_PORT_ID)) ? 0x2 : 0;
        if (copy_to_user((void*)(&e->port_map),
            (void*)&port_map, sizeof(int)))
        {
            return -EFAULT;
        }

        if (copy_to_user((void*)(&e->queue),
            (void*)&global.moca_queue_depth, sizeof(int)))
        {
            return -EFAULT;
        }

        BCM_ENET_DEBUG("e->status: 0x%02x, e->port_map: 0x%02x, e->queue: %d\n ",
                        e->status, e->port_map, e->queue);
    }
    else
    {
        BCM_ENET_DEBUG("Given queue depth: %d \n ", e->queue);
        if ((e->queue >= MOCA_TXQ_DEPTH_MAX) || (e->queue <= 0))
        {
            printk("Invalid queue depth (%d)\n", e->queue);
            return BCM_E_ERROR;
        }
        global.moca_queue_depth = e->queue;
    }

    return 0;
}

static int enet_ioctl_moca_xmit_budget(struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->ret_val),
            (void*)&global.moca_xmit_budget, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given moca_xmit_budget: 0x%02x \n ", e->val);
        global.moca_xmit_budget = e->val;
    }

    return 0;
}

static int enet_ioctl_moca_port_check(struct ethswctl_data *e)
{
    int isWan = e->port ? 1 : 0;
    moca_port_check_t * moca_port_check;
    int i;

    moca_port_check = &global.moca_port_check[isWan];

    if (e->type == TYPE_ENABLE) {
        moca_port_check->enable = 1;
    }
    else if (e->type == TYPE_DISABLE) {
        moca_port_check->enable = 0;
    }
    else if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->length),
            (void*)(&moca_port_check->interval_ns), sizeof(int))) {
            return -EFAULT;
        }
        if (copy_to_user((void*)(&e->val),
            (void*)(&moca_port_check->threshold), sizeof(int))) {
            return -EFAULT;
        }

        printk("MoCA %s Port Check: %s\n", (isWan ? "WAN" : "LAN"),
            (moca_port_check->enable ? "Enabled" : "Disabled"));
        printk("Interval (ns)   = %u\n", moca_port_check->interval_ns);
        printk("Threshold       = %u\n", moca_port_check->threshold);
        printk("Match count     = ");
        for (i = 0; i < NUM_MOCA_SW_QUEUES; i++)
            printk("%u  ", moca_port_check->match_count[i]);
        printk("\n");
        printk("Last count      = ");
        for (i = 0; i < NUM_MOCA_SW_QUEUES; i++)
            printk("%u  ", moca_port_check->last_count[i]);
        printk("\n");

        printk("Last Tx Count   = ");
        for (i = 0; i < NUM_MOCA_SW_QUEUES; i++)
            printk("%lu  ", moca_port_check->last_tx_count[i]);
        printk("\n");
        printk("Last Check      = %llu\n", moca_port_check->last_check_time);
#ifdef MOCA_PORT_CHECK_DEBUG
        printk("\nRun count         = %u\n", moca_port_check->run_count);
        printk("Reset count       = %u\n", moca_port_check->reset_count);
        printk("Total match count = %u\n", moca_port_check->total_match_count);
        printk("Test              = %d\n", moca_port_check->test);
#endif

        BCM_ENET_DEBUG("MoCA %s: e->length: 0x%02x  e->val\n ",
            (isWan ? "WAN" : "LAN"), e->length, e->val);
    }
    else if (e->type == TYPE_SET) {
        BCM_ENET_DEBUG("Given moca port check params: 0x%02x 0x%02x 0x%02x\n ",
            e->port, e->length, e->val);
        if (e->length != 0)
           moca_port_check->interval_ns = e->length;
        if (e->val != 0)
           moca_port_check->threshold = e->val;

#ifdef MOCA_PORT_CHECK_DEBUG
        if (e->speed != 0)
           moca_port_check->test = e->speed;
#endif
    }

    return 0;
}
#endif /* CONFIG_BCM_MOCA_SOFT_SWITCHING */
#endif /* CONFIG_BCM96816 */

static int enet_ioctl_test_config(struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        int ret_val = 0;
        if (e->sub_type == SUBTYPE_ISRCFG) {
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
            BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
            ret_val = rxchannel_isr_enable[e->channel];
#endif
        } else if (e->sub_type == SUBTYPE_RXDUMP) {
            ret_val = global.dump_enable;
        }

        if (copy_to_user((void*)(&e->ret_val), (void*)&ret_val, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        if (e->sub_type == SUBTYPE_ISRCFG) {
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
            BcmEnet_RxDma *rxdma;
            BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
            BcmPktDma_LocalEthRxDma * local_rxdma;

            BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
            BCM_ENET_DEBUG("Given val: %d \n ", e->val);
            rxchannel_isr_enable[e->channel] = e->val;

            /* Enable/Disable the interrupts for given RX DMA channel */
            rxdma = pDevCtrl->rxdma[e->channel];
            local_rxdma = &rxdma->pktDmaRxInfo;
            if (e->val) {
                bcmPktDma_EthRxEnable(local_rxdma);
                bcmPktDma_BcmHalInterruptEnable(e->channel, rxdma->rxIrq);
            } else {
                bcmPktDma_BcmHalInterruptDisable(e->channel, rxdma->rxIrq);
                bcmPktDma_EthRxDisable(local_rxdma);
            }
#endif
        } else if (e->sub_type == SUBTYPE_RXDUMP) {
            global.dump_enable = e->val;
        }
    }

    if (e->sub_type == SUBTYPE_RESETMIB) {
        reset_mib(global.pVnetDev0_g->extSwitch->present);
    } else if (e->sub_type == SUBTYPE_RESETSWITCH) {
#if defined(CONFIG_BCM96816)
        reset_switch((e->channel & BCM_EXT_6829) ? 1 : 0);
#endif
    }
#if defined(CONFIG_BCM96816) && defined (CONFIG_BCM_MOCA_SOFT_SWITCHING)
    else if (e->sub_type == 10) {
        int i;
        for (i=0; i<NUM_MOCA_SW_QUEUES; i++) {
            printk("MoCA LAN Queue %d tail = %d; head = %d \n", i,
                    global.moca_lan.tail[i], global.moca_lan.head[i]);
            printk("MoCA LAN Queue %d packets = %d \n", i,
                    MOCA_QUEUE_NUM_PACKETS(&global.moca_lan, i));
            printk("MoCA WAN Queue %d tail = %d; head = %d \n", i,
                    global.moca_wan.tail[i], global.moca_wan.head[i]);
            printk("MoCA WAN Queue %d packets = %d \n", i,
                    MOCA_QUEUE_NUM_PACKETS(&global.moca_wan, i));
        }
    }
#endif

    return 0;
}

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
static int enet_ioctl_rx_pkt_rate_limit_config(struct ethswctl_data *e)
{
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    BCM_ENET_DEBUG("Given channel: %d \n ", e->channel);
    if (e->channel >= ENET_RX_CHANNELS_MAX || e->channel < 0) {
        return -EINVAL;
    }
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->ret_val),
            (void*)&rxchannel_rate_limit_enable[e->channel], sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate_enable_cfg: %d \n ", e->val);
        rxdma = pDevCtrl->rxdma[e->channel];
        ENET_RX_LOCK();
        rxchannel_rate_limit_enable[e->channel] = e->val;
        if ((e->val == 0) && (rxchannel_isr_enable[e->channel] == 0)) {
            switch_rx_ring(pDevCtrl, e->channel, 0);
            bcmPktDma_BcmHalInterruptEnable(e->channel, rxdma->rxIrq);
            rxchannel_isr_enable[e->channel] = 1;
        }
        ENET_RX_UNLOCK();
    }

    return 0;
}

static int enet_ioctl_rx_pkt_rate_config(struct ethswctl_data *e)
{
    BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
    if (e->type == TYPE_GET) {
        int value = rxchannel_rate_credit[e->channel] * 10;
        if (copy_to_user((void*)(&e->ret_val), (void*)&value, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate: %d \n ", e->val);
        rxchannel_rate_credit[e->channel] = (e->val/10 > 1)?(e->val/10):1;
    }

    return 0;
}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

#ifdef BCM_ENET_DEBUG_BUILD
static int enet_ioctl_getrxcounters(void)
{
    int a = 0, b = 0, c = 0, d = 0, f = 0, cnt = 0;

    printk("Rx counters: %d %d %d %d \n", ch_pkts[0],
            ch_pkts[1], ch_pkts[2], ch_pkts[3]);
    printk("No Rx Pkts counters: %d %d %d %d \n", ch_no_pkts[0],
            ch_no_pkts[1], ch_no_pkts[2], ch_no_pkts[3]);
    printk("No Rx BDs counters: %d %d %d %d \n", ch_no_bds[0],
            ch_no_bds[1], ch_no_bds[2], ch_no_bds[3]);
    printk("Channels: ");
    for (cnt = 0; cnt < NUM_ELEMS; cnt++) {
        if (ch_serviced[cnt] == WRR_RELOAD) {
            printk("\nCh0 = %d, Ch1 = %d, Ch2 = %d, Ch3 = %d \n", a,b,c,d);
            a = b = c =d = 0;
            printk("\nReloaded WRR weights \n");
        } else if (ch_serviced[cnt] == ISR_START) {
            printk("ISR START (Weights followed by channels serviced) \n");
            printk("x- indicates pkt received \n");
        } else {
            if (ch_serviced[cnt] & (1<<31)) {
                printk("x-");
                f = ch_serviced[cnt] & 0xF;
                if (f == 0) {
                    a++;
                } else if (f == 1) {
                    b++;
                } else if (f == 2) {
                    c++;
                } else if (f == 3) {
                    d++;
                }
            }
            printk("%d ", ch_serviced[cnt] & (~(1<<31)));
        }
    }
    printk("\n");

    return 0;
}

static int enet_ioctl_setrxcounters(void)
{
    int cnt = 0;

    ch_pkts[0] = ch_pkts[1] = ch_pkts[2] = ch_pkts[3] = 0;
    ch_no_pkts[0] = ch_no_pkts[1] = ch_no_pkts[2] = ch_no_pkts[3] = 0;
    ch_no_bds[0] = ch_no_bds[1] = ch_no_bds[2] = ch_no_bds[3] = 0;
    for (cnt=0; cnt<4000; cnt++) {
        ch_serviced[cnt] = 0;
    }
    dbg_index = 0;

    return 0;
}
#endif


void display_software_stats(BcmEnet_devctrl * pDevCtrl)
{
    printk("\n");
    printk("TxPkts:       %10lu \n", pDevCtrl->stats.tx_packets);
    printk("TxOctets:     %10lu \n", pDevCtrl->stats.tx_bytes);
    printk("TxDropPkts:   %10lu \n", pDevCtrl->stats.tx_dropped);
    printk("\n");
    printk("RxPkts:       %10lu \n", pDevCtrl->stats.rx_packets);
    printk("RxOctets:     %10lu \n", pDevCtrl->stats.rx_bytes);
    printk("RxDropPkts:   %10lu \n", pDevCtrl->stats.rx_dropped);
}

#define BIT_15 0x8000
static int bcm63xx_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    BcmEnet_devctrl *pDevCtrl;
    int *data=(int*)rq->ifr_data;
    char *chardata = (char *)rq->ifr_data;
    char wanifnames[680];
#if defined(CONFIG_BCM96816)
    struct net_device *pNetDev;
    int swPort6829 = 0;
    unsigned char portInfo6829;
    int bExt6829 = 0;
    MirrorCfg mirrorCfg;
#if defined(DBL_DESC)
    struct gponif_data *g=(struct gponif_data*)rq->ifr_data;
#endif // #if defined(DBL_DESC)
#endif // #if defined(CONFIG_BCM96816)

    struct ethswctl_data *e=(struct ethswctl_data*)rq->ifr_data;
    struct ethctl_data *ethctl=(struct ethctl_data*)rq->ifr_data;
    struct interface_data *enetif_data=(struct interface_data*)rq->ifr_data;
    struct mii_ioctl_data *mii;
    int val = 0, mask = 0, len = 0, cum_len = 0;
    int i, vport, phy_id, atleast_one_added = 0;
    struct net_device_stats *vstats;
    struct sockaddr sockaddr;
    int swPort;
    int phyId;
#if defined(AEI_VDSL_TOOLBOX)
    MirrorCfg mirrorCfg;
#endif

    pDevCtrl = netdev_priv(vnet_dev[0]);
    ASSERT(pDevCtrl != NULL);

#if defined(CONFIG_BCM96816)
    if ( BP_SUCCESS != BpGet6829PortInfo(&portInfo6829) )
    {
        BCM_ENET_DEBUG("BpGet6829PortInfo failed\n");
    }
    else
    {
        if ( 0 != portInfo6829)
        {
            bExt6829 = 1;
        }
    }
#endif

    switch (cmd)
    {
        case SIOCGMIIPHY:       /* Get address of MII PHY in use. */
            mii = (struct mii_ioctl_data *)&rq->ifr_data;
            swPort = port_id_from_dev(dev);
#if defined(CONFIG_BCM96816)
            if ( (SERDES_PORT_ID == swPort) && (1 == bExt6829) )
            {
                /* get the phy_id from the board information */
                phy_id = pDevCtrl->EnetInfo[0].sw.phy_id[dev->base_addr] & (~BCM_WAN_PORT);
                mii->phy_id = phy_id | BCM_EXT_6829;
            }
            else
            {
#endif
            val = phyport_to_vport[swPort] - 1;
            if (val >= 0) {
                mii->phy_id = ethsw_port_to_phyid(val);
                val = 0;
            } else {
                val = -EINVAL;
            }
#if defined(CONFIG_BCM96816)
            }
#endif
            BCM_ENET_DEBUG("phy_id: %d \n", mii->phy_id);
            break;

        case SIOCGMIIREG:       /* Read MII PHY register. */
            mii = (struct mii_ioctl_data *)&rq->ifr_data;
            down(&bcm_ethlock_switch_config);
            BCM_ENET_DEBUG("phy_id: %d; reg_num = %d \n", mii->phy_id, mii->reg_num);
            ethsw_phy_rreg(mii->phy_id, mii->reg_num & 0x1f, (uint16 *)&mii->val_out);
            up(&bcm_ethlock_switch_config);
            break;

        case SIOCGSWITCHPORT:       /* Get Switch Port. */
            val = -1;
            for (vport = 1; vport <= vport_cnt; vport++) {
               if ((vnet_dev[vport]) &&
                   (strcmp(enetif_data->ifname, vnet_dev[vport]->name) == 0)) {
                   val = ((BcmEnet_devctrl *)netdev_priv(vnet_dev[vport]))->sw_port_id;
                   break;
               }
            }
#if defined(CONFIG_BCM96816)
            if ((val == -1) && (1 == bExt6829)) {
                for (vport = 0; vport < MAX_6829_IFS; vport++) {
                    if ((bcm6829_to_dev[vport] != NULL) &&
                       (strcmp(enetif_data->ifname, bcm6829_to_dev[vport]->name) == 0)) {
                        val = SERDES_PORT_ID;
                    }
                }
            }
#endif
            if (copy_to_user((void*)&enetif_data->switch_port_id, (void*)&val, sizeof(int)))
                return -EFAULT;
            break;

        case SIOCSMIIREG:       /* Write MII PHY register. */
            mii = (struct mii_ioctl_data *)&rq->ifr_data;
            down(&bcm_ethlock_switch_config);
            BCM_ENET_DEBUG("phy_id: %d; reg_num = %d; val = 0x%x \n", mii->phy_id,
                            mii->reg_num, mii->val_in);
            ethsw_phy_wreg(mii->phy_id, mii->reg_num & 0x1f, (uint16 *)&mii->val_in);
            up(&bcm_ethlock_switch_config);
            break;

        case SIOCGLINKSTATE:
            if (dev == vnet_dev[0])
                mask = 0xffffffff;
#if defined(CONFIG_BCM96816)
            else if ( (dev == bcm6829_to_dev[0]) ||
                      (dev == bcm6829_to_dev[1]) )
            {
               mask = (0x1 << dev->base_addr) << MAX_SWITCH_PORTS;
            }
#endif
            else
                mask = 0x00000001 << port_id_from_dev(dev);

            val = (pDevCtrl->linkState & mask)? 1: 0;

#if defined(AEI_VDSL_HPNA)
            if (hpna_support)
            {
                if (hpna_port == LOGICAL_PORT_TO_PHYSICAL_PORT(port_id_from_dev(dev)))
                {
                    PHY_STAT phys = AEI_ethsw_hpna_phy_stat(hpna_port);
                    val = phys.lnk;
                }
            }
#endif

            if (copy_to_user((void*)data, (void*)&val, sizeof(int)))
                return -EFAULT;

            val = 0;
            break;

        case SIOCSCLEARMIBCNTR:
            ASSERT(pDevCtrl != NULL);

            memset(&pDevCtrl->stats, 0, sizeof(struct net_device_stats));
            /* port 0 is bcmsw */
            for (vport = 1; vport <= vport_cnt; vport++)
            {
               if (vnet_dev[vport])
               {
                  vstats = &(((BcmEnet_devctrl *)netdev_priv(vnet_dev[vport]))->stats);
                  memset(vstats, 0, sizeof(struct net_device_stats));
               }
            }
#if defined(CONFIG_BCM96816)
            for (vport = 0; vport <= MAX_6829_IFS; vport++)
            {
               if (NULL != bcm6829_to_dev[vport])
               {
                  vstats = &(((BcmEnet_devctrl *)netdev_priv(bcm6829_to_dev[vport]))->stats);
                  memset(vstats, 0, sizeof(struct net_device_stats));
               }
            }
#endif
#ifdef REPORT_HARDWARE_STATS
            reset_mib(global.pVnetDev0_g->extSwitch->present);
#endif
            val = 0;
            break;

        case SIOCMIBINFO:
            // Setup correct port indexes.
            swPort = port_id_from_dev(dev);
            vport = phyport_to_vport[swPort];

            if (vnet_dev[vport])
            {
                IOCTL_MIB_INFO *mib;

                // Create MIB address.
                mib = &((BcmEnet_devctrl *)netdev_priv(vnet_dev[vport]))->MibInfo;

                // Copy MIB to caller.
                if (copy_to_user((void*)data, (void*)mib, sizeof(IOCTL_MIB_INFO)))
                    return -EFAULT;
            }
            else
            {
              return -EFAULT;
            }

            val = 0;
            break;

        case SIOCGQUERYNUMPORTS:
            val = 1;
            if (copy_to_user((void*)data, (void*)&val, sizeof(int))) {
                return -EFAULT;
            }
            val = 0;
            break;

#if defined(CONFIG_BCM96816)
    case SIOCPORTMIRROR:
        if(copy_from_user((void*)&mirrorCfg,data,sizeof(MirrorCfg)))
            val = -EFAULT;
        else
        {
            if( mirrorCfg.nDirection == MIRROR_DIR_IN )
            {
                memcpy(&gemMirrorCfg[0], &mirrorCfg, sizeof(MirrorCfg));
            }
            else /* MIRROR_DIR_OUT */
            {
                memcpy(&gemMirrorCfg[1], &mirrorCfg, sizeof(MirrorCfg));
            }
        }
        break;
#endif
        case SIOCSWANPORT:
            if (dev == vnet_dev[0])
                return -EFAULT;

            swPort = port_id_from_dev(dev);

#if  defined(CONFIG_BCM963268)
            if ( (pDevCtrl->unit == 1) && (swPort < MAX_EXT_SWITCH_PORTS) ){
                phyId  = pDevCtrl->EnetInfo[1].sw.phy_id[swPort];
            } else {
                int port = LOGICAL_PORT_TO_PHYSICAL_PORT(swPort);
                phyId  = pDevCtrl->EnetInfo[0].sw.phy_id[port];
            }
#else
            if (pDevCtrl->unit == 1) {
                phyId  = pDevCtrl->EnetInfo[1].sw.phy_id[swPort];
            } else {
                phyId  = pDevCtrl->EnetInfo[0].sw.phy_id[swPort];
            }
#endif

#ifdef CONFIG_BCM96816
            if ( (SERDES_PORT_ID == swPort) && (1 == bExt6829) )
            {
                ETHERNET_SW_INFO *sw = &(((BcmEnet_devctrl *)netdev_priv(vnet_dev[0]))->EnetInfo[0].sw);

                swPort6829 = (1 << dev->base_addr);
                /* for the 6829 the phy port is stored in base_addr and we need to get the phy_id
                   directly from board params */
                phyId = sw->phy_id[swPort6829] & ~BCM_WAN_PORT;
            }
#endif
            if (phyId >= 0) {
                if(IsWanPort(phyId)) {
                    if ((int)data) {
                        return 0;
                    } else {
                        BCM_ENET_DEBUG("This port cannot be removed "
                            "from WAN port map");
                        return -EFAULT;
                    }
                }
            }
            if ( (int)data ) {
#ifdef CONFIG_BCM96816
                pDevCtrl->wanPort6829 |= swPort6829;
#endif
                pDevCtrl->wanPort |= (1 << swPort);
                dev->priv_flags |= IFF_WANDEV;
                dev->priv_flags &= ~IFF_HW_SWITCH;
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
        if(strstr(dev->name,"ewan0")!=NULL) 
            val=kerSysGetMacAddress( dev->dev_addr,  0x13ffffff);
        else
#endif
                val = kerSysGetMacAddress(dev->dev_addr, dev->ifindex);
                if (val == 0) {
                    memmove(sockaddr.sa_data, dev->dev_addr, ETH_ALEN);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
                    dev_set_mac_address(dev, &sockaddr);
#else
                    dev->set_mac_address(dev, &sockaddr);
#endif
                }
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING) || defined(CONFIG_BCM963268)
#if defined(CONFIG_BCM963268)
                if ( (extSwInfo.present == 0) || ((extSwInfo.present == 1) && (!IsExternalSwitchPort(swPort))) )
#endif
                {
                    struct ethswctl_data e2;
                    int i, j;

                    /* The equivalent of "ethswctl -c cosq -p {i} -q {j} -v 1" */
                    /* where i = all eth ports (0..5) except the WAN port (swPort) */
                    /* This routes packets of all priorities on the WAN eth port to egress queue 0 */
                    /* This routes packets of all priorities on all other eth ports to egress queue 1 */
                    for(i = 0; i < BP_MAX_SWITCH_PORTS; i++)
                    {
                        for(j = 0; j <= MAX_PRIORITY_VALUE; j++)
                        {
                            e2.type = TYPE_SET;
                            e2.port = i;
#if defined(CONFIG_BCM963268)
                            if ((extSwInfo.present == 1) && IsExternalSwitchPort(swPort)) 
                            {
                                e2.port = extSwInfo.connected_to_internalPort;
                            }
#endif

                            e2.priority = j;

                            if(LOGICAL_PORT_TO_PHYSICAL_PORT(swPort) == i)
                                e2.queue = PKTDMA_ETH_DS_IUDMA;  /* WAN port mapped to DS FAP */
                            else
                                e2.queue = PKTDMA_ETH_US_IUDMA;  /* other ports to US FAP */

                            mapEthPortToRxIudma(e2.port, e2.queue);
                            enet_ioctl_ethsw_cosq_port_mapping(&e2);
                        }
                    }
                }
#endif  /* if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING) */
#endif
            } else {
#ifdef CONFIG_BCM96816
                pDevCtrl->wanPort6829 &= ~swPort6829;
                /* only remove swport if this is a 6829 interface and there are no more
                   6829 WAN interfaces or this is not a 6829 port */
                if ( ((swPort6829 != 0) && (0 == pDevCtrl->wanPort6829)) ||
                     (0 == swPort6829) )
#endif
                {
                    pDevCtrl->wanPort &= ~(1 << swPort);
                }
                dev->priv_flags &= (~IFF_WANDEV);
                dev->priv_flags |= IFF_HW_SWITCH;
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
                kerSysReleaseMacAddress(dev->dev_addr);
                memmove(dev->dev_addr, vnet_dev[0]->dev_addr, ETH_ALEN);
                memmove(sockaddr.sa_data, vnet_dev[0]->dev_addr, ETH_ALEN);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
                dev_set_mac_address(dev, &sockaddr);
#else
                dev->set_mac_address(dev, &sockaddr);
#endif
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING) || defined(CONFIG_BCM963268)
#if defined(CONFIG_BCM963268)
                if ( (extSwInfo.present == 0) || ((extSwInfo.present == 1) && (!IsExternalSwitchPort(swPort))) )
#endif
                {
                    struct ethswctl_data e2;
                    int i, j;

                    /* Return all ethernet ports to be processed on the FAP - Nov 2010 (Jira 7811) */

                    /* The equivalent of "ethswctl -c cosq -p {i} -q {j} -v 0" */
                    /* where i = all eth ports (0..5) including the WAN port (swPort) */
                    for(i = 0; i < BP_MAX_SWITCH_PORTS ; i++)
                    {
                        for(j = 0; j <= MAX_PRIORITY_VALUE; j++)
                        {
                            e2.type = TYPE_SET;
                            e2.port = i;
                            e2.priority = j;
                            /* All ports mapped to default iuDMA - Mar 2011 */
                            /* US iuDMA for 63268 and DS iuDMA (ie FAP owned) for 6362 */
                            e2.queue = PKTDMA_DEFAULT_IUDMA;
                            mapEthPortToRxIudma(e2.port, e2.queue);
                            enet_ioctl_ethsw_cosq_port_mapping(&e2);
                        }
                    }
            }
#endif  /* if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING) */
#endif
            }
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
            {
                int tmpWanPort = pDevCtrl->wanPort;
                if ( (pDevCtrl->unit == 1) && (swPort < MAX_EXT_SWITCH_PORTS) )
                    extsw_set_wanoe_portmap(pDevCtrl->wanPort);
                else {
                    if (swPort >= MAX_EXT_SWITCH_PORTS) {
                        tmpWanPort >>= MAX_EXT_SWITCH_PORTS;
                    }
                    ethsw_set_wanoe_portmap(tmpWanPort);
                }
            }
#else
            if (pDevCtrl->unit == 1)
                extsw_set_wanoe_portmap(pDevCtrl->wanPort);
            else
#if defined(CONFIG_BCM96816)
                ethsw_port_based_vlan(pDevCtrl->EnetInfo[0].sw.port_map,
                                      pDevCtrl->wanPort,
                                      pDevCtrl->softSwitchingMap);
#else
                ethsw_port_based_vlan(pDevCtrl->EnetInfo[0].sw.port_map,
                                      pDevCtrl->wanPort,
                                      0);
#endif
#endif
            TRACE(("Set %s wan port %d", dev->name, (int)data));
            val = 0;
            break;

        case SIOCGWANPORT:
            BCM_ENET_DEBUG("pDevCtrl->wanPort = 0x%x \n", pDevCtrl->wanPort);
            for (i = 0; i < MAX_SWITCH_PORTS-1; i++) {
#if defined(CONFIG_BCM96816)
                if ( (SERDES_PORT_ID == i) && (1 == bExt6829) )
                  continue;
#endif
                if ((pDevCtrl->wanPort >> i) & 0x1) {
                    if (phyport_to_vport[i] > 0) {
                        len = strlen((vnet_dev[phyport_to_vport[i]])->name);
                        if (atleast_one_added) {
                            wanifnames[cum_len] = ',';
                            cum_len += 1;
                        }
                        memcpy(wanifnames+cum_len, (vnet_dev[phyport_to_vport[i]])->name, len);
                        cum_len += len;
                        atleast_one_added = 1;
                    }
                }
            }
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
            for (i = 0; i < MAX_GPON_IFS; i++) {
                pNetDev = gponifid_to_dev[i];
                if (pNetDev != NULL) {
                    len = strlen(pNetDev->name);
                    if (atleast_one_added) {
                        wanifnames[cum_len] = ',';
                        cum_len += 1;
                    }
                    memcpy(wanifnames+cum_len, pNetDev->name, len);
                    cum_len += len;
                    atleast_one_added = 1;
                }
            }
#endif
#if defined(CONFIG_BCM96816)
            if ( 1 == bExt6829 )
            {
               for (i = 0; i < MAX_6829_IFS; i++)
               {
                   pNetDev = bcm6829_to_dev[i];
                   if (pNetDev != NULL)
                   {
                       if ((pDevCtrl->wanPort6829 >> pNetDev->base_addr) & 0x1)
                       {
                          len = strlen(pNetDev->name);
                          if (atleast_one_added)
                          {
                              wanifnames[cum_len] = ',';
                              cum_len += 1;
                          }
                          memcpy(wanifnames+cum_len, pNetDev->name, len);
                          cum_len += len;
                          atleast_one_added = 1;
                       }
                   }
               }
            }
#endif

            wanifnames[cum_len] = '\0';
            cum_len += 1;
            BCM_ENET_DEBUG("cum_len = %d \n", cum_len);
            if (copy_to_user((void*)chardata, (void*)wanifnames, cum_len))
                return -EFAULT;
            BCM_ENET_DEBUG("WAN interfaces: %s", chardata);
            val = 0;
            break;

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
        case SIOCGPONIF:
            BCM_ENET_DEBUG("The op is %d \n", g->op);
            BCM_ENET_DEBUG("The gem_map is %x \n", g->gem_map);
            BCM_ENET_DEBUG("The ifnum is %d \n", g->ifnumber);
            BCM_ENET_DEBUG("The ifname is %s \n", g->ifname);
            switch (g->op) {
                /* Add, Remove, and Show gem_ids */
                case GETFREEGEMIDMAP:
                case SETGEMIDMAP:
                case GETGEMIDMAP:
                val = set_get_gem_map(g->op, g->ifname, g->ifnumber,
                                      &g->gem_map);
                break;

                /* Create a gpon virtual interface */
                case CREATEGPONVPORT:
                val = create_gpon_vport(g->ifname);
                break;

                /* Delete the given gpon virtual interface */
                case DELETEGPONVPORT:
                val = delete_gpon_vport(g->ifname);
                break;

                /* Delete all gpon virtual interfaces */
                case DELETEALLGPONVPORTS:
                val = delete_all_gpon_vports();
                break;

                /* Set multicast gem index */
                case SETMCASTGEMID:
                val = set_mcast_gem_id(g->gem_map);
                break;

                default:
                val = -EOPNOTSUPP;
                break;
            }
            break;
#endif

       case SIOCETHSWCTLOPS:

            switch(e->op) {
                case ETHSWDUMPPAGE:
#if defined(CONFIG_BCM96816)
                    // make sure 6829 is present
                    if ( IsExt6829(e->page) && (0 == bExt6829))
                    {
                        printk("Invalid page or not yet implemented \n");
                        break;
                    }
#endif
                    BCM_ENET_DEBUG("ethswctl ETHSWDUMPPAGE ioctl");
                    ethsw_dump_page(e->page);
                    val = 0;
                    break;

                /* Print out enet iuDMA info - Aug 2010 */
                case ETHSWDUMPIUDMA:
                    {
                        BcmPktDma_LocalEthRxDma * rxdma;
                        int                       channel;

                        for(channel = 0; channel < ENET_RX_CHANNELS_MAX; channel++)
                        {
                            rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;
#if defined(CONFIG_BCM_PKTDMA_RX_SPLITTING)
                            if(rxdma->rxOwnership != HOST_OWNED) continue;
#endif
                            if(!rxdma->rxEnabled) continue;

                            printk("\n\nENET RXDMA STATUS Ch%d: HeadIndex: %d TailIndex: %d numRxBds: %d rxAssignedBds: %d rxToss: %u\n",
                                channel, rxdma->rxHeadIndex, rxdma->rxTailIndex,
                                rxdma->numRxBds, rxdma->rxAssignedBds, bcmenet_rxToss);

                            printk("     RXDMA CFG Ch%d: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n\n",
                                channel, rxdma->rxDma->cfg,
                                rxdma->rxDma->intStat,
                                rxdma->rxDma->intMask);
                        }
                    }

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) || defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
                    {
                        BcmPktDma_LocalEthTxDma * txdma;
                        int                       channel;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
                        {
                            channel = global.pVnetDev0_g->enetTxChannel;
#else
                        for(channel = 0; channel < ENET_TX_CHANNELS_MAX; channel++)
                        {
#endif
                            txdma = pDevCtrl->txdma[channel];
                            if(txdma->txEnabled)
                            {

                                printk("\nENET TXDMA STATUS Ch%d: HeadIndex: %d TailIndex: %d txFreeBds: %d BDs at: 0x%08x\n",
                                    channel, txdma->txHeadIndex,
                                    txdma->txTailIndex,
                                    txdma->txFreeBds,
                                    (unsigned int)&pDevCtrl->txdma[channel]->txBds[0]);

                                printk("     TXDMA CFG Ch%d: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n\n\n",
                                    channel, txdma->txDma->cfg,
                                    txdma->txDma->intStat,
                                    txdma->txDma->intMask);
                            }
                        }
                    }
#endif  /*  !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) || defined(CONFIG_BCM_PKTDMA_TX_SPLITTING) */
                    break;

                /* Get/Set the iuDMA rx channel for a specific eth port - Jan 2011 */
                case ETHSWIUDMASPLIT:
                    {
                        struct ethswctl_data e2;
                        int iudma_ch = e->val;
                        int retval = 0;
                        int j;

                        if(e->port < BP_MAX_SWITCH_PORTS)
                        {
                            if(TYPE_GET == e->type)
                            {
                                e2.type = TYPE_GET;
                                e2.port = e->port;
                                e2.priority = 0;
                                retval = enet_ioctl_ethsw_cosq_port_mapping(&e2);
                                if(retval >= 0)
                                {
                                    printk("eth%d mapped to iuDMA%d\n", e2.port, retval);
                                    return(0);
                                }
                            }
                            else if(iudma_ch < ENET_RX_CHANNELS_MAX)
                            {   /* TYPE_SET */
                                /* The equivalent of "ethswctl -c cosq -p port -q {j} -v {iudma_ch}" */
                                /* This routes packets of all priorities on eth 'port' to egress queue 'iudma_ch' */
                                e2.port = e->port;
                                for(j = 0; j <= MAX_PRIORITY_VALUE; j++)
                                {
                                    e2.type = TYPE_SET;
                                    e2.priority = j;
                                    e2.queue = iudma_ch;
                                    mapEthPortToRxIudma(e2.port, e2.queue);
                                    retval += enet_ioctl_ethsw_cosq_port_mapping(&e2);
                                }
                                if(retval == 0)
                                {
                                    printk("eth%d mapped to iuDMA%d\n", e->port, iudma_ch);
                                    return(0);
                                }
                            }
                            else
                                printk("Invalid iuDMA channel number %d\n", iudma_ch);
                        }
                        else
                            printk("Invalid Ethernet port number %d\n", e->port);
                    }
                    return(BCM_E_ERROR);
                    break;

                case ETHSWDUMPMIB:
#if defined(CONFIG_BCM96816)
                    // make sure 6829 is present
                    if ( IsExt6829(e->port) && (0 == bExt6829))
                    {
                        printk("Invalid port number \n");
                        break;
                    }
#endif
                    BCM_ENET_DEBUG("ethswctl ETHSWDUMPMIB ioctl");
                    val = ethsw_dump_mib(e->port, e->type);
                    break;

                case ETHSWSWITCHING:
                    BCM_ENET_DEBUG("ethswctl ETHSWSWITCHING ioctl");
                    if (e->type == TYPE_ENABLE) {
                        val = ethsw_enable_hw_switching();
                    } else if (e->type == TYPE_DISABLE) {
                        val = ethsw_disable_hw_switching();
                    } else {
                        val = ethsw_get_hw_switching_state();
                        if (copy_to_user((void*)(&e->status), (void*)&val,
                            sizeof(int))) {
                            return -EFAULT;
                        }
                        val = 0;
                    }
                    break;

                case ETHSWRXSCHEDULING:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXSCHEDULING ioctl");
                    return enet_ioctl_ethsw_rxscheduling(e);
                    break;

                case ETHSWWRRPARAM:
                    BCM_ENET_DEBUG("ethswctl ETHSWWRRPARAM ioctl");
                    return enet_ioctl_ethsw_wrrparam(e);
                    break;

                case ETHSWUSEDEFTXQ:
                    BCM_ENET_DEBUG("ethswctl ETHSWUSEDEFTXQ ioctl");
                    pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    return enet_ioctl_use_default_txq_config(pDevCtrl,e);
                    break;

                case ETHSWDEFTXQ:
                    BCM_ENET_DEBUG("ethswctl ETHSWDEFTXQ ioctl");
                    pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    return enet_ioctl_default_txq_config(pDevCtrl, e);
                    break;

#if defined(RXCHANNEL_BYTE_RATE_LIMIT) && defined(CONFIG_BCM96816)
                case ETHSWRXRATECFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATECFG ioctl");
                    return enet_ioctl_rx_rate_config(e);
                    break;

                case ETHSWRXRATELIMITCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATELIMITCFG ioctl");
                    return enet_ioctl_rx_rate_limit_config(e);
                    break;
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
                case ETHSWRXPKTRATECFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATECFG ioctl");
                    return enet_ioctl_rx_pkt_rate_config(e);
                    break;

                case ETHSWRXPKTRATELIMITCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATELIMITCFG ioctl");
                    return enet_ioctl_rx_pkt_rate_limit_config(e);
                    break;
#endif

                case ETHSWTEST1:
                    BCM_ENET_DEBUG("ethswctl ETHSWTEST1 ioctl");
                    enet_ioctl_test_config(e);
                    break;

#ifdef CONFIG_BCM96816
                case ETHSWPORTTAGREPLACE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTAGREPLACE ioctl");
                    return enet_ioctl_ethsw_port_tagreplace(e);
                    break;

                case ETHSWPORTTAGMANGLE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPPORTTAGMANGLE ioctl");
                    return enet_ioctl_ethsw_port_tagmangle(e);
                    break;

                case ETHSWPORTTAGMANGLEMATCHVID:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTAGMANGLEMATCHVID ioctl");
                    return enet_ioctl_ethsw_port_tagmangle_matchvid(e);
                    break;

                case ETHSWPORTTAGSTRIP:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTAGSTRIP ioctl");
                    return enet_ioctl_ethsw_port_tagstrip(e);
                    break;
#endif

                case ETHSWPORTPAUSECAPABILITY:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTPAUSECAPABILITY ioctl");
                    return enet_ioctl_ethsw_port_pause_capability(e);
                    break;

                case ETHSWCONTROL:
                    BCM_ENET_DEBUG("ethswctl ETHSWCONTROL ioctl");
                    return enet_ioctl_ethsw_control(e);
                    break;

                case ETHSWPRIOCONTROL:
                    BCM_ENET_DEBUG("ethswctl ETHSWPRIOCONTROL ioctl");
                    return enet_ioctl_ethsw_prio_control(e);
                    break;

                case ETHSWVLAN:
                    BCM_ENET_DEBUG("ethswctl ETHSWVLAN ioctl");
                    return enet_ioctl_ethsw_vlan(e);
                    break;

#ifdef BCM_ENET_DEBUG_BUILD
                case ETHSWGETRXCOUNTERS:
                    BCM_ENET_DEBUG("ethswctl ETHSWGETRXCOUNTERS ioctl");
                    return enet_ioctl_getrxcounters();
                    break;

                case ETHSWRESETRXCOUNTERS:
                    BCM_ENET_DEBUG("ethswctl ETHSWRESETRXCOUNTERS ioctl");
                    return enet_ioctl_setrxcounters();
                    break;
#endif

                case ETHSWPBVLAN:
                    BCM_ENET_DEBUG("ethswctl ETHSWPBVLAN ioctl");
                    return enet_ioctl_ethsw_pbvlan(e);
                    break;

                case ETHSWCOSCONF:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSCONF ioctl");
                    return enet_ioctl_ethsw_cosq_config(e);
                    break;

                case ETHSWCOSSCHED:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSSCHED ioctl");
                    return enet_ioctl_ethsw_cosq_sched(e);
                    break;

                case ETHSWCOSPORTMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSMAP ioctl");
                    val = enet_ioctl_ethsw_cosq_port_mapping(e);
                    if(val < 0) {
                        if(-BCM_E_ERROR == val)
                            return BCM_E_ERROR;
                        return(val);
                    }
                    if(e->type == TYPE_GET) {
                        /* queue returned from function. Return value to user */
                        if (copy_to_user((void*)(&e->queue), (void*)&val, sizeof(int))) {
                            return -EFAULT;
                        }
                    }
                    else if(e->type == TYPE_SET) {
                        mapEthPortToRxIudma(e->port, e->queue);
                    }
                    return 0;
                    break;

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM96816)
                case ETHSWCOSRXCHMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXCOSCHMAP ioctl");
                    return enet_ioctl_ethsw_cosq_rxchannel_mapping(e);
                    break;

                case ETHSWCOSTXCHMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSTXCHMAP ioctl");
                    return enet_ioctl_ethsw_cosq_txchannel_mapping(e);
                    break;
#endif

                case ETHSWCOSTXQSEL:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSTXQSEL ioctl");
                    return enet_ioctl_ethsw_cosq_txq_sel(e);
                    break;

                case ETHSWSTATCLR:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATINIT ioctl");
                    return enet_ioctl_ethsw_clear_stats
                        ((uint32_t)pDevCtrl->EnetInfo[0].sw.port_map);
                    break;

                case ETHSWSTATPORTCLR:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATCLEAR ioctl");
                    return enet_ioctl_ethsw_clear_port_stats(e);
                    break;

                case ETHSWSTATSYNC:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATSYNC ioctl");
                    return ethsw_counter_collect
                        ((uint32_t)pDevCtrl->EnetInfo[0].sw.port_map, 0);
                    break;

                case ETHSWSTATGET:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATGET ioctl");
                    return enet_ioctl_ethsw_counter_get(e);
                    break;

                case ETHSWPORTRXRATE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTRXRATE ioctl");
                    if (e->type == TYPE_GET) {
                        return enet_ioctl_ethsw_port_irc_get(e);
                    } else {
                        return enet_ioctl_ethsw_port_irc_set(e);
                    }
                    break;

                case ETHSWPORTTXRATE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTXRATE ioctl");
                    if (e->type == TYPE_GET) {
                        return enet_ioctl_ethsw_port_erc_get(e);
                    } else {
                        return enet_ioctl_ethsw_port_erc_set(e);
                    }
                    break;

#ifdef CONFIG_BCM96816
                case ETHSWPKTPAD:
                    BCM_ENET_DEBUG("ethswctl ETHSWPKTPAD ioctl");
                    return enet_ioctl_ethsw_pkt_padding(e);
                    break;
#endif /*CONFIG_BCM96816*/

                case ETHSWJUMBO:
                    BCM_ENET_DEBUG("ethswctl ETHSWJUMBO ioctl");
                    return enet_ioctl_ethsw_port_jumbo_control(e);
                    break;

                case ETHSWPORTTRAFFICCTRL:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTRAFFICCTRL ioctl");
                    return enet_ioctl_ethsw_port_traffic_control(e);
                    break;

                case ETHSWPORTLOOPBACK:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTLOOPBACK ioctl");
                    phy_id = pDevCtrl->EnetInfo[0].sw.phy_id[e->port];
                    return enet_ioctl_ethsw_port_loopback(e, phy_id);
                    break;

                case ETHSWARLACCESS:
                    BCM_ENET_DEBUG("ethswctl ETHSWARLACCESS ioctl");
                    return enet_ioctl_ethsw_arl_access(e);
                    break;

                case ETHSWPORTDEFTAG:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTDEFTAG ioctl");
                    return enet_ioctl_ethsw_port_default_tag_config(e);
                    break;

                case ETHSWCOSPRIORITYMETHOD:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSPRIORITYMETHOD ioctl");
                    return enet_ioctl_ethsw_cos_priority_method_config(e);
                    break;

                case ETHSWCOSDSCPPRIOMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSDSCPPRIOMAP ioctl");
                    return ethsw_dscp_to_priority_mapping(e);
                    break;

                case ETHSWREGACCESS:
                    BCM_ENET_DEBUG("ethswctl ETHSWREGACCESS ioctl");
                    val = enet_ioctl_ethsw_regaccess(e);
                    break;

                case ETHSWSPIACCESS:
                    BCM_ENET_DEBUG("ethswctl ETHSWSPIACCESS ioctl");
                    val = enet_ioctl_ethsw_spiaccess(global.pVnetDev0_g->extSwitch->bus_num, 
                        global.pVnetDev0_g->extSwitch->spi_ss, global.pVnetDev0_g->extSwitch->spi_cid, e);
                    break;

                case ETHSWPSEUDOMDIOACCESS:
                    BCM_ENET_DEBUG("ethswctl ETHSWPSEUDOMDIOACCESS ioctl");
                    val = enet_ioctl_ethsw_pmdioaccess(dev, e);
                    break;

                case ETHSWINFO:
                    BCM_ENET_DEBUG("ethswctl ETHSWINFO ioctl");
                    val = enet_ioctl_ethsw_info(dev, e);
                    break;

                case ETHSWLINKSTATUS:
                    BCM_ENET_DEBUG("ethswctl ETHSWLINKSTATUS ioctl");
                    swPort = e->port;
#if defined(CONFIG_BCM963268)
                    if ( (extSwInfo.present == 1) && (e->unit == 0) )
                        swPort += MAX_EXT_SWITCH_PORTS;
#endif
                    link_change_handler(swPort, e->status, e->speed, e->duplex);
                    val = 0;
                    break;

#if defined(SUPPORT_SWMDK)
                case ETHSWKERNELPOLL:
                    BCM_ENET_DEBUG("ethswctl ETHSWKERNELPOLL ioctl");
                    val = enet_ioctl_kernel_poll();
                    break;
#endif

                case ETHSWPHYCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWLINKSTATUS ioctl");
                    val = enet_ioctl_phy_cfg_get(dev, e);
                    break;

#if defined(CONFIG_BCM96816)
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)

                case ETHSWMOCASOFTSWITCH:
                    BCM_ENET_DEBUG("ethswctl ETHSWMOCASOFTSWITCH ioctl");
                    val = enet_ioctl_moca_soft_switch(e);
                    break;

                case ETHSWMOCAXMITBUDGET:
                    BCM_ENET_DEBUG("ethswctl ETHSWMOCAXMITBUDGET ioctl");
                    val = enet_ioctl_moca_xmit_budget(e);
                    break;

                case ETHSWMOCAPORTCHECK:
                    BCM_ENET_DEBUG("ethswctl ETHSWMOCAPORTCHECK ioctl");
                    val = enet_ioctl_moca_port_check(e);
                    break;

                case ETHSWSOFTSWITCHSTARTQ:
                    BCM_ENET_DEBUG("ethswctl ETHSWSOFTSWITCHSTARTQ ioctl");
                    val = enet_ioctl_soft_switch_start_queue(e);
                    break;
#endif
#endif
                case ETHSWPHYMODE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPHYMODE ioctl");
                    phy_id = pDevCtrl->EnetInfo[0].sw.phy_id[e->port];
                    val = enet_ioctl_ethsw_phy_mode(e, phy_id);
                    return val;
                    break;

                default:
                    BCM_ENET_DEBUG("ethswctl unsupported ioctl");
                    val = -EOPNOTSUPP;
                    break;
            }
            break;

        case SIOCETHCTLOPS:
            switch(ethctl->op) {
                case ETHGETNUMTXDMACHANNELS:
                    ethctl->ret_val = cur_txdma_channels;
                    val = 0;
                    break;

                case ETHSETNUMTXDMACHANNELS:
                    if (ethctl->num_channels <= ENET_TX_CHANNELS_MAX) {
                        if (ethctl->num_channels > 1) {
                            printk("Warning: If the DUT does not support "
                                    "un-aligned Tx buffers, you should not be "
                                    "doing this!!! \n");
                            printk("Continuing with set_txdma_channels... \n");
                        }
                        if (set_cur_txdma_channels(ethctl->num_channels)) {
                            printk("Error in setting cur_txdma_channels \n");
                            return -EFAULT;
                        }
                        val = 0;
                    } else {
                        printk("Max: %d \n", ENET_TX_CHANNELS_MAX);
                        val = -EINVAL;
                    }
                    break;

                case ETHGETNUMRXDMACHANNELS:
                    ethctl->ret_val = cur_rxdma_channels;
                    val = 0;
                    break;

                case ETHSETNUMRXDMACHANNELS:
                    if (ethctl->num_channels <= ENET_RX_CHANNELS_MAX) {
                        if (ethctl->num_channels < ENET_RX_CHANNELS_MAX) {
                            printk("Warning: The switch buffers will fill up "
                                    "if the switch configuration is not modified "
                                    "to not to send packets on disabled rx dma "
                                    "channels!!! \n");
                            printk("Continuing with set_rxdma_channels... \n");
                        }
                        if (set_cur_rxdma_channels(ethctl->num_channels)) {
                            printk("Error in setting cur_rxdma_channels \n");
                            return -EFAULT;
                        }
                        val = 0;
                    } else {
                        printk("Max: %d \n", ENET_RX_CHANNELS_MAX);
                        val = -EINVAL;
                    }
                    break;

                case ETHGETSOFTWARESTATS:
                    pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    display_software_stats(pDevCtrl);
                    val = 0;
                    break;

                case ETHSETSPOWERUP:
                    swPort = port_id_from_dev(dev);
                    ethsw_switch_manage_port_power_mode(swPort, 1);
                    val = 0;
                    break;

                case ETHSETSPOWERDOWN:
                    swPort = port_id_from_dev(dev);
                    ethsw_switch_manage_port_power_mode(swPort, 0);
                    val = 0;
                    break;

                case ETHGETMIIREG:       /* Read MII PHY register. */
                    BCM_ENET_DEBUG("phy_id: %d; reg_num = %d \n", ethctl->phy_addr, ethctl->phy_reg);
                    {
                        uint16 val;
                        down(&bcm_ethlock_switch_config);
                        ethsw_phy_read_reg(ethctl->phy_addr & 0x1f, ethctl->phy_reg & 0x1f, 
                            &val, (ethctl->flags & ETHCTL_FLAG_ACCESS_EXT_PHY)?1:0);
                        up(&bcm_ethlock_switch_config);
                        ethctl->ret_val = val;
                    }
                    break;

                case ETHSETMIIREG:       /* Write MII PHY register. */
                    BCM_ENET_DEBUG("phy_id: %d; reg_num = %d; val = 0x%x \n", ethctl->phy_addr,
                            ethctl->phy_reg, ethctl->val);
                    {
                        uint16 val = ethctl->val;
                        down(&bcm_ethlock_switch_config);
                        ethsw_phy_write_reg(ethctl->phy_addr & 0x1f, ethctl->phy_reg & 0x1f, 
                            &val, (ethctl->flags & ETHCTL_FLAG_ACCESS_EXT_PHY)?1:0);
                        up(&bcm_ethlock_switch_config);
                    }
                    break;

                default:
                    val = -EOPNOTSUPP;
                    break;
            }
            break;
#if defined(AEI_VDSL_TOOLBOX)
        case SIOCPORTMIRROR:
            if(copy_from_user((void*)&mirrorCfg,data,sizeof(MirrorCfg)))
                return -EFAULT;
            else
            {
                BcmEnet_devctrl *pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);

                if( mirrorCfg.nDirection == MIRROR_DIR_IN )
                {
                    AEI_UpdateMirrorFlag(&(pEthCtrl->usMirrorInFlags),
                              mirrorCfg.szMirrorInterface, mirrorCfg.nStatus);
                }
                else /* MIRROR_DIR_OUT */
                {
                    AEI_UpdateMirrorFlag(&(pEthCtrl->usMirrorOutFlags),
                              mirrorCfg.szMirrorInterface, mirrorCfg.nStatus);
                }
            }
            break;
#endif
#if defined(AEI_VDSL_POWERSAVE)
        case SIOCPOWERSAVE:
        {
            PowerSaveCfg cfg;

            if (copy_from_user((void*)&cfg, data, sizeof(PowerSaveCfg)))
                return -EFAULT;
            else
            {
                if ((cfg.enable != pwsCfg.enable) ||
                    (cfg.mode != pwsCfg.mode))
                {
                    if (!cfg.enable)
                        AEI_ethsw_disable_pws();
                    else
                        AEI_ethsw_set_pws_mode(cfg.mode, TRUE);

                    pwsCfg.enable = cfg.enable;
                    pwsCfg.mode = cfg.mode;
                }
            }
            break;
        }
#endif
#if defined(AEI_VDSL_HPNA)
        case SIOCGHPNALINKSTATUS:
        {
            PHY_STAT phys;
            int link_status;

            phys = AEI_ethsw_hpna_phy_stat(hpna_port);
            link_status = phys.lnk;

            if (copy_to_user((void *)data, (void *)&link_status, sizeof(int)))
            {
                return -EFAULT;
            }
            val = 0;
            break;
        }
        case SIOCSHPNALINKSTATUS:
        {
            int link_status;

            if (copy_from_user((void*)&link_status, data, sizeof(int)))
                return -EFAULT;
            else
            {
                if (link_status)
                {
                    if (netif_carrier_ok(vnet_dev[hpna_dev_index]) == 0)
                        netif_carrier_on(vnet_dev[hpna_dev_index]);
                }
                else
                {
                    if (netif_carrier_ok(vnet_dev[hpna_dev_index]) != 0)
                        netif_carrier_off(vnet_dev[hpna_dev_index]);
                }
            }
            break;
        }
        case SIOCSHPNAADMSTATE:
            {
                int *state = (int *)&rq->ifr_data;

                hpna_admin_state = state[0];
                hpna_mode = state[1];

                /* C2000 has HPNA and WAN eth tied to internal switch, so do not do this on C2000. 
                   Also, setting port 5 to 100 Mbps is going to make a lot of pause frames come out and stop traffic.                   
                   Actually seems like someone already copied similar functionality in ethsw_init_config() so might not have
                   to do this at all for R1000/V1000/etc... 
                 */
#if defined(AEI_63168_CHIP) 
                ;
#else
                //flowcontrol defer until have some more time to test
                if (hpna_admin_state) {
                    uint32 val = 0;

                    uint8 v8 = REG_PORT_STATE_LNK | REG_PORT_STATE_FDX | REG_PORT_STATE_100 | REG_PORT_STATE_FLOWCTL | REG_PORT_STATE_OVERRIDE;

                    extsw_wreg(PAGE_CONTROL, REG_PORT5_STATE, (uint8 *)&v8, 1);

                    extsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8 *)&val, sizeof(val));

                    val = swab32(val);

                    val |= (REG_PAUSE_CAPBILITY_OVERRIDE | 
                    REG_PAUSE_CAPBILITY_MIPS_TX | //need this to improve hpna throughput but doing this will mean no eth Qos!!!
                                                  //w/o it,throughput from 53115 into hpna cannot get above 65Mbps for large packet sizes > 512
                                                  //Even with it, throughput for >1 stream from 53115 into hpna is lower than 1 stream...

                    //REG_PAUSE_CAPBILITY_MIPS_RX | //this can stop driver from receiving packets when ewan pps is very high
                    REG_PAUSE_CAPBILITY_GMII1_TX | REG_PAUSE_CAPBILITY_GMII1_RX

                    //need this to cut wan/BA ftp server side error
                    | REG_PAUSE_CAPBILITY_GMII0_TX 

                    //need this to cut lan/BA ftp client side errors
                    |  REG_PAUSE_CAPBILITY_EPHY0_TX |   
                    REG_PAUSE_CAPBILITY_EPHY1_TX |
                    REG_PAUSE_CAPBILITY_EPHY2_TX |
                    REG_PAUSE_CAPBILITY_EPHY3_TX 
                    );

                    val = swab32(val);
                    extsw_wreg(PAGE_CONTROL,REG_PAUSE_CAPBILITY, (uint8 *)&val, sizeof(val));
                }
                else {
                    uint32 val = 0;

                    extsw_rreg(PAGE_CONTROL,REG_PAUSE_CAPBILITY, (uint8 *)&val, sizeof(val));
                    val = swab32(val);

                    val &=~(REG_PAUSE_CAPBILITY_GMII1_RX);
                    val |= REG_PAUSE_CAPBILITY_OVERRIDE;
                    val = swab32(val);

                    extsw_wreg(PAGE_CONTROL,REG_PAUSE_CAPBILITY, (uint8 *)&val, sizeof(val));
                }
#endif
            }

            val = 0;
            break;
        case SIOCGHPNAADMSTATE:
            if (copy_to_user((void*)data, (void*)&hpna_admin_state, sizeof(int)))
                return -EFAULT;
            val = 0;
            break;
#endif
#if defined(AEI_VDSL_STATS_DIAG)
        case SIOCGETDEVSTATS:
        {
            BcmEnet_devctrl *pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);

            bcm63xx_enet_query(dev);

            if (copy_to_user((void *)data, (void *)&pEthCtrl->dev_stats,
                             sizeof(struct enet_dev_stats)))
            {
                return -EFAULT;
            }
            val = 0;
            break;
        }
#endif
        default:
            val = -EOPNOTSUPP;
            break;
    }

    return val;
}



static int set_cur_txdma_channels(int num_channels)
{
    int i, j, tmp_channels;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
  #if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING) || defined(CONFIG_BCM963268)
    if (num_channels != ENET_TX_CHANNELS_MAX) {
  #else
    if (num_channels != 1) {
  #endif
        BCM_LOG_ERROR(BCM_LOG_ID_ENET, "Invalid number of Tx channels : %u\n",
                      num_channels);
        return -EINVAL;
    }
#endif

    if (cur_txdma_channels == num_channels) {
        BCM_ENET_DEBUG("Not changing current txdma channels"
                       "as it is same as what is given \n");
        return 0;
    }
    if (num_channels > ENET_TX_CHANNELS_MAX) {
        BCM_ENET_DEBUG("Not changing current txdma channels"
                       "as it is greater than max (%d) \n",ENET_TX_CHANNELS_MAX);
        return 0;
    }

    /* Increasing the number of Tx channels */
    if (num_channels > cur_txdma_channels) {
        /* Initialize the new channels */
        for (i = cur_txdma_channels; i < num_channels; i++) {
            if (init_tx_channel(pDevCtrl, i)) {
                for (j = cur_txdma_channels; j < i; j++) {
                    uninit_tx_channel(pDevCtrl, j);
                }
                return -1;
            }
        }

        for (i = cur_txdma_channels; i < num_channels; i++) {
            bcmPktDma_EthTxEnable(pDevCtrl->txdma[i]);
        }

        /* Set the current Tx DMA channels to given num_channels */
        cur_txdma_channels = num_channels;

    } else { /* Decreasing the number of Tx channels */
        for (i = num_channels; i < cur_txdma_channels; i++) {
            bcmPktDma_EthTxDisable(pDevCtrl->txdma[i]);
        }

        /* Remember the cur_txdma_channels as we are changing it now */
        tmp_channels = cur_txdma_channels;

        /* Set the current Tx DMA channels to given num_channels */
        cur_txdma_channels = num_channels;

        /*Un-allocate the BD ring */
        for (i = num_channels; i < tmp_channels; i++) {
            uninit_tx_channel(pDevCtrl, i);
        }
    }

    return 0;
}

static int set_cur_rxdma_channels(int num_channels)
{
    int i, j, tmp_channels, total_of_weights = 0;
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    if (cur_rxdma_channels == num_channels) {
        BCM_ENET_DEBUG("Not changing current rxdma channels"
                       "as it is same as what is given \n");
        return 0;
    }
    if (num_channels > ENET_RX_CHANNELS_MAX) {
        BCM_ENET_DEBUG("Not changing current rxdma channels"
                       "as it is greater than MAX (%d) \n",ENET_RX_CHANNELS_MAX);
        return 0;
    }

    /* Increasing the number of Rx channels */
    if (num_channels > cur_rxdma_channels) {
        for (i = cur_rxdma_channels; i < num_channels; i++) {
            /* Init the Rx Channel. */
            if (init_rx_channel(pDevCtrl, i)) {
                for (j = cur_rxdma_channels; j < i; j++) {
                    uninit_rx_channel(pDevCtrl, j);
                }
                return -1;
            }
        }

        for (i = cur_rxdma_channels; i < num_channels; i++) {
            rxdma = pDevCtrl->rxdma[i];
            bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);
            bcmPktDma_BcmHalInterruptEnable(i, rxdma->rxIrq);
            rxdma->pktDmaRxInfo.rxDma->cfg |= DMA_ENABLE;
        }

        /* Set the current Rx DMA channels to given num_channels */
        cur_rxdma_channels = num_channels;

    } else { /* Decreasing the number of Rx channels */
        /* Stop the DMA channels */
        for (i = num_channels; i < cur_rxdma_channels; i++) {
            rxdma = pDevCtrl->rxdma[i];
            rxdma->pktDmaRxInfo.rxDma->cfg = 0;
        }

        /* Disable the interrupts */
        for (i = num_channels; i < cur_rxdma_channels; i++) {
            rxdma = pDevCtrl->rxdma[i];
            bcmPktDma_BcmHalInterruptDisable(i, rxdma->rxIrq);
            bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);
        }

        /* Remember the cur_rxdma_channels as we are changing it now */
        tmp_channels = cur_rxdma_channels;

        /* Set the current Rx DMA channels to given num_channels */
        /* Set this before unint_rx_channel, so that ISR will not
           try to service a channel which is uninitialized. */
        cur_rxdma_channels = num_channels;

        /* Free the buffers and BD ring */
        for (i = num_channels; i < tmp_channels; i++) {
            uninit_rx_channel(pDevCtrl, i);
        }
    }

    /* Recalculate the WRR weights based on cur_rxdma_channels */
    for(i=0; i<cur_rxdma_channels; i++) {
        total_of_weights += weights[i];
    }
    for(i=0; i<cur_rxdma_channels; i++) {
        weight_pkts[i] = (max_pkts/total_of_weights) * weights[i];
        pending_weight_pkts[i] = weight_pkts[i];
        BCM_ENET_DEBUG("weight[%d]_pkts: %d \n", i, weight_pkts[i]);
        pending_channel[i] = i;
    }
    global_channel = channel_ptr = loop_index = 0;
    pending_ch_tbd = cur_rxdma_channels;

    return 0;
}

/*
 * init_rx_channel: Initialize Rx DMA channel
 */
static int init_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    DmaStateRam *StateRam;
    BcmEnet_RxDma *rxdma;

    TRACE(("bcm63xxenet: init_rx_channel\n"));
    BCM_ENET_DEBUG("Initializing Rx channel %d \n", channel);

    /* setup the RX DMA channel */
    rxdma = pDevCtrl->rxdma[channel];

    /* Reset the DMA channel */
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 1 << (channel * 2);
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 0;

    /* allocate RX BDs */
    if (bcm63xx_alloc_rxdma_bds(channel,pDevCtrl) < 0)
    {
        return -1;
    }
    /*
     * clear State RAM of DMA channels
     */
    StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[channel*2];
    memset(StateRam, 0, sizeof(DmaStateRam));

    /* setup receive dma register */
    rxdma->pktDmaRxInfo.rxDma->cfg = 0;  /*initialize first (will enable later)*/
    rxdma->pktDmaRxInfo.rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
    rxdma->pktDmaRxInfo.rxDma->intMask = 0;   /* mask all ints */
    /* clr any pending interrupts on channel */
    rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    /* set to interrupt on packet complete and no descriptor available */
    rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    pDevCtrl->dmaCtrl->stram.s[channel * 2].baseDescPtr =
        (uint32)VIRT_TO_PHY((uint32 *)rxdma->pktDmaRxInfo.rxBds);

    bcmPktDma_EthInitRxChan(rxdma->pktDmaRxInfo.numRxBds, &rxdma->pktDmaRxInfo);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    enet_rx_set_iq_thresh( pDevCtrl, channel );
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    enet_rx_set_bpm_alloc_trig( pDevCtrl, channel );
#endif

    if (init_buffers(pDevCtrl, channel)) {
        printk(KERN_NOTICE CARDNAME": Low memory.\n");
        uninit_buffers(pDevCtrl->rxdma[channel]);
        return -1;
    }
    return 0;
}

/*
 * uninit_rx_channel: un-initialize Rx DMA channel
 */
void uninit_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    BcmEnet_RxDma *rxdma;

    TRACE(("bcm63xxenet: init_rx_channel\n"));
    BCM_ENET_DEBUG("un-initializing Rx channel %d \n", channel);

    /* setup the RX DMA channel */
    rxdma = pDevCtrl->rxdma[channel];

    /* Reset the DMA channel */
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 1 << (channel * 2);
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 0;

#if !defined(ENET_RX_BDS_IN_PSM)
    /* remove the rx bd ring & rxBdsStdBy */
    if (rxdma->pktDmaRxInfo.rxBdsBase) {
        kfree((void *)rxdma->pktDmaRxInfo.rxBdsBase);
    }
#endif

    uninit_buffers(rxdma);
}


/*
 * init_tx_channel: Initialize Tx DMA channel
 */
static int init_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    DmaStateRam *StateRam;
    BcmPktDma_EthTxDma *txdma;

    TRACE(("bcm63xxenet: init_txdma\n"));
    BCM_ENET_DEBUG("Initializing Tx channel %d \n", channel);

    /* allocate and assign tx buffer descriptors */
    txdma = pDevCtrl->txdma[channel];

    /* Reset the DMA channel */
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 1 << ((channel * 2) + 1);
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 0;

    /* allocate TX BDs */
    if (bcm63xx_alloc_txdma_bds(channel,pDevCtrl) < 0)
    {
        return -1;
    }

    /*
     * clear State RAM of DMA channels
     */
    StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[(channel * 2) + 1];
    memset(StateRam, 0, sizeof(DmaStateRam));

    /* setup the TX DMA channel */
    txdma = pDevCtrl->txdma[channel];
    /* setup transmit dma register */
    txdma->txDma->cfg = 0; /*initialize first (will enable later)*/
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH | DMA_DESCSIZE_SEL;
#else
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH;
#endif
    txdma->txDma->intMask = 0;   /* mask all ints */
    pDevCtrl->dmaCtrl->stram.s[(channel * 2) + 1].baseDescPtr =
        (uint32)VIRT_TO_PHY((uint32 *)txdma->txBds);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    enet_bpm_set_tx_drop_thr( pDevCtrl, channel );
#endif
#endif

    return 0;
}

/*
 * uninit_tx_channel: un-initialize Tx DMA channel
 */
void uninit_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    BcmPktDma_EthTxDma *txdma;

    TRACE(("bcm63xxenet: uninit_tx_channel\n"));
    BCM_ENET_DEBUG("un-initializing Tx channel %d \n", channel);

    /* setup the RX DMA channel */
    txdma = pDevCtrl->txdma[channel];

    /* Reset the DMA channel */
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 1 << ((channel * 2) + 1);
    pDevCtrl->dmaCtrl->ctrl_channel_reset = 0;

#if !defined(ENET_TX_BDS_IN_PSM)
    /* remove the tx bd ring */
    if (txdma->txBdsBase) {
        kfree((void *)txdma->txBdsBase);
    }
#endif
}


#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))

static const char* gponif_name = "gpon%d";

/****************************************************************************/
/* Create a new GPON Virtual Interface                                      */
/* Inputs: name = the name for the gpon i/f. If not specified, a name of    */
/*          gponXX will be assigned where XX is the next available number   */
/* Returns: 0 on success; a negative value on failure                       */
/* Notes: 1. The max num gpon virtual interfaces is limited to MAX_GPON_IFS.*/
/****************************************************************************/
static int create_gpon_vport(char *name)
{
    struct net_device *dev;
    struct sockaddr sockaddr;
    BcmEnet_devctrl *pDevCtrl = NULL;
    int status, ifid = 0;
    PHY_STAT phys;

    phys.lnk = 0;
    phys.fdx = 0;
    phys.spd1000 = 0;
    phys.spd100 = 0;

    /* Verify that given name is valid */
    if (name[0] != 0) {
        if (!dev_valid_name(name)) {
            printk("The given interface name is invalid \n");
            return -EINVAL;
        }

        /* Verify that no interface exists with this name */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
        dev = dev_get_by_name(&init_net, name);
#else
        dev = dev_get_by_name(name);
#endif
        if (dev != NULL) {
            dev_put(dev);
            printk("The given interface already exists \n");
            return -EINVAL;
        }
    }

    /* Find a free id for the gponif */
    for (ifid = 0; ifid < MAX_GPON_IFS; ifid++) {
        if (gponifid_to_dev[ifid] == NULL)
            break;
    }
    /* No free id is found. We can't create a new gpon virtual i/f */
    if (ifid == MAX_GPON_IFS) {
        printk("Create Failed as the number of gpon interfaces is "
               "limited to %d\n", MAX_GPON_IFS);
        return -EPERM;
    }

    /* Allocate the dev */
    if ((dev = alloc_etherdev(sizeof(BcmEnet_devctrl))) == NULL) {
        return -ENOMEM;
    }
    /* Set the private are to 0s */
    memset(netdev_priv(dev), 0, sizeof(BcmEnet_devctrl));

    /* Set the pDevCtrl->dev to dev */
    pDevCtrl = netdev_priv(dev);
    pDevCtrl->dev = dev;

    /* Assign name to the i/f */
    if (name[0] == 0) {
        /* Allocate and assign a name to the i/f */
        dev_alloc_name(dev, gponif_name);
    } else {
        /* Assign the given name to the i/f */
        strcpy(dev->name, name);
    }

    SET_MODULE_OWNER(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    dev->netdev_ops       = vnet_dev[0]->netdev_ops;
#else
    /* Set the dev fields same as the GPON port */
    dev->open             = vnet_dev[0]->open;
    dev->stop             = vnet_dev[0]->stop;
    dev->hard_start_xmit  = (HardStartXmitFuncP)vnet_dev[0]->hard_start_xmit;
    dev->tx_timeout       = vnet_dev[0]->tx_timeout;
    dev->set_mac_address  = vnet_dev[0]->set_mac_address;
    dev->do_ioctl         = vnet_dev[0]->do_ioctl;
    dev->get_stats        = vnet_dev[0]->get_stats;
#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    dev->priv_flags       = vnet_dev[0]->priv_flags;

    /* Set this flag to block forwarding of traffic between
       GPON virtual interfaces */
    dev->priv_flags       |= IFF_WANDEV;

    /* For now, let us use this base_addr field to identify GPON port */
    /* TBD: Change this to a private field in pDevCtrl for all eth and gpon
       virtual ports */
    dev->base_addr        = GPON_PORT_ID;
    pDevCtrl->sw_port_id  = GPON_PORT_ID;

    netdev_path_set_hw_port(dev, GPON_PORT_ID, BLOG_GPONPHY);

    /* Set the default tx queue to 0 */
    pDevCtrl->default_txq = 0;
    pDevCtrl->use_default_txq = 0;

    /* The unregister_netdevice will call the destructor
       through netdev_run_todo */
    dev->destructor        = free_netdev;

    /* Note: Calling from ioctl, so don't use register_netdev
       which takes rtnl_lock */
    status = register_netdevice(dev);

    if (status != 0) {
        unregister_netdevice(dev);
        return status;
    }

    /* Indicate that ifid is used */
    freeid_map[ifid] = 1;

    /* Store the dev pointer at the index given by ifid */
    gponifid_to_dev[ifid] = dev;

    /* Store the ifid in dev private area */
    pDevCtrl->gponifid = ifid;

#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
    status = kerSysGetMacAddress(dev->dev_addr, dev->ifindex);
    if (status < 0)
#endif
    {
        memmove(dev->dev_addr, vnet_dev[0]->dev_addr, ETH_ALEN);
    }
    memmove(sockaddr.sa_data, dev->dev_addr, ETH_ALEN);
    dev->set_mac_address(dev, &sockaddr);

    printk("%s: MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
            dev->name,
            dev->dev_addr[0],
            dev->dev_addr[1],
            dev->dev_addr[2],
            dev->dev_addr[3],
            dev->dev_addr[4],
            dev->dev_addr[5]
            );

    return 0;
}

/****************************************************************************/
/* Delete GPON Virtual Interface                                            */
/* Inputs: ifname = the GPON virtual interface name                         */
/****************************************************************************/
static int delete_gpon_vport(char *ifname)
{
    int i, ifid = 0;
    struct net_device *dev = NULL;
    BcmEnet_devctrl *pDevCtrl = NULL;

    /* Get the device structure from ifname */
    for (ifid = 0; ifid < MAX_GPON_IFS; ifid++) {
        dev = gponifid_to_dev[ifid];
        if (dev != NULL) {
            if (strcmp(dev->name, ifname) == 0) {
                break;
            }
        }
    }

    if (ifid >= MAX_GPON_IFS) {
        printk("delete_gpon_vport() : No such device \n");
        return -ENXIO;
    }

    /* Get the ifid of this interface */
    pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
    ifid =  pDevCtrl->gponifid;

    /* */
    synchronize_net();

    /* Remove the gem_ids supported by this interface */
    for (i = 0; i < MAX_GEM_IDS; i++) {
        remove_gemid_to_gponif_mapping(i, ifid);
    }

    if(memcmp(vnet_dev[0]->dev_addr, dev->dev_addr, ETH_ALEN)) {
        kerSysReleaseMacAddress(dev->dev_addr);
    }

    /* Delete the given gopn virtual interfaces. No need to call free_netdev
       after this as dev->destructor is set to free_netdev */
    unregister_netdevice(dev);
    gponifid_to_dev[ifid] = NULL;

    /* Free the ifid */
    freeid_map[ifid] = 0;

    /* Set the default gemid for this ifid as 0 */
    default_gemid[ifid] = 0;

    return 0;
}

/****************************************************************************/
/* Delete GPON Virtual Interface                                            */
/* Inputs: port = the GPON virtual interface number                         */
/****************************************************************************/
static int delete_all_gpon_vports(void)
{
    int i;
    struct net_device *dev = NULL;

    /* */
    synchronize_net();

    /* Make no gemid is assigned to an interface */
    initialize_gemid_to_ifIdx_mapping();

    /* Delete all gpon virtual interfaces */
    for (i = 0; i < MAX_GPON_IFS; i++) {
        if (gponifid_to_dev[i] == NULL) {
            continue;
        }
        dev = gponifid_to_dev[i];

        if(memcmp(vnet_dev[0]->dev_addr, dev->dev_addr, ETH_ALEN)) {
            kerSysReleaseMacAddress(dev->dev_addr);
        }

        /* No need to call free_netdev after this as dev->destructor
           is set to free_netdev */
        unregister_netdevice(dev);
        gponifid_to_dev[i] = NULL;
    }

    /* Free the ifids */
    for (i = 0; i < MAX_GPON_IFS; i++) {
        freeid_map[i] = 0;
    }

    /* Make default gem_ids for all ifids as 0 */
    for (i = 0; i < MAX_GPON_IFS; i++) {
        default_gemid[i] = 0;
    }

    return 0;
}

/****************************************************************************/
/* Set the multicast gem ID in GPON virtual interface                       */
/* Inputs: multicast gem port index                                         */
/****************************************************************************/
static int set_mcast_gem_id(u32 gem_map)
{
    int i;
    int mcast_gemid;
    int ifIdx;
    int ifid;
    struct net_device *dev = NULL;
    if (!gem_map)
    {
        printk("Error - set_mcast_gem_id() : No gemIdx in gem_map\n");
        return -1;
    }
    for (i = 0; i < MAX_GEM_IDS; i++) {
        if (gem_map & (1<<i)) {
            mcast_gemid = i;
            break;
        }
    }
    for (ifIdx = 0; ifIdx < MAX_GPON_IFS_PER_GEM; ++ifIdx)
    {
        if (gem_to_gponifid[mcast_gemid][ifIdx] == UNASSIGED_IFIDX_VALUE)
        {
            break;
        }
        ifid = gem_to_gponifid[mcast_gemid][ifIdx];
        if (ifid >= 0 && ifid < MAX_GPON_IFS) {
            if (gponifid_to_dev[ifid] != NULL) {
                dev = gponifid_to_dev[ifid];
                netdev_path_set_hw_subport_mcast(dev, mcast_gemid);
                printk("mcast_gem <%d> added to if <%s>\n",mcast_gemid,dev->name);
            }
        }
    }
    return 0;
}
/****************************************************************************/
/* Set the GEM Mask or Get the GEM Mask or Get the Free GEM Mask            */
/* Inputs: ifname = the interface name                                      */
/*         pgemmask = ptr to GEM mask if op is Set                          */
/* Outputs: pgem_map = ptr to GEM mask of an interface for Get op           */
/*                     ptr to Free GEM mask for GetFree op                  */
/* Returns: 0 on success; non-zero on failure                               */
/****************************************************************************/
static int set_get_gem_map(int op, char *ifname, int ifnum, u32 *pgem_map)
{
    int i, ifid = 0, count = 0, def_gem = 0;
    struct net_device *dev = NULL;
    BcmEnet_devctrl *pDevCtrl = NULL;

    /* Check whether ifname is all */
    if (ifname[0] != 0) {
        /* The ifname is not all */

        /* Get the device structure from ifname */
        for (ifid = 0; ifid < MAX_GPON_IFS; ifid++) {
            if (gponifid_to_dev[ifid] != NULL) {
                if (strcmp(gponifid_to_dev[ifid]->name, ifname) == 0) {
                    dev = gponifid_to_dev[ifid];
                    break;
                }
            }
        }

        if (dev == NULL) {
            printk("set_get_gem_map() : No such device \n");
            return -ENXIO;
        }

        /* Get the pointer to DevCtrl private structure */
        pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);

        if (op == GETGEMIDMAP) {
            *pgem_map = 0;
            /* Get the gem ids of given interface */
            for (i = 0; i < MAX_GEM_IDS; i++) {
                if (is_gemid_mapped_to_gponif(i, ifid) == TRUE) {
                    *pgem_map |= 1 << i;
                }
            }
        } else if (op == GETFREEGEMIDMAP) {
            *pgem_map = 0;
            /* Get the free gem ids */
            for (i = 0; i < MAX_GEM_IDS; i++) {
                if (get_first_gemid_to_ifIdx_mapping(i) == UNASSIGED_IFIDX_VALUE) {
                    *pgem_map |= 1 << i;
                }
            }
        } else if (op == SETGEMIDMAP) {
//printk("Given gemmap is %x \n", *pgem_map);
            /* Set the gem ids for given interface */
            for (i = 0; i < MAX_GEM_IDS; i++) {
                /* Check if gem_id(=i) is already a member */
                if (is_gemid_mapped_to_gponif(i, ifid) == TRUE) {
                    /* gem_id is already a member */
                    /* Check whether to remove it or not */
                    if (!((*pgem_map >> i) & 1)) {
                        /* It is not a member in the new
                           gem_map, so remove it */
                        remove_gemid_to_gponif_mapping(i, ifid);
                        netdev_path_rem_hw_subport(dev, i);
                    } else {
                        count++;
                        if (count == 1)
                            def_gem = i;
                    }
                }
                else if ((*pgem_map >> i) & 1) {
                    /* gem_id(=i) is not a member and is in the new
                       gem_map, so add it */
                    if (add_gemid_to_gponif_mapping(i, ifid) < 0)
                    {
                        printk("Error while adding gem<%d> to if<%s>\n",i,ifname);
                        return -ENXIO;
                    }
                    netdev_path_add_hw_subport(dev, i);
                    count++;
                    if (count == 1)
                        def_gem = i;
                }
            }
            pDevCtrl->gem_count = count;

            /* Set the default_gemid[ifid] when count is 1 */
            if (count == 1) {
                default_gemid[ifid] = def_gem;
            }
        }
    } else {
        /* ifname is all */
        if (op == GETGEMIDMAP) {
            /* Give the details if there is an interface at given ifnumber */
            *pgem_map = 0;
            if (gponifid_to_dev[ifnum] != NULL) {
                dev = gponifid_to_dev[ifnum];
                /* Get the gem ids of given interface */
                for (i = 0; i < MAX_GEM_IDS; i++) {
                    if (is_gemid_mapped_to_gponif(i, ifnum) == TRUE) {
                        *pgem_map |= 1 << i;
                    }
                }
                /* Get the interface name */
                strcpy(ifname, dev->name);
            }
        }
        else {
            printk("valid ifname is required \n");
            return -ENXIO;
        }
    }

    return 0;
}
#endif


#if defined(CONFIG_BCM96816)
static int reset_switch_wrapper(void *ctxt)
{
  return reset_switch(0);
};
#endif /*BCM96816*/
static int __init bcmenet_module_init(void)
{
    int status;
    int idx;

    TRACE(("bcm63xxenet: bcmenet_module_init\n"));

    if ( SKB_ALIGNED_SIZE != skb_aligned_size() )
    {
        printk("skb_aligned_size mismatch. Need to recompile enet module\n");
        return -ENOMEM;
    }

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    iqos_enet_status_hook_g = enet_iq_status;
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    gbpm_enet_status_hook_g = enet_bpm_status;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    gbpm_enet_thresh_hook_g = enet_bpm_dump_tx_drop_thr;
#endif
#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)
    gbpm_moca_thresh_hook_g = moca_bpm_dump_txq_thresh;
#endif
#endif

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
    bcm_arl_process_hook_g = enet_hook_for_arl_access;
#endif

    /* Initialize the static global array */
    for (idx = 0; idx < ENET_RX_CHANNELS_MAX; ++idx)
    {
        pending_channel[idx] = idx;
    }
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
    initialize_gemid_to_ifIdx_mapping();
#endif

    status = bcm63xx_enet_probe();

#if defined(SUPPORT_SWMDK)
    bcmFun_reg(BCM_FUN_ID_ENET_LINK_CHG, link_change_handler_wrapper);
#endif /*SUPPORT_SWMDK*/

#if defined(CONFIG_BCM96816)
    bcmFun_reg(BCM_FUN_ID_RESET_SWITCH, reset_switch_wrapper);
    bcmFun_reg(BCM_FUN_ID_ENET_CHECK_SWITCH_LOCKUP, ethsw_is_switch_locked);
    ethsw_rreg_ext(PAGE_CONTROL, 0x0a, &defaultIPG, 1, global.Is6829);
#endif /*BCM96816*/

    bcmFun_reg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY, remove_arl_entry_wrapper);
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Add code for buffer quick free between enet and xtm - June 2010 */
    bcmPktDma_set_enet_recycle((RecycleFuncP)bcm63xx_enet_recycle);
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */


    return status;
}



#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
/* configure ENET IQ thresholds for DQM */
static void enet_set_iq_thresh_dqm(BcmEnet_devctrl *pDevCtrl, int channel,
                                        uint16 loThresh, uint16 hiThresh)
{
    BcmPktDma_EthRxDma  *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;

    rxdma->iqLoThreshDqm = loThresh;
    rxdma->iqHiThreshDqm = hiThresh;
    rxdma->iqDroppedDqm  = 0;
    BCM_ENET_DEBUG("ENET: DQM Chnl=%d IQ Thresh lo=%d hi=%d\n",
           channel, (int) rxdma->iqLoThreshDqm, (int) rxdma->iqHiThreshDqm );
}

/* Update CPU congestion status based on the DQM IQ thresholds */
static void enet_iq_dqm_update_cong_status(BcmEnet_devctrl *pDevCtrl)
{
    int chnl;
    int iqDepth;

    if (iqos_fap_ethRxDqmQueue_hook_g == NULL)
        return;

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        BcmPktDma_EthRxDma  *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
            continue;

        /* get the DQM queue depth */
        iqDepth = iqos_fap_ethRxDqmQueue_hook_g( chnl );

        if (iqDepth >= rxdma->iqHiThreshDqm)
        {/* high thresh crossed on upside */
            iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_HI);
        }
        else if (iqDepth < rxdma->iqLoThreshDqm)
        {/* low thresh crossed on downside */
            iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_LO);
        }
        /* else donot change the congestion status */
    }
}

/* print the IQ DQM status */
static void enet_iq_dqm_status(void)
{
    int chnl;
    int iqDepth = 0;

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
        BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
            continue;

        if (iqos_fap_ethRxDqmQueue_hook_g == NULL)
            iqDepth = 0xFFFF;           /* Invalid value */
        else
            iqDepth = iqos_fap_ethRxDqmQueue_hook_g( chnl );

        printk("[DQM ] ENET %4d %5d %5d %5d %10u %8x\n",
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

/* init ENET IQ thresholds */
static void enet_rx_init_iq_thresh(BcmEnet_devctrl *pDevCtrl, int chnl)
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;
    int nr_rx_bds;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    {
        nr_rx_bds = bcmPktDma_EthGetRxBds( rxdma, chnl );
        BCM_ASSERT(nr_rx_bds > 0);
        enet_rx_dma_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_ENET_LO_THRESH_PCT)/100;
        enet_rx_dma_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_ENET_HI_THRESH_PCT)/100;
        BCM_ENET_RX_DEBUG("Enet: rxbds=%u, iqLoThresh=%u, iqHiThresh=%u\n",
                    nr_rx_bds,
                    enet_rx_dma_iq_thresh[chnl].loThresh,
                    enet_rx_dma_iq_thresh[chnl].hiThresh);
    }

    {/* DQM */
        nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxdqm[chnl];

        enet_rx_dqm_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_ENET_LO_THRESH_PCT)/100;
        enet_rx_dqm_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_ENET_HI_THRESH_PCT)/100;

        BCM_ENET_RX_DEBUG("Enet: dqm=%u, iqLoThresh=%u, iqHiThresh=%u\n",
                    nr_rx_bds,
                    enet_rx_dqm_iq_thresh[chnl].loThresh,
                    enet_rx_dqm_iq_thresh[chnl].hiThresh);
    }
#else
    {
        nr_rx_bds = bcmPktDma_EthGetRxBds( rxdma, chnl );

        enet_rx_dma_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_ENET_LO_THRESH_PCT)/100;
        enet_rx_dma_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_ENET_HI_THRESH_PCT)/100;
    }
#endif
}


static void enet_rx_set_iq_thresh( BcmEnet_devctrl *pDevCtrl, int chnl )
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    BCM_ENET_RX_DEBUG("Enet: Rx Chan=%d Owner=%d\n", chnl,
        g_Eth_rx_iudma_ownership[chnl]);
    if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
    {
        enet_set_iq_thresh_dqm(pDevCtrl, chnl,
                    enet_rx_dqm_iq_thresh[chnl].loThresh,
                    enet_rx_dqm_iq_thresh[chnl].hiThresh);

        bcmPktDma_EthSetIqDqmThresh(rxdma,
                    enet_rx_dqm_iq_thresh[chnl].loThresh,
                    enet_rx_dqm_iq_thresh[chnl].hiThresh);
    }
    else
    {
        rxdma->iqLoThresh = enet_rx_dma_iq_thresh[chnl].loThresh;
        rxdma->iqHiThresh = enet_rx_dma_iq_thresh[chnl].hiThresh;
    }

    bcmPktDma_EthSetIqThresh(rxdma,
                enet_rx_dma_iq_thresh[chnl].loThresh,
                enet_rx_dma_iq_thresh[chnl].hiThresh);
#else
    bcmPktDma_EthSetIqThresh(rxdma,
                enet_rx_dma_iq_thresh[chnl].loThresh,
                enet_rx_dma_iq_thresh[chnl].hiThresh);

    BCM_ENET_RX_DEBUG("Enet: chan=%d iqLoThresh=%d iqHiThresh=%d\n",
        chnl, (int) rxdma->iqLoThresh, (int) rxdma->iqHiThresh );
#endif
}


static void enet_iq_update_cong_status(BcmEnet_devctrl *pDevCtrl)
{
    int chnl;
    int thrOfst;
    DmaDesc  dmaDesc;
    volatile DmaDesc *rxBd_pv;
    BcmPktDma_EthRxDma  *rxdma;

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
            continue;
#endif

        if (iqos_get_cong_status(IQOS_IF_ENET, chnl) == IQOS_CONG_STATUS_HI)
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
                iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_LO);
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
                iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_HI);
            }
        }
    }
}

/* print the IQ status */
static void enet_iq_dma_status(void)
{
    int chnl;
    BcmPktDma_EthRxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
            continue;
#endif

        printk("[HOST] ENET %4d %5d %5d %5d %10u %8x\n",
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

/* print the IQ status */
static void enet_iq_status(void)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    enet_iq_dqm_status();
#endif
    enet_iq_dma_status();
}
#endif


#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/*
 * Assumptions:-
 * 1. Align data buffers on 16-byte boundary - Apr 2010
 */

uint32_t eth_alloc_buf_addr[BPM_ENET_BULK_ALLOC_COUNT];

/* Allocates BPM_ENET_BULK_ALLOC_COUNT number of bufs and assigns to the
 * DMA ring of an RX channel. The allocation is done in groups for
 * optimization.
 */
static inline int enet_bpm_alloc_buf(BcmEnet_devctrl *pDevCtrl, int channel)
{
    unsigned char *pFkBuf, *pData;
    int buf_ix;
    uint32_t *pBuf = eth_alloc_buf_addr;
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;

    if (gbpm_enable_g)
    {
        if((rxdma->numRxBds - rxdma->rxAssignedBds) >= rxdma->allocTrig)
        { /* number of used buffers has crossed the trigger threshold */
            if ( (gbpm_alloc_mult_buf( rxdma->bulkAlloc, pBuf ) )
                        == GBPM_ERROR)
            {
                /* may be temporarily global buffer pool is depleted.
                 * Later try again */
                return GBPM_ERROR;
            }

            rxdma->alloc += rxdma->bulkAlloc;

            for (buf_ix=0; buf_ix < rxdma->bulkAlloc; buf_ix++, pBuf++)
            {
                pFkBuf = (uint8_t *) (*pBuf);
                pData = PFKBUFF_TO_PDATA(pFkBuf,RX_ENET_SKB_HEADROOM);
                flush_assign_rx_buffer(pDevCtrl, channel, pData,
                            (uint8_t*)pFkBuf + RX_BUF_SIZE);
            }
        }
    }

    return GBPM_SUCCESS;
}

/* Dumps the BPM status for Eth channels */
static void enet_bpm_status(void)
{
    int chnl;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        BcmPktDma_EthRxDma *rxdma;
        rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
        if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
             continue;
#endif

        printk("[HOST] ENET %4d %10d %6d %10d %6d %7d %4d %4d\n",
               chnl, (int) rxdma->alloc,
               (rxdma->numRxBds - rxdma->rxAssignedBds),
               (int) rxdma->free, 0,
               (int) rxdma->numRxBds,
               (int) rxdma->allocTrig,
               (int) rxdma->bulkAlloc );
    }
}



/* Frees a buffer for an Eth RX channel to global BPM */
static inline int enet_bpm_free_buf(BcmEnet_devctrl *pDevCtrl, int channel,
                uint8 *pData)
{
    if (gbpm_enable_g)
    {
        BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;
        gbpm_free_buf((uint32_t *) PDATA_TO_PFKBUFF(pData,RX_ENET_SKB_HEADROOM));
        rxdma->free++;
    }

    return GBPM_SUCCESS;
}


/* Allocates the buffer ring for an Eth RX channel */
static int enet_bpm_alloc_buf_ring(BcmEnet_devctrl *pDevCtrl,
        int channel, uint32 num)
{
    unsigned char *pFkBuf, *pData;
    uint32 context = 0;
    uint32 buf_ix;

    RECYCLE_CONTEXT(context)->channel = channel;

    for (buf_ix=0; buf_ix < num; buf_ix++)
    {
        if ( (pFkBuf = (uint8_t *) gbpm_alloc_buf()) == NULL )
            return GBPM_ERROR;

        pData = PFKBUFF_TO_PDATA(pFkBuf,RX_ENET_SKB_HEADROOM);

        /* Place a FkBuff_t object at the head of pFkBuf */
        fkb_preinit(pFkBuf, (RecycleFuncP)bcm63xx_enet_recycle, context);

        cache_flush_region(pData, (uint8_t*)pFkBuf + RX_BUF_SIZE);
        bcmPktDma_EthFreeRecvBuf(&pDevCtrl->rxdma[channel]->pktDmaRxInfo, pData);
    }

    return GBPM_SUCCESS;
}


/* Frees the buffer ring for an Eth RX channel */
static void enet_bpm_free_buf_ring(BcmEnet_RxDma *rxdma, int channel)
{
    uninit_buffers(rxdma);
}


static void enet_rx_set_bpm_alloc_trig( BcmEnet_devctrl *pDevCtrl, int chnl )
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;
    uint32  allocTrig = rxdma->numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
    {
        rxdma->allocTrig = allocTrig;
        rxdma->bulkAlloc = BPM_ENET_BULK_ALLOC_COUNT;
    }
    else
    {
        bcmPktDma_EthSetRxChanBpmThresh(rxdma,
            allocTrig, BPM_ENET_BULK_ALLOC_COUNT);
        BCM_ENET_DEBUG( "Enet: Chan=%d BPM Rx allocTrig=%d bulkAlloc=%d\n",
            chnl, (int) (rxdma->allocTrig), (int) (rxdma->bulkAlloc) );
    }
#else
    bcmPktDma_EthSetRxChanBpmThresh(rxdma,
        allocTrig, BPM_ENET_BULK_ALLOC_COUNT);
    BCM_ENET_DEBUG( "Enet: Chan=%d BPM Rx allocTrig=%d bulkAlloc=%d\n",
        chnl, (int) (rxdma->allocTrig), (int) (rxdma->bulkAlloc) );
#endif
}


#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
/* Dumps the TxDMA drop thresh for eth channels */
static void enet_bpm_dma_dump_tx_drop_thr(void)
{
    int chnl;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    for (chnl = 0; chnl < cur_txdma_channels; chnl++)
    {
        BcmPktDma_EthTxDma *txdma = pDevCtrl->txdma[chnl];
        int q;

        if (g_Eth_tx_iudma_ownership[chnl] != HOST_OWNED)
             continue;

        for ( q=0; q < ENET_TX_EGRESS_QUEUES_MAX; q++ )
            printk("[HOST] ENET %4u %4u %10u %10u\n",
               chnl, q,
               (uint32_t) txdma->txDropThr[q], 
               (uint32_t) txdma->txDropThrPkts[q]);
    }
}

/* print the BPM TxQ Drop Thresh */
static void enet_bpm_dump_tx_drop_thr(void)
{
    enet_bpm_dma_dump_tx_drop_thr();
}


/* init ENET TxQ drop thresholds */
static void enet_bpm_init_tx_drop_thr(BcmEnet_devctrl *pDevCtrl, int chnl)
{
    BcmPktDma_EthTxDma *txdma = pDevCtrl->txdma[chnl];
    int nr_tx_bds;

    nr_tx_bds = bcmPktDma_EthGetTxBds( txdma, chnl );
    BCM_ASSERT(nr_tx_bds > 0);
    enet_bpm_dma_tx_drop_thr[chnl][0] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ0_DROP_THRESH)/100;
    enet_bpm_dma_tx_drop_thr[chnl][1] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ1_DROP_THRESH)/100;
    enet_bpm_dma_tx_drop_thr[chnl][2] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ2_DROP_THRESH)/100;
    enet_bpm_dma_tx_drop_thr[chnl][3] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ3_DROP_THRESH)/100;

    BCM_ENET_DEBUG("Enet: BPM DMA Init Tx Drop Thresh: chnl=%u txbds=%u thr[0]=%u thr[1]=%u thr[2]=%u thr[3]=%u\n",
                chnl, nr_tx_bds, 
                enet_bpm_dma_tx_drop_thr[chnl][0],
                enet_bpm_dma_tx_drop_thr[chnl][1],
                enet_bpm_dma_tx_drop_thr[chnl][2],
                enet_bpm_dma_tx_drop_thr[chnl][3]);
}


static void enet_bpm_set_tx_drop_thr( BcmEnet_devctrl *pDevCtrl, int chnl )
{
    BcmPktDma_EthTxDma *txdma = pDevCtrl->txdma[chnl];
    int q;
    BCM_ENET_DEBUG("Enet: BPM Set Tx Chan=%d Owner=%d\n", chnl,
        g_Eth_tx_iudma_ownership[chnl]);
    if (g_Eth_tx_iudma_ownership[chnl] == HOST_OWNED)
    {
        for (q=0; q < ENET_TX_EGRESS_QUEUES_MAX; q++)
            txdma->txDropThr[q] = enet_bpm_dma_tx_drop_thr[chnl][q];
    }

    bcmPktDma_EthBpmSetTxChanThresh(txdma, 
        (uint16 *) &enet_bpm_dma_tx_drop_thr[chnl]);
}
#endif


#if defined(CONFIG_BCM_MOCA_SOFT_SWITCHING)

/*
 *------------------------------------------------------------------------------
 * function   : moca_lan_bpm_txq_thresh
 * description: configures the MoCA LAN queue thresholds
 *------------------------------------------------------------------------------
 */
static int moca_lan_bpm_txq_thresh(int qid)
{
    moca_queue_t *moca_lan_p = &global.moca_lan;

    moca_lan_p->thresh[qid].q_lo_thresh = MOCA_LAN_BPM_TXQ_LO_THRESH;
    moca_lan_p->thresh[qid].q_hi_thresh = MOCA_LAN_BPM_TXQ_HI_THRESH;
    moca_lan_p->thresh[qid].q_dropped = 0;

    BCM_ENET_DEBUG("MOCA LAN Tx qid[%d] q_lo_thresh=%d, q_hi_thresh=%d\n",
        qid, moca_lan_p->thresh[qid].q_lo_thresh,
        moca_lan_p->thresh[qid].q_hi_thresh);

    return GBPM_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : moca_lan_dump_txq_thresh
 * Description: function for dumping the MoCA LAN TxQ thresh
 *------------------------------------------------------------------------------
 */
static void moca_lan_dump_txq_thresh( void )
{
    int qid;
    moca_queue_t *moca_lan_p = &global.moca_lan;

    for (qid=0; qid < NUM_MOCA_SW_QUEUES; qid++)
        printk( "LAN MoCA %4d %5d %5d %10u\n", qid,
            moca_lan_p->thresh[qid].q_lo_thresh,
            moca_lan_p->thresh[qid].q_hi_thresh,
            moca_lan_p->thresh[qid].q_dropped );

    printk( "\n" );

    return;
}


/*
 *------------------------------------------------------------------------------
 * function   : moca_wan_bpm_txq_thresh
 * description: configures the moca WAN queue thresholds
 *------------------------------------------------------------------------------
 */
static int moca_wan_bpm_txq_thresh(int qid)
{
    moca_queue_t *moca_wan_p = &global.moca_wan;

    moca_wan_p->thresh[qid].q_lo_thresh = MOCA_WAN_BPM_TXQ_LO_THRESH;
    moca_wan_p->thresh[qid].q_hi_thresh = MOCA_WAN_BPM_TXQ_HI_THRESH;
    moca_wan_p->thresh[qid].q_dropped = 0;

    BCM_ENET_DEBUG("MOCA WAN Tx qid[%d] q_lo_thresh=%d, q_hi_thresh=%d\n",
        qid, moca_wan_p->thresh[qid].q_lo_thresh,
        moca_wan_p->thresh[qid].q_hi_thresh);

    return GBPM_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : moca_wan_dump_txq_thresh
 * Description: function for dumping the MoCA WAN TxQ thresh
 *------------------------------------------------------------------------------
 */
static void moca_wan_dump_txq_thresh( void )
{
    int qid;
    moca_queue_t *moca_wan_p = &global.moca_wan;

    for (qid=0; qid < NUM_MOCA_SW_QUEUES; qid++)
        printk( "WAN MoCA %4d %5d %5d %10u\n", qid,
            moca_wan_p->thresh[qid].q_lo_thresh,
            moca_wan_p->thresh[qid].q_hi_thresh,
            moca_wan_p->thresh[qid].q_dropped );

    return;
}


static void moca_bpm_dump_txq_thresh(void)
{
    moca_lan_dump_txq_thresh();
    moca_wan_dump_txq_thresh();
}
#endif
#endif


module_init(bcmenet_module_init);
module_exit(bcmenet_module_cleanup);
MODULE_LICENSE("GPL");

