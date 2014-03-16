/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom Corporation
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

#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <asm/uaccess.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map.h>
#include "bcmSpiRes.h"
#include <linux/bcm_log.h>
#include <bcm/bcmswapitypes.h>
#include "bcmswshared.h"
#include "bcmswaccess.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#include "bcmsw.h"

#define IS_PHY_ADDR_FLAG 0x80000000
#define IS_SPI_ACCESS    0x40000000

#define PORT_ID_M 0xF
#define PORT_ID_S 0
#define PHY_REG_M 0x1F
#define PHY_REG_S 4

extern int dump_enable;

extern struct semaphore bcm_ethlock_switch_config;
extern spinlock_t bcm_extsw_access;
/* The external switch physical port to phyid mapping */
extern int switch_pport_phyid[TOTAL_SWITCH_PORTS];
extern int ext_switch_pport_phyid[TOTAL_SWITCH_PORTS];


#define SWITCH_ADDR_MASK 0xFFFF
extern void ethsw_phy_rreg(int phy_id, int reg, uint16 *data);
extern void ethsw_phy_wreg(int phy_id, int reg, uint16 *data);

static void bcmsw_spi_select(int bus_num, int spi_ss, int chip_id, int page)
{
    unsigned char buf[3];
    int tryCount = 0;
    static int spiRdyErrCnt = 0;

    /* SPIF status bit must be clear */
    while(1)
    {
       buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
           ((chip_id & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
       buf[1] = (unsigned char)BCM5325_SPI_STS;
       BcmSpiSyncTrans(buf, buf, BCM5325_SPI_PREPENDCNT, 1, bus_num, spi_ss);
       if (buf[0] & BCM5325_SPI_CMD_SPIF)
       {
          if ( spiRdyErrCnt < 10 )
          {
             spiRdyErrCnt++;
             printk("bcmsw_spi_select: SPIF set, not ready\n");
          }
          else if ( 10 == spiRdyErrCnt )
          {
             spiRdyErrCnt++;
             printk("bcmsw_spi_select: SPIF set, not ready - suppressing prints\n");
          }
          tryCount++;
          if (tryCount > 10)
          {
             return;
          }
       }
       else
       {
          break;
       }
    }

    /* Select new chip */
    buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_WRITE |
        ((chip_id & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);

    /* Select new page */
    buf[1] = PAGE_SELECT;
    buf[2] = (char)page;
    BcmSpiSyncTrans(buf, NULL, 0, sizeof(buf), bus_num, spi_ss);
}

void bcmsw_spi_rreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len)
{
    unsigned char buf[64];
    int rc;
    int i;
    int max_check_spi_sts;

    BCM_ENET_DEBUG("%s, spi_ss = %d, chip_id = %d, page = %d, " 
        "reg = %d, len = %d \n", (bus_num == LEG_SPI_BUS_NUM)?"Legacy SPI":"High Speed SPI", spi_ss, chip_id, page, reg, len);
    if (bus_num > HS_SPI_BUS_NUM) {
        printk("Invalid SPI bus number: %d \n", bus_num);
        return;
    }

    spin_lock(&bcm_extsw_access);

    bcmsw_spi_select(bus_num, spi_ss, chip_id, page);

    /* write command byte and register address */
    buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
        ((chip_id & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
    buf[1] = (unsigned char)reg;
    rc = BcmSpiSyncTrans(buf, buf, BCM5325_SPI_PREPENDCNT, 1, bus_num, spi_ss);

    if (rc == SPI_STATUS_OK) {
        max_check_spi_sts = 0;
        do {
            /* write command byte and read spi_sts address */
            buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
                ((chip_id & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
            buf[1] = (unsigned char)BCM5325_SPI_STS;
            rc = BcmSpiSyncTrans(buf, buf, BCM5325_SPI_PREPENDCNT, 1, bus_num, spi_ss);
            if (rc == SPI_STATUS_OK) {
                /* check the bit 0 RACK bit is set */
                if (buf[0] & BCM5325_SPI_CMD_RACK) {
                    break;
                }
                mdelay(1);
            } else {
                printk("BcmSpiSyncTrans failure \n");
                break;
            }
        } while (max_check_spi_sts++ < 10);


        if (rc == SPI_STATUS_OK) {
            buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_READ |
                ((chip_id & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);
            buf[1] = (unsigned char)0xf0;
            rc = BcmSpiSyncTrans(buf, buf, BCM5325_SPI_PREPENDCNT, len, bus_num, spi_ss);
            if (rc == SPI_STATUS_OK) {
                /* Write the data out in LE format to the switch */
                for (i = 0; i < len; i++)
                    *(data + i) = buf[i];
            } else {
                printk("BcmSpiSyncTrans failure \n");
            }
        }
        BCM_ENET_DEBUG( 
            "Data: %02x%02x%02x%02x %02x%02x%02x%02x \n", *(data+7), *(data+6),
            *(data+5), *(data+4), *(data+3), *(data+2), *(data+1), *(data+0));
   }
   spin_unlock(&bcm_extsw_access);
}

void bcmsw_spi_wreg(int bus_num, int spi_ss, int chip_id, int page, int reg, uint8 *data, int len)
{
    unsigned char buf[64];
    int i;

    BCM_ENET_DEBUG("%s, spi_ss = %d, chip_id = %d, page = %d, " 
        "reg = %d, len = %d \n", (bus_num == LEG_SPI_BUS_NUM)?"Legacy SPI":"High Speed SPI", spi_ss, chip_id, page, reg, len);
    BCM_ENET_DEBUG("Data: %02x%02x%02x%02x %02x%02x%02x%02x \n",
        *(data+7), *(data+6), *(data+5), *(data+4), *(data+3), *(data+2),
        *(data+1), *(data+0));

    if (bus_num > HS_SPI_BUS_NUM) {
        printk("Invalid SPI bus number: %d \n", bus_num);
        return;
    }

    spin_lock(&bcm_extsw_access);

    bcmsw_spi_select(bus_num, spi_ss, chip_id, page);

    buf[0] = BCM5325_SPI_CMD_NORMAL | BCM5325_SPI_CMD_WRITE |
        ((chip_id & BCM5325_SPI_CHIPID_MASK) << BCM5325_SPI_CHIPID_SHIFT);

    buf[1] = (char)reg;

    for (i = 0; i < len; i++) {
        /* Write the data out in LE format to the switch */
        buf[BCM5325_SPI_PREPENDCNT+i] = *(data + i);
    }

    BCM_ENET_DEBUG("%02x:%02x:%02x:%02x%02x%02x\n", buf[5], buf[4], buf[3],
                    buf[2], buf[1], buf[0]);
    BcmSpiSyncTrans(buf, NULL, 0, len+BCM5325_SPI_PREPENDCNT, bus_num, spi_ss);

    spin_unlock(&bcm_extsw_access);
}

static int bcmsw_phy_access(struct ethswctl_data *e, int access_type)
{
    uint16 phy_reg_val;
    uint8 data[8] = {0};
    int ext_bit = 0, phy_id, reg_offset; 

    if (access_type == MBUS_UBUS)
        phy_id = switch_pport_phyid[e->offset & PORT_ID_M];
    else
        phy_id = ext_switch_pport_phyid[e->offset & PORT_ID_M];

    reg_offset = (e->offset >> PHY_REG_S) & PHY_REG_M;
    
    if (e->type == TYPE_GET) {
        down(&bcm_ethlock_switch_config);
        if (access_type == MBUS_UBUS) {
            ethsw_phy_rreg(phy_id, reg_offset, &phy_reg_val);
        } else {
            ext_bit = 1;
            ethsw_phy_read_reg(phy_id, reg_offset, &phy_reg_val, ext_bit);
        }
        up(&bcm_ethlock_switch_config);
        BCM_ENET_DEBUG("phy_reg_val =0x%x \n", phy_reg_val);
        data[1] = phy_reg_val >> 8;
        data[0] = phy_reg_val & 0xFF;
        if (copy_to_user((void*)(&e->data), (void*)&data, 4)) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("e->data:%x %x  %x %x \n", e->data[3], e->data[2], e->data[1], e->data[0]);
    } else {
        BCM_ENET_DEBUG("Phy Data: %x %x %x %x \n", e->data[3], e->data[2], 
            e->data[1], e->data[0]);
        phy_reg_val = (e->data[1] << 8) | e->data[0];
        BCM_ENET_DEBUG("phy_reg_val = %x \n", phy_reg_val);
        down(&bcm_ethlock_switch_config);
        if (access_type == MBUS_UBUS) {
            ethsw_phy_wreg(phy_id, reg_offset, &phy_reg_val);
        } else {
            ext_bit = 1;
            ethsw_phy_write_reg(phy_id, reg_offset, &phy_reg_val, ext_bit);
        }
        up(&bcm_ethlock_switch_config);
    }
    return 0;
}

/* BCM5325E PSEUDO PHY register access through MDC/MDIO */
/* When reading or writing PSEUDO PHY registers, we must use the exact starting address and
   exact length for each register as defined in the data sheet.  In other words, for example,
   dividing a 32-bit register read into two 16-bit reads will produce wrong result.  Neither
   can we start read/write from the middle of a register.  Yet another bad example is trying
   to read a 32-bit register as a 48-bit one.  This is very important!!
*/
void bcmsw_pmdio_rreg(int page, int reg, uint8 *data, int len)
{
    uint16 v;
    int i;

    BCM_ENET_DEBUG("read op; page = %x; reg = %x; len = %d \n", 
        (unsigned int) page, (unsigned int) reg, len);
    
    spin_lock(&bcm_extsw_access);
    
    v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, &v);
    v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);

    for (i = 0; i < 20; i++) {
        ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);
        if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20) {
        printk("bcmsw_mdio_rreg: timeout!\n");
        spin_unlock(&bcm_extsw_access);
        return;
    }

    switch (len) {
        case 1:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            data[0] = (uint8)v;
            break;
        case 2:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[0] = swab16((uint16)v);
            break;
        case 4:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[0] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);            
            break;
        case 6:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint16 *)data)[0] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[2] = (uint16)v;
            if ((reg %4) == 0) {
                *(uint32 *)data = swab32(*(uint32 *)data);
                *((uint16 *)(data + 4)) = swab16(*((uint16 *)(data + 4)));
            } else {
                *(uint16 *)data = swab32(*(uint16 *)data);
                *((uint32 *)(data + 2)) = swab16(*((uint32 *)(data + 2)));
            }
            break;
        case 8:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &v);
            ((uint16 *)data)[0] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[2] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[3] = (uint16)v;
            ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
            break;
    }

    BCM_ENET_DEBUG("read data = %02x %02x %02x %02x \n", 
        data[0], data[1], data[2], data[3]);
    spin_unlock(&bcm_extsw_access);
}

