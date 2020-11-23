#include <string>
#include <vector>

#include "PCSecurity.h"
#include "ShareFolderCtrl.h"
#include <unistd.h>
#include <pthread.h>
#include "LogWriter.h"

CPCSecurity::CPCSecurity() : m_pSFThread((pthread_t)NULL), m_bThreadExit(true)
{
    pthread_mutex_init( &m_SFMutex, NULL );
}

CPCSecurity::~CPCSecurity()
{
    m_bThreadExit = false;
    pthread_mutex_destroy( &m_SFMutex );
}

bool CPCSecurity::Load(CShareFolder& cSFolder)
{
    LoadPCSecurityInfo( cSFolder );
    ApplyShareFolder();
    return true;
}


long CPCSecurity::LoadPCSecurityInfo(CShareFolder& cSFolder)
{
    char czModulePath[MAX_PATH] = {0};
    memset( czModulePath, 0, sizeof(czModulePath) );
    
    if(true == cSFolder.getBlockAll())
    {
        m_SFCtrl.GetSFPolicy().setBlockAll( true );
        m_SFCtrl.GetSFPolicy().setBlockEveryOne( true );
        m_SFCtrl.GetSFPolicy().setBlockDefault( true );
    }
    else
    {
        m_SFCtrl.GetSFPolicy().setBlockAll( false );
        if(true == cSFolder.getBlockEveryOne())
        {
            m_SFCtrl.GetSFPolicy().setBlockEveryOne( true );
        }
        else
        {
            m_SFCtrl.GetSFPolicy().setBlockEveryOne( false );
        }
        
        if(true == cSFolder.getBlockDefault())
        {
            m_SFCtrl.GetSFPolicy().setBlockDefault( true );
        }
        else
        {
            m_SFCtrl.GetSFPolicy().setBlockDefault( false );
        }
    }
    
    printf( "[PISupervisor][%s] BlockAll=%d, BlockEveryone=%d, BlockDefault=%d \n",
            __FUNCTION__,
            m_SFCtrl.GetSFPolicy().getBlockAll(), m_SFCtrl.GetSFPolicy().getBlockEveryOne(), m_SFCtrl.GetSFPolicy().getBlockDefault() );
    
    m_SFCtrl.GetSFPolicy().setCheckInterval( cSFolder.getCheckInterval() );
    return 0;
}


void CPCSecurity::ApplyShareFolder()
{
    // Lock
    pthread_mutex_lock( &m_SFMutex );
    
    m_SFCtrl.ShareFolderEnum();
    if(true == m_SFCtrl.GetSFPolicy().getBlockDefault())
    {
        m_SFCtrl.ShareFolderDisableDefault();
    }
    
    if(true == m_SFCtrl.GetSFPolicy().getBlockEveryOne())
    {
        m_SFCtrl.ShareFolderDisableEveryOne();
    }
    
    if(true == m_SFCtrl.GetSFPolicy().getBlockAll())
    {
        m_SFCtrl.ShareFolderDisableAll();
    }
    
    pthread_mutex_unlock( &m_SFMutex );
}


bool CPCSecurity::PCSecurity_SFThread_Start()
{
    int nReturn=0;
    if(!m_pSFThread && true == GetSFPolicy().IsExistPolicy())
    {
        nReturn = pthread_create( &m_pSFThread, NULL, CPCSecurity::PCSecuritySFolderThreadProc, (void*)this );
        if(nReturn < 0)
        {
            return false;
        }
        m_bThreadExit = false;
        
        INFO_LOG1("PCSecuritySFolderThreadProc Request-Start.");
        return true;
    }
    return false;
}


bool CPCSecurity::PCSecurity_SFThread_Stop()
{
    if(!m_pSFThread) return false;
    
    pthread_cancel( m_pSFThread );
    m_pSFThread = (pthread_t)NULL;
    m_bThreadExit = true;
    INFO_LOG1("PCSecuritySFolderThreadProc Request-Stop.");
    return true;
}


void* CPCSecurity::PCSecuritySFolderThreadProc(void* pParam)
{
    CPCSecurity* pPCSecurity = NULL;
    int nIntervalTime = 60;
    
    pPCSecurity = (CPCSecurity*)pParam;
    if(!pPCSecurity)
    {
        return NULL;
    }
    
    if(pPCSecurity->GetSFPolicy().getCheckInterval() > 120 || pPCSecurity->GetSFPolicy().getCheckInterval() < 0)
    {
        nIntervalTime = 60;
    }
    else
    {
        nIntervalTime = pPCSecurity->GetSFPolicy().getCheckInterval();
    }
    
    do
    {
        if(pPCSecurity->GetSFPolicy().IsExistPolicy())
        {
            pPCSecurity->ApplyShareFolder();
        }
        sleep(5);
    } while(false == pPCSecurity->m_bThreadExit);
    
    pthread_exit( (void*)1);
    INFO_LOG1("PCSecuritySFolderThreadProc Stopped.");
    return ((void*)1);
}







