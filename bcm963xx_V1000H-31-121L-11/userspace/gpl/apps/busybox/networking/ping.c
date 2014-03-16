/* vi: set sw=4 ts=4: */
/*
 * $Id: ping.c,v 1.55 2003/07/22 08:56:51 andersen Exp $
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
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/signal.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "busybox.h"

#ifdef BRCM_CMS_BUILD
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
#define O_LOG           2
#define O_BRCM_MSG      4
#define PING_PID_FILE   "/var/pingPid"
#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))
#define PING_IN_PROGRESS  0
#define PING_FINISHED      1
#define PING_ERROR        2  /* ping process exits on error */
#define PING_UNKNOWN_HOST 3  /* ping unable to resolve host name */

//BRCM begin
/* store ping statistics to a file */
FILE *pingFile=NULL;
FILE *pingPid=NULL;


#ifdef BRCM_CMS_BUILD
#ifdef BRCM_OMCI
static UINT16 tcID = 0;
static OmciPingDataMsgBody omciMsg;
#endif
static void *msgHandle=NULL;
static CmsEntityId requesterId=0;

#ifdef AEI_VDSL_CUSTOMER_NCS
char *pingHost=NULL;
#endif
// brcm
/* this is call to send message back to SMD to relay to interested party about the
 * statistic of the most recent completed or stopped PING test */
void sendEventMessage(int finish, char *host, int ntransmitted, int nreceived, int tmin,
                      int tmax, int tsum)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(PingDataMsgBody)]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;
   PingDataMsgBody *pingBody = (PingDataMsgBody*) (msg+1);
   CmsRet ret;

   cmsLog_debug("finish %d, ip %s, nTransmited/receive/tmin/tmax/tsum %d/%d/%d/%d/%d",
                finish,host,ntransmitted,nreceived,tmin,tmax,tsum);

   msg->type = CMS_MSG_PING_STATE_CHANGED;
   msg->src = EID_PING;
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
#ifdef AEI_VDSL_CUSTOMER_NCS
   else if (finish == PING_ERROR || finish == PING_UNKNOWN_HOST)
#else
   else if (finish == PING_ERROR)
#endif
   {
      /* none, no status to report because ping process has error and die */
      sprintf(pingBody->diagnosticsState,MDMVS_ERROR_CANNOTRESOLVEHOSTNAME); 
   }

   //   sprintf(pingBody->host,ip); 
   sprintf(pingBody->host,host); 
   pingBody->requesterId = requesterId;
   pingBody->successCount = nreceived;
   pingBody->failureCount = (ntransmitted-nreceived);
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

#ifdef BRCM_OMCI
void sendOmciEventMessage(int finish)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(OmciPingDataMsgBody)]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;
   OmciPingDataMsgBody *pingBody = (OmciPingDataMsgBody*) (msg+1);
   CmsRet ret;

   if (finish == PING_IN_PROGRESS)
      return;

   cmsLog_debug("finish %d", finish);

   msg->type = CMS_MSG_PING_STATE_CHANGED;
   msg->src = EID_PING;
   msg->dst = EID_OMCIPMD;
   msg->flags_event = 1;
   msg->dataLength = sizeof(OmciPingDataMsgBody);

   omciMsg.tcid = tcID;
   if (finish == PING_FINISHED)
   {
      omciMsg.result = CMSRET_SUCCESS;
   }
   else 
   {
      /* none, no status to report because ping process has error and die */
      omciMsg.result = CMSRET_INTERNAL_ERROR;
   }
   memcpy(pingBody, &omciMsg, sizeof(omciMsg));

   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_PING_STATE_CHANGED to OMCIPMD, ret=%d", ret);
   }
   else
   {
      cmsLog_notice("sent out CMS_MSG_PING_STATE_CHANGED (finish=%d) to OMCIPMD", finish);
   }
}
#endif

#endif  /* BRCM_CMS_BUILD */

#ifndef AEI_VDSL_CUSTOMER_NCS
static void ping(const char *host);
#endif
/* common routines */
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

/* simple version */
#ifndef CONFIG_FEATURE_FANCY_PING
static char *hostname = NULL;
static void noresp(int ign)
{
	printf("No response from %s\n", hostname);
	exit(EXIT_FAILURE);
}

