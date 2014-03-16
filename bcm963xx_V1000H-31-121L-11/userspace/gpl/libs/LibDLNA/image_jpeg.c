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

/* Profile for image media class content of small resolution */
static dlna_profile_t jpeg_sm = {
    "JPEG_SM",
    MIME_IMAGE_JPEG,
    LABEL_IMAGE_PICTURE
};

/* Profile for image media class content of medium resolution */
static dlna_profile_t jpeg_med = {
    "JPEG_MED",
    MIME_IMAGE_JPEG,
    LABEL_IMAGE_PICTURE
};

/* Profile for image media class content of high resolution */
static dlna_profile_t jpeg_lrg = {
    "JPEG_LRG",
    MIME_IMAGE_JPEG,
    LABEL_IMAGE_PICTURE
};

/* Profile for image thumbnails */
static dlna_profile_t jpeg_tn = {
    "JPEG_TN",
    MIME_IMAGE_JPEG,
    LABEL_IMAGE_ICON
};

/* Profile for small icons */
static dlna_profile_t jpeg_sm_ico = {
    "JPEG_SM_ICO",
    MIME_IMAGE_JPEG,
    LABEL_IMAGE_ICON
};

/* Profile for large icons */
static dlna_profile_t jpeg_lrg_ico = {
    "JPEG_LRG_ICO",
    MIME_IMAGE_JPEG,
    LABEL_IMAGE_ICON
};

static const struct {
  dlna_profile_t *profile;
  int max_width;
  int max_height;
} jpeg_profiles_mapping[] = {
  { &jpeg_sm_ico,    48,   48 },
  { &jpeg_lrg_ico,  120,  120 },
  { &jpeg_tn,       160,  160 },
  { &jpeg_sm,       640,  480 },
  { &jpeg_med,     1024,  768 },
  { &jpeg_lrg,     4096, 4096 },
  { NULL, 0, 0 }
};

static dlna_profile_t *
probe_jpeg (AVFormatContext *ctx,
            dlna_container_type_t st,
            av_codecs_t *codecs)
{
  int i;

  if (!stream_ctx_is_image (ctx, codecs, st))
    return NULL;

  /* check for JPEG compliant codec */
  if (codecs->vc->codec_id != CODEC_ID_MJPEG &&
      codecs->vc->codec_id != CODEC_ID_MJPEGB &&
      codecs->vc->codec_id != CODEC_ID_LJPEG &&
      codecs->vc->codec_id != CODEC_ID_JPEGLS)
    return NULL;

  for (i = 0; jpeg_profiles_mapping[i].profile; i++)
    if (codecs->vc->width  <= jpeg_profiles_mapping[i].max_width &&
        codecs->vc->height <= jpeg_profiles_mapping[i].max_height)
      return jpeg_profiles_mapping[i].profile;
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_image_jpeg = {
    DLNA_PROFILE_IMAGE_JPEG,
    DLNA_CLASS_IMAGE,
    "jpg,jpe,jpeg",
    probe_jpeg,
    NULL
};
