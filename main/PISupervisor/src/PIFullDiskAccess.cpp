
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
    return 0;
}


boolean_t
CPIFullDiskAccess::GetFDAEnable(void)
{
    return false;
}

int
CPIFullDiskAccess::GetFDACheckInterval(void)
{
    return 0;
}


boolean_t
CPIFullDiskAccess::IsPolicyExistCopyPrevent()
{
    return false;
}


boolean_t
CPIFullDiskAccess::IsPolicyExistShareFolderPrevent()
{
    return false;
}


boolean_t
CPIFullDiskAccess::ThreadStart()
{
    return false;
}


boolean_t
CPIFullDiskAccess::ThreadStop()
{
    return true;
}


boolean_t
CPIFullDiskAccess::SetFullDiskAccessCheck(void)
{
    return false;
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

   
