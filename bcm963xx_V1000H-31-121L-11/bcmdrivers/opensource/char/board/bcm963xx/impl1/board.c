/*
<:copyright-BRCM:2002:GPL/GPL:standard

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
/***************************************************************************
* File Name  : board.c
*
* Description: This file contains Linux character device driver entry
*              for the board related ioctl calls: flash, get free kernel
*              page and dump kernel memory, etc.
*
*
***************************************************************************/
/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/if.h>
#include <linux/pci.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/smp.h>
#include <linux/version.h>
#include <linux/reboot.h>
#include <linux/bcm_assert_locks.h>
#include <asm/delay.h>
#ifdef AEI_CONFIG_JFFS
#include <linux/mtd/mtd.h>
#include <linux/syscalls.h>
#endif
#include <bcmnetlink.h>
#include <net/sock.h>
#include <bcm_map_part.h>
#include <board.h>
#include <spidevices.h>
#define  BCMTAG_EXE_USE
#include <bcmTag.h>
#include <boardparms.h>
#include <boardparms_voice.h>
#include <flash_api.h>
#include <bcm_intr.h>
#include <flash_common.h>
#include <bcmpci.h>
#include <linux/bcm_log.h>
#include <bcmSpiRes.h>

//extern unsigned int flash_get_reserved_bytes_at_end(const FLASH_ADDR_INFO *fInfo);

/* Typedefs. */

#if defined (WIRELESS)
#define SES_EVENT_BTN_PRESSED      0x00000001
#define SES_EVENTS                 SES_EVENT_BTN_PRESSED /*OR all values if any*/
#define SES_LED_OFF                0
#define SES_LED_ON                 1
#define SES_LED_BLINK              2

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
#define WLAN_ONBOARD_SLOT	WLAN_ONCHIP_DEV_SLOT
#else
#define WLAN_ONBOARD_SLOT       1 /* Corresponds to IDSEL -- EBI_A11/PCI_AD12 */
#endif

#define BRCM_VENDOR_ID       0x14e4
#define BRCM_WLAN_DEVICE_IDS 0x4300
#define BRCM_WLAN_DEVICE_IDS_DEC 43

#define WLAN_ON   1
#define WLAN_OFF  0

#if defined(AEI_VDSL_CUSTOMER_NCS)

#define WSC_PROC_IDLE         0
#define WSC_PROC_WAITING      1
#define WSC_PROC_SUCC         2
#define WSC_PROC_TIMEOUT      3
#define WSC_PROC_FAIL         4
#define WSC_PROC_M2_SENT      5
#define WSC_PROC_M7_SENT      6
#define WSC_PROC_MSG_DONE     7
#define WSC_PROC_PBC_OVERLAP  8
/*WPS SM State*/
#define WSC_EVENTS_PROC_START              2
#define WSC_EVENTS_PROC_IDLE               (WSC_PROC_IDLE + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_WAITING            (WSC_PROC_WAITING + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_SUCC               (WSC_PROC_SUCC  + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_TIMEOUT            (WSC_PROC_TIMEOUT + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_FAIL               (WSC_PROC_FAIL + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_M2_SENT            (WSC_PROC_M2_SENT + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_M7_SENT            (WSC_PROC_M7_SENT + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_MSG_DONE           (WSC_PROC_MSG_DONE + WSC_EVENTS_PROC_START)
#define WSC_EVENTS_PROC_PBC_OVERLAP        (WSC_PROC_PBC_OVERLAP + WSC_EVENTS_PROC_START)

#endif //AEI_VDSL_CUSTOMER_NCS

#endif

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
extern int gPowerLedStatus;
#endif

#ifdef AEI_NAND_IMG_CHECK
int gSetWrongCRC = 0; //1=set wrong crc
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
AEI_BOARD_ID aeiBoardId = AEI_BOARD_UNKNOWN;
#endif

typedef struct
{
    unsigned long ulId;
    char chInUse;
    char chReserved[3];
} MAC_ADDR_INFO, *PMAC_ADDR_INFO;

typedef struct
{
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    MAC_ADDR_INFO MacAddrs[1];
} MAC_INFO, *PMAC_INFO;

typedef struct
{
    unsigned char gponSerialNumber[NVRAM_GPON_SERIAL_NUMBER_LEN];
    unsigned char gponPassword[NVRAM_GPON_PASSWORD_LEN];
} GPON_INFO, *PGPON_INFO;

typedef struct
{
    unsigned long eventmask;
} BOARD_IOC, *PBOARD_IOC;


/*Dyinggasp callback*/
typedef void (*cb_dgasp_t)(void *arg);
typedef struct _CB_DGASP__LIST
{
    struct list_head list;
    char name[IFNAMSIZ];
    cb_dgasp_t cb_dgasp_fn;
    void *context;
}CB_DGASP_LIST , *PCB_DGASP_LIST;


/* Externs. */
extern struct file *fget_light(unsigned int fd, int *fput_needed);
extern unsigned long getMemorySize(void);
extern void __init boardLedInit(void);
extern void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);

/* Prototypes. */
static void set_mac_info( void );
static void set_gpon_info( void );
static int board_open( struct inode *inode, struct file *filp );
static int board_ioctl( struct inode *inode, struct file *flip, unsigned int command, unsigned long arg );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos);
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);

static BOARD_IOC* borad_ioc_alloc(void);
static void borad_ioc_free(BOARD_IOC* board_ioc);

/*
 * flashImageMutex must be acquired for all write operations to
 * nvram, CFE, or fs+kernel image.  (cfe and nvram may share a sector).
 */
DEFINE_MUTEX(flashImageMutex);
static void writeNvramDataCrcLocked(PNVRAM_DATA pNvramData);
static PNVRAM_DATA readNvramData(void);

/* DyingGasp function prototype */
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id);
static void __init kerSysInitDyingGaspHandler( void );
static void __exit kerSysDeinitDyingGaspHandler( void );
/* -DyingGasp function prototype - */
/* dgaspMutex protects list add and delete, but is ignored during isr. */
static DEFINE_MUTEX(dgaspMutex);

static int ConfigCs(BOARD_IOCTL_PARMS *parms);


#if defined (WIRELESS)
static irqreturn_t sesBtn_isr(int irq, void *dev_id);
static void __init sesBtn_mapIntr(int context);
static Bool sesBtn_pressed(void);
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait);
static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos);
static void __init sesLed_mapGpio(void);
static void sesLed_ctrl(int action);
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
static void AEI_wlanLed_ctrl(int action);
#endif
#if defined(AEI_VOIP_LED)
static void AEI_VoipLed_ctrl(char *action);
#endif
static void __init ses_board_init(void);
static void __exit ses_board_deinit(void);
static void __init kerSysScreenPciDevices(void);
static void kerSetWirelessPD(int state);
#endif
#if defined(AEI_VDSL_DOWNGRADE_NVRAM_ADJUST)
static UBOOL8 AEI_DownGrade_AdjustNVRAM(void);
static UBOOL8 AEI_IsDownGrade_FromSDK12To6(char* string);
#endif
#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
static void __init kerSysCheckPowerDownPcie(void);
#endif

static void str_to_num(char* in, char *out, int len);
static int add_proc_files(void);
static int del_proc_files(void);
static int proc_get_param(char *page, char **start, off_t off, int cnt, int *eof, void *data);
static int proc_set_param(struct file *f, const char *buf, unsigned long cnt, void *data);
static int proc_set_led(struct file *f, const char *buf, unsigned long cnt, void *data);

static irqreturn_t reset_isr(int irq, void *dev_id);

#if defined(AEI_VDSL_CUSTOMER_QWEST)
 #define RESET_HOLD_TIME		10
 #define FACTORY_HOLD_TIME	20
#elif defined(AEI_VDSL_CUSTOMER_TELUS)
 #define NOT_LONG_ENOUGH_TIME    3
 #define FACTORY_HOLD_TIME	30
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
#define RESET_POLL_TIME		1
static int holdTime = 0;
static struct timer_list resetBtnTimer, *pTimer = NULL;

#if defined(AEI_VDSL_CUSTOMER_TELUS)
static unsigned short rirq = BP_NOT_DEFINED;
#endif
#endif

// macAddrMutex is used by kerSysGetMacAddress and kerSysReleaseMacAddress
// to protect access to g_pMacInfo
static DEFINE_MUTEX(macAddrMutex);
static PMAC_INFO g_pMacInfo = NULL;
static PGPON_INFO g_pGponInfo = NULL;
static unsigned long g_ulSdramSize;
#if defined(CONFIG_BCM96368)
static unsigned long g_ulSdramWidth;
#endif
static int g_ledInitialized = 0;
static wait_queue_head_t g_board_wait_queue;
static CB_DGASP_LIST *g_cb_dgasp_list_head = NULL;

#define MAX_PAYLOAD_LEN 64
static struct sock *g_monitor_nl_sk;
static int g_monitor_nl_pid = 0 ;
static void kerSysInitMonitorSocket( void );
static void kerSysCleanupMonitorSocket( void );

#if defined(CONFIG_BCM96368)
static void ChipSoftReset(void);
static void ResetPiRegisters( void );
static void PI_upper_set( volatile uint32 *PI_reg, int newPhaseInt );
static void PI_lower_set( volatile uint32 *PI_reg, int newPhaseInt );
static void TurnOffSyncMode( void );
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
static int  AEI_get_flash_mafId(void);
#endif
#if defined(CONFIG_BCM96816)
void board_Init6829( void );
#endif

/* restore default work structure */
static struct work_struct restoreDefaultWork;

static struct file_operations board_fops =
{
    open:       board_open,
    ioctl:      board_ioctl,
    poll:       board_poll,
    read:       board_read,
    release:    board_release,
};

uint32 board_major = 0;

#if defined (WIRELESS)
static unsigned short sesBtn_irq = BP_NOT_DEFINED;
static unsigned short sesLed_gpio = BP_NOT_DEFINED;
#endif

#if defined(MODULE)
int init_module(void)
{
    return( brcm_board_init() );
}

void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("brcm flash: cleanup_module failed because module is in use\n");
    else
        brcm_board_cleanup();
}
#endif //MODULE

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
#include <linux/syscalls.h>
#define BCM_SYSLOG_MAX_LINE_SIZE 256
static int AEI_SaveSyslogOnReboot()
{
    int ret = 0;
    long cfs = 0;
    cfs = sys_open("/tmp/clearonreboot", O_RDONLY, 0);
    if (cfs < 0) 
    {
        FLASH_ADDR_INFO fInfo;
        kerSysFlashAddrInfoGet(&fInfo);
        if (fInfo.flash_syslog_length >0)
        {
             struct file *fp; 
             int readlen=0;
             char *savebuf,*tempbuf;
             fp = filp_open("/tmp/syslogbak", O_RDONLY, 0);
             if(!IS_ERR(fp) && fp->f_op && fp->f_op->read && fp->f_op->llseek) 
             {
#if defined(AEI_CONFIG_JFFS)
                 int flen;
                 fp->f_op->llseek(fp,0, SEEK_END);
                 flen = fp->f_pos;
                 fp->f_op->llseek(fp,0, SEEK_SET);
                 tempbuf = (char*)kmalloc(flen, GFP_ATOMIC);
                 memset(tempbuf,0,flen);
                 savebuf = (char*)kmalloc(flen+16, GFP_ATOMIC);
                 memset(savebuf,0,flen+16);
#else
                 tempbuf = (char*)kmalloc(fInfo.flash_syslog_length, GFP_ATOMIC);
                 memset(tempbuf,0,fInfo.flash_syslog_length);
                 savebuf = (char*)kmalloc(fInfo.flash_syslog_length, GFP_ATOMIC);
                 memset(savebuf,0,fInfo.flash_syslog_length);
#endif             
                 mm_segment_t fs = get_fs();
                 set_fs(get_ds());
#if defined(AEI_CONFIG_JFFS)
                 readlen = fp->f_op->read(fp, (void *) tempbuf,flen,&fp->f_pos);
#else
                 fp->f_op->llseek(fp, -(fInfo.flash_syslog_length-16), SEEK_END);
                 readlen = fp->f_op->read(fp, (void *) tempbuf, fInfo.flash_syslog_length-16,&fp->f_pos);
#endif
                 if(readlen >0)
                 {
                     int savebuflen=0;      
                     int i = 0;              
                     char line[BCM_SYSLOG_MAX_LINE_SIZE];
                     char *pblank = NULL;
                     int tzlen = 0; 
                     char *linestart,*lineend;
                     char *savebuf_ptr;
                     
                     savebuf_ptr = savebuf+12;
                     linestart = tempbuf;
                     lineend = strchr(linestart,'\n');
                     while(lineend){
                         memset(line,0,BCM_SYSLOG_MAX_LINE_SIZE);
                         if((lineend - linestart)<= BCM_SYSLOG_MAX_LINE_SIZE)
                             strncpy(line,linestart,lineend - linestart +1);
                         else
                             strncpy(line,linestart,BCM_SYSLOG_MAX_LINE_SIZE );

                         linestart = lineend+1;
                         lineend = strchr(linestart,'\n');

                         if((pblank = strchr(line, ')')) != NULL&&(line[0]=='('))
                             tzlen = pblank - line + 1;
                         else
                             continue;

                         if (strlen(line) < tzlen+25 || line[tzlen-1] != ')' || line[tzlen+4] != ' ' ||
                               line[tzlen+8] != '-' || line[tzlen+12] != ' ' || line[tzlen+15] != ' ' ||
                               line[tzlen+24] != ' ')
                             continue;                                            
                         strcat(savebuf_ptr,line);
                         savebuflen += strlen(line);    
               //          printk("%d####%s",strlen(line),line);  
                    }
                    set_fs(fs);
                    if(savebuflen>0)
                    {
                        char header[16];
                        snprintf(header,sizeof(header),"SYSLOG%06d",savebuflen+16);
                        memcpy(savebuf,header,12);
                        ret = kerSysSyslogSet(savebuf, savebuflen+16,0); 
                    }
                    kfree(savebuf);
                }
                filp_close(fp, NULL);
                kfree(tempbuf);
            }           
        }
    }
    return ret;
}
#endif

static int map_external_irq (int irq)
{
    int map_irq;

    switch (irq) {
    case BP_EXT_INTR_0   :
        map_irq = INTERRUPT_ID_EXTERNAL_0;
        break ;
    case BP_EXT_INTR_1   :
        map_irq = INTERRUPT_ID_EXTERNAL_1;
        break ;
    case BP_EXT_INTR_2   :
        map_irq = INTERRUPT_ID_EXTERNAL_2;
        break ;
    case BP_EXT_INTR_3   :
        map_irq = INTERRUPT_ID_EXTERNAL_3;
        break ;
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    case BP_EXT_INTR_4   :
        map_irq = INTERRUPT_ID_EXTERNAL_4;
        break ;
    case BP_EXT_INTR_5   :
        map_irq = INTERRUPT_ID_EXTERNAL_5;
        break ;
#endif
    default           :
        printk ("Invalid External Interrupt definition \n") ;
        map_irq = 0 ;
        break ;
    }
    return (map_irq) ;
}

/* A global variable used by Power Management and other features to determine if Voice is idle or not */
volatile int isVoiceIdle = 1;
EXPORT_SYMBOL(isVoiceIdle);

#if defined(CONFIG_BCM_AVS_PWRSAVE)
/* A generic kernel thread for board related maintenance activities */
/* Only used if AVS is configured, but feel free to change */
static DECLARE_COMPLETION(poll_done);
static atomic_t poll_lock = ATOMIC_INIT(1);
static int poll_pid = -1;

/* The AVS algorith lowers 1V2 until the slowest ring osc (the one with
   the largest count) reaches the RING_OSC_LOWER_BOUND. Then the algorithm
   will re-increase the voltage if the ring osc continues to slow down to a
   point where its count exceeds the RING_OSC_UPPER_BOUND. */

#if defined(CONFIG_BCM_PWRMNGT_MODULE)
int AvsEnabled = -1; // Wait for the module to control if it is enabled or not
#else
int AvsEnabled = 1; // There is no control so force it to be enabled
#endif
void kerSysBcmEnableAvs(int enable)
{
    unsigned short vregVsel1P2;

    /* Set target 1V2 level */
    if (BpGetVregSel1P2(&vregVsel1P2) == BP_SUCCESS ) {
        AvsEnabled = -1;
        printk("Adaptive Voltage Scaling is disabled because 1V2 is forced to a specific level by design\n");
    } else {
        AvsEnabled = enable;
        printk("Adaptive Voltage Scaling is now %s\n", (enable==1?"enabled":(enable==0?"disabled":"stopped")));
    }
}

int kerSysBcmAvsEnabled(void)
{
    return AvsEnabled;
}

#if defined(CONFIG_BCM96362)
/* 36483, Highest ring osc count read during successful PVT
   a small margin is included to avoid ever reaching this highest count */
//#define RING_OSC_UPPER_BOUND (0x8E83 - 100)
#define RING_OSC_UPPER_BOUND (0x6F1C - 100) // 3.6 MHz + tidbits
/* 1100 is about the max ring osc variation when doing a 1V2 step change */
#define RING_OSC_LOWER_BOUND (RING_OSC_UPPER_BOUND - 1100)
/* 1V2 is set by default to 1.225. HW team is asking that we don't allow the
   voltage to go below 1.19 V. */
#define VREG_VSEL1P2_LOWER_BOUND ((VREG_VSEL1P2_MIDDLE+1) - 3)
/* 6362 Slow parts need to have their voltage increased to avoid WLAN issues
   This is the threshold we use to identify what looks like a slow part.
   A large enough delta is needed between this threshold and the 
   RING_OSC_UPPER_BOUND (~2 MHz here) */
#define RING_OSC_SS_UPPER_BOUND (0x75A5) // 3.4 MHz
#define VREG_VSEL1P2_SS_TARGET   (VREG_VSEL1P2_MIDDLE+1)
#endif
#if defined(CONFIG_BCM96816)
/* PVT suggests we can go as high as A6AA but choosing to be conservative */
#define RING_OSC_UPPER_BOUND (0x9000 - 100)
#define RING_OSC_LOWER_BOUND (RING_OSC_UPPER_BOUND - 1100)
#define VREG_VSEL1P2_LOWER_BOUND ((VREG_VSEL1P2_MIDDLE+1) - 4)
#endif
#if defined(CONFIG_BCM96368)
#define RING_OSC_UPPER_BOUND (0x84FA - 100)
#define RING_OSC_LOWER_BOUND (RING_OSC_UPPER_BOUND - 1500)
#define VREG_VSEL1P2_LOWER_BOUND  ((VREG_VSEL1P2_MIDDLE+1) - 4)
#endif
#if defined(CONFIG_BCM96328)
#define RING_OSC_UPPER_BOUND
#define RING_OSC_LOWER_BOUND
#define VREG_VSEL1P2_LOWER_BOUND
#endif
#if defined(CONFIG_BCM963268) // untested
#define RING_OSC_UPPER_BOUND (0x84FA - 100)
#define RING_OSC_LOWER_BOUND (RING_OSC_UPPER_BOUND - 1500)
#define VREG_VSEL1P2_LOWER_BOUND  ((VREG_VSEL1P2_MIDDLE+1) - 4)
#endif

#define AVSDEBUG(f, ...)
//#define AVSDEBUG(f, ...) printk(f, __VA_ARGS__)

