#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/if.h>
#include "br_igmp.h"
#include "br_mld.h"

#define MAX_PAYLOAD             1024

#define MCPD_MAX_IFS            10

#if 0
static DEFINE_RWLOCK(mcpd_queue_lock);
static LIST_HEAD(mcpd_queue_list);
static DEFINE_MUTEX(mcpd_mutex);
static unsigned int mcpd_queue_total;
static unsigned int mcpd_queue_dropped = 0;
static unsigned int mcpd_queue_user_dropped = 0;
static unsigned int mcpd_queue_maxlen = 64;
#endif

static struct sock *nl_sk = NULL;
static int mcpd_pid = 0;

#define MCPD_SNOOP_IN_ADD    1
#define MCPD_SNOOP_IN_CLEAR  2
#define MCPD_SNOOP_EX_ADD    3
#define MCPD_SNOOP_EX_CLEAR  4


struct mcpd_queue_entry 
{
    struct list_head list;
    struct sk_buff *skb;
    struct net_bridge *br;
};

typedef struct mcpd_msg_hdr 
{
    __u16 type;
    __u16 len;
} t_MCPD_MSG_HDR;

typedef enum mcpd_msgtype 
{
    MCDP_MSG_BASE = 0,
    MCPD_MSG_REGISTER, /* usr - > krnl -> usr */
    MCPD_MSG_VERDICT,
    MCPD_MSG_IGMP_PKT, /* krnl -> usr */
    MCPD_MSG_IGMP_SNOOP_ENTRY,
    MCPD_MSG_IGMP_SNOOP_ADD,
    MCPD_MSG_IGMP_SNOOP_DEL,
    MCPD_MSG_IGMP_LAN2LAN_SNOOP,
    MCPD_MSG_MLD_PKT, /* krnl -> usr */
    MCPD_MSG_MLD_SNOOP_ENTRY,
    MCPD_MSG_MLD_SNOOP_ADD,
    MCPD_MSG_MLD_SNOOP_DEL,
    MCPD_MSG_MLD_LAN2LAN_SNOOP,
    MCPD_MSG_IGMP_PURGE_ENTRY,
    MCPD_MSG_MLD_PURGE_ENTRY,
    MCPD_MSG_IF_CHANGE,
    MCPD_MSG_MC_FDB_CLEANUP, /* clean up for MIB RESET */
    MCPD_MSG_MAX
} t_MCPD_MSGTYPES;

typedef enum mcpd_ret_codes 
{
    MCPD_SUCCESS = 0,
    MCPD_GEN_ERR = 1,
    MCPD_RET_MEMERR = 2,
    MCPD_RET_ACCEPT = 3,
    MCPD_RET_DROP   = 4
} t_MCPD_RET_CODE;

typedef enum mcpd_proto_type
{
    MCPD_PROTO_IGMP = 0,
    MCPD_PROTO_MLD  = 1,
    MCPD_PROTO_MAX  = 2,
} t_MCPD_PROTO_TYPE;

typedef struct mcpd_register 
{
    int code;
} t_MCPD_REGISTER;

typedef struct mcpd_verdict
{
    __u32                     corr_tag;
    __u32                     code;
} t_MCPD_VERDICT;

typedef struct mcpd_pkt_info
{
    char                      br_name[IFNAMSIZ];
    char                      port_name[IFNAMSIZ];
    int                       port_no;
    int                       if_index;
    int                       data_len;
    __u32                     corr_tag;
    int                       is_new_grp_pkt;/* only used in user space */
    __u16                     tci;/* vlan id */
} t_MCPD_PKT_INFO;

typedef struct mcpd_wan_info
{
    char                      if_name[IFNAMSIZ];
    int                       if_ops;
} t_MCPD_WAN_INFO;

typedef struct mcpd_igmp_snoop_entry 
{
    char                      br_name[IFNAMSIZ];
    //char                      port_name[IFNAMSIZ];
    int                       port_no;
    struct                    in_addr grp;
    struct                    in_addr src;
    struct                    in_addr rep;
    int                       mode;
    int                       code;
    __u16                     tci;/* vlan id */
    t_MCPD_WAN_INFO           wan_info[MCPD_MAX_IFS];
} t_MCPD_IGMP_SNOOP_ENTRY;

