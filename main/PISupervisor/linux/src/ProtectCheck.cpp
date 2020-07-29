
#include "ProtectCheck.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <errno.h>
#include <libproc.h>




using namespace std;



ProtectCheck::ProtectCheck()
{
    
}


ProtectCheck::~ProtectCheck()
{
    
}

std::string
ProtectCheck::ReadCommandOutput(std::string strCommand)
{
    FILE* pPipe     = NULL;
    string strResult = "";
    char  czBuffer[MAX_COMMAND];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    pPipe = popen( strCommand.c_str(), "r" );
    if(NULL != pPipe)
    {
        while(!feof(pPipe))
        {
            if(fgets( czBuffer, MAX_COMMAND, pPipe) != NULL)
            {
                strResult += czBuffer;
            }
        }
        pclose(pPipe);
    }
    return strResult;
}


ULONG
ProtectCheck::GetProcessIDCommand( const char* pczCommand )
{
    ULONG  nPID=0, nLength=0;
    string strCommand, strTemp;
    char    czProcName[128];
    
    if(!pczCommand)
    {
        printf( "[DLP-App][%s] InvalidParameter \n", __FUNCTION__  );
        return 0;
    }
    
    strCommand = pczCommand;
    if(true == strCommand.empty())
    {
        printf( "[DLP-App][%s] string empty \n", __FUNCTION__  );
        return 0;
    }
    
    strTemp = ReadCommandOutput( strCommand );
    if(true == strTemp.empty())
    {
        return 0;
    }
    
    try
    {
        nPID = atoi( strTemp.c_str() );
        memset( czProcName, 0, sizeof(czProcName) );
        proc_name( nPID, czProcName, sizeof(czProcName) );
        nLength = (ULONG)strlen(czProcName);
        if(nLength <= 0)
        {
            nPID = -1;
        }
    }
    catch(...)
    {
    }
    
    return nPID;
}



ULONG ProtectCheck::GetProcessID(const char* pczProcName)
{
    int   nMaxProc=0, nLength=0, nPos=0;
    pid_t PIDArray[ 2048 ];
    char  czCurPath[ PROC_PIDPATHINFO_MAXSIZE ];
    char  czCurName[ PROC_PIDPATHINFO_MAXSIZE ];
    
    if(!pczProcName)
    {
        printf( "[DLP-App][%s] Invalid Parameter. \n", __FUNCTION__  );
        return 0;
    }
    
    nMaxProc = proc_listpids( PROC_ALL_PIDS, 0, NULL, 0 );
    
    memset( &PIDArray, 0, sizeof(PIDArray) );
    proc_listpids( PROC_ALL_PIDS, 0, PIDArray, sizeof(PIDArray) );
    
    for(int i=0; i<nMaxProc; i++)
    {
        if(!PIDArray[i]) continue;
        
        memset( czCurPath, 0, sizeof(czCurPath) );
        proc_pidpath( PIDArray[i], czCurPath, sizeof(czCurPath) );
        
        nLength = (int)strlen(czCurPath );
        if(nLength)
        {
            nPos = nLength;
            
            while(nPos && czCurPath[ nPos ] != '/')
                --nPos;
            
            strcpy( czCurName, czCurPath+ nPos + 1);
            if(!strcmp( czCurName, pczProcName ))
            {
                return PIDArray[i];
            }
        }
    }
    return 0;
}


bool ProtectCheck::GetProcessPath( ULONG nPID, char* pczOutPath, uint32_t nMaxOutPath )
{
    int    nRet = 0;
    size_t nLength = 0;
    char   czProcPath[PROC_PIDPATHINFO_MAXSIZE];
    
    memset( czProcPath, 0, sizeof(czProcPath) );
    nRet = proc_pidpath( nPID, czProcPath, sizeof(czProcPath) );
    if(nRet <= 0)
    {
        printf( "[%s] pid=%d, err=%d, err-msg=%s \n", __FUNCTION__, nPID, errno, strerror(errno) );
        return false;
    }
    
    nLength = MIN( strlen(czProcPath), nMaxOutPath );
    strncpy( pczOutPath, czProcPath, nLength );
    
    printf( "[%s] ProcessId=%d, ProcessPath=%s \n", __FUNCTION__, nPID, czProcPath );
    return true;
}




