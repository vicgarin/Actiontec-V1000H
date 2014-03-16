#ifndef __FAP4KE_MEMORY_H_INCLUDED__
#define __FAP4KE_MEMORY_H_INCLUDED__

/*

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved

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

/*
 *******************************************************************************
 * File Name  : fap4ke_memory.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#include "fap4ke_init.h"
#include "fap4ke_timers.h"
#include "fap4ke_irq.h"
#include "fap_dqm.h"
#include "fap4ke_packet.h"
#include "fap4ke_iopDma.h"
#include "fap4ke_mailBox.h"
#include "bcmPktDma_structs.h"
#include "fap4ke_xtmrt.h"
#include "fap4ke_gso.h"
#if defined(CONFIG_BCM963268)
#include "bcmxtmrtbond.h"
#endif
//#include "fap_hw.h"

/***************************************************
 * 4ke Data Scratch Pad Ram (DSPRAM) Mappings
 ***************************************************/

/* In the 6362 we will always keep the XTM Tx BDs in DDR because the upstream
   bandwidth is fairly small, and we do not have enough space in the PSM to
   allocate the BDs required for 16 channels */
#define ENET_RX_BDS_IN_PSM
#define ENET_TX_BDS_IN_PSM
#define XTM_RX_BDS_IN_PSM
//#define XTM_TX_BDS_IN_PSM

// Uncomment to enable enet rx polling in the 4ke task loop
//#define ENABLE_ENET_RX_POLLING

// Uncomment to enable counters to examine the enet comms between FAP and Host - May 2010
//#define ENABLE_FAP_COMMS_DEBUG

#if defined(ENABLE_FAP_COMMS_DEBUG)
#define ENET_TX_HOST_2_FAP_REJECT		0
#define ENET_RXFREE_HOST_2_FAP_WAIT		1
#define ENET_RX_FAP_2_HOST_REJECT		2
#define ENET_TXFREE_FAP_2_HOST_WAIT		3
#define ENET_RX_NO_BDS				4
#endif

#define FAP_PSM_MANAGED_MEMORY_SIZE    11008

#define p4keDspram ( (fap4keDspram_alloc_t *)(DSPRAM_VBASE) )

#define p4keDspramGbl ( (fap4keDspram_global_t *)(&p4keDspram->global) )

#define CC_FAP_BPM_BUF_FREE_IN_PSM


/*
 * fap4keDspram_timers_t: all Timers must be declared here
 */
typedef struct {
    /* Timer Management */
    volatile int64 jiffies64;
    Dll_t timersList;

    /* CPU Utilization */
    fap4keTmr_cpuSample_t cpu;

    /* User-defined timers */
    fap4keTmr_timer_t keepAlive;
    fap4keTmr_timer_t flowStatsTimer;
} fap4keDspram_timers_t;

/*
 * fap4keDspram_tasks_t: all Tasks must be declared here
 */
typedef struct {
    /* Task Management */
    fap4keTsk_scheduler_t scheduler;

    /* User-defined Tasks */
    fap4keTsk_task_t enetRecv0;
    fap4keTsk_task_t enetRecv1;
    fap4keTsk_task_t xtmRecv0;
    fap4keTsk_task_t xtmRecv1;
} fap4keDspram_tasks_t;

/*
 * fap4keDspram_irq_t: Interrupt Management variables
 */
typedef struct {
    /* Interrupt Management */
    uint32 handlerCount;
    uint32 wait_pc;
    uint32 epc_jump;
    fap4keIrq_handlerInfo_t handlerInfo[FAP4KE_IRQ_HANDLERS_MAX];
} fap4keDspram_irq_t;

/*
 * fap4keDqm_handlerInfo_t: DQM queue handler structure
 */
typedef struct {
   uint32                   mask;
   uint32                   enable;
   fap4keTsk_taskPriority_t taskPriority;
   fap4keTsk_task_t         task;
} fap4keDqm_handlerInfo_t;

/*
 * fap4keDspram_dqm_t: DQM variables
 */
typedef struct{
    /* Queue Handlers */
    uint32 handlerCount;
    fap4keDqm_handlerInfo_t handlerInfo[DQM_MAX_HANDLER];
} fap4keDspram_dqm_t;

