/***********************************************************************
 *
 *  Copyright (c) 2011  Actiontec Electronics Inc.
 *  All Rights Reserved
 *
 *  This file is to store all functions that developed by Actiontec Electronics
 *  in addition to routines provided by Broadcom. All additional routines that 
 *  are missing from oal_timestamp.c file will locate in this file. 
 *
 ************************************************************************/

#include <time.h>
#include <string.h>

#include "cms.h"

#if defined(AEI_VDSL_CUSTOMER_NCS)
CmsRet AEI_oalTms_getGUIDateTime(char *buf, UINT32 bufLen)
{
    int c;
    time_t now;
    struct tm *tmp;

    now = time(NULL);

    tmp = localtime(&now);
    memset(buf, 0, bufLen);
    c = strftime(buf, bufLen, "%A, %B %d, %Y - %I:%M %p", tmp);
    if ((c == 0) || (c+1 > bufLen))
    {
        /* buf was not long enough */
        return CMSRET_RESOURCE_EXCEEDED;
    }
    return CMSRET_SUCCESS;
}
#endif

