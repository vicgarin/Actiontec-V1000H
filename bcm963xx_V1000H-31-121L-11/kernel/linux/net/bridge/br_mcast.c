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
#include <linux/list.h>
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#include <linux/if_vlan.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#endif
#include <linux/rtnetlink.h>
#include "br_private.h"
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
#include "br_igmp.h"
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
#include "br_mld.h"
#endif


#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)

inline void br_mcast_ipv4_to_eth(unsigned long ipv4_addr,
                                       unsigned char *mac_addr_p)
{
    unsigned char *ipv4_addr8_p = (unsigned char *)(&ipv4_addr);

    mac_addr_p[0] = 0x01;
    mac_addr_p[1] = 0x00;
    mac_addr_p[2] = 0x5E;
    mac_addr_p[3] = ipv4_addr8_p[1] & 0x7F;
    mac_addr_p[4] = ipv4_addr8_p[2];
    mac_addr_p[5] = ipv4_addr8_p[3];
}

inline void br_mcast_ipv6_to_eth(unsigned char *ipv6_addr,
                                       unsigned char *mac_addr_p)
{
    mac_addr_p[0] = 0x33;
    mac_addr_p[1] = 0x33;
    mac_addr_p[2] = *(ipv6_addr + 13);
    mac_addr_p[3] = *(ipv6_addr + 14);
    mac_addr_p[4] = *(ipv6_addr + 15);
    mac_addr_p[5] = *(ipv6_addr + 16);
}

void br_mcast_blog_release(t_BR_MCAST_PROTO_TYPE proto, void *mc_fdb)
{
	Blog_t *blog_p = BLOG_NULL;
    uint32_t blog_idx = 0;
    BlogTraffic_t traffic;

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
    if(proto == BR_MCAST_PROTO_IGMP)
    {
	    blog_idx =  ((struct net_bridge_mc_fdb_entry *)mc_fdb)->blog_idx;
        traffic = BlogTraffic_IPV4_MCAST;
    }
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    if(proto == BR_MCAST_PROTO_MLD)
    {
	    blog_idx =  ((struct net_br_mld_mc_fdb_entry *)mc_fdb)->blog_idx;
        traffic = BlogTraffic_IPV6_MCAST;
    }
#endif

	if(!blog_idx)
	    return;

	blog_p = blog_deactivate(blog_idx, traffic);
	if ( blog_p )
	{
		blog_rule_free_list(blog_p);
		blog_put(blog_p);
	}

	return;
}

static void br_mcast_blog_process_wan(blogRule_t *rule_p,
                                     void *mc_fdb,
                                     t_BR_MCAST_PROTO_TYPE proto,
                                     int wan_ops,
                                     struct net_device **wan_dev_pp,
                                     struct net_device **wan_vlan_dev_pp)
{
    blogRuleAction_t ruleAction;
    struct net_device *dev_p = NULL;
    struct net_bridge_mc_fdb_entry *igmp_fdb = NULL;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    struct net_br_mld_mc_fdb_entry *mld_fdb = NULL;
#endif
	uint8_t                *dev_addr = NULL;
    uint32_t phyType;

    if(!mc_fdb)
        return;

    if(BR_MCAST_PROTO_IGMP == proto)
    {
        igmp_fdb = (struct net_bridge_mc_fdb_entry *)mc_fdb;

        dev_p = igmp_fdb->from_dev;

        dev_addr = igmp_fdb->dst->dev->dev_addr;
    }
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else
    {
        mld_fdb = (struct net_br_mld_mc_fdb_entry *)mc_fdb;

        dev_p = mld_fdb->from_dev;

        dev_addr = mld_fdb->dst->dev->dev_addr;
    }
#endif


    while(1)
    {
        if(netdev_path_is_root(dev_p))
        {
            *wan_dev_pp = dev_p;
            break;
        }

        if(dev_p->priv_flags & IFF_PPP)
        {
            rule_p->filter.hasPppoeHeader = 1;
            memset(&ruleAction, 0, sizeof(blogRuleAction_t));
            ruleAction.cmd = BLOG_RULE_CMD_POP_PPPOE_HDR;
            blog_rule_add_action(rule_p, &ruleAction);

            memset(&ruleAction, 0, sizeof(blogRuleAction_t));
            ruleAction.cmd = BLOG_RULE_CMD_SET_MAC_DA;
            if(igmp_fdb)
                br_mcast_ipv4_to_eth(igmp_fdb->grp.s_addr, ruleAction.macAddr);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
            else
                br_mcast_ipv6_to_eth(mld_fdb->grp.s6_addr, ruleAction.macAddr);
#endif
            blog_rule_add_action(rule_p, &ruleAction);
        }
        else if(*wan_vlan_dev_pp == NULL &&
                dev_p->priv_flags & IFF_BCM_VLAN)
        {
            *wan_vlan_dev_pp = dev_p;
        }

        dev_p = netdev_path_next_dev(dev_p);
    }

