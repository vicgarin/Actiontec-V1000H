/* vi: set sw=4 ts=4: */
/* eraseall.c -- erase the whole of a MTD device
 *
 * Ported to busybox from mtd-utils.
 *
 * Copyright (C) 2000 Arcom Control System Ltd
 *
 * Renamed to flash_eraseall.c
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include <getopt.h>
#include <features.h>
#include <sys/types.h>
#include <sys/stat.h>           /* stat */
#include <sys/ioctl.h>           /* stat */
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "libbb.h"
#include <mtd/mtd-user.h>
#include <mtd/jffs2-user.h>


/* Broadcom has changed the definition of this structure in the kernel version
 * of mtdabi.h.  This file includes a version of mtdabi.h from the toolchain.
 * Therefore, copy the kernel definition here.
 */
#undef ECCGETLAYOUT
#define ECCGETLAYOUT        _IOR('M', 17, struct k_nand_ecclayout)

#undef MTD_MAX_OOBFREE_ENTRIES
#define MTD_MAX_OOBFREE_ENTRIES 9
#define MTD_MAX_ECCPOS_ENTRIES  64  // Was 64 for SLC
/*
 * ECC layout control structure. Exported to userspace for
 * diagnosis and to allow creation of raw images
 */
struct k_nand_ecclayout {
    uint32_t eccbytes;
    uint32_t eccpos[MTD_MAX_ECCPOS_ENTRIES];
    uint32_t oobavail;
    struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES];
};


#define OPTION_J    (1 << 0)
#define OPTION_Q    (1 << 1)
#define OPTION_R    (1 << 2)
#define OPTION_E    (1 << 3)
#define IS_NAND     (1 << 4)
#define BBTEST      (1 << 5)

int target_endian = __BYTE_ORDER;

static void show_progress(mtd_info_t *meminfo, erase_info_t *erase)
{
    printf("\rErasing %d Kibyte @ %x -- %2llu %% complete.",
        (unsigned)meminfo->erasesize / 1024, erase->start,
        (unsigned long long) erase->start * 100 / meminfo->size);
    fflush(stdout);
}