static void brcm_adaptive_voltage_scaling(void)
{
    static int max_count = 0;
    static int is_ss_part = 0;
    int ring_osc_select = 0;
    int current_1v2 = 0;
    int next_1v2 = 0;
    uint32 RingOscCtrl1 = GPIO->RingOscCtrl1;

    /* Verify is AVS is not forced off */
    if (AvsEnabled != -1) {
        /* Verify if the ring oscillator has completed a measurement */
        /* This will only fail on the very first call to this function */
        if (RingOscCtrl1 & RING_OSC_IRQ)
        {
            AVSDEBUG("Read ring osc %d: %lx\n",
                    (RingOscCtrl1 & RING_OSC_SELECT_MASK) >> RING_OSC_SELECT_SHIFT,
                     RingOscCtrl1 & RING_OSC_COUNT_MASK);
            if ((RingOscCtrl1 & RING_OSC_COUNT_MASK) > max_count)
            {
                max_count = RingOscCtrl1 & RING_OSC_COUNT_MASK;
                AVSDEBUG("max_count: %x\n", max_count);
            }

            /* Move to the next enabled ring osc */
            ring_osc_select = (RingOscCtrl1 & RING_OSC_SELECT_MASK) >> RING_OSC_SELECT_SHIFT;
            while (++ring_osc_select < RING_OSC_MAX)
            {
                if ((((1<<ring_osc_select)<<RING_OSC_ENABLE_SHIFT) & RING_OSC_ENABLE_MASK) != 0)
                {
                    break;
                }
            }

            /* If we have read all ring osc, determine if the voltage should be changed */
            if (ring_osc_select == RING_OSC_MAX)
            {
                /* All ring osc have been read, prepare for the next round */
                /* 0 is always a valid ring osc so no need to verify if it is enabled */
                ring_osc_select = 0;

            /* Check if the voltage should be adjusted */
            if ((max_count < RING_OSC_LOWER_BOUND) && AvsEnabled && !is_ss_part)
            {
                /* The ring osc is too fast, reduce the voltage if it is not too low */
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
                    current_1v2 = (GPIO->VregConfig & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
#elif defined(CONFIG_BCM96362) 
                    current_1v2 = (MISC->miscVregCtrl1 & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
#else
                    current_1v2 = (MISC->miscVregCtrl0 & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
#endif
                    next_1v2 = current_1v2;
                    if (current_1v2 == 0)
                    {
                        next_1v2 = VREG_VSEL1P2_MIDDLE;
                    }
                    else if (current_1v2 > VREG_VSEL1P2_LOWER_BOUND)
                    {
                        next_1v2--;
                    }
                    AVSDEBUG("ring_osc is fast, can reduce voltage: %d to %d\n", current_1v2, next_1v2);
                }
            else if ((max_count > RING_OSC_UPPER_BOUND) || !AvsEnabled || is_ss_part)
            {
                /* The ring osc is too slow, increase the voltage up to the default of 0 */
                /* If AVS is disabled, we need to force the voltage to come back up to default */
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
                current_1v2 = (GPIO->VregConfig & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
#elif defined(CONFIG_BCM96362) 
                current_1v2 = (MISC->miscVregCtrl1 & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
#else
                current_1v2 = (MISC->miscVregCtrl0 & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
#endif
                next_1v2 = current_1v2;

#if defined(CONFIG_BCM96362)
                /* On 6362, we try to identify SS parts to increase their voltage
                   to help WLAN performance */
                if (AvsEnabled && (max_count > RING_OSC_SS_UPPER_BOUND))
                {
                    is_ss_part = 1;
                }

                if (is_ss_part)
                {
                    if (current_1v2 == VREG_VSEL1P2_MIDDLE)
                    {
                        next_1v2 = 0;
                    }
                    else if (current_1v2 == 0)
                    {
                        next_1v2 = VREG_VSEL1P2_MIDDLE+1;
                    }
                    else if (current_1v2 < VREG_VSEL1P2_SS_TARGET)
                    {
                        next_1v2++;
                    }
                } else
#endif
                if (current_1v2 == VREG_VSEL1P2_MIDDLE)
                {
                    next_1v2 = 0;
                }
                else if (current_1v2 != 0)
                {
                    next_1v2++;
                }
                AVSDEBUG("ring_osc is slow, can increase voltage: %d to %d\n", current_1v2, next_1v2);
            }
            if (next_1v2 != current_1v2)
            {
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
                    GPIO->VregConfig = (GPIO->VregConfig & ~VREG_VSEL1P2_MASK) | (next_1v2 << VREG_VSEL1P2_SHIFT);
#elif defined(CONFIG_BCM96362) 
                    MISC->miscVregCtrl1 = (MISC->miscVregCtrl1 & ~VREG_VSEL1P2_MASK) | (next_1v2 << VREG_VSEL1P2_SHIFT);
#else
                    MISC->miscVregCtrl0 = (MISC->miscVregCtrl0 & ~VREG_VSEL1P2_MASK) | (next_1v2 << VREG_VSEL1P2_SHIFT);
#endif
                    AVSDEBUG("Adjusted voltage: %d to %d\n", current_1v2, next_1v2);
                }
                max_count = 0;
            }
        }         

        /* Start a new ring osc count cycle by resetting the counter */
        GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK |
                             RING_OSC_COUNT_RESET;
        GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK |
                             (ring_osc_select << RING_OSC_SELECT_SHIFT);
        /* Writing to this register starts the count */
        GPIO->RingOscCtrl0 = RING_OSC_512_CYCLES;
    }
}

static int brcm_board_timer( void *data )
{
    daemonize("board-timer");

    while (atomic_read(&poll_lock) > 0)
    {
        brcm_adaptive_voltage_scaling();

        /* Sleep for 1 second (HZ jiffies) */
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ);
    }

    complete_and_exit(&poll_done, 0);
    printk("brcm_board_timer: thread exits!\n");

}
#endif

#if defined(CONFIG_BCM96368)
static unsigned long getMemoryWidth(void)
{
    unsigned long memCfg;

    memCfg = MEMC->Config;
    memCfg &= MEMC_WIDTH_MASK;
    memCfg >>= MEMC_WIDTH_SHFT;

    return memCfg;
}
#endif

static int __init brcm_board_init( void )
{
    unsigned short rstToDflt_irq;
    int ret;
    bcmLogSpiCallbacks_t loggingCallbacks;
#if defined(AEI_VDSL_CUSTOMER_NCS)
    char board_id[32]={0};
#endif

    ret = register_chrdev(BOARD_DRV_MAJOR, "brcmboard", &board_fops );
    if (ret < 0)
        printk( "brcm_board_init(major %d): fail to register device.\n",BOARD_DRV_MAJOR);
    else
    {
        printk("brcmboard: brcm_board_init entry\n");
        board_major = BOARD_DRV_MAJOR;

        g_ulSdramSize = getMemorySize();
#if defined(CONFIG_BCM96368)
        g_ulSdramWidth = getMemoryWidth();
#endif
        set_mac_info();
        set_gpon_info();

        init_waitqueue_head(&g_board_wait_queue);
#if defined (WIRELESS)
        kerSysScreenPciDevices();
        ses_board_init();
        kerSetWirelessPD(WLAN_ON);
#endif
#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
        kerSysCheckPowerDownPcie();
#endif
        kerSysInitMonitorSocket();
        kerSysInitDyingGaspHandler();

        boardLedInit();
        g_ledInitialized = 1;

        if( BpGetResetToDefaultExtIntr(&rstToDflt_irq) == BP_SUCCESS )
        {
            rstToDflt_irq = map_external_irq (rstToDflt_irq) ;
            BcmHalMapInterrupt((FN_HANDLER)reset_isr, 0, rstToDflt_irq);
            BcmHalInterruptEnable(rstToDflt_irq);
        }

#if defined(CONFIG_BCM_CPLD1)
        // Reserve SPI bus to control external CPLD for Standby Timer
        BcmCpld1Initialize();
#endif

#if defined(CONFIG_BCM_AVS_PWRSAVE)
        poll_pid = kernel_thread(brcm_board_timer, NULL, CLONE_KERNEL);
#if !defined(CONFIG_BCM_PWRMNGT_MODULE)
    // Show that AVS is enabled when PWRMGNT control is not compiled in
    printk("Adaptive Voltage Scaling is always enabled\n");
#endif
#endif

    }

    add_proc_files();

#if defined(CONFIG_BCM96816)
    board_Init6829();
    loggingCallbacks.kerSysSlaveRead   = kerSysBcmSpiSlaveRead;
    loggingCallbacks.kerSysSlaveWrite  = kerSysBcmSpiSlaveWrite;
    loggingCallbacks.bpGet6829PortInfo = BpGet6829PortInfo;
#else
    loggingCallbacks.kerSysSlaveRead   = NULL;
    loggingCallbacks.kerSysSlaveWrite  = NULL;
    loggingCallbacks.bpGet6829PortInfo = NULL;
#endif
    loggingCallbacks.reserveSlave      = BcmSpiReserveSlave;
    loggingCallbacks.syncTrans         = BcmSpiSyncTrans;
    bcmLog_registerSpiCallbacks(loggingCallbacks);

#if defined(AEI_VDSL_CUSTOMER_NCS)
    kerSysNvRamGetBoardId(board_id);

    if (strstr(board_id, "C2000") != NULL)
        aeiBoardId = AEI_BOARD_C2000A;
    else if (strstr(board_id, "V2000") != NULL)
        aeiBoardId = AEI_BOARD_V2000H;
    else if (strstr(board_id, "FV2200") != NULL)
        aeiBoardId = AEI_BOARD_FV2200;
    else if (strstr(board_id, "V1000H") != NULL)
        aeiBoardId = AEI_BOARD_V1000H;
    else if (strstr(board_id, "VB784WG") != NULL)
        aeiBoardId = AEI_BOARD_V1000H;
    else if (strstr(board_id, "V2200H") != NULL)
        aeiBoardId = AEI_BOARD_V2200H;
#endif

    return ret;
}

static void __init set_mac_info( void )
{
    NVRAM_DATA *pNvramData;
    unsigned long ulNumMacAddrs;

    if (NULL == (pNvramData = readNvramData()))
    {
        printk("set_mac_info: could not read nvram data\n");
        return;
    }

    ulNumMacAddrs = pNvramData->ulNumMacAddrs;

#if defined(AEI_VDSL_CUSTOMER_NCS)
    /* If MAC OUI is 00-26-88, then re-write to 10:9F:A9 not 40-8b-07 */
    if (pNvramData->ucaBaseMacAddr[0]==0x0 && pNvramData->ucaBaseMacAddr[1]==0x26 && pNvramData->ucaBaseMacAddr[2]==0x88)
    {
        int spot = sizeof(pNvramData->chUnused);
        pNvramData->ucaBaseMacAddr[0]=0x10;
        pNvramData->ucaBaseMacAddr[1]=0x9f;
        pNvramData->ucaBaseMacAddr[2]=0xa9;
        /* need to find new storage for CL images to mark if MAC address rewritten 
           as new SDK does not preserve chReserved so use last 2 bytes in chUnused as that is constant place
         */ 
        //pNvramData->chReserved[0]='f';
        //pNvramData->chReserved[1]='u'; 
        if (spot > 0)
           pNvramData->chUnused[spot-1]='u';
        if (spot > 1)
           pNvramData->chUnused[spot-2]='f';
        writeNvramDataCrcLocked(pNvramData); 
    }
#endif

    if( ulNumMacAddrs > 0 && ulNumMacAddrs <= NVRAM_MAC_COUNT_MAX )
    {
        unsigned long ulMacInfoSize =
#if defined(AEI_VDSL_CUSTOMER_NCS)
            sizeof(MAC_INFO) + ((sizeof(MAC_ADDR_INFO)) * (ulNumMacAddrs-1));
#else
            sizeof(MAC_INFO) + ((sizeof(MAC_ADDR_INFO) - 1) * ulNumMacAddrs);
#endif

        g_pMacInfo = (PMAC_INFO) kmalloc( ulMacInfoSize, GFP_KERNEL );

        if( g_pMacInfo )
        {
            memset( g_pMacInfo, 0x00, ulMacInfoSize );
            g_pMacInfo->ulNumMacAddrs = pNvramData->ulNumMacAddrs;
            memcpy( g_pMacInfo->ucaBaseMacAddr, pNvramData->ucaBaseMacAddr,
                NVRAM_MAC_ADDRESS_LEN );
        }
        else
            printk("ERROR - Could not allocate memory for MAC data\n");
    }
    else
        printk("ERROR - Invalid number of MAC addresses (%ld) is configured.\n",
        ulNumMacAddrs);
    kfree(pNvramData);
}

static int gponParamsAreErased(NVRAM_DATA *pNvramData)
{
    int i;
    int erased = 1;

    for(i=0; i<NVRAM_GPON_SERIAL_NUMBER_LEN-1; ++i) {
        if((pNvramData->gponSerialNumber[i] != (char) 0xFF) &&
            (pNvramData->gponSerialNumber[i] != (char) 0x00)) {
                erased = 0;
                break;
        }
    }

    if(!erased) {
        for(i=0; i<NVRAM_GPON_PASSWORD_LEN-1; ++i) {
            if((pNvramData->gponPassword[i] != (char) 0xFF) &&
                (pNvramData->gponPassword[i] != (char) 0x00)) {
                    erased = 0;
                    break;
            }
        }
    }

    return erased;
}

static void __init set_gpon_info( void )
{
    NVRAM_DATA *pNvramData;

    if (NULL == (pNvramData = readNvramData()))
    {
        printk("set_gpon_info: could not read nvram data\n");
        return;
    }

    g_pGponInfo = (PGPON_INFO) kmalloc( sizeof(GPON_INFO), GFP_KERNEL );

    if( g_pGponInfo )
    {
        if ((pNvramData->ulVersion < NVRAM_FULL_LEN_VERSION_NUMBER) ||
            gponParamsAreErased(pNvramData))
        {
            strcpy( g_pGponInfo->gponSerialNumber, DEFAULT_GPON_SN );
            strcpy( g_pGponInfo->gponPassword, DEFAULT_GPON_PW );
        }
        else
        {
            strncpy( g_pGponInfo->gponSerialNumber, pNvramData->gponSerialNumber,
                NVRAM_GPON_SERIAL_NUMBER_LEN );
            g_pGponInfo->gponSerialNumber[NVRAM_GPON_SERIAL_NUMBER_LEN-1]='\0';
            strncpy( g_pGponInfo->gponPassword, pNvramData->gponPassword,
                NVRAM_GPON_PASSWORD_LEN );
            g_pGponInfo->gponPassword[NVRAM_GPON_PASSWORD_LEN-1]='\0';
        }
    }
    else
    {
        printk("ERROR - Could not allocate memory for GPON data\n");
    }
    kfree(pNvramData);
}

void __exit brcm_board_cleanup( void )
{
    printk("brcm_board_cleanup()\n");
    del_proc_files();

    if (board_major != -1)
    {
#if defined (WIRELESS)
        ses_board_deinit();
#endif
        kerSysDeinitDyingGaspHandler();
        kerSysCleanupMonitorSocket();
        unregister_chrdev(board_major, "board_ioctl");

#if defined(CONFIG_BCM_AVS_PWRSAVE)
        if (poll_pid >= 0)
        {
            atomic_dec(&poll_lock);
            wait_for_completion(&poll_done);
        }
#endif
    }
}

static BOARD_IOC* borad_ioc_alloc(void)
{
    BOARD_IOC *board_ioc =NULL;
    board_ioc = (BOARD_IOC*) kmalloc( sizeof(BOARD_IOC) , GFP_KERNEL );
    if(board_ioc)
    {
        memset(board_ioc, 0, sizeof(BOARD_IOC));
    }
    return board_ioc;
}

static void borad_ioc_free(BOARD_IOC* board_ioc)
{
    if(board_ioc)
    {
        kfree(board_ioc);
    }
}


static int board_open( struct inode *inode, struct file *filp )
{
    filp->private_data = borad_ioc_alloc();

    if (filp->private_data == NULL)
        return -ENOMEM;

    return( 0 );
}

static int board_release(struct inode *inode, struct file *filp)
{
    BOARD_IOC *board_ioc = filp->private_data;

    wait_event_interruptible(g_board_wait_queue, 1);
    borad_ioc_free(board_ioc);

    return( 0 );
}


static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
#if defined (WIRELESS)
    BOARD_IOC *board_ioc = filp->private_data;
#endif

    poll_wait(filp, &g_board_wait_queue, wait);
#if defined (WIRELESS)
    if(board_ioc->eventmask & SES_EVENTS){
        mask |= sesBtn_poll(filp, wait);
    }
#endif

    return mask;
}

static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos)
{
#if defined (WIRELESS)
    BOARD_IOC *board_ioc = filp->private_data;
    if(board_ioc->eventmask & SES_EVENTS){
        return sesBtn_read(filp, buffer, count, ppos);
    }
#endif
    return 0;
}

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
static UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

/** calculate the CRC for the nvram data block and write it to flash.
 * Must be called with flashImageMutex held.
 */
static void writeNvramDataCrcLocked(PNVRAM_DATA pNvramData)
{
    UINT32 crc = CRC32_INIT_VALUE;

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);

    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, sizeof(NVRAM_DATA), crc);
    pNvramData->ulCheckSum = crc;
    kerSysNvRamSet((char *)pNvramData, sizeof(NVRAM_DATA), 0);
}


/** read the nvramData struct from the in-memory copy of nvram.
 * The caller is not required to have flashImageMutex when calling this
 * function.  However, if the caller is doing a read-modify-write of
 * the nvram data, then the caller must hold flashImageMutex.  This function
 * does not know what the caller is going to do with this data, so it
 * cannot assert flashImageMutex held or not when this function is called.
 *
 * @return pointer to NVRAM_DATA buffer which the caller must free
 *         or NULL if there was an error
 */
static PNVRAM_DATA readNvramData(void)
{
    UINT32 crc = CRC32_INIT_VALUE, savedCrc;
    NVRAM_DATA *pNvramData;

    // use GFP_ATOMIC here because caller might have flashImageMutex held
    if (NULL == (pNvramData = kmalloc(sizeof(NVRAM_DATA), GFP_ATOMIC)))
    {
        printk("readNvramData: could not allocate memory\n");
        return NULL;
    }

    kerSysNvRamGet((char *)pNvramData, sizeof(NVRAM_DATA), 0);
    savedCrc = pNvramData->ulCheckSum;
    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, sizeof(NVRAM_DATA), crc);
    if (savedCrc != crc)
    {
        // this can happen if we write a new cfe image into flash.
        // The new image will have an invalid nvram section which will
        // get updated to the inMemNvramData.  We detect it here and
        // commonImageWrite will restore previous copy of nvram data.
        kfree(pNvramData);
        pNvramData = NULL;
    }

    return pNvramData;
}
#if defined(AEI_VDSL_CUSTOMER_NCS)
static int AEI_readNvramData(PNVRAM_DATA pNvramData)
{
    UINT32 crc = CRC32_INIT_VALUE, savedCrc;

    kerSysNvRamGet((char *)pNvramData, sizeof(NVRAM_DATA), 0);
    savedCrc = pNvramData->ulCheckSum;
    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, sizeof(NVRAM_DATA), crc);
    if (savedCrc != crc)
        return -1;

    return 0;
}
#endif
//**************************************************************************************
// Utitlities for dump memory, free kernel pages, mips soft reset, etc.
//**************************************************************************************

/***********************************************************************
* Function Name: dumpaddr
* Description  : Display a hex dump of the specified address.
***********************************************************************/
void dumpaddr( unsigned char *pAddr, int nLen )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i, j;
    unsigned long ul;

    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (unsigned long) pAddr );
        p = szLine + strlen(szLine);

        for(i = 0; i < 16 && nLen > 0; i += sizeof(long), nLen -= sizeof(long))
        {
            ul = *(unsigned long *) &pAddr[i];
            q = (unsigned char *) &ul;
            for( j = 0; j < sizeof(long); j++ )
            {
                *p++ = szHexChars[q[j] >> 4];
                *p++ = szHexChars[q[j] & 0x0f];
                *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pAddr[j];
            *p++ = (ch > ' ' && ch < '~') ? ch : '.';
        }

        *p++ = '\0';
        printk( "%s\r\n", szLine );

        pAddr += i;
    }
    printk( "\r\n" );
} /* dumpaddr */


/** this function actually does two things, stop other cpu and reset mips.
 * Kept the current name for compatibility reasons.  Image upgrade code
 * needs to call the two steps separately.
 */
void kerSysMipsSoftReset(void)
{
	unsigned long cpu;
	cpu = smp_processor_id();
	printk(KERN_INFO "kerSysMipsSoftReset: called on cpu %lu\n", cpu);

	stopOtherCpu();
	local_irq_disable();  // ignore interrupts, just execute reset code now
	resetPwrmgmtDdrMips();
}

extern void stop_other_cpu(void);  // in arch/mips/kernel/smp.c

void stopOtherCpu(void)
{
#if defined(CONFIG_SMP)
    stop_other_cpu();
#elif defined(CONFIG_BCM_ENDPOINT_MODULE) && defined(CONFIG_BCM_BCMDSP_MODULE)
    unsigned long cpu = (read_c0_diag3() >> 31) ? 0 : 1;

	// Disable interrupts on the other core and allow it to complete processing 
	// and execute the "wait" instruction
    printk(KERN_INFO "stopOtherCpu: stopping cpu %lu\n", cpu);	
    PERF->IrqControl[cpu].IrqMask = 0;
    mdelay(5);
#endif
}

void resetPwrmgmtDdrMips(void)
{
#if !defined (CONFIG_BCM96816)
    // Power Management on Ethernet Ports may have brought down EPHY PLL
    // and soft reset below will lock-up 6362 if the PLL is not up
    // therefore bring it up here to give it time to stabilize
    GPIO->RoboswEphyCtrl &= ~EPHY_PWR_DOWN_DLL;
#endif

    // let UART finish printing
    udelay(100);


#if defined(CONFIG_BCM_CPLD1)
    // Determine if this was a request to enter Standby mode
    // If yes, this call won't return and a hard reset will occur later
    BcmCpld1CheckShutdownMode();
#endif

#if defined (CONFIG_BCM96368)
    {
        volatile int delay;
        volatile int i;
        local_irq_disable();
        // after we reset DRAM controller we can't access DRAM, so
        // the first iteration put things in i-cache and the scond interation do the actual reset
        for (i=0; i<2; i++) {
            DDR->DDR1_2PhaseCntl0 &= i - 1;
            DDR->DDR3_4PhaseCntl0 &= i - 1;

            if( i == 1 )
                ChipSoftReset();

            delay = 1000;
            while (delay--);
            PERF->pll_control |= SOFT_RESET*i;
            for(;i;) {} // spin mips and wait soft reset to take effect
        }
    }
#endif
#if !defined(CONFIG_BCM96328)
#if defined (CONFIG_BCM96816)
    /* Work around reset issues */
    HVG_MISC_REG_CHANNEL_A->mask |= HVG_SOFT_INIT_0;
    HVG_MISC_REG_CHANNEL_B->mask |= HVG_SOFT_INIT_0;

    {
        unsigned char portInfo6829;
        /* for BHRGR board we need to toggle GPIO30 to
           reset - on early BHR baords this is the GPHY2
           link100 so setting it does not matter */
        if ( (BP_SUCCESS == BpGet6829PortInfo(&portInfo6829)) &&
             (0 != portInfo6829))
        {
            GPIO->GPIODir |= 1<<30;
            GPIO->GPIOio  &= ~(1<<30);
        }
    }
#endif
    PERF->pll_control |= SOFT_RESET;    // soft reset mips
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    PERF->pll_control = 0;
#endif
#else
    TIMER->SoftRst = 1;
#endif
    for(;;) {} // spin mips and wait soft reset to take effect
}

unsigned long kerSysGetMacAddressType( unsigned char *ifName )
{
    unsigned long macAddressType = MAC_ADDRESS_ANY;

    if(strstr(ifName, IF_NAME_ETH))
    {
        macAddressType = MAC_ADDRESS_ETH;
    }
#if defined (AEI_VDSL_WAN_ETH)
    else if (strstr(ifName, IF_NAME_EWAN))
    {
        macAddressType = MAC_ADDRESS_ETH;
    }
#endif

    else if(strstr(ifName, IF_NAME_USB))
    {
        macAddressType = MAC_ADDRESS_USB;
    }
    else if(strstr(ifName, IF_NAME_WLAN))
    {
        macAddressType = MAC_ADDRESS_WLAN;
    }
    else if(strstr(ifName, IF_NAME_MOCA))
    {
        macAddressType = MAC_ADDRESS_MOCA;
    }
    else if(strstr(ifName, IF_NAME_ATM))
    {
        macAddressType = MAC_ADDRESS_ATM;
    }
    else if(strstr(ifName, IF_NAME_PTM))
    {
#if defined(AEI_VDSL_CUSTOMER_QWEST)
        macAddressType = 0x12ffffff;
#else
        macAddressType = MAC_ADDRESS_PTM;
#endif
    }
    else if(strstr(ifName, IF_NAME_GPON) || strstr(ifName, IF_NAME_VEIP))
    {
        macAddressType = MAC_ADDRESS_GPON;
    }
    else if(strstr(ifName, IF_NAME_EPON))
    {
        macAddressType = MAC_ADDRESS_EPON;
    }

    return macAddressType;
}

#if defined(AEI_VDSL_CUSTOMER_TELUS)
#define AEI_INVALID_ASSIGN_ID -1
#endif

int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, baseMacAddr = 0;

#if defined(AEI_VDSL_CUSTOMER_TELUS)
    UINT32 valueType = 0;
    UINT32 valueNum = 0;
    UINT32 valueId = 0;
    int  assignIndex = AEI_INVALID_ASSIGN_ID;

    valueType = (ulId & AEI_MAC_ADDRESS_TYPE_MASK);
    valueNum  = (ulId & AEI_MAC_ADDRESS_NUM_MASK);
    valueId   = (ulId & AEI_MAC_ADDRESS_ID_MASK);
#endif

    mutex_lock(&macAddrMutex);

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pMacInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

#if defined(AEI_VDSL_CUSTOMER_TELUS)
    /*
     *  1. It is used to keep using WAN MAC address is same as before while upgrade TELUS SDK3/SDK6 image to SDK12.
     *  2. SDK3-V1000H/VB784WG, atm0 = B + 1, atm1 = B +4, ptm0 = B + 1, ewan0 = B + 2.
     *  3. SDK6-V2000H, atm0 = B, atm1 = B + 4, ptm0.1 = B + 4, ewan0.1 = B.
     *  4. ptm0/ewan0 is used for using WAN interface in SDK3, but it is ptm0.1/ewan0.1 in SDK12,
     *     So make sure that ptm0/ewan0(sdk3) = ptm0.1/ewan0.1 (SDK12).
     *  5. For other TELUS project (fox example V2200H), my suggestion is that using WAN interface(atm0/ptm0.1/ewan0.1) is B + 1, and atm1 = B + 2.
     *  6. B is base MAC address.
     *  7. If value type is MAC_ADDRESS_ETH, it indicate that WAN Ethernet to request WAN MAC address. Not Lan.
     *  8. In SDK12, eth0-eth4 and ewan0 MAC are set early in ethernet driver and the ulId is low enough that
     *     valueType is 0 and should not come here. ewan0.1 and ptm0.1 are created by the brcm vlanmux module
     *     and the ulId passed had been left-shifted to high number so valueType will hit one of these cases.
     */
    switch (aeiBoardId)
    {
    case AEI_BOARD_V2000H:
        if (valueType == MAC_ADDRESS_ATM)
        {
            if (valueNum == 0) // atm0
                assignIndex = 0;
            else //atm1
                assignIndex = 4;
        }
        else if (valueType == MAC_ADDRESS_PTM)
        {
            if (valueId == 0)
                assignIndex = 0;
            else
                assignIndex = 4;
        }
        else if (valueType == MAC_ADDRESS_ETH)
        {
            assignIndex = 0;
        }
        break;
    case AEI_BOARD_V1000H:
        if (valueType == MAC_ADDRESS_ATM)
        {
            if (valueNum == 0) // atm0
                assignIndex = 1;
            else //atm1
                assignIndex = 4;
        }
        else if (valueType == MAC_ADDRESS_PTM)
        {
            assignIndex = 1;
        }
        else if (valueType == MAC_ADDRESS_ETH)
        {
            assignIndex = 2;
        }
        break;
    default:
        if (valueType == MAC_ADDRESS_ATM)
        {
            if (valueNum == 0) // atm0
                assignIndex = 1;
            else //atm1
                assignIndex = 2;
        }
        else if (valueType == MAC_ADDRESS_PTM)
        {
            assignIndex = 1;
        }
        else if (valueType == MAC_ADDRESS_ETH)
        {
            assignIndex = 1;
        }

        break;
    }

    if ((assignIndex > AEI_INVALID_ASSIGN_ID) && (assignIndex < g_pMacInfo->ulNumMacAddrs))
    {
         baseMacAddr = (baseMacAddr + assignIndex) << 8;
         memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
         memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
         g_pMacInfo->MacAddrs[assignIndex].ulId = ulId;
         g_pMacInfo->MacAddrs[assignIndex].chInUse = 1;
         mutex_unlock(&macAddrMutex);

         return nRet;
    }
