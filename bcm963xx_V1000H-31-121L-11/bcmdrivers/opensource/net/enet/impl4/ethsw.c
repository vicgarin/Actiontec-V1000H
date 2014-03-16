/*
 Copyright 2007-2010 Broadcom Corp. All Rights Reserved.

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

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map.h>
#include "bcm_intr.h"
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethswdefs.h"
#include "ethsw.h"
#include "ethsw_phy.h"
#include "bcmsw.h"
#include "bcmSpiRes.h"
#include "bcmswaccess.h"
#include "bcmswshared.h"

#if defined(AEI_VDSL_CUSTOMER_NCS)
void extsw_rreg(int page, int reg, uint8 *data, int len);
void extsw_wreg(int page, int reg, uint8 *data, int len);
#endif

static void str_to_num(char *in, char *out, int len);
static int proc_get_sw_param(char *page, char **start, off_t off, int cnt, int *eof, void *data);
static int proc_set_sw_param(struct file *f, const char *buf, unsigned long cnt, void *data);
static void ethsw_init_table(ETHERNET_SW_INFO *sw);

static int proc_get_mii_param(char *page, char **start, off_t off, int cnt, int *eof, void *data);
static int proc_set_mii_param(struct file *f, const char *buf, unsigned long cnt, void *data);

extern struct net_device *vnet_dev[MAX_NUM_OF_VPORTS];
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
extern struct net_device *gponifid_to_dev[MAX_GEM_IDS];
#endif
#if defined(CONFIG_BCM96816)
extern struct net_device* bcm6829_to_dev[MAX_6829_IFS];
#endif
extern struct semaphore bcm_ethlock_switch_config;
extern uint8_t port_in_loopback_mode[TOTAL_SWITCH_PORTS];

// Software port index ---> real hardware param.
int switch_port_index[TOTAL_SWITCH_PORTS];
int switch_port_phyid[TOTAL_SWITCH_PORTS];
int switch_pport_phyid[TOTAL_SWITCH_PORTS] = {-1, -1, -1, -1, -1, -1, -1, -1};
int ext_switch_pport_phyid[TOTAL_SWITCH_PORTS] = {-1, -1, -1, -1, -1, -1, -1, -1};

spinlock_t spl_lock;
BcmEnet_devctrl *pVnetDev0_g;

#if defined(AEI_VDSL_HPNA)
extern  int hpna_support;  // 0 : un-support, 1 : support
#endif

static void ethsw_init_table(ETHERNET_SW_INFO *sw)
{
    int map, cnt;
    int i, j;

    spin_lock_init(&spl_lock);

    pVnetDev0_g = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);

    map = sw->port_map;
    bitcount(cnt, map);

    if ((cnt <= 0) || (cnt > BP_MAX_SWITCH_PORTS))
        return;

    for (i = 0, j = 0; i < cnt; i++, j++, map /= 2)
    {
        while ((map % 2) == 0)
        {
            map /= 2;
            j++;
        }
        switch_port_index[i] = j;
        switch_port_phyid[i] = sw->phy_id[j];
        switch_pport_phyid[j] = sw->phy_id[j];
//        ETHSW_PHY_SET_PHYID(j/*port*/, sw->phy_id[j] /*phy id*/);
    }

    return;
}

int ethsw_port_to_phyid(int port)
{
    return switch_port_phyid[port];
}

#if defined(CONFIG_BCM_ETH_PWRSAVE)
void ethsw_isolate_phy(int phyId, int isolate)
{
    uint16 v16;
    ethsw_phy_rreg(phyId, MII_CONTROL, &v16);
    if (isolate) {
        v16 |= MII_CONTROL_ISOLATE_MII;
    } else {
        v16 &= ~MII_CONTROL_ISOLATE_MII;
    }
    ethsw_phy_wreg(phyId, MII_CONTROL, &v16);
}
#endif

int ethsw_set_mac(int port, PHY_STAT ps)
{
    uint16 sw_port;
    int is6829 = 0;

#if defined(CONFIG_BCM96816)
    if ( IsExt6829(port) )
    {
       /* for the 6829 the port passed in is the
          physical port not the virtual port */
       sw_port = (uint16)(port & ~BCM_EXT_6829);
       is6829  = 1;
    }
    else
    {
        sw_port = (uint16)switch_port_index[port];
    }
#else
    sw_port = (uint16)switch_port_index[port];
#endif

#if defined(CONFIG_BCM96816)
    if (!is6829)
#endif
    if (port_in_loopback_mode[sw_port]) {
        return 0;
    }

    ethsw_set_mac_hw(sw_port, ps, is6829);

    return 0;
}

/* port = physical port */
int ethsw_phy_intr_ctrl(int port, int on)
{
    uint16 v16;

    down(&bcm_ethlock_switch_config);

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96368) || defined(CONFIG_BCM963268)
    if (on != 0)
        v16 = MII_INTR_ENABLE | MII_INTR_FDX | MII_INTR_SPD | MII_INTR_LNK;
    else
        v16 = 0;

    ethsw_phy_wreg(switch_pport_phyid[port], MII_INTERRUPT, &v16);
#endif

#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM963268)
#if defined(CONFIG_BCM963268)
    if (port == GPHY_PORT_ID)
#endif
    {
        if (on != 0)
            v16 = ~(MII_INTR_FDX | MII_INTR_SPD | MII_INTR_LNK);
        else
            v16 = 0xFFFF;

        ethsw_phy_wreg(switch_pport_phyid[port], MII_INTERRUPT_MASK, &v16);
    }
#endif

    up(&bcm_ethlock_switch_config);

    return 0;
}

/* Called from ISR context. No sleeping locks */
void ethsw_set_mac_link_down(void)
{
    int i = 0;
    uint16 v16 = 0;
    uint8 v8 = 0;

    for (i = 0; i < EPHY_PORTS; i++) {
        if(!IsExtPhyId(switch_pport_phyid[i])) {
            ethsw_phy_rreg(switch_pport_phyid[i], MII_INTERRUPT, &v16);
            if ((pVnetDev0_g->EnetInfo[0].sw.port_map & (1U<<i)) != 0) {
                if (v16 & MII_INTR_LNK) {
                    ethsw_phy_rreg(switch_pport_phyid[i], MII_BMSR, &v16);
                    if (!(v16 & BMSR_LSTATUS)) {
                        ethsw_rreg(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1);
                        if (v8 & REG_PORT_STATE_LNK) {
                            v8 &= (~REG_PORT_STATE_LNK);
                            ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1);
                        }
                    }
                }
            }
        }
    }
}

PHY_STAT ethsw_phy_stat(int port)
{
    PHY_STAT ps;
    uint16 v16;
    uint16 ctrl;
    int phyId;
#if defined(CONFIG_BCM96816)
    int is6829 = 0;
    uint16 sw_port;
#endif

    ps.lnk = 0;
    ps.fdx = 0;
    ps.spd1000 = 0;
    ps.spd100 = 0;

#if defined(CONFIG_BCM96816)
    if ( IsExt6829(port) )
    {
       /* for the 6829 the port passed in is the physical port not a virtual port
          no guarantee that physical maps to virtual so get the phy id from board info */
       ETHERNET_SW_INFO *sw = &(((BcmEnet_devctrl *)netdev_priv(vnet_dev[0]))->EnetInfo[0].sw);

       sw_port = (uint16)(port & ~BCM_EXT_6829);
       phyId   = sw->phy_id[sw_port];
       if ( IsPhyConnected(phyId) )
       {
          phyId &= ~BCM_WAN_PORT;
          phyId  |= BCM_EXT_6829;
       }
       is6829  = 1;
    }
    else
    {
       sw_port = (uint16)switch_port_index[port];
       phyId   = switch_port_phyid[port];
    }
#else
    phyId  = switch_port_phyid[port];
#endif
    if ( !IsPhyConnected(phyId) )
    {
        // 0xff PHY ID means no PHY on this port.
        ps.lnk = 1;
        ps.fdx = 1;
        ps.spd1000 = 1;
        return ps;
    }

#if defined(CONFIG_BCM96816)
    if (!is6829)
#endif
    if (port_in_loopback_mode[switch_port_index[port]]) {
        return ps;
    }

    down(&bcm_ethlock_switch_config);

    ethsw_phy_rreg(phyId, MII_INTERRUPT, &v16);
    ethsw_phy_rreg(phyId, MII_ASR, &v16);

    if (!MII_ASR_LINK(v16)) {
        up(&bcm_ethlock_switch_config);
        return ps;
    }

    ps.lnk = 1;

    if (!MII_ASR_DONE(v16)) {
        ethsw_phy_rreg(phyId, MII_BMCR, &ctrl);
        if (ctrl & BMCR_ANENABLE) {
            up(&bcm_ethlock_switch_config);
            return ps;
        }
        // auto-negotiation disabled
        ps.fdx = (ctrl & BMCR_FULLDPLX) ? 1 : 0;
        if((ctrl & BMCR_SPEED100) && !(ctrl & BMCR_SPEED1000))
            ps.spd100 = 1;
        else if(!(ctrl & BMCR_SPEED100) && (ctrl & BMCR_SPEED1000))
            ps.spd1000 = 1;

        up(&bcm_ethlock_switch_config);
        return ps;
    }

#ifdef CONFIG_BCM96368
    if ((!IsExtPhyId(phyId)) && MII_ASR_NOHCD(v16)) {
        ethsw_phy_rreg(phyId, MII_AENGSR, &ctrl);
        if (ctrl & MII_AENGSR_SPD) {
            ps.spd100 = 1;
        }
        if (ctrl & MII_AENGSR_DPLX) {
            ps.fdx = 1;
        }
        return ps;
    }
#endif
    up(&bcm_ethlock_switch_config);

    if (MII_ASR_FDX(v16))
        ps.fdx = 1;

    if (MII_ASR_1000(v16))
        ps.spd1000 = 1;
    else if (MII_ASR_100(v16))
        ps.spd100 = 1;

    return ps;
}