typedef struct mcpd_mld_snoop_entry 
{
    char                      br_name[IFNAMSIZ];
    char                      port_no;
    struct                    in6_addr grp;
    struct                    in6_addr src;
    struct                    in6_addr rep;
    int                       mode;
    int                       code;
    __u16                     tci;/* vlan id */
    t_MCPD_WAN_INFO           wan_info[MCPD_MAX_IFS];
} t_MCPD_MLD_SNOOP_ENTRY;

typedef struct mcpd_igmp_purge_entry
{
    struct in_addr            grp;
    struct in_addr            src;
    struct in_addr            rep;
    t_MCPD_PKT_INFO           pkt_info;
} t_MCPD_IGMP_PURGE_ENTRY;

static void mcpd_dump_buf(char *buf, int len)
{
#if 0
    int i;
    printk("========================KRNEL BPEELA START===================================\n");
    for(i =0; i < len; i++) 
    {
     printk("%02x", (unsigned char)buf[i]);
     if(!((i+1)%2))
         printk(" ");
     if(!((i+1)%16))
       printk("\n");
    }
    printk("\n");
    printk("=======================KRNL BPEELA END====================================\n");
#endif
}

#ifdef dead_debug_code
/* called under bridge lock */
/* this function is not used anywhere.  debug code only? */
static struct net_bridge_port *br_get_port_by_name(struct net_bridge *br, 
                                                   char* port_name)
{
    struct net_bridge_port *p;

    rcu_read_lock();
    list_for_each_entry_rcu(p, &br->port_list, list) {
        if (!memcmp(p->dev->name, port_name, IFNAMSIZ))
            return p;
    }
    rcu_read_unlock();

    return NULL;
}
#endif /* dead_debug_code */

#if 0
static int mcpd_enqueue_packet(struct net_bridge *br, struct sk_buff *skb)
{
    int status = 0;
    struct mcpd_queue_entry *entry;

    /* TODO: this code should be above */
    if (mcpd_queue_total >= mcpd_queue_maxlen) 
    {
        mcpd_queue_dropped++;
        mcpd_queue_user_dropped++;
        status = -ENOSPC;
        if (net_ratelimit())
        printk (KERN_WARNING "full at %d entries, " 
                              "dropping packets(s). Dropped: %d\n", 
                              mcpd_queue_total, mcpd_queue_dropped);
        goto err_code1;
    }

    entry = kmalloc(sizeof(struct mcpd_queue_entry), GFP_ATOMIC);

    if (entry == NULL) 
    {
        printk(KERN_ERR "mcpd_enqueue_packet()\n");
        return -ENOMEM;
    }

    entry->skb = skb;
    entry->br = br;

    write_lock_bh(&mcpd_queue_lock);
    list_add(&entry->list, &mcpd_queue_list);
    write_unlock_bh(&mcpd_queue_lock);

    mcpd_queue_total++;

    err_code1:

    return status;

} /* mcpd_enqueue_packet */
#endif

int mcpd_process_skb(struct net_bridge *br, struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    char *ptr = NULL;
    struct sk_buff *new_skb;
    t_MCPD_PKT_INFO *pkt_info;
    int buf_size;
    char *br_name = br->dev->name;
    int if_index = br->dev->ifindex;
    int port_no = skb->dev->br_port->port_no;

    if(!mcpd_pid)
        return 0;

    buf_size = skb->len + sizeof(t_MCPD_MSG_HDR) + sizeof(t_MCPD_PKT_INFO);
    new_skb = alloc_skb(NLMSG_SPACE(buf_size), GFP_ATOMIC);

    if(!new_skb) {
        printk("br_netlink_mcpd.c:%d %s() errr no mem\n", __LINE__, __FUNCTION__);
        return 0;
    }

    nlh = (struct nlmsghdr *)new_skb->data;
    ptr = NLMSG_DATA(nlh);
    nlh->nlmsg_len = NLMSG_SPACE(buf_size);
    nlh->nlmsg_pid = 0;
    nlh->nlmsg_flags = 0;
    skb_put(new_skb, NLMSG_SPACE(buf_size));
    if(skb->protocol == 0x86DD)
        ((t_MCPD_MSG_HDR *)ptr)->type = MCPD_MSG_MLD_PKT;
    else
        ((t_MCPD_MSG_HDR *)ptr)->type = MCPD_MSG_IGMP_PKT;
    ((t_MCPD_MSG_HDR *)ptr)->len = sizeof(t_MCPD_PKT_INFO);
    ptr += sizeof(t_MCPD_MSG_HDR);

    pkt_info = (t_MCPD_PKT_INFO *)ptr;

    memcpy(pkt_info->br_name, br_name, IFNAMSIZ);
    memcpy(pkt_info->port_name, skb->dev->name, IFNAMSIZ);
    pkt_info->port_no = port_no;
    pkt_info->if_index = if_index;
    pkt_info->data_len = skb->len;
    pkt_info->corr_tag = (__u32)skb;
    pkt_info->tci = 0;

#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
    if(skb->vlan_count)
        pkt_info->tci = (skb->vlan_header[0] >> 16);
#endif /*  defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE) */
    ptr += sizeof(t_MCPD_PKT_INFO);

    memcpy(ptr, skb->data, skb->len);

    NETLINK_CB(new_skb).dst_group = 0;
    NETLINK_CB(new_skb).pid = mcpd_pid;
    mcpd_dump_buf((char *)nlh, 128);

    netlink_unicast(nl_sk, new_skb, mcpd_pid, MSG_DONTWAIT);

#if 0
    mcpd_enqueue_packet(br, skb);
#endif

    return 1;
} /* mcpd_process_skb */
EXPORT_SYMBOL(mcpd_process_skb);

