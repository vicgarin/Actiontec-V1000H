/* ebt_ip6
 * 
 * Authors:
 * Kuo-Lang Tseng <kuo-lang.tseng@intel.com>
 * Manohar Castelino <manohar.castelino@intel.com>
 *
 * Summary:
 * This is just a modification of the IPv4 code written by 
 * Bart De Schuymer <bdschuym@pandora.be>
 * with the changes required to support IPv6
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include "../include/ebtables_u.h"
#include "../include/linux/netfilter_bridge/ebt_ip6.h"
#include <arpa/inet.h>



#define IP_SOURCE '1'
#define IP_DEST   '2'
#define IP_TCLASS '3'
#define IP_PROTO  '4'
#define IP_SPORT  '5'
#define IP_DPORT  '6'

static struct option opts[] =
{
	{ "ip6-source"           , required_argument, 0, IP_SOURCE },
	{ "ip6-src"              , required_argument, 0, IP_SOURCE },
	{ "ip6-destination"      , required_argument, 0, IP_DEST   },
	{ "ip6-dst"              , required_argument, 0, IP_DEST   },
	{ "ip6-traffic-class"    , required_argument, 0, IP_TCLASS },
	{ "ip6-tclass"           , required_argument, 0, IP_TCLASS },
	{ "ip6-protocol"         , required_argument, 0, IP_PROTO  },
	{ "ip6-proto"            , required_argument, 0, IP_PROTO  },
	{ "ip6-source-port"      , required_argument, 0, IP_SPORT  },
	{ "ip6-sport"            , required_argument, 0, IP_SPORT  },
	{ "ip6-destination-port" , required_argument, 0, IP_DPORT  },
	{ "ip6-dport"            , required_argument, 0, IP_DPORT  },
	{ 0 }
};

/* transform a protocol and service name into a port number */
static uint16_t parse_port(const char *protocol, const char *name)
{
	struct servent *service;
	char *end;
	int port;

	port = strtol(name, &end, 10);
	if (*end != '\0') {
		if (protocol && 
		    (service = getservbyname(name, protocol)) != NULL)
			return ntohs(service->s_port);
	}
	else if (port >= 0 || port <= 0xFFFF) {
		return port;
	}
	print_error("Problem with specified %s port '%s'", 
			protocol?protocol:"", name);
	return 0;
}

static void
parse_port_range(const char *protocol, const char *portstring, uint16_t *ports)
{
	char *buffer;
	char *cp;
	
	buffer = strdup(portstring);
	if ((cp = strchr(buffer, ':')) == NULL)
		ports[0] = ports[1] = parse_port(protocol, buffer);
	else {
		*cp = '\0';
		cp++;
		ports[0] = buffer[0] ? parse_port(protocol, buffer) : 0;
//		if (ebt_errormsg[0] != '\0')
//			return;
		ports[1] = cp[0] ? parse_port(protocol, cp) : 0xFFFF;
//		if (ebt_errormsg[0] != '\0')
//			return;
		
		if (ports[0] > ports[1])
			print_error("Invalid portrange (min > max)");
	}
	free(buffer);
}

static void print_port_range(uint16_t *ports)
{
	if (ports[0] == ports[1])
		printf("%d ", ports[0]);
	else
		printf("%d:%d ", ports[0], ports[1]);
}

static void print_help()
{
	printf(
"ip6 options:\n"
"--ip6-src    [!] address[/mask]: ipv6 source specification\n"
"--ip6-dst    [!] address[/mask]: ipv6 destination specification\n"
"--ip6-tclass [!] tclass        : ipv6 traffic class specification\n"
"--ip6-proto  [!] protocol      : ipv6 protocol specification\n"
"--ip6-sport  [!] port[:port]   : tcp/udp source port or port range\n"
"--ip6-dport  [!] port[:port]   : tcp/udp destination port or port range\n");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	ipinfo->invflags = 0;
	ipinfo->bitmask = 0;
}

