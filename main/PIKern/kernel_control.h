#ifndef _KERNEL_CONTROL_H_
#define _KERNEL_CONTROL_H_


#ifndef kern_ctl_ref
#define kern_ctl_ref int
#endif


#ifdef LINUX
#include "KernelDataType.h"
#else
#include <mach/mach_types.h>
#include <sys/kernel_types.h>
#include <sys/kern_control.h>
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef LINUX
typedef struct _PARAM_KCTL
{
    kern_ctl_ref pKctlRef;
    void*        pMsgBuf;
    void*          pUnitInfo;
    uint32_t      nUnit;
    uint32_t      nFlags;
} PARAM_KCTL;
#else
typedef struct _PARAM_KCTL
{
    kern_ctl_ref pKctlRef;
    mbuf_t        pMsgBuf;
    void*          pUnitInfo;
    uint32_t      nUnit;
    uint32_t      nFlags;
} PARAM_KCTL;
#endif


kern_return_t
StartKernelControl(void);

kern_return_t
StopKernelControl(void);

kern_return_t
QueueEventData(void *data, size_t len);
    
kern_return_t
QueueEventData_WithEvent( kern_ctl_ref pEventRef, uint32_t nEventUnit, void* pData, size_t nLength );

    
    
    
    
#ifdef __cplusplus
};
#endif


#endif // _KERNEL_CONTROL_H_ //
