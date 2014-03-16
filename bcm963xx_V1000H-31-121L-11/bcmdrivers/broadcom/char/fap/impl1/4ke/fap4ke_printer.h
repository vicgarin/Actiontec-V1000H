#ifndef __FAP4KE_PRINTER_H_INCLUDED__
#define __FAP4KE_PRINTER_H_INCLUDED__

/*
 <:copyright-BRCM:2009:DUAL/GPL:standard
 
    Copyright (c) 2009 Broadcom Corporation
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

/*
 *******************************************************************************
 * File Name  : fap4ke_printer.h
 *
 * Description: This file contains the implementation of the FAP print support.
 *
 *******************************************************************************
 */

#include "fap4keLib_snprintf.h"
#ifndef FAP_4KE
#include <linux/bcm_colors.h>
#define fapMailBox_4kePrint(fmt, arg...)
#else /* FAP_4KE */
/* using backdoor -- because bcm_colors.h does not include
   any other linux files, it's ok to include it from here... */
#include <kernel/linux/include/linux/bcm_colors.h>
#endif /* FAP_4KE */

#define CC_FAP4KE_PRINT_DEBUG
#define CC_FAP4KE_PRINT_INFO
#define CC_FAP4KE_PRINT_NOTICE
#define CC_FAP4KE_PRINT_ERROR

#define fap4kePrt_Print(fmt, arg...)                            \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_PRINT, fmt, ##arg)

#if defined(CC_FAP4KE_PRINT_DEBUG)
#define fap4kePrt_Debug(fmt, arg...)                                  \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_DEBUG, CLRm "%s: " fmt CLRnorm, __FUNCTION__, ##arg)
#else
#define fap4kePrt_Debug(fmt, arg...)
#endif

#if defined(CC_FAP4KE_PRINT_INFO)
#define fap4kePrt_Info(fmt, arg...)                                   \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_INFO, CLRg "%s: " fmt CLRnorm, __FUNCTION__, ##arg)
#else
#define fap4kePrt_Info(fmt, arg...)
#endif

#if defined(CC_FAP4KE_PRINT_NOTICE)
#define fap4kePrt_Notice(fmt, arg...)                                 \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_NOTICE, CLRb "%s: " fmt CLRnorm, __FUNCTION__, ##arg)
#else
#define fap4kePrt_Notice(fmt, arg...)
#endif

#if defined(CC_FAP4KE_PRINT_ERROR)
#define fap4kePrt_Error(fmt, arg...)                                  \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_ERROR, CLRerr "%s,%d: " fmt CLRnorm, __FUNCTION__, __LINE__, ##arg)
#else
#define fap4kePrt_Error(fmt, arg...)
#endif

#define fap4kePrt_Assert(_condition)                                    \
    do {                                                                \
        if(!(_condition)) {                                             \
            fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_ASSERT, CLRerr "%s,%s,%d: " #_condition CLRnl, \
                                __FILE__, __FUNCTION__, __LINE__);      \
        }                                                               \
    } while(0)

#endif  /* defined(__FAP4KE_PRINTER_H_INCLUDED__) */
