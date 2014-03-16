/* dhcpd.c
 *
 * udhcp DHCP client
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "packet.h"
#include "script.h"
#include "socket.h"
#include "debug.h"
#include "pidfile.h"

// brcm
#include "cms_msg.h"

#ifdef AEI_CONTROL_LAYER
#include "ctl_msg.h"
//#include "ctl_nid.h"
#include "dbussend_msg.h"
#include "dbus_define.h"
#endif
#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)
#define ALARM_1MIN 60
#define MAX_ARP_FAIL_COUNT 15
int needArpDetectContinue = 0;
int arpDetectFailCount = 0;
extern char router_ip[32];
extern char local_ip[32];
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
#define MAXNONVALIDIPCOUNT 3
#define NTP_SERVER_FILE "/var/ntp-server"
#endif
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
char server_hwaddr[6] = MAC_INVALID_ADDR;   /* Server MAC address to use for raw mode */
#endif

/*
 * Since dhcpc and dhcpd are actually the same binary, the msgHandle
 * is declared in dhcpd.c and used by dhcpc.
 */

static int state;
static unsigned long requested_ip; /* = 0 */
static unsigned long server_addr;
static unsigned long timeout;
static int packet_num; /* = 0 */

// brcm
char session_path[64];
static char status_path[128]="";
static char pid_path[128]="";
char en_vendor_class_id=0;
char vendor_class_id[256]="";
char iaid[10]="";
char duid[256]="";
char en_client_id=0;
char en_125=0;
char oui_125[10]="";
char sn_125[64]="";
char prod_125[64]="";
int ipv6rd_opt = 0;


#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2
static int listen_mode = LISTEN_RAW;
// brcm
static int old_mode = LISTEN_RAW;
#define INIT_TIMEOUT 5
#define REQ_TIMEOUT 4

#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
#define BACKOFF_THRESHOLD 6
#endif

#define DEFAULT_SCRIPT	"/etc/dhcp/dhcp_getdata"

struct client_config_t client_config = {
	/* Default options. */
	abort_if_no_lease: 0,
	foreground: 0,
	quit_after_lease: 0,
	interface: "eth0",
	pidfile: NULL,
	script: DEFAULT_SCRIPT,
	clientid: NULL,
	hostname: NULL,
	ifindex: 0,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
};

#if defined(AEI_SUPPORT_6RD) && defined(AEI_CONTROL_LAYER)

static tsl_bool ctlMsg_send(CtlMsgHeader * msg)
{
    tsl_bool result = TSL_B_FALSE;
    dbussend_hdl_st *sendhdl = NULL;

    // init msg bus send handle
    //tsl_char_t buffer[BUFF256] = {0};
    //dbussend_msg_st *p_dbussend_msg = (dbussend_msg_st *)buffer;
    do {
        sendhdl = dbussend_init();
        if (NULL == sendhdl) {
            printf("dbussend_init() return fail\n");
            break;
        }
        //p_dbussend_msg->data_length = sizeof(buffer) - sizeof(dbussend_msg_st);
        //memcpy( p_dbussend_msg->buffer,
        //              msg, sizeof(msg));

        if (0 == dbussend_sendmsg(sendhdl, CTL_MSG_TYPE(CTL_MSG_DHCPC_STATE_CHANGED),   // msg
                                  NULL, // method
                                  msg,  // p_dbussend_msg,
                                  sizeof(CtlDhcpcStateChangedMsgBody))) {
            //printf("Successfully to send msg!\n");
            result = TSL_B_TRUE;
        } else {
            printf("Failed to send msg!\n");
        }
    } while (0);

    // uninit msg bus
    if (NULL != sendhdl) {
        dbussend_uninit(sendhdl);
    }
    return result;
}

