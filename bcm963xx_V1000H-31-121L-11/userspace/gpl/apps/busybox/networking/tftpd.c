/* 
 * $Id$
 * A simple tftpd server for busybox
 *
 * Copyright (C) 2001 Steven Carr <Steven_Carr@yahoo.com>
 *
 * Tries to follow RFC1350 and RFC2347.
 * Only "octet" mode supported.
 * tsize option is supported on sending files only (pxelinux support).
 * chroot jail for security.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-137 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/tftp.h>
#include <linux/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include "busybox.h"

#include "cms.h"
#include "cms_util.h"
#include "cms_msg.h"


#ifndef OACK
#define OACK 6
#endif
#ifndef EOPTNEG
#define EOPTNEG 8
#endif

#define ENOPUT -2
#define ENOGET -3

#if !defined(__UCLIBC__) || defined(__UCLIBC_HAS_MMU__)
#define FORK() fork()
#else
#define FORK() vfork()
#endif

#define TFTP_BLOCKSIZE_DEFAULT 512   /* according to RFC 1350, don't change */
#define TFTP_TIMEOUT           5     /* seconds */
#define TFTPD_TIMEOUT          300   /* seconds */

// brcm begine.  Enable put only

#define DUP_PKT_CHECK_COUNT      3
//Enable put only
//#define CONFIG_FEATURE_TFTPD_GET
#define CONFIG_FEATURE_TFTPD_PUT
static char connIfName[CMS_IFNAME_LENGTH]={0};
static void *msgHandle=NULL;
static int glbPeerFd=-1;


void common_exit(int code)
{
   if (glbPeerFd != -1)
   {
      close(glbPeerFd);
   }
   
   exit(code);  
}

void handler_sigterm(int signum)
{
   cmsLog_notice("received signal %d", signum);
   common_exit(0);
}


void perror_msg_and_die(char * msg)
{
    printf("fatal: %s\n", msg);
    common_exit(0);
}



/*
 * Handle initial connection protocol.
 *     +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+-->  >-------+---+---~~---+---+
 *     |  opc  |filename| 0 |  mode  | 0 |  opt1  | 0 | value1 | 0 | <  <  optN  | 0 | valueN | 0 |
 *     +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+-->  >-------+---+---~~---+---+
 *             ^--->
 */

/* The options are zero terminated, retrieve a list of pointers to the first character of each option */
int tftpd_options (char *options, int opt_len, char **argv, int max_arg)
{
  int x;
  int y;
  argv[0] = options;
  for (y = 1, x = 0; (y < max_arg) && (x < (opt_len - 1)); x++)
    {
      if (options[x] == 0)
   {
     if (options[x + 1] == 0)
       return y;
     argv[y] = &options[x + 1];
     y++;
   }
    }
  return y;
}


/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
void tftpd_nak (int peer, int error)
{
  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct tftphdr *pkt;

  pkt = (struct tftphdr *) buf;
  pkt->th_opcode = htons ((u_short) ERROR);
  pkt->th_code = htons ((u_short) error);

  switch (error)
    {
    case ENOPUT:
      strcpy (pkt->th_msg, "Put not supported");
      pkt->th_code = htons (EUNDEF);
      break;
    case ENOGET:
      strcpy (pkt->th_msg, "Get not supported");
      pkt->th_code = htons (EUNDEF);
      break;
    case EUNDEF:
      strcpy (pkt->th_msg, "Undefined error code");
      break;
    case ENOTFOUND:
      strcpy (pkt->th_msg, "File not found");
      break;
    case EACCESS:
      strcpy (pkt->th_msg, "Access violation");
      break;
    case ENOSPACE:
      strcpy (pkt->th_msg, "Disk full or allocation exceeded");
      break;
    case EBADOP:
      strcpy (pkt->th_msg, "Illegal TFTP operation");
      break;
    case EBADID:
      strcpy (pkt->th_msg, "Unknown transfer ID");
      break;
    case EEXISTS:
      strcpy (pkt->th_msg, "File already exists");
      break;
    case ENOUSER:
      strcpy (pkt->th_msg, "No such user");
      break;
    case EOPTNEG:
      strcpy (pkt->th_msg, "Failure to negotiate RFC2347 options");
      break;
    default:
      strcpy (pkt->th_msg, strerror (error - 100));
      pkt->th_code = htons (EUNDEF);
      break;
    }

  send (peer, buf, strlen (pkt->th_msg) + 5, 0);
}

