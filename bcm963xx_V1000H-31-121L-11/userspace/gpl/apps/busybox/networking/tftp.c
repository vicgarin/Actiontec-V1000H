/* ------------------------------------------------------------------------- */
/* tftp.c                                                                    */
/*                                                                           */
/* A simple tftp client for busybox.                                         */
/* Tries to follow RFC1350.                                                  */
/* Only "octet" mode supported.                                              */
/* Optional blocksize negotiation (RFC2347 + RFC2348)                        */
/*                                                                           */
/* Copyright (C) 2001 Magnus Damm <damm@opensource.se>                       */
/*                                                                           */
/* Parts of the code based on:                                               */
/*                                                                           */
/* atftp:  Copyright (C) 2000 Jean-Pierre Lefebvre <helix@step.polymtl.ca>   */
/*                        and Remi Lefebvre <remi@debian.org>                */
/*                                                                           */
/* utftp:  Copyright (C) 1999 Uwe Ohse <uwe@ohse.de>                         */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          */
/* General Public License for more details.                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "busybox.h"

//#define CONFIG_FEATURE_TFTP_DEBUG

#define TFTP_BLOCKSIZE_DEFAULT 512 /* according to RFC 1350, don't change */
#define TFTP_TIMEOUT 5             /* seconds */

/* opcodes we support */

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5
#define TFTP_OACK  6

static const char *tftp_bb_error_msg[] = {
	"Undefined error",
	"File not found",
	"Access violation",
	"Disk full or allocation error",
	"Illegal TFTP operation",
	"Unknown transfer ID",
	"File already exists",
	"No such user"
};

const int tftp_cmd_get = 1;
const int tftp_cmd_put = 2;

// brcm begin
#include "cms_util.h"
#include "cms_msg.h"

// use some globals to reduce the messiness of change the original tftp function.
char *glbImagePtr = NULL;
int glbUploadSize = 0;
char *glbCurPtr = NULL;
char glbUploadType = 'i';
static void *msgHandle=NULL;
/* 
 * connIfName is the linux interface name that our socket is going out of.
 * We need this information when doing image downloads because we might
 * want to kill all unneeded WAN services to free up memory.
 */
char connIfName[CMS_IFNAME_LENGTH]={0};


int myRead(char *outBuf, int inLen)
{
   int readLen = 0;
   static int xmlCfgLen = 0;
   static int offset = 0;
   static CmsMsgHeader *responseMsg=NULL;
   CmsMsgHeader requestMsg = EMPTY_MSG_HEADER;
   char *cfgStart;
   CmsRet ret;
   
   
   if (responseMsg == NULL) 
   {
      cmsLog_debug("first time, get config file from smd");
      /*
       * This is the first time that we were called.
       * Send a message to smd to request a copy of the config file.
       */
      requestMsg.src = EID_TFTP;
      requestMsg.dst = EID_SMD;
      requestMsg.type = CMS_MSG_GET_CONFIG_FILE;
      requestMsg.flags_request = 1;
      
      if ((ret = cmsMsg_send(msgHandle, &requestMsg)) != CMSRET_SUCCESS)
      {
         cmsLog_error("could not send GET_CONFIG_FILE msg to smd.");
         return -1;
      }

      if ((ret = cmsMsg_receive(msgHandle, &responseMsg)) != CMSRET_SUCCESS)
      {
         cmsLog_error("could not receive GET_CONFIG_FILE msg from smd.");
         CMSMEM_FREE_BUF_AND_NULL_PTR(responseMsg);
         return -1;
      }      

      xmlCfgLen = (int) responseMsg->dataLength;
      cmsLog_debug("got config buffer len=%u", xmlCfgLen);
   }

   /* config data starts immediately after the header */
   cfgStart = (char *) (responseMsg + 1);
   
   if (xmlCfgLen <= inLen)
      readLen = xmlCfgLen;
   else
      readLen = inLen;

   memcpy(outBuf, (cfgStart + offset), readLen);

   xmlCfgLen -= readLen;
   offset += readLen;
   glbUploadSize += readLen;

   if (xmlCfgLen == 0)
   {
      /* done copying all the config data out, free the message */
      CMSMEM_FREE_BUF_AND_NULL_PTR(responseMsg);
      offset = 0;
      cmsLog_debug("send out entire config buf, free msg");
   }

   return readLen;
}


