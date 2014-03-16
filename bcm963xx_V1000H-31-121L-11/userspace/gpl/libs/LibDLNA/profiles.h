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

#ifndef _PROFILES_H_
#define _PROFILES_H_

#ifndef inline
#define inline __inline
#endif

#include <avcodec.h>
#include <avformat.h>

#include "dlna_internals.h"
#include "containers.h"

/* DLNA MIME types */
#define MIME_IMAGE_JPEG                   "image/jpeg"
#define MIME_IMAGE_PNG                    "image/png"

#define MIME_AUDIO_3GP                    "audio/3gpp"
#define MIME_AUDIO_ADTS                   "audio/vnd.dlna.adts"
#define MIME_AUDIO_ATRAC                  "audio/x-sony-oma"
#define MIME_AUDIO_DOLBY_DIGITAL          "audio/vnd.dolby.dd-raw"
#define MIME_AUDIO_LPCM                   "audio/L16"
#define MIME_AUDIO_MPEG                   "audio/mpeg"
#define MIME_AUDIO_MPEG_4                 "audio/mp4"
#define MIME_AUDIO_WMA                    "audio/x-ms-wma"

#define MIME_VIDEO_3GP                    "video/3gpp"
#define MIME_VIDEO_ASF                    "video/x-ms-asf"
#define MIME_VIDEO_MPEG                   "video/mpeg"
#define MIME_VIDEO_MPEG_4                 "video/mp4"
#define MIME_VIDEO_MPEG_TS                "video/vnd.dlna.mpeg-tts"
#define MIME_VIDEO_WMV                    "video/x-ms-wmv"

/* DLNA Labels */
#define LABEL_IMAGE_PICTURE               "picture"
#define LABEL_IMAGE_ICON                  "icon"

#define LABEL_AUDIO_MONO                  "mono"
#define LABEL_AUDIO_2CH                   "2-ch"
#define LABEL_AUDIO_2CH_MULTI             "2-ch multi"
#define LABEL_AUDIO_MULTI                 "multi"

#define LABEL_VIDEO_CIF15                 "CIF15"
#define LABEL_VIDEO_CIF30                 "CIF30"
#define LABEL_VIDEO_QCIF15                "QCIF15"
#define LABEL_VIDEO_SD                    "SD"
#define LABEL_VIDEO_HD                    "HD"

typedef struct av_codecs_s {
  /* audio stream and codec */
  AVStream *as;
  AVCodecContext *ac;
  /* video stream and codec */
  AVStream *vs;
  AVCodecContext *vc;
} av_codecs_t;

typedef struct dlna_registered_profile_s {
  dlna_media_profile_t id;
  dlna_media_class_t class;
  char *extensions;
  dlna_profile_t * (*probe) (AVFormatContext *ctx,
                             dlna_container_type_t st,
                             av_codecs_t *codecs);
  struct dlna_registered_profile_s *next;
} dlna_registered_profile_t;

char * get_file_extension (const char *filename);

/* audio profile checks */

typedef enum {
  AUDIO_PROFILE_INVALID = 0,

  /* Advanced Audio Codec variants */
  AUDIO_PROFILE_AAC,
  AUDIO_PROFILE_AAC_320,
  AUDIO_PROFILE_AAC_MULT5,
  AUDIO_PROFILE_AAC_BSAC,  
  AUDIO_PROFILE_AAC_BSAC_MULT5,
  AUDIO_PROFILE_AAC_HE_L2,
  AUDIO_PROFILE_AAC_HE_L2_320,
  AUDIO_PROFILE_AAC_HE_L3,
  AUDIO_PROFILE_AAC_HE_MULT5,
  AUDIO_PROFILE_AAC_HE_V2_L2,
  AUDIO_PROFILE_AAC_HE_V2_L2_320,
  AUDIO_PROFILE_AAC_HE_V2_L3,
  AUDIO_PROFILE_AAC_HE_V2_MULT5,
  AUDIO_PROFILE_AAC_LTP,  
  AUDIO_PROFILE_AAC_LTP_MULT5,
  AUDIO_PROFILE_AAC_LTP_MULT7,
  
  AUDIO_PROFILE_AC3,
  AUDIO_PROFILE_AC3_EXTENDED,
  
  AUDIO_PROFILE_AMR,
  AUDIO_PROFILE_AMR_WB,
  
  AUDIO_PROFILE_ATRAC,

  AUDIO_PROFILE_G726,

  AUDIO_PROFILE_LPCM,

  /* MPEG audio variants */
  AUDIO_PROFILE_MP2,
  AUDIO_PROFILE_MP3,
  AUDIO_PROFILE_MP3_EXTENDED,

  /* Windows Media Audio variants */
  AUDIO_PROFILE_WMA_BASELINE,
  AUDIO_PROFILE_WMA_FULL,
  AUDIO_PROFILE_WMA_PRO
} audio_profile_t;

audio_profile_t audio_profile_guess (AVCodecContext *ac);

audio_profile_t audio_profile_guess_aac (AVCodecContext *ac);
audio_profile_t audio_profile_guess_ac3 (AVCodecContext *ac);
audio_profile_t audio_profile_guess_amr (AVCodecContext *ac);
audio_profile_t audio_profile_guess_atrac (AVCodecContext *ac);
audio_profile_t audio_profile_guess_g726 (AVCodecContext *ac);
audio_profile_t audio_profile_guess_lpcm (AVCodecContext *ac);
audio_profile_t audio_profile_guess_mp2 (AVCodecContext *ac);
audio_profile_t audio_profile_guess_mp3 (AVCodecContext *ac);
audio_profile_t audio_profile_guess_wma (AVCodecContext *ac);

/* stream context check routines */
int stream_ctx_is_image (AVFormatContext *ctx,
                         av_codecs_t *codecs, dlna_container_type_t st);
int stream_ctx_is_audio (av_codecs_t *codecs);
int stream_ctx_is_av (av_codecs_t *codecs);

#endif /* _PROFILES_H_ */