#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM963268)
void ethsw_phy_advertise_all(uint32 phy_id)
{
   uint16 v16;
   /* Advertise all speed & duplex combinations */
   /* Advertise 100BaseTX FD/HD and 10BaseT FD/HD */
   ethsw_phy_rreg(phy_id, MII_ADVERTISE, &v16);
   v16 |= AN_ADV_ALL;
   ethsw_phy_wreg(phy_id, MII_ADVERTISE, &v16);
   /* Advertise 1000BaseT FD/HD */
   ethsw_phy_rreg(phy_id, MII_CTRL1000, &v16);
   v16 |= AN_1000BASET_CTRL_ADV_ALL;
   ethsw_phy_wreg(phy_id, MII_CTRL1000, &v16);
}
#endif

#ifdef CONFIG_BCM96816
/* Setup the PHY with initial PHY config */
/* The internal Phys do not seem to advertise 10HD, 10FD, and 100HD.
   Fixing this for now. */
/* TBD: Maintain the PHY config state for each PHY */
int ethsw_setup_phys(void)
{
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int portmap, i, phy_id;
    uint16 v16;
    int is6829Present = 0;

    /* Get the portmap */
    portmap = pVnetDev0->EnetInfo[0].sw.port_map;

    /* For each port that has an internal or external PHY, configure it
       as per the required initial configuration */
    for (i = 0; i < (TOTAL_SWITCH_PORTS - 1); i++) {
        /* Check if the port is in the portmap or not */
        if ((portmap & (1U<<i)) != 0) {
            /* Check if the port is connected to a PHY or not */
            phy_id = pVnetDev0->EnetInfo[0].sw.phy_id[i];
            /* If a Phy is connected, set it up with initial config */
            /* TBD: Maintain the config for each Phy */
            if(IsPhyConnected(phy_id))
            {
                if ( !IsExt6829(phy_id) )
                {
                    ethsw_phy_advertise_all(phy_id);
                }
                else
                {
                    /* Advertise all speed & duplex combinations */
                    /* Advertise 100BaseTX FD/HD and 10BaseT FD/HD */
                    int phyId6829;
                    int j;

                    phy_id &= ~(BCM_EXT_6829 | BCM_WAN_PORT);
                    for (j = 0; j < (TOTAL_SWITCH_PORTS - 1); j++)
                    {
                        if ((phy_id & (1U<<j)) != 0)
                        {
                            is6829Present = 1;
                            /* ids for 6829 match 6819 ids*/
                            phyId6829 = (pVnetDev0->EnetInfo[0].sw.phy_id[j] & ~BCM_WAN_PORT) | BCM_EXT_6829;
                            ethsw_phy_advertise_all(phyId6829);
                        }
                    }
                }
            }
        }
    }

    /* Initialize the RC calibration of the internal Phys to be able to
       pass the Ethernet Mask Measurements. The calibration for both the
       internal Phys is shared and configurable through the first Phy.
       The values written are as recommended by the Board Team */
    phy_id = pVnetDev0->EnetInfo[0].sw.phy_id[0];
    v16 = 0x0F96;
    ethsw_phy_wreg(phy_id, MII_DSP_COEFF_ADDR, &v16);
    v16 = 0x000C;
    ethsw_phy_wreg(phy_id, MII_DSP_COEFF_RW_PORT, &v16);
    v16 = 0x0000;
    ethsw_phy_wreg(phy_id, MII_DSP_COEFF_RW_PORT, &v16);

    if ( 1 == is6829Present )
    {
       phy_id |= BCM_EXT_6829;
       v16 = 0x0F96;
       ethsw_phy_wreg(phy_id, MII_DSP_COEFF_ADDR, &v16);
       v16 = 0x000C;
       ethsw_phy_wreg(phy_id, MII_DSP_COEFF_RW_PORT, &v16);
       v16 = 0x0000;
       ethsw_phy_wreg(phy_id, MII_DSP_COEFF_RW_PORT, &v16);
    }
    return 0;
}
#endif

#if defined(CONFIG_BCM96816)
#define NUM_INT_EPHYS 0
#define NUM_INT_GPHYS 2
#elif defined(CONFIG_BCM963268)
#define NUM_INT_EPHYS 3
#define NUM_INT_GPHYS 1
#else
#define NUM_INT_EPHYS 4
#define NUM_INT_GPHYS 0
#endif
#define NUM_INT_PHYS (NUM_INT_EPHYS + NUM_INT_GPHYS)

#if defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
void ethsw_setup_hw_apd(unsigned int enable)
{
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int phy_id, i;
    uint16 v16;

    down(&bcm_ethlock_switch_config);

    /* For each configured external PHY, enable/disable APD */
    for (i = 0; i < (TOTAL_SWITCH_PORTS - 1); i++) {
        /* Check if the port is in the portmap or not */
        if ((pVnetDev0->EnetInfo[0].sw.port_map & (1U<<i)) != 0) {
            /* Check if the port is connected to a PHY or not */
            phy_id = pVnetDev0->EnetInfo[0].sw.phy_id[i];
            /* If a Phy is connected, and is external, set APD */
            if(IsPhyConnected(phy_id) && IsExtPhyId(phy_id)) {
                /* Assume that the PHY is compatible to BCM54610... write 0xa821 */
                v16 = MII_1C_WRITE_ENABLE | MII_1C_AUTO_POWER_DOWN_SV | MII_1C_WAKEUP_TIMER_SEL_84;
                if (enable) {
                    v16 |= MII_1C_AUTO_POWER_DOWN;
                }
                ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
            }
        }
    }

    /* For each configured internal PHY, enable/disable APD */
#if NUM_INT_EPHYS > 0
    /* EPHYs */
    for (i = 1; i <= NUM_INT_EPHYS; i++) {
        v16 = 0x008B;
        ethsw_phy_wreg(i, MII_BRCM_TEST, &v16);
        v16 = 0x7021;
        ethsw_phy_wreg(i, 0x1b, &v16);
        v16 = 0x000B;
        ethsw_phy_wreg(i, MII_BRCM_TEST, &v16);
    }
#endif

#if NUM_INT_GPHYS > 0
    /* GPHYs */
    for (i = 1+NUM_INT_EPHYS; i <= NUM_INT_GPHYS+NUM_INT_EPHYS; i++) {
        v16 = MII_1C_WRITE_ENABLE | MII_1C_AUTO_POWER_DOWN_SV | MII_1C_WAKEUP_TIMER_SEL_84;
        if (enable) {
            v16 |= MII_1C_AUTO_POWER_DOWN;
        }
        ethsw_phy_wreg(i, MII_REGISTER_1C, &v16);
    }
#endif

    up(&bcm_ethlock_switch_config);
}
#endif

#if defined(AEI_VDSL_POWERSAVE)
static void AEI_ethsw_set_pws_mode_apd(UBOOL8 enable)
{
    ethsw_setup_hw_apd(enable);
}

static void AEI_ethsw_set_pws_mode_eee(UBOOL8 enable)
{
    int i;
    uint32 v32;
    uint16 v16;

    down(&bcm_ethlock_switch_config);

    for (i = 0; i < TOTAL_SWITCH_PORTS; i++)
    {
        if (enable)
            v32 = 0x190; /* 400 us */
        else
            v32 = 0x4;   /* 4 us */

        v32 = swab32(v32);

        /* set the Giga EEE Sleep Timer */
        extsw_wreg(PAGE_EEE, REG_EEE_SLEEP_TIME_GIG + (i * 4), (uint8 *)&v32, 4);

        if (i == (TOTAL_SWITCH_PORTS - 1))
        {
            extsw_wreg(PAGE_EEE, REG_EEE_SLEEP_TIME_GIG_IMP, (uint8 *)&v32, 4);
        }

        if (enable)
            v32 = 0xfa0; /* 4000 us */
        else
            v32 = 0x28;  /* 40 us */

        v32 = swab32(v32);

        /* set the 100Mbps EEE Sleep Timer */
        extsw_wreg(PAGE_EEE, REG_EEE_SLEEP_TIME_FE + (i * 4), (uint8 *)&v32, 4);
    }

    if (enable)
        v16 = 0x13f;
    else
        v16 = 0x000;

    v16 = swab16(v16);

    /* enable/disable EEE on all ports */
    extsw_wreg(PAGE_EEE, REG_EEE_ENABLE_CTRL, (uint8 *)&v16, 2);

    up(&bcm_ethlock_switch_config);
}