static void mcpd_nl_process_registration(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    char *ptr  = NULL;
    struct sk_buff *new_skb = NULL;
    char *new_ptr = NULL;
    struct nlmsghdr *new_nlh = NULL;
    int buf_size;

    mcpd_pid = nlh->nlmsg_pid;

    buf_size = NLMSG_SPACE((sizeof(t_MCPD_MSG_HDR) + sizeof(t_MCPD_REGISTER)));

    new_skb = alloc_skb(buf_size, GFP_ATOMIC);

    if(!new_skb) {
        printk("br_netlink_mcpd.c:%d %s() errr no mem\n", __LINE__, __FUNCTION__);
        return;
    }

    ptr = NLMSG_DATA(nlh);

    if(new_skb)
    {
        new_nlh = (struct nlmsghdr *)new_skb->data;
        new_ptr = NLMSG_DATA(new_nlh);
        new_nlh->nlmsg_len = buf_size;
        new_nlh->nlmsg_pid = 0;
        new_nlh->nlmsg_flags = 0;
        skb_put(new_skb, buf_size);
        ((t_MCPD_MSG_HDR *)new_ptr)->type = MCPD_MSG_REGISTER;
        ((t_MCPD_MSG_HDR *)new_ptr)->len = sizeof(t_MCPD_REGISTER);
        new_ptr += sizeof(t_MCPD_MSG_HDR);
        ((t_MCPD_REGISTER *)new_ptr)->code = MCPD_SUCCESS;

        NETLINK_CB(new_skb).dst_group = 0;
        NETLINK_CB(new_skb).pid = mcpd_pid;


        netlink_unicast(nl_sk, new_skb, mcpd_pid, MSG_DONTWAIT);
    }

    return;
} /* mcpd_nl_process_registration */

static void mcpd_nl_process_lan2lan_snooping(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    unsigned char *ptr;
    int val;

    ptr = NLMSG_DATA(nlh);
    ptr += sizeof(t_MCPD_MSG_HDR);

    val = (int)*(int *)ptr;

    br_igmp_lan2lan_snooping_update(val);

    return;
}