/*
 * Send a ack packet 
 */
void tftpd_ack (int peer, int block)
{
  struct tftphdr pkt;

  pkt.th_opcode = htons (ACK);
  pkt.th_block = htons (block);

  if (send (peer, &pkt, sizeof(pkt), 0)!=sizeof(pkt))
     perror_msg_and_die("tftpd_ack send");
}


/*
 * send an oack
 */
void tftpd_oack (int peer, int count, char **list)
{
  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct tftphdr *pkt;
  int x;
  char *ptr;

  pkt=(struct tftphdr *)buf;
  pkt->th_opcode = htons (OACK);
  ptr=pkt->th_stuff;
  
  for (x=0;x<count;x++)
    ptr=strrchr (strcpy (ptr, list[x]), '\0') + 1;

 if ( send (peer, buf, (ptr-buf), 0)!=(ptr-buf))
     perror_msg_and_die("tftpd_oack send");
}


/*
 * send data
 */
void tftpd_data (int peer, int block, char *data, int size)
{
  struct tftphdr *pkt;
  char buf[TFTP_BLOCKSIZE_DEFAULT + 4];

  pkt=(struct tftphdr *)buf;
  pkt->th_opcode = htons (DATA);
  pkt->th_block  = htons(block);
  
  memcpy(pkt->th_data,data,size);

  if (send (peer, &buf, size+4, 0)!=(size+4))
     perror_msg_and_die("tftpd_data send");
}

int tftpd_getdata(int peer, int block, char *data, int size)
{
  struct tftphdr *pkt;
  struct timeval tv;
  fd_set rfds;
  int len=-1;
  int timeout_counter = 4;
  
  pkt=(struct tftphdr *)data;
  
  do {   
      tv.tv_sec = TFTP_TIMEOUT;
      tv.tv_usec = 0;
      FD_ZERO (&rfds);
      FD_SET (peer, &rfds);
      switch (select (FD_SETSIZE, &rfds, NULL, NULL, &tv))
   {
   case 1:      /* data ready */
     len = recv (peer, data, size, 0);
     if (len < 0)
       perror_msg_and_die ("failed to read (data)");

     pkt->th_opcode = ntohs (pkt->th_opcode);
     pkt->th_block = ntohs (pkt->th_block);
     if (pkt->th_opcode == ERROR)
       {
         bb_error_msg (pkt->th_data);
         common_exit(0);
       }
     if ((pkt->th_opcode == DATA) && (pkt->th_block != block))
       {
         //synchronize (peer);
       }
     break;
   case 0:      /* timeout */
     timeout_counter--;
     if (timeout_counter == 0)
       {
         bb_error_msg ("last timeout");
         common_exit(0);
       }
     break;
   default:      /* error */
     perror_msg_and_die ("select failed");
     break;
   }
   }while (!(pkt->th_opcode == DATA) && (pkt->th_block == block));
   return len;
}



