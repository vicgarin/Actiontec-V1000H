/*
<:copyright-gpl
 Copyright 2010 Broadcom Corp. All Rights Reserved.

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

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/ip.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>

MODULE_AUTHOR("Pavan Kumar <pavank@broadcom.com>");
MODULE_DESCRIPTION("Netfilter connection tracking module for ipsec");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_conntrack_ipsec");

static DEFINE_SPINLOCK(nf_ipsec_lock);

int
(*nf_nat_ipsec_hook_outbound)(struct sk_buff **pskb,
      struct nf_conn *ct, enum ip_conntrack_info ctinfo) __read_mostly;
EXPORT_SYMBOL_GPL(nf_nat_ipsec_hook_outbound);

int
(*nf_nat_ipsec_hook_inbound)(struct sk_buff **pskb,
      struct nf_conn *ct, enum ip_conntrack_info ctinfo, __be32 lan_ip) __read_mostly;
EXPORT_SYMBOL_GPL(nf_nat_ipsec_hook_inbound);

struct sk_buff nfskb_p;

#if 0
#include <net/inet_sock.h>
#define DEBUGP(format, args...)	printk(KERN_DEBUG "%s:%s: " format, __FILE__, __FUNCTION__, ## args)
#else
#define DEBUGP(format, args...)
#endif

#define REFRESH_TIMEOUT    (10 * HZ)
#define CT_REFRESH_TIMEOUT (13 * HZ)
#define CT_15MIN_TIME      (60 * HZ * 15)
#define MAX_TIMEOUT_COUNT  ((CT_15MIN_TIME)/REFRESH_TIMEOUT)

/* Internal table for ISAKMP */
struct _ipsec_table 
{
   u_int32_t initcookie;
   __be32 lan_ip;
   struct nf_conn *ct;
   struct timer_list refresh_timer;
   int pkt_rcvd;
   int ntimeouts;
   int inuse;
} ipsec_table[MAX_VPN_CONNECTION];

static void ipsec_free_entry(unsigned long index)
{
   if( ipsec_table[index].inuse ) 
   {
      del_timer(&ipsec_table[index].refresh_timer);
      memset(&ipsec_table[index], 0, sizeof(struct _ipsec_table));
   }

   DEBUGP("try to free ipsec_table[%ld] which is not in use\n", index);
}

static void ipsec_refresh_ct(unsigned long data)
{
   int i;
   struct _ipsec_table *ipsec_entry = NULL;

   if( data > MAX_VPN_CONNECTION )
      return;

   ipsec_entry = &ipsec_table[data];

   DEBUGP( "ntimeouts %d pkt_rcvd %d entry %p data %lu ct %p\n",
          ipsec_entry->ntimeouts, ipsec_entry->pkt_rcvd, ipsec_entry, data,
          ipsec_entry->ct);

   if( ipsec_entry->pkt_rcvd ) 
   {
      ipsec_entry->pkt_rcvd = 0;
      ipsec_entry->ntimeouts = 0;
   } 
   else 
   {
      ipsec_entry->ntimeouts++;
      if( ipsec_entry->ntimeouts >= MAX_TIMEOUT_COUNT )
      {
         ipsec_free_entry(data);
         return;
      }
   }
   ipsec_entry->refresh_timer.expires = jiffies + REFRESH_TIMEOUT;
   nfskb_p.nfct = (struct nf_conntrack *)ipsec_entry->ct;
   nf_ct_refresh_acct(ipsec_entry->ct, 0, &nfskb_p, CT_REFRESH_TIMEOUT);
   add_timer(&ipsec_entry->refresh_timer);
#if 0
   for( i=0;i<MAX_VPN_CONNECTION;i++ )
      if( ipsec_table[i].inuse )
         printk("table %d with lanIP %u.%u.%u.%u is inuse\n", i, NIPQUAD(ipsec_table[i].lan_ip));
#endif
}

static struct _ipsec_table *ipsec_alloc_entry(int *index)
{
   int idx = 0;

