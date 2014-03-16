/*
<:copyright-broadcom 
 
 Copyright (c) 2010 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation
          5300 California Avenue
          Irvine, California 92617
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  63268_common.h                                           */
/*   DATE:    11/09/10                                                 */
/*   PURPOSE: Define addresses of major hardware components of         */
/*            BCM63268                                                 */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM63268_MAP_COMMON_H
#define __BCM63268_MAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__KERNEL__) || defined (_CFE_) || defined(FAP_4KE)
/* Access to registers from kernelspace */
#define REG_BASE                 0xb0000000
#else
/* Access to registers from userspace, see bcm_mmap.h for api */
#define REG_BASE                 (bcm_mmap_info.mmap_addr)
#define BCM_MMAP_INFO_BASE       0x10000000
#define BCM_MMAP_INFO_SIZE       0x01000000
#endif

#define PERF_BASE                   (REG_BASE + 0x00000000)  /* chip control */
#define TIMR_BASE                   (REG_BASE + 0x00000080)  /* timer registers */
#define NAND_INTR_BASE              (REG_BASE + 0x000000b0)  /* nand interrupt control */
#define GPIO_BASE                   (REG_BASE + 0x000000c0)  /* gpio registers */
#define UART_BASE                   (REG_BASE + 0x00000180)  /* uart registers */
#define UART1_BASE                  (REG_BASE + 0x000001a0)  /* uart registers */
#define NAND_REG_BASE               (REG_BASE + 0x00000200)  /* nand interrupt control */
#define OTP_BASE                    (REG_BASE + 0x00000400)
#define NAND_CACHE_BASE             (REG_BASE + 0x00000600)
#define SPI_BASE                    (REG_BASE + 0x00000800)  /* SPI master controller */
#define HSSPIM_BASE                 (REG_BASE + 0x00001000)  /* High-Speed SPI registers */
#define VDSL_CTRL_BASE              (REG_BASE + 0x00001800)
#define MISC_BASE                   (REG_BASE + 0x00001800)  /* Miscellaneous Registers */
#define LED_BASE                    (REG_BASE + 0x00001900)  /* LED control registers */
#define UBUS_STAT_BASE              (REG_BASE + 0x00001a00)
#define USB_CTL_BASE                (REG_BASE + 0x00002400)  /* USB 2.0 device control */
#ifdef __KERNEL__
#define USB_EHCI_BASE               0x10002500  /* USB host registers */
#define USB_OHCI_BASE               0x10002600  /* USB host registers */
#endif
#define USBH_CFG_BASE               (REG_BASE + 0x00002700)
#define IPSEC_BASE                  (REG_BASE + 0x00002800)
#define IPSEC_RXBUF_CMD             (REG_BASE + 0x0000287c)
#define MEMC_BASE                   (REG_BASE + 0x00003000)  /* DDR IO Buf Control */
#ifdef __KERNEL__
#define WLAN_CHIPC_BASE             0x10004000  /* WLAN ChipCommon registers, use 1xxx for ioremap */
#define WLAN_MAC_BASE               0x10005000  /* WLAN d11mac registers */
#endif
#define WLAN_SHIM_BASE              (REG_BASE + 0x00007000)
#define SAR_BASE                    (REG_BASE + 0x00007800)
#define DECT_SHIM_CTRL_BASE         (REG_BASE + 0x0000a000)
#define DECT_SHIM_DMA_CTRL_BASE     (REG_BASE + 0x0000a050)
#define DECT_SHIM_TEST_BASE         (REG_BASE + 0x0000a0f0)
#define DECT_APB_REG_BASE           (REG_BASE + 0x0000a800)
#define PCM_BASE                    (REG_BASE + 0x0000b000)  /* PCM control registers */
#define PCM_DMA_BASE                (REG_BASE + 0x0000b800)  /* PCM UIDMA register base */
#define SAR_DMA_BASE                (REG_BASE + 0x0000c000)  /* ATM SAR DMA control */
#define USB_DMA_BASE                (REG_BASE + 0x0000c800)  /* USB 2.0 device DMA */
#define IPSEC_DMA_BASE              (REG_BASE + 0x0000d000)
#define SWITCH_DMA_BASE             (REG_BASE + 0x0000d800)
#define PCIE_BASE                   (REG_BASE + 0x006e0000)
#define DECT_AHB_SHARED_RAM_BASE    (REG_BASE + 0x006f0000)
#define DECT_AHB_REG_BASE           (REG_BASE + 0x006f7f80)
#define SWITCH_BASE                 (REG_BASE + 0x00700000)
#define PCIE_MEM1_BASE              (REG_BASE + 0x00f00000)

