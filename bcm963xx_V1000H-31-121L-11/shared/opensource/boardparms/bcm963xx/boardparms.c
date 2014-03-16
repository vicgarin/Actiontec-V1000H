/*
    Copyright 2000-2010 Broadcom Corporation

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

/**************************************************************************
* File Name  : boardparms.c
*
* Description: This file contains the implementation for the BCM63xx board
*              parameter access functions.
*
* Updates    : 07/14/2003  Created.
***************************************************************************/

/* Includes. */
#include "boardparms.h"
#include "bcmSpiRes.h"

/* Typedefs */

/*
You can add a new enum entry anywhere in the enum list below.
Then, you can use the enums in the board parm arrays, with the following restrictions:
-An API must be created for accessing any data from the array.
-bp_cpBoardId  must be the first element of the array.
-Most enums that are used only once in the array can be placed anywhere after bp_cpBoardId .
-These enums are read with the BpGetUc/BpGetUs/BpGetUl private functions.
-Certain enums can appear multiple times in the board parm arrays
-These enums have special APIs that understand how to loop through each occurence
 They are:
 *packet switch related parameters (from bp_ucPhyAddress to bp_ulPhyId7) which can
  appear once per packet switch (bp_ucPhyType0 and bp_ucPhyType1)
 *led info related parameters (bp_usDuplexLed, bp_usSpeedLed100 and bp_usSpeedLed1000) which can
  appear once per internal led (bp_ulPhyId0 and bp_ulPhyId1)
 *voip dsp related parameters (from bp_ucDspAddress to bp_usGpioDectLed) which can
  appear once per dsp (bp_ucDspType0 and bp_ucDspType1)
*/

enum bp_id {
  bp_cpBoardId,
  bp_cpComment,
  bp_usGpioOverlay,
  bp_usGpioLedAdsl,
  bp_usGpioLedAdslFail,
  bp_usGpioSecLedAdsl,
  bp_usGpioSecLedAdslFail,
  bp_usGpioLedSesWireless,
  bp_usGpioLedWanData,
  bp_usGpioLedWanError,
  bp_usGpioLedBlPowerOn,
  bp_usGpioLedBlStop,
  bp_usGpioFpgaReset,
  bp_usGpioLedGpon,
  bp_usGpioLedGponFail,
  bp_usGpioLedMoCA,
  bp_usGpioLedMoCAFail,
#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
  bp_usGpioLedUsb,            /* GPIO pin or not defined */
  bp_usGpioLedSesWirelessFail,
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
  bp_usGpioLedWirelessRed,
  bp_usGpioLedWirelessGreen,
  bp_usGpioLedWirelessAct,
#endif
#if defined(AEI_63168_CHIP)
  bp_usGpioLedEnetWan,
#endif
#endif
  bp_usExtIntrResetToDefault,
  bp_usExtIntrSesBtnWireless,
  bp_usAntInUseWireless,
  bp_usWirelessFlags,
  bp_usGpioWirelessPowerDown,
  bp_ucPhyType0,
  bp_ucPhyType1,
  bp_ucPhyAddress,
  bp_usConfigType,
  bp_ulPortMap,
  bp_ulPhyId0,
  bp_ulPhyId1,
  bp_ulPhyId2,
  bp_ulPhyId3,
  bp_ulPhyId4,
  bp_ulPhyId5,
  bp_ulPhyId6,
  bp_ulPhyId7,
  bp_usDuplexLed,
  bp_usSpeedLed100,
  bp_usSpeedLed1000,
  bp_ucDspType0,
  bp_ucDspType1,
  bp_ucDspAddress,
  bp_usGpioLedVoip,
  bp_usGpioVoip1Led,
  bp_usGpioVoip1LedFail,
  bp_usGpioVoip2Led,
  bp_usGpioVoip2LedFail,
  bp_usGpioPotsLed,
  bp_usGpioDectLed,
  bp_usGpioPassDyingGasp,
  bp_ulAfeId0,
  bp_ulAfeId1,
  bp_usGpioExtAFEReset,
  bp_usGpioExtAFELDPwr,
  bp_usGpioExtAFELDMode,
  bp_usGpioIntAFELDPwr,
  bp_usGpioIntAFELDMode,
  bp_usGpioAFELDRelay,
  bp_usGpioUart2Sdin,
  bp_usGpioUart2Sdout,
  bp_usGpioLaserDis,
  bp_usGpioLaserTxPwrEn,
  bp_usVregSel1P2,
  bp_usGponOpticsType,
  bp_usGpioFemtoReset,
  bp_cpDefaultOpticalParams,
  bp_usEphyBaseAddress,
  bp_usGpioSpiSlaveReset,  
  bp_usSpiSlaveBusNum,  
  bp_usSpiSlaveSelectNum,
  bp_usSpiSlaveMode,
  bp_ulSpiSlaveCtrlState,
  bp_ulSpiSlaveMaxFreq,
  bp_usSpiSlaveProtoRev,
  bp_usGpioExtAFELDData,
  bp_usGpioExtAFELDClk,
  bp_last
};

typedef struct bp_elem {
  enum bp_id id;
  union {
    char * cp;
    unsigned char * ucp;
    unsigned char uc;
    unsigned short us;
    unsigned long ul;
  } u;
} bp_elem_t;

/* Variables */
#if 0
/* Sample structure with all elements present in a valid order */
/* Indentation is used to illustrate the groupings where parameters can be repeated */
static bp_elem_t g_sample[] = {
  {bp_cpBoardId,               .u.cp = "SAMPLE"},
  {bp_usGpioOverlay,           .u.us = 0;
  {bp_usGpioLedAdsl,           .u.us = 0},
  {bp_usGpioLedAdslFail,       .u.us = 0},
  {bp_usGpioSecLedAdsl,        .u.us = 0},
  {bp_usGpioSecLedAdslFail,    .u.us = 0},
  {bp_usGpioLedSesWireless,    .u.us = 0},
  {bp_usGpioLedWanData,        .u.us = 0},
  {bp_usGpioLedWanError,       .u.us = 0},
  {bp_usGpioLedBlPowerOn,      .u.us = 0},
  {bp_usGpioLedBlStop,         .u.us = 0},
  {bp_usGpioFpgaReset,         .u.us = 0},
  {bp_usGpioLedGpon,           .u.us = 0},
  {bp_usGpioLedGponFail,       .u.us = 0},
  {bp_usGpioLedMoCA,           .u.us = 0},
  {bp_usGpioLedMoCAFail,       .u.us = 0},
  {bp_usExtIntrResetToDefault, .u.us = 0},
  {bp_usExtIntrSesBtnWireless, .u.us = 0},
  {bp_usAntInUseWireless,      .u.us = 0},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_usGpioWirelessPowerDown, .u.us = 0},
  {bp_ucPhyType0,              .u.uc = 0}, // First switch
    {bp_ucPhyAddress,          .u.uc = 0},
    {bp_usConfigType,          .u.us = 0},
    {bp_ulPortMap,             .u.ul = 0},
    {bp_ulPhyId0,              .u.ul = 0},
      {bp_usDuplexLed,         .u.us = 0},
      {bp_usSpeedLed100,       .u.us = 0},
      {bp_usSpeedLed1000,      .u.us = 0},
    {bp_ulPhyId1,              .u.ul = 0},
      {bp_usDuplexLed,         .u.us = 0},
      {bp_usSpeedLed100,       .u.us = 0},
      {bp_usSpeedLed1000,      .u.us = 0},
    {bp_ulPhyId2,              .u.ul = 0},
    {bp_ulPhyId3,              .u.ul = 0},
    {bp_ulPhyId4,              .u.ul = 0},
    {bp_ulPhyId5,              .u.ul = 0},
    {bp_ulPhyId6,              .u.ul = 0},
    {bp_ulPhyId7,              .u.ul = 0},
  {bp_ucPhyType1,              .u.uc = 0}, // Second switch
    {bp_ucPhyAddress,          .u.uc = 0},
    {bp_usConfigType,          .u.us = 0},
    {bp_ulPortMap,             .u.ul = 0},
    {bp_ulPhyId0,              .u.ul = 0},
      {bp_usDuplexLed,         .u.us = 0},
      {bp_usSpeedLed100,       .u.us = 0},
      {bp_usSpeedLed1000,      .u.us = 0},
    {bp_ulPhyId1,              .u.ul = 0},
      {bp_usDuplexLed,         .u.us = 0},
      {bp_usSpeedLed100,       .u.us = 0},
      {bp_usSpeedLed1000,      .u.us = 0},
    {bp_ulPhyId2,              .u.ul = 0},
    {bp_ulPhyId3,              .u.ul = 0},
    {bp_ulPhyId4,              .u.ul = 0},
    {bp_ulPhyId5,              .u.ul = 0},
    {bp_ulPhyId6,              .u.ul = 0},
    {bp_ulPhyId7,              .u.ul = 0},
  {bp_ucDspType0,              .u.uc = 0}, // First VOIP DSP
    {bp_ucDspAddress,          .u.uc = 0},
    {bp_usGpioLedVoip,         .u.us = 0},
    {bp_usGpioVoip1Led,        .u.us = 0},
    {bp_usGpioVoip1LedFail,    .u.us = 0},
    {bp_usGpioVoip2Led,        .u.us = 0},
    {bp_usGpioVoip2LedFail,    .u.us = 0},
    {bp_usGpioPotsLed,         .u.us = 0},
    {bp_usGpioDectLed,         .u.us = 0},
  {bp_ucDspType1,              .u.uc = 0}, // Second VOIP DSP
    {bp_ucDspAddress,          .u.uc = 0},
    {bp_usGpioLedVoip,         .u.us = 0},
    {bp_usGpioVoip1Led,        .u.us = 0},
    {bp_usGpioVoip1LedFail,    .u.us = 0},
    {bp_usGpioVoip2Led,        .u.us = 0},
    {bp_usGpioVoip2LedFail,    .u.us = 0},
    {bp_usGpioPotsLed,         .u.us = 0},
    {bp_usGpioDectLed,         .u.us = 0},
  {bp_ulAfeId0,                .u.ul = 0},
  {bp_ulAfeId1,                .u.ul = 0},
  {bp_usGpioExtAFEReset,       .u.us = 0},
  {bp_usGpioExtAFELDPwr,       .u.us = 0},
  {bp_usGpioExtAFELDMode,      .u.us = 0},
  {bp_usGpioLaserDis,          .u.us = 0},
  {bp_usGpioLaserTxPwrEn,      .u.us = 0},
  {bp_usVregSel1P2,            .u.us = 0},
  {bp_usGpioFemtoReset,        .u.us = 0},
  {bp_last}
};

#endif

#if defined(_BCM96362_) || defined(CONFIG_BCM96362)

