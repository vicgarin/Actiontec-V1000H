/***********************************************************************
 *
 *  Copyright (c) 2011  Actiontec Electronics Inc.
 *  All Rights Reserved
 *
 *  This file is to store all functions that developed by Actiontec Electronics
 *  in addition to routines provided by Broadcom. All additional routines that 
 *  are missing from timestamp.c file will locate in this file.
 *
 ************************************************************************/

#include <time.h>

#include "cms.h"
#include "cms_tms.h"
#include "oal.h"

#ifdef DMP_PERIODICSTATSBASE_1
/* xsiTime1 minus xsiTime2, return the result in seconds */
CmsRet AEI_cmsTms_subXSIDateTime(const char *xsiTime1, const char *xsiTime2, UINT32 *result)
{
    struct tm tm1, tm2;
    time_t sec1, sec2;

    if (xsiTime1 == NULL || xsiTime2 == NULL || result == NULL)
        return CMSRET_INVALID_ARGUMENTS;

    if (strptime(xsiTime1, "%Y-%m-%dT%H:%M:%S", &tm1) == NULL ||
        strptime(xsiTime2, "%Y-%m-%dT%H:%M:%S", &tm2) == NULL )
        return CMSRET_INVALID_ARGUMENTS;

    sec1 = mktime(&tm1);
    sec2 = mktime(&tm2);

    *result = sec1 - sec2;

    return CMSRET_SUCCESS;
}
#endif /* DMP_PERIODICSTATSBASE_1 */

#if defined(AEI_VDSL_CUSTOMER_NCS)
CmsRet AEI_cmsTms_getGUIDateTime(char *buf, UINT32 bufLen)
{   
   return (AEI_oalTms_getGUIDateTime(buf, bufLen));
}   
#endif
