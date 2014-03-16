#ifndef _BR_FDB_H
#define _BR_FDB_H

#if defined(CONFIG_MIPS_BRCM)
extern int br_fdb_adddel_static(struct net_bridge *br, 
                                struct net_bridge_port *source,
                                const unsigned char *addr, int bInsert);
#endif /* CONFIG_MIPS_BRCM */

#endif /* _BR_FDB_H */

