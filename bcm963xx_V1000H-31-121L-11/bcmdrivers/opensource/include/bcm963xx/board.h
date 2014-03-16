/*
<:copyright-BRCM:2002:GPL/GPL:standard

   Copyright (c) 2002 Broadcom Corporation
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
/***********************************************************************/
/*                                                                     */
/*   MODULE:  board.h                                                  */
/*   PURPOSE: Board specific information.  This module should include  */
/*            all base device addresses and board specific macros.     */
/*                                                                     */
/***********************************************************************/
#ifndef _BOARD_H
#define _BOARD_H

#include "bcm_hwdefs.h"

#ifdef __cplusplus
extern "C" {
#endif
#define BOARD_H_API_VER 2

/*****************************************************************************/
/*          board ioctl calls for flash, led and some other utilities        */
/*****************************************************************************/
/* Defines. for board driver */
#define BOARD_IOCTL_MAGIC       'B'
#define BOARD_DRV_MAJOR          206

#define BOARD_IOCTL_FLASH_WRITE                 _IOWR(BOARD_IOCTL_MAGIC, 0, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_FLASH_READ                  _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_DUMP_ADDR                   _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_MEMORY                  _IOWR(BOARD_IOCTL_MAGIC, 3, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_MIPS_SOFT_RESET             _IOWR(BOARD_IOCTL_MAGIC, 4, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_LED_CTRL                    _IOWR(BOARD_IOCTL_MAGIC, 5, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_PSI_SIZE                _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_SDRAM_SIZE              _IOWR(BOARD_IOCTL_MAGIC, 7, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_ID                      _IOWR(BOARD_IOCTL_MAGIC, 8, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_CHIP_ID                 _IOWR(BOARD_IOCTL_MAGIC, 9, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_CHIP_REV                _IOWR(BOARD_IOCTL_MAGIC, 10, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_CFE_VER                 _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BASE_MAC_ADDRESS        _IOWR(BOARD_IOCTL_MAGIC, 12, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_MAC_ADDRESS             _IOWR(BOARD_IOCTL_MAGIC, 13, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_RELEASE_MAC_ADDRESS         _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_ENET_MACS           _IOWR(BOARD_IOCTL_MAGIC, 15, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_ENET_PORTS          _IOWR(BOARD_IOCTL_MAGIC, 16, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_MONITOR_FD              _IOWR(BOARD_IOCTL_MAGIC, 17, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_WAKEUP_MONITOR_TASK         _IOWR(BOARD_IOCTL_MAGIC, 18, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_TRIGGER_EVENT           _IOWR(BOARD_IOCTL_MAGIC, 19, BOARD_IOCTL_PARMS)        
#define BOARD_IOCTL_GET_TRIGGER_EVENT           _IOWR(BOARD_IOCTL_MAGIC, 20, BOARD_IOCTL_PARMS)        
#define BOARD_IOCTL_UNSET_TRIGGER_EVENT         _IOWR(BOARD_IOCTL_MAGIC, 21, BOARD_IOCTL_PARMS) 
#define BOARD_IOCTL_GET_WLAN_ANT_INUSE          _IOWR(BOARD_IOCTL_MAGIC, 22, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_SES_LED                 _IOWR(BOARD_IOCTL_MAGIC, 23, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_CS_PAR                  _IOWR(BOARD_IOCTL_MAGIC, 25, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_GPIO                    _IOWR(BOARD_IOCTL_MAGIC, 26, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_FLASH_LIST                  _IOWR(BOARD_IOCTL_MAGIC, 27, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BACKUP_PSI_SIZE         _IOWR(BOARD_IOCTL_MAGIC, 28, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_SYSLOG_SIZE             _IOWR(BOARD_IOCTL_MAGIC, 29, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_SHUTDOWN_MODE           _IOWR(BOARD_IOCTL_MAGIC, 30, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_STANDBY_TIMER           _IOWR(BOARD_IOCTL_MAGIC, 31, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_BOOT_IMAGE_OPERATION        _IOWR(BOARD_IOCTL_MAGIC, 32, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_TIMEMS                  _IOWR(BOARD_IOCTL_MAGIC, 33, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_GPON_OPTICS_TYPE        _IOWR(BOARD_IOCTL_MAGIC, 34, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_DEFAULT_OPTICAL_PARAMS  _IOWR(BOARD_IOCTL_MAGIC, 35, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_INIT              _IOWR(BOARD_IOCTL_MAGIC, 36, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_READ              _IOWR(BOARD_IOCTL_MAGIC, 37, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_WRITE             _IOWR(BOARD_IOCTL_MAGIC, 38, BOARD_IOCTL_PARMS)


#if defined(AEI_VDSL_CUSTOMER_NCS)
#define BASE_AEI_IOCPRIVATE	50
#define BOARD_IOCTL_GET_SN              _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 1, BOARD_IOCTL_PARMS)    
#define BOARD_IOCTL_POWER_LED_ON        _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 2, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_POWER_LED_FLASH     _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 3, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_FW_VERSION      _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 4, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_RESET_FW_VERSION    _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 5, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_FW_VERSION      _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 6, BOARD_IOCTL_PARMS)

#if defined(AEI_VDSL_SMARTLED)
#define BOARD_IOCTL_SET_INET            _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 7, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_INET_TRAFFIC_BLINK _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 8, BOARD_IOCTL_PARMS)
#endif
#if defined(AEI_CONFIG_JFFS) || defined(AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD)
#define BOARD_IOCTL_GET_DUAL_FW_VERSION _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 9, BOARD_IOCTL_PARMS)
#endif
#ifdef AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD
#define BOARD_IOCTL_GET_DUAL_UG_INFO _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 10, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_DUAL_UG_INFO _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 11, BOARD_IOCTL_PARMS)
#else
#define BOARD_IOCTL_GET_UG_INFO _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 10, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_UG_INFO _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 11, BOARD_IOCTL_PARMS)
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
#define BOARD_IOCTL_GET_PRODUCTID _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 12, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_FS_OFFSET _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 13, BOARD_IOCTL_PARMS)
#endif   

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) 
#define BOARD_IOCTL_GET_POWERLED_STATUS  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 14, BOARD_IOCTL_PARMS) 
#define BOARD_IOCTL_GET_FLASH_TOTAL  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 15, BOARD_IOCTL_PARMS) 
#define BOARD_IOCTL_GET_FLASH_USED  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 16, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_WLANLEDMODE _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE+17, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_WLANLEDMODE _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE+18, BOARD_IOCTL_PARMS)
#define AEI_BOARD_IOCTL_WIRELESS_REDLED    _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE+19, BOARD_IOCTL_PARMS) 

