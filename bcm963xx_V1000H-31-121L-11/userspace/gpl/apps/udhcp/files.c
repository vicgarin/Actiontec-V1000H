/* 
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <ctype.h>

#include "cms_msg.h"
#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "static_leases.h"

#define BRCM_RETRY_INTERVAL 1
#define BRCM_RETRY_COUNT    3

#ifdef DHCP_RELAY
static void register_message(CmsMsgType msgType);
#endif

typedef struct netiface {
    char nif_name[32];
    unsigned char nif_mac[6];
    unsigned int nif_index;
    in_addr_t nif_ip;
} netiface;

netiface *get_netifaces(int *count)
{
    netiface *netifaces = NULL;
    struct ifconf ifc;
    char buf[1024];
    int skt;
    int i;

    /* Create socket for querying interfaces */
    if ((skt = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return NULL;

    /* Query available interfaces. */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(skt, SIOCGIFCONF, &ifc) < 0)
        goto err;

    /* Allocate memory for netiface array */
    if (ifc.ifc_len < 1)
        goto err;
    *count = ifc.ifc_len / sizeof(struct ifreq);
    netifaces = calloc(*count, sizeof(netiface));
    if (netifaces == NULL)
        goto err;

    /* Iterate through the list of interfaces to retrieve info */
    for (i = 0; i < *count; i++) {
        struct ifreq ifr;
        ifr.ifr_addr.sa_family = AF_INET;
        strcpy(ifr.ifr_name, ifc.ifc_req[i].ifr_name);

        /* Interface name */
        strcpy(netifaces[i].nif_name, ifc.ifc_req[i].ifr_name);

        /* Interface index */
        if (ioctl(skt, SIOCGIFINDEX, &ifr))
            goto err;
        netifaces[i].nif_index = ifr.ifr_ifindex;

        /* IPv4 address */
        if (ioctl(skt, SIOCGIFADDR, &ifr))
            goto err;
        netifaces[i].nif_ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

        /* MAC address */
        if (ioctl(skt, SIOCGIFHWADDR, &ifr))
            goto err;
        memcpy(netifaces[i].nif_mac, ifr.ifr_hwaddr.sa_data, 6);

    }
    close(skt);
    return netifaces;
err:
    close(skt);
    if (netifaces)
        free(netifaces);
    return NULL;
}

/* on these functions, make sure you datatype matches */
static int read_ip(char *line, void *arg)
{
    struct in_addr *addr = arg;
    inet_aton(line, addr);
    return 1;
}

//For static IP lease
static int read_mac(const char *line, void *arg)
{
    uint8_t *mac_bytes = arg;
    struct ether_addr *temp_ether_addr;
    int retval = 1;

    temp_ether_addr = ether_aton(line);

    if (temp_ether_addr == NULL)
        retval = 0;
    else
        memcpy(mac_bytes, temp_ether_addr, 6);

    return retval;
}

static int read_str(char *line, void *arg)
{
    char **dest = arg;
    int i;

    if (*dest)
        free(*dest);

    *dest = strdup(line);

    /* elimate trailing whitespace */
    for (i = strlen(*dest) - 1; i > 0 && isspace((*dest)[i]); i--) ;
    (*dest)[i > 0 ? i + 1 : 0] = '\0';
    return 1;
}

static int read_qstr(char *line, char *arg, int max_len)
{
    char *p = line;
    int quoted = 0;
    int len;

    if (*p == '\"') {
        quoted = 1;
        line++;
        p++;
    }

    while (*p) {
        if (*p == '\"' && quoted)
            break;
        else if (isspace(*p)) {
            if (!isblank(*p) || !quoted)
                break;
        }
        p++;
    }

    len = p - line;
    if (len >= max_len)
        len = max_len - 1;
    memcpy(arg, line, len);
    arg[len] = 0;

    return len;
}

static int read_u32(char *line, void *arg)
{
    u_int32_t *dest = arg;
    *dest = strtoul(line, NULL, 0);
    return 1;
}

static int read_yn(char *line, void *arg)
{
    char *dest = arg;
    if (!strcasecmp("yes", line) || !strcmp("1", line) || !strcasecmp("true", line))
        *dest = 1;
    else if (!strcasecmp("no", line) || !strcmp("0", line) || !strcasecmp("false", line))
        *dest = 0;
    else
        return 0;

    return 1;
}

#ifdef AEI_VDSL_CUSTOMER_QWEST
static int read_dns_opt(char *line, struct ip_list **dns_list)
{
    int i;
    char text[256];
    char *opt, *val;
    struct ip_list *p;
    struct dhcp_option *option = NULL;

    if (line == 0 || dns_list == 0)
        return 0;

    /* do not modify the original string */
    /*CID 10197: Buffer not null terminated*/
    strlcpy(text, line, sizeof(text));

    if (!(opt = strtok(text, " \t=")))
        return 0;

    for (i = 0; options[i].code; i++) {
        if (!strcmp(options[i].name, opt)) {
            option = &(options[i]);
            if (option->code != DHCP_DNS_SERVER)
                return 0;
            else
                break;
        }
    }

    if (!option)
        return 0;

    do {
        val = strtok(NULL, ", \t");
        if (val) {
            struct ip_list *new = calloc(1, sizeof(struct ip_list));
            if (*dns_list == NULL) {
                *dns_list = new;
                p = new;
            } else {
                for (p = *dns_list; p->next; p = p->next) ;

                p->next = new;
                p = new;
            }

            read_ip(val, &p->ip);
        } else
            break;
    }
    while (option->flags & OPTION_LIST);
    return 0;
}
#endif