void sendEventMessageWith6RD(UBOOL8 assigned, const char *ip, const char *mask, const char *gateway,
                             const char *nameserver, int ipv6rdipv4masklen, const char *ipv6rdbripv4addr,
                             int ipv6rdprefixlen, const char *ipv6rdprefix)
{
    char buf[sizeof(CtlMsgHeader) + sizeof(CtlDhcpcStateChangedMsgBody)] = { 0 };
    //CmsMsgHeader *msg=(CmsMsgHeader *) buf;
    //CmsRet ret;
    CtlMsgHeader *msg = (CtlMsgHeader *) buf;
    tsl_bool ret;
    CtlDhcpcStateChangedMsgBody *dhcpcBody = (CtlDhcpcStateChangedMsgBody *) (msg + 1);

#if 0
    msg->type = CMS_MSG_DHCPC_STATE_CHANGED;
    msg->src = MAKE_SPECIFIC_EID(getpid(), EID_DHCPC);
    msg->dst = EID_SSK;
    msg->flags_event = 1;
    msg->dataLength = sizeof(DhcpcStateChangedMsgBody);
#endif
    msg->data_length = sizeof(CtlDhcpcStateChangedMsgBody);

    dhcpcBody->addressAssigned = assigned;

    if (assigned) {
        if(client_config.interface)
            snprintf(dhcpcBody->wandev,sizeof(dhcpcBody->wandev)-1,"%s",client_config.interface);
        
        sprintf(dhcpcBody->ip, ip);
        sprintf(dhcpcBody->mask, mask);
        sprintf(dhcpcBody->gateway, gateway);
        sprintf(dhcpcBody->nameserver, nameserver);
        // 6rd
        strncpy(dhcpcBody->ipv6rdbripv4addr, ipv6rdbripv4addr, sizeof(dhcpcBody->ipv6rdbripv4addr) - 1);
        dhcpcBody->ipv6rdbripv4addr[sizeof(dhcpcBody->ipv6rdbripv4addr) - 1] = '\0';

        strncpy(dhcpcBody->ipv6rdprefix, ipv6rdprefix, sizeof(dhcpcBody->ipv6rdprefix) - 1);
        dhcpcBody->ipv6rdprefix[sizeof(dhcpcBody->ipv6rdprefix) - 1] = '\0';

        dhcpcBody->ipv6rdipv4masklen = ipv6rdipv4masklen;
        dhcpcBody->ipv6rdprefixlen = ipv6rdprefixlen;

        if (ipv6rdprefixlen) {  // TODO: more complex conditions
            dhcpcBody->b6rdAssigned = TRUE;
        } else {
            dhcpcBody->b6rdAssigned = FALSE;
        }
        //printf("dhcp4c info assigned\n");
    }
    //if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS) {
    if ((ret = ctlMsg_send(msg)) != TSL_B_TRUE) {
        cmsLog_error("could not send out DHCPC_STATUS_CHANGED, ret=%d", ret);
    } else {
        cmsLog_notice("sent out DHCPC_STATUS_CHANGED (assigned=%d)", assigned);
    }

    return;
}
#endif

// brcm

#if defined(AEI_VDSL_CUSTOMER_NCS)
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
void sendEventMessage(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway, const char *nameserver,
                      UINT32 LEDControl, unsigned int lease_time)
#else
void sendEventMessage(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway, const char *nameserver,
                      UINT32 LEDControl)