#endif
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
#define BOARD_IOCTL_PARTITION_WRITE  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE+20, BOARD_IOCTL_PARMS)
#endif
#if defined(AEI_VDSL_CUSTOMER_TELUS)    
#define BOARD_IOCTL_ADJUST_NVRAM      _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 21, BOARD_IOCTL_PARMS)
#endif
#endif
#if defined(AEI_VOIP_LED)
#define AEI_BOARD_IOCTL_VOIP_LED _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 22, BOARD_IOCTL_PARMS)
#endif
#if defined(AEI_VDSL_CHECK_FLASH_ID)
#define AEI_BOARD_IOCTL_GET_NAND_FLASH_ID  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 23, BOARD_IOCTL_PARMS)
#endif
#if defined(AEI_VDSL_FACTORY_TELNET)
#define AEI_BOARD_IOCTL_SET_RESTOREDEFAULT_FLAG  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 24, BOARD_IOCTL_PARMS)
#define AEI_BOARD_IOCTL_GET_RESTOREDEFAULT_FLAG  _IOWR(BOARD_IOCTL_MAGIC, BASE_AEI_IOCPRIVATE + 25, BOARD_IOCTL_PARMS)
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
typedef enum
{
    AEI_BOARD_UNKNOWN,
    AEI_BOARD_C1000A,
    AEI_BOARD_C1000B,
    AEI_BOARD_C2000A,
    AEI_BOARD_V1000H,
    AEI_BOARD_V2000H,
    AEI_BOARD_V1000W,
    AEI_BOARD_R1000H,
    AEI_BOARD_FV2200,
    AEI_BOARD_V2200H,
    AEI_BOARD_END
} AEI_BOARD_ID;

