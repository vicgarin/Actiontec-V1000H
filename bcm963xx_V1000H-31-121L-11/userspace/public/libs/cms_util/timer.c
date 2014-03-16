/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
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
#include "cms_util.h"


CmsRet cmsTmr_init(void **tmrHandle)
{

   (*tmrHandle) = cmsMem_alloc(sizeof(CmsTimerHandle), ALLOC_ZEROIZE);
   if ((*tmrHandle) == NULL)
   {
      cmsLog_error("could not malloc mem for tmrHandle");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   return CMSRET_SUCCESS;
}


void cmsTmr_cleanup(void **handle)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;

   while ((tmrEvent = tmrHandle->events) != NULL)
   {
      tmrHandle->events = tmrEvent->next;
      CMSMEM_FREE_BUF_AND_NULL_PTR(tmrEvent);
   }

   CMSMEM_FREE_BUF_AND_NULL_PTR((*handle));

   return;
}

CmsRet cmsTmr_set(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent, *newEvent;

   /*
    * First verify there is not a duplicate event.
    * (The original code first deleted any existing timer,
    * which is a "side-effect", bad style, but maybe tr69c requires
    * that functionality?)
    */
   if (cmsTmr_isEventPresent(handle, func, ctxData))
   {
      cmsLog_error("There is already an event func 0x%x ctxData 0x%x",
                   func, ctxData);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* make sure name is not too long */
   if ((name != NULL) && (strlen(name) >= CMS_EVENT_TIMER_NAME_LENGTH))
   {
      cmsLog_error("name of timer event is too long, max %d", CMS_EVENT_TIMER_NAME_LENGTH);
      return CMSRET_INVALID_ARGUMENTS;
   }


   /*
    * Allocate a structure for the timer event.
    */
   newEvent = cmsMem_alloc(sizeof(CmsTimerEvent), ALLOC_ZEROIZE);
   if (newEvent == NULL)
   {
      cmsLog_error("malloc of new timer event failed");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* fill in fields of new event timer structure. */
   newEvent->func = func;
   newEvent->ctxData = ctxData;

   cmsTms_get(&(newEvent->expireTms));
   cmsTms_addMilliSeconds(&(newEvent->expireTms), ms);

   if (name != NULL)
   {
      sprintf(newEvent->name, "%s", name);
   }


   /* 
    * Now we just need to insert it in the correct place in the timer handle.
    * We just insert the events in absolute order, i.e. smallest expire timer
    * at the head of the queue, largest at the end of the queue.  If the
    * modem is up long enough where timestamp rollover is an issue (139 years!)
    * cmsTmr_executeExpiredEvents and cmsTmr_getTimeToNextEvent will have to
    * be careful about where they pick the next timer to expire.
    */
   if (tmrHandle->numEvents == 0)
   {
      tmrHandle->events = newEvent;
   }
   else 
   {
      currEvent = tmrHandle->events;

      if (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms)))
      {
         /* queue at the head */
         newEvent->next = currEvent;
         tmrHandle->events = newEvent;
      }
      else
      {
         UBOOL8 done = FALSE;

         while (!done)
         {
            prevEvent = currEvent;
            currEvent = currEvent->next;

            if ((currEvent == NULL) ||
                (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms))))
            {
               newEvent->next = prevEvent->next;
               prevEvent->next = newEvent;
               done = TRUE;
            }
         }
      }
   }

   tmrHandle->numEvents++;

   cmsLog_debug("added event %s, expires in %ums (at %u.%03u), func=0x%x data=%p count=%d",
                newEvent->name,
                ms,
                newEvent->expireTms.sec,
                newEvent->expireTms.nsec/NSECS_IN_MSEC,
                func,
                ctxData,
                tmrHandle->numEvents);

   return CMSRET_SUCCESS;
}  


void cmsTmr_cancel(void *handle, CmsEventHandler func, void *ctxData)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent;

   if ((currEvent = tmrHandle->events) == NULL)
   {
      cmsLog_debug("no events to delete (func=0x%x data=%p)", func, ctxData);
      return;
   }

   if (currEvent->func == func && currEvent->ctxData == ctxData)
   {
      /* delete from head of the queue */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
   }
   else
   {
      UBOOL8 done = FALSE;

      while ((currEvent != NULL) && (!done))
      {
         prevEvent = currEvent;
         currEvent = currEvent->next;

         if (currEvent != NULL && currEvent->func == func && currEvent->ctxData == ctxData)
         {
            prevEvent->next = currEvent->next;
            currEvent->next = NULL;
            done = TRUE;
         }
      }
   }

   if (currEvent != NULL)
   {
      tmrHandle->numEvents--;

      cmsLog_debug("canceled event %s, count=%d", currEvent->name, tmrHandle->numEvents);

      CMSMEM_FREE_BUF_AND_NULL_PTR(currEvent);
   }
   else
   {
      cmsLog_debug("could not find requested event to delete, func=0x%x data=%p count=%d",
                   func, ctxData, tmrHandle->numEvents);
   }

   return;
}


