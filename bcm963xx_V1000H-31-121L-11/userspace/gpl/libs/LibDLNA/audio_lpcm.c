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
static dlna_profile_t lpcm = {
  "LPCM",
  NULL,
  LABEL_AUDIO_2CH
};

static dlna_profile_t lpcm_low = {
  "LPCM_low",
  NULL,
  LABEL_AUDIO_2CH
};

audio_profile_t
audio_profile_guess_lpcm (AVCodecContext *ac)
{
  if (!ac)
    return AUDIO_PROFILE_INVALID;

  /* check for 16-bit signed network-endian PCM codec  */
  if (ac->codec_id != CODEC_ID_PCM_S16BE &&
      ac->codec_id != CODEC_ID_PCM_S16LE)
    return AUDIO_PROFILE_INVALID;

  /* supported channels: mono or stereo */
  if (ac->channels > 2)
    return AUDIO_PROFILE_INVALID;

  /* supported sampling rate: 8 kHz -> 48 kHz */
  if (ac->sample_rate < 8000 || ac->sample_rate > 48000)
    return AUDIO_PROFILE_INVALID;
  
  return AUDIO_PROFILE_LPCM;
}

static dlna_profile_t *
probe_lpcm (AVFormatContext *ctx dlna_unused,
            dlna_container_type_t st dlna_unused,
            av_codecs_t *codecs)
{
  static dlna_profile_t p;
  char mime[128];

  if (!stream_ctx_is_audio (codecs))
    return NULL;

  if (audio_profile_guess_lpcm (codecs->ac) != AUDIO_PROFILE_LPCM)
    return NULL;

  if (codecs->ac->sample_rate <= 32000)
    memcpy (&p, &lpcm_low, sizeof (lpcm_low));
  else
    memcpy (&p, &lpcm, sizeof (lpcm));
  sprintf (mime, "%s;rate=%d;channels=%d",
           MIME_AUDIO_LPCM, codecs->ac->sample_rate, codecs->ac->channels);
  p.mime = strdup (mime);
  
  return &p;
}

dlna_registered_profile_t dlna_profile_audio_lpcm = {
  DLNA_PROFILE_AUDIO_LPCM,
  DLNA_CLASS_AUDIO,
  "pcm,lpcm,wav,aiff",
  probe_lpcm,
  NULL
};
