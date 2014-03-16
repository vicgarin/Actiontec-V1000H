
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
/*
 ***************************************************************************
 * File Name  : bcm63xx_flash.c
 *
 * Description: This file contains the flash device driver APIs for bcm63xx board. 
 *
 * Created on :  8/10/2002  seanl:  use cfiflash.c, cfliflash.h (AMD specific)
 *
 ***************************************************************************/


/* Includes. */
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/preempt.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/jffs2.h>
#include <linux/mount.h>
#include <linux/crc32.h>
#include <linux/sched.h>
#include <linux/bcm_assert_locks.h>
#include <asm/uaccess.h>
#include <asm/delay.h>

#include <bcm_map_part.h>
#include <board.h>
#include <bcmTag.h>
#include "flash_api.h"
#include "boardparms.h"
#include "boardparms_voice.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#include <linux/fs_struct.h>
#endif
//#define DEBUG_FLASH

/* Only NAND flash build configures and uses CONFIG_CRC32. */
#if !defined(CONFIG_CRC32)
#undef crc32
#define crc32(a,b,c) 0
#endif

#ifdef AEI_NAND_IMG_CHECK
extern int gSetWrongCRC; //1=set wrong crc
#endif

#ifdef AEI_VDSL_CUSTOMER_NCS
//#define BCMTAG_EXE_USE
static unsigned long Crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
static UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}
#endif
extern int kerSysGetSequenceNumber(int);
extern PFILE_TAG kerSysUpdateTagSequenceNumber(int);

/*
 * inMemNvramData an in-memory copy of the nvram data that is in the flash.
 * This in-memory copy is used by NAND.  It is also used by NOR flash code
 * because it does not require a mutex or calls to memory allocation functions
 * which may sleep.  It is kept in sync with the flash copy by
 * updateInMemNvramData.
 */
static unsigned char *inMemNvramData_buf;
static NVRAM_DATA inMemNvramData;
static DEFINE_SPINLOCK(inMemNvramData_spinlock);
static void updateInMemNvramData(const unsigned char *data, int len, int offset);
#define UNINITIALIZED_FLASH_DATA_CHAR  0xff
static FLASH_ADDR_INFO fInfo;
static struct semaphore semflash;
#ifdef AEI_VDSL_CUSTOMER_NCS
static struct semaphore rd_semflash;
#endif

// mutex is preferred over semaphore to provide simple mutual exclusion
// spMutex protects scratch pad writes
static DEFINE_MUTEX(spMutex);
extern struct mutex flashImageMutex;
static char bootCfeVersion[CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE];
static int bootFromNand = 0;

static int setScratchPad(char *buf, int len);
static char *getScratchPad(int len);
static int nandNvramSet(const char *nvramString );

#ifdef AEI_CONFIG_JFFS
static int nandEraseBlkNotSpare( struct mtd_info *mtd, int blk );
#endif


#define ALLOC_TYPE_KMALLOC   0
#define ALLOC_TYPE_VMALLOC   1

static void *retriedKmalloc(size_t size)
{
    void *pBuf;
    unsigned char *bufp8 ;

    size += 4 ; /* 4 bytes are used to store the housekeeping information used for freeing */

    // Memory allocation changed from kmalloc() to vmalloc() as the latter is not susceptible to memory fragmentation under low memory conditions
    // We have modified Linux VM to search all pages by default, it is no longer necessary to retry here
    if (!in_interrupt() ) {
        pBuf = vmalloc(size);
        if (pBuf) {
            memset(pBuf, 0, size);
            bufp8 = (unsigned char *) pBuf ;
            *bufp8 = ALLOC_TYPE_VMALLOC ;
            pBuf = bufp8 + 4 ;
        }
    }
    else { // kmalloc is still needed if in interrupt
        printk("retriedKmalloc: someone calling from intrrupt context?!");
#ifndef AEI_VDSL_CUSTOMER_NCS
        BUG();
#endif
        pBuf = kmalloc(size, GFP_ATOMIC);
        if (pBuf) {
            memset(pBuf, 0, size);
            bufp8 = (unsigned char *) pBuf ;
            *bufp8 = ALLOC_TYPE_KMALLOC ;
            pBuf = bufp8 + 4 ;
        }
    }

    return pBuf;
}

static void retriedKfree(void *pBuf)
{
    unsigned char *bufp8  = (unsigned char *) pBuf ;
    bufp8 -= 4 ;

    if (*bufp8 == ALLOC_TYPE_KMALLOC)
        kfree(bufp8);
    else
        vfree(bufp8);
}

// get shared blks into *** pTempBuf *** which has to be released bye the caller!
// return: if pTempBuf != NULL, poits to the data with the dataSize of the buffer
// !NULL -- ok
// NULL  -- fail
static char *getSharedBlks(int start_blk, int num_blks)
{
    int i = 0;
    int usedBlkSize = 0;
    int sect_size = 0;
    char *pTempBuf = NULL;
    char *pBuf = NULL;

    down(&semflash);

    for (i = start_blk; i < (start_blk + num_blks); i++)
        usedBlkSize += flash_get_sector_size((unsigned short) i);

    if ((pTempBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
    {
        printk("failed to allocate memory with size: %d\n", usedBlkSize);
        up(&semflash);
        return pTempBuf;
    }
    
    pBuf = pTempBuf;
    for (i = start_blk; i < (start_blk + num_blks); i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);

#if defined(DEBUG_FLASH)
        printk("getSharedBlks: blk=%d, sect_size=%d\n", i, sect_size);
#endif
        flash_read_buf((unsigned short)i, 0, pBuf, sect_size);
        pBuf += sect_size;
    }
    up(&semflash);
    
    return pTempBuf;
}

// Set the pTempBuf to flash from start_blk for num_blks
// return:
// 0 -- ok
// -1 -- fail
static int setSharedBlks(int start_blk, int num_blks, char *pTempBuf)
{
    int i = 0;
    int sect_size = 0;
    int sts = 0;
    char *pBuf = pTempBuf;

    down(&semflash);

    for (i = start_blk; i < (start_blk + num_blks); i++)
    {
        sect_size = flash_get_sector_size((unsigned short) i);
        flash_sector_erase_int(i);

        if (flash_write_buf(i, 0, pBuf, sect_size) != sect_size)
        {
            printk("Error writing flash sector %d.", i);
            sts = -1;
            break;
        }

#if defined(DEBUG_FLASH)
        printk("setShareBlks: blk=%d, sect_size=%d\n", i, sect_size);
#endif

        pBuf += sect_size;
    }

    up(&semflash);

    return sts;
}
#if !defined(CONFIG_BRCM_IKOS)
// Initialize the flash and fill out the fInfo structure
void kerSysEarlyFlashInit( void )
{
#ifdef AEI_VDSL_CUSTOMER_NCS
   unsigned long flags;
   NVRAM_DATA NvramDataTmp;
   unsigned char *pStrTmp=(unsigned char *)&NvramDataTmp;
#endif
#ifdef CONFIG_BCM_ASSERTS
    // ASSERTS and bug() may be too unfriendly this early in the bootup
    // sequence, so just check manually
    if (sizeof(NVRAM_DATA) != NVRAM_LENGTH)
        printk("kerSysEarlyFlashInit: nvram size mismatch! "
               "NVRAM_LENGTH=%d sizeof(NVRAM_DATA)=%d\n",
               NVRAM_LENGTH, sizeof(NVRAM_DATA));
#endif
    inMemNvramData_buf = (unsigned char *) &inMemNvramData;
    memset(inMemNvramData_buf, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);

    flash_init();
    bootFromNand = 0;

#if defined(CONFIG_BCM96368)
    if( ((GPIO->StrapBus & MISC_STRAP_BUS_BOOT_SEL_MASK) >>
        MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_BUS_BOOT_NAND)
        bootFromNand = 1;
#elif defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM96816) || defined(CONFIG_BCM963268)
    if( ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL_MASK) >>
        MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_BUS_BOOT_NAND )
        bootFromNand = 1;
#endif

#if defined(CONFIG_BCM96368) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM96816) || defined(CONFIG_BCM963268)
    if( bootFromNand == 1 )
    {
        unsigned long bootCfgSave =  NAND->NandNandBootConfig;
        NAND->NandNandBootConfig = NBC_AUTO_DEV_ID_CFG | 0x101;
        NAND->NandCsNandXor = 1;

        memcpy((unsigned char *)&bootCfeVersion, (unsigned char *)
            FLASH_BASE + CFE_VERSION_OFFSET, sizeof(bootCfeVersion));
        memcpy(inMemNvramData_buf, (unsigned char *)
            FLASH_BASE + NVRAM_DATA_OFFSET, sizeof(NVRAM_DATA));

        NAND->NandNandBootConfig = bootCfgSave;
        NAND->NandCsNandXor = 0;
    }
    else
#endif
    {
        fInfo.flash_rootfs_start_offset = flash_get_sector_size(0);
        if( fInfo.flash_rootfs_start_offset < FLASH_LENGTH_BOOT_ROM )
            fInfo.flash_rootfs_start_offset = FLASH_LENGTH_BOOT_ROM;
     
        flash_read_buf (NVRAM_SECTOR, CFE_VERSION_OFFSET,
            (unsigned char *)&bootCfeVersion, sizeof(bootCfeVersion));

        /* Read the flash contents into NVRAM buffer */
        flash_read_buf (NVRAM_SECTOR, NVRAM_DATA_OFFSET,
                        inMemNvramData_buf, sizeof (NVRAM_DATA)) ;
    }
#ifdef AEI_VDSL_CUSTOMER_NCS
    /* Enable Backup PSI by default */
    //printk("###set backup psi before,crc=%x\n",inMemNvramData.ulCheckSum);
    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    if(inMemNvramData.backupPsi != 0x01)
    {
        inMemNvramData.backupPsi = 0x01;

        inMemNvramData.ulCheckSum=0;
        inMemNvramData.ulCheckSum = getCrc32(inMemNvramData_buf, sizeof (NVRAM_DATA), CRC32_INIT_VALUE);
        memset(pStrTmp, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);
        memcpy(pStrTmp,inMemNvramData_buf, sizeof(NVRAM_DATA));
        //printk("###set backup psi end,crc=%x\n",inMemNvramData.ulCheckSum);
        //printk("Enable Backup PSI inMemNvramData.backupPsi = 0x%x\n", inMemNvramData.backupPsi);
    }
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
#endif  // AEI_VDSL_CUSTOMER_NCS

#if defined(DEBUG_FLASH)
    printk("reading nvram into inMemNvramData\n");
    printk("ulPsiSize 0x%x\n", (unsigned int)inMemNvramData.ulPsiSize);
    printk("backupPsi 0x%x\n", (unsigned int)inMemNvramData.backupPsi);
    printk("ulSyslogSize 0x%x\n", (unsigned int)inMemNvramData.ulSyslogSize);
#endif

    if ((BpSetBoardId(inMemNvramData.szBoardId) != BP_SUCCESS))
        printk("\n*** Board is not initialized properly ***\n\n");
#if defined (CONFIG_BCM_ENDPOINT_MODULE)
    if ((BpSetVoiceBoardId(inMemNvramData.szVoiceBoardId) != BP_SUCCESS))
        printk("\n*** Voice Board id is not initialized properly ***\n\n");
#endif
}

/***********************************************************************
 * Function Name: kerSysCfeVersionGet
 * Description  : Get CFE Version.
 * Returns      : 1 -- ok, 0 -- fail
 ***********************************************************************/
int kerSysCfeVersionGet(char *string, int stringLength)
{
    memcpy(string, (unsigned char *)&bootCfeVersion, stringLength);
    return(0);
}

/****************************************************************************
 * NVRAM functions
 * NVRAM data could share a sector with the CFE.  So a write to NVRAM
 * data is actually a read/modify/write operation on a sector.  Protected
 * by a higher level mutex, flashImageMutex.
 * Nvram data is cached in memory in a variable called inMemNvramData, so
 * writes will update this variable and reads just read from this variable.
 ****************************************************************************/


/** set nvram data
 * Must be called with flashImageMutex held
 *
 * @return 0 on success, -1 on failure.
 */
int kerSysNvRamSet(const char *string, int strLen, int offset)
{
    int sts = -1;  // initialize to failure
    char *pBuf = NULL;

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);
    BCM_ASSERT_R(offset+strLen <= NVRAM_LENGTH, sts);

    if (bootFromNand == 0)
    {
        if ((pBuf = getSharedBlks(NVRAM_SECTOR, 1)) == NULL)
            return sts;

        // set string to the memory buffer
        memcpy((pBuf + NVRAM_DATA_OFFSET + offset), string, strLen);

        sts = setSharedBlks(NVRAM_SECTOR, 1, pBuf);
    
        retriedKfree(pBuf);       
    }
    else
    {
#ifdef AEI_CONFIG_JFFS
       struct mtd_info *mtd1 = get_mtd_device_nm("nvram");
       char *tempStorage;
       char *block_buf=NULL;
       int retlen = 0;
       if (NULL != (tempStorage = kmalloc(mtd1->erasesize, GFP_KERNEL)) && mtd1 !=NULL)
       {
            PNVRAM_DATA pnd = NULL;
            unsigned long flags;
            int j=0;
            down(&semflash);
            memset(tempStorage, UNINITIALIZED_FLASH_DATA_CHAR, mtd1->erasesize );
            for(j=0;j<5;j++)
            {    
                mtd1->read(mtd1, 0, mtd1->erasesize, &retlen, tempStorage);
                pnd = (PNVRAM_DATA) (tempStorage + NVRAM_DATA_OFFSET);
                printk("before change szboard(%s) psk(%s) datapump(%ld),offset(%d),len(%d)\n",pnd->szBoardId,pnd->wpaKey,pnd->dslDatapump,offset,strLen);
                if(tempStorage[0]== 0x10 && tempStorage[1] ==0x00 || tempStorage[2]==0x02 || tempStorage[3] == 0x7b)
                {
                    break;
                }
            }

            if(j==5)
            {
                if( mtd1 )
                    put_mtd_device(mtd1);

                kfree(tempStorage);
                up(&semflash);
                return sts;

            }


            memcpy((tempStorage + NVRAM_DATA_OFFSET + offset), string, strLen);
                        /* Flash the CFE ROM boot loader. */
            nandEraseBlkNotSpare( mtd1, 0 );
            mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, tempStorage);

            do 
            {    
                if (NULL !=(block_buf = kmalloc(mtd1->erasesize, GFP_KERNEL)) )
                {
                    memset(block_buf,0,mtd1->erasesize);
                    mtd1->read(mtd1, 0, mtd1->erasesize, &retlen, block_buf);
                    if(memcmp(block_buf,tempStorage,mtd1->erasesize)!=0)
                    {
                        printk("##write nvram error\n");
                        nandEraseBlkNotSpare( mtd1, 0 );
                        mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, tempStorage);
                        kfree(block_buf);
                    }
                    else
                    {
                        printk("##write nvram correct\n");
                        kfree(block_buf);
                        break;
                    }
                }
                else
                {
                    printk("##malloc fail\n");
                    break;
                }
            }
            while(1);
            

            up(&semflash);
            spin_lock_irqsave(&inMemNvramData_spinlock, flags);
            memcpy(inMemNvramData_buf + offset,string, strLen);
            spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

            kfree(tempStorage);
            if( mtd1 )
                put_mtd_device(mtd1);
				
            sts = 0;
		
        }
