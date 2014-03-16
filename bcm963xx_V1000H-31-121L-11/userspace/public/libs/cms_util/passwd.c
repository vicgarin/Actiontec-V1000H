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

#include "cms.h"
#include "cms_util.h"
#include "oal.h"
#include <time.h>

int i64c(int i)
{
    if (i <= 0)
        return ('.');
    if (i == 1)
        return ('/');
    if (i >= 2 && i < 12)
        return ('0' - 2 + i);
    if (i >= 12 && i < 38)
        return ('A' - 12 + i);
    if (i >= 38 && i < 63)
        return ('a' - 38 + i);
    return ('z');
}


char *cmsUtil_cryptMakeSalt(void)
{
    time_t now;
    static unsigned long x;
    static char result[3];

    time(&now);
    x += now + oal_getPid() + clock();
    result[0] = i64c(((x >> 18) ^ (x >> 6)) & 077);
    result[1] = i64c(((x >> 12) ^ x) & 077);
    result[2] = '\0';
    
    return result;
}


// function to support passowrd hashing
char *cmsUtil_pwEncrypt(const char *clear, const char *salt)
{
    static char cipher[BUFLEN_24];
    char *cp;

#ifdef CONFIG_FEATURE_SHA1_PASSWORDS
    if (strncmp(salt, "$2$", 3) == 0) {
        return sha1_crypt(clear);
    }
#endif

    cp = oalPass_crypt(clear, salt);
    /* if crypt (a nonstandard crypt) returns a string too large,
       truncate it so we don't overrun buffers and hope there is
       enough security in what's left */
    if (strlen(cp) > sizeof(cipher)-1)
    {
        cp[sizeof(cipher)-1] = 0;
    }
    strcpy(cipher, cp);

    return cipher;
}
