/*
 *
 * Constuct and send DHCP server packets
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"
#include "static_leases.h"

#if defined(AEI_VDSL_DHCP_LEASE)
extern int ip_changed;
#endif

#if defined(AEI_VDSL_CUSTOMER_TELUS)
/*
 * check the ip is in the lease range
 */
static int is_in_range(int vendor, u_int32_t req_align)
{

    if (vendor == 1) {
        if (ntohl(req_align) >= ntohl(cur_iface->vendorClassIdMinAddress) &&
            ntohl(req_align) <= ntohl(cur_iface->vendorClassIdMaxAddress))
            return 1;
        else
            return 0;
    }
    if (vendor == 2) {
        if (ntohl(req_align) >= ntohl(cur_iface->start) && ntohl(req_align) <= ntohl(cur_iface->end))
            return 1;
    }
    return 0;
}

static UBOOL8 wantOption(struct dhcpMessage *oldpacket, int option)
{
    int i, len;
    char *param_list = NULL;
    UBOOL8 found = FALSE;

    if (oldpacket) {
        param_list = get_option(oldpacket, DHCP_PARAM_REQ);
        if (param_list) {
            len = *(param_list - 1);
            for (i = 0; i < len; i++) {
                if (*(param_list + i) == option) {
                    found = TRUE;
                    break;
                }
            }
        }
    }
    return found;
}

/* This function is written for TELUS DHCP option 67 requirement
 * to identify STB clients, normally if you want to identify STB 
 * clients in dhcpd, you should alway use is_stb() instead.
 */
static UBOOL8 is_IPTVSTB(struct dhcpMessage *oldpacket)
{
    int i;
    UBOOL8 stb = FALSE;
    int len = 0;
    char vendorid[256] = { 0 };

    if (oldpacket) {
        unsigned char *vid = get_option(oldpacket, DHCP_VENDOR);

        if (vid) {
            len = *(vid - 1);
            memcpy(vendorid, vid, (size_t) len);

            for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
                if (cur_iface->opt67WarrantVids[i] && !AEI_wstrcmp(cur_iface->opt67WarrantVids[i], vendorid)) {
                    stb = TRUE;
                    break;
                }
            }
        }
    }
    return stb;
}
#endif

#if defined(AEI_VDSL_DHCP_LEASE)
UBOOL8 is_stb(const char *vid)
{
    int i;
    UBOOL8 stb = FALSE;

    if (vid && *vid) {
        for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
            if (cur_iface->stbVids[i] && !AEI_wstrcmp(cur_iface->stbVids[i], vid)) {
                stb = TRUE;
                break;
            }
        }
    }

    return stb;
}
#endif

#if defined(AEI_VDSL_WP)&& defined(AEI_VDSL_DHCP_LEASE)
UBOOL8 is_WP(const char *vid)
{
    int i;
    UBOOL8 WP = FALSE;

    if (vid && *vid && !AEI_wstrcmp(AEI_OPT60_WP_STRING, vid))
        WP = TRUE;

    return WP;
}
#endif



#if defined(AEI_VDSL_STB_NO_FIREWALL)
#define NIPQUID(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

void do_mark(u_int32_t ip, UBOOL8 mark)
{
    char cmd[256];

    if (mark)
    {
        sprintf(cmd, "iptables -I INPUT 1 -i br0 -p UDP --dport 53 -s %u.%u.%u.%u -j ACCEPT", NIPQUID(ip));
        system(cmd);
        sprintf(cmd, "iptables -I FORWARD 1 -s %u.%u.%u.%u -j ACCEPT", NIPQUID(ip));
        system(cmd);
        sprintf(cmd, "iptables -I FORWARD 1 -d %u.%u.%u.%u -j ACCEPT", NIPQUID(ip));
        system(cmd);
    }
    else
    {
        sprintf(cmd, "iptables -D INPUT -i br0 -p UDP --dport 53 -s %u.%u.%u.%u -j ACCEPT", NIPQUID(ip));
        system(cmd);
        sprintf(cmd, "iptables -D FORWARD -s %u.%u.%u.%u -j ACCEPT", NIPQUID(ip));
        system(cmd);
        sprintf(cmd, "iptables -D FORWARD -d %u.%u.%u.%u -j ACCEPT", NIPQUID(ip));
        system(cmd);
    }
}

