#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <syslog.h>
#include <unistd.h>
#include "filter.h"
#include "aei_url_util.h"
#include <signal.h>
#if defined (DMP_CAPTIVEPORTAL_1)
#include "cms.h"
#include "cms_util.h"
#include "cms_msg.h"
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#if defined (DMP_CAPTIVEPORTAL_1)
void *msgHandle=NULL;  
char captiveAllowList[10001]={0};
char captiveAllowDomain[10001]={0};
int flagCaptiveURL = 0;
char captiveURL[BUFLEN_256] = {0};
char captiveIPAddr[32] = {0};
char GlbRedirectUrl[BUFLEN_256]={ 0 };

#if defined(AEI_VDSL_TR098_QWEST)
int flagOneTimeRedirect = 0;
char oneTimeRedirectURL[256] = {0};
char oneTimeRedirectIPAdress[32];
#endif
#endif

#if defined(AEI_VDSL_CUSTOMER_WEBACTIVELOG_SWITCH)
#define webActiveLogFile  "/var/webActiveLogFile"
int webActiveLogEnable = 0;
void AEI_getWebActiveInfo();
#endif

#define BUFSIZE 2048

// turn on the urlfilterd debug message.
// #define UFD_DEBUG 1

typedef enum
{
	PKT_ACCEPT,
	PKT_DROP
#if defined(DMP_CAPTIVEPORTAL_1)
	,PKT_REDIRECT
#endif
}pkt_decision_enum;

struct nfq_handle *h;
struct nfq_q_handle *qh;
char listtype[8];

#if defined (AEI_WLAN_URL_REDIRECT)
char brname[16]={0};
#endif 

#if defined(AEI_VDSL_CUSTOMER_NCS)
char allLanIpList[1024] = { 0 };
void AEI_HandleWebActivityLog(const char *match, const struct iphdr *iph);
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
void add_entry(char *website, char *folder, char *lanIP)
#else
void add_entry(char *website, char *folder)
#endif
{
    PURL new_entry, current, prev;
    new_entry = (PURL) malloc(sizeof(URL));
    strcpy(new_entry->website, website);
    strcpy(new_entry->folder, folder);

#if defined(AEI_VDSL_CUSTOMER_NCS)
    strncpy(new_entry->lanIP, lanIP, 15);
#endif

    new_entry->next = NULL;

    if (purl == NULL) {
        purl = new_entry;
    } else {
        current = purl;
        while (current) {
            prev = current;
            current = current->next;
        }
        prev->next = new_entry;
    }
}

int get_url_info()
{
	char temp[MAX_WEB_LEN + MAX_FOLDER_LEN], *temp1, *temp2, web[MAX_WEB_LEN], folder[MAX_FOLDER_LEN];
			
	FILE *f = fopen("/var/url_list", "r");
	if (f != NULL){
	   while (fgets(temp,96, f) != '\0')
	   {
#if defined(AEI_VDSL_CUSTOMER_NCS)
        char lanIP[16] = { 0 };
        char *pos = NULL;
        char *pe = NULL;
        if ((pos = strchr(temp, ';')) != NULL) {
            *pos++ = '\0';
            if (*pos == '\0')
                strcpy(lanIP, "\0");
            else {
                pe = strrchr(pos, '\n');
                if (pe)
                    *pe = '\0';
            }
            strcpy(lanIP, pos);
        }
        strcat(allLanIpList, lanIP);
        strcat(allLanIpList, " ");
#endif
		if (temp[0]=='h' && temp[1]=='t' && temp[2]=='t' && 
			temp[3]=='p' && temp[4]==':' && temp[5]=='/' && temp[6]=='/')
		{
			temp1 = temp + 7;	
		}
		else
		{
			temp1 = temp;	
		}

		if ((*temp1=='w') && (*(temp1+1)=='w') && (*(temp1+2)=='w') && (*(temp1+3)=='.'))
		{
			temp1 = temp1 + 4;
		}

		if ((temp2 = strchr(temp1, '\n')))
		{
			*temp2 = '\0';
		}
		       
		sscanf(temp1, "%[^/]", web);		
		temp1 = strchr(temp1, '/');
		if (temp1 == NULL)
		{
			strcpy(folder, "\0");
		}
		else
		{
			strcpy(folder, ++temp1);		
		}
#if defined(AEI_VDSL_CUSTOMER_NCS)
        add_entry(web, folder, lanIP);
#else
        add_entry(web, folder);
#endif		
		list_count ++;
	   }
	   fclose(f);
	}
#ifdef UFD_DEBUG
	else {
	   printf("/var/url_list isn't presented.\n");
	   return 1;
	}
#endif


	return 0;
}