#endif

#if defined(AEI_VDSL_CUSTOMER_QWEST)
    if(ulId == 0x12ffffff)
    {
            baseMacAddr = (baseMacAddr + 1) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            g_pMacInfo->MacAddrs[1].ulId = ulId;
            g_pMacInfo->MacAddrs[1].chInUse = 1;
            mutex_unlock(&macAddrMutex);
            return nRet;
  
    }
#endif

#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
    if(ulId == 0x13ffffff)
    {
            baseMacAddr = (baseMacAddr + 2) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            g_pMacInfo->MacAddrs[2].ulId = ulId;
            g_pMacInfo->MacAddrs[2].chInUse = 1;
            mutex_unlock(&macAddrMutex);
            return nRet;
  
    }
#endif


    for( i = 0, pMai = g_pMacInfo->MacAddrs; i < g_pMacInfo->ulNumMacAddrs;
        i++, pMai++ )
    {

#if defined(AEI_VDSL_CUSTOMER_TELUS)
        /*
         *  1. It is used to preserve mac address.
         *  2. i = 0 (B) could not be preserved, because it is used for LAN ethernet(eth0~eth4).
         */
        if (aeiBoardId == AEI_BOARD_V1000H)
        {
            /*
             *  1. In SDK 3(AEI_BOARD_V1000H), B + 1, B + 2, B + 4 is used for WAN MAC address, so we need to preserve it,
             *     used for static assign the mac address for WAN interface in SDK12.
             *  2. Other except 1, 2, 4 is used to auto assign mac address for other interface, for example usbX, wlX, and so on.
             */

            if ((i == 1) || (i == 2) || (i == 4))
                continue;
        }
        else if (aeiBoardId == AEI_BOARD_V2000H)
        {
            if (i == 4)
                continue;
        }
        else
        {
            if ((i == 1) || (i == 2))
                continue;
        }
#endif

#if defined(AEI_VDSL_CUSTOMER_QWEST)
		if (i == 1)  /*This mac addr is used for atm0 or ptm0*/
			continue;
#endif

#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
		if (i == 2)  /*This mac addr is used for ewan0*/
        {
			continue;
        }
#endif

        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            baseMacAddr = (baseMacAddr + i) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMai->chInUse = 1;
            pMaiFreeNoId = pMaiFreeId = NULL;
            break;
        }
        else
            if( pMai->chInUse == 0 )
            {
                if( pMai->ulId == 0 && pMaiFreeNoId == NULL )
                {
                    /* This is an available MAC address that has never been
                    * used.
                    */
                    pMaiFreeNoId = pMai;
                    ulIdxNoId = i;
#ifdef AEI_VDSL_CUSTOMER_NCS
                    continue;
#else
		    break;
#endif
                }
                else
                    if( pMai->ulId != 0 && pMaiFreeId == NULL )
                    {
                        /* This is an available MAC address that has been used
                        * before.  Use addresses that have never been used
                        * first, before using this one.
                        */
                        pMaiFreeId = pMai;
                        ulIdxId = i;
#ifdef AEI_VDSL_CUSTOMER_NCS
                        continue;
#else
			break;
#endif
                    }
            }
    }

    if( pMaiFreeNoId || pMaiFreeId )
    {
        /* An available MAC address was found. */
        memcpy(pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
#ifdef AEI_VDSL_CUSTOMER_NCS
        if( pMaiFreeId )
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
#else
        if( pMaiFreeNoId )
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pMacInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
#endif
    }
    else
        if( i == g_pMacInfo->ulNumMacAddrs )
            nRet = -EADDRNOTAVAIL;

    mutex_unlock(&macAddrMutex);

    return( nRet );
} /* kerSysGetMacAddr */

int kerSysReleaseMacAddress( unsigned char *pucaMacAddr )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = -EINVAL;
    unsigned long ulIdx = 0;
    unsigned long baseMacAddr = 0;
    unsigned long relMacAddr = 0;

    mutex_lock(&macAddrMutex);

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pMacInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    /* Get last 3 bytes of MAC address to release. */
    memcpy((unsigned char *) &relMacAddr, &pucaMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    relMacAddr >>= 8;

    ulIdx = relMacAddr - baseMacAddr;

    if( ulIdx < g_pMacInfo->ulNumMacAddrs )
    {
        PMAC_ADDR_INFO pMai = &g_pMacInfo->MacAddrs[ulIdx];
        if( pMai->chInUse == 1 )
        {
            pMai->chInUse = 0;
            nRet = 0;
        }
    }

    mutex_unlock(&macAddrMutex);

    return( nRet );
} /* kerSysReleaseMacAddr */


void kerSysGetGponSerialNumber( unsigned char *pGponSerialNumber )
{
    strcpy( pGponSerialNumber, g_pGponInfo->gponSerialNumber );
}


void kerSysGetGponPassword( unsigned char *pGponPassword )
{
    strcpy( pGponPassword, g_pGponInfo->gponPassword );
}

int kerSysGetSdramSize( void )
{
    return( (int) g_ulSdramSize );
} /* kerSysGetSdramSize */


#if defined(CONFIG_BCM96368)
/*
 * This function returns:
 * MEMC_32BIT_BUS for 32-bit SDRAM
 * MEMC_16BIT_BUS for 16-bit SDRAM
 */
unsigned int kerSysGetSdramWidth( void )
{
    return (unsigned int)(g_ulSdramWidth);
} /* kerSysGetSdramWidth */
#endif


/*Read Wlan Params data from CFE */
int kerSysGetWlanSromParams( unsigned char *wlanParams, unsigned short len)
{
    NVRAM_DATA *pNvramData;

    if (NULL == (pNvramData = readNvramData()))
    {
        printk("kerSysGetWlanSromParams: could not read nvram data\n");
        return -1;
    }

    memcpy( wlanParams,
           (char *)pNvramData + ((size_t) &((NVRAM_DATA *)0)->wlanParams),
            len );
    kfree(pNvramData);

    return 0;
}

/*Read Wlan Params data from CFE */
int kerSysGetAfeId( unsigned long *afeId )
{
    NVRAM_DATA *pNvramData;

    if (NULL == (pNvramData = readNvramData()))
    {
        printk("kerSysGetAfeId: could not read nvram data\n");
        return -1;
    }

    afeId [0] = pNvramData->afeId[0];
    afeId [1] = pNvramData->afeId[1];
    kfree(pNvramData);

    return 0;
}

void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    if (g_ledInitialized)
        boardLedCtrl(ledName, ledState);
}

/*functionto receive message from usersapce
 * Currently we dont expect any messages fromm userspace
 */
void kerSysRecvFrmMonitorTask(struct sk_buff *skb)
{

   /*process the message here*/
   printk(KERN_WARNING "unexpected skb received at %s \n",__FUNCTION__);
   kfree_skb(skb);
   return;
}

void kerSysInitMonitorSocket( void )
{
   g_monitor_nl_sk = netlink_kernel_create(&init_net, NETLINK_BRCM_MONITOR, 0, kerSysRecvFrmMonitorTask, NULL, THIS_MODULE);

   if(!g_monitor_nl_sk)
   {
      printk(KERN_ERR "Failed to create a netlink socket for monitor\n");
      return;
   }

}


void kerSysSendtoMonitorTask(int msgType, char *msgData, int msgDataLen)
{

   struct sk_buff *skb =  NULL;
   struct nlmsghdr *nl_msgHdr = NULL;
   unsigned int payloadLen =sizeof(struct nlmsghdr);

   if(!g_monitor_nl_pid)
   {
      printk(KERN_INFO "message received before monitor task is initialized %s \n",__FUNCTION__);
      return;
   } 

   if(msgData && (msgDataLen > MAX_PAYLOAD_LEN))
   {
      printk(KERN_ERR "invalid message len in %s",__FUNCTION__);
      return;
   } 

   payloadLen += msgDataLen;
   payloadLen = NLMSG_SPACE(payloadLen);

   /*Alloc skb ,this check helps to call the fucntion from interrupt context */

   if(in_atomic())
   {
      skb = alloc_skb(payloadLen, GFP_ATOMIC);
   }
   else
   {
      skb = alloc_skb(payloadLen, GFP_KERNEL);
   }

   if(!skb)
   {
      printk(KERN_ERR "failed to alloc skb in %s",__FUNCTION__);
      return;
   }

   nl_msgHdr = (struct nlmsghdr *)skb->data;
   nl_msgHdr->nlmsg_type = msgType;
   nl_msgHdr->nlmsg_pid=0;/*from kernel */
   nl_msgHdr->nlmsg_len = payloadLen;
   nl_msgHdr->nlmsg_flags =0;

   if(msgData)
   {
      memcpy(NLMSG_DATA(nl_msgHdr),msgData,msgDataLen);
   }      

   NETLINK_CB(skb).pid = 0; /*from kernel */

   skb->len = payloadLen; 

   netlink_unicast(g_monitor_nl_sk, skb, g_monitor_nl_pid, MSG_DONTWAIT);
   return;
}

void kerSysCleanupMonitorSocket(void)
{
   g_monitor_nl_pid = 0 ;
   sock_release(g_monitor_nl_sk->sk_socket);
}

// Must be called with flashImageMutex held
static PFILE_TAG getTagFromPartition(int imageNumber)
{
#if defined(AEI_CONFIG_JFFS)
    #define tag_not_searched    0
    #define tag_not_found       1
    #define tag_found           2

    #define UNINITIALIZED_FLASH_DATA_CHAR  0xff

    static FILE_TAG Tag1 = {{tag_not_searched}};
    static FILE_TAG Tag2 = {{tag_not_searched}};
    PFILE_TAG pTag = (imageNumber == 2) ? &Tag2 : &Tag1;
    PFILE_TAG ret = NULL;
	struct mtd_info *mtd=NULL;
    int retlen = 0;
    switch( pTag->tagVersion[0] )
    {
    	case tag_not_searched:
			{
			    unsigned char *inMemNvramData_buf;
			    NVRAM_DATA inMemNvramData;
				unsigned long bootCfgSave =  NAND->NandNandBootConfig;
#if 1				
			    unsigned long rootfs_ofs;
#else
                char *pStr=NULL;
				char BootImage='0';
#endif
			    inMemNvramData_buf = (unsigned char *) &inMemNvramData;
			    memset(inMemNvramData_buf, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);

			    NAND->NandNandBootConfig = NBC_AUTO_DEV_ID_CFG | 0x101;
			    NAND->NandCsNandXor = 1;	
			    memcpy(inMemNvramData_buf, (unsigned char *)
			        FLASH_BASE + NVRAM_DATA_OFFSET, sizeof(NVRAM_DATA));
			    NAND->NandNandBootConfig = bootCfgSave;
			    NAND->NandCsNandXor = 0;	
				

			    memset(pTag, 0, sizeof(FILE_TAG));
#if 1
			    kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);
			    if(rootfs_ofs == inMemNvramData.ulNandPartOfsKb[NP_ROOTFS_1])
#else
                pStr=strstr(&(inMemNvramData.szBootline),"p=");
                if(pStr)
                    BootImage=pStr[2];
                if(BootImage == BOOT_LATEST_IMAGE)
#endif
                {
                    if(imageNumber==1)
                        mtd=get_mtd_device_nm("tag");
                    else
                        mtd=get_mtd_device_nm("tag_update");

                }
                else
                {
                    if(imageNumber==1)
                        mtd=get_mtd_device_nm("tag_update");
                    else
                        mtd=get_mtd_device_nm("tag");

                }

                if(mtd)
                {
                    mtd->read(mtd, 0, sizeof(FILE_TAG), &retlen, pTag);

                    if(strlen(pTag->signiture_1)>0 && strlen(pTag->signiture_1)<SIG_LEN)
                    {
                        pTag->tagVersion[0] = tag_found;
                        ret = pTag;
                    }
                    else
                        pTag->tagVersion[0] = tag_not_found;

                    put_mtd_device(mtd);

                }
                else
                    pTag->tagVersion[0] = tag_not_found;
			
				if(pTag->tagVersion[0] == tag_found)
				{
					char * pVer = NULL;
#ifdef AEI_VER2_DUAL_IMAGE
//because imageSequence number don't input from imagefile , 
//we always write sequence number 0 in tag, 
//so we need  read sequence  number from cferam.
					int seq=-1;
					char fname[] = NAND_CFE_RAM_NAME;
					int fname_actual_len = strlen(fname);
					int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
					char cferam_base[32], cferam_buf[32], cferam_fmt[32]; 
					int i;
					struct file *fp;
					
					strcpy(cferam_base, fname);
					cferam_base[fname_cmp_len] = '\0';
					strcpy(cferam_fmt, cferam_base);
					strcat(cferam_fmt, "%3.3d");
								
					if((rootfs_ofs == inMemNvramData.ulNandPartOfsKb[NP_ROOTFS_1] && imageNumber == 1)
						|| (rootfs_ofs == inMemNvramData.ulNandPartOfsKb[NP_ROOTFS_2] && imageNumber == 2))
					{
						/* Find the sequence number of the partion that is booted from. */
						for( i = 0; i < 999; i++ )
						{
							sprintf(cferam_buf, cferam_fmt, i);
							fp = filp_open(cferam_buf, O_RDONLY, 0);
							if (!IS_ERR(fp) )
							{
								filp_close(fp, NULL);

								/* Seqence number found. */
								seq = i;
								break;
							}
						}		 
					}
					else
					{
						/* Find the sequence number of the partion that is not booted from. */
						if( do_mount("mtd:rootfs_update", "/mnt", "jffs2", MS_RDONLY, NULL) == 0 )
						{
							strcpy(cferam_fmt, "/mnt/");
							strcat(cferam_fmt, cferam_base);
							strcat(cferam_fmt, "%3.3d");

							for( i = 0; i < 999; i++ )
							{
								sprintf(cferam_buf, cferam_fmt, i);
								fp = filp_open(cferam_buf, O_RDONLY, 0);
								if (!IS_ERR(fp) )
								{
									filp_close(fp, NULL);
									/*Seq number found. */
										seq = i;
									break;
								}
							}
						}
					}					 

					snprintf(pTag->imageSequence,FLAG_LEN * 2, "%d",seq);
#endif
					pVer = strstr(pTag->signiture_2, ".sip");
					if(pVer != NULL)
						*pVer = '\0';
					pVer = strstr(pTag->signiture_2, ".SIP");
					if(pVer != NULL)
						*pVer = '\0';
					pVer = strstr(pTag->imageVersion, ".sip");
					if(pVer != NULL)
						*pVer = '\0';
					pVer = strstr(pTag->imageVersion, ".SIP");
					if(pVer != NULL)
						*pVer = '\0';

					if(imageNumber==1)
						printk("###Tag1 signature(%s),version(%s),version2(%s),imageSequence(%s)\n",pTag->signiture_1,pTag->signiture_2,pTag->imageVersion,pTag->imageSequence);
					else
						printk("###Tag2 signature(%s),version(%s),version2(%s),imageSequence(%s)\n",pTag->signiture_1,pTag->signiture_2,pTag->imageVersion,pTag->imageSequence);
					
				}
	    	}
			break;
    	case tag_found:
        	ret = pTag;
        	break;

    	case tag_not_found:
        	ret = NULL;
        	break;
    }

    return(ret);	
#else
    static unsigned char sectAddr1[sizeof(FILE_TAG) + sizeof(int)];
    static unsigned char sectAddr2[sizeof(FILE_TAG) + sizeof(int)];
    int blk = 0;
    UINT32 crc;
    PFILE_TAG pTag = NULL;
    unsigned char *pBase = flash_get_memptr(0);
    unsigned char *pSectAddr = NULL;

    /* The image tag for the first image is always after the boot loader.
    * The image tag for the second image, if it exists, is at one half
    * of the flash size.
    */
    if( imageNumber == 1 )
    {
        FLASH_ADDR_INFO flash_info;

        kerSysFlashAddrInfoGet(&flash_info);
        blk = flash_get_blk((int) (pBase+flash_info.flash_rootfs_start_offset));
        pSectAddr = sectAddr1;
    }
    else
        if( imageNumber == 2 )
        {
            blk = flash_get_blk((int) (pBase + (flash_get_total_size() / 2)));
            pSectAddr = sectAddr2;
        }

        if( blk )
        {
            int *pn;

            memset(pSectAddr, 0x00, sizeof(FILE_TAG));
            flash_read_buf((unsigned short) blk, 0, pSectAddr, sizeof(FILE_TAG));
            crc = CRC32_INIT_VALUE;
            crc = getCrc32(pSectAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);
            pTag = (PFILE_TAG) pSectAddr;
            pn = (int *) (pTag + 1);
            *pn = blk;
            if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
                pTag = NULL;
        }

        return( pTag );
#endif
}

// must be called with flashImageMutex held
static int getPartitionFromTag( PFILE_TAG pTag )
{
    int ret = 0;

    if( pTag )
    {
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);
        int sequence = simple_strtoul(pTag->imageSequence,  NULL, 10);
        int sequence1 = (pTag1) ? simple_strtoul(pTag1->imageSequence, NULL, 10)
            : -1;
        int sequence2 = (pTag2) ? simple_strtoul(pTag2->imageSequence, NULL, 10)
            : -1;

        if( pTag1 && sequence == sequence1 )
            ret = 1;
        else
            if( pTag2 && sequence == sequence2 )
                ret = 2;
    }

    return( ret );
}


// must be called with flashImageMutex held
static PFILE_TAG getBootImageTag(void)
{
    static int displayFsAddr = 1;
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);

    if( pTag1 && pTag2 )
    {
        /* Two images are flashed. */
        int sequence1 = simple_strtoul(pTag1->imageSequence, NULL, 10);
        int sequence2 = simple_strtoul(pTag2->imageSequence, NULL, 10);
        int imgid = 0;

        kerSysBlParmsGetInt(BOOTED_IMAGE_ID_NAME, &imgid);
#if defined(AEI_VDSL_DUAL_IMAGE)
        if(sequence1 != IMAGE1_SEQUENCE || sequence2 != IMAGE2_SEQUENCE )
        {  
            if( imgid == BOOT_LATEST_IMAGE )
            {
                pTag = pTag1;
            }
            else /* Boot from the second image. */
            {
                pTag = pTag2;
            }
        }
        else
#endif
        if( imgid == BOOTED_OLD_IMAGE )
            pTag = (sequence2 < sequence1) ? pTag2 : pTag1;
        else
            pTag = (sequence2 > sequence1) ? pTag2 : pTag1;
    }
    else
        /* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;

    if( pTag && displayFsAddr )
    {
        displayFsAddr = 0;
        printk("File system address: 0x%8.8lx\n",
            simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET);
    }

    return( pTag );
}

// Must be called with flashImageMutex held
static void UpdateImageSequenceNumber( unsigned char *imageSequence )
{
    int newImageSequence = 0;
    PFILE_TAG pTag = getTagFromPartition(1);

    if( pTag )
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    pTag = getTagFromPartition(2);
    if(pTag && simple_strtoul(pTag->imageSequence, NULL, 10) > newImageSequence)
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    newImageSequence++;
    sprintf(imageSequence, "%d", newImageSequence);
}

/* Must be called with flashImageMutex held */
static int flashFsKernelImage( unsigned char *imagePtr, int imageLen,
    int flashPartition, int *numPartitions )
{
    int status = 0;
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int rootfsAddr = simple_strtoul(pTag->rootfsAddress, NULL, 10);
    int kernelAddr = simple_strtoul(pTag->kernelAddress, NULL, 10);
#if defined(AEI_VDSL_CUSTOMER_NCS) && !defined(AEI_VDSL_DUAL_IMAGE)   
    char *p;
#endif
    char *tagFs = imagePtr;
    unsigned int baseAddr = (unsigned int) flash_get_memptr(0);
    unsigned int totalSize = (unsigned int) flash_get_total_size();
    unsigned int reservedBytesAtEnd;
    unsigned int availableSizeOneImg;
    unsigned int reserveForTwoImages;
    unsigned int availableSizeTwoImgs;
    unsigned int newImgSize = simple_strtoul(pTag->rootfsLen, NULL, 10) +
        simple_strtoul(pTag->kernelLen, NULL, 10);
    PFILE_TAG pCurTag = getBootImageTag();
    int nCurPartition = getPartitionFromTag( pCurTag );
    int should_yield =
        (flashPartition == 0 || flashPartition == nCurPartition) ? 0 : 1;
    UINT32 crc;
    unsigned int curImgSize = 0;
    unsigned int rootfsOffset = (unsigned int) rootfsAddr - IMAGE_BASE - TAG_LEN;
    FLASH_ADDR_INFO flash_info;
    NVRAM_DATA *pNvramData;
#if defined(AEI_VDSL_DUAL_IMAGE)
    int newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);
#endif

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);

    if (NULL == (pNvramData = readNvramData()))
    {
        return -ENOMEM;
    }

    kerSysFlashAddrInfoGet(&flash_info);
    if( rootfsOffset < flash_info.flash_rootfs_start_offset )
    {
        // Increase rootfs and kernel addresses by the difference between
        // rootfs offset and what it needs to be.
        rootfsAddr += flash_info.flash_rootfs_start_offset - rootfsOffset;
        kernelAddr += flash_info.flash_rootfs_start_offset - rootfsOffset;
        sprintf(pTag->rootfsAddress,"%lu", (unsigned long) rootfsAddr);
        sprintf(pTag->kernelAddress,"%lu", (unsigned long) kernelAddr);
        crc = CRC32_INIT_VALUE;
        crc = getCrc32((unsigned char *)pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);
        *(unsigned long *) &pTag->tagValidationToken[0] = crc;
    }

    rootfsAddr += BOOT_OFFSET;
    kernelAddr += BOOT_OFFSET;

    reservedBytesAtEnd = flash_get_reserved_bytes_at_end(&flash_info);
    availableSizeOneImg = totalSize - ((unsigned int) rootfsAddr - baseAddr) -
        reservedBytesAtEnd;
    reserveForTwoImages =
        (flash_info.flash_rootfs_start_offset > reservedBytesAtEnd)
        ? flash_info.flash_rootfs_start_offset : reservedBytesAtEnd;
    availableSizeTwoImgs = (totalSize / 2) - reserveForTwoImages;

    //    printk("availableSizeOneImage=%dKB availableSizeTwoImgs=%dKB reserve=%dKB\n",
    //            availableSizeOneImg/1024, availableSizeTwoImgs/1024, reserveForTwoImages/1024);
    if( pCurTag )
    {
        curImgSize = simple_strtoul(pCurTag->rootfsLen, NULL, 10) +
            simple_strtoul(pCurTag->kernelLen, NULL, 10);
    }

    if( newImgSize > availableSizeOneImg)
    {
        printk("Illegal image size %d.  Image size must not be greater "
            "than %d.\n", newImgSize, availableSizeOneImg);
        kfree(pNvramData);
        return -1;
    }

    *numPartitions = (curImgSize <= availableSizeTwoImgs &&
         newImgSize <= availableSizeTwoImgs &&
         flashPartition != nCurPartition) ? 2 : 1;

    // If the current image fits in half the flash space and the new
    // image to flash also fits in half the flash space, then flash it
    // in the partition that is not currently being used to boot from.
    if( curImgSize <= availableSizeTwoImgs &&
        newImgSize <= availableSizeTwoImgs &&
#ifdef AEI_VDSL_DUAL_IMAGE
         newImageSequence== IMAGE2_SEQUENCE )
#else
        ((nCurPartition == 1 && flashPartition != 1) || flashPartition == 2) )
#endif
    {
        // Update rootfsAddr to point to the second boot partition.
        int offset = (totalSize / 2) + TAG_LEN;

        sprintf(((PFILE_TAG) tagFs)->kernelAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset + (kernelAddr - rootfsAddr));
        kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr);

        sprintf(((PFILE_TAG) tagFs)->rootfsAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset);
        rootfsAddr = baseAddr + offset;
    }
