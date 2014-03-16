/***********************************************************************
 *
 *  Copyright (c) 2011  Actiontec Electronics Inc.
 *  All Rights Reserved
 *
 *  This file is to store all functions that developed by Actiontec Electronics
 *  in addition to routines provided by Broadcom. All additional routines that 
 *  are missing from nslookup.c file will locate in this file. 
 *
 ************************************************************************/

#if defined(AEI_VDSL_SMARTLED)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>

#include "cms_msg.h"
#include "cms_util.h"

#define MAXPACKET 65468
#define __NETDB_MAXADDRS 35
#define __NETDB_MAXALIASES 35
#define pthread_ipaddr_type int 
#define HOST_BUFSIZE 4096
#define ALIGN(p, t) ((char *)(((((long)(p) - 1) / sizeof(t)) + 1) * sizeof(t)))
#define SP(p, t, n) (ALIGN(p, t) + (n) * sizeof(t))

struct __res_state _res = {
    RES_TIMEOUT,               	/* retransmition time interval */
    4,                         	/* number of times to retransmit */
    RES_DEFAULT,		/* options flags */
    1,                         	/* number of name servers */
};

struct	res_data {
    char *buf;
    struct __res_state state;
    int errval;
    int sock;
};

typedef union {
    HEADER hdr;
    unsigned char buf[MAXPACKET];
} querybuf;

typedef union {
    long al;
    char ac;
} align;


/* Performs global initialization. */
struct res_data *globledata = NULL;
static struct __res_state start;
char dns_txt[1024] = {0};

struct res_data *_res_init()
{
    /* Initialize thread-specific data for this thread if it hasn't
     * been done already. */
    if (globledata)
        return globledata;
    else
    {
        globledata = (struct res_data *) malloc(sizeof(struct res_data));
        if (globledata == NULL)
            return NULL;
		
        globledata->buf = NULL;
        globledata->state = start;
        globledata->errval = NO_RECOVERY;
        globledata->sock = -1;
	
        return globledata;
    }
}

struct __res_state *_res_status()
{
    struct res_data *data;
	
    data = _res_init();
    return (data) ? &data->state : NULL;
}

static int qcomp(const void *arg1, const void *arg2)
{
    const struct in_addr **a1 = (const struct in_addr **) arg1;
    const struct in_addr **a2 = (const struct in_addr **) arg2;
    struct __res_state *state = _res_status();

    int pos1, pos2;
	
    for (pos1 = 0; pos1 < state->nsort; pos1++) {
        if (state->sort_list[pos1].addr.s_addr ==
            ((*a1)->s_addr & state->sort_list[pos1].mask))
            break;
    }
    for (pos2 = 0; pos2 < state->nsort; pos2++) {
        if (state->sort_list[pos2].addr.s_addr ==
            ((*a2)->s_addr & state->sort_list[pos2].mask))
            break;
    }
    return pos1 - pos2;
}

int __dn_skipname(comp_dn, eom)
    const u_char *comp_dn, *eom;
{
    register u_char *cp;
    register int n;

    cp = (u_char *)comp_dn;
    while (cp < eom && (n = *cp++)) {
        /*
         * check for indirection
         */
        switch (n & INDIR_MASK) {
            case 0:			/* normal case, n == len */
                cp += n;
                continue;
            case INDIR_MASK:		/* indirection */
                cp++;
                break;
            default:			/* illegal type */
                return (-1);
        }
        break;
    }

    if (cp > eom)
        return -1;

    return (cp - comp_dn);
}

u_short
_getshort(msgp)
    register const u_char *msgp;
{
    register u_short u;

    GETSHORT(u, msgp);
    return (u);
}

