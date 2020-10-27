
#include "../include/PIFullDiskAccess.h"
#include "../include/PIDeviceMan.h"
#include "../include/PIDocument.h"
#include "../include/PIDeviceLog.h"
#include "nsPISecObject.h"
#include "PIConfigMan.h"
#include "PIPolicyMan.h"
#include "LogWriter.h"
#include <os/log.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/mount.h>


CPIFullDiskAccess::CPIFullDiskAccess() : m_nMountPos(0), m_bFDAThreadExit(false), m_pFDAThread(NULL)
{
}

CPIFullDiskAccess::~CPIFullDiskAccess()
{
}


void CPIFullDiskAccess::FDAJobRequest(void)
{
    if(true == IsPolicyExistShareFolderPrevent() || true == IsPolicyExistCopyPrevent())
    {
        if(GetFDAThread())
        {
            ThreadStop();
        }
        ThreadStart();
    }
    else
    {
        if(GetFDAThread())
        {
            ThreadStop();
            sleep(1);
        }
    }
}


void CPIFullDiskAccess::MountCtx_Clear(void)
{
    m_nMountPos = 0;
    memset( &m_Mount, 0, sizeof(m_Mount) );
    os_log( OS_LOG_DEFAULT, "[DLP][%s]", __FUNCTION__ );
}

boolean_t
CPIFullDiskAccess::MountCtx_Update(char* pczDeviceName, char* pczBasePath, ULONG nBusType)
{
    ULONG nPos=0, nMaxPos=0, nLength=0;
    
    if(!pczDeviceName || !pczBasePath)
    {
        return false;
    }
    
    nMaxPos = m_nMountPos;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        if(nPos >= MAX_MOUNT)
        {
            return false;
        }
        
        if(0 == strncmp( m_Mount[nPos].czDeviceName, pczDeviceName, strlen(pczDeviceName)))
        {
            if(nBusType == m_Mount[nPos].nBusType)
            {
                return true;
            }
            
            memset( &m_Mount[nPos], 0, sizeof(m_Mount[nPos]) );
            m_Mount[nPos].nBusType = nBusType;
            nLength = (ULONG)strlen(pczBasePath)+1;
            nLength = min( nLength, (MAX_BASEPATH-1) );
            strncpy( m_Mount[nPos].czBasePath, pczBasePath, nLength );
            nLength = (ULONG)strlen( pczDeviceName)+1;
            nLength = min(nLength, (MAX_DEVICE_SIZE-1) );
            strncpy( m_Mount[nPos].czDeviceName, pczDeviceName, nLength );
            
            printf( "DLP][%s] Exist, BusType=%d, Device=%s, BasePath=%s. Count=%d \n", __FUNCTION__, nBusType, pczDeviceName, pczBasePath, m_nMountPos );
            return true;
        }
    }
    
    m_Mount[nPos].nBusType = nBusType;
    nLength = (ULONG)strlen(pczBasePath)+1;
    nLength = min( nLength, (MAX_BASEPATH-1) );
    strncpy( m_Mount[nPos].czBasePath, pczBasePath, nLength );
    
    nLength = (ULONG)strlen( pczDeviceName)+1;
    nLength = min(nLength, (MAX_DEVICE_SIZE-1) );
    strncpy( m_Mount[nPos].czDeviceName, pczDeviceName, nLength );
    
    m_nMountPos++;
    
    printf( "[DLP][%s] New-Append. BusType=%d, Device=%s, BasePath=%s. Count=%d \n", __FUNCTION__, nBusType, pczDeviceName, pczBasePath, m_nMountPos );
    return TRUE;
}

