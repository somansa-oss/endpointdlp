#ifndef _PIESF_h_
#define _PIESF_h_

#include "PIObject.h"

#ifdef LINUX
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/signalfd.h>
#include <fcntl.h>

#include <linux/fanotify.h>
#include <sys/fanotify.h>

#define FANOTIFY_BUFFER_SIZE 1024 * 10

enum {
//    FD_POLL_SIGNAL = 0,
    FD_POLL_FANOTIFY = 0,
//    FD_POLL_FANOTIFY_OPEN_PERM,
    FD_POLL_MAX
};

typedef struct _monitored {
    char *path;
} monitored_t;

#endif

class CPIESF : public CPIObject {
public:
    CPIESF();
    ~CPIESF();
    
private:
    bool m_bIsContinue;

protected:
    pthread_t serverThread;
    pthread_attr_t serverThreadAttr;
    pthread_mutex_t mutexClient;
    pthread_cond_t condClient;
    pthread_t clientThread;
    pthread_attr_t clientThreadAttr;
    bool isContinue;
    bool runningServerThread;
    bool runningClientThread;

#ifdef LINUX
    int fanotify_fd;
    struct pollfd fds[FD_POLL_MAX];

public:
    static monitored_t *monitors;
    static int n_monitors;
#endif

protected:
    bool startThreads(void);
    void waitThreads(void);
    bool isRunningServerThread(void);
    bool isRunningClientThread(void);

public:
    bool isActive(void);
    virtual bool initialize(void);
    virtual bool finalize(void);

public:
    static CPIESF& getInstance( void );

#ifdef LINUX
    static void* fnCommunicateClient(void* pzArg);
    static void* fnWaitClient(void* pzArg);    

    void fnAddNotify(void* pzArg);    
    void fnRemoveNotify(void* pzArg);    
#endif

    void run(void);
    int stop(void);
};

#endif /* _PIESF_h_ */
