#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <syslog.h>
#include <linux/if.h>
#include <sys/stat.h>
#include "dproxy.h"
#include "dns_decode.h"
#include "cache.h"
#include "conf.h"
#include "dns_list.h"
#include "dns_construct.h"
#include "dns_io.h"
#include "dns_dyn_cache.h"
#include "dns_probe.h"
/*****************************************************************************/
/*****************************************************************************/
int dns_main_quit;
int dns_sock;
fd_set rfds;

//BRCM
int dns_querysock;
int dns_querysock_ipv6;
int dns_queryport_ipv6 = -1;
#ifdef DMP_X_ITU_ORG_GPON_1
static unsigned int dns_query_error = 0;
#endif

int dns_wanup = 0;
/* CMS message handle */
void *msgHandle=NULL;
int msg_fd;

#ifndef DNS_PROBE
extern time_t dns_recover_time;
#endif
extern void dns_probe_print(void);


// from dns_subnet.c
extern void dns_sunbet_init(void);
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
extern UBOOL8 dns_subnet_map(struct in6_addr *lanIp, char *dns1);
#else
extern UBOOL8 dns_subnet_map(struct in_addr *lanIp, char *dns1);
#endif


#if defined(AEI_VDSL_DNS_CACHE)
time_t syn_time=0;
int enable_dns_cache=0;
 
int dns_cache_enabled()
{
   time_t t=time(NULL);
   if (t-syn_time>3)
   {
	syn_time=t;
   	struct stat stats;
   	if(stat("/var/dns_cache", &stats) == 0)
	  enable_dns_cache=1;
	else
	  enable_dns_cache=0;
   }
   return enable_dns_cache;
   
}
#endif

/*****************************************************************************/
int is_connected()
{
#if 0 //BRCM
  FILE *fp;
  if(!config.ppp_detect)return 1;

  fp = fopen( config.ppp_device_file, "r" );
  if(!fp)return 0;
  fclose(fp);
  return 1;
#endif
  //BRCM
  return dns_wanup;
}
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)

#define CT_RANDOM_PORT_START   11000
#define CT_RANDOM_PORT_END     21000

/*****************************************************************************/
int dns_init_ipv6socket()
{
  struct sockaddr_in6 mySocketAddress;
  int ipv6socket = -1;
  int a,b,z;
  a = CT_RANDOM_PORT_START;
  b = CT_RANDOM_PORT_END;
  
  ipv6socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( ipv6socket < 0 )
  {
     debug_perror("Could not create ipv6socket");
     return -1;
  } 

  memset((void *)&mySocketAddress, 0, sizeof(mySocketAddress));
  z = ((double)rand()/RAND_MAX)*(b-a) + a;

  dns_queryport_ipv6 = z;

  mySocketAddress.sin6_family = AF_INET6;
  mySocketAddress.sin6_addr = in6addr_any;
  mySocketAddress.sin6_port = htons(z);
  
  /* bind() the socket to the interface */
  if (bind(ipv6socket, (struct sockaddr *)&mySocketAddress, sizeof(mySocketAddress)) < 0)
  {
     /* fix resource leak, coverity id 11834 */
#if defined(AEI_COVERITY_FIX)
     close(ipv6socket);
#endif
     debug_perror("dns_init: bind: Could not bind to port");
     return -1;
  }

  return ipv6socket;
}

