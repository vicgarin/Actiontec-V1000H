#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include <linux/ip.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/list.h>
#include <linux/rtnetlink.h>
#include "br_private.h"
#include "br_mld.h"
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#include <linux/if_vlan.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#endif
#include "br_mcast.h"

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)

struct proc_dir_entry *br_mld_entry = NULL;
static int br_mld_lan2lan_snooping = 0;

extern int mcpd_process_skb(struct net_bridge *br, struct sk_buff *skb);

static int br_mld_control_filter(const unsigned char *dest, const struct in6_addr *ipv6)
{
    if(((dest) && is_broadcast_ether_addr(dest)) || 
       (!BCM_IN6_IS_ADDR_MULTICAST(ipv6)) ||
       (BCM_IN6_IS_ADDR_MC_SCOPE0(ipv6)) ||
       (BCM_IN6_IS_ADDR_MC_NODELOCAL(ipv6)) ||
       (BCM_IN6_IS_ADDR_MC_LINKLOCAL(ipv6)))
        return 0;
    else
        return 1;
}

void br_mld_lan2lan_snooping_update(int val)
{
    br_mld_lan2lan_snooping = val;
}

int br_mld_get_lan2lan_snooping_info(void)
{
    return br_mld_lan2lan_snooping;
}

static void mld_query_timeout(unsigned long ptr)
{
	struct net_br_mld_mc_fdb_entry *dst, *dst_n;
	struct net_bridge *br;
    
	br = (struct net_bridge *) ptr;

	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_entry_safe(dst, dst_n, &br->mld_mc_list, list) {
	    if (time_after_eq(jiffies, dst->tstamp)) {
            list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		    br_mcast_blog_release(BR_MCAST_PROTO_MLD, (void *)dst);
#endif
            kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mld_mcl_lock);
		
	mod_timer(&br->mld_timer, jiffies + TIMER_CHECK_TIMEOUT*HZ);		
}

static int br_mld_is_br_port(struct net_bridge *br,struct net_device *from_dev)
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


static int br_mld_mc_fdb_update(struct net_bridge *br, 
                                struct net_bridge_port *prt, 
                                struct in6_addr *grp, 
                                struct in6_addr *rep, 
                                int mode, 
                                struct in6_addr *src,
                                struct net_device *from_dev)
{
    struct net_br_mld_mc_fdb_entry *dst;
	int ret = 0;
	int filt_mode;

	if(mode == SNOOP_IN_ADD)
	   filt_mode = MCAST_INCLUDE;
	else
	   filt_mode = MCAST_EXCLUDE;
    
	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_entry(dst, &br->mld_mc_list, list) {
	    if ((BCM_IN6_ARE_ADDR_EQUAL(&dst->grp, grp)) &&
            (filt_mode == dst->src_entry.filt_mode) && 
            (dst->from_dev == from_dev) &&
            (dst->dst == prt) &&
            (BCM_IN6_ARE_ADDR_EQUAL(&dst->rep, rep)) && 
	        (BCM_IN6_ARE_ADDR_EQUAL(src, &dst->src_entry.src))) {
	             dst->tstamp = jiffies + (BR_MLD_MEMBERSHIP_TIMEOUT*HZ);
	             ret = 1;
           }
	}
	spin_unlock_bh(&br->mld_mcl_lock);

	return ret;
}

static struct net_br_mld_mc_fdb_entry *br_mld_mc_fdb_get(struct net_bridge *br, 
                                                         struct net_bridge_port *prt, 
                                                         struct in6_addr *grp, 
                                                         struct in6_addr *rep, 
                                                         int mode, 
                                                         struct in6_addr *src,
                                                         struct net_device *from_dev)
{
	struct net_br_mld_mc_fdb_entry *dst;
	int filt_mode;
    
	if(mode == SNOOP_IN_CLEAR)
	   filt_mode = MCAST_INCLUDE;
	else
	   filt_mode = MCAST_EXCLUDE;
          
	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_entry(dst, &br->mld_mc_list, list) {
	    if (BCM_IN6_ARE_ADDR_EQUAL(&dst->grp, grp) && 
            (dst->from_dev == from_dev) &&
		(dst->dst == prt) &&
                (filt_mode == dst->src_entry.filt_mode) && 
                (BCM_IN6_ARE_ADDR_EQUAL(&dst->rep, rep)) &&
                (BCM_IN6_ARE_ADDR_EQUAL(&dst->src_entry.src, src))) {
	            spin_unlock_bh(&br->mld_mcl_lock);
		    return dst;
	    }
	}
	spin_unlock_bh(&br->mld_mcl_lock);
	
	return NULL;
}

int br_mld_mc_fdb_add(struct net_device *from_dev,
                        int wan_ops,
                        struct net_bridge *br, 
                        struct net_bridge_port *prt, 
                        struct in6_addr *grp, 
                        struct in6_addr *rep, 
                        int mode, 
                        int tci, 
                        struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *mc_fdb;

    if(!br || !prt || !grp|| !rep || !from_dev)
        return 0;

    if(!br_mld_control_filter(NULL, grp))
        return 0;

	if(!netdev_path_is_leaf(from_dev))
	    return 0;

    if((SNOOP_IN_ADD != mode) && (SNOOP_EX_ADD != mode))
        return 0;

    if((wan_ops == MCPD_IF_TYPE_BRIDGED) && 
        (!br_mld_is_br_port(br, from_dev)))
        return 0;

	if (br_mld_mc_fdb_update(br, prt, grp, rep, mode, src, from_dev))
	    return 0;

	mc_fdb = kzalloc(sizeof(struct net_br_mld_mc_fdb_entry), GFP_KERNEL);
	if (!mc_fdb)
	    return ENOMEM;
	BCM_IN6_ASSIGN_ADDR(&mc_fdb->grp, grp);
	BCM_IN6_ASSIGN_ADDR(&mc_fdb->rep, rep);
	BCM_IN6_ASSIGN_ADDR(&mc_fdb->src_entry, src);
	mc_fdb->src_entry.filt_mode = 
                  (mode == SNOOP_IN_ADD) ? MCAST_INCLUDE : MCAST_EXCLUDE;
	mc_fdb->dst = prt;
	mc_fdb->tstamp = jiffies + (BR_MLD_MEMBERSHIP_TIMEOUT*HZ);
	mc_fdb->lan_tci = tci;
	mc_fdb->wan_tci = 0;
	mc_fdb->num_tags = 0;
    mc_fdb->from_dev = from_dev;
    memcpy(mc_fdb->wan_name, from_dev->name, IFNAMSIZ);
    memcpy(mc_fdb->lan_name, prt->dev->name, IFNAMSIZ);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
    mc_fdb->blog_idx = 0;
#endif

	spin_lock_bh(&br->mld_mcl_lock);
	list_add_tail(&mc_fdb->list, &br->mld_mc_list);
	spin_unlock_bh(&br->mld_mcl_lock);

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
    br_mcast_blog_process(br, (void *)mc_fdb, BR_MCAST_PROTO_MLD, wan_ops);
#endif

	if (!br->mld_start_timer) {
    	    init_timer(&br->mld_timer);
	    br->mld_timer.expires = jiffies + TIMER_CHECK_TIMEOUT*HZ;
	    br->mld_timer.function = mld_query_timeout;
	    br->mld_timer.data = (unsigned long) br;
	    add_timer(&br->mld_timer);
	    br->mld_start_timer = 1;
	}

	return 1;
}

void br_mld_mc_fdb_cleanup(struct net_bridge *br)
{
	struct net_br_mld_mc_fdb_entry *dst, *dst_n;
    
	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_entry_safe(dst, dst_n, &br->mld_mc_list, list) {
	    list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG) 
        br_mcast_blog_release(BR_MCAST_PROTO_MLD, (void *)dst);
#endif
	    kfree(dst);
	}
	spin_unlock_bh(&br->mld_mcl_lock);
}

