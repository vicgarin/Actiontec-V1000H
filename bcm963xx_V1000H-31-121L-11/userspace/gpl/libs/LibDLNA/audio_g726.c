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

audio_profile_t
audio_profile_guess_g726 (AVCodecContext *ac)
{
  if (!ac)
    return AUDIO_PROFILE_INVALID;

  if (ac->codec_id != CODEC_ID_ADPCM_G726)
    return AUDIO_PROFILE_INVALID;
  
  if (ac->channels != 1)
    return AUDIO_PROFILE_INVALID;

  if (ac->sample_rate != 8000)
    return AUDIO_PROFILE_INVALID;

  if (ac->bit_rate != 32000)
    return AUDIO_PROFILE_INVALID;

  return AUDIO_PROFILE_G726;
}
