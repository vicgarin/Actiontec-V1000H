#if defined(AEI_VDSL_CUSTOMER_NCS)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <net/if.h>
#include "cms_util.h"
#include "oal.h"
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/file.h>
#include <sys/types.h>
#include <dirent.h>
#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
#include "bcmTag.h" /* in shared/opensource/include/bcm963xx, for FILE_TAG */
#include "board.h" /* in bcmdrivers/opensource/include/bcm963xx, for BCM_IMAGE_CFE */
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
#include "cms_boardioctl.h"
#endif
#define READ_BUF_SIZE 128


int AEI_get_value_by_file(char *file, int size, char *value)
{
    FILE *fp = NULL;

    if((fp = fopen(file, "r")) != NULL)
    {
        fgets(value, size - 1 , fp);
        fclose(fp);
        return 1;
    }
    else
        return 0;
}


UINT16 AEI_get_interface_mtu(char *ifname)
{
   struct ifreq ifr;
   int sockfd, err;

   if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) <= 0 )
   {
      return -1;
   }

   strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
   err = ioctl(sockfd, SIOCGIFMTU, (void*)&ifr);
   close(sockfd);

   if(err < 0)
       return 1500; //default MTU
   else
       return ifr.ifr_mtu;
}
void AEI_createFile(char *filename, char *content)
{
    FILE *fp=NULL;
    if((fp=fopen(filename,"w"))==NULL)
   {//file exist 
        printf("write %s failed\n",filename);
        return;
    }
    if(content != NULL)
    fprintf(fp, "%s", content);
    fclose(fp);
}


int AEI_removeFile(char *filename)
{
   int ret = 0;    
    if (remove(filename) != 0) {
            cmsLog_error("Could not remove %s file=%d", filename);
            ret = -1;
    }
    return ret;
}

/* 0- file exist*/
int AEI_isFileExist(char *filename)
{
   int ret = 0;    
   int fileFlag=access(filename,F_OK);
   return ret;
}
int AEI_get_mac_addr(char *ifname, char *mac)
{
	int fd, rtn;
	struct ifreq ifr;
	unsigned char arp[6];

	if( !ifname || !mac ) {
		return -1;
	}
	fd = socket(AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) {
		perror("socket");
		return -1;
	}
	ifr.ifr_addr.sa_family = AF_INET;    
	strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1 );

	if ((rtn = ioctl(fd, SIOCGIFHWADDR, &ifr)) == 0)
	{
         sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",
                 (UINT8)(ifr.ifr_hwaddr.sa_data[0]),(UINT8)(ifr.ifr_hwaddr.sa_data[1]),
                 (UINT8)(ifr.ifr_hwaddr.sa_data[2]),(UINT8)(ifr.ifr_hwaddr.sa_data[3]),
                 (UINT8)(ifr.ifr_hwaddr.sa_data[4]),(UINT8)(ifr.ifr_hwaddr.sa_data[5]));
	}
	close(fd);
	return rtn;
}

int AEI_convert_space(char *src,char *dst)
{
    int i=0;
    int len=0;
    int j=0;
    if(src==NULL || dst==NULL)
        return -1;
    len=strlen(src);
    for(i=0;i<len;i++)
    {
        if(src[i]==' ')
        {
            dst[j]='%';
            j++;
            sprintf(&dst[j],"20");
            j+=2;
        }
        else
        {
            dst[j]=src[i];
            j++;
        }
    }
    return 0;
}


int AEI_convert_spec_chars(char *src,char *dst)
{
    int len=0;
    int i=0;
    int j=0;
    if(src==NULL || dst == NULL)
        return -1;

    len=strlen(src);
    for(i=0;i<len;i++)
    {
        switch(src[i])
        {
            case ':':
            case ';':
            case '|':
            case '"':
            case ' ':
            case '+':    
            case '\'':
            case '\\':
                dst[j]='%';
                j++;
                sprintf(&dst[j],"%02x",src[i]);
                j+=2;
                break;
            default:
                dst[j]=src[i];
                j++;    
        }
    }
    return 0;
}

