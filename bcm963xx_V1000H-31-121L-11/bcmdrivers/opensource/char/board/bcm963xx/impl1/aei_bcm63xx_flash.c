#ifdef AEI_CONFIG_JFFS
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

#include <board.h>
#include <bcmTag.h>
#include "flash_api.h"
#include "63268_map.h"  


typedef struct CfeNandChip
{
    char *chip_name;
    unsigned long chip_device_id;
    unsigned long chip_base;
    unsigned long chip_total_size;
    unsigned long chip_block_size;
    unsigned long chip_page_size;
    unsigned long chip_spare_size;
    unsigned long chip_spare_step_size;
    unsigned char *chip_spare_mask;
    unsigned long chip_bi_index_1;
    unsigned long chip_bi_index_2;
} CFE_NAND_CHIP, *PCFE_NAND_CHIP;

struct flash_name_from_id
{
    unsigned short fnfi_id;
    char fnfi_name[30];
};


typedef struct SpareLayout
{
    unsigned char sl_bi_ofs[2];
    unsigned char sl_spare_mask[];
} SPARE_LAYOUT, *PSPARE_LAYOUT;

static flash_device_info_t flash_nand_dev =
    {
        0xffff,
        FLASH_IFC_NAND,
        "",
        NULL,
        AEI_nand_flash_read_buf,
        AEI_nand_flash_write_buf,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

/* B,B,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 */
SPARE_LAYOUT brcmnand_oob_64 =
    {{0, 1}, {0xc3, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01}};

/* 0,0,0,0,0,B,E,E-E,0,0,0,0,0,0,0 */
SPARE_LAYOUT brcmnand_oob_16 =
    {{5, 5}, {0xe0, 0x01}};

/* 0,0,0,0,0,B,0,0-0,E,E,E,E,E,E,E */
SPARE_LAYOUT brcmnand_oob_bch4_512 =
    {{5, 5}, {0x20, 0xfe}};

/* B,B,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 */
SPARE_LAYOUT brcmnand_oob_128 =
    {{0, 1}, {0xc3, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
              0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01}};

/* B,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch4_4k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe}};

/* B,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch4_2k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_2k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_4k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_4k =
    {{0, 0}, {0x01, 0xc0, 0xff, 0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff,
              0x01, 0x80, 0xff, 0x0f, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0xff,
              0x03, 0x00, 0xff, 0x1f, 0x00, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_4k =
    {{0, 0}, {0x81, 0xff, 0xff, 0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff,
              0x01, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0x7f, 0xc0, 0xff, 0xff,
              0x03, 0xfe, 0xff, 0x1f, 0xf0, 0xff, 0xff}};

/* B,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch4_8k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_8k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_8k =
    {{0, 0}, {0x01, 0xc0, 0xff, 0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff,
              0x01, 0x80, 0xff, 0x0f, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0xff,
              0x03, 0x00, 0xff, 0x1f, 0x00, 0xf8, 0xff, 0x00, 0xc0, 0xff,
              0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff, 0x01, 0x80, 0xff,
              0x0f, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0xff, 0x03, 0x00, 0xff,
              0x1f, 0x00, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_8k =
    {{0, 0}, {0x81, 0xff, 0xff, 0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff,
              0x01, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0x7f, 0xc0, 0xff, 0xff,
              0x03, 0xfe, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x81, 0xff, 0xff,
              0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff, 0x01, 0xff, 0xff,
              0x0f, 0xf8, 0xff, 0x7f, 0xc0, 0xff, 0xff, 0x03, 0xfe, 0xff,
              0x1f, 0xf0, 0xff, 0xff}};

#if defined(_BCM96368_) || defined(_BCM96816_) || defined(_BCM96362_) || defined(_BCM96328_)
/* Not used on non-BCM963268 chips but prevents compile errors. */
#define NC_BLK_SIZE_2048K       0x60000000
#define NC_BLK_SIZE_1024K       0x50000000
#define NC_BLK_SIZE_256K        0x40000000
#define NC_PG_SIZE_8K           0x00300000
#define NC_PG_SIZE_4K           0x00200000
#define NAC_FAST_PGM_RDIN       0x10000000
#define NAC_PARTIAL_PAGE_EN     0x04000000
#define NAC_ECC_LVL_0_SHIFT     20     
#define NAC_ECC_LVL_0_MASK      0x00f00000
#define NAC_ECC_LVL_SHIFT       16     
#define NAC_ECC_LVL_MASK        0x000f0000
#define NAC_SPARE_SZ_SHIFT      0
#define NAC_SPARE_SZ_MASK       0x0000003f
#define NAC_ECC_LVL_DISABLE     0
#define NAC_ECC_LVL_BCH_1       1
#define NAC_ECC_LVL_BCH_2       2
#define NAC_ECC_LVL_BCH_3       3
#define NAC_ECC_LVL_BCH_4       4
#define NAC_ECC_LVL_BCH_5       5
#define NAC_ECC_LVL_BCH_6       6
#define NAC_ECC_LVL_BCH_7       7
#define NAC_ECC_LVL_BCH_8       8
#define NAC_ECC_LVL_BCH_9       9
#define NAC_ECC_LVL_BCH_10      10
#define NAC_ECC_LVL_BCH_11      11
#define NAC_ECC_LVL_BCH_12      12
#define NAC_ECC_LVL_RESVD_1     13
#define NAC_ECC_LVL_RESVD_2     14
#define NAC_ECC_LVL_HAMMING     15
#endif



/** Defines. **/

#define SPARE_MAX_SIZE          (27 * 16)
#define CTRLR_CACHE_SIZE        512

#define NAND_CI_CELLTYPE_MSK    0x00000f00
#define NAND_IS_MLC(chip)       ((chip)->chip_device_id & NAND_CI_CELLTYPE_MSK)
#define NAND_CHIPID(chip)       ((chip)->chip_device_id >> 16)             

/* Flash manufacturers. */
#define FLASHTYPE_SAMSUNG       0xec
#define FLASHTYPE_ST            0x20
#define FLASHTYPE_MICRON        0x2c

/* Samsung flash parts. */
#define SAMSUNG_K9F5608U0A      0x55

/* ST flash parts. */
#define ST_NAND512W3A2CN6       0x76
#define ST_NAND01GW3B2CN6       0xf1

/* Micron flash parts. */
#define MICRON_MT29F1G08AAC     0xf1

/* Flash id to name mapping. */
#define NAND_MAKE_ID(A,B)    \
    (((unsigned short) (A) << 8) | ((unsigned short) B & 0xff))

#define NAND_FLASH_DEVICES                                                    \
  {{NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F5608U0A),"Samsung K9F5608U0"},  \
   {NAND_MAKE_ID(FLASHTYPE_ST,ST_NAND512W3A2CN6),"ST NAND512W3A2CN6"},        \
   {NAND_MAKE_ID(FLASHTYPE_ST,ST_NAND01GW3B2CN6),"ST NAND01GW3B2CN6"},        \
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F1G08AAC),"Micron MT29F1G08AAC"},\
   {0,""}                                                                     \
  }

