#include <string>
#include <vector>

#include "ShareFolderCtrl.h"

using namespace std;


CShareFolderCtrl::CShareFolderCtrl()
{
    m_vecSFList.clear();
}

CShareFolderCtrl::~CShareFolderCtrl()
{
    m_vecSFList.clear();
}


int CShareFolderCtrl::SelectKextMenu()
{
    int nSelect = 0;
    printf("1. FileShareFile.  \n");
    printf("2. FileShareDeleteAll. \n");
    printf("3. ApplyPolicyPCSerity. \n");
    printf("9. ProgramExit. \n");
    
    printf("Select: ");
    scanf( "%d", &nSelect );
    fflush(stdin);
    return nSelect;
}



std::string
CShareFolderCtrl::StringReplaceAll( const std::string& strMessage, const std::string&& strPattern, const std::string&& strReplace )
{
    std::string strResult = strMessage;
    std::string::size_type nPos = 0;
    
    while((nPos = strResult.find( strPattern )) != std::string::npos)
    {
        strResult.replace( nPos, strPattern.size(), strReplace );
    }
    return strResult;
}


bool CShareFolderCtrl::ShellExecute( std::string strCommand, std::string& strResult )
{
    FILE*  pPipe = NULL;
    char   czBuffer[1024];
    int  nLength=0;
    
    pPipe = popen( strCommand.c_str(), "r" );
    if(!pPipe) return false;
    
    while(!feof(pPipe))
    {
        memset(czBuffer, 0, sizeof(czBuffer) );
        if(fgets(czBuffer, 1024, pPipe) != NULL)
        {
            nLength = (int)strlen(czBuffer)-1;
            czBuffer[ nLength ] = '\0';
            strResult += czBuffer;
            printf( "[%s] Token=%s, Result=%s, Command=%s \n", __FUNCTION__, czBuffer, strResult.c_str(), strCommand.c_str() );
        }
    }
    pclose( pPipe );
    return true;
}


bool CShareFolderCtrl::ShellExecuteFetchSFRecordList( std::string strCommand, std::vector<SF_RECORD>& vecSFList )
{
    FILE*  pPipe = NULL;
    char czBuffer[1024];
    int  nLength=0;
    SF_RECORD SFInfo;
    
    pPipe = popen( strCommand.c_str(), "r" );
    if(!pPipe) return false;
    
    while(!feof(pPipe))
    {
        memset(czBuffer, 0, sizeof(czBuffer) );
        if(fgets(czBuffer, 1024, pPipe) != NULL)
        {
            nLength = (int)strlen(czBuffer)-1;
            czBuffer[ nLength ] = '\0';
            
            memset( &SFInfo, 0, sizeof(SFInfo) );
            strncpy( SFInfo.czSFRecord, czBuffer, strlen(czBuffer) );
            vecSFList.push_back( SFInfo );            
            printf( "[%s] Token=%s, Command=%s \n", __FUNCTION__, czBuffer, strCommand.c_str() );
        }
    }
    pclose( pPipe );
    return true;
}


bool CShareFolderCtrl::ShellExecuteFetchSFFilePath( std::string strCommand, std::string& strOutFilePath )
{
    FILE*  pPipe = NULL;
    char  czBuffer[1024];
    int  nLength=0;

#ifdef _FIXME_

    pPipe = popen( strCommand.c_str(), "r" );
    if(!pPipe) return false;
    
    while(!feof(pPipe))
    {
        memset(czBuffer, 0, sizeof(czBuffer) );
        if(fgets(czBuffer, 1024, pPipe) != NULL)
        {
            nLength = (int)strlen(czBuffer)-1;
            czBuffer[ nLength ] = '\0';
            strOutFilePath += czBuffer;
            strOutFilePath = StringReplaceAll( czBuffer, "dsAttrTypeNative:directory_path:", "" );
            strOutFilePath.erase( std::remove(strOutFilePath.begin(), strOutFilePath.end(), ' '), strOutFilePath.end() );
            // printf( "[%s] Result=%s, Command=%s \n", __FUNCTION__, strOutFilePath.c_str(), strCommand.c_str() );
        }
    }
    pclose( pPipe );

#endif

    return true;
}


