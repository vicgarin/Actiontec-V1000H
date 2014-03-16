/*	$KAME: dhcp6c.c,v 1.164 2006/01/10 02:46:09 jinmei Exp $	*/
/*
 * Copyright (C) 1998 and 1999 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/queue.h>
#include <errno.h>
#include <limits.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_var.h>
#endif

#include <netinet/in.h>
#ifdef __KAME__
#include <net/if_dl.h>
#include <netinet6/in6_var.h>
#endif

#include <arpa/inet.h>
#include <netdb.h>

#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <ifaddrs.h>

#include <dhcp6.h>
#include <config.h>
#include <common.h>
#include <timer.h>
#include <dhcp6c.h>
#include <control.h>
#include <dhcp6_ctl.h>
#include <dhcp6c_ia.h>
#include <prefixconf.h>
#include <auth.h>

#ifdef AEI_CONTROL_LAYER
#include "tsl_common.h"
#include "ctl_msg.h"
#include "dbussend_msg.h"

#ifdef ACTION_TEC_IPV6_CODE_FOR_IOT
#include "netlink_dad.h"
#endif

//#define ACTION_TEC_IPV6_CODE_FOR_RAD
#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT)// && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
#include <sys/ioctl.h>
#include <linux/rtnetlink.h>
#include <asm/byteorder.h>

#include "ctl_objectid.h"
#include "ctl_validstrings.h"

static int rad_ra_Mbit = -1;
static int rad_ra_Obit = -1;
static int rad_ra_routelifetime = -1;
static int rad_fd = -1;
struct msghdr rad_msg;
struct sockaddr_nl rad_dest_addr;
struct nlmsghdr *p_rad_nlh;
struct iovec rad_iov;
struct icmpv6_nd_ra *p_rad_icmpv6;
#endif

#ifdef CDROUTER_TEST_DHCP6C
int all_ia_pre_order=0;
#endif
CtlDhcp6cStateChangedMsgBody ctldhcp6cMsgBody;
CtlDhcp6cStateChangedMsgBody ctldhcp6cMsgBody_Old;
static int send_confirm=1;
#ifdef CONFIG_IOT_RECONFIGURATION
static struct authparam g_auth_param;
//current only support one valid PD. 
int got_valid_ia_pd=0;
int got_valid_ia_na=0;
#endif
#endif
//#define AEI_RENEW_OLD_PD 
#ifdef AEI_RENEW_OLD_PD
static int iReq = 1;
#endif

static int debug = 0;
static int exit_ok = 0;
static sig_atomic_t sig_flags = 0;
#define SIGF_TERM 0x1
#define SIGF_HUP 0x2

const dhcp6_mode_t dhcp6_mode = DHCP6_MODE_CLIENT;

int sock;	/* inbound/outbound udp port */
int rtsock;	/* routing socket */
int ctlsock = -1;		/* control TCP port */
char *ctladdr = DEFAULT_CLIENT_CONTROL_ADDR;
char *ctlport = DEFAULT_CLIENT_CONTROL_PORT;

#define DEFAULT_KEYFILE SYSCONFDIR "/dhcp6cctlkey"
#define CTLSKEW 300

static char *conffile = DHCP6C_CONF;

static const struct sockaddr_in6 *sa6_allagent;
static struct duid client_duid;
static char *pid_file = DHCP6C_PIDFILE;

static char *ctlkeyfile = DEFAULT_KEYFILE;
static struct keyinfo *ctlkey = NULL;
static int ctldigestlen;

static int infreq_mode = 0;
static int infreq_reconf = 0;

static inline int get_val32 __P((char **, int *, u_int32_t *));
static inline int get_ifname __P((char **, int *, char *, int));

static void usage __P((void));
static void client6_init __P((void));
static void client6_startall __P((int));
static void free_resources __P((struct dhcp6_if *));
static void client6_mainloop __P((void));
static int client6_do_ctlcommand __P((char *, ssize_t));
static void client6_reload __P((void));
static int client6_ifctl __P((char *ifname, u_int16_t));
static void check_exit __P((void));
static void process_signals __P((void));
static struct dhcp6_serverinfo *find_server __P((struct dhcp6_event *,
						 struct duid *));
static struct dhcp6_serverinfo *select_server __P((struct dhcp6_event *));
static void client6_recv __P((void));
static int client6_recvadvert __P((struct dhcp6_if *, struct dhcp6 *,
				   ssize_t, struct dhcp6_optinfo *));
static int client6_recvreply __P((struct dhcp6_if *, struct dhcp6 *,
				  ssize_t, struct dhcp6_optinfo *));
static void client6_signal __P((int));
static struct dhcp6_event *find_event_withid __P((struct dhcp6_if *,
						  u_int32_t));
#ifdef ACTION_TEC_IPV6_CODE_FOR_IOT
#ifdef CONFIG_IOT_RECONFIGURATION
static int client6_reconfigure __P((struct dhcp6_if *, struct dhcp6 *,
				  ssize_t, struct dhcp6_optinfo *));
static int client6_reconfigure_info __P((struct dhcp6_if *));
static struct dhcp6_event *find_event_withoutid __P((struct dhcp6_if *));
#endif
void remove_na_address __P((struct dhcp6_if *,char*,int*) );
int rmna = 0;
int rmpd = 0;
#endif
static int construct_confdata __P((struct dhcp6_if *, struct dhcp6_event *));
static int construct_reqdata __P((struct dhcp6_if *, struct dhcp6_optinfo *,
    struct dhcp6_event *));
static void destruct_iadata __P((struct dhcp6_eventdata *));
static void tv_sub __P((struct timeval *, struct timeval *, struct timeval *));
static struct dhcp6_timer *client6_expire_refreshtime __P((void *));
static int process_auth __P((struct authparam *, struct dhcp6 *dh6, ssize_t,
    struct dhcp6_optinfo *));
static int set_auth __P((struct dhcp6_event *, struct dhcp6_optinfo *));

struct dhcp6_timer *client6_timo __P((void *));
int client6_start __P((struct dhcp6_if *));
static void info_printf __P((const char *, ...));

#if 1 //brcm
int updateDhcp6sConfDnsList __P((struct dhcp6_optinfo *));
static void sendDnsEventMessage __P((const char *));
//static void sendDhcp6cEventMessage __P((void));
char *ifname_info;
char *brcm_ptr;
char brcm_ifname[32];
char l2_ifname[32];
#endif

extern int client6_script __P((char *, int, struct dhcp6_optinfo *));

#if ACTION_TEC_IPV6_CODE_FOR_IOT
#include <libtr69_client.h>
//#define CTLOID_WAN_IP_CONN              "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1"

#define MAX_TIMES_TO_RETRY_TR69 	(5)  // Try 5 times, interval 1 seconds.

tsl_int_t tsl_strcmp(const char *s1, const char *s2) 
{
   char emptyStr = '\0';
   char *str1 = (char *) s1;
   char *str2 = (char *) s2;

   if (str1 == NULL)
   {
      str1 = &emptyStr;
   }
   if (str2 == NULL)
   {
      str2 = &emptyStr;
   }

   return strcmp(str1, str2);
}

tsl_int_t ctlUtl_getWANDeviceID(const char* WANAccessType)
{
	tsl_char_t *c_WANDeviceCount = NULL;
	tsl_int_t i_WANDeviceCount =1;
	tsl_int_t i;
	tsl_char_t WANDeviceOID[256] = {0};
	tsl_char_t *c_WANAccessType = NULL;
	tsl_int_t  val_type;
	
	tr69_get_unfresh_leaf_data( CTLOID_IGD".WANDeviceNumberOfEntries",
				& c_WANDeviceCount,
				& val_type );
	i_WANDeviceCount = (tsl_int_t)c_WANDeviceCount;

	for(i = 0; i<i_WANDeviceCount; i++)
	{
		sprintf(WANDeviceOID, "InternetGatewayDevice.WANDevice.%d.WANCommonInterfaceConfig.WANAccessType", i+1);
		//ctllog_debug("WANDeviceOID = %s\n", WANDeviceOID);
		tr69_get_unfresh_leaf_data( WANDeviceOID,
					& c_WANAccessType,
					& val_type );
		if(!tsl_strcmp(WANAccessType,c_WANAccessType))
		{
			CTLMEM_FREE_BUF_AND_NULL_PTR(c_WANAccessType);
			return i+1;
		}
		CTLMEM_FREE_BUF_AND_NULL_PTR(c_WANAccessType);
	}
	return -1;
}
tsl_bool_t ctlUtl_genNewCtlObjectID(const char* srcOid, char* destOid, int iIndex, int newId)
{
	char* delim = ".";
	char* p;
	int i = 0;
	tsl_bool_t rv = tsl_b_false;
	char src_Oid[256] = {0};
	char new_Id[2]={0};
	strncpy(src_Oid, srcOid, 256);
	strcpy(destOid, strtok(src_Oid, delim));
	//ctllog_debug("src_Oid = %s \n", src_Oid);
	//ctllog_debug("destOid = %s \n", destOid);
//	if(newId>=10 || newId<=0)
//		perror("not support device id: %d now, only allow 1~9\n", newId);
	newId = newId%10;
	sprintf(new_Id, "%d", newId);
	//ctllog_debug("newId = %d \n", newId);
	//ctllog_debug("new_Id = %s \n", new_Id);
	//ctllog_debug("new_Id[0] = %c \n", new_Id[0]);
	
	while((p=strtok(NULL, delim)))
	{
		strcat(destOid, ".");
		//ctllog_debug("destOid = %s \n", destOid);
		//ctllog_debug("p = %s \n", p);

		if(!strcmp(p, "1"))
		{
			i++;
			if(i == iIndex)
			{
				strcat(destOid, new_Id);
				//ctllog_debug("destOid = %s \n", destOid);
				rv = tsl_b_true;
			}
			else
			{
				strcat(destOid, "1");
				//ctllog_debug("destOid = %s \n", destOid);
			}
		}
		else
		{
			strcat(destOid, p);
			//ctllog_debug("destOid = %s \n", destOid);
		}
	}
	return rv;
}

tsl_int_t getPppConnOID(char* strPPPOID)
{
	tsl_int_t result = -1;
	tsl_int_t i;
	tsl_int_t type;
	tsl_char_t * conntype = NULL;
	tsl_int_t i_DeviceID = 1;

	do {
		if( NULL == strPPPOID ) {
			perror("PppOid NULL!\n");
			break;
		}

		for( i=0; i<MAX_TIMES_TO_RETRY_TR69; i++) {
			if( tr69_get_unfresh_leaf_data( CTLOID_IGD".X_AEI_COM_Physical_WAN",
						& conntype,
						& type ) >=0 )
			{
				break;
			}
			sleep(1);
		}
		if( i>= MAX_TIMES_TO_RETRY_TR69 ) {
			perror("FAIL to access DataCenter!\n");
			break;
		}
		if( (i_DeviceID = ctlUtl_getWANDeviceID(conntype)) < 0 ) {
			CTLMEM_FREE_BUF_AND_NULL_PTR(conntype);
			perror("FAIL to get Physical_WAN device ID\n");
			break;
		}
		ctlUtl_genNewCtlObjectID(CTLOID_WAN_PPP_CONN, strPPPOID, 1, i_DeviceID);
		CTLMEM_FREE_BUF_AND_NULL_PTR(conntype);
		result = 0;
	} while(0);

	return result;
}

tsl_int_t getIPConnOID(char* strIPOID)
{
	tsl_int_t result = -1;
	tsl_int_t i;
	tsl_int_t type;
	tsl_char_t * conntype = NULL;
	tsl_int_t i_DeviceID = 1;
	
	do {
		if( NULL == strIPOID ) {
			perror("IpOid NULL!\n");
			break;
		}
		
		for( i=0; i<MAX_TIMES_TO_RETRY_TR69; i++) {
			if( tr69_get_unfresh_leaf_data( CTLOID_IGD".X_AEI_COM_Physical_WAN",
						& conntype,
						& type ) >=0 )
			{
				break;
			}
			sleep(1);
		}
		if( i>= MAX_TIMES_TO_RETRY_TR69 ) {
			perror("FAIL to access DataCenter!\n");
			break;
		}
		if( (i_DeviceID = ctlUtl_getWANDeviceID(conntype)) < 0 ) {
			CTLMEM_FREE_BUF_AND_NULL_PTR(conntype);
			perror("FAIL to get Physical_WAN device ID\n");
			break;
		}
		ctlUtl_genNewCtlObjectID(CTLOID_WAN_IP_CONN, strIPOID, 1, i_DeviceID);
		CTLMEM_FREE_BUF_AND_NULL_PTR(conntype);
		result = 0;
	} while(0);

	return result;
}



int
ctl_layer_cfg_check( struct dhcp6_if *ifp,
        char* wan_dhcpv6_ipv6_addr )
{
       /* read startup data from ctl_layer data center */
       char* data_val = NULL;
       int data_type = 0;


	   tsl_bool ipConn = TSL_B_FALSE;
	   tsl_bool pppConn = TSL_B_FALSE;
	   tsl_int_t ret;
	   tsl_int_t  type;
	   tsl_bool found = TSL_B_FALSE;

	   tsl_bool b_ipv6enable = TSL_B_FALSE;
	   tsl_char_t *c_ipv6enable = NULL;
	   tsl_char_t *ifName = NULL;
	   /////////////////////////////////////////
	   
	   tsl_char_t strIPOID[256]={0};
	   tsl_char_t strPara[256]={0};
	   tsl_char_t strPPPOID[256]={0};
	   
	   if( getPppConnOID(strPPPOID) < 0 ) {
		   perror( "Fail to get PPPConnOid\n" );
		   return -1;
	   }
	   if( getIPConnOID(strIPOID) < 0 ) {
		   perror( "Fail to get IPConnOid\n" );
		   return -1;
	   }
	   
	   sprintf(strPara, "%s.Enable", strIPOID);
	   ret = tr69_get_unfresh_leaf_data( strPara,
				   & c_ipv6enable,
				   & type );
	   if( ret >=0 ) {
		   b_ipv6enable = (tsl_bool)c_ipv6enable;
		   if( b_ipv6enable ) {
			   found = TSL_B_TRUE;
			   ipConn = TSL_B_TRUE;
		   }
	   }
	   if( TSL_B_FALSE == found ) {
				   sprintf(strPara, "%s.Enable", strPPPOID);
				   ret = tr69_get_unfresh_leaf_data( strPara,
				   & c_ipv6enable,
				   & type );
		   if( ret >= 0 ) {
			   b_ipv6enable = (tsl_bool)c_ipv6enable;
			   if( b_ipv6enable ) {
				   found = TSL_B_TRUE;
				   pppConn = TSL_B_TRUE;
			   }
		   }
	   }
   
	   if (TSL_B_FALSE == found) {
		   perror( "NOT found wan connection!" );
		   return -1;
	   } else {

		   sprintf(strPara, "%s.X_BROADCOM_COM_ExternalIPv6Address", ipConn ?strIPOID :strPPPOID );
	       tr69_get_unfresh_leaf_data( strPara,
	           &data_val,
	           &data_type);
			if(!IS_EMPTY_STRING(data_val))
			{
		   		strcpy( wan_dhcpv6_ipv6_addr, data_val );
				CTLMEM_FREE_BUF_AND_NULL_PTR(data_val);
			}
			else {
                if(data_val) {
                    CTLMEM_FREE_BUF_AND_NULL_PTR(data_val);
                }
                    
				return -1;
             }

	   }

    return 0;
}

void remove_na_address(struct dhcp6_if *ifp,char * wanip,int* flag )
{
	if( 0 == *flag )
	{
		char szCmd[264] = {0};
		snprintf(szCmd,263, "ip -6 addr del %s dev %s", wanip,ifp->ifname);
		system(szCmd);
		*flag  = 1;
	}
}
#endif

/**********************************************************************************
*	ACTION_TEC_IPV6_CODE_FOR_IOT  and  ACTION_TEC_IPV6_CODE_FOR_RAD
**********************************************************************************/
#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
typedef enum {
	DHCPV6_MODE_NONE,
	DHCPV6_MODE_STATELESS,
	DHCPV6_MODE_STATEFUL
}DHCPV6_MODE;

