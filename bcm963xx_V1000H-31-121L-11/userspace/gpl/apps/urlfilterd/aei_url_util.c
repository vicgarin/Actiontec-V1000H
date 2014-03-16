#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/netfilter.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <syslog.h>
//#include "filter.h"
#include "aei_url_util.h"

#if defined (DMP_CAPTIVEPORTAL_1)
#include "cms.h"
#include "cms_util.h"
#include "cms_msg.h"
#endif


#if defined(AEI_VDSL_CUSTOMER_NCS)

#if defined(AEI_WLAN_URL_REDIRECT)
extern char brname[16];
#endif

#if defined (DMP_CAPTIVEPORTAL_1)
extern void *msgHandle;  
extern char captiveAllowList[10001];
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
extern char captiveAllowDomain[10001];
#endif

void AEI_getCaptiveAllowList()
{
	FILE *fp = NULL;
	
	if ((fp = fopen(capAllowListFile, "r")) != NULL)
	{
		fgets (captiveAllowList, sizeof(captiveAllowList)-1, fp);
		if (strstr(captiveAllowList, "None"))
		{
			captiveAllowList[0] = '\0';
		}
		printf("captiveAllowList %s\n", captiveAllowList);
		fclose(fp);	
	}
}

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
void AEI_getCaptiveAllowDomain()
{
	FILE *fp = NULL;
	
	if ((fp = fopen(capAllowDomainFile, "r")) != NULL)
	{
		fgets (captiveAllowDomain, sizeof(captiveAllowDomain)-1, fp);
		if (strstr(captiveAllowDomain, "None"))
		{
			captiveAllowDomain[0] = '\0';
		}
		printf("captiveAllowDomain %s\n", captiveAllowDomain);
		fclose(fp);	
	}
}
#endif

#if defined(AEI_VDSL_TR098_QWEST)
CmsRet AEI_send_msg_to_set_oneTimeRedirectURLFlag()
{
	CmsRet ret = CMSRET_SUCCESS;
	CmsMsgHeader send_Msg = EMPTY_MSG_HEADER;

	send_Msg.type = CMS_MSG_SET_ONE_TIME_REDIRECT_URL_FLAG;
	send_Msg.src = EID_URLFILTERD;
	send_Msg.dst = EID_SSK;

	if ((ret = cmsMsg_send(msgHandle, &send_Msg)) != CMSRET_SUCCESS)
	{

		printf("Failed to send message (ret=%d)", ret);
	}

	return ret;
}
#endif	//AEI_VDSL_TR098_QWEST
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
int AEI_checkCaptiveAllowDomain(char *allowDomain, char* host)
{
        char *p, *pLast, *pTemp;
        char allowbuf[10001] = {0};

        if (strlen(allowDomain) == 0)
        {
                return 0;
        }
        pLast = NULL;
        sprintf(allowbuf, "%s", allowDomain);
        p = strtok_r(allowbuf, ",", &pLast);
        while(p != NULL)
        {
                if(!strcasecmp(p, host))
                        return 1;
                p = strtok_r(NULL, ",", &pLast);
        }
        return 0;
}

char *AEI_getdomain(char *data, char *url)
{
	char *httpData = data;
        if(httpData == NULL) return NULL;

        char *pHost = strstr(httpData, "Host");
        if(pHost == NULL)  return NULL;

	char *pHostEnd = strstr(pHost, "\r");
	if(pHostEnd != NULL)
	{
		int nHostLen = (pHostEnd - pHost) - 6;
		if((nHostLen <= 0 ) || (nHostLen > 1024))  return NULL;
		
		strncpy(url,pHost+6,nHostLen);
		return url;
	}

	return NULL;
}
#endif

int AEI_checkCaptiveAllowList(char *allowList, __be32 ip)
{
	char *p, *pLast, *pTemp;
	char allowIpAddr[32] = {0};
	char allowbuf[10001] = {0};
	char buf[32] = {0};
	int maskNum = 0;
	struct in_addr allowIP;
	__be32 mask;
	__be32 ipNet;
	
	if (strlen(allowList) == 0)
	{
		return 0;
	}
	pLast = NULL;
	sprintf(allowbuf, "%s", allowList);
	p = strtok_r(allowbuf, ",", &pLast);
	while(p != NULL)
	{
		mask = 0xFFFFFFFF;
		sprintf(allowIpAddr, "%s", p);
		if ((pTemp = strchr(allowIpAddr, '/')) != NULL)
		{
			*pTemp = ' ';
			sscanf(allowIpAddr, "%s %d", buf, &maskNum);	
			inet_aton(allowIpAddr, &allowIP);	
			ipNet = htonl(ip);
			mask = mask<<(32-maskNum);
			if ((__be32)(allowIP.s_addr&mask) == (__be32)(ipNet&mask))
			{
				printf("find mask\n");
				return 1;
			}	
		}
		else
		{
			inet_aton(allowIpAddr, &allowIP);			
			if (allowIP.s_addr == ip)
			{
				printf("find ip\n");
				return 1;
			}
		}
		p = strtok_r(NULL, ",", &pLast);
	}
	return 0;
}