/*
 * fap4keDspram_enet_t: ENET variables
 */
typedef struct{
    fap4keTsk_task_t *xmitFromHostTask;
    fap4keTsk_task_t *recvFreeFromHostTask;
    /* Channel Bitmap used to enable/disable Tx cleanup for each iuDMA channel */
    uint32 txCleanupChannelMap;
    int      enetTxChannel;		/* enet iuDMA channel that FAP should use when TX_SPLITTING enabled - Aug 2010 */
    uint32_t enetRxToss;		/* number of times rx pkts dropped so 1 rx iuDMA ch not limit rx on another - Sept 2010 */
#if defined(CONFIG_BCM963268) && (CONFIG_BCM_EXT_SWITCH)
    uint32 extSwConnPort; /* internal switch port that is connected to external switch - Aug 2011 */
    uint32 extSwConnPortMask; /* Bitmask of internal switch port that is connected to external switch - Aug 2011 */
#endif
} fap4keDspram_enet_t;

/*
 * fap4keDspram_xtm_t: XTM variables
 */
typedef struct{
    fap4keTsk_task_t *xmitFromHostTask;
    fap4keTsk_task_t *recvFreeFromHostTask;
    /* Channel Bitmap used to enable/disable Tx cleanup for each iuDMA channel */
    uint32 txCleanupChannelMap;
    fap4keXtm_devMap_t devMap;
    fap4keXtm_qos_t qos;
    int                xtmTxChannel;		/* xmt iuDMA channel that FAP should use when TX_SPLITTING enabled - Aug 2010 */
} fap4keDspram_xtm_t;

/*
 * fap4keDspram_packet_t: Packet processing variables
 */
typedef struct {
    fap4kePkt_runtime_t runtime;
} fap4keDspram_packet_t;


#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
/*
 * fap4keDspram_iq_t: Ingress Qos
 */
typedef struct {
    uint32 enable;          /* IQ feature enable */
    uint32 cpu_cong;        /* IQ CPU congestion */
} fap4keDspram_iq_t;
#define p4keDspIq ( &p4keDspramGbl->iq )
#endif



typedef enum {
    W2S_state_enetStopIudmas,
    W2S_state_enetFlushIudmaRings, 
    W2S_state_xtmStopIudmas,
    W2S_state_xtmFlushIudmaRings, 
    W2S_state_flushFreeCache, 
    W2S_state_flushFreeCacheCont, 
    W2S_state_stopFfe, 
} W2S_state_t;


#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define FAP_BPM_ENET_BULK_ALLOC_MAX     128
#define FAP_BPM_XTM_BULK_ALLOC_MAX      128

#define FAP_BPM_FREE_TRIG_THRESH        256
#define FAP_BPM_BULK_FREE_COUNT         256
#define FAP_BPM_BULK_FREE_MAX_REQ       4

/*
 * fap4keDspram_bpm_t: BPM info.
 * Note: local free cache is shared by all Eth and XTM channels
 */
typedef struct {
    uint32 enable;              /* BPM feature enable */
    uint32 pend_buf_free;       /* number of buffers pending free */
    uint32 freeReqIdx;          /* the active local free cache index  */
    uint32 *freeCachePtr;       /* the pointer to active local free cache */
    uint32 free;                /* count of number of buffers freed */
    /* waiting response from BPM for the free requests. */
    uint32 buf_free_wait[FAP_BPM_BULK_FREE_MAX_REQ]; /* 1=wait, 0=no wait */
} fap4keDspram_bpm_t;
#define p4keDspBpm ( &p4keDspramGbl->bpm )
#endif

/*
 * fap4keDspram_global_t: contains all global variables stored in DSPRAM
 */
