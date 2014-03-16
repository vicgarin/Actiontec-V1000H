#ifndef _SERVERPACKET_H
#define _SERVERPACKET_H

int sendOffer(struct dhcpMessage *oldpacket);
int sendNAK(struct dhcpMessage *oldpacket);
int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr);
int send_inform(struct dhcpMessage *oldpacket);

#if defined(AEI_VDSL_DHCP_LEASE)
UBOOL8 is_stb(const char *vid);
#endif

#endif
