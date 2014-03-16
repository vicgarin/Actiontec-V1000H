#ifndef _CTL_MSG_H
#define _CTL_MSG_H
#include "ctl.h"
#include "tsl_common.h"

#define _STR(s)		#s
#define STR(s)		_STR(s)

#define CTL_MSG_PREFIX	"ctl.msg"
#define CTL_MSG_TYPE(id)		(CTL_MSG_PREFIX STR(id))


// for CTL_MSG_ACCESS_CFG
#define CTL_METHOD_SAVE 	"SAVE"
#define CTL_METHOD_RESTORE 	"RESTORE"
#define CTL_METHOD_RESTORE_DEFAULT 	"RESTORE_DEFAULT"

typedef enum 
{
   CTL_MSG_ACCESS_CFG_METHODCALL  = 0x01, 	// Method call, Ask RTD to Save/Restore cfg.xml
   CTL_MSG_ACCESS_CFG_SIGNAL      = 0x02, 	// Signal msg,  Ask RTD to Save cfg.xml
   CTL_MSG_L2_STATE_CHANGED       = 0x03,       // DEBUG, simulator driver event

   //CTL_MSG_WAN_CONN_IPv4_ADDR = 0x10002408,
   CTL_MSG_WAN_CONN_IPV4_CHANGED = 0x04, 	// To support ip6ip4 tunnel, When IPv6 Changed in BRCM MDM, sync to CTL.
   CTL_MSG_WATCH_WAN_CONN_IPV4    = 0x05, 	// Request RTD to watch the ipv4 status
   CTL_MSG_WATCH_WAN_CONN_PPP	  = 0x06,	// Request RTD to watch the ppp status
   CTL_MSG_WAN_CONN_PHYSICAL_CHANGED = 0x07, //Physical Wan is Ethernet or xDSL
   CTL_MSG_WAN_CONN_INTERFACE_CHANGED = 0x08, //Wan Interface is ppp0 or ppp0.1 or others

   CTL_MSG_SYSTEM_BOOT    = 0x10000250, /**< system has booted, delivered to apps
                                         *   EIF_LAUNCH_ON_STARTUP set in their
                                         *   CmsEntityInfo.flags structure.
                                         */
   CTL_MSG_APP_LAUNCHED   = 0x10000251, /**< Used by apps to confirm that launch succeeded.
                                         *   Sent from app to smd in cmsMsg_init.
                                         */
   CTL_MSG_WAN_LINK_UP    = 0x10000252, /**< wan link is up (includes dsl, ethernet, etc) */
   CTL_MSG_WAN_LINK_DOWN  = 0x10000253, /**< wan link is down */
   CTL_MSG_WAN_CONNECTION_UP   = 0x10000254, /**< WAN connection is up (got IP address) */
   CTL_MSG_WAN_CONNECTION_DOWN = 0x10000255, /**< WAN connection is down (lost IP address) */
   CTL_MSG_ETH_LINK_UP    = 0x10000256, /**< eth link is up (only if eth is used as LAN interface) */
   CTL_MSG_ETH_LINK_DOWN  = 0x10000257, /**< eth link is down (only if eth is used as LAN interface) */
//   CTL_MSG_USB_LINK_UP    = 0x10000258, /**< usb link is up (only if eth is used as LAN interface) */
//   CTL_MSG_USB_LINK_DOWN  = 0x10000259, /**< usb link is down (only if eth is used as LAN interface) */
//   CTL_MSG_ACS_CONFIG_CHANGED = 0x1000025A, /**< ACS configuration has changed. */
//   CTL_MSG_DELAYED_MSG    = 0x1000025B, /**< This message is delivered to when delayed msg timer expires. */
//   CTL_MSG_TR69_ACTIVE_NOTIFICATION = 0x1000025C,
						/**< This message is sent to tr69c when one or more
                                                   *   parameters with active notification attribute
                                                   *   has had their value changed.
                                                   */
//   CTL_MSG_SHMID                     = 0x10000262, /**< Sent from ssk to smd when shmid is obtained. */
   CTL_MSG_MDM_INITIALIZED           = 0x10000263, /**< Sent from ssk to smd when MDM has been initialized. */
   CTL_MSG_DHCPC_STATE_CHANGED       = 0x10000264, /**< Sent from dhcp client when state changes, see also DhcpcStateChangeMsgBody */
   CTL_MSG_PPPOE_STATE_CHANGED       = 0x10000265, /**< Sent from pppoe when state changes, see also PppoeStateChangeMsgBody */
   CTL_MSG_DHCP6C_STATE_CHANGED      = 0x10000266, /**< Sent from dhcpv6 client when state changes, see also Dhcp6cStateChangeMsgBody */
//   CTL_MSG_PING_STATE_CHANGED        = 0x10000267, /**< Ping state changed (completed, or stopped) */
//   CTL_MSG_DHCPD_RELOAD		     = 0x10000268, /**< Sent to dhcpd to force it reload config file without restart */
//   CTL_MSG_DHCPD_DENY_VENDOR_ID	    = 0x10000269, /**< Sent from dhcpd to notify a denied request with some vendor ID */
//   CTL_MSG_DHCPD_HOST_INFO           = 0x1000026A, /**< Sent from dhcpd to ssk to inform of lan host add/delete */
   CTL_MSG_DNSPROXY_RELOAD	     = 0x10000270, /**< Sent to dnsproxy to force it reload config file without restart */
//   CTL_MSG_SNTP_STATE_CHANGED 	     = 0x10000271, /**< SNTP state changed */
   CTL_MSG_MCPD_RELOAD	             = 0x10000276, /**< Sent to mcpd to force it reload config file without restart */
   CTL_MSG_6RD_NUD_INFO             = 0x10000278,
//   CTL_MSG_CONFIG_WRITTEN             = 0x10000280, /**< Event sent when a config file is written. */

   CTL_MSG_SET_PPP_UP                 = 0x10000290, /* Sent to ppp when set ppp up manually */
   CTL_MSG_SET_PPP_DOWN               = 0x10000291, /* Sent to ppp when set ppp down manually */  

   CTL_MSG_DNSPROXY_DUMP_STATUS       = 0x100002A1, /* Tell dnsproxy to dump its current status */
   CTL_MSG_DNSPROXY_DUMP_STATS        = 0x100002A2, /* Tell dnsproxy to dump its statistics */
   CTL_MSG_L3INTERFACE_START	 	  = 0x100002A3, /* rutWan_stopL3Interface */
   CTL_MSG_L3INTERFACE_STOP			  = 0x100002A4, /* rutWan_startL3Interface */

//#ifdef BRCM_WLAN
//   CTL_MSG_WLAN_CHANGED          		     = 0x10000300,  /**< wlmngr jhc*/
//#endif
//   CTL_MSG_SNMPD_CONFIG_CHANGED = 0x1000301, /**< ACS configuration has changed. */
//   CTL_MSG_MANAGEABLE_DEVICE_NOTIFICATION_LIMIT_CHANGED = 0x1000302, /**< Notification Limit of number of management device. */

//   CTL_MSG_STORAGE_ADD_PHYSICAL_MEDIUM = 0x1000310,
//   CTL_MSG_STORAGE_REMOVE_PHYSICAL_MEDIUM = 0x1000311,
//   CTL_MSG_STORAGE_ADD_LOGICAL_VOLUME = 0x1000312,
//   CTL_MSG_STORAGE_REMOVE_LOGICAL_VOLUME = 0x1000313,

//   CTL_MSG_REGISTER_DELAYED_MSG      = 0x10000800, /**< request a message sometime in the future. */
//   CTL_MSG_UNREGISTER_DELAYED_MSG    = 0x10000801, /**< cancel future message delivery. */
//   CTL_MSG_REGISTER_EVENT_INTEREST   = 0x10000802, /**< request receipt of the specified event msg. */
//   CTL_MSG_UNREGISTER_EVENT_INTEREST = 0x10000803, /**< cancel receipt of the specified event msg. */
   CTL_MSG_DIAG                    = 0x10000805, /**< request diagnostic to be run */
//   CTL_MSG_TR69_GETRPCMETHODS_DIAG = 0x10000806, /**< request tr69c send out a GetRpcMethods */
   CTL_MSG_DSL_LOOP_DIAG_COMPLETE  = 0x10000807, /**< dsl loop diagnostic completes */

   CTL_MSG_START_APP         = 0x10000807, /**< request smd to start an app; pid is returned in the wordData */
   CTL_MSG_RESTART_APP       = 0x10000809, /**< request smd to stop and then start an app; pid is returned in the wordData */
   CTL_MSG_STOP_APP          = 0x1000080A, /**< request smd to stop an app */
   CTL_MSG_IS_APP_RUNNING    = 0x1000080B, /**< request to check if the the application is running or not */

//   CTL_MSG_REBOOT_SYSTEM     = 0x10000850,  /**< request smd to reboot, a response means reboot sequence has started. */

   CTL_MSG_SET_LOG_LEVEL       = 0x10000860,  /**< request app to set its log level. */
   CTL_MSG_SET_LOG_DESTINATION = 0x10000861,  /**< request app to set its log destination. */

//   CTL_MSG_MEM_DUMP_STATS      = 0x1000086A,  /**< request app to dump its memstats */
//   CTL_MSG_MEM_DUMP_TRACEALL   = 0x1000086B,  /**< request app to dump all of its mem leak traces */
//   CTL_MSG_MEM_DUMP_TRACE50    = 0x1000086C,  /**< request app to its last 50 mem leak traces */
//   CTL_MSG_MEM_DUMP_TRACECLONES= 0x1000086D,  /**< request app to dump mem leak traces with clones */

//   CTL_MSG_LOAD_IMAGE_STARTING = 0x10000870,  /**< notify smd that image network loading is starting. */
//   CTL_MSG_LOAD_IMAGE_DONE     = 0x10000871,  /**< notify smd that image network loading is done. */
   CTL_MSG_GET_CONFIG_FILE     = 0x10000872,  /**< ask smd for a copy of the config file. */
   CTL_MSG_VALIDATE_CONFIG_FILE= 0x10000873,  /**< ask smd to validate the given config file. */
   CTL_MSG_WRITE_CONFIG_FILE   = 0x10000874,  /**< ask smd to write the config file. */
   CTL_MSG_VENDOR_CONFIG_UPDATE = 0x10000875,  /**<  the config file. */

   CTL_MSG_GET_WAN_LINK_STATUS = 0x10000880,  /**< request current WAN LINK status. */
   CTL_MSG_GET_WAN_CONN_STATUS = 0x10000881,  /**< request current WAN Connection status. */
   CTL_MSG_GET_LAN_LINK_STATUS = 0x10000882,  /**< request current LAN LINK status. */
#ifdef SUPPORT_DYNA_PHY_AUTO_SWITCH
    CTL_MSG_WATCH_WAN_PHY_TYPE= 0x10000895,
#endif

   CTL_MSG_WATCH_WAN_CONNECTION= 0x10000890,  /**< request ssk to watch the dsl link status and then change the connectionStatus for bridge, static MER and ipoa */
   CTL_MSG_WATCH_DSL_LOOP_DIAG = 0x10000891,  /**< request ssk to watch the dsl loop diag and then update the stats */

//   CTL_MSG_GET_LEASE_TIME_REMAINING = 0x100008A0,  /**< ask dhcpd how much time remains on lease for particular LAN host */
   CTL_MSG_GET_DEVICE_INFO          = 0x100008A1,  /**< request system/device's info */
   CTL_MSG_REQUEST_FOR_PPP_CHANGE   = 0x100008A2,  /**< request for disconnect/connect ppp  */

#ifdef AEI_SUPPORT_IPV6_STATICROUTE
   CTL_MSG_IPV6_STATIC_ROUTE  = 0x100008D1,   
#endif

#if AEI_SUPPORT_PHYTYPE_IFNAME_CHANGE
   CTL_MSG_IPV6_PHYTYPE_IFNAME = 0x100008D2,

#endif   
   CTL_MSG_DATA_CENTER_READY		  = 0x100008D3, /* data_center has initialized completely */


#if 0
   CTL_MSG_MOCA_WRITE_LOF           = 0x100008B1, /**< mocad reporting last operational frequency */
   CTL_MSG_MOCA_READ_LOF            = 0x100008B2, /**< mocad reporting last operational frequency */
   CTL_MSG_MOCA_WRITE_MRNONDEFSEQNUM= 0x100008B3, /**< mocad reporting moca reset non-def sequence number */
   CTL_MSG_MOCA_READ_MRNONDEFSEQNUM = 0x100008B4, /**< mocad reporting moca reset non-def sequence number */
   CTL_MSG_MOCA_NOTIFICATION        = 0x100008B5, /**< application reporting that it has updated moca parameters */

   CTL_MSG_QOS_DHCP_OPT60_COMMAND   = 0x100008C0, /**< QoS Vendor Class ID classification command */
   CTL_MSG_QOS_DHCP_OPT77_COMMAND   = 0x100008C1, /**< QoS User   Class ID classification command */
   
   CTL_MSG_VOICE_CONFIG_CHANGED= 0x10002000,  /**< Voice Configuration parameter changed private event msg. */
   CTL_MSG_VODSL_BOUNDIFNAME_CHANGED = 0x10002001, /**< vodsl BoundIfName param has changed. */
   CTL_MSG_SHUTDOWN_VODSL= 0x10002002,  /**< Voice shutdown request. */
   CTL_MSG_START_VODSL                 = 0x10002003, /**< Voice start request. */
   CTL_MSG_REBOOT_VODSL                = 0x10002004, /**< Voice reboot request. This is for the voice reboot command */
   CTL_MSG_RESTART_VODSL               = 0x10002005, /**< Voice re-start request. This is to restart the call manager when the IP address changes*/
   CTL_MSG_INIT_VODSL                  = 0x10002006, /**< Voice init request. */
   CTL_MSG_DEINIT_VODSL                = 0x10002007, /**< Voice init request. */
   CTL_MSG_RESTART_VODSL_CALLMGR       = 0x10002008, /**< Voice call manager re-start request. */
#ifdef DMP_X_BROADCOM_COM_NTR_1
   CTL_MSG_VOICE_NTR_CONFIG_CHANGED    = 0x10002009, /**< Voice NTR Configuration parameter changed private event msg. */
#endif /* DMP_X_BROADCOM_COM_NTR_1 */
	
   CTL_MSG_VOICE_DIAG          = 0x10002100,  /**< request voice diagnostic to be run */
   CTL_MSG_VOICE_STATISTICS_REQUEST    = 0x10002101, /**< request for Voice call statistics */
   CTL_MSG_VOICE_STATISTICS_RESPONSE   = 0x10002102, /**< response for Voice call statistics */
   CTL_MSG_VOICE_STATISTICS_RESET      = 0x10002103, /**< request to reset Voice call statistics */
   CTL_MSG_VOICE_CM_ENDPT_STATUS         = 0x10002104, /**< request for Voice Line obj */
   CTL_MSG_VODSL_IS_READY_FOR_DEINIT   = 0x10002105, /**< query if the voice app is ready to deinit */
#ifdef DMP_X_BROADCOM_COM_DECTENDPOINT_1
   CTL_MSG_VOICE_DECT_OPEN_REG_WND     = 0x100021F0, /**< request for opening DECT registration window */
   CTL_MSG_VOICE_DECT_CLOSE_REG_WND    = 0x100021F1, /**< request for closing DECT registration window */
   CTL_MSG_VOICE_DECT_INFO_REQ         = 0x100021F2, /**< request for Voice DECT status information */
   CTL_MSG_VOICE_DECT_INFO_RSP         = 0x100021F3, /**< response for Voice DECT status information */
   CTL_MSG_VOICE_DECT_AC_SET           = 0x100021F4, /**< request for Voice DECT Access Code Set */
   CTL_MSG_VOICE_DECT_HS_INFO_REQ      = 0x100021F5, /**< request for Voice DECT handset status information */
   CTL_MSG_VOICE_DECT_HS_INFO_RSP      = 0x100021F6, /**< response for Voice DECT handset status information */
   CTL_MSG_VOICE_DECT_HS_DELETE        = 0x100021F7, /**< request for deleting a handset from DECT module */
   CTL_MSG_VOICE_DECT_HS_PING          = 0x100021F8, /**< request for pinging a handset from DECT module */
   CTL_MSG_VOICE_DECT_NUM_ENDPT        = 0x100021F9, /**< request for number of DECT endpoints */
#endif /* DMP_X_BROADCOM_COM_DECTENDPOINT_1 */

   CTL_MSG_GET_GPON_OMCI_STATS = 0x10002200, /**< request GPON OMCI statistics */
   CTL_MSG_OMCI_COMMAND_REQUEST = 0x10002201, /**< GPON OMCI command message request */
   CTL_MSG_OMCI_COMMAND_RESPONSE = 0x10002202, /**< GPON OMCI command message response */
   CTL_MSG_OMCI_DEBUG_GET_REQUEST = 0x10002203, /**< GPON OMCI debug get message request */
   CTL_MSG_OMCI_DEBUG_GET_RESPONSE = 0x10002204, /**< GPON OMCI debug get message response */
   CTL_MSG_OMCI_DEBUG_SET_REQUEST = 0x10002205, /**< GPON OMCI debug set message request */
   CTL_MSG_OMCI_DEBUG_SET_RESPONSE = 0x10002206, /**< GPON OMCI debug set message response */

   CTL_MSG_CMF_SEND_REQUEST = 0x10002301, /**< CMF File Send message request */
   CTL_MSG_CMF_SEND_RESPONSE = 0x10002302, /**< CMF File Send message response */
#endif
#if defined(AEI_VDSL_CUSTOMER_NCS)
//	CTL_MSG_SET_PortMapping_LeaseDuration = 0x10002309,
//	CTL_MSG_GET_PortMapping_Remaining_Time = 0x10002310,
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
//	CTL_MSG_WLAN_RESTORE_DEFAULT = 0x10002312,  /**< wlmngr Restore Default*/
#endif

#if 0
#ifdef DMP_DOWNLOAD_1
   CTL_MSG_TR143_DLD_DIAG_COMPLETED = 0x10002400,
#endif
#ifdef DMP_UPLOAD_1
   CTL_MSG_TR143_UPLD_DIAG_COMPLETED = 0x10002401,
#endif
#ifdef DMP_UDPECHO_1
   CTL_MSG_TR143_ECHO_DIAG_RESULT = 0x10002402,
#endif

#if defined(DMP_UPNPDISCBASIC_1) && defined (DMP_UPNPDISCADV_1)
   CTL_MSG_UPNP_DISCOVERY_INFO = 0x10002403,   /*Send upnp device and service info*/
#endif
#ifdef DMP_PERIODICSTATSBASE_1
   CTL_MSG_PERIODIC_STATISTICS_TIMER = 0x10002404,
#endif
#endif

#if defined (AEI_VDSL_CUSTOMER_BELLALIANT) || defined(AEI_VDSL_CUSTOMER_BELLCANADA)
//   CTL_MSG_TR69C_ENABLE_CWMP_FLAG = 0x10002405,
#endif
#if defined(DMP_TRACEROUTE_1)
//	CTL_MSG_TRACE_ROUTE_STATE_CHANGE = 0x10002406,
#endif

#ifdef AEI_SUPPORT_6RD
    CTL_MSG_WATCH_6RD_OPTION = 0x10002409,
#endif
#ifdef AEI_CONTROL_LAYER_DAD
    CTL_MSG_WATCH_DAD_RESULT = 0x10002410,
#endif
#ifdef DMP_SIMPLEFIREWALL_1
//   CTL_MSG_TR69_FIREWALL_LEVEL_CHANGED = 0x10002500, /* tr69c -> httpd */
#endif


#ifdef AEI_VDSL_CUSTOMER_NCS
   CTL_MSG_GET_LANHOSTS_RENEW = 0x10002407  /**< ask mynetwork to renew the lanhost table */
#endif

} CtlMsgType;


