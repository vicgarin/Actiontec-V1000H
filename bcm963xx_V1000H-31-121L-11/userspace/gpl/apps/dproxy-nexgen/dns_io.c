#include <sys/socket.h>
#include <sys/uio.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dns_io.h"
#include "dns_decode.h"

/*****************************************************************************/
int dns_read_packet(int sock, dns_request_t *m)
{

#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
  struct sockaddr_in6 sa;
#else
  struct sockaddr_in sa;
#endif

  socklen_t salen;
  
  /* Read in the actual packet */
  salen = sizeof(sa);
  
  m->numread = recvfrom(sock, m->original_buf, sizeof(m->original_buf), 0,
		     (struct sockaddr *)&sa, &salen);
  
  if ( m->numread < 0) {
    debug_perror("dns_read_packet: recvfrom\n");
    return -1;
  }
  
  /* TODO: check source addr against list of allowed hosts */

  /* record where it came from */
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
  memcpy( (void *)&m->src_addr, (void *)&sa.sin6_addr, sizeof(struct in6_addr));
  m->src_port = ntohs( sa.sin6_port );
#else
  memcpy( (void *)&m->src_addr, (void *)&sa.sin_addr, sizeof(struct in_addr));
  m->src_port = ntohs( sa.sin_port );
  debug("received pkt from %s:%d len %d", inet_ntoa(m->src_addr),
         m->src_port, m->numread);
#endif

  /* check that the message is long enough */
  if( m->numread < sizeof (m->message.header) )
  {
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
    char ipaddr[40];
    inet_ntop(AF_INET6, &sa.sin6_addr, ipaddr, sizeof(ipaddr));
    debug("dns_read_packet: packet from '%s' to short to be dns packet", ipaddr );
#else
    debug("dns_read_packet: packet from '%s' to short to be dns packet", inet_ntoa (sa.sin_addr) );
#endif
    return -1;
  }

  /* pass on for full decode */
  dns_decode_request( m );

  return 0;
}
/*****************************************************************************/
int dns_write_packet(int sock, struct in_addr in, int port, dns_request_t *m)
{
  struct sockaddr_in sa;
  int retval;

  /* Zero it out */
  memset((void *)&sa, 0, sizeof(sa));

  /* Fill in the information */
  //inet_aton( "203.12.160.35", &in );
  memcpy( &sa.sin_addr.s_addr, &in, sizeof(in) );
  sa.sin_port = htons(port);
  sa.sin_family = AF_INET;

  retval = sendto(sock, m->original_buf, m->numread, 0, 
		(struct sockaddr *)&sa, sizeof(sa));
  
  if( retval < 0 ){
    debug_perror("dns_write_packet: sendto");
  }

  return retval;
}

/*****************************************************************************/
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
int dns_write_packet_ipv6(int sock, struct in6_addr in, int port, dns_request_t *m)
{
  struct sockaddr_in6 sa;
  int retval;

  /* Zero it out */
  memset((void *)&sa, 0, sizeof(sa));

  /* Fill in the information */
  //inet_aton( "203.12.160.35", &in );

  memcpy( &sa.sin6_addr.s6_addr, &in, sizeof(in) );
  sa.sin6_port = htons(port);
  sa.sin6_family = AF_INET6;
  retval = sendto(sock, m->original_buf, m->numread, 0, (struct sockaddr *)&sa, sizeof(sa));


  if( retval < 0 ){
    debug_perror("dns_write_packet: sendto");
  }

  return retval;
}
#endif
