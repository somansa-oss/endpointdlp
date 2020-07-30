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

#ifdef _FIXME_

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
    
    
    nLength = 2;
    sysctlnametomib( sysctl_name_somansa, nMib, &nLength );
    
    for(int i=0; i<nCpu*2; i++)
    {
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
        
        int command = proto->command;
        int event_owner_pid = proto->pid;
        
        if (command == DESTROY_THREAD_POOL)
        {
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
            
            char path[MAXPATHLEN];
            strcpy(path, proto->buf);
            
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
                printf("open() failed(%d)\n", errno);
            }
            else
            {
                char* buf = (char*)malloc(filesize);
                if (buf == NULL)
                {
                    close(tempfile);
                    unlink(path_tempfile);
                }
                else
                {
                    proto->size = sizeof(struct event_proto);
                    proto->command = GET_FILEDATA;
                    proto->param = (void*)buf;
                    proto->param2 = (void*)0;
                    proto->param3 = (void*)filesize;
                    
                    if (sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd)) < 0)
                    {
                        printf("sysctl(GET_FILEDATA) failed(%d)\n", errno);
                        free(buf);
                        close(tempfile);
                        unlink(path_tempfile);
                    }
                    else
                    {
                        if (write(tempfile, buf, filesize) < 0)
                        {
                            printf("write() failed(%d)\n", errno);
                            free(buf);
                            close(tempfile);
                            unlink(path_tempfile);
                        }
                        else
                        {
                            free(buf);
                            close(tempfile);
                            
                            if (g_AppCallback == NULL)
                            {
                                unlink(path_tempfile);
                            }
                            else
                            {
                                EVT_PARAM  EvtInfo;
                                memset( &EvtInfo, 0, sizeof(EvtInfo) );
                                EvtInfo.Command = command;
                                EvtInfo.ProcessId = event_owner_pid;
                                EvtInfo.pFilePath = path;
                                EvtInfo.pQtFilePath = path_tempfile;
                               
                                int err = g_AppCallback( &EvtInfo );
                                accessible = EvtInfo.bAccess;
                                if(accessible)
                                {
                                }
                                else
                                {
                                    if (err != 0)
                                    {
                                        printf("callback() failed(%d), file not accessible\n", err);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            proto->size = sizeof(struct event_proto);
            proto->command = REPORT_DIRTY;
            if (accessible)
            {
                proto->param = (void*)1;
            }
            else
            {
                proto->param = NULL;
            }
            
            if (sysctl( nMib, 2, NULL, NULL, &newv_cmd, sizeof(newv_cmd)) < 0)
            {
                printf("sysctl(NOTIFY_EOE) failed(%d)\n", errno);
            }
        }
        else if (command == NOTIFY_NO_READ || command == NOTIFY_NO_WRITE)
        {
            if (g_AppCallback != NULL)
            {
                EVT_PARAM EvtInfo;
                memset( &EvtInfo, 0, sizeof(EvtInfo) );
                EvtInfo.Command = command;
                EvtInfo.ProcessId = event_owner_pid;
                EvtInfo.pFilePath = proto->buf;
                
                g_AppCallback( &EvtInfo );
            }
        }
        free(proto);
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