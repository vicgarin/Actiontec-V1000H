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
static dlna_profile_t mp3 = {
  "MP3",
  MIME_AUDIO_MPEG,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content with extensions
   for lower sampling rates and bitrates */
static dlna_profile_t mp3x = {
  "MP3X",
  MIME_AUDIO_MPEG,
  LABEL_AUDIO_2CH
};

static int
audio_is_valid_mp3_common (AVCodecContext *ac)
{
  if (!ac)
    return 0;

  /* check for MP3 codec */
  if (ac->codec_id != CODEC_ID_MP3)
    return 0;
  
  /* only mono and stereo are supported */
  if (ac->channels > 2)
    return 0;

  return 1;
}

static int
audio_is_valid_mp3 (AVCodecContext *ac)
{
  if (!ac)
    return 0;

  if (!audio_is_valid_mp3_common (ac))
    return 0;

  if (ac->sample_rate != 32000 &&
      ac->sample_rate != 44100 &&
      ac->sample_rate != 48000)
    return 0;
  
  switch (ac->bit_rate)
  {
  case 32000:
  case 40000:
  case 48000:
  case 56000:
  case 64000:
  case 80000:
  case 96000:
  case 112000:
  case 128000:
  case 160000:
  case 192000:
  case 224000:
  case 256000:
  case 320000:
    return 1;
  default:
      break;
  }
  
  return 0;
}

static int
audio_is_valid_mp3x (AVCodecContext *ac)
{
  if (!ac)
    return 0;

  if (!audio_is_valid_mp3_common (ac))
    return 0;

  if (ac->sample_rate != 16000 &&
      ac->sample_rate != 22050 &&
      ac->sample_rate != 24000)
    return 0;
  
  switch (ac->bit_rate)
  {
    case 8000:
    case 16000:
    case 24000:
    case 32000:
    case 40000:
    case 48000:
    case 56000:
    case 64000:
    case 80000:
    case 96000:
    case 112000:
    case 128000:
    case 160000:
    case 192000:
    case 224000:
    case 256000:
    case 320000:
    return 1;
  default:
      break;
  }
  
  return 0;
}

audio_profile_t
audio_profile_guess_mp3 (AVCodecContext *ac)
{
  audio_profile_t ap = AUDIO_PROFILE_INVALID;
  
  if (!ac)
    return ap;

  if (audio_is_valid_mp3x (ac))
    return AUDIO_PROFILE_MP3_EXTENDED;

  if (audio_is_valid_mp3 (ac))
    return AUDIO_PROFILE_MP3;
  
  return AUDIO_PROFILE_INVALID;
}

/* Audio encoding must be MPEG-1 Layer 3 */
static dlna_profile_t *
probe_mp3 (AVFormatContext *ctx dlna_unused,
           dlna_container_type_t st,
           av_codecs_t *codecs)
{
  if (!stream_ctx_is_audio (codecs))
    return NULL;

  /* check for supported container */
  if (st != CT_MP3)
    return NULL;
  
  switch (audio_profile_guess_mp3 (codecs->ac))
  {
  case AUDIO_PROFILE_MP3:
    return &mp3;
  case AUDIO_PROFILE_MP3_EXTENDED:
    return &mp3x;
  default:
    break;
  }
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_audio_mp3 = {
  DLNA_PROFILE_AUDIO_MP3,
  DLNA_CLASS_AUDIO,
  "mp3",
  probe_mp3,
  NULL
};