#ifdef AEI_VDSL_DUAL_IMAGE
    memset(((PFILE_TAG) tagFs)->imageSequence,0,sizeof(((PFILE_TAG) tagFs)->imageSequence));
    if(newImageSequence== IMAGE2_SEQUENCE )
    {
        sprintf(((PFILE_TAG) tagFs)->imageSequence,"%d",IMAGE2_SEQUENCE);
    }
    else
        sprintf(((PFILE_TAG) tagFs)->imageSequence,"%d",IMAGE1_SEQUENCE);
#else
    UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
#endif
    crc = CRC32_INIT_VALUE;
    crc = getCrc32((unsigned char *)tagFs, (UINT32)TAG_LEN-TOKEN_LEN, crc);
    *(unsigned long *) &((PFILE_TAG) tagFs)->tagValidationToken[0] = crc;

    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs,
        TAG_LEN + newImgSize, should_yield)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        kfree(pNvramData);
        return status;
    }

#if defined(AEI_VDSL_CUSTOMER_NCS) && !defined(AEI_VDSL_DUAL_IMAGE)   
    for( p = pNvramData->szBootline; p[2] != '\0'; p++ )
    {
        if( p[0] == 'p' && p[1] == '=' && p[2] != BOOT_LATEST_IMAGE )
        {
            // Change boot partition to boot from new image.
            p[2] = BOOT_LATEST_IMAGE;
//            writeNvramData(pNvramData);
            writeNvramDataCrcLocked(pNvramData);
            break;
        }
    }
#endif
    kfree(pNvramData);
#if defined(AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD)

    if(status==0)
    {
         NVRAM_DATA *nvramData = NULL;
         if (NULL != (nvramData = readNvramData()))
         {
            // memset(&nvramData.ugstatus[0],0,3);
            //  strncpy(&nvramData.ugstatus[0],ugstatus,3-1);
            if(nvramData->ugstatus[1] != '0')
                 nvramData->ugstatus[1] = '2';
  //          writeNvramData(nvramData);
              writeNvramDataCrcLocked(nvramData);
            kfree(nvramData);
        }
    }
#else
             char ug_info[2]={0};
             kerSysScratchPadGet("UpGrade_Info",ug_info,sizeof(ug_info));
             if(ug_info[0] != '0')
	     {
                ug_info[0] = '2';
                kerSysScratchPadSet("UpGrade_Info",ug_info,sizeof(ug_info));
          }
#endif // AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD
    return(status);
}

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
static int getFlashUsedSize()
{
//    unsigned int totalSize = (unsigned int) flash_get_total_size();
    unsigned int imgblocksize = 0,reservedBytesAtEnd=0;
    PFILE_TAG pTag1,pTag2;
    FLASH_ADDR_INFO flash_info;
    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);
    pTag1 = getTagFromPartition(1);
    pTag2 = getTagFromPartition(2);
    kerSysFlashAddrInfoGet(&flash_info);
    reservedBytesAtEnd = flash_get_reserved_bytes_at_end(&flash_info);
    if(pTag1)
    {
        if(simple_strtoul(pTag1->totalImageLen, NULL, 10)%1024==0)
            imgblocksize += simple_strtoul(pTag1->totalImageLen, NULL, 10)/1024;
        else
            imgblocksize += (simple_strtoul(pTag1->totalImageLen, NULL, 10)/1024+1);
    }
    if(pTag2)
    {
        if(simple_strtoul(pTag2->totalImageLen, NULL, 10)%1024==0)
            imgblocksize += simple_strtoul(pTag2->totalImageLen, NULL, 10)/1024;
        else
            imgblocksize += (simple_strtoul(pTag2->totalImageLen, NULL, 10)/1024+1);
    }
    return reservedBytesAtEnd/1024 + imgblocksize;
}
#endif

static int getImageVersion( int imageNumber, char *verStr, int verStrSize)
{
    int ret = 0; /* zero bytes copied to verStr so far */

    PFILE_TAG pTag = NULL;

    if( imageNumber == 1 )
        pTag = getTagFromPartition(1);
    else
        if( imageNumber == 2 )
            pTag = getTagFromPartition(2);

    if( pTag )
    {
#ifdef AEI_VDSL_CUSTOMER_NCS
        if( verStrSize > sizeof(pTag->signiture_2) )
            ret = sizeof(pTag->signiture_2);
        else
            ret = verStrSize;

        memcpy(verStr, pTag->signiture_2, ret);
#else
        if( verStrSize > sizeof(pTag->imageVersion) )
            ret = sizeof(pTag->imageVersion);
        else
            ret = verStrSize;

        memcpy(verStr, pTag->imageVersion, ret);
#endif
    }

    return( ret );
}

PFILE_TAG kerSysUpdateTagSequenceNumber(int imageNumber)
{
    PFILE_TAG pTag = NULL;
    UINT32 crc;

    switch( imageNumber )
    {
    case 0:
        pTag = getBootImageTag();
        break;

    case 1:
        pTag = getTagFromPartition(1);
        break;

    case 2:
        pTag = getTagFromPartition(2);
        break;

    default:
        break;
    }

    if( pTag )
    {
        UpdateImageSequenceNumber( pTag->imageSequence );
        crc = CRC32_INIT_VALUE;
        crc = getCrc32((unsigned char *)pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);
        *(unsigned long *) &pTag->tagValidationToken[0] = crc;
    }

    return(pTag);
}

int kerSysGetSequenceNumber(int imageNumber)
{
    PFILE_TAG pTag = NULL;
    int seqNumber = -1;

    switch( imageNumber )
    {
    case 0:
        pTag = getBootImageTag();
        break;

    case 1:
        pTag = getTagFromPartition(1);
        break;

    case 2:
        pTag = getTagFromPartition(2);
        break;

    default:
        break;
    }

    if( pTag )
        seqNumber= simple_strtoul(pTag->imageSequence, NULL, 10);

    return(seqNumber);
}

static int getBootedValue(int getBootedPartition)
{
    int ret = -1;
    int imgId = -1;

    kerSysBlParmsGetInt(BOOTED_IMAGE_ID_NAME, &imgId);

    /* The boot loader parameter will only be "new image", "old image" or "only
     * image" in order to be compatible with non-OMCI image update. If the
     * booted partition is requested, convert this boot type to partition type.
     */
    if( imgId != -1 )
    {
        if( getBootedPartition )
        {
            /* Get booted partition. */
            int seq1 = kerSysGetSequenceNumber(1);
            int seq2 = kerSysGetSequenceNumber(2);

            switch( imgId )
            {
            case BOOTED_NEW_IMAGE:
                if( seq1 == -1 || seq2 > seq1 )
                    ret = BOOTED_PART2_IMAGE;
                else
                    if( seq2 == -1 || seq1 >= seq2 )
                        ret = BOOTED_PART1_IMAGE;
                break;

            case BOOTED_OLD_IMAGE:
                if( seq1 == -1 || seq2 < seq1 )
                    ret = BOOTED_PART2_IMAGE;
                else
                    if( seq2 == -1 || seq1 <= seq2 )
                        ret = BOOTED_PART1_IMAGE;
                break;

            case BOOTED_ONLY_IMAGE:
                ret = (seq1 == -1) ? BOOTED_PART2_IMAGE : BOOTED_PART1_IMAGE;
                break;

            default:
                break;
            }
        }
        else
            ret = imgId;
    }

    return( ret );
}


#if !defined(CONFIG_BRCM_IKOS)
PFILE_TAG kerSysImageTagGet(void)
{
    PFILE_TAG tag;

    mutex_lock(&flashImageMutex);
    tag = getBootImageTag();
    mutex_unlock(&flashImageMutex);

    return tag;
}
#else
PFILE_TAG kerSysImageTagGet(void)
{
    return( (PFILE_TAG) (FLASH_BASE + FLASH_LENGTH_BOOT_ROM));
}
#endif

/*
 * Common function used by BCM_IMAGE_CFE and BCM_IMAGE_WHOLE ioctls.
 * This function will acquire the flashImageMutex
 *
 * @return 0 on success, -1 on failure.
 */
static int commonImageWrite(int flash_start_addr, char *string, int size)
{
    NVRAM_DATA * pNvramDataOrig;
    NVRAM_DATA * pNvramDataNew=NULL;
    int ret;

    mutex_lock(&flashImageMutex);

    // Get a copy of the nvram before we do the image write operation
    if (NULL != (pNvramDataOrig = readNvramData()))
    {

        ret = kerSysBcmImageSet(flash_start_addr, string, size, 0);

        /*
         * After the image is written, check the nvram.
         * If nvram is bad, write back the original nvram.
         */
        pNvramDataNew = readNvramData();
        if ((0 != ret) ||
            (NULL == pNvramDataNew) ||
            (BpSetBoardId(pNvramDataNew->szBoardId) != BP_SUCCESS)
#if defined (CONFIG_BCM_ENDPOINT_MODULE)
            || (BpSetVoiceBoardId(pNvramDataNew->szVoiceBoardId) != BP_SUCCESS)
#endif
            )
        {
            // we expect this path to be taken.  When a CFE or whole image
            // is written, it typically does not have a valid nvram block
            // in the image.  We detect that condition here and restore
            // the previous nvram settings.  Don't print out warning here.
            writeNvramDataCrcLocked(pNvramDataOrig);

            // don't modify ret, it is return value from kerSysBcmImageSet
        }
#if defined(AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD)
		if(ret==0)
		{
			 NVRAM_DATA *nvramData = NULL;
			 if (NULL != (nvramData = readNvramData()))
			 {
				if(nvramData->ugstatus[1] != '0')
					 nvramData->ugstatus[1] = '2';
				  writeNvramDataCrcLocked(nvramData);
				kfree(nvramData);
			}
			 
#if defined(AEI_VER2_DUAL_IMAGE)
//boot from latest partition after flash image.
			 if((nvramData = kmalloc(sizeof(NVRAM_DATA), GFP_KERNEL))!= NULL)
			 {
				 int bskip = true;
				 char *p = NULL;
 
				 memset(nvramData, 0, sizeof(NVRAM_DATA));
 
				 if (AEI_readNvramData(nvramData) == 0)
				 {
					 for( p = nvramData->szBootline; p[2] != '\0'; p++ )
					 {
						 if( p[0] == 'p' && p[1] == '=' )
						 {
							 p[2]=BOOT_LATEST_IMAGE;
							 bskip = false;
							 break;
						 }
					 }
 
					 writeNvramDataCrcLocked(nvramData);
					 kfree(nvramData);
					 nvramData=NULL;
					 if (!bskip)
					 {
						 kerSysSetBootImageState('0');
					 }
				 }
				 if (nvramData)
					 kfree(nvramData);
			 }
#endif

		}
#endif // AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD

    }
    else
    {
        ret = -1;
    }

    mutex_unlock(&flashImageMutex);

    if (pNvramDataOrig)
        kfree(pNvramDataOrig);
    if (pNvramDataNew)
        kfree(pNvramDataNew);

    return ret;
}

#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
/*
 * Common function used by BCM_IMAGE_CFE and BCM_IMAGE_WHOLE ioctls.
 * This function will acquire the flashImageMutex
 *
 * @return 0 on success, -1 on failure.
 */
static int AEI_commonImageWrite(int flash_start_addr, char *string, int size, int partition)
{
    NVRAM_DATA * pNvramDataOrig;
    NVRAM_DATA * pNvramDataNew=NULL;
    int ret = 1 ;

    mutex_lock(&flashImageMutex);

    // Get a copy of the nvram before we do the image write operation
    if (NULL != (pNvramDataOrig = readNvramData()))
    {
		ret = AEI_kerSysBcmImageSet(partition, string, size);
        /*
         * After the image is written, check the nvram.
         * If nvram is bad, write back the original nvram.
         */
        pNvramDataNew = readNvramData();
        if ((0 != ret) ||
            (NULL == pNvramDataNew) ||
            (BpSetBoardId(pNvramDataNew->szBoardId) != BP_SUCCESS)
#if defined (CONFIG_BCM_ENDPOINT_MODULE)
            || (BpSetVoiceBoardId(pNvramDataNew->szVoiceBoardId) != BP_SUCCESS)
#endif
            )
        {
            // we expect this path to be taken.  When a CFE or whole image
            // is written, it typically does not have a valid nvram block
            // in the image.  We detect that condition here and restore
            // the previous nvram settings.  Don't print out warning here.
            writeNvramDataCrcLocked(pNvramDataOrig);

            // don't modify ret, it is return value from kerSysBcmImageSet
        }

		if(ret==0)
		{
            NVRAM_DATA *nvramData = NULL;
			 if((nvramData = kmalloc(sizeof(NVRAM_DATA), GFP_KERNEL))!= NULL)
			 {
				 int bskip = true;
				 char *p = NULL;
 
				 memset(nvramData, 0, sizeof(NVRAM_DATA));
 
				 if (AEI_readNvramData(nvramData) == 0)
				 {
					 for( p = nvramData->szBootline; p[2] != '\0'; p++ )
					 {
						 if( p[0] == 'p' && p[1] == '=' )
						 {
							 p[2]=BOOT_LATEST_IMAGE;
							 bskip = false;
							 break;
						 }
					 }
 
					 writeNvramDataCrcLocked(nvramData);
					 kfree(nvramData);
					 nvramData=NULL;
					 if (!bskip)
					 {
						 kerSysSetBootImageState('0');
					 }
				 }
				 if (nvramData)
				     kfree(nvramData);
			 }

		}
    }
    else
    {
        ret = -1;
    }

    mutex_unlock(&flashImageMutex);

    if (pNvramDataOrig)
        kfree(pNvramDataOrig);
    if (pNvramDataNew)
        kfree(pNvramDataNew);

    return ret;
}
#endif

struct file_operations monitor_fops;

//********************************************************************************************
// misc. ioctl calls come to here. (flash, led, reset, kernel memory access, etc.)
//********************************************************************************************
static int board_ioctl( struct inode *inode, struct file *flip,
                       unsigned int command, unsigned long arg )
{
    int ret = 0;
    BOARD_IOCTL_PARMS ctrlParms;
    unsigned char ucaMacAddr[NVRAM_MAC_ADDRESS_LEN];

    switch (command) {
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
    case BOARD_IOCTL_PARTITION_WRITE:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
		{
            if(ctrlParms.action)
			{
				if(ctrlParms.strLen <= 0)
				{
					printk("Illegal flash image size [%d].\n", ctrlParms.strLen);
					ret = -1;
					break;
				}

				if (ctrlParms.offset == 0)
				{
					ctrlParms.offset = FLASH_BASE;
				}

				ret = AEI_commonImageWrite(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen, ctrlParms.action);

				if (ret == 0)
				{
					//resetPwrmgmtDdrMips();
				}
				else
				{
					printk("flash of whole image failed, ret=%d\n", ret);
				}
			}
			else
			{
                ret = -EINVAL;
                printk("flash_ioctl_command: invalid command %d\n", ctrlParms.action);		
			}
			ctrlParms.result = ret;
			__copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
        else
			ret = -EFAULT;
        break;
#endif

    case BOARD_IOCTL_FLASH_WRITE:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {

            switch (ctrlParms.action) {
            case SCRATCH_PAD:
                if (ctrlParms.offset == -1)
                    ret =  kerSysScratchPadClearAll();
                else
                    ret = kerSysScratchPadSet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                break;

            case PERSISTENT:
                ret = kerSysPersistentSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                break;

            case BACKUP_PSI:
                ret = kerSysBackupPsiSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                break;

            case SYSLOG:
                ret = kerSysSyslogSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                break;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
            case SYSLOGONREBOOT:
                ret = AEI_SaveSyslogOnReboot();
                break;
#endif
            case NVRAM:
            {
                NVRAM_DATA * pNvramData;

                /*
                 * Note: even though NVRAM access is protected by
                 * flashImageMutex at the kernel level, this protection will
                 * not work if two userspaces processes use ioctls to get
                 * NVRAM data, modify it, and then use this ioctl to write
                 * NVRAM data.  This seems like an unlikely scenario.
                 */
                mutex_lock(&flashImageMutex);
                if (NULL == (pNvramData = readNvramData()))
                {
                    mutex_unlock(&flashImageMutex);
                    return -ENOMEM;
                }
                if ( !strncmp(ctrlParms.string, "WLANDATA", 8 ) ) { //Wlan Data data
                    memset((char *)pNvramData + ((size_t) &((NVRAM_DATA *)0)->wlanParams),
                        0, sizeof(pNvramData->wlanParams) );
                    memcpy( (char *)pNvramData + ((size_t) &((NVRAM_DATA *)0)->wlanParams),
                        ctrlParms.string+8,
                        ctrlParms.strLen-8);
                    writeNvramDataCrcLocked(pNvramData);
                }
#if defined (AEI_VDSL_CUSTOMER_NCS) 
                //SUPPORT_DSL_BONDING macro not carried here so leave out since non-bonding will not call this anyways
                else if (ctrlParms.string && !strncmp(ctrlParms.string, "DSLDATAPUMP", 11)) {
                    if (strlen(ctrlParms.string) > 11)
                        pNvramData->dslDatapump = (unsigned long) simple_strtol(ctrlParms.string+11, NULL, 10);
                    else
                        pNvramData->dslDatapump = 0;

                    writeNvramDataCrcLocked(pNvramData);
                }
#endif
                else {
                    // assumes the user has calculated the crc in the nvram struct
                    ret = kerSysNvRamSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                }
                mutex_unlock(&flashImageMutex);
                kfree(pNvramData);
                break;
            }

            case BCM_IMAGE_CFE:
                if( ctrlParms.strLen <= 0 || ctrlParms.strLen > FLASH_LENGTH_BOOT_ROM )
                {
                    printk("Illegal CFE size [%d]. Size allowed: [%d]\n",
                        ctrlParms.strLen, FLASH_LENGTH_BOOT_ROM);
                    ret = -1;
                    break;
                }

                ret = commonImageWrite(ctrlParms.offset + BOOT_OFFSET, ctrlParms.string, ctrlParms.strLen);

                break;

            case BCM_IMAGE_FS:
                {
                int numPartitions = 1;
                int noReboot = FLASH_IS_NO_REBOOT(ctrlParms.offset);
                int partition = FLASH_GET_PARTITION(ctrlParms.offset);

                mutex_lock(&flashImageMutex);
                ret = flashFsKernelImage(ctrlParms.string, ctrlParms.strLen,
                    partition, &numPartitions);
                mutex_unlock(&flashImageMutex);
#if defined(AEI_VDSL_DOWNGRADE_NVRAM_ADJUST)
                if(ret == 0 && (numPartitions == 1 || noReboot == 0))
                {
                    if(AEI_IsDownGrade_FromSDK12To6(ctrlParms.string))
                       AEI_DownGrade_AdjustNVRAM();
                    resetPwrmgmtDdrMips();
                }
#else
                if(ret == 0 && (numPartitions == 1 || noReboot == 0))
                    resetPwrmgmtDdrMips();
#endif
                }

                break;

            case BCM_IMAGE_KERNEL:  // not used for now.
                break;

            case BCM_IMAGE_WHOLE:
                if(ctrlParms.strLen <= 0)
                {
                    printk("Illegal flash image size [%d].\n", ctrlParms.strLen);
                    ret = -1;
                    break;
                }

                if (ctrlParms.offset == 0)
                {
                    ctrlParms.offset = FLASH_BASE;
                }

                ret = commonImageWrite(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);

                if (ret == 0)
                {
#ifdef AEI_CONFIG_JFFS
		    sys_sync();
#endif
                    resetPwrmgmtDdrMips();
                }
                else
                {
                    printk("flash of whole image failed, ret=%d\n", ret);
                }
                break;

            default:
                ret = -EINVAL;
                printk("flash_ioctl_command: invalid command %d\n", ctrlParms.action);
                break;
            }
            ctrlParms.result = ret;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_FLASH_READ:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            switch (ctrlParms.action) {
            case SCRATCH_PAD:
                ret = kerSysScratchPadGet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                break;

            case PERSISTENT:
                ret = kerSysPersistentGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                break;

            case BACKUP_PSI:
                ret = kerSysBackupPsiGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                break;

            case SYSLOG:
                ret = kerSysSyslogGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                break;

            case NVRAM:
                kerSysNvRamGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                ret = 0;
                break;

            case FLASH_SIZE:
                ret = kerSysFlashSizeGet();
                break;

            default:
                ret = -EINVAL;
                printk("Not supported.  invalid command %d\n", ctrlParms.action);
                break;
            }
            ctrlParms.result = ret;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_FLASH_LIST:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            switch (ctrlParms.action) {
            case SCRATCH_PAD:
                ret = kerSysScratchPadList(ctrlParms.buf, ctrlParms.offset);
                break;

            default:
                ret = -EINVAL;
                printk("Not supported.  invalid command %d\n", ctrlParms.action);
                break;
            }
            ctrlParms.result = ret;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_DUMP_ADDR:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            dumpaddr( (unsigned char *) ctrlParms.string, ctrlParms.strLen );
            ctrlParms.result = 0;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_SET_MEMORY:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            unsigned long  *pul = (unsigned long *)  ctrlParms.string;
            unsigned short *pus = (unsigned short *) ctrlParms.string;
            unsigned char  *puc = (unsigned char *)  ctrlParms.string;
            switch( ctrlParms.strLen ) {
            case 4:
                *pul = (unsigned long) ctrlParms.offset;
                break;
            case 2:
                *pus = (unsigned short) ctrlParms.offset;
                break;
            case 1:
                *puc = (unsigned char) ctrlParms.offset;
                break;
            }
#if !defined(CONFIG_BCM96816)
            /* This is placed as MoCA blocks are 32-bit only
            * accessible and following call makes access in terms
            * of bytes. Probably MoCA address range can be checked
            * here.
            */
            dumpaddr( (unsigned char *) ctrlParms.string, sizeof(long) );
#endif
            ctrlParms.result = 0;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_MIPS_SOFT_RESET:
        kerSysMipsSoftReset();
        break;

    case BOARD_IOCTL_LED_CTRL:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            kerSysLedCtrl((BOARD_LED_NAME)ctrlParms.strLen, (BOARD_LED_STATE)ctrlParms.offset);
            ret = 0;
        }
        break;

    case BOARD_IOCTL_GET_ID:
        if (copy_from_user((void*)&ctrlParms, (void*)arg,
            sizeof(ctrlParms)) == 0)
        {
            if( ctrlParms.string )
            {
                char p[NVRAM_BOARD_ID_STRING_LEN];
                kerSysNvRamGetBoardId(p);
#ifdef AEI_VDSL_CUSTOMER_CENTURYLINK
                if(strstr(p,"C1000"))
                {
                    memset(p,0,sizeof(p));
                    strcpy(p,"C1000A");
                }
                else if(strstr(p,"C2000"))
                {
                    memset(p,0,sizeof(p));
                    strcpy(p,"C2000A");
                }
#endif               
#ifdef AEI_VDSL_CUSTOMER_NCS
                if(strstr(p,"VB784WG"))
                {
                    memset(p,0,sizeof(p));
                    strcpy(p,"V1000H");

                }

#endif
                if( strlen(p) + 1 < ctrlParms.strLen )
                    ctrlParms.strLen = strlen(p) + 1;
                __copy_to_user(ctrlParms.string, p, ctrlParms.strLen);
            }

            ctrlParms.result = 0;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                sizeof(BOARD_IOCTL_PARMS));
        }
        break;

    case BOARD_IOCTL_GET_MAC_ADDRESS:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            ctrlParms.result = kerSysGetMacAddress( ucaMacAddr,
                ctrlParms.offset );

            if( ctrlParms.result == 0 )
            {
                __copy_to_user(ctrlParms.string, ucaMacAddr,
                    sizeof(ucaMacAddr));
            }

            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_RELEASE_MAC_ADDRESS:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            if (copy_from_user((void*)ucaMacAddr, (void*)ctrlParms.string, \
                NVRAM_MAC_ADDRESS_LEN) == 0)
            {
                ctrlParms.result = kerSysReleaseMacAddress( ucaMacAddr );
            }
            else
            {
                ctrlParms.result = -EACCES;
            }

            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else
            ret = -EFAULT;
        break;

#if defined(AEI_VDSL_CUSTOMER_NCS)
        case BOARD_IOCTL_GET_SN:
            if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0)
            {
                if( ctrlParms.string )
                {
                    NVRAM_DATA nvramData;
                    AEI_readNvramData(&nvramData);
                    __copy_to_user(ctrlParms.string, &nvramData.ulSerialNumber[0], 32 - 1);
                }

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
            }
            break;

        case BOARD_IOCTL_GET_FW_VERSION:
            if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0)
            {
                if (ctrlParms.offset==0){
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                __copy_to_user(ctrlParms.string, &nvramData.chFactoryFWVersion[0], 48 - 1);
                        }
                }else if(ctrlParms.offset==1){
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                __copy_to_user(ctrlParms.string, &nvramData.ulSerialNumber[0], 32 - 1);
                        }

                }else if(ctrlParms.offset==2){
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                __copy_to_user(ctrlParms.string, &nvramData.wpsPin[0], 32 - 1);
                        }

                }else if(ctrlParms.offset==3){
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                __copy_to_user(ctrlParms.string, &nvramData.wpaKey[0], 32 - 1);
                        }

                }
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                else if(ctrlParms.offset==4){
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                __copy_to_user(ctrlParms.string, &nvramData.adminPassword[0], 32 - 1);
                        }
                }
