/*
<:copyright-BRCM:2004:DUAL/GPL:standard

   Copyright (c) 2004 Broadcom Corporation
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
// File Name  : Adsl.h
//
// Description: This file contains the definitions, structures and function
//              prototypes for ADSL PHY interface
//
//**************************************************************************
#if !defined(_BCMADSL_H_)
#define _BCMADSL_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* Incldes. */
#ifndef _CFE_
#include <linux/version.h>
#endif
#include "AdslMibDef.h"

#ifndef _CFE_
#define DSL_KTHREAD
#endif

//**************************************************************************
// Type Definitions
//**************************************************************************

// Return status values
typedef enum BcmAdslStatus
{
	BCMADSL_STATUS_SUCCESS = 0,
	BCMADSL_STATUS_ERROR
} BCMADSL_STATUS;

// Return status values
typedef enum AdslLinkState
{
	BCM_ADSL_LINK_UP = 0,
	BCM_ADSL_LINK_DOWN,
	BCM_ADSL_TRAINING_G992_EXCHANGE,
	BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS,
	BCM_ADSL_TRAINING_G992_STARTED,
	BCM_ADSL_TRAINING_G993_EXCHANGE,
	BCM_ADSL_TRAINING_G993_CHANNEL_ANALYSIS,
	BCM_ADSL_TRAINING_G993_STARTED,		
	BCM_ADSL_TRAINING_G994,
	BCM_ADSL_G994_NONSTDINFO_RECEIVED,
	BCM_ADSL_BERT_COMPLETE,
	BCM_ADSL_ATM_IDLE,
	BCM_ADSL_EVENT,
	BCM_ADSL_G997_FRAME_RECEIVED,
	BCM_ADSL_G997_FRAME_SENT
} ADSL_LINK_STATE;

#ifndef DISABLE_ADSL_OLD_DEF
#define	ADSL_LINK_UP		BCM_ADSL_LINK_UP
#define	ADSL_LINK_DOWN		BCM_ADSL_LINK_DOWN
#endif

/* ADSL test modes */
typedef enum AdslTestMode
{
	ADSL_TEST_NORMAL = 0,
	ADSL_TEST_REVERB,
	ADSL_TEST_MEDLEY,
	ADSL_TEST_SELECT_TONES,
	ADSL_TEST_NO_AUTO_RETRAIN,
	ADSL_TEST_MARGIN_TWEAK,
	ADSL_TEST_ESTIMATE_PLL_PHASE,
	ADSL_TEST_REPORT_PLL_PHASE_STATUS,
	ADSL_TEST_AFELOOPBACK,
	ADSL_TEST_L3,
	ADSL_TEST_DIAGMODE,
	ADSL_TEST_L0,
	ADSL_TEST_FREEZE_REVERB = 20,
	ADSL_TEST_FREEZE_MEDLEY
} ADSL_TEST_MODE;

// ADSL_CHANNEL_ADDR Contains ADSL Utopia PHY addresses
typedef struct AdslChannelAddr
{
	UINT16 usFastChannelAddr;
	UINT16 usInterleavedChannelAddr;
} ADSL_CHANNEL_ADDR, *PADSL_CHANNEL_ADDR;

// ADSL_CONNECTION_INFO Contains ADSL Connection Info
typedef struct AdslConnectionInfo
{
	ADSL_LINK_STATE LinkState;
//LGD_FOR_TR098
	unsigned long ShowtimeStart;
	UINT32 ulFastUpStreamRate;
	UINT32 ulFastDnStreamRate;
	UINT32 ulInterleavedUpStreamRate;
	UINT32 ulInterleavedDnStreamRate;
#if defined (AEI_VDSL_CUSTOMER_CENTURYLINK)
	UINT32 demodCapabilities;
	UINT32 demodCapabilities2;
#endif
#if defined(AEI_VDSL_MEDIASEARCH_MOD) 
	UINT32	mediaSearch; /* use to determine if linkup state is during mediasearch, 32 bit to maintain alignment */
#endif
#if defined(AEI_VDSL_ADSL_BONDING_AUTODETECT)
	UINT32	knowBondingMode;    /* bonding/non-bonding mode info to send so config manager can delay autopvc during transition */
#endif
} ADSL_CONNECTION_INFO, *PADSL_CONNECTION_INFO;

