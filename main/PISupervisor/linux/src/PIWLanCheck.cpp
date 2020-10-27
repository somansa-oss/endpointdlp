
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
}

CPIWLanCheck::~CPIWLanCheck()
{
}


bool CPIWLanCheck::LibUninit()
{
}

bool CPIWLanCheck::LibInit(const char* pczDyLibName)
{
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
    return false;
}

bool CPIWLanCheck::LibWLanLibGetIfaceName()
{
    return false;
}


bool CPIWLanCheck::LibWLanLibPolicyCheck()
{
    return false;
}


bool CPIWLanCheck::LibWLanLibSetPolicySSID( vector<string> vecPolicySSID )
{
    return false;
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























