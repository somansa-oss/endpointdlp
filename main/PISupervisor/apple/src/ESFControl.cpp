//
//  ESFControl.cpp
//  PISupervisor
//
//  Created by Juno on 2020/07/07.
//  Copyright © 2020 somansa. All rights reserved.
//
#define ACCESS_CHECK_NONE     0
#define ACCESS_CHECK_COPY     1
#define ACCESS_CHECK_PRINT    2
#define ACCESS_CHECK_UPLOAD   3

#ifdef LINUX
#include <string>
#include <map>
#endif


#include "PIDocument.h"
#include "ESFControl.h"
#include "ESFManager.h"
#include "PWProtocol.h"
#include "LogWriter.h"
#include "PIFullDiskAccess.h"
#include "PICommunicator.h"

#include <pthread.h>

#ifndef LINUX
#include <libproc.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/kern_event.h>
#include <CoreServices/CoreServices.h>
#include <sys/vnode.h>
#include <os/log.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <errno.h>
#endif

CESFControl g_AppESFctl;


CESFControl::CESFControl()
{
    srand((unsigned int)time(NULL));
    m_nESFCtlId = 0;
}

CESFControl::~CESFControl()
{
}


int CESFControl::ESFControl_Init()
{
    int  nRet = 0;
    pthread_t  PosixTID = 0;
    pthread_attr_t  Attr = {0};
    
    m_nESFCtlId = GetESFControlId();
    if(m_nESFCtlId == 0)
    {
        DEBUG_LOG("[ESF] GetESFControlId() failed err=%d, err-msg=%s \n", errno, strerror(errno) );
        return -1;
    }
    
    // Event Queue Listener Thread Create.
    nRet = pthread_attr_init( &Attr );
    if(nRet != 0)
    {
        DEBUG_LOG("[ESF] pthread_atrr_init() failed (%d) \n", nRet );
    }
    
    nRet = pthread_attr_setdetachstate( &Attr, PTHREAD_CREATE_DETACHED );
    if(nRet != 0)
    {
        DEBUG_LOG( "[ESF] pthread_attr_setdetachstate() failed(%d) \n", nRet );
        pthread_attr_destroy( &Attr );
        return -1;
    }
    
    nRet = pthread_create( &PosixTID, &Attr, CESFControl::ListenEventQueueThread, NULL );
    if(nRet != 0)
    {
        DEBUG_LOG("[ESF] pthread_create() failed(%d) \n", nRet );
        pthread_attr_destroy( &Attr );
        return -1;
    }
    
    pthread_attr_destroy( &Attr );
    DEBUG_LOG1("[ESF] Event Receiver thread Created. \n" );

    return 0;
}


int CESFControl::ESFControl_Uninit()
{
    int nRet = 0;
    nRet = QuitESFControl();
    return nRet;
}


int CESFControl::ConnectESFControl()
{
    int nSock = 0;

#ifndef LINUX    
    nSock = socket(AF_INET, SOCK_STREAM, SYSPROTO_CONTROL);
    if(nSock < 0)
    {
        DEBUG_LOG("[ESF] socket() failed. err=%d, err-msg=%s \n", errno, strerror(errno) );
        return -1;
    }
    
    struct sockaddr_ctl sc = { 0 };
    bzero(&sc, sizeof(struct sockaddr_ctl));
    sc.sc_len     = sizeof(struct sockaddr_ctl);
    sc.sc_family  = AF_INET;
    sc.ss_sysaddr = AF_SYS_CONTROL;
    sc.sc_id      = m_nESFCtlId;
    sc.sc_unit    = 0;
    
    if(connect( nSock, (struct sockaddr*)&sc, sizeof(sc)) < 0)
    {
        DEBUG_LOG("[ESF] connect() failed. err=%d, err-msg=%s \n", errno, strerror(errno) );
        close( nSock );
        nSock = 0;
        return -1;
    }
#endif

    return nSock;
}

int CESFControl::CloseESFControl(int nSock)
{
    if(close(nSock) < 0)
    {
        DEBUG_LOG("close() failed err=%d, err-msg=%s \n", errno, strerror(errno) );
        return -1;
    }
    return 0;
}


u_int32_t CESFControl::GetESFControlId()
{
#ifdef FIXME
    int nSock = 0;
    
    nSock = socket( AF_INET, SOCK_DGRAM, SYSPROTO_CONTROL);
    if(nSock < 0)
    {
        DEBUG_LOG("socket() failed(%d)\n", errno);
        return 0;
    }
    
    struct ctl_info ctl_info = { 0 };
    memset( &ctl_info, 0, sizeof(ctl_info));
    strncpy( ctl_info.ctl_name, kernctl_name_somansa, MAX_KCTL_NAME);
    
    ctl_info.ctl_name[MAX_KCTL_NAME-1] = '\0';
    if(ioctl( nSock, CTLIOCGINFO, &ctl_info) < 0)
    {
        DEBUG_LOG("ioctl(CTLIOCGINFO) failed(%d)\n", errno);
        close( nSock );
        return 0;
    }
    close( nSock );
    
    DEBUG_LOG("ESF control id: 0x%x for control name: %s\n", ctl_info.ctl_id, ctl_info.ctl_name);
    return ctl_info.ctl_id;
#endif
    return 1;
}

//
// ESF control 채널을 해제하는 함수.
// ESF control 자체를 없애는 것이 아니고, 리시버 등록을 해제한 것임.
// 이로써 등록된 에이전트 리시버가 없으므로 kext는 이벤트 발생해도 패킷을 전송하지 않게 됨.
//
// Return value
//      성공이면 0을 리턴함. 실패하면 errno을 리턴함.
//
int CESFControl::QuitESFControl()
{
    int   nSock = 0;
    COMMAND_MESSAGE CmdMsg;
    
    nSock = ConnectESFControl();
    if(nSock < 0)
    {
        return errno;
    }
    
    DEBUG_LOG1("waiting for threads exit\n");
    
    memset( &CmdMsg, 0, sizeof(CmdMsg) );
    CmdMsg.Size = sizeof(CmdMsg);
    CmdMsg.Command  = DESTROY_EVENT_QUEUE;
    
    if(send( nSock, &CmdMsg, CmdMsg.Size, 0) < 0)
    {
        printf("[ESF] send(DESTROY_EVENT_QUEUE) failed(%d)\n", errno);
        CloseESFControl( nSock );
        return errno;
    }
    
    CloseESFControl( nSock );
    DEBUG_LOG1("[ESF] all wait threads exited \n");
    return 0;
}

bool CESFControl::sendToPIESF(CPIPacket& packet, int& response) {

    bool returnValue = false;
    CPICommunicator::COMM_RESULT_T result;
    CPICommunicator* comm = new CPICommunicator();
    comm->support.bigEndian = ConfigMan.socket.bigEndian;
    comm->support.savePacket = ConfigMan.socket.savePacket;
    comm->support.savePacketPath = ConfigMan.path.dlpLogPath;
    comm->ipString = "127.0.0.1";
    comm->tcpPort = nsPISupervisor::PIESF_PORT;

    do {
        if( false == comm->initializeTCPClient() ) {
            ERROR_LOG( "PIESF Unable to create socket - %s",  strerror(errno));
            break;
        }

        result = comm->connectTCP();
        if(  CPICommunicator::RESULT_OK != result ) {
            ERROR_LOG( "PIESF Unable to connect server - code(%d) - (%d)%s", result, errno, strerror(errno));
            break;
        }

        DEBUG_LOG( "connect PIESF - ok - tcpState(%d)", comm->tcpState );

        comm->listSend.push_back(packet);

        result = comm->sendTCP();
        if(  CPICommunicator::RESULT_OK != result ) {
            ERROR_LOG( "PIESF Unable to send - code(%d) - tcpState(%d) - (%d)%s", result, comm->tcpState, errno, strerror(errno));
            break;
        }

        result = comm->recvTCP();
        if(  CPICommunicator::RESULT_OK != result ) {
            ERROR_LOG( "PIESF Unable to recv - code(%d) - (%d)%s", result, errno, strerror(errno));
            break;
        }

        returnValue = true;
    }while(false);

    if( 0 < comm->listRecv.size()) {
        DEBUG_LOG( "PIESF recv - msg_count:%d", comm->listRecv.size());
        std::list<CPIPacket>::iterator itr = comm->listRecv.begin();
        for( ; itr != comm->listRecv.end(); ++itr) {
            DEBUG_LOG( "PIESF recv - command(%d) - param_count(%d)", itr->getCommand(), itr->getParamCount());
            response = itr->get_int(0);
        }
    }

    if( CPICommunicator::TCPSTATE_NOTCONNECTED != comm->tcpState ) {
        comm->terminateTCPClient();
    }

    delete comm;
    comm = NULL;
    return returnValue;
}

