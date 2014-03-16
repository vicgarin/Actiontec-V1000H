/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
# 
# 
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License, version 2, as published by  
# the Free Software Foundation (the "GPL"). 
# 
#
# 
# This program is distributed in the hope that it will be useful,  
# but WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
# GNU General Public License for more details. 
#  
# 
#  
#   
# 
# A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by 
# writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
# Boston, MA 02111-1307, USA. 
#
 *
 ************************************************************************/


#ifndef __OAL_H__
#define __OAL_H__


#include "cms.h"
#include "cms_eid.h"
#include "cms_msg.h"


/** This is the internal structure of the message handle.
 *
 * It is highly OS dependent.  Management applications should not
 * use any of its fields directly.
 *
 */
typedef struct
{
   CmsEntityId  eid;        /**< Entity id of the owner of this handle. */
   SINT32       commFd;     /**< communications fd */
   UBOOL8       standalone; /**< are we running without smd, for unittests */
   CmsMsgHeader *putBackQueue;  /**< Messages pushed back into the handle. */
} CmsMsgHandle;



/** Initialize messaging system.
 *
 * Same semantics as cmsMsg_init().
 * 
 * @param eid       (IN)  Entity id of the calling process.
 * @param msgHandle (OUT) msgHandle.
 *
 * @return CmsRet enum.
 */
CmsRet oalMsg_init(CmsEntityId eid, void **msgHandle);


/** Clean up messaging system.
 *
 * Same semantics as cmsMsg_cleanup().
 * @param msgHandle (IN) This was the msg_handle that was
 *                       created by cmsMsg_init().
 */
void oalMsg_cleanup(void **msgHandle);



/** Send a message (blocking).
 *
 * Same semantics as cmsMsg_send().
 *
 * @param fd        (IN) The commFd to send the msg out of.
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 * @return CmsRet enum.
 */
CmsRet oalMsg_send(SINT32 fd, const CmsMsgHeader *buf);


/** Receive a new message from fd.
 *
 * @param fd         (IN) commFd to receive input from.
 * @param buf       (OUT) Returns a pointer to message buffer.  Caller is responsible
 *                        for freeing the buffer.
 * @param timeout    (IN) Pointer to UINT32, specifying the timeout in milliseconds.
 *                        If pointer is NULL, receive will block until a message is
 *                        received.
 *
 * @return CmsRet enum.
 */
CmsRet oalMsg_receive(SINT32 fd, CmsMsgHeader **buf, UINT32 *timeout);


/** Get operating system dependent handle for receive message notification.
 *
 * Same semantics as cmsMsg_getEventHandle();
 * @param msgHandle    (IN) This was the msgHandle created by cmsMsg_init().
 * @param eventHandle (OUT) This is the OS dependent event handle.  For LINUX,
 *                          eventHandle is the file descriptor number.
 * @return CmsRet enum.
 */
CmsRet oalMsg_getEventHandle(const CmsMsgHandle *msgHandle, void *eventHandle);



#endif /* __OAL_H__ */

