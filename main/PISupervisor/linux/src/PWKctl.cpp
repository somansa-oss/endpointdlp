
#include "PWKctl.h"
#include "KextManager.h"

#include <pthread.h>
#include <libproc.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/kern_event.h>
#include <CoreServices/CoreServices.h>
#include "LogWriter.h"

CPWKctl g_PWKctl;


CPWKctl::CPWKctl() 
{
    m_nKernCtlId = 0;
}

CPWKctl::~CPWKctl()
{
}


int CPWKctl::KernelControl_Init()
{
    int  nRet = 0;
    pthread_t  PosixTID;
    pthread_attr_t  Attr;
    
    m_nKernCtlId = GetKernelControlId();
    if(m_nKernCtlId == 0)
    {
        DEBUG_LOG("[DLP][%s] GetKernelControlId() failed(%d) \n", __FUNCTION__, errno );
        return -1;
    }
    
    // Event Queue Listener Thread Create.
    nRet = pthread_attr_init( &Attr );
    if(nRet != 0)
    {
        DEBUG_LOG("[DLP][%s] pthread_atrr_init() failed(%d) \n", __FUNCTION__, nRet );
    }
    
    nRet = pthread_attr_setdetachstate( &Attr, PTHREAD_CREATE_DETACHED );
    if(nRet != 0)
    {
        DEBUG_LOG( "[DLP][%s] pthread_attr_setdetachstate() failed(%d) \n", __FUNCTION__, nRet );
        pthread_attr_destroy( &Attr );
        return -1;
    }
    
    nRet = pthread_create( &PosixTID, &Attr, CPWKctl::ListenEventQueueThread_PWKctl, NULL );
    if(nRet != 0)
    {
        DEBUG_LOG("[DLP][%s] pthread_create() failed(%d) \n", __FUNCTION__, nRet );
        pthread_attr_destroy( &Attr );
        return -1;
    }
    
    pthread_attr_destroy( &Attr );
    DEBUG_LOG("[DLP][%s] Event Receiver thread Created. \n", __FUNCTION__ );
    // 이 시점에
    // 위에서 생성한 이벤트 큐 listener 쓰레드는 kext에 자신을 이벤트 리시버로 등록한 상태에서 kext로부터의 이벤트나 명령 수신을 대기 중임.
    return 0;
}


int CPWKctl::KernelControl_Uninit()
{
    int nRet = 0;
    nRet = QuitKernelControl();
    return nRet;
}


int CPWKctl::ConnectKernelControl()
{
    int nSock = 0;
    
    nSock = socket(PF_SYSTEM, SOCK_STREAM, SYSPROTO_CONTROL);
    if(nSock < 0)
    {
        DEBUG_LOG("socket() failed(%d)\n", errno);
        return -1;
    }
    
    struct sockaddr_ctl sc = { 0 };
    bzero(&sc, sizeof(struct sockaddr_ctl));
    sc.sc_len     = sizeof(struct sockaddr_ctl);
    sc.sc_family  = AF_SYSTEM;
    sc.ss_sysaddr = AF_SYS_CONTROL;
    sc.sc_id      = m_nKernCtlId;
    sc.sc_unit    = 0;
    
    if(connect( nSock, (struct sockaddr*)&sc, sizeof(sc)) < 0)
    {
        DEBUG_LOG("connect() failed(%d)\n", errno);
        close( nSock );
        nSock = 0;
        return -1;
    }
    return nSock;
}

//
// 소켓 연결을 해제하는 함수.
//
// Parameter
//      sock : 소켓 descriptor.
// Return value
//      성공이면 0을 리턴. 실패이면 -1을 리턴하며, 상세 오류는 errno 참조.
//
int CPWKctl::CloseKernelControl(int nSock)
{
    if(close(nSock) < 0)
    {
        DEBUG_LOG("close() failed(%d)\n", errno);
        return -1;
    }
    return 0;
}


//
// Kernel control의 ID를 구하여 리턴하는 함수.
// "com.somansa.MyKext"(가칭) 이름으로 등록한 kernel control의 ID를 구하여 리턴함.
// 이 control은 kext 초기화 과정에서 등록해 놓은 상태임.
//
// Return value
//      성공이면 kernel control ID를 리턴.
//      실패하면 0을 리턴함. 이 경우 상세 오류는 errno 참조.
//