void bcmsw_pmdio_wreg(int page, int reg, uint8 *data, int len)
{
    uint16 v;
    int i;

    BCM_ENET_DEBUG("write op; page = %x; reg = %x; len = %d \n", 
        (unsigned int) page, (unsigned int) reg, len);
    BCM_ENET_DEBUG("given data = %02x %02x %02x %02x \n", 
        data[0], data[1], data[2], data[3]);

    spin_lock(&bcm_extsw_access);
    v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, &v);

    switch (len) {
        case 1:
            v = data[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            break;
        case 2:
            v = swab16(((uint16 *)data)[0]);
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            break;
        case 4:
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            break;
        case 6:
            if ((reg %4) == 0) {
                *(uint32 *)data = swab32(*(uint32 *)data);
                *((uint16 *)(data + 4)) = swab16(*((uint16 *)(data + 4)));
            } else {
#if defined(AEI_VDSL_CUSTOMER_NCS)
                /*  Swab should be wrong,it will set the high 32 bits to zero.
                 *  Might BCM guys make a mistake when copy above swab code.
                 *  right code like this?
                 *  *(uint16 *)data = swab16(*(uint16 *)data);
                 *  *((uint32 *)(data + 2)) = swab32(*((uint32 *)(data + 2)));
                 */
                if(!(page == PAGE_AVTBL_ACCESS && reg == REG_ARL_MAC_INDX_LO))
#endif
                {
                    *(uint16 *)data = swab32(*(uint16 *)data);
                    *((uint32 *)(data + 2)) = swab16(*((uint32 *)(data + 2)));
                }
            }
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[2];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            break;
        case 8:
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
            v = ((uint16 *)data)[2];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[3];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            break;
    }

    v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);

    for (i = 0; i < 20; i++) {
        ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);
        if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    spin_unlock(&bcm_extsw_access);

    if (i >= 20)
        printk("ethsw_mdio_wreg: timeout!\n");
}

