/* vi: set sw=4 ts=4: */
/*
 * $Id: ping6.c,v 1.5 2003/05/22 07:10:22 andersen Exp $
 * Mini ping implementation for busybox
 *
 * Copyright (C) 1999 by Randolph Chung <tausq@debian.org>
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
 * This version of ping is adapted from the ping in netkit-base 0.10,
 * which is:
 *
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Original copyright notice is retained at the end of this file.
 *
 * This version is an adaptation of ping.c from busybox.
 * The code was modified by Bart Visscher <magick@linux-fan.com>
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/signal.h>

#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>				/* offsetof */
#include "busybox.h"
#ifdef AEI_CONTROL_PING6
#include "cms_msg.h"
#include "cms_util.h"
#include "cms_log.h"
#endif
static const int DEFDATALEN = 56;
static const int MAXIPLEN = 60;
static const int MAXICMPLEN = 76;
static const int MAXPACKET = 65468;
#define	MAX_DUP_CHK	(8 * 128)
static const int MAXWAIT = 10;
static const int PINGINTERVAL = 1;		/* second */

#define O_QUIET         (1 << 0)
#define O_VERBOSE       (1 << 1)
#define O_LOG           2
#define O_BRCM_MSG      4
#define PING_PID_FILE   "/var/ping6Pid"

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))
#define PING_IN_PROGRESS  0
#define PING_FINISHED      1
#define PING_ERROR        2  /* ping process exits on error */
#define PING_UNKNOWN_HOST 3  /* ping unable to resolve host name */

FILE *ping6Pid=NULL;

#ifdef AEI_CONTROL_PING6
static void *msgHandle=NULL;
static CmsEntityId requesterId=0;

char *ping6Host=NULL;

// brcm
/* this is call to send message back to SMD to relay to interested party about the
 * statistic of the most recent completed or stopped PING test */
void sendPing6EventMessage(int finish, char *host, int ntransmitted, int nreceived, int tmin,
                      int tmax, int tsum)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(PingDataMsgBody)]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;
   PingDataMsgBody *pingBody = (PingDataMsgBody*) (msg+1);
   CmsRet ret;

   cmsLog_error("finish %d, ip %s, nTransmited/receive/tmin/tmax/tsum %d/%d/%d/%d/%d",
                finish,host,ntransmitted,nreceived,tmin,tmax,tsum);

   msg->type = CMS_MSG_PING_STATE_CHANGED;
   msg->src = EID_PING6;
   msg->dst = EID_SSK;
   msg->flags_event = 1;
   msg->dataLength = sizeof(PingDataMsgBody);

   if (finish == PING_FINISHED)
   {
      sprintf(pingBody->diagnosticsState,MDMVS_COMPLETE); 
   }
   else if (finish == PING_IN_PROGRESS)
   {
      sprintf(pingBody->diagnosticsState,MDMVS_PING_INPROGRESS); 
   }
   else if (finish == PING_ERROR || finish == PING_UNKNOWN_HOST)
   {
      /* none, no status to report because ping process has error and die */
      sprintf(pingBody->diagnosticsState,MDMVS_ERROR_CANNOTRESOLVEHOSTNAME); 
   }

   //   sprintf(pingBody->host,ip); 
   sprintf(pingBody->host,host); 
   pingBody->requesterId = requesterId;
   pingBody->successCount = nreceived;
   pingBody->failureCount = (ntransmitted-nreceived);
    cmsLog_error("requesterId=%d,successCount=%d,failureCount=%d",
    pingBody->requesterId,pingBody->successCount,pingBody->failureCount);
    
   if (nreceived)
   {
#ifndef CONFIG_FEATURE_FANCY_PING
      pingBody->averageResponseTime = (tsum / ((nreceived + nrepeats)*10));
#else
      pingBody->averageResponseTime = (tsum / (nreceived *10 ));
#endif
      pingBody->minimumResponseTime = tmin/10;
      pingBody->maximumResponseTime = tmax/10;
   }
   else 
   {
      pingBody->averageResponseTime = 0;
      pingBody->minimumResponseTime = 0;
      pingBody->maximumResponseTime = 0;
   }
