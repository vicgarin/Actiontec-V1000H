/*
<:label-BRCM:2011:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

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
#ifndef FAP_4KE
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
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"
#include "bcmPktDma.h"
#endif   /* FAP_4KE */

#ifdef FAP_4KE
#include "fap4ke_local.h"
#include "fap4ke_printer.h"
#include "bcmxtmrtbond.h"
#endif

#ifdef PERF_MON_BONDING_US
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

#if defined(FAP_4KE) || !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))

static inline int getPtmFragmentLen(uint32 len);
static int constructPtmBondHdr(XtmRtPtmTxBondHeader *ptmBondHdr_p, uint32 len,
                               uint32 ulPtmPrioIdx);


/***************************************************************************
 * Function Name: getPtmFragmentLen
 * Description  : Compute next fragment length
 * Returns      : fragment length.
 ***************************************************************************/
static inline int getPtmFragmentLen(uint32 len)
{
	int fragmentLen;
	int leftOver;

	if (len <= XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE)
   {
		fragmentLen = len;
	}
	else
   {
		/* send as much as possible unless we don't have
		   enough data left anymore for a full fragment */
		fragmentLen = XTMRT_PTM_BOND_TX_MAX_FRAGMENT_SIZE;
		leftOver    = len - fragmentLen;
		if (leftOver < XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE)
      {
			fragmentLen -= (XTMRT_PTM_BOND_TX_MIN_FRAGMENT_SIZE + leftOver);
			fragmentLen &= ~0x3; /* make it a multiple of 4 bytes */
		}
	}
	return fragmentLen;
   
}  /* getPtmFragmentLen() */

/***************************************************************************
 * Function Name: constructPtmBondHdr
 * Description  : Calculates the PTM bonding hdr information and fills it up
 *                in the global buffer to be used for the packet.
 * Returns      : NoofFragments.
 ***************************************************************************/
static int constructPtmBondHdr(XtmRtPtmTxBondHeader *ptmBondHdr_p, uint32 len,
                               uint32 ptmPrioIdx)
{
   XtmRtPtmTxBondHeader *bondHdr_p  = NULL;
#ifdef FAP_4KE
   XtmRtPtmBondInfo     *bondInfo_p = &(p4kePsmGbl->ptmBondInfo);
#else   
   XtmRtPtmBondInfo     *bondInfo_p = &(g_GlobalInfo.ptmBondInfo);
#endif   
   int    *linkWt_p    = &(bondInfo_p->linkWt[0]);
   uint16 *fragCount_p = &(bondInfo_p->fragCount);
   uint16 *distIdx_p   = &(bondInfo_p->distIdx);
   uint16 nextPort;
   int    fragNo = 0;
   int    fragLen;
   
   len += ETH_FCS_LEN;    /* Original Packet Len + 4 bytes of Eth FCS Len */
   
#ifndef PTMBOND_US_PRIO_TRAFFIC_SPLIT

   if ((bondInfo_p->portMask & 0x3) == 0x3)
   {
      /* both ports are up. */
      while (len)
      {
         /* select next port */     
         nextPort = (*fragCount_p % bondInfo_p->hi2loRatio)?
                        bondInfo_p->hibwPort : (bondInfo_p->hibwPort ^ 0x1);
                        
         
         if (linkWt_p[nextPort] <= 0)
         {
            /* nextPort runs out of tokens. try the other port. */
            nextPort ^= 0x1;
            
            if (linkWt_p[nextPort] <= 0)
            {
               /* the other port also runs out of tokens.
                * reload both token buckets.
                */
               linkWt_p[0] = bondInfo_p->confLinkWt[0];
               linkWt_p[1] = bondInfo_p->confLinkWt[1];
            }
         }
         
         /* select nextPort. */
         fragLen  = getPtmFragmentLen(len);
         len     -= fragLen;
         fragLen += XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE;  /* With actual hdrs/trailers */
   
         bondHdr_p = ptmBondHdr_p + fragNo;
         bondHdr_p->usVal =
            (nextPort << XTMRT_PTM_BOND_PORTSEL_SHIFT) | (uint16)fragLen;
         
         /* deduct fragLen from its bucket. */
         linkWt_p[nextPort] -= fragLen;

         fragNo++;
         
         if (++(*distIdx_p) < bondInfo_p->hi2loRatio)
         {
            (*fragCount_p)++;
         }
         else
         {
            *distIdx_p = 0;
         }
         
      } /* while (len) */
   }
   else if (bondInfo_p->portMask & 0x3)
   {
      /* only one port is up. select it. */
      nextPort = ((bondInfo_p->portMask & 0x1)? PHY_PORTID_0 : PHY_PORTID_1) << XTMRT_PTM_BOND_PORTSEL_SHIFT;
         
      while (len)
      {
         fragLen  = getPtmFragmentLen(len);
         len     -= fragLen;
         fragLen += XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE;  /* With actual hdrs/trailers */
   
         bondHdr_p = ptmBondHdr_p + fragNo;
         bondHdr_p->usVal = nextPort | (uint16)fragLen;
         
         /* since only one port is up, don't deduct token from bucket. */
         
         fragNo++;
         
      } /* while (len) */
   }
   else
   {
      /* both ports are down. */
      return 0;   /* discard packet. */
   }
   
#else /* #ifndef PTMBOND_US_PRIO_TRAFFIC_SPLIT */ 

   while (len)
   {
      fragLen  = getPtmFragmentLen(len);
      len     -= fragLen;
      fragLen += XTMRT_PTM_BOND_FRAG_HDR_SIZE + XTMRT_PTM_CRC_SIZE;  /* With actual hdrs/trailers */
      
      bondHdr_p = ptmBondHdr_p + fragNo;
      bondHdr_p->sVal.FragSize = fragLen;
      bondHdr_p->sVal.portSel  = (ptmPrioIdx == PTM_FLOW_PRI_LOW) ? PHY_PORTID_0 : PHY_PORTID_1;
      bondHdr_p->sVal.PktEop   = XTMRT_PTM_BOND_HDR_NON_EOP;
      
      fragNo++;
      
   } /* while (len) */

#endif /* #ifndef PTMBOND_US_PRIO_TRAFFIC_SPLIT */
   
   bondHdr_p->usVal |= XTMRT_PTM_BOND_FRAG_HDR_EOP_MASK;
   
   return (fragNo);
   
}  /* constructPtmBondHdr() */

