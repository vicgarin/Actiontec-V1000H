/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
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
#include "cms_ast.h"
#include "cms_log.h"
#include "prctl.h"


void cmsAst_assertFunc(const char *filename, UINT32 lineNumber, const char *exprString, SINT32 expr)
{

   if (expr == 0)
   {
      cmsLog_error("assertion \"%s\" failed at %s:%d", exprString, filename, lineNumber);

#ifndef NDEBUG
      /* Send SIGABRT only if NDEBUG is not defined */
      prctl_signalProcess(getpid(), SIGABRT);
#endif
   }

}