#define OPT_SOURCE 0x01
#define OPT_DEST   0x02
#define OPT_TCLASS 0x04
#define OPT_PROTO  0x08
#define OPT_SPORT  0x10
#define OPT_DPORT  0x20

/* Most of the following code is derived from iptables */
static void
in6addrcpy(struct in6_addr *dst, struct in6_addr *src)
{
	memcpy(dst, src, sizeof(struct in6_addr));
}

int string_to_number_ll(const char *s, unsigned long long min,
            unsigned long long max, unsigned long long *ret)
{
	unsigned long long number;
	char *end;

	/* Handle hex, octal, etc. */
	number = strtoull(s, &end, 0);
	if (*end == '\0' && end != s) {
		/* we parsed a number, let's see if we want this */
		if (min <= number && (!max || number <= max)) {
			*ret = number;
			return 0;
		}
	}
	return -1;
}

int string_to_number_l(const char *s, unsigned long min, unsigned long max,
                       unsigned long *ret)
{
	int result;
	unsigned long long number;

	result = string_to_number_ll(s, min, max, &number);
	*ret = (unsigned long)number;

	return result;
}

int string_to_number(const char *s, unsigned int min, unsigned int max,
                     unsigned int *ret)
{
	int result;
	unsigned long number;

	result = string_to_number_l(s, min, max, &number);
	*ret = (unsigned int)number;

	return result;
}

static struct in6_addr *numeric_to_addr(const char *num)
{
	static struct in6_addr ap;
	int err;

	if ((err=inet_pton(AF_INET6, num, &ap)) == 1)
		return &ap;
	return (struct in6_addr *)NULL;
}

static struct in6_addr *parse_ip6_mask(char *mask)
{
	static struct in6_addr maskaddr;
	struct in6_addr *addrp;
	unsigned int bits;

	if (mask == NULL) {
		/* no mask at all defaults to 128 bits */
		memset(&maskaddr, 0xff, sizeof maskaddr);
		return &maskaddr;
	}
	if ((addrp = numeric_to_addr(mask)) != NULL)
		return addrp;
	if (string_to_number(mask, 0, 128, &bits) == -1)
		print_error("Invalid IPv6 Mask '%s' specified", mask);
	if (bits != 0) {
		char *p = (char *)&maskaddr;
		memset(p, 0xff, bits / 8);
		memset(p + (bits / 8) + 1, 0, (128 - bits) / 8);
		p[bits / 8] = 0xff << (8 - (bits & 7));
		return &maskaddr;
	}

	memset(&maskaddr, 0, sizeof maskaddr);
	return &maskaddr;
}

/* Set the ipv6 mask and address. Callers should check ebt_errormsg[0].
 *  * The string pointed to by address can be altered. */
void ebt_parse_ip6_address(char *address, struct in6_addr *addr,
                           struct in6_addr *msk)
{
   struct in6_addr *tmp_addr;
   char buf[256];
   char *p;
   int i;
   int err;

   strncpy(buf, address, sizeof(buf) - 1);
   /* first the mask */
   buf[sizeof(buf) - 1] = '\0';
   if ((p = strrchr(buf, '/')) != NULL) {
      *p = '\0';
      tmp_addr = parse_ip6_mask(p + 1);
   } else
      tmp_addr = parse_ip6_mask(NULL);
   in6addrcpy(msk, tmp_addr);

   /* if a null mask is given, the name is ignored, like in "any/0" */
   if (!memcmp(msk, &in6addr_any, sizeof(in6addr_any)))
      strcpy(buf, "::");

   if ((err=inet_pton(AF_INET6, buf, addr)) < 1) {
      print_error("Invalid IPv6 Address '%s' specified", buf);
      return;
   }

   for (i = 0; i < 4; i++)
      addr->s6_addr32[i] &= msk->s6_addr32[i];
}

