/* dhcpd.c
 *
 * Moreton Bay DHCP Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"
#include "static_leases.h"
#include "cms_mem.h"
#include "cms_msg.h"
#ifdef DHCP_RELAY
#include "relay.h"
#endif

/* globals */
struct server_config_t server_config;
struct iface_config_t *iface_config;
struct iface_config_t *cur_iface;
#ifdef DHCP_RELAY
struct relay_config_t *relay_config;
struct relay_config_t *cur_relay;
#endif
// BRCM_begin
struct dhcpOfferedAddr *declines;
pVI_INFO_LIST viList;
void *msgHandle=NULL;
extern char deviceOui[];
extern char deviceSerialNum[];
extern char deviceProductClass[];

extern void bcmQosDhcp(int optionnum, char *cmd);
extern void bcmDelObsoleteRules(void);
extern void bcmExecOptCmd(void);
// BRCM_end

#if defined(AEI_VDSL_DHCP_LEASE)
int ip_changed = 0;
static int dhcp_lease_changed = 0;
static int reduce_lease_time = 0;
static unsigned long select_begin_time = 0;
static unsigned long select_end_time = 0;
#endif

#if defined(AEI_VDSL_QOS)
extern void AEI_bcmSearchOptCmdByVendorId(struct dhcpMessage *oldpacket, const char *vendorid);
#endif