cmsLog_error("averageResponseTime=%d,minimumResponseTime=%d,maximumResponseTime=%d",
    pingBody->averageResponseTime,pingBody->minimumResponseTime,pingBody->maximumResponseTime);


   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_PING_STATE_CHANGED to SSK, ret=%d", ret);
   }
   else
   {
      cmsLog_notice("sent out CMS_MSG_PING_STATE_CHANGED (finish=%d) to SSK", finish);
   }
   if (requesterId != 0)
   {
      msg->dst = requesterId;
      if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
      {
         cmsLog_error("could not send out CMS_MSG_PING_STATE_CHANGED to requestId %d, ret=%d", ret,(int)requesterId);
      }
      else
      {
         cmsLog_notice("sent out CMS_MSG_PING_STATE_CHANGED (finish=%d) to requesterId %d", finish,(int)requesterId);
      }   
   }
   return;
}


#endif  /* AEI_CONTROL_PING6 */

#ifdef AEI_VDSL_CUSTOMER_NCS
static void ping(const char *host,unsigned int dscp);
#else
static void ping(const char *host);
#endif
/* simple version */
#ifndef CONFIG_FEATURE_FANCY_PING6
static struct hostent *h;

void noresp(int ign)
{
	printf("No response from %s\n", h->h_name);
	exit(EXIT_FAILURE);
}

static void ping(const char *host)
{
	struct sockaddr_in6 pingaddr;
	struct icmp6_hdr *pkt;
	int pingsock, c;
	int sockopt;
	char packet[DEFDATALEN + MAXIPLEN + MAXICMPLEN];

	pingsock = create_icmp6_socket();

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin6_family = AF_INET6;
	h = xgethostbyname2(host, AF_INET6);
	memcpy(&pingaddr.sin6_addr, h->h_addr, sizeof(pingaddr.sin6_addr));

	pkt = (struct icmp6_hdr *) packet;
	memset(pkt, 0, sizeof(packet));
	pkt->icmp6_type = ICMP6_ECHO_REQUEST;

	sockopt = offsetof(struct icmp6_hdr, icmp6_cksum);
	setsockopt(pingsock, SOL_RAW, IPV6_CHECKSUM, (char *) &sockopt,
			   sizeof(sockopt));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in6));

	if (c < 0 || c != sizeof(packet))
		bb_perror_msg_and_die("sendto");

	signal(SIGALRM, noresp);
	alarm(5);					/* give the host 5000ms to respond */
	/* listen for replies */
	while (1) {
		struct sockaddr_in6 from;
		size_t fromlen = sizeof(from);

		if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
						  (struct sockaddr *) &from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
			bb_perror_msg("recvfrom");
			continue;
		}
		if (c >= 8) {			/* icmp6_hdr */
			pkt = (struct icmp6_hdr *) packet;
			if (pkt->icmp6_type == ICMP6_ECHO_REPLY)
				break;
		}
	}
	printf("%s is alive!\n", h->h_name);
	return;
}

extern int ping6_main(int argc, char **argv)
{
	argc--;
	argv++;
	if (argc < 1)
		bb_show_usage();
	ping(*argv);
	return EXIT_SUCCESS;
}

#else /* ! CONFIG_FEATURE_FANCY_PING6 */
/* full(er) version */
static struct sockaddr_in6 pingaddr;
static int pingsock = -1;
static int datalen; /* intentionally uninitialized to work around gcc bug */
static char* ifname;

static long ntransmitted, nreceived, nrepeats, pingcount;
static int myid, options;
static unsigned long tmin = ULONG_MAX, tmax, tsum;
static char rcvd_tbl[MAX_DUP_CHK / 8];

# ifdef CONFIG_FEATURE_FANCY_PING
extern
# endif
	struct hostent *hostent;

