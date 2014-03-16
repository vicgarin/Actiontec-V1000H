/*
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "dhcp6.h"
#include "config.h"
#include "common.h"
#include "timer.h"
#include "dhcp6c_ia.h"
#include "prefixconf.h"

#include "dhcp6.h"
#include "config.h"
#include "common.h"
***/
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/ioctl.h>

#include <net/if.h>
#ifdef __FreeBSD__
jdfjksdfj#include <net/if_var.h>
#endif

#include <netinet/in.h>

#ifdef __KAME__
dfsdfsd#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#endif

#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

//#include <util/mgt_client.h>

//#include "ctl_msg.h"
//#include "dbussend_msg.h"
//extern CtlDhcp6cStateChangedMsgBody ctldhcp6cMsgBody;

#include "dhcp6.h"
#include "config.h"
#include "common.h"
#include "timer.h"
#include "dhcp6c_ia.h"
#include "prefixconf.h"

#include "netlink_dad.h"

static void
nl_close(int sd)
{
    if (sd >= 0) close(sd);
}

static int
nl_open(void)
{
    struct sockaddr_nl nladdr;
    int sd;

    sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_DAD);
    if (sd < 0) return -1;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    nladdr.nl_pid = getpid();  /* self pid */
    /* interested in group 1<<0 */
    nladdr.nl_groups = 1;

    if (bind(sd, (struct sockaddr*)&nladdr, sizeof(nladdr)) < 0) {
        nl_close(sd);
        return -1;
    }
    return sd;
}

int
netlink_dad_start( int *fd )
{
    int status;
    //int fd;
    struct msghdr msg;
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct dad_failed_msg_t *dad_msg;

    *fd = nl_open();
    if ( *fd < 0) {
        dprintf(LOG_INFO, FNAME, "nl_open failed\n");
        perror("Error\n");
        return 1;
    }

    return 0;
}

    int
netlink_recv( int fd,
        struct dhcp6_if **pp_if)
{
    int status;
    struct msghdr msg = {0};
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct dad_failed_msg_t *dad_msg = NULL;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct dad_failed_msg_t)));
    if (!nlh) {
        dprintf(LOG_INFO, FNAME, "malloc failed\n");
        return 2;
    }
    memset(nlh, 0, NLMSG_SPACE(sizeof(struct dad_failed_msg_t)));

    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(sizeof(struct dad_failed_msg_t));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    dprintf(LOG_INFO, FNAME, "Starting recv,iov length is %d",(int)iov.iov_len);
    
    status = recvmsg(fd, &msg, 0);
    if (status < 0) {
        // close(fd);
        free(nlh);
        dprintf(LOG_INFO, FNAME, "recvmsg failed, error: %s", strerror(errno) );
        return 1;
    }

    if (iov.iov_len == nlh->nlmsg_len) {
        dad_msg = (struct dad_failed_msg_t *)NLMSG_DATA(nlh);

#define ADDR_PREFIX_FMT "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X/%d"
#define ADDR_PREFIX_PARAM(dad_msg) dad_msg->addr.s6_addr16[0],\
        dad_msg->addr.s6_addr16[1],\
        dad_msg->addr.s6_addr16[2],\
        dad_msg->addr.s6_addr16[3],\
        dad_msg->addr.s6_addr16[4],\
        dad_msg->addr.s6_addr16[5],\
        dad_msg->addr.s6_addr16[6],\
        dad_msg->addr.s6_addr16[7],\
        dad_msg->prefix_len

        dprintf(LOG_INFO, FNAME, "DAD failed on interface %s,Address:" ADDR_PREFIX_FMT, dad_msg->name, ADDR_PREFIX_PARAM(dad_msg));
    }

    free(nlh);
    // close(fd);
    

    if(dad_msg)
    {
        struct dhcp6_if *ifp = NULL;
        if ((ifp = find_ifconfbyname(dad_msg->name)) == NULL) {
            dprintf(LOG_INFO, FNAME,
                    "failed to find interface configuration for %s",
                    dad_msg->name);
            return 11;
        }
    
        // returned value
        memcpy( &(ifp->addr.s6_addr32), &(dad_msg->addr.s6_addr32), sizeof( struct in6_addr ) );
        *pp_if = ifp;
    
        dprintf(LOG_INFO, FNAME, "Then we should send DH6_DECLINE to: %s", ifp->ifname );
    }
    
    return 0;
}

#if 0
int
main(int argc,char **argv)
{
    int status;
    int fd;
    struct msghdr msg;
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct dad_failed_msg_t *dad_msg;

    fd = nl_open();
    if (fd < 0) {
        perror("Error\n");
        return -1;
    }

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct dad_failed_msg_t)));
    if (!nlh) {
        perror("Malloc Error\n");
        return -1;
    }
    memset(nlh, 0, NLMSG_SPACE(sizeof(struct dad_failed_msg_t)));

    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(sizeof(struct dad_failed_msg_t));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Starting recv,iov length is %d\n",(int)iov.iov_len);

    while (1) {
        status = recvmsg(fd, &msg, 0);

        if (status < 0) {
            close(fd);
            free(nlh);
            perror("recv");
            return 1;
        }

        if (iov.iov_len == nlh->nlmsg_len) {
            dad_msg = (struct dad_failed_msg_t *)NLMSG_DATA(nlh);

#define ADDR_PREFIX_FMT "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X/%d"
#define ADDR_PREFIX_PARAM(dad_msg) dad_msg->addr.s6_addr16[0],\
            dad_msg->addr.s6_addr16[1],\
            dad_msg->addr.s6_addr16[2],\
            dad_msg->addr.s6_addr16[3],\
            dad_msg->addr.s6_addr16[4],\
            dad_msg->addr.s6_addr16[5],\
            dad_msg->addr.s6_addr16[6],\
            dad_msg->addr.s6_addr16[7],\
            dad_msg->prefix_len

            printf("DAD failed on interface %s,Address:" ADDR_PREFIX_FMT "\n",
                   dad_msg->name,ADDR_PREFIX_PARAM(dad_msg));
        }
    }

    close(fd);
    free(nlh);

    return 0;
}
#endif

