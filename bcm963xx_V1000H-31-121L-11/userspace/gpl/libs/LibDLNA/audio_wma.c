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

/* WMA content (bit rate less than 193 kbps) */
static dlna_profile_t wmabase = {
  "WMABASE",
  MIME_AUDIO_WMA,
  LABEL_AUDIO_2CH
};

/* WMA content */
static dlna_profile_t wmafull = {
  "WMAFULL",
  MIME_AUDIO_WMA,
  LABEL_AUDIO_2CH
};

/* WMA professional version */
static dlna_profile_t wmapro = {
  "WMAPRO",
  MIME_AUDIO_WMA,
  LABEL_AUDIO_2CH_MULTI
};

audio_profile_t
audio_profile_guess_wma (AVCodecContext *ac)
{
  audio_profile_t ap = AUDIO_PROFILE_INVALID;
  
  if (!ac)
    return ap;

  /* check for WMA codec */
  if ( (ac->codec_id != CODEC_ID_WMAV1) && 
      (ac->codec_id != CODEC_ID_WMAV2) && 
      (ac->codec_id != CODEC_ID_WMAPRO))
    return ap;

    if(ac->channels <= 2)
    {
        if (ac->sample_rate <= 48000)    
        {
            if (ac->bit_rate <= 193000)
                /* WMA Baseline: stereo, up to 48 KHz, up to 192,999 bps */
                return AUDIO_PROFILE_WMA_BASELINE;
            else if (ac->bit_rate <= 385000)
                /* WMA Full: stereo, up to 48 KHz, up to 385 Kbps */
                return AUDIO_PROFILE_WMA_FULL;
        }
    }
  
  if(ac->channels <= 8)
  {
      if((ac->sample_rate <= 96000) && 
          (ac->bit_rate <= 1500000))
      {
          /* WMA Professional: up to 7.1 channels, up to 1.5 Mbps and 96 KHz */
            return AUDIO_PROFILE_WMA_PRO;
      }
  }

/* ======================================================
 * The following is the original code that we got 
 * ======================================================
 */
 // if (ac->sample_rate <= 48000)
 // {
 //   if (ac->bit_rate <= 193000)
 //   {
 //     /* WMA Baseline: stereo, up to 48 KHz, up to 192,999 bps */
 //     if (ac->channels <= 2)
 //       return AUDIO_PROFILE_WMA_BASELINE;
 //   }
 //   else if (ac->bit_rate <= 385000)
//    {
 //     /* WMA Full: stereo, up to 48 KHz, up to 385 Kbps */
 //     if (ac->channels <= 2)
 //       return AUDIO_PROFILE_WMA_FULL;
 //   }
 // }
 // else if (ac->sample_rate <= 96000)
 // {
 //   /* WMA Professional: up to 7.1 channels, up to 1.5 Mbps and 96 KHz */
 //   if (ac->channels <= 8 && ac->bit_rate <= 1500000)
 //     return AUDIO_PROFILE_WMA_PRO;
 // }

  return AUDIO_PROFILE_INVALID;
}

static dlna_profile_t *
probe_wma (AVFormatContext *ctx dlna_unused,
           dlna_container_type_t st,
           av_codecs_t *codecs)
{
  if (!stream_ctx_is_audio (codecs))
    return NULL;

  /* check for supported container */
  if (st != CT_ASF)
    return NULL;
  
  switch (audio_profile_guess_wma (codecs->ac))
  {
  case AUDIO_PROFILE_WMA_BASELINE:
    return &wmabase;
  case AUDIO_PROFILE_WMA_FULL:
    return &wmafull;
  case AUDIO_PROFILE_WMA_PRO:
    return &wmapro;
  default:
    break;
  }

  return NULL;
}

dlna_registered_profile_t dlna_profile_audio_wma = {
  DLNA_PROFILE_AUDIO_WMA,
  DLNA_CLASS_AUDIO,
  "wma,asf",
  probe_wma,
  NULL
};
