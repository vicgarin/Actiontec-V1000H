#ifndef __NBUFF_H_INCLUDED__
#define __NBUFF_H_INCLUDED__

/*
<:copyright-gpl

 Copyright 2009 Broadcom Corp. All Rights Reserved.

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
 *
 * File Name  : nbuff.h
 * Description: Definition of a network buffer to support various forms of
 *      network buffer, to include Linux socket buff (SKB), lightweight
 *      fast kernel buff (FKB), BRCM Free Pool buffer (FPB), and traffic
 *      generator support buffer (TGB)
 *
 *      nbuff.h may also be used to provide an interface to common APIs 
 *      available on other OS (in particular BSD style mbuf).
 *
 * Common APIs provided: pushing, pulling, reading, writing, cloning, freeing
 *
 * Implementation Note:
 *
 * One may view NBuff as a base class from which other buff types are derived.
 * Examples of derived network buffer types are sk_buff, fkbuff, fpbuff, tgbuff
 *
 * A pointer to a buffer is converted to a pointer to a special (derived) 
 * network buffer type by encoding the type into the least significant 2 bits
 * of a word aligned buffer pointer. pBuf points to the real network 
 * buffer and pNBuff refers to pBuf ANDed with the Network Buffer Type.
 * C++ this pointer to a virtual class (vtable based virtual function thunks).
 *
 * Thunk functions to redirect the calls to the appropriate buffer type, e.g.
 * SKB or FKB uses the Network Buffer Pointer type information.
 *
 * This file also implements the Fast Kernel Buffer API. The fast kernel buffer
 * carries a minimal context of the received buffer and associated buffer
 * recycling information.
 *
 ******************************************************************************* */

#include <linux/autoconf.h>
#include <linux/types.h>            /* include ISO C99 inttypes.h             */
#include <linux/skbuff.h>           /* include corresponding BSD style mbuf   */
#include <linux/blog.h>

#define NBUFF_VERSION              "v1.0"

/* Engineering Constants for Fast Kernel Buffer Global Pool (used for clones) */
#define SUPPORT_FKB_EXTEND
#define FKBC_POOL_SIZE_ENGG         800
#define FKBC_EXTEND_SIZE_ENGG       32      /* Number of FkBuf_t per extension*/
#define FKBC_EXTEND_MAX_ENGG        16      /* Maximum extensions allowed     */

#define FKBM_POOL_SIZE_ENGG         128
#define FKBM_EXTEND_SIZE_ENGG       2
#define FKBM_EXTEND_MAX_ENGG        200     /* Assuming one unshare           */

/*
 * Network device drivers ported to NBUFF must ensure that the headroom is at
 * least 176 bytes in size. Remove this dependancy (TBD).
 */
// #define CC_FKB_HEADROOM_AUDIT
#define FKB_HEADROOM                ((176 + 0x0F) & ~0x0F)
#define FKB_XLATE_SKB_HEADROOM      FKB_HEADROOM
#define FKB_XLATE_SKB_TAILROOM      32

    /* FkBuff + headroom + buffer + tailroom */
#define FKBM_BUFFER_SIZE            2048

/* Conditional compile of FKB functional APIs as inlined or non-inlined */
#define CC_CONFIG_FKB_FN_INLINE
#ifdef CC_CONFIG_FKB_FN_INLINE
#define FKB_FN(fn_name, fn_signature, body)                                    \
static inline fn_signature { body; }    /* APIs inlined in header file */
#else
#ifdef FKB_IMPLEMENTATION_FILE
#define FKB_FN(fn_name, fn_signature, body)                                    \
fn_signature { body; }                                                         \
EXPORT_SYMBOL(fn_name);                 /* APIs declared in implementation */
#else
#define FKB_FN(fn_name, fn_signature, body)                                    \
extern fn_signature;
#endif  /* !defined(FKB_IMPLEMENTATION_FILE) */
#endif  /* !defined(FKB_FN) */

/* LAB ONLY: Design development */
// #define CC_CONFIG_FKB_STATS
// #define CC_CONFIG_FKB_COLOR
// #define CC_CONFIG_FKB_DEBUG
// #define CC_CONFIG_FKB_AUDIT
// #define CC_CONFIG_FKB_STACK

// #include <linux/smp.h>       /* smp_processor_id() CC_CONFIG_FKB_AUDIT */

#if defined(CC_CONFIG_FKB_STATS)
#define FKB_STATS(stats_code)   do { stats_code } while(0)
#else
#define FKB_STATS(stats_code)   NULL_STMT
#endif

#if defined(CC_CONFIG_FKB_STACK)
extern void dump_stack(void);
#define DUMP_STACK()            dump_stack()
#else
#define DUMP_STACK()            NULL_STMT
#endif

#if defined(CC_CONFIG_FKB_AUDIT)
#define FKB_AUDIT(audit_code)   do { audit_code } while(0)
#else
#define FKB_AUDIT(audit_code)   NULL_STMT
#endif

extern int nbuff_dbg;
#if defined(CC_CONFIG_FKB_DEBUG)
#define fkb_dbg(lvl, fmt, arg...) \
    if (nbuff_dbg >= lvl) printk( "FKB %s :" fmt "[<%08x>]\n", \
        __FUNCTION__, ##arg, (int)__builtin_return_address(0) )
#define FKB_DBG(debug_code)     do { debug_code } while(0)
#else
#define fkb_dbg(lvl, fmt, arg...)      do {} while(0)
#define FKB_DBG(debug_code)     NULL_STMT
#endif

#define CC_NBUFF_FLUSH_OPTIMIZATION

/* OS Specific Section Begin */
#if defined(__KERNEL__)     /* Linux MIPS Cache Specific */
/*
 *------------------------------------------------------------------------------
 * common cache operations:
 *
 * - addr is rounded down to the cache line
 * - end is rounded up to cache line.
 *
 * - if ((addr == end) and (addr was cache aligned before rounding))
 *       no operation is performed.
 *   else
 *       flush data cache line UPTO but NOT INCLUDING rounded up end.
 *
 * Note:
 * if before rounding, (addr == end)  AND addr was not cache aligned,
 *      we would flush at least one line.
 *
 * Uses: L1_CACHE_BYTES
 *------------------------------------------------------------------------------
 */
#include <asm/cache.h>
#include <asm/r4kcache.h>

/*
 * Macros to round down and up, an address to a cachealigned address
 */
#define ADDR_ALIGN_DN(addr, align)  ( (addr) & ~((align) - 1) )
#define ADDR_ALIGN_UP(addr, align)  ( ((addr) + (align) - 1) & ~((align) - 1) )

/*
 *------------------------------------------------------------------------------
 * Function   : cache_flush_region
 * Description: 
 * Writeback flush, then invalidate a region demarcated by addr to end.
 * Cache line following rounded up end is not flushed.
 *------------------------------------------------------------------------------
 */
static inline void cache_flush_region(void *addr, void *end)
{
    unsigned long a = ADDR_ALIGN_DN( (unsigned long)addr, L1_CACHE_BYTES );
    unsigned long e = ADDR_ALIGN_UP( (unsigned long)end, L1_CACHE_BYTES );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += L1_CACHE_BYTES;    /* next cache line base */
    }
}

