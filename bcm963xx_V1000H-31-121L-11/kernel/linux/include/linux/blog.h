#ifndef __BLOG_H_INCLUDED__
#define __BLOG_H_INCLUDED__

                /*--------------------------------*/
                /* Blog.h and Blog.c for Linux OS */
                /*--------------------------------*/

/* 
* <:copyright-BRCM:2003:DUAL/GPL:standard
* 
*    Copyright (c) 2003 Broadcom Corporation
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
:>
*/

/*
 *******************************************************************************
 *
 * File Name  : blog.h
 *
 * Description:
 *
 * A Blog is an extension of the native OS network stack's packet context.
 * In Linux a Blog would be an extansion of the Linux socket buffer (aka skbuff)
 * or a network device driver level packet context FkBuff. The nbuff layer
 * provides a transparent access SHIM to the underlying packet context, may it
 * be a skbuff or a fkbuff. In a BSD network stack, a packet context is the
 * BSD memory buffer (aka mbuff).
 *
 * Blog layer provides Blog clients a SHIM to the native OS network stack:
 * Blog clients may be impleted to:
 *  - debug trace a packet as it passes through the network stack,
 *  - develop traffic generators (loop) at the network device driver level.
 *  - develop network driver level promiscuous mode bound applications and use
 *    the Blog SHIM to isolate themselves from the native OS network constructs
 *    or proprietery network constructs such as Ethernet bridges, VLAN network
 *    interfaces, IGMP, firewall and connection tracking systems.
 *
 * As such, Blog provides an extension of the packet context and contains the
 * received and transmitted packets data and parsed information. Parsing results
 * are saved to describe, the type of layer 1, 2, 3 and 4 headers seen, whether
 * the packet was a unicast, broadcast or multicast, a tunnel 4in6 or 6in4 etc.
 *
 * Blog views a receive or transmit end-point to be any construct that can
 * described by a end point context and a handler op. An end-point could hence
 * be a:
 *  - a network device (Linux net_device with hard start transmit handler),
 *  - a link or queue in the network stack (e.g. a Linux Traffic Control queue
 *    or a netlink or raw socket queue),
 *  - a file system logging interface and its logging handler,
 *  - a virtual interface to some hardware block that provides some hardware
 *    assisted functionality (e.g. IPSEC acceleration or checksum offloading
 *    or GSO block),
 *  - a raw interface to an external hardware test traffic generator using say
 *    a DMA mapped packet reception or transmission.
 *
 * Blog clients are hence applications that provide value added capability by
 * binding at such end-points.
 *
 * A simple Blog client application is a loop traffic generator that simply
 * acts as a sink of packets belonging to a specific "l3 flow" and mirrors
 * them to another interface or loops them back into the stack by serving as a
 * source to a receive network device, while measuring the packet processing
 * datapath performance in the native OS network stack/proprietary constructs.
 * Such a loop traffic generator could be used to inject N cells/packets
 * that cycle through the system endlessly, serving as background traffic while
 * a few flows are studied from say a QOS perspective.
 *
 * Another example of a Blog client is a proxy accelerator (hardware / software)
 * that is capable of snooping on specific flows and accelerating them while
 * bypassing the native OS network stack and/or proprietery constructs. It is
 * however required that the native OS constructs can co-exist. E.g. it may be
 * necessary to refresh a network bridge's ARL table, or a connection/session
 * tracker, or update statistics, when individual packets bypass such network
 * constructs. A proxy accelerator may also reside between a Rx network device
 * a hardware IPSEC accelerator block and a Tx network device.
 *
 * Blog layer provides a logical composite SHIM to the network constructs
 * Linux or proprietery, allowing 3rd party network constructs to be seemlesly
 * supported in the native OS.  E.g a network stack that uses a proprietery
 * session tracker with firewalling capability would need to be transparently
 * accessed, so that a Blog client may refresh the session tracking object when
 * packets bypass the network stack.
 *
 * For each OS (eCOS, Linux, BSD) a blog.c implementation file is provided that
 * implements the OS specific SHIM. Support for 3rd-party network constructs
 * would need to be defined in the blog.c . E.g. for Linux, if a proprietery
 * session tracker replaces the Linux netfilter connection tracking framework,
 * then the void * ct_p and the corresponding query/set operations would need to
 * be implemented. The Blog clients SHOULD NOT rely on any function other than
 * those specifically defined allowing a coexistence of the Blog client and the
 * native construct. In the example of a ct_p, for all practice and purposes,
 * the void *, could have been a key or a handle to a connection tracking object
 *
 * Likewise, the Blog client may save need to save a client key with the
 * network constuct. Again a client key may be a pointer to a client object or
 * simply a hash key or some handle semantics.
 *
 * The logical SHIM is defined as follows:
 *
 * __doc_include_if_linux__
 *
 * 1. Extension of a packet context with a logging context:
 * ========================================================
 *   Explicit APIS to allocate/Free a Blog structure, and bind to the packet
 *   context, may it be a skbuff or a fkbuff. Support for transferring a
 *   Blog_t structure from one packet context to another during the course of
 *   a packet in the network stack involving a packet context clone/copy is
 *   also included. The release and recycling of Blog_t structures when a 
 *   packet context is freed are also providied.
 *   Binding is bi-directional: packet context <-- --> Blog_t
 * 
 *
 * 2. Associating native OS or 3rd-party network constructs: blog_link()
 * ==========================================================================
 *   Examples of network constructs
 *      "dev"   - Network device 
 *      "ct"    - Connection or session tracker
 *      "fdb"   - Network bridge forwarding database entity
 *
 *   Association is pseudo bi-directional, using "void *" binding in a Blog_t to
 *   a network construct. In the reverse, a network construct will link to a
 *   Blog client entity using a Key concept. Two types of keys are currently
 *   employed, a BlogFlowKey and a BlogGroupKey. 
 *
 *   A BlogFlowKey would typically refer to a single unidirectional packet
 *   stream defined by say all packets belonging to a unidirectional IPv4 flow,
 *   whereas a BlogGroupKey could be used to represent a single downstream
 *   multicast stream (IP multicast group) that results in replicated streams
 *   pertaining to multiple clients joining a the IPv4 multicast group.
 *
 *   Likewise, one may represent a single unidirectional IPv4 UDP flow using
 *   BlogFlowKey, and the reverse direction IPv4 UDP reply flow
 *   using another BlogFlowKey, and represent the mated pair using a
 *   BlogGroupKey.
 *
 *   In a Blog traffic generator client, where in several IPv4 UDP flows, each
 *   represented independently using a BlogFlowKey, allows for a set of them
 *   (background downstream stress traffic) to be managed as a group using a
 *   BlogGroupKey.
 *
 *   Designer Note:
 *   A network construct may be required to save a BlogFlowKey and/or
 *   BlogGroupKey to complete the reverse binding between a network construct
 *   and the Blog client application. An alternate approach would be to save
 *   a pointer to the Blog_t in the network construct with an additional
 *   dereference through the keys saved within the Blog_t object.
 *
 *   A BlogFlowKey and a BlogGroupKey is a 32bt sized unit and can serve either
 *   as a pointer (32bit processor) or a index or a hash key or ...
 *
 *
 * 3. Network construct and Blog client co-existence call backs:
 * =============================================================
 *
 * blog_notify():
 * ==============
 * A network construct may notify a Blog client of a change of status and may
 * be viewed as a "downcall" from specialized network construct to a Blog client
 * E.g. if a connection/session tracking system deems that a flow needs to be
 * deleted or say it itself is being destroyed, then it needs to notify the Blog
 * client. This would allow the Blog client to cleanup any association with the
 * network construct.
 * Ability for a Blog client to receive general system wide notifications of
 * changes, to include, network interfaces or link state changes, protocol stack
 * service access point changes, etc.
 * Designer Note: Linux notification list?
 *
 * blog_request():
 * ===============
 * A Blog client may request a change in state in the network construct and may
 * be viewed as a "upcall" from the Blog client into the network construct. A
 * timer refresh of the bridge fdb or connection tracking object, or a query
 * whether the session tracker has successfully established (e.g. a TCP 3-way
 * handshake has completed, or a IGMP client was permitted to join a group, or a
 * RTSP session was successful) a uni-driectional or bi-directional flow.
 *
 *
 * 4. Network end-point binding of Blog client
 * ===========================================
 *
 * blog_init(), blog_sinit(), blog_finit():
 * ========================================
 * __comment_if_linux__ : This function is invoked by a Linux network device on
 * packet reception to pass the packet to a Blog client application.
 *
 * Pass a packet context to a Blog client at a "RX" network device either using
 * a skbuff or a fkbuff packet context. Blog client MAY ONLY ACCESS fkbuff
 * fields. As per the nbuff specification, a FkBuff may be considered as a
 * base class and a skbuff is a derived class, inheriting the base class members
 * of the base class, fkbuff. The basic fields of a packet context are a pointer
 * to the received packet's data, data length, a set of reserved fields to carry
 * layer 1 information, queue priority, etc, and packet context and or packet
 * recycling. The layer 1 information is described in terms of channels and
 * and link layer phy preambles. A channel could be an ATM VCI, a DSL queue, a
 * PON Gem Port. A Phy could describe the LINK layer type and or a preamble for
 * instance a RFC2684 header in the DSL world.
 *
 * blog_[s|f]init() will setup the L1 coarse key<channel,phy> and invokes a Blog
 * client's receive hook. A Blog client may consume the packet bypassing the
 * native OS network stack, may suggest that the packet context be extended by
 * a Blog_t structure or may deem that the packet is of not interest. As such
 * the Blog client will return PKT_DONE, PKT_BLOG or PKT_NORM, respectively. In
 * case no Blog client has been registered for receiving packets (promiscuous)
 * driectly from RX network devices, then the packet will follow a normal data
 * path within the network stack (PKT_NORM).
 *
 * Designer Note: Blog clients MAY NOT use fields not defined in FkBuff.
 * 
 *
 * blog_emit():
 * ============
 * __comment_if_linux__ : This function is invoked by a Linux network device
 * prior to packet transmission to pass the packet to a Blog client application.
 *
 * Pass a packet context to a Blog client at a "TX" network device either using
 * a skbuff or a fkbuff packet context. The same restrictions on a Blog client
 * pertaining to packet field context access as defined in the blog_init()
 * variant of APIs is applicable to blog_emit(). A Blog client may also return
 * PKT_NORM or PKT_DONE, to indicate normal processing, or packet consumption.
 *
 * Designer Note: blog_emit() will ONLY pass those packets to Blog clients that
 * have a packet context extended with a Blog_t structure. Hence skbuffs or
 * fkbuffs that do not have a Blog_t extension will not be handed to the Blog
 * client. Do we need blog_semit/blog_femit variants.
 *
 *
 * 5. Binding Blog client applications: blog_bind()
 * ================================================
 * blog_bind() enables a "single" client to bind into the network stack by
 * specifying a network device packet reception handler, a network device packet
 * transmission handler, network stack to blog client notify hook.
 *
 *
 * 6. Miscellanous
 * ===============
 * - Blog_t management.
 * - Data-filling a Blog_t.
 * - Protocol Header specifications independent of OS.
 * - Debug printing.
 *
 *
 * __end_include_if_linux__
 *
 *  Version 1.0 SKB based blogging
 *  Version 2.0 NBuff/FKB based blogging (mbuf)
 *  Version 2.1 IPv6 Support
 *  Version 3.0 Restructuring Blog SHIM to support eCOS, Linux and proprietery
 *              network constructs
 *
 *******************************************************************************
 */