void br_mld_mc_fdb_remove_grp(struct net_bridge *br, 
                              struct net_bridge_port *prt, 
                              struct in6_addr *grp)
{
	struct net_br_mld_mc_fdb_entry *dst, *dst_n;

	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_entry_safe(dst, dst_n, &br->mld_mc_list, list) {
	    if ((BCM_IN6_ARE_ADDR_EQUAL(&dst->grp, grp)) && 
	        (dst->dst == prt)) {
		list_del(&dst->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG) 
		    br_mcast_blog_release(BR_MCAST_PROTO_MLD, (void *)dst);
#endif
            kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mld_mcl_lock);
}

int br_mld_mc_fdb_remove(struct net_device *from_dev,
                        int wan_ops,
                        struct net_bridge *br, 
                        struct net_bridge_port *prt, 
                        struct in6_addr *grp, 
                        struct in6_addr *rep, 
                        int mode, 
                        struct in6_addr *src)
{
	struct net_br_mld_mc_fdb_entry *mc_fdb;
	int ret =0;

	if(!br || !prt || !grp|| !rep || !from_dev)
	    return 0;

    if(!br_mld_control_filter(NULL, grp))
        return 0;

	if(!netdev_path_is_leaf(from_dev))
	    return 0;

	if((SNOOP_IN_CLEAR != mode) && (SNOOP_EX_CLEAR != mode))
	   return 0;

