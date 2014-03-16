#ifndef __FAP4KE_PACKET_H_INCLUDED__
#define __FAP4KE_PACKET_H_INCLUDED__

/*

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved

<:label-BRCM:2011:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *******************************************************************************
 * File Name  : fap4ke_packet.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

//#define CC_FAP4KE_PKT_FFE

//#define CC_FAP4KE_PKT_TEST

//#define CC_FAP4KE_PKT_HW_ICSUM

#define CC_FAP4KE_PKT_ERROR_CHECK

#define CC_FAP4KE_PKT_IPV6_GSO

#if defined(CONFIG_BLOG_IPV6)
#define CC_FAP4KE_PKT_IPV6_SUPPORT
#endif

//#define CC_FAP4KE_PKT_IPV6_FRAGMENTATION

/* IPv4 Dot Decimal Notation formating */
#define IP4DDN   " <%03u.%03u.%03u.%03u>"
#define IP4PDDN  " <%03u.%03u.%03u.%03u:%05u>"
#define IP4(ip) ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

#define IP6HEX  "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x>"
#define IP6PHEX "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%u>"
#define IP6(ip) ((uint16_t*)&ip)[0], ((uint16_t*)&ip)[1],   \
                ((uint16_t*)&ip)[2], ((uint16_t*)&ip)[3],   \
                ((uint16_t*)&ip)[4], ((uint16_t*)&ip)[5],   \
                ((uint16_t*)&ip)[6], ((uint16_t*)&ip)[7]

#define ETH_ALEN           6
#define BRCM_TAG_LENGTH    6
#define ETHERTYPE_LENGTH   2
#define IPOA_HLEN          8

#define MAX_FAP_MTU        1500
#define MIN_FAP_MTU        60

/* IPv4 Multicast range: 224.0.0.0 to 239.255.255.255 (E0.*.*.* to EF.*.*.*) */
#define FAP4KE_PKT_MCAST_IPV4_MASK  0xF0000000
#define FAP4KE_PKT_MCAST_IPV4_VAL   0xE0000000
#define FAP4KE_PKT_SSM_IPV4_MASK    0xFF000000
#define FAP4KE_PKT_SSM_IPV4_VAL     0xE8000000 /* 232.*.*.* */

#define FAP4KE_PKT_IS_MCAST_IPV4(_addr)                                  \
    ( ((_addr) & FAP4KE_PKT_MCAST_IPV4_MASK) == FAP4KE_PKT_MCAST_IPV4_VAL)

#define FAP4KE_PKT_IS_SSM_IPV4(_addr)                                    \
    ( ((_addr) & FAP4KE_PKT_SSM_IPV4_MASK) == FAP4KE_PKT_SSM_IPV4_VAL )


/* IPv6 Multicast range:  FF00::/8  */
#define FAP4KE_PKT_MCAST_IPV6_VAL   0xFF
/* IPv6 Multicast SSM range:  FF30::/96  */
#define FAP4KE_PKT_SSM_IPV6_MASK    0xFF30FFFF
#define FAP4KE_PKT_SSM_IPV6_VAL     0xFF300000 

#define FAP4KE_PKT_IS_MCAST_IPV6(_addr)                                  \
    ( (_addr)  == FAP4KE_PKT_MCAST_IPV6_VAL)

#define FAP4KE_PKT_IS_SSM_IPV6(_addr)                                    \
    ( ((_addr) & FAP4KE_PKT_SSM_IPV6_MASK) == FAP4KE_PKT_SSM_IPV6_VAL )

