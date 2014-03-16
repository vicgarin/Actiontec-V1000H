/*
 Copyright 2004-2010 Broadcom Corp. All Rights Reserved.

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
#ifndef _BCMMII_H_
#define _BCMMII_H_

#include "bcmmii_exports.h"

/****************************************************************************
    Internal PHY registers
****************************************************************************/

//#define BCM_PHY_ID_M                        0x1F
//#define IsExtPhyId(id)                      ((id & BCM_PHY_ID_M) >= 0x10)
#define BCM_EXT_6829                        0x80
#define IsExt6829(id)                       (((id) & BCM_EXT_6829) && ((id & 0xFF) != 0xFF))
//#define isPhyConnected(phy_id)              (phy_id != 0xFF)

#define MII_CONTROL                         0x00
    #define MII_CONTROL_POWER_DOWN          (1 << 11)
    #define MII_CONTROL_ISOLATE_MII         (1 << 10)
    #define MII_CONTROL_RESTART_AUTONEG     (1 << 9)

#define MII_ASR                             0x19
#define MII_INTERRUPT                       0x1a
#define MII_INTERRUPT_MASK                  0x1b
#define MII_AS2SR                           0x1b  // Aux Status 2 Shadow Register (53101)
    #define MII_AS2SR_APD_EN                (1 << 5)
#define MII_REGISTER_1C                     0x1c
    #define MII_1C_WRITE_ENABLE             (1 << 15)
    #define MII_1C_AUTO_POWER_DOWN_SV       (0x0a << 10)
        #define MII_1C_AUTO_POWER_DOWN      (1 << 5)
        #define MII_1C_SLEEP_TIMER_SEL      (1 << 4)
        #define MII_1C_WAKEUP_TIMER_SEL_84  (1 << 0)
#define MII_1C_SHADOW_REG_SEL_S         10
#define MII_1C_SHADOW_REG_SEL_M         0x1F
#define MII_1C_SHADOW_CLK_ALIGN_CTRL        0x3
    #define GTXCLK_DELAY_BYPASS_DISABLE     (1 << 9)
#define MII_1C_SHADOW_LED_CONTROL           0x9
    #define ACT_LINK_LED_ENABLE             (1 << 4)
#define MII_1C_EXTERNAL_CONTROL_1           0xB
    #define LOM_LED_MODE                    (1 << 2)
#define MII_BRCM_TEST                       0x1f
    #define MII_BRCM_TEST_SHADOW_ENABLE     0x0080
    #define MII_BRCM_TEST_SHADOW2_ENABLE    0x0004
#if defined (CONFIG_BCM96816)
#define MII_DSP_COEFF_ADDR                  0x17
#define MII_DSP_COEFF_RW_PORT               0x15
#endif

/* MII Auxiliary Status Register */
/*
 * 0x8000 auto negotiation done
 * 0x0004 link up
 * 0x0700 1000 Mbps, FDX
 * 0x0600 1000 Mbps, HDX
 * 0x0500  100 Mbps, FDX
 * 0x0300  100 Mbps, HDX
 * 0x0200   10 Mpbs, FDX
 * 0x0100   10 Mbps, HDX
 */

#define MII_ASR_ANHCD_MASK 0x0700
#define MII_ASR_DONE(r) ((r & 0x8000) != 0)
#define MII_ASR_LINK(r) ((r & 0x0004) != 0)
#define MII_ASR_FDX(r)  (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0200))
#define MII_ASR_1000(r) (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0600))
#define MII_ASR_100(r)  (((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0300))
#define MII_ASR_10(r)   (((r & 0x0700) == 0x0200) || ((r & 0x0700) == 0x0100))
#define MII_ASR_NOHCD(r) ((r & MII_ASR_ANHCD_MASK) == 0)
#if defined (CONFIG_BCM96368)
#define MII_AENGSR                          0x1c
#define MII_AENGSR_SPD                      0x2 
#define MII_AENGSR_DPLX                     0x1 
#endif

typedef struct {
  int lnk:1;
  int fdx:1;
  int spd1000:1;
  int spd100:1;
} PHY_STAT;

/* MII Interrupt register. */
#define MII_INTR_ENABLE                     0x4000
#define MII_INTR_FDX                        0x0008
#define MII_INTR_SPD                        0x0004
#define MII_INTR_LNK                        0x0002


