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
#ifndef ETHSW_PHY_H
#define ETHSW_PHY_H

#include "bcm_OS_Deps.h"
#include "bcmtypes.h"
#include "bcmmii.h"
#include "ethsw.h"

#define ETHSW_PHY_GET_PHYID(port) switch_pport_phyid[port]

void ethsw_phy_rreg(int phy_id, int reg, uint16 *data);
void ethsw_phy_wreg(int phy_id, int reg, uint16 *data);

void ethsw_phy_read_reg(int phy_id, int reg, uint16 *data, int ext_bit);
void ethsw_phy_write_reg(int phy_id, int reg, uint16 *data, int ext_bit);

#endif /*ETHSW_PHY_H*/