typedef struct {
    uint32 scribble0;
    /* Timers */
    fap4keDspram_timers_t timers;
    /* Tasks */
    fap4keDspram_tasks_t tasks;
    /* Interrupts */
    fap4keDspram_irq_t irq;
    /* DQM */
    fap4keDspram_dqm_t dqm;
    /* ENET */
    fap4keDspram_enet_t enet;
    /* XTM */
    fap4keDspram_xtm_t xtm;
    /* Packet */
    fap4keDspram_packet_t packet;
    /* Eth Iudma */
    BcmPktDma_LocalEthRxDma EthRxDma[ENET_RX_CHANNELS_MAX];
    BcmPktDma_LocalEthTxDma EthTxDma[ENET_TX_CHANNELS_MAX];

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    fap4keDspram_iq_t  iq;
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    fap4keDspram_bpm_t  bpm;
#endif
    uint8  repLowStack;
    uint32 scribble1;
} fap4keDspram_global_t;

/*
 * fap4keDspram_alloc_t: used to manage the overall 4ke DSPRAM allocation
 */
typedef struct {
    /* 4ke stack: Never write to this area!!! */
    volatile const uint8 stack4ke[FAP_INIT_4KE_STACK_SIZE];

    union {
        volatile const uint8 global_u8[DSPRAM_SIZE - FAP_INIT_4KE_STACK_SIZE];

        fap4keDspram_global_t global;
    };
} fap4keDspram_alloc_t;


/***************************************************
 * SDRAM Mappings
 ***************************************************/

/*
 * We need to avoid global allocations in SDRAM to be cached by the Host MIPS,
 * by aligning all SDRAM allocations to a Host D$ line size, and making the
 * allocations to be an integer multiple of the Host D$ line size
 */

#define FAP4KE_SDRAM_ALLOC_SIZE 4096 /* bytes */

typedef struct {
    fap4keTmr_timer_t testTimer;
} fap4keSdram_main_t;

typedef struct {
    uint16 printCount4ke;
    uint16 keepAliveCount4ke;
} fap4keSdram_mailBox_t;

typedef struct {
    /* used in QSM memory management */
    uint32 availableMemory;
    uint32 nextAddress;         /* next available address (byte address) */

    /* test code */
    fap4keTmr_timer_t dqmTestTimer;
    uint32 inc;
} fap4keSdram_dqm_t;

typedef struct {
    fap4kePkt_flowInfo_t flowInfoPool[FAP4KE_PKT_MAX_FLOWS];
    fap4kePkt_learn_t learn;
} fap4keSdram_packet_t;

typedef struct {
    uint8 *gsoBuffer;
} fap4keSdram_initParams_t;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
 
typedef struct {
    uint32 dbg;        /* IQ debug flag */
} fap4keSdram_iq_t;
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
typedef struct {
    /* storage for buffer address requesed for Eth RX channel */
    uint32 bufAddr[FAP_BPM_ENET_BULK_ALLOC_MAX];
} fap4keSdram_enetBpm_t;

typedef struct {
    /* storage for buffer address requesed for XTM RX channel */
    uint32 bufAddr[FAP_BPM_XTM_BULK_ALLOC_MAX];
} fap4keSdram_xtmBpm_t;

typedef struct {
    uint32 dbg;        /* BPM debug flag */
    fap4keSdram_enetBpm_t enet[ENET_RX_CHANNELS_MAX];
    fap4keSdram_xtmBpm_t  xtm[XTM_RX_CHANNELS_MAX];
    /*
     * The local free cache is a temporary storage (cache) of buffer addresses.
     * When the addresses are recycled and there is no space in the RX ring, the
     * excess buffers are stored in the local free cache.
     * There are FAP_BPM_BULK_FREE_MAX_REQ number of local free caches, each
     * of size FAP_BPM_BULK_FREE_COUNT words.
     */
#if !defined(CC_FAP_BPM_BUF_FREE_IN_PSM)
    uint32 freeCache[FAP_BPM_BULK_FREE_MAX_REQ][FAP_BPM_BULK_FREE_COUNT];
#endif
} fap4keSdram_bpm_t;
#endif

