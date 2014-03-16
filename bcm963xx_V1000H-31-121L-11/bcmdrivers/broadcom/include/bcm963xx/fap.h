#ifndef __FAP_H_INCLUDED__
#define __FAP_H_INCLUDED__

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
 * File Name  : fap.h
 *
 * Description: This file contains the specification of some common definitions
 *      and interfaces to other modules. This file may be included by both
 *      Kernel and userapp (C only).
 *
 *******************************************************************************
 */

#include <pktHdr.h>

/*----- Defines -----*/

#define FAP_VERSION              "0.1"
#define FAP_VER_STR              "v" FAP_VERSION " " __DATE__ " " __TIME__
#define FAP_MODNAME              "Broadcom Forwarding Assist Processor (FAP)"

#define FAP_NAME                 "bcmfap"

#ifndef FAP_ERROR
#define FAP_ERROR                (-1)
#endif
#ifndef FAP_SUCCESS
#define FAP_SUCCESS              0
#endif

/* FAP Character Device */
#define FAPDRV_MAJOR             241
#define FAPDRV_NAME              FAP_NAME
#define FAPDRV_DEVICE_NAME       "/dev/" FAPDRV_NAME

/* FAP Control Utility Executable */
#define FAP_CTL_UTILITY_PATH     "/bin/fapctl"

/* FAP Proc FS Directory Path */
#define FAP_PROC_FS_DIR_PATH     FAP_NAME

/* Menuconfig: BRCM_DRIVER_PKTFLOW_DEBUG selection will cause -DPKTDBG C Flags*/
#ifdef PKTDBG
#define CC_FAP_DEBUG
#define CC_FAP_ASSERT
#endif

#if defined( __KERNEL__ )
#include <asm/system.h>             /* interrupt locking for MIPS */
#define KERNEL_LOCK(level)          local_irq_save(level)
#define KERNEL_UNLOCK(level)        local_irq_restore(level)
#endif

/*
 *------------------------------------------------------------------------------
 * Common defines for FAP layers.
 *------------------------------------------------------------------------------
 */
#define FAP_DECL(x)                 #x, /* for string declaration in C file */
#undef FAP_DECL
#define FAP_DECL(x)                 x,  /* for enum declaration in H file */

/*
 *------------------------------------------------------------------------------
 *              Packet CFM character device driver IOCTL enums
 * A character device and the associated userspace utility for design debug.
 * Include fapParser.h for ACTIVATE/DEACTIVATE IOCTLs
 *------------------------------------------------------------------------------
 */
typedef enum {
    FAP_DECL(FAP_IOC_HW)
    FAP_DECL(FAP_IOC_STATUS)
    FAP_DECL(FAP_IOC_INIT)
    FAP_DECL(FAP_IOC_ENABLE)
    FAP_DECL(FAP_IOC_DISABLE)
    FAP_DECL(FAP_IOC_DEBUG)
    FAP_DECL(FAP_IOC_PRINT)
    FAP_DECL(FAP_IOC_CPU)
    FAP_DECL(FAP_IOC_DMA_DEBUG)
    FAP_DECL(FAP_IOC_MEM_DEBUG)
    FAP_DECL(FAP_IOC_MTU)
    FAP_DECL(FAP_IOC_MAX)
} fapIoctl_t;

/*
 *------------------------------------------------------------------------------
 * Flow cache uses a 16bit CMF Tuple for identifying a Flow accelerated in
 * hardware. As there is only a single FAP engine shared by all ports, the
 * entire 16bit value of the tuple is used to represent the matched Flow.
 *
 * Macros shared by FlowCache for NATed configurations of FAP.
 *------------------------------------------------------------------------------
 */
typedef enum {
    FAP_DECL(PKTCMF_ENGINE_SWC)
    FAP_DECL(PKTCMF_ENGINE_MCAST)
    FAP_DECL(PKTCMF_ENGINE_ARL)
    FAP_DECL(PKTCMF_ENGINE_ALL) /* max number of CMF enum */
} PktCmfEngine_t;