static void sendping(int);
static void pingstats(int);
static void unpack(char *, int, struct sockaddr_in6 *, int);

/**************************************************************************/
#ifdef AEI_CONTROL_PING6
static void cmsCleanup(void)
{
   if (options & O_BRCM_MSG)
   {
      cmsMsg_cleanup(&msgHandle);
   }
   cmsLog_cleanup();
}
#endif
static void logStat(int finish)
{
  char ipv6_addr[44];
  struct in6_addr addr;

#ifdef AEI_CONTROL_PING6
  if (hostent != NULL) {
     memcpy(&addr, hostent->h_addr, sizeof(addr));
     inet_ntop(AF_INET6, &addr, ipv6_addr, sizeof(ipv6_addr));
  }
#else
  memcpy(&addr, hostent->h_addr, sizeof(addr));
  inet_ntop(AF_INET6, &addr, ipv6_addr, sizeof(ipv6_addr));
#endif

#ifdef AEI_CONTROL_PING6
   /* 
    * Only call sendPing6EventMessage if msgHandle to smd was successfully initialized.
    */
   if (msgHandle != NULL)
   {
      if (hostent != NULL)
         sendPing6EventMessage(finish,hostent->h_name,ntransmitted,nreceived,tmin,tmax,tsum);
      else 
         sendPing6EventMessage(finish,ping6Host,ntransmitted,nreceived,tmin,tmax,tsum);
   }
#endif
}
static void pingstats(int junk)
{
	int status;

	signal(SIGINT, SIG_IGN);

	printf("\n--- %s ping statistics ---\n", hostent->h_name);
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received, ", nreceived);
	if (nrepeats)
		printf("%ld duplicates, ", nrepeats);
	if (ntransmitted)
		printf("%ld%% packet loss\n",
			   (ntransmitted - nreceived) * 100 / ntransmitted);
	if (nreceived)
		printf("round-trip min/avg/max = %lu.%lu/%lu.%lu/%lu.%lu ms\n",
			   tmin / 10, tmin % 10,
			   (tsum / (nreceived + nrepeats)) / 10,
			   (tsum / (nreceived + nrepeats)) % 10, tmax / 10, tmax % 10);
	if (nreceived != 0)
		status = EXIT_SUCCESS;
	else
		status = EXIT_FAILURE;
#ifdef AEI_CONTROL_PING6
   logStat(PING_FINISHED);
   cmsCleanup();
#else
   if (options & O_LOG) {
      logStat(PING_FINISHED);
      remove_file(PING_PID_FILE,FILEUTILS_FORCE);
   }
#endif /* AEI_CONTROL_PING6 */
    
	exit(status);
}

static void sendping(int junk)
{
	struct icmp6_hdr *pkt;
	int i;
	char packet[datalen + 8];

	pkt = (struct icmp6_hdr *) packet;

	pkt->icmp6_type = ICMP6_ECHO_REQUEST;
	pkt->icmp6_code = 0;
	pkt->icmp6_cksum = 0;
	pkt->icmp6_seq = ntransmitted++;
	pkt->icmp6_id = myid;
	CLR(pkt->icmp6_seq % MAX_DUP_CHK);

	gettimeofday((struct timeval *) &pkt->icmp6_data8[4], NULL);

	i = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in6));

	if (i < 0)
   {
        logStat(PING_ERROR);
#ifdef AEI_CONTROL_PING6
        cmsCleanup();
#endif
		bb_perror_msg_and_die("sendto");
   }     
	else if ((size_t)i != sizeof(packet))
   {
        logStat(PING_ERROR);
#ifdef AEI_CONTROL_PING6
        cmsCleanup();
#endif
		bb_error_msg_and_die("ping wrote %d chars; %d expected", i,
			   (int)sizeof(packet));
   }     

	signal(SIGALRM, sendping);
	if (pingcount == 0 || ntransmitted < pingcount) {	/* schedule next in 1s */
		alarm(PINGINTERVAL);
	} else {					/* done, wait for the last ping to come back */
		/* todo, don't necessarily need to wait so long... */
		signal(SIGALRM, pingstats);
		alarm(MAXWAIT);
	}
}

