#include <string>

#include "KextManager.h"
#include "ProtectCheck.h"
#include "KernelProtocol.h"
#include "KernelCommand.h"
#include "PWKctl.h"
#include "ProtectCheck.h"
#include "LogWriter.h"

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

LPCALLBACK   g_AppCallback = NULL;
CKextManager g_AppKext;


int CKextManager::EventNotify_PostProcess( PEVT_PARAM pEvtInfo )
{
    int nRet = 0;
    
    if(!pEvtInfo) return 0;
    
    switch(pEvtInfo->Command)
    {
            
        // PISecSmartDrv.kext
        case FileIsRemove:
            nRet = g_AppKext.EventNotify_FileIsRemove( pEvtInfo );
            break;
        case FileScan:
            nRet = g_AppKext.EventNotify_FileScan( pEvtInfo );
            break;
        case FileDelete:
            nRet = g_AppKext.EventNotify_FileDelete( pEvtInfo );
            break;
        case FileExchangeData:
            nRet = g_AppKext.EventNotify_FileExchangeData( pEvtInfo );
            break;
        case FileRename:
            nRet = g_AppKext.EventNotify_FileRename( pEvtInfo );
            break;
        case FileEventDiskFull:
            nRet = g_AppKext.EventNotify_FileEventDiskFull( pEvtInfo );
            break;
       
        case SmartLogNotify:
            nRet = g_AppKext.EventNotify_SmartLogNotify( pEvtInfo );
            break;
        
		case ProcessAccessCheck:
            nRet = g_AppKext.EventNotify_ProcessAccessCheck( pEvtInfo );
            break;
            
        // PWDisk.kext
        case PWNotify_ProcessNotify:
            nRet = g_AppKext.PWDisk_EN_Process( pEvtInfo );
            break;
            
        case PWNotify_PWDiskLogNotify:
            nRet = g_AppKext.PWDisk_EN_Log( pEvtInfo );
            break;
            
            
        default: break;
    }

    return nRet;
}

/*****************************************************************************************/

int CKextManager::EventNotify_FileIsRemove(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    printf( "[DLP][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CKextManager::EventNotify_FileScan(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, Path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    printf( "[DLP][%s] pid=%d, QtPath=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pQtFilePath );
    return 0;
}


