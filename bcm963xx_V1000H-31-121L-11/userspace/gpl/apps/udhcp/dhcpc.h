/* dhcpd.h */
#ifndef _DHCPC_H
#define _DHCPC_H

#define INIT_SELECTING	0
#define REQUESTING	1
#define BOUND		2
#define RENEWING	3
#define REBINDING	4
#define INIT_REBOOT	5
#define RENEW_REQUESTED 6
#define RELEASED	7
#ifdef AEI_SUPPORT_6RD
#define DHCP_6RD	212
#endif
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
#define MAC_INVALID_ADDR    "\x00\x00\x00\x00\x00\x00"
#endif

/* Paramaters the client should request from the server */

#ifdef AEI_SUPPORT_6RD
#define PARM_REQUESTS \
	DHCP_SUBNET, \
	DHCP_ROUTER, \
	DHCP_DNS_SERVER, \
	DHCP_HOST_NAME, \
	DHCP_DOMAIN_NAME, \
	DHCP_BROADCAST, \
	DHCP_6RD
	
#else
#define PARM_REQUESTS \
	DHCP_SUBNET, \
	DHCP_ROUTER, \
	DHCP_DNS_SERVER, \
	DHCP_HOST_NAME, \
	DHCP_DOMAIN_NAME, \
	DHCP_BROADCAST
#endif

struct client_config_t {
    char foreground;            /* Do not fork */
    char quit_after_lease;      /* Quit after obtaining lease */
    char abort_if_no_lease;     /* Abort if no lease */
    char *interface;            /* The name of the interface to use */
    char *pidfile;              /* Optionally store the process ID */
    char *script;               /* User script to run at dhcp events */
    char *clientid;             /* Optional client id to use */
    char *hostname;             /* Optional hostname to use */
    int ifindex;                /* Index number of the interface to use */
    unsigned char arp[6];       /* Our arp address */
};

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_OPTION121) //add william 2011-12-7
typedef struct route_info{
	struct route_info_t *next;
	char desIP[16];
	char netmask[16];
	char gateway[16];
}route_info_t;
#endif


extern struct client_config_t client_config;

// brcm
extern char vendor_class_id[];
extern char en_vendor_class_id;
extern char en_client_id;
extern char duid[];
extern char iaid[];
extern char en_125;
extern char oui_125[];
extern char sn_125[];
extern char prod_125[];
#endif