static unsigned short AEI_in_cksum (unsigned short *ptr, int nbytes)
{
	register long sum;		/* assumes long == 32 bits */
	u_short oddbyte;
	register u_short answer;	/* assumes u_short == 16 bits */
	/*
	* Our algorithm is simple, using a 32-bit accumulator (sum),
	* we add sequential 16-bit words to it, and at the end, fold back
	* all the carry bits from the top 16 bits into the lower 16 bits.
	*/
	
	sum = 0;
	while (nbytes > 1) 
	{
		sum += *ptr++;
		nbytes -= 2;
	}
	/* mop up an odd byte, if necessary */ 
	if (nbytes == 1) 
	{
		oddbyte = 0;		/* make sure top half is zero */
		*((u_char *) & oddbyte) = *(u_char *) ptr;	/* one byte only */
		sum += oddbyte;
	}
	
	/*
	* Add back carry outs from top 16 bits to low 16 bits.
	*/
	
	sum = (sum >> 16) + (sum & 0xffff);	/* add high-16 to low-16 */
	sum += (sum >> 16);		/* add carry */
	answer = ~sum;		/* ones-complement, then truncate to 16 bits */
	return (answer);
}

struct psuedohdr  
{
  struct in_addr source_address;
  struct in_addr dest_address;
  unsigned char place_holder;
  unsigned char protocol;
  unsigned short length;
} psuedohdr;

unsigned short AEI_trans_check(unsigned char proto,char *packet,int length,struct in_addr source_address,struct in_addr dest_address)
{
  char *psuedo_packet;
  unsigned short answer;

  psuedohdr.protocol = proto;
  psuedohdr.length = htons(length);
  psuedohdr.place_holder = 0;

  psuedohdr.source_address = source_address;
  psuedohdr.dest_address = dest_address;

  if((psuedo_packet = malloc(sizeof(psuedohdr) + length)) == NULL)  
  {
    perror("malloc");
    exit(1);
  }

  memcpy(psuedo_packet,&psuedohdr,sizeof(psuedohdr));
  memcpy((psuedo_packet + sizeof(psuedohdr)), packet,length);

  answer = (unsigned short)AEI_in_cksum((unsigned short *)psuedo_packet, (length + sizeof(psuedohdr)));
  free(psuedo_packet);
  return answer;
}

