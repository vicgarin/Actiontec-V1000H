#ifndef NETLINK_DAD_H_
#define NETLINK_DAD_H_

struct dad_failed_msg_t {
    struct in6_addr addr;
    int prefix_len;
    char name[32];
};

int netlink_dad_start( int *fd );
int netlink_recv( int fd, struct dhcp6_if ** pp_if );

#endif  //NETLINK_DAD_H_

