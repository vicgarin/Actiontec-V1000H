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


#include "bcm_OS_Deps.h"
#include "board.h"
#include "spidevices.h"
#include <bcm_map.h>
#include "bcm_intr.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#include "bcmswdefs.h"
#include "bcmsw.h"
/* Exports for other drivers */
#include "bcmsw_api.h"
#include "bcmswshared.h"
#include "bcmPktDma_defines.h"
#include "pktCmf_public.h"
#include "boardparms.h"
#include "bcmenet.h"

#ifndef SINGLE_CHANNEL_TX
/* for enet driver txdma channel selection logic */
extern int channel_for_queue[NUM_EGRESS_QUEUES];
/* for enet driver txdma channel selection logic */
extern int use_tx_dma_channel_for_priority;
#endif /*SINGLE_CHANNEL_TX*/

extern struct semaphore bcm_ethlock_switch_config;
extern spinlock_t spl_lock;
/* The switch physical port to phyid mapping */
extern int switch_pport_phyid[TOTAL_SWITCH_PORTS];

/* Forward declarations */
void __ethsw_get_txrx_imp_port_pkts(void);

/****************************************/
/* Hardware Access Functions            */
/****************************************/

void ethsw_rreg_ext(int page, int reg, uint8 *data, int len, int is6829)
{
    unsigned long   addr = (SWITCH_BASE + (page << 8) + reg);
    volatile uint8 *base = (volatile uint8 *)addr;

    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("ethsw_rreg_ext: wrong length!\n");

    if ( 0 == is6829 )
    {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ethsw_phy_pll_up(0);
#endif

        if (len == 1)
        {
            *data = *base;
        }
        else if (len == 2)
        {
            *(uint16 *)(data + 0) = *(uint16 *)(base + 0);
        }
        else if (len == 4)
        {
            if ((reg % 4) == 0)
            {
                *(uint32 *)(data + 0) = *(uint32 *)(base + 0);
            }
            else
            {
                *(uint16 *)(data + 2) = *(uint16 *)(base + 0);
                *(uint16 *)(data + 0) = *(uint16 *)(base + 2);
            }
        }
        else if (len == 6)
        {
            if ((reg % 4) == 0)
            {
                *(uint32 *)(data + 2) = *(uint32 *)(base + 0);
                *(uint16 *)(data + 0) = *(uint16 *)(base + 4);
            }
            else
            {
                *(uint16 *)(data + 4) = *(uint16 *)(base + 0);
                *(uint32 *)(data + 0) = *(uint32 *)(base + 2);
            }
        }
        else if (len == 8)
        {
            *(uint32 *)(data + 4) = *(uint32 *)(base + 0);
            *(uint32 *)(data + 0) = *(uint32 *)(base + 4);
        }
    }
#if defined(CONFIG_BCM96816)
    else
    {
        unsigned long spiData;
        if (1 == len)
        {
            kerSysBcmSpiSlaveRead(addr, &spiData, 1);
            *data = (char)spiData;
        }
        else if (len == 2)
        {
            kerSysBcmSpiSlaveRead(addr, &spiData, 2);
            *(uint16 *)(data + 0) = (uint16)spiData;
        }
        else if (len == 4)
        {
            if ((reg % 4) == 0)
            {
               kerSysBcmSpiSlaveRead(addr, &spiData, 4);
               *(uint32 *)(data + 0) = (uint32)spiData;
            }
            else
            {
                kerSysBcmSpiSlaveRead(addr, &spiData, 2);
                *(uint16 *)(data + 2) = (uint16)spiData;
                kerSysBcmSpiSlaveRead(addr+2, &spiData, 2);
                *(uint16 *)(data + 0) = (uint16)spiData;
            }
        }
        else if (len == 6)
        {
            if ((reg % 4) == 0)
            {
                kerSysBcmSpiSlaveRead(addr, &spiData, 4);
                *(uint32 *)(data + 2) = (uint32)spiData;
                kerSysBcmSpiSlaveRead(addr+4, &spiData, 2);
                *(uint16 *)(data + 0) = (uint16)spiData;
            }
            else
            {
                kerSysBcmSpiSlaveRead(addr, &spiData, 2);
                *(uint16 *)(data + 4) = (uint16)spiData;
                kerSysBcmSpiSlaveRead(addr+2, &spiData, 4);
                *(uint32 *)(data + 0) = (uint32)spiData;
            }
        }
        else if (len == 8)
        {
            kerSysBcmSpiSlaveRead(addr, &spiData, 4);
            *(uint32 *)(data + 4) = (uint32)spiData;
            kerSysBcmSpiSlaveRead(addr+4, &spiData, 4);
            *(uint32 *)(data + 0) = (uint32)spiData;
        }
    }
#else
    else
    {
        panic("ethsw_rreg_ext: 6829 not availble\n");
    }
#endif
}

void ethsw_wreg_ext(int page, int reg, uint8 *data, int len, int is6829)
{
    unsigned long   addr = (SWITCH_BASE + (page << 8) + reg);
    volatile uint8 *base = (volatile uint8 *)addr;

    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("ethsw_wreg_ext: wrong length!\n");

    if ( 0 == is6829 )
    {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ethsw_phy_pll_up(0);
#endif

        if (len == 1)
        {
            *base = *data;
        }
        else if (len == 2)
        {
            *(uint16 *)base = *(uint16 *)data;
        }
        else if (len == 4)
        {
            if ((reg % 4) == 0)
            {
                if ( (int)data & 3 )
                {
                    *(uint32 *)(base + 0) = ((*(uint16 *)(data + 0) << 16) | 
                                             (*(uint16 *)(data + 2) <<  0));
                }
                else
                {
                   *(uint32 *)(base + 0) = *(uint32 *)(data + 0);
                }
            }
            else
            {
                *(uint16 *)(base + 0) = *(uint16 *)(data + 2);
                *(uint16 *)(base + 2) = *(uint16 *)(data + 0);
            }
        }
        else if (len == 6)
        {
            if (reg % 4 == 0)
            {
                if ( (int)(data + 2) & 3 )
                {
                    *(uint32 *)(base + 0) = ((*(uint16 *)(data + 2) << 16) | 
                                             (*(uint16 *)(data + 4) <<  0));
                }
                else
                {
                   *(uint32 *)(base + 0) = *(uint32 *)(data + 2);
                }
                *(uint16 *)(base + 4) = *(uint16 *)(data + 0);
            }
            else
            {
                *(uint16 *)(base + 0) = *(uint16 *)(data + 4);
                if ( (int)(data + 0) & 3 )
                {
                    *(uint32 *)(base + 2) = ((*(uint16 *)(data + 0) << 16) | 
                                             (*(uint16 *)(data + 2) <<  0));
                }
                else
                {
                   *(uint32 *)(base + 2) = *(uint32 *)(data + 0);
                }
            }
        }
        else if (len == 8)
        {
            if ( (int)(data + 0) & 3 )
            {
                *(uint32 *)(base + 0) = ((*(uint16 *)(data + 4) << 16) | 
                                         (*(uint16 *)(data + 6) <<  0));
                *(uint32 *)(base + 4) = ((*(uint16 *)(data + 0) << 16) | 
                                         (*(uint16 *)(data + 2) <<  0));
            }
            else
            {
                *(uint32 *)(base + 0) = *(uint32 *)(data + 4);
                *(uint32 *)(base + 4) = *(uint32 *)(data + 0);
            }
        }
    }
#if defined(CONFIG_BCM96816)
    else
    {
        unsigned long spiData;
        if (len == 1)
        {
            spiData = (unsigned long)*data;
            kerSysBcmSpiSlaveWrite(addr, spiData, 1);
        }
        else if (len == 2)
        {
            spiData = (unsigned long)*(uint16 *)data;
            kerSysBcmSpiSlaveWrite(addr, spiData, 2);
        }
        else if (len == 4)
        {
            if ((reg % 4) == 0)
            {
                spiData = (unsigned long)*(uint32 *)(data + 0);
                kerSysBcmSpiSlaveWrite(addr, spiData, 4);
            }
            else
            {
                spiData = (unsigned long)*(uint16 *)(data + 2);
                kerSysBcmSpiSlaveWrite(addr, spiData, 2);
                spiData = (unsigned long)*(uint16 *)(data + 0);
                kerSysBcmSpiSlaveWrite(addr+2, spiData, 2);
            }
        }
        else if (len == 6)
        {
            if (reg % 4 == 0)
            {
                spiData = (unsigned long)*(uint32 *)(data + 2);
                kerSysBcmSpiSlaveWrite(addr, spiData, 4);
                spiData = (unsigned long)*(uint16 *)(data + 0);
                kerSysBcmSpiSlaveWrite(addr+4, spiData, 2);
            }
            else
            {
                spiData = (unsigned long)*(uint16 *)(data + 4);
                kerSysBcmSpiSlaveWrite(addr, spiData, 2);
                spiData = (unsigned long)*(uint32 *)(data + 0);
                kerSysBcmSpiSlaveWrite(addr+2, spiData, 4);
            }
        }
        else if (len == 8)
        {
            spiData = (unsigned long)*(uint32 *)(data + 4);
            kerSysBcmSpiSlaveWrite(addr, spiData, 4);
            spiData = (unsigned long)*(uint32 *)(data + 0);
            kerSysBcmSpiSlaveWrite(addr+4, spiData, 4);
        }
    }
#else
    else
    {
        panic("ethsw_wreg_ext: 6829 not availble\n");
    }
#endif
}

/*******************/
/* Local functions */
/*******************/
static void fast_age_start_done(uint8_t ctrl) {
    uint8_t timeout = 100;

    ethsw_wreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    ethsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    while (ctrl & FAST_AGE_START_DONE) {
        mdelay(1);
        ethsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }
}

void fast_age_all(uint8_t age_static) {
    uint8_t v8;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC;
  if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }

    fast_age_start_done(v8);
}

void fast_age_port(uint8_t port, uint8_t age_static) {
    uint8_t v8;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }

    ethsw_wreg(PAGE_CONTROL, REG_FAST_AGING_PORT, &port, 1);
    fast_age_start_done(v8);
}

static int read_vlan_table(bcm_vlan_t vid, uint32_t *val32)
{
    uint8_t val8;
    int i, timeout = 200;

    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX, (uint8_t *)&vid, 2);
    val8 = 0x81;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);
        if (((val8) & 0x80) == 0) {
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY,
                       (uint8_t *)val32, 4);
            return 0;
        }
        udelay(100);
    }

    printk("Timeout reading VLAN table \n");
    return BCM_E_ERROR;
}

static int write_vlan_table(bcm_vlan_t vid, uint32_t val32)
{
    uint8_t val8;
    int i, timeout = 200;

    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX, (uint8_t *)&vid, 2);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY, (uint8_t *)&val32, 4);
    val8 = 0x80;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);
        if (((val8) & 0x80) == 0) {
            return 0;
        }
        udelay(100);
    }

    printk("Timeout writing to VLAN table \n");
    return BCM_E_ERROR;
}

static void restart_autoneg(int phyid)
{
    uint16_t v16;

    /* read control register */
    ethsw_phy_rreg(phyid, MII_BMCR, &v16);
    BCM_ENET_DEBUG("MII_BMCR Read Value = %4x", v16);

    /* Write control register wth AN_EN and RESTART_AN bits set */
    v16 |= (BMCR_ANENABLE | BMCR_ANRESTART);
    BCM_ENET_DEBUG("MII_BMCR Written Value = %4x", v16);
    ethsw_phy_wreg(phyid, MII_BMCR, &v16);
}

static void set_pause_capability(int port, int req_flow_ctrl)
{
    uint16_t an_adv, v16, bmcr;
    uint32_t override_val;
    int phyid = ETHSW_PHY_GET_PHYID(port);

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG("given req_flow_ctrl = %4x", req_flow_ctrl);
    ethsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&override_val, 4);
    BCM_ENET_DEBUG("override_val read = %4x", (unsigned int)override_val);
    override_val &= (~((1 << port) | (1 << (port + TOTAL_SWITCH_PORTS))));
    /* resolve pause mode and advertisement
     * Please refer to Table 28B-3 of the 802.3ab-1999 spec */
    switch (req_flow_ctrl) {
        case PAUSE_FLOW_CTRL_AUTO:
        case PAUSE_FLOW_CTRL_BOTH:
            v16 = (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
            override_val |= ((1 << port) | (1 << (port +TOTAL_SWITCH_PORTS)));
            break;

        case PAUSE_FLOW_CTRL_TX:
        v16 = ADVERTISE_PAUSE_ASYM;
        override_val |= (1 << port);
        break;

      case PAUSE_FLOW_CTRL_RX:
//          an_adv |= (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
        v16 = ADVERTISE_PAUSE_CAP;
        override_val |= (1 << (port +TOTAL_SWITCH_PORTS));
        break;

      case PAUSE_FLOW_CTRL_NONE:
      default:
            v16 = 0;
        break;
  }

    if ((port < EPHY_PORTS) && (phyid != -1)) {
        ethsw_phy_rreg(ETHSW_PHY_GET_PHYID(port), MII_BMCR, &bmcr);
        if (bmcr & BMCR_ANENABLE) {
            ethsw_phy_rreg(phyid, MII_ADVERTISE, &an_adv);
            BCM_ENET_DEBUG("an_adv read from PHY = %4x", an_adv);
        an_adv &= ~(ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
            an_adv |= v16;
            BCM_ENET_DEBUG("an_adv written to PHY = %4x", an_adv);
            ethsw_phy_wreg(phyid, MII_ADVERTISE, &an_adv);
            restart_autoneg(phyid);
        } else {
            override_val |= REG_PAUSE_CAPBILITY_OVERRIDE;
        }
    } else {
        override_val |= REG_PAUSE_CAPBILITY_OVERRIDE;
    }
    BCM_ENET_DEBUG("val written to REG_PAUSE_CAPABILITY = %4x",
                   (unsigned int)override_val);
    ethsw_wreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&override_val, 4);

    up(&bcm_ethlock_switch_config);

}

static void get_pause_capability(int port, int *flow_ctrl)
{
    uint16_t an_adv, v16;
    uint32_t val;
    int phyid = ETHSW_PHY_GET_PHYID(port);

    down(&bcm_ethlock_switch_config);

    ethsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&val, 4);
    if (val & REG_PAUSE_CAPBILITY_OVERRIDE) {
    if ((val & (1 << port)) && (val & (1 << (port +TOTAL_SWITCH_PORTS)))) {
        *flow_ctrl = PAUSE_FLOW_CTRL_BOTH;
        } else if (val & (1 << port)) {
        *flow_ctrl = PAUSE_FLOW_CTRL_TX;
        } else if (val & (1 << (port + TOTAL_SWITCH_PORTS))) {
        *flow_ctrl = PAUSE_FLOW_CTRL_RX;
        } else {
        *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
        }
    } else if ((port < EPHY_PORTS) && (phyid != -1)) {
        ethsw_phy_rreg(phyid, MII_BMCR, &v16);
        if (v16 & BMCR_ANENABLE) {
            /*  Read ANAR */
            ethsw_phy_rreg(phyid, MII_ADVERTISE, &an_adv);
            BCM_ENET_DEBUG("an_adv = %4x", an_adv);

          switch (an_adv & (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM)) {
                case (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM):
                *flow_ctrl = PAUSE_FLOW_CTRL_BOTH;
                break;

                case ADVERTISE_PAUSE_ASYM:
                    *flow_ctrl = PAUSE_FLOW_CTRL_TX;
                    break;

                case ADVERTISE_PAUSE_CAP:
                    *flow_ctrl = PAUSE_FLOW_CTRL_RX;
                    break;

                default:
                    *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
                    break;
          }
        } else {
            *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
        }
    } else {
        *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
    }
    BCM_ENET_DEBUG("*flow_ctrl = %4x", *flow_ctrl);

    up(&bcm_ethlock_switch_config);
}

/****************************************/
/* Ethernet Switch APIs For Enet Driver */
/****************************************/
void ethsw_port_based_vlan(int port_map, int wan_port_map, int softSwitchingMap)
{
    uint16 value;
    int    port;
    int    ageMap;

    down(&bcm_ethlock_switch_config);

    /* If wan port, forward to IMP. Else clear the bits of wan ports. */
    for (port = 0; port < TOTAL_SWITCH_PORTS; port++)
    {
        /* if the port is enabled in port_map, then change the reg */
        if (port_map & (1 << port))
        {
            if (wan_port_map & (1 << port))
            {
#if defined(CONFIG_BCM96368)
                /* For 6368, we use mirroring to forward to cpu */
                value = 0;
#else
                /* The port is wan port, set forward to cpu */
                value = PBMAP_MIPS;
#endif
            }
            else
            {
                value = DEFAULT_PBVLAN_MAP & (~wan_port_map) & (~softSwitchingMap);
            }

            /* set to register */
            ethsw_wreg(PAGE_PORT_BASED_VLAN, (port * 2), (uint8*)&value, 2);
        }
    }

    value = wan_port_map;
#if defined(CONFIG_BCM96368)
    /* For 6368, we use mirroring to forward to cpu */
    ethsw_wreg(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, (uint8 *)&value, 2);
#endif
#ifdef CONFIG_BCM96816
    value |= PBMAP_MIPS_N_GPON;
#else
    value |= PBMAP_MIPS;
#endif
    value |= softSwitchingMap;
    ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&value, 2);

    ageMap = wan_port_map | softSwitchingMap;
    for(port = 0; port < TOTAL_SWITCH_PORTS; port++)
    {
        if((ageMap >> port) & 0x1)
        {
            fast_age_port(port, 1);
        }
    }

    up(&bcm_ethlock_switch_config);
}

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
void ethsw_set_wanoe_portmap(uint16 wan_port_map)
{
    int i;
    uint16 map;
    BCM_ENET_DEBUG("wanoe port map = 0x%x", wan_port_map);

    down(&bcm_ethlock_switch_config);

    /* Set WANoE port map */
    map = wan_port_map | EN_MAN_TO_WAN;
    ethsw_wreg(PAGE_CONTROL, REG_WAN_PORT_MAP, (uint8 *)&map, 2);

    map = PBMAP_MIPS | wan_port_map;
    /* Disable learning */
    ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&map, 2);

    for(i=0; i < TOTAL_SWITCH_PORTS; i++) {
       if((wan_port_map >> i) & 0x1) {
            fast_age_port(i, 1);
       }
    }

    up(&bcm_ethlock_switch_config);
}
#endif