/*
 *------------------------------------------------------------------------------
 * Function   : cache_flush_len
 * Description: 
 * Writeback flush, then invalidate a region given an address and a length.
 * The demarcation end is computed by applying length to address before
 * rounding down address. End is rounded up.
 * Cache line following rounded up end is not flushed.
 *------------------------------------------------------------------------------
 */
static inline void cache_flush_len(void *addr, int len)
{
    unsigned long a = ADDR_ALIGN_DN( (unsigned long)addr, L1_CACHE_BYTES );
    unsigned long e = ADDR_ALIGN_UP( ((unsigned long)addr + len),
                                     L1_CACHE_BYTES );
    while ( a < e )
    {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        a += L1_CACHE_BYTES;    /* next cache line base */
    }
}

/*
 *------------------------------------------------------------------------------
 * Function   : _is_kptr_
 * Description: Test whether a variable can be a pointer to a kernel space.
 *              This form of variable overloading may only be used for denoting
 *              pointers to kernel space or as a variable where the most
 *              significant nibble is unused.
 *              In 32bit Linux kernel, a pointer to a KSEG0, KSEG1, KSEG2 will
 *              have 0x8, 0xA or 0xC in the most significant nibble.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _is_kptr_(const void * vptr)
{
    return ( (uint32_t)vptr > 0x0FFFFFFF );
}
#endif  /* defined(__KERNEL__) Linux MIPS Cache Specific */
/* OS Specific Section End */


/*
 * For BSD style mbuf with FKB : 
 * generate nbuff.h by replacing "SKBUFF" to "BCMMBUF", and,
 * use custom arg1 and arg2 instead of mark and priority, respectively.
 */

struct sk_buff;
struct blog_t;
struct net_device;
typedef int (*HardStartXmitFuncP) (struct sk_buff *skb,
                                   struct net_device *dev);

struct fkbuff;
typedef struct fkbuff FkBuff_t;

#define FKB_NULL                    ((FkBuff_t *)NULL)

#include <linux/nbuff_types.h>

/*
 *------------------------------------------------------------------------------
 *
 * Pointer conversion between pBuf and pNBuff encoded buffer pointers
 * uint8_t * pBuf;
 * pNBuff_t  pNBuff;
 * ...
 * // overlays FKBUFF_PTR into pointer to build a virtual pNBuff_t
 * pNBuff = PBUF_2_PNBUFF(pBuf,FKBUFF_PTR);
 * ...
 * // extracts a real uint8_t * from a virtual pNBuff_t
 * pBuf = PNBUFF_2_PBUF(pNBuff);
 *
 *------------------------------------------------------------------------------
 */
#define PBUF_2_PNBUFF(pBuf,realType) \
            ( (pNBuff_t) ((uint32_t)(pBuf)   | (uint32_t)(realType)) )
#define PNBUFF_2_PBUF(pNBuff)       \
            ( (uint8_t*) ((uint32_t)(pNBuff) & (uint32_t)NBUFF_PTR_MASK) )

#if (MUST_BE_ZERO != 0)
#error  "Design assumption SKBUFF_PTR == 0"
#endif
#define PNBUFF_2_SKBUFF(pNBuff)     ((struct sk_buff *)(pNBuff))

#define SKBUFF_2_PNBUFF(skb)        ((pNBuff_t)(skb)) /* see MUST_BE_ZERO */
#define FKBUFF_2_PNBUFF(fkb)        PBUF_2_PNBUFF(fkb,FKBUFF_PTR)

/*
 *------------------------------------------------------------------------------
 *
 * Cast from/to virtual "pNBuff_t" to/from real typed pointers
 *
 *  pNBuff_t pNBuff2Skb, pNBuff2Fkb;    // "void *" with NBuffPtrType_t
 *  struct sk_buff * skb_p;
 *  struct fkbuff  * fkb_p;
 *  ...
 *  pNBuff2Skb = CAST_REAL_TO_VIRT_PNBUFF(skb_p,SKBUFF_PTR);
 *  pNBuff2Fkb = CAST_REAL_TO_VIRT_PNBUFF(fkb_p,FKBUFF_PTR);
 *  ...
 *  skb_p = CAST_VIRT_TO_REAL_PNBUFF(pNBuff2Skb, struct sk_buff *);
 *  fkb_p = CAST_VIRT_TO_REAL_PNBUFF(pNBuff2Fkb, struct fkbuff  *);
 * or,
 *  fkb_p = PNBUFF_2_FKBUFF(pNBuff2Fkb);  
 *------------------------------------------------------------------------------
 */

#define CAST_REAL_TO_VIRT_PNBUFF(pRealNBuff,realType) \
            ( (pNBuff_t) (PBUF_2_PNBUFF((pRealNBuff),(realType))) )

#define CAST_VIRT_TO_REAL_PNBUFF(pVirtNBuff,realType) \
            ( (realType) PNBUFF_2_PBUF(pVirtNBuff) )

#define PNBUFF_2_FKBUFF(pNBuff) CAST_VIRT_TO_REAL_PNBUFF((pNBuff),struct fkbuff*)

/*
 *------------------------------------------------------------------------------
 *  FKB: Fast Kernel Buffers placed directly into Rx DMA Buffer
 *  May be used ONLY for common APIs such as those available in BSD-Style mbuf
 *------------------------------------------------------------------------------
 */

struct fkbuff
{
    /* List pointer must be the first field */
    union {
        FkBuff_t  * list;           /* SLL of free FKBs for cloning           */
        FkBuff_t  * master_p;       /* Clone FKB to point to master FKB       */
        atomic_t  users;            /* (private) # of references to FKB       */
    };
    union {                         /* Use _is_kptr_ to determine if ptr      */
        union {
            void          *ptr;
            struct blog_t *blog_p;  /* Pointer to a blog                      */
            uint8_t       *dirty_p; /* Pointer to packet payload dirty incache*/
            uint32_t       flags;   /* Access all flags                       */
        };
        /*
         * First nibble denotes a pointer or flag usage.
         * Lowest two significant bits denote the type of pinter
         * Remaining 22 bits may be used as flags
         */
        struct {
            uint32_t   ptr_type : 8;/* Identifies whether pointer             */
            uint32_t   unused   :21;/* Future use for flags                   */
            uint32_t   in_skb   : 1;/* flag: FKB passed inside a SKB          */
            uint32_t   other_ptr: 1;/* future use, to override another pointer*/
            uint32_t   dptr_tag : 1;/* Pointer type is a dirty pointer        */
        };
    };
    uint8_t       * data;           /* Pointer to packet data                 */
    uint32_t      len;              /* Packet length                          */

