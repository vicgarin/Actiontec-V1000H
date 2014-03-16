/* Shared library add-on to iptables to modify src ip . */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_IP.h>
#include <linux/netfilter/nf_nat.h>


/* Function which prints out usage message. */
static void help(void)
{
	printf(
"SET-IP v%s options:\n"
" --set-ip <ipaddr>[-<ipaddr>]\n"
"				Address to map source to.\n"
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "set-ip", 1, 0, '1' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

/* Parses ip */
static void
parse_ip(const char *arg, struct ipt_ip_target_info *info)
{
        struct in_addr *ip;
        ip = dotted_to_addr(arg);
        if (!ip)
            exit_error(PARAMETER_PROBLEM, "Bad IP address `%s'\n", 
			   arg);
        info->ip = ip->s_addr;
        return;
}

/* Function which parses command options; returns true if it
 *    ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_ip_target_info *ipinfo
		= (struct ipt_ip_target_info *)(*target)->data;

	switch (c) {
	case '1':
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				   "IP target: Cant specify --set-ip twice");
		parse_ip(optarg, ipinfo);
                *flags = 1;
		return 1;

	default:
		return 0;
	}
}

static void final_check(unsigned int flags)
{
   if(!flags)
      exit_error(PARAMETER_PROBLEM, "IP target: Parameter --set-ip is required");
}

static void
print_ip(u_int32_t ip)
{
	const unsigned char *byte;
	byte = (const unsigned char *) &ip;
	printf("%d.%d.%d.%d", 
		byte[0], byte[1], byte[2], byte[3]);
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
   const struct ipt_ip_target_info *ipinfo = (const struct ipt_ip_target_info *)target->data;
   printf("IP SET");
   print_ip(ipinfo->ip);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	struct ipt_ip_target_info *info
		= (struct ipt_ip_target_info *)target->data;
        printf("--set-ip");
        print_ip(info->ip);
}

static
struct iptables_target ip= {
	.next		=  NULL,
	.name		=  "IP",
	.version	=  IPTABLES_VERSION,
	.size		=  IPT_ALIGN(sizeof(struct ipt_ip_target_info)),
	.userspacesize = IPT_ALIGN(sizeof(struct ipt_ip_target_info)),
	.help		=  &help,
	.init		=  &init,
	.parse		=  &parse,
	.final_check =  &final_check,
	.print		=  &print,
	.save		=  &save,
	.extra_opts	=  opts
};

void _init(void)
{
	register_target(&ip);
}
