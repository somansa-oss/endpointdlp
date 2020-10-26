#include <string>
#include <string.h>

#ifdef LINUX
#include <unistd.h>     // getuid
#include <lsf-api/media-api.h>
#include <assert.h>
#include <stdio.h>
#endif

#define LSF_MEDIA_USB_MEMORY       "usb_memory"


#ifdef LINUX
#include <glib.h>
#include <gio/gio.h>
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
#include "KextDeviceNotify.h"
#include "SmartCmd.h"


#ifdef LINUX
monitored_t* CPIESF::monitors = NULL;
int CPIESF::n_monitors = 0;
#else
es_client_t *g_client = nil;
#endif

CPIESF::CPIESF()
{
    m_bIsContinue = true;
    monitors = (monitored_t*)malloc (MAX_DEVICE * sizeof (monitored_t));
}

CPIESF::~CPIESF()
{
    m_bIsContinue = false;
    if (monitors != NULL)
    {
        for(int i = 0; i < n_monitors; i++)
        {
            if (monitors[i].path != NULL)
            {
                free (monitors[i].path);
            }
        }        
        free( monitors );
    }
}

bool CPIESF::finalize(void)
{
    pthread_mutex_destroy( &mutexClient); 
    pthread_cond_destroy( &condClient); 

    if (fanotify_fd != 0)
    {
        close (fanotify_fd);
    }

    return true;
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

bool CPIESF::initialize(void) {

	if( true == isInitialized() ) {
		return true;
	}

	pthread_mutex_init( &mutexClient, 0 );
	pthread_cond_init( &condClient, 0 );


    fanotify_fd = 0;
    memset(fds, 0, sizeof(fds));

    if ((fanotify_fd = fanotify_init (FAN_CLOEXEC| FAN_CLASS_CONTENT,
                                      O_RDONLY | O_CLOEXEC | O_LARGEFILE | O_NOATIME)) < 0)
    {
        ERROR_LOG ("Couldn't setup new fanotify device: %s\n", strerror (errno));
        return -1;
    }    

    fds[FD_POLL_FANOTIFY].fd = fanotify_fd;
    fds[FD_POLL_FANOTIFY].events = POLLIN;

	return CPIObject::initialize();
}

bool CPIESF::isRunningServerThread(void) {
    return runningServerThread;
}

bool CPIESF::isRunningClientThread(void) {
    return runningClientThread;
}


void CPIESF::waitThreads(void) {

	DEBUG_LOG1("mount_monitor_thread - begin");

	if( true == isRunningServerThread() ) {
		pthread_join( serverThread, (void**)NULL );
	}

	if( true == isRunningClientThread() ) {
		pthread_join( clientThread, (void**)NULL );
	}
	
	DEBUG_LOG1("mount_monitor_thread - end");
}

int CPIESF::stop(void) {
	DEBUG_LOG1("mount_monitor_thread - begin");
	
	isContinue = false;
		
	pthread_cond_signal(&condClient);
	waitThreads();
	finalize();

	DEBUG_LOG1("mount_monitor_thread - end");
	return 0;
}

static gboolean
proc_mounts_changed (GIOChannel   *channel,
                     GIOCondition  cond,
                     gpointer      user_data)
{
    if (cond & G_IO_ERR)
    {
        DEBUG_LOG1 ("MOUNTS CHANGED!");

        FetchVolumes();
    }

    return TRUE;
}

void* CPIESF::fnWaitClient(void* pzArg) {
	DEBUG_LOG1("mount_monitor_thread - begin");

	CPIESF* instance;
	instance = reinterpret_cast<CPIESF*>(pzArg);
	instance->runningServerThread = true;

    GIOChannel *proc_mounts_channel = NULL;
    GSource *proc_mounts_watch_source = NULL;
    GError *error = NULL;
    GMainLoop *loop = NULL;

    //proc_mounts_channel = g_io_channel_new_file ("/proc/self/mountinfo", "r", &error);
    proc_mounts_channel = g_io_channel_new_file ("/proc/mounts", "r", &error);
    if (proc_mounts_channel == NULL)
    {
        INFO_LOG ("Error creating IO channel for %s: %s (%s, %d)", "/proc/mounts",
                error->message, g_quark_to_string (error->domain), error->code);
        g_error_free (error);
        return NULL;
    }
    
    proc_mounts_watch_source = g_io_create_watch (proc_mounts_channel, G_IO_ERR);
    g_source_set_callback (proc_mounts_watch_source,
                        (GSourceFunc) proc_mounts_changed,
                        NULL, NULL);
    g_source_attach (proc_mounts_watch_source,
                    g_main_context_get_thread_default ());
    g_source_unref (proc_mounts_watch_source);
    g_io_channel_unref (proc_mounts_channel);
    
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    while(instance->isContinue) {
        sleep(3);
    }
    
	instance->runningServerThread = false;
	DEBUG_LOG1("mount_monitor_thread - end");
	return NULL;
}

static char *
get_program_name_from_pid (int     pid,
                           char   *buffer,
                           size_t  buffer_size)
{
    int fd = 0;
    ssize_t len = 0;
    char *aux = NULL;
    
    // Try to get program name by PID
    sprintf (buffer, "/proc/%d/cmdline", pid);
    if ((fd = open (buffer, O_RDONLY)) < 0)
        return NULL;
    
    if ((len = read (fd, buffer, buffer_size - 1)) <= 0)
    {
        close (fd);
        return NULL;
    }
    close (fd);
    
    buffer[len] = '\0';
    aux = strstr (buffer, "^@");
    if (aux)
        *aux = '\0';
    
    return buffer;
}

static char *
get_file_path_from_fd (int     fd,
                       char   *buffer,
                       size_t  buffer_size)
{
    ssize_t len = 0;
    
    if (fd <= 0)
        return NULL;
    
    sprintf (buffer, "/proc/self/fd/%d", fd);
    if ((len = readlink (buffer, buffer, buffer_size - 1)) < 0)
        return NULL;
    
    buffer[len] = '\0';
    return buffer;
}

std::string base_name(std::string const &path)
{
    return path.substr(path.find_last_of("/") + 1);
}

static bool enableDevice(const char * media_type)
{
	bool result = false;

	lsf_media_error_t *error = NULL;
	LSF_MEDIA_STATE state = LSF_MEDIA_NONE; 
	LSF_MEDIA_RESULT r = LSF_MEDIA_FAIL;

	if (media_type == NULL)
	{
		return result;
	}

	r = lsf_media_get_state( media_type, &state, &error);
	if (r == LSF_MEDIA_SUCCESS)
	{
		if (state == LSF_MEDIA_ALLOW)
		{
			if (error != NULL)
			{
				DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
				lsf_media_free_error(error);
			}

			return false;
		}
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	r = lsf_media_set_state( media_type, LSF_MEDIA_ALLOW, &error );
	if (r == LSF_MEDIA_SUCCESS)
	{
		result = true;
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_set_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	return result;
}

static bool controlDevice(const char *media_type, const char *pvi_media_type, int target_state)
{
	bool result = false;

	lsf_media_error_t *error = NULL;
	LSF_MEDIA_STATE state = LSF_MEDIA_NONE; 
	LSF_MEDIA_RESULT r = LSF_MEDIA_FAIL;

	if (media_type == NULL || pvi_media_type == NULL)
	{
		return result;
	}	

	r = lsf_media_get_state( media_type, &state, &error);
	if (r == LSF_MEDIA_SUCCESS)
	{
		//DEBUG_LOG( "command:lsf_media_get_state(%d),result-success,state-%d", LSF_MEDIA_BLUETOOTH, state);
		if (state == target_state)
		{
			if (error != NULL)
			{
				DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
				lsf_media_free_error(error);
			}

			return false;
		}
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	r = lsf_media_set_state( media_type, (LSF_MEDIA_STATE)target_state, &error );
	if (r == LSF_MEDIA_SUCCESS)
	{
		result = true;
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_set_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	return result;
}

static int
event_process (struct fanotify_event_metadata *event,
               int                             fanotify_fd,
               char *file_path)
{
    char program_path[PATH_MAX] = {0};
    int change_mod_value = 0;
    
    if (NULL == get_file_path_from_fd (event->fd, file_path, PATH_MAX))
    {
        struct fanotify_response access = {0};
        access.fd = event->fd;
        access.response = FAN_ALLOW;
        write (fanotify_fd, &access, sizeof (access));
        return change_mod_value;
    }
    
    printf ("Received event in path '%s'=>[%lld]", file_path, event->mask);
    printf (" pid=%d (%s): \n",
            event->pid,
            (get_program_name_from_pid (event->pid, program_path, PATH_MAX) ?
             program_path : "unknown"));
    
    LOG_PARAM  LogParam = {0};
    
    bool modified = true;
    int nProcessId = event->pid;
    std::string process_path = program_path;
    std::string process_name = base_name( process_path );
    std::string target_file_path = file_path;
    int nFlag = KAUTH_FILEOP_CLOSE_MODIFIED;
    
    //printf("ES_EVENT_TYPE_NOTIFY_CLOSE\n");
    printf("pid-[%d]\n", nProcessId);
    printf("proc.name-[%s]\n", process_name.c_str());
    printf("targe_file_path-[%s]\n", target_file_path.c_str());

    LogParam.nProcessId  = nProcessId;
    LogParam.pczProcName = (char*)process_name.c_str();
    LogParam.nAction = KAUTH_FILEOP_CLOSE;

    bool bAllow = false;
    bool bBlock = false;
    struct fanotify_response access = {0};
    access.fd = event->fd;
    access.response = FAN_ALLOW;

    if (event->mask & FAN_CLOSE_WRITE)
    {
        LogParam.nLogType    = LOG_FILEOP;

        bAllow = Kauth_FileOp_FileClose(nProcessId,
                                (char*)process_name.c_str(),
                                NULL,
                                (char*)target_file_path.c_str(),
                                nFlag,
                                &LogParam);
        printf( "[DLP][%s] Kauth_FileOp_FileClose(), bAllow=%d \n", __FUNCTION__, bAllow );
    }
    else
    {
        // do nothing!!!
    }

    write (fanotify_fd, &access, sizeof (access));

    return change_mod_value;
}

void* CPIESF::fnCommunicateClient(void* pzArg) {
	DEBUG_LOG1("mount_monitor_thread - begin");

    char file_path[PATH_MAX] = {0};
	CPIESF* instance;
	instance = reinterpret_cast<CPIESF*>(pzArg);
	instance->runningClientThread = true;

    while(instance->isContinue)
    {
        if (poll (instance->fds, FD_POLL_MAX, -1) < 0)
        {
            fprintf (stderr,
                     "Couldn't poll(): '%s'\n",
                     strerror (errno));
            exit (EXIT_FAILURE);
        }
        
        if (instance->fds[FD_POLL_FANOTIFY].revents & POLLIN)
        {
            char buffer[FANOTIFY_BUFFER_SIZE];
            ssize_t length;
            
            // Read from the FD. It will read all events available up to
            // the given buffer size.
            if ((length = read (instance->fds[FD_POLL_FANOTIFY].fd,
                                buffer,
                                FANOTIFY_BUFFER_SIZE)) > 0)
            {
                struct fanotify_event_metadata *metadata = NULL;
                
                metadata = (struct fanotify_event_metadata *)buffer;
                while (FAN_EVENT_OK (metadata, length))
                {
                    int change_mod_value = event_process (metadata, instance->fanotify_fd, file_path);
                    if (metadata->fd > 0)
                    {
                        close (metadata->fd);
                    }

                    metadata = FAN_EVENT_NEXT (metadata, length);
                }
            }
        }
    }

	instance->runningClientThread = false;
	DEBUG_LOG1("mount_monitor_thread - end");
	return NULL;
}

bool CPIESF::startThreads(void) {

	DEBUG_LOG1("mount_monitor_thread - begin");

	int result = 0;

    isContinue = true;

	if( false == isRunningServerThread() ) {
		pthread_attr_init( &serverThreadAttr);
		pthread_attr_setscope( &serverThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &serverThread, &serverThreadAttr, CPIESF::fnWaitClient, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for monitoring the mount event" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("mount_monitor_thread - skip - already running - mount event thread");
	}

	if( false == isRunningClientThread() ) {
		pthread_attr_init( &clientThreadAttr);
		pthread_attr_setscope( &clientThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &clientThread, &clientThreadAttr, CPIESF::fnCommunicateClient, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for monitoring the file-system" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("mount_monitor_thread - skip - already running - file-system event thread");
	}

	DEBUG_LOG1("mount_monitor_thread - end");
	return true;
}

#ifdef LINUX
void CPIESF::run(void)//int argc, const char * argv[])
{
	initialize();

	if( false == startThreads() ) {
		ERROR_LOG1("open_client_ipc - create_thread failed");
	}


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

#ifdef LINUX


static uint64_t event_mask =
(
    FAN_CLOSE_WRITE |    /* Writtable file closed */
    FAN_ONDIR |           /* We want to be reported of events in the directory */
    FAN_EVENT_ON_CHILD); /* We want to be reported of events in files of the directory */

void CPIESF::fnAddNotify(void* pzArg)
{
    if (pzArg == NULL || monitors == NULL)
        return;
    
    PVOLUME_DEVICE pDevice = (PVOLUME_DEVICE)pzArg;

    pthread_mutex_lock( &(CPIESF::getInstance().mutexClient) );

    // e.g.
    // [VolCtx_Update] NewAdd BusType=7, Device=/dev/disk2s2, BasePath=/Volumes/새 볼륨. Count=2
    monitors[ n_monitors ].path = strdup ( pDevice->czBasePath );
    if (monitors[ n_monitors ].path != NULL)
    {
        // Add new fanotify mark
        if (fanotify_mark (fanotify_fd,
                            FAN_MARK_ADD | FAN_MARK_MOUNT,
                            event_mask,
                            AT_FDCWD,
                            monitors[ n_monitors ].path) < 0)
        {
            fprintf (stderr,
                        "Add monitor in mount '%s' [%d] FAIL: '%s'\n",
                        monitors[ n_monitors ].path,
                        n_monitors,
                        strerror (errno));

            free( monitors[ n_monitors ].path );
        }
        else
        {
            fprintf (stdout,
                        "Add monitor in mount '%s' [%d] SUCCESS\n",
                        monitors[ n_monitors ].path,
                        n_monitors);        
            n_monitors++;
        }
    }
    else
    {
        fprintf (stderr,
                    "Add monitor in mount '%s' FAIL: '%s'\n",
                    monitors[ n_monitors ].path,
                    strerror (errno));        
    }

    pthread_mutex_unlock( &(CPIESF::getInstance().mutexClient) );
}

void CPIESF::fnRemoveNotify(void* pzArg)
{
    if (n_monitors <= 0 || monitors == NULL)
    {
        return;
    }

    pthread_mutex_lock( &(CPIESF::getInstance().mutexClient) );

    for(int i = 0; i < n_monitors; i++)
    {
        //monitors = (monitored_t*)malloc (n_monitors * sizeof (monitored_t));    
        if (monitors[i].path == NULL)
        {
            continue;
        }
        int ret = fanotify_mark (fanotify_fd,
                           FAN_MARK_REMOVE,
                           event_mask,
                           AT_FDCWD,
                           monitors[i].path);
        fprintf (stdout,
                    "fanotify_mark()-REMOVE-'%s':'%d'\n",
                    monitors[i].path,
                    ret);
        free (monitors[i].path);
    }
    n_monitors = 0;

    pthread_mutex_unlock( &(CPIESF::getInstance().mutexClient) );
}


void CPIESF_fnAddNotify(void* pzArg)
{
    return CPIESF::getInstance().fnAddNotify(pzArg);
}

void CPIESF_fnRemoveNotify(void* pzArg)
{
    return CPIESF::getInstance().fnRemoveNotify(pzArg);
}

#endif

#ifdef __cplusplus
}
#endif
