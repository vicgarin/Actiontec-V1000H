/*
<:copyright-gpl 
 Copyright 2004 Broadcom Corp. All Rights Reserved. 
 
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
 * prom.c: PROM library initialization code.
 *
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <bcm_map_part.h>
#include <bcm_cpu.h>
#include <board.h>
#include <boardparms.h>

extern int  do_syslog(int, char *, int);


unsigned char g_blparms_buf[1024];

#if defined (CONFIG_BCM_BCMDSP_MODULE)
unsigned int main_tp_num;
#endif

void __init create_root_nfs_cmdline( char *cmdline );
UINT32 __init calculateCpuSpeed(void);
void __init retrieve_boot_loader_parameters(void);

#if defined (CONFIG_BCM96328)
const uint32 cpu_speed_table[0x20] = {
    320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320,
    0, 320, 160, 200, 160, 200, 400, 320, 320, 160, 384, 320, 192, 320, 320, 320
};
#endif

#if defined (CONFIG_BCM96362)
const uint32 cpu_speed_table[0x20] = {
    320, 320, 320, 240, 160, 400, 440, 384, 320, 320, 320, 240, 160, 320, 400, 320,
    320, 320, 320, 240, 160, 200, 400, 384, 320, 320, 320, 240, 160, 200, 400, 400
};
#endif

#if defined (CONFIG_BCM963268)
const uint32 cpu_speed_table[0x20] = {
    0, 0, 400, 320, 0, 0, 0, 0, 0, 0, 333, 400, 0, 0, 320, 400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif

#if defined (CONFIG_BCM96816)
const uint32 cpu_speed_table[0x20] = {
    200, 400, 400, 320, 200, 400, 333, 333, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 400, 400, 200, 360, 400, 400, 300, 300, 320, 320, 400, 400
};
#endif

static char promBoardIdStr[NVRAM_BOARD_ID_STRING_LEN];
const char *get_system_type(void)
{
    kerSysNvRamGetBoardId(promBoardIdStr);
    return(promBoardIdStr);
}


/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */

extern struct plat_smp_ops brcm_smp_ops;

void __init prom_init(void)
{
    kerSysEarlyFlashInit();

    // too early in bootup sequence to acquire spinlock, not needed anyways
    // only the kernel is running at this point
    kerSysNvRamGetBoardIdLocked(promBoardIdStr);
    printk( "%s prom init\n", promBoardIdStr );

#if defined(CONFIG_BCM_BCMDSP_MODULE)
    main_tp_num = ((read_c0_diag3() & CP0_CMT_TPID) == CP0_CMT_TPID) ? 1 : 0;
    printk("Linux TP ID = %u \n", (unsigned int)main_tp_num);
#endif

    PERF->IrqControl[0].IrqMask=0;

    arcs_cmdline[0] = '\0';

#if defined(CONFIG_ROOT_NFS)
    create_root_nfs_cmdline( arcs_cmdline );
#elif defined(CONFIG_ROOT_FLASHFS)
    strcpy(arcs_cmdline, CONFIG_ROOT_FLASHFS);
#endif
    strcat(arcs_cmdline, " ");
    strcat(arcs_cmdline, CONFIG_CMDLINE);

    /* Count register increments every other clock */
    mips_hpt_frequency = calculateCpuSpeed() / 2;

#if defined (CONFIG_SMP)
    register_smp_ops(&brcm_smp_ops);
#endif

    retrieve_boot_loader_parameters();
}


/* --------------------------------------------------------------------------
    Name: prom_free_prom_memory
Abstract: 
 -------------------------------------------------------------------------- */
void __init prom_free_prom_memory(void)
{

}

#if defined(CONFIG_ROOT_NFS) && defined(SUPPORT_SWMDK)
  /* We can't use gendefconfig to automatically fix this, so instead we will
     raise an error here */
  #error "Kernel cannot be configured for both SWITCHMDK and NFS."
#endif

#if defined(CONFIG_ROOT_NFS)
/* This function reads in a line that looks something like this:
 *
 *
 * CFE bootline=bcmEnet(0,0)host:vmlinux e=192.169.0.100:ffffff00 h=192.169.0.1
 *
 *
 * and retuns in the cmdline parameter some that looks like this:
 *
 * CONFIG_CMDLINE="root=/dev/nfs nfsroot=192.168.0.1:/opt/targets/96345R/fs
 * ip=192.168.0.100:192.168.0.1::255.255.255.0::eth0:off rw"
 */
