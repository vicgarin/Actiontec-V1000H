/*
    Copyright 2000-2010 Broadcom Corporation

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

        As a special exception, the copyright holders of this software give
        you permission to link this software with independent modules, and to
        copy and distribute the resulting executable under terms of your
        choice, provided that you also meet, for each linked independent
        module, the terms and conditions of the license of that module. 
        An independent module is a module which is not derived from this
        software.  The special exception does not apply to any modifications
        of the software.

    Notwithstanding the above, under no circumstances may you combine this
    software in any way with any other Broadcom software provided under a
    license other than the GPL, without Broadcom's express prior written
    consent.
*/                       

/** Includes. **/
#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"  
#define printk  printf
#else       // linux
#include <linux/version.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#include <linux/semaphore.h>
#endif
#include <linux/hardirq.h>

#include <bcm_map_part.h> 
#endif

#include "bcmtypes.h"
#include "bcm_hwdefs.h"
#include "flash_api.h"
#include "bcmSpiRes.h"


/** Defines. **/
#define OSL_DELAY(X)                        \
    do { { int i; for( i = 0; i < (X) * 500; i++ ) ; } } while(0)

#define MAX_RETRY           3

#ifndef NULL
#define NULL 0
#endif

#define MAXSECTORS          8192    /* maximum number of sectors supported */

#define FLASH_PAGE_256      256
#define SECTOR_SIZE_4K      (4 * 1024)
#define SECTOR_SIZE_64K     (64 * 1024)

/* Standard Boolean declarations */
#define TRUE                1
#define FALSE               0

/* Command codes for the flash_command routine */
#define FLASH_WRST          0x01    /* write status register */
#define FLASH_PROG          0x02    /* program data into memory array */
#define FLASH_READ          0x03    /* read data from memory array */
#define FLASH_WRDI          0x04    /* reset write enable latch */
#define FLASH_RDSR          0x05    /* read status register */
#define FLASH_WREN          0x06    /* set write enable latch */
#define FLASH_READ_FAST     0x0B    /* read data from memory array */
#define FLASH_SERASE        0x20    /* erase one sector in memory array */
#define FLASH_BERASE        0xD8    /* erase one block in memory array */
#define FLASH_RDID          0x9F    /* read manufacturer and product id */
#define FLASH_EN4B          0xB7    /* Enable 4 byte address mode */
#define FLASH_READ_DOR      0x3B    /* dual output read */
#define FLASH_READ_DIOR     0xBB    /* dual i/o read */

/* RDSR return status bit definition */
#define SR_WPEN             0x80
#define SR_BP3              0x20
#define SR_BP2              0x10
#define SR_BP1              0x08
#define SR_BP0              0x04
#define SR_WEN              0x02
#define SR_RDY              0x01

/* Return codes from flash_status */
#define STATUS_READY        0       /* ready for action */
#define STATUS_BUSY         1       /* operation in progress */
#define STATUS_TIMEOUT      2       /* operation timed out */
#define STATUS_ERROR        3       /* unclassified but unhappy status */

/* Define different type of flash */
#define FLASH_UNDEFINED     0
#define FLASH_SPAN          2

/* SST's manufacturer ID */
#define SSTPART             0xBF
/* A list of SST device IDs */
#define ID_SST25VF016       0x41
#define ID_SST25VF032       0x4A
#define ID_SST25VF064       0x4B

/* SPANSION manufacturer IDs */
#define SPANPART            0x01
/* SPANSION device ID's */
#define ID_SPAN25FL016      0x14
#define ID_SPAN25FL032      0x15
#define ID_SPAN25FL064      0x16
#define ID_SPAN25FL128      0x18

/* EON manufacturer ID */
#define EONPART             0x1C
/* NUMONYX manufacturer ID */
#define NUMONYXPART         0x20
/* AMIC manufacturer ID */
#define AMICPART            0x37
/* Macronix manufacturer ID */
#define MACRONIXPART        0xC2
/* Winbond's manufacturer ID */
#define WBPART              0xEF

/* JEDEC device IDs */
#define ID_M25P16           0x15
#define ID_M25P32           0x16
#define ID_M25P64           0x17
#define ID_M25P128          0x18
#define ID_M25P256          0x19

#define SPI_MAKE_ID(A,B)    \
    (((unsigned short) (A) << 8) | ((unsigned short) B & 0xff))

