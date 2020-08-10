#include <string>
#include <string.h>

#ifdef LINUX
#else
#import <Foundation/Foundation.h>
#import <EndpointSecurity/EndpointSecurity.h>
#import <bsm/libbsm.h>
#endif

#define BOOL_VALUE(x) x ? "Yes" : "No"
#define LOG_INFO(fmt, ...) NSLog(@#fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) NSLog(@"ERROR: " @#fmt, ##__VA_ARGS__)

#include "PIESF.h"

#ifdef LINUX
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "DataType.h"
#include "KernelCommand.h"
#include "kernel_control.h"
#include "PISecSmartDrv.h"
#include "KauthEventFunc.h"

#ifdef LINUX
#else
es_client_t *g_client = nil;
#endif

CPIESF::CPIESF()
{
    m_bIsContinue = true;
}

CPIESF::~CPIESF()
{
    m_bIsContinue = false;
}


CPIESF& CPIESF::getInstance()
{
    static CPIESF instance;
    return instance;
}

bool CPIESF::isActive()
{
    return m_bIsContinue;
}

#ifdef LINUX
void CPIESF::run(void)//int argc, const char * argv[])
{
    //FIXME_MUST
}
#else
NSString* esstring_to_nsstring(const es_string_token_t *es_string_token) {
    if (es_string_token && es_string_token->data && es_string_token->length > 0) {
        return [NSString stringWithUTF8String:es_string_token->data];
    }
    return @"";
}

NSString* event_type_str(const es_event_type_t event_type) {
    switch(event_type) {
        case ES_EVENT_TYPE_AUTH_EXEC: return @"ES_EVENT_TYPE_AUTH_EXEC";
        case ES_EVENT_TYPE_NOTIFY_FORK: return @"ES_EVENT_TYPE_NOTIFY_FORK";
            
        case ES_EVENT_TYPE_AUTH_CREATE: return @"ES_EVENT_TYPE_AUTH_CREATE";
        case ES_EVENT_TYPE_NOTIFY_CREATE: return @"ES_EVENT_TYPE_NOTIFY_CREATE";
            
        case ES_EVENT_TYPE_AUTH_OPEN: return @"ES_EVENT_TYPE_AUTH_OPEN";
        case ES_EVENT_TYPE_NOTIFY_OPEN: return @"ES_EVENT_TYPE_NOTIFY_OPEN";
        case ES_EVENT_TYPE_NOTIFY_IOKIT_OPEN: return @"ES_EVENT_TYPE_NOTIFY_IOKIT_OPEN";
            
        case ES_EVENT_TYPE_AUTH_MOUNT: return @"ES_EVENT_TYPE_AUTH_MOUNT";
        case ES_EVENT_TYPE_NOTIFY_MOUNT: return @"ES_EVENT_TYPE_NOTIFY_MOUNT";
        case ES_EVENT_TYPE_NOTIFY_UNMOUNT: return @"ES_EVENT_TYPE_NOTIFY_UNMOUNT";
            
        case ES_EVENT_TYPE_NOTIFY_WRITE: return @"ES_EVENT_TYPE_NOTIFY_WRITE";
            
        case ES_EVENT_TYPE_NOTIFY_CLOSE: return @"ES_EVENT_TYPE_NOTIFY_CLOSE";

        case ES_EVENT_TYPE_AUTH_RENAME: return @"ES_EVENT_TYPE_AUTH_RENAME";
        case ES_EVENT_TYPE_NOTIFY_RENAME: return @"ES_EVENT_TYPE_NOTIFY_RENAME";

        case ES_EVENT_TYPE_AUTH_UNLINK: return @"ES_EVENT_TYPE_AUTH_UNLINK";
        case ES_EVENT_TYPE_NOTIFY_UNLINK: return @"ES_EVENT_TYPE_NOTIFY_UNLINK";

        case ES_EVENT_TYPE_AUTH_READDIR: return @"ES_EVENT_TYPE_AUTH_READDIR";
        case ES_EVENT_TYPE_NOTIFY_READDIR: return @"ES_EVENT_TYPE_NOTIFY_READDIR";
            
        default: return [NSString stringWithFormat:@"Unknown/Unsupported event type: %d", event_type];
    }
}