/************************/
/* Ethernet Switch APIs */
/************************/
#if defined(CONFIG_BCM96816)
int enet_ioctl_ethsw_port_tagreplace(struct ethswctl_data *e)
{
    uint32_t val32;

    BCM_ENET_DEBUG("Port: %d \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    ethsw_rreg(PAGE_QOS, REG_QOS_VID_REMAP + (e->port * 4), (uint8_t *)&val32, 4);
    BCM_ENET_DEBUG("REG_QOS_VID_REMAP Read Val = 0x%08x", (unsigned int)val32);
    if (e->type == TYPE_GET) {
        if (copy_to_user((void*)(&e->vlan_tag), (void*)&val32, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
    } else {
        switch (e->replace_type) {
            case REPLACE_VLAN_TAG:
                val32 = e->vlan_tag;
                BCM_ENET_DEBUG("Given VLAN Tag: 0x%08x \n ", e->vlan_tag);
                break;
            case REPLACE_VLAN_TPID:
                BCM_ENET_DEBUG("Given TPID: 0x%04x \n ", e->vlan_param);
                val32 &= (~(BCM_NET_VLAN_TPID_M << BCM_NET_VLAN_TPID_S));
                val32 |= ((e->vlan_param & BCM_NET_VLAN_TPID_M)
                           << BCM_NET_VLAN_TPID_S);
                break;
            case REPLACE_VLAN_TCI:
                BCM_ENET_DEBUG("Given TCI: 0x%04x \n ", e->vlan_param);
                val32 &= (~(BCM_NET_VLAN_TCI_M << BCM_NET_VLAN_TCI_S));
                val32 |= ((e->vlan_param & BCM_NET_VLAN_TCI_M)
                           << BCM_NET_VLAN_TCI_S);
                break;
            case REPLACE_VLAN_VID:
                BCM_ENET_DEBUG("Given VID: 0x%04x \n ", e->vlan_param);
                val32 &= (~(BCM_NET_VLAN_VID_M << BCM_NET_VLAN_VID_S));
                val32 |= ((e->vlan_param & BCM_NET_VLAN_VID_M)
                           << BCM_NET_VLAN_VID_S);
                break;
            case REPLACE_VLAN_8021P:
                BCM_ENET_DEBUG("Given 8021P: 0x%04x \n ", e->vlan_param);
                val32 &= (~(BCM_NET_VLAN_8021P_M << BCM_NET_VLAN_8021P_S));
                val32 |= ((e->vlan_param & BCM_NET_VLAN_8021P_M)
                           << BCM_NET_VLAN_8021P_S);
                break;
            case REPLACE_VLAN_CFI:
                BCM_ENET_DEBUG("Given CFI: 0x%04x \n ", e->vlan_param);
                val32 &= (~(BCM_NET_VLAN_CFI_M << BCM_NET_VLAN_CFI_S));
                val32 |= ((e->vlan_param & BCM_NET_VLAN_CFI_M)
                           << BCM_NET_VLAN_CFI_S);
                break;
            default:
                break;
        }
        BCM_ENET_DEBUG("REG_QOS_VID_REMAP Val Written = 0x%08x",
                       (unsigned int)val32);
        ethsw_wreg(PAGE_QOS, REG_QOS_VID_REMAP + (e->port * 4),
                   (uint8_t *)&val32, 4);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int enet_ioctl_ethsw_port_tagmangle(struct ethswctl_data *e)
{
    uint32_t val32;
    uint16_t val16;

    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        /* Read of REG_QOS_VID_REMAP_CTRL is broken. So, read from SW copy
           of these registers */
        /* ethsw_rreg(PAGE_QOS, REG_QOS_VID_REMAP_CTRL + (e->port * 2),
                   (uint8_t *)&val32, 2); */
        val32 = (uint32_t)vid_remap_ctrl_regs[e->port];
        BCM_ENET_DEBUG("QoS_VID_REMAP_CTRL Read Val: 0x%04x \n ",
                       (uint16_t)val32);
        if (copy_to_user((void*)(&e->op_map), (void*)&val32, sizeof(int))) {
            return -EFAULT;
        }
    } else {
        /* Read of REG_QOS_VID_REMAP_CTRL is broken. So, read from SW copy
           of these registers */
        val16 = vid_remap_ctrl_regs[e->port];
        val16 &= BCM_NET_VLAN_VID_M;
        val16 |= (e->op_map & 0xF000);
        down(&bcm_ethlock_switch_config);
        ethsw_wreg(PAGE_QOS, REG_QOS_VID_REMAP_CTRL + (e->port * 2),
                   (uint8_t *)&val16, 2);
        up(&bcm_ethlock_switch_config);
        vid_remap_ctrl_regs[e->port] = val16;
        BCM_ENET_DEBUG("QoS_VID_REMAP_CTRL is set to: 0x%04x \n ",
                       (uint16_t)val16);
    }

    return 0;
}

int enet_ioctl_ethsw_port_tagmangle_matchvid(struct ethswctl_data *e)
{
    uint32_t val32;
    uint16_t val16;

    BCM_ENET_DEBUG("Given Port: 0x%02x \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        /* Read of REG_QOS_VID_REMAP_CTRL is broken. So, read from SW copy
           of these registers */
        val32 = (uint32_t)vid_remap_ctrl_regs[e->port];
        BCM_ENET_DEBUG("QoS_VID_REMAP_CTRL Read Val: 0x%04x \n ",
                       (uint16_t)val32);
        val32 &= BCM_NET_VLAN_VID_M;
        if (copy_to_user((void*)(&e->vlan_param), (void*)&val32, sizeof(int))) {
            return -EFAULT;
        }
    } else {
        /* Read of REG_QOS_VID_REMAP_CTRL is broken. So, read from SW copy
           of these registers */
        BCM_ENET_DEBUG("Given Match VID: 0x%04x \n ", e->vlan_param);
        val16 = vid_remap_ctrl_regs[e->port];
        val16 &= ~BCM_NET_VLAN_VID_M;
        val16 |= (e->vlan_param & BCM_NET_VLAN_VID_M);
        down(&bcm_ethlock_switch_config);
        ethsw_wreg(PAGE_QOS, REG_QOS_VID_REMAP_CTRL + (e->port * 2),
                   (uint8_t *)&val16, 2);
        up(&bcm_ethlock_switch_config);
        vid_remap_ctrl_regs[e->port] = val16;
        BCM_ENET_DEBUG("QoS_VID_REMAP_CTRL is set to: 0x%04x \n ",
                       (uint16_t)val16);
    }

    return 0;
}

#define VID_FFF_EN 0x04
int enet_ioctl_ethsw_port_tagstrip(struct ethswctl_data *e)
{
    uint32_t val32, tag_strip_status;
    uint8_t val8;
    bcm_vlan_t vid;

    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    /* Read of REG_QOS_VID_REMAP_CTRL is broken. So, read from SW copy
       of these registers */
    val32 = (uint32_t)vid_remap_ctrl_regs[e->port];
    vid = val32 & BCM_NET_VLAN_VID_M;
    if (read_vlan_table(vid, &val32)) {
        up(&bcm_ethlock_switch_config);
        printk("VLAN Table Read Failed \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        /* If given port is a member, find the tag_strip status;
           otherwise return error */
        if (val32 & (1 << e->port)) {
            if (val32 & (1 << (e->port + TOTAL_SWITCH_PORTS))) {
                tag_strip_status = TRUE;
            } else {
                tag_strip_status = FALSE;
            }
            if (copy_to_user((void*)(&e->ret_val), (void*)&tag_strip_status,
                sizeof(int))) {
                up(&bcm_ethlock_switch_config);
                return -EFAULT;
            }
        } else {
            printk("Given port is not a member of match_vid <%0x04x> \n", vid);
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
    } else {
        /* If given port is a member, set the tag_strip status;
           otherwise return error */
        if (val32 & (1 << e->port)) {
            if (e->val) {
                /* Request for tag_strip enable; so if the given port is not
                   in the untagged bitmap, put it in untagged bitmap and update
                   the VLAN table */
                if (!(val32 & (1 << (e->port + TOTAL_SWITCH_PORTS)))) {
                    val32 |= 1 << (e->port + TOTAL_SWITCH_PORTS);
                    if (write_vlan_table(vid, val32)) {
                        up(&bcm_ethlock_switch_config);
                        printk("VLAN Table Write Failed \n");
                        return BCM_E_ERROR;
                    }
                }
            } else {
                /* Request for tag_strip disable; so if the given port is in
                   the untagged bitmap, remove it and update the VLAN table */
                if (val32 & (1 << (e->port + TOTAL_SWITCH_PORTS))) {
                    /* Make sure given port is not in the untag map */
                    val32 &= (~(1 << (e->port + TOTAL_SWITCH_PORTS)));
                    if (write_vlan_table(vid, val32)) {
                        up(&bcm_ethlock_switch_config);
                        printk("VLAN Table Write Failed \n");
                        return BCM_E_ERROR;
                    }
                }
            }
        } else {
            printk("Given port is not a member of match_vid <%d> \n", vid);
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }

        if (vid == 0xFFF) {
            /* Make sure VID_FFF_EN (bit-2 of 0x3407) is set */
            ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5,
                       (uint8_t *)&val8, 1);
            val8 |= VID_FFF_EN;
            ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5,
                       (uint8_t *)&val8, 1);
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
#endif // #if 6816

int enet_ioctl_ethsw_port_pause_capability(struct ethswctl_data *e)
{
    int val = 0;

    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        get_pause_capability(e->port, &val);
        BCM_ENET_DEBUG("flow_ctrl = %4x", val);
        if (copy_to_user((void*)(&e->ret_val), (void*)&val, sizeof(int))) {
            return -EFAULT;
        }
    } else {
        BCM_ENET_DEBUG("e->val = %4x", e->val);
        set_pause_capability(e->port, e->val);
    }

    return 0;
}

int enet_ioctl_ethsw_control(struct ethswctl_data *e)
{
    uint16_t val16;
    uint8_t val8;
    unsigned int val;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        switch (e->sw_ctrl_type) {
            case bcmSwitchBufferControl:
                ethsw_rreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8_t *)&val16, 2);
                BCM_ENET_DEBUG("FC_CTRL = %4x", val16);
                val = val16 & QOS_PAUSE_DROP_EN_MAP;
                BCM_ENET_DEBUG("FC_CTRL & MASK = %4x", val);
                if (copy_to_user((void*)(&e->val), (void*)&val,
                    sizeof(uint32_t))) {
                    up(&bcm_ethlock_switch_config);
                    return -EFAULT;
                }
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
                break;

            case bcmSwitch8021QControl:
                /* Read the 802.1Q control register */
                ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
                val = (val8 >> VLAN_EN_8021Q_S) & VLAN_EN_8021Q_M;
                if (copy_to_user((void*)(&e->val), (void*)&val,
                    sizeof(uint32_t))) {
                    up(&bcm_ethlock_switch_config);
                    return -EFAULT;
                }
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
                break;

            default:
                up(&bcm_ethlock_switch_config);
                return BCM_E_PARAM;
                break;
        }
    } else {
        switch (e->sw_ctrl_type) {
            case bcmSwitchBufferControl:
                /* Read the Pause/Drop control register */
                ethsw_rreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8_t *)&val16, 2);
                /* Modify the Pause/Drop control register as requested*/
                val16 &= ~QOS_PAUSE_DROP_EN_MAP;
                val16 |= (e->val & QOS_PAUSE_DROP_EN_MAP);
                BCM_ENET_DEBUG("Write FC_CTRL = %4x", val16);
                ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8_t *)&val16, 2);
                break;

            case bcmSwitch8021QControl:
                /* Read the 802.1Q control register */
                ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
                /* Enable/Disable the 802.1Q */
                val8 &= (~(VLAN_EN_8021Q_M << VLAN_EN_8021Q_S));
                val8 |= ((e->val & VLAN_EN_8021Q_M) << VLAN_EN_8021Q_S);
                ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
                break;

            default:
                up(&bcm_ethlock_switch_config);
                return BCM_E_PARAM;
                break;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int enet_ioctl_ethsw_prio_control(struct ethswctl_data *e)
{
    uint32_t val32;
    uint16_t val16;
    int reg;

    BCM_ENET_DEBUG("e->priority = %2d", e->priority);
    if (e->priority > MAX_PRIORITY_VALUE) {
        printk("Invalid Priority \n");
        return BCM_E_ERROR;
    }

    /* */
    switch (e->sw_ctrl_type) {
        case bcmSwitchTxQHiHysteresisThreshold:
            reg = REG_FC_PRIQ_HYST + (e->priority * 2);
            break;
        case bcmSwitchTxQHiPauseThreshold:
            reg = REG_FC_PRIQ_PAUSE + (e->priority * 2);
            break;
        case bcmSwitchTxQHiDropThreshold:
            reg = REG_FC_PRIQ_DROP + (e->priority * 2);
            break;
#if defined(CONFIG_BCM96816)
        case bcmSwitchTxQLowDropThreshold:
            reg = REG_FC_PRIQ_LO_DROP + (e->priority * 2);
            break;
#endif
        case bcmSwitchTotalHysteresisThreshold:
            reg = REG_FC_PRIQ_TOTAL_HYST + (e->priority * 2);
            break;
        case bcmSwitchTotalPauseThreshold:
            reg = REG_FC_PRIQ_TOTAL_PAUSE + (e->priority * 2);
            break;
        case bcmSwitchTotalDropThreshold:
            reg = REG_FC_PRIQ_TOTAL_DROP + (e->priority * 2);
            break;
        default:
            printk("unknown threshold type \n");
            return BCM_E_ERROR;
    }

    BCM_ENET_DEBUG("Threshold register offset = %4x", reg);

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_FLOW_CTRL, reg, (uint8_t *)&val16, 2);
        BCM_ENET_DEBUG("Threshold read = %4x", val16);
        val32 = val16;
        if (copy_to_user((void*)(&e->ret_val), (void*)&val32, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
    } else {
       val16 = (uint32_t)e->val;
       BCM_ENET_DEBUG("e->val is = %4x", e->val);
       ethsw_wreg(PAGE_FLOW_CTRL, reg, (uint8_t *)&val16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int enet_ioctl_ethsw_vlan(struct ethswctl_data *e)
{
    bcm_vlan_t vid;
    uint32_t val32, tmp;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        vid = e->vid & BCM_NET_VLAN_VID_M;
        if (read_vlan_table(vid, &val32)) {
            up(&bcm_ethlock_switch_config);
            printk("VLAN Table Read Failed \n");
            return BCM_E_ERROR;
        }
        BCM_ENET_DEBUG("Combined fwd and untag map: 0x%08x \n",
                       (unsigned int)val32);
        tmp = val32 & VLAN_FWD_MAP_M;
        if (copy_to_user((void*)(&e->fwd_map), (void*)&tmp, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        tmp = (val32 >> VLAN_UNTAG_MAP_S) & VLAN_UNTAG_MAP_M;
        if (copy_to_user((void*)(&e->untag_map), (void*)&tmp, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
    } else {
        vid = e->vid & BCM_NET_VLAN_VID_M;
        val32 = e->fwd_map | (e->untag_map << TOTAL_SWITCH_PORTS);
        BCM_ENET_DEBUG("VLAN_ID = %4d; fwd_map = 0x%04x; ", vid, e->fwd_map);
        BCM_ENET_DEBUG("untag_map = 0x%04x \n", e->untag_map);
        if (write_vlan_table(vid, val32)) {
            up(&bcm_ethlock_switch_config);
            printk("VLAN Table Write Failed \n");
            return BCM_E_ERROR;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int enet_ioctl_ethsw_pbvlan(struct ethswctl_data *e)
{
    uint32_t val32;
    uint16_t val16;

    BCM_ENET_DEBUG("Given Port: 0x%02x \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
        BCM_ENET_DEBUG("Threshold read = %4x", val16);
        val32 = val16;
        if (copy_to_user((void*)(&e->fwd_map), (void*)&val32, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
    } else {
        val16 = (uint32_t)e->fwd_map;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
        ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

static int robo_ingress_rate_init_flag = 0;
#define IRC_PKT_MASK    0x3f
/*
 *  Function : enet_ioctl_ethsw_port_irc_set
 *
 *  Purpose :
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit   :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1).
 *
 */
int enet_ioctl_ethsw_port_irc_set(struct ethswctl_data *e)
{
    uint32_t  reg_value, burst_kbyte = 0, temp = 0;
    int rv = BCM_E_NONE;

    down(&bcm_ethlock_switch_config);

    /* Enable XLEN_EN bit to include IPG for rate limiting */
    ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&reg_value, 4);
    reg_value |= (1 << IRC_CFG_XLENEN);
    ethsw_wreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&reg_value, 4);

    /* Read the current Ingress Rate Config of the given port */
    ethsw_rreg(PAGE_BSS, REG_BSS_RX_RATE_CTRL_P0 + (e->port * 4),
               (uint8_t *)&reg_value, 4);

    if (e->limit == 0) { /* Disable ingress rate control */
         /* Disable ingress rate control
          *    - ING_RC_ENf can't be set as 0, it will stop this port's storm
          *       control rate also.
          *    - to prevent the affecting on other ports' ingress rate cotrol,
          *       global ingress rate setting is not allowed been modified on
          *       trying to disable this port's ingress rate control also.
          *    - set the REF_CNT to the MAX value means packets could
          *       be forwarded by no limit rate. (set to 0 will block all this
          *       port's traffic)
          */
         reg_value &= ~(IRC_BKT0_RATE_CNT_M << IRC_BKT0_RATE_CNT_S);
         reg_value |= 254 << IRC_BKT0_RATE_CNT_S;
    } else {    /* Enable ingress rate control */
        /* if not, config the pkt_type_mask for ingress rate control */
        if (!robo_ingress_rate_init_flag) {
            ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&temp, 4);
            /* Enable bucket0 rate limiting for all types of traffic */
            temp |= IRC_PKT_MASK;
            /* Extended packet mask: SA lookup fail */
            temp |= 1 << IRC_CFG_PKT_MSK0_EXT_S;
            ethsw_wreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&temp, 4);
            robo_ingress_rate_init_flag = 1;
        }

        burst_kbyte = e->burst_size / 8;
        if (e->burst_size > (500 * 8)) { /* 500 KB */
            return BCM_E_PARAM;
        }
        if (burst_kbyte <= 16) { /* 16KB */
            temp = 0;
        } else if (burst_kbyte <= 20) { /* 20KB */
            temp = 1;
        } else if (burst_kbyte <= 28) { /* 28KB */
            temp = 2;
        } else if (burst_kbyte <= 40) { /* 40KB */
            temp = 3;
        } else if (burst_kbyte <= 76) { /* 76KB */
            temp = 4;
        } else if (burst_kbyte <= 140){ /* 140KB */
            temp = 5;
        } else if (burst_kbyte <= 268){ /* 268KB */
            temp = 6;
        } else if (burst_kbyte <= 500){ /* 500KB */
            temp = 7;
        }

        reg_value &= ~(IRC_BKT0_SIZE_M << IRC_BKT0_SIZE_S);
        reg_value |= temp << IRC_BKT0_SIZE_S;

        /* refresh count  (fixed type)*/
        if (e->limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((e->limit-1) / 64) +1;
        } else if (e->limit <= 102400){ /* 2MB ~ 100MB */
            temp = (e->limit /1024 ) + 27;
        } else if (e->limit <= 1024000){ /* 104MB ~ 1000MB */
            temp = (e->limit /8192) + 115;
        } else {
            temp = 255;
        }

        /* Setting ingress rate
         *    - here we defined ingress rate control will be disable if
         *       REF_CNT=255. (means no rate control)
         *    - this definition is for seperate different rate between
         *       "Ingress rate control" and "Strom rate control"
         *    - thus if the gave limit value trasfer REF_CNT is 255, we reasign
         *       REF_CNT to be 254
         */
        temp = (temp == 255) ? 254 : temp;
        reg_value &= ~(IRC_BKT0_RATE_CNT_M << IRC_BKT0_RATE_CNT_S);
        reg_value |= (temp << IRC_BKT0_RATE_CNT_S);

        /* enable ingress rate control */
        reg_value &= ~(IRC_BKT0_EN_M << IRC_BKT0_EN_S);
        reg_value |= (1 << IRC_BKT0_EN_S);
    }
    /* write register */
    ethsw_wreg(PAGE_BSS, REG_BSS_RX_RATE_CTRL_P0 + (e->port * 4),
               (uint8_t *)&reg_value, 4);

    up(&bcm_ethlock_switch_config);
    return rv;
}

/*
 *  Function : enet_ioctl_ethsw_port_irc_get
 *
 *  Purpose :
 *   Get the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1).
 *
 */
int enet_ioctl_ethsw_port_irc_get(struct ethswctl_data *e)
{
    uint32_t  reg_value, temp;
    int     rv= BCM_E_NONE;

    down(&bcm_ethlock_switch_config);

    /* check global ingress rate control setting */
    if (robo_ingress_rate_init_flag) {
        ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&temp, 4);
        /* if pkt_type_mask is 0, then reset robo_ingress_rate_init_flag to 0
         * so that the next irc_set will set it correctly.
         */
        temp &= (IRC_PKT_MASK | (1 << IRC_CFG_PKT_MSK0_EXT_S));
        robo_ingress_rate_init_flag = (temp == 0) ? 0 : 1;
    }

    /* Check ingress rate control
      *    - ING_RC_ENf should not be 0 in the runtime except the system been
      *       process without ingress rate setting or user manuel config
      *       register value. It will stop this port's storm control rate also.
      *    - set the REF_CNT to the MAX value means packets could
      *       be forwarded by no limit rate. (set to 0 will block all this
      *       port's traffic)
      */
    ethsw_rreg(PAGE_BSS, REG_BSS_RX_RATE_CTRL_P0 + (e->port * 4),
               (uint8_t *)&reg_value, 4);
    temp = (reg_value >> IRC_BKT0_RATE_CNT_S) & IRC_BKT0_RATE_CNT_M;


    if (temp == 254) {
        e->limit = 0;
        e->burst_size = 0;
    } else {
        temp = (reg_value >> IRC_BKT0_SIZE_S) & IRC_BKT0_SIZE_M;
        switch (temp) {
            case 0:
                e->burst_size = 16 * 8; /* 16KB */
                break;
            case 1:
                e->burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                e->burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                e->burst_size = 40 * 8; /* 40KB */
                break;
            case 4:
                e->burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
                e->burst_size = 140 * 8; /* 140KB */
                break;
            case 6:
                e->burst_size = 268 * 8; /* 268KB */
                break;
            case 7:
                e->burst_size = 500 * 8; /* 500KB */
                break;

            default:
                up(&bcm_ethlock_switch_config);
                return BCM_E_INTERNAL;
        }

        temp = (reg_value >> IRC_BKT0_RATE_CNT_S) & IRC_BKT0_RATE_CNT_M;
        if (temp <= 28) {
            e->limit = temp * 64;
        } else if (temp <= 127) {
            e->limit = (temp -27) * 1024;
        } else if (temp <=243) {
            e->limit = (temp -115) * 1024 * 8;
        } else {
            rv = BCM_E_INTERNAL;
        }
    }

    up(&bcm_ethlock_switch_config);
    return rv;
}

/*
 *  Function : enet_ioctl_ethsw_port_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected port egress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int enet_ioctl_ethsw_port_erc_set(struct ethswctl_data *e)
{
    uint32_t  port_cfg_reg_value;
    uint16_t  rate_cfg_reg_value;
    uint32_t  temp = 0;
    uint32_t  burst_kbyte = 0;
    int     rv = BCM_E_NONE;

    down(&bcm_ethlock_switch_config);

    /* Enable XLEN_EN bit to include IPG for egress rate limiting */
    ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&port_cfg_reg_value, 4);
    port_cfg_reg_value |= (1 << IRC_CFG_XLENEN);
    ethsw_wreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&port_cfg_reg_value, 4);

    /* Read the current Egress Rate Config of the given port */
    ethsw_rreg(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + (e->port * 2),
               (uint8_t *)&rate_cfg_reg_value, 2);
    if (e->limit == 0) { /* Disable egress rate control */
        rate_cfg_reg_value &= ~(ERC_ERC_EN_M << ERC_ERC_EN_S);
    } else {    /* Enable egress rate control */
        /* burst size */
        burst_kbyte = e->burst_size / 8;
        if (e->burst_size > (500 * 8)) { /* 500 KB */
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        if (burst_kbyte <= 16) { /* 16KB */
            temp = 0;
        } else if (burst_kbyte <= 20) { /* 20KB */
            temp = 1;
        } else if (burst_kbyte <= 28) { /* 28KB */
            temp = 2;
        } else if (burst_kbyte <= 40) { /* 40KB */
            temp = 3;
        } else if (burst_kbyte <= 76) { /* 76KB */
            temp = 4;
        } else if (burst_kbyte <= 140) { /* 140KB */
            temp = 5;
        } else if (burst_kbyte <= 268) { /* 268KB */
            temp = 6;
        } else if (burst_kbyte <= 500) { /* 500KB */
            temp = 7;
        }

        rate_cfg_reg_value &= ~(ERC_BKT_SIZE_M << ERC_BKT_SIZE_S);
        rate_cfg_reg_value |= temp << ERC_BKT_SIZE_S;

        /* refresh count  (fixed type)*/
        if (e->limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((e->limit-1) / 64) +1;
        } else if (e->limit <= 102400){ /* 2MB ~ 100MB */
            temp = (e->limit /1024 ) + 27;
        } else { /* 104MB ~ 1000MB */
            temp = (e->limit /8192) + 115;
        }
        rate_cfg_reg_value &= ~(ERC_RFSH_CNT_M << ERC_RFSH_CNT_S);
        rate_cfg_reg_value |= temp << ERC_RFSH_CNT_S;

        /* enable egress rate control */
        rate_cfg_reg_value &= ~(ERC_ERC_EN_M << ERC_ERC_EN_S);
        rate_cfg_reg_value |= (1 << ERC_ERC_EN_S);
    }
    /* write register */
    ethsw_wreg(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + (e->port * 2),
               (uint8_t *)&rate_cfg_reg_value, 2);

    up(&bcm_ethlock_switch_config);
    return rv;
}

/*
 *  Function : enet_ioctl_ethsw_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int enet_ioctl_ethsw_port_erc_get(struct ethswctl_data *e)
{
    uint16_t reg_value;
    uint32_t temp;
    int     rv= BCM_E_NONE;

    ethsw_rreg(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + (e->port * 2),
               (uint8_t *)&reg_value, 2);
    temp = (reg_value >> ERC_ERC_EN_S) & ERC_ERC_EN_M;
    if (temp ==0) {
        e->limit = 0;
        e->burst_size = 0;
    } else {
        temp = (reg_value >> ERC_BKT_SIZE_S) & ERC_BKT_SIZE_M;
        switch (temp) {
            case 0:
                e->burst_size = 16 * 8; /* 16KB */
                break;
            case 1:
                e->burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                e->burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                e->burst_size = 40 * 8; /* 40KB */
                break;
            case 4:
                e->burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
                e->burst_size = 140 * 8; /* 140KB */
                break;
            case 6:
                e->burst_size = 268 * 8; /* 268KB */
                break;
            case 7:
                e->burst_size = 500 * 8; /* 500KB */
                break;
            default:
                return BCM_E_INTERNAL;
        }
        temp = (reg_value >> ERC_RFSH_CNT_S) & ERC_RFSH_CNT_M;
        if (temp <= 28) {
            e->limit = temp * 64;
        } else if (temp <= 127) {
            e->limit = (temp -27) * 1024;
        } else if (temp <=243) {
            e->limit = (temp -115) * 1024 * 8;
        } else {
            return BCM_E_INTERNAL;
        }
    }

    return rv;
}

int enet_ioctl_ethsw_cosq_config(struct ethswctl_data *e)
{
    uint8_t  val8;
    int val;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%2x", val8);
        /* If TXQ_MODE is non-zero, then we have multiple egress queues */
        if ((val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M) {
            val = NUM_EGRESS_QUEUES;
        } else {
            val = 1;
        }
        if (copy_to_user((void*)(&e->numq), (void*)&val, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->numq is = %2d", e->numq);
    } else {
        BCM_ENET_DEBUG("Given numq: 0x%02x \n ", e->numq);
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%02x", val8);
        if ((e->numq > 1) && (e->numq <= NUM_EGRESS_QUEUES)) {
            if ( ((val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M)
                != (e->numq - 1) ) {
                /* Set the number of queues to the given value */
                val8 &= (~(TXQ_CTRL_TXQ_MODE_M << TXQ_CTRL_TXQ_MODE_S));
                val8 |= (((e->numq - 1) & TXQ_CTRL_TXQ_MODE_M) <<
                         TXQ_CTRL_TXQ_MODE_S);
            }
        } else if (e->numq == 1){
            if ((val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M) {
                /* Set the number of queues to 1 */
                val8 &= (~(TXQ_CTRL_TXQ_MODE_M << TXQ_CTRL_TXQ_MODE_S));
            }
        } else {
            BCM_ENET_DEBUG("Invalid number of queues = %2d", e->numq);
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        BCM_ENET_DEBUG("Writing 0x%02x to REG_QOS_TXQ_CTRL", val8);
        ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

#define MAX_WRR_WEIGHT 0x31
int enet_ioctl_ethsw_cosq_sched(struct ethswctl_data *e)
{
    uint8_t  val8, txq_mode, hq_preempt;
    int i, val, sched;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%2x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M;
        if(txq_mode == 0) {
            BCM_ENET_DEBUG("Multiple egress queues feature is not enabled");
            up(&bcm_ethlock_switch_config);
            return -1;
        }
        hq_preempt = (val8 >> TXQ_CTRL_HQ_PREEMPT_S) & TXQ_CTRL_HQ_PREEMPT_M;
        /* If HQP is set then the scheduling is either SP or COMBO.
           Else it is WRR */
        if (hq_preempt) {
            if(txq_mode == 1) {
                sched = BCM_COSQ_STRICT;
            } else {
                sched = BCM_COSQ_COMBO;
                val = txq_mode;
                if (copy_to_user((void*)(&e->queue), (void*)&val,
                    sizeof(int))) {
                    up(&bcm_ethlock_switch_config);
                    return -EFAULT;
                }
            }
        } else {
            sched = BCM_COSQ_WRR;
        }
        if (copy_to_user((void*)(&e->scheduling), (void*)&sched, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        /* Get the weights */
        if(sched != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, val8);
                val = val8;
                if (copy_to_user((void*)(&e->weights[i]), (void*)&val,
                    sizeof(int))) {
                    up(&bcm_ethlock_switch_config);
                    return -EFAULT;
                }
                BCM_ENET_DEBUG("e->weight[%2d] = %02d ", i, e->weights[i]);
            }
        }
    } else {
        BCM_ENET_DEBUG("Given scheduling mode: %02d", e->scheduling);
        BCM_ENET_DEBUG("Given sp_endq: %02d", e->queue);
        for (i=0; i < NUM_EGRESS_QUEUES; i++) {
            BCM_ENET_DEBUG("Given weight[%2d] = %02d ", i, e->weights[i]);
            if (e->weights[i] <= 0 || e->weights[i] > MAX_WRR_WEIGHT) {
                BCM_ENET_DEBUG("Invalid weight");
                up(&bcm_ethlock_switch_config);
                return BCM_E_ERROR;
            }
        }
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%02x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M;
        if(txq_mode == 0) {
            BCM_ENET_DEBUG("Multiple egress queues feature is not enabled");
            up(&bcm_ethlock_switch_config);
            return -1;
        }
        /* Set the scheduling mode */
        if (e->scheduling == BCM_COSQ_WRR) {
            /* Set HQP to 0 to enable WRR */
            val8 &= (~(TXQ_CTRL_HQ_PREEMPT_M << TXQ_CTRL_HQ_PREEMPT_S));
        } else if ((e->scheduling == BCM_COSQ_STRICT) ||
                   (e->scheduling == BCM_COSQ_COMBO)){
            /* Enable HQP for SP mode */
            val8 |= (TXQ_CTRL_HQ_PREEMPT_M << TXQ_CTRL_HQ_PREEMPT_S);
            /* Set TXQ_MODE as 1 for SP across all egress queues */
            if (e->scheduling == BCM_COSQ_STRICT) {
                txq_mode = 1;
            } else {
                txq_mode = e->queue;
            }
            val8 &= (~(TXQ_CTRL_TXQ_MODE_M << TXQ_CTRL_TXQ_MODE_S));
            val8 |= ((txq_mode & TXQ_CTRL_TXQ_MODE_M) << TXQ_CTRL_TXQ_MODE_S);
        } else {
            BCM_ENET_DEBUG("Invalid scheduling mode %02d", e->scheduling);
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        BCM_ENET_DEBUG("Writing 0x%02x to REG_QOS_TXQ_CTRL", val8);
        ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        /* Set the weights if WRR or COMBO */
        if(e->scheduling != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, e->weights[i]);
                val8 =  e->weights[i];
                ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
            }
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/* For TYPE_GET, return the queue value to caller. Push the copy_to_user to the next level up
   so enet_ioctl_ethsw_cosq_port_mapping can be used by the enet driver.
   Negative return values used to indicate an error - Jan 2011 */
int enet_ioctl_ethsw_cosq_port_mapping(struct ethswctl_data *e)
{
    uint32_t val32;
#if !defined(CONFIG_BCM96816)
    uint16_t val16;
#endif
    int queue;
    int retval = 0;

    BCM_ENET_DEBUG("Given port: %02d \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return -BCM_E_ERROR;
    }
    BCM_ENET_DEBUG("Given priority: %02d \n ", e->priority);
    if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
        printk("Invalid Priority \n");
        return -BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
#if !defined(CONFIG_BCM96816)
        ethsw_rreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
        BCM_ENET_DEBUG("REG_QOS_PORT_PRIO_MAP_Px = 0x%04x", val16);
        val32 = val16;
#else
        ethsw_rreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 4),
                   (uint8_t *)&val32, 4);
        BCM_ENET_DEBUG("REG_QOS_PORT_PRIO_MAP_Px = 0x%08x",
                       (unsigned int)val32);
#endif
        /* Get the queue */
        queue = val32 >> (e->priority * REG_QOS_PRIO_TO_QID_SEL_BITS);
        queue &= REG_QOS_PRIO_TO_QID_SEL_M;
        retval = queue;
        BCM_ENET_DEBUG("e->queue is = %4x", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
#if !defined(CONFIG_BCM96816)
        ethsw_rreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
        val32 = val16;
#else
        ethsw_rreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 4),
                   (uint8_t *)&val32, 4);
#endif
        val32 &= ~(REG_QOS_PRIO_TO_QID_SEL_M <<
                   (e->priority * REG_QOS_PRIO_TO_QID_SEL_BITS));
        val32 |= ((e->queue & REG_QOS_PRIO_TO_QID_SEL_M) <<
                  (e->priority * REG_QOS_PRIO_TO_QID_SEL_BITS));
        BCM_ENET_DEBUG("Writing = 0x%08x to REG_QOS_PORT_PRIO_MAP_Px",
                       (unsigned int)val32);
#if !defined(CONFIG_BCM96816)
        val16 = val32;
        ethsw_wreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
#else
        ethsw_wreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 4),
                   (uint8_t *)&val32, 4);
#endif
    }

    up(&bcm_ethlock_switch_config);
    return retval;
}

int ethsw_dscp_to_priority_mapping(struct ethswctl_data *e)
{
    uint32_t val32, offsetlo, offsethi, mapnum;
    uint16_t val16;
    int priority, dscplsbs, offsetlolen = 4;

    BCM_ENET_DEBUG("Given dscp: %02d \n ", e->val);
    if (e->val > 0x3F) {
        printk("Invalid DSCP Value \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    dscplsbs = e->val & 0xF;
    mapnum = (e->val >> 4) & 0x3;
    switch (mapnum) {
        case 0:
            offsetlo = REG_QOS_DSCP_PRIO_MAP0LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP0HI;
            break;
        case 1:
            offsetlo = REG_QOS_DSCP_PRIO_MAP1LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP1HI;
            offsetlolen = 2;
            break;
        case 2:
            offsetlo = REG_QOS_DSCP_PRIO_MAP2LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP2HI;
            break;
        case 3:
            offsetlo = REG_QOS_DSCP_PRIO_MAP3LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP3HI;
            offsetlolen = 2;
            break;
        default:
            return -1;
    }

    if (e->type == TYPE_GET) {
        if (offsetlolen == 2) {
            if (dscplsbs <= 4) {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
                priority = (val16 >> (dscplsbs * 3)) & 0x7;
            } else if (dscplsbs > 5) {
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
                priority = (val32 >> (((dscplsbs - 5) * 3) - 1) ) & 0x7;
            } else {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
                priority = ((val16 >> 15) & 0x1) | ((val32 & 0x3) << 1);
            }
        } else {
            if (dscplsbs <= 9) {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
                priority = (val32 >> (dscplsbs * 3)) & 0x7;
            } else if (dscplsbs > 10) {
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
                priority = (val16 >> (((dscplsbs - 10) * 3) - 2) ) & 0x7;
            } else {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
                priority = ((val32 >> 30) & 0x3) | ((val16 & 0x1) << 2);
            }
        }
        if (copy_to_user((void*)(&e->priority), (void*)&priority, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("dscp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given priority: %02d \n ", e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority \n");
            return BCM_E_ERROR;
        }
        if (offsetlolen == 2) {
            ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
            ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
            if (dscplsbs <= 4) {
                val16 &= ~(0x7 << (dscplsbs * 3));
                val16 |= ((e->priority & 0x7) << (dscplsbs * 3));
            } else if (dscplsbs > 5) {
                val32 &= ~(0x7 << (((dscplsbs - 5) * 3) - 1));
                val32 |= ((e->priority & 0x7) << (((dscplsbs - 5) * 3) - 1));
            } else {
                val16 &= ~(1 << 15);
                val16 |= ((e->priority & 0x1) << 15);
                val32 &= ~(0x3);
                val32 |= ((e->priority >> 1) & 0x3);
            }
            ethsw_wreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
            ethsw_wreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
        } else {
            ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
            ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
            if (dscplsbs <= 9) {
                val32 &= ~(0x7 << (dscplsbs * 3));
                val32 |= ((e->priority & 0x7) << (dscplsbs * 3));
            } else if (dscplsbs > 10) {
                val16 &= ~(0x7 << (((dscplsbs - 10) * 3) - 2));
                val16 |= ((e->priority & 0x7) << (((dscplsbs - 10) * 3) - 2));
            } else {
                val32 &= ~(0x3 << 30);
                val32 |= ((e->priority & 0x3) << 30);
                val16 &= ~(0x1);
                val16 |= ((e->priority >> 2) & 0x1);
            }
            ethsw_wreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
            ethsw_wreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}


int enet_ioctl_ethsw_cosq_rxchannel_mapping(struct ethswctl_data *e)
{
    uint32_t val32;
    int channel;

    BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
    if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
        printk("Invalid queue \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        BCM_ENET_DEBUG("REG_IUDMA_QUEUE_CTRL = 0x%08x", (unsigned int)val32);
        /* Get the channel */
        channel = val32 >> (REG_IUDMA_Q_CTRL_RXQ_SEL_S + (e->queue * 2));
        channel &= REG_IUDMA_Q_CTRL_PRIO_TO_CH_M;
        if (copy_to_user((void*)(&e->channel), (void*)&channel, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->channel is = %4x", e->channel);
    } else {
        BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
        if ((e->channel >= ENET_RX_CHANNELS_MAX) || (e->channel < 0)) {
            printk("Invalid Channel \n");
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }

        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        val32 &= ~(REG_IUDMA_Q_CTRL_PRIO_TO_CH_M <<
                   (REG_IUDMA_Q_CTRL_RXQ_SEL_S + (e->queue * 2)));
        val32 |= ((e->channel & REG_IUDMA_Q_CTRL_PRIO_TO_CH_M) <<
                  (REG_IUDMA_Q_CTRL_RXQ_SEL_S + (e->queue * 2)));
        BCM_ENET_DEBUG("Writing = 0x%08x to REG_IUDMA_QUEUE_CTRL",
                       (unsigned int)val32);
        ethsw_wreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int enet_ioctl_ethsw_cosq_txq_sel(struct ethswctl_data *e)
{
    uint32_t v32, method = 0;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        /* Configure the switch to use Desc priority */
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
        if (v32 & REG_IUDMA_CTRL_USE_DESC_PRIO) {
            method = USE_TX_BD_PRIORITY;
        } else if (v32 & REG_IUDMA_CTRL_USE_QUEUE_PRIO){
            method = USE_TX_DMA_CHANNEL;
        } else {
            /* Indicate neither BD nor iuDMA queue priority are used */
            method = NONE_OF_THE_METHODS;
        }
        if (copy_to_user((void*)(&e->ret_val), (void*)&method, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given txq_sel method: %02d \n ", e->val);

        /* Set the txq selection method as given */
        if (e->val == USE_TX_BD_PRIORITY) {
            /* Configure the switch to use Desc priority */
            ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
            v32 &= ~REG_IUDMA_CTRL_USE_QUEUE_PRIO;
            v32 |= REG_IUDMA_CTRL_USE_DESC_PRIO;
            ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
#ifndef SINGLE_CHANNEL_TX
            use_tx_dma_channel_for_priority = 0;
#endif /*SINGLE_CHANNEL_TX*/
        } else if (e->val == USE_TX_DMA_CHANNEL) {
            /* Configure the switch to use Desc priority */
            ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
            v32 &= ~REG_IUDMA_CTRL_USE_DESC_PRIO;
            v32 |= REG_IUDMA_CTRL_USE_QUEUE_PRIO;
            ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
#ifndef SINGLE_CHANNEL_TX
            use_tx_dma_channel_for_priority = 1;
#endif /*SINGLE_CHANNEL_TX*/
        } else {
            up(&bcm_ethlock_switch_config);
            printk("Invalid method \n");
            return BCM_E_ERROR;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int ethsw_iudmaq_to_egressq_map_get(int iudmaq, int *egressq)
{
    uint32_t val32;

    if ((iudmaq >= ENET_RX_CHANNELS_MAX) || (iudmaq < 0)) {
        printk("Invalid iudma queue \n");
        return -1;
    }

    down(&bcm_ethlock_switch_config);
    ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
    up(&bcm_ethlock_switch_config);

    *egressq = val32 >> (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (iudmaq * 3));
    *egressq &= REG_IUDMA_Q_CTRL_CH_TO_PRIO_M;
    BCM_ENET_DEBUG("The iudmaq %02d is mapped to egress queue: %02d \n ", iudmaq, *egressq);

    return 0;
}
EXPORT_SYMBOL(ethsw_iudmaq_to_egressq_map_get);

int enet_ioctl_ethsw_cosq_txchannel_mapping(struct ethswctl_data *e)
{
    uint32_t val32;
    int queue;

    BCM_ENET_DEBUG("Given channel: %02d \n ", e->channel);
    if ((e->channel >= ENET_RX_CHANNELS_MAX) || (e->channel < 0)) {
        printk("Invalid Channel \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        BCM_ENET_DEBUG("REG_IUDMA_QUEUE_CTRL = 0x%08x", (unsigned int)val32);
        /* Get the queue */
        queue = val32 >> (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (e->channel * 3));
        queue &= REG_IUDMA_Q_CTRL_CH_TO_PRIO_M;
        if (copy_to_user((void*)(&e->queue), (void*)&queue, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("The queue: %02d \n ", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: %02d \n ", e->queue);
        if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
            up(&bcm_ethlock_switch_config);
            printk("Invalid queue \n");
            return BCM_E_ERROR;
        }

        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        val32 &= ~(REG_IUDMA_Q_CTRL_CH_TO_PRIO_M <<
                   (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (e->channel * 3)));
        val32 |= ((e->queue & REG_IUDMA_Q_CTRL_CH_TO_PRIO_M) <<
                  (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (e->channel * 3)));
        BCM_ENET_DEBUG("Writing = 0x%08x to REG_IUDMA_QUEUE_CTRL",
                       (unsigned int)val32);
        ethsw_wreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);

#ifndef SINGLE_CHANNEL_TX
        /* Maintain the channel to queue mapping information in driver */
        channel_for_queue[e->queue] = e->channel;
#endif /*SINGLE_CHANNEL_TX*/
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/* Stats API */
typedef enum bcm_hw_stat_e {
    TXOCTETSr = 0,
    TXDROPPKTSr,
    TXQOSPKTSr,
    TXBROADCASTPKTSr,
    TXMULTICASTPKTSr,
    TXUNICASTPKTSr,
    TXCOLLISIONSr,
    TXSINGLECOLLISIONr,
    TXMULTIPLECOLLISIONr,
    TXDEFERREDTRANSMITr,
    TXLATECOLLISIONr,
    TXEXCESSIVECOLLISIONr,
    TXFRAMEINDISCr,
    TXPAUSEPKTSr,
    TXQOSOCTETSr,
    RXOCTETSr,
    RXUNDERSIZEPKTSr,
    RXPAUSEPKTSr,
    PKTS64OCTETSr,
    PKTS65TO127OCTETSr,
    PKTS128TO255OCTETSr,
    PKTS256TO511OCTETSr,
    PKTS512TO1023OCTETSr,
    PKTS1024TO1522OCTETSr,
    RXOVERSIZEPKTSr,
    RXJABBERSr,
    RXALIGNMENTERRORSr,
    RXFCSERRORSr,
    RXGOODOCTETSr,
    RXDROPPKTSr,
    RXUNICASTPKTSr,
    RXMULTICASTPKTSr,
    RXBROADCASTPKTSr,
    RXSACHANGESr,
    RXFRAGMENTSr,
    RXEXCESSSIZEDISCr,
    RXSYMBOLERRORr,
    RXQOSPKTSr,
    RXQOSOCTETSr,
    PKTS1523TO2047r,
    PKTS2048TO4095r,
    PKTS4096TO8191r,
    PKTS8192TO9728r,
    MAXNUMCNTRS,
} bcm_hw_stat_t;


typedef struct bcm_reg_info_t {
    uint8_t offset;
    uint8_t len;
} bcm_reg_info_t;

/* offsets and lengths of stats defined in bcm_hw_stat_t */
bcm_reg_info_t bcm_stat_reg_list[] =  {
    {0x00, 8}, {0x08, 4}, {0x0c, 4}, {0x10, 4}, {0x14, 4},
    {0x18, 4}, {0x1C, 4}, {0x20, 4}, {0x24, 4}, {0x28, 4},
    {0x2C, 4}, {0x30, 4}, {0x34, 4}, {0x38, 4}, {0x3c, 8},
    {0x44, 8}, {0x4c, 4}, {0x50, 4}, {0x54, 4}, {0x58, 4},
    {0x5c, 4}, {0x60, 4}, {0x64, 4}, {0x68, 4}, {0x6c, 4},
    {0x70, 4}, {0x74, 4}, {0x78, 4}, {0x7c, 8}, {0x84, 4},
    {0x88, 4}, {0x8c, 4}, {0x90, 4}, {0x94, 4}, {0x98, 4},
    {0x9c, 4}, {0xa0, 4}, {0xa4, 4}, {0xa8, 8}, {0xb0, 4},
    {0xb4, 4}, {0xb8, 4}, {0xbc, 4},
};

#define COMPILER_64_SET(dst, src_hi, src_lo) \
    ((dst) = (((uint64) (src_hi)) << 32) | ((uint64) (src_lo)))
#define COMPILER_64_ZERO(dst)       ((dst) = 0)
#define COMPILER_64_ADD_64(dst, src)  ((dst) += (src))
#define COMPILER_64_SUB_64(dst, src)  ((dst) -= (src))
#define COMPILER_64_EQ(src1, src2)      ((src1) == (src2))
#define COMPILER_64_LT(src1, src2)      ((src1) <  (src2))

static uint64 counter_hw_val[TOTAL_SWITCH_PORTS][MAXNUMCNTRS];
static uint64 counter_sw_val[TOTAL_SWITCH_PORTS][MAXNUMCNTRS];
static uint64 counter_delta[TOTAL_SWITCH_PORTS][MAXNUMCNTRS];


/*
 * Function:
 *      soc_robo_counter_get
 * Purpose:
 *      Retrieves the value of a 64-bit software shadow counter.
 * Parameters:
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - (OUT) Pointer to place the 64-bit result.
 * Returns:
 *      BCM_E_XXX.
 */
static inline int ethsw_robo_counter_get(bcm_port_t port, bcm_hw_stat_t
                                         ctr_idx, uint64 *val)
{
    BCM_ENET_INFO("port = %2d; ctr_idx = %4d", port, ctr_idx);
    *val = counter_sw_val[port][ctr_idx];
    BCM_ENET_INFO("counter = 0x%llx", counter_sw_val[port][ctr_idx]);

    return BCM_E_NONE;
}

/*
 *  Function : enet_ioctl_ethsw_counter_get
 *
 *  Purpose :
 *      Get the snmp counter value.
 *
 *  Parameters :
 *      uint    :   uint number.
 *      port        :   port number.
 *      counter_type   :   counter_type.
 *      val  :   counter val.
 *
 *  Return :
 *      BCM_E_XXX
 *
 *  Note :
 *
 */
int enet_ioctl_ethsw_counter_get(struct ethswctl_data *e)
{
    uint64  count=0, count_tmp=0;
    uint32_t port = e->port;
    uint32_t counter_type = e->counter_type;
    uint64 val;

    BCM_ENET_INFO("Port = %2d", e->port);
    BCM_ENET_INFO("Counter Type = %4d", e->counter_type);
    switch (counter_type)
    {
        /* *** RFC 1213 *** */

        case snmpIfInOctets:
            ethsw_robo_counter_get(port, RXOCTETSr, &count);
            break;

        case snmpIfInUcastPkts:
            /* Total non-error frames minus broadcast/multicast frames */
            ethsw_robo_counter_get(port, RXUNICASTPKTSr, &count);
            break;

        case snmpIfInNUcastPkts:
            /* Multicast frames plus broadcast frames */
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfInDiscards:
            ethsw_robo_counter_get(port, RXDROPPKTSr, &count);
            break;

        case snmpIfInErrors:
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXFCSERRORSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFRAGMENTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXEXCESSSIZEDISCr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXJABBERSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfInUnknownProtos:
            val = 0;
            return BCM_E_UNAVAIL;
            break;

        case snmpIfOutOctets:
            ethsw_robo_counter_get(port, TXOCTETSr, &count);
            break;

        case snmpIfOutUcastPkts: /* ALL - mcast - bcast */
            ethsw_robo_counter_get(port, TXUNICASTPKTSr, &count);
            break;

        case snmpIfOutNUcastPkts:
            /* broadcast frames plus multicast frames */
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfOutDiscards:
            ethsw_robo_counter_get(port, TXDROPPKTSr, &count);
            break;

        case snmpIfOutErrors:
            ethsw_robo_counter_get(port, TXEXCESSIVECOLLISIONr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXLATECOLLISIONr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfOutQLen:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpInReceives:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpInHdrErrors:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpForwDatagrams:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpInDiscards:
            return BCM_E_UNAVAIL;
            break;

        /* *** RFC 1493 *** */

        case snmpDot1dBasePortDelayExceededDiscards:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot1dBasePortMtuExceededDiscards:
            /* robo not suppport */
            return BCM_E_UNAVAIL;
            break;

        case snmpDot1dTpPortInFrames:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpDot1dTpPortOutFrames:
            ethsw_robo_counter_get(port, TXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpDot1dPortInDiscards:
            ethsw_robo_counter_get(port, RXDROPPKTSr, &count);
            break;

        /* *** RFC 1757 *** */
        case snmpEtherStatsDropEvents:
            ethsw_robo_counter_get(port, TXDROPPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXDROPPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsMulticastPkts:
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count);
            break;

        case snmpEtherStatsBroadcastPkts:
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count);
            break;

        case snmpEtherStatsUndersizePkts:
            ethsw_robo_counter_get(port, RXUNDERSIZEPKTSr, &count);
            break;

        case snmpEtherStatsFragments:
            ethsw_robo_counter_get(port, RXFRAGMENTSr,&count);
            break;

        case snmpEtherStatsPkts64Octets:
            ethsw_robo_counter_get(port, PKTS64OCTETSr,&count);
            break;

        case snmpEtherStatsPkts65to127Octets:
            ethsw_robo_counter_get(port, PKTS65TO127OCTETSr,&count);
            break;

        case snmpEtherStatsPkts128to255Octets:
            ethsw_robo_counter_get(port, PKTS128TO255OCTETSr,&count);
            break;

        case snmpEtherStatsPkts256to511Octets:
            ethsw_robo_counter_get(port, PKTS256TO511OCTETSr,&count);
            break;

        case snmpEtherStatsPkts512to1023Octets:
            ethsw_robo_counter_get(port, PKTS512TO1023OCTETSr,&count);
            break;

        case snmpEtherStatsPkts1024to1518Octets:
            ethsw_robo_counter_get(port, PKTS1024TO1522OCTETSr,&count);
            break;

        case snmpEtherStatsOversizePkts:
            ethsw_robo_counter_get(port, RXOVERSIZEPKTSr,&count);
            break;

        case snmpEtherStatsJabbers:
            ethsw_robo_counter_get(port, RXJABBERSr,&count);
            break;

        case snmpEtherStatsOctets:
            ethsw_robo_counter_get(port, RXOCTETSr,&count);
            break;

        case snmpEtherStatsPkts:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr,&count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFCSERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFRAGMENTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXOVERSIZEPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXJABBERSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsCollisions:
            ethsw_robo_counter_get(port, TXCOLLISIONSr,&count);
            break;

        case snmpEtherStatsCRCAlignErrors:
            /* CRC errors + alignment errors */
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFCSERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsTXNoErrors:
            ethsw_robo_counter_get(port, TXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsRXNoErrors:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        /* RFC 1643 */
        case snmpDot3StatsInternalMacReceiveErrors:
            ethsw_robo_counter_get(port, RXDROPPKTSr,&count);
            break;

        case snmpDot3StatsFrameTooLongs:
            ethsw_robo_counter_get(port, RXOVERSIZEPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXJABBERSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpDot3StatsAlignmentErrors:  /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr,&count);
            break;

        case snmpDot3StatsFCSErrors:    /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, RXFCSERRORSr,&count);
            break;

        case snmpDot3StatsInternalMacTransmitErrors:
            ethsw_robo_counter_get(port, TXDROPPKTSr,&count);
            break;

        case snmpDot3StatsSingleCollisionFrames:
            /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, TXSINGLECOLLISIONr,&count);
            break;

        case snmpDot3StatsMultipleCollisionFrames:
            /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, TXMULTIPLECOLLISIONr,&count);
            break;

        case snmpDot3StatsDeferredTransmissions:
            ethsw_robo_counter_get(port, TXDEFERREDTRANSMITr,&count);
            break;

        case snmpDot3StatsLateCollisions:
            ethsw_robo_counter_get(port, TXLATECOLLISIONr,&count);
            break;

        case snmpDot3StatsExcessiveCollisions:
            ethsw_robo_counter_get(port, TXEXCESSIVECOLLISIONr,&count);
            break;

        case snmpDot3StatsCarrierSenseErrors:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot3StatsSQETTestErrors:
            /* not support for BCM5380 */
            return BCM_E_UNAVAIL;
            break;


        /* *** RFC 2665 *** some object same as RFC 1643 */

        case snmpDot3StatsSymbolErrors:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot3ControlInUnknownOpcodes:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot3InPauseFrames:
            ethsw_robo_counter_get(port, RXPAUSEPKTSr, &count);
            break;

        case snmpDot3OutPauseFrames:
            ethsw_robo_counter_get(port, TXPAUSEPKTSr, &count);
            break;

        /*** RFC 2233 ***/
        case snmpIfHCInOctets:
            ethsw_robo_counter_get(port, RXOCTETSr,&count);
            break;

        case snmpIfHCInUcastPkts:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr,&count);
            break;

        case snmpIfHCInMulticastPkts:
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr,&count);
            break;

        case snmpIfHCInBroadcastPkts:
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr,&count);
            break;

        case snmpIfHCOutOctets:
            ethsw_robo_counter_get(port, TXOCTETSr,&count);
            break;

        case snmpIfHCOutUcastPkts:
            ethsw_robo_counter_get(port, TXUNICASTPKTSr,&count);
            break;

        case snmpIfHCOutMulticastPkts:
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr,&count);
            break;

        case snmpIfHCOutBroadcastPckts:
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr,&count);
            break;

        /*** RFC 2465 ***/
        case snmpIpv6IfStatsInReceives:
        case snmpIpv6IfStatsInHdrErrors:
        case snmpIpv6IfStatsInAddrErrors:
        case snmpIpv6IfStatsInDiscards:
        case snmpIpv6IfStatsOutForwDatagrams:
        case snmpIpv6IfStatsOutDiscards:
        case snmpIpv6IfStatsInMcastPkts:
        case snmpIpv6IfStatsOutMcastPkts:
            val = 0;
            return BCM_E_UNAVAIL;
            break;

        case snmpBcmIPMCBridgedPckts:
        case snmpBcmIPMCRoutedPckts:
        case snmpBcmIPMCInDroppedPckts:
        case snmpBcmIPMCOutDroppedPckts:
            val = 0;
            return BCM_E_UNAVAIL;

        case snmpBcmEtherStatsPkts1519to1522Octets:
            val = 0;
            return BCM_E_UNAVAIL;

        case snmpBcmEtherStatsPkts1522to2047Octets:
            ethsw_robo_counter_get(port, PKTS1523TO2047r,&count);
            break;

        case snmpBcmEtherStatsPkts2048to4095Octets:
            ethsw_robo_counter_get(port, PKTS2048TO4095r,&count);
            break;

        case snmpBcmEtherStatsPkts4095to9216Octets:
            val = 0;
            return BCM_E_UNAVAIL;

        default:
            val = 0;
            BCM_ENET_DEBUG("Not supported\n");
            return BCM_E_PARAM;
    }

    BCM_ENET_INFO("count = 0x%llx", count);
    val = count;
    if (copy_to_user( (void*)(&e->counter_val), (void*)&val, 8)) {
        return -EFAULT;
    }
    BCM_ENET_INFO("e->counter_val = 0x%llx", e->counter_val);
    return BCM_E_NONE;
}

#define PBMP_MEMBER(bmp, port)  (((bmp) & (1U<<(port))) != 0)
#define PBMP_ITER(bmp, port) \
        for ((port) = 0; (port) < TOTAL_SWITCH_PORTS; (port)++) \
            if (PBMP_MEMBER((bmp), (port)))

/*
 * Function:
 *      ethsw_robo_counter_collect
 * Purpose:
 *      Collects and accumulates the stats
 * Parameters:
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      BCM_E_XXX
 */
int ethsw_counter_collect(uint32_t portmap, int discard)
{
    int32_t         i = 0;
    uint32_t        ctr_new32, ctraddr, len;
    uint16_t        v16;
    bcm_port_t      port;
    uint64          ctr_new, ctr_prev, ctr_diff;
    bcm_reg_info_t  reg_info;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ethsw_phy_pll_up(0);
#endif

    PBMP_ITER(portmap, port) {
        for (i = 0; i < MAXNUMCNTRS; i++) {
            /* Get the counter offset and length */
            reg_info = bcm_stat_reg_list[i];
            ctraddr = reg_info.offset;
            len = reg_info.len;

            ctr_prev = counter_hw_val[port][i];

            /* Read the counter value from H/W */
            down(&bcm_ethlock_switch_config);
            if(len == 8) {
                /* For the 64-bit counter */
                ethsw_rreg(PAGE_MIB_P0 + (port), ctraddr,
                           (uint8_t *)&ctr_new, 8);
            } else {
                /* For the 32-bit counter */
                ethsw_rreg(PAGE_MIB_P0 + (port), ctraddr,
                           (uint8_t *)&ctr_new32, 4);
                COMPILER_64_SET(ctr_new, 0, ctr_new32);
            }
            up(&bcm_ethlock_switch_config);

            BCM_ENET_DEBUG("port= %d; i = %d; The ctr_new = 0x%llx",
                                port, i, ctr_new);

            if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                COMPILER_64_ZERO(counter_delta[port][i]);
                continue;
            }

            if (discard) {
                /* Update the previous value buffer */
                counter_hw_val[port][i] = ctr_new;
                COMPILER_64_ZERO(counter_delta[port][i]);
                continue;
            }

            ctr_diff = ctr_new;
            if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                int             width;
                uint64          wrap_amt;

                /*
                 * Counter must have wrapped around.
                 * Add the proper wrap-around amount.
                 */
                width = len * 8;
                if (width < 32) {
                    COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                } else if (width < 64) {
                    COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                }
            }
            COMPILER_64_SUB_64(ctr_diff, ctr_prev);
            /*
             * For ROBO chips,
             * MIB counter TxPausePkts always counts both backpressure(half duplex) and
             * pause frames(full duplex). But this counter should not accumulate
             * when duplex is half.
             * Thus, update SW counter table only when duplex is full.
             */
            if (i == TXPAUSEPKTSr) {
                down(&bcm_ethlock_switch_config);
                ethsw_phy_rreg(ETHSW_PHY_GET_PHYID(port), MII_ASR, &v16);
                up(&bcm_ethlock_switch_config);
                if (MII_ASR_FDX(v16)) {
                    COMPILER_64_ADD_64(counter_sw_val[port][i], ctr_diff);
                    counter_delta[port][i] = ctr_diff;
                } else {
                    counter_delta[port][i] = 0;
                }
                counter_hw_val[port][i] = ctr_new;
            } else {
                COMPILER_64_ADD_64(counter_sw_val[port][i], ctr_diff);
                counter_delta[port][i] = ctr_diff;
                counter_hw_val[port][i] = ctr_new;
            }
            BCM_ENET_DEBUG("counter_sw_val = 0x%llx",
                                counter_sw_val[port][i]);
        }
    }

    __ethsw_get_txrx_imp_port_pkts();

    return BCM_E_NONE;
}

/*
 * Function:
 *      enet_ioctl_ethsw_clear_port_stats
 * Purpose:
 *      Clear the software accumulated counters for a given port
 * Parameters:
 *      e->port - StrataSwitch port #.
 * Returns:
 *      BCM_E_XXX
 */
int enet_ioctl_ethsw_clear_port_stats(struct ethswctl_data *e)
{
    bcm_port_t port = e->port;
    int i = 0;

    for (i = 0; i < MAXNUMCNTRS; i++) {

        /* Update the S/W counter */
        COMPILER_64_SET(counter_sw_val[port][i], 0, 0);
        COMPILER_64_SET(counter_delta[port][i], 0, 0);

        /* Do not update the H/W counter */
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      enet_ioctl_ethsw_clear_stats
 * Purpose:
 *      Clear the software accumulated counters
 * Returns:
 *      BCM_E_XXX
 */
int enet_ioctl_ethsw_clear_stats(uint32_t portmap)
{
    bcm_port_t port;
    int i = 0;

    PBMP_ITER(portmap, port) {
        for (i = 0; i < MAXNUMCNTRS; i++) {
            /* Update the S/W counter */
            COMPILER_64_SET(counter_sw_val[port][i], 0, 0);
            COMPILER_64_SET(counter_delta[port][i], 0, 0);
            /* Do not update the H/W counter */
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      enet_ioctl_ethsw_arl_access
 * Purpose:
 *      ARL table accesses
 * Returns:
 *      BCM_E_XXX
 */
int enet_ioctl_ethsw_arl_access(struct ethswctl_data *e)
{
    int timeout = 100;
    uint16_t v16;
    uint8_t v8;
    uint32_t v32;

    if (e->type == TYPE_GET) {
        BCM_ENET_DEBUG("e->mac: %02x %02x %02x %02x %02x %02x", e->mac[5],
                   e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0]);
        BCM_ENET_DEBUG("e->vid: %d", e->vid);
        v16 = e->mac[0] | (e->mac[1] << 8);
        BCM_ENET_INFO("mac_lo (16-bit) = 0x%x -> 0x502", v16);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
        v32 = e->mac[2] | (e->mac[3] << 8) |
              (e->mac[4] << 16) | (e->mac[5] << 24);
        BCM_ENET_INFO("mac_hi (32-bit) = 0x%x -> 0x504", (unsigned int)v32);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_HI, (uint8_t *)&v32, 4);
        v16 = e->vid;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&v16, 2);
        v8 = 0x81;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        while(v8 & ARL_TBL_CTRL_START_DONE) {
            mdelay(1);
            if (timeout-- <= 0)  {
                printk("Timeout Waiting for ARL Read Access Done \n");
                break;
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        }
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY, (uint8_t *)&v32, 4);
        BCM_ENET_INFO("0x510 read value(32-bit) = 0x%x", (unsigned int)v32);
        e->mac[0] = v32 & 0xFF;
        e->mac[1] = (v32 >> 8) & 0xFF;
        e->mac[2] = (v32 >> 16) & 0xFF;
        e->mac[3] = (v32 >> 24) & 0xFF;
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_VID_MAC_HI_ENTRY,
                   (uint8_t *)&v32, 4);
        BCM_ENET_INFO("0x514 read value(32-bit (VID:mac_hi)) = 0x%x",
                      (unsigned int)v32);
        e->mac[4] = v32 & 0xFF;
        e->mac[5] = (v32 >> 8) & 0xFF;
        e->vid = v32 >> 16;
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY, (uint8_t *)&v16, 2);
        e->val = v16;
    } else if (e->type == TYPE_SET) {
        BCM_ENET_DEBUG("e->mac: %02x %02x %02x %02x %02x %02x", e->mac[5],
                   e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0]);
        BCM_ENET_DEBUG("e->vid: %d", e->vid);
        v16 = e->mac[0] | (e->mac[1] << 8);
        BCM_ENET_INFO("mac_lo (16-bit) = 0x%x -> 0x502", v16);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
        v32 = e->mac[2] | (e->mac[3] << 8) |
              (e->mac[4] << 16) | (e->mac[5] << 24);
        BCM_ENET_INFO("mac_hi (32-bit) = 0x%x -> 0x504", (unsigned int)v32);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_HI, (uint8_t *)&v32, 4);
        v16 = e->vid;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&v16, 2);
        v8 = 0x81;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        while(v8 & ARL_TBL_CTRL_START_DONE) {
            mdelay(1);
            if (timeout-- <= 0)  {
                printk("Timeout Waiting for ARL Read Access Done \n");
                break;
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        }
        v32 = e->mac[0] | (e->mac[1] << 8) |
              (e->mac[2] << 16) | (e->mac[3] << 24);
        BCM_ENET_INFO("mac_lo (32-bit) = 0x%x -> 0x510", (unsigned int)v32);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY, (uint8_t *)&v32, 4);
        v32 = e->mac[4] | (e->mac[5] << 8) | (e->vid << 16);
        BCM_ENET_INFO("mac_hi (32-bit) = 0x%x -> 0x514", (unsigned int)v32);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VID_MAC_HI_ENTRY, (uint8_t *)&v32, 4);
        v16 = e->val;
        BCM_ENET_INFO("data (16-bit) = 0x%x -> 0x518", v16);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY, (uint8_t *)&v16, 2);
        v8 = 0x80;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        timeout = 100;
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        while(v8 & ARL_TBL_CTRL_START_DONE) {
            mdelay(1);
            if (timeout-- <= 0)  {
                printk("Timeout Waiting for ARL Write Access Done \n");
                break;
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        }
    } else if (e->type == TYPE_DUMP) {
        int timeout, count = 0, first = 1;
        uint32_t val32, first_mac_lo = 0, first_vid_mac_hi = 0;
        v16 = ARL_SRCH_CTRL_START_DONE;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        BCM_ENET_INFO("ARL_SRCH_CTRL (0x530) = 0x%x ", v16);
        while (v16 & ARL_SRCH_CTRL_START_DONE) {
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
            timeout = 1000;
            while((v16 & ARL_SRCH_CTRL_SR_VALID) == 0) {
                ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL,
                           (uint8_t *)&v16, 2);
                if (v16 & ARL_SRCH_CTRL_START_DONE) {
                    mdelay(1);
                    if (timeout-- <= 0) {
                        printk("ARL Search Timeout for Valid to be 1 \n");
                        return BCM_E_NONE;
                    }
                } else {
                    printk("ARL Search Done count %d\n", count);
                    return BCM_E_NONE;
                }
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY,
                       (uint8_t *)&val32, 4);
            BCM_ENET_INFO("ARL_SRCH_MAC_LO (0x534) = 0x%x ",
                          (unsigned int)val32);
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_VID_MAC_HI_ENTRY,
                       (uint8_t *)&v32, 4);
            BCM_ENET_INFO("ARL_SRCH_VID_MAC_HI (0x538) = 0x%x ",
                          (unsigned int)v32);
            if (first) {
                first_mac_lo = val32;
                first_vid_mac_hi = v32;
                first = 0;
            } else if ((first_mac_lo == val32) && (first_vid_mac_hi == v32)) {
                printk("ARL Search Done. \n");
                /* Complete the Search */
                count = 0;
                ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL,
                           (uint8_t *)&v16, 2);
                while (v16 & ARL_SRCH_CTRL_START_DONE) {
                    ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY,
                               (uint8_t *)&v16, 2);
                    ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL,
                               (uint8_t *)&v16, 2);
                    if ((count++) > NUM_ARL_ENTRIES) {
                        printk("Hmmm...Check why ARL serach isn'y yet done?\n");
                        printk("ARL Search Done count %d\n", count);
                        return BCM_E_NONE;
                    }
                }
                break;
            }
            if (count % 10 == 0) {
                printk(" VLAN  MAC          DATA");
                printk("(static-15,age-14,pri-13:11,pmap-9:1)");
            }
            printk("\n %04d", (int)(v32 >> 16) & 0xFFFF);
            printk("  %04x", (unsigned int)(v32 & 0xFFFF));
            printk("%08x", (unsigned int)val32);
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY,
                       (uint8_t *)&v16, 2);
            printk(" 0x%04x \n", v16);
            BCM_ENET_INFO("ARL_SRCH_DATA (0x53c) = 0x%x ", v16);
            if ((count++) > NUM_ARL_ENTRIES) {
                printk("ARL Search Done \n");
                break;
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        }
        printk("ARL Search Done count %d\n", (int)count);
    } else if (e->type == TYPE_FLUSH) {
        /* Flush the ARL table */
        fast_age_all(1);
    } else {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

#ifdef CONFIG_BCM96816
int enet_ioctl_ethsw_pkt_padding(struct ethswctl_data *e)
{
    uint32 val32;
    int enable, length;

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_FFE, REG_FFE_RXPORT_CTRL, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("REG_SWPKT_CTRL_USB = 0x%08x", (unsigned int)val32);
        /* Get the enable/disable status */
        enable = (val32 >> FFE_PAD_ENABLE_S) & FFE_PAD_ENABLE_M;
        if (copy_to_user((void*)(&e->ret_val), (void*)&enable, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("The enable: %02d \n ", e->ret_val);
        length = (val32 >> FFE_PAD_SIZE_S) & FFE_PAD_SIZE_M;
        if (copy_to_user((void*)(&e->length), (void*)&length, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("The pad length: %02d \n ", e->length);
    } else {
        BCM_ENET_DEBUG("Given enable: %02d \n ", e->val);
        BCM_ENET_DEBUG("Given length: %04d \n ", e->length);
        ethsw_rreg(PAGE_FFE, REG_FFE_RXPORT_CTRL, (uint8 *)&val32, 4);
        val32 &= ~((FFE_PAD_ENABLE_M << FFE_PAD_ENABLE_S) |
                   (FFE_PAD_SIZE_M << FFE_PAD_SIZE_S));
        val32 |= ((e->val & FFE_PAD_ENABLE_M) << FFE_PAD_ENABLE_S);
        val32 |= ((e->length & FFE_PAD_SIZE_M) << FFE_PAD_SIZE_S);
        BCM_ENET_DEBUG("Writing = 0x%08x to REG_FFE_RXPORT_CTRL",
                       (unsigned int)val32);
        ethsw_wreg(PAGE_FFE, REG_FFE_RXPORT_CTRL, (uint8 *)&val32, 4);
    }

    return BCM_E_NONE;
}
#endif /*CONFIG_BCM96816*/


int enet_ioctl_ethsw_port_default_tag_config(struct ethswctl_data *e)
{
    uint32_t v32;
    uint16_t v16;

    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG("Given port: %02d \n ", e->port);
    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (e->port * 2),
                   (uint8_t *)&v16, 2);
        v32 = v16;
        if (copy_to_user((void*)(&e->priority), (void*)&v32, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->priority is = %02d", e->priority);
    } else {
        BCM_ENET_DEBUG("Given priority: %02d \n ", e->priority);
        ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (e->port * 2),
                   (uint8_t *)&v16, 2);
        v16 &= (~(DEFAULT_TAG_PRIO_M << DEFAULT_TAG_PRIO_S));
        v16 |= ((e->priority & DEFAULT_TAG_PRIO_M) << DEFAULT_TAG_PRIO_S);
        ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (e->port * 2),
                   (uint8_t *)&v16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int enet_ioctl_ethsw_cos_priority_method_config(struct ethswctl_data *e)
{
    uint32_t v32;
    uint8_t v8;
    uint8_t port_qos_en, qos_layer_sel;

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG("Given method: %02d \n ", e->val);
    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, (uint8_t *)&v8, 1);
        port_qos_en = (v8 >> PORT_QOS_EN_S) & PORT_QOS_EN_M;
        qos_layer_sel = (v8 >> QOS_LAYER_SEL_S) & QOS_LAYER_SEL_M;
        if (port_qos_en && qos_layer_sel != 3) {
            v32 = PORT_QOS;
        } else {
            v32 = COMBO_QOS;
        }
        if (copy_to_user((void*)(&e->ret_val), (void*)&v32, sizeof(int))) {
            up(&bcm_ethlock_switch_config);
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->ret_val is = %02d", e->ret_val);
    } else {
        if (e->val == PORT_QOS) {
            ethsw_rreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, (uint8_t *)&v8, 1);
            v8 |= (PORT_QOS_EN_M << PORT_QOS_EN_S);
            v8 &= ~(QOS_LAYER_SEL_M << QOS_LAYER_SEL_S);
            ethsw_wreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, (uint8_t *)&v8, 1);
        } else {
            ethsw_rreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, (uint8_t *)&v8, 1);
            v8 &= ~(PORT_QOS_EN_M << PORT_QOS_EN_S);
            ethsw_wreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, (uint8_t *)&v8, 1);
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int reset_mib(int extswitch)
{
    uint8_t val8;

    ethsw_rreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    val8 |= GLOBAL_CFG_RESET_MIB;
    ethsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    val8 &= (~GLOBAL_CFG_RESET_MIB);
    ethsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);

    if (extswitch) {
        extsw_rreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
        val8 |= GLOBAL_CFG_RESET_MIB;
        extsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
        val8 &= (~GLOBAL_CFG_RESET_MIB);
        extsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    }

    return BCM_E_NONE;
}


int enet_ioctl_ethsw_port_traffic_control(struct ethswctl_data *e)
{
    uint32_t val32;
    uint8_t v8;

    BCM_ENET_DEBUG("Given port: %02d \n ", e->port);
    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + e->port, &v8, 1);
        /* Get the enable/disable status */
        val32 = v8 & (REG_PORT_CTRL_DISABLE);
        if (copy_to_user((void*)(&e->ret_val), (void*)&val32, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("The port ctrl status: %02d \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given port control: %02x \n ", e->val);
        ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + e->port, &v8, 1);
        v8 &= (~REG_PORT_CTRL_DISABLE);
        v8 |= (e->val & REG_PORT_CTRL_DISABLE);
        BCM_ENET_DEBUG("Writing = 0x%x to REG_PORT_CTRL", v8);
        ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + e->port, &v8, 1);
    }

    return BCM_E_NONE;
}

int enet_ioctl_ethsw_port_loopback(struct ethswctl_data *e, int phy_id)
{
    uint32_t val32, v32;
    uint8_t v8, phy_connected = 0;
    uint16_t v16;

    BCM_ENET_DEBUG("Given physical port %d", e->port);
    if (e->port >= EPHY_PORTS && e->port != USB_PORT_ID) {
        BCM_ENET_DEBUG("Invalid port %d ", e->port);
        return -EINVAL;
    }
    BCM_ENET_DEBUG("Given phy id %d", phy_id);
    if ((e->port < EPHY_PORTS) && IsPhyConnected(phy_id)) {
        phy_connected = 1;
    }

    if (e->type == TYPE_GET) {
        v32 = port_in_loopback_mode[e->port];
        if (copy_to_user((void*)(&e->ret_val), (void*)&v32, sizeof(int))) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("The port %d loopback status: %02d \n ", e->port, e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given enable/disable control %02x \n ", e->val);
        if (e->port == USB_PORT_ID) {
            if (e->val) {
                port_in_loopback_mode[e->port] = 1;
                ethsw_rreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&swpkt_ctrl_usb, 4);
                swpkt_ctrl_usb_saved = 1;
                val32 = USB_SWPKTBUS_LOOPBACK_VAL;
                ethsw_wreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&val32, 4);
                v8 = LINKDOWN_OVERRIDE_VAL;
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + USB_PORT_ID, &v8, 1);
            } else {
                v8 = LINKDOWN_OVERRIDE_VAL & ~(REG_PORT_STATE_LNK);
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + e->port, &v8, 1);
                if (swpkt_ctrl_usb_saved) {
                    /* Disable USB loopback by restoring the REG_SWPKT_CTRL_USB */
                    ethsw_wreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&swpkt_ctrl_usb, 4);
                } else {
                    val32 = 0;
                    ethsw_wreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&val32, 4);
                }
                port_in_loopback_mode[e->port] = 0;
            }
        } else {
            if (e->val) {
                if (phy_connected) {
                    port_in_loopback_mode[e->port] = 1;
                    ethsw_phy_rreg(phy_id, MII_BMCR, &v16);
                    v16 |= BMCR_LOOPBACK;
                    ethsw_phy_wreg(phy_id, MII_BMCR, &v16);
                } else {
                    printk("No Phy");
                }
                v8 = LINKDOWN_OVERRIDE_VAL;
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + e->port, &v8, 1);
            } else {
                v8 = LINKDOWN_OVERRIDE_VAL & ~(REG_PORT_STATE_LNK);
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + e->port, &v8, 1);
                if (phy_connected) {
                    ethsw_phy_rreg(phy_id, MII_BMCR, &v16);
                    v16 &= (~BMCR_LOOPBACK);
                    ethsw_phy_wreg(phy_id, MII_BMCR, &v16);
                } else {
                    printk("No Phy");
                }
                port_in_loopback_mode[e->port] = 0;
            }
        }
    }

    return BCM_E_NONE;
}

int enet_ioctl_ethsw_phy_mode(struct ethswctl_data *e, int phy_id)
{
    uint16_t v16;

    e->ret_val = -1;
    BCM_ENET_DEBUG("Given physical port %d", e->port);
    if (e->port >= EPHY_PORTS) {
        BCM_ENET_DEBUG("Invalid port %d ", e->port);
        return -EINVAL;
    }
    BCM_ENET_DEBUG("Given phy id %d", phy_id);
    if ((e->port < EPHY_PORTS) && !IsPhyConnected(phy_id)) {
        BCM_ENET_DEBUG("port %d: No Phy", e->port);
        return -EINVAL;
    }

    if (e->type == TYPE_GET) {
        ethsw_phy_rreg(phy_id, MII_BMCR, &v16);
        if (v16 & BMCR_ANENABLE)
            e->speed = 0;
        else if (v16 & BMCR_SPEED1000)
            e->speed = 1000;
        else if (v16 & BMCR_SPEED100)
            e->speed = 100;
        else
            e->speed = 10;

        if (v16 & BMCR_FULLDPLX)
            e->duplex = 1;
        else
            e->duplex = 0;
    } else {
        ethsw_phy_rreg(phy_id, MII_BMCR, &v16);
        if (e->speed == 0)
            v16 |= BMCR_ANENABLE;
        else {
            v16 &= (~BMCR_ANENABLE);
            if (e->speed == 1000) {
                v16 |= BMCR_SPEED1000;
            }
            else {
                v16 &= (~BMCR_SPEED1000);
                if (e->speed == 100)
                    v16 |= BMCR_SPEED100;
                else
                    v16 &= (~BMCR_SPEED100);
            }
            if (e->duplex)
                v16 |= BMCR_FULLDPLX;
            else
                v16 &= (~BMCR_FULLDPLX);
        }

        ethsw_phy_wreg(phy_id, MII_BMCR, &v16);
    }

    e->ret_val = 0;
    return BCM_E_NONE;
}

#if defined(CONFIG_BCM96816)
#define ETHSW_PORTS_6816 8 /* No CMF on MIPS Port */
uint8 portState[ETHSW_PORTS_6816];
/*
*------------------------------------------------------------------------------
* Function   : ethsw_save_port_state
* Description: Saves the current state of each Switch port and disables each
*              Switch port.
*
* Design Note1: Invoked by Packet CMF when resetting or initializing the CMF
*              hardware via the CMF Hook pktCmfSaveSwitchPortState.
*            Note2: Invoked with kernel lock taken, so no need to take bcm_ethlock_switch_config
*------------------------------------------------------------------------------
*/
int ethsw_save_port_state(void)
{
    int i;
    uint8 v8 = REG_PORT_CTRL_DISABLE;

    for (i = 0; i < ETHSW_PORTS_6816; i++)
    {
        ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + i, &portState[i], sizeof(uint8));
        ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, sizeof(uint8));
    }

    return 0;
}
/*
*------------------------------------------------------------------------------
* Function   : ethsw_restore_port_state
* Description: Restores the enable/disable state of each Switch port.
*
* Design Note: Invoked by Packet CMF when resetting or initializing the CMF
*              hardware via the CMF Hook pktCmfRestoreSwitchPortState.
*            Note2: Invoked with kernel lock taken, so no need to take bcm_ethlock_switch_config
*------------------------------------------------------------------------------
*/
int ethsw_restore_port_state(void)
{
    int i;

    for (i = 0; i < ETHSW_PORTS_6816; i++)
    {
        ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + i, &portState[i], sizeof(uint8));
    }

    return 0;
}