#define SPI_FLASH_DEVICES                                   \
    {{SPI_MAKE_ID(SSTPART, ID_SST25VF016), "SST25VF016"},   \
     {SPI_MAKE_ID(SSTPART, ID_SST25VF032), "SST25VF032"},   \
     {SPI_MAKE_ID(SSTPART, ID_SST25VF064), "SST25VF064"},   \
     {SPI_MAKE_ID(SPANPART, ID_SPAN25FL016), "S25FL016"},   \
     {SPI_MAKE_ID(SPANPART, ID_SPAN25FL032), "S25FL032"},   \
     {SPI_MAKE_ID(SPANPART, ID_SPAN25FL064), "S25FL064"},   \
     {SPI_MAKE_ID(SPANPART, ID_SPAN25FL128), "S25FL128"},   \
     {SPI_MAKE_ID(WBPART, ID_M25P16), "ID_W25X16"},         \
     {SPI_MAKE_ID(WBPART, ID_M25P32), "ID_W25X32"},         \
     {SPI_MAKE_ID(WBPART, ID_M25P64), "ID_W25X64"},         \
     {SPI_MAKE_ID(WBPART, ID_M25P128), "ID_W25X128"},       \
     {SPI_MAKE_ID(EONPART, ID_M25P16), "EN25P16"},          \
     {SPI_MAKE_ID(EONPART, ID_M25P32), "EN25P32"},          \
     {SPI_MAKE_ID(EONPART, ID_M25P64), "EN25P64"},          \
     {SPI_MAKE_ID(EONPART, ID_M25P128), "EN25P128"},        \
     {SPI_MAKE_ID(AMICPART, ID_M25P16), "A25L016"},         \
     {SPI_MAKE_ID(AMICPART, ID_M25P32), "A25L032"},         \
     {SPI_MAKE_ID(NUMONYXPART, ID_M25P16), "NMNXM25P16"},   \
     {SPI_MAKE_ID(NUMONYXPART, ID_M25P32), "NMNXM25P32"},   \
     {SPI_MAKE_ID(NUMONYXPART, ID_M25P64), "NMNXM25P64"},   \
     {SPI_MAKE_ID(NUMONYXPART, ID_M25P128), "NMNXM25P128"}, \
     {SPI_MAKE_ID(MACRONIXPART, ID_M25P16), "MX25L16"},     \
     {SPI_MAKE_ID(MACRONIXPART, ID_M25P32), "MX25L32"},     \
     {SPI_MAKE_ID(MACRONIXPART, ID_M25P64), "MX25L64"},     \
     {SPI_MAKE_ID(MACRONIXPART, ID_M25P128), "MX25L128"},   \
     {SPI_MAKE_ID(MACRONIXPART, ID_M25P256), "MX25L256"},   \
     {0,""}                                                 \
    }

/** Structs. **/
/* A structure for identifying a flash part.  There is one for each
 * of the flash part definitions.  We need to keep track of the
 * sector organization, the address register used, and the size
 * of the sectors.
 */
struct flashinfo {
    char *name;         /* "AT25F512", etc. */
    unsigned long addr; /* physical address, once translated */
    int nsect;          /* # of sectors */
    struct {
        long size;      /* # of bytes in this sector */
        long base;      /* offset from beginning of device */
    } sec[MAXSECTORS];  /* per-sector info */
};

struct flash_name_from_id {
    unsigned short fnfi_id;
    char fnfi_name[30];
};


/** Prototypes. **/
static int my_spi_read( struct spi_transfer *xfer );
static int my_spi_write( unsigned char *msg_buf, int nbytes );

int spi_flash_init(flash_device_info_t **flash_info);
static int spi_flash_sector_erase_int(unsigned short sector);
static int spi_flash_reset(void);
static int spi_flash_read_buf(unsigned short sector, int offset,
    unsigned char *buffer, int nbytes);
static int spi_flash_ub(unsigned short sector);
static int spi_flash_write(unsigned short sector, int offset,
    unsigned char *buffer, int nbytes);
static int spi_flash_write_buf(unsigned short sector, int offset,
    unsigned char *buffer, int nbytes);
static int spi_flash_get_numsectors(void);
static int spi_flash_get_sector_size(unsigned short sector);
static unsigned char *spi_get_flash_memptr(unsigned short sector);
static unsigned char *spi_flash_get_memptr(unsigned short sector);
static int spi_flash_status(void);
static unsigned short spi_flash_get_device_id(unsigned short sector);
static int spi_flash_get_blk(int addr);
static int spi_flash_get_total_size(void);
static int spi_flash_en4b(void);
static void spi_flash_multibit_en(void);

/** Variables. **/
static flash_device_info_t flash_spi_dev =
    {
        0xffff,
        FLASH_IFC_SPI,
        "",
        spi_flash_sector_erase_int,
        spi_flash_read_buf,
        spi_flash_write_buf,
        spi_flash_get_numsectors,
        spi_flash_get_sector_size,
        spi_flash_get_memptr,
        spi_flash_get_blk,
#ifdef AEI_NAND_IMG_CHECK
        spi_flash_get_total_size,
        NULL,
        NULL
#else
        spi_flash_get_total_size
#endif
    };

static struct flash_name_from_id fnfi[] = SPI_FLASH_DEVICES;

/* the controller will handle operati0ns that are greater than the FIFO size
   code that relies on READ_BUF_LEN_MAX, READ_BUF_LEN_MIN or spi_max_op_len
   could be changed */
