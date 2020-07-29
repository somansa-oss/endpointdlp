
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
    
    // System control API를 kext와의 통신 수단으로 사용할 때
    // kext에 진입하여 쓰레드 풀에 참여할 쓰레들들의 실행 함수임.
    // CPU 개수의 두 배에 해당하는 쓰레드들이 생성되어 이 함수를 통해 kext의 쓰레드 풀에 대기하게 됨.
    // 이 때 쓰레드 풀에 참여한다는 명령이 BEGIN_WAIT_THREAD임.
    // 쓰레드 풀에서 대기하다가 이벤트를 수신한 쓰레드는 유저 모드로 돌아와서 이벤트 처리를 진행함.
    // 이벤트 처리를 마치면 다시 쓰레드 풀로 들어가 대기 상태가 됨. 
    // DESTROY_THREAD_POOL 이벤트를 수신하여 돌아온 경우에만 루프를 종료하고 리턴함.
    static void* HandlerEventThread_SysCtl(void* pParam);
public: // Protocol
    // int SendCommand_SysCtl_EP(struct event_proto* pProto);
    int SendCommand_SysCtl( PCOMMAND_MESSAGE pCmdMsg );
private:
    // System control 채널을 해제하는 함수./Users/somansa/Documents/Project/EAT/Privacy-i/Source_MacOS/trunk/Common/DataType 복사본.h
    // Kernel control 자체를 없애는 것이 아니고, 쓰레드 풀을 해제함.
    // 이벤트 수신 대기 중인 쓰레드들이 없으므로 kext는 이벤트 발생해도 패킷을 전송하지 않게 됨.
    int QuitSystemControl();
    
};

extern CSysControl g_AppSctl;


#endif /* SysControl_hpp */
