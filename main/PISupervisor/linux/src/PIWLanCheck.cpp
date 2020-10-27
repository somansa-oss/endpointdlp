
#include "PIWLanCheck.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <errno.h>
#include <libproc.h>
#include <dlfcn.h>

#define MAX_PATH 260
#define PIWLANLIB_PATH  "/usr/local/Privacy-i/libPIWLanLib.dylib"

using namespace std;


CPIWLanCheck g_SSID;

CPIWLanCheck::CPIWLanCheck()
: m_pHandle(NULL),
  m_pWLanLibPolicyCheck(NULL),
  m_pWLanLibSetPolicySSID(NULL),
  m_pWLanLibGetBlockSSID(NULL)
{
    LibInit( NULL );
    m_strBlockSSID = "";
}

CPIWLanCheck::~CPIWLanCheck()
{
    LibUninit();
}


bool CPIWLanCheck::LibUninit()
{
    if(m_pHandle)
    {
        dlclose( m_pHandle );
        m_pHandle = NULL;
    }
    return true;
}


bool CPIWLanCheck::LibInit(const char* pczDyLibName)
{
    char czLibName[MAX_PATH] = {0};
    
    memset(czLibName, 0, sizeof(czLibName));
    if(!pczDyLibName)
    {
        strcpy( czLibName, PIWLANLIB_PATH );
    }
    else
    {
        strcpy( czLibName, pczDyLibName );
    }
    
    if(m_pHandle)
    {
        dlclose( m_pHandle );
        m_pHandle = NULL;
    }
    
    m_pHandle = dlopen( czLibName, RTLD_NOW );
    if(!m_pHandle)
    {
        printf("[DLPApp][%s] Fail. LibName=%s errno=%d. errstr=%s \n", __FUNCTION__, czLibName, errno, dlerror() );
        return false;
    }
    
    
    m_pWLanLibPolicyCheck = (fpWLanLibPolicyCheck)dlsym( m_pHandle, "WLanLibPolicyCheck" );
    if(!m_pWLanLibPolicyCheck)
    {
        printf("[DLPApp][%s] GetFunctionPointer( WLanLibPolicyCheck ) errno=%d errstr=%s. \n", __FUNCTION__, errno, dlerror() );
        return false;
    }
    
    m_pWLanLibSetPolicySSID = (fpWLanLibSetPolicySSID)dlsym( m_pHandle, "WLanLibSetPolicySSID" );
    if(!m_pWLanLibSetPolicySSID)
    {
        printf("[DLPApp][%s] GetFunctionPointer( WLanLibSetPolicySSID ) errno=%d errstr=%s. \n", __FUNCTION__, errno, dlerror() );
        return false;
    }
    
    m_pWLanLibGetBlockSSID = (fpWLanLibGetBlockSSID)dlsym( m_pHandle, "WLanLibGetBlockSSID" );
    if(!m_pWLanLibGetBlockSSID)
    {
        printf("[DLPApp][%s] GetFunctionPointer( WLanLibGetBlockSSID ) errno=%d errstr=%s. \n", __FUNCTION__, errno, dlerror() );
        return false;
    }
    
    m_pWLanLibGetIfaceName = (fpWLanLibGetIfaceName)dlsym( m_pHandle, "WLanLibGetIfaceName" );
    if(!m_pWLanLibGetIfaceName)
    {
        printf("[DLPApp][%s] GetFunctionPointer( WLanLibGetIfaceName ) errno=%d errstr=%s. \n", __FUNCTION__, errno, dlerror() );
        return false;
    }
    return true;
}


string
CPIWLanCheck::GetBlockSSID()
{
    return m_strBlockSSID;
}

string
CPIWLanCheck::GetIfaceName(void)
{
    return m_strIfaceName;
}

bool CPIWLanCheck::LibWLanLibGetBlockSSID()
{
    if(!m_pWLanLibGetBlockSSID)
    {
        return false;
    }
    return m_pWLanLibGetBlockSSID( m_strBlockSSID );
}

bool CPIWLanCheck::LibWLanLibGetIfaceName()
{
    if(!m_pWLanLibGetIfaceName)
    {
        return false;
    }
    return m_pWLanLibGetIfaceName( m_strIfaceName );
}


bool CPIWLanCheck::LibWLanLibPolicyCheck()
{
    if(!m_pWLanLibPolicyCheck)
    {
        return false;
    }
    return m_pWLanLibPolicyCheck();
}


bool CPIWLanCheck::LibWLanLibSetPolicySSID( vector<string> vecPolicySSID )
{
    if(!m_pWLanLibSetPolicySSID)
    {
        return false;
    }
    return m_pWLanLibSetPolicySSID( vecPolicySSID );
}


bool CPIWLanCheck::SetWLanSSIDControl( std::vector<std::string> vecWLanPermitList )
{
    bool bBlock = false;

    LibWLanLibSetPolicySSID( vecWLanPermitList );

    bBlock = LibWLanLibPolicyCheck();
    if(true == bBlock)
    {
        LibWLanLibGetBlockSSID();
    }
    return bBlock;
}