#define BLOG_VERSION            "v3.0"

#if defined(__KERNEL__)                 /* Kernel space compilation           */
#include <linux/types.h>                /* LINUX ISO C99 7.18 Integer types   */
#else                                   /* User space compilation             */
#include <stdint.h>                     /* C-Lib ISO C99 7.18 Integer types   */
#endif
#include <linux/blog_net.h>             /* IEEE and RFC standard definitions  */
#include <linux/nbuff_types.h>          /* for IS_SKBUFF_PTR                  */

#ifndef NULL_STMT
#define NULL_STMT                   do { /* NULL BODY */ } while (0)
#endif

#undef  BLOG_DECL
#define BLOG_DECL(x)                x,

/* Forward declarations */
struct blog_t;
typedef struct blog_t Blog_t;
#define BLOG_NULL                   ((Blog_t*)NULL)
#define BLOG_KEY_NONE               0

/* __bgn_include_if_linux__ */

struct sk_buff;                         /* linux/skbuff.h                     */
struct fkbuff;                          /* linux/nbuff.h                      */

/* See RFC 4008 */
extern uint32_t blog_nat_tcp_def_idle_timeout;
extern uint32_t blog_nat_udp_def_idle_timeout;

/*
 * Linux Netfilter Conntrack registers it's conntrack refresh function which
 * will be invoked to refresh a conntrack when packets belonging to a flow
 * managed by Linux conntrack are bypassed by a Blog client.
 */