static void AEI_ethsw_set_pws_mode_sc(UBOOL8 enable)
{
}

static void AEI_ethsw_set_pws_mode_dg(UBOOL8 enable)
{
}

void AEI_ethsw_disable_pws(void)
{
    ethsw_setup_hw_apd(FALSE);
    AEI_ethsw_set_pws_mode_eee(FALSE);
}

void AEI_ethsw_set_pws_mode(enum PowerSaveMode mode, UBOOL8 enable)
{
    switch (mode)
    {
        case PWS_AUTO_POWER_DOWN:
            AEI_ethsw_set_pws_mode_apd(enable);
            break;
        case PWS_ENERGY_EFFICIENT_ETHERNET:
            AEI_ethsw_set_pws_mode_eee(enable);
            break;
        case PWS_SHORT_CABLE:
            AEI_ethsw_set_pws_mode_sc(enable);
            break;
        case PWS_DEEP_GREEN:
            AEI_ethsw_set_pws_mode_dg(enable);
            break;
        default:
            break;
    }
}
#endif

static uint32 ephy_forced_pwr_down_status = 0;
#if defined(CONFIG_BCM_ETH_PWRSAVE)
/* Delay in miliseconds after enabling the EPHY PLL before reading the different EPHY status      */
/* The PLL requires 400 uSec to stabilize, but Energy detection on the ports requires more time. */
/* Normally, Energy detection works when PLL is down, but a long delay (minutes) is present     */
/* for ports that are in Auto Power Down mode. 40 mSec is chosen because this is the delay      */
/* that allows EPHY to send two link pulses (or series of pulses) at 16 mSec interval                  */
#define PHY_PLL_ENABLE_DELAY 1
#define PHY_PORT_ENABLE_DELAY 40

/* Describe internal PHYs */
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
static uint64 ephy_energy_det[NUM_INT_PHYS] = {1<<(INTERRUPT_ID_EPHY_ENERGY_0-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_1-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_2-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_3-INTERNAL_ISR_TABLE_OFFSET)};
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
static uint32 mdix_manual_swap = 0;
#endif

#elif defined(CONFIG_BCM96816)
static uint64 ephy_energy_det[NUM_INT_PHYS] = {((uint64)1)<<(INTERRUPT_ID_EPHY_ENERGY_0-INTERNAL_ISR_TABLE_OFFSET),
                                               ((uint64)1)<<(INTERRUPT_ID_EPHY_ENERGY_1-INTERNAL_ISR_TABLE_OFFSET)};
static uint32 gphy_pwr_dwn[NUM_INT_GPHYS] =   {GPHY_PWR_DOWN_0, GPHY_PWR_DOWN_1};
#define ROBOSWGPHYCTRL RoboswEphyCtrl
#undef PHY_PORT_ENABLE_DELAY
#define PHY_PORT_ENABLE_DELAY 300 // GPHY on 6816 need much more time to link when connecting one to the other

#elif defined(CONFIG_BCM963268)
static uint64 ephy_energy_det[NUM_INT_PHYS] = {1<<(INTERRUPT_ID_EPHY_ENERGY_0-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_1-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_EPHY_ENERGY_2-INTERNAL_ISR_TABLE_OFFSET),
                                               1<<(INTERRUPT_ID_GPHY_ENERGY_0-INTERNAL_ISR_TABLE_OFFSET)};
static uint32 gphy_pwr_dwn[NUM_INT_GPHYS] =   {GPHY_LOW_PWR};
#define ROBOSWGPHYCTRL RoboswGphyCtrl

#else
#error /* Add definitions for new chips */
#endif

#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
static uint32 ephy_pwr_down_status = 0;
#endif
unsigned int  ephy_auto_pwr_down_enabled = 1;
extern int vport_cnt;  /* number of vports: bitcount of Enetinfo.sw.port_map */

#ifdef CONFIG_BCM_ETH_PWRSAVE
static void ethsw_eee_all_enable(int enable);

void BcmPwrMngtSetEthAutoPwrDwn(unsigned int enable)
{
   ephy_auto_pwr_down_enabled = enable;
#if defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
   ethsw_setup_hw_apd(enable);
#endif
   ethsw_eee_all_enable(enable);

   printk("Ethernet pwrsaving is %s\n", enable?"enabled":"disabled");
}
EXPORT_SYMBOL(BcmPwrMngtSetEthAutoPwrDwn);

int BcmPwrMngtGetEthAutoPwrDwn(void)
{
   return (ephy_auto_pwr_down_enabled);
}
EXPORT_SYMBOL(BcmPwrMngtGetEthAutoPwrDwn);
#endif

int ethsw_phy_pll_up(int ephy_and_gphy)
{
    int ephy_status_changed = 0;

#if NUM_INT_GPHYS > 0
    if (ephy_and_gphy)
    {
        int i;
        uint32 roboswGphyCtrl = GPIO->ROBOSWGPHYCTRL;

        /* Bring up internal GPHY PLLs if they are down */
        for (i = 0; i < NUM_INT_GPHYS; i++)
        {
            if ((roboswGphyCtrl & gphy_pwr_dwn[i]) && !(ephy_forced_pwr_down_status & (1<<(i+NUM_INT_EPHYS))))
            {
                roboswGphyCtrl &= ~gphy_pwr_dwn[i];
                ephy_status_changed = 1;
            }
        }
        if (ephy_status_changed) {
            GPIO->ROBOSWGPHYCTRL = roboswGphyCtrl;
        }
    }
#endif

    /* This is a safety measure in case one tries to access the EPHY */
    /* while the PLL/RoboSw is powered down */
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
    if (GPIO->RoboswEphyCtrl & EPHY_PWR_DOWN_DLL)
    {
        /* Wait for PLL to stabilize before reading EPHY registers */
        GPIO->RoboswEphyCtrl &= ~EPHY_PWR_DOWN_DLL;
        ephy_status_changed = 1;
    }
#elif defined(CONFIG_BCM963268)
    if (!(PERF->blkEnables & RS_PLL250_CLK_EN))
    {
        /* Enable robosw clock */
        PERF->blkEnables |= RS_PLL250_CLK_EN;
        ephy_status_changed = 1;
    }
#endif

    if (ephy_status_changed) {
        if (irqs_disabled() || (preempt_count() != 0)) {
            mdelay(PHY_PLL_ENABLE_DELAY);
        } else {
            msleep(PHY_PLL_ENABLE_DELAY);
        }
        return (msecs_to_jiffies(PHY_PLL_ENABLE_DELAY));
    }
    return 0;
}

uint32 ethsw_ephy_auto_power_down_wakeup(void)
{
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
    int phy_id;
    int ephy_sleep_delay = 0;
    int ephy_status_changed = 0;
    int i;
    uint16 v16;
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);

    /* Ensure that only this thread accesses PHY registers in this interval */
    down(&bcm_ethlock_switch_config);

    /* Make sure EPHY PLL is up */
    ephy_sleep_delay = ethsw_phy_pll_up(1);

#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
    /* Update counter to toggle cross-over cable detection */
    mdix_manual_swap++;
#endif

    /* Make sure all PHY Ports are up */
    for (i = 0; i < NUM_INT_PHYS; i++)
    {
        if (ephy_pwr_down_status & (1<<i) && !(ephy_forced_pwr_down_status & (1<<(i))))
        {
#if NUM_INT_GPHYS > 0
            if (i >= NUM_INT_EPHYS)
            {
                // This GPHY port was down
                // Toggle pwr down bit, register 0, bit 11
                // if it was not already down, otherwise leave it down
                phy_id = priv->EnetInfo[0].sw.phy_id[i];
                ethsw_phy_rreg(phy_id, MII_CONTROL, &v16);
                if (!(v16 & MII_CONTROL_POWER_DOWN)) {
                    v16 |= MII_CONTROL_POWER_DOWN;
                    ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
                    v16 &= ~MII_CONTROL_POWER_DOWN;
                    ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
                    ephy_status_changed = 1;
                    ephy_sleep_delay += 3;
                }
                ephy_pwr_down_status &= ~(1<<i);
            }
#endif
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
            /* The following is a SW implementation of APD. It is preferable now to use HW APD when available */
            {
                /* This EPHY port is down, bring it up */
                phy_id = priv->EnetInfo[0].sw.phy_id[i];
                v16 = 0x008B;
                ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
#if defined(CONFIG_BCM96368)
                /* 6368 EPHYs are finicky and re-enabling Rx sometimes */
                /* takes too long, so we only do Tx */
                /* Enable Rx first */
                /* v16 = 0x0000; */
                /* ethsw_phy_wreg(phy_id, 0x14, &v16); */
                /* Then Tx */
                v16 = 0x0000;
                ethsw_phy_wreg(phy_id, 0x10, &v16);
#elif defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
                /* Enable Rx first */
                v16 = 0x0008;
                ethsw_phy_wreg(phy_id, 0x14, &v16);
                /* Then Tx */
                v16 = 0x0400;
                ethsw_phy_wreg(phy_id, 0x10, &v16);
                ephy_sleep_delay += 1;
#endif
                v16 = 0x000B;
                ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
                ephy_pwr_down_status &= ~(1<<i);
                ephy_status_changed = 1;
                ephy_sleep_delay += 3;

                /* If the port has no energy, swap MDIX control */
                /* (cross-over/straight-through support) every second */
                if (!(PERF->IrqControl[0].IrqStatus & ephy_energy_det[i]))
                {
                    ethsw_phy_rreg(phy_id, 0x1c, &v16);
                    v16 |= 0x0800 | (mdix_manual_swap&1)?0x1000:0x0000;
                    ethsw_phy_wreg(phy_id, 0x1c, &v16);
                    ephy_sleep_delay += 2;
                }
           }
#endif
        }
    }

    up(&bcm_ethlock_switch_config);

    if (ephy_status_changed)
    {
        /* Allow the ports to be enabled and transmitting link pulses */
        msleep(PHY_PORT_ENABLE_DELAY);
        ephy_sleep_delay += msecs_to_jiffies(PHY_PORT_ENABLE_DELAY);
    }

    return ephy_sleep_delay;
