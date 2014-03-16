/*
<:copyright-gpl
 Copyright 2010 Broadcom Corp. All Rights Reserved.

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
/***************************************************************************
* File Name  : bcm63xx_gpio.c
*
* Description: This file contains functions related to GPIO access.
*     See GPIO register defs in shared/broadcom/include/bcm963xx/xxx_common.h
*
*
***************************************************************************/

/*
 * Access to _SHARED_ GPIO registers should go through common functions
 * defined in board.h.  These common functions will use a spinlock with
 * irq's disabled to prevent concurrent access.
 * Functions which don't want to call the common gpio access functions must
 * acquire the bcm_gpio_spinlock with irq's disabled before accessing the
 * shared GPIO registers.
 * The GPIO registers that must be protected are:
 * GPIO->GPIODir
 * GPIO->GPIOio
 * GPIO->GPIOMode
 *
 * Note that many GPIO registers are dedicated to some driver or sub-system.
 * In those cases, the driver/sub-system can use its own locking scheme to
 * ensure serial access to its GPIO registers.
 *
 * DEFINE_SPINLOCK is the new, recommended way to declaring a spinlock.
 * See spinlock_types.h
 *
 * I am using a spin_lock_irqsave/spin_lock_irqrestore to lock and unlock
 * so that GPIO's can be accessed in interrupt context.
 */
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/bcm_assert_locks.h>
#include <bcm_map_part.h>
#include "board.h"
#include <boardparms.h>

DEFINE_SPINLOCK(bcm_gpio_spinlock);
EXPORT_SYMBOL(bcm_gpio_spinlock);


void kerSysSetGpioStateLocked(unsigned short bpGpio, GPIO_STATE_t state)
{
    BCM_ASSERT_V(bpGpio != BP_NOT_DEFINED);
    BCM_ASSERT_V((bpGpio & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX);
    BCM_ASSERT_HAS_SPINLOCK_V(&bcm_gpio_spinlock);

    kerSysSetGpioDirLocked(bpGpio);

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    /* Take over high GPIOs from WLAN block */
    if ((bpGpio & BP_GPIO_NUM_MASK) > 31)
        GPIO->GPIOCtrl |= GPIO_NUM_TO_MASK(bpGpio - 32);
#endif

    if((state == kGpioActive && !(bpGpio & BP_ACTIVE_LOW)) ||
        (state == kGpioInactive && (bpGpio & BP_ACTIVE_LOW)))
        GPIO->GPIOio |= GPIO_NUM_TO_MASK(bpGpio);
    else
        GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(bpGpio);

}

void kerSysSetGpioState(unsigned short bpGpio, GPIO_STATE_t state)
{
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);
    kerSysSetGpioStateLocked(bpGpio, state);
    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
}



void kerSysSetGpioDirLocked(unsigned short bpGpio)
{
    BCM_ASSERT_V(bpGpio != BP_NOT_DEFINED);
    BCM_ASSERT_V((bpGpio & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX);
    BCM_ASSERT_HAS_SPINLOCK_V(&bcm_gpio_spinlock);

    GPIO->GPIODir |= GPIO_NUM_TO_MASK(bpGpio);
}

void kerSysSetGpioDir(unsigned short bpGpio)
{
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);
    kerSysSetGpioDirLocked(bpGpio);
    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
}


EXPORT_SYMBOL(kerSysSetGpioState);
EXPORT_SYMBOL(kerSysSetGpioStateLocked);
EXPORT_SYMBOL(kerSysSetGpioDir);
EXPORT_SYMBOL(kerSysSetGpioDirLocked);
