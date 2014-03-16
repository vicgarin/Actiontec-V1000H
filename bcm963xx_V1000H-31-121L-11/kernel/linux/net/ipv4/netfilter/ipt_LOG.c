/*
 * This is a module which is used for logging packets.
 */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/route.h>

#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ipt_LOG.h>
#include <net/netfilter/nf_log.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: IPv4 packet logging to syslog");

/* Use lock to serialize, so printks don't overlap */
static DEFINE_SPINLOCK(log_lock);
#if defined(AEI_LOG_FIREWALL_DROP)
static unsigned char _logstr[896]={0};
#endif

/* One level of recursion won't kill us */
static void dump_packet(const struct nf_loginfo *info,
			const struct sk_buff *skb,
			unsigned int iphoff)
{
	struct iphdr _iph;
	const struct iphdr *ih;
	unsigned int logflags;
#if defined(AEI_LOG_FIREWALL_DROP)
	unsigned char _tmpstr[128]={0};
#endif

	if (info->type == NF_LOG_TYPE_LOG)
		logflags = info->u.log.logflags;
	else
		logflags = NF_LOG_MASK;

	ih = skb_header_pointer(skb, iphoff, sizeof(_iph), &_iph);
	if (ih == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"TRUNCATED");
#else
		printk("TRUNCATED");
#endif
		return;
	}

	/* Important fields:
	 * TOS, len, DF/MF, fragment offset, TTL, src, dst, options. */
	/* Max length: 40 "SRC=255.255.255.255 DST=255.255.255.255 " */
#if defined(AEI_LOG_FIREWALL_DROP)
	snprintf(_tmpstr, sizeof(_tmpstr), "SRC=%pI4 DST=%pI4 ",
	       &ih->saddr, &ih->daddr);
	strcat(_logstr, _tmpstr);
#else
	printk("SRC=%pI4 DST=%pI4 ",
	       &ih->saddr, &ih->daddr);
#endif

	/* Max length: 46 "LEN=65535 TOS=0xFF PREC=0xFF TTL=255 ID=65535 " */
#if defined(AEI_LOG_FIREWALL_DROP)
	snprintf(_tmpstr, sizeof(_tmpstr), "LEN=%u TOS=0x%02X PREC=0x%02X TTL=%u ID=%u ",
	       ntohs(ih->tot_len), ih->tos & IPTOS_TOS_MASK,
	       ih->tos & IPTOS_PREC_MASK, ih->ttl, ntohs(ih->id));
	strcat(_logstr, _tmpstr);
#else
	printk("LEN=%u TOS=0x%02X PREC=0x%02X TTL=%u ID=%u ",
	       ntohs(ih->tot_len), ih->tos & IPTOS_TOS_MASK,
	       ih->tos & IPTOS_PREC_MASK, ih->ttl, ntohs(ih->id));
#endif

	/* Max length: 6 "CE DF MF " */
	if (ntohs(ih->frag_off) & IP_CE)
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"CE ");
#else
		printk("CE ");
#endif
	if (ntohs(ih->frag_off) & IP_DF)
#if defined(AEI_LOG_FIREWALL_DROP)
				strcat(_logstr,"DF ");
#else
		printk("DF ");
#endif
	if (ntohs(ih->frag_off) & IP_MF)
#if defined(AEI_LOG_FIREWALL_DROP)
				strcat(_logstr,"MF ");
#else
		printk("MF ");
#endif

	/* Max length: 11 "FRAG:65535 " */
	if (ntohs(ih->frag_off) & IP_OFFSET)
#if defined(AEI_LOG_FIREWALL_DROP)
	{
		snprintf(_tmpstr, sizeof(_tmpstr), "FRAG:%u ", ntohs(ih->frag_off) & IP_OFFSET);
		strcat(_logstr, _tmpstr);	
	}
#else
		printk("FRAG:%u ", ntohs(ih->frag_off) & IP_OFFSET);