#if (defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)) || defined(AEI_VDSL_CUSTOMER_TELUS)
#define BLOCK_ALLOC 1000000
/* let's do a 1MB alloc and add realloc increment upto max flash size */
int myWrite(char *inBuf, int inBufLen)
{
   static SINT32 allocSize = 0;
   char *myglbCurPtr = NULL;

   if (glbImagePtr == NULL)
   {
      if (inBufLen < TFTP_BLOCKSIZE_DEFAULT)   // not enough data for a valid first packet and exit
         return -1;

      // Allocate maximum flash image size + possible broadcom header TAG.
      // (Don't bother getting the length from the broadcom TAG, we don't
      // get a TAG if it is a whole image anyways.)
      // The Linux kernel will not assign physical pages to the buffer
      // until we write to it, so it is OK if we allocate a little more
      // than we really need.
      system("killall -9 sntp >/dev/null");
      system("killall -9 tr64c >/dev/null");
      system("killall -9 dsldiagd >/dev/null");
      system("killall -9 swmdk >/dev/null");
      system("killall -9 mynetwork >/dev/null");
      system("killall -9 rtd >/dev/null");
      system("killall -9 data_center >/dev/null");
      system("killall -9 dbus-daemon >/dev/null");
      system("killall -9 wps_monitor >/dev/null");
      system("killall -9 httpd >/dev/null");
      system("killall -9 urlfilterd >/dev/null");
      system("killall -9 wlevt >/dev/null");
      system("killall -9 pppd >/dev/null");

      allocSize = BLOCK_ALLOC ; //cmsImg_getImageFlashSize() + cmsImg_getBroadcomImageTagSize();
      printf("Allocating %d bytes for flash image.\n", allocSize);
      if ((glbImagePtr = (char *) malloc(allocSize)) == NULL)
      {
         printf("Not enough memory error.  Could not allocate %u bytes.", allocSize);
         return -1;
      }
      else
      {
         printf("Memory allocated %u bytes\n",allocSize);
         glbCurPtr = glbImagePtr;
         if (glbUploadSize + inBufLen < allocSize)
         {
            memcpy(glbImagePtr, inBuf, inBufLen);
            glbCurPtr += inBufLen;
            glbUploadSize += inBufLen;
            return inBufLen;
         }
      }
   }
   else
   {
      if (glbUploadSize + inBufLen < allocSize)
      {
          memcpy(glbCurPtr, inBuf, inBufLen);
          glbCurPtr += inBufLen;
          glbUploadSize += inBufLen;
      }
      else
      {
          /* need to reset pointers */
          char *myglbCurPtr = NULL;
          SINT32 maxSize = cmsImg_getImageFlashSize() + cmsImg_getBroadcomImageTagSize();
          allocSize = glbUploadSize + BLOCK_ALLOC;
          if ( allocSize > maxSize )
          {
              allocSize = maxSize;
          }

          myglbCurPtr  = realloc(glbImagePtr, allocSize);

          if (myglbCurPtr)
          {
              printf("Memory reallocated %u bytes\n",allocSize);
              glbImagePtr = myglbCurPtr;
              glbCurPtr = myglbCurPtr +  glbUploadSize;
              if (glbUploadSize + inBufLen < allocSize)
              {
                  memcpy(glbCurPtr, inBuf, inBufLen);
                  glbCurPtr += inBufLen;
                  glbUploadSize += inBufLen;
              }
              else
              {
                  printf("Image could not fit into %u byte buffer.\n", allocSize);
                  return -1;
              }
          }
          else
          {
              printf("Image could not fit into %u byte buffer.\n", allocSize);
              return -1;

          }
      }
   }

   return inBufLen;
}
#else

