/*
<:copyright-gpl
 Copyright 2007-2010 Broadcom Corp. All Rights Reserved.

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

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map.h>
#include "bcmenet.h"
#include "bcmPktDma.h"



#define ENET_POS_TO_DMA_CHANNEL(pos)              (((pos) & 0xffff0000) >> 16)
#define ENET_POS_TO_SLOT(pos)                     ((pos) & 0xffff)
#define ENET_DMA_CHANNEL_SLOT_TO_POS(chan, slot)  ((chan) << 16 | (slot))

#define ENET_GET_NUMRXBDS(r)        ((r)->pktDmaRxInfo.numRxBds)
#define ENET_GET_RXASSIGNEDBDS(r)   ((r)->pktDmaRxInfo.rxAssignedBds)
#define ENET_GET_RXHEADINDEX(r)     ((r)->pktDmaRxInfo.rxHeadIndex)
#define ENET_GET_RXTAILINDEX(r)     ((r)->pktDmaRxInfo.rxTailIndex)
#define ENET_GET_RXBDS_LENGTH(r, i) ((r)->pktDmaRxInfo.rxBds[(i)].length)
#define ENET_GET_RXBDS_STATUS(r, i) ((r)->pktDmaRxInfo.rxBds[(i)].status)
#define ENET_GET_RXBDS_ADDR(r, i)   ((r)->pktDmaRxInfo.rxBds[(i)].address)

/* TODO: only 1 DMA channel is supported for now */
static int enet_max_dma_channels=1;

/* this is common for pktdma and non-pktdma */
static void *rxbd_seq_start(struct seq_file *f, loff_t *pos)
{
	BcmEnet_RxDma *rxdma;
	uint32_t val = (uint32_t) *pos;
	uint16_t dma_channel;
	uint16_t slot;

	dma_channel = ENET_POS_TO_DMA_CHANNEL(val);
	slot = ENET_POS_TO_SLOT(val);

	rxdma = ((BcmEnet_devctrl *)f->private)->rxdma[dma_channel];

	/* check if there is more data to return */

	if (dma_channel < enet_max_dma_channels && slot < ENET_GET_NUMRXBDS(rxdma)) {
		return pos;
	} else {
		return NULL;
	}
}

static void *rxbd_seq_next(struct seq_file *f, void *v, loff_t *pos)
{
	BcmEnet_RxDma *rxdma;
	uint32_t val = (uint32_t) *pos;
	uint16_t dma_channel;
	uint16_t slot;

	dma_channel = ENET_POS_TO_DMA_CHANNEL(val);
	slot = ENET_POS_TO_SLOT(val);

	rxdma = ((BcmEnet_devctrl *)f->private)->rxdma[dma_channel];

	/* move to the next slot, if no more slots, move to next DMA channel */
	slot++;
	if (slot > ENET_GET_NUMRXBDS(rxdma)) {
		dma_channel++;
		if (dma_channel >= enet_max_dma_channels) {
			return NULL;
		}
		slot = 0;
	}

	val = ENET_DMA_CHANNEL_SLOT_TO_POS(dma_channel, slot);
	*pos = val;
	return pos;
}

static int rxbd_seq_show(struct seq_file *f, void *v)
{
  BcmEnet_devctrl *enetdev = (BcmEnet_devctrl *) f->private;
  BcmEnet_RxDma *rxdma;
  uint32_t pos = *(loff_t *)v;
  uint16_t dma_channel;
  uint16_t slot;

  dma_channel = ENET_POS_TO_DMA_CHANNEL(pos);
  slot = ENET_POS_TO_SLOT(pos);
  rxdma =  enetdev->rxdma[dma_channel];

  if (slot == 0) {
    seq_printf(f,
        "rxbd: total %d, avail %d, head %d, tail %d\n",
        ENET_GET_NUMRXBDS(rxdma),
        ENET_GET_RXASSIGNEDBDS(rxdma),
        ENET_GET_RXHEADINDEX(rxdma),
        ENET_GET_RXHEADINDEX(rxdma)
        );

    seq_printf(f, "idx:len :stat:address\n");
  }

  seq_printf(f,
          "%03d:%04x:%04x:%08lx\n",
          slot,
          ENET_GET_RXBDS_LENGTH(rxdma, slot),
          ENET_GET_RXBDS_STATUS(rxdma, slot),
          ENET_GET_RXBDS_ADDR(rxdma, slot)
          );

  return 0;
}

static void rxbd_seq_stop(struct seq_file *f, void *v)
{
	/* Nothing to do */
}

static const struct seq_operations rxbd_seq_ops = {
	.start = rxbd_seq_start,
	.next  = rxbd_seq_next,
	.stop  = rxbd_seq_stop,
	.show  = rxbd_seq_show
};

static int rxbd_open(struct inode *inode, struct file *filp)
{
	int res;
	struct proc_dir_entry *leafentry = PDE(inode);
	if (0 == (res = seq_open(filp, &rxbd_seq_ops)))
		((struct seq_file *)filp->private_data)->private=leafentry->data;

	return res;
}

static const struct file_operations rxbd_fops = {
	.open		= rxbd_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};