    uint32_t      mark;             /* Custom arg1, e.g. tag or mark field    */
    uint32_t      priority;         /* Custom arg2, packet priority, tx info  */
    RecycleFuncP  recycle_hook;     /* Nbuff recycle handler                  */
    uint32_t      recycle_context;  /* Rx network device/channel or pool      */

} ____cacheline_aligned;   /* 2 cache lines wide */

/*
 *------------------------------------------------------------------------------
 * An fkbuff may be referred to as a:
 *  master - a pre-allocated rxBuffer, inplaced ahead of the headroom.
 *  cloned - allocated from a free pool of fkbuff and points to a master.
 *
 *  in_skb - when a FKB is passed as a member of a SKB structure.
 *------------------------------------------------------------------------------
 */
#define FKB_IN_SKB                  (1 << 2)    /* Bit#2 is in_skb */

/* Return flags with the in_skb tag set */
static inline uint32_t _set_in_skb_tag_(uint32_t flags)
{
    return (flags | FKB_IN_SKB);
}

/* Fetch the in_skb tag in flags */
static inline uint32_t _get_in_skb_tag_(uint32_t flags)
{
    return (flags & FKB_IN_SKB);
}

/* Determine whether the in_skb tag is set in flags */
static inline uint32_t _is_in_skb_tag_(uint32_t flags)
{
    return ( _get_in_skb_tag_(flags) ? 1 : 0 );
}

#define CHK_IQ_PRIO                  (1 << 3)    /* Bit#3 is check IQ Prio */

/* Return flags with the in_skb_tag and chk_iq_prio set */
static inline uint32_t _set_in_skb_n_chk_iq_prio_tag_(uint32_t flags)
{
    return (flags | FKB_IN_SKB | CHK_IQ_PRIO);
}

/* Return flags with the chk_iq_prio set */
static inline uint32_t _set_chk_iq_prio_tag_(uint32_t flags)
{
    return (flags | CHK_IQ_PRIO);
}

/* Fetch the chk_iq_prio tag in flags */
static inline uint32_t _get_chk_iq_prio_tag_(uint32_t flags)
{
    return (flags & CHK_IQ_PRIO);
}

/* Determine whether the chk_iq_prio tag is set in flags */
static inline uint32_t _is_chk_iq_prio_tag_(uint32_t flags)
{
    return ( _get_chk_iq_prio_tag_(flags) ? 1 : 0 );
}


/*
 *------------------------------------------------------------------------------
 * APIs to convert between a real kernel pointer and a dirty pointer.
 *------------------------------------------------------------------------------
 */

#define FKB_DPTR_TAG                (1 << 0)    /* Bit#0 is dptr_tag */

/* Test whether a pointer is a dirty pointer type */
static inline uint32_t is_dptr_tag_(uint8_t * ptr)
{
    return ( ( (uint32_t) ((uint32_t)ptr & FKB_DPTR_TAG) ) ? 1 : 0);
}

/* Encode a real kernel pointer to a dirty pointer type */
static inline uint8_t * _to_dptr_from_kptr_(uint8_t * kernel_ptr)
{
    if((uint32_t)(kernel_ptr) & FKB_DPTR_TAG)
        kernel_ptr++;
    /* Tag a kernel pointer's dirty_ptr bit, to denote a FKB dirty pointer */
    return ( (uint8_t*) ((uint32_t)(kernel_ptr) | FKB_DPTR_TAG) );
}

/* Decode a dirty pointer type into a real kernel pointer */
static inline uint8_t * _to_kptr_from_dptr_(uint8_t * dirty_ptr)
{
    FKB_AUDIT(
        if ( dirty_ptr && !is_dptr_tag_(dirty_ptr) )
            printk("FKB ASSERT %s !is_dptr_tag_(0x%08x)\n",
                   __FUNCTION__, (int)dirty_ptr); );

    /* Fetch kernel pointer from encoded FKB dirty_ptr,
       by clearing dirty_ptr bit */
    return ( (uint8_t*) ((uint32_t)(dirty_ptr) & (~FKB_DPTR_TAG)) );
}

#define FKB_OPTR_TAG                (1<<1)      /* Bit#1 other_ptr tag */

#define FKB_BLOG_TAG_MASK           (FKB_DPTR_TAG | FKB_OPTR_TAG)

/* Verify whether a FKB pointer is pointing to a Blog */
#define _IS_BPTR_(fkb_ptr) \
         ( _is_kptr_(fkb_ptr) && ! ((uint32_t)(fkb_ptr) & FKB_BLOG_TAG_MASK) )


/*
 *------------------------------------------------------------------------------
 *
 *                  Types of preallocated FKB pools
 * 
 *  - A Master FKB object contains memory for the rx buffer, with a FkBuff_t
 *    placed at the head of the buffer. A Master FKB object may serve to
 *    replenish a network devices receive ring, when packet buffers are not
 *    promptly recycled. A Master FKB may also be used for packet replication
 *    where in one of the transmitted packet replicas may need a unique
 *    modification distinct from other replicas. In such a case, the FKB must
 *    be first "unshared" by a deep packet buffer copy into a Master Fkb.
 *    A Free Pool of Master FKB objects is maintained. Master FKB may be
 *    alocated and recycled from this Master FKB Pool.
 *    The Master FKB Pool may also be used for replinishing a network device
 *    driver's rx buffer ring.
 *
 *  - A Cloned FKB object does not contain memory for the rx buffer.
 *    Used by fkb_clone, to create multiple references to a packet buffer.
 *    Multiple references to a packet buffer may be used for packet replication.
 *    A FKB allocated from the FKB Cloned Pool will have master_p pointing to
 *    a Master FKB and the recycle_hook member set to NULL.
 *
 *------------------------------------------------------------------------------
 */
typedef enum {
    FkbMasterPool_e = 0,
    FkbClonedPool_e = 1,
    FkbMaxPools_e
} FkbObject_t;

/*
 * Function   : _get_master_users_
 * Description: Given a pointer to a Master FKB, fetch the users count
 * Caution    : Does not check whether the FKB is a Master or not!
 */
