/*
* <:copyright-BRCM:2011:DUAL/GPL:standard
* 
*    Copyright (c) 2011 Broadcom Corporation
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include <linux/ip.h>
#include "br_private.h"
#include "br_igmp.h"
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/rtnetlink.h>
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#include <linux/if_vlan.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#endif
#include "br_mcast.h"
#if defined(AEI_VDSL_MC_SSM_HIT)
#include <linux/in.h>
#include <linux/udp.h>
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)

static struct proc_dir_entry *br_igmp_entry = NULL;
static int br_igmp_lan2lan_snooping = 0;

extern int mcpd_process_skb(struct net_bridge *br, struct sk_buff *skb);

static struct in_addr ip_upnp_addr      = {0xEFFFFFFA}; /* UPnP / SSDP */
static struct in_addr ip_ntfy_srvr_addr = {0xE000FF87}; /* Notificatoin Server*/

int br_igmp_control_filter(const unsigned char *dest, __be32 dest_ip)
{
    if(((dest) && is_broadcast_ether_addr(dest)) ||
       ((htonl(dest_ip) & htonl(0xFFFFFF00)) == htonl(0xE0000000)) ||
       (htonl(dest_ip) == htonl(ip_upnp_addr.s_addr)) || /* UPnp/SSDP */
       (htonl(dest_ip) == htonl(ip_ntfy_srvr_addr.s_addr)))   /* Notification srvr */
    {
        return 0;
    }
    else
    {
        return 1;
    }
} /* br_igmp_control_filter */

void br_igmp_lan2lan_snooping_update(int val)
{
    br_igmp_lan2lan_snooping = val;
}

int br_igmp_get_lan2lan_snooping_info(void)
{
    return br_igmp_lan2lan_snooping;
}