/* Condition to determine the spare layout. */
#define LAYOUT_PARMS(L,S,P)     \
    (((unsigned long)(L)<<28) | ((unsigned long)(S)<<16) | (P))

/* Each bit in the ECCMSK array represents a spare area byte. Bits that are
 * set correspond to spare area bytes that are reserved for the ECC or bad
 * block indicator. Bits that are not set can be used for data such as the
 * JFFS2 clean marker. This macro returns 0 if the spare area byte at offset,
 * OFS, is available and non-0 if it is being used for the ECC or BI.
 */
#define ECC_MASK_BIT(ECCMSK, OFS)   (ECCMSK[OFS / 8] & (1 << (OFS % 8)))

#define SPARE_BI_MARKER         0

#define JFFS2_CLEANMARKER      {JFFS2_MAGIC_BITMASK, \
    JFFS2_NODETYPE_CLEANMARKER, 0x0000, 0x0008}

#undef DEBUG_NAND
#if defined(DEBUG_NAND) && defined(CFG_RAMAPP)
#define DBG_PRINTF printk
#else
#define DBG_PRINTF(...)
#endif
/** Variables for CFE RAM. **/
CFE_NAND_CHIP g_chip = {NULL,0,0,0,0,0,0};
static unsigned char g_spare_cleanmarker[SPARE_MAX_SIZE];
static int AEI_nandflash_block_erase(PCFE_NAND_CHIP pchip, unsigned long blk_addr);
static int AEI_nandflash_write_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len);

 /***************************************************************************/
static int AEI_nandflash_wait_status(unsigned long status_mask)
{

    const unsigned long nand_poll_max = 1000000;
    unsigned long data;
    unsigned long poll_count = 0;
    int ret = FLASH_API_OK;

    do
    {
        data = NAND->NandIntfcStatus;
    } while(!(status_mask & data) && (++poll_count < nand_poll_max));

    if(poll_count >= nand_poll_max)
    {
        printk("Status wait timeout: nandsts=0x%8.8lx mask=0x%8.8lx, count="
            "%lu\n", NAND->NandIntfcStatus, status_mask, poll_count);
        ret = FLASH_API_ERROR;
    }

    return( ret );
} /* nandflash_wait_status */

static inline int AEI_nandflash_wait_cmd(void)
{
    return AEI_nandflash_wait_status(NIS_CTLR_READY);
} /* nandflash_wait_cmd */

static inline int AEI_nandflash_wait_cache(void)
{
    return AEI_nandflash_wait_status(NIS_CACHE_VALID);
} /* nandflash_wait_cache */

static inline int AEI_nandflash_wait_spare(void)
{
    return AEI_nandflash_wait_status(NIS_SPARE_VALID);
} /* nandflash_wait_spare */

/***************************************************************************
 * Function Name: nandflash_check_ecc
 * Description  : Reads ECC status.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nandflash_check_ecc(void)
{
    int ret = FLASH_API_OK;
    UINT32 intrCtrl;
    UINT32 accessCtrl;

    /* read interrupt status */
    intrCtrl = NAND_INTR->NandInterrupt;
    accessCtrl = NAND->NandAccControl;


    if( (intrCtrl & NINT_ECC_ERROR_UNC) != 0 )
        ret = FLASH_API_ERROR;

    if( (intrCtrl & NINT_ECC_ERROR_CORR) != 0 )
    {
        printk("Correctable ECC Error detected: addr=0x%8.8lx, intrCtrl=0x"
            "%08X, accessCtrl=0x%08X\n", NAND->NandEccCorrAddr, (UINT)intrCtrl,
            (UINT)accessCtrl);
    }

    return( ret );
}
/***************************************************************************
 * Function Name: nandflash_copy_from_spare
 * Description  : Copies data from the chip NAND spare registers to a local
 *                memory buffer.
 * Returns      : None.
 ***************************************************************************/
