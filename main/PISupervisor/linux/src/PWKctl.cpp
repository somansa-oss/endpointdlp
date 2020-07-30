
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

int CPWKctl::CloseKernelControl(int nSock)
{
    if(close(nSock) < 0)
    {
        DEBUG_LOG("close() failed(%d)\n", errno);
        return -1;
    }
    return 0;
}

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
    }
    
    if(send( nListenSock, pCmdInitNew, nTotalSize, 0 ) < 0)
    {
        DEBUG_LOG("send( PWEventQueue_BEGIN ) failed(%d) \n", errno );
        g_PWKctl.CloseKernelControl( nListenSock );
        free( pCmdInitNew );
        return NULL;
    }
    
    nRecv = 0;
    nPacketSize = 0;
    while( (nRecv = recv( nListenSock, &nPacketSize, sizeof(nPacketSize), MSG_PEEK)) )
    {
        if(nRecv < 0)
        {
            DEBUG_LOG("recv(%ldbyte) 01 failed(%d)\n", sizeof(nPacketSize), errno );
            continue;
        }
        
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
        
        pCmdMsg = (PMSG_COMMAND)pPacket;
        if(pCmdMsg->Command == PWEventQueue_DESTROY)
        {
            free( pPacket);
            break;
        }
		
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
    }
    
    g_PWKctl.CloseKernelControl( nListenSock );
    if(pCmdInitNew) free( pCmdInitNew );
    return NULL;
}


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
    {
        printf("[DLP] open() failed(%d)\n", errno);
        return FALSE;
    }
    
    pczBuf = (char*)malloc( nFileSize );
    if(pczBuf == NULL)
    {
        close( nQtFile );
        unlink( pNotify->czFilePathDst );
        return FALSE;
    }
    
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
    
    if(write( nQtFile, pczBuf, nFileSize) < 0)
    {
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
    {
        printf("[DLP] open() failed(%d)\n", errno);
        return 0;
    }
    
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




