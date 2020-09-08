#include <stdio.h>

#ifdef LINUX
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
    #include <sys/time.h>
    #include <unistd.h>    
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "SmartCmd.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <chrono>

#include "kauth_listener.h"
#include "KextDeviceNotify.h"


#define MAX_CHECK  16
#define MAX_FPATH  260
typedef struct _LOG_CHECK
{
    int  nPID;
    int  nPolicyType;
    int  nLogType;
    int  nAction;
    uint64_t nTimeStamp;
    char  czFilePath[MAX_FPATH];
} LOG_CHECK, *PLOG_CHECK;

typedef struct _CHECK_CTX
{
    LOG_CHECK Data[MAX_CHECK];
    //mach_timebase_info_data_t TimeBase;
    long TimeBase;
    bool bInit;
} CHECK_CTX, *PCHECK_CTX;


CHECK_CTX  g_Check  = {0, };


#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef __cplusplus
extern "C" {
#endif

void* MyMAlloc(size_t nAllocSize);

#ifdef __cplusplus
}
#endif

extern char* sms_strupr(char* pczStr);

errno_t
SmartCmd_Parser_CM( PCOMMAND_MESSAGE pCmdMsg, void* pParam )
{
    errno_t      nError = 0;
    
    if(!pCmdMsg) return -1;
    
    switch(pCmdMsg->Command)
    {
        case SetDrivePolicy:
            LOG_MSG("[DLP][%s] SetDrivePolicy \n", __FUNCTION__ );
            nError = SmartCmd_SetDrivePolicy( pCmdMsg );
            break;
        case SetExceptDrivePolicy:
            LOG_MSG("[DLP][%s] SetExceptDrivePolicy \n", __FUNCTION__ );
            nError = SmartCmd_SetExceptDrivePolicy( pCmdMsg );
            break;
        case SetPermitProcessName:
            LOG_MSG("[DLP][%s] SetPermitProcessName \n", __FUNCTION__ );
            nError = SmartCmd_SetPermitProcessName( pCmdMsg );
            break;
        case SetPermitFolderName:
            LOG_MSG("[DLP][%s] SetPermitFolderName  \n", __FUNCTION__ );
            nError = SmartCmd_SetPermitFolderName( pCmdMsg );
            break;
        case SetPermitFileExtName:
            LOG_MSG("[DLP][%s] SetPermitFileExtName \n", __FUNCTION__ );
            nError = SmartCmd_SetPermitFileExtName( pCmdMsg );
            break;
        case SetCtrlMobile:
            nError = SmartCmd_SetCtrlMobile( pCmdMsg );
            break;
        case MobileNodeRemoveAll:
             nError = SmartCmd_MobileNode_RemoveAll( pCmdMsg );
             break;
        case SetMobilePermitList:
            nError = SmartCmd_SetMobilePermitList( pCmdMsg );
            break;
        case ClrMobilePermitList:
            LOG_MSG("[DLP][%s] ClrMobilePermitList \n", __FUNCTION__ );
            nError = SmartCmd_ClrMobilePermitList( pCmdMsg );
            break;
        case SetQuarantinePathExt:
            LOG_MSG("[DLP][%s] SetQuarantinePathExt \n", __FUNCTION__ );
            nError = SmartCmd_SetQtPathExt( pCmdMsg );
            break;

        case SetQuarantineLimit:
            LOG_MSG("[DLP][%s] SetQuarantineLimit \n", __FUNCTION__ );
            nError = SmartCmd_SetQtLimit( pCmdMsg );
            break;

        case SetPrintPrevent:
            LOG_MSG("[DLP][%s] SetPrinterPrevent \n", __FUNCTION__ );
            nError = SmartCmd_SetPrintPrevent( pCmdMsg );
            break;
        case SetSelfProtect: // 1081
            LOG_MSG("[DLP][%s] SetSelfProtect \n", __FUNCTION__ );
            nError = SmartCmd_SetSelfProtect( pCmdMsg );
            break;
        case SetUploadPrevent:
            LOG_MSG("[DLP][%s] SetUploadPrevent \n", __FUNCTION__ );
            nError = SmartCmd_SetUploadPrevent( pCmdMsg );
            break;
        case SetProcessAccessCheck:
            LOG_MSG("[DLP][%s] SetProcessAccessCheck \n", __FUNCTION__ );
            nError = SmartCmd_SetProcessAC( pCmdMsg );
            break;
        case SetControlCamera:
            printf("[DLP][%s] SetControlCamera \n", __FUNCTION__ );
            nError = SmartCmd_SetControlCamera( pCmdMsg );
            break;
        case SetControlRNDIS:
            printf("[DLP][%s] SetControlRNDIS \n", __FUNCTION__ );
            nError = SmartCmd_SetControlRNDIS( pCmdMsg );
            break;
        case SetControlAirDrop:
            printf("[DLP][%s] SetControlAirDrop \n", __FUNCTION__ );
            nError = SmartCmd_SetControlAirDrop( pCmdMsg );
            break;
        
        default:
            printf("[DLP][%s] NOT IMPLEMENTED !!!!\n", __FUNCTION__ );
            break;
    }
    return nError;
}


errno_t
SmartCmd_SetDrivePolicy( PCOMMAND_MESSAGE pCmdMsg )
{
    errno_t nError   = 0;
    int     nPos=0, nArrayEx=0, nLength  = 0;
    PATTACHED_DEVICE_EX pArrayEx = NULL;
    Boolean bUseSelectedBluetoothBlock = FALSE;
    Boolean bBluetoothLog = FALSE;

    if(!pCmdMsg) return EINVAL;

    pArrayEx = (PATTACHED_DEVICE_EX)pCmdMsg->Data;
    if(!pArrayEx) return EINVAL;

    nLength = pCmdMsg->Size - sizeof(ULONG)*2;
    nArrayEx = nLength/sizeof(ATTACHED_DEVICE_EX);

    printf("[DLP] SmartCmd_SetDrivePolicy. MAX_POLICY=%d, nArrayEx=%d. \n", MAX_POLICY, nArrayEx );
    memset( &g_DrvKext.DrvCtx.Policy, 0, sizeof(g_DrvKext.DrvCtx.Policy) );
    g_DrvKext.DrvCtx.Policy.nCount = nArrayEx;
    for(nPos=0; nPos<nArrayEx; nPos++)
    {
        if(nPos > MAX_POLICY)
        {
            printf( "[DLP] SmartCmd_SetDrivePolicy Skip. MAX_POLICY=%d, nPos=%d. \n", MAX_POLICY, nPos );
            break;
        }

        memset( &g_DrvKext.DrvCtx.Policy.DeviceEx[nPos], 0, sizeof(ATTACHED_DEVICE_EX));
        memcpy( &g_DrvKext.DrvCtx.Policy.DeviceEx[nPos], &pArrayEx[nPos], sizeof(ATTACHED_DEVICE_EX) );
        printf("[DLP] SetDrivePolicy: nPos=%d, DeviceType=0x%03x, Logging=%d DisableAll=%d DisableRead=%d, DisableWrite=%d nReserved1=%d Device=%s \n",
                nPos,
                (int)g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].ulStorDevType,
                (int)g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.bLoggingOn,
                (int)g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.bDisableAll,
                (int)g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.bDisableRead,
                (int)g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.bDisableWrite,
                (int)g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.nReserved1,
                g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.DeviceNames  );

        if(0 == strcmp(g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.DeviceNames, "Media\\Bluetooth"))
        {
            if(1 == g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.nReserved1)
            {
                bUseSelectedBluetoothBlock = TRUE;
            }

            if(1 == g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].cDevice.bLoggingOn)
            {
                bBluetoothLog = TRUE;
            }
        }
    }

    // Selectedbluetooth block flag setting
    g_DrvKext.MediaCtx.Bth.bSelectedBluetoothBlock = bUseSelectedBluetoothBlock;
    g_DrvKext.MediaCtx.Bth.bBluetoothLog = bBluetoothLog;

    DrvCtx_Policy_Update();
    return nError;
}