static int save_mib_state(int is6829)
{
    int i, j;

    for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
        for (j = 0; j < TOTAL_MIB_COUNTERS; j++) {
            ethsw_rreg_ext(PAGE_MIB_P0 + i, j * 4, (uint8_t*)&switch_mibs[i][j], 4, is6829);
        }
    }
    return 0;
}

static int restore_mib_state(int is6829)
{
    int i, j;

    for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
        for (j = 0; j < TOTAL_MIB_COUNTERS; j++) {
            ethsw_wreg_ext(PAGE_MIB_P0 + i, j * 4, (uint8_t*)&switch_mibs[i][j], 4, is6829);
        }
    }
    return 0;
}

static int save_arl_table(void)
{
    uint16_t v16;
    int timeout, first = 1;
    uint32_t first_mac_lo = 0, first_vid_mac_hi = 0;

    v16 = ARL_SRCH_CTRL_START_DONE;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);

    valid_arl_entries = 0;
    while (v16 & ARL_SRCH_CTRL_START_DONE) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        timeout = 1000;
        while((v16 & ARL_SRCH_CTRL_SR_VALID) == 0) {
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
            if (v16 & ARL_SRCH_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0) {
                    return 0;
                }
            } else {
                return 0;
            }
        }
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY,
                   (uint8_t *)&arl_lo_entries[valid_arl_entries], 4);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_VID_MAC_HI_ENTRY,
                   (uint8_t *)&arl_hi_entries[valid_arl_entries], 4);
        if (first) {
            first_mac_lo = arl_lo_entries[valid_arl_entries];
            first_vid_mac_hi = arl_hi_entries[valid_arl_entries];
            first = 0;
        } else if ((first_mac_lo == arl_lo_entries[valid_arl_entries]) &&
                   (first_vid_mac_hi == arl_hi_entries[valid_arl_entries])) {
            break;
        }
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY,
                   (uint8_t *)&arl_data_entries[valid_arl_entries], 2);
        arl_data_entries[valid_arl_entries] =
            arl_data_entries[valid_arl_entries] >> 1 | 0x8000;
        BCM_ENET_DEBUG("Low = %x; ",
                       (unsigned int)arl_lo_entries[valid_arl_entries]);
        BCM_ENET_DEBUG("High = %x; ",
                       (unsigned int)arl_hi_entries[valid_arl_entries]);
        BCM_ENET_DEBUG("Data = %x \n",
                       (unsigned short)arl_data_entries[valid_arl_entries]);
        if ((valid_arl_entries++) > NUM_ARL_ENTRIES) {
            break;
        }
    }
    BCM_ENET_DEBUG("valid_arl_entries = %d \n", valid_arl_entries);
    return 0;
}

