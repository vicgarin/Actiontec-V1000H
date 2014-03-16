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

#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

typedef enum {
  CT_UNKNOWN,
  CT_IMAGE, /* for PNG and JPEG */
  CT_ASF, /* usually for WMA/WMV */
  CT_AMR,
  CT_AAC,
  CT_AC3,
  CT_MP3,
  CT_WAV,
  CT_MOV,
  CT_3GP,
  CT_MP4,
  CT_FF_MPEG, /* FFMPEG "mpeg" */
  CT_FF_MPEG_TS, /* FFMPEG "mpegts" */
  CT_MPEG_ELEMENTARY_STREAM,
  CT_MPEG_PROGRAM_STREAM,
  CT_MPEG_TRANSPORT_STREAM,
  CT_MPEG_TRANSPORT_STREAM_DLNA,
  CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS,
} dlna_container_type_t;

dlna_container_type_t stream_get_container (AVFormatContext *ctx);

#endif /* _CONTAINERS_H_ */
