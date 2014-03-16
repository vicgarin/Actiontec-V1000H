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

#include "dlna_internals.h"
#include "profiles.h"
#include "containers.h"

typedef struct mpeg_ps_es_stream_s {
  int width;
  int height;
} mpeg_ps_es_stream_t;

typedef struct mpeg_ts_stream_s {
  int width;
  int height;
  int fps_num;
  int fps_den;
} mpeg_ts_stream_t;

static mpeg_ps_es_stream_t mpeg_ps_es_valid_streams_ntsc[] = {
  { 720, 480 },
  { 704, 480 },
  { 544, 480 },
  { 480, 480 },
  { 352, 480 },
  { 352, 288 },
  { 352, 240 }
};

static mpeg_ps_es_stream_t mpeg_ps_es_valid_streams_pal[] = {
  { 720, 576 },
  { 704, 576 },
  { 544, 576 },
  { 480, 576 },
  { 352, 576 },
  { 352, 288 }
};

static mpeg_ts_stream_t mpeg_ts_valid_streams_eu_sd[] = {
  { 720, 576, 25, 1},
  { 544, 576, 25, 1},
  { 480, 576, 25, 1},
  { 352, 576, 25, 1},
  { 352, 288, 25, 1},
  { 252, 288, 25, 1}
};

static mpeg_ts_stream_t mpeg_ts_valid_streams_na_sd[] = {
  { 720, 480, 30, 1001},
  { 720, 480, 30, 1},
  { 720, 480, 30000, 1001},
  { 704, 480, 30, 1001},
  { 704, 480, 30000, 1001},
  { 704, 480, 30, 1},
  { 704, 480, 60, 1},
  { 704, 480, 60000, 1001},
  { 704, 480, 24, 1001},
  { 704, 480, 24000, 1001},
  { 704, 480, 24, 1},
  { 640, 480, 60000, 1001}, /*Support for 59.94*/
  { 640, 480, 60, 1},
  { 640, 480, 30, 1001},
  { 640, 480, 30, 1},
  { 640, 480, 24, 1001},
  { 640, 480, 24, 1},
  { 544, 480, 30, 1001},
  { 544, 480, 30000, 1001},
  { 544, 480, 30, 1},
  { 480, 480, 30, 1001},
  { 480, 480, 30000, 1001}, /*Support for 29.97*/
  { 480, 480, 30, 1},
  { 352, 480, 30000, 1001},
  { 352, 480, 30, 1001},
  { 352, 480, 30, 1}
};

static mpeg_ts_stream_t mpeg_ts_valid_streams_na_hd[] = {
  { 1920, 1080, 30, 1001},
  { 1920, 1080, 30000, 1001},
  { 1920, 1080, 30, 1},
  { 1920, 1080, 24, 1001},
  { 1920, 1080, 24000, 1001},
  { 1920, 1080, 24, 1},
  { 1280, 720, 30, 1001},
  { 1280, 720, 30000, 1001},
  { 1280, 720, 30, 1},
  { 1280, 720, 60, 1001},
  { 1280, 720, 60000, 1001},
  { 1280, 720, 60, 1},
  { 1280, 720, 24000, 1001},
  { 1280, 720, 24, 1001},
  { 1280, 720, 24, 1},
  { 1440, 1080, 30000, 1001},
  { 1440, 1080, 30, 1001},
  { 1440, 1080, 30, 1},
  { 1440, 1080, 24000, 1001},
  { 1440, 1080, 24, 1001},
  { 1440, 1080, 24, 1},
  { 1280, 1080, 30000, 1001},
  { 1280, 1080, 30, 1001},
  { 1280, 1080, 30, 1},
  { 1280, 1080, 24000, 1001},
  { 1280, 1080, 24, 1001},
  { 1280, 1080, 24, 1}
};

