/*
<:GPL/GPL:standard

 Copyright 2003 Broadcom Corp. All Rights Reserved.

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

/*
 *******************************************************************************
 * File Name  : blog.c
 * Description: Implements the tracing of L2 and L3 modifications to a packet
 * 		buffer while it traverses the Linux networking stack.
 *******************************************************************************
 */

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/blog.h>
#include <linux/blog_net.h>
#include <linux/nbuff.h>
#include <linux/skbuff.h>
#include <skb_defines.h>

#if defined(CONFIG_BLOG)

#include <linux/netdevice.h>
#include <linux/slab.h>
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)   
#define BLOG_NF_CONNTRACK
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#endif /* defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE) */

#include "../bridge/br_private.h"
#include "../bridge/br_igmp.h"
#include "../bridge/br_mld.h"

#include <linux/bcm_colors.h>
#include <linux/bcm_assert_locks.h>

/*--- globals ---*/

/* RFC4008 */
uint32_t blog_nat_tcp_def_idle_timeout = 86400 *HZ;  /* 5 DAYS */
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
uint32_t blog_nat_udp_def_idle_timeout = 30 *HZ;    /* 30 seconds */
#else
uint32_t blog_nat_udp_def_idle_timeout = 300 *HZ;    /* 300 seconds */
#endif
uint32_t blog_nat_generic_def_idle_timeout = 600 *HZ;/* 600 seconds */

EXPORT_SYMBOL(blog_nat_tcp_def_idle_timeout);
EXPORT_SYMBOL(blog_nat_udp_def_idle_timeout);
EXPORT_SYMBOL(blog_nat_generic_def_idle_timeout);

/* Debug macros */
int blog_dbg = 0;


#if defined(CC_BLOG_SUPPORT_DEBUG)
#define blog_print(fmt, arg...)                                         \
    if ( blog_dbg )                                                     \
    printk( CLRc "BLOG %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define blog_assertv(cond)                                              \
    if ( !cond ) {                                                      \
        printk( CLRerr "BLOG ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return;                                                         \
    }