static void ethsw_read_reg(int addr, uint8 *data, int len)
{
    volatile uint8 *base = (volatile uint8 *)
       (SWITCH_BASE + (addr & SWITCH_ADDR_MASK));

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

    if (len == 1) {
        *data = *base;
    } else if (len == 2) {
        *(uint16 *)data = swab16(*(uint16 *)base);
    } else if (len == 4) {
        *(uint32 *)data = swab32(*(uint32 *)base);
    } else if (len == 6) {
        if ((addr % 4) == 0) {
            *(uint32 *)data = swab32(*(uint32 *)base);
            *(uint16 *)(data + 4) = swab16(*(uint16 *)(base + 4));
        } else {
            *(uint16 *)data = swab16(*(uint16 *)base);
            *(uint32 *)(data + 2) = swab32(*(uint32 *)(base + 2));
        }
    } else if (len == 8) {
        *(uint32 *)data = swab32(*(uint32 *)base);
        *(uint32 *)(data + 4) = swab32(*(uint32 *)(base + 4));
    }
}

static void ethsw_write_reg(int addr, uint8 *data, int len)
{
    volatile uint8 *base = (volatile uint8 *)
       (SWITCH_BASE + (addr & SWITCH_ADDR_MASK));
    int val32;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

    if (len == 1) {
        *base = *data;
    } else if (len == 2) {
        *(uint16 *)base = swab16(*(uint16 *)data);
    } else if (len == 4) {
        if ( (int)data & 0x3 )
        {
           val32 = ((*(uint16 *)data) << 16) | ((*(uint16 *)(data+2)) << 0);
           *(uint32 *)base = swab32(val32);
        }
        else
        {
           *(uint32 *)base = swab32(*(uint32 *)data);
        }
    } else if (len == 6) {
        if (addr % 4 == 0) {
            if ( (int)data & 0x3 )
            {
               val32 = ((*(uint16 *)data) << 16) | ((*(uint16 *)(data+2)) << 0);
               *(uint32 *)base = swab32(val32);
            }
            else
            {
               *(uint32 *)base = swab32(*(uint32 *)data);
            }
            *(uint16 *)(base + 4) = swab16(*(uint16 *)(data + 4));
        } else {
            *(uint16 *)base = swab16(*(uint16 *)data);
            if ( (int)(data+2) & 0x3 )
            {
               val32 = ((*(uint16 *)(data+2)) << 16) | ((*(uint16 *)(data+4)) << 0);
               *(uint32 *)(base + 2) = swab32(val32);
            }
            else
            {
               *(uint32 *)(base + 2) = swab32(*(uint32 *)(data + 2));
            }
        }
    } else if (len == 8) {
         if ( (int)data & 0x3 )
         {
            val32 = ((*(uint16 *)data) << 16) | ((*(uint16 *)(data+2)) << 0);
            *(uint32 *)base = swab32(val32);
            val32 = ((*(uint16 *)(data+4)) << 16) | ((*(uint16 *)(data+6)) << 0);
            *(uint32 *)(base + 4) = swab32(val32);
         }
         else
         {
            *(uint32 *)base = swab32(*(uint32 *)data);
            *(uint32 *)(base + 4) = swab32(*(uint32 *)(data + 4));
         }
    }
}