static bp_elem_t g_bcm96362advnx[] = {
  {bp_cpBoardId,               .u.cp = "96362ADVNX"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm96362advngr[] = {
  {bp_cpBoardId,               .u.cp = "96362ADVNgr"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_SPI_SSB3_EXT_CS |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ulPhyId5,                .u.ul = 0x19},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm96362advngr2[] = {
  {bp_cpBoardId,               .u.cp = "96362ADVNgr2"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2},
  {bp_last}
};

static bp_elem_t g_bcm963629epon[] = {
  {bp_cpBoardId,               .u.cp = "963629epon"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | BCM_WAN_PORT | CONNECTED_TO_EPON_MAC},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_last}
};

static bp_elem_t g_bcm96362advn2xh[] = {
  {bp_cpBoardId,               .u.cp = "96362ADVN2xh"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_SPI_SSB2_EXT_CS |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x10},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_SPI_SSB_2},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0x02},
  {bp_ulPhyId3,                .u.ul = 0x03},
  {bp_ulPhyId4,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm96361XF[] = {
  {bp_cpBoardId,               .u.cp = "96361XF"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = MII_DIRECT},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2},
  {bp_usVregSel1P2,            .u.us = 0x13},
  {bp_usGpioFemtoReset,        .u.us = BP_GPIO_35_AH},
  {bp_last}
};

static bp_elem_t g_bcm96361I2[] = {
  {bp_cpBoardId ,              .u.cp = "96361I2"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_24 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_25 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm96362rpvt[] = {
  {bp_cpBoardId ,              .u.cp = "96362RPVT"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId5,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2}, 
  {bp_last}
};

static bp_elem_t g_bcm96362rpvt_2u[] = {
  {bp_cpBoardId ,              .u.cp = "96362RPVT_2U"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId5,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2}, 
  {bp_last}
};

static bp_elem_t g_bcm96362radvngr2[] = {
  {bp_cpBoardId,               .u.cp = "96362RADVNgr2"},
  {bp_usGpioOverlay,           .u.us =(BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usWirelessFlags,         .u.us = BP_WLAN_EXCLUDE_ONBOARD},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_9_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2},
  {bp_last}
};

static bp_elem_t * g_BoardParms[] = {g_bcm96362advnx, g_bcm96362advngr, g_bcm96362advngr2, g_bcm963629epon, g_bcm96362advn2xh, g_bcm96361XF, g_bcm96361I2, g_bcm96362rpvt, g_bcm96362rpvt_2u, g_bcm96362radvngr2, 0};
#endif

#if defined(_BCM96368_) || defined(CONFIG_BCM96368)
#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
static bp_elem_t g_Q1000[] = {
    {bp_cpBoardId ,              .u.cp = "Q1000"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedSesWirelessFail,    .u.us = BP_GPIO_30_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usGpioLedUsb,            .u.us = BP_GPIO_21_AL},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
	{bp_ulAfeId1,				 .u.ul = BP_AFE_DEFAULT},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};

static bp_elem_t g_Q2000[] = {
    {bp_cpBoardId ,              .u.cp = "Q2000"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
    {bp_usGpioLedAdslFail,       .u.us = BP_GPIO_16_AL},
    {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
    {bp_usGpioSecLedAdslFail,    .u.us = BP_GPIO_17_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedSesWirelessFail, .u.us = BP_GPIO_30_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usGpioLedUsb,            .u.us = BP_GPIO_21_AL},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
	{bp_ulAfeId1,				 .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};

//ken add for C1000 & C2000A debug  on 6368 board
static bp_elem_t g_C1000A[] = { 
    {bp_cpBoardId ,              .u.cp = "C1000A"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedSesWirelessFail,    .u.us = BP_GPIO_30_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usGpioLedUsb,            .u.us = BP_GPIO_21_AL},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x1f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
//    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
	{bp_ulAfeId1,				 .u.ul = BP_AFE_DEFAULT},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};

static bp_elem_t g_C2000A[] = {
    {bp_cpBoardId ,              .u.cp = "C2000A"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
    {bp_usGpioLedAdslFail,       .u.us = BP_GPIO_16_AL},
    {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
    {bp_usGpioSecLedAdslFail,    .u.us = BP_GPIO_17_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedSesWirelessFail, .u.us = BP_GPIO_30_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usGpioLedUsb,            .u.us = BP_GPIO_21_AL},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
	{bp_ulAfeId1,				 .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};


static bp_elem_t g_V2000H[] = {
    {bp_cpBoardId ,              .u.cp = "V2000H"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
    {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
#if defined(AEI_VDSL_CUSTOMER_NCS)
    {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_30_AL},
#endif
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
	{bp_ulAfeId1,				 .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};

static bp_elem_t g_R1000H[] = {
    {bp_cpBoardId ,              .u.cp = "R1000H"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
    {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
	{bp_usGpioLedUsb,            .u.us = BP_GPIO_21_AL},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
#if defined(AEI_VDSL_CUSTOMER_NCS)
    {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_30_AL},
#endif
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_last}
};


static bp_elem_t g_V1000H[] = {
    {bp_cpBoardId ,              .u.cp = "V1000H"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
//    {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
#if defined(AEI_VDSL_CUSTOMER_NCS)
    {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_30_AL},
#endif
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};

static bp_elem_t g_VB784WG[] = {
    {bp_cpBoardId ,              .u.cp = "VB784WG"},
    {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                         BP_OVERLAY_PHY |
                                         BP_OVERLAY_EPHY_LED_0 |
                                         BP_OVERLAY_EPHY_LED_1 |
                                         BP_OVERLAY_EPHY_LED_2 |
                                         BP_OVERLAY_EPHY_LED_3 |
                                         BP_OVERLAY_INET_LED |
                                         BP_OVERLAY_USB_DEVICE)},
    {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
//    {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
    {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
    {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
    {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
    {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
    {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
    {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
    {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
    {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
    {bp_usWirelessFlags,         .u.us = 0},
#if defined(AEI_VDSL_CUSTOMER_NCS)
    {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_30_AL},
#endif
    {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
    {bp_ulPortMap,               .u.ul = 0x20},
    {bp_ulPhyId5,                .u.ul = 0xff},
    {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
    {bp_ucPhyAddress,            .u.uc = 0x0},
    {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO_PSEUDO_PHY},
    {bp_ulPortMap,               .u.ul = 0x3f},
    {bp_ulPhyId0,                .u.ul = 0x00},
    {bp_ulPhyId1,                .u.ul = 0x01},
    {bp_ulPhyId2,                .u.ul = 0x02},
    {bp_ulPhyId3,                .u.ul = 0x03},
    {bp_ulPhyId4,                .u.ul = 0x04},
    {bp_ulPhyId5,                .u.ul = 0x11},
    {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
    {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_28_AL},
    {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
    {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
    {bp_last}
};



#endif

static bp_elem_t g_bcm96368vvw[] = {
  {bp_cpBoardId,               .u.cp = "96368VVW"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_33_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_0_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_1_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368vvwb[] = {
  {bp_cpBoardId,               .u.cp = "96368VVWB"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_24_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_33_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_0_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_1_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXB | BP_AFE_FE_REV_ISIL_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368ntr[] = {
  {bp_cpBoardId,               .u.cp = "96368NTR"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_33_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_0_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_1_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_25_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_26_AL},
  {bp_usGpioPotsLed,           .u.us = BP_GPIO_27_AL},
  {bp_last}
};

static bp_elem_t g_bcm96368sv2[] = {
  {bp_cpBoardId,               .u.cp = "96368SV2"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_33_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_30_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_31_AH},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x12},
  {bp_ulPhyId5,                .u.ul = 0x11},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_25_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_26_AL},
  {bp_usGpioPotsLed,           .u.us = BP_GPIO_27_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368mvwg[] = {
  {bp_cpBoardId,               .u.cp = "96368MVWG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x36},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId4,                .u.ul = 0x12},
  {bp_ulPhyId5,                .u.ul = 0x11},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368mvwgb[] = {
  {bp_cpBoardId,               .u.cp = "96368MVWGB"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x36},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId4,                .u.ul = 0x12},
  {bp_ulPhyId5,                .u.ul = 0x11},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXB | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368mvwgj[] = {
  {bp_cpBoardId,               .u.cp = "96368MVWGJ"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x36},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x00},
  {bp_ulPhyId4,                .u.ul = 0x12},
  {bp_ulPhyId5,                .u.ul = 0x11},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXJ | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368mb2g[] = {
  {bp_cpBoardId,               .u.cp = "96368MB2G"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_35_AL},
  {bp_last}
};

static bp_elem_t g_bcm96368mbg6b[] = {
  {bp_cpBoardId,               .u.cp = "96368MBG6b"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_35_AL},
  {bp_last}
};

static bp_elem_t g_bcm96368mbg6302[] = {
  {bp_cpBoardId,               .u.cp = "96368MBG6302"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_27_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_35_AL},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_37_AH},
  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_36_AH},
  {bp_last}
};

static bp_elem_t g_bcm96368mng[] = {
  {bp_cpBoardId,               .u.cp = "96368MNG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x20},
  {bp_ulPhyId5,                .u.ul = 0xff},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_SPI_SSB_1},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0x02},
  {bp_ulPhyId3,                .u.ul = 0x03},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_37_AL},
  {bp_last}
};

static bp_elem_t g_bcm96367avng[] = {
  {bp_cpBoardId,               .u.cp = "96367AVNG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_31_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x2f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId5,                .u.ul = 0x11},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368mvngr[] = {
  {bp_cpBoardId,               .u.cp = "96368MVNgr"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368rmvng_nand[] = {
  {bp_cpBoardId,               .u.cp = "96368RMVNg NAND"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x03},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368rmvng_nor[] = {
  {bp_cpBoardId,               .u.cp = "96368RMVNg NOR"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x13},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368mvngrP2[] = {
  {bp_cpBoardId,               .u.cp = "96368MVNgrP2"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_23_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_3_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t g_bcm96368Ext[] = {
  {bp_cpBoardId,               .u.cp = "96368EXT"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_2_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x03},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_last}
};

static bp_elem_t * g_BoardParms[] =
#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
{g_Q1000, g_Q2000, g_R1000H,g_V2000H,g_V1000H,g_VB784WG,g_C1000A,g_C2000A,
g_bcm96368vvw, g_bcm96368mvwg, g_bcm96368sv2, g_bcm96368mb2g,
#else
{g_bcm96368vvw, g_bcm96368mvwg, g_bcm96368sv2, g_bcm96368mb2g,
#endif
g_bcm96368ntr, g_bcm96368Ext, g_bcm96368vvwb, g_bcm96368mvwgb,
g_bcm96368mng, g_bcm96368mbg6302, g_bcm96368mvwgj, g_bcm96367avng,
g_bcm96368mvngr, g_bcm96368rmvng_nand, g_bcm96368rmvng_nor, g_bcm96368mvngrP2, g_bcm96368mbg6b, 0};
#endif

#if defined(_BCM96816_) || defined(CONFIG_BCM96816)

static bp_elem_t g_bcm96816sv[] = {
  {bp_cpBoardId,               .u.cp = "96816SV"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCI |
                                       BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_MOCA_LED)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_GPIO_16_AH},
  {bp_usGpioLedMoCA,           .u.us = BP_GPIO_5_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0x14},
  {bp_ulPhyId3,                .u.ul = 0x12},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ulPhyId5,                .u.ul = 0xff},
  {bp_ulPhyId7,                .u.ul = 0xff},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGponOpticsType,        .u.ul = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm96816pvwm[] = {
  {bp_cpBoardId,               .u.cp = "96816PVWM"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_MOCA_LED)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_GPIO_2_AH},
  {bp_usGpioLedMoCA,           .u.us = BP_GPIO_5_AH},
  {bp_usGpioLedMoCAFail,       .u.us = BP_GPIO_37_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0x11},
  {bp_ulPhyId3,                .u.ul = 0x12},
  {bp_ulPhyId4,                .u.ul = 0xff | PHYCFG_VALID}, /* WAN interface */
  {bp_ulPhyId5,                .u.ul = 0xff},
  {bp_ulPhyId7,                .u.ul = 0xff},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

unsigned char _6818RgParams[BP_OPTICAL_PARAMS_LEN] = {0x28,0x03,0x02,0x0B,0x00,0xD8,0x00,0x77,
                                                      0x01,0xB8,0x7F,0x02,0x02,0x32,0x00,0x00,
                                                      0x00,0x34,0xFA,0x00,0x00,0x09,0x00,0x00,
                                                      0x08,0x6F,0xFF,0xFF,0xEF,0xFF,0xFF,0xFF,
                                                      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
                                                      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
                                                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                      0x00,0x00,0x00,0x00,0x00,0x00,0xA5,0x5A};

static bp_elem_t g_bcm96818rg[] = {
  {bp_cpBoardId,               .u.cp = "96818RG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_SERIAL_LEDS)},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x8f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_ulPhyId2,                .u.ul  = PHY_INTEGRATED_VALID | PHY_EXTERNAL | MAC_IF_RGMII | BP_PHY_ID_24},
  {bp_ulPhyId3,                .u.ul  = PHY_INTEGRATED_VALID | PHY_EXTERNAL | MAC_IF_RGMII | BP_PHY_ID_25},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_8_AL},  
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpioLaserDis,          .u.us = BP_GPIO_0_AL},
  {bp_usGpioLaserTxPwrEn,      .u.us = BP_GPIO_5_AL},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},  
  {bp_last}
};

static bp_elem_t g_bcm96818rg_bosa[] = {
  {bp_cpBoardId,               .u.cp = "96818RG_BOSA"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_SERIAL_LEDS)},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x8f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_ulPhyId2,                .u.ul  = PHY_INTEGRATED_VALID | PHY_EXTERNAL | MAC_IF_RGMII | BP_PHY_ID_24},
  {bp_ulPhyId3,                .u.ul  = PHY_INTEGRATED_VALID | PHY_EXTERNAL | MAC_IF_RGMII | BP_PHY_ID_25},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_8_AL},  
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usGpioLaserDis,          .u.us = BP_GPIO_0_AL},
  {bp_usGpioLaserTxPwrEn,      .u.us = BP_GPIO_5_AL},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_BOSA},
  {bp_cpDefaultOpticalParams,  .u.ucp = _6818RgParams},    
  {bp_last}
};

static bp_elem_t g_bcm96818rg_fe[] = {
  {bp_cpBoardId,               .u.cp = "96818RG_FE"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_SERIAL_LEDS)},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_12_AH},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_0_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_1_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_SERIAL_GPIO_6_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x9f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_usSpeedLed100,           .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usSpeedLed1000,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_ulPhyId2,                .u.ul  = PHY_INTEGRATED_VALID | PHY_EXTERNAL | MAC_IF_RGMII| BP_PHY_ID_24},
  {bp_ulPhyId3,                .u.ul  = PHY_INTEGRATED_VALID | PHY_EXTERNAL | MAC_IF_RGMII| BP_PHY_ID_25},
  {bp_ulPhyId4,                .u.ul = PHYCFG_VALID | MII_OVER_GPIO_VALID | PHY_INTEGRATED_VALID | PHY_EXTERNAL | BP_PHY_ID_16},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},  
  {bp_usGpioLaserDis,          .u.us = BP_GPIO_0_AL},
  {bp_usGpioLaserTxPwrEn,      .u.us = BP_GPIO_5_AL},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_BOSA},
  {bp_cpDefaultOpticalParams,  .u.ucp = _6818RgParams},    
  {bp_last}
};

unsigned char _68128RgParams[BP_OPTICAL_PARAMS_LEN] = {0x28,0x03,0x02,0x0B,0x00,0xD8,0x00,0x77,
                                                       0x01,0xC5,0x26,0x02,0x02,0x32,0x00,0x00,
                                                       0x00,0x34,0xFA,0x00,0x00,0x09,0x00,0x00,
                                                       0x08,0x6F,0xFF,0xFF,0xEF,0xFF,0xFF,0xFF,
                                                       0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
                                                       0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,
                                                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                       0x00,0x00,0x00,0x00,0x00,0x00,0xA5,0x5A};

static bp_elem_t g_bcm968128rg[] = {
  {bp_cpBoardId,               .u.cp = "968128RG"},
  {bp_usGpioOverlay,           .u.ul = BP_OVERLAY_GPHY_LED_0},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_18_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_19_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AL},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x81},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_29_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_26_AL},
  {bp_ulPhyId7,                .u.ul = BP_PHY_ID_NOT_SPECIFIED},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioLaserDis,          .u.us = BP_GPIO_11_AL},
  {bp_usGpioLaserTxPwrEn,      .u.us = BP_GPIO_39_AH},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_BOSA},
  {bp_cpDefaultOpticalParams,  .u.ucp = _68128RgParams},  
  {bp_last}
};

static bp_elem_t g_bcm96829rg[] = {
  {bp_cpBoardId,               .u.cp = "96829RG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_MOCA_LED)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioLedMoCA,           .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedMoCAFail,       .u.us = BP_GPIO_37_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xa3},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0xff},
  {bp_ulPhyId3,                .u.ul = 0xff},
  {bp_ulPhyId4,                .u.ul = 0x00},
  {bp_ulPhyId5,                .u.ul = 0xff},
  {bp_ulPhyId6,                .u.ul = 0x00},
  {bp_ulPhyId7,                .u.ul = 0xff},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm96816p2og[] = {
  {bp_cpBoardId,               .u.cp = "96816P2OG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_PCI |
                                       BP_OVERLAY_MOCA_LED)},
  {bp_usGpioFpgaReset,         .u.us = BP_GPIO_2_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_GPIO_16_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xaf},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0x11},
  {bp_ulPhyId3,                .u.ul = 0xff},
  {bp_ulPhyId5,                .u.ul = 0xff},
  {bp_ulPhyId7,                .u.ul = 0xff},    
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm96812pg[] = {
  {bp_cpBoardId,               .u.cp = "96812PG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AH},
  {bp_usGpioLedGponFail,       .u.us = BP_GPIO_2_AH},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xaf},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_ulPhyId2,                .u.ul = 0x11},
  {bp_ulPhyId3,                .u.ul = 0x12},
  {bp_ulPhyId5,                .u.ul = 0xff},
  {bp_ulPhyId7,                .u.ul = 0xff},
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm96819bhr[] = {
  {bp_cpBoardId,               .u.cp = "96819BHR"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_MOCA_LED)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioLedMoCA,           .u.us = BP_GPIO_5_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_3},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_13_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_38_AL},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_30_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_31_AL},
  {bp_ulPhyId2,                .u.ul = 0x14},
  {bp_ulPhyId3,                .u.ul = 0x12},
  {bp_ulPhyId4,                .u.ul = 0xa1},
  {bp_ulPhyId5,                .u.ul = 0xff},    
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm968121pg[] = {
  {bp_cpBoardId,               .u.cp = "968121PG"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AL},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x95},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId1,                .u.ul = 0x00},
  {bp_ulPhyId2,                .u.ul = 0x18},
  {bp_ulPhyId3,                .u.ul = 0x00},
  {bp_ulPhyId4,                .u.ul = 0xff | PHYCFG_VALID}, /* WAN Interface */
  {bp_ulPhyId5,                .u.ul = 0x00},
  {bp_ulPhyId6,                .u.ul = 0x00},
  {bp_ulPhyId7,                .u.ul = 0xff},    
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm968121p[] = {
  {bp_cpBoardId,               .u.cp = "968121P"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_24_AH},
  {bp_usGpioLedGpon,           .u.us = BP_GPIO_8_AL},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x91},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_ulPhyId4,                .u.ul = 0xff | PHYCFG_VALID}, /* WAN Interface */
  {bp_ulPhyId7,                .u.ul = 0xff},    
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t g_bcm96819bhrgr[] = {
  {bp_cpBoardId,               .u.cp = "96819BHRGR"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_1 |
                                       BP_OVERLAY_MOCA_LED)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_3_AH},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioLedMoCA,           .u.us = BP_GPIO_5_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_3},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x3f},
  {bp_ulPhyId0,                .u.ul = 0x00},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_13_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_38_AL},
  {bp_ulPhyId1,                .u.ul = 0x01},
  {bp_usSpeedLed100,           .u.us = BP_GPIO_2_AL},
  {bp_usSpeedLed1000,          .u.us = BP_GPIO_31_AL},
  {bp_ulPhyId2,                .u.ul = 0x18},
  {bp_ulPhyId3,                .u.ul = 0x19},
  {bp_ulPhyId4,                .u.ul = 0xa1},
  {bp_ulPhyId5,                .u.ul = 0xff},    
  {bp_usGponOpticsType,        .u.us = BP_GPON_OPTICS_TYPE_LEGACY},
  {bp_last}
};

static bp_elem_t * g_BoardParms[] = {g_bcm96816sv, g_bcm96816pvwm,
  g_bcm96829rg, g_bcm96816p2og, g_bcm96812pg, g_bcm96819bhr,
  g_bcm968121pg, g_bcm968121p, g_bcm96819bhrgr, g_bcm96818rg, g_bcm96818rg_bosa, g_bcm96818rg_fe, g_bcm968128rg, 0};
#endif

#if defined(_BCM96328_) || defined(CONFIG_BCM96328)

static bp_elem_t g_bcm96328avng[] = {
  {bp_cpBoardId,               .u.cp = "96328avng"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_0_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_4_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_8_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_6_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_7_AL},
  {bp_usGpioPotsLed,           .u.us = BP_GPIO_5_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm96328avngrP1[] = {
  {bp_cpBoardId,               .u.cp = "96328avngrP1"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm96328avngr[] = {
  {bp_cpBoardId,               .u.cp = "96328avngr"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_10_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_13_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm963281TAN[] = {
  {bp_cpBoardId,               .u.cp = "963281TAN"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_11_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_7_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_4_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_8_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm963281TAN2[] = {
  {bp_cpBoardId,               .u.cp = "963281TAN2"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_11_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_7_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_4_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_8_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_2},
  {bp_last}
};

static bp_elem_t g_bcm963281TAN3[] = {
  {bp_cpBoardId,               .u.cp = "963281TAN3"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_11_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_7_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_4_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_8_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302| BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2},
  {bp_last}
};

static bp_elem_t g_bcm963281TAN4[] = {
  {bp_cpBoardId,               .u.cp = "963281TAN4"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_11_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_7_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_4_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_8_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_2},
  {bp_last}
};

static bp_elem_t g_bcm963293epon[] = {
  {bp_cpBoardId,               .u.cp = "963293epon"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = MII_DIRECT  | CONNECTED_TO_EPON_MAC},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm963293epon_gphy[] = {
  {bp_cpBoardId,               .u.cp = "963293epon_gphy"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SPI_EXT_CS |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_EPHY_LED_1 |
                                       BP_OVERLAY_EPHY_LED_2 |
                                       BP_OVERLAY_EPHY_LED_3 |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_11_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_15_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | CONNECTED_TO_EPON_MAC},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_12_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},
  {bp_last}
};

static bp_elem_t g_bcm963283_24tstbrd[] = {
  {bp_cpBoardId,               .u.cp = "963283_24tstbrd"},
  {bp_usGpioOverlay,           .u.ul =( BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_29_AL},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_last}
};

static bp_elem_t g_bcm93715rv[] = {
  {bp_cpBoardId,                 .u.cp = "93715rv"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SPI_EXT_CS)},

  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x01},
  {bp_ulPhyId0,                .u.ul = 0x01 | CONNECTED_TO_EPON_MAC},

  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_14_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_12_AL},
  
  {bp_usGpioPassDyingGasp,     .u.us = BP_SERIAL_GPIO_1_AL},
  
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1},

  {bp_last}
 };


static bp_elem_t * g_BoardParms[] = {g_bcm96328avng, g_bcm96328avngrP1, g_bcm96328avngr, g_bcm963281TAN, g_bcm963281TAN2, g_bcm963281TAN3, g_bcm963281TAN4, g_bcm963293epon, g_bcm963293epon_gphy, g_bcm963283_24tstbrd, g_bcm93715rv, 0};
#endif

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)

static char g_obsoleteStr[] = "(obsolete)";

static bp_elem_t g_bcm963268sv1[] = {
  {bp_cpBoardId,               .u.cp = "963268SV1"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_last}
};

static bp_elem_t g_bcm963168mbv_17a[] = {
  {bp_cpBoardId,               .u.cp = "963168MBV_17A"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0}, 
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId6,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306| BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_21},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_usGpioAFELDRelay,      .u.us = BP_GPIO_39_AH},
  {bp_last}
};

static bp_elem_t g_bcm963168mbv_30a[] = {
  {bp_cpBoardId ,              .u.cp = "963168MBV_30A"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0}, 
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId6,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_AVMODE_VDSL | BP_AFE_FE_REV_12_21 | BP_AFE_FE_ANNEXA },
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_usGpioAFELDRelay,      .u.us = BP_GPIO_39_AH},
  {bp_last}
};

static bp_elem_t g_bcm963268mbv[] = {
  {bp_cpBoardId,                 .u.cp = "963268MBV"},
  {bp_cpComment,               .u.cp = g_obsoleteStr},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId6,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_usGpioAFELDRelay,      .u.us = BP_GPIO_39_AH},
  {bp_last}
};

#if 0
/* Uncomment "#define BP_GET_INT_AFE_DEFINED" in Boardparams.h when these bp_ids are in used:
* bp_usGpioIntAFELDPwr
* bp_usGpioIntAFELDMode
* bp_usGpioAFELDRelay
*/

static bp_elem_t g_bcm963268mbv6b[] = {
  {bp_cpBoardId,               .u.cp = "963168MBV6b"},
  {bp_usGpioOverlay,           .u.us =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ulPhyId6,                .u.ul = 0x19},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_12_21},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_13_AH},
  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_12_AH},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioAFELDRelay,      .u.us = BP_GPIO_39_AH},
  {bp_last}
};
#endif

static bp_elem_t g_bcm963168vx[] = {
  {bp_cpBoardId,               .u.cp = "963168VX"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_last}
};

static bp_elem_t g_bcm963168xf[] = {
  {bp_cpBoardId,               .u.cp = "963168XF"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = MII_DIRECT},  
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_14_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_15_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioFemtoReset,        .u.us = BP_GPIO_8_AH},  
  {bp_last}
};

static bp_elem_t g_bcm963268sv2_extswitch[] = {
  {bp_cpBoardId,               .u.cp = "963268SV2_EXTSW"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE |
                                       BP_OVERLAY_HS_SPI_SSB7_EXT_CS)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xbf},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_3},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPhyId5,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId7,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},  
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO},
  {bp_ulPortMap,               .u.ul = 0x03},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioSpiSlaveReset,     .u.us = BP_GPIO_22_AH},
  {bp_usSpiSlaveBusNum,        .u.us = HS_SPI_BUS_NUM},
  {bp_usSpiSlaveSelectNum,     .u.us = 7},
  {bp_usSpiSlaveMode,          .u.us = SPI_MODE_3},
  {bp_ulSpiSlaveCtrlState,     .u.ul = SPI_CONTROLLER_STATE_GATE_CLK_SSOFF},
  {bp_ulSpiSlaveMaxFreq,       .u.ul = 781000},
  {bp_usSpiSlaveProtoRev,      .u.us = 0},   
  {bp_last}
};

