/*
<:copyright-gpl
 Copyright 2011 Broadcom Corp. All Rights Reserved.

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

#ifndef _LASER_H_
#define _LASER_H_

#include <linux/if.h>
#include <linux/types.h>
#include <linux/ioctl.h>




#define LASER_IOC_MAGIC		'O'


#define LASER_IOCTL_GET_DRV_INFO	    _IOR    (LASER_IOC_MAGIC, 0, long)
#define LASER_IOCTL_SET_OPTICAL_PARAMS  _IOW    (LASER_IOC_MAGIC, 1, char *)
#define LASER_IOCTL_GET_OPTICAL_PARAMS  _IOW    (LASER_IOC_MAGIC, 2, char *)
#define LASER_IOCTL_READ_LASER_DEV	    _IOR    (LASER_IOC_MAGIC, 3, short)
#define LASER_IOCTL_WRITE_LASER_DEV	    _IOW    (LASER_IOC_MAGIC, 4, long)
#define LASER_IOCTL_GET_RX_PWR	        _IOR    (LASER_IOC_MAGIC, 5, short)
#define LASER_IOCTL_GET_TX_PWR	        _IOR    (LASER_IOC_MAGIC, 6, short)
#define LASER_IOCTL_GET_TEMP	        _IOR    (LASER_IOC_MAGIC, 7, short)


int laser_i2c_write_byte(unsigned char , unsigned char);
unsigned char laser_i2c_read_byte(unsigned char );
ssize_t laser_i2c_write(char *, size_t);
ssize_t laser_i2c_read(char *, size_t);

#endif /* ! _LASER_H_ */