#else
    return ethsw_phy_pll_up(1);
#endif
}

uint32 ethsw_ephy_auto_power_down_sleep(void)
{
    int i;
    int ephy_sleep_delay = 0;
    uint64 irqStatus = PERF->IrqControl[0].IrqStatus;
    int ephy_has_energy = 0;
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    int map = priv->EnetInfo[0].sw.port_map;
    int phy_id;
    uint16 v16;

    if (!ephy_auto_pwr_down_enabled)
    {
        return ephy_sleep_delay;
    }

    /* Ensure that only this thread accesses PHY registers in this interval */
    down(&bcm_ethlock_switch_config);

    /* Turn off EPHY Ports that have no energy */
    for (i = 0; i < NUM_INT_PHYS; i++)
    {
        if (map & (1<<i))
        {
            /* Verify if the link is down, don't want to force down while the link is still up */
            phy_id = priv->EnetInfo[0].sw.phy_id[i];
            ethsw_phy_rreg(phy_id, 0x1, &v16);
            if (!(irqStatus & ephy_energy_det[i]) || (!(v16&0x4) && (ephy_forced_pwr_down_status & (1<<(i)))))
            {
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE) && NUM_INT_GPHYS > 0
                if (i >= NUM_INT_EPHYS)
                {
                    /* This GPHY port has no energy, bring it down */
                    GPIO->ROBOSWGPHYCTRL |= gphy_pwr_dwn[i-NUM_INT_EPHYS];
                    ephy_pwr_down_status |= (1<<i);
                }
#endif
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
                /* The following is a SW implementation of APD. It is preferable now to use HW APD when available */
                {
                    /* This EPHY port has no energy, bring it down */
                    v16 = 0x008B;
                    ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
#if defined(CONFIG_BCM96368)
                    /* 6368 EPHYs are finicky and re-enabling Rx sometimes */
                    /* takes too long, so we only do Tx */
                    v16 = 0x01c0;
                    ethsw_phy_wreg(phy_id, 0x10, &v16);
                    /* v16 = 0x7000; */
                    /* ethsw_phy_wreg(phy_id, 0x14, &v16); */
#elif defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
                    v16 = 0x0700;
                    ethsw_phy_wreg(phy_id, 0x10, &v16);
                    v16 = 0x6008;
                    ethsw_phy_wreg(phy_id, 0x14, &v16);
                    ephy_sleep_delay += 1;
#endif
                    v16 = 0x000B;
                    ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
#if !defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
                    ephy_pwr_down_status |= (1<<i);
#endif
                    ephy_sleep_delay += 3;
                }
#endif
            }
            else
            {
                ephy_has_energy = 1;
            }
        }
    }

#if !defined(CONFIG_BCM96816)
    if (priv->extSwitch->brcm_tag_type != BRCM_TYPE2) {
        /* If no energy was found on any PHY and no other switch port is linked, bring down PLL to save power */
        if (!ephy_has_energy && !priv->linkState)
        {
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
            GPIO->RoboswEphyCtrl |= EPHY_PWR_DOWN_DLL;
#elif defined(CONFIG_BCM963268)
            PERF->blkEnables &= ~RS_PLL250_CLK_EN;
#endif
        }
    }
#endif

    up(&bcm_ethlock_switch_config);

    return ephy_sleep_delay;
}
#endif

void ethsw_switch_power_off(void *context)
{
}

void ethsw_init_config(void)
{
    int i;
#if defined(AEI_VDSL_HPNA)
    uint32 v32;
#endif

    /* Save the state that is restored in enable_hw_switching */
    for(i = 0; i < TOTAL_SWITCH_PORTS; i++)  {
        ethsw_rreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                   (uint8 *)&pbvlan_map[i], 2);
    }
    ethsw_rreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&dis_learning, 2);
    ethsw_rreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&port_fwd_ctrl, 1);

#if defined(CONFIG_BCM963268)
    {
        /* Disable tags for internal switch ports */
        uint32 tmp;
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&tmp, 4);
        tmp |= REG_IUDMA_CTRL_TX_MII_TAG_DISABLE;
        ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&tmp, 4); 
    }
#endif

#if defined(AEI_VDSL_HPNA)
    if (hpna_support)
    {
        extsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8 *)&v32, sizeof(v32));
        v32 = swab32(v32);

        v32 |= (REG_PAUSE_CAPBILITY_OVERRIDE |
            // pause control for hpna port
            REG_PAUSE_CAPBILITY_GMII1_TX | REG_PAUSE_CAPBILITY_GMII1_RX |
            // need this to cut wan/BA ftp server side error
            REG_PAUSE_CAPBILITY_GMII0_TX |
            // need this to cut lan/BA ftp client side errors
            REG_PAUSE_CAPBILITY_EPHY0_TX |
            REG_PAUSE_CAPBILITY_EPHY1_TX |
            REG_PAUSE_CAPBILITY_EPHY2_TX |
            REG_PAUSE_CAPBILITY_EPHY3_TX);

        v32 = swab32(v32);
        extsw_wreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8 *)&v32, sizeof(v32));
    }
#endif
}

int ethsw_setup_led(void)
{
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int phy_id, i;
    uint16 v16;

#if defined(AEI_VDSL_CUSTOMER_NCS)
    uint32 v32;

    extsw_rreg(PAGE_MANAGEMENT, REG_DEV_ID, (uint8 *)&v32, sizeof(v32));
    v32 = swab32(v32);
    if (v32 == 0x53115)
    {
        /* Enable traffic blinking for all ethernet port LEDs */
        v16 = REG_LED_FUNCTION1_LNK_ACT | REG_LED_FUNCTION1_10M_ACT | REG_LED_FUNCTION1_100M_ACT | REG_LED_FUNCTION1_1000M_ACT;
        v16 = swab16(v16);
        extsw_wreg(PAGE_CONTROL, REG_LED_FUNCTION1_CTRL, (uint8 *)&v16, sizeof(v16));
    }
#endif

    /* For each port that has an internal or external PHY, configure it
       as per the required initial configuration */
    for (i = 0; i < (TOTAL_SWITCH_PORTS - 1); i++) {
        /* Check if the port is in the portmap or not */
        if ((pVnetDev0->EnetInfo[0].sw.port_map & (1U<<i)) != 0) {
            /* Check if the port is connected to a PHY or not */
            phy_id = pVnetDev0->EnetInfo[0].sw.phy_id[i];
            /* If a Phy is connected, set it up with initial config */
            /* TBD: Maintain the config for each Phy */
            if(IsPhyConnected(phy_id) && !IsExtPhyId(phy_id)) {
#if defined(CONFIG_BCM96368)
                v16 = 1 << 2;
                ethsw_phy_wreg(phy_id, MII_TPISTATUS, &v16);
#elif defined(CONFIG_BCM96816)
                v16 = 0xa410;
                ethsw_phy_wreg(phy_id, 0x1c, &v16);
#elif defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
                v16 = 0xa410;
                // Enable Shadow register 2
                ethsw_phy_rreg(phy_id, MII_BRCM_TEST, &v16);
                v16 |= MII_BRCM_TEST_SHADOW2_ENABLE;
                ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
#if defined(CONFIG_BCM963268)
                if (i != GPHY_PORT_ID) {
                    // Set LED1 to speed. Set LED0 to blinky link
                    v16 = 0x08;
                }
#else
                // Set LED0 to speed. Set LED1 to blinky link
                v16 = 0x71;
#endif
                ethsw_phy_wreg(phy_id, 0x15, &v16);
                // Disable Shadow register 2
                ethsw_phy_rreg(phy_id, MII_BRCM_TEST, &v16);
                v16 &= ~MII_BRCM_TEST_SHADOW2_ENABLE;
                ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
#endif
            }
#if defined(CONFIG_BCM96816)
            if(IsExt6829(phy_id))
            {
                int phyId6829;
                int j;

                phy_id &= ~(BCM_EXT_6829 | BCM_WAN_PORT);
                for ( j = 0; j < (TOTAL_SWITCH_PORTS - 1); j++ )
                {
                    if ((phy_id & (1U<<j)) != 0)
                    {
                        /* ids for 6829 match 6819 ids*/
                        phyId6829 = pVnetDev0->EnetInfo[0].sw.phy_id[j] & ~BCM_WAN_PORT;
                        if ( !IsExtPhyId(phyId6829) )
                        {
                            phyId6829 |= BCM_EXT_6829;
                            v16        = 0xa410;
                            ethsw_phy_wreg(phyId6829, 0x1c, &v16);
                        }
                    }
                }
            }
#endif
            if (IsExtPhyId(phy_id)) {
                /* Configure LED for link/activity */
                v16 = MII_1C_SHADOW_LED_CONTROL << MII_1C_SHADOW_REG_SEL_S;
                ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
                ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
                v16 |= ACT_LINK_LED_ENABLE;
                v16 |= MII_1C_WRITE_ENABLE;
                v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
                v16 |= (MII_1C_SHADOW_LED_CONTROL << MII_1C_SHADOW_REG_SEL_S);
                ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);

                ethsw_phy_rreg(phy_id, MII_PHYSID2, &v16);
                if ((v16 & BCM_PHYID_M) == (BCM54610_PHYID2 & BCM_PHYID_M)) {
                    /* Configure LOM LED Mode */
                    v16 = MII_1C_EXTERNAL_CONTROL_1 << MII_1C_SHADOW_REG_SEL_S;
                    ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
                    ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
                    v16 |= LOM_LED_MODE;
                    v16 |= MII_1C_WRITE_ENABLE;
                    v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
                    v16 |= (MII_1C_EXTERNAL_CONTROL_1 << MII_1C_SHADOW_REG_SEL_S);
                    ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
                }
            }
        }
    }
    return 0;
}