#endif
{
    char buf[sizeof(CmsMsgHeader) + sizeof(DhcpcStateChangedMsgBody)] = { 0 };
    CmsMsgHeader *msg = (CmsMsgHeader *) buf;
    DhcpcStateChangedMsgBody *dhcpcBody = (DhcpcStateChangedMsgBody *) (msg + 1);
    CmsRet ret;

    msg->type = CMS_MSG_DHCPC_STATE_CHANGED;
    msg->src = MAKE_SPECIFIC_EID(getpid(), EID_DHCPC);
    msg->dst = EID_SSK;
    msg->flags_event = 1;
    msg->dataLength = sizeof(DhcpcStateChangedMsgBody);

    dhcpcBody->addressAssigned = assigned;
	dhcpcBody->isExpired = isExpired;
    if (assigned || isExpired) {
        sprintf(dhcpcBody->ip, ip);
        sprintf(dhcpcBody->mask, mask);
        sprintf(dhcpcBody->gateway, gateway);
        sprintf(dhcpcBody->nameserver, nameserver);

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
        dhcpcBody->lease_time = lease_time;
#endif

    }
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    dhcpcBody->ledControl = LEDControl;
#endif

    if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS) {
        cmsLog_error("could not send out DHCPC_STATUS_CHANGED, ret=%d", ret);
    } else {
        cmsLog_notice("sent out DHCPC_STATUS_CHANGED (assigned=%d)", assigned);
    }

    return;
}
#else
void sendEventMessage(UBOOL8 assigned, UBOOL8 isExpired, DhcpcStateChangedMsgBody *options)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(DhcpcStateChangedMsgBody)]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;
   DhcpcStateChangedMsgBody *dhcpcBody = (DhcpcStateChangedMsgBody *) (msg+1);
   CmsRet ret;

   msg->type = CMS_MSG_DHCPC_STATE_CHANGED;
   msg->src = MAKE_SPECIFIC_EID(getpid(), EID_DHCPC);
   msg->dst = EID_SSK;
   msg->flags_event = 1;
   msg->dataLength = sizeof(DhcpcStateChangedMsgBody);

   if (assigned)
   {
      memcpy(dhcpcBody, options, sizeof(DhcpcStateChangedMsgBody));
   }
   dhcpcBody->addressAssigned = assigned;
   dhcpcBody->isExpired = isExpired;

   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out DHCPC_STATUS_CHANGED, ret=%d", ret);
   }
   else
   {
      cmsLog_notice("sent out DHCPC_STATUS_CHANGED (assigned=%d)", assigned);
   }

   return;
}
#endif

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_OPTION121)

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME)
void sendEventMessageWithRoute(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway, const char *nameserver,UINT32 LEDControl, route_info_t *routeList, int routeNum, unsigned int lease_time)
#else
void sendEventMessageWithRoute(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway, const char *nameserver,UINT32 LEDControl, route_info_t *routeList, int routeNum)
#endif
{
	char *buf = (char*)malloc(sizeof(CmsMsgHeader) + sizeof(DhcpcStateChangedMsgBody) + sizeof(route_info_t)*routeNum);
	if (buf == NULL)
	{
		return;
	}

	memset(buf, 0, sizeof(CmsMsgHeader) + sizeof(DhcpcStateChangedMsgBody) + sizeof(route_info_t)*routeNum);
	CmsMsgHeader *msg=(CmsMsgHeader *) buf;
	DhcpcStateChangedMsgBody *dhcpcBody = (DhcpcStateChangedMsgBody *) (msg+1);
	route_info_t *routeInfo = (route_info_t *)(dhcpcBody+1);
	route_info_t *routeTemp = routeList;
	int i;
	CmsRet ret;

	msg->type = CMS_MSG_DHCPC_STATE_CHANGED;
	msg->src = MAKE_SPECIFIC_EID(getpid(), EID_DHCPC);
	msg->dst = EID_SSK;
	msg->flags_event = 1;
	msg->dataLength = sizeof(DhcpcStateChangedMsgBody) + sizeof(route_info_t)*routeNum;

	dhcpcBody->addressAssigned = assigned;
	dhcpcBody->isExpired = isExpired;
	if (assigned)
	{
		sprintf(dhcpcBody->ip, ip);
		sprintf(dhcpcBody->mask, mask);
		sprintf(dhcpcBody->gateway, gateway);
		sprintf(dhcpcBody->nameserver, nameserver);
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) 
		dhcpcBody->lease_time = lease_time;
#endif

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    dhcpcBody->ledControl = LEDControl;
#endif	
	}else {
		if(nameserver) sprintf(dhcpcBody->nameserver, nameserver);
	}
	for (i=0; i<routeNum; i++)
	{
		routeInfo->next = routeNum;
		if (routeTemp != NULL)
		{
			sprintf((routeInfo+i)->desIP, "%s", routeTemp->desIP);
			sprintf((routeInfo+i)->netmask, "%s", routeTemp->netmask);
			sprintf((routeInfo+i)->gateway, "%s", routeTemp->gateway);
			routeTemp = routeTemp->next;	
		}		
	}

	if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
	{
		cmsLog_error("could not send out DHCPC_STATUS_CHANGED, ret=%d", ret);
	}
	else
	{
		cmsLog_notice("sent out DHCPC_STATUS_CHANGED (assigned=%d)", assigned);
	}
	
	free(buf);
   return;
}
#endif

void setStatus(int status)
{
   static int wasAssigned=0;

   if (status == 1)
   {
      wasAssigned = 1;
      /*
       * We don't have to send out a DHCPC_STATUS_CHANGED msg here.
       * We did that from run_script.
       */
   }
   else
   {
      /*
       * We went from assigned to un-assigned, send a DHCPC_STATUS_CHANGED
       * msg.
       */
      if (wasAssigned == 1)
      {
         wasAssigned = 0;
#if defined(AEI_VDSL_CUSTOMER_NCS)
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
            sendEventMessage(FALSE,FALSE, NULL, NULL, NULL, NULL, 0, 0);
#else
            sendEventMessage(FALSE,FALSE, NULL, NULL, NULL, NULL, 0);
#endif
#else
         sendEventMessage(FALSE, FALSE, NULL);
#endif
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
        memset(server_hwaddr,0,sizeof(server_hwaddr));
#endif
      }
   }

   return;
}

void setPid(void) {
    char cmd[128] = "";
    
    sprintf(cmd, "echo %d > %s", getpid(), pid_path);
    system(cmd); 
}

static void print_usage(void)
{
	printf(
"Usage: udhcpcd [OPTIONS]\n\n"
"  -c, --clientid=CLIENTID         Client identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"
"  -n, --now                       Exit with failure if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
"  -s, --script=file               Run file at dhcp events (default:\n"
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
	);
}