static bp_elem_t g_bcm963268bu[] = {
  {bp_cpBoardId,               .u.cp = "963268BU"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  // {bp_usEphyBaseAddress,       .u.us = 0x10},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xFC},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_3 | PHY_INTERNAL | PHY_INTEGRATED_VALID },
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4 },
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_0 | MAC_IFACE_VALID | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL},
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_1 | MAC_IFACE_VALID | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL },
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_24 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId7,                .u.ul = BP_PHY_ID_25 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_last}
};

#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
static bp_elem_t g_C1000A[] = {
  {bp_cpBoardId,               .u.cp = "C1000A"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
//  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_40_AL},
  {bp_usGpioLedAdsl,        .u.us = BP_GPIO_17_AL},
  {bp_usGpioLedAdslFail,    .u.us = BP_GPIO_8_AL},

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_18_AL},
#if defined(AEI_VDSL_CUSTOMER_NCS)
  {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_19_AL},
#endif
  {bp_usGpioLedUsb,            .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_13_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_12_AL},

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_35_AL},

  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
//  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4}, //ewan0
//  {bp_ulPhyId4,                .u.ul = 0x11}, // HPNA 
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO}, // Remember to make MDIO HW changes(install resistors R540, R541 and R553)
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},

//not using serial GPIO
//  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
//  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
//  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},

  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
//  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
{bp_last}

};

static bp_elem_t g_C1000B[] = {
  {bp_cpBoardId,               .u.cp = "C1000B"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
//  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_40_AL},
  {bp_usGpioLedAdsl,        .u.us = BP_GPIO_17_AL},
  {bp_usGpioLedAdslFail,    .u.us = BP_GPIO_8_AL},

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_18_AL},
#if defined(AEI_VDSL_CUSTOMER_NCS)
  {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_19_AL},
#endif
  {bp_usGpioLedUsb,            .u.us = BP_GPIO_22_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_13_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_12_AL},

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_35_AL},

  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
//  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4}, //ewan0
//  {bp_ulPhyId4,                .u.ul = 0x11}, // HPNA 
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO}, // Remember to make MDIO HW changes(install resistors R540, R541 and R553)
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},

//not using serial GPIO
//  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
//  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
//  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},

  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
//  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
{bp_last}

};

static bp_elem_t g_C2000A[] = {
  {bp_cpBoardId,               .u.cp = "C2000A"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},

  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_40_AL},
  {bp_usGpioLedAdslFail,       .u.us = BP_GPIO_8_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_46_AL},
  {bp_usGpioSecLedAdslFail,    .u.us = BP_GPIO_49_AL},

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_43_AL},
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
  {bp_usGpioLedWirelessAct,    .u.us = BP_GPIO_36_AL},
  {bp_usGpioLedWirelessGreen,  .u.us = BP_GPIO_37_AH},
  {bp_usGpioLedWirelessRed,    .u.us = BP_GPIO_39_AL},
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
  {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_44_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_42_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_41_AL},
#if defined(AEI_VOIP_LED)
  {bp_usGpioVoip1LedFail,      .u.us = BP_GPIO_51_AL},
  {bp_usGpioVoip2LedFail,      .u.us = BP_GPIO_48_AL},
#endif
#endif
  {bp_usGpioLedUsb,            .u.us = BP_GPIO_9_AL},

  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_0_AL},

#if defined(AEI_63168_CHIP)
  {bp_usGpioLedEnetWan,        .u.us = BP_GPIO_22_AL},
#endif

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_35_AL},

  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = 0x11},
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO}, // Remember to make MDIO HW changes(install resistors R540, R541 and R553)
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},

//not using serial GPIO
//  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
//  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
//  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},

/* use external AFE setting recommended by Broadcom */
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_21},

/* per Broadcom, comment out AFELDPWr and AFELDMode setting so VDSL_PHY_OVERRIDEs will be set to 1 to help ext AFE line */
//  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
//  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
//  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_13_AH},
//  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_12_AH},

{bp_last}
};

static bp_elem_t g_R2200H[] = {
  {bp_cpBoardId,               .u.cp = "R2200H"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
// change line order
//  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_46_AL},
//  {bp_usGpioLedAdslFail,       .u.us = BP_GPIO_49_AL},
//  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_40_AL},
//  {bp_usGpioSecLedAdslFail,    .u.us = BP_GPIO_8_AL},

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_43_AL},
#if defined(AEI_VDSL_CUSTOMER_NCS)
  {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_44_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
#endif
  {bp_usGpioLedUsb,            .u.us = BP_GPIO_9_AL},

  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_0_AL},

#if defined(AEI_63168_CHIP)
  {bp_usGpioLedEnetWan,        .u.us = BP_GPIO_22_AL},
#endif

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_35_AL},

  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = 0x11},
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO}, // Remember to make MDIO HW changes(install resistors R540, R541 and R553)
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},

//not using serial GPIO
//  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
//  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
//  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},

/* use external AFE setting recommended by Broadcom */
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_21},

/* per Broadcom, comment out AFELDPWr and AFELDMode setting so VDSL_PHY_OVERRIDEs will be set to 1 to help ext AFE line */
//  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
//  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
//  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_13_AH},
//  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_12_AH},

{bp_last}
};


static bp_elem_t g_V2200H[] = {
  {bp_cpBoardId,               .u.cp = "V2200H"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_40_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_46_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_43_AL},
#if defined(AEI_VDSL_CUSTOMER_NCS)
  {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_44_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
#endif
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_0_AL},

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},

  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = 0x11},
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO}, // Remember to make MDIO HW changes(install resistors R540, R541 and R553)
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},

//not using serial GPIO
//  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
//  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
//  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},

/* use external AFE setting recommended by Broadcom */
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_21},

/* per Broadcom, comment out AFELDPWr and AFELDMode setting so VDSL_PHY_OVERRIDEs will be set to 1 to help ext AFE line */
//  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
//  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
//  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_13_AH},
//  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_12_AH},