static inline void AEI_nandflash_copy_from_spare(unsigned char *buffer,
    int numbytes)
{
    unsigned long *spare_area = (unsigned long *) &NAND->NandSpareAreaReadOfs0;
    unsigned long *pbuff = (unsigned long *)buffer;
    unsigned long i;

    /* TBD. Support 27 byte spare area. */
    if( numbytes > 16 )
        numbytes = 16;

    numbytes /= sizeof(long);
    for(i=0; i< numbytes; ++i)
         pbuff[i] = spare_area[i];
} /* nandflash_copy_from_spare */

/***************************************************************************
 * Function Name: nandflash_copy_to_cache
 * Description  : Copies data from a local memory buffer to the the chip NAND
 *                cache.
 * Returns      : None.
 ***************************************************************************/
static inline void AEI_nandflash_copy_to_cache(unsigned char *buffer, int offset,
    int numbytes)
{
    unsigned char *cache = (unsigned char *) NAND_CACHE;
    unsigned long i;

    for( i = 0; i < numbytes; i += sizeof(long) )
        *(unsigned long *) &cache[i] =
            ((unsigned long) buffer[i + 0] << 24) |
            ((unsigned long) buffer[i + 1] << 16) |
            ((unsigned long) buffer[i + 2] <<  8) |
            ((unsigned long) buffer[i + 3] <<  0);
} /* nandflash_copy_to_cache */


/***************************************************************************
 * Function Name: nandflash_copy_to_spare
 * Description  : Copies data from a local memory buffer to the the chip NAND
 *                spare registers.
 * Returns      : None.
 ***************************************************************************/
static inline void AEI_nandflash_copy_to_spare(unsigned char *buffer,int numbytes)
{
    unsigned long *spare_area = (unsigned long *) &NAND->NandSpareAreaWriteOfs0;
    unsigned long *pbuff = (unsigned long *)buffer;
    int i;

    /* TBD. Support 27 byte spare area. */
    if( numbytes > 16 )
        numbytes = 16;

    for(i=0; i< numbytes / sizeof(unsigned long); ++i)
        spare_area[i] = pbuff[i];
} /* nandflash_copy_to_spare */

/***************************************************************************
 * Function Name: nandflash_read_spare_area
 * Description  : Reads the spare area for the specified page.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nandflash_read_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

    if( len >= pchip->chip_spare_size )
    {
        UINT32 steps = pchip->chip_spare_size / pchip->chip_spare_step_size;
        UINT32 i;

        for( i = 0; i < steps; i++ )
        {
            NAND->NandCmdAddr = pchip->chip_base + page_addr +
                (i * CTRLR_CACHE_SIZE);
            NAND->NandCmdExtAddr = 0;
            NAND->NandCmdStart = NCMD_SPARE_READ;

            if( (ret = AEI_nandflash_wait_cmd()) == FLASH_API_OK )
            {
                /* wait until data is available in the spare area registers */
                if( (ret = AEI_nandflash_wait_spare()) == FLASH_API_OK )
                    AEI_nandflash_copy_from_spare(buffer +
                        (i * pchip->chip_spare_step_size),
                        pchip->chip_spare_step_size);
                else
                    break;
            }
            else
                break;
        }
    }

    return ret;
} /* nandflash_read_spare_area */

/***************************************************************************
 * Function Name: nand_mark_bad_blk
 * Description  : Marks the specified block as bad by writing 0xFFs to the
 *                spare area and updating the in memory bad block table.
 * Returns      : None.
 ***************************************************************************/
static void AEI_nand_mark_bad_blk(PCFE_NAND_CHIP pchip, unsigned long page_addr)
{
    static int marking_bad_blk = 0;

    unsigned char spare[SPARE_MAX_SIZE];

    if( marking_bad_blk == 0 )
    {
        marking_bad_blk = 1;
        DBG_PRINTF(">> nand_mark_bad_blk - addr=0x%8.8lx, block=0x%8.8lx\n",
            page_addr, page_addr / pchip->chip_block_size);

        AEI_nandflash_block_erase(pchip, page_addr);
        memset(spare, 0xff, pchip->chip_spare_size);
        spare[pchip->chip_bi_index_1] = SPARE_BI_MARKER;
        spare[pchip->chip_bi_index_2] = SPARE_BI_MARKER;
        AEI_nandflash_write_spare_area(pchip,page_addr,spare,pchip->chip_spare_size);
        marking_bad_blk = 0;
    }
} /* nand_mark_bad_blk */


/***************************************************************************
 * Function Name: nandflash_block_erase
 * Description  : Erases a block.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nandflash_block_erase(PCFE_NAND_CHIP pchip, unsigned long blk_addr )
{

    int ret = FLASH_API_OK;

    /* send command */
    NAND->NandCmdAddr = pchip->chip_base + blk_addr;
    NAND->NandCmdExtAddr = 0;
    NAND->NandCmdStart = NCMD_BLOCK_ERASE;
    if( (ret = AEI_nandflash_wait_cmd()) == FLASH_API_OK )
    {
        unsigned long sts = NAND->NandIntfcStatus;

        if( (sts & NIS_PGM_ERASE_ERROR) != 0 )
        {
            DBG_PRINTF("\nError erasing block 0x%8.8lx, sts=0x%8.8lx\n", \
                blk_addr, sts);
            AEI_nand_mark_bad_blk(pchip, blk_addr);
            ret = FLASH_API_ERROR;
        }
    }

    DBG_PRINTF(">> nandflash_block_erase - addr=0x%8.8lx, ret=%d\n", blk_addr,
        ret);

    return( ret );
} /* nandflash_block_erase */




