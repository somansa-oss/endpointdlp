#include <string>

#include "SysControl.h"
#include "KernelProtocol.h"
#include "KextManager.h"
#include <pthread.h>
#include <sys/sysctl.h>

CSysControl g_AppSctl;

CSysControl::CSysControl()
{
}

CSysControl::~CSysControl()
{
}

int CSysControl::SystemControl_Init()
{
    int    nRet = 0;

#ifndef LINUX

    int    nCpu = 0;
    int    nMib[2];
    size_t nLength = sizeof(nCpu);
    pthread_attr_t  Attr;
    pthread_t       PosixTID;
    bool            bAttrInit = false;

    nRet = sysctlbyname( "hw.logicalcpu_max", &nCpu, &nLength, NULL, 0 );
    if(nRet < 0)
    {
        printf("[DLP] sysctlbyname(hw.logicalcpu_max) failed(%d) \n", errno );
        return -1;
    }
    
    printf("[DLP] logical cpus: %d \n", nCpu );
    
    // Kext 에서 등록한 system control 의 이름으로 ID를 구함.
    // 이후의 모든 sysctl() 호출은 이 ID 르 키로 호출함
    
    nLength = 2;
    sysctlnametomib( sysctl_name_somansa, nMib, &nLength );
    
    // (CPU 개수 * 2)개의 쓰레드 생성함.
    for(int i=0; i<nCpu*2; i++)
    {
        // CreateThread
        nRet = pthread_attr_init( &Attr );
        if(nRet != 0)
        {
            printf("[DLP] pthread_attr_init() failed(%d) \n", nRet );
            if(bAttrInit)
            {
                pthread_attr_destroy( &Attr );
            }
            continue;
        }
        
        bAttrInit = true;
        nRet = pthread_attr_setdetachstate( &Attr, PTHREAD_CREATE_DETACHED );
        if(nRet != 0)
        {
            printf("[DLP] pthread_attr_setdeatchstate() failed(%d) \n", nRet );
            if(bAttrInit)
            {
                pthread_attr_destroy( &Attr );
            }
            continue;
        }
        
        nRet = pthread_create( &PosixTID, &Attr, &HandlerEventThread_SysCtl, NULL );
        if(nRet != 0)
        {
            printf( "[DLP] pthread_create() failed(%d) \n", nRet );
            if(bAttrInit)
            {
                pthread_attr_destroy( &Attr );
            }
            continue;
        }
        
        printf( "[DLP] pthread created \n" );
    }
#endif

    return 0;
}


int CSysControl::SystemControl_Uninit()
{
    int nRet = 0;

#ifdef _FIXME_
    nRet = QuitSystemControl();
#endif

    return nRet;
}


// System control 채널을 해제하는 함수.
// Kernel control 자체를 없애는 것이 아니고, 쓰레드 풀을 해제함.
// 이벤트 수신 대기 중인 쓰레드들이 없으므로 kext는 이벤트 발생해도 패킷을 전송하지 않게 됨.
//
// Return value
//      성공이면 0을 리턴함. 실패하면 errno을 리턴함.
int CSysControl::QuitSystemControl()
{
    printf("waiting for threads exit\n");

#ifdef _FIXME_    

    struct event_proto proto;
    proto.size = sizeof(struct event_proto);
    proto.command = DESTROY_THREAD_POOL;
    
    int64_t cmd = (int64_t)&proto;
    if (sysctlbyname(sysctl_name_somansa, NULL, NULL, &cmd, sizeof(cmd)) < 0)
    {
        printf("[DLP] sysctl(DESTROY_THREAD_POOL) failed(%d)\n", errno);
        return errno;
    }
#endif

    printf("[DLP] all wait threads exited\n");
    
    return 0;
}


