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

#ifndef _BMOCA_H_
#define _BMOCA_H_

#include <linux/if.h>
#include <linux/types.h>
#include <linux/ioctl.h>

/* NOTE: These need to match what is defined in the API template */
#define MOCA_IE_DRV_PRINTF	0xff00
#define MOCA_IE_WDT		0xff01

#define MOCA_BAND_HIGHRF	0
#define MOCA_BAND_MIDRF		1
#define MOCA_BAND_WANRF		2

#define MOCA_IOC_MAGIC		'M'

#define MOCA_IOCTL_GET_DRV_INFO_V1	_IOR(MOCA_IOC_MAGIC, 0, \
	struct moca_kdrv_info_v1)

#define MOCA_IOCTL_START	_IOW(MOCA_IOC_MAGIC, 1, struct moca_start)
#define MOCA_IOCTL_STOP		_IO(MOCA_IOC_MAGIC, 2)
#define MOCA_IOCTL_READMEM	_IOR(MOCA_IOC_MAGIC, 3, struct moca_xfer)
#define MOCA_IOCTL_WRITEMEM	_IOR(MOCA_IOC_MAGIC, 4, struct moca_xfer)

#define MOCA_IOCTL_CHECK_FOR_DATA	_IOR(MOCA_IOC_MAGIC, 5, int)
#define MOCA_IOCTL_GET_DRV_INFO	_IOR(MOCA_IOC_MAGIC, 0, struct moca_kdrv_info)

/* this must match MoCAOS_IFNAMSIZE */
#define MOCA_IFNAMSIZ		16

/* Legacy version of moca_kdrv_info */
struct moca_kdrv_info_v1 {
	__u32			version;
	__u32			build_number;
	__u32			builtin_fw;

	__u32			hw_rev;
	__u32			rf_band;

	__u32			uptime;
	__s32			refcount;
	__u32			gp1;

	__s8			enet_name[MOCA_IFNAMSIZ];
	__u32			enet_id;

	__u32			macaddr_hi;
	__u32			macaddr_lo;
};

/* this must match MoCAOS_DrvInfo */
struct moca_kdrv_info {
	__u32			version;
	__u32			build_number;
	__u32			builtin_fw;

	__u32			hw_rev;
	__u32			rf_band;

	__u32			uptime;
	__s32			refcount;
	__u32			gp1;

	__s8			enet_name[MOCA_IFNAMSIZ];
	__u32			enet_id;

	__u32			macaddr_hi;
	__u32			macaddr_lo;

	__u32			phy_freq;
	__u32			cpu_freq;
};

struct moca_xfer {
	__u64			buf;
	__u32			len;
	__u32			moca_addr;
};

struct moca_start {
	struct moca_xfer	x;
	__u32			continuous_power_tx_mode;
};

#ifdef __KERNEL__

static inline void mac_to_u32(uint32_t *hi, uint32_t *lo, const uint8_t *mac)
{
	*hi = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | (mac[3] << 0);
	*lo = (mac[4] << 24) | (mac[5] << 16);
}

static inline void u32_to_mac(uint8_t *mac, uint32_t hi, uint32_t lo)
{
	mac[0] = (hi >> 24) & 0xff;
	mac[1] = (hi >> 16) & 0xff;
	mac[2] = (hi >>  8) & 0xff;
	mac[3] = (hi >>  0) & 0xff;
	mac[4] = (lo >> 24) & 0xff;
	mac[5] = (lo >> 16) & 0xff;
}

struct moca_platform_data {
	char			enet_name[IFNAMSIZ];
	unsigned int		enet_id;

	u32			macaddr_hi;
	u32			macaddr_lo;

	phys_t			bcm3450_i2c_base;
	int			bcm3450_i2c_addr;

	u32			hw_rev;
	u32			rf_band;

	int			useDma;
	int			useSpi;
#ifdef CONFIG_SMP
	int			smp_processor_id;
#endif
};

/* in bmoca-6816.c */
extern void moca_get_fc_bits(int isWan, unsigned long *moca_fc_reg);

#endif /* __KERNEL__ */

#endif /* ! _BMOCA_H_ */