static void mcpd_nl_process_igmp_snoop_entry(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    struct net_device *dev = NULL;
    struct net_bridge *br = NULL;
    struct net_bridge_port *prt;
    t_MCPD_IGMP_SNOOP_ENTRY *snoop_entry;
    unsigned char *ptr;
    struct net_device *from_dev= NULL;
    int idx = 0;

    ptr = NLMSG_DATA(nlh);
    ptr += sizeof(t_MCPD_MSG_HDR);

    snoop_entry = (t_MCPD_IGMP_SNOOP_ENTRY *)ptr;

    dev = dev_get_by_name(&init_net, snoop_entry->br_name);
        
    if(!dev)
        return;

    br = netdev_priv(dev);

    if(!br)
    {
        dev_put(dev);
        return;
    }

    if(!(br->igmp_snooping))
        return;

    rcu_read_lock();
    prt = br_get_port(br, snoop_entry->port_no);
    rcu_read_unlock();

    for(idx = 0; idx < MCPD_MAX_IFS; idx++)
    {
        if(snoop_entry->wan_info[idx].if_ops)
        {
            from_dev = dev_get_by_name(&init_net, 
                                       snoop_entry->wan_info[idx].if_name);

            if((snoop_entry->mode == MCPD_SNOOP_IN_CLEAR) ||
                (snoop_entry->mode == MCPD_SNOOP_EX_CLEAR)) 
            {
                br_igmp_mc_fdb_remove(from_dev,
                                  snoop_entry->wan_info[idx].if_ops,
                                  br, 
                                  prt, 
                                  &snoop_entry->grp, 
                                  &snoop_entry->rep, 
                                  snoop_entry->mode, 
                                  &snoop_entry->src);
            }
            else
            {

                br_igmp_mc_fdb_add(from_dev,
                               snoop_entry->wan_info[idx].if_ops,
                               br, 
                               prt, 
                               &snoop_entry->grp, 
                               &snoop_entry->rep, 
                               snoop_entry->mode, 
                               snoop_entry->tci, 
                               &snoop_entry->src);
            }

            if(from_dev)
                dev_put(from_dev);
        }
        else
        {
            break;
        }
    }

    /* if LAN-2-LAN snooping enabled make an entry */
    if(br_igmp_get_lan2lan_snooping_info())
    {
        if((snoop_entry->mode == MCPD_SNOOP_IN_CLEAR) ||
            (snoop_entry->mode == MCPD_SNOOP_EX_CLEAR)) 
        {
            br_igmp_mc_fdb_remove(dev,
                                    snoop_entry->wan_info[idx].if_ops,
                                    br, 
                                    prt, 
                                    &snoop_entry->grp, 
                                    &snoop_entry->rep, 
                                    snoop_entry->mode, 
                                    &snoop_entry->src);
        }
        else
        {

            br_igmp_mc_fdb_add(dev,
                               snoop_entry->wan_info[idx].if_ops,
                               br, 
                               prt, 
                               &snoop_entry->grp, 
                               &snoop_entry->rep, 
                               snoop_entry->mode, 
                               snoop_entry->tci, 
                               &snoop_entry->src);
        }
    }

    if(dev)
        dev_put(dev);

    return;
} /* mcpd_nl_process_igmp_snoop_entry*/

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
static void mcpd_nl_process_mld_lan2lan_snooping(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    unsigned char *ptr;
    int val;

    ptr = NLMSG_DATA(nlh);
    ptr += sizeof(t_MCPD_MSG_HDR);

    val = (int)*(int *)ptr;

    br_mld_lan2lan_snooping_update(val);

    return;
}

static void mcpd_nl_process_mld_snoop_entry(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    struct net_device *dev = NULL;
    struct net_bridge *br = NULL;
    struct net_bridge_port *prt;
    t_MCPD_MLD_SNOOP_ENTRY *snoop_entry;
    unsigned char *ptr;
    struct net_device *from_dev= NULL;
    int idx = 0;

    ptr = NLMSG_DATA(nlh);
    ptr += sizeof(t_MCPD_MSG_HDR);

    snoop_entry = (t_MCPD_MLD_SNOOP_ENTRY *)ptr;

    dev = dev_get_by_name(&init_net, snoop_entry->br_name);

    if(!dev)
        return;

    br = netdev_priv(dev);

    if(!br)
    {
        dev_put(dev);
        return;
    }

    if(!(br->mld_snooping))
      return;

    rcu_read_lock();
    prt = br_get_port(br, snoop_entry->port_no);
    rcu_read_unlock();

    for(idx = 0; idx < MCPD_MAX_IFS; idx++)
    {
        if(snoop_entry->wan_info[idx].if_ops)
        {
            from_dev = dev_get_by_name(&init_net, 
                                       snoop_entry->wan_info[idx].if_name);

            if((snoop_entry->mode == MCPD_SNOOP_IN_CLEAR) ||
                (snoop_entry->mode == MCPD_SNOOP_EX_CLEAR)) 
            {
                br_mld_mc_fdb_remove(from_dev,
                                    snoop_entry->wan_info[idx].if_ops,
                                    br, 
                                    prt, 
                                    &snoop_entry->grp, 
                                    &snoop_entry->rep, 
                                    snoop_entry->mode, 
                                    &snoop_entry->src);
            }
            else
            {
                br_mld_mc_fdb_add(from_dev,
                                snoop_entry->wan_info[idx].if_ops,
                                br, 
                                prt, 
                                &snoop_entry->grp, 
                                &snoop_entry->rep, 
                                snoop_entry->mode, 
                                snoop_entry->tci, 
                                &snoop_entry->src);
            }

            if(from_dev)
                dev_put(from_dev);
        }
        else
        {
            break;
        }
    }

    /* if LAN-2-LAN snooping enabled make an entry */
    if(br_mld_get_lan2lan_snooping_info())
    {
        if((snoop_entry->mode == MCPD_SNOOP_IN_CLEAR) ||
            (snoop_entry->mode == MCPD_SNOOP_EX_CLEAR)) 
        {
            br_mld_mc_fdb_remove(dev,
                                 snoop_entry->wan_info[idx].if_ops,
                                 br, 
                                 prt, 
                                 &snoop_entry->grp, 
                                 &snoop_entry->rep, 
                                 snoop_entry->mode, 
                                 &snoop_entry->src);
        }
        else
        {
            br_mld_mc_fdb_add(dev,
                              snoop_entry->wan_info[idx].if_ops,
                              br, 
                              prt, 
                              &snoop_entry->grp, 
                              &snoop_entry->rep, 
                              snoop_entry->mode, 
                              snoop_entry->tci, 
                              &snoop_entry->src);
        }
    }

    if(dev)
        dev_put(dev);

    return;
} /* mcpd_nl_process_mld_snoop_entry*/
#endif