// System control API를 kext와의 통신 수단으로 사용할 때
// kext에 진입하여 쓰레드 풀에 참여할 쓰레들들의 실행 함수임.
// CPU 개수의 두 배에 해당하는 쓰레드들이 생성되어 이 함수를 통해 kext의 쓰레드 풀에 대기하게 됨.
// 이 때 쓰레드 풀에 참여한다는 명령이 BEGIN_WAIT_THREAD임.
// 쓰레드 풀에서 대기하다가 이벤트를 수신한 쓰레드는 유저 모드로 돌아와서 이벤트 처리를 진행함.
// 이벤트 처리를 마치면 다시 쓰레드 풀로 들어가 대기 상태가 됨.
// DESTROY_THREAD_POOL 이벤트를 수신하여 돌아온 경우에만 루프를 종료하고 리턴함.
//
// Parameter
//		param : 사용하지 않음.
//
void* CSysControl::HandlerEventThread_SysCtl(void* pParam)
{
#ifdef _FIXME_

    int     nMib[2];
    size_t  nLength = 0;
    
    nLength = 2;
    sysctlnametomib( sysctl_name_somansa, nMib, &nLength );
    
    printf("[DLP] waiting for event\n");
    
    for( int64_t wait_count = 0; ; wait_count++)
    {
        // BEGIN_WAIT_THREAD 명령을 sysctl()을 통해 전송하면
        // sysctl() 함수는 이벤트 수신 전에는 리턴하지 않고 kext의 쓰레드 풀에 대기함.
        int16_t proto_size = sizeof(struct event_proto) + MAXPATHLEN - 1 + MAXPATHLEN;
        struct event_proto* proto = (struct event_proto*)malloc(proto_size);
        proto->size = proto_size;
        proto->command = BEGIN_WAIT_THREAD;
        proto->param = (void*)wait_count;
        proto->param2 = NULL;
        proto->param3 = NULL;
        proto->vnode = NULL;
        proto->wakeup_chan = NULL;
        memset(proto->buf, 0, MAXPATHLEN);
        int64_t newv_cmd = (int64_t)proto;
        
        if (sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd)) < 0)
        {
            printf("[DLP] sysctl(BEGIN_WAIT_THREAD) failed(%d)\n", errno);
            free(proto);
            break;
        }
        
        // 이 시점은 kext의 쓰레드 풀에 대기 중이던 이 쓰레드가 이벤트 수신하여 리턴한 상태임.
        // 이제 이벤트 처리 과정을 진행하게 됨.
        
        int command = proto->command;
        int event_owner_pid = proto->pid;
        
        if (command == DESTROY_THREAD_POOL)
        {
            // 쓰레드 풀 종료 명령을 받음.
            // 이 쓰레드는 일을 끝마치고 드디어 리턴함.
            free(proto);
            break;
        }
        
        if (command == IS_REMOVABLE)
        {
            proto->size = sizeof(struct event_proto);
            proto->command = REPORT_REMOVABLE;
            if(g_AppKext.IsRemovable(proto->buf))
                proto->param = (void*)1;
            else
                proto->param = NULL;
            
            sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd));
        }
        else if (command == DELETE_FILE)
        {
            if (unlink(proto->buf) < 0)
            {
                printf("[DLP] unlink(%s) failed(%d)\n", proto->buf, errno);
                proto->param = NULL;
            }
            else
            {
                printf("deleted %s\n", proto->buf);
                proto->param = (void*)1;
            }
            proto->size = sizeof(struct event_proto);
            proto->command = REPORT_DELETE_FILE;
            sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd));
        }
        else if (command == EXCHANGE_DATA)
        {
            char* path = proto->buf;
            char* path2 = proto->buf + strlen(path) + 1;
            if (exchangedata(path, path2, 0) < 0)
            {
                printf("exchangedata() failed(%d)\n", errno);
                proto->param = NULL;
            }
            else
            {
                printf("data exchanged. %s, %s\n", path, path2);
                proto->param = (void*)1;
            }
            proto->size = sizeof(struct event_proto);
            proto->command = REPORT_EXCHANGE_DATA;
            sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd));
        }
        else if (command == NOTIFY_DIRTY)
        {
            size_t filesize = (size_t)proto->param;
            boolean_t accessible = FALSE;
            
            // 1. 파일 경로 복사
            char path[MAXPATHLEN];
            strcpy(path, proto->buf);
            
            // 2. 임시 파일 만들기
            char path_tempfile[MAXPATHLEN];
            strcpy(path_tempfile, path);
            char* p = path_tempfile + strlen(path_tempfile);
            *p++ = '.';
            *p++ = '_';
            *p++ = '@';
            *p++ = '_';
            *p++ = 0;
            
            int tempfile = open(path_tempfile, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU);
            if (tempfile < 0)
            {
                // 임시 파일 만들기 실패. '권한 없음'을 리턴함.
                printf("open() failed(%d)\n", errno);
            }
            else
            {
                // 3. 파일 읽기/쓰기에 사용할 버퍼 할당하기
                char* buf = (char*)malloc(filesize);
                if (buf == NULL)
                {
                    // 버퍼를 할당하는데 실패함. '권한 없음'을 리턴함.
                    close(tempfile);
                    unlink(path_tempfile); // 임시 파일 지움.
                }
                else
                {
                    // 4. 원본 파일 내용 읽어오기
                    proto->size = sizeof(struct event_proto);
                    proto->command = GET_FILEDATA;
                    proto->param = (void*)buf;
                    proto->param2 = (void*)0; // offset
                    proto->param3 = (void*)filesize;
                    
                    if (sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd)) < 0)
                    {
                        // 원본 파일 내용 읽기 실패. '권한 없음'을 리턴함.
                        printf("sysctl(GET_FILEDATA) failed(%d)\n", errno);
                        free(buf);
                        close(tempfile);
                        unlink(path_tempfile); // 임시 파일을 지움.
                    }
                    else
                    {
                        // 5. 원본 파일 내용을 임시 파일에 쓰기
                        if (write(tempfile, buf, filesize) < 0)
                        {
                            // 임시 파일에 쓰기 실패. '권한 없음'을 리턴함.
                            printf("write() failed(%d)\n", errno);
                            free(buf);
                            close(tempfile);
                            unlink(path_tempfile); // 임시 파일을 지움.
                        }
                        else
                        {
                            free(buf);
                            close(tempfile);
                            
                            // 6. 콜백 함수 호출
                            if (g_AppCallback == NULL)
                            {
                                // 콜백 함수 없음. '권한 없음'을 리턴함.
                                // 콜백 함수를 등록한 후 파일 모니터링을 시작하기 때문에 발생하지 않을 오류임.
                                unlink(path_tempfile); // 임시 파일을 지움.
                            }
                            else
                            {
                                // 콜백 함수 호출
                                // 임시 파일은 콜백 함수에서 따로 사용할 수도 있으므로
                                // 임시 파일을 지우는 작업은 콜백 함수에 일임함.
                                EVT_PARAM  EvtInfo;
                                memset( &EvtInfo, 0, sizeof(EvtInfo) );
                                EvtInfo.Command = command;
                                EvtInfo.ProcessId = event_owner_pid;
                                EvtInfo.pFilePath = path;
                                EvtInfo.pQtFilePath = path_tempfile;
                                
                               //  int err = g_AppCallback(command, event_owner_pid, path, path_tempfile, &accessible);
                                
                                int err = g_AppCallback( &EvtInfo );
                                accessible = EvtInfo.bAccess;
                                if(accessible)
                                {
                                    // 콜백 함수가 '권한 있음'으로 판단했음.
                                }
                                else
                                {
                                    // 콜백 함수가 '권한 없음'으로 판단했거나, 오류 발생함.
                                    if (err != 0)
                                    {
                                        // 콜백 함수에서 권한 판단하는 과정에서 API 오류 발생함.
                                        // 이 경우도 '권한 없음'을 리턴함.
                                        printf("callback() failed(%d), file not accessible\n", err);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // 파일 검색 과정이 끝났음을 kext에 알림.
            // Kext는 최대 10초 동안 에이전트의 응답을 기다림.
            proto->size = sizeof(struct event_proto);
            proto->command = REPORT_DIRTY;
            if (accessible)
            {
                // '권한 있음'을 리턴함.
                // 파일이 잘 저장됨.
                proto->param = (void*)1;
            }
            else
            {
                // '권한 없음'을 리턴함.
                // Kext가 파일 내용을 다 지우게 됨.
                proto->param = NULL;
            }
            
            if (sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd)) < 0)
            {
                printf("sysctl(NOTIFY_EOE) failed(%d)\n", errno);
            }
        }
        else if (command == NOTIFY_NO_READ || command == NOTIFY_NO_WRITE)
        {
            // 1,2번 정책 적용 중일 때, kauth listener로부터 받는 명령임.
            // 응용프로그램에서 removable 디바이스의 파일을 읽거나 쓰려고 할 때
            // kauth listener가 접근을 차단한 후 결과를 에이전트에 통지하는 중.
            // Kext에 처리 결과를 리턴하지 않는 단방향 이벤트임.
            if (g_AppCallback != NULL)
            {
                EVT_PARAM EvtInfo;
                memset( &EvtInfo, 0, sizeof(EvtInfo) );
                EvtInfo.Command = command;
                EvtInfo.ProcessId = event_owner_pid;
                EvtInfo.pFilePath = proto->buf;
                
                // g_AppCallback(command, event_owner_pid, proto->buf, NULL, NULL);
                g_AppCallback( &EvtInfo );
            }
        }
        free(proto);
        // 이벤트 처리 과정을 마치고 다시 쓰레드 풀에 대기하러 가는 중..
    }
#endif

    printf("thread leaving\n");

    return NULL;
}




int CSysControl::SendCommand_SysCtl( PCOMMAND_MESSAGE pCmdMsg )
{
    int64_t  ullCommand = 0;

#ifdef _FIXME_    
    
    if(!pCmdMsg)
    {
        printf( "SetSendCommand_SysCtl() Invalid Parameter \n" );
        return -1;
    }
    
    ullCommand = (int64_t)pCmdMsg;
    
    if(sysctlbyname( sysctl_name_somansa, NULL, NULL, &ullCommand, sizeof(ullCommand) ) < 0)
    {
        printf("SetSendCommand_SysCtl sysctl failed(%d) \n", errno );
        return errno;
    }
    
#endif

    printf("SendCommand_SysCtl Success. \n" );
    return 0;
}

/*
int CSysControl::SendCommand_SysCtl_EP(struct event_proto* pProto)
{
    int64_t  ullCommand = 0;
    
    if(!pProto)
    {
        printf( "SetSendCommand_SysCtl() Invalid Parameter \n" );
        return -1;
    }
    
    ullCommand = (int64_t)pProto;
    
    if(sysctlbyname( sysctl_name_somansa, NULL, NULL, &ullCommand, sizeof(ullCommand) ) < 0)
    {
        printf("SetSendCommand_SysCtl sysctl failed(%d) \n", errno );
        return errno;
    }
    
    printf("SendCommand_SysCtl Success. \n" );
    return 0;
}
*/