   for( ; idx < MAX_VPN_CONNECTION; idx++ ) 
   {
      if( ipsec_table[idx].inuse )
         continue;
   
      *index = idx;
      memset(&ipsec_table[idx], 0, sizeof(struct _ipsec_table));
      init_timer( &ipsec_table[idx].refresh_timer );
      ipsec_table[idx].refresh_timer.expires = jiffies + REFRESH_TIMEOUT;
      ipsec_table[idx].refresh_timer.function = ipsec_refresh_ct;
      ipsec_table[idx].refresh_timer.data = (unsigned long)idx;
      add_timer( &ipsec_table[idx].refresh_timer );      

      return (&ipsec_table[idx]);
   }
   
   return NULL;
}

/*
 * Search an IPSEC table entry by the initiator cookie.
 */
struct _ipsec_table *
search_ipsec_entry_by_cookie(struct isakmp_pkt_hdr *isakmph)
{
   int idx = 0;
   struct _ipsec_table *ipsec_entry = ipsec_table;

   for( ; idx < MAX_VPN_CONNECTION; idx++ ) 
   {
      DEBUGP("Searching initcookie %x <-> %x\n",
          ntohl(isakmph->initcookie), ntohl(ipsec_entry->initcookie));
      
      if( isakmph->initcookie == ipsec_entry->initcookie ) 
         return ipsec_entry;
      
      ipsec_entry++;
   }
   
   return NULL;
}

/*
 * Search an IPSEC table entry by the source IP address.
 */
struct _ipsec_table *
search_ipsec_entry_by_addr(const __be32 lan_ip, int *index)
{
   int idx = 0;
   struct _ipsec_table *ipsec_entry = ipsec_table;

   for( ; idx < MAX_VPN_CONNECTION; idx++ ) 
   {
      DEBUGP("Looking up lan_ip=%u.%u.%u.%u table entry %u.%u.%u.%u\n", 
              NIPQUAD(lan_ip), NIPQUAD(ipsec_entry->lan_ip));

      if( ntohl(ipsec_entry->lan_ip) == ntohl(lan_ip) ) 
      {
         DEBUGP("Search by addr returning entry %p\n", ipsec_entry);

         *index = idx;
         return ipsec_entry;
      }
      ipsec_entry++;
   }
   
   return NULL;
}

static inline int
ipsec_inbound_pkt(struct sk_buff **pskb, struct nf_conn *ct, 
		  enum ip_conntrack_info ctinfo, __be32 lan_ip)
{
//   struct nf_ct_ipsec_master *info = &nfct_help(ct)->help.ct_ipsec_info;
   typeof(nf_nat_ipsec_hook_inbound) nf_nat_ipsec_inbound;

   DEBUGP("inbound ISAKMP packet for LAN %u.%u.%u.%u\n", NIPQUAD(lan_ip));

   nf_nat_ipsec_inbound = rcu_dereference(nf_nat_ipsec_hook_inbound);
   if (nf_nat_ipsec_inbound && ct->status & IPS_NAT_MASK)
      return nf_nat_ipsec_inbound(pskb, ct, ctinfo, lan_ip);
   
   return NF_ACCEPT;
}

/*
 * For outgoing ISAKMP packets, we need to make sure UDP ports=500
 */
static inline int
ipsec_outbound_pkt(struct sk_buff **pskb,
                   struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
   typeof(nf_nat_ipsec_hook_outbound) nf_nat_ipsec_outbound;

   DEBUGP("outbound ISAKMP packet\n");

   nf_nat_ipsec_outbound = rcu_dereference(nf_nat_ipsec_hook_outbound);
   if( nf_nat_ipsec_outbound && ct->status & IPS_NAT_MASK )
      return nf_nat_ipsec_outbound(pskb, ct, ctinfo);
   
   return NF_ACCEPT;
}


