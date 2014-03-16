/* vi: set sw=4 ts=4: */
/*
 * Mini nslookup implementation for busybox
 *
 * Copyright (C) 1999,2000 by Lineo, inc. and John Beppu
 * Copyright (C) 1999,2000,2001 by John Beppu <beppu@codepoet.org>
 *
 * Correct default name server display and explicit name server option
 * added by Ben Zeckel <bzeckel@hmc.edu> June 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdint.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include <arpa/inet.h>
#include "busybox.h"

#if defined(AEI_VDSL_SMARTLED) || defined (AEI_VDSL_CUSTOMER_BELLALIANT)
#include "cms_msg.h"
#include "cms_log.h"

extern struct hostent *AEI_gethostbyname_txt(const char *hostname);
extern void AEI_sendTxtRecordForNslookup(void *msgHandle);
extern void AEI_cleanup();

static void *msgHandle = NULL;
#endif

/*
 |  I'm only implementing non-interactive mode;
 |  I totally forgot nslookup even had an interactive mode.
 */

/* only works for IPv4 */
static int addr_fprint(char *addr)
{
	uint8_t split[4];
	uint32_t ip;
	uint32_t *x = (uint32_t *) addr;

	ip = ntohl(*x);
	split[0] = (ip & 0xff000000) >> 24;
	split[1] = (ip & 0x00ff0000) >> 16;
	split[2] = (ip & 0x0000ff00) >> 8;
	split[3] = (ip & 0x000000ff);
	printf("%d.%d.%d.%d", split[0], split[1], split[2], split[3]);
	return 0;
}

/* takes the NULL-terminated array h_addr_list, and
 * prints its contents appropriately
 */
static int addr_list_fprint(char **h_addr_list)
{
	int i, j;
	char *addr_string = (h_addr_list[1])
		? "Addresses:" : "Address:";

	printf("%s", addr_string);
	for (i = 0, j = 0; h_addr_list[i]; i++, j++) {
		addr_fprint(h_addr_list[i]);

		/* real nslookup does this */
		if (j == 4) {
			if (h_addr_list[i + 1]) {
				printf("\n          ");
			}
			j = 0;
		} else {
			if (h_addr_list[i + 1]) {
				printf(", ");
			}
		}

	}
	printf("\n");
	return 0;
}

/* print the results as nslookup would */
static struct hostent *hostent_fprint(struct hostent *host, const char *server_host)
{
	if (host) {
		printf("%s     %s\n", server_host, host->h_name);
		addr_list_fprint(host->h_addr_list);
	} else {
		printf("*** Unknown host\n");
	}
	return host;
}

/* changes a c-string matching the perl regex \d+\.\d+\.\d+\.\d+
 * into a uint32_t
 */
static uint32_t str_to_addr(const char *addr)
{
	uint32_t split[4];
	uint32_t ip;

	sscanf(addr, "%d.%d.%d.%d",
		   &split[0], &split[1], &split[2], &split[3]);

	/* assuming sscanf worked */
	ip = (split[0] << 24) |
		(split[1] << 16) | (split[2] << 8) | (split[3]);

	return htonl(ip);
}

/* gethostbyaddr wrapper */
static struct hostent *gethostbyaddr_wrapper(const char *address)
{
	struct in_addr addr;

	addr.s_addr = str_to_addr(address);
	return gethostbyaddr((char *) &addr, 4, AF_INET);	/* IPv4 only for now */
}

/* lookup the default nameserver and display it */
static inline void server_print(void)
{
	struct sockaddr_in def = _res.nsaddr_list[0];
	char *ip = inet_ntoa(def.sin_addr);


	hostent_fprint(gethostbyaddr_wrapper(ip), "Server:");
	printf("\n");
}

/* alter the global _res nameserver structure to use
   an explicit dns server instead of what is in /etc/resolv.h */