/* Exit and cleanup */
void exit_server(int retval)
{
	cmsMsg_cleanup(&msgHandle);
	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* SIGTERM handler */
static void udhcpd_killed(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGTERM");
	exit_server(0);
}


// BRCM_BEGIN
static void test_vendorid(struct dhcpMessage *packet, char *vid, int *declined)
{
	*declined = 0;

	/*
	 * If both internal vendor_ids list is not NULL and client
	 * request packet contains a vendor id, then check for match.
	 * Otherwise, leave declined at 0.
	 */
	if (cur_iface->vendor_ids != NULL && vid != NULL) {
		int len = (int)(*((unsigned char*)vid - 1));
		vendor_id_t * id = cur_iface->vendor_ids;
		//char tmpbuf[257] = {0};
		//memcpy(tmpbuf, vid, len);
		do {
			//printf("dhcpd:test_vendorid: id=%s(%d) vid=%s(%d)\n", id->id, id->len, tmpbuf, len);
			if (id->len == len && memcmp(id->id, vid, len) == 0) {
				/* vid matched something in our list, decline */
				memcpy(declines->chaddr,  packet->chaddr, sizeof(declines->chaddr));

				/*
				 * vid does not contain a terminating null, so we have to make sure
				 * the declines->vendorid is null terminated.  And use memcpy 
				 * instead of strcpy because vid is not null terminated.
				 */
				memset(declines->vendorid, 0, sizeof(declines->vendorid));
				memcpy(declines->vendorid, vid, len);

				*declined = 1;
			}
		} while((*declined == 0) && (id = id->next));
	}

	if (*declined) {
		write_decline();
		sendNAK(packet);
	}
}

// BRCM_END

#ifdef COMBINED_BINARY	
int udhcpd(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	CmsRet ret;
	int msg_fd;
	fd_set rfds;
	struct timeval tv;
        //BRCM --initialize server_socket to -1
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
        // BRCM added vendorid
	char *server_id, *requested, *hostname, *vendorid = NULL;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct dhcpOfferedAddr *lease;
	//For static IP lease
	struct dhcpOfferedAddr static_lease;
	uint32_t static_lease_ip;

	int pid_fd;

	/* Optimize malloc */
	mallopt(M_TRIM_THRESHOLD, 8192);
	mallopt(M_MMAP_THRESHOLD, 16384);

#ifdef AEI_VDSL_CUSTOMER_NCS
	memset(&static_lease, 0, sizeof(static_lease));
#endif
	//argc = argv[0][0]; /* get rid of some warnings */

	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);
	
	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((ret = cmsMsg_init(EID_DHCPD, &msgHandle)) != CMSRET_SUCCESS) {
		LOG(LOG_ERR, "cmsMsg_init failed, ret=%d", ret);
		pidfile_delete(server_config.pidfile);
		CLOSE_LOG();
		exit(1);
	}

#ifdef AEI_VDSL_CUSTOMER_NCS
	if (!read_config(argc < 2 ? DHCPD_CONF_FILE : argv[1])) {
		LOG(LOG_ERR, "read_config failed, ret=%d===[%ld]", ret,getppid());
		cmsMsg_cleanup(&msgHandle);
		pidfile_delete(server_config.pidfile);
		CLOSE_LOG();
		exit(1);
	}
#else
	//read_config(DHCPD_CONF_FILE);
	read_config(argc < 2 ? DHCPD_CONF_FILE : argv[1]);
#endif
	
	read_leases(server_config.lease_file);

        // BRCM begin
	declines = malloc(sizeof(struct dhcpOfferedAddr));
	/* vendor identifying info list */
	viList = malloc(sizeof(VI_INFO_LIST));
	memset(viList, 0, sizeof(VI_INFO_LIST));
        deviceOui[0] = '\0';
        deviceSerialNum[0] = '\0';
        deviceProductClass[0] = '\0';
        // BRCM end

#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */

#ifdef BRCM_CMS_BUILD
	/*
	 * BRCM_BEGIN: mwang: In CMS architecture, we don't want dhcpd to
	 * daemonize itself.  It creates an extra zombie process that
	 * we don't want to deal with.
	 */
#else
	switch(fork()) {
	case -1:
		perror("fork");
		exit_server(1);
		/*NOTREACHED*/
	case 0:
		break; /* child continues */
	default:
		exit(0); /* parent exits */
		/*NOTREACHED*/
	}
	close(0);
	setsid();
#endif /* BRCM_CMS_BUILD */

	pidfile_write_release(pid_fd);
#endif
	signal(SIGUSR1, write_leases);
	signal(SIGTERM, udhcpd_killed);
	signal(SIGUSR2, write_viTable);

	timeout_end = time(0) + server_config.auto_time;

	cmsMsg_getEventHandle(msgHandle, &msg_fd);

	while(1) { /* loop until universe collapses */
		//BRCM_begin
		int declined = 0;
		int max_skt = msg_fd;
		FD_ZERO(&rfds);
		FD_SET(msg_fd, &rfds);

#ifdef DHCP_RELAY
		for(cur_relay = relay_config; cur_relay;
			cur_relay = cur_relay->next ) {
			if (cur_relay->skt < 0) {
				cur_relay->skt = listen_socket(INADDR_ANY,
					SERVER_PORT, cur_relay->interface);
				if(cur_relay->skt == -1) {
					LOG(LOG_ERR, "couldn't create relay "
						"socket");
					exit_server(0);
				}
			}

			FD_SET(cur_relay->skt, &rfds);
			if (cur_relay->skt > max_skt)
				max_skt = cur_relay->skt;
		}
#endif

		for(cur_iface = iface_config; cur_iface;
			cur_iface = cur_iface->next ) {
			if (cur_iface->skt < 0) {
				cur_iface->skt = listen_socket(INADDR_ANY,
					SERVER_PORT, cur_iface->interface);
				if(cur_iface->skt == -1) {
					LOG(LOG_ERR, "couldn't create server "
						"socket on %s -- au revoir",
						cur_iface->interface);
#if defined(AEI_VDSL_CUSTOMER_QWEST) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
					if (strcmp(cur_iface->interface, "br0") == 0) {
						exit_server(0);
					} else {
						break;
					}
#else
					exit_server(0);
#endif
				}
			}

			FD_SET(cur_iface->skt, &rfds);
			if (cur_iface->skt > max_skt)
				max_skt = cur_iface->skt;
		}  //BRCM_end

		if (server_config.auto_time) {
#if defined(AEI_VDSL_DHCP_LEASE)
			tv.tv_sec = server_config.auto_time;
#else
			tv.tv_sec = timeout_end - time(0);
#endif
			if (tv.tv_sec <= 0) {
				tv.tv_sec = server_config.auto_time;
				timeout_end = time(0) + server_config.auto_time;
				write_leases(0);
			}
			tv.tv_usec = 0;
		}

#if defined(AEI_VDSL_DHCP_LEASE)
		ip_changed = 0;

		if (select_begin_time == 0)
			select_begin_time = time(0);
		else
		{
			long val;

			select_end_time = time(0);

			val = select_end_time - select_begin_time;
			if (val < 0 || val >= server_config.auto_time)
				val = server_config.auto_time;

			reduce_lease_time += val;

			select_begin_time = select_end_time;
		}
#endif

		retval = select(max_skt + 1, &rfds, NULL, NULL,
				server_config.auto_time ? &tv : NULL);
		if (retval == 0) {
#if defined(AEI_VDSL_DHCP_LEASE)
			reduce_lease_time += server_config.auto_time;
			update_lease_time_remaining();
			for (cur_iface = iface_config; cur_iface; cur_iface = cur_iface->next )
			{
				if (dhcp_lease_changed)
				{
					sendDhcpLeasesUpdatedMsg();
				}
			}
			dhcp_lease_changed = 0;
#endif

			write_leases(0);
			timeout_end = time(0) + server_config.auto_time;
			continue;
		} else if (retval < 0) {
			if (errno != EINTR)
				perror("select()");
			continue;
		}

		/* Is there a CMS message received? */
		if (FD_ISSET(msg_fd, &rfds)) {
			CmsMsgHeader *msg;
			CmsRet ret;

			ret = cmsMsg_receiveWithTimeout(msgHandle, &msg, 0);
			if (ret == CMSRET_SUCCESS) {
				switch (msg->type) {
				case CMS_MSG_DHCPD_RELOAD:
					/* Reload config file */
					write_leases(0);
					read_config(argc < 2 ? DHCPD_CONF_FILE : argv[1]);
					read_leases(server_config.lease_file);
					break;
#if defined(AEI_VDSL_CUSTOMER_NCS) && !defined(AEI_VDSL_DHCP_LEASE)
				case CMS_MSG_DHCP_TIME_UPDATED:
				{
					for (cur_iface = iface_config; cur_iface; 
					     cur_iface = cur_iface->next) {
						write_leases(msg->wordData);
						read_leases(server_config.lease_file);
					}
					break;
				}
#endif
#if defined(AEI_VDSL_DHCP_LEASE)
				case CMS_MSG_DHCPD_RELOAD_LEASE_FILE:
                                {
					for (cur_iface = iface_config; cur_iface; 
					     cur_iface = cur_iface->next) {
						read_leases(server_config.lease_file);
					}
					break;
				}
				case CMS_MSG_UPDATE_LEASE_TIME_REMAINING:
				{
					long val;

					select_end_time = time(0);

					val = select_end_time - select_begin_time;
					if (val < 0 || val >= server_config.auto_time)
						val = server_config.auto_time;

					reduce_lease_time += val;
					update_lease_time_remaining();
					write_leases(0);

					msg->dst = msg->src;
					msg->src = EID_DHCPD;
					msg->flags_request = 0;
					msg->flags_response = 1;
					msg->wordData = 0;
					msg->dataLength = 0;
					cmsMsg_send(msgHandle, msg);
					break;
				}
#endif /* AEI_VDSL_DHCP_LEASE */
				case CMS_MSG_WAN_CONNECTION_UP:
#ifdef DHCP_RELAY
					/* Refind local addresses for relays */
					set_relays();
#endif
					break;
				case CMS_MSG_GET_LEASE_TIME_REMAINING:
					if (msg->dataLength == sizeof(GetLeaseTimeRemainingMsgBody)) {
						GetLeaseTimeRemainingMsgBody *body = (GetLeaseTimeRemainingMsgBody *) (msg + 1);
						u_int8_t chaddr[16]={0};

						cur_iface = find_iface_by_ifname(body->ifName);
						if (cur_iface != NULL) {
							cmsUtl_macStrToNum(body->macAddr, chaddr);
							lease = find_lease_by_chaddr(chaddr);
							if (lease != NULL) {
								msg->dst = msg->src;
								msg->src = EID_DHCPD;
								msg->flags_request = 0;
								msg->flags_response = 1;
#if defined (AEI_VDSL_CUSTOMER_TELUS) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
								//static leases
								if (getIpByMac(cur_iface->static_leases, chaddr))
									msg->wordData = 0;
								else
#endif
									msg->wordData = lease_time_remaining(lease);
								msg->dataLength = 0;
								cmsMsg_send(msgHandle, msg);
							}
#if defined(AEI_VDSL_CUSTOMER_NCS)
							else {
								msg->dst = msg->src;
								msg->src = EID_DHCPD;
								msg->flags_request = 0;
								msg->flags_response = 1;
								msg->wordData = 0;
								msg->dataLength = 0;
								cmsMsg_send(msgHandle, msg);
							}
#endif
						}
					}
					else {
						LOG(LOG_ERR, "invalid msg, type=0x%x dataLength=%d", msg->type, msg->dataLength);
					}
					break;
#if !defined(AEI_VDSL_DHCP_LEASE)
				case CMS_MSG_EVENT_SNTP_SYNC:               
					if (msg->dataLength == sizeof(long)) 
					{
						long *delta = (long *) (msg + 1); 
						adjust_lease_time(*delta);
					}
					else
					{
						LOG(LOG_ERR, "Invalid CMS_MSG_EVENT_SNTP_SYNC msg, dataLength=%d", msg->dataLength);
					}
					break;
#endif
				case CMS_MSG_QOS_DHCP_OPT60_COMMAND:
					bcmQosDhcp(DHCP_VENDOR, (char *)(msg+1));
					break;
				case CMS_MSG_QOS_DHCP_OPT77_COMMAND:
					bcmQosDhcp(DHCP_USER_CLASS_ID, (char *)(msg+1));
					break;
				default:
					LOG(LOG_ERR, "unrecognized msg, type=0x%x dataLength=%d", msg->type, msg->dataLength);
					break;
				}
				cmsMem_free(msg);
			} else if (ret == CMSRET_DISCONNECTED) {
				LOG(LOG_ERR, "lost connection to smd, "
					"exiting now.");
				exit_server(0);
			}
			continue;
		}

#ifdef DHCP_RELAY
		/* Check packets from upper level DHCP servers */
		for (cur_relay = relay_config; cur_relay;
			cur_relay = cur_relay->next) {
			if (FD_ISSET(cur_relay->skt, &rfds))
				break;
		}
		if (cur_relay) {
			process_relay_response();
			continue;
		}
#endif

		/* Find out which interface is selected */
		for(cur_iface = iface_config; cur_iface;
			cur_iface = cur_iface->next ) {
			if (FD_ISSET(cur_iface->skt, &rfds))
				break;
		}
		if (cur_iface == NULL)
			continue;

		bytes = get_packet(&packet, cur_iface->skt); /* this waits for a packet - idle */
		if(bytes < 0)
			continue;

		/* Decline requests */
		if (cur_iface->decline)
			continue;

#ifdef DHCP_RELAY
		/* Relay request? */
		if (cur_iface->relay_interface[0]) {
			process_relay_request((char*)&packet, bytes);
			continue;
		}
#endif

		if((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
			continue;
		}
		
		/* For static IP lease */
		/* Look for a static lease */
		static_lease_ip = getIpByMac(cur_iface->static_leases, &packet.chaddr);

		if(static_lease_ip)
		{
			memcpy(&static_lease.chaddr, &packet.chaddr, 16);
			static_lease.yiaddr = static_lease_ip;
			static_lease.expires = -1; /* infinite lease time */
			lease = &static_lease;
		} /* For static IP lease end */
		else
		{
			lease = find_lease_by_chaddr(packet.chaddr);
		}

		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			//BRCM_begin
			vendorid = (char *)get_option(&packet, DHCP_VENDOR);
			test_vendorid(&packet, vendorid, &declined);
			// BRCM_end

#if defined(AEI_VDSL_QOS)
			AEI_bcmSearchOptCmdByVendorId(&packet, vendorid);
#endif

			if (!declined) {
#if defined(AEI_VDSL_DHCP_LEASE)
				ip_changed = 1;
#endif

				if (sendOffer(&packet) < 0) {
					LOG(LOG_DEBUG, "send OFFER failed -- ignoring");
				}
//brcm begin
				/* delete any obsoleted QoS rules because sendOffer() may have
			 	 * cleaned up the lease data.
			 	 */
				bcmDelObsoleteRules(); 
//brcm end
			}
			break;			
 		case DHCPREQUEST:
			DEBUG(LOG_INFO,"received REQUEST");

			requested = (char *)get_option(&packet, DHCP_REQUESTED_IP);
			server_id = (char *)get_option(&packet, DHCP_SERVER_ID);
			hostname  = (char *)get_option(&packet, DHCP_HOST_NAME);
			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)|| defined(AEI_VDSL_CUSTOMER_TELUS)
			if (lease)
				getClientIDOption(&packet,lease);
#endif
			//BRCM_begin
			vendorid = (char *)get_option(&packet, DHCP_VENDOR);
			test_vendorid(&packet, vendorid, &declined);
			// BRCM_end


#if defined(AEI_VDSL_QOS)
			AEI_bcmSearchOptCmdByVendorId(&packet, vendorid);
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS) //add william 2012-1-11
			char actVendorid[VENDOR_CLASS_ID_STR_SIZE + 1];
			actVendorid[0] = '\0';
			if (1 != AEI_is_vendor_equipped(&packet, actVendorid))
#endif

			if (!declined) {
				if (lease) {
#if defined(AEI_VDSL_CUSTOMER_QWEST)
					int illegal_name = 0;
					int iconflag = 0;

					if (vendorid) {
						int len = vendorid[-1];
						if (len >= (int)sizeof(lease->vendorid))
							len = sizeof(lease->vendorid) - 1;

						snprintf(lease->vendorid, len + 1, "%s", vendorid);

						if (!strncmp(vendorid, "Xbox 360", len)) {
							printf("The device is Xbox!\n");
							iconflag = 1;
						}
					}
#endif
					if (hostname) {
						bytes = hostname[-1];
						if (bytes >= (int) sizeof(lease->hostname))
							bytes = sizeof(lease->hostname) - 1;

						strncpy(lease->hostname, hostname, bytes);
						lease->hostname[bytes] = '\0';

#if defined(AEI_VDSL_CUSTOMER_QWEST)
						if (strstr(lease->hostname, "\(") || 
						    strstr(lease->hostname, "\)") || 
						    strstr(lease->hostname, ".") || 
						    strstr(lease->hostname, ";") || 
						    strstr(lease->hostname, "'") || 
						    strstr(lease->hostname, ",")) {
							printf("host name has illegal characters!\n");
							illegal_name = 1;
						} else {
							if (!strncmp(lease->hostname, "SipuraSPA", bytes)) {
								printf("Qwest Linksys VoIP ATA device!\n");
								iconflag = 7;
							} else if (!strncmp(lease->hostname, "NP-", strlen("NP-"))) {
								printf("RoKu device!\n");
								iconflag = 4;   //type is set to "Set Top box"
							} else if ((!strncmp(lease->hostname, "DIRECTV-HR20", strlen("DIRECTV-HR20")) || 
								    !strncmp(lease->hostname, "DIRECTV-HR21", strlen("DIRECTV-HR21")) || 
								    !strncmp(lease->hostname, "DIRECTV-HR22", strlen("DIRECTV-HR22")) || 
								    !strncmp(lease->hostname, "DIRECTV-HR23", strlen("DIRECTV-HR23")) || 
								    !strncmp(lease->hostname, "DIRECTV-HR24", strlen("DIRECTV-HR24")))) {
								iconflag = 3;
							} else {
								//printf("Other device!\n");
							}
						}
#endif

#if !defined(AEI_VDSL_CUSTOMER_QWEST)
						send_lease_info(FALSE, lease);
#endif
					}  else
						lease->hostname[0] = '\0';
     
#if defined (AEI_VDSL_CUSTOMER_NCS)
					if (vendorid) {
						int len = vendorid[-1];
						if (len >= (int)sizeof(lease->vendorid))
							len = sizeof(lease->vendorid) - 1;

						snprintf(lease->vendorid, len + 1, "%s", vendorid);

#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
						AEI_InitLeaseWP(&packet,lease);
#endif


#if defined(AEI_VDSL_CUSTOMER_QWEST)
						if (!strncmp(vendorid, "MSFT_IPTV", strlen("MSFT_IPTV"))) {
							iconflag = 4;

#ifdef AEI_VDSL_CUSTOMER_CENTURYLINK
							strncpy(lease->hostname,IPTV_STB_STR, sizeof(lease->hostname));
#else
							strncpy(lease->hostname, "Qwest Video", sizeof(lease->hostname));
#endif
						}
#endif

#ifdef AEI_VDSL_CUSTOMER_TELUS
                                                if (!strncmp(vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")) || !strncmp(vendorid, "SAIP", strlen("SAIP"))) {
                                                        if (hostname == NULL) {
                                                               strncpy(lease->hostname, "OptikTV", sizeof(lease->hostname));
                                                        }
                                                }    
                                                        
#endif
					}
#endif
					if (server_id) {
						/* SELECTING State */
						DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
						if (server_id_align == cur_iface->server && requested && 
						    requested_align == lease->yiaddr) {

#if defined(AEI_VDSL_CUSTOMER_QWEST)
							if (illegal_name == 1) {
								printf("host name has illegal characters and return NAK!\n");
								sendNAK(&packet);
							} else {
#endif
								sendACK(&packet, lease->yiaddr);
#if defined(AEI_VDSL_CUSTOMER_QWEST)
								lease->icon = iconflag;
#ifdef AEI_VDSL_CUSTOMER_CENTURYLINK
								if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")))
									strncpy(lease->hostname,IPTV_STB_STR, sizeof(lease->hostname));
#else
								if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")))
									strncpy(lease->hostname, "Qwest Video", sizeof(lease->hostname));
#endif
								if (lease->icon == 1)
									snprintf(lease->hostname, sizeof(lease->hostname), "%s", "Xbox 360");
#endif

#ifdef AEI_VDSL_CUSTOMER_TELUS
                                                               if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")) || !strncmp(lease->vendorid, "SAIP", strlen("SAIP"))) {
                                                                      if (hostname == NULL) {
                                                                             strncpy(lease->hostname, "OptikTV", sizeof(lease->hostname));
                                                                      }
                                                               } 

#endif
#if defined (AEI_VDSL_CUSTOMER_NCS)
								if ((strcmp(lease->hostname, "") == 0) && (hostname != NULL)) {
									strncpy(lease->hostname, hostname, bytes);
									lease->hostname[bytes] = '\0';
								}
#endif
								send_lease_info(FALSE, lease);

								//brcm begin
								/* delete any obsoleted QoS rules because sendACK() may have
							 	* cleaned up the lease data.
							 	*/
								bcmDelObsoleteRules(); 
								bcmExecOptCmd(); 
								//brcm end

#if defined(AEI_VDSL_CUSTOMER_QWEST)
							}
#endif
						}
					} else {
						if (requested) {
							/* INIT-REBOOT State */
							if (lease->yiaddr == requested_align) {
#if defined(AEI_VDSL_CUSTOMER_QWEST)
								if (illegal_name == 1) {
									printf("host name has illegal characters and return NAK!\n");
									sendNAK(&packet);
								} else {
#endif
									sendACK(&packet, lease->yiaddr);
#if defined(AEI_VDSL_CUSTOMER_QWEST)
									lease->icon = iconflag;
#ifdef AEI_VDSL_CUSTOMER_CENTURYLINK
									if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")))
										strncpy(lease->hostname,IPTV_STB_STR, sizeof(lease->hostname));
#else
									if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")))
										strncpy(lease->hostname, "Qwest Video", sizeof(lease->hostname));
#endif
									if (lease->icon == 1)
										snprintf(lease->hostname, sizeof(lease->hostname), "%s", "Xbox 360");
#endif

#if defined(AEI_VDSL_CUSTOMER_TELUS)
                                                                       if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")) || !strncmp(lease->vendorid, "SAIP", strlen("SAIP"))) {
                                                                            if (hostname == NULL) {
                                                                                  strncpy(lease->hostname, "OptikTV", sizeof(lease->hostname));
                                                                            }
                                                                       }

#endif

#if defined (AEI_VDSL_CUSTOMER_NCS)
									if ((strcmp(lease->hostname, "") == 0) && (hostname != NULL)) {
										strncpy(lease->hostname, hostname, bytes);
										lease->hostname[bytes] = '\0';
									}
#endif
									send_lease_info(FALSE, lease);

									//brcm begin
									/* delete any obsoleted QoS rules because sendACK() may have
									 * cleaned up the lease data.
									 */
									bcmDelObsoleteRules(); 
									bcmExecOptCmd(); 
//brcm end
#if defined(AEI_VDSL_CUSTOMER_QWEST)
								}
#endif
							}
							else sendNAK(&packet);
						} else {
							/* RENEWING or REBINDING State */
							if (lease->yiaddr == packet.ciaddr) {
#if defined(AEI_VDSL_CUSTOMER_QWEST)
								if (illegal_name == 1) {
									printf("host name has illegal characters and return NAK!\n");
									sendNAK(&packet);
								} else {
#endif
									sendACK(&packet, lease->yiaddr);

#if defined(AEI_VDSL_CUSTOMER_QWEST)
									lease->icon = iconflag;
#ifdef AEI_VDSL_CUSTOMER_CENTURYLINK
									if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")))
										strncpy(lease->hostname,IPTV_STB_STR, sizeof(lease->hostname));
#else
									if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")))
										strncpy(lease->hostname, "Qwest Video", sizeof(lease->hostname));
#endif
									if (lease->icon == 1)
										snprintf(lease->hostname, sizeof(lease->hostname), "%s", "Xbox 360");
#endif

#ifdef AEI_VDSL_CUSTOMER_TELUS
                                                                       if (!strncmp(lease->vendorid, "MSFT_IPTV", strlen("MSFT_IPTV")) || !strncmp(lease->vendorid, "SAIP", strlen("SAIP"))) {
                                                                               if (hostname == NULL) {
                                                                                       strncpy(lease->hostname, "OptikTV", sizeof(lease->hostname));
                                                                               }
                                                                       }

#endif

#if defined (AEI_VDSL_CUSTOMER_NCS)
									if ((strcmp(lease->hostname, "") == 0) && (hostname != NULL)) {
										strncpy(lease->hostname, hostname, bytes);
										lease->hostname[bytes] = '\0';
									}
#endif
									send_lease_info(FALSE, lease);

									//brcm begin
									/* delete any obsoleted QoS rules because sendACK() may have
									 * cleaned up the lease data.
									 */
									bcmDelObsoleteRules(); 
									bcmExecOptCmd(); 
									//brcm end
#if defined(AEI_VDSL_CUSTOMER_QWEST)
								}
#endif
							}
							else {
								/* don't know what to do!!!! */
								sendNAK(&packet);
							}
						}						
					}
				} else { /* else remain silent */				
    					sendNAK(&packet);
            			}
			}
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");

			if (lease) {
				memset(lease->chaddr, 0, 16);
#if defined(AEI_VDSL_DHCP_LEASE)
				lease->expires = server_config.decline_time;
#else
				lease->expires = time(0) + server_config.decline_time;
#endif
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
			if (lease) {
#if defined(AEI_VDSL_DHCP_LEASE)
				ip_changed = 1;
				lease->expires = 0;
#else
				lease->expires = time(0);
#endif
				send_lease_info(TRUE, lease);

				//brcm begin
				sleep(1);
				/* delete the obsoleted QoS rules */
				bcmDelObsoleteRules();
				//brcm end

#if defined(AEI_VDSL_STB_NO_FIREWALL)
				if (lease->is_stb)
				{
					del_stb_from_list(lease->chaddr, lease->yiaddr);
				}
#endif
			}
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}

