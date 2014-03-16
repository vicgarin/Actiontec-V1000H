/*
<:copyright-broadcom

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          5300 California Avenue
          Irvine, California 92617
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

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
    }

#endif /* _BCM_LOG_MODULES_ */