#define MAX_PAYLOAD 1024
struct icmpv6_nd_ra {
			__u8		hop_limit;
#if defined(__LITTLE_ENDIAN_BITFIELD)
			__u8		reserved:3,
					router_pref:2,
				        home_agent:1,
					other:1,
					managed:1;

#elif defined(__BIG_ENDIAN_BITFIELD)
			__u8		managed:1,
					other:1,
				        home_agent:1,
					router_pref:2,
					reserved:3;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
			__u16		rt_lifetime;
                } u_nd_ra;

static void nl_close(int sd)
{
    if (sd >= 0) close(sd);
}

static int nl_open(void)
{
    struct sockaddr_nl nladdr;
    int sd;

    sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_RAD);
    if (sd < 0)
    {
    	perror("DHCP6c: open socket failed!\n");
		return -1;
    }
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    nladdr.nl_pid = getpid();  /* self pid */
    /* interested in group 1<<0 */
    nladdr.nl_groups = 1;

    if (bind(sd, (struct sockaddr*)&nladdr, sizeof(nladdr)) < 0) 
	{
        nl_close(sd);
		fprintf(stdout, "DHCP6c: bind socket failed!\n");
        return -1;
    }
    return sd;
}

static int nl_get_msg(int fd, fd_set ra)
{
	
}

static int get_ifname_from_datamodel(char **pp_ifname)
{
	tsl_bool_t b_enable;

	tsl_int_t tr69_type = 0;

	tsl_int_t found = 0;

    char *p_ifname = NULL;


	tsl_char_t strIPOID[256]={0};
	tsl_char_t strPara[256]={0};
	tsl_char_t strPPPOID[256]={0};

	getPppConnOID(strPPPOID);   
	getIPConnOID(strIPOID);

	do {
		sprintf(strPara, "%s.Enable", strIPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN IPConn being Disabled.\n" );
			break;
		}
		///
		sprintf(strPara, "%s.X_BROADCOM_COM_IfName", strIPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&p_ifname, (tsl_int_t *)&tr69_type) < 0)
			break;

        *pp_ifname = p_ifname;
        found = 1;


                     
	} while(0);


	do {
		if (found )
			break;
		sprintf(strPara, "%s.Enable", strPPPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN PPPConn being Disabled.\n" );
			break;
		}

		sprintf(strPara, "%s.X_BROADCOM_COM_IfName", strPPPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&p_ifname, (tsl_int_t *)&tr69_type) < 0)
			break;

        *pp_ifname = p_ifname;
        found = 1;

                             
	} while(0);

	if (found) {
        fprintf(stdout,"pp_ifname is: %s\n", *pp_ifname);
		return 0;
	}
    else
    {
        fprintf(stdout,"pp_ifname: both IP and PPP disconnected\n");
        return -1;
    }

}

static int get_wan_conn_type(void)
{
	int tr69_type = 0;
	char *tr69_chr_val = NULL;
	int bb_type = 1;		//1:Eth;	2:Moca
	
	if (tr69_get_unfresh_leaf_data(CTLOID_IGD".X_AEI_COM_Physical_WAN",
					(tsl_void_t **)&tr69_chr_val,
					(tsl_int_t *)&tr69_type ) < 0)		
	{
		fprintf(stdout, "%s[%d]: get WAN physical type failed\n", __FILE__, __LINE__);
		return -1;
	}
	if ((tr69_chr_val != NULL) && (strcmp(tr69_chr_val, CTLVS_ETHERNET) == 0))
		bb_type = 1;
	else
		bb_type = 2;
	free(tr69_chr_val);
	tr69_chr_val = NULL;

	return bb_type;
}

static int get_router_lifetime_from_datamodel(void)
{
	tsl_bool_t b_enable;

	tsl_int_t tr69_type = 0;

	tsl_int_t found = 0;

    tsl_int_t param_int = 0;
    tsl_int_t route_lifetime = 0;

	tsl_char_t strIPOID[256]={0};
	tsl_char_t strPara[256]={0};
	tsl_char_t strPPPOID[256]={0};

	getPppConnOID(strPPPOID);   
	getIPConnOID(strIPOID);

	do {
		sprintf(strPara, "%s.Enable", strIPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf(stdout, "WAN IPConn being Disabled.\n" );
			break;
		}
		///
		sprintf(strPara, "%s.X_BROADCOM_COM_TMP_RALIFETIME", strIPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&param_int, (tsl_int_t *)&tr69_type) < 0)
			break;

        route_lifetime = param_int;
        found = 1;


                     
	} while(0);


	do {
		if (found )
			break;
		sprintf(strPara, "%s.Enable", strPPPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf(stdout, "WAN PPPConn being Disabled.\n" );
			break;
		}
		sprintf(strPara, "%s.X_BROADCOM_COM_TMP_RALIFETIME", strPPPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&param_int, (tsl_int_t *)&tr69_type) < 0)
			break;

        route_lifetime = param_int;
        found = 1;

                             
	} while(0);

	if (found) {
        fprintf(stdout,"RADVD Router Lifetime is: %d\n", route_lifetime);
		//return 0;
	}
    else
    {
        fprintf(stdout,"RADVD Router Lifetime is: both IP and PPP disconnected\n");
        //return -1;
    }

	fprintf(stdout, "%s[%d]: route_lifetime=%d\n", __FILE__, __LINE__, route_lifetime);

	return route_lifetime;
}

static int get_RA_config_type_from_datamodel(void)
{
    char * ipv6RAtype = NULL;
    int tr69_type = 0;
    tr69_get_unfresh_leaf_data( CTLOID_IPV6_LAN_HOST_CFG ".X_AEI_COM_IPv6RAConfigType",(tsl_void_t **)&ipv6RAtype, (tsl_int_t *)&tr69_type );
    if (!tsl_strcmp( ipv6RAtype, "Static"))
	{
	    CTLMEM_FREE_BUF_AND_NULL_PTR( ipv6RAtype );
		return 0;
	}
    else
        {
            CTLMEM_FREE_BUF_AND_NULL_PTR( ipv6RAtype );
            return 1;
        }
}



static int save_ra_info_to_datamodel(int Mbit, int Obit, int router_lifetime)
{
	tsl_bool_t b_enable;

	tsl_int_t tr69_type = 0;

	tsl_int_t found = 0;

    tsl_int_t param_int = 0;

	tsl_char_t strIPOID[256]={0};
	tsl_char_t strPara[256]={0};
	tsl_char_t strPPPOID[256]={0};

	getPppConnOID(strPPPOID);   
	getIPConnOID(strIPOID);

	do {
		sprintf(strPara, "%s.Enable", strIPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN IPConn being Disabled.\n" );
			break;
		}
		///
		param_int = Mbit;
		sprintf(strPara, "%s.X_BROADCOM_COM_MBIT", strIPOID);
		tr69_set_unfresh_leaf_data(strPara, (void *)&param_int, TR69_NODE_LEAF_TYPE_INT);

        param_int = Obit;
        sprintf(strPara, "%s.X_BROADCOM_COM_OBIT", strIPOID);
        tr69_set_unfresh_leaf_data(strPara, (void *)&param_int, TR69_NODE_LEAF_TYPE_INT);

         param_int = router_lifetime;
        sprintf(strPara, "%s.X_BROADCOM_COM_TMP_RALIFETIME", strIPOID);
		tr69_set_unfresh_leaf_data(strPara, (void *)&param_int, TR69_NODE_LEAF_TYPE_INT);
        
        found = 1;
                     
	} while(0);


	do {
		if (found )
			break;
		sprintf(strPara, "%s.Enable", strPPPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf(stdout, "WAN PPPConn being Disabled.\n" );
			break;
		}
		param_int = Mbit;
		sprintf(strPara, "%s.X_BROADCOM_COM_MBIT", strPPPOID);
		tr69_set_unfresh_leaf_data(strPara, (void *)&param_int, TR69_NODE_LEAF_TYPE_INT);

        param_int = Obit;
        sprintf(strPara, "%s.X_BROADCOM_COM_OBIT", strPPPOID);
        tr69_set_unfresh_leaf_data(strPara, (void *)&param_int, TR69_NODE_LEAF_TYPE_INT);

         param_int = router_lifetime;
        sprintf(strPara, "%s.X_BROADCOM_COM_TMP_RALIFETIME", strPPPOID);
		tr69_set_unfresh_leaf_data(strPara, (void *)&param_int, TR69_NODE_LEAF_TYPE_INT);
        
        //found = 1;

                             
	} while(0);


	return 0;
}

static int is_MO_flags_changed(int Mbit, int Obit)
{
	tsl_bool_t b_enable;

	tsl_int_t tr69_type = 0;

	tsl_int_t found = 0;
	int dm_Mbit = -1;
	int dm_Obit = -1;
	int is_changed = 0;
    
	char *tr69_chr_val = NULL;

	tsl_char_t strIPOID[256]={0};
	tsl_char_t strPara[256]={0};
	tsl_char_t strPPPOID[256]={0};

	getPppConnOID(strPPPOID);   
	getIPConnOID(strIPOID);

	do {
		sprintf(strPara, "%s.Enable", strIPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN IPConn being Disabled.\n" );
			break;
		}
		///
		
		sprintf(strPara, "%s.X_BROADCOM_COM_MBIT", strIPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&tr69_chr_val, (tsl_int_t *)&tr69_type) < 0)
			break;
        dm_Mbit = (int)((long)tr69_chr_val);

        sprintf(strPara, "%s.X_BROADCOM_COM_OBIT", strIPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&tr69_chr_val, (tsl_int_t *)&tr69_type) < 0)
            break;
        dm_Obit = (int)((long)tr69_chr_val);

        found = 1;
                     
	} while(0);


	do {
		if (found )
			break;
		sprintf(strPara, "%s.Enable", strPPPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN PPPConn being Disabled.\n" );
			break;
		}
		sprintf(strPara, "%s.X_BROADCOM_COM_MBIT", strPPPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&tr69_chr_val, (tsl_int_t *)&tr69_type) < 0)
			break;
        dm_Mbit = (int)((long)tr69_chr_val);

        sprintf(strPara, "%s.X_BROADCOM_COM_OBIT", strPPPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (tsl_void_t **)&tr69_chr_val, (tsl_int_t *)&tr69_type) < 0)
            break;
        dm_Obit = (int)((long)tr69_chr_val);
        
        //found = 1;

                             
	} while(0);

    
	
	if ((dm_Mbit == -1) || (dm_Obit == -1))
		is_changed = 0;
	else
	{
		if ((dm_Mbit == Mbit) && (dm_Obit == Obit))
			is_changed = 0;
		else if ((dm_Mbit == Mbit) && ((dm_Mbit == 1) && (dm_Obit != Obit)))
			is_changed = 0;
		else
			is_changed = 1;
	}
	fprintf(stdout, "rad: M/O, is_changed(%d)(%d,%d->%d,%d)\n", is_changed, dm_Mbit, dm_Obit, Mbit, Obit);
	
	return is_changed;
}

static DHCPV6_MODE get_dhcpv6_mode(int Mbit, int Obit)
{
	DHCPV6_MODE mode = DHCPV6_MODE_NONE;
	if ((0 == Mbit) && (0 == Obit))
		mode = DHCPV6_MODE_NONE;
	else if ((0 == Mbit) &&(1 == Obit))
		mode = DHCPV6_MODE_STATELESS;
	else
		mode = DHCPV6_MODE_STATEFUL;
	fprintf(stdout, "get_dhcpv6_mode: mode(%d), M(%d), O(%d)\n", mode, Mbit, Obit);
	return mode;
}
static int isUserDefineMode()
{
    int ret=0;
    /*only pppv6 mode could define address type
    * if dynamic address/pd and dynamic DNS,following normal case
    * if dyncmic address/pd and static DNS, following normal case
    * if static ddress/pd and dynamic DNS,should ignore RA M/O flag
    * if static ddress/pd and static DNS,should not dhcp6c
    */
	tsl_bool_t b_enable=TSL_B_FALSE;

	tsl_int_t tr69_type = 0;
	char strPara[256]={0};
	char strPPPOID[256]={0};
    char *addrType=NULL;

	getPppConnOID(strPPPOID);   
	sprintf(strPara, "%s.Enable", strPPPOID);
    tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type );
    fprintf(stdout, "%s[%d]: ===>>strPara=%s,b_enable=%d!\n", __FILE__, __LINE__, strPara,b_enable);    
    if( TSL_B_FALSE == b_enable )  
        return 0;
    sprintf(strPara, "%s.X_BROADCOM_COM_IPv6AddressingType", strPPPOID);
    tr69_get_unfresh_leaf_data(strPara,	&addrType, &tr69_type );
    fprintf(stdout, "%s[%d]: addrType=%s!\n", __FILE__, __LINE__, addrType); 
    if(!IS_EMPTY_STRING(addrType)&& cmsUtl_strcmp(addrType, CTLVS_STATIC)==0)
        return 1;