	while ((mc_fdb = br_mld_mc_fdb_get(br, prt, grp, rep, mode, src, from_dev))) {
	    spin_lock_bh(&br->mld_mcl_lock);
	    list_del(&mc_fdb->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG) 
        br_mcast_blog_release(BR_MCAST_PROTO_MLD, (void *)mc_fdb);
#endif
	    spin_unlock_bh(&br->mld_mcl_lock);
	    kfree(mc_fdb);

	    ret = 1;
	}
	
	return ret;
}

int br_mld_mc_forward(struct net_bridge *br, 
                      struct sk_buff *skb, 
                      int forward, 
                      int is_routed)
{
	struct net_br_mld_mc_fdb_entry *dst;
	int status = 0;
	struct sk_buff *skb2;
	struct net_bridge_port *p, *p_n;
	unsigned char *mld_type = NULL;
	unsigned char *ipv6_addr = NULL;
	const struct ipv6hdr *pipv6 = ipv6_hdr(skb);
	const unsigned char *dest = eth_hdr(skb)->h_dest;

    if((eth_hdr(skb)->h_proto != ETH_P_IPV6) || (!pipv6))
        return status;

	if (skb->data[6] == IPPROTO_ICMPV6) {
              mld_type = &skb->data[40];
	      ipv6_addr = &skb->data[48];
	} 
	else if ((skb->data[6] == IPPROTO_HOPOPTS) &&
                   (skb->data[40] == IPPROTO_ICMPV6)) {
	      mld_type = &skb->data[48];
	      ipv6_addr = &skb->data[56];
	}

	if((mld_type != NULL) && ((*mld_type == ICMPV6_MGM_REPORT) ||
	   (*mld_type == ICMPV6_MGM_REDUCTION) || 
	   (*mld_type == ICMPV6_MLD2_REPORT))) 
	{
		if(skb->dev && (skb->dev->br_port) &&
		   (br->mld_snooping || br->mld_proxy)) 
		{
			/* for bridged WAN service, do not pass any MLD packets
			   coming from the WAN port to mcpd */
			if ( skb->dev->priv_flags & IFF_WANDEV )
			{
				kfree_skb(skb);
				status = 1;
			}
         else
         {
			   mcpd_process_skb(br, skb);
         }
		}
		return status;
	}

	if (!br->mld_snooping)
		return 0;

	if ((br->mld_snooping == SNOOPING_BLOCKING_MODE) &&
	     br_mld_control_filter(dest, &pipv6->daddr))
		status = 1;

	spin_lock_bh(&br->mld_mcl_lock);
	list_for_each_entry(dst, &br->mld_mc_list, list) {
		if (BCM_IN6_ARE_ADDR_EQUAL(&dst->grp, &pipv6->daddr)) {
			if((dst->src_entry.filt_mode == MCAST_INCLUDE) && 
	         (BCM_IN6_ARE_ADDR_EQUAL(&pipv6->saddr, &dst->src_entry.src))) {

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
	         if((0 == dst->src_entry.src.s6_addr[0]) ||
                    (BCM_IN6_ARE_ADDR_EQUAL(&pipv6->saddr, &dst->src_entry.src))) {

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
	      }
	   }
	}
	spin_unlock_bh(&br->mld_mcl_lock);

	if (status) {
		list_for_each_entry_safe(p, p_n, &br->port_list, list) {
			p->dirty = 0;
		}
	}

	if(status)
		kfree_skb(skb);

	return status;
}

struct net_br_mld_mc_fdb_entry *br_mld_mc_fdb_copy(struct net_bridge *br, 
                                     const struct net_br_mld_mc_fdb_entry *mld_fdb)
{
    struct net_br_mld_mc_fdb_entry *new_mld_fdb = NULL;

	new_mld_fdb = kmalloc(sizeof(struct net_br_mld_mc_fdb_entry), GFP_KERNEL);

	if (new_mld_fdb)
	{
		memcpy(new_mld_fdb, mld_fdb, sizeof(struct net_br_mld_mc_fdb_entry));

		spin_lock_bh(&br->mld_mcl_lock);
		list_add_tail(&new_mld_fdb->list, &br->mld_mc_list);
		spin_unlock_bh(&br->mld_mcl_lock);
	}

    return new_mld_fdb;
} /* br_igmp_mc_fdb_copy */

void br_mld_mc_fdb_del_entry(struct net_bridge *br, 
                              struct net_br_mld_mc_fdb_entry *mld_fdb)
{

    spin_lock_bh(&br->mld_mcl_lock);
    list_del(&mld_fdb->list);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
    br_mcast_blog_release(BR_MCAST_PROTO_MLD, (void *)mld_fdb);
#endif
    kfree(mld_fdb);
    spin_unlock_bh(&br->mld_mcl_lock);

    return;
}


static void *snoop_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net_device *dev;
	loff_t offs = 0;

    read_lock(&dev_base_lock);
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
			return dev;
	}
	return NULL;
}

