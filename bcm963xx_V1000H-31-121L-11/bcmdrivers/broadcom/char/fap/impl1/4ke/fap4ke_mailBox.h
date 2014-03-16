#ifndef __FAP4KE_MAILBOX_H_INCLUDED__
#define __FAP4KE_MAILBOX_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom Corporation
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
 * File Name  : fap4ke_MailBox.h
 *
 * Description: This file contains global definitions and API of the 6362 FAP
 *              Message FIFOs.
 *
 *******************************************************************************
 */

#include <stdarg.h>

#define FAP_MAILBOX_PRINTBUF_SIZE 2048 /* bytes */

typedef enum {
    FAP_MAILBOX_MSGID_SUCCESS,
    FAP_MAILBOX_MSGID_ERROR,
    FAP_MAILBOX_MSGID_PRINT,
    FAP_MAILBOX_MSGID_LOG_ASSERT,
    FAP_MAILBOX_MSGID_LOG_ERROR,
    FAP_MAILBOX_MSGID_LOG_NOTICE,
    FAP_MAILBOX_MSGID_LOG_INFO,
    FAP_MAILBOX_MSGID_LOG_DEBUG,
    FAP_MAILBOX_MSGID_KEEPALIVE,
    FAP_MAILBOX_MSGID_WORD_HI,
    FAP_MAILBOX_MSGID_WORD_LO,
    FAP_MAILBOX_MSGID_MAX
} fapMailBox_msgId_t;

typedef union {
    struct {
        uint32 msgId : 16;
        uint32 data  : 16;
    };
    uint32 u32;
} fapMailBox_msg_t;


/*******************************************************************
 * FAP API
 *******************************************************************/

extern char fapMailBox_printBuffer[FAP_MAILBOX_PRINTBUF_SIZE];

/* #define fapMailBox_4kePrint(_msgId, fmt, arg...)                        \ */
/*     do {                                                                \ */
/*         uint32 _flags;                                                  \ */
/*         fapMailBox_msg_t _msg;                                          \ */
/*         FAP4KE_LOCK(_flags);                                            \ */
/*         fap4ke_snprintf(fapMailBox_printBuffer, FAP_MAILBOX_PRINTBUF_SIZE, \ */
/*                         fmt, ##arg);                                    \ */
/*         _msg.msgId = (_msgId);                                          \ */
/*         _msg.data = ++fapMailBox_4kePrintCount;                         \ */
/*         fapToHost_xmitAndWaitForAck(_msg);                              \ */
/*         FAP4KE_UNLOCK(_flags);                                          \ */
/*     } while(0) */

void fapMailBox_4kePrint(fapMailBox_msgId_t msgId, const char *templat, ...);
void fapMailBox_4kePrintk(const char *templat, ...);
void fapMailBox_4keSendKeepAlive(void);
void fapMailBox_4keSendWord(uint32 val32);
void fapMailBox_4keInit(void);

#endif  /* defined(__FAP4KE_MAILBOX_H_INCLUDED__) */
