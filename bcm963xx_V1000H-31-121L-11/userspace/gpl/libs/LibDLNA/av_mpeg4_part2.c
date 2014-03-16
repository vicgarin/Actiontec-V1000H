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
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <avcodec.h>

#include "dlna_internals.h"
#include "profiles.h"
#include "containers.h"

typedef enum {
  MPEG4_VCODEC_INVALID,
  MPEG4_VCODEC_H263,
  MPEG4_VCODEC_P2,
} mpeg4_vcodec_type_t;

static const struct {
  int codec_id;
  mpeg4_vcodec_type_t type;
} avf_vcodec_mapping[] = {
  { CODEC_ID_H263,                      MPEG4_VCODEC_H263 },
  { CODEC_ID_H263I,                     MPEG4_VCODEC_H263 },
  { CODEC_ID_H263P,                     MPEG4_VCODEC_H263 },
  { CODEC_ID_MPEG4,                     MPEG4_VCODEC_P2 },
  { CODEC_ID_MSMPEG4V1,                 MPEG4_VCODEC_P2 },
  { CODEC_ID_MSMPEG4V2,                 MPEG4_VCODEC_P2 },
  { CODEC_ID_MSMPEG4V3,                 MPEG4_VCODEC_P2 },
  { -1, 0 }
};
   
typedef enum {
  MPEG4_VIDEO_PROFILE_INVALID,
  MPEG4_VIDEO_PROFILE_H263,
  MPEG4_VIDEO_PROFILE_P2_SP_L0B,
  MPEG4_VIDEO_PROFILE_P2_SP_L2,
  MPEG4_VIDEO_PROFILE_P2_SP_L3,
  MPEG4_VIDEO_PROFILE_P2_SP_L3_VGA,
  MPEG4_VIDEO_PROFILE_P2_ASP_L4,
  MPEG4_VIDEO_PROFILE_P2_ASP_L5
} mpeg4_video_profile_t;

typedef struct video_properties_s {
  int width;
  int height;
  int fps_num;
  int fps_den;
} video_properties_t;

/* H263 Resolutions (all <= 15 fps) */
static video_properties_t profile_h263_res[] = {
  { 176, 144, 15, 1 }, /* QCIF */
  { 128, 96,  15, 1 }  /* SQCIF */
};

/* MPEG-4 SP L0B Resolutions (all <= 15 fps) */
static video_properties_t profile_p2_sp_l0b_res[] = {
  { 176, 144, 15, 1 }, /* QCIF */
  { 128, 96,  15, 1 }  /* SQCIF */
};

/* MPEG-4 SP L2 Resolutions */
static video_properties_t profile_p2_sp_l2_res[] = {
  { 352, 288, 15, 1 }, /* CIF */
  { 320, 240, 15, 1 }, /* QVGA 4:3 */
  { 320, 180, 15, 1 }, /* QVGA 16:9 */
  { 176, 144, 30, 1 }, /* QCIF */
  { 128, 96,  30, 1 }  /* SQCIF */
};

/* MPEG-4 SP L3_VGA Resolutions */
static video_properties_t profile_p2_sp_l3_vga_res[] = {
  { 640, 480, 30, 1 }, /* VGA */
  { 640, 360, 30, 1 }  /* VGA 16:9 */
};

/* MPEG-4 L3 / CO Resolutions (all <= 30 fps) */
static video_properties_t profile_p2_sp_l3_co_res[] = {
  { 352, 288, 30, 1 }, /* CIF, 625SIF */
  { 352, 240, 30, 1 }, /* 525SIF */
  { 320, 240, 30, 1 }, /* QVGA 4:3 */
  { 320, 180, 30, 1 }, /* QVGA 16:9 */
  { 240, 180, 30, 1 }, /* 1/7 VGA 4:3 */
  { 208, 160, 30, 1 }, /* 1/9 VGA 4:3 */
  { 176, 144, 30, 1 }, /* QCIF,625QCIF */
  { 176, 120, 30, 1 }, /* 525QCIF */
  { 160, 120, 30, 1 }, /* SQVGA 4:3 */
  { 160, 112, 30, 1 }, /* 1/16 VGA 4:3 */
  { 160, 90,  30, 1 }, /* SQVGA 16:9 */
  { 128, 96,  30, 1 }  /* SQCIF */
};