static int restore_arl_table(void)
{
    uint8_t v8;
    uint16_t v16;
    uint32_t v32;
    int i, timeout = 10;


    BCM_ENET_DEBUG("valid_arl_entries = %d \n", valid_arl_entries);
    for (i = 0; i < valid_arl_entries; i++) {
        v16 = arl_lo_entries[i] & 0xFFFF;
        BCM_ENET_DEBUG("Low 16 = %x \n", (unsigned short)v16);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
        v32 = (arl_lo_entries[i] >> 16) | (arl_hi_entries[i] << 16);
        BCM_ENET_DEBUG("High 32 = %x \n", (unsigned int)v32);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_HI, (uint8_t *)&v32, 4);
        v16 = (arl_hi_entries[i] >> 16) & 0xFFFF;
        BCM_ENET_DEBUG("VID = %x \n", (unsigned short)v16);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&v16, 2);
        v8 = 0x81;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        timeout = 10;
        while(v8 & ARL_TBL_CTRL_START_DONE) {
            mdelay(1);
            if (timeout-- <= 0)  {
                break;
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        }
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY,
                   (uint8_t *)&arl_lo_entries[i], 4);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VID_MAC_HI_ENTRY,
                   (uint8_t *)&arl_hi_entries[i], 4);
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY,
                   (uint8_t *)&arl_data_entries[i], 2);
        v8 = 0x80;
        ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        timeout = 10;
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        while(v8 & ARL_TBL_CTRL_START_DONE) {
            mdelay(1);
            if (timeout-- <= 0)  {
                break;
            }
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        }
    }
    return 0;
}

