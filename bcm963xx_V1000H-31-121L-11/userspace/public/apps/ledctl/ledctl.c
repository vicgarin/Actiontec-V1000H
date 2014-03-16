/***********************************************************************
 *
 *  Copyright (c) 2007-2010  Broadcom Corporation
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


#include "cms.h"
#include "cms_util.h"

#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
static void *msgHandle = NULL;
static UBOOL8 keepLooping = TRUE;
#endif

void usage(UINT32 exitCode)
{
   printf("usage: ledctl [WAN] [on|off|red]\n");
   printf("    WAN is the only led that is controllable by this app right now.\n");
   printf("    WAN must be specified.\n");
   printf("    one of on, off, or red must be specified.\n");

   exit(exitCode);
}


void processWanLed(const char *state)
{
   if (!cmsUtl_strcmp(state, "on"))
   {
      cmsLed_setWanConnected();
   }
   else if (!cmsUtl_strcmp(state, "off"))
   {
      cmsLed_setWanDisconnected();
   }
   else if (!cmsUtl_strcmp(state, "red"))
   {
      cmsLed_setWanFailed();
   }
   else if(!cmsUtl_strcmp(state, "flashgreen"))
   {
        while (keepLooping)
        {
            cmsLed_setWanConnected();
            usleep(250*1000);
            cmsLed_setWanDisconnected();
            usleep(750*1000);
        }
   }
   else
   {
      usage(1);
   }

}


int main(int argc, char *argv[])
{
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
    CmsRet ret;
    cmsLog_init(EID_LEDCTL);
    cmsLog_setLevel(DEFAULT_LOG_LEVEL);

    if ((ret = cmsMsg_init(EID_LEDCTL, &msgHandle)) != CMSRET_SUCCESS)
    {
        cmsLog_error("msg initialization failed, ret=%d", ret);
        cmsLog_cleanup();
        return 0;
    }
#endif
  
   if (argc != 3)
   {
      usage(1);
   }
cmsLog_error("++++++++%s %s",argv[1],argv[2]);
   if (!cmsUtl_strcmp(argv[1], "WAN"))
   {
      processWanLed(argv[2]);
   }
   else
   {
      usage(1);
   }
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
    cmsMsg_cleanup(&msgHandle);
    cmsLog_cleanup();
#endif
   return 0;
}