#define CMF_TUPLE16_MCAST_MASK    (1<<12)  /* must be an integer power of 2 */
#define CMF_TUPLE16_ARL_MASK      (1<<13)  /* must be an integer power of 2 */

/* Construct a 16bit CMF tuple from the Engine and matched FlowInfo Element. */
#define CMF_TUPLE16(eng,matchIx)                                        \
    ( (eng == PKTCMF_ENGINE_MCAST) ?                                    \
      (__force uint16_t)(matchIx | CMF_TUPLE16_MCAST_MASK) :            \
      ( (eng == PKTCMF_ENGINE_ARL) ?                                      \
        (__force uint16_t)(CMF_TUPLE16_ARL_MASK) :                        \
        (__force uint16_t)(matchIx) ) )

/* Extract the Engine (CMF memory space) from the CMF <Engine,MatchIx> tuple. */
extern uint8_t GET_CMF_ENGINE(uint16_t cmfTuple16);


/* Extract the matched FlowINFO RAM index from the CMF tuple. */
extern uint16_t GET_CMF_MATCHIX(uint16_t cmfTuple16);


/* Special tuple to signify an invalid tuple. */
#define CMF_TUPLE16_INVALID         ((__force uint16_t)(0xFFFF))

#define PKTCMF_MAX_FLOWS            512


//#define CC_FAP_ENET_STATS

#if defined(CC_FAP_ENET_STATS)
void fapEnetStats_contextFull(void);
void fapEnetStats_dqmRxFull(void);
void fapEnetStats_rxPackets(void);
void fapEnetStats_txPackets(uint32_t contextCount);
void fapEnetStats_interrupts(void);
void fapEnetStats_dump(void);
#else
#define fapEnetStats_contextFull()
#define fapEnetStats_dqmRxFull()
#define fapEnetStats_rxPackets()
#define fapEnetStats_txPackets(_contextCount)
#define fapEnetStats_interrupts()
#define fapEnetStats_dump()
#endif


//#define CC_FAP_EVENTS

#if defined(CC_FAP_EVENTS)
#undef FAP_DECL
#define FAP_DECL(x) #x,

#define FAP_EVENT_TYPE_NAME                     \
    {                                           \
        FAP_DECL(RX_BEGIN)            \
        FAP_DECL(RX_END)          \
        FAP_DECL(TX_SCHED)    \
        FAP_DECL(TX_BEGIN)    \
        FAP_DECL(TX_END)      \
    }

typedef enum {
    FAP_EVENT_RX_BEGIN,
    FAP_EVENT_RX_END,
    FAP_EVENT_TX_SCHED,
    FAP_EVENT_TX_BEGIN,
    FAP_EVENT_TX_END,
    FAP_EVENT_MAX
} fapEvent_type_t;

void fapEvent_record(fapEvent_type_t type, uint32_t arg);
void fapEvent_print(void);
uint32_t fapEnet_txQueueUsage(uint32 fapIdx);
#else
#define fapEvent_record(_type, _arg)
#define fapEvent_print()
#define fapEnet_txQueueUsage() 0
#endif

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
/*
 *------------------------------------------------------------------------------
 *  Invoked by ARL Protocol layer to clear HW association.
 *  Based on the scope of the request:
 *------------------------------------------------------------------------------
 */

typedef int ( *FAP_CLEAR_HOOK)(uint32_t mcast, uint32_t port);

/*
 *------------------------------------------------------------------------------
 * Flow cache binding to ARL to register ARL upcalls and downcalls
 * Upcalls from FAP to ARL: activate, deactivate and refresh functions.
 * Downcalls from ARL to FAP: clear hardware associations function.
 *------------------------------------------------------------------------------
 */
extern void fap_bind_arl(HOOKP activate_fn, HOOK4PARM deactivate_fn,
                        HOOK3PARM refresh_fn, HOOK16 clear_fn,
                        FAP_CLEAR_HOOK *fap_clear_fn);
#endif

#endif  /* defined(__FAP_H_INCLUDED__) */