static int snoop_seq_show(struct seq_file *seq, void *v)
{
	struct net_device *dev = v;
	struct net_br_mld_mc_fdb_entry *dst;
	struct net_bridge *br = netdev_priv(dev);

	seq_printf(seq, "mld snooping %d  proxy %d  lan2lan-snooping %d\n", br->mld_snooping, br->mld_proxy, br_mld_lan2lan_snooping);
	seq_printf(seq, "bridge device src-dev #tags lan-tci    wan-tci");
	seq_printf(seq, "    group                               mode source");
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	seq_printf(seq, "                              timeout reporter");
	seq_printf(seq, "                              Index\n");
#else
	seq_printf(seq, "                              timeout reporter\n");
#endif

	list_for_each_entry(dst, &br->mld_mc_list, list)
	{
		seq_printf(seq, "%-6s %-6s %-7s %02d    0x%08x 0x%08x", 
		           br->dev->name, 
		           dst->dst->dev->name, 
		           dst->from_dev->name, 
		           dst->num_tags,
		           dst->lan_tci,
		           dst->wan_tci);

		seq_printf(seq, " %08x:%08x:%08x:%08x",
		           dst->grp.s6_addr32[0],
		           dst->grp.s6_addr32[1],
		           dst->grp.s6_addr32[2],
		           dst->grp.s6_addr32[3]);

		seq_printf(seq, " %-4s %08x:%08x:%08x:%08x %-7d", 
		           (dst->src_entry.filt_mode == MCAST_EXCLUDE) ? 
		            "EX" : "IN",
		           dst->src_entry.src.s6_addr32[0], 
		           dst->src_entry.src.s6_addr32[1], 
		           dst->src_entry.src.s6_addr32[2], 
		           dst->src_entry.src.s6_addr32[3], 
		           (int) (dst->tstamp - jiffies)/HZ);

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
		seq_printf(seq, " %08x:%08x:%08x:%08x 0x%08x\n", 
		           dst->rep.s6_addr32[0],
		           dst->rep.s6_addr32[1],
		           dst->rep.s6_addr32[2],
		           dst->rep.s6_addr32[3], dst->blog_idx);
#else
		seq_printf(seq, " %08x:%08x:%08x:%08x\n", 
		           dst->rep.s6_addr32[0],
		           dst->rep.s6_addr32[1],
		           dst->rep.s6_addr32[2],
		           dst->rep.s6_addr32[3]);
#endif
	}

	return 0;
}

static void snoop_seq_stop(struct seq_file *seq, void *v)
{
    read_unlock(&dev_base_lock);
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

static struct file_operations br_mld_snoop_proc_fops = {
	.owner = THIS_MODULE,
	.open  = snoop_seq_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

void br_mld_snooping_init(void)
{
	br_mld_entry = proc_create("mld_snooping", 0, init_net.proc_net,
			   &br_mld_snoop_proc_fops);

	if(!br_mld_entry) {
		printk("error while creating mld_snooping proc\n");
	}
}
#endif