static int save_switch_state(int is6829)
{
    int i = 0;
    ethsw_reg *swreg;

    swreg = &ethsw_cfg_regs[i];
    while (swreg->page != 0xFF) {
        ethsw_rreg_ext(swreg->page, swreg->offset, (uint8 *)&swreg->val, swreg->len, is6829);
        BCM_ENET_DEBUG("page=0x%x; offset=0x%x; val=0x%02x%02x%02x%02x",
                       swreg->page, swreg->offset, swreg->val[3],
                       swreg->val[2], swreg->val[1], swreg->val[0]);
        swreg = &ethsw_cfg_regs[++i];
    }

    if (!is6829) {
        for (i = 0; i < MAX_VLANS; i++) {
            read_vlan_table(i, &vlan_table[i]);
        }

        save_arl_table();
    }

    return 0;
}

static int restore_switch_state(int is6829)
{
    int i = 0;
    ethsw_reg *swreg;

    swreg = &ethsw_cfg_regs[i];
    while (swreg->page != 0xFF) {
        ethsw_wreg_ext(swreg->page, swreg->offset, (uint8 *)&swreg->val, swreg->len, is6829);
        BCM_ENET_DEBUG("page=0x%x; offset=0x%x; val=0x%02x%02x%02x%02x",
                       swreg->page, swreg->offset, swreg->val[3],
                       swreg->val[2], swreg->val[1], swreg->val[0]);

        swreg = &ethsw_cfg_regs[++i];
    }

    if (!is6829) {
        for (i = 0; i < MAX_VLANS; i++) {
            write_vlan_table(i, vlan_table[i]);
        }

        restore_arl_table();
    }

    return 0;
}

/* Reset the Switch */
int reset_switch(int is6829)
{
    unsigned long flags = 0;
    uint32_t dma_state[32];
    uint32_t dma_config[32];
    uint8_t v8;
    uint16_t v16;
    uint32_t v32;
    int i;
    uint32_t num_ports = 0;

    if (!in_interrupt()) {
        local_irq_save(flags);
    }

    /* Don't read the external ports on the 6829 */
    if (is6829)
        num_ports = NUM_RGMII_PORTS;
    else
        num_ports = EPHY_PORTS;

    BCM_ENET_DEBUG("Saving Switch State");
    save_switch_state(is6829);
    BCM_ENET_DEBUG("Saving MIB State");
    save_mib_state(is6829);

    for (i = 0; i < num_ports; i++) {
        if (ETHSW_PHY_GET_PHYID(i) != -1) {
            ethsw_phy_rreg(ETHSW_PHY_GET_PHYID(i) | (is6829 ? BCM_EXT_6829 : 0),
               MII_BMCR, &v16);
        v16 |= BMCR_LOOPBACK;
            ethsw_phy_wreg(ETHSW_PHY_GET_PHYID(i) | (is6829 ? BCM_EXT_6829 : 0),
               MII_BMCR, &v16);
        }
    }

    BCM_ENET_DEBUG("Disable Rx on all ports and Link Down on all ports");
    /* Disable Rx on all Ports. Link Down on all ports*/
    for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
        ethsw_rreg_ext(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1, is6829);
        v8 |= REG_PORT_RX_DISABLE;
        ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1, is6829);

        if (i < (TOTAL_SWITCH_PORTS -1)) {
            ethsw_rreg_ext(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1, is6829);
            v8 &= ~REG_PORT_STATE_LNK;
            v8 |= REG_PORT_STATE_OVERRIDE;
            ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1, is6829);
        } else {
            ethsw_rreg_ext(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1, is6829);
            v8 &= ~REG_CONTROL_MPSO_LINKPASS;
            v8 |= REG_CONTROL_MPSO_MII_SW_OVERRIDE;
            ethsw_wreg_ext(PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE,
                       &v8, 1, is6829);
        }
    }

    if (!is6829) {
        BCM_ENET_DEBUG("Stop the iuDMA");
        SW_DMA->controller_cfg = 0;

        BCM_ENET_DEBUG("Save the iuDMA Config");
        for (i = 0; i < 32; i++) {
            dma_config[i] = *(DMA_CFG + i);
            dma_state[i] = *(DMA_STATE + i);
        }

#if defined(CONFIG_BCM_PKTCMF_MODULE) || defined(CONFIG_BCM_PKTCMF)
        /* Disable Parser/FFE and save CMF state in preparation of switch reset */
        pktCmfIf( PKTCMF_IF_PRE_SYSTEMRESET, NULL, NULL );
#endif
    }

    /* Before reset, need to enable Switch clock that may have been turned
       off in cfe. We can't turn them back off after the reset because figuring
       out whether to turn them off or not would make this code non portable */
#if defined(CONFIG_BCM96816)
    if (is6829) {
        kerSysBcmSpiSlaveRead((unsigned long)(&(PERF->blkEnables)), (unsigned long *)&v32, sizeof(PERF->blkEnables));
        kerSysBcmSpiSlaveWrite((unsigned long)(&(PERF->blkEnables)),
            (v32 | SWPKT_GPON_CLK_EN | SWPKT_USB_CLK_EN), sizeof(PERF->blkEnables));
    }
    else {
        PERF->blkEnables |= SWPKT_GPON_CLK_EN | SWPKT_USB_CLK_EN;
    }
    mdelay(1);
#endif

    BCM_ENET_DEBUG("Issue Switch Reset");
    if (is6829) {
        kerSysBcmSpiSlaveRead((unsigned long)(&(PERF->softResetB)), (unsigned long *)&v32, sizeof(PERF->softResetB));
        kerSysBcmSpiSlaveWrite((unsigned long)(&(PERF->softResetB)),
            (v32 & ~SOFT_RST_SWITCH), sizeof(PERF->softResetB));
    }
    else {
        PERF->softResetB &= ~SOFT_RST_SWITCH;
    }
    mdelay(1);
    if (is6829) {
        kerSysBcmSpiSlaveWrite((unsigned long)(&(PERF->softResetB)),
            (v32 | SOFT_RST_SWITCH), sizeof(PERF->softResetB));
    }
    else {
        PERF->softResetB |= SOFT_RST_SWITCH;
    }

    mdelay(1);
    for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
        v8 = REG_PORT_CTRL_DISABLE;
        ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1, is6829);
    }

    for (i = 0; i < num_ports; i++) {
        if (!port_in_loopback_mode[i]) {
            if (ETHSW_PHY_GET_PHYID(i) != -1) {
                ethsw_phy_rreg(ETHSW_PHY_GET_PHYID(i) | (is6829 ? BCM_EXT_6829 : 0),
                  MII_BMCR, &v16);
            v16 &= (~BMCR_LOOPBACK);
                ethsw_phy_wreg(ETHSW_PHY_GET_PHYID(i) | (is6829 ? BCM_EXT_6829 : 0),
                  MII_BMCR, &v16);
            }
        }
    }

    /* It's possible to learn an address on the wrong port from loopback
     * between the time the switch is reset and when loopback is disabled
     * so clear out the ARL table before we restore it. */
    fast_age_all(1);

#if defined(CONFIG_BCM96816)
    /* Turn these clocks off on the 6829 */
    if (is6829) {
        kerSysBcmSpiSlaveRead((unsigned long)(&(PERF->blkEnables)), (unsigned long *)&v32, sizeof(PERF->blkEnables));
        kerSysBcmSpiSlaveWrite((unsigned long)(&(PERF->blkEnables)),
            (v32 & ~SWPKT_GPON_CLK_EN & ~SWPKT_USB_CLK_EN), sizeof(PERF->blkEnables));
    }
    mdelay(1);
#endif

#if defined(CONFIG_BCM_PKTCMF_MODULE) || defined(CONFIG_BCM_PKTCMF)
    if (!is6829) {
        /* Restore CMF saved state and re-apply configuration */
        BCM_ENET_DEBUG("Restore CMF state");
        pktCmfIf( PKTCMF_IF_POST_SYSTEMRESET, NULL, NULL );
    }
#endif

    BCM_ENET_DEBUG("Restore MIB state");
    restore_mib_state(is6829);
    BCM_ENET_DEBUG("Restore Switch state");

    restore_switch_state(is6829);

    if (!is6829) {
        BCM_ENET_DEBUG("Restore the iuDMA Config");
        for (i = 0; i < 32; i++) {
            *(DMA_CFG + i) = dma_config[i];
            *(DMA_STATE + i) = dma_state[i];
        }

        BCM_ENET_DEBUG("Enable iuDMA");
        SW_DMA->controller_cfg |= DMA_MASTER_EN;
    }

    if (!in_interrupt()) {
        local_irq_restore(flags);
    }

    return BCM_E_NONE;
}

/*******************************************************************************
 * CHIP:  BCM6816_A0
 * REGISTER:  WATCH_DOG_RPT3
 * BLOCKS:   SYS
 * DESC:     Watch Dog Report 3 Registrer
 * SIZE:     16
 * FIELDS:
 *     ARL_HUNG_INDICATOR ARL Indicator.Bit 8:0 = Port 8:01: Means hung case happen0: Means Ok
 *     RESERVED_R       Reserved
 *
 ******************************************************************************/
#define BCM6816_A0_WATCH_DOG_RPT3r 0x7e
int ethsw_check_arl_hung_indicator(void) {

  uint16_t val;
  ethsw_rreg(PAGE_CONTROL,BCM6816_A0_WATCH_DOG_RPT3r, (uint8_t *)&val, 2);

  return val!=0;
}
#define NUM_POLLS_FOR_LOCKUP 10
#define LOCKUP_MIN_RXBUFF_CNT 0x3C0
int ethsw_is_switch_locked(void *ptr)
{
    static uint16 last_peak = 0;
    static int num_peak_unchanged = 0;
    uint16 cur_peak = 0;
    static uint32 last_ucast_pkts[TOTAL_SWITCH_PORTS] = {0};
    static uint32 last_mcast_pkts[TOTAL_SWITCH_PORTS] = {0};
    static uint32 last_bcast_pkts[TOTAL_SWITCH_PORTS] = {0};
    static int num_pktcnt_unchanged = 0;
    int i, ports_unchanged = 0;
    volatile EthSwMIBRegs *e;

    if (ethsw_check_arl_hung_indicator())
    {
        /* Reset the variables for next detection */
        num_peak_unchanged = 0;
        num_pktcnt_unchanged = 0;
        return 1;
    }

    /* If ARL hung register has't detected the lock; proceed with counter based detection */
    ethsw_rreg(PAGE_FLOW_CTRL, REG_PEAK_RXBUF_COUNT, (uint8 *)&cur_peak, 2);
    ethsw_rreg(PAGE_FLOW_CTRL, REG_PEAK_RXBUF_COUNT, (uint8 *)&cur_peak, 2);
    if (cur_peak < LOCKUP_MIN_RXBUFF_CNT) {
        return 0;
    }
    BCM_ENET_DEBUG("cur_peak = 0x%x \n", cur_peak);
    if(cur_peak == last_peak) {
        if(num_peak_unchanged <= NUM_POLLS_FOR_LOCKUP) {
            num_peak_unchanged++;
        }
    } else {
        last_peak = cur_peak;
        num_peak_unchanged = 0;
  return 0;
    }
    BCM_ENET_DEBUG("num times peak_unchanged = %d \n", num_peak_unchanged);

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

    for(i = 0; i < TOTAL_SWITCH_PORTS; i++) {
        e = ((volatile EthSwMIBRegs *) (SWITCH_BASE + 0x2000 + (i * 0x100)));
        BCM_ENET_DEBUG("Port %d: e->TxUnicastPkts = %u \n", i, e->TxUnicastPkts);
        BCM_ENET_DEBUG("Port %d: e->TxMulticastPkts = %u \n", i, e->TxMulticastPkts);
        BCM_ENET_DEBUG("Port %d: e->TxBroadcastPkts = %u \n", i, e->TxBroadcastPkts);

        if( (e->TxUnicastPkts == last_ucast_pkts[i]) &&
            (e->TxMulticastPkts == last_mcast_pkts[i]) &&
            (e->TxBroadcastPkts == last_bcast_pkts[i]) ) {
                ports_unchanged++;
        } else {
            last_ucast_pkts[i] = e->TxUnicastPkts;
          last_mcast_pkts[i] = e->TxMulticastPkts;
          last_bcast_pkts[i] = e->TxBroadcastPkts;
        }
    }
    if(ports_unchanged >= TOTAL_SWITCH_PORTS) {
        if (num_pktcnt_unchanged <= NUM_POLLS_FOR_LOCKUP) {
            num_pktcnt_unchanged++;
        }
    } else {
        num_pktcnt_unchanged = 0;
      return 0;
    }
    BCM_ENET_DEBUG("num times pktcnt unchanged = %d \n", num_pktcnt_unchanged);

    if( (num_peak_unchanged >= NUM_POLLS_FOR_LOCKUP) &&
        (num_pktcnt_unchanged >= NUM_POLLS_FOR_LOCKUP) ) {

        BCM_ENET_ERROR("lockup detected (%d, %d) \n", num_peak_unchanged, num_pktcnt_unchanged);

        num_peak_unchanged = 0;
        num_pktcnt_unchanged = 0;
        return 1;
    }

    return 0;
}
#endif

/************** Debug Functions ******************/
static void ethsw_dump_page0(int bExt6829) {
#ifdef BCM_FULL_SRC
    int i = 0, page = 0;
    volatile EthSwRegs *e = ETHSWREG;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

    if (0 == bExt6829)
    {
        printk("#The Page0 Registers \n");
        for (i=0; i<9; i++) {
            printk("%02x %02x = 0x%02x (%u) \n", page,
                   ((int)&e->port_traffic_ctrl[i]) & 0xFF, e->port_traffic_ctrl[i],
                   e->port_traffic_ctrl[i]); /* 0x00 - 0x08 */
        }
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->switch_mode) & 0xFF,
                e->switch_mode, e->switch_mode); /* 0x0b */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->pause_quanta) & 0xFF,
                e->pause_quanta, e->pause_quanta); /*0x0c */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->imp_port_state) & 0xFF,
                e->imp_port_state, e->imp_port_state); /*0x0e */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->led_refresh) & 0xFF,
                e->led_refresh, e->led_refresh); /* 0x0f */
        for (i=0; i<2; i++) {
            printk("%02x %02x 0x%04x (%u) \n", page,
                    ((int)&e->led_function[i]) & 0xFF, e->led_function[i],
                    e->led_function[i]); /* 0x10 */
        }
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->led_function_map) & 0xFF,
               e->led_function_map, e->led_function_map); /* 0x14 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->led_enable_map) & 0xFF,
                e->led_enable_map, e->led_enable_map); /* 0x16 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->led_mode_map0) & 0xFF,
               e->led_mode_map0, e->led_mode_map0); /* 0x18 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->led_function_map1) & 0xFF,
               e->led_function_map1, e->led_function_map1); /* 0x1a */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->reserved2[3]) & 0xFF,
               e->reserved2[3], e->reserved2[3]); /* 0x1f */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->port_forward_ctrl) & 0xFF,
               e->port_forward_ctrl, e->port_forward_ctrl); /* 0x21 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->protected_port_selection)
               & 0xFF, e->protected_port_selection,
               e->protected_port_selection); /* 0x24 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->wan_port_select) & 0xFF,
               e->wan_port_select, e->wan_port_select); /* 0x26 */
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->pause_capability)
               & 0xFF, e->pause_capability, e->pause_capability);/*0x28*/
        printk("%02x %02x 0x%02x (%u) \n", page,
        ((int)&e->reserved_multicast_control) & 0xFF, e->reserved_multicast_control,
        e->reserved_multicast_control); /* 0x2f */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->txq_flush_mode_control) &
        0xFF, e->txq_flush_mode_control, e->txq_flush_mode_control); /* 0x31 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->ulf_forward_map) & 0xFF,
               e->ulf_forward_map, e->ulf_forward_map); /* 0x32 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->mlf_forward_map) & 0xFF,
               e->mlf_forward_map, e->mlf_forward_map); /* 0x34 */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->mlf_impc_forward_map) &
         0xFF, e->mlf_impc_forward_map, e->mlf_impc_forward_map); /* 0x36 */
        printk("%02x %02x 0x%04x (%u) \n", page,
         ((int)&e->pause_pass_through_for_rx) & 0xFF, e->pause_pass_through_for_rx,
         e->pause_pass_through_for_rx); /* 0x38 */
        printk("%02x %02x 0x%04x (%u) \n", page,
         ((int)&e->pause_pass_through_for_tx) & 0xFF, e->pause_pass_through_for_tx,
         e->pause_pass_through_for_tx); /* 0x3a */
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->disable_learning) & 0xFF,
               e->disable_learning, e->disable_learning); /* 0x3c */
	
        for (i=0; i<8; i++) {
            printk("%02x %02x 0x%02x (%u) \n", page,
             ((int)&e->port_state_override[i]) & 0xFF, e->port_state_override[i],
             e->port_state_override[i]); /* 0x58 - 0x5f */
        }