int CKextManager::EventNotify_FileDelete(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CKextManager::EventNotify_FileRename(PEVT_PARAM pEvtInfo )
{
    
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CKextManager::EventNotify_FileExchangeData(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CKextManager::EventNotify_FileEventDiskFull(PEVT_PARAM pEvtInfo )
{
    
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}


int CKextManager::EventNotify_SmartLogNotify(PEVT_PARAM pEvtInfo )
{
    PSMART_LOG_RECORD_EX pLogEx = NULL;
    
    if(!pEvtInfo) return 0;
    
    pLogEx = (PSMART_LOG_RECORD_EX)pEvtInfo->pEvtCtx;
    if(!pLogEx) return 0;
    
    printf( "[%s][DLP][%s] policy=%d AccessType=%d, RecordType=%d, Major=%d, Minor=%d, Result=%d, pid=%d, proc=%s path=%s \n",
            pLogEx->Log.Data.LogTime,
            __FUNCTION__,
            pLogEx->PolicyType,
            pLogEx->Log.AccessType,
            pLogEx->Log.RecordType,
            pLogEx->Log.Data.LogType,
            pLogEx->Log.Data.LogAction,
            (int)pLogEx->Log.Data.bDisableResult,
            pLogEx->Log.Data.ProcessId,
            pLogEx->Log.Data.ProcName,
            pLogEx->Log.Data.FileName  );
    
    return 0;
}

int CKextManager::EventNotify_ProcessAccessCheck( PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, czFilePath=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}


//
// PWDisk.kext
//


int CKextManager::PWDisk_EN_Process(PEVT_PARAM pEvtInfo)
{
    if(!pEvtInfo) return 0;
    
    printf( "[DLP][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}


int CKextManager::PWDisk_EN_Log(PEVT_PARAM pEvtInfo)
{
    PLOG_RECORD_PW_EX pLogEx = NULL;
    
    if(!pEvtInfo) return 0;
    
    pLogEx = (PLOG_RECORD_PW_EX)pEvtInfo->pEvtCtx;
    if(!pLogEx) return 0;
    
    printf( "[DLP][%s][%s] p=%d A=%d, R=%d, Main=%d, Sub=%d, Ret=%d, pid=%d, proc=%s path=%s \n",
              __FUNCTION__,
              pLogEx->Log.Data.LogTime,
              pLogEx->PolicyType,
              pLogEx->Log.AccessType,
              pLogEx->Log.RecordType,
              pLogEx->Log.Data.LogType,
              pLogEx->Log.Data.LogAction,
              (int)pLogEx->Log.Data.bDisableResult,
              pLogEx->Log.Data.nPID,
              pLogEx->Log.Data.ProcName,
              pLogEx->Log.Data.FileName  );
    
    return 0;
}


CKextManager::CKextManager() : m_bKctl(true)
{
    memset( &m_DiskSize, 0, sizeof(m_DiskSize) );
}


CKextManager::~CKextManager()
{
    
}

int CKextManager::SelectKextMenu()
{
    int nSelect = 0;
    printf("1. PWDisk.kext        \n");
    printf("2. PISecSmartDrv.kext \n");
    printf("3. ProgramExit. \n");
    
    printf("Select: ");
    scanf( "%d", &nSelect );
    fflush(stdin);
    return nSelect;
}


int CKextManager::SelectDisplayMenu_PWDisk()
{
    int  nSelect = 0;
    
    printf("#############################################\n");
    printf("### PWDisk DisplayMenu \n");
    printf("#############################################\n\n");

    printf("00. clear. \n");
    printf("01. PWDisk_process_load. \n");
    printf("03. PWDisk_process_unload. \n\n");
    
    printf("05. PWDisk_PWSetDiskVolumePath  \n");
    printf("06. PWDisk_PWClrDiskVolumePath  \n");
    printf("07. PWDisk_PWProcessID_Add       \n");
    printf("08. PWDisk_PWProcessID_Remove    \n");
    printf("09. PWDisk_PWProcessID_RemoveAll \n");
    printf("11. PWDisk_PWSetFilter. \n");
    printf("12. PWDisk_PWUnsetFilter. \n\n");
    
    printf("31. ProcessIsQuarantinePath. \n");
    printf("32. ProcessQuarantineFile.    \n");
    printf("33. ProcessQuarantineFile_Release. \n");
    printf("34. ProcessQuarantineFile_Delete. \n\n");
    
    printf("35. PWDisk_GetDiskVolumePath. \n\n");

    printf("41. GetDiskSize. \n");
    printf("42. ConvertDiskSize. \n");
    printf("43. DefaultAllowProcess. \n\n");
    
    printf("99. ProgramExit. \n");
    printf("Select: ");
    scanf( "%d", &nSelect );
    fflush(stdin);
    return nSelect;
}



int CKextManager::SelectDisplayMenu_PISecSmartDrv()
{
    int  nSelect = 0;
    printf("00. Screen Clear                 \n");
    printf("10. GetDrivePolicy               \n");
    printf("20. SetDrivePolicy(Allow)        \n");
    printf("21. SetDrivePolicy(DisableRead)  \n");
    printf("22. SetDrivePolicy(DisableWrite) \n");
    printf("23. SetDrivePolicy(CopyPrevent)  \n\n");
    
    printf("25. SetExceptDrivePolicy   \n\n");
    
    printf("31. SetQuarantinePathExt   \n");
    printf("32. SetQuarantineLimit( true  ) \n");
    printf("33. SetQuarantineLimit( false ) \n\n");
        
    printf("41. SetCtrlMobile( true )  \n");
    printf("42. SetCtrlMobile( false ) \n");
    printf("43. MobileNode_Insert      \n");
    printf("44. MobileNode_RemoveAll   \n\n");

    printf("51. SetPermitProcessName  \n");
    printf("53. SetPermitFolderName   \n");
    printf("55. SetPermitFileExtName  \n");
    printf("57. SetProcessIdUnprotect \n\n");
    
    printf("71. SetPrintPrevent (Off)     \n");
    printf("72. SetPriiagent1@somansa.comntPrevent (On, Allow) \n");
    printf("73. SetPrintPrevent (On, Watermark) \n");
    printf("74. SetPrintPrevent (On, DisableWrite, Watermark)\n");
    printf("75. SetPrintPrevent (On, CopyPrevent, Watermark)\n\n");
    
    printf("99. ProgramExit. \n");
    
    printf("Select: ");
    scanf( "%d", &nSelect );
    fflush(stdin);
    return nSelect;
}


boolean_t
CKextManager::IsRemovable( char* pczDevice )
{
#ifndef LINUX

    kern_return_t nResult = 0;
    io_iterator_t iter;
    io_service_t  service;
    boolean_t     bRemovable = false;

    CFMutableDictionaryRef matchingDict = IOBSDNameMatching(kIOMasterPortDefault, 0, pczDevice );
    if(matchingDict == NULL)
    {
        printf("[DLP] IOBSDNameMatching() returned NULL\n");
        return false;
    }
    
    service = IOServiceGetMatchingService(kIOMasterPortDefault, matchingDict);
    if(IO_OBJECT_NULL == service)
    {
        printf("[DLP] IOServiceGetMatchingService() returned NULL\n");
        return false;
    }
    
    nResult = IORegistryEntryCreateIterator( service, kIOServicePlane, kIORegistryIterateRecursively | kIORegistryIterateParents, &iter );
    if(KERN_SUCCESS != nResult)
    {
        printf("[DLP] IORegistryEntryCreateIterator() returned 0x%08x\n", nResult);
        IOObjectRelease( service );
        return false;
    }
    
    if(IO_OBJECT_NULL == iter)
    {
        printf("[DLP] IORegistryEntryCreateIterator() returned a NULL iterator\n");
        IOObjectRelease( service );
        return false;
    }
    
    bRemovable = false;
    
    do
    {
        if (IOObjectConformsTo(service, kIOMediaClass))
        {
            CFTypeRef RemovableMedia = IORegistryEntryCreateCFProperty( service, CFSTR(kIOMediaRemovableKey), kCFAllocatorDefault, 0 );
            if(RemovableMedia != NULL)
            {
                bRemovable = CFBooleanGetValue( (CFBooleanRef)RemovableMedia );
                CFRelease(RemovableMedia);
            }
            IOObjectRelease(service);
            break;
        }
        IOObjectRelease(service);
        
    } while( (service = IOIteratorNext(iter)) );
    
    IOObjectRelease(iter);
    IOObjectRelease(service);
    return bRemovable;
#else
    return false;
#endif    
}


int CKextManager::KextModule_Startup(LPCALLBACK pCallback, boolean_t bPWDisk )
{
	int nRet = 0;

#ifndef LINUX
	if( NULL == pCallback )
    {
		pCallback = reinterpret_cast<LPCALLBACK>(CKextManager::EventNotify_PostProcess);
	}
    RegisterCallback(pCallback);
    DEBUG_LOG("RegisterCallback() Call. \n", __FUNCTION__ );
    DEBUG_LOG("CommStartControl( bPWDisk=%d ) \n", __FUNCTION__, bPWDisk );
    nRet = CommStartControl( bPWDisk );    
#endif    
    return nRet;
}

int CKextManager::KextModule_Cleanup( boolean_t bPWDisk )
{
    int nRet = 0;

#ifndef LINUX
    DEBUG_LOG("[DLP][%s] CommStopControl( bPWDisk=%d ) \n", __FUNCTION__, bPWDisk );
    nRet = CommStopControl( bPWDisk );

#endif    
    return nRet;
}

void CKextManager::RegisterCallback( LPCALLBACK pCallback )
{
    if(pCallback == NULL)
    {
        return;
    }
    
    if(g_AppCallback != NULL)
    {
        g_AppCallback = NULL;
    }
    
    g_AppCallback = pCallback;

#ifndef LINUX
    DEBUG_LOG("[DLP] Callback Function Reset: %p \n", pCallback );
#endif    
}


int CKextManager::CommStartControl( boolean_t bPWDisk )
{
    int nRet = 0;

#ifndef LINUX
    if(true == bPWDisk)
    {
        nRet = g_PWKctl.KernelControl_Init();        
    }
    else
    {
        if(m_bKctl == true)
        {
            nRet = g_AppKctl.KernelControl_Init();
        }
        else
        {
            nRet = g_AppSctl.SystemControl_Init();
        }
    }
#endif    
    return nRet;
}


int CKextManager::CommStopControl( boolean_t bPWDisk )
{
    int nRet = 0;

#ifndef LINUX
    
    if(true == bPWDisk)
    {
        nRet = g_PWKctl.KernelControl_Uninit();
    }
    else
    {
        if(m_bKctl == true)
        {
            nRet = g_AppKctl.KernelControl_Uninit();
        }
        else
        {
            nRet = g_AppSctl.SystemControl_Uninit();
        }
    }

#endif

    return nRet;
}


/****************************************************************************************/
// Policy
/****************************************************************************************/

#ifndef LINUX

int CKextManager::Test_SetExceptUsbPolicy(int nCmd, void* pBuf,  const char* pczVolumePath )
{
    PATTACHED_DEVICE_EX pDrv = NULL;
    
    pDrv = (PATTACHED_DEVICE_EX)pBuf;
    if(!pDrv) return 0;
    
    memset( pDrv, 0, sizeof(ATTACHED_DEVICE_EX) );
    pDrv->ulStorDevType = MEDIA_COPYPREVENT;
    pDrv->cDevice.bLoggingOn    = TRUE;
    pDrv->cDevice.bDisableAll   = FALSE;
    pDrv->cDevice.bDisableRead  = FALSE;
    pDrv->cDevice.bDisableWrite = FALSE;
    pDrv->cDevice.nReserved1    = -1;
    if(pczVolumePath)
    {
        strcpy( pDrv->cDevice.DeviceNames, pczVolumePath );
    }
    
    if(1 == nCmd)
    {  // ReadDeny
        pDrv->ulStorDevType = MEDIA_USB;
        pDrv->cDevice.bLoggingOn   = TRUE;
        pDrv->cDevice.bDisableRead = FALSE;
        pDrv->cDevice.nReserved1   = -1;
    }
    else if(2 == nCmd)
    { // Write - Deny
        pDrv->ulStorDevType = MEDIA_USB;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(3 == nCmd)
    { // Copy Prevent
        pDrv->ulStorDevType = MEDIA_COPYPREVENT;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    return 0;
}



int CKextManager::Test_SetExceptCDPolicy(int nCmd, void* pBuf, const char* pczVolumePath )
{
    PATTACHED_DEVICE_EX pDrv = NULL;
    
    pDrv = (PATTACHED_DEVICE_EX)pBuf;
    if(!pDrv) return 0;
    
    memset( pDrv, 0, sizeof(ATTACHED_DEVICE_EX) );
    pDrv->ulStorDevType = MEDIA_CD_DVD;
    pDrv->cDevice.bLoggingOn    = TRUE;
    pDrv->cDevice.bDisableAll   = FALSE;
    pDrv->cDevice.bDisableRead  = FALSE;
    pDrv->cDevice.bDisableWrite = FALSE;
    pDrv->cDevice.nReserved1    = -1;
    if(pczVolumePath)
    {
        strcpy( pDrv->cDevice.DeviceNames, pczVolumePath );
    }
    
    if(1 == nCmd)
    {  // ReadDeny
        pDrv->ulStorDevType = MEDIA_CD_DVD;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableRead  = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(2 == nCmd)
    { // Write - Deny
        pDrv->ulStorDevType = MEDIA_CD_DVD;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(3 == nCmd)
    { // Copy Prevent
        pDrv->ulStorDevType = MEDIA_COPYPREVENT;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    return 0;
}


int CKextManager::Test_SetExceptSFolderPolicy( int nCmd, void* pBuf, const char* pczVolumePath )
{
    PATTACHED_DEVICE_EX pDrv = NULL;
    
    pDrv = (PATTACHED_DEVICE_EX)pBuf;
    if(!pDrv) return 0;
    
    memset( pDrv, 0, sizeof(ATTACHED_DEVICE_EX) );
    pDrv->ulStorDevType = MEDIA_NET_DRIVE;
    pDrv->cDevice.bLoggingOn    = TRUE;
    pDrv->cDevice.bDisableAll   = FALSE;
    pDrv->cDevice.bDisableRead  = FALSE;
    pDrv->cDevice.bDisableWrite = FALSE;
    pDrv->cDevice.nReserved1    = -1;
    if(pczVolumePath)
    {
        strcpy( pDrv->cDevice.DeviceNames, pczVolumePath );
    }
    
    if(1 == nCmd)
    {  // ReadDeny
        pDrv->ulStorDevType = MEDIA_NET_DRIVE;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableRead  = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(2 == nCmd)
    { // Write - Deny
        pDrv->ulStorDevType = MEDIA_NET_DRIVE;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(3 == nCmd)
    { // Copy Prevent
        pDrv->ulStorDevType = MEDIA_NET_DRIVE;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = FALSE;
        pDrv->cDevice.nReserved1    = -1;
    }
    return 0;
}



int CKextManager::Test_SetUsbPolicy(int nCmd, void* pBuf )
{
    PATTACHED_DEVICE_EX pDrv = NULL;
    
    pDrv = (PATTACHED_DEVICE_EX)pBuf;
    if(!pDrv) return 0;
    
    memset( pDrv, 0, sizeof(ATTACHED_DEVICE_EX) );
    pDrv->ulStorDevType = MEDIA_COPYPREVENT;
    pDrv->cDevice.bLoggingOn    = TRUE;
    pDrv->cDevice.bDisableAll   = FALSE;
    pDrv->cDevice.bDisableRead  = FALSE;
    pDrv->cDevice.bDisableWrite = FALSE;
    pDrv->cDevice.nReserved1    = -1;

    if(1 == nCmd)
    {  // ReadDeny
        pDrv->ulStorDevType = MEDIA_USB;
        pDrv->cDevice.bLoggingOn   = TRUE;
        pDrv->cDevice.bDisableRead = TRUE;
        pDrv->cDevice.nReserved1   = -1;
    }
    else if(2 == nCmd)
    { // Write - Deny
        pDrv->ulStorDevType = MEDIA_USB;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = TRUE;
        pDrv->cDevice.nReserved1    = 1;
    }
    else if(3 == nCmd)
    { // Copy Prevent
        pDrv->ulStorDevType = MEDIA_COPYPREVENT;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = TRUE;
        pDrv->cDevice.nReserved1    = -1;
    }
    return 0;
}



int CKextManager::Test_SetCDPolicy(int nCmd, void* pBuf )
{
    PATTACHED_DEVICE_EX pDrv = NULL;
    
    pDrv = (PATTACHED_DEVICE_EX)pBuf;
    if(!pDrv) return 0;
    
    memset( pDrv, 0, sizeof(ATTACHED_DEVICE_EX) );
    pDrv->ulStorDevType = MEDIA_CD_DVD;
    pDrv->cDevice.bLoggingOn    = TRUE;
    pDrv->cDevice.bDisableAll   = FALSE;
    pDrv->cDevice.bDisableRead  = FALSE;
    pDrv->cDevice.bDisableWrite = FALSE;
    pDrv->cDevice.nReserved1    = -1;
    
    if(1 == nCmd)
    {  // ReadDeny
        pDrv->ulStorDevType = MEDIA_CD_DVD;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableRead  = TRUE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(2 == nCmd)
    { // Write - Deny
        pDrv->ulStorDevType = MEDIA_CD_DVD;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = TRUE;
        pDrv->cDevice.nReserved1    = 1;
    }
    else if(3 == nCmd)
    { // Copy Prevent
        pDrv->ulStorDevType = MEDIA_CD_DVD;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = TRUE;
        pDrv->cDevice.nReserved1    = -1;
    }
    return 0;
}

int CKextManager::Test_SetSFolderPolicy( int nCmd, void* pBuf )
{
    
    PATTACHED_DEVICE_EX pDrv = NULL;
    
    pDrv = (PATTACHED_DEVICE_EX)pBuf;
    if(!pDrv) return 0;
    
    memset( pDrv, 0, sizeof(ATTACHED_DEVICE_EX) );
    pDrv->ulStorDevType = MEDIA_NET_DRIVE;
    pDrv->cDevice.bLoggingOn    = TRUE;
    pDrv->cDevice.bDisableAll   = FALSE;
    pDrv->cDevice.bDisableRead  = FALSE;
    pDrv->cDevice.bDisableWrite = FALSE;
    pDrv->cDevice.nReserved1    = -1;
    
    if(1 == nCmd)
    {  // ReadDeny
        pDrv->ulStorDevType = MEDIA_NET_DRIVE;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableRead  = TRUE;
        pDrv->cDevice.nReserved1    = -1;
    }
    else if(2 == nCmd)
    { // Write - Deny
        pDrv->ulStorDevType = MEDIA_NET_DRIVE;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = TRUE;
        pDrv->cDevice.nReserved1    = 1;
    }
    else if(3 == nCmd)
    { // Copy Prevent
        pDrv->ulStorDevType = MEDIA_NET_DRIVE;
        pDrv->cDevice.bLoggingOn    = TRUE;
        pDrv->cDevice.bDisableWrite = TRUE;
        pDrv->cDevice.nReserved1    = -1;
    }
    return 0;
}

int CKextManager::fnGetDrivePolicy(void* pBuf, int nLength )
{
    int  nRet=0;
    int  nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    
    if(!pBuf || nLength <= 0)
    {
        printf("[DLP] GetDrivePolicy Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nLength;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = GetDrivePolicy;
    memcpy( &pCmdNew->Data, pBuf, nLength );
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}
#endif

int CKextManager::fnSetDrivePolicy( void* pBuf, int nLength )
{
    int  nRet=0;

#ifndef LINUX

    int  nTotalSize =0;
    PCOMMAND_MESSAGE    pCmdNew   = NULL;
    
    if(!pBuf || nLength <= 0)
    {
        printf("[DLP] SetDrivePolicy Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nLength;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetDrivePolicy;
    memcpy( &pCmdNew->Data, pBuf, nLength );
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    free( pCmdNew );
#endif    
    return nRet;
}

int CKextManager::fnSetExceptDrivePolicy( void* pBuf, int nLength )
{
    int  nRet=0;

#ifndef LINUX

    int  nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    
    if(!pBuf || nLength <= 0)
    {
        printf("[DLP] SetExceptDrivePolicy Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nLength;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetExceptDrivePolicy;
    memcpy( &pCmdNew->Data, pBuf, nLength );
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


int CKextManager::fnSetQuarantinePathExt(PQT_CTX pQtCtx)
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_QT_INFO    pCommQtInfo = NULL;
    
    if(!pQtCtx)
    {
        printf("[DLP] SetQuarantinePathExt Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(QT_CTX);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command = (ULONG)SetQuarantinePathExt;
    pCommQtInfo = (PCOMM_QT_INFO)pCmdNew->Data;
    if(pCommQtInfo)
    {
        memcpy( &pCommQtInfo->QtCtx, pQtCtx, sizeof(QT_CTX) );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


int CKextManager::fnSetQuarantineLimit(boolean_t bQtLimit)
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_QT_INFO    pCommQtInfo = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(QT_CTX);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command = (ULONG)SetQuarantineLimit;
    pCommQtInfo = (PCOMM_QT_INFO)pCmdNew->Data;
    if(pCommQtInfo)
    {
        pCommQtInfo->QtCtx.bQtLimit = bQtLimit;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


int CKextManager::fnSetUSBMobilePermit( MB_PERMIT& PermitInfo )
{
    int nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_PERMIT  pCommPermit = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_PERMIT);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetMobilePermitList;
    pCommPermit = (PCOMM_MB_PERMIT)pCmdNew->Data;
    if(pCommPermit)
    {
        memcpy( &pCommPermit->Permit, &PermitInfo, sizeof(MB_PERMIT) );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}



int CKextManager::fnSetUSBMobilePermitList(std::vector<MB_PERMIT>* pPermitList)
{
    int nRet=0, nPos=0, nCount=0;

#ifndef LINUX

    std::vector<MB_PERMIT>::iterator Iter;
    MB_PERMIT PermitInfo;
    
    if(!pPermitList) return EINVAL;
    nCount = (int)pPermitList->size();
    if(nCount <= 0) return EINVAL;
    
    for(Iter=pPermitList->begin(); nPos<nCount; nPos++, Iter++)
    {
        PermitInfo = (*Iter);
        fnSetUSBMobilePermit( PermitInfo );
    }

#endif

    return nRet;
}


int CKextManager::fnClrUSBMobilePermitList()
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_PERMIT pCommPermit = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_PERMIT);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = ClrMobilePermitList;
    pCommPermit = (PCOMM_MB_PERMIT)pCmdNew->Data;
    if(pCommPermit)
    {
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}



int CKextManager::fnSetCtrlMobile(PMB_POLICY pPolicy)
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_POLICY  pMbPolicy = NULL;
    
    if(!pPolicy) return EINVAL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetCtrlMobile;
    pMbPolicy = (PCOMM_MB_POLICY)pCmdNew->Data;
    if(pMbPolicy)
    {
        pMbPolicy->Policy = *pPolicy;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


int CKextManager::fnMobileNodeInsert_SetMtp(const char* pczBasePath, const char* pczKeyword )
{
    int      nReturn = 0;

#ifndef LINUX

    size_t   nLength = 0;
    MB_NODE  Node;
    memset( &Node, 0, sizeof(Node) );
    
    Node.MbType = MB_TYPE_MTP;
    if(pczBasePath)
    {
        nLength = 0;
        nLength = strlen(pczBasePath);
        if(nLength > 0)
        {
            if(nLength > MAX_BASEPATH-1)
            {
                nLength = MAX_BASEPATH-1;
            }
            strncpy( Node.czBasePath, pczBasePath, nLength );
        }
    }
    
    if(pczKeyword)
    {
        nLength = 0;
        nLength = strlen(pczKeyword);
        if(nLength > 0)
        {
            if(nLength > MAX_KEYWORD-1)
            {
                nLength = MAX_KEYWORD-1;
            }
            strncpy( Node.czKeyword, pczKeyword, nLength );
        }
    }
    
    nReturn = fnMobileNodeInsert( &Node );


#endif

    return nReturn;
}


int CKextManager::fnMobileNodeInsert_SetPtp(const char* pczBasePath, const char* pczKeyword )
{
    int      nReturn = 0;

#ifndef LINUX

    size_t   nLength = 0;
    MB_NODE  Node;
    memset( &Node, 0, sizeof(Node) );
    
    Node.MbType = MB_TYPE_PTP;
    if(pczBasePath)
    {
        nLength = 0;
        nLength = strlen(pczBasePath);
        if(nLength > 0)
        {
            if(nLength > MAX_BASEPATH-1)
            {
                nLength = MAX_BASEPATH-1;
            }
            strncpy( Node.czBasePath, pczBasePath, nLength );
        }
    }
    
    if(pczKeyword)
    {
        nLength = 0;
        nLength = strlen(pczKeyword);
        if(nLength > 0)
        {
            if(nLength > MAX_KEYWORD-1)
            {
                nLength = MAX_KEYWORD-1;
            }
            strncpy( Node.czKeyword, pczKeyword, nLength );
        }
    }
    
    nReturn = fnMobileNodeInsert( &Node );

#endif

    return nReturn;
}

#ifndef LINUX

int CKextManager::fnMobileNodeInsert(PMB_NODE pNode)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_NODE    pMbNode = NULL;
    
    if(!pNode)
    {
        printf("[DLP] MobileNodeInsert Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_NODE);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = (ULONG)MobileNodeInsert;
    pMbNode = (PCOMM_MB_NODE)pCmdNew->Data;
    if(pMbNode)
    {
        pMbNode->Node = *pNode;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}

#endif


int CKextManager::fnMobileNodeRemoveAll()
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
   
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_NODE);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = MobileNodeRemoveAll;
    
    printf( "[DLP][%s] SendCommand_KernCtl \n", __FUNCTION__ );
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}

#ifndef LINUX

int CKextManager::fnSetProcessIdUnprotect(ULONG ProcessId)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PALLOWPROCESSID  pAllowProcessID = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(ALLOWPROCESSID);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = (ULONG)SetProcessIdUnprotect;
    pAllowProcessID = (PALLOWPROCESSID)pCmdNew->Data;
    if(pAllowProcessID)
    {
        pAllowProcessID->nAllowedProcID = ProcessId;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

    return nRet;
}
#endif


int CKextManager::fnSetPermitProcessName(void* pBuf, int nBufSize)
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE  pCmdNew = NULL;
    PALLOWPROCESSNAME pAllowProcName = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[DLP] RemoveAndroidDeviceNode Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = (ULONG)SetPermitProcessName;
    pAllowProcName = (PALLOWPROCESSNAME)pCmdNew->Data;
    if(pAllowProcName)
    {
        memcpy( pAllowProcName, pBuf, nBufSize );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    
#endif

    return nRet;
}


int CKextManager::fnSetPermitFolderName(void* pBuf, int nBufSize)
{
    int  nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PALLOWFOLDERNAME pAllowFolderName = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[DLP] RemoveAndroidDeviceNode Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command = (ULONG)SetPermitFolderName;
    pAllowFolderName = (PALLOWFOLDERNAME)pCmdNew->Data;
    if(pAllowFolderName)
    {
        memcpy( pAllowFolderName, pBuf, nBufSize );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;

}


int CKextManager::fnSetPermitFileExtName(void* pBuf, int nBufSize)
{
    int   nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE  pCmdNew = NULL;
    PALLOWFILEEXTNAME pAllowFileExt = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[DLP] RemoveAndroidDeviceNode Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetPermitFileExtName;
    pAllowFileExt = (PALLOWFILEEXTNAME)pCmdNew->Data;
    if(pAllowFileExt)
    {
        memcpy( pAllowFileExt, pBuf, nBufSize );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;

}



int CKextManager::fnGetSmartLogEx()
{
    return 0;
}


int CKextManager::fnSetPrintPrevent_Off()
{
    int nRet = 0;

#ifndef LINUX

    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );
    
    policy.bPolicy = false;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );

#endif

    return nRet;
}

#ifndef LINUX

int CKextManager::fnSetPrintPrevent_Allow()
{
    int nRet = 0;
    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );
    
    policy.bPolicy = true;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );
    return nRet;
}

int CKextManager::fnSetPrintPrevent_Watermark()
{
    int nRet = 0;
    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );

    policy.bPolicy    = true;
    policy.bWatermark = true;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );
    return nRet;
}

int CKextManager::fnSetPrintPrevent_DisableWrite()
{
    int nRet = 0;
    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );
    
    policy.bPolicy       = true;
    policy.bWatermark    = true;
    policy.bDisableWrite = true;
    policy.nReserved1    = 1;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );
    return nRet;
    
}


int CKextManager::fnSetPrintPrevent_CopyPrevent()
{
    int nRet = 0;


    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );
    
    policy.bPolicy       = true;
    policy.bWatermark    = true;
    policy.bDisableWrite = true;
    policy.nReserved1    = -1;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );
    return nRet;
    
}
#endif


int CKextManager::fnSetPrintPrevent(void* pBuf, int nBufSize )
{
    int nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PPRINT_POLICY    pPolicy = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[DLP] SetPrintPrevent Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetPrintPrevent;    
    pPolicy = (PPRINT_POLICY)pCmdNew->Data;
    if(pPolicy)
    {
        memcpy( pPolicy, pBuf, nBufSize );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


int CKextManager::fnSetUploadPrevent(void* pBuf, int nBufSize )
{
    int nRet=0, nTotalSize =0;

#ifndef LINUX

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PUPLOAD_POLICY    pPolicy = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[DLP] SetUploadPrevent Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetUploadPrevent;
    pPolicy = (PUPLOAD_POLICY)pCmdNew->Data;
    if(pPolicy)
    {
        memcpy( pPolicy, pBuf, nBufSize );
        
        //printf("applyPolicyUploadPrevent pDLPPolicyUpload->m_sControlList.size == %lu, [%s]",
        //          pPolicy->lControlListSize, &(pPolicy->szControlList));
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}

int CKextManager::fnSetProcessAccessCheck( BOOLEAN bProcAC, BOOLEAN bLog )
{
    int nRet=0, nTotalSize =0;

#ifdef _FIXME_

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PROC_AC_POLICY*  pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(PROC_AC_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        DEBUG_LOG( "[DLP] malloc failed(%d) \n", errno  );
        return -1;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetProcessAccessCheck;
    pPolicy = (PROC_AC_POLICY*)pCmdNew->Data;
    if(pPolicy)
    {
        pPolicy->bLog = bLog;
        pPolicy->bProcAC = bProcAC;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


// SetControlCamera
int CKextManager::fnSetControlCamera( bool bControl, bool bLog  )
{
    int nRet=0, nTotalSize =0;

#ifdef _FIXME_

    PCOMMAND_MESSAGE pCmdNew = NULL;
    CAMERA_POLICY*  pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(CAMERA_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetControlCamera;
    pPolicy = (CAMERA_POLICY*)pCmdNew->Data;
    if(pPolicy)
    {
        pPolicy->bCameraLog = bLog;
        pPolicy->bCameraBlock = bControl;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}


// SetControlRNDIS
int CKextManager::fnSetControlRndis( bool bRNdisCtrl, bool bRNdisLog )
{
    int nRet=0, nTotalSize =0;

#ifdef _FIXME_

    PCOMMAND_MESSAGE pCmdNew = NULL;
    RNDIS_POLICY* pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(RNDIS_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetControlRNDIS;
    pPolicy = (RNDIS_POLICY *)pCmdNew->Data;
    if(pPolicy)
    {
        pPolicy->bRNdisLog = bRNdisLog;
        pPolicy->bRNdisBlock = bRNdisCtrl;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif
        
    return nRet;
}


// SetControlAirDrop
int CKextManager::fnSetControlAirDrop( bool bAirDropBlock, bool bAirDropLog )
{
    int nRet=0, nTotalSize =0;

#ifdef _FIXME_

    PCOMMAND_MESSAGE pCmdNew = NULL;
    AIRDROP_POLICY* pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(AIRDROP_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetControlAirDrop;
    pPolicy = (AIRDROP_POLICY *)pCmdNew->Data;
    if(pPolicy)
    {
        pPolicy->bAirDropLog = bAirDropLog;
        pPolicy->bAirDropBlock = bAirDropBlock;
        pPolicy->nReserved0 = 0;
        pPolicy->nReserved1 = 0;
        pPolicy->nReserved2 = 0;
        pPolicy->nReserved3 = 0;
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    else
    {
        nRet = g_AppSctl.SendCommand_SysCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );

#endif

    return nRet;
}

#ifdef _NOTUSED_

//
// PWDisk.kext
//
int CKextManager::PWDisk_PWSetFilter()
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_FILTER   UKFilter;
    
    nLength = sizeof(UKP_FILTER);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    UKFilter.bFilter = TRUE;
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWSetFilter;
    memcpy( &pCmdNew->Data, &UKFilter, nLength );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
        nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}


int CKextManager::PWDisk_PWUnsetFilter()
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_FILTER   UKFilter;
    
    nLength = sizeof(UKP_FILTER);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    UKFilter.bFilter = FALSE;
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWUnsetFilter;
    memcpy( &pCmdNew->Data, &UKFilter, nLength );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
        nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}


// SetDiskPath
int CKextManager::PWDisk_PWSetDiskVolumePath( BOOLEAN bDiskDeny, BOOLEAN bDiskLog, char* pczVolumePath )
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_DISK  UKDisk;
    
    if(!pczVolumePath)
    {
        printf("[DLP][%s] Invalid Parameter. \n", __FUNCTION__ );
        return -1;
    }
    
    memset( &UKDisk, 0, sizeof(UKP_DISK) );
    UKDisk.bDiskDeny = bDiskDeny;
    UKDisk.bDiskLog  = bDiskLog;
    strncpy( UKDisk.czVolumePath, pczVolumePath, strlen(pczVolumePath) );
    
    nLength = sizeof(UKP_DISK);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWSetDiskVolumePath;
    memcpy( &pCmdNew->Data, &UKDisk, sizeof(UKP_DISK) );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
        nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}



int CKextManager::PWDisk_PWClrDiskVolumePath()
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_DISK  UKDisk;
    
    nLength = sizeof(UKP_DISK);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( &UKDisk, 0, sizeof(UKDisk) );
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWClrDiskVolumePath;
    memcpy( &pCmdNew->Data, &UKDisk, nLength );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
        nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
    
}



// ProcessAdd
int CKextManager::PWDisk_PWProcessID_Add( ULONG nPID, char* pczDocPath )
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_PROC UKProc;
    
    memset( &UKProc, 0, sizeof(UKProc) );
    UKProc.nPID = nPID;
    if(pczDocPath)
    {
        strncpy( UKProc.czDocPath, pczDocPath, strlen(pczDocPath) );
    }
    
    nLength = sizeof(UKP_PROC);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWProcessID_Add;
    memcpy( &pCmdNew->Data, &UKProc, nLength );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
        nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}


// PWProcess_Remove
int CKextManager::PWDisk_PWProcessID_Remove( ULONG nPID, char* pczDocPath )
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_PROC UKProc;
    
    memset( &UKProc, 0, sizeof(UKProc) );
    UKProc.nPID = nPID;
    if(pczDocPath)
    {
        strncpy( UKProc.czDocPath, pczDocPath, strlen(pczDocPath) );
    }
    
    nLength = sizeof(UKP_PROC);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWProcessID_Remove;
    memcpy( &pCmdNew->Data, &UKProc, nLength );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
         nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
    
}



// PWProcess_RemoveAll
int CKextManager::PWDisk_PWProcessID_RemoveAll()
{
    int  nRet=0;
    int  nLength=0, nTotalSize =0;
    PMSG_COMMAND pCmdNew = NULL;
    UKP_PROC UKProc;
    
    memset( &UKProc, 0, sizeof(UKProc) );
    
    nLength = sizeof(UKP_PROC);
    nTotalSize = sizeof(MSG_COMMAND) + nLength;
    pCmdNew = (PMSG_COMMAND)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = PWProcessID_RemoveAll;
    memcpy( &pCmdNew->Data, &UKProc, nLength );
    
    if(g_PWKctl.m_nKernCtlId > 0)
    {
        nRet = g_PWKctl.SendCommand_PWKctl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}




boolean_t
CKextManager::PWDisk_DefaultCreateDir()
{
    boolean_t bSuc = false;
    char czDirPath[MAX_PATH];
    
    if(false == m_DiskSize.bMount)
    {
        return false;
    }

    
    memset( czDirPath, 0, sizeof(czDirPath) );
    snprintf( czDirPath, sizeof(czDirPath), "%s%s", m_DiskSize.czVolumePath, PATH_SMSDISK_DATA );
    bSuc = PWDisk_SyCall_CreateDir( czDirPath );
    if(bSuc == false)
    {
        printf( "[DLP][%s] Fail. DirPath=%s \n", __FUNCTION__, czDirPath );
        return false;
    }
    
    memset( czDirPath, 0, sizeof(czDirPath) );
    snprintf( czDirPath, sizeof(czDirPath), "%s%s", m_DiskSize.czVolumePath, PATH_SMSDISK_DRIVE );
    bSuc = PWDisk_SyCall_CreateDir( czDirPath );
    if(bSuc == false)
    {
        printf( "[DLP][%s] Fail. DirPath=%s \n", __FUNCTION__, czDirPath );
        return false;
    }
    
    printf( "[DLP][%s] Success. DirPath=%s \n", __FUNCTION__, czDirPath );
    return true;
}


//
// bSuc = PWDisk_GetFreeDiskPath( &Disk );
// if(bSuc == false)
//

// Plain Disk
int CKextManager::PWDisk_PWProcess_load()
{
    int  nRet = 0;
    boolean_t  bSuc = false;
    PWDISK_SIZE Disk;
    
    memset( &Disk, 0, sizeof(Disk) );
    strncpy( Disk.czDirPath, PWDISK_DIRPATH, strlen(PWDISK_DIRPATH) );
    strncpy( Disk.czImagePath, PWDISK_IMAGEPATH, strlen(PWDISK_IMAGEPATH) );
    strncpy( Disk.czVolumeName, PWDISK_VOLUMENAME, strlen(PWDISK_VOLUMENAME) );
    strncpy( Disk.czVolumePath, PWDISK_VOLUMEPATH, strlen(PWDISK_VOLUMEPATH) );
    printf( "[DLP][%s] Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, Disk.czVolumePath );
    
    nRet = access( Disk.czDirPath, 0 );
    if(nRet != 0)
    {
        bSuc = PWDisk_SyCall_CreateDir( Disk.czDirPath );
    }
    
    
    PWDisk_PWProcess_unload();
    
    
    nRet = access( Disk.czVolumePath, 0 );
    if(nRet == 0)
    {
        bSuc = PWDisk_SyCall_DeleteFile( Disk.czVolumePath );
        printf( "[DLP][%s] Ret=%d, VolumePath=%s Delete. \n", __FUNCTION__, bSuc, Disk.czVolumePath  );
    }
    
    bSuc = PWDisk_QuarantineDisk_mount( &Disk );
    if(bSuc == false)
    {
        printf( "[DLP][%s] Failed. Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, Disk.czVolumePath  );
        return EPERM;
    }
   
    memset( &m_DiskSize, 0, sizeof(m_DiskSize) );
    m_DiskSize = Disk;
    m_DiskSize.bMount = true;
    PWDisk_GetDiskSize( &m_DiskSize );
    PWDisk_DefaultCreateDir();
    
    printf( "[DLP][%s] Mount=%d      \n", __FUNCTION__, m_DiskSize.bMount         );
    printf( "[DLP][%s] VolumeName=%s \n", __FUNCTION__, m_DiskSize.czVolumeName );
    printf( "[DLP][%s] VolumePath=%s \n", __FUNCTION__, m_DiskSize.czVolumePath );
    printf( "[DLP][%s] DirPath=%s    \n", __FUNCTION__, m_DiskSize.czDirPath     );
    printf( "[DLP][%s] ImagePath=%s  \n", __FUNCTION__, m_DiskSize.czVolumePath );
    printf( "[DLP][%s] TotalSize=%.04lf \n", __FUNCTION__, m_DiskSize.nTotalSize/SIZE_GB );
    printf( "[DLP][%s] FreeSize=%.04lf  \n", __FUNCTION__, m_DiskSize.nFreeSize/SIZE_GB );

    
    // 1. ProcessID_Add
    PWDisk_PWProcessID_Add( getpid() );
    // 2. Privay-i Default PermitProcess
    PWDisk_PWDefaultAllowProcess();
    // 3. SetDiskPath
    PWDisk_PWSetDiskVolumePath( TRUE, TRUE, m_DiskSize.czVolumePath );
    // 4. PWSetFilter
    PWDisk_PWSetFilter();
    
    return 0;
}


int CKextManager::PWDisk_PWProcess_unload()
{
    int nReturn = 0;
    boolean_t bSuc = false;
    PWDISK_SIZE Disk;
    
    // 1. PWUnsetFilter
    PWDisk_PWUnsetFilter();
    // 2. ClrDiskPath
    PWDisk_PWClrDiskVolumePath();
    // 3. Clear PermitProcessList
    PWDisk_PWProcessID_RemoveAll();
    
    
    // Default Disk Unmount
    memset( &Disk, 0, sizeof(Disk) );
    strncpy( Disk.czVolumeName, PWDISK_VOLUMENAME, strlen(PWDISK_VOLUMENAME) );
    strncpy( Disk.czVolumePath, PWDISK_VOLUMEPATH, strlen(PWDISK_VOLUMEPATH) );
    printf( "[DLP][%s] Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, Disk.czVolumePath );

    // mount
    nReturn = access( Disk.czVolumePath, 0 );
    if(nReturn == 0)
    {
        bSuc = PWDisk_QuarantineDisk_unmount( &Disk );
        if(bSuc == false)
        {
            printf( "[DLP][%s] Fail. Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, Disk.czVolumePath  );
        }
        else
        {
            printf( "[DLP][%s] Success. Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, Disk.czVolumePath  );
        }
    }

    // mount
    nReturn = access( m_DiskSize.czVolumePath, 0 );
    if(nReturn == 0)
    {
        bSuc = PWDisk_QuarantineDisk_unmount( &m_DiskSize );
        if(bSuc == false)
        {
            printf( "[DLP][%s] Fail. Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, m_DiskSize.czVolumePath  );
        }
        else
        {
            printf( "[DLP][%s] Success. Ret=%d, VolumePath=%s \n", __FUNCTION__, bSuc, m_DiskSize.czVolumePath  );
        }
    }
    m_DiskSize.bMount = false;
    return 0;
}



int CKextManager::PWDisk_mkdirs( const char* pczDirPath, mode_t nMode )
{
    char* pczPos = NULL;
    char  czTempPath[MAX_PATH];
    int   nLength=0, nRet=0;
    
    pczPos = (char*)pczDirPath;
    if(!pczDirPath || strlen(pczDirPath) >= MAX_PATH)
    {
        return -1;
    }
    
    while((pczPos = strchr(pczPos, '/')) != NULL)
    {
        nLength = (int)(pczPos - pczDirPath);
        pczPos++;
        if(nLength <= 0)
        {
            continue;
        }
        
        memset( czTempPath, 0, sizeof(czTempPath) );
        strncpy( czTempPath, pczDirPath, nLength );
        czTempPath[ nLength ] = '\0';
        
        nRet = mkdir( czTempPath, nMode);
        if(nRet == -1)
        {
            if(errno != EEXIST)
            {
                return -1;
            }
        }
    }
    return mkdir( pczDirPath, nMode );
}


boolean_t
CKextManager::PWDisk_SyCall_CreateDir( char* pczDirPath )
{
    int nRet = 0;
    std::string strResult  = "";
    std::string strCommand = "";
    
    if(!pczDirPath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nRet = PWDisk_mkdirs( pczDirPath, 0766 );
    if(nRet != 0)
    {
        if(errno != EEXIST)
        {
            printf( "[DLP][%s] Failed. Ret=%d, error=%d, Path=%s \n", __FUNCTION__, nRet, errno, pczDirPath );
            return false;
        }
    }
    
    printf( "[DLP][%s] Success. Ret=%d, error=%d, Path=%s \n", __FUNCTION__, nRet, errno, pczDirPath );
    return true;
}


boolean_t
CKextManager::PWdisk_GetDiskVolumePath( char* pczOutVolumePath, int nMaxOutVolumePath )
{
    if(!pczOutVolumePath || nMaxOutVolumePath <= 0)
    {
        printf( "[DLP][%s] einval \n", __FUNCTION__ );
        return false;
    }
    
    if(true == m_DiskSize.bMount)
    {
        strncpy( pczOutVolumePath, m_DiskSize.czVolumePath, strlen(m_DiskSize.czVolumePath) );
        
        printf( "[DLP][%s] Success. TotalSize=%dGB \n", __FUNCTION__, (int)round(m_DiskSize.nTotalSize/SIZE_GB) );
        printf( "[DLP][%s] Success. VolumePath=%s \n", __FUNCTION__, m_DiskSize.czVolumePath );
        return true;
    }
    
    printf( "[DLP][%s] Failed. DiskVolume Not Mounted. \n", __FUNCTION__ );
    return false;
}


boolean_t
CKextManager::PWDisk_GetDiskVolumeInfo( PWDISK_SIZE* pDiskInfo )
{
    if(!pDiskInfo)
    {
        printf( "[DLP][%s] einval \n", __FUNCTION__ );
        return false;
    }
    
    if(true == m_DiskSize.bMount)
    {
        memcpy( pDiskInfo, &m_DiskSize, sizeof(m_DiskSize) );
        
        printf( "[DLP][%s] Success. TotalSize=%dGB \n", __FUNCTION__, (int)round(m_DiskSize.nTotalSize/SIZE_GB) );
        printf( "[DLP][%s] Success. VolumePath=%s \n", __FUNCTION__, m_DiskSize.czVolumePath );
        return true;
    }
    
    printf( "[DLP][%s] Failed. DiskVolume Not Mounted. \n", __FUNCTION__ );
    return false;
}


boolean_t
CKextManager::PWDisk_SyCall_CreateFile( const char* pczNewFilePath )
{
    int      nNewFd=0;
    ssize_t nToWrite=0, nWritten=0;
    char     czBuffer[MAX_BUFFER];
    
    if(!pczNewFilePath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nNewFd = open( pczNewFilePath, O_CREAT | O_WRONLY, 0644 );
    if(nNewFd == -1)
    {
        printf( "[DLP][%s] open error=%d NewFilePath=%s \n", __FUNCTION__, errno, pczNewFilePath );
        return false;
    }
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    strcpy( czBuffer, "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ \n" );
    nToWrite = strlen(czBuffer);
    nWritten = write( nNewFd, czBuffer, nToWrite );
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    strcpy( czBuffer, "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ \n" );
    nToWrite = strlen(czBuffer);
    nWritten = write( nNewFd, czBuffer, nToWrite );
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    strcpy( czBuffer, "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ \n" );
    nToWrite = strlen(czBuffer);
    nWritten = write( nNewFd, czBuffer, nToWrite );
    
    close( nNewFd );
    
    printf( "[DLP][%s] Success. NetPath=%s \n", __FUNCTION__, pczNewFilePath );
    return true;
}


boolean_t
CKextManager::PWDisk_SyCall_CopyFile( const char* pczSrcFilePath, const char* pczDstFilePath )
{
    int    nRet=0, nSrcFd=0, nDstFd=0;
    ssize_t nRead=0, nToRead=0, nWritten=0;
    char   czBuffer[MAX_BUFFER];
    
    if(!pczSrcFilePath || !pczDstFilePath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nRet = access( pczSrcFilePath, 0 );
    if(nRet != 0)
    {
        printf( "[DLP][%s] File-NotExist. error=%d, SrcFilePath=%s \n", __FUNCTION__, errno, pczSrcFilePath );
        return false;
    }
    
    nSrcFd = open( pczSrcFilePath, O_RDONLY, S_IRUSR );
    if(nSrcFd == -1)
    {
        printf( "[DLP][%s] open error=%d SrcPath=%s \n", __FUNCTION__, errno, pczSrcFilePath );
        return false;
    }
    
    nDstFd = open( pczDstFilePath, O_CREAT | O_WRONLY, 0644 );
    if(nDstFd == -1)
    {
        printf( "[DLP][%s] open error=%d DstPath=%s \n", __FUNCTION__, errno, pczDstFilePath );
        return false;
    }
    
    while(true)
    {
        nToRead = sizeof(czBuffer);
        memset( czBuffer, 0, nToRead );
        nRead = read( nSrcFd, czBuffer, nToRead );
        if(nRead <= 0)
        {
            break;
        }
        
        nWritten = write( nDstFd, czBuffer, nRead );
        if(nWritten <= 0)
        {
            break;
        }
    }
    
    close( nDstFd );
    close( nSrcFd );
    printf( "[DLP][%s] Success. Src=%s, Dst=%s \n", __FUNCTION__, pczSrcFilePath, pczDstFilePath );
    return true;
}


boolean_t
CKextManager::PWDisk_SyCall_DeleteFile( const char* pczFilePath )
{
    int nRet=0;
    
    if(!pczFilePath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nRet = access( pczFilePath, 0 );
    if(nRet != 0)
    {
        printf( "[DLP][%s] File-NotExist. error=%d, Path=%s \n", __FUNCTION__, errno, pczFilePath );
        return false;
    }
    
    nRet = unlink( pczFilePath );
    if(nRet == -1)
    {
        printf( "[DLP][%s] unlink failed. error=%d, Path=%s \n", __FUNCTION__, errno, pczFilePath );
        return false;
    }
    
    printf( "[DLP][%s] Success. Path=%s \n", __FUNCTION__, pczFilePath );
    return true;
}


boolean_t  
CKextManager::PWDisk_rmdirs( char* pczDirPath, boolean_t bIsErrStop )
{
    struct stat St;
    struct dirent* pDirEntry=NULL;
    char   czFilePath[MAX_PATH];
    DIR*   pDirInfo=NULL;
    
    if(!pczDirPath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return -1;
    }
    
    pDirInfo = opendir( pczDirPath );
    if(!pDirInfo) return false;

    while(true)
    {
        pDirEntry = readdir( pDirInfo );
        if(!pDirEntry)
        {
            break;
        }
        
        if(0 == strcmp(pDirEntry->d_name, ".") || 0 == strcmp(pDirEntry->d_name, ".."))
        {
            continue;
        }
        
        memset( czFilePath, 0, sizeof(czFilePath) );
        snprintf( czFilePath, sizeof(czFilePath), "%s/%s", pczDirPath, pDirEntry->d_name );
        memset( &St, 0, sizeof(St) );
        
        // ?�일???�성(?�일???�형, ?�기, ?�성/변�??�간 ?�을 ?�기 ?�하??
        if(-1 == lstat(czFilePath, &St )) continue;
        
        if(S_ISDIR( St.st_mode ))
        {
            if(-1 == PWDisk_rmdirs(czFilePath, bIsErrStop) && true == bIsErrStop)
            {
                return -1;
            }
        }
        else if(S_ISREG(St.st_mode) || S_ISLNK(St.st_mode))
        {
            if(-1 == unlink(czFilePath) && true == bIsErrStop)
            {
                return -1;
            }
        }
    }
    closedir( pDirInfo );
    return true;
}


boolean_t
CKextManager::PWDisk_IsEmptyDir(char* pczDirPath)
{
    int  nFileCount = 0;
    struct stat St;
    struct dirent* pDirEntry=NULL;
    char   czFilePath[MAX_PATH];
    DIR*   pDirInfo=NULL;
    
    if(!pczDirPath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return -1;
    }
    
    pDirInfo = opendir( pczDirPath );
    if(!pDirInfo) return false;
    
    while(true)
    {
        pDirEntry = readdir( pDirInfo );
        if(!pDirEntry)
        {
            break;
        }
        
        if(0 == strcmp(pDirEntry->d_name, ".") || 0 == strcmp(pDirEntry->d_name, ".."))
        {
            continue;
        }
        
        memset( czFilePath, 0, sizeof(czFilePath) );
        snprintf( czFilePath, sizeof(czFilePath), "%s/%s", pczDirPath, pDirEntry->d_name );
        
        // ?�일???�성(?�일???�형, ?�기, ?�성/변�??�간 ?�을 ?�기 ?�하??
        memset( &St, 0, sizeof(St) );
        if(-1 == lstat(czFilePath, &St )) continue;
        
        if(S_ISREG(St.st_mode) || S_ISLNK(St.st_mode))
        {
            nFileCount++;
        }
    }
    closedir( pDirInfo );
    
    if(nFileCount > 0)
    {
        return false;
    }
    return true;
}


boolean_t
CKextManager::PWDisk_SyCall_DeleteDir( char* pczDirPath )
{
    int  nRet = 0;
    DIR* pDir = NULL;
    
    if(!pczDirPath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    pDir = opendir( pczDirPath );
    if(!pDir)
    {
        printf( "[DLP][%s] opendir failed. Path=%s \n", __FUNCTION__, pczDirPath );
        return false;
    }
    closedir( pDir );
    
    
    if(false == PWDisk_IsEmptyDir( pczDirPath ))
    {
        printf( "[DLP][%s] Failed. PWDisk_IsEmptyDir Path=%s \n", __FUNCTION__, pczDirPath );
        return false;
    }
    
    nRet = rmdir( pczDirPath );
    if(nRet == -1)
    {
        printf( "[DLP][%s] Failed. Path=%s \n", __FUNCTION__, pczDirPath );
        return false;
    }
    
    printf( "[DLP][%s] Succes. Path=%s \n", __FUNCTION__, pczDirPath );
    return true;
}


boolean_t
CKextManager::PWDisk_DeleteEmptyDir( char* pczDirPath )
{
    boolean_t bSuc = false;
    
    if(!pczDirPath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    bSuc = PWDisk_SyCall_DeleteDir( pczDirPath );
    printf( "[DLP][%s] Ret=%d, Path=%s \n", __FUNCTION__, bSuc, pczDirPath );
    return bSuc;
}


boolean_t
CKextManager::PWDisk_DeleteEmptyDir_Recur( char* pczDirPath )
{
    int   nPos=0, nLength=0;
    uint64_t nCopySize=0, nAllocSize=0;
    char*  pczToken = NULL;
    char*  pczPos = NULL;
    char*  pczNewDir = NULL;
    
    if(!pczDirPath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nPos = (int)strlen(pczDirPath);
    if(pczDirPath[ nPos-1 ] == '/')
    {
        pczDirPath[ nPos-1 ] = '\0';
    }
    
    // EmptyDir Delete
    if(false == PWDisk_DeleteEmptyDir( pczDirPath ))
    {
        printf( "[DLP][%s] PWDisk_DeleteEmptyDir Failed. Dir=%s \n", __FUNCTION__, pczDirPath );
        return false;
    }
    
    // Fetch ParentDir
    pczToken = strrchr( pczDirPath, '/' );
    if(!pczToken) return false;
    
    nCopySize  = (uint64_t)((uint64_t)pczToken - (uint64_t)pczDirPath);
    nAllocSize = nCopySize + sizeof(char)*2;
    
    // Memory Allocation
    pczNewDir = (char *)malloc( nAllocSize );
    if(!pczNewDir) return false;
    
    memset( pczNewDir, 0, nAllocSize );
    memcpy( pczNewDir, pczDirPath, nCopySize );
    
    pczPos = strstr( pczNewDir, PATH_SMSDISK_DRIVE  );
    if(!pczPos)
    {
        free( pczNewDir );
        pczNewDir = NULL;
        return false;
    }
    
    pczPos += strlen( PATH_SMSDISK_DRIVE );
    nLength = (int)strlen(pczPos);
    if(nLength <= 1)
    {
        free( pczNewDir );
        pczNewDir = NULL;
        return false;
    }
    
    PWDisk_DeleteEmptyDir_Recur( pczNewDir );
    printf( "[DLP][%s] Success. DirPath=%s \n", __FUNCTION__, pczNewDir );
    free( pczNewDir );
    return true;
}



boolean_t
CKextManager::PWDisk_SplitFilePath( char* pczFilePath, char* pczOutDirPath, char* pczOutFileName )
{
    int    nLength=0;
    char*  pczToken = NULL;
    char   czTempPath[MAX_PATH];
    
    if(!pczFilePath)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nLength = (int)strlen( pczFilePath );
    memset( czTempPath, 0, sizeof(czTempPath) );
    strncpy( czTempPath, pczFilePath, nLength );
    
    pczToken = strrchr( czTempPath, '/' );
    if(!pczToken) return false;
    
    (*pczToken) = '\0';
    pczToken++;
    
    nLength = (int)strlen( pczToken );
    if(pczOutFileName)
    {
        strncpy( pczOutFileName, pczToken, nLength );
    }
    
    nLength = (int)strlen( czTempPath );
    if(pczOutDirPath)
    {
        strncpy( pczOutDirPath, czTempPath, nLength );
    }
    
    printf( "[DLP][%s] DirPath=%s \n", __FUNCTION__, pczOutDirPath );
    printf( "[DLP][%s] FileName=%s \n", __FUNCTION__, pczOutFileName );
    return true;
}


boolean_t
CKextManager::PWDisk_FetchQuarantineFilePath( char* pczFilePath, char* pczOutQtFilePath, int& nOutQtFilePath, int& nErrCode )
{
    int  nPos=0, nLength=0;
    char czQtFilePath[MAX_PATH];
    char czDirPath[MAX_PATH];
    char czFileName[MAX_PATH];
    char czFileTemp[MAX_PATH];
    boolean_t bSuc = false;
    
    nErrCode = PWAPI_OK;
    
    if(!pczFilePath || !pczOutQtFilePath || nOutQtFilePath <= 0)
    {
        nErrCode = PWAPI_INVALIDARG_EINVAL;
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    memset( czQtFilePath, 0, sizeof(czQtFilePath) );
    memset( czDirPath,    0, sizeof(czDirPath)  );
    memset( czFileName,   0, sizeof(czFileName) );
    memset( czFileTemp,   0, sizeof(czFileTemp) );
    
    bSuc = PWDisk_SplitFilePath( pczFilePath, czDirPath, czFileName );
    
    do
    {
        if(nPos == 0)
        {
            sprintf( czFileTemp, "%s", czFileName );
        }
        else
        {
            sprintf( czFileTemp, "%s_[%d]", czFileName, nPos );
        }
        
        // Make Quarantine FilePath
        nLength = (int)strlen( czDirPath );
        if(nLength > 1)
        {
            if('/' == czDirPath[ nLength-1 ])
            {
                snprintf( czQtFilePath, sizeof(czQtFilePath), "%s/%s%s%s", m_DiskSize.czVolumePath, "SMSDisk/Drive", czDirPath, czFileTemp );
            }
            else
            {
                snprintf( czQtFilePath, sizeof(czQtFilePath), "%s/%s%s/%s", m_DiskSize.czVolumePath, "SMSDisk/Drive", czDirPath, czFileTemp );
            }
        }
        printf( "[DLP][%s] QtFilePath=%s \n", __FUNCTION__, czQtFilePath );
        
        if(0 != access( czQtFilePath, 0 ))
        {
            break;
        }
        
    } while(MAX_RETRY >= ++nPos);
    
    
    if(nPos > MAX_RETRY)
    {
        nErrCode = PWAPI_FAIL;
        printf( "[DLP][%s] MaxRetry Overed. \n", __FUNCTION__ );
        return false;
    }
    
    nLength = (int)strlen( czQtFilePath );
    if(nLength > nOutQtFilePath)
    {
        nOutQtFilePath = nLength;
        nErrCode = PWAPI_INVALID_BUFFERSIZE;
        printf( "[DLP][%s] Invalid Quarantine BufferSize. \n", __FUNCTION__ );
        return false;
    }
    
    strncpy( pczOutQtFilePath, czQtFilePath, nLength );
    nOutQtFilePath = nLength;
    
    printf( "[DLP][%s] Success. QtPath=%s \n", __FUNCTION__, czQtFilePath );
    return true;
}


boolean_t
CKextManager::PWDisk_ProcessIsQuarantineFile( char* pczFilePath, int& nErrCode )
{
    boolean_t bSuc = false;
    int   nOutQtFilePath=0;
    char  czOutQtFilePath[MAX_PATH];
    
    nErrCode = PWAPI_OK;
    if(!pczFilePath)
    {
        nErrCode = PWAPI_INVALIDARG_EINVAL;
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nOutQtFilePath = sizeof(czOutQtFilePath);
    memset( czOutQtFilePath, 0, sizeof(czOutQtFilePath) );
    
    bSuc = PWDisk_FetchQuarantineFilePath( pczFilePath, czOutQtFilePath, nOutQtFilePath, nErrCode );
    if(bSuc == false)
    {
        printf( "[DLP][%s] QuarantineFilePath Fetch Failed. \n", __FUNCTION__ );
        return false;
    }
    
    // Check for existence
    bSuc = access( czOutQtFilePath, 0 );
    printf( "[DLP][%s] Ret=%d QtPath=%s \n", __FUNCTION__, (int)bSuc, czOutQtFilePath );
    return bSuc;
    
}


boolean_t
CKextManager::PWDisk_ProcessQuarantineFile( char* pczFilePath, char* pczOutQtFilePath, int& nOutQtFilePath, int& nErrCode )
{
    int    nLength=0;
    char*  pczToken = NULL;
    char   czQtFilePath[MAX_PATH];
    
    nErrCode = PWAPI_OK;
    // Check Paramenter
    if(!pczFilePath || !pczOutQtFilePath || nOutQtFilePath <= 0)
    {
        nErrCode = PWAPI_INVALIDARG_EINVAL;
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nLength = sizeof(czQtFilePath);
    memset( czQtFilePath, 0, sizeof(czQtFilePath) );
    if(false == PWDisk_FetchQuarantineFilePath( pczFilePath, czQtFilePath, nLength, nErrCode ))
    {
        nOutQtFilePath = nLength;
        printf( "[DLP][%s] Failed. PWDisk_FetchQuarantineFilePath \n", __FUNCTION__ );
        return false;
    }
    
    strncpy( pczOutQtFilePath, czQtFilePath, nLength );
    nOutQtFilePath = nLength;
       
    pczToken = strrchr( czQtFilePath, '/' );
    if(pczToken)
    {
        *pczToken = '\0';
    }
       
    // CreateDirectory
    if(false == PWDisk_SyCall_CreateDir( czQtFilePath ))
    {
        nErrCode = PWAPI_CREATE_DIR_FAILED;
        printf( "[DLP][%s] Failed. PWDisk_SyCall_CreateDir DirPath=%s \n", __FUNCTION__, czQtFilePath );
        return false;
    }
       
    // CopyFile
    if(false == PWDisk_SyCall_CopyFile( pczFilePath, pczOutQtFilePath ))
    {
        nErrCode = PWAPI_COPY_FILE_FAILED;
        printf( "[DLP][%s] Failed. PWDisk_SyCall_CopyFile SrcPath=%s, DstFilePath=%s \n", __FUNCTION__, pczFilePath, pczOutQtFilePath );
        return false;
    }
    
    // DeleteFile
    if(false == PWDisk_SyCall_DeleteFile( pczFilePath ))
    {
        PWDisk_SyCall_DeleteFile( pczOutQtFilePath );
        
        nErrCode = PWAPI_DELETE_FILE_FAILED;
        printf( "[DLP][%s] Failed. PWDisk_SyCall_DeleteFile FilePath=%s \n", __FUNCTION__, pczFilePath );
        return false;
    }
    
    printf( "[DLP][%s] Success. FilePath=%s, QtFilePath=%s \n", __FUNCTION__, pczFilePath, pczOutQtFilePath );
    return true;
}


boolean_t
CKextManager::PWDisk_FetchReleaseFilePath( char* pczQtFilePath, char* pczDstDirPath, char* pczOutRelFilePath, int& nOutRelFilePath, int& nErrCode )
{
    int  nPos=0, nLength=0;
    char czRelFilePath[MAX_PATH];
    char czDirPath[MAX_PATH];
    char czFileName[MAX_PATH];
    char czFileTemp[MAX_PATH];
    boolean_t bSuc = false;
    
    nErrCode = PWAPI_OK;
    
    if(!pczQtFilePath || !pczDstDirPath || !pczOutRelFilePath || nOutRelFilePath <= 0)
    {
        nErrCode = PWAPI_INVALIDARG_EINVAL;
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    memset( czRelFilePath, 0, sizeof(czRelFilePath) );
    memset( czDirPath,    0, sizeof(czDirPath)  );
    memset( czFileName,   0, sizeof(czFileName) );
    memset( czFileTemp,   0, sizeof(czFileTemp) );
    
    bSuc = PWDisk_SplitFilePath( pczQtFilePath, czDirPath, czFileName );
    
    do
    {
        if(nPos == 0)
        {
            sprintf( czFileTemp, "%s", czFileName );
        }
        else
        {
            sprintf( czFileTemp, "%s_[%d]", czFileName, nPos );
        }
        
        // Make Release FilePath
        nLength = (int)strlen(pczDstDirPath);
        if(nLength >= 3)
        {
            if('/' == pczDstDirPath[ nLength-1 ])
            {
                snprintf( czRelFilePath, sizeof(czRelFilePath), "%s%s", pczDstDirPath, czFileTemp );
            }
            else
            {
                snprintf( czRelFilePath, sizeof(czRelFilePath), "%s/%s", pczDstDirPath, czFileTemp );
            }
        }
        
        printf( "[DLP][%s] ReleasePath=%s \n", __FUNCTION__, czRelFilePath );
    
        if(0 != access( czRelFilePath, 0 ))
        {
            break;
        }
        
    } while(MAX_RETRY >= ++nPos);
    
    
    if(nPos > MAX_RETRY)
    {
        nErrCode = PWAPI_FAIL;
        printf( "[DLP][%s] Max Retry Overed. \n", __FUNCTION__ );
        return false;
    }
    
    nLength = (int)strlen( czRelFilePath );
    if(nLength > nOutRelFilePath)
    {
        nOutRelFilePath = nLength;
        nErrCode = PWAPI_INVALID_BUFFERSIZE;
        printf( "[DLP][%s] Invalid Quarantine BufferSize. \n", __FUNCTION__ );
        return false;
    }
    
    strncpy( pczOutRelFilePath, czRelFilePath, nLength );
    nOutRelFilePath = nLength;
    
    printf( "[DLP][%s] Success, RelPath=%s \n", __FUNCTION__, czRelFilePath );
    return true;
}



boolean_t
CKextManager::PWDisk_ProcessQuarantineFile_Release( char* pczQtFilePath, char* pczDstDirPath, char* pczOutRelFilePath, int& nOutRelFilePath, int& nErrCode )
{
    int  nLength=0;
    char czOutDirPath[MAX_PATH];
    char czOutFileName[MAX_PATH];
    
    nErrCode = PWAPI_OK;
    if(!pczQtFilePath || !pczDstDirPath || !pczOutRelFilePath || nOutRelFilePath <= 0)
    {
        nErrCode = PWAPI_INVALIDARG_EINVAL;
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    // Fetch Release FilePath
    if(false == PWDisk_FetchReleaseFilePath( pczQtFilePath, pczDstDirPath, pczOutRelFilePath, nOutRelFilePath, nErrCode ))
    {
        nOutRelFilePath = nLength;
        printf( "[DLP][%s] Failed. FetchReleaseFilePath \n", __FUNCTION__ );
        return false;
    }

    // CreateDirectory
    if(false == PWDisk_SyCall_CreateDir( pczDstDirPath ))
    {
        nErrCode = PWAPI_CREATE_DIR_FAILED;
        printf( "[DLP][%s] Failed. PWDisk_SyCall_CreateDir DstDirPath=%s \n", __FUNCTION__, pczDstDirPath );
        return false;
    }
    
    // CopyFile
    if(false == PWDisk_SyCall_CopyFile( pczQtFilePath, pczOutRelFilePath ))
    {
        nErrCode = PWAPI_COPY_FILE_FAILED;
        printf( "[DLP][%s] Failed. PWDisk_SyCall_CopyFile RelPath=%s \n", __FUNCTION__, pczOutRelFilePath );
        return false;
    }
    
    // DeleteFile
    if(false == PWDisk_SyCall_DeleteFile( pczQtFilePath ))
    {
        nErrCode =  PWAPI_DELETE_FILE_FAILED;
        printf( "[DLP][%s] PWDisk_SyCall_DeleteFile Failed. QtFilePath=%s \n", __FUNCTION__, pczQtFilePath );
        return false;
    }
    
    //
    // 빈폴????��(?�당?�더 �??�더?�에 ?�???��??�으�??�작)
    //
    memset( czOutDirPath,  0, sizeof(czOutDirPath)  );
    memset( czOutFileName, 0, sizeof(czOutFileName) );
    
    if(false == PWDisk_SplitFilePath( pczQtFilePath, czOutDirPath, czOutFileName ))
    {
        return false;
    }
    
    if(false == PWDisk_DeleteEmptyDir_Recur( czOutDirPath ))
    {
         printf( "[DLP][%s] Failed. PWDisk_DeleteEmptyDir DirPath=%s \n", __FUNCTION__, czOutDirPath );
    }
    
    printf( "[DLP][%s] Success. QtPath=%s, RelPath=%s \n", __FUNCTION__, pczQtFilePath, pczOutRelFilePath );
    return true;
}


boolean_t
CKextManager::PWDisk_ProcessQuarantineFile_Delete( char* pczQtFilePath, int& nErrCode )
{
    char czOutDirPath[MAX_PATH];
    char czOutFileName[MAX_PATH];
    
    if(!pczQtFilePath)
    {
        nErrCode = PWAPI_INVALIDARG_EINVAL;
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return EFAULT;
    }

    // DeleteFile
    if(false == PWDisk_SyCall_DeleteFile( pczQtFilePath ))
    {
        nErrCode = PWAPI_DELETE_FILE_FAILED;
        printf( "[DLP][%s] PWDisk_ShellDeleteFile Failed. QtFilePath=%s \n", __FUNCTION__, pczQtFilePath );
        return false;
    }
    
    //
    // 빈폴????��(?�당?�더 �??�더?�에 ?�???��??�으�??�작)
    //
    memset( czOutDirPath,  0, sizeof(czOutDirPath)  );
    memset( czOutFileName, 0, sizeof(czOutFileName) );
    
    if(false == PWDisk_SplitFilePath( pczQtFilePath, czOutDirPath, czOutFileName ))
    {
        printf( "[DLP][%s] PWDisk_SplitFilePath Failed. \n", __FUNCTION__ );
        return false;
    }
    
    if(false == PWDisk_DeleteEmptyDir_Recur( czOutDirPath ))
    {
        printf( "[DLP][%s] PWDisk_DeleteEmptyDir Failed. DirPath=%s \n", __FUNCTION__, czOutDirPath );
    }
    printf( "[DLP][%s] Success. QtPath=%s \n", __FUNCTION__, pczQtFilePath );
    return true;
}


boolean_t
CKextManager::PWDisk_GetDiskSize( PWDISK_SIZE* pDiskSize )
{
    int nRet = 0;
    struct statvfs St;
    double nUsedSize = 0;

    if(!pDiskSize || strlen(pDiskSize->czVolumePath) <= 0)
    {
        return false;
    }
    
    memset( &St, 0, sizeof(St) );
    nRet = statvfs( pDiskSize->czVolumePath, &St );
    if(nRet != 0)
    {
        return false;
    }
    
    pDiskSize->nTotalSize = (double)(St.f_blocks * St.f_frsize); // / SIZE_KB;
    pDiskSize->nFreeSize  = (double)(St.f_bfree * St.f_frsize);   // / SIZE_KB;
    nUsedSize  = pDiskSize->nTotalSize - pDiskSize->nFreeSize;
    
    printf( "\n[DLP][%s] DiskPath=%s, total=%.0lfB \n", __FUNCTION__, pDiskSize->czVolumePath, pDiskSize->nTotalSize );
    printf( "[DLP][%s] DiskPath=%s,  free=%.0lfB \n", __FUNCTION__, pDiskSize->czVolumePath, pDiskSize->nFreeSize );
    printf( "[DLP][%s] DiskPath=%s,  used=%.0lfB \n\n", __FUNCTION__, pDiskSize->czVolumePath, nUsedSize );
    
    printf( "[DLP][%s] DiskPath=%s, total=%.2lfMB \n", __FUNCTION__, pDiskSize->czVolumePath, pDiskSize->nTotalSize/SIZE_MB );
    printf( "[DLP][%s] DiskPath=%s,  free=%.2lfMB \n", __FUNCTION__, pDiskSize->czVolumePath, pDiskSize->nFreeSize/SIZE_MB );
    printf( "[DLP][%s] DiskPath=%s,  used=%.2lfMB \n\n", __FUNCTION__, pDiskSize->czVolumePath, nUsedSize/SIZE_MB );
    
    printf( "[DLP][%s] DiskPath=%s, total=%.4lfGB \n", __FUNCTION__, pDiskSize->czVolumePath, pDiskSize->nTotalSize/SIZE_GB );
    printf( "[DLP][%s] DiskPath=%s,  free=%.4lfGB \n", __FUNCTION__, pDiskSize->czVolumePath, pDiskSize->nFreeSize/SIZE_GB );
    printf( "[DLP][%s] DiskPath=%s,  used=%.4lfGB \n\n", __FUNCTION__, pDiskSize->czVolumePath, nUsedSize/SIZE_GB );
    return true;
}


boolean_t
CKextManager::PWDisk_ConvertDiskSize( PWDISK_SIZE* pDiskSize, int nNewSize )
{
    int nOldSize = 0;
    
    if(!pDiskSize || strlen(pDiskSize->czVolumePath) <= 0)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    nOldSize = (int) round( pDiskSize->nTotalSize / SIZE_GB );
    printf( "[DLP][%s] OldSize=%dGB, nNewSize=%dGB. \n", __FUNCTION__, nOldSize, nNewSize );
    if(nNewSize <= nOldSize)
    {
        printf( "[DLP][%s] OldSize=%dGB, nNewSize=%dGB Invalid Request. \n", __FUNCTION__, nOldSize, nNewSize );
        return false;
    }

    // unload
    PWDisk_PWProcess_unload();
    // resize
    PWDisk_QuarantineDisk_resize( pDiskSize, nNewSize );
    // load
    PWDisk_PWProcess_load();
    return true;
}



boolean_t
CKextManager::PWDisk_ProcessConvertDiskSize()
{
    boolean_t  bSuc = false;
    int   nNewSize = 0, nOldSize=0;

    nOldSize = (int)round( m_DiskSize.nTotalSize/SIZE_GB );
    
    printf("\n");
    printf("DiskSize=%dGB NewDiskSize=", nOldSize );
    scanf( "%d", &nNewSize );
    if(nNewSize <= nOldSize)
    {
        printf( "[DLP][%s] Invalid NewDiskSize. \n", __FUNCTION__ );
    }
    
    bSuc = g_AppKext.PWDisk_ConvertDiskSize( &m_DiskSize,  nNewSize );
    if(bSuc == false)
    {
        printf( "[DLP][%s] VolumePath=%s, ConvertDiskSize Failed. Error=%d \n", __FUNCTION__, m_DiskSize.czVolumePath, errno );
        return false;
    }
    
    printf( "[DLP][%s] Success. VolumePath=%s \n", __FUNCTION__, m_DiskSize.czVolumePath );
    return true;
}


//
// Default AllowProcess
//
boolean_t
CKextManager::PWDisk_PWDefaultAllowProcess()
{
    ULONG nPID=0;
    ProtectCheck ProcCheck;
    char czProcPath[MAX_PATH];
    
    nPID = ProcCheck.GetProcessID( SPT_PROC_PISupervisor );
    if(nPID > 0)
    {
         printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_PISupervisor );
        memset( czProcPath, 0, sizeof(czProcPath) );
        if(true == ProcCheck.GetProcessPath( nPID, czProcPath, sizeof(czProcPath) ))
        {
            PWDisk_PWProcessID_Add( nPID );
        }
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIAgent );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIAgent );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_WebAgent );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_WebAgent );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIClient );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIClient );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIAgentChecker );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIAgentChecker );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIAutoScan );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIAutoScan );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIDelete );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIDelete );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIEncrypt );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIEncrypt );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIScanWorker );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIScanWorker );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PISelfPatch );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PISelfPatch );
        PWDisk_PWProcessID_Add( nPID );
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIUpdate );
    if(nPID > 0)
    {
        printf( "[DLP][%s] PID=%d, ProcStr=%s \n", __FUNCTION__, nPID, SPT_PROC_CMD_PIUpdate );
        PWDisk_PWProcessID_Add( nPID );
    }
    return true;
}




//
// SetSelfProtect
//

int CKextManager::fnSetSelfProtect_CommitComplete( void* pBuf, int nBufSize, ULONG nCommand )
{
    int nRet=0, nTotalSize =0;

    PCOMMAND_MESSAGE pCmdNew = NULL;
    PSELF_PROTECT    pProtect = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[DLP-App] SetPrintPrevent Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[DLP-App] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command = nCommand;
    
    pProtect = (PSELF_PROTECT)pCmdNew->Data;
    if(pProtect)
    {
        memcpy( pProtect, pBuf, nBufSize );
    }
    
    if(m_bKctl == true || g_AppKctl.m_nKernCtlId > 0)
    {
        nRet = g_AppKctl.SendCommand_KernCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );


    return nRet;
}

#endif


int CKextManager::fnSetSelfProtect_Commit( boolean_t bProtect, ULONG nCommand )
{
    int  nRet=0;

#ifdef _FIXME_

    ULONG nPID=0;
    ProtectCheck ProcCheck;
    SELF_PROTECT Protect;
    
    memset( &Protect, 0, sizeof(Protect) );
    Protect.bProtect = bProtect;
    strncpy( Protect.FilePath, SPT_FILEPATH, strlen(SPT_FILEPATH) );
    
    nPID = ProcCheck.GetProcessID( SPT_PROC_PISupervisor );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PLIST_PISupervisor, strlen(SPT_PLIST_PISupervisor) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIAgent );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PLIST_PIAgent, strlen(SPT_PLIST_PIAgent) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_WebAgent );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PLIST_WebAgent, strlen(SPT_PLIST_WebAgent) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIClient );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIClient, strlen(SPT_PROC_CMD_PIClient) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIAgentChecker );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIAgentChecker, strlen(SPT_PROC_CMD_PIAgentChecker) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIAutoScan );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIAutoScan, strlen(SPT_PROC_CMD_PIAutoScan) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIDelete );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIDelete, strlen(SPT_PROC_CMD_PIDelete) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIEncrypt );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIEncrypt, strlen(SPT_PROC_CMD_PIEncrypt) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIScanWorker );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIScanWorker, strlen(SPT_PROC_CMD_PIScanWorker) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PISelfPatch );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PISelfPatch, strlen(SPT_PROC_CMD_PISelfPatch) );
        Protect.nService++;
    }
    
    nPID = ProcCheck.GetProcessIDCommand( SPT_PROC_CMD_PIUpdate );
    if(nPID > 0)
    {
        Protect.Services[ Protect.nService ].nPID = nPID;
        strncpy(Protect.Services[ Protect.nService ].czPList, SPT_PROC_CMD_PIUpdate, strlen(SPT_PROC_CMD_PIUpdate) );
        Protect.nService++;
    }
    
    nRet = fnSetSelfProtect_CommitComplete( &Protect, sizeof(Protect), nCommand );

#endif

    return nRet;
}

#ifdef _NOTUSED_

boolean_t
CKextManager::ShellCommand_Execute( std::string strCommand, std::string& strResult )
{
    FILE*  pPipe = NULL;
    char   czBuffer[MAX_BUFFER];
    
    pPipe = popen( strCommand.c_str(), "r" );
    if(!pPipe) return false;
    
    while(!feof(pPipe))
    {
        memset(czBuffer, 0, sizeof(czBuffer) );
        if(fgets(czBuffer, MAX_BUFFER, pPipe) != NULL)
        {
            strResult += czBuffer;
        }
    }
    pclose( pPipe );
    return true;
}


boolean_t
CKextManager::PWDisk_GetFreeDiskPath( PWDISK_SIZE* pDiskSize )
{
    int nReturn = 0;
    char czPos  = '\0';
    char czDiskName[MAX_PATH];
    char czDiskPath[MAX_PATH];
    
    if(!pDiskSize)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    for(czPos='D'; czPos<='Z'; czPos++)
    {
        memset( czDiskName, 0, sizeof(czDiskName) );
        memset( czDiskPath, 0, sizeof(czDiskPath) );
        snprintf( czDiskName, sizeof(czDiskName), "PWDisk_%c", czPos );
        snprintf( czDiskPath, sizeof(czDiskPath), "/Volumes/%s", czDiskName  );
        
        nReturn = access( czDiskPath, 0 );
        if(nReturn != 0)
        {
            strncpy( pDiskSize->czVolumeName, czDiskName, strlen(czDiskName) );
            strncpy( pDiskSize->czVolumePath, czDiskPath, strlen(czDiskPath) );
            printf( "[DLP][%s] Free-VolumePath=%s, VolumeName=%s \n", __FUNCTION__, czDiskPath, czDiskName );
            return true;
        }
        else
        {
            printf( "[DLP][%s] Mount-VolumePath=%s, VolumeName=%s \n", __FUNCTION__, czDiskPath, czDiskName );
        }
    }
    
    return false;
}



/*
 if [ -d $VolumePath ]; then
 echo "detach $VolumePath"
 sudo hdiutil detach $VolumePath
 fi
 
 
 if [ -e $ImagePath ]; then
 echo "attach $ImagePath"
 sudo hdiutil attach $ImagePath
 else
 echo "mkdir -p $DirPath"
 sudo mkdir -p $DirPath
 echo "create $ImagePath"
 sudo hdiutil create $ImagePath -size 1g -fs HFS+ -volname $VolumeName -format UDRW -srcfolder $DirPath
 echo "attach $ImagePath"
 sudo hdiutil attach $ImagePath
 fi
 
 */


boolean_t
CKextManager::PWDisk_QuarantineDisk_mount( PWDISK_SIZE* pDiskSize )
{
    int nReturn = 0;
    boolean_t bSuc = false;
    std::string strCommand;
    std::string strResult;
    char czCommand[MAX_PATH];
    
    if(!pDiskSize)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }

    nReturn = access( pDiskSize->czVolumePath, 0 );
    if(nReturn == 0)
    {
        memset( czCommand, 0, sizeof(czCommand) );
        snprintf( czCommand, sizeof(czCommand), "sudo hdiutil detach %s", pDiskSize->czVolumePath );
        strCommand = czCommand;
        
        bSuc = ShellCommand_Execute( strCommand, strResult );
        printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );
    }

    nReturn = access( pDiskSize->czImagePath, 0 );
    if(nReturn == 0)
    {
        // attach $ImagePath
        memset( czCommand, 0, sizeof(czCommand) );
        snprintf( czCommand, sizeof(czCommand), "sudo hdiutil attach %s", pDiskSize->czImagePath );
        strCommand = czCommand;
        
        bSuc = ShellCommand_Execute( strCommand, strResult );
        printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );
    }
    else
    {
        // mkdir -p $DirPath
        memset( czCommand, 0, sizeof(czCommand) );
        snprintf( czCommand, sizeof(czCommand), "mkdir -p %s", pDiskSize->czDirPath );
        strCommand = czCommand;
        
        bSuc = ShellCommand_Execute( strCommand, strResult );
        printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );

        // create $Imagepath
        memset( czCommand, 0, sizeof(czCommand) );
        snprintf( czCommand, sizeof(czCommand), "sudo hdiutil create %s -size 1g -fs HFS+ -volname %s -format UDRW -srcfolder %s",
                    pDiskSize->czImagePath, pDiskSize->czVolumeName, pDiskSize->czDirPath );
        strCommand = czCommand;
        
        bSuc = ShellCommand_Execute( strCommand, strResult );
        printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );

        // attach $ImagePath
        memset( czCommand, 0, sizeof(czCommand) );
        snprintf( czCommand, sizeof(czCommand), "sudo hdiutil attach %s", pDiskSize->czImagePath );
        strCommand = czCommand;
        
        bSuc = ShellCommand_Execute( strCommand, strResult );
        printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand  );
    }
    return true;
}




/*
if [ "$1" == "unload" ]; then
echo "unload $VolumePath"
sudo hdiutil detach $VolumePath
exit 0
fi
*/

boolean_t
CKextManager::PWDisk_QuarantineDisk_unmount( PWDISK_SIZE* pDiskSize )
{
    boolean_t bSuc = false;
    std::string strCommand;
    std::string strResult;
    char czCommand[MAX_PATH];
    
    if(!pDiskSize)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }
    
    memset( czCommand, 0, sizeof(czCommand) );
    snprintf( czCommand, sizeof(czCommand), "sudo hdiutil detach %s", pDiskSize->czVolumePath );
    strCommand = czCommand;
    
    bSuc = ShellCommand_Execute( strCommand, strResult );
    printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand );
    if(bSuc == false) return false;

    return true;
}


/*
 if [ "$1" == "size" -a "$2" ]; then
 if [ -d $VolumePath ]; then
 echo "detach $VolumePath"
 sudo hdiutil detach $VolumePath
 fi
 
 echo "resize $2GB"
 sudo hdiutil resize -size "$2g" $ImagePath
 exit 0
 fi
 
 */


boolean_t
CKextManager::PWDisk_QuarantineDisk_resize( PWDISK_SIZE* pDiskSize, int nNewSize )
{
    int nReturn = 0;
    boolean_t bSuc = false;
    std::string strCommand;
    std::string strResult;
    char  czCommand[MAX_PATH];
    
    if(!pDiskSize)
    {
        printf( "[DLP][%s] invalid parameter. \n", __FUNCTION__ );
        return false;
    }

    nReturn = access( pDiskSize->czVolumePath, 0 );
    if(nReturn == 0)
    {
        memset( czCommand, 0, sizeof(czCommand) );
        snprintf( czCommand, sizeof(czCommand), "sudo hdiutil detach %s", pDiskSize->czVolumePath );
        strCommand = czCommand;
        
        bSuc = ShellCommand_Execute( strCommand, strResult );
        printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, bSuc, czCommand );
    }
    
    memset( czCommand, 0, sizeof(czCommand) );
    snprintf( czCommand, sizeof(czCommand), "sudo hdiutil resize -size %dg %s", nNewSize, pDiskSize->czImagePath );
    strCommand = czCommand;
    
    bSuc = ShellCommand_Execute( strCommand, strResult );
    printf( "[DLP][%s] return=%d, Command=%s \n", __FUNCTION__, nReturn, czCommand );
    if(bSuc == false) return false;
    
    return true;
}

boolean_t
CKextManager::PWDisk_DuplicateDiskSize( PWDISK_SIZE* pDiskSize )
{
	if( NULL == pDiskSize )
	{
		return false;
	}

    memset( pDiskSize, 0, sizeof(PWDISK_SIZE));
    memcpy( pDiskSize, &m_DiskSize, sizeof(PWDISK_SIZE));
    
	return true;
}

#endif
