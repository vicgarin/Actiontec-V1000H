/*
    Copyright 2000-2011 Broadcom Corporation

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
#include <bcm_map_part.h>
#include <linux/device.h>
#include <bcmSpiRes.h>
#include <spidevices.h>
#include <board.h>
#include <boardparms.h>
#include <linux/mii.h>

/***************************************************************************
* File Name  : spidevices.c
*
* Description: This file contains the functions for communicating between a brcm
*              cpe chip(63268) to another brcm cpe chip(6368) which is connected 
*              as a spi slave device.
*
***************************************************************************/

/*********************************************************************************************************
 * Eg. configuration required for spi slave devices
 * 
 * 6368: BcmSpiReserveSlave2(HS_SPI_BUS_NUM, 7, 781000, SPI_MODE_3, SPI_CONTROLLER_STATE_GATE_CLK_SSOFF);
 *
 *
 **********************************************************************************************************/

extern spinlock_t bcm_gpio_spinlock;

#define BCM_SPI_SLAVE_ID     1
#define BCM_SPI_SLAVE_FREQ   6250000

static unsigned int bcmSpiSlaveResetGpio = 0xFF;
static unsigned int bcmSpiSlaveBus       = LEG_SPI_BUS_NUM;
static unsigned int bcmSpiSlaveId        = BCM_SPI_SLAVE_ID;
static unsigned int bcmSpiSlaveMaxFreq   = BCM_SPI_SLAVE_FREQ;
static unsigned int bcmSpiSlaveMode      = SPI_MODE_DEFAULT;
static unsigned int bcmSpiSlaveCtrState  = SPI_CONTROLLER_STATE_CPHA_EXT;
static unsigned int bcmSpiSlaveProtoRev  = 1;


// HW SPI supports 2 modes rev0 and rev1 

static int kerSysBcmSpiSlaveInit_rev0(void);
static int kerSysBcmSpiSlaveRead_rev0(unsigned long addr, unsigned long *data, unsigned long len);
static int kerSysBcmSpiSlaveWrite_rev0(unsigned long addr, unsigned long data, unsigned long len);
static int kerSysBcmSpiSlaveWriteBuf_rev0(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize);
static int kerSysBcmSpiSlaveInit_rev1(void);
static int kerSysBcmSpiSlaveRead_rev1(unsigned long addr, unsigned long *data, unsigned long len);
static int kerSysBcmSpiSlaveWrite_rev1(unsigned long addr, unsigned long data, unsigned long len);
static int kerSysBcmSpiSlaveWriteBuf_rev1(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize);


static void getSpiSlaveDeviceInfo(void);
static void resetSpiSlaveDevice(void);

typedef int (*spiSlaveInit)( void );
typedef int (*spiSlaveRead)(unsigned long addr, unsigned long *data, unsigned long len);
typedef int (*spiSlaveWrite)(unsigned long addr, unsigned long data, unsigned long len);
typedef int (*spiSlaveWriteBuf)(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize);

typedef struct 
{
    spiSlaveInit slaveInit;
    spiSlaveRead slaveRead;
    spiSlaveWrite slaveWrite;
    spiSlaveWriteBuf slaveWriteBuf;    
} spiSlaveOps;

static spiSlaveOps spiOps[2] = {
                            {kerSysBcmSpiSlaveInit_rev0, kerSysBcmSpiSlaveRead_rev0, kerSysBcmSpiSlaveWrite_rev0, kerSysBcmSpiSlaveWriteBuf_rev0},
                            {kerSysBcmSpiSlaveInit_rev1, kerSysBcmSpiSlaveRead_rev1, kerSysBcmSpiSlaveWrite_rev1, kerSysBcmSpiSlaveWriteBuf_rev1}
                        };
                                

static uint8_t  init_seq_rev0[3] = { 0x11, 0x01, 0xfc };
static uint8_t  init_adr_rev0[8] = { 0x11, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t  init_seq_rev1[3] = { 0x11, 0x01, 0xfd };
static uint8_t  init_cfg_rev1[3] = { 0x11, 0x03, 0x58 };
static uint8_t  init_adr_rev1[7] = { 0x11, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };

static struct mutex bcmSpiSlaveMutex;

static int spi_setup_addr( uint32_t addr, uint32_t len )
{
   uint8_t buf[7];
   int     status;

   if ((addr & ~(len-1)) != addr)
   {
      printk(KERN_ERR "spi_setup_addr: Invalid address - bad alignment\n");
      return(-1);
   }

   buf[0] = 0x11;
   buf[1] = 0x01;
   buf[2] = ((1 << len) - 1) << ((4 - len) - (addr & 3));
   buf[3] = (uint8_t)(addr >> 0);
   buf[4] = (uint8_t)(addr >> 8);
   buf[5] = (uint8_t)(addr >> 16);
   buf[6] = (uint8_t)(addr >> 24);

   status = BcmSpiSyncTrans(buf, NULL, 0, 7, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "spi_setup_addr: BcmSpiSyncTrans error\n");
      return(-1);
   }

   return(0);
}