int CESFControl::SendCommand_ESFCtl( PCOMMAND_MESSAGE pCmdMsg )
{
    int nSock=0;
    ssize_t nSend = 0;

    if(!pCmdMsg)
    {
        DEBUG_LOG1("SendCommand_ESFCtl InvalidParameter. \n");
        return -1;
    }

//    nSock = ConnectESFControl();
//    if(nSock < 0)
//    {
//        DEBUG_LOG1("ConnectESFControl Error.\n");
//        return -1;
//    }
//
//    nSend = send( nSock, pCmdMsg, pCmdMsg->Size, 0 );
//    if(nSend < 0)
//    {
//        DEBUG_LOG( "[ESF] SendCommand_ESFCtl send failed(%d) \n", errno);
//        CloseESFControl( nSock );
//        return -1;
//    }
//    CloseESFControl( nSock );
//    DEBUG_LOG1("[ESF] SendCommand_ESFCtl Success. \n" );
    
//    DEBUG_LOG( "piagent - begin - event:%d - param:%s", eventId, param.c_str());
//
//    if( false == isRegistered() ) {
//        DEBUG_LOG1( "piagent - stop - agent_not_registered");
//        INFO_LOG1( "notify event to piagent : stop - agent_not_registered");
//        return false;
//    }
    
    CPIPacket packet;
    packet.support.bigEndian = ConfigMan.socket.bigEndian;
    packet.support.unsignedType = ConfigMan.socket.unsignedType;
    packet.setCommand(pCmdMsg->Command);
    packet.addBinary(pCmdMsg, pCmdMsg->Size);

    int response = 0;
    bool result = sendToPIESF(packet, response);

    DEBUG_LOG( "pCmdMsg->Command: %d - result: %s - response: %d", pCmdMsg->Command, util.getValueAsString(result).c_str(), response);
    INFO_LOG( "pCmdMsg->Command: %d - result: %s, response: %d", pCmdMsg->Command, util.getValueAsString(result).c_str(), response);

    return result;

    
    return 0;
}


int CESFControl::SendRecvCommand_ESFCtl( PCOMMAND_MESSAGE pCmdMsg )
{
    int nSock=0;
    ssize_t nSend = 0, nRecv=0;
    
    if(!pCmdMsg)
    {
        DEBUG_LOG1("[ESF] SendRecvCommand_ESFCtl InvalidParameter. \n");
        return -1;
    }
    
    nSock = ConnectESFControl();
    if(nSock < 0)
    {
        DEBUG_LOG("ConnectESFControl Error=%d \n", errno );
        return -1;
    }
    
    nSend = send( nSock, pCmdMsg, pCmdMsg->Size, 0 );
    if(nSend < 0)
    {
        DEBUG_LOG( "SendRecvCommand_ESFCtl send failed(%d) \n", errno);
        CloseESFControl( nSock );
        return -1;
    }
    
    nRecv = recv( nSock, pCmdMsg, pCmdMsg->Size, 0 );
    if(nRecv < 0)
    {
        DEBUG_LOG( "SendRecvCommand_ESFCtl recv failed(%d) \n", errno);
        CloseESFControl( nSock );
        return -1;
    }
    
    CloseESFControl( nSock );
    DEBUG_LOG1("[ESF] SendRecvCommand_ESFCtl Success. \n" );
    return 0;
}




//
// 이 에이전트를 이벤트 리시버로 kext에 등록하고 아래 1,2번 과정을 무한 반복함.
// 1. recv() 함수를 호출하여 kext로부터의 이벤트 수신을 대기함.
// 2. 이벤트 수신하면 쓰레드를 생성하여 이벤트 넘기며 쓰레드에 이벤트 처리를 맡김.
// 이상의 1,2번 과정을 무한 반복하다가 DESTROY_EVENT_QUEUE 명령을 수신하면 종료함.
//
// Parameter
//        param : 사용하지 않음.
//

void* CESFControl::ListenEventQueueThread(void* pParam)
{
#ifndef LINUX
    // Kext에 소켓으로 연결함.
    ssize_t           nRecv=0;
    size_t            nDataSize=0, nTotalSize=0;
    int               nListenSock=0, nReturn=0;
    char*             pPacket = NULL;
    pthread_t         PosixTID;
    pthread_attr_t    Attr;
    PCOMMAND_MESSAGE  pCmdInitNew = NULL;
    PCOMMAND_MESSAGE  pCmdMsg     = NULL;
    COMM_AGENT_INFO*  pAgentInfo  = NULL;
    ULONG             nPacketSize = 0;
    
    DEBUG_LOG1("begin.");
    nListenSock = g_AppESFctl.ConnectESFControl();
    if(nListenSock < 0)
    {
        DEBUG_LOG("ConnectESFControl - creating socket failed(%d)\n", errno );
        return NULL;
    }
    
    // 이벤트 리시버로 등록하도록 kext에 BEGIN_EVENT_QUEUE 명령 전송함.
    nDataSize = sizeof(COMM_AGENT_INFO) + (MAXPATHLEN-1);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;
    pCmdInitNew = (PCOMMAND_MESSAGE)malloc( nDataSize );
    if(pCmdInitNew == NULL)
    {
        DEBUG_LOG("malloc() failed(%d)\n", errno );
        g_AppESFctl.CloseESFControl( nListenSock );
        return NULL;
    }
    
    pCmdInitNew->Size = (ULONG)nTotalSize;
    pCmdInitNew->Command  = BEGIN_EVENT_QUEUE;
    pAgentInfo = (COMM_AGENT_INFO*)pCmdInitNew->Data;
    if(pAgentInfo)
    {
        pAgentInfo->nPID = getpid();
        proc_pidpath( pAgentInfo->nPID, pAgentInfo->pBuf, MAXPATHLEN );
        DEBUG_LOG("[ESF] pCmdInitNew->pBuf=%s \n", pAgentInfo->pBuf );
    }
    
    if(send( nListenSock, pCmdInitNew, nTotalSize, 0 ) < 0)
    {
        DEBUG_LOG("send(BEGIN_EVENT_QUEUE) failed(%d) \n", errno );
        g_AppESFctl.CloseESFControl( nListenSock );
        free( pCmdInitNew );
        return NULL;
    }
    
    DEBUG_LOG( "pid=%d, HomeDir=%s \n", pAgentInfo->nPID, pAgentInfo->pBuf );
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
            DEBUG_LOG("[ESF] recv(%d bytes) 02 failed(%d)\n", nPacketSize, errno );
            free( pPacket );
            continue;
        }
        
        // DESTROY_EVENT_QUEUE 명령 수신 시 루프 종료하고 리턴함.
        pCmdMsg = (PCOMMAND_MESSAGE)pPacket;
        if(pCmdMsg->Command == DESTROY_EVENT_QUEUE)
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
            DEBUG_LOG("[ESF] pthread_attr_setdetachstate() failed(%d) \n", nReturn );
            pthread_attr_destroy( &Attr );
            free( pPacket );
            continue;
        }
        
        nReturn = pthread_create( &PosixTID, &Attr, CESFControl::JobEventThread_ESFCtl, (void*)pPacket );
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
    
    g_AppESFctl.CloseESFControl( nListenSock );
    if(pCmdInitNew) free( pCmdInitNew );
#endif

    return NULL;
}






//
// ESF control API를 kext와의 통신 수단으로 사용하는 경우에,
// 단위 이벤트 처리를 담당하는 쓰레드의 실행 함수.
// 파라미터로 전달받은 이벤트 패킷에 대한 처리를 마치면 쓰레드는 종료함.
// Kext가 100개의 이벤트를 전송하면 100개의 쓰레드를 생성하여 각각의 이벤트를 처리하는 개념.
// CPU 개수 두 배 만큼의 쓰레드들이 번갈아가며 이벤트를 처리하는 system control 모델과의 차이점임.
//
// Parameter
//        param : 이벤트 패킷 포인터. 즉, struct event_proto 구조체의 포인터.
//