/* MPEG-4 ASP L4 SO Resolutions (all <= 30 fps) */
static video_properties_t profile_p2_asp_l4_res[] = {
  { 352, 576, 30, 1 }, /* 625 1/2 D1 */
  { 352, 480, 30, 1 }, /* 525 1/2 D1 */
  { 352, 288, 30, 1 }, /* CIF, 625SIF */
  { 352, 240, 30, 1 }, /* 525SIF */
  { 320, 240, 30, 1 }, /* QVGA 4:3 */
  { 320, 180, 30, 1 }, /* QVGA 16:9 */
  { 240, 180, 30, 1 }, /* 1/7 VGA 4:3 */
  { 208, 160, 30, 1 }, /* 1/9 VGA 4:3 */
  { 176, 144, 30, 1 }, /* QCIF,625QCIF */
  { 176, 120, 30, 1 }, /* 525QCIF */
  { 160, 120, 30, 1 }, /* SQVGA 4:3 */
  { 160, 112, 30, 1 }, /* 1/16 VGA 4:3 */
  { 160, 90,  30, 1 }, /* SQVGA 16:9 */
  { 128, 96,  30, 1 }  /* SQCIF */
};

/* MPEG-4 ASP L5 Resolutions (all <= 30 fps) */
static video_properties_t profile_p2_asp_l5_res[] = {
  { 720, 576, 30, 1 }, /* 625 D1 */
  { 720, 480, 30, 1 }, /* 525 D1 */
  { 704, 576, 30, 1 }, /* 625 4SIF */
  { 704, 480, 30, 1 }, /* 525 4SIF */
  { 640, 480, 30, 1 }, /* VGA */
  { 640, 360, 30, 1 }, /* VGA 16:9 */
  { 544, 576, 30, 1 }, /* 625 3/4 D1 */
  { 544, 480, 30, 1 }, /* 525 3/4 D1 */
  { 480, 576, 30, 1 }, /* 625 2/3 D1 */
  { 480, 480, 30, 1 }, /* 525 2/3 D1 */
  { 480, 360, 30, 1 }, /* 9/16 VGA 4:3 */
  { 480, 270, 30, 1 }, /* 9/16 VGA 16:9 */
  { 352, 576, 30, 1 }, /* 625 1/2 D1 */
  { 352, 480, 30, 1 }, /* 525 1/2 D1 */
  { 352, 288, 30, 1 }, /* CIF, 625SIF */
  { 352, 240, 30, 1 }, /* 525SIF */
  { 320, 240, 30, 1 }, /* QVGA 4:3 */
  { 320, 180, 30, 1 }, /* QVGA 16:9 */
  { 240, 180, 30, 1 }, /* 1/7 VGA 4:3 */
  { 208, 160, 30, 1 }, /* 1/9 VGA 4:3 */
  { 176, 144, 30, 1 }, /* QCIF,625QCIF */
  { 176, 120, 30, 1 }, /* 525QCIF */
  { 160, 120, 30, 1 }, /* SQVGA 4:3 */
  { 160, 112, 30, 1 }, /* 1/16 VGA 4:3 */
  { 160, 90,  30, 1 }, /* SQVGA 16:9 */
  { 128, 96,  30, 1 }  /* SQCIF */
};

/********************/
/* MPEG-4 Container */
/********************/