/* TODO: fine tune the max flows for all configurations */
#if defined(CONFIG_BCM963268)
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE) ||  defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
/* 63268, IPV6, BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         60
#define FAP4KE_PKT_MAX_PSM_FLOWS         75  
#define FAP4KE_PKT_MAX_QSM_FLOWS         102
#define FAP4KE_PKT_CMDLISTS_IN_PSM       0
#define FAP4KE_PKT_CMDLISTS_IN_QSM       237
#else /* CC_FAP4KE_PKT_IPV6_SUPPORT */
/* 63268, IPV4, BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         84  //85
#define FAP4KE_PKT_MAX_PSM_FLOWS         107
#define FAP4KE_PKT_MAX_QSM_FLOWS         184
#define FAP4KE_PKT_CMDLISTS_IN_PSM       0
#define FAP4KE_PKT_CMDLISTS_IN_QSM       376
#endif /* else CC_FAP4KE_PKT_IPV6_SUPPORT */
#else /* BPM or IQ */
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
/* 63268, IPV6, no BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         69
#define FAP4KE_PKT_MAX_PSM_FLOWS         90
#define FAP4KE_PKT_MAX_QSM_FLOWS         80
#define FAP4KE_PKT_CMDLISTS_IN_PSM       0
#define FAP4KE_PKT_CMDLISTS_IN_QSM       239
#else /* CC_FAP4KE_PKT_IPV6_SUPPORT */
/* 63268, IPV4, no BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         102 //103
#define FAP4KE_PKT_MAX_PSM_FLOWS         208
#define FAP4KE_PKT_MAX_QSM_FLOWS         135
#define FAP4KE_PKT_CMDLISTS_IN_PSM       0
#define FAP4KE_PKT_CMDLISTS_IN_QSM       446
#endif /* CC_FAP4KE_PKT_IPV6_SUPPORT */
#endif /* else BPM or IQ */
#elif defined(CONFIG_BCM96362)
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE) ||  defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
/* 6362 IPV6, BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         60
#define FAP4KE_PKT_MAX_PSM_FLOWS         20
#define FAP4KE_PKT_MAX_QSM_FLOWS         0 
#define FAP4KE_PKT_CMDLISTS_IN_PSM       73
#define FAP4KE_PKT_CMDLISTS_IN_QSM       7
#else /* CC_FAP4KE_PKT_IPV6_SUPPORT */
/* 6362 IPV4, BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         85
#define FAP4KE_PKT_MAX_PSM_FLOWS         43
#define FAP4KE_PKT_MAX_QSM_FLOWS         0
#define FAP4KE_PKT_CMDLISTS_IN_PSM       116
#define FAP4KE_PKT_CMDLISTS_IN_QSM       12
#endif /* else CC_FAP4KE_PKT_IPV6_SUPPORT */
#else /* BPM or IQ */
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
/* 6362 IPV6, no BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         69
#define FAP4KE_PKT_MAX_PSM_FLOWS         42
#define FAP4KE_PKT_MAX_QSM_FLOWS         0
#define FAP4KE_PKT_CMDLISTS_IN_PSM       88
#define FAP4KE_PKT_CMDLISTS_IN_QSM       23
#else /* CC_FAP4KE_PKT_IPV6_SUPPORT */
/* 6362 IPV4, no BPM */
#define FAP4KE_PKT_MAX_DSP_FLOWS         110
#define FAP4KE_PKT_MAX_PSM_FLOWS         76
#define FAP4KE_PKT_MAX_QSM_FLOWS         0
#define FAP4KE_PKT_CMDLISTS_IN_DSP       146
#define FAP4KE_PKT_CMDLISTS_IN_PSM       40
#define FAP4KE_PKT_CMDLISTS_IN_QSM       0
#endif /* CC_FAP4KE_PKT_IPV6_SUPPORT */
#endif /* else BPM or IQ */
#else /* defined(CONFIG_BCM96362) */
#error "CHIP NOT SUPPORTED"
#endif
#define FAP4KE_PKT_MAX_FLOWS             ( FAP4KE_PKT_MAX_DSP_FLOWS + FAP4KE_PKT_MAX_PSM_FLOWS + FAP4KE_PKT_MAX_QSM_FLOWS)

#if ( FAP4KE_PKT_MAX_FLOWS != FAP4KE_PKT_MAX_DSP_FLOWS + FAP4KE_PKT_MAX_PSM_FLOWS + FAP4KE_PKT_MAX_QSM_FLOWS )
#error "FLOWLIST/CMDLIST SIZE MISMATCH"
#endif

#define FAP4KE_PKT_HASH_TABLE_SIZE       128  /* 256 MAX! */
#define FAP4KE_PKT_HASH_TABLE_ENTRY_MAX  FAP4KE_PKT_MAX_FLOWS

#define FAP4KE_PKT_MAX_HEADERS           4

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
/* Maximum header size:
 * 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 40 (IPv6) + 20 (IPv4) + 20 (TCP/UDP) = 126 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_ENET      116
/* Maximum header size:
 * 10 (LLC/SNAP) + 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 40 (IPv6) + 20 (IPv4) + 20 (TCP/UDP) = 126 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_XTM       126
#define FAP4KE_PKT_CMD_LIST_SIZE         168   /* max 156 expected */
#else
/* ENET Maximum header size:
 * 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 20 (IPv4) + 20 (TCP/UDP) = 76 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_ENET      76
/* XTM Maximum header size:
 * 10 (LLC/SNAP) + 14 (ETH) + 6 (BRCM Tag) + 8 (2 VLANs) +
 * 8 (PPPoE) + 20 (IPv4) + 20 (TCP/UDP) = 86 bytes
 */