extern int flash_eraseall_main(int argc, char **argv)
{
    struct jffs2_unknown_node oob_cleanmarker;
    mtd_info_t meminfo;
    int fd, clmpos, clmlen;
    erase_info_t erase;
    struct stat st;
    unsigned int flags;
    char *mtd_name;

    flags = BBTEST | bb_getopt_ulflags(argc, argv, "jqre");

    mtd_name = argv[optind];
    stat(mtd_name, &st);
    if (!S_ISCHR(st.st_mode))
        bb_error_msg_and_die("%s: not a char device", mtd_name);

    fd = bb_xopen(mtd_name, O_RDWR);

    ioctl(fd, MEMGETINFO, &meminfo);
    erase.length = meminfo.erasesize;
    if (meminfo.type == MTD_NANDFLASH)
        flags |= IS_NAND;

    clmpos = 0;
    clmlen = 8;
    if (flags & OPTION_J) {

        oob_cleanmarker.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
        oob_cleanmarker.nodetype = cpu_to_je16(JFFS2_NODETYPE_CLEANMARKER);
        oob_cleanmarker.totlen = cpu_to_je32(8);
        oob_cleanmarker.hdr_crc = cpu_to_je32(0xffffffff);

        if (!(flags & IS_NAND))
            oob_cleanmarker.totlen = cpu_to_je32(sizeof(struct jffs2_unknown_node));
        else {
            struct k_nand_ecclayout ecclayout;

            ioctl(fd, ECCGETLAYOUT, &ecclayout);
            clmlen = ecclayout.oobavail;
            clmpos = 0;
        }
    }

    /* Don't want to destroy progress indicator by bb_error_msg's */
    printf("\nflash_eraseall: %s", mtd_name);

    for (erase.start = 0; erase.start < meminfo.size;
         erase.start += meminfo.erasesize) {
        if (flags & BBTEST) {
            int ret;
            loff_t offset = erase.start;

            ret = ioctl(fd, MEMGETBADBLOCK, &offset);
            if (ret > 0) {
                if (!(flags & OPTION_Q))
                    printf("\nSkipping bad block at 0x%08x", erase.start);
                continue;
            }
            if (ret < 0) {
                /* Black block table is not available on certain flash
                 * types e.g. NOR
                 */
                if (errno == EOPNOTSUPP) {
                    flags = ~BBTEST;
                    if (flags & IS_NAND)
                        bb_error_msg_and_die("bad block check not available");
                } else {
                    bb_perror_msg_and_die("MEMGETBADBLOCK error");
                }
            }
        }

        if (!(flags & OPTION_Q))
            show_progress(&meminfo, &erase);

        if (!(flags & OPTION_E))
            ioctl(fd, MEMERASE, &erase);

        /* format for JFFS2 ? */
        if (!(flags & OPTION_J))
            continue;

        /* write cleanmarker */
        if (flags & IS_NAND) {
            struct mtd_oob_buf oob;

            oob.ptr = (unsigned char *) &oob_cleanmarker;
            oob.start = erase.start + clmpos;
            oob.length = clmlen;
#if 0 /* TESTING */
            printf("W %8.8lx %lu - %8.8lx %8.8lx %8.8lx\n",
                (unsigned long) oob.start, (unsigned long) oob.length,
                *((unsigned long *) &oob_cleanmarker + 0),
                *((unsigned long *) &oob_cleanmarker + 1),
                *((unsigned long *) &oob_cleanmarker + 2));
#endif
            ioctl(fd, MEMWRITEOOB, &oob);
        } else {
            lseek(fd, erase.start, SEEK_SET);
            /* if (lseek(fd, erase.start, SEEK_SET) < 0) {
                bb_perror_msg("MTD %s failure", "seek");
                continue;
            } */
            write(fd, &oob_cleanmarker, sizeof(oob_cleanmarker));
            /* if (write(fd, &oob_cleanmarker, sizeof(oob_cleanmarker)) != sizeof(oob_cleanmarker)) {
                bb_perror_msg("MTD %s failure", "write");
                continue;
            } */
        }
        if (!(flags & OPTION_Q))
            printf(" Cleanmarker written at %x.", erase.start);
    }
    if (!(flags & OPTION_Q)) {
        show_progress(&meminfo, &erase);
        putchar('\n');
    }

    if (flags & OPTION_R) {
        int i;
        /* For testing, read back cleanmarker plus 12 bytes. */
        for (i= 0; i< meminfo.size; i+= meminfo.erasesize) { 
            if (flags & BBTEST) {
                int ret;
                loff_t offset = i;

                ret = ioctl(fd, MEMGETBADBLOCK, &offset);
                if (ret > 0) {
                    if (!(flags & OPTION_Q))
                        printf("\nSkipping bad block at 0x%08x", i);
                    continue;
                }
                if (ret < 0) {
                    /* Black block table is not available on certain flash
                     * types e.g. NOR
                     */
                    if (errno == EOPNOTSUPP) {
                        flags = ~BBTEST;
                        if (flags & IS_NAND)
                            bb_error_msg_and_die("bad block check not available");
                    } else {
                        bb_perror_msg_and_die("MEMGETBADBLOCK error");
                    }
                }
            }

            if (flags & IS_NAND) {
                unsigned char clnmrk[20];
                struct mtd_oob_buf oob;

                memset(clnmrk, 0x00, sizeof(clnmrk));
                oob.ptr = (unsigned char *) &clnmrk;
                oob.start = i + clmpos;
                oob.length = 16; // clmlen;
                ioctl(fd, MEMREADOOB, &oob);

                printf("R %8.8lx %lu - %8.8lx %8.8lx %8.8lx %8.8lx\n",
                    (unsigned long) (i + clmpos),
                    (unsigned long) clmlen,
                    *((unsigned long *) &clnmrk[0]),
                    *((unsigned long *) &clnmrk[4]),
                    *((unsigned long *) &clnmrk[8]),
                    *((unsigned long *) &clnmrk[12]));
            }
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}