int enet_ioctl_ethsw_regaccess(struct ethswctl_data *e)
{
    int i, access_type = MBUS_UBUS;
    unsigned char data[8] = {0};

    BCM_ENET_DEBUG("addr=0x%x; len=%d\n", e->offset, e->length);

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

    if (e->offset & IS_PHY_ADDR_FLAG) {
        return bcmsw_phy_access(e, access_type);
    }

    if ( ((e->length != 1) && (e->length % 2)) || (e->length > 8)) {
        BCM_ENET_DEBUG("Invalid length");
        return -EINVAL;
    }
    if ( ((e->offset % 2) && (e->length == 2 || e->length == 6)) ||
         ((e->offset % 4) && (e->length == 4 || e->length == 8)) )
    {
        BCM_ENET_DEBUG("Invalid alignment");
        return -EINVAL;
    }

    if (e->type == TYPE_GET) {
        if (e->offset & IS_SPI_ACCESS)
            ethsw_rreg_ext(0, e->offset&(~IS_SPI_ACCESS), data, e->length, 1);
        else
            ethsw_read_reg(e->offset, data, e->length);
        if (copy_to_user((void*)(&e->data), (void*)&data, e->length)) {
            return -EFAULT;
        }
        BCM_ENET_DEBUG("Data: ");
        for (i = e->length-1; i >= 0; i--)
            BCM_ENET_DEBUG("%02x ", e->data[i]);
        BCM_ENET_DEBUG("\n");
    } else {
        BCM_ENET_DEBUG("Data: ");
        for (i = e->length-1; i >= 0; i--)
            BCM_ENET_DEBUG("%02x ", data[i]);
        BCM_ENET_DEBUG("\n");
        ethsw_write_reg(e->offset, e->data, e->length);
    }
    return BCM_E_NONE;
}