/* read a dhcp option and add it to opt_list */
static int read_opt(char *line, void *arg)
{
    struct option_set **opt_list = arg;
    char *opt, *val;
    char fail;
    struct dhcp_option *option = NULL;
    int length = 0;
    char buffer[255];
    u_int16_t result_u16;
    int16_t result_s16;
    u_int32_t result_u32;
    int32_t result_s32;

    int i;

    if (!(opt = strtok(line, " \t=")))
        return 0;

    for (i = 0; options[i].code; i++)
        if (!strcmp(options[i].name, opt)) {
            option = &(options[i]);
            break;
        }

    if (!option)
        return 0;

    do {
        val = strtok(NULL, ", \t");
        if (val) {
            fail = 0;
            length = 0;
            switch (option->flags & TYPE_MASK) {
            case OPTION_IP:
                read_ip(val, buffer);
                break;
            case OPTION_IP_PAIR:
                read_ip(val, buffer);
                if ((val = strtok(NULL, ", \t/-")))
                    read_ip(val, buffer + 4);
                else
                    fail = 1;
                break;
            case OPTION_STRING:
                length = strlen(val);
                if (length > 254)
                    length = 254;
                memcpy(buffer, val, length);
                break;
            case OPTION_BOOLEAN:
                if (!read_yn(val, buffer))
                    fail = 1;
                break;
            case OPTION_U8:
                buffer[0] = strtoul(val, NULL, 0);
                break;
            case OPTION_U16:
                result_u16 = htons(strtoul(val, NULL, 0));
                memcpy(buffer, &result_u16, 2);
                break;
            case OPTION_S16:
                result_s16 = htons(strtol(val, NULL, 0));
                memcpy(buffer, &result_s16, 2);
                break;
            case OPTION_U32:
                result_u32 = htonl(strtoul(val, NULL, 0));
                memcpy(buffer, &result_u32, 4);
                break;
            case OPTION_S32:
                result_s32 = htonl(strtol(val, NULL, 0));
                memcpy(buffer, &result_s32, 4);
                break;
            default:
                break;
            }
            length += option_lengths[option->flags & TYPE_MASK];
            if (!fail)
                attach_option(opt_list, option, buffer, length);
        } else
            fail = 1;
    } while (!fail && option->flags & OPTION_LIST);
    return 1;
}

//For static IP lease
static int read_staticlease(const char *const_line, void *arg)
{

    char *line;
    char *mac_string;
    char *ip_string;
    uint8_t *mac_bytes;
    uint32_t *ip;
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	uint32_t gw = 0;
	uint32_t subnet = 0;
	uint32_t dns1 = 0;
	uint32_t dns2 = 0;
#endif
    /* Allocate memory for addresses */
    mac_bytes = xmalloc(sizeof(unsigned char) * 8);
    ip = xmalloc(sizeof(uint32_t));

    /* Read mac */
    line = (char *)const_line;
    mac_string = strtok(line, " \t");
    read_mac(mac_string, mac_bytes);

    /* Read ip */
    ip_string = strtok(NULL, " \t");
    read_ip(ip_string, ip);

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	ip_string = strtok(NULL, " \t");
        if (ip_string)
	{
		//if not placeholder, read the ip
		if (strcasecmp(ip_string,"gateway"))
			read_ip(ip_string, &gw);
		ip_string = strtok(NULL, " \t");
		if (ip_string)
		{
			if (strcasecmp(ip_string,"subnet"))
				read_ip(ip_string, &subnet);
			ip_string = strtok(NULL, " \t");
			if (ip_string)
			{
				if (strcasecmp(ip_string,"dns"))
				{
					cmsLog_error("========dns %s=========",ip_string);
					char *d1 = strtok(ip_string, ",");
					if (d1)
					{
						cmsLog_error("========dns1 %s=========",d1);
						read_ip(d1, &dns1);
					}
					ip_string = strtok(NULL, " \t");
					if (ip_string)
					{
						cmsLog_error("========dns2 %s=========",ip_string);
						read_ip(ip_string, &dns2);
					}
				}
			}
		}
	}
	addStaticLease(arg, mac_bytes, ip, gw, subnet, dns1, dns2);
#else
	addStaticLease(arg, mac_bytes, ip);
#endif

#ifdef UDHCP_DEBUG
    printStaticLeases(arg);
#endif

    return 1;

}

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) //add william 2012-4-25
static int AEI_read_dhcpvlanoption60(const char *const_line, struct iface_config_t *iface)
{
	int i = 0;
	char *line, *token;

	line = (char *) const_line;
	#if 0
	LOG(LOG_ERR, "william->AEI_read_dhcpvlanoption60() line=\"%s\"\n", line);
	#endif
	token = strtok(line, "|");

	if (token) 
	{
        if (strlen(token)) 
		{
            if (iface->dhcpvlanOption60list==NULL) 
			{
                iface->dhcpvlanOption60list =  malloc(sizeof(struct dhcpvlanOption60));
				memset(iface->dhcpvlanOption60list,0,sizeof(struct dhcpvlanOption60));
                if (iface->dhcpvlanOption60list) 
				{
                    iface->dhcpvlanOption60list->next=NULL;

                    //iface->vlanOption60list->vlanID = strdup(token);
                    strcpy(iface->dhcpvlanOption60list->vlanID,token);
	                token = strtok(NULL, VENDOR_CLASS_ID_TOKEN);

					if(token)
					{
						strcpy(iface->dhcpvlanOption60list->vendorClassId,token);
					}
                    //then init the list
               
                }   
            }
		    else 
			{
		       struct dhcpvlanOption60 * prev = iface->dhcpvlanOption60list;
		       struct dhcpvlanOption60 * curr = iface->dhcpvlanOption60list;
		       do {
		           prev = curr;
		           curr = curr->next; 
		       }
		       while (curr);

		       curr = prev->next = malloc(sizeof(struct dhcpvlanOption60));
			   memset(curr,0,sizeof(struct dhcpvlanOption60));
		       if (curr) 
			   {
		            curr->next=NULL;

					strcpy(curr->vlanID,token);
	                token = strtok(NULL, VENDOR_CLASS_ID_TOKEN);

					if(token)
					{
						strcpy(curr->vendorClassId,token);
					}	

		      	}    
		    }
            }
        }

	return 0;
}
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS) //add william 2012-1-11
static int AEI_read_vlanoption60(const char *const_line, struct iface_config_t *iface)
{
	int i = 0;
	char *line, *token;

	line = (char *) const_line;
	#if 0
	LOG(LOG_ERR, "william->read_vendorClassId() line=\"%s\"\n", line);
	#endif

        token = strtok(line, "|");
        if (token) {
            if (strlen(token)) {
                if (iface->vlanOption60list==NULL) {
                    iface->vlanOption60list =  malloc(sizeof(struct vlanOption60));
                    if (iface->vlanOption60list) {
                        iface->vlanOption60list->next=NULL;

                        //blank the list because uninit things might not be NULL                        
                        for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++)
                            iface->vlanOption60list->vendorClassId[i]=NULL;

                        i = 0;
                        iface->vlanOption60list->vlanID = strdup(token);
	                token = strtok(NULL, VENDOR_CLASS_ID_TOKEN);
                        //then init the list
	                while (token && i < VENDOR_CLASS_ID_TAB_SIZE) {
		            iface->vlanOption60list->vendorClassId[i] = strdup(token);
		            token = strtok(NULL, VENDOR_CLASS_ID_TOKEN);
		            i++;
	                }
                    }    
                }
                else {
                   struct vlanOption60 * prev = iface->vlanOption60list;
                   struct vlanOption60 * curr = iface->vlanOption60list;
                   do {
                       prev = curr;
                       curr = curr->next; 
                   }
                   while (curr);

                   curr = prev->next = malloc(sizeof(struct vlanOption60));
                   if (curr) {
                        curr->next=NULL;
                        //blank the list                        
                        for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++)
                            curr->vendorClassId[i]=NULL;
                        i = 0;
                        curr->vlanID = strdup(token);
	                token = strtok(NULL, VENDOR_CLASS_ID_TOKEN);
	                while (token && i < VENDOR_CLASS_ID_TAB_SIZE) {
		            curr->vendorClassId[i] = strdup(token);
		            token = strtok(NULL, VENDOR_CLASS_ID_TOKEN);
		            i++;
	                }
                    }    
                }
            }
        }

	return 1;
}
#endif