//tr69_get_unfresh_leaf_data( CTLOID_IPV6_LAN_HOST_CFG ".IPv6DNSConfigType",&ipv6dnstype, &type );
    
   
    return ret;
}
static int gen_dhcp6c_conf(const char *ifName_info, tsl_bool dynamicIpEnabled, DHCPV6_MODE mode)
{
   char cmdLine[128];
   char ifName[32];
   char *ptr;
   //tsl_u32_t pid;
   char dhcp6cConfFile[64];
   char ianaAssoc_buff[512] = { 0 };
   FILE *fp;
   char iapdAssocLine[256];
const char *dhcp6cConf = "\
interface %s\n\
{\n\
   send %s%s%s;\n\
   request domain-name-servers;\n\
   request domain-name;\n\
};\n\
%s\
%s\
";
const char *dhcp6cConf_none = "\
interface %s\n\
{\n\
   send %s%s%s;\n\
};\n\
%s\
%s\
";

	const char *dhcp6cConf1 = "\
	interface %s\n\
	{\n\
	   request domain-name-servers;\n\
	   request domain-name;\n\
	};\n\
	";
/*
 * AEI_IPV6_DEPLOY
 * Sam @ Fri Oct 22 16:22:07 CST 2010
 */
const char *iapd = "ia-pd 0";
const char *iapdAssoc = "\
id-assoc pd 0\n\
{\n\
   prefix-interface br0\n\
   {\n\
	  sla-id 0;\n\
	  sla-len 0;\n\
   };\n\
};\n\
";
const char *iapdAssoc1 = "\
id-assoc pd 0\n\
{\n\
   prefix-interface br0\n\
   {\n\
	  sla-id 0;\n\
	  sla-len 0;\n\
   };\n\
   prefix %s infinity;\n\
};\n\
";
const char *iana = "ia-na 1";

// UNH-IOT
// for No WAN.IPv6.address
const char *ianaAssoc_0 = "\
id-assoc na 1\n\
{\n\
};\n\
";
// for Having WAN.IPv6.address
const char *ianaAssoc_1 = "\
id-assoc na 1\n\
{\n\
	address %s %u %u;\n\
};\n\
";
	char* wan_ipv6_addr = NULL;
	char* pfx = NULL;
	//int type;
	//int ret = -1;
	tsl_bool_t b_enable=TSL_B_FALSE;

	tsl_int_t tr69_type = 0;

	tsl_int_t found = 0;

	char strIPOID[256]={0};
	char strPara[256]={0};
	char strPPPOID[256]={0};

	getPppConnOID(strPPPOID);   
	getIPConnOID(strIPOID);

 	do {
		sprintf(strPara, "%s.Enable", strIPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
        
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN IPConn being Disabled.\n" );
			break;
		}
		///
		sprintf(strPara, "%s.X_AEI_COM_IPv6SitePrefix_BackUp", strIPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (void**)&pfx, (tsl_int_t *)&tr69_type) < 0)
			break;
      
		sprintf(strPara, "%s.X_BROADCOM_COM_ExternalIPv6Address", strIPOID);
		if(tr69_get_unfresh_leaf_data(strPara, (void**)&wan_ipv6_addr, &tr69_type)<0)
          break;
        found = 1;
             
	} while(0);   
    
	do {
		if (found )
			break;
		sprintf(strPara, "%s.Enable", strPPPOID);
		if( tr69_get_unfresh_leaf_data(strPara,	&b_enable, &tr69_type ) < 0 )
			break;
		if( TSL_B_FALSE == b_enable ) {
			fprintf( stdout,"WAN PPPConn being Disabled.\n" );
			break;
		}
		sprintf(strPara, "%s.X_AEI_COM_IPv6SitePrefix_BackUp", strPPPOID);
		if (tr69_get_unfresh_leaf_data(strPara, (void**)&pfx, (tsl_int_t *)&tr69_type) < 0)
			break;


        found = 1;

                             
	} while(0);
    
    
	if (found == 1)
	{
		if (!IS_EMPTY_STRING(pfx))
		{
			fprintf(stdout, "X_BROADCOM_COM_IPv6SitePrefix = %s\n", pfx);
			snprintf(iapdAssocLine, sizeof(iapdAssocLine), iapdAssoc1, pfx);
			CTLMEM_FREE_BUF_AND_NULL_PTR(pfx);
		}
		else
		{
			fprintf(stdout, "X_BROADCOM_COM_IPv6SitePrefix = NULL\n");
			snprintf(iapdAssocLine, sizeof(iapdAssocLine), iapdAssoc);
		}
	}
	else
	{
		fprintf(stdout, "can't get X_BROADCOM_COM_IPv6SitePrefix\n");
		snprintf(iapdAssocLine, sizeof(iapdAssocLine), iapdAssoc);
	}

	// UNH-IOT
	// for adding dhcp6c.conf with WAN.IPv6.address
	if (found == 1)
	{
		if (!IS_EMPTY_STRING(wan_ipv6_addr))
		{
        	char* pTmp = strstr(wan_ipv6_addr, "/");
			if(pTmp!=NULL)
				*pTmp = '\0';
			fprintf(stdout, "X_BROADCOM_COM_ExternalIPv6Address = %s\n", wan_ipv6_addr );
			snprintf( ianaAssoc_buff, sizeof(ianaAssoc_buff), ianaAssoc_1, wan_ipv6_addr, 1800, 3600 );
			CTLMEM_FREE_BUF_AND_NULL_PTR(wan_ipv6_addr);
		}
		else
		{
			fprintf(stdout, "X_BROADCOM_COM_ExternalIPv6Address = NULL\n");
			snprintf( ianaAssoc_buff, sizeof(ianaAssoc_buff), ianaAssoc_0 );
		}
	}
	else
	{
		fprintf(stdout, "can't get X_BROADCOM_COM_ExternalIPv6Address\n");
	}


	strncpy(ifName, ifName_info, sizeof(ifName));
	if ((ptr = strstr(ifName_info, "__")) != NULL)
	{
	  ifName[ptr-ifName_info] = '\0';
	}

	sprintf(dhcp6cConfFile, "/var/dhcp6c_%s.conf", ifName);
	if ((fp = fopen(dhcp6cConfFile, "w")) == NULL)
	{
	  /* error */
	  fprintf(stdout, "failed to create %s\n", dhcp6cConfFile);
	  return TSL_B_FALSE;
	}
	else
	{
	  /* create dhcp6c.conf */
	  if (dynamicIpEnabled)
	  {
		 //fprintf(fp, dhcp6cConf, ifName, iapd, ", ", iana, iapdAssocLine, ianaAssoc);
		 if (mode == DHCPV6_MODE_STATEFUL)
			 fprintf(fp, dhcp6cConf, ifName, iapd, ", ", iana, iapdAssocLine, ianaAssoc_buff );
		 else if (mode == DHCPV6_MODE_STATELESS)
			fprintf(fp, dhcp6cConf, ifName, iapd, "", "", iapdAssocLine, "");
		 else if (mode == DHCPV6_MODE_NONE)
			fprintf(fp, dhcp6cConf_none, ifName, iapd, "", "", iapdAssocLine, "");
	  }
	  else
	  {
		 //fprintf(fp, dhcp6cConf, ifName, iapd, "", "", iapdAssocLine, "");
		 fprintf(fp, dhcp6cConf1, ifName);
	  }

	  fclose(fp);
	}

	//	 ctl_rutIpt_setupFirewallForDHCPv6(TSL_B_TRUE, ifName);

#if defined( _BHR2 ) || defined( SUPPORT_BHR1 )
	snprintf(cmdLine, sizeof(cmdLine), "/bin/dhcp6c -c %s -f %s -D", dhcp6cConfFile, ifName_info);
#elif defined (SUPPORT_PK5000)
	snprintf(cmdLine, sizeof(cmdLine), "/bin/dhcp6c -c %s -dDf %s ", dhcp6cConfFile, ifName_info);
#else
	snprintf(cmdLine, sizeof(cmdLine), "/bin/dhcp6c -c %s %s ", dhcp6cConfFile, ifName_info);
#endif
	//ctllog_debug("==================>>>>>>>>>>>>>>cmdLine: %s \n"  cmdLine);

   return 0;
}


#endif		//defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
/**********************************************************************************
*	ACTION_TEC_IPV6_CODE_FOR_IOT  and  ACTION_TEC_IPV6_CODE_FOR_RAD   end
**********************************************************************************/
static int get_dhcpv6_request_manner(void)
{
    char * dhcpv6_manner = NULL;
    int tr69_type = 0;
    tr69_get_unfresh_leaf_data( CTLOID_IPV6_LAN_HOST_CFG ".X_AEI_COM_IPv6DHCPReqManner",(tsl_void_t **)&dhcpv6_manner, (tsl_int_t *)&tr69_type );
    if (!tsl_strcmp( dhcpv6_manner, "Renew"))
	{
	    CTLMEM_FREE_BUF_AND_NULL_PTR( dhcpv6_manner );
		return 0;
	}
    else
        {
            CTLMEM_FREE_BUF_AND_NULL_PTR( dhcpv6_manner );
            return 1;
        }
}



#define MAX_ELAPSED_TIME 0xffff

int
main(argc, argv)
	int argc;
	char **argv;
{
	int ch, pid;
	char *progname;
	FILE *pidfp;
	struct dhcp6_if *ifp;
#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
		int retval = 0;
		static fd_set rad_fds;
		struct timeval timeout;
		char *p_ifname = NULL;
		int status = -1;
#endif

#ifndef HAVE_ARC4RANDOM
	srandom(time(NULL) & getpid());
#endif

	if ((progname = strrchr(*argv, '/')) == NULL)
		progname = *argv;
	else
		progname++;

	while ((ch = getopt(argc, argv, "c:dDfik:p:")) != -1) {
		switch (ch) {
		case 'c':
			conffile = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'D':
			debug = 2;
			break;
		case 'f':
			foreground++;
			break;
		case 'i':
			infreq_mode = 1;
			break;
		case 'k':
			ctlkeyfile = optarg;
			break;
		case 'p':
			pid_file = optarg;
			break;
		default:
			usage();
			exit(0);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0) {
		usage();
		exit(0);
	}
	
#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
		rad_fd = nl_open();
		if (rad_fd < 0) 
		{
			fprintf(stdout, "nl_open failed\n");
			return -1;
		}
		p_rad_nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
		if (!p_rad_nlh) {
			fprintf(stdout, "Malloc Error\n");
			return -1;
		}
		memset(p_rad_nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	
		rad_iov.iov_base = (void *)p_rad_nlh;
		rad_iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
		rad_msg.msg_name = (void *)&rad_dest_addr;
		rad_msg.msg_namelen = sizeof(rad_dest_addr);
		rad_msg.msg_iov = &rad_iov;
		rad_msg.msg_iovlen = 1; 
	
		FD_ZERO(&rad_fds);
		FD_SET(rad_fd, &rad_fds);
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		retval = select(rad_fd + 1, &rad_fds, NULL, NULL, &timeout);
		if (-1 == retval)
		{
			fprintf(stdout, "%s[%d]: select error!\n", __FILE__, __LINE__);
			nl_close(rad_fd);
			return -1;
		}
		else if (0 == retval)
		{//timeout
			rad_ra_Mbit = 1;
			rad_ra_Obit = 1;
			/* save default ra indo to datamode */
			save_ra_info_to_datamodel(rad_ra_Mbit, rad_ra_Obit, 0);
		}
		else if (retval)
		{
			status = recvmsg(rad_fd, &rad_msg, 0);
			if (status < 0) 
			{
				nl_close(rad_fd);	
				free(p_rad_nlh);
				fprintf(stdout, "%s[%d]: recvmsg failed!\n", __FILE__, __LINE__);
				return -1;
			}
			p_rad_icmpv6 = (struct icmpv6_nd_ra *)NLMSG_DATA(p_rad_nlh);
			fprintf(stdout, "Message length is %d,M bit %d,O bit %d,lifetime is %d\n",
				p_rad_nlh->nlmsg_len,p_rad_icmpv6->managed,p_rad_icmpv6->other,ntohs(p_rad_icmpv6->rt_lifetime));
			
			/* check if Mbit and Obit is invalid */
			rad_ra_Mbit = p_rad_icmpv6->managed;
			rad_ra_Obit = p_rad_icmpv6->other;
			if ((rad_ra_Mbit < 0) || (rad_ra_Obit < 0))
			{
				fprintf(stdout, "%s[%d]: M(%d) or O(%d) flag is wrong!\n", __FILE__, __LINE__, rad_ra_Mbit, rad_ra_Obit);
				nl_close(rad_fd);
				free(p_rad_nlh);
				return -1;
			}
			
			/* save ra info to datamode */
			save_ra_info_to_datamodel(p_rad_icmpv6->managed, p_rad_icmpv6->other, p_rad_icmpv6->rt_lifetime);
		}
		if (get_ifname_from_datamodel(&p_ifname) < 0)
		{
			fprintf(stdout, "%s[%d]: get ifname from datamodel failed!\n", __FILE__, __LINE__);
			nl_close(rad_fd);
			free(p_rad_nlh);
			return -1;
		}
		fprintf(stdout, "%s[%d]: ifname=%s,rad_ra_Mbit=%d,rad_ra_Obit=%d!\n", 
          __FILE__, __LINE__, p_ifname,rad_ra_Mbit,rad_ra_Obit);
        if(!isUserDefineMode())
		gen_dhcp6c_conf(p_ifname, 1, get_dhcpv6_mode(rad_ra_Mbit, rad_ra_Obit));
		fprintf(stdout, "dhcp6c mode when starting: %d\n", get_dhcpv6_mode(rad_ra_Mbit, rad_ra_Obit));						
#endif

	memset(&ctldhcp6cMsgBody, 0, sizeof(CtlDhcp6cStateChangedMsgBody));
	memset(&ctldhcp6cMsgBody_Old, 0, sizeof(CtlDhcp6cStateChangedMsgBody));

	if (foreground == 0)
		openlog(progname, LOG_NDELAY|LOG_PID, LOG_DAEMON);


	setloglevel(debug);
	
   ifname_info = argv[0];

   cmsUtl_strncpy(brcm_ifname, ifname_info, sizeof(brcm_ifname));
   if ( (brcm_ptr = strstr(ifname_info, "__")) == NULL)
   {
      cmsUtl_strncpy(l2_ifname, ifname_info, sizeof(l2_ifname));
   }
   else
   {
      brcm_ifname[brcm_ptr-ifname_info] = '\0';
      brcm_ptr += 2;
      cmsUtl_strncpy(l2_ifname, brcm_ptr, sizeof(l2_ifname));
   }

	client6_init();
	while (argc-- > 0) { 
		if ((ifp = ifinit(argv[0])) == NULL) {
			dprintf(LOG_ERR, FNAME, "failed to initialize %s",
			    argv[0]);
			exit(1);
		}
		argv++;
	}

	if (infreq_mode == 0 && (cfparse(conffile)) != 0) {
		dprintf(LOG_ERR, FNAME, "failed to parse configuration file");
		exit(1);
	}

#if 0 //brcm
	if (foreground == 0 && infreq_mode == 0) {
		if (daemon(0, 0) < 0)
			err(1, "daemon");
	}
#endif

	/* dump current PID */
	pid = getpid();
	if ((pidfp = fopen(pid_file, "w")) != NULL) {
		fprintf(pidfp, "%d\n", pid);
		fclose(pidfp);
	}

	client6_startall(0);
	client6_mainloop();
#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
	nl_close(rad_fd);
	rad_fd = -1;
#endif	
	exit(0);
}

static void
usage()
{

	fprintf(stderr, "usage: dhcp6c [-c configfile] [-dDfi] "
	    "[-p pid-file] interface [interfaces...]\n");
}

/*------------------------------------------------------------*/

void
client6_init()
{
	struct addrinfo hints, *res;
	static struct sockaddr_in6 sa6_allagent_storage;
	int error, on = 1;

	/* get our DUID */
	if (get_duid(DUID_FILE, &client_duid, l2_ifname)) {
#ifdef AEI_CONTROL_LAYER
		if (get_duid(DUID_FILE, &client_duid, "br0")) {
#endif		
		dprintf(LOG_ERR, FNAME, "failed to get a DUID");
		exit(1);
#ifdef AEI_CONTROL_LAYER
		}
#endif		
	}

	if (dhcp6_ctl_authinit(ctlkeyfile, &ctlkey, &ctldigestlen) != 0) {
		dprintf(LOG_NOTICE, FNAME,
		    "failed initialize control message authentication");
		/* run the server anyway */
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	error = getaddrinfo(NULL, DH6PORT_DOWNSTREAM, &hints, &res);
	if (error) {
		dprintf(LOG_ERR, FNAME, "getaddrinfo: %s",
		    gai_strerror(error));
		exit(1);
	}
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) {
		dprintf(LOG_ERR, FNAME, "socket");
		exit(1);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,
		       &on, sizeof(on)) < 0) {
		dprintf(LOG_ERR, FNAME,
		    "setsockopt(SO_REUSEPORT): %s", strerror(errno));
		exit(1);
	}
#ifdef IPV6_RECVPKTINFO
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on,
		       sizeof(on)) < 0) {
		dprintf(LOG_ERR, FNAME,
			"setsockopt(IPV6_RECVPKTINFO): %s",
			strerror(errno));
		exit(1);
	}
#else
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_PKTINFO, &on,
		       sizeof(on)) < 0) {
		dprintf(LOG_ERR, FNAME,
		    "setsockopt(IPV6_PKTINFO): %s",
		    strerror(errno));
		exit(1);
	}
#endif
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on,
		       sizeof(on)) < 0) {
		dprintf(LOG_ERR, FNAME,
		    "setsockopt(sock, IPV6_MULTICAST_LOOP): %s",
		    strerror(errno));
		exit(1);
	}
#ifdef IPV6_V6ONLY
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
	    &on, sizeof(on)) < 0) {
		dprintf(LOG_ERR, FNAME, "setsockopt(IPV6_V6ONLY): %s",
		    strerror(errno));
		exit(1);
	}
#endif

	/*
	 * According RFC3315 2.2, only the incoming port should be bound to UDP
	 * port 546.  However, to have an interoperability with some servers,
	 * the outgoing port is also bound to the DH6PORT_DOWNSTREAM.
	 */
	if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
		dprintf(LOG_ERR, FNAME, "bind: %s", strerror(errno));
		exit(1);
	}
	freeaddrinfo(res);

	/* open a routing socket to watch the routing table */
	if ((rtsock = socket(PF_ROUTE, SOCK_RAW, 0)) < 0) {
		dprintf(LOG_ERR, FNAME, "open a routing socket: %s",
		    strerror(errno));
		exit(1);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	error = getaddrinfo(DH6ADDR_ALLAGENT, DH6PORT_UPSTREAM, &hints, &res);
	if (error) {
		dprintf(LOG_ERR, FNAME, "getaddrinfo: %s",
		    gai_strerror(error));
		exit(1);
	}
	memcpy(&sa6_allagent_storage, res->ai_addr, res->ai_addrlen);
	sa6_allagent = (const struct sockaddr_in6 *)&sa6_allagent_storage;
	freeaddrinfo(res);

	/* set up control socket */
	if (ctlkey == NULL)
		dprintf(LOG_NOTICE, FNAME, "skip opening control port");
	else if (dhcp6_ctl_init(ctladdr, ctlport,
	    DHCP6CTL_DEF_COMMANDQUEUELEN, &ctlsock)) {
		dprintf(LOG_ERR, FNAME,
		    "failed to initialize control channel");
		exit(1);
	}

	if (signal(SIGHUP, client6_signal) == SIG_ERR) {
		dprintf(LOG_WARNING, FNAME, "failed to set signal: %s",
		    strerror(errno));
		exit(1);
	}
	if (signal(SIGTERM, client6_signal) == SIG_ERR) {
		dprintf(LOG_WARNING, FNAME, "failed to set signal: %s",
		    strerror(errno));
		exit(1);
	}
}

