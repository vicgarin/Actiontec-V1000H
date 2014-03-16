/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
# 
# 
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License, version 2, as published by  
# the Free Software Foundation (the "GPL"). 
# 
#
# 
# This program is distributed in the hope that it will be useful,  
# but WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
# GNU General Public License for more details. 
#  
# 
#  
#   
# 
# A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by 
# writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
# Boston, MA 02111-1307, USA. 
#
 *
 ************************************************************************/

#include "cms.h"
#include "cms_util.h"
#include "oal.h"


static CmsEntityInfo entityInfoArray[] = {

   {EID_TR69C,        /* eid */
    NDA_ACCESS_TR69C, /* TR69 attribute access bit */
    "tr69c",          /* name */
    "/bin/tr69c",     /* path */
    "",               /* run args */
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_LAUNCH_ON_BOOT|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE|EIF_IPV6,
#else
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_LAUNCH_ON_BOOT|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
#endif
    1,                /* backlog for TCP server port */
    TR69C_CONN_REQ_PORT,       /* TCP server port */
#ifdef SUPPORT_TR69C
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},         /* misc mem usage parameters */

   {EID_TR64C,
    NDA_ACCESS_TR64C,
    "tr64c",
    "/bin/tr64c",
    "",
    EIF_MDM|EIF_LAUNCH_ON_BOOT,
    0,
    TR64C_HTTP_CONN_PORT,
#ifdef SUPPORT_TR64C
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_HTTPD,
    NDA_ACCESS_HTTPD,
    "httpd",
    "/bin/httpd",
    "",
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE|EIF_IPV6,
#else
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
#endif
    3,                /* backlog for TCP server port */
    HTTPD_PORT,       /* TCP server port */
#ifdef SUPPORT_HTTPD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif

    0, 0, 0},

   {EID_SNMPD,
    NDA_ACCESS_SNMPD,
    "snmpd",
    "/bin/snmpd",
    "",
    EIF_MDM|EIF_MESSAGING_CAPABLE,
    /*    EIF_MDM, */
    0,
    SNMPD_PORT,
#ifdef SUPPORT_SNMP
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_CONSOLED,
    NDA_ACCESS_CONSOLED,
    "consoled",
    "/bin/consoled",
    "",
    EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
#ifdef SUPPORT_CONSOLED
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_TELNETD,
    NDA_ACCESS_TELNETD,
    "telnetd",
    "/bin/telnetd",
    "",
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE|EIF_IPV6,
#else
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
#endif
    3,             /* backlog for TCP server port */
    TELNETD_PORT,
#ifdef SUPPORT_TELNETD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_SSHD,
    NDA_ACCESS_SSHD,
    "sshd",
    "/bin/sshd",
    "",
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE|EIF_IPV6,
#else
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
#endif
    3,             /* backlog for TCP server port */
    SSHD_PORT,
#ifdef SUPPORT_SSHD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_UPNP,
    NDA_ACCESS_UPNP,
    "upnp",
    "/bin/upnp",
    "",
#if defined(AEI_VDSL_CUSTOMER_NCS)
    EIF_AUTO_RELAUNCH|
#endif
    EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
#ifdef SUPPORT_UPNP
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_LASERD,
    0,
    "laserd",
    "/bin/laserd",
    "",
#ifdef DMP_X_ITU_ORG_GPON_1
    EIF_LAUNCH_ON_BOOT|EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
#else
    EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},


   {EID_OMCID,
    0,
    "omcid",
    "/bin/omcid",
    "-v 0 start",
    EIF_LAUNCH_ON_BOOT|EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
#ifdef DMP_X_ITU_ORG_GPON_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},


   /*
    */
   {EID_OMCIPMD,
    0,
    "omcipmd",
    "/bin/omcipmd",
    "",
    EIF_LAUNCH_ON_BOOT|EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
#ifdef DMP_X_ITU_ORG_GPON_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

#ifndef DMP_X_ITU_ORG_GPON_1
    {EID_CVTPSI,
     0,
     "cvtpsi",
     "/bin/cvtpsi",
     NULL,
     0,
     0,
     0,
     FALSE,            /* isFeatureCompiledIn */
     0, 0, 0},
#endif

   {EID_VECTORINGD,
    NDA_ACCESS_VECTORINGD,
    "vectoringd",
    "/bin/vectoringd",
    NULL,
    EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_LAUNCH_ON_BOOT,
    0,
    0,
#ifdef SUPPORT_VECTORINGD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_SMD,
    0,
    "smd",
    "/bin/smd",
    "",
    EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,   /* flags */
    0,
    0,
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_SSK,
    0,
    "ssk",
    "/bin/ssk",
    "",
    EIF_LAUNCH_IN_STAGE_1|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_PPP,
    0,
    "pppd",
    "/bin/pppd",
    "-c",
    EIF_MESSAGING_CAPABLE|EIF_MULTIPLE_INSTANCES,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_DHCPC,
    0,
    "dhcpc",
    "/bin/dhcpc",
    "-f",  /* stay in foreground, we don't want it to fork or daemonize */
    EIF_MESSAGING_CAPABLE|EIF_MULTIPLE_INSTANCES,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_DHCPD,
    0,
    "dhcpd",
    "/bin/dhcpd",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef SUPPORT_UDHCP
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_FTPD,
    0,
    "bftpd",
    "/bin/bftpd",
    "",
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_IPV6,
#else
    EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE,
#endif
    1, /* backlog for TCP server port */
    FTPD_PORT,
#ifdef SUPPORT_FTPD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_TFTPD,
    0,
    "tftpd",
    "/usr/bin/tftpd",
    "",
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_SERVER|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE|EIF_IPV6,
#else
    EIF_SERVER|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
#endif
    0,
    TFTPD_PORT,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

    {EID_TFTP,  /* this is the client */
    0,
    "tftp",
    "/usr/bin/tftp",
    "",
    EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

#if defined(AEI_VDSL_CES)
   {EID_CES,
    NDA_ACCESS_HTTPD,
    "ces",
    "/nagios/bin/ces",
    "",                 /* run args */
   EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},
#endif

   {EID_DNSPROBE,
    0,                  /* TR69 attribute access bit */
    "dnsprobe",
    "/bin/dnsprobe",
    "",                 /* run args */
    0,                  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_DNSPROXY,
    0,
    "dnsproxy",
    "/bin/dnsproxy",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE|EIF_AUTO_RELAUNCH,  /* EIF_ flags */
    0,
    0,
#ifdef DMP_X_BROADCOM_COM_DNSPROXY_1 /* aka SUPPORT_DNSPROXY */
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_SYSLOGD,
    0,                  /* TR69 attribute access bit */
    "syslogd",
    "/sbin/syslogd",
    "-n",               /* run args (don't daemonize) */
    0,                  /* flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_KLOGD,
    0,                  /* TR69 attribute access bit */
    "klogd",
    "/sbin/klogd",
    "-n",               /* run args (don't daemonize) */
    0,                  /* flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_VODSL,
    NDA_ACCESS_VODSL,   /* tr69 attribute access bit, not applicable to vodsl */
    "vodsl",
    "/bin/vodsl",
    "",           /* run args */
    EIF_MDM|EIF_MESSAGING_CAPABLE|EIF_AUTO_RELAUNCH,
    0,            /* backlog for TCP server port, if this is a tcp server */
    0,            /* TCP/UDP server port */
#ifdef SUPPORT_VOXXXLOAD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},     /* misc memory parameters */

   {EID_DECTDBGD,
    NDA_ACCESS_DECTDBGD,   /* tr69 attribute access bit, not applicable to vodsl */
    "dectdbgd_NEW",
    "/etc/dectdbgd",
    "",           /* run args */
    EIF_MDM|EIF_MESSAGING_CAPABLE,
    0,            /* backlog for TCP server port, if this is a tcp server */
    0,            /* TCP/UDP server port */
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},     /* misc memory parameters */

   {EID_DDNSD,
    0,                  /* TR69 attribute access bit */
    "ddnsd",
    "/bin/ddnsd",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_ZEBRA,
    0,                  /* TR69 attribute access bit */
    "zebra",
    "/bin/zebra",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_RIPD,
    0,                  /* TR69 attribute access bit */
    "ripd",
    "/bin/ripd",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_SNTP,
    0,                  /* TR69 attribute access bit */
    "sntp",
    "/bin/sntp",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_URLFILTERD,
    0,
    "urlfilterd",
    "/bin/urlfilterd",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,
    0,
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_IGMP,
    0,
    "igmp",
    "/bin/igmp",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_DHCP6C,
    0,
    "dhcp6c",
    "/bin/dhcp6c",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE|EIF_MULTIPLE_INSTANCES,
    0,
    0,
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_DHCP6S,
    0,
    "dhcp6s",
    "/bin/dhcp6s",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_RADVD,
    0,
    "radvd",
    "/bin/radvd",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_IPPD,
    0,
    "ippd",
    "/bin/ippd",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef SUPPORT_IPP
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_DSLDIAGD,
    0,
    "dsldiagd",
    "/bin/dsldiagd",
    "",                 /* run args */
    EIF_LAUNCH_ON_BOOT,
    0,
    0,
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
    FALSE,            /*so much compile error after disable DSL, so rescue memory only here*/
#else
#ifdef SUPPORT_DSLDIAGD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
#endif
    0, 0, 0},

   {EID_SOAPSERVER,
    0,
    "soapserver",
    "/bin/soapserver",
    "-f",                /* run args */
    EIF_LAUNCH_ON_BOOT,
    0,
    0,
#ifdef SUPPORT_SOAP
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

    {EID_FTP,   /* this is FTP the client */
    0,
    "ftp",
    "/usr/bin/ftp",
    "",
    EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_MLD,
    0,
    "mld",
    "/bin/mld",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_UNITTEST,
    0,
    "ut",
    "/",
    "",
    0,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_PING,
    0,
    "ping",
    "/bin/ping",
    "",                 /* run args */
#ifdef DMP_X_ITU_ORG_GPON_1
    EIF_MESSAGING_CAPABLE | EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
#else
    EIF_MESSAGING_CAPABLE ,  /* EIF_ flags */
#endif
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */
#ifdef AEI_CONTROL_PING6
   {EID_PING6,
    0,
    "ping6",
    "/bin/ping6",
    "",                 /* run args */
#ifdef DMP_X_ITU_ORG_GPON_1
    EIF_MESSAGING_CAPABLE | EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
#else
    EIF_MESSAGING_CAPABLE ,  /* EIF_ flags */
#endif
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */
#endif

#if defined(DMP_TRACEROUTE_1)
   {EID_TRACEROUTE,
    0,
    "traceroute",
    "/usr/bin/traceroute",
    "",                 /* run args */
#ifdef DMP_X_ITU_ORG_GPON_1
    EIF_MESSAGING_CAPABLE | EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
#else
    EIF_MESSAGING_CAPABLE ,  /* EIF_ flags */
#endif
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
#ifdef DMP_X_ITU_ORG_GPON_1
    TRUE,               /* isFeatureCompiledIn */
#else
    FALSE,
#endif
    0, 0, 0},           /* misc memory parameters */
#endif

#ifdef AEI_CONTROL_TRACEROUTE6
   {EID_TRACEROUTE6,
    0,
    "traceroute6",
    "/usr/bin/traceroute6",
    "",                 /* run args */
#ifdef DMP_X_ITU_ORG_GPON_1
    EIF_MESSAGING_CAPABLE | EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
#else
    EIF_MESSAGING_CAPABLE ,  /* EIF_ flags */
#endif
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
#ifdef DMP_X_ITU_ORG_GPON_1
    TRUE,               /* isFeatureCompiledIn */
#else
    FALSE,
#endif
    0, 0, 0},           /* misc memory parameters */
#endif

   {EID_PWRCTL,
    0,
    "pwrctl",
    "/bin/pwrctl",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */

   {EID_HOTPLUG,
    0,
    "hotplug",
    "/bin/hotplug",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef DMP_STORAGESERVICE_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_SAMBA,
    0,
    "smbd",
    "/bin/smbd",
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT) && defined(SUPPORT_SAMBA)
    "-i",
#else
    "-D",
#endif
	0,
    0,
    0,
#ifdef SUPPORT_SAMBA
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_MISC,
    0,
    "misc",
    "/",
    "",
    0,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

   {EID_WLMNGR,
    NDA_ACCESS_WLAN,
    "WLMNGR-DAEMON",
    "/bin/wlmngr",
    "",
    EIF_MDM|EIF_MESSAGING_CAPABLE, //|EIF_DESKTOP_LINUX_CAPABLE |EIF_LAUNCH_ON_BOOT,
    0,
    0,
#ifdef BRCM_WLAN
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,             /* isFeatureCompiledIn */
#endif
    0, 0, 0},

 {EID_WLNVRAM,
    NDA_ACCESS_WLAN,
    "WLNVRAM",
    "/bin/nvram",
    "",
    EIF_MDM|EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef BRCM_WLAN
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,             /* isFeatureCompiledIn */
#endif
    0, 0, 0},
    
   {EID_WLEVENT,
    NDA_ACCESS_WLAN,
    "WLEVENT",
    "/bin/wlevt",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef BRCM_WLAN
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,             /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_WLWPS,
    NDA_ACCESS_WLAN,
    "WLWPS",
    "/bin/wps_monitor",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef BRCM_WLAN
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,             /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_WLWAPID,
    NDA_ACCESS_WLAN,
    "WLWAPID",
    "/bin/wapid",
    "",
    EIF_MESSAGING_CAPABLE, //|EIF_DESKTOP_LINUX_CAPABLE |EIF_LAUNCH_ON_BOOT,
    0,
    0,
#ifdef BCMWAPI_WAI
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,             /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_CMFD,
    0,
    "cmfd",
    "/bin/cmfd",
    "",
    EIF_LAUNCH_ON_BOOT|EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef SUPPORT_CMFD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_MCPD,
    0,
    "mcpd",
    "/bin/mcpd",
    "",                 /* run args */
#if defined(AEI_VDSL_CUSTOMER_NCS)
    EIF_MESSAGING_CAPABLE | EIF_AUTO_RELAUNCH,  /* flags (later make it desktop capable) */
#else
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
#endif
    0,
    0,
#if defined(SUPPORT_IGMP) || defined (SUPPORT_MLD)
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_MCPCTL,
    0,
    "mcpctl",
    "/bin/mcpctl",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* flags (later make it desktop capable) */
    0,
    0,
#if defined(SUPPORT_IGMP) || defined (SUPPORT_MLD)
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_L2TPD,
    0,
    "l2tpd",
    "/bin/openl2tpd",
    "",                 /* run args */
    0,
    0,
    0,
#ifdef DMP_X_BROADCOM_COM_L2TPAC_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_PPTPD,
    0,
    "pptp",
    "/bin/pptp",
    "",                 /* run args */
    0,
    0,
    0,
#ifdef DMP_X_BROADCOM_COM_PPTPAC_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_MOCAD,
    0,
    "mocad",
    "/bin/mocad",
    "",
    EIF_MESSAGING_CAPABLE|EIF_MULTIPLE_INSTANCES,
    0,
    0,
#ifdef BRCM_MOCA_DAEMON
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_RNGD,
    0,
    "rngd",
    "/bin/rngd",
    "",                 /* run args */
    EIF_LAUNCH_ON_BOOT,
    0,
    0,
#ifdef SUPPORT_HWRANDOM
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_DMSD,
    0,
    "bcmmserver",
    "/bin/bcmmserver",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef DMP_X_BROADCOM_COM_DLNA_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_SWMDK,
    //NDA_ACCESS_SWMDK,
    0,
    "swmdk",
    "/bin/swmdk",
    "",                 /* run args */
    EIF_LAUNCH_ON_BOOT,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},

#ifdef DMP_DOWNLOAD_1
   {EID_TR143_DLD,
    0,
    "tr143-dld",
    "/bin/tr143",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif

#ifdef DMP_UPLOAD_1
   {EID_TR143_UPLD,
    0,
    "tr143-upld",
    "/bin/tr143",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif

#ifdef DMP_UDPECHO_1
   {EID_TR143_ECHO,
    0,
    "tr143-echo",
    "/bin/tr143",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif

   {EID_EPON_APP,
    NDA_ACCESS_EPONAPP,
    "eponapp",
    "/bin/eponapp",
    "",                 /* run args */
    EIF_MDM | EIF_LAUNCH_ON_BOOT | EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef DMP_X_BROADCOM_COM_EPON_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,             /* isFeatureCompiledIn */
#endif
    0, 0, 0},

{EID_OSGID,        /* eid */
    NDA_ACCESS_TR69C, /* attribute access bit */
    "osgid",          /* name */
    "/bin/osgid",     /* path */
    "",               /* run args */
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_LAUNCH_ON_BOOT|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    0,                /* backlog for TCP server port: TO_DO, should be NULL? */
    0,       /* TCP server port: TO_DO; should be NULL? unless felix is sending somethign up*/
#ifdef DMP_X_BROADCOM_COM_OSGI_JVM_1
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},         /* misc mem usage parameters */

   {EID_DECT,
    NDA_ACCESS_VODSL,   /* tr69 attribute access bit, not applicable to vodsl */
    "dectd",
    "/bin/dectd",
    "",           /* run args */
    EIF_MDM|EIF_MESSAGING_CAPABLE,
    0,            /* backlog for TCP server port, if this is a tcp server */
    0,            /* TCP/UDP server port */
#ifdef SUPPORT_VOXXXLOAD
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},     /* misc memory parameters */

   {EID_BMUD,
    0,
    "bmud",
    "/bin/bmud",
    "",                 /* run args */
    EIF_LAUNCH_ON_BOOT|EIF_MESSAGING_CAPABLE,
    0,
    0,
#ifdef SUPPORT_BMU
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},

   {EID_BMUCTL,
    0,
    "bmuctl",
    "/bin/bmuctl",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
#ifdef SUPPORT_BMU
    TRUE,             /* isFeatureCompiledIn */
#else
    FALSE,            /* isFeatureCompiledIn */
#endif
    0, 0, 0},           /* misc memory parameters */
#if defined(AEI_VDSL_DETECT_WAN_SERVICE) || defined(AEI_VDSL_TELUS_DETECT_WAN_SERVICE)
   {EID_DETECT_WAN_SERVICE,
    0,
    "detectWANService",
    "/bin/detectWANService",
    "",                 /* run args */
   EIF_MESSAGING_CAPABLE|EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},
#endif

#if defined(AEI_VDSL_DETECT_WAN_SERVICE)
    {EID_SET_DETECT_WAN_SERVICE,
    NDA_ACCESS_TR69C,
    "setdetectWANService",
    "/bin/setdetectWANService",
    "",                 /* run args */
    EIF_MDM|EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},
#endif

#if defined(AEI_VDSL_MYNETWORK)
   {EID_MYNETWORK,
    0,
    "mynetwork",
    "/bin/mynetwork",
    "",
    EIF_MESSAGING_CAPABLE|EIF_LAUNCH_ON_BOOT|EIF_DESKTOP_LINUX_CAPABLE ,
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},
#endif

#ifdef AEI_VDSL_HPNA
   {EID_INHPNA,
    0,
    "inhpna",
    "/bin/inhpna",
    "-i eth4 -b br0",
    EIF_LAUNCH_ON_BOOT | EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},

#ifdef AEI_VDSL_CUSTOMER_TELUS
   {EID_NETPERD,
    0,
    "netperd",
    "/bin/netperd",
    "-i br0 -p 900",
    EIF_LAUNCH_ON_BOOT | EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif

#ifdef AEI_VDSL_WT107
   {EID_HPNA_NODESTATS,
    0,
    "hpna_nodestats",
    "/bin/node_stats",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif
#endif

#if defined(AEI_VDSL_STATS_DIAG)
   {EID_STATS_DIAG,
    0,
    "statsdiag",
    "/bin/statsdiag",
    "",
    EIF_MESSAGING_CAPABLE,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif 

#if defined(AEI_VDSL_SMARTLED)
    {EID_SMARTLED,
    0,
    "smartled",
    "/bin/smartled",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */
#endif
#if defined(AEI_VDSL_SMARTLED)  || defined (AEI_VDSL_CUSTOMER_BELLALIANT)
   {EID_NSLOOKUP,
    0,
    "nslookup",
    "/usr/bin/nslookup",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},
#endif
#if defined(AEI_VDSL_CUSTOMER_BELLALIANT)
    {EID_LEDCTL,
    0,
    "ledctl",
    "/bin/ledctl",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    {EID_SAVESYSLOG,
    0,
    "savesyslog",
    "/bin/savesyslog",
    "",                 /* run args */
    EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
    0,                  /* backlog for TCP server port */
    0,                  /* TCP/UDP server port */
    TRUE,               /* isFeatureCompiledIn */
    0, 0, 0},           /* misc memory parameters */
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
#ifdef SUPPORT_HTTPD_SSL
   {EID_HTTPSD,
    NDA_ACCESS_HTTPD,
    "httpd", /*combine httpsd with httpd process*/
    "/bin/httpd",
    "-s", //launch httpd with SSL support
#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE|EIF_IPV6,
#else
    EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
#endif
    3,                /* backlog for TCP server port */
    HTTPDS_PORT,       /* TCP server port */
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},
#endif
#endif
    {EID_MODUPDTD,
     0,
     "modupdtd",
     "/bin/modupdtd",
     "",                 /* run args */
     EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
     0,                  /* backlog for TCP server port */
     0,                  /* TCP/UDP server port */
 #ifdef SUPPORT_MOD_SW_UPDATE
     TRUE,             /* isFeatureCompiledIn */
 #else
     FALSE,            /* isFeatureCompiledIn */
 #endif
     0, 0, 0},           /* misc memory parameters */

    {EID_MODUPDTV,
      0,
      "modupdtv",
      "",                 /* this app is never launched by smd */
      "",                 /* run args */
      EIF_MESSAGING_CAPABLE,  /* EIF_ flags */
      0,                  /* backlog for TCP server port */
      0,                  /* TCP/UDP server port */
  #ifdef SUPPORT_MOD_SW_UPDATE
      TRUE,             /* isFeatureCompiledIn */
  #else
      FALSE,            /* isFeatureCompiledIn */
  #endif
      0, 0, 0},           /* misc memory parameters */
#ifdef AEI_CONTROL_LAYER
   {EID_AEI_RTD,
    NDA_ACCESS_RTD, //NDA_ACCESS_WLAN,
    "rtd",
    "/bin/rtd",
    "", //parameter
    //EIF_MDM|EIF_SERVER|EIF_SERVER_TCP|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE,
    EIF_MDM|EIF_LAUNCH_ON_BOOT|EIF_MESSAGING_CAPABLE|EIF_DESKTOP_LINUX_CAPABLE ,
    0,                /* backlog for TCP server port */
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},
#endif

#if defined(AEI_VDSL_CPU_SYSLOG)
   {EID_CPULOGD,
    0,
    "cpulogd",
    "/sbin/cpulogd",
    "",
    0,
    0,
    0,
    TRUE,
    0, 0, 0},
#endif
#if defined(AEI_VDSL_CAPTIVE_PAGES)
   {EID_IPTABLES,
    0,
    "iptables",
    "/bin/iptables",
    "",                 /* run args */
	EIF_MULTIPLE_INSTANCES,  /* EIF_ flags */
    0,
    0,
    TRUE,             /* isFeatureCompiledIn */
    0, 0, 0},
#endif
};


#define NUM_ENTITY_INFO_ENTRIES (sizeof(entityInfoArray)/sizeof(CmsEntityInfo))


UINT32 cmsEid_getNumberOfEntityInfoEntries(void)
{
   return NUM_ENTITY_INFO_ENTRIES;
}


const CmsEntityInfo *cmsEid_getFirstEntityInfo(void)
{
   return &(entityInfoArray[0]);
}


const CmsEntityInfo *cmsEid_getEntityInfo(CmsEntityId eid)
{
   UINT32 i=0;

   while (i < NUM_ENTITY_INFO_ENTRIES)
   {
      if (entityInfoArray[i].eid == eid)
      {
         return (&entityInfoArray[i]);
      }

      i++;
   }

   return NULL;
}


const CmsEntityInfo *cmsEid_getEntityInfoByAccessBit(UINT16 bit)
{
   UINT32 i=0;

   if (bit == 0)
   {
      return NULL;
   }

   while (i < NUM_ENTITY_INFO_ENTRIES)
   {
      if (entityInfoArray[i].accessBit == bit)
      {
         return (&entityInfoArray[i]);
      }

      i++;
   }

   return NULL;
}


const CmsEntityInfo *cmsEid_getEntityInfoByStringName(const char *name)
{
   UINT32 i=0;

   if (name == NULL)
   {
      return NULL;
   }

   while (i < NUM_ENTITY_INFO_ENTRIES)
   {
      if (!strcmp(entityInfoArray[i].name, name))
      {
         return (&entityInfoArray[i]);
      }

      i++;
   }

   return NULL;
}


CmsRet cmsEid_getStringNamesFromBitMask(UINT16 bitMask, char **buf)
{
   UINT32 i, mask;
   UINT32 len=1; /* for empty mask, return a buffer with an empty string. */
   UINT32 idx=0;
   UINT32 numBits = sizeof(bitMask) * 8;
   const CmsEntityInfo *info;
   UBOOL8 firstName=TRUE;
   CmsRet ret = CMSRET_SUCCESS;

   /* first loop calculates how much space needed for string names */
   for (mask=1, i=0; i < numBits; i++)
   {
      info = cmsEid_getEntityInfoByAccessBit(bitMask & mask);
      if (info == NULL)
      {
         if (bitMask & mask)
         {
            cmsLog_error("unrecognized bitmask 0x%x", (bitMask & mask));
            ret = CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED;
         }
      }
      else
      {
         len += strlen(info->name) + 1;
      }

      mask = mask << 1;
   }

   if (((*buf) = cmsMem_alloc(len, ALLOC_ZEROIZE)) == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /* this loop copies string names into buffer */
   for (mask=1, i=0; i < numBits; i++)
   {
      info = cmsEid_getEntityInfoByAccessBit(bitMask & mask);
      if (info != NULL)
      {
         if (firstName)
         {
            idx = sprintf((*buf), "%s", info->name);
            firstName = FALSE;
         }
         else
         {
            idx += sprintf(&((*buf)[idx]), ",%s", info->name);
         }
      }

      mask = mask << 1;
   }

   cmsAst_assert(idx < len);

   return ret;
}



CmsRet cmsEid_getBitMaskFromStringNames(const char *buf, UINT16 *bitMask)
{
   const char *start;
   const char *end;
   const CmsEntityInfo *info;
   CmsRet ret = CMSRET_SUCCESS;

   if (bitMask == NULL)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }

   *bitMask = 0;

   if ((buf == NULL) || (strlen(buf) == 0))
   {
      /* null or empty string means no bits are set */
      return CMSRET_SUCCESS;
   }

   start = buf;
   end = strchr(start, ',');
   while (end != NULL)
   {
      char name[BUFLEN_256]; /* this should be long enough to hold string names */

      cmsAst_assert((end - start + 1) < (SINT32) sizeof(name));
      snprintf(name, end - start + 1, "%s", start);
      info = cmsEid_getEntityInfoByStringName(name);
      if (info == NULL)
      {
         cmsLog_debug("ignoring name %s", name);
         ret = CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED;
      }
      else
      {
         (*bitMask) |= info->accessBit;
      }

      start = end+1;
      while ((*start == ' ') && (*start != 0))
      {
         start++;
      }

      if (*start != 0)
      {
         end = strchr(start, ',');
      }
      else
      {
         end = NULL;
      }
   }

   /* there is one more name at the end (with no trailing comma) */
   info = cmsEid_getEntityInfoByStringName(start);
   if (info == NULL)
   {
      cmsLog_debug("ignoring name %s", start);
      ret = CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED;
   }
   else
   {
      (*bitMask) |= info->accessBit;
   }



   return ret;
}


SINT32 cmsEid_getPid(void)
{
   return (oal_getPid());
}
