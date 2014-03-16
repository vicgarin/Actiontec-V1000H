#include "mdm_object.h"
int AEI_convert_space(char *src,char *dst);
int AEI_get_mac_addr(char *ifname, char *mac);
int AEI_convert_spec_chars(char *src,char *dst);
char* AEI_SpeciCharEncode(char *s, int len);
CmsRet AEI_addQueueByObj(char *intfName, QMgmtQueueObject *old);
UINT16 AEI_get_interface_mtu(char *ifname);
int AEI_get_value_by_file(char *file, int size, char *value);
#if defined(AEI_VDSL_CUSTOMER_CENTURYLINK)
int AEI_save_syslog();
#endif