void do_all_mark(UBOOL8 mark)
{
    struct ip_mac_list *stb;

    for (stb = cur_iface->stb_list; stb; stb = stb->next)
    {
        do_mark(stb->ip, mark);
    }
}

struct ip_mac_list* find_stb_by_chaddr(u_int8_t *mac)
{
    struct ip_mac_list *stb;

    if (mac == NULL)
        return NULL;

    for (stb = cur_iface->stb_list; stb; stb = stb->next)
    {
        if (!memcmp(stb->mac, mac, 16))
            return stb;
    }

    return NULL;
}

void del_stb_from_list(u_int8_t *mac, u_int32_t ip)
{
    struct ip_mac_list *stb, *pre, *next;
    UBOOL8 found = FALSE;

    if (mac == NULL)
        return;

    stb = cur_iface->stb_list;
    pre = next = NULL;

    while (stb)
    {
        if (!memcmp(stb->mac, mac, 16) &&
            (stb->ip == ip))
        {
            found = TRUE;
        }

        next = stb->next;
        if (found)
        {
            if (!pre)
                cur_iface->stb_list = next;
            else
                pre->next = next;

            do_mark(stb->ip, FALSE);
            free(stb);
            break;
        }
        else
        {
            pre = stb;
            stb = next;
        }
    }
}

void add_stb_to_list(u_int8_t *mac, u_int32_t ip)
{
    unsigned int i;
    struct ip_mac_list *stb;

    if (mac == NULL)
        return;

    for (i = 0; i < 16 && !mac[i]; i++);
    /* blank mac */
    if (i == 16)
        return;

    stb = find_stb_by_chaddr(mac);
    if (stb == NULL)
    {
        stb = calloc(1, sizeof(struct ip_mac_list));
        if (stb)
        {
            stb->ip = ip;
            memcpy(stb->mac, mac, 16);
            stb->next = cur_iface->stb_list;
            cur_iface->stb_list = stb;

            do_mark(stb->ip, TRUE);
        }
    }
    else
    {
        if (stb->ip != ip)
        {
            do_mark(stb->ip, FALSE);
            stb->ip = ip;
            do_mark(stb->ip, TRUE);
        }
    }
}
#endif

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
    DEBUG(LOG_INFO, "Forwarding packet to relay");

    return kernel_packet(payload, cur_iface->server, SERVER_PORT, payload->giaddr, SERVER_PORT);
}

/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
    u_int32_t ciaddr;
    char chaddr[6];

    if (force_broadcast) {
        DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
        ciaddr = INADDR_BROADCAST;
        memcpy(chaddr, MAC_BCAST_ADDR, 6);
    } else if (payload->ciaddr) {
        DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
        ciaddr = payload->ciaddr;
        memcpy(chaddr, payload->chaddr, 6);
    } else if (ntohs(payload->flags) & BROADCAST_FLAG) {
        DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
        ciaddr = INADDR_BROADCAST;
        memcpy(chaddr, MAC_BCAST_ADDR, 6);
    } else {
        DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
        ciaddr = payload->yiaddr;
        memcpy(chaddr, payload->chaddr, 6);
    }
    return raw_packet(payload, cur_iface->server, SERVER_PORT, ciaddr, CLIENT_PORT, chaddr, cur_iface->ifindex);
}

/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
    int ret;

    if (payload->giaddr)
        ret = send_packet_to_relay(payload);
    else
        ret = send_packet_to_client(payload, force_broadcast);
    return ret;
}