int ethsw_reset_ports(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int map, cnt, i;
    uint16 v16;
    int phyid;
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM96816)
    uint8 v8;
#endif

    ethsw_init_table(&pDevCtrl->EnetInfo[0].sw);

    if (pDevCtrl->unit == 1) {
        for (i = 0; i < TOTAL_SWITCH_PORTS-1; i++)
        {
            ext_switch_pport_phyid[i] = pDevCtrl->EnetInfo[1].sw.phy_id[i];
        }
    }

    map = pDevCtrl->EnetInfo[0].sw.port_map;
    bitcount(cnt, map);

    if (cnt <= 0)
        return 0;

    // MAC level reset
    for (i = 0; i < 8; i++)
    {
#if defined(CONFIG_BCM96816)
        int phy_id = pDevCtrl->EnetInfo[0].sw.phy_id[i];
        if(IsExt6829(phy_id))
        {
            int j;
            phy_id &= ~(BCM_EXT_6829 | BCM_WAN_PORT);
            for ( j = 0; j < (TOTAL_SWITCH_PORTS - 1); j++ )
            {
                v8 = ((phy_id & (1U<<j)) != 0) ? 0: REG_PORT_CTRL_DISABLE;
                if ( j == SERDES_PORT_ID )
                {
                   /* make sure 6829 SERDES port stays active */
                   v8 = 0;
                }
                ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_CTRL + j, &v8, 1, 1);
            }
        }
#endif
    }

#if defined(CONFIG_BCM963268)
    if (map & (1 << (RGMII_PORT_ID + 1))) {
        GPIO->RoboswSwitchCtrl |= RSW_MII_2_IFC_EN;
    }
#endif

    for (i = 0; i < NUM_RGMII_PORTS; i++) {

        phyid = pDevCtrl->EnetInfo[0].sw.phy_id[RGMII_PORT_ID + i];

#if defined(CONFIG_BCM963268)
        ethsw_rreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
        v8 |= REG_RGMII_CTRL_ENABLE_RGMII_OVERRIDE;
        v8 &= ~REG_RGMII_CTRL_MODE;
        if (IsRGMII(phyid)) {
            v8 |= REG_RGMII_CTRL_MODE_RGMII;
        } else if (IsRvMII(phyid)) {
            v8 |= REG_RGMII_CTRL_MODE_RvMII;
        } else {
            v8 |= REG_RGMII_CTRL_MODE_MII;
        }
        if ((pDevCtrl->chipRev == 0xA0) || (pDevCtrl->chipRev == 0xB0)) {
            /* RGMII timing workaround */
            v8 &= ~REG_RGMII_CTRL_TIMING_SEL;
        } else {
            v8 |= REG_RGMII_CTRL_TIMING_SEL;
        }

        ethsw_wreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
        if ((pDevCtrl->chipRev == 0xA0) || (pDevCtrl->chipRev == 0xB0)) {
            /* RGMII timing workaround */
            v8 = 0xAB;
            ethsw_wreg(PAGE_CONTROL, REG_RGMII_TIMING_P4 + i, &v8, 1);
        }
#endif

        /* No need to check the PhyID if the board params is set correctly for RGMII. However, keeping
              *   the phy id check to make it work even when customer does not set the RGMII flag in the phy_id
              *   in board params
              */
        ethsw_phy_rreg(phyid, MII_PHYSID2, &v16);
        if ((IsRGMII(phyid) && IsPhyConnected(phyid)) ||
            ((v16 & BCM_PHYID_M) == (BCM54610_PHYID2 & BCM_PHYID_M)) ||
            ((v16 & BCM_PHYID_M) == (BCM50612_PHYID2 & BCM_PHYID_M))) {

            v16 = MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S;
            ethsw_phy_wreg(phyid, MII_REGISTER_1C, &v16);
            ethsw_phy_rreg(phyid, MII_REGISTER_1C, &v16);
#if defined(CONFIG_BCM963268)
            /* Temporary work-around for MII2 port RGMII delay programming */
            if (i == 1 && ((pDevCtrl->chipRev == 0xA0) || (pDevCtrl->chipRev == 0xB0)) )
                v16 |= GTXCLK_DELAY_BYPASS_DISABLE;
            else
#endif
                v16 &= (~GTXCLK_DELAY_BYPASS_DISABLE);
            v16 |= MII_1C_WRITE_ENABLE;
            v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
            v16 |= (MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S);
            ethsw_phy_wreg(phyid, MII_REGISTER_1C, &v16);
        }
    }

    /*Remaining port reset functionality is moved into ethsw_init_hw*/

    return 0;
}

#ifdef NO_CFE
void ethsw_configure_ports(int port_map, int *pphy_id)
{
    uint8 v8;
    int i, phy_id;

    for (i = 0; i < 8; i++) {
        v8 = REG_PORT_CTRL_DISABLE;
        ethsw_wreg_ext(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1, 0);

        if ((port_map & (1U<<i)) != 0) {
#if defined(CONFIG_BCM96368)
            if (i == 4)
                GPIO->GPIOBaseMode |= (EN_GMII1);
            if (i == 5)
                GPIO->GPIOBaseMode |= (EN_GMII2);
#elif defined(CONFIG_BCM96816)
            if (i == 2)
                GPIO->GPIOBaseMode |= (EN_GMII1);
            if (i == 3)
                GPIO->GPIOBaseMode |= (EN_GMII2);
#elif defined(CONFIG_BCM96362)
            if (i == 4)
                GPIO->RoboswSwitchCtrl |= (RSW_MII_SEL_2P5V << RSW_MII_SEL_SHIFT);
            if (i == 5)
                GPIO->RoboswSwitchCtrl |= (RSW_MII_2_IFC_EN | (RSW_MII_SEL_2P5V << RSW_MII_2_SEL_SHIFT));
#elif defined(CONFIG_BCM96328)
            if (i == 4)
                MISC->miscPadCtrlHigh |= (MISC_MII_SEL_2P5V << MISC_MII_SEL_SHIFT);
#endif
        }
#if defined(CONFIG_BCM96816)
        else {
            phy_id = *(pphy_id + i);
            if (!IsExtPhyId(phy_id)) {
                if (i == 0)
                    GPIO->RoboswEphyCtrl |= (GPHY_PWR_DOWN_0 | GPHY_PWR_DOWN_SD_0);
                if (i == 1)
                    GPIO->RoboswEphyCtrl |= (GPHY_PWR_DOWN_1 | GPHY_PWR_DOWN_SD_1);
            }
        }
#endif
    }

    v8 = LINK_OVERRIDE_1000FDX;
    ethsw_wreg_ext(PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, &v8, 1, 0);
}
#endif


static uint8 swdata[16];
static uint8 miidata[16];
#if defined(AEI_VDSL_EXTSW_DEBUG)
static uint8 is_extsw = 0;
#endif