boolean_t
CPIFullDiskAccess::IsDiskAccessPermission( char* czPath )
{
    struct statfs fileStat = { 0, };
    char czRand[8] = { '.', 0, };
    if(!czPath)
    {
        os_log(OS_LOG_DEFAULT, "[DLP][%s] Invalid Paramenters.", __FUNCTION__ );
        return true;
    }
    
    if(statfs(czPath, &fileStat) == 0)
    {
        os_log(OS_LOG_DEFAULT, "[DLP][%s] [%s]-FileSystem Type: [%s]", __FUNCTION__ , czPath, fileStat.f_fstypename);
    
        if(0 == strcmp(fileStat.f_fstypename, "ntfs") || 0 == strcmp(fileStat.f_fstypename, "cd9660"))
        {
            int res = open(czPath, O_RDONLY);
            if (res >= 0)
                return true;
            else
                return false;
        }
    }
    
    /*struct statfs fileStat;
    char czRand[8] = { '.', 0, };
    if(!czPath)
    {
        os_log(OS_LOG_DEFAULT, "[DLP][%s] Invalid Paramenters.", __FUNCTION__ );
        return true;
    }*/
    
    for (int i = 1; i < 7; i++)
    {
        czRand[i] = 'a' + rand() % 26;
    }
    
    size_t nSize = strlen(czPath) + strlen(czRand) + 1;
    char* czFullPath = (char *)malloc(nSize);
    if( !czFullPath )
    {
        os_log(OS_LOG_DEFAULT, "[DLP][%s] MemoryAllocation.", __FUNCTION__ );
        return true;
    }
    
    memset(czFullPath, 0, nSize);
    sprintf(czFullPath, "%s/%s", czPath, czRand);
    os_log(OS_LOG_DEFAULT, "[DLP][%s] czRand: %s", __FUNCTION__, czRand );
    os_log(OS_LOG_DEFAULT, "[DLP][%s] czPath: %s", __FUNCTION__, czPath );
    os_log(OS_LOG_DEFAULT, "[DLP][%s] czFullPath: %s", __FUNCTION__, czFullPath);
    
    int nRes = open(czFullPath, O_CREAT);
    os_log(OS_LOG_DEFAULT, "[DLP][%s] %s / O_CREAT nRes: %d", __FUNCTION__, czRand, nRes);
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
    os_log(OS_LOG_DEFAULT, "[DLP][%s] %s / O_WRONLY nRes: %d", __FUNCTION__, czRand, nRes);
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
    os_log(OS_LOG_DEFAULT, "[DLP][%s] %s / O_RDONLY nRes: %d", __FUNCTION__, czRand, nRes);
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
    os_log(OS_LOG_DEFAULT, "[DLP][%s] %s return true.", __FUNCTION__, czRand);
    return true;
}


size_t
CPIFullDiskAccess::FindDelimiter(char *cpPath)
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
CPIFullDiskAccess::GetFDAEnable(void)
{
    return ConfigMan.m_bFDAEnable;
}

int
CPIFullDiskAccess::GetFDACheckInterval(void)
{
    return ConfigMan.m_nFDACheckInterval;
}


boolean_t
CPIFullDiskAccess::IsPolicyExistCopyPrevent()
{
    if(PolicyMan.getCurrentDLPPolicy( nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeCopyPreventPlus ))
    {
        return true;
    }
    
    if(PolicyMan.getCurrentDLPPolicy( nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeCopyExPreventPlus ))
    {
        return true;
    }
    return false;
}


boolean_t
CPIFullDiskAccess::IsPolicyExistShareFolderPrevent()
{
    if(PolicyMan.getCurrentDLPPolicy( nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeSharedFolderPreventPlus ))
    {
        return true;
    }
    return false;
}


boolean_t
CPIFullDiskAccess::ThreadStart()
{
    int nReturn=0;
    if(!m_pFDAThread && (true == IsPolicyExistCopyPrevent() || true == IsPolicyExistShareFolderPrevent()) )
    {
        nReturn = pthread_create( &m_pFDAThread, NULL, CPIFullDiskAccess::FullDiskAccessCheckThread, (void*)this );
        if(nReturn < 0)
        {
            return false;
        }
        m_bFDAThreadExit = false;
        DEBUG_LOG( "[DLP][%s] Start. ", __FUNCTION__ );
        return true;
    }
    return false;
}


