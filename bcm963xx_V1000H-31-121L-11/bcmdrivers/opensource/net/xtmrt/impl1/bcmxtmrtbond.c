/*
<:copyright-gpl 
 Copyright 2007 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/**************************************************************************
 * File Name  : bcmxtmrtbond.c
 *
 * Description: This file implements BCM6368 ATM/PTM bonding network device driver
 *              runtime processing - sending and receiving data.
 *              Current implementation pertains to PTM bonding. Broadcom ITU G.998.2 
 *              solution.
 ***************************************************************************/


/* Includes. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/blog.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmnet.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include <asm/io.h>
#include <asm/r4kcache.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"

#if defined(PERF_MON_BONDING_US) || defined(PERF_MON_BONDING_DS)
#include <asm/pmonapi.h>
#endif

#ifdef PERF_MON_BONDING_US
#define PMON_US_LOG(x)        pmon_log(x)
#define PMON_US_CLR(x)        pmon_clr(x)
#define PMON_US_BGN           pmon_bgn()
#define PMON_US_END(x)        pmon_end(x)
#define PMON_US_REG(x,y)      pmon_reg(x,y)
#else
#define PMON_US_LOG(x)        
#define PMON_US_CLR(x)        
#define PMON_US_BGN
#define PMON_US_END(x)        
#define PMON_US_REG(x,y)
#endif

#ifdef PERF_MON_BONDING_DS
#define PMON_DS_LOG(x)        pmon_log(x)
#define PMON_DS_CLR(x)        pmon_clr(x)
#define PMON_DS_BGN           pmon_bgn()
#define PMON_DS_END(x)        pmon_end(x)
#define PMON_DS_REG(x,y)      pmon_reg(x,y)
#else
#define PMON_DS_LOG(x)        
#define PMON_DS_CLR(x)        
#define PMON_DS_BGN
#define PMON_DS_END(x)        
#define PMON_DS_REG(x,y)
#endif


#ifdef PTM_BONDING_DEBUG
#define XTMRT_BOND_LOG(level, format, args...)					\
	do {                                            \
		if (level < g_GlobalInfo.ptmBondInfo.logLevel) {				\
			printk(format, ## args);				\
		}											\
	} while (0)

#define XTMRT_BOND_LOG3_FKB(fkb, dir)						\
	if (3 < g_GlobalInfo.ptmBondInfo.logLevel)						\
		log_fkb(fkb, dir)

#define XTMRT_BOND_LOG4_FKB(fkb, dir)						\
	if (4 < g_GlobalInfo.ptmBondInfo.logLevel)						\
		log_fkb(fkb, dir)

static void log_fkb(FkBuff_t *fkb, char *dir)
{
	int i, j;
	for (i=0; i<fkb->len-1; ) {
		printk("\n%s %04X: ", dir, i);
		for (j=0; j<16 && i<fkb->len-1; j+=2, i+=2)
			printk("%02X%02X ", fkb->data[i], fkb->data[i+1]);
	}
	if (i==fkb->len-1)
		printk("%02X ", fkb->data[i]);
	printk("\n\n");
}
#else
#define XTMRT_BOND_LOG(level, format, args...) 
#define XTMRT_BOND_LOG3_FKB(fkb, dir)
#define XTMRT_BOND_LOG4_FKB(fkb, dir)
#endif

#define XTMRT_BOND_LOG0(format, args...) XTMRT_BOND_LOG(0, format, ## args)
#define XTMRT_BOND_LOG1(format, args...) XTMRT_BOND_LOG(1, format, ## args)
#define XTMRT_BOND_LOG2(format, args...) XTMRT_BOND_LOG(2, format, ## args)
#define XTMRT_BOND_LOG3(format, args...) XTMRT_BOND_LOG(3, format, ## args)

static inline int getPtmFragmentLen (int len) ;
static int constructPtmBondHdr (PBCMXTMRT_DEV_CONTEXT pDevCtx, int len) ;
static inline int bcmxtmrt_ptmbond_get_port (PBCMXTMRT_DEV_CONTEXT pDevCtx, UINT16 bufStatus) ;
static int bcmxtmrt_ptmbond_process_rx_fragment (XtmRtPtmBondInfo *pBondInfo,
                                                 XtmRtPtmBondRxQInfo *inBuf) ;
static int bcmxtmrt_ptmbond_rx_send_expected_fragment (XtmRtPtmBondInfo *pBondInfo,
                                                       XtmRtPtmBondRxQInfo *inBuf, int seq_nr,
                                                       int rxFlags) ;
static int bcmxtmrt_ptmbond_rx_send (XtmRtPtmBondInfo *pBondInfo, XtmRtPtmBondRxQInfo *inBuf,
                                     int flags) ;
static void bcmxtmrt_ptmbond_rx_resync (XtmRtPtmBondInfo *pBondInfo, int type, int port) ;
static int bcmxtmrt_ptmbond_rx_send_waiting_fragments (XtmRtPtmBondInfo *pBondInfo, int start) ;
static inline void bcmxtmrt_ptmbond_del_link (XtmRtPtmBondInfo *pBondInfo, int port) ;
static void bond_fkb_free(FkBuff_t *fkb, int flushLen) ;

/***************************************************************************
 * Function Name: getPtmFragmentLen
 * Description  : Compute next fragment length
 * Returns      : fragment length.
 ***************************************************************************/
static inline int getPtmFragmentLen (int len)
{
	int fragmentLen ;
	int leftOver ;

	if (len <= XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE) {
		fragmentLen = len ;
	}
	else {
		/* send as much as possible unless we don't have
		   enough data left anymore for a full fragment */
		fragmentLen = XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE ;
		leftOver = len-fragmentLen ;
		if (leftOver < XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE) {
			fragmentLen -= (XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE + leftOver) ;
			fragmentLen &= ~0x3 ; /* make it a multiple of 4 bytes */
		}
	}
	return fragmentLen ;
}

/***************************************************************************
 * Function Name: constructPtmBondHdr
 * Description  : Calculates the PTM bonding hdr information and fills it up
 *                in the global buffer to be used for the packet.
 * Returns      : NoofFragments.
 ***************************************************************************/
static int constructPtmBondHdr (PBCMXTMRT_DEV_CONTEXT pDevCtx, int len)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
   XtmRtPtmTxBondHeader  *pPtmHeader  = NULL ;
   int                   fragNo, fragLen ;
   UINT32                portDataMask = pDevCtx->ulPortDataMask ;
   static unsigned int   portCount = 0 ;

   fragNo = 0 ;
   len += ETH_FCS_LEN ;    /* Original Packet Len + 4 bytes of Eth FCS Len */

   while (len != 0) {
      fragLen = getPtmFragmentLen (len) ;
      len -= fragLen ;
      pPtmHeader = &pGi->ptmBondHdr [fragNo] ;
      //pPtmHeader->sVal.FragSize = fragLen ;
      pPtmHeader->sVal.FragSize = fragLen + XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE ;
      if (((portDataMask >> portCount) & 0x1) != 0)   /* Is bit set */
         pPtmHeader->sVal.portSel = portCount ;
      else
         pPtmHeader->sVal.portSel = MAX_BOND_PORTS-portCount-1 ;
      portCount = (portCount + 1) & (MAX_BOND_PORTS-1) ;
      pPtmHeader->sVal.PktEop = XTMRT_PTM_BOND_HDR_NON_EOP ;
      fragNo++ ;
   }

   pPtmHeader->sVal.PktEop = XTMRT_PTM_BOND_FRAG_HDR_EOP ;

   /* Oscillate port count for fragment scheduling */
   if (!(fragNo & 0x1))
      portCount = (portCount + 1) & (MAX_BOND_PORTS-1) ;

   return (fragNo) ;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_add_hdr
 * Description  : Adds the PTM Bonding Tx header to a WAN packet before transmitting
 *                it.
 * Returns      : None.
 ***************************************************************************/