static void br_igmp_query_timeout(unsigned long ptr)
{
	struct net_bridge_mc_fdb_entry *dst, *dst_n;
	struct net_bridge *br;
	struct net_bridge_mc_rep_entry *rep_entry, *rep_entry_n;
    
	br = (struct net_bridge *) ptr;

	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry_safe(dst, dst_n, &br->mc_list, list) {
	    if (time_after_eq(jiffies, dst->tstamp)) {
            mcpd_nl_send_igmp_purge_entry(dst);
		    list_for_each_entry_safe(rep_entry, 
                                         rep_entry_n, &dst->rep_list, list)     {
		        list_del(&rep_entry->list);
		        kfree(rep_entry);
		    }
		    list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		    br_mcast_blog_release(BR_MCAST_PROTO_IGMP, (void *)dst);
#endif
		    kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
		
	mod_timer(&br->igmp_timer, jiffies + TIMER_CHECK_TIMEOUT*HZ);		
}

static struct net_bridge_mc_rep_entry *
                br_igmp_rep_find(const struct net_bridge_mc_fdb_entry *mc_fdb,
                                 const struct in_addr *rep)
{
	struct net_bridge_mc_rep_entry *rep_entry;
	
	list_for_each_entry(rep_entry, &mc_fdb->rep_list, list) {
	    if(rep_entry->rep.s_addr == rep->s_addr)
		return rep_entry;
	}

	return NULL;
}

static int br_mc_fdb_update(struct net_bridge *br, 
                            struct net_bridge_port *prt, 
                            struct in_addr *grp, 
                            struct in_addr *rep, 
                            int mode, 
                            struct in_addr *src,
                            struct net_device *from_dev)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct net_bridge_mc_fdb_entry *dst_main = NULL;
	struct net_bridge_mc_rep_entry *rep_entry = NULL;
	int ret = 0;
	int filt_mode;

	if(mode == SNOOP_IN_ADD)
	   filt_mode = MCAST_INCLUDE;
	else
	   filt_mode = MCAST_EXCLUDE;
    
	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry(dst, &br->mc_list, list) {
        if (dst->grp.s_addr == grp->s_addr) {
            if((src->s_addr == dst->src_entry.src.s_addr) &&
                (filt_mode == dst->src_entry.filt_mode) && 
                (dst->from_dev == from_dev) &&
                (dst->dst == prt)) {
                if(br_igmp_rep_find(dst, rep)) {
                    dst->tstamp = jiffies + BR_IGMP_MEMBERSHIP_TIMEOUT*HZ;
                }
                else
                {
                    dst_main = dst;
                }
                ret = 1;
            }
#if defined(CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH)
	/* patch for igmp report flooding by robo */
            else if ((0 == dst->src_entry.src.s_addr) &&
	                (MCAST_EXCLUDE == dst->src_entry.filt_mode)) {
                dst->tstamp = jiffies + BR_IGMP_MEMBERSHIP_TIMEOUT*HZ;
            }
#endif /* CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH*/
        }
	}
	spin_unlock_bh(&br->mcl_lock);

	if(dst_main) {
	    rep_entry = 
                    kmalloc(sizeof(struct net_bridge_mc_rep_entry), GFP_KERNEL);
	    if(rep_entry) {
            rep_entry->rep.s_addr = rep->s_addr;
            spin_lock_bh(&br->mcl_lock);
            list_add_tail(&rep_entry->list, &dst_main->rep_list);
            spin_unlock_bh(&br->mcl_lock);
        }
	}

	return ret;
}

#if 0
static struct net_bridge_mc_fdb_entry *br_mc_fdb_get(struct net_bridge *br, 
                                                     struct net_bridge_port *prt, 
                                                     struct in_addr *grp, 
                                                     struct in_addr *rep, 
                                                     int mode, 
                                                     struct in_addr *src,
                                                     struct net_device *from_dev)
{
    struct net_bridge_mc_fdb_entry *dst;
	int filt_mode;
    
	if(mode == SNOOP_IN_CLEAR)
        filt_mode = MCAST_INCLUDE;
	else
        filt_mode = MCAST_EXCLUDE;
          
	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry(dst, &br->mc_list, list) {
	    if ((dst->grp.s_addr == grp->s_addr) && 
            (br_igmp_rep_find(dst, rep)) &&
            (filt_mode == dst->src_entry.filt_mode) && 
            (dst->src_entry.src.s_addr == src->s_addr) &&
            (dst->from_dev == from_dev) &&
            (dst->dst == prt)) {
            spin_unlock_bh(&br->mcl_lock);
            return dst;
        }
    }
	spin_unlock_bh(&br->mcl_lock);
	
	return NULL;
}
#endif

int br_igmp_process_if_change(struct net_bridge *br)
{
    struct net_bridge_mc_fdb_entry *dst, *dst_n;
    struct net_bridge_mc_rep_entry *rep_entry, *rep_entry_n;

    spin_lock_bh(&br->mcl_lock);
    list_for_each_entry_safe(dst, dst_n, &br->mc_list, list) {
        mcpd_nl_send_igmp_purge_entry(dst);
        list_for_each_entry_safe(rep_entry, 
                rep_entry_n, &dst->rep_list, list)     {
            list_del(&rep_entry->list);
            kfree(rep_entry);
        }
        list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	    br_mcast_blog_release(BR_MCAST_PROTO_IGMP, (void *)dst);
#endif
        kfree(dst);
    }
    spin_unlock_bh(&br->mcl_lock);

    return 0;
}

static int br_igmp_is_br_port(struct net_bridge *br,struct net_device *from_dev)
{
    struct net_bridge_port *p = NULL;
    int ret = 0;

    spin_lock_bh(&br->lock);
    list_for_each_entry_rcu(p, &br->port_list, list) {
        if ((p->dev) && (!memcmp(p->dev->name, from_dev->name, IFNAMSIZ)))
            ret = 1;
    }
    spin_unlock_bh(&br->lock);

    return ret;
} /* br_igmp_is_br_port */

#if defined(AEI_VDSL_MC_SSM_HIT) && defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
/* Ken, 2012/10/12, fixed Agile 8347.
  * Two STBs can't watch same channel with SSM group.
  * The SSM group data will be hit by flow cache when first STB watch channel,
  * and the flow cache entry of second STB will not be update, the idea  is
  * update the flow cache entry when got the join packet from second STB.
  */
static int AEI_handle_mc_ssm_blog_same_group(struct net_bridge *br, 
                            struct net_bridge_port *prt, 
                            struct in_addr *grp, 
                            struct in_addr *rep, 
                            int mode, 
                            struct in_addr *src,
                            struct net_device *from_dev,
                            struct net_bridge_mc_fdb_entry *mc_fdb)
{
    struct net_bridge_mc_fdb_entry *dst;
    int found = 0;
    int filt_mode;
    Blog_t *blog_p = BLOG_NULL;
    uint32_t blog_idx = 0;
    BlogTraffic_t traffic = BlogTraffic_IPV4_MCAST;
    /* check SSM group and not have special source address*/
    if(((htonl(grp->s_addr) & htonl(0xFF000000)) != htonl(0xE8000000)) ||
       src->s_addr !=0)
	  return 0;

    if(mode == SNOOP_IN_ADD)
	filt_mode = MCAST_INCLUDE;
    else
        filt_mode = MCAST_EXCLUDE;
     //printk("grp->s_addr =%x,src->s_addr =%x\n",grp->s_addr,src->s_addr);
    
    spin_lock_bh(&br->mcl_lock);
    /* search the mcast entry of bridge and find the one or not*/
    list_for_each_entry(dst, &br->mc_list, list) {
        if (dst->grp.s_addr == grp->s_addr) {
            if((src->s_addr == dst->src_entry.src.s_addr) &&
                (filt_mode == dst->src_entry.filt_mode) && 
                (dst->from_dev == from_dev)) {
                    //printk("find the same group. blog_idx=%x \n",dst->blog_idx);
                    blog_idx = dst->blog_idx;
                    found = 1;
                    break;
            }
        }
    }

    /* if find one, update the flow cache with same source IP of video data */
    if(found) {

        if(!blog_idx) {
            //printk("AEI_handle_mc_ssm_blog  can't find blog_idx, return\n");
	    spin_unlock_bh(&br->mcl_lock);
            return 0;
        }

     
        blog_p = blog_deactivate(blog_idx, traffic);
        if(blog_p) {
            blog_link(MCAST_FDB, blog_p, (void *)dst, 0, 0);
            mc_fdb->saddr = blog_p->rx.tuple.saddr;

            if ( blog_activate(blog_p, traffic) == BLOG_KEY_INVALID ) {
                printk("BLOG_KEY_INVALID");
                blog_rule_free_list(blog_p);
                blog_put(blog_p);
            }
            //  printk("KEN-> s_addr=%x\n", mc_fdb->src_entry.src.s_addr);
            //   printk(KERN_DEBUG"KEN-> after blog_idx=%u\n",mc_fdb->blog_idx);

        }
        else {
            printk("blog_p is NULL\n");
        }
    }
    spin_unlock_bh(&br->mcl_lock);

    return found;
}
#endif

int br_igmp_mc_fdb_add(struct net_device *from_dev,
                       int wan_ops,
                       struct net_bridge *br, 
                       struct net_bridge_port *prt, 
                       struct in_addr *grp, 
                       struct in_addr *rep, 
                       int mode, 
                       int tci, 
                       struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *mc_fdb = NULL;
#if defined(CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH)
	struct net_bridge_mc_fdb_entry *mc_fdb_robo, *mc_fdb_robo_n;
#endif /* CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH */
	struct net_bridge_mc_rep_entry *rep_entry = NULL;
    int ret = 1;

	//printk("--- add mc entry ---\n");

    if(!br || !prt || !grp|| !rep || !from_dev)
        return 0;

    if(!br_igmp_control_filter(NULL, grp->s_addr))
        return 0;

    if(!netdev_path_is_leaf(from_dev))
        return 0;

    if((wan_ops == MCPD_IF_TYPE_BRIDGED) && 
        (!br_igmp_is_br_port(br, from_dev)))
        return 0;

    if((SNOOP_IN_ADD != mode) && (SNOOP_EX_ADD != mode))             
        return 0;

    if(grp->s_addr == ip_upnp_addr.s_addr)
        return 0;

	if (br_mc_fdb_update(br, prt, grp, rep, mode, src, from_dev))
	    return 0;

#if defined(CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH)
	/* patch for snooping entry when LAN client access port is moved & 
           igmp report flooding by robo */
	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry_safe(mc_fdb_robo, mc_fdb_robo_n, &br->mc_list, list) {
        if ((mc_fdb_robo->grp.s_addr == grp->s_addr) &&
            (0 == mc_fdb_robo->src_entry.src.s_addr) &&
            (MCAST_EXCLUDE == mc_fdb_robo->src_entry.filt_mode) && 
            (br_igmp_rep_find(mc_fdb_robo, rep)) &&
            (mc_fdb_robo->dst != prt)) {
            list_del(&mc_fdb_robo->list);
            kfree(mc_fdb_robo);
        }
    }
    spin_unlock_bh(&br->mcl_lock);
#endif /* CONFIG_BR_IGMP_SNOOP_SWITCH_PATCH */
	    
	mc_fdb = kmalloc(sizeof(struct net_bridge_mc_fdb_entry), GFP_KERNEL);
	rep_entry = kmalloc(sizeof(struct net_bridge_mc_rep_entry), GFP_KERNEL);

	if (mc_fdb && rep_entry)
	{
        mc_fdb->grp.s_addr = grp->s_addr;
        memcpy(&mc_fdb->src_entry, src, sizeof(struct in_addr));
        mc_fdb->src_entry.filt_mode = 
                  (mode == SNOOP_IN_ADD) ? MCAST_INCLUDE : MCAST_EXCLUDE;
        mc_fdb->dst = prt;
        mc_fdb->tstamp = jiffies + BR_IGMP_MEMBERSHIP_TIMEOUT * HZ;
        mc_fdb->lan_tci = tci;
        mc_fdb->wan_tci = 0;
        mc_fdb->num_tags = 0;
        mc_fdb->from_dev = from_dev;
        memcpy(mc_fdb->wan_name, from_dev->name, IFNAMSIZ);
        memcpy(mc_fdb->lan_name, prt->dev->name, IFNAMSIZ);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
        mc_fdb->blog_idx = 0;
#endif
#if defined(AEI_VDSL_MC_SSM_HIT) && defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	mc_fdb->saddr = 0;
        AEI_handle_mc_ssm_blog_same_group(br, prt, grp, rep, mode, src, from_dev,mc_fdb);
#endif
	    INIT_LIST_HEAD(&mc_fdb->rep_list);
	    rep_entry->rep.s_addr = rep->s_addr;
	    spin_lock_bh(&br->mcl_lock);
	    list_add_tail(&rep_entry->list, &mc_fdb->rep_list);
        spin_unlock_bh(&br->mcl_lock);

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
        ret = br_mcast_blog_process(br, (void*)mc_fdb, BR_MCAST_PROTO_IGMP, wan_ops);
        if(ret < 0)
        {
            kfree(mc_fdb);
            kfree(rep_entry);
            return ret;
        }
#endif

	    spin_lock_bh(&br->mcl_lock);
        list_add_tail(&mc_fdb->list, &br->mc_list);
        spin_unlock_bh(&br->mcl_lock);

        if (!br->start_timer) {
            init_timer(&br->igmp_timer);
            br->igmp_timer.expires = jiffies + TIMER_CHECK_TIMEOUT*HZ;
            br->igmp_timer.function = br_igmp_query_timeout;
            br->igmp_timer.data = (unsigned long) br;
            add_timer(&br->igmp_timer);
            br->start_timer = 1;
        }
    }
	else
	{	
	    kfree(mc_fdb);
	    kfree(rep_entry);
	}

	return ret;
}

void br_igmp_mc_fdb_cleanup(struct net_bridge *br)
{
	struct net_bridge_mc_fdb_entry *dst, *dst_n;
	struct net_bridge_mc_rep_entry *rep_entry, *rep_entry_n;
    
	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry_safe(dst, dst_n, &br->mc_list, list) {
	    list_for_each_entry_safe(rep_entry, 
	                             rep_entry_n, &dst->rep_list, list) {
            list_del(&rep_entry->list);
            kfree(rep_entry);
	    }
	    list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG) 
	    br_mcast_blog_release(BR_MCAST_PROTO_IGMP, (void *)dst);
#endif
	    kfree(dst);
	}
	spin_unlock_bh(&br->mcl_lock);
}