typedef union {
    uint8 u8[FAP4KE_SDRAM_ALLOC_SIZE];

    struct {
        uint8 resv1[16];
        uint32 dbgVals[10];
        fap4keSdram_mailBox_t mailBox;
        fap4keSdram_main_t main;
        fap4keSdram_dqm_t dqm;
        fap4keSdram_packet_t packet;
        uint8 localPrintBuf[FAP_MAILBOX_PRINTBUF_SIZE];
        volatile fap4keSdram_initParams_t initParams;
        W2S_state_t w2s_state;
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
        fap4keSdram_iq_t iq;
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        fap4keSdram_bpm_t bpm;
#endif
        uint8 resv2[16];
    } alloc;
} __attribute__((aligned(16))) fap4keSdram_alloc_t;

#ifdef FAP_4KE

/* declared in fap4ke_main.c */
extern fap4keSdram_alloc_t fap4keSdram_g;
#define p4keSdram ( (fap4keSdram_alloc_t *)(mCacheToNonCacheVirtAddr(&fap4keSdram_g)) )
#endif

/***************************************************
 * Packet Shared Memory (PSM) Mappings
 ***************************************************/
#define p4kePsm ( (fap4kePsm_alloc_t *)(FAP_4KE_PSM_BASE) )
#define p4kePsmGbl ( &p4kePsm->global )

/* In the 63268, the PSM has a different base address for each port.
   This is the PSM address of the port connected to the IOP */
#define p4kePsmIop ( (fap4kePsm_alloc_t *)(FAP_4KE_PSM_BASE_IOP) )
#define p4kePsmIopGbl ( &p4kePsmIop->global )

/*
 * fap4kePsm_packet_t: Packet Manager Variables, including flow definitions
 */
typedef struct {
    uint8 headerPool[FAP4KE_PKT_MAX_HEADERS][FAP4KE_PKT_HEADER_SIZE_MAX];
    fap4kePkt_flow_t flowPool[FAP4KE_PKT_MAX_PSM_FLOWS];    
    fap4kePkt_shared_t shared[FAP4KE_PKT_CMDLISTS_IN_PSM];
} fap4kePsm_packet_t;

/*
 * fap4kePsm_timers_t: Timers variables in PSM
 */
typedef struct {
    fap4keTmr_cpuHistory_t cpu;
} fap4kePsm_timers_t;


//#define CC_FAP4KE_PMON

#if defined(CC_FAP4KE_PMON)
/*
 * fap4kePsm_pmon_t: Performance Monitoring variables in PSM
 */
typedef enum {
    FAP4KE_PMON_ID_NOP50 = 0,
    FAP4KE_PMON_ID_REG_WR,
    FAP4KE_PMON_ID_REG_RD,

    FAP4KE_PMON_ID_ENET_RECV,
    FAP4KE_PMON_ID_ENET_DMA_IN,
    FAP4KE_PMON_ID_ENET_CLASSIFY,
    FAP4KE_PMON_ID_ENET_HIT,
    FAP4KE_PMON_ID_ENET_XMIT_PREP,
    FAP4KE_PMON_ID_ENET_RECYCLE,
    FAP4KE_PMON_ID_ENET_XMIT,
    FAP4KE_PMON_ID_ENET_EXIT,

    FAP4KE_PMON_ID_XTM_RX_BEGIN,
    FAP4KE_PMON_ID_XTM_RX_HEADER,
    FAP4KE_PMON_ID_XTM_RX_CLASSIFY,
    FAP4KE_PMON_ID_XTM_RX_XMIT,
    FAP4KE_PMON_ID_XTM_RX_END,

    FAP4KE_PMON_ID_DMA_RX_START,
    FAP4KE_PMON_ID_DMA_RX_FINISH,
    FAP4KE_PMON_ID_CLASSIFY,
    FAP4KE_PMON_ID_SW_MOD,
    FAP4KE_PMON_ID_DMA_TX_START,
    FAP4KE_PMON_ID_DMA_TX_FINISH,

    FAP4KE_PMON_ID_FFE1,
    FAP4KE_PMON_ID_FFE2,
    FAP4KE_PMON_ID_FFE3,

    FAP4KE_PMON_ID_IUDMA_XMIT,
    FAP4KE_PMON_ID_IUDMA_RECV,
    FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET,
    FAP4KE_PMON_ID_IUDMA_FREERECVBUF,

    FAP4KE_PMON_ID_ENET_IQ,
    FAP4KE_PMON_ID_XTM_IQ,

    FAP4KE_PMON_ID_ENET_BPM_ALLOC,
    FAP4KE_PMON_ID_ENET_BPM_FREE,
    FAP4KE_PMON_ID_XTM_BPM_ALLOC,
    FAP4KE_PMON_ID_XTM_BPM_FREE,

    FAP4KE_PMON_ID_MAX
} fap4kePsm_pmonId_t;