/* SIGUSR1 handler (renew) */
static void renew_requested(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGUSR1");
	if (state == BOUND || state == RENEWING || state == REBINDING ||
	    state == RELEASED) {
	    	listen_mode = LISTEN_KERNEL;
		server_addr = 0;
		packet_num = 0;
		state = RENEW_REQUESTED;
	}

	if (state == RELEASED) {
		listen_mode = LISTEN_RAW;
		state = INIT_SELECTING;
	}

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* SIGUSR2 handler (release) */
static void release_requested(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGUSR2");
	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "release");
	}

	listen_mode = 0;
	state = RELEASED;
	timeout = 0xffffffff;
/*	
#if defined(AEI_VDSL_CUSTOMER_NCS)
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
    sendEventMessage(TRUE, "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0", 0, 0);
#else

    #if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) //add william 2012-3-15
	sendEventMessage(TRUE, "0.0.0.0", "0.0.0.0", "0.0.0.0", "0.0.0.0", 0);
	#else
    sendEventMessage(FALSE, NULL, NULL, NULL, NULL, 0);
	#endif

#endif
#endif
*/
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
        memset(server_hwaddr,0,sizeof(server_hwaddr));
#endif
}

#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)

int AEI_checkIfHasIP(char *wanif)
{
    int fd;
    struct ifreq intf;
    int ret = 0;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket error!\n");
        return ret;
    }

    /*Coverity Fix CID 12256: Copy into fixed size buffer*/
    strlcpy(intf.ifr_name, wanif, sizeof(intf.ifr_name));
    if (ioctl(fd, SIOCGIFADDR, &intf) != -1) {
        if (strcmp(inet_ntoa(((struct sockaddr_in *)&intf.ifr_addr)->sin_addr), "0.0.0.0"))
            ret = 1;
        else
            ret = 0;
    } else
        ret = 0;

    close(fd);
    return ret;
}

static void AEI_handle_alarm(int sig)
{
    //check arp 
    LOG(LOG_INFO, "Received alarm\n");
    LOG(LOG_INFO, "needArpDetectContinue=%d,router_ip=%s,local_ip=%s, int=%s\n", needArpDetectContinue, router_ip,
        local_ip, client_config.interface);

    if (needArpDetectContinue) {
        if ((AEI_checkIfHasIP(client_config.interface) == 1)
            && !arpping(inet_addr(router_ip), inet_addr(local_ip), client_config.interface)) {
            LOG(LOG_INFO, "Gateway is up\n");
            arpDetectFailCount = 0;
            alarm(ALARM_1MIN);
            // Gateway is up
        } else {
            LOG(LOG_INFO, "Gateway is down for  %d times*%ds\n", arpDetectFailCount + 1, ALARM_1MIN);
            /*If 15 consecutive failures, the device will renew the DHCP address. */
            if (++arpDetectFailCount == MAX_ARP_FAIL_COUNT) {
                LOG(LOG_ERR, "Gateway is down for 15 mins,renew the IP\n");
                arpDetectFailCount = 0;
                needArpDetectContinue = 0;
                // Gateway is down
                //release_requested(0);
                renew_requested(0);
            } else {
                alarm(ALARM_1MIN);
            }
        }
    }
}
#endif

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
static int
AEI_ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, 6);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == 6)
			break;
	}
	return (i == 6);
}

static int AEI_read_server_hwaddr(char *mac_addr, unsigned long ip_addr)
{
	FILE *arpfile;
       int found = 0;
    char titlestr[100], ipaddrstr[20], hwtypestr[10], flagstr[6],
		hwaddrstr[20], maskstr[6], devicestr[20];
    
    struct in_addr	tmp_ip_addr;

    tmp_ip_addr.s_addr = ip_addr;
	if (!(arpfile = fopen("/proc/net/arp", "r")) || !fgets(titlestr, 100, arpfile)) {
            printf(" Failed to open /proc/net/arp or file format is invalid \n");
            if (arpfile)
                fclose(arpfile);
		return found;
	}
	/* parsing /proc/net/arp contents to get desired client mac */
	while (fscanf(arpfile, "%s\t%s\t%s\t%s\t%s\t%s\n",ipaddrstr, hwtypestr, flagstr, hwaddrstr, maskstr, devicestr) != EOF) {
            if (!strcasecmp(inet_ntoa(tmp_ip_addr), ipaddrstr)) {
                AEI_ether_atoe(hwaddrstr, mac_addr);
                found = 1;
		  break;
	     }
	}
	fclose(arpfile);
    return found;
}
#endif

