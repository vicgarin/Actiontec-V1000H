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
#ifndef _ETHSW_H_
#define _ETHSW_H_

/****************************************************************************
    Prototypes
****************************************************************************/

#if defined(AEI_VDSL_POWERSAVE)
#include "bcmenet.h"
#endif

int ethsw_setup_led(void);
void ethsw_setup_hw_apd(unsigned int enable);
int ethsw_phy_pll_up(int ephy_and_gphy);
uint32 ethsw_ephy_auto_power_down_wakeup(void);
uint32 ethsw_ephy_auto_power_down_sleep(void);
int ethsw_reset_ports(struct net_device *dev);
int ethsw_set_mac(int port, PHY_STAT ps);
int ethsw_phy_intr_ctrl(int port, int on);
PHY_STAT ethsw_phy_stat(int port);
void ethsw_switch_power_off(void* context);
void ethsw_init_config(void);
int ethsw_setup_phys(void);
int ethsw_add_proc_files(struct net_device *dev);
int ethsw_del_proc_files(void);
int ethsw_enable_sar_port(void);
int ethsw_disable_sar_port(void);
int ethsw_save_port_state(void);
int ethsw_restore_port_state(void);
int ethsw_port_to_phyid(int port);
void ethsw_port_based_vlan(int port_map, int wan_port_bit, int softSwitchingMap);
int ethsw_enable_hw_switching(void);
int ethsw_disable_hw_switching(void);
int ethsw_get_hw_switching_state(void);

void extsw_wreg(int page, int reg, uint8 *data, int len);
void extsw_rreg(int page, int reg, uint8 *data, int len);
void extsw_set_wanoe_portmap(uint16 wan_port_map);

void ethsw_configure_ports(int port_map, int *pphy_id);
void ethsw_set_mac_link_down(void);
void ethsw_phy_advertise_all(uint32 phy_id);
void ethsw_isolate_phy(int phyId, int isolate);
void extsw_fast_age_port(uint8 port, uint8 age_static);
void ethsw_eee_link_enable(int port, int enable);
void ethsw_eee_process_delayed_enable_requests(void);

#if defined(AEI_VDSL_HPNA)
PHY_STAT AEI_ethsw_hpna_phy_stat(int port);
#endif

#if defined(AEI_VDSL_POWERSAVE)
void AEI_ethsw_disable_pws(void);
void AEI_ethsw_set_pws_mode(enum PowerSaveMode mode, UBOOL8 enable);
#endif

#endif /* _ETHSW_H_ */
