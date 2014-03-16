/*
 * arpping.c
 *
 * Mostly stolen from: dhcpcd - DHCP client daemon
 * by Yoichi Hariguchi <yoichi@fore.com>
 */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "dhcpd.h"
#include "debug.h"
#include "arpping.h"

/* local prototypes */
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)                
int arpCheck(u_long inaddr, struct ifinfo *ifbuf, long timeout,int isCopyMac, char*server_addr);
#else
int arpCheck(u_long inaddr, struct ifinfo *ifbuf, long timeout);
#endif
void mkArpMsg(int opcode, u_long tInaddr, u_char *tHaddr, u_long sInaddr, u_char *sHaddr, struct arpMsg *msg);
int openRawSocket (int *s, u_short type);


/* args:	yiaddr - what IP to ping (eg. on the NETtel cb189701)
 * retn: 	1 addr free
 *		0 addr used
 *		-1 error 
 */  
int arpping(u_int32_t yiaddr, u_int32_t ip, char *interface) {
	struct ifinfo ifbuf;
	int fd, ret;
	struct ifreq ifr;
	unsigned char sMac[6];

	ret = 1; // default action is to assign the address

	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		LOG(LOG_ERR, "socket failed!");
		return ret;
	}

	ifr.ifr_addr.sa_family = AF_INET;
#if defined(AEI_COVERITY_FIX)
        /*Coverity Fix CID 12253 copy into fixed size buffer*/
        strlcpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
#else
	strcpy(ifr.ifr_name, interface);
#endif

	/* Retrieve MAC of the interface */
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
		memcpy(sMac, ifr.ifr_hwaddr.sa_data, 6);
		close(fd);
	}
	else
	{
		close(fd);
		return ret;
	}

#if defined(AEI_COVERITY_FIX)
        /*Coverity Fix CID 12253:Copy into fixed size buffer*/
        strlcpy(ifbuf.ifname, interface, sizeof(ifbuf.ifname));
#else
	strcpy(ifbuf.ifname, interface);
#endif
	ifbuf.addr = ip;
	ifbuf.mask = 0x0;
	ifbuf.bcast = 0x0;
	
	memcpy(ifbuf.haddr, sMac, 6);
	ifbuf.flags = 0;
	
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)                
    return arpCheck(yiaddr, &ifbuf, 2,0,NULL);
#else
    return arpCheck(yiaddr, &ifbuf, 2);
#endif	
}
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)                
/* ken, get the Server MAC address*/
int arppingForDHCPc(u_int32_t yiaddr, u_int32_t ip, char *arp, char *interface,int isCopyMac, char*server_addr)
{
    struct ifinfo ifbuf;

    strcpy(ifbuf.ifname, interface);
    ifbuf.addr = ip;
    ifbuf.mask = 0x0;
    ifbuf.bcast = 0x0;

    memcpy(ifbuf.haddr, arp, 6);
    ifbuf.flags = 0;

    return arpCheck(yiaddr, &ifbuf, 2,isCopyMac,server_addr);
}
#endif

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)                
int arpCheck(u_long inaddr, struct ifinfo *ifbuf, long timeout,int isCopyMac, char*server_addr)
#else
int arpCheck(u_long inaddr, struct ifinfo *ifbuf, long timeout)
#endif
{
	int				s;			/* socket */
	int				rv;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;
	fd_set			fdset;
	struct timeval	tm;
	time_t			prevTime;

	rv = 1;
	openRawSocket(&s, ETH_P_ARP);

	/* send arp request */
	mkArpMsg(ARPOP_REQUEST, inaddr, NULL, ifbuf->addr, ifbuf->haddr, &arp);
	bzero(&addr, sizeof(addr));
#if defined(AEI_COVERITY_FIX)
        /*Coverity Fix CID 12252 Destination buffer too small*/
        strlcpy(addr.sa_data, ifbuf->ifname,sizeof(addr.sa_data));
#else
        strcpy(addr.sa_data, ifbuf->ifname);
#endif
	if ( sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0 ) rv = 0;
	
	/* wait arp reply, and check it */
	tm.tv_usec = 0;
	time(&prevTime);
	while ( timeout > 0 ) {
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		tm.tv_sec  = timeout;
		if ( select(s+1, &fdset, (fd_set *)NULL, (fd_set *)NULL, &tm) < 0 ) {
			DEBUG(LOG_ERR, "Error on ARPING request: errno=%d", errno);
			if (errno != EINTR) rv = 0;
		} else if ( FD_ISSET(s, &fdset) ) {
			if (recv(s, &arp, sizeof(arp), 0) < 0 ) rv = 0;
			if(arp.operation == htons(ARPOP_REPLY) && 
			   bcmp(arp.tHaddr, ifbuf->haddr, 6) == 0 && 
			   *((u_int *)arp.sInaddr) == inaddr ) {
				DEBUG(LOG_INFO, "Valid arp reply receved for this address");

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)                
                if(isCopyMac && server_addr !=NULL)
                {
                    memcpy(server_addr,arp.sHaddr,6);
                    LOG(LOG_ERR, "arp sHaddr %02x:%02x:%02x:%02x:%02x:%02x",
                    server_addr[0], server_addr[1],server_addr[2],
                    server_addr[3], server_addr[4], server_addr[5]);
                }
#endif                
				rv = 0;
				break;
			}
		}
		timeout -= time(NULL) - prevTime;
		time(&prevTime);
	}
	close(s);
	DEBUG(LOG_INFO, "%salid arp replies for this address", rv ? "No v" : "V");	 
	return rv;
}

void mkArpMsg(int opcode, u_long tInaddr, u_char *tHaddr,
		 u_long sInaddr, u_char *sHaddr, struct arpMsg *msg) {
	bzero(msg, sizeof(*msg));
	bcopy(MAC_BCAST_ADDR, msg->ethhdr.h_dest, 6); /* MAC DA */
	bcopy(sHaddr, msg->ethhdr.h_source, 6);	/* MAC SA */
	msg->ethhdr.h_proto = htons(ETH_P_ARP);	/* protocol type (Ethernet) */
	msg->htype = htons(ARPHRD_ETHER);		/* hardware type */
	msg->ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	msg->hlen = 6;							/* hardware address length */
	msg->plen = 4;							/* protocol address length */
	msg->operation = htons(opcode);			/* ARP op code */
//brcm start
	bcopy((u_char *)&sInaddr, &msg->sInaddr[0], 4);	/* source IP address */
	bcopy(sHaddr, msg->sHaddr, 6);			/* source hardware address */
   bcopy((u_char *)&tInaddr, &msg->tInaddr[0], 4);	/* target IP address */
//brcm end
	if ( opcode == ARPOP_REPLY )
		bcopy(tHaddr, msg->tHaddr, 6);		/* target hardware address */
}


int openRawSocket (int *s, u_short type) {
	int optval = 1;

	if((*s = socket (PF_PACKET, SOCK_PACKET, htons (type))) == -1) {
		LOG(LOG_ERR, "Could not open raw socket");
		return -1;
	}
	
	if(setsockopt (*s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval)) == -1) {
		LOG(LOG_ERR, "Could not setsocketopt on raw socket");
		return -1;
	}
	return 0;
}