int
client6_start(ifp)
	struct dhcp6_if *ifp;
{
	struct dhcp6_event *ev;

	/* make sure that the interface does not have a timer */
	if (ifp->timer != NULL) {
		dprintf(LOG_DEBUG, FNAME,
		    "removed existing timer on %s", ifp->ifname);
		dhcp6_remove_timer(&ifp->timer);
	}

	/* create an event for the initial delay */
	if ((ev = dhcp6_create_event(ifp, DHCP6S_INIT)) == NULL) {
		dprintf(LOG_NOTICE, FNAME, "failed to create an event");
		return (-1);
	}
	TAILQ_INSERT_TAIL(&ifp->event_list, ev, link);

	if ((ev->authparam = new_authparam(ifp->authproto,
	    ifp->authalgorithm, ifp->authrdm)) == NULL) {
		dprintf(LOG_WARNING, FNAME, "failed to allocate "
		    "authentication parameters");
		dhcp6_remove_event(ev);
		return (-1);
	}

	if ((ev->timer = dhcp6_add_timer(client6_timo, ev)) == NULL) {
		dprintf(LOG_NOTICE, FNAME, "failed to add a timer for %s",
		    ifp->ifname);
		dhcp6_remove_event(ev);
		return (-1);
	}
	dhcp6_reset_timer(ev);

	return (0);
}

static void
client6_startall(isrestart)
	int isrestart;
{
	struct dhcp6_if *ifp;

	for (ifp = dhcp6_if; ifp; ifp = ifp->next) {
		if (isrestart &&ifreset(ifp)) {
			dprintf(LOG_NOTICE, FNAME, "failed to reset %s",
			    ifp->ifname);
			continue; /* XXX: try to recover? */
		}
		if (client6_start(ifp))
			exit(1); /* initialization failure.  we give up. */
	}
}

static void
free_resources(freeifp)
	struct dhcp6_if *freeifp;
{
	struct dhcp6_if *ifp;

	for (ifp = dhcp6_if; ifp; ifp = ifp->next) {
		struct dhcp6_event *ev, *ev_next;

		if (freeifp != NULL && freeifp != ifp)
			continue;

		/* release all IAs as well as send RELEASE message(s) */
		release_all_ia(ifp);

		/*
		 * Cancel all outstanding events for each interface except
		 * ones being released.
		 */
		for (ev = TAILQ_FIRST(&ifp->event_list); ev; ev = ev_next) {
			ev_next = TAILQ_NEXT(ev, link);

			if (ev->state == DHCP6S_RELEASE)
				continue; /* keep it for now */

			dhcp6_remove_event(ev);
		}
	}
}

static void
check_exit()
{
	struct dhcp6_if *ifp;

	if (!exit_ok)
		return;

	for (ifp = dhcp6_if; ifp; ifp = ifp->next) {
		/*
		 * Check if we have an outstanding event.  If we do, we cannot
		 * exit for now.
		 */
		if (!TAILQ_EMPTY(&ifp->event_list))
			dprintf(LOG_INFO, FNAME, "WARNING: EVENT LEFT WHILE EXITING!!");
//			return;   //brcm: TODO!!
	}

	/* We have no existing event.  Do exit. */
	dprintf(LOG_INFO, FNAME, "exiting");

	exit(0);
}

static void
process_signals()
{
	if ((sig_flags & SIGF_TERM)) {
		exit_ok = 1;
		free_resources(NULL);
		unlink(pid_file);
		check_exit();
	}
	if ((sig_flags & SIGF_HUP)) {
		dprintf(LOG_INFO, FNAME, "restarting");
		free_resources(NULL);
		client6_startall(1);
	}

	sig_flags = 0;
}

static void
client6_mainloop()
{
	struct timeval *w;
	int ret, maxsock;
	fd_set r;

#ifdef ACTION_TEC_IPV6_CODE_FOR_DAD
	 int fd_netlink = 0;
	 int rc_netlink = 0;

	 rc_netlink = netlink_dad_start( &fd_netlink );
	 if( rc_netlink == 0 ) {
				dprintf(LOG_INFO, FNAME, "netlink_dad_start ok, fd_netlink = %u", fd_netlink );
	 }
	 else {
				dprintf(LOG_INFO, FNAME, "netlink_dad_start failed, so, I cannot get the DAD failed messge from kernel.");
	 }
#endif

	while(1) {
		if (sig_flags)
			process_signals();

		w = dhcp6_check_timer();

		FD_ZERO(&r);
		FD_SET(sock, &r);
		maxsock = sock;
		if (ctlsock >= 0) {
			FD_SET(ctlsock, &r);
			maxsock = (sock > ctlsock) ? sock : ctlsock;
			(void)dhcp6_ctl_setreadfds(&r, &maxsock);
		}

#ifdef ACTION_TEC_IPV6_CODE_FOR_DAD
		 // added fd_netlink
		 if( rc_netlink == 0 ) {
						FD_SET( fd_netlink, &r);
						maxsock = (fd_netlink > maxsock) ? fd_netlink: maxsock;

			 //dprintf(LOG_INFO, FNAME, "FD_SET: fd_netlink, maxsock = %u", maxsock );
		 }
#endif
#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
				FD_SET(rad_fd, &r);
				maxsock = (rad_fd > maxsock) ? rad_fd: maxsock;
#endif

		ret = select(maxsock + 1, &r, NULL, NULL, w);

		switch (ret) {
		case -1:
			if (errno != EINTR) {
				dprintf(LOG_ERR, FNAME, "select: %s",
				    strerror(errno));
				exit(1);
			}
			continue;
		case 0:	/* timeout */
			break;	/* dhcp6_check_timer() will treat the case */
		default:
			break;
		}
		if (FD_ISSET(sock, &r))
			client6_recv();

#ifdef ACTION_TEC_IPV6_CODE_FOR_DAD
        if (FD_ISSET(fd_netlink, &r)) {
            static struct dhcp6_if *p_if = NULL;

            rc_netlink = netlink_recv( fd_netlink, &p_if );
            if( rc_netlink == 0 ) {
                // ifconfig DADFAILED detected
                char temp[64] = { 0 };
                inet_ntop( AF_INET6, &( p_if->addr), temp, sizeof(temp));
                dprintf(LOG_INFO, FNAME, "netlink_recv: DADFAILED detected, ifname: %s, ipaddr: %s", p_if->ifname, temp );

                decline_all_ia( p_if );
            }
        }				
#endif

#if defined(ACTION_TEC_IPV6_CODE_FOR_IOT) && defined(ACTION_TEC_IPV6_CODE_FOR_RAD)
				if (FD_ISSET(rad_fd, &r))
				{
					int status = -1;
					int tr69_type = 0;
					char *tr69_chr_val = NULL;
					int b_dhcp6senable;
			
					status = recvmsg(rad_fd, &rad_msg, 0);
					if (status < 0) 
					{
						nl_close(rad_fd);	
						free(p_rad_nlh);
						fprintf(stdout, "%s[%d]: select, recvmsg failed!!!\n", __FILE__, __LINE__); 			
					}
					else
					{								
						p_rad_icmpv6 = (struct icmpv6_nd_ra *)NLMSG_DATA(p_rad_nlh);
						fprintf(stdout, "Message length is %d,M bit %d,O bit %d,lifetime is %d\n",
							p_rad_nlh->nlmsg_len,p_rad_icmpv6->managed,p_rad_icmpv6->other,ntohs(p_rad_icmpv6->rt_lifetime));
						
						/* check if Mbit and Obit is invalid */
						rad_ra_Mbit = p_rad_icmpv6->managed;
						rad_ra_Obit = p_rad_icmpv6->other;
						if ((rad_ra_Mbit < 0) || (rad_ra_Obit < 0))
						{
							fprintf(stdout, "%s[%d]: select, M(%d) or O(%d) flag is wrong!!!\n", __FILE__, __LINE__, rad_ra_Mbit, rad_ra_Obit);
							nl_close(rad_fd);
							free(p_rad_nlh);
						}
						else	//M and O flag is right
						{					
							/* check if M/O flags is changed */
							if (is_MO_flags_changed(rad_ra_Mbit, rad_ra_Obit) == 1) //M and O flag is changed
							{
								char *p_ifname = NULL;
								
								if (get_ifname_from_datamodel(&p_ifname) < 0)
								{
									fprintf(stdout, "%s[%d]: select, get ifname from datamodel failed!\n", __FILE__, __LINE__);
									nl_close(rad_fd);
									free(p_rad_nlh);							
								}
								else	//get ifname right
								{
								    if(!isUserDefineMode())
								        {
									//re-generate conf file 				
									fprintf(stdout, "%s[%d]: ifname=%s!\n", __FILE__, __LINE__, p_ifname);
									gen_dhcp6c_conf(p_ifname, 1, get_dhcpv6_mode(rad_ra_Mbit, rad_ra_Obit));
									fprintf(stdout, "dhcp6c mode change: %d\n", get_dhcpv6_mode(rad_ra_Mbit, rad_ra_Obit));
									
									//reload dhcp6c
									free_resources(NULL);
									client6_reload();
									//restart dhcp6c
									dprintf(LOG_INFO, FNAME, "dhcp6c restarting!!!!!\n");							
									client6_startall(1);
								}
							}					
						}
						}
						
						/* Check if router lifetime is changed */
						rad_ra_routelifetime = get_router_lifetime_from_datamodel();
						/* save ra info to datamode */
						save_ra_info_to_datamodel(p_rad_icmpv6->managed, p_rad_icmpv6->other, p_rad_icmpv6->rt_lifetime);
						
						/* check if Rouetr Lifetime is changed */
                        if(get_RA_config_type_from_datamodel()==1)//dynamic
                        {
    						if ((rad_ra_routelifetime != p_rad_icmpv6->rt_lifetime) &&
    							((p_rad_icmpv6->rt_lifetime == 0) || (rad_ra_routelifetime == 0)))
    						{
    							rad_ra_routelifetime = p_rad_icmpv6->rt_lifetime;
    							tr69_get_unfresh_leaf_data( CTLOID_IPV6_LAN_HOST_CFG".DHCPv6ServerEnable",
    									(tsl_void_t **)&tr69_chr_val,
    									(tsl_int_t *)&tr69_type);
    							b_dhcp6senable = (int)((long)tr69_chr_val);
    							fprintf(stdout, "rad: b_dhcp6senable=%d\n", b_dhcp6senable);
    							tr69_set_leaf_data( CTLOID_IPV6_LAN_HOST_CFG".DHCPv6ServerEnable",
    									(tsl_void_t *)&b_dhcp6senable,
    									TR69_NODE_LEAF_TYPE_BOOL);
    						}	
                        }
					}
				}
#endif

		if (ctlsock >= 0) {
			if (FD_ISSET(ctlsock, &r)) {
				(void)dhcp6_ctl_acceptcommand(ctlsock,
				    client6_do_ctlcommand);
			}
			(void)dhcp6_ctl_readcommand(&r);
		}
	}
}

static inline int
get_val32(bpp, lenp, valp)
	char **bpp;
	int *lenp;
	u_int32_t *valp;
{
	char *bp = *bpp;
	int len = *lenp;
	u_int32_t i32;

	if (len < sizeof(*valp))
		return (-1);

	memcpy(&i32, bp, sizeof(i32));
	*valp = ntohl(i32);

	*bpp = bp + sizeof(*valp);
	*lenp = len - sizeof(*valp);

	return (0);
}

static inline int
get_ifname(bpp, lenp, ifbuf, ifbuflen)
	char **bpp;
	int *lenp;
	char *ifbuf;
	int ifbuflen;
{
	char *bp = *bpp;
	int len = *lenp, ifnamelen;
	u_int32_t i32;

	if (get_val32(bpp, lenp, &i32))
		return (-1);
	ifnamelen = (int)i32;

	if (*lenp < ifnamelen || ifnamelen > ifbuflen)
		return (-1);

#if defined(AEI_COVERITY_FIX)
	memset(ifbuf, 0, ifnamelen);
#else
	memset(ifbuf, 0, sizeof(ifbuf));
#endif
	memcpy(ifbuf, *bpp, ifnamelen);
	if (ifbuf[ifbuflen - 1] != '\0')
		return (-1);	/* not null terminated */

	*bpp = bp + sizeof(i32) + ifnamelen;
	*lenp = len - (sizeof(i32) + ifnamelen);

	return (0);
}

static int
client6_do_ctlcommand(buf, len)
	char *buf;
	ssize_t len;
{
	struct dhcp6ctl *ctlhead;
	u_int16_t command, version;
	u_int32_t p32, ts, ts0;
	int commandlen;
	char *bp;
	char ifname[IFNAMSIZ];
	time_t now;

	memset(ifname, 0, sizeof(ifname));

	ctlhead = (struct dhcp6ctl *)buf;

	command = ntohs(ctlhead->command);
	commandlen = (int)(ntohs(ctlhead->len));
	version = ntohs(ctlhead->version);
	if (len != sizeof(struct dhcp6ctl) + commandlen) {
		dprintf(LOG_ERR, FNAME,
		    "assumption failure: command length mismatch");
		return (DHCP6CTL_R_FAILURE);
	}

	/* replay protection and message authentication */
	if ((now = time(NULL)) < 0) {
		dprintf(LOG_ERR, FNAME, "failed to get current time: %s",
		    strerror(errno));
		return (DHCP6CTL_R_FAILURE);
	}
	ts0 = (u_int32_t)now;
	ts = ntohl(ctlhead->timestamp);
	if (ts + CTLSKEW < ts0 || (ts - CTLSKEW) > ts0) {
		dprintf(LOG_INFO, FNAME, "timestamp is out of range");
		return (DHCP6CTL_R_FAILURE);
	}

	if (ctlkey == NULL) {	/* should not happen!! */
		dprintf(LOG_ERR, FNAME, "no secret key for control channel");
		return (DHCP6CTL_R_FAILURE);
	}
	if (dhcp6_verify_mac(buf, len, DHCP6CTL_AUTHPROTO_UNDEF,
	    DHCP6CTL_AUTHALG_HMACMD5, sizeof(*ctlhead), ctlkey) != 0) {
		dprintf(LOG_INFO, FNAME, "authentication failure");
		return (DHCP6CTL_R_FAILURE);
	}

	bp = buf + sizeof(*ctlhead) + ctldigestlen;
	commandlen -= ctldigestlen;

	if (version > DHCP6CTL_VERSION) {
		dprintf(LOG_INFO, FNAME, "unsupported version: %d", version);
		return (DHCP6CTL_R_FAILURE);
	}

	switch (command) {
	case DHCP6CTL_COMMAND_RELOAD:
		if (commandlen != 0) {
			dprintf(LOG_INFO, FNAME, "invalid command length "
			    "for reload: %d", commandlen);
			return (DHCP6CTL_R_DONE);
		}
		client6_reload();
		break;
	case DHCP6CTL_COMMAND_START:
		if (get_val32(&bp, &commandlen, &p32))
			return (DHCP6CTL_R_FAILURE);
		switch (p32) {
		case DHCP6CTL_INTERFACE:
			if (get_ifname(&bp, &commandlen, ifname,
			    sizeof(ifname))) {
				return (DHCP6CTL_R_FAILURE);
			}
			if (client6_ifctl(ifname, DHCP6CTL_COMMAND_START))
				return (DHCP6CTL_R_FAILURE);
			break;
		default:
			dprintf(LOG_INFO, FNAME,
			    "unknown start target: %ul", p32);
			return (DHCP6CTL_R_FAILURE);
		}
		break;
	case DHCP6CTL_COMMAND_STOP:
		if (commandlen == 0) {
			exit_ok = 1;
			free_resources(NULL);
			unlink(pid_file);
			check_exit();
		} else {
			if (get_val32(&bp, &commandlen, &p32))
				return (DHCP6CTL_R_FAILURE);

			switch (p32) {
			case DHCP6CTL_INTERFACE:
				if (get_ifname(&bp, &commandlen, ifname,
				    sizeof(ifname))) {
					return (DHCP6CTL_R_FAILURE);
				}
				if (client6_ifctl(ifname,
				    DHCP6CTL_COMMAND_STOP)) {
					return (DHCP6CTL_R_FAILURE);
				}
				break;
			default:
				dprintf(LOG_INFO, FNAME,
				    "unknown start target: %ul", p32);
				return (DHCP6CTL_R_FAILURE);
			}
		}
		break;
	default:
		dprintf(LOG_INFO, FNAME,
		    "unknown control command: %d (len=%d)",
		    (int)command, commandlen);
		return (DHCP6CTL_R_FAILURE);
	}

  	return (DHCP6CTL_R_DONE);
}