int ethsw_add_proc_files(struct net_device *dev)
{
    struct proc_dir_entry *p;

    p = create_proc_entry("switch", 0644, NULL);

    if (p == NULL)
        return -1;

    memset(swdata, 0, sizeof(swdata));

    p->data        = dev;
    p->read_proc   = proc_get_sw_param;
    p->write_proc  = proc_set_sw_param;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
    p->owner       = THIS_MODULE;
#endif

    p = create_proc_entry("mii", 0644, NULL);

    if (p == NULL)
        return -1;

    memset(miidata, 0, sizeof(miidata));

    p->data       = dev;
    p->read_proc  = proc_get_mii_param;
    p->write_proc = proc_set_mii_param;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
    p->owner      = THIS_MODULE;
#endif

    return 0;
}

int ethsw_del_proc_files(void)
{
    remove_proc_entry("switch", NULL);

    remove_proc_entry("mii", NULL);
    return 0;
}

static void str_to_num(char* in, char* out, int len)
{
    int i;
    memset(out, 0, len);

    for (i = 0; i < len * 2; i ++)
    {
        if ((*in >= '0') && (*in <= '9'))
            *out += (*in - '0');
        else if ((*in >= 'a') && (*in <= 'f'))
            *out += (*in - 'a') + 10;
        else if ((*in >= 'A') && (*in <= 'F'))
            *out += (*in - 'A') + 10;
        else
            *out += 0;

        if ((i % 2) == 0)
            *out *= 16;
        else
            out ++;

        in ++;
    }
    return;
}

static int proc_get_sw_param(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
    int reg_page  = swdata[0];
    int reg_addr  = swdata[1];
    int reg_len   = swdata[2];
    int i = 0;
    int r = 0;

    *eof = 1;

    if (reg_len == 0)
        return 0;

#if defined(AEI_VDSL_EXTSW_DEBUG)
    if (is_extsw)
    {
        extsw_rreg(reg_page, reg_addr, swdata + 3, reg_len);

        if (reg_len == 2)
        {
            ((uint16 *)(swdata + 3))[0] = swab16(((uint16 *)(swdata + 3))[0]);
        }
        else if (reg_len == 4)
        {
            ((uint32 *)(swdata + 3))[0] = swab32(((uint32 *)(swdata + 3))[0]);
        }
        else if (reg_len == 6)
        {
            if ((reg_addr %4) == 0)
            {
                ((uint32 *)(swdata + 3))[0] = swab32(((uint32 *)(swdata + 3))[0]);
                ((uint16 *)(swdata + 7))[0] = swab16(((uint16 *)(swdata + 7))[0]);
            }
            else
            {
                ((uint16 *)(swdata + 3))[0] = swab32(((uint16 *)(swdata + 3))[0]);
                ((uint32 *)(swdata + 5))[0] = swab16(((uint32 *)(swdata + 5))[0]);
            }
        }
        else if (reg_len == 8)
        {
           ((uint32 *)(swdata + 3))[0] = swab32(((uint32 *)(swdata + 3))[0]);
           ((uint32 *)(swdata + 3))[1] = swab32(((uint32 *)(swdata + 3))[1]);
        }
    }
    else
    {
        down(&bcm_ethlock_switch_config);
        ethsw_rreg(reg_page, reg_addr, swdata + 3, reg_len);
        up(&bcm_ethlock_switch_config);
    }
    
    r += sprintf(page + r, "%s switch:", (is_extsw) ? "External" : "Internal");
#else 
    down(&bcm_ethlock_switch_config);
    ethsw_rreg(reg_page, reg_addr, swdata + 3, reg_len);
    up(&bcm_ethlock_switch_config);
#endif

    r += sprintf(page + r, "[%02x:%02x] = ", swdata[0], swdata[1]);

    for (i = 0; i < reg_len; i ++)
        r += sprintf(page + r, "%02x ", swdata[3 + i]);

    r += sprintf(page + r, "\n");
    return (r < cnt)? r: 0;
}

static int proc_set_sw_param(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    char input[32];
    int i;
    int r;
    int num_of_octets;

    int reg_page;
    int reg_addr;
    int reg_len;

    if (cnt > 32)
        cnt = 32;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    r = cnt;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]))
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    num_of_octets = r / 2;

    if (num_of_octets < 3) // page, addr, len
        return -EFAULT;

    str_to_num(input, swdata, num_of_octets);

    reg_page  = swdata[0];
    reg_addr  = swdata[1];
    reg_len   = swdata[2];

#if defined(AEI_VDSL_EXTSW_DEBUG)
    is_extsw = 0;
#endif

    if (((reg_len != 1) && (reg_len % 2) != 0) || reg_len > 8)
    {
        memset(swdata, 0, sizeof(swdata));
        return -EFAULT;
    }

#if defined(AEI_VDSL_EXTSW_DEBUG)
    if (num_of_octets > 3)
    {
        if (num_of_octets == reg_len + 4)
        {
            is_extsw = ((swdata[reg_len + 3]) ? 1 : 0);
            swdata[reg_len + 3] = 0;
        }
        else if (num_of_octets != reg_len + 3)
        {
            memset(swdata, 0, sizeof(swdata));
            return -EFAULT;
        }

        if (is_extsw)
        {
            if (reg_len == 2)
            {
                ((uint16 *)(swdata + 3))[0] = swab16(((uint16 *)(swdata + 3))[0]);
            }
            else if (reg_len == 4)
            {
                ((uint32 *)(swdata + 3))[0] = swab32(((uint32 *)(swdata + 3))[0]);
            }
            else if (reg_len == 6)
            {
                if ((reg_addr %4) == 0)
                {
                    ((uint32 *)(swdata + 3))[0] = swab32(((uint32 *)(swdata + 3))[0]);
                    ((uint16 *)(swdata + 7))[0] = swab16(((uint16 *)(swdata + 7))[0]);
                }
                else
                {
                    ((uint16 *)(swdata + 3))[0] = swab32(((uint16 *)(swdata + 3))[0]);
                    ((uint32 *)(swdata + 5))[0] = swab16(((uint32 *)(swdata + 5))[0]);
                }
            }
            else if (reg_len == 8)
            {
               ((uint32 *)(swdata + 3))[0] = swab32(((uint32 *)(swdata + 3))[0]);
               ((uint32 *)(swdata + 3))[1] = swab32(((uint32 *)(swdata + 3))[1]);
            }

            extsw_wreg(reg_page, reg_addr, swdata + 3, reg_len);
        }
        else
        {
            down(&bcm_ethlock_switch_config);
            ethsw_wreg(reg_page, reg_addr, swdata + 3, reg_len);
            up(&bcm_ethlock_switch_config);
        }
    }
#else
    if ((num_of_octets > 3) && (num_of_octets != reg_len + 3))
    {
        memset(swdata, 0, sizeof(swdata));
        return -EFAULT;
    }

    if (num_of_octets > 3) {
        down(&bcm_ethlock_switch_config);
        ethsw_wreg(reg_page, reg_addr, swdata + 3, reg_len);
        up(&bcm_ethlock_switch_config);
    }
#endif
    return cnt;
}

static int proc_get_mii_param(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
    int mii_port  = miidata[0];
    int mii_addr  = miidata[1];
    int r = 0;

    *eof = 1;

    down(&bcm_ethlock_switch_config);
    ethsw_phy_rreg(mii_port, mii_addr, (uint16 *)(miidata + 2));
    up(&bcm_ethlock_switch_config);

    r += sprintf(
        page + r,
        "[%02x:%02x] = %02x %02x\n",
        miidata[0], miidata[1], miidata[2], miidata[3]
    );

    return (r < cnt)? r: 0;
}

static int proc_set_mii_param(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    char input[32];
    int i;
    int r;
    int num_of_octets;

    int mii_port;
    int mii_addr;

    if (cnt > 32)
        cnt = 32;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    r = cnt;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]))
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    num_of_octets = r / 2;

    if ((num_of_octets!= 2) && (num_of_octets != 4))
    {
        memset(miidata, 0, sizeof(miidata));
        return -EFAULT;
    }

    str_to_num(input, miidata, num_of_octets);
    mii_port  = miidata[0];
    mii_addr  = miidata[1];

    down(&bcm_ethlock_switch_config);

    if (num_of_octets > 2)
        ethsw_phy_wreg(mii_port, mii_addr, (uint16 *)(miidata + 2));

    up(&bcm_ethlock_switch_config);
    return cnt;
}