int tftpd_getack(int peer, int block)
{
  char data[TFTP_BLOCKSIZE_DEFAULT + 4];
  struct tftphdr *pkt;
  struct timeval tv;
  fd_set rfds;
  int timeout_counter = 4;
  int len;
  
  pkt=(struct tftphdr *)data;

  do {   
      tv.tv_sec = TFTP_TIMEOUT;
      tv.tv_usec = 0;
      FD_ZERO (&rfds);
      FD_SET (peer, &rfds);
      switch (select (FD_SETSIZE, &rfds, NULL, NULL, &tv))
   {
   case 1:      /* data ready */

     len = recv (peer, data, TFTP_BLOCKSIZE_DEFAULT + 4, 0);
     if (len < 0)
       perror_msg_and_die ("failed to read (data)");

     pkt->th_opcode = ntohs (pkt->th_opcode);
     pkt->th_block = ntohs (pkt->th_block);

     if (pkt->th_opcode == ERROR)
       {
         bb_error_msg (pkt->th_data);
         common_exit(0);
       }

     if ((pkt->th_opcode == ACK) && (pkt->th_block != block))
       {
         //synchronize (peer);
       }
     break;
   case 0:      /* timeout */
     timeout_counter--;
     if (timeout_counter == 0)
       {
         bb_error_msg ("last timeout");
         common_exit(0);
       }
     break;
   default:      /* error */
     perror_msg_and_die ("select failed");
     break;
   }
   }while (! ((pkt->th_opcode == ACK) && (pkt->th_block == block)) );

  return (1==1);
}





#ifndef CONFIG_FEATURE_TFTPD_GET
void
tftpd_send (int peer,
            struct tftphdr *tp __attribute((unused)),
            int n __attribute__((unused)),
            int buffersize __attribute__((unused)))
{
   /* we aren't configured for sending files */
   tftpd_nak (peer, ENOGET);
   common_exit(0);
}

#else
void
tftpd_send (int peer, struct tftphdr *first_pkt, int pkt_len, int buffersize)
{
   FILE *file=NULL;
   char buffer[TFTP_BLOCKSIZE_DEFAULT+4];
   char *list[64]; /* list of pointers to options and values */
   int listcount;
   char *reply_list[64];
   int reply_listcount=0;
   char tsize_ret[32];
   int block, inbytes, x;
   
   listcount = tftpd_options (first_pkt->th_stuff, pkt_len, list ,64);

   /* get the size of the file (remember, chroot() supposed to point us in the right directory) */

   if (strcasecmp(list[1],"octet")!=0)
   {
      tftpd_nak(peer,EBADOP);
      common_exit(0);
   }


   file = fopen (list[0], "r");
   if (file == NULL)
   {
     tftpd_nak (peer, ENOTFOUND);
     common_exit(0);
   }
   fseek (file, 0, SEEK_END);
   sprintf(tsize_ret,"%lu", ftell (file));
   fseek (file, 0, SEEK_SET);


   /* 0=filename, 1=mode, 2=option, 3=option_value ... */
   block = 1;
   reply_listcount=0;

   /* look through the options for the ones we support */
   for (x=2;x<listcount;x++)
   {
      if (strcasecmp(list[x],"tsize")==0) /* only one option supported so far */
      {
         reply_list[reply_listcount]=list[x];      
         reply_listcount++;
         reply_list[reply_listcount]=tsize_ret; /* point to the real value */
         reply_listcount++;
      }
   }

   /* if there are any options, send an OACK instead of an ACK */
   if (reply_listcount>0)
   {
      do
      {
         tftpd_oack(peer,reply_listcount,reply_list);
      }
      while (!tftpd_getack(peer,0));
   }   


   /* Send the file! */
   while ((inbytes = fread(buffer,1,TFTP_BLOCKSIZE_DEFAULT,file))>0)
   {
      do
      {
         tftpd_data(peer,block,buffer,inbytes);
      }
      while (!tftpd_getack(peer,block));
      block++;
   }
   fclose(file);
   common_exit(0);
}

#endif


#ifndef CONFIG_FEATURE_TFTPD_PUT
void
tftpd_receive (int peer, struct tftphdr *tp, int n, int buffersize)
{
   /* we aren't configured for receiving files */
   tftpd_nak (peer, ENOPUT);
   common_exit(0);
}