#define FAP0_BASE                   (REG_BASE + 0x00800000)
#define FAP0_CNTRL_BASE             (REG_BASE + 0x00801000)
#define FAP0_IN_FIFO_BASE           (REG_BASE + 0x00801100)
#define FAP0_OUT_FIFO_BASE          (REG_BASE + 0x00801200)
#define FAP0_DMA0_BASE              (REG_BASE + 0x00801300)
#define FAP0_DMA1_BASE              (REG_BASE + 0x00801320)
#define FAP0_TOKEN_BASE             (REG_BASE + 0x00801400)
#define FAP0_MSG_BASE               (REG_BASE + 0x00801700)
#define FAP0_DQM_BASE               (REG_BASE + 0x00801800)
#define FAP0_DQM_CNTRL_BASE         (REG_BASE + 0x00801a00)
#define FAP0_DQM_DATA_BASE          (REG_BASE + 0x00801c00)
#define FAP0_DQM_MIB_BASE           (REG_BASE + 0x00802000)
#define FAP0_QSM_UBUS_BASE          (REG_BASE + 0x00804000)
#define FAP0_QSM_SMI_BASE           (REG_BASE + 0x00c04000)
#define FAP0_PSM_BASE               (REG_BASE + 0x00820000)
#define FAP0_SMISB                  (REG_BASE + 0x00c00000)

#define FAP1_BASE                   (REG_BASE + 0x00a00000)
#define FAP1_CNTRL_BASE             (REG_BASE + 0x00a01000)
#define FAP1_IN_FIFO_BASE           (REG_BASE + 0x00a01100)
#define FAP1_OUT_FIFO_BASE          (REG_BASE + 0x00a01200)
#define FAP1_DMA0_BASE              (REG_BASE + 0x00a01300)
#define FAP1_DMA1_BASE              (REG_BASE + 0x00a01320)
#define FAP1_TOKEN_BASE             (REG_BASE + 0x00a01400)
#define FAP1_MSG_BASE               (REG_BASE + 0x00a01700)
#define FAP1_DQM_BASE               (REG_BASE + 0x00a01800)
#define FAP1_DQM_CNTRL_BASE         (REG_BASE + 0x00a01a00)
#define FAP1_DQM_DATA_BASE          (REG_BASE + 0x00a01c00)
#define FAP1_DQM_MIB_BASE           (REG_BASE + 0x00a02000)
#define FAP1_QSM_UBUS_BASE          (REG_BASE + 0x00a04000)
#define FAP1_QSM_SMI_BASE           (REG_BASE + 0x00e04000)
#define FAP1_PSM_BASE               (REG_BASE + 0x00a20000)
#define FAP1_SMISB                  (REG_BASE + 0x00e00000)



/*
#####################################################################
# System PLL Control Register
#####################################################################
*/


/*
#####################################################################
# GPIO Control Registers
#####################################################################
*/
#define GPIO_DATA_HI               (GPIO_BASE + 0x8)
#define GPIO_DATA                  (GPIO_BASE + 0xC)

/*
#####################################################################
# Miscellaneous Registers
#####################################################################
*/
#define MISC_MEMC_CONTROL                       0x10
#define MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE    (1<<3)
#define MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE     (1<<2)

