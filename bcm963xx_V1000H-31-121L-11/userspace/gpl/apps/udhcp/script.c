/* script.c
*
* Functions to call the DHCP client notification scripts 
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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "packet.h"
#include "options.h"
#include "debug.h"
#ifdef AEI_CONTROL_LAYER
//#include "ctl_log.h"
#endif

// brcm
#if defined(AEI_VDSL_CUSTOMER_DHCP_PROBE)
char local_ip[32] = "";
char router_ip[32] = "";
#else
static char local_ip[32] = "";
static char router_ip[32] = "";
#endif
static char dns_ip[128] = "";
static char subnet_ip[32] = "";

#ifdef AEI_SUPPORT_6RD
// 6rd option
static int n_ipv6rd_ipv4_mask_len = 0;
static char s_ipv6rd_bripv4_addr[32] = "";
static int n_ipv6rd_prefix_len = 32;
static char s_ipv6rd_prefix[128] = "";
#endif

#include "cms.h"

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
static unsigned int lease_time = 0;
extern void sendEventMessage(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway,
                             const char *nameserver, int ledControl, unsigned int lease_time);
#else
extern void sendEventMessage(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway,
                             const char *nameserver, int ledControl);
#endif


#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_OPTION121)

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME)
extern void sendEventMessageWithRoute(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway, const char *nameserver,UINT32 ledControl,route_info_t *routeList, int routeNum, unsigned int lease_time);
#else
extern void sendEventMessageWithRoute(UBOOL8 assigned,UBOOL8 isExpired, const char *ip, const char *mask, const char *gateway, const char *nameserver, UINT32 ledControl,route_info_t *routeList, int routeNum);
#endif

#endif


/* get a rough idea of how long an option will be (rounding up...) */
static int max_option_length(char *option, struct dhcp_option *type)
{
    int size = 0;

    switch (type->flags & TYPE_MASK) {
    case OPTION_IP:
    case OPTION_IP_PAIR:
        size = (option[OPT_LEN - 2] / 4) * sizeof("255.255.255.255 ");
        break;
    case OPTION_STRING:
        size = option[OPT_LEN - 2] + 1;
        break;
    case OPTION_BOOLEAN:
        size = option[OPT_LEN - 2] * sizeof("yes ");
        break;
    case OPTION_U8:
        size = option[OPT_LEN - 2] * sizeof("255 ");
        break;
    case OPTION_U16:
        size = (option[OPT_LEN - 2] / 2) * sizeof("65535 ");
        break;
    case OPTION_S16:
        size = (option[OPT_LEN - 2] / 2) * sizeof("-32768 ");
        break;
    case OPTION_U32:
        size = (option[OPT_LEN - 2] / 4) * sizeof("4294967295 ");
        break;
    case OPTION_S32:
        size = (option[OPT_LEN - 2] / 4) * sizeof("-2147483684 ");
        break;
    }

    return size;
}

