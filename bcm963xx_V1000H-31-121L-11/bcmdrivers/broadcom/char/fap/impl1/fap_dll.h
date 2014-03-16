#ifndef __FAP_DLL_H_INCLUDED__
#define __FAP_DLL_H_INCLUDED__

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
 * File Name  : fap_dll.h
 *
 * Description: This file contains global definitions for the use of
 *              dynamic linked lists.
 *
 *******************************************************************************
 */

#include "bcmtypes.h"

/*
 *******************************************************************************
 *                      Double Linked List Macros
 *******************************************************************************
 *
 * All dll operations must be performed on a pre-initialized node.
 * Inserting an uninitialized node into a list effectively initialized it.
 *
 * When a node is deleted from a list, you may initialize it to avoid corruption
 * incurred by double deletion. You may skip initialization if the node is
 * immediately inserted into another list.
 *
 * By placing a Dll_t element at the start of a struct, you may cast a PDll_t
 * to the struct or vice versa.
 *
 * Example of declaring an initializing someList and inserting nodeA, nodeB
 *
 *     typedef struct item {
 *         Dll_t node;
 *         int someData;
 *     } Item_t;
 *     Item_t nodeA, nodeB, nodeC;
 *     nodeA.someData = 11111, nodeB.someData = 22222, nodeC.someData = 33333;
 *
 *     Dll_t someList;
 *     dll_init( &someList );
 *
 *     dll_append(  &someList, (PDll_t) &nodeA );
 *     dll_prepend( &someList, &nodeB.node );
 *     dll_insert( (PDll_t)&nodeC, &nodeA.node );
 *
 *     dll_delete( (PDll_t) &nodeB );
 *
 * Example of a for loop to walk someList of node_p
 *
 *   extern void mydisplay( Item_t * item_p );
 *
 *   PDll_t item_p, next_p;
 *   for ( item_p = dll_head_p( &someList );
 *         ! dll_end( &someList, item_p);
 *         item_p = next_p )
 *   {
 *       next_p = dll_next_p(item_p);
 *       ... use item_p at will, including removing it from list ...
 *       mydisplay( (PItem_t)item_p );
 *   }
 *
 */
#ifndef _dll_t_
#define _dll_t_
typedef struct dll_t {
    struct dll_t * next_p;
    struct dll_t * prev_p;
} Dll_t, * PDll_t;

#define dll_init(node_p)        ((node_p)->next_p = (node_p)->prev_p = (node_p))

/* dll macros returing a PDll_t */
#define dll_head_p(list_p)      ((list_p)->next_p)
#define dll_tail_p(list_p)      ((list_p)->prev_p)

#define dll_next_p(node_p)      ((node_p)->next_p)
#define dll_prev_p(node_p)      ((node_p)->prev_p)

#define dll_empty(list_p)       ((list_p)->next_p == (list_p))
#define dll_end(list_p, node_p) ((list_p) == (node_p))

/* inserts the node new_p "after" the node at_p */
#define dll_insert(new_p, at_p) ((new_p)->next_p = (at_p)->next_p,      \
                                 (new_p)->prev_p = (at_p),              \
                                 (at_p)->next_p = (new_p),              \
                                 (new_p)->next_p->prev_p = (new_p))

#define dll_append(list_p, node_p)      dll_insert((node_p), dll_tail_p(list_p))
#define dll_prepend(list_p, node_p)     dll_insert((node_p), (list_p))

/* deletes a node from any list that it "may" be in, if at all. */
#define dll_delete(node_p)      ((node_p)->prev_p->next_p = (node_p)->next_p, \
                                 (node_p)->next_p->prev_p = (node_p)->prev_p)

#endif  /* ! defined(_dll_t_) */

#endif  /* defined(__FAP_DLL_H_INCLUDED__) */
