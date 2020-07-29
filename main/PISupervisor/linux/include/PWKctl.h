
#ifndef _PW_KCTL_H_
#define _PW_KCTL_H_

#include "PWProtocol.h"


class CPWKctl
{
public:
    CPWKctl();
    ~CPWKctl();
    
public:
    unsigned int m_nKernCtlId;
    
private:
    int QuitKernelControl();
    
public:
    unsigned int GetKernelControlId();
    
    int KernelControl_Init();
    int KernelControl_Uninit();
    
    int ConnectKernelControl();       // 소켓 연결을 설정하는 함수
    int CloseKernelControl(int nSock);// 소켓 연결을 해제하는 함수.
    
    int SendCommand_PWKctl( PMSG_COMMAND pCmdMsg );
    int SendRecvCommand_PWKctl( PMSG_COMMAND pCmdMsg );
    
    static void GetLogTime(char* pczCurTime, int nTimeBufSize);
    
public:
    static void* ListenEventQueueThread_PWKctl(void* pParam);
    static void* JobEventThread_PWKctl(void* pPacket);
    
public:
    int JobCopyFile( const char* pczSrc, const char* pczDst );
    
    boolean_t QtCopyFileUser( PMSG_COMMAND pCmdMsg );
    
    static boolean_t JobEvent_PWDisk_ProcessNoify( int nSock, PMSG_COMMAND pCmdMsg );    
    static boolean_t JobEvent_PWDisk_LogNotify( int nSock, PMSG_COMMAND pCmdMsg );
    
    
    

    
    


};


extern CPWKctl  g_PWKctl;


#endif /* _PW_KCTL_H_ */
