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
//  Filename:       ip_conntrack_ipsec.h
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the IPSec ALG connectiontracking data structures.
//
*****************************************************************************/
#ifndef _IP_CONNTRACK_IPSEC_H
#define _IP_CONNTRACK_IPSEC_H
/* FTP tracking. */

#ifndef __KERNEL__
#error Only in kernel.
#endif

#include <linux/netfilter_ipv4/lockhelp.h>

#define IPSEC_UDP_PORT 500

/* Protects ftp part of conntracks */
DECLARE_LOCK_EXTERN(ip_ipsec_lock);

struct isakmphdr {
	u_int32_t initcookie[2];
	u_int32_t respcookie[2];
};

/* This structure is per expected connection */
struct ip_ct_ipsec_expect
{
	/* We record initiator cookie and source IP address: all in
	 * host order. */

 	/* source cookie */
	u_int32_t initcookie[2];	/* initiator cookie */
	u_int32_t respcookie[2];	/* initiator cookie */
	u_int32_t saddr; 		/* source IP address in the orig dir */
};

/* This structure exists only once per master */
struct ip_ct_ipsec_master {
	u_int32_t initcookie[2];
	u_int32_t saddr;
};

#endif /* _IP_CONNTRACK_IPSEC_H */
