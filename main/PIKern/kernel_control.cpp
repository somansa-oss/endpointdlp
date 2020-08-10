#include <stdio.h>
#include <sys/mount.h>

#ifdef LINUX
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "DataType.h"
#include "KernelCommand.h"

#include "kernel_control.h"

#include "PISecSmartDrv.h"

kern_return_t
UserModeTo_SendCommand(void *data, size_t len  );

kern_return_t
UserModeTo_SendCommandULONG(SMARTDRV_COMMAND cmd, ULONG value);

kern_return_t
StartKernelControl(void)
{
    errno_t error = 0;

    g_DrvKext.CommCtx.Kctl.bRegistered = TRUE;
    LOG_MSG("[DLP][%s] ctl_register() success(%d), gKernCtlRegistered:%d \n", __FUNCTION__, error, g_DrvKext.CommCtx.Kctl.bRegistered );
    return KERN_SUCCESS;
}

kern_return_t
StopKernelControl(void)
{
    errno_t error = 0;
    if(g_DrvKext.CommCtx.Kctl.bRegistered == TRUE)
    {
        g_DrvKext.CommCtx.Kctl.bRegistered = FALSE;
        LOG_MSG( "[DLP][%s] ctl_deregister() success(%d), gKernCtlRegistered:%s\n",
                 __FUNCTION__, error, g_DrvKext.CommCtx.Kctl.bRegistered ? "true" : "false");
    }
    return KERN_SUCCESS;
}
