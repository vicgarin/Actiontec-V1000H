/* FILE NAME: tsl_common.h
 * PURPOSE: the header file 
 * NOTES:
 * REASON:
 *      DESCRIPTION:  
 *      CREATOR: Yunhai Zhu  
 *      DATE: 2010/9/1
 */

#ifndef TSL_COMMON_H
#define TSL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
 *
 *      Define included files according the platform 
 *
 ************************************************************/
#if defined _WIN32
#include <sys/timeb.h>
#include <time.h>
#include <direct.h>
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <limitr.h>

#elif defined _LINUX
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#if ( !defined (_BHR2 ) && !defined( SUPPORT_BHR1) )
#include <semaphore.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>


#elif defined _VXWORK
#include <vxworks.h>
#include <stdio.h>
#include <string.h>
#include <taskLib.h>
#include <msgQLib.h>
#include <sysLib.h>
#include <stdio.h>
#include <ioLib.h>
#include <usrlib.h>
#include <stat.h>
#include <fcntl.h>
#include <time.h>
#include <tftplib.h>
#include <sys/socket.h>
#include <inetLib.h>
#endif


/***********************************************************
 *
 *                     Define common structure
 *
 ************************************************************/
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#ifdef _WIN32
#define inline
#define INLINE
#define __FUNCTION__ NULL
#endif

#ifdef _VXWORK
#define inline 
#endif

#ifdef _SOLARIS
#define __FUNCTION__ NULL
#endif

#if 0
#ifndef DEBUG
#ifdef _DEBUG
#define DEBUG(act) act
#else
#define DEBUG(act)
#endif
#endif
#endif

#define NAME_SIZE               64
#define MAX_NAME_SIZE           128
#define MIN_BUF_SIZE            128
#define BUF_SIZE                1024
#define MAX_BUF_SIZE            4096
#define MAX_UINT_SIZE           4294967296
#define MAX_INT_SIZE            2147483648
#define MAX_USHORT_SIZE         65536
#define MAX_SHORT_SIZE          32768

typedef unsigned long           tsl_u32_t;
typedef unsigned short          tsl_u16_t;
typedef unsigned char           tsl_u8_t;
typedef signed   long           tsl_32_t;
typedef signed   short          tsl_16_t;
typedef signed   char           tsl_8_t;

typedef unsigned int            tsl_uint_t;
typedef signed   int            tsl_int_t;
typedef unsigned char           tsl_byte_t;
typedef unsigned long           tsl_ulong_t;
typedef signed   long           tsl_long_t;

typedef char                    tsl_char_t;
typedef unsigned char           tsl_uchar_t;

typedef void                    tsl_void_t;
typedef void                    *tsl_pvoid_t;

typedef size_t                  tsl_size_t;


#define TSL_B_FALSE 0
#define TSL_B_TRUE  1  
typedef int tsl_bool;

typedef enum{
        tsl_b_true               = 1,
        tsl_b_false              = 0
} tsl_bool_t;


#define TSL_RV_SUC               0
#define TSL_RV_ERR               -10
#define TSL_RV_ERR_PARM          -11
#define TSL_RV_FAIL              -20
#define TSL_RV_FAIL_MEM          -21   
#define TSL_RV_FAIL_FUNC         -22
typedef int tsl_rv;


typedef enum{
        tsl_rv_suc               =  0,
        tsl_rv_err               = -10,
        tsl_rv_err_parm          = -11,
        tsl_rv_fail              = -20,
        tsl_rv_fail_mem          = -21,
        tsl_rv_fail_func         = -22      
} tsl_rv_t;



/***********************************************************
 *
 *              Define Common Macro 
 *
 ************************************************************/