static void release_iface_config(struct iface_config_t *iface)
{
    int i;
    struct option_set *cur, *next;
    struct static_lease *sl_cur, *sl_next;
    vendor_id_t *vid_cur, *vid_next;

    if (iface->skt >= 0) {
        close(iface->skt);
        iface->skt = -1;
    }

#if defined(AEI_VDSL_CUSTOMER_NCS)
	//int i;
    struct vlanOption60 * curr = iface->vlanOption60list;
    struct vlanOption60 * prev = NULL;
	iface->vendorClassIdMinAddress = 0;
	iface->vendorClassIdMaxAddress = 0;
    for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
    	if (iface->vendorClassId[i]) {
        	free(iface->vendorClassId[i]);
            iface->vendorClassId[i] = NULL;
        }
    }
        
    while (curr) {
    	prev = curr;
        curr = curr->next;
        for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
        	if (prev->vendorClassId[i]) {
            	free(prev->vendorClassId[i]);
                prev->vendorClassId[i] = NULL;
            }
        }
        if (prev->vlanID) {
            free(prev->vlanID);
            prev->vlanID=NULL;
        }
        prev->next=NULL;
        free(prev);
        prev = NULL;
    } 
#endif

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) //add william 2012-4-25
	struct vlanOption60 * vlancurr = iface->dhcpvlanOption60list;
    struct vlanOption60 * vlanprev = NULL;
	while (vlancurr) 
	{
    	vlanprev = vlancurr;
        vlancurr = vlancurr->next;
        vlanprev->next=NULL;
        free(vlanprev);
        vlanprev = NULL;
    } 
#endif

	
#ifdef AEI_VDSL_CUSTOMER_TELUS
    for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
        if (iface->opt67WarrantVids[i])
            free(iface->opt67WarrantVids[i]);
    }
#endif

#if defined(AEI_VDSL_DHCP_LEASE)
    for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
        if (iface->stbVids[i])
            free(iface->stbVids[i]);
    }
#endif


#if defined(AEI_VDSL_STB_NO_FIREWALL)
    struct ip_mac_list *p, *n;

    for (p = iface->stb_list; p != NULL; p = n) {
        n = p->next;
        do_mark(p->ip, FALSE);
        free(p);
        p = NULL;
    }
#endif

#ifdef AEI_VDSL_CUSTOMER_QWEST
    struct ip_list *p, *n;

    for (p = iface->dns_srv_ips; p != NULL; p = n) {
        n = p->next;
        free(p);
        p = NULL;
    }

    if (iface->dns_passthru_chaddr)
        free(iface->dns_passthru_chaddr);
#endif

    cur = iface->options;
    while (cur) {
        next = cur->next;
        if (cur->data)
            free(cur->data);
        free(cur);
        cur = next;
    }
    if (iface->interface)
        free(iface->interface);
    if (iface->sname)
        free(iface->sname);
    if (iface->boot_file)
        free(iface->boot_file);
    if (iface->leases)
        free(iface->leases);
    sl_cur = iface->static_leases;
    while (sl_cur) {
        sl_next = sl_cur->next;
        if (sl_cur->mac)
            free(sl_cur->mac);
        if (sl_cur->ip)
            free(sl_cur->ip);
        free(sl_cur);
        sl_cur = sl_next;
    }
    vid_cur = iface->vendor_ids;
    while (vid_cur) {
        vid_next = vid_cur->next;
        free(vid_cur);
        vid_cur = vid_next;
    }
    free(iface);
}

static void set_server_config_defaults(void)
{
#if defined(AEI_VDSL_DHCP_LEASE)
    server_config.remaining = 0;
    server_config.auto_time = UPDATE_DHCP_LEASE_TIMEOUT;
#else
    server_config.remaining = 1;
    server_config.auto_time = 7200;
#endif
    server_config.decline_time = 3600;
    server_config.conflict_time = 3600;
    server_config.offer_time = 3600;
    server_config.min_lease = 60;
    if (server_config.lease_file)
        free(server_config.lease_file);
    server_config.lease_file = strdup("/etc/udhcpd.leases");
    if (server_config.pidfile)
        free(server_config.pidfile);
    server_config.pidfile = strdup("/var/run/udhcpd.pid");
    if (server_config.notify_file)
        free(server_config.notify_file);
    server_config.notify_file = NULL;
    if (server_config.decline_file)
        free(server_config.decline_file);
    server_config.decline_file = strdup("");
}

