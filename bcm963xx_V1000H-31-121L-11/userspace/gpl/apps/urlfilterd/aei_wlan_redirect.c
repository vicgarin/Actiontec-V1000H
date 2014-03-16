#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(AEI_WLAN_URL_REDIRECT)
#define WLANURL       "/var/ssid%d_redirect_setting"
#define DEFAULT_URL_FLAG   "/var/ssid%d_onetimesRedirect"

/*
/var/wlan_redirect_url
ssid2:1|http://www.ontimesurl2.org|http://www.lockurl2.com|
ssid3:1|http://www.ontimesurl3.org|http://www.lockurl3.com|
ssid4:1|http://www.ontimesurl4.org|http://www.lockurl4.com|

/var/ssid2_onetimesRedirect
/var/ssid3_onetimesRedirect
/var/ssid3_onetimesRedirect
*/

int getOneTimesRedirect(int ssid,char *mac)
{
    FILE *fp = NULL;
    char OnetimeRedirectfile[256]={0};
    char onetimes[64]={0};
    sprintf(OnetimeRedirectfile,DEFAULT_URL_FLAG,ssid);
    if ((fp = fopen(OnetimeRedirectfile, "r")) != NULL)        
    {
    	while ( fgets(onetimes, sizeof(onetimes)-1, fp)!=NULL )    		
    	{
    	    if (strncmp(mac,onetimes,strlen(mac))==0){
                fclose(fp);
    	        return 1;
            }
    	}		
    	fclose(fp);
    }
    return 0;
}

/*enable: 0: enable, 1: onetimes 2: lockurl*/
int getSSIDSetting(const char *mac,int index,int *enable,char *onetimesurl,char *lockurl)
{
    FILE *fp = NULL;
    int i=0;
    char ssid_url[1024]={0};
    char *p=ssid_url;
    char ssidindex[8]={0};
    char file[64]={0};
    
    sprintf(ssidindex,"ssid%d",index);
    sprintf(file,WLANURL,index);
    if ((fp = fopen(file, "r")) != NULL)        
    {
    	while(fgets(ssid_url, sizeof(ssid_url)-1, fp)!=NULL )
    	{
    	    if(!strncmp(ssid_url,ssidindex,5)){
    	    	//...
    	    	*enable = ssid_url[6]-48;
    	    	char *one=strstr(p+8,"|");
    	    	if (one!=NULL){
    	            strncpy(onetimesurl,p+8,one-p-8);   
    	            char *lock = strstr(one+1,"|");
    	            strncpy(lockurl,one+1,lock-one-1);
    	    	}
    	    	break;
    	    }
    	}
    	fclose(fp);
    }
    return index;
}

void writeUrlRedirect(int index,int type,char *url_default,char *url_lock)
{
    FILE *File;
    char line[256]={0};
    char file[64]={0};
    sprintf(file,WLANURL,index);
    File = fopen(file, "w");
    if (!File){
         printf("%s is created now.\n",WLANURL);
         return;
    }
    sprintf(line,"ssid%d:%d|%s|%s|\n",index,type,url_default,url_lock);
    fputs(line, File);
    
    fclose(File);

}
void addmaclist(int ssid,char *mac,char *filename)
{
    FILE *fp;
    char file[64]={0};
    char mac_tmp[128]={0};
    sprintf(file,filename,ssid);
    fp = fopen(file, "w");
    if (!fp){
        printf("file open error,create it now\n");
        fp = fopen(file,"w");
        fputs(mac,fp);
        fputs("\n",fp);
        fclose(fp);
        return;
    }
    while(fgets(mac_tmp, sizeof(mac_tmp)-1, fp)!=NULL )
    {
    	if(!strncmp(mac,mac_tmp,strlen(mac))){
    	    fclose(fp);
      	    printf("mac %s existed\n",mac);
    	    return;
    	}
    }
    fputs(mac,fp);
    fputs("\n",fp);
    printf("mac %s add ok\n",mac);
    fclose(fp);
}
int toupperStr(char *src,char *dst) 
{  
    int   length,   i;  
    
    length   =   strlen(src);  
    for   (i=0;   i <length;   i++)   {
            dst[i]   =   toupper(src[i]);  
    }  
    
    return   0;  
}   