static void
client6_reload()
{
	/* reload the configuration file */
	if (cfparse(conffile) != 0) {
		dprintf(LOG_WARNING, FNAME,
		    "failed to reload configuration file");
		return;
	}

	dprintf(LOG_NOTICE, FNAME, "client reloaded");

	return;
}

static int
client6_ifctl(ifname, command)
	char *ifname;
	u_int16_t command;
{
	struct dhcp6_if *ifp;

	if ((ifp = find_ifconfbyname(ifname)) == NULL) {
		dprintf(LOG_INFO, FNAME,
		    "failed to find interface configuration for %s",
		    ifname);
		return (-1);
	}

	dprintf(LOG_DEBUG, FNAME, "%s interface %s",
	    command == DHCP6CTL_COMMAND_START ? "start" : "stop", ifname);

	switch(command) {
	case DHCP6CTL_COMMAND_START:
		free_resources(ifp);
		if (client6_start(ifp)) {
			dprintf(LOG_NOTICE, FNAME, "failed to restart %s",
			    ifname);
			return (-1);
		}
		break;
	case DHCP6CTL_COMMAND_STOP:
		free_resources(ifp);
		if (ifp->timer != NULL) {
			dprintf(LOG_DEBUG, FNAME,
			    "removed existing timer on %s", ifp->ifname);
			dhcp6_remove_timer(&ifp->timer);
		}
		break;
	default:		/* impossible case, should be a bug */
		dprintf(LOG_ERR, FNAME, "unknown command: %d", (int)command);
		break;
	}

	return (0);
}

static struct dhcp6_timer *
client6_expire_refreshtime(arg)
	void *arg;
{
	struct dhcp6_if *ifp = arg;

	dprintf(LOG_DEBUG, FNAME,
	    "information refresh time on %s expired", ifp->ifname);

	dhcp6_remove_timer(&ifp->timer);
	client6_start(ifp);

	return (NULL);
}
#ifdef CDROUTER_TEST_DHCP6C
static int all_ia_busy(ifp)
    struct dhcp6_if *ifp;
{
    struct ia_conf *iac;
    int tot_ia_cnt=0;
    int busy_ia_cnt=0;    
	for (iac = TAILQ_FIRST(&ifp->iaconf_list); iac;
	    iac = TAILQ_NEXT(iac, link)) {
		tot_ia_cnt++;
        
		if (!TAILQ_EMPTY(&iac->iadata))
		    {
                ctllog_debug(LOG_NOTICE,FLNAME,LINENUM,FNAME,"iac->type=%d,iac->iaid=%d, busy now",iac->type,iac->iaid);
                busy_ia_cnt++;
		    }
	     }
              ctllog_debug(LOG_ERR,FLNAME,LINENUM,FNAME, "tot_ia_cnt=%d,busy_ia_cnt=%d",tot_ia_cnt,busy_ia_cnt);   
       
     if(tot_ia_cnt && tot_ia_cnt==busy_ia_cnt)
        {
            ctllog_debug(LOG_NOTICE,FLNAME,LINENUM,FNAME,"==>all IA busy now");      
            return 1;
        }
     else
        return 0;
}
#endif

struct dhcp6_timer *
client6_timo(arg)
	void *arg;
{
	struct dhcp6_event *ev = (struct dhcp6_event *)arg;
	struct dhcp6_if *ifp;
	int state = ev->state;

	ifp = ev->ifp;
	ev->timeouts++;

	/*
	 * Unless MRC is zero, the message exchange fails once the client has
	 * transmitted the message MRC times.
	 * [RFC3315 14.]
	 */
	if (ev->max_retrans_cnt && ev->timeouts >= ev->max_retrans_cnt) {
		dprintf(LOG_INFO, FNAME, "no responses were received");
		dhcp6_remove_event(ev);

		if (state == DHCP6S_RELEASE)
			check_exit();

		return (NULL);
	}

	switch(ev->state) {
	case DHCP6S_INIT:
		ev->timeouts = 0; /* indicate to generate a new XID. */
#ifdef CONFIG_IOT_RECONFIGURATION
		if ((ifp->send_flags & DHCIFF_INFO_ONLY) || infreq_mode || infreq_reconf)
#else
		if ((ifp->send_flags & DHCIFF_INFO_ONLY) || infreq_mode)
#endif
			ev->state = DHCP6S_INFOREQ;
		else {
#ifdef CDROUTER_TEST_DHCP6C
          ctllog_debug(LOG_INFO,FLNAME,LINENUM,FNAME, "to test al ia busy,all_ia_pre_order=%d",
          all_ia_pre_order);
            if( all_ia_busy(ifp) || all_ia_pre_order==1)
            {
                ctllog_debug(LOG_INFO,FLNAME,LINENUM,FNAME,"dhcp6_remove_event");
                dhcp6_remove_event(ev);
                return (NULL);
            }
#endif
			ev->state = DHCP6S_SOLICIT;
			if (construct_confdata(ifp, ev)) {
				dprintf(LOG_ERR, FNAME, "can't send solicit");
				exit(1); /* XXX */
			}
		}
//confict with QWEST requirement,we wan to keep the old PD
//#ifdef ACTION_TEC_IPV6_CODE_FOR_IOT
	if( ev->state == DHCP6S_SOLICIT )
	    {
            if(get_dhcpv6_request_manner()==1 && send_confirm==1)
            {
				 char *wan_dhcpv6_ipv6_addr[64] = { 0 };
				 if( ctl_layer_cfg_check( ifp, (char *)wan_dhcpv6_ipv6_addr ) == 0 ) {
					 ev->state = DHCP6S_CONFIRM;
		
					 dprintf(LOG_INFO, FNAME, "wan_dhcpv6_ipv6_addr = %s\n", wan_dhcpv6_ipv6_addr);
				 }

                }
                dprintf(LOG_NOTICE, FNAME,
			    "will not send comfirm later");
                send_confirm=0;  
            }
      
//#endif

		dhcp6_set_timeoparam(ev); /* XXX */
		/* fall through */
	case DHCP6S_REQUEST:
	case DHCP6S_RELEASE:
	case DHCP6S_INFOREQ:
#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
       case DHCP6S_CONFIRM:
#endif		
		client6_send(ev);
		break;
	case DHCP6S_RENEW:
	case DHCP6S_REBIND:
		if (!TAILQ_EMPTY(&ev->data_list))
		{
#if 0 //ifdef ACTION_TEC_IPV6_CODE_FOR_IOT
			if(  DHCP6S_REBIND == ev->state )
			{
				dprintf(LOG_INFO, FNAME,"******DHCP6S_REBIND %d\n ",rmna);
				struct dhcp6_eventdata * pevt =NULL;
				for (pevt = TAILQ_FIRST(&ev->data_list); pevt; pevt = TAILQ_NEXT(pevt, link)) 
					{
						if( pevt->type == DHCP6_EVDATA_IANA && 0 == rmna)
							{
							       char *wan_dhcpv6_ipv6_addr[64] = { 0 };
							       if( ctl_layer_cfg_check( ifp, wan_dhcpv6_ipv6_addr ) == 0 ) 
							       {
									remove_na_address(ifp,wan_dhcpv6_ipv6_addr, &rmna);
									dprintf(LOG_INFO, FNAME,"*****RM_NA_Addr type[%d] ifname[%s]  wan[%s]",
										pevt->type,ifp->ifname,wan_dhcpv6_ipv6_addr);							
							       }
							}
						else if( pevt->type == DHCP6_EVDATA_IAPD && 0 == rmpd )
							{
								char *wan_dhcpv6_ipv6_addr[64] = { 0 };
							       if( ctl_layer_cfg_check( ifp, wan_dhcpv6_ipv6_addr ) == 0 ) 
							       {
									remove_na_address(ifp,wan_dhcpv6_ipv6_addr, &rmpd);
									dprintf(LOG_INFO, FNAME,"*****RM_PD_Addr type[%d] ifname[%s]  wan[%s]",
										pevt->type,ifp->ifname,wan_dhcpv6_ipv6_addr);							
							       }
							}
					}
			}
#endif		
			client6_send(ev);
		}
		else {
			dprintf(LOG_INFO, FNAME,
			    "all information to be updated was canceled");
			dhcp6_remove_event(ev);
			return (NULL);
		}
		break;
	case DHCP6S_SOLICIT:
		if (ev->servers) {
			/*
			 * Send a Request to the best server.
			 * Note that when we set Rapid-commit in Solicit,
			 * but a direct Reply has been delayed (very much),
			 * the transition to DHCP6S_REQUEST (and the change of
			 * transaction ID) will invalidate the reply even if it
			 * ever arrives.
			 */
			ev->current_server = select_server(ev);
			if (ev->current_server == NULL) {
				/* this should not happen! */
				dprintf(LOG_NOTICE, FNAME,
				    "can't find a server");
				exit(1); /* XXX */
			}
			if (duidcpy(&ev->serverid,
			    &ev->current_server->optinfo.serverID)) {
				dprintf(LOG_NOTICE, FNAME,
				    "can't copy server ID");
				return (NULL); /* XXX: better recovery? */
			}
			ev->timeouts = 0;
			ev->state = DHCP6S_REQUEST;
			dhcp6_set_timeoparam(ev);

			if (ev->authparam != NULL)
				free(ev->authparam);
			ev->authparam = ev->current_server->authparam;
			ev->current_server->authparam = NULL;

			if (construct_reqdata(ifp,
			    &ev->current_server->optinfo, ev)) {
				dprintf(LOG_NOTICE, FNAME,
				    "failed to construct request data");
				break;
			}
		}
		client6_send(ev);
		break;
	}

	dhcp6_reset_timer(ev);

#ifdef AEI_COVERITY_FIX
    if(ev)
    {	    
	    return (ev->timer);
    }
    else
    {
        return NULL;
    }
#else
    return (ev->timer);
#endif
}

static int
construct_confdata(ifp, ev)
	struct dhcp6_if *ifp;
	struct dhcp6_event *ev;
{
	struct ia_conf *iac;
	struct dhcp6_eventdata *evd = NULL;
	struct dhcp6_list *ial = NULL, pl;
	struct dhcp6_ia iaparam;
#ifdef CDROUTER_TEST_DHCP6C
    int tot_ia_cnt=0;
    int busy_ia_cnt=0;
#endif
	TAILQ_INIT(&pl);	/* for safety */

	for (iac = TAILQ_FIRST(&ifp->iaconf_list); iac;
	    iac = TAILQ_NEXT(iac, link)) {
#ifdef CDROUTER_TEST_DHCP6C
         tot_ia_cnt++;
#endif
		/* ignore IA config currently used */
		if (!TAILQ_EMPTY(&iac->iadata))
		    {
#ifdef CDROUTER_TEST_DHCP6C
        ctllog_debug(LOG_NOTICE,FLNAME,LINENUM,FNAME,"iac->type=%d,iac->iaid=%d",iac->type,iac->iaid);
		        busy_ia_cnt++;
#endif
			continue;
		    }

		evd = NULL;
		if ((evd = malloc(sizeof(*evd))) == NULL) {
			dprintf(LOG_NOTICE, FNAME,
			    "failed to create a new event data");
			goto fail;
		}
#if defined(AEI_COVERITY_FIX)
		memset(evd, 0, sizeof(*evd));
#else
		memset(evd, 0, sizeof(evd));
#endif

		memset(&iaparam, 0, sizeof(iaparam));
		iaparam.iaid = iac->iaid;
		switch (iac->type) {
		case IATYPE_PD:
			ial = NULL;
			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;
			TAILQ_INIT(ial);

			TAILQ_INIT(&pl);
			dhcp6_copy_list(&pl,
			    &((struct iapd_conf *)iac)->iapd_prefix_list);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IAPD,
			    &iaparam, &pl, 0) == NULL) {
				goto fail;
			}
			dhcp6_clear_list(&pl);

			evd->type = DHCP6_EVDATA_IAPD;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		case IATYPE_NA:
			ial = NULL;
			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;
			TAILQ_INIT(ial);

			TAILQ_INIT(&pl);
			dhcp6_copy_list(&pl,
			    &((struct iana_conf *)iac)->iana_address_list);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IANA,
			    &iaparam, &pl, 0) == NULL) {
				goto fail;
			}
			dhcp6_clear_list(&pl);

			evd->type = DHCP6_EVDATA_IANA;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		default:
			dprintf(LOG_ERR, FNAME, "internal error");
			exit(1);
		}
	}
#ifdef CDROUTER_TEST_DHCP6C
     ctllog_debug(LOG_ERR,FLNAME,LINENUM,FNAME, "tot_ia_cnt=%d,busy_ia_cnt=%d",tot_ia_cnt,busy_ia_cnt);   
    if(busy_ia_cnt==0)
        {
            all_ia_pre_order=1;
            ctllog_debug(LOG_ERR,FLNAME,LINENUM,FNAME, "set all_ia_pre_order to %d",all_ia_pre_order);
        }
#endif
	return (0);

  fail:
	if (evd)
		free(evd);
	if (ial)
		free(ial);
	dhcp6_remove_event(ev);	/* XXX */
	
	return (-1);
}

static int
construct_reqdata(ifp, optinfo, ev)
	struct dhcp6_if *ifp;
	struct dhcp6_optinfo *optinfo;
	struct dhcp6_event *ev;
{
	struct ia_conf *iac;
	struct dhcp6_eventdata *evd = NULL;
	struct dhcp6_list *ial = NULL;
	struct dhcp6_ia iaparam;

	/* discard previous event data */
	dhcp6_remove_evdata(ev);

	if (optinfo == NULL)
		return (0);

	for (iac = TAILQ_FIRST(&ifp->iaconf_list); iac;
	    iac = TAILQ_NEXT(iac, link)) {
		struct dhcp6_listval *v;

		/* ignore IA config currently used */
		if (!TAILQ_EMPTY(&iac->iadata))
			continue;

		memset(&iaparam, 0, sizeof(iaparam));
		iaparam.iaid = iac->iaid;

		ial = NULL;
		evd = NULL;

		switch (iac->type) {
		case IATYPE_PD:
			if ((v = dhcp6_find_listval(&optinfo->iapd_list,
			    DHCP6_LISTVAL_IAPD, &iaparam, 0)) == NULL)
				continue;

			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;

			TAILQ_INIT(ial);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IAPD,
			    &iaparam, &v->sublist, 0) == NULL) {
				goto fail;
			}

			if ((evd = malloc(sizeof(*evd))) == NULL)
				goto fail;
			memset(evd, 0, sizeof(*evd));
			evd->type = DHCP6_EVDATA_IAPD;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		case IATYPE_NA:
			if ((v = dhcp6_find_listval(&optinfo->iana_list,
			    DHCP6_LISTVAL_IANA, &iaparam, 0)) == NULL)
				continue;

			if ((ial = malloc(sizeof(*ial))) == NULL)
				goto fail;

			TAILQ_INIT(ial);
			if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IANA,
			    &iaparam, &v->sublist, 0) == NULL) {
				goto fail;
			}

			if ((evd = malloc(sizeof(*evd))) == NULL)
				goto fail;
			memset(evd, 0, sizeof(*evd));
			evd->type = DHCP6_EVDATA_IANA;
			evd->data = ial;
			evd->event = ev;
			evd->destructor = destruct_iadata;
			TAILQ_INSERT_TAIL(&ev->data_list, evd, link);
			break;
		default:
			dprintf(LOG_ERR, FNAME, "internal error");
			exit(1);
		}
	}

	return (0);

  fail:
	if (evd)
		free(evd);
	if (ial)
		free(ial);
	dhcp6_remove_event(ev);	/* XXX */
	
	return (-1);
}

