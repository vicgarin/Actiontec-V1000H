#ifndef __FAP4KE_TASK_H_INCLUDED__
#define __FAP4KE_TASK_H_INCLUDED__

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
 * File Name  : fap4ke_task.h
 *
 * Description: This file contains the Task Manager Definitions.
 *
 *******************************************************************************
 */


/*******************************************************************
 * Public API
 *******************************************************************/

#include "fap_task.h"

#define FAP4KE_TASK_INIT(_task, _taskHandler, _arg)     \
    do {                                                \
        (_task)->handler = (_taskHandler);              \
        (_task)->arg = (_arg);                          \
        (_task)->name = #_taskHandler;                  \
        (_task)->refCount = 0;                          \
    } while(0)

void fap4keTsk_init(void);

void fap4keTsk_loop(void);

/* IMPORTANT: tasks may only be scheduled in interrupt context! */
fapRet fap4keTsk_schedule(fap4keTsk_taskPriority_t taskPriority,
                          fap4keTsk_task_t *task);

/*******************************************************************
 * Private
 *******************************************************************/

typedef struct {
    Dll_t list;
    uint32 count;
} fap4keTsk_workQueue_t;

typedef struct {
    fap4keTsk_workQueue_t workQueue[FAP4KE_TASK_PRIORITY_MAX];
    volatile int32 count;
} fap4keTsk_scheduler_t;

#endif  /* defined(__FAP4KE_TASK_H_INCLUDED__) */
