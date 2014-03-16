/*
    Copyright 2000-2010 Broadcom Corporation

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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <bcmSpiRes.h>
#include <spidevices.h>
#include <bcm_map.h>
#include <boardparms.h>
#include <bcmenet.h>
#include <bcmmii.h>
#include <linux/nbuff.h>
#include <linux/mii.h>
#include "ethsw.h"
#include "ethsw_phy.h"

extern struct net_device* vnet_dev[MAX_NUM_OF_VPORTS];
extern atomic_t bcm6829ActDevIdx;
extern struct net_device* bcm6829_to_dev[MAX_6829_IFS];
extern struct semaphore bcm_ethlock_switch_config;
extern int phyport_to_vport_6829[MAX_SWITCH_PORTS];
extern void link_change_handler(int port, int linkstatus, int speed, int duplex);

#define BLOCK_ADDR_REG            0x1f
#define BCM_GPON_SERDES_SATA_PHY  1
#define BCM_GPON_SERDES_AE_PHY    2

#define WAIT_LOOP_COUNT           100

#define READ_REG(x)    ((spiDev == 0) ? \
                        (*((volatile unsigned long *)((unsigned long)(x) | KSEG1))) : \
                        (kerSysBcmSpiSlaveReadReg32((unsigned long)(x))))

#define READ_REG16(x, y)    ((spiDev == 0) ? \
                        (*(y) = *((volatile unsigned short *)((unsigned long)(x) | KSEG1))) : \
                        (kerSysBcmSpiSlaveRead((unsigned long)(x), y, 2)))

#define READ_REG8(x, y)    ((spiDev == 0) ? \
                        (*(y) = *((volatile unsigned char *)((unsigned long)(x) | KSEG1))) : \
                        (kerSysBcmSpiSlaveRead((unsigned long)(x), y, 1)))
 
#define WRITE_REG(x,y)     do { ((spiDev == 0) ? \
                               (*((volatile unsigned long *)((unsigned long)(x) | KSEG1))) = (unsigned long)(y) : \
                               kerSysBcmSpiSlaveWriteReg32((unsigned long)(x), (y))); } while(0)

#define WRITE_REG16(x,y)   do { ((spiDev == 0) ? \
                               (*((volatile unsigned short *)((unsigned long)(x) | KSEG1))) = (unsigned short)(y) : \
                               kerSysBcmSpiSlaveWrite((unsigned long)(x), (y), 2)); } while(0)

#define WRITE_REG8(x,y)    do { ((spiDev == 0) ? \
                               (*((volatile unsigned char *)((unsigned long)(x) | KSEG1))) = (unsigned char)(y) : \
                               kerSysBcmSpiSlaveWrite((unsigned long)(x), (y), 1)); } while(0)

static int waitMdioReady( bool spiDev )
{
	int i = 0;

	while ( 1 )
	{
		if ( 0 == (READ_REG(0xb0004854) & 0x1) )
		{
			return( 0 );
		}
		else
		{
			udelay(10);
		}
		i++;
		if ( i >= WAIT_LOOP_COUNT )
		{
			return( -1 );
		}
	}

}

static int waitPllLock(bool spiDev)
{
	uint32_t i;
	int      lock;

   /* Wait for pll_lock=1 */
	lock = 0;
	for (i=1; i<=WAIT_LOOP_COUNT; i++)
	{
		if ((READ_REG(0xb0004814) & 0x1))
		{
			lock = 1;
			break;
		}
		mdelay(1);
	}

	if (lock == 0)
	{
		printk(KERN_ERR "ActEth: PLL locked failed %d, status -- 0x%08lx\n", spiDev, READ_REG(0xb0004814));
	}

	return lock;
}

static int waitRxLockCdr(bool spiDev)
{
	int i;
	int lock=0;

	/* Wait for sapis_rxlock_cdr=1*/
	for ( i=1; i<=WAIT_LOOP_COUNT; i++ )
	{
		if ((READ_REG(0xb0004814) & 0x200))
		{
			lock = 1;
			break;
		}
		udelay(100);
	}

	if (lock == 0)
	{
      printk(KERN_ERR "ActEth: RX CDR locked failed %d, status -- 0x%08lx\n", spiDev, READ_REG(0xb0004814));
   }

	return lock;
}

static int waitTxLockCdr(bool spiDev)
{
	int i;
	int lock=0;

	/* Wait for sapis_rxlock_cdr=1*/
	for ( i=1; i<=WAIT_LOOP_COUNT; i++ )
	{
		if ((READ_REG(0xb0004814) & 0x100))
		{
			lock = 1;
			break;
		}
		udelay(100);
	}

	if (lock == 0)
	{
      printk(KERN_ERR "ActEth: TX CDR locked failed %d, status -- 0x%08lx\n", spiDev, READ_REG(0xb0004814));
   }

	return lock;
}


