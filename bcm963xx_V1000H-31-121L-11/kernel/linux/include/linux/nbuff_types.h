#ifndef __NBUFF_TYPES_H_INCLUDED__
#define __NBUFF_TYPES_H_INCLUDED__

/*
<:copyright-gpl

 Copyright 2011 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.

:>
*/

/*
 *******************************************************************************
 *
 * File Name  : nbuff_types.h
 * Description: Simple nbuff type defines.
 *
 ******************************************************************************* */

#define MUST_BE_ZERO                0

/* virtual network buffer pointer to SKB|FPB|TGB|FKB  */
typedef void * pNBuff_t;
#define PNBUFF_NULL                 ((pNBuff_t)NULL)

typedef enum NBuffPtrType
{
    SKBUFF_PTR = MUST_BE_ZERO,      /* Default Linux networking socket buffer */
    FPBUFF_PTR,                     /* Experimental BRCM IuDMA freepool buffer*/
    TGBUFF_PTR,                     /* LAB Traffic generated network buffer   */
    FKBUFF_PTR,                     /* Lightweight fast kernel network buffer */
    /* Do not add new ptr types */
} NBuffPtrType_t;

                                    /* 2lsbits in pointer encode NbuffType_t  */
#define NBUFF_TYPE_MASK             0x3u
#define NBUFF_PTR_MASK              (~NBUFF_TYPE_MASK)
#define NBUFF_PTR_TYPE(pNBuff)      ((uint32_t)(pNBuff) & NBUFF_TYPE_MASK)


#define IS_SKBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == SKBUFF_PTR )
#define IS_FPBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == FPBUFF_PTR )
#define IS_TGBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == TGBUFF_PTR )
#define IS_FKBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == FKBUFF_PTR )


#endif  /* defined(__NBUFF_TYPES_H_INCLUDED__) */