// SetExceptDrivePolicy
errno_t
SmartCmd_SetExceptDrivePolicy( PCOMMAND_MESSAGE pCmdMsg )
{
    errno_t   nError = 0;
    int       nPos=0, nLength=0, nArray=0;
    PATTACHED_DEVICE pArray = NULL;

    if(!pCmdMsg) return EINVAL;

    pArray = (PATTACHED_DEVICE)pCmdMsg->Data;
    if(!pArray) return EINVAL;

    nLength = pCmdMsg->Size - sizeof(ULONG)*2;
    nArray = nLength/sizeof(ATTACHED_DEVICE);
    LOG_MSG("[DLP] SetExceptDrivePolicy: nArray=%d \n", nArray );

    memset( g_DrvKext.DrvCtx.Except.DriveEx, 0, sizeof(g_DrvKext.DrvCtx.Except.DriveEx) );
    g_DrvKext.DrvCtx.Except.nCount = nArray;
    for(nPos=0; nPos<nArray; nPos++)
    {
        if(nArray > MAX_DRIVE)
        {
            break;
        }
        //g_DrvKext.DrvCtx.Except.DriveEx[nPos].ulStorDevType = EXCEPT_FLAG_POLCY;
        g_DrvKext.DrvCtx.Except.DriveEx[nPos].ulStorDevType = 0x0008; //Media\\Removable

        memcpy( &g_DrvKext.DrvCtx.Except.DriveEx[nPos].cDevice, &pArray[nPos], sizeof(ATTACHED_DEVICE) );

        LOG_MSG( "[DLP][%s][%d] DeviceName=%s \n",
                 __FUNCTION__, nPos, g_DrvKext.DrvCtx.Except.DriveEx[nPos].cDevice.DeviceNames );
    }

    DrvCtx_Policy_Update();
    return nError;
}

errno_t
SmartCmd_SetPermitProcessName( PCOMMAND_MESSAGE pCmdMsg )
{
    ULONG   nLength=0, nMaxPos=0, nPos=0;
    PALLOWPROCESSNAME pAllowProcName = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        g_DrvKext.nAllowProcName = 0;
        memset( g_DrvKext.AllowProcName, 0, sizeof(ALLOWPROCESSNAME)*MAX_ALLOW_PROCNAME );
        return EINVAL;
    }

    pAllowProcName = (PALLOWPROCESSNAME)pCmdMsg->Data;
    if(pAllowProcName)
    {
        nLength = pCmdMsg->Size - sizeof(ULONG)*2;
        g_DrvKext.nAllowProcName = nLength/sizeof(ALLOWPROCESSNAME);

        memset( g_DrvKext.AllowProcName, 0, sizeof(ALLOWPROCESSNAME)*MAX_ALLOW_PROCNAME );
        memcpy( g_DrvKext.AllowProcName, pAllowProcName, sizeof(ALLOWPROCESSNAME)*g_DrvKext.nAllowProcName );

        nMaxPos = g_DrvKext.nAllowProcName;
        for(nPos=0; nPos<nMaxPos; nPos++)
        {
            LOG_MSG( "[DLP][%s] proc=%s \n", __FUNCTION__, g_DrvKext.AllowProcName[nPos].czAllowProcName );
        }
    }
    return 0;
}


errno_t
SmartCmd_SetPermitFolderName( PCOMMAND_MESSAGE pCmdMsg )
{
    ULONG  nLength=0, nCopySize=0;
    PALLOWFOLDERNAME pAllowFolder = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        g_DrvKext.nAllowFolder = 0;
        memset( g_DrvKext.AllowFolder, 0, sizeof(ALLOWFOLDERNAME)*MAX_FOLDER_NAME );
        return EINVAL;
    }

    pAllowFolder = (PALLOWFOLDERNAME)pCmdMsg->Data;
    if(pAllowFolder)
    {
        nLength = pCmdMsg->Size - sizeof(ULONG)*2;
        g_DrvKext.nAllowFolder = nLength/sizeof(ALLOWFOLDERNAME);

        nCopySize = min(sizeof(ALLOWFOLDERNAME)*MAX_FOLDER_NAME, sizeof(ALLOWFOLDERNAME)*g_DrvKext.nAllowFolder);
        memset( g_DrvKext.AllowFolder, 0, sizeof(g_DrvKext.AllowFolder) );
        memcpy( g_DrvKext.AllowFolder, pAllowFolder, nCopySize );
        for(int i=0; i<g_DrvKext.nAllowFolder; i++)
        {
            sms_strupr( g_DrvKext.AllowFolder[i].czAllowFolder );
            LOG_MSG("[DLP][%s] Index=%d, FolderName=%s \n", __FUNCTION__, i, g_DrvKext.AllowFolder[i].czAllowFolder );
        }
    }
    return 0;
}


errno_t
SmartCmd_SetPermitFileExtName( PCOMMAND_MESSAGE pCmdMsg )
{
    ULONG             nLength=0, nCopySize=0;
    PALLOWFILEEXTNAME pAllowFileExt = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        g_DrvKext.nAllowFileExt = 0;
        memset( g_DrvKext.AllowFileExt, 0, sizeof(ALLOWFILEEXTNAME)*MAX_FOLDER_NAME );
        return EINVAL;
    }

    pAllowFileExt = (PALLOWFILEEXTNAME)pCmdMsg->Data;
    if(pAllowFileExt)
    {
        nLength = pCmdMsg->Size - sizeof(ULONG)*2;
        g_DrvKext.nAllowFileExt = nLength/sizeof(ALLOWFILEEXTNAME);

        nCopySize = min(sizeof(ALLOWFILEEXTNAME)*MAX_FOLDER_NAME, sizeof(ALLOWFILEEXTNAME)*g_DrvKext.nAllowFileExt );
        memset( g_DrvKext.AllowFileExt, 0, sizeof(g_DrvKext.AllowFileExt) );
        memcpy( g_DrvKext.AllowFileExt, pAllowFileExt, nCopySize );
        for(int i=0; i<g_DrvKext.nAllowFileExt; i++)
        {
            sms_strupr( g_DrvKext.AllowFileExt[i].czAllowFileExt );
            LOG_MSG("[DLP][%s] Index=%d, FileExt=%s \n", __FUNCTION__, i, g_DrvKext.AllowFileExt[i].czAllowFileExt );
        }
    }

    return 0;
}


errno_t
SmartCmd_SetQtPathExt( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_QT_INFO pQtInfo = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }

    pQtInfo = (PCOMM_QT_INFO)pCmdMsg->Data;
    if(!pQtInfo) return EINVAL;

    memset( g_DrvKext.QtCtx.Entry, 0, sizeof(g_DrvKext.QtCtx.Entry) );
    memcpy( g_DrvKext.QtCtx.Entry, pQtInfo->QtCtx.Entry, sizeof(g_DrvKext.QtCtx.Entry) );
    g_DrvKext.QtCtx.ulCount = pQtInfo->QtCtx.ulCount;

    LOG_MSG("[DLP][%s] QtPath=%s \n", __FUNCTION__, g_DrvKext.QtCtx.Entry[0].czBuffer );
    return 0;
}


errno_t
SmartCmd_SetQtLimit( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_QT_INFO pQtInfo = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }

    pQtInfo = (PCOMM_QT_INFO)pCmdMsg->Data;
    if(!pQtInfo) return EINVAL;

    if(FALSE == pQtInfo->QtCtx.bQtLimit)
    {
        LOG_MSG("[DLP][%s] g_DrvKext.QtCtx.bQtLimit == FALSE. \n", __FUNCTION__ );
        g_DrvKext.QtCtx.bQtLimit = FALSE;
    }
    else
    {
        LOG_MSG("[DLP][%s] g_DrvKext.QtCtx.bQtLimit == TRUE. \n", __FUNCTION__ );
        g_DrvKext.QtCtx.bQtLimit = TRUE;
    }
    return 0;
}


boolean_t USBMobilePermitList_Append( MB_PERMIT* pPermit )
{
    int  i=0, nLength=0;
    PMB_PERMIT pNewPermit = NULL;
    PMB_PERMIT pPosPermit = NULL;
    MB_PERMIT PermitInfo;

    if(!pPermit)
    {
        LOG_MSG( "[DLP][%s] INVALID PARAMETER. \n", __FUNCTION__ );
        return false;
    }

    memset( &PermitInfo, 0, sizeof(MB_PERMIT) );
    memcpy( &PermitInfo, pPermit, sizeof(MB_PERMIT) );

    lck_rw_lock_exclusive( g_DrvKext.MobileCtx.RwLock );
    
    for(i=0; i<USBMOBILE_PERMIT_COUNT; i++)
    {
        pPosPermit = g_DrvKext.MobileCtx.pNewPermitList[i];
        if(pPosPermit)
        {
            continue;
        }

        nLength = sizeof(MB_PERMIT);
        // pNewPermit = _MALLOC( nLength, 1, M_ZERO );
        pNewPermit = (PMB_PERMIT)MyMAlloc( nLength );
        if(!pNewPermit)
        {
            LOG_MSG( "[DLP][%s] INSUFFICIENT BUFFER. \n", __FUNCTION__ );
            break;
        }
        printf( "[DLP][%s] Index=%d, medianame=%s, mediaid=%s \n", __FUNCTION__, i, PermitInfo.czMediaName, PermitInfo.czMediaID );
        memset( pNewPermit, 0, nLength );
        memcpy( pNewPermit, &PermitInfo, nLength );
        g_DrvKext.MobileCtx.pNewPermitList[i] = pNewPermit;
        pNewPermit = NULL;
        break;
    }
    lck_rw_unlock_exclusive( g_DrvKext.MobileCtx.RwLock );
    
    return true;
}