#define READ_BUF_LEN_MAX   544    /* largest of the maximum transaction sizes for SPI */
#define READ_BUF_LEN_MIN   60     /* smallest of the maximum transaction sizes for SPI */
/* this is the slave ID of the SPI flash for use with the SPI controller */
#define SPI_FLASH_SLAVE_DEV_ID    0
/* clock defines for the flash */
#define SPI_FLASH_DEF_CLOCK       781000

/* default to smallest transaction size - updated later */
static unsigned int spi_max_op_len = READ_BUF_LEN_MIN; 
static int spi_read_cmd            = FLASH_READ_FAST;
static int spi_dummy_bytes         = 1;
static int spi_multibit_en         = 0;
static int flash_page_size         = FLASH_PAGE_256;

/* default to legacy controller - updated later */
static int spi_flash_clock  = SPI_FLASH_DEF_CLOCK;
static int spi_flash_busnum = LEG_SPI_BUS_NUM;

#ifndef _CFE_
static DECLARE_MUTEX(spi_flash_lock);
static bool bSpiFlashSlaveRes = FALSE;
#endif

static struct flashinfo meminfo; /* Flash information structure */
static int totalSize = 0;
static int addr32 = FALSE;

static int my_spi_read(struct spi_transfer *xfer)
{
    int status;

#ifndef _CFE_
    if ( FALSE == bSpiFlashSlaveRes )
#endif
    {
        status = BcmSpi_MultibitRead(xfer, spi_flash_busnum, SPI_FLASH_SLAVE_DEV_ID);
    }
#ifndef _CFE_
    else
    {
        status = BcmSpiSyncMultTrans(xfer, 1, spi_flash_busnum, SPI_FLASH_SLAVE_DEV_ID);
    }
#endif

    return status;
}

static int my_spi_write(unsigned char *msg_buf, int nbytes)
{
    int status; 

#ifndef _CFE_
    if ( FALSE == bSpiFlashSlaveRes )
#endif
    {
        status = BcmSpi_Write(msg_buf, nbytes, spi_flash_busnum, 
                              SPI_FLASH_SLAVE_DEV_ID, spi_flash_clock);
    }
#ifndef _CFE_
    else
    {
        status = BcmSpiSyncTrans(msg_buf, NULL, 0, nbytes, spi_flash_busnum,
                                 SPI_FLASH_SLAVE_DEV_ID);
    }
#endif
    return status;
}


/*********************************************************************/
/* Init_flash is used to build a sector table. This information is   */
/* translated from erase_block information to base:offset information*/
/* for each individual sector. This information is then stored       */
/* in the meminfo structure, and used throughout the driver to access*/
/* sector information.                                               */
/*                                                                   */
/* This is more efficient than deriving the sector base:offset       */
/* information every time the memory map switches (since on the      */
/* development platform can only map 64k at a time).  If the entire  */
/* flash memory array can be mapped in, then the addition static     */
/* allocation for the meminfo structure can be eliminated, but the   */
/* drivers will have to be re-written.                               */
/*                                                                   */
/* The meminfo struct occupies 44 bytes of heap space, depending     */
/* on the value of the define MAXSECTORS.  Adjust to suit            */
/* application                                                       */ 
/*********************************************************************/

