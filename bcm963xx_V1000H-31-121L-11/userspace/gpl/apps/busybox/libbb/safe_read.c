/* vi: set sw=4 ts=4: */
/*
 * Utility routines.
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "libbb.h"



ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

#ifdef BRCM_CMS_BUILD
   struct timeval  timer;
   fd_set          readFds;
   int             width;
   int             rv;
   int             tmout=300;  /* make default timeout for shell 300 seconds */
   char *          tmout_str;

   tmout_str = getenv("TMOUT");
   if (tmout_str != NULL)
   {
       tmout = atoi(tmout_str);
   }

   while (1)
   {
      //clear bits in read bit mask
      FD_ZERO(&readFds);

      //initialize bit mask
      FD_SET(fd, &readFds);
      width = fd + 1;

      //pend, waiting for one or more fds to become ready
      if (tmout > 0)
      {
          /* set timout each time before calling select */
          timer.tv_sec  = tmout;
          timer.tv_usec = 0;

          rv = select(width, &readFds, NULL, NULL, &timer);
      }
      else
      {
          /* tmout set to 0, no timeout */
          rv = select(width, &readFds, NULL, NULL, NULL);
      }

      if (rv == 0)
      {
         printf("Shell exited due to idle timeout (%d seconds)\n", tmout);
         exit(0);
      }
      else if ((rv > 0) && FD_ISSET(fd, &readFds))
      {
         n = read(fd, buf, count);
         if (n >= 0 || errno != EINTR)
            break;
      }
   }
#else
	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

#endif  /* BRCM_CMS_BUILD */

	return n;
}


/* END CODE */
/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
