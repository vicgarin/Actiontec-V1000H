
#ifndef __AEI_CMS_MSG_H__
#define __AEI_CMS_MSG_H__

#ifdef DMP_DOWNLOAD_1
/** Data body for CMS_MSG_TR143_DLD_DIAG_COMPLETED message type.
 *
 */
 #define dummyVLANID  -178

typedef struct
{
   CmsEntityId requesterId;
   char diagnosticsState[BUFLEN_32];
   char romTime[BUFLEN_32];
   char bomTime[BUFLEN_32];
   char eomTime[BUFLEN_32];
   UINT32 testBytesReceived;
   UINT32 totalBytesReceived;
   char tcpOpenRequestTime[BUFLEN_32];
   char tcpOpenResponseTime[BUFLEN_32];
#ifdef AEI_VDSL_CUSTOMER_QWEST
   UINT32 X_ACTIONTEC_COM_PeriodOfFullLoading;
   UINT32 X_ACTIONTEC_COM_TotalBytesReceivedUnderFullLoading;
   UINT32 X_ACTIONTEC_COM_Throughput;
#endif
#ifdef AEI_CONTROL_TR143_IPV6
   UINT32 X_ACTIONTEC_COM_IPVersion;
#endif
} DldDiagMsgBody;

typedef struct
{
   char romTime[BUFLEN_32];
   char bomTime[BUFLEN_32];
   char eomTime[BUFLEN_32];
   UINT32 testBytesReceived;
   char tcpOpenRequestTime[BUFLEN_32];
   char tcpOpenResponseTime[BUFLEN_32];
} DldDiagThreadMsgBody;
#endif //DMP_DOWNLOAD_1

#ifdef DMP_UPLOAD_1
/** Data body for CMS_MSG_TR143_UPLD_DIAG_COMPLETED message type.
 *
 */
typedef struct
{
   CmsEntityId requesterId;
   char diagnosticsState[BUFLEN_32];
   char romTime[BUFLEN_32];
   char bomTime[BUFLEN_32];
   char eomTime[BUFLEN_32];
   UINT32 totalBytesSent;
   char tcpOpenRequestTime[BUFLEN_32];
   char tcpOpenResponseTime[BUFLEN_32];
#ifdef AEI_VDSL_CUSTOMER_QWEST
   UINT32 X_ACTIONTEC_COM_PeriodOfFullLoading;
   UINT32 X_ACTIONTEC_COM_TotalBytesSentUnderFullLoading;
   UINT32 X_ACTIONTEC_COM_Throughput;
#endif
#ifdef AEI_CONTROL_TR143_IPV6
   UINT32 X_ACTIONTEC_COM_IPVersion;
#endif
} UpldDiagMsgBody;

typedef struct
{
   char romTime[BUFLEN_32];
   char bomTime[BUFLEN_32];
   char eomTime[BUFLEN_32];
   UINT32 totalBytesSent;
   char tcpOpenRequestTime[BUFLEN_32];
   char tcpOpenResponseTime[BUFLEN_32];
} UpldDiagThreadMsgBody;
#endif // DMP_UPLOAD_1

/** Data body for CMS_MSG_TRACEROUTE_DATA message type.
 *  *
 *   */
#if defined(DMP_TRACEROUTE_1)
typedef enum
{
        None = 0,
        Requested = 1,
        Complete = 2,
        Error_CannotResolveHostName = 3,
        Error_MaxHopCountExceeded = 4,
}TraceRouteResult;

typedef struct
{
        char hopHost[BUFLEN_256];
        char hopHostAddress[BUFLEN_32];
        UINT32 hopErrorCode;
        char hopRTTimes[BUFLEN_16];
}Route_Hops_t;

typedef struct
{
        UINT32 responseTime;
        UINT32 routeHopsNumberOfEntries;
        CmsEntityId requesterId;
        TraceRouteResult result;
        Route_Hops_t routeHops[64];
}TraceRouteDataMsgBody;
#endif

#ifdef DMP_UDPECHO_1
/** Data body for CMS_MSG_TR143_ECHO_DIAG_RESULT message type.
 *
 */
typedef struct
{
   UINT32 packetsReceived;
   UINT32 packetsResponded;
   UINT32 bytesReceived;
   UINT32 bytesResponded;
   char timeFirstPacketReceived[BUFLEN_32];
   char timeLastPacketReceived[BUFLEN_32];
} EchoDiagMsgBody;
#endif