static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
    memset(packet, 0, sizeof(struct dhcpMessage));

    packet->op = BOOTREPLY;
    packet->htype = ETH_10MB;
    packet->hlen = ETH_10MB_LEN;
    packet->xid = oldpacket->xid;
    memcpy(packet->chaddr, oldpacket->chaddr, 16);
    packet->cookie = htonl(DHCP_MAGIC);
    packet->options[0] = DHCP_END;
    packet->flags = oldpacket->flags;
    packet->giaddr = oldpacket->giaddr;
    packet->ciaddr = oldpacket->ciaddr;
    add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
    add_simple_option(packet->options, DHCP_SERVER_ID, ntohl(cur_iface->server));       /* expects host order */
}

/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
    packet->siaddr = cur_iface->siaddr;
    if (cur_iface->sname)
        strncpy(packet->sname, cur_iface->sname, sizeof(packet->sname) - 1);
    if (cur_iface->boot_file)
        strncpy(packet->file, cur_iface->boot_file, sizeof(packet->file) - 1);
}

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
static int is_dhcpvlan_vendor(char *ip,char *optionvendor) //add william 2012-4-25
{
	struct dhcpvlanOption60 * curr = cur_iface->dhcpvlanOption60list;
	int i=0;
	char tempip[16]={0};
	while (curr) 
	{
		//LOG(LOG_ERR, "william->is_dhcpvlan_vendor vendorClassId (%s) optionvendor(%s) ip(%s)\n",curr->vendorClassId,optionvendor,ip);
        if(AEI_wstrcmp(curr->vendorClassId,optionvendor)==0) 
		{
                    /*Coverity Fix CID 12260:Copy into fixed size buffer*/
                    strlcpy(tempip,ip,sizeof(tempip));
                    sendDhcpVlanUpdatedMsg(tempip,curr->vlanID);
                    break;
		}
    
        curr = curr->next;
	}
	return 0;
}

static UBOOL8 is_IPTVSTB(struct dhcpMessage *oldpacket)
{
    char *vendorid = NULL;
    UBOOL8 stb = FALSE;

    vendorid = get_option(oldpacket, DHCP_VENDOR);

    if (vendorid && !strncmp(vendorid, "MSFT_IPTV", strlen("MSFT_IPTV"))) {
        stb = TRUE;
        //      printf("option 60 is %s\n", vendorid);
    }
    return stb;
}

/* check the ip is in the lease range */
static int is_in_range(struct dhcpMessage *oldpacket, u_int32_t req_align)
{
    if (is_IPTVSTB(oldpacket)) {
        if (ntohl(req_align) >= ntohl(cur_iface->vendorClassIdMinAddress) &&
            ntohl(req_align) <= ntohl(cur_iface->vendorClassIdMaxAddress))
            return 1;
    } else {
        if (ntohl(req_align) >= ntohl(cur_iface->start) && ntohl(req_align) <= ntohl(cur_iface->end))
            return 1;
    }
    return 0;
}
#endif

#ifdef AEI_VDSL_CUSTOMER_QWEST
void custom_dns_option(struct dhcpMessage *packet, struct option_set *curr)
{
    unsigned char data[256];
    char mac_addr[20] = { 0 };
    struct ip_list *p;
    int len = 0;

    memset(data, 0, sizeof(data));

    if (cur_iface->dns_proxy_ip != 0 && !IS_EMPTY_STRING(cur_iface->dns_passthru_chaddr)) {
        cmsUtl_macNumToStr(packet->chaddr, mac_addr);

        if (strcasecmp(cur_iface->dns_passthru_chaddr, mac_addr) == 0) {
            for (p = cur_iface->dns_srv_ips; p; p = p->next) {
                if (p->ip != cur_iface->dns_proxy_ip && sizeof(u_int32_t) <= sizeof(data) - len) {
                    memcpy(data + len, &p->ip, sizeof(u_int32_t));
                    len += sizeof(u_int32_t);
                }
            }
        } else {
            memcpy(data, &cur_iface->dns_proxy_ip, sizeof(u_int32_t));
            len += sizeof(u_int32_t);
        }
    } else {
        if (cur_iface->dns_proxy_ip != 0) {
            memcpy(data, &cur_iface->dns_proxy_ip, sizeof(u_int32_t));
            len += sizeof(u_int32_t);
        }

        for (p = cur_iface->dns_srv_ips; p; p = p->next) {
            if (p->ip != cur_iface->dns_proxy_ip && sizeof(u_int32_t) <= sizeof(data) - len) {
                memcpy(data + len, &p->ip, sizeof(u_int32_t));
                len += sizeof(u_int32_t);
            }
        }
    }

    if (curr->data)
        free(curr->data);

    curr->data = malloc(len + 2);
    curr->data[OPT_CODE] = DHCP_DNS_SERVER;
    curr->data[OPT_LEN] = len;
    memcpy(curr->data + 2, data, len);
}
#endif

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
#define ADVANCED_DMZ_LEASE_TIME 600