void br_igmp_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, struct in_addr *grp)
{
	struct net_bridge_mc_fdb_entry *dst, *dst_n;
	struct net_bridge_mc_rep_entry *rep_entry, *rep_entry_n;

	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry_safe(dst, dst_n, &br->mc_list, list) {
	    if ((dst->grp.s_addr == grp->s_addr) && (dst->dst == prt)) {
		list_for_each_entry_safe(rep_entry, 
	                             rep_entry_n, &dst->rep_list, list) {
		    list_del(&rep_entry->list);
		    kfree(rep_entry);
		}
		list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG) 
		br_mcast_blog_release(BR_MCAST_PROTO_IGMP, (void *)dst);
#endif
		kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
}

int br_igmp_mc_fdb_remove(struct net_device *from_dev,
                          int wan_ops,
                          struct net_bridge *br, 
                          struct net_bridge_port *prt, 
                          struct in_addr *grp, 
                          struct in_addr *rep, 
                          int mode, 
                          struct in_addr *src)
{
	struct net_bridge_mc_fdb_entry *mc_fdb, *mc_fdb_n;
	struct net_bridge_mc_rep_entry *rep_entry, *rep_entry_n;
	int filt_mode;

	//printk("--- remove mc entry ---\n");
	
	if(!br || !prt || !grp|| !rep || !from_dev)
	    return 0;

    if(!br_igmp_control_filter(NULL, grp->s_addr))
        return 0;

	if(!netdev_path_is_leaf(from_dev))
	    return 0;

	if((SNOOP_IN_CLEAR != mode) && (SNOOP_EX_CLEAR != mode))
	   return 0;

	if(mode == SNOOP_IN_CLEAR)
        filt_mode = MCAST_INCLUDE;
	else
        filt_mode = MCAST_EXCLUDE;

    spin_lock_bh(&br->mcl_lock);
    list_for_each_entry_safe(mc_fdb, mc_fdb_n, &br->mc_list, list) {
	    if ((mc_fdb->grp.s_addr == grp->s_addr) && 
            (filt_mode == mc_fdb->src_entry.filt_mode) && 
            (mc_fdb->src_entry.src.s_addr == src->s_addr) &&
            (mc_fdb->from_dev == from_dev) &&
            (mc_fdb->dst == prt)) {
            list_for_each_entry_safe(rep_entry, 
	                             rep_entry_n, &mc_fdb->rep_list, list) {
		        if(rep_entry->rep.s_addr == rep->s_addr) {
                    list_del(&rep_entry->list);
                    kfree(rep_entry);
                }
	        }
	        if(list_empty(&mc_fdb->rep_list)) {
                list_del(&mc_fdb->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG) 
                br_mcast_blog_release(BR_MCAST_PROTO_IGMP, (void *)mc_fdb);
#endif
                kfree(mc_fdb);
	        }
        }
	}
    spin_unlock_bh(&br->mcl_lock);
	
	return 0;
}

#if defined(AEI_VDSL_MC_SSM_HIT) && defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)

/* Ken, 2012/06/28
  * For IGMPV2, the fc in 4.12.02/04/06RC3 can't hit the MC SSM group packets,
  * since the fc is obj code, this fix is just a workaround for the 6368 chip which not support FAP module.
  * If BCM fix it in the further release or customer's depolyment use IGMPV3 and specify the source IP , you can discard it.
  */

void AEI_handle_mc_ssm_blog(t_BR_MCAST_PROTO_TYPE proto, struct net_bridge_mc_fdb_entry *mc_fdb, struct iphdr *pip,struct sk_buff *skb)
{
    Blog_t *blog_p = BLOG_NULL;
    uint32_t blog_idx = 0;
    BlogTraffic_t traffic = BlogTraffic_IPV4_MCAST;
    Blog_t *new_blog_p = BLOG_NULL;
    struct udphdr *uh = NULL;

   // printk(KERN_DEBUG"KEN-> pip->saddr =%x, pip->daddr=%x, src.s_addr=%x\n",pip->saddr,pip->daddr,mc_fdb->src_entry.src.s_addr );
     /* 1. The dest multicast address is SSM group(232.0.0.0/8),RFC4607/5771
       * 2. discard: The 232.239.0.0/16 in Telus deployment is not for video, if fc hit it some unexcepted message will be outputted by fc.
       * 3. The source IP of mc table is zero
       * 4. Use UDP src/des port check to instead of #2.
       */
     if(((htonl(pip->daddr) & htonl(0xFF000000)) == htonl(0xE8000000)) &&  //232.0.0.0/8
   //   ((htonl(pip->daddr) & htonl(0x00FF0000)) != htonl(0x00EF0000)) && //0.239.0.0/16
	mc_fdb->src_entry.src.s_addr ==0)
      {
         if(pip->protocol == IPPROTO_UDP)
         {
               uh = (struct udphdr*)(skb->data+(pip->ihl<<2));
              // printk("skb uh->source =%x,dest=%x\n",htons(uh->source),htons(uh->dest));
               if(htons(uh->source) == htons(uh->dest))
                  return;
         }
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
         if(proto == BR_MCAST_PROTO_IGMP)
         {
	         blog_idx =  mc_fdb->blog_idx;
              traffic = BlogTraffic_IPV4_MCAST;
         }
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
         if(proto == BR_MCAST_PROTO_MLD)
         {
	         blog_idx =  mc_fdb->blog_idx;
               traffic = BlogTraffic_IPV6_MCAST;
         }
#endif

	   if(!blog_idx)
        {
        //    printk(KERN_DEBUG"AEI_handle_mc_ssm_blog  can't find blog_idx, return\n");
            return;
         }
      
      //  printk(KERN_DEBUG"KEN->  find blog_idx=%u\n",blog_idx);
	   blog_p = blog_deactivate(blog_idx, traffic);
         
        if(blog_p)
        {
            new_blog_p = blog_get();
            
            if(new_blog_p != BLOG_NULL) 
            { 
                blog_copy(new_blog_p, blog_p);
                /* get the source address of MR server and update the fc entry*/
                new_blog_p->rx.tuple.saddr = ntohl(pip->saddr);
                new_blog_p->tx.tuple.saddr = ntohl(pip->saddr);
                blog_link(MCAST_FDB, new_blog_p, (void *)mc_fdb, 0, 0);
                if ( blog_activate(new_blog_p, traffic) == BLOG_KEY_INVALID )
                {
                   blog_rule_free_list(new_blog_p);
                   blog_put(new_blog_p);
                }
            }
	       else
	      {	
	          blog_rule_free_list(blog_p);
	      }
            
            blog_put(blog_p);
         //   printk(KERN_DEBUG"KEN-> after blog_idx=%u\n",mc_fdb->blog_idx);

       }    
    }
	return;
}

#endif

int br_igmp_mc_forward(struct net_bridge *br, 
                       struct sk_buff *skb, 
                       int forward,
                       int is_routed)
{
	struct net_bridge_mc_fdb_entry *dst;
	int status = 0;
	struct sk_buff *skb2;
	struct net_bridge_port *p, *p_n;
	struct iphdr *pip = ip_hdr(skb);
	const unsigned char *dest = eth_hdr(skb)->h_dest;

	if(eth_hdr(skb)->h_proto != ETH_P_IP)
		return status;

	if ((pip->protocol == IPPROTO_IGMP)  &&
		 (br->igmp_proxy || br->igmp_snooping))
	{
		/* for bridged WAN service, do not pass any IGMP packets
		   coming from the WAN port to mcpd. Queries can be passed 
		   through for forwarding, other types should be dropped */
		if ( skb->dev->priv_flags & IFF_WANDEV )
		{
			__u8 pos = (pip->ihl << 2);
			__u8 type = skb->data[pos];
			if (type != IGMP_HOST_MEMBERSHIP_QUERY)
			{
				kfree_skb(skb);
				status = 1;
			}
		}
		else
		{
			if(skb->dev && (skb->dev->br_port)) 
			{ 
				mcpd_process_skb(br, skb);
			}
		}
		return status;
	}

	if (!br->igmp_snooping)
		return status;

	if ((br->igmp_snooping == SNOOPING_BLOCKING_MODE) && 
		br_igmp_control_filter(dest, pip->daddr))
		status = 1;


	spin_lock_bh(&br->mcl_lock);
	list_for_each_entry(dst, &br->mc_list, list) {
		if (dst->grp.s_addr == pip->daddr) {
			if((dst->src_entry.filt_mode == MCAST_INCLUDE) && 
				(pip->saddr == dst->src_entry.src.s_addr)) {
				if (!dst->dst->dirty) {
					if((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL)
					{
						return 0;
					}
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
					blog_clone(skb, blog_ptr(skb2));
#endif
					if(forward)
						br_forward(dst->dst, skb2);
					else
						br_deliver(dst->dst, skb2);
				}
				dst->dst->dirty = 1;
				status = 1;
			}
			else if(dst->src_entry.filt_mode == MCAST_EXCLUDE) {
				if((0 == dst->src_entry.src.s_addr) ||
					(pip->saddr != dst->src_entry.src.s_addr)) {
					if (!dst->dst->dirty) {
						if((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL)
						{
							return 0;
						}
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#if defined(AEI_VDSL_MC_SSM_HIT)
                                AEI_handle_mc_ssm_blog(BR_MCAST_PROTO_IGMP, dst,pip,skb2);
#endif
						blog_clone(skb, blog_ptr(skb2));

#endif
						if(forward)
							br_forward(dst->dst, skb2);
						else
							br_deliver(dst->dst, skb2);
					}
					dst->dst->dirty = 1;
					status = 1;
				}
				else if(pip->saddr == dst->src_entry.src.s_addr) {
					status = 1;
				}
			}
		}
	}
	spin_unlock_bh(&br->mcl_lock);

	if (status) {
		list_for_each_entry_safe(p, p_n, &br->port_list, list) {
			p->dirty = 0;
		}
	}

	if(status)
		kfree_skb(skb);

	return status;
}

struct net_bridge_mc_fdb_entry *br_igmp_mc_fdb_copy(struct net_bridge *br, 
                                     const struct net_bridge_mc_fdb_entry *igmp_fdb)
{
    struct net_bridge_mc_fdb_entry *new_igmp_fdb = NULL;
    struct net_bridge_mc_rep_entry *rep_entry = NULL;
    struct net_bridge_mc_rep_entry *rep_entry_n = NULL;
    int success = 1;

	new_igmp_fdb = kmalloc(sizeof(struct net_bridge_mc_fdb_entry), GFP_KERNEL);

	if (new_igmp_fdb)
    {
        memcpy(new_igmp_fdb, igmp_fdb, sizeof(struct net_bridge_mc_fdb_entry));
	    INIT_LIST_HEAD(&new_igmp_fdb->rep_list);

        list_for_each_entry(rep_entry, &igmp_fdb->rep_list, list) {
            rep_entry_n = kmalloc(sizeof(struct net_bridge_mc_rep_entry), GFP_KERNEL);
            if(rep_entry_n)
            {
                memcpy(rep_entry_n, 
                       rep_entry, 
                       sizeof(struct net_bridge_mc_rep_entry));
                spin_lock_bh(&br->mcl_lock);
                list_add_tail(&rep_entry_n->list, &new_igmp_fdb->rep_list);
                spin_unlock_bh(&br->mcl_lock);
            }
            else 
            {
                success = 0;
                break;
            }
        }

        if(success)
        {
            spin_lock_bh(&br->mcl_lock);
            list_add_tail(&new_igmp_fdb->list, &br->mc_list);
            spin_unlock_bh(&br->mcl_lock);
        }
        else
        {
            spin_lock_bh(&br->mcl_lock);
	        list_for_each_entry_safe(rep_entry, 
	                             rep_entry_n, &new_igmp_fdb->rep_list, list) {
                list_del(&rep_entry->list);
                kfree(rep_entry);
	        }
		    list_del(&new_igmp_fdb->list);
            kfree(new_igmp_fdb);
            spin_unlock_bh(&br->mcl_lock);
            new_igmp_fdb = NULL;
        }
    }

    return new_igmp_fdb;
} /* br_igmp_mc_fdb_copy */

void br_igmp_mc_fdb_del_entry(struct net_bridge *br, 
                              struct net_bridge_mc_fdb_entry *igmp_fdb)
{
    struct net_bridge_mc_rep_entry *rep_entry = NULL;
    struct net_bridge_mc_rep_entry *rep_entry_n = NULL;

    spin_lock_bh(&br->mcl_lock);
    list_for_each_entry_safe(rep_entry, 
        rep_entry_n, &igmp_fdb->rep_list, list) {
            list_del(&rep_entry->list);
            kfree(rep_entry);
    }
    list_del(&igmp_fdb->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
    br_mcast_blog_release(BR_MCAST_PROTO_IGMP, (void *)igmp_fdb);
#endif
    kfree(igmp_fdb);
    spin_unlock_bh(&br->mcl_lock);

    return;
}

static void *snoop_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net_device *dev;
	loff_t offs = 0;

	rtnl_lock();
	ASSERT_RTNL();
	for_each_netdev(&init_net, dev)
    {
		if ((dev->priv_flags & IFF_EBRIDGE) &&
            (*pos == offs)) { 
            return dev;
		}
	}
	++offs;
	return NULL;
}

static void *snoop_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net_device *dev = v;

	++*pos;
	
	for(dev = next_net_device(dev); dev; dev = next_net_device(dev)) {
		if(dev->priv_flags & IFF_EBRIDGE)
        {
			return dev;
        }
	}
	return NULL;
}

static int snoop_seq_show(struct seq_file *seq, void *v)
{
	struct net_device *dev = v;
	struct net_bridge_mc_fdb_entry *dst;
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_mc_rep_entry *rep_entry;
	int first = 1;

	seq_printf(seq, "igmp snooping %d  proxy %d  lan2lan-snooping %d, rate-limit %dpps\n", 
	           br->igmp_snooping, 
	           br->igmp_proxy,
	           br_igmp_lan2lan_snooping,
              br->igmp_rate_limit);
	seq_printf(seq, "bridge device src-dev #tags lan-tci    wan-tci");
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	seq_printf(seq, "    group      mode source     timeout reporter   Index\n");
#else
	seq_printf(seq, "    group      mode source     timeout reporter");
#endif

	list_for_each_entry(dst, &br->mc_list, list)
	{
		seq_printf(seq, "%-6s %-6s %-7s %02d    0x%08x 0x%08x", 
		           br->dev->name, 
		           dst->dst->dev->name, 
		           dst->from_dev->name, 
		           dst->num_tags,
		           dst->lan_tci,
		           dst->wan_tci);

		seq_printf(seq, " 0x%08x", dst->grp.s_addr);

		seq_printf(seq, " %-4s 0x%08x %-7d", 
		           (dst->src_entry.filt_mode == MCAST_EXCLUDE) ? 
		           "EX" : "IN", dst->src_entry.src.s_addr, 
		           (int) (dst->tstamp - jiffies)/HZ);

		list_for_each_entry(rep_entry, &dst->rep_list, list)
		{ 
			if(first)
			{
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
				seq_printf(seq, " 0x%08x 0x%08x\n", rep_entry->rep.s_addr, dst->blog_idx);
#else
				seq_printf(seq, " 0x%08x\n", rep_entry->rep.s_addr);
#endif
				first = 0;
			}
			else
			{
				seq_printf(seq, "%84s 0x%08x\n", " ", rep_entry->rep.s_addr);
			}
		}
		first = 1;
	}

	return 0;
}

static void snoop_seq_stop(struct seq_file *seq, void *v)
{
	rtnl_unlock();
}

static struct seq_operations snoop_seq_ops = {
	.start = snoop_seq_start,
	.next  = snoop_seq_next,
	.stop  = snoop_seq_stop,
	.show  = snoop_seq_show,
};

static int snoop_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &snoop_seq_ops);
}

static struct file_operations br_igmp_snoop_proc_fops = {
	.owner = THIS_MODULE,
	.open  = snoop_seq_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void br_igmp_snooping_init(void)
{
	br_igmp_entry = proc_create("igmp_snooping", 0, init_net.proc_net,
			   &br_igmp_snoop_proc_fops);

	if(!br_igmp_entry) {
		printk("error while creating igmp_snooping proc\n");
	}
}
#endif
