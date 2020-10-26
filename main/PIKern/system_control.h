#ifndef __system_control__
#define __system_control__

#ifndef LINUX
#include <sys/sysctl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    
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