struct hostent *_res_parse_answer(querybuf *answer, int anslen, int iquery,
                                  struct hostent *result, char *buf,
                                  int bufsize, int *errval)
{
    struct res_data *data = _res_init();
    register HEADER *hp;
    register u_char *cp;
    register int n;
    u_char *eom;
    char *aliases[__NETDB_MAXALIASES], *addrs[__NETDB_MAXADDRS];
    char *bp = buf, **ap = aliases, **hap = addrs;
    int type, class, ancount, qdcount, getclass = C_ANY, iquery_done = 0;
	
    eom = answer->buf + anslen;
    /*
     * find first satisfactory answer
     */
    hp = &answer->hdr;
    ancount = ntohs(hp->ancount);
    qdcount = ntohs(hp->qdcount);
    bp = buf;
    cp = answer->buf + sizeof(HEADER);

    /* Read in the hostname if this is an address lookup. */
    if (qdcount) {
        if (iquery) {
            if ((n = dn_expand((u_char *) answer->buf,
                               (u_char *) eom, (u_char *) cp, (u_char *) bp,
                                bufsize - (bp - buf))) < 0) {
                *errval = NO_RECOVERY;
                return ((struct hostent *) NULL);
            }
            cp += n + QFIXEDSZ;
            result->h_name = bp;
            bp += strlen(bp) + 1;
        } else {
            cp += __dn_skipname(cp, eom) + QFIXEDSZ;
        }
        while (--qdcount > 0)
            cp += __dn_skipname(cp, eom) + QFIXEDSZ;
    } else if (iquery) {
        *errval = (hp->aa) ? HOST_NOT_FOUND : TRY_AGAIN;
        return ((struct hostent *) NULL);
    }

    /* Read in the answers. */
    *ap = NULL;
    *hap = NULL;
    while (--ancount >= 0 && cp < eom) {
        if ((n = dn_expand((u_char *) answer->buf, (u_char *) eom,
                           (u_char *) cp, (u_char *) bp,
                           bufsize - (bp - buf))) < 0)
            break;

        cp += n;
        type = _getshort(cp);
        cp += sizeof(u_short);
        class = _getshort(cp);
        cp += sizeof(u_short) + sizeof(pthread_ipaddr_type);
        n = _getshort(cp);
        cp += sizeof(u_short);
        if (type == T_CNAME) {
            cp += n;
            if (ap >= aliases + __NETDB_MAXALIASES - 1)
                continue;
            *ap++ = bp;
            bp += strlen(bp) + 1;
            continue;
        }
        if (iquery && type == T_PTR) {
            if ((n = dn_expand((u_char *) answer->buf, (u_char *) eom,
                               (u_char *) cp, (u_char *) bp,
                                bufsize - (bp - buf))) < 0)
                break;

            cp += n;
            result->h_name = bp;
            bp += strlen(bp) + 1;
            iquery_done = 1;
            break;
        }
		
        if (iquery || type == 0x0010)	{
            //printf("expected answer type %d, size %d\n", type, n);
            memset(dns_txt, 0, sizeof(dns_txt));
            strncpy(dns_txt, cp, n);
            dns_txt[n] = '\0';
            //printf("dns txt %s\n", dns_txt);
            cp += n;
        }
		
        if (hap > addrs) {
            if (n != result->h_length) {
                cp += n;
                continue;
            }
            if (class != getclass) {
                cp += n;
                continue;
            }
        } else {
            result->h_length = n;
            getclass = class;
            result->h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
            if (!iquery) {
                result->h_name = bp;
                bp += strlen(bp) + 1;
            }
        }
		
        bp = ALIGN(bp, char *);
        //bp = ALIGN(bp, pthread_ipaddr_type);
        if (bp + n >= buf + bufsize) {
            errno = ERANGE;
            return NULL;
        }
        memcpy(bp, cp, n);
        cp += n;
        if (hap >= addrs + __NETDB_MAXADDRS - 1)
            continue;
        *hap++ = bp;
        bp += n;
        cp += n;
    }

    if (hap > addrs || iquery_done) {
        *ap++ = NULL;
        *hap++ = NULL;
        if (data->state.nsort)
            qsort(addrs, hap - addrs, sizeof(struct in_addr), qcomp);

        if (SP(bp, char *, (hap - addrs) + (ap - aliases)) > buf + bufsize) {
            errno = ERANGE;
            return NULL;
        }
        result->h_addr_list = (char **) ALIGN(bp, char *);
        memcpy(result->h_addr_list, addrs, (hap - addrs) * sizeof(char *));
        result->h_aliases = result->h_addr_list + (hap - addrs);
        memcpy(result->h_aliases, aliases, (ap - aliases) * sizeof(char *));
        return result;
    } else {
        *errval = TRY_AGAIN;
        return NULL;
    }
}

static struct hostent *fake_hostent(const char *hostname, struct in_addr addr,
                                    struct hostent *result, char *buf,
                                    int bufsize, int *errval)
{
    int len = strlen(hostname);
    char *name, *addr_ptr;

    if (SP(SP(SP(buf, char, len + 1), addr, 1), char *, 3) > buf + bufsize) {
        errno = ERANGE;
        return NULL;
    }

    /* Copy faked name and address into buffer. */
    strcpy(buf, hostname);
    name = buf;
    buf = ALIGN(buf + len + 1, addr);
    *((struct in_addr *) buf) = addr;
    addr_ptr = buf;
    buf = ALIGN(buf + sizeof(addr), char *);
    ((char **) buf)[0] = addr_ptr;
    ((char **) buf)[1] = NULL;
    ((char **) buf)[2] = NULL;

    result->h_name = name;
    result->h_aliases = ((char **) buf) + 2;
    result->h_addrtype = AF_INET;
    result->h_length = sizeof(addr);
    result->h_addr_list = (char **) buf;

    return result;
}