/****************************************************************************
    External switch pseudo PHY: Page (0x00)
****************************************************************************/

#define PAGE_CONTROL                                      0x00

    #define REG_PORT_CTRL                                 0x00

        #define REG_PORT_NO_SPANNING_TREE                 0x00
        #define REG_PORT_STP_STATE_DISABLED               0x20
        #define REG_PORT_STP_STATE_BLOCKING               0x40
        #define REG_PORT_STP_STATE_LISTENING              0x60
        #define REG_PORT_STP_STATE_LEARNING               0x80
        #define REG_PORT_STP_STATE_FORWARDING             0xA0
        #define REG_PORT_TX_DISABLE                       0x02
        #define REG_PORT_RX_DISABLE                       0x01
        #define REG_PORT_CTRL_DISABLE                     0x03

#if defined (CONFIG_BCM96328)
    #define EPHY_PORTS       5
    #define NUM_RGMII_PORTS  1
    #define RGMII_PORT_ID    4
#elif defined (CONFIG_BCM96362)
    #define EPHY_PORTS       6
    #define NUM_RGMII_PORTS  2
    #define RGMII_PORT_ID    4
#elif defined (CONFIG_BCM963268)
    #define EPHY_PORTS       8
    #define GPHY_PORT_ID     3
    #define GPHY_PORT_PHY_ID 4
    #define NUM_RGMII_PORTS  4
    #define RGMII_PORT_ID    4
#elif defined (CONFIG_BCM96368)
    #define EPHY_PORTS       6
    #define NUM_RGMII_PORTS  2
    #define RGMII_PORT_ID    4
    #define SAR_PORT_ID      7
#elif defined (CONFIG_BCM96816)
    #define EPHY_PORTS       4
    #define NUM_RGMII_PORTS  2
    #define SERDES_PORT_ID   4
    #define MOCA_PORT_ID     5
    #define GPON_PORT_ID     7
    #define RGMII_PORT_ID    2
#endif

#define USB_PORT_ID     6
#define MIPS_PORT_ID    8

#define BCM54610_PHYID2  0xBD63
#define BCM50612_PHYID2  0x5E60
#define BCM_PHYID_M      0xFFF0

    #define REG_MII_PORT_CONTROL                          0x08

        #define REG_MII_PORT_CONTROL_RX_UCST_EN           0x10
        #define REG_MII_PORT_CONTROL_RX_MCST_EN           0x08
        #define REG_MII_PORT_CONTROL_RX_BCST_EN           0x04 

    #define REG_SWITCH_MODE                               0x0b

        #define REG_SWITCH_MODE_FRAME_MANAGE_MODE         0x01
        #define REG_SWITCH_MODE_SW_FWDG_EN                0x02
        #define REG_SWITCH_MODE_FLSH_GPON_EGRESS_Q        0x08

    #define REG_CONTROL_MII1_PORT_STATE_OVERRIDE          0x0e
        #define REG_CONTROL_MPSO_MII_SW_OVERRIDE          0x80
        #define REG_CONTROL_MPSO_REVERSE_MII              0x10
        #define REG_CONTROL_MPSO_LP_FLOW_CONTROL          0x08
        #define REG_CONTROL_MPSO_SPEED100                 0x04
        #define REG_CONTROL_MPSO_FDX                      0x02
        #define REG_CONTROL_MPSO_LINKPASS                 0x01

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_LED_REFRESH                               0x0f
    #define REG_LED_FUNCTION0_CTRL                        0x10
    #define REG_LED_FUNCTION1_CTRL                        0x12
        #define REG_LED_FUNCTION1_LNK_ACT                 0x20
        #define REG_LED_FUNCTION1_10M_ACT                 0x400
        #define REG_LED_FUNCTION1_100M_ACT                0x800
        #define REG_LED_FUNCTION1_10_100_ACT              0x1000
        #define REG_LED_FUNCTION1_1000M_ACT               0x2000
    #define REG_LED_FUNCTION_MAP                          0x14
    #define REG_LED_ENABLE_MAP                            0x16
    #define REG_LED_MODE_MAP0                             0x18
    #define REG_LED_MODE_MAP1                             0x1a