void log_proc(const NSString* header, const es_process_t *proc) {
    if(!proc) {
        LOG_INFO("%@: (null)", header);
        return;
    }
    LOG_INFO("%@:", header);
    LOG_INFO("  proc.pid: %d", audit_token_to_pid(proc->audit_token));
    LOG_INFO("  proc.ppid: %d", proc->ppid);
    LOG_INFO("  proc.original_ppid: %d", proc->original_ppid);
    LOG_INFO("  proc.ruid: %d", audit_token_to_ruid(proc->audit_token));
    LOG_INFO("  proc.euid: %d", audit_token_to_euid(proc->audit_token));
    LOG_INFO("  proc.rgid: %d", audit_token_to_rgid(proc->audit_token));
    LOG_INFO("  proc.egid: %d", audit_token_to_egid(proc->audit_token));
    LOG_INFO("  proc.group_id: %d", proc->group_id);
    LOG_INFO("  proc.session_id: %d", proc->session_id);
    LOG_INFO("  proc.codesigning_flags: %x", proc->codesigning_flags);
    LOG_INFO("  proc.is_platform_binary: %s", BOOL_VALUE(proc->is_platform_binary));
    LOG_INFO("  proc.is_es_client: %s", BOOL_VALUE(proc->is_es_client));
    LOG_INFO("  proc.signing_id: %@", esstring_to_nsstring(&proc->signing_id));
    LOG_INFO("  proc.team_id: %@", esstring_to_nsstring(&proc->team_id));
    
    // proc.cdhash
    NSMutableString *hash = [NSMutableString string];
    for(uint32_t i = 0; i < CS_CDHASH_LEN; i++) {
        [hash appendFormat:@"%x", proc->cdhash[i]];
    }
    LOG_INFO("  proc.cdhash: %@", hash);
    LOG_INFO("  proc.executable.path: %@",
             proc->executable ? esstring_to_nsstring(&proc->executable->path) : @"(null)");
}

void log_event_message(const es_message_t *msg) {
    LOG_INFO("--- EVENT MESSAGE ----");
    LOG_INFO("event_type: %@ (%d)", event_type_str(msg->event_type), msg->event_type);
    // Note: Message structure could change in future versions
    LOG_INFO("version: %u", msg->version);
    LOG_INFO("time: %lld.%.9ld, mach_time: %lld",
             (long long) msg->time.tv_sec, msg->time.tv_nsec, (long long) msg->mach_time);
    // It's very important that the message is processed within the deadline:
    // https://developer.apple.com/documentation/endpointsecurity/es_message_t/3334985-deadline
    LOG_INFO("deadline: %lld", (long long) msg->deadline);
    uint64_t deadlineInterval = msg->deadline;
    if(deadlineInterval > 0) {
        deadlineInterval -= msg->mach_time;
    }
    LOG_INFO("deadline interval: %lld (%d seconds)", (long long) deadlineInterval,
             (int) (deadlineInterval / 1.0e9));
    
    LOG_INFO("action_type: %s", (msg->action_type == ES_ACTION_TYPE_AUTH) ? "Auth" : "Notify");
    log_proc(@"process", msg->process);
    
    // Type specific logging
    switch(msg->event_type) {
        case ES_EVENT_TYPE_AUTH_EXEC: {
            log_proc(@"event.exec.target", msg->event.exec.target);
            
            // Log program arguments
            uint32_t argCount = es_exec_arg_count(&msg->event.exec);
            LOG_INFO("event.exec.arg_count: %u", argCount);
            
            // Extract each argument and log it out
            for(uint32_t i = 0; i < argCount; i++) {
                es_string_token_t arg = es_exec_arg(&msg->event.exec, i);
                LOG_INFO("arg %d: %@", i, esstring_to_nsstring(&arg));
            }
        }
            break;
        case ES_EVENT_TYPE_NOTIFY_FORK: {
        }
            break;
        case ES_EVENT_TYPE_LAST:
        default: {
            // Not interested
        }
    }
}

