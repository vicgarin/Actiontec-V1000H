/* Copyright (C) 2009-2011 Broadcom Corporation. */

#ifdef BCM_HAS_NEW_PCI_DMA_MAPPING_ERROR
#define tg3_pci_dma_mapping_error(pdev, mapping)  pci_dma_mapping_error((pdev), (mapping))
#elif defined(BCM_HAS_PCI_DMA_MAPPING_ERROR)
#define tg3_pci_dma_mapping_error(pdev, mapping)  pci_dma_mapping_error((mapping))
#else
#define tg3_pci_dma_mapping_error(pdev, mapping)  0
#endif


#ifdef HAVE_POLL_CONTROLLER
#define CONFIG_NET_POLL_CONTROLLER
#endif

static inline void tg3_5780_class_intx_workaround(struct tg3 *tp)
{
#ifndef BCM_HAS_INTX_MSI_WORKAROUND
	if ((tp->tg3_flags2 & TG3_FLG2_5780_CLASS) &&
	    (tp->tg3_flags2 & TG3_FLG2_USING_MSI))
		tg3_enable_intx(tp->pdev);
#endif
}

#ifdef BCM_HAS_TXQ_TRANS_UPDATE
#define tg3_update_trans_start(dev)
#else
#define tg3_update_trans_start(dev)		((dev)->trans_start = jiffies)
#endif

#ifdef __VMKLNX__

/**
 *      skb_copy_expand -       copy and expand sk_buff
 *      @skb: buffer to copy
 *      @newheadroom: new free bytes at head
 *      @newtailroom: new free bytes at tail
 *      @gfp_mask: allocation priority
 *
 *      Make a copy of both an &sk_buff and its data and while doing so
 *      allocate additional space.
 *
 *      This is used when the caller wishes to modify the data and needs a
 *      private copy of the data to alter as well as more space for new fields.
 *      Returns %NULL on failure or the pointer to the buffer
 *      on success. The returned buffer has a reference count of 1.
 *
 *      You must pass %GFP_ATOMIC as the allocation priority if this function
 *      is called from an interrupt.
 */
struct sk_buff *skb_copy_expand(const struct sk_buff *skb,
                                int newheadroom, int newtailroom,
                                gfp_t gfp_mask)
{
	int rc;
	struct sk_buff *new_skb = skb_copy((struct sk_buff *) skb, gfp_mask);

	if(new_skb == NULL)
		return NULL;

	rc = pskb_expand_head(new_skb, newheadroom, newtailroom, gfp_mask);

	if(rc != 0)
		return NULL;

	return new_skb;
}

void *memmove(void *dest, const void *src, size_t count)
{
	if (dest < src) {
		return memcpy(dest, src, count);
	} else {
		char *p = dest + count;
		const char *s = src + count;
		while (count--)
			*--p = *--s;
	}
	return dest;
}


#endif