static void ActiveEthMdioWrite(int phyAddr, uint8_t regAddr, uint16_t data, bool spiDev)
{
	uint32_t regVal;

	if ( waitMdioReady(spiDev) !=0 )
	{
      printk(KERN_ERR "ActEth: MDIO write timeout %d\n", spiDev);
	}

	/* Management Frame Format for MDIO Write:
	* [31:30] = Start of frame (ST) = 2'b01
	* [29:28] = Operation code (R/W) = 2'b01
	* [27:23] = 5-bit PHY address (phyAddr)
	* [22:18] = 5-bit Management Register address (regAddr)
	* [17:16] = Turnaround time (TA) = 2'b10
	* [15: 0] = 16-bit data
	*/
	regVal  = 1 << 30;
	regVal |= (1 << 28);
	regVal |= (phyAddr << 23);
	regVal |= (regAddr << 18);
	regVal |= (2 << 16);
	regVal |= data;
	//GPON_SERDES->mdioWr = regVal;
	WRITE_REG(0x10004818, regVal);
}


static uint32_t ActiveEthMdioRead(int phyAddr, uint8_t regAddr, bool spiDev)
{
	uint32_t rdData;
	uint32_t regVal;

	if (regAddr == BLOCK_ADDR_REG)
	{
		return READ_REG(0x10004818);
	}

	/* Management Frame Format for MDIO Read:
	 * [31:30] = Start of frame (ST) = 2'b01
	 * [29:28] = Operation code (R/W) = 2'b10
	 * [27:23] = 5-bit PHY address (phyAddr)
	 * [22:18] = 5-bit Management Register address (regAddr)
	 * [17:16] = Turnaround time (TA) = 2'b00
	 * [15: 0] = 16-bit data
	 */
	 
	/*Wait till MDIO R/W pending status is clear*/
	if ( waitMdioReady(spiDev) !=0 )
	{
      printk(KERN_ERR "ActEth: MDIO read timeout %d\n", spiDev);
	}

	/* Write followed by read */
	regVal  = 1 << 30;
	regVal |= (2 << 28);
	regVal |= (phyAddr << 23);
	regVal |= (regAddr << 18);
	regVal |= (2 << 16);
	WRITE_REG(0x10004818, regVal);

	/*Wait till MDIO R/W pending status is clear*/
	if ( waitMdioReady(spiDev) !=0 )
	{
      printk(KERN_ERR "ActEth: MDIO read timeout %d\n", spiDev);
	}

	rdData = READ_REG(0x1000481C);

	return rdData;
}


void ActiveEthInit ( bool spiDev )
{
   uint32_t tempReg;
   // Configure SATA PHY to be used for Ethernet 1000Base-X mode
   WRITE_REG(0xb0004808, 0x00d0c003);
   WRITE_REG(0xb0004804, 0x00103b00);
   WRITE_REG(0xb0004804, 0x00003b00);

   tempReg  = READ_REG(0xb0000010);
   tempReg &= ~(1 << 21);
   WRITE_REG(0xb0000010, tempReg);
   udelay(100);
   tempReg |= (1 << 21);
   WRITE_REG(0xb0000010, tempReg);
   udelay(100);

   WRITE_REG(0xb0004810, 0x08401400);
   WRITE_REG(0xb0004818, 0x50fe81c0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50c2a400);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe81b0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50c20000);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe8050);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50f20191);
   waitMdioReady( spiDev );

   tempReg  = READ_REG(0xb0000010);
   tempReg &= ~(1 << 20);
   WRITE_REG(0xb0000010, tempReg);
   udelay(100);
   tempReg |= (1 << 20);
   WRITE_REG(0xb0000010, tempReg);
   udelay(100);

   tempReg  = READ_REG(0xb0000010);
   tempReg &= ~(1 << 22);
   WRITE_REG(0xb0000010, tempReg);
   udelay(100);
   tempReg |= (1 << 22);
   WRITE_REG(0xb0000010, tempReg);
   udelay(100);

   WRITE_REG(0xb0004818, 0x50fe80b0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50c65cc0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe80b0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50e60000);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe81c0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50c6333f);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50ce001b);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe81b0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50ca0367);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe81c0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50ea0400);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004810, 0x08499400);

   waitPllLock(spiDev);
   waitRxLockCdr(spiDev);
   waitTxLockCdr(spiDev);

   WRITE_REG(0xb0004818, 0x50fe8060);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50d60320);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50de0f12);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x517e0000);
   waitMdioReady( spiDev );
   
   WRITE_REG(0xb0004818, 0x51420151);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x517e0000);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x51021000);
   waitMdioReady( spiDev );

   // Low power settings for chip-to-chip connection via Active Ethernet
   WRITE_REG(0xb0004818, 0x50fe8060);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50d6a360);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50e200e1);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50def512);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe00b0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50c64300);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe8060);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50da4124);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe80b0);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50ee0020);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50fe8050);
   waitMdioReady( spiDev );

   WRITE_REG(0xb0004818, 0x50f63b88);
   waitMdioReady( spiDev ); 
}


