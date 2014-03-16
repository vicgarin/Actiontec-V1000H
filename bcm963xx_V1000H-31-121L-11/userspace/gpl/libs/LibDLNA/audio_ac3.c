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

/* Profile for audio media class content */
static dlna_profile_t ac3 = {
  "AC3",
  MIME_AUDIO_DOLBY_DIGITAL,
  LABEL_AUDIO_2CH_MULTI
};

audio_profile_t
audio_profile_guess_ac3 (AVCodecContext *ac)
{
  if (!ac)
    return AUDIO_PROFILE_INVALID;

  /* check for AC3 codec */
  if (ac->codec_id != CODEC_ID_AC3)
    return AUDIO_PROFILE_INVALID;
  
  /* supported channels: 1/0, 2/0, 3/0, 2/1, 3/1, 2/2, 3/2 */
  if (ac->channels > 5)
    return AUDIO_PROFILE_INVALID;

  /* supported sampling rate: 32, 44.1 and 48 kHz */
  if (ac->sample_rate != 32000 &&
      ac->sample_rate != 44100 &&
      ac->sample_rate != 48000)
    return AUDIO_PROFILE_INVALID;

  /* supported bitrate: 32 Kbps - 640 Kbps */
  if (ac->bit_rate < 32000)
    return AUDIO_PROFILE_INVALID;

  if (ac->bit_rate <= 448000)
    return AUDIO_PROFILE_AC3;

  if (ac->bit_rate <= 640000)
    return AUDIO_PROFILE_AC3_EXTENDED;

  return AUDIO_PROFILE_INVALID;
}

static dlna_profile_t *
probe_ac3 (AVFormatContext *ctx dlna_unused,
           dlna_container_type_t st,
           av_codecs_t *codecs)
{
  if (!stream_ctx_is_audio (codecs))
    return NULL;

  /* check for supported container */
  if (st != CT_AC3)
    return NULL;
  
  switch (audio_profile_guess_ac3 (codecs->ac))
  {
  case AUDIO_PROFILE_AC3:
  case AUDIO_PROFILE_AC3_EXTENDED:
    return &ac3;
  default:
    break;
  }
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_audio_ac3 = {
  DLNA_PROFILE_AUDIO_AC3,
  DLNA_CLASS_AUDIO,
  "ac3",
  probe_ac3,
  NULL
};