typedef void (*blog_refresh_t)(void * ct_p, uint32_t ctinfo,
                               struct sk_buff * skb_p,
                               uint32_t jiffies, int do_acct);
extern blog_refresh_t blog_refresh_fn;
/* __end_include_if_linux__ */


/*
 *------------------------------------------------------------------------------
 * Denotes whether a packet is consumed and freed by a Blog client application,
 * whether a packet needs to be processed normally within the network stack or
 * whether a packet context is extended with a Blog_t object.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(PKT_DONE)             /* Packet consumed and freed          */
        BLOG_DECL(PKT_NORM)             /* Continue normal stack processing   */
        BLOG_DECL(PKT_BLOG)             /* Continue stack with blogging       */
        BLOG_DECL(PKT_DROP)             /* Drop Packet                        */
        BLOG_DECL(BLOG_ACTION_MAX)
} BlogAction_t;

/*
 *------------------------------------------------------------------------------
 * Denotes the direction in the network stack when a packet is processed by a
 * virtual network interface/network device.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(DIR_RX)               /* Receive path in network stack      */
        BLOG_DECL(DIR_TX)               /* Transmit path in network stack     */
        BLOG_DECL(BLOG_DIR_MAX)
} BlogDir_t;

/*
 *------------------------------------------------------------------------------
 * Denotes the type of Network entity associated with a Blog_t.
 *
 * BlogNetEntity_t may be linked to a blog using blog_link to make the Blog_t
 * point to the BlogNetEntity_t. A reverse linking from the BlogNetEntity_t to
 * Blog_t is only possible via a key (if necessary when a one to one association
 * between the BlogNetEntity_t and a Blog exists. For instance, there is a
 * one to one association between a Flow Connection Tracker and a Blog. In fact
 * a Linux Netfilter Connection Tracking object manages a bi-directional flow
 * and thus may have 2 keys to reference the corresponding Blog_t. However, a
 * network device (physical end device or a virtual device) may have multiple
 * Flows passing through it and hence no one-to-one association exists. In this
 * can a Blog may have a link to a network device, but the reverse link (via a
 * key) is not saved in the network device.
 *
 * Linking a BlogNetEntity_t to a blog is done via blog_link() whereas saving
 * a reference key into a BlogNetEntity_t is done via blog_request() by the
 * Blog client application, if needed.
 *
 *------------------------------------------------------------------------------
 */

/* FLOWTRACK: param1 is ORIG=0 or REPLY=1 direction */
#define BLOG_PARAM1_DIR_ORIG    0U
#define BLOG_PARAM1_DIR_REPLY   1U

/* FLOWTRACK: param2 is IPv4=0 or IPv6=1 */
#define BLOG_PARAM2_IPV4        0U
#define BLOG_PARAM2_IPV6        1U
#define BLOG_PARAM2_MAX         2U

/* BRIDGEFDB: param1 is src|dst */
#define BLOG_PARAM1_SRCFDB      0U
#define BLOG_PARAM1_DSTFDB      1U

/* IF_DEVICE: param1 is direction RX or TX, param 2 is minMtu */

typedef enum {
        BLOG_DECL(FLOWTRACK)            /* Flow (connection|session) tracker  */
        BLOG_DECL(BRIDGEFDB)            /* Bridge Forwarding Database entity  */
        BLOG_DECL(MCAST_FDB)            /* Multicast Client FDB entity        */
        BLOG_DECL(IF_DEVICE)            /* Virtual Interface (network device) */
        BLOG_DECL(BLOG_NET_ENTITY_MAX)
} BlogNetEntity_t;

/*
 *------------------------------------------------------------------------------
 * Denotes a type of notification sent from the network stack to the Blog client
 * See blog_notify(BlogNotify_t, void *, uint32_t param1, uint32_t param2);
 *------------------------------------------------------------------------------
 */

/* MCAST_CONTROL_EVT: param1 is add|del, and param2 is IPv4|IPv6 */
#define BLOG_PARAM1_MCAST_ADD       0U
#define BLOG_PARAM1_MCAST_DEL       1U
#define BLOG_PARAM2_MCAST_IPV4      0U
#define BLOG_PARAM2_MCAST_IPV6      1U

/* LINK_STATE_CHANGE: param1 */
#define BLOG_PARAM1_LINK_STATE_UP   0U
#define BLOG_PARAM1_LINK_STATE_DOWN 1U

/* FETCH_NETIF_STATS: param1 is address of BlogStats_t, param2 */
#define BLOG_PARAM2_NO_CLEAR        0U
#define BLOG_PARAM2_DO_CLEAR        1U

