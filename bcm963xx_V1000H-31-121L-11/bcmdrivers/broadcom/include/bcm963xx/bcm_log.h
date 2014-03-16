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
#ifndef _BCM_LOG_SERVICES_
#define _BCM_LOG_SERVICES_

#if defined(__KERNEL__)
#include "bcm_OS_Deps.h"
#else
#include <stdint.h>             /**< ISO C99 7.18 Integer Types */
#include <stdio.h>
#include <string.h>
#endif

#include "bcm_log_mod.h"

#if defined(__KERNEL__)
#define bcmPrint            printk
#else
#define bcmPrint            printf
#define BUG()
#define EXPORT_SYMBOL(sym)
#endif

/*********
 *********
 * Private:
 *********
 *********/

/*
 *------------------------------------------------------------------------------
 * Color encodings for console printing:
 *
 * To enable  color coded console printing: #define COLOR(clr_code)  clr_code
 * To disable color coded console printing: #define COLOR(clr_code)
 *
 * You may select a color specific to your subsystem by:
 *  #define CLRsys CLRg
 *
 *------------------------------------------------------------------------------
 */

#define IN /*Input parameters*/
#define OUT /*Output parameters*/
#define INOUT /*Input/Output parameters*/

/* Defines the supported funtionality */
#undef BCM_LOG_COLOR_SUPPORTED
#define BCM_ASSERT_SUPPORTED
#define BCM_LOG_SUPPORTED
#define BCM_DATADUMP_SUPPORTED
#define BCM_ERROR_SUPPORTED
#undef BCM_SNAPSHOT_SUPPORTED

#if !defined(COLOR)

#if defined(BCM_LOG_COLOR_SUPPORTED)
#define COLOR(clr_code)     clr_code
#else
#define COLOR(clr_code)
#endif

/* White background */
#define CLRr                COLOR("\e[0;31m")       /* red              */
#define CLRg                COLOR("\e[0;32m")       /* green            */
#define CLRy                COLOR("\e[0;33m")       /* yellow           */
#define CLRb                COLOR("\e[0;34m")       /* blue             */
#define CLRm                COLOR("\e[0;35m")       /* magenta          */
#define CLRc                COLOR("\e[0;36m")       /* cyan             */

/* blacK "inverted" background */
#define CLRrk               COLOR("\e[0;31;40m")    /* red     on blacK */
#define CLRgk               COLOR("\e[0;32;40m")    /* green   on blacK */
#define CLRyk               COLOR("\e[0;33;40m")    /* yellow  on blacK */
#define CLRmk               COLOR("\e[0;35;40m")    /* magenta on blacK */
#define CLRck               COLOR("\e[0;36;40m")    /* cyan    on blacK */
#define CLRwk               COLOR("\e[0;37;40m")    /* white   on blacK */

/* Colored background */
#define CLRcb               COLOR("\e[0;36;44m")    /* cyan    on blue  */
#define CLRyr               COLOR("\e[0;33;41m")    /* yellow  on red   */
#define CLRym               COLOR("\e[0;33;45m")    /* yellow  on magen */

/* Generic foreground colors */
#define CLRhigh             CLRm                    /* Highlight color  */
#define CLRbold             CLRcb                   /* Bold      color  */
#define CLRbold2            CLRym                   /* Bold2     color  */
#define CLRerr              CLRwk                   /* Error     color  */
#define CLRnorm             COLOR("\e[0m")          /* Normal    color  */
#define CLRnl               CLRnorm "\n"            /* Normal + newline */

#endif

#if defined(BCM_ASSERT_SUPPORTED)
#define BCM_ASSERTCODE(code)    code
#else
#define BCM_ASSERTCODE(code)
#endif /*defined(BCM_ASSERT_SUPPORTED)*/

#if defined(BCM_LOG_SUPPORTED)
#define BCM_LOGCODE(code)    code
#else
#define BCM_LOGCODE(code)
#endif /*defined(BCM_LOG_SUPPORTED)*/

#if defined(BCM_ERROR_SUPPORTED)
#define BCM_ERRORCODE(code)    code
#else
#define BCM_ERRORCODE(code)
#endif /*defined(BCM_ERROR_SUPPORTED)*/

