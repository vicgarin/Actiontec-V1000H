/***********************************************************
 * <:copyright-BRCM:2009:DUAL/GPL:standard
 * 
 *    Copyright (c) 2009 Broadcom Corporation
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 ************************************************************/

#ifndef __FAP4KE_INIT_H_INCLUDED__
#define __FAP4KE_INIT_H_INCLUDED__

/*
 *******************************************************************************
 * File Name  : fap4ke_init.h
 *
 * FAP_TBD: move this out
 * Description: This file contains some chip specific stuff... maybe move this...
 *
 *******************************************************************************
 */

//#define CC_FAP_6362A0

#define FAP_INIT_4KE_STACK_SIZE 1024

#if defined(CONFIG_BCM96362)
#if defined(CC_FAP_6362A0)
#define DSPRAM_SIZE          (1<<12)
#define DSPRAM_BASE          0x00002000
#else /* CC_FAP_6362A0 */
#define DSPRAM_SIZE          (1<<13)
#define DSPRAM_BASE          0x00004000
#endif /* CC_FAP_6362A0 */

#define DSPRAM_VBASE         (DSPRAM_BASE | 0x80000000)

#define FAP_INIT_DSPRAM_STACK_POINTER_LOC ( DSPRAM_VBASE + FAP_INIT_4KE_STACK_SIZE )

#else /* defined(CONFIG_BCM96362) */
#define DSPRAM_SIZE          (1<<13)
#define DSPRAM_BASE          0x00000000
#define DSPRAM_VBASE         (DSPRAM_BASE | 0x80000000)
#define FAP_INIT_DSPRAM_STACK_POINTER_LOC ( DSPRAM_VBASE + FAP_INIT_4KE_STACK_SIZE )
#endif /* defined(CONFIG_BCM96362) */

#endif  /* defined(__FAP4KE_INIT_H_INCLUDED__) */