int myWrite(char *inBuf, int inBufLen)
{
   static SINT32 allocSize = 0;
   
   if (glbCurPtr == NULL) 
   {
      if (inBufLen < TFTP_BLOCKSIZE_DEFAULT)   // not enough data for a valid first packet and exit
         return -1;   

      // Allocate maximum flash image size + possible broadcom header TAG.
      // (Don't bother getting the length from the broadcom TAG, we don't
      // get a TAG if it is a whole image anyways.)
      // The Linux kernel will not assign physical pages to the buffer
      // until we write to it, so it is OK if we allocate a little more
      // than we really need.

#if defined(AEI_VDSL_CUSTOMER_NCS)
        system("killall -9 sntp >/dev/null");
        system("killall -9 tr64c >/dev/null");
        system("killall -9 dsldiagd >/dev/null");
        system("killall -9 swmdk >/dev/null");
        system("killall -9 mynetwork >/dev/null");
        system("killall -9 rtd >/dev/null");
        system("killall -9 data_center >/dev/null");
        system("killall -9 dbus-daemon >/dev/null");
        system("killall -9 wps_monitor >/dev/null");
        system("killall -9 httpd >/dev/null");
        system("killall -9 urlfilterd >/dev/null");
        system("killall -9 wlevt >/dev/null");
        system("killall -9 pppd >/dev/null");
#endif

      allocSize = cmsImg_getImageFlashSize() + cmsImg_getBroadcomImageTagSize();

      printf("Allocating %d bytes for flash image.\n", allocSize);
      if ((glbCurPtr = (char *) malloc(allocSize)) == NULL)
      {
         printf("Not enough memory error.  Could not allocate %u bytes.", allocSize);   
         return -1;
      }
      printf("Memory allocated\n");
      glbImagePtr = glbCurPtr;
   }

   // copy the data from the current packet into our buffer
   if (glbUploadSize + inBufLen < allocSize)
   {
      memcpy(glbCurPtr, inBuf, inBufLen);
      glbCurPtr += inBufLen;
      glbUploadSize += inBufLen;
   }
   else
   {
       printf("Image could not fit into %u byte buffer.\n", allocSize);
       return -1;
   }

   return inBufLen;
}
#endif


//-- from igmp
#include <bits/socket.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#define MAXCTRLSIZE						\
	(sizeof(struct cmsghdr) + sizeof(struct sockaddr_in) +	\
	sizeof(struct cmsghdr) + sizeof(int) + 32)

/*
 * Interesting, tftp uses a different method for getting the 
 * linux interface name. See also cmsImg_saveIfNameFromSocket().
 * Sean says tftp is a client, so it needs to use this method.
 * In all other upload methods, the modem is the server, so the method
 * in cmsImg_saveIfNameFromSocket() is used.
 */
char *myif_indextoname(int sockfd,unsigned int ifindex,char *ifname)
{
	struct ifreq ifr;
	int status;

	memset(&ifr,0,sizeof(struct ifreq));
 	ifr.ifr_ifindex = ifindex;
	
	status = ioctl(sockfd,SIOCGIFNAME,&ifr);
	
	if (status < 0) {
		//printf("ifindex %d has no device \n",ifindex);
		return NULL;
	}
	else
		return strncpy(ifname,ifr.ifr_name,IFNAMSIZ);
}
// brmc end

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE

static int tftp_blocksize_check(int blocksize, int bufsize)
{
        /* Check if the blocksize is valid:
	 * RFC2348 says between 8 and 65464,
	 * but our implementation makes it impossible
	 * to use blocksizes smaller than 22 octets.
	 */

        if ((bufsize && (blocksize > bufsize)) ||
	    (blocksize < 8) || (blocksize > 65464)) {
	        bb_error_msg("bad blocksize");
	        return 0;
	}

	return blocksize;
}

static char *tftp_option_get(char *buf, int len, char *option)
{
        int opt_val = 0;
	int opt_found = 0;
	int k;

	while (len > 0) {

	        /* Make sure the options are terminated correctly */

	        for (k = 0; k < len; k++) {
		        if (buf[k] == '\0') {
			        break;
			}
		}

		if (k >= len) {
		        break;
		}

		if (opt_val == 0) {
			if (strcasecmp(buf, option) == 0) {
			        opt_found = 1;
			}
		}
		else {
		        if (opt_found) {
				return buf;
			}
		}

		k++;

		buf += k;
		len -= k;

		opt_val ^= 1;
	}

	return NULL;
}

#endif

