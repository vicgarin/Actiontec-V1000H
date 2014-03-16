/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007 Benjamin Zores <ben@geexbox.org>
 *
 * This file is part of libdlna.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>

#include "dlna_internals.h"
#include "profiles.h"

/* MPEG-1 video with 2 channel MPEG-1 Layer2 audio
   encapsulated in MPEG-1 system */
static dlna_profile_t mpeg1 = {
  "MPEG1",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t *
probe_mpeg1 (AVFormatContext *ctx dlna_unused,
             dlna_container_type_t st dlna_unused,
             av_codecs_t *codecs)
{
  if (!stream_ctx_is_av (codecs))
    return NULL;
  
  /* check for MPEG-1 video codec */
  if (codecs->vc->codec_id != CODEC_ID_MPEG1VIDEO)
    return NULL;

  /* video bitrate must be CBR at 1,151,929.1 bps */
  if (codecs->vc->bit_rate != 1150000)
    return NULL;

  /* supported resolutions:
     - 352x288 @ 25 Hz (PAL)
     - 352x240 @ 29.97 Hz (NTSC)
     - 352x240 @ 23.976 Hz
  */
  if (codecs->vc->width == 352 && codecs->vc->height == 288)
  {
    if (codecs->vs->r_frame_rate.num != 25 &&
        codecs->vs->r_frame_rate.den != 1)
          return NULL;
  }
  else if (codecs->vc->width == 352 && codecs->vc->height == 240)
  {
    if ((codecs->vs->r_frame_rate.num != 30000 &&
         codecs->vs->r_frame_rate.den != 1001) ||
        (codecs->vs->r_frame_rate.num != 24000 &&
         codecs->vs->r_frame_rate.den != 1001))
          return NULL;
  }
  else
    return NULL;

  /* check for MPEG-1 Layer-2 audio codec */
  if (codecs->ac->codec_id != CODEC_ID_MP2)
    return NULL;
  
  /* supported channels: stereo only */
  if (codecs->ac->channels != 2)
    return NULL;

  /* supported sampling rate: 44.1 kHz only */
  if (codecs->ac->sample_rate != 44100)
    return NULL;

  /* supported bitrate: 224 Kbps only */
  if (codecs->ac->bit_rate != 224000)
    return NULL;

  return &mpeg1;
}

dlna_registered_profile_t dlna_profile_av_mpeg1 = {
    DLNA_PROFILE_AV_MPEG1,
    DLNA_CLASS_AV,
    "mpg,mpeg,mpe,m1v",
    probe_mpeg1,
    NULL
};