/*****************************************************************************/
int dns_init()
{
  int ret;
  int opt = 1;
  struct sockaddr_in6 mySocketAddress;

  /* Clear it out */
  memset((void *)&mySocketAddress, 0, sizeof(mySocketAddress));
    
  dns_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( dns_sock < 0 )
  {
     debug_perror("Could not create socket");
     exit(1);
  } 

  mySocketAddress.sin6_family = AF_INET6;
  mySocketAddress.sin6_addr = in6addr_any;
  mySocketAddress.sin6_port = htons(PORT);
 
  /* bind() the socket to the interface */
  if (bind(dns_sock, (struct sockaddr *)&mySocketAddress, sizeof(mySocketAddress)) < 0)
  {
     debug_perror("dns_init: bind: Could not bind to port");
     exit(1);
  }

  setsockopt(dns_sock, SOL_IP, IPV6_PKTINFO, &opt, sizeof(opt));

  dns_querysock_ipv6 = dns_init_ipv6socket();
  while(dns_querysock_ipv6 <0 )
  {
     dns_querysock_ipv6 = dns_init_ipv6socket();
  }

  // BRCM: use a different socket for queries so we can use ephemeral port
  // Our corp DNS server does not like queries with src port 53.
  dns_querysock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( dns_querysock < 0 )
  {
     debug_perror("Could not create query socket");
     exit(1);
  } 

  memset((void *)&mySocketAddress, 0, sizeof(mySocketAddress));

  mySocketAddress.sin6_family = AF_INET6;
  mySocketAddress.sin6_addr = in6addr_any;
  mySocketAddress.sin6_port = htons(0);
  
  /* bind() the socket to the interface */
  if (bind(dns_querysock, (struct sockaddr *)&mySocketAddress, sizeof(mySocketAddress)) < 0)
  {
     debug_perror("dns_init: bind: Could not bind to ephmeral port");
     exit(1);
  }


  // BRCM: Connect to ssk
  if ((ret = cmsMsg_init(EID_DNSPROXY, &msgHandle)) != CMSRET_SUCCESS) 
  {
    debug_perror("dns_init: cmsMsg_init() failed");
    exit(1);
  }
  cmsMsg_getEventHandle(msgHandle, &msg_fd);

  dns_main_quit = 0;

  FD_ZERO( &rfds );
  FD_SET( dns_sock, &rfds );
  FD_SET( dns_querysock_ipv6, &rfds );
  FD_SET( dns_querysock, &rfds );
  FD_SET( msg_fd, &rfds );

#ifdef DNS_DYN_CACHE
  dns_dyn_hosts_add();
#endif

  //BCM
  dns_wanup = dns_probe_init();
  
  return 1;
}

void dns_handle_ipv6srvaddr_changed(CmsMsgHeader *msg)
{
   if(msg == NULL)
   {
      printf("dproxy get the msg to change the IPv6 DNS server address, but the message is NULL \n");
      return;
   }

   if(msg->wordData == 0) /* Clear the IPv6 DNS Server Address*/
   {
      if(name_server_ipv6)
      {
         free(name_server_ipv6);
         name_server_ipv6 = NULL;
      }
   }
   else if(msg->wordData == 1) /* Set the IPv6 DNS Server Address*/
   {
      if(msg->dataLength>0)
      {
         char data[128];
         char *separator = NULL;
         memcpy(data, (char *) (msg + 1), msg->dataLength);
         while ((separator = strchr(data, ',')))
         {
            /* TODO: Now only support one IPv6 DNS Server Address*/
            *separator = '\0';
         }
         if(name_server_ipv6)
         {
            free(name_server_ipv6);
         }
         name_server_ipv6 = malloc(strlen(data)+1);
         if(name_server_ipv6 == NULL)
         {
            printf("dns_handle_ipv6srvaddr_changed alloc memory failed\n");
            return;
         }
         strcpy(name_server_ipv6, data);
      }
   }
}