static void mcpd_nl_process_if_change(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    struct net_device *dev = NULL;
    struct net_bridge *br = NULL;
    unsigned char *ptr;
    char *if_name;

    ptr = NLMSG_DATA(nlh);
    ptr += sizeof(t_MCPD_MSG_HDR);

    if_name = ptr;

    dev = dev_get_by_name(&init_net, if_name);
        
    if(!dev)
        return;

    br = netdev_priv(dev);

    if(!br)
    {
        dev_put(dev);
        return;
    }

    br_igmp_process_if_change(br);

    if(dev)
        dev_put(dev);

    return;
} /* mcpd_nl_process_igmp_snoop_entry*/

static void mcpd_nl_process_mc_fdb_cleanup(void)
{
    struct net_device *dev = NULL;
    struct net_bridge *br = NULL;

    rtnl_lock();
    ASSERT_RTNL();
    for(dev = first_net_device(&init_net); 
        dev; 
        dev = next_net_device(dev)) 
    {
        br = netdev_priv(dev);
        if((dev->priv_flags & IFF_EBRIDGE) && (br))
        {
            if(br->igmp_snooping)
                br_igmp_mc_fdb_cleanup(br);

#if (defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP))
            if(br->mld_snooping)
                br_mld_mc_fdb_cleanup(br);
#endif
        }
    }
    rtnl_unlock();
    return;
}

static int mcpd_nl_process_verdict(t_MCPD_VERDICT *verdict)
{
#if 0
    struct mcpd_queue_entry *entry = NULL;
    struct list_head *p;

    list_for_each_prev(p, &mcpd_queue_list) 
    {
        if((__u32)(((struct mcpd_queue_entry *)p)->skb) == verdict->corr_tag)
        {
            entry = (struct mcpd_queue_entry *)p;
            break;
        }
    }

    if(entry)
    {
        write_lock_bh(&mcpd_queue_lock);
        list_del(&entry->list);
        write_unlock_bh(&mcpd_queue_lock);

        if(verdict->code != MCPD_RET_DROP)
        {
            br_flood_forward(entry->br, entry->skb);
        }
        else
        {
            kfree_skb(entry->skb);
        }
        kfree(entry);

        mcpd_queue_total--;
    }
#endif

    return 0;
} /* mcpd_nl_process_verdict */