static void ping(const char *host)
{
	struct hostent *h;
	struct sockaddr_in pingaddr;
	struct icmp *pkt;
	int pingsock, c;
	char packet[DEFDATALEN + MAXIPLEN + MAXICMPLEN];

	pingsock = create_icmp_socket();

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin_family = AF_INET;
	h = xgethostbyname(host);
	memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
	hostname = h->h_name;

	pkt = (struct icmp *) packet;
	memset(pkt, 0, sizeof(packet));
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

	if (c < 0 || c != sizeof(packet))
        {
                logStat(PING_ERROR);
		bb_perror_msg_and_die("sendto");
        }

	signal(SIGALRM, noresp);
	alarm(5);					/* give the host 5000ms to respond */
	/* listen for replies */
	while (1) {
		struct sockaddr_in from;
		size_t fromlen = sizeof(from);

		if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
						  (struct sockaddr *) &from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
			bb_perror_msg("recvfrom");
			continue;
		}
		if (c >= 76) {			/* ip + icmp */
			struct iphdr *iphdr = (struct iphdr *) packet;

			pkt = (struct icmp *) (packet + (iphdr->ihl << 2));	/* skip ip hdr */
			if (pkt->icmp_type == ICMP_ECHOREPLY)
				break;
		}
	}
	printf("%s is alive!\n", hostname);
	return;
}

extern int ping_main(int argc, char **argv)
{
	argc--;
	argv++;
	if (argc < 1)
		bb_show_usage();

#ifdef BRCM_CMS_BUILD
   cmsLog_init(EID_PING);
   cmsLog_setLevel(DEFAULT_LOG_LEVEL);
   cmsMsg_init(EID_PING, &msgHandle);
#endif

	ping(*argv);
	return EXIT_SUCCESS;
}

#else /* ! CONFIG_FEATURE_FANCY_PING */
/* full(er) version */
static struct sockaddr_in pingaddr;
static int pingsock = -1;
static int datalen; /* intentionally uninitialized to work around gcc bug */

// brcm: changed default value of pingcount from 0 to 4.
static long ntransmitted, nreceived, nrepeats, pingcount=4;
static int myid, options;
static unsigned long tmin = ULONG_MAX, tmax, tsum;
static char rcvd_tbl[MAX_DUP_CHK / 8];

struct hostent *hostent;

static void sendping(int);
static void pingstats(int);
static void unpack(char *, int, struct sockaddr_in *);

#ifdef BRCM_CMS_BUILD
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
  char ip[16];
  struct in_addr addr;

#ifdef AEI_VDSL_CUSTOMER_NCS
  if (hostent != NULL) {
     memcpy(&addr, hostent->h_addr, sizeof(addr));
     strncpy(ip,inet_ntoa(addr),16);
  }
#else
  memcpy(&addr, hostent->h_addr, sizeof(addr));
  strncpy(ip,inet_ntoa(addr),16);
#endif

#ifdef BRCM_CMS_BUILD
   /* 
    * Only call sendEventMessage if msgHandle to smd was successfully initialized.
    */
   if (msgHandle != NULL)
   {
#ifdef BRCM_OMCI
      if (tcID)
         sendOmciEventMessage(finish);
      else
#endif

#ifdef AEI_VDSL_CUSTOMER_NCS
      if (hostent != NULL)
         sendEventMessage(finish,hostent->h_name,ntransmitted,nreceived,tmin,tmax,tsum);
      else 
         sendEventMessage(finish,pingHost,ntransmitted,nreceived,tmin,tmax,tsum);
#else
      sendEventMessage(finish,hostent->h_name,ntransmitted,nreceived,tmin,tmax,tsum);
#endif
   }
#endif
}

/**************************************************************************/

static void pingstats(int junk)
{
	int status;

	signal(SIGINT, SIG_IGN);
    if (!(options & O_QUIET))
    {
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
    }
	if (nreceived != 0)
		status = EXIT_SUCCESS;
	else
		status = EXIT_FAILURE;

   //BRCM begin
#ifdef BRCM_CMS_BUILD
   logStat(PING_FINISHED);
#if defined(AEI_VDSL_CUSTOMER_NCS)
   sleep(1);
#endif
   cmsCleanup();
#else
   if (options & O_LOG) {
      logStat(PING_FINISHED);
      remove_file(PING_PID_FILE,FILEUTILS_FORCE);
   }
#endif /* BRCM_CMS_BUILD */
	exit(status);
   //BRCM end
}

