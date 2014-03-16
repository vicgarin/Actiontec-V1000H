#ifndef _BR_MLD_H
#define _BR_MLD_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/igmp.h>
#include <linux/in6.h>
#include "br_private.h"
#include <linux/blog.h>
#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BLOG)
#include "br_mcast.h"
#endif

#if defined(CONFIG_MIPS_BRCM) && defined(CONFIG_BR_MLD_SNOOP)

#define SNOOPING_BLOCKING_MODE 2

#define TIMER_CHECK_TIMEOUT 10
#define BR_MLD_MEMBERSHIP_TIMEOUT 260 /* RFC3810 */

#define BR_MLD_MULTICAST_MAC_PREFIX 0x33

#define BCM_IN6_ARE_ADDR_EQUAL(a,b)                                       \
       ((((__const uint32_t *) (a))[0] == ((__const uint32_t *) (b))[0])  \
	 && (((__const uint32_t *) (a))[1] == ((__const uint32_t *) (b))[1])  \
	 && (((__const uint32_t *) (a))[2] == ((__const uint32_t *) (b))[2])  \
	 && (((__const uint32_t *) (a))[3] == ((__const uint32_t *) (b))[3])) 

#define BCM_IN6_ASSIGN_ADDR(a,b)                                  \
    do {                                                          \
        ((uint32_t *) (a))[0] = ((__const uint32_t *) (b))[0];    \
        ((uint32_t *) (a))[1] = ((__const uint32_t *) (b))[1];    \
        ((uint32_t *) (a))[2] = ((__const uint32_t *) (b))[2];    \
        ((uint32_t *) (a))[3] = ((__const uint32_t *) (b))[3];    \
    } while(0)

#define BCM_IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)
#define BCM_IN6_MULTICAST(x)   (BCM_IN6_IS_ADDR_MULTICAST(x))
#define BCM_IN6_IS_ADDR_MC_NODELOCAL(a) \
	(BCM_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x1))

#define BCM_IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(BCM_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x2))

#define BCM_IN6_IS_ADDR_MC_SITELOCAL(a) \
	(BCM_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x5))

#define BCM_IN6_IS_ADDR_MC_ORGLOCAL(a) \
	(BCM_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x8))

#define BCM_IN6_IS_ADDR_MC_GLOBAL(a) \
	(BCM_IN6_IS_ADDR_MULTICAST(a) \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0xe))

#define BCM_IN6_IS_ADDR_MC_SCOPE0(a) \
	(BCM_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x0))

struct mld2_grec {
	__u8		grec_type;
	__u8		grec_auxwords;
	__be16		grec_nsrcs;
	struct in6_addr	grec_mca;
	struct in6_addr	grec_src[0];
};

struct mld2_report {
	__u8	type;
	__u8	resv1;
	__sum16	csum;
	__be16	resv2;
	__be16	ngrec;
	struct mld2_grec grec[0];
};

#define MLDV2_GRP_REC_SIZE(x)  (sizeof(struct mld2_grec) + \
                       (sizeof(struct in6_addr) * ((struct mld2_grec *)x)->grec_nsrcs))

struct net_br_mld_mc_src_entry
{
	struct in6_addr		src;
	unsigned long		tstamp;
    int                 filt_mode;
};

struct net_br_mld_mc_rep_entry
{
	struct in6_addr     rep;
	struct list_head    list;
};

struct net_br_mld_mc_fdb_entry
{
	struct net_bridge_port *dst;
	struct in6_addr         grp;
	struct in6_addr         rep;
	struct net_br_mld_mc_src_entry src_entry;
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
#endif
	struct net_device      *from_dev;
	struct list_head 		list;
};

int br_mld_blog_rule_update(struct net_br_mld_mc_fdb_entry *mc_fdb, int wan_ops);

extern int br_mld_mc_forward(struct net_bridge *br, 
                             struct sk_buff *skb, 
                             int forward,
                             int is_routed);

extern int br_mld_mc_fdb_add(struct net_device *from_dev,
                        int wan_ops,
                        struct net_bridge *br, 
                        struct net_bridge_port *prt, 
                        struct in6_addr *grp, 
                        struct in6_addr *rep, 
                        int mode, 
                        int tci, 
                        struct in6_addr *src);

extern void br_mld_mc_fdb_remove_grp(struct net_bridge *br, 
                                     struct net_bridge_port *prt, 
                                     struct in6_addr *grp);

extern void br_mld_mc_fdb_cleanup(struct net_bridge *br);

int br_mld_mc_fdb_remove(struct net_device *from_dev,
                        int wan_ops,
                        struct net_bridge *br, 
                        struct net_bridge_port *prt, 
                        struct in6_addr *grp, 
                        struct in6_addr *rep, 
                        int mode, 
                        struct in6_addr *src);

void br_mld_snooping_init(void);

extern int br_mld_set_port_snooping(struct net_bridge_port *p,  void __user * userbuf);

extern int br_mld_clear_port_snooping(struct net_bridge_port *p,  void __user * userbuf);

struct net_br_mld_mc_fdb_entry *br_mld_mc_fdb_copy(struct net_bridge *br, 
                                     const struct net_br_mld_mc_fdb_entry *mld_fdb);
void br_mld_mc_fdb_del_entry(struct net_bridge *br, 
                              struct net_br_mld_mc_fdb_entry *mld_fdb);

void br_mld_lan2lan_snooping_update(int val);
int br_mld_get_lan2lan_snooping_info(void);

#endif
#endif /* _BR_MLD_H */