static int pkt_decision(struct nfq_data *payload)
{
	char *data;
	char *match, *folder, *url;
	PURL current;
	int payload_offset, data_len;
	struct iphdr *iph;
	struct tcphdr *tcp;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
    char domain[1024+1]="";
#endif
	match = folder = url = NULL;

	data_len = nfq_get_payload(payload, &data);
	if( data_len == -1 )
	{
#ifdef UFD_DEBUG
	printf("data_len == -1!!!!!!!!!!!!!!!, EXIT\n");
#endif
		exit(1);
	}
#ifdef UFD_DEBUG
	printf("data_len=%d ", data_len);
#endif

    iph = (struct iphdr *)data;
    tcp = (struct tcphdr *)(data + (iph->ihl << 2));

	payload_offset = ((iph->ihl)<<2) + (tcp->doff<<2);
	match = (char *)(data + payload_offset);

	if(strstr(match, "GET ") == NULL && strstr(match, "POST ") == NULL && strstr(match, "HEAD ") == NULL)
	{
#ifdef UFD_DEBUG
	printf("****NO HTTP INFORMATION!!!\n");
#endif
		return PKT_ACCEPT;
	}
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
        else{
                AEI_getdomain(match, domain);
        }
#endif


#ifdef UFD_DEBUG
	printf("####payload = %s\n\n", match);
#endif

#if defined (AEI_WLAN_URL_REDIRECT)
        {
            char mac_str[128]={0};
            char url_redirect[256]={0};
            char *pUrl = NULL;
            struct nfqnl_msg_packet_hw *hw = NULL;
            hw = nfq_get_packet_hw(payload);
            if ( tcp->dest == htons (WEB_PORT) && (strstr(match,"GET / HTTP/")||
                        strstr(match,"get / HTTP/")||
                        strstr(match,"GET /"))||
                    strstr(match,"get /") )
            {       
                sprintf(mac_str,"%02x:%02x:%02x:%02x:%02x:%02x",hw->hw_addr[0],hw->hw_addr[1],hw->hw_addr[2],
                        hw->hw_addr[3],hw->hw_addr[4],hw->hw_addr[5]);
                AEI_processSSID234UrlRedirect(mac_str,url_redirect,sizeof(url_redirect)-1,brname,match);
                if (strcmp(url_redirect,"")){
                    AEI_urltrim(url_redirect);
                    pUrl = strstr(url_redirect,"http://");
                    if(pUrl){
                        pUrl = pUrl + 7;
                        if  (strstr(match, pUrl) == NULL) {
//                            status = send_redirect (qh, id, payload, url_redirect);
                            memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl)); 
                            sprintf(GlbRedirectUrl, "%s", url_redirect);                           
                            return PKT_REDIRECT;
                        }
                    }else{
                        if  (strstr(match, url_redirect) == NULL) {
//                            status = send_redirect (qh, id, payload, url_redirect);
                            memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl)); 
                            sprintf(GlbRedirectUrl, "%s", url_redirect);                           
                            return PKT_REDIRECT;
                        }
                    }
                }

            }
        }
#endif

#if defined (DMP_CAPTIVEPORTAL_1)
#if defined(AEI_VDSL_TR098_QWEST)
	if ((flagOneTimeRedirect == 1) && (tcp->dest == htons (WEB_PORT)))
	{
		if(strstr(match, "GET / HTTP/") || strstr(match, "get HTTP/")) 
		{        
			flagOneTimeRedirect = 2;
			AEI_send_msg_to_set_oneTimeRedirectURLFlag();
			if ((strstr(match, oneTimeRedirectURL) == NULL) && (strstr(oneTimeRedirectIPAdress, inet_ntoa(*(struct in_addr *) &iph->daddr)) == NULL))
			{
				//status = send_redirect (qh, id, payload, oneTimeRedirectURL);
				//return status;
    			memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl));
    			sprintf(GlbRedirectUrl, "%s", oneTimeRedirectURL);
				return PKT_REDIRECT;
			}
		}	
	}
	else if ((flagOneTimeRedirect == 2) && (tcp->dest == htons (WEB_PORT)))
	{               
		if ((strstr(match, "GET / HTTP/") || strstr(match, "get / HTTP/")))	
		{           
			if ((strstr(match, oneTimeRedirectURL) == NULL) && (strstr(oneTimeRedirectIPAdress, inet_ntoa(*(struct in_addr *) &iph->daddr)) == NULL))
			{              
				flagOneTimeRedirect = 0;
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                                if ((flagCaptiveURL == 1) && ((!AEI_checkCaptiveAllowList(captiveAllowList, iph->daddr))
                                        &&(domain[0] && (!AEI_checkCaptiveAllowDomain(captiveAllowDomain, domain)))))
#else
				if ((flagCaptiveURL == 1) && (!AEI_checkCaptiveAllowList(captiveAllowList, iph->daddr)))
#endif
				{            
					if ((strstr(match, captiveURL) == NULL) && (strstr(captiveIPAddr, inet_ntoa(*(struct in_addr *) &iph->daddr)) == NULL))
					{    
			            //status = send_redirect (qh, id, payload, captiveURL);
						//return status;
    					memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl));
    					sprintf(GlbRedirectUrl, "%s", captiveURL);
						return PKT_REDIRECT;
					}
				}
			}
		}	
	}
	else	
