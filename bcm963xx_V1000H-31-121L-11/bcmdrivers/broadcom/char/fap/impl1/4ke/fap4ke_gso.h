#ifndef __FAP4KE_GSO_H_INCLUDED__
#define __FAP4KE_GSO_H_INCLUDED__

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
 * File Name  : fap4ke_gso.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#define CC_FAP4KE_PKT_GSO		/* Enable GSO support by default - July 2010 */
#define CC_FAP4KE_PKT_GSO_FRAG

//#define CC_FAP4KE_GSO_DEBUG
//#define CC_FAP4KE_GSO_ERROR_CHECK
//#define CC_FAP4KE_GSO_PMON_ENABLE

#define FAP4KE_GSO_CMD_LIST_SIZE      48
#define FAP4KE_GSO_CSUM_LISTS         2
#define FAP4KE_GSO_CSUM_LIST_SIZE     28

#define FAP4KE_GSO_NUM_BUFFERS        256 /* MUST BE A INTEGER MULTIPLE OF 32 */

#define FAP4KE_GSO_BUFFER_MAP_SIZE_32 (FAP4KE_GSO_NUM_BUFFERS / 32)

#define FAP4KE_GSO_BUFFER_SIZE_SHIFT  12 /* 4096 */
#define FAP4KE_GSO_BUFFER_SIZE        (1 << FAP4KE_GSO_BUFFER_SIZE_SHIFT)

#if defined(CC_FAP4KE_GSO_DEBUG)
#define FAP4KE_GSO_ASSERT(condition) fap4kePrt_Assert(condition)
#define FAP4KE_GSO_DEBUG(fmt, arg...) fap4kePrt_Debug(fmt, ##arg)
#define FAP4KE_GSO_PRINT(fmt, arg...) fap4kePrt_Print(fmt, ##arg)
#define FAP4KE_GSO_ERROR(fmt, arg...) fap4kePrt_Error(fmt, ##arg)
#define FAP4KE_GSO_DUMP_PACKET(_packet_p, _length) dumpPacket(_packet_p, _length)
#else
#define FAP4KE_GSO_ASSERT(condition)
#define FAP4KE_GSO_DEBUG(fmt, arg...)
#define FAP4KE_GSO_PRINT(fmt, arg...)
#define FAP4KE_GSO_ERROR(fmt, arg...)
#define FAP4KE_GSO_DUMP_PACKET(_packet_p, _length)
#endif

#if defined(CC_FAP4KE_GSO_PMON_ENABLE)
#define FAP4KE_GSO_PMON_DECLARE() FAP4KE_PMON_DECLARE()
#define FAP4KE_GSO_PMON_BEGIN(_pmonId) FAP4KE_PMON_BEGIN(_pmonId)
#define FAP4KE_GSO_PMON_END(_pmonId) FAP4KE_PMON_END(_pmonId)
#else
#define FAP4KE_GSO_PMON_DECLARE()
#define FAP4KE_GSO_PMON_BEGIN(_pmonId)
#define FAP4KE_GSO_PMON_END(_pmonId)
#endif


/* Maximum header size:
 * 14 (ETH) + 8 (2 VLANs) +
 * 8 (PPPoE) + 20 (IPv4) + 60 (TCP/UDP) = 110 bytes
 */
#define FAP4KE_GSO_HEADER_SIZE      110

typedef struct {
    uint8 cmdList[FAP4KE_GSO_CMD_LIST_SIZE];
    uint8 csumList[FAP4KE_GSO_CSUM_LISTS][FAP4KE_GSO_CSUM_LIST_SIZE];
} __attribute__((aligned(4))) fap4keGso_iopDmaCmd_t;

typedef struct {
    uint32 packets;
    uint32 bytes;
} fap4keGso_stats_t;

typedef struct {
    uint8 header[FAP4KE_GSO_HEADER_SIZE];
    fap4keGso_iopDmaCmd_t iopDmaCmd[IOPDMA_DMA_DEPTH];
    fap4keGso_stats_t stats;
} fap4keGso_shared_t;

typedef struct {
    uint8 logTable256[256];
    uint8 *buffer_p;
    uint32 freeBufferMap[FAP4KE_GSO_BUFFER_MAP_SIZE_32];
} fap4keGso_bufferMgmt_t;

typedef struct {
    fap4keGso_bufferMgmt_t bufferMgmt;
} fap4keGso_runtime_t;

void fap4keGso_init(void);

int fap4keGso_start(fap4kePkt_gso_pkt *pGsoPkt, fap4kePkt_gso_arg *pGsoArg);
int freeTxBuffersGso(fap4kePkt_phy_t phy, uint8 *txAddr, uint32 key, int source, int rxChannel);

#if defined(CONFIG_BCM963268) && (CONFIG_BCM_EXT_SWITCH)
int fap4keGso_checksum(uint32 channel, uint8 *packet_p, int len, uint32 encapType, uint8 isExtSwitch);
#else
int fap4keGso_checksum(uint32 channel, uint8 *packet_p, int len, uint32 encapType);
#endif

void fap4keGso_freeBuffer(uint8 *gsoBuffer_p);

#endif  /* defined(__FAP4KE_GSO_H_INCLUDED__) */