/* Exit and cleanup */
static void exit_client(int retval)
{
	pidfile_delete(client_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* SIGTERM handler */
static void terminate(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGTERM");
	exit_client(0);
}


static void background(void)
{
	int pid_fd;
	if (client_config.quit_after_lease) {
		exit_client(0);
	} else if (!client_config.foreground) {
		pid_fd = pidfile_acquire(client_config.pidfile); /* hold lock during fork. */
		switch(fork()) {
		case -1:
			perror("fork");
			exit_client(1);
			/*NOTREACHED*/
		case 0:
			// brcm
			setPid();
			break; /* child continues */
		default:
			exit(0); /* parent exits */
			/*NOTREACHED*/
		}
		close(0);
		close(1);
		close(2);
		setsid();
		client_config.foreground = 1; /* Do not fork again. */
		pidfile_write_release(pid_fd);
	}
}


#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
/*
 * Function: AEI_write_ntp_server_to_file
 *
 * Description: To write the ntp server getting from dhcp packet into file.
 * Why don't we only send message to ntp? Because ntp will be killed and 
 * cann't receive message from the message queue. We should persist it in file.
 */
void AEI_write_ntp_server_to_file(char * s)
{
    FILE *fp = NULL;
    fp = fopen(NTP_SERVER_FILE, "w");
    if(fp != NULL)
    {
        fputs(s, fp);
        fclose(fp);
    }
}
/*
 * Function: AEI_send_ntp_server_msg
 *
 * Description: To send a message to ntp if dhcp client find 
 * ntp server address in dhcp packets.
 */
void AEI_send_ntp_server_msg()
{
    CmsMsgHeader ntpSrvMsg = EMPTY_MSG_HEADER;
    CmsRet ret = CMSRET_SUCCESS;

    /* Set CMS message header */
    ntpSrvMsg.type = CMS_MSG_DHCP_NTP_SERVER_FOUND;
    ntpSrvMsg.src = MAKE_SPECIFIC_EID(getpid(), EID_DHCPC);
    ntpSrvMsg.dst = EID_SNTP;

    if ((ret = cmsMsg_send(msgHandle, &ntpSrvMsg)) != CMSRET_SUCCESS) {
        LOG(LOG_ERR, "Send message failed");
    } else {
        LOG(LOG_ERR, "Send message success");
    }

}

/*
 * Function: AEI_find_ntp_server
 *
 * Description: To find option 42(RFC 2132: DHCP Options and BOOTP
 * Vender Extensions) in dhcp packet. If find option 42(DHCP_NTP_SERVER)
 * in packet, send a message with ntp server address to ntp.
 */
void AEI_find_ntp_server(struct dhcpMessage *packet)
{
    unsigned char *option= NULL;
    char * ntp_srv = NULL;

    /* Because the max length of option is 308, 1024 is ok here */
    int size = BUFLEN_1024; 
    char * tmp = NULL;

    /* How many octets found in dhcp packet */
    int len = 0; 

    /* 
     * The minimum length of option 42(DHCP_NTP_SERVER) is 4 octets, and
     * the length must be a multiple of 4.
     */
    int optlen = 4; 


    /* If find option 42(DHPC_NTP_SERVER) in dhcp packet */
    if((option = (unsigned char *)get_option(packet, DHCP_NTP_SERVER))) {

        /* 
         * According to file option.c, the number of octets stores in the  
         * address OPT_LEN - 2 offset to varible option
         */
        len = option[OPT_LEN - 2];

        ntp_srv = (char *)malloc(size * sizeof(char));
        memset(ntp_srv,'\0', size);
        tmp = ntp_srv;
        
        /* 
         * Read all options 42(DHCP_NTP_SERVER) from the packet.
         * Read 4 octets in every loop. 
         */
        for(;;) {

            /* Store ntp server ip address in ntp_srv seperating with a comma */
            tmp += sprintf(tmp, "%d.%d.%d.%d,", option[0], option[1], option[2], option[3]);
            
            /* Next 4 octets */
            option += optlen;
            len -= optlen;

            /* No more options */
            if (len <= 0)
                break;
        }

        /* Write ntp server to file */
        AEI_write_ntp_server_to_file(ntp_srv);

        /* Tell ntp to read the file to get ntp server */
        AEI_send_ntp_server_msg();

        /* After sending the message, we should free the memory we used */
        free(ntp_srv);
    } else {
        unlink(NTP_SERVER_FILE);
        LOG(LOG_ERR, "No NTP Server in message");
    }
}
#endif /* AEI_VDSL_CUSTOMER_CENTURYLINK */

#ifdef COMBINED_BINARY
int udhcpc(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	char *temp, *message;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int fd, retval;
	struct timeval tv;
	int c, len;
	struct ifreq ifr;
	struct dhcpMessage packet;
	struct in_addr temp_addr;
	int pid_fd;

	static struct option options[] = {
		{"clientid",	required_argument,	0, 'c'},
		{"foreground",	no_argument,		0, 'f'},
		{"hostname",	required_argument,	0, 'H'},
		{"help",	no_argument,		0, 'h'},
		{"interface",	required_argument,	0, 'i'},
		{"now", 	no_argument,		0, 'n'},
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
		{"script",	required_argument,	0, 's'},
		{"version",	no_argument,		0, 'v'},
		{"6rd",	no_argument,		0, '6'},
		{0, 0, 0, 0}
	};

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
int needUpdateServerMac = 0;
int arpTimeOut = 0;
#endif

	/* get options */
	while (1) {
		int option_index = 0;
// brcm
		c = getopt_long(argc, argv, "c:fH:hi:np:qr:s:d:v:6I:D:O:S:P:", options, &option_index);
		if (c == -1) break;
		
		switch (c) {
		case 'c':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
			client_config.clientid = malloc(len + 2);
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
			client_config.clientid[OPT_LEN] = len;
			strncpy(client_config.clientid + 2, optarg, len);
			break;
		case 'f':
			client_config.foreground = 1;
			break;
		case 'H':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = malloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, optarg, len);
			break;
		case 'h':
			print_usage();
			return 0;
		case 'i':
			client_config.interface =  optarg;
// brcm
			strcpy(session_path, optarg);
			break;
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
// brcm
		case 'd':
			strcpy(vendor_class_id, optarg);
			en_vendor_class_id=1;
			break;
		case '6':
			ipv6rd_opt = 1;
			break;
		case 's':
			client_config.script = optarg;
			break;
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			break;
		case 'I':
			strcpy(iaid, optarg);
            en_client_id = 1;
			break;
		case 'D':
			strcpy(duid, optarg);
            en_client_id = 1;
			break;
		case 'O':
			strcpy(oui_125, optarg);
			en_125 = 1;
			break;
		case 'S':
			strcpy(sn_125, optarg);
			en_125 = 1;
			break;
		case 'P':
			strcpy(prod_125, optarg);
			en_125 = 1;
			break;
		}
	}

	// brcm
        if (strlen(session_path) > 0) {
	    sprintf(status_path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_MSG);
	    sprintf(pid_path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_PID);
	}

	OPEN_LOG("udhcpc");
#ifdef VERBOSE
   cmsLog_setLevel(LOG_LEVEL_DEBUG);
#else
   cmsLog_setLevel(DEFAULT_LOG_LEVEL);
#endif

	LOG(LOG_INFO, "udhcp client (v%s) started", VERSION);

   cmsMsg_init(EID_DHCPC, &msgHandle);

	pid_fd = pidfile_acquire(client_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
#if defined(AEI_COVERITY_FIX)
            /*CID 12257:Copy into fixed size buffer*/
            strlcpy(ifr.ifr_name, client_config.interface, sizeof(ifr.ifr_name));
#else
            strcpy(ifr.ifr_name, client_config.interface);
#endif
		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			DEBUG(LOG_INFO, "adapter index %d", ifr.ifr_ifindex);
			client_config.ifindex = ifr.ifr_ifindex;
		} else {
			LOG(LOG_ERR, "SIOCGIFINDEX failed! %s", strerror(errno));
			exit_client(1);
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(client_config.arp, ifr.ifr_hwaddr.sa_data, 6);
			DEBUG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",
				client_config.arp[0], client_config.arp[1], client_config.arp[2], 
				client_config.arp[3], client_config.arp[4], client_config.arp[5]);
		} else {
			LOG(LOG_ERR, "SIOCGIFHWADDR failed! %s", strerror(errno));
			exit_client(1);
		}
	} else {
		LOG(LOG_ERR, "socket failed! %s", strerror(errno));
		exit_client(1);
	}
	close(fd);
	fd = -1;

	/* setup signal handlers */
	signal(SIGUSR1, renew_requested);
	signal(SIGUSR2, release_requested);
	signal(SIGTERM, terminate);
