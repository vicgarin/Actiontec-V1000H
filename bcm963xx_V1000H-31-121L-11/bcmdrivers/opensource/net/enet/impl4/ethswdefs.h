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
#ifndef _ETHSWDEFS_H_
#define _ETHSWDEFS_H_

/* Advertise 100BaseTxFD/HD and 10BaseTFD/HD */
#define AN_ADV_ALL                         0x1E1

/* Advertise 1000BaseTFD/HD */
#define AN_1000BASET_CTRL_ADV_ALL          0x300

#define HW_SWITCHING_DISABLED              0
#define HW_SWITCHING_ENABLED               1
static uint8_t  hw_switching_state = HW_SWITCHING_ENABLED;

static uint16_t dis_learning = 0x0180;
static uint8_t  port_fwd_ctrl = 0xC1;
static uint16_t pbvlan_map[TOTAL_SWITCH_PORTS];

#endif /* _ETHSWDEFS_H_ */