#else
        sts = nandNvramSet(string);
#endif
    }
    
    if (0 == sts)
    {
        // write to flash was OK, now update in-memory copy
        updateInMemNvramData((unsigned char *) string, strLen, offset);
    }

    return sts;
}


/** get nvram data
 *
 * since it reads from in-memory copy of the nvram data, always successful.
 */
void kerSysNvRamGet(char *string, int strLen, int offset)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    memcpy(string, inMemNvramData_buf + offset, strLen);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

    return;
}


/** Erase entire nvram area.
 *
 * Currently there are no callers of this function.  THe return value is
 * the opposite of kerSysNvramSet.  Kept this way for compatibility.
 *
 * @return 0 on failure, 1 on success.
 */
int kerSysEraseNvRam(void)
{
    int sts = 1;

    BCM_ASSERT_NOT_HAS_MUTEX_C(&flashImageMutex);

    if (bootFromNand == 0)
    {
        char *tempStorage;
        if (NULL == (tempStorage = kmalloc(NVRAM_LENGTH, GFP_KERNEL)))
        {
            sts = 0;
        }
        else
        {
            // just write the whole buf with '0xff' to the flash
            memset(tempStorage, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);
            mutex_lock(&flashImageMutex);
            if (kerSysNvRamSet(tempStorage, NVRAM_LENGTH, 0) != 0)
                sts = 0;
            mutex_unlock(&flashImageMutex);
            kfree(tempStorage);
        }
    }
    else
    {
        printk("kerSysEraseNvram: not supported when bootFromNand == 1\n");
        sts = 0;
    }

    return sts;
}

unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len )
{
    int sect = flash_get_blk((int) fromaddr);
    unsigned char *start = flash_get_memptr(sect);

#ifdef AEI_VDSL_CUSTOMER_NCS
    down(&rd_semflash);
#else
    down(&semflash);
#endif
    flash_read_buf( sect, (int) fromaddr - (int) start, toaddr, len );
#ifdef AEI_VDSL_CUSTOMER_NCS
    up(&rd_semflash);
#else
    up(&semflash);
#endif

    return( len );
}

#else // CONFIG_BRCM_IKOS
static NVRAM_DATA ikos_nvram_data =
    {
    NVRAM_VERSION_NUMBER,
    "",
    "ikos",
    0,
    DEFAULT_PSI_SIZE,
    11,
    {0x02, 0x10, 0x18, 0x01, 0x00, 0x01},
    0x00, 0x00,
    0x720c9f60
    };

void kerSysEarlyFlashInit( void )
{
    inMemNvramData_buf = (unsigned char *) &inMemNvramData;
    memset(inMemNvramData_buf, UNINITIALIZED_FLASH_DATA_CHAR, NVRAM_LENGTH);

    memcpy(inMemNvramData_buf, (unsigned char *)&ikos_nvram_data,
        sizeof (NVRAM_DATA));
    fInfo.flash_scratch_pad_length = 0;
    fInfo.flash_persistent_start_blk = 0;
}

int kerSysCfeVersionGet(char *string, int stringLength)
{
    *string = '\0';
    return(0);
}

int kerSysNvRamGet(char *string, int strLen, int offset)
{
    memcpy(string, (unsigned char *) &ikos_nvram_data, sizeof(NVRAM_DATA));
    return(0);
}

int kerSysNvRamSet(char *string, int strLen, int offset)
{
    return(0);
}

int kerSysEraseNvRam(void)
{
    return(0);
}

unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr,
    unsigned long len )
{
    return(memcpy((unsigned char *) toaddr, (unsigned char *) fromaddr, len));
}
#endif  // CONFIG_BRCM_IKOS


/** Update the in-Memory copy of the nvram with the given data.
 *
 * @data: pointer to new nvram data
 * @len: number of valid bytes in nvram data
 * @offset: offset of the given data in the nvram data
 */
void updateInMemNvramData(const unsigned char *data, int len, int offset)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    memcpy(inMemNvramData_buf + offset, data, len);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
}


/** Get the bootline string from the NVRAM data.
 * Assumes the caller has the inMemNvramData locked.
 * Special case: this is called from prom.c without acquiring the
 * spinlock.  It is too early in the bootup sequence for spinlocks.
 *
 * @param bootline (OUT) a buffer of NVRAM_BOOTLINE_LEN bytes for the result
 */
void kerSysNvRamGetBootlineLocked(char *bootline)
{
    memcpy(bootline, inMemNvramData.szBootline,
                     sizeof(inMemNvramData.szBootline));
}
EXPORT_SYMBOL(kerSysNvRamGetBootlineLocked);


/** Get the bootline string from the NVRAM data.
 *
 * @param bootline (OUT) a buffer of NVRAM_BOOTLINE_LEN bytes for the result
 */
void kerSysNvRamGetBootline(char *bootline)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    kerSysNvRamGetBootlineLocked(bootline);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
}
EXPORT_SYMBOL(kerSysNvRamGetBootline);


/** Get the BoardId string from the NVRAM data.
 * Assumes the caller has the inMemNvramData locked.
 * Special case: this is called from prom_init without acquiring the
 * spinlock.  It is too early in the bootup sequence for spinlocks.
 *
 * @param boardId (OUT) a buffer of NVRAM_BOARD_ID_STRING_LEN
 */
void kerSysNvRamGetBoardIdLocked(char *boardId)
{
    memcpy(boardId, inMemNvramData.szBoardId,
                    sizeof(inMemNvramData.szBoardId));
}
EXPORT_SYMBOL(kerSysNvRamGetBoardIdLocked);


/** Get the BoardId string from the NVRAM data.
 *
 * @param boardId (OUT) a buffer of NVRAM_BOARD_ID_STRING_LEN
 */
void kerSysNvRamGetBoardId(char *boardId)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    kerSysNvRamGetBoardIdLocked(boardId);
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
}
EXPORT_SYMBOL(kerSysNvRamGetBoardId);


/** Get the base mac addr from the NVRAM data.  This is 6 bytes, not
 * a string.
 *
 * @param baseMacAddr (OUT) a buffer of NVRAM_MAC_ADDRESS_LEN
 */
void kerSysNvRamGetBaseMacAddr(unsigned char *baseMacAddr)
{
    unsigned long flags;

    spin_lock_irqsave(&inMemNvramData_spinlock, flags);
    memcpy(baseMacAddr, inMemNvramData.ucaBaseMacAddr,
                        sizeof(inMemNvramData.ucaBaseMacAddr));
    spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);
#ifdef AEI_VDSL_CUSTOMER_NCS
    printk("###baseMacAddr(%s)\n",baseMacAddr);
#endif
}
EXPORT_SYMBOL(kerSysNvRamGetBaseMacAddr);


/** Get the nvram version from the NVRAM data.
 *
 * @return nvram version number.
 */
unsigned long kerSysNvRamGetVersion(void)
{
    return (inMemNvramData.ulVersion);
}
EXPORT_SYMBOL(kerSysNvRamGetVersion);


void kerSysFlashInit( void )
{
    sema_init(&semflash, 1);
#ifdef AEI_VDSL_CUSTOMER_NCS
    sema_init(&rd_semflash, 1);
#endif

    // too early in bootup sequence to acquire spinlock, not needed anyways
    // only the kernel is running at this point
    flash_init_info(&inMemNvramData, &fInfo);
}

/***********************************************************************
 * Function Name: kerSysFlashAddrInfoGet
 * Description  : Fills in a structure with information about the NVRAM
 *                and persistent storage sections of flash memory.  
 *                Fro physmap.c to mount the fs vol.
 * Returns      : None.
 ***********************************************************************/
void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info)
{
    memcpy(pflash_addr_info, &fInfo, sizeof(FLASH_ADDR_INFO));
}

/*******************************************************************************
 * PSI functions
 * PSI is where we store the config file.  There is also a "backup" PSI
 * that stores an extra copy of the PSI.  THe idea is if the power goes out
 * while we are writing the primary PSI, the backup PSI will still have
 * a good copy from the last write.  No additional locking is required at
 * this level.
 *******************************************************************************/
#define PSI_FILE_NAME           "/data/psi"
#define PSI_BACKUP_FILE_NAME    "/data/psibackup"
#define SCRATCH_PAD_FILE_NAME   "/data/scratchpad"


// get psi data
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Read PSI from
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;
        int len;

        memset(string, 0x00, strLen);
        fp = filp_open(PSI_FILE_NAME, O_RDONLY, 0);
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((len = (int) fp->f_op->read(fp, (void *) string, strLen,
               &fp->f_pos)) <= 0)
                printk("Failed to read psi from '%s'\n", PSI_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }

        return 0;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        fInfo.flash_persistent_number_blk)) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + fInfo.flash_persistent_blk_offset + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

int kerSysBackupPsiGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Read backup PSI from
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;
        int len;

        memset(string, 0x00, strLen);
        fp = filp_open(PSI_BACKUP_FILE_NAME, O_RDONLY, 0);
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((len = (int) fp->f_op->read(fp, (void *) string, strLen,
               &fp->f_pos)) <= 0)
                printk("Failed to read psi from '%s'\n", PSI_BACKUP_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }

        return 0;
    }

    if (fInfo.flash_backup_psi_number_blk <= 0)
    {
        printk("No backup psi blks allocated, change it in CFE\n");
        return -1;
    }

    if (fInfo.flash_persistent_start_blk == 0)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_backup_psi_start_blk,
                              fInfo.flash_backup_psi_number_blk)) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

// set psi 
// return:
//  0 - ok
//  -1 - fail
int kerSysPersistentSet(char *string, int strLen, int offset)
{
    int sts = 0;
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Write PSI to
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;

        fp = filp_open(PSI_FILE_NAME, O_RDWR | O_TRUNC | O_CREAT,
            S_IRUSR | S_IWUSR);

        if (!IS_ERR(fp) && fp->f_op && fp->f_op->write)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((int) fp->f_op->write(fp, (void *) string, strLen,
               &fp->f_pos) != strLen)
                printk("Failed to write psi to '%s'.\n", PSI_FILE_NAME);

            vfs_fsync(fp, fp->f_path.dentry, 0);
            filp_close(fp, NULL);
            set_fs(fs);
        }
        else
            printk("Unable to open '%s'.\n", PSI_FILE_NAME);

        return 0;
    }

    if ((pBuf = getSharedBlks(fInfo.flash_persistent_start_blk,
        fInfo.flash_persistent_number_blk)) == NULL)
        return -1;

    // set string to the memory buffer
    memcpy((pBuf + fInfo.flash_persistent_blk_offset + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_persistent_start_blk, 
        fInfo.flash_persistent_number_blk, pBuf) != 0)
        sts = -1;
    
    retriedKfree(pBuf);

    return sts;
}

int kerSysBackupPsiSet(char *string, int strLen, int offset)
{
    int i;
    int sts = 0;
    int usedBlkSize = 0;
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Write backup PSI to
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;

        fp = filp_open(PSI_BACKUP_FILE_NAME, O_RDWR | O_TRUNC | O_CREAT,
            S_IRUSR | S_IWUSR);

        if (!IS_ERR(fp) && fp->f_op && fp->f_op->write)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((int) fp->f_op->write(fp, (void *) string, strLen,
               &fp->f_pos) != strLen)
                printk("Failed to write psi to '%s'.\n", PSI_BACKUP_FILE_NAME);

            vfs_fsync(fp, fp->f_path.dentry, 0);
            filp_close(fp, NULL);
            set_fs(fs);
        }
        else
            printk("Unable to open '%s'.\n", PSI_BACKUP_FILE_NAME);


        return 0;
    }

    if (fInfo.flash_backup_psi_number_blk <= 0)
    {
        printk("No backup psi blks allocated, change it in CFE\n");
        return -1;
    }

    if (fInfo.flash_persistent_start_blk == 0)
        return -1;

    /*
     * The backup PSI does not share its blocks with anybody else, so I don't have
     * to read the flash first.  But now I have to make sure I allocate a buffer
     * big enough to cover all blocks that the backup PSI spans.
     */
    for (i=fInfo.flash_backup_psi_start_blk;
         i < (fInfo.flash_backup_psi_start_blk + fInfo.flash_backup_psi_number_blk); i++)
    {
       usedBlkSize += flash_get_sector_size((unsigned short) i);
    }

#ifdef AEI_VDSL_CUSTOMER_NCS
    if (strLen + offset > usedBlkSize)
    {
        printk("kerSysBackupPsiSet strLen = %d, offset = %d, usedBlkSize = %d\n", strLen, offset, usedBlkSize);
        return -1;
    }
#endif
    if ((pBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
    {
       printk("failed to allocate memory with size: %d\n", usedBlkSize);
       return -1;
    }

    memset(pBuf, 0, usedBlkSize);

    // set string to the memory buffer
    memcpy((pBuf + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_backup_psi_start_blk, fInfo.flash_backup_psi_number_blk, 
                      pBuf) != 0)
        sts = -1;
    
    retriedKfree(pBuf);

    return sts;
}


/*******************************************************************************
 * "Kernel Syslog" is one or more sectors allocated in the flash
 * so that we can persist crash dump or other system diagnostics info
 * across reboots.  This feature is current not implemented.
 *******************************************************************************/
#if defined(AEI_VDSL_CUSTOMER_NCS)
#define SYSLOG_FILE_NAME        "/data/syslog"
#else
#define SYSLOG_FILE_NAME        "/etc/syslog"
#endif

int kerSysSyslogGet(char *string, int strLen, int offset)
{
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Read syslog from
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;

        memset(string, 0x00, strLen);
        fp = filp_open(SYSLOG_FILE_NAME, O_RDONLY, 0);
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((int) fp->f_op->read(fp, (void *) string, strLen,
               &fp->f_pos) <= 0)
#if defined(AEI_VDSL_CUSTOMER_NCS)
                printk("Failed to read syslog from '%s'\n", SYSLOG_FILE_NAME);
#else
                printk("Failed to read psi from '%s'\n", SYSLOG_FILE_NAME);
#endif
            filp_close(fp, NULL);
            set_fs(fs);
        }

        return 0;
    }

    if (fInfo.flash_syslog_number_blk <= 0)
    {
        printk("No syslog blks allocated, change it in CFE\n");
        return -1;
    }
    
    if (strLen > fInfo.flash_syslog_length)
        return -1;

    if ((pBuf = getSharedBlks(fInfo.flash_syslog_start_blk,
                              fInfo.flash_syslog_number_blk)) == NULL)
        return -1;

    // get string off the memory buffer
    memcpy(string, (pBuf + offset), strLen);

    retriedKfree(pBuf);

    return 0;
}

int kerSysSyslogSet(char *string, int strLen, int offset)
{
    int i;
    int sts = 0;
    int usedBlkSize = 0;
    char *pBuf = NULL;

    if( bootFromNand )
    {
        /* Root file system is on a writable NAND flash.  Write syslog to
         * a file on the NAND flash.
         */
        struct file *fp;
        mm_segment_t fs;
#if defined(AEI_VDSL_CUSTOMER_NCS)
        fp = filp_open(SYSLOG_FILE_NAME, O_RDWR | O_TRUNC | O_CREAT,
            S_IRUSR | S_IWUSR);
#else
        fp = filp_open(PSI_FILE_NAME, O_RDWR | O_TRUNC | O_CREAT,
            S_IRUSR | S_IWUSR);
#endif
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->write)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((int) fp->f_op->write(fp, (void *) string, strLen,
               &fp->f_pos) != strLen)
#if defined(AEI_VDSL_CUSTOMER_NCS)
                printk("Failed to write syslog to '%s'.\n", SYSLOG_FILE_NAME);
#else
                printk("Failed to write psi to '%s'.\n", PSI_FILE_NAME);
#endif
            filp_close(fp, NULL);
            set_fs(fs);
        }
        else
#if defined(AEI_VDSL_CUSTOMER_NCS)
            printk("Unable to open '%s'.\n", SYSLOG_FILE_NAME);
#else
            printk("Unable to open '%s'.\n", PSI_FILE_NAME);
#endif
        return 0;
    }

    if (fInfo.flash_syslog_number_blk <= 0)
    {
        printk("No syslog blks allocated, change it in CFE\n");
        return -1;
    }
    
    if (strLen > fInfo.flash_syslog_length)
        return -1;

    /*
     * The syslog does not share its blocks with anybody else, so I don't have
     * to read the flash first.  But now I have to make sure I allocate a buffer
     * big enough to cover all blocks that the syslog spans.
     */
    for (i=fInfo.flash_syslog_start_blk;
         i < (fInfo.flash_syslog_start_blk + fInfo.flash_syslog_number_blk); i++)
    {
        usedBlkSize += flash_get_sector_size((unsigned short) i);
    }

    if ((pBuf = (char *) retriedKmalloc(usedBlkSize)) == NULL)
    {
       printk("failed to allocate memory with size: %d\n", usedBlkSize);
       return -1;
    }

    memset(pBuf, 0, usedBlkSize);

    // set string to the memory buffer
    memcpy((pBuf + offset), string, strLen);

    if (setSharedBlks(fInfo.flash_syslog_start_blk, fInfo.flash_syslog_number_blk, pBuf) != 0)
        sts = -1;

    retriedKfree(pBuf);

    return sts;
}


/*******************************************************************************
 * Writing software image to flash operations
 * This procedure should be serialized.  Look for flashImageMutex.
 *******************************************************************************/


#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

/*
 * nandUpdateSeqNum
 * 
 * Read the sequence number from each rootfs partition.  The sequence number is
 * the extension on the cferam file.  Add one to the highest sequence number
 * and change the extenstion of the cferam in the image to be flashed to that
 * number.
 */
static int nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen)
{
    char fname[] = NAND_CFE_RAM_NAME;
    int fname_actual_len = strlen(fname);
    int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
    char cferam_base[32], cferam_buf[32], cferam_fmt[32]; 
    int i;
    struct file *fp;
    int seq = -1;
    int ret = 1;
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_DUAL_IMAGE)
	PFILE_TAG pTag=(PFILE_TAG)(imagePtr);
#endif


    strcpy(cferam_base, fname);
    cferam_base[fname_cmp_len] = '\0';
    strcpy(cferam_fmt, cferam_base);
    strcat(cferam_fmt, "%3.3d");
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_DUAL_IMAGE)
    seq = simple_strtoul(pTag->imageSequence, NULL, 10);
#else
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
                /*Seq number found. Take the greater of the two seq numbers.*/
                if( seq < i )
                    seq = i;
                break;
            }
        }
    }
#endif
    if( seq != -1 )
    {
        unsigned char *buf, *p;
        int len = blkLen;
        struct jffs2_raw_dirent *pdir;
        unsigned long version = 0;
        int done = 0;

        if( *(unsigned short *) imagePtr != JFFS2_MAGIC_BITMASK )
        {
            imagePtr += len;
            imageSize -= len;
        }

#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_DUAL_IMAGE)
#else
        /* Increment the new highest sequence number. Add it to the CFE RAM
         * file name.
         */
        seq++;
#endif

        /* Search the image and replace the last three characters of file
         * cferam.000 with the new sequence number.
         */
        for(buf = imagePtr; buf < imagePtr+imageSize && done == 0; buf += len)
        {
            p = buf;
            while( p < buf + len )
            {
                pdir = (struct jffs2_raw_dirent *) p;
                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                {
                    if( je16_to_cpu(pdir->nodetype) == JFFS2_NODETYPE_DIRENT &&
                        fname_actual_len == pdir->nsize &&
                        !memcmp(fname, pdir->name, fname_cmp_len) &&
                        je32_to_cpu(pdir->version) > version &&
                        je32_to_cpu(pdir->ino) != 0 )
                     {
                        /* File cferam.000 found. Change the extension to the
                         * new sequence number and recalculate file name CRC.
                         */
                        p = pdir->name + fname_cmp_len;
                        p[0] = (seq / 100) + '0';
                        p[1] = ((seq % 100) / 10) + '0';
                        p[2] = ((seq % 100) % 10) + '0';
                        p[3] = '\0';

                        je32_to_cpu(pdir->name_crc) =
                            crc32(0, pdir->name, (unsigned int)
                            fname_actual_len);

                        version = je32_to_cpu(pdir->version);

                        /* Setting 'done = 1' assumes there is only one version
                         * of the directory entry.
                         */
                        done = 1;
                        ret = (buf - imagePtr) / len; /* block number */
                        break;
                    }

                    p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                }
                else
                {
                    done = 1;
                    break;
                }
            }
        }
    }

    return(ret);
}

