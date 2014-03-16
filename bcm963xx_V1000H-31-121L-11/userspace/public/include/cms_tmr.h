/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
:>
 *
 ************************************************************************/

#ifndef __CMS_TMR_H__
#define __CMS_TMR_H__

#include "cms_tms.h"
/*!\file cms_tmr.h
 * \brief Header file for the CMS Event Timer API.
 *  This is in the cms_util library.
 *
 */

/** Event handler type definition
 */
typedef void (*CmsEventHandler)(void*);

/** Max length (including NULL character) of an event timer name.
 *
 * When an event timer is created, the caller can give it a name
 * to help with debugging and lookup.  Name is optional.
 */
#define CMS_EVENT_TIMER_NAME_LENGTH  32
/** Internal event timer structure
 */
 /** This macro will evaluate TRUE if a is earlier than b */
#define IS_EARLIER_THAN(a, b) (((a)->sec < (b)->sec) || \
                               (((a)->sec == (b)->sec) && ((a)->nsec < (b)->nsec)))
typedef struct cms_timer_event
{
   struct cms_timer_event *next;      /**< pointer to the next timer. */
   CmsTimestamp            expireTms; /**< Timestamp (in the future) of when this
                                       *   timer event will expire. */
   CmsEventHandler         func;      /**< handler func to call when event expires. */
   void *                  ctxData;   /**< context data to pass to func */
   char name[CMS_EVENT_TIMER_NAME_LENGTH]; /**< name of this timer */
} CmsTimerEvent;


/** Internal timer handle. */
typedef struct
{
   CmsTimerEvent *events;     /**< Singly linked list of events */
   UINT32         numEvents;  /**< Number of events in this handle. */
} CmsTimerHandle;


/** Initialize a timer handle.
 *
 * @param tmrHandle (OUT) On successful return, a handle to be used for
 *                        future handle operation is returned.
 *
 * @return CmsRet enum.
 */
CmsRet cmsTmr_init(void **tmrHandle);


/** Clean up a timer handle, including stopping and deleting all 
 *  unexpired timers and freeing the timer handle itself.
 *
 * @param tmrHandle (IN/OUT) Timer handle returned by cmsTmr_init().
 */
void cmsTmr_cleanup(void **tmrHandle);


/** Create a new event timer which will expire in the specified number of 
 *  milliseconds.
 *
 * Since lookups are done using a combination of the handler func and
 * context data, there must not be an existing timer event in the handle
 * with the same handler func and context data.  (We could allow 
 * multiple entries with the same func and ctxData, but we will have to
 * clarify what it means to cancel a timer, cancel all or cancel the
 * next timer.)
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN)     The handler func.
 * @param ctxData   (IN)     Optional data to be passed in with the handler func.
 * @param ms        (IN)     Timer expiration value in milliseconds.
 * @param name      (IN)     Optional name of this timer event.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_set(void *tmrHandle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);


/** Stop an event timer and delete it.
 *
 * The event timer is found by matching the callback func and ctxData.
 *
 * @param tmrHandle (IN/OUT) Pointer to the event timer handle;
 * @param func      (IN) The event handler.
 * @param *handle   (IN) Argument passed to the event handler.
 */   
void cmsTmr_cancel(void *tmrHandle, CmsEventHandler func, void *ctxData);

/** Replace current event timer with a new event timer which will 
 *  expire in the specified number of milliseconds.  The current event timer
 *  has shorter expire time than the current event in list.  In other words,
 *  this routine will do nothing if the new event timer has longer expire time
 *  than the one in the timer list.
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN)     The handler func.
 * @param ctxData   (IN)     Optional data to be passed in with the handler func.
 * @param ms        (IN)     Timer expiration value in milliseconds.
 * @param name      (IN)     Optional name of this timer event.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_replaceIfSooner(void *tmrHandle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);


/** Get the number of milliseconds until the next event is due to expire.
 *
 * @param tmrHandle (IN)  Pointer to timer handle that was returned by cmsTmr_init().
 * @param ms        (OUT) Number of milliseconds until the next event.
 *
 * @return CmsRet enum.  Specifically, CMSRET_SUCCESS if there is a next event.
 *         If there are no more events in the timer handle, CMSRET_NO_MORE_INSTANCES
 *         will be returned and the parameter ms is set to MAX_UINT32.
 */
CmsRet cmsTmr_getTimeToNextEvent(const void *tmrHandle, UINT32 *ms);


/** Get the number of timer events in the timer handle.
 *
 * @param tmrHandle (IN)  Pointer to timer handle that was returned by cmsTmr_init().
 *
 * @return The number of timer events in the given handle.
 */
UINT32 cmsTmr_getNumberOfEvents(const void *tmrHandle);


/** Execute all events which have expired.
 *
 * This function will call the handler func with the ctxData for all
 * timer events that have expired.  There may be 0, 1, 2, etc. handler
 * functions called by this function.  It is up to the caller of this
 * function to call this function at the appropriate time (using the
 * value of cmsTmr_getTimeToNextEvent() and cmsTmr_getEventCount() as a guide).
 *
 * Once an event is executed, it is deleted and freed.  
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 *
 */
void cmsTmr_executeExpiredEvents(void *tmrHandle);


/** Return true if the specified handler func and context data (event) is set.
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN) The handler func.
 * @param ctxData   (IN) Optional data to be passed in with the handler func.
 *
 * @return TRUE if specified event is present, otherwise, FALSE.
 */   
UBOOL8 cmsTmr_isEventPresent(const void *tmrHandle, CmsEventHandler func, void *ctxData);


/** Use debug logging to dump out all timers in the timer handle.
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 *
 */   
void cmsTmr_dumpEvents(const void *tmrHandle);

#ifdef DMP_PERIODICSTATSBASE_1
void cmsTmr_cancel2(void *handle, CmsEventHandler func, void *ctxData);
UBOOL8 cmsTmr_isEventPresent2(const void *handle, CmsEventHandler func, void *ctxData);
CmsRet cmsTmr_set2(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);
UINT32 cmsTmr_Event_TimeRemaining(const void *handle, CmsEventHandler func, void *ctxData);
#endif
#if defined(AEI_VDSL_CUSTOMER_QWEST) || defined(AEI_VDSL_CUSTOMER_TELUS)  
UBOOL8 AEI_cmsTmr_isWlanBatchEventPresent(const void *handle, CmsEventHandler func);
void AEI_cmsTmr_WlanBatchcancel(void *handle, CmsEventHandler func, const char *name);
#endif
#endif  /* __CMS_TMR_H__ */