static inline int tftp(const int cmd, const struct hostent *host,
	const char *remotefile, int localfd, const unsigned short port, int tftp_bufsize)
{
	const int cmd_get = cmd & tftp_cmd_get;
	const int cmd_put = cmd & tftp_cmd_put;
	const int bb_tftp_num_retries = 5;

	struct sockaddr_in sa;
	struct sockaddr_in from;
	struct timeval tv;
	socklen_t fromlen;
	fd_set rfds;
	char *cp;
	unsigned short tmp;
	int socketfd;
	int len;
	int opcode = 0;
	int finished = 0;
	int timeout = bb_tftp_num_retries;
	unsigned short block_nr = 1;
// brcm begine
	struct sockaddr_in saTmp;
	int i = 1;
   
   cmsLog_debug("main command loop: cmd_get=%d cmd_put=%d remotefile=%s", cmd_get, cmd_put, remotefile);
// brcm end

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
	int want_option_ack = 0;
#endif

	/* Can't use RESERVE_CONFIG_BUFFER here since the allocation
	 * size varies meaning BUFFERS_GO_ON_STACK would fail */
	char *buf=xmalloc(tftp_bufsize + 4);

	tftp_bufsize += 4;

	if ((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		bb_perror_msg("socket");
		return EXIT_FAILURE;
	}

// brcm begine
    len = sizeof(saTmp);
	memset(&saTmp, 0, len);
    saTmp.sin_family = AF_INET;
    saTmp.sin_addr.s_addr = INADDR_ANY;
   
    // set option for getting the to ip address.
    setsockopt(socketfd, IPPROTO_IP, IP_PKTINFO, &i, sizeof(i));

    if (bind(socketfd, (struct sockaddr *)&saTmp, len) == -1)
    {
        printf("Cannot bind socket.\n\n");
        return EXIT_FAILURE;
    }
    memset(&sa, 0, len);
    sa.sin_family = host->h_addrtype;
    sa.sin_port = htons(port);
    memcpy(&sa.sin_addr, (struct in_addr *) host->h_addr, host->h_length);
// brcm end

	/* build opcode */

	if (cmd_get) {
		opcode = TFTP_RRQ;
	}

	if (cmd_put) {
		opcode = TFTP_WRQ;
	}

	while (1) {

		cp = buf;

		/* first create the opcode part */

		*((unsigned short *) cp) = htons(opcode);

		cp += 2;

		/* add filename and mode */

		if ((cmd_get && (opcode == TFTP_RRQ)) ||
			(cmd_put && (opcode == TFTP_WRQ))) {
                        int too_long = 0;

			/* see if the filename fits into buf */
			/* and fill in packet                */

			len = strlen(remotefile) + 1;

			if ((cp + len) >= &buf[tftp_bufsize - 1]) {
			        too_long = 1;
			}
			else {
			        safe_strncpy(cp, remotefile, len);
				cp += len;
			}

			if (too_long || ((&buf[tftp_bufsize - 1] - cp) < 6)) {
				bb_error_msg("too long remote-filename");
				break;
			}

			/* add "mode" part of the package */

			memcpy(cp, "octet", 6);
			cp += 6;

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE

			len = tftp_bufsize - 4; /* data block size */

			if (len != TFTP_BLOCKSIZE_DEFAULT) {

			        if ((&buf[tftp_bufsize - 1] - cp) < 15) {
				        bb_error_msg("too long remote-filename");
					break;
				}

				/* add "blksize" + number of blocks  */

				memcpy(cp, "blksize", 8);
				cp += 8;

				cp += snprintf(cp, 6, "%d", len) + 1;

				want_option_ack = 1;
			}
#endif
		}

		/* add ack and data */

		if ((cmd_get && (opcode == TFTP_ACK)) ||
			(cmd_put && (opcode == TFTP_DATA))) {

			*((unsigned short *) cp) = htons(block_nr);

			cp += 2;

			block_nr++;

			if (cmd_put && (opcode == TFTP_DATA)) {
// brcm				len = bb_full_read(localfd, cp, tftp_bufsize - 4);
				len = myRead(cp, tftp_bufsize - 4);

				if (len < 0) {
					bb_perror_msg("read");
					break;
				}

				if (len != (tftp_bufsize - 4)) {
					finished++;
				}

				cp += len;
			} 
		}


		/* send packet */


		timeout = bb_tftp_num_retries;  /* re-initialize */
		do {

			len = cp - buf;

#ifdef CONFIG_FEATURE_TFTP_DEBUG
			fprintf(stderr, "sending %u bytes\n", len);
			for (cp = buf; cp < &buf[len]; cp++)
				fprintf(stderr, "%02x ", (unsigned char)*cp);
			fprintf(stderr, "\n");
#endif
			if (sendto(socketfd, buf, len, 0,
					(struct sockaddr *) &sa, sizeof(sa)) < 0) {
				bb_perror_msg("send");
				len = -1;
				break;
			}


			if (finished && (opcode == TFTP_ACK)) {
				break;
			}

			/* receive packet */

			memset(&from, 0, sizeof(from));
			fromlen = sizeof(from);

			tv.tv_sec = TFTP_TIMEOUT;
			tv.tv_usec = 0;

			FD_ZERO(&rfds);
			FD_SET(socketfd, &rfds);

			switch (select(FD_SETSIZE, &rfds, NULL, NULL, &tv)) {
			case 1: {
// brcm begin
				static int firstTime = 1;
				// first time, use recvmsg to get ifc name
				if (firstTime) {
					struct iovec iov = {buf, tftp_bufsize + 4};
					struct cmsghdr *cmsg;
					char *ctrl = (char *)xmalloc(MAXCTRLSIZE);
					struct msghdr msg = {(void*)&from, sizeof from, &iov, 1, (void*)ctrl, MAXCTRLSIZE, 0};
					struct in_pktinfo *info = NULL;
					char *p = connIfName;

					len = recvmsg (socketfd, &msg, MSG_WAITALL);
					if (len <= 0) {
						printf("*** error recvmsg len=%d\n", len);
						break;
					}
					for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL;cmsg =CMSG_NXTHDR(&msg,cmsg)) {
						if (cmsg->cmsg_type == IP_PKTINFO){
							info = (struct in_pktinfo *)CMSG_DATA(cmsg);
							i = info->ipi_ifindex;
							//printf("indx=%d, sepc_dst=%s, ipi_addr=%s\n", 
							//i, inet_ntoa(info->ipi_spec_dst),inet_ntoa(info->ipi_addr));
						}
						else {
							printf("BAD CMSG_HDR\n");
							break;
						}
					}
					free(ctrl);
 					p = myif_indextoname(socketfd, i, connIfName);
					firstTime = 0;

					/* need to send in the connection interface name to smd */
					if (glbUploadType == 'i')
					{
					   /*
					    * There is a big image coming.  tftp is about to malloc a big buffer
					    * and start filling it.  Notify smd so it can do killAllApps or
					    * something to make memory available on the modem.
					    */
					   cmsImg_sendLoadStartingMsg(msgHandle, connIfName);
					}
					
				}
				else
					len = recvfrom(socketfd, buf, tftp_bufsize, 0, (struct sockaddr *) &from, &fromlen);

				if (len < 0) {
					bb_error_msg("recvmsg");
					break;
				}

				timeout = 0;
				//printf("sa.sin_port =%d, from.sin_port = %d\n", sa.sin_port, from.sin_port);
// brcm end
				if (sa.sin_port ==  htons(port)) {      // brcm modified.
					sa.sin_port = from.sin_port;
				}
				if (sa.sin_port == from.sin_port) {
					break;
				}

				/* fall-through for bad packets! */
				/* discard the packet - treat as timeout */
				timeout = bb_tftp_num_retries;
            }
			case 0:
				bb_error_msg("timeout");

				timeout--;
				if (timeout == 0) {
					len = -1;
					bb_error_msg("last timeout");
				}
				break;

			default:
				bb_perror_msg("select");
				len = -1;
			}

		} while (timeout && (len >= 0));

		if ((finished) || (len < 0)) {
			break;
		}

		/* process received packet */


		opcode = ntohs(*((unsigned short *) buf));
		tmp = ntohs(*((unsigned short *) &buf[2]));

#ifdef CONFIG_FEATURE_TFTP_DEBUG
		fprintf(stderr, "received %d bytes: %04x %04x\n", len, opcode, tmp);
#endif

		if (opcode == TFTP_ERROR) {
			char *msg = NULL;

			if (buf[4] != '\0') {
				msg = &buf[4];
				buf[tftp_bufsize - 1] = '\0';
			} else if (tmp < (sizeof(tftp_bb_error_msg)
					  / sizeof(char *))) {

				msg = (char *) tftp_bb_error_msg[tmp];
			}

			if (msg) {
				bb_error_msg("server says: %s", msg);
			}

			break;
		}

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
		if (want_option_ack) {
          
			 want_option_ack = 0;

		    if (opcode == TFTP_OACK) {

			    /* server seems to support options */

			    char *res;

				 res = tftp_option_get(&buf[2], len-2,
						       "blksize");

				 if (res) {
				         int blksize = atoi(res);
			
					 if (tftp_blocksize_check(blksize,
							   tftp_bufsize - 4)) {

					    if (cmd_put) {
				           opcode = TFTP_DATA;
						 }
						 else {
				           opcode = TFTP_ACK;
						 }
#ifdef CONFIG_FEATURE_TFTP_DEBUG
						 fprintf(stderr, "using blksize %u\n", blksize);
#endif
					         tftp_bufsize = blksize + 4;
						 block_nr = 0;
						 continue;
					 }
				 }
				 /* FIXME:
				  * we should send ERROR 8 */
				 bb_error_msg("bad server option");
				 break;
			 }

			 bb_error_msg("warning: blksize not supported by server"
				   " - reverting to 512");

			 tftp_bufsize = TFTP_BLOCKSIZE_DEFAULT + 4;
		}
#endif  /* CONFIG_FEATURE_TFTP_BLOCKSIZE */


		if (cmd_get && (opcode == TFTP_DATA)) {

			if (tmp == block_nr) {
			
//	brcm			len = bb_full_ write(localfd, &buf[4], len - 4);
				if(localfd != -1)
				{
					len = write(localfd, &buf[4], len-4);
					glbUploadSize += len;
				}
				else
					len = myWrite(&buf[4], len-4);
				

				if (len < 0) {
					bb_perror_msg("write");
					break;
				}

				if (len != (tftp_bufsize - 4)) {
					finished++;
				}

				opcode = TFTP_ACK;
				continue;
			}
		}

		if (cmd_put && (opcode == TFTP_ACK)) {
         
			if (tmp == (unsigned short)(block_nr - 1)) {
				if (finished) {
					break;
				}
				opcode = TFTP_DATA;
				continue;
			}
		}
	} /* end of while(1) */

#ifdef CONFIG_FEATURE_CLEAN_UP
	close(socketfd);

        free(buf);
#endif

	return finished ? EXIT_SUCCESS : EXIT_FAILURE;
}