/***************************************************************************
 * Function Name: nandflash_write_spare_area
 * Description  : Reads the spare area for the specified page.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nandflash_write_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len)
{
    int ret = FLASH_API_OK;
    unsigned char nand_cache[CTRLR_CACHE_SIZE];
    unsigned char spare[SPARE_MAX_SIZE];
    unsigned long is_mlc = NAND_IS_MLC(pchip);

    if( len <= pchip->chip_spare_size )
    {
        unsigned long acc = NAND->NandAccControl;
        unsigned long acc_save = acc;
                        
        UINT32 steps = pchip->chip_spare_size / pchip->chip_spare_step_size;
        UINT32 i;

        if( is_mlc )
            memset(nand_cache, 0xff, sizeof(nand_cache));

        memset(spare, 0xff, pchip->chip_spare_size);
        memcpy(spare, buffer, len);
        for( i = 0; i < steps; i++ )
        {
            NAND->NandCmdAddr = pchip->chip_base + page_addr +
                (i * CTRLR_CACHE_SIZE);
            NAND->NandCmdExtAddr = 0;

            if( is_mlc )
            {
                /* Disable ECC so 0xFFs are stored in the ECC offsets. Doing
                 * this allows the next page write to store the ECC correctly.
                 * If the ECC is not disabled here, then a ECC value will be
                 * stored at the ECC offsets.  This will cause the ECC value
                 * on the next write to be stored incorrectly.
                 */
                acc &= ~(NAC_ECC_LVL_0_MASK | NAC_ECC_LVL_MASK);
                acc |= (NAC_ECC_LVL_DISABLE << NAC_ECC_LVL_0_SHIFT) |
                       (NAC_ECC_LVL_DISABLE << NAC_ECC_LVL_SHIFT);

                NAND->NandAccControl = acc;

                /* MLC parts use PROGRAM_PAGE to program spare area. */
                AEI_nandflash_copy_to_cache(nand_cache, 0, sizeof(nand_cache));
                AEI_nandflash_copy_to_spare(spare+(i*pchip->chip_spare_step_size),
                    pchip->chip_spare_step_size);
                NAND->NandCmdStart = NCMD_PROGRAM_PAGE;
            }
            else
            {
                /* SLC parts use PROGRAM_SPARE to program spare area. */
                AEI_nandflash_copy_to_spare(spare+(i*pchip->chip_spare_step_size),
                    pchip->chip_spare_step_size);
                NAND->NandCmdStart = NCMD_PROGRAM_SPARE;
            }

            if( (ret = AEI_nandflash_wait_cmd()) == FLASH_API_OK )
            {
                unsigned long sts = NAND->NandIntfcStatus;

                if( (sts & NIS_PGM_ERASE_ERROR) != 0 )
                {
                    printk("Error writing to spare area, sts=0x%8.8lx\n", sts);
                    AEI_nand_mark_bad_blk(pchip, page_addr);
                    ret = FLASH_API_ERROR;
                }
            }

            if( is_mlc )
                NAND->NandAccControl = acc_save;
        }
    }
    else
        ret = FLASH_API_ERROR;
 
    return( ret );
} /* nandflash_write_spare_area */


/***************************************************************************
 * Function Name: nandflash_copy_from_cache
 * Description  : Copies data from the chip NAND cache to a local memory
 *                buffer.
 * Returns      : None.
 ***************************************************************************/
static inline void AEI_nandflash_copy_from_cache(unsigned char *buffer,
    int offset, int numbytes)
{
    unsigned long *cache = (unsigned long *) NAND_CACHE;
    unsigned long *pbuff = (unsigned long *)buffer;
    unsigned long i;

    numbytes /= sizeof(long);
    for(i=0; i< numbytes; ++i)
         pbuff[i] = cache[i];
} /* nandflash_copy_from_cache */

static int AEI_nand_is_blk_cleanmarker(PCFE_NAND_CHIP pchip,
    unsigned long start_addr, int write_if_not)
{
    unsigned short cleanmarker[] = JFFS2_CLEANMARKER;
    unsigned char *pcm = (unsigned char *) cleanmarker;
    unsigned char spare[SPARE_MAX_SIZE], comparebuf[SPARE_MAX_SIZE];
    unsigned char *spare_mask = pchip->chip_spare_mask;
    unsigned long i, j;
    int ret = 0;

    if( AEI_nandflash_read_spare_area( pchip, start_addr, spare,
        pchip->chip_spare_size) == FLASH_API_OK )
    {
        /* Skip spare offsets that are reserved for the ECC.  Make spare data
         * bytes contiguous in the spare buffer.
         */
        for( i = 0, j = 0; i < pchip->chip_spare_size; i++ )
            if( ECC_MASK_BIT(spare_mask, i) == 0 )
                comparebuf[j++] = spare[i];

        /* Compare spare area data to the JFFS2 cleanmarker. */
        for( i = 0, ret = 1; i < sizeof(cleanmarker) && ret == 1; i++ )
            if( comparebuf[i] != pcm[i])
                ret = 0;
    }

    if( ret == 0 && spare[pchip->chip_bi_index_1] != SPARE_BI_MARKER &&
        spare[pchip->chip_bi_index_2] != SPARE_BI_MARKER && write_if_not )
    {
        /* The spare area is not a clean marker but the block is not bad.
         * Write a clean marker to this block. (Assumes the block is erased.)
         */
        if( AEI_nandflash_write_spare_area(pchip, start_addr, (unsigned char *)
            g_spare_cleanmarker, pchip->chip_spare_size) == FLASH_API_OK )
        {
            ret = AEI_nand_is_blk_cleanmarker(pchip, start_addr, 0);
        }
    }

    return( ret );
} /* nand_is_blk_cleanmarker */

