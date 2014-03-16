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
  AVC_VIDEO_PROFILE_INVALID,
  AVC_VIDEO_PROFILE_BL_QCIF15,
  AVC_VIDEO_PROFILE_BL_L1B_QCIF,
  AVC_VIDEO_PROFILE_BL_L12_CIF15,
  AVC_VIDEO_PROFILE_BL_CIF15,
  AVC_VIDEO_PROFILE_BL_CIF15_520,
  AVC_VIDEO_PROFILE_BL_CIF15_540,
  AVC_VIDEO_PROFILE_BL_L2_CIF30,
  AVC_VIDEO_PROFILE_BL_CIF30,
  AVC_VIDEO_PROFILE_BL_CIF30_940,
  AVC_VIDEO_PROFILE_BL_L3L_SD,
  AVC_VIDEO_PROFILE_BL_L3_SD,
  AVC_VIDEO_PROFILE_MP_SD,
  AVC_VIDEO_PROFILE_MP_HD
} avc_video_profile_t;

typedef struct video_properties_s {
  int width;
  int height;
} video_properties_t;

static video_properties_t profile_cif_res[] = {
  { 352, 288 }, /* CIF */
  { 352, 240 }, /* 525SIF */
  { 320, 240 }, /* QVGA 4:3 */
  { 320, 180 }, /* QVGA 16:9 */
  { 240, 180 }, /* 1/7 VGA 4:3 */
  { 240, 135 }, /* 1/7 VGA 16:9 */
  { 208, 160 }, /* 1/9 VGA 4:3 */
  { 176, 144 }, /* QCIF,625QCIF */
  { 176, 120 }, /* 525QCIF */
  { 160, 120 }, /* SQVGA 4:3 */
  { 160, 112 }, /* 1/16 VGA 4:3 */
  { 160, 90  }, /* SQVGA 16:9 */
  { 128, 96  }  /* SQCIF */
};

static video_properties_t profile_mp_l3_sd_res[] = {
  { 720, 576 }, /* 625 D1 */
  { 720, 480 }, /* 525 D1 */
  { 640, 480 }, /* VGA */
  { 640, 360 }  /* VGA 16:9 */
};

static video_properties_t profile_mp_sd_res[] = {
  { 720, 576 }, /* 625 D1 */
  { 720, 480 }, /* 525 D1 */
  { 704, 576 }, /* 625 4SIF */
  { 704, 480 }, /* 525 4SIF */
  { 640, 480 }, /* VGA */
  { 640, 360 }, /* VGA 16:9 */
  { 544, 576 }, /* 625 3/4 D1 */
  { 544, 480 }, /* 525 3/4 D1 */
  { 480, 576 }, /* 625 2/3 D1 */
  { 480, 480 }, /* 525 2/3 D1 */
  { 480, 360 }, /* 9/16 VGA 4:3 */
  { 480, 270 }, /* 9/16 VGA 16:9 */
  { 352, 576 }, /* 625 1/2 D1 */
  { 352, 480 }, /* 525 1/2 D1 */
  { 352, 288 }, /* CIF, 625SIF */
  { 352, 240 }, /* 525SIF */
  { 320, 240 }, /* QVGA 4:3 */
  { 320, 180 }, /* QVGA 16:9 */
  { 240, 180 }, /* 1/7 VGA 4:3 */
  { 208, 160 }, /* 1/9 VGA 4:3 */
  { 176, 144 }, /* QCIF,625QCIF */
  { 176, 120 }, /* 525QCIF */
  { 160, 120 }, /* SQVGA 4:3 */
  { 160, 112 }, /* 1/16 VGA 4:3 */
  { 160, 90  }, /* SQVGA 16:9 */
  { 128, 96  }  /* SQCIF */
};

static video_properties_t profile_mp_hd_res[] = {
  { 1920, 1080 }, /* 1080p */
  { 1920, 1152 },
  { 1920, 540  }, /* 1080i */
  { 1280, 720  }  /* 720p */
};