#endif

    #define REG_PORT_FORWARD                              0x21
        #define REG_PORT_FORWARD_MCST                     0x80
        #define REG_PORT_FORWARD_UCST                     0x40
        #define REG_PORT_FORWARD_IP_MCST                  0x01

    #define REG_PROTECTED_PORT_MAP                        0x24 

    #define REG_WAN_PORT_MAP                              0x26
        #define EN_MAN_TO_WAN                             (1<<9)

    #define REG_PAUSE_CAPBILITY                           0x28
                
        #define REG_PAUSE_CAPBILITY_OVERRIDE              (1<<23)
        #define REG_PAUSE_CAPBILITY_MIPS_RX               (1<<17)
        #define REG_PAUSE_CAPBILITY_SAR_RX                (1<<16)
        #define REG_PAUSE_CAPBILITY_USB_RX                (1<<15)
        #define REG_PAUSE_CAPBILITY_GMII1_RX              (1<<14)
        #define REG_PAUSE_CAPBILITY_GMII0_RX              (1<<13)
        #define REG_PAUSE_CAPBILITY_EPHY3_RX              (1<<12)
        #define REG_PAUSE_CAPBILITY_EPHY2_RX              (1<<11)
        #define REG_PAUSE_CAPBILITY_EPHY1_RX              (1<<10)
        #define REG_PAUSE_CAPBILITY_EPHY0_RX              (1<<9)
        #define REG_PAUSE_CAPBILITY_MIPS_TX               (1<<8)
        #define REG_PAUSE_CAPBILITY_SAR_TX                (1<<7)
        #define REG_PAUSE_CAPBILITY_USB_TX                (1<<6)
        #define REG_PAUSE_CAPBILITY_GMII1_TX              (1<<5)
        #define REG_PAUSE_CAPBILITY_GMII0_TX              (1<<4)
        #define REG_PAUSE_CAPBILITY_EPHY3_TX              (1<<3)
        #define REG_PAUSE_CAPBILITY_EPHY2_TX              (1<<2)
        #define REG_PAUSE_CAPBILITY_EPHY1_TX              (1<<1)
        #define REG_PAUSE_CAPBILITY_EPHY0_TX              (1<<0)

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_RESERVED_MULTICAST_CTRL                   0x2f
#endif

    #define REG_UCST_LOOKUP_FAIL                          0x32 /* 5397 only */

    #define REG_MCST_LOOKUP_FAIL                          0x34 /* 5397 only */

    #define REG_IPMC_LOOKUP_FAIL                          0x36 /* 5397 only */

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_PAUSE_PASS_THROUGH_RX                     0x38
    #define REG_PAUSE_PASS_THROUGH_TX                     0x3a
#endif

    #define REG_DISABLE_LEARNING                          0x3c

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_SOFTWARE_LEARNING                         0x3e
#endif

    #define REG_PORT_STATE                                0x58

        #define REG_PORT_STATE_OVERRIDE                   0x40
        #define REG_PORT_STATE_FLOWCTL                    0x30
        #define REG_PORT_STATE_100                        0x04
        #define REG_PORT_STATE_1000                       0x08
        #define REG_PORT_STATE_FDX                        0x02
        #define REG_PORT_STATE_LNK                        0x01
        #define LINK_OVERRIDE_1000FDX  (REG_PORT_STATE_OVERRIDE |  \
            REG_PORT_STATE_1000 | REG_PORT_STATE_FDX | REG_PORT_STATE_LNK)

#if defined(AEI_VDSL_CUSTOMER_NCS)
#if defined(AEI_63168_CHIP)
#else
    #define REG_PORT5_STATE                                0x5D /*For HPNA port in 6368 chip*/
#endif
#endif

    #define REG_RGMII_CTRL_P4                             0x64
    #define REG_RGMII_CTRL_P5                             0x65
        #define REG_RGMII_CTRL_ENABLE_GMII                0x80
        #define REG_RGMII_CTRL_DLL_IQQD                   0x04
        #define REG_RGMII_CTRL_DLL_RXC_BYPASS             0x02
        #define REG_RGMII_CTRL_TIMING_SEL                 0x01
        #define REG_RGMII_CTRL_ENABLE_RGMII_OVERRIDE      0x40 
        #define REG_RGMII_CTRL_MODE                       0x30
            #define REG_RGMII_CTRL_MODE_RGMII             0x00
            #define REG_RGMII_CTRL_MODE_RvMII             0x20
            #define REG_RGMII_CTRL_MODE_MII               0x10

    #define REG_RGMII_TIMING_P4                           0x6c
    #define REG_RGMII_TIMING_P5                           0x6d
        #define RGMII_EN_DELAY_WRITE                      0x80
        #define RGMII_DELAY_VAL_M                         0x7
        #define RGMII_DELAY_VAL_S                         4
            #define RGMII_DELAY_VAL2                      0x7
        #define RGMII_DELAY_SEL_M                         0xF
        #define RGMII_DELAY_SEL_S                         0
            #define RGMII_DELAY_CLK_GTXR                  0x9
            #define RGMII_DELAY_CLK_RXR                   0xb

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_MDIO_WAN_PORT_ADDR                        0x75
    #define REG_MDIO_IMP_PORT_ADDR                        0x78