typedef struct ctl_msg_header
{
    tsl_u32_t data_length; //this is the length of buffer
    tsl_u8_t buffer[0];     //the buffer head address. 
}CtlMsgHeader;

/** Data body for CTL_MSG_DHCP6C_STATE_CHANGED message type.
 *
 */
typedef struct
{
   tsl_bool prefixAssigned;  /**< Have we been assigned a site prefix ? */
   tsl_bool addrAssigned;    /**< Have we been assigned an IPv6 address ? */
   tsl_bool dnsAssigned;     /**< Have we been assigned dns server addresses ? */
   tsl_8_t sitePrefix[BUFLEN_48];   /**< New site prefix, if prefixAssigned==TRUE */
   tsl_u32_t prefixPltime;
   tsl_u32_t prefixVltime;
   tsl_u32_t prefixCmd;
   tsl_8_t ifname[BUFLEN_32];
   tsl_8_t address[BUFLEN_48];      /**< New IPv6 address, if addrAssigned==TRUE */
   tsl_8_t pdIfAddress[BUFLEN_48];      /**< New IPv6 address of PD interface */
   tsl_u32_t addrCmd;
   tsl_8_t nameserver[BUFLEN_128];  /**< New nameserver, if addressAssigned==TRUE */
   tsl_bool defaultGWAssigned; /** if default gateway assigned */
   tsl_8_t defaultGW[BUFLEN_48]; /** default gateway if defaultGWAssigned it true */
   tsl_u32_t t1;					/*Timeout for renew*/
   tsl_u32_t t2;					/*Timeout for rebind*/
   tsl_8_t namesearchlist[BUFLEN_128];  /**dns search list */
} CtlDhcp6cStateChangedMsgBody;