int enet_ioctl_ethsw_spiaccess(int bus_num, int spi_id, int chip_id, struct ethswctl_data *e) 
{
    int page, reg;
    uint8 data[8] = {0};

    if (e->offset & IS_PHY_ADDR_FLAG) {
        return bcmsw_phy_access(e, MBUS_SPI);
    } else {
        page = (e->offset >> 8) & 0xFF;
        reg = e->offset & 0xFF;
        if (e->type == TYPE_GET) {
            bcmsw_spi_rreg(bus_num, spi_id, chip_id, page,
                           reg, data, e->length);
            if (copy_to_user((void*)(e->data), (void*)data, e->length))
                return -EFAULT;
        } else {
            bcmsw_spi_wreg(bus_num, spi_id, chip_id, page,
                           reg, e->data, e->length);
        }
    }
    return 0;
}

int enet_ioctl_ethsw_pmdioaccess(struct net_device *dev, struct ethswctl_data *e) 
{
    int page, reg;
    uint8 data[8] = {0};

    if (e->offset & IS_PHY_ADDR_FLAG) {
        return bcmsw_phy_access(e, MBUS_MDIO);
    } else {
        page = (e->offset >> 8) & 0xFF;
        reg = e->offset & 0xFF;
        if (e->type == TYPE_GET) {
            bcmsw_pmdio_rreg(page, reg, data, e->length);
            if (copy_to_user((void*)(e->data), (void*)data, e->length))
                return -EFAULT;
        } else {
            bcmsw_pmdio_wreg(page, reg, e->data, e->length);
        }
    }
    return 0;
}

