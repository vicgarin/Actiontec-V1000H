/*
<:copyright-gpl
 Copyright 2002-2010 Broadcom Corp. All Rights Reserved.

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
#ifndef _BCMSWACCESS_H_
#define _BCMSWACCESS_H_

void bcmsw_pmdio_rreg(int page, int reg, uint8 *data, int len);
void bcmsw_pmdio_wreg(int page, int reg, uint8 *data, int len);
void get_ext_switch_access_info(int usConfigType, int *bus_type, int *spi_id);
void bcmsw_spi_rreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len);
void bcmsw_spi_wreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len);
int enet_ioctl_ethsw_regaccess(struct ethswctl_data *e);
int enet_ioctl_ethsw_spiaccess(int bus_num, int spi_id, int chip_id, struct ethswctl_data *e);
int enet_ioctl_ethsw_pmdioaccess(struct net_device *dev, struct ethswctl_data *e);
int enet_ioctl_ethsw_info(struct net_device *dev, struct ethswctl_data *e);
int enet_ioctl_phy_cfg_get(struct net_device *dev, struct ethswctl_data *e);

#endif /* _BCMSWACCESS_H_ */