#endif

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
            }
            break;

        case BOARD_IOCTL_RESET_FW_VERSION:
                if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0)
            {
                if (ctrlParms.offset==0){
                        NVRAM_DATA nvramData;
                        AEI_readNvramData(&nvramData);
                        memset(&nvramData.chFactoryFWVersion[0],0,48);
                        nvramData.chFactoryFWVersion[0]='\0';
                        writeNvramDataCrcLocked(&nvramData);
                }else if(ctrlParms.offset==1){
                        NVRAM_DATA nvramData;
                        AEI_readNvramData(&nvramData);
                        memset(&nvramData.ulSerialNumber[0],0,32);
                        nvramData.ulSerialNumber[0]='\0';
                        writeNvramDataCrcLocked(&nvramData);

                }else if(ctrlParms.offset==2){
                        NVRAM_DATA nvramData;
                        AEI_readNvramData(&nvramData);
                        memset(&nvramData.wpsPin[0],0,32);
                        nvramData.wpsPin[0]='\0';
                        writeNvramDataCrcLocked(&nvramData);
                }else if(ctrlParms.offset==3){
                        NVRAM_DATA nvramData;
                        AEI_readNvramData(&nvramData);
                        memset(&nvramData.wpaKey[0],0,32);
                        nvramData.wpaKey[0]='\0';
                        writeNvramDataCrcLocked(&nvramData);
                 }
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                 else if(ctrlParms.offset==4){
                        NVRAM_DATA nvramData;
                        AEI_readNvramData(&nvramData);
                        memset(&nvramData.adminPassword[0],0,32);
                        nvramData.adminPassword[0]='\0';
                        writeNvramDataCrcLocked(&nvramData);
                 }
#endif

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));

             }
             break;
         case BOARD_IOCTL_SET_FW_VERSION:
                if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0)
            {

                if(ctrlParms.string && strlen(ctrlParms.string)>0)
                {
                        if (ctrlParms.offset==0){

                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                memset(&nvramData.chFactoryFWVersion[0],0,48);
                                strncpy(&nvramData.chFactoryFWVersion[0],ctrlParms.string,48-1);
                                writeNvramDataCrcLocked(&nvramData);
                        }else if(ctrlParms.offset==1){
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                memset(&nvramData.ulSerialNumber[0],0,32);
                                strncpy(&nvramData.ulSerialNumber[0],ctrlParms.string,32-1);
                                writeNvramDataCrcLocked(&nvramData);

                        }else if(ctrlParms.offset==2){
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                memset(&nvramData.wpsPin[0],0,32);
                                strncpy(&nvramData.wpsPin[0],ctrlParms.string,32-1);
                                writeNvramDataCrcLocked(&nvramData);

                        }else if(ctrlParms.offset==3){
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                memset(&nvramData.wpaKey[0],0,32);
                                strncpy(&nvramData.wpaKey[0],ctrlParms.string,32-1);
                                writeNvramDataCrcLocked(&nvramData);

                        }
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                        else if(ctrlParms.offset==4){
                                NVRAM_DATA nvramData;
                                AEI_readNvramData(&nvramData);
                                memset(&nvramData.adminPassword[0],0,32);
                                strncpy(&nvramData.adminPassword[0],ctrlParms.string,32-1);
                                writeNvramDataCrcLocked(&nvramData);

                        }
#endif
                }
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));

             }
             break;
#if defined(AEI_VDSL_CUSTOMER_TELUS)
         case BOARD_IOCTL_ADJUST_NVRAM:
            {

                {
                    NVRAM_DATA nvramData;
                    TELUS_V2000H_NVRAM_DATA telus_v2000h_nvramData;
                    AEI_readNvramData(&nvramData);
                    AEI_readNvramData((PNVRAM_DATA)&telus_v2000h_nvramData);
                    if(nvramData.ulVersion == 6 
						&& telus_v2000h_nvramData.ulSerialNumber[0] != '\0' 
						&& telus_v2000h_nvramData.chFactoryFWVersion[0] != '\0' 
						&& (unsigned char)telus_v2000h_nvramData.ulSerialNumber[0] != 0xFF 
						&& (unsigned char)telus_v2000h_nvramData.chFactoryFWVersion[0] != 0xFF 
						&& (nvramData.wlanParams[2] != 'p' || nvramData.wlanParams[3] != 'c' || nvramData.wlanParams[4] != 'i')
						&& strncmp(nvramData.ulSerialNumber,telus_v2000h_nvramData.ulSerialNumber,32)!=0)
                    {
                        memset(&nvramData, 0, sizeof(nvramData));

                        memcpy(&nvramData.szBootline[0],&telus_v2000h_nvramData.szBootline[0],NVRAM_BOOTLINE_LEN);
                        memcpy(&nvramData.szBoardId[0],&telus_v2000h_nvramData.szBoardId[0],NVRAM_BOARD_ID_STRING_LEN);
                        nvramData.ulMainTpNum = telus_v2000h_nvramData.ulMainTpNum;
                        nvramData.ulPsiSize = telus_v2000h_nvramData.ulPsiSize;
                        nvramData.ulNumMacAddrs = telus_v2000h_nvramData.ulNumMacAddrs;
                        memcpy(&nvramData.ucaBaseMacAddr[0],&telus_v2000h_nvramData.ucaBaseMacAddr[0],NVRAM_MAC_ADDRESS_LEN);

                        nvramData.pad = telus_v2000h_nvramData.pad;
                        nvramData.backupPsi = telus_v2000h_nvramData.backupPsi;
                        nvramData.ulCheckSumV4 = telus_v2000h_nvramData.ulCheckSumV4;

                        memcpy(&nvramData.gponSerialNumber[0],&telus_v2000h_nvramData.gponSerialNumber[0],NVRAM_GPON_SERIAL_NUMBER_LEN);
                        memcpy(&nvramData.gponPassword[0],&telus_v2000h_nvramData.gponPassword[0],NVRAM_GPON_PASSWORD_LEN);
                        memcpy(&nvramData.wpsDevicePin[0],&telus_v2000h_nvramData.wpsDevicePin[0],NVRAM_WPS_DEVICE_PIN_LEN);
                        memcpy(&nvramData.wlanParams[0],&telus_v2000h_nvramData.wlanParams[0],NVRAM_WLAN_PARAMS_LEN);

                        nvramData.ulSyslogSize = telus_v2000h_nvramData.ulSyslogSize;

                        memcpy(&nvramData.ulNandPartOfsKb[0], &telus_v2000h_nvramData.ulNandPartOfsKb[0], sizeof(nvramData.ulNandPartOfsKb));
                        memcpy(&nvramData.ulNandPartSizeKb[0], &telus_v2000h_nvramData.ulNandPartSizeKb[0], sizeof(nvramData.ulNandPartSizeKb));
                        memcpy(&nvramData.szVoiceBoardId[0], &telus_v2000h_nvramData.szVoiceBoardId[0], sizeof(nvramData.szVoiceBoardId));

                        memcpy(&nvramData.afeId[0], &telus_v2000h_nvramData.afeId[0], sizeof(nvramData.afeId));

                        memcpy(&nvramData.ulSerialNumber[0],&telus_v2000h_nvramData.ulSerialNumber[0],32);
                        memcpy(&nvramData.chFactoryFWVersion[0],&telus_v2000h_nvramData.chFactoryFWVersion[0],48);
                        memcpy(&nvramData.wpsPin[0],&telus_v2000h_nvramData.wpsPin[0],32);
                        memcpy(&nvramData.wpaKey[0],&telus_v2000h_nvramData.wpaKey[0],32);
                        nvramData.dslDatapump=telus_v2000h_nvramData.dslDatapump;

                        nvramData.ulVersion=NVRAM_VERSION_NUMBER;

						memset((char *)&nvramData + ((size_t) &((TELUS_V2000H_NVRAM_DATA *)0)->ulSerialNumber), 0, 1);
						memset((char *)&nvramData + ((size_t) &((TELUS_V2000H_NVRAM_DATA *)0)->chFactoryFWVersion), 0, 1);

                        nvramData.ulCheckSum = getCrc32((unsigned char *)&nvramData, sizeof(NVRAM_DATA),CRC32_INIT_VALUE);

                        writeNvramDataCrcLocked(&nvramData);
                    }

                    if(nvramData.ulVersion == 5)
                    {
                        NVRAM_DATA_OLD nvramDataOld;
                        AEI_readNvramData((PNVRAM_DATA)&nvramDataOld);

                        memset(&nvramData, 0, sizeof(nvramData));

                        nvramData.ulVersion=NVRAM_VERSION_NUMBER;

                        memcpy(&nvramData.szBootline[0],&nvramDataOld.szBootline[0],NVRAM_BOOTLINE_LEN);
                        memcpy(&nvramData.szBoardId[0],&nvramDataOld.szBoardId[0],NVRAM_BOARD_ID_STRING_LEN);
                        nvramData.ulMainTpNum = nvramDataOld.ulMainTpNum;
                        nvramData.ulPsiSize = nvramDataOld.ulPsiSize;
                        nvramData.ulNumMacAddrs = nvramDataOld.ulNumMacAddrs;
                        memcpy(&nvramData.ucaBaseMacAddr[0],&nvramDataOld.ucaBaseMacAddr[0],NVRAM_MAC_ADDRESS_LEN);

                        memcpy(&nvramData.ulSerialNumber[0],&nvramDataOld.ulSerialNumber[0],32);
                        memcpy(&nvramData.chFactoryFWVersion[0],&nvramDataOld.chFactoryFWVersion[0],48);
                        memcpy(&nvramData.wpsPin[0],&nvramDataOld.wpsPin[0],32);
                        memcpy(&nvramData.wpaKey[0],&nvramDataOld.wpaKey[0],32);

                        nvramData.ulCheckSumV4 = nvramDataOld.ulCheckSumV4;

                        memcpy(&nvramData.gponSerialNumber[0],&nvramDataOld.gponSerialNumber[0],NVRAM_GPON_SERIAL_NUMBER_LEN);
                        memcpy(&nvramData.gponPassword[0],&nvramDataOld.gponPassword[0],NVRAM_GPON_PASSWORD_LEN);
                        memcpy(&nvramData.wpsDevicePin[0],&nvramDataOld.wpsDevicePin[0],NVRAM_WPS_DEVICE_PIN_LEN);
                        memcpy(&nvramData.wlanParams[0],&nvramDataOld.wlanParams[0],NVRAM_WLAN_PARAMS_LEN);

                        nvramData.ulCheckSum = getCrc32((unsigned char *)&nvramData, sizeof(NVRAM_DATA),CRC32_INIT_VALUE);

                        writeNvramDataCrcLocked(&nvramData);
                    }
                }

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));

             }
             break;

#endif
#endif   //AEI_VDSL_CUSTOMER_NCS
#ifdef AEI_VDSL_UPGRADE_DUALIMG_HISTORY_SPAD
    case BOARD_IOCTL_GET_DUAL_FW_VERSION:
            if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0)
            {

                if (ctrlParms.offset==0){ //image 0
                        if( ctrlParms.string )
                        {
                              PFILE_TAG pTag1 = getTagFromPartition(1);
                              if(pTag1)
                              {
#if defined(AEI_TWO_IN_ONE_FIRMWARE)
                                  unsigned char boardid[16]={0};
#endif                              
#if defined(AEI_TWO_IN_ONE_FIRMWARE) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                                  if((kerSysGetBoardID(boardid)==0) && (strcmp(boardid,"C2000A")==0))
                                  {
									  __copy_to_user(ctrlParms.string, pTag1->imageVersion, SIG_LEN_2 - 1);
                                  }
                                  else
#endif
                                      __copy_to_user(ctrlParms.string, pTag1->signiture_2, SIG_LEN_2 - 1);
                              }
                              else
                                  __copy_to_user(ctrlParms.string, "", SIG_LEN_2 - 1);
                        }
                }else if(ctrlParms.offset==1){ //image 1
                        if( ctrlParms.string )
                        {
                              PFILE_TAG pTag2 = getTagFromPartition(2);
                              if(pTag2)
                              {
#if defined(AEI_TWO_IN_ONE_FIRMWARE)
                                  unsigned char boardid[16]={0};
#endif                              
#if defined(AEI_TWO_IN_ONE_FIRMWARE) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                                  if((kerSysGetBoardID(boardid)==0) && (strcmp(boardid,"C2000A")==0))
                                  {
									  __copy_to_user(ctrlParms.string, pTag2->imageVersion, SIG_LEN_2 - 1);
                                  }
                                  else
#endif                            
                                      __copy_to_user(ctrlParms.string, pTag2->signiture_2, SIG_LEN_2 - 1);
                              }
                              else
                                  __copy_to_user(ctrlParms.string, "", SIG_LEN_2 - 1);
				
                        }

                }

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
            }
	break;		
    case BOARD_IOCTL_GET_DUAL_UG_INFO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg,
                sizeof(ctrlParms)) == 0)
            {

                if (ctrlParms.offset==0){ //status
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
					AEI_readNvramData(&nvramData);
                                __copy_to_user(ctrlParms.string, &nvramData.ugstatus[0], 3-1);
                        }
                }else if(ctrlParms.offset==1){ //history image1
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
					AEI_readNvramData(&nvramData);				
                                __copy_to_user(ctrlParms.string, &nvramData.ugimage1[0], 30-1);
                        }

                }else if(ctrlParms.offset==2){ //history image1
                        if( ctrlParms.string )
                        {
                                NVRAM_DATA nvramData;
					AEI_readNvramData(&nvramData);				
                                __copy_to_user(ctrlParms.string, &nvramData.ugimage2[0], 30-1);
                        }

                }

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
            }
					
	break;
    case BOARD_IOCTL_SET_DUAL_UG_INFO:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            if(ctrlParms.string && strlen(ctrlParms.string)>0)
            {
                if (ctrlParms.offset==0)
                {
                    NVRAM_DATA nvramData;
                    AEI_readNvramData(&nvramData);
                    memset(&nvramData.ugstatus[0],0,3);
                    strncpy(&nvramData.ugstatus[0],ctrlParms.string,3-1);
                    writeNvramDataCrcLocked(&nvramData);
                 }
                 else if(ctrlParms.offset==1)
                 {
                    NVRAM_DATA nvramData;
                    AEI_readNvramData(&nvramData);
                    memset(&nvramData.ugimage1[0],0,30);
                    strncpy(&nvramData.ugimage1[0],ctrlParms.string,30-1);
                    writeNvramDataCrcLocked(&nvramData);
                 }
                 else if(ctrlParms.offset==2)
                 {
                    NVRAM_DATA nvramData;
                    AEI_readNvramData(&nvramData);
                    memset(&nvramData.ugimage2[0],0,30);
                    strncpy(&nvramData.ugimage2[0],ctrlParms.string,30-1);
                    writeNvramDataCrcLocked(&nvramData);
                 }
             }
             ctrlParms.result = 0;
             __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
	break;	
#else
  case  BOARD_IOCTL_SET_UG_INFO:
         if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            if(ctrlParms.string && strlen(ctrlParms.string)>0)
            {
                if (ctrlParms.offset==0)
                {
                    char ug_info[2]={0};
                    strncpy(ug_info,ctrlParms.string,1);
                    kerSysScratchPadSet("UpGrade_Info",ug_info,sizeof(ug_info));
                 }
             }
             ctrlParms.result = 0;
             __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
	break;
 case  BOARD_IOCTL_GET_UG_INFO:
	if (copy_from_user((void*)&ctrlParms, (void*)arg,sizeof(ctrlParms)) == 0)
            {
                if (ctrlParms.offset==0)
	     {
                        if( ctrlParms.string )
                        {
                                 char ug_info[2]={0};
                                 kerSysScratchPadGet("UpGrade_Info",ug_info,sizeof(ug_info));
                                __copy_to_user(ctrlParms.string, ug_info,1);
                        }
                 }
	ctrlParms.result = 0;
             __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
              }
	break;
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
    case BOARD_IOCTL_GET_PRODUCTID:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            if (ctrlParms.offset==0)
            { //image 0
                printk("ioctrl set image 0\n");
                if( ctrlParms.string )
                {
                    PFILE_TAG pTag1 = getTagFromPartition(1);
                    if(pTag1)
                        __copy_to_user(ctrlParms.string, pTag1->signiture_1, 10);
		    else
			__copy_to_user(ctrlParms.string, "", 10);
                }
             }
             else if(ctrlParms.offset==1)
             { //image 1
                        if( ctrlParms.string )
                        {
    		  			PFILE_TAG pTag2 = getTagFromPartition(2);
					if(pTag2)
                                __copy_to_user(ctrlParms.string, pTag2->signiture_1, 10);		
						else
						__copy_to_user(ctrlParms.string, "", 10);
				
                        }

                }
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
        }
	break;		
    case BOARD_IOCTL_GET_FS_OFFSET:
        {
#if defined(AEI_63168_CHIP) && defined(AEI_CONFIG_JFFS)
            struct mtd_info *mtd1 = get_mtd_device_nm("nvram");
            if( mtd1 )
            {
                ctrlParms.result = mtd1->erasesize;
            }
            else
            {
                ctrlParms.result = 0;
            }
#else
            FLASH_ADDR_INFO fInfo;
            kerSysFlashAddrInfoGet(&fInfo);
            ctrlParms.result = fInfo.flash_rootfs_start_offset;
#endif
            //printk("###offset(%x)\n",fInfo.flash_rootfs_start_offset);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
	    break;		
#endif
    case BOARD_IOCTL_GET_PSI_SIZE:
        {
            FLASH_ADDR_INFO fInfo;
            kerSysFlashAddrInfoGet(&fInfo);
            ctrlParms.result = fInfo.flash_persistent_length;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        break;

    case BOARD_IOCTL_GET_BACKUP_PSI_SIZE:
        {
            FLASH_ADDR_INFO fInfo;
            kerSysFlashAddrInfoGet(&fInfo);
            // if number_blks > 0, that means there is a backup psi, but length is the same
            // as the primary psi (persistent).

            ctrlParms.result = (fInfo.flash_backup_psi_number_blk > 0) ?
                fInfo.flash_persistent_length : 0;
            printk("backup_psi_number_blk=%d result=%d\n", fInfo.flash_backup_psi_number_blk, fInfo.flash_persistent_length);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        break;

    case BOARD_IOCTL_GET_SYSLOG_SIZE:
        {
            FLASH_ADDR_INFO fInfo;
            kerSysFlashAddrInfoGet(&fInfo);
            ctrlParms.result = fInfo.flash_syslog_length;
       //     printk("---------syslog size = %d\n",fInfo.flash_syslog_length);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        break;

    case BOARD_IOCTL_GET_SDRAM_SIZE:
        ctrlParms.result = (int) g_ulSdramSize;
        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        ret = 0;
        break;

    case BOARD_IOCTL_GET_BASE_MAC_ADDRESS:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            __copy_to_user(ctrlParms.string, g_pMacInfo->ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN);
            ctrlParms.result = 0;

            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else
            ret = -EFAULT;
        break;

    case BOARD_IOCTL_GET_CHIP_ID:
        ctrlParms.result = kerSysGetChipId();


        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        ret = 0;
        break;

    case BOARD_IOCTL_GET_CHIP_REV:
        ctrlParms.result = (int) (PERF->RevID & REV_ID_MASK);
        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        ret = 0;
        break;

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    case BOARD_IOCTL_GET_FLASH_TOTAL:
         ctrlParms.result = flash_get_total_size()/1024;
         __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));
         ret = 0;
        break;
    case BOARD_IOCTL_GET_FLASH_USED:
         ctrlParms.result = getFlashUsedSize();
         __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));
         ret = 0;
        break;
    case BOARD_IOCTL_GET_POWERLED_STATUS:
         ctrlParms.result = gPowerLedStatus;
         __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));
         ret = 0;
        break;
#endif
	
    case BOARD_IOCTL_GET_NUM_ENET_MACS:
    case BOARD_IOCTL_GET_NUM_ENET_PORTS:
        {
            ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
            int i, cnt, numEthPorts = 0;
            if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
                for( i = 0; i < BP_MAX_ENET_MACS; i++) {
                    if (EnetInfos[i].ucPhyType != BP_ENET_NO_PHY) {
                        bitcount(cnt, EnetInfos[i].sw.port_map);
                        numEthPorts += cnt;
                    }
                }
                ctrlParms.result = numEthPorts;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else {
                ret = -EFAULT;
            }
            break;
        }

    case BOARD_IOCTL_GET_CFE_VER:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            char vertag[CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE];
            kerSysCfeVersionGet(vertag, sizeof(vertag));
            if (ctrlParms.strLen < CFE_VERSION_SIZE) {
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = -EFAULT;
            }
            else if (strncmp(vertag, "cfe-v", 5)) { // no tag info in flash
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else {
                ctrlParms.result = 1;
                __copy_to_user(ctrlParms.string, vertag+CFE_VERSION_MARK_SIZE, CFE_VERSION_SIZE);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
        }
        else {
            ret = -EFAULT;
        }
        break;

#if defined (WIRELESS)
    case BOARD_IOCTL_GET_WLAN_ANT_INUSE:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            unsigned short antInUse = 0;
            if (BpGetWirelessAntInUse(&antInUse) == BP_SUCCESS) {
                if (ctrlParms.strLen == sizeof(antInUse)) {
                    __copy_to_user(ctrlParms.string, &antInUse, sizeof(antInUse));
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                } else
                    ret = -EFAULT;
            }
            else {
                ret = -EFAULT;
            }
            break;
        }
        else {
            ret = -EFAULT;
        }
        break;
#endif
    case BOARD_IOCTL_SET_TRIGGER_EVENT:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
            ctrlParms.result = -EFAULT;
            ret = -EFAULT;
            if (ctrlParms.strLen == sizeof(unsigned long)) {
                board_ioc->eventmask |= *((int*)ctrlParms.string);
#if defined (WIRELESS)
                if((board_ioc->eventmask & SES_EVENTS)) {
                    if(sesBtn_irq != BP_NOT_DEFINED) {
                        BcmHalInterruptEnable(sesBtn_irq);
                        ctrlParms.result = 0;
                        ret = 0;
                    }
                }
#endif
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            break;
        }
        else {
            ret = -EFAULT;
        }
        break;

    case BOARD_IOCTL_GET_TRIGGER_EVENT:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
            if (ctrlParms.strLen == sizeof(unsigned long)) {
                __copy_to_user(ctrlParms.string, &board_ioc->eventmask, sizeof(unsigned long));
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } else
                ret = -EFAULT;

            break;
        }
        else {
            ret = -EFAULT;
        }
        break;

    case BOARD_IOCTL_UNSET_TRIGGER_EVENT:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            if (ctrlParms.strLen == sizeof(unsigned long)) {
                BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
                board_ioc->eventmask &= (~(*((int*)ctrlParms.string)));
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } else
                ret = -EFAULT;

            break;
        }
        else {
            ret = -EFAULT;
        }
        break;
#if defined (WIRELESS)
    case BOARD_IOCTL_SET_SES_LED:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            if (ctrlParms.strLen == sizeof(int)) {
                sesLed_ctrl(*(int*)ctrlParms.string);
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } else
                ret = -EFAULT;

            break;
        }
        else {
            ret = -EFAULT;
        }
        break;