void USBMobilePermitList_RemoveAll()
{
    MB_PERMIT* pPosPermit = NULL;
    
    lck_rw_lock_exclusive( g_DrvKext.MobileCtx.RwLock );
    for(int i=0; i<USBMOBILE_PERMIT_COUNT; i++)
    {
        pPosPermit = g_DrvKext.MobileCtx.pNewPermitList[i];
        if(pPosPermit == NULL)
        {
            continue;
        }
        _FREE( g_DrvKext.MobileCtx.pNewPermitList[i], 1 );
        g_DrvKext.MobileCtx.pNewPermitList[i] = NULL;
    }
    lck_rw_unlock_exclusive( g_DrvKext.MobileCtx.RwLock );
    
}



errno_t
SmartCmd_SetMobilePermitList( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_MB_PERMIT pCommPermit = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }

    pCommPermit = (PCOMM_MB_PERMIT)pCmdMsg->Data;
    if(!pCommPermit) return EINVAL;

    USBMobilePermitList_Append( &pCommPermit->Permit );
    return 0;
}


errno_t
SmartCmd_ClrMobilePermitList( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_MB_PERMIT pCommPermit = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }

    pCommPermit = (PCOMM_MB_PERMIT)pCmdMsg->Data;
    if(!pCommPermit) return EINVAL;

    USBMobilePermitList_RemoveAll();
    return 0;
}


errno_t
SmartCmd_SetCtrlMobile( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_MB_POLICY pMbPolicy = NULL;
    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }
    
    pMbPolicy = (PCOMM_MB_POLICY)pCmdMsg->Data;
    if(pMbPolicy)
    {
        g_DrvKext.MobileCtx.Policy = pMbPolicy->Policy;

        LOG_MSG( "[DLP][%s] bMtpLog=%d bMtpBlock=%d bPtpLog=%d, bPtpBlock=%d \n", __FUNCTION__,
                 (boolean_t)g_DrvKext.MobileCtx.Policy.bMtpLog,
                 (boolean_t)g_DrvKext.MobileCtx.Policy.bMtpBlock,
                 (boolean_t)g_DrvKext.MobileCtx.Policy.bPtpLog,
                 (boolean_t)g_DrvKext.MobileCtx.Policy.bPtpBlock  );
    }
    SetProtectUsbMobileNotify();
    
    return 0;
}

errno_t
SmartCmd_MobileNode_Insert( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_MB_NODE pMbNode = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }

    pMbNode = (PCOMM_MB_NODE)pCmdMsg->Data;
    if(pMbNode)
    {
       //
       // MobileNode_Insert( &pMbNode->Node );
       //
    }
    return 0;
}


errno_t
SmartCmd_MobileNode_RemoveAll( PCOMMAND_MESSAGE pCmdMsg )
{
    PCOMM_MB_NODE pMbNode = NULL;

    if(!pCmdMsg || pCmdMsg->Size <= 0)
    {
        return EINVAL;
    }

    pMbNode = (PCOMM_MB_NODE)pCmdMsg->Data;
    if(pMbNode)
    {
        //
        // MobileNode_RemoveAll();
        //
    }
    return 0;
}


void MobileNode_Insert(PMB_NODE pNode)
{
    int      i=0, nLength= 0;
    PMB_NODE pNewNode = NULL;
    PMB_NODE pPosNode = NULL;

    if(!pNode)
    {
        LOG_MSG( "[DLP][%s] INVALID 	PARAMETER. \n", __FUNCTION__ );
        return;
    }

    nLength  = sizeof(MB_NODE);	
    // pNewNode = _MALLOC( nLength, 1, M_ZERO );
    pNewNode = (PMB_NODE)MyMAlloc( nLength );
    if(!pNewNode)
    {
        LOG_MSG( "[DLP][%s] INSUFFICIENT BUFFER. \n", __FUNCTION__ );
        return;
    }
    memcpy( pNewNode, pNode, sizeof(MB_NODE) );

    lck_rw_lock_exclusive( g_DrvKext.MobileCtx.RwLock );
    for(i=0; i<MB_NODE_COUNT; i++)
    {
        pPosNode = g_DrvKext.MobileCtx.pNewMBNodes[i];
        if(pPosNode != NULL)
        {
            if(pNewNode->MbType != pPosNode->MbType) continue;
            if(strcmp(pNewNode->czBasePath, pPosNode->czBasePath) == 0 && strcmp(pNewNode->czKeyword, pPosNode->czKeyword) == 0)
            {
                _FREE( pNewNode, 1 );
                pNewNode = NULL;
                break;
            }
            continue;
        }

        g_DrvKext.MobileCtx.pNewMBNodes[i] = pNewNode;
        pNewNode = NULL;
        break;
    }
    lck_rw_unlock_exclusive( g_DrvKext.MobileCtx.RwLock );

    if(pNewNode != NULL)
    {
        _FREE( pNewNode, 1);
    }
}

void MobileNode_RemoveAll()
{
    PMB_NODE pPosNode = NULL;

   //  LOG_MSG("[DLP][%s] \n", __FUNCTION__ );

    lck_rw_lock_exclusive( g_DrvKext.MobileCtx.RwLock );
    for(int i=0; i<MB_NODE_COUNT; i++)
    {
        pPosNode = g_DrvKext.MobileCtx.pNewMBNodes[i];
        if(pPosNode == NULL)
        {
            continue;
        }

        _FREE( g_DrvKext.MobileCtx.pNewMBNodes[i], 1 );
        g_DrvKext.MobileCtx.pNewMBNodes[i] = NULL;
    }
    lck_rw_unlock_exclusive( g_DrvKext.MobileCtx.RwLock );
}


// Upload Control
errno_t
SmartCmd_SetUploadPrevent( PCOMMAND_MESSAGE pCmdMsg )
{
    PUPLOAD_POLICY pPolicy = NULL;

    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }
    
    return KERN_SUCCESS;
}

// Printer Control
errno_t
SmartCmd_SetPrintPrevent( PCOMMAND_MESSAGE pCmdMsg )
{
    PPRINT_POLICY pPolicy = NULL;

    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }

    pPolicy = (PPRINT_POLICY)pCmdMsg->Data;
    if(!pPolicy)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_VALUE \n", __FUNCTION__ );
        return KERN_INVALID_VALUE;
    }

    LOG_MSG("[DLP][%s] PRINT_POLICY bPolicy: %d, bWatermark: %d, czDeviceName: %s, nPolicyType: %d \n", __FUNCTION__,
            (int)pPolicy->bPolicy, (int)pPolicy->bWatermark, pPolicy->czDeviceName, pPolicy->nPolicyType);

    // Print Policy Set
    memset( &g_DrvKext.PrintCtx.Policy, 0, sizeof(g_DrvKext.PrintCtx.Policy) );
    g_DrvKext.PrintCtx.Policy = (*pPolicy);

    return KERN_SUCCESS;
}

// SelfProtect
errno_t
SmartCmd_SetSelfProtect( PCOMMAND_MESSAGE pCmdMsg )
{
    ULONG nCount=0, nProcID=0, nLength=0, nIndex=0;
    char  czProcName[MAX_PROC_NAME];
    PSELF_PROTECT pProtect = NULL;

    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }

    pProtect = (PSELF_PROTECT)pCmdMsg->Data;
    if(!pProtect)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_VALUE \n", __FUNCTION__ );
        return KERN_INVALID_VALUE;
    }

    memset( &g_DrvKext.SelfCtx, 0, sizeof(g_DrvKext.SelfCtx) );
    g_DrvKext.SelfCtx.bProtect = pProtect->bProtect;
    strncpy( g_DrvKext.SelfCtx.FilePath, pProtect->FilePath, strlen(pProtect->FilePath)+1 );
    nCount = min(pProtect->nService, SPT_SERVICE_COUNT);
    for(int i=0; i<nCount; i++)
    {
        memset( czProcName, 0, sizeof(czProcName) );
        {
            LOG_MSG( "[DLP][%s] pid=%d, proc=%s, plist=%s \n", __FUNCTION__,
                    pProtect->Services[i].nPID, czProcName, pProtect->Services[i].czPList );
            memcpy( &g_DrvKext.SelfCtx.Services[nIndex++], &pProtect->Services[i], sizeof(SPT_SERVICE) );
        }
    }
    g_DrvKext.SelfCtx.nService = nIndex;
    
    return KERN_SUCCESS;
}