#define BOOT_LINE_ADDR   0x0
#define HEXDIGIT(d) ((d >= '0' && d <= '9') ? (d - '0') : ((d | 0x20) - 'W'))
#define HEXBYTE(b)  (HEXDIGIT((b)[0]) << 4) + HEXDIGIT((b)[1])

void __init create_root_nfs_cmdline( char *cmdline )
{
    char *localip = NULL;
    char *hostip = NULL;
    char mask[16] = "";
    char bootline[NVRAM_BOOTLINE_LEN] = "";
    char *p = bootline;

    // too early in bootup sequence to acquire spinlock, not needed anyways
    // only the kernel is running at this point
    kerSysNvRamGetBootlineLocked(bootline);

    while( *p )
    {
        if( p[0] == 'e' && p[1] == '=' )
        {
            /* Found local ip address */
            p += 2;
            localip = p;
            while( *p && *p != ' ' && *p != ':' )
                p++;
            if( *p == ':' )
            {
                /* Found network mask (eg FFFFFF00 */
                *p++ = '\0';
                sprintf( mask, "%u.%u.%u.%u", HEXBYTE(p), HEXBYTE(p + 2),
                HEXBYTE(p + 4), HEXBYTE(p + 6) );
                p += 4;
            }
            else if( *p == ' ' )
                *p++ = '\0';
        }
        else if( p[0] == 'h' && p[1] == '=' )
        {
            /* Found host ip address */
            p += 2;
            hostip = p;
            while( *p && *p != ' ' )
                p++;
            if( *p == ' ' )
                    *p++ = '\0';
        }
        else 
            p++;
    }

    if( localip && hostip ) 
    {
        sprintf( cmdline, "root=/dev/nfs nfsroot=%s:" CONFIG_ROOT_NFS_DIR " ip=%s:%s::%s::eth0:off rw",
                 hostip, localip, hostip, mask );
    }
    
}
#endif

/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM63xx CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fcpu_clk = (25 * MIPSDDR_NDIV) / MIPS_MDIV
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */
#if defined(CONFIG_BCM96368)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 cpu_speed;
    UINT32 numerator;
    UINT32 pllConfig = DDR->MIPSDDRPLLConfig;
    UINT32 pllMDiv = DDR->MIPSDDRPLLMDiv;

    numerator = 64000000 / ((pllConfig & MIPSDDR_P1_MASK) >> MIPSDDR_P1_SHFT) * 
        ((pllConfig & MIPSDDR_P2_MASK) >> MIPSDDR_P2_SHFT) *
        ((pllConfig & MIPSDDR_NDIV_MASK) >> MIPSDDR_NDIV_SHFT);

    cpu_speed = numerator / ((pllMDiv & MIPS_MDIV_MASK) >> MIPS_MDIV_SHFT);

    return cpu_speed;
}
#endif

#if defined(CONFIG_BCM96328) || defined(CONFIG_BCM96362) || defined(CONFIG_BCM96816) || defined(CONFIG_BCM963268)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 mips_pll_fvco;

    mips_pll_fvco = MISC->miscStrapBus & MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK;
    mips_pll_fvco >>= MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT;

    return cpu_speed_table[mips_pll_fvco] * 1000000;
}
#endif

/* Retrieve a buffer of paramters passed by the boot loader.  Functions in
 * board.c can return requested parameter values to a calling Linux function.
 */
void __init retrieve_boot_loader_parameters(void)
{
    extern unsigned char _text;
    unsigned long blparms_magic = *(unsigned long *) (&_text - 8);
    unsigned long blparms_buf = *(unsigned long *) (&_text - 4);
    unsigned char *src = (unsigned char *) blparms_buf;
    unsigned char *dst = g_blparms_buf;

    if( blparms_magic != BLPARMS_MAGIC )
    {
        /* Subtract four more bytes for NAND flash images. */
        blparms_magic = *(unsigned long *) (&_text - 12);
        blparms_buf = *(unsigned long *) (&_text - 8);
        src = (unsigned char *) blparms_buf;
    }

    if( blparms_magic == BLPARMS_MAGIC )
    {
        do
        {
            *dst++ = *src++;
        } while( (src[0] != '\0' || src[1] != '\0') &&
          (unsigned long) (dst - g_blparms_buf) < sizeof(g_blparms_buf) - 2);
    }

    dst[0] = dst[1] = '\0';
}

