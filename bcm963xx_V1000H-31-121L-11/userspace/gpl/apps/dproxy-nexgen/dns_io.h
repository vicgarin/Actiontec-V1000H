#include "dproxy.h"

int dns_read_packet(int sock, dns_request_t *m);
int dns_write_packet(int sock, struct in_addr in, int port, dns_request_t *m);
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
int dns_write_packet_ipv6(int sock, struct in6_addr in, int port, dns_request_t *m);
#endif