/* Erase the specified NAND flash block but preserve the spare area. */
static int nandEraseBlkNotSpare( struct mtd_info *mtd, int blk )
{
    int sts = -1;

    /* block_is bad returns 0 if block is not bad */
    if( mtd->block_isbad(mtd, blk) == 0 )
    {
        unsigned char oobbuf[64]; /* expected to be a max size */
        struct mtd_oob_ops ops;

        memset(&ops, 0x00, sizeof(ops));
        ops.ooblen = mtd->oobsize;
        ops.ooboffs = 0;
        ops.datbuf = NULL;
        ops.oobbuf = oobbuf;
        ops.len = 0;
        ops.mode = MTD_OOB_PLACE;

        /* Read and save the spare area. */
        sts = mtd->read_oob(mtd, blk, &ops);
        if( sts == 0 )
        {
            struct erase_info erase;

            /* Erase the flash block. */
            memset(&erase, 0x00, sizeof(erase));
            erase.addr = blk;
            erase.len = mtd->erasesize;
            erase.mtd = mtd;

            sts = mtd->erase(mtd, &erase);
            if( sts == 0 )
            {
                int i;

                /* Function local_bh_disable has been called and this
                 * is the only operation that should be occurring.
                 * Therefore, spin waiting for erase to complete.
                 */
                for(i = 0; i < 10000 && erase.state != MTD_ERASE_DONE &&
                    erase.state != MTD_ERASE_FAILED; i++ )
                {
                    udelay(100);
                }

                if( erase.state != MTD_ERASE_DONE )
                    sts = -1;
            }

            if( sts == 0 )
            {
#ifdef AEI_NAND_IMG_CHECK
		/* clear crc field */
		if (!strncmp(mtd->name, "rootfs", 6))
			oobbuf[ops.ooblen-4] = oobbuf[ops.ooblen-3] = oobbuf[ops.ooblen-2] = oobbuf[ops.ooblen-1] = 0xff;
#endif
                memset(&ops, 0x00, sizeof(ops));
                ops.ooblen = mtd->oobsize;
                ops.ooboffs = 0;
                ops.datbuf = NULL;
                ops.oobbuf = oobbuf;
                ops.len = 0;
                ops.mode = MTD_OOB_PLACE;

                /* Restore the spare area. */
                if( (sts = mtd->write_oob(mtd, blk, &ops)) != 0 )
                    printk("nandImageSet - Block 0x%8.8x. Error writing spare "
                        "area.\n", blk);
            }
            else
                printk("nandImageSet - Block 0x%8.8x. Error erasing block.\n",blk);
        }
        else
            printk("nandImageSet - Block 0x%8.8x. Error read spare area.\n", blk);
    }

    return( sts );
}

#if defined(AEI_VDSL_CUSTOMER_NCS)
static void reportUpgradePercent(int percent)
{
        struct file     *f;

        long int        rv;
        mm_segment_t    old_fs;
        loff_t          offset=0;
        char buf[128] = "";

        sprintf(buf, "%d\n", percent);
        f = filp_open("/var/UpgradePercent", O_RDWR|O_CREAT|O_TRUNC, 0600);
        if(f == NULL)
                return;

        old_fs = get_fs();
        set_fs( get_ds() );
        rv = f->f_op->write(f, buf, strlen(buf), &offset);
        set_fs(old_fs);

        if (rv < strlen(buf)) {
                printk("write /var/UpgradePercent  error\n");
        }
        filp_close(f , 0);
        return;
}
#endif
// NAND flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
static int nandImageSet( int flash_start_addr, char *string, int img_size )
{
    /* Allow room to flash cferam sequence number at start of file system. */
    const int fs_start_blk_num = 8;

    int sts = -1;
    int blk = 0;
    int cferam_blk;
    int fs_start_blk;
    int ofs;
    int old_img = 0;
    char *cferam_string;
    char *end_string = string + img_size;
    struct mtd_info *mtd0 = NULL;
    struct mtd_info *mtd1 = get_mtd_device_nm("nvram");
    WFI_TAG wt = {0};
#ifdef AEI_CONFIG_JFFS
    //char blockbuf[16*1024];
    char * block_buf=NULL;
#if defined(AEI_CONFIG_JFFS)
	char * cferombuf=NULL;
    char * tagbuf=NULL;
#endif
    struct mtd_info *mtd2 = NULL;
    int image_number=1;
#if defined(AEI_VDSL_DUAL_IMAGE)
	PFILE_TAG pTag;
	int newImageSequence;
#endif
#endif
    if( mtd1 )
    {
        int blksize = mtd1->erasesize / 1024;

        memcpy(&wt, end_string, sizeof(wt));
        if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            ((blksize == 16 && wt.wfiFlashType != WFI_NAND16_FLASH) ||
             (blksize < 128 && wt.wfiFlashType == WFI_NAND128_FLASH)) )
        {
            printk("\nERROR: NAND flash block size %dKB does not work with an "
                "image built with %dKB block size\n\n", blksize,
                (wt.wfiFlashType == WFI_NAND16_FLASH) ? 16 : 128);
        }
        else
        {
            mtd0 = get_mtd_device_nm("rootfs_update");

            /* If the image version indicates that is uses a 1MB data partition
             * size and the image is intended to be flashed to the second file
             * system partition, change to the flash to the first partition.
             * After new image is flashed, delete the second file system and
             * data partitions (at the bottom of this function).
             */
            if( wt.wfiVersion == WFI_VERSION_NAND_1MB_DATA )
            {
                unsigned long rootfs_ofs;
                kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);
                
                if(rootfs_ofs == inMemNvramData.ulNandPartOfsKb[NP_ROOTFS_1] &&
                    mtd0)
                {
                    printk("Old image, flashing image to first partition.\n");
                    put_mtd_device(mtd0);
                    mtd0 = NULL;
                    old_img = 1;
                }
            }
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_DUAL_IMAGE)
	unsigned long rootfs_ofs;
	int iRootfs=NP_ROOTFS_1;
	kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);

	if( *(unsigned short *) string == JFFS2_MAGIC_BITMASK || *(unsigned short *) string == AEI_MAGIC_BITMASK )
		/* Image only contains file system. */
		ofs = 0; /* use entire string image to find sequence number */
	else
	{
		/* skip block 0 to find sequence number */
		switch(wt.wfiFlashType)
		{
		case WFI_NAND16_FLASH:
			ofs = 16 * 1024;
			break;

		case WFI_NAND128_FLASH:
			ofs = 128 * 1024;
			break;
		}
	}
	pTag = (PFILE_TAG)(string+ofs);

	newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);



	if(newImageSequence == IMAGE2_SEQUENCE)
		iRootfs=NP_ROOTFS_2;
	else
		iRootfs=NP_ROOTFS_1;
	if(rootfs_ofs == inMemNvramData.ulNandPartOfsKb[iRootfs] &&
		mtd0)
	{
		printk("Old image, flashing image to first partition.\n");
		put_mtd_device(mtd0);
		mtd0 = NULL;
	}
    image_number=iRootfs;
#endif

            if( mtd0 == NULL || mtd0->size == 0LL )
            {
                /* Flash device is configured to use only one file system. */
                if( mtd0 )
                    put_mtd_device(mtd0);
                mtd0 = get_mtd_device_nm("rootfs");
                #ifdef AEI_CONFIG_JFFS
                mtd2 = get_mtd_device_nm("tag");
                #endif
            }
#ifdef AEI_CONFIG_JFFS
            else
            {
                mtd2 = get_mtd_device_nm("tag_update");
            }
#endif
        }
    }

    if( mtd0 && mtd1 )
    {
        unsigned long flags;
        int retlen = 0;

        #ifdef AEI_CONFIG_JFFS
        if( *(unsigned short *) string == JFFS2_MAGIC_BITMASK || *(unsigned short *) string == AEI_MAGIC_BITMASK )
        #else
        if( *(unsigned short *) string == JFFS2_MAGIC_BITMASK )
        #endif    
            /* Image only contains file system. */
            ofs = 0; /* use entire string image to find sequence number */
        else
        {
            /* Image contains CFE ROM boot loader. */
            PNVRAM_DATA pnd = (PNVRAM_DATA) (string + NVRAM_DATA_OFFSET);

            /* skip block 0 to find sequence number */
            switch(wt.wfiFlashType)
            {
            case WFI_NAND16_FLASH:
                ofs = 16 * 1024;
                break;

            case WFI_NAND128_FLASH:
                ofs = 128 * 1024;
                break;
            }

            /* Copy NVRAM data to block to be flashed so it is preserved. */
            spin_lock_irqsave(&inMemNvramData_spinlock, flags);
#if defined(AEI_63168_CHIP)  && defined(AEI_CONFIG_JFFS)
                if (NULL !=(cferombuf = kmalloc(mtd1->erasesize, GFP_KERNEL)) )
                {
                    memset(cferombuf,0,mtd1->erasesize);
					memcpy((unsigned char *) cferombuf, string,
						mtd1->erasesize);
					pnd = (PNVRAM_DATA) (cferombuf + NVRAM_DATA_OFFSET);
					memcpy((unsigned char *) pnd, inMemNvramData_buf,
									sizeof(NVRAM_DATA));

                }
                else
#endif
            memcpy((unsigned char *) pnd, inMemNvramData_buf,
                sizeof(NVRAM_DATA));
            spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

            /* Recalculate the nvramData CRC. */
            pnd->ulCheckSum = 0;
            pnd->ulCheckSum = crc32(CRC32_INIT_VALUE, pnd, sizeof(NVRAM_DATA));
        }

        /* Update the sequence number that replaces that extension in file
         * cferam.000
         */
        cferam_blk = nandUpdateSeqNum((unsigned char *) string + ofs,
            img_size - ofs, mtd0->erasesize) * mtd0->erasesize;

        cferam_string = string + ofs + cferam_blk;

        fs_start_blk = fs_start_blk_num * mtd0->erasesize;

        // Disable other tasks from this point on
#ifdef AEI_VDSL_CUSTOMER_NCS
    //If stop other CPU before beginning writing flash, 
	//it'll cause client Browser has no upgrade's status! 
#else
        stopOtherCpu();
#endif
        local_irq_save(flags);
#if defined(AEI_VDSL_CUSTOMER_NCS)
	// do nothing here
#else
        local_bh_disable();
#endif
#ifdef AEI_CONFIG_JFFS
        if( *(unsigned short *) string != JFFS2_MAGIC_BITMASK &&  *(unsigned short *) string != AEI_MAGIC_BITMASK)
#else
        if( *(unsigned short *) string != JFFS2_MAGIC_BITMASK )
#endif
        {
            /* Flash the CFE ROM boot loader. */
            nandEraseBlkNotSpare( mtd1, 0 );
#if defined(AEI_63168_CHIP)  && defined(AEI_CONFIG_JFFS)
			if(cferombuf != NULL)
			{
				mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, cferombuf);
			}
			else
#endif
            mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, string);
#ifdef AEI_CONFIG_JFFS
            // Because we don't disable the interrupt, NAND flash write is unreliable. We add to check the correctness of NAND flash writing by the software.

            do 
            {    
                if (NULL !=(block_buf = kmalloc(mtd1->erasesize, GFP_KERNEL)) )
                {
                    memset(block_buf,0,mtd1->erasesize);
                    mtd1->read(mtd1,0,mtd1->erasesize,&retlen,block_buf);
#if defined(AEI_63168_CHIP)  && defined(AEI_CONFIG_JFFS)
                    if((cferombuf != NULL && memcmp(block_buf,cferombuf,mtd1->erasesize)!=0) || (cferombuf == NULL && memcmp(block_buf,string,mtd1->erasesize)!=0))
#else					
                    if(memcmp(block_buf,string,mtd1->erasesize)!=0)
#endif
                    {
                        printk("##write cfe error\n");
                        nandEraseBlkNotSpare( mtd1, 0 );
#if defined(AEI_63168_CHIP)  && defined(AEI_CONFIG_JFFS)
						if(cferombuf != NULL)
						mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, cferombuf);
						else
#endif
                        mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, string);
                        kfree(block_buf);
                    }
                    else
                    {
                        printk("##write cfe correct\n");
                        kfree(block_buf);
                        break;
                    }
                }
                else
                {
                    printk("##malloc fail\n");
                    break;
                }
            }
            while(1);
#if defined(AEI_63168_CHIP)  && defined(AEI_CONFIG_JFFS)
			kfree(cferombuf);
