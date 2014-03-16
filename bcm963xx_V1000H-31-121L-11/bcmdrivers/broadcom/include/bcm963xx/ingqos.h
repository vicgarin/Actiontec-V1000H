#ifndef __INGQOS_H_INCLUDED__
#define __INGQOS_H_INCLUDED__

/*
 *
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom Corporation
   All Rights Reserved

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

/*
 *******************************************************************************
 * File Name : ingqos.h
 *
 *******************************************************************************
 */
#define IQ_VERSION             "v0.1"

#define IQ_VER_STR             IQ_VERSION " " __DATE__ " " __TIME__
#define IQ_MODNAME             "Broadcom Ingress QoS Module "

/* Ingess QoS Character Device */
#define IQ_DRV_MAJOR             243


#define IQ_HASHTBL_SIZE             64
#define IQ_OVFLTBL_SIZE             64

#define CC_IQ_STATS


typedef struct {
    uint32_t loThresh;
    uint32_t hiThresh;
} thresh_t;


/*
 * CAUTION!!! 
 * It is highly recommended NOT to change the tuning parameters
 * in this file from their default values. Any change may badly affect
 * the performance of the system.
 */

/* It is recommneded to keep the low thresh > 50%        */
/* Ethernet Ingress QoS low and high thresholds as % of Ring size */
#define IQ_ENET_LO_THRESH_PCT       66
#define IQ_ENET_HI_THRESH_PCT       75

/* Ethernet Ingress QoS low and high thresholds as % of Ring size */
#if !defined(CONFIG_BCM96816)
#define IQ_XTM_LO_THRESH_PCT        66
#define IQ_XTM_HI_THRESH_PCT        75
#endif

/* CMF Fwd Ingress QoS low and high thresholds as % of Ring size */
#if defined(CONFIG_BCM96368)
#define IQ_FWD_LO_THRESH_PCT        66
#define IQ_FWD_HI_THRESH_PCT        75
#endif


#endif  /* defined(__INGQOS_H_INCLUDED__) */