/*!\PPPOE state defines
 * (was in syscall.h before)
 */

#define CTL_PPPOE_CLIENT_STATE_PADO          0   /* waiting for PADO */
#define CTL_PPPOE_CLIENT_STATE_PADS          1   /* got PADO, waiting for PADS */
#define CTL_PPPOE_CLIENT_STATE_CONFIRMED     2   /* got PADS, session ID confirmed */
#define CTL_PPPOE_CLIENT_STATE_DOWN          3   /* totally down */
#define CTL_PPPOE_CLIENT_STATE_UP            4   /* totally up */
#define CTL_PPPOE_SERVICE_AVAILABLE          5   /* ppp service is available on the remote */
#define CTL_PPPOE_AUTH_FAILED                7
#define CTL_PPPOE_RETRY_AUTH                 8
#define CTL_PPPOE_REPORT_LASTCONNECTERROR    9
#define CTL_PPPOE_CLIENT_STATE_UNCONFIGURED   10
#define CTL_PPPOE_CLIENT_IPV6_STATE_UP   11
#define CTL_PPPOE_CLIENT_IPV6_STATE_DOWN   12

/** Data body for CTL_MSG_DHCPC_STATE_CHANGED message type.
 *
 */
typedef struct
{
   
   tsl_bool addressAssigned; /**< Have we been assigned an IP address ? */
   tsl_char_t ip[BUFLEN_32];   /**< New IP address, if addressAssigned==TRUE */
   tsl_char_t mask[BUFLEN_32]; /**< New netmask, if addressAssigned==TRUE */
   tsl_char_t gateway[BUFLEN_32];    /**< New gateway, if addressAssigned==TRUE */
   tsl_char_t nameserver[BUFLEN_64]; /**< New nameserver, if addressAssigned==TRUE */
   tsl_char_t wandev[BUFLEN_64];/*to support multi-wan*/
#ifdef AEI_SUPPORT_6RD
   // 6rd
   tsl_bool   b6rdAssigned; // Have we been assigned 6rd prefix
   tsl_u32_t  ipv6rdipv4masklen;
   tsl_char_t ipv6rdbripv4addr[BUFLEN_32];
   tsl_u32_t  ipv6rdprefixlen;
   tsl_char_t ipv6rdprefix[BUFLEN_64];
#endif
} CtlDhcpcStateChangedMsgBody;