// Clean-up before exiting
void sig_handler(int sig) {
    LOG_INFO("Tidying Up");
    
    if(g_client) {
        es_unsubscribe_all(g_client);
        es_delete_client(g_client);
    }
    
    LOG_INFO("Exiting");
    exit(EXIT_SUCCESS);
}

std::string base_name(std::string const &path)
{
    return path.substr(path.find_last_of("/") + 1);
}

void CPIESF::run(void)//int argc, const char * argv[])
{
    m_bIsContinue = true;
    
    signal(SIGINT, &sig_handler);
    
    @autoreleasepool {
        
        es_handler_block_t usbHandler = ^(es_client_t *clt, const es_message_t *msg) {
            /*
             Ignore background event
             */
            
            LOG_PARAM  LogParam = {0};
            
            if([esstring_to_nsstring(&msg->process->executable->path) rangeOfString:@"/System/Library/Frameworks/CoreServices.framework/Versions/"].location == 0)
                goto usbHandlerOut;
            
            switch(msg->event_type) {
                    
                case ES_EVENT_TYPE_NOTIFY_CLOSE:
                    if([esstring_to_nsstring(&msg->event.close.target->path) rangeOfString:@"/Volumes/"].location == 0) {
                        
                        bool modified = msg->event.close.modified;
                        int nProcessId = audit_token_to_pid(msg->process->audit_token);
                        std::string process_path = msg->process->executable->path.data;
                        std::string process_name = base_name( process_path );
                        std::string target_file_path = msg->event.close.target->path.data;
                        int nFlag = KAUTH_FILEOP_CLOSE_MODIFIED;
                        
                        printf("ES_EVENT_TYPE_NOTIFY_CLOSE\n");
                        printf("pid-[%d]\n", nProcessId);
                        printf("proc.name-[%s]\n", process_name.c_str());
                        printf("targe_file_path-[%s]\n", target_file_path.c_str());
                        printf("targe_modified-[%d]\n", modified);
                        fflush(stdout);

                        if (msg->event.close.modified == false ||
                            msg->event.close.target == NULL
                            || msg->event.close.target->path.data == NULL)
                            break;

                        LogParam.nLogType    = LOG_FILEOP;
                        LogParam.nProcessId  = nProcessId;
                        LogParam.pczProcName = (char*)process_name.c_str();
                        LogParam.nAction = KAUTH_FILEOP_CLOSE;

                        Kauth_FileOp_FileClose(nProcessId,
                                               (char*)process_name.c_str(),
                                               NULL,
                                               (char*)target_file_path.c_str(),
                                               nFlag,
                                               &LogParam);
                    }
                    break;
                    
                // Rename event
                case ES_EVENT_TYPE_AUTH_RENAME:
                case ES_EVENT_TYPE_NOTIFY_RENAME:
                    {
                        es_string_token_t *path = (msg->event.rename.destination_type == ES_DESTINATION_TYPE_NEW_PATH) ? (&msg->event.rename.destination.existing_file->path) : (&msg->event.rename.destination.new_path.dir->path);
                        
                        if (path == NULL)
                            break;
                        
                        if([esstring_to_nsstring(path) rangeOfString:@"/Volumes/"].location == 0) {
                            //if( msg->event_type == ES_EVENT_TYPE_AUTH_RENAME )
                            {
                                //bool modified = msg->event.rename.destination;
                                bool modified = true;
                                int nProcessId = audit_token_to_pid(msg->process->audit_token);
                                std::string process_path = msg->process->executable->path.data;
                                std::string process_name = base_name( process_path );
                                std::string source_file_path = msg->event.rename.source->path.data;
                                std::string target_file_path;
                                
                                if (ES_DESTINATION_TYPE_NEW_PATH == msg->event.rename.destination_type)
                                {
                                    target_file_path = msg->event.rename.destination.new_path.dir->path.data;
                                    target_file_path += "/";
                                    target_file_path += msg->event.rename.destination.new_path.filename.data;
                                }
                                else
                                {
                                    target_file_path = msg->event.rename.destination.existing_file->path.data;
                                }
                                
                                int nFlag = KAUTH_FILEOP_CLOSE_MODIFIED;
                                
                                printf("ES_EVENT_TYPE_NOTIFY_RENAME\n");
                                printf("pid-[%d]\n", nProcessId);
                                printf("proc.name-[%s]\n", process_name.c_str());
                                printf("source_file_path-[%s]\n", source_file_path.c_str());
                                if (ES_DESTINATION_TYPE_NEW_PATH == msg->event.rename.destination_type)
                                {
                                    printf("targe_file_path-(new)[%s]\n", target_file_path.c_str());
                                }
                                else
                                {
                                    printf("targe_file_path-(existing)[%s]\n", target_file_path.c_str());
                                }
                                
                                fflush(stdout);

                                LogParam.nLogType    = LOG_FILEOP;
                                LogParam.nProcessId  = nProcessId;
                                LogParam.pczProcName = (char*)process_name.c_str();
                                LogParam.nAction = KAUTH_FILEOP_CLOSE;

                                Kauth_FileOp_FileClose(nProcessId,
                                                       (char*)process_name.c_str(),
                                                       NULL,
                                                       (char*)target_file_path.c_str(),
                                                       nFlag,
                                                       &LogParam);

                            }
                        }
                    }
                    break;
                  default: break;
            }
            
        usbHandlerOut:
            if(ES_ACTION_TYPE_AUTH == msg->action_type) {
                es_respond_result_t res = es_respond_auth_result(clt, msg, ES_AUTH_RESULT_ALLOW, false);
                if(ES_RESPOND_RESULT_SUCCESS != res) {
                    res = es_respond_flags_result(clt, msg, 0x7fffffff, false);
                    if(ES_RESPOND_RESULT_SUCCESS != res) {
                        LOG_ERROR("res_respond_flags_result: %d\n\n", res);
                    }
                }
            }
        };
        
        es_new_client_result_t res = es_new_client(&g_client, usbHandler);
        
        if(ES_NEW_CLIENT_RESULT_SUCCESS != res) {
            if(ES_NEW_CLIENT_RESULT_ERR_NOT_ENTITLED == res) {
               LOG_ERROR("Application requires 'com.apple.developer.endpoint-security.client' entitlement");
            } else if(ES_NEW_CLIENT_RESULT_ERR_NOT_PERMITTED == res) {
                LOG_ERROR("Application needs to run as root (and SIP disabled).");
            } else {
                LOG_ERROR("es_new_client: %d", res);
            }
        }
        
        // Cache needs to be explicitly cleared between program invocations
        es_clear_cache_result_t resCache = es_clear_cache(g_client);
        if(ES_CLEAR_CACHE_RESULT_SUCCESS != resCache) {
            LOG_ERROR("es_clear_cache: %d", resCache);
            //return 1;
        }
        
        // Subscribe to the events we're interested in
        es_event_type_t events[] = {
            ES_EVENT_TYPE_NOTIFY_CLOSE,
            ES_EVENT_TYPE_NOTIFY_RENAME,
        };
        
        es_return_t subscribed = es_subscribe(g_client,
                                       events,
                                       (sizeof(events) / sizeof((events)[0])) // Event count
                                       );
        if(ES_RETURN_ERROR == subscribed) {
            LOG_ERROR("es_subscribe: ES_RETURN_ERROR");
            //return 1;
        }
        
        dispatch_main();
    }
    
    
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

errno_t SmartCmd_SetQtPathExt( PCOMMAND_MESSAGE pCmdMsg );

errno_t SmartCmd_SetDrivePolicy( PCOMMAND_MESSAGE pCmdMsg );

#ifdef __cplusplus
}
#endif