int spi_flash_init(flash_device_info_t **flash_info)
{
    struct flash_name_from_id *fnfi_ptr;
    int i=0, count=0;
    int basecount=0L;
    unsigned short device_id;
    int sectorsize = 0;
    int numsector = 0;
    int spiCtrlState;

#if defined(_BCM96816_) || defined(CONFIG_BCM96816)
    uint32 miscStrapBus = MISC->miscStrapBus;

    if ( miscStrapBus & MISC_STRAP_BUS_LS_SPIM_ENABLED )
    {
        spi_flash_busnum = LEG_SPI_BUS_NUM;
        if ( miscStrapBus & MISC_STRAP_BUS_SPI_CLK_FAST )
        {
            spi_flash_clock = 20000000;
        }
        else
        {
            spi_flash_clock = 781000;
        }
    }
    else
    {
        spi_flash_busnum = HS_SPI_BUS_NUM;
        if ( miscStrapBus & MISC_STRAP_BUS_SPI_CLK_FAST )
        {
            spi_flash_clock = 20000000;
        }
        else
        {
            spi_flash_clock = 66666667;
        }
    }
#endif
#if defined(_BCM96328_) || defined(CONFIG_BCM96328)
    uint32 miscStrapBus = MISC->miscStrapBus;

    spi_flash_busnum = HS_SPI_BUS_NUM;
    if ( miscStrapBus & MISC_STRAP_BUS_HS_SPIM_FAST_B_MASK )
        spi_flash_clock = 33333334;
    else
        spi_flash_clock = 16666667;
#endif
#if defined(_BCM96362_) || defined(CONFIG_BCM96362) || defined(_BCM963268_) || defined(CONFIG_BCM963268)
    uint32 miscStrapBus = MISC->miscStrapBus;

    spi_flash_busnum = HS_SPI_BUS_NUM;
    if ( miscStrapBus & MISC_STRAP_BUS_HS_SPIM_CLK_SLOW_N_FAST )
        spi_flash_clock = 50000000;
    else
        spi_flash_clock = 20000000;
#endif
#if defined(_BCM96368_) || defined(CONFIG_BCM96368)
    uint32 miscStrapBus = GPIO->StrapBus;
 
    if ( miscStrapBus & MISC_STRAP_BUS_SPI_CLK_FAST )
       spi_flash_clock = 20000000;
    else
       spi_flash_clock = 781000;
#endif

    /* retrieve the maximum read/write transaction length from the SPI controller */
    spi_max_op_len = BcmSpi_GetMaxRWSize( spi_flash_busnum, 1 );

    /* set the controller state, spi_mode_0 */
    spiCtrlState = SPI_CONTROLLER_STATE_DEFAULT;
    if ( spi_flash_clock > SPI_CONTROLLER_MAX_SYNC_CLOCK )
       spiCtrlState |= SPI_CONTROLLER_STATE_ASYNC_CLOCK;
    BcmSpi_SetCtrlState(spi_flash_busnum, SPI_FLASH_SLAVE_DEV_ID, SPI_MODE_DEFAULT, spiCtrlState);

    if (HS_SPI_BUS_NUM == spi_flash_busnum)
        flash_spi_dev.flash_type = FLASH_IFC_HS_SPI;

    *flash_info = &flash_spi_dev;
#if 0
    /* 
     * in case of flash corrupt, the following steps can erase the flash
     * 1. jumper USE_SPI_SLAVE to make SPI in slave mode
     * 2. start up JTAG debuger and remove the USE_SPI_SLAVE jumper 
     * 3. run the following code to erase the flash
     */
    flash_sector_erase_int(0);
    flash_sector_erase_int(1);
    printk("flash_init: erase all sectors\n");
    return FLASH_API_OK;
#endif

    flash_spi_dev.flash_device_id = device_id = spi_flash_get_device_id(0);

    switch( device_id >> 8 ) {
        case SSTPART:
            sectorsize = SECTOR_SIZE_4K;
            switch ((unsigned char)(device_id & 0x00ff)) {
                case ID_SST25VF016:
                    numsector = 512;
                    break;
                case ID_SST25VF032:
                    numsector = 1024;
                    break;
                case ID_SST25VF064:
                    numsector = 2048;
                    break;
            }
            break;

        case SPANPART:
            sectorsize = SECTOR_SIZE_64K;
            switch ((unsigned short)(device_id & 0x00ff)) {
                case ID_SPAN25FL016:
                    numsector = 32;
                    break;
                case ID_SPAN25FL032:
                    numsector = 64;
                    break;
                case ID_SPAN25FL064:
                    numsector = 128;
                    break;
                case ID_SPAN25FL128:
                    numsector = 256;
                    break;
            }
            break;

        case EONPART:
            sectorsize = SECTOR_SIZE_64K;
            switch ((unsigned short)(device_id & 0x00ff)) {
                case ID_M25P16:
                    numsector = 32;
                    break;
                case ID_M25P32:
                    numsector = 64;
                    break;
                case ID_M25P64:
                    numsector = 128;
                    break;
                case ID_M25P128:
                    numsector = 256;
                    break;
            }
            break;

        case NUMONYXPART:
        case MACRONIXPART:
        case WBPART:
        case AMICPART:
            sectorsize = SECTOR_SIZE_4K;
            switch ((unsigned short)(device_id & 0x00ff)) {
                case ID_M25P16:
                    numsector = 512;
                    break;
                case ID_M25P32:
                    numsector = 1024;
                    break;
                case ID_M25P64:
                    numsector = 2048;
                    break;
                case ID_M25P128:
                    numsector = 4096;
                    break;
                case ID_M25P256:
                    addr32 = TRUE;
                    numsector = 8192;
                    break;
            }
            break;

        default:
            meminfo.addr = 0L;
            meminfo.nsect = 1;
            meminfo.sec[0].size = SECTOR_SIZE_4K;
            meminfo.sec[0].base = 0x00000;
            return FLASH_API_ERROR;
    }

    if ( addr32 ) {
        /* Enable 4 byte mode */
        spi_flash_en4b();
    }

    meminfo.addr = 0L;
    meminfo.nsect = numsector;
    for (i = 0; i < numsector; i++) {
        meminfo.sec[i].size = sectorsize;
        meminfo.sec[i].base = basecount;
        basecount += meminfo.sec[i].size;
        count++;
    }
    totalSize = meminfo.sec[count-1].base + meminfo.sec[count-1].size;

    for( fnfi_ptr = fnfi; fnfi_ptr->fnfi_id != 0; fnfi_ptr++ ) {
        if( fnfi_ptr->fnfi_id == device_id ) {
            strcpy( flash_spi_dev.flash_device_name, fnfi_ptr->fnfi_name ); 
            break;
        }
    }

    /* check to see if multibit mode is supported */
    switch( device_id >> 8 ) {
        case SPANPART:
            if ( HS_SPI_BUS_NUM == spi_flash_busnum )
            {
               spi_flash_multibit_en();
            }
            break;

        default:
            break;
    }

    BcmSpi_SetFlashCtrl(spi_read_cmd, 1, spi_dummy_bytes, spi_flash_busnum, SPI_FLASH_SLAVE_DEV_ID, spi_flash_clock, spi_multibit_en);

    return (FLASH_API_OK);
}

