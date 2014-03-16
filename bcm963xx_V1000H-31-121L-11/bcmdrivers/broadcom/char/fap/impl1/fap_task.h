#ifndef __FAP_TASK_H_INCLUDED__
#define __FAP_TASK_H_INCLUDED__

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
 * File Name  : fap_task.h
 *
 * Description: This file contains the constants and structs for FAP 4ke Tasks.
 *
 *******************************************************************************
 */

#include "fap_dll.h"

#ifndef __FAP_H_INCLUDED__
#define FAP_SUCCESS  0
#define FAP_ERROR   -1
#endif

typedef int32 fapRet;

typedef enum {
    FAP4KE_TASK_PRIORITY_HIGH=0,
    FAP4KE_TASK_PRIORITY_LOW,
    FAP4KE_TASK_PRIORITY_MAX
} fap4keTsk_taskPriority_t;

typedef fapRet(* fap4keTsk_handler_t)(uint32 arg);

typedef struct {
    Dll_t node; /* used to maintain linked-lists of tasks */
    fap4keTsk_handler_t handler;
    uint32 arg;
    char *name;
    uint32 refCount;
} fap4keTsk_task_t;

#endif  /* defined(__FAP_TASK_H_INCLUDED__) */