    /* For IPoA */
    phyType = netdev_path_get_hw_port_type(*wan_dev_pp);
    phyType = BLOG_GET_HW_ACT(phyType);
    if((phyType == VC_MUX_IPOA) || (phyType == LLC_SNAP_ROUTE_IP))
    {
        memset(&ruleAction, 0, sizeof(blogRuleAction_t));
        ruleAction.cmd = BLOG_RULE_CMD_SET_MAC_DA;
        if(igmp_fdb)
            br_mcast_ipv4_to_eth(igmp_fdb->grp.s_addr, ruleAction.macAddr);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
        else
            br_mcast_ipv6_to_eth(mld_fdb->grp.s6_addr, ruleAction.macAddr);
#endif
        blog_rule_add_action(rule_p, &ruleAction);
    }

    if(wan_ops == MCPD_IF_TYPE_ROUTED)
    {
        memset(&ruleAction, 0, sizeof(blogRuleAction_t));
        ruleAction.cmd = BLOG_RULE_CMD_SET_MAC_SA;
        memcpy(ruleAction.macAddr, dev_addr, ETH_ALEN);
        blog_rule_add_action(rule_p, &ruleAction);

        memset(&ruleAction, 0, sizeof(blogRuleAction_t));
        ruleAction.cmd = BLOG_RULE_CMD_DECR_TTL;
        blog_rule_add_action(rule_p, &ruleAction);
    }
}

static void br_mcast_blog_process_lan(void *mc_fdb,
                                     t_BR_MCAST_PROTO_TYPE proto,
                                     struct net_device **lan_dev_pp,
                                     struct net_device **lan_vlan_dev_pp)
{
    struct net_device *dev_p = NULL;
    struct net_bridge_mc_fdb_entry *igmp_fdb = NULL;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    struct net_br_mld_mc_fdb_entry *mld_fdb = NULL;
#endif

    if(!mc_fdb)
        return;

    if(BR_MCAST_PROTO_IGMP == proto)
    {
        igmp_fdb = (struct net_bridge_mc_fdb_entry *)mc_fdb;

        dev_p = igmp_fdb->dst->dev;
    }
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else
    {
        mld_fdb = (struct net_br_mld_mc_fdb_entry *)mc_fdb;

        dev_p = mld_fdb->dst->dev;
    }
#endif

    while(1)
    {
        if(netdev_path_is_root(dev_p))
        {
            *lan_dev_pp = dev_p;
            break;
        }

        if(*lan_vlan_dev_pp == NULL &&
           dev_p->priv_flags & IFF_BCM_VLAN)
        {
            *lan_vlan_dev_pp = dev_p;
        }

        dev_p = netdev_path_next_dev(dev_p);
    }
}