static void
destruct_iadata(evd)
	struct dhcp6_eventdata *evd;
{
	struct dhcp6_list *ial;

	if (evd->type != DHCP6_EVDATA_IAPD && evd->type != DHCP6_EVDATA_IANA) {
		dprintf(LOG_ERR, FNAME, "assumption failure %d", evd->type);
		exit(1);
	}

	ial = (struct dhcp6_list *)evd->data;
	dhcp6_clear_list(ial);
	free(ial);
}

static struct dhcp6_serverinfo *
select_server(ev)
	struct dhcp6_event *ev;
{
	struct dhcp6_serverinfo *s;

	/*
	 * pick the best server according to RFC3315 Section 17.1.3.
	 * XXX: we currently just choose the one that is active and has the
	 * highest preference.
	 */
	for (s = ev->servers; s; s = s->next) {
		if (s->active) {
			dprintf(LOG_DEBUG, FNAME, "picked a server (ID: %s)",
			    duidstr(&s->optinfo.serverID));
			return (s);
		}
	}

	return (NULL);
}

static void
client6_signal(sig)
	int sig;
{

	switch (sig) {
	case SIGTERM:
		sig_flags |= SIGF_TERM;
		break;
	case SIGHUP:
		sig_flags |= SIGF_HUP;
		break;
	}
}

#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
static int s_stcode_confirm = -1; // Restore CONFRIM state code
#endif

void
client6_send(ev)
	struct dhcp6_event *ev;
{
	struct dhcp6_if *ifp;
	char buf[BUFSIZ];
	struct sockaddr_in6 dst;
	struct dhcp6 *dh6;
	struct dhcp6_optinfo optinfo;
	ssize_t optlen, len;
	struct dhcp6_eventdata *evd;

	ifp = ev->ifp;

	dh6 = (struct dhcp6 *)buf;
	memset(dh6, 0, sizeof(*dh6));

	switch(ev->state) {
	case DHCP6S_SOLICIT:
		dh6->dh6_msgtype = DH6_SOLICIT;
		break;
	case DHCP6S_REQUEST:
		dh6->dh6_msgtype = DH6_REQUEST;
		break;
#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
	case DHCP6S_CONFIRM:
		dh6->dh6_msgtype = DH6_CONFIRM;
		break;
    case DHCP6S_DECLINE:
        dh6->dh6_msgtype = DH6_DECLINE;
        break;		
#endif
	case DHCP6S_RENEW:
		dh6->dh6_msgtype = DH6_RENEW;
		break;
	case DHCP6S_REBIND:
		dh6->dh6_msgtype = DH6_REBIND;
		break;
	case DHCP6S_RELEASE:
		dh6->dh6_msgtype = DH6_RELEASE;
		break;
	case DHCP6S_INFOREQ:
		dh6->dh6_msgtype = DH6_INFORM_REQ;
		break;
	default:
		dprintf(LOG_ERR, FNAME, "unexpected state");
		exit(1);	/* XXX */
	}

	if (ev->timeouts == 0) {
		/*
		 * A client SHOULD generate a random number that cannot easily
		 * be guessed or predicted to use as the transaction ID for
		 * each new message it sends.
		 *
		 * A client MUST leave the transaction-ID unchanged in
		 * retransmissions of a message. [RFC3315 15.1]
		 */
#ifdef HAVE_ARC4RANDOM
		ev->xid = arc4random() & DH6_XIDMASK;
#else
		ev->xid = random() & DH6_XIDMASK;
#endif
		dprintf(LOG_DEBUG, FNAME, "a new XID (%x) is generated",
		    ev->xid);
	}
	dh6->dh6_xid &= ~ntohl(DH6_XIDMASK);
	dh6->dh6_xid |= htonl(ev->xid);
	len = sizeof(*dh6);

	/*
	 * construct options
	 */
	dhcp6_init_options(&optinfo);

	/* server ID */
	switch (ev->state) {
	case DHCP6S_REQUEST:
	case DHCP6S_RENEW:
	case DHCP6S_RELEASE:
#ifdef ACTION_TEC_IPV6_CODE_FOR_DECLINE
	case DHCP6S_DECLINE:
#endif		
		if (duidcpy(&optinfo.serverID, &ev->serverid)) {
			dprintf(LOG_ERR, FNAME, "failed to copy server ID");
			goto end;
		}
		break;
	}

	/* client ID */
	if (duidcpy(&optinfo.clientID, &client_duid)) {
		dprintf(LOG_ERR, FNAME, "failed to copy client ID");
		goto end;
	}

	/* rapid commit (in Solicit only) */
	if (ev->state == DHCP6S_SOLICIT &&
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT)) {
		optinfo.rapidcommit = 1;
	}

	/* elapsed time */
	if (ev->timeouts == 0) {
		gettimeofday(&ev->tv_start, NULL);
		optinfo.elapsed_time = 0;
	} else {
		struct timeval now, tv_diff;
		long et;

		gettimeofday(&now, NULL);
		tv_sub(&now, &ev->tv_start, &tv_diff);

		/*
		 * The client uses the value 0xffff to represent any elapsed
		 * time values greater than the largest time value that can be
		 * represented in the Elapsed Time option.
		 * [RFC3315 22.9.]
		 */
		if (tv_diff.tv_sec >= (MAX_ELAPSED_TIME / 100) + 1) {
			/*
			 * Perhaps we are nervous too much, but without this
			 * additional check, we would see an overflow in 248
			 * days (of no responses). 
			 */
			et = MAX_ELAPSED_TIME;
		} else {
			et = tv_diff.tv_sec * 100 + tv_diff.tv_usec / 10000;
			if (et >= MAX_ELAPSED_TIME)
				et = MAX_ELAPSED_TIME;
		}
		optinfo.elapsed_time = (int32_t)et;
	}

	/* option request options */
	if (ev->state != DHCP6S_RELEASE &&
#ifdef ACTION_TEC_IPV6_CODE_FOR_DECLINE
	    ev->state != DHCP6S_DECLINE &&
#endif		
	    dhcp6_copy_list(&optinfo.reqopt_list, &ifp->reqopt_list)) {
		dprintf(LOG_ERR, FNAME, "failed to copy requested options");
		goto end;
	}

	/* configuration information specified as event data */
	for (evd = TAILQ_FIRST(&ev->data_list); evd;
	     evd = TAILQ_NEXT(evd, link)) {
		switch(evd->type) {
		case DHCP6_EVDATA_IAPD:
#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
						// CONFIRM cannot send IA_PD @ RFC3633 12.1
						if ( ev->state == DHCP6S_CONFIRM )
							break;
#endif
			if (dhcp6_copy_list(&optinfo.iapd_list,
			    (struct dhcp6_list *)evd->data)) {
				dprintf(LOG_NOTICE, FNAME,
				    "failed to add an IAPD");
				goto end;
			}
#ifdef AEI_RENEW_OLD_PD
            if(get_dhcpv6_request_manner()==0)
            {
			if(iReq==1 && dh6->dh6_msgtype == DH6_REQUEST)
			{
				iReq=2;
				dh6->dh6_msgtype = DH6_RENEW;
			}			
            }
#endif			
			break;
		case DHCP6_EVDATA_IANA:
#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
            // After CONFIRM, Only SOLICIT for PD, not for NA
            if ( ev->state == DHCP6S_SOLICIT && s_stcode_confirm == DH6OPT_STCODE_SUCCESS )
                break;
#endif
			if (dhcp6_copy_list(&optinfo.iana_list,
			    (struct dhcp6_list *)evd->data)) {
				dprintf(LOG_NOTICE, FNAME,
				    "failed to add an IAPD");
				goto end;
			}
			break;
		default:
			dprintf(LOG_ERR, FNAME, "unexpected event data (%d)",
			    evd->type);
			exit(1);
		}
	}

	/* authentication information */
	if (set_auth(ev, &optinfo)) {
		dprintf(LOG_INFO, FNAME,
		    "failed to set authentication option");
		goto end;
	}

	/* set options in the message */
	if ((optlen = dhcp6_set_options(dh6->dh6_msgtype,
	    (struct dhcp6opt *)(dh6 + 1),
	    (struct dhcp6opt *)(buf + sizeof(buf)), &optinfo)) < 0) {
		dprintf(LOG_INFO, FNAME, "failed to construct options");
		goto end;
	}
	len += optlen;

	/* calculate MAC if necessary, and put it to the message */
	if (ev->authparam != NULL) {
		switch (ev->authparam->authproto) {
		case DHCP6_AUTHPROTO_DELAYED:
			if (ev->authparam->key == NULL)
				break;

			if (dhcp6_calc_mac((char *)dh6, len,
			    optinfo.authproto, optinfo.authalgorithm,
			    optinfo.delayedauth_offset + sizeof(*dh6),
			    ev->authparam->key)) {
				dprintf(LOG_WARNING, FNAME,
				    "failed to calculate MAC");
				goto end;
			}
			break;
		default:
			break;	/* do nothing */
		}
	}

	/*
	 * Unless otherwise specified in this document or in a document that
	 * describes how IPv6 is carried over a specific type of link (for link
	 * types that do not support multicast), a client sends DHCP messages
	 * to the All_DHCP_Relay_Agents_and_Servers.
	 * [RFC3315 Section 13.]
	 */
	dst = *sa6_allagent;
	dst.sin6_scope_id = ifp->linkid;

	if (sendto(sock, buf, len, 0, (struct sockaddr *)&dst,
	    sysdep_sa_len((struct sockaddr *)&dst)) == -1) {
		dprintf(LOG_ERR, FNAME,
		    "transmit failed: %s", strerror(errno));
		goto end;
	}

	dprintf(LOG_DEBUG, FNAME, "send %s to %s",
	    dhcp6msgstr(dh6->dh6_msgtype), addr2str((struct sockaddr *)&dst));

  end:
	dhcp6_clear_options(&optinfo);
	return;
}

/* result will be a - b */
static void
tv_sub(a, b, result)
	struct timeval *a, *b, *result;
{
	if (a->tv_sec < b->tv_sec ||
	    (a->tv_sec == b->tv_sec && a->tv_usec < b->tv_usec)) {
		result->tv_sec = 0;
		result->tv_usec = 0;

		return;
	}

	result->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_usec < b->tv_usec) {
		result->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
		result->tv_sec -= 1;
	} else
		result->tv_usec = a->tv_usec - b->tv_usec;

	return;
}

static void
client6_recv()
{
   ctllog_debug(LOG_ERR,FLNAME,LINENUM,FNAME, "<<===========recv msg, BUFSIZ=%d",BUFSIZ);
#ifdef CDROUTER_TEST_DHCP6C
    char rbuf[BUFSIZ*2];
#else
    char rbuf[BUFSIZ];
#endif
	char cmsgbuf[BUFSIZ];
	struct msghdr mhdr;
	struct iovec iov;
	struct sockaddr_storage from;
	struct dhcp6_if *ifp;
	struct dhcp6opt *p, *ep;
	struct dhcp6_optinfo optinfo;
	ssize_t len;
	struct dhcp6 *dh6;
	struct cmsghdr *cm;
	struct in6_pktinfo *pi = NULL;

	memset(&iov, 0, sizeof(iov));
	memset(&mhdr, 0, sizeof(mhdr));

	iov.iov_base = (caddr_t)rbuf;
	iov.iov_len = sizeof(rbuf);
	mhdr.msg_name = (caddr_t)&from;
	mhdr.msg_namelen = sizeof(from);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (caddr_t)cmsgbuf;
	mhdr.msg_controllen = sizeof(cmsgbuf);
	if ((len = recvmsg(sock, &mhdr, 0)) < 0) {
		dprintf(LOG_ERR, FNAME, "recvmsg: %s", strerror(errno));
		return;
	}
    ctllog_debug(LOG_ERR,FLNAME,LINENUM,FNAME, "msg length=%d",len );
	/* detect receiving interface */
	for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;
	     cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm)) {
		if (cm->cmsg_level == IPPROTO_IPV6 &&
		    cm->cmsg_type == IPV6_PKTINFO &&
		    cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))) {
			pi = (struct in6_pktinfo *)(CMSG_DATA(cm));
		}
	}
	if (pi == NULL) {
		dprintf(LOG_NOTICE, FNAME, "failed to get packet info");
		return;
	}

	if ((ifp = find_ifconfbyid((unsigned int)pi->ipi6_ifindex)) == NULL) {
		dprintf(LOG_INFO, FNAME, "unexpected interface (%d)",
		    (unsigned int)pi->ipi6_ifindex);
		return;
	}

	if (len < sizeof(*dh6)) {
		dprintf(LOG_INFO, FNAME, "short packet (%d bytes)", len);
		return;
	}

	dh6 = (struct dhcp6 *)rbuf;

	dprintf(LOG_DEBUG, FNAME, "receive %s from %s on %s",
	    dhcp6msgstr(dh6->dh6_msgtype),
	    addr2str((struct sockaddr *)&from), ifp->ifname);

	/* get options */
	dhcp6_init_options(&optinfo);
	p = (struct dhcp6opt *)(dh6 + 1);
	ep = (struct dhcp6opt *)((char *)dh6 + len);
	if (dhcp6_get_options(p, ep, &optinfo) < 0) {
		dprintf(LOG_INFO, FNAME, "failed to parse options");
		return;
	}

	switch(dh6->dh6_msgtype) {
	case DH6_ADVERTISE:
#ifdef AEI_CONTROL_LAYER
		ctldhcp6cMsgBody.defaultGWAssigned = TSL_B_TRUE;
		strcpy(ctldhcp6cMsgBody.defaultGW, addr2str((struct sockaddr *)&from));
#endif
		(void)client6_recvadvert(ifp, dh6, len, &optinfo);
		break;
	case DH6_REPLY:
		(void)client6_recvreply(ifp, dh6, len, &optinfo);
		break;
#ifdef CONFIG_IOT_RECONFIGURATION
	case DH6_RECONFIGURE:
		(void)client6_reconfigure(ifp,dh6,len, &optinfo);
		break;
#endif
	default:
		dprintf(LOG_INFO, FNAME, "received an unexpected message (%s) "
		    "from %s", dhcp6msgstr(dh6->dh6_msgtype),
		    addr2str((struct sockaddr *)&from));
		break;
	}

	dhcp6_clear_options(&optinfo);
	return;
}

static int
client6_recvadvert(ifp, dh6, len, optinfo)
	struct dhcp6_if *ifp;
	struct dhcp6 *dh6;
	ssize_t len;
	struct dhcp6_optinfo *optinfo;
{
	struct dhcp6_serverinfo *newserver, **sp;
	struct dhcp6_event *ev;
	struct dhcp6_eventdata *evd;
	struct authparam *authparam = NULL, authparam0;

	/* find the corresponding event based on the received xid */
	ev = find_event_withid(ifp, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
	if (ev == NULL) {
		dprintf(LOG_INFO, FNAME, "XID mismatch");
		return (-1);
	}

	/* packet validation based on Section 15.3 of RFC3315. */
	if (optinfo->serverID.duid_len == 0) {
		dprintf(LOG_INFO, FNAME, "no server ID option");
		return (-1);
	} else {
		dprintf(LOG_DEBUG, FNAME, "server ID: %s, pref=%d",
		    duidstr(&optinfo->serverID),
		    optinfo->pref);
	}
	if (optinfo->clientID.duid_len == 0) {
		dprintf(LOG_INFO, FNAME, "no client ID option");
		return (-1);
	}
	if (duidcmp(&optinfo->clientID, &client_duid)) {
		dprintf(LOG_INFO, FNAME, "client DUID mismatch");
		return (-1);
	}

	/* validate authentication */
	authparam0 = *ev->authparam;
	if (process_auth(&authparam0, dh6, len, optinfo)) {
		dprintf(LOG_INFO, FNAME, "failed to process authentication");
		return (-1);
	}

	/*
	 * The requesting router MUST ignore any Advertise message that
	 * includes a Status Code option containing the value NoPrefixAvail
	 * [RFC3633 Section 11.1].
	 * Likewise, the client MUST ignore any Advertise message that includes
	 * a Status Code option containing the value NoAddrsAvail. 
	 * [RFC3315 Section 17.1.3].
	 * We only apply this when we are going to request an address or
	 * a prefix.
	 */
	for (evd = TAILQ_FIRST(&ev->data_list); evd;
	    evd = TAILQ_NEXT(evd, link)) {
		u_int16_t stcode;
		char *stcodestr;

		switch (evd->type) {
		case DHCP6_EVDATA_IAPD:
			stcode = DH6OPT_STCODE_NOPREFIXAVAIL;
			stcodestr = "NoPrefixAvail";
			break;
		case DHCP6_EVDATA_IANA:
			stcode = DH6OPT_STCODE_NOADDRSAVAIL;
			stcodestr = "NoAddrsAvail";
			break;
		default:
			continue;
		}
		if (dhcp6_find_listval(&optinfo->stcode_list,
		    DHCP6_LISTVAL_STCODE, &stcode, 0)) {
			dprintf(LOG_INFO, FNAME,
			    "advertise contains %s status", stcodestr);
			return (-1);
		}
	}

	if (ev->state != DHCP6S_SOLICIT ||
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT) || infreq_mode) {
		/*
		 * We expected a reply message, but do actually receive an
		 * Advertise message.  The server should be configured not to
		 * allow the Rapid Commit option.
		 * We process the message as if we expected the Advertise.
		 * [RFC3315 Section 17.1.4]
		 */
		dprintf(LOG_INFO, FNAME, "unexpected advertise");
		/* proceed anyway */
	}