#define BP_ACTIVE_HIGH                          0x0000
#define BP_ACTIVE_LOW                           0x8000

#if 0
#if defined(AEI_63168_CHIP) // For 63168
  #define AEI_POWER_ON_LED    (BP_ACTIVE_LOW | 20)
  #define AEI_POST_FAIL_LED   (BP_ACTIVE_LOW | 35)
  #define AEI_INET_ACT_LED    (BP_ACTIVE_LOW | 13)
  #define AEI_INET_FAIL_LED   (BP_ACTIVE_LOW | 12)
#else  // For 6368
  #define AEI_POWER_ON_LED    (BP_ACTIVE_HIGH | 22)
  #define AEI_POST_FAIL_LED   (BP_ACTIVE_HIGH | 24)
  #define AEI_INET_ACT_LED    (BP_ACTIVE_HIGH | 5)
  #define AEI_INET_FAIL_LED   (BP_ACTIVE_HIGH | 31)
#endif
#endif
#endif // AEI_VDSL_CUSTOMER_NCS

    

// for the action in BOARD_IOCTL_PARMS for flash operation
typedef enum 
{
    PERSISTENT,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
    SET_CS_PARAM,
    BACKUP_PSI,
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    SYSLOG,
    SYSLOGONREBOOT
#else
    SYSLOG
#endif
} BOARD_IOCTL_ACTION;
    
typedef struct boardIoctParms
{
    char *string;
    char *buf;
    int strLen;
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;

// LED defines 
typedef enum
{   
    kLedAdsl,
    kLedSecAdsl,
    kLedWanData,
    kLedSes,
    kLedVoip,
    kLedVoip1,
    kLedVoip2,
    kLedPots,
    kLedDect,
    kLedGpon,
    kLedMoCA,
    kLedEth0Duplex,
    kLedEth0Spd100,
    kLedEth0Spd1000,
    kLedEth1Duplex,
    kLedEth1Spd100,
    kLedEth1Spd1000,
#if defined(AEI_VDSL_CUSTOMER_NCS)
    kLedUsb,
    kLedPower,
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    AEI_kLedWlan,
    AEI_kLedWlanAct,
    AEI_kLedWlanGreen,
    AEI_kLedWlanRed,
#endif
#if defined(AEI_63168_CHIP)
    kLedEnetWan,
#endif
#endif
    kLedEnd                             // NOTE: Insert the new led name before this one.
} BOARD_LED_NAME;

typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
#if defined(AEI_VDSL_CUSTOMER_NCS)
    kLedStateAmber,                      /* turn green led on , turn red led on*/
    kLedStateVerySlowBlinkContinues,         /* slow blink continues at 1HZ interval */
#endif
    kLedStateSlowBlinkContinues,         /* slow blink continues at 2HZ interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at 4HZ interval */
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
    kLedStateAmberSlowBlinkContinues,
    kLedStateAmberAndGreenSlowBlinkContinues,
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
    kLedStateUserWANGreenRedVerySlowBlinkContinues, /*rotate between green and red in a 1 second on/off interval*/   
    kLedStatePowerOneSecondBlinkContinues,          /*rotate between green and off in a 1 second on/off interva*/ 
#endif
    kLedStateUserWpsInProgress,          /* 200ms on, 100ms off */
    kLedStateUserWpsError,               /* 100ms on, 100ms off */
    kLedStateUserWpsSessionOverLap       /* 100ms on, 100ms off, 5 times, off for 500ms */                     
} BOARD_LED_STATE;

typedef void (*HANDLE_LED_FUNC)(BOARD_LED_STATE ledState);

typedef enum
{
    kGpioInactive,
    kGpioActive
} GPIO_STATE_t;

// FLASH_ADDR_INFO is now defined in flash_common.h
#include "flash_common.h"

typedef struct cs_config_pars_tag
{
  int   mode;
  int   base;
  int   size;
  int   wait_state;
  int   setup_time;
  int   hold_time;
} cs_config_pars_t;