static inline void set_default_dns(char *server)
{
	struct in_addr server_in_addr;

	if(inet_aton(server,&server_in_addr))
	{
		_res.nscount = 1;
		_res.nsaddr_list[0].sin_addr = server_in_addr;
	}
}

/* naive function to check whether char *s is an ip address */
static int is_ip_address(const char *s)
{
	while (*s) {
		if ((isdigit(*s)) || (*s == '.')) {
			s++;
			continue;
		}
		return 0;
	}
	return 1;
}

/* ________________________________________________________________________ */


#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
unsigned int get_curr_timestamp_ms()
{
        unsigned int timestamp = 0;

        struct timespec ts;
        int rc;

        rc = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (rc == 0)
        {
                timestamp =ts.tv_sec*1000 + ts.tv_nsec/(1000*1000);
        }
        else
        {
                timestamp = time(0)*1000;
        }

	printf(" ts.tv_sec:%d  ts.tv_nsec:%d timestamp:%d\n",ts.tv_sec,  ts.tv_nsec,timestamp);
	return timestamp;


}

static struct hostent *hostent_fprint_with_responsetime(struct hostent *host, const char *server_host, unsigned int responsetime)
{
	if (host) {
		printf("%s:%d\n", "responsetime", responsetime);
		printf("%s:%s\n", server_host, host->h_name);
		addr_list_fprint(host->h_addr_list);
	} else {
//		printf("%s:%d\n", "responsetime", responsetime);
//		printf("%s:%s\n", server_host, host->h_name);
		printf("resolve failed\n");
	}
	return host;
}

#endif

#if defined (AEI_VDSL_CUSTOMER_BELLALIANT)
void AEI_sendEventMessageForNsloopup(int flag)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(int)]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;
   int *nslookupBody = (int*) (msg+1);
   CmsRet ret;

   *nslookupBody=flag;

   msg->type = CMS_MSG_NSLOOKUP_BACKOFF_RETURN;
   msg->src = EID_NSLOOKUP;
   msg->dst = EID_SSK;
   msg->flags_event = 1;
   msg->dataLength = sizeof(int);

   
   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_NSLOOKUP_BACKOFF_RETURN to SSK, ret=%d", ret);
   }
   else
   {
      cmsLog_error("sent out CMS_MSG_NSLOOKUP_BACKOFF_RETURN (state=%d) to SSK", flag);
   }
   cmsMsg_cleanup(&msgHandle);
   return;
}

//BRCM end
#endif