#endif   /* #if defined(FAP_4KE) || !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */

#if !defined(FAP_4KE)
/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_calculate_link_weights
 * Description  : Calculates the ptm bonding link weights, based on the
 *                link availability. For host execution, also forwards the
 *                the upstream rate of each link and the portMask to 4KE
 *                with a DQM message.
 * Returns      : 0
 * Note: Apply the FAP_4KE compiler flag to allow this function source shared
 * by Host and 4KE.
 ***************************************************************************/
int bcmxtmrt_ptmbond_calculate_link_weights(uint32 linkRate0, uint32 linkRate1,
                                            uint32 portMask) 
{
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   volatile fap4kePsm_global_t * pPsmGbl = (volatile fap4kePsm_global_t *)pHostPsmGbl(PKTDMA_US_FAP_INDEX) ;
   volatile XtmRtPtmBondInfo *p4keBondInfo_p = &(pPsmGbl->ptmBondInfo);
#endif
   volatile XtmRtPtmBondInfo *bondInfo_p = &(g_GlobalInfo.ptmBondInfo);
   int lo, hi;

   /* use the link byte rate as the number of tokens for each bucket. */   
   bondInfo_p->linkWt[0] = bondInfo_p->confLinkWt[0] = linkRate0/8;  /* bytes per sec */
   bondInfo_p->linkWt[1] = bondInfo_p->confLinkWt[1] = linkRate1/8;
   bondInfo_p->portMask  = portMask;
   if (bondInfo_p->linkWt[0] >= bondInfo_p->linkWt[1])
   {
      hi = PHY_PORTID_0;
      lo = PHY_PORTID_1;
   }
   else
   {
      hi = PHY_PORTID_1;
      lo = PHY_PORTID_0;
   }
   
   if (bondInfo_p->linkWt[lo] == 0)
   {
      bondInfo_p->hi2loRatio = 0xFF;
   }
   else
   {
      /* calculate the high to low bandwidth ratio. add 1 for modulo operation. */
      bondInfo_p->hi2loRatio =
         ((bondInfo_p->linkWt[hi]+(bondInfo_p->linkWt[lo]/2))/bondInfo_p->linkWt[lo])+1;
   }

   bondInfo_p->hibwPort  = hi;
   bondInfo_p->fragCount = 0;
   bondInfo_p->distIdx   = 0;
   bondInfo_p->bonding   = 1; /* set bonding to true */
   
   printk("[HOST] link0=%u link1=%u mask=0x%x hibwPort=%u hi2loRatio=%d\n",
          (unsigned int)linkRate0, (unsigned int)linkRate1, (unsigned int)bondInfo_p->portMask,
          (unsigned int)bondInfo_p->hibwPort, (unsigned int)bondInfo_p->hi2loRatio);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   memcpy ((XtmRtPtmBondInfo *) p4keBondInfo_p, (XtmRtPtmBondInfo *) bondInfo_p, sizeof (XtmRtPtmBondInfo)) ;
   
   /* Wait for FAP to finish re-calculating the weights. */
   udelay(50);

   printk("[FAP] link0=%u link1=%u mask=0x%x hibwPort=%u hi2loRatio=%d\n",
          (unsigned int)linkRate0, (unsigned int)linkRate1, (unsigned int)p4keBondInfo_p->portMask,
          (unsigned int)p4keBondInfo_p->hibwPort, (unsigned int)p4keBondInfo_p->hi2loRatio);
#endif

   return 0;

}  /* bcmxtmrt_ptmbond_calculate_link_weights() */
#endif

