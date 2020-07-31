
#ifndef _SYS_CONTROL_H_
#define _SYS_CONTROL_H_

#include "linux/include/KernelProtocol.h"

class CSysControl
{
public:
    CSysControl();
    ~CSysControl();
    
public:
    int SystemControl_Init();
    int SystemControl_Uninit();
    
    static void* HandlerEventThread_SysCtl(void* pParam);
public: // Protocol
    // int SendCommand_SysCtl_EP(struct event_proto* pProto);
    int SendCommand_SysCtl( PCOMMAND_MESSAGE pCmdMsg );
private:
    int QuitSystemControl();
    
};

extern CSysControl g_AppSctl;


#endif /* SysControl_hpp */