int tftp_main(int argc, char **argv)
{
	struct hostent *host = NULL;

	// brcm begin
#ifdef CONFIG_FEATURE_TFTP_DEBUG
	const char *localfile = NULL;
#endif
	const char *remotefile = "mdm.config";
   CmsLogLevel logLevel=DEFAULT_LOG_LEVEL;
   SINT32 logLevelNum;
   CmsRet ret;
	// brcm end
   
	int port;
	int cmd = 0;
	int fd = -1;
	int flags = 0;
	int opt;
	int result;
	int blocksize = TFTP_BLOCKSIZE_DEFAULT;
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
    int dualPartition = 0;
#endif

	/* figure out what to pass to getopt */

#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
#define BS "b:"
#else
#define BS
#endif

#ifdef CONFIG_FEATURE_TFTP_GET
#define GET "g"
#else
#define GET
#endif

#ifdef CONFIG_FEATURE_TFTP_PUT
#define PUT "p"
#else
#define PUT
#endif

// brcm begin
    cmsLog_init(EID_TFTP);
    cmsLog_setLevel(logLevel);
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
    while ((opt = getopt(argc, argv, BS GET PUT "f:t:v:d:")) != -1) {
#else
    while ((opt = getopt(argc, argv, BS GET PUT "f:t:v:")) != -1) {
#endif
		switch (opt) {
#ifdef CONFIG_FEATURE_TFTP_BLOCKSIZE
		case 'b':
			blocksize = atoi(optarg);
			if (!tftp_blocksize_check(blocksize, 0)) {
                                return EXIT_FAILURE;
			}
			break;
#endif
#ifdef CONFIG_FEATURE_TFTP_GET
		case 'g':
			cmd = tftp_cmd_get;
			flags = O_WRONLY | O_CREAT | O_TRUNC;
			break;
#endif
#ifdef CONFIG_FEATURE_TFTP_PUT
		case 'p':
			cmd = tftp_cmd_put;
			flags = O_RDONLY;
         glbUploadType = 'f';
			break;
#endif

// brcm begin
#if 0  
		case 'l':
			localfile = optarg;
			break;
		case 'r':
			remotefile = optarg;
			break;
#endif
		case 'f':
			remotefile = optarg;
			break;
		case 't':
			glbUploadType = optarg[0];
			break;
         
      case 'v':
         logLevelNum = atoi(optarg);
         if (logLevelNum == 0)
         {
            logLevel = LOG_LEVEL_ERR;
         }
         else if (logLevelNum == 1)
         {
            logLevel = LOG_LEVEL_NOTICE;
         }
         else
         {
            logLevel = LOG_LEVEL_DEBUG;
         }
         cmsLog_setLevel(logLevel);
         break;
		 
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
		case 'd':
			dualPartition = atoi(optarg);
			break;
#endif

		}
	}
	if ((cmd == 0) || (optind == argc) || !(glbUploadType == 'i' || glbUploadType == 'c' || glbUploadType == 'f')) {
      cmsLog_debug("cmd=%d optind=%d argc=%d", cmd, optind, argc);
		bb_show_usage();
	}

   cmsLog_debug("mypid=%d glbUploadType=%c", getpid(), glbUploadType);
	
#if 0 // brcm
	if(localfile && strcmp(localfile, "-") == 0) {
	    fd = fileno((cmd==tftp_cmd_get)? stdout : stdin);
	}
	if(localfile == NULL)
	    localfile = remotefile;
	if(remotefile == NULL)
	    remotefile = localfile;
	if (fd==-1) {
	    fd = open(localfile, flags, 0644);
	}
	if (fd < 0) {
		bb_perror_msg_and_die("local file");
	}
#endif //brcm

	host = xgethostbyname(argv[optind]);
	port = 69;      // brcm port = bb_lookup_port(argv[optind + 1], "udp", 69);

#ifdef CONFIG_FEATURE_TFTP_DEBUG
	fprintf(stderr, "using server \"%s\", remotefile \"%s\", "
		"localfile \"%s\".\n",
		inet_ntoa(*((struct in_addr *) host->h_addr)),
		remotefile, localfile);
#endif


// brcm: We need to establish a comm link with smd.
   if ((ret = cmsMsg_init(EID_TFTP, &msgHandle)) != CMSRET_SUCCESS)
   {
      printf("failed to open comm link with smd, tftp failed.");
      return 0;
   }
   

	if (glbUploadType == 'f') {	
// brcm: mwang: is this for sending some arbitrary file to the other side?
// but the usage says this is for backing up the config file.
// The existing code tries to open a local file, but other code assumes
// the config file is in a memory buffer.  Oh, well, comment out the 
// code which tries to open a local file.
      cmsLog_debug("sending config to remote file %s", remotefile);
#if 0
	 	fd = open(remotefile, flags, 0644);
		if (fd < 0) {
			printf("Failed opening local file: %s\n", remotefile);
	      return EXIT_FAILURE;
		}	
#endif	
		result = tftp(cmd, host, remotefile, fd, port, blocksize);
      if (result == 0)
      {		
	      printf("backed up config file to %s (%d bytes)\n", remotefile, glbUploadSize);
      }
      else
      {
         printf("Could not back up config file.\n");
      }
          
		// close(fd);		
	}
   else
   {
	   if ((result = tftp(cmd, host, remotefile, fd, port, blocksize)) == EXIT_SUCCESS)
      {
         printf("Got image via tftp, total image size: %d\n", glbUploadSize);
         
         /*
          * cmsImsg_writeImage will determine the image format and write
          * to flash.  If successful, the system will do a sysMipsSoftReset
          * immediately.  So we will not return from this function call.
          * (But on the desktop, this call does return, so we still have to check the
          * return value.)
          */
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
        if(dualPartition)
        {
            if ((ret = AEI_writeDualPartition(glbImagePtr, glbUploadSize, msgHandle, dualPartition)) != CMSRET_SUCCESS)
            {
                printf("Tftp Image failed: Illegal image.\n");
            }
        }
		else
#endif
         if ((ret = cmsImg_writeImage(glbImagePtr, glbUploadSize, msgHandle)) != CMSRET_SUCCESS)
         {
            printf("Tftp Image failed: Illegal image.\n");
         }
      }
      else
      {
         printf("Tftp Image failed: tftp server OR file name not found.\n");
      }
   
      /*
       * If we get here, the image download has failed.  Tell smd to go back
       * to normal operation.
       */
      if (glbImagePtr)
      {
         free(glbImagePtr);
      }
   
      if (glbUploadType == 'i')
      {
         cmsImg_sendLoadDoneMsg(msgHandle);
      }
   }
   
   cmsMsg_cleanup(&msgHandle);
      
// brcm end

#ifdef CONFIG_FEATURE_CLEAN_UP
	if (!(fd == STDOUT_FILENO || fd == STDIN_FILENO)) {
	    close(fd);
	}
#endif
	return(result);
}