void*
CESFControl::JobEventThread_ESFCtl(void* pPacket)
{
    int        nSock = 0;
//    ULONG      nCmd  = 0;
//    boolean_t  bSuc  = FALSE;
//    PCOMMAND_MESSAGE pCmdMsg = NULL;
//
//    pCmdMsg = (PCOMMAND_MESSAGE)pPacket;
//    if(pCmdMsg == NULL)
//    {
//        assert( pCmdMsg );
//        return NULL;
//    }
//
//    nSock = g_AppESFctl.ConnectESFControl();
//    if(nSock < 0)
//    {
//        printf("[ESF] socket() failed(%d)\n", errno );
//        free( pPacket );
//        pPacket = NULL;
//        return NULL;
//    }
//
//    nCmd = (ULONG)pCmdMsg->Command;
//    switch(nCmd)
//    {
//    case FileIsRemove:
//        bSuc = CESFControl::JobEvent_IsRemoable(nSock, pCmdMsg);
//        break;
//
//    case FileScan:
//        bSuc = CESFControl::JobEvent_FileScan(nSock, pCmdMsg);
//        break;
//
//    case FileDelete:
//        bSuc = CESFControl::JobEvent_FileDelete(nSock, pCmdMsg);
//        break;
//    case FileRename:
//        bSuc = CESFControl::JobEvent_FileRename(nSock, pCmdMsg);
//        break;
//    case FileExchangeData:
//        bSuc = CESFControl::JobEvent_FileExchangeData(nSock, pCmdMsg);
//        break;
//    case FileEventDiskFull:
//        bSuc = CESFControl::JobEvent_FileEventDiskFull(nSock, pCmdMsg);
//        break;
//
//    case FileEventNotify:
//        bSuc = CESFControl::JobEvent_FileEventNotify(nSock, pCmdMsg);
//        break;
//
//    case SmartLogNotify:
//        bSuc = CESFControl::JobEvent_SmartLogNotify(nSock, pCmdMsg );
//        break;
//    case GetPrintSpoolPath:
//        bSuc = CESFControl::JobEvent_GetPrintSpoolPath(nSock, pCmdMsg);
//        break;
//
//    case ProcessCallback:
//        bSuc = CESFControl::JobEvent_ProcessCallback(nSock, pCmdMsg);
//        break;
//
//    case ProcessAccessCheck:
//        bSuc = CESFControl::JobEvent_ProcessAccessCheck( nSock, pCmdMsg );
//        break;
//
//    case FullDiskAccessCheck:
//        bSuc = CESFControl::JobEvent_FullDiskAccessCheck( nSock, pCmdMsg );
//        break;
//
//     default: break;
//    }
//
//    g_AppESFctl.CloseESFControl( nSock );
//    if(pPacket) free( pPacket );
    return NULL;
}


/*******************************************************************************************************************/
// EventCallback Function
/*******************************************************************************************************************/


boolean_t
CESFControl::JobEvent_IsRemoable(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
    boolean_t  bRemovable = FALSE;
    size_t     nDataSize=0, nTotalSize=0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
    if(nSock < 0 || !pCmdMsg)
    {
        return FALSE;
    }
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    bRemovable = g_AppESF.IsRemovable( pNotify->czFilePath );
    if(bRemovable)
    {
        pNotify->nResult = RESULT_ALLOW;
    }
    else
    {
        pNotify->nResult = RESULT_DENY;
    }
    
    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;
    
    pCmdMsg->Size = (ULONG)nTotalSize;
    pCmdMsg->Command  = (ULONG)FileIsRemoveResult;
    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
    {
        printf("[ESF] send(FileIsRemoveResult) failed(%d)\n", errno );
    }
    return TRUE;
}


boolean_t
CESFControl::JobEvent_FileEventNotify(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
    ULONG  nCommand  = 0;
    int    nEventPID = 0, nAction=0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
    if(nSock < 0 || !pCmdMsg) return FALSE;
    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nEventPID = pNotify->nPID;
    nCommand  = (ULONG)pCmdMsg->Command;
    nAction   = (ULONG)pNotify->nAction;
    
    if(nAction == NOTIFY_BLOCK_READ || nAction == NOTIFY_BLOCK_WRITE)
    {
        // 1,2번 정책 적용 중일 때, kauth listener로부터 받는 명령임.
        // 응용프로그램에서 removable 디바이스의 파일을 읽거나 쓰려고 할 때
        // kauth listener가 접근을 차단한 후 결과를 에이전트에 통지하는 중.
        // Kext에 처리 결과를 리턴하지 않는 단방향 이벤트임.
        
        if(g_AppCallback != NULL)
        {
            EVT_PARAM  EvtInfo;
            memset( &EvtInfo, 0, sizeof(EvtInfo) );
            EvtInfo.Command   = nCommand;
            EvtInfo.ProcessId = nEventPID;
            EvtInfo.pFilePath  = pNotify->czFilePath;
            g_AppCallback( &EvtInfo );
        }
    }
    
    return TRUE;
}


void CESFControl::GetLogTime(char* pczCurTime, int nTimeBufSize)
{
    time_t     CurTime;
    struct tm* pTimeData = NULL;
    
    if(!pczCurTime) return;
    
    time( &CurTime );
    pTimeData = localtime( &CurTime );
    if(!pTimeData) return;
    
    snprintf( pczCurTime, nTimeBufSize, "%04d-%02d-%02d %02d:%02d:%02d",
             pTimeData->tm_year+1900, pTimeData->tm_mon+1, pTimeData->tm_mday,
             pTimeData->tm_hour, pTimeData->tm_min, pTimeData->tm_sec  );
    
}

boolean_t
CESFControl::JobEvent_SmartLogNotify(int nSock, PCOMMAND_MESSAGE pCmdMsg)
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

//
// ProcessAccessCheck 체크하는 로직추가 예시
//
boolean_t
CESFControl::IsProcessAccessCheckExample( int nPID, char* pczFilePath )
{
#ifndef LINUX

    char czProcName[260];
    char* pczToken = NULL;
    
    if(!pczFilePath)
    {
        return TRUE;
    }
    
    memset( czProcName, 0x00, sizeof(czProcName) );
    proc_name( nPID, czProcName, sizeof(czProcName) );
    
    // FilePath Control
    pczToken = strrchr( pczFilePath, '/' );
    if(pczToken)
    {
        pczToken++;
        if(strstr( pczToken, "Sublime") ||
            strstr( pczToken, "TextEdit") ||
            strstr( pczToken, "Safari") || strstr( pczToken, "Xcode") || strstr(pczToken, "sudo"))
        {
            printf("%s, pid=%d, proc=%s, FilePath=%s \n", __FUNCTION__, nPID, czProcName, pczFilePath );
            printf("%s, pid=%d, proc=%s, Token=%s, Deny. \n\n", __FUNCTION__, nPID, czProcName, pczToken );
            return FALSE;
        }
    }
    
    // ProcessName Control
    if(strstr(czProcName, "Sublime") ||
        strstr(czProcName, "TextEdit") ||
        strstr(czProcName, "Safari") || strstr( czProcName, "Xcode") || strstr(czProcName, "sudo"))
    {
        printf("%s, pid=%d, proc=%s, FilePath=%s Deny. \n\n", __FUNCTION__, nPID, czProcName, pczFilePath  );
        return FALSE;
    }
#endif    
    return TRUE;
}


boolean_t
CESFControl::IsProcessAccessCheck( const int nCommand, const int nPID, char* pczFilePath )
{
#ifndef LINUX

    boolean_t  bAccess = TRUE;
    EVT_PARAM  EvtInfo;
    char       czProcName[260];
    
    if(!pczFilePath)
    {
        return TRUE;
    }
    memset( czProcName, 0x00, sizeof(czProcName) );
    proc_name( nPID, czProcName, sizeof(czProcName) );
    memset( &EvtInfo, 0, sizeof(EvtInfo) );
    EvtInfo.Command     = nCommand;
    EvtInfo.ProcessId   = nPID; // pid
    EvtInfo.pFilePath   = pczFilePath; // path
    EvtInfo.pQtFilePath = czProcName; // process name

    if(g_AppCallback)
    {
        g_AppCallback( &EvtInfo );
        bAccess = EvtInfo.bAccess;
        if(!bAccess)
        {
            printf("[ESF] callback() failed, process not accessible \n" );
        }
    }
    return bAccess;
#else
    return TRUE;
#endif    
}