void bcmxtmrt_ptmbond_add_hdr (PBCMXTMRT_DEV_CONTEXT pDevCtx, pNBuff_t *ppNBuff, 
                               struct sk_buff **ppNBuffSkb, UINT8 **ppData, 
                               int *pLen)
{
   int                   frags ;
   XtmRtPtmTxBondHeader  *pPtmBondHdr ;
   int                   headroom ;
   int                   len ;
   int                   minheadroom ;
   PBCMXTMRT_GLOBAL_INFO pGi ;
   XtmRtPtmBondInfo      *pBondInfo ;
   XtmRtPtmTxBondHeader  *pPtmSrcBondHdr ;

   PMON_US_BGN ;

   minheadroom  = sizeof (XtmRtPtmTxBondHeader) * XTMRT_PTM_BOND_MAX_FRAG_PER_PKT ;
   pGi = &g_GlobalInfo ;
   pBondInfo = &pGi->ptmBondInfo ;
   pPtmSrcBondHdr = &pGi->ptmBondHdr[0] ;

   PMON_US_LOG(1) ;

   len = *pLen ;

   if ( *ppNBuffSkb == NULL) {

      struct fkbuff *fkb = PNBUFF_2_FKBUFF(*ppNBuff);
      headroom = fkb_headroom (fkb) ;

      if (headroom >= minheadroom) {
         frags = constructPtmBondHdr (pDevCtx, len) ;

         PMON_US_LOG(2) ;

         *ppData = fkb_push (fkb, minheadroom) ;
         *pLen   = len + minheadroom ;
         pPtmBondHdr = (XtmRtPtmTxBondHeader *) *ppData ;
         PMON_US_LOG(3) ;
         u16cpy (pPtmBondHdr, pPtmSrcBondHdr, sizeof (XtmRtPtmTxBondHeader) * frags) ;
         PMON_US_LOG(4) ;
      }
      else
         printk(CARDNAME "bcmxtmrt_xmit: FKB not enough headroom.\n") ;
   }
   else {
      struct sk_buff *skb = *ppNBuffSkb ;
      headroom = skb_headroom (skb) ;

      if (headroom < minheadroom) {
         struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom) ;
         XTMRT_BOND_LOG2 ("bcmxtmrt: Warning!!, headroom (%d) is less than min headroom (%d) \n",
               headroom, minheadroom) ;
         dev_kfree_skb_any(skb);
         if (skb2 == NULL) {
            printk (CARDNAME ": Fatal!!, NULL Skb \n") ;
            skb = NULL ;
         }
         else
            skb = skb2 ;
      }

      if( skb ) {
         frags = constructPtmBondHdr (pDevCtx, len) ;

         PMON_US_LOG(2) ;

         *ppData = skb_push (skb, minheadroom) ;
         *pLen   = len + minheadroom ;
         pPtmBondHdr = (XtmRtPtmTxBondHeader *) *ppData ;
         PMON_US_LOG(3) ;
         u16cpy (pPtmBondHdr, pPtmSrcBondHdr, sizeof (XtmRtPtmTxBondHeader) * frags) ;
         PMON_US_LOG(4) ;
      }

      *ppNBuffSkb = skb ;
      *ppNBuff = SKBUFF_2_PNBUFF(skb) ;
   }

   PMON_US_LOG(5) ;
   PMON_US_END(5) ;

} /* bcmxtmrt_ptmbond_add_hdr */


/***************************************************************** */
/* ================= Re-Order And Forward engine ================= */
/***************************************************************** */

void *bond_memcpy (void * dest, void const * src, size_t cnt)
{
   if ((!((unsigned int) src & 0x03)) && (!((unsigned int) dest & 0x03))) {
      /* Word Alignment Copy */

      long * pldest = (long *) dest;
      long const * plSrc = (long const *) src;
      long lval ;

      while (cnt >= 4) {
         lval = *plSrc ;
         plSrc++ ;
         cnt -= 4;
         *pldest = lval ;
         pldest++ ;
      }

      {
         char * pcdest = (char *) pldest;
         char const * pcSrc = (char const *) plSrc;
         char cval ;

         while (cnt) {
            cval = *pcSrc ;
            pcSrc++ ;
            cnt-- ;
            *pcdest = cval ;
            pcdest++ ;
         }
      }
   }
   else if ((!((unsigned int) src & 0x01)) && (!((unsigned int) dest & 0x01))) {
      /* Short Word Alignment Copy */

      short * psdest = (short *) dest;
      short const * psSrc = (short const *) src;
      long sval ;

      while (cnt >= 2) {
         sval = *psSrc ;
         psSrc++ ;
         cnt -=2 ;
         *psdest = sval ;
         psdest++ ;
      }

      {
         char * pcdest = (char *) psdest;
         char const * pcSrc = (char const *) psSrc;
         char cval ;

         while (cnt) {
            cval = *pcSrc ;
            pcSrc++ ;
            cnt-- ;
            *pcdest = cval ;
            pcdest++ ;
         }
      }
   }
   else {
      /* Byte Alignment Copy */
      memcpy (dest, src, cnt) ;
   }

   return (dest) ;
}

static void bond_fkb_free (FkBuff_t *fkb, int flushLen)
{
   UINT8 *pucData ;
   PRXBDINFO pRxBdInfo = (PRXBDINFO) fkb->recycle_context ;

   pucData = PFKBUFF_TO_PDATA(fkb, RXBUF_HEAD_RESERVE) ;
   FlushAssignRxBuffer (pRxBdInfo, pucData, pucData+flushLen) ;
}

/***************************************************************************
 * Function Name: bond_ptm_rx_send
 * Description  : Assemble and send the fragment
 * Returns      : 0 on success.
 ***************************************************************************/
