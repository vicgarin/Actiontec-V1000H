/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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


#include "cms.h"
#include "cms_tms.h"
#include "oal.h"

void cmsTms_get(CmsTimestamp *tms)
{
   oalTms_get(tms);
}

void cmsTms_delta(const CmsTimestamp *newTms,
                  const CmsTimestamp *oldTms,
                  CmsTimestamp *deltaTms)
{
   if (newTms->sec >= oldTms->sec)
   {
      if (newTms->nsec >= oldTms->nsec)
      {
         /* no roll-over in the sec and nsec fields, straight subtract */
         deltaTms->nsec = newTms->nsec - oldTms->nsec;
         deltaTms->sec = newTms->sec - oldTms->sec;
      }
      else
      {
         /* no roll-over in the sec field, but roll-over in nsec field */
         deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
         deltaTms->sec = newTms->sec - oldTms->sec - 1;
      }
   }
   else
   {
      if (newTms->nsec >= oldTms->nsec)
      {
         /* roll-over in the sec field, but no roll-over in the nsec field */
         deltaTms->nsec = newTms->nsec - oldTms->nsec;
         deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec + 1; /* +1 to account for time spent during 0 sec */
      }
      else
      {
         /* roll-over in the sec and nsec fields */
         deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
         deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec;
      }
   }
}

UINT32 cmsTms_deltaInMilliSeconds(const CmsTimestamp *newTms,
                                  const CmsTimestamp *oldTms)
{
   CmsTimestamp deltaTms;
   UINT32 ms;

   cmsTms_delta(newTms, oldTms, &deltaTms);

   if (deltaTms.sec > MAX_UINT32 / MSECS_IN_SEC)
   {
      /* the delta seconds is larger than the UINT32 return value, so return max value */
      ms = MAX_UINT32;
   }
   else
   {
      ms = deltaTms.sec * MSECS_IN_SEC;

      if ((MAX_UINT32 - ms) < (deltaTms.nsec / NSECS_IN_MSEC))
      {
         /* overflow will occur when adding the nsec, return max value */
         ms = MAX_UINT32;
      }
      else
      {
         ms += deltaTms.nsec / NSECS_IN_MSEC;
      }
   }

   return ms;
}


void cmsTms_addMilliSeconds(CmsTimestamp *tms, UINT32 ms)
{
   UINT32 addSeconds;
   UINT32 addNano;

   addSeconds = ms / MSECS_IN_SEC;
   addNano = (ms % MSECS_IN_SEC) * NSECS_IN_MSEC;

   tms->sec += addSeconds;
   tms->nsec += addNano;

   /* check for carry-over in nsec field */
   if (tms->nsec > NSECS_IN_SEC)
   {
      /* we can't have carried over by more than 1 second */
      tms->sec++;
      tms->nsec -= NSECS_IN_SEC;
   }

   return;
}


CmsRet cmsTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen)
{
   return (oalTms_getXSIDateTime(t, buf, bufLen));
}

CmsRet cmsTms_getDaysHoursMinutesSeconds(UINT32 t, char *buf, UINT32 bufLen)
{
    UINT32 days, hours, minutes, seconds;
    SINT32 r;
    CmsRet ret=CMSRET_SUCCESS;

    days = t / SECS_IN_DAY;
    t -= (days * SECS_IN_DAY);

    hours = t / SECS_IN_HOUR;
    t -= (hours * SECS_IN_HOUR);

    minutes = t / SECS_IN_MINUTE;
    t -= (minutes * SECS_IN_MINUTE);

    seconds = t;

    memset(buf, 0, bufLen);
    r = snprintf(buf, bufLen-1, "%dD %dH %dM %dS", days, hours, minutes, seconds);
    if (r >= bufLen)
    {
        ret = CMSRET_RESOURCE_EXCEEDED;
    }

    return ret;
}
