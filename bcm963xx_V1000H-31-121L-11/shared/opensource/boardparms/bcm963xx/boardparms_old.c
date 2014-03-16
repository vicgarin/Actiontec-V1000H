/*
    Copyright 2000-2010 Broadcom Corporation

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

        As a special exception, the copyright holders of this software give
        you permission to link this software with independent modules, and to
        copy and distribute the resulting executable under terms of your
        choice, provided that you also meet, for each linked independent
        module, the terms and conditions of the license of that module.
        An independent module is a module which is not derived from this
        software.  The special exception does not apply to any modifications
        of the software.

    Notwithstanding the above, under no circumstances may you combine this
    software in any way with any other Broadcom software provided under a
    license other than the GPL, without Broadcom's express prior written
    consent.
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
#include "boardparms_voice.h"

/* Typedefs */
typedef struct boardparameters
{
    char szBoardId[BP_BOARD_ID_LEN];        /* board id string */
    unsigned short usGPIOOverlay;           /* enabled interfaces */

    unsigned short usGpioRj11InnerPair;     /* GPIO pin or not defined */
    unsigned short usGpioRj11OuterPair;     /* GPIO pin or not defined */
    unsigned short usGpioUartRts;           /* GPIO pin or not defined */
    unsigned short usGpioUartCts;           /* GPIO pin or not defined */

    unsigned short usGpioLedAdsl;           /* GPIO pin or not defined */
    unsigned short usGpioLedAdslFail;       /* GPIO pin or not defined */
    unsigned short usGpioSecLedAdsl;        /* GPIO pin or not defined */
    unsigned short usGpioSecLedAdslFail;    /* GPIO pin or not defined */
    unsigned short usGpioLedSesWireless;    /* GPIO pin or not defined */
    unsigned short usGpioLedHpna;           /* GPIO pin or not defined */
    unsigned short usGpioLedWanData;        /* GPIO pin or not defined */
    unsigned short usGpioLedWanError;       /* GPIO pin or not defined */
    unsigned short usGpioLedBlPowerOn;      /* GPIO pin or not defined */
    unsigned short usGpioLedBlStop;         /* GPIO pin or not defined */
    unsigned short usGpioFpgaReset;         /* GPIO pin or not defined */
    unsigned short usGpioLedGpon;           /* GPIO pin or not defined */
    unsigned short usGpioLedGponFail;       /* GPIO pin or not defined */

    unsigned short usGpioLedMoCA;           /* GPIO pin or not defined */
    unsigned short usGpioLedMoCAFail;       /* GPIO pin or not defined */

    unsigned short usExtIntrResetToDefault; /* ext intr or not defined */
    unsigned short usExtIntrSesBtnWireless; /* ext intr or not defined */
    unsigned short usExtIntrHpna;           /* ext intr or not defined */

    unsigned short usCsHpna;                /* HPNA chip select or not defined */

    unsigned short usAntInUseWireless;      /* antenna in use or not defined */
    unsigned short usWirelessFlags;         /* WLAN flags */

    ETHERNET_MAC_INFO EnetMacInfos[BP_MAX_ENET_MACS];
    VOIP_DSP_INFO VoIPDspInfo[BP_MAX_VOIP_DSP];
    unsigned short usGpioWirelessPowerDown; /* WLAN_PD control or not defined */
    unsigned long ulAfeIds[2];              /* DSL AFE Ids */
    unsigned short usGpioExtAFEReset;       /* GPIO pin or not defined */
    unsigned short usGpioExtAFELDPwr;       /* GPIO pin or not defined */
    unsigned short usGpioExtAFELDMode;      /* GPIO pin or not defined */
    unsigned short usGpioGponAeDisable;     /* GPIO pin or not defined */
    unsigned short usGpioLaserEnable;       /* GPIO pin or not defined */
} BOARD_PARAMETERS, *PBOARD_PARAMETERS;

#define SW_INFO_DEFAULT(n) { \
    (0x00000001 << (n)) - 1,      /* port_map */ \
{0, 0, 0, 0, 0, 0, 0, 0}      /* phy_id */ \
}

/* Variables */
#if defined(_BCM96362_) || defined(CONFIG_BCM96362)