#else
void
//brcm begin
tftpd_receive (int peer, struct tftphdr *first_pkt, int pkt_len, int buffersize __attribute__((unused)))
{
// brcm   FILE *file=NULL;
   char buffer[TFTP_BLOCKSIZE_DEFAULT+4];
   struct tftphdr *pkt;
   int block, inbytes;
   char *list[64];
   int listcount;

    //brcm begin
    int byteRd = 0;
    int i = 0;   
    CmsRet ret;
    static UBOOL8 isConfigFile=FALSE;
    int totalAllocatedSize = 0;
    int uploadSize = 0;
    char *imagePtr = NULL;
    char *curPtr = NULL;
    


//printf("tftpd_receive, peer = %d, pkt_len = %d, buffersize=%d\n", peer, pkt_len, buffersize);

   pkt=(struct tftphdr *)buffer;
   listcount = tftpd_options (first_pkt->th_stuff, pkt_len, list ,64);

   /* get the size of the file (remember, chroot() supposed to point us in the right directory) */

//printf ("mode= %s, file= %s\n", list[1], list[0]);   
    if (strcasecmp(list[1],"octet")!=0)
   {
        printf("Only support 'bin' mode. Type 'bin' at tftp client\n");
      tftpd_nak(peer,EBADOP);
      common_exit(0);
   }

#if 0//brcm
    file = fopen (list[0], "w");
   if (file == NULL)
   {
     tftpd_nak (peer, EACCESS);
     common_exit(0);
   }
#endif //brcm



   block=0;

   do
   {
      tftpd_ack(peer,block);
      block++;

      // if duplicate pkt, (for slow ack on 38R board) discard it.
      for (i = 0; i < DUP_PKT_CHECK_COUNT; i++)
      {
         inbytes=tftpd_getdata(peer,block,buffer,TFTP_BLOCKSIZE_DEFAULT+4);
         if (block == (int) (*(short*)(buffer+2)))
            break;
      }
      
      
      // brcm fwrite(pkt->th_msg,1,inbytes-4,file);
      byteRd=inbytes-4;
      
      // brcm begin
      if (curPtr == NULL) 
      {
         // First time through, need to allocate buffer for image
         
         if (byteRd < TFTP_BLOCKSIZE_DEFAULT)   // not enough data for a valid first packet and exit
         {
            cmsLog_error("first packet too short, byteRd=%d default block size=%d", byteRd, TFTP_BLOCKSIZE_DEFAULT);
            uploadSize = byteRd;
            break;
         }
         
         // The first TFTP_BLOCKSIZE_DEFAULT (512 bytes) of the image
         // is in pkt->th_msg.  First determine if it is a config file.
         isConfigFile = cmsImg_isConfigFileLikely(pkt->th_msg);
         cmsLog_debug("isConfigFile = %d", isConfigFile);

         if (isConfigFile)
         {
            totalAllocatedSize = cmsImg_getConfigFlashSize();
         }
         else
         {
            totalAllocatedSize = cmsImg_getImageFlashSize() + cmsImg_getBroadcomImageTagSize();
            // let smd know that we are about to start a big download
            cmsImg_sendLoadStartingMsg(msgHandle, connIfName);
         }       

         if ((curPtr = (char *) malloc(totalAllocatedSize)) == NULL)
         {
            printf("Not enough memory error (%d bytes needed).\n", totalAllocatedSize);       
            common_exit(0);
         }

         printf("%d bytes allocated for image\n", totalAllocatedSize);
         imagePtr = curPtr;
      } // if curPtr == NULL

      if (uploadSize + byteRd < totalAllocatedSize)
      {
         memcpy(curPtr, pkt->th_msg, byteRd);
         curPtr += byteRd;
         uploadSize += byteRd;
      }
      else
      {
         printf("Image could not fit into %d byte buffer.\n", totalAllocatedSize);
         common_exit(0);
      }
   }
   while (inbytes==(TFTP_BLOCKSIZE_DEFAULT+4));

    tftpd_ack(peer,block); /* final acknowledge */

// brcm   fclose(file);

   printf("Total upload size: %d bytes\n", uploadSize);
   
    /*
     * cmsImsg_writeImage will determine the image format and write
     * to flash.  If successful, the system will do a sysMipsSoftReset
     * immediately.  So we will not return from this function call.
     * But on desktop linux, we still return from this call, so still
     * check for return value.
     */
    if ((ret = cmsImg_writeImage(imagePtr, uploadSize, msgHandle)) != CMSRET_SUCCESS)
    {
       printf("Tftp Image failed: Illegal image.\n");
    }  
    
    if (!isConfigFile)
    {
       cmsImg_sendLoadDoneMsg(msgHandle);
    } 
    
    if (imagePtr)
    {
       free(imagePtr);
    }
    
    common_exit(0);
}
// brcm end
#endif

