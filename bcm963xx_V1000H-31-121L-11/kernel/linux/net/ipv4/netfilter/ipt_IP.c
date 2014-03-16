#include <linux/types.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ipt_IP.h>
#include <net/netfilter/nf_nat_rule.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Richard<rdiao@actiontec.com>");
MODULE_DESCRIPTION("iptables target SNAT for advanced DMZ enabled");

static bool checkentry(const struct xt_tgchk_param *par)
{
	//const struct ipt_ip_target_info *info = par->targinfo;
	return true;
}

static unsigned int
target(struct sk_buff *skb, const struct xt_target_param *par)
{
        const struct ipt_ip_target_info *info = par->targinfo;
	//struct nf_conn *ct;
	//enum ip_conntrack_info ctinfo;
        struct iphdr     *iph = ip_hdr(skb);
        u_int32_t diffs[2];

        if (par->hooknum != NF_INET_POST_ROUTING)
		return NF_ACCEPT;

	//ct = nf_ct_get(skb, &ctinfo);
        //if ( ct && (ctinfo == IP_CT_NEW) )
        //{
            diffs[0] = htonl(iph->saddr) ^ 0xFFFFFFFF;
            iph->saddr = info->ip;
            diffs[1] = htonl(iph->saddr);
            iph->check = csum_fold(csum_partial((char *)diffs, sizeof(diffs), iph->check ^ 0xFFFFFFFF));
            //skb->nfcache |= NFC_ALTERD;
        //}
        return IPT_CONTINUE;
}

static struct xt_target ipt_ip_reg __read_mostly = {
	.name		= "IP",
	.family		= NFPROTO_IPV4,
	.target		= target,
	.targetsize	= sizeof(struct ipt_ip_target_info),
	.table		= "mangle",
	.hooks		= (1 << NF_INET_POST_ROUTING),
	.checkentry	= checkentry,
	.me		= THIS_MODULE,
};

static int __init init(void)
{
	return xt_register_target(&ipt_ip_reg);
}

static void __exit fini(void)
{
	xt_unregister_target(&ipt_ip_reg);
}

module_init(init);
module_exit(fini);

