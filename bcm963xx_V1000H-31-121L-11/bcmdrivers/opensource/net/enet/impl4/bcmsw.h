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
#ifndef _BCMSW_H_
#define _BCMSW_H_

#include <bcm/bcmswapitypes.h>
#include <bcm/bcmswapistat.h>

/****************************************************************************
    Prototypes
****************************************************************************/
void ethsw_rreg_ext(int page, int reg, uint8 *data, int len, int is6829);
void ethsw_wreg_ext(int page, int reg, uint8 *data, int len, int is6829);
#define ethsw_wreg(page, reg, data, len)  ethsw_wreg_ext(page, reg, data, len, 0)
#define ethsw_rreg(page, reg, data, len)  ethsw_rreg_ext(page, reg, data, len, 0)

int remove_arl_entry_wrapper(void *ptr);
int  ethsw_save_port_state(void);
int  ethsw_restore_port_state(void);
void ethsw_port_based_vlan(int port_map, int wan_port_bit, int softSwitchingMap);
void ethsw_dump_page(int page);
int  ethsw_dump_mib(int port, int type);
int  reset_mib(int extswitch);
int  ethsw_counter_collect(uint32_t port_map, int discard);
int  enet_ioctl_ethsw_port_tagreplace(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_tagmangle(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_tagmangle_matchvid(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_tagstrip(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_pause_capability(struct ethswctl_data *e);
int  enet_ioctl_ethsw_control(struct ethswctl_data *e);
int  enet_ioctl_ethsw_prio_control(struct ethswctl_data *e);
int  enet_ioctl_ethsw_vlan(struct ethswctl_data *e);
int  enet_ioctl_ethsw_pbvlan(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_irc_set(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_irc_get(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_erc_set(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_erc_get(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cosq_config(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cosq_sched(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cosq_port_mapping(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cosq_rxchannel_mapping(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cosq_txchannel_mapping(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cosq_txq_sel(struct ethswctl_data *e);
int  enet_ioctl_ethsw_clear_port_stats(struct ethswctl_data *e);
int  enet_ioctl_ethsw_clear_stats(uint32_t port_map);
int  enet_ioctl_ethsw_counter_get(struct ethswctl_data *e);
int  enet_ioctl_ethsw_arl_access(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_default_tag_config(struct ethswctl_data *e);
int  enet_ioctl_ethsw_cos_priority_method_config(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_traffic_control(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_loopback(struct ethswctl_data *e, int phy_id);
int  enet_ioctl_ethsw_phy_mode(struct ethswctl_data *e, int phy_id);
int  enet_ioctl_ethsw_pkt_padding(struct ethswctl_data *e);
int  enet_ioctl_ethsw_port_jumbo_control(struct ethswctl_data *e); // bill
void fast_age_all(uint8_t age_static);
int ethsw_dscp_to_priority_mapping(struct ethswctl_data *e);
void ethsw_set_wanoe_portmap(uint16 wan_port_map);
#ifdef NO_CFE
void ethsw_reset(int is6829);
#endif
void ethsw_init_hw(int unit, uint32_t map, int wanPort, int is6829);
int ethsw_set_multiport_address(uint8_t* addr);
void ethsw_set_mac_hw(uint16_t sw_port, PHY_STAT ps, int is6829);

#ifdef REPORT_HARDWARE_STATS
#if defined(AEI_VDSL_STATS_DIAG)
int ethsw_get_hw_stats(int port, int extswitch, struct net_device_stats *stats, struct net_device * dev);
#else
int ethsw_get_hw_stats(int port, int extswitch, struct net_device_stats *stats);
#endif
#endif

int reset_switch(int is6829);

#if defined(CONFIG_BCM96816)
int ethsw_is_switch_locked(void *ptr);
#endif /* CONFIG_BCM96816 */

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
int enet_hook_for_arl_access(void *ethswctl);
#endif

void fast_age_port(uint8_t port, uint8_t age_static);

#endif /* _BCMSW_H_ */