uint16_t ActiveEthGetStatus( int spiDev )
{
   uint16_t aeStatus;
   
	ActiveEthMdioWrite(BCM_GPON_SERDES_AE_PHY, 0x1F, 0x0, spiDev);
	aeStatus = (uint16_t)ActiveEthMdioRead(BCM_GPON_SERDES_AE_PHY, 0x14, spiDev);

   return aeStatus;

}


/****************************************************************************/
/* Create virtual interfaces for the 6829                                   */ 
/* Inputs: name = active ports on the 6829                                  */ 
/* Returns: 0 on success; a negative value on failure                       */  
/****************************************************************************/
int create_6829_vport(int portMap)
{
   struct net_device *dev;
   struct sockaddr sockaddr;
   BcmEnet_devctrl *pDevCtrl = NULL;
   BcmEnet_devctrl *pDevCtrl0 = netdev_priv(vnet_dev[0]);
   int status;
   int activePorts;
   int i;
   int j;
   int index = 1;
   PHY_STAT phys;

   phys.lnk     = 0;
   phys.fdx     = 0;
   phys.spd1000 = 0;
   phys.spd100  = 0;  

   /* configure the SERDES port as a WAN port. This will prevent
      LAN<->WAN traffic during boot.
      Note that the switch ports must be enabled after wan port 
      configuration has completed (ethsw_init_config handles this) */ 
   pDevCtrl0->wanPort |= (1 << SERDES_PORT_ID);

   /* intialize active ethernet on 6819 and 6829 */
   for ( i = 0; i < 2; i++ )
   {
      ActiveEthInit( i );
   }

   bitcount(activePorts, portMap & ~(BCM_EXT_6829 | BCM_WAN_PORT));
   if (activePorts > MAX_6829_IFS)
   {
      printk(KERN_ERR "create_6829_vport failed - too many ports\n");
      return -EPERM;
   }

   for (i = 0, j = 0; i < activePorts; i++, j++, portMap /= 2)
   {
      /* Skip the switch ports which are not in the port_map */
      while ((portMap % 2) == 0) 
      {
         portMap /= 2;
         j++;
      }

      /* Allocate the dev */
      if ((dev = alloc_etherdev(sizeof(BcmEnet_devctrl))) == NULL)
      {
         return -ENOMEM;
      }
      /* Set the private are to 0s */
      memset(netdev_priv(dev), 0, sizeof(BcmEnet_devctrl));

      /* Set the pDevCtrl->dev to dev */
      pDevCtrl      = netdev_priv(dev);
      pDevCtrl->dev = dev;

      if ( j < EPHY_PORTS )
      {
         dev_alloc_name(dev, "eth1%d");
      }
      else if ( j == MOCA_PORT_ID )
      {
         dev_alloc_name(dev, "moca10");
      }
      else
      {
         dev_alloc_name(dev, dev->name);
      }
 
      SET_MODULE_OWNER(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
      dev->netdev_ops       = vnet_dev[0]->netdev_ops;
#else
      /* Set the dev fields same as the GPON port */
      dev->open             = vnet_dev[0]->open;
      dev->stop             = vnet_dev[0]->stop;
      dev->hard_start_xmit  = (HardStartXmitFuncP)vnet_dev[0]->hard_start_xmit;
      dev->tx_timeout       = vnet_dev[0]->tx_timeout;
      dev->set_mac_address  = vnet_dev[0]->set_mac_address;
      dev->do_ioctl         = vnet_dev[0]->do_ioctl;
      dev->get_stats        = vnet_dev[0]->get_stats;
#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
      dev->priv_flags       = vnet_dev[0]->priv_flags;

      /* Set this flag to block forwarding of traffic between 
         6829 virtual interfaces */
      dev->base_addr         = j;
      pDevCtrl->sw_port_id   = SERDES_PORT_ID;
      if ( IsWanPort(pDevCtrl0->EnetInfo[0].sw.phy_id[SERDES_PORT_ID]) )
         pDevCtrl0->wanPort6829 |= (1 << j);

      /* Set the default tx queue to 0 */
      pDevCtrl->default_txq     = 0;
      pDevCtrl->use_default_txq = 0;

      /* The unregister_netdevice will call the destructor 
         through netdev_run_todo */
      dev->destructor = free_netdev;

      /* add device information to IFDB (netdev_path) */
      netdev_path_set_hw_port(dev, SERDES_PORT_ID, BLOG_ENETPHY);

      status = register_netdev(dev);
      if (status != 0)
      {
          unregister_netdevice(dev);
          return status;
      }

      /* make MoCA port the first device - needed for check_6829_vports */
      if ( MOCA_PORT_ID == j )
      {
         bcm6829_to_dev[0] = dev;
         phyport_to_vport_6829[j] = 0;
      }
      else
      {
         bcm6829_to_dev[index] = dev;
         phyport_to_vport_6829[j] = index;
         index++;
      }

      memmove(dev->dev_addr, vnet_dev[0]->dev_addr, ETH_ALEN);
      memmove(sockaddr.sa_data, vnet_dev[0]->dev_addr, ETH_ALEN);
      dev->set_mac_address(dev, &sockaddr);

      /* init MAC interface */
      ethsw_set_mac(j | BCM_EXT_6829, phys);

      printk("%s: MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
              dev->name,
              dev->dev_addr[0],
              dev->dev_addr[1],
              dev->dev_addr[2],
              dev->dev_addr[3],
              dev->dev_addr[4],
              dev->dev_addr[5]
              );
    }

    return 0;
}


void check_6829_vports( int portMap, int * newStat )
{
   BcmEnet_devctrl   *pPrivDev = NULL;
   struct net_device *pNetDev = NULL;
   BcmEnet_devctrl   *pDevCtrl = netdev_priv(vnet_dev[0]);
   int                mask;
   PHY_STAT           phys;
   int                i;
   int                port;
   static int         mocaStatus = 0;
   int                phy_id;
   uint16             mdioData;
   int                disablePhy = 0;
   int                speed;

   for ( i = 0; i < MAX_6829_IFS; i++ )
   {
      pNetDev  = bcm6829_to_dev[i];
      pPrivDev = (BcmEnet_devctrl *)netdev_priv(pNetDev);
      port     = pNetDev->base_addr;
      mask     = (1 << port) << MAX_SWITCH_PORTS;
      phy_id   = (pDevCtrl->EnetInfo[0].sw.phy_id[port] & ~BCM_WAN_PORT) | BCM_EXT_6829;

      /* moca is always index 0 */
      if ( 0 == i )
      {
         /* moca daemon handles the moca link, just read current status */
         if ( (1 == mocaStatus) && (0 == (pDevCtrl->linkState & mask)) )
         {
            /* moca link went from up to down - enable ethernet */
            disablePhy = 2;
            mocaStatus = 0;
         }
         else if ( (0 == mocaStatus) && (mask == (pDevCtrl->linkState & mask)) ) 
         {
            /* moca link went from down to up - disable ethernet */
            disablePhy = 1;
            mocaStatus = 1;
         }
         else
         {
            /* no change */
            disablePhy = 0;
         }
         if ( 1 == mocaStatus )
            *newStat |= mask;
         else
            *newStat &= ~mask;

         continue;
      }
      else
      {
         phys = ethsw_phy_stat(port | BCM_EXT_6829);

         down(&bcm_ethlock_switch_config);
         /* disablePhy = 0 means nothing has changed */
         if ( 1 == disablePhy )
         {
            /* disable the ethernet phy */
            ethsw_phy_rreg(phy_id, MII_BMCR, &mdioData);
            mdioData |= BMCR_PDOWN;
            ethsw_phy_wreg(phy_id, MII_BMCR, &mdioData);
            phys.lnk = 0; /* force link off */
         }
         else if ( 2 == disablePhy )
         {
            /* enable the ethernet phy */
            ethsw_phy_rreg(phy_id, MII_BMCR, &mdioData);
            mdioData &= ~BMCR_PDOWN;
            ethsw_phy_wreg(phy_id, MII_BMCR, &mdioData);
         }
         up(&bcm_ethlock_switch_config);

         if ( ( (phys.lnk) && (0 == (pDevCtrl->linkState & mask))) ||
              ( (!phys.lnk) && (mask == (pDevCtrl->linkState & mask)) ))
         {
            /* link changed */
            /* MoCA dev has priority update current link state */
            if ( phys.lnk )
            {
               atomic_set(&bcm6829ActDevIdx, i);
               *newStat |= mask;
            }
            else
            {
               atomic_set(&bcm6829ActDevIdx, 0);
               *newStat &= ~mask;
            }

            if ( phys.spd1000 )
               speed = 1000;
            else if ( phys.spd100 )
               speed = 100;
            else
               speed = 10;
      
            link_change_handler(port | BCM_EXT_6829, (phys.lnk ? 1 : 0), speed, 
                                (phys.fdx ? 1 : 0));
         }
      }
   }
}

