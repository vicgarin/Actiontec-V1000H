#ifndef _BR_IGMP_H
#define _BR_IGMP_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/igmp.h>
#include <linux/in.h>
#include "br_private.h"
#include <linux/blog.h>
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#include "br_mcast.h"
#endif

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_IGMP_SNOOP)
#define SNOOPING_BLOCKING_MODE 2

union ip_array {
	unsigned int ip_addr;
    unsigned char ip_ar[4];
};

#define TIMER_CHECK_TIMEOUT 10
#define BR_IGMP_MEMBERSHIP_TIMEOUT 260 /* RFC3376 */

#define IGMPV3_GRP_REC_SIZE(x)  (sizeof(struct igmpv3_grec) + \
                       (sizeof(struct in_addr) * ((struct igmpv3_grec *)x)->grec_nsrcs))


struct net_bridge_mc_src_entry
{
    struct in_addr		src;
    unsigned long		tstamp;
    int                 filt_mode;
};

struct net_bridge_mc_rep_entry
{
	struct in_addr      rep;
	struct list_head    list;
};

struct net_bridge_mc_fdb_entry
{
	struct net_bridge_port *dst;
	struct in_addr          grp;
	struct list_head        rep_list;
	struct net_bridge_mc_src_entry  src_entry;
	uint32_t                lan_tci; /* vlan id */
	uint32_t                wan_tci; /* vlan id */
    int                     num_tags;
	unsigned char			is_local;
	unsigned char			is_static;
	unsigned long			tstamp;
    char                    wan_name[IFNAMSIZ];
    char                    lan_name[IFNAMSIZ];
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
	uint32_t                blog_idx;
#if defined(AEI_VDSL_MC_SSM_HIT)
        uint32_t                saddr;
#endif
#endif
	struct net_device      *from_dev;
	struct list_head 		list;
};

int br_igmp_control_filter(const unsigned char *dest, __be32 dest_ip);

extern void mcpd_nl_send_igmp_purge_entry(struct net_bridge_mc_fdb_entry *igmp_entry);

int br_igmp_blog_rule_update(struct net_bridge_mc_fdb_entry *mc_fdb, int wan_ops);

extern int br_igmp_mc_forward(struct net_bridge *br, 
                              struct sk_buff *skb, 
                              int forward,
                              int is_routed);
void br_igmp_delbr_cleanup(struct net_bridge *br);

int br_igmp_mc_fdb_add(struct net_device *from_dev,
                       int wan_ops,
                       struct net_bridge *br, 
                       struct net_bridge_port *prt, 
                       struct in_addr *grp, 
                       struct in_addr *rep, 
                       int mode, 
                       int tci, 
                       struct in_addr *src);

extern void br_igmp_mc_fdb_remove_grp(struct net_bridge *br, 
                                      struct net_bridge_port *prt, 
                                      struct in_addr *grp);
extern void br_igmp_mc_fdb_cleanup(struct net_bridge *br);
extern int br_igmp_mc_fdb_remove(struct net_device *from_dev,
                                 int wan_ops,
                                 struct net_bridge *br, 
                                 struct net_bridge_port *prt, 
                                 struct in_addr *grp, 
                                 struct in_addr *rep, 
                                 int mode, 
                                 struct in_addr *src);
void br_igmp_snooping_init(void);
void br_igmp_set_snooping(int val);
void br_igmp_handle_netdevice_events(struct net_device *ndev, unsigned long event);
void br_igmp_lan2lan_snooping_update(int val);
int br_igmp_get_lan2lan_snooping_info(void);
int br_igmp_process_if_change(struct net_bridge *br);
struct net_bridge_mc_fdb_entry *br_igmp_mc_fdb_copy(struct net_bridge *br, 
                                     const struct net_bridge_mc_fdb_entry *igmp_fdb);
void br_igmp_mc_fdb_del_entry(struct net_bridge *br, 
                              struct net_bridge_mc_fdb_entry *igmp_fdb);
#endif
#endif /* _BR_IGMP_H */