static int spi_read_status(uint8_t *data)
{
   uint8_t read_status[2] = {0x10, 0x00};
   int     status;

   status = BcmSpiSyncTrans(read_status, &read_status[0], 2, 1, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "spi_read_status: BcmSpiSyncTrans returned error\n");
      *data = read_status[0];
      return(-1);
   }

   *data = read_status[0];
   return(0);
}

static int kerSysBcmSpiSlaveRead_rev0(unsigned long addr, unsigned long *data, unsigned long len)
{
   uint8_t buf[4] = { 0, 0, 0, 0 };
   int     status;

   *data = 0;
   mutex_lock(&bcmSpiSlaveMutex);

   addr &= 0x1fffffff;
   spi_setup_addr( addr, len );

   buf[0] = 0x12;
   buf[1] = (uint8_t)(addr >> 0);
   status = BcmSpiSyncTrans(&buf[0], &buf[0], 2, len, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncTrans returned error\n");
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }
   
   *data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
   *data >>= ((4 - len) * 8);

   if((spi_read_status(&buf[0]) == -1) || (buf[0] & 0x0f))
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveRead: spi_read_status returned error - %02x\n", buf[0]);
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }
   
   mutex_unlock(&bcmSpiSlaveMutex);

   return(0);
}

static int kerSysBcmSpiSlaveWrite_rev0(unsigned long addr, unsigned long data, unsigned long len)
{
   uint8_t buf[6];
   int     status;

   mutex_lock(&bcmSpiSlaveMutex);

   addr &= 0x1fffffff;
   if(spi_setup_addr(addr, len) == -1)
   {
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }
   
   data <<= 8 * (4 - len);

   buf[0] = 0x13;
   buf[1] = addr & 0xff;
   buf[2] = data >> 24;
   buf[3] = data >> 16;
   buf[4] = data >> 8;
   buf[5] = data >> 0;
   status = BcmSpiSyncTrans(buf, NULL, 0, 2 + len, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: BcmSpiSyncTrans returned error\n");
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   if((spi_read_status(buf) == -1) || (buf[0] & 0x0f))
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: spi_read_status returned error - %02x\n", buf[0]);
      mutex_unlock(&bcmSpiSlaveMutex);
      return(-1);
   }

   mutex_unlock(&bcmSpiSlaveMutex);

   return(0);
}


static int kerSysBcmSpiSlaveWriteBuf_rev0(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize)
{
   int            status = SPI_STATUS_ERR;
   int            maxSize;
   unsigned char *pWriteData;
   unsigned long  nBytes = 0;
   unsigned long  length = len;   

   maxSize    = BcmSpi_GetMaxRWSize(bcmSpiSlaveBus, 0);  
   maxSize   -= 2;
   maxSize   &= ~(unitSize - 1);
   pWriteData = kmalloc(maxSize+2, GFP_KERNEL);
   if ( NULL == pWriteData )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: Out of memory\n");
      return(SPI_STATUS_ERR);
   }

   mutex_lock(&bcmSpiSlaveMutex);

   addr &= 0x1fffffff;
   while ( length > 0 )
   {
      if(spi_setup_addr(addr, unitSize) == -1)
      {
         mutex_unlock(&bcmSpiSlaveMutex);
         status = SPI_STATUS_ERR;
         goto out;
      }
      
      nBytes        = (length > maxSize) ? maxSize : length;
      pWriteData[0] = 0x13;
      pWriteData[1] = addr & 0xff;
      memcpy(&pWriteData[2], data, nBytes);
      status = BcmSpiSyncTrans(&pWriteData[0], NULL, 0, nBytes+2, bcmSpiSlaveBus, bcmSpiSlaveId);
      if ( SPI_STATUS_OK != status )
      {
         printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: BcmSpiSyncTrans returned error\n");
         status = SPI_STATUS_ERR;
         goto out;
      }
      
      if((spi_read_status(pWriteData) == -1) || (pWriteData[0] & 0x0f))
      {
         printk(KERN_ERR "kerSysBcmSpiSlaveWrite: spi_read_status returned error - %02x\n", pWriteData[0]);
         status = SPI_STATUS_ERR;
         goto out;
      }
      addr    = (unsigned int)addr + nBytes;
      data    = (unsigned long *)((unsigned long)data + nBytes);
      length -= nBytes;
   }
   
out:
   mutex_unlock(&bcmSpiSlaveMutex);
   kfree(pWriteData);

   return( status );
}