/* important don't change curr->data since that holds interface options, just pass something else to add_option_string */
void AEI_modify_option_string(struct static_lease *special_lease, struct dhcpMessage *packet, struct option_set *curr)
{
	unsigned char data[308];
	unsigned char *data2 = data + 2;
	int len = 0;
	int code = 0;

	if (!curr || !curr->data)
		return;

	/* if not special lease, then just regular options so just add it regularly */
	if (!special_lease) {
		add_option_string(packet->options, curr->data);
		return;
	}

	memset(data, 0, sizeof(data));
	data[OPT_CODE] = curr->data[OPT_CODE];

	switch(data[OPT_CODE])
	{
		case DHCP_DNS_SERVER:
			if (special_lease->dns1 != 0) {
				memcpy(data2, &special_lease->dns1, sizeof(u_int32_t));
				len += sizeof(u_int32_t);
			}

			if (special_lease->dns2 != 0) {
				memcpy(data2 + len, &special_lease->dns2, sizeof(u_int32_t));
				len += sizeof(u_int32_t);
			}
		break;
		case DHCP_SUBNET:
			if (special_lease->subnet != 0) {
				memcpy(data2, &special_lease->subnet, sizeof(u_int32_t));
				len += sizeof(u_int32_t);
			}
		break;
		case DHCP_ROUTER:
			if (special_lease->gw != 0) {
				memcpy(data2, &special_lease->gw, sizeof(u_int32_t));
				len += sizeof(u_int32_t);
				//cmsLog_error("--------old router 0x%x 0x%x 0x%x------------",cur_iface->server,packet->siaddr, packet->giaddr);
				packet->siaddr = cur_iface->server;
			}
		break;
		default:
		break;
	}
         
	if (len==0)
	{
		//if len 0, data not changed, this should cover regular static lease
		add_option_string(packet->options, curr->data);
	}
	else
	{
		data[OPT_LEN] = len;
		add_option_string(packet->options, data);
	}
}


#endif


/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
    struct dhcpMessage packet;
    struct dhcpOfferedAddr *lease = NULL;
    u_int32_t req_align, lease_time_align = cur_iface->lease;
    char *req, *lease_time;
    struct option_set *curr;
    struct in_addr addr;
    //For static IP lease
    uint32_t static_lease_ip;
    //brcm begin
    char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
    //brcm end
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	struct static_lease *special_lease = NULL;
#endif

	#if defined(AEI_VDSL_CUSTOMER_NCS) //add william 2012-1-11
	    /* */
        int vendor;
        char vendorid[VENDOR_CLASS_ID_STR_SIZE + 1];
        vendorid[0] = '\0';
        /* */

        vendor = AEI_is_vendor_equipped(oldpacket, vendorid);
	
        switch (vendor) {
        case 1:
            /* client discover match in the server config */
#if 0 //debug
            LOG(LOG_ERR, "(debug message) DHCP server equipped the"
                " vendorClassId:'%s' MAC=%02X:%02X:%02X:%02X:%02X:%02X\n"
                "Ignore this client DHCP DISCOVERY in the local server.",
                vendorid,
                oldpacket->chaddr[0],
                oldpacket->chaddr[1],
                oldpacket->chaddr[2],
                oldpacket->chaddr[3],
                oldpacket->chaddr[4],
                oldpacket->chaddr[5],
                oldpacket->chaddr[6]);
#endif
            return 1;
        default:
            /*
             * == 0 client discover not match in the server config
             * == 2 client send a DHCP discover not include option-60
             */
            break;
        }