#if defined(CONFIG_BCM96816)
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->imp_rgmii_ctrl_p2) &
               0xFF, e->imp_rgmii_ctrl_p2, e->imp_rgmii_ctrl_p2); /* 0x64 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->imp_rgmii_ctrl_p3) &
               0xFF, e->imp_rgmii_ctrl_p3, e->imp_rgmii_ctrl_p3); /* 0x65 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->rgmii_timing_delay_p2) &
         0xFF, e->rgmii_timing_delay_p2, e->rgmii_timing_delay_p2); /* 0x6c */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->gmii_timing_delay_p3) &
         0xFF, e->gmii_timing_delay_p3, e->gmii_timing_delay_p3); /* 0x6d */
#else
       printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->imp_rgmii_ctrl_p4) &
              0xFF, e->imp_rgmii_ctrl_p4, e->imp_rgmii_ctrl_p4); /* 0x64 */
       printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->imp_rgmii_ctrl_p5) &
              0xFF, e->imp_rgmii_ctrl_p5, e->imp_rgmii_ctrl_p5); /* 0x65 */
       printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->rgmii_timing_delay_p4) &
        0xFF, e->rgmii_timing_delay_p4, e->rgmii_timing_delay_p4); /* 0x6c */
       printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->gmii_timing_delay_p5) &
         0xFF, e->gmii_timing_delay_p5, e->gmii_timing_delay_p5); /* 0x6d */
#endif /*CONFIG_BCM96816*/
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->software_reset) & 0xFF,
         e->software_reset, e->software_reset); /* 0x79 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->pause_frame_detection) &
         0xFF, e->pause_frame_detection, e->pause_frame_detection); /* 0x80 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->fast_aging_ctrl) & 0xFF,
               e->fast_aging_ctrl, e->fast_aging_ctrl); /* 0x88 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->fast_aging_port) & 0xFF,
               e->fast_aging_port, e->fast_aging_port); /* 0x89 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)&e->fast_aging_vid) & 0xFF,
               e->fast_aging_vid, e->fast_aging_vid); /* 0x8a */
#if defined(CONFIG_BCM96816)
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->swpkt_ctrl_usb) & 0xFF,
               e->swpkt_ctrl_usb, e->swpkt_ctrl_usb); /*0xa0 */
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->swpkt_ctrl_gpon) & 0xFF,
               e->swpkt_ctrl_gpon, e->swpkt_ctrl_gpon); /*0xa4 */
#else
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->swpkt_ctrl_sar) & 0xFF,
               e->swpkt_ctrl_sar, e->swpkt_ctrl_sar); /*0xa0 */
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->swpkt_ctrl_usb) & 0xFF,
               e->swpkt_ctrl_usb, e->swpkt_ctrl_usb); /*0xa4 */
#endif /*CONFIG_BCM96816*/
        printk("%02x %02x 0x%04x (%u) \n", page, ((int)&e->iudma_ctrl) & 0xFF,
               e->iudma_ctrl, e->iudma_ctrl); /*0xa8 */
#if defined(CONFIG_BCM96816)
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->iudma_queue_ctrl) & 0xFF,
               e->iudma_queue_ctrl, e->iudma_queue_ctrl); /*0xac */
#else
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->rxfilt_ctrl) & 0xFF,
               e->rxfilt_ctrl, e->rxfilt_ctrl); /*0xac */
#endif /*CONFIG_BCM96816*/
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->mdio_ctrl) & 0xFF,
               e->mdio_ctrl, e->mdio_ctrl); /*0xb0 */
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->mdio_data) & 0xFF,
               e->mdio_data, e->mdio_data); /*0xb4 */
        printk("%02x %02x 0x%08x (%u) \n", page, ((int)&e->sw_mem_test) & 0xFF,
               e->sw_mem_test, e->sw_mem_test); /*0xe0 */
    }
#if defined(CONFIG_BCM96816)
    else
    {
        unsigned long spiData;
        int addr = SWITCH_BASE;

        printk("#The Page0 Registers \n");
        for (i=0; i<9; i++)
        {
            addr = SWITCH_BASE + offsetof(EthSwRegs, port_traffic_ctrl) + i;
            kerSysBcmSpiSlaveRead(addr, &spiData, 1);
            printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
                   (unsigned char)spiData, (unsigned char)spiData );  /* 0x00 - 0x08 */
        }

        addr = SWITCH_BASE + offsetof(EthSwRegs, switch_mode);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x0b */

        addr = SWITCH_BASE + offsetof(EthSwRegs, pause_quanta);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x0c */

        addr = SWITCH_BASE + offsetof(EthSwRegs, imp_port_state);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x0e */

        addr = SWITCH_BASE + offsetof(EthSwRegs, led_refresh);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x0f */

        for (i=0; i<2; i++)
        {
            addr = SWITCH_BASE + offsetof(EthSwRegs, led_function) + (i*2);
            kerSysBcmSpiSlaveRead(addr, &spiData, 2);
            printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
                   (unsigned short)spiData, (unsigned short)spiData ); /*0x10 */
        }

        addr = SWITCH_BASE + offsetof(EthSwRegs, led_function_map);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x14 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, led_enable_map);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x16 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, led_mode_map0);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x18 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, led_function_map1);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x18 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, reserved2) + 3;
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x1f */

        addr = SWITCH_BASE + offsetof(EthSwRegs, port_forward_ctrl);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x21 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, protected_port_selection);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x24 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, wan_port_select);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x26 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, pause_capability);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned int)spiData, (unsigned int)spiData ); /*0x28 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, reserved_multicast_control);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x2f */

        addr = SWITCH_BASE + offsetof(EthSwRegs, txq_flush_mode_control);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x31 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, ulf_forward_map);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x32 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, mlf_forward_map);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x34 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, mlf_impc_forward_map);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x36 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, pause_pass_through_for_rx);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x38 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, pause_pass_through_for_tx);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x3a */

        addr = SWITCH_BASE + offsetof(EthSwRegs, disable_learning);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0x3c */

        for (i=0; i<8; i++) {
            addr = SWITCH_BASE + offsetof(EthSwRegs, port_state_override) + i;
            kerSysBcmSpiSlaveRead(addr, &spiData, 1);
            printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
                   (unsigned char)spiData, (unsigned char)spiData ); /* 0x58 */
        }

        addr = SWITCH_BASE + offsetof(EthSwRegs, imp_rgmii_ctrl_p2);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x64 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, imp_rgmii_ctrl_p3);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x65 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, rgmii_timing_delay_p2);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x6c */

        addr = SWITCH_BASE + offsetof(EthSwRegs, gmii_timing_delay_p3);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x6d */

        addr = SWITCH_BASE + offsetof(EthSwRegs, software_reset);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x79 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, pause_frame_detection);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x80 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, fast_aging_ctrl);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x88 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, fast_aging_port);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x89 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, fast_aging_vid);
        kerSysBcmSpiSlaveRead(addr, &spiData, 1);
        printk("%02x %02x = 0x%02x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0x8a */

        addr = SWITCH_BASE + offsetof(EthSwRegs, swpkt_ctrl_usb);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned int)spiData, (unsigned int)spiData ); /* 0xa0 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, swpkt_ctrl_gpon);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned char)spiData, (unsigned char)spiData ); /* 0xa4 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, iudma_ctrl);
        kerSysBcmSpiSlaveRead(addr, &spiData, 2);
        printk("%02x %02x = 0x%04x (%u) \n", page, addr & 0xFF,
               (unsigned short)spiData, (unsigned short)spiData ); /*0xa8 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, iudma_queue_ctrl);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned int)spiData, (unsigned int)spiData ); /* 0xac */

        addr = SWITCH_BASE + offsetof(EthSwRegs, mdio_ctrl);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned int)spiData, (unsigned int)spiData ); /* 0xb0 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, mdio_data);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned int)spiData, (unsigned int)spiData ); /* 0xb4 */

        addr = SWITCH_BASE + offsetof(EthSwRegs, sw_mem_test);
        kerSysBcmSpiSlaveRead(addr, &spiData, 4);
        printk("%02x %02x = 0x%08x (%u) \n", page, addr & 0xFF,
               (unsigned int)spiData, (unsigned int)spiData ); /* 0xe0 */
    }
#else
    else
    {
        panic("ethsw_dump_page0: External SPI slave not available\n");
    }
#endif
#else
    printk("bExt6829=%d\n", bExt6829);
#endif  /*BCM_FULL_SRC */
}

#if defined(AEI_VDSL_EXTSW_DEBUG)
void extsw_rreg(int page, int reg, uint8 *data, int len);
void extsw_wreg(int page, int reg, uint8 *data, int len);