int br_mcast_blog_rule_update(void *mc_fdb,
                                t_BR_MCAST_PROTO_TYPE proto,
                                int wan_ops)
{
    Blog_t *blog_p = BLOG_NULL;
    blogRule_t *rule_p = NULL;
    struct net_device *wan_vlan_dev_p = NULL;
    struct net_device *lan_vlan_dev_p = NULL;
    struct net_device *wan_dev_p = NULL;
    struct net_device *lan_dev_p = NULL;
    int ret = 0;
    struct net_bridge_mc_fdb_entry *igmp_fdb = NULL;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    struct net_br_mld_mc_fdb_entry *mld_fdb = NULL;
#endif
	uint32_t                blog_idx = 0;
	struct net_device      *from_dev = NULL;


    if(!mc_fdb)
        return -1;

    if(BR_MCAST_PROTO_IGMP == proto)
    {
        igmp_fdb = (struct net_bridge_mc_fdb_entry *)mc_fdb;

        if((!igmp_fdb) || ((igmp_fdb) && (!igmp_fdb->blog_idx)))
            return -1;

	    if((igmp_fdb) && (!br_igmp_control_filter(NULL, igmp_fdb->grp.s_addr)))
	        return -1;

        blog_idx = igmp_fdb->blog_idx;
        from_dev = igmp_fdb->from_dev;
    }
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else
    {
        mld_fdb = (struct net_br_mld_mc_fdb_entry *)mc_fdb;

        if((!mld_fdb) || ((mld_fdb) && (!mld_fdb->blog_idx)))
            return -1;

        blog_idx = mld_fdb->blog_idx;
        from_dev = mld_fdb->from_dev;
    }
#endif


    /* allocate blog */
    blog_p = blog_get();

    if(blog_p == BLOG_NULL) 
    {
        return -1;
    }
    
    rule_p = blog_rule_alloc();

    if(rule_p == NULL)
    {
	    blog_put(blog_p);
        return -1;
    }

    blog_rule_init(rule_p);

    blog_p->blogRule_p = (void *)rule_p;

    /* for lan2lan mcast don't do anything */
    if((from_dev->name[0] != 'b') && (from_dev->name[1] != 'r'))
    {
        /* find WAN devices */
        br_mcast_blog_process_wan(rule_p, mc_fdb, proto, wan_ops,
                                 &wan_dev_p, &wan_vlan_dev_p);
    }

    /* find LAN devices */
    br_mcast_blog_process_lan(mc_fdb, proto, &lan_dev_p, &lan_vlan_dev_p);

     /* add vlan blog rules, if any vlan interfaces were found */
    if(blogRuleVlanHook && (wan_vlan_dev_p || lan_vlan_dev_p)) 
    {
        if(blogRuleVlanHook(blog_p, wan_vlan_dev_p, lan_vlan_dev_p) < 0)
        {
            printk(KERN_WARNING "Error while processing VLAN blog rules\n");
            blog_rule_free_list(blog_p);
            blog_put(blog_p);
            return -1;
        }
    }

    blog_notify(MCAST_SYNC_EVT, (void *)rule_p, blog_idx, 0);

    blog_put(blog_p);

    return ret;
}

void br_mcast_vlan_notify_for_blog_update(struct net_device *ndev,
                                   blogRuleVlanNotifyDirection_t direction,
                                   uint32_t nbrOfTags)
{
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
    struct net_bridge_mc_fdb_entry *mc_fdb;
#endif
    struct net_bridge *br = NULL;
    struct net_device *dev = NULL;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    struct net_br_mld_mc_fdb_entry *mld_mc_fdb;
#endif

    if((ndev->priv_flags & IFF_WANDEV) && (direction == DIR_TX))
        return;

    rtnl_lock();
    ASSERT_RTNL();
    for(dev = first_net_device(&init_net); 
        dev; 
        dev = next_net_device(dev)) 
    {
        br = netdev_priv(dev);
        if((dev->priv_flags & IFF_EBRIDGE) && (br))
        {
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
            if(br->igmp_snooping) 
            {
                spin_lock_bh(&br->lock);
                spin_lock_bh(&br->mcl_lock);
                list_for_each_entry(mc_fdb, &br->mc_list, list) 
                { 
                    if(((mc_fdb->dst->dev == ndev) ||
                        (mc_fdb->from_dev == ndev)) &&
                        (mc_fdb->blog_idx))
                    {
                        br_mcast_blog_rule_update((void *)mc_fdb,
                                                    BR_MCAST_PROTO_IGMP,
                                                    MCPD_IF_TYPE_BRIDGED);
                    }
                }
                spin_unlock_bh(&br->mcl_lock);
                spin_unlock_bh(&br->lock);
            }
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
            if(br->mld_snooping)
            {
                spin_lock_bh(&br->lock);
                spin_lock_bh(&br->mld_mcl_lock);
                list_for_each_entry(mld_mc_fdb, &br->mld_mc_list, list) 
                { 
                    if(((mld_mc_fdb->dst->dev == ndev) ||
                        (mld_mc_fdb->from_dev == ndev)) &&
                        (mld_mc_fdb->blog_idx))
                    {
                        br_mcast_blog_rule_update((void *)mld_mc_fdb,
                                                    BR_MCAST_PROTO_MLD,
                                                    MCPD_IF_TYPE_BRIDGED);
                    }
                }
                spin_unlock_bh(&br->mld_mcl_lock);
                spin_unlock_bh(&br->lock);
            }
#endif /* CONFIG_BR_MLD_SNOOP */
        }
    }
    rtnl_unlock();

    return;
}

