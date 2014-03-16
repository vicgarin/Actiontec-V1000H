/*
<:copyright-gpl
 Copyright 2004-2010 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/
#include "bcm_OS_Deps.h"
#include "ethsw_phy.h"
#include "bcmsw.h"
#include "boardparms.h"

extern spinlock_t bcm_ethlock_phy_access;
extern atomic_t phy_read_ref_cnt;
extern atomic_t phy_write_ref_cnt;

void ethsw_phy_rreg(int phy_id, int reg, uint16 *data)
{
    int ext_bit = IsExtPhyId(phy_id);
    ethsw_phy_read_reg(phy_id, reg, data, ext_bit);
}

void ethsw_phy_read_reg(int phy_id, int reg, uint16 *data, int ext_bit)
{
    uint32 reg_value;
    int    is6829 = 0, in_interrupt = 0;
#if defined(CONFIG_BCM96816)
    if ( IsExt6829(phy_id) )
    {
       is6829  = 1;
    }
#endif
    phy_id &= BCM_PHY_ID_M;

    reg_value = 0;

    if (in_interrupt()) {
        in_interrupt = 1;
    }

    /* Disable Phy interrupt so Phy ISR will not enter this function while a transaction is in progress. Will need similar protection if there is ever a need
           to access phy in another interrupt context (like the packet rx isr (napi)). We can use spin_lock_irqsave but the phy access takes >50uS and
           it disables all the interrupts on the local CPU(not just the Phy interrupt) */
    if (!in_interrupt) {
        BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
        BcmHalInterruptDisable(INTERRUPT_ID_GPHY);
#endif
    }
    atomic_inc(&phy_read_ref_cnt);
    spin_lock(&bcm_ethlock_phy_access);

    ethsw_wreg_ext(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&reg_value, 4, is6829);
    reg_value = (ext_bit? REG_MDIO_CTRL_EXT: 0) | (REG_MDIO_CTRL_READ) |
        ((phy_id << REG_MDIO_CTRL_ID_SHIFT) & REG_MDIO_CTRL_ID_MASK) |
        (reg  << REG_MDIO_CTRL_ADDR_SHIFT);
    ethsw_wreg_ext(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&reg_value, 4, is6829);

    if (irqs_disabled() || (preempt_count() != 0)) {
        udelay(60);
    } else {
        msleep(1);
    }
    ethsw_rreg_ext(PAGE_CONTROL, REG_MDIO_DATA_ADDR, (uint8 *)data, 2, is6829);

    spin_unlock(&bcm_ethlock_phy_access);
    atomic_dec(&phy_read_ref_cnt);
    if (!in_interrupt) {
        if (atomic_read(&phy_read_ref_cnt) == 0) {
            BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
            BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif
        }
    }
}

void ethsw_phy_wreg(int phy_id, int reg, uint16 *data)
{
    int ext_bit = IsExtPhyId(phy_id);
    ethsw_phy_write_reg(phy_id, reg, data, ext_bit);
}

void ethsw_phy_write_reg(int phy_id, int reg, uint16 *data, int ext_bit)
{
    uint32 reg_value;
    int    is6829 = 0, in_interrupt = 0;

#if defined(CONFIG_BCM96816)
    if ( IsExt6829(phy_id) )
    {
       is6829  = 1;
    }
#endif
    phy_id &= BCM_PHY_ID_M;

    reg_value = 0;

    if (in_interrupt()) {
        in_interrupt = 1;
    }

    /* Disable Phy interrupt so Phy ISR will not enter this function while a transaction is in progress. Will need similar protection if there is ever a need
           to access phy in another interrupt context (like the packet rx isr (napi)). We can use spin_lock_irqsave but the phy access takes >50uS and
           it disables all the interrupts on the local CPU(not just the Phy interrupt) */
    if (!in_interrupt) {
        BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
        BcmHalInterruptDisable(INTERRUPT_ID_GPHY);
#endif
    }
    atomic_inc(&phy_write_ref_cnt);
    spin_lock(&bcm_ethlock_phy_access);

    ethsw_wreg_ext(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&reg_value, 4, is6829);
    reg_value = (ext_bit? REG_MDIO_CTRL_EXT: 0) | (REG_MDIO_CTRL_WRITE) |
        ((phy_id << REG_MDIO_CTRL_ID_SHIFT) & REG_MDIO_CTRL_ID_MASK) |
        (reg  << REG_MDIO_CTRL_ADDR_SHIFT) | *data;
    ethsw_wreg_ext(PAGE_CONTROL, REG_MDIO_CTRL_ADDR, (uint8 *)&reg_value, 4, is6829);

    if (irqs_disabled() || (preempt_count() != 0)) {
        udelay(60);
    } else {
      msleep(1);
    }

    spin_unlock(&bcm_ethlock_phy_access);
    atomic_dec(&phy_write_ref_cnt);
    if (!in_interrupt) {
        if (atomic_read(&phy_write_ref_cnt) <= 0) {
            BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
            BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif
        }
    }
}