typedef enum {
        BLOG_DECL(DESTROY_FLOWTRACK)    /* Session/connection is deleted      */
        BLOG_DECL(DESTROY_BRIDGEFDB)    /* Bridge FDB has aged                */
        BLOG_DECL(MCAST_CONTROL_EVT)    /* Mcast client joins a group event   */
        BLOG_DECL(MCAST_SYNC_EVT)       /* Topology change for mcast event    */
        BLOG_DECL(DESTROY_NETDEVICE)    /* Network device going down          */
        BLOG_DECL(LINK_STATE_CHANGE)    /* Physical network link event        */
        BLOG_DECL(FETCH_NETIF_STATS)    /* Fetch accumulated stats            */
        BLOG_DECL(DYNAMIC_DSCP_EVENT)   /* Dynamic DSCP change event          */
        BLOG_DECL(UPDATE_NETDEVICE)     /* Netdevice has been modified (MTU, etc) */
        BLOG_DECL(BLOG_NOTIFY_MAX)
} BlogNotify_t;


/*
 *------------------------------------------------------------------------------
 * Denotes a type of request from a Blog client to a network stack entity.
 *------------------------------------------------------------------------------
 */

typedef enum {
        BLOG_DECL(FLOWTRACK_KEY_SET)    /* Set Client key into Flowtracker    */
        BLOG_DECL(FLOWTRACK_KEY_GET)    /* Get Client key into Flowtracker    */
        BLOG_DECL(FLOWTRACK_DSCP_GET)   /* Get DSCP from Flow tracker:DYNDSCP */
        BLOG_DECL(FLOWTRACK_CONFIRMED)  /* Test whether session is confirmed  */
        BLOG_DECL(FLOWTRACK_ASSURED)    /* Test whether session is assured    */
        BLOG_DECL(FLOWTRACK_ALG_HELPER) /* Test whether flow has an ALG       */
        BLOG_DECL(FLOWTRACK_EXCLUDE)    /* Clear flow candidacy by Client     */
        BLOG_DECL(FLOWTRACK_REFRESH)    /* Refresh a flow tracker             */
        BLOG_DECL(BRIDGE_REFRESH)       /* Refresh a Bridge FDB entry         */
        BLOG_DECL(NETIF_PUT_STATS)      /* Push accumulated stats to devices  */
        BLOG_DECL(LINK_XMIT_FN)         /* Fetch device link transmit function*/
        BLOG_DECL(LINK_NOCARRIER)       /* Fetch device link carrier          */
        BLOG_DECL(NETDEV_NAME)          /* Network device name                */
        BLOG_DECL(MCAST_KEY_SET)        /* Set Client key into IGMP/MLD       */
        BLOG_DECL(MCAST_KEY_GET)        /* Get Client key from IGMP/MLD       */
        BLOG_DECL(IQPRIO_SKBMARK_SET)   /* Set IQOS Prio in skb->mark */
        BLOG_DECL(MCAST_DFLT_MIPS)      /* Delete action in blogRule chain    */
        BLOG_DECL(BLOG_REQUEST_MAX)
} BlogRequest_t;


/*----- LinkType: First header type ------------------------------------------*/
/* Used by network drivers to determine the Layer 1 encapsulation or LinkType */
typedef enum {
        BLOG_DECL(TYPE_ETH)             /* LAN: ETH, WAN: EoA, MER, PPPoE     */
        BLOG_DECL(TYPE_PPP)             /*           WAN: PPPoA               */
        BLOG_DECL(TYPE_IP)              /*           WAN: IPoA                */
} BlogLinkType_t;


/*
 *------------------------------------------------------------------------------
 * Clean this up.
 *------------------------------------------------------------------------------
 */

#define BLOG_ENCAP_MAX          6       /* Maximum number of L2 encaps        */
#define BLOG_HDRSZ_MAX          32      /* Maximum size of L2 encaps          */

typedef enum {
        BLOG_DECL(BCM_XPHY)             /* e.g. BLOG_XTMPHY, BLOG_GPONPHY     */
        BLOG_DECL(BCM_SWC)              /* BRCM LAN Switch Tag/Header         */
        BLOG_DECL(ETH_802x)             /* Ethernet                           */
        BLOG_DECL(VLAN_8021Q)           /* Vlan 8021Q (incld stacked)         */
        BLOG_DECL(PPPoE_2516)           /* PPPoE RFC 2516                     */
        BLOG_DECL(PPP_1661)             /* PPP RFC 1661                       */
        BLOG_DECL(L3_IPv4)              /* L3 IPv4                            */
        BLOG_DECL(L3_IPv6)              /* L3 IPv6                            */
        BLOG_DECL(PROTO_MAX)
} BlogEncap_t;



/*
 *------------------------------------------------------------------------------
 * RFC 2684 header logging.
 * CAUTION: 0'th enum corresponds to either header was stripped or zero length
 *          header. VC_MUX_PPPOA and VC_MUX_IPOA have 0 length RFC2684 header.
 *          PTM does not have an rfc2684 header.
 *------------------------------------------------------------------------------
 */
typedef enum {
        BLOG_DECL(RFC2684_NONE)         /*                               */
        BLOG_DECL(LLC_SNAP_ETHERNET)    /* AA AA 03 00 80 C2 00 07 00 00 */
        BLOG_DECL(LLC_SNAP_ROUTE_IP)    /* AA AA 03 00 00 00 08 00       */
        BLOG_DECL(LLC_ENCAPS_PPP)       /* FE FE 03 CF                   */
        BLOG_DECL(VC_MUX_ETHERNET)      /* 00 00                         */
        BLOG_DECL(VC_MUX_IPOA)          /*                               */
        BLOG_DECL(VC_MUX_PPPOA)         /*                               */
        BLOG_DECL(PTM)                  /*                               */
        BLOG_DECL(RFC2684_MAX)
} Rfc2684_t;


/*
 *------------------------------------------------------------------------------
 * Denotes the type of physical interface and the presence of a preamble.
 *------------------------------------------------------------------------------
 */
typedef enum {
    BLOG_DECL(BLOG_XTMPHY)
    BLOG_DECL(BLOG_ENETPHY)
    BLOG_DECL(BLOG_GPONPHY)
    BLOG_DECL(BLOG_USBPHY)
    BLOG_DECL(BLOG_WLANPHY)
    BLOG_DECL(BLOG_MOCAPHY)
    BLOG_DECL(BLOG_MAXPHY)
} BlogPhy_t;