static char *icmp6_type_name (int id)
{
	switch (id) {
	case ICMP6_DST_UNREACH:				return "Destination Unreachable";
	case ICMP6_PACKET_TOO_BIG:			return "Packet too big";
	case ICMP6_TIME_EXCEEDED:			return "Time Exceeded";
	case ICMP6_PARAM_PROB:				return "Parameter Problem";
	case ICMP6_ECHO_REPLY:				return "Echo Reply";
	case ICMP6_ECHO_REQUEST:			return "Echo Request";
#if 0 /* incompatiable with newer kernel */
	case ICMP6_MEMBERSHIP_QUERY:		return "Membership Query";
	case ICMP6_MEMBERSHIP_REPORT:		return "Membership Report";
	case ICMP6_MEMBERSHIP_REDUCTION:	return "Membership Reduction";
#endif
	default: 							return "unknown ICMP type";
	}
}

static void unpack(char *packet, int sz, struct sockaddr_in6 *from, int hoplimit)
{
	struct icmp6_hdr *icmppkt;
	struct timeval tv, *tp;
	int dupflag;
	unsigned long triptime;
	char buf[INET6_ADDRSTRLEN];

	gettimeofday(&tv, NULL);

	/* discard if too short */
	if (sz < (datalen + sizeof(struct icmp6_hdr)))
		return;

	icmppkt = (struct icmp6_hdr *) packet;

	if (icmppkt->icmp6_id != myid)
	    return;				/* not our ping */

	if (icmppkt->icmp6_type == ICMP6_ECHO_REPLY) {
	    ++nreceived;
		tp = (struct timeval *) &icmppkt->icmp6_data8[4];

		if ((tv.tv_usec -= tp->tv_usec) < 0) {
			--tv.tv_sec;
			tv.tv_usec += 1000000;
		}
		tv.tv_sec -= tp->tv_sec;

		triptime = tv.tv_sec * 10000 + (tv.tv_usec / 100);
		tsum += triptime;
		if (triptime < tmin)
			tmin = triptime;
		if (triptime > tmax)
			tmax = triptime;

		if (TST(icmppkt->icmp6_seq % MAX_DUP_CHK)) {
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} else {
			SET(icmppkt->icmp6_seq % MAX_DUP_CHK);
			dupflag = 0;
		}
#ifndef AEI_CONTROL_PING6
      /* we don't update this periodically */
      //BRCM begin
      if (options & O_LOG) {
         logStat(PING_IN_PROGRESS);
         usleep(1);
      } /* O_LOG */
#else
      logStat(PING_IN_PROGRESS);
#endif /* endif */

		if (options & O_QUIET)
			return;

		printf("%d bytes from %s: icmp6_seq=%u", sz,
			   inet_ntop(AF_INET6, (struct in_addr6 *) &pingaddr.sin6_addr,
						 buf, sizeof(buf)),
			   icmppkt->icmp6_seq);
		printf(" ttl=%d time=%lu.%lu ms", hoplimit,
			   triptime / 10, triptime % 10);
		if (dupflag)
			printf(" (DUP!)");
		printf("\n");
	} else
		if (icmppkt->icmp6_type != ICMP6_ECHO_REQUEST)
			bb_error_msg("Warning: Got ICMP %d (%s)",
					icmppkt->icmp6_type, icmp6_type_name (icmppkt->icmp6_type));
}