/* Profile for MPEG-4 Part 2 Simple Profile with AAC LC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_aac = {
    "MPEG4_P2_MP4_SP_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with HE AAC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_heaac = {
    "MPEG4_P2_MP4_SP_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with ATRAC3plus audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_atrac3plus = {
    "MPEG4_P2_MP4_SP_ATRAC3plus",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with AAC LTP audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_aac_ltp = {
    "MPEG4_P2_MP4_SP_AAC_LTP",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile Level 2 with AAC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_l2_aac = {
    "MPEG4_P2_MP4_SP_L2_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

/* Profile for MPEG-4 Part 2 Simple Profile Level 2 with AMR audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_l2_amr = {
    "MPEG4_P2_MP4_SP_L2_AMR",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

/* Profile for MPEG-4 Part 2 Simple Profile Level 3+ with AAC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_vga_aac = {
    "MPEG4_P2_MP4_SP_VGA_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Simple Profile Level 3+ with HEAAC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_sp_vga_heaac = {
    "MPEG4_P2_MP4_SP_VGA_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AAC LC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_aac = {
    "MPEG4_P2_MP4_ASP_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with HEAAC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_heaac = {
    "MPEG4_P2_MP4_ASP_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with HEAAC
   multi-channel audio, encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_heaac_mult5 = {
    "MPEG4_P2_MP4_ASP_HEAAC_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with ATRAC3plus audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_actrac3plus = {
  "MPEG4_P2_MP4_ASP_ATRAC3plus",
  MIME_VIDEO_MPEG_4,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 5
   with only Simple Object with AAC LC audio, encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_l5_so_aac = {
  "MPEG4_P2_MP4_ASP_L5_SO_AAC",
  MIME_VIDEO_MPEG_4,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 5
   with only Simple Object with HEAAC audio, encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_l5_so_heaac = {
  "MPEG4_P2_MP4_ASP_L5_SO_HEAAC",
  MIME_VIDEO_MPEG_4,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 5
   with only Simple Object with HEAAC multichannel audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_l5_so_heaac_mult5 = {
    "MPEG4_P2_MP4_ASP_L5_SO_HEAAC_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 4
   with only Simple Object with AAC LC audio, encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_l4_so_aac = {
  "MPEG4_P2_MP4_ASP_L4_SO_AAC",
  MIME_VIDEO_MPEG_4,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 4
   with only Simple Object with HEAAC audio, encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_l4_so_heaac = {
  "MPEG4_P2_MP4_ASP_L4_SO_HEAAC",
  MIME_VIDEO_MPEG_4,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 4
   with only Simple Object with HEAAC multichannel audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_p2_mp4_asp_l4_so_heaac_mult5 = {
  "MPEG4_P2_MP4_ASP_L4_SO_HEAAC_MULT5",
  MIME_VIDEO_MPEG_4,
  LABEL_VIDEO_CIF30
};

/* Profile for H263 Profile 0 Level 10 with AAC LC audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_h263_mp4_p0_l10_aac = {
  "MPEG4_H263_MP4_P0_L10_AAC",
  MIME_VIDEO_3GP,
  LABEL_VIDEO_QCIF15
};

/* Profile for H263 Profile 0 Level 10 with AAC LTP audio,
   encapsulated in MP4 */
static dlna_profile_t mpeg4_h263_mp4_p0_l10_aac_ltp = {
  "MPEG4_H263_MP4_P0_L10_AAC_LTP",
  MIME_VIDEO_3GP,
  LABEL_VIDEO_QCIF15
};

/*********************/
/* MPEG-TS Container */
/*********************/

