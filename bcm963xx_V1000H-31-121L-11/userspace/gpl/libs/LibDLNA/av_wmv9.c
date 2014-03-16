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

/* Summary of VMW9 Profiles
 * Simple and Main profiles are identified by FourCC WMV3
 * Advanced profile is identified by FourCC WVC1
 *
 * Profile      Level      Max. Bitrate      Format
 *
 * Simple      Low      96 Kbps      176 x 144 @ 15 Hz (QCIF)
 *               Medium  384 Kbps      240 x 176 @ 30 Hz
 *                                       352 x 288 @ 15 Hz (CIF)
 *
 * Main      Low      2 Mbps      320 x 240 @ 24 Hz (QVGA)
 *               Medium     10 Mbps      720 x 480 @ 30 Hz (480p)
 *                                       720 x 576 @ 25 Hz (576p)
 *               High      20 Mbps      1920 x 1080 @ 30 Hz (1080p)
 *
 * Advanced      L0      2 Mbps       352 x 288 @ 30 Hz (CIF)
 *             L1      10 Mbps      720 x 480 @ 30 Hz (NTSC-SD)
 *                                       720 x 576 @ 25 Hz (PAL-SD)
 *             L2      20 Mbps      720 x 480 @ 60 Hz (480p)
 *                                       1280 x 720 @ 30 Hz (720p)
 *               L3      45 Mbps      1920 x 1080 @ 24 Hz (1080p)
 *                                       1920 x 1080 @ 30 Hz (1080i)
 *                                       1280 x 720 @ 60 Hz (720p)
 *               L4      135 Mbps      1920 x 1080 @ 60 Hz (1080p)
 *                                       2048 x 1536 @ 24 Hz
 */

#include <stdlib.h>
#include <string.h>

#include "dlna_internals.h"
#include "profiles.h"
#include "containers.h"

typedef enum {
  WMV_VIDEO_PROFILE_INVALID,
  WMV_VIDEO_PROFILE_SIMPLE_LOW,
  WMV_VIDEO_PROFILE_SIMPLE_MEDIUM,
  WMV_VIDEO_PROFILE_MAIN_MEDIUM,
  WMV_VIDEO_PROFILE_MAIN_HIGH
} wmv_video_profile_t;

typedef struct wmv9_profile_s {
  int max_width;
  int max_height;
  int fps_num;
  int fps_den;
  int max_bitrate;
} wmv9_profile_t;

static wmv9_profile_t wmv9_profile_simple_low[] = {
  { 176, 144, 15, 1, 96000}
};

static wmv9_profile_t wmv9_profile_simple_medium[] = {
  { 240, 176, 30, 1, 384000},
  { 240, 176, 30000, 1001, 384000},
  { 352, 288, 15, 1, 384000}
};

static wmv9_profile_t wmv9_profile_main_medium[] = {
  { 720, 480, 30, 1, 10000000},
  { 720, 480, 30000, 1001, 10000000},
  { 720, 576, 25, 1, 10000000}
};

static wmv9_profile_t wmv9_profile_main_high[] = {
  { 1920, 1080, 30, 1, 20000000},
  { 1920, 1080, 30000, 1001, 20000000}
};

/* Medium resolution video (Main profile at Medium Level)
   with baseline WMA audio */
static dlna_profile_t wmvmed_base = {
    "WMVMED_BASE",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_SD
};

/* Medium resolution video (Main profile at Medium Level)
   with full WMA audio */
static dlna_profile_t wmvmed_full = {
    "WMVMED_FULL",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_SD
};

/* Medium resolution video (Main profile at Medium Level)
   with WMA professional audio */
static dlna_profile_t wmvmed_pro = {
    "WMVMED_PRO",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_HD
};

/* High resolution video (Main profile at High Level)
   with full WMA audio */
static dlna_profile_t wmvhigh_full = {
    "WMVHIGH_FULL",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_HD
};

/* High resolution video (Main profile at High Level)
   with WMA professional audio */
static dlna_profile_t wmvhigh_pro = {
    "WMVHIGH_PRO",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_HD
};

/* HighMAT profile */
static dlna_profile_t wmvhm_base dlna_unused = {
    "WMVHM_BASE",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_SD
};

/* Low resolution video (Simple Profile at Low Level)
   with baseline WMA audio */
static dlna_profile_t wmvspll_base = {
    "WMVSPLL_BASE",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_QCIF15
};

/* Low resolution video (Simple Profile at Medium Level)
   with baseline WMA audio */
static dlna_profile_t wmvspml_base = {
    "WMVSPML_BASE",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_CIF15
};

/* Low resolution video (Simple Profile at Medium Level) with MP3 audio */
static dlna_profile_t wmvspml_mp3 = {
    "WMVSPML_MP3",
    MIME_VIDEO_WMV,
    LABEL_VIDEO_CIF15
};