#endif

    #define REG_SW_RESET                                  0x79
        #define REG_SW_RST                                0x80
        #define REG_EN_RESET_REG                          0x02
        #define REG_EN_RESET_SW_CORE                      0x01

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_FRAME_DETECT_CTRL                         0x80
#endif

    #define REG_FAST_AGING_CTRL                           0x88
        #define FAST_AGE_START_DONE                       0x80
        #define FAST_AGE_DYNAMIC                          0x02
        #define FAST_AGE_STATIC                           0x01
        #define FAST_AGE_PORT                             0x04
        #define FAST_AGE_VLAN                             0x08
        #define FAST_AGE_SPT                              0x10
    #define REG_FAST_AGING_PORT                           0x89

#if defined(AEI_VDSL_CUSTOMER_NCS)
    #define REG_FAST_AGING_VID_CTRL                       0x8a
#endif

    #define REG_SWPKT_CTRL_USB                            0xa0
    #define REG_SWPKT_CTRL_GPON                           0xa4

    #define REG_IUDMA_CTRL                                0xa8

        #define REG_IUDMA_CTRL_USE_QUEUE_PRIO             0x0400
        #define REG_IUDMA_CTRL_USE_DESC_PRIO              0x0200
        #define REG_IUDMA_CTRL_TX_DMA_EN                  0x0100
        #define REG_IUDMA_CTRL_RX_DMA_EN                  0x0080
        #define REG_IUDMA_CTRL_PRIO_SEL                   0x0040
        #define REG_IUDMA_CTRL_TX_MII_TAG_DISABLE         0x1000		

    #define REG_IUDMA_QUEUE_CTRL                          0xac
        #define REG_IUDMA_Q_CTRL_RXQ_SEL_S                16
        #define REG_IUDMA_Q_CTRL_RXQ_SEL_M                0xFFFF
        #define REG_IUDMA_Q_CTRL_PRIO_TO_CH_M             0x3
        #define REG_IUDMA_Q_CTRL_TXQ_SEL_S                0
        #define REG_IUDMA_Q_CTRL_TXQ_SEL_M                0xFFF
        #define REG_IUDMA_Q_CTRL_CH_TO_PRIO_M             0x7

    #define REG_MDIO_CTRL_ADDR                            0xb0
        #define REG_MDIO_CTRL_WRITE                       (1 << 31)
        #define REG_MDIO_CTRL_READ                        (1 << 30)
        #define REG_MDIO_CTRL_EXT                         (1 << 16)
        #define REG_MDIO_CTRL_EXT_BIT(id)                 (IsExtPhyId(id)? REG_MDIO_CTRL_EXT: 0)
        #define REG_MDIO_CTRL_ID_SHIFT                    25
        #define REG_MDIO_CTRL_ID_MASK                     (0x1f << REG_MDIO_CTRL_ID_SHIFT)
        #define REG_MDIO_CTRL_ADDR_SHIFT                  20
        #define REG_MDIO_CTRL_ADDR_MASK                   (0x1f << REG_MDIO_CTRL_ADDR_SHIFT)

    #define REG_MDIO_DATA_ADDR                            0xb4

    #define REG_EEE_CTRL                                  0xc0
        #define REG_EEE_CTRL_ENABLE                       0x01

    #define REG_EEE_TW_SYS_TX_100                         0xd0
    #define REG_EEE_TW_SYS_TX_1000                        0xd2

/****************************************************************************
    External switch pseudo PHY: Page (0x02)
****************************************************************************/

