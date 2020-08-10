#ifndef __system_control__
#define __system_control__

#ifndef LINUX
#include <sys/sysctl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//#include "event_proto.h"
//    
//typedef struct _PARAM_SCTL
//{
//    struct sysctl_oid* pOid;
//    struct sysctl_req* pReq;
//    void*    pArg1;
//    int      nArg2;
//} PARAM_SCTL;
    
kern_return_t
StartSystemControl(void);

kern_return_t
StopSystemControl(void);

kern_return_t
SendEventData( void* pData );


#ifdef __cplusplus
};
#endif
    
#endif //__system_control__