boolean_t
CESFControl::JobEvent_ProcessAccessCheck(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
#ifndef LINUX
    // size_t     nDataSize=0, nTotalSize=0;
    boolean_t  bAccess = FALSE;
    PSCANNER_NOTIFICATION pNotify = NULL;
    APP_PROCINFO  AppInfo = {0};
    char  czProcName[MAX_PROC];
    char  czParentName[MAX_PROC];
    
    if(nSock < 0 || !pCmdMsg)
    {
        return FALSE;
    }
    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify)
    {
        return FALSE;
    }
    
    memcpy( &AppInfo, pNotify->czQtFilePath, sizeof(APP_PROCINFO) );
    memset( czParentName, 0, sizeof(czParentName) );
    memset( czProcName, 0, sizeof(czProcName) );
    proc_name( AppInfo.nPPID, czParentName, sizeof(czParentName) );
    proc_name( AppInfo.nPID, czProcName, sizeof(czProcName) );
    DEBUG_LOG( "ProcessCreate, UID=%d, Parent=%s, Proc=%s, PID=%d, FilePath=%s", AppInfo.nUID, czParentName, czProcName, pNotify->nPID, pNotify->czFilePath );
    
    // ProcessAccessCheck 체크하는 로직추가 예시
    bAccess = IsProcessAccessCheck( pCmdMsg->Command, pNotify->nPID, pNotify->czFilePath );
    if(bAccess)
    {
        pNotify->nResult = RESULT_ALLOW;
    }
    else
    {
        pNotify->nResult = RESULT_DENY;
        DeviceMan.RequestProcessTerminate( pNotify );
    }
#endif    
    return TRUE;
}


boolean_t
CESFControl::CheckDiskAccessPermission( char* czPath )
{
#ifndef LINUX

    struct statfs fileStat = { 0, };
    char czRand[8] = { '.', 0, };
    if(!czPath)
    {
        os_log(OS_LOG_DEFAULT, "[ESF][%s] Invalid Paramenters.", __FUNCTION__ );
        return true;
    }
    
    if(statfs(czPath, &fileStat) == 0)
    {
        os_log(OS_LOG_DEFAULT, "[ESF][%s] [%s]-FileSystem Type: [%s]", __FUNCTION__ , czPath, fileStat.f_fstypename);
    
        if(0 == strcmp(fileStat.f_fstypename, "ntfs") || 0 == strcmp(fileStat.f_fstypename, "cd9660"))
        {
            int res = open(czPath, O_RDONLY);
            if (res >= 0)
                return true;
            else
                return false;
        }
    }
    
    for (int i = 1; i < 7; i++)
    {
        czRand[i] = 'a' + rand() % 26;
    }
    
    size_t nSize = strlen(czPath) + strlen(czRand) + 1;
    char* czFullPath = (char *)malloc(nSize);
    if( !czFullPath )
    {
        os_log(OS_LOG_DEFAULT, "[ESF][%s] MemoryAllocation.", __FUNCTION__ );
        return true;
    }
    
    memset(czFullPath, 0, nSize);
    sprintf(czFullPath, "%s/%s", czPath, czRand);
    os_log(OS_LOG_DEFAULT, "[ESF][%s] czRand: %s", __FUNCTION__, czRand );
    os_log(OS_LOG_DEFAULT, "[ESF][%s] czPath: %s", __FUNCTION__, czPath );
    os_log(OS_LOG_DEFAULT, "[ESF][%s] czFullPath: %s", __FUNCTION__, czFullPath);
    
    int nRes = open(czFullPath, O_CREAT);
    os_log(OS_LOG_DEFAULT, "[ESF][%s] %s / O_CREAT nRes: %d", __FUNCTION__, czRand, nRes);
    if (nRes == -1)
    {
        free(czFullPath);
        return false;
    }
    else
    {
        close(nRes);
    }
    
    nRes = open(czFullPath, O_WRONLY);
    os_log(OS_LOG_DEFAULT, "[ESF][%s] %s / O_WRONLY nRes: %d", __FUNCTION__, czRand, nRes);
    if (write(nRes, "\0", 2) == -1)
    {
        close(nRes);
        free(czFullPath);
        return false;
    }
    else
    {
        close(nRes);
    }
    
    nRes = open(czFullPath, O_RDONLY);
    os_log(OS_LOG_DEFAULT, "[ESF][%s] %s / O_RDONLY nRes: %d", __FUNCTION__, czRand, nRes);
    char cTemp[2] = { 0, };
    if(read(nRes, cTemp, 2) == -1)
    {
        close(nRes);
        free(czFullPath);
        return false;
    }
    else
    {
        close(nRes);
    }
    
    if(remove(czFullPath) == -1)
    {
        free(czFullPath);
        return false;
    }
    
    free(czFullPath);
    os_log(OS_LOG_DEFAULT, "[ESF][%s] %s return true.", __FUNCTION__, czRand);
#endif    
    return true;
}


size_t
CESFControl::FindDelimiter(char *cpPath)
{
    size_t i=0, nLength = 0;
    if(!cpPath)
    {
        return -1;
    }
    nLength = strlen(cpPath);
    for(i=0; i<nLength; i++)
    {
        if(cpPath[i] == '/')
        {
            return i;
        }
    }
    return nLength;
}

boolean_t
CESFControl::JobEvent_FullDiskAccessCheck( int nSock, PCOMMAND_MESSAGE pCmdMsg )
{
#ifndef LINUX

    int nResult = 0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    char czCompare[10] = { 0, };
    char czPath[512] = { 0, };
    
    if(nSock < 0 || !pCmdMsg)
    {
        return FALSE;
    }
    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify)
    {
        return FALSE;
    }

    os_log(OS_LOG_DEFAULT, "[ESF] pNotify->czFilePath: %s", pNotify->czFilePath);
    memcpy(czCompare, pNotify->czFilePath, sizeof(char) * 9);
    if((strcmp("/Volumes/", czCompare) != 0) || (strlen(pNotify->czFilePath) < 9))
    {
        return FALSE;
    }
    else
    {
        size_t nDelimeter = FindDelimiter(pNotify->czFilePath + 9);
        memcpy(czPath, pNotify->czFilePath, strlen("/Volumes/") + nDelimeter);
    }
    
    os_log(OS_LOG_DEFAULT, "[ESF] JobEvent_FullDiskAccessCheck / czPath: %s", czPath);
    size_t nSize = strlen(czPath) + strlen("diskutil unmount \"\"") + 1;
    char *czCmd = (char *)malloc(nSize);
    if (czCmd == NULL)
    {
        os_log(OS_LOG_DEFAULT, "[ESF] malloc fail");
        return FALSE;
    }
    
    memset(czCmd, 0, nSize);
    switch (pNotify->nAction)
    {
        case ACCESS_CHECK_COPY:
        {
            DeviceMan.m_cFDA.MountCtx_Update( czPath, czPath, BusTypeUsb );
            if(CPIFullDiskAccess::IsDiskAccessPermission( czPath ) == FALSE)
            {
                // os_log(OS_LOG_DEFAULT, "[ESF] CheckDiskAccessPermission == FALSE");
                // sprintf(czCmd, "diskutil unmount \"%s\"", czPath);
                // os_log(OS_LOG_DEFAULT, "[ESF] JobEvent_FullDiskAccessCheck / pzCmd: %s", czCmd);
                // int nResult = system(czCmd);
                //
                nResult = unmount( czPath, MNT_CMDFLAGS );
                os_log(OS_LOG_DEFAULT, "[ESF][%s] nResult=%d, Error=%d", __FUNCTION__, nResult, errno );
                if(nResult == 0)
                {
                    int nResponse = 0;
                    PIAgentStub.notifyNeedFullDiskAccessAuth(nResponse);
                }
            }
            break;
        }
        case ACCESS_CHECK_UPLOAD:
        {
            if(CPIFullDiskAccess::IsDiskAccessPermission( czPath ) == FALSE)
            {
                os_log(OS_LOG_DEFAULT, "[ESF] ACCESS_CHECK_UPLOAD");
                sprintf(czCmd, "diskutil unmount \"%s\"", czPath);
                int nResult = system(czCmd);
                if (nResult == 0)
                {
                    int nResponse = 0;
                    PIAgentStub.notifyNeedFullDiskAccessAuth(nResponse);
                }
            }
            break;
        }
        case ACCESS_CHECK_NONE:
            break;
        default:
            break;
    }
    free(czCmd);