#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)
    signal(SIGALRM, AEI_handle_alarm);
#endif
	
	state = INIT_SELECTING;
	// brcm
	// run_script(NULL, "deconfig");

	// brcm
	setStatus(0);

	for (;;) {

		// brcm
		if ((old_mode != listen_mode) || (fd == -1)) {
		    old_mode = listen_mode;
		
        /*
        * After dhcpc runs as daemon(backgroud)mode, fd 0-2 be closed.  
        *  The sock fd may be 0.
        */
		    if (fd >= 0) {
			    close(fd);
			    fd = -1;
		    }
		
		    if (listen_mode == LISTEN_RAW) {
			    if ((fd = raw_socket(client_config.ifindex)) < 0) {
				    LOG(LOG_ERR, "couldn't create raw socket -- au revoir");
				    exit_client(0);
			    }
		    }
		    else if (listen_mode == LISTEN_KERNEL) {
			    if ((fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface)) < 0) {
				    LOG(LOG_ERR, "couldn't create server socket -- au revoir");
				    exit_client(0);
			    }			
		    } else 
			fd = -1;
		}
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
        if (needUpdateServerMac) {
                if(arpTimeOut && (arpTimeOut <= time(0)) )
                {
                 arppingForDHCPc(server_addr, requested_ip, client_config.arp, client_config.interface,TRUE,server_hwaddr);     
                 AEI_read_server_hwaddr(server_hwaddr, server_addr);
                 LOG(LOG_ERR, "main server address %02x:%02x:%02x:%02x:%02x:%02x",
                  server_hwaddr[0], server_hwaddr[1],server_hwaddr[2],
                  server_hwaddr[3], server_hwaddr[4], server_hwaddr[5]);

                    needUpdateServerMac = 0;
                    /* Set timeout back to its normal value */
                    arpTimeOut = 0;
					start = time(0); //add william 2011-12-30
                    timeout = t1 + start;
                }
        }