#if defined(AEI_VDSL_DHCP_LEASE)
		if (state[0] == DHCPDISCOVER ||
		    state[0] == DHCPRELEASE)
		{
			lease = find_lease_by_chaddr(packet.chaddr);

			/* only update dhcp leases for STB clients */
			if (lease && lease->is_stb && ip_changed)
			{
				dhcp_lease_changed = 1;
			}
#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
			/* update dhcp leases for WP clients */
			if (lease && lease->isWP && ip_changed)
			{
				dhcp_lease_changed = 1;
			}
#endif
		}
#endif
	}
	return 0;
}

#ifdef AEI_VDSL_CUSTOMER_CENTURYLINK //add william 2012-4-25
void sendDhcpVlanUpdatedMsg(char *ip,char *vlanId)
{
    CmsRet ret;
	CmsMsgHeader *msg=NULL;
	char msgBuf[sizeof(CmsMsgHeader) + sizeof(char)*32] = { 0 };
	char *chTemp=NULL;

	msg=(CmsMsgHeader *)msgBuf;
    msg->src = EID_DHCPD;//cmsMsg_getHandleEid(msgHandle);
    msg->dst = EID_SSK;
	msg->type = CMS_MSG_DHCP_VLAN_VENDOR;
    msg->flags_event = 1;
    msg->dataLength = sizeof(char)*32;
	chTemp = (char *) (msg+1);

	memcpy(chTemp,ip,16);
	memcpy(chTemp+16,vlanId,16);
	
    if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
        cmsLog_error("william->Failed to send message CMS_MSG_DHCP_LEASES_UPDATED to ssk, ret = %d", ret);
   
	return;
}
#endif