#ifdef AEI_FRONTIER_V2200H
int nslookup_main(int argc, char **argv)
{

	struct hostent *host;

	/*
	* initialize DNS structure _res used in printing the default
	* name server and in the explicit name server option feature.
	*/

	
	res_init();

	/*
	* We allow 1 or 2 arguments.
	* The first is the name to be looked up and the second is an
	* optional DNS server with which to do the lookup.
	* More than 3 arguments is an error to follow the pattern of the
	* standard nslookup
	*/

	if (argc < 2 || *argv[1]=='-' || argc > 3)
		bb_show_usage();
	else if(argc == 3)
		set_default_dns(argv[2]);

	if (is_ip_address(argv[1])) {
		host = gethostbyaddr_wrapper(argv[0]);
		printf("Name:       %s\n", argv[1]);
		printf("Address:    %s\n", argv[1]);
		return EXIT_SUCCESS;
	} else {
		host = xgethostbyname(argv[1]);
	}
	hostent_fprint(host, "Name:  ");
	if (host) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

#else
int nslookup_main(int argc, char **argv)
{
	struct hostent *host;

	/*
	* initialize DNS structure _res used in printing the default
	* name server and in the explicit name server option feature.
	*/

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
	unsigned int timestamp = 0;
	unsigned int responsetime = 0;
#endif

	res_init();

#if defined(AEI_VDSL_SMARTLED) || defined (AEI_VDSL_CUSTOMER_BELLALIANT)
    cmsLog_init(EID_NSLOOKUP);
    cmsLog_setLevel(DEFAULT_LOG_LEVEL);
    cmsMsg_init(EID_NSLOOKUP, &msgHandle);
#endif

	/*
	* We allow 1 or 2 arguments.
	* The first is the name to be looked up and the second is an
	* optional DNS server with which to do the lookup.
	* More than 3 arguments is an error to follow the pattern of the
	* standard nslookup
	*/

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
	if (argc < 2 || *argv[1]=='-' || argc > 4)
		bb_show_usage();
	else if(argc==3 || argc==4)
		set_default_dns(argv[2]);
#else
	if (argc < 2 || *argv[1]=='-' || argc > 3)
		bb_show_usage();
	else if(argc == 3)
		set_default_dns(argv[2]);
#endif

#if defined(AEI_VDSL_SMARTLED)
	if (is_ip_address(argv[1])) {
		host = gethostbyaddr_wrapper(argv[1]);
	} else {
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
		if(argc==4 && !strcmp(argv[3], "nslookup_tmp")){
			timestamp = get_curr_timestamp_ms();
			host = gethostbyname(argv[1]);
			responsetime =  get_curr_timestamp_ms() - timestamp;
			hostent_fprint_with_responsetime(host, "hostname", responsetime);
		}else
#endif
		host = AEI_gethostbyname_txt(argv[1]);
	}
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
	if(argc==4 && !strcmp(argv[3], "nslookup_tmp")){
		AEI_sendNslookupResult(msgHandle);
	}else
#endif
	AEI_sendTxtRecordForNslookup(msgHandle);
	AEI_cleanup();

	if (host) {
#if defined (AEI_VDSL_CUSTOMER_BELLALIANT)	    
	    if(!strcmp(argv[1], "www.microsoft.com")||!strcmp(argv[1], "www.google.ca")||!strcmp(argv[1], "www.bellaliant.net"))
	        AEI_sendEventMessageForNsloopup(1);
#endif	    
		return EXIT_SUCCESS;
	}
#if defined (AEI_VDSL_CUSTOMER_BELLALIANT)	    
	if(!strcmp(argv[1], "www.microsoft.com")||!strcmp(argv[1], "www.google.ca")||!strcmp(argv[1], "www.bellaliant.net"))
	    AEI_sendEventMessageForNsloopup(0);
#endif		
	return EXIT_FAILURE;
#else
	if (is_ip_address(argv[1])) {
		host = gethostbyaddr_wrapper(argv[1]);
	} else {
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
		if(argc==4 && !strcmp(argv[3], "nslookup_tmp")){
			timestamp = get_curr_timestamp_ms();
			host = gethostbyname(argv[1]);
			responsetime =  get_curr_timestamp_ms() - timestamp;
			hostent_fprint_with_responsetime(host, "hostname", responsetime);
		}else
#endif
		host = xgethostbyname(argv[1]);
	}
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
	if(argc==4 && !strcmp(argv[3], "nslookup_tmp")){
		AEI_sendNslookupResult(msgHandle);
	}else
#endif
	hostent_fprint(host, "Name:  ");
	if (host) {
#if defined (AEI_VDSL_CUSTOMER_BELLALIANT)	    
	    if(!strcmp(argv[1], "www.microsoft.com")||!strcmp(argv[1], "www.google.ca")||!strcmp(argv[1], "www.bellaliant.net"))
	        AEI_sendEventMessageForNsloopup(1);
#endif	
		return EXIT_SUCCESS;
	}
#if defined (AEI_VDSL_CUSTOMER_BELLALIANT)	    
	if(!strcmp(argv[1], "www.microsoft.com")||!strcmp(argv[1], "www.google.ca")||!strcmp(argv[1], "www.bellaliant.net"))
	    AEI_sendEventMessageForNsloopup(0);
#endif
	return EXIT_FAILURE;
#endif
}
#endif

/* $Id: nslookup.c,v 1.31 2004/01/30 22:40:05 andersen Exp $ */