/* OEM parameter definition */
#define ADSL_OEM_G994_VENDOR_ID       1    /* Vendor ID used during G.994 handshake */
#define ADSL_OEM_G994_XMT_NS_INFO     2    /* G.994 non-standard info field to send */
#define ADSL_OEM_G994_RCV_NS_INFO     3    /* G.994 received non-standard */
#define ADSL_OEM_EOC_VENDOR_ID        4    /* EOC reg. 0 */
#define ADSL_OEM_EOC_VERSION          5    /* EOC reg. 1 */
#define ADSL_OEM_EOC_SERIAL_NUMBER    6    /* EOC reg. 2 */
#define ADSL_OEM_T1413_VENDOR_ID      7    /* Vendor ID used during T1.413 handshake */
#define ADSL_OEM_T1413_EOC_VENDOR_ID  8    /* EOC reg. 0 (vendor ID) in T1.413 mode */

/* XMT gain definitions */
#define ADSL_XMT_GAIN_AUTO			  0x80000000

#ifdef SUPPORT_MULTI_PHY
#define	BCM_IMAGETYPE_SHIFT			0
#define	BCM_IMAGETYPE_MSK			(1 << BCM_IMAGETYPE_SHIFT)
#define	BCM_IMAGETYPE_BONDING		(0 << BCM_IMAGETYPE_SHIFT)
#define	BCM_IMAGETYPE_SINGLELINE	(1 << BCM_IMAGETYPE_SHIFT)
#define	BCM_XDSLMODE_SHIFT			1
#define	BCM_XDSLMODE_MSK			(1 << BCM_XDSLMODE_SHIFT)
#define	BCM_VDSL_MODE				(0 << BCM_XDSLMODE_SHIFT)
#define	BCM_ADSL_MODE				(1 << BCM_XDSLMODE_SHIFT)
#define	BCM_SWITCH_SEARCH_SHIFT		2
#define	BCM_SWITCH_SEARCH_MSK		(3 << BCM_SWITCH_SEARCH_SHIFT)
#define	BCM_SWITCHPHY_DISABLED		(1 << BCM_SWITCH_SEARCH_SHIFT)
#define	BCM_MEDIASEARCH_DISABLED	(2 << BCM_SWITCH_SEARCH_SHIFT)
#define	BCM_MEDIATYPE_SHIFT			4
#define	BCM_MEDIATYPE_MSK			(3 << BCM_MEDIATYPE_SHIFT)
#define	BCM_MEDIATYPE_INTERNALAFE	(0 << BCM_MEDIATYPE_SHIFT)
#define	BCM_MEDIATYPE_EXTERNALAFE	(1 << BCM_MEDIATYPE_SHIFT)
#endif

typedef struct
{
	int				diagCmd;
	int				diagMap;
	int				logTime;
	int				srvIpAddr;
	int				gwIpAddr;
} ADSL_DIAG, *PADSL_DIAG;

typedef struct
{
	short			x;
	short			y;
} ADSL_CONSTELLATION_POINT, *PADSL_CONSTELLATION_POINT;

#define ADSL_CONSTEL_DATA_ID		0
#define ADSL_CONSTEL_PILOT_ID		1

#define ADSL_MIB_INFO	adslMibInfo
typedef	ADSL_MIB_INFO   *PADSL_MIB_INFO;

typedef void (*ADSL_FN_NOTIFY_CB) (unsigned char lineId, ADSL_LINK_STATE dslLinkState, ADSL_LINK_STATE dslPrevLinkState, UINT32 ulParm);