#if defined(AEI_VDSL_DHCP_LEASE)
void sendDhcpLeasesUpdatedMsg(void)
{
    CmsRet ret;
    int i, leases_num = 0;
    int msgsize, bodysize;
    char *msgBuf;
    CmsMsgHeader *msg;
    DhcpLeasesUpdatedMsgBody *body;

    if (cur_iface == NULL)
        return;

    for (i = 0; i < cur_iface->max_leases; i++)
    {
        if (!lease_expired(&(cur_iface->leases[i])))
        {
            /* we only update dhcp leases for STB clients */
            /* also consider WP devices for dhcp lease retention */
            if (cur_iface->leases[i].is_stb
#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
              || cur_iface->leases[i].isWP
#endif
            )
            {
                leases_num++;
            }


        }
    }

    if (leases_num == 0)
        return;

    bodysize = leases_num * sizeof(DhcpLeasesUpdatedMsgBody);
    msgsize = sizeof(CmsMsgHeader) + bodysize;
    msgBuf = calloc(msgsize, sizeof(char));
    if (msgBuf == NULL)
        return;

    msg = (CmsMsgHeader *) msgBuf;
    body = (DhcpLeasesUpdatedMsgBody *) (msg + 1);

    for (i = 0; i < cur_iface->max_leases; i++)
    {
        if (!lease_expired(&(cur_iface->leases[i])))
        {
            if (cur_iface->leases[i].is_stb)
            {
                cmsUtl_macNumToStr(cur_iface->leases[i].chaddr, body->chaddr);
                body->yiaddr = cur_iface->leases[i].yiaddr;
                body->expires = cur_iface->leases[i].expires;
                strcpy(body->hostname, cur_iface->leases[i].hostname);
                if(cur_iface->interface && strlen(cur_iface->interface))
                    snprintf(body->interface, sizeof(body->interface), cur_iface->interface);
                body->is_stb = cur_iface->leases[i].is_stb;

#if defined(AEI_VDSL_CUSTOMER_TELUS)
                strcpy(body->oui, cur_iface->leases[i].oui);
                strcpy(body->serialNumber, cur_iface->leases[i].serialNumber);
                strcpy(body->productClass, cur_iface->leases[i].productClass);
#endif

#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
                body->isWP = cur_iface->leases[i].isWP;
                if (cur_iface->leases[i].isWP)
                {
                    strlcpy(body->WPProductType,cur_iface->leases[i].WPProductType,sizeof(body->WPProductType));
                    strlcpy(body->WPFirmwareVersion,cur_iface->leases[i].WPFirmwareVersion,sizeof(body->WPFirmwareVersion));
                    strlcpy(body->WPProtocolVersion,cur_iface->leases[i].WPProtocolVersion,sizeof(body->WPProtocolVersion));
                }
#endif
                body++;
            }
#if defined(AEI_VDSL_WP) && defined(AEI_VDSL_DHCP_LEASE)
            /* if  WP */
            else if (cur_iface->leases[i].isWP)
            {
                cmsUtl_macNumToStr(cur_iface->leases[i].chaddr, body->chaddr);
                body->yiaddr = cur_iface->leases[i].yiaddr;
                body->expires = cur_iface->leases[i].expires;
                strcpy(body->hostname, cur_iface->leases[i].hostname);
                if(cur_iface->interface && strlen(cur_iface->interface))
                    snprintf(body->interface, sizeof(body->interface), cur_iface->interface);
                body->isWP = cur_iface->leases[i].isWP;
                if (cur_iface->leases[i].isWP)
                {
                    strlcpy(body->WPProductType,cur_iface->leases[i].WPProductType,sizeof(body->WPProductType));
                    strlcpy(body->WPFirmwareVersion,cur_iface->leases[i].WPFirmwareVersion,sizeof(body->WPFirmwareVersion));
                    strlcpy(body->WPProtocolVersion,cur_iface->leases[i].WPProtocolVersion,sizeof(body->WPProtocolVersion));
                }
                body++;
            }
#endif
        }
    }

    msg->type = CMS_MSG_DHCP_LEASES_UPDATED;
    msg->src = EID_DHCPD;
    msg->dst = EID_SSK;
    msg->flags_event = 1;
    msg->dataLength = bodysize;

    if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
        cmsLog_error("Failed to send message CMS_MSG_DHCP_LEASES_UPDATED to ssk, ret = %d", ret);
    else
        cmsLog_debug("Message CMS_MSG_DHCP_LEASES_UPDATED sent to ssk successfully");

    free(msgBuf);
}

void update_lease_time_remaining(void)
{
    int n;

    for (cur_iface = iface_config; cur_iface; cur_iface = cur_iface->next )
    {
        for (n = 0; n < cur_iface->max_leases; n++)
        {
            if (cur_iface->leases[n].yiaddr &&
               (cur_iface->leases[n].expires > 0) &&
               (cur_iface->leases[n].expires != 0xffffffff)) /* -1 means infinite lease time */
            {
                if (cur_iface->leases[n].expires <= reduce_lease_time)
                {
                    send_lease_info(TRUE, &(cur_iface->leases[n]));
                    cur_iface->leases[n].expires = 0;
#if defined(AEI_VDSL_STB_NO_FIREWALL)
                    if (cur_iface->leases[n].is_stb)
                    {
                        del_stb_from_list(cur_iface->leases[n].chaddr, cur_iface->leases[n].yiaddr);
                    }
#endif
                }
                else
                    cur_iface->leases[n].expires -= reduce_lease_time;
            }
        }
    }

    reduce_lease_time = 0;
    select_begin_time = 0;
    select_end_time = 0;
}
#endif /* AEI_VDSL_DHCP_LEASE */