/* Encode the output value base on microsoft standard*/
char* AEI_SpeciCharEncode(char *s, int len)
{
    int c;
    int n = 0;
    char t[4096]={0};
    char *p;
	
    p=s;
    memset(t,0,sizeof(t));
	
    if (s == NULL) {
        cmsLog_error("The transfer object is null");
        return s;
    }
    while ((c = *p++)&& n < 4096) {
        /*
        Yuki: Special characters |,/()+ \are used by join characters, also need Encode them.
        */
        
        if (!strchr("<>\"'%;)(&+|,/\\", c)) {
            t[n++] = c;
        } else if (n < 4096-5) {
            t[n++] = '&';
            t[n++] = '#';
            /*
            Yuki:| encode is &#124;
            */
            if(strchr("|",c)){
                t[n++] = (c/100) + '0';
                t[n++] = ((c%100)/10) + '0';
                t[n++] = ((c%100)%10) + '0';
            }else{
                t[n++] = (c/10) + '0';
                t[n++] = (c%10) + '0';
            }
            t[n++] = ';';
        } else {
            cmsLog_error("The Array size overflow Exception");
            return s;;
        }
    }
    t[n] = '\0';
    if (n <= len && n < 4096)
    {
        memset(s,0,len);
        strncpy(s, t,  n);
    }
    else
        cmsLog_error("The Array size overflow Exception");
    return s;
}

static void remove_delimitor( char *s)
{
    char *p1, *p2;

    p1 = p2 = s;
    while ( *p1 != '\0' || *(p1+1) != '\0') {
        if (*p1 != '\0') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

}


pid_t* find_pid_by_name( char* pidName)
{
        DIR *dir;
        struct dirent *next;
        pid_t* pidList=NULL;
        int i=0;

        /*FILE *status */
        FILE *cmdline;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        /* char name[READ_BUF_SIZE]; */
                
        dir = opendir("/proc");
        if (!dir) {
                printf("cfm:Cannot open /proc");
                return NULL;
        }

        while ((next = readdir(dir)) != NULL) {
                /* re-initialize buffers */
                memset(filename, 0, sizeof(filename));
                memset(buffer, 0, sizeof(buffer));  

                /* Must skip ".." since that is outside /proc */
                if (strcmp(next->d_name, "..") == 0)
                        continue;

                /* If it isn't a number, we don't want it */
                if (!isdigit(*next->d_name))
                        continue;

                /* sprintf(filename, "/proc/%s/status", next->d_name); */
                /* read /porc/<pid>/cmdline instead to get full cmd line */
                sprintf(filename, "/proc/%s/cmdline", next->d_name);
                if (! (cmdline = fopen(filename, "r")) ) {
                        continue;
                }
                if (fgets(buffer, READ_BUF_SIZE-1, cmdline) == NULL) {
                        fclose(cmdline);
                        continue;
                }
                fclose(cmdline);

                /* Buffer should contain a string like "Name:   binary_name" */
                /*sscanf(buffer, "%*s %s", name);*/
                /* buffer contains full commandline params separted by '\0' */
                remove_delimitor(buffer);
                if (strstr(buffer, pidName) != NULL) {
                        pidList=realloc( pidList, sizeof(pid_t) * (i+2));
                        if (!pidList) {
                                printf("cfm: Out of memeory!\n");
				closedir(dir);
                                return NULL;
                        }
                        pidList[i++]=strtol(next->d_name, NULL, 0);
                }
        }
        closedir(dir);

        if (pidList)
                pidList[i]=0;
        else if ( strcmp(pidName, "init")==0) {
                /* If we found nothing and they were trying to kill "init",
                 * guess PID 1 and call it good...  Perhaps we should simply
                 * exit if /proc isn't mounted, but this will do for now. */
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=1;
        } else {
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=-1;
        }
        return pidList;
}


int AEI_GetPid(char * command)
{
    char cmdline[128], *p1, *p2;
    pid_t *pid = NULL;
    int ret = 0;

    p1 = command;
    p2 = cmdline;
    while ( *p1 != '\0') {
        if (*p1 != ' ') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

    pid = find_pid_by_name(cmdline);
    if ( pid != NULL ) {
       ret = (int)(*pid);
       free(pid);
    }

    return ret;
}


#if defined(AEI_CONFIG_JFFS) && defined(AEI_VDSL_CUSTOMER_CENTURYLINK_C1000A)
CmsRet AEI_writeDualPartition(char *imagePtr, UINT32 imageLen, void *msgHandle, int partition)
{
   CmsImageFormat format;
   CmsRet ret;
   
   if ((format = cmsImg_validateImage(imagePtr, imageLen, msgHandle)) == CMS_IMAGE_FORMAT_INVALID)
   {
      ret = CMSRET_INVALID_IMAGE;
   }
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
   else if(format == CMS_IMAGE_FORMAT_CORRUPTED)
   {
      ret = CMSRET_INVALID_IMAGE;
   }
#endif
   else if(format == CMS_IMAGE_FORMAT_FLASH)
   {
        ret = devCtl_boardIoctl(BOARD_IOCTL_PARTITION_WRITE,
									   partition,
									   imagePtr,
									   imageLen-TOKEN_LEN,
									   0, 0);
   }
   return ret;
}
#endif
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
int AEI_save_syslog()
{
    return devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE,SYSLOGONREBOOT,NULL,0,0,0);
}
#endif
#endif