typedef struct {
	int		nStatus;		/* Total number of statuses saved */
	void		*pAddr;		/* Point to the first saved status */
	int		len;			/* Lenght of saved statuses */
	void	*pAddr1;	/* Point to the first wrap-around saved status */
	int		len1;		/* Lenght of the wrap-around saved statuses */
} ADSL_SAVEDSTATUS_INFO, *PADSL_SAVEDSTATUS_INFO;

//**************************************************************************
// Function Prototypes
//**************************************************************************

BCMADSL_STATUS BcmAdsl_Check(void);
#ifdef LINUX
BCMADSL_STATUS BcmAdsl_MapAtmPortIDs(UINT16 usAtmFastPortId, UINT16 usAtmInterleavedPortId);
#endif

#if defined(__KERNEL__) || defined(_CFE_)
BCMADSL_STATUS BcmAdsl_Initialize(unsigned char lineId, ADSL_FN_NOTIFY_CB pFnNotifyCb, UINT32 ulParm, adslCfgProfile *pAdslCfg);
BCMADSL_STATUS BcmAdsl_Uninitialize(unsigned char lineId);
BCMADSL_STATUS BcmAdsl_ConnectionStart(unsigned char lineId);
BCMADSL_STATUS BcmAdsl_ConnectionStop(unsigned char lineId);
BCMADSL_STATUS BcmAdsl_GetConnectionInfo(unsigned char lineId, PADSL_CONNECTION_INFO pConnectionInfo);
BCMADSL_STATUS BcmAdsl_DiagCommand(unsigned char lineId, PADSL_DIAG pAdslDiag);
BCMADSL_STATUS BcmAdsl_StartBERT(unsigned char lineId, unsigned long totalBits);
BCMADSL_STATUS BcmAdsl_StopBERT(unsigned char lineId);
BCMADSL_STATUS BcmAdsl_BertStartEx(unsigned char lineId, unsigned long bertSec);
BCMADSL_STATUS BcmAdsl_BertStopEx(unsigned char lineId);
BCMADSL_STATUS BcmAdsl_Configure(unsigned char lineId, adslCfgProfile *pAdslCfg);
BCMADSL_STATUS BcmAdsl_ResetStatCounters(unsigned char lineId);
BCMADSL_STATUS BcmAdsl_SetTestMode(unsigned char lineId, ADSL_TEST_MODE testMode);
BCMADSL_STATUS BcmAdsl_SelectTones(
					unsigned char lineId,
					int		xmtStartTone,
					int		xmtNumTones,
					int		rcvStartTone,
					int		rcvNumTones,
					char	*xmtToneMap,
					char	*rcvToneMap
					);