#if defined(AEI_VOIP_LED)
     case AEI_BOARD_IOCTL_VOIP_LED:
#if defined(AEI_63168_CHIP)
         if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
             if (ctrlParms.strLen == 8) {
                 AEI_VoipLed_ctrl((char *)ctrlParms.string);
                 ctrlParms.result = 0;
                 __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                 ret = 0;
             } else
                 ret = -EFAULT;

             break;
         }
         else {
             ret = -EFAULT;
         }
#endif
         break;
#endif 
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)         
     case AEI_BOARD_IOCTL_WIRELESS_REDLED:
#if defined(AEI_63168_CHIP)
         if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
             if (ctrlParms.strLen == sizeof(int)) {
                 AEI_wlanLed_ctrl(*(int*)ctrlParms.string);
                 ctrlParms.result = 0;
                 __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                 ret = 0;
             } else
                 ret = -EFAULT;
 
             break;
         }
         else {
             ret = -EFAULT;
         }
#endif
         break;
    case BOARD_IOCTL_SET_WLANLEDMODE:
#if defined(AEI_63168_CHIP)
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {            
            /*change wireless led mode */
            if (ctrlParms.strLen == sizeof(int)) {
                if ( *(int*)ctrlParms.string == 1) {
                    GPIO->GPIOCtrl &= ~(0x3 << 4); /*wlan control*/
                }
                else
                    GPIO->GPIOCtrl |= (0x3 << 4); /*periph control*/

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }else
                ret = -EFAULT;
            break;
        }else {
            ret = -EFAULT;
        }
#endif
        break;
    case BOARD_IOCTL_GET_WLANLEDMODE:
#if defined(AEI_63168_CHIP)
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            int ledctrlmode = 0;
            int temp = 0;
            /*Get wireless led mode */
            if (ctrlParms.strLen == sizeof(int)) {
                printk("%x\n",GPIO->GPIOCtrl);
                temp = GPIO->GPIOCtrl & (0x3 << 4);
                if ( temp == 0x0){ 
                    ledctrlmode = 1;/*wlan control*/  
                    printk("ledctrlmode=wlan control\n");
                }              
                else {
                    ledctrlmode = 0;/*periph control*/
                    printk("ledctrlmode=periph control\n");
                }
                    
                __copy_to_user(ctrlParms.string,(char *)&ledctrlmode,sizeof(int));   

                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }else
                ret = -EFAULT;
            break;
        }else {
            ret = -EFAULT;
        }
#endif /*AEI_63168_CHIP*/
        break;      
#endif /*end AEI_VDSL_CUSTOMER_CENTURYLINK*/        
#endif /*end WIRELESS*/    

    case BOARD_IOCTL_SET_MONITOR_FD:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {

           g_monitor_nl_pid =  ctrlParms.offset;
           printk(KERN_INFO "monitor task is initialized pid= %d \n",g_monitor_nl_pid);
        }
        break;

    case BOARD_IOCTL_WAKEUP_MONITOR_TASK:
        kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_WAKEUP_MONITOR_TASK, NULL, 0);
        break;

    case BOARD_IOCTL_SET_CS_PAR:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            ret = ConfigCs(&ctrlParms);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
        else {
            ret = -EFAULT;
        }
        break;

    case BOARD_IOCTL_SET_GPIO:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            kerSysSetGpioState(ctrlParms.strLen, ctrlParms.offset);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else {
            ret = -EFAULT;
        }
        break;

#if defined(CONFIG_BCM_CPLD1)
    case BOARD_IOCTL_SET_SHUTDOWN_MODE:
        BcmCpld1SetShutdownMode();
        ret = 0;
        break;

    case BOARD_IOCTL_SET_STANDBY_TIMER:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            BcmCpld1SetStandbyTimer(ctrlParms.offset);
            ret = 0;
        }
        else {
            ret = -EFAULT;
        }
        break;
#endif

    case BOARD_IOCTL_BOOT_IMAGE_OPERATION:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            switch(ctrlParms.offset)
            {
            case BOOT_SET_PART1_IMAGE:
            case BOOT_SET_PART2_IMAGE:
            case BOOT_SET_PART1_IMAGE_ONCE:
            case BOOT_SET_PART2_IMAGE_ONCE:
            case BOOT_SET_OLD_IMAGE:
            case BOOT_SET_NEW_IMAGE:
            case BOOT_SET_NEW_IMAGE_ONCE:
                ctrlParms.result = kerSysSetBootImageState(ctrlParms.offset);
                break;

            case BOOT_GET_BOOT_IMAGE_STATE:
                ctrlParms.result = kerSysGetBootImageState();
                break;

            case BOOT_GET_IMAGE_VERSION:
                /* ctrlParms.action is parition number */
                ctrlParms.result = getImageVersion((int) ctrlParms.action,
                    ctrlParms.string, ctrlParms.strLen);
                break;

            case BOOT_GET_BOOTED_IMAGE_ID:
                /* ctrlParm.strLen == 1: partition or == 0: id (new or old) */
                ctrlParms.result = getBootedValue(ctrlParms.strLen);
                break;

            default:
                ctrlParms.result = -EFAULT;
                break;
            }
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else {
            ret = -EFAULT;
        }
        break;

    case BOARD_IOCTL_GET_TIMEMS:
        ret = jiffies_to_msecs(jiffies - INITIAL_JIFFIES);
        break;

    case BOARD_IOCTL_GET_DEFAULT_OPTICAL_PARAMS:
    {
        unsigned char ucDefaultOpticalParams[NVRAM_OPTICAL_PARAMS_SIZE];
            
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            ret = 0;
            if (BP_SUCCESS == (ctrlParms.result = BpGetDefaultOpticalParams(ucDefaultOpticalParams)))
            {
                __copy_to_user(ctrlParms.string, ucDefaultOpticalParams, NVRAM_OPTICAL_PARAMS_SIZE);

                if (__copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS)) != 0)
                {
                    ret = -EFAULT;
                }
            }                        
        }
        else
        {
            ret = -EFAULT;
        }

        break;
    }
    
    break;
    case BOARD_IOCTL_GET_GPON_OPTICS_TYPE:
     
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {
            unsigned short Temp=0;
            BpGetGponOpticsType(&Temp);
            *((UINT32*)ctrlParms.buf) = Temp;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }        
        ret = 0;

        break;

#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM963268)
    case BOARD_IOCTL_SPI_SLAVE_INIT:  
        ret = 0;
        if (kerSysBcmSpiSlaveInit() != SPI_STATUS_OK)  
        {
            ret = -EFAULT;
        }        
        break;   
        
    case BOARD_IOCTL_SPI_SLAVE_READ:  
        ret = 0;
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
             if (kerSysBcmSpiSlaveRead(ctrlParms.offset, (unsigned long *)ctrlParms.buf, ctrlParms.strLen) != SPI_STATUS_OK)  
             {
                 ret = -EFAULT;
             } 
             else
             {
                   __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));    
             }
        }
        else
        {
            ret = -EFAULT;
        }                 
        break;    
        
    case BOARD_IOCTL_SPI_SLAVE_WRITE:  
        ret = 0;
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
             if (kerSysBcmSpiSlaveWrite(ctrlParms.offset, ctrlParms.result, ctrlParms.strLen) != SPI_STATUS_OK)  
             {
                 ret = -EFAULT;
             } 
             else
             {
                   __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));    
             }
        }
        else
        {
            ret = -EFAULT;
        }                 
        break;    
#endif

#if defined(AEI_VDSL_SMARTLED)
    case BOARD_IOCTL_SET_INET_TRAFFIC_BLINK:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            register uint32 *pv32;

#ifndef AEI_63168_CHIP //For 6368
            pv32 = (uint32 *)0xb0000090;
            if (ctrlParms.strLen == 0)
               (*pv32) |= 0x00004000;
            else
               (*pv32) &= 0xffffbfff;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
            /*Activity blink of .5 seconds on and off during LAN to WAN and WAN to LAN activity.*/            
            pv32 = (uint32 *)0xb00018ac;
            //printk("18ac =%x\n",*pv32);
            (*pv32) |= 0x00000060; // 0x60 = 01100000
#endif        
#endif
            ctrlParms.result = 1;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else {
            ret = -EFAULT;
        }
        break;
#endif /* AEI_VDSL_SMARTLED */
#if defined(AEI_VDSL_CHECK_FLASH_ID)
    case AEI_BOARD_IOCTL_GET_NAND_FLASH_ID:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            int Temp=0;
            Temp=AEI_get_flash_mafId();
            *((UINT32*)ctrlParms.buf) = Temp;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
        ret = 0;
        break;
#endif
#if defined(AEI_VDSL_FACTORY_TELNET)
    case AEI_BOARD_IOCTL_SET_RESTOREDEFAULT_FLAG:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
        {
            if(ctrlParms.string && ctrlParms.strLen ==1)
            {
                if (ctrlParms.offset==0)
                {
                    kerSysScratchPadSet("RestoreDefault",ctrlParms.string,ctrlParms.strLen);
                }
            }
            ctrlParms.result = 0;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
       }
       break;
    case AEI_BOARD_IOCTL_GET_RESTOREDEFAULT_FLAG:
	if (copy_from_user((void*)&ctrlParms, (void*)arg,sizeof(ctrlParms)) == 0)
        {
             if (ctrlParms.offset==0)
	     {
                if( ctrlParms.string )
                {
                  kerSysScratchPadGet("RestoreDefault",ctrlParms.string,ctrlParms.strLen);
                  ctrlParms.result = 0;
                }
             }
             __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
	break;
#endif
    default:
        ret = -EINVAL;
        ctrlParms.result = 0;
        printk("board_ioctl: invalid command %x, cmd %d .\n",command,_IOC_NR(command));
        break;

    } /* switch */

    return (ret);

} /* board_ioctl */

/***************************************************************************
* SES Button ISR/GPIO/LED functions.
***************************************************************************/
#if defined (WIRELESS)

static Bool sesBtn_pressed(void)
{
    if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_0) && (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_3)) {
        if (!(PERF->ExtIrqCfg & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_0 + EI_STATUS_SHFT)))) {
            return 1;
        }
    }
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    else if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_4) || (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_5)) {
        if (!(PERF->ExtIrqCfg1 & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_4 + EI_STATUS_SHFT)))) {
            return 1;
        }
    }
#endif
    return 0;
}

static irqreturn_t sesBtn_isr(int irq, void *dev_id)
{
    if (sesBtn_pressed()){
        wake_up_interruptible(&g_board_wait_queue);
        return IRQ_RETVAL(1);
    } else {
        return IRQ_RETVAL(0);
    }
}

static void __init sesBtn_mapIntr(int context)
{
    if( BpGetWirelessSesExtIntr(&sesBtn_irq) == BP_SUCCESS )
    {
        printk("SES: Button Interrupt 0x%x is enabled\n", sesBtn_irq);
    }
    else
        return;

    sesBtn_irq = map_external_irq (sesBtn_irq) ;

    if (BcmHalMapInterrupt((FN_HANDLER)sesBtn_isr, context, sesBtn_irq)) {
        printk("SES: Interrupt mapping failed\n");
    }
    BcmHalInterruptEnable(sesBtn_irq);
}


static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{

    if (sesBtn_pressed()){
        return POLLIN;
    }
    return 0;
}

static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
    volatile unsigned int event=0;
    ssize_t ret=0;

    if(!sesBtn_pressed()){
        BcmHalInterruptEnable(sesBtn_irq);
        return ret;
    }
    event = SES_EVENTS;
    __copy_to_user((char*)buffer, (char*)&event, sizeof(event));
    BcmHalInterruptEnable(sesBtn_irq);
    count -= sizeof(event);
    buffer += sizeof(event);
    ret += sizeof(event);
    return ret;
}

static void __init sesLed_mapGpio()
{
    if( BpGetWirelessSesLedGpio(&sesLed_gpio) == BP_SUCCESS )
    {
        printk("SES: LED GPIO 0x%x is enabled\n", sesLed_gpio);
    }
}

#if defined(AEI_VOIP_LED)
static void AEI_VoipLed_ctrl(char *action)
{
    char * p = NULL;
    int line = 0;
    int status = 0;

    p = action;

    line = *p - 48 + kLedVoip1;
    p = p+2;
    status = *p - 48;

    if (status == kLedStateAmber){
        kerSysLedCtrl(line, kLedStateOn);
        kerSysLedCtrl(line, kLedStateOff);
    }
    else{
        kerSysLedCtrl(line, status);
    }

    return;
}
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
static void AEI_wlanLed_ctrl(int action)
{
    if (action == kLedStateAmber){
        kerSysLedCtrl(AEI_kLedWlanGreen, kLedStateOn);
        kerSysLedCtrl(AEI_kLedWlanRed, kLedStateOn);
    }
    else if (action == kLedStateOff) {
        kerSysLedCtrl(AEI_kLedWlanGreen, kLedStateOff);
        kerSysLedCtrl(AEI_kLedWlanRed, kLedStateOff);
    }
    else
        kerSysLedCtrl(AEI_kLedWlan, action);

    return;
}
#endif
#if defined(AEI_VDSL_CHECK_FLASH_ID)
static int  AEI_get_flash_mafId(void)
{   struct mtd_info *mtd=NULL;
    int  mafId=0;
    mtd=get_mtd_device_nm("tag");
    if(mtd!=NULL)
      mafId=mtd->get_fact_prot_info(mtd,NULL,0);
    else
      printk("mtd is NULL\n");
    return mafId;
}
#endif
static void sesLed_ctrl(int action)
{
    char blinktype = ((action >> 24) & 0xff); /* extract blink type for SES_LED_BLINK  */

    BOARD_LED_STATE led;

    if(sesLed_gpio == BP_NOT_DEFINED)
        return;

#if defined(AEI_VDSL_CUSTOMER_NCS)
    {
    char status = ((action >> 8) & 0xff); /* extract status */
    char event = ((action >> 16) & 0xff); /* extract event */

    if ((action & 0xff) == SES_LED_OFF) { /* extract led */
       kerSysLedCtrl(kLedSes, kLedStateOff);
       printk("< SES_LED_OFF >\n");
       return;  
    }       
    
    switch ((int) event) {
        case   WSC_EVENTS_PROC_IDLE:
            printk("< WSC_EVENTS_PROC_IDLE >\n");
            kerSysLedCtrl(kLedSes, kLedStateOn);
            break;
        case    WSC_EVENTS_PROC_WAITING:
            printk("< WSC_EVENTS_PROC_WAITING >\n");
            kerSysLedCtrl(kLedSes, kLedStateOn);
            kerSysLedCtrl(kLedSes, kLedStateUserWpsInProgress);
            break;
        case    WSC_EVENTS_PROC_SUCC:
            printk("< WSC_EVENTS_PROC_SUCC >\n");
            kerSysLedCtrl(kLedSes, kLedStateOn);
            break;
        case    WSC_EVENTS_PROC_FAIL:
            printk("< WSC_EVENTS_PROC_FAIL > status=%d\n", status);
            if ( status == 0 ) {
                kerSysLedCtrl(kLedSes, kLedStateFail);
                kerSysLedCtrl(kLedSes, kLedStateSlowBlinkContinues);
            }
            else if ( status == 1 ) {
                kerSysLedCtrl(kLedSes, kLedStateFail);
            }
            break;
        case    WSC_EVENTS_PROC_PBC_OVERLAP:
            printk("< WSC_EVENTS_PROC_PBC_OVERLAP >\n");
            if ( status == 0 ) {
                kerSysLedCtrl(kLedSes, kLedStateOn);
                kerSysLedCtrl(kLedSes, kLedStateSlowBlinkContinues);
            }
            else if ( status == 1 )
                kerSysLedCtrl(kLedSes, kLedStateOn);
            break;
        default:
            printk("< WSC_EVENTS_UNRECGNIZED >\n");
//            SetGpio(30,0);
//            SetGpio(23,0);
           //kerSysLedCtrl(kLedSes, kLedStateSlowBlinkContinues);
    }//switch
    }//block
#else


    action &= 0xff; /* extract led */

    switch (action) {
    case SES_LED_ON:
        led = kLedStateOn;
        break;
    case SES_LED_BLINK:
        if(blinktype)
            led = blinktype;
        else
            led = kLedStateSlowBlinkContinues;           		
        break;
    case SES_LED_OFF:
    default:
        led = kLedStateOff;
    }

    kerSysLedCtrl(kLedSes, led);
#endif //(AEI_VDSL_CUSTOMER_NCS)
}

static void __init ses_board_init()
{
    sesBtn_mapIntr(0);
    sesLed_mapGpio();
}
static void __exit ses_board_deinit()
{
    if(sesBtn_irq)
        BcmHalInterruptDisable(sesBtn_irq);
}
#endif

/***************************************************************************
* Dying gasp ISR and functions.
***************************************************************************/

static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp = NULL, *dslOrGpon = NULL;
	unsigned short usPassDyingGaspGpio;		// The GPIO pin to propogate a dying gasp signal

    UART->Data = 'D';
    UART->Data = '%';
    UART->Data = 'G';

#if defined (WIRELESS)
    kerSetWirelessPD(WLAN_OFF);
#endif
    /* first to turn off everything other than dsl or gpon */
    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
        if(strncmp(tmp->name, "dsl", 3) && strncmp(tmp->name, "gpon", 4)) {
            (tmp->cb_dgasp_fn)(tmp->context);
        }else {
            dslOrGpon = tmp;
        }
    }
	
    // Invoke dying gasp handlers
    if(dslOrGpon)
        (dslOrGpon->cb_dgasp_fn)(dslOrGpon->context);

    /* reset and shutdown system */

    /* Set WD to fire in 1 sec in case power is restored before reset occurs */
    TIMER->WatchDogDefCount = 1000000 * (FPERIPH/1000000);
    TIMER->WatchDogCtl = 0xFF00;
    TIMER->WatchDogCtl = 0x00FF;

	// If configured, propogate dying gasp to other processors on the board
	if(BpGetPassDyingGaspGpio(&usPassDyingGaspGpio) == BP_SUCCESS)
	    {
	    // Dying gasp configured - set GPIO
	    kerSysSetGpioState(usPassDyingGaspGpio, kGpioInactive);
	    }

    // If power is going down, nothing should continue!
    while (1);
    return( IRQ_HANDLED );
}

static void __init kerSysInitDyingGaspHandler( void )
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head != NULL) {
        printk("Error: kerSysInitDyingGaspHandler: list head is not null\n");
        return;
    }
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);
    g_cb_dgasp_list_head = new_node;

    BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, 0, INTERRUPT_ID_DG);
    BcmHalInterruptEnable( INTERRUPT_ID_DG );
} /* kerSysInitDyingGaspHandler */

static void __exit kerSysDeinitDyingGaspHandler( void )
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;

    if(g_cb_dgasp_list_head == NULL)
        return;

    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
        list_del(pos);
        kfree(tmp);
    }

    kfree(g_cb_dgasp_list_head);
    g_cb_dgasp_list_head = NULL;

} /* kerSysDeinitDyingGaspHandler */

void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
    CB_DGASP_LIST *new_node;

    // do all the stuff that can be done without the lock first
    if( devname == NULL || cbfn == NULL ) {
        printk("Error: kerSysRegisterDyingGaspHandler: register info not enough (%s,%x,%x)\n", devname, (unsigned int)cbfn, (unsigned int)context);
        return;
    }

    if (strlen(devname) > (IFNAMSIZ - 1)) {
        printk("Warning: kerSysRegisterDyingGaspHandler: devname too long, will be truncated\n");
    }

    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, IFNAMSIZ-1);
    new_node->cb_dgasp_fn = (cb_dgasp_t)cbfn;
    new_node->context = context;

    // OK, now acquire the lock and insert into list
    mutex_lock(&dgaspMutex);
    if( g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysRegisterDyingGaspHandler: list head is null\n");
        kfree(new_node);
    } else {
        list_add(&new_node->list, &g_cb_dgasp_list_head->list);
        printk("dgasp: kerSysRegisterDyingGaspHandler: %s registered \n", devname);
    }
    mutex_unlock(&dgaspMutex);

    return;
} /* kerSysRegisterDyingGaspHandler */

void kerSysDeregisterDyingGaspHandler(char *devname)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;
    int found=0;

    if(devname == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: devname is null\n");
        return;
    }

    printk("kerSysDeregisterDyingGaspHandler: %s is deregistering\n", devname);

    mutex_lock(&dgaspMutex);
    if(g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: list head is null\n");
    } else {
        list_for_each(pos, &g_cb_dgasp_list_head->list) {
            tmp = list_entry(pos, CB_DGASP_LIST, list);
            if(!strcmp(tmp->name, devname)) {
                list_del(pos);
                kfree(tmp);
                found = 1;
                printk("kerSysDeregisterDyingGaspHandler: %s is deregistered\n", devname);
                break;
            }
        }
        if (!found)
            printk("kerSysDeregisterDyingGaspHandler: %s not (de)registered\n", devname);
    }
    mutex_unlock(&dgaspMutex);

    return;
} /* kerSysDeregisterDyingGaspHandler */


/***************************************************************************
 *
 *
 ***************************************************************************/
static int ConfigCs (BOARD_IOCTL_PARMS *parms)
{
    int                     retv = 0;
#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96816)
    int                     cs, flags;
    cs_config_pars_t        info;

    if (copy_from_user(&info, (void*)parms->buf, sizeof(cs_config_pars_t)) == 0)
    {
        cs = parms->offset;

        MPI->cs[cs].base = ((info.base & 0x1FFFE000) | (info.size >> 13));

        if ( info.mode == EBI_TS_TA_MODE )     // syncronious mode
            flags = (EBI_TS_TA_MODE | EBI_ENABLE);
        else
        {
            flags = ( EBI_ENABLE | \
                (EBI_WAIT_STATES  & (info.wait_state << EBI_WTST_SHIFT )) | \
                (EBI_SETUP_STATES & (info.setup_time << EBI_SETUP_SHIFT)) | \
                (EBI_HOLD_STATES  & (info.hold_time  << EBI_HOLD_SHIFT )) );
        }
        MPI->cs[cs].config = flags;
        parms->result = BP_SUCCESS;
        retv = 0;
    }
    else
    {
        retv -= EFAULT;
        parms->result = BP_NOT_DEFINED;
    }
#endif
    return( retv );
}


/***************************************************************************
* Handle push of restore to default button
***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static void restore_to_default_thread(struct work_struct *work)
#else
static void restore_to_default_thread(void *arg)
#endif
{
    char buf[256];

    memset(buf, 0, sizeof(buf));

#if defined(AEI_VDSL_CUSTOMER_NCS)
 
    printk("\n*** Restore to Factory Default Setting ***\n\n");       
    printk("\r\nThe system is being reset. Please wait...\r\n");

    kerSysPersistentSet( buf, sizeof(buf), 0 );

#if defined(AEI_VDSL_CUSTOMER_BELLALIANT) || defined(AEI_VDSL_CUSTOMER_TELUS) || defined(SUPPORT_BACKUP_PSI)
    kerSysBackupPsiSet( buf, sizeof(buf), 0 );
#endif

#else	
    // Do this in a kernel thread so we don't have any restriction
    printk("Restore to Factory Default Setting ***\n\n");
    kerSysPersistentSet( buf, sizeof(buf), 0 );
#endif // AEI_VDSL_CUSTOMER_NCS

    // kernel_restart is a high level, generic linux way of rebooting.
    // It calls a notifier list and lets sub-systems know that system is
    // rebooting, and then calls machine_restart, which eventually
    // calls kerSysMipsSoftReset.
    kernel_restart(NULL);
    return;
}


#if defined(AEI_VDSL_CUSTOMER_NCS)
#if defined(AEI_63168_CHIP)
#define RESET_FLAG_LEFT_SHIFT 4
#else
#define RESET_FLAG_LEFT_SHIFT 6
#endif
#endif

#if defined(AEI_VDSL_CUSTOMER_QWEST)
static void AEI_resetBtnTimerFunc(unsigned long data)
{
    uint64 flag = ((uint64)0x10000000 << RESET_FLAG_LEFT_SHIFT);

    if (!(GPIO->GPIOio & flag))
    {
        holdTime++;

        if (holdTime >= RESET_HOLD_TIME && holdTime <= FACTORY_HOLD_TIME)
        {
            printk("Set power LED color\n");
            kerSysLedCtrl(kLedPower, kLedStateAmber);
        }
        else if (holdTime > FACTORY_HOLD_TIME)
        {
            printk("\r\nThe system is being reset. Hold it longer to get into bootloader...\r\n");
            kerSysMipsSoftReset();
        }
    }
    else
    {
        if (holdTime < RESET_HOLD_TIME)
        {
            printk("\r\nThe system is being reset. Please wait...\r\n");
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
            AEI_SaveSyslogOnReboot();
#endif
            kerSysMipsSoftReset();
        }
        else if (holdTime >= RESET_HOLD_TIME && holdTime <= FACTORY_HOLD_TIME)
        {
            char buf[256];

            memset(buf, 0, 256);

            printk("\n*** Restore to Factory Default Setting ***\n\n");
            printk("\r\nThe system is being reset. Please wait...\r\n");

            kerSysPersistentSet( buf, 256, 0 );
#ifdef SUPPORT_BACKUP_PSI
            kerSysBackupPsiSet( buf, 256, 0 );
#endif
#ifdef AEI_CONFIG_JFFS
            /*Now in 63268 Chip, we use jffs2 fs. So we need to sync data to flash from buffer before reboot system.*/
            sys_sync();