#if defined(CONFIG_BCM96368)
/*
*------------------------------------------------------------------------------
* Function   : ethsw_enable_sar_port
* Description: Setup the SAR port of a 6368 Switch as follows:
*              - Learning is disabled.
*              - Full duplex, 1000Mbs, Link Up
*              - Rx Enabled, Tx Disabled.
*
* Design Note: Invoked by SAR Packet CMF when SAR XTM driver is operational,
*              via CMF Hook pktCmfSarPortEnable.
*
*              This function expects to be called from a softirq context.
*------------------------------------------------------------------------------
*/
int ethsw_enable_sar_port(void)
{
    uint8  val8;
    uint16 val16;

    ethsw_rreg(PAGE_CONTROL, REG_DISABLE_LEARNING,
        (uint8 *)&val16, sizeof(val16) );
    val16 |= ( 1 << SAR_PORT_ID );  /* add sar_port_id to disabled ports */
    ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING,
        (uint8 *)&val16, sizeof(val16) );

    val8 = ( REG_PORT_STATE_OVERRIDE    /* software override Phy values */
        | REG_PORT_STATE_1000        /* Speed of 1000 Mbps */
        | REG_PORT_STATE_FDX         /* Full duplex mode */
        | REG_PORT_STATE_LNK );      /* Link State Up */
    ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + SAR_PORT_ID,
        &val8, sizeof(val8) );

    val8 = REG_PORT_TX_DISABLE;
    ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + SAR_PORT_ID,
        &val8, sizeof(val8) );

    return 0;
}

/*
*------------------------------------------------------------------------------
* Function   : ethsw_disable_sar_port
* Description: Setup the SAR port of a 6368 Switch as follows:
*              - Link Sown
*              - Rx Disabled, Tx Disabled.
*
* Design Note: Invoked via CMF Hook pktCmfSarPortDisable.
*
*              This function expects to be called from a softirq context.
*------------------------------------------------------------------------------
*/
int ethsw_disable_sar_port(void)
{
    uint8  val8;

    val8 = REG_PORT_STATE_OVERRIDE;
    ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + SAR_PORT_ID,
        &val8, sizeof(val8) );

    val8 = (REG_PORT_TX_DISABLE | REG_PORT_RX_DISABLE);
    ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + SAR_PORT_ID,
        &val8, sizeof(val8) );

    return 0;
}
#endif

int ethsw_enable_hw_switching(void)
{
    u8 i;

    down(&bcm_ethlock_switch_config);

    /*Don't do anything if already enabled.
     *Enable is implemented by restoring values saved by disable_hw_switching().
     *This check is necessary to make sure we get correct behavior when
     *enable_hw_switching() is called without a preceeding disable_hw_switching() call.
     */
    if (hw_switching_state != HW_SWITCHING_ENABLED) {
      /* restore pbvlan config */
      for(i = 0; i < TOTAL_SWITCH_PORTS; i++)
      {
          ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                     (uint8 *)&pbvlan_map[i], 2);
      }

      /* restore disable learning register */
      ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&dis_learning, 2);

      /* restore port forward control register */
      ethsw_wreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&port_fwd_ctrl, 1);

      i = 0;
      while (vnet_dev[i])
      {
          /* When hardware switching is enabled, enable the Linux bridge to
             not to forward the bcast packets on hardware ports */
          vnet_dev[i++]->priv_flags |= IFF_HW_SWITCH;
      }
#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
      for (i = 0; i < MAX_GEM_IDS; i++)
      {
          if (gponifid_to_dev[i]) {
              /* When hardware switching is enabled, enable the Linux bridge to
                 not to forward the bcast packets on hardware ports */
              gponifid_to_dev[i]->priv_flags |= IFF_HW_SWITCH;
          }
      }
#endif
#if defined(CONFIG_BCM96816)
      for (i = 0; i < MAX_6829_IFS; i++)
      {
          if ( bcm6829_to_dev[i] )
          {
              /* When hardware switching is enabled, enable the Linux bridge to
                 not to forward the bcast packets on hardware ports */
              bcm6829_to_dev[i]->priv_flags |= IFF_HW_SWITCH;
          }
      }
#endif

      hw_switching_state = HW_SWITCHING_ENABLED;
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int ethsw_disable_hw_switching(void)
{
    u8 i, byte_value;
    u16 reg_value;

    down(&bcm_ethlock_switch_config);

    /*Don't do anything if already disabled.*/
    if (hw_switching_state == HW_SWITCHING_ENABLED) {

       /* set the port-based vlan control reg of each port with fwding mask of
          only that port and MIPS. For MIPS port, set the forwarding mask of
          all the ports */
       for(i = 0; i < TOTAL_SWITCH_PORTS; i++)
       {
           ethsw_rreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                      (uint8 *)&pbvlan_map[i], 2);
           if (i == MIPS_PORT_ID) {
               reg_value = PBMAP_ALL;
           }
           else {
               reg_value = PBMAP_MIPS;
           }
           ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                      (uint8 *)&reg_value, 2);
       }

       /* Save disable_learning_reg setting */
       ethsw_rreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&dis_learning, 2);
       /* disable learning on all ports */
       reg_value = PBMAP_ALL;
       ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&reg_value, 2);

       /* Save port forward control setting */
       ethsw_rreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&port_fwd_ctrl, 1);
       /* flood unlearned packets */
       byte_value = 0x00;
       ethsw_wreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&byte_value, 1);

       i = 0;
       while (vnet_dev[i])
       {
           /* When hardware switching is disabled, enable the Linux bridge to
              forward the bcast on hardware ports as well */
           vnet_dev[i++]->priv_flags &= ~IFF_HW_SWITCH;
       }

#if (defined(CONFIG_BCM96816) && defined(DBL_DESC))
       for (i = 0; i < MAX_GEM_IDS; i++)
       {
           if (gponifid_to_dev[i]) {
               /* When hardware switching is enabled, enable the Linux bridge to
                  not to forward the bcast on hardware ports */
               gponifid_to_dev[i]->priv_flags &= ~IFF_HW_SWITCH;
           }
       }
#endif
#if defined(CONFIG_BCM96816)
       for (i = 0; i < MAX_6829_IFS; i++)
       {
           if ( bcm6829_to_dev[i] )
           {
               /* When hardware switching is enabled, enable the Linux bridge to
                  not to forward the bcast packets on hardware ports */
               bcm6829_to_dev[i]->priv_flags &= ~IFF_HW_SWITCH;
           }
       }
#endif

       hw_switching_state = HW_SWITCHING_DISABLED;

       /* Flush arl table dynamic entries */
       fast_age_all(0);
    }
    up(&bcm_ethlock_switch_config);
    return 0;
}

int ethsw_get_hw_switching_state(void)
{
    return hw_switching_state;
}

#if defined(CONFIG_BCM96368)
void ethsw_switch_manage_port_power_mode(int portnumber, int power_mode)
{
   uint16 reg = 0;
   int phy_id = switch_pport_phyid[portnumber];

   down(&bcm_ethlock_switch_config);

   if(!power_mode) {
      // To power down an EPHY channel, the following sequence should be used
      reg=0x008B; ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &reg); //Shadow reg enabled
      reg=0x01C0; ethsw_phy_wreg(phy_id, 0x10, &reg);
      reg=0x7000; ethsw_phy_wreg(phy_id, 0x14, &reg);
      reg=0x000F; ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &reg); //Shadow reg 2 enabled
      reg=0x20D0; ethsw_phy_wreg(phy_id, 0x10, &reg);
      reg=0x000B; ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &reg); //Shadow regs disabled
      // Set EPHY_PWR_DOWN_x bit of this register to 0x1
      GPIO->RoboswEphyCtrl |= (1 << (2 + portnumber));
      ephy_forced_pwr_down_status |= (1<<portnumber);
   }
   else {
      // Set EPHY_PWR_DOWN_x bit of this register to 0x0
      GPIO->RoboswEphyCtrl &= ~(1 << (2 + portnumber));
      // To get out of power down an EPHY channel, the following sequence should be used
      reg=0x008B; ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &reg); //Shadow reg enabled
      reg=0x0000; ethsw_phy_wreg(phy_id, 0x10, &reg);
      reg=0x0000; ethsw_phy_wreg(phy_id, 0x14, &reg);
      reg=0x000F; ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &reg); //Shadow reg 2 enabled
      reg=0x00D0; ethsw_phy_wreg(phy_id, 0x10, &reg);
      reg=0x000B; ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &reg); //Shadow regs disabled
      ephy_forced_pwr_down_status &= ~(1<<portnumber);
   }

   up(&bcm_ethlock_switch_config);
}
#else
extern atomic_t phy_write_ref_cnt;
extern atomic_t phy_read_ref_cnt;