#endif

    init_packet(&packet, oldpacket, DHCPOFFER);

    //For static IP lease
    static_lease_ip = getIpByMac(cur_iface->static_leases, oldpacket->chaddr);
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	special_lease = AEI_getLeaseByMac(cur_iface->static_leases,
		oldpacket->chaddr);
#endif

    if (!static_lease_ip) {
        /* the client is in our lease/offered table */
#if defined(AEI_VDSL_CUSTOMER_NCS)
        lease = find_lease_by_chaddr(oldpacket->chaddr);

        /* if this IP address has already been assigned to other client as a static lease,
         * we than need to assign a new free IP address to this client */
        if (lease && !reservedIp(cur_iface->static_leases, lease->yiaddr))
#else
        if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) 
#endif
        {
#if defined(AEI_VDSL_DHCP_LEASE)
            if (!lease_expired(lease))
            {
                ip_changed = 0;
                lease_time_align = lease->expires;
            }
#else
            if (!lease_expired(lease))
                lease_time_align = lease->expires - time(0);
#endif
            packet.yiaddr = lease->yiaddr;
            /* Or the client has a requested ip */
        } else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&
                   /* Don't look here (ugly hackish thing to do) */
                   memcpy(&req_align, req, 4) &&
#if defined(AEI_VDSL_CUSTOMER_NCS)
                   (!reservedIp(cur_iface->static_leases, req_align)) &&
#endif
                   /* and the ip is in the lease range */
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                   is_in_range(oldpacket, req_align) &&
#else
                   ntohl(req_align) >= ntohl(cur_iface->start) && ntohl(req_align) <= ntohl(cur_iface->end) &&
#endif
                   /* and its not already taken/offered */
                   ((!(lease = find_lease_by_yiaddr(req_align)) ||
                     /* or its taken, but expired */
                     lease_expired(lease)))) {
            packet.yiaddr = req_align;
            /* otherwise, find a free IP */
        } else {
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
            if (is_IPTVSTB(oldpacket)) {
                packet.yiaddr = find_address_vendorid(0);
                /* try for an expired lease */
                if (!packet.yiaddr)
                    packet.yiaddr = find_address_vendorid(1);
            } else {
#endif
                packet.yiaddr = find_address(0);
                /* try for an expired lease */
                if (!packet.yiaddr)
                    packet.yiaddr = find_address(1);

#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
                /* get oldest lease ipaddress from lease tables */
                if (!packet.yiaddr)
                    packet.yiaddr = AEI_find_address();
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                if (!packet.yiaddr)
                    packet.yiaddr = AEI_find_rs_staticaddress();
            }
#endif
        }

        if (!packet.yiaddr) {
            LOG(LOG_WARNING, "no IP addresses to give -- " "OFFER abandoned");
            return -1;
        }

#if !defined(AEI_VDSL_DHCP_LEASE)
        if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
            LOG(LOG_WARNING, "lease pool is full -- " "OFFER abandoned");
            return -1;
        }
