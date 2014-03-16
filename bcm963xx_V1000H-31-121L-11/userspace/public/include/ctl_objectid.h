#ifndef _CTL_OBJECTID_H
#define _CTL_OBJECTID_H

// This file was referred to:
//      ctl_layer/tr69fw/include/tr69_st.h
//      brcm462/userspace/public/include/mdm_objectid.h


#define CTLOID_IGD 					"InternetGatewayDevice"
#define CTLOID_IGD_Q_MGMT			"InternetGatewayDevice.QueueManagement"
#define CTLOID_IGD_Q_MGMT_CLASS		"InternetGatewayDevice.QueueManagement.Classification"
#define CTLOID_IGD_AEI_FIREWALL		"InternetGatewayDevice.X_AEI_COM_Firewall"
#define CTLOID_IGD_DEVICE_INFO 		"InternetGatewayDevice.DeviceInfo" 
#define CTLOID_VENDOR_CONFIG_FILE	"InternetGatewayDevice.DeviceInfo.VendorConfigFile" 
#define CTLOID_DEVICE_CONFIG 		"InternetGatewayDevice.DeviceConfig"
#define CTLOID_MANAGEMENT_SERVER 	"InternetGatewayDevice.ManagementServer"
#define CTLOID_TIME_SERVER_CFG 		"InternetGatewayDevice.Time"
#define CTLOID_IP_PING_DIAG 		"InternetGatewayDevice.IPPingDiagnostics"
#define CTLOID_LAN_DEV 				"InternetGatewayDevice.LANDevice"
#define CTLOID_LAN_HOST_CFG 		"InternetGatewayDevice.LANDevice.1.LANHostConfigManagement"
#define CTLOID_LAN_IP_INTF 			"InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1"
#define CTLOID_IPV6_LAN_HOST_CFG 	"InternetGatewayDevice.LANDevice.1.X_BROADCOM_COM_IPv6LANHostConfigManagement"
#define CTLOID_MLD_SNOOPING_CFG 	"InternetGatewayDevice.LANDevice.1.X_BROADCOM_COM_IPv6LANHostConfigManagement.X_BROADCOM_COM_MldSnoopingConfig"
#define CTLOID_LAN_ETH_INTF 		"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig"
#define CTLOID_LAN_ETH_INTF_STATS 	"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1.Stats"
#define CTLOID_LAN_HOSTS 			"InternetGatewayDevice.LANDevice.1.Hosts"
#define CTLOID_LAN_HOST_ENTRY 		"InternetGatewayDevice.LANDevice.1.Hosts.Host"
#define CTLOID_WAN_DEV 				"InternetGatewayDevice.WANDevice.1"
#define CTLOID_WAN_COMMON_INTF_CFG 	"InternetGatewayDevice.WANDevice.1.WANCommonInterfaceConfig"
#define CTLOID_WAN_CONN_DEVICE 		"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1"
#define CTLOID_WAN_IP_CONN 			"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1"
#define CTLOID_WAN_MOCA_CONN 		"InternetGatewayDevice.WANDevice.2.WANConnectionDevice.1.WANIPConnection.1"
#define CTLOID_WAN_IP_CONN_PORTMAPPING 	"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1.PortMapping"
#define CTLOID_WAN_PPP_CONN 		"InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.1"
#define CTLOID_WAN_PPP_CONN_PORTMAPPING "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.1.PortMapping"
#define CTLOID_IPV6_L3_FORWARDING 	"InternetGatewayDevice.X_BROADCOM_COM_IPv6Layer3Forwarding"
#define CTLOID_IPV6_L3_FORWARDING_ENTRY "InternetGatewayDevice.X_BROADCOM_COM_IPv6Layer3Forwarding.IPv6Forwarding"
#define CTLOID_IPV6_LAN_PD_CFG		"InternetGatewayDevice.LANDevice.1.LANPDConfigManagement"
#define CTLOID_IPV6_LAN_PD_INST		"InternetGatewayDevice.LANDevice.1.LANPDConfigManagement.LanPDInstance"
#define CTLOID_6RD_NUD              "InternetGatewayDevice.X_AEI_COM_IPv6NUD"
#endif //_CTL_OBJECTID_H