/* track cookies inside ISAKMP, call expect_related */
static int conntrack_ipsec_help(struct sk_buff **pskb, unsigned int protoff,
                             struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
   int dir = CTINFO2DIR(ctinfo);
   struct nf_ct_ipsec_master *info = &nfct_help(ct)->help.ct_ipsec_info;
//   struct iphdr *iph = (*pskb)->nh.iph;
   struct isakmp_pkt_hdr _isakmph, *isakmph;
   struct _ipsec_table *ipsec_entry = ipsec_table;
   unsigned int nexthdr_off;
   int ret, index=0;

   nexthdr_off = protoff + 8;   /* UDP header length = 8 */

   isakmph = skb_header_pointer(*pskb, nexthdr_off, 
                                sizeof(_isakmph), &_isakmph);
   if( !isakmph ) 
   {
      DEBUGP("no full ISAKMP header, can't track\n");
      return NF_ACCEPT;
   }

   spin_lock_bh(&nf_ipsec_lock);

   if( dir == IP_CT_DIR_ORIGINAL )
   {
      int lan_ip = ct->tuplehash[dir].tuple.src.u3.ip;
      DEBUGP("LAN -> WAN: old cookies:%x new cookies:%x, src: %u.%u.%u.%u, dst: %u.%u.%u.%u\n",
              ntohl(info->initcookie), ntohl(isakmph->initcookie),
	      NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), 
	      NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip) );
      
      /* create one entry in the internal table if a new connection is found */
      if( (ipsec_entry = search_ipsec_entry_by_cookie(isakmph)) == NULL ) 
      {
         /* NOTE: cookies may be updated in the connection */
         if( (ipsec_entry = 
              search_ipsec_entry_by_addr(lan_ip, &index)) == NULL ) 
         {
            ipsec_entry = ipsec_alloc_entry(&index);
            if( ipsec_entry == NULL ) 
            {
               /* All entries are currently in use */
               DEBUGP("%s:%s Out of table entries\n", __FILE__, __FUNCTION__);
               spin_unlock_bh(&nf_ipsec_lock);
               return NF_DROP;
            }
            
            ipsec_entry->lan_ip = ct->tuplehash[dir].tuple.src.u3.ip;
            ipsec_entry->inuse = 1;
         }
         /* Update our cookie information */
         ipsec_entry->initcookie = isakmph->initcookie;
         ipsec_entry->ct = ct;

         DEBUGP("create a new ipsec_entry with ct=%p, lan_ip=%u.%u.%u.%u, initcookie=%x\n", 
            ipsec_entry->ct, NIPQUAD(ipsec_entry->lan_ip), 
            ntohl(ipsec_entry->initcookie) );
      }
      ipsec_entry->pkt_rcvd++;

      info->initcookie = isakmph->initcookie;
      info->lan_ip = ct->tuplehash[dir].tuple.src.u3.ip;

      DEBUGP("ct_ipsec_info => initcookie=%x, lan_ip=%u.%u.%u.%u, ct->tuplehash[%d].tuple.src.u3.ip=%u.%u.%u.%u, ct->tuplehash[%d].tuple.dst.u3.ip=%u.%u.%u.%u\n\n", 
              info->initcookie, NIPQUAD(info->lan_ip), 
              dir, NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), 
              dir, NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip));
      
      ret = ipsec_outbound_pkt(pskb, ct, ctinfo); 
   }
   else
   {
      DEBUGP("WAN->LAN\n");
      
      if( (ipsec_entry = search_ipsec_entry_by_cookie(isakmph)) != NULL )
      {
         ipsec_entry->pkt_rcvd++;
         ret = ipsec_inbound_pkt(pskb, ct, ctinfo, ipsec_entry->lan_ip);
      }
      else
      {
         DEBUGP("WARNNING: client from WAN tries to connect to VPN server in the LAN\n");
         ret = NF_ACCEPT;
      }
   }

   spin_unlock_bh(&nf_ipsec_lock);

   return ret;
}

/* ISAKMP protocol helper */
static struct nf_conntrack_helper ipsec __read_mostly = {
   .name = "ipsec",
   .me = THIS_MODULE,
   .max_expected = 1,
   .timeout = 300,
   .tuple.src.l3num = AF_INET,
   .tuple.dst.protonum = IPPROTO_UDP,
   .tuple.src.u.udp.port = __constant_htons(IPSEC_PORT),
   .mask.src.l3num = 0xffff,
   .mask.dst.protonum = 0xff,
   .mask.src.u.udp.port	= __constant_htons(0xffff),
   .help = conntrack_ipsec_help,
};

static int __init nf_conntrack_ipsec_init(void)
{
   return nf_conntrack_helper_register(&ipsec);
}

static void __exit nf_conntrack_ipsec_fini(void)
{
   int idx = 0;

   for( ; idx < MAX_VPN_CONNECTION; idx++ )
      if( ipsec_table[idx].inuse )
         del_timer(&ipsec_table[idx].refresh_timer);

   nf_conntrack_helper_unregister(&ipsec);
}

module_init(nf_conntrack_ipsec_init);
module_exit(nf_conntrack_ipsec_fini);