// SetProcessAccessCheck
errno_t
SmartCmd_SetProcessAC( PCOMMAND_MESSAGE pCmdMsg )
{
    PROC_AC_POLICY* pPolicy = NULL;
    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }
    pPolicy = (PROC_AC_POLICY*)pCmdMsg->Data;
    if(!pPolicy)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_VALUE \n", __FUNCTION__ );
        return KERN_INVALID_VALUE;
    }
    g_DrvKext.ProcCtx.Policy.bLog = pPolicy->bLog;
    g_DrvKext.ProcCtx.Policy.bProcAC = pPolicy->bProcAC;
    LOG_MSG("[DLP][%s] ProcessAccessCheck=%d, bLog=%d \n", __FUNCTION__, (int)pPolicy->bProcAC, (int)pPolicy->bLog );
    return KERN_SUCCESS;
}

errno_t
SmartCmd_SetControlCamera( PCOMMAND_MESSAGE pCmdMsg )
{
    PCAMERA_POLICY pPolicy = NULL;

    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }

    pPolicy = (CAMERA_POLICY*)pCmdMsg->Data;
    if(!pPolicy)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_VALUE \n", __FUNCTION__ );
        return KERN_INVALID_VALUE;
    }

    memset( &g_DrvKext.MediaCtx.Camera, 0, sizeof(g_DrvKext.MediaCtx.Camera) );
    g_DrvKext.MediaCtx.Camera.bCameraLog = pPolicy->bCameraLog;
    g_DrvKext.MediaCtx.Camera.bCameraBlock = pPolicy->bCameraBlock;
    g_DrvKext.MediaCtx.Camera.nReserved = pPolicy->nReserved;
    printf("[DLP][%s] bCameraBlock=%d, bLog=%d \n", __FUNCTION__, (int)pPolicy->bCameraBlock, (int)pPolicy->bCameraLog );

    SetProtect_Camera_UsbDeviceNotify();
    
    return KERN_SUCCESS;
}

errno_t
SmartCmd_SetControlRNDIS( PCOMMAND_MESSAGE pCmdMsg )
{
    PRNDIS_POLICY pPolicy = NULL;

    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }

    pPolicy = (RNDIS_POLICY*)pCmdMsg->Data;
    if(!pPolicy)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_VALUE \n", __FUNCTION__ );
        return KERN_INVALID_VALUE;
    }

    memset( &g_DrvKext.MediaCtx.Rndis, 0, sizeof(g_DrvKext.MediaCtx.Rndis) );
    g_DrvKext.MediaCtx.Rndis.bRNdisLog = pPolicy->bRNdisLog;
    g_DrvKext.MediaCtx.Rndis.bRNdisBlock = pPolicy->bRNdisBlock;
    printf("[DLP][%s] bBlock=%d, bLog=%d \n", __FUNCTION__, (int)pPolicy->bRNdisBlock, (int)pPolicy->bRNdisLog );

    SetProtect_RNDIS_UsbDeviceNotify();
    SetProtect_RNDIS_BthDeviceNotify();
    
    return KERN_SUCCESS;
}

errno_t
SmartCmd_SetControlAirDrop( PCOMMAND_MESSAGE pCmdMsg )
{
    AIRDROP_POLICY* pPolicy = NULL;

    if(!pCmdMsg)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_ARGUMENT \n", __FUNCTION__ );
        return KERN_INVALID_ARGUMENT;
    }

    pPolicy = (AIRDROP_POLICY*)pCmdMsg->Data;
    if(!pPolicy)
    {
        LOG_MSG("[DLP][%s] KERN_INVALID_VALUE \n", __FUNCTION__ );
        return KERN_INVALID_VALUE;
    }

    memset( &g_DrvKext.MediaCtx.AirDrop, 0, sizeof(g_DrvKext.MediaCtx.AirDrop) );
    g_DrvKext.MediaCtx.AirDrop.bAirDropLog = pPolicy->bAirDropLog;
    g_DrvKext.MediaCtx.AirDrop.bAirDropBlock = pPolicy->bAirDropBlock;
    g_DrvKext.MediaCtx.AirDrop.nReserved0 = pPolicy->nReserved0;
    g_DrvKext.MediaCtx.AirDrop.nReserved1 = pPolicy->nReserved1;
    g_DrvKext.MediaCtx.AirDrop.nReserved2 = pPolicy->nReserved2;
    g_DrvKext.MediaCtx.AirDrop.nReserved3 = pPolicy->nReserved3;
    printf("[DLP][%s] bBlock=%d, bLog=%d \n", __FUNCTION__, (int)pPolicy->bAirDropBlock, (int)pPolicy->bAirDropLog );
    
    return KERN_SUCCESS;
}

// Print Prevent
kern_return_t DrvPrintCtx_Init()
{
    kern_return_t nRet = 0;

    memset( &g_DrvKext.PrintCtx, 0, sizeof(g_DrvKext.PrintCtx) );

    if (0 == pthread_mutex_init( &g_DrvKext.PrintCtx._pPrtMtxGrp, NULL))
    {
        g_DrvKext.PrintCtx.pPrtMtxGrp = &g_DrvKext.PrintCtx._pPrtMtxGrp;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_grp_alloc_init() failed\n", __FUNCTION__);
        nRet = KERN_FAILURE;
    }

    if (0 == pthread_mutex_init( &g_DrvKext.PrintCtx._pPrtMtx, NULL))
    {
        g_DrvKext.PrintCtx.pPrtMtx = &g_DrvKext.PrintCtx._pPrtMtx;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() failed\n", __FUNCTION__);
        nRet = KERN_FAILURE;
    }

    return nRet;
}

kern_return_t DrvPrintCtx_Uninit()
{
    kern_return_t nRet = 0;
    if(g_DrvKext.PrintCtx.pPrtMtx != NULL)
    {
        //lck_mtx_free( g_DrvKext.PrintCtx.pPrtMtx, g_DrvKext.PrintCtx.pPrtMtx
        pthread_mutex_destroy( g_DrvKext.PrintCtx.pPrtMtx );
        g_DrvKext.PrintCtx.pPrtMtx = NULL;
    }
    
    if(g_DrvKext.PrintCtx.pPrtMtxGrp != NULL)
    {
        //lck_grp_free( g_DrvKext.PrintCtx.pPrtMtxGrp );
        pthread_mutex_destroy( g_DrvKext.PrintCtx.pPrtMtxGrp );
        g_DrvKext.PrintCtx.pPrtMtxGrp = NULL;
    }
    return nRet;
}


// Upload Prevent
kern_return_t DrvUploadCtx_Init()
{
    kern_return_t nRet = 0;

    memset( &g_DrvKext.UploadCtx, 0, sizeof(g_DrvKext.UploadCtx) );

    if (0 == pthread_mutex_init( &g_DrvKext.UploadCtx._pPrtMtxGrp, NULL))
    {
        g_DrvKext.UploadCtx.pPrtMtxGrp = &g_DrvKext.UploadCtx._pPrtMtxGrp;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_grp_alloc_init() failed\n", __FUNCTION__);
        nRet = KERN_FAILURE;
    }

    //g_DrvKext.UploadCtx.pPrtMtx = lck_mtx_alloc_init( g_DrvKext.UploadCtx.pPrtMtxGrp, LCK_ATTR_NULL );
    //if(NULL == g_DrvKext.UploadCtx.pPrtMtx)
    if (0 == pthread_mutex_init( &g_DrvKext.UploadCtx._pPrtMtx, NULL))
    {
        g_DrvKext.UploadCtx.pPrtMtx = &g_DrvKext.UploadCtx._pPrtMtx;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() failed\n", __FUNCTION__);
        nRet = KERN_FAILURE;
    }

    if (0 == pthread_mutex_init( &g_DrvKext.UploadCtx._pPrtMtxFileAccessTime, NULL))
    {
        g_DrvKext.UploadCtx.pPrtMtxFileAccessTime = &g_DrvKext.UploadCtx._pPrtMtxFileAccessTime;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() - pPrtMtxFileAccessTime failed\n", __FUNCTION__);
        nRet = KERN_FAILURE;
    }

    if (0 == pthread_mutex_init( &g_DrvKext.UploadCtx._pPrtMtxFileAccessInfo, NULL))
    {
        g_DrvKext.UploadCtx.pPrtMtxFileAccessInfo = &g_DrvKext.UploadCtx._pPrtMtxFileAccessInfo;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() - pPrtMtxFileAccessInfo failed\n", __FUNCTION__);
        nRet = KERN_FAILURE;
    }

    return nRet;
}