#undef FAP_DECL
#define FAP_DECL(x) #x,

#define FAP4KE_PMON_ID_NAME \
    {                                                   \
        FAP_DECL(FAP4KE_PMON_ID_NOP50)                  \
        FAP_DECL(FAP4KE_PMON_ID_REG_WR)                 \
        FAP_DECL(FAP4KE_PMON_ID_REG_RD)                 \
        FAP_DECL(FAP4KE_PMON_ID_ENET_RECV)          \
        FAP_DECL(FAP4KE_PMON_ID_ENET_DMA_IN)          \
        FAP_DECL(FAP4KE_PMON_ID_ENET_CLASSIFY)         \
        FAP_DECL(FAP4KE_PMON_ID_ENET_HIT)       \
        FAP_DECL(FAP4KE_PMON_ID_ENET_XMIT_PREP)           \
        FAP_DECL(FAP4KE_PMON_ID_ENET_RECYCLE)           \
        FAP_DECL(FAP4KE_PMON_ID_ENET_XMIT)           \
        FAP_DECL(FAP4KE_PMON_ID_ENET_EXIT)            \
        FAP_DECL(FAP4KE_PMON_ID_XTM_RX_BEGIN)           \
        FAP_DECL(FAP4KE_PMON_ID_XTM_RX_HEADER)          \
        FAP_DECL(FAP4KE_PMON_ID_XTM_RX_CLASSIFY)        \
        FAP_DECL(FAP4KE_PMON_ID_XTM_RX_XMIT)            \
        FAP_DECL(FAP4KE_PMON_ID_XTM_RX_END)             \
        FAP_DECL(FAP4KE_PMON_ID_DMA_RX_START)           \
        FAP_DECL(FAP4KE_PMON_ID_DMA_RX_FINISH)          \
        FAP_DECL(FAP4KE_PMON_ID_CLASSIFY)               \
        FAP_DECL(FAP4KE_PMON_ID_SW_MOD)                 \
        FAP_DECL(FAP4KE_PMON_ID_DMA_TX_START)           \
        FAP_DECL(FAP4KE_PMON_ID_DMA_TX_FINISH)          \
        FAP_DECL(FAP4KE_PMON_ID_FFE1)                   \
        FAP_DECL(FAP4KE_PMON_ID_FFE2)                   \
        FAP_DECL(FAP4KE_PMON_ID_FFE3)                   \
        FAP_DECL(FAP4KE_PMON_ID_IUDMA_XMIT)             \
        FAP_DECL(FAP4KE_PMON_ID_IUDMA_RECV)             \
        FAP_DECL(FAP4KE_PMON_ID_IUDMA_FREEXMITBUFGET)   \
        FAP_DECL(FAP4KE_PMON_ID_IUDMA_FREERECVBUF)      \
        FAP_DECL(FAP4KE_PMON_ID_ENET_IQ) \
        FAP_DECL(FAP4KE_PMON_ID_XTM_IQ) \
        FAP_DECL(FAP4KE_PMON_ID_ENET_BPM_ALLOC) \
        FAP_DECL(FAP4KE_PMON_ID_ENET_BPM_FREE)  \
        FAP_DECL(FAP4KE_PMON_ID_XTM_BPM_ALLOC)  \
        FAP_DECL(FAP4KE_PMON_ID_XTM_BPM_FREE)   \
    }

typedef struct {
    uint32 globalIrqs;
    uint32 halfCycles[FAP4KE_PMON_ID_MAX];
    uint32 instncomplete[FAP4KE_PMON_ID_MAX];
    uint32 icachehit[FAP4KE_PMON_ID_MAX];
    uint32 icachemiss[FAP4KE_PMON_ID_MAX];
    uint32 interrupts[FAP4KE_PMON_ID_MAX];
} fap4kePsm_pmon_t;
#endif /* CC_FAP4KE_PMON */


