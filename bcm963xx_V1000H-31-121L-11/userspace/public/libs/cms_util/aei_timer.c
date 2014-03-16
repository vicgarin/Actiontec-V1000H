#include "cms.h"
#include "cms_util.h"

#if defined (DMP_PERIODICSTATSBASE_1)
void AEI_cmsTmr_cancel2(void *handle, CmsEventHandler func, void *ctxData)
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

UBOOL8 AEI_cmsTmr_isEventPresent2(const void *handle, CmsEventHandler func, void *ctxData)
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

CmsRet AEI_cmsTmr_set2(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent, *newEvent;

   /*
    * First verify there is not a duplicate event.
    * (The original code first deleted any existing timer,
    * which is a "side-effect", bad style, but maybe tr69c requires
    * that functionality?)
    */
   if (AEI_cmsTmr_isEventPresent2(handle, func, ctxData))
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
 
UINT32 AEI_cmsTmr_Event_TimeRemaining(const void *handle, CmsEventHandler func, void *ctxData)
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

#if defined(AEI_VDSL_CUSTOMER_QWEST) || defined(AEI_VDSL_CUSTOMER_TELUS) 
UBOOL8 AEI_cmsTmr_isWlanBatchEventPresent(const void *handle, CmsEventHandler func)
{
   const CmsTimerHandle *tmrHandle = (const CmsTimerHandle *) handle;
   CmsTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {

      if (tmrEvent->func == func)
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
void AEI_cmsTmr_WlanBatchcancel(void *handle, CmsEventHandler func, const char *name)
{
   CmsTimerHandle *tmrHandle = (CmsTimerHandle *) handle;
   CmsTimerEvent *currEvent, *prevEvent;

   if ((currEvent = tmrHandle->events) == NULL)
   {
      cmsLog_error("no events to delete (func=0x%x name=%s)", func, name);
      return;
   }

   if (currEvent->func == func )
   {
      /* delete from head of the queue */
      cmsLog_debug("F=%s , L=%d ,  (func=0x%x name=%s)", __func__ , __LINE__ , func, name);
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

         if (currEvent != NULL && currEvent->func == func  )
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
      cmsLog_error("could not find requested event to delete, func=0x%x name=%s count=%d",
                   func, name, tmrHandle->numEvents);
   }

   return;
}
#endif