	/* ignore the server if it is known */
	if (find_server(ev, &optinfo->serverID)) {
		dprintf(LOG_INFO, FNAME, "duplicated server (ID: %s)",
		    duidstr(&optinfo->serverID));
		return (-1);
	}

	/* keep the server */
	if ((newserver = malloc(sizeof(*newserver))) == NULL) {
		dprintf(LOG_WARNING, FNAME,
		    "memory allocation failed for server");
		return (-1);
	}
	memset(newserver, 0, sizeof(*newserver));

	/* remember authentication parameters */
	newserver->authparam = ev->authparam;
	newserver->authparam->flags = authparam0.flags;
	newserver->authparam->prevrd = authparam0.prevrd;
	newserver->authparam->key = authparam0.key;

	/* allocate new authentication parameter for the soliciting event */
	if ((authparam = new_authparam(ev->authparam->authproto,
	    ev->authparam->authalgorithm, ev->authparam->authrdm)) == NULL) {
		dprintf(LOG_WARNING, FNAME, "memory allocation failed "
		    "for authentication parameters");
		free(newserver);
		return (-1);
	}
	ev->authparam = authparam;

	/* copy options */
	dhcp6_init_options(&newserver->optinfo);
	if (dhcp6_copy_options(&newserver->optinfo, optinfo)) {
		dprintf(LOG_ERR, FNAME, "failed to copy options");
		if (newserver->authparam != NULL)
			free(newserver->authparam);
		free(newserver);
		return (-1);
	}
	if (optinfo->pref != DH6OPT_PREF_UNDEF)
		newserver->pref = optinfo->pref;
	newserver->active = 1;
	for (sp = &ev->servers; *sp; sp = &(*sp)->next) {
		if ((*sp)->pref != DH6OPT_PREF_MAX &&
		    (*sp)->pref < newserver->pref) {
			break;
		}
	}
	newserver->next = *sp;
	*sp = newserver;

	if (newserver->pref == DH6OPT_PREF_MAX) {
		/*
		 * If the client receives an Advertise message that includes a
		 * Preference option with a preference value of 255, the client
		 * immediately begins a client-initiated message exchange.
		 * [RFC3315 Section 17.1.2]
		 */
		ev->current_server = newserver;
		if (duidcpy(&ev->serverid,
		    &ev->current_server->optinfo.serverID)) {
			dprintf(LOG_NOTICE, FNAME, "can't copy server ID");
			return (-1); /* XXX: better recovery? */
		}
		if (construct_reqdata(ifp, &ev->current_server->optinfo, ev)) {
			dprintf(LOG_NOTICE, FNAME,
			    "failed to construct request data");
			return (-1); /* XXX */
		}

		ev->timeouts = 0;
		ev->state = DHCP6S_REQUEST;

		free(ev->authparam);
		ev->authparam = newserver->authparam;
		newserver->authparam = NULL;

		client6_send(ev);

		dhcp6_set_timeoparam(ev);
		dhcp6_reset_timer(ev);
	} else if (ev->servers->next == NULL) {
		struct timeval *rest, elapsed, tv_rt, tv_irt, timo;

		/*
		 * If this is the first advertise, adjust the timer so that
		 * the client can collect other servers until IRT elapses.
		 * XXX: we did not want to do such "low level" timer
		 *      calculation here.
		 */
		rest = dhcp6_timer_rest(ev->timer);
		tv_rt.tv_sec = (ev->retrans * 1000) / 1000000;
		tv_rt.tv_usec = (ev->retrans * 1000) % 1000000;
		tv_irt.tv_sec = (ev->init_retrans * 1000) / 1000000;
		tv_irt.tv_usec = (ev->init_retrans * 1000) % 1000000;
		timeval_sub(&tv_rt, rest, &elapsed);
		if (TIMEVAL_LEQ(elapsed, tv_irt))
			timeval_sub(&tv_irt, &elapsed, &timo);
		else
			timo.tv_sec = timo.tv_usec = 0;

		dprintf(LOG_DEBUG, FNAME, "reset timer for %s to %d.%06d",
		    ifp->ifname, (int)timo.tv_sec, (int)timo.tv_usec);

		dhcp6_set_timer(&timo, ev->timer);
	}

	return (0);
}

static struct dhcp6_serverinfo *
find_server(ev, duid)
	struct dhcp6_event *ev;
	struct duid *duid;
{
	struct dhcp6_serverinfo *s;

	for (s = ev->servers; s; s = s->next) {
		if (duidcmp(&s->optinfo.serverID, duid) == 0)
			return (s);
	}

	return (NULL);
}

static int
client6_recvreply(ifp, dh6, len, optinfo)
	struct dhcp6_if *ifp;
	struct dhcp6 *dh6;
	ssize_t len;
	struct dhcp6_optinfo *optinfo;
{
	struct dhcp6_listval *lv;
	struct dhcp6_event *ev;
	int state;

	/* find the corresponding event based on the received xid */
	ev = find_event_withid(ifp, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
	if (ev == NULL) {
		dprintf(LOG_INFO, FNAME, "XID mismatch");
		return (-1);
	}

	state = ev->state;
	if (state != DHCP6S_INFOREQ &&
	    state != DHCP6S_REQUEST &&
#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
           state != DHCP6S_CONFIRM &&
#endif	    
	    state != DHCP6S_RENEW &&
	    state != DHCP6S_REBIND &&
	    state != DHCP6S_RELEASE &&
#ifdef ACTION_TEC_IPV6_CODE_FOR_IOT
		(state != DHCP6S_SOLICIT)) {
#else
		(state != DHCP6S_SOLICIT || !(ifp->send_flags & DHCIFF_RAPID_COMMIT))) {
#endif
		dprintf(LOG_INFO, FNAME, "unexpected reply");
		return (-1);
	}

	/* A Reply message must contain a Server ID option */
	if (optinfo->serverID.duid_len == 0) {
		dprintf(LOG_INFO, FNAME, "no server ID option");
		return (-1);
	}

	/*
	 * DUID in the Client ID option (which must be contained for our
	 * client implementation) must match ours.
	 */
	if (optinfo->clientID.duid_len == 0) {
		dprintf(LOG_INFO, FNAME, "no client ID option");
		return (-1);
	}
	if (duidcmp(&optinfo->clientID, &client_duid)) {
		dprintf(LOG_INFO, FNAME, "client DUID mismatch");
		return (-1);
	}

	/* validate authentication */
	if (process_auth(ev->authparam, dh6, len, optinfo)) {
		dprintf(LOG_INFO, FNAME, "failed to process authentication");
		return (-1);
	}

	/*
	 * If the client included a Rapid Commit option in the Solicit message,
	 * the client discards any Reply messages it receives that do not
	 * include a Rapid Commit option.
	 * (should we keep the server otherwise?)
	 * [RFC3315 Section 17.1.4]
	 */
	if (state == DHCP6S_SOLICIT &&
	    (ifp->send_flags & DHCIFF_RAPID_COMMIT) &&
	    !optinfo->rapidcommit) {
		dprintf(LOG_INFO, FNAME, "no rapid commit");
		return (-1);
	}

	/*
	 * The client MAY choose to report any status code or message from the
	 * status code option in the Reply message.
	 * [RFC3315 Section 18.1.8]
	 */

	unsigned short stcode = 0;

	for (lv = TAILQ_FIRST(&optinfo->stcode_list); lv;
	     lv = TAILQ_NEXT(lv, link)) {
#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
		 stcode = lv->val_num16;
#endif
		dprintf(LOG_INFO, FNAME, "status code: %s",
		    dhcp6_stcodestr(lv->val_num16));
	}

	if (!TAILQ_EMPTY(&optinfo->dns_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->dns_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("nameserver[%d] %s",
			    i, in6addr2str(&d->val_addr6, 0));
		}

#if 1 //brcm
      updateDhcp6sConfDnsList(optinfo);
#endif
#ifdef CONFIG_IOT_RECONFIGURATION
	if( state ==  DHCP6S_INFOREQ && infreq_reconf )
		infreq_reconf = 0;
#endif
	}

	if (!TAILQ_EMPTY(&optinfo->dnsname_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->dnsname_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("Domain search list[%d] %s",
			    i, d->val_vbuf.dv_buf);
#ifdef ACTION_TEC_IPV6_CODE_FOR_IOT                                             
			  if( i == 0 )
			  {
						 snprintf(ctldhcp6cMsgBody.namesearchlist, BUFLEN_128-1,"%s",d->val_vbuf.dv_buf);
				  //remove last dot
				   int nLen = strlen(ctldhcp6cMsgBody.namesearchlist);
				   if( nLen > 0 )
				   {
					  ctldhcp6cMsgBody.namesearchlist[nLen-1] = '\0';	 
				   }
					  dprintf(LOG_INFO, FNAME, "***DNS from dhcpv6 search list %s\n",ctldhcp6cMsgBody.namesearchlist);	  
			  }
#endif

		}
#ifdef CONFIG_IOT_RECONFIGURATION
		if( state ==  DHCP6S_INFOREQ && infreq_reconf )
			infreq_reconf = 0;
#endif	             
	}

	if (!TAILQ_EMPTY(&optinfo->ntp_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->ntp_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("NTP server[%d] %s",
			    i, in6addr2str(&d->val_addr6, 0));
		}
	}

	if (!TAILQ_EMPTY(&optinfo->sip_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->sip_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("SIP server address[%d] %s",
			    i, in6addr2str(&d->val_addr6, 0));
		}
	}

	if (!TAILQ_EMPTY(&optinfo->sipname_list)) {
		struct dhcp6_listval *d;
		int i = 0;

		for (d = TAILQ_FIRST(&optinfo->sipname_list); d;
		     d = TAILQ_NEXT(d, link), i++) {
			info_printf("SIP domain name[%d] %s",
			    i, d->val_vbuf.dv_buf);
		}
	}

	/*
	 * Call the configuration script, if specified, to handle various
	 * configuration parameters.
	 */
	if (ifp->scriptpath != NULL && strlen(ifp->scriptpath) != 0) {
		dprintf(LOG_DEBUG, FNAME, "executes %s", ifp->scriptpath);
		client6_script(ifp->scriptpath, state, optinfo);
	}

	/*
	 * Set refresh timer for configuration information specified in
	 * information-request.  If the timer value is specified by the server
	 * in an information refresh time option, use it; use the protocol
	 * default otherwise.
	 */
	if (state == DHCP6S_INFOREQ) {
		int64_t refreshtime = DHCP6_IRT_DEFAULT;

		if (optinfo->refreshtime != DH6OPT_REFRESHTIME_UNDEF)
			refreshtime = optinfo->refreshtime;

		ifp->timer = dhcp6_add_timer(client6_expire_refreshtime, ifp);
		if (ifp->timer == NULL) {
			dprintf(LOG_WARNING, FNAME,
			    "failed to add timer for refresh time");
		} else {
			struct timeval tv;

			tv.tv_sec = (long)refreshtime;
			tv.tv_usec = 0;

			if (tv.tv_sec < 0) {
				/*
				 * XXX: tv_sec can overflow for an
				 * unsigned 32bit value.
				 */
				dprintf(LOG_WARNING, FNAME,
				    "refresh time is too large: %lu",
				    (u_int32_t)refreshtime);
				tv.tv_sec = 0x7fffffff;	/* XXX */
			}

			dhcp6_set_timer(&tv, ifp->timer);
		}
	} else if (optinfo->refreshtime != DH6OPT_REFRESHTIME_UNDEF) {
		/*
		 * draft-ietf-dhc-lifetime-02 clarifies that refresh time
		 * is only used for information-request and reply exchanges.
		 */
		dprintf(LOG_INFO, FNAME,
		    "unexpected information refresh time option (ignored)");
	}

	/* update stateful configuration information */
	if (state != DHCP6S_RELEASE) {
		update_ia(IATYPE_PD, &optinfo->iapd_list, ifp,
		    &optinfo->serverID, ev->authparam);
		update_ia(IATYPE_NA, &optinfo->iana_list, ifp,
		    &optinfo->serverID, ev->authparam);
	}

//brcm
//Now, we should finish updating prefix, addr, and dns info
//send the message to smd now.
	if (memcmp(&ctldhcp6cMsgBody, &ctldhcp6cMsgBody_Old, sizeof(CtlDhcp6cStateChangedMsgBody))) {
		sendDhcp6cEventMessage();
		memcpy(&ctldhcp6cMsgBody_Old, &ctldhcp6cMsgBody, sizeof(CtlDhcp6cStateChangedMsgBody));
	}

#ifdef ACTION_TEC_IPV6_CODE_FOR_CONFIRM
    // sam @ 2011-05-12 11:22:40, Thu
    // for In one case we were able to CONFIM our NA but Cisco replied to CONFIRM for PD with "not on link" error;
    // we did not recover.
    // We should SOLICIT for PD per RFC 3315 section 18.1.8 and 20.1.1.
    if(  DHCP6S_CONFIRM == ev->state ) {
        s_stcode_confirm = stcode;

        ev->state = DHCP6S_SOLICIT;

        ev->timeouts = -1;  // make sure: timeouts ++ = 0
        // dhcp6_set_timeoparam(ev);
        // dhcp6_reset_timer(ev);

        dprintf(LOG_DEBUG, FNAME, "got an expected REPLY for CONFRIM, stcode=%s, then send SOLICIT ...\n", dhcp6_stcodestr(s_stcode_confirm) );

        return (0);
    }
#endif

	dhcp6_remove_event(ev);

#ifdef ACTION_TEC_IPV6_CODE_FOR_DECLINE
    if(ev)
    {
		if(  DHCP6S_REBIND == ev->state )
		{
			dprintf(LOG_INFO, FNAME,"&&&&&&&&DHCP6C_REBIND restore rm NA address flag1\n ");
			struct dhcp6_eventdata * pevt =NULL;
			for (pevt = TAILQ_FIRST(&ev->data_list); pevt; pevt = TAILQ_NEXT(pevt, link)) 
				{
					if( pevt->type == DHCP6_EVDATA_IANA &&	1 == rmna)
						{
							rmna = 0;
							dprintf(LOG_INFO, FNAME,"&&&&&&&&DHCP6C_REBIND restore rm NA address flag2\n ");
						}
				}
		}
    }
#endif

	if (state == DHCP6S_RELEASE) {
		/*
		 * When the client receives a valid Reply message in response
		 * to a Release message, the client considers the Release event
		 * completed, regardless of the Status Code option(s) returned
		 * by the server.
		 * [RFC3315 Section 18.1.8]
		 */
		check_exit();
	}

	dprintf(LOG_DEBUG, FNAME, "got an expected reply, sleeping.");

	if (infreq_mode) {
		exit_ok = 1;
		free_resources(NULL);
		unlink(pid_file);
		check_exit();
	}
	return (0);
}
#ifdef CONFIG_IOT_RECONFIGURATION
static int
client6_reconfigure_info(ifp)
        struct dhcp6_if *ifp;
{
        infreq_reconf = 1;
        return client6_start(ifp);
}

static int
client6_reconfigure(ifp, dh6, len, optinfo)
	struct dhcp6_if *ifp;
	struct dhcp6 *dh6;
	ssize_t len;
	struct dhcp6_optinfo *optinfo;
{
	struct ia *ia;


