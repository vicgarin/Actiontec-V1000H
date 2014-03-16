#ifndef _CTL_H
#define _CTL_H

#ifndef BUFLEN_4 // if this macro was defined, measn already included BRCM's header file

#define IP_TBL_COL_NUM     0
#define IP_TBL_COL_PKTS    1
#define IP_TBL_COL_BYTES   2
#define IP_TBL_COL_TARGET  3
#define IP_TBL_COL_PROT    4
#define IP_TBL_COL_OPT     5
#define IP_TBL_COL_IN      6
#define IP_TBL_COL_OUT     7
#define IP_TBL_COL_SRC     8
#define IP_TBL_COL_DST     9
#define IP_TBL_COL_MAX     10


#define BUFLEN_4        4     //!< buffer length 4
#define BUFLEN_8        8     //!< buffer length 8
#define BUFLEN_16       16    //!< buffer length 16
#define BUFLEN_18       18    //!< buffer length 18 -- for ppp session id
#define BUFLEN_24       24    //!< buffer length 24 -- mostly for password
#define BUFLEN_32       32    //!< buffer length 32
#define BUFLEN_40       40    //!< buffer length 40
#define BUFLEN_48       48    //!< buffer length 48
#define BUFLEN_64       64    //!< buffer length 64
#define BUFLEN_80       80    //!< buffer length 80
#define BUFLEN_128      128   //!< buffer length 128
#define BUFLEN_256      256   //!< buffer length 256
#define BUFLEN_264      264   //!< buffer length 264
#define BUFLEN_512      512   //!< buffer length 512
#define BUFLEN_1024     1024  //!< buffer length 1024

#define IIDSTACK_BUF_LEN  40  //!< good length to use for mdm_dumpIidStack
#define MAC_ADDR_LEN    6     //!< Mac address len in an array of 6 bytes
#define MAC_STR_LEN     17    //!< Mac String len with ":". eg: xx:xx:xx:xx:xx:xx
#define VPI_MIN         0     //!< VPI min 
#define VPI_MAX         255   //!< VPI max 
#define VCI_MIN         32    //!< VCI min 
#define VCI_MAX         65535 //!< VCI max 

#define PPP_CONNECT_ERROR_REASON_LEN 48

#define CTL_IFNAME_LENGTH  BUFLEN_32   //!< broadcom interface name length

//#ifdef DMP_X_BROADCOM_COM_IPV6_1    /* aka SUPPORT_IPV6 */
#define CTL_IPADDR_LENGTH  44          //!< IP address length to hold IPv6 in CIDR notation
//#else
//#endif /* DMP_X_BROADCOM_COM_IPV6_1 */
//#
#define CTL_MAX_DEFAULT_GATEWAY     8  //!< max default gateways allowed in L3 X_BROADCOM_COM_DefaultConnectionServices
#define CTL_MAX_DNSIFNAME           8  //!< max dns wan interface names in X_BROADCOM_Networking.DNSIfName
#define CTL_MAX_ACTIVE_DNS_IP       4  //!< max active dns ip (in resolv.conf)

#define NUD_PORT    15535
/** Free a buffer and set the pointer to null.
 */
 #define CTLMEM_FREE_BUF_AND_NULL_PTR(p) \
    do { \
	  if ((p) != NULL) {free((p)); (p) = NULL;}   \
	} while (0)

#define IS_EMPTY_STRING(s)    ((s == NULL) || (*s == '\0'))

#endif // BUFLEN_4
#endif //_CTL_H

