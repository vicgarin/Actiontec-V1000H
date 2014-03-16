#ifndef __AEI_URL_UTIL_H__
#define __AEI_URL_UTIL_H__

#define WEB_PORT 80
#define capURLFile "/var/captive_url"
#define capAllowListFile  "/var/captive_allowlist"
#define capAllowDomainFile  "/var/captive_allowlist_domain"
#define oneTimeCapURLFile "/var/oneTimeRedirectURL"

int AEI_checkCaptiveAllowList(char *allowList, __be32 ip);
int AEI_send_redirect (struct nfq_q_handle *qh, int id, struct nfq_data * payload, char *capurl);
void AEI_get_lan_ip(char *addr);
void AEI_getCaptiveAllowList();
void AEI_getCaptiveURLandIPAddr(char *fileName, char *url, char *ipAddr, int *flag);
#endif