typedef struct {
/* Stats for 4ke enet rx (iuDMA interrupt) */
    uint32 rxCount;             /* snapshot */
    uint32 rxHighWm;            /* peak value */
    int32 txCount;
    uint32 rxTotal;
    uint32 rxDropped;
    uint32 rxNoBd;
    uint32 rxAssignedBdsMin;    /* peak value */
    uint32 txFreeBdsMin;        /* peak value */
/* Stats for Host enet rx (Q7) */
    uint32 Q7budget;            /* snapshot */
    uint32 Q7rxCount;           /* snapshot */
    uint32 Q7rxHighWm;          /* peak value */
    uint32 Q7rxTotal;
/* Stats for 4ke enet rx free (Q12) */
    uint32 Q12rxCount;          /* snapshot */
    uint32 Q12rxHighWm;         /* peak value */
    uint32 Q12rxTotal;
/* Stats for 4ke enet tx (Q11) */
    uint32 Q11txCount;          /* snapshot */
    uint32 Q11txHighWm;         /* peak value */
    uint32 Q11txTotal;
/* Stats for Host enet tx free (Q13) */
    uint32 Q13txCount;          /* snapshot */
    uint32 Q13txHighWm;         /* peak value */
    uint32 Q13txTotal;
} fap4kePsm_stats_t;

/*
 * fap4kePsm_gso_t: gso variables in PSM
 */
#if defined(CC_FAP4KE_PKT_GSO)
typedef struct {
    /* Move GSO variables from DSPRAM to PSM - July 2010 */
    fap4keGso_runtime_t runtime;
    fap4keGso_shared_t shared;
} fap4kePsm_gso_t;
#endif

//#define CC_FAP4KE_TRACE

#if defined(CC_FAP4KE_TRACE)
#define FAP4KE_TRACE_HISTORY_SIZE 300

#undef FAP4KE_DECL
#define FAP4KE_DECL(x) #x,

#define FAP4KE_TRACE_TYPE_NAME       \
    {                                \
        FAP4KE_DECL(RX_BEGIN)        \
        FAP4KE_DECL(RX_PACKET)       \
        FAP4KE_DECL(RX_END)          \
        FAP4KE_DECL(TX_BEGIN)        \
        FAP4KE_DECL(TX_FREE)         \
        FAP4KE_DECL(TX_PACKET)       \
        FAP4KE_DECL(TX_END)          \
        FAP4KE_DECL(IRQ_BEGIN)       \
        FAP4KE_DECL(IRQ_CALL_START)  \
        FAP4KE_DECL(IRQ_CALL_END)    \
        FAP4KE_DECL(IRQ_END)         \
        FAP4KE_DECL(TASK)            \
        FAP4KE_DECL(WAIT_START)      \
        FAP4KE_DECL(WAIT_END)        \
        FAP4KE_DECL(FAP4KE_TRACE_ALLOC_BUF_BEGIN)   \
        FAP4KE_DECL(FAP4KE_TRACE_ALLOC_BUF_END)     \
        FAP4KE_DECL(FAP4KE_TRACE_FREE_BUF_BEGIN)    \
        FAP4KE_DECL(FAP4KE_TRACE_FREE_BUF_END)      \
    }

typedef enum {
    FAP4KE_TRACE_RX_BEGIN,
    FAP4KE_TRACE_RX_PACKET,
    FAP4KE_TRACE_RX_END,

    FAP4KE_TRACE_TX_BEGIN,
    FAP4KE_TRACE_TX_FREE,
    FAP4KE_TRACE_TX_PACKET,
    FAP4KE_TRACE_TX_END,

    FAP4KE_TRACE_IRQ_BEGIN,
    FAP4KE_TRACE_IRQ_CALL_START,
    FAP4KE_TRACE_IRQ_CALL_END,
    FAP4KE_TRACE_IRQ_END,

    FAP4KE_TRACE_TASK,
    FAP4KE_TRACE_WAIT_START,
    FAP4KE_TRACE_WAIT_END,

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    FAP4KE_TRACE_ALLOC_BUF_BEGIN,
    FAP4KE_TRACE_ALLOC_BUF_END,
    FAP4KE_TRACE_FREE_BUF_BEGIN,
    FAP4KE_TRACE_FREE_BUF_END,
#endif
    FAP4KE_TRACE_MAX
} fap4keTrace_id_t;