typedef struct
{
   tsl_bool logState;      
   tsl_u32_t logLevel;  
   tsl_char_t moduleName[BUFLEN_32];
} CtlLogLevelChangeMsgBody;


typedef struct
{
   tsl_u8_t pppState;       /**< pppoe states */
   tsl_char_t ip[BUFLEN_32];   /**< New IP address, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   tsl_char_t mask[BUFLEN_32]; /**< New netmask, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   tsl_char_t gateway[BUFLEN_32];    /**< New gateway, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   tsl_char_t nameserver[BUFLEN_64]; /**< New nameserver, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   tsl_char_t servicename[BUFLEN_256]; /**< service name, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   tsl_char_t ppplastconnecterror[PPP_CONNECT_ERROR_REASON_LEN];
} CtlPppoeStateChangeMsgBody;

typedef struct
{
	tsl_char_t ConnectionStatus[BUFLEN_32];
	tsl_char_t ConnectionType[BUFLEN_32];
	tsl_char_t AddressingType[BUFLEN_32];
	tsl_char_t ExternalIPAddress[BUFLEN_32];
	//tsl_char_t SubnetMask[BUFLEN_32];
	//tsl_char_t DefaultGateway[BUFLEN_32];
	tsl_u32_t MaxMTUSize;
	//MACAddress;
	tsl_char_t X_BROADCOM_COM_IfName[BUFLEN_32];
} CtlWANConnIPv4StateChangedMsgBody;

typedef struct
{
	tsl_char_t Result[BUFLEN_32];
	tsl_char_t RTT[BUFLEN_32];
} Ctl6RDNUDMsgBody;

typedef struct
{
	tsl_char_t PhysicalWANType[BUFLEN_32];
	tsl_char_t AddressingType[BUFLEN_32];
}CtlPhysicalWANChangedMsgBody;

typedef struct
{
	tsl_bool isIpObj;
	tsl_char_t IfName[BUFLEN_32];
}CtlWANIfNameChangedMsgBody;

#ifdef AEI_SUPPORT_IPV6_STATICROUTE
typedef struct
{
    tsl_bool        isAdd;
    tsl_u8_t        enable;   /**< Enable */
    tsl_char_t      status[BUFLEN_32];   /**< Status */
    tsl_u8_t        fromIPVersion;  /**< Version */
    tsl_u8_t        staticRoute;  /**< StaticRoute */
    tsl_char_t      type[BUFLEN_32]; /**< Type */
    tsl_char_t      X_BROADCOM_COM_PolicyRoutingName[BUFLEN_32]; /**< X_BROADCOM_COM_PolicyRoutingName */
    tsl_char_t      destIPAddress[BUFLEN_256];    /**< DestIPAddress */
    tsl_char_t      destSubnetMask[BUFLEN_256];   /**< DestSubnetMask */
    tsl_char_t      sourceIPAddress[BUFLEN_256];  /**< SourceIPAddress */
    tsl_char_t      sourceSubnetMask[BUFLEN_256]; /**< SourceSubnetMask */
    tsl_char_t      X_BROADCOM_COM_SourceIfName[BUFLEN_32];  /**< X_BROADCOM_COM_SourceIfName */
#ifdef DMP_QOS_1
    tsl_u32_t       forwardingPolicy; /**< ForwardingPolicy */
#endif
    tsl_char_t      gatewayIPAddress[BUFLEN_256]; /**< GatewayIPAddress */
    tsl_char_t      interface[BUFLEN_32];    /**< Interface */
    tsl_u32_t       forwardingMetric; /**< ForwardingMetric */
    tsl_u32_t       MTU;  /**< MTU */
    tsl_u8_t        X_BROADCOM_COM_VlanRouteHide; /**< X_BROADCOM_COM_VlanRouteHide */
}CtlStaticRouteMsgBody;