/*
 *------------------------------------------------------------------------------
 * Logging of a maximum 4 "virtual" network devices that a flow can traverse.
 * Virtual devices are interfaces that do not perform the actual DMA transfer.
 * E.g. an ATM interface would be referred to as a physical interface whereas
 * a ppp interface would be referred to as a Virtual interface.
 *------------------------------------------------------------------------------
 */
#define MAX_VIRT_DEV           7

#define DEV_DIR_MASK           0x3u
#define DEV_PTR_MASK           (~DEV_DIR_MASK)
#define DEV_DIR(ptr)           ((uint32_t)(ptr) & DEV_DIR_MASK)

#define IS_RX_DIR(ptr)         ( DEV_DIR(ptr) == DIR_RX )
#define IS_TX_DIR(ptr)         ( DEV_DIR(ptr) == DIR_TX )

/*
 *------------------------------------------------------------------------------
 * Device pointer conversion between with and without embeded direction info
 *------------------------------------------------------------------------------
 */
#define DEVP_APPEND_DIR(ptr,dir) ((void *)((uint32_t)(ptr) | (uint32_t)(dir)))
#define DEVP_DETACH_DIR(ptr)     ((void *)((uint32_t)(ptr) & (uint32_t) \
                                                              DEV_PTR_MASK))

/*
 *------------------------------------------------------------------------------
 * Blog statistics structure
 *------------------------------------------------------------------------------
 */
typedef struct{
    unsigned long	rx_packets;		        /* total blog packets received	  */
    unsigned long	tx_packets;		        /* total blog packets transmitted */
    unsigned long	rx_bytes;		        /* total blog bytes received 	  */
    unsigned long	tx_bytes;		        /* total blog bytes transmitted	  */
    unsigned long	multicast;		        /* total blog multicast packets	  */
} BlogStats_t;


/*
 * -----------------------------------------------------------------------------
 * Support blogging of multicast packets.
 *
 * When Multicast support is enabled system wide, the default to be used may
 * be set in CC_BLOG_SUPPORT_MCAST which gets saved in blog_support_mcast_g.
 * One may change the default (at runtime) by invoking blog_support_mcast().
 * -----------------------------------------------------------------------------
 */

/* Multicast Support for IPv4 and IPv6 Control */
#define BLOG_MCAST_DISABLE          0
#define BLOG_MCAST_IPV4             1
#define BLOG_MCAST_IPV6             2

#ifdef CONFIG_BLOG_MCAST
#define CC_BLOG_SUPPORT_MCAST        BLOG_MCAST_IPV4 + BLOG_MCAST_IPV6
#else
#define CC_BLOG_SUPPORT_MCAST        BLOG_MCAST_DISABLE
#endif

extern int blog_support_mcast_g;
extern void blog_support_mcast(int enable);

/*
 * -----------------------------------------------------------------------------
 * Support blogging of IPv6 traffic
 *
 * When IPv6 support is enabled system wide, the default to be used may
 * be set in CC_BLOG_SUPPORT_IPV6 which gets saved in blog_support_ipv6_g.
 * One may change the default (at runtime) by invoking blog_support_ipv6().
 * -----------------------------------------------------------------------------
 */

/* IPv6 Support Control: see blog_support_ipv6_g and blog_support_ipv6() */
#define BLOG_IPV6_DISABLE           0
#define BLOG_IPV6_ENABLE            1

#ifdef CONFIG_BLOG_IPV6
#define CC_BLOG_SUPPORT_IPV6        BLOG_IPV6_ENABLE
#else
#define CC_BLOG_SUPPORT_IPV6        BLOG_IPV6_DISABLE
#endif

extern int blog_support_ipv6_g;
extern void blog_support_ipv6(int enable);


/* Traffic type */
typedef enum {
    BLOG_DECL(BlogTraffic_IPV4_UCAST)
    BLOG_DECL(BlogTraffic_IPV6_UCAST)
    BLOG_DECL(BlogTraffic_IPV4_MCAST)
    BLOG_DECL(BlogTraffic_IPV6_MCAST)
    BLOG_DECL(BlogTraffic_MAX)
} BlogTraffic_t;


#define BLOG_KEY_INVALID             0xFFFFFFFF
typedef union {
    uint32_t    u32;
    struct {
        uint16_t flowkey;
        uint16_t blogkey;
    };
} BlogActivateKey_t;

#define BLOG_SET_PHYHDR(a, b)   ( (((a) & 0xf) << 4) | ((b) & 0xf) )
#define BLOG_GET_PHYTYPE(a)     ( (a) & 0xf )
#define BLOG_GET_PHYLEN(a)      ( (a) >> 4 )

#define BLOG_PHYHDR_MASK        0xff
#define BLOG_SET_HW_ACT(a)      ( ((a) & 0xf) << 8 )
#define BLOG_GET_HW_ACT(a)      ( (a) >> 8 )

/*
 * =============================================================================
 * CAUTION: OS and network stack may be built without CONFIG_BLOG defined.
 * =============================================================================
 */

#if defined(CONFIG_BLOG)

/*
 *------------------------------------------------------------------------------
 *
 *              Section: Blog Conditional Compiles CC_BLOG_SUPPORT_...
 *
 * These conditional compiles are not controlled by a system wide build process.
 * E.g. CONFIG_BLOG_MCAST is a system wide build configuration
 *      CC_BLOG_SUPPORT_MCAST is a blog defined build configuration
 *
 * Do not use any CONFIG_ or CC_BLOG_SUPPORT_ in Blog_t structure definitions.
 *
 *------------------------------------------------------------------------------
 */

/* LAB ONLY: Design development, uncomment to enable */
/* #define CC_BLOG_SUPPORT_COLOR */
/* #define CC_BLOG_SUPPORT_DEBUG */