void get_ext_switch_access_info(int usConfigType, int *bus_type, int *spi_id) 
{
    switch (usConfigType) {
      case BP_ENET_CONFIG_SPI_SSB_0:
      case BP_ENET_CONFIG_SPI_SSB_1:
      case BP_ENET_CONFIG_SPI_SSB_2:
      case BP_ENET_CONFIG_SPI_SSB_3:
        *bus_type = MBUS_SPI;
        /* No ambiguity here, SSB_0 -> 0 and so on. So hard-coding instead of using SPI_DEV_ID_# defines. 
                  This also avoids the need to have separate SS# defines for the HS SPI in 63xx_map.h files*/
        *spi_id = usConfigType - BP_ENET_CONFIG_SPI_SSB_0;
        break;
      case BP_ENET_CONFIG_HS_SPI_SSB_0:
      case BP_ENET_CONFIG_HS_SPI_SSB_1:
      case BP_ENET_CONFIG_HS_SPI_SSB_2:
      case BP_ENET_CONFIG_HS_SPI_SSB_3:
      case BP_ENET_CONFIG_HS_SPI_SSB_4:
      case BP_ENET_CONFIG_HS_SPI_SSB_5:
      case BP_ENET_CONFIG_HS_SPI_SSB_6:
      case BP_ENET_CONFIG_HS_SPI_SSB_7:
        *bus_type = MBUS_HS_SPI;
        *spi_id =  usConfigType - BP_ENET_CONFIG_HS_SPI_SSB_0;
        break;
      case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
      case BP_ENET_CONFIG_GPIO_MDIO:
      case BP_ENET_CONFIG_MDIO:
        *bus_type = MBUS_MDIO;
        break;
      case BP_ENET_CONFIG_MMAP:
        *bus_type = MBUS_UBUS;
        break;
      default:
        *bus_type = MBUS_NONE;          
        break;
    }
}