void br_mcast_handle_netdevice_events(struct net_device *ndev, unsigned long event)
{
    struct net_bridge *br = NULL;
    struct net_device *dev = NULL;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
    struct net_bridge_mc_fdb_entry *dst, *dst_n;
    struct net_bridge_mc_rep_entry *rep_entry, *rep_entry_n;
#endif

    switch (event) {
        case NETDEV_GOING_DOWN:
        case NETDEV_CHANGE:
            for(dev = first_net_device(&init_net); 
                dev; 
                dev = next_net_device(dev)) {
                br = netdev_priv(dev);
                if((dev->priv_flags & IFF_EBRIDGE) && (br)) {
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
                    if(br->igmp_snooping) {
                        spin_lock_bh(&br->lock);
                        spin_lock_bh(&br->mcl_lock);
                        list_for_each_entry_safe(dst, dst_n, &br->mc_list, list) {
                        if((!memcmp(ndev->name, dst->wan_name, IFNAMSIZ)) ||
                            (!memcmp(ndev->name, dst->lan_name, IFNAMSIZ)) ||
                            (!memcmp(ndev->name, dev->name, IFNAMSIZ))) {
                                mcpd_nl_send_igmp_purge_entry(dst);
                                list_for_each_entry_safe(rep_entry, 
                                         rep_entry_n, &dst->rep_list, list) {
                                    list_del(&rep_entry->list);
                                    kfree(rep_entry);
                                }
                                list_del(&dst->list);
                                br_mcast_blog_release(BR_MCAST_PROTO_IGMP, dst);
                                kfree(dst);
                            }
                        }
                        spin_unlock_bh(&br->mcl_lock);
                        spin_unlock_bh(&br->lock);
                    }
#endif
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
                    if(br->mld_snooping)
                    {
                        struct net_br_mld_mc_fdb_entry *mld_dst, *mld_dst_n;
                        spin_lock_bh(&br->lock);
                        spin_lock_bh(&br->mld_mcl_lock);
                        list_for_each_entry_safe(mld_dst, mld_dst_n, &br->mld_mc_list, list) {
                        if((!memcmp(ndev->name, mld_dst->wan_name, IFNAMSIZ)) ||
                            (!memcmp(ndev->name, mld_dst->lan_name, IFNAMSIZ)) ||
                            (!memcmp(ndev->name, dev->name, IFNAMSIZ))) {
                                list_del(&mld_dst->list);
                                br_mcast_blog_release(BR_MCAST_PROTO_MLD, mld_dst);
                                kfree(mld_dst);
                            }
                        }
                        spin_unlock_bh(&br->mld_mcl_lock);
                        spin_unlock_bh(&br->lock);
                    }
#endif
                }
            }
            break;
    }

    return;
}

static void *br_mcast_mc_fdb_copy(t_BR_MCAST_PROTO_TYPE proto,
								struct net_bridge *br, 
                                const void *mc_fdb)
{
    if(!mc_fdb)
        return NULL;

    if(BR_MCAST_PROTO_IGMP == proto)
        return br_igmp_mc_fdb_copy(br, (struct net_bridge_mc_fdb_entry *)mc_fdb);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else if(BR_MCAST_PROTO_MLD == proto)
        return br_mld_mc_fdb_copy(br, (struct net_br_mld_mc_fdb_entry *)mc_fdb);
#endif

	return NULL;
}

static void br_mcast_mc_fdb_del_entry(t_BR_MCAST_PROTO_TYPE proto, 
                               struct net_bridge *br, 
                               void *mc_fdb)
{

    if(!mc_fdb)
        return;

    if(BR_MCAST_PROTO_IGMP == proto)
        br_igmp_mc_fdb_del_entry(br, (struct net_bridge_mc_fdb_entry *)mc_fdb);
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else if(BR_MCAST_PROTO_MLD == proto)
        br_mld_mc_fdb_del_entry(br, (struct net_br_mld_mc_fdb_entry *)mc_fdb);
#endif

    return;
} /* br_mcast_mc_fdb_del_entry */

