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

#ifndef __PKTCMF_6816_PUBLIC_H_INCLUDED__
#define __PKTCMF_6816_PUBLIC_H_INCLUDED__

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

/* Redefined from unexported header: ./broadcom/net/enet/impl2/bcmenet.h */
#define BRCM_TAG_ETH_TYPE           0x8874  /* BRCM_TYPE */
#define BRCM_TAG_LENGTH             6

#define PKTCMF_ALL_ENTRIES          (-1)


/*
 * Interface: Single callback entry point into Packet CMF subsystem
 * from CMF Control utility or other kernel modules.
 */
typedef enum {
    CMF_DECL(PKTCMF_IF_STATUS)
    CMF_DECL(PKTCMF_IF_RESET)
    CMF_DECL(PKTCMF_IF_INIT)
    CMF_DECL(PKTCMF_IF_ENABLE)
    CMF_DECL(PKTCMF_IF_DISABLE)
    CMF_DECL(PKTCMF_IF_PRE_SYSTEMRESET)
    CMF_DECL(PKTCMF_IF_POST_SYSTEMRESET)
    CMF_DECL(PKTCMF_IF_FLUSH)
    CMF_DECL(PKTCMF_IF_DEBUG)
    CMF_DECL(PKTCMF_IF_PRINT)
    CMF_DECL(PKTCMF_IF_UNITTEST)
    CMF_DECL(PKTCMF_IF_CONFIG)
    CMF_DECL(PKTCMF_IF_TRAFFIC)
    CMF_DECL(PKTCMF_IF_SET_ASPF)
    CMF_DECL(PKTCMF_IF_GET_ASPF)
    CMF_DECL(PKTCMF_IF_SET_TPID)
    CMF_DECL(PKTCMF_IF_GET_TPID)
    CMF_DECL(PKTCMF_IF_CMF_CFG_MISS)
    CMF_DECL(PKTCMF_IF_LABFLOW)
    CMF_DECL(PKTCMF_IF_FCBCTRL)
    CMF_DECL(PKTCMF_IF_SET_PADLEN)
    CMF_DECL(PKTCMF_IF_GET_PADLEN)
    CMF_DECL(PKTCMF_IF_SET_GBL_STATUS_RL)
    CMF_DECL(PKTCMF_IF_GET_GBL_STATUS_RL)
    CMF_DECL(PKTCMF_IF_SET_TCICFG_RL)
    CMF_DECL(PKTCMF_IF_GET_TCICFG_RL)
    CMF_DECL(PKTCMF_IF_CFG_RL)
    CMF_DECL(PKTCMF_IF_GET_RL)
} pktCmf_if_t;

extern int pktCmf_isEnabled(void);

extern int pktCmfIf( pktCmf_if_t callback, int *parg1, int *parg2 );


/*
 *------------------------------------------------------------------------------
 *                  Enet Switch Driver Hooks
 *  Ethernet switch driver will register the appropriate handlers to
 *  manage the Switch ports. CMF will invoke these handlers when CMF is
 *  initialized.
 *------------------------------------------------------------------------------
 */
extern HOOKV pktCmfSaveSwitchPortState;      /* Binding with Switch ENET */
extern HOOKV pktCmfRestoreSwitchPortState;   /* Binding with Switch ENET */
extern HOOKV pktCmfResetSwitch;              /* LAB trigger of reset_switch() */

#endif /* __PKTCMF_6816_PUBLIC_H_INCLUDED__ */