/* Profile for MPEG-4 Part 2 Simple Profile with AAC LC audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_aac = {
  "MPEG4_P2_TS_SP_AAC",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with AAC LC audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_aac_t = {
  "MPEG4_P2_TS_SP_AAC_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with AAC LC audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_aac_iso = {
  "MPEG4_P2_TS_SP_AAC_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with MPEG-1 Layer3 audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_mpeg1_l3 = {
  "MPEG4_P2_TS_SP_MPEG1_L3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with MPEG-1 Layer3 audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_mpeg1_l3_t = {
  "MPEG4_P2_TS_SP_MPEG1_L3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with MPEG-1 Layer3 audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_mpeg1_l3_iso = {
  "MPEG4_P2_TS_SP_MPEG1_L3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with AC3 multichannel audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_ac3 = {
  "MPEG4_P2_TS_SP_AC3_L3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with AC3 multichannel audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_ac3_t = {
  "MPEG4_P2_TS_SP_AC3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with AC3 multichannel audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_ac3_iso = {
  "MPEG4_P2_TS_SP_AC3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with MPEG-1/2 Layer 1/2
   multichannel audio, encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_mpeg2_l2 = {
  "MPEG4_P2_TS_SP_MPEG2_L2",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with MPEG-1/2 Layer1/2
   multichannel audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_mpeg2_l2_t = {
  "MPEG4_P2_TS_SP_MPEG2_L2_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Simple Profile with MPEG-1/2 Layer1/2
   multichannel audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_sp_mpeg2_l2_iso = {
  "MPEG4_P2_TS_SP_MPEG2_L2_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AAC LC audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_aac = {
  "MPEG4_P2_TS_ASP_AAC",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AAC LC audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_aac_t = {
  "MPEG4_P2_TS_ASP_AAC_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AAC LC audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_aac_iso = {
  "MPEG4_P2_TS_ASP_AAC_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with MPEG-1 Layer3 audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_mpeg1_l3 = {
  "MPEG4_P2_TS_ASP_MPEG1_L3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with MPEG-1 Layer3 audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_mpeg1_l3_t = {
  "MPEG4_P2_TS_ASP_MPEG1_L3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with MPEG-1 Layer3 audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_mpeg1_l3_iso = {
  "MPEG4_P2_TS_ASP_MPEG1_L3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AC3
   multichannel audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_ac3 = {
  "MPEG4_P2_TS_ASP_AC3_L3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AC3
   multichannel audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_ac3_t = {
  "MPEG4_P2_TS_ASP_AC3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile with AC3
   multichannel audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_asp_ac3_iso = {
  "MPEG4_P2_TS_ASP_AC3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Core Profile with AC3 multichannel audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_co_ac3 = {
  "MPEG4_P2_TS_CO_AC3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Core Profile with AC3 multichannel audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_co_ac3_t = {
  "MPEG4_P2_TS_CO_AC3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Core Profile with AC3 multichannel audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_co_ac3_iso = {
  "MPEG4_P2_TS_CO_AC3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Core Profile with MPEG-1/2 Layer1/2 audio,
   encapsulated in MPEG2-TS with a zero timestamp field */