/***************************************************************************
 * Function Name: bcmxtmrt_ptmbond_add_hdr
 * Description  : Adds the PTM Bonding Tx header to a packet before transmitting
 *                it. The header is composed of 8 16-bit words of fragment info.
 *                Each fragment info word contains the length of the fragment and
 *                its tx port (link).
 *                The number of fragments is calculated from the packet length.
 *                The end-of-packet bit is set in the last fragment info word.
 *                The tx port for each fragment is selected based on the tx
 *                bandwidth of each link so that both links are load balanced.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
 * Returns      : 0: fail, others: success.
 ***************************************************************************/
#ifdef FAP_4KE

int bcmxtmrt_ptmbond_addHdr_4ke(uint8 **packetTx_pp, uint16 *len_p, uint32 ptmPrioIdx)
{
   XtmRtPtmTxBondHeader ptmBondHdr[XTMRT_PTM_BOND_MAX_FRAG_PER_PKT];
   int i, frags;
   
   PMON_US_BGN;

   PMON_US_LOG(1);
   frags = constructPtmBondHdr(&ptmBondHdr[0], *len_p, ptmPrioIdx);
   if (frags)
   {
      *len_p +=
         (sizeof(XtmRtPtmTxBondHeader) * XTMRT_PTM_BOND_MAX_FRAG_PER_PKT);
      *packetTx_pp -=
         (sizeof(XtmRtPtmTxBondHeader) * XTMRT_PTM_BOND_MAX_FRAG_PER_PKT);
      if ((uint32)(*packetTx_pp) & 0x3)
      {
         uint16 *packet_p = (uint16*)mCacheToNonCacheVirtAddr(*packetTx_pp);
         uint16 *hdr_p    = (uint16*)(&ptmBondHdr[0]);
         *packet_p = *hdr_p;  /* each fragment header is 2 bytes */
         for (i = 2; i <= frags; i++)
            *(++packet_p) = *(++hdr_p);
      }
      else
      {
         uint32 *packet_p = (uint32*)mCacheToNonCacheVirtAddr(*packetTx_pp);
         uint32 *hdr_p    = (uint32*)(&ptmBondHdr[0]);
         *packet_p = *hdr_p;  /* each fragment header is 2 bytes */
         for (i = 3; i <= frags; i += 2)
            *(++packet_p) = *(++hdr_p);
      }
   }
   PMON_US_LOG(2);
   PMON_US_END(2);

   return frags;
   
} /* bcmxtmrt_ptmbond_addHdr_4ke() */

#else /* #ifdef FAP_4KE */

/* this function will be called by host regardless FAP is compiled in or not.
 */