/***************************************************************************
 * Function Name: check_ecc_for_ffs
 * Description  : Checks if the ECC bytes are all 0xFFs
 * Returns      : 1 if all ECC bytes are 0xff, 0 if not
 ***************************************************************************/
static int AEI_check_ecc_for_ffs(PCFE_NAND_CHIP pchip, UINT32 step)
{
    int ret = 1;
    UINT8 spare[32];
    UINT8 *spare_mask = pchip->chip_spare_mask;
    UINT32 i, j = step * pchip->chip_spare_step_size;

    AEI_nandflash_copy_from_spare(spare, pchip->chip_spare_step_size);
    for( i = 0; i < pchip->chip_spare_step_size; i++, j++ )
    {
        if( ECC_MASK_BIT(spare_mask, j) != 0 && spare[i] != 0xff )
        {
            ret = 0;
            break;
        }
    }

    return( ret );
}

/***************************************************************************
 * Function Name: nandflash_read_page
 * Description  : Reads up to a NAND block of pages into the specified buffer.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nandflash_read_page(PCFE_NAND_CHIP pchip, unsigned long start_addr,
    unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

    if( len <= pchip->chip_block_size )
    {
        UINT32 page_addr = start_addr & ~(pchip->chip_page_size - 1);
        UINT32 page_offset = start_addr - page_addr;
        UINT32 size = pchip->chip_page_size - page_offset;
        UINT32 index = 0;
        UINT32 i;

        if(size > len)
            size = len;
        //printk("####AEI_nandflash_read_page start\n");
        do
        {
            for( i = 0, ret = FLASH_API_OK; i < pchip->chip_page_size &&
                 ret == FLASH_API_OK; i += CTRLR_CACHE_SIZE)
            {
                /* clear interrupts, so we can check later for ECC errors */
                NAND_INTR->NandInterrupt = NINT_STS_MASK;

                /* send command */
                NAND->NandCmdAddr = pchip->chip_base + page_addr + i;
                NAND->NandCmdExtAddr = 0;
                NAND->NandCmdStart = NCMD_PAGE_READ;

                if( (ret = AEI_nandflash_wait_cmd()) == FLASH_API_OK )
                {
                    /* wait until data is available in the cache */
                    if( (ret = AEI_nandflash_wait_cache()) == FLASH_API_OK )
                    {
                        if((ret = AEI_nandflash_check_ecc())==FLASH_API_ERROR)
                        {
                            if(AEI_check_ecc_for_ffs(pchip,i/CTRLR_CACHE_SIZE))
                                ret = FLASH_API_OK;
                            else
                                printk("Uncorrectable ECC Error: addr="
                                    "0x%8.8lx, intrCtrl=0x%8.8lx\n",
                                    NAND->NandEccUncAddr);
                        }
                    }

                    if( ret == FLASH_API_OK )
                    {
                        if( i < size )
                        {
                            UINT32 copy_size =
                                (i + CTRLR_CACHE_SIZE <= size)
                                ? CTRLR_CACHE_SIZE : size - i;

                            AEI_nandflash_copy_from_cache(&buffer[index + i],
                                page_offset, copy_size);
                        }
                    }
                    else
                    {   
                        printk("##wait cache fail\n");
                        break;
                    }
                }
            }

            if(ret != FLASH_API_OK)
            {
                printk("##wait cmd fail\n");
                break;
            }

            page_offset = 0;
            page_addr += pchip->chip_page_size;
            index += size;
            len -= size;
            if(len > pchip->chip_page_size)
                size = pchip->chip_page_size;
            else
                size = len;
        } while(len);

        /* Verify that the spare area contains a JFFS2 cleanmarker. */
        if( !AEI_nand_is_blk_cleanmarker(pchip,
            start_addr & ~(pchip->chip_page_size - 1), 0) )
        {
            ret = FLASH_API_ERROR;
            DBG_PRINTF("nandflash_read_page: cleanmarker not found at 0x%8.8lx\n",
                page_addr);
        }
    }
    return( ret ) ;
} /* nandflash_read_page */

/***************************************************************************
 * Function Name: nand_read_cfg
 * Description  : Reads and stores the chip configuration.
 * Returns      : None.
 ***************************************************************************/
