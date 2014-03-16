/*
    Copyright 2000-2010 Broadcom Corporation

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the “GPL?, available at http://www.broadcom.com/licenses/GPLv2.php,
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

/***************************************************************************
 * File Name  : flash_api.h
 *
 * Description: This file contains definitions and prototypes for a public
 *              flash device interface and an internal flash device interface.
 ***************************************************************************/

#if !defined(_FLASH_API_H)
#define _FLASH_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* Flash definitions. */
#define FLASH_API_OK                1
#define FLASH_API_ERROR             -1

#define FLASH_IFC_UNKNOWN           0
#define FLASH_IFC_PARALLEL          1
#define FLASH_IFC_SPI               2
#define FLASH_IFC_HS_SPI            3
#define FLASH_IFC_NAND              4

#define NAND_REINIT_FLASH           0xffff

/* Public Interface Prototypes. */
int flash_init(void);
int flash_sector_erase_int(unsigned short sector);
int flash_read_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int flash_write_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int flash_get_numsectors(void);
int flash_get_sector_size(unsigned short sector);
unsigned char *flash_get_memptr(unsigned short sector);
int flash_get_blk(int addr);
int flash_get_total_size(void);
int flash_get_flash_type(void);
void flash_change_flash_type(int type);
#ifdef AEI_CONFIG_JFFS
int AEI_nand_flash_read_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
int AEI_nand_flash_write_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
#endif
#ifdef AEI_NAND_IMG_CHECK
int flash_nand_img_check(unsigned short s_sector, unsigned short e_sector);
int flash_write_buf_crc(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes);
#endif
#if defined(AEI_VDSL_CHECK_FLASH_ID)
int AEI_flash_get_flash_id(void);
#endif
/* Internal Flash Device Driver Information. */
typedef struct flash_device_info_s
{
    unsigned short flash_device_id;
    unsigned short flash_type;
    char flash_device_name[30];

    int (*fn_flash_sector_erase_int) (unsigned short sector);
    int (*fn_flash_read_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_write_buf) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_get_numsectors) (void);
    int (*fn_flash_get_sector_size) (unsigned short sector);
    unsigned char * (*fn_flash_get_memptr) (unsigned short sector);
    int (*fn_flash_get_blk) (int addr);
    int (*fn_flash_get_total_size) (void);
#ifdef AEI_NAND_IMG_CHECK
    int (*fn_flash_write_buf_crc) (unsigned short sector, int offset,
        unsigned char *buffer, int numbytes);
    int (*fn_flash_nand_img_check) (unsigned short s_sector, unsigned short e_sector);
#endif
} flash_device_info_t;

#ifdef __cplusplus
}
#endif

#endif /* _FLASH_API_H */