#endif    
    return TRUE;
}



/*

boolean_t
CESFControl::FileScan_Process(int nSock, PCOMMAND_MESSAGE pCmdMsg, char* pczQtFilePath )
{
    boolean_t  bAccess   = FALSE;
    size_t     nFileSize = 0, nDataSize=0, nTotalSize=0;
    int        nCommand=0, nEventPID = 0;
    int        nTempFile=0;
    char       czPath[ MAXPATHLEN ];
    char*      pczPos = NULL;
    char*      pczBuf = NULL;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
    if(nSock < 0 || !pCmdMsg || !pczTempFilePath) return FALSE;
    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nCommand  = pCmdMsg->Command;
    nEventPID = pNotify->nPID;
    nFileSize = (size_t)pNotify->pParam;
    
    if(nFileSize <= 0) return FALSE;
    
    memset( czPath, 0, sizeof(czPath) );
    memset( pczTempFilePath, 0, MAXPATHLEN );
    strcpy( czPath, pNotify->czFilePath );
    strcpy( pczTempFilePath, czPath );
    
    pczPos = pczTempFilePath + strlen(pczTempFilePath);
    *pczPos++ = '.';
    *pczPos++ = '_';
    *pczPos++ = '@';
    *pczPos++ = '_';
    *pczPos++ = 0;
    
    printf("[ESF][%s] FilePath=%s \n",     __FUNCTION__, czPath );
    printf("[ESF][%s] TempFilePath=%s \n", __FUNCTION__, pczTempFilePath );
    
    nTempFile = open( pczTempFilePath, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU );
    if(nTempFile < 0)
    {   // 임시 파일 만들기 실패. '권한 없음'을 리턴함.
        printf("[ESF] open() failed(%d)\n", errno);
        return FALSE;
    }
    
    // 3. 파일 읽기/쓰기에 사용할 버퍼 할당하기
    pczBuf = (char*)malloc( nFileSize );
    if(pczBuf == NULL)
    {   // 버퍼를 할당하는데 실패함. '권한 없음'을 리턴함.
        close( nTempFile );
        unlink( pczTempFilePath ); // 임시 파일 지움.
        return FALSE;
    }
    
    nDataSize = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(SCANNER_NOTIFICATION);
    // 4. 원본 파일 내용 읽어오기
    pCmdMsg->Size = (ULONG)nTotalSize;
    pCmdMsg->Command  = (ULONG)GetFileData;
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(pNotify)
    {
        memset( pNotify, 0, nDataSize );
        pNotify->pParam   = (void*)pczBuf;
        pNotify->pParam02 = (void*)0;
        pNotify->pParam03 = (void*)nFileSize;
    }
    
    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
    {   // 원본 파일 내용 읽기 실패. '권한 없음'을 리턴함.
        printf("[ESF] send(GET_FILEDATA) failed(%d) \n", errno );
        free( pczBuf );
        close( nTempFile );
        unlink( pczTempFilePath );
        return FALSE;
    }
    
    // 5. 원본 파일 내용을 임시 파일에 쓰기
    if(write( nTempFile, pczBuf, nFileSize) < 0)
    {   // 임시 파일에 쓰기 실패. '권한 없음'을 리턴함.
        printf("[ESF] write() failed(%d)\n", errno);
        free( pczBuf );
        close( nTempFile );
        unlink( pczTempFilePath );
        return FALSE;
    }
    
    
    free( pczBuf );
    close( nTempFile );
    return bAccess;
}
*/

#define PRT_FILE "/Users/somansa/test/prt.pdf"

#ifdef LINUX
int
_mkpath_np(const char *path, mode_t omode, const char ** firstdir)
{
	char *apath = NULL;
	unsigned int depth = 0;
	mode_t chmod_mode = 0;
	int retval = 0;
	int old_errno = errno;
	struct stat sbuf;

	/* Try the trivial case first. */
	if (0 == mkdir(path, omode)) {
		if (firstdir) {
			*firstdir = strdup(path);
		}
		goto mkpath_exit;
	}

	/* Anything other than an ENOENT, EEXIST, or EISDIR indicates an
	 * error that we need to send back to the caller.  ENOENT indicates
	 * that we need to try a lower level.
	 */
	switch (errno) {
		case ENOENT:
			break;
		case EEXIST:
			if (stat(path, &sbuf) == 0) {
			    if (S_ISDIR(sbuf.st_mode)) {
					retval = EEXIST;
				} else {
					retval = ENOTDIR;
				}
			} else {
				retval = EIO;
			}
			goto mkpath_exit;
		case EISDIR: /* <rdar://problem/10288022> */
			retval = EEXIST;
			goto mkpath_exit;
		default:
			retval = errno;
			goto mkpath_exit;
	}

	apath = strdup(path);
	if (apath == NULL) {
		retval = ENOMEM;
		goto mkpath_exit;
	}

	while (1) {
		/* Increase our depth and try making that directory */
		char *s = strrchr(apath, '/');
		if (!s) {
			/* We should never hit this under normal circumstances,
			 * but it can occur due to really unfortunate timing
			 */
			retval = ENOENT;
			goto mkpath_exit;
		}
		*s = '\0';
		depth++;

		if (0 == mkdir(apath, S_IRWXU | S_IRWXG | S_IRWXO)) {
			/* Found our starting point */

			/* POSIX 1003.2:
			 * For each dir operand that does not name an existing
			 * directory, effects equivalent to those cased by the
			 * following command shall occcur:
			 *
			 * mkdir -p -m $(umask -S),u+wx $(dirname dir) &&
			 *    mkdir [-m mode] dir
			 */

			struct stat dirstat;
			if (-1 == stat(apath, &dirstat)) {
				/* Really unfortunate timing ... */
				retval = ENOENT;
				goto mkpath_exit;
			}

			if ((dirstat.st_mode & (S_IWUSR | S_IXUSR)) != (S_IWUSR | S_IXUSR)) {
			        chmod_mode = dirstat.st_mode | S_IWUSR | S_IXUSR;
				if (-1 == chmod(apath, chmod_mode)) {
					/* Really unfortunate timing ... */
					retval = ENOENT;
					goto mkpath_exit;
				}
			}

			if (firstdir) {
				*firstdir = strdup(apath);
			}
			break;
		} else if (errno == EEXIST) {
			/* Some other process won the race in creating this directory
			 * before we did.  We will use this as our starting point.
			 * See: <rdar://problem/10279893>
			 */
			if (stat(apath, &sbuf) == 0 &&
			    S_ISDIR(sbuf.st_mode)) {

				if (firstdir) {
					*firstdir = strdup(apath);
				}
				break;
			}

			retval = ENOTDIR;
			goto mkpath_exit;
		} else if (errno != ENOENT) {
			retval = errno;
			goto mkpath_exit;
		}
	}

	while (depth > 1) {
		/* Decrease our depth and make that directory */
		char *s = strrchr(apath, '\0');
		*s = '/';
		depth--;

		if (-1 == mkdir(apath, S_IRWXU | S_IRWXG | S_IRWXO)) {
			/* This handles "." and ".." added to the new section of path */
			if (errno == EEXIST)
				continue;
			retval = errno;
			goto mkpath_exit;
		}

		if (chmod_mode) {
			if (-1 == chmod(apath, chmod_mode)) {
				/* Really unfortunate timing ... */
				retval = ENOENT;
				goto mkpath_exit;
			}
		}
	}

	if (-1 == mkdir(path, omode)) {
		retval = errno;
		if (errno == EEXIST &&
		    stat(path, &sbuf) == 0 &&
		    !S_ISDIR(sbuf.st_mode)) {
			retval = ENOTDIR;
		}
	}

mkpath_exit:
	free(apath);

	errno = old_errno;
	return retval;
}

int mkpath_np(const char *path, mode_t omode) {
	return _mkpath_np(path, omode, NULL);
}
#endif