/* Transform the ip6 addr into a string ready for output. */
char *ebt_ip6_to_numeric(const struct in6_addr *addrp)
{
   /* 0000:0000:0000:0000:0000:000.000.000.000
    *     * 0000:0000:0000:0000:0000:0000:0000:0000 */
   static char buf[50+1];
      return (char *)inet_ntop(AF_INET6, addrp, buf, sizeof(buf));
}

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)(*match)->data;
	char *end;
	long int i;

	switch (c) {
	case IP_SOURCE:
		check_option(flags, OPT_SOURCE);
		ipinfo->bitmask |= EBT_IP6_SOURCE;
		if (check_inverse(optarg)) {
		    ipinfo->invflags |= EBT_IP6_SOURCE;
		}
		ebt_parse_ip6_address(argv[optind - 1], &ipinfo->saddr, &ipinfo->smsk);
		break;

	case IP_DEST:
		check_option(flags, OPT_DEST);
		ipinfo->bitmask |= EBT_IP6_DEST;
		if (check_inverse(optarg)) {
			ipinfo->invflags |= EBT_IP6_DEST;
		}

      if (optind > argc)
         print_error("Missing IP address argument");

		ebt_parse_ip6_address(argv[optind - 1], &ipinfo->daddr, &ipinfo->dmsk);
		break;

	case IP_SPORT:
	case IP_DPORT:
		if (c == IP_SPORT) {
			check_option(flags, OPT_SPORT);
			ipinfo->bitmask |= EBT_IP6_SPORT;
			if (check_inverse(optarg))
				ipinfo->invflags |= EBT_IP6_SPORT;
		} else {
			check_option(flags, OPT_DPORT);
			ipinfo->bitmask |= EBT_IP6_DPORT;
			if (check_inverse(optarg))
				ipinfo->invflags |= EBT_IP6_DPORT;
		}

      if (optind > argc)
         print_error("Missing port argument");

		if (c == IP_SPORT)
			parse_port_range(NULL, argv[optind - 1], ipinfo->sport);
		else
			parse_port_range(NULL, argv[optind - 1], ipinfo->dport);
		break;

	case IP_TCLASS:
		check_option(flags, OPT_TCLASS);
		if (check_inverse(optarg))
			ipinfo->invflags |= EBT_IP6_TCLASS;

      if (optind > argc)
         print_error("Missing TCLASS argument");

		i = strtol(argv[optind - 1], &end, 16);
		if (i < 0 || i > 255 || *end != '\0')
			print_error("Problem with specified IPv6 traffic class");
		ipinfo->tclass = i;
		ipinfo->bitmask |= EBT_IP6_TCLASS;
		break;

	case IP_PROTO:
		check_option(flags, OPT_PROTO);
		if (check_inverse(optarg))
			ipinfo->invflags |= EBT_IP6_PROTO;

      if (optind > argc)
         print_error("Missing IP protocol argument");

		i = strtoul(argv[optind - 1], &end, 10);
		if (*end != '\0') {
			struct protoent *pe;

			pe = getprotobyname(argv[optind - 1]);
			if (pe == NULL)
				print_error("Unknown specified IP protocol - %s", argv[optind - 1]);
			ipinfo->protocol = pe->p_proto;
		} else {
			ipinfo->protocol = (unsigned char) i;
		}
		ipinfo->bitmask |= EBT_IP6_PROTO;
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	if (entry->ethproto != ETH_P_IPV6 || entry->invflags & EBT_IPROTO) {
		print_error("For IPv6 filtering the protocol must be "
		            "specified as IPv6");
	} else if (ipinfo->bitmask & (EBT_IP6_SPORT|EBT_IP6_DPORT) &&
		(!(ipinfo->bitmask & EBT_IP6_PROTO) ||
		ipinfo->invflags & EBT_IP6_PROTO ||
		(ipinfo->protocol!=IPPROTO_TCP &&
		 ipinfo->protocol!=IPPROTO_UDP &&
		 ipinfo->protocol!=IPPROTO_SCTP)))
//		 ipinfo->protocol!=IPPROTO_SCTP &&
//		 ipinfo->protocol!=IPPROTO_DCCP)))
		print_error("For port filtering the IP protocol must be "
				"either 6 (tcp), 17 (udp), 33 (dccp) or "
				"132 (sctp)");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	if (ipinfo->bitmask & EBT_IP6_SOURCE) {
		printf("--ip6-src ");
		if (ipinfo->invflags & EBT_IP6_SOURCE)
			printf("! ");
		printf("%s", ebt_ip6_to_numeric(&ipinfo->saddr));
		printf("/%s ", ebt_ip6_to_numeric(&ipinfo->smsk));
	}
	if (ipinfo->bitmask & EBT_IP6_DEST) {
		printf("--ip6-dst ");
		if (ipinfo->invflags & EBT_IP6_DEST)
			printf("! ");
		printf("%s", ebt_ip6_to_numeric(&ipinfo->daddr));
		printf("/%s ", ebt_ip6_to_numeric(&ipinfo->dmsk));
	}
	if (ipinfo->bitmask & EBT_IP6_TCLASS) {
		printf("--ip6-tclass ");
		if (ipinfo->invflags & EBT_IP6_TCLASS)
			printf("! ");
		printf("0x%02X ", ipinfo->tclass);
	}
	if (ipinfo->bitmask & EBT_IP6_PROTO) {
		struct protoent *pe;

		printf("--ip6-proto ");
		if (ipinfo->invflags & EBT_IP6_PROTO)
			printf("! ");
		pe = getprotobynumber(ipinfo->protocol);
		if (pe == NULL) {
			printf("%d ", ipinfo->protocol);
		} else {
			printf("%s ", pe->p_name);
		}
	}
	if (ipinfo->bitmask & EBT_IP6_SPORT) {
		printf("--ip6-sport ");
		if (ipinfo->invflags & EBT_IP6_SPORT)
			printf("! ");
		print_port_range(ipinfo->sport);
	}
	if (ipinfo->bitmask & EBT_IP6_DPORT) {
		printf("--ip6-dport ");
		if (ipinfo->invflags & EBT_IP6_DPORT)
			printf("! ");
		print_port_range(ipinfo->dport);
	}
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_ip6_info *ipinfo1 = (struct ebt_ip6_info *)m1->data;
	struct ebt_ip6_info *ipinfo2 = (struct ebt_ip6_info *)m2->data;

	if (ipinfo1->bitmask != ipinfo2->bitmask)
		return 0;
	if (ipinfo1->invflags != ipinfo2->invflags)
		return 0;
	if (ipinfo1->bitmask & EBT_IP6_SOURCE) {
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->saddr, &ipinfo2->saddr))
			return 0;
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->smsk, &ipinfo2->smsk))
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_DEST) {
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->daddr, &ipinfo2->daddr))
			return 0;
		if (!IN6_ARE_ADDR_EQUAL(&ipinfo1->dmsk, &ipinfo2->dmsk))
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_TCLASS) {
		if (ipinfo1->tclass != ipinfo2->tclass)
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_PROTO) {
		if (ipinfo1->protocol != ipinfo2->protocol)
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_SPORT) {
		if (ipinfo1->sport[0] != ipinfo2->sport[0] ||
		   ipinfo1->sport[1] != ipinfo2->sport[1])
			return 0;
	}
	if (ipinfo1->bitmask & EBT_IP6_DPORT) {
		if (ipinfo1->dport[0] != ipinfo2->dport[0] ||
		   ipinfo1->dport[1] != ipinfo2->dport[1])
			return 0;
	}
	return 1;
}

static struct ebt_u_match ip6_match =
{
	.name		= "ip6",
	.size		= sizeof(struct ebt_ip6_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _init(void) __attribute((constructor));
static void _init(void)
{
   register_match(&ip6_match);
}