#endif
	{              
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                if ((flagCaptiveURL == 1) && (tcp->dest == htons (WEB_PORT)) && ((!AEI_checkCaptiveAllowList(captiveAllowList, iph->daddr))
                        &&(domain[0] && (!AEI_checkCaptiveAllowDomain(captiveAllowDomain, domain)))))
#else
		if ((flagCaptiveURL == 1) && (tcp->dest == htons (WEB_PORT)) && (!AEI_checkCaptiveAllowList(captiveAllowList, iph->daddr)))
#endif
		{               
			if(strstr(match, "GET /") || strstr(match, "get /")) 
			{        
				if ((strstr(match, captiveURL) == NULL) && (strstr(captiveIPAddr, inet_ntoa(*(struct in_addr *) &iph->daddr)) == NULL))
				{
					//status = send_redirect (qh, id, payload, captiveURL);
					flagCaptiveURL = 1;
					//return status;
    				memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl));
    				sprintf(GlbRedirectUrl, "%s", captiveURL);
					return PKT_REDIRECT;
				}
			}
		}
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
                else if ((flagCaptiveURL == 2) && (tcp->dest == htons (WEB_PORT)) && ((!AEI_checkCaptiveAllowList(captiveAllowList, iph->daddr))
                        &&(domain[0] && (!AEI_checkCaptiveAllowDomain(captiveAllowDomain, domain)))))
#else
		else if ((flagCaptiveURL == 2) && (tcp->dest == htons (WEB_PORT)) && (!AEI_checkCaptiveAllowList(captiveAllowList, iph->daddr)))
#endif
		{
			if(strstr(match, "GET / HTTP/") || strstr(match, "get / HTTP/")) 
			{
				if ((strstr(match, captiveURL) == NULL) && (strstr(captiveIPAddr, inet_ntoa(*(struct in_addr *) &iph->daddr)) == NULL))
				{
					//status = send_redirect (qh, id, payload, captiveURL);
					flagCaptiveURL = 1;
					//return status;
    				memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl));
    				sprintf(GlbRedirectUrl, "%s", captiveURL);
					return PKT_REDIRECT;
				}
			}
		}	
	}	
#endif

#if defined(AEI_VDSL_CUSTOMER_NCS)
    AEI_HandleWebActivityLog(match, iph);
#endif
	for (current = purl; current != NULL; current = current->next)
	{
		if (current->folder[0] != '\0')
		{
			folder = strstr(match, current->folder);
		}

		if ( (url = strstr(match, current->website)) != NULL ) 
		{
			if (strcmp(listtype, "Exclude") == 0) 
			{
				if ( (folder != NULL) || (current->folder[0] == '\0') )
				{
#if defined(AEI_VDSL_CUSTOMER_NCS)
                    if (strstr(current->lanIP, "all") != NULL) {        //block all PCs
                        printf("All####This page is blocked by Exclude list!, into send_redirect\n");
#if defined(AEI_VDSL_CAPTIVE_PAGES)
						return PKT_REDIRECT;
#else                        
                        return PKT_DROP;
#endif                        
                    } else {    //block specific PCs
                        struct in_addr lanIP;
                        inet_aton(current->lanIP, &lanIP);
                        if (lanIP.s_addr == iph->saddr) {
                            printf("IP####This page is blocked by Exclude list!, into send_redirect\n");
#if defined(AEI_VDSL_CAPTIVE_PAGES)
                            return PKT_REDIRECT;
#else                            
                            return PKT_DROP;
#endif                            
                        }
                    }
#else
#ifdef UFD_DEBUG
					printf("####This page is blocked by Exclude list!");
#endif
					return PKT_DROP;
#endif
				}
				else 
				{
#ifdef UFD_DEBUG
					printf("###Website hits but folder no hit in Exclude list! packets pass\n");
#endif
					return PKT_ACCEPT;
				}
			}
			else 
			{
				if ( (folder != NULL) || (current->folder[0] == '\0') )
				{
#ifdef UFD_DEBUG
					printf("####This page is accepted by Include list!");
#endif
					return PKT_ACCEPT;
				}
				else 
				{
#ifdef UFD_DEBUG
					printf("####Website hits but folder no hit in Include list!, packets drop\n");
#endif
					return PKT_DROP;
				}
			}
		}
	}

	if (url == NULL) 
	{
		if (strcmp(listtype, "Exclude") == 0) 
		{
#ifdef UFD_DEBUG
			printf("~~~~No Url hits!! This page is accepted by Exclude list!\n");
#endif
			return PKT_ACCEPT;
		}
		else 
		{
#ifdef UFD_DEBUG
			printf("~~~~No Url hits!! This page is blocked by Include list!\n");
#endif
			return PKT_DROP;
		}
	}

