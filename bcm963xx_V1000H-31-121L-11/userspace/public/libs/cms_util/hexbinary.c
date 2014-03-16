/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cms_util.h"


CmsRet cmsUtl_binaryBufToHexString(const UINT8 *binaryBuf, UINT32 binaryBufLen, char **hexStr)
{
   UINT32 i, j;

   if (hexStr == NULL)
   {
      cmsLog_error("hexStr buffer is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }

   *hexStr = cmsMem_alloc((binaryBufLen*2)+1, ALLOC_ZEROIZE);
   if (*hexStr == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   for (i=0, j=0; i < binaryBufLen; i++, j+=2)
   {
      sprintf(&((*hexStr)[j]), "%02x", binaryBuf[i]);
   }

   return CMSRET_SUCCESS;
}


CmsRet cmsUtl_hexStringToBinaryBuf(const char *hexStr, UINT8 **binaryBuf, UINT32 *binaryBufLen)
{
   UINT32 len;
   UINT32 val;
   UINT32 i, j;
   char tmpbuf[3];
   CmsRet ret;

   len = strlen(hexStr);
   if (len % 2 != 0)
   {
      cmsLog_error("hexStr must be an even number of characters");
      return CMSRET_INVALID_ARGUMENTS;
   }

   *binaryBuf = cmsMem_alloc(len/2, 0);
   if (*binaryBuf == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   for (i=0, j=0; j < len; i++, j+=2)
   {
      tmpbuf[0] = hexStr[j];
      tmpbuf[1] = hexStr[j+1];
      tmpbuf[2] = 0;

      ret = cmsUtl_strtoul(tmpbuf, NULL, 16, &val);
      if (ret != CMSRET_SUCCESS)
      {
         cmsMem_free(*binaryBuf);
         *binaryBuf = NULL;
         return ret;
      }
      else
      {
         (*binaryBuf)[i] = (UINT8) val;
      }
   }

   /* if we get here, we were successful, set length */
   *binaryBufLen = len / 2;

   return ret;
}