static dlna_profile_t mpeg4_p2_ts_co_mpeg2_l2 = {
  "MPEG4_P2_TS_CO_MPEG2_L2",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Core Profile with MPEG-1/2 Layer1/2 audio,
   encapsulated in MPEG2-TS with a valid timestamp field */
static dlna_profile_t mpeg4_p2_ts_co_mpeg2_l2_t = {
  "MPEG4_P2_TS_CO_MPEG2_L2_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Core Profile with MPEG-1/2 Layer1/2 audio,
   encapsulated in MPEG2-TS without a timestamp field */
static dlna_profile_t mpeg4_p2_ts_co_mpeg2_l2_iso = {
  "MPEG4_P2_TS_CO_MPEG2_L2_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

/*****************/
/* ASF Container */
/*****************/

/* Profile for MPEG-4 Part 2 Simple Profile with G.726 audio,
   encapsulated in ASF */
static dlna_profile_t mpeg4_p2_asf_sp_g726 = {
  "MPEG4_P2_ASF_SP_G726",
  MIME_VIDEO_ASF,
  LABEL_VIDEO_CIF30
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 5
   with only Simple Object with G.726 audio, encapsulated in ASF */
static dlna_profile_t mpeg4_p2_asf_asp_l5_so_g726 = {
  "MPEG4_P2_ASF_ASP_L5_SO_G726",
  MIME_VIDEO_ASF,
  LABEL_VIDEO_SD
};

/* Profile for MPEG-4 Part 2 Advanced Simple Profile up to Level 4
   with only Simple Object with G.726 audio, encapsulated in ASF */
static dlna_profile_t mpeg4_p2_asf_asp_l4_so_g726 = {
  "MPEG4_P2_ASF_ASP_L4_SO_G726",
  MIME_VIDEO_ASF,
  LABEL_VIDEO_CIF30
};

/******************/
/* 3GPP Container */
/******************/

/* Profile for H.263 Profile 0 Level 10 with AMR-WB+ audio,
   encapsulated in 3GPP */
static dlna_profile_t mpeg4_h263_3gpp_p0_l10_amr_wbplus = {
    "MPEG4_H263_3GPP_P0_L10_AMR_WBplus",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

/* Profile for MPEG-4 Part 2 Simple Profile Level 0b with AAC audio,
   encapsulated in 3GPP */
static dlna_profile_t mpeg4_p2_3gpp_sp_l0b_aac = {
    "MPEG4_P2_3GPP_SP_L0B_AAC",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

/* Profile for MPEG-4 Part 2 Simple Profile Level 0b with AMR audio,
   encapsulated in 3GPP */
static dlna_profile_t mpeg4_p2_3gpp_sp_l0b_amr = {
    "MPEG4_P2_3GPP_SP_L0B_AMR",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

/* Profile for MPEG-4 H.263 Profile 3 Level 10 with AMR audio,
   encapsulated in 3GPP */
static dlna_profile_t mpeg4_h263_3gpp_p3_l10_amr = {
    "MPEG4_H263_3GPP_P3_L10_AMR",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

static const struct {
  dlna_profile_t *profile;
  dlna_container_type_t st;
  mpeg4_video_profile_t vp;
  audio_profile_t ap;
} mpeg4_profiles_mapping[] = {
  /* MPEG-4 Container */
  { &mpeg4_p2_mp4_sp_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_mp4_sp_heaac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AAC_HE_L2 },
  { &mpeg4_p2_mp4_sp_atrac3plus, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_ATRAC },
  { &mpeg4_p2_mp4_sp_aac_ltp, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AAC_LTP },

  { &mpeg4_p2_mp4_sp_l2_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L2, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_mp4_sp_l2_amr, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L2, AUDIO_PROFILE_AMR },

  { &mpeg4_p2_mp4_sp_vga_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L3_VGA, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_mp4_sp_vga_heaac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_SP_L3_VGA, AUDIO_PROFILE_AAC_HE_L2 },

  { &mpeg4_p2_mp4_asp_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_mp4_asp_heaac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC_HE_L2 },
  { &mpeg4_p2_mp4_asp_heaac_mult5, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC_HE_MULT5 },
  { &mpeg4_p2_mp4_asp_actrac3plus, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_ATRAC },

  { &mpeg4_p2_mp4_asp_l5_so_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_mp4_asp_l5_so_heaac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC_HE_L2 },
  { &mpeg4_p2_mp4_asp_l5_so_heaac_mult5, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC_HE_MULT5 },

  { &mpeg4_p2_mp4_asp_l4_so_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_mp4_asp_l4_so_heaac, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_AAC_HE_L2 },
  { &mpeg4_p2_mp4_asp_l4_so_heaac_mult5, CT_MP4,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_AAC_HE_MULT5 },

  { &mpeg4_h263_mp4_p0_l10_aac, CT_MP4,
    MPEG4_VIDEO_PROFILE_H263, AUDIO_PROFILE_AAC },
  { &mpeg4_h263_mp4_p0_l10_aac_ltp, CT_MP4,
    MPEG4_VIDEO_PROFILE_H263, AUDIO_PROFILE_AAC_LTP },

  /* MPEG-TS Container */
  { &mpeg4_p2_ts_sp_aac, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_ts_sp_aac_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_ts_sp_aac_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AAC },

  { &mpeg4_p2_ts_sp_mpeg1_l3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_MP3 },
  { &mpeg4_p2_ts_sp_mpeg1_l3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_MP3 },
  { &mpeg4_p2_ts_sp_mpeg1_l3_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_MP3 },

  { &mpeg4_p2_ts_sp_ac3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AC3 },
  { &mpeg4_p2_ts_sp_ac3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AC3 },
  { &mpeg4_p2_ts_sp_ac3_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_AC3 },

  { &mpeg4_p2_ts_sp_mpeg2_l2, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_MP2 },
  { &mpeg4_p2_ts_sp_mpeg2_l2_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_MP2 },
  { &mpeg4_p2_ts_sp_mpeg2_l2_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_MP2 },

  { &mpeg4_p2_ts_asp_aac, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_ts_asp_aac_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC },
  { &mpeg4_p2_ts_asp_aac_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AAC },

  { &mpeg4_p2_ts_asp_mpeg1_l3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_MP3 },
  { &mpeg4_p2_ts_asp_mpeg1_l3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_MP3 },
  { &mpeg4_p2_ts_asp_mpeg1_l3_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_MP3 },

  { &mpeg4_p2_ts_asp_ac3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AC3 },
  { &mpeg4_p2_ts_asp_ac3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AC3 },
  { &mpeg4_p2_ts_asp_ac3_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_AC3 },

  { &mpeg4_p2_ts_co_ac3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_AC3 },
  { &mpeg4_p2_ts_co_ac3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_AC3 },
  { &mpeg4_p2_ts_co_ac3_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_AC3 },

  { &mpeg4_p2_ts_co_mpeg2_l2, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_MP2 },
  { &mpeg4_p2_ts_co_mpeg2_l2_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_MP2 },
  { &mpeg4_p2_ts_co_mpeg2_l2_iso, CT_MPEG_TRANSPORT_STREAM,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_MP2 },
  
  /* ASF Container */
  { &mpeg4_p2_asf_sp_g726, CT_ASF,
    MPEG4_VIDEO_PROFILE_P2_SP_L3, AUDIO_PROFILE_G726 },
  { &mpeg4_p2_asf_asp_l5_so_g726, CT_ASF,
    MPEG4_VIDEO_PROFILE_P2_ASP_L5, AUDIO_PROFILE_G726 },
  { &mpeg4_p2_asf_asp_l4_so_g726, CT_ASF,
    MPEG4_VIDEO_PROFILE_P2_ASP_L4, AUDIO_PROFILE_G726 },

  /* 3GPP Container */
  { &mpeg4_h263_3gpp_p0_l10_amr_wbplus, CT_3GP,
    MPEG4_VIDEO_PROFILE_H263, AUDIO_PROFILE_AMR_WB },
  { &mpeg4_p2_3gpp_sp_l0b_aac, CT_3GP,
    MPEG4_VIDEO_PROFILE_P2_SP_L0B, AUDIO_PROFILE_AAC },   
  { &mpeg4_p2_3gpp_sp_l0b_amr, CT_3GP,
    MPEG4_VIDEO_PROFILE_P2_SP_L0B, AUDIO_PROFILE_AMR },
  { &mpeg4_h263_3gpp_p3_l10_amr, CT_3GP,
    MPEG4_VIDEO_PROFILE_H263, AUDIO_PROFILE_AMR },
  
  { NULL, 0, 0, 0 }
};

static mpeg4_vcodec_type_t
mpeg4_get_vcodec (AVCodecContext *vc)
{
  int i;

  if (!vc)
    return MPEG4_VCODEC_INVALID;
  
  for (i = 0; avf_vcodec_mapping[i].codec_id != -1; i++)
    if (vc->codec_id == (unsigned int) avf_vcodec_mapping[i].codec_id)
      return avf_vcodec_mapping[i].type;

  return MPEG4_VCODEC_INVALID;
}

static inline int
is_valid_video_profile (video_properties_t res[], int size,
                        AVStream *vs, AVCodecContext *vc)
{
  int i;

  for (i = 0; i < size / (int) sizeof (video_properties_t); i++)
  {
    if (res[i].width == vc->width &&
        res[i].height == vc->height &&
        (vs->r_frame_rate.num / vs->r_frame_rate.num)
        <= (res[i].fps_num / res[i].fps_den))
      return 1;
  }

  return 0;
}

static mpeg4_video_profile_t
mpeg4_video_get_profile (mpeg4_vcodec_type_t vctype,
                         AVStream *vs, AVCodecContext *vc)
{
  if (!vs || !vc)
    return MPEG4_VIDEO_PROFILE_INVALID;

  if (vctype == MPEG4_VCODEC_H263)
  {
    if (vc->bit_rate > 64000) /* max bitrate is 64 kbps */
      return MPEG4_VIDEO_PROFILE_INVALID;

    if (is_valid_video_profile (profile_h263_res,
                                sizeof (profile_h263_res), vs, vc))
      return MPEG4_VIDEO_PROFILE_H263;

    return MPEG4_VIDEO_PROFILE_INVALID;
  }
  else if (vctype == MPEG4_VCODEC_P2)
  {
    if (vc->bit_rate <= 128000) /* SP_L2 and SP_L0B */
    {
      if (is_valid_video_profile (profile_p2_sp_l0b_res,
                                  sizeof (profile_p2_sp_l0b_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_SP_L0B;

      if (is_valid_video_profile (profile_p2_sp_l2_res,
                                  sizeof (profile_p2_sp_l2_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_SP_L2;

      return MPEG4_VIDEO_PROFILE_INVALID;
    }
    else if (vc->bit_rate <= 384000) /* SP_L3 */
    {
      if (is_valid_video_profile (profile_p2_sp_l3_co_res,
                                  sizeof (profile_p2_sp_l3_co_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_SP_L3;

      return MPEG4_VIDEO_PROFILE_INVALID;
    }
    else if (vc->bit_rate <= 2000000) /* CO and ASP_L4 */
    {
      if (is_valid_video_profile (profile_p2_sp_l3_co_res,
                                  sizeof (profile_p2_sp_l3_co_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_ASP_L4;

      if (is_valid_video_profile (profile_p2_asp_l4_res,
                                  sizeof (profile_p2_asp_l4_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_ASP_L4;

      return MPEG4_VIDEO_PROFILE_INVALID;
    }
    else if (vc->bit_rate <= 3000000) /* SP_L3_VGA */
    {
      if (is_valid_video_profile (profile_p2_sp_l3_vga_res,
                                  sizeof (profile_p2_sp_l3_vga_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_SP_L3_VGA;

      return MPEG4_VIDEO_PROFILE_INVALID;
     }
    else if (vc->bit_rate <= 8000000) /* ASP_L5 */
    {
      if (is_valid_video_profile (profile_p2_asp_l5_res,
                                  sizeof (profile_p2_asp_l5_res), vs, vc))
        return MPEG4_VIDEO_PROFILE_P2_ASP_L5;

      return MPEG4_VIDEO_PROFILE_INVALID;
    }

    return MPEG4_VIDEO_PROFILE_INVALID;
  }

  return MPEG4_VIDEO_PROFILE_INVALID;
}

static dlna_profile_t *
probe_mpeg4_part2 (AVFormatContext *ctx,
                   dlna_container_type_t st,
                   av_codecs_t *codecs)
{
  mpeg4_vcodec_type_t vctype;
  mpeg4_video_profile_t vp;
  audio_profile_t ap;
  int i;

  if (!stream_ctx_is_av (codecs))
    return NULL;
  
  vctype = mpeg4_get_vcodec (codecs->vc);
  if (vctype == MPEG4_VCODEC_INVALID)
    return NULL;

  /* check for a supported container */
  if (st != CT_ASF &&
      st != CT_3GP &&
      st != CT_MP4 &&
      st != CT_MPEG_TRANSPORT_STREAM &&
      st != CT_MPEG_TRANSPORT_STREAM_DLNA &&
      st != CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS)
    return NULL;

  /* ensure we have a valid video codec bit rate */
  if (codecs->vc->bit_rate == 0)
    codecs->vc->bit_rate = codecs->ac->bit_rate ?
      ctx->bit_rate - codecs->ac->bit_rate : ctx->bit_rate;

  /* check for valid video profile */
  vp = mpeg4_video_get_profile (vctype, codecs->vs, codecs->vc);
  if (vp == MPEG4_VIDEO_PROFILE_INVALID)
    return NULL;
  
  /* check for valid audio profile */
  ap = audio_profile_guess (codecs->ac);
  if (ap == AUDIO_PROFILE_INVALID)
    return NULL;

  /* AAC fixup: _320 profiles are audio-only profiles */
  if (ap == AUDIO_PROFILE_AAC_320)
    ap = AUDIO_PROFILE_AAC;
  if (ap == AUDIO_PROFILE_AAC_HE_L2_320)
    ap =  AUDIO_PROFILE_AAC_HE_L2;
  
  /* find profile according to container type, video and audio profiles */
  for (i = 0; mpeg4_profiles_mapping[i].profile; i++)
    if (mpeg4_profiles_mapping[i].st == st &&
        mpeg4_profiles_mapping[i].vp == vp &&
        mpeg4_profiles_mapping[i].ap == ap)
      return mpeg4_profiles_mapping[i].profile;
 
  return NULL;
}

dlna_registered_profile_t dlna_profile_av_mpeg4_part2 = {
  DLNA_PROFILE_AV_MPEG4_PART2,
  DLNA_CLASS_AV,
  "mov,hdmov,mp4,3gp,3gpp,asf,mpg,mpeg,mpe,mp2t,ts",
  probe_mpeg4_part2,
  NULL
};