static int bcmxtmrt_ptmbond_rx_send (XtmRtPtmBondInfo *pBondInfo, XtmRtPtmBondRxQInfo *inBuf,
                                     int flags)
{
   FkBuff_t       *fkb_frag = inBuf->rxFkb ;
   FkBuff_t       *processFkb ;
   XtmRtPtmBondRxQInfo  *fwd = &pBondInfo->fwd [PTMBOND_FWD_BUF_INDEX] ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;

	if (fkb_frag) {
		/* remove PTM fragment header */
		fkb_pull(fkb_frag, XTMRT_PTM_BOND_FRAG_HDR_SIZE);

      //printk ("F len = %d flags=%x \n", fkb_frag->len, flags) ;

		/* copy data to current forward buffer */
		if (fwd->valid == TRUE) {
			unsigned char *to;
         FkBuff_t *rxFkb ;

         rxFkb = fwd->rxFkb ;
#if 0
			if (unlikely(fkb_tailroom(rxFkb) < fkb_frag->len)) {
				processFkb = fkb_copy_expand(rxFkb, 0, fkb_frag->len, GFP_ATOMIC);
				//processFkb = fkb_copy_expand(rxFkb, 0, RXBUF_SIZE-rxFkb->len, GFP_ATOMIC);
            XTMRT_BOND_LOG2 ("btmxtmrt: fkb_copy_expand len %d \n", fkb_frag->len) ;
				if (processFkb == NULL)
					return 1;
            //spin_unlock_bh(&pGi->xtmlock_rx);
				bond_fkb_free(rxFkb, 8);
            //spin_lock_bh(&pGi->xtmlock_rx);
				fwd->rxFkb = processFkb;
            rxFkb = fwd->rxFkb ;
            pBondInfo->bStats.cpe++ ;
			}
#endif
			to = _fkb_put(rxFkb, fkb_frag->len);
			bond_memcpy(to, fkb_frag->data, fkb_frag->len);
         pBondInfo->bStats.cpe++ ;
			bond_fkb_free(fkb_frag, fkb_frag->len+8) ;
		}
		else {					/* first frame fragment */
         fwd->rxFkb       = fkb_frag ;
         fwd->pDevCtx     = inBuf->pDevCtx ;
         fwd->rxFkbStatus = inBuf->rxFkbStatus ;
         fwd->phyPort     = inBuf->phyPort ;
         fwd->valid       = TRUE ;
		}
	} /* fkb_frag */
  
	/* when packet is complete, send it */
   if ((fwd->valid == TRUE) && (flags & XTMRT_PTM_BOND_FRAG_HDR_EOP)) {
      processFkb = fwd->rxFkb ;
      //__fkb_trim(processFkb, processFkb->len-ETH_FCS_LEN) ; /* remove FCS */
      processFkb->len -= ETH_FCS_LEN ;

      XTMRT_BOND_LOG2("rx forward frame len=%d\n", processFkb->len);
      XTMRT_BOND_LOG4_FKB(processFkb, "Rx");

      //pBondInfo->lastRxPktSz = processFkb->len + 8 ;
      cache_flush_len (processFkb->data-XTMRT_PTM_BOND_FRAG_HDR_SIZE, 8) ;
      spin_lock_bh (&pGi->xtmlock_rx) ;
      if (bcmxtmrt_process_rx_pkt (fwd->pDevCtx, processFkb, fwd->rxFkbStatus,
                                   XTMRT_PTM_BOND_FRAG_HDR_SIZE, 4) == PACKET_BLOG)
         pBondInfo->bStats.rxp++;
      else
         pBondInfo->bStats.rxpl++;
      fwd->valid = FALSE ;
      //printk("rx forward frame count fc=%d l=%d\n", pBondInfo->bStats.rxp, pBondInfo->bStats.rxpl) ;
      pBondInfo->bStats.rxo += processFkb->len ;
   }

	pBondInfo->lastRxFlags = flags;
	return 0;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_rx_send_expected_fragment
 * Description  : Check for assembly error on expected fragment and send it
 * Returns      : 0 on success.
 ***************************************************************************/
static int bcmxtmrt_ptmbond_rx_send_expected_fragment (XtmRtPtmBondInfo *pBondInfo,
                                                       XtmRtPtmBondRxQInfo *inBuf,
                                                       int seq_nr, int rxFlags)
{
   FkBuff_t    *fkb  = inBuf->rxFkb ;
   //PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;

	/* discard offending non-SOP fragment */
	if (!(rxFlags & XTMRT_PTM_BOND_FRAG_HDR_SOP) &&
		  (pBondInfo->lastRxFlags & XTMRT_PTM_BOND_FRAG_HDR_EOP)) {
		pBondInfo->bStats.eop++;
		//XTMRT_BOND_LOG0("unexpected m/e of packet, total=%d\n", pBondInfo->bStats.eop);
		XTMRT_BOND_LOG0("E-eop\n") ;
		bond_fkb_free(fkb,8) ;
		return 1;
	}

	if ((rxFlags & XTMRT_PTM_BOND_FRAG_HDR_SOP) &&
		!(pBondInfo->lastRxFlags & XTMRT_PTM_BOND_FRAG_HDR_EOP)) {
		pBondInfo->bStats.sop++;
		//XTMRT_BOND_LOG0("unexpected sop, total=%d\n", pBondInfo->bStats.sop);
		XTMRT_BOND_LOG0("E-sop\n") ;
		/* abort previous frame and continue with this new one */
      inBuf->rxFkb = NULL ;
		bcmxtmrt_ptmbond_rx_send (pBondInfo, inBuf, XTMRT_PTM_BOND_FRAG_HDR_EOP);
      inBuf->rxFkb = fkb ;
	}
      
	pBondInfo->expectedSeqNum = NEXT_SEQ_NR(seq_nr);
	pBondInfo->bStats.rfw++;
      
   cache_flush_len (fkb->data, 8) ;
	return bcmxtmrt_ptmbond_rx_send (pBondInfo, inBuf, rxFlags);
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_rx_send_waiting_fragments
 * Description  : Send and de-queue waiting fragments
 * Returns      : 0 on success.
 ***************************************************************************/
static int bcmxtmrt_ptmbond_rx_send_waiting_fragments (XtmRtPtmBondInfo *pBondInfo, int start)
{
	int   result = 0 ;
	XtmRtPtmBondRxQInfo  *processBuf ;

   XTMRT_BOND_LOG2 ("Flushing from %d\n", start) ;

   processBuf = &pBondInfo->rxq[start] ;
   
   while (processBuf->valid == TRUE) {
		int seq_nr = SEQ_NR(processBuf->fragHdr) ;
		int rxFlags = RX_FLAGS(processBuf->fragHdr) ;

		if (seq_nr != pBondInfo->expectedSeqNum) {
			//XTMRT_BOND_LOG0("unexp seq %d in ordered queue\n", seq_nr);
			XTMRT_BOND_LOG0("E-usq %d Exp %d \n", seq_nr, pBondInfo->expectedSeqNum);
			pBondInfo->bStats.usq ++;
			return 1;
		}

		result = bcmxtmrt_ptmbond_rx_send_expected_fragment (pBondInfo, processBuf, seq_nr, rxFlags);
		processBuf->valid = FALSE ;
		pBondInfo->rxFragQueued-- ;
      if (!pBondInfo->rxFragQueued)
         break ;
		start = Q_INDEX(start+1);
      processBuf = &pBondInfo->rxq[start] ;
	}

	return 0;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_rx_resync
 * Description  : Flush queue and resync on error condition
 * Returns      : 0 on success.
 ***************************************************************************/
static void bcmxtmrt_ptmbond_rx_resync (XtmRtPtmBondInfo *pBondInfo, int type, int port)
{
	XtmRtPtmBondRxQInfo  *processBuf ;
   XtmRtPtmBondRxQInfo  *inBuf2 = &pBondInfo->fwd [PTMBOND_SCRATCHPAD_BUF_INDEX2] ;
	int seqStart, qIndex, sendStart = -1;
	int discardedPackets = 0;
	int resyncSequenceNr = -1;
   int expectedSeqNum ;
   //PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;
	
	XTMRT_BOND_LOG2("resync type=%d at expected seq=%d\n", type, pBondInfo->expectedSeqNum) ;

	qIndex = Q_INDEX(pBondInfo->expectedSeqNum);
	/* if we were waiting for an end-of-packet, close the ongoing packet */
	if (!(pBondInfo->lastRxFlags & XTMRT_PTM_BOND_FRAG_HDR_EOP)) {
		//XTMRT_BOND_LOG0("Terminating packet that will never arrive ... (seq=%d)\n",
		     //pBondInfo->expectedSeqNum);
		XTMRT_BOND_LOG0("E-end %d\n", pBondInfo->expectedSeqNum);
      inBuf2->rxFkb = NULL ;
		bcmxtmrt_ptmbond_rx_send (pBondInfo, inBuf2, XTMRT_PTM_BOND_FRAG_HDR_EOP);
		pBondInfo->bStats.end ++;
	}

	if (type & RESYNC_PORT) {
		resyncSequenceNr = Q_INDEX(pBondInfo->lastSeqNum[port]);
	}

	if (type & RESYNC_FLUSH) {
		resyncSequenceNr = XTMRT_PTM_BOND_RX_QUEUE_LEN;
	}

   seqStart = (type & RESYNC_STARTUP) ? 0 : 1 ;

   expectedSeqNum = pBondInfo->expectedSeqNum ;

   for (; seqStart<XTMRT_PTM_BOND_RX_QUEUE_LEN; seqStart++) {

      sendStart = Q_INDEX(qIndex+seqStart);
      pBondInfo->expectedSeqNum = INC_SEQ_NR(expectedSeqNum, seqStart) ;
      if (!pBondInfo->rxFragQueued) {
         break;
      }
      processBuf = &pBondInfo->rxq[sendStart];
      if (processBuf->valid == TRUE) {
         if (!(RX_FLAGS(processBuf->fragHdr) & XTMRT_PTM_BOND_FRAG_HDR_SOP)
               || 
               resyncSequenceNr != -1) {
            processBuf->valid = FALSE ;
            pBondInfo->rxFragQueued-- ;
            discardedPackets ++;
            bond_fkb_free(processBuf->rxFkb,8) ;
         }
         else {
            pBondInfo->expectedSeqNum = SEQ_NR(processBuf->fragHdr);
            break;
         }
      }

      /* as of now we can stop throwing away packets and look for  new start */
      if (sendStart == resyncSequenceNr) {
         resyncSequenceNr = -1;
      }
   } /* for (seqStart) */

	if (sendStart != -1) {
		bcmxtmrt_ptmbond_rx_send_waiting_fragments (pBondInfo, sendStart);
	}

	if (type & RESYNC_FLUSH) 
		pBondInfo->bStats.flu += discardedPackets;
	else if (type & RESYNC_TIMEOUT)
		pBondInfo->bStats.tim += discardedPackets;
	else if (type & RESYNC_PORT)
		pBondInfo->bStats.dro += discardedPackets;
	else if (type & RESYNC_OVERFLOW)
		pBondInfo->bStats.bqo += discardedPackets;
   else
		pBondInfo->bStats.stp += discardedPackets;

	XTMRT_BOND_LOG1("discarded=%d\n", discardedPackets);
}


/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_process_rx_fragment
 * Description  : Receive fragments.  If they come in sequence, assemble and
 *                forward them, else temporarily keep them in their buffer
 *                until the missing packet arrives/timeout condition
 *                happens.
 * Returns      : 0 on success.
 ***************************************************************************/
static int bcmxtmrt_ptmbond_process_rx_fragment (XtmRtPtmBondInfo *pBondInfo, XtmRtPtmBondRxQInfo
                                                 *inBuf)
{
#ifdef PTM_BONDING_DEBUG
   FkBuff_t *fkb = inBuf->rxFkb ;
#endif
   int            port = inBuf->phyPort ;
   int            seq_nr ;
   int            rxFlags ;
   int            qIndex ;
   int            delta = 0;
   int            result = 0;
#ifdef SYNC
   int            syncseq_nr, syncFlags, syncQIndex ;
#endif
#ifndef PTMBOND_DS_UNI_CHANNEL
   int            otherPort ;
#endif
   XtmRtPtmBondRxQInfo  *processBuf ;
#ifdef SYNC
   XtmRtPtmBondRxQInfo  *syncBuf ;
    

   syncBuf = NULL ;
#endif

   seq_nr = SEQ_NR(inBuf->fragHdr);
   rxFlags = RX_FLAGS(inBuf->fragHdr);
   qIndex = Q_INDEX(seq_nr);

	XTMRT_BOND_LOG2("{P-%d, L-%d, H-0x%x, H-0x%x},// expected=%d, seq=%d, rxFlags=%x\n", 
		 port, fkb->len, fkb->data[0], fkb->data[1], 
		 pBondInfo->expectedSeqNum, seq_nr, rxFlags);
	XTMRT_BOND_LOG3_FKB(fkb, "Rx");

#ifdef PTM_BONDING_DEBUG
	if (pBondInfo->seqNum >= MAX_SEQ_DUMP)
      pBondInfo->seqNum = 0 ;
   pBondInfo->seqLen[pBondInfo->seqNum] = fkb->len;
   pBondInfo->seqFlags[pBondInfo->seqNum] = *((XtmRtPtmBondFragHdr*) fkb->data) ;
   pBondInfo->port[pBondInfo->seqNum] = port ;
   pBondInfo->seqNum++ ;
#endif

   PMON_DS_LOG(2) ;

#ifndef PTMBOND_DS_UNI_CHANNEL
	/* verify if the received sequence number is in range */
	if (seq_nr != pBondInfo->expectedSeqNum) {

		if (pBondInfo->lastSeqNum[port] != -1) {
			int deltaOnThisLine = DELTA_SEQ_NR(pBondInfo->lastSeqNum[port], seq_nr);
         if (deltaOnThisLine>0)
            goto _skip1 ;
			if (deltaOnThisLine<0) {
				pBondInfo->bStats.old ++;
				//XTMRT_BOND_LOG0("old sequence=%d, last one on this port=%d, total=%d\n", 
					 //seq_nr, pBondInfo->lastSeqNum[port], pBondInfo->bStats.old);
				XTMRT_BOND_LOG0("E-old %d, last one=%d\n", seq_nr, pBondInfo->lastSeqNum[port]) ;
            //dumpaddr (fkb->data, 8) ;
				result = 1;
			}
			else if (deltaOnThisLine == 0) {
				pBondInfo->bStats.dup ++;
				//XTMRT_BOND_LOG0("duplicate sequence=%d on port %d, total=%d\n",
					 //seq_nr, port, pBondInfo->bStats.dup);
				XTMRT_BOND_LOG0("E-dup %d P-%d\n", seq_nr, port) ;
				result = 1;
			}
		}

_skip1 :
      if (!result) {
         otherPort = port^1 ;
         /* if this is not the first time reception */
         if (pBondInfo->expectedSeqNum != -1) {
            delta = DELTA_SEQ_NR(pBondInfo->expectedSeqNum, seq_nr);
            /* clearly out-of-range sequence number */
            if (ABSOLUTE(delta) > MAX_SEQ_NR_DEVIATION) {
               pBondInfo->bStats.oos ++;
               //XTMRT_BOND_LOG0("out of range seq=%d, delta=%d, lastSeq=%d, total=%d\n",
                     //seq_nr, delta, pBondInfo->lastSeqNum[port], pBondInfo->bStats.oos);
               XTMRT_BOND_LOG0("E-oos %d, delta=%d, lastSeq=%d\n", seq_nr, delta, 
                               pBondInfo->lastSeqNum[port]) ;
               /* if we see this a number of times, force a resync  */
               result = 1;
            }
            else if (delta<0) {
               /* if delta<0 (old packet, older than what we expect), throw the data away */
               pBondInfo->bStats.old ++;
               //XTMRT_BOND_LOG0("old sequence=%d, expected=%d, total=%d\n", 
                     //seq_nr, pBondInfo->expectedSeqNum, pBondInfo->bStats.old);
               XTMRT_BOND_LOG0("E-old %d, exp=%d \n", seq_nr, pBondInfo->expectedSeqNum) ;
               //dumpaddr (fkb->data, 8) ;
               result = 1;
            }
            /* FIXME: case for more than two ports not covered here */
            else if (pBondInfo->lastSeqNum[otherPort] != -1) {
               int diff = DELTA_SEQ_NR(pBondInfo->lastSeqNum[otherPort], pBondInfo->expectedSeqNum);
               if (diff<0) {
                  /* This could actually happen if a port had errors for a while */
                  pBondInfo->bStats.ess ++;
                  //XTMRT_BOND_LOG0("unexpected sequence=%d, expected=%d, last=%d, other=%d\n", 
                        //seq_nr, pBondInfo->expectedSeqNum, 
                        //pBondInfo->lastSeqNum[port], pBondInfo->lastSeqNum[otherPort]);
                  XTMRT_BOND_LOG0("E-ess %d, exp=%d, last=%d, other=%d\n",
                        seq_nr, pBondInfo->expectedSeqNum,
                        pBondInfo->lastSeqNum[port], pBondInfo->lastSeqNum[otherPort]) ;
                  result = 1;
               }
            }
         }
         else {
#ifdef SYNC
            UINT32 portDataMask = inBuf->pDevCtx->ulPortDataMask ;
            UINT32   otherPortStatus = ((0x1 << otherPort) & portDataMask) ;

            /* Syncing */
            if ((pBondInfo->firstSeqNum[port] == -1) && 
                  ((!otherPortStatus) || (pBondInfo->firstSeqNum[otherPort] != -1))) {

               if (otherPortStatus) {
                  /* Decide which is less & sync */
                  int diff = DELTA_SEQ_NR (seq_nr, pBondInfo->firstSeqNum[otherPort]) ;
                  if (diff < 0) {
                     /* Save the current sequence number. Set the expected
                      * number to lastSeqNum[otherPort]. Form the buffer
                      * from here and proceed down as normal
                      */
                     syncseq_nr = pBondInfo->firstSeqNum[otherPort] ;
                     syncQIndex = Q_INDEX(syncseq_nr) ;
                     syncBuf = &pBondInfo->rxq [syncQIndex] ;

                     if (syncBuf->valid == TRUE) {

                        syncFlags  = RX_FLAGS(syncBuf->fragHdr) ;
                        pBondInfo->expectedSeqNum = syncseq_nr ;

                        if (!(syncFlags & XTMRT_PTM_BOND_FRAG_HDR_SOP)) {
                           /* Throw away the syncBuf and find the
                            * expected number.
                            */
                           pBondInfo->expectedSeqNum = syncseq_nr ;
                           XTMRT_BOND_LOG0 ("E-1SyncBuf flags non SOP %x !!!!!\n", syncFlags) ;
                           bcmxtmrt_ptmbond_rx_resync (pBondInfo, RESYNC_STARTUP, 0) ;
                           syncBuf = NULL ;
                        }
                     }
                     else {
                        pBondInfo->firstSeqNum [otherPort] = -1 ;
                        syncBuf = NULL ;
                        XTMRT_BOND_LOG0 ("E-SyncBuf %d not valid!!!!!", syncseq_nr) ;
                     }

                     goto _skipThisPort ;
                  }
               }

               /* Course follows normally */
               if (rxFlags & XTMRT_PTM_BOND_FRAG_HDR_SOP) { /* blindly synchronize */
                  XTMRT_BOND_LOG0("2First reception ... seq %d sync sequence number to %d\n", pBondInfo->firstSeqNum[otherPort], seq_nr);
                  pBondInfo->expectedSeqNum = seq_nr;
               }
               else {			/* throw any packet away until we see a SOP */
                  result = 1;
                  XTMRT_BOND_LOG0 ("E-flags non SOP %x !!!!!", rxFlags) ;
               }
            } /* if ((pBondInfo->firstSeqNum[port] == -1) && (pBondInfo->firstSeqNum[otherPort] != -1)) */

_skipThisPort :
            delta = 0 ;
#else
               /* Course follows normally */
               if (rxFlags & XTMRT_PTM_BOND_FRAG_HDR_SOP) { /* blindly synchronize */
                  XTMRT_BOND_LOG0("2First reception ... seq %d sync sequence number to %d\n", pBondInfo->firstSeqNum[otherPort], seq_nr);
                  pBondInfo->expectedSeqNum = seq_nr;
               }
               else {			/* throw any packet away until we see a SOP */
                  result = 1;
                  XTMRT_BOND_LOG0 ("E-flags non SOP %x !!!!!", rxFlags) ;
               }
#endif
         } /* else if (pBondInfo->expectedSeqNum != -1) */
      } /* (!result) */
   } /* seq_nr */
#else
	/* verify if the received sequence number is in range */
	if (seq_nr != pBondInfo->expectedSeqNum) {
      if (pBondInfo->expectedSeqNum != -1) {
         delta = DELTA_SEQ_NR(pBondInfo->expectedSeqNum, seq_nr);
      }
      else {
         /* Course follows normally */
         if (rxFlags & XTMRT_PTM_BOND_FRAG_HDR_SOP) { /* blindly synchronize */
            XTMRT_BOND_LOG0("2First reception ... seq %d sync sequence number to %d\n", pBondInfo->firstSeqNum[otherPort], seq_nr);
            pBondInfo->expectedSeqNum = seq_nr;
         }
         else {			/* throw any packet away until we see a SOP */
            result = 1;
            XTMRT_BOND_LOG0 ("E-flags non SOP %x !!!!!", rxFlags) ;
         }
      }
   }
#endif /* end of #ifndef PTMBOND_DS_UNI_CHANNEL */

   if (!result) {
      if (pBondInfo->lastSeqNum[port] == -1) {
         pBondInfo->firstSeqNum[port] = seq_nr ;
      }

      pBondInfo->lastSeqNum[port] = seq_nr;
      if (pBondInfo->expectedSeqNum == seq_nr) {
         result = bcmxtmrt_ptmbond_rx_send_expected_fragment (pBondInfo, inBuf, seq_nr, rxFlags);
         if (!result) {
            qIndex = Q_INDEX(qIndex+1);
            result = bcmxtmrt_ptmbond_rx_send_waiting_fragments (pBondInfo, qIndex);
         }
      }
      else {					/* store the fragment on the queue */

         /* is a fragment already pending at that qIndex ? */
         processBuf = &pBondInfo->rxq [qIndex] ;

         if (processBuf->valid == FALSE) {
            processBuf->rxFkb       = inBuf->rxFkb ;
            processBuf->fragHdr     = inBuf->fragHdr ;
            processBuf->pDevCtx     = inBuf->pDevCtx ;
            processBuf->rxFkbStatus = inBuf->rxFkbStatus ;
            processBuf->phyPort     = inBuf->phyPort ;
            processBuf->valid       = TRUE ;
            pBondInfo->rxFragQueued++;
         }
         else {
            /* the same sequence number/same slot seq that appears again.
             * more than 1 seq can map onto the same slot, especially, if
             * frag_q_window is full and we are still waiting for the expected
             * seq */
            if (SEQ_NR(processBuf->fragHdr) == seq_nr) {
               pBondInfo->bStats.dup ++;
               //XTMRT_BOND_LOG0("duplicate sequence=%d already on queue %d, total=%d\n",
                     //seq_nr, qIndex, pBondInfo->bStats.dup);
               XTMRT_BOND_LOG0("E-dup %d\n", seq_nr) ;
            }
            else { /* queue overflow not detected yet, discard old packet */
               //spin_unlock_bh(&pGi->xtmlock_rx);
               //spin_lock_bh(&pGi->xtmlock_rx);
               pBondInfo->bStats.bqo ++;
               //XTMRT_BOND_LOG0("collision sequence=%d, dropping queue=%d, total=%d\n",
                     //seq_nr, qIndex, pBondInfo->bStats.bqo);
               XTMRT_BOND_LOG0("E-bqo %d\n", seq_nr) ;
            }

            bond_fkb_free (inBuf->rxFkb, 8) ;
               result = 1;
            }

         inBuf->valid = FALSE ;

#ifdef SYNC
         if (syncBuf != NULL) {
            result = bcmxtmrt_ptmbond_rx_send_expected_fragment (pBondInfo, syncBuf, syncseq_nr, syncFlags);
		      syncBuf->valid = FALSE ;
		      pBondInfo->rxFragQueued-- ;
            if (!result) {
               syncQIndex = Q_INDEX(syncQIndex+1);
               result = bcmxtmrt_ptmbond_rx_send_waiting_fragments (pBondInfo, syncQIndex);
            }
         }
#endif

         if (delta >= (XTMRT_PTM_BOND_RX_QUEUE_LEN)) {
            XTMRT_BOND_LOG0("buffer queue overrun\n");
            bcmxtmrt_ptmbond_rx_resync (pBondInfo, RESYNC_OVERFLOW, 0);
            result = 0 ;
         }
      } /* else store the fragment */
   } /* !result */
   else {
      bond_fkb_free (inBuf->rxFkb, 8) ;
#ifdef SYNC
      if (syncBuf) {
         printk ("syncBuf non-null. TODO \n") ;
      }
#endif
      inBuf->valid = FALSE ;
   }

   PMON_DS_LOG(3) ;
 
   if (!result) {
      if (pBondInfo->dropMonitor) 
         pBondInfo->dropMonitor -= SEND_WEIGHT;
   } 
   else {
      pBondInfo->dropMonitor += DROP_WEIGHT;
      /* a number of consecutive drops flush the queue entirely throwing all the
         packets away */
      XTMRT_BOND_LOG0 ("E-Mon %d \n", pBondInfo->dropMonitor) ;
      if (pBondInfo->dropMonitor >= RESYNC_LIMIT) {
         bcmxtmrt_ptmbond_rx_resync (pBondInfo, RESYNC_FLUSH, 0);
         bcmxtmrt_ptmbond_initialize (PTM_BOND_INITIALIZE_LOCAL) ;
         pBondInfo->dropMonitor = 0;
      }
   }

   PMON_DS_LOG(4) ;

   pBondInfo->bStats.rfg[port]++;
   pBondInfo->rxLastFrag = jiffies;
   return result;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_get_port
 * Description  : Find port id of the rxed fragment from the buf status.
 * Returns      : port index.
 ***************************************************************************/
static inline int bcmxtmrt_ptmbond_get_port (PBCMXTMRT_DEV_CONTEXT pDevCtx, UINT16 bufStatus)
{
   UINT32           vcId ;
   int              port, retport ;
   UINT32  portDataMask = pDevCtx->ulPortDataMask ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   /* ports 0/1 split in even/odd ranges */
   vcId    = bufStatus & FSTAT_MATCH_ID_MASK ;
   port = pGi->pulRxCamBase [vcId << 0x1] ;     /* Access due to buffer not carrying
                                                 * the rx port  information.
                                                 */
   port &= 0x3 ;                          /* Hardcoded for 6368 */

#ifndef PTMBOND_DS_UNI_CHANNEL
   if (((0x1<<port) & portDataMask))
      retport = port ;
#else
   if (portDataMask != 0x0)
      retport = DS_PTMBOND_CHANNEL ;
#endif
   else 
      retport = MAX_BOND_PORTS ;

   /* if we are here, the port(s) are inoperational */
   return (retport) ;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_del_link
 * Description  : Remove a link from the bonding group
 * Returns      : None
 ***************************************************************************/
static inline void bcmxtmrt_ptmbond_del_link(XtmRtPtmBondInfo *pBondInfo, int port)
{
   if (pBondInfo->rxFragQueued) {
      XTMRT_BOND_LOG2 ("bcmxtmrt: Resync port %d \n", port) ;
      bcmxtmrt_ptmbond_rx_resync (pBondInfo, RESYNC_PORT, port) ;
   }
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_tick
 * Description  : Performs some periodic checks/update
 * Returns      : None
 ***************************************************************************/
void bcmxtmrt_ptmbond_tick (XtmRtPtmBondInfo *pBondInfo)
{
	unsigned long oldest_frag = jiffies-RX_SEQ_TIMEOUT;
	
	/* detect seq time out */
   if (pBondInfo->rxFragQueued
         &&
       time_after(oldest_frag, pBondInfo->rxLastFrag)) {

#ifdef SYNC
      //printk ("timer processing\n") ;
      if (pBondInfo->expectedSeqNum != -1) {
		   bcmxtmrt_ptmbond_rx_resync (pBondInfo, RESYNC_TIMEOUT, 0) ;
         XTMRT_BOND_LOG0 ("T-exp %d \n", pBondInfo->expectedSeqNum) ;
         //printk ("T-exp %d \n", pBondInfo->expectedSeqNum) ;
      }
      else {
         if (pBondInfo->firstSeqNum [0] != -1) {
            pBondInfo->expectedSeqNum = pBondInfo->firstSeqNum [0] ;
         }
         else {
            pBondInfo->expectedSeqNum = pBondInfo->firstSeqNum [1] ;
         }

         XTMRT_BOND_LOG0 ("T-exp %d \n", pBondInfo->expectedSeqNum) ;
         if (pBondInfo->expectedSeqNum != -1) {
            int qIndex = Q_INDEX (pBondInfo->expectedSeqNum) ;
            bcmxtmrt_ptmbond_rx_send_waiting_fragments (pBondInfo, qIndex);
         }
         else {
            printk ("bcmxtmrtbond: No Exp Seq but FragQueued!!!!\n") ;
         }
      }
#else
      bcmxtmrt_ptmbond_rx_resync (pBondInfo, RESYNC_TIMEOUT, 0) ;
#endif
   } /* (pBondInfo->rxFragQueued) */
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_receive_rx_fragment
 * Description  : PTM bonding fragment receive function
 * Returns      : 0 on success
 ***************************************************************************/
void bcmxtmrt_ptmbond_receive_rx_fragment (PBCMXTMRT_DEV_CONTEXT pDevCtx, FkBuff_t *fkb,
                                           UINT16 bufStatus)
{
	int result = 1 ;
   int port ;
	XtmRtPtmBondInfo *pBondInfo ;
   XtmRtPtmBondRxQInfo *inBuf ;
   //PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo ;

   PMON_DS_BGN ;

	pBondInfo = &g_GlobalInfo.ptmBondInfo ;
   inBuf     = &pBondInfo->fwd [PTMBOND_SCRATCHPAD_BUF_INDEX] ;

#if 1
   port = bcmxtmrt_ptmbond_get_port (pDevCtx, bufStatus) ;
#else
   port = bufStatus & FSTAT_MATCH_ID_MASK ;

   //pBondInfo->bStats.rfg[port]++;
   //pBondInfo->bStats.rfg[0]++;
#endif
   PMON_DS_LOG(1) ;

   if (port != MAX_BOND_PORTS) {
      inBuf->pDevCtx     = pDevCtx ;
      inBuf->rxFkbStatus = bufStatus ;
      inBuf->rxFkb       = fkb ;
      inBuf->phyPort     = port ;
      inBuf->fragHdr     = *((XtmRtPtmBondFragHdr *)fkb->data) ;
      inBuf->valid       = TRUE ;
#if 1
      result = bcmxtmrt_ptmbond_process_rx_fragment (pBondInfo, inBuf) ;
      inBuf->valid       = FALSE ;
      cache_flush_len (fkb->data, 8) ;
#else
      pBondInfo->bStats.rfg[port]++;
      //pBondInfo->lastRxPktSz = fkb->len + 8 ;
      bond_fkb_free(fkb, 8) ;
#endif
   }
   else {
      pBondInfo->bStats.inl++ ;
      XTMRT_BOND_LOG1("inactive-port error, total=%d\n", pBondInfo->bStats.inl);
      bond_fkb_free(fkb, 8) ;
   }

   PMON_DS_END(4) ;
}

void bcmxtmrt_ptm_receive_and_drop (PBCMXTMRT_DEV_CONTEXT pDevCtx, FkBuff_t *fkb,
                                    UINT16 bufStatus)
{
   int port ;
	XtmRtPtmBondInfo *pBondInfo ;

	pBondInfo = &g_GlobalInfo.ptmBondInfo ;
   port = bufStatus & FSTAT_MATCH_ID_MASK ;
   pBondInfo->bStats.rfg[port]++;
   bond_fkb_free(fkb, 8) ;
}

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_handle_link_change
 * Description  : Handle port status change conditions. Link becoming
 *                down will need to be handled for the ReSync operation.
 * Returns      : 0 on success
 ***************************************************************************/
void bcmxtmrt_ptmbond_handle_port_status_change (XtmRtPtmBondInfo *pBondInfo,
                                                 UINT32 ulLinkState, UINT32 ulPortMask)
{
#ifndef PTMBOND_DS_UNI_CHANNEL
	int port ;
#endif
   int delport ;

	/* on link down we just remove all links associated with slave from the list */
	if (ulLinkState != LINK_UP) {
#ifndef PTMBOND_DS_UNI_CHANNEL
		for (port=0; port<MAX_BOND_PORTS; port++) {
         if (!(ulPortMask & (0x1 << port))) {
            delport = port ;
            if (delport != DS_PTMBOND_CHANNEL)
               delport = DS_PTMBOND_CHANNEL ;
            bcmxtmrt_ptmbond_del_link (pBondInfo, delport) ;
         }
      }
#else
      delport = DS_PTMBOND_CHANNEL ;
      bcmxtmrt_ptmbond_del_link (pBondInfo, delport) ;
#endif
	} /* if (linkstate == down) */
}

/***************************************************************************
 * Function Name: ProcRxBondCtrs
 * Description  : Displays information about Bonding Rx side counters.
 *                Currently for PTM bonding.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int ProcRxBondCtrs (char *page, char **start, off_t off, int cnt, 
    int *eof, void *data)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    XtmRtPtmBondInfo *pBondInfo ;
    int sz = 0;

    if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {

       pBondInfo = &pGi->ptmBondInfo ;

       sz += sprintf(page + sz, "\nPTM Rx Bonding Information \n") ;
       sz += sprintf(page + sz, "\n========================== \n") ;
       sz += sprintf(page + sz, "Rx Expected Seq                  : %lu \n", (long unsigned int) pBondInfo->expectedSeqNum) ;
       sz += sprintf(page + sz, "Rx Number Of Outstanding Frags   : %lu \n", (long unsigned int) pBondInfo->rxFragQueued) ;
       sz += sprintf(page + sz, "Rx First Seq [0]                 : %lu \n", (long unsigned int) pBondInfo->firstSeqNum[0]) ;
       sz += sprintf(page + sz, "Rx First Seq [1]                 : %lu \n", (long unsigned int) pBondInfo->firstSeqNum[1]) ;
       sz += sprintf(page + sz, "Rx Last Seq [0]                  : %lu \n", (long unsigned int) pBondInfo->lastSeqNum[0]) ;
       sz += sprintf(page + sz, "Rx Last Seq [1]                  : %lu \n", (long unsigned int) pBondInfo->lastSeqNum[1]) ;
       sz += sprintf(page + sz, "Rx Forwarded Fragments           : %lu \n", (long unsigned int) pBondInfo->bStats.rfw) ;
       sz += sprintf(page + sz, "Rx Forwarded Octets              : %lu \n", (long unsigned int) pBondInfo->bStats.rxo) ;
       sz += sprintf(page + sz, "Rx Forwarded Packets             : %lu \n", (long unsigned int) pBondInfo->bStats.rxp) ;
       sz += sprintf(page + sz, "Rx Forwarded Packets (NonAccel)  : %lu \n", (long unsigned int) pBondInfo->bStats.rxpl) ;
       sz += sprintf(page + sz, "Rx Old Seq Nums Rcvd (Past)      : %lu \n", (long unsigned int) pBondInfo->bStats.old) ;
       sz += sprintf(page + sz, "Rx duplicated Seq Rcvd           : %lu \n", (long unsigned int) pBondInfo->bStats.dup) ;
       sz += sprintf(page + sz, "Rx Out Of Sync Fragments Rcvd    : %lu \n", (long unsigned int) pBondInfo->bStats.oos) ;
       sz += sprintf(page + sz, "Rx UnExp Seq Num From Q Store    : %lu \n", (long unsigned int) pBondInfo->bStats.usq) ;
       sz += sprintf(page + sz, "Rx UnExp InSeq Mid/End Of Pkt    : %lu \n", (long unsigned int) pBondInfo->bStats.eop) ;
       sz += sprintf(page + sz, "Rx UnExp InSeq Start Of Pkt      : %lu \n", (long unsigned int) pBondInfo->bStats.sop) ;
       sz += sprintf(page + sz, "Rx Exp Seq Number Skip           : %lu \n", (long unsigned int) pBondInfo->bStats.ess) ;
       sz += sprintf(page + sz, "Rx Artificially Ended Pkts       : %lu \n", (long unsigned int) pBondInfo->bStats.end) ;
       sz += sprintf(page + sz, "Rx Drop due to Flush/Buf Q ORun  : %lu \n", (long unsigned int) pBondInfo->bStats.bqo) ;
       sz += sprintf(page + sz, "Rx Drop due to Startup syncing   : %lu \n", (long unsigned int) pBondInfo->bStats.stp) ;
       sz += sprintf(page + sz, "Rx Drop due to Flush             : %lu \n", (long unsigned int) pBondInfo->bStats.flu) ;
       sz += sprintf(page + sz, "Rx Drop due to Exp Seq Num TmOut : %lu \n", (long unsigned int) pBondInfo->bStats.tim) ;
       sz += sprintf(page + sz, "Rx Drop due to Port Out Os Status: %lu \n", (long unsigned int) pBondInfo->bStats.dro) ;
       sz += sprintf(page + sz, "Rx Fragment Rcvd From InAct Port : %lu \n", (long unsigned int) pBondInfo->bStats.inl) ;
       sz += sprintf(page + sz, "Times of Buffer Expand And CopyDS: %lu \n", (long unsigned int) pBondInfo->bStats.cpe) ;

       sz += sprintf(page + sz, "Rx Fragment Port [0]             : 0x%x %d \n", (unsigned int) pBondInfo->bStats.rfg[0],
                     pBondInfo->bStats.rfg[0]) ;
       sz += sprintf(page + sz, "Rx Fragment Port [1]             : 0x%x %d \n", (unsigned int) pBondInfo->bStats.rfg[1],
                     pBondInfo->bStats.rfg[1]) ;
       sz += sprintf(page + sz, "Total Fragments                  : %ld \n", (long unsigned int) (pBondInfo->bStats.rfg[0]+pBondInfo->bStats.rfg[1])) ;

       memset (&pBondInfo->bStats, 0, sizeof (struct _bStats)) ;
    }
    else {
       sz += sprintf(page + sz, "No Bonding Information \n") ;
    }

    *eof = 1;
    return( sz );
} /* ProcRxBondCtrs */


/***************************************************************************
 * Function Name: ProcRxBondSeq0
 * Description  : Displays information about Bonding Rx side Sequences
 *                0-499.
 *                Currently for PTM bonding.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int ProcRxBondSeq0 (char *page, char **start, off_t off, int cnt, 
    int *eof, void *data)
{
    int sz = 0;
#ifdef PTM_BONDING_DEBUG
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    XtmRtPtmBondInfo *pBondInfo ;
    UINT32 seqCount ;

    if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {

       pBondInfo = &pGi->ptmBondInfo ;

       /* Rx Seq Information */
       for (seqCount = 0; seqCount < MAX_SEQ_DUMP/2; seqCount++) {
         if (!(seqCount%6))
            sz += sprintf(page + sz, "\n") ;
#if 0
         sz += sprintf(page + sz, "S%u P%u, ", pBondInfo->seqFlags[seqCount].s1Val.seqNo,
                       pBondInfo->port[seqCount]) ;
#else
         sz += sprintf(page + sz, "L%u, ", pBondInfo->seqLen[seqCount]) ;
#endif
       }
    }
    else {
#endif
       sz += sprintf(page + sz, "No Bonding Rx Sequence Information \n") ;
#ifdef PTM_BONDING_DEBUG
    }
#endif

   *eof =  1 ;
    return( sz );
} /* ProcRxBondSeq0 */

/***************************************************************************
 * Function Name: ProcRxBondSeq1
 * Description  : Displays information about Bonding Rx side Sequences
 *                500-999.
 *                Currently for PTM bonding.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int ProcRxBondSeq1 (char *page, char **start, off_t off, int cnt, 
    int *eof, void *data)
{
    int sz = 0;

#ifdef PTM_BONDING_DEBUG
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    XtmRtPtmBondInfo *pBondInfo ;
    UINT32 seqCount ;

    if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {

       pBondInfo = &pGi->ptmBondInfo ;

       /* Rx Seq Information */
       for (seqCount = MAX_SEQ_DUMP/2; seqCount < MAX_SEQ_DUMP; seqCount++) {
         if (!(seqCount%6))
            sz += sprintf(page + sz, "\n") ;
#if 0
         sz += sprintf(page + sz, "S%u P%u, ", pBondInfo->seqFlags[seqCount].s1Val.seqNo,
                       pBondInfo->port[seqCount]) ;
#else
         sz += sprintf(page + sz, "L%u, ", pBondInfo->seqLen[seqCount]) ;
#endif
       }
    }
    else {
#endif
       sz += sprintf(page + sz, "No Bonding Rx Sequence Information \n") ;
#ifdef PTM_BONDING_DEBUG
    }
#endif

   *eof =  1 ;
    return( sz );
} /* ProcRxBondSeq1 */

/***************************************************************************
 * Function Name: ProcTxBondInfo
 * Description  : Displays information about Bonding Tx side counters.
 *                Currently for PTM bonding.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
 int ProcTxBondInfo (char *page, char **start, off_t off, int cnt,
    int *eof, void *data)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    volatile XtmRtPtmTxBondHeader *pPtmHeader;
    UINT32 i ;
    int sz = 0, port, eopStatus, fragSize ;

    if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {

       pPtmHeader = &pGi->ptmBondHdr [0] ;

       sz += sprintf(page + sz, "\nPTM Tx Bonding Information \n") ;
       sz += sprintf(page + sz, "\n========================== \n") ;

       sz += sprintf(page + sz, "\nPTM Header Information") ; 
       
       for (i=0; i<XTMRT_PTM_BOND_MAX_FRAG_PER_PKT; i++) {
          
          port       = pPtmHeader[i].sVal.portSel ;
          eopStatus  = pPtmHeader[i].sVal.PktEop ;
          fragSize   = pPtmHeader[i].sVal.FragSize ;
          fragSize   -= (XTMRT_PTM_BOND_FRAG_HDR_SIZE +
                        XTMRT_PTM_CRC_SIZE) ;
          if (eopStatus == 1) {
             sz += sprintf(page + sz, "\nFragment[%u]: port-%u, eopStatus-%u, size-%u \n",
                           i, port, eopStatus, fragSize-ETH_FCS_LEN) ;
             break ;
          }
          else {
             sz += sprintf(page + sz, "\nFragment[%u]: port-%u, eopStatus-%u, size-%u \n",
                           i, port, eopStatus, fragSize) ;
          }
       } /* for (i) */

       for( i = 0; i < MAX_DEV_CTXS; i++ )
       {
          pDevCtx = pGi->pDevCtxs[i];
          if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
          {
             sz += sprintf(page + sz, "dev: %s, ActivePortMask: %u \n",
                   pDevCtx->pDev->name, pDevCtx->ulPortDataMask) ;

          }
       }

       //sz += sprintf(page + sz, "Last Tx Pkt Size       : %u \n",
             //pGi->ptmBondInfo.lastTxPktSz) ;
    }
    else {
       sz += sprintf(page + sz, "No Bonding Information \n") ;
    }

    *eof = 1;
    return( sz );
} /* ProcTxBondInfo */

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_initialize
 * Description  : Initialize ptm bonding information.
 * Returns      : None
 ***************************************************************************/
void bcmxtmrt_ptmbond_initialize (int globalInit)
{
   int i ;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

   if (globalInit) {
      pGi->bondVersionMajor = 2 ;
      pGi->bondVersionMinor = 1 ;
      pGi->bondVersionBuild = 5 ;
      printk(CARDNAME ": Bonding Version : %d.%d.%d \n", pGi->bondVersionMajor,
            pGi->bondVersionMinor, pGi->bondVersionBuild) ;
      printk (CARDNAME ": PTM Bonding configured in system \n") ;
#ifdef PTM_BONDING_DEBUG
      pGi->ptmBondInfo.logLevel = 1 ;
#endif
      //pGi->ptmBondInfo.lastTxPktSz   = 0 ;

      PMON_US_REG(1, "XTMBOND: Get a right size FKB") ;
      PMON_US_REG(2, "XTMBOND: Construct PTM Hdr/Frags") ;
      PMON_US_REG(3, "XTMBOND: Prepare FKB space for frag hdr") ;
      PMON_US_REG(4, "XTMBOND: COPY PTM Hdr/Frags to Tx Buf") ;
      PMON_US_REG(5, "XTMBOND: Return FKB to caller") ;

      PMON_DS_REG(1, "XTMBOND: Rx Fragment Processing Get Port") ;
      PMON_DS_REG(2, "XTMBOND: Rx Fragment Processing Get Seq Info") ;
      PMON_DS_REG(3, "XTMBOND: Rx Fragment Processing exp/non-exp") ;
      PMON_DS_REG(4, "XTMBOND: Rx Fragment Processing Drop Logic due to error") ;
   }

   pGi->ptmBondInfo.expectedSeqNum = -1 ;
   for (i=0; i<MAX_BOND_PORTS; ++i) {
      pGi->ptmBondInfo.lastSeqNum[i] = -1;
      pGi->ptmBondInfo.firstSeqNum[i] = -1;
   }


   for (i=0; i<XTMRT_PTM_BOND_RX_QUEUE_LEN; i++) {
      if (pGi->ptmBondInfo.rxq[i].valid == TRUE) {
         //printk (CARDNAME  ":ptm bond initialize rxQ [%d] is VALID!!!!!!!!. Invalidating...\n", i);
         bond_fkb_free (pGi->ptmBondInfo.rxq[i].rxFkb, 8) ;
         pGi->ptmBondInfo.rxq[i].valid = FALSE ;
      }
   }

   for (i=0; i<3; i++) {
      if (pGi->ptmBondInfo.fwd[i].valid == TRUE) {
         //printk (CARDNAME  ":ptm bond initialize fwd [%d] is VALID!!!!!!!!. Invalidating...\n", i);
         bond_fkb_free (pGi->ptmBondInfo.fwd[i].rxFkb, 8) ;
         pGi->ptmBondInfo.fwd[i].valid = FALSE ;
      }
   }

   pGi->ptmBondInfo.rxFragQueued = 0 ;
   pGi->ptmBondInfo.lastRxFlags = XTMRT_PTM_BOND_FRAG_HDR_EOP ; /* allow the first SOP */
   //pGi->ptmBondInfo.lastRxPktSz   = 512 ;
}