CmsRet cmsTmr_getTimeToNextEvent(const void *handle, UINT32 *ms)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;

   cmsTms_get(&nowTms);
   currEvent = tmrHandle->events;

   if (currEvent == NULL)
   {
      *ms = MAX_UINT32;
      return CMSRET_NO_MORE_INSTANCES;
   }

   /* this is the same code as in dumpEvents, integrate? */
   if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
   {
      /*
       * the next event is past due (nowTms is later than currEvent),
       * so time to next event is 0.
       */
      *ms = 0;
   }
   else
   {
      /*
       * nowTms is earlier than currEvent, so currEvent is still in
       * the future.  
       */
      (*ms) = cmsTms_deltaInMilliSeconds(&(currEvent->expireTms), &nowTms);
   }

   return CMSRET_SUCCESS;
}


UINT32 cmsTmr_getNumberOfEvents(const void *handle)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;

   return (tmrHandle->numEvents);
}


void cmsTmr_executeExpiredEvents(void *handle)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;

   cmsTms_get(&nowTms);
   currEvent = tmrHandle->events;

   while ((currEvent != NULL) && (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms)))
   {
      /*
       * first remove the currEvent from the tmrHandle because
       * when we execute the callback function, it might call the
       * cmsTmr API again.
       */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
      tmrHandle->numEvents--;

      cmsLog_debug("executing timer event %s func 0x%x data 0x%x",
                   currEvent->name, currEvent->func, currEvent->ctxData);

      /* call the function */
      (*currEvent->func)(currEvent->ctxData);

      /* free the event struct */
      cmsMem_free(currEvent);

      currEvent = tmrHandle->events;
   }

   return;
}


UBOOL8 cmsTmr_isEventPresent(const void *handle, CmsEventHandler func, void *ctxData)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   return found;
}

void cmsTmr_dumpEvents(const void *handle)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent;
   CmsTimestamp nowTms;
   UINT32 expires;

   cmsLog_debug("dumping %d events", tmrHandle->numEvents);
   cmsTms_get(&nowTms);

   currEvent = tmrHandle->events;

   while (currEvent != NULL)
   {

      /* this is the same code as in getTimeToNextEvent, integrate? */
      if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
      {
         /*
          * the currentevent is past due (nowTms is later than currEvent),
          * so expiry time is 0.
          */
         expires = 0;
      }
      else
      {
         /*
          * nowTms is earlier than currEvent, so currEvent is still in
          * the future.  
          */
         expires = cmsTms_deltaInMilliSeconds(&(currEvent->expireTms), &nowTms);
      }


      cmsLog_debug("event %s expires in %ums (at %u.%03u) func=0x%x data=%p",
                   currEvent->name,
                   expires,
                   currEvent->expireTms.sec,
                   currEvent->expireTms.nsec/NSECS_IN_MSEC,
                   currEvent->func,
                   currEvent->ctxData);

      currEvent = currEvent->next;
   }

   return;
}


CmsRet cmsTmr_replaceIfSooner(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   CmsTimestamp nowTms;
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }
   if (found)
   {
      /* find out the expire time of this event.  If it's sooner then the one in the 
       * timer list, then replace the one in list with this one.
       */
      cmsTms_get(&nowTms);
      cmsTms_addMilliSeconds(&nowTms, ms);
      if (IS_EARLIER_THAN(&nowTms, &(tmrEvent->expireTms)))
      {
         cmsTmr_cancel((void*)tmrHandle, func, (void*)NULL);
      }
      else
      {
         return CMSRET_SUCCESS;
      }
   } /* found */
   return(cmsTmr_set(handle, func, ctxData, ms, name));
}