#endif

        if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
            memcpy(&lease_time_align, lease_time, 4);
            lease_time_align = ntohl(lease_time_align);
            if (lease_time_align > cur_iface->lease)
                lease_time_align = cur_iface->lease;
        }

        /* Make sure we aren't just using the lease time from the
         * previous offer */
        if (lease_time_align < server_config.min_lease)
            lease_time_align = cur_iface->lease;

    } else {
        /* It is a static lease... use it */
        packet.yiaddr = static_lease_ip;
    }

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
    /* For passthru, instead of default 1 day, use a short least time like 2Wire to trigger a 
     * renew from the 3rd-party RG to pick up new IP in case cpe get's new wan connection.
     * renewal time usually is 1/2 of LEASE time so 5 min....
     */
    if (special_lease && (special_lease->dns1 != 0 || 
        special_lease->dns2 != 0 || 
        special_lease->subnet != 0 ||
        special_lease->gw != 0)) {
        lease_time_align = ntohl(ADVANCED_DMZ_LEASE_TIME);
    }
#endif                

#if defined(AEI_VDSL_DHCP_LEASE)
    u_int32_t lease_expires;

    if (static_lease_ip)
    {
        struct dhcpOfferedAddr *tmp_lease;

        tmp_lease = find_lease_by_chaddr(packet.chaddr);
        if (tmp_lease && (tmp_lease->yiaddr == static_lease_ip))
        {
            ip_changed = 0;
        }

        lease_expires = -1;
    }
    else
        lease_expires = lease_time_align;

    if (!add_lease(packet.chaddr, packet.yiaddr, lease_expires)) {
        LOG(LOG_WARNING, "lease pool is full -- " "OFFER abandoned");
        return -1;
    }

    struct dhcpOfferedAddr *tmp_lease;
    char tmp_vendorid[256];
    char *tmp_vid;
    int tmp_len;

    tmp_lease = find_lease_by_chaddr(packet.chaddr);
    if (tmp_lease)
    {
        memset(tmp_vendorid, 0, sizeof(tmp_vendorid));
                                
        tmp_vid = get_option(oldpacket, DHCP_VENDOR);
        if (tmp_vid != NULL)
        {
            tmp_len = *(tmp_vid - 1);
            memcpy(tmp_vendorid, tmp_vid, (size_t)tmp_len);

            if (is_stb(tmp_vendorid))
            {
                tmp_lease->is_stb = TRUE;
            }
            else
            {
                tmp_lease->is_stb = FALSE;
            }
        }

#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
        if (tmp_vid != NULL)
        {  
           /* current mechanism to retain lease done at DISCOVER
              so keep that scheme and process here  
            */
           AEI_InitLeaseWP(oldpacket, tmp_lease);
        }
#endif


    }
#endif

    add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);

    curr = cur_iface->options;
    while (curr) {
        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME) {
#ifdef AEI_VDSL_CUSTOMER_QWEST
            if (curr->data[OPT_CODE] == DHCP_DNS_SERVER)
                custom_dns_option(&packet, curr);
#endif
#ifdef AEI_VDSL_CUSTOMER_TELUS
            if (curr->data[OPT_CODE] == DHCP_BOOT_FILE) {
                int i, len;
                char vendorid[256];
                char *vid;
                char *param_list;
                UBOOL8 isStb = FALSE;
                UBOOL8 sendOpt67 = FALSE;

                memset(vendorid, 0, sizeof(vendorid));

                vid = get_option(oldpacket, DHCP_VENDOR);

                if (vid != NULL) {
                    len = *(vid - 1);
                    memcpy(vendorid, vid, (size_t) len);

                    for (i = 0; i < VENDOR_CLASS_ID_TAB_SIZE; i++) {
                        if (cur_iface->opt67WarrantVids[i] && !AEI_wstrcmp(cur_iface->opt67WarrantVids[i], vendorid)) {
                            isStb = TRUE;
                            break;
                        }
                    }
                }

                if (isStb) {
                    param_list = get_option(oldpacket, DHCP_PARAM_REQ);
                    if (param_list) {
                        len = *(param_list - 1);
                        for (i = 0; i < len; i++) {
                            if (*(param_list + i) == DHCP_BOOT_FILE) {
                                sendOpt67 = TRUE;
                                break;
                            }
                        }
                    }
                }

                if (sendOpt67)
                    add_option_string(packet.options, curr->data);

                curr = curr->next;
                continue;
            }
#endif

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
            AEI_modify_option_string(special_lease, &packet, curr);
#else
            add_option_string(packet.options, curr->data);
#endif        
        }

        curr = curr->next;
    }

    add_bootp_options(&packet);

    //brcm begin
    /* if DHCPDISCOVER from client has device identity, send back gateway identity */
    if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) {
        if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY, VIinfo) != -1)
            add_option_string(packet.options, VIinfo);
    }
    //brcm end

    addr.s_addr = packet.yiaddr;
    LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
    return send_packet(&packet, 0);
}