#ifdef AEI_VDSL_CUSTOMER_NCS
static void ping(const char *host,unsigned int dscp)
#else
static void ping(const char *host)
#endif
{
	char packet[datalen + MAXIPLEN + MAXICMPLEN];
	char buf[INET6_ADDRSTRLEN];
	int sockopt;
	struct msghdr msg;
	struct sockaddr_in6 from;
	struct iovec iov;
	char control_buf[CMSG_SPACE(36)];

	pingsock = create_icmp6_socket();

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin6_family = AF_INET6;
#ifdef AEI_CONTROL_PING6
	hostent = gethostbyname2(host, AF_INET6);
       
        if (hostent == NULL || (hostent != NULL && hostent->h_addrtype != AF_INET6)) 
#else
	hostent = xgethostbyname2(host, AF_INET6);
	if (hostent->h_addrtype != AF_INET6)
#endif
   {
        logStat(PING_UNKNOWN_HOST);

#ifdef AEI_CONTROL_PING6
        usleep(10000); //wait for send msg finished
#endif
#ifdef AEI_CONTROL_PING6
        if (hostent == NULL)
            bb_error_msg_and_die("Unknown Host.");
        else
            bb_error_msg_and_die("unknown address type; only AF_INET6 is currently supported.");
#else
		bb_error_msg_and_die("unknown address type; only AF_INET6 is currently supported.");
#endif
    }


	memcpy(&pingaddr.sin6_addr, hostent->h_addr, sizeof(pingaddr.sin6_addr));

#ifdef ICMP6_FILTER
	{
		struct icmp6_filter filt;
		if (!(options & O_VERBOSE)) {
			ICMP6_FILTER_SETBLOCKALL(&filt);
#if 0
			if ((options & F_FQDN) || (options & F_FQDNOLD) ||
				(options & F_NODEADDR) || (options & F_SUPTYPES))
				ICMP6_FILTER_SETPASS(ICMP6_NI_REPLY, &filt);
			else
#endif
				ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filt);
		} else {
			ICMP6_FILTER_SETPASSALL(&filt);
		}
		if (setsockopt(pingsock, IPPROTO_ICMPV6, ICMP6_FILTER, &filt,
					   sizeof(filt)) < 0)
			bb_error_msg_and_die("setsockopt(ICMP6_FILTER)");
	}
#endif /*ICMP6_FILTER*/

	/* enable broadcast pings */
	sockopt = 1;
	setsockopt(pingsock, SOL_SOCKET, SO_BROADCAST, (char *) &sockopt,
			   sizeof(sockopt));

	/* set recv buf for broadcast pings */
	sockopt = 48 * 1024;
	setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, (char *) &sockopt,
			   sizeof(sockopt));

	sockopt = offsetof(struct icmp6_hdr, icmp6_cksum);
	setsockopt(pingsock, SOL_RAW, IPV6_CHECKSUM, (char *) &sockopt,
			   sizeof(sockopt));

	sockopt = 1;
	setsockopt(pingsock, SOL_IPV6, IPV6_HOPLIMIT, (char *) &sockopt,
			   sizeof(sockopt));

	if (ifname) {
		if ((pingaddr.sin6_scope_id = if_nametoindex(ifname)) == 0)
			bb_error_msg_and_die("%s: invalid interface name", ifname);
	}
	
#ifdef AEI_VDSL_CUSTOMER_NCS
	if (dscp > 0)
	{
		unsigned int val = dscp;
		int on = 1;
	    if (setsockopt(pingsock, SOL_IPV6, IPV6_RECVTCLASS,&on, sizeof(on)) < 0)
			bb_perror_msg("set RECVTCLASS failed");
	    if (setsockopt(pingsock, SOL_IPV6, IPV6_TCLASS,&val, sizeof(val)) < 0)
			bb_perror_msg("set TCLASS %u failed", dscp);
	}
