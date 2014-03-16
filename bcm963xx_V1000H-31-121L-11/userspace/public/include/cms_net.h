/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
 ************************************************************************/


#ifndef __CMS_NET_H__
#define __CMS_NET_H__

/*!\file cms_net.h
 * \brief Header file for network utilities that do not require
 *  access to the MDM.
 */


#include "cms.h"
#include <arpa/inet.h>  /* mwang_todo: should not include OS dependent file */

/** Get LAN IP address and netmask.
 *
 * @param ifname (IN) name of LAN interface.
 * @param lan_ip (OUT) in_addr of the LAN interface.
 * @param lan_subnetmask (OUT) in_addr of the LAN interface subnet mask.
 *
 * @return CmsRet enum.
 */
CmsRet cmsNet_getLanInfo(const char *ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask);


/** Return TRUE if the specified interface is UP.
 *
 * @param ifname (IN) name of the interface.
 *
 * @return TRUE if the specified interface is UP.
 */
UBOOL8 cmsNet_isInterfaceUp(const char *ifname);


/** Return TRUE if the specified IP address is on the LAN side.
 *
 * @param ipAddr (IN) IP address in question.
 *
 * @return TRUE if the specified IP address is on the LAN side.
 */
UBOOL8 cmsNet_isAddressOnLanSide(const char *ipAddr);


/** Returns the number of left most one bit in subnet mask.
 *  Ex: 255.255.255.0 -> 24
 * 
 * @param ipMask (IN) IP mask string in dot notation.
 *
 * @return the number of left most one bit in the given subnet mask.
 */
UINT32 cmsNet_getLeftMostOneBitsInMask(const char *ipMask);


/** Convert an IP address/mask string from CIDR notation into binary address and mask
 *  and store them in the structures that ipAddr and ipMask point to respectively.
 *  Ex: 192.168.1.100/24 -> ipAddr->s_addr=0xCOA80164  ipMask->s_addr=0xFFFFFF00
 * 
 * @param cp (IN) IP address/mask string in CIDR notation.
 * @param ipAddr (OUT) IP address in binary. Set to 0 if invalid.
 * @param ipMask (OUT) IP mask in binary. Set to 0 if invalid.
 *
 * @return void
 */
void cmsNet_inet_cidrton(const char *cp, struct in_addr *ipAddr, struct in_addr *ipMask);

/** Return the ifindex of an interface.
 *
 * @param ifname (IN) .
 * @return ifindex of the interface.
 */
SINT32 cmsNet_getIfindexByIfname(char *ifname);


/** Get a list of interface names in the system.
 *
 * @param ifNameList (OUT) Pointer to char * of ifnames.  This function will allocate
 *                         a buffer long enough to hold a list of all the interface
 *                         names in the system, separated by comma.  e.g. atm0,ppp0,eth0,eth1
 *                         Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getIfNameList(char **ifNameList);


/** Get a list of persistent WAN ethernet interface names in the system.
 *
 * @param PersistentWanifNameList (OUT) Pointer to char * of PersistentWanifNameList.  
 *                                      This function will allocate
 *                                      a buffer long enough to hold a list of all the interface
 *                                      names in the system, separated by comma.  e.g. eth0,eth1
 *                                      Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getPersistentWanIfNameList(char **PersistentWanifNameList);

#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER) /* aka SUPPORT_IPV6 */
CmsRet cmsNet_getIfAddr6(const char *ifname, UINT32 addrIdx,
                         char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags);


/** Get the Globally Unique IPv6 address for the specified ifname.
 *
 * @param ifname  (IN)  desired ifname
 * @param ipAddr  (OUT) Caller supplies a buffer to hold address.  Must be at least
 *                      CMS_IPADDR_LENGTH (46 bytes) long.
 * @param prefixLen (OUT) Prefix len of the address
 *
 * @return CmsRet
 */
CmsRet cmsNet_getGloballyUniqueIfAddr6(const char *ifname, char *ipAddr, UINT32 *prefixLen);


/** Check if two IPv6 addresses are equal.
 *
 * @param ip6Addr1 (IN) IPv6 address 1.
 * @param ip6Addr2 (IN) IPv6 address 2.
 *
 * @return TRUE if the specified two IPv6 addresses are equal.
 */
UBOOL8 cmsNet_areIp6AddrEqual(const char *ip6Addr1, const char *ip6Addr2);

/** Check if two IPv6 DNS server addresses are equal.
 *
 * @param dnsServers1 (IN) DNS server address 1.
 * @param dnsServers2 (IN) DNS server address 2.
 *
 * @return TRUE if the specified two DNS server addresses are equal.
 */
UBOOL8 cmsNet_areIp6DnsEqual(const char *dnsServers1, const char *dnsServers2);

/** Check if a host IPv6 address is in the same subnet of an address prefix.
 *
 * @param addrHost   (IN) host address to check.
 * @param addrPrefix (IN) the address prefix.
 *
 * @return TRUE if the host address is in the same subnet of the address prefix.
 */
UBOOL8 cmsNet_isHostInSameSubnet(const char *addrHost, const char *addrPrefix);

/** This function will subnet an address prefix based on the subnet id and the
 *  subnet prefix length..
 *
 * @param sp       (IN) the address prefix in CIDR notation.
 * @param subnetId (IN) the subnet id.
 * @param snPlen   (IN) the subnet prefix length.
 * @param snPrefix (OUT) the subnet address prefix.
 *
 * @return CmsRet enum.
 */
CmsRet cmsNet_subnetIp6SitePrefix(const char *sp, UINT8 subnetId, UINT32 snPlen, char *snPrefix);
#endif

/** first sub interface number for virtual ports, e.g. eth1.2, eth1.3 */
#define START_PMAP_ID           2

/** Max vendor id string len */
#define DHCP_VENDOR_ID_LEN      64

/** Maximum number of vendor id strings we can support in WebUI for portmapping. 
 * 
 * This is an arbitrary limit from the WebUI, but it propagates through to
 * utility functions dealing with DHCP Vendor Id for port mapping.
 */
#define MAX_PORTMAPPING_DHCP_VENDOR_IDS     5


#endif /* __CMS_NET_H__ */