{bp_last}
};

static bp_elem_t g_FV2200[] = {
  {bp_cpBoardId,               .u.cp = "FV2200"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
// change line order
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_46_AL},
  {bp_usGpioLedAdslFail,       .u.us = BP_GPIO_49_AL},
  {bp_usGpioSecLedAdsl,        .u.us = BP_GPIO_40_AL},
  {bp_usGpioSecLedAdslFail,    .u.us = BP_GPIO_8_AL},

  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_43_AL},
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
  {bp_usGpioLedWirelessAct,    .u.us = BP_GPIO_36_AL},
  {bp_usGpioLedWirelessGreen,  .u.us = BP_GPIO_37_AH},
  {bp_usGpioLedWirelessRed,    .u.us = BP_GPIO_39_AL},
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
  {bp_usGpioLedSesWirelessFail,.u.us = BP_GPIO_44_AL},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_GPIO_41_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_GPIO_42_AL},
#endif
  {bp_usGpioLedUsb,            .u.us = BP_GPIO_9_AL},

  {bp_usGpioLedWanData,        .u.us = BP_GPIO_1_AL},
  {bp_usGpioLedWanError,       .u.us = BP_GPIO_0_AL},

#if defined(AEI_63168_CHIP)
  {bp_usGpioLedEnetWan,        .u.us = BP_GPIO_22_AL},
#endif

  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_35_AL},

  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = 0x11},
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO}, // Remember to make MDIO HW changes(install resistors R540, R541 and R553)
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},

//not using serial GPIO
//  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
//  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
//  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},

/* use external AFE setting recommended by Broadcom */
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_ulAfeId1,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_21},

/* per Broadcom, comment out AFELDPWr and AFELDMode setting so VDSL_PHY_OVERRIDEs will be set to 1 to help ext AFE line */
//  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
//  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
//  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_13_AH},
//  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_12_AH},

{bp_last}
};
#endif

static bp_elem_t g_bcm963268bu_p300[] = {
  {bp_cpBoardId,               .u.cp = "963268BU_P300"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_INET_LED |
                                       BP_OVERLAY_EPHY_LED_0 |
                                       BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_usEphyBaseAddress,       .u.us = 0x10},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xF9},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_17 | PHY_INTERNAL | PHY_INTEGRATED_VALID },
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4 | PHY_INTERNAL | PHY_INTEGRATED_VALID },
  {bp_ulPhyId4,                .u.ul = BP_PHY_ID_0 | MAC_IFACE_VALID | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL},
  {bp_ulPhyId5,                .u.ul = BP_PHY_ID_1 | MAC_IFACE_VALID | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL },
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_24 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId7,                .u.ul = BP_PHY_ID_25 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioIntAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioIntAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_last}
};

static bp_elem_t g_bcm963168xh[] = {
  {bp_cpBoardId,               .u.cp = "963168XH"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE|
                                       BP_OVERLAY_PCIE_CLKREQ |
                                       BP_OVERLAY_HS_SPI_SSB5_EXT_CS)},
  {bp_usGpioLedAdsl,           .u.us = BP_GPIO_13_AH},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_10_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x58},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
  {bp_ulPhyId6,                .u.ul = BP_PHY_ID_25},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_HS_SPI_SSB_5},// Remember to make MDIO HW changes(install resistors R540, R541 and R553) BP_ENET_CONFIG_HS_SPI_SSB_5},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_0},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_3},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_AVMODE_VDSL | BP_AFE_FE_REV_12_21 | BP_AFE_FE_ANNEXA },
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_11_AL},  
  {bp_last}
};

  static bp_elem_t g_bcm963168mp[] = {
  {bp_cpBoardId,               .u.cp = "963168MP"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1F},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = MII_DIRECT},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_last}
};

  static bp_elem_t g_bcm963268v30a[] = {
  {bp_cpBoardId,               .u.cp = "963268V30A"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCIE_CLKREQ |
                                       BP_OVERLAY_PHY |
                                       BP_OVERLAY_GPHY_LED_0 |
                                       BP_OVERLAY_SERIAL_LEDS )},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN}, // FIXME
  {bp_usWirelessFlags,         .u.us = 0}, // FIXME
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0xD8},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x00 | MAC_IFACE_VALID | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL},
  {bp_ulPhyId6,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ulPhyId7,                .u.ul = 0x19 | MAC_IFACE_VALID | MAC_IF_RGMII},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_6302_6306_REV_A_12_40},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_9_AL},  
  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_last}
};

  static bp_elem_t g_bcm963168media[] = {
  {bp_cpBoardId,               .u.cp = "963168MEDIA"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PCIE_CLKREQ |
                                       BP_OVERLAY_PHY |                                       
                                       BP_OVERLAY_SERIAL_LEDS )},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_SERIAL_GPIO_1_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN}, // FIXME
  {bp_usWirelessFlags,         .u.us = 0}, // FIXME
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x5F},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18 | MAC_IFACE_VALID | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL},
  {bp_ulPhyId6,                .u.ul = RGMII_DIRECT},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL}, 
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_11_AH},
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_12_AH},
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_13_AH},        
  {bp_last}
};


static bp_elem_t g_bcm963268sv2[] = {
  {bp_cpBoardId,               .u.cp = "963268SV2"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_LED |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x0f},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_1},
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_2},
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_3},
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_last}
};

static bp_elem_t g_bcm963168xfg3[] = {
  {bp_cpBoardId,               .u.cp = "963168XFG3"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE  |
                                       BP_OVERLAY_PHY         |
                                       BP_OVERLAY_PCIE_CLKREQ)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_0},
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_1},  
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x1F},
  {bp_ulPhyId0,                .u.ul = BP_PHY_ID_1}, 
  {bp_ulPhyId1,                .u.ul = BP_PHY_ID_2},  
  {bp_ulPhyId2,                .u.ul = BP_PHY_ID_3},         
  {bp_ulPhyId3,                .u.ul = BP_PHY_ID_4},
  {bp_ulPhyId4,                .u.ul = MII_DIRECT},  
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioExtAFELDMode,      .u.us = BP_GPIO_10_AH},
  {bp_usGpioExtAFELDPwr,       .u.us = BP_GPIO_11_AH},  
  {bp_usGpioFemtoReset,        .u.us = BP_GPIO_8_AH},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_7_2_30}, 
  {bp_last}
};

#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
static bp_elem_t * g_BoardParms[] = {g_C1000A,g_C2000A,g_R2200H,g_V2200H,g_FV2200,g_C1000B, g_bcm963268sv1, g_bcm963268mbv, g_bcm963168vx, g_bcm963268bu, g_bcm963268bu_p300, g_bcm963268sv2_extswitch, g_bcm963168mbv_17a, g_bcm963168mbv_30a, g_bcm963168xh, g_bcm963168mp, g_bcm963268v30a,   g_bcm963168media, g_bcm963268sv2, g_bcm963168xfg3, g_bcm963168xf, 0};
#else
static bp_elem_t * g_BoardParms[] = {g_bcm963268sv1, g_bcm963268mbv, g_bcm963168vx, g_bcm963268bu, g_bcm963268bu_p300, g_bcm963268sv2_extswitch, g_bcm963168mbv_17a, g_bcm963168mbv_30a, g_bcm963168xh, g_bcm963168mp, g_bcm963268v30a,   g_bcm963168media, g_bcm963268sv2, g_bcm963168xfg3, g_bcm963168xf, 0};
#endif

#endif

#if defined(_BCM96828_) || defined(CONFIG_BCM96828)

static bp_elem_t g_bcm96828epon[] = {
  {bp_cpBoardId,               .u.cp = "96828MBV"},
  {bp_usGpioOverlay,           .u.ul =(BP_OVERLAY_PHY |
                                       BP_OVERLAY_SERIAL_LEDS |
                                       BP_OVERLAY_USB_DEVICE)},
  {bp_usGpioLedAdsl,           .u.us = BP_SERIAL_GPIO_3_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_SERIAL_GPIO_7_AL},
  {bp_usGpioLedWanData,        .u.us = BP_SERIAL_GPIO_8_AL},
  {bp_usGpioLedWanError,       .u.us = BP_SERIAL_GPIO_2_AL},
  {bp_usGpioLedBlPowerOn,      .u.us = BP_GPIO_20_AL},
  {bp_usGpioLedBlStop,         .u.us = BP_GPIO_21_AL},
  {bp_usAntInUseWireless,      .u.us = BP_WLAN_ANT_MAIN},
  {bp_usWirelessFlags,         .u.us = 0},
  {bp_ucPhyType0,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MMAP},
  {bp_ulPortMap,               .u.ul = 0x5f},
  {bp_ulPhyId0,                .u.ul = 0x01},
  {bp_ulPhyId1,                .u.ul = 0x02},
  {bp_ulPhyId2,                .u.ul = 0x03},
  {bp_ulPhyId3,                .u.ul = 0x04},
  {bp_ulPhyId4,                .u.ul = 0x18},
  {bp_ulPhyId6,                .u.ul = 0x19},
  {bp_ucDspType0,              .u.uc = BP_VOIP_MIPS},
  {bp_ucDspAddress,            .u.uc = 0},
  {bp_usGpioVoip1Led,          .u.us = BP_SERIAL_GPIO_4_AL},
  {bp_usGpioVoip2Led,          .u.us = BP_SERIAL_GPIO_5_AL},
  {bp_usGpioPotsLed,           .u.us = BP_SERIAL_GPIO_6_AL},
  {bp_ulAfeId0,                .u.ul = BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV_7_2_30},
  {bp_usGpioExtAFEReset,       .u.us = BP_GPIO_17_AL},
  {bp_last}
};

static bp_elem_t * g_BoardParms[] = {g_bcm96828epon, 0};

#endif


static bp_elem_t * g_pCurrentBp = 0;

/* Private function prototypes */
bp_elem_t * BpGetElem(enum bp_id id, int *pindex, enum bp_id stopAtId);
char *BpGetSubCp(enum bp_id id, int index, enum bp_id stopAtId );
unsigned char BpGetSubUc(enum bp_id id, int index, enum bp_id stopAtId );
unsigned short BpGetSubUs(enum bp_id id, int index, enum bp_id stopAtId );
unsigned long BpGetSubUl(enum bp_id id, int index, enum bp_id stopAtId );
int BpGetCp(enum bp_id id, char **pcpValue );
int BpGetUc(enum bp_id id, unsigned char *pucValue );
int BpGetUs(enum bp_id id, unsigned short *pusValue );
int BpGetUl(enum bp_id id, unsigned long *pulValue );

/**************************************************************************
* Name       : bpstrcmp
*
* Description: String compare for this file so it does not depend on an OS.
*              (Linux kernel and CFE share this source file.)
*
* Parameters : [IN] dest - destination string
*              [IN] src - source string
*
* Returns    : -1 - dest < src, 1 - dest > src, 0 dest == src
***************************************************************************/
int bpstrcmp(const char *dest,const char *src)
{
    while (*src && *dest) {
        if (*dest < *src) return -1;
        if (*dest > *src) return 1;
        dest++;
        src++;
    }

    if (*dest && !*src) return 1;
    if (!*dest && *src) return -1;
    return 0;
} /* bpstrcmp */


/**************************************************************************
* Name       : BpGetElem
*
* Description: Private function to walk through the profile
*              and find the desired entry
*
* Parameters : [IN] id         - id to search for
*              [IN/OUT] pindex - where to start and where it was found
*              [IN] stopAtId   - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : ptr to entry found or to last entry otherwise
***************************************************************************/
bp_elem_t * BpGetElem(enum bp_id id, int *pindex, enum bp_id stopAtId)
{
    bp_elem_t * pelem;

    for (pelem = &g_pCurrentBp[*pindex]; pelem->id != bp_last && pelem->id != id && pelem->id != stopAtId; pelem++, (*pindex)++);

    return pelem;    
}