void ethsw_switch_manage_port_power_mode(int portnumber, int power_mode)
{
   uint16 reg = 0; /* MII reg 0x00 */
   uint16 v16 = 0;
   int phy_id = switch_pport_phyid[portnumber];

   down(&bcm_ethlock_switch_config);

   /* GPHYs and external PHYs */
   if (
#if NUM_INT_GPHYS > 0
       ((portnumber >= NUM_INT_EPHYS) && (portnumber < NUM_INT_PHYS)) ||
#endif
       (IsExtPhyId(phy_id)))
   {
       if(!power_mode) {
          /* MII Power Down enable, forces link down */
          ethsw_phy_rreg(phy_id, reg, &v16);
          v16 |= 0x0800;
          ethsw_phy_wreg(phy_id, reg, &v16);
          ephy_forced_pwr_down_status |= (1<<portnumber);
       }
       else {
          /* MII Power Down disable */
          ethsw_phy_rreg(phy_id, reg, &v16);
          v16 &= ~0x0800;
          ethsw_phy_wreg(phy_id, reg, &v16);
          ephy_forced_pwr_down_status &= ~(1<<portnumber);
       }
   }
#if NUM_INT_EPHYS > 0
   else if (portnumber < NUM_INT_EPHYS) {  /* EPHYs */
       /* When the link is being brought up or down, the link status interrupt may occur
          before this command is completed, where the PHY is configured in shadow mode.
          We need to prevent this by disabling EPHY interrupts. The problem does not exist
          for GPHY because the link status changes in a single command.
       */
       atomic_inc(&phy_write_ref_cnt);
       atomic_inc(&phy_read_ref_cnt);
       BcmHalInterruptDisable(INTERRUPT_ID_EPHY);

       if(!power_mode) {
          /* Bring it down */
          v16 = 0x008B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          v16 = 0x0700; /* tx pwr down */
          ethsw_phy_wreg(phy_id, 0x10, &v16);
          msleep(1); /* Without this, the speed LED on 63168 stays on */
          v16 = 0x7008; /* rx pwr down & link status disable */
          ethsw_phy_wreg(phy_id, 0x14, &v16);
          v16 = 0x000B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          ephy_forced_pwr_down_status |= (1<<portnumber);
       }
       else {
          /* Bring it up */
          v16 = 0x008B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          v16 = 0x0400;
          ethsw_phy_wreg(phy_id, 0x10, &v16);
          v16 = 0x0008;
          ethsw_phy_wreg(phy_id, 0x14, &v16);
          v16 = 0x000B;
          ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
          /* Restart Autoneg in case the cable was unplugged or plugged while down */
          ethsw_phy_rreg(phy_id, MII_CONTROL, &v16);
          v16 |= MII_CONTROL_RESTART_AUTONEG;
          ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
          ephy_forced_pwr_down_status &= ~(1<<portnumber);
       }

       atomic_dec(&phy_write_ref_cnt);
       atomic_dec(&phy_read_ref_cnt);
       BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
   }
#endif

   up(&bcm_ethlock_switch_config);
}
#endif
EXPORT_SYMBOL(ethsw_switch_manage_port_power_mode);

int ethsw_switch_get_port_power_mode(int portnumber)
{
   return (ephy_forced_pwr_down_status & (1<<portnumber));
}
EXPORT_SYMBOL(ethsw_switch_get_port_power_mode);


int ethsw_switch_manage_ports_leds(int led_mode)
{
#define AUX_MODE_REG 0x1d
#define LNK_LED_DIS  4 // Bit4

   uint16 v16, i;

   down(&bcm_ethlock_switch_config);

   for (i=0; i<4; i++) {
      ethsw_phy_rreg(switch_port_phyid[i], AUX_MODE_REG, &v16);

      if(led_mode)
         v16 &= ~(1 << LNK_LED_DIS);
      else
         v16 |= (1 << LNK_LED_DIS);

      ethsw_phy_wreg(switch_port_phyid[i], AUX_MODE_REG, &v16);
   }

   up(&bcm_ethlock_switch_config);
   return 0;
}
EXPORT_SYMBOL(ethsw_switch_manage_ports_leds);

// end power management routines


void extsw_rreg(int page, int reg, uint8 *data, int len)
{
    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("extsw_rreg: wrong length!\n");

    switch (pVnetDev0_g->extSwitch->accessType)
    {
      case MBUS_MDIO:
        bcmsw_pmdio_rreg(page, reg, data, len);
        break;

      case MBUS_SPI:
      case MBUS_HS_SPI:
        bcmsw_spi_rreg(pVnetDev0_g->extSwitch->bus_num, pVnetDev0_g->extSwitch->spi_ss,
                       pVnetDev0_g->extSwitch->spi_cid, page, reg, data, len);
        break;
      default:
        printk("Error: Neither SPI nor PMDIO access \n");
        break;
    }
}

void extsw_wreg(int page, int reg, uint8 *data, int len)
{
    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("extsw_wreg: wrong length!\n");

    switch (pVnetDev0_g->extSwitch->accessType)
    {
      case MBUS_MDIO:
        bcmsw_pmdio_wreg(page, reg, data, len);
        break;

      case MBUS_SPI:
      case MBUS_HS_SPI:
        bcmsw_spi_wreg(pVnetDev0_g->extSwitch->bus_num, pVnetDev0_g->extSwitch->spi_ss,
                       pVnetDev0_g->extSwitch->spi_cid, page, reg, data, len);
        break;
      default:
        printk("Error: Neither SPI nor PMDIO access \n");
        break;
    }
}

void extsw_fast_age_port(uint8 port, uint8 age_static) {
    uint8 v8;
    uint8 timeout = 100;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }
    extsw_wreg(PAGE_CONTROL, REG_FAST_AGING_PORT, &port, 1);

    extsw_wreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    extsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    while (v8 & FAST_AGE_START_DONE) {
        mdelay(1);
        extsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }
}

void extsw_set_wanoe_portmap(uint16 wan_port_map)
{
    int i;
    uint8 map[2] = {0};
    //BCM_ENET_DEBUG("wanoe port map = 0x%x", wan_port_map);

    /* Set WANoE port map */
    map[0] = (uint8)wan_port_map;

    extsw_wreg(PAGE_CONTROL, REG_WAN_PORT_MAP, map, 2);

    map[0] = (uint8)wan_port_map;
    /* MIPS port */
    map[1] = 1;
    /* Disable learning */
    extsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, map, 2);

    for(i=0; i < TOTAL_SWITCH_PORTS; i++) {
       if((wan_port_map >> i) & 0x1) {
            extsw_fast_age_port(i, 1);
       }
    }
}

void ethsw_eee_link_enable(int port, int enable)
{
#if defined(GPHY_EEE_1000BASE_T_DEF)
   uint16 v16 = 0;

   /* Ensure that EEE was enabled in bootloader */
   #define EEE_BITS (GPHY_LPI_FEATURE_EN_DEF_MASK | \
      GPHY_EEE_1000BASE_T_DEF | GPHY_EEE_100BASE_TX_DEF | \
      GPHY_EEE_PCS_1000BASE_T_DEF | GPHY_EEE_PCS_100BASE_TX_DEF)

   if ( (GPIO->RoboswGphyCtrl & EEE_BITS) == EEE_BITS ) {
      if (enable) {
         v16 = REG_EEE_CTRL_ENABLE;
      }

      /* Only the GPHY port supports EEE on 63268 */
      if ((port >= NUM_INT_EPHYS) && (port < NUM_INT_PHYS)) {
         ethsw_wreg(PAGE_CONTROL, REG_EEE_CTRL + (port << 1), (uint8_t *)&v16, 2);
      }
   }

#endif
}

#ifdef CONFIG_BCM_ETH_PWRSAVE
static void ethsw_eee_all_enable(int enable)
{
#if defined(GPHY_EEE_1000BASE_T_DEF)
   struct net_device *dev = vnet_dev[0];
   BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
   int i;

   for (i=0;i<MAX_SWITCH_PORTS;i++) {
      /* Only enable/disable eee on links that are up */
      if (priv->linkState & (1 << i)) {
         if (enable) {
            ethsw_eee_link_enable(i, 1);
         } else {
            ethsw_eee_link_enable(i, 0);
         }
      }
   }

   /* Clear the global variable since we took care of enabling/disabling eee */
   priv->eee_enable_request_flag = 0;
#endif
}
#endif

void ethsw_eee_process_delayed_enable_requests(void)
{
#if defined(GPHY_EEE_1000BASE_T_DEF)
   struct net_device *dev = vnet_dev[0];
   BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
   int i;

   /* Process enable requests that have been here for more than 1 second */
   if (priv->eee_enable_request_flag & (0xFFFFFFFF << MAX_SWITCH_PORTS)) {
      for (i=0;i<MAX_SWITCH_PORTS;i++) {
         if (priv->eee_enable_request_flag & (1<<(i+MAX_SWITCH_PORTS))) {
            ethsw_eee_link_enable(i, 1);
         }
      }
   }

   /* Now delay recent requests by one polling interval (1 second) */
   priv->eee_enable_request_flag <<= MAX_SWITCH_PORTS;
#endif
}

#if defined(AEI_VDSL_HPNA)
PHY_STAT AEI_ethsw_hpna_phy_stat(int port)
{
    PHY_STAT ps;
    uint16 v16;

    ps.lnk = 0;

    if ((port >= 0) && 
#if defined(AEI_63168_CHIP)
        (switch_pport_phyid[port] == HPNA_PORT_ID))
#else
        (ext_switch_pport_phyid[port] == HPNA_PORT_ID))
#endif
    {
        ethsw_phy_rreg(HPNA_PORT_ID, 0x10, &v16);
        ps.lnk = ((v16 & 0x8000) || (v16 & 0x0200)) ? 1:0;
    }

    return ps;
}
#endif