static int proc_get_txbd0(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
  int n, r, i;
  int nr_tx_bds; 
  
/* TBD: for each channel */
  BcmPktDma_EthTxDma *p = ((BcmEnet_devctrl *)data)->txdma[0];
  nr_tx_bds = bcmPktDma_EthGetTxBds( p, 0 );
  
  n = 0;
 
  r = sprintf(
        page, "== txbd (%d/2): total %d, avail %d, head %d, tail %d ==\n",
        (n == 0)? 1: 2,
        nr_tx_bds,
        p->txFreeBds,
        p->txHeadIndex,
        p->txTailIndex
        );

  r += sprintf(page + r, "idx: len:stat: address\n");

  for (i = n; i < n + nr_tx_bds / 2; i++)
  {
    r += sprintf(
          page + r, "%03d:%04x:%04x:%08lx\n",
          i,
          p->txBds[i].length,
          p->txBds[i].status,
          p->txBds[i].address
          );
  }

  r += sprintf(page + r, "\n");
  *eof = 1;
  return r;
}

static int proc_get_txbd1(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
  int n, r, i;
  int nr_tx_bds; 

/* TBD: for each channel */
  BcmPktDma_EthTxDma *p = ((BcmEnet_devctrl *)data)->txdma[0];
  nr_tx_bds = bcmPktDma_EthGetTxBds( p, 0 );

  n = nr_tx_bds / 2;
 
  r = sprintf(
        page, "== txbd (%d/2): total %d, avail %d, head %d, tail %d ==\n",
        (n == 0)? 1: 2,
        nr_tx_bds,
        p->txFreeBds,
        p->txHeadIndex,
        p->txTailIndex
        );

  r += sprintf(page + r, "idx: len:stat: address\n");

  for (i = n; i < n + nr_tx_bds / 2; i++)
  {
    r += sprintf(
          page + r, "%03d:%04x:%04x:%08lx\n",
          i,
          p->txBds[i].length,
          p->txBds[i].status,
          p->txBds[i].address
          );
  }

  r += sprintf(page + r, "\n");
  *eof = 1;
  return r;
}

extern volatile IrqControl_t * brcm_irq_ctrl[NR_CPUS];

static int proc_get_dma_summary(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
  int r;
  BcmEnet_devctrl *p = (BcmEnet_devctrl *)data;
  BcmPktDma_EthTxDma *tx = p->txdma[0];
  BcmEnet_RxDma *rx = p->rxdma[0];

  DmaStateRam *rx_sram = (DmaStateRam *)&p->dmaCtrl->stram.s[p->unit * 2];
  DmaStateRam *tx_sram = (DmaStateRam *)&p->dmaCtrl->stram.s[p->unit * 2 + 1];

  r = sprintf(page, "== dma controller registers ==\n");

  r += sprintf(page + r, "controller config: %08lx\n", p->dmaCtrl->controller_cfg);

  r += sprintf(page + r, "ch:  config:int stat:int mask\n");

  r += sprintf(page + r, "rx:%08lx:%08lx:%08lx\n",
        rx->pktDmaRxInfo.rxDma->cfg,
        rx->pktDmaRxInfo.rxDma->intStat,
        rx->pktDmaRxInfo.rxDma->intMask
        );

  r += sprintf(page + r, "tx:%08lx:%08lx:%08lx\n\n",
        tx->txDma->cfg,
        tx->txDma->intStat,
        tx->txDma->intMask
        );

  r += sprintf(page + r, "== sram contents ==\n");

  r += sprintf(page + r, "ch: bd base:  status:current bd content\n");

  r += sprintf(page + r, "rx:%08lx:%08lx:%08lx:%08lx\n",
        rx_sram->baseDescPtr,
        rx_sram->state_data,
        rx_sram->desc_len_status,
        rx_sram->desc_base_bufptr
        );

  r += sprintf(page + r, "tx:%08lx:%08lx:%08lx:%08lx\n\n",
        tx_sram->baseDescPtr,
        tx_sram->state_data,
        tx_sram->desc_len_status,
        tx_sram->desc_base_bufptr
        );

  r += sprintf(page + r, "== MIPS and MISC registers ==\n");
  r += sprintf(page + r, "CP0 cause:        %08lx\n",   read_c0_cause() & CAUSEF_IP);
  r += sprintf(page + r, "CP0 status:       %08x\n",    read_c0_status());
  //  r += sprintf(page + r, "PERF->IrqMask:    %08lx\n",   PERF->IrqMask);
  //  r += sprintf(page + r, "PERF->IrqStatus:  %08lx\n\n", PERF->IrqStatus);
 
  *eof = 1;
  return r;
}

int bcmenet_del_proc_files(struct net_device *dev)
{
  char tmp[32];

  sprintf(tmp, "driver/%s/rxbd", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s/txbd0", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s/txbd1", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s/dma_summary", dev->name);
  remove_proc_entry(tmp, NULL);

  sprintf(tmp, "driver/%s", dev->name);
  remove_proc_entry(tmp, NULL);
  return 0;
}

int bcmenet_add_proc_files(struct net_device *dev)
{
  char tmp[32];
  struct proc_dir_entry *parentdir;
  BcmEnet_devctrl *p = netdev_priv(dev);

  sprintf(tmp, "driver/%s", dev->name);
  parentdir = proc_mkdir (tmp, NULL);

  sprintf(tmp, "driver/%s/rxbd", dev->name);
  proc_create_data("rxbd", 0, parentdir, &rxbd_fops, p);

  sprintf(tmp, "driver/%s/txbd0", dev->name);
  create_proc_read_entry(tmp, 0, NULL, proc_get_txbd0, p);

  sprintf(tmp, "driver/%s/txbd1", dev->name);
  create_proc_read_entry(tmp, 0, NULL, proc_get_txbd1, p);

  sprintf(tmp, "driver/%s/dma_summary", dev->name);
  create_proc_read_entry(tmp, 0, NULL, proc_get_dma_summary, p);
  return 0;
}

/* End of file */