#define FAP4KE_PKT_HEADER_SIZE_XTM       86
#define FAP4KE_PKT_CMD_LIST_SIZE         96
#endif
#define FAP4KE_PKT_HEADER_SIZE_MAX       FAP4KE_PKT_HEADER_SIZE_XTM

#define FAP4KE_PKT_CSUM_CMD_LIST_SIZE    20

/*
 * Only 8 bits are allocated for channel in fap4kePkt_key_t. This
 * restricts the multicast destination port mask to 8 ports.
 */
#define FAP4KE_PKT_MAX_DEST_PORTS        8

/*
 * Header access macros for 8-bit and 16-bit fields
 */
#define FAP4KE_PKT_HEADER_READ(_field)         \
    ({                                         \
        typeof(_field) *_field_p = &(_field);  \
        *_field_p;                             \
    })

#define FAP4KE_PKT_HEADER_WRITE(_field, _val)  \
    {                                          \
        typeof(_field) *_field_p = &(_field);  \
        *_field_p = (_val);                    \
    }

typedef enum {
    FAP4KE_PKT_LOOKUP_MISS=0,
    FAP4KE_PKT_LOOKUP_HIT,
    FAP4KE_PKT_LOOKUP_DROP,
    FAP4KE_PKT_LOOKUP_MAX
} fap4kePkt_lookup_t;

typedef enum {
    FAP4KE_PKT_PHY_ENET=0,
    FAP4KE_PKT_PHY_XTM,
    FAP4KE_PKT_PHY_ENET_EXT,
    FAP4KE_PKT_PHY_MAX
} fap4kePkt_phy_t;

typedef enum {
    FAP4KE_TUNNEL_NONE,
    FAP4KE_TUNNEL_4in6_UP,
    FAP4KE_TUNNEL_6in4_UP,
    FAP4KE_TUNNEL_4in6_DN,
    FAP4KE_TUNNEL_6in4_DN
} fap4ke_tunnel_type;

typedef struct {
    uint32 key;
    uint16 dmaStatus;
    uint16 mss;
    uint8 bCSUM;
    uint8 source;
    uint8 txChannel;
    uint8 rxChannel;
    uint8 phy;
    uint8 bIpFrag; /* "1" means "TCP IP Fragmentation; "0" means "TCP GSO" */
    uint8 tunnelType;
} fap4kePkt_gso_arg;


typedef struct {
    uint8 *packet_p;
    uint8 *payload_p;
    uint16 len;
    uint8 encapType;
} fap4kePkt_gso_pkt;


typedef struct {
    uint8  macDa[ETH_ALEN];
    uint8  macSa[ETH_ALEN];
    uint16 etherType;
} fap4kePkt_ethHeader_t;

typedef struct {
    uint16 brcmTag[2];
    uint16 etherType;
} fap4kePkt_bcmHeader_t;

typedef struct {
    uint16 brcmTag;
    uint16 etherType;
} __attribute__((packed)) fap4kePkt_bcmHeader2_t;

typedef union {
    struct {
        uint16 pbits : 3;
        uint16 dei   : 1;
        uint16 vid   : 12;
    };
    uint16 u16;
} fap4kePkt_vlanHeaderTci_t;

typedef struct {
    fap4kePkt_vlanHeaderTci_t tci;
    uint16 etherType;
} fap4kePkt_vlanHeader_t;

typedef uint16 fap4kePkt_pppType_t;

typedef struct {
    struct {
        uint8  ver  : 4;
        uint8  type : 4;
    };
    uint8  code;
    uint16 sessionId;
    uint16 length;
    fap4kePkt_pppType_t pppType;
} fap4kePkt_pppoeHeader_t;

