
#ifndef _KERN_CONTROL_H_
#define _KERN_CONTROL_H_

#include "linux/include/KernelDataType.h"
#include "linux/include/KernelProtocol.h"

class CKernControl
{
public:
    CKernControl();
    ~CKernControl();
public:
    unsigned int m_nKernCtlId;
    
private:
    // Kernel control 채널을 해제하는 함수. Return value: 성공이면 0을 리턴함. 실패하면 errno을 리턴함.
    int QuitKernelControl();
    
public:
    // Kernel control의 ID를 구하여 리턴하는 함수.
    // "com.somansa.MyKext"(가칭) 이름으로 등록한 kernel control의 ID를 구하여 리턴함. 이 control은 kext 초기화 과정에서 등록해 놓은 상태임.
    // Return value
    //      성공 kernel control ID 리턴. //
    //      실패 0 리턴. 이 경우 상세 오류는 errno 참조.
    
    unsigned int GetKernelControlId();
    
    int KernelControl_Init();
    int KernelControl_Uninit();
    
    int ConnectKernelControl();       // 소켓 연결을 설정하는 함수
    int CloseKernelControl(int nSock);// 소켓 연결을 해제하는 함수.
    
    int SendCommand_KernCtl( PCOMMAND_MESSAGE pCmdMsg );
    
    int SendRecvCommand_KernCtl( PCOMMAND_MESSAGE pCmdMsg );
    
    // int SendCommand_KernCtl_EP( struct event_proto* pProto );
    
public:
    // 이 에이전트를 이벤트 리시버로 kext에 등록하고 아래 1,2번 과정을 무한 반복함.
    // 1. recv() 함수를 호출하여 kext로부터의 이벤트 수신을 대기함.
    // 2. 이벤트 수신하면 쓰레드를 생성하여 이벤트 넘기며 쓰레드에 이벤트 처리를 맡김.
    // 이상의 1,2번 과정을 무한 반복하다가 DESTROY_EVENT_QUEUE 명령을 수신하면 종료함.
    static void* ListenEventQueueThread(void* pParam);
    
    // Kernel control API를 kext와의 통신 수단으로 사용하는 경우에, 단위 이벤트 처리를 담당하는 쓰레드의 실행 함수.
    // 파라미터로 전달받은 이벤트 패킷에 대한 처리를 마치면 쓰레드는 종료함.
    // CPU 개수 두 배 만큼의 쓰레드들이 번갈아가며 이벤트를 처리하는 system control 모델과의 차이점임.
    static void* JobEventThread_KernCtl(void* pPacket);
    
public:
    int JobCopyFile( const char* pczSrc, const char* pczDst );
    
    boolean_t QtCopyFileUser( PCOMMAND_MESSAGE pCmdMsg );
	static boolean_t JobEvent_FileScan(int nSock, PCOMMAND_MESSAGE pCmdMsg);
   
    static boolean_t JobEvent_FileDelete(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    static boolean_t JobEvent_GetPrintSpoolPath(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    static boolean_t JobEvent_FileRename(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    static boolean_t JobEvent_FileExchangeData(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    static boolean_t JobEvent_FileEventDiskFull(int nSock, PCOMMAND_MESSAGE pCmdMsg);

    static boolean_t JobEvent_FileEventNotify(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    static boolean_t JobEvent_SmartLogNotify(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    
    static boolean_t JobEvent_IsRemoable(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    static boolean_t JobEvent_ProcessCallback(int nSock, PCOMMAND_MESSAGE pCmdMsg);
	
	static boolean_t IsProcessAccessCheckExample( int nPID, char* pczFilePath );
	static boolean_t IsProcessAccessCheck( const int nCommand, const int nPID, char* pczFilePath );
    static boolean_t JobEvent_ProcessAccessCheck(int nSock, PCOMMAND_MESSAGE pCmdMsg);
    
    static boolean_t JobEvent_FullDiskAccessCheck( int nSock, PCOMMAND_MESSAGE pCmdMsg );
    static boolean_t CheckDiskAccessPermission( char* czPath );
    static size_t FindDelimiter(char *cpPath);
    
public:
    static void GetLogTime(char* pczCurTime, int nTimeBufSize);
};

extern CKernControl  g_AppKctl;

#endif /* _KERN_CONTROL_H_ */