/*
 * -----------------------------------------------------------------------------
 * Engineering constants: Pre-allocated pool size 400 blogs Ucast+Mcast
 *
 * Extensions done in #blogs carved from a 2x4K page (external fragmentation)
 * Blog size = 240, 8192/240 = 34 extension 32bytes internal fragmentation
 *
 * Number of extensions engineered to permit approximately another 400 blogs.
 * -----------------------------------------------------------------------------
 */
#define CC_BLOG_SUPPORT_EXTEND              /* Conditional compile            */
#define BLOG_POOL_SIZE_ENGG         400     /* Pre-allocated pool size        */

#define BLOG_EXTEND_SIZE_ENGG       34      /* Number of Blog_t per extension */
#define BLOG_EXTEND_MAX_ENGG        125     /* Maximum extensions allowed     
                                               including 4K flows             */


/* To enable user filtering, see blog_filter(), invoked in blog_finit() */
/* #define CC_BLOG_SUPPORT_USER_FILTER */



/*
 * -----------------------------------------------------------------------------
 *                      Section: Definition of a Blog_t
 * -----------------------------------------------------------------------------
 */
#define BLOG_CHAN_INVALID   0xFF

typedef struct {
    uint8_t             channel;        /* e.g. port number, txchannel, ... */

    union {
        struct {
            uint8_t         phyHdrLen   : 4;
            uint8_t         phyHdrType  : 4;
        };
        uint8_t         phyHdr;
    };

    struct {
        uint8_t             vlan_8021ad :1;     /* 8021AD stacked */
        uint8_t             wan_qdisc   :1;     /* device type */
        uint8_t             multicast   :1;     /* multicast flag */
        uint8_t             fkbInSkb    :1;     /* fkb from skb */
        uint8_t             count       :4;     /* # of L2 encapsulations */
    };

    union {
        struct {
            uint8_t         L3_IPv6     : 1;
            uint8_t         L3_IPv4     : 1;
            uint8_t         PPP_1661    : 1;
            uint8_t         PPPoE_2516  : 1;
            uint8_t         VLAN_8021Q  : 1;    
            uint8_t         ETH_802x    : 1;
            uint8_t         BCM_SWC     : 1;
            uint8_t         BCM_XPHY    : 1;    /* e.g. BCM_XTM */
        }               bmap;/* as per order of BlogEncap_t enums declaration */
        uint8_t         hdrs;
    };
} BlogInfo_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log IP Tuple.
 * Packed: 1 16byte cacheline.
 *------------------------------------------------------------------------------
 */
struct blogTuple_t {
    uint32_t        saddr;          /* IP header saddr */
    uint32_t        daddr;          /* IP header daddr */

    union {
        struct {
            uint16_t    source;     /* L4 source port */
            uint16_t    dest;       /* L4 dest port */
        }           port;
        uint32_t    ports;
    };

    uint8_t         ttl;            /* IP header ttl */
    uint8_t         tos;            /* IP header tos */
    uint16_t        check;          /* checksum: rx tuple=l3, tx tuple=l4 */

} ____cacheline_aligned;
typedef struct blogTuple_t BlogTuple_t;

#define NEXTHDR_IPV4 IPPROTO_IPIP

#define HDRS_IPinIP     (( 1 << L3_IPv4 ) | ( 1 << L3_IPv6 )) /* 0xC0 */
#define RX_IPinIP(b)    (((b)->rx.info.hdrs & HDRS_IPinIP)==HDRS_IPinIP)
#define TX_IPinIP(b)    (((b)->tx.info.hdrs & HDRS_IPinIP)==HDRS_IPinIP)
#define RX_IPV6(b)      ((b)->rx.info.bmap.L3_IPv6)
#define RX_IPV4(b)      ((b)->rx.info.bmap.L3_IPv4)
#define TX_IPV6(b)      ((b)->tx.info.bmap.L3_IPv6)
#define TX_IPV4(b)      ((b)->tx.info.bmap.L3_IPv4)
#define RX_IPV4ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << L3_IPv4))
#define TX_IPV4ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << L3_IPv4))
#define RX_IPV6ONLY(b)  (((b)->rx.info.hdrs & HDRS_IPinIP)==(1 << L3_IPv6))
#define TX_IPV6ONLY(b)  (((b)->tx.info.hdrs & HDRS_IPinIP)==(1 << L3_IPv6))

#define T4in6UP(b)      (RX_IPV4ONLY(b) && TX_IPinIP(b))
#define T4in6DN(b)      (RX_IPinIP(b) && TX_IPV4ONLY(b))

#define T6in4UP(b)      (RX_IPV6ONLY(b) && TX_IPinIP(b))
#define T6in4DN(b)      (RX_IPinIP(b) && TX_IPV6ONLY(b))

#define CHK4in6(b)      (T4in6UP(b) || T4in6DN(b))
#define CHK6in4(b)      (T6in4UP(b) || T6in4DN(b))

typedef struct ip6_addr {
    union {
        uint8_t     p8[16];
        uint16_t    p16[8];
        uint32_t    p32[4];
    };
} ip6_addr_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log IPv6 Tuple.
 * Packed: 3 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogTupleV6_t {
    union {
        uint32_t    word0;
        struct {
            uint32_t ver:4; 
            uint32_t tclass:8; 
            uint32_t flow_label:20;
        };
    };

    union {
        uint32_t    word1;
        struct {
            uint16_t length; 
            uint8_t next_hdr; 
            uint8_t hop_limit;
        };
    };

    ip6_addr_t      saddr;
    ip6_addr_t      daddr;

    union {
        struct {
            uint16_t    source;     /* L4 source port */
            uint16_t    dest;       /* L4 dest port */
        }           port;
        uint32_t    ports;
    };

    struct {
       uint16_t    exthdrs:14; /* Bit field of IPv6 extension headers */
       uint16_t    fragflag:1; /* 6in4 Upstream IPv4 fragmentation flag */
       uint16_t    tunnel:1;   /* Indication of IPv6 tunnel */
    };

    uint16_t       ipid;       /* 6in4 Upstream IPv4 identification */

} ____cacheline_aligned;
typedef struct blogTupleV6_t BlogTupleV6_t;