static inline uint32_t _get_master_users_(FkBuff_t * fkbM_p)
{
    uint32_t users;
    users = atomic_read(&fkbM_p->users);

    FKB_AUDIT(
        if ( users == 0 )
            printk("FKB ASSERT cpu<%u> %s(0x%08x) users == 0, recycle<%pS>\n",
                   smp_processor_id(), __FUNCTION__,
                   (int)fkbM_p, fkbM_p->recycle_hook); );
    return users;
}

/*
 * Function   : _is_fkb_cloned_pool_
 * Description: Test whether an "allocated" FKB is from the FKB Cloned Pool.
 */
static inline uint32_t _is_fkb_cloned_pool_(FkBuff_t * fkb_p)
{
    if ( _is_kptr_(fkb_p->master_p)
         && (fkb_p->recycle_hook == (RecycleFuncP)NULL) )
    {
        FKB_AUDIT(
            /* ASSERT if the FKB is actually linked in a FKB pool */
            if ( _is_kptr_(fkb_p->master_p->list) )
            {
                printk("FKB ASSERT cpu<%u> %s :"
                       " _is_kptr_((0x%08x)->0x%08x->0x%08x)"
                       " master<0x%08x>.recycle<%pS>\n",
                       smp_processor_id(), __FUNCTION__, (int)fkb_p,
                       (int)fkb_p->master_p, (int)fkb_p->master_p->list,
                       (int)fkb_p->master_p,
                       fkb_p->master_p->recycle_hook);
            }
            /* ASSERT that Master FKB users count is greater than 0 */
            if ( _get_master_users_(fkb_p->master_p) == 0 )
            {
                printk("FKB ASSERT cpu<%u> %s :"
                       " _get_master_users_(0x%08x->0x%08x) == 0\n",
                       smp_processor_id(), __FUNCTION__,
                       (int)fkb_p, (int)fkb_p->master_p);
                return 0;
            } );

        return 1;   /* Allocated FKB is from the FKB Cloned Pool */
    }
    else
        return 0;
}

/*
 * Function   : _get_fkb_users_
 * Description: Given a pointer to a FKB (Master or Cloned), fetch users count
 */
static inline uint32_t _get_fkb_users_(FkBuff_t * fkb_p)
{
    if ( _is_kptr_(fkb_p->master_p) )       /* Cloned FKB */
    {
        FKB_AUDIT(
            if ( !_is_fkb_cloned_pool_(fkb_p) ) /* double check Cloned FKB */
            {
                printk("FKB ASSERT cpu<%u> %s :"
                       " !_is_fkb_cloned_pool_(0x%08x)"
                       " master<0x%08x>.recycle<%pS>\n",
                       smp_processor_id(), __FUNCTION__,
                       (int)fkb_p, (int)fkb_p->master_p,
                       fkb_p->master_p->recycle_hook);
                return 0;
            } );

        return _get_master_users_(fkb_p->master_p);
    }
    else                                    /* Master FKB */
        return _get_master_users_(fkb_p);
}

/*
 * Function   : _get_fkb_master_ptr_
 * Description: Fetch the pointer to the Master FKB.
 */
static inline FkBuff_t * _get_fkb_master_ptr_(FkBuff_t * fkb_p)
{
    if ( _is_kptr_(fkb_p->master_p) )       /* Cloned FKB */
    {
        FKB_AUDIT( 
            if ( !_is_fkb_cloned_pool_(fkb_p) ) /* double check Cloned FKB */
            {
                printk("FKB ASSERT cpu<%u> %s "
                       " !_is_fkb_cloned_pool_(0x%08x)"
                       " master<0x%08x>.recycle<%pS>\n",
                       smp_processor_id(), __FUNCTION__,
                       (int)fkb_p, (int)fkb_p->master_p,
                       fkb_p->master_p->recycle_hook);
                return FKB_NULL;
            } );

        return fkb_p->master_p;
    }
    else                                    /* Master FKB */
    {
        FKB_AUDIT( 
            if ( _get_master_users_(fkb_p) == 0 )  /* assert Master FKB users */
            {
                printk("FKB ASSERT cpu<%u> %s "
                       " _get_master_users_(0x%08x) == 0\n",
                       smp_processor_id(), __FUNCTION__, (int)fkb_p);
                return FKB_NULL;
            } );

        return fkb_p;
    }
}

/*
 *------------------------------------------------------------------------------
 * Placement of a FKB object in the Rx DMA buffer:
 *
 * RX DMA Buffer:   |----- FKB ----|--- reserve headroom ---|---...... 
 *                  ^              ^                        ^
 *                pFkb           pHead                    pData
 *                pBuf
 *------------------------------------------------------------------------------
 */
#define PFKBUFF_PHEAD_OFFSET        sizeof(FkBuff_t)
#define PFKBUFF_TO_PHEAD(pFkb)      ((uint8_t*)((FkBuff_t*)(pFkb) + 1))

#define PDATA_TO_PFKBUFF(pData,headroom)    \
            (FkBuff_t *)((uint8_t*)(pData)-(headroom)-PFKBUFF_PHEAD_OFFSET)
#define PFKBUFF_TO_PDATA(pFkb,headroom)     \
            (uint8_t*)((uint8_t*)(pFkb) + PFKBUFF_PHEAD_OFFSET + (headroom))


#define NBUFF_ALIGN_MASK_8   0x07
pNBuff_t nbuff_align_data(pNBuff_t pNBuff, uint8_t **data_pp,
                          uint32_t len, uint32_t alignMask);

/*
 *------------------------------------------------------------------------------
 *  FKB Functional Interfaces
 *------------------------------------------------------------------------------
 */

/*
 * Function   : fkb_in_skb_test
 * Description: Verifies that the layout of SKB member fields corresponding to
 *              a FKB have the same layout. This allows a FKB to be passed via
 *              a SKB.
 */

extern int fkb_in_skb_test( int fkb_in_skb_offset,
                            int list_offset, int blog_p_offset,
                            int data_offset, int len_offset, int mark_offset,
                            int priority_offset, int recycle_hook_offset,
                            int recycle_context_offset );

/*
 * Global FKB Subsystem Constructor
 * fkb_construct() validates that the layout of fkbuff members in sk_buff
 * is the same. An sk_buff contains an fkbuff and permits a quick translation
 * to and from a fkbuff. It also preallocates the pools of FKBs.
 */
extern int fkb_construct(int fkb_in_skb_offset);

/*
 * Function   : fkb_stats
 * Description: Report FKB Pool statistics, see CC_CONFIG_FKB_STATS
 */
extern void fkb_stats(void);

/*
 * Function   : fkb_alloc
 * Description: Allocate a Cloned/Master FKB object from preallocated pool
 */
extern FkBuff_t * fkb_alloc( FkbObject_t object );

/*
 * Function   : fkb_free
 * Description: Free a FKB object to its respective preallocated pool.
 */
extern void fkb_free(FkBuff_t * fkb_p);