int sendNAK(struct dhcpMessage *oldpacket)
{
    struct dhcpMessage packet;

    init_packet(&packet, oldpacket, DHCPNAK);

    DEBUG(LOG_INFO, "sending NAK");
    return send_packet(&packet, 1);
}

int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
    struct dhcpMessage packet;
    struct option_set *curr;
    struct dhcpOfferedAddr *offerlist;
    char *lease_time, *vendorid, *userclsid;
    char length = 0;
    u_int32_t lease_time_align = cur_iface->lease;
    struct in_addr addr;
    //brcm begin
    char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
    char *req;
    int saveVIoptionNeeded = 0;
    //brcm end
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	struct static_lease *special_lease = NULL;
	special_lease = AEI_getLeaseByMac(cur_iface->static_leases,
		oldpacket->chaddr);
#endif
#if defined(AEI_VDSL_DHCP_LEASE)
    uint32_t static_lease_ip;
    static_lease_ip = getIpByMac(cur_iface->static_leases, oldpacket->chaddr);
#endif
 
    init_packet(&packet, oldpacket, DHCPACK);
    packet.yiaddr = yiaddr;

    if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
        memcpy(&lease_time_align, lease_time, 4);
        lease_time_align = ntohl(lease_time_align);
        if (lease_time_align > cur_iface->lease)
            lease_time_align = cur_iface->lease;
        else if (lease_time_align < server_config.min_lease)
            lease_time_align = cur_iface->lease;
    }

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	/* For passthru, instead of default 1 day, use a short least time like 2Wire to trigger a 
	 * renew from the 3rd-party RG to pick up new IP in case cpe get's new wan connection. 
	 */
	if (special_lease && (special_lease->dns1 != 0 || 
		special_lease->dns2 != 0 || 
		special_lease->subnet != 0 ||
		special_lease->gw != 0)) {
		lease_time_align = ntohl(ADVANCED_DMZ_LEASE_TIME);
	}
#endif           

    add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);

    curr = cur_iface->options;
    while (curr) {
        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME) {
#ifdef AEI_VDSL_CUSTOMER_QWEST
            if (curr->data[OPT_CODE] == DHCP_DNS_SERVER)
                custom_dns_option(&packet, curr);
#endif
#ifdef AEI_VDSL_CUSTOMER_TELUS
            if (curr->data[OPT_CODE] == DHCP_BOOT_FILE) {
                if (is_IPTVSTB(oldpacket) && wantOption(oldpacket, DHCP_BOOT_FILE)) {
                    add_option_string(packet.options, curr->data);
                }

                curr = curr->next;
                continue;
            }
#endif

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
            AEI_modify_option_string(special_lease, &packet, curr);
#else
            add_option_string(packet.options, curr->data);
#endif
        }

        curr = curr->next;
    }

    add_bootp_options(&packet);

    //brcm begin
    /* if DHCPRequest from client has device identity, send back gateway identity,
       and save the device identify */
    if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) {
        if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY, VIinfo) != -1) {
            add_option_string(packet.options, VIinfo);
        }
        saveVIoptionNeeded = 1;
    }
    //brcm end

    addr.s_addr = packet.yiaddr;
    LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

    if (send_packet(&packet, 0) < 0)
        return -1;