void br_mcast_vlan_process(struct net_bridge *br,
                           void *mc_fdb,
                           t_BR_MCAST_PROTO_TYPE proto,
                           Blog_t *blog_p)
{
    Blog_t *new_blog_p;
    void *new_mc_fdb = NULL;
    blogRule_t *rule_p = NULL;
    blogRule_t *n_rule_p = NULL;
    uint32_t vid = 0;
    blogRuleFilter_t *rule_filter = NULL;

    if(!mc_fdb || !blog_p || !br)
        return;

    if(!blog_p->blogRule_p)
        return;

    if(!((BR_MCAST_PROTO_IGMP == proto) || (BR_MCAST_PROTO_MLD == proto)))
        return;

    /* no multiple vlan rules */
    if(((blogRule_t *)blog_p->blogRule_p)->next_p == NULL)
        return;

    for(rule_p = ((blogRule_t *)blog_p->blogRule_p)->next_p; rule_p; )
    {
        n_rule_p = rule_p->next_p;
		new_mc_fdb = br_mcast_mc_fdb_copy(proto, br , mc_fdb);

		if(!new_mc_fdb)
		{
			printk(KERN_WARNING "%s new_mc_fdb allocation failed\n",__FUNCTION__);
			return;
		}

	    new_blog_p = blog_get();

		if(new_blog_p == BLOG_NULL) {
			printk(KERN_WARNING "%s new_blog_p allocation failed\n",__FUNCTION__);
			br_mcast_mc_fdb_del_entry(proto, br, new_mc_fdb);
			return;
		}

        blog_copy(new_blog_p, blog_p);

        rule_p->next_p = NULL;

        new_blog_p->blogRule_p = rule_p;

        rule_filter = &(((blogRule_t *)new_blog_p->blogRule_p)->filter);
        new_blog_p->vtag_num = rule_filter->nbrOfVlanTags;
        vid = ((rule_filter->vlan[0].value.h_vlan_TCI &
                rule_filter->vlan[0].mask.h_vlan_TCI) & 0xFFF);
        new_blog_p->vid = vid ? vid : 0xFFFF; 
        vid = ((rule_filter->vlan[1].value.h_vlan_TCI &
                rule_filter->vlan[1].mask.h_vlan_TCI) & 0xFFF);
        new_blog_p->vid |= vid ? (vid << 16) : 0xFFFF0000;

        blog_link(MCAST_FDB, new_blog_p, (void *)new_mc_fdb, 0, 0);

        {
            BlogTraffic_t traffic;

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
            if(BR_MCAST_PROTO_MLD == proto)
            {
                traffic = BlogTraffic_IPV6_MCAST;
                ((struct net_br_mld_mc_fdb_entry *)new_mc_fdb)->wan_tci = new_blog_p->vid;
                ((struct net_br_mld_mc_fdb_entry *)new_mc_fdb)->num_tags = new_blog_p->vtag_num;
            }
            else//IGMP
#endif
            {
                traffic = BlogTraffic_IPV4_MCAST;
                ((struct net_bridge_mc_fdb_entry *)new_mc_fdb)->wan_tci = new_blog_p->vid;
                ((struct net_bridge_mc_fdb_entry *)new_mc_fdb)->num_tags = new_blog_p->vtag_num;
            }

            if ( blog_activate(new_blog_p, traffic) == BLOG_KEY_INVALID )
            {
                blog_rule_free_list(new_blog_p);
                blog_put(new_blog_p);
            }
        }

        rule_p = n_rule_p;
    }

    ((blogRule_t *)blog_p->blogRule_p)->next_p = NULL;

    return;
} /* br_mcast_vlan_process */


