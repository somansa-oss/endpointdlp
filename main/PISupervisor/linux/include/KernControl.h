
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
    int QuitKernelControl();
    
public:
    unsigned int GetKernelControlId();
    
    int KernelControl_Init();
    int KernelControl_Uninit();
    
    int ConnectKernelControl();
    int CloseKernelControl(int nSock);
    
    int SendCommand_KernCtl( PCOMMAND_MESSAGE pCmdMsg );
    
    int SendRecvCommand_KernCtl( PCOMMAND_MESSAGE pCmdMsg );
    
    // int SendCommand_KernCtl_EP( struct event_proto* pProto );
    
public:
    static void* ListenEventQueueThread(void* pParam);
    
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