static void AEI_extsw_dump_page0(void)
{
    int i;
    uint8 v8 = 0;
    uint16 v16 = 0;
    uint32 v32 = 0;

    for (i = 0; i <= 5; i++)
    {
        extsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + i, (uint8 *)&v8, 1);
        printk("PORT TRAFFIC CONTROL REGISTER %d: %02x = 0x%02x\n", i, (REG_PORT_CTRL + i), v8);
    }

    extsw_rreg(PAGE_CONTROL, REG_MII_PORT_CONTROL, (uint8 *)&v8, 1);
    printk("IMP PORT CONTROL REGISTER: %02x = 0x%02x\n", REG_MII_PORT_CONTROL, v8);

    extsw_rreg(PAGE_CONTROL, REG_SWITCH_MODE, (uint8 *)&v8, 1);
    printk("SWITCH MODE REGISTER: %02x = 0x%02x\n", REG_SWITCH_MODE, v8);
    extsw_rreg(PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, (uint8 *)&v8, 1);
    printk("IMP PORT STATE OVERRIDE REGISTER: %02x = 0x%02x\n", REG_CONTROL_MII1_PORT_STATE_OVERRIDE, v8);

    extsw_rreg(PAGE_CONTROL, REG_LED_REFRESH, (uint8 *)&v8, 1);
    printk("LED REFRESH REGISTER: %02x = 0x%02x\n", REG_LED_REFRESH, v8);

    extsw_rreg(PAGE_CONTROL, REG_LED_FUNCTION0_CTRL, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("LED FUNCTION 0 CONTROL REGISTER: %02x = 0x%04x\n", REG_LED_FUNCTION0_CTRL, v16);

    extsw_rreg(PAGE_CONTROL, REG_LED_FUNCTION1_CTRL, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("LED FUNCTION 1 CONTROL REGISTER: %02x = 0x%04x\n", REG_LED_FUNCTION1_CTRL, v16);

    extsw_rreg(PAGE_CONTROL, REG_LED_FUNCTION_MAP, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("LED FUNCTION MAP REGISTER: %02x = 0x%04x\n", REG_LED_FUNCTION_MAP, v16);

    extsw_rreg(PAGE_CONTROL, REG_LED_ENABLE_MAP, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("LED ENABLE MAP REGISTER: %02x = 0x%04x\n", REG_LED_ENABLE_MAP, v16);

    extsw_rreg(PAGE_CONTROL, REG_LED_MODE_MAP0, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("LED MODE MAP 0 REGISTER: %02x = 0x%04x\n", REG_LED_MODE_MAP0, v16);

    extsw_rreg(PAGE_CONTROL, REG_LED_MODE_MAP1, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("LED MODE MAP 1 REGISTER: %02x = 0x%04x\n", REG_LED_MODE_MAP1, v16);

    extsw_rreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&v8, 1);
    printk("PORT FORWARD CONTROL REGISTER: %02x = 0x%02x\n", REG_PORT_FORWARD, v8);

    extsw_rreg(PAGE_CONTROL, REG_PROTECTED_PORT_MAP, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("PROTECTED PORT SELECTION REGISTER: %02x = 0x%04x\n", REG_PROTECTED_PORT_MAP, v16);

    extsw_rreg(PAGE_CONTROL, REG_WAN_PORT_MAP, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("WAN PORT SELECT REGISTER: %02x = 0x%04x\n", REG_WAN_PORT_MAP, v16);

    extsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8 *)&v32, 4);
    v32 = swab32(v32);
    printk("PAUSE CAPABILITY REGISTER: %02x = 0x%08lx\n", REG_PAUSE_CAPBILITY, v32);

    extsw_rreg(PAGE_CONTROL, REG_RESERVED_MULTICAST_CTRL, (uint8 *)&v8, 1);
    printk("RESERVED MULTICAST CONTROL REGISTER: %02x = 0x%02x\n", REG_RESERVED_MULTICAST_CTRL, v8);

    extsw_rreg(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("UNICAST LOOKUP FAILED FORWARD MAP REGISTER: %02x = 0x%04x\n", REG_UCST_LOOKUP_FAIL, v16);

    extsw_rreg(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("MULTICAST LOOKUP FAILED FORWARD MAP REGISTER: %02x = 0x%04x\n", REG_MCST_LOOKUP_FAIL, v16);

    extsw_rreg(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("ALMLF IPMC FORWARD MAP REGISTER: %02x = 0x%04x\n", REG_IPMC_LOOKUP_FAIL, v16);

    extsw_rreg(PAGE_CONTROL, REG_PAUSE_PASS_THROUGH_RX, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("PAUSE PASS THROUGH FOR RX REGISTER: %02x = 0x%04x\n", REG_PAUSE_PASS_THROUGH_RX, v16);

    extsw_rreg(PAGE_CONTROL, REG_PAUSE_PASS_THROUGH_TX, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("PAUSE PASS THROUGH FOR TX REGISTER: %02x = 0x%04x\n", REG_PAUSE_PASS_THROUGH_TX, v16);

    extsw_rreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("DISABLE LEARNING REGISTER: %02x = 0x%04x\n", REG_DISABLE_LEARNING, v16);

    extsw_rreg(PAGE_CONTROL, REG_SOFTWARE_LEARNING, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("SOFTWARE LEARNING REGISTER: %02x = 0x%04x\n", REG_SOFTWARE_LEARNING, v16);
    for (i = 0; i <= 5; i++)
    {
        extsw_rreg(PAGE_CONTROL, REG_PORT_STATE + i, (uint8 *)&v8, 1);
        printk("PORT STATE OVERRIDE REGISTER %d: %02x = 0x%02x\n", i, (REG_PORT_STATE + i), v8);
    }

    extsw_rreg(PAGE_CONTROL, REG_MDIO_WAN_PORT_ADDR, (uint8 *)&v8, 1);
    printk("MDIO WAN PORT ADDRESS REGISTER: %02x = 0x%02x\n", REG_MDIO_WAN_PORT_ADDR, v8);

    extsw_rreg(PAGE_CONTROL, REG_MDIO_IMP_PORT_ADDR, (uint8 *)&v8, 1);
    printk("MDIO IMP PORT ADDRESS REGISTER: %02x = 0x%02x\n", REG_MDIO_IMP_PORT_ADDR, v8);

    extsw_rreg(PAGE_CONTROL, REG_SW_RESET, (uint8 *)&v8, 1);
    printk("SOFTWARE RESET CONTROL REGISTER: %02x = 0x%02x\n", REG_SW_RESET, v8);

    extsw_rreg(PAGE_CONTROL, REG_FRAME_DETECT_CTRL, (uint8 *)&v8, 1);
    printk("PAUSE FRAME DETECTION CONTROL REGISTER: %02x = 0x%02x\n", REG_FRAME_DETECT_CTRL, v8);

    extsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8 *)&v8, 1);
    printk("FAST-AGING CONTROL REGISTER: %02x = 0x%02x\n", REG_FAST_AGING_CTRL, v8);

    extsw_rreg(PAGE_CONTROL, REG_FAST_AGING_PORT, (uint8 *)&v8, 1);
    printk("FAST-AGING PORT CONTROL REGISTER: %02x = 0x%02x\n", REG_FAST_AGING_PORT, v8);

    extsw_rreg(PAGE_CONTROL, REG_FAST_AGING_VID_CTRL, (uint8 *)&v16, 2);
    v16 = swab16(v16);
    printk("FAST-AGING VID CONTROL REGISTER: %02x = 0x%04x\n", REG_FAST_AGING_VID_CTRL, v16);
}
#endif

/*
* The two functions below allow gathering SW tx/rx counters.
* Is used by wlan driver to stop calibration
* when heavy traffic runs between Eth Port, to avoid Eth pkt loss
*/
unsigned int sw_tx, sw_rx;

/*
* This function gathers the statistics every 1 second from the ethernet
* poll function. Do not create your own polling function that would call it,
* as this would interfere with power management and reduce power savings.
*/
void __ethsw_get_txrx_imp_port_pkts(void)
{
    volatile EthSwMIBRegs *e = ((volatile EthSwMIBRegs *)
                                (SWITCH_BASE + 0x2000 + ((MIPS_PORT_ID) * 0x100)));

    sw_tx = e->TxUnicastPkts + e->TxMulticastPkts + e->TxBroadcastPkts;
    sw_rx = e->RxUnicastPkts + e->RxMulticastPkts + e->RxBroadcastPkts;
    return;
}

/*
* This function provides the packet counts with up to 1 seconds delay.
*/
void ethsw_get_txrx_imp_port_pkts(unsigned int *tx, unsigned int *rx)
{
    *tx = sw_tx;
    *rx = sw_rx;

    return;
}
EXPORT_SYMBOL(ethsw_get_txrx_imp_port_pkts);


int ethsw_dump_mib(int port, int type)
{
    volatile EthSwMIBRegs *e;
    int base;
    int bExt6829 = 0;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

#if defined(CONFIG_BCM96816)
    if (IsExt6829(port))
    {
       bExt6829 = 1;
       port &= ~BCM_EXT_6829;
    }
#endif

    if (port > 8) {
        printk("Invalid port number \n");
        return -1;
    }
    base = (SWITCH_BASE + 0x2000 + (port * 0x100));

    if ( 0 == bExt6829 )
    {
        e = (volatile EthSwMIBRegs *)base;

        /* Display Tx statistics */
        printk("\n");
        printk("TxUnicastPkts:          %10u \n", e->TxUnicastPkts);
        printk("TxMulticastPkts:        %10u \n",  e->TxMulticastPkts);
        printk("TxBroadcastPkts:        %10u \n", e->TxBroadcastPkts);
        printk("TxDropPkts:             %10u \n", e->TxDropPkts);

        /* Display remaining tx stats only if requested */
        if (type) {
            printk("TxOctetsLo:             %10u \n", e->TxOctetsLo);
            printk("TxOctetsHi:             %10u \n", e->TxOctetsHi);
            printk("TxQoSPkts:              %10u \n", e->TxQoSPkts);
            printk("TxCol:                  %10u \n", e->TxCol);
            printk("TxSingleCol:            %10u \n", e->TxSingleCol);
            printk("TxMultipleCol:          %10u \n", e->TxMultipleCol);
            printk("TxDeferredTx:           %10u \n", e->TxDeferredTx);
            printk("TxLateCol:              %10u \n", e->TxLateCol);
            printk("TxExcessiveCol:         %10u \n", e->TxExcessiveCol);
            printk("TxFrameInDisc:          %10u \n", e->TxFrameInDisc);
            printk("TxPausePkts:            %10u \n", e->TxPausePkts);
            printk("TxQoSOctetsLo:          %10u \n", e->TxQoSOctetsLo);
            printk("TxQoSOctetsHi:          %10u \n", e->TxQoSOctetsHi);
        }

        /* Display Rx statistics */
        printk("\n");
        printk("RxUnicastPkts:          %10u \n", e->RxUnicastPkts);
        printk("RxMulticastPkts:        %10u \n", e->RxMulticastPkts);
        printk("RxBroadcastPkts:        %10u \n", e->RxBroadcastPkts);
        printk("RxDropPkts:             %10u \n", e->RxDropPkts);

        /* Display remaining rx stats only if requested */
        if (type) {
            printk("RxJabbers:              %10u \n", e->RxJabbers);
            printk("RxAlignErrs:            %10u \n", e->RxAlignErrs);
            printk("RxFCSErrs:              %10u \n", e->RxFCSErrs);
            printk("RxFragments:            %10u \n", e->RxFragments);
            printk("RxOversizePkts:         %10u \n", e->RxOversizePkts);
            printk("RxExcessSizeDisc:       %10u \n", e->RxExcessSizeDisc);
            printk("RxOctetsLo:             %10u \n", e->RxOctetsLo);
            printk("RxOctetsHi:             %10u \n", e->RxOctetsHi);
            printk("RxUndersizePkts:        %10u \n", e->RxUndersizePkts);
            printk("RxPausePkts:            %10u \n", e->RxPausePkts);
            printk("RxGoodOctetsLo:         %10u \n", e->RxGoodOctetsLo);
            printk("RxGoodOctetsHi:         %10u \n", e->RxGoodOctetsHi);
            printk("RxSAChanges:            %10u \n", e->RxSAChanges);
            printk("RxSymbolError:          %10u \n", e->RxSymbolError);
            printk("RxQoSPkts:              %10u \n", e->RxQoSPkts);
            printk("RxQoSOctetsLo:          %10u \n", e->RxQoSOctetsLo);
            printk("RxQoSOctetsHi:          %10u \n", e->RxQoSOctetsHi);
            printk("RxPkts64Octets:         %10u \n", e->Pkts64Octets);
            printk("RxPkts65to127Octets:    %10u \n", e->Pkts65to127Octets);
            printk("RxPkts128to255Octets:   %10u \n", e->Pkts128to255Octets);
            printk("RxPkts256to511Octets:   %10u \n", e->Pkts256to511Octets);
            printk("RxPkts512to1023Octets:  %10u \n", e->Pkts512to1023Octets);
            printk("RxPkts1024to1522Octets: %10u \n", e->Pkts1024to1522Octets);
            printk("RxPkts1523to2047:       %10u \n", e->Pkts1523to2047);
            printk("RxPkts2048to4095:       %10u \n", e->Pkts2048to4095);
            printk("RxPkts4096to8191:       %10u \n", e->Pkts4096to8191);
            printk("RxPkts8192to9728:       %10u \n", e->Pkts8192to9728);
        }
    }
#if defined(CONFIG_BCM96816)
    else
    {
        /* Display Tx statistics */
        printk("\n");
        printk("TxUnicastPkts:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxUnicastPkts)) );
        printk("TxMulticastPkts:        %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxMulticastPkts)) );
        printk("TxBroadcastPkts:        %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxBroadcastPkts)) );
        printk("TxDropPkts:             %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxDropPkts)) );

        /* Display remaining tx stats only if requested */
        if (type) {
             printk("TxOctetsLo:             %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxOctetsLo)) );
             printk("TxOctetsHi:             %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxOctetsHi)) );
             printk("TxQoSPkts:              %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxQoSPkts)) );
             printk("TxCol:                  %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxCol)) );
             printk("TxSingleCol:            %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxSingleCol)) );
             printk("TxMultipleCol:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxMultipleCol)) );
             printk("TxDeferredTx:           %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxDeferredTx)) );
             printk("TxLateCol:              %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxLateCol)) );
             printk("TxExcessiveCol:         %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxExcessiveCol)) );
             printk("TxFrameInDisc:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxFrameInDisc)) );
             printk("TxPausePkts:            %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxPausePkts)) );
             printk("TxQoSOctetsLo:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxQoSOctetsLo)) );
             printk("TxQoSOctetsHi:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,TxQoSOctetsHi)) );
        }

        /* Display Rx statistics */
        printk("\n");
        printk("RxUnicastPkts:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxUnicastPkts)) );
        printk("RxMulticastPkts:        %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxMulticastPkts)) );
        printk("RxBroadcastPkts:        %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxBroadcastPkts)) );
        printk("RxDropPkts:             %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxDropPkts)) );

        /* Display remaining rx stats only if requested */
        if (type) {
             printk("RxJabbers:              %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxJabbers)) );
             printk("RxAlignErrs:            %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxAlignErrs)) );
             printk("RxFCSErrs:              %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxFCSErrs)) );
             printk("RxFragments:            %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxFragments)) );
             printk("RxOversizePkts:         %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxOversizePkts)) );
             printk("RxExcessSizeDisc:       %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxExcessSizeDisc)) );
             printk("RxOctetsLo:             %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxOctetsLo)) );
             printk("RxOctetsHi:             %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxOctetsHi)) );
             printk("RxUndersizePkts:        %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxUndersizePkts)) );
             printk("RxPausePkts:            %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxPausePkts)) );
             printk("RxGoodOctetsLo:         %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxGoodOctetsLo)) );
             printk("RxGoodOctetsHi:         %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxGoodOctetsHi)) );
             printk("RxSAChanges:            %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxSAChanges)) );
             printk("RxSymbolError:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxSymbolError)) );
             printk("RxQoSPkts:              %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxQoSPkts)) );
             printk("RxQoSOctetsLo:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxQoSOctetsLo)) );
             printk("RxQoSOctetsHi:          %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,RxQoSOctetsHi)) );
             printk("RxPkts64Octets:         %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts64Octets)) );
             printk("RxPkts65to127Octets:    %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts65to127Octets)) );
             printk("RxPkts128to255Octets:   %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts128to255Octets)) );
             printk("RxPkts256to511Octets:   %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts256to511Octets)) );
             printk("RxPkts512to1023Octets:  %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts512to1023Octets)) );
             printk("RxPkts1024to1522Octets: %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts1024to1522Octets)) );
             printk("RxPkts1523to2047:       %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts1523to2047)) );
             printk("RxPkts2048to4095:       %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts2048to4095)) );
             printk("RxPkts4096to8191:       %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts4096to8191)) );
             printk("RxPkts8192to9728:       %10lu \n", kerSysBcmSpiSlaveReadReg32(base+ offsetof(EthSwMIBRegs,Pkts8192to9728)) );
        }
    }
#else
    else
    {
        panic("ethsw_dump_mib: External SPI slave not available\n");
    }
#endif

    return 0;
}

void ethsw_dump_page(int page) {
    int bExt6829 = 0;

#if defined(CONFIG_BCM96816)
    bExt6829 = IsExt6829(page);
    switch (page & ~BCM_EXT_6829) {
#else
    switch (page) {
#endif
        case 0:
            ethsw_dump_page0(bExt6829);
            break;

        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
            ethsw_dump_mib(page - 0x20, 1);
            break;
#if defined(AEI_VDSL_EXTSW_DEBUG)
        case 0x1:
            AEI_extsw_dump_page0();
            break;
#endif
        default:
            printk("Invalid page or not yet implemented \n");
            break;
    }
}



uint32 ConfigureJumboPort(uint32 regVal, int portVal, unsigned int configVal) // bill
{
  UINT32 controlBit;

  // Test for valid port specifier.
  if ((portVal >= ETHSWCTL_JUMBO_PORT_GPHY_0) && (portVal <= ETHSWCTL_JUMBO_PORT_ALL))
  {
    // Switch on port ID.
    switch (portVal)
    {
    case ETHSWCTL_JUMBO_PORT_MIPS:
      controlBit = ETHSWCTL_JUMBO_PORT_MIPS_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_GPON:
      controlBit = ETHSWCTL_JUMBO_PORT_GPON_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_USB:
      controlBit = ETHSWCTL_JUMBO_PORT_USB_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_MOCA:
      controlBit = ETHSWCTL_JUMBO_PORT_MOCA_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_GPON_SERDES:
      controlBit = ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_GMII_2:
      controlBit = ETHSWCTL_JUMBO_PORT_GMII_2_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_GMII_1:
      controlBit = ETHSWCTL_JUMBO_PORT_GMII_1_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_GPHY_1:
      controlBit = ETHSWCTL_JUMBO_PORT_GPHY_1_MASK;
      break;
    case ETHSWCTL_JUMBO_PORT_GPHY_0:
      controlBit = ETHSWCTL_JUMBO_PORT_GPHY_0_MASK;
      break;
    default: // ETHSWCTL_JUMBO_PORT_ALL:
      controlBit = ETHSWCTL_JUMBO_PORT_MASK_VAL;  // ALL bits
      break;
    }

    // Test for accept JUMBO frames.
    if (configVal != 0)
    {
      // Setup register value to accept JUMBO frames.
      regVal |= controlBit;
    }
    else
    {
      // Setup register value to reject JUMBO frames.
      regVal &= ~controlBit;
    }
  }

  // Return new JUMBO configuration control register value.
  return regVal;
}


int enet_ioctl_ethsw_port_jumbo_control(struct ethswctl_data *e)  // bill
{
    uint32 val32;

    if (e->type == TYPE_GET) {
      // Read & log current JUMBO configuration control register.
      ethsw_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
      BCM_ENET_DEBUG("JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

      // Attempt to transfer register read value to user space & test for success.
      if (copy_to_user((void*)(&e->ret_val), (void*)&val32, sizeof(int)))
      {
          // Report failure.
          return -EFAULT;
      }
    } else {
      // Setup JUMBO configuration frame size register.
      val32 = MAX_JUMBO_FRAME_SIZE;
      ethsw_wreg(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE, (uint8 *)&val32, 4);

      // Read & log current JUMBO configuration control register.
      ethsw_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
      BCM_ENET_DEBUG("Old JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

      // Setup JUMBO configuration control register.
      val32 = ConfigureJumboPort(val32, e->port, e->val);
      ethsw_wreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);

      // Attempt to transfer register write value to user space & test for success.
      if (copy_to_user((void*)(&e->ret_val), (void*)&val32, sizeof(int)))
      {
          // Report failure.
          return -EFAULT;
      }
    }

    return BCM_E_NONE;
}

#ifdef NO_CFE
void ethsw_reset(int is6829)
{
    // Power up and reset EPHYs
    GPIO->RoboswEphyCtrl = 0;
    msleep(1);

#if !defined(CONFIG_BCM96816)
    // Take EPHYs out of reset
    GPIO->RoboswEphyCtrl = EPHY_RST_4 | EPHY_RST_3 | EPHY_RST_2 | EPHY_RST_1;
    msleep(1);
#endif

#if defined(CONFIG_BCM963268)
    // Take GPHY out of low pwer and disable IDDQ
    GPIO->RoboswGphyCtrl &= ~( GPHY_IDDQ_BIAS | GPHY_LOW_PWR );
    msleep(1);

    // Bring internal GigPhy out of reset
    PERF->softResetB &= ~SOFT_RST_GPHY;
    msleep(1);
#endif


#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    GPIO->RoboswSwitchCtrl |= (RSW_MII_DUMB_FWDG_EN | RSW_HW_FWDG_EN);
#endif
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    GPIO->RoboswEphyCtrl |= (RSW_MII_DUMB_FWDG_EN | RSW_HW_FWDG_EN);
#endif

    // Enable Switch clock
    PERF->blkEnables |= ROBOSW_CLK_EN;
#if defined(CONFIG_BCM96368)
    PERF->blkEnables |= SWPKT_SAR_CLK_EN | SWPKT_USB_CLK_EN;
#endif
#if defined(CONFIG_BCM96816)
    PERF->blkEnables |= SWPKT_GPON_CLK_EN | SWPKT_USB_CLK_EN;
#endif
    msleep(1);

    PERF->softResetB &= ~SOFT_RST_SWITCH;
    msleep(1);
    PERF->softResetB |= SOFT_RST_SWITCH;
    msleep(1);

#if defined(CONFIG_BCM96816)
    /* After reset, we can disable unused switch clocks
       Only disabling the Switch GPON CLK really saves power
       so no need to bother for the other clocks on other chips */
    {
        /* Check if this is BHR */
        if (is6829)
        {
            /* BHR does not need GPON and USB Switch ports */
            PERF->blkEnables &= ~(SWPKT_GPON_CLK_EN | SWPKT_USB_CLK_EN);
        }
    }
#endif

}
#endif

#define ALL_PORTS_MASK                     0x1FF
#define ONE_TO_ONE_MAP                     0x00FAC688
#define MOCA_QUEUE_MAP                     0x0091B492
#define DEFAULT_FC_CTRL_VAL                0x1F
#if defined(CONFIG_BCM96816)
/* Tx: 0->2, 1->3, 2->4, 3->5. */
#define DEFAULT_IUDMA_QUEUE_SEL            0xB1A
#else
/* Tx: 0->0, 1->1, 2->2, 3->3. */
#define DEFAULT_IUDMA_QUEUE_SEL            0x688
#endif
void ethsw_init_hw(int unit, uint32_t portMap,  int wanPortMap, int is6829)
{
#if defined(CONFIG_BCM96816) || !defined(SUPPORT_SWMDK)
  int i;
  uint8 v8;
  uint32 v32;
  uint16 v16;
#if defined(CONFIG_BCM96816)
  uint8 wrr_queue_weights[NUM_EGRESS_QUEUES] = {0x1, 0x1, 0x1, 0x8, 0x20, 0x31, 0x31, 0x31};
#else
  uint8 wrr_queue_weights[NUM_EGRESS_QUEUES] = {[0 ... (NUM_EGRESS_QUEUES-1)] = 0x1};
#endif
#endif

#if !defined(SUPPORT_SWMDK)
  for (i=0; i<(TOTAL_SWITCH_PORTS - 1); i++)
  {
    v8 = ((portMap % 2) != 0)? 0: REG_PORT_CTRL_DISABLE;
    ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1);
    portMap /= 2;
  }

  /* Set IMP port RMII mode */
  v8 = 0;
  ethsw_wreg(PAGE_CONTROL, REG_MII_PORT_CONTROL, &v8, 1);

  // Enable the GMII clocks.
  for (i = 0; i < NUM_RGMII_PORTS; i++) {
    ethsw_rreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
    v8 |= REG_RGMII_CTRL_ENABLE_GMII;
    ethsw_wreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
  }

  /* RGMII Delay Programming. Enable ID mode */
  for (i = 0; i < NUM_RGMII_PORTS; i++) {
    /* Enable ID mode */
    ethsw_rreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
    v8 |= REG_RGMII_CTRL_TIMING_SEL;
#if defined(CONFIG_BCM963268)
    /* Force RGMII mode as these port support only RGMII */
    v8 |= REG_RGMII_CTRL_ENABLE_RGMII_OVERRIDE;
#endif
    ethsw_wreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
  }

    /* Put switch in frame management mode. */
    ethsw_rreg(PAGE_CONTROL, REG_SWITCH_MODE, &v8, sizeof(v8));
    v8 |= REG_SWITCH_MODE_FRAME_MANAGE_MODE;
    v8 |= REG_SWITCH_MODE_SW_FWDG_EN;
    ethsw_wreg(PAGE_CONTROL, REG_SWITCH_MODE, &v8, sizeof(v8));

    // MII port
    v8 = 0xa0;
    v8 |= REG_MII_PORT_CONTROL_RX_UCST_EN;
    v8 |= REG_MII_PORT_CONTROL_RX_MCST_EN;
    v8 |= REG_MII_PORT_CONTROL_RX_BCST_EN;
    ethsw_wreg(PAGE_CONTROL, REG_MII_PORT_CONTROL, &v8, sizeof(v8));

    // Management port configuration
    v8 = ENABLE_MII_PORT | RECEIVE_IGMP | RECEIVE_BPDU;
    ethsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &v8, sizeof(v8));

#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96328) 
    /*Don't enable flow control on 6816 and FAP based systems as FAP does not burst into the switch like 
          MIPS does (due to MIPS going out for snacks (like WLAN calibration) once in a while). 
          For 6368/6328, the back-pressure is needed to pass the 100Mbps DS zero packet loss 
          test as MIPS can burst into switch without flow control. If needed, the back pressure can be replaced with 
          the mechanism of polling for availability of switch buffers before sending packets to switch 
          (adds overhead to hard_xmit function and affects throughput performance but does not break QoS)*/
    v32 = REG_PAUSE_CAPBILITY_OVERRIDE | REG_PAUSE_CAPBILITY_MIPS_TX;
    ethsw_wreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8 *)&v32, sizeof(v32));
#endif

    /* Configure the switch to use Desc priority */
    ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8 *)&v32, 4);
    v32 |= REG_IUDMA_CTRL_USE_DESC_PRIO;
    ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8 *)&v32, 4);

#if defined(CONFIG_BCM_GPON_802_1Q_ENABLED)
    /* Enable tag_status_preserve and VID_FFE forward bits */
    v8 = (TAG_STATUS_PRESERVE_10 << TAG_STATUS_PRESERVE_S) | (1 << VID_FFF_ENABLE_S);
    ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5, (uint8 *)&v8, 1);
    /* Set the default VID for all switch ports (exluding MIPS) to 0xFFF */
    for (i = 0; i < (TOTAL_SWITCH_PORTS-1); i++) {
        ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG, (uint8 *)&v16, 2);
        /* Clear the VID bits */
        v16 &= (~DEFAULT_TAG_VID_M);
        v16 |= (0xFFF << DEFAULT_TAG_VID_S);
        ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (i*2), (uint8 *)&v16, 2);
    }
    /* Enable 802.1Q mode */
    ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, (uint8 *)&v8, 1);
    v8 |=  (1 << VLAN_EN_8021Q_S);
    ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, (uint8 *)&v8, 1);
#else
    /* Disable tag_status_preserve */
    v8 = 0;
    ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5, (uint8 *)&v8, 1);
#endif

#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM96368) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    /* Enable multiple queues and scheduling mode */
    v8 = (1 << TXQ_CTRL_TXQ_MODE_S) |
         (DEFAULT_HQ_PREEMPT_EN << TXQ_CTRL_HQ_PREEMPT_S);
    ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &v8, 1);
#endif

    /* Set the queue weights. */
    for (i = 0; i < NUM_EGRESS_QUEUES; i++) {
        ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i,
            &wrr_queue_weights[i], sizeof(wrr_queue_weights[i]));
    }

    v16 = 0;
    ethsw_wreg(PAGE_QOS, REG_QOS_THRESHOLD_CTRL, (uint8 *)&v16, 2);

    /* Set the default flow control value as desired */
    v16 = DEFAULT_FC_CTRL_VAL;
    ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8 *)&v16, 2);

    // Forward lookup failure to use ULF/MLF/IPMC lookup fail registers */
    v8 = (REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IP_MCST);
    ethsw_wreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&v8, sizeof(v8));

    // Forward unlearned unicast and unresolved mcast to the MIPS
    v16 = PBMAP_MIPS;
    ethsw_wreg(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16));
    ethsw_wreg(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16));
    ethsw_wreg(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16));

#ifdef CONFIG_BCM96816
    v16 = PBMAP_MIPS;
    ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (GPON_PORT_ID * 2),
               (uint8 *)&v16, 2);
#endif

#if defined(CONFIG_BCM96368)
   v16 = REG_MIRROR_ENABLE | (MIPS_PORT_ID & REG_CAPTURE_PORT_M);
   ethsw_wreg(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL,(uint8 *)&v16, 2);
#endif

#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
    v32 = DEFAULT_IUDMA_QUEUE_SEL;
    ethsw_wreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8 *)&v32, 4);
#endif
#endif // #if !defined(SUPPORT_SWMDK)

    if (unit == 1) {
        if (wanPortMap & 0xFF)
            extsw_set_wanoe_portmap(wanPortMap & 0xFF);
#if defined(CONFIG_BCM963268)
        if (wanPortMap >> MAX_EXT_SWITCH_PORTS)
            ethsw_set_wanoe_portmap(wanPortMap >> MAX_EXT_SWITCH_PORTS);
#endif
    } else {
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
        ethsw_set_wanoe_portmap(wanPortMap);
#else
        ethsw_port_based_vlan(portMap, wanPortMap, 0);
#endif
    }

#if defined(CONFIG_BCM96816)
    /* Enable the 802.1p based QoS by default */
    v8 = 0;
    ethsw_wreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, &v8, 1);
    v16 = ALL_PORTS_MASK;
    ethsw_wreg(PAGE_QOS, REG_QOS_8021P_EN, (uint8 *)&v16, 2);
    v32 = ONE_TO_ONE_MAP;
    ethsw_wreg(PAGE_QOS, REG_QOS_8021P_PRIO_MAP, (uint8 *)&v32, 4);
    for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
        ethsw_wreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (i * 4),
                   (uint8 *)&v32, 4);
    }

    /*Egress rate limit moca ports to 200 Mbps*/
    v16 = (1<<ERC_ERC_EN_S) | (0<<ERC_BKT_SIZE_S) | (0x8c<<ERC_RFSH_CNT_S);
    ethsw_wreg_ext(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + 2*MOCA_PORT_ID,
                   (uint8*)&v16, sizeof(v16), 0);

    if (is6829)
    {
      v8 = REG_PORT_STATE_OVERRIDE | REG_PORT_STATE_1000 |
         REG_PORT_STATE_FDX | REG_PORT_STATE_LNK;
      ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_STATE + SERDES_PORT_ID,
                     &v8, sizeof(v8), 0 );
      ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_STATE + SERDES_PORT_ID,
                     &v8, sizeof(v8), 1 );

      /*Egress rate limit moca ports to 200 Mbps*/
      v16 = (1<<ERC_ERC_EN_S) | (0<<ERC_BKT_SIZE_S) | (0x8c<<ERC_RFSH_CNT_S);
      ethsw_wreg_ext(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + 2*MOCA_PORT_ID,
                     (uint8*)&v16, sizeof(v16), 1);

      /* Enable multiple queues and scheduling mode on 6829*/
      v8 = (1 << TXQ_CTRL_TXQ_MODE_S) |
           (DEFAULT_HQ_PREEMPT_EN << TXQ_CTRL_HQ_PREEMPT_S);
      ethsw_wreg_ext(PAGE_QOS, REG_QOS_TXQ_CTRL, &v8, sizeof(v8), 1);
      /* Set the queue weights on 6829. */
      for (i = 0; i < NUM_EGRESS_QUEUES; i++) {
         ethsw_wreg_ext(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i,
            &wrr_queue_weights[i], sizeof(wrr_queue_weights[i]), 1);
      }

      /* Enable the 802.1p based QoS by default */
      v8 = 0;
      ethsw_wreg_ext(PAGE_QOS, REG_QOS_GLOBAL_CTRL, &v8, 1, 1);
      v16 = ALL_PORTS_MASK;
      ethsw_wreg_ext(PAGE_QOS, REG_QOS_8021P_EN, (uint8 *)&v16, 2, 1);
      v32 = ONE_TO_ONE_MAP;
      ethsw_wreg_ext(PAGE_QOS, REG_QOS_8021P_PRIO_MAP, (uint8 *)&v32, 4, 1);
      for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
          ethsw_wreg_ext(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (i * 4),
                     (uint8 *)&v32, 4, 1);
      }

      /* Set the default flow control value */
      v16 = DEFAULT_FC_CTRL_VAL;
      ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8 *)&v16, 2, 1);

      /*Set priority queue thresholds */
      for (i = 0; i < NUM_EGRESS_QUEUES; i++) {
         v16 = BCM6829_PRIQ_HYST;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_HYST + (2*i), (uint8 *)&v16, sizeof(v16), 1);
         v16 = BCM6829_PRIQ_PAUSE;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_PAUSE + (2*i), (uint8 *)&v16, sizeof(v16), 1);
         v16 = BCM6829_PRIQ_DROP;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_DROP + (2*i), (uint8 *)&v16, sizeof(v16), 1);
         v16 = BCM6829_PRIQ_LOWDROP;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_LO_DROP + (2*i), (uint8 *)&v16, sizeof(v16), 1);
         v16 = BCM6829_TOTAL_HYST;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_TOTAL_HYST + (2*i), (uint8 *)&v16, sizeof(v16), 1);
         v16 = BCM6829_TOTAL_PAUSE;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_TOTAL_PAUSE + (2*i), (uint8 *)&v16, sizeof(v16), 1);
         v16 = BCM6829_TOTAL_DROP;
         ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_PRIQ_TOTAL_DROP + (2*i), (uint8 *)&v16, sizeof(v16), 1);
      }

      /* setup forwarding to SERDES port */
      v16 = (1<<SERDES_PORT_ID);
      ethsw_wreg_ext(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16), 1);
      ethsw_wreg_ext(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16), 1);
      ethsw_wreg_ext(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16), 1);

      /* enable 8021Q and allow VLAN ID 0xFFF */
      v8 = 0x80;
      ethsw_wreg_ext(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, (uint8 *)&v8, sizeof(v8), 1);
      ethsw_rreg_ext(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5, (uint8 *)&v8, sizeof(v8), 1);
      v8 |= (1<<VID_FFF_ENABLE_S);
      ethsw_wreg_ext(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5, (uint8 *)&v8, sizeof(v8), 1);

      /* configure default tag for untagged frames */
      v16 = 0xFFF;
      ethsw_wreg_ext(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (2 * 0), (uint8 *)&v16, sizeof(v16), 1);
      ethsw_wreg_ext(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (2 * SERDES_PORT_ID), (uint8 *)&v16, sizeof(v16), 1);
      ethsw_wreg_ext(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (2 * MOCA_PORT_ID), (uint8 *)&v16, sizeof(v16), 1);

      /* configure rule for VID FFF - untag on egress */
      for ( i = 0; i < 1023; i++ )
      {
         v16 = i;
         ethsw_wreg_ext(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX, (uint8 *)&v16, sizeof(v16), 1);
         v32 = 0x00000031;
         ethsw_wreg_ext(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY, (uint8 *)&v32, sizeof(v32), 1);
         v8 = 0x80;
         ethsw_wreg_ext(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8 *)&v8, sizeof(v8), 1);
         udelay(100);
      }
      v16 = 0xFFF;
      ethsw_wreg_ext(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX, (uint8 *)&v16, sizeof(v16), 1);
      v32 = 0x00006231;
      ethsw_wreg_ext(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY, (uint8 *)&v32, sizeof(v32), 1);
      v8 = 0x80;
      ethsw_wreg_ext(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8 *)&v8, sizeof(v8), 1);
   }
#endif

#if defined(GPHY_EEE_1000BASE_T_DEF)
   {
      uint16 v16;
      /* Configure EEE delays. In the bootloader, we already initialized EEE
      on the GPHY before it was taken out of reset */
      v16 = 0x3F;
      ethsw_wreg(PAGE_CONTROL, REG_EEE_TW_SYS_TX_100, (uint8_t *)&v16, 2);
      v16 = 0x23;
      ethsw_wreg(PAGE_CONTROL, REG_EEE_TW_SYS_TX_1000, (uint8_t *)&v16, 2);
   }
#endif
}

/* The multiport address overrides ARL entry. So set own MAC
   in the multiport register so that packets will go to MIPS even if
   our own MAC is learned in ARL table due to a malicious packet */
int ethsw_set_multiport_address(uint8_t* addr)
{
    uint8 v8;
    uint32 v32;

    /* Enable multiport addresses */
    ethsw_rreg(PAGE_ARLCTRL, REG_ARLCFG, &v8, 1);
    v8 |= (MULTIPORT_ADDR_EN_M << MULTIPORT_ADDR_EN_S);
    ethsw_wreg(PAGE_ARLCTRL, REG_ARLCFG, &v8, 1);

    ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO,
               addr, 6);
    ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_ADDR2_LO,
               addr, 6);
    v32 = PBMAP_MIPS;
    ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1, (uint8 *)&v32, 4);
    ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR2, (uint8 *)&v32, 4);

    return 0;
}

void ethsw_set_mac_hw(uint16_t sw_port, PHY_STAT ps, int is6829)
{
#if defined(CONFIG_BCM96816)
  unsigned long flags = 0;
  uint8 val8, va8;
  uint16 val16, temp16;
  int i, timeout = 100000, num_queues;
#endif
  uint8 v8;

  if (ps.lnk == 0) {
      ethsw_rreg_ext(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1, is6829);
      v8 &= 0xFE;
  } else {
      v8 = REG_PORT_STATE_OVERRIDE;
      v8 |= (ps.lnk != 0)? REG_PORT_STATE_LNK: 0;
      v8 |= (ps.fdx != 0)? REG_PORT_STATE_FDX: 0;

      if (ps.spd1000 != 0)
          v8 |= REG_PORT_STATE_1000;
      else if (ps.spd100 != 0)
          v8 |= REG_PORT_STATE_100;
  }

  down(&bcm_ethlock_switch_config);

#if defined(CONFIG_BCM96816)
  local_irq_save(flags);
  ethsw_rreg_ext(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8), is6829);
  if (val8 & REG_SWITCH_MODE_FLSH_GPON_EGRESS_Q) {
      va8 = val8 & (~REG_SWITCH_MODE_FLSH_GPON_EGRESS_Q);
      ethsw_wreg_ext(PAGE_CONTROL, REG_SWITCH_MODE, &va8, sizeof(va8), is6829);
  }
  BCM_ENET_DEBUG("Override written val = 0x%2x", v8);
  ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1, is6829);

  if (ps.lnk == 0) {
      val16 = sw_port;
      if (is6829)
      {
          /* On the 6829, the software switching code relies on the DIAG_PORT_SEL
             register being set to 0x5 to avoid multiple reads over SPI. So, restore
             the state of the register when we're done with it. */
          ethsw_rreg_ext(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (uint8 *)&temp16, 2, is6829);
      }

      ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (uint8 *)&val16, 2, is6829);

      ethsw_rreg_ext(PAGE_QOS, REG_QOS_TXQ_CTRL, &va8, 1, is6829);
      if ((va8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M) {
          num_queues = NUM_EGRESS_QUEUES;
      } else {
          num_queues = 1;
      }

      for (i = 0; i < num_queues; i++) {
          ethsw_rreg_ext(PAGE_FLOW_CTRL, REG_FC_Q_MON_CNT + (i * 2),
                     (uint8 *)&val16, 2, is6829);
          while (val16) {
             if ((timeout--) <= 0) {
                 if (sw_port == MOCA_PORT_ID) {
                     printk("\nLink Down TIMEOUT: Port %d queues not"
                            " drained \n", sw_port);
                 }
                 else {
                     BCM_ENET_DEBUG("\nLink Down TIMEOUT: Port %d queues not"
                                    " drained \n", sw_port);
                 }
                 break;
             }
             ethsw_rreg_ext(PAGE_FLOW_CTRL, REG_FC_Q_MON_CNT + (i * 2),
                        (uint8 *)&val16, 2, is6829);
          }
      }

      if (is6829)
      {
          ethsw_wreg_ext(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (uint8 *)&temp16, 2, is6829);
      }
  }
  ethsw_wreg_ext(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8), is6829);
  local_irq_restore(flags);
#else
  ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1);
#endif

  up(&bcm_ethlock_switch_config);
}

#ifdef REPORT_HARDWARE_STATS
#if defined(AEI_VDSL_STATS_DIAG)
int ethsw_get_hw_stats(int port, int extswitch, struct net_device_stats *stats, struct net_device * dev) {
    uint32 ctr32 = 0;
    uint64 ctr64 = 0;
#if !defined(CONFIG_BCM96816)
    uint8 data[8] = {0};
#endif
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);

#if !defined(CONFIG_BCM96816)
    if (extswitch) {
#if (CONFIG_BCM_EXT_SWITCH == 5398)
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXMPKTS, data, 4);
        ctr64 = swab32(*(uint32 *)data);
        stats->multicast = ctr64;
        pDevCtrl->dev_stats.rx_multicast_packets = ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXUPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXBPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        stats->rx_packets = (unsigned long)ctr64;
        pDevCtrl->dev_stats.rx_packets = (unsigned long)ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS, data, 8);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->rx_bytes = *(unsigned long *)data;
        pDevCtrl->dev_stats.rx_bytes = *(unsigned long long *)data;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXDROPS, data, 4);
        stats->rx_dropped = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXFCSERRORS, data, 4);
        stats->rx_errors = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXSYMBOLERRORS, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXALIGNERRORS, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
#else
        extsw_rreg(PAGE_MIB_P0 + (port), 0x94, data, 4);
        ctr64 = swab32(*(uint32 *)data);
        BCM_ENET_DEBUG("read data = %02x %02x %02x %02x \n",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("ctr64 = %x \n", (unsigned int)ctr64);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x98, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x9C, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        stats->rx_packets = (unsigned long)ctr64;
        pDevCtrl->dev_stats.rx_packets = (unsigned long)ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), 0x50, data, 8);
        BCM_ENET_DEBUG("data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->rx_bytes = ((uint32 *)data)[1];
        pDevCtrl->dev_stats.rx_bytes = *(unsigned long long *)data;
        BCM_ENET_DEBUG("rx = %lx %llu\n", stats->rx_bytes,pDevCtrl->dev_stats.rx_bytes);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x90, data, 4);
        stats->rx_dropped = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x84, data, 4);
        stats->rx_errors = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0xAC, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x80, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x8C, data, 4);
        stats->multicast = swab32(*(uint32 *)data);
        pDevCtrl->dev_stats.rx_multicast_packets = stats->multicast; 
