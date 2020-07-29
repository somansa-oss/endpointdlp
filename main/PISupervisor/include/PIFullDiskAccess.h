
#ifndef _PI_FULLDISKACCESS_H_
#define _PI_FULLDISKACCESS_H_

#include <pthread.h>
#include "../apple/include/KernelProtocol.h"


#ifdef __cplusplus
extern "C"
{
#endif
    
#define DEFAULT_INTERVAL  2


class CPIFullDiskAccess
{
public:
    CPIFullDiskAccess();
    ~CPIFullDiskAccess();
    
public:
    int GetFDACheckInterval(void);
    boolean_t GetFDAEnable(void);
    boolean_t SetFullDiskAccessCheck(void);
    size_t    FindDelimiter(char *cpPath);
    static boolean_t IsDiskAccessPermission( char* czPath );

public:
    void FDAJobRequest(void);
    boolean_t IsPolicyExistCopyPrevent(void);
    boolean_t IsPolicyExistShareFolderPrevent(void);
    static void* FullDiskAccessCheckThread(void* pParam);
    boolean_t ThreadStop(void);
    boolean_t ThreadStart(void);
public:
    void SetFDAThreadExit(boolean_t bFDAThreadExit) { m_bFDAThreadExit = bFDAThreadExit; }
    boolean_t GetFDAThreadExit() { return m_bFDAThreadExit; }
    void SetFDAThread(pthread_t pFDAThread) { m_pFDAThread = pFDAThread; }
    pthread_t GetFDAThread() { return m_pFDAThread; }
    
    void MountCtx_Clear(void);
    boolean_t MountCtx_Update(char* pczDeviceName, char* pczBasePath, ULONG nBusType);
    
protected:
    int  m_nMountPos;
    MOUNT_VOLUME m_Mount[MAX_MOUNT];
private:
    boolean_t  m_bFDAThreadExit;
    pthread_t  m_pFDAThread;
    pthread_mutex_t  m_FDAMutex;
    
};





#ifdef __cplusplus
};
#endif

#endif /* _PI_FULLDISKACCESS_H_ */