/**************************************************************************
* Name       : BpGetSubCp
*
* Description: Private function to get an char * entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id       - id to search for
*              [IN] index    - where to start from
*              [IN] stopAtId - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char * from the entry
***************************************************************************/
char *BpGetSubCp(enum bp_id id, int index, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &index, stopAtId);
    if (id == pelem->id) { 
        return pelem->u.cp;
    } else { 
        return (char *)BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubUc
*
* Description: Private function to get an unsigned char entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id       - id to search for
*              [IN] index    - where to start from
*              [IN] stopAtId - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned char from the entry
***************************************************************************/
unsigned char BpGetSubUc(enum bp_id id, int index, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &index, stopAtId);
    if (id == pelem->id) {
        return pelem->u.uc;
    } else {
        return (unsigned char)BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubUs
*
* Description: Private function to get an unsigned short entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id       - id to search for
*              [IN] index    - where to start from
*              [IN] stopAtId - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned short from the entry
***************************************************************************/
unsigned short BpGetSubUs(enum bp_id id, int index, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &index, stopAtId);
    if (id == pelem->id) {
        return pelem->u.us;
    } else {
        return BP_NOT_DEFINED;
    }
}

/**************************************************************************
* Name       : BpGetSubUl
*
* Description: Private function to get an unsigned long entry from the profile
*              can be used to search an id within a group by specifying stop id
*
* Parameters : [IN] id       - id to search for
*              [IN] index    - where to start from
*              [IN] stopAtId - id to stop at if the searched id is not found
*                                (allows grouping and repeated ids)
*
* Returns    : the unsigned long from the entry
***************************************************************************/
unsigned long BpGetSubUl(enum bp_id id, int index, enum bp_id stopAtId )
{
    bp_elem_t *pelem;

    pelem = BpGetElem(id, &index, stopAtId);
    if (id == pelem->id)
        return pelem->u.ul;
    else
        return BP_NOT_DEFINED;
}

/**************************************************************************
* Name       : BpGetCp
*
* Description: Private function to get an char * entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pulValue - char ** found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetCp(enum bp_id id, char **pcpValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pcpValue = BpGetSubCp(id, 0, bp_last);
        nRet = ((char *)BP_NOT_DEFINED != *pcpValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {     
        *pcpValue = (char *)BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUc
*
* Description: Private function to get an unsigned char entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pucValue - unsigned char found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUc(enum bp_id id, unsigned char *pucValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pucValue = BpGetSubUc(id, 0, bp_last);
        nRet = ((unsigned char)BP_NOT_DEFINED != *pucValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pucValue = (unsigned char)BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUs
*
* Description: Private function to get an unsigned short entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pusValue - unsigned short found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUs(enum bp_id id, unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pusValue = BpGetSubUs(id, 0, bp_last);
        nRet = (BP_NOT_DEFINED != *pusValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetUl
*
* Description: Private function to get an unsigned long entry from the profile
*              can only be used to search an id which appears once in the profile
*
* Parameters : [IN] id       - id to search for
*              [IN] pulValue - unsigned long found
*
* Returns    : BP_SUCCESS or appropriate error
***************************************************************************/
int BpGetUl(enum bp_id id, unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp ) {
        *pulValue = BpGetSubUl(id, 0, bp_last);
        nRet = (BP_NOT_DEFINED != *pulValue ? BP_SUCCESS : BP_VALUE_NOT_DEFINED);
    } else {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
}

/**************************************************************************
* Name       : BpGetVoipDspConfig
*
* Description: Gets the DSP configuration from the board parameter
*              structure for a given DSP index.
*
* Parameters : [IN] dspNum - DSP index (number)
*
***************************************************************************/
VOIP_DSP_INFO g_VoIPDspInfo[BP_MAX_VOIP_DSP] = {{0}};
VOIP_DSP_INFO *BpGetVoipDspConfig( unsigned char dspNum )
{
    VOIP_DSP_INFO *pDspConfig = 0;
    int i;
    bp_elem_t *pelem;
    int iDspType;
    enum bp_id bp_aucDspType[BP_MAX_VOIP_DSP+1] = {bp_ucDspType0, bp_ucDspType1, bp_last};
    enum bp_id bp_current, bp_next;

    if( g_pCurrentBp ) {
        /* First initialize the structure to known values */
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ ) {
            g_VoIPDspInfo[i].ucDspType = BP_VOIP_NO_DSP;
        }

        /* Now populate it with what we have in the element array */
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ ) {
            iDspType = 0;
            bp_current = bp_aucDspType[i];
            bp_next    = bp_aucDspType[i+1];
            pelem = BpGetElem(bp_current, &iDspType, bp_next);
            if (bp_current != pelem->id) {
                continue;
            }
            g_VoIPDspInfo[i].ucDspType = pelem->u.uc;

            ++iDspType;
            g_VoIPDspInfo[i].ucDspAddress       = BpGetSubUc(bp_ucDspAddress, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioLedVoip      = BpGetSubUs(bp_usGpioLedVoip, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip1Led     = BpGetSubUs(bp_usGpioVoip1Led, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip1LedFail = BpGetSubUs(bp_usGpioVoip1LedFail, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip2Led     = BpGetSubUs(bp_usGpioVoip2Led, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioVoip2LedFail = BpGetSubUs(bp_usGpioVoip2LedFail, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioPotsLed      = BpGetSubUs(bp_usGpioPotsLed, iDspType, bp_next);
            g_VoIPDspInfo[i].usGpioDectLed      = BpGetSubUs(bp_usGpioDectLed, iDspType, bp_next);
        }

        /* Transfer the requested results */
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ ) {
            if( g_VoIPDspInfo[i].ucDspType != BP_VOIP_NO_DSP &&
                g_VoIPDspInfo[i].ucDspAddress == dspNum ) {
                pDspConfig = &g_VoIPDspInfo[i];
                break;
            }
        }
    }

    return pDspConfig;
} /* BpGetVoipDspConfig */

/**************************************************************************
* Name       : BpSetBoardId
*
* Description: This function find the BOARD_PARAMETERS structure for the
*              specified board id string and assigns it to a global, static
*              variable.
*
* Parameters : [IN] pszBoardId - Board id string that is saved into NVRAM.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_FOUND - Error, board id input string does not
*                  have a board parameters configuration record.
***************************************************************************/
int BpSetBoardId( char *pszBoardId )
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    bp_elem_t **ppcBp;

    for( ppcBp = g_BoardParms; *ppcBp; ppcBp++ ) {
        if( !bpstrcmp((*ppcBp)[0].u.cp, pszBoardId) ) {
            g_pCurrentBp = *ppcBp;
            nRet = BP_SUCCESS;
            break;
        }
    }

    return( nRet );
} /* BpSetBoardId */

/**************************************************************************
* Name       : BpGetBoardId
*
* Description: This function returns the current board id strings.
*
* Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
*                  string is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
***************************************************************************/
int BpGetBoardId( char *pszBoardId )
{
    int i;

    if (g_pCurrentBp == 0) {
        return -1;
    }

    for (i = 0; i < BP_BOARD_ID_LEN; i++) {
        pszBoardId[i] = g_pCurrentBp[0].u.cp[i];
    }

    return 0;
}

/**************************************************************************
* Name       : BpGetBoardIds
*
* Description: This function returns all of the supported board id strings.
*
* Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
*                  strings are returned in.  Each id starts at BP_BOARD_ID_LEN
*                  boundary.
*              [IN] nBoardIdsSize - Number of BP_BOARD_ID_LEN elements that
*                  were allocated in pszBoardIds.
*
* Returns    : Number of board id strings returned.
***************************************************************************/
int BpGetBoardIds( char *pszBoardIds, int nBoardIdsSize )
{
    int i;
    char *src;
    char *dest;
    bp_elem_t **ppcBp;

    for( i = 0, ppcBp = g_BoardParms; *ppcBp && nBoardIdsSize;
        i++, ppcBp++, nBoardIdsSize--, pszBoardIds += BP_BOARD_ID_LEN ) {
        dest = pszBoardIds;
        src = (*ppcBp)[0].u.cp;
        while( *src ) {
            *dest++ = *src++;
        }
        *dest = '\0';
    }

    return( i );
} /* BpGetBoardIds */

/**************************************************************************
* Name       : BpGetComment
*
* Description: This function returns is used to get a comment for a board.
*
* Parameters : [OUT] pcpValue - comment string.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetComment( char **pcpValue )
{
    return( BpGetCp(bp_cpComment, pcpValue ) );
} /* BpGetComment */

/**************************************************************************
* Name       : BpGetGPIOverlays
*
* Description: This function GPIO overlay configuration
*
* Parameters : [OUT] pusValue - Address of short word that interfaces in use.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGPIOverlays( unsigned long *pulValue )
{
    return( BpGetUl(bp_usGpioOverlay, pulValue ) );
} /* BpGetGPIOverlays */


/**************************************************************************
* Name       : BpGetEthernetMacInfo
*
* Description: This function returns all of the supported board id strings.
*
* Parameters : [OUT] pEnetInfos - Address of an array of ETHERNET_MAC_INFO
*                  buffers.
*              [IN] nNumEnetInfos - Number of ETHERNET_MAC_INFO elements that
*                  are pointed to by pEnetInfos.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
int BpGetEthernetMacInfo( PETHERNET_MAC_INFO pEnetInfos, int nNumEnetInfos )
{
    int i, j;
    bp_elem_t *pelem;
    int iPhyType;
    int iPhyId;
    int nRet = BP_BOARD_ID_NOT_SET;
    PETHERNET_MAC_INFO pE;
    enum bp_id bp_aucPhyType[BP_MAX_ENET_MACS+1] = {bp_ucPhyType0, bp_ucPhyType1, bp_last};
    enum bp_id bp_current, bp_next;
    enum bp_id bp_aulPhyId[BP_MAX_SWITCH_PORTS+1] = {bp_ulPhyId0, bp_ulPhyId1, bp_ulPhyId2,
                bp_ulPhyId3, bp_ulPhyId4, bp_ulPhyId5, bp_ulPhyId6, bp_ulPhyId7, bp_last};
    enum bp_id bp_current_phyid;

    /* First initialize the structure to known values */
    for( i = 0, pE = pEnetInfos; i < nNumEnetInfos; i++, pE++ ) {
        pE->ucPhyType = BP_ENET_NO_PHY;
        /* The old code only initialized the first set, let's tdo the same so the 2 compare without error */
        if (0 == i) {
            for( j = 0; j < BP_MAX_ENET_INTERNAL; j++ ) {
                pE->sw.ledInfo[j].duplexLed = BP_NOT_DEFINED;
                pE->sw.ledInfo[j].speedLed100 = BP_NOT_DEFINED;
                pE->sw.ledInfo[j].speedLed1000 = BP_NOT_DEFINED;
            }
        }
    }

    if( g_pCurrentBp ) {
        /* Populate it with what we have in the element array */
        for( i = 0, pE = pEnetInfos; i < BP_MAX_ENET_MACS; i++, pE++ ) {
            iPhyType = 0;
            bp_current = bp_aucPhyType[i];
            bp_next    = bp_aucPhyType[i+1];
            pelem = BpGetElem(bp_current, &iPhyType, bp_next);
            if (bp_current != pelem->id)
                continue;
            pE->ucPhyType = pelem->u.uc;

            ++iPhyType;
            pE->ucPhyAddress  = BpGetSubUc(bp_ucPhyAddress, iPhyType, bp_next);
            pE->usConfigType  = BpGetSubUs(bp_usConfigType, iPhyType, bp_next);
            pE->sw.port_map   = BpGetSubUl(bp_ulPortMap, iPhyType, bp_next);

            for( j = 0; j < BP_MAX_SWITCH_PORTS; j++ ) {
                iPhyId = iPhyType;
                bp_current_phyid = bp_aulPhyId[j];
                pelem = BpGetElem(bp_current_phyid, &iPhyId, bp_next);
                if (bp_current_phyid == pelem->id) {
                    pE->sw.phy_id[j] = pelem->u.ul & ~MII_OVER_GPIO_VALID;
                    ++iPhyId;
                    if (j < BP_MAX_ENET_INTERNAL) {
                        while (iPhyId) {
                            switch (g_pCurrentBp[iPhyId].id) {
                            case bp_usDuplexLed:
                                pE->sw.ledInfo[j].duplexLed = g_pCurrentBp[iPhyId].u.us;
                                ++iPhyId;
                                break;
                            case bp_usSpeedLed100:
                                pE->sw.ledInfo[j].speedLed100 = g_pCurrentBp[iPhyId].u.us;
                                ++iPhyId;
                                break;
                            case bp_usSpeedLed1000:
                                pE->sw.ledInfo[j].speedLed1000 = g_pCurrentBp[iPhyId].u.us;
                                ++iPhyId;
                                break;
                            default:
                                iPhyId = 0;
                                break;
                            }
                        }
                    }
                } else {
                    pE->sw.phy_id[j] = 0;
                }
            }
        }
        nRet = BP_SUCCESS;
    }
    return( nRet );

} /* BpGetEthernetMacInfo */

#if defined(CONFIG_BCM963268) && (CONFIG_BCM_EXT_SWITCH)
int BpGetPortConnectedToExtSwitch(void)
{
    unsigned long phy_id, port_map;
    int i, extSwConnPort = -1;

    BpGetUl(bp_ulPortMap, &port_map);

    for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
        if ( port_map &  (1 << i)) {
            BpGetUl(bp_ulPhyId0 + i, &phy_id);
            if (phy_id & EXTSW_CONNECTED)
            {
                extSwConnPort = i;
            }
        }
    }
    return extSwConnPort;
}
#endif

/**************************************************************************
* Name       : BpGetMiiOverGpioFlag
*
* Description: This function returns logical disjunction of MII over GPIO
*              flag over all PHY IDs.
*
* Parameters : [OUT] pMiiOverGpioFlag - MII over GPIO flag
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
***************************************************************************/
int BpGetMiiOverGpioFlag( unsigned long* pMiiOverGpioFlag )
{
    int i, j;
    bp_elem_t *pelem;
    int iPhyType;
    int iPhyId;
    int nRet;
    enum bp_id bp_aucPhyType[BP_MAX_ENET_MACS+1] = {bp_ucPhyType0, bp_ucPhyType1, bp_last};
    enum bp_id bp_current, bp_next;
    enum bp_id bp_aulPhyId[BP_MAX_SWITCH_PORTS+1] = {bp_ulPhyId0, bp_ulPhyId1, bp_ulPhyId2,
                bp_ulPhyId3, bp_ulPhyId4, bp_ulPhyId5, bp_ulPhyId6, bp_ulPhyId7, bp_last};
    enum bp_id bp_current_phyid;

    *pMiiOverGpioFlag = 0;

    if( g_pCurrentBp ) {
        for( i = 0; i < BP_MAX_ENET_MACS; i++ ) {
            iPhyType = 0;
            bp_current = bp_aucPhyType[i];
            bp_next    = bp_aucPhyType[i+1];
            pelem = BpGetElem(bp_current, &iPhyType, bp_next);
            if (bp_current != pelem->id)
                continue;

            ++iPhyType;
            for( j = 0; j < BP_MAX_SWITCH_PORTS; j++ ) {
                iPhyId = iPhyType;
                bp_current_phyid = bp_aulPhyId[j];
                pelem = BpGetElem(bp_current_phyid, &iPhyId, bp_next);
                if (bp_current_phyid == pelem->id) {
                    *pMiiOverGpioFlag |= pelem->u.ul & MII_OVER_GPIO_VALID;                
                    ++iPhyId;
                }
            }
        }
        // Normalize flag value by positioning in lsb position        
        *pMiiOverGpioFlag >>= MII_OVER_GPIO_S;
        nRet = BP_SUCCESS;
    }
    else
    {
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );

} /* BpGetMiiOverGpioFlag */

/**************************************************************************
* Name       : BpGetRj11InnerOuterPairGpios
*
* Description: This function returns the GPIO pin assignments for changing
*              between the RJ11 inner pair and RJ11 outer pair.
*
* Parameters : [OUT] pusInner - Address of short word that the RJ11 inner pair
*                  GPIO pin is returned in.
*              [OUT] pusOuter - Address of short word that the RJ11 outer pair
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetRj11InnerOuterPairGpios( unsigned short *pusInner,
                                 unsigned short *pusOuter )
{
    *pusInner = BP_NOT_DEFINED;
    *pusOuter = BP_NOT_DEFINED;

    return( BP_VALUE_NOT_DEFINED );
} /* BpGetRj11InnerOuterPairGpios */

/**************************************************************************
* Name       : BpGetUartRtsCtsGpios
*
* Description: This function returns the GPIO pin assignments for RTS and CTS
*              UART signals.
*
* Parameters : [OUT] pusRts - Address of short word that the UART RTS GPIO
*                  pin is returned in.
*              [OUT] pusCts - Address of short word that the UART CTS GPIO
*                  pin is returned in.
*
* Returns    : BP_SUCCESS - Success, values are returned.
*              BP_BOARD_ID_NOT_SET - Error, board id input string does not
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetRtsCtsUartGpios( unsigned short *pusRts, unsigned short *pusCts )
{
    *pusRts = BP_NOT_DEFINED;
    *pusCts = BP_NOT_DEFINED;

    return( BP_VALUE_NOT_DEFINED );
} /* BpGetUartRtsCtsGpios */

/**************************************************************************
* Name       : BpGetAdslLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAdslLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedAdsl, pusValue ) );
} /* BpGetAdslLedGpio */

/**************************************************************************
* Name       : BpGetAdslFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED that is used when there is a DSL connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetAdslFailLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedAdslFail, pusValue ) );
} /* BpGetAdslFailLedGpio */

/**************************************************************************
* Name       : BpGetSecAdslLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED of the Secondary line, applicable more for bonding.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSecAdslLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioSecLedAdsl, pusValue ) );
} /* BpGetSecAdslLedGpio */

/**************************************************************************
* Name       : BpGetSecAdslFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the ADSL
*              LED of the Secondary ADSL line, that is used when there is
*              a DSL connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the ADSL LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSecAdslFailLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioSecLedAdslFail, pusValue ) );
} /* BpGetSecAdslFailLedGpio */

/**************************************************************************
* Name       : BpGetWirelessAntInUse
*
* Description: This function returns the antennas in use for wireless
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Antenna
*                  is in use.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessAntInUse( unsigned short *pusValue )
{
    return( BpGetUs(bp_usAntInUseWireless, pusValue ) );
} /* BpGetWirelessAntInUse */

/**************************************************************************
* Name       : BpGetWirelessFlags
*
* Description: This function returns optional control flags for wireless
*
* Parameters : [OUT] pusValue - Address of short word control flags
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessFlags( unsigned short *pusValue )
{
    return( BpGetUs(bp_usWirelessFlags, pusValue ) );
} /* BpGetWirelessAntInUse */

/**************************************************************************
* Name       : BpGetWirelessSesExtIntr
*
* Description: This function returns the external interrupt number for the
*              Wireless Ses Button.
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
*                  external interrup is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessSesExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrSesBtnWireless, pusValue ) );
} /* BpGetWirelessSesExtIntr */

/**************************************************************************
* Name       : BpGetWirelessSesLedGpio
*
* Description: This function returns the GPIO pin assignment for the Wireless
*              Ses Led.
*
* Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
*                  Led GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessSesLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedSesWireless, pusValue ) );
} /* BpGetWirelessSesLedGpio */

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
int BpGetWirelessLedGpioRed( unsigned short *pusValue )
{
    return ( BpGetUs(bp_usGpioLedWirelessRed, pusValue ) );
}
int BpGetWirelessLedGpioGreen( unsigned short *pusValue )
{
    return ( BpGetUs(bp_usGpioLedWirelessGreen, pusValue ) );
}
int BpGetWirelessLedGpioAct( unsigned short *pusValue )
{
    return ( BpGetUs(bp_usGpioLedWirelessAct, pusValue ) );
}
#endif

/* this data structure could be moved to boardparams structure in the future */
/* does not require to rebuild cfe here if more srom entries are needed */
static WLAN_SROM_PATCH_INFO wlanPaInfo[]={
#if defined(_BCM96362_) || defined(CONFIG_BCM96362)
    /* this is the patch to srom map for 96362ADVNX */
    {"96362ADVNX",    0x6362, 220,
    {{65, 0x1100},
    { 87, 0x0319},
    { 96, 0x2058},
    { 97, 0xfe6f},
    { 98, 0x1785},
    { 99, 0xfa21},
    {112, 0x2058},
    {113, 0xfe77},
    {114, 0x17e0},
    {115, 0xfa16},
    {161, 0x5555},
    {162, 0x5555},
    {169, 0x5555},
    {170, 0x5555},
    {171, 0x5555},
    {172, 0x5555},
    {173, 0x3333},
    {174, 0x3333},
    {175, 0x3333},
    {176, 0x3333},
    {0,      0}}},
    /* this is the patch to srom map for 6362ADVNgr2 */
    {"96362ADVNgr2",  0x6362, 220,
    {{65, 0x1500},
    { 96, 0x2040},
    { 97, 0xffb5},
    { 98, 0x175f},
    { 99, 0xfb29},
    {100, 0x3c3c},
    {101, 0x3c38},    
    {112, 0x2040},
    {113, 0xffd7},
    {114, 0x17d6},
    {115, 0xfb67},
    {116, 0x3c3c},  
    {117, 0x3c38},
    {203, 0x2222},  
    {  0,      0}}},
    /* this is the patch to srom map for 63629 */
    /* this should be same as 6362ADVNgr2 */
    {"963629epon",  0x6362, 220,
    {{65, 0x1500},
    { 96, 0x2040},
    { 97, 0xffb5},
    { 98, 0x175f},
    { 99, 0xfb29},
    {100, 0x3c3c},
    {101, 0x3c38},    
    {112, 0x2040},
    {113, 0xffd7},
    {114, 0x17d6},
    {115, 0xfb67},
    {116, 0x3c3c},  
    {117, 0x3c38},
    {203, 0x2222},  
    {  0,      0}}},
    /* this is the patch to srom map for 96362ADVN2xh */
    {"96362ADVN2xh",  0x6362, 220,
    {{65, 0x1200},                
    { 66, 0x3200},
    { 67, 0x8000},
    { 78, 0x0003},
    { 79, 0x0000},
    { 80, 0x0000},
    { 87, 0x031f},
    { 88, 0x031f},
    { 93, 0x0202},
    { 96, 0x2068},
    { 97, 0xfe80},
    { 98, 0x1fa2},
    { 99, 0xf897},
    {112, 0x2068},
    {113, 0xfe8c},
    {114, 0x1f13},
    {115, 0xf8c4},
    {203, 0x2222},
    {  0,      0}}},
    /* this is the patch to srom map for 96361XF */
    {"96361XF",  0x6362, 220,
    {{65, 0x1100},                
    { 66, 0x0200},
    { 67, 0x0000},
    { 78, 0x0003},
    { 79, 0x0000},
    { 80, 0x0000},
    { 87, 0x0313},
    { 88, 0x0313},
    { 93, 0xffff},
    { 96, 0x2054},
    { 97, 0xfe80},
    { 98, 0x1fa2},
    { 99, 0xf897},
    {112, 0x2054},
    {113, 0xfe8c},
    {114, 0x1f13},
    {115, 0xf8c4},
    {161, 0x4444},
    {162, 0x4444},
    {169, 0x4444},
    {170, 0x4444},
    {171, 0x4444},
    {172, 0x4444},
    {203, 0x2222},
    {  0,      0}}},
  /* this is the patch to srom map for 6362RADVNgr2 */
    {"96362RADVNgr2",  0x6362, 220,
    {{65, 0x1500},
    { 96, 0x2040},
    { 97, 0xffb5},
    { 98, 0x175f},
    { 99, 0xfb29},
    {100, 0x3c3c},
    {101, 0x3c38},    
    {112, 0x2040},
    {113, 0xffd7},
    {114, 0x17d6},
    {115, 0xfb67},
    {116, 0x3c3c},  
    {117, 0x3c38},
    {203, 0x2222},  
    {  0,      0}}},    
#endif

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)
    /* this is the patch to srom map for 63268MBV */
    {"963268MBV",   0x6362, 220, 
    {{ 2,  0x05BB},
    { 65,  0x1204},
    { 78,  0x0303}, 
    { 79,  0x0202}, 
    { 80,  0xff02},
    { 87,  0x0315}, 
    { 88,  0x0315},
    { 96,  0x2048}, 
    { 97,  0xFFB5}, 
    { 98,  0x175F},
    { 99,  0xFB29},
    { 100, 0x3E3C},
    { 101, 0x4038},
    { 102, 0xFE7F},
    { 103, 0x1279},
    { 112, 0x2048},
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 116, 0x3E3C},
    { 117, 0x4038},
    { 118, 0xFE87},
    { 119, 0x1233},
    { 203, 0x2226},
    {   0,      0}}},
    /* this is the patch to srom map for 963168MBV_17A */
    {"963168MBV_17A",   0x6362, 220, 
    {{ 2,  0x05BB},
    { 65,  0x1204},
    { 78,  0x0303}, 
    { 79,  0x0202}, 
    { 80,  0xff02},
    { 87,  0x0315}, 
    { 88,  0x0315},
    { 96,  0x2048}, 
    { 97,  0xFFB5}, 
    { 98,  0x175F},
    { 99,  0xFB29},
    { 100, 0x3E3C},
    { 101, 0x4038},
    { 102, 0xFE7F},
    { 103, 0x1279},
    { 112, 0x2048},
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 116, 0x3E3C},
    { 117, 0x4038},
    { 118, 0xFE87},
    { 119, 0x1233},
    { 203, 0x2226},
    {   0,      0}}},   
    /* this is the patch to srom map for 963168MBV_30A */
    {"963168MBV_30A",   0x6362, 220, 
    {{ 2,  0x05BB},
    { 65,  0x1204},
    { 78,  0x0303}, 
    { 79,  0x0202}, 
    { 80,  0xff02},
    { 87,  0x0315}, 
    { 88,  0x0315},
    { 96,  0x2048}, 
    { 97,  0xFFB5}, 
    { 98,  0x175F},
    { 99,  0xFB29},
    { 100, 0x3E3C},
    { 101, 0x4038},
    { 102, 0xFE7F},
    { 103, 0x1279},
    { 112, 0x2048},
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 116, 0x3E3C},
    { 117, 0x4038},
    { 118, 0xFE87},
    { 119, 0x1233},
    { 203, 0x2226},
    {   0,      0}}},       
    /* this is the patch to srom map for 963268V30A */
    {"963268V30A",   0x6362, 220, 
    {{ 2,  0x05E7},
    { 65,  0x1101},
    { 78,  0x0303}, 
    { 79,  0x0202}, 
    { 80,  0xff02},
    { 87,  0x0315}, 
    { 88,  0x0315},
    { 96,  0x2048}, 
    { 97,  0xFFB5}, 
    { 98,  0x175F},
    { 99,  0xFB29},
    { 100, 0x3E3C},
    { 101, 0x4038},
    { 102, 0xFE7F},
    { 103, 0x1279},
    { 112, 0x2048},
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 116, 0x3E3C},
    { 117, 0x4038},
    { 118, 0xFE87},
    { 119, 0x1233},
    { 203, 0x2226},
    {   0,      0}}},       
    /* this is the patch to srom map for 63268BU */
    {"963268BU",   0x6362, 220,
    {{ 2,  0x05A7},
    { 65,  0x1201},
    { 78,  0x0303},
    { 79,  0x0202},
    { 80,  0xff02},
    { 87,  0x0315},
    { 88,  0x0315},
    { 96,  0x2048},
    { 97,  0xFFB5},
    { 98,  0x175F},
    { 99,  0xFB29},
    { 100, 0x3E3C},
    { 101, 0x4038},
    { 102, 0xFE7F},
    { 103, 0x1279},
    { 112, 0x2048}, 
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 116, 0x3E3C},
    { 117, 0x4038},
    { 118, 0xFE87},
    { 119, 0x1233},
    { 203, 0x2226},
    { 0,        0}}},
    /* this is the patch to srom map for 63268BU_P300 */
    {"963268BU_P300",   0x6362, 220,
    {{ 2,  0x05A7},
    { 65,  0x1201},
    { 78,  0x0303},
    { 79,  0x0202},
    { 80,  0xff02},
    { 87,  0x0315},
    { 88,  0x0315},
    { 96,  0x2048},
    { 97,  0xFFB5},
    { 98,  0x175F},
    { 99,  0xFB29},
    { 100, 0x3E3C},
    { 101, 0x4038},
    { 102, 0xFE7F},
    { 103, 0x1279},
    { 112, 0x2048}, 
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 116, 0x3E3C},
    { 117, 0x4038},
    { 118, 0xFE87},
    { 119, 0x1233},
    { 203, 0x2226},
    { 0,        0}}},
    /* this is the patch to srom map for 63168XH */
    {"963168XH",   0x6362, 220,
    {{ 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88,  0x031f},
    { 93,  0x0202},
    { 96,  0x2068},
    { 97,  0xfe80},
    { 98,  0x1fa2},
    { 99,  0xf897},
    { 112, 0x2068},
    { 113, 0xfe8c},
    { 114, 0x1f13},
    { 115, 0xf8c4},
    { 161, 0x0000},
    { 162, 0x0000},
    { 169, 0x0000},
    { 170, 0x0000},
    { 171, 0x0000},
    { 172, 0x0000},
    { 203, 0x2222},
    {   0,      0}}},