#ifdef DMP_USER_1
typedef enum
{
  USER_ENABLE_CHANGED = 0,
  USER_NAME_CHANGED,
  USER_PASSWORD_CHANGED,
  USER_REMOTE_ENABLE_CHANGED, 
  USER_LOCAL_ENABLE_CHANGED 
} UserChangedType;

/** Data body for the CMS_MSG_USER_AUTH_CHANGED message type.
 *
 */
typedef struct
{
   UserChangedType type;
   char username[BUFLEN_128];
} UserAuthMsgBody;
#endif



#if defined(CHIP_6368) || defined(CHIP_6362) || defined(CHIP_6328)
#define MAX_ATM_TRANSMIT_QUEUES  16
#else
#define MAX_ATM_TRANSMIT_QUEUES  8
#endif
#define MAX_PTM_TRANSMIT_QUEUES  8
#define BRIDGE_2IP_INF_STR          "br0:private"
#define BYPASS_STR                  "RMAT"

#define TEMPWANAPPLY "/var/applywanipsetting"

#define L2DETECTRUNNING "/var/L2DETECTRUNNING"
#define L2DETECTFAILURE "var/L2DETECTFAILURE"

#if defined(AEI_VDSL_DETECT_WAN_SERVICE) || defined(AEI_VDSL_TELUS_DETECT_WAN_SERVICE)
#define INTERFACE_LEN 10
/* Store detected WAN procotol  PPPoE or DHCP */
#define FDETECTSERVICE "/var/detectService" 
/* Store default wan interface */
#define FWANINTERFACE   "/var/baseL3ifName"
/* Store detected VLAN ID for PTM */
#define FDETECTL2INFO "/var/detectl2info"
#define FPPPINTERFACE   "/var/PPPifName"
#define FPIDOFPVCVLAN   "/var/pidofPVCVLAN"
#define FWANPVCLISTINFO "/var/wanPvcList"

/* */
#define MODE_ALL_OFF  0

#define MODE_L2_ADSL_PTM 1
#define MODE_L2_VDSL 1<<1
#define MODE_L3 1<<2
#define MODE_L2_ETH 1<<3
#define MODE_L2_ADSL_ATM 1<<4
#define MODE_L2_PTM_ATM_ETH (MODE_L2_ADSL_PTM|MODE_L2_ADSL_ATM|MODE_L2_VDSL|MODE_L2_ETH)

#define MODE_L2_VLAN (MODE_L2_ADSL_PTM|MODE_L2_VDSL|MODE_L2_ETH)
#define MODE_L2_PVC MODE_L2_ADSL_ATM

#define MODE_ALL_ON 0xFF // 11111111
#define DEFAULVLANALIAS 31
#define L3IFNAME  "ptm0.31"
#define ETH_L3IFNAME  "ewan0.31"
#define PTML2INTF  "ptm0"
#define EWANL2INTF  "ewan0"

typedef struct 
{
	SINT32 wanServiceType;
	char wanInterface[INTERFACE_LEN];
	SINT32 mode;
	SINT32  vlanId;
       SINT32 vci;
       SINT32 vpi;
}detectWANServiceBody;
typedef enum detectResult
{
    D_NO_SERVER = 0,
    D_DHCP_SERVER,
    D_PPPoE_SERVER,
    D_STATIC_TO_DHCP,
    D_PVC_OAMPING,
}detectResultType;


#endif /*AEI_VDSL_DETECT_WAN_SERVICE || AEI_VDSL_TELUS_DETECT_WAN_SERVICE*/


typedef enum
{
    X_ACTIONTEC_COM_PPPAUTOCONNECT = 0,
    X_ACTIONTEC_COM_PPPAUTOCONNECTTR69,
    X_ACTIONTEC_COM_PPPMACTR69,
    X_ACTIONTEC_COM_IPMACTR69,
    X_ACTIONTEC_COM_DNSCFGTR69,    
}AEI_GLOBAL_parameterIndex;

#if defined(AEI_VDSL_SMARTLED)
typedef enum
{
    INET_LED_OFF = 0,
    INET_LED_RED,
    INET_LED_GREEN,
    INET_LED_AMBER,
} LedColor;

typedef enum
{
    INET_LED_NONE = 0,
    INET_LED_FLASH,
    INET_LED_BLINK,
    INET_LED_ALTER,
} LedAction;
    
typedef struct
{
    LedColor color;
    LedAction action;
} InetLedControlBody;