#define PAGE_MANAGEMENT                                   0x02

    #define REG_GLOBAL_CONFIG                             0x00

        #define ENABLE_MII_PORT                           0x80
        #define RECEIVE_IGMP                              0x08
        #define RECEIVE_BPDU                              0x02
        #define GLOBAL_CFG_RESET_MIB                      0x01

    #define REG_AGING_TIME_CTRL                           0x08

        #define REG_AGE_CHANGE                            0x100000
        #define REG_AGE_TIME_MASK                         0x0FFFFF

    #define REG_MIRROR_CAPTURE_CTRL                       0x10

        #define REG_MIRROR_ENABLE                         0x8000
        #define REG_CAPTURE_PORT_M                        0xF

    #define REG_MIRROR_INGRESS_CTRL                       0x12

        #define REG_INGRESS_MIRROR_M                      0x1FF

    #define REG_DEV_ID                                    0x30

/****************************************************************************
    ARL CTRL Regsiters: Page (0x04)
****************************************************************************/

#define PAGE_ARLCTRL                                      0x04

    #define REG_ARLCFG                                    0x00
        #define MULTIPORT_ADDR_EN_M                       0x1
        #define MULTIPORT_ADDR_EN_S                       4
    #define REG_MULTIPORT_ADDR1_LO                        0x10
    #define REG_MULTIPORT_ADDR1_HI                        0x14
    #define REG_MULTIPORT_VECTOR1                         0x18
    #define REG_MULTIPORT_ADDR2_LO                        0x20
    #define REG_MULTIPORT_ADDR2_HI                        0x24
    #define REG_MULTIPORT_VECTOR2                         0x28

/****************************************************************************
    ARL/VLAN Table Access: Page (0x05)
****************************************************************************/

#define PAGE_AVTBL_ACCESS                                 0x05
    #define REG_ARL_TBL_CTRL                              0x00
    #define ARL_TBL_CTRL_START_DONE                   0x80 
    #define ARL_TBL_CTRL_READ                         0x01
    #define REG_ARL_MAC_INDX_LO                           0x02
    #define REG_ARL_MAC_INDX_HI                           0x04
    #define REG_ARL_VLAN_INDX                             0x08
    #define REG_ARL_MAC_LO_ENTRY                          0x10
    #define REG_ARL_VID_MAC_HI_ENTRY                      0x14
    #define REG_ARL_DATA_ENTRY                            0x18
    #define ARL_DATA_ENTRY_VALID                      0x8000
    #define ARL_DATA_ENTRY_STATIC                     0x4000
    #define REG_ARL_SRCH_CTRL                             0x30
    #define ARL_SRCH_CTRL_START_DONE                  0x80 
    #define ARL_SRCH_CTRL_SR_VALID                    0x01
    #define REG_ARL_SRCH_ADDR                             0x32
    #define REG_ARL_SRCH_MAC_LO_ENTRY                     0x34
    #define REG_ARL_SRCH_VID_MAC_HI_ENTRY                 0x38
    #define REG_ARL_SRCH_DATA_ENTRY                       0x3C
    #define REG_VLAN_TBL_CTRL                             0x60
    #define REG_VLAN_TBL_INDX                             0x62
    #define REG_VLAN_TBL_ENTRY                            0x64

/* External switch 53115 and 53125 ARL search registers */
    #define REG_ARL_SRCH_CTRL_531xx                       0x50
    #define REG_ARL_SRCH_MAC_LO_ENTRY_531xx               0x60
    #define REG_ARL_SRCH_VID_MAC_HI_ENTRY_531xx           0x64
    #define REG_ARL_SRCH_DATA_ENTRY_531xx                 0x68
        #define ARL_DATA_ENTRY_VALID_531xx                0x10000
        #define ARL_DATA_ENTRY_STATIC_531xx               0x8000
    #define REG_ARL_SRCH_DATA_RESULT_DONE_531xx           0x78

/****************************************************************************
   Flow Control: Page (0x0A)
****************************************************************************/

#define PAGE_FLOW_CTRL                                    0x0A

#if defined (CONFIG_BCM96816)
    #define REG_FC_DIAG_PORT_SEL                          0x00
    #define REG_FC_CTRL                                   0x0E
    #define REG_FC_PRIQ_HYST                              0x10
    #define REG_FC_PRIQ_PAUSE                             0x20
    #define REG_FC_PRIQ_DROP                              0x30
    #define REG_FC_PRIQ_TOTAL_HYST                        0x40
    #define REG_FC_PRIQ_TOTAL_PAUSE                       0x50
    #define REG_FC_PRIQ_TOTAL_DROP                        0x60
    #define REG_FC_PRIQ_LO_DROP                           0x70
    #define REG_FC_Q_MON_CNT                              0x90
    #define REG_FC_PEAK_Q_MON_CNT                         0xA0
    #define REG_PEAK_RXBUF_COUNT                          0xB0