typedef union {
    struct {
        union {
            struct {
                uint8 version:4;
                uint8 ihl:4;
            };
            uint8 version_ihl;
        };
        uint8  tos;
        uint16 totalLength;
        uint16 id;
        uint16 fragOffset;
#define FLAGS_CE     0x8000 /* Flag: "Congestion" */
#define FLAGS_DF     0x4000 /* Flag: "Don't Fragment" */
#define FLAGS_MF     0x2000 /* Flag: "More Fragments" */
#define OFFSET_MASK  0x1FFF /* "Fragment Offset" part */
#define FLAGS_SHIFT  13
        uint8  ttl;
        uint8  protocol;
        uint16 csum;
        uint32 ipSa;
        uint32 ipDa;
        /* options... */
    };
    uint32 u32[5];
} fap4kePkt_ipv4Header_t;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT) || defined(CC_FAP4KE_PKT_IPV6_GSO)
typedef union {
    struct {
        uint8   nextHeader;
        uint8   reservedU8;
        uint16  ipFragment;
        uint32  identification;
    };
    uint32 u32[2];
} fap4kePkt_ipv6_fragHeader_t;

typedef union {
    uint8  u8[16];
    uint16 u16[8];
    uint32 u32[4];
} fap4kePkt_ipv6Address_t;

typedef union {
    struct {
        union {
            struct {
                uint32 version:4;
                uint32 tclass:8;
                uint32 flowLabel:20;
            };
            uint32 word0;
        };
        uint16 payloadLen;
        uint8 nextHeader;
        uint8 hopLimit;
        fap4kePkt_ipv6Address_t ipSa;
        fap4kePkt_ipv6Address_t ipDa;
    };
    uint32 u32[10];
} fap4kePkt_ipv6Header_t;


/* this packed verison of header is used to avoid unaligned exceptions
 * when ipv6 header of packet is not word aligned
 */
typedef union {
    struct {
        union {
            struct {
                uint32 ver:4;
                uint32 tclass:8;
                uint32 flowLabel:20;
            };
            uint32 word0;
        };
        uint16 payloadLen;
        uint8 nextHeader;
        uint8 hopLimit;
        fap4kePkt_ipv6Address_t ipSa;
        fap4kePkt_ipv6Address_t ipDa;
    };
    uint32 u32[10];
}__attribute__((packed)) fap4kePkt_packed_ipv6Hdr_t;

#define FAP4KE_PKT_IPV6_VERSION(_ipv6Header_p)   (((fap4kePkt_packed_ipv6Hdr_t *)_ipv6Header_p)->ver)
#define FAP4KE_PKT_IPV6_TCLASS(_ipv6Header_p)    (((fap4kePkt_packed_ipv6Hdr_t *)_ipv6Header_p)->tclass)
#define FAP4KE_PKT_IPV6_FLOWLABEL(_ipv6Header_p) (((fap4kePkt_packed_ipv6Hdr_t *)_ipv6Header_p)->flowLabel)

typedef struct {
    union {
        struct {
            uint8 next_hdr;
            uint8 hdr_len;
            uint16 u16;
        };
        uint32    word0;
    };
    uint32 word1;
} fap4kePkt_ipv6ExtHeader_t;
#endif /* defined(CC_FAP4KE_PKT_IPV6_SUPPORT) */

typedef union {
    fap4kePkt_ipv4Header_t v4;
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    fap4kePkt_ipv6Header_t v6;
#endif
} fap4kePkt_ipHeader_t;

#define FAP4KE_TCP_RST_SYN_FIN_MASK 0x07

typedef struct {
    uint16 sPort;
    uint16 dPort;
    uint32 seq;        /* word1 */
    uint32 ack_seq;    /* word2 */
    uint8 doff_res;
#define FAP4KE_PKT_DOFF_SHIFT  4
#define FAP4KE_PKT_DOFF_MASK   0XF
    uint8 flags;
#define FAP4KE_PKT_TCP_FLAGS_CWR (1 << 7)
#define FAP4KE_PKT_TCP_FLAGS_ECE (1 << 6)
#define FAP4KE_PKT_TCP_FLAGS_URG (1 << 5)
#define FAP4KE_PKT_TCP_FLAGS_ACK (1 << 4)
#define FAP4KE_PKT_TCP_FLAGS_PSH (1 << 3)
#define FAP4KE_PKT_TCP_FLAGS_RST (1 << 2)
#define FAP4KE_PKT_TCP_FLAGS_SYN (1 << 1)
#define FAP4KE_PKT_TCP_FLAGS_FIN (1 << 0)
    uint16 window;
    uint16 csum;
    uint16 urg_ptr;
} fap4kePkt_tcpHeader_t;

