/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
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
 * File Name  : bcm63xx_led.c
 *
 * Description: 
 *    This file contains bcm963xx board led control API functions. 
 *
 ***************************************************************************/

/* Includes. */
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/bcm_assert_locks.h>
#include <asm/uaccess.h>

#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>

extern spinlock_t bcm_gpio_spinlock;

#define k125ms              (HZ / 8)   // 125 ms
#define kFastBlinkCount     0          // 125ms
#define kSlowBlinkCount     1          // 250ms
#define kVerySlowBlinkCount     3          // 500ms
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
#define kOneSecondBlinkCount 7          //1 second
#endif  
#define kLedOff             0
#define kLedOn              1

#define kLedGreen           0
#define kLedRed             1

// uncomment // for debug led
// #define DEBUG_LED

typedef int (*BP_LED_FUNC) (unsigned short *);

typedef struct {
    BOARD_LED_NAME ledName;
    BP_LED_FUNC bpFunc;
    BP_LED_FUNC bpFuncFail;
} BP_LED_INFO, *PBP_LED_INFO;

typedef struct {
    short ledGreenGpio;             // GPIO # for LED
    short ledRedGpio;               // GPIO # for Fail LED
    BOARD_LED_STATE ledState;       // current led state
    short blinkCountDown;           // Count for blink states
} LED_CTRL, *PLED_CTRL;

static BP_LED_INFO bpLedInfo[] =
{
    {kLedAdsl, BpGetAdslLedGpio, BpGetAdslFailLedGpio},
    {kLedSecAdsl, BpGetSecAdslLedGpio, BpGetSecAdslFailLedGpio},
    {kLedWanData, BpGetWanDataLedGpio, BpGetWanErrorLedGpio},
    {kLedSes, BpGetWirelessSesLedGpio, NULL},
    {kLedVoip, BpGetVoipLedGpio, NULL},
    {kLedVoip1, BpGetVoip1LedGpio, BpGetVoip1FailLedGpio},
    {kLedVoip2, BpGetVoip2LedGpio, BpGetVoip2FailLedGpio},
    {kLedPots, BpGetPotsLedGpio, NULL},
    {kLedDect, BpGetDectLedGpio, NULL},
    {kLedGpon, BpGetGponLedGpio, BpGetGponFailLedGpio},
    {kLedMoCA, BpGetMoCALedGpio, BpGetMoCAFailLedGpio},
#if defined(AEI_VDSL_CUSTOMER_NCS)
    {kLedUsb, BpGetUsbLedGpio, NULL},
    {kLedPower, BpGetBootloaderPowerOnLedGpio, BpGetBootloaderStopLedGpio},
    {kLedSes, BpGetWirelessSesLedGpio, BpGetWirelessFailSesLedGpio},
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    {AEI_kLedWlanAct, BpGetWirelessLedGpioAct, NULL},
    {AEI_kLedWlan, BpGetWirelessLedGpioGreen, BpGetWirelessLedGpioRed},
    {AEI_kLedWlanGreen, BpGetWirelessLedGpioGreen, NULL},
    {AEI_kLedWlanRed, BpGetWirelessLedGpioRed, NULL},
#endif
#if defined(AEI_63168_CHIP)
    {kLedEnetWan, BpGetEnetWanLedGpio, NULL},
#endif

#endif
    {kLedEnd, NULL, NULL}
};

// global variables:
static struct timer_list gLedTimer;
static PLED_CTRL gLedCtrl = NULL;
static int gTimerOn = FALSE;
static int gTimerOnRequests = 0;
static unsigned int gLedRunningCounter = 0;  // only used by WLAN

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
static bool gPowerLedLocalUpgrade = FALSE;
static bool gPowerLedTr69Upgrade = TRUE;
int  gPowerLedStatus = 1;                              //1:green; 2:amber and green alternative;3 amber blink
static unsigned int gWANLedRunningCounter = 0;  // only used by WAN
#endif
void (*ethsw_led_control)(unsigned long ledMask, int value) = NULL;
EXPORT_SYMBOL(ethsw_led_control);

/** This spinlock protects all access to gLedCtrl, gTimerOn
 *  gTimerOnRequests, gLedRunningCounter, and ledTimerStart function.
 *  Use spin_lock_irqsave to lock the spinlock because ledTimerStart
 *  may be called from interrupt handler (WLAN?)
 */