int enet_ioctl_ethsw_info(struct net_device *dev, struct ethswctl_data *e) 
{
    int bus_type = MBUS_NONE, spi_id = 0, spi_cid = 0;
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
    ETHERNET_MAC_INFO *info;
    unsigned int vend_id = 0, dev_id = 0, rev_id = 0;
    uint8 data[4];
    unsigned int port_map, phy_map;
	int epon_port = -1;

    if (e->val > BP_MAX_ENET_MACS) {
        if (copy_to_user((void*)(&e->ret_val), (void*)&bus_type, sizeof(int))) {
            return -EFAULT;
        }
        return BCM_E_NONE;
    }

    if(BpGetEthernetMacInfo(&EnetInfo[0], BP_MAX_ENET_MACS) != BP_SUCCESS) 
    {
            printk(KERN_DEBUG " board id not set\n");
            return -ENODEV;
    }
    info = &EnetInfo[e->val];

    if ((info->ucPhyType == BP_ENET_EXTERNAL_SWITCH) || 
        (info->ucPhyType == BP_ENET_SWITCH_VIA_INTERNAL_PHY)) {

        get_ext_switch_access_info(info->usConfigType, &bus_type, &spi_id);

        switch (info->usConfigType) {
          case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
          case BP_ENET_CONFIG_GPIO_MDIO:
          case BP_ENET_CONFIG_MDIO:
            ethsw_phy_read_reg(0, 2, (uint16 *)data, 1);
            vend_id = (data[0] << 8) | data[1];
            ethsw_phy_read_reg(0, 3, (uint16 *)data, 1);
            dev_id = (data[0] << 8) | data[1];
            if (dev_id >= 0xb000) {
                rev_id = dev_id & 0xF;
                dev_id &= 0xFFF0;
            }
            break;
          case BP_ENET_CONFIG_SPI_SSB_0:
          case BP_ENET_CONFIG_SPI_SSB_1:
          case BP_ENET_CONFIG_SPI_SSB_2:
          case BP_ENET_CONFIG_SPI_SSB_3:
            bcmsw_spi_rreg(LEG_SPI_BUS_NUM, spi_id, spi_cid, 0x10, 0x04, data, 2);
            vend_id = (data[1] << 8) | data[0];
            bcmsw_spi_rreg(LEG_SPI_BUS_NUM, spi_id, spi_cid, 0x10, 0x06, data, 2);
            dev_id = (data[1] << 8) | data[0];
            if (dev_id >= 0xb000) {
                rev_id = dev_id & 0xF;
                dev_id &= 0xFFF0;
            }
            break;
          case BP_ENET_CONFIG_HS_SPI_SSB_0:
          case BP_ENET_CONFIG_HS_SPI_SSB_1:
          case BP_ENET_CONFIG_HS_SPI_SSB_2:
          case BP_ENET_CONFIG_HS_SPI_SSB_3:
          case BP_ENET_CONFIG_HS_SPI_SSB_4:
          case BP_ENET_CONFIG_HS_SPI_SSB_5:
          case BP_ENET_CONFIG_HS_SPI_SSB_6:
          case BP_ENET_CONFIG_HS_SPI_SSB_7:
            bcmsw_spi_rreg(HS_SPI_BUS_NUM, spi_id, spi_cid, 0x10, 0x04, data, 2);
            vend_id = (data[1] << 8) | data[0];
            bcmsw_spi_rreg(HS_SPI_BUS_NUM, spi_id, spi_cid, 0x10, 0x06, data, 2);
            dev_id = (data[1] << 8) | data[0];
            if (dev_id >= 0xb000) {
                rev_id = dev_id & 0xF;
                dev_id &= 0xFFF0;
            }
            break;
          case BP_ENET_CONFIG_MMAP:
            vend_id = 0x6300;
            dev_id = PERF->RevID >> CHIP_ID_SHIFT;
            break;
          default:
            break;
        }
    }

    if (copy_to_user((void*)(&e->ret_val), (void*)&bus_type, sizeof(int))) {
        return -EFAULT;
    }
    if (copy_to_user((void*)(&e->vendor_id), (void*)&vend_id, sizeof(int))) {
        return -EFAULT;
    }
    if (copy_to_user((void*)(&e->dev_id), (void*)&dev_id, sizeof(int))) {
        return -EFAULT;
    }
    if (copy_to_user((void*)(&e->rev_id), (void*)&rev_id, sizeof(int))) {
        return -EFAULT;
    }
    if (copy_to_user((void*)(&e->spi_id), (void*)&spi_id, sizeof(int))) {
        return -EFAULT;
    }
    if (copy_to_user((void*)(&e->chip_id), (void*)&spi_cid, sizeof(int))) {
        return -EFAULT;
    }
    port_map = info->sw.port_map;
    BCM_ENET_DEBUG("port_map = 0x%x \n", port_map);
    if (copy_to_user((void*)(&e->port_map), (void*)&port_map, sizeof(int))) {
        return -EFAULT;
    }

    phy_map = info->sw.port_map;
    if (vend_id == 0x6300) {
        int i;
        for (i = 0; i < TOTAL_SWITCH_PORTS; i++) {
			
			if (info->sw.phy_id[i] & CONNECTED_TO_EPON_MAC) {
				epon_port = i;
			}
		
            if (!IsPhyConnected(info->sw.phy_id[i]) || 
                (info->sw.phy_id[i] & BCM_EXT_6829))
                phy_map &= ~(1 << i);
        }
    }
    if (copy_to_user((void*)(&e->phy_portmap), (void*)&phy_map, sizeof(int))) {
        return -EFAULT;
    }
	
	
    if (copy_to_user((void*)(&e->epon_port), (void*)&epon_port, sizeof(int))) {
        return -EFAULT;
    }

    return BCM_E_NONE;
}

int enet_ioctl_phy_cfg_get(struct net_device *dev, struct ethswctl_data *e)
{
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
    ETHERNET_MAC_INFO *info;
    int phycfg, ret_val = -1;

    if (e->val > BP_MAX_ENET_MACS) {
        if (copy_to_user((void*)(&e->ret_val), (void*)&ret_val, sizeof(int))) {
            return -EFAULT;
        }
        return BCM_E_NONE;
    }

    if(BpGetEthernetMacInfo(&EnetInfo[e->val], BP_MAX_ENET_MACS) != BP_SUCCESS) 
    {
        printk(KERN_DEBUG " board id not set\n");
        return -ENODEV;
    }
    info = &EnetInfo[e->val];

    phycfg = info->sw.phy_id[e->port];

    if (copy_to_user((void*)(&e->phycfg), (void*)&phycfg, sizeof(int))) {
        return -EFAULT;
    }
    return BCM_E_NONE;
}

