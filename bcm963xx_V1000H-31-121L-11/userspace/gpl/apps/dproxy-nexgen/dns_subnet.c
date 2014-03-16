#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <syslog.h>
#include <linux/if.h>
#include "dproxy.h"
#include "dns_decode.h"
#include "conf.h"
#include "dns_list.h"
#include "dns_construct.h"
#include "dns_io.h"
#include "dns_dyn_cache.h"
#include "dns_probe.h"



/* Used for the interface grouping where the incoming lan ip will be  used to check against 
 * the lanSubnet/lanMask for the correct dns.  Currently, only the dns1 will be used which means
 * dnsprobe will only work on the default WAN connection (dns from /etc/resolv.conf)
 */
typedef struct _SubnetDnsPair
{
    /* LAN IPv4 address */
   struct in_addr lanSubnet;
   struct in_addr lanMask;
   char *dns1;
   char *dns2;
   struct _SubnetDnsPair *next;
} SubnetDnsPair;

static SubnetDnsPair *head = NULL;


void dns_sunbet_init(void)
{
   FILE *fp;
   char wanIfName[CMS_IFNAME_LENGTH] = {0};
   char subnetCidr[CMS_IPADDR_LENGTH] = {0};
   char dnsList[5 * CMS_IPADDR_LENGTH] = {0};   /* make space for 5 dns entries in case  they have that many */
   char dns1[CMS_IPADDR_LENGTH];
   char dns2[CMS_IPADDR_LENGTH];
   char line[BUFLEN_128];
   SubnetDnsPair *curr = NULL;
   UBOOL8 done = FALSE;
   struct in_addr lanSubnet;
   struct in_addr lanMask;
   char *curPtr;
   char *nullPtr;
   
   if ((fp = fopen(WAN_DNS_SUBNET_FILE, "r")) == NULL)
   {
      cmsLog_notice(" %s does not exist.", WAN_DNS_SUBNET_FILE);
      return;
   }

   /* First free the linked list */
   curr = head;
   while (curr)
   {
      cmsLog_notice("Free subnet=%X, mask=%X, dns1=%s, dns2=%s",
                    curr->lanSubnet,  curr->lanMask,
                    curr->dns1, curr->dns2);
      cmsMem_free(curr->dns1);
      cmsMem_free(curr->dns2);
      curr = curr->next;
      cmsMem_free(curr);
   }
   head = NULL;
   
   while (!done &&  fgets(line, sizeof(line), fp))
   {
      curPtr = line;
      if ((nullPtr = strchr(curPtr, ';')) != NULL)
      {
         *nullPtr = '\0';
#if defined(AEI_COVERITY_FIX)
         /*CID 12231: Destination buffer too small*/
         strlcpy(wanIfName, curPtr, sizeof(wanIfName));
#else
         strcpy(wanIfName, curPtr);
#endif
         curPtr = nullPtr + 1;         
      }
      
      if ((nullPtr = strchr(curPtr, ';')) != NULL)
      {
         *nullPtr = '\0';
         strcpy(subnetCidr, curPtr);
         curPtr = nullPtr + 1;         
      }
      
      sprintf(dnsList, "%s", curPtr);

      /* get rid of '\n' */
#if defined(AEI_COVERITY_FIX)
      /*CID 12157:Stray semicolon*/
      if (dnsList[strlen(dnsList) -1] == '\n')
#else
      if (dnsList[strlen(dnsList) -1] == '\n');
#endif
      {
         dnsList[strlen(dnsList) - 1] = '\0';
      }
      
      cmsLog_notice("wanif=%s, subnetCidr=%s, dnsList=%s", wanIfName,  subnetCidr, dnsList);

      if (cmsUtl_parseDNS(dnsList, dns1, dns2, TRUE) != CMSRET_SUCCESS)
      {
         /* fix resource leak, coverity id 11833 */
#if defined(AEI_COVERITY_FIX)
         fclose(fp);
#endif
         cmsLog_error("Failed to parse dns list %s", dnsList);
         return;
      }

      if (!IS_EMPTY_STRING(subnetCidr))
      {
         cmsNet_inet_cidrton(subnetCidr, &lanSubnet, &lanMask);
         if (lanSubnet.s_addr != 0 && lanMask.s_addr != 0)
         {
            if ((cmsUtl_parseDNS(dnsList, dns1, dns2, TRUE)) == CMSRET_SUCCESS)
            {
               if ((curr = cmsMem_alloc(sizeof(SubnetDnsPair), ALLOC_ZEROIZE)) != NULL)
               {
                  curr->dns1 = cmsMem_strdup(dns1);
                  curr->dns2 = cmsMem_strdup(dns2);
                  curr->lanSubnet  = lanSubnet;
                  curr->lanMask = lanMask;
                  curr->next = head;
                  head = curr;
               }
               else
               {
                  cmsLog_error("Failed to allocate memory.");
                  done = TRUE;
               }              
            }
            else
            {
               cmsLog_error("Failed to parse dns list %s.", dnsList);
               done = TRUE;
            }

         }
         else
         {
            cmsLog_error("Failed to convert LAN subnet/mask from %s.", subnetCidr);
            done = TRUE;
         }
      }
      wanIfName[0] = '\0';
      subnetCidr[0]= '\0';
      dnsList[0] = '\0';
      
   }

   curr = head;
   while (curr)
   {
      cmsLog_notice("subnet=%s", inet_ntoa(curr->lanSubnet));
      cmsLog_notice("mask=%s, dns1=%s, dns2=%s",
                    inet_ntoa(curr->lanMask), curr->dns1, curr->dns2);
      curr = curr->next;
   }

   fclose(fp);
}

#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
UBOOL8 dns_subnet_map(struct in6_addr *lanIp, char *dns1)
#else
UBOOL8 dns_subnet_map(struct in_addr *lanIp, char *dns1)
#endif
 {
   UBOOL8 found = FALSE;
   SubnetDnsPair *curr = head;
   
   if (dns1 == NULL)
   {
      cmsLog_error("Invalid dns1 parameter");
      return found;
   }

   while (!found && curr)
   {

#if defined(DMP_X_BROADCOM_COM_IPV6_1) || defined(AEI_CONTROL_LAYER)
	   /* need ipv6-review, for now fail compare */
	  if (0)
#else
      if ( (lanIp->s_addr & curr->lanMask.s_addr) == curr->lanSubnet.s_addr)
#endif
      {
         /* same subnet found and need to get the dns1 */
         found = TRUE;
         strcpy(dns1, curr->dns1);
         /* todo: currently probe only supported for the default wan interface (dns from /etc/resolv.con)
         * and if probing is needed for other wan interface, dns2 need to be used 
         */
#ifndef DMP_X_BROADCOM_COM_IPV6_1 
         /* need ipv6-review */
#ifndef AEI_CONTROL_LAYER
         cmsLog_notice("Found dns %s for subnet %s", dns1, inet_ntoa(*lanIp));
#endif
#endif
      }
      else
      {
         curr = curr->next;
         cmsLog_notice("Not found. get next one");
      }
   }

   return found;
   
 }