static int kerSysBcmSpiSlaveRead_rev1(unsigned long addr, unsigned long *data, unsigned long len)
{
   struct spi_transfer xfer[2];
   uint8_t buf_0[3]  = { 0 };
   uint8_t buf_1[20] = { 0 };
   int     status;
   int     i;

   *data = 0;
   switch ( len )
   {  
      /* a read includes up to 10 status bytes,
         if read completes with one status byte slave will
         start reading next address. Disable address auto
         increment to avoid memory faults */
      case 1: buf_0[2] = 0x00 | 0x08; break;
      case 2: buf_0[2] = 0x20 | 0x08; break;
      case 4: buf_0[2] = 0x40 | 0x08; break;
      default: return(SPI_STATUS_INVALID_LEN);
   }

   memset(xfer, 0, (sizeof xfer));
   
   buf_0[0]         = 0x11;
   buf_0[1]         = 0x03;
   xfer[0].len      = 3;
   xfer[0].speed_hz = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf   = &buf_0[0];
   xfer[0].cs_change = 1;

   addr            &= 0x1fffffff;
   buf_1[0]         = 0x10;
   buf_1[1]         = 0xC0 | ((addr >>  0) & 0x3f);
   buf_1[2]         = 0x80 | ((addr >>  6) & 0x7f);
   buf_1[3]         = 0x80 | ((addr >> 13) & 0x7f);
   buf_1[4]         = 0x80 | ((addr >> 20) & 0x7f);
   buf_1[5]         = 0x00 | ((addr >> 27) & 0x1f);
   xfer[1].len      = 20; /* 6 cmd bytes, 1-4 data bytes, 10-13 status bytes */
   xfer[1].speed_hz = bcmSpiSlaveMaxFreq;
   xfer[1].tx_buf   = &buf_1[0];
   xfer[1].rx_buf   = &buf_1[0];
   xfer[1].cs_change = 1;

   status = BcmSpiSyncMultTrans(&xfer[0], 2, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveRead: BcmSpiSyncMultTrans returned error\n");
      return(SPI_STATUS_ERR);
   }

   /* there can be up to 10 status bytes starting at index 6 */
   for ( i = 6; i < (20 - len); i++)
   {
      if ( 0x01 & buf_1[i] )
      {
         /* completed successfully */
         switch ( len )
         {
            case 1: 
               *data = buf_1[i+1]; 
               break;
            case 2: 
               *data = (buf_1[i+1] << 8) | buf_1[i+2];
               break;
            case 4:
            default: 
               *data = (buf_1[i+1] << 24) | (buf_1[i+2] << 16) | 
                       (buf_1[i+3] <<  8) | buf_1[i+4];
               break;
         }
         return SPI_STATUS_OK;
      }
      else if ( 0x02 & buf_1[i] )
      {
          buf_0[0]         = 0x10;
          buf_0[1]         = 0x02;
          xfer[0].len      = 3;
          xfer[0].speed_hz = bcmSpiSlaveMaxFreq;
          xfer[0].tx_buf   = &buf_0[0];
          xfer[0].rx_buf   = &buf_0[0];

          BcmSpiSyncMultTrans(&xfer[0], 1, bcmSpiSlaveBus, bcmSpiSlaveId);
          printk(KERN_ERR "kerSysBcmSpiSlaveRead: SPI error: %x\n", buf_0[2] );
          return SPI_STATUS_ERR;
      }
   }

   /* read did not complete - read status register and return error 
      note that number of status bytes read should prevent this from happening */
   buf_0[0]         = 0x10;
   buf_0[1]         = 0x02;
   xfer[0].len      = 3;
   xfer[0].speed_hz = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf   = &buf_0[0];
   xfer[0].rx_buf   = &buf_0[0];
   BcmSpiSyncMultTrans(&xfer[0], 1, bcmSpiSlaveBus, bcmSpiSlaveId);
   printk(KERN_ERR "kerSysBcmSpiSlaveRead: SPI timeout: %x\n", buf_0[2] );

   return( SPI_STATUS_ERR );
   
}


static int kerSysBcmSpiSlaveWrite_rev1(unsigned long addr, unsigned long data, unsigned long len)
{
   struct spi_transfer xfer[3];
   int                 status;
   uint8_t buf_0[3]  = { 0 };
   uint8_t buf_1[10] = { 0 };
   uint8_t buf_2[3]  = { 0 };

   switch ( len )
   {
      case 1: buf_0[2] = 0x00; break;
      case 2: buf_0[2] = 0x20; break;
      case 4: buf_0[2] = 0x40; break;
      default: return(SPI_STATUS_INVALID_LEN);
   }

   memset(xfer, 0, (sizeof xfer));
   buf_0[0]          = 0x11;
   buf_0[1]          = 0x03;
   xfer[0].len       = 3;
   xfer[0].speed_hz  = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf    = &buf_0[0];
   xfer[0].cs_change = 1;

   addr               &= 0x1fffffff;
   data              <<= 8 * (4 - len);
   buf_1[0]            = 0x11;
   buf_1[1]            = 0xC0 | ((addr >>  0) & 0x3f);
   buf_1[2]            = 0x80 | ((addr >>  6) & 0x7f);
   buf_1[3]            = 0x80 | ((addr >> 13) & 0x7f);
   buf_1[4]            = 0x80 | ((addr >> 20) & 0x7f);
   buf_1[5]            = 0x00 | ((addr >> 27) & 0x1f);
   buf_1[6]            = data >> 24;
   buf_1[7]            = data >> 16;
   buf_1[8]            = data >> 8;
   buf_1[9]            = data >> 0;
   xfer[1].len         = 6 + len;
   xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[1].tx_buf      = &buf_1[0];
   xfer[1].rx_buf      = &buf_1[0];
   xfer[1].cs_change   = 1;
   xfer[1].delay_usecs = 10; /* delay to allow write to complete */

   buf_2[0]          = 0x10;
   buf_2[1]          = 0x02;
   xfer[2].len       = 3;
   xfer[2].speed_hz  = bcmSpiSlaveMaxFreq;
   xfer[2].tx_buf    = &buf_2[0];
   xfer[2].rx_buf    = &buf_2[0];
   xfer[2].cs_change = 1;
 
   status = BcmSpiSyncMultTrans(&xfer[0], 3, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: BcmSpiSyncMultTrans returned error\n");
      return(status);
   }

   if ( buf_2[2] != 0 )
   {
      /* transfer timed out or there was an error */
      printk(KERN_ERR "kerSysBcmSpiSlaveWrite: SPI error: %x\n", buf_2[2] );
      return SPI_STATUS_ERR;
   }

   return SPI_STATUS_OK;

}