void CShareFolderCtrl::ShareFolderPrint()
{
     SF_RECORD SFInfo;
    std::string strShareFolder;
    for(int nIndex=0; nIndex<m_vecSFList.size(); nIndex++)
    {
        memset( &SFInfo, 0, sizeof(SFInfo) );
        SFInfo = m_vecSFList[nIndex];
        printf( "[PISupervisor][%s] SFRecord=%s, SFFilePath=%s, EveryOne=%d \n", __FUNCTION__, SFInfo.czSFRecord, SFInfo.czSFFilePath, SFInfo.bEveryOne );
    }
}

bool CShareFolderCtrl::ShareFolderEnum()
{
    bool bSuc=false;
    std::string strCommand;
    std::string strResult;
    char czCommand[260];
    SF_RECORD SFInfo;
    
    m_vecSFList.clear();
    memset( czCommand, 0, sizeof(czCommand) );
    snprintf( czCommand, sizeof(czCommand), "dscl . -list /SharePoints" );
    strCommand = czCommand;
    
    bSuc = ShellExecuteFetchSFRecordList( strCommand, m_vecSFList );
    printf( "[PISupervisor][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );
    if(bSuc == false)
    {
        return false;
    }
    
    for(int nIndex=0; nIndex<m_vecSFList.size(); nIndex++)
    {
        memset( &SFInfo, 0, sizeof(SFInfo) );
        SFInfo = m_vecSFList[nIndex];
        snprintf( czCommand, sizeof(czCommand), "dscl . -read \"/SharePoints/%s\" directory_path", SFInfo.czSFRecord );
        strCommand = czCommand;
        bSuc = ShellExecuteFetchSFFilePath( strCommand, strResult );
        strncpy( SFInfo.czSFFilePath, strResult.c_str(), strResult.length() );
        stat( SFInfo.czSFFilePath, &SFInfo.SFStat );
        if(SFInfo.SFStat.st_mode & (S_IROTH | S_IWOTH | S_IXOTH))
        {
            SFInfo.bEveryOne = true;
        }
        m_vecSFList[nIndex] = SFInfo;
    }
    
    ShareFolderPrint();
    return true;
}

bool CShareFolderCtrl::ShareFolderDelete(std::string strSFolder)
{
    bool bSuc=false;
    std::string strCommand;
    std::string strResult;
    char czCommand[260];
    
    memset( czCommand, 0, sizeof(czCommand) );
    snprintf( czCommand, sizeof(czCommand), "sudo dscl . -delete \"/SharePoints/%s\" ",  strSFolder.c_str() );
    
    strCommand = czCommand;
    bSuc = ShellExecute( strCommand, strResult );
    printf( "[PISupervisor][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );
    return true;
}


bool CShareFolderCtrl::ShareFolderDisableAll()
{
    SF_RECORD SFInfo;
    for(int nIndex=0; nIndex<m_vecSFList.size(); nIndex++)
    {
        memset( &SFInfo, 0, sizeof(SFInfo) );
        SFInfo = m_vecSFList[nIndex];
        ShareFolderDelete( SFInfo.czSFRecord );
    }
    printf( "[PISupervisor][%s] \n", __FUNCTION__ );
    return true;
}


bool CShareFolderCtrl::ShareFolderDisableDefault()
{
    bool bReturn=false;
    
    bReturn = ShareFolderDisableAll();
    printf( "[PISupervisor][%s] \n", __FUNCTION__ );
    return bReturn;
}

bool CShareFolderCtrl::ShareFolderDisableEveryOne()
{
    SF_RECORD SFInfo;
    for(int nIndex=0; nIndex<m_vecSFList.size(); nIndex++)
    {
        memset( &SFInfo, 0, sizeof(SFInfo) );
        SFInfo = m_vecSFList[nIndex];
        if(true == SFInfo.bEveryOne)
        {
            ShareFolderDelete( SFInfo.czSFRecord );
        }
    }
    printf( "[PISupervisor][%s] \n", __FUNCTION__ );
    return true;
}