#endif

            kernel_restart(NULL);
        }
    }

    pTimer->expires = jiffies + HZ * RESET_POLL_TIME;
    add_timer(pTimer);

    return;
}
#elif defined(AEI_VDSL_CUSTOMER_BELLALIANT)
/* BA wants reset button to only restore default and not pause at cfe so just keep waiting until button released unless we change cfe also */
static void AEI_resetBtnTimerFunc(unsigned long data)
{
    uint64 flag = ((uint64)0x10000000 << RESET_FLAG_LEFT_SHIFT);

    if ((GPIO->GPIOio & flag))
    {
         printk("\n*** reset button released***\n");
         INIT_WORK(&restoreDefaultWork, restore_to_default_thread);
         schedule_work(&restoreDefaultWork);
    }
    else
    {
         printk("\n*** Still holding reset button ***\n");
         pTimer->expires = jiffies + HZ*RESET_POLL_TIME;
         add_timer(pTimer);
    }

    return;
}
#elif defined(AEI_VDSL_CUSTOMER_TELUS)
/* Telus requirement:  If reset button is held up to 3 seconds, do nothing.  If held from 3 to 30 seconds restore default
*/
static void AEI_resetBtnTimerFunc(unsigned long data)
{
    uint64 flag = ((uint64)0x10000000 << RESET_FLAG_LEFT_SHIFT);

    if (!(GPIO->GPIOio & flag))
    {
        holdTime++;

        if (holdTime > FACTORY_HOLD_TIME)
        {
            INIT_WORK(&restoreDefaultWork, restore_to_default_thread);
            schedule_work(&restoreDefaultWork);
            return;
        }
    }
    else
    {
        if (holdTime <= NOT_LONG_ENOUGH_TIME)
        { 
            printk("\n*** Reset held for %d seconds so not restoring factory default ***\n\n", holdTime);
            holdTime = 0;

            if (rirq != BP_NOT_DEFINED)
               BcmHalInterruptEnable(rirq);

            return;          
        }
        else if (holdTime <= FACTORY_HOLD_TIME)
        {
            INIT_WORK(&restoreDefaultWork, restore_to_default_thread);
            schedule_work(&restoreDefaultWork);
            return;
        }
    }

    pTimer->expires = jiffies + HZ * RESET_POLL_TIME;
    add_timer(pTimer);

    return;

}
#else
static void AEI_resetBtnTimerFunc(unsigned long data)
{
    INIT_WORK(&restoreDefaultWork, restore_to_default_thread);
    schedule_work(&restoreDefaultWork);
    return;
}
#endif

static irqreturn_t reset_isr(int irq, void *dev_id)
{
    printk("\n***reset button press detected***\n\n");
#if defined(AEI_VDSL_CUSTOMER_NCS)
    /* Create a timer which fires every seconds */
    pTimer = &resetBtnTimer;
    init_timer(pTimer);
    pTimer->function = AEI_resetBtnTimerFunc;
    pTimer->data = 0;

#if defined (AEI_VDSL_CUSTOMER_TELUS)
    rirq = irq;
#endif

    /* Start the timer */
    AEI_resetBtnTimerFunc(0);
#else
    INIT_WORK(&restoreDefaultWork, restore_to_default_thread);
    schedule_work(&restoreDefaultWork);
#endif
    return IRQ_HANDLED;
}



#if defined(WIRELESS)
/***********************************************************************
* Function Name: kerSysScreenPciDevices
* Description  : Screen Pci Devices before loading modules
***********************************************************************/
static void __init kerSysScreenPciDevices(void)
{
    unsigned short wlFlag;

    if((BpGetWirelessFlags(&wlFlag) == BP_SUCCESS) && (wlFlag & BP_WLAN_EXCLUDE_ONBOARD)) {
        /*
        * scan all available pci devices and delete on board BRCM wireless device
        * if external slot presents a BRCM wireless device
        */
        int foundPciAddOn = 0;
        struct pci_dev *pdevToExclude = NULL;
        struct pci_dev *dev = NULL;

        while((dev=pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev))!=NULL) {
            printk("kerSysScreenPciDevices: 0x%x:0x%x:(slot %d) detected\n", dev->vendor, dev->device, PCI_SLOT(dev->devfn));
            if((dev->vendor == BRCM_VENDOR_ID) &&
                (((dev->device & 0xff00) == BRCM_WLAN_DEVICE_IDS)|| 
                ((dev->device/1000) == BRCM_WLAN_DEVICE_IDS_DEC))) {
                    if(PCI_SLOT(dev->devfn) != WLAN_ONBOARD_SLOT) {
                        foundPciAddOn++;
                    } else {
                        pdevToExclude = dev;
                    }                
            }
        }

        if(((wlFlag & BP_WLAN_EXCLUDE_ONBOARD_FORCE) || foundPciAddOn) && pdevToExclude) {
            printk("kerSysScreenPciDevices: 0x%x:0x%x:(onboard) deleted\n", pdevToExclude->vendor, pdevToExclude->device);
            pci_remove_bus_device(pdevToExclude);
        }
    }
}

/***********************************************************************
* Function Name: kerSetWirelessPD
* Description  : Control Power Down by Hardware if the board supports
***********************************************************************/
static void kerSetWirelessPD(int state)
{
    unsigned short wlanPDGpio;
    if((BpGetWirelessPowerDownGpio(&wlanPDGpio)) == BP_SUCCESS) {
        if (wlanPDGpio != BP_NOT_DEFINED) {
            if(state == WLAN_OFF)
                kerSysSetGpioState(wlanPDGpio, kGpioActive);
            else
                kerSysSetGpioState(wlanPDGpio, kGpioInactive);
        }
    }
}

#endif


#if defined(CONFIG_BCM96816) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
/***********************************************************************
* Function Name: kerSysCheckPowerDownPcie
* Description  : Power Down PCIe if no device enumerated
*                Otherwise enable Power Saving modes
***********************************************************************/
static void __init kerSysCheckPowerDownPcie(void)
{
    struct pci_dev *dev = NULL;
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
    unsigned long GPIOOverlays;
#endif

    while ((dev=pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev))!=NULL) {
        if(BCM_BUS_PCIE_DEVICE == dev->bus->number) {
            /* Enable PCIe L1 PLL power savings */
            PCIEH_BLK_1800_REGS->phyCtrl[1] |= REG_POWERDOWN_P1PLL_ENA;
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268)
            /* Enable PCIe CLKREQ# power savings */
            if( (BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS) && (GPIOOverlays & BP_OVERLAY_PCIE_CLKREQ)) {
                PCIEH_BRIDGE_REGS->pcieControl |= PCIE_BRIDGE_CLKREQ_ENABLE;
            }
#endif
            return;
        }
    }
            
    printk("PCIe: No device found - Powering down\n");
    /* pcie clock disable*/
    PERF->blkEnables &= ~PCIE_CLK_EN;
#if defined(CONFIG_BCM963268)
    MISC->miscLcpll_ctrl |= MISC_CLK100_DISABLE;
#endif

    /* pcie serdes disable */
#if defined(CONFIG_BCM96816)   
    GPIO->SerdesCtl &= ~(SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
#endif
#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM963268)
    MISC->miscSerdesCtrl &= ~(SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
#endif

    /* pcie and ext device */
    PERF->softResetB &= ~(SOFT_RST_PCIE|SOFT_RST_PCIE_EXT|SOFT_RST_PCIE_CORE);
#if defined(CONFIG_BCM96328)
    PERF->softResetB &= ~SOFT_RST_PCIE_HARD;
#endif

}
#endif

#if defined(AEI_VDSL_DUAL_IMAGE) || defined(AEI_VER2_DUAL_IMAGE)
static int proc_get_bootimage_param(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
    int r = 0;
    int offset  = ((int *)data)[0];
    int length  = ((int *)data)[1];
    NVRAM_DATA *pNvramData;
    char bootPartition = BOOT_LATEST_IMAGE;
    char *p;

#if defined(AEI_VER2_DUAL_IMAGE)
	PFILE_TAG pTag1 = getTagFromPartition(1);
	PFILE_TAG pTag2 = getTagFromPartition(2);
	unsigned long sequence1 = pTag1?simple_strtoul(pTag1->imageSequence, NULL, 10):0;
	unsigned long sequence2 = pTag2?simple_strtoul(pTag2->imageSequence, NULL, 10):0;	 
#endif

    *eof = 1;

    if ((offset < 0) || (offset + length > sizeof(NVRAM_DATA)))
        return 0;

    pNvramData = kmalloc(sizeof(NVRAM_DATA), GFP_KERNEL);
    if (pNvramData == NULL)
    {
       return 0;
    }

    if (AEI_readNvramData(pNvramData) == 0)
    {
        for( p = pNvramData->szBootline; p[2] != '\0'; p++ )
        {
            if( p[0] == 'p' && p[1] == '=' )
            {
                bootPartition = p[2];
                break;
            }
        }

#if defined(AEI_VER2_DUAL_IMAGE)
//change bootPartition to Parition Order Number
    if( bootPartition == BOOT_LATEST_IMAGE )
        bootPartition = (sequence2 > sequence1) ? BOOT_PREVIOUS_IMAGE : BOOT_LATEST_IMAGE;
    else
        bootPartition = (sequence2 < sequence1) ? BOOT_PREVIOUS_IMAGE : BOOT_LATEST_IMAGE;
    if(pTag1 == NULL && pTag2 != NULL)
        bootPartition = BOOT_PREVIOUS_IMAGE;
    if(pTag1 != NULL && pTag2 == NULL)
        bootPartition = BOOT_LATEST_IMAGE;
#endif

        r += sprintf(page + r, "%c ", bootPartition);
    }

    r += sprintf(page + r, "\n");
    kfree(pNvramData);
    return (r < cnt)? r: 0;
}

static int proc_set_bootimage_param(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    NVRAM_DATA *pNvramData;
    char input[32];

    int offset  = ((int *)data)[0];
    int length  = ((int *)data)[1];
    int bskip = true;
    char *p = NULL;

#if defined(AEI_VER2_DUAL_IMAGE)
	PFILE_TAG pTag1 = getTagFromPartition(1);
	PFILE_TAG pTag2 = getTagFromPartition(2);
	unsigned long sequence1 = pTag1?simple_strtoul(pTag1->imageSequence, NULL, 10):0;
	unsigned long sequence2 = pTag2?simple_strtoul(pTag2->imageSequence, NULL, 10):0; 
	char cBootPartition = BOOT_LATEST_IMAGE;
#endif

    if ((offset < 0) || (offset + length > sizeof(NVRAM_DATA)))
        return 0;
    if ((cnt > 2) || (copy_from_user(input, buf, cnt) != 0))
        return -EFAULT;
#ifdef AEI_NAND_IMG_CHECK
	if(input[0] == '9')
	{
		if(gSetWrongCRC != 1)
		{
			gSetWrongCRC = 1; //1=set wrong crc
			printk("It will set wrong crc when you flash new image before reboot, poweroff, and set 0 or 1 again.\n");
		}
	}
	else
	{
		if(gSetWrongCRC != 0)
		{
			gSetWrongCRC = 0; //0=not set wrong crc
			printk("It will set correct crc when you flash new image.\n");
		}
	}
#endif	
		
    if(input[0]!=BOOT_LATEST_IMAGE && input[0]!=BOOT_PREVIOUS_IMAGE)
        return -EFAULT;

    pNvramData = kmalloc(sizeof(NVRAM_DATA), GFP_KERNEL);
    if (pNvramData == NULL)
    {
       printk("ERROR - Could not allocate memory for pNvramData (%s)\n", __FUNCTION__);
       return 0;
    }
    memset(pNvramData, 0, sizeof(NVRAM_DATA));

    if (AEI_readNvramData(pNvramData) == 0)
    {
        for( p = pNvramData->szBootline; p[2] != '\0'; p++ )
        {
            if( p[0] == 'p' && p[1] == '=' )
            {
#if defined(AEI_VER2_DUAL_IMAGE)
//change to really partition that user want to boot.
				if(input[0] == BOOT_LATEST_IMAGE )
					cBootPartition = (sequence2 > sequence1) ? BOOT_PREVIOUS_IMAGE : BOOT_LATEST_IMAGE;
				else
					cBootPartition = (sequence2 < sequence1) ? BOOT_PREVIOUS_IMAGE : BOOT_LATEST_IMAGE;
                if(pTag1 == NULL || pTag2 == NULL)
                    cBootPartition = BOOT_LATEST_IMAGE;				
				p[2]=cBootPartition;
#else
                p[2]=input[0];
#endif
                bskip = false;
                break;
            }
        }

        writeNvramDataCrcLocked(pNvramData);
        kfree(pNvramData);
        if (!bskip)
        {
#if defined(AEI_VER2_DUAL_IMAGE)
            kerSysSetBootImageState(cBootPartition == BOOT_LATEST_IMAGE ? '0' : '1');
#else
            kerSysSetBootImageState(input[0] == BOOT_LATEST_IMAGE ? '0' : '1');
#endif
        }
        return cnt;
    }
    kfree(pNvramData);
    return 0;
}

#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
static int proc_get_other_param(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
    int r = 0;
    int offset  = ((int *)data)[0];
    int length  = ((int *)data)[1];
    NVRAM_DATA *pNvramData = NULL;

    *eof = 1;

    if ((offset < 0) || (offset + length > sizeof(NVRAM_DATA)))
        return 0;

    pNvramData = kmalloc(sizeof(NVRAM_DATA), GFP_KERNEL);
    if (pNvramData == NULL)
    {
       return 0;
    }
    if (AEI_readNvramData(pNvramData) == 0)
    {
	r += snprintf(page + r, length, "%s", &((unsigned char *)pNvramData)[offset]);
    }

    r += sprintf(page + r, "\n");
    kfree(pNvramData);
    return (r < cnt)? r: 0;
}

static int proc_set_other_param(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    NVRAM_DATA *pNvramData = NULL;
    char input[64] = { 0 };
    int offset  = ((int *)data)[0];
    int length  = ((int *)data)[1];
    memset(input, 0, 64);
    if ((offset < 0) || (offset + length > sizeof(NVRAM_DATA)))
        return 0;
    if ((cnt  > length ) || (copy_from_user(input, buf, cnt) != 0))
        return -EFAULT;
    pNvramData = kmalloc(sizeof(NVRAM_DATA), GFP_KERNEL);
    if (pNvramData == NULL)
    {
       printk("ERROR - Could not allocate memory for pNvramData (%s)\n", __FUNCTION__);
       return 0;
    }
    if (AEI_readNvramData(pNvramData) == 0)
    {
        memset(((char *)pNvramData) + offset, 0, length);	
	strncpy(((char *)pNvramData) + offset, input, cnt - 1);
        writeNvramDataCrcLocked(pNvramData);
        kfree(pNvramData);
        return cnt;
    }
    kfree(pNvramData);
    return 0;
}
#endif



extern unsigned char g_blparms_buf[];

/***********************************************************************
 * Function Name: kerSysBlParmsGetInt
 * Description  : Returns the integer value for the requested name from
 *                the boot loader parameter buffer.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysBlParmsGetInt( char *name, int *pvalue )
{
    char *p2, *p1 = g_blparms_buf;
    int ret = -1;

    *pvalue = -1;

    /* The g_blparms_buf buffer contains one or more contiguous NULL termianted
     * strings that ends with an empty string.
     */
    while( *p1 )
    {
        p2 = p1;

        while( *p2 != '=' && *p2 != '\0' )
            p2++;

        if( *p2 == '=' )
        {
            *p2 = '\0';

            if( !strcmp(p1, name) )
            {
                *p2++ = '=';
                *pvalue = simple_strtol(p2, &p1, 0);
                if( *p1 == '\0' )
                    ret = 0;
                break;
            }

            *p2 = '=';
        }

        p1 += strlen(p1) + 1;
    }

    return( ret );
}

/***********************************************************************
 * Function Name: kerSysBlParmsGetStr
 * Description  : Returns the string value for the requested name from
 *                the boot loader parameter buffer.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysBlParmsGetStr( char *name, char *pvalue, int size )
{
    char *p2, *p1 = g_blparms_buf;
    int ret = -1;

    /* The g_blparms_buf buffer contains one or more contiguous NULL termianted
     * strings that ends with an empty string.
     */
    while( *p1 )
    {
        p2 = p1;

        while( *p2 != '=' && *p2 != '\0' )
            p2++;

        if( *p2 == '=' )
        {
            *p2 = '\0';

            if( !strcmp(p1, name) )
            {
                *p2++ = '=';
                strncpy(pvalue, p2, size);
                ret = 0;
                break;
            }

            *p2 = '=';
        }

        p1 += strlen(p1) + 1;
    }

    return( ret );
}