static int kerSysBcmSpiSlaveWriteBuf_rev1(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize)
{
   struct spi_transfer xfer[8];
   uint8_t             buf_0[3] = { 0 };
   uint8_t             buf_1[3] = { 0 };
   int                 status;
   unsigned long       length   = len;
   unsigned long       nBytes   = 0;
   int                 maxSize;
   unsigned char      *pWriteData;

   switch ( unitSize )
   {
      case 1: buf_0[2] = 0x00; break;
      case 2: buf_0[2] = 0x20; break;
      case 4: buf_0[2] = 0x40; break;
      default: return(SPI_STATUS_INVALID_LEN);
   }

 
   maxSize    = BcmSpi_GetMaxRWSize(bcmSpiSlaveBus, 0); // No Autobuffer
   maxSize   -= 6;      /* account for command bytes */
   maxSize   &= ~(unitSize - 1);
   pWriteData = kmalloc(maxSize+6, GFP_KERNEL);
   if ( NULL == pWriteData )
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: Out of memory\n");
      return(SPI_STATUS_ERR);
   }
   
   memset(&xfer[0], 0, sizeof(struct spi_transfer)*8);
   addr &= 0x1fffffff;
   while ( length > 0 )
   {
      buf_0[0]          = 0x11;
      buf_0[1]          = 0x03;
      xfer[0].len       = 3;
      xfer[0].speed_hz  = bcmSpiSlaveMaxFreq;
      xfer[0].tx_buf    = &buf_0[0];
      xfer[0].cs_change = 1;

      nBytes              = (length > maxSize) ? maxSize : length;
      pWriteData[0]            = 0x11;
      pWriteData[1]            = 0xC0 | ((addr >>  0) & 0x3f);
      pWriteData[2]            = 0x80 | ((addr >>  6) & 0x7f);
      pWriteData[3]            = 0x80 | ((addr >> 13) & 0x7f);
      pWriteData[4]            = 0x80 | ((addr >> 20) & 0x7f);
      pWriteData[5]            = 0x00 | ((addr >> 27) & 0x1f);
      memcpy(&pWriteData[6], data, nBytes);
      xfer[1].len         = 6 + nBytes;
      xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
      xfer[1].tx_buf      = pWriteData;
      xfer[1].cs_change   = 1;
      xfer[1].delay_usecs = 30; /* delay to allow write to complete */

      buf_1[0]          = 0x10;
      buf_1[1]          = 0x02;
      xfer[2].len       = 3;
      xfer[2].speed_hz  = bcmSpiSlaveMaxFreq;
      xfer[2].tx_buf    = &buf_1[0];
      xfer[2].rx_buf    = &buf_1[0];
      xfer[2].cs_change = 1;

      status = BcmSpiSyncMultTrans(&xfer[0], 3, bcmSpiSlaveBus, bcmSpiSlaveId);
      if ( SPI_STATUS_OK != status )
      {
         printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: BcmSpiSyncMultTrans returned error\n");
         kfree(pWriteData);
         return(status);
      }

      if ( buf_1[2] != 0 )
      {
         /* transfer timed out or there was an error */
         printk(KERN_ERR "kerSysBcmSpiSlaveWriteBuf: SPI error: %x\n", buf_1[2] );
         kfree(pWriteData);
         return SPI_STATUS_ERR;
      }
      addr    = (unsigned int)addr + nBytes;
      data    = (unsigned long *)((unsigned long)data + nBytes);
      length -= nBytes;
   }

   kfree(pWriteData);
   return SPI_STATUS_OK;
}

unsigned long kerSysBcmSpiSlaveReadReg32(unsigned long addr)
{
   unsigned long data = 0;

   BUG_ON(addr & 3);
   addr &= 0x1fffffff;
   
   if(kerSysBcmSpiSlaveRead(addr, &data, 4) < 0)
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveReadReg32: can't read %08x\n", (unsigned int)addr);
   }

   return(data);
}

void kerSysBcmSpiSlaveWriteReg32(unsigned long addr, unsigned long data)
{
   BUG_ON(addr & 3);
   addr &= 0x1fffffff;

   if(kerSysBcmSpiSlaveWrite(addr, data, 4) < 0)
   {
      printk(KERN_ERR "kerSysBcmSpiSlaveWriteReg32: can't write %08x (data %08x)\n", (unsigned int)addr, (unsigned int)data);
   }

}

 
static void resetSpiSlaveDevice(void)
{
   // unsigned long flags;
    
    printk(KERN_ERR "Entering %s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, bcmSpiSlaveResetGpio);  

    if ( bcmSpiSlaveResetGpio != 0xFF )
    {
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioInactive);
        mdelay(1);
        kerSysSetGpioState(bcmSpiSlaveResetGpio,  kGpioActive); 
        mdelay(350);
    }
  
}

