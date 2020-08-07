#include <stdio.h>
#include <sys/mount.h>

#include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"

#include "DataType.h"
#include "KernelCommand.h"

#include "kernel_control.h"

#include "PISecSmartDrv.h"

#include "system_control.h"

kern_return_t
StartSystemControl(void)
{
    if (0 == pthread_mutex_init( &g_DrvKext.CommCtx.Sctl._LockMutexGrp, NULL))
    {
        g_DrvKext.CommCtx.Sctl.LockMutexGrp = &g_DrvKext.CommCtx.Sctl._LockMutexGrp;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_grp_alloc_init() failed\n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
    if (0 == pthread_mutex_init( &g_DrvKext.CommCtx.Sctl._LockMutex, NULL))
    {
        g_DrvKext.CommCtx.Sctl.LockMutex = &g_DrvKext.CommCtx.Sctl._LockMutex;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() failed\n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
    g_DrvKext.CommCtx.Sctl.bRegisteredOID = TRUE;
    return KERN_SUCCESS;
}

kern_return_t
StopSystemControl(void)
{
    if(g_DrvKext.CommCtx.Sctl.bRegisteredOID)
    {
        g_DrvKext.CommCtx.Sctl.bRegisteredOID = FALSE;
    }
    
    if(g_DrvKext.CommCtx.Sctl.LockMutex != NULL)
    {
        pthread_mutex_destroy( g_DrvKext.CommCtx.Sctl.LockMutex );
        g_DrvKext.CommCtx.Sctl.LockMutex = NULL;
    }

    if(g_DrvKext.CommCtx.Sctl.LockMutexGrp != NULL)
    {
        pthread_mutex_destroy( g_DrvKext.CommCtx.Sctl.LockMutexGrp );
        g_DrvKext.CommCtx.Sctl.LockMutexGrp = NULL;
    }

    return KERN_SUCCESS;
}