#ifdef UFD_DEBUG
	printf("~~~None of rules can be applied!! Traffic is allowed!!\n");
#endif
	return PKT_ACCEPT;

}

/*
 * callback function for handling packets
 */
static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
	struct nfqnl_msg_packet_hdr *ph;
	int decision, id=0;

	ph = nfq_get_msg_packet_hdr(nfa);
	if (ph)
	{
		id = ntohl(ph->packet_id);
	}

	/* check if we should block this packet */
	decision = pkt_decision(nfa);
	if( decision == PKT_ACCEPT)
	{
		return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
	}
#if defined(DMP_CAPTIVEPORTAL_1)
    else if (decision == PKT_REDIRECT) {
    	printf("GlbRedirectUrl %s\n", GlbRedirectUrl);
		return AEI_send_redirect(qh, id, nfa, GlbRedirectUrl);
	}                        
#endif
	else
	{
		return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
	}

}

/*
 * Open a netlink connection and returns file descriptor
 */
int netlink_open_connection(void *data)
{
	struct nfnl_handle *nh;
 
#ifdef UFD_DEBUG
	printf("opening library handle\n");
#endif
	h = nfq_open();
	if (!h) 
	{
		fprintf(stderr, "error during nfq_open()\n");
		exit(1);
	}

#ifdef UFD_DEBUG
	printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
#endif
	if (nfq_unbind_pf(h, AF_INET) < 0) 
	{
		fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
	}

#ifdef UFD_DEBUG
	printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
#endif
	if (nfq_bind_pf(h, AF_INET) < 0) 
	{
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}

#ifdef UFD_DEBUG
	printf("binding this socket to queue '0'\n");
#endif
	qh = nfq_create_queue(h,  0, &cb, NULL);
	if (!qh) 
	{
		fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
	}

#ifdef UFD_DEBUG
	printf("setting copy_packet mode\n");
#endif
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) 
	{
		fprintf(stderr, "can't set packet_copy mode\n");
		exit(1);
	}

	nh = nfq_nfnlh(h);
	return nfnl_fd(nh);
}


int main(int argc, char **argv)
{
	int fd, rv;
	char buf[BUFSIZE]; 

	strcpy(listtype, argv[1]);
	if (get_url_info())
	{
	   printf("error during get_url_info()\n");
	   return 0;
	}

	memset(buf, 0, sizeof(buf));
#if defined(AEI_VDSL_CUSTOMER_NCS)
    signal(SIGINT, SIG_IGN);
#endif
#if defined (DMP_CAPTIVEPORTAL_1)
	cmsMsg_init(EID_URLFILTERD, &msgHandle);
	cmsLog_init(EID_URLFILTERD);
	
	AEI_getCaptiveURLandIPAddr(capURLFile, captiveURL, captiveIPAddr, &flagCaptiveURL);
#if defined(AEI_VDSL_TR098_QWEST)
	AEI_getCaptiveURLandIPAddr(oneTimeCapURLFile, oneTimeRedirectURL, oneTimeRedirectIPAdress, &flagOneTimeRedirect);
#endif        

#if defined(AEI_VDSL_CAPTIVE_PAGES)
	char lan_ip[16] = "\0";
    AEI_get_lan_ip(lan_ip);
    memset(GlbRedirectUrl, 0, sizeof(GlbRedirectUrl));
    sprintf(GlbRedirectUrl, "%s/captiveportal_pageblocked.html", lan_ip);
#endif
	AEI_getCaptiveAllowList();	
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
        AEI_getCaptiveAllowDomain();
#endif
#endif

#if defined(AEI_VDSL_CUSTOMER_WEBACTIVELOG_SWITCH)
	AEI_getWebActiveInfo();
#endif
	/* open a netlink connection to get packet from kernel */
	fd = netlink_open_connection(NULL);

	while (1)
	{
		rv = recv(fd, buf, sizeof(buf), 0);
		if ( rv >= 0) 
		{
#ifdef UFD_DEBUG
		   printf("pkt received\n");
#endif
		   nfq_handle_packet(h, buf, rv);
		   memset(buf, 0, sizeof(buf));
		}
		else
		{
		   nfq_close(h);
#ifdef UFD_DEBUG
		   printf("nfq close done\n");
#endif
		   fd = netlink_open_connection(NULL);
#ifdef UFD_DEBUG
		   printf("need to rebind to netfilter queue 0\n");
#endif
		}
	}
#ifdef UFD_DEBUG
        printf("unbinding from queue 0\n");
#endif
	nfq_destroy_queue(qh);
	nfq_close(h);

	return 0;
}