static void getSpiSlaveDeviceInfo(void)
{ 
    unsigned short gpio;
    unsigned short slaveId;
    unsigned short slaveBus;
    unsigned short slaveMode;  
    unsigned long  ctrlState;  
    unsigned long  slaveMaxFreq;     
    unsigned short protoRev;
    
    if ( BpGetSpiSlaveResetGpio(&gpio) == BP_SUCCESS ) 
    {
        bcmSpiSlaveResetGpio = gpio;
        printk(KERN_INFO "%s: bcmSpiSlaveResetGpio = %d\n", __FUNCTION__, bcmSpiSlaveResetGpio);        
    }

    if ( BpGetSpiSlaveSelectNum(&slaveId) == BP_SUCCESS ) 
    {
        bcmSpiSlaveId = slaveId;
        printk(KERN_INFO"%s: bcmSpiSlaveId = %d\n", __FUNCTION__, bcmSpiSlaveId);          
    }
    
    if ( BpGetSpiSlaveBusNum(&slaveBus) == BP_SUCCESS ) 
    {
        bcmSpiSlaveBus = slaveBus;
        printk(KERN_INFO "%s: bcmSpiSlaveBus = %d\n", __FUNCTION__, bcmSpiSlaveBus);           
    }
    
    if ( BpGetSpiSlaveMode(&slaveMode) == BP_SUCCESS ) 
    {
        bcmSpiSlaveMode = slaveMode;
        printk(KERN_INFO "%s: bcmSpiSlaveMode = %d\n", __FUNCTION__, bcmSpiSlaveMode);            
    }

    if ( BpGetSpiSlaveCtrlState(&ctrlState) == BP_SUCCESS ) 
    {
        bcmSpiSlaveCtrState = ctrlState;
        printk(KERN_INFO "%s: bcmSpiSlaveCtrState = 0x%x\n", __FUNCTION__, bcmSpiSlaveCtrState);           
    }
    
    if ( BpGetSpiSlaveMaxFreq(&slaveMaxFreq) == BP_SUCCESS ) 
    {
        bcmSpiSlaveMaxFreq = slaveMaxFreq;
        printk(KERN_INFO "%s: bcmSpiSlaveMaxFreq = %d\n", __FUNCTION__, bcmSpiSlaveMaxFreq);          
    }     
    
    if ( BpGetSpiSlaveProtoRev(&protoRev) == BP_SUCCESS ) 
    {
        bcmSpiSlaveProtoRev = protoRev;
        printk(KERN_INFO "%s: bcmSpiSlaveProtoRev = %d\n", __FUNCTION__, bcmSpiSlaveProtoRev);                  
    }       
}

