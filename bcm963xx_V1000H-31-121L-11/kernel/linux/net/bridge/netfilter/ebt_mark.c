/*
 *  ebt_mark
 *
 *	Authors:
 *	Bart De Schuymer <bdschuym@pandora.be>
 *
 *  July, 2002
 *
 */

/* The mark target can be used in any chain,
 * I believe adding a mangle table just for marking is total overkill.
 * Marking a frame doesn't really change anything in the frame anyway.
 */

#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>


static unsigned int
ebt_mark_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int action = info->target & -16;

	if (action == MARK_SET_VALUE)
		skb->mark = info->mark;
	else if (action == MARK_OR_VALUE)
		skb->mark |= info->mark;
	else if (action == MARK_AND_VALUE)
		skb->mark &= info->mark;
	else if (action == MARK_XOR_VALUE)
		skb->mark ^= info->mark;
	else  //brcm -- begin
   {
		skb->vtag = (unsigned short)(info->mark);

      /* if the 8021p priority field (bits 0-3) of skb->vtag is not zero, we need
       * to do p-bit marking.
       */
      if (skb->vtag & 0xf)
      {
         unsigned short TCI = 0;

         /* if this is a vlan frame, we want to re-mark its p-bit with the 8021p
          * priority in skb->vtag.
          * if this is not a vlan frame, we want to add a 8021p tag to it, with
          * vid=0 and p-bit=the 8021p priority in skb->vtag.
          */
	      if ((skb->protocol == __constant_htons(ETH_P_8021Q)))
	      {
   	      struct vlan_hdr *frame = (struct vlan_hdr *)(skb->network_header);

		      TCI = ntohs(frame->h_vlan_TCI);

            /* Since the 8021p priority value in vtag had been incremented by 1,
             * we need to minus 1 from it to get the exact value.
             */
            TCI = (TCI & 0x1fff) | (((skb->vtag & 0xf) - 1) << 13);

		      frame->h_vlan_TCI = htons(TCI);
   	   }
         else
         {
            if ((skb->mac_header - skb->head) < VLAN_HLEN)
            {
               printk("ebt_mark_tg: No headroom for VLAN tag. Marking is not done.\n");
            }
            else
            {
   	         struct vlan_ethhdr *ethHeader;

               skb->protocol = __constant_htons(ETH_P_8021Q);
               skb->mac_header -= VLAN_HLEN;
               skb->network_header -= VLAN_HLEN;
               skb->data -= VLAN_HLEN;
	            skb->len  += VLAN_HLEN;

               /* Move the mac addresses to the beginning of the new header. */
               memmove(skb->mac_header, skb->mac_header + VLAN_HLEN, 2 * ETH_ALEN);

               ethHeader = (struct vlan_ethhdr *)(skb->mac_header);

               ethHeader->h_vlan_proto = __constant_htons(ETH_P_8021Q);

               /* Since the 8021p priority value in vtag had been incremented by 1,
                * we need to minus 1 from it to get the exact value.
                */
               TCI = (TCI & 0x1fff) | (((skb->vtag & 0xf) - 1) << 13);

               ethHeader->h_vlan_TCI = htons(TCI);
            }
         }
         skb->vtag = 0;
      }
   }  // brcm -- end

	return info->target | ~EBT_VERDICT_BITS;
}

static bool ebt_mark_tg_check(const struct xt_tgchk_param *par)
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int tmp;

	tmp = info->target | ~EBT_VERDICT_BITS;
	if (BASE_CHAIN && tmp == EBT_RETURN)
		return false;
	if (tmp < -NUM_STANDARD_TARGETS || tmp >= 0)
		return false;
	tmp = info->target & ~EBT_VERDICT_BITS;
	if (tmp != MARK_SET_VALUE && tmp != MARK_OR_VALUE &&
	    tmp != MARK_AND_VALUE && tmp != MARK_XOR_VALUE &&
       tmp != VTAG_SET_VALUE)    /* brcm */
		return false;
	return true;
}

static struct xt_target ebt_mark_tg_reg __read_mostly = {
	.name		= "mark",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_mark_tg,
	.checkentry	= ebt_mark_tg_check,
	.targetsize	= XT_ALIGN(sizeof(struct ebt_mark_t_info)),
	.me		= THIS_MODULE,
};

static int __init ebt_mark_init(void)
{
	return xt_register_target(&ebt_mark_tg_reg);
}

static void __exit ebt_mark_fini(void)
{
	xt_unregister_target(&ebt_mark_tg_reg);
}

module_init(ebt_mark_init);
module_exit(ebt_mark_fini);
MODULE_DESCRIPTION("Ebtables: Packet mark modification");
MODULE_LICENSE("GPL");
