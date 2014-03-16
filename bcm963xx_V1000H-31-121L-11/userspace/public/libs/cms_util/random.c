/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
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
#include <string.h>

#include "cms.h"
#include "cms_util.h"

static char _randstrbuf[MAX_RANDSTRING_LEN+1];
static char *randspace="abcdefghijklmnopqurstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


char *cmsRand_getRandString(UINT32 minchars, UINT32 maxchars)
{
   int i;
   int mod;

   if (maxchars > MAX_RANDSTRING_LEN)
      return NULL;

   if (maxchars < minchars)
      return NULL;

   if (minchars < 1)
      return NULL;

   memset(_randstrbuf, 0, sizeof(_randstrbuf));
   for (i=0; i < maxchars; i++)
   {
      // first char is a-z, rest of chars can be anything
      mod = (0==i) ? 26 : strlen(randspace);
      _randstrbuf[i] = randspace[rand() % mod];

      if (i+1 >= minchars)
      {
         // we've satisfied the minchars requirement, see if we break
         if (0 == (rand() % 2))
            break;
      }
   }

   return _randstrbuf;
}