void mcpd_nl_send_igmp_purge_entry(struct net_bridge_mc_fdb_entry *igmp_entry)
{
    t_MCPD_IGMP_PURGE_ENTRY *purge_entry;
    int buf_size = 0;
    struct sk_buff *new_skb;
    struct nlmsghdr *nlh;
    char *ptr = NULL;
    struct net_bridge_mc_rep_entry *rep = NULL;

    if(!igmp_entry)
        return;

    rep = list_first_entry(&igmp_entry->rep_list, struct net_bridge_mc_rep_entry, list);

    if(!rep)
        return;

    if(!mcpd_pid)
        return;

    buf_size = sizeof(t_MCPD_IGMP_PURGE_ENTRY) + sizeof(t_MCPD_MSG_HDR);
    new_skb = alloc_skb(NLMSG_SPACE(buf_size), GFP_ATOMIC);

    if(!new_skb) 
    {
        return;
    }

    nlh = (struct nlmsghdr *)new_skb->data;
    ptr = NLMSG_DATA(nlh);
    nlh->nlmsg_len = NLMSG_SPACE(buf_size);
    nlh->nlmsg_pid = 0;
    nlh->nlmsg_flags = 0;
    skb_put(new_skb, NLMSG_SPACE(buf_size));
    ((t_MCPD_MSG_HDR *)ptr)->type = MCPD_MSG_IGMP_PURGE_ENTRY;
    ((t_MCPD_MSG_HDR *)ptr)->len = sizeof(t_MCPD_IGMP_PURGE_ENTRY);
    ptr += sizeof(t_MCPD_MSG_HDR);

    purge_entry = (t_MCPD_IGMP_PURGE_ENTRY *)ptr;

    purge_entry->grp.s_addr = igmp_entry->grp.s_addr;
    purge_entry->src.s_addr = igmp_entry->src_entry.src.s_addr;
    purge_entry->rep.s_addr = rep->rep.s_addr;

    purge_entry->pkt_info.br_name[0] = '\0';
    memcpy(purge_entry->pkt_info.port_name, igmp_entry->dst->dev->name, IFNAMSIZ);
    purge_entry->pkt_info.port_no = igmp_entry->dst->port_no;
    purge_entry->pkt_info.if_index = 0;
    purge_entry->pkt_info.data_len = 0;
    purge_entry->pkt_info.corr_tag = 0;

#if defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE)
    purge_entry->pkt_info.tci = igmp_entry->lan_tci;
#endif /*  defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE) */

    NETLINK_CB(new_skb).dst_group = 0;
    NETLINK_CB(new_skb).pid = mcpd_pid;
    mcpd_dump_buf((char *)nlh, 128);

    netlink_unicast(nl_sk, new_skb, mcpd_pid, MSG_DONTWAIT);

    return;
} /* mcpd_nl_send_purge_entry */

static inline void mcpd_nl_rcv_skb(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    char *ptr  = NULL;
    unsigned short msg_type;

    if (skb->len >= NLMSG_SPACE(0)) 
    {
        if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
            return;

        ptr = NLMSG_DATA(nlh);

        msg_type = *(unsigned short *)ptr;
        switch(msg_type)
        {
            case MCPD_MSG_REGISTER:
                mcpd_nl_process_registration(skb);
                break;

            case MCPD_MSG_IGMP_SNOOP_ENTRY:
                mcpd_nl_process_igmp_snoop_entry(skb);
                break;
                
            case MCPD_MSG_IGMP_LAN2LAN_SNOOP:
                mcpd_nl_process_lan2lan_snooping(skb);
                break;

            case MCPD_MSG_VERDICT:
                mcpd_nl_process_verdict((t_MCPD_VERDICT *)
                                            (ptr + sizeof(t_MCPD_MSG_HDR)));
                break;

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)
            case MCPD_MSG_MLD_SNOOP_ENTRY:
                mcpd_nl_process_mld_snoop_entry(skb);
                break;

            case MCPD_MSG_MLD_LAN2LAN_SNOOP:
                mcpd_nl_process_mld_lan2lan_snooping(skb);
                break;
#endif

            case MCPD_MSG_IF_CHANGE:
                mcpd_nl_process_if_change(skb);
                break;

            case MCPD_MSG_MC_FDB_CLEANUP:
                mcpd_nl_process_mc_fdb_cleanup();
                break;

            default:
                printk("MCPD Unknown usr->krnl msg type \n");
        }
    }

    return;
} /* mcpd_nl_rcv_skb */

#if 0
static void mcpd_nl_data_ready(struct sock *sk, int len)
{
    struct sk_buff *skb = NULL;
    unsigned int qlen = skb_queue_len(&sk->sk_receive_queue);

    while (qlen-- && (skb = skb_dequeue(&sk->sk_receive_queue))) 
    {
        mcpd_nl_rcv_skb(skb);
        kfree_skb(skb);
    }
} /* mcpd_nl_data_ready */
#endif

static int __init mcpd_module_init(void)
{
    printk(KERN_INFO "Initializing MCPD Module\n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_MCPD, 0, 
                                mcpd_nl_rcv_skb, NULL, THIS_MODULE);

    if(nl_sk == NULL) 
    {
        printk("MCPD: failure to create kernel netlink socket\n");
        return -ENOMEM;
    }

    return 0;
} /* mcpd_module_init */

static void __exit mcpd_module_exit(void)
{
    sock_release(nl_sk->sk_socket); 
    printk(KERN_INFO "Removed MCPD\n");
} /* mcpd_module_exit */

module_init(mcpd_module_init);
module_exit(mcpd_module_exit);