kern_return_t DrvUploadCtx_Uninit()
{
    kern_return_t nRet = 0;

    if(g_DrvKext.UploadCtx.pPrtMtxFileAccessInfo != NULL)
    {
        //lck_mtx_free( g_DrvKext.UploadCtx.pPrtMtxFileAccessInfo, g_DrvKext.UploadCtx.pPrtMtxGrp );
        pthread_mutex_destroy( g_DrvKext.UploadCtx.pPrtMtxFileAccessInfo );
        g_DrvKext.UploadCtx.pPrtMtxFileAccessInfo = NULL;
    }

    if(g_DrvKext.UploadCtx.pPrtMtxFileAccessTime != NULL)
    {
        //lck_mtx_free( g_DrvKext.UploadCtx.pPrtMtxFileAccessTime, g_DrvKext.UploadCtx.pPrtMtxGrp );
        pthread_mutex_destroy(  g_DrvKext.UploadCtx.pPrtMtxFileAccessTime );
        g_DrvKext.UploadCtx.pPrtMtxFileAccessTime = NULL;
    }

    if(g_DrvKext.UploadCtx.pPrtMtx != NULL)
    {
        //lck_mtx_free( g_DrvKext.UploadCtx.pPrtMtx, g_DrvKext.UploadCtx.pPrtMtxGrp );
        pthread_mutex_destroy( g_DrvKext.UploadCtx.pPrtMtx );
        g_DrvKext.UploadCtx.pPrtMtx = NULL;
    }

    if(g_DrvKext.UploadCtx.pPrtMtxGrp != NULL)
    {
        //lck_grp_free( g_DrvKext.UploadCtx.pPrtMtxGrp );
        pthread_mutex_destroy( g_DrvKext.UploadCtx.pPrtMtxGrp );
        g_DrvKext.UploadCtx.pPrtMtxGrp = NULL;
    }
    return nRet;
}

kern_return_t DrvMobile_Init()
{
    kern_return_t nResult = KERN_SUCCESS;

    // Mobile Init
    memset( &g_DrvKext.MobileCtx, 0, sizeof(g_DrvKext.MobileCtx) );

    if (0 == pthread_mutex_init( &g_DrvKext.MobileCtx._RwLockGrp, NULL))
    {
        g_DrvKext.MobileCtx.RwLockGrp = &g_DrvKext.MobileCtx._RwLockGrp;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_grp_alloc_init() failed\n", __FUNCTION__);
        nResult = KERN_FAILURE;
    }

    if (0 == pthread_mutex_init( &g_DrvKext.MobileCtx._RwLock, NULL))
    {
        g_DrvKext.MobileCtx.RwLock = &g_DrvKext.MobileCtx._RwLock;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() failed\n", __FUNCTION__);
        nResult = KERN_FAILURE;
    }
    return nResult;
}


kern_return_t DrvMobile_Uninit()
{
    MobileNode_RemoveAll();

    USBMobilePermitList_RemoveAll();

    if(g_DrvKext.MobileCtx.RwLock != NULL)
    {
        //lck_rw_free( g_DrvKext.MobileCtx.RwLock, g_DrvKext.MobileCtx.RwLockGrp );
        pthread_mutex_destroy( g_DrvKext.MobileCtx.RwLock );
        g_DrvKext.MobileCtx.RwLock = NULL;
    }

    if(g_DrvKext.MobileCtx.RwLockGrp != NULL)
    {
        //lck_grp_free( g_DrvKext.MobileCtx.RwLockGrp );
        pthread_mutex_destroy( g_DrvKext.MobileCtx.RwLockGrp );
        g_DrvKext.MobileCtx.RwLockGrp = NULL;
    }

    // Mobile Uninit
    memset( &g_DrvKext.MobileCtx, 0, sizeof(g_DrvKext.MobileCtx) );
    return KERN_SUCCESS;
}



kern_return_t
DrvCtx_Init()
{
    kern_return_t nResult = KERN_SUCCESS;

    memset( &g_DrvKext.DrvCtx, 0, sizeof(g_DrvKext.DrvCtx) );

    if (0 == pthread_mutex_init( &g_DrvKext.DrvCtx._DrvLockGroup, NULL))
    {
        g_DrvKext.DrvCtx.DrvLockGroup = &g_DrvKext.DrvCtx._DrvLockGroup;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_grp_alloc_init() failed\n", __FUNCTION__);
        nResult = KERN_FAILURE;
    }

    if (0 == pthread_mutex_init( &g_DrvKext.DrvCtx._DrvLock, NULL))
    {
        g_DrvKext.DrvCtx.DrvLock = &g_DrvKext.DrvCtx._DrvLock;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() failed\n", __FUNCTION__);
        nResult = KERN_FAILURE;
    }

    // Volume lock
    memset( &g_DrvKext.VolCtx, 0, sizeof(g_DrvKext.VolCtx) );
    if (0 == pthread_mutex_init( &g_DrvKext.VolCtx._VolLockGroup, NULL))
    {
        g_DrvKext.VolCtx.VolLockGroup = &g_DrvKext.VolCtx._VolLockGroup;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_grp_alloc_init() failed\n", __FUNCTION__);
        nResult = KERN_FAILURE;
    }

    if (0 == pthread_mutex_init( &g_DrvKext.VolCtx._VolLock, NULL))
    {
        g_DrvKext.VolCtx.VolLock = &g_DrvKext.VolCtx._VolLock;
    }
    else
    {
        LOG_MSG("[DLP][%s] lck_mtx_alloc_init() failed\n", __FUNCTION__);
        nResult = KERN_FAILURE;
    }
    g_DrvKext.VolCtx.nCheckCount = 0;
    return nResult;
}


kern_return_t
DrvCtx_Uninit()
{
    // VolCtx lock
    if (g_DrvKext.VolCtx.VolLock != NULL)
    {
        pthread_mutex_destroy( g_DrvKext.VolCtx.VolLock );
        g_DrvKext.VolCtx.VolLock = NULL;
    }

    if(g_DrvKext.VolCtx.VolLockGroup != NULL)
    {
        pthread_mutex_destroy( g_DrvKext.VolCtx.VolLockGroup );
        g_DrvKext.VolCtx.VolLockGroup = NULL;
    }

    // DrvKext
    if (g_DrvKext.DrvCtx.DrvLock != NULL)
    {
        pthread_mutex_destroy( g_DrvKext.DrvCtx.DrvLock );
        g_DrvKext.DrvCtx.DrvLock = NULL;
    }

    if(g_DrvKext.DrvCtx.DrvLockGroup != NULL)
    {
        pthread_mutex_destroy( g_DrvKext.DrvCtx.DrvLockGroup );
        g_DrvKext.DrvCtx.DrvLockGroup = NULL;
    }

    memset( &g_DrvKext.DrvCtx, 0, sizeof(g_DrvKext.DrvCtx) );
    return KERN_SUCCESS;
}


BOOLEAN
DrvCtx_Policy_Apply( PATTACHED_DEVICE_EX pPolDeviceEx )
{
    if(!pPolDeviceEx) return FALSE;

    if(pPolDeviceEx->cDevice.bLoggingOn  ||
       pPolDeviceEx->cDevice.bDisableAll || pPolDeviceEx->cDevice.bDisableRead || pPolDeviceEx->cDevice.bDisableWrite)
    {
        g_DrvKext.CtrlDeviceEx[ g_DrvKext.nDeviceCount ].cDevice       = pPolDeviceEx->cDevice;
        g_DrvKext.CtrlDeviceEx[ g_DrvKext.nDeviceCount ].ulStorDevType = pPolDeviceEx->ulStorDevType;

        g_DrvKext.nDeviceCount++;
        return TRUE;
    }
    return FALSE;
}