#endif

	if ((logflags & IPT_LOG_IPOPT)
	    && ih->ihl * 4 > sizeof(struct iphdr)) {
		const unsigned char *op;
		unsigned char _opt[4 * 15 - sizeof(struct iphdr)];
		unsigned int i, optsize;

		optsize = ih->ihl * 4 - sizeof(struct iphdr);
		op = skb_header_pointer(skb, iphoff+sizeof(_iph),
					optsize, _opt);
		if (op == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"TRUNCATED");
#else
			printk("TRUNCATED");
#endif
			return;
		}

		/* Max length: 127 "OPT (" 15*4*2chars ") " */
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"OPT (");
#else
		printk("OPT (");
#endif
		for (i = 0; i < optsize; i++)
#if defined(AEI_LOG_FIREWALL_DROP)
		{
			snprintf(_tmpstr, sizeof(_tmpstr), "%02X", op[i]);
			strcat(_logstr, _tmpstr);	
		}
#else
			printk("%02X", op[i]);
#endif
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,") ");
#else
		printk(") ");
#endif
	}

	switch (ih->protocol) {
	case IPPROTO_TCP: {
		struct tcphdr _tcph;
		const struct tcphdr *th;

		/* Max length: 10 "PROTO=TCP " */
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"PROTO=TCP ");
#else
		printk("PROTO=TCP ");
#endif

		if (ntohs(ih->frag_off) & IP_OFFSET)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		th = skb_header_pointer(skb, iphoff + ih->ihl * 4,
					sizeof(_tcph), &_tcph);
		if (th == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
			strcat(_logstr, _tmpstr);	
#else
			printk("INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
#endif
			break;
		}

		/* Max length: 20 "SPT=65535 DPT=65535 " */
#if defined(AEI_LOG_FIREWALL_DROP)
					snprintf(_tmpstr, sizeof(_tmpstr), "SPT=%u DPT=%u ",
		       ntohs(th->source), ntohs(th->dest));
					strcat(_logstr, _tmpstr);	
#else
		printk("SPT=%u DPT=%u ",
		       ntohs(th->source), ntohs(th->dest));
#endif
		/* Max length: 30 "SEQ=4294967295 ACK=4294967295 " */
		if (logflags & IPT_LOG_TCPSEQ)
#if defined(AEI_LOG_FIREWALL_DROP)
		{
			snprintf(_tmpstr, sizeof(_tmpstr), "SEQ=%u ACK=%u ",
			       ntohl(th->seq), ntohl(th->ack_seq));
			strcat(_logstr, _tmpstr);	
		}
#else
			printk("SEQ=%u ACK=%u ",
			       ntohl(th->seq), ntohl(th->ack_seq));
#endif
		/* Max length: 13 "WINDOW=65535 " */
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "WINDOW=%u ", ntohs(th->window));
		strcat(_logstr, _tmpstr);	
#else
		printk("WINDOW=%u ", ntohs(th->window));
#endif
		/* Max length: 9 "RES=0x3F " */
#if defined(AEI_LOG_FIREWALL_DROP)
				snprintf(_tmpstr, sizeof(_tmpstr), "RES=0x%02x ", (u8)(ntohl(tcp_flag_word(th) & TCP_RESERVED_BITS) >> 22));
		strcat(_logstr, _tmpstr);	
#else
		printk("RES=0x%02x ", (u8)(ntohl(tcp_flag_word(th) & TCP_RESERVED_BITS) >> 22));
#endif
		/* Max length: 32 "CWR ECE URG ACK PSH RST SYN FIN " */
		if (th->cwr)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"CWR ");
#else
			printk("CWR ");
#endif
		if (th->ece)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"ECE ");
#else
			printk("ECE ");
#endif
		if (th->urg)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"URG ");
#else
			printk("URG ");
#endif
		if (th->ack)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"ACK ");
#else
			printk("ACK ");
#endif
		if (th->psh)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"PSH ");
#else
			printk("PSH ");
#endif
		if (th->rst)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"RST ");
#else
			printk("RST ");
#endif
		if (th->syn)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"SYN ");
#else
			printk("SYN ");
#endif
		if (th->fin)
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"FIN ");
#else
			printk("FIN ");
#endif
		/* Max length: 11 "URGP=65535 " */
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "URGP=%u ", ntohs(th->urg_ptr));
		strcat(_logstr, _tmpstr);	