static void AEI_nand_read_cfg(PCFE_NAND_CHIP pchip)
{
    /* Read chip configuration. */
    unsigned long cfg = NAND->NandConfig;

    pchip->chip_total_size =
        (4 * (1 << ((cfg & NC_DEV_SIZE_MASK) >> NC_DEV_SIZE_SHIFT))) << 20;

    switch( (cfg & NC_BLK_SIZE_MASK) )
    {
    case NC_BLK_SIZE_2048K:
        pchip->chip_block_size = 2048 * 1024;
        break;

    case NC_BLK_SIZE_1024K:
        pchip->chip_block_size = 1024 * 1024;
        break;

    case NC_BLK_SIZE_512K:
        pchip->chip_block_size = 512 * 1024;
        break;

    case NC_BLK_SIZE_256K:
        pchip->chip_block_size = 256 * 1024;
        break;

    case NC_BLK_SIZE_128K:
        pchip->chip_block_size = 128 * 1024;
        break;

    case NC_BLK_SIZE_16K:
        pchip->chip_block_size = 16 * 1024;
        break;

    case NC_BLK_SIZE_8K:
        pchip->chip_block_size = 8 * 1024;
        break;
    }

    switch( (cfg & NC_PG_SIZE_MASK) )
    {
    case NC_PG_SIZE_8K:
        pchip->chip_page_size = 8 * 1024;
        break;

    case NC_PG_SIZE_4K:
        pchip->chip_page_size = 4 * 1024;
        break;

    case NC_PG_SIZE_2K:
        pchip->chip_page_size = 2 * 1024;
        break;

    case NC_PG_SIZE_512B:
        pchip->chip_page_size = 512;
        break;
    }

    if( NAND_IS_MLC(pchip) )
    {
        unsigned long acc = NAND->NandAccControl;
        unsigned long acc_save = acc;

        if( (acc & (NAC_FAST_PGM_RDIN | NAC_PARTIAL_PAGE_EN)) != 0 )
        {
            printk("Disable NAND fast program read and partial page enable"
                "for MLC part.\n");
            acc &= ~(NAC_FAST_PGM_RDIN | NAC_PARTIAL_PAGE_EN);
        }

        if(((acc & NAC_ECC_LVL_MASK)>>NAC_ECC_LVL_SHIFT)==NAC_ECC_LVL_HAMMING)
        {
            printk("Changing ECC to BCH-4 from Hamming for MLC part\n");
            acc &= ~(NAC_ECC_LVL_0_MASK | NAC_ECC_LVL_MASK);
            acc |= (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_0_SHIFT) |
                 (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_SHIFT);
        }
        if( acc != acc_save )
        {
            NAND->NandAccControl = acc;
            printk("NAND Access Control register = 0x%8.8lx\n", acc);
        }
    }

#if 1 //defined(CFG_RAMAPP)
    if( NAND->NandRevision <= 0x00000202 )
    {
        PSPARE_LAYOUT spare_layout = NULL;

        pchip->chip_spare_step_size = 16;
        pchip->chip_spare_size = pchip->chip_page_size >> 5;
        switch(pchip->chip_page_size)
        {
        case 2048:
            spare_layout = &brcmnand_oob_64;
            break;

        case 4096:
            spare_layout = &brcmnand_oob_128;
            break;

        /* case 512: */
        default:
            spare_layout = &brcmnand_oob_16;
            break;
        }

        pchip->chip_spare_mask = spare_layout->sl_spare_mask;
        pchip->chip_bi_index_1 = spare_layout->sl_bi_ofs[0];
        pchip->chip_bi_index_2 = spare_layout->sl_bi_ofs[1];
    }
    else
    {
        PSPARE_LAYOUT spare_layout = NULL;
        unsigned long acc = NAND->NandAccControl;
        unsigned long ecc_lvl = (acc & NAC_ECC_LVL_MASK) >> NAC_ECC_LVL_SHIFT;
        unsigned long i, layout_parms;

        /* The access control register spare size is the number of spare area
         * bytes per 512 bytes of data.  The chip_spare_size is the number
         * of spare area bytes per page.
         */
        pchip->chip_spare_step_size =
            ((acc & NAC_SPARE_SZ_MASK) >> NAC_SPARE_SZ_SHIFT);
        pchip->chip_spare_size = pchip->chip_spare_step_size *
            (pchip->chip_page_size >> 9);

        layout_parms =
            LAYOUT_PARMS(ecc_lvl, pchip->chip_spare_size, pchip->chip_page_size);
        DBG_PRINTF(">> nand_read_cfg - layout_parms=0x%8.8lx\n", layout_parms);
        switch(layout_parms)
        {
        case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,16,512)):
            spare_layout = &brcmnand_oob_16;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,16,512)):
            spare_layout = &brcmnand_oob_bch4_512;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,64,2048)):
            spare_layout = &brcmnand_oob_bch4_2k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,64,2048)):
            spare_layout = &brcmnand_oob_bch8_16_2k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,64,2048)):
            spare_layout = &brcmnand_oob_64;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,128,4096)):
            spare_layout = &brcmnand_oob_bch4_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,128,4096)):
            spare_layout = &brcmnand_oob_bch8_16_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,216,4096)):
            spare_layout = &brcmnand_oob_bch8_27_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,216,4096)):
            spare_layout = &brcmnand_oob_bch12_27_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,128,4096)):
            spare_layout = &brcmnand_oob_128;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,256,8192)):
            spare_layout = &brcmnand_oob_bch4_8k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,256,8192)):
            spare_layout = &brcmnand_oob_bch8_16_8k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,432,8192)):
            spare_layout = &brcmnand_oob_bch8_27_8k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,432,8192)):
            spare_layout = &brcmnand_oob_bch12_27_8k;
            break;

        default:
            if( ecc_lvl != NAC_ECC_LVL_DISABLE )
            {
                printk("No valid spare layout for level=%lu, spare size=%lu,"
                    " page size=%lu\n", ecc_lvl, pchip->chip_spare_size,
                    pchip->chip_page_size);
            }

            /* Change to a disabled spare layout. */
            spare_layout = &brcmnand_oob_bch12_27_8k;
            for( i = 0; i < pchip->chip_spare_size / 8; i++ )
                spare_layout->sl_spare_mask[i] = 0;
            break;
        }

        pchip->chip_spare_mask = spare_layout->sl_spare_mask;
        pchip->chip_bi_index_1 = spare_layout->sl_bi_ofs[0];
        pchip->chip_bi_index_2 = spare_layout->sl_bi_ofs[1];
    }

    DBG_PRINTF(">> nand_read_cfg - size=%luMB, block=%luKB, page=%luB, "
        "spare=%lu\n", pchip->chip_total_size / (1024 * 1024),
        pchip->chip_block_size / 1024, pchip->chip_page_size,
        pchip->chip_spare_size);
#endif
} /* nand_read_cfg */