int kerSysBcmSpiSlaveInit_rev0( void )
{
    unsigned long data;
    int32_t       retVal = 0;
    int           status;
    struct spi_transfer xfer[2];
   
    mutex_init(&bcmSpiSlaveMutex);

    status = BcmSpiReserveSlave2(bcmSpiSlaveBus, bcmSpiSlaveId, bcmSpiSlaveMaxFreq, bcmSpiSlaveMode, bcmSpiSlaveCtrState);
    if ( SPI_STATUS_OK != status )
    {
      printk(KERN_ERR "%s: BcmSpiReserveSlave2 returned error %d\n", __FUNCTION__, status);
      return(SPI_STATUS_ERR);
    }
  
    memset(xfer, 0, (sizeof xfer));
    xfer[0].len         = 3;
    xfer[0].speed_hz    = bcmSpiSlaveMaxFreq;
    xfer[0].tx_buf      = &init_seq_rev0[0];
    xfer[0].cs_change   = 1;
    xfer[0].delay_usecs = 10;
    
    xfer[1].len         = 8;
    xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
    xfer[1].tx_buf      = &init_adr_rev0[0];
    xfer[1].cs_change   = 1;
    xfer[1].delay_usecs = 10;
    
    status = BcmSpiSyncMultTrans(&xfer[0], 2, bcmSpiSlaveBus, bcmSpiSlaveId);
    if ( SPI_STATUS_OK != status )
    {
      printk(KERN_ERR "%s: BcmSpiSyncMultTrans returned error\n", __FUNCTION__);
      return(SPI_STATUS_ERR);
    }
    
    if ((kerSysBcmSpiSlaveRead(0x10000000, &data, 4) == -1) ||
       (data == 0) || (data == 0xffffffff))
    {   
      printk(KERN_ERR "%s: Failed to read the Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
      return -1;
    }
    else
    {
      printk(KERN_INFO "%s: Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
    }

    
    return( retVal );

}

int kerSysBcmSpiSlaveInit_rev1( void )
{
   unsigned long data;
   int32_t       retVal = 0;
   int           status;
   struct spi_transfer xfer[3];

#if defined(CONFIG_BCM96816)
   uint32        miscStrapBus; 
   unsigned long flags;

   /* the 6829 should only be reset for the BHR board and not the BHRGR board
      However, the BHRGR board does not use GPIO1 so running this code is okay */
   spin_lock_irqsave(&bcm_gpio_spinlock, flags);
   GPIO->GPIODir  |=  2;
   GPIO->GPIOio   &= ~2;
   mdelay(1);
   GPIO->GPIOio   |=  2;
   mdelay(350);
   GPIO->GPIODir  &=  ~2;
   spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);

   miscStrapBus = MISC->miscStrapBus;
   if ( miscStrapBus & MISC_STRAP_BUS_LS_SPIM_ENABLED )
   {
      bcmSpiSlaveBus = LEG_SPI_BUS_NUM;
   }
   else
   {
      bcmSpiSlaveBus = HS_SPI_BUS_NUM;
   }
  status =  BcmSpiReserveSlave2(bcmSpiSlaveBus, BCM_SPI_SLAVE_ID, BCM_SPI_SLAVE_FREQ, SPI_MODE_DEFAULT, SPI_CONTROLLER_STATE_CPHA_EXT); 
#else
   status = BcmSpiReserveSlave2(bcmSpiSlaveBus, bcmSpiSlaveId, bcmSpiSlaveMaxFreq, bcmSpiSlaveMode, bcmSpiSlaveCtrState);
#endif   
   if ( SPI_STATUS_OK != status )
   {
       printk(KERN_ERR "%s: BcmSpiSyncMultTrans returned error\n", __FUNCTION__);
       return(SPI_STATUS_ERR);
   }

   memset(xfer, 0, (sizeof xfer));
   xfer[0].len         = 3;
   xfer[0].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[0].tx_buf      = &init_seq_rev1[0];
   xfer[0].cs_change   = 1;
   xfer[0].delay_usecs = 10;
   
   xfer[1].len         = 3;
   xfer[1].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[1].tx_buf      = &init_cfg_rev1[0];
   xfer[1].cs_change   = 1;
   xfer[1].delay_usecs = 10;
   
   xfer[2].len         = 7;
   xfer[2].speed_hz    = bcmSpiSlaveMaxFreq;
   xfer[2].tx_buf      = &init_adr_rev1[0];
   xfer[2].cs_change   = 1;
   xfer[2].delay_usecs = 10;

   status = BcmSpiSyncMultTrans(&xfer[0], 3, bcmSpiSlaveBus, bcmSpiSlaveId);
   if ( SPI_STATUS_OK != status )
   {
      printk(KERN_ERR "%s: BcmSpiSyncMultTrans returned error\n", __FUNCTION__);
      return(SPI_STATUS_ERR);
   }

   if ((kerSysBcmSpiSlaveRead(0x10000000, &data, 4) == -1) ||
       (data == 0) || (data == 0xffffffff))
   {   
      printk(KERN_ERR "%s: Failed to read the Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
      return -1;
   }
   else
   {
      printk(KERN_INFO "%s: Chip ID: 0x%08x\n", __FUNCTION__, (unsigned int)data);
   }


   return( retVal );

}

int kerSysBcmSpiSlaveInit( void )
{
    getSpiSlaveDeviceInfo();             
    resetSpiSlaveDevice();             

    return spiOps[bcmSpiSlaveProtoRev].slaveInit();
}

int kerSysBcmSpiSlaveRead(unsigned long addr, unsigned long * data, unsigned long len)
{
    return spiOps[bcmSpiSlaveProtoRev].slaveRead(addr, data, len);
}

int kerSysBcmSpiSlaveWrite(unsigned long addr, unsigned long data, unsigned long len)
{
    return spiOps[bcmSpiSlaveProtoRev].slaveWrite(addr, data, len);
}

int kerSysBcmSpiSlaveWriteBuf(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize)
{
    return spiOps[bcmSpiSlaveProtoRev].slaveWriteBuf(addr, data, len, unitSize);
}


#if defined(CONFIG_BCM96816)  

#define REG_MDIO_CTRL_WRITE                       (1 << 31)
#define REG_MDIO_CTRL_READ                        (1 << 30)
#define REG_MDIO_CTRL_EXT                         (1 << 16)
#define REG_MDIO_CTRL_EXT_BIT(id)                 (((id) >= 0x10) ? REG_MDIO_CTRL_EXT: 0)
#define REG_MDIO_CTRL_ID_SHIFT                    25
#define REG_MDIO_CTRL_ID_MASK                     (0x1f << REG_MDIO_CTRL_ID_SHIFT)
#define REG_MDIO_CTRL_ADDR_SHIFT                  20
#define REG_MDIO_CTRL_ADDR_MASK                   (0x1f << REG_MDIO_CTRL_ADDR_SHIFT)
static void ethswMdioRead6829(int phy_id, int reg, uint16_t *data)
{
    unsigned long reg_value;
    
    reg_value = 0;
    kerSysBcmSpiSlaveWrite(0xb0e000b0, 0, 4);
    reg_value = (REG_MDIO_CTRL_EXT_BIT(phy_id)) | (REG_MDIO_CTRL_READ) |
        (phy_id << REG_MDIO_CTRL_ID_SHIFT) | (reg  << REG_MDIO_CTRL_ADDR_SHIFT);
    kerSysBcmSpiSlaveWrite(0xb0e000b0, reg_value, 4);
    udelay(50);
    kerSysBcmSpiSlaveRead(0xb0e000b4, &reg_value, 2);
    *data = (uint16_t)reg_value;
    
}


static void ethswMdioWrite6829(int phy_id, int reg, uint16_t *data)
{
    unsigned long reg_value;

    reg_value = 0;
    kerSysBcmSpiSlaveWrite(0xb0e000b0, 0, 4);
    reg_value = (REG_MDIO_CTRL_EXT_BIT(phy_id)) | (REG_MDIO_CTRL_WRITE) |
        (phy_id << REG_MDIO_CTRL_ID_SHIFT) | (reg  << REG_MDIO_CTRL_ADDR_SHIFT) |
        *data;
    kerSysBcmSpiSlaveWrite(0xb0e000b0, reg_value, 4);
    mdelay(1);
    
}

   
void board_Init6829( void )
{
   unsigned char     portInfo6829;
   int               retVal;
   unsigned long     data;
   unsigned long     data2;
   ETHERNET_MAC_INFO EnetInfo;
   int               i;

    /* disable interfaces on the 6829 that are not being used */
   retVal = BpGet6829PortInfo(&portInfo6829);
   if ( (BP_SUCCESS == retVal) && (0 != portInfo6829))
   {
      /* intialize SPI access to the 6829 chip */
      kerSysBcmSpiSlaveInit();    

      /* initialize the switch */
      data = 0;
      kerSysBcmSpiSlaveWrite(0xb00000bc, data, 4);
      mdelay(1);
      
      data = GPHY_PWR_DOWN_1 | GPHY_PWR_DOWN_SD_1;
      kerSysBcmSpiSlaveWrite(0xb00000bc, data, 4);
      mdelay(1);
      
      data |= (RSW_MII_DUMB_FWDG_EN | RSW_HW_FWDG_EN);
      kerSysBcmSpiSlaveWrite(0xb00000bc, data, 4);
      mdelay(1);

      kerSysBcmSpiSlaveRead(0xb0000010, &data, 4);
      data &= ~SOFT_RST_SWITCH;
      kerSysBcmSpiSlaveWrite(0xb0000010, data, 4);
      mdelay(1);
      data |= SOFT_RST_SWITCH;
      kerSysBcmSpiSlaveWrite(0xb0000010, data, 4);
      mdelay(1);

      portInfo6829 &= ~0x80;
      if ( BpGetEthernetMacInfo(&EnetInfo, 1) == BP_SUCCESS )
      {
         int      phy_id;
         uint16_t mdioData;

         for (i = 0; i < 2; i++)
         {
            phy_id = EnetInfo.sw.phy_id[i];
            if ((portInfo6829 & (1 << i)) != 0)
            {
               /*reset MII */
               ethswMdioRead6829(phy_id, MII_BMCR, &mdioData);
               mdioData |= BMCR_RESET;
               ethswMdioWrite6829(phy_id, MII_BMCR, &mdioData);

               // Restart auto-negotiation
               ethswMdioRead6829(phy_id, MII_BMCR, &mdioData);
               mdioData |= BMCR_ANRESTART;
               ethswMdioWrite6829(phy_id, MII_BMCR, &mdioData);
            }
            else
            {
               ethswMdioRead6829(phy_id, MII_BMCR, &mdioData);
               mdioData |= BMCR_PDOWN;
               ethswMdioWrite6829(phy_id, MII_BMCR, &mdioData);
            }
         }
      }    

      /* MoCA reset */      
      kerSysBcmSpiSlaveRead(0xb000180C, &data, 4);
      data &= ~(MISC_MOCA_RST_REF_DIV2RST | MISC_MOCA_RST_REF_FBDIVRST);
      kerSysBcmSpiSlaveWrite(0xb000180C, data, 4);
      data &= ~MISC_MOCA_RST_REF_VCRST;
      kerSysBcmSpiSlaveWrite(0xb000180C, data, 4);
      data &= ~(MISC_MOCA_RST_REF_OUTDIV_RESET_M_MASK | MISC_MOCA_RST_REF_MDIV2RST);
      kerSysBcmSpiSlaveWrite(0xb000180C, data, 4);

      kerSysBcmSpiSlaveRead(0xb0001804, &data2, 4);
      data2 |= (7 << MISC_MOCA_CTL_REF_QP_ICTRL_SHIFT);
      kerSysBcmSpiSlaveWrite(0xb0001804, data2, 4);
      
      data &= ~MISC_MOCA_RST_REF_LD_RESET_STRT;
      kerSysBcmSpiSlaveWrite(0xb000180C, data, 4);     

      /* Set blink rate for hardware LEDs. */
      kerSysBcmSpiSlaveRead(0xb0000090, &data, 4);
      data &= ~LED_INTERVAL_SET_MASK;
      kerSysBcmSpiSlaveWrite(0xb0000090, data, 4);
      data |= LED_INTERVAL_SET_80MS;
      kerSysBcmSpiSlaveWrite(0xb0000090, data, 4);

      /* enable required ports */
      kerSysBcmSpiSlaveWrite(0xb0e00000, 0x00, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00004, 0x00, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00005, 0x00, 1);
      /* disable all other ports */
      kerSysBcmSpiSlaveWrite(0xb0e00001, 0x03, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00002, 0x03, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00003, 0x03, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00006, 0x03, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00007, 0x03, 1);
      kerSysBcmSpiSlaveWrite(0xb0e00008, 0x03, 1);

      for ( i = 0; i < 8; i++ )
      {
         kerSysBcmSpiSlaveWrite(0xb0e00058 + i, 0x40, 1);
      }

      /* setup learning */
      kerSysBcmSpiSlaveWrite(0xb0e0003c, 0x1ff /*0x0190*/, 2);

      // Reset MIB counters
      kerSysBcmSpiSlaveWrite(0xb0e00200, 1, 1);
      udelay(100);
      kerSysBcmSpiSlaveWrite(0xb0e00200, 0, 1);
     
      /* force GPON laser off */
      kerSysBcmSpiSlaveWrite(0xb0004848, GPON_SERDES_LASERMODE_FORCE_OFF, 4);
      kerSysBcmSpiSlaveWrite(0xb0004850, 5, 4);

      /* setup GPIOs */
      kerSysBcmSpiSlaveWrite(0xb0000098, 0, 4);

      /* set bits for sys irq and gphy0 led */
      data  = kerSysBcmSpiSlaveReadReg32(0xb0000098);
      data |= (GPIO_MODE_GPHY0_LED | GPIO_MODE_SYS_IRQ);
      kerSysBcmSpiSlaveWriteReg32(0xb0000098, data);

      /* set direction for sys irq and gphy0 led*/
      data  = kerSysBcmSpiSlaveReadReg32(0xb0000084);
      data |= (GPIO_MODE_GPHY0_LED | GPIO_MODE_SYS_IRQ);
      kerSysBcmSpiSlaveWriteReg32(0xb0000084, data); 

      /* disable all interrupts except for MoCA general interrupt */
      kerSysBcmSpiSlaveWrite(0xb0000020, (1 << 10), 4);
      kerSysBcmSpiSlaveWrite(0xb0000024, 0, 4);  

      /* disable the APM block */
      kerSysBcmSpiSlaveWrite(0xb0008120, 0x01800000, 4);
      kerSysBcmSpiSlaveWrite(0xb0008120, 0x01C00000, 4);
      kerSysBcmSpiSlaveWrite(0xb0008124, 0x03000300, 4);
      kerSysBcmSpiSlaveWrite(0xb0008128, 0x22000000, 4);
      kerSysBcmSpiSlaveWrite(0xb000812C, 0x22000000, 4);
      kerSysBcmSpiSlaveWrite(0xb0008130, 0x22000000, 4);
      kerSysBcmSpiSlaveWrite(0xb0008134, 0x40000000, 4);

      /* disable the USBH */
      data  = kerSysBcmSpiSlaveReadReg32(0xb0002718);
      data &= ~(1 << 3);
      kerSysBcmSpiSlaveWrite(0xb0002718, data, 4);

      data  = kerSysBcmSpiSlaveReadReg32(0xb0002710);
      data &= ~((3<<30) | (3<<28) | (1<<24) | (1<<25));
      kerSysBcmSpiSlaveWrite(0xb0002710, data, 4);

      /* disable DDR */
      // temp -- check strap to make sure boot select is reserved otherwise DDR access hangs
      data  = kerSysBcmSpiSlaveReadReg32(0xb0001814);
      if ( 2 == ((data & MISC_STRAP_BUS_BOOT_SEL_MASK)>>MISC_STRAP_BUS_BOOT_SEL_SHIFT))
      {
         kerSysBcmSpiSlaveWrite(0xb0003238, 0x80000177, 4);
         kerSysBcmSpiSlaveWrite(0xb000333C, 0x800FFFFF, 4);
         kerSysBcmSpiSlaveWrite(0xb000343C, 0x800FFFFF, 4);
         kerSysBcmSpiSlaveWrite(0xb000353C, 0x800FFFFF, 4);
         kerSysBcmSpiSlaveWrite(0xb000363C, 0x800FFFFF, 4);
         kerSysBcmSpiSlaveWrite(0xb0003444, 0x00000001, 4);
         kerSysBcmSpiSlaveWrite(0xb0003204, 0x00000001, 4);
      }      

      /* disable clocks */
      data = SPI_CLK_EN | ROBOSW_CLK_EN | GPON_SER_CLK_EN;
      kerSysBcmSpiSlaveWrite(0xb0000004, data, 4);
      udelay(500); 
   }
 
}
#endif
#if 0
void board_Reset6829( void )
{
   unsigned char portInfo6829;
   int           retVal;
        
   retVal = BpGet6829PortInfo(&portInfo6829);
   if ( (BP_SUCCESS == retVal) && (0 != portInfo6829))
   {
      kerSysBcmSpiSlaveWriteReg32(0xb0000008, 0x1);
      kerSysBcmSpiSlaveWriteReg32(0xb0000008, 0x0);
   }
}
#endif