#else /* DMP_X_BROADCOM_COM_IPV6_1 */
/*****************************************************************************/
int dns_init()
{
  struct sockaddr_in sa;
  struct in_addr ip;
  int ret;

  /* Clear it out */
  memset((void *)&sa, 0, sizeof(sa));
    
  dns_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( dns_sock < 0 ){
	 debug_perror("Could not create socket");
	 exit(1);
  } 

  ip.s_addr = INADDR_ANY;
  sa.sin_family = AF_INET;
  memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
  sa.sin_port = htons(PORT);
  
  /* bind() the socket to the interface */
  if (bind(dns_sock, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0){
	 debug_perror("dns_init: bind: Could not bind to port");
	 exit(1);
  }


  // BRCM: use a different socket for queries so we can use ephemeral port
  // Our corp DNS server does not like queries with src port 53.
  dns_querysock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( dns_querysock < 0 ){
	 debug_perror("Could not create query socket");
	 exit(1);
  } 

  ip.s_addr = INADDR_ANY;
  sa.sin_family = AF_INET;
  memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
  sa.sin_port = htons(0);
  
  /* bind() the socket to the interface */
  if (bind(dns_querysock, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0){
	 debug_perror("dns_init: bind: Could not bind to ephmeral port");
	 exit(1);
  }


  // BRCM: Connect to ssk
  if ((ret = cmsMsg_init(EID_DNSPROXY, &msgHandle)) != CMSRET_SUCCESS) {
  	debug_perror("dns_init: cmsMsg_init() failed");
	exit(1);
  }
  cmsMsg_getEventHandle(msgHandle, &msg_fd);

  dns_main_quit = 0;

  FD_ZERO( &rfds );
  FD_SET( dns_sock, &rfds );
  FD_SET( dns_querysock, &rfds );
  FD_SET( msg_fd, &rfds );

#ifdef DNS_DYN_CACHE
  dns_dyn_hosts_add();
#endif

#if defined(AEI_VDSL_DNS_CACHE) 
  cache_purge( config.purge_time );
#endif

  //BCM
  dns_wanup = dns_probe_init();
  
  return 1;
}
#endif  /* DMP_X_BROADCOM_COM_IPV6_1 */


/*****************************************************************************/
void dns_handle_new_query(dns_request_t *m)
{
  struct in_addr in;
  int retval = -1;
  dns_dyn_list_t *dns_entry;
 
  /*BRCM: support IPv4 only*/
  if( m->message.question[0].type == A /*|| m->message.question[0].type == AAA*/){
      /* standard query */
      //retval = cache_lookup_name( m->cname, m->ip );
      
#ifdef DNS_DYN_CACHE
#if defined(AEI_VDSL_DNS_CACHE) 
    retval = cache_lookup_name( m->cname, m->ip );
#ifdef AEI_CONTROL_LAYER
    if (0==retval)
    {
    		char name_app[128] = {0};
		snprintf( name_app, sizeof(name_app)-1, "[%s]", m->cname );
		retval = cache_lookup_name( name_app, m->ip );
		if (retval==1)
			retval= 5;
    }
#endif
   
    if(0 == retval) 
    {
        if(!dns_cache_enabled())
            retval = 0;
        else
        {
#endif
      if ((dns_entry = dns_dyn_find(m))) {
#if defined(AEI_COVERITY_FIX)
          /*CID 12232: Copy into fixed size buffer*/
          strlcpy(m->ip, inet_ntoa(dns_entry->addr), sizeof(m->ip));
#else
        strcpy(m->ip, inet_ntoa(dns_entry->addr));
#endif
        m->ttl = abs(dns_entry->expiry - time(NULL));
        retval = 1;
      }
      else
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
      {
         //ipv4 dns is up
         if( is_connected())
         {
            retval = 0;
         }
         //ipv4 dns is down
         else
         {
            if(name_server_ipv6)
               retval = 6;           
            else
               retval = 0;
         }
      }

#else
        retval = 0;
#endif
#if defined(AEI_VDSL_DNS_CACHE) 
        }
    }
#endif
#endif

  }else if( m->message.question[0].type == PTR ) {
      /* reverse lookup */
      //retval = cache_lookup_ip( m->ip, m->cname );

#ifdef DNS_DYN_CACHE
#ifdef AEI_VDSL_DNS_CACHE
        if(!dns_cache_enabled())
            retval = 0;
        else
        {
#endif
      if ((dns_entry = dns_dyn_find(m))) {
#if defined(AEI_COVERITY_FIX)
          /*CID 12232: Copy into fixed size buffer*/
          strlcpy(m->cname, dns_entry->cname, sizeof(m->cname));
#else
        strcpy(m->cname, dns_entry->cname);
#endif
        m->ttl = abs(dns_entry->expiry - time(NULL));
        retval = 1;
      }
      else 
        retval = 0;
#ifdef AEI_VDSL_DNS_CACHE
        }
#endif	
#endif

  }
  else if( m->message.question[0].type == AAA)
  { /* IPv6 */
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
  		/*modify by libby support IPV6*/
#ifdef AEI_VDSL_DNS_CACHE
		char name_app[128] = {0};
		snprintf( name_app, sizeof(name_app)-1, "[%s]", m->cname );
		retval = cache_lookup_name( name_app, m->ip );
		//strcpy(m->cname,name_app);
		if( 1 != retval )
#endif
#endif

// To support query to WAN IPv6 DNS server (retval == 6),
// must implement CMS_MSG_DNSPROXY_IPV6_CHANGED to notice IPv6 DNS server info change.
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
		{
			debug(" remote ipv6 dns server address is %s\n", name_server_ipv6 );
			if(name_server_ipv6)
			{
				retval = 6;
			}
			else
			{
				retval = 0;
			}
		}
#else
      retval = 0;
#endif
  }
  else //BRCM
  {
      retval = 0;
  }

#ifndef DMP_X_BROADCOM_COM_IPV6_1   /* Fixme ~~~~ inet_ntoa cannot be used for IPv6 address; Use inet_ntop */
#ifndef AEI_CONTROL_LAYER 
  debug(".......... %s ---- %s\n", m->cname, inet_ntoa(m->src_addr));
#endif
#endif /* !DMP_X_BROADCOM_COM_IPV6_1 */
  switch( retval )
  {

     case 0:
     {
        char dns1[CMS_IPADDR_LENGTH];
        debug("config.name_server=%s is_connected=%d", config.name_server, is_connected());

        if (dns_subnet_map(&(m->src_addr), dns1) == TRUE)
        {
#ifndef DMP_X_BROADCOM_COM_IPV6_1   /* Fixme ~~~~ inet_ntoa cannot be used for IPv6 address; Use inet_ntop */
#ifndef AEI_CONTROL_LAYER 
	cmsLog_notice("Found dns %s for subnet %s", dns1, inet_ntoa(m->src_addr));
#endif
#endif /* !DMP_X_BROADCOM_COM_IPV6_1 */
           inet_aton(dns1, &in);
        }
        else
        {
            inet_aton( config.name_server, &in );
        }   
    
        if( is_connected()){
           debug("Adding to list-> id: %d\n", m->message.header.id);
#ifndef DNS_PROBE
           dns_list_add(m, dns_probe_is_primary_up());
#else
            dns_list_add(m,0);
#endif
           /* relay the query untouched */
           debug("sending query out on dns_querysock\n");
        }else{
           debug("wan not up, No DNS information: send to magic ppp addr.\n");
        }
		
        dns_write_packet( dns_querysock, in, PORT, m );
		

        
		
        break;
     }
	 
    
     case 1:
        dns_construct_reply( m );
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
        dns_write_packet_ipv6( dns_sock, m->src_addr, m->src_port, m );
#else
        dns_write_packet( dns_sock, m->src_addr, m->src_port, m );
#endif
        debug("Cache hit\n");
        break;
		
#ifdef AEI_CONTROL_LAYER
	case 5:
		dns_construct_reject_reply( m );
		dns_write_packet_ipv6( dns_sock, m->src_addr, m->src_port, m );
        debug("rejiect hit\n");
        break;
#endif 

#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
     case 6:

        debug(" remote ipv6 dns server address is %s\n", name_server_ipv6 );

        if(name_server_ipv6)
        {
           struct in6_addr in6;
           inet_pton(AF_INET6, name_server_ipv6, &in6);
#ifndef DNS_PROBE

           dns_list_add(m, dns_probe_is_primary_up());
#else
           dns_list_add(m,0);
#endif

           dns_write_packet_ipv6( dns_querysock_ipv6, in6, PORT, m );
        }
        break;
#endif
     default:
        debug("Unknown query type: %d\n", m->message.question[0].type );
   }
}
/*****************************************************************************/
void dns_handle_request(dns_request_t *m)
{
  struct in_addr in;
  dns_request_t *ptr = NULL;

  /* request may be a new query or a answer from the upstream server */
  ptr = dns_list_find_by_id(m);

  if( ptr != NULL ){
      debug("Found query in list; id 0x%04x flags 0x%04x\n ip %s --- cname %s\n", 
             m->message.header.id, m->message.header.flags.flags, m->ip, m->cname);

      /* message may be a response */
      if( m->message.header.flags.flags & 0x8000)
      {
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
          dns_write_packet_ipv6( dns_sock, ptr->src_addr, ptr->src_port, m );
          char ipaddr[40];
          inet_ntop(AF_INET6, &ptr->src_addr, ipaddr, sizeof(ipaddr));
          debug("---->sending to  %s:%d thru sock %d\n", ipaddr, ptr->src_port, dns_sock);
          inet_ntop(AF_INET6, &m->src_addr, ipaddr, sizeof(ipaddr));
          debug("Replying with answer from %s, id 0x%04x\n", ipaddr, m->message.header.id);
#else
          dns_write_packet( dns_sock, ptr->src_addr, ptr->src_port, m );
          debug("Replying with answer from %s, id 0x%04x\n", inet_ntoa( m->src_addr), m->message.header.id);
#endif
          dns_list_unarm_requests_after_this( ptr );  // BRCM
          dns_list_remove( ptr );         
#ifdef DMP_X_ITU_ORG_GPON_1
          if( m->message.header.flags.f.rcode != 0 ){ // lookup failed
              dns_query_error++;
              debug("dns_query_error = %u\n", dns_query_error);
          }
#endif
      #if 0 //BRCM: Don't write to cache for saving device resource.
          if( m->message.header.flags.f.rcode == 0 ){ // lookup was succesful
              debug("Cache append: %s ----> %s\n", m->cname, m->ip );
              cache_name_append( m->cname, m->ip );
          }
      #endif
#ifdef DNS_DYN_CACHE
#ifdef AEI_VDSL_DNS_CACHE
      if(dns_cache_enabled())
#endif
          if( m->message.question[0].type != AAA) /* No cache for IPv6 */
          {
             dns_dyn_cache_add(m);
          }
#endif
          //BRCM
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
          /* mwang: this is clearly wrong, need ipv6-review here */
          dns_probe_activate((uint32) m->src_addr.s6_addr);
#else
          dns_probe_activate(m->src_addr.s_addr);
#endif
      }
      else
      {
         debug("Duplicate query to %s (retx_count=%d)", m->cname, ptr->retx_count);
         if (ptr->retx_count < MAX_RETX_COUNT)
         {
            debug("=>send again\n");
            ptr->retx_count++;
            inet_aton( config.name_server, &in );
            dns_write_packet( dns_querysock, in, PORT, m );
         }
         else
         {
            debug("=>drop! retx limit reached.\n");
         }
      }
  }
  else
  {
      dns_handle_new_query( m );
  }

  debug("dns_handle_request: done\n\n");
}

/*****************************************************************************/
static void processCmsMsg(void)
{
  CmsMsgHeader *msg;
  CmsRet ret;

  while( (ret = cmsMsg_receiveWithTimeout(msgHandle, &msg, 0)) == CMSRET_SUCCESS) {
    switch(msg->type) {
    case CMS_MSG_SET_LOG_LEVEL:
      cmsLog_debug("got set log level to %d", msg->wordData);
      cmsLog_setLevel(msg->wordData);
      if ((ret = cmsMsg_sendReply(msgHandle, msg, CMSRET_SUCCESS)) != CMSRET_SUCCESS) {
        cmsLog_error("send response for msg 0x%x failed, ret=%d", msg->type, ret);
      }
      break;
    case CMS_MSG_SET_LOG_DESTINATION:
      cmsLog_debug("got set log destination to %d", msg->wordData);
      cmsLog_setDestination(msg->wordData);
      if ((ret = cmsMsg_sendReply(msgHandle, msg, CMSRET_SUCCESS)) != CMSRET_SUCCESS) {
        cmsLog_error("send response for msg 0x%x failed, ret=%d", msg->type, ret);
      }
      break;

#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined (AEI_CONTROL_LAYER)// AEI_IPV6_TODO
    case CMS_MSG_DNSPROXY_IPV6_CHANGED:
      dns_handle_ipv6srvaddr_changed(msg);
      if ((ret = cmsMsg_sendReply(msgHandle, msg, CMSRET_SUCCESS)) != CMSRET_SUCCESS) 
      {
         cmsLog_error("send response for msg 0x%x failed, ret=%d", msg->type, ret);
      }
      break;

#endif

    case CMS_MSG_DNSPROXY_RELOAD:
      cmsLog_debug("received CMS_MSG_DNSPROXY_RELOAD\n");
      /* Reload config file */
#ifdef DNS_DYN_CACHE
      dns_dyn_hosts_add();
#endif

      /* load the /var/wandns into the linked list for dns/subnet pair */
      dns_sunbet_init();
     

      dns_wanup = dns_probe_init();

      /*
       * During CDRouter dhcp scaling tests, this message is sent a lot to dnsproxy.
       * To make things more efficient/light weight, the sender of the message does
       * not expect a reply.
       */
      break;

#ifdef SUPPORT_DEBUG_TOOLS

    case CMS_MSG_DNSPROXY_DUMP_STATUS:
       printf("\n============= Dump dnsproxy status=====\n");
       printf("config.name_server=%s config.domain=%s dns_wanup=%d\n",
              config.name_server, config.domain_name, dns_wanup);
       dns_probe_print();
       dns_list_print();
       dns_dyn_print();
       break;

    case CMS_MSG_DNSPROXY_DUMP_STATS:
       printf("stats dump not implemented yet\n");
       break;

#endif /* SUPPORT_DEBUG_TOOLS */

#ifdef DMP_X_ITU_ORG_GPON_1
    case CMS_MSG_DNSPROXY_GET_STATS:
    {
        char buf[sizeof(CmsMsgHeader) + sizeof(DnsGetStatsMsgBody)]={0};
        CmsMsgHeader *replyMsgPtr = (CmsMsgHeader *) buf;
        DnsGetStatsMsgBody *dnsStats = (DnsGetStatsMsgBody *) (replyMsgPtr+1);


        // Setup response message.
        replyMsgPtr->type = msg->type;
        replyMsgPtr->dst = msg->src;
        replyMsgPtr->src = EID_DNSPROXY;
        replyMsgPtr->flags.all = 0;
        replyMsgPtr->flags_response = 1;
        //set dns query error counter 
        replyMsgPtr->dataLength = sizeof(DnsGetStatsMsgBody);
        dnsStats->dnsErrors = dns_query_error;

        cmsLog_notice("dns query error is %d", dns_query_error);
        // Attempt to send CMS response message & test result.
        ret = cmsMsg_send(msgHandle, replyMsgPtr);
        if (ret != CMSRET_SUCCESS)
        {
           // Log error.
           cmsLog_error("Send message failure, cmsReturn: %d", ret);
        }
    }
    break;
#endif

    default:
      cmsLog_error("unrecognized msg 0x%x", msg->type);
      break;
    }
    CMSMEM_FREE_BUF_AND_NULL_PTR(msg);
  }
  
  if (ret == CMSRET_DISCONNECTED) {
    cmsLog_error("lost connection to smd, exiting now.");
    dns_main_quit = 1;
  }
}

/*****************************************************************************/
int dns_main_loop()
{
    struct timeval tv, *ptv;
    fd_set active_rfds;
    int next_probe_time = 0;
    int retval;
    dns_request_t m;
    dns_request_t *ptr, *next;
#if defined(AEI_VDSL_DNS_CACHE) 
    int purge_time = PURGE_TIMEOUT;
#endif

    while( !dns_main_quit )
    {
      /* set the one second time out */
      //BRCM: set timeout to the earliest pending request's timeout or
      //next probe time (if in probing procedure). If there
      //is no pending requests and not in probing procedure, timeout will
      //be 0, causing select() to wait forever until received packets on
      //any sockets.
      int next_request_time = dns_list_next_time();
      if (next_request_time) {
         if (next_request_time < next_probe_time || !next_probe_time) {
            debug("use next_request_time = %d", next_request_time);
            tv.tv_sec = next_request_time;
         } else {
            tv.tv_sec = next_probe_time;
         }
      } else {
         tv.tv_sec = next_probe_time;
      }
	tv.tv_usec = 0;
      if (tv.tv_sec == 0) { /* To wait indefinitely */
         ptv = NULL;
         debug("\n\n=============select will wait indefinitely============");
      } else {
        tv.tv_usec = 0;
        ptv = &tv;
        debug("select timeout = %lu seconds", tv.tv_sec);
     }

      /* now copy the main rfds in the active one as it gets modified by select*/
      active_rfds = rfds;

      retval = select( FD_SETSIZE, &active_rfds, NULL, NULL, ptv );
      if (retval){
         debug("received something");

         if (FD_ISSET(msg_fd, &active_rfds)) { /* message from ssk */
            debug("received cms message");
            processCmsMsg();

         } else if (FD_ISSET(dns_sock, &active_rfds)) { /* DNS message */
            debug("received DNS message (LAN side)");
            /* data is now available */
            bzero(&m, sizeof(dns_request_t));
            //BRCM
            //dns_read_packet( dns_sock, &m );
            if (dns_read_packet(dns_sock, &m) == 0) {
               dns_handle_request( &m );
            }

         } else if (FD_ISSET(dns_querysock, &active_rfds)) {
            debug("received DNS response (WAN side)");
            bzero(&m, sizeof(dns_request_t));
            if (dns_read_packet(dns_querysock, &m) == 0 && !dns_probe_response(&m))
               dns_handle_request( &m );
         }
#if defined (DMP_X_BROADCOM_COM_IPV6_1 )|| defined (AEI_CONTROL_LAYER)
         else if (FD_ISSET(dns_querysock_ipv6, &active_rfds)) 
         {
            debug("received DNS response (WAN side IPv6)");
            bzero(&m, sizeof(dns_request_t));
            if (dns_read_packet(dns_querysock_ipv6, &m) == 0 && !dns_probe_response(&m))
            {
               dns_handle_request( &m );
            }
         }
#endif
      } else { /* select time out */
         time_t now = time(NULL);
         int doSwitch=0;
#ifndef DNS_PROBE
         debug("select timed out, next_request_time=%d next_probe_time=%d dns_recover_time=%d",
                next_request_time, next_probe_time, dns_recover_time);
#endif
         /*
          * There could be several reasons for select timeout.
          * a) It is time for switching back to primary server.  This will be
          *    handled at the end of the else block in dns_probe().
          * b) A query timed out, but we know from other dns responses that the
          *    dns server is actually up. The UDP request or response from this
          *    query was simply lost.  Do not switch dns servers.  Do not send an error
          *    reply to the LAN client, just let it timeout.  This is what the LAN
          *    client would experience if no dnsproxy was here.
          * c) A query timed out, and we suspect the primary server is down.
          *    Switch to the secondary DNS server.
          */
#ifndef DNS_PROBE
         if (now >= dns_recover_time) {
            dns_probe_switchback();
         }
#endif
         ptr = dns_request_list;
         while (ptr) {
            next = ptr->next;

            if (ptr->expiry <= now) {
               char date_time_buf[DATE_TIME_BUFLEN];
               get_date_time_str(date_time_buf, sizeof(date_time_buf));

               debug("removing expired request %p\n", ptr);
               debug("%s dnsproxy: query for %s timed out after %d secs "
                      "(type=%d switch_on_timeout=%d retx_count=%d)\n",
                      date_time_buf, ptr->cname, DNS_TIMEOUT,
                      (unsigned int) ptr->message.question[0].type,
                      ptr->switch_on_timeout, ptr->retx_count);

               if (ptr->switch_on_timeout)
               {
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
                  /*TODO : We also need to support the IPv6 DNS Server address Switch feature*/
                  if(ptr->message.question[0].type != AAA)
                     doSwitch = 1;
#else
                  doSwitch = 1;
#endif
                  /*
                   * I don't see the point of sending an error reply to
                   * the LAN client.  Why not just let it timeout.  It will
                   * resend the request anyways, right?  Since the original
                   * dproxy code did it, I'll leave it here.  mwang 8/31/09.
                   */
                  dns_construct_error_reply(ptr);
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
                  dns_write_packet_ipv6( dns_sock, ptr->src_addr, ptr->src_port, ptr );
#else
                  dns_write_packet( dns_sock, ptr->src_addr, ptr->src_port, ptr );
#endif
               }

               dns_list_remove(ptr);
            }

            ptr = next;
         }

#ifndef DNS_PROBE
         if (doSwitch) {
            dns_probe_set_recover_time();
         }
#endif

#if defined(DNS_PROBE) && defined(AEI_VDSL_DNS_PROBE)
         dns_probe_time_init();
#endif
      } /* if (retval) */

      //BRCM
      next_probe_time = dns_probe();

#if defined(AEI_VDSL_DNS_CACHE) 
      purge_time--;
      if( !purge_time ){
         cache_purge( config.purge_time );
         purge_time = PURGE_TIMEOUT;
      }
#endif

    }  /* while (!dns_main_quit) */
   return 0;
}


/*
 * Return a buffer which contains the current date/time.
 */
void get_date_time_str(char *buf, unsigned int buflen)
{
	time_t t;
	struct tm *tmp;

	memset(buf, 0, buflen);

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL) {
		debug_perror("could not get localtime");
		return;
	}

	strftime(buf, buflen, "[%F %k:%M:%S]", tmp);

}


/*****************************************************************************/
void debug_perror( char * msg ) {
	debug( "%s : %s\n" , msg , strerror(errno) );
}

#if 0 //BRCM: Mask the debug() function. It's redefined by cmsLog_debug()
#ifdef DPROXY_DEBUG
/*****************************************************************************/
void debug(char *fmt, ...)
{
#define MAX_MESG_LEN 1024
  
  va_list args;
  char text[ MAX_MESG_LEN ];
  
  sprintf( text, "[ %d ]: ", getpid());
  va_start (args, fmt);
  vsnprintf( &text[strlen(text)], MAX_MESG_LEN - strlen(text), fmt, args);	 
  va_end (args);
  
  printf(text);
#if 0 //BRCM 
  if( config.debug_file[0] ){
	 FILE *fp;
	 fp = fopen( config.debug_file, "a");
	 if(!fp){
		syslog( LOG_ERR, "could not open log file %m" );
		return;
	 }
	 fprintf( fp, "%s", text);
	 fclose(fp);
  }
  

  /** if not in daemon-mode stderr was not closed, use it. */
  if( ! config.daemon_mode ) {
	 fprintf( stderr, "%s", text);
  }
#endif
}

#endif
#endif
/*****************************************************************************
 * print usage informations to stderr.
 * 
 *****************************************************************************/
void usage(char * program , char * message ) {
  fprintf(stderr,"%s\n" , message );
  fprintf(stderr,"usage : %s [-c <config-file>] [-d] [-h] [-P]\n", program );
  fprintf(stderr,"\t-c <config-file>\tread configuration from <config-file>\n");
  fprintf(stderr,"\t-d \t\trun in debug (=non-daemon) mode.\n");
  fprintf(stderr,"\t-D \t\tDomain Name\n");
  fprintf(stderr,"\t-P \t\tprint configuration on stdout and exit.\n");
  fprintf(stderr,"\t-v \t\tset verbosity, where num==0 is LOG_ERROR, 1 is LOG_NOTICE, all others is LOG_DEBUG\n");
  fprintf(stderr,"\t-h \t\tthis message.\n");
}
/*****************************************************************************
 * get commandline options.
 * 
 * @return 0 on success, < 0 on error.
 *****************************************************************************/
int get_options( int argc, char ** argv ) 
{
  char c = 0;
  int not_daemon = 0;
  int want_printout = 0;
  char * progname = argv[0];
  SINT32 logLevelNum;
  CmsLogLevel logLevel=DEFAULT_LOG_LEVEL;
  //UBOOL8 useConfiguredLogLevel=TRUE;

  cmsLog_init(EID_DNSPROXY);

  conf_defaults();
#if 1 
  while( (c = getopt( argc, argv, "cD:dhPv:")) != EOF ) {
    switch(c) {
	 case 'c':
  		conf_load(optarg);
		break;
	 case 'd':
		not_daemon = 1;
		break;
     case 'D':
#if defined(AEI_COVERITY_FIX)
         /*CID 12233: Copy into fixed size buffer*/
         strlcpy(config.domain_name, optarg, sizeof(config.domain_name));
#else
        strcpy(config.domain_name, optarg);
#endif
        break;
	 case 'h':
		usage(progname,"");
		return -1;
	 case 'P':
		want_printout = 1;
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
         	//useConfiguredLogLevel = FALSE;
         	break;
	 default:
		usage(progname,"");
		return -1;
    }
  }
#endif

#if 0  
  /** unset daemon-mode if -d was given. */
  if( not_daemon ) {
	 config.daemon_mode = 0;
  }
  
  if( want_printout ) {
	 conf_print();
	 exit(0);
  }
#endif 

  return 0;
}
/*****************************************************************************/
void sig_hup (int signo)
{
  signal(SIGHUP, sig_hup); /* set this for the next sighup */
  conf_load (config.config_file);
}
/*****************************************************************************/
int main(int argc, char **argv)
{

  /* get commandline options, load config if needed. */
  if(get_options( argc, argv ) < 0 ) {
	  exit(1);
  }

  signal(SIGHUP, sig_hup);

  dns_init();

//BRCM: Don't fork a task again!
#if 0 
  if (config.daemon_mode) {
    /* Standard fork and background code */
    switch (fork()) {
	 case -1:	/* Oh shit, something went wrong */
		debug_perror("fork");
		exit(-1);
	 case 0:	/* Child: close off stdout, stdin and stderr */
		close(0);
		close(1);
		close(2);
		break;
	 default:	/* Parent: Just exit */
		exit(0);
    }
  }
#endif
  dns_main_loop();

  return 0;
}

