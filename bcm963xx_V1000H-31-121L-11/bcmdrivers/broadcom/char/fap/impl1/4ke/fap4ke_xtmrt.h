/***********************************************************
 *
 * <:copyright-BRCM:2009:DUAL/GPL:standard
 * 
 *    Copyright (c) 2009 Broadcom Corporation
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 ***********************************************************/

#ifndef __FAP4KE_XTMRT_H_INCLUDED__
#define __FAP4KE_XTMRT_H_INCLUDED__


 /*****************************************************************************
 * File Name  : fap4ke_xtmrt.h
 *
 * Description: This file contains the constants and prototypes needed for the
 *              XTMRT Driver running on the 4ke.
 *
 *****************************************************************************/

#include "bcmPktDma_defines.h"

#define FAP4KE_XTM_MAX_DEV_CTXS   16    /* up to 256 */
#define FAP4KE_XTM_MAX_MATCH_IDS  128

#define XTM_TX_VCID_MASK    0xf

#define pXtmCtrl ( &p4keDspramGbl->xtm )

typedef struct {
    uint16 encapType;
    uint8  headerLen;
    uint8  trailerLen;
} fap4keXtm_devContext_t;

typedef struct {
    fap4keXtm_devContext_t devContext[FAP4KE_XTM_MAX_DEV_CTXS];
    uint8 matchIdToDevId[FAP4KE_XTM_MAX_MATCH_IDS];
} fap4keXtm_devMap_t;

typedef struct {
    int numTxQueues;
    int numTxBufsQdAll;
    uint16 numTxBufsRsrvdPerQueue[XTM_TX_CHANNELS_MAX];
} fap4keXtm_qos_t;

/* XTMRT Driver Prototypes */
fapRet xtmInit(int isFirstTime);
fapRet xtmRxChannelInit(uint32 channel, uint32 numBds, uint32 Bds, uint32 Dma);
fapRet xtmTxChannelInit(uint32 channel, uint32 numBds, uint32 Bds, uint32 Dma);
fapRet xtmCreateDevice(uint32 devId, uint32 encapType, uint32 headerLen, uint32 trailerLen);
fapRet xtmLinkUp(uint32 devId, uint32 matchId);
void xtmDmaStatus(int channel);
void xtmRxDmaStatus(int channel);
void xtmTxDmaStatus(int channel);

/* For use by fap4ke_ffe.c fap4ke_enet.c and fap4ke_hostIf.c */
void xtmFreeTxBuffers(uint32 channel, int forceFree);
void xtmFreeRecvBuf(int channel, unsigned char *pBuf);

void xtmTxCleanup(void);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
int  xtmBpmAllocBufResp( uint32 channel, uint32 seqId );
void xtmBpmFreeBuf(int channel, uint8 *pData);
void xtmBpmFreeBufResp( uint32 channel, uint32 seqId );
int  xtmFlushIudmaRings( void );
#endif

#endif  /* defined(__FAP4KE_XTMRT_H_INCLUDED__) */