#else
    #define REG_FC_DIAG_PORT_SEL                          0x00
    #define REG_FC_CTRL                                   0x0E
    #define REG_FC_PRIQ_HYST                              0x10
    #define REG_FC_PRIQ_PAUSE                             0x18
    #define REG_FC_PRIQ_DROP                              0x20
    #define REG_FC_PRIQ_TOTAL_HYST                        0x28
    #define REG_FC_PRIQ_TOTAL_PAUSE                       0x30
    #define REG_FC_PRIQ_TOTAL_DROP                        0x38
    #define REG_FC_Q_MON_CNT                              0x60
    #define REG_PEAK_RXBUF_COUNT                          0x78
#endif

/****************************************************************************
   MIB Counters: Page (0x20 to 0x28)
****************************************************************************/

#define PAGE_MIB_P0                                       0x20

    #define REG_MIB_P0_TXOCTETS                           0x00
    #define REG_MIB_P0_TXDROPS                            0x08
    #define REG_MIB_P0_TXBPKTS                            0x10
    #define REG_MIB_P0_TXMPKTS                            0x14
    #define REG_MIB_P0_TXUPKTS                            0x18  
    #define REG_MIB_P0_TXFRAMEINDISC                      0x34
    #define REG_MIB_P0_RXOCTETS                           0x44
    #define REG_MIB_P0_RXUNDERSIZEPKTS                    0x4C
    #define REG_MIB_P0_RXJABBERS                          0x70
    #define REG_MIB_P0_RXALIGNERRORS                      0x74
    #define REG_MIB_P0_RXFCSERRORS                        0x78
    #define REG_MIB_P0_RXDROPS                            0x84
    #define REG_MIB_P0_RXUPKTS                            0x88
    #define REG_MIB_P0_RXMPKTS                            0x8c
    #define REG_MIB_P0_RXBPKTS                            0x90
    #define REG_MIB_P0_RXFRAGMENTS                        0x98
    #define REG_MIB_P0_RXSYMBOLERRORS                     0xA0
    #define REG_MIB_P0_RXEXCESSSIZEDISC                   0x9C


/****************************************************************************
   QOS : Page (0x30)
****************************************************************************/

#define PAGE_QOS                                          0x30

    #define REG_QOS_GLOBAL_CTRL                           0x00
#if defined (CONFIG_BCM96816)
        #define PORT_QOS_EN_M                             0x1
        #define PORT_QOS_EN_S                             7
        #define QOS_LAYER_SEL_M                           0x3
        #define QOS_LAYER_SEL_S                           3
        #define MIB_QOS_MONITOR_SET_M                     0x7
        #define MIB_QOS_MONITOR_SET_S                     0
#else
        #define PORT_QOS_EN_M                             0x1
        #define PORT_QOS_EN_S                             6
        #define QOS_LAYER_SEL_M                           0x3
        #define QOS_LAYER_SEL_S                           2
        #define MIB_QOS_MONITOR_SET_M                     0x3
        #define MIB_QOS_MONITOR_SET_S                     0
#endif
    #define REG_QOS_THRESHOLD_CTRL                        0x02
        #define AUTOSET_QOS_EN                            0x2000
    #define REG_QOS_8021P_EN                              0x04
    #define REG_QOS_8021P_PRIO_MAP                        0x10
    #define REG_QOS_PORT_PRIO_MAP_P0                      0x50
#if defined (CONFIG_BCM96816)
    #define REG_QOS_PRIO_TO_QID_SEL_BITS                  3
    #define REG_QOS_PRIO_TO_QID_SEL_M                     0x7
#else
    #define REG_QOS_PRIO_TO_QID_SEL_BITS                  2
    #define REG_QOS_PRIO_TO_QID_SEL_M                     0x3
