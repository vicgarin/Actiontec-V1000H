#ifndef _BR_MCAST_H
#define _BR_MCAST_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/igmp.h>
#include <linux/in.h>
#include "br_private.h"
#include <linux/if_vlan.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>

#define MCPD_IF_TYPE_UNKWN      0
#define MCPD_IF_TYPE_BRIDGED    1
#define MCPD_IF_TYPE_ROUTED     2

typedef enum br_mcast_proto_type {
    BR_MCAST_PROTO_NONE,
    BR_MCAST_PROTO_IGMP,
    BR_MCAST_PROTO_MLD
} t_BR_MCAST_PROTO_TYPE;

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
void br_mcast_blog_release(t_BR_MCAST_PROTO_TYPE proto, void *mc_fdb);
void br_mcast_vlan_notify_for_blog_update(struct net_device *ndev,
                                   blogRuleVlanNotifyDirection_t direction,
                                   uint32_t nbrOfTags);
void br_mcast_handle_netdevice_events(struct net_device *ndev, unsigned long event);
int br_mcast_blog_process(struct net_bridge *br,
                            void *mc_fdb,
                            t_BR_MCAST_PROTO_TYPE proto,
                            int wan_ops);
#endif
#endif