int br_mcast_blog_process(struct net_bridge *br,
                            void *mc_fdb,
                            t_BR_MCAST_PROTO_TYPE proto,
                            int wan_ops)
{
	Blog_t *blog_p = BLOG_NULL;
	blogRule_t *rule_p = NULL;
	struct net_device *wan_vlan_dev_p = NULL;
	struct net_device *lan_vlan_dev_p = NULL;
	struct net_device *wan_dev_p = NULL;
	struct net_device *lan_dev_p = NULL;
    struct net_bridge_mc_fdb_entry *igmp_fdb = NULL;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    struct net_br_mld_mc_fdb_entry *mld_fdb = NULL;
#endif
    uint8_t *dev_addr = NULL;
    struct net_device *from_dev = NULL;
    uint32_t vid = 0;
    uint32_t phyType;
    blogRuleFilter_t *rule_filter = NULL;

    if(!mc_fdb)
        return -1;

    if(BR_MCAST_PROTO_IGMP == proto)
    {
        igmp_fdb = (struct net_bridge_mc_fdb_entry *)mc_fdb;

        from_dev = igmp_fdb->from_dev;

        dev_addr = igmp_fdb->dst->dev->dev_addr;

    }
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else
    {
        mld_fdb = (struct net_br_mld_mc_fdb_entry *)mc_fdb;

        from_dev = mld_fdb->from_dev;

        dev_addr = mld_fdb->dst->dev->dev_addr;
    }
#endif

	/* allocate blog */
	blog_p = blog_get();

	if(blog_p == BLOG_NULL) {
		printk(KERN_WARNING "%s blog_p allocation failed\n",__FUNCTION__);
		return -1;
	}

        /* allocate blog rule */
	rule_p = blog_rule_alloc();

	if(rule_p == NULL)
	{
		printk(KERN_WARNING "%s blog_rule allocation failed\n",__FUNCTION__);
		blog_put(blog_p);
		return -1;
	}

    blog_rule_init(rule_p);

	blog_p->blogRule_p = (void *)rule_p;

    /* for LAN2LAN don't do anything */
    if(br->dev != from_dev) 
    {
        /* find WAN devices */
        br_mcast_blog_process_wan(rule_p, mc_fdb, proto, wan_ops,
                                 &wan_dev_p, &wan_vlan_dev_p);
    }

    /* find LAN devices */
    br_mcast_blog_process_lan(mc_fdb, proto, &lan_dev_p, &lan_vlan_dev_p);

    /* for LAN2LAN don't do anything */
    if(br->dev == from_dev) 
    {
        blog_p->rx.info.phyHdr = 0;
        blog_p->rx.info.channel = 0xFF; /* for lan2lan mcast */
        blog_p->rx.info.bmap.BCM_SWC = 1;

    }
    else
    {
        phyType = netdev_path_get_hw_port_type(wan_dev_p);
        blog_p->rx.info.phyHdr = (phyType & BLOG_PHYHDR_MASK);
        phyType = BLOG_GET_HW_ACT(phyType);

        if(blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
        {
            unsigned int hw_subport_mcast;
            unsigned int mask;
            int i;

            hw_subport_mcast = netdev_path_get_hw_subport_mcast(wan_dev_p);

            if(hw_subport_mcast)
            {
                for(i=0; i<(sizeof(unsigned int) * 8); ++i)
                {
                    mask = (unsigned int)(1 << i);

                    if(hw_subport_mcast & mask)
                    {
                        /* currently we only support one GEM port (source subport) */
                        blog_p->rx.info.channel = i;

                        if(hw_subport_mcast & ~mask)
                        {
                            printk("Warning: Multiple GPON multicast GEM sources "
                                   "<0x%08X>, selected <%u>\n",
                                   hw_subport_mcast, blog_p->rx.info.channel);
                        }

                        break;
                    }
                }
            }
            else
            {
                /* Not a GPON Multicast WAN device */
                blog_rule_free_list(blog_p);
                blog_put(blog_p);
                return -1;
            }
        }
        else /* Ethernet or DSL WAN device */
        {
            blog_p->rx.info.channel = netdev_path_get_hw_port(wan_dev_p);
        }

        if( (blog_p->rx.info.phyHdrType == BLOG_XTMPHY) &&
            ((phyType == VC_MUX_PPPOA) ||
             (phyType == VC_MUX_IPOA) ||
	     (phyType == LLC_SNAP_ROUTE_IP) ||
             (phyType == LLC_ENCAPS_PPP)) )
        {
            blog_p->insert_eth = 1;
        }

        if( (blog_p->rx.info.phyHdrType == BLOG_XTMPHY) &&
            ((phyType == VC_MUX_PPPOA) ||
             (phyType == LLC_ENCAPS_PPP)) )
        {
            blog_p->pop_pppoa = 1;
        }

#if defined(CONFIG_BCM96816)
        blog_p->rx.info.bmap.BCM_SWC = 1;
#else
        if(blog_p->rx.info.phyHdrType == BLOG_ENETPHY)
        {
            blog_p->rx.info.bmap.BCM_SWC = 1;
        }
        else
        {
            blog_p->rx.info.bmap.BCM_XPHY = 1;
        }
#endif
    }

	blog_p->tx.info.bmap.BCM_SWC = 1;

    blog_p->key.l1_tuple.phy = blog_p->rx.info.phyHdr;
    blog_p->key.l1_tuple.channel = blog_p->rx.info.channel;
    blog_p->key.protocol = BLOG_IPPROTO_UDP;

    phyType = netdev_path_get_hw_port_type(lan_dev_p);
    blog_p->tx.info.phyHdr = (phyType & BLOG_PHYHDR_MASK);
    blog_p->tx.info.channel = netdev_path_get_hw_port(lan_dev_p);

    if(BR_MCAST_PROTO_IGMP == proto)
    {
	    blog_p->rx.tuple.saddr = igmp_fdb->src_entry.src.s_addr;
	    blog_p->rx.tuple.daddr = igmp_fdb->grp.s_addr;
	    blog_p->tx.tuple.saddr = igmp_fdb->src_entry.src.s_addr;
	    blog_p->tx.tuple.daddr = igmp_fdb->grp.s_addr;
#if defined(AEI_VDSL_MC_SSM_HIT) && defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
            if(igmp_fdb->saddr != 0)
            {
                blog_p->rx.tuple.saddr = igmp_fdb->saddr;
                blog_p->tx.tuple.saddr = igmp_fdb->saddr;
            }
#endif
    }
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else
    {
        BCM_IN6_ASSIGN_ADDR(&blog_p->tupleV6.saddr, &mld_fdb->src_entry.src);
        BCM_IN6_ASSIGN_ADDR(&blog_p->tupleV6.daddr, &mld_fdb->grp);
    }
#endif

	blog_p->rx.dev_p = wan_dev_p;
	blog_p->rx.info.multicast = 1;

    if(BR_MCAST_PROTO_IGMP == proto)
        blog_p->rx.info.bmap.L3_IPv4 = 1;
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
    else
        blog_p->rx.info.bmap.L3_IPv6 = 1;
#endif

	blog_p->tx.dev_p = lan_dev_p;

    /* add vlan blog rules, if any vlan interfaces were found */
    if(blogRuleVlanHook && (wan_vlan_dev_p || lan_vlan_dev_p)) {
        if(blogRuleVlanHook(blog_p, wan_vlan_dev_p, lan_vlan_dev_p) < 0) {
            printk(KERN_WARNING "Error while processing VLAN blog rules\n");
            blog_rule_free_list(blog_p);
            blog_put(blog_p);
            return -1;
        }
    }

    br_mcast_vlan_process(br, mc_fdb, proto, blog_p);

    if(((BR_MCAST_PROTO_IGMP == proto) || (BR_MCAST_PROTO_MLD == proto)) && (blog_p->blogRule_p))
    {
        rule_filter = &(((blogRule_t *)blog_p->blogRule_p)->filter);
        blog_p->vtag_num = rule_filter->nbrOfVlanTags;
        vid = ((rule_filter->vlan[0].value.h_vlan_TCI &
                rule_filter->vlan[0].mask.h_vlan_TCI) & 0xFFF);
        blog_p->vid = vid ? vid : 0xFFFF; 
        vid = ((rule_filter->vlan[1].value.h_vlan_TCI &
                rule_filter->vlan[1].mask.h_vlan_TCI) & 0xFFF);
        blog_p->vid |= vid ? (vid << 16) : 0xFFFF0000;
    }
    else
    {
        blog_p->vid = 0xFFFFFFFF;

    }

	blog_link(MCAST_FDB, blog_p, (void *)mc_fdb, 0, 0);

    {
        BlogTraffic_t traffic;

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
        if(BR_MCAST_PROTO_MLD == proto)
        {
            traffic = BlogTraffic_IPV6_MCAST;
            mld_fdb->wan_tci = blog_p->vid;
            mld_fdb->num_tags = blog_p->vtag_num;
        }
        else
#endif
        {
            traffic = BlogTraffic_IPV4_MCAST;
            igmp_fdb->wan_tci = blog_p->vid;
            igmp_fdb->num_tags = blog_p->vtag_num;
        }

        if ( blog_activate(blog_p, traffic) == BLOG_KEY_INVALID )
        {
            blog_rule_free_list(blog_p);
            blog_put(blog_p);
        }
    }
    
	return 0;
} /* br_mcast_blog_process */
#endif