#if defined(BCM_DATADUMP_SUPPORTED)
#define BCM_DATADUMPCODE(code)    code
#else
#define BCM_DATADUMPCODE(code) 0
#endif /*defined(BCM_DATADUMP_SUPPORTED)*/

#if defined(BCM_SNAPSHOT_SUPPORTED)
#define BCM_SNAPSHOTCODE(code)    code
#else
#define BCM_SNAPSHOTCODE(code) 0
#endif /*defined(BCM_SNAPSHOT_SUPPORTED)*/

typedef enum {
    BCM_LOG_DD_IMPORTANT=0,
    BCM_LOG_DD_INFO,
    BCM_LOG_DD_DETAIL,
    BCM_LOG_DD_MAX
} bcmLogDataDumpLevel_t;

typedef struct {
    bcmLogId_t logId;
    char *name;
    bcmLogLevel_t logLevel;
    bcmLogDataDumpLevel_t ddLevel;
} bcmLogModuleInfo_t;



/********
 ********
 * Public: service API offered by LOGdriver to other drivers
 ********
 ********/

/**
 * Logging API: Activate by #defining BCM_LOG_SUPPORTED
 **/

#if defined(BCM_LOG_SUPPORTED)
bcmLogModuleInfo_t *bcmLog_logIsEnabled(bcmLogId_t logId, bcmLogLevel_t logLevel);
#endif

#define BCM_LOG_FUNC(logId)                     \
    BCM_LOG_DEBUG((logId), " ")