/*
 * Function   : fkb_unshare
 * Description: If a FKB is pointing to a buffer with multiple references
 * to this buffer, then create a copy of the buffer and return a FKB with a
 * single reference to this buffer.
 */
extern FkBuff_t * fkb_unshare(FkBuff_t * fkb_p);

/*
 * Function   : fkbM_borrow
 * Description: Allocate a Master FKB object from the pre-allocated pool.
 */
extern FkBuff_t * fkbM_borrow(void);

/*
 * Function   : fkbM_return
 * Description: Return a Master FKB object to a pre-allocated pool.
 */
extern void fkbM_return(FkBuff_t * fkbM_p);

/*
 * Function   : fkb_set_ref
 * Description: Set reference count to an FKB.
 */
static inline void _fkb_set_ref(FkBuff_t * fkb_p, const int count)
{
    atomic_set(&fkb_p->users, count);
}
FKB_FN( fkb_set_ref,
        void fkb_set_ref(FkBuff_t * fkb_p, const int count),
        _fkb_set_ref(fkb_p, count) )

/*
 * Function   : fkb_inc_ref
 * Description: Increment reference count to an FKB.
 */
static inline void _fkb_inc_ref(FkBuff_t * fkb_p)
{
    atomic_inc(&fkb_p->users);
}
FKB_FN( fkb_inc_ref,
        void fkb_inc_ref(FkBuff_t * fkb_p),
        _fkb_inc_ref(fkb_p) )

/*
 * Function   : fkb_dec_ref
 * Description: Decrement reference count to an FKB.
 */
static inline void _fkb_dec_ref(FkBuff_t * fkb_p)
{
    atomic_dec(&fkb_p->users);
    /* For debug, may want to assert that users does not become negative */
}
FKB_FN( fkb_dec_ref,
        void fkb_dec_ref(FkBuff_t * fkb_p),
        _fkb_dec_ref(fkb_p) )


/*
 * Function   : fkb_preinit
 * Description: A network device driver may use this function to place a
 * FKB object into rx buffers, when they are created. FKB objects preceeds
 * the reserved headroom.
 */
static inline void fkb_preinit(uint8_t * pBuf, RecycleFuncP recycle_hook,
                               uint32_t recycle_context)
{
    FkBuff_t * fkb_p = (FkBuff_t *)pBuf;
    fkb_p->recycle_hook = recycle_hook;         /* never modified */
    fkb_p->recycle_context = recycle_context;   /* never modified */

    fkb_p->ptr  = (void*)NULL;                  /* resets dirty_p, blog_p */
    fkb_p->data = (uint8_t*)NULL;
    fkb_p->len  = fkb_p->mark  = fkb_p->priority = 0;
    fkb_set_ref( fkb_p, 0 );
}

/*
 * Function   : fkb_init
 * Description: Initialize the FKB context for a received packet. Invoked by a
 * network device on extract the packet from a buffer descriptor and associating
 * a FKB context to the received packet.
 */
static inline FkBuff_t * _fkb_init(uint8_t * pBuf, uint32_t headroom,
                                   uint8_t * pData, uint32_t len)
{
    FkBuff_t * fkb_p = PDATA_TO_PFKBUFF(pBuf, headroom);
    fkb_dbg( 1, "fkb_p<0x%08x> pBuf<0x%08x> headroom<%u> pData<0x%08x> len<%d>",
              (int)fkb_p, (int)pBuf, (int)headroom, (int)pData, len );

#if defined(CC_FKB_HEADROOM_AUDIT)
    if ( headroom < FKB_HEADROOM )
        printk("NBUFF: Insufficient headroom <%u>, need <%u> %-10s\n",
               headroom, FKB_HEADROOM, __FUNCTION__);
#endif

    fkb_p->data = pData;
    fkb_p->len  = len;
    fkb_p->ptr  = (void*)NULL;   /* resets dirty_p, blog_p */

    fkb_set_ref( fkb_p, 1 );

    return fkb_p;
}
FKB_FN( fkb_init,
        FkBuff_t * fkb_init(uint8_t * pBuf, uint32_t headroom,
                            uint8_t * pData, uint32_t len),
        return _fkb_init(pBuf, headroom, pData, len) )

/*
 * Function   : fkb_qinit
 * Description: Same as fkb_init, with the exception that a recycle queue
 * context is associated with the FKB, each time the packet is receieved.
 */
static inline FkBuff_t * _fkb_qinit(uint8_t * pBuf, uint32_t headroom,
                    uint8_t * pData, uint32_t len, uint32_t qcontext)
{
    FkBuff_t * fkb_p = PDATA_TO_PFKBUFF(pBuf, headroom);
    fkb_dbg(1, "fkb_p<0x%08x> qcontext<0x%08x>", (int)fkb_p, qcontext );
    fkb_p->recycle_context = qcontext;

    return _fkb_init(pBuf, headroom, pData, len);
}
FKB_FN( fkb_qinit,
        FkBuff_t * fkb_qinit(uint8_t * pBuf, uint32_t headroom,
                             uint8_t * pData, uint32_t len, uint32_t qcontext),
        return _fkb_qinit(pBuf, headroom, pData, len, qcontext) )

/*
 * Function   : fkb_release
 * Description: Release any associated blog and set ref count to 0. A fkb
 * may be released multiple times (not decrement reference count).
 */
void blog_put(struct blog_t * blog_p);
static inline void _fkb_release(FkBuff_t * fkb_p)
{
    fkb_dbg(1, "fkb_p<0x%08x> fkb_p->blog_p<0x%08x>",
            (int)fkb_p, (int)fkb_p->blog_p );
    if ( _IS_BPTR_( fkb_p->blog_p ) )
        blog_put(fkb_p->blog_p);
    fkb_p->ptr = (void*)NULL;   /* reset dirty_p, blog_p */

    fkb_set_ref( fkb_p, 0 );    /* fkb_release may be invoked multiple times */
}
FKB_FN( fkb_release,
        void fkb_release(FkBuff_t * fkb_p),
        _fkb_release(fkb_p) )

/*
 * Function   : fkb_headroom
 * Description: Determine available headroom for the packet in the buffer.
 */
static inline int _fkb_headroom(const FkBuff_t *fkb_p)
{
    return (int)( (uint32_t)(fkb_p->data) - (uint32_t)(fkb_p+1) );
}
FKB_FN( fkb_headroom,
        int fkb_headroom(const FkBuff_t *fkb_p),
        return _fkb_headroom(fkb_p) )

/*
 * Function   : fkb_push
 * Description: Prepare space for data at head of the packet buffer.
 */
