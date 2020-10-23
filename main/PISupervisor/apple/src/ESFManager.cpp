//
//  ESFManager.cpp
//  PISupervisor
//
//  Created by Juno on 2020/07/07.
//  Copyright © 2020 somansa. All rights reserved.
//

#ifdef LINUX
#include <string>
#endif

#include "ESFManager.h"
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


#ifndef LINUX
#include <CoreServices/CoreServices.h>
#include <IOKit/storage/IOMedia.h>
#endif

LPCALLBACK   g_AppCallbackESF = NULL;
CESFManager g_AppESF;


int CESFManager::EventNotify_PostProcess( PEVT_PARAM pEvtInfo )
{
    int nRet = 0;
    
    if(!pEvtInfo) return 0;
    
    switch(pEvtInfo->Command)
    {
            
        // PISecSmartDrv.ESF
        case FileIsRemove:
            nRet = g_AppESF.EventNotify_FileIsRemove( pEvtInfo );
            break;
        case FileScan:
            nRet = g_AppESF.EventNotify_FileScan( pEvtInfo );
            break;
        case FileDelete:
            nRet = g_AppESF.EventNotify_FileDelete( pEvtInfo );
            break;
        case FileExchangeData:
            nRet = g_AppESF.EventNotify_FileExchangeData( pEvtInfo );
            break;
        case FileRename:
            nRet = g_AppESF.EventNotify_FileRename( pEvtInfo );
            break;
        case FileEventDiskFull:
            nRet = g_AppESF.EventNotify_FileEventDiskFull( pEvtInfo );
            break;
       
        case SmartLogNotify:
            nRet = g_AppESF.EventNotify_SmartLogNotify( pEvtInfo );
            break;
        
        case ProcessAccessCheck:
            nRet = g_AppESF.EventNotify_ProcessAccessCheck( pEvtInfo );
            break;
            
        default: break;
    }

    return nRet;
}

int CESFManager::EventNotify_FileIsRemove(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    printf( "[ESF][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CESFManager::EventNotify_FileScan(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[ESF][%s] pid=%d, Path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    printf( "[ESF][%s] pid=%d, QtPath=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pQtFilePath );
    return 0;
}