static void sendping(int junk)
{
	struct icmp *pkt;
	int i;
	char packet[datalen + 8];

	pkt = (struct icmp *) packet;

	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	CLR(pkt->icmp_seq % MAX_DUP_CHK);

	gettimeofday((struct timeval *) &packet[8], NULL);
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	i = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

	if (i < 0)
   {
        logStat(PING_ERROR);
#ifdef BRCM_CMS_BUILD
        cmsCleanup();
#endif
		bb_perror_msg_and_die("sendto");
   }
	else if ((size_t)i != sizeof(packet))
   {
        logStat(PING_ERROR);
#ifdef BRCM_CMS_BUILD
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

static char *icmp_type_name (int id)
{
	switch (id) {
	case ICMP_ECHOREPLY: 		return "Echo Reply";
	case ICMP_DEST_UNREACH: 	return "Destination Unreachable";
	case ICMP_SOURCE_QUENCH: 	return "Source Quench";
	case ICMP_REDIRECT: 		return "Redirect (change route)";
	case ICMP_ECHO: 			return "Echo Request";
	case ICMP_TIME_EXCEEDED: 	return "Time Exceeded";
	case ICMP_PARAMETERPROB: 	return "Parameter Problem";
	case ICMP_TIMESTAMP: 		return "Timestamp Request";
	case ICMP_TIMESTAMPREPLY: 	return "Timestamp Reply";
	case ICMP_INFO_REQUEST: 	return "Information Request";
	case ICMP_INFO_REPLY: 		return "Information Reply";
	case ICMP_ADDRESS: 			return "Address Mask Request";
	case ICMP_ADDRESSREPLY: 	return "Address Mask Reply";
	default: 					return "unknown ICMP type";
	}
}

static void unpack(char *buf, int sz, struct sockaddr_in *from)
{
	struct icmp *icmppkt;
	struct iphdr *iphdr;
	struct timeval tv, *tp;
	int hlen, dupflag;
	unsigned long triptime;

	gettimeofday(&tv, NULL);

	/* check IP header */
	iphdr = (struct iphdr *) buf;
	hlen = iphdr->ihl << 2;
	/* discard if too short */
	if (sz < (datalen + ICMP_MINLEN))
		return;

	sz -= hlen;
	icmppkt = (struct icmp *) (buf + hlen);

	if (icmppkt->icmp_id != myid)
	    return;				/* not our ping */

	if (icmppkt->icmp_type == ICMP_ECHOREPLY) {
	    ++nreceived;
		tp = (struct timeval *) icmppkt->icmp_data;

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

		if (TST(icmppkt->icmp_seq % MAX_DUP_CHK)) {
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} else {
			SET(icmppkt->icmp_seq % MAX_DUP_CHK);
			dupflag = 0;
		}

#ifndef BRCM_CMS_BUILD
      /* we don't update this periodically */
      //BRCM begin
      if (options & O_LOG) {
         logStat(PING_IN_PROGRESS);
         usleep(1);
      } /* O_LOG */
#else
#ifdef BRCM_OMCI
      if (icmppkt->icmp_seq < pingcount) {
          omciMsg.msg.responseTime[icmppkt->icmp_seq] = (triptime < 10) ? 1 : (triptime/10);
      }
#endif
      logStat(PING_IN_PROGRESS);
#endif /* endif */
      //BRCM end

		if (options & O_QUIET)
			return;

                /*brcm: changed display message to show actually receive data bytes length
                  rather than ICMP packet length which is ICMP_MINLEN+dataLen */
		printf("%d bytes from %s: icmp_seq=%u", (sz-ICMP_MINLEN),
			   inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr),
			   icmppkt->icmp_seq);
		printf(" ttl=%d", iphdr->ttl);
		printf(" time=%lu.%lu ms", triptime / 10, triptime % 10);
		if (dupflag)
			printf(" (DUP!)");
		printf("\n");
	} else 
    {
	   if (icmppkt->icmp_type != ICMP_ECHO)
       {
#if defined(BRCM_CMS_BUILD) && defined(BRCM_OMCI)
          if (tcID)
          {
              memcpy(omciMsg.msg.icmpReply, &buf[hlen], sizeof(omciMsg.msg.icmpReply));
              logStat(PING_ERROR);
              cmsCleanup();
              bb_error_msg_and_die("Warning: Got ICMP %d (%s)",
              icmppkt->icmp_type, icmp_type_name (icmppkt->icmp_type));
          }
          else
#endif
          bb_error_msg("Warning: Got ICMP %d (%s)",
            icmppkt->icmp_type, icmp_type_name (icmppkt->icmp_type));
       }
    }
}
#ifdef AEI_VDSL_CUSTOMER_NCS
static void ping(const char *host,unsigned int dscp)
#else
static void ping(const char *host)
#endif
{
	char packet[datalen + MAXIPLEN + MAXICMPLEN];
	int sockopt;

	pingsock = create_icmp_socket();
	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin_family = AF_INET;

	/*While hostent is NULL, we need to send msg to caller*/
#ifdef AEI_VDSL_CUSTOMER_NCS
	hostent = gethostbyname(host);
       
        if (hostent == NULL || (hostent != NULL && hostent->h_addrtype != AF_INET)) 
#else
	hostent = xgethostbyname(host);

	if (hostent->h_addrtype != AF_INET)
#endif
   {
        logStat(PING_UNKNOWN_HOST);
#ifndef AEI_VDSL_CUSTOMER_NCS
#ifdef BRCM_CMS_BUILD
        cmsCleanup();
#endif
#endif
#ifdef AEI_VDSL_CUSTOMER_NCS
        usleep(10000); //wait for send msg finished
#endif
#ifdef AEI_VDSL_CUSTOMER_NCS
        if (hostent == NULL)
            bb_error_msg_and_die("Unknown Host.");
        else
            bb_error_msg_and_die("unknown address type; only AF_INET is currently supported.");
#else
		bb_error_msg_and_die("unknown address type; only AF_INET is currently supported.");
#endif
   }
	memcpy(&pingaddr.sin_addr, hostent->h_addr, sizeof(pingaddr.sin_addr));

	/* enable broadcast pings */
	sockopt = 1;
	setsockopt(pingsock, SOL_SOCKET, SO_BROADCAST, (char *) &sockopt,
			   sizeof(sockopt));

	/* set recv buf for broadcast pings */
	sockopt = 48 * 1024;
	setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, (char *) &sockopt,
			   sizeof(sockopt));
#ifdef AEI_VDSL_CUSTOMER_NCS
	if (dscp > 0)
	{
		unsigned int val = dscp << 2;
		if (setsockopt(pingsock, IPPROTO_IP, IP_TOS, 
				(char *)&val, sizeof(val)) < 0)
		{
			bb_perror_msg("set Qos parameter DSCP %u failed", dscp);
		}
	}
#endif
    if (!(options & O_QUIET))
	  printf("PING %s (%s): %d data bytes\n",
	             hostent->h_name,
		     inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
		     datalen);

	signal(SIGINT, pingstats);

	/* start the ping's going ... */
	sendping(0);

	/* listen for replies */
	while (1) {
		struct sockaddr_in from;
		socklen_t fromlen = (socklen_t) sizeof(from);
		int c;

		if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
						  (struct sockaddr *) &from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
			bb_perror_msg("recvfrom");
			continue;
		}
		unpack(packet, c, &from);
		if (pingcount > 0 && nreceived >= pingcount)
			break;
	}

	pingstats(0);
}