static inline uint8_t * _fkb_push(FkBuff_t * fkb_p, uint32_t len)
{
    fkb_p->len  += len;
    fkb_p->data -= len;
    return fkb_p->data;
}
FKB_FN( fkb_push,
        uint8_t * fkb_push(FkBuff_t * fkb_p, uint32_t len),
        return _fkb_push(fkb_p, len) )

/*
 * Function   : fkb_pull
 * Description: Delete data from the head of packet buffer.
 */
static inline uint8_t * _fkb_pull(FkBuff_t * fkb_p, uint32_t len)
{
    fkb_p->len  -= len;
    fkb_p->data += len;
    return fkb_p->data;
}
FKB_FN( fkb_pull,
        uint8_t * fkb_pull(FkBuff_t * fkb_p, uint32_t len),
        return _fkb_pull(fkb_p, len) )

/*
 * Function   : fkb_put
 * Description: Prepare space for data at tail of the packet buffer.
 */
static inline uint8_t * _fkb_put(FkBuff_t * fkb_p, uint32_t len)
{
    uint8_t * tail_p = fkb_p->data + fkb_p->len; 
    fkb_p->len  += len;
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    fkb_p->dirty_p = _to_dptr_from_kptr_(tail_p + len); /* sets dptr_tag */
#endif
    return tail_p;
}
FKB_FN( fkb_put,
        uint8_t * fkb_put(FkBuff_t * fkb_p, uint32_t len),
        return _fkb_put(fkb_p, len) )

/*
 * Function   : fkb_pad
 * Description: Pad the packet by requested number of bytes.
 */
static inline uint32_t _fkb_pad(FkBuff_t * fkb_p, uint32_t padding)
{
    fkb_p->len  += padding;
    return fkb_p->len;
}
FKB_FN( fkb_pad,
        uint32_t fkb_pad(FkBuff_t * fkb_p, uint32_t padding),
        return _fkb_pad(fkb_p, padding) )

/*
 * Function   : fkb_len
 * Description: Determine the length of the packet.
 */
static inline uint32_t _fkb_len(FkBuff_t * fkb_p)
{
    return fkb_p->len;
}
FKB_FN( fkb_len,
        uint32_t fkb_len(FkBuff_t * fkb_p),
        return _fkb_len(fkb_p) )

/*
 * Function   : fkb_data
 * Description: Fetch the start of the packet.
 */
static inline uint8_t * _fkb_data(FkBuff_t * fkb_p)
{
    return fkb_p->data;
}
FKB_FN( fkb_data,
        uint8_t * fkb_data(FkBuff_t * fkb_p),
        return _fkb_data(fkb_p) )

/*
 * Function   : fkb_blog
 * Description: Fetch the associated blog.
 */
static inline struct blog_t * _fkb_blog(FkBuff_t * fkb_p)
{
    return fkb_p->blog_p;
}
FKB_FN( fkb_blog,
        struct blog_t * fkb_blog(FkBuff_t * fkb_p),
        return _fkb_blog(fkb_p) )

/*
 * Function   : fkb_clone
 * Description: Allocate a FKB from the Cloned Pool and make it reference the
 * same packet.
 */
static inline FkBuff_t * _fkb_clone(FkBuff_t * fkbM_p)
{
    FkBuff_t * fkbC_p;

    FKB_AUDIT( 
        if ( smp_processor_id() )
            printk("FKB ASSERT %s not supported on CP 1\n", __FUNCTION__); );

    /* Fetch a pointer to the Master FKB */
    fkbM_p = _get_fkb_master_ptr_( fkbM_p );

    fkbC_p = fkb_alloc( FkbClonedPool_e );  /* Allocate FKB from Cloned pool */

    if ( unlikely(fkbC_p != FKB_NULL) )
    {
        fkb_inc_ref( fkbM_p );
        fkbC_p->master_p   = fkbM_p;
        fkbC_p->ptr   = fkbM_p->ptr;

        fkbC_p->data       = fkbM_p->data;
        fkbC_p->len        = fkbM_p->len;
        fkbC_p->mark        = fkbM_p->mark;
        fkbC_p->priority        = fkbM_p->priority;
    }

    fkb_dbg(1, "fkbC_p<0x%08x> ---> fkbM_p<0x%08x>", (int)fkbC_p, (int)fkbM_p );

    return fkbC_p;       /* May be null */
}
FKB_FN( fkb_clone,
        FkBuff_t * fkb_clone(FkBuff_t * fkbM_p),
        return _fkb_clone(fkbM_p) )

/*
 * Function   : fkb_flush
 * Description: Flush a FKB from current data or received packet data upto
 * the dirty_p. When Flush Optimization is disabled, the entire length.
 */
static inline void _fkb_flush(FkBuff_t * fkb_p, uint8_t * data_p, int len)
{
    uint8_t * fkb_data_p;

    if ( _is_fkb_cloned_pool_(fkb_p) )
        fkb_data_p = PFKBUFF_TO_PDATA(fkb_p->master_p, FKB_HEADROOM);
    else
        fkb_data_p = PFKBUFF_TO_PDATA(fkb_p, FKB_HEADROOM);

    /* headers may have been popped */
    if ( (uint32_t)data_p < (uint32_t)fkb_data_p )
        fkb_data_p = data_p;

    {
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    uint8_t * dirty_p;  /* Flush only L1 dirty cache lines */
    dirty_p = _to_kptr_from_dptr_(fkb_p->dirty_p);  /* extract kernel pointer */

    fkb_dbg(1, "fkb_p<0x%08x> fkb_data<0x%08x> dirty_p<0x%08x> len<%d>",
            (int)fkb_p, (int)fkb_data_p, (int)dirty_p, len);

    cache_flush_region(fkb_data_p, dirty_p);
#else
    uint32_t data_offset;
    data_offset = (uint32_t)data_p - (uint32_t)fkb_data_p;

    fkb_dbg(1, "fkb_p<0x%08x> fkb_data<0x%08x> data_offset<%d> len<%d>",
            (int)fkb_p, (int)fkb_data_p, data_offset, len);

    cache_flush_len(fkb_data_p, data_offset + len);
#endif
    }

    fkb_p->dirty_p = (uint8_t*)NULL;
}
FKB_FN( fkb_flush,
        void fkb_flush(FkBuff_t * fkb_p, uint8_t * data, int len),
        _fkb_flush(fkb_p, data, len) )

/*
 *------------------------------------------------------------------------------
 * Virtual accessors to common members of network kernel buffer
 *------------------------------------------------------------------------------
 */