int AEI_send_redirect (struct nfq_q_handle *qh, int id, struct nfq_data * payload, char *capurl)
{
	struct iphdr *iph =  NULL;
	struct udphdr *udp = NULL;
	struct tcphdr *tcp = NULL;
	char *data = NULL;
	char *p_indata = NULL;
	char *p_outdata = NULL;
	struct in_addr saddr, daddr;
	char buf_redirect[1024];
	__u32	old_seq;
	unsigned short old_port;
	int data_len = 0;
	int org_len = 0;
	int redirect_flag = 0;
	int status= 0;
	int tcp_doff = 0;
	struct nfqnl_msg_packet_hw *hw = NULL;
	unsigned char dmac[6] = {0};
	int i = 0;
	char loc[20]= {0};

	memset (buf_redirect, 0, sizeof (buf_redirect));
	
	data_len = nfq_get_payload(payload, &p_indata);
	if( data_len == -1 )
	{
		printf("data_len == -1!!!!!!!!!!!!!!!, EXIT\n");
		exit(1);
	}

//	printf("data_len=%d, id=%d\n ", data_len, id);
	
	hw = nfq_get_packet_hw(payload);
	
	for (i = 0; i < 6; i++)
	{
		dmac[i] = hw->hw_addr[i];
	}	
	
	iph = (struct iphdr *)p_indata;
	tcp = (struct tcphdr *)(p_indata + (iph->ihl<<2));
	data = (char *)(p_indata + iph->ihl * 4 + tcp->doff * 4 );
	org_len = data_len - iph->ihl * 4 - tcp->doff * 4;

//printf("******payload = %s\n\n", data);
	
	//if (!schedule_access(hw_addr))
	{	
		if(strstr(capurl,"http://") || strstr(capurl,"https://"))
			memcpy(loc,"Location: ", sizeof("Location: "));
		else
			memcpy(loc,"Location: http://", sizeof("Location: http://"));

		sprintf (buf_redirect, 
		"HTTP/1.1 302 Moved Temporarily\r\n%s\r\n%s\r\n%s\r\n%s%s%s\r\n\r\n",
		"Content-Length: 0",
		"Pragma: no-cache",
		"Cache-Control: private, max-age=0, no-cache",
		loc,
		capurl,
		"");
		
		redirect_flag = 1; 
	}
	
	if (redirect_flag)
	{
		p_outdata = p_indata;
		memset (p_outdata + iph->ihl * 4 + tcp->doff * 4, 0, org_len);
		
		tcp->doff = 5;
		tcp_doff = tcp->doff; 
		memcpy (p_outdata + iph->ihl * 4 + tcp->doff * 4, buf_redirect, strlen (buf_redirect));
		
		data_len = strlen (buf_redirect) + iph->ihl * 4 + tcp->doff * 4;
		
		iph = (struct iphdr *)p_outdata;
		tcp = (struct tcphdr *)(p_outdata + (iph->ihl<<2));
		data = (char *)(p_outdata + iph->ihl * 4 + tcp->doff * 4);
		
		saddr.s_addr = iph->saddr;
		daddr.s_addr = iph->daddr;
		iph->saddr = daddr.s_addr;
		iph->daddr = saddr.s_addr;
		
		//change ip header check sum
		iph->tot_len = htons (data_len);
		iph->check = 0;
		iph->check = AEI_in_cksum((unsigned short *)iph, iph->ihl * 4); 
		
		//change tcp checksum
		
		memset ((char *)tcp+12, 0, 2);
		tcp->res1 = 0;
		tcp->doff = tcp_doff; //set the header len(options may set it other than 5)
		tcp->psh = 1; 
		tcp->ack = 1;
		tcp->fin = 1;
		
		old_seq = tcp->seq;
		tcp->seq= ntohl (htonl (tcp->ack_seq));
		tcp->ack_seq = ntohl (htonl (old_seq) + org_len );
		
		old_port = tcp->dest;
		tcp->dest = tcp->source;
		tcp->source = old_port;
		
		tcp->check = 0;
		tcp->check = AEI_trans_check(IPPROTO_TCP, (char *)tcp, data_len - sizeof(struct iphdr), daddr, saddr);

#if defined(AEI_WLAN_URL_REDIRECT)                
                AEI_SendPacketWithDestMac(p_outdata, data_len, dmac,brname);
#else  
		AEI_SendPacketWithDestMac(p_outdata, data_len, dmac);
#endif
		//printf("******p_outdata: %s\n", p_outdata + iph->ihl * 4 + tcp->doff * 4 );
		status = nfq_set_verdict(qh, id, NF_DROP, data_len, p_outdata);
		
		if (status < 0)
			printf("send redirect error\n");
		
		return status;
	}       
	
	//printf("******p_indata: %s\n", p_indata + iph->ihl * 4 + tcp->doff * 4 );
	status = nfq_set_verdict(qh, id, NF_ACCEPT, data_len, p_indata);
	if (status < 0)
		printf("send redirect error\n");
	
	return status;
}

#if defined (AEI_WLAN_URL_REDIRECT)
void AEI_urltrim(char *url_redirect)
{       
    int size = 0; 

    if (!url_redirect)
        return;
    size = strlen(url_redirect);
    if(!size)
        return;

    while(1)
    {
        size = strlen(url_redirect);
        if (!size)
            break;
        if(url_redirect[size-1]!='/')
            break;
        else
            url_redirect[size-1]='\0';
    }
}
#endif

void AEI_get_lan_ip(char *addr)
{
	int fd;
	struct ifreq intf;
	
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("socket error!\n");
		return;
	}

	strcpy(intf.ifr_name, "br0");
	if(ioctl(fd, SIOCGIFADDR, &intf) != -1)
	{
		strcpy(addr, inet_ntoa(((struct sockaddr_in *)&intf.ifr_addr)->sin_addr));
	}
	close(fd);
	return;
}