static int set_iface_config_defaults(void)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    struct option_set *option;
    struct iface_config_t *iface;
    int retry_count;
    int local_rc;
    int foundBr = 0;

    /* Create fd to retrieve interface info */
    if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        LOG(LOG_ERR, "socket failed!");
        return 0;
    }

    for (iface = iface_config; iface; iface = iface->next) {

        if (iface->interface == NULL || strstr(iface->interface, "br") == 0)
            continue;

        /* Initialize socket to invalid */
        iface->skt = -1;
        /* Retrieve IP of the interface */
        /*
         * BRCM begin: mwang: during startup, dhcpd is started by
         * rcl_lanHostCfgObject, but br0 has not been created yet
         * because that is done by rcl_lanIpIntfObject, which is
         * called after rcl_lanHostCfgObject. So retry a few times
         * to get br0 info before giving up.
         */
        local_rc = -1;
        for (retry_count = 0; retry_count < BRCM_RETRY_COUNT; retry_count++) {
            ifr.ifr_addr.sa_family = AF_INET;
#if defined(AEI_COVERITY_FIX)
            /*CID 12258:Copy into fixed size buffer*/
            strlcpy(ifr.ifr_name, iface->interface, sizeof(ifr.ifr_name));
#else
            strcpy(ifr.ifr_name, iface->interface);
#endif
            if ((local_rc = ioctl(fd, SIOCGIFADDR, &ifr)) == 0) {
                sin = (struct sockaddr_in *)&ifr.ifr_addr;
                iface->server = sin->sin_addr.s_addr;
                DEBUG(LOG_INFO, "server_ip(%s) = %s", ifr.ifr_name, inet_ntoa(sin->sin_addr));
                break;
            }
#if defined(AEI_VDSL_CUSTOMER_QWEST) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)   //hk_qwest//hk_ctl
            sleep((retry_count + 1) * (1 + retry_count));
#else
            sleep(BRCM_RETRY_INTERVAL);
#endif
        }
        if (local_rc < 0) {
            LOG(LOG_ERR, "SIOCGIFADDR failed on %s!", ifr.ifr_name);
#if !defined(AEI_VDSL_CUSTOMER_QWEST) && !defined(AEI_VDSL_CUSTOMER_BELLALIANT)  //hk_qwest //hk_ctl
            /*Coverity Fix CID:11895 Resource leak*/
            close(fd);
            return 0;
#else
            if (strcmp(ifr.ifr_name, "br0") == 0) {
#if defined(AEI_COVERITY_FIX)
                /*Coverity Fix CID:11895 Resource leak*/
                close(fd);
#endif
                return 0;
            } else {
#if defined(AEI_COVERITY_FIX)
                /*Coverity Fix CID:11895 Resource leak*/
                close(fd);
#endif
                return 1;
            }
#endif
        }

        /* Set default start and end if missing */
        if (iface->start == 0) {
            iface->start = (iface->server & htonl(0xffffff00)) | htonl(20);
        }
        if (iface->end == 0) {
            iface->end = (iface->server & htonl(0xffffff00)) | htonl(254);
        }
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
        if (iface->vendorClassIdMinAddress == 0) {
            iface->vendorClassIdMinAddress = iface->start;
        }
        if (iface->vendorClassIdMaxAddress == 0) {
            iface->vendorClassIdMaxAddress = iface->end;
        }
#endif

        /* Retrieve ifindex of the interface */
        if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
            DEBUG(LOG_INFO, "ifindex(%s)  = %d", ifr.ifr_name, ifr.ifr_ifindex);
            iface->ifindex = ifr.ifr_ifindex;
        } else {
            LOG(LOG_ERR, "SIOCGIFINDEX failed on %s!", ifr.ifr_name);
#if defined(AEI_COVERITY_FIX)
                /*Coverity Fix CID:11895 Resource leak*/
            close(fd);
#endif
            return 0;
        }
        /* Retrieve MAC of the interface */
        if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
            memcpy(iface->arp, ifr.ifr_hwaddr.sa_data, 6);
            DEBUG(LOG_INFO, "mac(%s) = "
                  "%02x:%02x:%02x:%02x:%02x:%02x", ifr.ifr_name,
                  iface->arp[0], iface->arp[1], iface->arp[2], iface->arp[3], iface->arp[4], iface->arp[5]);
        } else {
            LOG(LOG_ERR, "SIOCGIFHWADDR failed on %s!", ifr.ifr_name);
#if defined(AEI_COVERITY_FIX)
                /*Coverity Fix CID:11895 Resource leak*/
            close(fd);
#endif
            return 0;
        }
        /* set lease time from option or default */
        if ((option = find_option(iface->options, DHCP_LEASE_TIME))) {
            memcpy(&iface->lease, option->data + 2, 4);
            iface->lease = ntohl(iface->lease);
        } else
            iface->lease = LEASE_TIME;
        /* Set default max_leases */
        if (iface->max_leases == 0)
            iface->max_leases = 254;
        /* Allocate for leases */
        iface->leases = calloc(1, sizeof(struct dhcpOfferedAddr) * iface->max_leases);

        foundBr = 1;
    }
    close(fd);
//      return 1;
    return foundBr;
}

