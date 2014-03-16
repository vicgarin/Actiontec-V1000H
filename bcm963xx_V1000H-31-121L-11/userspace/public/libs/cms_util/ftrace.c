/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
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


#ifdef SUPPORT_FTRACE
#include "cms.h"
#include "cms_util.h"



void cmsFtr_enable(void)
{
   cmsFil_writeToProc(FTRACE_ON, "1");
}

void cmsFtr_disable(void)
{
   cmsFil_writeToProc(FTRACE_ON, "0");
}

void cmsFtr_insertMarker(const char *s)
{
   cmsFil_writeToProc(FTRACE_INSERT_MARKER, s);
}

void cmsFtr_setTraceOption(const char *s)
{
   cmsFil_writeToProc(FTRACE_TRACE_OPTIONS, s);
}

void cmsFtr_doFunctionTracing(void)
{
   cmsFil_writeToProc(FTRACE_CURRENT_TRACER, "function");
}

void cmsFtr_doFunctionGraphTracing(void)
{
   cmsFil_writeToProc(FTRACE_CURRENT_TRACER, "function_graph");
}

void cmsFtr_doNopTracing(void)
{
   cmsFil_writeToProc(FTRACE_CURRENT_TRACER, "nop");
}

#endif /* SUPPORT_FTRACE */