#endif
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXMPKTS, data, 4);
        ctr64 = swab32(*(uint32 *)data);
        pDevCtrl->dev_stats.tx_multicast_packets = ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXUPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXBPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        stats->tx_packets = (unsigned long)ctr64;
        pDevCtrl->dev_stats.tx_packets = (unsigned long)ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS, data, 8);
        BCM_ENET_DEBUG("data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->tx_bytes = ((uint32 *)data)[1];
        pDevCtrl->dev_stats.tx_bytes = *(unsigned long long *)data;
        BCM_ENET_DEBUG("tx = %lx %llu\n", stats->tx_bytes,pDevCtrl->dev_stats.tx_bytes);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXDROPS, data, 4);
        stats->tx_dropped = swab32(*(uint32 *)data);
    } else
#endif
    {
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXMPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 = (uint64)ctr32;
        stats->multicast = (unsigned long)ctr64;
        pDevCtrl->dev_stats.rx_multicast_packets = (unsigned long)ctr64;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXUPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXBPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        stats->rx_packets = (unsigned long)ctr64;
        pDevCtrl->dev_stats.rx_packets = (unsigned long)ctr64;
   /*   ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS,
               (uint8_t *)&ctr64, 8, extswitch);
        stats->rx_bytes = (unsigned long)ctr64;
        pDevCtrl->dev_stats.rx_bytes = (unsigned long long)ctr64;*/
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS,
               (uint8_t *)data, 8, extswitch);
        BCM_ENET_DEBUG("data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->rx_bytes = ((uint32 *)data)[1];
        pDevCtrl->dev_stats.rx_bytes = *(unsigned long long *)data;
        BCM_ENET_DEBUG("rx = %lx %llu\n",stats->rx_bytes,pDevCtrl->dev_stats.rx_bytes);
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXDROPS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_dropped = (unsigned long)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXFCSERRORS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_errors = (unsigned long)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXSYMBOLERRORS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_errors += (unsigned long)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXALIGNERRORS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_errors += (unsigned long)ctr32;

        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXMPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 = (uint64)ctr32;
        pDevCtrl->dev_stats.tx_multicast_packets = (unsigned long)ctr64;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXUPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXBPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        stats->tx_packets = (unsigned long)ctr64;
        pDevCtrl->dev_stats.tx_packets = (unsigned long)ctr64;
   /*   ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS,
               (uint8_t *)&ctr64, 8, extswitch);
        stats->tx_bytes = (unsigned long)ctr64;
        pDevCtrl->dev_stats.tx_bytes = (unsigned long long)ctr64;*/
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS,
               (uint8_t *)data, 8, extswitch);
        BCM_ENET_DEBUG("data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);      
        stats->tx_bytes = ((uint32 *)data)[1];
        pDevCtrl->dev_stats.tx_bytes = *(unsigned long long *)data;
        BCM_ENET_DEBUG("tx = %lx %llu\n",stats->tx_bytes,pDevCtrl->dev_stats.tx_bytes);
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXDROPS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->tx_dropped = (unsigned long)ctr32;
    }
    return 0;
}
#else
int ethsw_get_hw_stats(int port, int extswitch, struct net_device_stats *stats) {
    uint32 ctr32 = 0;
    uint64 ctr64 = 0;
#if !defined(CONFIG_BCM96816)
    uint8 data[8] = {0};
#endif


#if !defined(CONFIG_BCM96816)
    if (extswitch) {
#if (CONFIG_BCM_EXT_SWITCH == 5398)
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXMPKTS, data, 4);
        ctr64 = swab32(*(uint32 *)data);
        stats->multicast = ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXUPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXBPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        stats->rx_packets = (unsigned long)ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS, data, 8);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->rx_bytes = *(unsigned long *)data;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXDROPS, data, 4);
        stats->rx_dropped = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXFCSERRORS, data, 4);
        stats->rx_errors = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXSYMBOLERRORS, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXALIGNERRORS, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
#else
        extsw_rreg(PAGE_MIB_P0 + (port), 0x94, data, 4);
        ctr64 = swab32(*(uint32 *)data);
        BCM_ENET_DEBUG("read data = %02x %02x %02x %02x \n",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("ctr64 = %x \n", (unsigned int)ctr64);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x98, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x9C, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        stats->rx_packets = (unsigned long)ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), 0x50, data, 8);
        BCM_ENET_DEBUG("read data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->rx_bytes = ((uint32 *)data)[1];
        BCM_ENET_DEBUG("ctr64 = %lx \n", stats->rx_bytes);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x90, data, 4);
        stats->rx_dropped = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x84, data, 4);
        stats->rx_errors = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0xAC, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x80, data, 4);
        stats->rx_errors += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), 0x8C, data, 4);
        stats->multicast = swab32(*(uint32 *)data);
#endif
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXMPKTS, data, 4);
        ctr64 = swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXUPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXBPKTS, data, 4);
        ctr64 += swab32(*(uint32 *)data);
        stats->tx_packets = (unsigned long)ctr64;
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS, data, 8);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->tx_bytes = ((uint32 *)data)[1];
        extsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXDROPS, data, 4);
        stats->tx_dropped = swab32(*(uint32 *)data);
    } else
#endif
    {
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXMPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 = (uint64)ctr32;
        stats->multicast = (unsigned long)ctr64;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXUPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXBPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        stats->rx_packets = (unsigned long)ctr64;
  /*      ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS,
               (uint8_t *)&ctr64, 8, extswitch);
        stats->rx_bytes = (unsigned long)ctr64;*/
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS,
               (uint8_t *)data, 8, extswitch);
        BCM_ENET_DEBUG("data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
        stats->rx_bytes = ((uint32 *)data)[1];
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXDROPS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_dropped = (unsigned long)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXFCSERRORS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_errors = (unsigned long)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXSYMBOLERRORS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_errors += (unsigned long)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_RXALIGNERRORS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->rx_errors += (unsigned long)ctr32;

        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXMPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 = (uint64)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXUPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXBPKTS,
               (uint8_t *)&ctr32, 4, extswitch);
        ctr64 += (uint64)ctr32;
        stats->tx_packets = (unsigned long)ctr64;
/*        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS,
               (uint8_t *)&ctr64, 8, extswitch);
        stats->tx_bytes = (unsigned long)ctr64;*/
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS,
               (uint8_t *)data, 8, extswitch);
        BCM_ENET_DEBUG("data = %02x %02x %02x %02x ",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("%02x %02x %02x %02x \n",
            data[4], data[5], data[6], data[7]);
        ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
        ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);      
        stats->tx_bytes = ((uint32 *)data)[1];
        ethsw_rreg_ext(PAGE_MIB_P0 + (port), REG_MIB_P0_TXDROPS,
               (uint8_t *)&ctr32, 4, extswitch);
        stats->tx_dropped = (unsigned long)ctr32;
    }
    return 0;
}
#endif /* AEI_VDSL_STATS_DIAG */ 
#endif /* REPORT_HARDWARE_STATS */
#if defined(AEI_VDSL_CUSTOMER_NCS)
void AEI_convert_mac_vid_v64_format(uint8_t *mac_vid_v64)
{
	uint8_t mac_tmp[8]={0};	
        memcpy(mac_tmp,mac_vid_v64,8);
        ((uint32 *)mac_tmp)[0] = swab32(((uint32 *)mac_tmp)[0]);
        ((uint32 *)mac_tmp)[1] = swab32(((uint32 *)mac_tmp)[1]);
	//memset(mac_vid_v64,0,sizeof(mac_vid_v64));
        memcpy(mac_vid_v64,&mac_tmp[2],6);
        memcpy(&mac_vid_v64[6],&mac_tmp[0],2);
        BCM_ENET_DEBUG("covert result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
                       mac_vid_v64[0],mac_vid_v64[1],mac_vid_v64[2],mac_vid_v64[3],mac_vid_v64[4],mac_vid_v64[5],mac_vid_v64[6],mac_vid_v64[7]);
}   
#endif

/*Pick up it from SDK1207 */
int remove_arl_entry_ext(uint8_t *mac)
{
    int timeout, count = 0;
    uint32_t v32=0, cur_data_v32 = 0;
    uint16_t cur_vid_v16;
    uint8_t v8, mac_vid_v64[8];
    unsigned int extSwId;

    extSwId = bcm63xx_enet_extSwId();
    if (extSwId != 0x53115 && extSwId != 0x53125)
    {
        /* Currently only these two switches are supported */
        printk("Error - Ext-switch = 0x%x not supported\n", extSwId);
        return BCM_E_NONE;
    }

    BCM_ENET_DEBUG("mac: %02x %02x %02x %02x %02x %02x\n", mac[0],
                    mac[1], mac[2], mac[3], mac[4], (uint8_t)mac[5]);
    /* Setup ARL Search */
    v8 = ARL_SRCH_CTRL_START_DONE;
    extsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
    extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
    BCM_ENET_DEBUG("ARL_SRCH_CTRL (0x%02x%02x) = 0x%x \n",PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, v8);
    while (v8 & ARL_SRCH_CTRL_START_DONE) {
        extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
        timeout = 1000;
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        while((v8 & ARL_SRCH_CTRL_SR_VALID) == 0) {
            extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx,
                       (uint8_t *)&v8, 1);
            if (v8 & ARL_SRCH_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0) {
                    printk("ARL Search Timeout for Valid to be 1 \n");
                    return BCM_E_NONE;
                }
            } else {
                BCM_ENET_DEBUG("ARL Search Done count %d\n", count);
                return BCM_E_NONE;
            }
        }
        /* Found a valid entry */
        extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY_531xx,&mac_vid_v64[0], 8);
        BCM_ENET_DEBUG("ARL_SRCH_result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
                       mac_vid_v64[0],mac_vid_v64[1],mac_vid_v64[2],mac_vid_v64[3],mac_vid_v64[4],mac_vid_v64[5],mac_vid_v64[6],mac_vid_v64[7]);
#if defined(AEI_VDSL_CUSTOMER_NCS)
	AEI_convert_mac_vid_v64_format(mac_vid_v64);
#endif
        /* ARL Search results are read; Mark it done(by reading the reg)
           so ARL will start searching the next entry */
        extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_RESULT_DONE_531xx,
                   (uint8_t *)&v32, 4);
        /* Check if found the matching ARL entry */
#if defined(AEI_VDSL_CUSTOMER_NCS)
        if ( mac_vid_v64[5] == mac[5] &&
             mac_vid_v64[4] == mac[4] &&
             mac_vid_v64[3] == mac[3] &&
             mac_vid_v64[2] == mac[2] &&
             mac_vid_v64[1] == mac[1] &&
             mac_vid_v64[0] == mac[0])
#else
        if ( mac_vid_v64[0] == mac[5] && 
             mac_vid_v64[1] == mac[4] &&
             mac_vid_v64[2] == mac[3] &&
             mac_vid_v64[3] == mac[2] &&
             mac_vid_v64[4] == mac[1] &&
             mac_vid_v64[5] == mac[0])
#endif
        { /* found the matching entry ; invalidate it */
            BCM_ENET_DEBUG("Found matching ARL Entry\n");
            /* Write the MAC Address */
            extsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&mac_vid_v64[0], 6);
            /* Get the VID for this entry and write it */
            cur_vid_v16 = mac_vid_v64[6] | ((mac_vid_v64[7]<<8) &0xF);;
            extsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&cur_vid_v16, 2);
            /* Initiate a read transaction */
            v8 = ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ;
            extsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            timeout = 10;
            while(v8 & ARL_TBL_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0)  {
                    printk("Error - can't read/find the ARL entry\n");
                    return 0;
                }
                extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            }
            /* Read transaction complete - get the MAC + VID */
            extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY,&mac_vid_v64[0], 8);
            BCM_ENET_DEBUG("ARL_SRCH_result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
                           mac_vid_v64[0],mac_vid_v64[1],mac_vid_v64[2],mac_vid_v64[3],mac_vid_v64[4],mac_vid_v64[5],mac_vid_v64[6],mac_vid_v64[7]);
            /* Compare the MAC */
#if defined(AEI_VDSL_CUSTOMER_NCS)
            AEI_convert_mac_vid_v64_format(mac_vid_v64);
            if ( !(mac_vid_v64[5] == mac[5] &&
                 mac_vid_v64[4] == mac[4] &&
                 mac_vid_v64[3] == mac[3] &&
                 mac_vid_v64[2] == mac[2] &&
                 mac_vid_v64[1] == mac[1] &&
                 mac_vid_v64[0] == mac[0]))
#else
            if (!(mac_vid_v64[0] == mac[5] && 
                  mac_vid_v64[1] == mac[4] &&
                  mac_vid_v64[2] == mac[3] &&
                  mac_vid_v64[3] == mac[2] &&
                  mac_vid_v64[4] == mac[1] &&
                  mac_vid_v64[5] == mac[0]))
#endif
            {
                printk("Error - can't find the requested ARL entry\n");
                return 0;
            }
            /* Read the associated data entry */
            extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY,(uint8_t *)&cur_data_v32, 4);
            BCM_ENET_DEBUG("ARL_SRCH_DATA = 0x%08x \n", cur_data_v32);
            cur_data_v32 = swab32(cur_data_v32);
            /* Invalidate the entry -> clear valid bit */
            cur_data_v32 &= 0xFFFF; 
            cur_data_v32 = swab32(cur_data_v32);
            /* Modify the data entry for this ARL */
            extsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY,(uint8_t *)&cur_data_v32, 4);
            /* Initiate a write transaction */
            v8 = ARL_TBL_CTRL_START_DONE;
            extsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            timeout = 10;
            extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            while(v8 & ARL_TBL_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0)  {
                    printk("Error - can't write/find the ARL entry\n");
                    break;
                }
                extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            }
            return 0;
        }
        if ((count++) > NUM_ARL_ENTRIES) {
            break;
        }
        /* Now read the ARL Search Ctrl reg. again for next entry */
        extsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
        BCM_ENET_DEBUG("ARL_SRCH_CTRL (0x%02x%02x) = 0x%x \n",PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, v8);
    }

    return 0; /* Actually this is error but no-one should care the return value */
}
/*
*------------------------------------------------------------------------------
* Function   : remove_arl_entry
* Description: Removes/invalidates the matching MAC ARL entry in the switch.
* Input      : Pointer to MAC address string
* Design Note: Invoked by linux bridge during MAC station move.
* Pick up it from SDK1207
*------------------------------------------------------------------------------
*/
int remove_arl_entry(uint8_t *mac)
{
    uint16_t v16, mac_hi;
    int timeout, first = 1;
    uint32_t first_mac_lo = 0, first_vid_mac_hi = 0;
    uint32_t cur_lo = 0, cur_hi = 0, mac_lo, v32=0;
    uint16_t cur_data = 0;
    uint8_t v8;

    BCM_ENET_DEBUG("mac: %02x %02x %02x %02x %02x %02x\n", mac[0],
                    mac[1], mac[2], mac[3], mac[4], (uint8_t)mac[5]);
    /* Convert MAC string to hi and lo words */
    mac_hi = ((mac[0] & 0xFF) << 8) | (mac[1] & 0xFF) ;
    mac_lo = ((mac[2] & 0xFF) << 24) | 
             ((mac[3] & 0xFF) << 16) | 
             ((mac[4] & 0xFF) << 8) | 
              (mac[5] & 0xFF);

    BCM_ENET_DEBUG("mac_hi (16-bit) = 0x%04x \n", mac_hi);
    BCM_ENET_DEBUG("mac_lo (32-bit) = 0x%08x \n", (unsigned int)mac_lo);

    /* Setup ARL Search */
    v16 = ARL_SRCH_CTRL_START_DONE;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
    /* Read the complete ARL table */
    while (v16 & ARL_SRCH_CTRL_START_DONE) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        timeout = 1000;
        while((v16 & ARL_SRCH_CTRL_SR_VALID) == 0) {
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
            if (v16 & ARL_SRCH_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0) {
                    return 0;
                }
            } else {
                return 0;
            }
        }
        /* Grab the lo and hi MAC entry */
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY,
                   (uint8_t *)&cur_lo, 4);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_VID_MAC_HI_ENTRY,
                   (uint8_t *)&cur_hi, 4);
        /* Store the first MAC read */
        if (first) {
            first_mac_lo = cur_lo;
            first_vid_mac_hi = cur_hi;
            first = 0;
        } else if ((first_mac_lo == cur_lo) &&
                   (first_vid_mac_hi == cur_hi)) {
            /* Bail out if all the entries read */
            break;
        }
        /* Grab the data part of ARL entry */
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY,
                   (uint8_t *)&cur_data, 2);

        BCM_ENET_DEBUG("cur_lo = 0x%x cur_hi = 0x%x cur_data = 0x%x\n", cur_lo,cur_hi,cur_data);
        /* Check if found the matching ARL entry */
        if ((mac_hi == (cur_hi & 0xFFFF)) && (mac_lo == cur_lo))
        { /* found the matching entry ; invalidate it */
            v16 = cur_lo & 0xFFFF; /* Re-using v16 - no problem we go out of this function */
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
            v32 = (cur_lo >> 16) | (cur_hi << 16);
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_HI, (uint8_t *)&v32, 4);
            v16 = (cur_hi >> 16) & 0xFFFF;
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&v16, 2);
            v8 = ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ;
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            timeout = 10;
            while(v8 & ARL_TBL_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0)  {
                    break;
                }
                ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            }
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY,
                       (uint8_t *)&cur_lo, 4);
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VID_MAC_HI_ENTRY,
                       (uint8_t *)&cur_hi, 4);
            /* Invalidate the entry -> clear valid bit
             * Actually valid bit is always NOT set on read - so this is redundant */
            cur_data &= 0x7FFF; 
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY,
                       (uint8_t *)&cur_data, 2);
            v8 = ARL_TBL_CTRL_START_DONE;
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            timeout = 10;
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            while(v8 & ARL_TBL_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0)  {
                    break;
                }
                ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            }
            return 0;
        }
    }
    
	return 0; /* Actually this is error but no-one should care the return value */
}



int remove_arl_entry_wrapper(void *ptr)
{
    int ret = 0;
    ret = remove_arl_entry(ptr); /* remove entry from internal switch */
    if (bcm63xx_enet_isExtSwPresent()) {
        ret = remove_arl_entry_ext(ptr); /* remove entry from internal switch */
    }
    return ret;
}


#if defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE)
int enet_hook_for_arl_access(void *ethswctl)
{
    return enet_ioctl_ethsw_arl_access((struct ethswctl_data *)ethswctl);
}
#endif