static spinlock_t brcm_ledlock;
static void ledTimerExpire(void);

//**************************************************************************************
// LED operations
//**************************************************************************************
#if defined(AEI_VDSL_CUSTOMER_NCS)
/* ken, Set HW control for WAN Data LED. */
void AEI_SetWanLedHwControl(BOARD_LED_NAME ledName,PLED_CTRL pLed,int enable) 
{
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    if (ledName == kLedWanData)
    {
        if(enable)
                LED->ledHWDis &= ~GPIO_NUM_TO_MASK(pLed->ledGreenGpio);            
        else
            LED->ledHWDis |= GPIO_NUM_TO_MASK(pLed->ledGreenGpio);
    }
#elif defined(CONFIG_BCM96368)

    if (ledName == kLedWanData)
    {
        if(enable)
        {
                GPIO->AuxLedCtrl &= ~AUX_HW_DISAB_2;
                if (pLed->ledGreenGpio & BP_ACTIVE_LOW)
                    GPIO->AuxLedCtrl &= ~(LED_STEADY_ON << AUX_MODE_SHFT_2);
                else
                    GPIO->AuxLedCtrl |= (LED_STEADY_ON << AUX_MODE_SHFT_2);
        }
        else
        {
            GPIO->AuxLedCtrl |= AUX_HW_DISAB_2;
            if (pLed->ledGreenGpio & BP_ACTIVE_LOW)
                GPIO->AuxLedCtrl |= (LED_STEADY_ON << AUX_MODE_SHFT_2);
            else
                GPIO->AuxLedCtrl &= ~(LED_STEADY_ON << AUX_MODE_SHFT_2);
        }
    }

#endif

}
#endif
// turn led on and set the ledState
static void setLed (PLED_CTRL pLed, unsigned short led_state, unsigned short led_type)
{
    short led_gpio;
    unsigned short gpio_state;
    unsigned long flags;

    if (led_type == kLedRed)
        led_gpio = pLed->ledRedGpio;
    else
        led_gpio = pLed->ledGreenGpio;

    if (led_gpio == -1)
        return;

    if (((led_gpio & BP_ACTIVE_LOW) && (led_state == kLedOn)) || 
        (!(led_gpio & BP_ACTIVE_LOW) && (led_state == kLedOff)))
        gpio_state = 0;
    else
        gpio_state = 1;

    /* spinlock to protect access to GPIODir, GPIOio */
    spin_lock_irqsave(&bcm_gpio_spinlock, flags);

#if defined(CONFIG_BCM96328)
    /* Enable LED controller to drive this GPIO */
    if (!(led_gpio & BP_GPIO_SERIAL))
        GPIO->GPIOMode |= GPIO_NUM_TO_MASK(led_gpio);
#endif

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    /* Enable LED controller to drive this GPIO */
    if (!(led_gpio & BP_GPIO_SERIAL))
        GPIO->LEDCtrl |= GPIO_NUM_TO_MASK(led_gpio);
    if((BP_GPIO_NUM_MASK & led_gpio)>=2 && (BP_GPIO_NUM_MASK & led_gpio)<=7)
        printk("####set NAND FLASH gpio incorrectly (%d),LEDCtl(%x)\n",(BP_GPIO_NUM_MASK & led_gpio),GPIO->LEDCtrl);

    if((BP_GPIO_NUM_MASK & led_gpio)>=24 && (BP_GPIO_NUM_MASK & led_gpio)<=31)
        printk("####set NAND FLASH incorrectly (%d),LEDCtl(%x)\n",(BP_GPIO_NUM_MASK & led_gpio),GPIO->LEDCtrl);

#endif

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
#ifdef AEI_63168_CHIP
    if ((led_gpio & BP_GPIO_NUM_MASK) > 31) 
    {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
        if( gpio_state )
        {
            GPIO->GPIOio |= GPIO_NUM_TO_MASK(led_gpio);
        }
        else
        {
            GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(led_gpio); //off
        }
    }
    else 
    {
        LED->ledMode &= ~(LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
        if( gpio_state )
            LED->ledMode |= (LED_MODE_OFF << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
        else
            LED->ledMode |= (LED_MODE_ON << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    }
#else
    LED->ledMode &= ~(LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    if( gpio_state )
        LED->ledMode |= (LED_MODE_OFF << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    else
        LED->ledMode |= (LED_MODE_ON << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));

#endif
#else
    if (led_gpio & BP_GPIO_SERIAL) {
        while (GPIO->SerialLedCtrl & SER_LED_BUSY);
        if( gpio_state )
            GPIO->SerialLed |= GPIO_NUM_TO_MASK(led_gpio);
        else
            GPIO->SerialLed &= ~GPIO_NUM_TO_MASK(led_gpio);
    }
    else {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
        if( gpio_state )
            GPIO->GPIOio |= GPIO_NUM_TO_MASK(led_gpio);
        else
            GPIO->GPIOio &= ~GPIO_NUM_TO_MASK(led_gpio); //off
    }
#endif

    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
}

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
/* toggle the LED */
static void wpsledToggle(PLED_CTRL pLed)
{
    short led_gpio;
    short i;
    static unsigned short state;

    if (state == 0)
        state = 1;
    else
        state = 0;

    for (i=0;i<2;i++)/*To amber color*/
    {
        if (i==0)
            led_gpio = pLed->ledGreenGpio;
        else
            led_gpio = pLed->ledRedGpio;

        if (led_gpio == -1)
            return;

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    if((BP_GPIO_NUM_MASK & led_gpio)>=2 && (BP_GPIO_NUM_MASK & led_gpio)<=7)
        printk("####set NAND FLASH gpio incorrectly (%d),LEDCtl(%x)\n",(BP_GPIO_NUM_MASK & led_gpio),GPIO->LEDCtrl);

    if((BP_GPIO_NUM_MASK & led_gpio)>=24 && (BP_GPIO_NUM_MASK & led_gpio)<=31)
        printk("####set NAND FLASH incorrectly (%d),LEDCtl(%x)\n",(BP_GPIO_NUM_MASK & led_gpio),GPIO->LEDCtrl);

        if ((led_gpio & BP_GPIO_NUM_MASK) > 31)
        {
            GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
            GPIO->GPIOio ^= (GPIO_NUM_TO_MASK(led_gpio));
        }
        else
        {
            LED->ledMode ^= (LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
        }
#else

        if (led_gpio & BP_GPIO_SERIAL) {
            while (GPIO->SerialLedCtrl & SER_LED_BUSY);
            GPIO->SerialLed ^= GPIO_NUM_TO_MASK(led_gpio);
        }
        else {
            unsigned long flags;

            spin_lock_irqsave(&bcm_gpio_spinlock, flags);
            GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
            GPIO->GPIOio ^= GPIO_NUM_TO_MASK(led_gpio);
            /*printk("GPIO->GPIOio=%x\n", GPIO->GPIOio);*/

            spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
        }
#endif
    }
}
#endif
static void ledToggle(PLED_CTRL pLed)
{
    short led_gpio;
    led_gpio = pLed->ledGreenGpio;

    if (led_gpio == -1)
        return;

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    if((BP_GPIO_NUM_MASK & led_gpio)>=2 && (BP_GPIO_NUM_MASK & led_gpio)<=7)
        printk("####set NAND FLASH gpio incorrectly (%d),LEDCtl(%x)\n",(BP_GPIO_NUM_MASK & led_gpio),GPIO->LEDCtrl);

    if((BP_GPIO_NUM_MASK & led_gpio)>=24 && (BP_GPIO_NUM_MASK & led_gpio)<=31)
        printk("####set NAND FLASH incorrectly (%d),LEDCtl(%x)\n",(BP_GPIO_NUM_MASK & led_gpio),GPIO->LEDCtrl);

#ifdef AEI_63168_CHIP
    if ((led_gpio & BP_GPIO_NUM_MASK) > 31)
    {
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
        GPIO->GPIOio ^= (GPIO_NUM_TO_MASK(led_gpio));
    }
    else 
    {
        LED->ledMode ^= (LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
    }
#else
    LED->ledMode ^= (LED_MODE_MASK << GPIO_NUM_TO_LED_MODE_SHIFT(led_gpio));
#endif
#else

    if (led_gpio & BP_GPIO_SERIAL) {
        while (GPIO->SerialLedCtrl & SER_LED_BUSY);
        GPIO->SerialLed ^= GPIO_NUM_TO_MASK(led_gpio);
    }
    else {
        unsigned long flags;

        spin_lock_irqsave(&bcm_gpio_spinlock, flags);
        GPIO->GPIODir |= GPIO_NUM_TO_MASK(led_gpio);
        GPIO->GPIOio ^= GPIO_NUM_TO_MASK(led_gpio);

        spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
    }

#endif
}

/** Start the LED timer
 *
 * Must be called with brcm_ledlock held
 */
static void ledTimerStart(void)
{
#if defined(DEBUG_LED)
    printk("led: add_timer\n");
#endif

    BCM_ASSERT_HAS_SPINLOCK_C(&brcm_ledlock);

    init_timer(&gLedTimer);
    gLedTimer.function = (void*)ledTimerExpire;
    gLedTimer.expires = jiffies + k125ms;        // timer expires in ~100ms
    add_timer (&gLedTimer);
} 


// led timer expire kicks in about ~100ms and perform the led operation according to the ledState and
// restart the timer according to ledState
static void ledTimerExpire(void)
{
    int i;
    PLED_CTRL pCurLed;
    unsigned long flags;

    BCM_ASSERT_NOT_HAS_SPINLOCK_V(&brcm_ledlock);

    for (i = 0, pCurLed = gLedCtrl; i < kLedEnd; i++, pCurLed++)
    {
#if defined(DEBUG_LED)
        printk("led[%d]: Mask=0x%04x, State = %d, blcd=%d\n", i, pCurLed->ledGreenGpio, pCurLed->ledState, pCurLed->blinkCountDown);
#endif
        spin_lock_irqsave(&brcm_ledlock, flags);        // LEDs can be changed from ISR
        switch (pCurLed->ledState)
        {
        case kLedStateOn:
        case kLedStateOff:
        case kLedStateFail:
#if defined(AEI_VDSL_CUSTOMER_NCS)
        case kLedStateAmber:
#endif
            pCurLed->blinkCountDown = 0;            // reset the blink count down
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;

#if defined(AEI_VDSL_CUSTOMER_NCS)
        case kLedStateVerySlowBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kVerySlowBlinkCount;
                ledToggle(pCurLed);
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;
#endif

        case kLedStateSlowBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kSlowBlinkCount;
                ledToggle(pCurLed);
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;

        case kLedStateFastBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kFastBlinkCount;
                ledToggle(pCurLed);
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
        case kLedStateAmberSlowBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kOneSecondBlinkCount;
            if (gPowerLedLocalUpgrade == FALSE)
                {
                    gPowerLedLocalUpgrade = TRUE;
                    setLed(pCurLed, kLedOn, kLedGreen);
                    setLed(pCurLed, kLedOn, kLedRed);
                }
                else
                {
                    gPowerLedLocalUpgrade = FALSE;
                    setLed(pCurLed, kLedOff, kLedGreen);
                    setLed(pCurLed, kLedOff, kLedRed);
                }
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;

        case kLedStateAmberAndGreenSlowBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kOneSecondBlinkCount;
                if (gPowerLedTr69Upgrade == FALSE)
                {
                    gPowerLedTr69Upgrade = TRUE;
                    setLed(pCurLed, kLedOn, kLedGreen);
                    setLed(pCurLed, kLedOn, kLedRed);
                }
                else
                {
                    gPowerLedTr69Upgrade = FALSE;
                    setLed(pCurLed, kLedOff, kLedRed);
                }

            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;
#endif

        case kLedStateUserWpsInProgress:          /* 200ms on, 100ms off */
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = (((gLedRunningCounter++)&1)? kFastBlinkCount: kSlowBlinkCount);
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
                wpsledToggle(pCurLed);
#else
                ledToggle(pCurLed);
#endif
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;             

        case kLedStateUserWpsError:               /* 100ms on, 100ms off */
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kFastBlinkCount;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
                wpsledToggle(pCurLed);
#else
                ledToggle(pCurLed);
#endif
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;        

        case kLedStateUserWpsSessionOverLap:      /* 100ms on, 100ms off, 5 times, off for 500ms */        
            if (pCurLed->blinkCountDown-- == 0)
            {
                if(((++gLedRunningCounter)%10) == 0) {
                    pCurLed->blinkCountDown = 4;
                    setLed(pCurLed, kLedOff, kLedGreen);
                    pCurLed->ledState = kLedStateUserWpsSessionOverLap;
                }
                else
                {
                    pCurLed->blinkCountDown = kFastBlinkCount;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
                    wpsledToggle(pCurLed);
#else
                    ledToggle(pCurLed);
#endif
                }
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;        
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) || defined(AEI_VDSL_CUSTOMER_BELLALIANT) 
        case kLedStateUserWANGreenRedVerySlowBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
                pCurLed->blinkCountDown = kVerySlowBlinkCount;
                if(gWANLedRunningCounter == 0)
                {
                    AEI_SetWanLedHwControl(kLedWanData,pCurLed,1);         
                   setLed(pCurLed, kLedOff, kLedRed);                    
                   setLed(pCurLed, kLedOn, kLedGreen);
                    gWANLedRunningCounter = 1;
                }
                else
                {
                    AEI_SetWanLedHwControl(kLedWanData,pCurLed,0);            
                    setLed(pCurLed, kLedOff, kLedGreen);
                    setLed(pCurLed, kLedOn, kLedRed);
                    gWANLedRunningCounter = 0;
                }
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);

            break;
        case kLedStatePowerOneSecondBlinkContinues:
            if (pCurLed->blinkCountDown-- == 0)
            {
               pCurLed->blinkCountDown = kOneSecondBlinkCount;
               ledToggle(pCurLed);
            }
            gTimerOnRequests++;
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            break;
#endif            
        default:
            spin_unlock_irqrestore(&brcm_ledlock, flags);
            printk("Invalid state = %d\n", pCurLed->ledState);
        }
    }

    // Restart the timer if any of our previous LED operations required
    // us to restart the timer, or if any other threads requested a timer
    // restart.  Note that throughout this function, gTimerOn == TRUE, so
    // any other thread which want to restart the timer would only
    // increment the gTimerOnRequests and not call ledTimerStart.
    spin_lock_irqsave(&brcm_ledlock, flags);
    if (gTimerOnRequests > 0)
    {
        ledTimerStart();
        gTimerOnRequests = 0;
    }
    else
    {
        gTimerOn = FALSE;
    }
    spin_unlock_irqrestore(&brcm_ledlock, flags);
}

void __init boardLedInit(void)
{
    PBP_LED_INFO pInfo;
    unsigned short i;
    short gpio;
#if defined(CONFIG_BCM96816)
    ETHERNET_MAC_INFO EnetInfo;
#endif

    spin_lock_init(&brcm_ledlock);

#if !(defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268))
    /* Set blink rate for hardware LEDs. */
    GPIO->LEDCtrl &= ~LED_INTERVAL_SET_MASK;
    GPIO->LEDCtrl |= LED_INTERVAL_SET_80MS;
#else
#if defined(AEI_63168_CHIP)
    GPIO->GPIOCtrl |= 0x000FFFC0;
#endif
#endif

    gLedCtrl = (PLED_CTRL) kmalloc((kLedEnd * sizeof(LED_CTRL)), GFP_KERNEL);

    if( gLedCtrl == NULL ) {
        printk( "LED memory allocation error.\n" );
        return;
    }
    
    /* Initialize LED control */
    for (i = 0; i < kLedEnd; i++) {
        gLedCtrl[i].ledGreenGpio = -1;
        gLedCtrl[i].ledRedGpio = -1;
        gLedCtrl[i].ledState = kLedStateOff;
        gLedCtrl[i].blinkCountDown = 0;            // reset the blink count down
    }

    for( pInfo = bpLedInfo; pInfo->ledName != kLedEnd; pInfo++ ) {
        if( pInfo->bpFunc && (*pInfo->bpFunc) (&gpio) == BP_SUCCESS )
        {
            gLedCtrl[pInfo->ledName].ledGreenGpio = gpio;
        }
        if( pInfo->bpFuncFail && (*pInfo->bpFuncFail)(&gpio)==BP_SUCCESS )
        {
            gLedCtrl[pInfo->ledName].ledRedGpio = gpio;
        }

#if defined(AEI_VDSL_CUSTOMER_NCS)
        if (pInfo->ledName == kLedPower)
        {
            continue;  
        }     
#endif

        setLed(&gLedCtrl[pInfo->ledName], kLedOff, kLedGreen);
        setLed(&gLedCtrl[pInfo->ledName], kLedOff, kLedRed);
    }

#if defined(CONFIG_BCM96816)
    if( BpGetEthernetMacInfo( &EnetInfo, 1 ) == BP_SUCCESS )
    {
        if ( EnetInfo.sw.ledInfo[0].duplexLed != BP_NOT_DEFINED )
        {
            gLedCtrl[kLedEth0Duplex].ledGreenGpio = EnetInfo.sw.ledInfo[0].duplexLed;
            setLed(&gLedCtrl[kLedEth0Duplex], kLedOff, kLedGreen);
        }
        if ( EnetInfo.sw.ledInfo[0].speedLed100 != BP_NOT_DEFINED )
        {
            gLedCtrl[kLedEth0Spd100].ledGreenGpio = EnetInfo.sw.ledInfo[0].speedLed100;
            setLed(&gLedCtrl[kLedEth0Spd100], kLedOff, kLedGreen);
        }
        if ( EnetInfo.sw.ledInfo[0].speedLed1000 != BP_NOT_DEFINED )
        {
            gLedCtrl[kLedEth0Spd1000].ledGreenGpio = EnetInfo.sw.ledInfo[0].speedLed1000;
            setLed(&gLedCtrl[kLedEth0Spd1000], kLedOff, kLedGreen);
        }
     
        if ( EnetInfo.sw.ledInfo[1].duplexLed != BP_NOT_DEFINED )
        {
            gLedCtrl[kLedEth1Duplex].ledGreenGpio = EnetInfo.sw.ledInfo[1].duplexLed;
            setLed(&gLedCtrl[kLedEth1Duplex], kLedOff, kLedGreen);
        }
        if ( EnetInfo.sw.ledInfo[1].speedLed100 != BP_NOT_DEFINED )
        {
            gLedCtrl[kLedEth1Spd100].ledGreenGpio = EnetInfo.sw.ledInfo[1].speedLed100;
            setLed(&gLedCtrl[kLedEth1Spd100], kLedOff, kLedGreen);
        }
        if ( EnetInfo.sw.ledInfo[1].speedLed1000 != BP_NOT_DEFINED )
        {
            gLedCtrl[kLedEth1Spd1000].ledGreenGpio = EnetInfo.sw.ledInfo[1].speedLed1000;
            setLed(&gLedCtrl[kLedEth1Spd1000], kLedOff, kLedGreen);
        }
    }
#endif

#if defined(DEBUG_LED)
    for (i=0; i < kLedEnd; i++)
        printk("initLed: led[%d]: Gpio=0x%04x, FailGpio=0x%04x\n", i, gLedCtrl[i].ledGreenGpio, gLedCtrl[i].ledRedGpio);
#endif

}


// led ctrl.  Maps the ledName to the corresponding ledInfoPtr and perform the led operation
void boardLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    unsigned long flags;
    PLED_CTRL pLed;

    BCM_ASSERT_NOT_HAS_SPINLOCK_V(&brcm_ledlock);

    spin_lock_irqsave(&brcm_ledlock, flags);     // LED can be changed from ISR

    if( (int) ledName < kLedEnd ) {

        pLed = &gLedCtrl[ledName];

        // If the state is kLedStateFail and there is not a failure LED defined
        // in the board parameters, change the state to kLedStateSlowBlinkContinues.
        if( ledState == kLedStateFail && pLed->ledRedGpio == -1 )
            ledState = kLedStateSlowBlinkContinues;

        // Save current LED state
        pLed->ledState = ledState;

        // Start from LED Off and turn it on later as needed
        setLed (pLed, kLedOff, kLedGreen);
        setLed (pLed, kLedOff, kLedRed);

        // Disable HW control for WAN Data LED. 
        // It will be enabled only if LED state is On
 #if defined(AEI_VDSL_CUSTOMER_NCS)       
        AEI_SetWanLedHwControl(ledName,pLed,0);
#else
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362)
        if (ledName == kLedWanData)
            LED->ledHWDis |= GPIO_NUM_TO_MASK(pLed->ledGreenGpio);
#elif defined(CONFIG_BCM96368)
        if (ledName == kLedWanData) {
            GPIO->AuxLedCtrl |= AUX_HW_DISAB_2;
            if (pLed->ledGreenGpio & BP_ACTIVE_LOW)
                GPIO->AuxLedCtrl |= (LED_STEADY_ON << AUX_MODE_SHFT_2);
            else
                GPIO->AuxLedCtrl &= ~(LED_STEADY_ON << AUX_MODE_SHFT_2);
        }
#endif
 #endif
        switch (ledState) {
        case kLedStateOn:
            // Enable SAR to control INET LED
#if defined(AEI_VDSL_CUSTOMER_NCS)            
            AEI_SetWanLedHwControl(ledName,pLed,1);
#else
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362)
            if (ledName == kLedWanData)
                LED->ledHWDis &= ~GPIO_NUM_TO_MASK(pLed->ledGreenGpio);
#elif defined(CONFIG_BCM96368)
            if (ledName == kLedWanData) {
                GPIO->AuxLedCtrl &= ~AUX_HW_DISAB_2;
                if (pLed->ledGreenGpio & BP_ACTIVE_LOW)
                    GPIO->AuxLedCtrl &= ~(LED_STEADY_ON << AUX_MODE_SHFT_2);
                else
                    GPIO->AuxLedCtrl |= (LED_STEADY_ON << AUX_MODE_SHFT_2);
            }
#endif
#endif
            setLed (pLed, kLedOn, kLedGreen);
            break;

        case kLedStateOff:
            break;

        case kLedStateFail:
            setLed (pLed, kLedOn, kLedRed);
            break;

#if defined(AEI_VDSL_CUSTOMER_NCS)            
        case kLedStateAmber:
            setLed (pLed, kLedOn, kLedGreen);
            setLed (pLed, kLedOn, kLedRed);
            break;

        case kLedStateVerySlowBlinkContinues:
            pLed->blinkCountDown = kVerySlowBlinkCount;
            gTimerOnRequests++;
            break;

#endif

        case kLedStateSlowBlinkContinues:
            pLed->blinkCountDown = kSlowBlinkCount;
            gTimerOnRequests++;
            break;

        case kLedStateFastBlinkContinues:
            pLed->blinkCountDown = kFastBlinkCount;
            gTimerOnRequests++;
            break;

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
        case kLedStateAmberSlowBlinkContinues:
			gPowerLedStatus = 3;
            pLed->blinkCountDown = kOneSecondBlinkCount;
            gTimerOnRequests++;
            break;

        case kLedStateAmberAndGreenSlowBlinkContinues:
			gPowerLedStatus = 2;
            pLed->blinkCountDown = kOneSecondBlinkCount;
            gTimerOnRequests++;
            break;
#endif

        case kLedStateUserWpsInProgress:          /* 200ms on, 100ms off */
            pLed->blinkCountDown = kFastBlinkCount;
            gLedRunningCounter = 0;
            gTimerOnRequests++;
            break;             

        case kLedStateUserWpsError:               /* 100ms on, 100ms off */
            pLed->blinkCountDown = kFastBlinkCount;
            gLedRunningCounter = 0;
            gTimerOnRequests++;
            break;        

        case kLedStateUserWpsSessionOverLap:      /* 100ms on, 100ms off, 5 times, off for 500ms */
            pLed->blinkCountDown = kFastBlinkCount;
            gTimerOnRequests++;
            break;        
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
        case kLedStateUserWANGreenRedVerySlowBlinkContinues:      /*rotate between green and red in a 1 second on/off interval*/
            pLed->blinkCountDown = kVerySlowBlinkCount;
            gTimerOnRequests++;
            break;        
        case kLedStatePowerOneSecondBlinkContinues:
            pLed->blinkCountDown = kOneSecondBlinkCount;
            gTimerOnRequests++;
            break;
#endif        
        default:
            printk("Invalid led state\n");
        }
    }

    // If gTimerOn is false, that means ledTimerExpire has already finished
    // running and has not restarted the timer.  Then we can start it here.
    // Otherwise, we only leave the gTimerOnRequests > 0 which will be
    // noticed at the end of the ledTimerExpire function.
    if (!gTimerOn && gTimerOnRequests > 0)
    {
        ledTimerStart();
        gTimerOn = TRUE;
        gTimerOnRequests = 0;
    }
    spin_unlock_irqrestore(&brcm_ledlock, flags);
}
