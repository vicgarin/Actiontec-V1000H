#ifndef __AEI_TIMER_H__
#define __AEI_TIMER_H__
#include "cms_tmr.h"

#ifdef DMP_PERIODICSTATSBASE_1
void AEI_cmsTmr_cancel2(void *handle, CmsEventHandler func, void *ctxData);
UBOOL8 AEI_cmsTmr_isEventPresent2(const void *handle, CmsEventHandler func, void *ctxData);
CmsRet AEI_cmsTmr_set2(void *handle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);
UINT32 AEI_cmsTmr_Event_TimeRemaining(const void *handle, CmsEventHandler func, void *ctxData);
#endif

#endif  /* __AEI_TIMER_H__ */