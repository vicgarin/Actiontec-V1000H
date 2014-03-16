/*
 <:copyright-BRCM:2009:DUAL/GPL:standard
 
    Copyright (c) 2009 Broadcom Corporation
    All Rights Reserved
 
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
/*
 *******************************************************************************
 * File Name  : bcmPktDma.c
 *
 * Description: This file contains the Packet DMA initialization API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>

#include "bcmtypes.h"
#include "bcmPktDma_bds.h"
#include "bcmPktDma_structs.h"
#include "bcmPktDma.h"
#include "bcmenet.h"

//#define BCM_PKTDMA_DUMP_BDS       /* enable dump of RX and TX BDs */

extern int kerSysGetSdramSize( void );
BcmPktDma_Bds bcmPktDma_Bds;
BcmPktDma_Bds *bcmPktDma_Bds_p = &bcmPktDma_Bds;
uint32_t bcmPktDma_tot_rxbds_g = 0;

static int bcmPktDma_calc_rxbds( void );
static int bcmPktDma_calc_txbds( void );

#if defined(BCM_PKTDMA_DUMP_BDS)
static int bcmPktDma_dump_rxbds( void )
{
    uint32_t chnl;

    printk("\n-------- Packet DMA RxBDs ---------\n" );
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* ----------- FAP RX channel ---------- */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        printk( "ETH[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_rxbds[chnl] );
    }

    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        printk( "ETH[%d] Rx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_rxdqm[chnl] );
    }

    /* XTM config */
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxbds[chnl] );
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] Rx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] );
    }

    /* FAP config */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        printk( "FAP ETH[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.eth_rxbds[chnl] );
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "FAP XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] );
    }
#endif


#if defined(CONFIG_BCM96368)
    /* ----------- CMF FWD RX ---------- */
    printk( "ETH[0] # of RxBds=%d\n", bcmPktDma_Bds_p->host.eth_rxbds[0] );

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxbds[chnl] );
    }

    printk( "FWD ETH[0] # of RxBds=%d\n", bcmPktDma_Bds_p->fwd.eth_rxbds[0] );
    printk( "FWD XTM[0] # of RxBds=%d\n", bcmPktDma_Bds_p->fwd.xtm_rxbds[0] );
#endif

#if !(defined(CONFIG_BCM96368) || defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* ----------- Eth RX channel ---------- */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        printk( "ETH[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_rxbds[chnl] );
    }

#if !defined(CONFIG_BCM96816)
    /* ----------- XTM RX channel ---------- */
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxbds[chnl] );
    }
#endif
#endif
    return 0;
}


static int bcmPktDma_dump_txbds( void )
{
    uint32_t chnl;

    printk("\n-------- Packet DMA TxBDs ---------\n" );
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Host config */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        printk( "ETH[%d] # of TxBds =%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_txbds[chnl] );
    }

    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        printk( "ETH[%d] Tx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_txdqm[chnl] );
    }

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of TxBds =%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_txbds[chnl] );
    }

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] Tx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_txdqm[chnl] );
    }

    /* FAP config */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        printk( "FAP ETH[%d] # of TxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.eth_txbds[chnl] );
    }

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "FAP XTM[%d] # of TxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.xtm_txbds[chnl] );
    }
#endif

#if defined(CONFIG_BCM96368)
    printk( "ETH[0] # of TxBds=%d\n", bcmPktDma_Bds_p->host.eth_txbds[0]);

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of TxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_txbds[chnl]);
    }

    printk( "FWD ETH[0] # of TxBds=%d\n", bcmPktDma_Bds_p->fwd.eth_txbds[0]);
    printk( "FWD XTM[0] # of TxBds=%d\n", bcmPktDma_Bds_p->fwd.xtm_txbds[0]);
#endif

#if !(defined(CONFIG_BCM96368) || defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        printk( "ETH[%d] # of TxBds=%d\n", chnl,
                        bcmPktDma_Bds_p->host.eth_txbds[chnl]);
    }

#if !defined(CONFIG_BCM96816)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of TxBds=%d\n", chnl,
                    bcmPktDma_Bds_p->host.xtm_txbds[chnl]);
    }
#endif
#endif
    return 0;
}
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */



static int bcmPktDma_calc_rxbds( void )
{
    uint32_t tot_mem_size = kerSysGetSdramSize();
    uint32_t buf_mem_size = tot_mem_size * CONFIG_BRCM_DEF_BUF_MEM_PRCNT/100;
    uint32_t tot_num_bufs=0;
    uint32_t chnl;
    uint32_t host_eth_rxbds;
#if !defined(CONFIG_BCM96816)
    uint32_t host_xtm_rxbds;
#endif

#if defined(CONFIG_BCM96816)
    /* In case of 6816 the number of RXBDs for default channel is calculated
     * using non-jumbo frame size similar to other CHIPs */
    tot_num_bufs = (buf_mem_size/NON_JUMBO_RX_BUF_SIZE);
#else
    tot_num_bufs = (buf_mem_size/RX_BUF_SIZE);
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* ----------- FAP RX channel ---------- */
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/* Channel-0 is default */
    host_eth_rxbds = (ENET_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/200);

    if (host_eth_rxbds < HOST_ENET_NR_RXBDS_MIN)
        host_eth_rxbds = HOST_ENET_NR_RXBDS_MIN;

    host_xtm_rxbds = (XTM_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/200);

    if (host_xtm_rxbds < HOST_XTM_NR_RXBDS_MIN)
        host_xtm_rxbds = HOST_XTM_NR_RXBDS_MIN;

#else /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */
    host_eth_rxbds = HOST_ENET_NR_RXBDS;
    host_xtm_rxbds = HOST_XTM_NR_RXBDS;
#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED )
            bcmPktDma_Bds_p->host.eth_rxbds[chnl] = host_eth_rxbds;
        else
            bcmPktDma_Bds_p->host.eth_rxbds[chnl] = 0;

        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.eth_rxbds[chnl];
    }

    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED )
            bcmPktDma_Bds_p->host.eth_rxdqm[chnl] = 0;
        else
        {
            bcmPktDma_Bds_p->host.eth_rxdqm[chnl] = DQM_FAP2HOST_ETH0_RX_DEPTH;
        }
    }



    /* XTM config */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
        {
            bcmPktDma_Bds_p->host.xtm_rxbds[chnl] = host_xtm_rxbds;
        }
        else
            bcmPktDma_Bds_p->host.xtm_rxbds[chnl] =
                FAP_XTM_NON_DEF_CHNL_NR_RXBDS;

        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.xtm_rxbds[chnl];
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] = 0;
        else
        {
            if (chnl == 0)
            {
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] =
                    DQM_FAP2HOST_XTM0_RX_DEPTH;
            }
            else
            {
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] =
                    DQM_FAP2HOST_XTM1_RX_DEPTH;
            }
        }
    }
#endif /* #if defined(CONFIG_BCM_XTMCFG) */

    /* FAP config */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.eth_rxbds[chnl] = 0;
        else
            bcmPktDma_Bds_p->fap.eth_rxbds[chnl] = FAP_ENET_NR_RXBDS;

        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->fap.eth_rxbds[chnl];
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] = 0;
        else
        {
            if (chnl == 0)
                bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] = FAP_XTM_NR_RXBDS;
            else
                bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] =
                    FAP_XTM_NON_DEF_CHNL_NR_RXBDS;
        }
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->fap.xtm_rxbds[chnl];
    }
#endif /* defined(CONFIG_BCM_XTMCFG) */
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */


#if defined(CONFIG_BCM96368)
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    /* ----------- CMF FWD RX ---------- */
    /* In case of 6368 because of CMF the number of RXBDs is divided
     * equally between Eth driver and CMF FWD */

    host_eth_rxbds = (ENET_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/200);

    if (host_eth_rxbds < HOST_ENET_NR_RXBDS_MIN)
        host_eth_rxbds = HOST_ENET_NR_RXBDS_MIN;

    host_xtm_rxbds = (XTM_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/200);

    if (host_xtm_rxbds < HOST_XTM_NR_RXBDS_MIN)
        host_xtm_rxbds = HOST_XTM_NR_RXBDS_MIN;
#else /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */
    host_eth_rxbds = HOST_ENET_NR_RXBDS;
    host_xtm_rxbds = HOST_XTM_NR_RXBDS;