#define MISC_STRAP_BUS                          0x14
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT      21
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK       (0xF<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_DDR2_DDR3_N_SELECT       (1<<6)

#define MISC_VREG_CONTROL0                      0x1C
#define MISC_VREG_CONTROL0_REG_RESET_B          (1<<31)
#define MISC_VREG_CONTROL0_OVERCUR_SEL_2_SHIFT	18
#define MISC_VREG_CONTROL0_OVERCUR_SEL_2_MASK	0x3
#define MISC_VREG_CONTROL0_OVERCUR_SEL_1_SHIFT	16
#define MISC_VREG_CONTROL0_OVERCUR_SEL_1_MASK	0x3
#define MISC_VREG_CONTROL0_NOVL_2_SHIFT		4
#define MISC_VREG_CONTROL0_NOVL_2_MASK		0xf
#define MISC_VREG_CONTROL0_NOVL_1_SHIFT		0
#define MISC_VREG_CONTROL0_NOVL_1_MASK		0xf
    
#define MISC_VREG_CONTROL1                      0x20
#define MISC_VREG_CONTROL1_VCM2_ADJ_SHIFT       9
#define MISC_VREG_CONTROL1_VCM1_ADJ_SHIFT       0
#define MISC_VREG_CONTROL1_VCM2_ADJ_MASK        0x1FF
#define MISC_VREG_CONTROL1_VCM1_ADJ_MASK        0x1FF

#define MISC_VREG_CONTROL2                      0x24
#define MISC_VREG_CONTROL2_SWITCHCLOCKEN        (1<<7)

/*
#####################################################################
# Memory Control Registers
#####################################################################
*/
#define MEMC_CTL_CNFG                           0x000 
#define MEMC_CTL_CSST                           0x004 
#define MEMC_CTL_CSEND                          0x008 
#define MEMC_CTL_ROW00_0                        0x010 
#define MEMC_CTL_ROW00_1                        0x014 
#define MEMC_CTL_ROW01_0                        0x018 
#define MEMC_CTL_ROW01_1                        0x01c 
#define MEMC_CTL_ROW20_0                        0x030 
#define MEMC_CTL_ROW20_1                        0x034 
#define MEMC_CTL_ROW21_0                        0x038 
#define MEMC_CTL_ROW21_1                        0x03c 
#define MEMC_CTL_COL00_0                        0x050 
#define MEMC_CTL_COL00_1                        0x054 
#define MEMC_CTL_COL01_0                        0x058 
#define MEMC_CTL_COL01_1                        0x05c 
#define MEMC_CTL_COL20_0                        0x070 
#define MEMC_CTL_COL20_1                        0x074 
#define MEMC_CTL_COL21_0                        0x078 
#define MEMC_CTL_COL21_1                        0x07c 
#define MEMC_CTL_BNK10                          0x090 
#define MEMC_CTL_BNK32                          0x094 
#define MEMC_CTL_DCMD                           0x100 
#define MEMC_CTL_DMODE_0                        0x104 
#define MEMC_CTL_DMODE_2                        0x108 
#define MEMC_CTL_CLKS                           0x10c 
#define MEMC_CTL_ODT                            0x110 
#define MEMC_CTL_TIM1_0                         0x114 
#define MEMC_CTL_TIM1_1                         0x118 
#define MEMC_CTL_TIM2                           0x11c 
#define MEMC_CTL_CTL_CRC                        0x120 
#define MEMC_CTL_DOUT_CRC                       0x124 
#define MEMC_CTL_DIN_CRC                        0x128 
#define MEMC_CTL_DRAM_CFG                       0x134 
#define MEMC_CTL_STAT                           0x138 

#define PHY_CONTROL_REGS_REVISION               0x200 
#define PHY_CONTROL_REGS_CLK_PM_CTRL            0x204 
#define PHY_CONTROL_REGS_PLL_STATUS             0x210 
#define PHY_CONTROL_REGS_PLL_CONFIG             0x214 
#define PHY_CONTROL_REGS_PLL_PRE_DIVIDER        0x218 
#define PHY_CONTROL_REGS_PLL_DIVIDER            0x21c 
#define PHY_CONTROL_REGS_PLL_CONTROL1           0x220 
#define PHY_CONTROL_REGS_PLL_CONTROL2           0x224 
#define PHY_CONTROL_REGS_PLL_SS_EN              0x228 
#define PHY_CONTROL_REGS_PLL_SS_CFG             0x22c 
#define PHY_CONTROL_REGS_STATIC_VDL_OVERRIDE    0x230 
#define PHY_CONTROL_REGS_DYNAMIC_VDL_OVERRIDE   0x234 
#define PHY_CONTROL_REGS_IDLE_PAD_CONTROL       0x238 
#define PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL        0x23c 
#define PHY_CONTROL_REGS_DRIVE_PAD_CTL          0x240 

#define PHY_BYTE_LANE_0_REVISION                0x300 
#define PHY_BYTE_LANE_0_VDL_CALIBRATE           0x304 
#define PHY_BYTE_LANE_0_VDL_STATUS              0x308 
    #define PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MAX  31
    #define PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT 8
    #define PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_MASK (0x1f << PHY_BYTE_LANE_0_VDL_STATUS_CALIB_TOTAL_STEP_STRT)
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_0          0x310 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_1          0x314 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_2          0x318 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_3          0x31c 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_4          0x320 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_5          0x324 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_6          0x328 
#define PHY_BYTE_LANE_0_VDL_OVERRIDE_7          0x32c 
#define PHY_BYTE_LANE_0_READ_CONTROL            0x330 
#define PHY_BYTE_LANE_0_READ_FIFO_STATUS        0x334 
#define PHY_BYTE_LANE_0_READ_FIFO_CLEAR         0x338 
#define PHY_BYTE_LANE_0_IDLE_PAD_CONTROL        0x33c 
#define PHY_BYTE_LANE_0_DRIVE_PAD_CTL           0x340 
#define PHY_BYTE_LANE_0_CLOCK_PAD_DISABLE       0x344 
#define PHY_BYTE_LANE_0_WR_PREAMBLE_MODE        0x348 

#define PHY_BYTE_LANE_1_REVISION                0x400 
#define PHY_BYTE_LANE_1_VDL_CALIBRATE           0x404 
#define PHY_BYTE_LANE_1_VDL_STATUS              0x408 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_0          0x410 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_1          0x414 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_2          0x418 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_3          0x41c 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_4          0x420 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_5          0x424 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_6          0x428 
#define PHY_BYTE_LANE_1_VDL_OVERRIDE_7          0x42c 
#define PHY_BYTE_LANE_1_READ_CONTROL            0x430 
#define PHY_BYTE_LANE_1_READ_FIFO_STATUS        0x434 
#define PHY_BYTE_LANE_1_READ_FIFO_CLEAR         0x438 
#define PHY_BYTE_LANE_1_IDLE_PAD_CONTROL        0x43c 
#define PHY_BYTE_LANE_1_DRIVE_PAD_CTL           0x440 
#define PHY_BYTE_LANE_1_CLOCK_PAD_DISABLE       0x444 
#define PHY_BYTE_LANE_1_WR_PREAMBLE_MODE        0x448 

#define PHY_BYTE_LANE_2_REVISION                0x500 
#define PHY_BYTE_LANE_2_VDL_CALIBRATE           0x504 
#define PHY_BYTE_LANE_2_VDL_STATUS              0x508 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_0          0x510 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_1          0x514 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_2          0x518 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_3          0x51c 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_4          0x520 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_5          0x524 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_6          0x528 
#define PHY_BYTE_LANE_2_VDL_OVERRIDE_7          0x52c 
#define PHY_BYTE_LANE_2_READ_CONTROL            0x530 
#define PHY_BYTE_LANE_2_READ_FIFO_STATUS        0x534 
#define PHY_BYTE_LANE_2_READ_FIFO_CLEAR         0x538 
#define PHY_BYTE_LANE_2_IDLE_PAD_CONTROL        0x53c 
#define PHY_BYTE_LANE_2_DRIVE_PAD_CTL           0x540 
#define PHY_BYTE_LANE_2_CLOCK_PAD_DISABLE       0x544 
#define PHY_BYTE_LANE_2_WR_PREAMBLE_MODE        0x548 

#define PHY_BYTE_LANE_3_REVISION                0x600 
#define PHY_BYTE_LANE_3_VDL_CALIBRATE           0x604 
#define PHY_BYTE_LANE_3_VDL_STATUS              0x608 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_0          0x610 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_1          0x614 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_2          0x618 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_3          0x61c 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_4          0x620 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_5          0x624 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_6          0x628 
#define PHY_BYTE_LANE_3_VDL_OVERRIDE_7          0x62c 
#define PHY_BYTE_LANE_3_READ_CONTROL            0x630 
#define PHY_BYTE_LANE_3_READ_FIFO_STATUS        0x634 
#define PHY_BYTE_LANE_3_READ_FIFO_CLEAR         0x638 
#define PHY_BYTE_LANE_3_IDLE_PAD_CONTROL        0x63c 
#define PHY_BYTE_LANE_3_DRIVE_PAD_CTL           0x640 
#define PHY_BYTE_LANE_3_CLOCK_PAD_DISABLE       0x644 
#define PHY_BYTE_LANE_3_WR_PREAMBLE_MODE        0x648 

#define MEMC_CTL_GCFG                           0x800 
    #define MEMC_CTL_GCFG_MEMINITDONE                   (1 << 8)
#define MEMC_CTL_VERS                           0x804 
#define MEMC_CTL_ARB                            0x80c 
#define MEMC_CTL_PI_GCF                         0x810 
#define MEMC_CTL_PI_UBUS_CTL                    0x814 
#define MEMC_CTL_PI_MIPS_CTL                    0x818 
#define MEMC_CTL_PI_DSL_MIPS_CTL                0x81c 
#define MEMC_CTL_PI_DSL_PHY_CTL                 0x820 
#define MEMC_CTL_PI_UBUS_ST                     0x824 
#define MEMC_CTL_PI_MIPS_ST                     0x828 
#define MEMC_CTL_PI_DSL_MIPS_ST                 0x82c 
#define MEMC_CTL_PI_DSL_PHY_ST                  0x830 
#define MEMC_CTL_PI_UBUS_SMPL                   0x834
    #define MEMC_CTL_PI_UBUS_SMPL_SAMPLING_PERIOD_SHIFT 28
#define MEMC_CTL_TESTMODE                       0x838 
#define MEMC_CTL_TEST_CFG1                      0x83c 
#define MEMC_CTL_TEST_PAT                       0x840 
#define MEMC_CTL_TEST_COUNT                     0x844 
#define MEMC_CTL_TEST_CURR_COUNT                0x848 
#define MEMC_CTL_TEST_ADDR_UPDT                 0x84c 
#define MEMC_CTL_TEST_ADDR                      0x850 
#define MEMC_CTL_TEST_DATA0_0                   0x854 
#define MEMC_CTL_TEST_DATA0_1                   0x858 
#define MEMC_CTL_TEST_DATA0_2                   0x85c 
#define MEMC_CTL_TEST_DATA0_3                   0x860 
#define MEMC_CTL_TEST_DATA1_0                   0x864 
#define MEMC_CTL_TEST_DATA1_1                   0x868 
#define MEMC_CTL_TEST_DATA1_2                   0x86c 
#define MEMC_CTL_TEST_DATA1_3                   0x870 
#define MEMC_CTL_REPLY_DATA0                    0x874 
#define MEMC_CTL_REPLY_DATA1                    0x878 
#define MEMC_CTL_REPLY_DATA2                    0x87c 
#define MEMC_CTL_REPLY_DATA3                    0x880 
#define MEMC_CTL_REPLY_STAT                     0x884 
#define MEMC_CTL_LBIST_CFG                      0x888 
#define MEMC_CTL_LBIST_SEED                     0x88c 
#define MEMC_CTL_PI_MIPS_SMPL                   0x890 
    #define MEMC_CTL_PI_MIPS_SMPL_SAMPLING_PERIOD_SHIFT 28


/*
#####################################################################
# UART Control Registers
#####################################################################
*/
#define UART0CONTROL     0x01
#define UART0CONFIG      0x02
#define UART0RXTIMEOUT   0x03
#define UART0BAUD        0x04
#define UART0FIFOCFG     0x0a
#define UART0INTMASK     0x10
#define UART0INTSTAT     0x12
#define UART0DATA        0x17

#define BRGEN            0x80   /* Control register bit defs */
#define TXEN             0x40
#define RXEN             0x20
#define LOOPBK           0x10
#define TXPARITYEN       0x08
#define TXPARITYEVEN     0x04
#define RXPARITYEN       0x02
#define RXPARITYEVEN     0x01

#define XMITBREAK        0x40   /* Config register */
#define BITS5SYM         0x00
#define BITS6SYM         0x10
#define BITS7SYM         0x20
#define BITS8SYM         0x30
#define ONESTOP          0x07
#define TWOSTOP          0x0f

#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000


/*
#####################################################################
# DECT IP Control Registers
#####################################################################
*/
#define DECT_STARTCTL      (DECT_AHB_SHARED_RAM_BASE + 0x0818)
#define PCM_BUFF_CTL3      (DECT_AHB_SHARED_RAM_BASE + 0x082c)
#define PCM_BUFF_CTL7      (DECT_AHB_SHARED_RAM_BASE + 0x083c)
#define DECT_AHB_CHAN0_RX  (DECT_AHB_SHARED_RAM_BASE + 0x0a20)
#define DECT_AHB_CHAN1_RX  (DECT_AHB_SHARED_RAM_BASE + 0x0de0)
#define DECT_AHB_CHAN2_RX  (DECT_AHB_SHARED_RAM_BASE + 0x11a0)
#define DECT_AHB_CHAN3_RX  (DECT_AHB_SHARED_RAM_BASE + 0x1560)
#define DECT_AHB_CHAN0_TX  (DECT_AHB_SHARED_RAM_BASE + 0x0840)
#define DECT_AHB_CHAN1_TX  (DECT_AHB_SHARED_RAM_BASE + 0x0c00)
#define DECT_AHB_CHAN2_TX  (DECT_AHB_SHARED_RAM_BASE + 0x0fc0)
#define DECT_AHB_CHAN3_TX  (DECT_AHB_SHARED_RAM_BASE + 0x1380)
#define DECT_CLKEN         0x00000040   

#ifdef __cplusplus
}
#endif

#endif