int CESFManager::EventNotify_FileDelete(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[ESF][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CESFManager::EventNotify_FileRename(PEVT_PARAM pEvtInfo )
{
    
    if(!pEvtInfo) return 0;
    
    printf( "[ESF][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CESFManager::EventNotify_FileExchangeData(PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[ESF][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

int CESFManager::EventNotify_FileEventDiskFull(PEVT_PARAM pEvtInfo )
{
    
    if(!pEvtInfo) return 0;
    
    printf( "[ESF][%s] pid=%d, path=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}


int CESFManager::EventNotify_SmartLogNotify(PEVT_PARAM pEvtInfo )
{
    PSMART_LOG_RECORD_EX pLogEx = NULL;
    
    if(!pEvtInfo) return 0;
    
    pLogEx = (PSMART_LOG_RECORD_EX)pEvtInfo->pEvtCtx;
    if(!pLogEx) return 0;
    
    printf( "[%s][ESF][%s] policy=%d AccessType=%d, RecordType=%d, Major=%d, Minor=%d, Result=%d, pid=%d, proc=%s path=%s \n",
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

int CESFManager::EventNotify_ProcessAccessCheck( PEVT_PARAM pEvtInfo )
{
    if(!pEvtInfo) return 0;
    
    printf( "[ESF][%s] pid=%d, czFilePath=%s \n", __FUNCTION__, pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    return 0;
}

CESFManager::CESFManager()
{
    
}

CESFManager::~CESFManager()
{
    
}

boolean_t
CESFManager::IsRemovable( char* pczDevice )
{
#ifndef LINUX
    kern_return_t nResult = 0;
    io_iterator_t iter = 0;
    io_service_t  service = 0;
    boolean_t     bRemovable = false;

    CFMutableDictionaryRef matchingDict = IOBSDNameMatching(kIOMasterPortDefault, 0, pczDevice );
    if(matchingDict == NULL)
    {
        printf("[ESF] IOBSDNameMatching() returned NULL\n");
        return false;
    }
    
    service = IOServiceGetMatchingService(kIOMasterPortDefault, matchingDict);
    if(IO_OBJECT_NULL == service)
    {
        printf("[ESF] IOServiceGetMatchingService() returned NULL\n");
        return false;
    }
    
    nResult = IORegistryEntryCreateIterator( service, kIOServicePlane, kIORegistryIterateRecursively | kIORegistryIterateParents, &iter );
    if(KERN_SUCCESS != nResult)
    {
        printf("[ESF] IORegistryEntryCreateIterator() returned 0x%08x\n", nResult);
        IOObjectRelease( service );
        return false;
    }
    
    if(IO_OBJECT_NULL == iter)
    {
        printf("[ESF] IORegistryEntryCreateIterator() returned a NULL iterator\n");
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


int CESFManager::ESFModule_Startup(LPCALLBACK pCallback, boolean_t bPWDisk )
{
    int nRet = 0;
    if( NULL == pCallback )
    {
        pCallback = reinterpret_cast<LPCALLBACK>(CESFManager::EventNotify_PostProcess);
    }
    RegisterCallback(pCallback);
    DEBUG_LOG("RegisterCallback() Call. \n", __FUNCTION__ );
    DEBUG_LOG("CommStartControl( bPWDisk=%d ) \n", __FUNCTION__, bPWDisk );
    nRet = CommStartControl( bPWDisk );
    return nRet;
}

int CESFManager::ESFModule_Cleanup( boolean_t bPWDisk )
{
    int nRet = 0;
    DEBUG_LOG("[ESF][%s] CommStopControl( bPWDisk=%d ) \n", __FUNCTION__, bPWDisk );
    nRet = CommStopControl( bPWDisk );
    return nRet;
}

void CESFManager::RegisterCallback( LPCALLBACK pCallback )
{
    if(pCallback == NULL)
    {
        return;
    }
    
    if(g_AppCallbackESF != NULL)
    {
        g_AppCallbackESF = NULL;
    }

    g_AppCallbackESF = pCallback;
    
    if(g_AppCallback != NULL)
    {
        g_AppCallback = NULL;
    }
    
    g_AppCallback = pCallback;
    
    DEBUG_LOG("[ESF] CESFManager Callback Function Reset: %p \n", pCallback );
}


int CESFManager::CommStartControl( boolean_t bPWDisk )
{
    int nRet = 0;

    nRet = g_AppESFctl.ESFControl_Init();

    return nRet;
}


int CESFManager::CommStopControl( boolean_t bPWDisk )
{
    int nRet = 0;

    nRet = g_AppESFctl.ESFControl_Uninit();

    return nRet;
}

int CESFManager::fnGetDrivePolicy(void* pBuf, int nLength )
{
    int  nRet=0;
    int  nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    
    if(!pBuf || nLength <= 0)
    {
        printf("[ESF] GetDrivePolicy Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nLength;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = GetDrivePolicy;
    memcpy( &pCmdNew->Data, pBuf, nLength );
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetDrivePolicy( void* pBuf, int nLength )
{
    int  nRet=0;
    int  nTotalSize =0;
    PCOMMAND_MESSAGE    pCmdNew   = NULL;
    
    if(!pBuf || nLength <= 0)
    {
        printf("[ESF] SetDrivePolicy Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nLength;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetDrivePolicy;
    memcpy( &pCmdNew->Data, pBuf, nLength );
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    free( pCmdNew );
    return nRet;
}

int CESFManager::fnSetExceptDrivePolicy( void* pBuf, int nLength )
{
    int  nRet=0;
    int  nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    
    if(!pBuf || nLength <= 0)
    {
        printf("[ESF] SetExceptDrivePolicy Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nLength;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = SetExceptDrivePolicy;
    memcpy( &pCmdNew->Data, pBuf, nLength );
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetQuarantinePathExt(PQT_CTX pQtCtx)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_QT_INFO    pCommQtInfo = NULL;
    
    if(!pQtCtx)
    {
        printf("[ESF] SetQuarantinePathExt Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(QT_CTX);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}

#ifndef Linux
int CESFManager::fnSetQuarantineLimit(boolean_t bQtLimit)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_QT_INFO    pCommQtInfo = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(QT_CTX);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetUSBMobilePermit( MB_PERMIT& PermitInfo )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_PERMIT  pCommPermit = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_PERMIT);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetUSBMobilePermitList(std::vector<MB_PERMIT>* pPermitList)
{
    int nRet=0, nPos=0, nCount=0;
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
    return nRet;
}


int CESFManager::fnClrUSBMobilePermitList()
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_PERMIT pCommPermit = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_PERMIT);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = ClrMobilePermitList;
    pCommPermit = (PCOMM_MB_PERMIT)pCmdNew->Data;
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetCtrlMobile(PMB_POLICY pPolicy)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_POLICY  pMbPolicy = NULL;
    
    if(!pPolicy) return EINVAL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}

int CESFManager::fnMobileNodeInsert_SetMtp(const char* pczBasePath, const char* pczKeyword )
{
    int      nReturn = 0;
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
    return nReturn;
}


int CESFManager::fnMobileNodeInsert_SetPtp(const char* pczBasePath, const char* pczKeyword )
{
    int      nReturn = 0;
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
    return nReturn;
}

int CESFManager::fnMobileNodeInsert(PMB_NODE pNode)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PCOMM_MB_NODE    pMbNode = NULL;
    
    if(!pNode)
    {
        printf("[ESF] MobileNodeInsert Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_NODE);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}

int CESFManager::fnMobileNodeRemoveAll()
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
   
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(COMM_MB_NODE);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
        return errno;
    }
    
    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size = nTotalSize;
    pCmdNew->Command  = MobileNodeRemoveAll;
    
    printf( "[ESF][%s] SendCommand_KernCtl \n", __FUNCTION__ );
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}
#endif

int CESFManager::fnSetProcessIdUnprotect(ULONG ProcessId)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PALLOWPROCESSID  pAllowProcessID = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(ALLOWPROCESSID);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetPermitProcessName(void* pBuf, int nBufSize)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE  pCmdNew = NULL;
    PALLOWPROCESSNAME pAllowProcName = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[ESF] RemoveAndroidDeviceNode Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}



int CESFManager::fnSetPermitFolderName(void* pBuf, int nBufSize)
{
    int  nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PALLOWFOLDERNAME pAllowFolderName = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[ESF] RemoveAndroidDeviceNode Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;

}


int CESFManager::fnSetPermitFileExtName(void* pBuf, int nBufSize)
{
    int   nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE  pCmdNew = NULL;
    PALLOWFILEEXTNAME pAllowFileExt = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[ESF] RemoveAndroidDeviceNode Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;

}


int CESFManager::fnGetSmartLogEx()
{
    return 0;
}


int CESFManager::fnSetPrintPrevent_Off()
{
    int nRet = 0;
    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );
    
    policy.bPolicy = false;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );
    return nRet;
}

int CESFManager::fnSetPrintPrevent_Allow()
{
    int nRet = 0;
    PRINT_POLICY policy;
    memset( &policy, 0, sizeof(policy) );
    
    policy.bPolicy = true;
    policy.nPolicyType = MEDIA_PRINTPREVENT;
    
    nRet = fnSetPrintPrevent( &policy, sizeof(policy) );
    return nRet;
}

int CESFManager::fnSetPrintPrevent_Watermark()
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

int CESFManager::fnSetPrintPrevent_DisableWrite()
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

int CESFManager::fnSetPrintPrevent_CopyPrevent()
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

int CESFManager::fnSetPrintPrevent(void* pBuf, int nBufSize )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PPRINT_POLICY    pPolicy = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[ESF] SetPrintPrevent Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetUploadPrevent(void* pBuf, int nBufSize )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PUPLOAD_POLICY    pPolicy = NULL;
    
    if(!pBuf || nBufSize <= 0)
    {
        printf("[ESF] SetUploadPrevent Invalid Parameter. \n");
        return -1;
    }
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + nBufSize;
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}

int CESFManager::fnSetProcessAccessCheck( BOOLEAN bProcAC, BOOLEAN bLog )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    PROC_AC_POLICY*  pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(PROC_AC_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        DEBUG_LOG( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


// SetControlCamera
int CESFManager::fnSetControlCamera( bool bControl, bool bLog  )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    CAMERA_POLICY*  pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(CAMERA_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


// SetControlRNDIS
int CESFManager::fnSetControlRndis( bool bRNdisCtrl, bool bRNdisLog )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    RNDIS_POLICY* pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(RNDIS_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


// SetControlAirDrop
int CESFManager::fnSetControlAirDrop( bool bAirDropBlock, bool bAirDropLog )
{
    int nRet=0, nTotalSize =0;
    PCOMMAND_MESSAGE pCmdNew = NULL;
    AIRDROP_POLICY* pPolicy = NULL;
    
    nTotalSize = sizeof(COMMAND_MESSAGE) + sizeof(AIRDROP_POLICY);
    pCmdNew = (PCOMMAND_MESSAGE)malloc( nTotalSize );
    if(pCmdNew == NULL)
    {
        printf( "[ESF] malloc failed(%d) \n", errno  );
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}

//
// SetSelfProtect
//

int CESFManager::fnSetSelfProtect_CommitComplete( void* pBuf, int nBufSize, ULONG nCommand )
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
    
    if(g_AppESFctl.m_nESFCtlId > 0)
    {
        nRet = g_AppESFctl.SendCommand_ESFCtl( pCmdNew );
    }
    
    if(pCmdNew) free( pCmdNew );
    return nRet;
}


int CESFManager::fnSetSelfProtect_Commit( boolean_t bProtect, ULONG nCommand )
{
    int  nRet=0;

#ifndef LINUX

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