#define UBUS_BASE_FREQUENCY_IN_MHZ  160

#define IF_NAME_ETH    "eth"
#define IF_NAME_USB    "usb"
#define IF_NAME_WLAN   "wl"
#define IF_NAME_MOCA   "moca"
#define IF_NAME_ATM    "atm"
#define IF_NAME_PTM    "ptm"
#define IF_NAME_GPON   "gpon"
#define IF_NAME_EPON   "epon"
#define IF_NAME_VEIP    "veip"

#define MAC_ADDRESS_ANY         (unsigned long) -1

/* A unique mac id is required for a WAN interface to request for a MAC address.
 * The 32bit mac id is formated as follows:
 *     bit 28-31: MAC Address IF type (MAC_ADDRESS_*)
 *     bit 20-27: the unit number. 0 for atm0, 1 for atm1, ...
 *     bit 12-19: the connection id which starts from 1.
 *     bit 0-11:  not used. should be zero.
 */
 #define MAC_ADDRESS_EPON        0x40000000
#define MAC_ADDRESS_GPON        0x80000000
#define MAC_ADDRESS_ETH         0xA0000000
#define MAC_ADDRESS_USB         0xB0000000
#define MAC_ADDRESS_WLAN        0xC0000000
#define MAC_ADDRESS_MOCA        0xD0000000
#define MAC_ADDRESS_ATM         0xE0000000
#define MAC_ADDRESS_PTM         0xF0000000

#if defined(AEI_VDSL_CUSTOMER_NCS)
#define AEI_MAC_ADDRESS_TYPE_MASK 0xF0000000
#define AEI_MAC_ADDRESS_NUM_MASK  0x0FF00000  /*It is used for atmX now*/
#define AEI_MAC_ADDRESS_ID_MASK   0x000FF000  /*It is used for ewan0.X or ptm0.X now*/
#endif

/*****************************************************************************/
/*          Function Prototypes                                              */
/*****************************************************************************/
#if !defined(__ASM_ASM_H)
void dumpaddr( unsigned char *pAddr, int nLen );

extern void kerSysEarlyFlashInit( void );
extern int kerSysGetChipId( void );
extern int kerSysGetDslPhyEnable( void );
extern void kerSysFlashInit( void );
extern void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info);
extern int kerSysCfeVersionGet(char *string, int stringLength);

extern int kerSysNvRamSet(const char *string, int strLen, int offset);
extern void kerSysNvRamGet(char *string, int strLen, int offset);
extern void kerSysNvRamGetBootline(char *bootline);
extern void kerSysNvRamGetBootlineLocked(char *bootline);
extern void kerSysNvRamGetBoardId(char *boardId);
extern void kerSysNvRamGetBoardIdLocked(char *boardId);
extern void kerSysNvRamGetBaseMacAddr(unsigned char *baseMacAddr);
extern unsigned long kerSysNvRamGetVersion(void);

extern int kerSysPersistentGet(char *string, int strLen, int offset);
extern int kerSysPersistentSet(char *string, int strLen, int offset);
extern int kerSysBackupPsiGet(char *string, int strLen, int offset);
extern int kerSysBackupPsiSet(char *string, int strLen, int offset);
extern int kerSysSyslogGet(char *string, int strLen, int offset);
extern int kerSysSyslogSet(char *string, int strLen, int offset);
extern int kerSysScratchPadList(char *tokBuf, int tokLen);
extern int kerSysScratchPadGet(char *tokName, char *tokBuf, int tokLen);
extern int kerSysScratchPadSet(char *tokName, char *tokBuf, int tokLen);
#if defined(AEI_VDSL_CUSTOMER_NCS)
extern int kerClearScratchPad(int blk_size);
#endif
extern int kerSysScratchPadClearAll(void);
extern int kerSysBcmImageSet( int flash_start_addr, char *string, int size,
    int should_yield);