/*
 *------------------------------------------------------------------------------
 * Buffer to log Layer 2 and IP Tuple headers.
 * Packed: 4 16byte cachelines
 *------------------------------------------------------------------------------
 */
struct blogHeader_t {

    BlogTuple_t         tuple;          /* L3+L4 IP Tuple log */

    union {
        void            * dev_p;        /* physical network device */
        void            * reserved2;
    };

    union {
        BlogInfo_t      info;
        uint32_t        word;           /* channel, count, rfc2684, bmap */
        uint32_t        pktlen;         /* stats info */
    };

    union {
        uint8_t         reserved;
        uint8_t         nf_dir;
    };
    uint8_t             length;         /* L2 header total length */
    uint8_t /*BlogEncap_t*/ encap[ BLOG_ENCAP_MAX ];/* All L2 header types */

    uint8_t             l2hdr[ BLOG_HDRSZ_MAX ];    /* Data of all L2 headers */

} ____cacheline_aligned;

typedef struct blogHeader_t BlogHeader_t;           /* L2 and L3+4 tuple */

/* Coarse hash key: L1, L3, L4 hash */
union blogHash_t {
    uint32_t        match;
    struct {
        uint8_t     unused; 
        uint8_t     protocol;           /* IP protocol */

        struct {
            uint8_t channel;

            union {
                struct {
                    uint8_t         phyLen   : 4;
                    uint8_t         phyType  : 4;
                };
                uint8_t         phy;
            };
        } l1_tuple;
    };
};

typedef union blogHash_t BlogHash_t;

/*
 *------------------------------------------------------------------------------
 * Buffer log structure.
 * Packed: 17 16 byte cachelines, 272bytes per blog.
 *------------------------------------------------------------------------------
 */
struct blog_t {

    union {
        void            * void_p;
        struct blog_t   * blog_p;       /* Free list of Blog_t */
        struct sk_buff  * skb_p;        /* Associated sk_buff */
    };
    BlogHash_t          key;            /* Coarse hash search key */
    uint32_t            hash;           /* hash */
    void                * mc_fdb;       /* physical rx network device */

    void                * fdb[2];       /* fdb_src and fdb_dst */
    int8_t              delta[MAX_VIRT_DEV];  /* octet delta info */
    uint8_t             vtag_num;

    uint16_t            minMtu;
    union {
        uint16_t        flags;
        struct {
            uint16_t    unused:      9;
            uint16_t    nf_dir:      1;
            uint16_t    pop_pppoa:   1;
            uint16_t    insert_eth:  1;
            uint16_t    iq_prio:     1;
            uint16_t    mc_sync:     1;
            uint16_t    inactive:    1;
            uint16_t    incomplete:  1;
        };
    };
    uint32_t            mark;           /* NF mark value on tx */
    uint32_t            priority;       /* Tx  priority */
    void                * blogRule_p;   /* List of Blog Rules */

    /* pointers to the devices which the flow goes thru */
    void                * virt_dev_p[MAX_VIRT_DEV];
    uint32_t            vid;

    BlogTupleV6_t       tupleV6;        /* L3+L4 IP Tuple log */

    BlogHeader_t        tx;             /* Transmit path headers */
    BlogHeader_t        rx;             /* Receive path headers */

    uint32_t            dev_xmit;
    /* Flow connection/session tracker */
    void                * ct_p[BLOG_PARAM2_MAX];
    BlogActivateKey_t   activate_key;
} ____cacheline_aligned;


extern const char       * strBlogAction[];
extern const char       * strBlogEncap[];
extern const char       * strRfc2684[];
extern const uint8_t    rfc2684HdrLength[];
extern const uint8_t    rfc2684HdrData[][16];


#else
struct blog_t {void * blogRule_p;};
#endif /* defined(CONFIG_BLOG) */

/*
 * -----------------------------------------------------------------------------
 * Blog functional interface
 * -----------------------------------------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 * Section 1. Extension of a packet context with a logging context
 * -----------------------------------------------------------------------------
 */

#if defined(CONFIG_BLOG)
#define blog_ptr(skb_p)         skb_p->blog_p
#else
#define blog_ptr(skb_p)         BLOG_NULL
#endif

/* Allocate or deallocate a Blog_t */
Blog_t * blog_get(void);
void     blog_put(Blog_t * blog_p);

/* Allocate a Blog_t and associate with sk_buff or fkbuff */
extern Blog_t * blog_skb(struct sk_buff  * skb_p);
extern Blog_t * blog_fkb(struct fkbuff  * fkb_p);

/* Clear association of Blog_t with sk_buff */
extern Blog_t * blog_snull(struct sk_buff * skb_p);
extern Blog_t * blog_fnull(struct fkbuff  * fkb_p);

/* Clear association of Blog_t with sk_buff and free Blog_t object */
extern void blog_free(struct sk_buff * skb_p);

/* Disable further logging. Dis-associate with skb and free Blog object */
extern void blog_skip(struct sk_buff * skb_p);

/* Transfer association of a Blog_t object between two sk_buffs. */
extern void blog_xfer(struct sk_buff * skb_p, const struct sk_buff * prev_p);

/* Duplicate a Blog_t object for another skb. */
extern void blog_clone(struct sk_buff * skb_p, const struct blog_t * prev_p);

/* Copy a Blog_t object another blog object. */
extern void blog_copy(struct blog_t * new_p, const struct blog_t * prev_p);


/*
 *------------------------------------------------------------------------------
 *  Section 2. Associating native OS or 3rd-party network constructs
 *------------------------------------------------------------------------------
 */

extern void blog_link(BlogNetEntity_t entity_type, Blog_t * blog_p,
                      void * net_p, uint32_t param1, uint32_t param2);

/*
 *------------------------------------------------------------------------------
 * Section 3. Network construct and Blog client co-existence call backs
 *------------------------------------------------------------------------------
 */

extern void blog_notify(BlogNotify_t event, void * net_p,
                        uint32_t param1, uint32_t param2);

extern uint32_t blog_request(BlogRequest_t event, void * net_p,
                        uint32_t param1, uint32_t param2);