static struct hostent *gethostbyname_txt_r(const char *hostname, struct hostent *result,
                                           char *buf, int bufsize, int *errval)
{
    struct in_addr addr;
    querybuf qbuf;
    const char *p;
    int n;

    /* Default failure condition is not a range error and not recoverable. */
    errno = 0;
    *errval = NO_RECOVERY;
	
    /* Check for all-numeric hostname with no trailing dot. */
    if (isdigit(hostname[0])) {
        p = hostname;
        while (*p && (isdigit(*p) || *p == '.'))
            p++;

        if (!*p && p[-1] != '.') {
            /* Looks like an IP address; convert it. */
            if (inet_aton(hostname, &addr) == -1) {
                *errval = HOST_NOT_FOUND;
                return NULL;
            }
            return fake_hostent(hostname, addr, result, buf, bufsize, errval);
        }
    }
	
    /* Do the search. */
    //n = res_search(hostname, C_IN, T_A, qbuf.buf, sizeof(qbuf));
    n = res_search(hostname, C_IN, 0x0010, qbuf.buf, sizeof(qbuf));
    if (n >= 0)
        return _res_parse_answer(&qbuf, n, 0, result, buf, bufsize, errval);	
    else
        return NULL;
}

struct hostent *AEI_gethostbyname_txt(const char *hostname)
{
    struct res_data *data = _res_init();

    if (!data)
        return NULL;
    if (!data->buf) {
        data->buf = malloc(sizeof(struct hostent) + HOST_BUFSIZE);
        if (!data->buf) {
            errno = 0;
            data->errval = NO_RECOVERY;
            return NULL;
        }
    }
    return gethostbyname_txt_r(hostname, (struct hostent *) data->buf,
                               data->buf + sizeof(struct hostent), HOST_BUFSIZE,
                               &data->errval);
}

void AEI_sendTxtRecordForNslookup(void *msgHandle)
{
   char buf[sizeof(CmsMsgHeader) + 512] = {0};
   CmsMsgHeader *msg = (CmsMsgHeader *)buf;
   char *txtRecord = (char *)(msg + 1);
   int strLength = 0;
   CmsRet ret;

   if (dns_txt[0] != '\0')
       strLength = sprintf(txtRecord, "%s", dns_txt);
   else
       strLength = sprintf(txtRecord, "%s", "NSLookUp_NoResponse");

   msg->type = CMS_MSG_NSLOOKUP_TXT_RDATA;
   msg->src = EID_NSLOOKUP;
   msg->dst = EID_SSK;
   msg->flags_event = 1;
   msg->dataLength = strLength;

   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_NSLOOKUP_STATE_CHANGED to SSK, txtRecord : %s", txtRecord);
   }
   else
   {
      cmsLog_debug("sent out CMS_MSG_NSLOOKUP_TXT_RDATA to SSK txtRecord : %s", txtRecord);
   }
   cmsMsg_cleanup(&msgHandle);
   return;
}

void AEI_cleanup()
{
    if (globledata)
    {
        if (globledata->buf != NULL)
            free(globledata->buf);
        free(globledata);
    }
}
#endif /* AEI_VDSL_SMARTLED */

#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) || defined(AEI_VDSL_CUSTOMER_BELLALIANT)
#include "cms_msg.h"
#include "cms_util.h"
void AEI_sendNslookupResult(void *msgHandle)
{
   char buf[sizeof(CmsMsgHeader) + 512] = {0};
   CmsMsgHeader *msg = (CmsMsgHeader *)buf;
   char *txtRecord = (char *)(msg + 1);
   int strLength = 0;
   CmsRet ret;

   strLength = sprintf(txtRecord, "%s", "NSLookUp_Result");

   msg->type = CMS_MSG_NSLOOKUP_RESULT;
   msg->src = EID_NSLOOKUP;
   msg->dst = EID_SSK;
   msg->flags_event = 1;
   msg->dataLength = strLength;

   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_NSLOOKUP_STATE_CHANGED to SSK, txtRecord : %s", txtRecord);
   }
   else
   {
      cmsLog_debug("sent out CMS_MSG_NSLOOKUP_TXT_RDATA to SSK txtRecord : %s", txtRecord);
   }
   cmsMsg_cleanup(&msgHandle);
   return;
}

#endif /* #if defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A) */

