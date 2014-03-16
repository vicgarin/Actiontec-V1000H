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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcmnet.h                                                 */
/*   DATE:    05/16/02                                                 */
/*   PURPOSE: network interface ioctl definition                       */
/*                                                                     */
/***********************************************************************/
#ifndef _IF_NET_H_
#define _IF_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/sockios.h>
#include "skb_defines.h"
#include "bcmPktDma_defines.h"

#define LINKSTATE_DOWN      0
#define LINKSTATE_UP        1

#ifndef IFNAMSIZ
#define IFNAMSIZ  16
#endif

/*---------------------------------------------------------------------*/
/* Ethernet Switch Type                                                */
/*---------------------------------------------------------------------*/
#define ESW_TYPE_UNDEFINED                  0
#define ESW_TYPE_BCM5325M                   1
#define ESW_TYPE_BCM5325E                   2
#define ESW_TYPE_BCM5325F                   3
#define ESW_TYPE_BCM53101                   4

/*
 * Ioctl definitions.
 */
/* Note1: The maximum device private ioctls allowed are 16 */
/* Note2: SIOCDEVPRIVATE is reserved */
enum {
    SIOCGLINKSTATE = SIOCDEVPRIVATE + 1,
    SIOCSCLEARMIBCNTR,
    SIOCMIBINFO,
    SIOCGENABLEVLAN,
    SIOCGDISABLEVLAN,
    SIOCGQUERYNUMVLANPORTS,
    SIOCGQUERYNUMPORTS,
    SIOCPORTMIRROR,
    SIOCSWANPORT,
    SIOCGWANPORT,
    SIOCETHCTLOPS,
    SIOCGPONIF,
    SIOCETHSWCTLOPS,
    SIOCGSWITCHPORT,
#if defined(AEI_VDSL_HPNA)
    SIOCSHPNAVLANID,
    SIOCSHPNAADMSTATE, /* adminstate and mode (LAN only , LAN+WAN) */
    SIOCGHPNAADMSTATE,
    SIOCGHPNALINKSTATUS,
    SIOCSHPNALINKSTATUS,
#endif
#if defined(AEI_VDSL_SMARTLED)
    SIOCINETTRAFFICBLINK,
    SIOCINETAMBERSTATE,
#endif
#if defined(AEI_VDSL_POWERSAVE)
    SIOCPOWERSAVE,
#endif
#if defined(AEI_VDSL_STATS_DIAG)
    SIOCGETDEVSTATS,
#endif
    SIOCLAST,
};

/* Various operations through the SIOCETHCTLOPS */
enum {
    ETHGETNUMTXDMACHANNELS = 0,
    ETHSETNUMTXDMACHANNELS,
    ETHGETNUMRXDMACHANNELS,
    ETHSETNUMRXDMACHANNELS,   
    ETHGETSOFTWARESTATS,
    ETHSETSPOWERUP,
    ETHSETSPOWERDOWN,
    ETHGETMIIREG,
    ETHSETMIIREG,
};

struct ethctl_data {
    /* ethctl ioctl operation */
    int op;
    /* number of DMA channels */
    int num_channels;
    /* return value for get operations */
    int ret_val;
    /* value for set operations */
    int val;
    unsigned char phy_addr;
    unsigned char phy_reg;
    /* flags to indicate to ioctl functions */
    unsigned char flags;
};

/* Indicates the Access Requested is for External Phy */
#define ETHCTL_FLAG_ACCESS_EXT_PHY 1

/* Various operations through the SIOCGPONIF */
enum {
    GETFREEGEMIDMAP = 0,
    SETGEMIDMAP,
    GETGEMIDMAP,
    CREATEGPONVPORT,
    DELETEGPONVPORT,
    DELETEALLGPONVPORTS,
    SETMCASTGEMID,
};

struct gponif_data{
    /* gponif ioctl operation */
    int op;
    /* GEM ID map for addgem and remgem operations */
    unsigned int gem_map;
    /* ifnumber for show all operation */
    int ifnumber;
    /* interface name for create, delete, addgem, remgem, and show ops */
    char ifname[IFNAMSIZ];
};
struct interface_data{
    char ifname[IFNAMSIZ];
    int switch_port_id;
};



/* The enet driver subdivides queue field (mark[4:0]) in the skb->mark into
   priority and channel */
/* priority = queue[2:0] (=>mark[2:0]) */
#define SKBMARK_Q_PRIO_S        (SKBMARK_Q_S)
#define SKBMARK_Q_PRIO_M        (0x07 << SKBMARK_Q_PRIO_S)
#define SKBMARK_GET_Q_PRIO(MARK) \
    ((MARK & SKBMARK_Q_PRIO_M) >> SKBMARK_Q_PRIO_S)
#define SKBMARK_SET_Q_PRIO(MARK, Q) \
    ((MARK & ~SKBMARK_Q_PRIO_M) | (Q << SKBMARK_Q_PRIO_S))
/* channel = queue[4:3] (=>mark[4:3]) */
#define SKBMARK_Q_CH_S          (SKBMARK_Q_S + 3)
#define SKBMARK_Q_CH_M          (0x03 << SKBMARK_Q_CH_S)
#define SKBMARK_GET_Q_CHANNEL(MARK) ((MARK & SKBMARK_Q_CH_M) >> SKBMARK_Q_CH_S)
#define SKBMARK_SET_Q_CHANNEL(MARK, CH) \
    ((MARK & ~SKBMARK_Q_CH_M) | (CH << SKBMARK_Q_CH_S))

#define SPEED_10MBIT        10000000
#define SPEED_100MBIT       100000000
#define SPEED_1000MBIT      1000000000

typedef struct IoctlMibInfo
{
    unsigned long ulIfLastChange;
    unsigned long ulIfSpeed;
    unsigned long ulIfDuplex;
} IOCTL_MIB_INFO, *PIOCTL_MIB_INFO;


#define MIRROR_INTF_SIZE    32
#define MIRROR_DIR_IN       0
#define MIRROR_DIR_OUT      1
#define MIRROR_DISABLED     0
#define MIRROR_ENABLED      1

typedef struct _MirrorCfg
{
    char szMonitorInterface[MIRROR_INTF_SIZE];
    char szMirrorInterface[MIRROR_INTF_SIZE];
    int nDirection;
    int nStatus;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    int vlanId;
#endif
#if defined(CONFIG_BCM96816) || defined(DMP_X_ITU_ORG_GPON_1)
    unsigned int nGemPortMask;
#endif
} MirrorCfg ;

#if defined(AEI_VDSL_POWERSAVE)
enum PowerSaveMode
{
    PWS_INVALID_MODE = -1,
    PWS_AUTO_POWER_DOWN = 0,
    PWS_ENERGY_EFFICIENT_ETHERNET,
    PWS_SHORT_CABLE,
    PWS_DEEP_GREEN
};

typedef struct
{
    int enable;
    enum PowerSaveMode mode;
} PowerSaveCfg;
#endif

#ifdef __cplusplus
}
#endif

#endif /* _IF_NET_H_ */