#endif

    #define REG_QOS_DSCP_PRIO_MAP0LO                      0x30
    #define REG_QOS_DSCP_PRIO_MAP0HI                      0x34
    #define REG_QOS_DSCP_PRIO_MAP1LO                      0x36
    #define REG_QOS_DSCP_PRIO_MAP1HI                      0x38
    #define REG_QOS_DSCP_PRIO_MAP2LO                      0x3c
    #define REG_QOS_DSCP_PRIO_MAP2HI                      0x40
    #define REG_QOS_DSCP_PRIO_MAP3LO                      0x42
    #define REG_QOS_DSCP_PRIO_MAP3HI                      0x44

    #define REG_QOS_TXQ_CTRL                              0x80
#if defined (CONFIG_BCM96816)
        #define TXQ_CTRL_TXQ_MODE_M                       0x7 
        #define TXQ_CTRL_TXQ_MODE_S                       2 
        #define TXQ_CTRL_HQ_PREEMPT_M                     0x1
        #define TXQ_CTRL_HQ_PREEMPT_S                     5 
#else
        #define TXQ_CTRL_TXQ_MODE_M                       0x3 
        #define TXQ_CTRL_TXQ_MODE_S                       2 
        #define TXQ_CTRL_HQ_PREEMPT_M                     0x1
        #define TXQ_CTRL_HQ_PREEMPT_S                     4 
#endif
    #define REG_QOS_TXQ_WEIGHT_Q0                         0x81
    #define REG_QOS_VID_REMAP                             0xB0
    #define REG_QOS_VID_REMAP_CTRL                        0xD0

/****************************************************************************
    Port-Based VLAN Control: Page (0x31)
****************************************************************************/

#define PAGE_PORT_BASED_VLAN                              0x31

    #define REG_VLAN_CTRL_P0                              0x00
    #define REG_VLAN_CTRL_P1                              0x02
    #define REG_VLAN_CTRL_P2                              0x04
    #define REG_VLAN_CTRL_P3                              0x06
    #define REG_VLAN_CTRL_P4                              0x08
    #define REG_VLAN_CTRL_P5                              0x0a
    #define REG_VLAN_CTRL_P6                              0x0c
    #define REG_VLAN_CTRL_P7                              0x0e
    #define REG_VLAN_CTRL_P8                              0x10

/****************************************************************************
   802.1Q VLAN Control: Page (0x34)
****************************************************************************/

#define PAGE_8021Q_VLAN                                   0x34

    #define REG_VLAN_GLOBAL_8021Q                         0x00
        #define VLAN_EN_8021Q_M                           0x1
        #define VLAN_EN_8021Q_S                           7
        #define PRIO_TAG_FRAME_CTRL_M                     0x3
        #define PRIO_TAG_FRAME_CTRL_S                     0
    #define REG_VLAN_GLOBAL_CTRL1                         0x01
    #define REG_VLAN_GLOBAL_CTRL2                         0x02
    #define REG_VLAN_GLOBAL_CTRL3                         0x04
    #define REG_VLAN_GLOBAL_CTRL4                         0x06
    #define REG_VLAN_GLOBAL_CTRL5                         0x07
        #define TAG_STATUS_PRESERVE_M                     0x3
        #define TAG_STATUS_PRESERVE_S                     5
        #define TAG_STATUS_PRESERVE_01                    1
        #define TAG_STATUS_PRESERVE_10                    2
        #define VID_FFF_ENABLE_M                          0x1
        #define VID_FFF_ENABLE_S                          2
    #define REG_VLAN_8021P_REMAP                          0x0c
    #define REG_VLAN_DEFAULT_TAG                          0x10
        #define DEFAULT_TAG_PRIO_M                        0x7
        #define DEFAULT_TAG_PRIO_S                        13
        #define DEFAULT_TAG_CFI_M                         0x1
        #define DEFAULT_TAG_CFI_S                         12
        #define DEFAULT_TAG_VID_M                         0xFFF
        #define DEFAULT_TAG_VID_S                         0
    #define REG_VLAN_DOUBLE_TAG_TPID                      0x30
    #define REG_VLAN_ISP_PORT_SEL                         0x32

/****************************************************************************
   Jumbo Frame Control: Page (0x40)
****************************************************************************/