#endif
#endif       
            string += ofs;
        }

#ifdef AEI_CONFIG_JFFS
        if((*(unsigned short *) string == JFFS2_MAGIC_BITMASK && *(unsigned short *) (string+2) == AEI_MAGIC_BITMASK) && mtd2!=NULL)
        {
            nandEraseBlkNotSpare( mtd2, 0 );
            if (NULL !=(tagbuf = kmalloc(mtd2->erasesize, GFP_KERNEL)) )
            {
                memset(tagbuf,0,mtd2->erasesize);
                memcpy((unsigned char *) tagbuf, string,
                    mtd2->erasesize);
            }
            else
            {
                printk("##Write tag start,mtd->name(%s)\n",mtd2->name);
                /* Flash the image header. */
                mtd2->write(mtd2, 0, mtd2->erasesize, &retlen, string);
                // Because we don't disable the interrupt, NAND flash write is unreliable. We add to check the correctness of NAND flash writing by the software.
                do 
                {	 
					if (NULL !=(block_buf = kmalloc(mtd2->erasesize, GFP_KERNEL)) )
					{
						memset(block_buf,0,mtd2->erasesize);
						mtd2->read(mtd2,0,mtd2->erasesize,&retlen,block_buf);
					   // printk("#####block(%d),image(%d)\n",inMemNvramData.ulNandPartOfsKb[image_number]/inMemNvramData.ulNandPartSizeKb[NP_BOOT],image_number);
						if(memcmp(block_buf,string,mtd2->erasesize)!=0)
						{
							printk("##write tag error\n");
							nandEraseBlkNotSpare( mtd2, 0 );
							mtd2->write(mtd2, 0, mtd2->erasesize, &retlen, string);
							kfree(block_buf);
						}
						else
						{
							printk("##write tag correct\n");
							kfree(block_buf);
							break;
						}
					}
					else
					{
						printk("##malloc fail\n");
						break;
					}
                }
                while(1);

            }
            string +=mtd2->erasesize ;
        }
#endif
        /* Erase block with sequence number before flashing the image. */
#ifdef AEI_CONFIG_JFFS
        for( blk = 0; blk < fs_start_blk; blk += mtd0->erasesize )
        {
            nandEraseBlkNotSpare( mtd0, blk );
        }
#else
        nandEraseBlkNotSpare( mtd0, cferam_blk );
#endif
#ifdef AEI_NAND_IMG_CHECK
		if(gSetWrongCRC == 1) //1=set wrong crc
		{
			mtd0->oobavail = 9;
		}
#endif
        /* Flash the image except for the part with the sequence number. */
        for( blk = fs_start_blk; blk < mtd0->size; blk += mtd0->erasesize )
        {
#ifdef AEI_CONFIG_JFFS   
           do 
           {
#endif
            if( (sts = nandEraseBlkNotSpare( mtd0, blk )) == 0 )
            {
                /* Write a block of the image to flash. */
                if( string < end_string && string != cferam_string )
                {
                    int writelen = ((string + mtd0->erasesize) <= end_string)
                        ? mtd0->erasesize : (int) (end_string - string);

                    mtd0->write(mtd0, blk, writelen, &retlen, string);
#ifdef AEI_CONFIG_JFFS   
            // Because we don't disable the interrupt, NAND flash write is unreliable. We add to check the correctness of NAND flash writing by the software.
                 
                if (NULL !=(block_buf = kmalloc(writelen, GFP_KERNEL)) )
                {
                    memset(block_buf,0,writelen);
                    mtd0->read(mtd0, blk, writelen, &retlen, block_buf);
                    if(memcmp(block_buf,string,writelen)!=0)
                    {
                        printk("##write rf block (%d) error\n",inMemNvramData.ulNandPartOfsKb[image_number]/inMemNvramData.ulNandPartSizeKb[NP_BOOT]+1+blk/mtd0->erasesize);
                        kfree(block_buf);
                        continue;
                    }
                    else
                    {
                        //printk("##write rf block(%d) correct\n",inMemNvramData.ulNandPartOfsKb[image_number]/inMemNvramData.ulNandPartSizeKb[NP_BOOT]+1+blk/mtd0->erasesize);
                        kfree(block_buf);
                    }
                }
                else
                {
                    printk("##malloc fail\n");
                }
#endif


                    if( retlen == writelen )
                    {
                        printk(".");
                        string += writelen;
#if defined(AEI_VDSL_CUSTOMER_NCS)
                        reportUpgradePercent(100-(int) (end_string - string)*100/img_size);
#endif	
#ifdef AEI_CONFIG_JFFS
                        //writing flash is sucessful,writing the next block
                        break;
#endif
                    }
                }
                else
#ifdef AEI_CONFIG_JFFS
                {
#endif
                    string += mtd0->erasesize;
#ifdef AEI_CONFIG_JFFS
                    break;
                }
#endif
            }
#ifdef AEI_CONFIG_JFFS
            else
            {
                //the flash blcok is bad,skip it
                break;
            }
           }
            while(1);
#endif
        }

        /* Flash the image part with the sequence number. */
        for( blk = 0; blk < fs_start_blk; blk += mtd0->erasesize )
        {
#ifdef AEI_CONFIG_JFFS 
            do
            {
#endif
            if( (sts = nandEraseBlkNotSpare( mtd0, blk )) == 0 )
            {
                /* Write a block of the image to flash. */
                if( cferam_string )
                {
                    mtd0->write(mtd0, blk, mtd0->erasesize,
                        &retlen, cferam_string);
#ifdef AEI_CONFIG_JFFS 
            // Because we don't disable the interrupt, NAND flash write is unreliable. We add to check the correctness of NAND flash writing by the software.
                   
                if (NULL !=(block_buf = kmalloc(mtd0->erasesize, GFP_KERNEL)) )
                {
                    memset(block_buf,0,mtd0->erasesize);
                    mtd0->read(mtd0, blk, mtd0->erasesize,
                        &retlen, block_buf);

                    if(memcmp(block_buf,cferam_string,mtd0->erasesize)!=0)
                    {
                        printk("##write rf block (%d) error\n",inMemNvramData.ulNandPartOfsKb[image_number]/inMemNvramData.ulNandPartSizeKb[NP_BOOT]+1+blk/mtd0->erasesize);
                        kfree(block_buf);
                        continue;
                    }
                    else
                    {
                        //printk("##write rf block(%d) correct\n",inMemNvramData.ulNandPartOfsKb[image_number]/inMemNvramData.ulNandPartSizeKb[NP_BOOT]+1+blk/mtd0->erasesize);
                        kfree(block_buf);
                    }
                }
                else
                {
                    printk("##malloc fail\n");
                }
#endif

                    if( retlen == mtd0->erasesize )
                    {
                        printk(".");
                        cferam_string = NULL;
#ifdef AEI_CONFIG_JFFS
                        //writing flash is sucessful,writing the next block
                        break;
#endif

                    }
                }
#ifdef AEI_CONFIG_JFFS
                else
                    break;
#endif
            }
#ifdef AEI_CONFIG_JFFS
            else
            {
                //the flash blcok is bad,skip it
                break;
            }
            }
            while(1);
#endif
        }

#ifdef AEI_CONFIG_JFFS
        if(tagbuf != NULL)
        {
            printk("##Write tag start,mtd->name(%s)\n",mtd2->name);
            nandEraseBlkNotSpare( mtd2, 0 );
            mtd2->write(mtd2, 0, mtd2->erasesize, &retlen, tagbuf);
            kfree(tagbuf);
        }
#endif

        if (sts)
        {
            // re-enable bh and irq only if there was an error and router
            // will not reboot
            local_irq_restore(flags);
#if !defined(AEI_VDSL_CUSTOMER_NCS)
            local_bh_enable();
#endif
            sts = (blk > mtd0->erasesize) ? blk / mtd0->erasesize : 1;
        }

        printk("\n\n");
    }
    if( mtd0 )
        put_mtd_device(mtd0);

    if( mtd1 )
        put_mtd_device(mtd1);

#ifdef AEI_CONFIG_JFFS
    if( mtd2 )
        put_mtd_device(mtd2);
#endif      


    if( sts == 0 && old_img == 1 )
    {
        printk("\nOld image, deleting data and secondary file system partitions\n");
        mtd0 = get_mtd_device_nm("data");
        for( blk = 0; blk < mtd0->size; blk += mtd0->erasesize )
            nandEraseBlkNotSpare( mtd0, blk );
        mtd0 = get_mtd_device_nm("rootfs_update");
        for( blk = 0; blk < mtd0->size; blk += mtd0->erasesize )
            nandEraseBlkNotSpare( mtd0, blk );
    }
#ifdef AEI_VDSL_CUSTOMER_NCS
    //
    // Why stop other CPU here? Because the kernel will happen "call stack overflow" 
    // after finishing writing flash, so stop other CPU before restart kernel. If do
    // this before beginning writing flash, it'll cause client Browser has no upgrade's
    // status! 
    //
    //stopOtherCpu();
#endif

    return sts;
}

    // NAND flash overwrite nvram block.    
    // return: 
    // 0 - ok
    // !0 - the sector number fail to be flashed (should not be 0)
static int nandNvramSet(const char *nvramString )
{
    /* Image contains CFE ROM boot loader. */
    struct mtd_info *mtd = get_mtd_device_nm("nvram"); 
    char *cferom = NULL;
    int retlen = 0;
        
    if ( (cferom = (char *)retriedKmalloc(mtd->erasesize)) == NULL )
    {
        printk("\n Failed to allocated memory in nandNvramSet();");
        return -1;
    }

    /* Read the whole cfe rom nand block 0 */
    mtd->read(mtd, 0, mtd->erasesize, &retlen, cferom);

    /* Copy the nvram string into place */
    memcpy(cferom+NVRAM_DATA_OFFSET, nvramString, sizeof(NVRAM_DATA));
    
    /* Flash the CFE ROM boot loader. */
    nandEraseBlkNotSpare( mtd, 0 );
    mtd->write(mtd, 0, mtd->erasesize, &retlen, cferom);
    
    retriedKfree(cferom);
    return 0;
}
           

// flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
// Must be called with flashImageMutex held.
int kerSysBcmImageSet( int flash_start_addr, char *string, int size,
    int should_yield)
{
    int sts;
    int sect_size;
    int blk_start;
    int savedSize = size;
    int whole_image = 0;
    unsigned long flags=0;
    int is_cfe_write=0;
    WFI_TAG wt = {0};
#if defined(AEI_VDSL_CUSTOMER_NCS)
    int whole_size = size;
#endif

    BCM_ASSERT_HAS_MUTEX_C(&flashImageMutex);

    if (flash_start_addr == FLASH_BASE)
    {
        unsigned long chip_id = kerSysGetChipId();
        whole_image = 1;
        memcpy(&wt, string + size, sizeof(wt));
        if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            wt.wfiChipId != chip_id )
        {
            printk("Chip Id error.  Image Chip Id = %04lx, Board Chip Id = "
                "%04lx.\n", wt.wfiChipId, chip_id);
            return -1;
        }
    }

    if( bootFromNand )
    {
        if( whole_image == 1 && size > FLASH_LENGTH_BOOT_ROM )
            return( nandImageSet( flash_start_addr, string, size ) );

        printk("\n**** Illegal NAND flash image ****\n\n");
        return -1;
    }

    if( whole_image && (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
        wt.wfiFlashType != WFI_NOR_FLASH )
    {
        printk("ERROR: Image does not support a NOR flash device.\n");
        return -1;
    }


#if defined(DEBUG_FLASH)
    printk("kerSysBcmImageSet: flash_start_addr=0x%x string=%p len=%d whole_image=%d\n",
           flash_start_addr, string, size, whole_image);
#endif

    blk_start = flash_get_blk(flash_start_addr);
    if( blk_start < 0 )
        return( -1 );

    is_cfe_write = ((NVRAM_SECTOR == blk_start) &&
                    (size <= flash_get_sector_size(blk_start)));

    /*
     * write image to flash memory.
     * In theory, all calls to flash_write_buf() must be done with
     * semflash held, so I added it here.  However, in reality, all
     * flash image writes are protected by flashImageMutex at a higher
     * level.
     */
    down(&semflash);

    // Once we have acquired the flash semaphore, we can
    // disable activity on other processor and also on local processor.
    // Need to disable interrupts so that RCU stall checker will not complain.
    if (!is_cfe_write && !should_yield)
    {
#ifdef AEI_VDSL_CUSTOMER_NCS
    //If stop other CPU before beginning writing flash, 
	//it'll cause client Browser has no upgrade's status! 
#else
        stopOtherCpu();
#endif
        local_irq_save(flags);
    }

#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
    local_bh_disable();
#endif

    do 
    {
        sect_size = flash_get_sector_size(blk_start);

        flash_sector_erase_int(blk_start);     // erase blk before flash

        if (sect_size > size) 
        {
            if (size & 1) 
                size++;
            sect_size = size;
        }
        
        if (flash_write_buf(blk_start, 0, string, sect_size) != sect_size) {
            break;
        }

        // check if we just wrote into the sector where the NVRAM is.
        // update our in-memory copy
        if (NVRAM_SECTOR == blk_start)
        {
            updateInMemNvramData(string+NVRAM_DATA_OFFSET, NVRAM_LENGTH, 0);
        }
#if defined(AEI_VDSL_CUSTOMER_NCS)
        reportUpgradePercent(100-size*100/whole_size);
#endif

        printk(".");
        blk_start++;
        string += sect_size;
        size -= sect_size; 

        if (should_yield)
        {
#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
            local_bh_enable();
#endif
            yield();
#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
            local_bh_disable();
#endif
        }
    } while (size > 0);

    if (whole_image && savedSize > fInfo.flash_rootfs_start_offset)
    {
        // If flashing a whole image, erase to end of flash.
        int total_blks = flash_get_numsectors();
        while( blk_start < total_blks )
        {
            flash_sector_erase_int(blk_start);
            printk(".");
            blk_start++;

            if (should_yield)
            {
#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
                local_bh_enable();
#endif
                yield();
#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
                local_bh_disable();
#endif
            }
        }
    }

    up(&semflash);

    printk("\n\n");

    if (is_cfe_write || should_yield)
    {
#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
        local_bh_enable();
#endif
    }

    if( size == 0 )
    {
        sts = 0;  // ok
    }
    else
    {
        /*
         * Even though we try to recover here, this is really bad because
         * we have stopped the other CPU and we cannot restart it.  So we
         * really should try hard to make sure flash writes will never fail.
         */
        printk(KERN_ERR "kerSysBcmImageSet: write failed at blk=%d\n", blk_start);
        sts = blk_start;    // failed to flash this sector
        if (!is_cfe_write && !should_yield)
        {
            local_irq_restore(flags);
#if defined(AEI_VDSL_CUSTOMER_NCS)
    // do nothing here
#else
            local_bh_enable();
#endif
        }
    }
#if defined(AEI_VDSL_CUSTOMER_NCS)
    //
    // Why stop other CPU here? Because the kernel will happen "call stack overflow" 
    // after finishing writing flash, so stop other CPU before restart kernel. If do
    // this before beginning writing flash, it'll cause client Browser has no upgrade's
    // status! 
    //
    if (!is_cfe_write)
        stopOtherCpu();
#endif

    return sts;
}

/*******************************************************************************
 * SP functions
 * SP = ScratchPad, one or more sectors in the flash which user apps can
 * store small bits of data referenced by a small tag at the beginning.
 * kerSysScratchPadSet() and kerSysScratchPadCLearAll() must be protected by
 * a mutex because they do read/modify/writes to the flash sector(s).
 * kerSysScratchPadGet() and KerSysScratchPadList() do not need to acquire
 * the mutex, however, I acquire the mutex anyways just to make this interface
 * symmetrical.  High performance and concurrency is not needed on this path.
 *
 *******************************************************************************/

// get scratch pad data into *** pTempBuf *** which has to be released by the
//      caller!
// return: if pTempBuf != NULL, points to the data with the dataSize of the
//      buffer
// !NULL -- ok
// NULL  -- fail
static char *getScratchPad(int len)
{
    /* Root file system is on a writable NAND flash.  Read scratch pad from
     * a file on the NAND flash.
     */
    char *ret = NULL;

    if( (ret = retriedKmalloc(len)) != NULL )
    {
        struct file *fp;
        mm_segment_t fs;

        memset(ret, 0x00, len);
        fp = filp_open(SCRATCH_PAD_FILE_NAME, O_RDONLY, 0);
        if (!IS_ERR(fp) && fp->f_op && fp->f_op->read)
        {
            fs = get_fs();
            set_fs(get_ds());

            fp->f_pos = 0;

            if((int) fp->f_op->read(fp, (void *) ret, len, &fp->f_pos) <= 0)
                printk("Failed to read scratch pad from '%s'\n",
                    SCRATCH_PAD_FILE_NAME);

            filp_close(fp, NULL);
            set_fs(fs);
        }
    }
    else
        printk("Could not allocate scratch pad memory.\n");

    return( ret );
}

// set scratch pad - write the scratch pad file
// return:
// 0 -- ok
// -1 -- fail
static int setScratchPad(char *buf, int len)
{
    /* Root file system is on a writable NAND flash.  Write PSI to
     * a file on the NAND flash.
     */
    int ret = -1;
    struct file *fp;
    mm_segment_t fs;

    fp = filp_open(SCRATCH_PAD_FILE_NAME, O_RDWR | O_TRUNC | O_CREAT,
        S_IRUSR | S_IWUSR);

    if (!IS_ERR(fp) && fp->f_op && fp->f_op->write)
    {
        fs = get_fs();
        set_fs(get_ds());

        fp->f_pos = 0;

        if((int) fp->f_op->write(fp, (void *) buf, len, &fp->f_pos) == len)
            ret = 0;
        else
            printk("Failed to write scratch pad to '%s'.\n", 
                SCRATCH_PAD_FILE_NAME);

        vfs_fsync(fp, fp->f_path.dentry, 0);
        filp_close(fp, NULL);
        set_fs(fs);
    }
    else
        printk("Unable to open '%s'.\n", SCRATCH_PAD_FILE_NAME);

    return( ret );
}

/*
 * get list of all keys/tokenID's in the scratch pad.
 * NOTE: memcpy work here -- not using copy_from/to_user
 *
 * return:
 *         greater than 0 means number of bytes copied to tokBuf,
 *         0 means fail,
 *         negative number means provided buffer is not big enough and the
 *         absolute value of the negative number is the number of bytes needed.
 */
int kerSysScratchPadList(char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int tokenNameLen=0;
    int copiedLen=0;
    int needLen=0;
    int sts = 0;

    BCM_ASSERT_NOT_HAS_MUTEX_R(&spMutex, 0);

    mutex_lock(&spMutex);

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
            mutex_unlock(&spMutex);
            return sts;
        }

        pBuf = pShareBuf;
    }
    else
    {
        if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            printk("could not getSharedBlks.\n");
            mutex_unlock(&spMutex);
            return sts;
        }

        // pBuf points to SP buf
        pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
    }

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        printk("Scratch pad is not initialized.\n");
        retriedKfree(pShareBuf);
        mutex_unlock(&spMutex);
        return sts;
    }

    // Walk through all the tokens
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;

    while( pToken->tokenName[0] != '\0' && pToken->tokenLen > 0 &&
           ((usedLen + pToken->tokenLen) <= fInfo.flash_scratch_pad_length))
    {
        tokenNameLen = strlen(pToken->tokenName);
        needLen += tokenNameLen + 1;
        if (needLen <= bufLen)
        {
            strcpy(&tokBuf[copiedLen], pToken->tokenName);
            copiedLen += tokenNameLen + 1;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    if ( needLen > bufLen )
    {
        // User may purposely pass in a 0 length buffer just to get
        // the size, so don't log this as an error.
        sts = needLen * (-1);
    }
    else
    {
        sts = copiedLen;
    }

    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    return sts;
}

/*
 * get sp data.  NOTE: memcpy work here -- not using copy_from/to_user
 * return:
 *         greater than 0 means number of bytes copied to tokBuf,
 *         0 means fail,
 *         negative number means provided buffer is not big enough and the
 *         absolute value of the negative number is the number of bytes needed.
 */
int kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int sts = 0;

    mutex_lock(&spMutex);

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL) {
            mutex_unlock(&spMutex);
            return sts;
        }

        pBuf = pShareBuf;
    }
    else
    {
        if( (pShareBuf = getSharedBlks(fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            printk("could not getSharedBlks.\n");
            mutex_unlock(&spMutex);
            return sts;
        }

        // pBuf points to SP buf
        pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;
    }

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        printk("Scratch pad is not initialized.\n");
        retriedKfree(pShareBuf);
        mutex_unlock(&spMutex);
        return sts;
    }

    // search for the token
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;
    while( pToken->tokenName[0] != '\0' && pToken->tokenLen > 0 &&
        pToken->tokenLen < fInfo.flash_scratch_pad_length &&
        usedLen < fInfo.flash_scratch_pad_length )
    {

        if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
        {
            if ( pToken->tokenLen > bufLen )
            {
               // User may purposely pass in a 0 length buffer just to get
               // the size, so don't log this as an error.
               // printk("The length %d of token %s is greater than buffer len %d.\n", pToken->tokenLen, pToken->tokenName, bufLen);
                sts = pToken->tokenLen * (-1);
            }
            else
            {
                memcpy(tokBuf, startPtr + sizeof(SP_TOKEN), pToken->tokenLen);
                sts = pToken->tokenLen;
            }
            break;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    return sts;
}

// set sp.  NOTE: memcpy work here -- not using copy_from/to_user
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
    SP_HEADER SPHead;
    SP_TOKEN SPToken;
    char *curPtr;
    int sts = -1;

    if( bufLen >= fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
        sizeof(SP_TOKEN) )
    {
        printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
            bufLen  - fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
            sizeof(SP_TOKEN));
        return sts;
    }

    mutex_lock(&spMutex);

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        pBuf = pShareBuf;
    }
    else
    {
        if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        // pBuf points to SP buf
        pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  
    }

    // form header info.
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, TOKEN_NAME_LEN - 1);
    SPToken.tokenLen = bufLen;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        printk("No scratch pad found.  Initialize scratch pad...\n");
        memcpy(pBuf, (char *)&SPHead, sizeof(SP_HEADER));
        curPtr = pBuf + sizeof(SP_HEADER);
        memcpy(curPtr, (char *)&SPToken, sizeof(SP_TOKEN));
        curPtr += sizeof(SP_TOKEN);
        if( tokBuf )
            memcpy(curPtr, tokBuf, bufLen);
    }
    else  
    {
        int putAtEnd = 1;
        int curLen;
        int usedLen;
        int skipLen;

        /* Calculate the used length. */
        usedLen = sizeof(SP_HEADER);
        curPtr = pBuf + sizeof(SP_HEADER);
        pToken = (PSP_TOKEN) curPtr;
        skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        while( pToken->tokenName[0] >= 'A' && pToken->tokenName[0] <= 'z' &&
            strlen(pToken->tokenName) < TOKEN_NAME_LEN &&
            pToken->tokenLen > 0 &&
            pToken->tokenLen < fInfo.flash_scratch_pad_length &&
            usedLen < fInfo.flash_scratch_pad_length )
        {
            usedLen += sizeof(SP_TOKEN) + skipLen;
            curPtr += sizeof(SP_TOKEN) + skipLen;
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        }

        if( usedLen + SPToken.tokenLen + sizeof(SP_TOKEN) >
            fInfo.flash_scratch_pad_length )
        {
            printk("Scratch pad overflow by %d bytes.  Information not saved.\n",
                (usedLen + SPToken.tokenLen + sizeof(SP_TOKEN)) -
                fInfo.flash_scratch_pad_length);
            mutex_unlock(&spMutex);
            return sts;
        }

        curPtr = pBuf + sizeof(SP_HEADER);
        curLen = sizeof(SP_HEADER);
        while( curLen < usedLen )
        {
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
            if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
            {
                // The token id already exists.
                if( tokBuf && pToken->tokenLen == bufLen )
                {
                    // The length of the new data and the existing data is the
                    // same.  Overwrite the existing data.
                    memcpy((curPtr+sizeof(SP_TOKEN)), tokBuf, bufLen);
                    putAtEnd = 0;
                }
                else
                {
                    // The length of the new data and the existing data is
                    // different.  Shift the rest of the scratch pad to this
                    // token's location and put this token's data at the end.
                    char *nextPtr = curPtr + sizeof(SP_TOKEN) + skipLen;
                    int copyLen = usedLen - (curLen+sizeof(SP_TOKEN) + skipLen);
                    memcpy( curPtr, nextPtr, copyLen );
                    memset( curPtr + copyLen, 0x00, 
                        fInfo.flash_scratch_pad_length - (curLen + copyLen) );
                    usedLen -= sizeof(SP_TOKEN) + skipLen;
                }
                break;
            }

            // get next token
            curPtr += sizeof(SP_TOKEN) + skipLen;
            curLen += sizeof(SP_TOKEN) + skipLen;
        } // end while

        if( putAtEnd )
        {
            if( tokBuf )
            {
                memcpy( pBuf + usedLen, &SPToken, sizeof(SP_TOKEN) );
                memcpy( pBuf + usedLen + sizeof(SP_TOKEN), tokBuf, bufLen );
            }
            memcpy( pBuf, &SPHead, sizeof(SP_HEADER) );
        }

    } // else if not new sp

    if( bootFromNand )
        sts = setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
    else
        sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk, 
            fInfo.flash_scratch_pad_number_blk, pShareBuf);
    
    retriedKfree(pShareBuf);

    mutex_unlock(&spMutex);

    return sts;

    
}