typedef struct {
    uint16 sPort;
    uint16 dPort;
    uint16 length;
    uint16 csum;
} fap4kePkt_udpHeader_t;

typedef union {
    struct {
        uint16 ip;
        uint16 tu;
    };
    uint32 u32;
} fap4kePkt_icsum_t;

typedef union {
    struct {
        uint16 inner;
        uint16 outer;
    };
    uint32 u32;
} fap4kePkt_vlanIdFilter_t;

typedef struct {
    union {
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        struct {
            fap4kePkt_ipv6Address_t ipSa6;
            fap4kePkt_ipv6Address_t ipDa6;
        };
#endif
        struct {
            uint32 ipSa4;
            uint32 ipDa4;
        };
    };
    union {
        /* Unicast */
        union {
            struct {
                uint16 sPort;  /* TCP/UDP source port */
                uint16 dPort;  /* TCP/UDP dest port */
            };
            uint32 l4Ports;
        };
        /* Multicast */
        fap4kePkt_vlanIdFilter_t vlanId;
    };
    struct {
        uint16 isRouted  : 1;
        uint16 isIPv6    : 1;
        uint16 mangleTos : 1;
        uint16 mangle_ip6Tos : 1;
        uint16 drop      : 1;
        uint16 learn     : 1;
        uint16 reserved  : 10;
    } flags;
    struct{
        uint8 tclass;
        uint8 resvd_byte;
    };	
    fap4kePkt_icsum_t icsum;
} fap4kePkt_ipTuple_t;

typedef union {
    struct {
        uint8 phy;
        union {
            uint8 channel;
            uint8 channelMask;
        };
        union {
            struct{
                uint8 reserved  : 4;
                uint8 nbrOfTags : 4; /* use for src Multicast flows only */
            };
            uint8 queue;     /* use for dest only */
        };
        union {
            uint8 protocol; /* use for src only */
        };
    };
    uint32 u32;
} fap4kePkt_key_t;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
typedef struct {
    uint32 prio;        /* Ingress QoS packet priority */
    uint32 dropped;     /* packets dropped by Ingress QoS */
} fap4kePkt_iq_t;
#endif

typedef struct {
    fap4kePkt_key_t source;
    fap4kePkt_key_t dest;
    fap4kePkt_ipTuple_t ipTuple;
    int32 txAdjust;
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    fap4kePkt_iq_t iq;
#endif
    uint16          fapMtu;
    uint8   tunnelType;
} fap4kePkt_flowInfo_t;

typedef struct {
    uint32 hits;
    uint32 bytes;
} fap4kePkt_flowStats_t;

typedef struct fap4kePkt_flow {
    struct fap4kePkt_flow *next;

    struct {
        union {
            struct {
                uint8 isActive  : 1;
                uint8 reserved  : 7;
            };
            uint8 u8;
        } flags;
        uint8 hashIx;
        uint16 flowId;
    };

    fap4kePkt_flowInfo_t info;

    fap4kePkt_flowStats_t stats;
} fap4kePkt_flow_t;

typedef uint16 fap4kePkt_flowId_t;

#define FAP4KE_PKT_LEARN_MAX_VLAN_HEADERS  2

typedef struct {
    uint32 stripLen;
    uint32 vlanHdrCount;
    fap4kePkt_vlanHeader_t *vlanHdr_p[FAP4KE_PKT_LEARN_MAX_VLAN_HEADERS];
} fap4kePkt_learnHeaders_t;

typedef struct {
    uint8 macDa[ETH_ALEN];
    uint8 macSa[ETH_ALEN];
    uint16 etherType;
    union {
        struct {
            uint16 reserved  : 15;
            uint16 insertEth : 1;
        };
        uint16 flags;
    };
} fap4kePkt_cmdArg_t;

typedef struct {
    fap4kePkt_flow_t *flow_p;
    fap4kePkt_cmdArg_t *cmdArg_p;
    uint8 *cmdList;
    int cmdListIx;
    int cmdOffset;
    uint32 vlanHdrCount;
} fap4kePkt_cmdHandlerArg_t;

typedef void (* fap4kePkt_cmdHandler_t)(fap4kePkt_cmdHandlerArg_t *cmdHandlerArg_p);