#define BCM_LOG_DEBUG(logId, fmt, arg...)                               \
    BCM_LOGCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_logIsEnabled(logId, BCM_LOG_LEVEL_DEBUG); \
                      if (_pModInfo)                                              \
                          bcmPrint(CLRm "[DBG " "%s" "] %-10s: " fmt CLRnl, \
                                 _pModInfo->name, __FUNCTION__, ##arg); } while(0) )

#define BCM_LOG_INFO(logId, fmt, arg...)                               \
    BCM_LOGCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_logIsEnabled(logId, BCM_LOG_LEVEL_INFO); \
                      if (_pModInfo)                                              \
                          bcmPrint(CLRg "[INF " "%s" "] %-10s: " fmt CLRnl, \
                                 _pModInfo->name, __FUNCTION__, ##arg); } while(0) )

#define BCM_LOG_NOTICE(logId, fmt, arg...)                               \
    BCM_LOGCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_logIsEnabled(logId, BCM_LOG_LEVEL_NOTICE); \
                      if (_pModInfo)                                              \
                          bcmPrint(CLRb "[NTC " "%s" "] %-10s: " fmt CLRnl, \
                                 _pModInfo->name, __FUNCTION__, ##arg); } while(0) )


/**
 * Error Reporting API: Activate by #defining BCM_ERROR_SUPPORTED
 **/

#define BCM_LOG_ERROR(logId, fmt, arg...)                                \
    BCM_ERRORCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_logIsEnabled(logId, BCM_LOG_LEVEL_ERROR); \
                      if (_pModInfo)                                              \
                          bcmPrint(CLRerr "[ERROR " "%s" "] %-10s,%d: " fmt CLRnl, \
                                 _pModInfo->name, __FUNCTION__, __LINE__, ##arg); } while(0) )


/**
 * Assert API: Activate by #defining BCM_ASSERT_SUPPORTED
 **/

#define BCM_ASSERT(cond)                                         \
    BCM_ASSERTCODE( if ( !(cond) ) {                                    \
                        bcmPrint(CLRerr "[ASSERT " "%s" "] %-10s,%d: " #cond CLRnl, \
                               __FILE__, __FUNCTION__, __LINE__); \
                        BUG();                                          \
                     } )


/**
 * Datadump API: Activate by #defining BCM_DATADUMP_SUPPORTED
 **/

/*
 * Prototype of datadump print functions.
 * Note: parse functions must be exported (EXPORT_SYMBOL)
 */
typedef int (Bcm_DataDumpPrintFunc)(uint32_t dataDumpId, IN void* dataPtr, uint32_t numDataBytes,
                                    OUT char* buf, uint32_t bufSize);

#if defined(BCM_DATADUMP_SUPPORTED)
bcmLogModuleInfo_t *bcmLog_ddIsEnabled(bcmLogId_t logId, bcmLogDataDumpLevel_t ddLevel);
void bcm_dataDumpRegPrinter(uint32_t qId, uint32_t dataDumpId, Bcm_DataDumpPrintFunc *printFun);
void bcm_dataDump(uint32_t qID, uint32_t dataDumpID, const char* dataDumpName, void *ptr, uint32_t numBytes);
uint32_t bcm_dataDumpCreateQ(const char* qName);
void bcm_dataDumpDeleteQ(uint32_t qid);
#endif

/*
 * Create a DataDump queue. Different modules can share a queue.
 * Returns a queue ID (uint32_t).
 */
#define BCM_DATADUMP_CREATE_Q(qName) BCM_DATADUMPCODE(bcm_dataDumpCreateQ(qName))

/*
 * Delete a DataDump queue.
 */
#define BCM_DATADUMP_DELETE_Q(qID) BCM_DATADUMPCODE(bcm_dataDumpDeleteQ(qID))

/*
 * Dump data
 */
#define BCM_DATADUMP_IMPORTANT(logId, qID, dataDumpID, ptr, numBytes) \
    BCM_DATADUMPCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_ddIsEnabled(logId, BCM_LOG_DD_IMPORTANT); \
                      if (_pModInfo)                                              \
                          bcm_dataDump(qID, dataDumpID, #dataDumpID, (void*)(ptr), numBytes); } while(0) )
#define BCM_DATADUMP_INFO(logId, qID, dataDumpID, ptr, numBytes) \
    BCM_DATADUMPCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_ddIsEnabled(logId, BCM_LOG_DD_INFO); \
                      if (_pModInfo)                                              \
                          bcm_dataDump(qID, dataDumpID, #dataDumpID, (void*)(ptr), numBytes); } while(0) )
#define BCM_DATADUMP_DETAIL(logId, qID, dataDumpID, ptr, numBytes) \
    BCM_DATADUMPCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_ddIsEnabled(logId, BCM_LOG_DD_DETAIL); \
                      if (_pModInfo)                                              \
                          bcm_dataDump(qID, dataDumpID, #dataDumpID, (void*)(ptr), numBytes); } while(0) )
#define BCM_DATADUMP_MAX(logId, qID, dataDumpID, ptr, numBytes) \
    BCM_DATADUMPCODE( do { bcmLogModuleInfo_t *_pModInfo = bcmLog_ddIsEnabled(logId, BCM_LOG_DD_MAX); \
                      if (_pModInfo)                                              \
                          bcm_dataDump(qID, dataDumpID, #dataDumpID, (void*)(ptr), numBytes); } while(0) )

/*
 * Register a printer for a certain DataDump ID.
 * Datadumps for which no printer is registered will use a default printer.
 * The default printer will print the data as an array of bytes.
 */
#define BCM_DATADUMP_REG_PRINTER(qId, dataDumpId, printFun)             \
    BCM_DATADUMPCODE(bcm_dataDumpRegPrinter(qId, dataDumpId, printFun))

/* A helper macro for datadump printers */
#define DDPRINTF(buf, len, bufSize, arg...)                             \
    ({len += snprintf((buf)+(len), max_t(uint32_t, 0, (bufSize)-80-(len)), ##arg); \
        if ((len) >= (bufSize)-80) snprintf((buf)+(len), 80, "---BUFFER FULL---\n");})


/**
 * Snapshot API: Commit all logs to the Snapshot queue
 **/

#define BCM_LOG_SNAPSHOT() BCM_SNAPSHOTCODE() /*TBD*/


/**
 * API Function Prototypes
 **/

void bcmLog_setGlobalLogLevel(bcmLogLevel_t logLevel);
bcmLogLevel_t bcmLog_getGlobalLogLevel(void);

void bcmLog_setLogLevel(bcmLogId_t logId, bcmLogLevel_t logLevel);
bcmLogLevel_t bcmLog_getLogLevel(bcmLogId_t logId);

char *bcmLog_getModName(bcmLogId_t logId);

#endif /*_BCM_LOG_SERVICES_*/
