/******************************************************************************
 *    (c)2009 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 * USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: containers.c $
 * $brcm_Revision: 2 $
 * $brcm_Date: 8/5/09 1:13p $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: /AppLibs/dlna/dms/Dependencies/LibDLNA/containers.c $
 * 
 * 2   8/5/09 1:13p ajitabhp
 * PR55165: Fixed the code for reading the timestamps.
 * 
 *****************************************************************************/
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <avformat.h>

#include "containers.h"
#include "profiles.h"

#define MPEG_PACK_HEADER 0xba
#define MPEG_TS_SYNC_CODE 0x47
#define MPEG_TS_PACKET_LENGTH 188
#define MPEG_TS_PACKET_LENGTH_DLNA 192 /* prepends 4 bytes to TS packet */

#ifndef WIN32
#ifndef stricmp
#define stricmp strcasecmp
#endif
#endif

static const struct {
  const char *name;
  const dlna_container_type_t type;
} avf_format_mapping[] = {
  { "image2",                     CT_IMAGE         },
  { "asf",                        CT_ASF           },
  { "amr",                        CT_AMR           },
  { "aac",                        CT_AAC           },
  { "ac3",                        CT_AC3           },
  { "mp3",                        CT_MP3           },
  { "wav",                        CT_WAV           },
  { "mov,mp4,m4a,3gp,3g2,mj2",    CT_MOV           },
  { "mpeg",                       CT_FF_MPEG       },
  { "mpegts",                     CT_FF_MPEG_TS    },
  { NULL, 0 }
};

static dlna_container_type_t
mpeg_find_container_type (const char *filename)
{
  unsigned char buffer[2*MPEG_TS_PACKET_LENGTH_DLNA+1];
  int i;
  FILE *fd;

  /* read file header */
  fd = fopen(filename, "rb");
  fread(buffer, 2 * MPEG_TS_PACKET_LENGTH_DLNA,1,fd); 
  fclose (fd);

  /* check for MPEG-TS container */
  for (i = 0; i < MPEG_TS_PACKET_LENGTH; i++)
  {
    if (buffer[i] == MPEG_TS_SYNC_CODE)
    {
      if (buffer[i + MPEG_TS_PACKET_LENGTH] == MPEG_TS_SYNC_CODE)
        return CT_MPEG_TRANSPORT_STREAM;
    }
  }

  /* check for DLNA specific MPEG-TS container */
  for (i = 0; i < MPEG_TS_PACKET_LENGTH_DLNA; i++)
  {
    if (buffer[i] == MPEG_TS_SYNC_CODE)
    {
      if (buffer[i + MPEG_TS_PACKET_LENGTH_DLNA] == MPEG_TS_SYNC_CODE)
      {
#ifdef ORIGINAL_CODE
        if (buffer[i] == 0x00 && buffer [i+1] == 0x00 &&
            buffer [i+2] == 0x00 && buffer [i+3] == 0x00)
#else
        i = i + MPEG_TS_PACKET_LENGTH_DLNA;
        if (buffer[i-1] == 0x00 && buffer [i-2] == 0x00 &&
            buffer [i-3] == 0x00 && buffer [i-4] == 0x00)
#endif
          return CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS; /* empty timestamp */
        else
          return CT_MPEG_TRANSPORT_STREAM_DLNA; /* valid timestamp */
      }
    }
  }

  /* check for MPEG-PS and MPEG-(P)ES container */
  if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x01)
    return (buffer[3] == MPEG_PACK_HEADER) ?
      CT_MPEG_PROGRAM_STREAM : CT_MPEG_ELEMENTARY_STREAM;

  return CT_UNKNOWN;
}

static dlna_container_type_t
mov_find_container_type (const char *filename)
{
  if (!filename)
    return CT_UNKNOWN;

  if (!stricmp (get_file_extension (filename), "3gp") ||
      !stricmp (get_file_extension (filename), "3gpp") ||
      !stricmp (get_file_extension (filename), "3g2"))
    return CT_3GP;

  return CT_MP4;
}

dlna_container_type_t
stream_get_container (AVFormatContext *ctx)
{
  int i;

#ifdef HAVE_DEBUG
  fprintf (stderr, "Found container: %s\n", ctx->iformat->name);
#endif /* HAVE_DEBUG */
  
  for (i = 0; avf_format_mapping[i].name; i++)
    if (!strcmp (ctx->iformat->name, avf_format_mapping[i].name))
    {
      switch (avf_format_mapping[i].type)
      {
      case CT_FF_MPEG:
      case CT_FF_MPEG_TS:
        return mpeg_find_container_type (ctx->filename);
      case CT_MOV:
        return mov_find_container_type (ctx->filename);
      default:
        return avf_format_mapping[i].type;
      }
    }

  /* unknown or invalid container */
  return CT_UNKNOWN;
}