BCMADSL_STATUS BcmAdsl_SetDiagMode(unsigned char lineId, int diagMode);
int BcmAdsl_SetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
int BcmAdsl_GetObjectValue(unsigned char lineId, char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
BCMADSL_STATUS BcmAdsl_Initialize(ADSL_FN_NOTIFY_CB pFnNotifyCb, UINT32 ulParm, adslCfgProfile *pAdslCfg);
BCMADSL_STATUS BcmAdsl_Uninitialize(void);
BCMADSL_STATUS BcmAdsl_ConnectionStart(void);
BCMADSL_STATUS BcmAdsl_ConnectionStop(void);
BCMADSL_STATUS BcmAdsl_GetConnectionInfo(PADSL_CONNECTION_INFO pConnectionInfo);
BCMADSL_STATUS BcmAdsl_DiagCommand(PADSL_DIAG pAdslDiag);
BCMADSL_STATUS BcmAdsl_StartBERT(unsigned long totalBits);
BCMADSL_STATUS BcmAdsl_StopBERT(void);
BCMADSL_STATUS BcmAdsl_BertStartEx(unsigned long bertSec);
BCMADSL_STATUS BcmAdsl_BertStopEx(void);
BCMADSL_STATUS BcmAdsl_Configure(adslCfgProfile *pAdslCfg);
BCMADSL_STATUS BcmAdsl_ResetStatCounters(void);
BCMADSL_STATUS BcmAdsl_SetTestMode(ADSL_TEST_MODE testMode);
BCMADSL_STATUS BcmAdsl_SelectTones(
					int 	xmtStartTone,
					int 	xmtNumTones,
					int 	rcvStartTone,
					int 	rcvNumTones,
					char	*xmtToneMap,
					char	*rcvToneMap
					);
int BcmAdsl_SetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
int BcmAdsl_GetObjectValue(char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
#endif

BCMADSL_STATUS BcmAdsl_GetPhyAddresses(PADSL_CHANNEL_ADDR pChannelAddr);
BCMADSL_STATUS BcmAdsl_SetPhyAddresses(PADSL_CHANNEL_ADDR pChannelAddr);
BCMADSL_STATUS BcmAdsl_CheckPowerLoss(void);
BCMADSL_STATUS BcmAdsl_SendDyingGasp(int powerCtl);
BCMADSL_STATUS BcmAdsl_GetVersion(adslVersionInfo *pAdslVer);
#if defined(AEI_ADSL_SINGLE_LINE_PORT_MATCH) //add william 2012-9-19
BCMADSL_STATUS BcmAdsl_GetCurrentPort(unsigned int *pCurport);
#endif
BCMADSL_STATUS BcmAdsl_SetSDRAMBaseAddr(void *pAddr);
BCMADSL_STATUS BcmAdsl_SetVcEntry (int gfc, int port, int vpi, int vci);
BCMADSL_STATUS BcmAdsl_SetVcEntryEx (int gfc, int port, int vpi, int vci, int pti_clp);
BCMADSL_STATUS BcmAdsl_SetAtmLoopbackMode(void);

int BcmAdsl_GetConstellationPoints (int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints);
int BcmAdsl_GetOemParameter (int paramId, void *buf, int len);
int BcmAdsl_SetOemParameter (int paramId, void *buf, int len);
int BcmAdsl_SetXmtGain(unsigned char lineId, int gain);

UINT32 BcmAdsl_GetSelfTestMode(void);
void BcmAdsl_SetSelfTestMode(UINT32 stMode);
UINT32 BcmAdsl_GetSelfTestResults(void);

BCMADSL_STATUS BcmAdsl_G997SendData(unsigned char lineId, void *buf, int len);
void *BcmAdsl_G997FrameGet(unsigned char lineId, int *pLen);
void *BcmAdsl_G997FrameGetNext(unsigned char lineId, int *pLen);
void  BcmAdsl_G997FrameFinished(unsigned char lineId);
void BcmAdsl_DyingGaspHandler(void *context);

void BcmAdsl_AtmClearVcTable(void);
void BcmAdsl_AtmAddVc(int vpi, int vci);
void BcmAdsl_AtmDeleteVc(int vpi, int vci);
void BcmAdsl_AtmSetMaxSdu(unsigned short maxsdu);
void BcmAdsl_AtmSetPortId(int path, int portId) ;

BCMADSL_STATUS BcmAdsl_DiagStatSaveInit(void *pAddr, int len);
BCMADSL_STATUS BcmAdsl_DiagStatSaveContinous(void);
BCMADSL_STATUS BcmAdsl_DiagStatSaveStart(void);
BCMADSL_STATUS BcmAdsl_DiagStatSaveStop(void);
BCMADSL_STATUS BcmAdsl_DiagStatSaveUnInit(void);
BCMADSL_STATUS BcmAdsl_DiagStatSaveGet(PADSL_SAVEDSTATUS_INFO pSavedStatInfo);

BCMADSL_STATUS BcmXdsl_CallBackDrv(unsigned char lineId);

#if defined(__cplusplus)
}
#endif

#endif // _BCMADSL_H_