#else
		printk("URGP=%u ", ntohs(th->urg_ptr));
#endif

		if ((logflags & IPT_LOG_TCPOPT)
		    && th->doff * 4 > sizeof(struct tcphdr)) {
			unsigned char _opt[4 * 15 - sizeof(struct tcphdr)];
			const unsigned char *op;
			unsigned int i, optsize;

			optsize = th->doff * 4 - sizeof(struct tcphdr);
			op = skb_header_pointer(skb,
						iphoff+ih->ihl*4+sizeof(_tcph),
						optsize, _opt);
			if (op == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
				strcat(_logstr,"TRUNCATED");
#else
				printk("TRUNCATED");
#endif
				return;
			}

			/* Max length: 127 "OPT (" 15*4*2chars ") " */
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"OPT (");
#else
			printk("OPT (");
#endif
			for (i = 0; i < optsize; i++)
#if defined(AEI_LOG_FIREWALL_DROP)
			{
				snprintf(_tmpstr, sizeof(_tmpstr), "%02X", op[i]);
				strcat(_logstr, _tmpstr);	
			}
#else
				printk("%02X", op[i]);
#endif
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,") ");
#else
			printk(") ");
#endif
		}
		break;
	}
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE: {
		struct udphdr _udph;
		const struct udphdr *uh;

		if (ih->protocol == IPPROTO_UDP)
			/* Max length: 10 "PROTO=UDP "     */
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"PROTO=UDP " );
#else
			printk("PROTO=UDP " );
#endif
		else	/* Max length: 14 "PROTO=UDPLITE " */
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr,"PROTO=UDPLITE ");
#else
			printk("PROTO=UDPLITE ");
#endif

		if (ntohs(ih->frag_off) & IP_OFFSET)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		uh = skb_header_pointer(skb, iphoff+ih->ihl*4,
					sizeof(_udph), &_udph);
		if (uh == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
			strcat(_logstr, _tmpstr);	
#else
			printk("INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
#endif
			break;
		}

		/* Max length: 20 "SPT=65535 DPT=65535 " */
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "SPT=%u DPT=%u LEN=%u ",
		       ntohs(uh->source), ntohs(uh->dest),
		       ntohs(uh->len));
		strcat(_logstr, _tmpstr);	
#else
		printk("SPT=%u DPT=%u LEN=%u ",
		       ntohs(uh->source), ntohs(uh->dest),
		       ntohs(uh->len));
#endif
		break;
	}
	case IPPROTO_ICMP: {
		struct icmphdr _icmph;
		const struct icmphdr *ich;
		static const size_t required_len[NR_ICMP_TYPES+1]
			= { [ICMP_ECHOREPLY] = 4,
			    [ICMP_DEST_UNREACH]
			    = 8 + sizeof(struct iphdr),
			    [ICMP_SOURCE_QUENCH]
			    = 8 + sizeof(struct iphdr),
			    [ICMP_REDIRECT]
			    = 8 + sizeof(struct iphdr),
			    [ICMP_ECHO] = 4,
			    [ICMP_TIME_EXCEEDED]
			    = 8 + sizeof(struct iphdr),
			    [ICMP_PARAMETERPROB]
			    = 8 + sizeof(struct iphdr),
			    [ICMP_TIMESTAMP] = 20,
			    [ICMP_TIMESTAMPREPLY] = 20,
			    [ICMP_ADDRESS] = 12,
			    [ICMP_ADDRESSREPLY] = 12 };

		/* Max length: 11 "PROTO=ICMP " */
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"PROTO=ICMP ");
#else
		printk("PROTO=ICMP ");
#endif

		if (ntohs(ih->frag_off) & IP_OFFSET)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		ich = skb_header_pointer(skb, iphoff + ih->ihl * 4,
					 sizeof(_icmph), &_icmph);
		if (ich == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
			strcat(_logstr, _tmpstr);	
#else
			printk("INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
#endif
			break;
		}

		/* Max length: 18 "TYPE=255 CODE=255 " */
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "TYPE=%u CODE=%u ", ich->type, ich->code);
		strcat(_logstr, _tmpstr);	
#else
		printk("TYPE=%u CODE=%u ", ich->type, ich->code);