#if defined(AEI_VDSL_DHCP_LEASE)
    u_int32_t lease_expires;
    
    if (static_lease_ip)
        lease_expires = -1;
    else
        lease_expires = lease_time_align;

    add_lease(packet.chaddr, packet.yiaddr, lease_expires);
#else
    add_lease(packet.chaddr, packet.yiaddr, lease_time_align);
#endif
    offerlist = find_lease_by_chaddr(packet.chaddr);
    if (saveVIoptionNeeded) {
        saveVIoption(req, offerlist);
    }

#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
    /* need to reinit as add_lease above wiped old config */
    AEI_InitLeaseWP(oldpacket, offerlist);
#endif

    vendorid = get_option(oldpacket, DHCP_VENDOR);
    userclsid = get_option(oldpacket, DHCP_USER_CLASS_ID);
    memset(offerlist->classid, 0, sizeof(offerlist->classid));
    memset(offerlist->vendorid, 0, sizeof(offerlist->vendorid));
    if (vendorid != NULL) {
        length = *(vendorid - 1);
        memcpy(offerlist->vendorid, vendorid, (size_t) length);
        offerlist->vendorid[length] = '\0';
    }

    if (userclsid != NULL) {
        length = *(userclsid - 1);
        memcpy(offerlist->classid, userclsid, (size_t) length);
        offerlist->classid[length] = '\0';
    }

#if defined(AEI_VDSL_DHCP_LEASE)
    if (is_stb(offerlist->vendorid))
    {
        offerlist->is_stb = TRUE;
#if defined(AEI_VDSL_STB_NO_FIREWALL)
        add_stb_to_list(offerlist->chaddr, offerlist->yiaddr);
#endif
    }
    else
    {
#if defined(AEI_VDSL_STB_NO_FIREWALL)
        /* change to non-stb */
        if (offerlist->is_stb)
        { 
            del_stb_from_list(offerlist->chaddr, offerlist->yiaddr);
        }
#endif

        offerlist->is_stb = FALSE;
    }
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) //add william 2012-4-25
	if ( strlen(offerlist->vendorid) > 0 ) 
	{
		is_dhcpvlan_vendor(inet_ntoa(addr),offerlist->vendorid);
	}
#endif

#if defined(AEI_VDSL_CUSTOMER_TELUS)
    getClientIDOption(oldpacket, offerlist);
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    getClientIDOption(oldpacket, offerlist);
#endif

    return 0;
}

int send_inform(struct dhcpMessage *oldpacket)
{
    struct dhcpMessage packet;
    struct option_set *curr;
    //brcm begin
    char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
    char *req;
    //brcm end
#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
	struct static_lease *special_lease = NULL;
	special_lease = AEI_getLeaseByMac(cur_iface->static_leases,
		oldpacket->chaddr);
#endif
    init_packet(&packet, oldpacket, DHCPACK);

    curr = cur_iface->options;
    while (curr) {
        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME) {
#ifdef AEI_VDSL_CUSTOMER_QWEST
            if (curr->data[OPT_CODE] == DHCP_DNS_SERVER)
                custom_dns_option(&packet, curr);
#endif

#ifdef AEI_VDSL_CUSTOMER_TELUS
            if (curr->data[OPT_CODE] == DHCP_BOOT_FILE) {
                if (is_IPTVSTB(oldpacket) && wantOption(oldpacket, DHCP_BOOT_FILE)) {
                    add_option_string(packet.options, curr->data);
                }

                curr = curr->next;
                continue;
            }
#endif

#if defined(AEI_VDSL_CUSTOMER_ADVANCED_DMZ)
            AEI_modify_option_string(special_lease, &packet, curr);
#else
            add_option_string(packet.options, curr->data);
#endif
        }

        curr = curr->next;
    }

    add_bootp_options(&packet);

    //brcm begin
    /* if DHCPRequest from client has device identity, send back gateway identity,
       and save the device identify */
    if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) {
        if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY, VIinfo) != -1)
            add_option_string(packet.options, VIinfo);
    }
    //brcm end

    return send_packet(&packet, 0);
}