/***************************************************************************
 * Function Name: nand_init_cleanmarker
 * Description  : Initializes the JFFS2 clean marker buffer.
 * Returns      : None.
 ***************************************************************************/
static void AEI_nand_init_cleanmarker(PCFE_NAND_CHIP pchip)
{
    unsigned short cleanmarker[] = JFFS2_CLEANMARKER;
    unsigned char *pcm = (unsigned char *) cleanmarker;
    unsigned char *spare_mask = pchip->chip_spare_mask;
    int i, j;

    /* Skip spare area offsets reserved for ECC bytes. */
    for( i = 0, j = 0; i < pchip->chip_spare_size; i++ )
    {
        if( ECC_MASK_BIT(spare_mask, i) == 0 && j < sizeof(cleanmarker))
            g_spare_cleanmarker[i] = pcm[j++];
        else
            g_spare_cleanmarker[i] = 0xff;
    }
} /* nand_init_cleanmarker */

/***************************************************************************
 * Function Name: nand_initialize_spare_area
 * Description  : Initializes the spare area of the first page of each block 
 *                to a cleanmarker.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nand_initialize_spare_area(PCFE_NAND_CHIP pchip)
{
    unsigned char spare[SPARE_MAX_SIZE];
    unsigned long i;
    int ret;

    DBG_PRINTF(">> nand_initialize_spare_area - entry\n");

    for( i = 0; i < pchip->chip_total_size; i += pchip->chip_block_size )
    {
        /* Read the current spare area. */
        ret = AEI_nandflash_read_spare_area(pchip,0,spare,pchip->chip_spare_size);
        if(ret == FLASH_API_OK
          /*&& spare[pchip->chip_bi_index_1] != SPARE_BI_MARKER*/
          /*&& spare[pchip->chip_bi_index_2] != SPARE_BI_MARKER*/)
        {
            if( AEI_nandflash_block_erase(pchip, i) == FLASH_API_OK )
            {
                AEI_nandflash_write_spare_area(pchip, i, (unsigned char *)
                    g_spare_cleanmarker, pchip->chip_spare_size);
            }
            else
            {
                printk("\nInitialization error erasing flash block, "
                    "blk=%d\n", i);
            }
        }
    }

    return( FLASH_API_OK );
} /* nand_initialize_spare_area */


/***************************************************************************
 * Function Name: nand_flash_init
 * Description  : Initialize flash part.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int AEI_nand_flash_init(void)
{
    static int initialized = 0;
    int ret = FLASH_API_OK;


        //printk("###>> nand_flash_init - entry\n");
    if( initialized == 0 )
    {
        PCFE_NAND_CHIP pchip = &g_chip;
        static struct flash_name_from_id fnfi[] = NAND_FLASH_DEVICES;
        struct flash_name_from_id *fnfi_ptr;

        DBG_PRINTF(">> nand_flash_init - entry\n");
        /* Enable NAND data on MII ports. */
#if !defined(_BCM96328_)
        PERF->blkEnables |= NAND_CLK_EN;
#endif
#if (defined(_BCM96362_) || defined(_BCM963268_)) && (INC_SPI_FLASH_DRIVER==1)
        GPIO->GPIOBaseMode |= NAND_GPIO_OVERRIDE;
#endif
        NAND->NandNandBootConfig = NBC_AUTO_DEV_ID_CFG | 1;
#if (INC_SPI_FLASH_DRIVER==1)
        cfe_usleep(1000);
#endif
        /* Read the NAND flash chip id. */
        pchip->chip_device_id = NAND->NandFlashDeviceId;
        flash_nand_dev.flash_device_id = NAND_CHIPID(pchip);

        for( fnfi_ptr = fnfi; fnfi_ptr->fnfi_id != 0; fnfi_ptr++ )
        {
            if( fnfi_ptr->fnfi_id == flash_nand_dev.flash_device_id )
            {
                strcpy(flash_nand_dev.flash_device_name, fnfi_ptr->fnfi_name);
                break;
            }
        }

        /* If NAND chip is not in the list of NAND chips, the correct
         * configuration has still been set by the NAND controller.
         */
        if( flash_nand_dev.flash_device_name[0] == '\0' )
            strcpy(flash_nand_dev.flash_device_name, "<not identified>");


        NAND->NandCsNandXor = 0;
        pchip->chip_base = 0;
        AEI_nand_read_cfg(pchip);
        AEI_nand_init_cleanmarker(pchip);

        /* If the first block's spare area is not a JFFS2 cleanmarker,
         * initialize all block's spare area to a cleanmarker.
         */
        if( !AEI_nand_is_blk_cleanmarker(pchip, 0, 0) )
            ret = AEI_nand_initialize_spare_area(pchip);

        DBG_PRINTF(">> nand_flash_init - return %d\n", ret);

        initialized = 1;
    }

    return( ret );
} /* nand_flash_init */

int AEI_nand_flash_read_buf(unsigned short blk, int offset, unsigned char *buffer,
    int len)
{
    int ret = len;
    PCFE_NAND_CHIP pchip = &g_chip;
    UINT32 start_addr;
    UINT32 blk_addr;
    UINT32 blk_offset;
    UINT32 size;

    AEI_nand_flash_init();
    DBG_PRINTF(">> nand_flash_read_buf - 1 blk=0x%8.8lx, offset=%d, len=%lu\n",
        blk, offset, len);

    start_addr = (blk * pchip->chip_block_size) + offset;
    blk_addr = start_addr & ~(pchip->chip_block_size - 1);
    blk_offset = start_addr - blk_addr;
    size = pchip->chip_block_size - blk_offset;

    if(size > len)
        size = len;

    do
    {
        if(AEI_nandflash_read_page(pchip,start_addr,buffer,size) != FLASH_API_OK)
        {
            ret = FLASH_API_ERROR;
            break;
        }

        len -= size;
        if( len )
        {
            blk++;

            DBG_PRINTF(">> nand_flash_read_buf - 2 blk=0x%8.8lx, len=%lu\n",
                blk, len);

            start_addr = blk * pchip->chip_block_size;
            buffer += size;
            if(len > pchip->chip_block_size)
                size = pchip->chip_block_size;
            else
                size = len;
        }
    } while(len);

    DBG_PRINTF(">> nand_flash_read_buf - ret=%d\n", ret);

    return( ret ) ;
} /* nand_flash_read_buf */

