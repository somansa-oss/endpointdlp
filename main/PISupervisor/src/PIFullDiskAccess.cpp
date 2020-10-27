
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
}


void CPIFullDiskAccess::MountCtx_Clear(void)
{
}

boolean_t
CPIFullDiskAccess::MountCtx_Update(char* pczDeviceName, char* pczBasePath, ULONG nBusType)
{
    return true;
}

boolean_t
CPIFullDiskAccess::IsDiskAccessPermission( char* czPath )
{
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

   