#endif
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
        /* For mantis#442 issue */
        int aei_usec = get_randomize_usec();
        tv.tv_sec = timeout - time(0);
        if(aei_usec >= 0)
        {
            tv.tv_usec = aei_usec;
        }
        else
        {
            if(tv.tv_sec >= 1)
            {
                tv.tv_sec = tv.tv_sec - 1;
                tv.tv_usec = 1000000 + aei_usec;
            }
        }
#else
		tv.tv_sec = timeout - time(0);
		tv.tv_usec = 0;
#endif
		FD_ZERO(&rfds);
		if (listen_mode) FD_SET(fd, &rfds);
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
        if (tv.tv_sec >= 0) {
#else
		if (tv.tv_sec > 0) {
#endif
			retval = select(fd + 1, &rfds, NULL, NULL, &tv);
		} else retval = 0; /* If we already timed out, fall through */

		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)
                needArpDetectContinue = 0;
                arpDetectFailCount = 0;
#endif
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
                needUpdateServerMac = 1;
#endif
				// brcm
				setStatus(0);
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
                if(packet_num == 0 || packet_num >= BACKOFF_THRESHOLD)
                {
                    xid = random_xid();
                }
                /* send discover packet */
                send_discover(xid, requested_ip);   /* broadcast */


                if(packet_num < BACKOFF_THRESHOLD)
                {
                    timeout = time(0) + (2 << packet_num);
                }
                else
                {
                    if (client_config.abort_if_no_lease)
                    {
                        LOG(LOG_INFO, "No lease, failing.");
                        exit_client(1);
                    }
                    timeout = time(0) + (2 << (BACKOFF_THRESHOLD - 1));
                }
                packet_num ++;
#else				
				if (packet_num < 3) {
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */
					
#if defined(AEI_VDSL_CUSTOMER_NCS)
					timeout = time(0) + (2 << packet_num); //modify william 2011-12-14
#else
					timeout = time(0) + ((packet_num == 2) ? REQ_TIMEOUT : 2);
#endif
					packet_num++;
				} else {
					if (client_config.abort_if_no_lease) {
						LOG(LOG_INFO,
						    "No lease, failing.");
						exit_client(1);
				  	}
					/* wait to try again */
					packet_num = 0;
					timeout = time(0) + INIT_TIMEOUT;
				}
#endif				
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
                if(packet_num < BACKOFF_THRESHOLD) {
#else			
				if (packet_num < 3) {
#endif				
					/* send request packet */
					if (state == RENEW_REQUESTED)
						send_renew(xid, server_addr, requested_ip); /* unicast */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */
					
#if defined(AEI_VDSL_CUSTOMER_NCS)
                    timeout = time(0) + (2 << packet_num); //modify william 2011-12-14
#else
					timeout = time(0) + ((packet_num == 2) ? REQ_TIMEOUT : 2);
#endif
					packet_num++;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                    if (packet_num == MAXNONVALIDIPCOUNT)
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME)						
                    sendEventMessage(FALSE,FALSE, NULL, NULL, NULL, NULL, 1,0);
#else
                        sendEventMessage(FALSE,FALSE, NULL, NULL, NULL, NULL, 1);
#endif
#endif
				} else {
					/* timed out, go back to init state */
					state = INIT_SELECTING;
					timeout = time(0);
					packet_num = 0;
					listen_mode = LISTEN_RAW;
					
				}
				break;
			case BOUND:
				/* Lease is starting to run out, time to enter renewing state */
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
                if (needUpdateServerMac) {
                    LOG(LOG_ERR,"needUpdateServerMac , do nothing\n");               
                    break;
                }               
