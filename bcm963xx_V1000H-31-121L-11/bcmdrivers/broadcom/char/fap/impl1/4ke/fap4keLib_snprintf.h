#ifndef __FAP4KE_SNPRINTF_H_INCLUDED__
#define __FAP4KE_SNPRINTF_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom Corporation
    All Rights Reserved
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2, as published by
 the Free Software Foundation (the "GPL").
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 
 A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
 
:>
*/

/*
 *******************************************************************************
 * File Name  : fap4ke_snprintf.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

int fap4ke_snprintf(char *buf,int len,const char *templat,...);
int fap4ke_xvsprintf(char *outbuf,const char *templat,va_list marker);

#endif  /* defined(__FAP4KE_SNPRINTF_H_INCLUDED__) */