typedef enum {
    CMS_MSG_SET_INET_LED_NONE = 0,
    CMS_MSG_SET_INET_LED_RED,                      
    CMS_MSG_SET_INET_LED_GREEN,                      
    CMS_MSG_SET_INET_LED_AMBER,                      
    CMS_MSG_SET_INET_LED_OFF,                      
    CMS_MSG_SET_INET_LED_RED_FLASH,                      
    CMS_MSG_SET_INET_LED_GREEN_FLASH,                      
    CMS_MSG_SET_INET_LED_AMBER_FLASH,                      
    CMS_MSG_SET_INET_LED_RED_BLINK,                      
    CMS_MSG_SET_INET_LED_GREEN_BLINK,                      
    CMS_MSG_SET_INET_LED_AMBER_BLINK,
    CMS_MSG_SET_INET_LED_RED_GREEN_ALTERNATE, 
} InetLedCase;

#endif /* AEI_VDSL_SMARTLED */

#ifdef AEI_VDSL_HPNA
#ifdef AEI_VDSL_WT107
/** Data body for the CMS_MSG_HPNA_STATE_CHANGED message type.
 *
 */
typedef struct
{
   UBOOL8 enable;
   char status[BUFLEN_32];
   char macAddr[BUFLEN_32];
} LanHpnaIntfMsgBody;

typedef struct
{
   UINT32 nodeId;
   char macAddr[BUFLEN_32];
   UBOOL8 isMaster;
   UBOOL8 synced;
   char chipID[BUFLEN_16];
   char firmwareVersion[BUFLEN_64];
   char firmwareSignature[BUFLEN_64];
   char hardwareVersion[BUFLEN_32];
   char manufacturer[BUFLEN_64];
   char oui[BUFLEN_16];
   char productClass[BUFLEN_64];
   char serialNumber[BUFLEN_64];
   UINT32 mtu;
   UINT32 noiseMargin;
   UINT32 defNonLarqper;
} LanHpnaNodeMsgBody;

typedef struct
{
    UINT32 bytesSent;
    UINT32 bytesReceived;
    UINT32 packetsSent;
    UINT32 packetsReceived;
    UINT32 broadcastPacketsSent;
    UINT32 broadcastPacketsReceived;
    UINT32 multicastPacketsSent;
    UINT32 multicastPacketsReceived;
    UINT32 packetsCrcErrored;
    UINT32 packetsCrcErroredHost;
    UINT32 packetsShortErrored;
    UINT32 packetsShortHost;
    UINT32 rxPacketsDropped;
    UINT32 txPacketsDropped;
    UINT32 controlRequestLocal;
    UINT32 controlReplyLocal;
    UINT32 controlRequestRemote;
    UINT32 controlReplyRemote;
    UINT32 packetsSentWire;
    UINT32 broadcastPacketsSentWire;
    UINT32 multicastPacketsSentWire;
    UINT32 packetsInternalControl;
    UINT32 broadcastPacketsInternalControl;
    UINT32 packetsReceivedQueued;
    UINT32 packetsReceivedForwardUnknown;
    UINT32 nodeUtilization;
} SampleStats;

/** Data body for the CMS_MSG_HPNA_NODE_SAMPLE_COMPLETED message type.
 *
 */
typedef struct
{
    char sampleTime[BUFLEN_32];
    UINT32 nodeNum;
    UINT32 channelNum;
} LanHpnaSampleHeaderMsgBody;

typedef struct
{
    UBOOL8 autoSample;
    char macAddress[BUFLEN_32];
    SampleStats stats;
} LanHpnaNodeSampleMsgBody;

typedef struct
{
    char hpnaSrcMac[BUFLEN_32];
    char hpnaDestMac[BUFLEN_32];
    char hostSrcMac[BUFLEN_32];
    char hostDestMac[BUFLEN_32];
    char phyRate[BUFLEN_32];
    char baudRate[BUFLEN_32];
    char snr[BUFLEN_32];
    UINT32 packetsSent;
    UINT32 packetsRecv;
} LanHpnaChannelSampleMsgBody;
#endif /* AEI_VDSL_WT107 */

typedef struct
{
    char tx[BUFLEN_32 + 1];
    char rx[BUFLEN_32 + 1];
    char crc[BUFLEN_32 + 1];
    char drop[BUFLEN_32 + 1];
    char loss[BUFLEN_32 + 1];
    char per[BUFLEN_32 + 1];
}LanHpnaNetperResMsgBody;
    
#endif /* AEI_VDSL_HPNA */

#endif /* __AEI_CMS_MSG_H__ */