kern_return_t
DrvCtx_Policy_Update_DriveName( PVOLUME_DEVICE pVolDevice )
{
    ULONG  ulMaxPos=0, nPos=0, ulBusType=0;
    ULONG  ulPolCode=0, ulDeviceType=0;
    PATTACHED_DEVICE_EX pExceptEx = NULL;
    ATTACHED_DEVICE_EX  PolDeviceEx;

    if(!pVolDevice) return KERN_INVALID_ARGUMENT;

    memset( &PolDeviceEx, 0, sizeof(PolDeviceEx) );
    pExceptEx = DrvCtx_Except_Search( pVolDevice->czBasePath );
    if(pExceptEx)
    {
        lck_mtx_lock( g_DrvKext.DrvCtx.DrvLock );
        DrvCtx_Policy_Apply( pExceptEx );
        lck_mtx_unlock( g_DrvKext.DrvCtx.DrvLock );

        LOG_MSG( "[DLP]-[DrvCtx_Policy_Update_DriveName] Except-Policy Apply. \n" );
        return KERN_SUCCESS;
    }

    if(g_DrvKext.DrvCtx.Policy.nCount <= 0)
    {
        return KERN_SUCCESS;
    }

    lck_mtx_lock( g_DrvKext.DrvCtx.DrvLock );

    ulMaxPos = g_DrvKext.DrvCtx.Policy.nCount;
    for(nPos=0; nPos<ulMaxPos; nPos++)
    {
        memset( &PolDeviceEx, 0, sizeof(PolDeviceEx) );
        ulPolCode   = g_DrvKext.DrvCtx.Policy.DeviceEx[nPos].ulStorDevType;
        PolDeviceEx = g_DrvKext.DrvCtx.Policy.DeviceEx[nPos]; // SerDrivePolicy

        memset( PolDeviceEx.cDevice.DeviceNames, 0, DEVICE_NAME_SZ );
        strncpy( PolDeviceEx.cDevice.DeviceNames, pVolDevice->czBasePath, strlen(pVolDevice->czBasePath)+1 );

        ulBusType = pVolDevice->ulBusType;
        ulDeviceType = pVolDevice->ulDeviceType;
        if(ulBusType == BusTypeUnknown)
        {
            ulBusType = GetBusTypeFromVolume( pVolDevice->czBasePath );
        }

        if(ulBusType == BusTypeUsb || ulBusType == BusType1394 || ulBusType == BusTypeThunderBolt)
        {
            if(ulPolCode == g_pDeviceType_V6_List[DEVICE_V6_COPY].nCode)
            {
                DrvCtx_Policy_Apply( &PolDeviceEx );
                LOG_MSG( "[DLP][%s] CopyPrevent, BusType=%02d, Device=%s, BasePath=%s  \n",
                         __FUNCTION__, pVolDevice->ulBusType, pVolDevice->czDeviceName, pVolDevice->czBasePath );
                lck_mtx_unlock(g_DrvKext.DrvCtx.DrvLock);
                return KERN_SUCCESS;
            }
            else if(ulPolCode == g_pDeviceType_V6_List[DEVICE_V6_USB].nCode)
            {
                DrvCtx_Policy_Apply( &PolDeviceEx );

                LOG_MSG( "[DLP][%s] USB RW-Ctrl, BusType=%02d, Device=%s, BasePath=%s  \n",
                        __FUNCTION__, pVolDevice->ulBusType, pVolDevice->czDeviceName, pVolDevice->czBasePath );

                lck_mtx_unlock(g_DrvKext.DrvCtx.DrvLock);
                return KERN_SUCCESS;
             }
        }
        else if(ulBusType == BusTypeAtapi)
        {
            // CD/DVD
            if(ulPolCode == g_pDeviceType_V6_List[ DEVICE_V6_CDDVD ].nCode)
            {
                DrvCtx_Policy_Apply( &PolDeviceEx );

                LOG_MSG( "[DLP][%s] CDDVD-RW-Ctrl, BusType=%02d, Device=%s, BasePath=%s  \n",
                        __FUNCTION__, pVolDevice->ulBusType, pVolDevice->czDeviceName, pVolDevice->czBasePath );

                lck_mtx_unlock(g_DrvKext.DrvCtx.DrvLock);
                return KERN_SUCCESS;
            }
        }
        else if(ulBusType == BusTypeSFolder)
        {
            // NetDrive
            if(ulPolCode == g_pDeviceType_V6_List[ DEVICE_V6_NETDRIVE ].nCode)
            {
                DrvCtx_Policy_Apply( &PolDeviceEx );

                LOG_MSG( "[DLP][%s] SFolder-Ctrl, BusType=%02d, Device=%s, BasePath=%s  \n",
                        __FUNCTION__, pVolDevice->ulBusType, pVolDevice->czDeviceName, pVolDevice->czBasePath );

                lck_mtx_unlock(g_DrvKext.DrvCtx.DrvLock);
                return KERN_SUCCESS;
            }
        }
    }

    lck_mtx_unlock(g_DrvKext.DrvCtx.DrvLock);
    return KERN_SUCCESS;
}


kern_return_t
DrvCtx_Policy_Update()
{
    kern_return_t kernResult = KERN_SUCCESS;
    ULONG    nVol=0, nMaxPos=0;

    g_DrvKext.nDeviceCount = 0;
    memset( g_DrvKext.CtrlDeviceEx, 0, sizeof(ATTACHED_DEVICE_EX)*MAX_DEVICE );

    nMaxPos = g_DrvKext.VolCtx.nCount;
    for(nVol=0; nVol<nMaxPos; nVol++)
    {
        kernResult = DrvCtx_Policy_Update_DriveName( &g_DrvKext.VolCtx.VolumeDevice[nVol] );
        if( KERN_SUCCESS != kernResult)
        {
            printf( "[DLP][DrvCtx_Policy_Update_DriveName] Failed VolumeDevice=%s, Status=%x \n", g_DrvKext.VolCtx.VolumeDevice[nVol].czDeviceName,  kernResult );
        }
    }
    
    return kernResult;
}



PATTACHED_DEVICE_EX
DrvCtx_Except_Search( CHAR* pwzVolume )
{
    ULONG nPos=0, nMaxPos=0;
    if(!pwzVolume) return NULL;
    if(g_DrvKext.DrvCtx.Except.nCount <= 0) return NULL;

    nMaxPos = g_DrvKext.DrvCtx.Except.nCount;

    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        LOG_MSG( "Except DeviceNames :[%s] \n", g_DrvKext.DrvCtx.Except.DriveEx[nPos].cDevice.DeviceNames );
        if(0 == strcasecmp( g_DrvKext.DrvCtx.Except.DriveEx[nPos].cDevice.DeviceNames, pwzVolume))
        {
            return &g_DrvKext.DrvCtx.Except.DriveEx[nPos];
        }
    }
    return NULL;
}


char*
sms_strupr(char* pczStr)
{
    size_t i=0, len=0;
    
    if(!pczStr) return NULL;
    
    len = strlen( pczStr );
    for(i=0; i<len; i++)
    {
        if(pczStr[i] >= 97 && pczStr[i] <= 122)
        {
            pczStr[i] -= 32;
        }
    }
    return pczStr;
}

char*
sms_strstr2(const char* pczStr, const char* pczToken)
{
    int nPos = 0, nLength=0;  //s1과 s2를 비교할 때 서로 같은 문자열의 개수를 저장할 변수
    
    if(!pczStr || !pczToken)
    {
        return NULL;
    }
    
    for(; *pczStr; pczStr++)
    {
        if(*pczStr == *pczToken)
        {
            for(nPos=1; *(pczStr + nPos) == *(pczToken + nPos); nPos++)
            {
                ;
            }
            
             // printf("[DLP][%s] ret=NULL \n", __FUNCTION__ );
            nLength = (int)strlen(pczToken);
            if(nPos == nLength)
            {
                return (char *)pczStr;
            }
        }
    }
    return NULL;//일치하는 것이 없으면 NULL 반환
}

char*
sms_strstr(char* pczStr, char* pczToken)
{
    char* pczPos = NULL;
    char* pczFind = NULL;
    
    pczFind = pczToken;
    if(!pczStr || !pczFind)
    {
        return NULL;
    }
    
    while(*pczStr != '\0')
    {
        if(pczPos != NULL && *pczStr != *pczFind)
        {
            pczPos = NULL;
            pczFind = pczToken;
        }
        
        if(pczPos == NULL && *pczStr == *pczFind)
        {
            pczPos = pczStr;
        }
        
        if(pczPos != NULL && *pczStr == *pczFind)
        {
            pczFind++;
        }
        
        if(pczPos != NULL && *pczFind == '\0')
        {
            // printf("[DLP][%s] ret=%s \n", __FUNCTION__, pczPos );
            return pczPos;
        }
        pczStr++;
    }
    
    return NULL;
}

