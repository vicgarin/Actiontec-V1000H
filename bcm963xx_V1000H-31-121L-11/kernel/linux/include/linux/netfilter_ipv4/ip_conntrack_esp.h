/*****************************************************************************
//
//  Copyright (c) 2000-2002  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       ip_conntrack_esp.h
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the ESP ALG connectiontracking data structures.
//
*****************************************************************************/
#ifndef _IP_CONNTRACK_ESP_H
#define _IP_CONNTRACK_ESP_H
/* FTP tracking. */

#ifndef __KERNEL__
#error Only in kernel.
#endif

#include <linux/netfilter_ipv4/lockhelp.h>

/* Protects ftp part of conntracks */
DECLARE_LOCK_EXTERN(ip_esp_lock);

struct esphdr {
	u_int32_t spi;
	u_int32_t seq;
};

/* This structure is per expected connection */
struct ip_ct_esp_expect
{
	/* We record spi and source IP address: all in
	 * host order. */

	u_int32_t spi;	   /* Security Parameter Identifier */
	u_int32_t saddr;   /* source IP address in the orig dir */
	u_int32_t daddr;   /* remote IP address in the orig dir */
};

/* This structure exists only once per master */
struct ip_ct_esp_master {
	u_int32_t spi;
	u_int32_t saddr;
	u_int32_t daddr;
};

#endif /* _IP_CONNTRACK_ESP_H */