/*********************************************************************/
/* Flash_sector_erase_int() wait until the erase is completed before */
/* returning control to the calling function.  This can be used in   */
/* cases which require the program to hold until a sector is erased, */
/* without adding the wait check external to this function.          */
/*********************************************************************/

static int spi_flash_sector_erase_int(unsigned short sector)
{
    unsigned char buf[6];
    unsigned int cmd_length;
    unsigned int addr;

#ifndef _CFE_
    down(&spi_flash_lock);
#endif

    /* set device to write enabled */
    spi_flash_ub(sector);

    /* erase the sector  */
    addr = (unsigned int) spi_get_flash_memptr(sector);

    cmd_length = 0;
    if (meminfo.sec[sector].size == SECTOR_SIZE_4K)
        buf[cmd_length++] = FLASH_SERASE;
    else
        buf[cmd_length++] = FLASH_BERASE;

    if ( addr32 )
        buf[cmd_length++] = (unsigned char)((addr & 0xff000000) >> 24);
    buf[cmd_length++] = (unsigned char)((addr & 0x00ff0000) >> 16);
    buf[cmd_length++] = (unsigned char)((addr & 0x0000ff00) >> 8);
    buf[cmd_length++] = (unsigned char)(addr & 0x000000ff);

    /* check device is ready */
    if (my_spi_write(buf, cmd_length) == SPI_STATUS_OK) {
        while (spi_flash_status() != STATUS_READY);
    }

    spi_flash_reset();

#ifndef _CFE_
    up(&spi_flash_lock);
#endif

    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_reset() will reset the flash device to reading array data.  */
/* It is good practice to call this function after autoselect        */
/* sequences had been performed.                                     */
/*********************************************************************/

static int spi_flash_en4b(void)
{
    unsigned char buf[4];

    /* set device to write disabled */
    buf[0] = FLASH_EN4B;
    my_spi_write(buf, 1);
    while (spi_flash_status() != STATUS_READY);

    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_reset() will reset the flash device to reading array data.  */
/* It is good practice to call this function after autoselect        */
/* sequences had been performed.                                     */
/*********************************************************************/

static int spi_flash_reset(void)
{
    unsigned char buf[4];

    /* set device to write disabled */
    buf[0]        = FLASH_WRDI;
    my_spi_write(buf, 1);
    while (spi_flash_status() != STATUS_READY);

    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_read_buf() reads buffer of data from the specified          */
/* offset from the sector parameter.                                 */
/*********************************************************************/

static int spi_flash_read_buf(unsigned short sector, int offset,
    unsigned char *buffer, int nbytes)
{
    unsigned char buf[READ_BUF_LEN_MAX];
    unsigned int cmd_length;
    unsigned int addr;
    int maxread;
    int multiOffset = 0;
    struct spi_transfer xfer;

    memset(&xfer, 0, sizeof(struct spi_transfer));
#ifndef _CFE_
    down(&spi_flash_lock);
#endif

    addr = (unsigned int) spi_get_flash_memptr(sector);
    addr += offset;

    while (nbytes > 0) {
        maxread = (nbytes < spi_max_op_len) ? nbytes : spi_max_op_len;

        cmd_length = 0;
        buf[cmd_length++] = spi_read_cmd;
        if ( addr32 )
            buf[cmd_length++] = (unsigned char)((addr & 0xff000000) >> 24);
        buf[cmd_length++] = (unsigned char)((addr & 0x00ff0000) >> 16);
        buf[cmd_length++] = (unsigned char)((addr & 0x0000ff00) >> 8);
        buf[cmd_length++] = (unsigned char)(addr & 0x000000ff);
        if (spi_dummy_bytes)
            buf[cmd_length++] = (unsigned char)0xff;

        if (spi_multibit_en)
        {
            if (spi_read_cmd == FLASH_READ_DOR)
               multiOffset = cmd_length; /* only read data is multibit */
            else
               multiOffset = 1; /* addr and data is multibit */
        }

        xfer.tx_buf                 = buf;
        xfer.rx_buf                 = buffer;
        xfer.len                    = maxread;
        xfer.speed_hz               = spi_flash_clock;
        xfer.prepend_cnt            = cmd_length;
        xfer.multi_bit_en           = spi_multibit_en;
        xfer.multi_bit_start_offset = multiOffset;
        xfer.addr_len               = (addr32 ? 4 : 3);
        xfer.addr_offset            = 1;
        xfer.hdr_len                = cmd_length;
        xfer.unit_size              = 1;
        my_spi_read(&xfer);

        while (spi_flash_status() != STATUS_READY);

        buffer += maxread;
        nbytes -= maxread;
        addr += maxread;
    }

#ifndef _CFE_
    up(&spi_flash_lock);
#endif

    return (FLASH_API_OK);
}

/*********************************************************************/
/* flash_ub() places the flash into unlock bypass mode.  This        */
/* is REQUIRED to be called before any of the other unlock bypass    */
/* commands will become valid (most will be ignored without first    */
/* calling this function.                                            */
/*********************************************************************/

static int spi_flash_ub(unsigned short sector)
{
    unsigned char buf[4];
    struct spi_transfer xfer;

    do {
        buf[0]           = FLASH_RDSR;
        memset(&xfer, 0, sizeof(struct spi_transfer));
        xfer.tx_buf      = buf;
        xfer.rx_buf      = buf;
        xfer.len         = 1;
        xfer.speed_hz    = spi_flash_clock;
        xfer.prepend_cnt = 1;
        if (my_spi_read(&xfer) == SPI_STATUS_OK) {
            while (spi_flash_status() != STATUS_READY);
            if (buf[0] & (SR_BP3|SR_BP2|SR_BP1|SR_BP0)) {
                /* Sector is write protected. Unprotect it */
                buf[0] = FLASH_WREN;
                if (my_spi_write(buf, 1) == SPI_STATUS_OK) {
                    buf[0] = FLASH_WRST;
                    buf[1] = 0;
                    if (my_spi_write(buf, 2) == SPI_STATUS_OK)
                        while (spi_flash_status() != STATUS_READY);
                }
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    } while (1);

    /* set device to write enabled */
    buf[0] = FLASH_WREN;

    /* check device is ready */
    if (my_spi_write(buf, 1) == SPI_STATUS_OK) {
        while (spi_flash_status() != STATUS_READY);
        do {
            buf[0]           = FLASH_RDSR;
            memset(&xfer, 0, sizeof(struct spi_transfer));
            xfer.tx_buf      = buf;
            xfer.rx_buf      = buf;
            xfer.len         = 1;
            xfer.speed_hz    = spi_flash_clock;
            xfer.prepend_cnt = 1;
            if (my_spi_read(&xfer) == SPI_STATUS_OK) {
                while (spi_flash_status() != STATUS_READY);
                if (buf[0] & SR_WEN) {
                    break;
                }
            } 
            else {
                break;
            }
        } while (1);
    }

    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_write_buf() utilizes                                        */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/

static int spi_flash_write(unsigned short sector, int offset,
    unsigned char *buffer, int nbytes)
{
    unsigned char buf[FLASH_PAGE_256 + 6];
    unsigned int cmd_length;
    unsigned int addr;
    int maxwrite;
    int pagelimit;
    int bytes_written = 0;

#ifndef _CFE_
    down(&spi_flash_lock);
#endif

    addr = (unsigned int) spi_get_flash_memptr(sector);
    addr += offset;

    while (nbytes > 0) {
        spi_flash_ub(sector); /* enable write */

        cmd_length = 0;
        buf[cmd_length++] = FLASH_PROG;
        if ( addr32 )
            buf[cmd_length++] = (unsigned char)((addr & 0xff000000) >> 24);
        buf[cmd_length++] = (unsigned char)((addr & 0x00ff0000) >> 16);
        buf[cmd_length++] = (unsigned char)((addr & 0x0000ff00) >> 8);
        buf[cmd_length++] = (unsigned char)(addr & 0x000000ff);

        /* set length to the smaller of controller limit (spi_max_op_len) or nbytes
           spi_max_op_len considers both controllers */
        maxwrite = (nbytes < (spi_max_op_len - cmd_length)) ? nbytes : (spi_max_op_len - cmd_length);
        /* maxwrite is limit to page boundary */
        pagelimit = flash_page_size - (addr & (flash_page_size - 1));
        maxwrite = (maxwrite < pagelimit) ? maxwrite : pagelimit;

        memcpy(&buf[cmd_length], buffer, maxwrite);
        my_spi_write(buf, maxwrite + cmd_length);

        while (spi_flash_status() != STATUS_READY);

        buffer += maxwrite;
        nbytes -= maxwrite;
        addr += maxwrite;
        bytes_written += maxwrite;
    }

    spi_flash_reset();

#ifndef _CFE_
    up(&spi_flash_lock);
#endif

    return( bytes_written );
}

/*********************************************************************/
/* flash_write_buf() utilizes                                        */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/
static int spi_flash_write_buf(unsigned short sector, int offset,
    unsigned char *buffer, int nbytes)
{
    int ret;

    ret = spi_flash_write(sector, offset, buffer, nbytes);

    if( ret == FLASH_API_ERROR )
        printk( "Flash write error. Verify failed\n" );

    return( ret );
}

/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/

static int spi_flash_get_numsectors(void)
{
    return meminfo.nsect;
}

/*********************************************************************/
/* flash_get_sector_size() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/

static int spi_flash_get_sector_size(unsigned short sector)
{
    return meminfo.sec[sector].size;
}

/*********************************************************************/
/* The purpose of get_flash_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer.                                  */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/

static unsigned char *spi_get_flash_memptr(unsigned short sector)
{
    unsigned char *memptr = (unsigned char*)
        (FLASH_BASE + meminfo.sec[sector].base);

    return (memptr);
}

static unsigned char *spi_flash_get_memptr(unsigned short sector)
{
    return( spi_get_flash_memptr(sector) );
}

/*********************************************************************/
/* Flash_status return an appropriate status code                    */
/*********************************************************************/

static int spi_flash_status(void)
{
    unsigned char buf[4];
    int retry = 10;
    struct spi_transfer xfer;

    /* check device is ready */
    memset(&xfer, 0, sizeof(struct spi_transfer));
    do {
        buf[0]           = FLASH_RDSR;
        xfer.tx_buf      = buf;
        xfer.rx_buf      = buf;
        xfer.len         = 1;
        xfer.speed_hz    = spi_flash_clock;
        xfer.prepend_cnt = 1;
        if (my_spi_read(&xfer) == SPI_STATUS_OK) {
            if (!(buf[0] & SR_RDY)) {
                return STATUS_READY;
            }
        } else {
            return STATUS_ERROR;
        }
        OSL_DELAY(10);
    } while (retry--);

    return STATUS_TIMEOUT;
}

/*********************************************************************/
/* flash_get_device_id() return the device id of the component.      */
/*********************************************************************/

static unsigned short spi_flash_get_device_id(unsigned short sector)
{
    unsigned char buf[4];
    struct spi_transfer xfer;

    memset(&xfer, 0, sizeof(struct spi_transfer));
    buf[0]           = FLASH_RDID;
    xfer.tx_buf      = buf;
    xfer.rx_buf      = buf;
    xfer.len         = 3;
    xfer.speed_hz    = spi_flash_clock;
    xfer.prepend_cnt = 1;
    my_spi_read(&xfer);
    while (spi_flash_status() != STATUS_READY);
    buf[1] = buf[2];

    /* return manufacturer code and device code */
    return( *(unsigned short *)&buf[0] );
}

/*********************************************************************/
/* The purpose of flash_get_blk() is to return a the block number */
/* for a given memory address.                                       */
/*********************************************************************/

static int spi_flash_get_blk(int addr)
{
    int blk_start, i;
    int last_blk = spi_flash_get_numsectors();
    int relative_addr = addr - (int) FLASH_BASE;

    for(blk_start=0, i=0; i < relative_addr && blk_start < last_blk; blk_start++)
        i += spi_flash_get_sector_size(blk_start);

    if( (unsigned int)i > (unsigned int)relative_addr ) {
        blk_start--;        // last blk, dec by 1
    } else {
        if( blk_start == last_blk )
        {
            printk("Address is too big.\n");
            blk_start = -1;
        }
    }

    return( blk_start );
}

/************************************************************************/
/* The purpose of flash_get_total_size() is to return the total size of */
/* the flash                                                            */
/************************************************************************/
static int spi_flash_get_total_size(void)
{
    return totalSize;
}

static void spi_flash_multibit_en( void )
{
   unsigned char       buf[16];
   unsigned char       bufCmp[16];
   unsigned int        cmd_length;
   unsigned int        addr;
   struct spi_transfer xfer;
   int                 i;

   memset(&xfer, 0, sizeof(struct spi_transfer));

#ifndef _CFE_
   down(&spi_flash_lock);
#endif

   /* read 16 bytes of data form the first sector of flash
      this will be used to compare to the data read using the DOR
      and DIOR commands */
   addr              = (unsigned int) spi_get_flash_memptr(NVRAM_SECTOR);
   cmd_length        = 0;
   buf[cmd_length++] = FLASH_READ_FAST;
   if ( addr32 )
      buf[cmd_length++] = (unsigned char)((addr & 0xff000000) >> 24);
   buf[cmd_length++] = (unsigned char)((addr & 0x00ff0000) >> 16);
   buf[cmd_length++] = (unsigned char)((addr & 0x0000ff00) >> 8);
   buf[cmd_length++] = (unsigned char)(addr & 0x000000ff);
   if ( spi_dummy_bytes ) 
      buf[cmd_length++] = (unsigned char)0xff;

   xfer.tx_buf      = buf;
   xfer.rx_buf      = buf;
   xfer.len         = 16;
   xfer.speed_hz    = spi_flash_clock;
   xfer.prepend_cnt = cmd_length;
   my_spi_read(&xfer);
   while (spi_flash_status() != STATUS_READY);

   /* if the data read above is all 1's then we cannot determine if multibit is
      supported by the flash so just return */
   for ( i = 0; i < 16; i++)
   {
      if ( buf[i] != 0xFF )
      {
         break;
      }
      if ( 15 == i )
      {
         return;
      }
   }

   /* try the DIOR instruction
      if the data matches the previously read data then it is supported */
   addr              = (unsigned int) spi_get_flash_memptr(NVRAM_SECTOR);
   cmd_length        = 0;
   bufCmp[cmd_length++] = FLASH_READ_DIOR;

   if ( addr32 )
      bufCmp[cmd_length++] = (unsigned char)((addr & 0xff000000) >> 24);
   
   bufCmp[cmd_length++] = (unsigned char)((addr & 0x00ff0000) >> 16);
   bufCmp[cmd_length++] = (unsigned char)((addr & 0x0000ff00) >> 8);
   bufCmp[cmd_length++] = (unsigned char)(addr & 0x000000ff);
   /* add dummy byte */
   bufCmp[cmd_length++] = (unsigned char)0xff;

   xfer.tx_buf                 = bufCmp;
   xfer.rx_buf                 = bufCmp;
   xfer.len                    = 16;
   xfer.speed_hz               = spi_flash_clock;
   xfer.prepend_cnt            = cmd_length;
   xfer.multi_bit_en           = 1;
   xfer.multi_bit_start_offset = 1;
   my_spi_read(&xfer);
   while (spi_flash_status() != STATUS_READY);

   if ( 0 == memcmp(buf, bufCmp, 16) )
   {
      /* DIOR command is supported */
      spi_read_cmd    = FLASH_READ_DIOR;
      spi_multibit_en = 1;
#ifndef _CFE_
      up(&spi_flash_lock);
#endif
      return;
   }

   /* try the DOR command
      if the data matches the previously read data then it is supported */
   addr              = (unsigned int) spi_get_flash_memptr(NVRAM_SECTOR);
   cmd_length        = 0;
   bufCmp[cmd_length++] = FLASH_READ_DOR;
   if ( addr32 )
      bufCmp[cmd_length++] = (unsigned char)((addr & 0xff000000) >> 24);
   bufCmp[cmd_length++] = (unsigned char)((addr & 0x00ff0000) >> 16);
   bufCmp[cmd_length++] = (unsigned char)((addr & 0x0000ff00) >> 8);
   bufCmp[cmd_length++] = (unsigned char)(addr & 0x000000ff);
   /* add dummy byte */
   bufCmp[cmd_length++] = (unsigned char)0xff;
   
   xfer.tx_buf                 = bufCmp;
   xfer.rx_buf                 = bufCmp;
   xfer.len                    = 16;
   xfer.speed_hz               = spi_flash_clock;
   xfer.prepend_cnt            = cmd_length;
   xfer.multi_bit_en           = 1;
   xfer.multi_bit_start_offset = cmd_length;
   my_spi_read(&xfer);
   while (spi_flash_status() != STATUS_READY);

   if ( 0 == memcmp(buf, bufCmp, 16) )
   {
      /* DIOR command is supported */
      spi_read_cmd    = FLASH_READ_DOR;
      spi_multibit_en = 1;
#ifndef _CFE_
      up(&spi_flash_lock);
#endif
      return;
   }

#ifndef _CFE_
    up(&spi_flash_lock);
#endif

}

#ifndef _CFE_
static int __init BcmSpiflash_init(void)
{
    int flashType;
    int spiCtrlState;

    /* If serial flash is present then register the device. Otherwise do nothing */
    flashType  = flash_get_flash_type();
    if ((FLASH_IFC_SPI == flashType) || (FLASH_IFC_HS_SPI == flashType))
    {
        /* register the device */
        spiCtrlState = SPI_CONTROLLER_STATE_DEFAULT;
        if ( spi_flash_clock > SPI_CONTROLLER_MAX_SYNC_CLOCK )
           spiCtrlState |= SPI_CONTROLLER_STATE_ASYNC_CLOCK;
        
        BcmSpiReserveSlave2(spi_flash_busnum, SPI_FLASH_SLAVE_DEV_ID, spi_flash_clock, 
                            SPI_MODE_DEFAULT, spiCtrlState);
        bSpiFlashSlaveRes = TRUE;
        spi_max_op_len    = BcmSpi_GetMaxRWSize( spi_flash_busnum, 1 );
    }

    return 0;
}
module_init(BcmSpiflash_init);

static void __exit BcmSpiflash_exit(void)
{
    bSpiFlashSlaveRes = FALSE;
}
module_exit(BcmSpiflash_exit);
#endif