char*
sms_strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len=0;
    
    if((c = *find++) != '\0')
    {
        len = strlen(find);
        do
        {
            do
            {
                if((sc = *s++) == '\0' || slen-- < 1)
                {
                    return (NULL);
                }
            } while (sc != c);
            
            if(len > slen)
            {
                return (NULL);
            }
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

char*
sms_strrchr(char* pczStr, int ch)
{
    char* pczToken = NULL;
    
    for(pczToken=NULL;; ++pczStr)
    {
        if(*pczStr == ch)
        {
            pczToken = (char*)pczStr;
        }

        if(*pczStr == '\0')
        {
            return pczToken;
        }
    }
    
    return NULL;
    
}

ULONG
GetBusTypeFromVolume( CHAR* pczVolume )
{
    ULONG  ulBusType = 0;

    if(!pczVolume) return 0;

printf("FIXME!!!!!!!!!!!!!!!!!-%s\n", __FUNCTION__);

    if( sms_strnstr(pczVolume, VOLUME_VIPUSB, strlen(VOLUME_VIPUSB)) || sms_strnstr(pczVolume, VOLUME_NICSUSB, strlen(VOLUME_NICSUSB)) )
    {
        ulBusType = BusTypeUsb;
    }
    return ulBusType;
}

uint64_t
GetFileSize( const char *pPath )
{
    uint64_t _vnode_size = 0;

    if ( NULL == pPath || pPath[0] == 0)
        return _vnode_size;
    
    struct stat stat_buf = {0};
    if (0 == stat(pPath, &stat_buf))
    {
        _vnode_size = stat_buf.st_size;
    }
    
    return _vnode_size;
}

BOOLEAN
IsAllowedProcessId(int nProcID, PCHAR pczProcName)
{
    BOOLEAN bReturn = FALSE;

    if(!pczProcName) return FALSE;

    if(0 == strncasecmp( pczProcName, "PISupervisor", strlen("PISupervisor")) )
    {
        return TRUE;
    }

    if(nProcID == g_DrvKext.AgentCtx.nAgentPID )
    {
        bReturn = TRUE;
    }
    else if( 0 == strncasecmp( pczProcName, "PIAgent.exe", strlen("PIAgent.exe")) )
    {
        LOG_MSG("[DLP][%s] except-procname & hard-coded allow_pname:(%s) \n", __FUNCTION__, pczProcName);
        bReturn = TRUE;
    }
    else
    {
        size_t nLength=0, nLength2=0;
        int nIndex=0, nCount = 0;	//g_nAllowedProcessId;
        nCount = g_DrvKext.nAllowProcName;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            nLength  = strlen(g_DrvKext.AllowProcName[ nIndex ].czAllowProcName);
            nLength2 = strlen( pczProcName );
            if(nLength == nLength2 && strncasecmp( g_DrvKext.AllowProcName[ nIndex ].czAllowProcName, pczProcName, nLength ) == 0)
            {
                LOG_MSG("[DLP][%s] Matched current pname & allow_pname:(%s) \n", __FUNCTION__, pczProcName);
                bReturn = TRUE;
            }
        }
    }

    // SelfProtect Process Allow
    if(FALSE == bReturn && TRUE == g_DrvKext.SelfCtx.bProtect)
    {
        int nPos=0, nMaxPos= 0;
        nMaxPos = g_DrvKext.SelfCtx.nService;
        for(nPos=0; nPos<nMaxPos; nPos++)
        {
            if(nProcID == g_DrvKext.SelfCtx.Services[nPos].nPID)
            {
                bReturn = TRUE;
                break;
            }
        }
    }

    if(FALSE == bReturn)
    {
        bReturn = IsUnProtectId( nProcID );
    }
    return bReturn;
}

BOOLEAN
IsUnProtectId( ULONG nPID )
{
    ULONG nPos=0;
    for(nPos=0; nPos<g_DrvKext.nAllowProcID; nPos++)
    {
        if( g_DrvKext.AllowProcID[nPos].nAllowedProcID == (ULONG)nPID )
        {
            LOG_MSG("[DLP][%s] Count=%d ProcID=%d \n", __FUNCTION__, g_DrvKext.nAllowProcID, g_DrvKext.AllowProcID[nPos].nAllowedProcID );
            return TRUE;
        }
    }
    return FALSE;
}


BOOLEAN
IsFileGetExtName(char* pczFilePath, char* pczExt)
{
    int i=0, j=0, nLength=0;

    if(!pczFilePath) return FALSE;

    nLength = (int)strlen(pczFilePath);
    nLength -= 5;

    for(i=5; i>2; i--)
    {
        if( *(pczFilePath + nLength) == '.' )
        {
            for(j=0; j<i; j++)
            {
                pczExt[j] = *(pczFilePath + nLength + j);
            }
            return TRUE;
        }
        nLength++;
    }
    return FALSE;
}

BOOLEAN
IsAllowedFolderFileExt(char* pczPath, BOOLEAN bOpen)
{
    int     nPos=0, nIndex=0, nCount=0, nLength=0, nExtCount=0;
    char    czFileExt[ 16 ];
    char    czFilePath[MAX_FILE_LENGTH+1];

    if(!pczPath)
    {
        LOG_MSG("\n\n\n[DLP][%s] pczPath == null \n\n\n", __FUNCTION__ );
        return FALSE;
    }

    memset( czFileExt,  0, sizeof(czFileExt)  );
    memset( czFilePath, 0, sizeof(czFilePath) );
    nLength = min(MAX_FILE_LENGTH, (int)strlen(pczPath) );
    strncpy( czFilePath, pczPath, nLength );
    sms_strupr( czFilePath );

    nCount = g_DrvKext.nAllowFolder;
    for(nPos=0; nPos<nCount; nPos++)
    {
        if(NULL == sms_strstr( czFilePath, g_DrvKext.AllowFolder[nPos].czAllowFolder ))
        {
            continue;
        }

        memset( czFileExt,  0, sizeof(czFileExt)  );
        if(IsFileGetExtName( czFilePath, czFileExt) )
        {
            nExtCount = g_DrvKext.nAllowFileExt;
            for(nIndex=0; nIndex<nExtCount; nIndex++)
            {
                nLength = (int)strlen( g_DrvKext.AllowFileExt[nIndex].czAllowFileExt );
                if(sms_strnstr( sms_strupr( &czFileExt[1] ), g_DrvKext.AllowFileExt[nIndex].czAllowFileExt, nLength ))
                {
                    LOG_MSG( "[DLP][%s][%d] FileExt=%s AllowFileExt=%s return true. \n", __FUNCTION__,
                             nIndex, czFileExt, g_DrvKext.AllowFileExt[nIndex].czAllowFileExt );
                    return TRUE;
                }
            }
        }
        else
        {
            if(bOpen)
            {
                LOG_MSG("[DLP][%s] return=TRUE \n", __FUNCTION__ );
                return TRUE;
            }
            else
            {
                LOG_MSG("[DLP][%s] return=FALSE \n", __FUNCTION__ );
                return FALSE;
            }
        }
    }
    return FALSE;
}



long
GetCurrentlongTime()
{
    struct timeval t = {0};
    gettimeofday(&t, 0);
    return t.tv_sec * INT64_C(1000) + t.tv_usec / 1000;
}

void GetCurrentTime( char* pczCurTime, int nTimeBufSize )
{
    if(!pczCurTime) return;
}

bool
IsValidLogCheckTimestamp( uint64_t nLogCheckTimeStamp )
{
    uint64_t nDiff = 0, nCurStamp = 0;
    double nAbs2Clock = 0, nTimeNano = 0, nTimeSec = 0;

/*    
    if(nLogCheckTimeStamp <= 0 || false == g_Check.bInit || g_Check.TimeBase.denom <= 0)
    {
        return false;
    }

    nCurStamp = mach_absolute_time();
    nDiff = nCurStamp - nLogCheckTimeStamp;

    nAbs2Clock = g_Check.TimeBase.numer/(double)g_Check.TimeBase.denom;
    nTimeNano = (double)nAbs2Clock * nDiff;
    nTimeSec = (double)nTimeNano/(double)1000000000;

    // printf( "[DLP][%s] Nano=%f, nTimeSec=%f \n", __FUNCTION__, nTimeNano, nTimeSec  );
    if(nTimeSec >= 1)
    {
        return false;
    }
*/
    
    return true;
}



LOG_CHECK*
LogCheckSearchParent( LOG_PARAM* pParam )
{
    int nPos = 0;
    if(!pParam || !pParam->pczPath1)
    {
        return NULL;
    }

    for(nPos=0; nPos<MAX_CHECK; nPos++)
    {
        if(g_Check.Data[nPos].nTimeStamp <= 0) continue;
        if(sms_strstr( pParam->pczPath1, g_Check.Data[nPos].czFilePath ) && g_Check.Data[nPos].nPolicyType == pParam->nPolicyType)
        {
            if(true == IsValidLogCheckTimestamp( g_Check.Data[nPos].nTimeStamp ))
            {
                // printf("[DLP][%s] return true. Pos=%d, PolicyType=%02x, FilePath=%s \n", __FUNCTION__, nPos, pParam->nPolicyType, pParam->pczPath1 );
                return &g_Check.Data[nPos];
            }
        }
    }
    return NULL;
}


boolean_t IsExistParentLogCheck( LOG_PARAM* pParam )
{
    if(LogCheckSearchParent( pParam ))
    {
        return true;
    }
    return false;
}


LOG_CHECK*
LogCheckSearch( LOG_PARAM* pParam )
{
    int nPos = 0;
    if(!pParam || !pParam->pczPath1 || VDIR != pParam->nVnodeType)
    {
        return NULL;
    }

    for(nPos=0; nPos<MAX_CHECK; nPos++)
    {
        if(g_Check.Data[nPos].nTimeStamp <= 0) continue;
        if(0 == strncasecmp( g_Check.Data[nPos].czFilePath, pParam->pczPath1, strlen(pParam->pczPath1) ) && g_Check.Data[nPos].nPolicyType == pParam->nPolicyType)
        {
            if(true == IsValidLogCheckTimestamp( g_Check.Data[nPos].nTimeStamp ))
            {
                // printf("[DLP][%s] return true. Pos=%d, PolicyType=%02x, FilePath=%s \n", __FUNCTION__, nPos, pParam->nPolicyType, pParam->pczPath1 );
                return &g_Check.Data[nPos];
            }
        }
    }
    return NULL;
}


bool
LogCheckInsert( LOG_PARAM* pParam )
{
    int nPos = 0;
    if(!pParam || !pParam->pczPath1 || VDIR != pParam->nVnodeType)
    {
        return false;
    }

    for(nPos=0; nPos<MAX_CHECK; nPos++)
    {
        if(g_Check.Data[nPos].nTimeStamp <= 0 || false == IsValidLogCheckTimestamp( g_Check.Data[nPos].nTimeStamp ))
        {
            memset( &g_Check.Data[nPos], 0, sizeof(g_Check.Data[nPos]) );
            memcpy( g_Check.Data[nPos].czFilePath, pParam->pczPath1, strlen(pParam->pczPath1)+1 );
            g_Check.Data[nPos].nPID = pParam->nProcessId;
            g_Check.Data[nPos].nLogType = pParam->nLogType;
            g_Check.Data[nPos].nAction  = pParam->nAction;
            g_Check.Data[nPos].nPolicyType = pParam->nPolicyType;
/*
            g_Check.Data[nPos].nTimeStamp = mach_absolute_time();
*/
            printf("[DLP][%s] Success. Pos=%d, PolicyType=%02x, FilePath=%s \n", __FUNCTION__, nPos, pParam->nPolicyType, pParam->pczPath1 );
            return true;
        }
    }
    printf("[DLP][%s] Failed. PolicyType=%02x, FilePath=%s \n", __FUNCTION__, pParam->nPolicyType, pParam->pczPath1 );
    return false;
}


bool
IsDirDuplicateLog( LOG_PARAM* pParam )
{
    LOG_CHECK* pLogCheck = NULL;

    if(!pParam || !pParam->pczPath1 || VDIR != pParam->nVnodeType)
    {
        return false;
    }

/*
    if(false == g_Check.bInit)
    {
        clock_timebase_info( &g_Check.TimeBase );
        if(g_Check.TimeBase.denom <= 0)
        {
            printf("[DLP][%s] Failed == mach_timebase_Info() \n", __FUNCTION__ );
            return false;
        }

        printf("[DLP][%s] Success == mach_timebase_Info(), TimeBase.number=%u, TimeBase.denom=%u \n", __FUNCTION__, g_Check.TimeBase.numer, g_Check.TimeBase.denom );
        g_Check.bInit = true;
    }
*/
    
    pLogCheck = LogCheckSearch( pParam );
    if(pLogCheck)
    {
        return true;
    }
    LogCheckInsert( pParam );
    return false;
}


boolean_t IsLogByPassProcess( LOG_PARAM* pLogParam )
{
    if(!pLogParam || !pLogParam->pczProcName) return false;
    if(0 == strncasecmp(pLogParam->pczProcName, "mds", strlen(pLogParam->pczProcName)) ||
       0 == strncasecmp(pLogParam->pczProcName, "mdworker_shared", strlen(pLogParam->pczProcName)) )
    {
        return true;
    }
    return false;
}


boolean_t
SmartDrv_LogAppend( LOG_PARAM* pLogParam, int nVnodeType )
{
    ssize_t   nLogSize=0, nLogSizeEx=0;
    PSMART_LOG_RECORD_EX pLogExNew = NULL;

    if(!pLogParam) return FALSE;

    if(true == IsLogByPassProcess( pLogParam ))
    {        
        return false;
    }

    if(true == IsExistParentLogCheck( pLogParam ))
    {
        return false;
    }

    pLogParam->nVnodeType = nVnodeType;
    if(VDIR == nVnodeType && true == IsDirDuplicateLog(pLogParam))
    {
        return false;
    }

    nLogSize = sizeof(SMART_LOG_RECORD);
    nLogSizeEx = sizeof(SMART_LOG_RECORD_EX);
    // pLogExNew = (PSMART_LOG_RECORD_EX)_MALLOC( nLogSizeEx, 1, M_ZERO );
    pLogExNew = (PSMART_LOG_RECORD_EX)MyMAlloc( nLogSizeEx );
    if(!pLogExNew) return false;

    memset( pLogExNew, 0, nLogSizeEx );
    pLogExNew->Size           = (ULONG)nLogSizeEx;
    pLogExNew->PolicyType     = (ULONG)pLogParam->nPolicyType;
    // LogRecord
    pLogExNew->Log.Size       = (ULONG)nLogSize;
    pLogExNew->Log.AccessType = accessUnknown;
    pLogExNew->Log.RecordType = RECORD_TYPE_NORMAL;
    if(pLogParam->pczProcName)
    {
        memcpy( pLogExNew->Log.Data.ProcName, pLogParam->pczProcName, strlen(pLogParam->pczProcName)+1 );
    }

    if(pLogParam->pczPath1)
    {
        memcpy( pLogExNew->Log.Data.FileName, pLogParam->pczPath1, strlen(pLogParam->pczPath1)+1 );
    }

    // Data
    pLogExNew->Log.Data.LogType   = pLogParam->nLogType;
    pLogExNew->Log.Data.LogAction = pLogParam->nAction;
    pLogExNew->Log.Data.pVnode    = pLogParam->pVnode;
    pLogExNew->Log.Data.ProcessId = pLogParam->nProcessId;
    pLogExNew->Log.Data.Flags     = 0;
    pLogExNew->Log.Data.Status    = 0;
    pLogExNew->Log.Data.bDisableResult = !pLogParam->bAllow;

    // time
    GetCurrentTime( pLogExNew->Log.Data.LogTime, sizeof(pLogExNew->Log.Data.LogTime) );
    switch(pLogParam->nAction)
    {
        case ACTION_OPEN:
        case ACTION_READ:
        case ACTION_EXECUTE:
            pLogExNew->Log.AccessType = accessRead;
            break;
        case ACTION_APPEND:
        case ACTION_WRITE:
        case ACTION_CLOSE:
        case ACTION_EXCHANGE:
            pLogExNew->Log.AccessType = accessWrite;
            break;
        case ACTION_CONNECT:
            pLogExNew->Log.AccessType = accessRead;
            break;
        case ACTION_UNLINK:
        case ACTION_KILL:
        default:
            pLogExNew->Log.AccessType = accessUnknown;
            break;
    }

    if(!UserModeTo_SmartLogNotify( pLogExNew, (ULONG)nLogSizeEx ))
    {
        LOG_MSG("[DLP][%s] UserModeTo_SmartLogNotify failed. \n", __FUNCTION__);
        if(pLogExNew) _FREE( pLogExNew, 1 );
        return FALSE;
    }

    printf( "[DLP][%s] End. nPID=%d, proc=%s, Action=%08x, PolicyType=%02x, FilePath=%s \n",
            __FUNCTION__, pLogParam->nProcessId, pLogParam->pczProcName, pLogParam->nAction, pLogParam->nPolicyType, pLogParam->pczPath1 );

    if(pLogExNew) _FREE( pLogExNew, 1 );
    return TRUE;
}