extern int ping_main(int argc, char **argv)
{
  //        FILE *fd;
	char *thisarg;
#ifdef AEI_VDSL_CUSTOMER_NCS
    unsigned int dscp=0;
#endif
	datalen = DEFDATALEN; /* initialized here rather than in global scope to work around gcc bug */

	argc--;
	argv++;
	options = 0;
	/* Parse any options */
	while (argc >= 1 && **argv == '-') {
		thisarg = *argv;
		thisarg++;
		switch (*thisarg) {
		case 'q':
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
      //BRCM begin.  Is this option used anymore?
		case 'l':
			if ((pingPid = fopen (PING_PID_FILE, "w")) != NULL) {
					fprintf(pingPid,"%d\n",getpid());
					(void)fclose(pingPid);
			}
			/* log statistics to file */
			options |= O_LOG;
			break;
#ifdef BRCM_CMS_BUILD
		case 't':
			if (--argc <= 0)
				bb_show_usage();
			argv++;
			requesterId = atoi(*argv);
			break;
		case 'm':		
			options |= O_BRCM_MSG;
			break;         
#ifdef BRCM_OMCI
        case 'i':
			if (--argc <= 0)
				bb_show_usage();
			argv++;
			tcID = atoi(*argv);
            memset(&omciMsg, 0, sizeof(OmciPingDataMsgBody));
            break;
#endif
#endif
		//BRCM end
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

#ifdef BRCM_CMS_BUILD
   cmsLog_init(EID_PING);
   cmsLog_setLevel(DEFAULT_LOG_LEVEL);
   if (options & O_BRCM_MSG)
   {
      cmsMsg_init(EID_PING, &msgHandle);
   }
   cmsLog_debug("starting ping argv %s",*argv);
#endif

	myid = getpid() & 0xFFFF;
#ifdef AEI_VDSL_CUSTOMER_NCS
   if (*argv != NULL)
       pingHost = strdup(*argv);
    ping(*argv,dscp);
#else  
	ping(*argv);
#endif

   if (options & O_LOG) {
      remove_file(PING_PID_FILE,FILEUTILS_FORCE);
   }
	return EXIT_SUCCESS;
}
#endif /* ! CONFIG_FEATURE_FANCY_PING */

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