typedef enum {
    FAP4KE_PKT_CMD_DROP=0,
    FAP4KE_PKT_CMD_SET_MAC_DA,
    FAP4KE_PKT_CMD_INSERT_MAC_DA,
    FAP4KE_PKT_CMD_SET_MAC_SA,
    FAP4KE_PKT_CMD_INSERT_MAC_SA,
    FAP4KE_PKT_CMD_INSERT_ETHERTYPE,
    FAP4KE_PKT_CMD_POP_BRCM_TAG,
    FAP4KE_PKT_CMD_PUSH_BRCM2_TAG,
    FAP4KE_PKT_CMD_POP_VLAN_HDR_0,
    FAP4KE_PKT_CMD_POP_VLAN_HDR_1,
    FAP4KE_PKT_CMD_POP_PPPOE_HDR,
    FAP4KE_PKT_CMD_POP_PPPOA_HDR,
    FAP4KE_PKT_CMD_DECR_TTL,
    FAP4KE_PKT_CMD_MAX
} fap4kePkt_learnCmd_t;

typedef struct {
    uint8 cmdCount;
    uint8 cmd[FAP4KE_PKT_CMD_MAX]; /* fap4kePkt_learnCmd_t */
    fap4kePkt_cmdArg_t cmdArg;
} fap4kePkt_learnAction_t;

/*
 * Mapped to DDR
 */
typedef struct {
    fap4kePkt_learnAction_t action[FAP4KE_PKT_MAX_FLOWS];
} fap4kePkt_learn_t;

/*
 * Mapped to PSM
 */
typedef struct {
    uint8 cmdList[FAP4KE_PKT_CMD_LIST_SIZE];
#if defined(CC_FAP4KE_PKT_HW_ICSUM)
    uint8 checksum1[FAP4KE_PKT_CSUM_CMD_LIST_SIZE];
    uint8 checksum2[FAP4KE_PKT_CSUM_CMD_LIST_SIZE];
#endif
    fap4kePkt_flowStats_t stats;
} fap4kePkt_shared_t;

#define p4keFlowInfoPool  p4keSdram->alloc.packet.flowInfoPool
#define p4keHeaderPool    p4kePsmGbl->packet.headerPool
#if defined(CONFIG_BCM96362)
#define p4keHeaderPoolIop p4keHeaderPool
#elif defined(CONFIG_BCM963268)
#define p4keHeaderPoolIop p4kePsmIopGbl->packet.headerPool
#else
#error "Unknown FAP-based Chip"
#endif

/*
 * Mapped to DSPRAM
 */
typedef struct {
    uint32 flowCount;
    fap4kePkt_flow_t flowPool[FAP4KE_PKT_MAX_DSP_FLOWS];
    fap4kePkt_flow_t *hashTable[FAP4KE_PKT_HASH_TABLE_SIZE];
    fap4kePkt_cmdHandler_t cmdHandler[FAP4KE_PKT_CMD_MAX];
} fap4kePkt_runtime_t;

#define pktRuntime4ke p4keDspramGbl->packet.runtime

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#define IQINFO_FLAGS_MCAST      0x01
typedef union {
    uint32 u32;
    struct {
        uint8 flags;        /* Mcast */
        uint8 proto;        /* TCP/UDP */
        uint16 destPort;    /* Dest Port */
    } s;
} fap4kePkt_iqInfo_t;
#endif

typedef struct {
    uint8 *pBuf;
    union {
        struct {
            uint32 portId;
            uint32 dmaFlag;
        } rx;
        struct {
            fap4kePkt_lookup_t lookup;
            fap4kePkt_flow_t *flow_p;
            uint8 *packetTx_p;
        } tx;
    };
    uint16 length;
    uint16 ipLength;
    uint16 rxHdrLen;
    uint16 mssAdj;
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    fap4kePkt_iqInfo_t iqInfo;
#endif
    uint8 bNeedFragmentation;
} fap4kePkt_packetInfo_t;

void fap4kePktTest_runTests(void);

void fap4kePkt_init(void);
fapRet fap4kePkt_activate(fap4kePkt_flowId_t flowId);
fapRet fap4kePkt_deactivate(fap4kePkt_flowId_t flowId);
fapRet fap4kePkt_updateFlowInfo(fap4kePkt_flowId_t flowId);
fapRet fap4kePkt_printFlow(fap4kePkt_flowId_t flowId);
void fap4kePkt_learn(fap4kePkt_flow_t *flow_p, fap4kePkt_learnHeaders_t *headers_p);

#endif /* __FAP4KE_PACKET_H_INCLUDED__ */