#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
/* AEI start */
    /* this is the patch to srom map for C1000A */
    {"C1000A",   0x6362, 220,
    {{ 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88,  0x031f},
    { 93,  0x0202},
    { 96,  0x2068},
    { 97,  0xfe80},
    { 98,  0x1fa2},
    { 99,  0xf897},
    { 112, 0x2068},
    { 113, 0xfe8c},
    { 114, 0x1f13},
    { 115, 0xf8c4},
    { 161, 0x0000},
    { 162, 0x0000},
    { 169, 0x0000},
    { 170, 0x0000},
    { 171, 0x0000},
    { 172, 0x0000},
    { 203, 0x2222},
    {   0,      0}}},
    /* this is the patch to srom map for C2000A */
    {"C2000A",   0x6362, 220,
    {
    { 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88, 0x031f},
    { 93, 0x0202},
    { 96, 0x2068},
    { 97, 0xfe65},
    { 98, 0x1d07},
    { 99, 0xf8e5},
    { 112,  0x2068},
    { 113,  0xfe75},
    { 114,  0x1d43},
    { 115,  0xf8f5},
    { 161,  0x0},
    { 162,  0x0},
    { 169, 0x0},
    { 170, 0x0},
    { 171, 0x0},
    { 172,  0x0},
    { 203,  0x2222},
    {   0,      0}}},
    /* this is the patch to srom map for C1000B */
    {"C1000B",   0x6362, 220,
    {{ 97,  0xfea2},
    { 98,  0x1666},
    { 99,  0xfa8e},
    { 102, 0xff43},
    { 113, 0xfe8c},
    { 114, 0x15b0},
    { 115, 0xfa79},
    {   0,      0}}},
    /* this is the patch to srom map for V2200, the same with C2000A*/
    {"V2200",   0x6362, 220,
    {
    { 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88, 0x031f},
    { 93, 0x0202},
    { 96, 0x2068},
    { 97, 0xfe7d},
    { 98, 0x1df4},
    { 99, 0xf8df},
    { 112,  0x2068},
    { 113,  0xfe95},
    { 114,  0x1e98},
    { 115,  0xf8da},
    { 161,  0x0},
    { 162,  0x0},
    { 169, 0x0},
    { 170, 0x0},
    { 171, 0x0},
    { 172,  0x0},
    { 203,  0x2222},
    {   0,      0}}},   	
    /* this is the patch to srom map for V2200H, the same with C2000A*/
    {"V2200H",   0x6362, 220,
    {
    { 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88, 0x031f},
    { 93, 0x0202},
    { 96, 0x2068},
    { 97, 0xfe7d},
    { 98, 0x1df4},
    { 99, 0xf8df},
    { 112,  0x2068},
    { 113,  0xfe95},
    { 114,  0x1e98},
    { 115,  0xf8da},
    { 161,  0x0},
    { 162,  0x0},
    { 169, 0x0},
    { 170, 0x0},
    { 171, 0x0},
    { 172,  0x0},
    { 203,  0x2222},
    {   0,      0}}},   	
    /* this is the patch to srom map for R2200H, the same with C2000A*/
    {"R2200H",   0x6362, 220,
    {
    { 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88, 0x031f},
    { 93, 0x0202},
    { 96, 0x2068},
    { 97, 0xfe7d},
    { 98, 0x1df4},
    { 99, 0xf8df},
    { 112,  0x2068},
    { 113,  0xfe95},
    { 114,  0x1e98},
    { 115,  0xf8da},
    { 161,  0x0},
    { 162,  0x0},
    { 169, 0x0},
    { 170, 0x0},
    { 171, 0x0},
    { 172,  0x0},
    { 203,  0x2222},
    {   0,      0}}},   	
    /* this is the patch to srom map for FV2200, the same with C2000A*/
    {"FV2200",   0x6362, 220,
    {
    { 2,  0x05e2},
    { 65,  0x1100},
    { 66,  0x3200},
    { 67,  0x8000},
    { 87,  0x031f},
    { 88, 0x031f},
    { 93, 0x0202},
    { 96, 0x2068},
    { 97, 0xfe7d},
    { 98, 0x1df4},
    { 99, 0xf8df},
    { 112,  0x2068},
    { 113,  0xfe95},
    { 114,  0x1e98},
    { 115,  0xf8da},
    { 161,  0x0},
    { 162,  0x0},
    { 169, 0x0},
    { 170, 0x0},
    { 171, 0x0},
    { 172,  0x0},
    { 203,  0x2222},
    {   0,      0}}},