/* __BUILD_NBUFF_SET_ACCESSOR: generates function nbuff_set_MEMBER() */
#define __BUILD_NBUFF_SET_ACCESSOR( TYPE, MEMBER )                             \
static inline void nbuff_set_##MEMBER(pNBuff_t pNBuff, TYPE MEMBER) \
{                                                                              \
    void * pBuf = PNBUFF_2_PBUF(pNBuff);                                       \
    if ( IS_SKBUFF_PTR(pNBuff) )                                               \
        ((struct sk_buff *)pBuf)->MEMBER = MEMBER;                             \
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */                         \
    else                                                                       \
        ((FkBuff_t *)pBuf)->MEMBER = MEMBER;                                   \
}

/* __BUILD_NBUFF_GET_ACCESSOR: generates function nbuff_get_MEMBER() */
#define __BUILD_NBUFF_GET_ACCESSOR( TYPE, MEMBER )                             \
static inline TYPE nbuff_get_##MEMBER(pNBuff_t pNBuff)                         \
{                                                                              \
    void * pBuf = PNBUFF_2_PBUF(pNBuff);                                       \
    if ( IS_SKBUFF_PTR(pNBuff) )                                               \
        return (TYPE)(((struct sk_buff *)pBuf)->MEMBER);                       \
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */                         \
    else                                                                       \
        return (TYPE)(((FkBuff_t *)pBuf)->MEMBER);                             \
}

/*
 * Common set/get accessor of base network buffer fields:
 * nbuff_set_data(), nbuff_set_len(), nbuff_set_mark(), nbuff_set_priority()
 * nbuff_get_data(), nbuff_get_len(), nbuff_get_mark(), nbuff_get_priority()
 */
__BUILD_NBUFF_SET_ACCESSOR(uint8_t *, data) 
__BUILD_NBUFF_SET_ACCESSOR(uint32_t, len) 
__BUILD_NBUFF_SET_ACCESSOR(uint32_t, mark)      /* Custom network buffer arg1 */
__BUILD_NBUFF_SET_ACCESSOR(uint32_t, priority)  /* Custom network buffer arg2 */

__BUILD_NBUFF_GET_ACCESSOR(uint8_t *, data)
__BUILD_NBUFF_GET_ACCESSOR(uint32_t, len)
__BUILD_NBUFF_GET_ACCESSOR(uint32_t, mark)      /* Custom network buffer arg1 */
__BUILD_NBUFF_GET_ACCESSOR(uint32_t, priority)  /* Custom network buffer arg2 */

/*
 * Function   : nbuff_get_context
 * Description: Extracts the data and len fields from a pNBuff_t.
 */
static inline void * nbuff_get_context(pNBuff_t pNBuff,
                                     uint8_t ** data_p, uint32_t *len_p)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( pBuf == (void*) NULL )
        return pBuf;
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *data_p     = ((struct sk_buff *)pBuf)->data;
        *len_p      = ((struct sk_buff *)pBuf)->len;
    }
    else
    {
        *data_p     = ((FkBuff_t *)pBuf)->data;
        *len_p      = ((FkBuff_t *)pBuf)->len;
    }
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x> data_p<0x%08x>",
           (int)pNBuff, (int)pBuf, (int)*data_p );
    return pBuf;
}

/*
 * Function   : nbuff_get_params
 * Description: Extracts the data, len, mark and priority field from a network
 * buffer.
 */
static inline void * nbuff_get_params(pNBuff_t pNBuff,
                                     uint8_t ** data_p, uint32_t *len_p,
                                     uint32_t * mark_p, uint32_t *priority_p)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( pBuf == (void*) NULL )
        return pBuf;
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *data_p     = ((struct sk_buff *)pBuf)->data;
        *len_p      = ((struct sk_buff *)pBuf)->len;
        *mark_p     = ((struct sk_buff *)pBuf)->mark;
        *priority_p = ((struct sk_buff *)pBuf)->priority;
    }
    else
    {
        *data_p     = ((FkBuff_t *)pBuf)->data;
        *len_p      = ((FkBuff_t *)pBuf)->len;
        *mark_p     = ((FkBuff_t *)pBuf)->mark;
        *priority_p = ((FkBuff_t *)pBuf)->priority;
    }
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x> data_p<0x%08x>",
            (int)pNBuff, (int)pBuf, (int)*data_p );
    return pBuf;
}
    
/* adds recycle flags/context to nbuff_get_params used in impl4 enet */
/*
 * Function   : nbuff_get_params_ext
 * Description: Extracts the data, len, mark, priority and 
 * recycle flags/context field from a network buffer.
 */
static inline void * nbuff_get_params_ext(pNBuff_t pNBuff, uint8_t **data_p, 
                                          uint32_t *len_p, uint32_t *mark_p, 
                                          uint32_t *priority_p, 
                                          uint32_t *rflags_p)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( pBuf == (void*) NULL )
        return pBuf;
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *data_p     = ((struct sk_buff *)pBuf)->data;
        *len_p      = ((struct sk_buff *)pBuf)->len;
        *mark_p     = ((struct sk_buff *)pBuf)->mark;
        *priority_p = ((struct sk_buff *)pBuf)->priority;
        *rflags_p   = ((struct sk_buff *)pBuf)->recycle_flags;
    }
    else
    {
        *data_p     = ((FkBuff_t *)pBuf)->data;
        *len_p      = ((FkBuff_t *)pBuf)->len;
        *mark_p     = ((FkBuff_t *)pBuf)->mark;
        *priority_p = ((FkBuff_t *)pBuf)->priority;
        *rflags_p   = ((FkBuff_t *)pBuf)->recycle_context;
    }
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x> data_p<0x%08x>",
            (int)pNBuff, (int)pBuf, (int)*data_p );
    return pBuf;
}

/*
 *------------------------------------------------------------------------------
 * Virtual common functional apis of a network kernel buffer
 *------------------------------------------------------------------------------
 */

/*
 * Function   : nbuff_push
 * Description: Make space at the start of a network buffer.
 * CAUTION    : In the case of a FKB, no check for headroom is done.
 */
static inline uint8_t * nbuff_push(pNBuff_t pNBuff, uint32_t len)
{
    uint8_t * data;
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( IS_SKBUFF_PTR(pNBuff) )
        data = skb_push(((struct sk_buff *)pBuf), len);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        data = fkb_push((FkBuff_t*)pBuf, len);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x> data<0x%08x> len<%u>",
            (int)pNBuff,(int)pBuf, (int)data, len );
    return data;
}

/*
 * Function   : nbuff_pull
 * Description: Delete data from start of a network buffer.
 */
static inline uint8_t * nbuff_pull(pNBuff_t pNBuff, uint32_t len)
{
    uint8_t * data;
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( IS_SKBUFF_PTR(pNBuff) )
        data = skb_pull(((struct sk_buff *)pBuf), len);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        data = fkb_pull((FkBuff_t *)pBuf, len);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x> data<0x%08x> len<%u>",
            (int)pNBuff,(int)pBuf, (int)data, len );
    return data;
}

