///*
// * libdlna: reference DLNA standards implementation.
// * Copyright (C) 2007 Benjamin Zores <ben@geexbox.org>
// *
// * This file is part of libdlna.
// *
// * libdlna is free software; you can redistribute it and/or
// * modify it under the terms of the GNU Lesser General Public
// * License as published by the Free Software Foundation; either
// * version 2.1 of the License, or (at your option) any later version.
// *
// * libdlna is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// * Lesser General Public License for more details.
// *
// * You should have received a copy of the GNU Lesser General Public
// * License along with libdlna; if not, write to the Free Software
// * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
// */
//
//#include <stdio.h>
//#include <string.h>
//
//#include "dlna_internals.h"
//
//#define UPNP_DMS_DESCRIPTION \
//"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
//"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
//"  <specVersion>" \
//"    <major>1</major>" \
//"    <minor>0</minor>" \
//"  </specVersion>" \
//"  <device>" \
//"    <deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>" \
//"    <friendlyName>%s: 1</friendlyName>" \
//"    <manufacturer>%s</manufacturer>" \
//"    <manufacturerURL>%s</manufacturerURL>" \
//"    <modelDescription>%s</modelDescription>" \
//"    <modelName>%s</modelName>" \
//"    <modelNumber>%s</modelNumber>" \
//"    <modelURL>%s</modelURL>" \
//"    <serialNumber>%s</serialNumber>" \
//"    <UDN>uuid:%s</UDN>" \
//"    <presentationURL>%s</presentationURL>" \
//"    <dlna:X_DLNADOC>DMS-1.00</dlna:X_DLNADOC>" \
//"    <serviceList>" \
//"      <service>" \
//"        <serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>" \
//"        <serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>" \
//"        <SCPDURL>%s</SCPDURL>" \
//"        <controlURL>%s</controlURL>" \
//"        <eventSubURL>%s</eventSubURL>" \
//"      </service>" \
//"      <service>" \
//"        <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>" \
//"        <serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId>" \
//"        <SCPDURL>%s</SCPDURL>" \
//"        <controlURL>%s</controlURL>" \
//"        <eventSubURL>%s</eventSubURL>" \
//"      </service>" \
//"    </serviceList>" \
//"  </device>" \
//"</root>"
//
//char *
//dlna_dms_description_get (const char *friendly_name,
//                          const char *manufacturer,
//                          const char *manufacturer_url,
//                          const char *model_description,
//                          const char *model_name,
//                          const char *model_number,
//                          const char *model_url,
//                          const char *serial_number,
//                          const char *uuid,
//                          const char *presentation_url,
//                          const char *cms_scpd_url,
//                          const char *cms_control_url,
//                          const char *cms_event_url,
//                          const char *cds_scpd_url,
//                          const char *cds_control_url,
//                          const char *cds_event_url)
//{
//  char *desc = NULL;
//  size_t len;
//
//  if (!friendly_name || !manufacturer || !manufacturer_url ||
//      !model_description || !model_name || !model_number ||
//      !model_url || !serial_number || !uuid || !presentation_url ||
//      !cms_scpd_url || !cms_control_url || !cms_event_url ||
//      !cds_scpd_url || !cds_control_url || !cds_event_url)
//    return NULL;
//  
//  len = strlen (UPNP_DMS_DESCRIPTION) + strlen (friendly_name)
//    + strlen (manufacturer) + strlen (manufacturer_url)
//    + strlen (model_description) + strlen (model_name)
//    + strlen (model_number) + strlen (model_url) + strlen (serial_number)
//    + strlen (uuid) + strlen (presentation_url) + strlen (cms_scpd_url)
//    + strlen (cms_control_url) + strlen (cms_event_url)
//    + strlen (cds_scpd_url) + strlen (cds_control_url)
//    + strlen (cds_event_url) + 1;
//
//  desc = malloc (len);
//  memset (desc, 0, len);
//  sprintf (desc, UPNP_DMS_DESCRIPTION, friendly_name,
//           manufacturer, manufacturer_url, model_description,
//           model_name, model_number, model_url, serial_number, uuid,
//           presentation_url, cms_scpd_url, cms_control_url, cms_event_url,
//           cds_scpd_url, cds_control_url, cds_event_url);
//
//  return desc;
//}