/* AEI end */
#endif
#endif

#if defined(_BCM96828_) || defined(CONFIG_BCM96828)
    /* this is the patch to srom map for 63268MBV */
    {"96828MBV",   0x6362, 220, 
    {{65,  0x1230},
    { 78,  0x0303}, 
    { 79,  0x0202}, 
    { 80,  0xff02},
    { 87,  0x0315}, 
    { 88,  0x0315},
    { 96,  0x2040}, 
    { 97,  0xFFB5}, 
    { 98,  0x175F},
    { 99,  0xFB29},
    { 112, 0x2040},
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    {   0,      0}}},
    /* this is the patch to srom map for 63268BU */
    {"96828BU",   0x6362, 220,
    {{65,  0x1230},
    { 78,  0x0303},
    { 79,  0x0202},
    { 80,  0xff02},
    { 87,  0x0315},
    { 88,  0x0315},
    { 96,  0x2040},
    { 97,  0xFFB5},
    { 98,  0x175F},
    { 99,  0xFB29},
    { 112, 0x2040}, 
    { 113, 0xFFD7},
    { 114, 0x17D6},
    { 115, 0xFB67},
    { 0,        0}}},

#endif

      
    {"", 0, 0, {{0,0}}}, /* last entry*/
};

/**************************************************************************
* Name       : BpUpdateWirelessSromMap
*
* Description: This function patch wireless PA values
*
* Parameters : [IN] unsigned short chipID
*              [IN/OUT] unsigned short* pBase - base of srom map
*              [IN/OUT] int size - size of srom map
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpUpdateWirelessSromMap(unsigned short chipID, unsigned short* pBase, int sizeInWords)
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    int i = 0;
    int j = 0;

    if(chipID == 0 || pBase == 0 || sizeInWords <= 0 )
        return nRet;

    i = 0;
    while ( wlanPaInfo[i].szboardId[0] != 0 ) {
        /* check boardId */
        if ( !bpstrcmp(g_pCurrentBp[0].u.cp, wlanPaInfo[i].szboardId) ) {
            /* check chipId */
            if ( (wlanPaInfo[i].usWirelessChipId == chipID) && (wlanPaInfo[i].usNeededSize <= sizeInWords) ){
                /* valid , patch entry */
                while ( wlanPaInfo[i].entries[j].wordOffset != 0) {
                    pBase[wlanPaInfo[i].entries[j].wordOffset] = wlanPaInfo[i].entries[j].value;
                    j++;
                }
                nRet = BP_SUCCESS;
                goto srom_update_done;
            }
        }
        i++;
    }

srom_update_done:

    return( nRet );

} /* BpUpdateWirelessSromMap */