/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, unsigned char *option, struct dhcp_option *type_p)
{
    int type, optlen;
    u_int16_t val_u16;
    int16_t val_s16;
    u_int32_t val_u32;
    int32_t val_s32;
    int len = option[OPT_LEN - 2];
// brcm
    char tmp[128] = "";

    dest += sprintf(dest, "%s=", type_p->name);

    type = type_p->flags & TYPE_MASK;
    optlen = option_lengths[type];
    for (;;) {
        switch (type) {
        case OPTION_IP:        /* Works regardless of host byte order. */
            dest += sprintf(dest, "%d.%d.%d.%d", option[0], option[1], option[2], option[3]);
            // brcm
            sprintf(tmp, "%d.%d.%d.%d", option[0], option[1], option[2], option[3]);
            if (!strcmp(type_p->name, "dns")) {
                // cwu
                if (strlen(dns_ip) > 0) {
                    strcat(dns_ip, ",");
                }
                strcat(dns_ip, tmp);
            }
            if (!strcmp(type_p->name, "router"))
#if defined(AEI_COVERITY_FIX)
                /*CID 12259:Destination buffer too small*/
                {
                    strlcpy(router_ip, tmp, sizeof(router_ip));
                }
#else
                strcpy(router_ip, tmp);
#endif
            if (!strcmp(type_p->name, "subnet"))
#if defined(AEI_COVERITY_FIX)
                /*CID 12259:Destination buffer too small*/
                {
                    strlcpy(subnet_ip, tmp, sizeof(subnet_ip));
                }
#else
                strcpy(subnet_ip, tmp);
#endif
            break;
        case OPTION_IP_PAIR:
            dest += sprintf(dest, "%d.%d.%d.%d, %d.%d.%d.%d",
                            option[0], option[1], option[2], option[3], option[4], option[5], option[6], option[7]);
            break;
        case OPTION_BOOLEAN:
            dest += sprintf(dest, *option ? "yes" : "no");
            break;
        case OPTION_U8:
            dest += sprintf(dest, "%u", *option);
            break;
        case OPTION_U16:
            memcpy(&val_u16, option, 2);
            dest += sprintf(dest, "%u", ntohs(val_u16));
            break;
        case OPTION_S16:
            memcpy(&val_s16, option, 2);
            dest += sprintf(dest, "%d", ntohs(val_s16));
            break;
        case OPTION_U32:
            memcpy(&val_u32, option, 4);
            dest += sprintf(dest, "%lu", (unsigned long)ntohl(val_u32));

//#if defined(AEI_VDSL_CUSTOMER_TELUS)
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
            if (!strcmp(type_p->name, "lease")) {
                lease_time = val_u32;
            }
#endif

            break;
        case OPTION_S32:
            memcpy(&val_s32, option, 4);
            dest += sprintf(dest, "%ld", (long)ntohl(val_s32));
            break;
        case OPTION_STRING:
            memcpy(dest, option, len);
            dest[len] = '\0';
            return;             /* Short circuit this case */
        }
        option += optlen;
        len -= optlen;
        if (len <= 0)
            break;
        *(dest++) = ' ';
    }
}

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_OPTION121)

static int getClasslessRDDLength(unsigned maskNum)
{
	if (maskNum == 0)
	{
		return 1;
	}
	else if (maskNum<=8 && maskNum>=1)		
	{
		return 2;
	}
	else if (maskNum<=16 && maskNum>=9)
	{
		return 3;
	}
	else if (maskNum<=24 && maskNum>=17)
	{
		return 4;
	}
	else if (maskNum<=32 && maskNum>=25)
	{
		return 5;
	}
	return 0;
}	

static unsigned int changeNumtoNetMask(unsigned char netmask)
{
	unsigned int temp = 0;
	int i;
	for (i=0; i<netmask; i++)
	{
		temp = temp>>1;
		temp = temp|0x80000000;
	}
	
	return temp;
}

static void freeRouteList(route_info_t *routeList)
{
	route_info_t *routeTemp = routeList;
	while(routeTemp != NULL)
	{
		routeList = routeList->next;
		free(routeTemp);
		routeTemp = routeList;
	}
}


#endif


static char *find_env(const char *prefix, char *defaultstr)
{
    extern char **environ;
    char **ptr;
    const int len = strlen(prefix);

    for (ptr = environ; *ptr != NULL; ptr++) {
        if (strncmp(prefix, *ptr, len) == 0)
            return *ptr;
    }
    return defaultstr;
}