/********************/
/* MPEG-4 Container */
/********************/

static dlna_profile_t avc_mp4_mp_sd_aac_mult5 = {
    "AVC_MP4_MP_SD_AAC_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_heaac_l2 = {
    "AVC_MP4_MP_SD_HEAAC_L2",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_mpeg1_l3 = {
    "AVC_MP4_MP_SD_MPEG1_L3",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_ac3 = {
    "AVC_MP4_MP_SD_AC3",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_aac_ltp = {
    "AVC_MP4_MP_SD_AAC_LTP",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_aac_ltp_mult5 = {
    "AVC_MP4_MP_SD_AAC_LTP_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_aac_ltp_mult7 = {
    "AVC_MP4_MP_SD_AAC_LTP_MULT7",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_atrac3plus = {
    "AVC_MP4_MP_SD_ATRAC3plus",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_bl_l3l_sd_aac = {
    "AVC_MP4_BL_L3L_SD_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_bl_l3l_sd_heaac = {
    "AVC_MP4_BL_L3L_SD_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_bl_l3_sd_aac = {
    "AVC_MP4_BL_L3_SD_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_mp_sd_bsac = {
    "AVC_MP4_MP_SD_BSAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_mp4_bl_cif30_aac_mult5 = {
    "AVC_MP4_BL_CIF30_AAC_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_heaac_l2 = {
    "AVC_MP4_BL_CIF30_HEAAC_L2",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_mpeg1_l3 = {
    "AVC_MP4_BL_CIF30_MPEG1_L3",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_ac3 = {
    "AVC_MP4_BL_CIF30_AC3",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_aac_ltp = {
    "AVC_MP4_BL_CIF30_AAC_LTP",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_aac_ltp_mult5 = {
    "AVC_MP4_BL_CIF30_AAC_LTP_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_l2_cif30_aac = {
    "AVC_MP4_BL_L2_CIF30_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_bsac = {
    "AVC_MP4_BL_CIF30_BSAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif30_bsac_mult5 = {
    "AVC_MP4_BL_CIF30_BSAC_MULT5",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_mp4_bl_cif15_heaac = {
    "AVC_MP4_BL_CIF15_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_cif15_amr = {
    "AVC_MP4_BL_CIF15_AMR",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_cif15_aac = {
    "AVC_MP4_BL_CIF15_AAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_cif15_aac_520 = {
    "AVC_MP4_BL_CIF15_AAC_520",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_cif15_aac_ltp = {
    "AVC_MP4_BL_CIF15_AAC_LTP",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_cif15_aac_ltp_520 = {
    "AVC_MP4_BL_CIF15_AAC_LTP_520",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_cif15_bsac = {
    "AVC_MP4_BL_CIF15_BSAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_l12_cif15_heaac = {
    "AVC_MP4_BL_L12_CIF15_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_mp4_bl_l1b_qcif15_heaac = {
    "AVC_MP4_BL_L1B_QCIF15_HEAAC",
    MIME_VIDEO_MPEG_4,
    LABEL_VIDEO_QCIF15
};

/*********************/
/* MPEG-TS Container */
/*********************/

static dlna_profile_t avc_ts_mp_sd_aac_mult5 = {
    "AVC_TS_MP_SD_AAC_MULT5",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_mult5_t = {
    "AVC_TS_MP_SD_AAC_MULT5_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_mult5_iso = {
    "AVC_TS_MP_SD_AAC_MULT5_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_heaac_l2 = {
    "AVC_TS_MP_SD_HEAAC_L2",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_heaac_l2_t = {
    "AVC_TS_MP_SD_HEAAC_L2_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_heaac_l2_iso = {
    "AVC_TS_MP_SD_HEAAC_L2_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_mpeg1_l3 = {
    "AVC_TS_MP_SD_MPEG1_L3",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_mpeg1_l3_t = {
    "AVC_TS_MP_SD_MPEG1_L3_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_mpeg1_l3_iso = {
    "AVC_TS_MP_SD_MPEG1_L3_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_ac3 = {
    "AVC_TS_MP_SD_AC3",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_ac3_t = {
    "AVC_TS_MP_SD_AC3_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_ac3_iso = {
    "AVC_TS_MP_SD_AC3_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp = {
    "AVC_TS_MP_SD_AAC_LTP",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_t = {
    "AVC_TS_MP_SD_AAC_LTP_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_iso = {
    "AVC_TS_MP_SD_AAC_LTP_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_mult5 = {
    "AVC_TS_MP_SD_AAC_LTP_MULT5",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_mult5_t = {
    "AVC_TS_MP_SD_AAC_LTP_MULT5_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_mult5_iso = {
    "AVC_TS_MP_SD_AAC_LTP_MULT5_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_mult7 = {
    "AVC_TS_MP_SD_AAC_LTP_MULT7",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_mult7_t = {
    "AVC_TS_MP_SD_AAC_LTP_MULT7_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_aac_ltp_mult7_iso = {
    "AVC_TS_MP_SD_AAC_LTP_MULT7_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_bsac = {
    "AVC_TS_MP_SD_BSAC",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_bsac_t = {
    "AVC_TS_MP_SD_BSAC_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_mp_sd_bsac_iso = {
    "AVC_TS_MP_SD_BSAC_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static dlna_profile_t avc_ts_bl_cif30_aac_mult5 = {
    "AVC_TS_BL_CIF30_AAC_MULT5",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_mult5_t = {
    "AVC_TS_BL_CIF30_AAC_MULT5_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_mult5_iso = {
    "AVC_TS_BL_CIF30_AAC_MULT5_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_heaac_l2 = {
   "AVC_TS_BL_CIF30_HEAAC_L2",
   MIME_VIDEO_MPEG_TS,
   LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_heaac_l2_t = {
  "AVC_TS_BL_CIF30_HEAAC_L2_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_heaac_l2_iso = {
  "AVC_TS_BL_CIF30_HEAAC_L2_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_mpeg1_l3 = {
    "AVC_TS_BL_CIF30_MPEG1_L3",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_mpeg1_l3_t = {
  "AVC_TS_BL_CIF30_MPEG1_L3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_mpeg1_l3_iso = {
  "AVC_TS_BL_CIF30_MPEG1_L3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_ac3 = {
  "AVC_TS_BL_CIF30_AC3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_ac3_t = {
  "AVC_TS_BL_CIF30_AC3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_ac3_iso = {
  "AVC_TS_BL_CIF30_AC3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_ltp = {
  "AVC_TS_BL_CIF30_AAC_LTP",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_ltp_t = {
  "AVC_TS_BL_CIF30_AAC_LTP_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_ltp_iso = {
  "AVC_TS_BL_CIF30_AAC_LTP_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_ltp_mult5 = {
  "AVC_TS_BL_CIF30_AAC_LTP_MULT5",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_ltp_mult5_t = {
  "AVC_TS_BL_CIF30_AAC_LTP_MULT5_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_ltp_mult5_iso = {
  "AVC_TS_BL_CIF30_AAC_LTP_MULT5_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_940 = {
  "AVC_TS_BL_CIF30_AAC_940",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_940_t = {
  "AVC_TS_BL_CIF30_AAC_940_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_bl_cif30_aac_940_iso = {
  "AVC_TS_BL_CIF30_AAC_940_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_ts_mp_hd_aac_mult5 = {
  "AVC_TS_MP_HD_AAC_MULT5",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_mult5_t = {
  "AVC_TS_MP_HD_AAC_MULT5_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_mult5_iso = {
  "AVC_TS_MP_HD_AAC_MULT5_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_heaac_l2 = {
  "AVC_TS_MP_HD_HEAAC_L2",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_heaac_l2_t = {
  "AVC_TS_MP_HD_HEAAC_L2_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_heaac_l2_iso = {
  "AVC_TS_MP_HD_HEAAC_L2_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_mpeg1_l3 = {
  "AVC_TS_MP_HD_MPEG1_L3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_mpeg1_l3_t = {
  "AVC_TS_MP_HD_MPEG1_L3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_mpeg1_l3_iso = {
  "AVC_TS_MP_HD_MPEG1_L3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_ac3 = {
  "AVC_TS_MP_HD_AC3",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_ac3_t = {
  "AVC_TS_MP_HD_AC3_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_ac3_iso = {
  "AVC_TS_MP_HD_AC3_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac = {
  "AVC_TS_MP_HD_AAC",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_t = {
  "AVC_TS_MP_HD_AAC_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_iso = {
  "AVC_TS_MP_HD_AAC_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp = {
  "AVC_TS_MP_HD_AAC_LTP",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_t = {
  "AVC_TS_MP_HD_AAC_LTP_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_iso = {
  "AVC_TS_MP_HD_AAC_LTP_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_mult5 = {
  "AVC_TS_MP_HD_AAC_LTP_MULT5",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_mult5_t = {
    "AVC_TS_MP_HD_AAC_LTP_MULT5_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_mult5_iso = {
    "AVC_TS_MP_HD_AAC_LTP_MULT5_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_mult7 = {
  "AVC_TS_MP_HD_AAC_LTP_MULT7",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_mult7_t = {
    "AVC_TS_MP_HD_AAC_LTP_MULT7_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_mp_hd_aac_ltp_mult7_iso = {
    "AVC_TS_MP_HD_AAC_LTP_MULT7_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_HD
};

static dlna_profile_t avc_ts_bl_cif15_aac = {
    "AVC_TS_BL_CIF15_AAC",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_t = {
    "AVC_TS_BL_CIF15_AAC_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_iso = {
    "AVC_TS_BL_CIF15_AAC_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_540 = {
    "AVC_TS_BL_CIF15_AAC_540",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_540_t = {
  "AVC_TS_BL_CIF15_AAC_540_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_540_iso = {
  "AVC_TS_BL_CIF15_AAC_540_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_ltp = {
  "AVC_TS_BL_CIF15_AAC_LTP",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_ltp_t = {
  "AVC_TS_BL_CIF15_AAC_LTP_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_aac_ltp_iso = {
  "AVC_TS_BL_CIF15_AAC_LTP_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_bsac = {
  "AVC_TS_BL_CIF15_BSAC",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_bsac_t = {
  "AVC_TS_BL_CIF15_BSAC_T",
  MIME_VIDEO_MPEG_TS,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_ts_bl_cif15_bsac_iso = {
  "AVC_TS_BL_CIF15_BSAC_ISO",
  MIME_VIDEO_MPEG,
  LABEL_VIDEO_CIF15
};

/******************/
/* 3GPP Container */
/******************/

static dlna_profile_t avc_3gpp_bl_cif30_amr_wbplus = {
  "AVC_3GPP_BL_CIF30_AMR_WBplus",
  MIME_VIDEO_3GP,
  LABEL_VIDEO_CIF30
};

static dlna_profile_t avc_3gpp_bl_cif15_amr_wbplus = {
  "AVC_3GPP_BL_CIF15_AMR_WBplus",
  MIME_VIDEO_3GP,
  LABEL_VIDEO_CIF15
};

static dlna_profile_t avc_3gpp_bl_qcif15_aac = {
    "AVC_3GPP_BL_QCIF15_AAC",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

static dlna_profile_t avc_3gpp_bl_qcif15_aac_ltp = {
    "AVC_3GPP_BL_QCIF15_AAC_LTP",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

static dlna_profile_t avc_3gpp_bl_qcif15_heaac = {
    "AVC_3GPP_BL_QCIF15_HEAAC",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

static dlna_profile_t avc_3gpp_bl_qcif15_amr_wbplus = {
    "AVC_3GPP_BL_QCIF15_AMR_WBplus",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

static dlna_profile_t avc_3gpp_bl_qcif15_amr = {
    "AVC_3GPP_BL_QCIF15_AMR",
    MIME_VIDEO_3GP,
    LABEL_VIDEO_QCIF15
};

static const struct {
  dlna_profile_t *profile;
  dlna_container_type_t st;
  avc_video_profile_t vp;
  audio_profile_t ap;
} avc_profiles_mapping[] = {
  /* MPEG-4 Container */
  { &avc_mp4_mp_sd_aac_mult5, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_mp4_mp_sd_heaac_l2, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_mp4_mp_sd_mpeg1_l3, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_MP3 },
  { &avc_mp4_mp_sd_ac3, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AC3 },
  { &avc_mp4_mp_sd_aac_ltp, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP },
  { &avc_mp4_mp_sd_aac_ltp_mult5, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_mp4_mp_sd_aac_ltp_mult7, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT7 },
  { &avc_mp4_mp_sd_atrac3plus, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_ATRAC },
  { &avc_mp4_mp_sd_bsac, CT_MP4,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_BSAC },
  
  { &avc_mp4_bl_l3l_sd_aac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_L3L_SD, AUDIO_PROFILE_AAC },
  { &avc_mp4_bl_l3l_sd_heaac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_L3L_SD, AUDIO_PROFILE_AAC_HE_L2 },

  { &avc_mp4_bl_l3_sd_aac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_L3_SD, AUDIO_PROFILE_AAC },

  { &avc_mp4_bl_cif30_aac_mult5, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_mp4_bl_cif30_heaac_l2, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_mp4_bl_cif30_mpeg1_l3, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_MP3 },
  { &avc_mp4_bl_cif30_ac3, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AC3 },
  { &avc_mp4_bl_cif30_aac_ltp, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP },
  { &avc_mp4_bl_cif30_aac_ltp_mult5, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_mp4_bl_cif30_bsac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_BSAC },
  { &avc_mp4_bl_cif30_bsac_mult5, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_BSAC_MULT5 },
  
  { &avc_mp4_bl_l2_cif30_aac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_L2_CIF30, AUDIO_PROFILE_AAC },
  
  { &avc_mp4_bl_cif15_heaac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_mp4_bl_cif15_amr, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AMR },
  { &avc_mp4_bl_cif15_aac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC },
  { &avc_mp4_bl_cif15_aac_520, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15_520, AUDIO_PROFILE_AAC },
  { &avc_mp4_bl_cif15_aac_ltp, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_LTP },
  { &avc_mp4_bl_cif15_aac_ltp_520, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15_520, AUDIO_PROFILE_AAC_LTP },
  { &avc_mp4_bl_cif15_bsac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_BSAC },

  { &avc_mp4_bl_l12_cif15_heaac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_L12_CIF15, AUDIO_PROFILE_AAC_HE_L2 },

  { &avc_mp4_bl_l1b_qcif15_heaac, CT_MP4,
    AVC_VIDEO_PROFILE_BL_L1B_QCIF, AUDIO_PROFILE_AAC_HE_L2 },

  /* MPEG-TS Container */
  { &avc_ts_mp_sd_aac_mult5, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_ts_mp_sd_aac_mult5_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_ts_mp_sd_aac_mult5_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_MULT5 },

  { &avc_ts_mp_sd_heaac_l2, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_ts_mp_sd_heaac_l2_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_ts_mp_sd_heaac_l2_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_HE_L2 },

  { &avc_ts_mp_sd_mpeg1_l3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_MP3 },
  { &avc_ts_mp_sd_mpeg1_l3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_MP3 },
  { &avc_ts_mp_sd_mpeg1_l3_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_MP3 },

  { &avc_ts_mp_sd_ac3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AC3 },
  { &avc_ts_mp_sd_ac3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AC3 },
  { &avc_ts_mp_sd_ac3_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AC3 },

  { &avc_ts_mp_sd_aac_ltp, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_mp_sd_aac_ltp_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_mp_sd_aac_ltp_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP },

  { &avc_ts_mp_sd_aac_ltp_mult5, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_ts_mp_sd_aac_ltp_mult5_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_ts_mp_sd_aac_ltp_mult5_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT5 },

  { &avc_ts_mp_sd_aac_ltp_mult7, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT7 },
  { &avc_ts_mp_sd_aac_ltp_mult7_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT7 },
  { &avc_ts_mp_sd_aac_ltp_mult7_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_LTP_MULT7 },

  { &avc_ts_mp_sd_bsac, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_BSAC },
  { &avc_ts_mp_sd_bsac_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_BSAC },
  { &avc_ts_mp_sd_bsac_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_SD, AUDIO_PROFILE_AAC_BSAC },

  { &avc_ts_bl_cif30_aac_mult5, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_ts_bl_cif30_aac_mult5_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_ts_bl_cif30_aac_mult5_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_MULT5 },

  { &avc_ts_bl_cif30_heaac_l2, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_ts_bl_cif30_heaac_l2_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_ts_bl_cif30_heaac_l2_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_HE_L2 },

  { &avc_ts_bl_cif30_mpeg1_l3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_MP3 },
  { &avc_ts_bl_cif30_mpeg1_l3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_MP3 },
  { &avc_ts_bl_cif30_mpeg1_l3_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_MP3 },

  { &avc_ts_bl_cif30_ac3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AC3 },
  { &avc_ts_bl_cif30_ac3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AC3 },
  { &avc_ts_bl_cif30_ac3_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AC3 },

  { &avc_ts_bl_cif30_aac_ltp, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_bl_cif30_aac_ltp_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_bl_cif30_aac_ltp_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP },

  { &avc_ts_bl_cif30_aac_ltp_mult5, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_ts_bl_cif30_aac_ltp_mult5_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_ts_bl_cif30_aac_ltp_mult5_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AAC_LTP_MULT5 },

  { &avc_ts_bl_cif30_aac_940, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF30_940, AUDIO_PROFILE_AAC },
  { &avc_ts_bl_cif30_aac_940_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF30_940, AUDIO_PROFILE_AAC },
  { &avc_ts_bl_cif30_aac_940_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF30_940, AUDIO_PROFILE_AAC },

  { &avc_ts_mp_hd_aac_mult5, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_ts_mp_hd_aac_mult5_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_MULT5 },
  { &avc_ts_mp_hd_aac_mult5_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_MULT5 },

  { &avc_ts_mp_hd_heaac_l2, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_ts_mp_hd_heaac_l2_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_ts_mp_hd_heaac_l2_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_HE_L2 },

  { &avc_ts_mp_hd_mpeg1_l3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_MP3 },
  { &avc_ts_mp_hd_mpeg1_l3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_MP3 },
  { &avc_ts_mp_hd_mpeg1_l3_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_MP3 },

  { &avc_ts_mp_hd_ac3, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AC3 },
  { &avc_ts_mp_hd_ac3_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AC3 },
  { &avc_ts_mp_hd_ac3_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AC3 },

  { &avc_ts_mp_hd_aac, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC },
  { &avc_ts_mp_hd_aac_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC },
  { &avc_ts_mp_hd_aac_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC },

  { &avc_ts_mp_hd_aac_ltp, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_mp_hd_aac_ltp_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_mp_hd_aac_ltp_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP },

  { &avc_ts_mp_hd_aac_ltp_mult5, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_ts_mp_hd_aac_ltp_mult5_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP_MULT5 },
  { &avc_ts_mp_hd_aac_ltp_mult5_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP_MULT5 },

  { &avc_ts_mp_hd_aac_ltp_mult7, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP_MULT7 },
  { &avc_ts_mp_hd_aac_ltp_mult7_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP_MULT7 },
  { &avc_ts_mp_hd_aac_ltp_mult7_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_MP_HD, AUDIO_PROFILE_AAC_LTP_MULT7 },

  { &avc_ts_bl_cif15_aac, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC },
  { &avc_ts_bl_cif15_aac_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC },
  { &avc_ts_bl_cif15_aac_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC },

  { &avc_ts_bl_cif15_aac_540, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF15_540, AUDIO_PROFILE_AAC },
  { &avc_ts_bl_cif15_aac_540_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF15_540, AUDIO_PROFILE_AAC },
  { &avc_ts_bl_cif15_aac_540_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF15_540, AUDIO_PROFILE_AAC },

  { &avc_ts_bl_cif15_aac_ltp, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_bl_cif15_aac_ltp_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_LTP },
  { &avc_ts_bl_cif15_aac_ltp_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_LTP },

  { &avc_ts_bl_cif15_bsac, CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_BSAC },
  { &avc_ts_bl_cif15_bsac_t, CT_MPEG_TRANSPORT_STREAM_DLNA,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_BSAC },
  { &avc_ts_bl_cif15_bsac_iso, CT_MPEG_TRANSPORT_STREAM,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AAC_BSAC },
  
  /* 3GPP Container */
  { &avc_3gpp_bl_cif30_amr_wbplus, CT_3GP,
    AVC_VIDEO_PROFILE_BL_CIF30, AUDIO_PROFILE_AMR_WB },

  { &avc_3gpp_bl_cif15_amr_wbplus, CT_3GP,
    AVC_VIDEO_PROFILE_BL_CIF15, AUDIO_PROFILE_AMR_WB },

  { &avc_3gpp_bl_qcif15_aac, CT_3GP,
    AVC_VIDEO_PROFILE_BL_QCIF15, AUDIO_PROFILE_AAC },
  { &avc_3gpp_bl_qcif15_aac_ltp, CT_3GP,
    AVC_VIDEO_PROFILE_BL_QCIF15, AUDIO_PROFILE_AAC_LTP },
  { &avc_3gpp_bl_qcif15_heaac, CT_3GP,
    AVC_VIDEO_PROFILE_BL_QCIF15, AUDIO_PROFILE_AAC_HE_L2 },
  { &avc_3gpp_bl_qcif15_amr_wbplus, CT_3GP,
    AVC_VIDEO_PROFILE_BL_QCIF15, AUDIO_PROFILE_AMR_WB },
  { &avc_3gpp_bl_qcif15_amr, CT_3GP,
    AVC_VIDEO_PROFILE_BL_QCIF15, AUDIO_PROFILE_AMR },
  
  { NULL, 0, 0, 0 }
};

static inline int
is_valid_video_profile (video_properties_t res[],
                        int size, AVCodecContext *vc)
{
  int i;

  for (i = 0; i < size / (int) sizeof (video_properties_t); i++)
    if (res[i].width == vc->width &&
        res[i].height == vc->height)
      return 1;

  return 0;
}

static avc_video_profile_t
avc_video_get_profile (AVFormatContext *ctx, AVStream *vs, AVCodecContext *vc)
{
  if (!vs || !vc)
    return AVC_VIDEO_PROFILE_INVALID;

  /* stupid exception to CIF15 */
  if (vc->bit_rate <= 384000 && ctx->bit_rate <= 600000 &&
      vc->width == 320 && vc->height == 240)
    return AVC_VIDEO_PROFILE_BL_L12_CIF15;
  
  /* CIF */
  if (is_valid_video_profile (profile_cif_res,
                              sizeof (profile_cif_res), vc))
  {
    /* QCIF */
    if (vc->bit_rate <= 128000 && ctx->bit_rate <= 256000)
    {
      if (vs->r_frame_rate.num == 15 && vs->r_frame_rate.num == 1)
        return AVC_VIDEO_PROFILE_BL_QCIF15;
      else
        return AVC_VIDEO_PROFILE_BL_L1B_QCIF;
    }
    
    /* CIF15 */
    if (ctx->bit_rate <= 520000) /* 520 kbps max system bitrate */
      return AVC_VIDEO_PROFILE_BL_CIF15_520;
    if (ctx->bit_rate <= 540000) /* 540 kbps max system bitrate */
      return AVC_VIDEO_PROFILE_BL_CIF15_540;
    
    /* 384 kbps max video bitrate */
    if (vc->bit_rate <= 384000 && ctx->bit_rate <= 600000)
      return AVC_VIDEO_PROFILE_BL_CIF15;
    
    /* CIF30 */
    if (ctx->bit_rate <= 940000) /* 940 kbps max system bitrate */
      return AVC_VIDEO_PROFILE_BL_CIF30_940;
    if (ctx->bit_rate <= 1300000) /* 1.3 Mbps kbps max system bitrate */
      return AVC_VIDEO_PROFILE_BL_L2_CIF30;
    
    /* 2 Mbps max video bitrate */
    if (vc->bit_rate <= 2000000 && ctx->bit_rate <= 3000000) 
      return AVC_VIDEO_PROFILE_BL_CIF30;
  }
  
  /* SD */
  if (vc->bit_rate <= 4000000 /* 4 Mbps max */
      && is_valid_video_profile (profile_mp_l3_sd_res,
                                 sizeof (profile_mp_l3_sd_res), vc))
    return AVC_VIDEO_PROFILE_BL_L3_SD;
  /* what is BL_L3L ?? */
  
  if (vc->bit_rate <= 10000000 /* 10 Mbps max */
      && is_valid_video_profile (profile_mp_sd_res,
                                 sizeof (profile_mp_sd_res), vc))
    return AVC_VIDEO_PROFILE_MP_SD;

  /* HD */
  if (vc->bit_rate <= 20000000) /* 20 Mbps max */
  {
    if (is_valid_video_profile (profile_mp_hd_res,
                                sizeof (profile_mp_hd_res), vc))
      return AVC_VIDEO_PROFILE_MP_HD;

    /* dirty hack to support some excentric 480/720/1080(i,p) files
       where only one of the size is correct */
    if (vc->width == 1920 || vc->width == 1280 || vc->width == 720)
      return AVC_VIDEO_PROFILE_MP_HD;
    if (vc->height == 1080 || vc->height == 720 || vc->height == 480)
      return AVC_VIDEO_PROFILE_MP_HD;
  }
  
  return AVC_VIDEO_PROFILE_INVALID;
}

static dlna_profile_t *
probe_avc (AVFormatContext *ctx,
           dlna_container_type_t st,
           av_codecs_t *codecs)
{
  avc_video_profile_t vp;
  audio_profile_t ap;
  int i;
  
  if (!stream_ctx_is_av (codecs))
    return NULL;

  /* check for H.264/AVC codec */
  if (codecs->vc->codec_id != CODEC_ID_H264)
    return NULL;

  /* check for a supported container */
  if (st != CT_3GP &&
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
  vp = avc_video_get_profile (ctx, codecs->vs, codecs->vc);
  if (vp == AVC_VIDEO_PROFILE_INVALID)
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
  for (i = 0; avc_profiles_mapping[i].profile; i++)
    if (avc_profiles_mapping[i].st == st &&
        avc_profiles_mapping[i].vp == vp &&
        avc_profiles_mapping[i].ap == ap)
      return avc_profiles_mapping[i].profile;
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_av_mpeg4_part10 = {
    DLNA_PROFILE_AV_MPEG4_PART10,
    DLNA_CLASS_AV,
    "mov,hdmov,mp4,3gp,3gpp,mpg,mpeg,mpe,mp2t,ts",
    probe_avc,
    NULL
};
