/***********************************************************************
 *
 *  Copyright (c) 2011  Actiontec Electronics Inc.
 *  All Rights Reserved
 *
 *  This file is to store all functions that developed by Actiontec Electronics
 *  in addition to routines provided by Broadcom. All additional routines that 
 *  are missing from cms_tms.h file will locate in this file. 
 *
 ************************************************************************/

#ifndef __AEI_CMS_TMS_H__
#define __AEI_CMS_TMS_H__

#ifdef DMP_PERIODICSTATSBASE_1
CmsRet AEI_cmsTms_subXSIDateTime(const char *xsiTime1, const char *xsiTime2, UINT32 *result);
#endif
CmsRet AEI_oalTms_getGUIDateTime(char *buf, UINT32 bufLen);

#endif /* __AEI_CMS_TMS_H__ */