static const struct {
  dlna_profile_t *profile;
  wmv_video_profile_t vp;
  audio_profile_t ap;
} wmv_profiles_mapping[] = {
  { &wmvmed_base,  WMV_VIDEO_PROFILE_MAIN_MEDIUM,
                   AUDIO_PROFILE_WMA_BASELINE },
  { &wmvmed_full,  WMV_VIDEO_PROFILE_MAIN_MEDIUM,
                   AUDIO_PROFILE_WMA_FULL },
  { &wmvmed_pro,   WMV_VIDEO_PROFILE_MAIN_MEDIUM,
                   AUDIO_PROFILE_WMA_PRO },
  { &wmvhigh_full, WMV_VIDEO_PROFILE_MAIN_HIGH,
                   AUDIO_PROFILE_WMA_FULL },
  { &wmvhigh_pro,  WMV_VIDEO_PROFILE_MAIN_HIGH,
                   AUDIO_PROFILE_WMA_FULL },
  { &wmvspll_base, WMV_VIDEO_PROFILE_SIMPLE_LOW,
                   AUDIO_PROFILE_WMA_BASELINE },
  { &wmvspml_base, WMV_VIDEO_PROFILE_SIMPLE_MEDIUM,
                   AUDIO_PROFILE_WMA_BASELINE },
  { &wmvspml_mp3,  WMV_VIDEO_PROFILE_SIMPLE_MEDIUM,
                   AUDIO_PROFILE_MP3 },
  { NULL, 0, 0, }
};

static int
is_valid_wmv9_video_profile (wmv9_profile_t profile[], int size,
                             AVStream *vs, AVCodecContext *vc)
{
  int i;

  for (i = 0; i < size / (int) sizeof (wmv9_profile_t); i++)
    if (vc->width <= profile[i].max_width &&
        vc->height <= profile[i].max_height &&
        vs->r_frame_rate.num == profile[i].fps_num &&
        vs->r_frame_rate.den == profile[i].fps_den &&
        vc->bit_rate <= profile[i].max_bitrate)
      return 1;

  /* video properties do not fit the requested profile */
  return 0;
}

static wmv_video_profile_t
wmv_video_profile_get (AVStream *vs, AVCodecContext *vc)
{
  if (!vs || !vc)
    return WMV_VIDEO_PROFILE_INVALID;

  if (is_valid_wmv9_video_profile (wmv9_profile_simple_low,
                                   sizeof (wmv9_profile_simple_low), vs, vc))
    return WMV_VIDEO_PROFILE_SIMPLE_LOW;

  if (is_valid_wmv9_video_profile (wmv9_profile_simple_medium,
                                   sizeof (wmv9_profile_simple_medium),
                                   vs, vc))
    return WMV_VIDEO_PROFILE_SIMPLE_MEDIUM;

  if (is_valid_wmv9_video_profile (wmv9_profile_main_medium,
                                   sizeof (wmv9_profile_main_medium), vs, vc))
    return WMV_VIDEO_PROFILE_MAIN_MEDIUM;
  
  if (is_valid_wmv9_video_profile (wmv9_profile_main_high,
                                   sizeof (wmv9_profile_main_high), vs, vc))
    return WMV_VIDEO_PROFILE_MAIN_HIGH;
  
  return WMV_VIDEO_PROFILE_INVALID;
}

static dlna_profile_t *
probe_wmv9 (AVFormatContext *ctx dlna_unused,
            dlna_container_type_t st,
            av_codecs_t *codecs)
{
  wmv_video_profile_t vp;
  audio_profile_t ap;
  int i;

  if (!stream_ctx_is_av (codecs))
    return NULL;
  
  /* need to be in ASF container only */
  if (st != CT_ASF)
    return NULL;

  /* check for WMV3 (Simple and Main profiles) video codec */
  if (codecs->vc->codec_id != CODEC_ID_WMV3)
    return NULL;

  /* get video profile */
  vp = wmv_video_profile_get (codecs->vs, codecs->vc);
  if (vp == WMV_VIDEO_PROFILE_INVALID)
    return NULL;
  
  /* get audio profile */
  ap = audio_profile_guess (codecs->ac);
  if (ap == AUDIO_PROFILE_INVALID)
    return NULL;

  /* find profile according to container type, video and audio profiles */
  for (i = 0; wmv_profiles_mapping[i].profile; i++)
    if (wmv_profiles_mapping[i].vp == vp &&
        wmv_profiles_mapping[i].ap == ap)
      return wmv_profiles_mapping[i].profile;

  return NULL;
}

dlna_registered_profile_t dlna_profile_av_wmv9 = {
    DLNA_PROFILE_AV_WMV9,
    DLNA_CLASS_AV,
    "asf,wmv",
    probe_wmv9,
    NULL
};