#if defined(AEI_VDSL_CUSTOMER_NCS)
int kerClearScratchPad(int blk_size)
{
    char buf[256];

    memset(buf, 0, 256);

#if defined(AEI_CONFIG_JFFS)
    if(bootFromNand)
#else
    if(!bootFromNand)
#endif
    {
        kerSysPersistentSet( buf, 256, 0 );
#ifdef SUPPORT_BACKUP_PSI
        kerSysBackupPsiSet( buf, 256, 0 );
#endif

        kerSysScratchPadClearAll();
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
        unsigned char boardid[NVRAM_BOARD_ID_STRING_LEN] = {0};

        kerSysGetBoardID(boardid);

        if(strstr(boardid, "C2000"))
           restoreDatapump(0);
        else if (strstr(boardid, "C1000"))
           restoreDatapump(2);

#else  // AEI_CONFIG_JFFS
#if defined(AEI_VDSL_CUSTOMER_Q2000H)      
        restoreDatapump(2);       
#else
        restoreDatapump(0);
#endif
#endif 
    }
}
#endif

// wipe out the scratchPad
// return:
//  0 - ok
//  -1 - fail
int kerSysScratchPadClearAll(void)
{ 
    int sts = -1;
    char *pShareBuf = NULL;
   int j ;
   int usedBlkSize = 0;

#if defined(AEI_VDSL_UPGRADE_HISTORY_SPAD)
	/* get upgradeHistroy */
    char uph[1024] = {0};
#if defined(AEI_CONFIG_JFFS)
	if(bootFromNand)
#else
	if(!bootFromNand)
#endif
    {
        kerSysScratchPadGet("upgradeHistory", uph, 1024);
    }
#endif      

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
	typedef struct {
	    char date[128];
	    char time[128];
	}CfgSaveTime;
	CfgSaveTime savedTime;
	int ret = 0;
	ret=kerSysScratchPadGet("CfgSaveState", &savedTime, sizeof(CfgSaveTime) );
#endif

   // printk ("kerSysScratchPadClearAll.... \n") ;
   mutex_lock(&spMutex);

    if( bootFromNand )
    {
        if((pShareBuf = getScratchPad(fInfo.flash_scratch_pad_length)) == NULL)
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        memset(pShareBuf, 0x00, fInfo.flash_scratch_pad_length);

        setScratchPad(pShareBuf, fInfo.flash_scratch_pad_length);
    }
    else
    {
        if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
            fInfo.flash_scratch_pad_number_blk)) == NULL )
        {
            mutex_unlock(&spMutex);
            return sts;
        }

        if (fInfo.flash_scratch_pad_number_blk == 1)
            memset(pShareBuf + fInfo.flash_scratch_pad_blk_offset, 0x00, fInfo.flash_scratch_pad_length) ;
        else
        {
            for (j = fInfo.flash_scratch_pad_start_blk;
                j < (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk);
                j++)
            {
                usedBlkSize += flash_get_sector_size((unsigned short) j);
            }

            memset(pShareBuf, 0x00, usedBlkSize) ;
        }

        sts = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
            fInfo.flash_scratch_pad_number_blk,  pShareBuf);
    }

   retriedKfree(pShareBuf);

   mutex_unlock(&spMutex);

#if defined(AEI_VDSL_UPGRADE_HISTORY_SPAD)
    /* save upgradeHistory back to scratch */
#if defined(AEI_CONFIG_JFFS)
	if(bootFromNand)
#else
    if(!bootFromNand)
#endif
	{
        kerSysScratchPadSet("upgradeHistory", uph, 1024);
    }	
#endif		

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
	if ( ret )
	{	
	  kerSysScratchPadSet("CfgSaveState", &savedTime, sizeof(CfgSaveTime) );
	}
#endif

   //printk ("kerSysScratchPadClearAll Done.... \n") ;
   return sts;
}

int kerSysFlashSizeGet(void)
{
    int ret = 0;

    if( bootFromNand )
    {
        struct mtd_info *mtd;

        if( (mtd = get_mtd_device_nm("rootfs")) != NULL )
        {
            ret = mtd->size;
            put_mtd_device(mtd);
        }
    }
    else
        ret = flash_get_total_size();

    return ret;
}

/***********************************************************************
 * Function Name: kerSysSetBootImageState
 * Description  : Persistently sets the state of an image update.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int kerSysSetBootImageState( int state )
{
    int ret = -1;
    char *pShareBuf = NULL;
    int seqNumUpdatePart = -1;
    int writeImageState = 0;
    int seq1 = kerSysGetSequenceNumber(1);
    int seq2 = kerSysGetSequenceNumber(2);

    /* Update the image state persistently using "new image" and "old image"
     * states.  Convert "partition" states to "new image" state for
     * compatibility with the non-OMCI image update.
     */
    mutex_lock(&spMutex);
    if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        fInfo.flash_scratch_pad_number_blk)) != NULL )
    {
        PSP_HEADER pHdr = (PSP_HEADER) pShareBuf;
        unsigned long *pBootImgState=(unsigned long *)&pHdr->NvramData2[0];

        switch(state)
        {
        case BOOT_SET_PART1_IMAGE:
            if( seq1 != -1 )
            {
                if( seq1 < seq2 )
                    seqNumUpdatePart = 1;
                state = BOOT_SET_NEW_IMAGE;
                writeImageState = 1;
            }
            break;

        case BOOT_SET_PART2_IMAGE:
            if( seq2 != -1 )
            {
                if( seq2 < seq1 )
                    seqNumUpdatePart = 2;
                state = BOOT_SET_NEW_IMAGE;
                writeImageState = 1;
            }
            break;

        case BOOT_SET_PART1_IMAGE_ONCE:
            if( seq1 != -1 )
            {
                if( seq1 < seq2 )
                    seqNumUpdatePart = 1;
                state = BOOT_SET_NEW_IMAGE_ONCE;
                writeImageState = 1;
            }
            break;

        case BOOT_SET_PART2_IMAGE_ONCE:
            if( seq2 != -1 )
            {
                if( seq2 < seq1 )
                    seqNumUpdatePart = 2;
                state = BOOT_SET_NEW_IMAGE_ONCE;
                writeImageState = 1;
            }
            break;

        case BOOT_SET_OLD_IMAGE:
        case BOOT_SET_NEW_IMAGE:
        case BOOT_SET_NEW_IMAGE_ONCE:
            /* The boot image state is stored as a word in flash memory where
             * the most significant three bytes are a "magic number" and the
             * least significant byte is the state constant.
             */
            if((*pBootImgState & 0xffffff00) == (BLPARMS_MAGIC & 0xffffff00) &&
                (*pBootImgState & 0x000000ff) == (state & 0x000000ff))
            {
                ret = 0;
            }
            else
            {
                *pBootImgState = (BLPARMS_MAGIC & 0xffffff00);
                *pBootImgState |= (state & 0x000000ff);
                writeImageState = 1;

                if( state == BOOT_SET_NEW_IMAGE &&
                    (*pBootImgState & 0x000000ff) == BOOT_SET_OLD_IMAGE )
                {
                    /* The old (previous) image is being set as the new
                     * (current) image. Make sequence number of the old
                     * image the highest sequence number in order for it
                     * to become the new image.
                     */
#if defined(AEI_VDSL_CUSTOMER_NCS)
                    seqNumUpdatePart = -1;
#else
                    seqNumUpdatePart = 0;
#endif
                }
            }
            break;

        default:
            break;
        }

        if( writeImageState )
        {
            *pBootImgState = (BLPARMS_MAGIC & 0xffffff00);
            *pBootImgState |= (state & 0x000000ff);

            ret = setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
                fInfo.flash_scratch_pad_number_blk,  pShareBuf);
        }

        mutex_unlock(&spMutex);
        retriedKfree(pShareBuf);

        if( seqNumUpdatePart != -1 )
        {
            PFILE_TAG pTag;
            int blk;

            mutex_lock(&flashImageMutex);
            pTag = kerSysUpdateTagSequenceNumber(seqNumUpdatePart);
            blk = *(int *) (pTag + 1);

            if ((pShareBuf = getSharedBlks(blk, 1)) != NULL)
            {
                memcpy(pShareBuf, pTag, sizeof(FILE_TAG));
                setSharedBlks(blk, 1, pShareBuf);
                retriedKfree(pShareBuf);
            }
            mutex_unlock(&flashImageMutex);
        }
    }
    else
    {
        // getSharedBlks failed, release mutex
        mutex_unlock(&spMutex);
    }

    return( ret );
}

/***********************************************************************
 * Function Name: kerSysGetBootImageState
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int kerSysGetBootImageState( void )
{
    int ret = -1;
    char *pShareBuf = NULL;

    if( (pShareBuf = getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        fInfo.flash_scratch_pad_number_blk)) != NULL )
    {
        PSP_HEADER pHdr = (PSP_HEADER) pShareBuf;
        unsigned long *pBootImgState=(unsigned long *)&pHdr->NvramData2[0];

        /* The boot image state is stored as a word in flash memory where
         * the most significant three bytes are a "magic number" and the
         * least significant byte is the state constant.
         */
        if( (*pBootImgState & 0xffffff00) == (BLPARMS_MAGIC & 0xffffff00) )
        {
            int seq1 = kerSysGetSequenceNumber(1);
            int seq2 = kerSysGetSequenceNumber(2);

            switch(ret = (*pBootImgState & 0x000000ff))
            {
            case BOOT_SET_NEW_IMAGE:
                if( seq1 == -1 || seq1< seq2 )
                    ret = BOOT_SET_PART2_IMAGE;
                else
                    ret = BOOT_SET_PART1_IMAGE;
                break;

            case BOOT_SET_NEW_IMAGE_ONCE:
                if( seq1 == -1 || seq1< seq2 )
                    ret = BOOT_SET_PART2_IMAGE_ONCE;
                else
                    ret = BOOT_SET_PART1_IMAGE_ONCE;
                break;

            case BOOT_SET_OLD_IMAGE:
                if( seq1 == -1 || seq1> seq2 )
                    ret = BOOT_SET_PART2_IMAGE;
                else
                    ret = BOOT_SET_PART1_IMAGE;
                break;
                break;

            default:
                ret = -1;
                break;
            }
        }

        retriedKfree(pShareBuf);
    }

    return( ret );
}