boolean_t
CESFControl::QtCopyFileUser(PCOMMAND_MESSAGE pCmdMsg)
{
    boolean_t  bAccess   = FALSE;
    size_t     nFileSize = 0;
    int        nCommand=0, nEventPID = 0;
    int        nFile=0, nQtFile=0;
    char*      pczBuf = NULL;
    char*      pczToken = NULL;
#ifdef LINUX
    char*      pczExt = NULL;
#endif    
    PSCANNER_NOTIFICATION pNotify = NULL;
    ACTION_TYPE emType;
    
    if(!pCmdMsg) return FALSE;
    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nCommand  = pCmdMsg->Command;
    nEventPID = pNotify->nPID;
    emType = (ACTION_TYPE)pNotify->nAction;
    nFileSize = (size_t)pNotify->pParam;
    if(nFileSize <= 0)
    {
        return FALSE;
    }
    
    printf("[ESF][%s] FilePath=%s   \n", __FUNCTION__, pNotify->czFilePath   );
    printf("[ESF][%s] QtFilePath=%s \n", __FUNCTION__, pNotify->czQtFilePath );
    
    mkpath_np( pNotify->czQtFilePath, S_IRWXU | S_IRWXG | S_IRWXO );
    
    pczToken = strrchr( pNotify->czFilePath, '/' );
    if(pczToken)
    {
#ifdef LINUX
        std::string cur = util.getCurrentDate();
        strcat( pNotify->czQtFilePath, "/" );
        strcat( pNotify->czQtFilePath, cur.c_str());

        pczExt = strstr( pczToken, "." );
        if (pczExt != NULL)
        {
            strcat( pNotify->czQtFilePath, pczExt);
        }
#else        
        strncat( pNotify->czQtFilePath, pczToken, strlen(pczToken)+1 );
#endif        
    }
    
    if(emType == ActionTypePrint)
    {
        char szPdf[] = ".pdf";
        strncat( pNotify->czQtFilePath, szPdf, strlen(szPdf));
    }

    nQtFile = open( pNotify->czQtFilePath, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
    if(nQtFile < 0)
    {   // 임시 파일 만들기 실패. '권한 없음'을 리턴함.
        printf("[ESF] open() failed(%d)\n", errno);
        return FALSE;
    }
    
    // 3. 파일 읽기/쓰기에 사용할 버퍼 할당하기
    pczBuf = (char*)malloc( nFileSize );
    if(pczBuf == NULL)
    {   // 버퍼를 할당하는데 실패함. '권한 없음'을 리턴함.
        close( nQtFile );
        unlink( pNotify->czQtFilePath ); // 임시 파일 지움.
        return FALSE;
    }
    
    // 4. 원본 파일 내용 읽어오기
    nFile = open( pNotify->czFilePath, O_RDONLY );
    if(nFile < 0)
    {
        free( pczBuf );
        close( nQtFile );
        unlink( pNotify->czQtFilePath );
        return FALSE;
    }
    
    if(read( nFile, pczBuf, nFileSize ) < 0)
    {
        free( pczBuf );
        close( nFile );
        close( nQtFile );
        unlink( pNotify->czQtFilePath );
        return FALSE;
    }
    close( nFile );
    
    // 5. 원본 파일 내용을 임시 파일에 쓰기
    if(write( nQtFile, pczBuf, nFileSize) < 0)
    {   // 임시 파일에 쓰기 실패. '권한 없음'을 리턴함.
        printf("[ESF] write() failed(%d)\n", errno);
        free( pczBuf );
        close( nQtFile );
        unlink( pNotify->czQtFilePath );
        return FALSE;
    }
    
    free( pczBuf );
    close( nQtFile );

//    {
//        size_t nLength=0, nCupsTmp=0;
//        nCupsTmp = strlen( DIR_CUPS_TMP );
//        nLength  = strlen( pNotify->czFilePath );
//        if(nLength >= nCupsTmp && 0 == strncasecmp( pNotify->czFilePath, DIR_CUPS_TMP, nCupsTmp ))
//        {
//            printf("[ESF][%s] Except Path=%s \n", __FUNCTION__, pNotify->czFilePath );
//        }
//        else
//        {
//            // Watermark Test
//            nLength = JobCopyFile( PRT_FILE, pNotify->czFilePath );
//        }
//    }

    return bAccess;
}

int CESFControl::JobCopyFile( const char* pczSrc, const char* pczDst )
{
#ifndef LINUX

    int  nSrcFile=0, nDstFile=0, nLength=0;
    size_t nRead=0, nWrite=0;
    char czBuf[1024];
    
    if(!pczSrc || !pczDst) return 0;
    
    printf("[ESF][%s] src=%s \n", __FUNCTION__, pczSrc );
    printf("[ESF][%s] dst=%s \n", __FUNCTION__, pczDst );
    
    nDstFile = open( pczDst, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
    if(nDstFile < 0)
    {   // 임시 파일 만들기 실패. '권한 없음'을 리턴함.
        printf("[ESF] open() failed(%d)\n", errno);
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
#else
    return 0;
#endif        
}

boolean_t
CESFControl::JobEvent_FileScan(int nSock, PCOMMAND_MESSAGE pCmdMsg, int& resultCode, std::string& resultValue)
{
    boolean_t    bAccess    = FALSE;
    size_t       nTotalSize = 0;
    EVT_PARAM    EvtInfo;
    PSCANNER_NOTIFICATION pNotify = NULL;

#ifndef LINUX
    if(nSock < 0 || !pCmdMsg)
#else
    if(!pCmdMsg)
#endif    
    {
        return FALSE;
    }
    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify)
    {
        return FALSE;
    }

    memset( &EvtInfo, 0, sizeof(EvtInfo) );
    EvtInfo.Command     = pCmdMsg->Command;
    EvtInfo.ProcessId   = pNotify->nPID;
    EvtInfo.FileSize    = (size_t)pNotify->pParam;
    EvtInfo.pFilePath   = pNotify->czFilePath;
    EvtInfo.pQtFilePath = pNotify->czQtFilePath;
    // BusType
    EvtInfo.ullReserved = pNotify->nAction;
    
    bAccess = g_AppESFctl.QtCopyFileUser(pCmdMsg);
    
    // 6. 콜백 함수 호출
    if(g_AppCallback)
    {   // 콜백 함수가 임시 파일을 별도 용도로 사용할 수 있으므로 임시 파일을 지우는 작업은 콜백 함수에 일임함.
        g_AppCallback( &EvtInfo );
        bAccess = EvtInfo.bAccess;
        if(!bAccess)
        {
            printf("[ESF] callback() failed, file not accessible \n" );
        }
    }
    else
    {
        // 콜백 함수 없음. '권한 없음'을 리턴함.
        // 콜백 함수를 등록한 후 파일 모니터링을 시작하기 때문에 발생하지 않을 오류임.
        if(EvtInfo.pQtFilePath)
        {
            unlink( EvtInfo.pQtFilePath );    // 파일 검문 과정이 끝났음을 kext에 알림. Kext는 최대 10초 동안 에이전트의 응답을 기다림.
        }
    }
    
    if(bAccess)
    {
        // '권한 있음'을 리턴함. 파일이 잘 저장됨.
        pNotify->nResult = RESULT_ALLOW; // 1
    }
    else
    { // '권한 없음'을 리턴함. Kext가 파일 내용을 다 지우게 됨.
        // pProto->param = NULL;
        pNotify->nResult = RESULT_DENY;
    }
    
    resultCode = pNotify->nResult;
    
//    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(SCANNER_NOTIFICATION);
//    pCmdMsg->Size = (ULONG)nTotalSize;
//    pCmdMsg->Command = FileScanResult;
//    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
//    {
//        printf("[ESF][%s]  FileScanResult failed(%d)\n", __FUNCTION__, errno );
//    }
    
    printf("[ESF][%s]  FileScanResult Success. bAccess=%d \n", __FUNCTION__, bAccess  );
  
    return TRUE;
}



boolean_t
CESFControl::JobEvent_FileDelete(int nSock, PCOMMAND_MESSAGE pCmdMsg, int& resultCode, std::string& resultValue)
{
    int     nEventPID = 0;
    size_t  nDataSize=0, nTotalSize=0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
#ifndef LINUX
    if(nSock < 0 || !pCmdMsg)
#else
    if(!pCmdMsg)
#endif  
    {
        return FALSE;
    }
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nEventPID = pNotify->nPID;
    if(unlink( pNotify->czFilePath ) < 0)
    {
        printf( "[ESF] unlink(%s) failed(%d)\n", pNotify->czFilePath, errno );
        pNotify->nResult = RESULT_DENY;
    }
    else
    {
        printf("[ESF] deleted %s \n",  pNotify->czFilePath );
        pNotify->nResult = RESULT_ALLOW;
    }
    
    resultCode = pNotify->nResult;
    
//    nDataSize  = sizeof(SCANNER_NOTIFICATION);
//    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;
//
//    pCmdMsg->Size = (ULONG)nTotalSize;
//    pCmdMsg->Command  = (ULONG)FileDeleteResult;
//    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
//    {
//        printf("[ESF] send(FileDeleteResult) failed(%d)\n", errno);
//    }

    return TRUE;
}

boolean_t
CESFControl::JobEvent_FileExchangeData(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
    int     nEventPID = 0;
    size_t  nDataSize=0, nTotalSize=0;
    char*   pczPath  = NULL;
    char*   pczPath2 = NULL;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
#ifndef LINUX
    if(nSock < 0 || !pCmdMsg)
#else
    if(!pCmdMsg)
#endif  
    {
        return FALSE;
    }

#ifndef LINUX    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nEventPID = pNotify->nPID;
    pczPath  = pNotify->czFilePath;
    pczPath2 = pNotify->czQtFilePath;
    if(exchangedata( pczPath, pczPath2, 0) < 0)
    {
        printf("[ESF] exchangedata() failed(%d)\n", errno);
        pNotify->nResult = RESULT_DENY;
    }
    else
    {
        printf("[ESF] data exchanged. %s, %s\n", pczPath, pczPath2 );
        pNotify->nResult = RESULT_ALLOW;
    }
    
    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;
    
    pCmdMsg->Size = (ULONG)nTotalSize;
    pCmdMsg->Command  = (ULONG)FileExchangeDataResult;
    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
    {
        printf("[ESF] send(FileExchangeDataResult) failed(%d) \n", errno);
    }
#endif
    return TRUE;
}




boolean_t
CESFControl::JobEvent_FileEventDiskFull(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
#ifndef LINUX        
    int     nCommand=0, nEventPID = 0;
    char    czFilePath[ MAXPATHLEN ];
    PSCANNER_NOTIFICATION pNotify = NULL;
#endif

#ifndef LINUX
    if(nSock < 0 || !pCmdMsg)
#else
    if(!pCmdMsg)
#endif  
    {
        return FALSE;
    }

#ifndef LINUX    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nCommand   = (int)pCmdMsg->Command;
    nEventPID  = pNotify->nPID;
    memset( czFilePath, 0, sizeof(czFilePath) );
    strcpy( czFilePath, pNotify->czFilePath );
    
    if(g_AppCallback)
    {
        EVT_PARAM EvtInfo;
        memset( &EvtInfo, 0, sizeof(EvtInfo) );
        EvtInfo.Command   = nCommand;
        EvtInfo.ProcessId = nEventPID;
        EvtInfo.pFilePath = czFilePath;
        
        g_AppCallback( &EvtInfo );
        
    }
#endif
    return TRUE;
}



boolean_t
CESFControl::JobEvent_FileRename(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
    int     nEventPID = 0;
    size_t  nDataSize=0, nTotalSize=0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
#ifndef LINUX
    if(nSock < 0 || !pCmdMsg)
#else
    if(!pCmdMsg)
#endif  
    {
        return FALSE;
    }

#ifndef LINUX    
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    nEventPID = pNotify->nPID;
    
    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;

    pCmdMsg->Size = (ULONG)nTotalSize;
    pCmdMsg->Command  = (ULONG)FileRenameResult;
    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
    {
        printf("[ESF] send(FileExchangeDataResult) failed(%d) \n", errno);
    }
#endif
    return TRUE;
}


#  define HTTP_MAX_BUFFER    2048    /* Max length of data buffer */
typedef long ssize_t;            /* @private@ */

struct _cups_file_s            /**** CUPS file structure... ****/
{
    int        fd;            /* File descriptor */
    char        mode,            /* Mode ('r' or 'w') */
    compressed,        /* Compression used? */
    is_stdio,        /* stdin/out/err? */
    eof,            /* End of file? */
    buf[4096],        /* Buffer */
    *ptr,            /* Pointer into buffer */
    *end;            /* End of buffer data */
    off_t        pos,            /* Position in file */
    bufpos;            /* File position for start of buffer */
    
    char        *printf_buffer;        /* cupsFilePrintf buffer */
    size_t    printf_size;        /* Size of cupsFilePrintf buffer */
};

typedef struct _cups_file_s cups_file_t;/**** CUPS file type ****/

/*
 * 'cupsFileCloseESF()' - Close a CUPS file.
 *
 * @since CUPS 1.2/macOS 10.5@
 */

int                    /* O - 0 on success, -1 on error */
cupsFileCloseESF(cups_file_t *fp)        /* I - CUPS file */
{
    int    fd;                /* File descriptor */
    char    mode;                /* Open mode */
    int    status;                /* Return status */
    int    is_stdio;            /* Is a stsdio file? */
    
    /*
     * Range check...
     */
    
    if (!fp)
        return (-1);
    
    /*
     * Flush pending write data...
     */
    status = 0;
    
    /*
     * Save the file descriptor we used and free memory...
     */
    
    fd       = fp->fd;
    mode     = fp->mode;
    is_stdio = fp->is_stdio;
    
    if (fp->printf_buffer)
        free(fp->printf_buffer);
    
    free(fp);
    
    /*
     * Close the file, returning the close status...
     */
    
    if (!is_stdio)
    {
        if (close(fd) < 0)
            status = -1;
    }
    
    return (status);
}

/*
 * 'cups_read()' - Read from a file descriptor.
 */

static ssize_t                /* O - Number of bytes read or -1 */
cups_read(cups_file_t *fp,        /* I - CUPS file */
          char        *buf,        /* I - Buffer */
          size_t      bytes)        /* I - Number bytes */
{
    ssize_t    total;            /* Total bytes read */
    
    /*
     * Loop until we read at least 0 bytes...
     */
    
    for (;;)
    {
#ifdef WIN32
        //if (fp->mode == 's')
        //  total = (ssize_t)recv(fp->fd, buf, (unsigned)bytes, 0);
        //else
        total = (ssize_t)read(fp->fd, buf, (unsigned)bytes);
#else
        if (fp->mode == 's')
            total = recv(fp->fd, buf, bytes, 0);
        else
            total = read(fp->fd, buf, bytes);
#endif /* WIN32 */
        
        if (total >= 0)
            break;
        
        /*
         * Reads can be interrupted by signals and unavailable resources...
         */
        
        if (errno == EAGAIN || errno == EINTR)
            continue;
        else
            return (-1);
    }
    
    /*
     * Return the total number of bytes read...
     */
    
    return (total);
}

/*
 * 'cups_fill()' - Fill the input buffer.
 */

static ssize_t                /* O - Number of bytes or -1 */
cups_fill(cups_file_t *fp)        /* I - CUPS file */
{
    ssize_t        bytes;        /* Number of bytes read */
    
    if (fp->ptr && fp->end)
        fp->bufpos += fp->end - fp->buf;
    
    /*
     * Read a buffer's full of data...
     */
    
    if ((bytes = cups_read(fp, fp->buf, sizeof(fp->buf))) <= 0)
    {
        /*
         * Can't read from file!
         */
        
        fp->eof = 1;
        fp->ptr = fp->buf;
        fp->end = fp->buf;
    }
    else
    {
        /*
         * Return the bytes we read...
         */
        
        fp->eof = 0;
        fp->ptr = fp->buf;
        fp->end = fp->buf + bytes;
    }
    return (bytes);
}

/*
 * 'cupsFileGetsESF()' - Get a CR and/or LF-terminated line.
 *
 * @since CUPS 1.2/macOS 10.5@
 */

char *                    /* O - Line read or @code NULL@ on end of file or error */
cupsFileGetsESF(cups_file_t *fp,        /* I - CUPS file */
             char        *buf,        /* O - String buffer */
             size_t      buflen)    /* I - Size of string buffer */
{
    int        ch;            /* Character from file */
    char        *ptr,            /* Current position in line buffer */
    *end;            /* End of line buffer */
    
    
    /*
     * Range check input...
     */
    if (!fp || (fp->mode != 'r' && fp->mode != 's') || !buf || buflen < 2)
        return (NULL);
    
    /*
     * Now loop until we have a valid line...
     */
    
    for (ptr = buf, end = buf + buflen - 1; ptr < end ;)
    {
        if (fp->ptr >= fp->end)
            if (cups_fill(fp) <= 0)
            {
                if (ptr == buf)
                    return (NULL);
                else
                    break;
            }
        
        ch = *(fp->ptr)++;
        fp->pos ++;
        
        if (ch == '\r')
        {
            /*
             * Check for CR LF...
             */
            
            if (fp->ptr >= fp->end)
                if (cups_fill(fp) <= 0)
                    break;
            
            if (*(fp->ptr) == '\n')
            {
                fp->ptr ++;
                fp->pos ++;
            }
            
            break;
        }
        else if (ch == '\n')
        {
            /*
             * Line feed ends a line...
             */
            
            break;
        }
        else
            *ptr++ = (char)ch;
    }
    
    *ptr = '\0';
    
    return (buf);
}

void _cups_strcpy_ESF(char *dst, const char *src)
{
    struct stat stFileInfo;
    stat(dst, &stFileInfo);
    
    while (*src)
        *dst++ = *src++;
    *dst = '\0';
}

int            /* O - 1 on match, 0 otherwise */
_cups_isspace_ESF(int ch)            /* I - Character to test */
{
    return (ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' ||
            ch == '\v');
}

/*
 * 'cupsFileGetConfESF()' - Get a line from a configuration file.
 *
 * @since CUPS 1.2/macOS 10.5@
 */

char *                    /* O  - Line read or @code NULL@ on end of file or error */
cupsFileGetConfESF(cups_file_t *fp,    /* I  - CUPS file */
                char        *buf,    /* O  - String buffer */
                size_t      buflen,    /* I  - Size of string buffer */
                char        **value,    /* O  - Pointer to value */
                int         *linenum)    /* IO - Current line number */
{
    char    *ptr;                /* Pointer into line */
    
    
    /*
     * Range check input...
     */
    
    if (!fp || (fp->mode != 'r' && fp->mode != 's') ||
        !buf || buflen < 2 || !value)
    {
        if (value)
            *value = NULL;
        
        return (NULL);
    }
    
    /*
     * Read the next non-comment line...
     */
    
    *value = NULL;
    
    while (cupsFileGetsESF(fp, buf, buflen))
    {
        (*linenum) ++;
        
        /*
         * Strip any comments...
         */
        
        if ((ptr = strchr(buf, '#')) != NULL)
        {
            if (ptr > buf && ptr[-1] == '\\')
            {
                // Unquote the #...
                _cups_strcpy_ESF(ptr - 1, ptr);
            }
            else
            {
                // Strip the comment and any trailing whitespace...
                while (ptr > buf)
                {
                    if (!_cups_isspace_ESF(ptr[-1]))
                        break;
                    
                    ptr --;
                }
                
                *ptr = '\0';
            }
        }
        
        /*
         * Strip leading whitespace...
         */

        for (ptr = buf; _cups_isspace_ESF(*ptr); ptr ++);

        //_cups_strcpy_ESF(buf, ptr);
        
        if (ptr > buf)
            //strcpy(buf, ptr);
            _cups_strcpy_ESF(buf, ptr);
        
        /*
         * See if there is anything left...
         */
        
        if (buf[0])
        {
            /*
             * Yes, grab any value and return...
             */
            
            for (ptr = buf; *ptr; ptr ++)
                if (_cups_isspace_ESF(*ptr))
                    break;

            if (*ptr)
            {
                /*
                 * Have a value, skip any other spaces...
                 */
                
                while (_cups_isspace_ESF(*ptr))
                    *ptr++ = '\0';
                
                if (*ptr)
                {
                    *value = ptr;
                }
                
                /*
                 * Strip trailing whitespace and > for lines that begin with <...
                 */
                
                ptr += strlen(ptr) - 1;
                
                if (buf[0] == '<' && *ptr == '>')
                    *ptr-- = '\0';
                else if (buf[0] == '<' && *ptr != '>')
                {
                    /*
                     * Syntax error...
                     */
                    
                    *value = NULL;
                    return (buf);
                }
                
                while (ptr > *value && _cups_isspace_ESF(*ptr))
                    *ptr-- = '\0';
            }
            
            /*
             * Return the line...
             */
            
            return (buf);
        }
    }
    
    return (NULL);
}

void            /* O - 1 on success, 0 on failure */
read_cups_spool_path_ESF(char * szSpoolPath, size_t SpoolPathlen, char * szTempPath, size_t TempPathlen)
{
    int                     fd;            /* File descriptor */
    cups_file_t            *fp;            /* New CUPS file */
    int            linenum = 0;            /* Current line number */
    char        line[HTTP_MAX_BUFFER],    /* Line from file */
    *value;            /* Value from line */
    
    fd = open(FILE_CUPSD_CONFIG, O_RDONLY, 0);
    
    if(fd != -1)
    {
        if ((fp = (cups_file_t    *)calloc(1, sizeof(cups_file_t))) != NULL)
        {
            fp->mode = 'r';
            fp->fd = fd;
            
            while (cupsFileGetConfESF(fp, line, sizeof(line), &value, &linenum))
            {
                if (!strcasecmp(line, "RequestRoot"))
                {
                    printf("[ESF] RequestRoot: %s\n", value);
                    strncpy(szSpoolPath, value, SpoolPathlen);
                }
                else if (!strcasecmp(line, "TempDir"))
                {
                    printf("[ESF] TempDir: %s\n", value);
                    strncpy(szTempPath, value, TempPathlen);
                }
            }
            cupsFileCloseESF(fp);
            
            if(0 == strlen(szSpoolPath))
            {
                strncpy(szSpoolPath, DIR_CUPS, strlen(DIR_CUPS));
            }
            
            if(0 == strlen(szTempPath))
            {
                strncpy(szTempPath, szSpoolPath, strlen(szSpoolPath));
                if(szTempPath[strlen(szSpoolPath)-1] == '/')
                    strncat(szTempPath, "tmp", sizeof("tmp"));
                else
                    strncat(szTempPath, "/tmp", sizeof("/tmp"));
            }
        }
    }
}

boolean_t
CESFControl::JobEvent_GetPrintSpoolPath(int nSock, PCOMMAND_MESSAGE pCmdMsg, int& resultCode, std::string& resultValue)
{
    size_t  nDataSize=0, nTotalSize=0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
    if(nSock < 0 || !pCmdMsg)
    {
        return FALSE;
    }
    
    printf("[ESF] JobEvent_GetPrintSpoolPath\n");

    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify) return FALSE;
    
    read_cups_spool_path_ESF(pNotify->czFilePath, sizeof(pNotify->czFilePath), pNotify->czQtFilePath, sizeof(pNotify->czQtFilePath));
    
    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;
    
//    pCmdMsg->Size = (ULONG)nTotalSize;
//    pCmdMsg->Command  = (ULONG)GetPrintSpoolPathResult;
//    if(send( nSock, pCmdMsg, nTotalSize, 0) < 0)
//    {
//        printf("[ESF] send(FileDeleteResult) failed(%d)\n", errno);
//    }
    
#ifdef FIXME_PRINT
    // NOT IMPL.
#endif
    
    return TRUE;
}


boolean_t CESFControl::JobEvent_ProcessCallback(int nSock, PCOMMAND_MESSAGE pCmdMsg)
{
    ULONG  nCommand  = 0;
    PSCANNER_NOTIFICATION pNotify = NULL;
    
    if(nSock < 0 || !pCmdMsg)
    {
        return FALSE;
    }
    
    printf("[ESF] JobEvent_ProcessCallback. \n");
    pNotify = (PSCANNER_NOTIFICATION)pCmdMsg->Data;
    if(!pNotify)
    {
        return FALSE;
    }
    
    //
    // DeviceMan.setSelfProtect();
    //

    nCommand  = (ULONG)pCmdMsg->Command;
    if(g_AppCallback != NULL)
    {
        EVT_PARAM  EvtInfo;
        memset( &EvtInfo, 0, sizeof(EvtInfo) );
        EvtInfo.Command   = nCommand;
        g_AppCallback( &EvtInfo );
    }
    return TRUE;
}