/*
 *------------------------------------------------------------------------------
 * Section 4. Network end-point binding of Blog client
 *
 * If rx hook is defined,
 *  blog_sinit(): initialize a fkb from skb, and pass to hook
 *          if packet is consumed, skb is released.
 *          if packet is blogged, the blog is associated with skb.
 *  blog_finit(): pass to hook
 *          if packet is to be blogged, the blog is associated with fkb.
 *  blog_finit_locked(): same as blog_finit, but caller must have already
 *          locked the blog layer, see blog_lock/blog_unlock in section 6.
 *
 * If tx hook is defined, invoke tx hook, dis-associate and free Blog_t
 *------------------------------------------------------------------------------
 */
extern BlogAction_t blog_sinit(struct sk_buff *skb_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);


extern BlogAction_t blog_finit_locked(struct fkbuff *fkb_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);

extern BlogAction_t blog_finit(struct fkbuff *fkb_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);

#if defined(CONFIG_BLOG)
extern BlogAction_t _blog_emit(void * nbuff_p, void * dev_p,
                             uint32_t encap, uint32_t channel, uint32_t phyHdr);

static inline BlogAction_t blog_emit(void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr)
{
    if ( nbuff_p == NULL ) return PKT_NORM;
    if ( !IS_SKBUFF_PTR(nbuff_p) ) return PKT_NORM;
    // OK, this is something worth looking at, call real function
    return ( _blog_emit(nbuff_p, dev_p, encap, channel, phyHdr) );
}
#else
BlogAction_t blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr );
#endif

/*
 * blog_iq_prio determines the Ingress QoS priority of the packet
 */
extern int blog_iq_prio(struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr);
/*
 *------------------------------------------------------------------------------
 *  blog_activate(): static configuration function of blog application
 *             pass a filled blog to the hook for configuration
 *------------------------------------------------------------------------------
 */
extern uint32_t blog_activate( Blog_t * blog_p, BlogTraffic_t traffic );

/*
 *------------------------------------------------------------------------------
 *  blog_deactivate(): static deconfiguration function of blog application
 *------------------------------------------------------------------------------
 */
extern Blog_t * blog_deactivate( uint32_t key, BlogTraffic_t traffic );

/*
 * -----------------------------------------------------------------------------
 * User defined filter invoked invoked in the rx hook. A user may override the
 * Blog action defined by the client. To enable the invocation of this API
 * in blog_finit, ensure that CC_BLOG_SUPPORT_USER_FILTER is enabled. Also, a
 * network device driver may directly invoke blog_filter() to override PKT_BLOG
 * and return PKT_NORM (by releasing the associated Blog_t).
 * -----------------------------------------------------------------------------
 */
extern BlogAction_t blog_filter(Blog_t * blog_p);

/*
 * -----------------------------------------------------------------------------
 * Section 5. Binding Blog client applications:
 *
 * Blog defines three hooks:
 *
 *  RX Hook: If this hook is defined then blog_init() will pass the packet to
 *           the Rx Hook using the FkBuff_t context. L1 and encap information
 *           are passed to the receive hook. The private network device context 
 *           may be extracted using the passed net_device object, if needed.
 *
 *  TX Hook: If this hook is defined then blog_emit() will check to see whether
 *           the NBuff has a Blog_t, and if so pass the NBuff and Blog to the
 *           bound Tx hook.
 *
 *  NotifHook: When blog_notify is invoked, the bound hook is invoked. Based on
 *           event type the bound Blog client may perform a custom action.
 *
 *  SC Hook: If this hook is defined, blog_activate() will pass a blog with
 *           necessary information for statical configuration.
 *
 *  SD Hook: If this hook is defined, blog_deactivate() will pass a pointer
 *           to a network object with BlogActivateKey information. The
 *           respective flow entry will be deleted.
 *
 * -----------------------------------------------------------------------------
 */
typedef union {
    struct {
        uint8_t         unused      : 3;
        uint8_t         RX_HOOK     : 1;
        uint8_t         TX_HOOK     : 1;
        uint8_t         XX_HOOK     : 1;
        uint8_t         SC_HOOK     : 1;
        uint8_t         SD_HOOK     : 1;
    } bmap;
    uint8_t             hook_info;
} BlogBind_t;

typedef BlogAction_t (* BlogDevHook_t)(void * fkb_skb_p, void * dev_p,
                                       uint32_t encap, uint32_t blogHash);

typedef void (* BlogNotifyHook_t)(BlogNotify_t notification, void * net_p,
                                  uint32_t param1, uint32_t param2);

typedef uint32_t (* BlogScHook_t)(Blog_t * blog_p, BlogTraffic_t traffic);

typedef Blog_t * (* BlogSdHook_t)(uint32_t key, BlogTraffic_t traffic);

extern void blog_bind(BlogDevHook_t rx_hook,    /* Client Rx netdevice handler*/
                      BlogDevHook_t tx_hook,    /* Client Tx netdevice handler*/
                      BlogNotifyHook_t xx_hook, /* Client notification handler*/
                      BlogScHook_t sc_hook,    /* Client static config handler*/
                      BlogSdHook_t sd_hook,    /* Client static deconf handler*/
                      BlogBind_t   bind
                     );


/*
 * -----------------------------------------------------------------------------
 * Section 6. Miscellanous
 * -----------------------------------------------------------------------------
 */

/* Logging of L2|L3 headers */
extern void blog(struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,  
                 size_t len, void * data_p);

/* Dump a Blog_t object */
extern void blog_dump(Blog_t * blog_p);

/* Get the minimum Tx MTU for a blog */
uint16_t blog_getTxMtu(Blog_t * blog_p);

/*
 * Lock and unlock the blog layer.  This is used to reduce the number of
 * times the blog lock must be acquired and released during bulk rx processing.
 * See also blog_finit_locked.
 */
extern void blog_lock_bh(void);
extern void blog_unlock_bh(void);


#endif /* defined(__BLOG_H_INCLUDED__) */
