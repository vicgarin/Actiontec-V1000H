/*
<:copyright-gpl
 Copyright 2007 Broadcom Corp. All Rights Reserved.

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

#ifndef __PKT_CMF_PUBLIC_H_INCLUDED__
#define __PKT_CMF_PUBLIC_H_INCLUDED__

#if defined(CONFIG_BCM96368) || defined(CHIP_6368)
#include "pktCmf_6368_public.h"
#endif

#if defined(CONFIG_BCM96816) || defined(CHIP_6816)
#include "pktCmf_6816_public.h"
#endif

#if defined(CONFIG_BCM_FAP_MODULE) || defined(CONFIG_BCM_FAP) || \
    defined(CONFIG_BCM_ARL_MODULE) || defined(CONFIG_BCM_ARL)

// No need for fap.h right now.  Should be fap_public.h anyways.
//#include "fap.h"
#endif

#endif  /* defined(__PKT_CMF_PUBLIC_H_INCLUDED__) */