#endif

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		if (ich->type <= NR_ICMP_TYPES
		    && required_len[ich->type]
		    && skb->len-iphoff-ih->ihl*4 < required_len[ich->type]) {
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
			strcat(_logstr, _tmpstr);	
#else
			printk("INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
#endif
			break;
		}

		switch (ich->type) {
		case ICMP_ECHOREPLY:
		case ICMP_ECHO:
			/* Max length: 19 "ID=65535 SEQ=65535 " */
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "ID=%u SEQ=%u ",
			       ntohs(ich->un.echo.id),
			       ntohs(ich->un.echo.sequence));
			strcat(_logstr, _tmpstr);	
#else
			printk("ID=%u SEQ=%u ",
			       ntohs(ich->un.echo.id),
			       ntohs(ich->un.echo.sequence));
#endif
			break;

		case ICMP_PARAMETERPROB:
			/* Max length: 14 "PARAMETER=255 " */
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "PARAMETER=%u ",
			       ntohl(ich->un.gateway) >> 24);
			strcat(_logstr, _tmpstr);	
#else
			printk("PARAMETER=%u ",
			       ntohl(ich->un.gateway) >> 24);
#endif
			break;
		case ICMP_REDIRECT:
			/* Max length: 24 "GATEWAY=255.255.255.255 " */
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "GATEWAY=%pI4 ", &ich->un.gateway);
			strcat(_logstr, _tmpstr);	
#else
			printk("GATEWAY=%pI4 ", &ich->un.gateway);
#endif
			/* Fall through */
		case ICMP_DEST_UNREACH:
		case ICMP_SOURCE_QUENCH:
		case ICMP_TIME_EXCEEDED:
			/* Max length: 3+maxlen */
			if (!iphoff) { /* Only recurse once. */
#if defined(AEI_LOG_FIREWALL_DROP)
				strcat(_logstr,"[");
#else
				printk("[");
#endif
				dump_packet(info, skb,
					    iphoff + ih->ihl*4+sizeof(_icmph));
#if defined(AEI_LOG_FIREWALL_DROP)
				strcat(_logstr,"] ");
#else
				printk("] ");
#endif
			}

			/* Max length: 10 "MTU=65535 " */
			if (ich->type == ICMP_DEST_UNREACH
			    && ich->code == ICMP_FRAG_NEEDED)
#if defined(AEI_LOG_FIREWALL_DROP)
			{
				snprintf(_tmpstr, sizeof(_tmpstr), "MTU=%u ", ntohs(ich->un.frag.mtu));
				strcat(_logstr, _tmpstr);	
			}
#else
				printk("MTU=%u ", ntohs(ich->un.frag.mtu));
#endif
		}
		break;
	}
	/* Max Length */
	case IPPROTO_AH: {
		struct ip_auth_hdr _ahdr;
		const struct ip_auth_hdr *ah;

		if (ntohs(ih->frag_off) & IP_OFFSET)
			break;

		/* Max length: 9 "PROTO=AH " */
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"PROTO=AH ");
#else
		printk("PROTO=AH ");
#endif

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		ah = skb_header_pointer(skb, iphoff+ih->ihl*4,
					sizeof(_ahdr), &_ahdr);
		if (ah == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
			strcat(_logstr, _tmpstr);	
#else
			printk("INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
#endif
			break;
		}

		/* Length: 15 "SPI=0xF1234567 " */
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "SPI=0x%x ", ntohl(ah->spi));
		strcat(_logstr, _tmpstr);	
#else
		printk("SPI=0x%x ", ntohl(ah->spi));
#endif
		break;
	}
	case IPPROTO_ESP: {
		struct ip_esp_hdr _esph;
		const struct ip_esp_hdr *eh;

		/* Max length: 10 "PROTO=ESP " */
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr,"PROTO=ESP ");
#else
		printk("PROTO=ESP ");