int bcmxtmrt_ptmbond_add_hdr(pNBuff_t *ppNBuff, struct sk_buff **ppNBuffSkb, 
                             uint8 **ppData, int *pLen, uint32 ulPtmPrioIdx)
{
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
   XtmRtPtmTxBondHeader *bondHdr_p = &(g_GlobalInfo.ptmBondHdr[0]);
#endif   
   int                   frags = 0;
   int                   headroom;
   int                   minheadroom;

   PMON_US_BGN;

   minheadroom = sizeof (XtmRtPtmTxBondHeader) * XTMRT_PTM_BOND_MAX_FRAG_PER_PKT;

//   printk("bcmxtmrt_ptmbond_add_hdr: ptmPrioIdx=%u\n", (unsigned int)ulPtmPrioIdx);                   

   PMON_US_LOG(1);

   if (*ppNBuffSkb == NULL)
   {
      struct fkbuff *fkb = PNBUFF_2_FKBUFF(*ppNBuff);
      headroom = fkb_headroom(fkb);

      if (headroom >= minheadroom)
      {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
         /* When FAP is compiled in, just make room for the bonding header
          * without actually copy the header to the packet. The header
          * will be added in by FAP 4KE.
          */
         *ppData = fkb_push(fkb, minheadroom);
         *pLen  += minheadroom;
         frags   = 1;
#else
         frags = constructPtmBondHdr(bondHdr_p, *pLen, ulPtmPrioIdx);
         if (frags != 0)
         {
             PMON_US_LOG(2);
             *ppData = fkb_push(fkb, minheadroom);
             *pLen  += minheadroom;
             PMON_US_LOG(3);
             u16cpy ((XtmRtPtmTxBondHeader *)*ppData, bondHdr_p,
                     sizeof(XtmRtPtmTxBondHeader) * frags);
             PMON_US_LOG(4);
         }
#endif         
      }
      else
         printk(CARDNAME "bcmxtmrt_xmit: FKB not enough headroom.\n");
   }
   else
   {
      struct sk_buff *skb = *ppNBuffSkb;
      headroom = skb_headroom(skb);

      if (headroom < minheadroom)
      {
         struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);
#if 0
         printk ("bcmxtmrt_ptmbond_add_hdr: Warning!!, headroom (%d) is less than min headroom (%d) \n",
               headroom, minheadroom);
#endif
         dev_kfree_skb_any(skb);
         if (skb2 == NULL)
         {
            printk (CARDNAME ": Fatal!!, NULL Skb \n");
            skb = NULL;
         }
         else
            skb = skb2;
      }

      if( skb )
      {
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
         /* When FAP is compiled in, just make room for the bonding header
          * without actually copy the header to the packet. The header
          * will be added in by FAP 4KE.
          */
         *ppData = skb_push(skb, minheadroom);
         *pLen  += minheadroom;
         frags   = 1;
#else
         frags = constructPtmBondHdr(bondHdr_p, *pLen, ulPtmPrioIdx);
         if (frags != 0)
         {
             PMON_US_LOG(2);
             *ppData = skb_push(skb, minheadroom);
             *pLen  += minheadroom;
             PMON_US_LOG(3);
             u16cpy ((XtmRtPtmTxBondHeader *)*ppData, bondHdr_p,
                     sizeof(XtmRtPtmTxBondHeader) * frags);
             PMON_US_LOG(4);
         }
#endif         
      }

      *ppNBuffSkb = skb;
      *ppNBuff = SKBUFF_2_PNBUFF(skb);
   }

   PMON_US_LOG(5);
   PMON_US_END(5);

   return frags;
   
} /* bcmxtmrt_ptmbond_add_hdr() */

/***************************************************************************
 * Function Name: ProcTxBondInfo
 * Description  : Displays information about Bonding Tx side counters.
 *                Currently for PTM bonding.
 * Returns      : 0 if successful or error status
 ***************************************************************************/
int ProcTxBondInfo(char *page, char **start, off_t off, int cnt,
                   int *eof, void *data)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
   XtmRtPtmBondInfo     *bondInfo_p = &pGi->ptmBondInfo;
   XtmRtPtmTxBondHeader *bondHdr_p  = &pGi->ptmBondHdr[0];
   uint32 i;
   int sz = 0, port, eopStatus, fragSize;

   if (pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
   {
      sz += sprintf(page + sz, "\nPTM Tx Bonding Information \n");
      sz += sprintf(page + sz, "\n========================== \n");

      sz += sprintf(page + sz, "\nPTM Header Information"); 
       
      for (i = 0; i < XTMRT_PTM_BOND_MAX_FRAG_PER_PKT; i++)
      {
         port       = bondHdr_p[i].sVal.portSel;
         eopStatus  = bondHdr_p[i].sVal.PktEop;
         fragSize   = bondHdr_p[i].sVal.FragSize;
         fragSize   -= (XTMRT_PTM_BOND_FRAG_HDR_SIZE+
                        XTMRT_PTM_CRC_SIZE);
         if (eopStatus == 1)
         {
            sz += sprintf(page + sz, "\nFragment[%u]: port<%d> eopStatus<%d> size<%d>\n",
                          (unsigned int)i, port, eopStatus, fragSize-ETH_FCS_LEN);
            break;
         }
         else
         {
            sz += sprintf(page + sz, "\nFragment[%u]: port<%u> eopStatus<%u> size<%u>\n",
                          (unsigned int)i, port, eopStatus, fragSize);
         }
      } /* for (i) */

      for (i = 0; i < MAX_DEV_CTXS; i++)
      {
         pDevCtx = pGi->pDevCtxs[i];
         if ( pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL )
         {
            sz += sprintf(page + sz, "dev: %s, ActivePortMask<%u>\n",
                  pDevCtx->pDev->name, (unsigned int)pDevCtx->ulPortDataMask);
         }
      }

      for (i = 0; i < MAX_BOND_PORTS; i++ )
      {
         sz += sprintf(page + sz, "Port[%u]: ConfWt<%u> CurrWt<%u>\n",
                (unsigned int)i, (unsigned int)bondInfo_p->confLinkWt[i],
                (unsigned int)bondInfo_p->linkWt[i]);
      }

      sz += sprintf(page + sz, "\n");
    }
    else
    {
       sz += sprintf(page + sz, "No Bonding Information \n");
    }

    *eof = 1;
    return( sz );
        
} /* ProcTxBondInfo() */

#endif   /* FAP_4KE */