#if 0 /* not used */
static struct in_addr getLanIp(void) //struct in_addr *lan_ip)
{
#ifdef DESKTOP_LINUX
   struct in_addr inaddr;
   
   inet_aton("127.0.0.1", &inaddr);
   return inaddr;
#else

   int socketfd;
   struct ifreq lan;

   cmsLog_debug("getting LAN ip (what for?)");
   
   memset(&lan, 0, sizeof(lan));
   if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Error openning socket when getting LAN info\n");
   }
   else  {
        strcpy(lan.ifr_name, "br0");
        if (ioctl(socketfd,SIOCGIFADDR,&lan) < 0) {
            printf("Error getting LAN IP address\n");
        }
   }
   close(socketfd);
   return ((struct sockaddr_in *)&(lan.ifr_addr))->sin_addr;
#endif
}
#endif

// brcm -- from igmp
#include <bits/socket.h>
#include <sys/uio.h>
#define MAXCTRLSIZE                  \
   (sizeof(struct cmsghdr) + sizeof(struct sockaddr_in) +   \
   sizeof(struct cmsghdr) + sizeof(int) + 32)
// brmc end

static int
tftpd_daemon (char *directory __attribute__((unused)),
              char *address __attribute__((unused)),
              int port __attribute__((unused)))
{
   struct tftphdr *tp;
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
   struct sockaddr_in6 from;
   struct sockaddr_in6 myaddr;
#else
   struct sockaddr_in from;
   struct sockaddr_in myaddr;
#endif
   int fd = -1;
   int rv;
   int n;
#if 0
   struct sockaddr_in bindaddr;
   pid_t pid;
   int i = 1;
#endif
   int peer;

   char buf[TFTP_BLOCKSIZE_DEFAULT + 4];
   struct iovec iov = { buf, sizeof buf };
   struct cmsghdr *cmsg;
   char *ctrl = (char *)xmalloc(MAXCTRLSIZE);
   struct msghdr msg = { (void*)&from, sizeof from, &iov, 1, (void*)ctrl, MAXCTRLSIZE, 0};
   struct in_pktinfo *info = NULL;

#if 1

   //
   // We should have defined a CONFIG_FEATURE_TFTPD_INETD
   // to distinguish between inetd based tftpd or consoled based tftpd spawning
   // In the latter case, we do need to daemonize using daemon(0,1) so that
   // the tftpd could detach from the console/cli.
   //
   // CRDDB00016882
   // daemon(0,1);	inetd already daemonizes tftpd.
   //


   signal(SIGTERM, handler_sigterm);
   
   /* mwang: for smd dynamic launch, the server fd is at a fixed number */
   fd = CMS_DYNAMIC_LAUNCH_SERVER_FD;
   
#endif

#if 0
   /* mwang: do not daemonize in CMS */
   daemon(0,1);

   if ((fd = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
      perror_msg_and_die ("socket");
   memset (&bindaddr, 0, sizeof (bindaddr));
   bindaddr.sin_family = AF_INET;
   bindaddr.sin_addr.s_addr = INADDR_ANY;
   bindaddr.sin_port = htons (port);
   if (address != NULL)
   {
      struct hostent *hostent;
      hostent = xgethostbyname (address);
      if (!hostent || hostent->h_addrtype != AF_INET)
         perror_msg_and_die ("cannot resolve local bind address");
      memcpy (&bindaddr.sin_addr, hostent->h_addr, hostent->h_length);
   }
   // set option for getting the to ip address.
   setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &i, sizeof(i));

   if (bind (fd, (struct sockaddr *) &bindaddr, sizeof (bindaddr)) < 0)
      perror_msg_and_die ("daemon bind failed");
   /* This means we don't want to wait() for children */
   signal (SIGCHLD, SIG_IGN);

  // get pid for web access function to start/stop tftpd.
  {
    FILE *pid_fp;
    if (!(pid_fp = fopen("/var/run/tftpd_pid", "w"))) 
    {
        printf("Error open /var/run/tftpd_pid");
        common_exit(0);
    }
    fprintf(pid_fp, "%d\n", getpid());
    fclose(pid_fp);
  }
#endif

   while (1)
   {
      struct timeval timer;
      fd_set         readset;
      
      memset(buf,0,TFTP_BLOCKSIZE_DEFAULT + 4);
      memset (&myaddr, 0, sizeof (myaddr));

      /* You have to do this in Linux, reinitialize timer each iteration */
      timer.tv_sec  = TFTPD_TIMEOUT;
      timer.tv_usec = 0;

      FD_ZERO (&readset);
      FD_SET (fd, &readset);

      /* Never time out, we're in standalone mode */
      rv = select (fd + 1, &readset, NULL, NULL, &timer);
      if (rv <= 0)
      {
         perror_msg_and_die("error during select, exit");
      }

#if 0 //inetd
      if (rv == -1 && errno == EINTR)
         continue;      /* Signal caught, reloop */
      if (rv == -1)
         perror_msg_and_die ("select loop");
      if (rv == 0)
      {
	  bb_error_msg ("We shouldn't be timeing out!");
	  exit (0);		/* Timeout, return to inetd */
      }
#endif
      n = recvmsg (fd, &msg, MSG_WAITALL);
      if (n <= 0)
      {
         if (n == 0)  /* Keven -- Received message with zero length, reloop */
            continue;
         else
         {         
            perror_msg_and_die("*** error recvmsg < 0\n");
         }
      }
//printf("incoming_ip=%s, n=%d\n", inet_ntoa(from.sin_addr), n);
      for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL;cmsg =CMSG_NXTHDR(&msg,cmsg))
      {
         if (cmsg->cmsg_type == IP_PKTINFO)
         {
            info = (struct in_pktinfo *)CMSG_DATA(cmsg);
//            printf("sepc_dst=%s, ipi_addr=%s\n", inet_ntoa(info->ipi_spec_dst),inet_ntoa(info->ipi_addr));
            break;
         }
      }

      /* Process the request */
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
      bzero((char *)&myaddr, sizeof(struct sockaddr_in6));
      myaddr.sin6_family = AF_INET6;
      myaddr.sin6_port = htons (0);   /* we want a new local port */
      myaddr.sin6_addr = in6addr_any;
#else
      bzero((char *)&myaddr, sizeof(struct sockaddr_in));
      myaddr.sin_family = AF_INET;
      myaddr.sin_port = htons (0);   /* we want a new local port */
      myaddr.sin_addr.s_addr = INADDR_ANY;   //getLanIp();
#endif

#if 0
      // mwang: not sure exactly what this is trying to do, but it doesn't
      // make any sense and does no good.  Just skip it.
      if (myaddr.sin_addr.s_addr != info->ipi_spec_dst.s_addr)
         memcpy (&myaddr.sin_addr, &bindaddr.sin_addr,sizeof bindaddr.sin_addr);
#endif

      /* we got the request, break out of loop */
      break;

#if 0 //inetd
      /* Now that we have read the request packet from the UDP
         socket, we fork and go back to listening to the socket. */
      pid = FORK ();

      if (pid < 0)
         perror_msg_and_die ("cannot fork");
      if (pid == 0)
         break;         /* Child exits the while(1), parent continues to loop */
#endif
   }  //while (1)

   /* Close file descriptors we don't need */
   // brcm close (fd);

   /* Get a socket.  This has to be done before the chroot() (/dev goes away) */
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
   peer = socket (AF_INET6, SOCK_DGRAM, 0);
#else
   peer = socket (AF_INET, SOCK_DGRAM, 0);
#endif
   glbPeerFd = peer;
   if (peer < 0)
      perror_msg_and_die ("socket");
#ifndef DESKTOP_LINUX
   if (chroot ("."))
      perror_msg_and_die ("chroot");
#endif
//   from.sin_family = AF_INET;

   /* Process the request */
//   cmsLog_debug("binding to %s", inet_ntoa(myaddr.sin_addr));
   if (bind (peer, (struct sockaddr *) &myaddr, sizeof myaddr) < 0)
      perror_msg_and_die ("daemon-child bind");

//printf("after bind. my_ip=%s*****\n", inet_ntoa(myaddr.sin_addr));

   if (connect (peer, (struct sockaddr *) &from, sizeof from) < 0)
   {
      perror_msg_and_die ("daemon-child connect");
   }
   
   /* save the connection interface name for later deciding if
   * it is a WAN or LAN interface in the uploading process
   */
   if (cmsImg_saveIfNameFromSocket(peer, connIfName) != CMSRET_SUCCESS)
   {
      printf("Failed to get remote ifc name!\n");
   }         


   tp = (struct tftphdr *) buf;

//printf("after connect \n");

   /*
    * iptables will block any traffic that is not allowed.
    * Unlike before, userspace code do not need to check with access
    * control lists.  If tftpd gets a packet, it should service it.
    */

   tp->th_opcode = ntohs (tp->th_opcode);

   switch(tp->th_opcode)
   {
      case RRQ:
         tftpd_send (peer, tp, n, TFTP_BLOCKSIZE_DEFAULT);
         break;
      case WRQ:
         tftpd_receive (peer, tp, n, TFTP_BLOCKSIZE_DEFAULT);
         break;
   }

   common_exit(0);
   return 0; // not reached, but suppresses a compiler warning
}