#endif
	printf("PING %s (%s): %d data bytes\n",
	           hostent->h_name,
			   inet_ntop(AF_INET6, (struct in_addr6 *) &pingaddr.sin6_addr,
						 buf, sizeof(buf)),
		   datalen);

	signal(SIGINT, pingstats);

	/* start the ping's going ... */
	sendping(0);

	/* listen for replies */
	msg.msg_name=&from;
	msg.msg_namelen=sizeof(from);
	msg.msg_iov=&iov;
	msg.msg_iovlen=1;
	msg.msg_control=control_buf;
	iov.iov_base=packet;
	iov.iov_len=sizeof(packet);
	while (1) {
		int c;
		struct cmsghdr *cmsgptr = NULL;
		int hoplimit=-1;
		msg.msg_controllen=sizeof(control_buf);

		if ((c = recvmsg(pingsock, &msg, 0)) < 0) {
			if (errno == EINTR)
				continue;
			bb_perror_msg("recvfrom");
			continue;
		}
		for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL;
			 cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
			if (cmsgptr->cmsg_level == SOL_IPV6 &&
				cmsgptr->cmsg_type == IPV6_HOPLIMIT ) {
				hoplimit=*(int*)CMSG_DATA(cmsgptr);
			}
		}
		unpack(packet, c, &from, hoplimit);
		if (pingcount > 0 && nreceived >= pingcount)
			break;
	}
	pingstats(0);
}

extern int ping6_main(int argc, char **argv)
{
	char *thisarg;

	datalen = DEFDATALEN; /* initialized here rather than in global scope to work around gcc bug */
#ifdef AEI_VDSL_CUSTOMER_NCS
    unsigned int dscp=0;
#endif
	argc--;
	argv++;
	options = 0;
	/* Parse any options */
	while (argc >= 1 && **argv == '-') {
		thisarg = *argv;
		thisarg++;
		switch (*thisarg) {
		case 'v':
			options &= ~O_QUIET;
			options |= O_VERBOSE;
			break;
		case 'q':
			options &= ~O_VERBOSE;
			options |= O_QUIET;
			break;
		case 'c':
			if (--argc <= 0)
			        bb_show_usage();
			argv++;
			pingcount = atoi(*argv);
			break;
		case 's':
			if (--argc <= 0)
			        bb_show_usage();
			argv++;
			datalen = atoi(*argv);
			break;
		case 'I':
			if (--argc <= 0)
			        bb_show_usage();
			argv++;
			ifname = *argv;
			break;
      //BRCM begin.  Is this option used anymore?
		case 'l':
			if ((ping6Pid = fopen (PING_PID_FILE, "w")) != NULL) {
					fprintf(ping6Pid,"%d\n",getpid());
					(void)fclose(ping6Pid);
			}
			/* log statistics to file */
			options |= O_LOG;
			break;            
#ifdef AEI_CONTROL_PING6
		case 't':
			if (--argc <= 0)
				bb_show_usage();
			argv++;
			requesterId = atoi(*argv);
			break;
		case 'm':		
			options |= O_BRCM_MSG;
			break; 
#endif      
#ifdef AEI_VDSL_CUSTOMER_NCS
		case 'Q':
			if (--argc <= 0)
			        bb_show_usage();
			argv++;
			dscp = atoi(*argv);
			break;
#endif      
		default:
			bb_show_usage();
		}
		argc--;
		argv++;
	}
	if (argc < 1)
		bb_show_usage();
#ifdef AEI_CONTROL_PING6
   cmsLog_init(EID_PING6);
   cmsLog_setLevel(DEFAULT_LOG_LEVEL/*LOG_LEVEL_DEBUG*/);
   if (options & O_BRCM_MSG)
   {
      cmsMsg_init(EID_PING6, &msgHandle);
   }
   //cmsLog_error("starting ping argv %s",*argv);
#endif
	myid = getpid() & 0xFFFF;
#ifdef AEI_CONTROL_PING6
   if (*argv != NULL)
       ping6Host = strdup(*argv);
#endif
#ifdef AEI_VDSL_CUSTOMER_NCS
    ping(*argv,dscp);
#else 
	ping(*argv);
#endif
   if (options & O_LOG) {
      remove_file(PING_PID_FILE,FILEUTILS_FORCE);
   }
	return EXIT_SUCCESS;
}
#endif /* ! CONFIG_FEATURE_FANCY_PING6 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. <BSD Advertising Clause omitted per the July 22, 1999 licensing change
 *		ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change>
 *
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
