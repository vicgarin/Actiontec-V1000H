/*
<:copyright-gpl
 Copyright 2007 Broadcom Corp. All Rights Reserved.

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

#ifndef __PKTCMF_6368_PUBLIC_H_INCLUDED__
#define __PKTCMF_6368_PUBLIC_H_INCLUDED__

#include "pktHdr.h"


/*
 *------------------------------------------------------------------------------
 * Common defines for Packet CMF layers.
 *------------------------------------------------------------------------------
 */
#define CMF_DECL(x)                 #x, /* for string declaration in C file */
#undef CMF_DECL
#define CMF_DECL(x)                 x,  /* for enum declaration in H file */

/* Offsets and sizes in CMF are specified as half words */
#define CMFUNIT                     (sizeof(uint16_t))

#define CMP1BYTE                    (sizeof(uint8_t))
#define CMP2BYTES                   (sizeof(uint16_t))
#define CMP4BYTES                   (sizeof(uint32_t))

/* Creation of CMF NIBBLE MASK, b0=[0..3] b1=[4..7] b2=[8..11] b3=[12..15] */
#define NBLMSK1BYTE                 0x3
#define NBLMSK3NBLS                 0x7
#define NBLMSK2BYTES                0xF


/* Explicit wrappers to callback */
extern int pktCmfSwcConfig(void);
extern int pktCmfSarConfig(int ulHwFwdTxChannel, unsigned int ulTrafficType);
extern int pktCmfSarAbort(void);


/*
 *------------------------------------------------------------------------------
 *                  Enet Switch Driver Hooks
 *  Ethernet switch driver will register the appropriate handlers to
 *  configure the SAR Port of the Switch. CMF will invoke these handlers when
 *  the SAR runtime driver becomes operational and wishes to use CMF for
 *  downstream hardware accelerated forwarding via the SAR port of the Switch.
 *------------------------------------------------------------------------------
 */
extern HOOKV pktCmfSarPortEnable;   /* Binding with Switch ENET */
extern HOOKV pktCmfSarPortDisable;  /* Binding with Switch ENET */


#endif /* __PKTCMF_6368_PUBLIC_H_INCLUDED__ */