int scanFileForMAC(char *fname, char *mac) {
   char buf[32+60];
   FILE *fp = fopen(fname, "r");
   char mac_upper[64]={0};
   toupperStr(mac,mac_upper);
   if ( fp != NULL ) {
      for (;fgets(buf, sizeof(buf), fp);) {
         if (strstr(buf, mac_upper)) {
            fclose(fp);
            return 1;
         }
      }
      fclose(fp);
   }
   return 0;
}

void getIfname(char *brctl_file,char *wlifname,char *brname)
{
    char buf[32+60];
    FILE *fp = fopen(brctl_file, "r");
    if ( fp != NULL ) {
        for (;fgets(buf, sizeof(buf), fp);) {
            if (strstr(buf, wlifname)) {
                if(!strncmp(buf,"br",2))
                {
                    strncpy(brname,buf,3);
                    fclose(fp);
                    return;
                }
            }
        }
        fclose(fp);
    }
    strcpy(brname,"");
}

void getDevice(const char *mac,int *ssidindex,char *ifname)
{
	int i=0;
	char cmd[80]={0};
	char wl_assocfile[80]; 
	char *brctl_file="/var/brctl_show";
	char wlifname[16]={0};
	char brname[16]={0};
	for(i=1;i<4;i++){
//		sprintf(cmd, "wlctl -i wl0.%d assoclist > /var/wl%d_assoc_url", i, i);
//		system(cmd);
		memset(wl_assocfile,0x00,sizeof(wl_assocfile));
	        sprintf(wl_assocfile, "/var/wl%d_assoc", i);
	        if(scanFileForMAC(wl_assocfile,mac)){
	    	    sprintf(wlifname,"wl0.%d",i);
	    	    *ssidindex = i+1;
	    	    break;
	        }
	}
	memset(cmd,0x00,sizeof(cmd));
	sprintf(cmd,"brctl show > %s",brctl_file);
	system(cmd);
	if (strcmp(wlifname,"")){
	    getIfname(brctl_file,wlifname,brname);
            strcpy(ifname,brname);
	}
}
void AEI_processSSID234UrlRedirect(char *mac,char *url,int size,char *ifname,char *match)
{       
        int enable;
        char onetimesurl[256]={0};
        char lockurl[256]={0};
        int  ssidindex=0;
        char onetimeredirectmac[256]={0};

        getDevice(mac,&ssidindex,ifname);
        if (!ssidindex ){
//	    printf("Can't get index=%d for mac:%s\n",ssidindex,mac);
            return;
	}
	if (!strcmp(ifname,""))
		strcpy(ifname,"br0");
        getSSIDSetting(mac,ssidindex,&enable,onetimesurl,lockurl);
//        printf("[processSSID234UrlRedirect]ssidindex=%d\n",ssidindex);
//        printf("[processSSID234UrlRedirect]ifname=%s\n",ifname);
//        printf("[processSSID234UrlRedirect]enable=%d\n",enable);
//        printf("[processSSID234UrlRedirect]onetimesurl=%s\n",onetimesurl);
//        printf("[processSSID234UrlRedirect]lockurl=%s\n",lockurl);
        if (enable){
                if(enable==2){
                        strncpy(url,lockurl,size);
                }
                else if(enable==1 && (strstr(match,"GET / HTTP/")||strstr(match,"get HTTP/")))
                {
                        if(getOneTimesRedirect(ssidindex,mac)==0){
                                strncpy(url,onetimesurl,size);
                                sprintf(onetimeredirectmac,DEFAULT_URL_FLAG,ssidindex);
                                addmaclist(ssidindex,mac,onetimeredirectmac);
                        }else
                                strcpy(url,"");
                }     
        } 
}
#endif