#define PAGE_JUMBO                                        0x40

    #define REG_JUMBO_PORT_MASK                           0x04
    #define REG_JUMBO_FRAME_SIZE                          0x08

    #define MAX_JUMBO_FRAME_SIZE                          2048

    // Defines for JUMBO register control
    #define ETHSWCTL_JUMBO_PORT_ALL                       9
    #define ETHSWCTL_JUMBO_PORT_MIPS                      8
    #define ETHSWCTL_JUMBO_PORT_GPON                      7
    #define ETHSWCTL_JUMBO_PORT_USB                       6
    #define ETHSWCTL_JUMBO_PORT_MOCA                      5
    #define ETHSWCTL_JUMBO_PORT_GPON_SERDES               4
    #define ETHSWCTL_JUMBO_PORT_GMII_2                    3
    #define ETHSWCTL_JUMBO_PORT_GMII_1                    2
    #define ETHSWCTL_JUMBO_PORT_GPHY_1                    1
    #define ETHSWCTL_JUMBO_PORT_GPHY_0                    0

    #define ETHSWCTL_JUMBO_PORT_MIPS_MASK                 0x0100
    #define ETHSWCTL_JUMBO_PORT_GPON_MASK                 0x0080
    #define ETHSWCTL_JUMBO_PORT_USB_MASK                  0x0040
    #define ETHSWCTL_JUMBO_PORT_MOCA_MASK                 0x0020
    #define ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK          0x0010
    #define ETHSWCTL_JUMBO_PORT_GMII_2_MASK               0x0008
    #define ETHSWCTL_JUMBO_PORT_GMII_1_MASK               0x0004
    #define ETHSWCTL_JUMBO_PORT_GPHY_1_MASK               0x0002
    #define ETHSWCTL_JUMBO_PORT_GPHY_0_MASK               0x0001
    #define ETHSWCTL_JUMBO_PORT_MASK_VAL                  0x01FF

/****************************************************************************
   Broadcast Storm Suprression Registers
****************************************************************************/

#define PAGE_BSS                                          0x41

    #define REG_BSS_IRC_CONFIG                            0x00
        #define IRC_CFG_XLENEN                            16
        #define IRC_CFG_PKT_MSK0_EXT_S                    17

    #define REG_BSS_RX_RATE_CTRL_P0                       0x10
        #define IRC_BKT0_RATE_CNT_M                       0xFF
        #define IRC_BKT0_RATE_CNT_S                       0
        #define IRC_BKT0_SIZE_M                           0x7
        #define IRC_BKT0_SIZE_S                           8
        #define IRC_BKT0_EN_M                             0x01
        #define IRC_BKT0_EN_S                             22

    #define REG_BSS_PKT_DROP_CNT_P0                       0x50

    #define REG_BSS_TX_RATE_CTRL_P0                       0x80
        #define ERC_RFSH_CNT_M                            0xFF
        #define ERC_RFSH_CNT_S                            0
        #define ERC_BKT_SIZE_M                            0x7
        #define ERC_BKT_SIZE_S                            8
        #define ERC_ERC_EN_M                              0x01
        #define ERC_ERC_EN_S                              11

#if defined(AEI_VDSL_POWERSAVE)
    #define PAGE_EEE                                      0x92
        #define REG_EEE_ENABLE_CTRL                       0x00
        #define REG_EEE_SLEEP_TIME_GIG                    0x10
        #define REG_EEE_SLEEP_TIME_GIG_IMP                0x30
        #define REG_EEE_SLEEP_TIME_FE                     0x34
#endif

/****************************************************************************
   FFE Registers
****************************************************************************/

#define PAGE_FFE                                          0xB4
    #define REG_FFE_RXPORT_CTRL                           0x1C
	    #define FFE_PAD_ENABLE_M                          0x1
	    #define FFE_PAD_ENABLE_S                          8
	    #define FFE_PAD_SIZE_M                            0xFF
	    #define FFE_PAD_SIZE_S                            0

/****************************************************************************
   Shared Defines for bcmsw.c and ethsw.c files
****************************************************************************/
#define TOTAL_SWITCH_PORTS   9
#define PBMAP_ALL            0x1FF
#define PBMAP_MIPS           0x100
#define PBMAP_MIPS_N_GPON    0x180
#define DEFAULT_PBVLAN_MAP   0x1FF


/****************************************************************************
   Actiontec specific defines
****************************************************************************/
#if defined(AEI_VDSL_WAN_ETH)
#define EWAN_PORT_ID    4
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
#define LAN_PORT_ID_4   3        
#define LAN_PORT_ID_3   2                             
#define LAN_PORT_ID_2   1
#define LAN_PORT_ID_1   0  
#endif

#if defined(AEI_VDSL_HPNA)
#define HPNA_PORT_ID    0x11
#endif

#endif /* _BCMMII_H_ */