extern int kerSysSetBootImageState( int state );
extern int kerSysGetBootImageState( void );
extern int kerSysBlParmsGetInt( char *name, int *pvalue );
extern int kerSysBlParmsGetStr( char *name, char *pvalue, int size );
extern unsigned long kerSysGetMacAddressType( unsigned char *ifName );
extern int kerSysGetMacAddress( unsigned char *pucaAddr, unsigned long ulId );
extern int kerSysReleaseMacAddress( unsigned char *pucaAddr );
extern void kerSysGetGponSerialNumber( unsigned char *pGponSerialNumber);
extern void kerSysGetGponPassword( unsigned char *pGponPassword);
extern int kerSysGetSdramSize( void );
#if defined(CONFIG_BCM96368)
extern unsigned int kerSysGetSdramWidth( void );
#endif
extern void kerSysGetBootline(char *string, int strLen);
extern void kerSysSetBootline(char *string, int strLen);
extern void kerSysMipsSoftReset(void);
extern void kerSysLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
extern int kerSysFlashSizeGet(void);
extern int kerSysMemoryMappedFlashSizeGet(void);
extern unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr, unsigned long len );
extern void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context);
extern void kerSysDeregisterDyingGaspHandler(char *devname);    
extern int kerConfigCs(BOARD_IOCTL_PARMS *parms);

/* private functions used within board driver only */
void stopOtherCpu(void);
void resetPwrmgmtDdrMips(void);

/*
 * Access to shared GPIO registers should be done by calling these
 * functions below, which will grab a spinlock while accessing the
 * GPIO register.  However, if your code needs to access a shared
 * GPIO register but cannot call these functions, you should still
 * acquire the spinlock.
 * (Technically speaking, I should include spinlock.h and declare extern here
 * but that breaks the C++ compile of xtm.)
 */
//#include <linux/spinlock.h>
//extern spinlock_t bcm_gpio_spinlock;
extern void kerSysSetGpioState(unsigned short bpGpio, GPIO_STATE_t state);
extern void kerSysSetGpioStateLocked(unsigned short bpGpio, GPIO_STATE_t state);
extern void kerSysSetGpioDir(unsigned short bpGpio);
extern void kerSysSetGpioDirLocked(unsigned short bpGpio);
// for the voice code, which has too many kernSysSetGpio to change
#define kerSysSetGpio kerSysSetGpioState


extern unsigned long kerSysGetUbusFreq(unsigned long miscStrapBus);
extern int kerSysGetAfeId( unsigned long *afeId );
#define __kerSysGetAfeId  kerSysGetAfeId

#if defined(CONFIG_BCM_CPLD1)
int BcmCpld1Initialize(void);
void BcmCpld1CheckShutdownMode(void);
void BcmCpld1SetShutdownMode(void);
void BcmCpld1SetStandbyTimer(unsigned int duration);
#endif

#if defined (CONFIG_BCM_AVS_PWRSAVE)
extern void kerSysBcmEnableAvs(int enable);
extern int kerSysBcmAvsEnabled(void);
#endif

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
#define CONFIG_BCM_PWRMNGT_DDR_SR_API
// The structure below is to be declared in ADSL PHY MIPS LMEM, if ADSL is compiled in
typedef struct _PWRMNGT_DDR_SR_CTRL_ {
  union {
   struct {
      unsigned char   phyBusy;
      unsigned char   tp0Busy;
      unsigned char   tp1Busy;
      unsigned char   reserved;
    };
    unsigned int      word;
  };
} PWRMNGT_DDR_SR_CTRL;

void BcmPwrMngtRegisterLmemAddr(PWRMNGT_DDR_SR_CTRL *pDdrSr);
#endif

#endif

#ifdef __cplusplus
}
#endif


/* ACTIONTEC Specific defines. */
#if defined(AEI_VDSL_WAN_ETH)
#define IF_NAME_EWAN    "ewan"
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
int kerClearScratchPad(int blk_size);
extern int kerSysGetDslDatapump(unsigned char *dslDatapump);
extern void restoreDatapump(int value);
#endif
#ifdef AEI_TWO_IN_ONE_FIRMWARE
extern int kerSysGetBoardID(unsigned char *boardid);
#endif
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
extern int AEI_kerSysBcmImageSet( int partition, char *string, int img_size );
#endif

#endif /* _BOARD_H */