#endif                
				state = RENEWING;
				listen_mode = LISTEN_KERNEL;
				DEBUG(LOG_INFO, "Entering renew state");
				/* fall right through */
			case RENEWING:
				/* Either set a new T1, or enter REBINDING state */
				if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
					timeout = time(0) + (t2 - t1);
					DEBUG(LOG_INFO, "Entering rebinding state");
				} else {
					/* send a request packet */
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
                    if(send_renew(xid, server_addr, requested_ip) ==0)
                        listen_mode = LISTEN_RAW;      
#else
					send_renew(xid, server_addr, requested_ip); /* unicast */
#endif
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
				if ((lease - t2) <= (lease / 14400 + 1)) {
					/* timed out, enter init state */
					state = INIT_SELECTING;
					LOG(LOG_INFO, "Lease lost, entering init state");
					run_script(NULL, "deconfig");
					timeout = time(0);
					packet_num = 0;
					listen_mode = LISTEN_RAW;
				} else {
					/* send a request packet */
					send_renew(xid, 0, requested_ip); /* broadcast */

#if defined(AEI_VDSL_CUSTOMER_NCS)
                    listen_mode = LISTEN_RAW;
#endif

					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
				timeout = 0xffffffff;
#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)
                needArpDetectContinue = 0;
                arpDetectFailCount = 0;
#endif
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */

			if (listen_mode == LISTEN_KERNEL) {
				if (get_packet(&packet, fd) < 0) continue;
			} else {
				if (get_raw_packet(&packet, fd) < 0) continue;
			} 

#if defined(AEI_VDSL_CUSTOMER_DHCPFORCERENEW) //add william 2011-11-18
			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) 
			{
                DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
                continue;
            }

            if (packet.xid != xid && *message != DHCPFORCERENEW ) 
            {
                DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)", (unsigned long)packet.xid, xid);
                continue;
            }
#else
			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
			
			if ((message = (char *)get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}

#endif
			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = (char *)get_option(&packet, DHCP_SERVER_ID))) {
						memcpy(&server_addr, temp, 4);
						xid = packet.xid;
						requested_ip = packet.yiaddr;
						
						/* enter requesting state */
						state = REQUESTING;
						timeout = time(0);
						packet_num = 0;
					} else {
						DEBUG(LOG_ERR, "No server ID in message");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
					if (!(temp = (char *)get_option(&packet, DHCP_LEASE_TIME))) {
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
						lease = 60*60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
					}
						
					/* enter bound state */
					t1 = lease / 2;
					
					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;
					temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
					start = time(0);
					timeout = t1 + start;
					requested_ip = packet.yiaddr;
					run_script(&packet,
						   ((state == RENEWING || state == REBINDING) ? "renew" : "bound"));

					state = BOUND;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                                        AEI_find_ntp_server(&packet);
#endif
#if defined(AEI_VDSL_CUSTOMER_DHCPFORCERENEW) //add william 2011-11-18
					listen_mode = LISTEN_KERNEL;
					//listen_mode = LISTEN_RAW;
#else
					listen_mode = LISTEN_NONE;
#endif
#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)
                    alarm(ALARM_1MIN);
                    needArpDetectContinue = 1;
                    arpDetectFailCount = 0;
#endif
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
                   if(needUpdateServerMac)
                   {
                        if(AEI_read_server_hwaddr(server_hwaddr, server_addr))
                        {
                            needUpdateServerMac = 0;
                            LOG(LOG_ERR,"AEI_read_server_hwaddr sucessful");               
                        }
                        else
                        {
                            if (0 == arpTimeOut) 
                                timeout = arpTimeOut = time(0) + 30; 
                        }  
                   }
#endif
					
					// brcm
                    close(fd);
                    fd = -1;
					setStatus(1);
					background();
					
				} else if (*message == DHCPNAK) {
					/* return to init state */
					LOG(LOG_INFO, "Received DHCP NAK");
					if (state != REQUESTING)
						run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = time(0);
					requested_ip = 0;
					packet_num = 0;
					listen_mode = LISTEN_RAW;

					// brcm
					setStatus(0);
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME)						
					sendEventMessage(FALSE,FALSE, NULL, NULL, NULL, NULL, 1,0);
#else
					sendEventMessage(FALSE,FALSE, NULL, NULL, NULL, NULL, 1);
#endif
#endif
				}
				break;
			case BOUND:
#if defined(AEI_VDSL_CUSTOMER_DHCPFORCERENEW)	//add william 2011-11-18			
				if(*message == DHCPFORCERENEW)
				{
					state = RENEWING;
					packet_num = 0;
					timeout=0;
				}
				break;
#endif
			case RELEASED:
				/* ignore all packets */
				break;
			}					
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */
			
		} else {
			/* An error occured */
			DEBUG(LOG_ERR, "Error on select");
		}
		
	}
	return 0;
}