#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
static void AEI_reportTftpUpgradeStat(int partition)
{
        struct file     *f;

        long int        rv;
        mm_segment_t    old_fs;
        loff_t          offset=0;
        char buf[128] = "";

        sprintf(buf, "%d\n", partition);
        f = filp_open("/var/tftpupgradedone", O_RDWR|O_CREAT|O_TRUNC, 0600);
        if(f == NULL)
                return;

        old_fs = get_fs();
        set_fs( get_ds() );
        rv = f->f_op->write(f, buf, strlen(buf), &offset);
        set_fs(old_fs);

        if (rv < strlen(buf)) {
                printk("write /var/tftpupgradedone  error\n");
        }
        filp_close(f , 0);
        return;
}
/*
 * nandUpdateSeqNum
 * 
 * Read the sequence number from each rootfs partition.  The sequence number is
 * the extension on the cferam file.  Add one to the highest sequence number
 * and change the extenstion of the cferam in the image to be flashed to that
 * number.
 */
static int AEI_nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen, int Partition)
{
    char fname[] = NAND_CFE_RAM_NAME;
    int fname_actual_len = strlen(fname);
    int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
    char cferam_base[32], cferam_buf[32], cferam_fmt[32]; 
    int i;
    struct file *fp;
    int seq = -1;
    int ret = 1;


    strcpy(cferam_base, fname);
    cferam_base[fname_cmp_len] = '\0';
    strcpy(cferam_fmt, cferam_base);
    strcat(cferam_fmt, "%3.3d");

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
                /*Seq number found. Take the greater of the two seq numbers.*/
                if( seq < i )
                    seq = i;
                break;
            }
        }
    }

    if( seq != -1 )
    {
        unsigned char *buf, *p;
        int len = blkLen;
        struct jffs2_raw_dirent *pdir;
        unsigned long version = 0;
        int done = 0;

        if( *(unsigned short *) imagePtr != JFFS2_MAGIC_BITMASK )
        {
            imagePtr += len;
            imageSize -= len;
        }


        /* Increment the new highest sequence number. Add it to the CFE RAM
         * file name.
         */
        seq += (3 - Partition);


        /* Search the image and replace the last three characters of file
         * cferam.000 with the new sequence number.
         */
        for(buf = imagePtr; buf < imagePtr+imageSize && done == 0; buf += len)
        {
            p = buf;
            while( p < buf + len )
            {
                pdir = (struct jffs2_raw_dirent *) p;
                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                {
                    if( je16_to_cpu(pdir->nodetype) == JFFS2_NODETYPE_DIRENT &&
                        fname_actual_len == pdir->nsize &&
                        !memcmp(fname, pdir->name, fname_cmp_len) &&
                        je32_to_cpu(pdir->version) > version &&
                        je32_to_cpu(pdir->ino) != 0 )
                     {
                        /* File cferam.000 found. Change the extension to the
                         * new sequence number and recalculate file name CRC.
                         */
                        p = pdir->name + fname_cmp_len;
                        p[0] = (seq / 100) + '0';
                        p[1] = ((seq % 100) / 10) + '0';
                        p[2] = ((seq % 100) % 10) + '0';
                        p[3] = '\0';

                        je32_to_cpu(pdir->name_crc) =
                            crc32(0, pdir->name, (unsigned int)
                            fname_actual_len);

                        version = je32_to_cpu(pdir->version);

                        /* Setting 'done = 1' assumes there is only one version
                         * of the directory entry.
                         */
                        done = 1;
                        ret = (buf - imagePtr) / len; /* block number */
                        break;
                    }

                    p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                }
                else
                {
                    done = 1;
                    break;
                }
            }
        }
    }

    return(ret);
}


// NAND flash bcm image 
// return: 
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
int AEI_nandImageSet( int partition, char *string, int img_size )
{
    /* Allow room to flash cferam sequence number at start of file system. */
    const int fs_start_blk_num = 8;

    int sts = -1;
    int blk = 0;
    int cferam_blk;
    int fs_start_blk;
    int ofs;
    int old_img = 0;
    char *cferam_string;
    char *end_string = string + img_size;
    struct mtd_info *mtd0 = NULL;
    struct mtd_info *mtd1 = get_mtd_device_nm("nvram");
    WFI_TAG wt = {0};

	char * cferombuf=NULL;
	char * tagbuf=NULL;
    struct mtd_info *mtd2 = NULL;

    if( mtd1 )
    {
        int blksize = mtd1->erasesize / 1024;

        memcpy(&wt, end_string, sizeof(wt));
        if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            ((blksize == 16 && wt.wfiFlashType != WFI_NAND16_FLASH) ||
             (blksize < 128 && wt.wfiFlashType == WFI_NAND128_FLASH)) )
        {
            printk("\nERROR: NAND flash block size %dKB does not work with an "
                "image built with %dKB block size\n\n", blksize,
                (wt.wfiFlashType == WFI_NAND16_FLASH) ? 16 : 128);
        }
        else
        {
			unsigned long rootfs_ofs;
			int iRootfs=NP_ROOTFS_1;
            mtd0 = get_mtd_device_nm("rootfs_update");


            kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);


            if(partition == NP_ROOTFS_2)
            {
                printk("flashing image to second partition.\n");
                iRootfs=NP_ROOTFS_2;
            }
            else
            {
                printk("flashing image to first partition.\n");
                iRootfs=NP_ROOTFS_1;
            }

            if(rootfs_ofs == inMemNvramData.ulNandPartOfsKb[iRootfs] && mtd0)
            {
                put_mtd_device(mtd0);
                mtd0 = NULL;
            }

            if( mtd0 == NULL || mtd0->size == 0LL )
            {
                /* Flash device is configured to use only one file system. */
                if( mtd0 )
                    put_mtd_device(mtd0);
                mtd0 = get_mtd_device_nm("rootfs");
                #ifdef AEI_CONFIG_JFFS
                mtd2 = get_mtd_device_nm("tag");
                #endif
            }
            else
            {
                mtd2 = get_mtd_device_nm("tag_update");
            }

        }
    }

    if( mtd0 && mtd1 )
    {
        unsigned long flags;
        int retlen = 0;

        if( *(unsigned short *) string == JFFS2_MAGIC_BITMASK || *(unsigned short *) string == AEI_MAGIC_BITMASK )
            /* Image only contains file system. */
            ofs = 0; /* use entire string image to find sequence number */
        else
        {
            /* Image contains CFE ROM boot loader. */
            PNVRAM_DATA pnd = (PNVRAM_DATA) (string + NVRAM_DATA_OFFSET);

            /* skip block 0 to find sequence number */
            switch(wt.wfiFlashType)
            {
            case WFI_NAND16_FLASH:
                ofs = 16 * 1024;
                break;

            case WFI_NAND128_FLASH:
                ofs = 128 * 1024;
                break;
            }

            /* Copy NVRAM data to block to be flashed so it is preserved. */
            spin_lock_irqsave(&inMemNvramData_spinlock, flags);
            if (NULL !=(cferombuf = kmalloc(mtd1->erasesize, GFP_KERNEL)) )
            {
                memset(cferombuf,0,mtd1->erasesize);
                memcpy((unsigned char *) cferombuf, string,
						mtd1->erasesize);
                pnd = (PNVRAM_DATA) (cferombuf + NVRAM_DATA_OFFSET);
                memcpy((unsigned char *) pnd, inMemNvramData_buf,
									sizeof(NVRAM_DATA));
            }
            else
                memcpy((unsigned char *) pnd, inMemNvramData_buf,
                    sizeof(NVRAM_DATA));
            spin_unlock_irqrestore(&inMemNvramData_spinlock, flags);

            /* Recalculate the nvramData CRC. */
            pnd->ulCheckSum = 0;
            pnd->ulCheckSum = crc32(CRC32_INIT_VALUE, pnd, sizeof(NVRAM_DATA));
        }

        /* Update the sequence number that replaces that extension in file
         * cferam.000
         */
        cferam_blk = AEI_nandUpdateSeqNum((unsigned char *) string + ofs,
            img_size - ofs, mtd0->erasesize, partition) * mtd0->erasesize;

        cferam_string = string + ofs + cferam_blk;

        fs_start_blk = fs_start_blk_num * mtd0->erasesize;

        local_irq_save(flags);
        local_bh_disable();

        if( *(unsigned short *) string != JFFS2_MAGIC_BITMASK &&  *(unsigned short *) string != AEI_MAGIC_BITMASK)
        {
            /* Flash the CFE ROM boot loader. */
            nandEraseBlkNotSpare( mtd1, 0 );
			if(cferombuf != NULL)
			{
                mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, cferombuf);
                kfree(cferombuf);
			}
			else
                mtd1->write(mtd1, 0, mtd1->erasesize, &retlen, string);
            string += ofs;
        }


        if((*(unsigned short *) string == JFFS2_MAGIC_BITMASK && *(unsigned short *) (string+2) == AEI_MAGIC_BITMASK) && mtd2!=NULL)
        {
        
            nandEraseBlkNotSpare( mtd2, 0 );
            if (NULL !=(tagbuf = kmalloc(mtd2->erasesize, GFP_KERNEL)) )
            {
                memset(tagbuf,0,mtd2->erasesize);
                memcpy((unsigned char *) tagbuf, string,
                    mtd2->erasesize);
            }
            else
            {
                printk("##Write tag start,mtd->name(%s)\n",mtd2->name);
                /* Flash the image header. */
                mtd2->write(mtd2, 0, mtd2->erasesize, &retlen, string);
            }
            string +=mtd2->erasesize ;
        }

        /* Erase block with sequence number before flashing the image. */
        for( blk = 0; blk < fs_start_blk; blk += mtd0->erasesize )
        {
            nandEraseBlkNotSpare( mtd0, blk );
        }

        /* Flash the image except for the part with the sequence number. */
        for( blk = fs_start_blk; blk < mtd0->size; blk += mtd0->erasesize )
        {

            if( (sts = nandEraseBlkNotSpare( mtd0, blk )) == 0 )
            {
                /* Write a block of the image to flash. */
                if( string < end_string && string != cferam_string )
                {
                    int writelen = ((string + mtd0->erasesize) <= end_string)
                        ? mtd0->erasesize : (int) (end_string - string);

                    mtd0->write(mtd0, blk, writelen, &retlen, string);
                    if( retlen == writelen )
                    {
                        printk(".");
                        string += writelen;
                    }
                }
                else
                    string += mtd0->erasesize;

            }

        }

        /* Flash the image part with the sequence number. */
        for( blk = 0; blk < fs_start_blk; blk += mtd0->erasesize )
        {

            if( (sts = nandEraseBlkNotSpare( mtd0, blk )) == 0 )
            {
                /* Write a block of the image to flash. */
                if( cferam_string )
                {
                    mtd0->write(mtd0, blk, mtd0->erasesize,
                        &retlen, cferam_string);

                    if( retlen == mtd0->erasesize )
                    {
                        printk(".");
                        cferam_string = NULL;
                        break;
                    }
                }
            }

        }

        if(tagbuf != NULL)
        {
            printk("##Write tag start,mtd->name(%s)\n",mtd2->name);
            nandEraseBlkNotSpare( mtd2, 0 );
            mtd2->write(mtd2, 0, mtd2->erasesize, &retlen, tagbuf);
            kfree(tagbuf);
        }

        if (sts)
        {
            sts = (blk > mtd0->erasesize) ? blk / mtd0->erasesize : 1;
        }
        local_irq_restore(flags);
        local_bh_enable();

        printk("\n\n");
    }
    if( mtd0 )
        put_mtd_device(mtd0);

    if( mtd1 )
        put_mtd_device(mtd1);

    if( mtd2 )
        put_mtd_device(mtd2);
   
    return sts;
}

int AEI_kerSysBcmImageSet( int partition, char *string, int img_size )
{
    int ret = 0;
    int iPartition = 0;

    if(partition & 2)
    {
        ret |= AEI_nandImageSet(NP_ROOTFS_2, string, img_size);
        if(ret==0)
        {
            iPartition |= 2;
        }
    }
    if((partition & 1) && (ret==0))
    {
        ret |= AEI_nandImageSet(NP_ROOTFS_1, string, img_size);
        if(ret==0)
        {
            iPartition |= 1;
        }
    }

    AEI_reportTftpUpgradeStat(iPartition);

    return ret;
}

#endif

