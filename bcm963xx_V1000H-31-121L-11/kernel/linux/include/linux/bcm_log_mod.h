/*
<:copyright-gpl 
 Copyright 2010 Broadcom Corp. All Rights Reserved. 
 
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
#ifndef _BCM_LOG_MODULES_
#define _BCM_LOG_MODULES_

typedef enum {
    BCM_LOG_LEVEL_ERROR=0,
    BCM_LOG_LEVEL_NOTICE,
    BCM_LOG_LEVEL_INFO,
    BCM_LOG_LEVEL_DEBUG,
    BCM_LOG_LEVEL_MAX
} bcmLogLevel_t;

/* To support a new module, create a new log ID in bcmLogId_t,
   and a new entry in BCM_LOG_MODULE_INFO */


typedef enum {
    BCM_LOG_ID_LOG=0,
    BCM_LOG_ID_VLAN,
    BCM_LOG_ID_GPON,
    BCM_LOG_ID_PLOAM,
    BCM_LOG_ID_PLOAM_FSM,
    BCM_LOG_ID_PLOAM_HAL,
    BCM_LOG_ID_PLOAM_PORT,
    BCM_LOG_ID_PLOAM_ALARM,
    BCM_LOG_ID_OMCI,
    BCM_LOG_ID_I2C,
    BCM_LOG_ID_ENET,
    BCM_LOG_ID_CMF,
    BCM_LOG_ID_CMFAPI,
    BCM_LOG_ID_CMFNAT,
    BCM_LOG_ID_CMFHAL,
    BCM_LOG_ID_CMFHW,
    BCM_LOG_ID_CMFHWIF,
    BCM_LOG_ID_CMFFFE,
    BCM_LOG_ID_GPON_SERDES,
    BCM_LOG_ID_FAP,
    BCM_LOG_ID_FAPPROTO,
    BCM_LOG_ID_FAP4KE,
    BCM_LOG_ID_AE,
    BCM_LOG_ID_XTM,
    BCM_LOG_ID_VOICE_EPT,
    BCM_LOG_ID_VOICE_XDRV,
    BCM_LOG_ID_VOICE_BOS,
    BCM_LOG_ID_VOICE_XDRV_SLIC,
    BCM_LOG_ID_IQ,
    BCM_LOG_ID_BPM,
    BCM_LOG_ID_ARL,
    BCM_LOG_ID_MAX
} bcmLogId_t;

#define BCM_LOG_MODULE_INFO                             \
    {                                                   \
        {.logId = BCM_LOG_ID_LOG, .name = "bcmlog", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_VLAN, .name = "vlan", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_GPON, .name = "gpon", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM, .name = "ploam", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_FSM, .name = "ploamFsm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_HAL, .name = "ploamHal", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_PORT, .name = "ploamPort", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_PLOAM_ALARM, .name = "ploamAlarm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_OMCI, .name = "omci", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_I2C, .name = "i2c", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_ENET, .name = "enet", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMF, .name = "pktcmf", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMFAPI, .name = "cmfapi", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMFNAT, .name = "cmfnat", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMFHAL, .name = "cmfhal", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMFHW, .name = "cmfhw", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMFHWIF, .name = "cmfhwif", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_CMFFFE, .name = "cmfffe", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_GPON_SERDES, .name = "gponSerdes", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_FAP, .name = "fap", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_FAPPROTO, .name = "fapProto", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_FAP4KE, .name = "fap4ke", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_AE, .name = "ae", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_XTM, .name = "xtm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_VOICE_EPT, .name = "ept", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_VOICE_XDRV, .name = "xdrv", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_VOICE_BOS, .name = "bos", .logLevel = BCM_LOG_LEVEL_ERROR}, \
        {.logId = BCM_LOG_ID_VOICE_XDRV_SLIC, .name = "xdrv_slic", .logLevel = BCM_LOG_LEVEL_DEBUG}, \
        {.logId = BCM_LOG_ID_IQ, .name = "iq", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_BPM, .name = "bpm", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
        {.logId = BCM_LOG_ID_ARL, .name = "arl", .logLevel = BCM_LOG_LEVEL_NOTICE}, \
    }

/* To support a new registered function,
 * create a new BCM_FUN_ID */

typedef enum {
    BCM_FUN_ID_RESET_SWITCH=0,
    BCM_FUN_ID_ENET_LINK_CHG,
    BCM_FUN_ID_ENET_CHECK_SWITCH_LOCKUP,
    BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY,
    BCM_FUN_ID_MAX

} bcmFunId_t;

#endif /* _BCM_LOG_MODULES_ */