/*
 * Function   : nbuff_put
 * Description: Make space at the tail of a network buffer.
 * CAUTION: In the case of a FKB, no check for tailroom is done.
 */
static inline uint8_t * nbuff_put(pNBuff_t pNBuff, uint32_t len)
{
    uint8_t * tail;
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    if ( IS_SKBUFF_PTR(pNBuff) )
        tail = skb_put(((struct sk_buff *)pBuf), len);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        tail = fkb_put((FkBuff_t *)pBuf, len);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x> tail<0x%08x> len<%u>",
            (int)pNBuff,(int)pBuf, (int)tail, len );
    return tail;
}

/*
 * Function   : nbuff_free
 * Description: Free/recycle a network buffer and associated data
 *
 * Freeing may involve a recyling of the network buffer into its respective
 * pool (per network device driver pool, kernel cache or FKB pool). Likewise
 * the associated buffer may be recycled if there are no other network buffers
 * referencing it.
 */

extern void dev_kfree_skb_irq(struct sk_buff *skb);

static inline void nbuff_free(pNBuff_t pNBuff)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x>", (int)pNBuff,(int)pBuf);
    if ( IS_SKBUFF_PTR(pNBuff) )
        dev_kfree_skb_irq((struct sk_buff *)pBuf);
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        fkb_free(pBuf);
    fkb_dbg(2, "<<");
}

/*
 * Function   : nbuff_unshare
 * Description: If there are more than one references to the data buffer
 * associated with the network buffer, create a deep copy of the data buffer
 * and return a network buffer context to it. The returned network buffer
 * may be then used to modify the data packet without impacting the original
 * network buffer and its data buffer.
 *
 * If the data packet had a single network buffer referencing it, then the
 * original network buffer is returned.
 */
static inline pNBuff_t nbuff_unshare(pNBuff_t pNBuff)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x>", (int)pNBuff,(int)pBuf);
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        struct sk_buff *skb_p;
        skb_p = skb_unshare( (struct sk_buff *)pBuf, GFP_ATOMIC);
        pNBuff = SKBUFF_2_PNBUFF(skb_p);
    }
    else
    {
        FkBuff_t * fkb_p;
        fkb_p = fkb_unshare( (FkBuff_t *)pBuf );
        pNBuff = FKBUFF_2_PNBUFF(fkb_p);
    }

    fkb_dbg(2, "<<");
    return pNBuff;
}

/*
 * Function   : nbuff_flush
 * Description: Flush (Hit_Writeback_Inv_D) a network buffer's packet data.
 */
static inline void nbuff_flush(pNBuff_t pNBuff, uint8_t * data, int len)
{
    fkb_dbg(1, "pNBuff<0x%08x> data<0x%08x> len<%d>",
            (int)pNBuff, (int)data, len);
    if ( IS_SKBUFF_PTR(pNBuff) )
        cache_flush_len(data, len);
    else
    {
        FkBuff_t * fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
        fkb_flush(fkb_p, data, len); 
    }
    fkb_dbg(2, "<<");
}

/*
 * Function   : nbuff_flushfree
 * Description: Flush (Hit_Writeback_Inv_D) and free/recycle a network buffer.
 * If the data buffer was referenced by a single network buffer, then the data
 * buffer will also be freed/recycled. 
 */
static inline void nbuff_flushfree(pNBuff_t pNBuff)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x>", (int)pNBuff,(int)pBuf);
    if ( IS_SKBUFF_PTR(pNBuff) )
    {
        struct sk_buff * skb_p = (struct sk_buff *)pBuf;
        cache_flush_len(skb_p->data, skb_p->len);
        dev_kfree_skb_irq(skb_p);
    }
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
    {
        FkBuff_t * fkb_p = (FkBuff_t *)pBuf;
        fkb_flush(fkb_p, fkb_p->data, fkb_p->len);
        fkb_free(fkb_p);
    }
    fkb_dbg(2, "<<");
}

/*
 * Function   : nbuff_xlate
 * Description: Convert a FKB to a SKB. The SKB is data filled with the
 * data, len, mark, priority, and recycle hook and context. 
 *
 * Other SKB fields for SKB API manipulation are also initialized.
 * SKB fields for network stack manipulation are NOT initialized.
 *
 * This function is typically used only in a network device drivers' hard
 * start xmit function handler. A hard start xmit function handler may receive
 * a network buffer of a FKB type and may not wish to rework the implementation
 * to use nbuff APIs. In such an event, a nbuff may be translated to a skbuff.
 */
struct sk_buff * fkb_xlate(FkBuff_t * fkb_p);
static inline struct sk_buff * nbuff_xlate( pNBuff_t pNBuff )
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);
    fkb_dbg(1, "pNBuff<0x%08x> pBuf<0x%08x>", (int)pNBuff,(int)pBuf);

    if ( IS_SKBUFF_PTR(pNBuff) )
        return (struct sk_buff *)pBuf;
    /* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
    else
        return fkb_xlate( (FkBuff_t *)pBuf );
}


/* Miscellaneous helper routines */
static inline void u16cpy( void * dst_p, const void * src_p, uint32_t bytes )
{
    uint16_t * dst16_p = (uint16_t*)dst_p;
    uint16_t * src16_p = (uint16_t*)src_p;
    do { // assuming: (bytes % sizeof(uint16_t) == 0 !!!
        *dst16_p++ = *src16_p++;
    } while ( bytes -= sizeof(uint16_t) );
}

static inline int u16cmp( void * dst_p, const void * src_p,
                          uint32_t bytes )
{
    uint16_t * dst16_p = (uint16_t*)dst_p;
    uint16_t * src16_p = (uint16_t*)src_p;
    do { // assuming: (bytes % sizeof(uint16_t) == 0 !!!
        if ( *dst16_p++ != *src16_p++ )
            return -1;
    } while ( bytes -= sizeof(uint16_t) );

    return 0;
}

#ifdef DUMP_DATA
/* dumpHexData dump out the hex base binary data */
static inline void dumpHexData(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

static inline void dump_pkt(const char * fname, uint8_t * pBuf, uint32_t len)
{
    int dump_len = ( len < 64) ? len : 64;
    printk("%s: data<0x%08x> len<%u>", fname, (int)pBuf, len);
    dumpHexData(pBuf, dump_len);
    cache_flush_len((void*)pBuf, dump_len);
}
#define DUMP_PKT(pBuf,len)      dump_pkt(__FUNCTION__, (pBuf), (len))
#else   /* !defined(DUMP_DATA) */
#define DUMP_PKT(pBuf,len)      do {} while(0)
#endif

#endif  /* defined(__NBUFF_H_INCLUDED__) */