/* put all the paramaters into an environment */
static char **fill_envp(struct dhcpMessage *packet)
{
    int num_options = 0;
    int i, j;
    unsigned char *addr;
    char **envp;
    unsigned char *temp;
    char over = 0;

    if (packet == NULL)
        num_options = 0;
    else {
        for (i = 0; options[i].code; i++)
            if (get_option(packet, options[i].code))
                num_options++;
        if (packet->siaddr)
            num_options++;
        if ((temp = get_option(packet, DHCP_OPTION_OVER)))
            over = *temp;
        if (!(over & FILE_FIELD) && packet->file[0])
            num_options++;
        if (!(over & SNAME_FIELD) && packet->sname[0])
            num_options++;
    }

    envp = malloc((num_options + 5) * sizeof(char *));
    envp[0] = malloc(strlen("interface=") + strlen(client_config.interface) + 1);
    sprintf(envp[0], "interface=%s", client_config.interface);
    envp[1] = malloc(sizeof("ip=255.255.255.255"));
    envp[2] = find_env("PATH", "PATH=/bin:/usr/bin:/sbin:/usr/sbin");
    envp[3] = find_env("HOME", "HOME=/");

    if (packet == NULL) {
        envp[4] = NULL;
        return envp;
    }

    addr = (unsigned char *)&packet->yiaddr;
    sprintf(envp[1], "ip=%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
// brcm
    sprintf(local_ip, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    strcpy(dns_ip, "");

#ifdef AEI_SUPPORT_6RD
// cleanup
    n_ipv6rd_ipv4_mask_len = 0;
    memset(s_ipv6rd_bripv4_addr, 0, sizeof(s_ipv6rd_bripv4_addr));
    n_ipv6rd_prefix_len = 0;    //32
    memset(s_ipv6rd_prefix, 0, sizeof(s_ipv6rd_prefix));
#endif
    for (i = 0, j = 4; options[i].code; i++) {
        if ((temp = get_option(packet, options[i].code))) {
#ifdef AEI_SUPPORT_6RD
            //ctllog_debug( "code=0X%02x,len=%d",temp[-2],temp[-1]);
            if (temp[OPT_CODE - 2] == DHCP_6RD) {
                char ipv6prefix[16] = "";
                char ipv4BR[4] = "";

                n_ipv6rd_ipv4_mask_len = temp[0];
                n_ipv6rd_prefix_len = temp[1];
                //ctllog_debug( "temp[0]=0X%02x\n",temp[0]);
                //ctllog_debug( "6rd_netmask_len=%d\n", n_ipv6rd_ipv4_mask_len);
                //ctllog_debug( "6rd_ipv6_prefix_len=%d\n", n_ipv6rd_prefix_len);
                memcpy(ipv6prefix, &temp[2], sizeof(ipv6prefix));
                memcpy(ipv4BR, &temp[18], sizeof(ipv4BR));
                if (inet_ntop(AF_INET6, (const void *)ipv6prefix, s_ipv6rd_prefix, sizeof(s_ipv6rd_prefix))) {
                    //ctllog_debug( "6rd_ipv6_prefix=%s\n", s_ipv6rd_prefix );
                } else {
                    //ctllog_error( "Cann't translate binary(ipv6rd prefix) to asc\n");
                }

                if (inet_ntop(AF_INET, (const void *)ipv4BR, s_ipv6rd_bripv4_addr, sizeof(s_ipv6rd_bripv4_addr))) {
                    //ctllog_debug("6rd_ipv4_br=%s\n",s_ipv6rd_bripv4_addr);
                } else {
                    //ctllog_error("Cann't translate binary(br ipv4 addr) to asc\n");
                }
            } else
#endif
            {
                envp[j] = malloc(max_option_length(temp, &options[i]) + strlen(options[i].name) + 2);
                fill_options(envp[j], temp, &options[i]);
                j++;
            }
        }
    }


#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_OPTION121) //add william 2011-12-7

	int len=0, routeNum=0,tempLen=0, totalLen=0;
	unsigned char netMaskNum;
	unsigned char routeBuf[4];
	u_int32_t netMask;
	u_int32_t destIP;
	u_int32_t gateWay;
	route_info_t *routeList=NULL, *routeTemp=NULL;

	if ((temp = get_option(packet, DHCP_CLASSLESS_ROUTE)))
	{
		len = *(temp - 1);

		if ((routeList=routeTemp= (route_info_t*)malloc(sizeof(route_info_t))) == NULL)
		{
			printf("Mem is poor\n");
		}
		else
		{
			while (totalLen < len)
			{
				routeNum++;
				netMaskNum = *temp;
				tempLen = getClasslessRDDLength(netMaskNum);
				netMask = changeNumtoNetMask(netMaskNum);	
				sprintf(routeTemp->netmask, "%s", inet_ntoa(*(struct in_addr*)(&netMask)));
				memset(routeBuf, 0, sizeof(routeBuf));
				memcpy(routeBuf, temp+1, tempLen-1);
				destIP = *(u_int32_t*)(routeBuf);
				sprintf(routeTemp->desIP, "%s", inet_ntoa(*(struct in_addr*)(&destIP)));
				memset(routeBuf, 0, sizeof(routeBuf));
				memcpy(routeBuf, temp+tempLen, 4);
				gateWay = *(u_int32_t*)(routeBuf);
				sprintf(routeTemp->gateway, "%s", inet_ntoa(*(struct in_addr*)(&gateWay)));
				routeTemp->next = NULL;
				totalLen +=tempLen + 4;
				temp = temp+ tempLen +4;
				if (totalLen <len)
				{
					if ((routeTemp->next = (route_info_t*)malloc(sizeof(route_info_t))) != NULL)
					{
						routeTemp = routeTemp->next;
					}
					else
					{
						printf("Mem is poor\n");
						break;	
					}
				}
			}
		}		
	}
	
#endif

	
    if (packet->siaddr) {
        envp[j] = malloc(sizeof("siaddr=255.255.255.255"));
        addr = (unsigned char *)&packet->yiaddr;
        sprintf(envp[j++], "siaddr=%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    }
    if (!(over & FILE_FIELD) && packet->file[0]) {
        /* watch out for invalid packets */
        packet->file[sizeof(packet->file) - 1] = '\0';
        envp[j] = malloc(sizeof("boot_file=") + strlen(packet->file));
        sprintf(envp[j++], "boot_file=%s", packet->file);
    }
    if (!(over & SNAME_FIELD) && packet->sname[0]) {
        /* watch out for invalid packets */
        packet->sname[sizeof(packet->sname) - 1] = '\0';
        envp[j] = malloc(sizeof("sname=") + strlen(packet->sname));
        sprintf(envp[j++], "sname=%s", packet->sname);
    }
    envp[j] = NULL;

//#if defined(AEI_VDSL_CUSTOMER_TELUS)

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_OPTION121)

  #if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME)
	sendEventMessageWithRoute(TRUE,FALSE,local_ip, subnet_ip, router_ip, dns_ip,0, routeList, routeNum, lease_time);
  #else
	sendEventMessageWithRoute(TRUE,FALSE,local_ip, subnet_ip, router_ip, dns_ip,0,routeList, routeNum);
  #endif

	freeRouteList(routeList);

#else

#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
    sendEventMessage(TRUE,FALSE, local_ip, subnet_ip, router_ip, dns_ip, 0, lease_time);
#else
    sendEventMessage(TRUE,FALSE, local_ip, subnet_ip, router_ip, dns_ip, 0);
#endif

#endif



#if defined(AEI_CONTROL_LAYER) && defined(AEI_SUPPORT_6RD)
    sendEventMessageWith6RD(TRUE, local_ip, subnet_ip, router_ip, dns_ip,
                            n_ipv6rd_ipv4_mask_len, s_ipv6rd_bripv4_addr, n_ipv6rd_prefix_len, s_ipv6rd_prefix);
#endif

    return envp;
}

/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
    char **envp;
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //add william 2011-12-5
   if (!strcmp(name, "bound")||!strcmp(name, "renew"))
#else	
    if (!strcmp(name, "bound") ) 
#endif
	{
        envp = fill_envp(packet);

        /*Coverity Fix CID 11898 Resource Leak fix*/
#if defined(AEI_VDSL_CUSTOMER_NCS)
        int i = 0;
        while ( envp[i] != NULL)
        {
        	if ( i != 2 && i !=3 )
        	{	
        		//skip 2, 3, it is not allocated by malloc
        		free(envp[i]);
        	}
    		envp[i] = NULL;
        	i++;
        }
        free(envp);
#else	        
        free(*envp);
#endif    		     		
    }
	else if (!strcmp(name, "release")) /* brcm: Expired, set the connection down */
	{
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
    sendEventMessage(FALSE,TRUE ,"0", "0", "0", "0", 0, 0);
#else
    sendEventMessage(FALSE,TRUE, "0", "0", "0", "0",0);
#endif
	}
	else if (!strcmp(name, "deconfig")) /* brcm: Expired, set the connection down */
	{
#if defined(AEI_VDSL_CUSTOMER_DHCP_WAN_LEASETIME) //modify william 2011-11-29
    sendEventMessage(FALSE,TRUE ,"1", "1", "1", "1", 0, 0);
#else
    sendEventMessage(FALSE,TRUE, "1", "1", "1", "1",0);
#endif
	}	

}