#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

    bcmPktDma_Bds_p->host.eth_rxbds[0] = host_eth_rxbds;
    bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.eth_rxbds[0];

    bcmPktDma_Bds_p->host.xtm_rxbds[0] = host_xtm_rxbds;
    for (chnl=1; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_rxbds[chnl] = HOST_XTM_NON_DEF_CHNL_NR_RXBDS;
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.xtm_rxbds[chnl];
    }

    /* Note: The # of FWD RXBDs is equal to host RXBDs */
    bcmPktDma_Bds_p->fwd.eth_rxbds[0] = host_eth_rxbds;
    bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->fwd.eth_rxbds[0];

    bcmPktDma_Bds_p->fwd.xtm_rxbds[0] = host_xtm_rxbds;
    bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->fwd.xtm_rxbds[0];
#endif

#if !(defined(CONFIG_BCM96368) || defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* ----------- Eth RX channel ---------- */
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    host_eth_rxbds = ENET_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/100;
#else /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */
    host_eth_rxbds = HOST_ENET_NR_RXBDS;
#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

    bcmPktDma_Bds_p->host.eth_rxbds[0] = host_eth_rxbds;
    for (chnl=1; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        bcmPktDma_Bds_p->host.eth_rxbds[chnl] =
                    HOST_ENET_NON_DEF_CHNL_NR_RXBDS;
    }

    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS; chnl++)
    {
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.eth_rxbds[chnl];
    }

#if !defined(CONFIG_BCM96816)
    /* ----------- XTM RX channel ---------- */
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    host_xtm_rxbds = XTM_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/100;
#else /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

    /* ----------- XTM RX channel ---------- */
    {
        uint32_t tot_mem_size = kerSysGetSdramSize();

        if (tot_mem_size < 0x800000)    // less than 8MB
            host_xtm_rxbds = 60;
        else
            host_xtm_rxbds = HOST_XTM_NR_RXBDS;
    }
#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

    bcmPktDma_Bds_p->host.xtm_rxbds[0] = host_xtm_rxbds;
    for (chnl=1; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_rxbds[chnl] = HOST_XTM_NON_DEF_CHNL_NR_RXBDS;
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.xtm_rxbds[chnl];
    }
#endif
#endif

#if defined(BCM_PKTDMA_DUMP_BDS)
    bcmPktDma_dump_rxbds();
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */

    printk( "Total # RxBds=%d\n", bcmPktDma_tot_rxbds_g);
    if (bcmPktDma_tot_rxbds_g > tot_num_bufs)
    {
        printk( "ERROR!!!!: Not enough buffers available\n" );
        printk( "ERROR!!!!: Either increase the %% of buffer memory "
                            "using make menuconfig\n" );
        printk( "ERROR!!!!: Or reduce the # of RxBDs (bcmPktDma_bds.h)\n" );
        return -1;
    }
    else if (bcmPktDma_tot_rxbds_g > (tot_num_bufs*2/3))
    {
        printk( "WARNING: # of RXBDs > (buffers*2/3)\n" );
        printk( "WARNING: less buffers available for BPM\n" );
    }

    return 0;
}


static int bcmPktDma_calc_txbds( void )
{
    uint32_t chnl;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    int iudmaIdx;
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Host config */
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        if (g_Eth_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS;
        else
            bcmPktDma_Bds_p->host.eth_txbds[chnl] = 0;
    }

    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        if (g_Eth_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.eth_txdqm[chnl] = 0;
        else
            bcmPktDma_Bds_p->host.eth_txdqm[chnl] = DQM_HOST2FAP_ETH_XMIT_DEPTH;
    }

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_txbds[chnl] = 0;
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.xtm_txdqm[chnl] = 0;
        else
            bcmPktDma_Bds_p->host.xtm_txdqm[chnl] = DQM_HOST2FAP_XTM_XMIT_DEPTH;
    }