static int add_proc_files(void)
{
#define offset(type, elem) ((int)&((type *)0)->elem)

    static int BaseMacAddr[2] = {offset(NVRAM_DATA, ucaBaseMacAddr), NVRAM_MAC_ADDRESS_LEN};
#if defined(AEI_VDSL_DUAL_IMAGE) || defined(AEI_VER2_DUAL_IMAGE)
    static int BootImage[2] = {offset(NVRAM_DATA, szBootline), NVRAM_BOOTLINE_LEN};
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
    static int Serial_num[2] = {offset(NVRAM_DATA, ulSerialNumber), 32};
    static int Wpa_key[2] = {offset(NVRAM_DATA, wpaKey), 32};
    static int Wps_pin[2] = {offset(NVRAM_DATA,wpsPin), 32};
    static int Hw_ver[2] = {offset(NVRAM_DATA, chFactoryFWVersion), 48};
#endif
    struct proc_dir_entry *p0;
    struct proc_dir_entry *p1;

    p0 = proc_mkdir("nvram", NULL);

    if (p0 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1 = create_proc_entry("BaseMacAddr", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = BaseMacAddr;
    p1->read_proc   = proc_get_param;
    p1->write_proc  = proc_set_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
	//New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif

#if defined(AEI_VDSL_DUAL_IMAGE) || defined(AEI_VER2_DUAL_IMAGE)
    p1 = create_proc_entry("BootImage", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = BootImage;
    p1->read_proc   = proc_get_bootimage_param;
    p1->write_proc  = proc_set_bootimage_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
	//New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif

#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
    p1 = create_proc_entry("Serial_num", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = Serial_num;
    p1->read_proc   = proc_get_other_param;
    p1->write_proc  = proc_set_other_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        //New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif

    p1 = create_proc_entry("Wpa_key", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = Wpa_key;
    p1->read_proc   = proc_get_other_param;
    p1->write_proc  = proc_set_other_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        //New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif
 p1 = create_proc_entry("Wps_pin", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = Wps_pin;
    p1->read_proc   = proc_get_other_param;
    p1->write_proc  = proc_set_other_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        //New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif

    p1 = create_proc_entry("Hw_ver", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = Hw_ver;
    p1->read_proc   = proc_get_other_param;
    p1->write_proc  = proc_set_other_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        //New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif

#if 0
    p1 = create_proc_entry("Ssid", 0644, p0);

    if (p1 == NULL)
    {
        printk("add_proc_files: failed to create proc files!\n");
        return -1;
    }

    p1->data        = Ssid;
    p1->read_proc   = proc_get_other_param;
    p1->write_proc  = proc_set_other_param;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        //New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif
#endif
#endif
    p1 = create_proc_entry("led", 0644, NULL);
    if (p1 == NULL)
        return -1;

    p1->write_proc  = proc_set_led;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
	//New linux no longer requires proc_dir_entry->owner field.
#else
    p1->owner       = THIS_MODULE;
#endif

    return 0;
}

static int del_proc_files(void)
{
    remove_proc_entry("nvram", NULL);
    remove_proc_entry("led", NULL);
    return 0;
}

static void str_to_num(char* in, char* out, int len)
{
    int i;
    memset(out, 0, len);

    for (i = 0; i < len * 2; i ++)
    {
        if ((*in >= '0') && (*in <= '9'))
            *out += (*in - '0');
        else if ((*in >= 'a') && (*in <= 'f'))
            *out += (*in - 'a') + 10;
        else if ((*in >= 'A') && (*in <= 'F'))
            *out += (*in - 'A') + 10;
        else
            *out += 0;

        if ((i % 2) == 0)
            *out *= 16;
        else
            out ++;

        in ++;
    }
    return;
}

static int proc_get_param(char *page, char **start, off_t off, int cnt, int *eof, void *data)
{
    int i = 0;
    int r = 0;
    int offset  = ((int *)data)[0];
    int length  = ((int *)data)[1];
    NVRAM_DATA *pNvramData;

    *eof = 1;

    if ((offset < 0) || (offset + length > sizeof(NVRAM_DATA)))
        return 0;

    if (NULL != (pNvramData = readNvramData()))
    {
        for (i = 0; i < length; i ++)
            r += sprintf(page + r, "%02x ", ((unsigned char *)pNvramData)[offset + i]);
    }

    r += sprintf(page + r, "\n");
    if (pNvramData)
        kfree(pNvramData);
    return (r < cnt)? r: 0;
}

static int proc_set_param(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    NVRAM_DATA *pNvramData;
    char input[32];

    int i = 0;
    int r = cnt;
    int offset  = ((int *)data)[0];
    int length  = ((int *)data)[1];

    if ((offset < 0) || (offset + length > sizeof(NVRAM_DATA)))
        return 0;

    if ((cnt > 32) || (copy_from_user(input, buf, cnt) != 0))
        return -EFAULT;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]))
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    mutex_lock(&flashImageMutex);

    if (NULL != (pNvramData = readNvramData()))
    {
        str_to_num(input, ((char *)pNvramData) + offset, length);
        writeNvramDataCrcLocked(pNvramData);
    }
    else
    {
        cnt = 0;
    }

    mutex_unlock(&flashImageMutex);

    if (pNvramData)
        kfree(pNvramData);

    return cnt;
}

/*
 * This function expect input in the form of:
 * echo "xxyy" > /proc/led
 * where xx is hex for the led number
 * and   yy is hex for the led state.
 * For example,
 *     echo "0301" > led
 * will turn on led 3
 */
static int proc_set_led(struct file *f, const char *buf, unsigned long cnt, void *data)
{
    char leddata[16];
    char input[32];
    int i;
    int r;
    int num_of_octets;

    if (cnt > 32)
        cnt = 32;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    r = cnt;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]))
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    num_of_octets = r / 2;

    if (num_of_octets != 2)
        return -EFAULT;

    str_to_num(input, leddata, num_of_octets);
    kerSysLedCtrl (leddata[0], leddata[1]);
    return cnt;
}


#if defined(CONFIG_BCM96368)

#define DSL_PHY_PHASE_CNTL      ((volatile uint32* const) 0xb00012a8)
#define DSL_CPU_PHASE_CNTL      ((volatile uint32* const) 0xb00012ac)
#define MIPS_PHASE_CNTL         ((volatile uint32* const) 0xb00012b0)
#define DDR1_2_PHASE_CNTL       ((volatile uint32* const) 0xb00012b4)
#define DDR3_4_PHASE_CNTL       ((volatile uint32* const) 0xb00012b8)

// The direction bit tells the automatic counters to count up or down to the
// desired value.
#define PI_VALUE_WIDTH 14
#define PI_COUNT_UP    ( 1 << PI_VALUE_WIDTH )
#define PI_MASK        ( PI_COUNT_UP - 1 )

// Turn off sync mode.  Set bit 28 of CP0 reg 22 sel 5.
static void TurnOffSyncMode( void )
{
    uint32 value;

    value = __read_32bit_c0_register( $22, 5 ) | (1<<28);
    __write_32bit_c0_register( $22, 5, value );
    //    Print( "Sync mode %x\n", value );
    value = DDR->MIPSPhaseCntl;

    // Reset the PH_CNTR_CYCLES to 7.
    // Set the phase counter cycles (bits 16-19) back to 7.
    value &= ~(0xf<<16);
    value |= (7<<16);

    // Set the LLMB counter cycles back to 7.
    value &= ~(0xf<<24);
    value |= (7<<24);
    // Set the UBUS counter cycles back to 7.
    value &= ~(0xf<<28);
    value |= (7<<28);

    // Turn off the LLMB counter, which is what maintains sync mode.
    // Clear bit 21, which is LLMB_CNTR_EN.
    value &= ~(1 << 21);
    // Turn off UBUS LLMB CNTR EN
    value &= ~(1 << 23);

    DDR->MIPSPhaseCntl = value;

    // Reset the MIPS phase to 0.
    PI_lower_set( MIPS_PHASE_CNTL, 0 );

    //Clear Count Bit
    value &= ~(1 << 14);
    DDR->MIPSPhaseCntl = value;

}

// Write the specified value in the lower half of a PI control register.  Each
// 32-bit register holds two values, but they can't be addressed separately.
static void
PI_lower_set( volatile uint32  *PI_reg,
             int               newPhaseInt )
{
    uint32  oldRegValue;
    uint32  saveVal;
    int32   oldPhaseInt;
    int32   newPhase;
    uint32  newVal;
    int     equalCount      = 0;

    oldRegValue = *PI_reg;
    // Save upper 16 bits, which is the other PI value.
    saveVal     = oldRegValue & 0xffff0000;

    // Sign extend the lower PI value, and shift it down into the lower 16 bits.
    // This gives us a 32-bit signed value which we can compare to the newPhaseInt
    // value passed in.

    // Shift the sign bit to bit 31
    oldPhaseInt = oldRegValue << ( 32 - PI_VALUE_WIDTH );
    // Sign extend and shift the lower value into the lower 16 bits.
    oldPhaseInt = oldPhaseInt >> ( 32 - PI_VALUE_WIDTH );

    // Take the low 10 bits as the new phase value.
    newPhase = newPhaseInt & PI_MASK;

    // If our new value is larger than the old value, tell the automatic counter
    // to count up.
    if ( newPhaseInt > oldPhaseInt )
    {
        newPhase = newPhase | PI_COUNT_UP;
    }

    // Or in the value originally in the upper 16 bits.
    newVal  = newPhase | saveVal;
    *PI_reg = newVal;

    // Wait until we match several times in a row.  Only the low 4 bits change
    // while the counter is working, so we can get a false "done" indication
    // when we read back our desired value.
    do
    {
        if ( *PI_reg == newVal )
        {
            equalCount++;
        }
        else
        {
            equalCount = 0;
        }

    } while ( equalCount < 3 );

}

// Write the specified value in the upper half of a PI control register.  Each
// 32-bit register holds two values, but they can't be addressed separately.
static void
PI_upper_set( volatile uint32  *PI_reg,
             int               newPhaseInt )
{
    uint32  oldRegValue;
    uint32  saveVal;
    int32   oldPhaseInt;
    int32   newPhase;
    uint32  newVal;
    int     equalCount      = 0;

    oldRegValue = *PI_reg;
    // Save lower 16 bits, which is the other PI value.
    saveVal     = oldRegValue & 0xffff;

    // Sign extend the upper PI value, and shift it down into the lower 16 bits.
    // This gives us a 32-bit signed value which we can compare to the newPhaseInt
    // value passed in.

    // Shift the sign bit to bit 31
    oldPhaseInt = oldRegValue << ( 16 - PI_VALUE_WIDTH );
    // Sign extend and shift the upper value into the lower 16 bits.
    oldPhaseInt = oldPhaseInt >> ( 32 - PI_VALUE_WIDTH );

    // Take the low 10 bits as the new phase value.
    newPhase = newPhaseInt & PI_MASK;

    // If our new value is larger than the old value, tell the automatic counter
    // to count up.
    if ( newPhaseInt > oldPhaseInt )
    {
        newPhase = newPhase | PI_COUNT_UP;
    }

    // Shift the new phase value into the upper 16 bits, and restore the value
    // originally in the lower 16 bits.
    newVal = (newPhase << 16) | saveVal;
    *PI_reg = newVal;

    // Wait until we match several times in a row.  Only the low 4 bits change
    // while the counter is working, so we can get a false "done" indication
    // when we read back our desired value.
    do
    {
        if ( *PI_reg == newVal )
        {
            equalCount++;
        }
        else
        {
            equalCount = 0;
        }

    } while ( equalCount < 3 );

}

// Reset the DDR PI registers to the default value of 0.
static void ResetPiRegisters( void )
{
    volatile int delay;
    uint32 value;

    //Skip This step for now load_ph should be set to 0 for this anyways.
    //Print( "Resetting DDR phases to 0\n" );
    //PI_lower_set( DDR1_2_PHASE_CNTL, 0 ); // DDR1 - Should be a NOP.
    //PI_upper_set( DDR1_2_PHASE_CNTL, 0 ); // DDR2
    //PI_lower_set( DDR3_4_PHASE_CNTL, 0 ); // DDR3 - Must remain at 90 degrees for normal operation.
    //PI_upper_set( DDR3_4_PHASE_CNTL, 0 ); // DDR4

    // Need to have VDSL back in reset before this is done.
    // Disable VDSL Mip's
    GPIO->VDSLControl = GPIO->VDSLControl & ~VDSL_MIPS_RESET;
    // Disable VDSL Core
    GPIO->VDSLControl = GPIO->VDSLControl & ~(VDSL_CORE_RESET | 0x8);


    value = DDR->DSLCpuPhaseCntr;

    // Reset the PH_CNTR_CYCLES to 7.
    // Set the VDSL Mip's phase counter cycles (bits 16-19) back to 7.
    value &= ~(0xf<<16);
    value |= (7<<16);

    // Set the VDSL PHY counter cycles back to 7.
    value &= ~(0xf<<24);
    value |= (7<<24);
    // Set the VDSL AFE counter cycles back to 7.
    value &= ~(0xf<<28);
    value |= (7<<28);

    // Turn off the VDSL MIP's PHY auto counter
    value &= ~(1 << 20);
    // Clear bit 21, which is VDSL PHY CNTR_EN.
    value &= ~(1 << 21);
    // Turn off the VDSL AFE auto counter
    value &= ~(1 << 22);

    DDR->DSLCpuPhaseCntr = value;

    // Reset the VDSL MIPS phase to 0.
    PI_lower_set( DSL_PHY_PHASE_CNTL, 0 ); // VDSL PHY - should be NOP
    PI_upper_set( DSL_PHY_PHASE_CNTL, 0 ); // VDSL AFE - should be NOP
    PI_lower_set( DSL_CPU_PHASE_CNTL, 0 ); // VDSL MIP's

    //Clear Count Bits for DSL CPU
    value &= ~(1 << 14);
    DDR->DSLCpuPhaseCntr = value;
    //Clear Count Bits for DSL Core
    DDR->DSLCorePhaseCntl &= ~(1<<30);
    DDR->DSLCorePhaseCntl &= ~(1<<14);
    // Allow some settle time.
    delay = 100;
    while (delay--);

    printk("\n****** DDR->DSLCorePhaseCntl=%lu ******\n\n", (unsigned long)
        DDR->DSLCorePhaseCntl);

    // Turn off the automatic counters.
    // Clear bit 20, which is PH_CNTR_EN.
    DDR->MIPSPhaseCntl &= ~(1<<20);
    // Turn Back UBUS Signals to reset state
    DDR->UBUSPhaseCntl = 0x00000000;
    DDR->UBUSPIDeskewLLMB0 = 0x00000000;
    DDR->UBUSPIDeskewLLMB1 = 0x00000000;

}

static void ChipSoftReset(void)
{
    TurnOffSyncMode();
    ResetPiRegisters();
}
#endif


/***************************************************************************
 * Function Name: kerSysGetUbusFreq
 * Description  : Chip specific computation.
 * Returns      : the UBUS frequency value in MHz.
 ***************************************************************************/
unsigned long kerSysGetUbusFreq(unsigned long miscStrapBus)
{
   unsigned long ubus = UBUS_BASE_FREQUENCY_IN_MHZ;

#if defined(CONFIG_BCM96362)
   /* Ref RDB - 6362 */
   switch (miscStrapBus) {

      case 0x4 :
      case 0xc :
      case 0x14:
      case 0x1c:
      case 0x15:
      case 0x1d:
         ubus = 100;
         break;
      case 0x2 :
      case 0xa :
      case 0x12:
      case 0x1a:
         ubus = 96;
         break;
      case 0x1 :
      case 0x9 :
      case 0x11:
      case 0xe :
      case 0x16:
      case 0x1e:
         ubus = 200;
         break;
      case 0x6:
         ubus = 183;
         break;
      case 0x1f:
         ubus = 167;
         break;
      default:
         ubus = 160;
         break;
   }
#endif

   return (ubus);

}  /* kerSysGetUbusFreq */

#ifdef AEI_VDSL_CUSTOMER_NCS
void restoreDatapump(int value){
    NVRAM_DATA *pNvramData;

    if (NULL == (pNvramData = readNvramData()))
    {
        printk("%s could not read nvram data\n",__FUNCTION__);
        return;
    }

    pNvramData->dslDatapump=value;
    writeNvramDataCrcLocked(pNvramData);
}

int kerSysGetDslDatapump(unsigned char *dslDatapump)
{
    NVRAM_DATA *pNvramData;

    if (NULL == (pNvramData = readNvramData()))
        printk("%s could not read nvram data\n",__FUNCTION__);
    else
        *dslDatapump = (unsigned char)pNvramData->dslDatapump;
    return 0;
}
#endif

#ifdef AEI_TWO_IN_ONE_FIRMWARE
int kerSysGetBoardID(unsigned char *boardid)
{
    NVRAM_DATA *pNvramData;
    if(boardid==NULL) 
        return -1;

    if (NULL == (pNvramData = readNvramData()))
    {
        printk("%s could not read nvram data\n",__FUNCTION__);
        return -1;
    }
    else
        strcpy(boardid,pNvramData->szBoardId);
    return 0;
}

#endif
#if defined(AEI_VDSL_DOWNGRADE_NVRAM_ADJUST)

/*
 * Function Name: AEI_IsDownGrade_FromSDK12To6
 * Description  : Check whether it is downgrade form sdk1204 to sdk6 firmware.
 *                sdk6 firmware's version number is between 31.60L.0c and 31.60L.18,if firmware's minor number
 *                is "60", we will deal with it as downgrade from sdk1204 to sdk6.Maybe there's more effectvie
 *                ways to check it.
 *input         : buffer of firmware needed to write to flash.
 *return        : downgrading will return TRUE,otherwise will return FALSE
 */
       #define  BUFL            10
#if defined(AEI_VDSL_CUSTOMER_SASKTEL)
       #define  SDK6_BUILD_NMMBER  4
#else
       #define  SDK6_MINOR_NUMBER  "60L"
#endif
static UBOOL8 AEI_IsDownGrade_FromSDK12To6(char* string)
{
    PFILE_TAG pTag=NULL;
    char major[BUFL]={0};
    char minor[BUFL]={0};
    char build[BUFL]={0};
    if(string ==NULL)
    {
       printk("AEI_IsDownGrade_FromSDK12To6:Invalid string\n");
       return FALSE;
    }
    pTag=(PFILE_TAG)string;
    if(pTag->signiture_2 != NULL && (strlen(pTag->signiture_2) < SIG_LEN_2))
    {
       if(sscanf(pTag->signiture_2,"%2s.%3s.%s",major,minor,build))
       {
#if defined(AEI_VDSL_CUSTOMER_SASKTEL)
          int mbuild=0;
          sscanf(build,"%d",&mbuild);
          if(mbuild <= SDK6_BUILD_NMMBER)
#else
          if( strncmp(minor,SDK6_MINOR_NUMBER,3)==0 )
#endif
            return TRUE;
       }
    }
    return FALSE;
}
/* Function Name :AEI_DownGrade_AdjustNVRAM
 * Description   :Adjust NVRAM mapping when downgarde form sdk1204 to sdk6 firmware.
 */
static UBOOL8 AEI_DownGrade_AdjustNVRAM(void)
{

    NVRAM_DATA nvramData;
    TELUS_V2000H_NVRAM_DATA telus_v2000h_nvramData;
    AEI_readNvramData(&nvramData);
    AEI_readNvramData((PNVRAM_DATA)&telus_v2000h_nvramData);
    if(nvramData.ulVersion == 6
         && nvramData.ulSerialNumber[0] != '\0'
         && nvramData.chFactoryFWVersion[0] != '\0'
         && (unsigned char)nvramData.ulSerialNumber[0] != 0xFF
         && (unsigned char)nvramData.chFactoryFWVersion[0] != 0xFF
         && strncmp(nvramData.ulSerialNumber,telus_v2000h_nvramData.ulSerialNumber,32)!=0)
    {
        memset(&telus_v2000h_nvramData, 0, sizeof(telus_v2000h_nvramData));

        memcpy(&telus_v2000h_nvramData.szBootline[0],&nvramData.szBootline[0],NVRAM_BOOTLINE_LEN);
        memcpy(&telus_v2000h_nvramData.szBoardId[0],&nvramData.szBoardId[0],NVRAM_BOARD_ID_STRING_LEN);
        telus_v2000h_nvramData.ulMainTpNum = nvramData.ulMainTpNum;
        telus_v2000h_nvramData.ulPsiSize = nvramData.ulPsiSize;
        telus_v2000h_nvramData.ulNumMacAddrs = nvramData.ulNumMacAddrs;
        memcpy(&telus_v2000h_nvramData.ucaBaseMacAddr[0],&nvramData.ucaBaseMacAddr[0],NVRAM_MAC_ADDRESS_LEN);

        telus_v2000h_nvramData.pad = nvramData.pad;
        telus_v2000h_nvramData.backupPsi = nvramData.backupPsi;
        telus_v2000h_nvramData.ulCheckSumV4 = nvramData.ulCheckSumV4;

        memcpy(&telus_v2000h_nvramData.gponSerialNumber[0],&nvramData.gponSerialNumber[0],NVRAM_GPON_SERIAL_NUMBER_LEN);
        memcpy(&telus_v2000h_nvramData.gponPassword[0],&nvramData.gponPassword[0],NVRAM_GPON_PASSWORD_LEN);
        memcpy(&telus_v2000h_nvramData.wpsDevicePin[0],&nvramData.wpsDevicePin[0],NVRAM_WPS_DEVICE_PIN_LEN);
        memcpy(&telus_v2000h_nvramData.wlanParams[0],&nvramData.wlanParams[0],NVRAM_WLAN_PARAMS_LEN);

        telus_v2000h_nvramData.ulSyslogSize = nvramData.ulSyslogSize;

        memcpy(&telus_v2000h_nvramData.ulNandPartOfsKb[0], &nvramData.ulNandPartOfsKb[0], sizeof(nvramData.ulNandPartOfsKb));
        memcpy(&telus_v2000h_nvramData.ulNandPartSizeKb[0], &nvramData.ulNandPartSizeKb[0], sizeof(nvramData.ulNandPartSizeKb));
        memcpy(&telus_v2000h_nvramData.szVoiceBoardId[0], &nvramData.szVoiceBoardId[0], sizeof(nvramData.szVoiceBoardId));

        memcpy(&telus_v2000h_nvramData.afeId[0], &nvramData.afeId[0], sizeof(nvramData.afeId));

        memcpy(&telus_v2000h_nvramData.ulSerialNumber[0],&nvramData.ulSerialNumber[0],32);
        memcpy(&telus_v2000h_nvramData.chFactoryFWVersion[0],&nvramData.chFactoryFWVersion[0],48);
        memcpy(&telus_v2000h_nvramData.wpsPin[0],&nvramData.wpsPin[0],32);
        memcpy(&telus_v2000h_nvramData.wpaKey[0],&nvramData.wpaKey[0],32);
        telus_v2000h_nvramData.dslDatapump=nvramData.dslDatapump;

        telus_v2000h_nvramData.ulVersion=NVRAM_VERSION_NUMBER;

        memset((char *)&telus_v2000h_nvramData + ((size_t) &((NVRAM_DATA *)0)->ulSerialNumber), 0, 1);
        memset((char *)&telus_v2000h_nvramData + ((size_t) &((NVRAM_DATA *)0)->chFactoryFWVersion), 0, 1);

        telus_v2000h_nvramData.ulCheckSum = getCrc32((unsigned char *)&telus_v2000h_nvramData, sizeof(TELUS_V2000H_NVRAM_DATA),CRC32_INIT_VALUE);

        writeNvramDataCrcLocked(&telus_v2000h_nvramData);
        return TRUE;
     }
     return FALSE;

}

#endif

/***************************************************************************
 * Function Name: kerSysGetChipId
 * Description  : Map id read from device hardware to id of chip family
 *                consistent with  BRCM_CHIP
 * Returns      : chip id of chip family
 ***************************************************************************/
int kerSysGetChipId() { 
        int r;
        r = (int) ((PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT);
        /* Force BCM681x variants to be be BCM6816) */
        if( (r & 0xfff0) == 0x6810 )
            r = 0x6816;

        /* Force BCM6369 to be BCM6368) */
        if( (r & 0xfffe) == 0x6368 )
            r = 0x6368;

        /* Force BCM63168, BCM63169, and BCM63269 to be BCM63268) */
        if( ( (r & 0xffffe) == 0x63168 )
          || ( (r & 0xffffe) == 0x63268 ))
            r = 0x63268;

        return(r);
}

/***************************************************************************
 * Function Name: kerSysGetDslPhyEnable
 * Description  : returns true if device should permit Phy to load
 * Returns      : true/false
 ***************************************************************************/
int kerSysGetDslPhyEnable() {
        int id;
        int r = 1;
        id = (int) ((PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT);
        if ((id == 0x63169) || (id == 0x63269)) {
	    r = 0;
        }
        return(r);
}

/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetChipId);
EXPORT_SYMBOL(kerSysGetMacAddressType);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetGponSerialNumber);
EXPORT_SYMBOL(kerSysGetGponPassword);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysGetDslPhyEnable);
#if defined(CONFIG_BCM96368)
EXPORT_SYMBOL(kerSysGetSdramWidth);
#endif
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysSendtoMonitorTask);
EXPORT_SYMBOL(kerSysGetWlanSromParams);
EXPORT_SYMBOL(kerSysGetAfeId);
EXPORT_SYMBOL(kerSysGetUbusFreq);
#if defined(CONFIG_BCM96816)
EXPORT_SYMBOL(kerSysBcmSpiSlaveRead);
EXPORT_SYMBOL(kerSysBcmSpiSlaveReadReg32);
EXPORT_SYMBOL(kerSysBcmSpiSlaveWrite);
EXPORT_SYMBOL(kerSysBcmSpiSlaveWriteReg32);
EXPORT_SYMBOL(kerSysBcmSpiSlaveWriteBuf);
#endif
EXPORT_SYMBOL(BpGetBoardId);
EXPORT_SYMBOL(BpGetBoardIds);
EXPORT_SYMBOL(BpGetGPIOverlays);
EXPORT_SYMBOL(BpGetFpgaResetGpio);
EXPORT_SYMBOL(BpGetEthernetMacInfo);
#if defined(CONFIG_BCM963268) && (CONFIG_BCM_EXT_SWITCH)
EXPORT_SYMBOL(BpGetPortConnectedToExtSwitch);
#endif
EXPORT_SYMBOL(BpGetRj11InnerOuterPairGpios);
EXPORT_SYMBOL(BpGetRtsCtsUartGpios);
EXPORT_SYMBOL(BpGetAdslLedGpio);
EXPORT_SYMBOL(BpGetAdslFailLedGpio);
EXPORT_SYMBOL(BpGetWanDataLedGpio);
EXPORT_SYMBOL(BpGetWanErrorLedGpio);
EXPORT_SYMBOL(BpGetVoipLedGpio);
EXPORT_SYMBOL(BpGetPotsLedGpio);
EXPORT_SYMBOL(BpGetVoip2FailLedGpio);
EXPORT_SYMBOL(BpGetVoip2LedGpio);
EXPORT_SYMBOL(BpGetVoip1FailLedGpio);
EXPORT_SYMBOL(BpGetVoip1LedGpio);
EXPORT_SYMBOL(BpGetDectLedGpio);
EXPORT_SYMBOL(BpGetMoCALedGpio);
EXPORT_SYMBOL(BpGetMoCAFailLedGpio);
#if defined(AEI_VDSL_CUSTOMER_NCS)
EXPORT_SYMBOL(BpGetUsbLedGpio);
EXPORT_SYMBOL(BpGetBootloaderPowerOnLedGpio);
EXPORT_SYMBOL(BpGetBootloaderStopLedGpio);
EXPORT_SYMBOL(BpGetWirelessFailSesLedGpio);
#if defined(AEI_63168_CHIP)
EXPORT_SYMBOL(BpGetEnetWanLedGpio);
#endif
#endif
EXPORT_SYMBOL(BpGetWirelessSesExtIntr);
EXPORT_SYMBOL(BpGetWirelessSesLedGpio);
EXPORT_SYMBOL(BpGetWirelessFlags);
EXPORT_SYMBOL(BpGetWirelessPowerDownGpio);
EXPORT_SYMBOL(BpUpdateWirelessSromMap);
EXPORT_SYMBOL(BpGetSecAdslLedGpio);
EXPORT_SYMBOL(BpGetSecAdslFailLedGpio);
EXPORT_SYMBOL(BpGetDslPhyAfeIds);
EXPORT_SYMBOL(BpGetExtAFEResetGpio);
EXPORT_SYMBOL(BpGetExtAFELDPwrGpio);
EXPORT_SYMBOL(BpGetExtAFELDModeGpio);
EXPORT_SYMBOL(BpGetIntAFELDPwrGpio);
EXPORT_SYMBOL(BpGetIntAFELDModeGpio);
EXPORT_SYMBOL(BpGetAFELDRelayGpio);
EXPORT_SYMBOL(BpGetExtAFELDDataGpio);
EXPORT_SYMBOL(BpGetExtAFELDClkGpio);
EXPORT_SYMBOL(BpGetUart2SdoutGpio);
EXPORT_SYMBOL(BpGetUart2SdinGpio);
EXPORT_SYMBOL(BpGet6829PortInfo);
EXPORT_SYMBOL(BpGetEthSpdLedGpio);
EXPORT_SYMBOL(BpGetLaserDisGpio);
EXPORT_SYMBOL(BpGetLaserTxPwrEnGpio);
EXPORT_SYMBOL(BpGetVregSel1P2);
EXPORT_SYMBOL(BpGetGponOpticsType);
EXPORT_SYMBOL(BpGetDefaultOpticalParams);
EXPORT_SYMBOL(BpGetMiiOverGpioFlag);
#if defined (CONFIG_BCM_ENDPOINT_MODULE)
EXPORT_SYMBOL(BpGetVoiceBoardId);
EXPORT_SYMBOL(BpGetVoiceBoardIds);
EXPORT_SYMBOL(BpGetVoiceParms);
#endif
#if defined (CONFIG_BCM_AVS_PWRSAVE)
EXPORT_SYMBOL(kerSysBcmEnableAvs);
EXPORT_SYMBOL(kerSysBcmAvsEnabled);
#endif
#ifdef AEI_VDSL_CUSTOMER_NCS
EXPORT_SYMBOL(kerSysGetDslDatapump);
#endif
#ifdef AEI_TWO_IN_ONE_FIRMWARE
EXPORT_SYMBOL(kerSysGetBoardID);
#endif