void AEI_getCaptiveURLandIPAddr(char *fileName, char *url, char *ipAddr, int *flag)
{
	FILE *fp = NULL;
	char buf[2032] = {0};
	char *pTemp = NULL;
	
	if ((fp=fopen(fileName, "r")) != NULL)
	{
		fscanf(fp, "%s", buf);
		printf("fileName %s buf %s\n", fileName, buf);
		if (strcmp(buf, "None") != 0)
		{
			pTemp = strchr(buf, '_');
			*pTemp = ' ';
			*flag = 1;
			sscanf(buf, "%s %s", url, ipAddr);
			printf("url %s ip %s\n", url, ipAddr);
		}
		fclose(fp);
	}
}

static int AEI_initSocketAddress(struct sockaddr_ll* socket_address, char *ifName, int sockfd, unsigned char *src_mac, unsigned char *dest_mac)
{
    struct ifreq ifr;
    int i = 0;

    /* RAW communication */
    socket_address->sll_family = AF_PACKET; /* TX */
    socket_address->sll_protocol = 0; /* BIND */ /* FIXME: htons(ETH_P_ALL) ??? */

	memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        printf("Fail to get ifindex\n");
        return -1;
    }
    socket_address->sll_ifindex = ifr.ifr_ifindex; /* BIND, TX */

    socket_address->sll_hatype = 0; /* RX */
    socket_address->sll_pkttype = PACKET_OTHERHOST; /* RX */

    socket_address->sll_halen = ETH_ALEN; /* TX */

    /* MAC */
    for(i = 0; i < ETH_ALEN; i++)
    {
        socket_address->sll_addr[i] = dest_mac[i]; /* TX */
    }
    socket_address->sll_addr[6] = 0x00;/*not used*/
    socket_address->sll_addr[7] = 0x00;/*not used*/

    /* Get source MAC of the Interface we want to bind to */
    memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("Fail to get hw addr\n");
        return -1;
    }

    for(i = 0; i < ETH_ALEN; i++)
    {
        src_mac[i] = (unsigned char)ifr.ifr_hwaddr.sa_data[i];
    }

    //printf("Binding to %s: ifindex <%d>, protocol <0x%04X>...\n",
    //       ifName, socket_address->sll_ifindex, socket_address->sll_protocol);

    /* Bind to Interface */
    if(bind(sockfd, (struct sockaddr*)socket_address, sizeof(struct sockaddr_ll)) < 0)
    {
        printf("Binding error\n");
        return -1;
    }

    //printf("Done!\n");

    return 0;
}

#if defined(AEI_WLAN_URL_REDIRECT)
int AEI_SendPacketWithDestMac(char *data, int len, unsigned int *dmac,char *brname)
#else
int AEI_SendPacketWithDestMac(char *data, int len, unsigned char *dmac)
#endif
{
	int ret = -1;
#if defined(AEI_WLAN_URL_REDIRECT)
        char *ifName;
        if (!strcmp(brname,""))
                ifName = "br0";
        else
                ifName = brname;
#else
	char *ifName = "br0";
#endif

    int sockfd = -1;
    unsigned char src_mac[ETH_ALEN] = {'\0'};
    unsigned char dest_mac[ETH_ALEN] = {'\0'};	

	struct sockaddr_ll socket_address;
	
    memcpy(dest_mac, dmac, sizeof(dest_mac));
	//printf("\n finished reading mac address from arp table \n");
	//printf("\n the mac address is : %02x:%02x:%02x:%02x:%02x:%02x\n", dest_mac[0],dest_mac[1],dest_mac[2],dest_mac[3],dest_mac[4],dest_mac[5]);
	struct ethhdr raw_eth_hdr;
	char tx_buf[1512];
	
	if((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
	{
		printf("ERROR: Could not open Raw Socket");
		return ret;
	}
	//printf("\n finished socket cration \n");
	ret = AEI_initSocketAddress(&socket_address, ifName, sockfd, src_mac, dest_mac);
	//printf("\n finished initSocketAddress  \n");
	memset(&raw_eth_hdr,0,sizeof(struct ethhdr));
	memset(tx_buf,0,sizeof(tx_buf));

	int i;
	for (i=0; i<6; i++)
	{
		raw_eth_hdr.h_dest[i] = (u_char)dest_mac[i];
		raw_eth_hdr.h_source[i] = (u_char)src_mac[i];
	}
	raw_eth_hdr.h_proto = 0x0800;
	memcpy(tx_buf,&raw_eth_hdr,14);
	memcpy(tx_buf+sizeof(raw_eth_hdr),data,len);

	if ( sendto(sockfd, (void *)tx_buf, len+sizeof(raw_eth_hdr), 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0)
	{
		printf("\n sending data on Raw socket is failed \n");

	}
	//printf("\n great done with testing....   \n");
	close(sockfd);
	return 0;
}
#endif