#if defined (DMP_PERIODICSTATSBASE_1)
void cmsTmr_cancel2(void *handle, CmsEventHandler func, void *ctxData)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent;

   if ((currEvent = tmrHandle->events) == NULL)
   {
      cmsLog_debug("no events to delete (func=0x%x data=%p)", func, ctxData);
      return;
   }

   if (currEvent->func == func && cmsUtl_strcmp(currEvent->ctxData, ctxData) == 0)
   {
      /* delete from head of the queue */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
   }
   else
   {
      UBOOL8 done = FALSE;

      while ((currEvent != NULL) && (!done))
      {
         prevEvent = currEvent;
         currEvent = currEvent->next;

         if (currEvent != NULL && currEvent->func == func && 
             cmsUtl_strcmp(currEvent->ctxData, ctxData) == 0)
         {
            prevEvent->next = currEvent->next;
            currEvent->next = NULL;
            done = TRUE;
         }
      }
   }

   if (currEvent != NULL)
   {
      tmrHandle->numEvents--;

      cmsLog_debug("canceled event %s, count=%d", currEvent->name, tmrHandle->numEvents);
      CMSMEM_FREE_BUF_AND_NULL_PTR(currEvent->ctxData); 
      CMSMEM_FREE_BUF_AND_NULL_PTR(currEvent);
   }
   else
   {
      cmsLog_debug("could not find requested event to delete, func=0x%x data=%p count=%d",
                   func, ctxData, tmrHandle->numEvents);
   }

   return;
}

UBOOL8 cmsTmr_isEventPresent2(const void *handle, CmsEventHandler func, void *ctxData)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && cmsUtl_strcmp(tmrEvent->ctxData, ctxData) == 0)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   return found;
}

CmsRet cmsTmr_set2(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent, *newEvent;

   /*
    * First verify there is not a duplicate event.
    * (The original code first deleted any existing timer,
    * which is a "side-effect", bad style, but maybe tr69c requires
    * that functionality?)
    */
   if (cmsTmr_isEventPresent2(handle, func, ctxData))
   {
      cmsLog_error("There is already an event func 0x%x ctxData 0x%x",
                   func, ctxData);
      return CMSRET_INVALID_ARGUMENTS;
   }

   /* make sure name is not too long */
   if ((name != NULL) && (strlen(name) >= CMS_EVENT_TIMER_NAME_LENGTH))
   {
      cmsLog_error("name of timer event is too long, max %d", CMS_EVENT_TIMER_NAME_LENGTH);
      return CMSRET_INVALID_ARGUMENTS;
   }


   /*
    * Allocate a structure for the timer event.
    */
   newEvent = cmsMem_alloc(sizeof(CmsTimerEvent), ALLOC_ZEROIZE);
   if (newEvent == NULL)
   {
      cmsLog_error("malloc of new timer event failed");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* fill in fields of new event timer structure. */
   newEvent->func = func;
   newEvent->ctxData = ctxData;

   cmsTms_get(&(newEvent->expireTms));
   cmsTms_addMilliSeconds(&(newEvent->expireTms), ms);

   if (name != NULL)
   {
      sprintf(newEvent->name, "%s", name);
   }


   /* 
    * Now we just need to insert it in the correct place in the timer handle.
    * We just insert the events in absolute order, i.e. smallest expire timer
    * at the head of the queue, largest at the end of the queue.  If the
    * modem is up long enough where timestamp rollover is an issue (139 years!)
    * cmsTmr_executeExpiredEvents and cmsTmr_getTimeToNextEvent will have to
    * be careful about where they pick the next timer to expire.
    */
   if (tmrHandle->numEvents == 0)
   {
      tmrHandle->events = newEvent;
   }
   else 
   {
      currEvent = tmrHandle->events;

      if (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms)))
      {
         /* queue at the head */
         newEvent->next = currEvent;
         tmrHandle->events = newEvent;
      }
      else
      {
         UBOOL8 done = FALSE;

         while (!done)
         {
            prevEvent = currEvent;
            currEvent = currEvent->next;

            if ((currEvent == NULL) ||
                (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms))))
            {
               newEvent->next = prevEvent->next;
               prevEvent->next = newEvent;
               done = TRUE;
            }
         }
      }
   }

   tmrHandle->numEvents++;

   cmsLog_debug("added event %s, expires in %ums (at %u.%03u), func=0x%x data=%p count=%d",
                newEvent->name,
                ms,
                newEvent->expireTms.sec,
                newEvent->expireTms.nsec/NSECS_IN_MSEC,
                func,
                ctxData,
                tmrHandle->numEvents);

   return CMSRET_SUCCESS;
}
 
UINT32 cmsTmr_Event_TimeRemaining(const void *handle, CmsEventHandler func, void *ctxData)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;
   CmsTimestamp tms;
   UINT32 timeremaining = 0;

   cmsTms_get(&tms);
   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && cmsUtl_strcmp(tmrEvent->ctxData, ctxData) == 0)
      {
         found = TRUE;
         if (tmrEvent->expireTms.sec > tms.sec)
         {
            timeremaining = tmrEvent->expireTms.sec - tms.sec;
         }
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   return timeremaining;
}
#endif /* DMP_PERIODICSTATSBASE_1 */