static BOARD_PARAMETERS g_bcm96362advnx =
{
    "96362ADVNX",                           /* szBoardId */
    (BP_OVERLAY_USB_LED |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_14_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_WLAN_EXCLUDE_ONBOARD,                /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_9_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */
};

static BOARD_PARAMETERS g_bcm96362advngr =
{
    "96362ADVNgr",                          /* szBoardId */
    (BP_OVERLAY_USB_LED |
    BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_14_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_WLAN_EXCLUDE_ONBOARD,                /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x3f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x19, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_9_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */
};

static BOARD_PARAMETERS g_bcm96362advngr2 =
{
    "96362ADVNgr2",                         /* szBoardId */
    (BP_OVERLAY_USB_LED |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_14_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_WLAN_EXCLUDE_ONBOARD,                /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_9_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_SERIAL_GPIO_11_AL,                   /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */
};

static BOARD_PARAMETERS g_bcm963629epon =
{
    "963629epon",                            /* szBoardId */
    (BP_OVERLAY_USB_LED |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_WLAN_EXCLUDE_ONBOARD,                /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18 | BCM_WAN_PORT, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_9_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT,                        /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96362advn2xh =
{
    "96362ADVN2xh",                          /* szBoardId */
    (BP_OVERLAY_USB_LED |
    BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_14_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_WLAN_EXCLUDE_ONBOARD,                /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x3f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x19, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_9_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96361XF =
{
    "96361XF",                              /* szBoardId */
    (BP_OVERLAY_USB_LED |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_14_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_WLAN_EXCLUDE_ONBOARD,                /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, MII_DIRECT, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_9_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6302_REV_5_2_2,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED                          /* usGpioExtAFELDMode */
};


static PBOARD_PARAMETERS g_BoardParms[] = {&g_bcm96362advnx, &g_bcm96362advngr, &g_bcm96362advngr2, &g_bcm963629epon, &g_bcm96362advn2xh, &g_bcm96361XF, 0};
#endif

#if defined(_BCM96368_) || defined(CONFIG_BCM96368)

static BOARD_PARAMETERS g_bcm96368vvw =
{
    "96368VVW",                             /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_24_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_33_AH,                          /* usGpioLedWanError */
    BP_GPIO_0_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_1_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_GPIO_25_AL,                          /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_GPIO_26_AL,                          /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_GPIO_27_AL,                          /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368vvwb =
{
    "96368VVWB",                            /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_24_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_33_AH,                          /* usGpioLedWanError */
    BP_GPIO_0_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_1_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_GPIO_25_AL,                          /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_GPIO_26_AL,                          /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_GPIO_27_AL,                          /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXB| BP_AFE_FE_REV_ISIL_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368ntr =
{
    "96368NTR",                             /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_33_AH,                          /* usGpioLedWanError */
    BP_GPIO_0_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_1_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_GPIO_25_AL,                          /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_GPIO_26_AL,                          /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_GPIO_27_AL,                          /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT,                        /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368sv2 =
{
    "96368SV2",                             /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_33_AH,                          /* usGpioLedWanError */
    BP_GPIO_30_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_31_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x3f, {0x01, 0x02, 0x03, 0x04, 0x12, 0x11, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_GPIO_25_AL,                          /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_GPIO_26_AL,                          /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_GPIO_27_AL,                          /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mvwg =
{
    "96368MVWG",                            /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_22_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x36, {0x00, 0x02, 0x03, 0x00, 0x12, 0x11, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mvwgb =
{
    "96368MVWGB",                           /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_22_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x36, {0x00, 0x02, 0x03, 0x00, 0x12, 0x11, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXB| BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mvwgj =
{
    "96368MVWGJ",                           /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_22_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x36, {0x00, 0x02, 0x03, 0x00, 0x12, 0x11, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXJ | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mbg =
{
    "96368MBG",                             /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_GPIO_27_AL,                          /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1}, /* ulAfeIds */
    BP_GPIO_35_AL,                          /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mbg6b =
{
    "96368MBG6b",                           /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_GPIO_27_AL,                          /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1, /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_GPIO_35_AL,                          /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mbg6302 =
{
    "96368MBG6302",                         /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_GPIO_27_AL,                          /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_CHIP_6306 | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1}, /* ulAfeIds */
    BP_GPIO_35_AL,                          /* usGpioExtAFEReset */
    BP_GPIO_37_AH,                          /* usGpioExtAFELDPwr */
    BP_GPIO_36_AH,                          /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */
    
};

static BOARD_PARAMETERS g_bcm96368mng =
{
    "96368MNG",                             /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED),                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    BP_NOT_DEFINED,                         /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x20, {0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_EXTERNAL_SWITCH,               /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_SPI_SSB_1,               /* usConfigType */
    {0x0f, {0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}}}, /* sw */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_6306 | BP_AFE_LD_ISIL1556 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_ISIL_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_GPIO_37_AL,                          /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96367avng =
{
    "96367AVNG",                            /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_23_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_GPIO_31_AH,                          /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x2f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x11, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mvngr =
{
    "96368MVNgr",                           /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_23_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_3_AH,                           /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368rmvng_nand =
{
    "96368RMVNg NAND",                      /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_23_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_3_AH,                           /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x03, {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368rmvng_nor =
{
    "96368RMVNg NOR",                       /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_23_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AH,                           /* usGpioLedWanData */
    BP_GPIO_3_AH,                           /* usGpioLedWanError */
    BP_GPIO_22_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x13, {0x01, 0x02, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96368mvngrP2 =
{
    "96368MVNgrP2",                           /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_PHY |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_INET_LED |
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_2_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_23_AL,                          /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_5_AL,                           /* usGpioLedWanData */
    BP_GPIO_3_AL,                           /* usGpioLedWanError */
    BP_GPIO_22_AL,                          /* usGpioLedBlPowerOn */
    BP_GPIO_24_AL,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},                        /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static PBOARD_PARAMETERS g_BoardParms[] =
{&g_bcm96368vvw, &g_bcm96368mvwg, &g_bcm96368sv2, &g_bcm96368mbg,
&g_bcm96368ntr, &g_bcm96368mbg6b, &g_bcm96368vvwb, &g_bcm96368mvwgb,
&g_bcm96368mng, &g_bcm96368mbg6302, &g_bcm96368mvwgj, &g_bcm96367avng,
&g_bcm96368mvngr, &g_bcm96368rmvng_nand, &g_bcm96368rmvng_nor, &g_bcm96368mvngrP2, 0};
#endif

#if defined(_BCM96816_) || defined(CONFIG_BCM96816)

static BOARD_PARAMETERS g_bcm96816sv =
{
    "96816SV",                              /* szBoardId */
    (BP_OVERLAY_PCI |
    BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_MOCA_LED ),                  /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AH,                           /* usGpioLedGpon */
    BP_GPIO_16_AH,                          /* usGpioLedGponFail */
    BP_GPIO_5_AH,                           /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0xbf, {0x00, 0x01, 0x14, 0x12, 0x18, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT,                        /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96816pvwm =
{
    "96816PVWM",                            /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_MOCA_LED),                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AH,                           /* usGpioLedGpon */
    BP_GPIO_2_AH,                           /* usGpioLedGponFail */
    BP_GPIO_5_AH,                           /* usGpioLedMoCA */
    BP_GPIO_37_AH,                          /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0xbf, {0x00, 0x01, 0x11, 0x12, 0xff, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT,                        /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96818rg =
{
    "96818RG",                              /* szBoardId */
    (
    BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_SERIAL_LEDS),                /* usGPIOOverlay */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_11_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_SERIAL_GPIO_12_AH,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_0_AH,                    /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_1_AH,                    /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AH,                           /* usGpioLedGpon */
    BP_SERIAL_GPIO_6_AH,                    /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */
    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */
    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x9f, {BP_PHY_ID_0, 
            BP_PHY_ID_1, 
            PHYCFG_VALID | PHY_EXTERNAL | BP_PHY_ID_24, 
            PHYCFG_VALID | PHY_EXTERNAL | BP_PHY_ID_25, 
            MAC_PHY_IF_GMII_MII | ADVERTISE_ALL_MII| PHYCFG_VALID | PHY_EXTERNAL | MII_OVER_GPIO_VALID | BP_PHY_ID_16, 
            BP_PHY_ID_NOT_SPECIFIED, 
            BP_PHY_NOT_PRESENT, 
            BP_PHY_ID_NOT_SPECIFIED},
    {{BP_NOT_DEFINED, BP_SERIAL_GPIO_2_AL, BP_SERIAL_GPIO_3_AL },
    {BP_NOT_DEFINED, BP_SERIAL_GPIO_4_AL, BP_SERIAL_GPIO_5_AL} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT,                        /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_GPIO_0_AL,                           /* usGpioGponAeDisable*/
    BP_GPIO_5_AL                            /* usGpioLaserEnable   */
  
};

static BOARD_PARAMETERS g_bcm968128rg =
{
    "968128RG",                             /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0),                /* usGPIOOverlay */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */
    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_18_AH,                          /* usGpioLedBlPowerOn */
    BP_GPIO_19_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AL,                           /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */
    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */
    BP_NOT_DEFINED,                         /* usCsHpna */
    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */
    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x81, {BP_PHY_ID_0, 
             BP_PHY_NOT_PRESENT, 
             BP_PHY_NOT_PRESENT, 
             BP_PHY_NOT_PRESENT, 
             BP_PHY_NOT_PRESENT, 
             BP_PHY_NOT_PRESENT, 
             BP_PHY_NOT_PRESENT, 
             BP_PHY_ID_NOT_SPECIFIED},
             {{BP_NOT_DEFINED, BP_GPIO_29_AL, BP_GPIO_26_AL},
             {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT,                        /* ulAfeIds */
    BP_AFE_DEFAULT},                    
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_GPIO_11_AL,                          /* usGpioGponAeDisable*/
    BP_GPIO_39_AH                           /* usGpioLaserEnable */
  
};

static BOARD_PARAMETERS g_bcm96829rg =
{
    "96829RG",                              /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_MOCA_LED),                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_GPIO_5_AL,                           /* usGpioLedMoCA */
    BP_GPIO_37_AH,                          /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0xa3, {0x00, 0x01, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96816p2og =
{
    "96816P2OG",                            /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_PCI |
    BP_OVERLAY_MOCA_LED),                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_GPIO_2_AH,                           /* usGpioFpgaReset */
    BP_GPIO_8_AH,                           /* usGpioLedGpon */
    BP_GPIO_16_AH,                          /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0xaf, {0x00, 0x01, 0x11, 0xff, 0x00, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96812pg =
{
    "96812PG"  ,                            /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1),                 /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AH,                           /* usGpioLedGpon */
    BP_GPIO_2_AH,                           /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0xaf, {0x00, 0x01, 0x11, 0x12, 0x00, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96819bhr =
{
    "96819BHR",                              /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_MOCA_LED),                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_GPIO_5_AL,                           /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_3,                          /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x3f, {0x00, 0x01, 0x14, 0x12, 0xA1, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_GPIO_13_AL, BP_GPIO_38_AL},
         {BP_NOT_DEFINED, BP_GPIO_30_AL, BP_GPIO_31_AL} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm968121pg =
{
    "968121PG"  ,                            /* szBoardId */
    BP_OVERLAY_GPHY_LED_0,                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AL,                           /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x95, {0x00, 0x00, 0x18, 0x00, 0xff, 0x00, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm968121p =
{
    "968121P"  ,                            /* szBoardId */
    BP_OVERLAY_GPHY_LED_0,                  /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_24_AH,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_GPIO_8_AL,                           /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x91, {0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96819bhrgr =
{
    "96819BHRGR",                           /* szBoardId */
    (BP_OVERLAY_GPHY_LED_0 |
    BP_OVERLAY_GPHY_LED_1 |
    BP_OVERLAY_MOCA_LED),                   /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_GPIO_3_AH,                           /* usGpioLedBlPowerOn */
    BP_GPIO_4_AH,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_GPIO_5_AL,                           /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_3,                          /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_NOT_DEFINED,                         /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x3f, {0x00, 0x01, 0x18, 0x19, 0xA1, 0xff, 0x00, 0xff},
        {{BP_NOT_DEFINED, BP_GPIO_13_AL, BP_GPIO_38_AL },
         {BP_NOT_DEFINED, BP_GPIO_2_AL, BP_GPIO_31_AL} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_DEFAULT, BP_AFE_DEFAULT},       /* ulAfeIds */
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static PBOARD_PARAMETERS g_BoardParms[] = {&g_bcm96816sv, &g_bcm96816pvwm,
  	&g_bcm96829rg, &g_bcm96816p2og, &g_bcm96812pg, &g_bcm96819bhr,
	&g_bcm968121pg, &g_bcm968121p, &g_bcm96819bhrgr, &g_bcm96818rg, &g_bcm968128rg, 0};


#endif

#if defined(_BCM96328_) || defined(CONFIG_BCM96328)
static BOARD_PARAMETERS g_bcm96328avng =
{
    "96328avng",                            /* szBoardId */
    (BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_3_AL,                           /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_9_AL,                           /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    // **** Set to BP_GPIO_0_AL when WanData LED is connected to GPIO11
    BP_GPIO_0_AL,                           /* usGpioLedWanData */
    BP_GPIO_2_AL,                           /* usGpioLedWanError */
    BP_GPIO_4_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_8_AL,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_GPIO_6_AL,                           /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_GPIO_7_AL,                           /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_GPIO_5_AL,                           /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96328avngrP1 =
{
    "96328avngrP1",                         /* szBoardId */
    (BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_11_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_SERIAL_GPIO_1_AL,                    /* usGpioLedWanData */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_14_AL,                   /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm96328avngr =
{
    "96328avngr",                         /* szBoardId */
    (BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_10_AL,                   /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_11_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_SERIAL_GPIO_13_AL,                   /* usGpioLedWanError */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_14_AL,                   /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm963281TAN =
{
    "963281TAN",                            /* szBoardId */
    (BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_GPIO_11_AL,                          /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_GPIO_9_AL,                           /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_GPIO_1_AL,                           /* usGpioLedWanData */
    BP_GPIO_7_AL,                           /* usGpioLedWanError */
    BP_GPIO_4_AL,                           /* usGpioLedBlPowerOn */
    BP_GPIO_8_AL,                           /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_NO_DSP},                      /* ucDspType */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

#define MII_DIRECT  (PHYCFG_VALID | MAC_MAC_IF | FORCE_LINK_100FD)

static BOARD_PARAMETERS g_bcm963293epon =
{
    "963293epon",                           /* szBoardId */
    (BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_11_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, MII_DIRECT, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_14_AL,                   /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm963293epon_gphy =
{
    "963293epon_gphy",                      /* szBoardId */
    (BP_OVERLAY_SPI_EXT_CS |
    BP_OVERLAY_EPHY_LED_0 |
    BP_OVERLAY_EPHY_LED_1 |
    BP_OVERLAY_EPHY_LED_2 |
    BP_OVERLAY_EPHY_LED_3 |
    BP_OVERLAY_SERIAL_LEDS |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */

    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_11_AL,                   /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioLedBlPowerOn */
    BP_SERIAL_GPIO_15_AL,                   /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x1f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_14_AL,                   /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_12_AL,                   /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6301 | BP_AFE_FE_ANNEXA | BP_AFE_FE_AVMODE_ADSL | BP_AFE_FE_REV_6301_REV_5_1_1,    /* ulAfeIds */
    BP_AFE_DEFAULT},
    BP_NOT_DEFINED,                         /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};


static PBOARD_PARAMETERS g_BoardParms[] = {&g_bcm96328avng, &g_bcm96328avngrP1, &g_bcm96328avngr, &g_bcm963281TAN, &g_bcm963293epon, &g_bcm963293epon_gphy, 0};
#endif

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)
static BOARD_PARAMETERS g_bcm963268sv1 =
{
    "963268SV1",                            /* szBoardId */
    (BP_OVERLAY_PHY |
    BP_OVERLAY_SERIAL_LEDS | 
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_NOT_DEFINED,                         /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_NOT_DEFINED,                         /* usGpioLedWanData */
    BP_NOT_DEFINED,                         /* usGpioLedWanError */
    BP_NOT_DEFINED,                         /* usGpioLedBlPowerOn */
    BP_NOT_DEFINED,                         /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_NOT_DEFINED,                         /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_NOT_DEFINED,                         /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_NOT_DEFINED,                         /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},
    BP_GPIO_17_AL,                          /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm963268mbv =
{
    "963268MBV",                            /* szBoardId */
    (BP_OVERLAY_PHY |
    BP_OVERLAY_SERIAL_LEDS | 
    BP_OVERLAY_USB_DEVICE),                 /* usGPIOOverlay */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_3_AL,                    /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioLedWanData */
    BP_SERIAL_GPIO_2_AL,                    /* usGpioLedWanError */
    BP_GPIO_20_AL,                          /* usGpioLedBlPowerOn */
    BP_GPIO_21_AL,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_NOT_DEFINED,                         /* usExtIntrResetToDefault */
    BP_NOT_DEFINED,                         /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x5f, {0x01, 0x02, 0x03, 0x04, 0x18, 0x00, 0x19, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_4_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_5_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_SERIAL_GPIO_6_AL,                    /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},
    BP_GPIO_17_AL,                          /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static BOARD_PARAMETERS g_bcm963168vx =
{
    "963168VX",                             /* szBoardId */
    (BP_OVERLAY_PHY |
    BP_OVERLAY_SERIAL_LEDS | 
    BP_OVERLAY_USB_DEVICE |
    BP_OVERLAY_PCIE_CLKREQ),                /* usGPIOOverlay */
    BP_NOT_DEFINED,                         /* usGpioRj11InnerPair */
    BP_NOT_DEFINED,                         /* usGpioRj11OuterPair */
    BP_NOT_DEFINED,                         /* usGpioUartRts */
    BP_NOT_DEFINED,                         /* usGpioUartCts */

    BP_SERIAL_GPIO_3_AL,                    /* usGpioLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioLedAdslFail */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdsl */
    BP_NOT_DEFINED,                         /* usGpioSecLedAdslFail */
    BP_SERIAL_GPIO_7_AL,                    /* usGpioLedSesWireless */
    BP_NOT_DEFINED,                         /* usGpioLedHpna */
    BP_SERIAL_GPIO_8_AL,                    /* usGpioLedWanData */
    BP_SERIAL_GPIO_2_AL,                    /* usGpioLedWanError */
    BP_GPIO_20_AL,                          /* usGpioLedBlPowerOn */
    BP_GPIO_21_AL,                          /* usGpioLedBlStop */
    BP_NOT_DEFINED,                         /* usGpioFpgaReset */
    BP_NOT_DEFINED,                         /* usGpioLedGpon */
    BP_NOT_DEFINED,                         /* usGpioLedGponFail */
    BP_NOT_DEFINED,                         /* usGpioLedMoCA */
    BP_NOT_DEFINED,                         /* usGpioLedMoCAFail */

    BP_EXT_INTR_0,                          /* usExtIntrResetToDefault */
    BP_EXT_INTR_1,                          /* usExtIntrSesBtnWireless */
    BP_NOT_DEFINED,                         /* usExtIntrHpna */

    BP_NOT_DEFINED,                         /* usCsHpna */

    BP_WLAN_ANT_MAIN,                       /* usAntInUseWireless */
    0,                                      /* WLAN flags */

    {{BP_ENET_EXTERNAL_SWITCH,              /* ucPhyType */
    0x0,                                    /* ucPhyAddress */
    BP_ENET_CONFIG_MMAP,                    /* usConfigType */
    {0x0f, {0x01, 0x02, 0x03, 0x04, 0x10, 0x00, 0x10, 0x00},
        {{BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED},
         {BP_NOT_DEFINED, BP_NOT_DEFINED, BP_NOT_DEFINED} }}},  /* sw */
    {BP_ENET_NO_PHY}},                      /* ucPhyType */
    {{BP_VOIP_MIPS,                         /* ucDspType */
    0,                                      /* ucDspAddress */
    BP_NOT_DEFINED,                         /* usGpioLedVoip */
    BP_SERIAL_GPIO_4_AL,                    /* usGpioVoip1Led */
    BP_NOT_DEFINED,                         /* usGpioVoip1LedFail */
    BP_SERIAL_GPIO_5_AL,                    /* usGpioVoip2Led */
    BP_NOT_DEFINED,                         /* usGpioVoip2LedFail */
    BP_SERIAL_GPIO_6_AL,                    /* usGpioPotsLed */
    BP_NOT_DEFINED},                        /* usGpioDectLed */
    {BP_VOIP_NO_DSP}},                      /* ucDspType */
    BP_NOT_DEFINED,                         /* usGpioWirelessPowerDown */
    {BP_AFE_CHIP_INT | BP_AFE_LD_6302 | BP_AFE_FE_ANNEXA | BP_AFE_FE_REV_6302_REV1,
    BP_AFE_DEFAULT},
    BP_GPIO_17_AL,                          /* usGpioExtAFEReset */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDPwr */
    BP_NOT_DEFINED,                         /* usGpioExtAFELDMode */
    BP_NOT_DEFINED,                         /* usGpioGponAeDisable*/
    BP_NOT_DEFINED                          /* usGpioLaserEnable   */

};

static PBOARD_PARAMETERS g_BoardParms[] = {&g_bcm963268sv1, &g_bcm963268mbv,
    &g_bcm963168vx, 0};
#endif

static PBOARD_PARAMETERS g_pCurrentBp = 0;

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
    while (*src && *dest)
    {
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
* Name       : BpGetVoipDspConfig
*
* Description: Gets the DSP configuration from the board parameter
*              structure for a given DSP index.
*
* Parameters : [IN] dspNum - DSP index (number)
*
* Returns    : Pointer to DSP configuration block if found/valid, NULL
*              otherwise.
***************************************************************************/
VOIP_DSP_INFO *BpGetVoipDspConfig( unsigned char dspNum )
{
    VOIP_DSP_INFO *pDspConfig = 0;
    int i;

    if( g_pCurrentBp )
    {
        for( i = 0 ; i < BP_MAX_VOIP_DSP ; i++ )
        {
            if( g_pCurrentBp->VoIPDspInfo[i].ucDspType != BP_VOIP_NO_DSP &&
                g_pCurrentBp->VoIPDspInfo[i].ucDspAddress == dspNum )
            {
                pDspConfig = &g_pCurrentBp->VoIPDspInfo[i];
                break;
            }
        }
    }

    return pDspConfig;
}


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
    PBOARD_PARAMETERS *ppBp;

    for( ppBp = g_BoardParms; *ppBp; ppBp++ )
    {
        if( !bpstrcmp((*ppBp)->szBoardId, pszBoardId) )
        {
            g_pCurrentBp = *ppBp;
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

    if (g_pCurrentBp == 0)
        return -1;

    for (i = 0; i < BP_BOARD_ID_LEN; i++)
        pszBoardId[i] = g_pCurrentBp->szBoardId[i];

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
    PBOARD_PARAMETERS *ppBp;
    int i;
    char *src;
    char *dest;

    for( i = 0, ppBp = g_BoardParms; *ppBp && nBoardIdsSize;
        i++, ppBp++, nBoardIdsSize--, pszBoardIds += BP_BOARD_ID_LEN )
    {
        dest = pszBoardIds;
        src = (*ppBp)->szBoardId;
        while( *src )
            *dest++ = *src++;
        *dest = '\0';
    }

    return( i );
} /* BpGetBoardIds */

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
int BpGetGPIOverlays( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGPIOOverlay;

        if( g_pCurrentBp->usGPIOOverlay != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int i, nRet;

    if( g_pCurrentBp )
    {
        for( i = 0; i < nNumEnetInfos; i++, pEnetInfos++ )
        {
            if( i < BP_MAX_ENET_MACS )
            {
int Index;
            
            
                unsigned char *src = (unsigned char *)
                    &g_pCurrentBp->EnetMacInfos[i];
                unsigned char *dest = (unsigned char *) pEnetInfos;
                int len = sizeof(ETHERNET_MAC_INFO);
                while( len-- )
                    *dest++ = *src++;                                   
                for  (Index=0; Index<BP_MAX_SWITCH_PORTS; Index++)
                {   // clear MII over GPIO flag from PHY ID space
                    pEnetInfos->sw.phy_id[Index] &= ~MII_OVER_GPIO_VALID;
                }    
            }
            else
                pEnetInfos->ucPhyType = BP_ENET_NO_PHY;
        }

        nRet = BP_SUCCESS;
    }
    else
    {
        for( i = 0; i < nNumEnetInfos; i++, pEnetInfos++ )
            pEnetInfos->ucPhyType = BP_ENET_NO_PHY;

        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetEthernetMacInfo */

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
int
    nRet;
unsigned long
    Index1, 
    Index2;
   

    *pMiiOverGpioFlag = 0;

    if( g_pCurrentBp )
    {
        for( Index1 = 0; Index1 < BP_MAX_ENET_MACS; Index1++)
        {
            if (g_pCurrentBp->EnetMacInfos[Index1].ucPhyType != BP_ENET_NO_PHY)
            {
                for( Index2 = 0; Index2 < BP_MAX_SWITCH_PORTS; Index2++)
                {
                    *pMiiOverGpioFlag |= g_pCurrentBp->EnetMacInfos[Index1].sw.phy_id[Index2] & MII_OVER_GPIO_VALID;                
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusInner = g_pCurrentBp->usGpioRj11InnerPair;
        *pusOuter = g_pCurrentBp->usGpioRj11OuterPair;

        if( g_pCurrentBp->usGpioRj11InnerPair != BP_NOT_DEFINED &&
            g_pCurrentBp->usGpioRj11OuterPair != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusInner = *pusOuter = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusRts = g_pCurrentBp->usGpioUartRts;
        *pusCts = g_pCurrentBp->usGpioUartCts;

        if( g_pCurrentBp->usGpioUartRts != BP_NOT_DEFINED &&
            g_pCurrentBp->usGpioUartCts != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusRts = *pusCts = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedAdsl;

        if( g_pCurrentBp->usGpioLedAdsl != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedAdslFail;

        if( g_pCurrentBp->usGpioLedAdslFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioSecLedAdsl;

        if( g_pCurrentBp->usGpioSecLedAdsl != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioSecLedAdslFail;

        if( g_pCurrentBp->usGpioSecLedAdslFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usAntInUseWireless;

        if( g_pCurrentBp->usAntInUseWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usWirelessFlags;

        if( g_pCurrentBp->usWirelessFlags != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usExtIntrSesBtnWireless;

        if( g_pCurrentBp->usExtIntrSesBtnWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedSesWireless;

        if( g_pCurrentBp->usGpioLedSesWireless != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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

} /* BpGetWirelessSesLedGpio */

/* this data structure could be moved to boardparams structure in the future */
/* does not require to rebuild cfe here if more srom entries are needed */
static WLAN_SROM_PATCH_INFO wlanPaInfo[]={
#if defined(_BCM96362_) || defined(CONFIG_BCM96362)
    /* this is the patch to srom map for 96362ADVNX */
    {"96362ADVNX",    0x6362, 220,
    {{"boardrev", 65, 0x1100},
    {"fem2g", 87, 0x0319},
    {"ittmaxp2ga0", 96, 0x2058},
    {"pa2gw0a0", 97, 0xfe6f},
    {"pa2gw1a0", 98, 0x1785},
    {"pa2gw2a0", 99, 0xfa21},
    {"ittmaxp2ga1", 112, 0x2058},
    {"pa2gw0a0", 113, 0xfe77},
    {"pa2gw1a0", 114, 0x17e0},
    {"pa2gw2a0", 115, 0xfa16},
    {"ofdm2gpo0", 161, 0x5555},
    {"ofdm2gpo1", 162, 0x5555},
    {"mcs2gpo0", 169, 0x5555},
    {"mcs2gpo1", 170, 0x5555},
    {"mcs2gpo2", 171, 0x5555},
    {"mcs2gpo3", 172, 0x5555},
    {"mcs2gpo4", 173, 0x3333},
    {"mcs2gpo5", 174, 0x3333},
    {"mcs2gpo6", 175, 0x3333},
    {"mcs2gpo7", 176, 0x3333},
    {"",       0,      0}}},

    /* this is the patch to srom map for 6362ADVNgr2 */
    {"96362ADVNgr2",  0x6362, 220, 
    {{"boardrev",   65, 0x1431},    	
    {"ittmaxp2ga0", 96, 0x2048},
    {"pa2gw0a0",    97, 0xfe99},
    {"pa2gw1a0",    98, 0x1397},
    {"pa2gw2a0",    99, 0xfaf7},
    {"iit5ga0",    100, 0x3e40},
    {"maxp5gla0",  101, 0x4040},     
    {"ittmaxp2ga1",112, 0x2048},
    {"pa2gw0a0",   113, 0xfea2},
    {"pa2gw1a0",   114, 0x13c3},
    {"pa2gw2a0",   115, 0xfaf2},
    {"iit5ga1",    116, 0x3e40},  
    {"maxp5gla1",  117, 0x4040},
    {"ofdm2gpo0",  161, 0x3333},
    {"ofdm2gpo1",  162, 0x3333},      
    {"mcs2gpo0",   169, 0x3333},
    {"mcs2gpo1",   170, 0x3333},
    {"mcs2gpo2",   171, 0x3333},
    {"mcs2gpo3",   172, 0x3333},
    {"mcs2gpo4",   173, 0x3333},
    {"mcs2gpo5",   174, 0x3333},
    {"mcs2gpo6",   175, 0x3333},
    {"mcs2gpo7",   176, 0x3333},
    {"",             0,      0}}},
    	
    /* this is the patch to srom map for 96362ADVN2xh */
    {"96362ADVN2xh",  0x6362, 220, 
    {{"boardrev",   65, 0x1100},    	    	
    {"boardflagsw0",66, 0x3200},
    {"boardflagsw1",67, 0x8000},
    {"aa5gaa2g",    78, 0x0003},
    {"ag1ag0",      79, 0x0000},
    {"ag3ag2",      80, 0x0000},
    {"fem2g",       87, 0x031f},
    {"fem5g",       88, 0x031f},
    {"elna5gelna2g",93, 0x7800},
    {"ittmaxp2ga0", 96, 0x2074},
    {"pa2gw0a0",    97, 0xfe80},
    {"pa2gw1a0",    98, 0x1fa2},
    {"pa2gw2a0",    99, 0xf897},
    {"ittmaxp2ga1",112, 0x2074}, 
    {"pa2gw0a0",   113, 0xfe8c},
    {"pa2gw1a0",   114, 0x1f13},
    {"pa2gw2a0",   115, 0xf8c4},
    {"ofdm2gpo0",  161, 0x8880},
    {"ofdm2gpo1",  162, 0x8888},
    {"mcs2gpo0",   169, 0x8880},
    {"mcs2gpo1",   170, 0x8888},
    {"mcs2gpo2",   171, 0x8880},
    {"mcs2gpo3",   172, 0x8888},
    {"mcs2gpo4",   173, 0x8880},
    {"mcs2gpo5",   174, 0x8888},
    {"mcs2gpo6",   175, 0x8880},
    {"mcs2gpo7",   176, 0x8888},
    {"",             0,      0}}},
#endif

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)
    /* this is the patch to srom map for 63268MBV */
    {"963268MBV",   0x6362, 220, 
    {{"boardrev",   65, 0x1230},
    {"aa5gaa2g",    78, 0x0303}, 
    {"ag1ag0",      79, 0x0202}, 
    {"ag3ag2",      80, 0xff02},
    {"fem2g",       87, 0x0315}, 
    {"fem5g",       88, 0x0315},
    {"ittmaxp2ga0", 96, 0x2038}, 
    {"pa2gw0a0",    97, 0xFFB5}, 
    {"pa2gw1a0",    98, 0x175F},
    {"pa2gw2a0",    99, 0xFB29},
    {"ittmaxp2ga1", 112,0x2038}, 
    {"pa2gw0a0",    113,0xFFD7},
    {"pa2gw1a0",    114,0x17D6},
    {"pa2gw2a0",    115,0xFB67},
    {"",            0,       0}}},
#endif
  	
    {"",                 0, 0, {{"",       0,      0}}}, /* last entry*/
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
        if ( !bpstrcmp(g_pCurrentBp->szBoardId, wlanPaInfo[i].szboardId) ) {
            /* check chipId */
            if ( (wlanPaInfo[i].usWirelessChipId == chipID) && (wlanPaInfo[i].usNeededSize <= sizeInWords) ){
                /* valid , patch entry */
                while ( wlanPaInfo[i].entries[j].name[0] ) {
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
        if ( !bpstrcmp(g_pCurrentBp->szBoardId, wlanPciInfo[i].szboardId) ) {
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

/**************************************************************************
* Name       : BpGetHpnaLedGpio
*
* Description: This function returns the GPIO pin assignment for the HPNA
*              LED.
*
* Parameters : [OUT] pusValue - Address of short word that the HPNA LED
*                  GPIO pin is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetHpnaLedGpio( unsigned short *pusValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedHpna;

        if( g_pCurrentBp->usGpioLedHpna != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
} /* BpGetHpnaLedGpio */

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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedWanData;

        if( g_pCurrentBp->usGpioLedWanData != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedWanError;

        if( g_pCurrentBp->usGpioLedWanError != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedBlPowerOn;

        if( g_pCurrentBp->usGpioLedBlPowerOn != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedBlStop;

        if( g_pCurrentBp->usGpioLedBlStop != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = BpGetVoipDspConfig( 0 );

        if( pDspInfo )
        {
            *pusValue = pDspInfo->usGpioLedVoip;

            if( *pusValue != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_BOARD_ID_NOT_FOUND;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = &g_pCurrentBp->VoIPDspInfo[0];

        if( pDspInfo->ucDspType != BP_VOIP_NO_DSP)
        {
            *pusValue = pDspInfo->usGpioVoip1Led;

            if( pDspInfo->usGpioVoip1Led != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = &g_pCurrentBp->VoIPDspInfo[0];

        if( pDspInfo->ucDspType != BP_VOIP_NO_DSP)
        {
            *pusValue = pDspInfo->usGpioVoip1LedFail;

            if( pDspInfo->usGpioVoip1LedFail != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = &g_pCurrentBp->VoIPDspInfo[0];

        if( pDspInfo->ucDspType != BP_VOIP_NO_DSP)
        {
            *pusValue = pDspInfo->usGpioVoip2Led;

            if( pDspInfo->usGpioVoip2Led != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = &g_pCurrentBp->VoIPDspInfo[0];

        if( pDspInfo->ucDspType != BP_VOIP_NO_DSP)
        {
            *pusValue = pDspInfo->usGpioVoip2LedFail;

            if( pDspInfo->usGpioVoip2LedFail != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = &g_pCurrentBp->VoIPDspInfo[0];

        if( pDspInfo->ucDspType != BP_VOIP_NO_DSP)
        {
            *pusValue = pDspInfo->usGpioPotsLed;

            if( pDspInfo->usGpioPotsLed != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
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
    int nRet;

    if( g_pCurrentBp )
    {
        VOIP_DSP_INFO *pDspInfo = &g_pCurrentBp->VoIPDspInfo[0];

        if( pDspInfo->ucDspType != BP_VOIP_NO_DSP)
        {
            *pusValue = pDspInfo->usGpioDectLed;

            if( pDspInfo->usGpioDectLed != BP_NOT_DEFINED )
            {
                nRet = BP_SUCCESS;
            }
            else
            {
                nRet = BP_VALUE_NOT_DEFINED;
            }
        }
        else
        {
            *pusValue = BP_NOT_DEFINED;
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetDectLedGpio */

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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioFpgaReset;

        if( g_pCurrentBp->usGpioFpgaReset != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedGpon;

        if( g_pCurrentBp->usGpioLedGpon != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedGponFail;

        if( g_pCurrentBp->usGpioLedGponFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedMoCA;

        if( g_pCurrentBp->usGpioLedMoCA != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioLedMoCAFail;

        if( g_pCurrentBp->usGpioLedMoCAFail != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
} /* BpGetMoCAFailLedGpio */

/**************************************************************************
* Name       : BpGetHpnaExtIntr
*
* Description: This function returns the HPNA external interrupt number.
*
* Parameters : [OUT] pulValue - Address of short word that the HPNA
*                  external interrupt number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetHpnaExtIntr( unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulValue = g_pCurrentBp->usExtIntrHpna;

        if( g_pCurrentBp->usExtIntrHpna != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetHpnaExtIntr */

/**************************************************************************
* Name       : BpGetHpnaChipSelect
*
* Description: This function returns the HPNA chip select number.
*
* Parameters : [OUT] pulValue - Address of short word that the HPNA
*                  chip select number is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_SET - Error, BpSetBoardId has not been called.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/
int BpGetHpnaChipSelect( unsigned long *pulValue )
{
    int nRet;

    if( g_pCurrentBp )
    {
        *pulValue = g_pCurrentBp->usCsHpna;

        if( g_pCurrentBp->usCsHpna != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
        {
            nRet = BP_VALUE_NOT_DEFINED;
        }
    }
    else
    {
        *pulValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetHpnaChipSelect */

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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usExtIntrResetToDefault;

        if( g_pCurrentBp->usExtIntrResetToDefault != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioWirelessPowerDown;

        if( g_pCurrentBp->usGpioWirelessPowerDown != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
        pulValues[0] = g_pCurrentBp->ulAfeIds[0];
        pulValues[1] = g_pCurrentBp->ulAfeIds[1];
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioExtAFEReset;

        if( g_pCurrentBp->usGpioExtAFEReset != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
} /* BpGetExtAFEResetGpio */

/**************************************************************************
* Name       : BpGetExtAFELDModeGpio
*
* Description: This function returns the GPIO pin assignment for setting LD Mode to ADSL/VDSL
*
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioExtAFELDMode;

        if( g_pCurrentBp->usGpioExtAFELDMode != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
    int nRet;

    if( g_pCurrentBp )
    {
        *pusValue = g_pCurrentBp->usGpioExtAFELDPwr;

        if( g_pCurrentBp->usGpioExtAFELDPwr != BP_NOT_DEFINED )
        {
            nRet = BP_SUCCESS;
        }
        else
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
} /* BpGetExtAFELDPwrGpio */


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
   if ( BP_SUCCESS == retVal )
   {
      pSwInfo = &enetMacInfo.sw;
      for (i = 0; i < BP_MAX_SWITCH_PORTS; i++)
      {
         if ( ((pSwInfo->phy_id[i] & PHYID_LSBYTE_M) != 0xFF) &&
              ((pSwInfo->phy_id[i] & PHYID_LSBYTE_M) &  0x80) )
         {
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
    unsigned short *pShort;
    int nRet;

    if( g_pCurrentBp )
    {
        if ((enetIdx >= BP_MAX_ENET_MACS) ||
            (port >= BP_MAX_ENET_INTERNAL) ||
            (g_pCurrentBp->EnetMacInfos[enetIdx].ucPhyType == BP_ENET_NO_PHY))
        {
           *pusValue = BP_NOT_DEFINED;
           nRet = BP_VALUE_NOT_DEFINED;
        }
        else
        {
           pShort   = &g_pCurrentBp->EnetMacInfos[enetIdx].sw.ledInfo[port].duplexLed;
           pShort   += ledIdx;
           *pusValue = *pShort;
           if( *pShort == BP_NOT_DEFINED )
           {
               nRet = BP_VALUE_NOT_DEFINED;
           }
           else
           {
               nRet = BP_SUCCESS;
           }
        }
    }
    else
    {
        *pusValue = BP_NOT_DEFINED;
        nRet = BP_BOARD_ID_NOT_SET;
    }

    return( nRet );
} /* BpGetEthSpdLedGpio */