#endif

		if (ntohs(ih->frag_off) & IP_OFFSET)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		eh = skb_header_pointer(skb, iphoff+ih->ihl*4,
					sizeof(_esph), &_esph);
		if (eh == NULL) {
#if defined(AEI_LOG_FIREWALL_DROP)
			snprintf(_tmpstr, sizeof(_tmpstr), "INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
			strcat(_logstr, _tmpstr);	
#else
			printk("INCOMPLETE [%u bytes] ",
			       skb->len - iphoff - ih->ihl*4);
#endif
			break;
		}

		/* Length: 15 "SPI=0xF1234567 " */
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "SPI=0x%x ", ntohl(eh->spi));
		strcat(_logstr, _tmpstr);	
#else
		printk("SPI=0x%x ", ntohl(eh->spi));
#endif
		break;
	}
	/* Max length: 10 "PROTO 255 " */
	default:
#if defined(AEI_LOG_FIREWALL_DROP)
		snprintf(_tmpstr, sizeof(_tmpstr), "PROTO=%u ", ih->protocol);
		strcat(_logstr, _tmpstr);	
#else
		printk("PROTO=%u ", ih->protocol);
#endif
	}

	/* Max length: 15 "UID=4294967295 " */
	if ((logflags & IPT_LOG_UID) && !iphoff && skb->sk) {
		read_lock_bh(&skb->sk->sk_callback_lock);
		if (skb->sk->sk_socket && skb->sk->sk_socket->file)
#if defined(AEI_LOG_FIREWALL_DROP)
		{
			snprintf(_tmpstr, sizeof(_tmpstr), "UID=%u GID=%u ",
				skb->sk->sk_socket->file->f_cred->fsuid,
				skb->sk->sk_socket->file->f_cred->fsgid);
			strcat(_logstr, _tmpstr);	
		}
#else
			printk("UID=%u GID=%u ",
				skb->sk->sk_socket->file->f_cred->fsuid,
				skb->sk->sk_socket->file->f_cred->fsgid);
#endif
		read_unlock_bh(&skb->sk->sk_callback_lock);
	}

	/* Max length: 16 "MARK=0xFFFFFFFF " */
	if (!iphoff && skb->mark)
#if defined(AEI_LOG_FIREWALL_DROP)
	{
		snprintf(_tmpstr, sizeof(_tmpstr), "MARK=0x%x ", skb->mark);
		strcat(_logstr, _tmpstr);	
	}
#else
		printk("MARK=0x%x ", skb->mark);
#endif

	/* Proto    Max log string length */
	/* IP:      40+46+6+11+127 = 230 */
	/* TCP:     10+max(25,20+30+13+9+32+11+127) = 252 */
	/* UDP:     10+max(25,20) = 35 */
	/* UDPLITE: 14+max(25,20) = 39 */
	/* ICMP:    11+max(25, 18+25+max(19,14,24+3+n+10,3+n+10)) = 91+n */
	/* ESP:     10+max(25)+15 = 50 */
	/* AH:      9+max(25)+15 = 49 */
	/* unknown: 10 */

	/* (ICMP allows recursion one level deep) */
	/* maxlen =  IP + ICMP +  IP + max(TCP,UDP,ICMP,unknown) */
	/* maxlen = 230+   91  + 230 + 252 = 803 */
}

static struct nf_loginfo default_loginfo = {
	.type	= NF_LOG_TYPE_LOG,
	.u = {
		.log = {
			.level    = 0,
			.logflags = NF_LOG_MASK,
		},
	},
};