/* Profile for NTSC-formatted AV class media */
static dlna_profile_t mpeg_ps_ntsc = {
    "MPEG_PS_NTSC",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* Profile for NTSC-formatted AV class media */
static dlna_profile_t mpeg_ps_ntsc_xac3 = {
    "MPEG_PS_NTSC_XAC3",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* Profile for PAL-formatted AV class media */
static dlna_profile_t mpeg_ps_pal = {
    "MPEG_PS_PAL",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* Profile for PAL-formatted AV class media */
static dlna_profile_t mpeg_ps_pal_xac3 = {
    "MPEG_PS_PAL_XAC3",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* MPEG-2 Main Profile at Low Level AAC LC audio encapsulated in
   MPEG-2 transport stream with zero value timestamp */
static dlna_profile_t mpeg_ts_mp_ll_aac = {
    "MPEG_TS_MP_LL_AAC",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF30
};

/* MPEG-2 Main Profile at Low Level AAC LC audio encapsulated in
   MPEG-2 transport stream with valid value timestamp */
static dlna_profile_t mpeg_ts_mp_ll_aac_t = {
    "MPEG_TS_MP_LL_AAC_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_CIF30
};

/* MPEG-2 Main Profile at Low Level AAC LC audio encapsulated in
   MPEG-2 transport stream without a Timestamp field */
static dlna_profile_t mpeg_ts_mp_ll_aac_iso = {
    "MPEG_TS_MP_LL_AAC_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_CIF30
};

/* European region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_sd_eu = {
    "MPEG_TS_SD_EU",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

/* European region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_sd_eu_t = {
    "MPEG_TS_SD_EU_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

/* European region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_sd_eu_iso = {
    "MPEG_TS_SD_EU_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na = {
    "MPEG_TS_SD_NA",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na_t = {
    "MPEG_TS_SD_NA_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_sd_na_iso = {
    "MPEG_TS_SD_NA_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na_xac3 = {
    "MPEG_TS_SD_NA_XAC3",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na_xac3_t = {
    "MPEG_TS_SD_NA_XAC3_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_sd_na_xac3_iso = {
    "MPEG_TS_SD_NA_XAC3_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* North America region profile for High Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na = {
    "MPEG_TS_HD_NA",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_HD
};

/* North America region profile for High Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na_t = {
    "MPEG_TS_HD_NA_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_HD
};

/* North America region profile for High Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_hd_na_iso = {
    "MPEG_TS_HD_NA_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_HD
};

/* North America region profile for transcoded High Definition AV class
   media with a zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na_xac3 = {
    "MPEG_TS_HD_NA_XAC3",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_HD
};

/* North America region profile for transcoded High Definition AV class
   media with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na_xac3_t = {
    "MPEG_TS_HD_NA_XAC3_T",
    MIME_VIDEO_MPEG_TS,
    LABEL_VIDEO_HD
};

/* North America region profile for transcoded High Definition AV class
   media without a Timestamp field */
static dlna_profile_t mpeg_ts_hd_na_xac3_iso = {
    "MPEG_TS_HD_NA_XAC3_ISO",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_HD
};

/* Profile defining ES encapsulation for transport of MPEG_PS_PAL over RTP */
static dlna_profile_t mpeg_es_pal = {
    "MPEG_ES_PAL",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* Profile defining ES encapsulation for transport of MPEG_PS_NTSC over RTP */
static dlna_profile_t mpeg_es_ntsc = {
    "MPEG_ES_NTSC",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* Profile defining ES encapsulation for transport of
   MPEG_PS_PAL_XAC3 over RTP */
static dlna_profile_t mpeg_es_pal_xac3 = {
    "MPEG_ES_PAL_XAC3",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

/* Profile defining ES encapsulation for transport of
   MPEG_PS_NTSC_XAC3 over RTP */
static dlna_profile_t mpeg_es_ntsc_xac3 = {
    "MPEG_ES_NTSC_XAC3",
    MIME_VIDEO_MPEG,
    LABEL_VIDEO_SD
};

static int
is_mpeg_ps_es_audio_stream_lpcm (AVFormatContext *ctx dlna_unused,
                                 av_codecs_t *codecs)
{
  if (audio_profile_guess_lpcm (codecs->ac) != AUDIO_PROFILE_LPCM)
    return 0;
  
  /* audio bit rate: 1.536 Mbps for stereo, 768 Kbps for mono */
  if (codecs->ac->channels == 2 && codecs->ac->bit_rate > 1536000)
    return 0;
  if (codecs->ac->channels == 1 && codecs->ac->bit_rate > 768000)
    return 0;
  
  return 1;
}

static int
is_mpeg_ps_es_audio_stream_extended_ac3 (AVFormatContext *ctx dlna_unused,
                                      av_codecs_t *codecs)
{
  return (audio_profile_guess_ac3 (codecs->ac) == AUDIO_PROFILE_AC3_EXTENDED)
    ? 1 : 0;
}

static int
is_mpeg_ps_es_audio_stream_ac3 (AVFormatContext *ctx dlna_unused,
                                av_codecs_t *codecs)
{
  return (audio_profile_guess_ac3 (codecs->ac) == AUDIO_PROFILE_AC3) ? 1 : 0;
}

static int
is_mpeg_ps_es_audio_stream_mp2 (AVFormatContext *ctx dlna_unused,
                                av_codecs_t *codecs)
{
  if (audio_profile_guess_mp2 (codecs->ac) != AUDIO_PROFILE_MP2)
    return 0;

  /* supported channels: 1/0, 1/0 + 1/0, 2/0 */
  if (codecs->ac->channels > 2)
    return 0;

  /* audio bit rate: 64-192 Kbps for mono, 64-384 Kbps for stereo */
  if (codecs->ac->channels == 1 &&
      (codecs->ac->bit_rate < 64000 || codecs->ac->bit_rate > 192000))
    return 0;
  if (codecs->ac->channels == 2 &&
      (codecs->ac->bit_rate < 64000 || codecs->ac->bit_rate > 384000))
    return 0;
  
  return 1;
}

static int
is_mpeg_ts_audio_stream_mp2 (AVFormatContext *ctx dlna_unused,
                             av_codecs_t *codecs)
{
  return (audio_profile_guess_mp2 (codecs->ac) == AUDIO_PROFILE_MP2) ? 1 : 0;
}

static int
is_mpeg_ts_audio_stream_ac3 (AVFormatContext *ctx dlna_unused,
                             av_codecs_t *codecs)
{
  return (audio_profile_guess_ac3 (codecs->ac) == AUDIO_PROFILE_AC3) ? 1 : 0;
}

static dlna_profile_t *
probe_mpeg_ps_es (AVFormatContext *ctx, av_codecs_t *codecs,
                  dlna_profile_t *pal, dlna_profile_t *pal_xac3,
                  dlna_profile_t *ntsc, dlna_profile_t *ntsc_xac3)
{
  int i;

  /* determine region through frame rate */
  if ((codecs->vs->r_frame_rate.num == 30000 &&
       codecs->vs->r_frame_rate.den == 1001)) /* NTSC */
  {
    for (i = 0; i < (int) (sizeof (mpeg_ps_es_valid_streams_ntsc)
                           / sizeof (mpeg_ps_es_stream_t)); i++)
    {
      if (mpeg_ps_es_valid_streams_ntsc[i].width == codecs->vc->width &&
          mpeg_ps_es_valid_streams_ntsc[i].height == codecs->vc->height)
      {
        if (is_mpeg_ps_es_audio_stream_extended_ac3 (ctx, codecs))
          return ntsc_xac3;
        else if (is_mpeg_ps_es_audio_stream_lpcm (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_ac3 (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_mp2 (ctx, codecs))
          return ntsc;

        return NULL;
      }
    }

    /* invalid resolution */
    return NULL;
  }
  else if (codecs->vs->r_frame_rate.num == 25 &&
           codecs->vs->r_frame_rate.den == 1) /* PAL */
  {
    for (i = 0; i < (int) (sizeof (mpeg_ps_es_valid_streams_pal)
                           / sizeof (mpeg_ps_es_stream_t)); i++)
    {
      if (mpeg_ps_es_valid_streams_pal[i].width == codecs->vc->width &&
          mpeg_ps_es_valid_streams_pal[i].height == codecs->vc->height)
      {
        if (is_mpeg_ps_es_audio_stream_extended_ac3 (ctx, codecs))
          return pal_xac3;
        else if (is_mpeg_ps_es_audio_stream_lpcm (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_ac3 (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_mp2 (ctx, codecs))
          return pal;

        return NULL;
      }
    }

    /* invalid resolution */
    return NULL;
  }

  return NULL;
}

static dlna_profile_t *
probe_mpeg_ps (AVFormatContext *ctx, av_codecs_t *codecs)
{
  return probe_mpeg_ps_es (ctx, codecs,
                           &mpeg_ps_pal, &mpeg_ps_pal_xac3,
                           &mpeg_ps_ntsc, &mpeg_ps_ntsc_xac3);
}

static dlna_profile_t *
probe_mpeg_es (AVFormatContext *ctx, av_codecs_t *codecs)
{
  return probe_mpeg_ps_es (ctx, codecs,
                           &mpeg_es_pal, &mpeg_es_pal_xac3,
                           &mpeg_es_ntsc, &mpeg_es_ntsc_xac3);
}

static dlna_profile_t *
probe_mpeg_ts (AVFormatContext *ctx,
               av_codecs_t *codecs, dlna_container_type_t st)
{
  int xac3 = 0; /* extended AC3 audio */
  int i;
  
  /* check for MPEG-2 MP@LL profile */
  if (codecs->ac->codec_id == CODEC_ID_AAC)
  {
    /* 352x288 only */
    if (codecs->vc->width != 352 && codecs->vc->height != 288)
      return NULL;

    /* 30 fps */
    if (codecs->vs->r_frame_rate.num != 30 &&
        codecs->vs->r_frame_rate.den != 1)
      return NULL;

    /* video bitrate is less or equal to 4 Mbps */
    if (codecs->vc->bit_rate > 4000000)
      return NULL;

    /* audio bitrate is less or equal to 256 Kbps */
    if (codecs->ac->bit_rate > 256000)
      return NULL;

    switch (st)
    {
    case CT_MPEG_TRANSPORT_STREAM:
      return &mpeg_ts_mp_ll_aac_iso;
    case CT_MPEG_TRANSPORT_STREAM_DLNA:
      return &mpeg_ts_mp_ll_aac_t;
    case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
      return &mpeg_ts_mp_ll_aac;
    default:
      return NULL;
    }
  }

  /* check for Region: only Europe supports 25 fps (50i) */
  if (codecs->vs->r_frame_rate.num == 25 &&
      codecs->vs->r_frame_rate.den == 1)
  {
    for (i = 0; i < (int) (sizeof (mpeg_ts_valid_streams_eu_sd)
                           / sizeof (mpeg_ts_stream_t)); i++)
    {
      if (mpeg_ts_valid_streams_eu_sd[i].width == codecs->vc->width &&
          mpeg_ts_valid_streams_eu_sd[i].height == codecs->vc->height &&
          mpeg_ts_valid_streams_eu_sd[i].fps_num == 25 &&
          mpeg_ts_valid_streams_eu_sd[i].fps_den == 1)
      {
// Why do we Need to Check for Audio?? From FFMPEG libraries we are 
//  are getting MP3 as the audio type of some of the MPEG AV stream 
// though the AUDIO actually inside is a AC3. We do not need to check 
// for audio data type at all since all other information properly 
// categories the stream.
//          if (is_mpeg_ts_audio_stream_ac3 (ctx, codecs) ||
//            is_mpeg_ts_audio_stream_mp2 (ctx, codecs))
//
        {
          switch (st)
          {
          case CT_MPEG_TRANSPORT_STREAM:
            return &mpeg_ts_sd_eu_iso;
          case CT_MPEG_TRANSPORT_STREAM_DLNA:
            return &mpeg_ts_sd_eu_t;
          case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
            return &mpeg_ts_sd_eu;
          default:
            return NULL;
          }
        }

        /* invalid audio stream */
        return NULL;
      }
    }

    /* invalid resolution */
    return NULL;
  }

  /* now comes the stupid part: there's no way to differentiate region
     codes between North America (NA) and Korea (KO) as both have exactly
     the same requirements !! NA however supports additional stream formats
     so all streams will be declared as NA ones (which shouldn't bother
     the real KO ones). */

  /* NA and KO streams can be either SD (Standard Definition)
     or HD (High-Definition) and only support AC3 as audio stream codec */

  /* maximum system bit rate is 19.3927 Mb/s */
#if ORIGINAL_CODE
  /* The FFMPEG codec library is giving us a bit rate of 19.9 Mbps
   * So we are removing the bitrate check.
   */
      if (ctx->bit_rate > 19392700)
        return NULL;
#endif

  if (codecs->ac->codec_id != CODEC_ID_AC3)
    return NULL;

  /* 48 KHz only */
  if (codecs->ac->sample_rate != 48000)
    return NULL;

  /* up to 5 audio channels */
  if (codecs->ac->channels > 5)
    return NULL;

  /* audio bitrate up to 448 Kbps (or 640 for extended AC3) */
  if (codecs->ac->bit_rate > 448000)
    xac3 = 1;
  if (codecs->ac->bit_rate > 640000)
    return NULL;


  /* look for compatible SD video stream */
  for (i = 0; i < (int) (sizeof (mpeg_ts_valid_streams_na_sd)
                         / sizeof (mpeg_ts_stream_t)); i++)
  {
#if ORIGINAL_CODE
    if (mpeg_ts_valid_streams_na_sd[i].width == codecs->vc->width &&
        mpeg_ts_valid_streams_na_sd[i].height == codecs->vc->height &&
        mpeg_ts_valid_streams_na_sd[i].fps_num
        == codecs->vs->r_frame_rate.num &&
        mpeg_ts_valid_streams_na_sd[i].fps_den
        == codecs->vs->r_frame_rate.num)
#endif

    if ((mpeg_ts_valid_streams_na_sd[i].width == codecs->vc->width) &&                // Check for Width
        (mpeg_ts_valid_streams_na_sd[i].height == codecs->vc->height) &&            // Check For Height    
        (mpeg_ts_valid_streams_na_sd[i].fps_num == codecs->vs->r_frame_rate.num) && //Check For Frame Rate
        (mpeg_ts_valid_streams_na_sd[i].fps_den == codecs->vs->r_frame_rate.den))
    {
      switch (st)
      {
      case CT_MPEG_TRANSPORT_STREAM:
        return xac3 ? &mpeg_ts_sd_na_xac3_iso : &mpeg_ts_sd_na_iso;
      case CT_MPEG_TRANSPORT_STREAM_DLNA:
        return xac3 ? &mpeg_ts_sd_na_xac3_t : &mpeg_ts_sd_na_t;
      case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
        return xac3 ? &mpeg_ts_sd_na_xac3 : &mpeg_ts_sd_na;
      default:
        return NULL;
      }
    }
  }

  /* look for compatible HD video stream */
  for (i = 0; i < (int) (sizeof (mpeg_ts_valid_streams_na_hd)
                         / sizeof (mpeg_ts_stream_t)); i++)
  {
#if ORIGINAL_CODE
    if (mpeg_ts_valid_streams_na_hd[i].width == codecs->vc->width &&
        mpeg_ts_valid_streams_na_hd[i].height == codecs->vc->height &&
        mpeg_ts_valid_streams_na_hd[i].fps_num
        == codecs->vs->r_frame_rate.num &&
        mpeg_ts_valid_streams_na_hd[i].fps_den
        == codecs->vs->r_frame_rate.num)
#endif
    if ( (mpeg_ts_valid_streams_na_hd[i].width == codecs->vc->width) &&                    // Check for Width
         (mpeg_ts_valid_streams_na_hd[i].height == codecs->vc->height) &&                // Check For Height    
        (mpeg_ts_valid_streams_na_hd[i].fps_num == codecs->vs->r_frame_rate.num) &&        //Check For Frame Rate
        (mpeg_ts_valid_streams_na_hd[i].fps_den == codecs->vs->r_frame_rate.den))

    {
      switch (st)
      {
      case CT_MPEG_TRANSPORT_STREAM:
        return xac3 ? &mpeg_ts_hd_na_xac3_iso : &mpeg_ts_hd_na_iso;
      case CT_MPEG_TRANSPORT_STREAM_DLNA:
        return xac3 ? &mpeg_ts_hd_na_xac3_t : &mpeg_ts_hd_na_t;
      case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
        return xac3 ? &mpeg_ts_hd_na_xac3 : &mpeg_ts_hd_na;
      default:
        return NULL;
      }
    }
  }

  /* no compliant resolution found */
  return NULL;
}

static dlna_profile_t *
probe_mpeg2 (AVFormatContext *ctx,
             dlna_container_type_t st,
             av_codecs_t *codecs)
{
  if (!stream_ctx_is_av (codecs))
    return NULL;
  
  /* check for MPEG-2 video codec */
  if (codecs->vc->codec_id != CODEC_ID_MPEG2VIDEO)
    return NULL;

  switch (st)
  {
  case CT_MPEG_ELEMENTARY_STREAM:
    return probe_mpeg_es (ctx, codecs);
  case CT_MPEG_PROGRAM_STREAM:
    return probe_mpeg_ps (ctx, codecs);
  case CT_MPEG_TRANSPORT_STREAM:
  case CT_MPEG_TRANSPORT_STREAM_DLNA:
  case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
    return probe_mpeg_ts (ctx, codecs, st);
  default:
    break;
  }
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_av_mpeg2 = {
    DLNA_PROFILE_AV_MPEG2,
    DLNA_CLASS_AV,
    "mpg,mpeg,mpe,m2v,mp2p,mp2t,ts,ps,pes",
    probe_mpeg2,
    NULL
};