#endif /* defined(CONFIG_BCM_XTMCFG) */

    /* FAP config */
    for (iudmaIdx = 0; iudmaIdx < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; iudmaIdx++)
    {
        if (g_Eth_tx_iudma_ownership[iudmaIdx] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.eth_txbds[iudmaIdx] = 0;
        else
            bcmPktDma_Bds_p->fap.eth_txbds[iudmaIdx] = FAP_ENET_NR_TXBDS;
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.xtm_txbds[chnl] = 0;
        else
            bcmPktDma_Bds_p->fap.xtm_txbds[chnl] = FAP_XTM_NR_TXBDS;
    }
#endif /* defined(CONFIG_BCM_XTMCFG) */
#endif

#if defined(CONFIG_BCM96368)
    bcmPktDma_Bds_p->host.eth_txbds[0] = HOST_ENET_NR_TXBDS;

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_txbds[chnl] = HOST_XTM_NR_TXBDS;
    }

    bcmPktDma_Bds_p->fwd.eth_txbds[0] = FWD_ENET_NR_TXBDS;
    bcmPktDma_Bds_p->fwd.xtm_txbds[0] = FWD_XTM_NR_TXBDS;
#endif

#if !(defined(CONFIG_BCM96368) || defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    for (chnl=0; chnl < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; chnl++)
    {
        bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS;
    }

#if !defined(CONFIG_BCM96816)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_txbds[chnl] = HOST_XTM_NR_TXBDS;
    }
#endif
#endif

#if defined(BCM_PKTDMA_DUMP_BDS)
    bcmPktDma_dump_txbds();
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */

    return 0;
}



int bcmPktDma_EthGetRxBds( BcmPktDma_LocalEthRxDma *rxdma, int channel )
{
    int nr_rx_bds;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Eth_rx_iudma_ownership[channel] == HOST_OWNED)
        nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxbds[channel];
    else
        nr_rx_bds = bcmPktDma_Bds_p->fap.eth_rxbds[channel];
#else
    nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxbds[channel];
#endif

    return nr_rx_bds;
}


int bcmPktDma_EthGetTxBds( BcmPktDma_LocalEthTxDma *txdma, int channel )
{
    int nr_tx_bds;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Eth_tx_iudma_ownership[channel] == HOST_OWNED)
        nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];
    else
        nr_tx_bds = bcmPktDma_Bds_p->fap.eth_txbds[channel];
#else
    nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];
#endif

    return nr_tx_bds;
}


int bcmPktDma_XtmGetRxBds( int channel )
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int nr_rx_bds = 0;

#if !defined(CONFIG_BCM96816)
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Xtm_rx_iudma_ownership[channel] == HOST_OWNED)
        nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[channel];
    else
        nr_rx_bds = bcmPktDma_Bds_p->fap.xtm_rxbds[channel];
#else
    nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[channel];
#endif
#endif

    return nr_rx_bds;
#else
    return -1;
#endif
}


int bcmPktDma_XtmGetTxBds( int channel )
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int nr_tx_bds = 0;

#if !defined(CONFIG_BCM96816)
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Xtm_tx_iudma_ownership[channel] == HOST_OWNED)
        nr_tx_bds = bcmPktDma_Bds_p->host.xtm_txbds[channel];
    else
        nr_tx_bds = bcmPktDma_Bds_p->fap.xtm_txbds[channel];
#else
    nr_tx_bds = bcmPktDma_Bds_p->host.xtm_txbds[channel];
#endif
#endif
    return nr_tx_bds;
#else
    return -1;
#endif
}


int __init bcmPktDmaBds_init(void)
{
    memset( bcmPktDma_Bds_p, 0, sizeof(BcmPktDma_Bds) );
    bcmPktDma_calc_rxbds();
    bcmPktDma_calc_txbds();
    printk("%s: Broadcom Packet DMA BDs initialized\n\n", __FUNCTION__);

    return 0;
}

void __exit bcmPktDmaBds_exit(void)
{
    printk("Broadcom Packet DMA BDs exited\n");
}

module_init(bcmPktDmaBds_init);
module_exit(bcmPktDmaBds_exit);


EXPORT_SYMBOL(bcmPktDma_Bds_p);
EXPORT_SYMBOL(bcmPktDma_tot_rxbds_g);
EXPORT_SYMBOL(bcmPktDma_EthGetRxBds);
EXPORT_SYMBOL(bcmPktDma_EthGetTxBds);
EXPORT_SYMBOL(bcmPktDma_XtmGetRxBds);
EXPORT_SYMBOL(bcmPktDma_XtmGetTxBds);