/** interface msg for clear static route from rtd to ipv4.
 *
 */
typedef struct {
    tsl_bool        isAdd; 
    tsl_char_t      interface[BUFLEN_32];    /**< Interface */    
    tsl_char_t      destIPAddress[BUFLEN_256];    /**< DestIPAddress */
    tsl_char_t      destSubnetMask[BUFLEN_256];   /**< DestSubnetMask */
    tsl_char_t      gatewayIPAddress[BUFLEN_256]; /**< GatewayIPAddress */    
    tsl_u32_t       forwardingPolicy; /**< ForwardingPolicy */
} InforForStaticRoute;
#endif

#if AEI_SUPPORT_PHYTYPE_IFNAME_CHANGE
typedef struct {
    tsl_char_t      WANPhyType[BUFLEN_32];
    tsl_char_t      IfName[BUFLEN_32];  
    tsl_int_t       isIP;
} CtlPhyTypeIfNameMsgBody;
#endif

typedef struct
{
	tsl_char_t ConnectionStatus[BUFLEN_32];
	tsl_char_t ConnectionType[BUFLEN_32];
	tsl_char_t AddressingType[BUFLEN_32];
	//tsl_char_t ExternalIPAddress[BUFLEN_32];
	//tsl_char_t SubnetMask[BUFLEN_32];
	//tsl_char_t DefaultGateway[BUFLEN_32];
	tsl_u32_t MaxMTUSize;
	//MACAddress;
	tsl_char_t X_BROADCOM_COM_IfName[BUFLEN_32];
} CtlWANConnPppStateChangedMsgBody;

#endif //_CTL_MSG_H
