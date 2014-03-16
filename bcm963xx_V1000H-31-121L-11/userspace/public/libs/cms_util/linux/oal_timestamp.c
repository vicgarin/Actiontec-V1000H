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

#include "../oal.h"
#include <unistd.h>
#include <time.h>


/** OS dependent timestamp functions go in this file.
 */
void oalTms_get(CmsTimestamp *tms)
{
   struct timespec ts;
   SINT32 rc;

   if (tms == NULL)
   {
      return;
   }

   rc = clock_gettime(CLOCK_MONOTONIC, &ts);
   if (rc == 0)
   {
      tms->sec = ts.tv_sec;
      tms->nsec = ts.tv_nsec;
   }
   else
   {
      cmsLog_error("clock_gettime failed, set timestamp to 0");
      tms->sec = 0;
      tms->nsec = 0;
   }
}


CmsRet oalTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen)
{
	int          c;
   time_t       now;
	struct tm   *tmp;

   if (t == 0)
   {
      now = time(NULL);
   }
   else
   {
      now = t;
   }

	tmp = localtime(&now);
   memset(buf, 0, bufLen);
	c = strftime(buf, bufLen, "%Y-%m-%dT%H:%M:%S%z", tmp);
   if ((c == 0) || (c+1 > bufLen))
   {
      /* buf was not long enough */
      return CMSRET_RESOURCE_EXCEEDED;
   }

	/* fix missing : in time-zone offset-- change -500 to -5:00 */
   buf[c+1] = '\0';
   buf[c] = buf[c-1];
   buf[c-1] = buf[c-2];
   buf[c-2]=':';

   return CMSRET_SUCCESS;
}