#define blog_assertr(cond, rtn)                                         \
    if ( !cond ) {                                                      \
        printk( CLRerr "BLOG ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return rtn;                                                     \
    }
#define BLOG_DBG(debug_code)    do { debug_code } while(0)
#else
#define blog_print(fmt, arg...) NULL_STMT
#define blog_assertv(cond)      NULL_STMT
#define blog_assertr(cond, rtn) NULL_STMT
#define BLOG_DBG(debug_code)    NULL_STMT
#endif

#define blog_error(fmt, arg...)                                         \
    printk( CLRerr "BLOG ERROR %s :" fmt CLRnl, __FUNCTION__, ##arg)

#undef  BLOG_DECL
#define BLOG_DECL(x)        #x,         /* string declaration */

/*--- globals ---*/

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
DEFINE_SPINLOCK(blog_lock_g);               /* blogged packet flow */
EXPORT_SYMBOL(blog_lock_g);
static DEFINE_SPINLOCK(blog_pool_lock_g);   /* blog pool only */
#define BLOG_POOL_LOCK()    spin_lock_irq( &blog_pool_lock_g )
#define BLOG_POOL_UNLOCK()  spin_unlock_irq( &blog_pool_lock_g )
#define BLOG_LOCK()         spin_lock_bh( &blog_lock_g )
#define BLOG_UNLOCK()       spin_unlock_bh( &blog_lock_g )
#define BLOG_LOCK_BH()      spin_lock_bh( &blog_lock_g )
#define BLOG_UNLOCK_BH()    spin_unlock_bh( &blog_lock_g )
#else
#define BLOG_POOL_LOCK()    local_irq_disable()
#define BLOG_POOL_UNLOCK()  local_irq_enable()
#define BLOG_LOCK()         local_irq_disable()
#define BLOG_UNLOCK()       local_irq_enable()
#define BLOG_LOCK_BH()      NULL_STMT
#define BLOG_UNLOCK_BH()    NULL_STMT
#endif


/*
 * blog_support_mcast_g inherits the default value from CC_BLOG_SUPPORT_MCAST
 * Exported blog_support_mcast() may be used to set blog_support_mcast_g.
 */
int blog_support_mcast_g = CC_BLOG_SUPPORT_MCAST;
void blog_support_mcast(int config) { blog_support_mcast_g = config; }

/*
 * blog_support_ipv6_g inherits the value from CC_BLOG_SUPPORT_IPV6
 * Exported blog_support_ipv6() may be used to set blog_support_ipv6_g.
 */
int blog_support_ipv6_g = CC_BLOG_SUPPORT_IPV6;
void blog_support_ipv6(int config) { blog_support_ipv6_g = config; }

/*
 * Traffic flow generator, keep conntrack alive during idle traffic periods
 * by refreshing the conntrack. Dummy sk_buff passed to nf_conn.
 * Netfilter may not be statically loaded.
 */
blog_refresh_t blog_refresh_fn = (blog_refresh_t) NULL;
struct sk_buff * nfskb_p = (struct sk_buff *) NULL;

/*----- Constant string representation of enums for print -----*/
const char * strBlogAction[BLOG_ACTION_MAX] =
{
    BLOG_DECL(PKT_DONE)
    BLOG_DECL(PKT_NORM)
    BLOG_DECL(PKT_BLOG)
    BLOG_DECL(PKT_DROP)
};

const char * strBlogDir[BLOG_DIR_MAX] =
{
    BLOG_DECL(DIR_RX)
    BLOG_DECL(DIR_TX)
};

const char * strBlogNetEntity[BLOG_NET_ENTITY_MAX] =
{
    BLOG_DECL(FLOWTRACK)
    BLOG_DECL(BRIDGEFDB)
    BLOG_DECL(MCAST_FDB)
    BLOG_DECL(IF_DEVICE)
};

const char * strBlogNotify[BLOG_NOTIFY_MAX] =
{
    BLOG_DECL(DESTROY_FLOWTRACK)
    BLOG_DECL(DESTROY_BRIDGEFDB)
    BLOG_DECL(MCAST_CONTROL_EVT)
    BLOG_DECL(MCAST_SYNC_EVT)
    BLOG_DECL(DESTROY_NETDEVICE)
    BLOG_DECL(LINK_STATE_CHANGE)
    BLOG_DECL(FETCH_NETIF_STATS)
    BLOG_DECL(DYNAMIC_DSCP_EVENT)
    BLOG_DECL(UPDATE_NETDEVICE)
};

const char * strBlogRequest[BLOG_REQUEST_MAX] =
{
    BLOG_DECL(FLOWTRACK_KEY_SET)
    BLOG_DECL(FLOWTRACK_KEY_GET)
    BLOG_DECL(FLOWTRACK_DSCP_GET)
    BLOG_DECL(FLOW_CONFIRMED)
    BLOG_DECL(FLOW_ASSURED)
    BLOG_DECL(FLOW_ALG_HELPER)
    BLOG_DECL(FLOW_EXCLUDE)
    BLOG_DECL(FLOW_REFRESH)
    BLOG_DECL(BRIDGE_REFRESH)
    BLOG_DECL(NETIF_PUT_STATS)
    BLOG_DECL(LINK_XMIT_FN)
    BLOG_DECL(LINK_NOCARRIER)
    BLOG_DECL(NETDEV_NAME)
    BLOG_DECL(MCAST_KEY_SET)
    BLOG_DECL(MCAST_KEY_GET)
    BLOG_DECL(MCAST_DFLT_MIPS)
    BLOG_DECL(IQPRIO_SKBMARK_SET)
};

const char * strBlogEncap[PROTO_MAX] =
{
    BLOG_DECL(BCM_XPHY)
    BLOG_DECL(BCM_SWC)
    BLOG_DECL(ETH_802x)
    BLOG_DECL(VLAN_8021Q)
    BLOG_DECL(PPPoE_2516)
    BLOG_DECL(PPP_1661)
    BLOG_DECL(L3_IPv4)
    BLOG_DECL(L3_IPv6)
};

/*
 *------------------------------------------------------------------------------
 * Support for RFC 2684 headers logging.
 *------------------------------------------------------------------------------
 */
const char * strRfc2684[RFC2684_MAX] =
{
    BLOG_DECL(RFC2684_NONE)         /*                               */
    BLOG_DECL(LLC_SNAP_ETHERNET)    /* AA AA 03 00 80 C2 00 07 00 00 */
    BLOG_DECL(LLC_SNAP_ROUTE_IP)    /* AA AA 03 00 00 00 08 00       */
    BLOG_DECL(LLC_ENCAPS_PPP)       /* FE FE 03 CF                   */
    BLOG_DECL(VC_MUX_ETHERNET)      /* 00 00                         */
    BLOG_DECL(VC_MUX_IPOA)          /*                               */
    BLOG_DECL(VC_MUX_PPPOA)         /*                               */
    BLOG_DECL(PTM)                  /*                               */
};

const uint8_t rfc2684HdrLength[RFC2684_MAX] =
{
     0, /* header was already stripped. :                               */
    10, /* LLC_SNAP_ETHERNET            : AA AA 03 00 80 C2 00 07 00 00 */
     8, /* LLC_SNAP_ROUTE_IP            : AA AA 03 00 00 00 08 00       */
     4, /* LLC_ENCAPS_PPP               : FE FE 03 CF                   */
     2, /* VC_MUX_ETHERNET              : 00 00                         */
     0, /* VC_MUX_IPOA                  :                               */
     0, /* VC_MUX_PPPOA                 :                               */
     0, /* PTM                          :                               */
};

const uint8_t rfc2684HdrData[RFC2684_MAX][16] =
{
    {},
    { 0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00 },
    { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00 },
    { 0xFE, 0xFE, 0x03, 0xCF },
    { 0x00, 0x00 },
    {},
    {},
    {}
};

const char * strBlogPhy[BLOG_MAXPHY] =
{
    BLOG_DECL(BLOG_XTMPHY)
    BLOG_DECL(BLOG_ENETPHY)
    BLOG_DECL(BLOG_GPONPHY)
    BLOG_DECL(BLOG_USBPHY)
    BLOG_DECL(BLOG_WLANPHY)
    BLOG_DECL(BLOG_MOCAPHY)
};

const char * strIpctDir[] = {   /* in reference to enum ip_conntrack_dir */
    BLOG_DECL(DIR_ORIG)
    BLOG_DECL(DIR_RPLY)
    BLOG_DECL(DIR_UNKN)
};

const char * strIpctStatus[] =  /* in reference to enum ip_conntrack_status */
{
    BLOG_DECL(EXPECTED)
    BLOG_DECL(SEEN_REPLY)
    BLOG_DECL(ASSURED)
    BLOG_DECL(CONFIRMED)
    BLOG_DECL(SRC_NAT)
    BLOG_DECL(DST_NAT)
    BLOG_DECL(SEQ_ADJUST)
    BLOG_DECL(SRC_NAT_DONE)
    BLOG_DECL(DST_NAT_DONE)
    BLOG_DECL(DYING)
    BLOG_DECL(FIXED_TIMEOUT)
    BLOG_DECL(BLOG)
};

/*
 *------------------------------------------------------------------------------
 * Default Rx and Tx hooks.
 * FIXME: Group these hooks into a structure and change blog_bind to use
 *        a structure.
 *------------------------------------------------------------------------------
 */
static BlogDevHook_t blog_rx_hook_g = (BlogDevHook_t)NULL;
static BlogDevHook_t blog_tx_hook_g = (BlogDevHook_t)NULL;
static BlogNotifyHook_t blog_xx_hook_g = (BlogNotifyHook_t)NULL;
static BlogScHook_t blog_sc_hook_g = (BlogScHook_t)NULL;
static BlogSdHook_t blog_sd_hook_g = (BlogSdHook_t)NULL;

/*
 *------------------------------------------------------------------------------
 * Blog_t Free Pool Management.
 * The free pool of Blog_t is self growing (extends upto an engineered
 * value). Could have used a kernel slab cache. 
 *------------------------------------------------------------------------------
 */

/* Global pointer to the free pool of Blog_t */
static Blog_t * blog_list_gp = BLOG_NULL;

static int blog_extends = 0;        /* Extension of Pool on depletion */
#if defined(CC_BLOG_SUPPORT_DEBUG)
static int blog_cnt_free = 0;       /* Number of Blog_t free */
static int blog_cnt_used = 0;       /* Number of in use Blog_t */
static int blog_cnt_hwm  = 0;       /* In use high water mark for engineering */
static int blog_cnt_fails = 0;
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : blog_extend
 * Description: Create a pool of Blog_t objects. When a pool is exhausted
 *              this function may be invoked to extend the pool. The pool is
 *              identified by a global pointer, blog_list_gp. All objects in
 *              the pool chained together in a single linked list.
 * Parameters :
 *   num      : Number of Blog_t objects to be allocated.
 * Returns    : Number of Blog_t objects allocated in pool.
 *
 * CAUTION: blog_extend must be called with blog_pool_lock_g acquired.
 *------------------------------------------------------------------------------
 */
uint32_t blog_extend( uint32_t num )
{
    register int i;
    register Blog_t * list_p;

    blog_print( "%u", num );

    list_p = (Blog_t *) kmalloc( num * sizeof(Blog_t), GFP_ATOMIC);
    if ( list_p == BLOG_NULL )
    {
#if defined(CC_BLOG_SUPPORT_DEBUG)
        blog_cnt_fails++;
#endif
        blog_print( "WARNING: Failure to initialize %d Blog_t", num );
        return 0;
    }

    /* memset( (void *)list_p, 0, (sizeof(Blog_t) * num ); */
    for ( i = 0; i < num; i++ )
        list_p[i].blog_p = &list_p[i+1];

    blog_extends++;

    BLOG_DBG( blog_cnt_free += num; );
    list_p[num-1].blog_p = blog_list_gp; /* chain last Blog_t object */
    blog_list_gp = list_p;  /* Head of list */

    return num;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clr
 * Description  : Clear the data of a Blog_t
 *                Need not be protected by blog_pool_lock_g
 *------------------------------------------------------------------------------
 */
static inline void blog_clr( Blog_t * blog_p )
{
    blog_assertv( ((blog_p != BLOG_NULL) && (_IS_BPTR_(blog_p))) );
    BLOG_DBG( memset( (void*)blog_p, 0, sizeof(Blog_t) ); );

    /* clear phyHdr, count, bmap, and channel */
    blog_p->rx.word = 0;
    blog_p->tx.word = 0;
    blog_p->key.match = 0;    /* clears hash, protocol, l1_tuple */
    blog_p->ct_p[BLOG_PARAM2_IPV4] = (void *)NULL;
    blog_p->ct_p[BLOG_PARAM2_IPV6] = (void *)NULL;
    blog_p->tx.dev_p = (void *)NULL;
    blog_p->fdb[0] = (void *)NULL;
    blog_p->fdb[1] = (void *)NULL;
    blog_p->minMtu = BLOG_ETH_MTU_LEN;
    blog_p->flags = 0;
    blog_p->vid = 0xFFFFFFFF;
    blog_p->vtag_num = 0;
    blog_p->mark = 0;
    blog_p->rx.dev_p = (void *)NULL;
    memset( (void*)blog_p->virt_dev_p, 0, sizeof(void*) * MAX_VIRT_DEV);

    blog_print( "blog<0x%08x>", (int)blog_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_get
 * Description  : Allocate a Blog_t from the free list
 * Returns      : Pointer to an Blog_t or NULL, on depletion.
 *------------------------------------------------------------------------------
 */
Blog_t * blog_get( void )
{
    register Blog_t * blog_p;

    BLOG_POOL_LOCK();   /* DO NOT USE blog_assertr() until BLOG_POOL_UNLOCK() */

    if ( blog_list_gp == BLOG_NULL )
    {
#ifdef CC_BLOG_SUPPORT_EXTEND
        if ( (blog_extends >= BLOG_EXTEND_MAX_ENGG)/* Try extending free pool */
          || (blog_extend( BLOG_EXTEND_SIZE_ENGG ) != BLOG_EXTEND_SIZE_ENGG))
        {
            blog_print( "WARNING: free list exhausted" );
        }
#else
        if ( blog_extend( BLOG_EXTEND_SIZE_ENGG ) == 0 )
        {
            blog_print( "WARNING: out of memory" );
        }
#endif
        blog_p = BLOG_NULL;

        BLOG_POOL_UNLOCK(); /* May use blog_assertr() now onwards */

        goto blog_get_return;
    }

    BLOG_DBG(
        blog_cnt_free--;
        if ( ++blog_cnt_used > blog_cnt_hwm )
            blog_cnt_hwm = blog_cnt_used;
        );

    blog_p = blog_list_gp;
    blog_list_gp = blog_list_gp->blog_p;

    BLOG_POOL_UNLOCK();     /* May use blog_assertr() now onwards */

    blog_clr( blog_p );     /* quickly clear the contents */

blog_get_return:

    blog_print( "blog<0x%08x>", (int)blog_p );

    return blog_p;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_put
 * Description  : Release a Blog_t back into the free pool
 * Parameters   :
 *  blog_p      : Pointer to a non-null Blog_t to be freed.
 *------------------------------------------------------------------------------
 */
void blog_put( Blog_t * blog_p )
{
    blog_assertv( ((blog_p != BLOG_NULL) && (_IS_BPTR_(blog_p))) );

    blog_clr( blog_p );

    BLOG_POOL_LOCK();   /* DO NOT USE blog_assertv() until BLOG_POOL_UNLOCK() */

    BLOG_DBG( blog_cnt_used--; blog_cnt_free++; );
    blog_p->blog_p = blog_list_gp;  /* clear pointer to skb_p */
    blog_list_gp = blog_p;          /* link into free pool */

    BLOG_POOL_UNLOCK();/* May use blog_assertv() now onwards */

    blog_print( "blog<0x%08x>", (int)blog_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_skb
 * Description  : Allocate and associate a Blog_t with an sk_buff.
 * Parameters   :
 *  skb_p       : pointer to a non-null sk_buff
 * Returns      : A Blog_t object or NULL,
 *------------------------------------------------------------------------------
 */
Blog_t * blog_skb( struct sk_buff * skb_p )
{
    blog_assertr( (skb_p != (struct sk_buff *)NULL), BLOG_NULL );
    blog_assertr( (!_IS_BPTR_(skb_p->blog_p)), BLOG_NULL ); /* avoid leak */

    skb_p->blog_p = blog_get(); /* Allocate and associate with sk_buff */

    blog_print( "skb<0x%08x> blog<0x%08x>", (int)skb_p, (int)skb_p->blog_p );

    /* CAUTION: blog_p does not point back to the skb, do it explicitly */
    return skb_p->blog_p;       /* May be null */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_fkb
 * Description  : Allocate and associate a Blog_t with an fkb.
 * Parameters   :
 *  fkb_p       : pointer to a non-null FkBuff_t
 * Returns      : A Blog_t object or NULL,
 *------------------------------------------------------------------------------
 */
Blog_t * blog_fkb( struct fkbuff * fkb_p )
{
    uint32_t in_skb_tag;
    blog_assertr( (fkb_p != (FkBuff_t *)NULL), BLOG_NULL );
    blog_assertr( (!_IS_BPTR_(fkb_p->blog_p)), BLOG_NULL ); /* avoid leak */

    in_skb_tag = _is_in_skb_tag_( fkb_p->flags );

    fkb_p->blog_p = blog_get(); /* Allocate and associate with fkb */

    if ( fkb_p->blog_p != BLOG_NULL )   /* Move in_skb_tag to blog rx info */
        fkb_p->blog_p->rx.info.fkbInSkb = in_skb_tag;

    blog_print( "fkb<0x%08x> blog<0x%08x> in_skb_tag<%u>",
                (int)fkb_p, (int)fkb_p->blog_p, in_skb_tag );
    return fkb_p->blog_p;       /* May be null */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_snull
 * Description  : Dis-associate a sk_buff with any Blog_t
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 * Returns      : Previous Blog_t associated with sk_buff
 *------------------------------------------------------------------------------
 */
inline Blog_t * _blog_snull( struct sk_buff * skb_p )
{
    register Blog_t * blog_p;
    blog_p = skb_p->blog_p;
    skb_p->blog_p = BLOG_NULL;
    return blog_p;
}

Blog_t * blog_snull( struct sk_buff * skb_p )
{
    blog_assertr( (skb_p != (struct sk_buff *)NULL), BLOG_NULL );
    blog_print( "skb<0x%08x> blog<0x%08x>", (int)skb_p, (int)skb_p->blog_p );
    return _blog_snull( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_fnull
 * Description  : Dis-associate a fkbuff with any Blog_t
 * Parameters   :
 *  fkb_p       : Pointer to a non-null fkbuff
 * Returns      : Previous Blog_t associated with fkbuff
 *------------------------------------------------------------------------------
 */
inline Blog_t * _blog_fnull( struct fkbuff * fkb_p )
{
    register Blog_t * blog_p;
    blog_p = fkb_p->blog_p;
    fkb_p->blog_p = BLOG_NULL;
    return blog_p;
}

Blog_t * blog_fnull( struct fkbuff * fkb_p )
{
    blog_assertr( (fkb_p != (struct fkbuff *)NULL), BLOG_NULL );
    blog_print( "fkb<0x%08x> blogp<0x%08x>", (int)fkb_p, (int)fkb_p->blog_p );
    return _blog_fnull( fkb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_free
 * Description  : Free any Blog_t associated with a sk_buff
 * Parameters   :
 *  skb_p       : Pointer to a non-null sk_buff
 *------------------------------------------------------------------------------
 */
inline void _blog_free( struct sk_buff * skb_p )
{
    register Blog_t * blog_p;
    blog_p = _blog_snull( skb_p );   /* Dis-associate Blog_t from skb_p */
    if ( likely(blog_p != BLOG_NULL) )
        blog_put( blog_p );         /* Recycle blog_p into free list */
}

void blog_free( struct sk_buff * skb_p )
{
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );
    BLOG_DBG(
        if ( skb_p->blog_p != BLOG_NULL )
            blog_print( "skb<0x%08x> blog<0x%08x> [<%08x>]",
                        (int)skb_p, (int)skb_p->blog_p,
                        (int)__builtin_return_address(0) ); );
    _blog_free( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_skip
 * Description  : Disable further tracing of sk_buff by freeing associated
 *                Blog_t (if any)
 * Parameters   :
 *  skb_p       : Pointer to a sk_buff
 *------------------------------------------------------------------------------
 */
void blog_skip( struct sk_buff * skb_p )
{
    blog_print( "skb<0x%08x> [<%08x>]",
                (int)skb_p, (int)__builtin_return_address(0) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );
    _blog_free( skb_p );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_xfer
 * Description  : Transfer ownership of a Blog_t between two sk_buff(s)
 * Parameters   :
 *  skb_p       : New owner of Blog_t object 
 *  prev_p      : Former owner of Blog_t object
 *------------------------------------------------------------------------------
 */
void blog_xfer( struct sk_buff * skb_p, const struct sk_buff * prev_p )
{
    Blog_t * blog_p;
    struct sk_buff * mod_prev_p;
    blog_assertv( (prev_p != (struct sk_buff *)NULL) );
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    mod_prev_p = (struct sk_buff *) prev_p; /* const removal without warning */
    blog_p = _blog_snull( mod_prev_p );
    skb_p->blog_p = blog_p;

    if ( likely(blog_p != BLOG_NULL) )
    {
        blog_print( "skb<0x%08x> to new<0x%08x> blog<0x%08x> [<%08x>]",
                    (int)prev_p, (int)skb_p, (int)blog_p,
                    (int)__builtin_return_address(0) );
        blog_assertv( (_IS_BPTR_(blog_p)) );
        blog_p->skb_p = skb_p;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_clone
 * Description  : Duplicate a Blog_t for another sk_buff
 * Parameters   :
 *  skb_p       : New owner of cloned Blog_t object 
 *  prev_p      : Blog_t object to be cloned
 *------------------------------------------------------------------------------
 */
void blog_clone( struct sk_buff * skb_p, const struct blog_t * prev_p )
{
    blog_assertv( (skb_p != (struct sk_buff *)NULL) );

    if ( likely(prev_p != BLOG_NULL) )
    {
        Blog_t * blog_p;

        blog_assertv( (_IS_BPTR_(prev_p)) );
        
        skb_p->blog_p = blog_get(); /* Allocate and associate with skb */
        blog_p = skb_p->blog_p;

        blog_print( "orig blog<0x%08x> new skb<0x%08x> blog<0x%08x> [<%08x>]",
                    (int)prev_p, (int)skb_p, (int)blog_p,
                    (int)__builtin_return_address(0) );

        if ( likely(blog_p != BLOG_NULL) )
        {
            blog_p->skb_p = skb_p;
#define CPY(x) blog_p->x = prev_p->x
            CPY(key.match);
            CPY(hash);
            CPY(mark);
            CPY(priority);
            CPY(rx);
            CPY(vid);
            CPY(vtag_num);
            CPY(tupleV6);
            blog_p->tx.word = 0;
        }
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_copy
 * Description  : Copy a Blog_t object another blog object.
 * Parameters   :
 *  new_p       : Blog_t object to be filled in
 *  prev_p      : Blog_t object with the data information
 *------------------------------------------------------------------------------
 */
void blog_copy(struct blog_t * new_p, const struct blog_t * prev_p)
{
    blog_assertv( (new_p != BLOG_NULL) );
    blog_print( "new_p<0x%08x> prev_p<0x%08x>", (int)new_p, (int)prev_p );

    if ( likely(prev_p != BLOG_NULL) )
    {
       memcpy( new_p, prev_p, sizeof(Blog_t) );
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_link
 * Description  : Associate a network entity with an skb's blog object
 * Parameters   :
 *  entity_type : Network entity type
 *  blog_p      : Pointer to a Blog_t
 *  net_p       : Pointer to a network stack entity 
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 *------------------------------------------------------------------------------
 */
void blog_link( BlogNetEntity_t entity_type, Blog_t * blog_p,
                void * net_p, uint32_t param1, uint32_t param2 )
{
    blog_assertv( (entity_type < BLOG_NET_ENTITY_MAX) );
    blog_assertv( (net_p != (void *)NULL) );

    if ( unlikely(blog_p == BLOG_NULL) )
        return;

    blog_assertv( (_IS_BPTR_(blog_p)) );

    blog_print( "link<%s> skb<0x%08x> blog<0x%08x> net<0x%08x> %u %u [<%08x>]",
                strBlogNetEntity[entity_type], (int)blog_p->skb_p, (int)blog_p,
                (int)net_p, param1, param2, (int)__builtin_return_address(0) );

    switch ( entity_type )
    {
        case FLOWTRACK:
        {
#if defined(BLOG_NF_CONNTRACK)
            blog_assertv( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)||
                           (param2 == BLOG_PARAM2_IPV4)     ||
                           (param2 == BLOG_PARAM2_IPV6)) );

            if ( unlikely(blog_p->rx.info.multicast) )
                return;

            BLOG_LOCK_BH();

            /* param2 indicates the ct_p belongs to IPv4 or IPv6 */
            blog_p->ct_p[param2] = net_p; /* Pointer to conntrack */
            /* 
             * Save flow direction. Here we make one assumption:
             * If a packet traverses both IPv4 and IPv6 conntracks,
             * for example, 6in4 or 4in6 tunnel, the nf_dir must be the same
             * for both conntracks.
             */
            blog_p->nf_dir = param1;

            BLOG_UNLOCK_BH();
#endif
            break;
        }

        case BRIDGEFDB:
        {
            blog_assertv( ((param1 == BLOG_PARAM1_SRCFDB) ||
                           (param1 == BLOG_PARAM1_DSTFDB)) );

            BLOG_LOCK_BH();

            blog_p->fdb[param1] = net_p;

            BLOG_UNLOCK_BH();

            break;
        }

        case MCAST_FDB:
        {
            BLOG_LOCK_BH();
            blog_p->mc_fdb = net_p; /* Pointer to mc_fdb */
            BLOG_UNLOCK_BH();
            break;
        }

        case IF_DEVICE: /* link virtual interfaces traversed by flow */
        {
            int i;

            blog_assertv( (param1 < BLOG_DIR_MAX) );

            BLOG_LOCK_BH();

            for (i=0; i<MAX_VIRT_DEV; i++)
            {
                /* A flow should not rx and tx with the same device!!  */
                blog_assertv((net_p != DEVP_DETACH_DIR(blog_p->virt_dev_p[i])));

                if ( blog_p->virt_dev_p[i] == NULL )
                {
                    blog_p->virt_dev_p[i] = DEVP_APPEND_DIR(net_p, param1);
                    blog_p->delta[i] = param2 - blog_p->tx.pktlen;
                    break;
                }
            }

            BLOG_UNLOCK_BH();

            blog_assertv( (i != MAX_VIRT_DEV) );
            break;
        }

        default:
            break;
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_notify
 * Description  : Notify a Blog client (xx_hook) of an event.
 * Parameters   :
 *  event       : notification
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 *------------------------------------------------------------------------------
 */
void blog_notify( BlogNotify_t event, void * net_p,
                  uint32_t param1, uint32_t param2 )
{
    blog_assertv( (event < BLOG_NOTIFY_MAX) );
    blog_assertv( (net_p != (void *)NULL) );

    if ( unlikely(blog_xx_hook_g == (BlogNotifyHook_t)NULL) )
        return;

    blog_print( "notify<%s> net_p<0x%08x>"
                " param1<%u:0x%08x> param2<%u:0x%08x> [<%08x>]",
                strBlogNotify[event], (int)net_p,
                param1, (int)param1, param2, (int)param2,
                (int)__builtin_return_address(0) );

    BLOG_LOCK();

    blog_xx_hook_g( event, net_p, param1, param2 );

    BLOG_UNLOCK();

    return;
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_request
 * Description  : Blog client requests an operation to be performed on a network
 *                stack entity.
 * Parameters   :
 *  request     : request type
 *  net_p       : Pointer to a network stack entity
 *  param1      : optional parameter 1
 *  param2      : optional parameter 2
 *------------------------------------------------------------------------------
 */
extern void br_fdb_refresh( struct net_bridge_fdb_entry *fdb );
extern int blog_rule_delete_action( void *rule_p );

uint32_t blog_request( BlogRequest_t request, void * net_p,
                       uint32_t param1, uint32_t param2 )
{
    uint32_t ret;

    blog_assertr( (request < BLOG_REQUEST_MAX), 0 );
    blog_assertr( (net_p != (void *)NULL), 0 );

#if defined(CC_BLOG_SUPPORT_DEBUG)
    if ( (request!=FLOWTRACK_REFRESH) && (request!=BRIDGE_REFRESH) )
#endif
        blog_print( "request<%s> net_p<0x%08x>"
                    " param1<%u:0x%08x> param2<%u:0x%08x>",
                    strBlogRequest[request], (int)net_p,
                    param1, (int)param1, param2, (int)param2);

    switch ( request )
    {
#if defined(BLOG_NF_CONNTRACK)
        case FLOWTRACK_KEY_SET:
            blog_assertr( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)), 0 );
            ((struct nf_conn *)net_p)->blog_key[param1] = param2;
            return 0;

        case FLOWTRACK_KEY_GET:
            blog_assertr( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)), 0 );
            ret = ((struct nf_conn *)net_p)->blog_key[param1];
            break;

#if defined(CONFIG_NF_DYNDSCP) || defined(CONFIG_NF_DYNDSCP_MODULE)
        case FLOWTRACK_DSCP_GET:
            blog_assertr( ((param1 == BLOG_PARAM1_DIR_ORIG) ||
                           (param1 == BLOG_PARAM1_DIR_REPLY)), 0 );
            ret = ((struct nf_conn *)net_p)->dyndscp.dscp[param1];
            break;
#endif

        case FLOWTRACK_CONFIRMED:    /* E.g. UDP connection confirmed */
            ret = test_bit( IPS_CONFIRMED_BIT,
                            &((struct nf_conn *)net_p)->status );
            break;

        case FLOWTRACK_ASSURED:      /* E.g. TCP connection confirmed */
            ret = test_bit( IPS_ASSURED_BIT,
                            &((struct nf_conn *)net_p)->status );
            break;

        case FLOWTRACK_ALG_HELPER:
        {
            struct nf_conn * nfct_p;
            struct nf_conn_help * help;

            nfct_p = (struct nf_conn *)net_p;
            help = nfct_help(nfct_p);

            if ( (help != (struct nf_conn_help *)NULL )
                && (help->helper != (struct nf_conntrack_helper *)NULL) )
            {
                blog_print( "HELPER ct<0x%08x> helper<%s>",
                            (int)net_p, help->helper->name );
                return 1;
            }
            return 0;
        }

        case FLOWTRACK_EXCLUDE:  /* caution: modifies net_p */
            clear_bit(IPS_BLOG_BIT, &((struct nf_conn *)net_p)->status);
            return 0;

        case FLOWTRACK_REFRESH:
        {
            if ( blog_refresh_fn  )
            {
                uint32_t jiffies;

                if ( param2 == 0 )
                {
                    if ( param1 == IPPROTO_TCP )
                        jiffies = blog_nat_tcp_def_idle_timeout;
                    else if ( param1 == IPPROTO_UDP )
                        jiffies = blog_nat_udp_def_idle_timeout;
                    else
                        jiffies = 60;   /* default:non-TCP|UDP timer refresh */
                }
                else
                {
                   /* refresh timeout of unknown protocol */
                   jiffies = blog_nat_generic_def_idle_timeout;
                }

                nfskb_p->nfct = (struct nf_conntrack *)net_p;
                (*blog_refresh_fn)( net_p, 0, nfskb_p, jiffies, 0 );
            }
            return 0;
        }

#endif /* defined(BLOG_NF_CONNTRACK) */

        case BRIDGE_REFRESH:
        {
            br_fdb_refresh( (struct net_bridge_fdb_entry *)net_p );
            return 0;
        }

        case NETIF_PUT_STATS:
        {
            struct net_device * dev_p = (struct net_device *)net_p;
            BlogStats_t * bstats_p = (BlogStats_t *) param1;
            blog_assertr( (bstats_p != (BlogStats_t *)NULL), 0 );

            blog_print("dev_p<0x%08x> rx_pkt<%lu> rx_byte<%lu> tx_pkt<%lu>"
                       " tx_byte<%lu> multicast<%lu>", (int)dev_p,
                        bstats_p->rx_packets, bstats_p->rx_bytes,
                        bstats_p->tx_packets, bstats_p->tx_bytes,
                        bstats_p->multicast);

            if ( dev_p->put_stats )
                dev_p->put_stats( dev_p, bstats_p );
            return 0;
        }
        
        case LINK_XMIT_FN:
        {
            struct net_device * dev_p = (struct net_device *)net_p;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
            ret = (uint32_t)(dev_p->netdev_ops->ndo_start_xmit);
#else
            ret = (uint32_t)(dev_p->hard_start_xmit);
#endif
            break;
        }

        case LINK_NOCARRIER:
            ret = test_bit( __LINK_STATE_NOCARRIER,
                            &((struct net_device *)net_p)->state );
            break;

        case NETDEV_NAME:
        {
            struct net_device * dev_p = (struct net_device *)net_p;
            ret = (uint32_t)(dev_p->name);
            break;
        }

        case MCAST_KEY_SET:
        {
#if defined(CONFIG_BR_IGMP_SNOOP)
            if ( param1 == BlogTraffic_IPV4_MCAST )
               ((struct net_bridge_mc_fdb_entry *)net_p)->blog_idx = param2;
#endif
#if defined(CONFIG_BR_MLD_SNOOP)
            if ( param1 == BlogTraffic_IPV6_MCAST )
                ((struct net_br_mld_mc_fdb_entry *)net_p)->blog_idx = param2;
#endif
            return 0;
        }
#if 0
        case MCAST_KEY_GET:
            if ( param1 == BlogTraffic_IPV4_MCAST )
               ret = ((struct net_bridge_mc_fdb_entry *)net_p)->blog_idx;
#if defined(CONFIG_BR_MLD_SNOOP)
            else
               ret = ((struct net_br_mld_mc_fdb_entry *)net_p)->blog_idx;
#endif

            break;
#endif
        case IQPRIO_SKBMARK_SET:
        {
            Blog_t *blog_p = (Blog_t *)net_p;
            blog_p->mark = SKBMARK_SET_IQPRIO_MARK(blog_p->mark, param1 );
            return 0;
        }

        case MCAST_DFLT_MIPS:
        {
            blog_rule_delete_action( net_p );
            return 0;
        }

        default:
            return 0;
    }

    blog_print("ret<%u:0x%08x>", ret, (int)ret);

    return ret;
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_filter
 * Description  : Filter packets that need blogging.
 *                E.g. To skip logging of control versus data type packet.
 *   blog_p     : Received packet parsed and logged into a blog
 * Returns      :
 *   PKT_NORM   : If normal stack processing without logging
 *   PKT_BLOG   : If stack processing with logging
 *------------------------------------------------------------------------------
 */
BlogAction_t blog_filter( Blog_t * blog_p )
{
    blog_assertr( ((blog_p != BLOG_NULL) && (_IS_BPTR_(blog_p))), PKT_NORM );
    blog_assertr( (blog_p->rx.info.hdrs != 0), PKT_NORM );

    /*
     * E.g. IGRS/UPnP using Simple Service Discovery Protocol SSDP over HTTPMU
     *      HTTP Multicast over UDP 239.255.255.250:1900,
     *
     *  if ( ! RX_IPinIP(blog_p) && RX_IPV4(blog_p)
     *      && (blog_p->rx.tuple.daddr == htonl(0xEFFFFFFA))
     *      && (blog_p->rx.tuple.port.dest == 1900)
     *      && (blog_p->key.protocol == IPPROTO_UDP) )
     *          return PKT_NORM;
     *
     *  E.g. To filter IPv4 Local Network Control Block 224.0.0/24
     *             and IPv4 Internetwork Control Block  224.0.1/24
     *
     *  if ( ! RX_IPinIP(blog_p) && RX_IPV4(blog_p)
     *      && ( (blog_p->rx.tuple.daddr & htonl(0xFFFFFE00))
     *           == htonl(0xE0000000) )
     *          return PKT_NORM;
     *  
     */
    return PKT_BLOG;    /* continue in stack with logging */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_finit, blog_sinit
 * Description  : This function may be inserted in a physical network device's
 *                packet receive handler. A receive handler typically extracts
 *                the packet data from the rx DMA buffer ring, allocates and
 *                sets up a sk_buff, decodes the l2 headers and passes the
 *                sk_buff into the network stack via netif_receive_skb/netif_rx.
 *
 *                Prior to constructing a sk_buff, blog_finit() may be invoked
 *                using a fast kernel buffer to carry the received buffer's
 *                context <data,len>, and the receive net_device and l1 info.
 *
 *                This function invokes the bound receive blog hook.
 *
 * Parameters   :
 *  blog_finit() fkb_p: Pointer to a fast kernel buffer<data,len>
 *  blog_sinit() skb_p: Pointer to a Linux kernel skbuff
 *  dev_p       : Pointer to the net_device on which the packet arrived.
 *  encap       : First encapsulation type
 *  channel     : Channel/Port number on which the packet arrived.
 *  phyHdr      : e.g. XTM device RFC2684 header type
 *
 * Returns      :
 *  PKT_DONE    : The fkb|skb is consumed and device should not process fkb|skb.
 *
 *  PKT_NORM    : Device may invoke netif_receive_skb for normal processing.
 *                No Blog is associated and fkb reference count = 0.
 *                [invoking fkb_release() has no effect]
 *
 *  PKT_BLOG    : PKT_NORM behaviour + Blogging enabled.
 *                Must call fkb_release() to free associated Blog
 *
 *------------------------------------------------------------------------------
 */
inline
BlogAction_t blog_finit_locked( struct fkbuff * fkb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    BlogHash_t blogHash;
    BlogAction_t action = PKT_NORM;

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    BCM_ASSERT_HAS_SPINLOCK_R(&blog_lock_g, action);
#endif

    blogHash.match = 0U;     /* also clears hash, protocol = 0 */

    if ( unlikely(blog_rx_hook_g == (BlogDevHook_t)NULL) )
        goto bypass;

    blogHash.l1_tuple.channel = (uint8_t)channel;
    blogHash.l1_tuple.phy     = phyHdr;

    blog_assertr( (blogHash.l1_tuple.phyType < BLOG_MAXPHY), PKT_NORM);
    blog_print( "fkb<0x%08x:%x> pData<0x%08x> length<%d> dev<0x%08x>"
                " chnl<%u> %s PhyHdrLen<%u> key<0x%08x>",
                (int)fkb_p, _is_in_skb_tag_(fkb_p->flags),
                (int)fkb_p->data, fkb_p->len, (int)dev_p,
                channel, strBlogPhy[blogHash.l1_tuple.phyType],
                rfc2684HdrLength[blogHash.l1_tuple.phyLen],
                blogHash.match );

    action = blog_rx_hook_g( fkb_p, (void *)dev_p, encap, blogHash.match );

#if defined(CC_BLOG_SUPPORT_USER_FILTER)
    if ( action == PKT_BLOG )
        action = blog_filter(fkb_p->blog_p);
#endif

    if ( unlikely(action == PKT_NORM) )
        fkb_release( fkb_p );

bypass:
    return action;
}

BlogAction_t blog_finit( struct fkbuff * fkb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    BlogAction_t ret;

    BLOG_LOCK_BH();

    ret = blog_finit_locked(fkb_p, dev_p, encap, channel, phyHdr);

    BLOG_UNLOCK_BH();

    return ret;
}

/*
 * blog_sinit serves as a wrapper to blog_finit() by overlaying an fkb into a
 * skb and invoking blog_finit().
 */
BlogAction_t blog_sinit( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    struct fkbuff * fkb_p;
    BlogAction_t action = PKT_NORM;

    if ( unlikely(blog_rx_hook_g == (BlogDevHook_t)NULL) )
        goto bypass;

    blog_assertr( (BLOG_GET_PHYTYPE(phyHdr) < BLOG_MAXPHY), PKT_NORM );
    blog_print( "skb<0x%08x> pData<0x%08x> length<%d> dev<0x%08x>"
                " chnl<%u> %s PhyHdrLen<%u>",
                (int)skb_p, (int)skb_p->data, skb_p->len, (int)dev_p,
                channel, strBlogPhy[BLOG_GET_PHYTYPE(phyHdr)],
                rfc2684HdrLength[BLOG_GET_PHYLEN(phyHdr)] );

    /* CAUTION: Tag that the fkbuff is from sk_buff */
    fkb_p = (FkBuff_t *) &skb_p->fkbInSkb;
    fkb_p->flags = _set_in_skb_tag_(0); /* clear and set in_skb tag */

    action = blog_finit( fkb_p, dev_p, encap, channel, phyHdr );

    if ( action == PKT_BLOG )
    {
         blog_assertr( (fkb_p->blog_p != BLOG_NULL), PKT_NORM );
         fkb_p->blog_p->skb_p = skb_p;
         fkb_p->blog_p->rx.dev_p = (void *)dev_p;   /* Log device info */
    } 
    else
         fkb_p->blog_p = NULL;

bypass:
    return action;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_emit
 * Description  : This function may be inserted in a physical network device's
 *                hard_start_xmit function just before the packet data is
 *                extracted from the sk_buff and enqueued for DMA transfer.
 *
 *                This function invokes the transmit blog hook.
 * Parameters   :
 *  nbuff_p     : Pointer to a NBuff
 *  dev_p       : Pointer to the net_device on which the packet is transmited.
 *  encap       : First encapsulation type
 *  channel     : Channel/Port number on which the packet is transmited.
 *  phyHdr      : e.g. XTM device RFC2684 header type
 *
 * Returns      :
 *  PKT_DONE    : The skb_p is consumed and device should not process skb_p.
 *  PKT_NORM    : Device may use skb_p and proceed with hard xmit 
 *                Blog object is disassociated and freed.
 *------------------------------------------------------------------------------
 */
BlogAction_t _blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    BlogHash_t blogHash;
    struct sk_buff * skb_p;
    Blog_t * blog_p;
    BlogAction_t action = PKT_NORM;

    // outer inline function has already verified this is a skbuff
    skb_p = PNBUFF_2_SKBUFF(nbuff_p);   /* same as nbuff_p */

    blog_p = skb_p->blog_p;
    if ( blog_p == BLOG_NULL )
        goto bypass;

    blog_assertr( (_IS_BPTR_(blog_p)), PKT_NORM );

    blogHash.match = 0U;

    if ( likely(blog_tx_hook_g != (BlogDevHook_t)NULL) )
    {
        BLOG_LOCK_BH();

        blog_p->tx.dev_p = (void *)dev_p;           /* Log device info */

        blogHash.l1_tuple.channel = (uint8_t)channel;
        blogHash.l1_tuple.phy     = phyHdr;

        blog_p->priority = skb_p->priority;         /* Log skb info */
        blog_p->mark = skb_p->mark;

        blog_assertr( (BLOG_GET_PHYTYPE(phyHdr) < BLOG_MAXPHY), PKT_NORM);
        blog_print( "skb<0x%08x> blog<0x%08x> pData<0x%08x> length<%d>"
                    " dev<0x%08x> chnl<%u> %s PhyHdrLen<%u> key<0x%08x>",
            (int)skb_p, (int)blog_p, (int)skb_p->data, skb_p->len,
            (int)dev_p, channel, strBlogPhy[BLOG_GET_PHYTYPE(phyHdr)],
            rfc2684HdrLength[BLOG_GET_PHYLEN(phyHdr)],
            blogHash.match );

        action = blog_tx_hook_g( skb_p, (void*)skb_p->dev,
                                 encap, blogHash.match );

        BLOG_UNLOCK_BH();
    }
    blog_free( skb_p );                             /* Dis-associate w/ skb */

bypass:
    return action;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_activate
 * Description  : This function is a static configuration function of blog
 *                application. It invokes blog configuration hook
 * Parameters   :
 *  blog_p      : pointer to a blog with configuration information
 *  traffic     : type of the traffic
 *
 * Returns      :
 *  ActivateKey : If the configuration is successful, a key is returned.
 *                Otherwise, BLOG_KEY_INVALID is returned
 *------------------------------------------------------------------------------
 */
uint32_t blog_activate( Blog_t * blog_p, BlogTraffic_t traffic )
{
    uint32_t key;

    key = BLOG_KEY_INVALID;

    if ( unlikely(blog_sc_hook_g == (BlogScHook_t)NULL) )
        goto bypass;

#if defined(CC_BLOG_SUPPORT_DEBUG)
    blog_print( "traffic<%u> blog_p<0x%08x>", traffic, (int)blog_p );
    blog_dump( blog_p );
#endif

    if ( (blog_p == BLOG_NULL) || (traffic >= BlogTraffic_MAX) )
    {
        blog_assertr( ( blog_p != BLOG_NULL ), key );
        goto bypass;
    }

    BLOG_LOCK_BH();
    key = blog_sc_hook_g( blog_p, traffic );
    BLOG_UNLOCK_BH();

bypass:
    return key;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_deactivate
 * Description  : This function is a deconfiguration function of blog
 *                application
 * Parameters   :
 *  key         : blog key information
 *  traffic     : type of traffic
 *
 * Returns      :
 *  blog_p      : If the deconfiguration is successful, the associated blog 
 *                pointer is returned to the caller
 *------------------------------------------------------------------------------
 */
Blog_t * blog_deactivate( uint32_t key, BlogTraffic_t traffic )
{
    Blog_t * blog_p = NULL;

    if ( unlikely(blog_sd_hook_g == (BlogSdHook_t)NULL) )
        goto bypass;

    blog_print( "key<%08x> traffic<%u>", key, traffic );

    if ( key == BLOG_KEY_INVALID )
    {
        blog_assertr( (key != BLOG_KEY_INVALID), blog_p );
        goto bypass;
    }

    BLOG_LOCK_BH();
    blog_p = blog_sd_hook_g( key, traffic );
    BLOG_UNLOCK_BH();

#if defined(CC_BLOG_SUPPORT_DEBUG)
    blog_dump( blog_p );
#endif

bypass:
    return blog_p;
}


/*
 * blog_iq_prio determines the Ingress QoS priority of the packet
 */
int blog_iq_prio( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
{
    struct fkbuff * fkb_p;
    BlogAction_t action = PKT_NORM;
    int iq_prio = 1;
    uint32_t dummy;
    void *dummy_dev_p = &dummy;

    if ( unlikely(blog_rx_hook_g == (BlogDevHook_t)NULL) )
        goto bypass;

    blog_assertr( (BLOG_GET_PHYTYPE(phyHdr) < BLOG_MAXPHY), 1 );
    blog_print( "skb<0x%08x> pData<0x%08x> length<%d> dev<0x%08x>"
                " chnl<%u> %s PhyHdrLen<%u>",
                (int)skb_p, (int)skb_p->data, skb_p->len, (int)dev_p,
                channel, strBlogPhy[BLOG_GET_PHYTYPE(phyHdr)],
                rfc2684HdrLength[BLOG_GET_PHYLEN(phyHdr)] );

    /* CAUTION: Tag that the fkbuff is from sk_buff */
    fkb_p = (FkBuff_t *) &skb_p->fkbInSkb;

    /* set in_skb and chk_iq_prio tag */
    fkb_p->flags = _set_in_skb_n_chk_iq_prio_tag_(0); 
    action = blog_finit( fkb_p, dummy_dev_p, encap, channel, phyHdr );

    if ( action == PKT_BLOG )
    {
         blog_assertr( (fkb_p->blog_p != BLOG_NULL), PKT_NORM );
         fkb_p->blog_p->skb_p = skb_p;
         iq_prio = fkb_p->blog_p->iq_prio;
         blog_free( skb_p );
    } 
    else
         fkb_p->blog_p = NULL;

bypass:
    return iq_prio;
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_bind
 * Description  : Override default rx and tx hooks.
 *  blog_rx     : Function pointer to be invoked in blog_finit(), blog_sinit()
 *  blog_tx     : Function pointer to be invoked in blog_emit()
 *  blog_xx     : Function pointer to be invoked in blog_notify()
 *  blog_sc     : Function pointer to be invoked in blog_activate()
 *  blog_sd     : Function pointer to be invoked in blog_deactivate()
 *  info        : Mask of the function pointers for configuration
 *------------------------------------------------------------------------------
 */
void blog_bind( BlogDevHook_t blog_rx, BlogDevHook_t blog_tx,
                BlogNotifyHook_t blog_xx, BlogScHook_t blog_sc,
                BlogSdHook_t blog_sd, 
                BlogBind_t bind)
{
    blog_print( "Bind Rx[<%08x>] Tx[<%08x>] Notify[<%08x>]" 
                "Sc[<%08x>] Sd[<%08x>] bind[<%u>]",
                (int)blog_rx, (int)blog_tx, (int)blog_xx,
                (int)blog_sc, (int)blog_sd, (uint8_t)bind.hook_info );

    if ( bind.bmap.RX_HOOK )
        blog_rx_hook_g = blog_rx;   /* Receive  hook */
    if ( bind.bmap.TX_HOOK )
        blog_tx_hook_g = blog_tx;   /* Transmit hook */
    if ( bind.bmap.XX_HOOK )
        blog_xx_hook_g = blog_xx;   /* Notify hook */
    if ( bind.bmap.SC_HOOK )
        blog_sc_hook_g = blog_sc;   /* Static config hook */
    if ( bind.bmap.SD_HOOK )
        blog_sd_hook_g = blog_sd;   /* Static deconf hook */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog
 * Description  : Log the L2 or L3+4 tuple information
 * Parameters   :
 *  skb_p       : Pointer to the sk_buff
 *  dir         : rx or tx path
 *  encap       : Encapsulation type
 *  len         : Length of header
 *  data_p      : Pointer to encapsulation header data.
 *------------------------------------------------------------------------------
 */
void blog( struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,
           size_t len, void * data_p )
{
    BlogHeader_t * bHdr_p;
    Blog_t * blog_p;

    blog_assertv( (skb_p != (struct sk_buff *)NULL ) );
    blog_assertv( (skb_p->blog_p != BLOG_NULL) );
    blog_assertv( (_IS_BPTR_(skb_p->blog_p)) );
    blog_assertv( (data_p != (void *)NULL ) );
    blog_assertv( (len <= BLOG_HDRSZ_MAX) );
    blog_assertv( (encap < PROTO_MAX) );

    blog_p = skb_p->blog_p;
    blog_assertv( (blog_p->skb_p == skb_p) );

    bHdr_p = &blog_p->rx + dir;

    if ( encap == L3_IPv4 )    /* Log the IP Tuple */
    {
        BlogTuple_t * bTuple_p = &bHdr_p->tuple;
        BlogIpv4Hdr_t * ip_p   = (BlogIpv4Hdr_t *)data_p;

        /* Discontinue if non IPv4 or with IP options, or fragmented */
        if ( (ip_p->ver != 4) || (ip_p->ihl != 5)
             || (ip_p->flagsFrag & htons(BLOG_IP_FRAG_OFFSET|BLOG_IP_FLAG_MF)) )
            goto skip;

        if ( ip_p->proto == BLOG_IPPROTO_TCP )
        {
            BlogTcpHdr_t * th_p;
            th_p = (BlogTcpHdr_t*)( (uint8_t *)ip_p + BLOG_IPV4_HDR_LEN );

            /* Discontinue if TCP RST/FIN */
            if ( TCPH_RST(th_p) | TCPH_FIN(th_p) )
                goto skip;
            bTuple_p->port.source = th_p->sPort;
            bTuple_p->port.dest = th_p->dPort;
        }
        else if ( ip_p->proto == BLOG_IPPROTO_UDP )
        {
            BlogUdpHdr_t * uh_p;
            uh_p = (BlogUdpHdr_t *)( (uint8_t *)ip_p + BLOG_UDP_HDR_LEN );
            bTuple_p->port.source = uh_p->sPort;
            bTuple_p->port.dest = uh_p->dPort;
        }
        else
            goto skip;  /* Discontinue if non TCP or UDP upper layer protocol */

        bTuple_p->ttl = ip_p->ttl;
        bTuple_p->tos = ip_p->tos;
        bTuple_p->check = ip_p->chkSum;
        bTuple_p->saddr = blog_read32_align16( (uint16_t *)&ip_p->sAddr );
        bTuple_p->daddr = blog_read32_align16( (uint16_t *)&ip_p->dAddr );
        blog_p->key.protocol = ip_p->proto;
    }
    else if ( encap == L3_IPv6 )    /* Log the IPv6 Tuple */
    {
        printk("FIXME blog encap L3_IPv6 \n");
    }
    else    /* L2 encapsulation */
    {
        register short int * d;
        register const short int * s;

        blog_assertv( (bHdr_p->info.count < BLOG_ENCAP_MAX) );
        blog_assertv( ((len<=20) && ((len & 0x1)==0)) );
        blog_assertv( ((bHdr_p->length + len) < BLOG_HDRSZ_MAX) );

        bHdr_p->info.hdrs |= (1U << encap);
        bHdr_p->encap[ bHdr_p->info.count++ ] = encap;
        s = (const short int *)data_p;
        d = (short int *)&(bHdr_p->l2hdr[bHdr_p->length]);
        bHdr_p->length += len;

        switch ( len ) /* common lengths, using half word alignment copy */
        {
            case 20: *(d+9)=*(s+9);
                     *(d+8)=*(s+8);
                     *(d+7)=*(s+7);
            case 14: *(d+6)=*(s+6);
            case 12: *(d+5)=*(s+5);
            case 10: *(d+4)=*(s+4);
            case  8: *(d+3)=*(s+3);
            case  6: *(d+2)=*(s+2);
            case  4: *(d+1)=*(s+1);
            case  2: *(d+0)=*(s+0);
                 break;
            default:
                 goto skip;
        }
    }

    return;

skip:   /* Discontinue further logging by dis-associating Blog_t object */

    blog_skip( skb_p );

    /* DO NOT ACCESS blog_p !!! */
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_nfct_dump
 * Description  : Dump the nf_conn context
 *  dev_p       : Pointer to a net_device object
 * CAUTION      : nf_conn is not held !!!
 *------------------------------------------------------------------------------
 */
void blog_nfct_dump( struct sk_buff * skb_p, struct nf_conn * ct, uint32_t dir )
{
#if defined(BLOG_NF_CONNTRACK)
    struct nf_conn_help *help_p;
    struct nf_conn_nat  *nat_p;
    int bitix;
    if ( ct == NULL )
    {
        blog_error( "NULL NFCT error" );
        return;
    }

#ifdef CONFIG_NF_NAT_NEEDED
    nat_p = nfct_nat(ct);
#else
    nat_p = (struct nf_conn_nat *)NULL;
#endif

    help_p = nfct_help(ct);
    printk("\tNFCT: ct<0x%p>, info<%x> master<0x%p>\n"
           "\t\tF_NAT<%p> keys[%u %u] dir<%s>\n"
           "\t\thelp<0x%p> helper<%s>\n",
            ct, 
            (int)skb_p->nfctinfo, 
            ct->master,
            nat_p, 
            ct->blog_key[IP_CT_DIR_ORIGINAL], 
            ct->blog_key[IP_CT_DIR_REPLY],
            (dir<IP_CT_DIR_MAX)?strIpctDir[dir]:strIpctDir[IP_CT_DIR_MAX],
            help_p,
            (help_p && help_p->helper) ? help_p->helper->name : "NONE" );

    printk( "\t\tSTATUS[ " );
    for ( bitix = 0; bitix <= IPS_BLOG_BIT; bitix++ )
        if ( ct->status & (1 << bitix) )
            printk( "%s ", strIpctStatus[bitix] );
    printk( "]\n" );
#endif /* defined(BLOG_NF_CONNTRACK) */
}


/*
 *------------------------------------------------------------------------------
 * Function     : blog_netdev_dump
 * Description  : Dump the contents of a net_device object.
 *  dev_p       : Pointer to a net_device object
 *
 * CAUTION      : Net device is not held !!!
 *
 *------------------------------------------------------------------------------
 */
static void blog_netdev_dump( struct net_device * dev_p )
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    int i;
    printk( "\tDEVICE: %s dev<0x%08x> ndo_start_xmit[<0x%08x>]\n"
            "\t  dev_addr[ ", dev_p->name,
            (int)dev_p, (int)dev_p->netdev_ops->ndo_start_xmit );
    for ( i=0; i<dev_p->addr_len; i++ )
        printk( "%02x ", *((uint8_t *)(dev_p->dev_addr) + i) );
    printk( "]\n" );
#else
    int i;
    printk( "\tDEVICE: %s dev<0x%08x>: poll[<%08x>] hard_start_xmit[<%08x>]\n"
            "\t  hard_header[<%08x>] hard_header_cache[<%08x>]\n"
            "\t  dev_addr[ ", dev_p->name,
            (int)dev_p, (int)dev_p->poll, (int)dev_p->hard_start_xmit,
            (int)dev_p->hard_header, (int)dev_p->hard_header_cache );
    for ( i=0; i<dev_p->addr_len; i++ )
        printk( "%02x ", *((uint8_t *)(dev_p->dev_addr) + i) );
    printk( "]\n" );
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) */
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_tuple_dump
 * Description  : Dump the contents of a BlogTuple_t object.
 *  bTuple_p    : Pointer to the BlogTuple_t object
 *------------------------------------------------------------------------------
 */
static void blog_tuple_dump( BlogTuple_t * bTuple_p )
{
    printk( "\tIPv4:\n"
            "\t\tSrc" BLOG_IPV4_ADDR_PORT_FMT
             " Dst" BLOG_IPV4_ADDR_PORT_FMT "\n"
            "\t\tttl<%3u> tos<%3u> check<0x%04x>\n",
            BLOG_IPV4_ADDR(bTuple_p->saddr), bTuple_p->port.source,
            BLOG_IPV4_ADDR(bTuple_p->daddr), bTuple_p->port.dest,
            bTuple_p->ttl, bTuple_p->tos, bTuple_p->check );
}
 
/*
 *------------------------------------------------------------------------------
 * Function     : blog_tupleV6_dump
 * Description  : Dump the contents of a BlogTupleV6_t object.
 *  bTupleV6_p    : Pointer to the BlogTupleV6_t object
 *------------------------------------------------------------------------------
 */
static void blog_tupleV6_dump( BlogTupleV6_t * bTupleV6_p )
{
    printk( "\tIPv6:\n"
            "\t\tSrc" BLOG_IPV6_ADDR_PORT_FMT "\n"
            "\t\tDst" BLOG_IPV6_ADDR_PORT_FMT "\n"
            "\t\thop_limit<%3u>\n",
            BLOG_IPV6_ADDR(bTupleV6_p->saddr), bTupleV6_p->port.source,
            BLOG_IPV6_ADDR(bTupleV6_p->daddr), bTupleV6_p->port.dest,
            bTupleV6_p->hop_limit );
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_l2_dump
 * Description  : parse and dump the contents of all L2 headers
 *  bHdr_p      : Pointer to logged header
 *------------------------------------------------------------------------------
 */
void blog_l2_dump( BlogHeader_t * bHdr_p )
{
    register int i, ix, length, offset = 0;
    BlogEncap_t type;
    char * value = bHdr_p->l2hdr;

    for ( ix=0; ix<bHdr_p->info.count; ix++ )
    {
        type = bHdr_p->encap[ix];

        switch ( type )
        {
            case PPP_1661   : length = BLOG_PPP_HDR_LEN;    break;
            case PPPoE_2516 : length = BLOG_PPPOE_HDR_LEN;  break;
            case VLAN_8021Q : length = BLOG_VLAN_HDR_LEN;   break;
            case ETH_802x   : length = BLOG_ETH_HDR_LEN;    break;
            case BCM_SWC    : 
                              if ( *((uint16_t *)(bHdr_p->l2hdr + 12) ) 
                                   == BLOG_ETH_P_BRCM4TAG)
                                  length = BLOG_BRCM4_HDR_LEN;
                              else
                                  length = BLOG_BRCM6_HDR_LEN;
                              break;

            case L3_IPv4    :
            case L3_IPv6    :
            case BCM_XPHY   :
            default         : printk( "Unsupported type %d\n", type );
                              return;
        }

        printk( "\tENCAP %d. %10s +%2d %2d [ ",
                ix, strBlogEncap[type], offset, length );

        for ( i=0; i<length; i++ )
            printk( "%02x ", (uint8_t)value[i] );

        offset += length;
        value += length;

        printk( "]\n" );
    }
}

void blog_virdev_dump( Blog_t * blog_p )
{
    int i;

    printk( " VirtDev: ");

    for (i=0; i<MAX_VIRT_DEV; i++)
        printk("<0x%08x> ", (int)blog_p->virt_dev_p[i]);

    printk("\n");
}

void blog_lock_bh(void)
{
    BLOG_LOCK_BH();
}

void blog_unlock_bh(void)
{
    BLOG_UNLOCK_BH();
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_dump
 * Description  : Dump the contents of a Blog object.
 *  blog_p      : Pointer to the Blog_t object
 *------------------------------------------------------------------------------
 */
void blog_dump( Blog_t * blog_p )
{
    if ( blog_p == BLOG_NULL )
        return;

    blog_assertv( (_IS_BPTR_(blog_p)) );

    printk( "BLOG <0x%08x> owner<0x%08x> v4ct<0x%08x> v6ct<0x%08x>\n"
            "\t\tL1 channel<%u> phyLen<%u> phy<%u> <%s>\n"
            "\t\tfdb_src<0x%08x> fdb_dst<0x%08x>\n"
            "\t\thash<%u> prot<%u> prio<0x%08x> mark<0x%08x> Mtu<%u>\n",
            (int)blog_p, (int)blog_p->skb_p, 
            (int)blog_p->ct_p[BLOG_PARAM2_IPV4],
            (int)blog_p->ct_p[BLOG_PARAM2_IPV6],
            blog_p->key.l1_tuple.channel,
            rfc2684HdrLength[blog_p->key.l1_tuple.phyLen],
            blog_p->key.l1_tuple.phy,
            strBlogPhy[blog_p->key.l1_tuple.phyType],
            (int)blog_p->fdb[0], (int)blog_p->fdb[1],
            blog_p->hash, blog_p->key.protocol,
            blog_p->priority, blog_p->mark, blog_p->minMtu);

    if ( blog_p->ct_p[BLOG_PARAM2_IPV4] )
        blog_nfct_dump( blog_p->skb_p, blog_p->ct_p[BLOG_PARAM2_IPV4], 
                        blog_p->nf_dir );

    if ( blog_p->ct_p[BLOG_PARAM2_IPV6] )
        blog_nfct_dump( blog_p->skb_p, blog_p->ct_p[BLOG_PARAM2_IPV6], 
                        blog_p->nf_dir );

    printk( "  RX count<%u> channel<%02u> bmap<0x%x> phyLen<%u> phyHdr<%u> %s\n"
            "     wan_qdisc<%u> multicast<%u> fkbInSkb<%u>\n",
            blog_p->rx.info.count, blog_p->rx.info.channel,
            blog_p->rx.info.hdrs,
            rfc2684HdrLength[blog_p->rx.info.phyHdrLen],
            blog_p->rx.info.phyHdr, 
            strBlogPhy[blog_p->rx.info.phyHdrType],
            blog_p->rx.info.wan_qdisc,
            blog_p->rx.info.multicast, blog_p->rx.info.fkbInSkb );
    if ( blog_p->rx.info.bmap.L3_IPv4 )
        blog_tuple_dump( &blog_p->rx.tuple );
    blog_l2_dump( &blog_p->rx );

    printk("  TX count<%u> channel<%02u> bmap<0x%x> phyLen<%u> phyHdr<%u> %s\n",
            blog_p->tx.info.count, blog_p->tx.info.channel,
            blog_p->tx.info.hdrs, 
            rfc2684HdrLength[blog_p->tx.info.phyHdrLen],
            blog_p->tx.info.phyHdr, 
            strBlogPhy[blog_p->tx.info.phyHdrType] );
    if ( blog_p->tx.dev_p )
        blog_netdev_dump( blog_p->tx.dev_p );
    if ( blog_p->rx.info.bmap.L3_IPv4 )
        blog_tuple_dump( &blog_p->tx.tuple );
    blog_l2_dump( &blog_p->tx );
    blog_virdev_dump( blog_p );

    if ( blog_p->rx.info.bmap.L3_IPv6 )
        blog_tupleV6_dump( &blog_p->tupleV6 );

#if defined(CC_BLOG_SUPPORT_DEBUG)
    printk( "\t\textends<%d> free<%d> used<%d> HWM<%d> fails<%d>\n",
            blog_extends, blog_cnt_free, blog_cnt_used, blog_cnt_hwm,
            blog_cnt_fails );
#endif

}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_getTxMtu
 * Description  : Gets unadjusted mtu from tx network devices associated with blog.
 *  blog_p      : Pointer to the Blog_t object
 *------------------------------------------------------------------------------
 */
uint16_t blog_getTxMtu(Blog_t * blog_p)
{
    int     i;
    uint16_t  minMtu;
    void *  dir_dev_p; 
    struct net_device *  dev_p;

    dev_p = (struct net_device *)blog_p->tx.dev_p;
    if (dev_p)
        minMtu = dev_p->mtu;
    else
        minMtu = 0xFFFF;
    
    
    for (i = 0; i < MAX_VIRT_DEV; i++)
    {
        dir_dev_p = blog_p->virt_dev_p[i];
        if ( dir_dev_p == (void *)NULL ) 
            continue;
        if ( IS_RX_DIR(dir_dev_p) )
            continue;
        dev_p = (struct net_device *)DEVP_DETACH_DIR(dir_dev_p);
        if (dev_p && dev_p->mtu < minMtu)
        {
            minMtu = dev_p->mtu;
        }
    }
    
    blog_print( "minMtu <%d>", (int)minMtu );

    return minMtu;
}


/*
 *------------------------------------------------------------------------------
 * Function     : __init_blog
 * Description  : Incarnates the blog system during kernel boot sequence,
 *                in phase subsys_initcall()
 *------------------------------------------------------------------------------
 */
static int __init __init_blog( void )
{
    nfskb_p = alloc_skb( 0, GFP_ATOMIC );
    blog_refresh_fn = (blog_refresh_t)NULL;
    blog_extend( BLOG_POOL_SIZE_ENGG ); /* Build preallocated pool */
    BLOG_DBG( printk( CLRb "BLOG blog_dbg<0x%08x> = %d\n"
                           "%d Blogs allocated of size %d" CLRnl,
                           (int)&blog_dbg, blog_dbg,
                           BLOG_POOL_SIZE_ENGG, sizeof(Blog_t) ););
    printk( CLRb "BLOG %s Initialized" CLRnl, BLOG_VERSION );
    return 0;
}

subsys_initcall(__init_blog);

EXPORT_SYMBOL(_blog_emit);
EXPORT_SYMBOL(blog_extend);

EXPORT_SYMBOL(strBlogAction);
EXPORT_SYMBOL(strBlogEncap);

EXPORT_SYMBOL(strRfc2684);
EXPORT_SYMBOL(rfc2684HdrLength);
EXPORT_SYMBOL(rfc2684HdrData);

#else   /* !defined(CONFIG_BLOG) */

int blog_dbg = 0;
int blog_support_mcast_g = BLOG_MCAST_DISABLE; /* = CC_BLOG_SUPPORT_MCAST; */
void blog_support_mcast(int enable) {blog_support_mcast_g = BLOG_MCAST_DISABLE;}

int blog_support_ipv6_g = BLOG_IPV6_DISABLE; /* = CC_BLOG_SUPPORT_IPV6; */
void blog_support_ipv6(int enable) {blog_support_ipv6_g = BLOG_IPV6_DISABLE;}

blog_refresh_t blog_refresh_fn = (blog_refresh_t) NULL;

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)   
spinlock_t blog_lock_g = SPIN_LOCK_UNLOCKED;      /* blogged packet flow */
#endif

/* Stub functions for Blog APIs that may be used by modules */
Blog_t * blog_get( void ) { return BLOG_NULL; }
void     blog_put( Blog_t * blog_p ) { return; }

Blog_t * blog_skb( struct sk_buff * skb_p) { return BLOG_NULL; }
Blog_t * blog_fkb( struct fkbuff * fkb_p ) { return BLOG_NULL; }

Blog_t * blog_snull( struct sk_buff * skb_p ) { return BLOG_NULL; }
Blog_t * blog_fnull( struct fkbuff * fkb_p ) { return BLOG_NULL; }

void     blog_free( struct sk_buff * skb_p ) { return; }

void     blog_skip( struct sk_buff * skb_p ) { return; }
void     blog_xfer( struct sk_buff * skb_p, const struct sk_buff * prev_p )
         { return; }
void     blog_clone( struct sk_buff * skb_p, const struct blog_t * prev_p )
         { return; }
void     blog_copy(struct blog_t * new_p, const struct blog_t * prev_p)
         { return; }

void     blog_link( BlogNetEntity_t entity_type, Blog_t * blog_p,
                    void * net_p, uint32_t param1, uint32_t param2 ) { return; }

void     blog_notify( BlogNotify_t event, void * net_p,
                      uint32_t param1, uint32_t param2 ) { return; }

uint32_t blog_request( BlogRequest_t event, void * net_p,
                       uint32_t param1, uint32_t param2 ) { return 0; }

BlogAction_t blog_filter( Blog_t * blog_p )
         { return PKT_NORM; }

BlogAction_t blog_sinit( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

BlogAction_t blog_finit( struct fkbuff * fkb_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

BlogAction_t blog_finit_locked( struct fkbuff * fkb_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

BlogAction_t blog_emit( void * nbuff_p, void * dev_p,
                        uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return PKT_NORM; }

int blog_iq_prio( struct sk_buff * skb_p, void * dev_p,
                         uint32_t encap, uint32_t channel, uint32_t phyHdr )
         { return 1; }

void blog_bind( BlogDevHook_t blog_rx, BlogDevHook_t blog_tx,
                BlogNotifyHook_t blog_xx, BlogScHook_t blog_sc,
                BlogSdHook_t blog_sd, BlogBind_t bind ) { return; }

void     blog( struct sk_buff * skb_p, BlogDir_t dir, BlogEncap_t encap,
               size_t len, void * data_p ) { return; }

void     blog_dump( Blog_t * blog_p ) { return; }

void     blog_lock_bh(void) {return; }

void     blog_unlock_bh(void) {return; }

uint16_t   blog_getTxMtu(Blog_t * blog_p) {return 0;}


EXPORT_SYMBOL(blog_emit);

#endif  /* else !defined(CONFIG_BLOG) */

EXPORT_SYMBOL(blog_dbg);
EXPORT_SYMBOL(blog_support_mcast_g);
EXPORT_SYMBOL(blog_support_mcast);
EXPORT_SYMBOL(blog_support_ipv6_g);
EXPORT_SYMBOL(blog_support_ipv6);
EXPORT_SYMBOL(blog_refresh_fn);

EXPORT_SYMBOL(blog_get);
EXPORT_SYMBOL(blog_put);
EXPORT_SYMBOL(blog_skb);
EXPORT_SYMBOL(blog_fkb);
EXPORT_SYMBOL(blog_snull);
EXPORT_SYMBOL(blog_fnull);
EXPORT_SYMBOL(blog_free);
EXPORT_SYMBOL(blog_dump);
EXPORT_SYMBOL(blog_skip);
EXPORT_SYMBOL(blog_xfer);
EXPORT_SYMBOL(blog_clone);
EXPORT_SYMBOL(blog_copy);
EXPORT_SYMBOL(blog_link);
EXPORT_SYMBOL(blog_notify);
EXPORT_SYMBOL(blog_request);
EXPORT_SYMBOL(blog_filter);
EXPORT_SYMBOL(blog_sinit);
EXPORT_SYMBOL(blog_finit);
EXPORT_SYMBOL(blog_finit_locked);
EXPORT_SYMBOL(blog_lock_bh);
EXPORT_SYMBOL(blog_unlock_bh);
EXPORT_SYMBOL(blog_bind);
EXPORT_SYMBOL(blog_iq_prio);
EXPORT_SYMBOL(blog_getTxMtu);

EXPORT_SYMBOL(blog);