u_int32_t
CPWKctl::GetKernelControlId()
{
    int nSock = 0;
    
    nSock = socket( PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if(nSock < 0)
    {
        DEBUG_LOG("[DLP] socket() failed(%d)\n", errno);
        return 0;
    }
    
    struct ctl_info ctl_info = { 0 };
    memset( &ctl_info, 0, sizeof(ctl_info));
    strncpy( ctl_info.ctl_name, KERNCTL_PWDISK_NAME, MAX_KCTL_NAME);
    
    ctl_info.ctl_name[MAX_KCTL_NAME-1] = '\0';
    if(ioctl( nSock, CTLIOCGINFO, &ctl_info) < 0)
    {
        DEBUG_LOG("[DLP] ioctl(CTLIOCGINFO) failed(%d)\n", errno);
        close( nSock );
        return 0;
    }
    close( nSock );
    
    DEBUG_LOG("kernel control id: 0x%x for control name: %s\n", ctl_info.ctl_id, ctl_info.ctl_name);
    return ctl_info.ctl_id;
}

//
// Kernel control 채널을 해제하는 함수.
// Kernel control 자체를 없애는 것이 아니고, 리시버 등록을 해제한 것임.
// 이로써 등록된 에이전트 리시버가 없으므로 kext는 이벤트 발생해도 패킷을 전송하지 않게 됨.
//
// Return value
//      성공이면 0을 리턴함. 실패하면 errno을 리턴함.
//
int CPWKctl::QuitKernelControl()
{
    int   nSock = 0;
    MSG_COMMAND CmdMsg;
    
    nSock = ConnectKernelControl();
    if(nSock < 0)
    {
        DEBUG_LOG(" ConnectKernelControl() Failed=%d \n", errno);
        return -1;
    }
    
    DEBUG_LOG1("waiting for threads exit\n");
    
    memset( &CmdMsg, 0, sizeof(CmdMsg) );
    CmdMsg.Size = sizeof(CmdMsg);
    CmdMsg.Command  = PWEventQueue_DESTROY;
    
    if(send( nSock, &CmdMsg, CmdMsg.Size, 0) < 0)
    {
        DEBUG_LOG(" send(PWEventQueue_DESTROY) failed(%d)\n", errno);
        CloseKernelControl( nSock );
        return -1;
    }
    
    CloseKernelControl( nSock );
    DEBUG_LOG1("all wait threads exited\n");
    return 0;
}


int CPWKctl::SendCommand_PWKctl( PMSG_COMMAND pCmdMsg )
{
    int nSock=0;
    ssize_t nSend = 0;
    
    if(!pCmdMsg)
    {
        printf("[DLP] SendCommand_PWKctl InvalidParameter. \n");
        return -1;
    }
    
    nSock = ConnectKernelControl();
    if(nSock < 0) return errno;
    
    nSend = send( nSock, pCmdMsg, pCmdMsg->Size, 0 );
    if(nSend < 0)
    {
        printf( "[DLP] SendCommand_PWKctl send failed(%d) \n", errno);
        CloseKernelControl( nSock );
        return errno;
    }
    CloseKernelControl( nSock );
    // printf("[DLP] SendCommand_PWKctl Success. \n" );
    return 0;
}


int CPWKctl::SendRecvCommand_PWKctl( PMSG_COMMAND pCmdMsg )
{
    int nSock=0;
    ssize_t nSend = 0, nRecv=0;
    
    if(!pCmdMsg)
    {
        printf("[DLP] SendRecvCommand_PWKctl InvalidParameter. \n");
        return -1;
    }
    
    nSock = ConnectKernelControl();
    if(nSock < 0) return errno;
    
    nSend = send( nSock, pCmdMsg, pCmdMsg->Size, 0 );
    if(nSend < 0)
    {
        printf( "[DLP] SendRecvCommand_PWKctl send failed(%d) \n", errno);
        CloseKernelControl( nSock );
        return errno;
    }
    
    nRecv = recv( nSock, pCmdMsg, pCmdMsg->Size, 0 );
    if(nRecv < 0)
    {
        printf( "[DLP] SendRecvCommand_PWKctl recv failed(%d) \n", errno);
        CloseKernelControl( nSock );
        return errno;
    }
    
    CloseKernelControl( nSock );
    printf("[DLP] SendRecvCommand_PWKctl Success. \n" );
    return 0;
}


void* CPWKctl::ListenEventQueueThread_PWKctl(void* pParam)
{
    // Kext에 소켓으로 연결함.
    ssize_t           nRecv=0;
    size_t            nDataSize=0, nTotalSize=0;
    int               nListenSock=0, nReturn=0;
    char*             pPacket = NULL;
    pthread_t        PosixTID;
    pthread_attr_t  Attr;
    PMSG_COMMAND    pCmdInitNew = NULL;
    PMSG_COMMAND    pCmdMsg     = NULL;
    UKP_AGENT*       pAgentInfo  = NULL;
    ULONG             nPacketSize = 0;
    
    DEBUG_LOG1("begin");
    nListenSock = g_PWKctl.ConnectKernelControl();
    if(nListenSock < 0)
    {
        DEBUG_LOG("creating socket failed(%d)\n", errno );
        return NULL;
    }
    
    // 이벤트 리시버로 등록하도록 kext에 PWEventQueue_BEGIN 명령 전송함.
    nDataSize = sizeof(UKP_AGENT) + (MAXPATHLEN-1);
    nTotalSize = sizeof(MSG_COMMAND) + nDataSize;
    pCmdInitNew = (PMSG_COMMAND)malloc( nDataSize );
    if(pCmdInitNew == NULL)
    {
        DEBUG_LOG("malloc() failed(%d)\n", errno );
        g_PWKctl.CloseKernelControl( nListenSock );
        return NULL;
    }
    
    pCmdInitNew->Size = (ULONG)nTotalSize;
    pCmdInitNew->Command  = PWEventQueue_BEGIN;
    pAgentInfo = (UKP_AGENT*)pCmdInitNew->Data;
    if(pAgentInfo)
    {
        pAgentInfo->nPID = getpid();
        proc_pidpath( pAgentInfo->nPID, pAgentInfo->pBuf, MAXPATHLEN );
        // printf("[DLP] pCmdInitNew->pBuf=%s \n", pAgentInfo->pBuf );
    }
    
    if(send( nListenSock, pCmdInitNew, nTotalSize, 0 ) < 0)
    {
        DEBUG_LOG("send( PWEventQueue_BEGIN ) failed(%d) \n", errno );
        g_PWKctl.CloseKernelControl( nListenSock );
        free( pCmdInitNew );
        return NULL;
    }
    
    // 이벤트 큐 리시버로서의 동작을 시작함.
    nRecv = 0;
    nPacketSize = 0;
    while( (nRecv = recv( nListenSock, &nPacketSize, sizeof(nPacketSize), MSG_PEEK)) )
    {
        if(nRecv < 0)
        {   // 오류 발생. 로그 남긴 후 다시 대기 상태로..
            DEBUG_LOG("recv(%ldbyte) 01 failed(%d)\n", sizeof(nPacketSize), errno );
            continue;
        }
        
        // 이벤트 큐에서 패킷 길이만큼 이벤트 메시지를 읽어냄.
        pPacket = (char*)malloc( nPacketSize );
        if(pPacket == NULL)
        {
            continue;
        }
        
        nRecv = recv( nListenSock, pPacket, nPacketSize, 0);
        if(nRecv < 0)
        {
            DEBUG_LOG("recv(%d bytes) 02 failed(%d)\n", nPacketSize, errno );
            free( pPacket );
            continue;
        }
        
        
        // PWEventQueue_DESTROY 명령 수신 시 루프 종료하고 리턴함.
        pCmdMsg = (PMSG_COMMAND)pPacket;
        if(pCmdMsg->Command == PWEventQueue_DESTROY)
        {
            free( pPacket);
            break;
        }
        
        // 이벤트를 처리할 쓰레드를 생성해서 이벤트 패킷을 넘김. 이벤트 패킷을 넘겨 받은 쓰레드는 이벤트를 처리한 후 종료함.
        nReturn = pthread_attr_init( &Attr );
        if(nReturn != 0)
        {
            DEBUG_LOG("pthread_attr_init() failed(%d)\n", nReturn );
            free( pPacket );
            continue;
        }
        
        nReturn = pthread_attr_setdetachstate( &Attr, PTHREAD_CREATE_DETACHED );
        if(nReturn != 0)
        {
            DEBUG_LOG("pthread_attr_setdetachstate() failed(%d) \n", nReturn );
            pthread_attr_destroy( &Attr );
            free( pPacket );
            continue;
        }
        
        nReturn = pthread_create( &PosixTID, &Attr, CPWKctl::JobEventThread_PWKctl, (void*)pPacket );
        if(nReturn != 0)
        {
            DEBUG_LOG("pthread_create() failed(%d)\n", nReturn );
            pthread_attr_destroy( &Attr );
            free( pPacket );
            continue;
        }
        
        pthread_attr_destroy( &Attr );
        pPacket = NULL;
        // 다시 이벤트 수신 대기 상태로 들어감.
    }
    
    g_PWKctl.CloseKernelControl( nListenSock );
    if(pCmdInitNew) free( pCmdInitNew );
    return NULL;
}






//
// Kernel control API를 kext와의 통신 수단으로 사용하는 경우에,
// 단위 이벤트 처리를 담당하는 쓰레드의 실행 함수.
// 파라미터로 전달받은 이벤트 패킷에 대한 처리를 마치면 쓰레드는 종료함.
// Kext가 100개의 이벤트를 전송하면 100개의 쓰레드를 생성하여 각각의 이벤트를 처리하는 개념.
// CPU 개수 두 배 만큼의 쓰레드들이 번갈아가며 이벤트를 처리하는 system control 모델과의 차이점임.
//
// Parameter
//		param : 이벤트 패킷 포인터. 즉, struct event_proto 구조체의 포인터.
//


void*
CPWKctl::JobEventThread_PWKctl(void* pPacket)
{
    int        nSock = 0;
    ULONG      nCmd  = 0;
    boolean_t  bSuc  = FALSE;
    PMSG_COMMAND pCmdMsg = NULL;
    
    pCmdMsg = (PMSG_COMMAND)pPacket;
    if(pCmdMsg == NULL)
    {
        return NULL;
    }
    
    nSock = g_PWKctl.ConnectKernelControl();
    if(nSock < 0)
    {
        DEBUG_LOG("ConnectKernelControl() Failed. socket() failed(%d)\n", errno );
        free( pPacket );
        pPacket = NULL;
        return NULL;
    }
    
    nCmd = (ULONG)pCmdMsg->Command;
    switch(nCmd)
    {
    case PWNotify_ProcessNotify:
        bSuc = CPWKctl::JobEvent_PWDisk_ProcessNoify( nSock, pCmdMsg );
        break;
            
    case PWNotify_PWDiskLogNotify:
        bSuc = CPWKctl::JobEvent_PWDisk_LogNotify( nSock, pCmdMsg );
        break;
            
     default: break;
    }
    
    g_PWKctl.CloseKernelControl( nSock );
    if(pPacket) free( pPacket );
    return NULL;
}


/*******************************************************************************************************************/
// EventCallback Function
/*******************************************************************************************************************/

void CPWKctl::GetLogTime(char* pczCurTime, int nTimeBufSize)
{
    time_t     CurTime;
    struct tm* pTimeData = NULL;
    
    if(!pczCurTime) return;
    
    time( &CurTime );
    pTimeData = localtime( &CurTime );
    if(!pTimeData) return;
    
    snprintf( pczCurTime, nTimeBufSize, "%04d%02d%02d-%02d%02d%02d",
             pTimeData->tm_year+1900, pTimeData->tm_mon+1, pTimeData->tm_mday,
             pTimeData->tm_hour, pTimeData->tm_min, pTimeData->tm_sec  );
    
}



boolean_t
CPWKctl::JobEvent_PWDisk_ProcessNoify( int nSock, PMSG_COMMAND pCmdMsg )
{
    return TRUE;
}


boolean_t
CPWKctl::JobEvent_PWDisk_LogNotify(int nSock, PMSG_COMMAND pCmdMsg)
{
    ULONG  Command = 0;
    PSMART_LOG_RECORD_EX pLogEx = NULL;
    
    if(nSock < 0 || !pCmdMsg) return FALSE;
    
    pLogEx = (PSMART_LOG_RECORD_EX)pCmdMsg->Data;
    if(!pLogEx) return FALSE;
    
    GetLogTime( pLogEx->Log.Data.LogTime, sizeof(pLogEx->Log.Data.LogTime) );
    
    Command = (ULONG)pCmdMsg->Command;
    if(g_AppCallback != NULL)
    {
        EVT_PARAM EvtInfo;
        memset( &EvtInfo, 0, sizeof(EvtInfo) );
        EvtInfo.Command = Command;
        EvtInfo.ProcessId = pLogEx->Log.Data.ProcessId;
        EvtInfo.pEvtCtx   = pLogEx;
        g_AppCallback( &EvtInfo );
    }
    return TRUE;
}



#define PRT_FILE "/Users/somansa/test/prt.pdf"

boolean_t
CPWKctl::QtCopyFileUser(PMSG_COMMAND pCmdMsg)
{
    boolean_t  bAccess   = FALSE;
    size_t     nFileSize = 0;
    int        nCommand=0, nEventPID = 0;
    int        nFile=0, nQtFile=0;
    char*      pczBuf = NULL;
    char*      pczToken = NULL;
    PMSG_NOTIFY pNotify = NULL;
    
    if(!pCmdMsg) return FALSE;
    
    pNotify = (PMSG_NOTIFY)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nCommand  = pCmdMsg->Command;
    nEventPID = pNotify->nPID;
    nFileSize = (size_t)pNotify->pParam;
    if(nFileSize <= 0)
    {
        return FALSE;
    }
    
    printf("[DLP][%s] FilePath=%s   \n", __FUNCTION__, pNotify->czFilePath   );
    printf("[DLP][%s] QtFilePath=%s \n", __FUNCTION__, pNotify->czFilePathDst );
    
    mkpath_np( pNotify->czFilePathDst, S_IRWXU | S_IRWXG | S_IRWXO );
    
    pczToken = strrchr( pNotify->czFilePath, '/' );
    if(pczToken)
    {
        strncat( pNotify->czFilePathDst, pczToken, strlen(pczToken)+1 );
    }
    
    nQtFile = open( pNotify->czFilePathDst, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
    if(nQtFile < 0)
    {   // 임시 파일 만들기 실패. '권한 없음'을 리턴함.
        printf("[DLP] open() failed(%d)\n", errno);
        return FALSE;
    }
    
    // 3. 파일 읽기/쓰기에 사용할 버퍼 할당하기
    pczBuf = (char*)malloc( nFileSize );
    if(pczBuf == NULL)
    {   // 버퍼를 할당하는데 실패함. '권한 없음'을 리턴함.
        close( nQtFile );
        unlink( pNotify->czFilePathDst ); // 임시 파일 지움.
        return FALSE;
    }
    
    // 4. 원본 파일 내용 읽어오기
    nFile = open( pNotify->czFilePath, O_RDONLY );
    if(nFile < 0)
    {
        free( pczBuf );
        close( nQtFile );
        unlink( pNotify->czFilePathDst );
        return FALSE;
    }
    
    if(read( nFile, pczBuf, nFileSize ) < 0)
    {
        free( pczBuf );
        close( nFile );
        close( nQtFile );
        unlink( pNotify->czFilePathDst );
        return FALSE;
    }
    close( nFile );
    
    // 5. 원본 파일 내용을 임시 파일에 쓰기
    if(write( nQtFile, pczBuf, nFileSize) < 0)
    {   // 임시 파일에 쓰기 실패. '권한 없음'을 리턴함.
        printf("[DLP] write() failed(%d)\n", errno);
        free( pczBuf );
        close( nQtFile );
        unlink( pNotify->czFilePathDst );
        return FALSE;
    }
    
    free( pczBuf );
    close( nQtFile );
    
    {
        size_t nLength=0, nCupsTmp=0;
        nCupsTmp = strlen( DIR_CUPS_TMP );
        nLength  = strlen( pNotify->czFilePath );
        if(nLength >= nCupsTmp && 0 == strncasecmp( pNotify->czFilePath, DIR_CUPS_TMP, nCupsTmp ))
        {
            printf("[DLP][%s] Except Path=%s \n", __FUNCTION__, pNotify->czFilePath );
        }
        else
        {
            // Watermark Test
            nLength = JobCopyFile( PRT_FILE, pNotify->czFilePath );
        }
    }
    
    return bAccess;
}


int CPWKctl::JobCopyFile( const char* pczSrc, const char* pczDst )
{
    int  nSrcFile=0, nDstFile=0, nLength=0;
    size_t nRead=0, nWrite=0;
    char czBuf[1024];
    
    if(!pczSrc || !pczDst) return 0;
    
    printf("[DLP][%s] src=%s \n", __FUNCTION__, pczSrc );
    printf("[DLP][%s] dst=%s \n", __FUNCTION__, pczDst );
    
    nDstFile = open( pczDst, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
    if(nDstFile < 0)
    {   // 임시 파일 만들기 실패. '권한 없음'을 리턴함.
        printf("[DLP] open() failed(%d)\n", errno);
        return 0;
    }
    
    // 원본 파일 내용 읽어오기
    nSrcFile = open( pczSrc, O_RDONLY );
    if(nSrcFile < 0)
    {
        close( nDstFile );
        unlink( pczDst );
        return 0;
    }
    
    while(true)
    {
        memset( czBuf, 0, sizeof(czBuf) );
        nRead = read( nSrcFile, czBuf, sizeof(czBuf) );
        if(nRead <= 0)
        {
            break;
        }
        
        nWrite = write( nDstFile, czBuf, nRead );
        if(nWrite <= 0)
        {
            break;
        }
        
        nLength += nWrite;
    }
    
    close( nSrcFile );
    close( nDstFile );
    return nLength;
}