#ifdef DHCP_RELAY
#if 0                           // For single interface
static u_int32_t relay_remote;
#endif
void set_relays(void)
{
    int skt;
    int socklen;
    struct sockaddr_in addr;
    struct iface_config_t *iface;
    struct relay_config_t *relay;
    struct relay_config_t *new_relay;

    netiface *nifs = NULL;
    int nif_count;
    int i;

    /* Release all relays */
    cur_relay = relay_config;
    while (cur_relay) {
        relay = cur_relay->next;
        if (cur_relay->skt >= 0)
            close(cur_relay->skt);
        free(cur_relay);
        cur_relay = relay;
    }
    relay_config = cur_relay = NULL;

    /* Reset all relay interface names */
    for (iface = iface_config; iface; iface = iface->next) {
#if 0                           // For single interface
        iface->relay_remote = relay_remote;
#endif
        iface->relay_interface[0] = 0;
    }

    /* Get network interface array */
    for (i = 0; i < BRCM_RETRY_COUNT; i++) {
        if ((nifs = get_netifaces(&nif_count)))
            break;
        if (i < BRCM_RETRY_COUNT)
            sleep(BRCM_RETRY_INTERVAL);
    }
    if (nifs == NULL) {
        LOG(LOG_ERR, "failed querying interfaces\n");
        return;
    }

    /* Create UDP for looking up routes */
    if ((skt = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        free(nifs);
        return;
    }

    for (iface = iface_config; iface; iface = iface->next) {
        /* Is this a relay interface? */
        if (iface->decline || iface->relay_remote == 0)
            continue;

        /* Connect UDP socket to relay to find out local IP address */
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = SERVER_PORT;
        addr.sin_addr.s_addr = iface->relay_remote;
        if (connect(skt, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            LOG(LOG_WARNING, "no route to relay %u.%u.%u.%u",
                ((unsigned char *)&addr.sin_addr.s_addr)[0],
                ((unsigned char *)&addr.sin_addr.s_addr)[1],
                ((unsigned char *)&addr.sin_addr.s_addr)[2], ((unsigned char *)&addr.sin_addr.s_addr)[3]);
            continue;
        }
        socklen = sizeof(addr);
        if (getsockname(skt, (struct sockaddr *)&addr, &socklen) < 0)
            continue;

        /* Iterate through the list of interfaces to find the one that
         * has route to remote DHCP server */
        for (i = 0; i < nif_count; i++) {
            if (nifs[i].nif_ip == addr.sin_addr.s_addr) {
                strcpy(iface->relay_interface, nifs[i].nif_name);
                break;
            }
        }
        if (!iface->relay_interface[0])
            continue;

        /* If the same relay (same relay interface) has been created,
         * don't do it again */
        for (relay = relay_config; relay; relay = relay->next) {
            if (!strcmp(relay->interface, iface->relay_interface))
                break;
        }
        if (relay)
            continue;

        /* Create new relay entry */
        new_relay = malloc(sizeof(*new_relay));
        new_relay->next = NULL;
        strcpy(new_relay->interface, iface->relay_interface);
        new_relay->skt = -1;

        /* Link new relay */
        if (relay_config) {
            for (relay = relay_config; relay->next; relay = relay->next) ;
            relay->next = new_relay;
        } else
            relay_config = new_relay;
    }
    close(skt);
    free(nifs);
}
#endif

#ifdef AEI_VDSL_CUSTOMER_NCS
static void read_vids(const char *line, char **vids, int size)
{
    int i;
    char *token, *str;

    if (line == NULL || vids == NULL || size <= 0)
        return;

    for (i = 0, str = line; i < size; i++, str = NULL) {
        token = strtok(str, ",");
        if (token == NULL)
            break;

        vids[i] = strdup(token);
    }
}
#define _CONFIG_BUF_SIZE        150

#endif
int read_config(char *file)
{
    FILE *in;
#ifdef AEI_VDSL_CUSTOMER_NCS    
    char buffer[_CONFIG_BUF_SIZE], *token, *line;
#else
    char buffer[80], *token, *line;
#endif
    struct iface_config_t *iface;
#ifdef DHCP_RELAY
    int relayEnabled = 0;
#endif

    /* Release all interfaces */
    cur_iface = iface_config;
    while (cur_iface) {
        iface = cur_iface->next;
        release_iface_config(cur_iface);
        cur_iface = iface;
    }
    iface_config = cur_iface = NULL;

    /* Reset server config to defaults */
    set_server_config_defaults();

    /* Allocate the first interface config */
    iface_config = cur_iface = calloc(1, sizeof(struct iface_config_t));

    if (!(in = fopen(file, "r"))) {
        LOG(LOG_ERR, "unable to open config file: %s", file);
        return 0;
    }
#ifdef DHCP_RELAY
#if 0                           // For single interface
    relay_remote = 0;
#endif
#endif
    /* Read lines */
#ifdef AEI_VDSL_CUSTOMER_NCS
    while (fgets(buffer, _CONFIG_BUF_SIZE, in)) {
#else
    while (fgets(buffer, 150, in)) {
#endif
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = '\0';
        if (strchr(buffer, '#'))
            *(strchr(buffer, '#')) = '\0';
        token = buffer + strspn(buffer, " \t");
        if (*token == '\0')
            continue;
        line = token + strcspn(token, " \t=");
        if (*line == '\0')
            continue;
        *line = '\0';
        line++;
        line = line + strspn(line, " \t=");
        if (*line == '\0')
            continue;

        if (strcasecmp(token, "interface") == 0) {
            /* Read interface name */
            char *iface_name = NULL;
            read_str(line, &iface_name);
            if (!iface_name)
                continue;
            /* Lookup read interfaces. If this interface already
             * read, ignore it */
            for (iface = iface_config; iface; iface = iface->next) {
                if (iface->interface && strcmp(iface->interface, iface_name) == 0) {
                    free(iface_name);
                    iface_name = NULL;
                    break;
                }
            }
            if (iface_name == NULL)
                continue;
            /* Assign the interface name to the first iface */
            if (cur_iface->interface == NULL)
                cur_iface->interface = iface_name;
            /* Finish the current iface, start a new one */
            else {
                iface = calloc(1, sizeof(struct iface_config_t));
                iface->interface = iface_name;
                cur_iface->next = iface;
                cur_iface = iface;
            }
        } else if (strcasecmp(token, "start") == 0)
            read_ip(line, &cur_iface->start);
        else if (strcasecmp(token, "end") == 0)
            read_ip(line, &cur_iface->end);
#if defined(AEI_VDSL_CUSTOMER_TELUS)
        else if (strcasecmp(token, "opt67_warrant_vid") == 0)
            read_vids(line, cur_iface->opt67WarrantVids, VENDOR_CLASS_ID_TAB_SIZE);
#endif
#if defined(AEI_VDSL_DHCP_LEASE)
        else if (strcasecmp(token, "stb_vid") == 0)
            read_vids(line, cur_iface->stbVids, VENDOR_CLASS_ID_TAB_SIZE);
#endif

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) //add william 2012-4-25
		else if (strcasecmp(token, "dhcpvlanoption60") == 0) 
			AEI_read_dhcpvlanoption60(line, cur_iface);	
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
        else if (strcasecmp(token, "vendorClassIdMinAddress") == 0)
            read_ip(line, &cur_iface->vendorClassIdMinAddress);
        else if (strcasecmp(token, "vendorClassIdMaxAddress") == 0)
            read_ip(line, &cur_iface->vendorClassIdMaxAddress);
		else if (strcasecmp(token, "vlanoption60") == 0) //add william 2012-1-10
			AEI_read_vlanoption60(line, cur_iface);	
#endif
#ifdef AEI_VDSL_CUSTOMER_QWEST
        else if (strcasecmp(token, "dns_proxy") == 0)
            read_ip(line, &cur_iface->dns_proxy_ip);
        else if (strcasecmp(token, "dns_passthrough") == 0)
            read_str(line, &cur_iface->dns_passthru_chaddr);
#endif
        else if (strcasecmp(token, "option") == 0 || strcasecmp(token, "opt") == 0) {
#ifdef AEI_VDSL_CUSTOMER_QWEST
            if (strstr(line, "dns"))
                read_dns_opt(line, &cur_iface->dns_srv_ips);
#endif

            read_opt(line, &cur_iface->options);
        } else if (strcasecmp(token, "max_leases") == 0)
            read_u32(line, &cur_iface->max_leases);
        else if (strcasecmp(token, "remaining") == 0)
            read_yn(line, &server_config.remaining);
        else if (strcasecmp(token, "auto_time") == 0)
            read_u32(line, &server_config.auto_time);
        else if (strcasecmp(token, "decline_time") == 0)
            read_u32(line, &server_config.decline_time);
        else if (strcasecmp(token, "conflict_time") == 0)
            read_u32(line, &server_config.conflict_time);
        else if (strcasecmp(token, "offer_time") == 0)
            read_u32(line, &server_config.offer_time);
        else if (strcasecmp(token, "min_lease") == 0)
            read_u32(line, &server_config.min_lease);
        else if (strcasecmp(token, "lease_file") == 0)
            read_str(line, &server_config.lease_file);
        else if (strcasecmp(token, "pidfile") == 0)
            read_str(line, &server_config.pidfile);
        else if (strcasecmp(token, "notify_file") == 0)
            read_str(line, &server_config.notify_file);
        else if (strcasecmp(token, "siaddr") == 0)
            read_ip(line, &cur_iface->siaddr);
        else if (strcasecmp(token, "sname") == 0)
            read_str(line, &cur_iface->sname);
        else if (strcasecmp(token, "boot_file") == 0)
            read_str(line, &cur_iface->boot_file);
        else if (strcasecmp(token, "static_lease") == 0)
            read_staticlease(line, &cur_iface->static_leases);
        else if (strcasecmp(token, "vendor_id") == 0) {
            vendor_id_t *new = malloc(sizeof(vendor_id_t));
            new->next = NULL;
            new->len = read_qstr(line, new->id, sizeof(new->id));
            if (new->len > 0) {
                if (cur_iface->vendor_ids == NULL) {
                    cur_iface->vendor_ids = new;
                } else {
                    vendor_id_t *vid;

                    for (vid = cur_iface->vendor_ids; vid->next; vid = vid->next) ;
                    vid->next = new;
                }
            } else
                free(new);
        } else if (strcasecmp(token, "decline_file") == 0)
            read_str(line, &server_config.decline_file);
        else if (strcasecmp(token, "decline") == 0)
            cur_iface->decline = 1;
#ifdef DHCP_RELAY
        else if (strcasecmp(token, "relay") == 0) {
            relayEnabled = 1;

#if 0                           // For single interface
            read_ip(line, &relay_remote);
#else
            read_ip(line, &cur_iface->relay_remote);
#endif
        }
#endif
        else
            LOG(LOG_WARNING, "unknown keyword '%s'", token);
    }
    fclose(in);

    /* Set default interface name if it's missing */
    if (iface_config->interface == NULL)
        iface_config->interface = strdup("eth0");

    /* Finish interface config automatically */
    if (!set_iface_config_defaults())
        exit_server(1);

#ifdef DHCP_RELAY
    set_relays();
    if (relayEnabled) {
        register_message(CMS_MSG_WAN_CONNECTION_UP);
    }
#endif

    return 1;
}

static long AEI_getUpTime_dhcpd()
{
    FILE *fp=NULL;
    float time1, time2;

    fp = fopen("/proc/uptime", "r");
    if( !fp )
        return time(0);

    fscanf(fp, "%f %f\n", &time1, &time2);
    fclose(fp);

    return (long)(time1/1);
}

/* the dummy var is here so this can be a signal handler */
void write_leases(int dummy __attribute__ ((unused)))
{
    FILE *fp;
    unsigned int i;
    char buf[255];
    time_t curr = time(0);
    unsigned long lease_time;
    struct iface_config_t *iface;

    dummy = 0;

    if (!(fp = fopen(server_config.lease_file, "w"))) {
        LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
        return;
    }

    for (iface = iface_config; iface; iface = iface->next) {
        for (i = 0; i < iface->max_leases; i++) {
            if (iface->leases[i].yiaddr != 0) {
                if (server_config.remaining) {
                    if (lease_expired(&(iface->leases[i])))
                        lease_time = 0;
                    else
                        lease_time = iface->leases[i].expires - curr;
                } else
                    lease_time = iface->leases[i].expires;
                lease_time = htonl(lease_time);
                fwrite(iface->leases[i].chaddr, 16, 1, fp);
                fwrite(&(iface->leases[i].yiaddr), 4, 1, fp);
                fwrite(&lease_time, 4, 1, fp);
                //BRCM: hostname field is used by dproxy
                fwrite(iface->leases[i].hostname, sizeof(iface->leases[i].hostname), 1, fp);
#if defined(AEI_VDSL_DHCP_LEASE)
                char interface[32];

                memset(interface, 0, sizeof(interface));
                if(iface->interface && strlen(iface->interface))
                    strlcpy(interface, iface->interface, sizeof(interface));

                fwrite(interface, sizeof(interface), 1, fp);
                fwrite(&(iface->leases[i].is_stb), 4, 1, fp);
#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
                fwrite(&(iface->leases[i].isWP), 4, 1, fp);
#endif
#if defined(AEI_VDSL_CUSTOMER_TELUS)
                fwrite(iface->leases[i].oui, sizeof(iface->leases[i].oui), 1, fp);
                fwrite(iface->leases[i].serialNumber, sizeof(iface->leases[i].serialNumber), 1, fp);
                fwrite(iface->leases[i].productClass, sizeof(iface->leases[i].productClass), 1, fp);
#endif
#endif
            }
        }
    }

    fclose(fp);

    if (server_config.notify_file) {
        sprintf(buf, "%s %s", server_config.notify_file, server_config.lease_file);
        system(buf);
    }
}

struct saved_lease {
    u_int8_t chaddr[16];
    u_int32_t yiaddr;           /* network order */
    u_int32_t expires;          /* host order */
    char hostname[64];
#if defined(AEI_VDSL_DHCP_LEASE)
    char layer2Interface[32];
    u_int32_t is_stb;
#endif
#if defined(AEI_VDSL_CUSTOMER_TELUS)
    char oui[8];
    char serialNumber[64];
    char productClass[64];
#endif
#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
    u_int32_t isWP;
#endif
};

void read_leases(char *file)
{
    FILE *fp;
    time_t curr = time(0);
    struct saved_lease lease;
    struct iface_config_t *iface;
    int count = 0;

    if (!(fp = fopen(file, "r"))) {
        LOG(LOG_ERR, "Unable to open %s for reading", file);
        return;
    }

    while ((fread(&lease, sizeof lease, 1, fp) == 1)) {
        for (iface = iface_config; iface; iface = iface->next) {
            if (lease.yiaddr >= iface->start && lease.yiaddr <= iface->end && iface->cnt_leases < iface->max_leases) {
#if defined(AEI_VDSL_DHCP_LEASE)
                if (iface == cur_iface) {
#if defined(AEI_VDSL_STB_NO_FIREWALL)
                    if (lease.is_stb) {
                        add_stb_to_list(lease.chaddr, lease.yiaddr);
                    }
#endif
                    iface->leases[cur_iface->cnt_leases].is_stb = lease.is_stb;
#if defined(AEI_VDSL_CUSTOMER_TELUS)
                    memcpy(iface->leases[cur_iface->cnt_leases].oui, lease.oui, sizeof(lease.oui));
                    memcpy(iface->leases[cur_iface->cnt_leases].serialNumber, lease.serialNumber, sizeof(lease.serialNumber));
                    memcpy(iface->leases[cur_iface->cnt_leases].productClass, lease.productClass, sizeof(lease.productClass));
#endif
                }
#endif

                iface->leases[cur_iface->cnt_leases].yiaddr = lease.yiaddr;
                iface->leases[cur_iface->cnt_leases].expires = ntohl(lease.expires);
 
                if (server_config.remaining)
                    iface->leases[cur_iface->cnt_leases].expires += curr;
                memcpy(iface->leases[cur_iface->cnt_leases].chaddr, lease.chaddr, sizeof(lease.chaddr));
                memcpy(iface->leases[cur_iface->cnt_leases].hostname, lease.hostname, sizeof(lease.hostname));

#if defined(AEI_VDSL_CUSTOMER_NCS)
                struct static_lease *static_lease;               

                static_lease = AEI_getLeaseByIp(iface->static_leases, lease.yiaddr);
                if (static_lease && (memcmp(static_lease->mac, lease.chaddr, 6) != 0))
                {
                    memset(&(iface->leases[cur_iface->cnt_leases]), 0, sizeof(struct dhcpOfferedAddr));
                    break;
                }
#endif

                iface->cnt_leases++;
                count++;
                break;
            }
        }
    }

    DEBUG(LOG_INFO, "Read %d leases", count);

    fclose(fp);
}

// BRCM_begin
void send_lease_info(UBOOL8 isDelete, const struct dhcpOfferedAddr *lease)
{
    char buf[sizeof(CmsMsgHeader) + sizeof(DhcpdHostInfoMsgBody)] = { 0 };
    CmsMsgHeader *hdr = (CmsMsgHeader *) buf;
    DhcpdHostInfoMsgBody *body = (DhcpdHostInfoMsgBody *) (hdr + 1);
    CmsRet ret;
    struct in_addr inaddr;
    UINT32 remaining, now;

    inaddr.s_addr = lease->yiaddr;

#if defined(AEI_VDSL_CUSTOMER_NCS)
    write_leases(0);
#endif

#if defined(AEI_VDSL_DHCP_LEASE)
    remaining = lease->expires;
#else
    if (lease->expires == 0xffffffff) { /* check if expires == -1 */
        remaining = lease->expires;
    } else {
        now = time(0);
        if (lease->expires < now) {
            remaining = 0;
        } else {
            remaining = lease->expires - now;
            /*
             * dhcpd is reporting remaining time to ssk, which sticks it into
             * the data model.  The data model expects a SINT32, so make sure
             * our UINT32 remaining does not go above MAX_SINT32.
             */
            if (remaining > MAX_SINT32) {
                remaining = MAX_SINT32;
            }
        }
    }
#endif

    DEBUG(LOG_INFO, "sending lease info update msg, isDelete=%d, leaseTimeRemaining=%d", isDelete, remaining);
    DEBUG(LOG_INFO, "assigned addr = %s", inet_ntoa(inaddr));

    hdr->type = CMS_MSG_DHCPD_HOST_INFO;
    hdr->src = EID_DHCPD;
    hdr->dst = EID_SSK;
    hdr->flags_event = 1;
    hdr->dataLength = sizeof(DhcpdHostInfoMsgBody);

    body->deleteHost = isDelete;
    body->leaseTimeRemaining = (SINT32) remaining;
    /**
     * @file files.c
     * @Synopsis  
     * @author Mathias Lorente
     * @version 
     * @date 2011-11-11
     */

    snprintf(body->ifName, sizeof(body->ifName), cur_iface->interface);
    snprintf(body->ipAddr, sizeof(body->ipAddr), inet_ntoa(inaddr));
    snprintf(body->hostName, sizeof(body->hostName), lease->hostname);
    cmsUtl_macNumToStr(lease->chaddr, body->macAddr);
#if defined(AEI_VDSL_CUSTOMER_NCS)
    body->icon = lease->icon;
    //printf("msg icon:%d\n", lease->icon);
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
    snprintf(body->venderClassID, sizeof(body->venderClassID), lease->vendorid);
    snprintf(body->userClassID, sizeof(body->userClassID), lease->classid);
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) || defined(AEI_VDSL_CUSTOMER_TELUS)
    snprintf(body->clientID, sizeof(body->clientID), lease->clientid);
#endif
#if defined(AEI_VDSL_DHCP_LEASE)
    body->isStb = is_stb(lease->vendorid);
#endif
#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
    body->isWP = lease->isWP;
    if (lease->isWP)
    {
        strlcpy(body->WPProductType,lease->WPProductType,sizeof(body->WPProductType));
        strlcpy(body->WPFirmwareVersion,lease->WPFirmwareVersion,sizeof(body->WPFirmwareVersion));
        strlcpy(body->WPProtocolVersion,lease->WPProtocolVersion,sizeof(body->WPProtocolVersion));
    }
#endif
    /* does DHCP include the statically assigned addresses?  Or should that be STATIC? */
    snprintf(body->addressSource, sizeof(body->addressSource), MDMVS_DHCP);

    /* is there a way we can tell if we assigned this address to a host on WLAN? */
    snprintf(body->interfaceType, sizeof(body->interfaceType), MDMVS_ETHERNET);

    /* the vendor id is also contained in the lease struct,
     * we could also send that to ssk to put into the host entry. */

    snprintf(body->oui, sizeof(body->oui), lease->oui);
    snprintf(body->serialNum, sizeof(body->serialNum), lease->serialNumber);
    snprintf(body->productClass, sizeof(body->productClass), lease->productClass);

    if ((ret = cmsMsg_send(msgHandle, hdr)) != CMSRET_SUCCESS) {
        LOG(LOG_WARNING, "could not send lease info update");
    } else {
        DEBUG(LOG_INFO, "lease info update sent!");
    }
#ifdef AEI_VDSL_CUSTOMER_VIDEO_GUARANTEE
    // also send this msg to CPU-monitor (/bin/statd)
    hdr->dst = EID_CPU_MONITOR;
    if ((ret = cmsMsg_send(msgHandle, hdr)) != CMSRET_SUCCESS) {
        LOG(LOG_WARNING, "could not send lease info update");
    } else {
        DEBUG(LOG_INFO, "lease info update sent to statd!");
    }
#endif
}

void write_decline(void)
{
    FILE *fp;
    char msg[sizeof(CmsMsgHeader) + sizeof(DHCPDenyVendorID)] = { 0 };
    CmsMsgHeader *hdr = (CmsMsgHeader *) & msg;
    DHCPDenyVendorID *vid = (DHCPDenyVendorID *) (&msg[sizeof(CmsMsgHeader)]);

    /* Write a log to console */
    printf("Denied vendor ID \"%s\", MAC=%02x:%02x:%02x:%02x:%02x:%02x Interface=%s\n",
           declines->vendorid, declines->chaddr[0], declines->chaddr[1],
           declines->chaddr[2], declines->chaddr[3], declines->chaddr[4], declines->chaddr[5], cur_iface->interface);
    fflush(stdout);

    if (!(fp = fopen(server_config.decline_file, "w"))) {
        LOG(LOG_ERR, "Unable to open %s for writing", server_config.decline_file);
        return;
    }

    fwrite(declines->chaddr, 16, 1, fp);
    fwrite(declines->vendorid, 64, 1, fp);
    fclose(fp);

    /*
     * Send an event msg to ssk.
     */
    hdr->type = CMS_MSG_DHCPD_DENY_VENDOR_ID;
    hdr->src = EID_DHCPD;
    hdr->dst = EID_SSK;
    hdr->flags_event = 1;
    hdr->dataLength = sizeof(DHCPDenyVendorID);
    vid->deny_time = time(NULL);
    memcpy(vid->chaddr, declines->chaddr, sizeof(vid->chaddr));
    strncpy(vid->vendor_id, declines->vendorid, sizeof(vid->vendor_id) - 1);
    strncpy(vid->ifName, cur_iface->interface, sizeof(vid->ifName) - 1);
    cmsMsg_send(msgHandle, hdr);
}

static struct dhcpOfferedAddr *find_expired_lease_by_yiaddr(u_int32_t yiaddr)
{
    struct iface_config_t *iface;

    for (iface = iface_config; iface; iface = iface->next) {
        unsigned int i;
        for (i = 0; i < iface->max_leases; i++) {
            if (iface->leases[i].yiaddr == yiaddr) {
#if defined(AEI_VDSL_DHCP_LEASE)
                if (iface->leases[i].expires == 0)
#else
                if (iface->leases[i].expires > (unsigned long)time(0))
#endif
                    return &(iface->leases[i]);
                else
                    return NULL;
            }
        }
    }
    return NULL;
}

/* get signal to write viTable to file */
void write_viTable(int dummy)
{
    FILE *fp;
    int count;
    pVI_OPTION_INFO pPtr = NULL;

    if (!(fp = fopen("/var/udhcpd/managable.device", "w+"))) {
        LOG(LOG_ERR, "Unable to open %s for writing", "/var/udhcpd/managable.device");
        return;
    }
    count = viList->count;
    fprintf(fp, "NumberOfDevices %d\n", count);
    if (count > 0) {
        pPtr = viList->pHead;
        while (pPtr) {
            if (find_expired_lease_by_yiaddr(pPtr->ipAddr)) {
                strcpy(pPtr->oui, "");
                strcpy(pPtr->serialNumber, "");
                strcpy(pPtr->productClass, "");
            }
            fprintf(fp, "IPaddr %x Enterprise %d OUI %s SerialNumber %s ProductClass %s\n",
                    pPtr->ipAddr, pPtr->enterprise, pPtr->oui, pPtr->serialNumber, pPtr->productClass);
            pPtr = pPtr->next;
        }
    }
    fclose(fp);
}

#ifdef DHCP_RELAY
/* Register interested message to smd to receive it later */
static void register_message(CmsMsgType msgType)
{
    CmsMsgHeader msg;
    CmsRet ret;

    memset(&msg, 0, sizeof(msg));
    msg.type = CMS_MSG_REGISTER_EVENT_INTEREST;
    msg.src = EID_DHCPD;
    msg.dst = EID_SMD;
    msg.flags_request = 1;
    msg.wordData = msgType;

    ret = cmsMsg_sendAndGetReply(msgHandle, &msg);
    if (ret != CMSRET_SUCCESS) {
        cmsLog_error("register_message(%d) error (%d)", msgType, ret);
    } else {
        cmsLog_debug("register_message(%d) succeeded", msgType);
    }

    return;
}
#endif                          /* DHCP_RELAY */

// BRCM_end