static WLAN_PCI_PATCH_INFO wlanPciInfo[]={
#if defined(_BCM96362_) || defined(CONFIG_BCM96362)
    /* this is the patch to boardtype(boardid) for internal PA */
    {"96362ADVNX", 0x435f14e4, 64,
    {{"subpciids", 11, 0x53614e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 96362ADVNgr2 */
    {"96362ADVNgr2", 0x435f14e4, 64,
    {{"subpciids", 11, 0x58014e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 96362ADVN2xh */
    {"96362ADVN2xh", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5a614e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 96361XF */
    {"96361XF", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5b814e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 96361I2 */
    {"96361I2", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5e514e4},
    {"",       0,      0}}},
#endif
#if defined(_BCM963268_) || defined(CONFIG_BCM963268)
    /* this is the patch to boardtype(boardid) for 63268MBV */
    {"963268MBV", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5BB14e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 963168MBV_17A */
    {"963168MBV_17A", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5BB14e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 963168MBV_30A */
    {"963168MBV_30A", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5BB14e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 963268V30A */
    {"963268V30A", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5E714e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 63268BU */
    {"63268BU", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5A714e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 963168VX */
    {"963168VX", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5A814e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for 63168XH */
    {"963168XH", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5E214e4},
    {"",       0,      0}}},
#if defined(AEI_VDSL_CUSTOMER_NCS) || defined(AEI_VDSL_BCMSDK)
/* AEI start */
    /* this is the patch to boardtype(boardid) for C1000 */
    {"C1000A", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5E214e4},
    {"",       0,      0}}},
    {"C1000B", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5E214e4},
    {"",       0,      0}}},
    /* this is the patch to boardtype(boardid) for C2000 */
    {"C2000A", 0x435f14e4, 64,
    {{"subpciids", 11, 0x5E214e4},
    {"",       0,      0}}},
/* AEI end */
#endif
#endif
    {"",                 0, 0, {{"",       0,      0}}}, /* last entry*/
};

/**************************************************************************
* Name       : BpUpdateWirelessPciConfig
*
* Description: This function patch wireless PCI Config Header
*              This is not functional critial/necessary but for dvt database maintenance
*
* Parameters : [IN] unsigned int pciID
*              [IN/OUT] unsigned int* pBase - base of pci config header
*              [IN/OUT] int sizeInDWords - size of pci config header
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpUpdateWirelessPciConfig (unsigned long pciID, unsigned long* pBase, int sizeInDWords)
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    int i = 0;
    int j = 0;

    if(pciID == 0 || pBase == 0 || sizeInDWords <= 0 )
        return nRet;

    i = 0;
    while ( wlanPciInfo[i].szboardId[0] != 0 ) {
        /* check boardId */
        if ( !bpstrcmp(g_pCurrentBp[0].u.cp, wlanPciInfo[i].szboardId) ) {
            /* check pciId */
            if ( (wlanPciInfo[i].usWirelessPciId == pciID) && (wlanPciInfo[i].usNeededSize <= sizeInDWords) ){
                /* valid , patch entry */
                while ( wlanPciInfo[i].entries[j].name[0] ) {
                    pBase[wlanPciInfo[i].entries[j].dwordOffset] = wlanPciInfo[i].entries[j].value;
                    j++;
                }
                nRet = BP_SUCCESS;
                goto pciconfig_update_done;
            }
        }
        i++;
    }

pciconfig_update_done:

    return( nRet );

}

#if defined(AEI_63168_CHIP)
/**************************************************************************
* Name       : BpGetEnetWanLedGpio
*
* Description: This function returns the GPIO pin assignment for the Enet WAN 
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the Enet WAN LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetEnetWanLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedEnetWan, pusValue ) );
} /* BpGetEnetWanLedGpio */
#endif

/**************************************************************************
* Name       : BpGetWanDataLedGpio
*
* Description: This function returns the GPIO pin assignment for the WAN Data
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the WAN Data LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanDataLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedWanData, pusValue ) );
} /* BpGetWanDataLedGpio */

/**************************************************************************
* Name       : BpGetWanErrorLedGpio
*
* Description: This function returns the GPIO pin assignment for the WAN
*              LED that is used when there is a WAN connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the WAN LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWanErrorLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedWanError, pusValue ) );
} /* BpGetWanErrorLedGpio */

/**************************************************************************
* Name       : BpGetBootloaderPowerOnLedGpio
*
* Description: This function returns the GPIO pin assignment for the power
*              on LED that is set by the bootloader.
*
* Parameters : [OUT] pusValue - Address of short word that the alarm LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBootloaderPowerOnLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedBlPowerOn, pusValue ) );
} /* BpGetBootloaderPowerOn */

/**************************************************************************
* Name       : BpGetBootloaderStopLedGpio
*
* Description: This function returns the GPIO pin assignment for the break
*              into bootloader LED that is set by the bootloader.
*
* Parameters : [OUT] pusValue - Address of short word that the break into
*                  bootloader LED GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetBootloaderStopLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedBlStop, pusValue ) );
} /* BpGetBootloaderStopLedGpio */

/**************************************************************************
* Name       : BpGetVoipLedGpio
*
* Description: This function returns the GPIO pin assignment for the VOIP
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the VOIP LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
*
* Note       : The VoIP structure would allow for having one LED per DSP
*              however, the board initialization function assumes only one
*              LED per functionality (ie one LED for VoIP).  Therefore in
*              order to keep this tidy and simple we do not make usage of the
*              one-LED-per-DSP function.  Instead, we assume that the LED for
*              VoIP is unique and associated with DSP 0 (always present on
*              any VoIP platform).  If changing this to a LED-per-DSP function
*              then one need to update the board initialization driver in
*              bcmdrivers\opensource\char\board\bcm963xx\impl1
***************************************************************************/
int BpGetVoipLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedVoip, pusValue ) );
} /* BpGetVoipLedGpio */

/**************************************************************************
* Name       : BpGetVoip1LedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP1.
*              LED which is used when FXS0 is active
* Parameters : [OUT] pusValue - Address of short word that the VoIP1
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip1LedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioVoip1Led, pusValue ) );
} /* BpGetVoip1LedGpio */

/**************************************************************************
* Name       : BpGetVoip1FailLedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP1
*              Fail LED which is used when there's an error with FXS0
* Parameters : [OUT] pusValue - Address of short word that the VoIP1
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip1FailLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioVoip1LedFail, pusValue ) );
} /* BpGetVoip1FailLedGpio */

/**************************************************************************
* Name       : BpGetVoip2LedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP2.
*              LED which is used when FXS1 is active
* Parameters : [OUT] pusValue - Address of short word that the VoIP2
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip2LedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioVoip2Led, pusValue ) );
} /* BpGetVoip2LedGpio */

/**************************************************************************
* Name       : BpGetVoip2FailLedGpio
*
* Description: This function returns the GPIO pin assignment for the VoIP2
*              Fail LED which is used when there's an error with FXS1
* Parameters : [OUT] pusValue - Address of short word that the VoIP2
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetVoip2FailLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioVoip2LedFail, pusValue ) );
} /* BpGetVoip2FailLedGpio */

/**************************************************************************
* Name       : BpGetPotsLedGpio
*
* Description: This function returns the GPIO pin assignment for the POTS1.
*              LED which is used when DAA is active
* Parameters : [OUT] pusValue - Address of short word that the POTS11
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPotsLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioPotsLed, pusValue ) );
} /* BpGetPotsLedGpio */

/**************************************************************************
* Name       : BpGetDectLedGpio
*
* Description: This function returns the GPIO pin assignment for the DECT.
*              LED which is used when DECT is active
* Parameters : [OUT] pusValue - Address of short word that the DECT
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetDectLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioDectLed, pusValue ) );
} /* BpGetDectLedGpio */


/**************************************************************************
* Name       : BpGetPassDyingGaspGpio
*
* Description: This function returns the GPIO pin assignment used to pass
*                  a dying gasp interrupt to an external processor.
* Parameters : [OUT] pusValue - Address of short word that the DECT
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetPassDyingGaspGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioPassDyingGasp, pusValue ) );
} /* BpGetPassDyingGaspGpio */


/**************************************************************************
* Name       : BpGetFpgaResetGpio
*
* Description: This function returns the GPIO pin assignment for the FPGA
*              Reset signal.
*
* Parameters : [OUT] pusValue - Address of short word that the FPGA Reset
*                  signal GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetFpgaResetGpio( unsigned short *pusValue ) {
    return( BpGetUs(bp_usGpioFpgaReset, pusValue ) );
} /*BpGetFpgaResetGpio*/

/**************************************************************************
* Name       : BpGetGponLedGpio
*
* Description: This function returns the GPIO pin assignment for the GPON
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the GPON LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGponLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedGpon, pusValue ) );
} /* BpGetGponLedGpio */

/**************************************************************************
* Name       : BpGetGponFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the GPON
*              LED that is used when there is a GPON connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the GPON LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetGponFailLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedGponFail, pusValue ) );
} /* BpGetGponFailLedGpio */

/**************************************************************************
* Name       : BpGetMoCALedGpio
*
* Description: This function returns the GPIO pin assignment for the MoCA
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the MoCA LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMoCALedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedMoCA, pusValue ) );
} /* BpGetMoCALedGpio */

/**************************************************************************
* Name       : BpGetMoCAFailLedGpio
*
* Description: This function returns the GPIO pin assignment for the MoCA
*              LED that is used when there is a MoCA connection failure.
*
* Parameters : [OUT] pusValue - Address of short word that the MoCA LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetMoCAFailLedGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLedMoCAFail, pusValue ) );
} /* BpGetMoCAFailLedGpio */

#if defined(AEI_VDSL_CUSTOMER_NCS)
/**************************************************************************
* Name       : BpGetUsbLedGpio
*
* Description: This function returns the GPIO pin assignment for the USB
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the USB LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetUsbLedGpio( unsigned short *pusValue )
{
    return ( BpGetUs(bp_usGpioLedUsb, pusValue ) );
} /* BpGetUsbLedGpio */

/**************************************************************************
 * * Name       : BpGetWirelessFailSesLedGpio
 * *   
 * * Description: This function returns the GPIO pin assignment for the Wireless
 * *              Ses Led.
 * *       
 * * Parameters : [OUT] pusValue - Address of short word that the Wireless Ses
 * *                  Led GPIO pin is returned in.
 * *   
 * * Returns    : BP_SUCCESS - Success, value is returned.
 * *              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
 * *              BP_VALUE_NOT_DEFINED - At least one return value is not defined
 * *                  for the board.
 * ***************************************************************************/
int BpGetWirelessFailSesLedGpio( unsigned short *pusValue )
{
//    int nRet;

    return ( BpGetUs(bp_usGpioLedSesWirelessFail, pusValue ) );
/*
    if( g_pCurrentBp )
    {
 
#if (!defined(CONFIG_BCM96328) && !defined(_BCM96328_))
      *pusValue = g_pCurrentBp->bp_usGpioLedSesWirelessFail;*/
/*        BpGetUs(bp_usGpioLedSesWirelessFail, pusValue);

        if( g_pCurrentBp->bp_usGpioLedSesWirelessFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
 #endif
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
*/
} /* BpGetWirelessFailSesLedGpio */
#endif

/**************************************************************************
* Name       : BpGetResetToDefaultExtIntr
*
* Description: This function returns the external interrupt number for the
*              reset to default button.
*
* Parameters : [OUT] pusValue - Address of short word that reset to default
*                  external interrupt is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetResetToDefaultExtIntr( unsigned short *pusValue )
{
    return( BpGetUs(bp_usExtIntrResetToDefault, pusValue ) );
} /* BpGetResetToDefaultExtIntr */

/**************************************************************************
* Name       : BpGetWirelessPowerDownGpio
*
* Description: This function returns the GPIO pin assignment for WLAN_PD
*
*
* Parameters : [OUT] pusValue - Address of short word that the WLAN_PD
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetWirelessPowerDownGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioWirelessPowerDown, pusValue ) );
} /* usGpioWirelessPowerDown */

/**************************************************************************
* Name       : BpGetDslPhyAfeIds
*
* Description: This function returns the DSL PHY AFE ids for primary and
*              secondary PHYs.
*
* Parameters : [OUT] pulValues-Address of an array of two long words where
*              AFE Id for the primary and secondary PHYs are returned.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET-Error, BpSetBoardId hasn't been called.
*              BP_VALUE_NOT_DEFINED - No defined AFE Ids.
**************************************************************************/
int BpGetDslPhyAfeIds( unsigned long *pulValues )
{
    int nRet;

    if( g_pCurrentBp )
    {
        if (BpGetUl(bp_ulAfeId0, &pulValues[0]) != BP_SUCCESS) {
          pulValues[0] = BP_AFE_DEFAULT;
        }
        if (BpGetUl(bp_ulAfeId1, &pulValues[1]) != BP_SUCCESS) {
          pulValues[1] = BP_AFE_DEFAULT;
        }
        nRet = BP_SUCCESS;
    }
    else
    {
        pulValues[0] = BP_AFE_DEFAULT;
        pulValues[1] = BP_AFE_DEFAULT;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetDslPhyAfeIds */

/**************************************************************************
* Name       : BpGetUart2SdoutGpio
*
* Description: This function returns the GPIO pin assignment for UART2 SDOUT
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioUart2Sdout
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetUart2SdoutGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioUart2Sdout, pusValue ) );
} /* BpGetUart2SdoutGpio */

/**************************************************************************
* Name       : BpGetUart2SdinGpio
*
* Description: This function returns the GPIO pin assignment for UART2 SDIN
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioUart2Sdin
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetUart2SdinGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioUart2Sdin, pusValue ) );
} /* BpGetUart2SdinGpio */

/**************************************************************************
* Name       : BpGetExtAFEResetGpio
*
* Description: This function returns the GPIO pin assignment for resetting the external AFE chip
*
*
* Parameters : [OUT] pusValue - Address of short word that the ExtAFEReset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFEResetGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioExtAFEReset, pusValue ) );
} /* BpGetExtAFEResetGpio */

/**************************************************************************
* Name       : BpGetAFELDRelayGpio
*
* Description: This function returns the GPIO pin assignment for switching LD relay
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioAFELDRelay
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetAFELDRelayGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioAFELDRelay, pusValue ) );
} /* BpGetAFELDRelayGpio */

/**************************************************************************
* Name       : BpGetIntAFELDModeGpio
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*                  for the internal path.
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioIntAFELDMode
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetIntAFELDModeGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioIntAFELDMode, pusValue ) );
} /* BpGetIntAFELDModeGpio */

/**************************************************************************
* Name       : BpGetIntAFELDPwrGpio
*
* Description: This function returns the GPIO pin assignment for turning on/off the internal AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDPwr
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetIntAFELDPwrGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioIntAFELDPwr, pusValue ) );
} /* BpGetIntAFELDPwrGpio */

/**************************************************************************
* Name       : BpGetExtAFELDModeGpio
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*                  for the external path.
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDMode
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDModeGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioExtAFELDMode, pusValue ) );
} /* BpGetExtAFELDModeGpio */

/**************************************************************************
* Name       : BpGetExtAFELDPwrGpio
*
* Description: This function returns the GPIO pin assignment for turning on/off the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioExtAFELDPwr
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDPwrGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioExtAFELDPwr, pusValue ) );
} /* BpGetExtAFELDPwrGpio */

/**************************************************************************
* Name       : BpGetExtAFELDDataGpio
*
* Description: This function returns the GPIO pin assignment for sending config data to the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioExtAFELDData
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetExtAFELDDataGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioExtAFELDData, pusValue ) );
} /* BpGetExtAFELDDataGpio */

/**************************************************************************
* Name       : BpGetExtAFELDClkGpio
*
* Description: This function returns the GPIO pin assignment for sending the clk to the external AFE LD
*
*
* Parameters : [OUT] pusValue - Address of short word that the bp_usGpioExtAFELDClk
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/

int BpGetExtAFELDClkGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioExtAFELDClk, pusValue ) );
} /* BpGetExtAFELDClkGpio */

/**************************************************************************
* Name       : BpGet6829PortInfo
*
* Description: This function checks the ENET MAC info to see if a 6829
*              is connected
*
* Parameters : [OUT] portInfo6829 - 0 if 6829 is not present
*                                 - 6829 port information otherwise
*
* Returns    : BP_SUCCESS           - Success, value is returned.
*              BP_BOARD_ID_NOT_SET  - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGet6829PortInfo( unsigned char *portInfo6829 )
{
   ETHERNET_MAC_INFO enetMacInfo;
   ETHERNET_SW_INFO *pSwInfo;
   int               retVal;
   int               i;

   *portInfo6829 = 0;
   retVal = BpGetEthernetMacInfo( &enetMacInfo, 1 );
   if ( BP_SUCCESS == retVal ) {
      pSwInfo = &enetMacInfo.sw;
      for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
         if ( ((pSwInfo->phy_id[i] & PHYID_LSBYTE_M) != 0xFF) &&
              ((pSwInfo->phy_id[i] & PHYID_LSBYTE_M) &  0x80) ) {
            *portInfo6829 = pSwInfo->phy_id[i] & PHYID_LSBYTE_M;
            retVal        = BP_SUCCESS;
            break;
         }
      }
   }

   return retVal;

}

/**************************************************************************
* Name       : BpGetEthSpdLedGpio
*
* Description: This function returns the GPIO pin assignment for the
*              specified port and link speed 
*
* Parameters : [IN] port - Internal phy number
*              [IN] enetIdx - index for Ethernet MAC info
*              [IN] ledIdx - 0 -> duplex GPIO
*                          - 1 -> spd 100 GPIO
*                          - 2 -> spd 1000 GPIO
*              [OUT] pusValue - Address of a short word to store the GPIO
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetEthSpdLedGpio( unsigned short port, unsigned short enetIdx,
                        unsigned short ledIdx, unsigned short *pusValue )
{
    ETHERNET_MAC_INFO enetMacInfos[BP_MAX_ENET_MACS];
    unsigned short *pShort;
    int nRet;

    if( g_pCurrentBp ) {
        nRet = BpGetEthernetMacInfo( enetMacInfos, BP_MAX_ENET_MACS );

        if ((enetIdx >= BP_MAX_ENET_MACS) ||
            (port >= BP_MAX_ENET_INTERNAL) ||
            (enetMacInfos[enetIdx].ucPhyType == BP_ENET_NO_PHY)) {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_VALUE_NOT_DEFINED;
        } else {
           pShort   = &enetMacInfos[enetIdx].sw.ledInfo[port].duplexLed;
           pShort   += ledIdx;
           *pusValue = *pShort;
           if( *pShort == BP_NOT_DEFINED ) {
               nRet = BP_VALUE_NOT_DEFINED;
           } else {
               nRet = BP_SUCCESS;
           }
        }
    } else {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetEthSpdLedGpio */


/**************************************************************************
* Name       : BpGetLaserDisGpio
*
* Description: This function returns the GPIO pin assignment for disabling
*              the laser
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioLaserDis
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetLaserDisGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLaserDis, pusValue ) );
} /* BpGetLaserDisGpio */


/**************************************************************************
* Name       : BpGetLaserTxPwrEnGpio
*
* Description: This function returns the GPIO pin assignment for enabling
*              the transmit power of the laser
*
* Parameters : [OUT] pusValue - Address of short word that the usGpioLaserTxPwrEn
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetLaserTxPwrEnGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioLaserTxPwrEn, pusValue ) );
} /* BpGetLaserTxPwrEnGpio */


/**************************************************************************
* Name       : BpGetVregSel1P2
*
* Description: This function returns the desired voltage level for 1V2
*
* Parameters : [OUT] pusValue - Address of short word that the 1V2 level
*                  is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetVregSel1P2( unsigned short *pusValue )
{
    return( BpGetUs(bp_usVregSel1P2, pusValue ) );
} /* BpGetVregSel1P2 */


/**************************************************************************
* Name       : BpGetGponOpticsType
*
* Description: This function returns an indication of whether the current 
*              board type supports GPON legacy or BOSA optics.
*
* Parameters : [Out] pusValue
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetGponOpticsType( unsigned short *pusValue )
{
    return ( BpGetUs (bp_usGponOpticsType, pusValue));
} /* BpGetGponOpticsType */


/**************************************************************************
* Name       : BpGetDefaultOpticalParams
*
* Description: This function returns the optical params for BOSA optics if
*              they exist.  These are only used if they do not exist in NVRAM.
*
* Parameters : [OUT] pOpticalParams - Address of a buffer that the optical
*              params are returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
***************************************************************************/
int BpGetDefaultOpticalParams( unsigned char *pOpticalParams )
{
    int Index, Ret;    
    unsigned char * pBpOpticalParams = (unsigned char *)0;


    if (BP_SUCCESS == (Ret = BpGetCp (bp_cpDefaultOpticalParams, (char**)&pBpOpticalParams)))
    {
        for (Index = 0; Index < BP_OPTICAL_PARAMS_LEN; Index++)
        {
            pOpticalParams[Index] = pBpOpticalParams[Index];
        }
    }

    return (Ret);

} /* BpGetDefaultOpticalParams */


/**************************************************************************
* Name       : BpGetFemtoResetGpio
*
* Description: This function returns the GPIO that needs to be toggled high
*              for 2 msec at least to reset the FEMTO chip
*
* Parameters : [OUT] pusValue - Address of short word that the GPIO for
*                  resetting FEMTO chip is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetFemtoResetGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioFemtoReset, pusValue ) );
} /*  BpGetFemtoResetGpio */



/**************************************************************************
* Name       : BpGetEphyBaseAddress
*
* Description: This function returns the base address requested for
*              the internal EPHYs
*
* Parameters : [OUT] pusValue - Address of short word for returned value.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - Not defined
***************************************************************************/
int BpGetEphyBaseAddress( unsigned short *pusValue )
{
    return( BpGetUs(bp_usEphyBaseAddress, pusValue ) );
} /*  BpGetEphyBaseAddress */


/**************************************************************************
* Name       : BpGetSpiSlaveResetGpio
*
* Description: This function returns the GPIO pin assignment for the resetting the 
*              the SPI slave.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave reset
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveResetGpio( unsigned short *pusValue )
{
    return( BpGetUs(bp_usGpioSpiSlaveReset, pusValue ) );
} /* BpGetSpiSlaveResetGpio */

/**************************************************************************
* Name       : BpGetSpiSlaveBusNum
*
* Description: This function returns the bus number of the SPI slave device.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave select number
*                    is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveBusNum( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveBusNum, pusValue ) );
} /* BpGetSpiSlaveBusNum */

/**************************************************************************
* Name       : BpGetSpiSlaveSelectNum
*
* Description: This function returns the SPI slave select number connected  
*              to the slave device.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave select number
*                    is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveSelectNum( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveSelectNum, pusValue ) );
} /* BpGetSpiSlaveSelectNum */

/**************************************************************************
* Name       : BpGetSpiSlaveMode
*
* Description: This function returns the SPI slave select number connected  
*              to the slave device.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave mode is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveMode( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveMode, pusValue ) );
} /* BpGetSpiSlaveMode */

/**************************************************************************
* Name       : BpGetSpiSlaveCtrlState
*
* Description: This function returns the spi controller state that is needed to talk
*              to the spi slave device.
*
* Parameters : [OUT] pusValue - Address of long word that the spi controller state is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveCtrlState( unsigned long *pulValue )
{
    return( BpGetUl(bp_ulSpiSlaveCtrlState, pulValue ) );
} /* BpGetSpiSlaveCtrlState */

/**************************************************************************
* Name       : BpGetSpiSlaveMaxFreq
*
* Description: This function returns the SPI slaves max frequency for communication.
*
* Parameters : [OUT] pusValue - Address of long word that the max freq is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveMaxFreq( unsigned long *pulValue )
{
    return( BpGetUl(bp_ulSpiSlaveMaxFreq, pulValue ) );
} /* BpGetSpiSlaveMaxFreq */

/**************************************************************************
* Name       : BpGetSpiSlaveProtoRev
*
* Description: This function returns the protocol revision that the slave device uses.
*
* Parameters : [OUT] pusValue - Address of short word that the spi slave protocol revision
                                is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetSpiSlaveProtoRev( unsigned short *pusValue )
{
    return( BpGetUs(bp_usSpiSlaveProtoRev, pusValue ) );
} /* BpGetSpiSlaveProtoRev */

