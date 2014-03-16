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

/* Profile for audio media class content */
static dlna_profile_t aac_adts = {
  "AAC_ADTS",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t aac_adts_320 = {
  "AAC_ADTS_320",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t aac_iso = {
  "AAC_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t aac_iso_320 = {
  "AAC_ISO_320",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content. In the case of AAC LTP profiles,
   both the ISO file formats and the ADTS format are supported by
   the same profile. */
static dlna_profile_t aac_ltp_iso = {
  "AAC_LTP_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content with up to 5.1 channels */
static dlna_profile_t aac_ltp_mult5_iso = {
  "AAC_LTP_MULT5_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_MULTI
};

/* Profile for audio media class content with up to 7.1 channels */
static dlna_profile_t aac_ltp_mult7_iso = {
  "AAC_LTP_MULT7_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_MULTI
};

/* Profile for audio media class content with up to 5.1 channels */
static dlna_profile_t aac_mult5_adts = {
  "AAC_MULT5_ADTS",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_MULTI
};

/* Profile for audio media class content with up to 5.1 channels */
static dlna_profile_t aac_mult5_iso = {
  "AAC_MULT5_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_MULTI
};

/* Profile for audio media class content */
static dlna_profile_t heaac_l2_adts = {
  "HEAAC_L2_ADTS",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t heaac_l2_iso = {
  "HEAAC_L2_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t heaac_l3_adts = {
  "HEAAC_L3_ADTS",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t heaac_l3_iso = {
  "HEAAC_L3_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content with up to 5.1 channels */
static dlna_profile_t heaac_mult5_adts = {
  "HEAAC_MULT5_ADTS",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_MULTI
};

/* Profile for audio media class content with up to 5.1 channels */
static dlna_profile_t heaac_mult5_iso = {
  "HEAAC_MULT5_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_MULTI
};

/* Profile for audio media class content */
static dlna_profile_t heaac_l2_adts_320 = {
  "HEAAC_L2_ADTS_320",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t heaac_l2_iso_320 = {
  "HEAAC_L2_ISO_320",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

/* Profile for audio media class content */
static dlna_profile_t bsac_iso = {
  "BSAC_ISO",
  MIME_AUDIO_MPEG_4,
   LABEL_AUDIO_2CH
};
/* Profile for audio media class content with up to 5.1 channels */
static dlna_profile_t bsac_mult5_iso = {
  "BSAC_MULT5_ISO",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_MULTI
};

static dlna_profile_t heaac_v2_l2 = {
  "HEAACv2_L2",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_l2_adts = {
  "HEAACv2_L2",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_l2_320 = {
  "HEAACv2_L2_320",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_l2_320_adts = {
  "HEAACv2_L2_320",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_l3 = {
  "HEAACv2_L3",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_l3_adts = {
  "HEAACv2_L3",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_mult5 = {
  "HEAACv2_MULT5",
  MIME_AUDIO_MPEG_4,
  LABEL_AUDIO_2CH
};

static dlna_profile_t heaac_v2_mult5_adts = {
  "HEAACv2_MULT5",
  MIME_AUDIO_ADTS,
  LABEL_AUDIO_2CH
};

typedef enum {
  AAC_MUXED,          /* AAC is muxed in a container */
  AAC_RAW             /* AAC is raw (ADTS) */
} aac_container_type_t;

/* HeAACv1 (a.k.a. AACplus v1) is AAC LC + Spectral Band Replication (SBR) */
/* HeAACv2 (a.ka.a AACplus v2) is HeAACv1 + Parametric Stereo (PS) */

typedef enum {
  AAC_INVALID   =  0, 
  AAC_MAIN      =  1, /* AAC Main */
  AAC_LC        =  2, /* AAC Low complexity */
  AAC_SSR       =  3, /* AAC SSR */
  AAC_LTP       =  4, /* AAC Long term prediction */
  AAC_HE        =  5, /* AAC High efficiency (SBR) */
  AAC_SCALE     =  6, /* Scalable */
  AAC_TWINVQ    =  7, /* TwinVQ */
  AAC_CELP      =  8, /* CELP */
  AAC_HVXC      =  9, /* HVXC */
  AAC_TTSI      = 12, /* TTSI */
  AAC_MS        = 13, /* Main synthetic */
  AAC_WAVE      = 14, /* Wavetable synthesis */
  AAC_MIDI      = 15, /* General MIDI */
  AAC_FX        = 16, /* Algorithmic Synthesis and Audio FX */
  AAC_LC_ER     = 17, /* AAC Low complexity with error recovery */
  AAC_LTP_ER    = 19, /* AAC Long term prediction with error recovery */
  AAC_SCALE_ER  = 20, /* AAC scalable with error recovery */
  AAC_TWINVQ_ER = 21, /* TwinVQ with error recovery */
  AAC_BSAC_ER   = 22, /* BSAC with error recovery */
  AAC_LD_ER     = 23, /* AAC LD with error recovery */
  AAC_CELP_ER   = 24, /* CELP with error recovery */
  AAC_HXVC_ER   = 25, /* HXVC with error recovery */
  AAC_HILN_ER   = 26, /* HILN with error recovery */
  AAC_PARAM_ER  = 27, /* Parametric with error recovery */
  AAC_SSC       = 28, /* AAC SSC */
  AAC_HE_L3     = 31, /* Reserved : seems to be HeAAC L3 */
} aac_object_type_t;

static const struct {
  dlna_profile_t *profile;
  aac_container_type_t ct;
  audio_profile_t ap;
} aac_profiles_mapping[] = {
  { &aac_adts,              AAC_RAW,     AUDIO_PROFILE_AAC                 },
  { &aac_adts_320,          AAC_RAW,     AUDIO_PROFILE_AAC_320             },
  { &aac_iso,               AAC_MUXED,   AUDIO_PROFILE_AAC                 },
  { &aac_iso_320,           AAC_MUXED,   AUDIO_PROFILE_AAC_320             },
  { &aac_ltp_iso,           AAC_MUXED,   AUDIO_PROFILE_AAC_LTP             },
  { &aac_ltp_mult5_iso,     AAC_MUXED,   AUDIO_PROFILE_AAC_LTP_MULT5       },
  { &aac_ltp_mult7_iso,     AAC_MUXED,   AUDIO_PROFILE_AAC_LTP_MULT7       },
  { &aac_mult5_adts,        AAC_RAW,     AUDIO_PROFILE_AAC_MULT5           },
  { &aac_mult5_iso,         AAC_MUXED,   AUDIO_PROFILE_AAC_MULT5           },
  { &heaac_l2_adts,         AAC_RAW,     AUDIO_PROFILE_AAC_HE_L2           },
  { &heaac_l2_iso,          AAC_MUXED,   AUDIO_PROFILE_AAC_HE_L2           },
  { &heaac_l3_adts,         AAC_RAW,     AUDIO_PROFILE_AAC_HE_L3           },
  { &heaac_l3_iso,          AAC_MUXED,   AUDIO_PROFILE_AAC_HE_L3           },
  { &heaac_mult5_adts,      AAC_RAW,     AUDIO_PROFILE_AAC_HE_MULT5        },
  { &heaac_mult5_iso,       AAC_MUXED,   AUDIO_PROFILE_AAC_HE_MULT5        },
  { &heaac_l2_adts_320,     AAC_RAW,     AUDIO_PROFILE_AAC_HE_L2_320       },
  { &heaac_l2_iso_320,      AAC_MUXED,   AUDIO_PROFILE_AAC_HE_L2_320       },
  { &heaac_v2_l2,           AAC_MUXED,   AUDIO_PROFILE_AAC_HE_V2_L2        },
  { &heaac_v2_l2_adts,      AAC_RAW,     AUDIO_PROFILE_AAC_HE_V2_L2        },
  { &heaac_v2_l2_320,       AAC_MUXED,   AUDIO_PROFILE_AAC_HE_V2_L2_320    },
  { &heaac_v2_l2_320_adts,  AAC_RAW,     AUDIO_PROFILE_AAC_HE_V2_L2_320    },
  { &heaac_v2_l3,           AAC_MUXED,   AUDIO_PROFILE_AAC_HE_V2_L3        },
  { &heaac_v2_l3_adts,      AAC_RAW,     AUDIO_PROFILE_AAC_HE_V2_L3        },
  { &heaac_v2_mult5,        AAC_MUXED,   AUDIO_PROFILE_AAC_HE_V2_MULT5     },
  { &heaac_v2_mult5_adts,   AAC_RAW,     AUDIO_PROFILE_AAC_HE_V2_MULT5     },
  { &bsac_iso,              AAC_MUXED,   AUDIO_PROFILE_AAC_BSAC            },
  { &bsac_mult5_iso,        AAC_MUXED,   AUDIO_PROFILE_AAC_BSAC_MULT5      },
  { NULL, 0, 0 }
};

static aac_object_type_t
aac_object_type_get (uint8_t *data, int len)
{
  uint8_t t = AAC_INVALID;
  
  if (!data || len < 1)
    goto aac_object_type_error;

  t = data[0] >> 3; /* Get 5 first bits */
  
 aac_object_type_error:
#ifdef HAVE_DEBUG
    fprintf (stderr, "AAC Object Type: %d\n", t);
#endif /* HAVE_DEBUG */
  
  return t;
}

static audio_profile_t
audio_profile_guess_aac_priv (AVCodecContext *ac, aac_object_type_t type)
{
  if (!ac)
    return AUDIO_PROFILE_INVALID;

  /* check for AAC variants codec */
  if (ac->codec_id != CODEC_ID_AAC)
    return AUDIO_PROFILE_INVALID;
  
  switch (type)
  {
  /* AAC Low Complexity variants */
  case AAC_LC:
  case AAC_LC_ER:
  {
    if (ac->sample_rate < 8000 || ac->sample_rate > 48000)
      break;

    if (ac->channels <= 2) /* AAC @ Level 1/2 */
    {
      if (ac->bit_rate <= 320000)
        return AUDIO_PROFILE_AAC_320;

      if (ac->bit_rate <= 576000)
        return AUDIO_PROFILE_AAC;

      break;
    }
    else if (ac->channels <= 6)
    {
      if (ac->bit_rate <= 1440000)
        return AUDIO_PROFILE_AAC_MULT5;

      break;
    }
    break;
  }

  /* AAC Long Term Prediction variants */
  case AAC_LTP:
  case AAC_LTP_ER:
  {
    if (ac->sample_rate < 8000)
      break;

    if (ac->sample_rate <= 48000)
    {
      if (ac->channels <= 2 && ac->bit_rate <= 576000)
        return AUDIO_PROFILE_AAC_LTP;

      break;
    }
    else if (ac->sample_rate <= 96000)
    {
      if (ac->channels <= 6 && ac->bit_rate <= 2880000)
        return AUDIO_PROFILE_AAC_LTP_MULT5;

      if (ac->channels <= 8 && ac->bit_rate <= 4032000)
        return AUDIO_PROFILE_AAC_LTP_MULT7;

      break;
    }

    break;
  }

  /* AAC High efficiency (with SBR) variants */
  case AAC_HE:
  case AAC_HE_L3:
  {
    if (ac->sample_rate < 8000)
      break;

    if (ac->sample_rate <= 24000) /* HE-AAC @ Level 2 */
    {
      if (ac->channels > 2)
        break;

      if (ac->bit_rate <= 320000)
        return AUDIO_PROFILE_AAC_HE_L2_320;

      if (ac->bit_rate <= 576000)
        return AUDIO_PROFILE_AAC_HE_L2;

      break;
    }
    else if (ac->sample_rate <= 48000)
    {
      /* HE-AAC @ Level 3 */
      if (ac->channels <= 2 && ac->bit_rate <= 576000)
        return AUDIO_PROFILE_AAC_HE_L3;

      /* HE-AAC @ Level 4/5 */
      if (ac->channels <= 6 && ac->bit_rate <= 1440000)
        return AUDIO_PROFILE_AAC_HE_MULT5;

      break;
    }
    
    break;
  }

  /* AAC High efficiency v2 (with PS) variants */
  case AAC_PARAM_ER:
  {
    if (ac->sample_rate < 8000)
      break;

    if (ac->sample_rate <= 24000) /* HE-AAC @ Level 2 */
    {
      if (ac->channels > 2)
        break;

      if (ac->bit_rate <= 320000)
        return AUDIO_PROFILE_AAC_HE_V2_L2_320;

      if (ac->bit_rate <= 576000)
        return AUDIO_PROFILE_AAC_HE_V2_L2;

      break;
    }
    else if (ac->sample_rate <= 48000)
    {
      /* HE-AAC @ Level 3 */
      if (ac->channels <= 2 && ac->bit_rate <= 576000)
        return AUDIO_PROFILE_AAC_HE_V2_L3;

      /* HE-AAC @ Level 4/5 */
      if (ac->channels <= 6 && ac->bit_rate <= 2880000)
        return AUDIO_PROFILE_AAC_HE_V2_MULT5;

      break;
    }

    break;
  }
  
  case AAC_BSAC_ER:
  {
    if (ac->sample_rate < 16000 || ac->sample_rate > 48000)
      break;

    if (ac->bit_rate > 128000)
      break;

    if (ac->channels <= 2)
      return AUDIO_PROFILE_AAC_BSAC;
    else if (ac->channels <= 6)
      return AUDIO_PROFILE_AAC_BSAC_MULT5;
 
    break;
  }

  default:
    break;
  }
  
  return AUDIO_PROFILE_INVALID;
}

audio_profile_t
audio_profile_guess_aac (AVCodecContext *ac)
{
  aac_object_type_t type;
  
  if (!ac)
    return AUDIO_PROFILE_INVALID;

  type = aac_object_type_get (ac->extradata, ac->extradata_size);
  return audio_profile_guess_aac_priv (ac, type);
}

static aac_object_type_t
aac_adts_object_type_get (AVFormatContext *ctx)
{
  FILE *fd;
  unsigned char buf[4];
  uint8_t t = AAC_INVALID;

  if (!ctx)
    return t;
  
  fd = fopen (ctx->filename, "rb");
  fread(buf, sizeof (buf) - 1,1,fd);
  t = (buf[2] & 0xC0) >> 6;
  fclose (fd);
  
#ifdef HAVE_DEBUG
    fprintf (stderr, "AAC Object Type: %d\n", t);
#endif /* HAVE_DEBUG */
  
  return t;
}

static aac_container_type_t
aac_get_format (AVFormatContext *ctx)
{
  FILE *fd;
  unsigned char buf[4];
  aac_container_type_t ct = AAC_MUXED;
  
  if (!ctx)
    return ct;

  fd = fopen (ctx->filename, "rb");
  fread (buf, sizeof (buf) - 1,1,fd);
  if ((buf[0] == 0xFF) && ((buf[1] & 0xF6) == 0xF0))
  {
    ct = AAC_RAW;
#ifdef HAVE_DEBUG
    fprintf (stderr, "AAC has an ADTS header\n");
#endif /* HAVE_DEBUG */
  }
  fclose (fd);

  return ct;
}

static dlna_profile_t *
probe_mpeg4 (AVFormatContext *ctx,
             dlna_container_type_t st,
             av_codecs_t *codecs)
{
  audio_profile_t ap;
  aac_container_type_t ct = AAC_MUXED;
  int i;

  if (!stream_ctx_is_audio (codecs))
    return NULL;

  /* check for ADTS */
  if (st == CT_AAC)
  {
    aac_object_type_t ot;
    ct = aac_get_format (ctx);
    ot = aac_adts_object_type_get (ctx);
    ap = audio_profile_guess_aac_priv (codecs->ac, ot);
  }
  else
    ap = audio_profile_guess_aac (codecs->ac);
  
  if (ap == AUDIO_PROFILE_INVALID)
    return NULL;

  /* find profile according to container type and audio profiles */
  for (i = 0; aac_profiles_mapping[i].profile; i++)
    if (aac_profiles_mapping[i].ct == ct &&
        aac_profiles_mapping[i].ap == ap)
      return aac_profiles_mapping[i].profile;
  
  return NULL;
}

dlna_registered_profile_t dlna_profile_audio_mpeg4 = {
  DLNA_PROFILE_AUDIO_MPEG4,
  DLNA_CLASS_AUDIO,
  "aac,adts,3gp,mp4,mov,qt,m4a",
  probe_mpeg4,
  NULL
};