typedef enum {
    FAP4KE_TRACE_TYPE_DEC,
    FAP4KE_TRACE_TYPE_HEX,
    FAP4KE_TRACE_TYPE_STR,
    FAP4KE_TRACE_TYPE_MAX
} fap4keTrace_type_t;

typedef struct {
    fap4keTrace_id_t id;
    uint32_t cycles;
    uint32_t arg;
    fap4keTrace_type_t type;
} fap4keTrace_record_t;

typedef struct {
    uint32_t write;
    uint32_t count;
    fap4keTrace_record_t record[FAP4KE_TRACE_HISTORY_SIZE];
} fap4keTrace_history_t;

/*
 * fap4kePsm_trace_t: 4ke Trace variables in PSM
 */
typedef struct {
    uint32_t enable;
    fap4keTrace_history_t history;
} fap4kePsm_trace_t;

#define p4keTrace ( &p4kePsmGbl->trace )
#endif /* CC_FAP4KE_TRACE */


#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
typedef struct {
    uint32 buf_thresh_lvl;  /* the current buffer level in global BPM */
    /*
     * The local free cache is a temporary storage (cache) of buffer addresses.
     * When the addresses are recycled and there is no space in the RX ring, the
     * excess buffers are stored in the local free cache.
     * There are FAP_BPM_BULK_FREE_MAX_REQ number of local free caches, each
     * of size FAP_BPM_BULK_FREE_COUNT words.
     */
#if defined(CC_FAP_BPM_BUF_FREE_IN_PSM)
    uint32 freeCache[FAP_BPM_BULK_FREE_MAX_REQ][FAP_BPM_BULK_FREE_COUNT];
#endif
} fap4kePsm_bpm_t;
#define p4kePsmBpm ( &p4kePsmGbl->bpm )
#endif


/*
 * fap4kePsm_global_t: contains all global variables stored in the PSM
 */

typedef struct {
    fap4kePsm_timers_t timers;
    uint32 scribble0;
#if defined(CC_FAP4KE_PMON)
    fap4kePsm_pmon_t pmon;
#endif
    uint32 scribble1;
    fap4kePsm_stats_t stats;
    fap4kePsm_stats_t xtmStats;

    /* Xtm Iudma */
    BcmPktDma_LocalXtmRxDma XtmRxDma[XTM_RX_CHANNELS_MAX];
    BcmPktDma_LocalXtmTxDma XtmTxDma[XTM_TX_CHANNELS_MAX];

    uint32 scribble2;
    /* Global flag to coordinate XTM tx disable between 4ke and Host - May 2010 */
    uint8 XtmTxDownFlags[XTM_TX_CHANNELS_MAX];

    /* ManagedMemory replaces TxKeys, txSources, txAddresses, enet & xtm rx/tx BDs - Apr 2010 */
    uint8   ManagedMemory[FAP_PSM_MANAGED_MEMORY_SIZE];
    uint32 scribble3;
    uint8 * pManagedMemory;
    uint32 scribble4;

#if defined(CC_FAP4KE_PKT_GSO)
    fap4kePsm_gso_t gso;
#endif

#if defined(CC_FAP4KE_TRACE)
    fap4kePsm_trace_t trace;
#endif

#if defined(ENABLE_FAP_COMMS_DEBUG)
    int debug_ctrs[5];
#endif

    int blockHalt;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    fap4kePsm_bpm_t bpm;
#endif
    fap4kePsm_packet_t  packet;
    uint16              mtuOverride;
#if defined(CONFIG_BCM963268)
    XtmRtPtmBondInfo   ptmBondInfo;
#endif    
} fap4kePsm_global_t;