/***************************************************************************
 * Function Name: nandflash_write_page
 * Description  : Writes up to a NAND block of pages from the specified buffer.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int AEI_nandflash_write_page(PCFE_NAND_CHIP pchip, unsigned long start_addr,
    unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

    if( len <= pchip->chip_block_size )
    {
        unsigned char xfer_buf[CTRLR_CACHE_SIZE];
        UINT32 page_addr = start_addr & ~(pchip->chip_page_size - 1);
        UINT32 page_offset = start_addr - page_addr;
        UINT32 size = pchip->chip_page_size - page_offset;
        UINT32 index = 0;

        /* Verify that the spare area contains a JFFS2 cleanmarker. */
        if( AEI_nand_is_blk_cleanmarker(pchip, page_addr, 1) )
        {
            UINT32 steps = pchip->chip_page_size / CTRLR_CACHE_SIZE;
            UINT32 i, xfer_ofs, xfer_size;

            if(size > len)
                size = len;

            do
            {
                for( i = 0, xfer_ofs = 0, xfer_size = 0, ret = FLASH_API_OK;
                     i < steps && ret==FLASH_API_OK; i++)
                {
                    memset(xfer_buf, 0xff, sizeof(xfer_buf));

                    if(size - xfer_ofs > CTRLR_CACHE_SIZE)
                        xfer_size = CTRLR_CACHE_SIZE;
                    else
                        xfer_size = size - xfer_ofs;

                    if( xfer_size )
                    {
                        memcpy(xfer_buf + page_offset, buffer + index + xfer_ofs,
                            xfer_size);
                    }

                    xfer_ofs += xfer_size;

                    NAND->NandCmdAddr = pchip->chip_base + page_addr +
                        (i * CTRLR_CACHE_SIZE);
                    NAND->NandCmdExtAddr = 0;

                    AEI_nandflash_copy_to_spare(g_spare_cleanmarker +
                        (i * pchip->chip_spare_step_size),
                        pchip->chip_spare_step_size);

                    AEI_nandflash_copy_to_cache(xfer_buf, 0, CTRLR_CACHE_SIZE);

                    NAND->NandCmdStart = NCMD_PROGRAM_PAGE;
                    if( (ret = AEI_nandflash_wait_cmd()) == FLASH_API_OK )
                    {
                        unsigned long sts = NAND->NandIntfcStatus;

                        if( (sts & NIS_PGM_ERASE_ERROR) != 0 )
                        {
                            printk("Error writing to block, sts=0x%8.8lx\n", sts);
                            AEI_nand_mark_bad_blk(pchip,
                                start_addr & ~(pchip->chip_page_size - 1));
                            ret = FLASH_API_ERROR;
                        }
                    }
                }

                if(ret != FLASH_API_OK)
                    break;

                page_offset = 0;
                page_addr += pchip->chip_page_size;
                index += size;
                len -= size;
                if(len > pchip->chip_page_size)
                    size = pchip->chip_page_size;
                else
                    size = len;
            } while(len);
        }
        else
            DBG_PRINTF("nandflash_write_page: cleanmarker not found at 0x%8.8lx\n",
                page_addr);
    }

    return( ret );
} /* nandflash_write_page */

/***************************************************************************
 * Function Name: nand_flash_write_buf
 * Description  : Writes to flash memory.
 * Returns      : number of bytes written or FLASH_API_ERROR
 ***************************************************************************/
int AEI_nand_flash_write_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len)
{
    int ret = len;
    PCFE_NAND_CHIP pchip = &g_chip;
    UINT32 start_addr;
    UINT32 blk_addr;
    UINT32 blk_offset;
    UINT32 size;

    DBG_PRINTF(">> nand_flash_write_buf - 1 blk=0x%8.8lx, offset=%d, len=%d\n",
        blk, offset, len);

    start_addr = (blk * pchip->chip_block_size) + offset;
    blk_addr = start_addr & ~(pchip->chip_block_size - 1);
    blk_offset = start_addr - blk_addr;
    size = pchip->chip_block_size - blk_offset;

    if(size > len)
        size = len;

    do
    {
        if(AEI_nandflash_write_page(pchip,start_addr,buffer,size) != FLASH_API_OK)
        {
            ret = ret - len;
            break;
        }
        else
        {
            len -= size;
            if( len )
            {
                blk++;

                DBG_PRINTF(">> nand_flash_write_buf- 2 blk=0x%8.8lx, len=%d\n",
                    blk, len);

                offset = 0;
                start_addr = blk * pchip->chip_block_size;
                buffer += size;
                if(len > pchip->chip_block_size)
                    size = pchip->chip_block_size;
                else
                    size = len;
            }
        }
    } while(len);

    DBG_PRINTF(">> nand_flash_write_buf - ret=%d\n", ret);

    return( ret ) ;
} /* nand_flash_write_buf */

#endif