/*return with rv*/
#define TSL_VASSERT_RV(cond, rv)                                 \
{                                                                \
        if (!(cond)){                                            \
                printf("ASSERT:(%s),%s %s line %d\n",            \
                       #cond, __FUNCTION__, __FILE__, __LINE__); \
                return rv;                                       \
        }                                                        \
}

#define TSL_VASSERT_RV_ACT(cond, rv, action)                     \
{                                                                \
        if (!(cond)){                                            \
                printf("ASSERT:(%s),%s %s line %d\n",            \
                       #cond, __FUNCTION__, __FILE__, __LINE__); \
                {action;}                                        \
                return rv;                                       \
        }                                                        \
}

/*return with void*/
#define TSL_VASSERT_VRV(cond)                                    \
{                                                                \
        if (!(cond)){                                            \
               printf("ASSERT:(%s),%s %s line %d\n",             \
                       #cond, __FUNCTION__, __FILE__, __LINE__); \
               return ;                                          \
        }                                                        \
}

#define TSL_VASSERT_VRV_ACT(cond, action)                        \
{                                                                \
        if (!(cond)){                                            \
                printf("ASSERT:(%s),%s %s line %d\n",            \
                       #cond, __FUNCTION__, __FILE__, __LINE__); \
                {action;}                                        \
                return ;                                         \
        }                                                        \
}

/* do not return*/
#define TSL_VASSERT_NRV(cond)                                    \
{                                                                \
        if (!(cond)){                                            \
                printf("ASSERT:(%s),%s %s,line %d\n",            \
                       #cond, __FUNCTION__, __FILE__, __LINE__); \
        }                                                        \
}

#define TSL_VASSERT_NRV_ACT(cond, action)                        \
{                                                                \
        if (!(cond)){                                            \
                printf("ASSERT:(%s),%s %s,line %d\n",            \
                       #cond, __FUNCTION__, __FILE__, __LINE__); \
                {action;}                                        \
        }                                                        \
}


/*default TSL_VASSERT macro used to judge the parameter is correct or not*/
#define TSL_VASSERT(cond)                    TSL_VASSERT_RV(cond, TSL_RV_ERR_PARM)
#define TSL_VASSERT_ACT(cond, action)        TSL_VASSERT_RV_ACT(cond, TSL_RV_ERR_PARM, action)

/*default TSL_VASSERT macro used to judge the function called is correct or not*/
#define TSL_FVASSERT(cond)                   TSL_VASSERT_RV(cond, TSL_RV_FAIL_FUNC)
#define TSL_FVASSERT_RV(cond, rv)            TSL_VASSERT_RV(cond, rv)
#define TSL_FVASSERT_ACT(cond, act)          TSL_VASSERT_RV_ACT(cond, TSL_RV_FAIL_FUNC, act)
#define TSL_FVASSERT_RV_ACT(cond, rv, act)   TSL_VASSERT_RV_ACT(cond, rv, act)
#define TSL_FVASSERT_NRV(cond)               TSL_VASSERT_NRV(cond)
#define TSL_FVASSERT_NRV_ACT(cond, act)      TSL_VASSERT_NRV_ACT(cond, act)
#define TSL_FVASSERT_VRV(cond)               TSL_VASSERT_VRV(cond)
#define TSL_FVASSERT_VRV_ACT(cond, act)      TSL_VASSERT_VRV_ACT(cond, act)


#define _MALLOC(size)            malloc(size) 
#define _CALLOC(size, size_t)    calloc(size, size_t)
#define _FREE(ptr)               free(ptr)


#define TSL_SMALLOC_RV(ptr, type, size, rv)                                        \
{                                                                                  \
        TSL_VASSERT_RV((ptr = (type *)_MALLOC(size)) != NULL, rv);                 \
        memset(ptr, 0, size);                                                      \
        TSL_VASSERT_RV(ptr != NULL, rv);                                           \
}

#define TSL_SMALLOC_RV_ACT(ptr, type, size, rv, act)                               \
{                                                                                  \
        TSL_VASSERT_RV_ACT((ptr = (type *)_MALLOC(size)) != NULL, rv, act);        \
        memset(ptr, 0, size);                                                      \
        TSL_VASSERT_RV_ACT(ptr != NULL, rv, act);                                  \
}
#define TSL_SMALLOC(ptr, type, size) TSL_SMALLOC_RV(ptr, type, size, TSL_RV_FAIL_MEM)
                 

#define TSL_MALLOC_RV(ptr, type, rv)                                               \
{                                                                                  \
        TSL_VASSERT_RV((ptr = (type *)_MALLOC(sizeof(type))) != NULL, rv);         \
        memset(ptr, 0, sizeof(type));                                              \
        TSL_VASSERT_RV(ptr != NULL, rv);                                           \
}                                          
#define TSL_MALLOC_RV_ACT(ptr, type, rv, act)                                       \
{                                                                                  \
        TSL_VASSERT_RV_ACT((ptr = (type *)_MALLOC(sizeof(type))) != NULL, rv, act);\
        memset(ptr, 0, sizeof(type));                                              \
        TSL_VASSERT_RV_ACT(ptr != NULL, rv, act);                                  \
}
#define TSL_MALLOC_ACT(ptr, type, act)        TSL_MALLOC_RV_ACT(ptr, type, TSL_RV_FAIL_MEM, act)                    
#define TSL_MALLOC(ptr, type)                 TSL_MALLOC_RV(ptr, type, TSL_RV_FAIL_MEM)                    

#define TSL_FREE(ptr) _FREE(ptr)

#ifndef _DEBUG
#define printf(format, arg...)  
#endif
        

#define tsl_printf(format, arg...)              \
do{\
    fprintf(stdout, format, ##arg);\
}while(0)

//#define tsl_printf(format, arg...) 

#define MEM_ALIGN(size, boundary) \
    (((size) + ((boundary) - 1)) & ~((boundary) - 1))

#ifdef __cplusplus
}
#endif


#endif

