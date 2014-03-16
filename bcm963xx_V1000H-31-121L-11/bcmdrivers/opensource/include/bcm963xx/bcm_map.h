/*
<:copyright-gpl 
 Copyright 2004 Broadcom Corp. All Rights Reserved. 
 
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

#ifndef __BCM_MAP_PART_H
#define __BCM_MAP_PART_H

#if defined(CONFIG_BCM963268)
#include <63268_map_part.h>
#endif
#if defined(CONFIG_BCM96328)
#include <6328_map_part.h>
#endif
#if defined(CONFIG_BCM96368)
#include <6368_map_part.h>
#endif
#if defined(CONFIG_BCM96816)
#include <6816_map_part.h>
#endif
#if defined(CONFIG_BCM96362)
#include <6362_map_part.h>
#endif

#endif


#if defined(CONFIG_BCM96362)
#include <6362_intr.h>
#endif