boolean_t
CPIFullDiskAccess::ThreadStop()
{
    if(!m_pFDAThread)
    {
        return false;
    }
    pthread_cancel( m_pFDAThread );
    m_pFDAThread = NULL;
    m_bFDAThreadExit = true;
    DEBUG_LOG( "[DLP][%s] Exit. ", __FUNCTION__ );
    return true;
}


boolean_t
CPIFullDiskAccess::SetFullDiskAccessCheck(void)
{
    ULONG        nBusType = 0;
    int          nPos=0, nMaxPos=0, nResult=0, nResponse=0;
    boolean_t    bPermission = true;
    boolean_t    bCopyPrevent=false, bShareFolderPrevent=false;
    // char         czCommand[MAX_PATH] = {0};
    MOUNT_VOLUME Mount = {0};
    
    if(m_nMountPos <= 0)
    {
        return false;
    }
    
    bCopyPrevent = IsPolicyExistCopyPrevent();
    bShareFolderPrevent = IsPolicyExistShareFolderPrevent();
    
    os_log(OS_LOG_DEFAULT, "[DLP][%s]", __FUNCTION__ );
    
    nMaxPos = m_nMountPos;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        if(nPos >= MAX_MOUNT)
        {
            break;
        }
        memset( &Mount, 0, sizeof(Mount) );
        memcpy( &Mount, &m_Mount[nPos], sizeof(Mount) );
        nBusType = Mount.nBusType;
        
        if(true == bShareFolderPrevent || true == bCopyPrevent)
        {   // ShareFolderPrevent
            bPermission = IsDiskAccessPermission( Mount.czBasePath );
        }
        else
        {
            os_log(OS_LOG_DEFAULT, "[DLP][%s] Policy Not Exist. ", __FUNCTION__ );
        }
        
        if(false == bPermission)
        {
            // memset( czCommand, 0, sizeof(czCommand) );
            // sprintf( czCommand, "diskutil unmount \"%s\"", Mount.czBasePath );
            // nResult = system( czCommand );
            // if(nResult != 0) nResult = system( czCommand );
            //
            nResult = unmount( Mount.czBasePath, MNT_CMDFLAGS );
            os_log(OS_LOG_DEFAULT, "[DLP][%s] nResult=%d, Error=%d", __FUNCTION__, nResult, errno );
            if(nResult == 0)
            {
                PIAgentStub.notifyNeedFullDiskAccessAuth( nResponse );
            }
        }
    }
    MountCtx_Clear();
    return bPermission;
}


void*
CPIFullDiskAccess::FullDiskAccessCheckThread( void*  pParam )
{
    CPIFullDiskAccess* pFullDiskAccess = NULL;
    int nCheckInterval = DEFAULT_INTERVAL;
    
    DEBUG_LOG( "[DLP][%s] Start.", __FUNCTION__ );
    pFullDiskAccess = (CPIFullDiskAccess*)pParam;
    if(!pFullDiskAccess)
    {
        DEBUG_LOG( "[DLP][%s] Invalid Pointer.", __FUNCTION__ );
        return NULL;
    }
    
    if(false == pFullDiskAccess->GetFDAEnable())
    {
        DEBUG_LOG( "[DLP][%s] FullDiskAccessCheck enable=%d. Thread Exit.", __FUNCTION__, pFullDiskAccess->GetFDAEnable() );
        return NULL;
    }
    
    nCheckInterval = pFullDiskAccess->GetFDACheckInterval();
    if(nCheckInterval <= 0)
    {
        nCheckInterval = DEFAULT_INTERVAL;
    }
    
    do
    {
        pFullDiskAccess->SetFullDiskAccessCheck();
        sleep( nCheckInterval );
    } while( false == pFullDiskAccess->m_bFDAThreadExit );
    
    pthread_exit( (void*)1);
    DEBUG_LOG( "[DLP][%s] Stop. ", __FUNCTION__ );
    return ((void*)1);
}

   
