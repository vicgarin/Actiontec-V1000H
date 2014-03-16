/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
# 
# 
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License, version 2, as published by  
# the Free Software Foundation (the "GPL"). 
# 
#
# 
# This program is distributed in the hope that it will be useful,  
# but WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
# GNU General Public License for more details. 
#  
# 
#  
#   
# 
# A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by 
# writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
# Boston, MA 02111-1307, USA. 
#
 * 
 ************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include "../oal.h"

CmsRet oal_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val)
{
   CmsRet ret=CMSRET_SUCCESS;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtol(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


CmsRet oal_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val)
{
   CmsRet ret=CMSRET_SUCCESS;
   char *localEndPtr=NULL;

   /*
    * Linux strtoul allows a minus sign in front of the number.
    * This seems wrong to me.  Specifically check for this and reject
    * such strings.
    */
   while (isspace(*str))
   {
      str++;
   }
   if (*str == '-')
   {
      if (endptr)
      {
         *endptr = (char *) str;
      }
      *val = 0;
      return CMSRET_INVALID_ARGUMENTS;
   }

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoul(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


CmsRet oal_strtol64(const char *str, char **endptr, SINT32 base, SINT64 *val)
{
   CmsRet ret=CMSRET_SUCCESS;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoll(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


CmsRet oal_strtoul64(const char *str, char **endptr, SINT32 base, UINT64 *val)
{
   CmsRet ret=CMSRET_SUCCESS;
   char *localEndPtr=NULL;

   /*
    * Linux strtoul allows a minus sign in front of the number.
    * This seems wrong to me.  Specifically check for this and reject
    * such strings.
    */
   while (isspace(*str))
   {
      str++;
   }
   if (*str == '-')
   {
      if (endptr)
      {
         *endptr = (char *) str;
      }
      *val = 0;
      return CMSRET_INVALID_ARGUMENTS;
   }

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoull(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = CMSRET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