typedef union {
    uint8 u8[FAP_PSM_SIZE];
    uint32 u32[FAP_PSM_SIZE_32];

    fap4kePsm_global_t global;
} fap4kePsm_alloc_t;


/***************************************************
 * Queue Shared Memory (QSM) Mappings
 ***************************************************/
#define p4keQsm ( (fap4keQsm_alloc_t *)(FAP_4KE_QSM_BASE) )
#define p4keQsmGbl ( &p4keQsm->global )

/*
 * fap4keQsm_packet_t: Packet Manager Variables, including flow definitions
 */
typedef struct {
    fap4kePkt_flow_t flowPool[FAP4KE_PKT_MAX_QSM_FLOWS];    
    fap4kePkt_shared_t shared[FAP4KE_PKT_CMDLISTS_IN_QSM];
} fap4keQsm_packet_t;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
typedef struct {
    uint16 lo;
    uint16 hi;
    uint32 dropped;
} fap4ke_iq_thresh_t;

typedef struct {
    uint16_t port;          /* dest port */
    uint8_t  ent    :1;     /* static entry */
    uint8_t  unused :4;     /* unused */
    uint8_t  prio   :3;     /* prio */
    uint8_t  nextIx;        /* overflow bucket index */
} fap4ke_iq_hent_t;

typedef struct {
    uint8_t count;
    uint8_t nextIx;
} fap4ke_free_ovfl_ent_t;

#define FAP_IQ_HASHTBL_SIZE             64
#define FAP_IQ_OVFLTBL_SIZE             64

typedef enum {
    FAP_IQ_L4PROTO_TCP,
    FAP_IQ_L4PROTO_UDP,
    FAP_IQ_L4PROTO_MAX
} fap4ke_iq_L4proto_t;

typedef struct {
    fap4ke_iq_thresh_t enetDqmThresh[ENET_RX_CHANNELS_MAX];
    fap4ke_iq_thresh_t xtmDqmThresh[XTM_RX_CHANNELS_MAX];

     /* Main Hash Table(s): for UDP and TCP */
    fap4ke_iq_hent_t  htbl[FAP_IQ_L4PROTO_MAX][FAP_IQ_HASHTBL_SIZE];

    /* Overflow Table(s): for UDP and TCP */
    fap4ke_iq_hent_t  ovfl_tbl[FAP_IQ_L4PROTO_MAX][FAP_IQ_OVFLTBL_SIZE];

    /* Free Overflow Entry List(s): for UDP and TCP */
    fap4ke_free_ovfl_ent_t free_ovfl_list[FAP_IQ_L4PROTO_MAX];
} fap4keQsm_iq_t;
#endif

typedef struct {
    uint32 scribble0;
    fap4keQsm_packet_t packet;
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    fap4keQsm_iq_t iq;
#endif
    uint32 scribble1;
} fap4keQsm_global_t;

typedef union {
    uint8 u8[FAP_QSM_SIZE];
    uint32 u32[FAP_QSM_SIZE / 4];
    fap4keQsm_global_t global;
} fap4keQsm_alloc_t;

/***************************************************
 * Miscellaneous
 ***************************************************/

#define FAP4KE_PSM_SCRIBBLE_0 sizeof(fap4kePsm_global_t)
#define FAP4KE_PSM_SCRIBBLE_1 0x12345678
#define FAP4KE_PSM_SCRIBBLE_2 0x12345678
#define FAP4KE_PSM_SCRIBBLE_3 0x12345678
#define FAP4KE_PSM_SCRIBBLE_4 0x12345678


#define FAP4KE_DSP_SCRIBBLE_0 sizeof(fap4keDspram_global_t)
#define FAP4KE_DSP_SCRIBBLE_1 0x12345678


#define FAP4KE_QSM_SCRIBBLE_0 sizeof(fap4keQsm_global_t)
#define FAP4KE_QSM_SCRIBBLE_1 0x12345678


fapRet fap4keHw_dspramCheck(int isFirstTime);
fapRet fap4keHw_psmCheck(int isFirstTime);
fapRet fap4keHw_qsmCheck(int isFirstTime);


#endif  /* defined(__FAP4KE_MEMORY_H_INCLUDED__) */