       /* RFC 3315, 19.4.1. 
        * The client ignores the transaction-id field in the received Reconfigure
        * message.  
        */ 
       dprintf(LOG_DEBUG, FNAME, "XID should be zero.(%d)\n",ntohl(dh6->dh6_xid));

	   /* A Reconfigure message must contain a Server ID option */
       if (optinfo->serverID.duid_len == 0) {
               dprintf(LOG_INFO, FNAME, "no server ID option");
               return (-1);
       }


       /*
        * DUID in the Client ID option (which must be contained for our
        * client implementation) must match ours.
        */
       if (optinfo->clientID.duid_len == 0) {
               dprintf(LOG_INFO, FNAME, "no client ID option");
               return (-1);
       }
       if (duidcmp(&optinfo->clientID, &client_duid)) {
               dprintf(LOG_INFO, FNAME, "client DUID mismatch");
               return (-1);
       }
    	/* validate authentication */
    	if (process_auth(&g_auth_param, dh6, len, optinfo)) {
    		ctllog_debug(LOG_INFO,FLNAME,LINENUM,FNAME, "failed to process authentication");
    		return (-1);
    	}  
        if( optinfo->reconf_type == RECONF_TYPE_INFO)
        {
                client6_reconfigure_info(ifp);
        }
        else if( optinfo->reconf_type == RECONF_TYPE_RENEW)
            renew_when_reconfig(ifp,&optinfo->serverID);
        else if( optinfo->reconf_type == RECONF_TYPE_REBIND)
        {
            ctllog_debug(LOG_INFO,FLNAME,LINENUM,FNAME, "reconf_type %d unsupport now!!!",optinfo->reconf_type);
        }
        else
            ctllog_debug(LOG_INFO,FLNAME,LINENUM,FNAME, "reconf_type %d is not define in RFC!!!",optinfo->reconf_type);


       if (infreq_mode) {
               exit_ok = 1;
               free_resources(NULL);
               unlink(pid_file);
               check_exit();
       }
       return (0);
}

static struct dhcp6_event *
find_event_withoutid(ifp)
       struct dhcp6_if *ifp;
{
       struct dhcp6_event *ev;

       for (ev = TAILQ_FIRST(&ifp->event_list); ev;
            ev = TAILQ_NEXT(ev, link)) {
               return (ev);
       }

       return (NULL);
}
#endif

static struct dhcp6_event *
find_event_withid(ifp, xid)
	struct dhcp6_if *ifp;
	u_int32_t xid;
{
	struct dhcp6_event *ev;

	for (ev = TAILQ_FIRST(&ifp->event_list); ev;
	     ev = TAILQ_NEXT(ev, link)) {
		if (ev->xid == xid)
			return (ev);
	}

	return (NULL);
}

static int
process_auth(authparam, dh6, len, optinfo)
	struct authparam *authparam;
	struct dhcp6 *dh6;
	ssize_t len;
	struct dhcp6_optinfo *optinfo;
{
	struct keyinfo *key = NULL;
	int authenticated = 0;

	switch (optinfo->authproto) {
	case DHCP6_AUTHPROTO_UNDEF:
		/* server did not provide authentication option */
		break;
	case DHCP6_AUTHPROTO_DELAYED:
		if ((optinfo->authflags & DHCP6OPT_AUTHFLAG_NOINFO)) {
			dprintf(LOG_INFO, FNAME, "server did not include "
			    "authentication information");
			break;
		}

		if (optinfo->authalgorithm != DHCP6_AUTHALG_HMACMD5) {
			dprintf(LOG_INFO, FNAME, "unknown authentication "
			    "algorithm (%d)", optinfo->authalgorithm);
			break;
		}

		if (optinfo->authrdm != DHCP6_AUTHRDM_MONOCOUNTER) {
			dprintf(LOG_INFO, FNAME,"unknown RDM (%d)",
			    optinfo->authrdm);
			break;
		}

		/*
		 * Replay protection.  If we do not know the previous RD value,
		 * we accept the message anyway (XXX).
		 */
		if ((authparam->flags & AUTHPARAM_FLAGS_NOPREVRD)) {
			dprintf(LOG_WARNING, FNAME, "previous RD value is "
			    "unknown (accept it)");
		} else {
			if (dhcp6_auth_replaycheck(optinfo->authrdm,
			    authparam->prevrd, optinfo->authrd)) {
				dprintf(LOG_INFO, FNAME,
				    "possible replay attack detected");
				break;
			}
		}

		/* identify the secret key */
		if ((key = authparam->key) != NULL) {
			/*
			 * If we already know a key, its identification should
			 * match that contained in the received option.
			 * (from Section 21.4.5.1 of RFC3315)
			 */
			if (optinfo->delayedauth_keyid != key->keyid ||
			    optinfo->delayedauth_realmlen != key->realmlen ||
			    memcmp(optinfo->delayedauth_realmval, key->realm,
			    key->realmlen) != 0) {
				dprintf(LOG_INFO, FNAME,
				    "authentication key mismatch");
				break;
			}
		} else {
			key = find_key(optinfo->delayedauth_realmval,
			    optinfo->delayedauth_realmlen,
			    optinfo->delayedauth_keyid);
			if (key == NULL) {
				dprintf(LOG_INFO, FNAME, "failed to find key "
				    "provided by the server (ID: %x)",
				    optinfo->delayedauth_keyid);
				break;
			} else {
				dprintf(LOG_DEBUG, FNAME, "found key for "
				    "authentication: %s", key->name);
			}
			authparam->key = key;
		}

		/* check for the key lifetime */
		if (dhcp6_validate_key(key)) {
			dprintf(LOG_INFO, FNAME, "key %s has expired",
			    key->name);
			break;
		}

		/* validate MAC */
		if (dhcp6_verify_mac((char *)dh6, len, optinfo->authproto,
		    optinfo->authalgorithm,
		    optinfo->delayedauth_offset + sizeof(*dh6), key) == 0) {
			dprintf(LOG_DEBUG, FNAME, "message authentication "
			    "validated");
			authenticated = 1;
		} else {
			dprintf(LOG_INFO, FNAME, "invalid message "
			    "authentication");
		}

		break;
#ifdef CONFIG_IOT_RECONFIGURATION
		case DHCP6_AUTHPROTO_RECONFIG:
				if (optinfo->authflags == DHCP6OPT_AUTHFLAG_NOINFO) {
						dprintf(LOG_INFO, FNAME, "No INFO for RECONFIGURE KEY");
						break;
				}

				dprintf(LOG_INFO, FNAME, "Need check this "
					"authentication protocol (%d)- AUTHPROTO_RECONFIG, "
						"Algorithm is %d,RDM is %d",
										optinfo->authproto,optinfo->authalgorithm,optinfo->authrdm);
		 if (optinfo->authalgorithm != DHCP6_AUTHALG_HMACMD5) {
			 dprintf(LOG_INFO, FNAME, "unknown authentication "
					 "algorithm (%d)", optinfo->authalgorithm);
			 break;
		 }

		 if (optinfo->authrdm != DHCP6_AUTHRDM_MONOCOUNTER) {
			 dprintf(LOG_INFO, FNAME,"unknown RDM (%d)",
					 optinfo->authrdm);
			 break;
		 }
		 /*
		 * Replay protection.  If we do not know the previous RD value,
		 * we accept the message anyway (XXX).
		 */
		 if ((authparam->flags & AUTHPARAM_FLAGS_NOPREVRD)) {
			 dprintf(LOG_WARNING, FNAME, "previous RD value is "
					 "unknown (accept it)");
		 }
		 else {
			 if (dhcp6_auth_replaycheck(optinfo->authrdm,
										authparam->prevrd, optinfo->authrd)) {
				 dprintf(LOG_INFO, FNAME,
						 "possible replay attack detected");
				 break;
			 }
		 }

				switch(optinfo->reconfigauth_type ) {
				case DHCP6_RECONF_AUTH_KEY:
						/*Got the key*/
						memcpy(authparam->reconf_key,optinfo->reconfigauth_val,DHCP6_RECONFIGURE_AUTH_KEY_LENGTH);
						authparam->reconf_key_type = optinfo->authalgorithm;
                        memcpy(g_auth_param.reconf_key,optinfo->reconfigauth_val,DHCP6_RECONFIGURE_AUTH_KEY_LENGTH);
						g_auth_param.reconf_key_type = optinfo->authalgorithm; 
						authenticated = 1; 
						dprintf(LOG_INFO, FNAME,
										"GOT KEY");
			 break;
		 case DHCP6_RECONF_AUTH_INFO:
			 /*Need authentication*/
						if (authparam->reconf_key_type != optinfo->authalgorithm) {
								dprintf(LOG_INFO, FNAME,
						 "KEY type and auth algorithm mismatched");
						}

						/* validate MAC */
			 if (dhcp6_reconf_verify_mac((char *)dh6, len, optinfo->authproto,
								  optinfo->authalgorithm,
								  optinfo->reconfigauth_offset + sizeof(*dh6),
																 authparam->reconf_key ,DHCP6_RECONFIGURE_AUTH_KEY_LENGTH) == 0) {
				 dprintf(LOG_DEBUG, FNAME, "message authentication "
						 "validated");
				 authenticated = 1;
			 }
			 else {
				 dprintf(LOG_INFO, FNAME, "invalid message "
						 "authentication");
			 }
						break;
				default:
						break;
				}

				break;
#endif
	default:
		dprintf(LOG_INFO, FNAME, "server sent unsupported "
		    "authentication protocol (%d)", optinfo->authproto);
		break;
	}

	if (authenticated == 0) {
		if (authparam->authproto != DHCP6_AUTHPROTO_UNDEF) {
			dprintf(LOG_INFO, FNAME, "message not authenticated "
			    "while authentication required");

			/*
			 * Right now, we simply discard unauthenticated
			 * messages.
			 */
			return (-1);
		}
	} else {
		/* if authenticated, update the "previous" RD value */
		authparam->prevrd = optinfo->authrd;
		authparam->flags &= ~AUTHPARAM_FLAGS_NOPREVRD;
#ifdef CONFIG_IOT_RECONFIGURATION
		g_auth_param.prevrd = optinfo->authrd;
		g_auth_param.flags &= ~AUTHPARAM_FLAGS_NOPREVRD;      
#endif    
	}

	return (0);
}

static int
set_auth(ev, optinfo)
	struct dhcp6_event *ev;
	struct dhcp6_optinfo *optinfo;
{
	struct authparam *authparam = ev->authparam;

	if (authparam == NULL)
		return (0);

	optinfo->authproto = authparam->authproto;
	optinfo->authalgorithm = authparam->authalgorithm;
	optinfo->authrdm = authparam->authrdm;

	switch (authparam->authproto) {
	case DHCP6_AUTHPROTO_UNDEF: /* we simply do not need authentication */
		return (0);
	case DHCP6_AUTHPROTO_DELAYED:
		if (ev->state == DHCP6S_INFOREQ) {
			/*
			 * In the current implementation, delayed
			 * authentication for Information-request and Reply
			 * exchanges doesn't work.  Specification is also
			 * unclear on this usage.
			 */
			dprintf(LOG_WARNING, FNAME, "delayed authentication "
			    "cannot be used for Information-request yet");
			return (-1);
		}

		if (ev->state == DHCP6S_SOLICIT) {
			optinfo->authflags |= DHCP6OPT_AUTHFLAG_NOINFO;
			return (0); /* no auth information is needed */
		}

		if (authparam->key == NULL) {
			dprintf(LOG_INFO, FNAME,
			    "no authentication key for %s",
			    dhcp6_event_statestr(ev));
			return (-1);
		}

		if (dhcp6_validate_key(authparam->key)) {
			dprintf(LOG_INFO, FNAME, "key %s is invalid",
			    authparam->key->name);
			return (-1);
		}

		if (get_rdvalue(optinfo->authrdm, &optinfo->authrd,
		    sizeof(optinfo->authrd))) {
			dprintf(LOG_ERR, FNAME, "failed to get a replay "
			    "detection value");
			return (-1);
		}

		optinfo->delayedauth_keyid = authparam->key->keyid;
		optinfo->delayedauth_realmlen = authparam->key->realmlen;
		optinfo->delayedauth_realmval =
		    malloc(optinfo->delayedauth_realmlen);
		if (optinfo->delayedauth_realmval == NULL) {
			dprintf(LOG_ERR, FNAME, "failed to allocate memory "
			    "for authentication realm");
			return (-1);
		}
		memcpy(optinfo->delayedauth_realmval, authparam->key->realm,
		    optinfo->delayedauth_realmlen);

		break;
	default:
		dprintf(LOG_ERR, FNAME, "unsupported authentication protocol "
		    "%d", authparam->authproto);
		return (-1);
	}

	return (0);
}

static void
info_printf(const char *fmt, ...)
{
	va_list ap;
	char logbuf[LINE_MAX];

	va_start(ap, fmt);
	vsnprintf(logbuf, sizeof(logbuf), fmt, ap);

	dprintf(LOG_DEBUG, FNAME, "%s", logbuf);
	if (infreq_mode)
		printf("%s\n", logbuf);

	return;
}

#ifdef AEI_CONTROL_LAYER
int updateDhcp6sConfDnsList(struct dhcp6_optinfo *opt)
{
   struct dhcp6_listval *d;
   char   nameserver[BUFLEN_128];

   d = TAILQ_FIRST(&opt->dns_list);
   if (d != NULL)
   {
      sprintf(nameserver, "%s", in6addr2str(&d->val_addr6, 0));
   }
   d = TAILQ_NEXT(d, link);
   if (d != NULL)
   {
      strcat(nameserver, ",");
      strcat(nameserver, in6addr2str(&d->val_addr6, 0));
   }
   
   sendDnsEventMessage(nameserver);

   return 0;

}  /* End of updateDhcp6sConfDnsList() */

void sendDnsEventMessage(const char *nameserver)
{
	ctldhcp6cMsgBody.dnsAssigned = TSL_B_TRUE;
	strcpy(ctldhcp6cMsgBody.nameserver, nameserver);

	dprintf(LOG_NOTICE, FNAME, "DHCP6C_DNS_CHANGED");

	return;

}  /* End of sendDnsEventMessage() */

void sendDhcp6cEventMessage(void)
{

   char buf[sizeof(CtlMsgHeader) + sizeof(CtlDhcp6cStateChangedMsgBody)]={0};
   CtlMsgHeader *msg=(CtlMsgHeader *) buf;
   CtlDhcp6cStateChangedMsgBody *dhcp6cBody = (CtlDhcp6cStateChangedMsgBody *) (msg->buffer);
   dbussend_hdl_st *ctlMsgHandle=NULL;
   ctlMsgHandle = dbussend_init(); 
#ifdef CONFIG_IOT_RECONFIGURATION
    got_valid_ia_pd=0;
    got_valid_ia_na=0;
#endif

   if (!ctlMsgHandle) {
	   tsl_printf("dbussend_init fail\n");
	   return -1;
   }
   
   memcpy(dhcp6cBody, &ctldhcp6cMsgBody, sizeof(CtlDhcp6cStateChangedMsgBody));
   msg->data_length=sizeof(CtlDhcp6cStateChangedMsgBody);
   if (dbussend_sendmsg(ctlMsgHandle, CTL_MSG_TYPE(CTL_MSG_DHCP6C_STATE_CHANGED), NULL, msg, sizeof(CtlMsgHeader)+sizeof(CtlDhcp6cStateChangedMsgBody)) < 0)
   {
	   tsl_printf("could not send out DHCP6C_STATUS_CHANGED\n");
	   dbussend_uninit(ctlMsgHandle);
	   return -1;
   }else
   {
		//tsl_printf("===>>>sent out DHCP6C_STATUS_CHANGED\n");
   }

   dbussend_uninit(ctlMsgHandle);
   return;

}  /* End of sendDhcp6cEventMessage() */

#endif