int
tftpd_main (int argc, char **argv)
{
  int result;
  char *address = NULL;      /* address to listen to */
  
  // brcm in CMS, smd opens the tftpd server port and launches tftpd when
  // activity is detected on that port.  So port number is not used here.
  int port = -1;  /* actually 69 */
  
  CmsLogLevel logLevel=DEFAULT_LOG_LEVEL;
  SINT32 logLevelNum;
  int opt;
  
  cmsLog_init(EID_TFTPD);
  while ((opt = getopt(argc, argv, "v:")) != -1)
  {
     switch(opt)
     {
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
     }
  }

#if 0 //brcm
  int daemonize = 0;  
  int on = 1;
  int fd = 0;
  int opt;  
  char directory[256];      /* default directory "/tftpboot/" */
  memset (directory, 0, sizeof (directory));
  strcpy (directory, "/tftpboot/");

  while ((opt = getopt (argc, argv, "sp:a:d:h")) != -1)
    {
      switch (opt)
   {
   case 'p':
     port = atoi (optarg);
     break;
   case 'a':
     address = optarg;
     break;
   case 's':
     daemonize = (1 == 1);
     break;
   case 'd':
     safe_strncpy (directory, optarg, sizeof (directory));
     break;
   case 'h':
     show_usage ();
     break;
   }
    }
  if (chdir (directory))
    perror_msg_and_die ("Invalid Directory");

  if (ioctl (fd, FIONBIO, &on) < 0)
    perror_msg_and_die ("ioctl(FIONBIO)");

  /* daemonize this process */
  if (daemonize)
    {
      pid_t f = FORK ();
      if (f > 0) {
        FILE *pid_fp;
        if (!(pid_fp = fopen("/var/run/tftpd_pid", "w"))) 
        {
            printf("Error open /var/run/tftpd_pid");
            exit(0);
        }
        fprintf(pid_fp, "%d\n", f);
        fclose(pid_fp);
       exit (0);
      }
      if (f < 0)
       perror_msg_and_die ("cannot fork");
      close (0);
      close (1);
      close (2);
    }
#endif /* 0 */ /* brcm */

  cmsMsg_init(EID_TFTPD, &msgHandle);

  result = tftpd_daemon ("", address, port);
  
  cmsMsg_cleanup(&msgHandle);
  
  return (result);
}