#if defined(AEI_VDSL_CUSTOMER_NCS)
time_t before;
int log_count = 0;
int url_count = -1;
int logNeedChange = 0;
char circularLog[URL_COUNT][ENTRY_SIZE] = { 0 };

#if defined(AEI_VDSL_CUSTOMER_WEBACTIVELOG_SWITCH)
void AEI_getWebActiveInfo()
{
	int isFile=0;
	if((isFile = access(webActiveLogFile,F_OK)) == 0)
		webActiveLogEnable=1;
	return;
}
#endif

int AEI_timeIsUp(time_t time)
{
    if ((time - before) > LOG_TIMEOUT) {
        before = time;
        return 1;
    } else
        return 0;
}

void AEI_writeLog()
{
    FILE *webLog;
    int i = 0;

    if (!logNeedChange || (url_count == -1))
        return;
    webLog = fopen("/var/webActivityLog", "w");

    if (!webLog) {
        fprintf(stderr, "/var/webActivityLog is created now.\n");
        return;
    }

    for (i = 0; i < log_count; i++) {
        if ((url_count - i) >= 0)
            fputs(circularLog[url_count - i], webLog);
        else
            fputs(circularLog[URL_COUNT + (url_count - i)], webLog);
    }
    fclose(webLog);
    logNeedChange = 0;
}

void AEI_HandleWebActivityLog(const char *match, const struct iphdr *iph)
{
    time_t now;
    struct tm *tmp = NULL;
    char currTime[64] = { 0 };
    char ip_addr[64] = { 0 };
    char web_site[64] = { 0 };
    char *temp_start = NULL;
    char *temp_end = NULL;
    char *cpy_start = NULL;
    char *cpy_end = NULL;

#if defined(AEI_VDSL_CUSTOMER_WEBACTIVELOG_SWITCH)
	if ( !webActiveLogEnable )
	{	
		//printf("#######webActivitylog disabled ...\n");
		return;
	}	
#endif
		//printf("#######webActivitylog disabled ...\n");

    temp_start = strstr(match, "Host: ");
    if (!temp_start) {
        return;
    }

    cpy_start = temp_start + 6;
    temp_end = strchr(cpy_start, '\n');
    if (!temp_end)
        return;

    cpy_end = temp_end - 1;
    if (cpy_end - cpy_start <= 0)
        return;

    memset(web_site, 0, sizeof(web_site));

    if ((sizeof(web_site) - 1) > (cpy_end - cpy_start))
        strncpy(web_site, cpy_start, (cpy_end - cpy_start));
    else
        strncpy(web_site, cpy_start, sizeof(web_site) - 1);

    strncpy(ip_addr, inet_ntoa(*(struct in_addr *)&iph->saddr), sizeof(ip_addr) - 1);

    now = time(NULL);
    tmp = localtime(&now);
    memset(currTime, 0, sizeof(currTime));
    strftime(currTime, sizeof(currTime), "%m/%d/%Y|%I:%M:%S:%p|", tmp);

    if (log_count < URL_COUNT)
        log_count++;
    if (++url_count == URL_COUNT)
        url_count = 0;

    snprintf(circularLog[url_count], ENTRY_SIZE - 1, "%s%s|%s|\n", currTime, ip_addr, web_site);
    logNeedChange = 1;
    if (AEI_timeIsUp(now)) {
        AEI_writeLog();
    }
}
#endif