static void
ipt_log_packet(u_int8_t pf,
	       unsigned int hooknum,
	       const struct sk_buff *skb,
	       const struct net_device *in,
	       const struct net_device *out,
	       const struct nf_loginfo *loginfo,
	       const char *prefix)
{
#if defined(AEI_LOG_FIREWALL_DROP)
	unsigned char _tmpstr[128]={0};
#endif
	if (!loginfo)
		loginfo = &default_loginfo;

	spin_lock_bh(&log_lock);
#if defined(AEI_LOG_FIREWALL_DROP)
	snprintf(_logstr,sizeof(_logstr),"<%d>%sIN=%s OUT=%s ", loginfo->u.log.level,
	       prefix,
	       in ? in->name : "",
	       out ? out->name : "");
#else
	printk("<%d>%sIN=%s OUT=%s ", loginfo->u.log.level,
	       prefix,
	       in ? in->name : "",
	       out ? out->name : "");
#endif
#ifdef CONFIG_BRIDGE_NETFILTER
	if (skb->nf_bridge) {
		const struct net_device *physindev;
		const struct net_device *physoutdev;

		physindev = skb->nf_bridge->physindev;
		if (physindev && in != physindev)
#if defined(AEI_LOG_FIREWALL_DROP)
		{
			strcat(_logstr, "PHYSIN=");
			strcat(_logstr, physindev->name);
		}
#else
			printk("PHYSIN=%s ", physindev->name);
#endif
		physoutdev = skb->nf_bridge->physoutdev;
		if (physoutdev && out != physoutdev)
#if defined(AEI_LOG_FIREWALL_DROP)
		{
			strcat(_logstr,"PHYSOUT=");
			strcat(_logstr, physoutdev->name);
		}
#else
			printk("PHYSOUT=%s ", physoutdev->name);
#endif
	}
#endif

	if (in && !out) {
		/* MAC logging for input chain only. */
#if defined(AEI_LOG_FIREWALL_DROP)
		strcat(_logstr, "MAC=");
#else
		printk("MAC=");
#endif
		if (skb->dev && skb->dev->hard_header_len
		    && skb->mac_header != skb->network_header) {
			int i;
			const unsigned char *p = skb_mac_header(skb);
			for (i = 0; i < skb->dev->hard_header_len; i++,p++)
#if defined(AEI_LOG_FIREWALL_DROP)
			{
				snprintf(_tmpstr,sizeof(_tmpstr),"%02x%c", *p,
				       i==skb->dev->hard_header_len - 1
				       ? ' ':':');
				strcat(_logstr, _tmpstr);
			}
#else
				printk("%02x%c", *p,
				       i==skb->dev->hard_header_len - 1
				       ? ' ':':');
#endif
		} else
#if defined(AEI_LOG_FIREWALL_DROP)
			strcat(_logstr, " ");
#else
			printk(" ");
#endif
	}

	dump_packet(loginfo, skb, 0);
#if defined(AEI_LOG_FIREWALL_DROP)
	printk("%s\n",_logstr);
#else
	printk("\n");
#endif
	spin_unlock_bh(&log_lock);
}

static unsigned int
log_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ipt_log_info *loginfo = par->targinfo;
	struct nf_loginfo li;

	li.type = NF_LOG_TYPE_LOG;
	li.u.log.level = loginfo->level;
	li.u.log.logflags = loginfo->logflags;

	ipt_log_packet(NFPROTO_IPV4, par->hooknum, skb, par->in, par->out, &li,
		       loginfo->prefix);
	return XT_CONTINUE;
}

static bool log_tg_check(const struct xt_tgchk_param *par)
{
	const struct ipt_log_info *loginfo = par->targinfo;

	if (loginfo->level >= 8) {
		pr_debug("LOG: level %u >= 8\n", loginfo->level);
		return false;
	}
	if (loginfo->prefix[sizeof(loginfo->prefix)-1] != '\0') {
		pr_debug("LOG: prefix term %i\n",
			 loginfo->prefix[sizeof(loginfo->prefix)-1]);
		return false;
	}
	return true;
}

static struct xt_target log_tg_reg __read_mostly = {
	.name		= "LOG",
	.family		= NFPROTO_IPV4,
	.target		= log_tg,
	.targetsize	= sizeof(struct ipt_log_info),
	.checkentry	= log_tg_check,
	.me		= THIS_MODULE,
};

static struct nf_logger ipt_log_logger __read_mostly = {
	.name		= "ipt_LOG",
	.logfn		= &ipt_log_packet,
	.me		= THIS_MODULE,
};

static int __init log_tg_init(void)
{
	int ret;

	ret = xt_register_target(&log_tg_reg);
	if (ret < 0)
		return ret;
	nf_log_register(NFPROTO_IPV4, &ipt_log_logger);
	return 0;
}

static void __exit log_tg_exit(void)
{
	nf_log_unregister(&ipt_log_logger);
	xt_unregister_target(&log_tg_reg);
}

module_init(log_tg_init);
module_exit(log_tg_exit);
