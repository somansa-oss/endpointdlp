#include <stdio.h>

#include <libproc.h>

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"

#include "KauthEventFunc.h"
#include "SmartCmd.h"
#include "PISecSmartDrv.h"
#include "kauth_listener.h"
#include "KernelCommand.h"

#ifdef __cplusplus
extern "C"
{
#endif

void* MyMAlloc(size_t nAllocSize);

#ifdef __cplusplus
}
#endif

kern_return_t
UserModeTo_SendCommand( void* pData, size_t nLength )
{
    return SendEventDataToPISupervisor( pData, nLength );
}

boolean_t
UserModeTo_SmartLogNotify( void* pLogBuf, ULONG nLogBufSize )
{
    PCOMMAND_MESSAGE     pCmdNew = NULL;
    PSMART_LOG_RECORD_EX pLogEx  = NULL;
    size_t               nTotalSize = 0;

    nTotalSize = sizeof(COMMAND_MESSAGE) + nLogBufSize;
    // pCmdNew = (PCOMMAND_MESSAGE)_MALLOC( nTotalSize, 1, M_ZERO );
    pCmdNew = (PCOMMAND_MESSAGE)MyMAlloc( nTotalSize );
    if(!pCmdNew)
    {
        LOG_MSG("[DLP][%s] Memory Allocate Failed. \n", __FUNCTION__ );
        return FALSE;
    }

    memset( pCmdNew, 0, nTotalSize );
    pCmdNew->Size    = (ULONG)nTotalSize;
    pCmdNew->Command = (ULONG)SmartLogNotify;
    pLogEx = (PSMART_LOG_RECORD_EX)pCmdNew->Data;
    if(pLogEx)
    {
        memcpy( pLogEx, pLogBuf, nLogBufSize );
    }

    int response = 0;
    if(UserModeTo_SendCommand( pCmdNew, nTotalSize ) != KERN_SUCCESS)
    {
        LOG_MSG("[DLP][%s] UserModeTo_SendCommand() Failed. \n", __FUNCTION__ );
        if(pCmdNew) _FREE( pCmdNew, 1 );
        return FALSE;
    }

    if(pCmdNew) _FREE( pCmdNew, 1 );
    return TRUE;
}

boolean_t
QtFetchValidPath(char* pczPath, char* pczOutQtPath, size_t nMaxQtPath, int nPolicyType)
{
    char*         pczToken = NULL;
    long          nCurTime = 0;

    if(!pczPath || !pczOutQtPath)
    {
        return FALSE;
    }
    // Source
    pczToken = sms_strrchr( pczPath, '/' );
    if(!pczToken || strlen(pczToken) <= 1)
    {
        return FALSE;
    }

    nCurTime = GetCurrentlongTime();
    if(MEDIA_PRINTPREVENT == nPolicyType)
        snprintf( pczOutQtPath, nMaxQtPath, "%s/Print/%012ld_%s", g_DrvKext.QtCtx.Entry[0].czBuffer, nCurTime, pczToken+1 );
    else
        snprintf( pczOutQtPath, nMaxQtPath, "%s/%012ld_%s", g_DrvKext.QtCtx.Entry[0].czBuffer, nCurTime, pczToken+1 );

    return TRUE;
}

// 에이전트에 내용 기반 검색 요청
boolean_t
UserModeTo_FileScan( int nPID, vnode_t pVnode, char* pczFilePath, int64_t nFileSize, int nPolicyType )
{
    PCOMMAND_MESSAGE      pCmdNew = NULL;
    PSCANNER_NOTIFICATION pNotify = NULL;
    boolean_t             bAccess = FALSE, bSuc = FALSE;
    size_t                nTotalSize = 0, nDataSize=0;
    int                   nBusType = 0;

    if(!pczFilePath)
    {
        LOG_MSG("[DLP][%s] FilePath is null. \n", __FUNCTION__ );
        return FALSE;
    }

    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;

    // LOG_MSG("[DLP][%s] FilePath=%s \n", __FUNCTION__, pczFilePath );
    // pCmdNew = (PCOMMAND_MESSAGE)_MALLOC( nTotalSize, 1, M_ZERO );
    pCmdNew = (PCOMMAND_MESSAGE)MyMAlloc( nTotalSize );
    if(!pCmdNew)
    {
        LOG_MSG("[DLP][%s] Memory Allocate Failed. FilePath=%s \n", __FUNCTION__, pczFilePath );
        return FALSE;
    }

    // BusType 을 에이전트 한테 전달하기 위하여 추가 함. 2016.10.24
    nBusType = VolCtx_Search_BusType( pczFilePath );
    pCmdNew->Size = (ULONG)nTotalSize;
    pCmdNew->Command  = (ULONG)FileScan;
    pNotify = (PSCANNER_NOTIFICATION)pCmdNew->Data;
    if(pNotify)
    {
        pNotify->nSize   = (ULONG)nDataSize;
        pNotify->pWakeup = (void*)&bAccess;
        pNotify->pParam  = (void*)nFileSize;
        pNotify->pVnode  = pVnode;
        pNotify->nPID    = nPID;
        //

        if(nPolicyType == g_DrvKext.PrintCtx.Policy.nPolicyType)
            pNotify->nAction = ActionTypePrint; // PrintType Set
        else if(nPolicyType == g_DrvKext.UploadCtx.Policy.nPolicyType)
            pNotify->nAction = ActionTypeUpload; // PrintType Set
        else
            pNotify->nAction = nBusType; // BusType Set
        //
        strncpy( (char*)pNotify->czFilePath, pczFilePath, strlen(pczFilePath)+1 );
        // QtFilePath
        bSuc = QtFetchValidPath( (char*)pNotify->czFilePath, pNotify->czQtFilePath, MAX_FILE_LENGTH, nPolicyType );
        //
        // if(bSuc == TRUE) QtCopyFileKern( (char*)pNotify->czFilePath, pNotify->czQtFilePath, nFileSize );
        //
        LOG_MSG("[DLP][%s] BusType=%02x, FilePath=%s, QtFilePath=%s \n", __FUNCTION__, nBusType, pczFilePath, pNotify->czQtFilePath );
    }

    int response = 0;
    if(UserModeTo_SendCommand( pCmdNew, nTotalSize ) != KERN_SUCCESS)
    {
        LOG_MSG("[DLP][%s] UserModeTo_SendCommand() Failed. \n", __FUNCTION__ );
        if(pCmdNew) _FREE( pCmdNew, 1 );
        return FALSE;
    }

    if (pNotify->nResult == RESULT_DENY)
    {
        bAccess = FALSE;
    }
    else
    {
        bAccess = TRUE;
    }
    
    LOG_MSG("[DLP][%s] bAccess=%d, FilePath=%s \n", __FUNCTION__, bAccess, pczFilePath );
    if(pCmdNew) _FREE( pCmdNew, 1 );
    return bAccess;
}


// 에이전트에 파일 삭제 요청
boolean_t
UserModeTo_FileDelete(char* pczFilePath)
{
    PCOMMAND_MESSAGE      pCmdNew = NULL;
    PSCANNER_NOTIFICATION pNotify = NULL;
    boolean_t             bDelete = FALSE;
    size_t                nTotalSize = 0, nDataSize=0;

    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;

    LOG_MSG("[DLP][%s] FilePath=%s \n", __FUNCTION__, pczFilePath );
    // pCmdNew = (PCOMMAND_MESSAGE)_MALLOC( nTotalSize, 1, M_ZERO );
    pCmdNew = (PCOMMAND_MESSAGE)MyMAlloc( nTotalSize );
    if(!pCmdNew)
    {
        LOG_MSG("[DLP]-[%s] Memory Allocate Failed. FilePath=%s \n", __FUNCTION__, pczFilePath );
        return FALSE;
    }

    pCmdNew->Size = (ULONG)nTotalSize;
    pCmdNew->Command  = (ULONG)FileDelete;
    pNotify = (PSCANNER_NOTIFICATION)pCmdNew->Data;
    if(pNotify)
    {
        pNotify->nSize    = (ULONG)nDataSize;
        pNotify->pWakeup  = (void*)&bDelete;
        strncpy( (char*)pNotify->czFilePath, pczFilePath, strlen(pczFilePath) );
    }

    if(UserModeTo_SendCommand( pCmdNew, nTotalSize) != KERN_SUCCESS)
    {
        LOG_MSG("[DLP][%s] UserModeTo_SendCommand() Failed. FilePath=%s \n", __FUNCTION__, pczFilePath );
        _FREE( pCmdNew, 1 );
        return FALSE;
    }

    if (pNotify->nResult == RESULT_DENY)
    {
        bDelete = FALSE;
    }
    else
    {
        bDelete = TRUE;
    }
    
    _FREE( pCmdNew, 1 );
    return bDelete;
}

boolean_t
CtrlCheck_Contents(int nPID, char* pczProcName, vnode_t pVnode, const char* pczPath, LOG_PARAM* pLogParam, DLP_POLICY_TYPE_ENUM emPolicyType)
{
    uint64_t   nFileSize = 0;
    boolean_t  bAccess = FALSE;

    //if(!pVnode || !pczPath) return FALSE;
    if(!pczPath) return FALSE;

    // 파일 크기를 알아냄.
    //nFileSize = GetFileSize( pVnode );
    nFileSize = GetFileSize( pczPath );

    if((IsPolicyExist_BlockWrite( nPID, pczProcName, (int)VREG, pczPath, pLogParam ) && emPolicyType == POLICY_COPY)
       || (IsPolicyExist_Print_BlockWrite( nPID, pczProcName, pczPath, pLogParam ) && emPolicyType == POLICY_PRINT))
    {
        LOG_MSG("[DLP][%s] KAUTH_FILEOP_CLOSE, dirty, %s\n", __FUNCTION__, pczPath );

        if(FALSE == UserModeTo_FileDelete((char*)pczPath))
        {   // 파일을 삭제하도록 에이전트에 요청함.
            LOG_MSG("[DLP][%s] KAUTH_FILEOP_CLOSE, DeleteFile() failed, %s\n", __FUNCTION__, pczPath );
        }
    }
    else if( IsPolicyExist_AllowWrite( nPID, pczProcName, VREG, pczPath, pLogParam ) && emPolicyType == POLICY_COPY)
    {
        LOG_MSG("[DLP][%s] IsPolicyExist_AllowWrite, %s\n", __FUNCTION__, pczPath );
    }
    else if((IsPolicyExist_CopyPrevent( nPID, pczProcName, pczPath, pLogParam) && emPolicyType == POLICY_COPY) ||
            (IsPolicyExist_PrintPrevent( nPID, pczProcName, pczPath, pLogParam ) && emPolicyType == POLICY_PRINT) ||
            (IsPolicyExist_UploadPrevent( nPID, pczProcName, pczPath, pLogParam ) && emPolicyType == POLICY_UPLOAD))
    {
        // IsCupsDirectory( pczPath )
        // 3번 정책인 경우, 내용 기반 검색 실시. // 내용 기반 검색이 목적이므로 파일 크기가 0보다 큰 경우만 다룸.

        printf( "[DLP][%s] !!!! UserModeTo_FileScan --> pid=%d, proc=%s, FileSize=%d, FilePath=%s, nPolicyType=%d, UploadCtxPolicy.nPolicyType=%d, nPID=%d\n", __FUNCTION__,
               g_DrvKext.PrintCtx.Node.nPID, g_DrvKext.PrintCtx.Node.czProcName, (int)nFileSize, pczPath, pLogParam->nPolicyType, g_DrvKext.UploadCtx.Policy.nPolicyType, nPID );

        if(nFileSize > 0)
        {
            // g_DrvKext.PrintCtx.Node.nPID;
            // g_DrvKext.PrintCtx.Node.czProcName;
            printf( "[DLP][%s] UserModeTo_FileScan --> pid=%d, proc=%s, FileSize=%d, FilePath=%s \n", __FUNCTION__,
                    g_DrvKext.PrintCtx.Node.nPID, g_DrvKext.PrintCtx.Node.czProcName, (int)nFileSize, pczPath );

            bAccess = UserModeTo_FileScan( nPID, pVnode, (char*)pczPath, nFileSize, pLogParam->nPolicyType );
            if(emPolicyType != POLICY_UPLOAD && !bAccess)
            {
                UserModeTo_FileDelete( (char*)pczPath );
            }
        }
    }


    return bAccess;
}

boolean_t
IsPolicyExist_BlockWrite( int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bSafe = TRUE;
    boolean_t bLog  = FALSE;
    int   nPos=0, nMaxPos =0, nLength=0;
    char* pczTargetBase = NULL;

    if(!pLogParam || !pczFilePath)
    {
        LOG_MSG("[DLP][%s] INVALID PARAMETER. \n", __FUNCTION__);
        return FALSE;
    }

    nMaxPos = (int)g_DrvKext.nDeviceCount;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        nLength = (int)strlen(g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames);
        pczTargetBase = sms_strnstr( pczFilePath, g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames, nLength );
        if(pczTargetBase != NULL && (pczFilePath[nLength] == '/' || pczFilePath[nLength] == 0))
        {
            if(g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableAll)
            {
                bSafe = FALSE;
                bLog = (boolean_t)g_DrvKext.CtrlDeviceEx[nPos].cDevice.bLoggingOn;
                pLogParam->bLog = bLog;
                pLogParam->nPolicyType = g_DrvKext.CtrlDeviceEx[nPos].ulStorDevType;
            }
            else if(g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableWrite && g_DrvKext.CtrlDeviceEx[nPos].cDevice.nReserved1 == 1)
            {
                bSafe = FALSE;
                bLog = (boolean_t)g_DrvKext.CtrlDeviceEx[nPos].cDevice.bLoggingOn;
                pLogParam->bLog = bLog;
                pLogParam->nPolicyType = g_DrvKext.CtrlDeviceEx[nPos].ulStorDevType;
            }
            break;
        }
    }

    if(FALSE == bSafe && IsAllowedFolderFileExt((char*)pczFilePath, TRUE) == FALSE)
    {   // if(bLog && (VDIR != nVnodeType))
        if(bLog)
        {
            SmartDrv_LogAppend( pLogParam, nVnodeType );
        }
        // LOG_MSG("[DLP][%s] ExistPolicy \n", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}

boolean_t
IsPolicyExist_AllowWrite( int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bAllowPolicy = FALSE;
    boolean_t bLog  = FALSE;
    int   nPos=0, nMaxPos =0, nLength=0;
    char* pczTargetBase = NULL;

    if(!pLogParam || !pczFilePath)
    {
        LOG_MSG("[DLP][%s] INVALID PARAMETER. \n", __FUNCTION__);
        return FALSE;
    }

    nMaxPos = (int)g_DrvKext.nDeviceCount;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        nLength = (int)strlen(g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames);
        pczTargetBase = sms_strnstr( pczFilePath, g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames, nLength );

        if(pczTargetBase != NULL && (pczFilePath[nLength] == '/' || pczFilePath[nLength] == 0))
        {
            if( FALSE == g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableRead && FALSE == g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableWrite )
            {
                bAllowPolicy = TRUE;
                bLog = (boolean_t)g_DrvKext.CtrlDeviceEx[nPos].cDevice.bLoggingOn;
                pLogParam->bLog = bLog;
                pLogParam->nPolicyType = g_DrvKext.CtrlDeviceEx[nPos].ulStorDevType;
            }
            break;
        }
    }

    if(bAllowPolicy)
    {   // if(bLog && (VDIR != nVnodeType))
        if(bLog)
        {
            pLogParam->bAllow = TRUE;
            SmartDrv_LogAppend( pLogParam, nVnodeType );
        }
        // LOG_MSG("[DLP][%s] ExistPolicy \n", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}

boolean_t
IsPolicyExist_BlockAll(int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bSafe = TRUE;
    boolean_t bLog = FALSE;
    int   nPos=0, nMaxPos=0, nLength=0;
    char* pczTargetBase = NULL;

    if(!pLogParam || !pczFilePath)
    {
        LOG_MSG("[DLP][%s] INVALID PARAMTER. \n", __FUNCTION__);
        return FALSE;
    }

    nMaxPos = (int)g_DrvKext.nDeviceCount;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        nLength = (int)strlen(g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames);
        pczTargetBase = sms_strnstr( pczFilePath, g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames, nLength );
        if(pczTargetBase != NULL && (pczFilePath[nLength] == '/' || pczFilePath[nLength] == 0))
        {
            if(g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableAll || g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableRead)
            {
                bSafe = FALSE;
                bLog  = (boolean_t)g_DrvKext.CtrlDeviceEx[nPos].cDevice.bLoggingOn;
                pLogParam->bLog = bLog;
                pLogParam->nPolicyType = g_DrvKext.CtrlDeviceEx[nPos].ulStorDevType;

            }
            else if(g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableWrite && g_DrvKext.CtrlDeviceEx[nPos].cDevice.nReserved1 == 1)
            {
                bSafe = FALSE;
                bLog  = (boolean_t)g_DrvKext.CtrlDeviceEx[nPos].cDevice.bLoggingOn;
                pLogParam->bLog = bLog;
                pLogParam->nPolicyType = g_DrvKext.CtrlDeviceEx[nPos].ulStorDevType;
            }
            break;
        }
    }

    if(FALSE == bSafe && IsAllowedFolderFileExt( (char*)pczFilePath, TRUE) == FALSE)
    {   // if(bLog && (VDIR != nVnodeType))
        if(bLog)
        {
            SmartDrv_LogAppend( pLogParam, nVnodeType );
        }
        LOG_MSG("[DLP][%s] ApplyPolicy \n", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}


boolean_t
IsPolicyExist_CopyPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bSafe = TRUE;
    boolean_t bLog  = FALSE;
    int   nPos=0, nMaxPos=0, nLength=0;
    char* pczTargetBase = NULL;

    if(!pLogParam || !pczFilePath)
    {
        return FALSE;
    }

    nMaxPos = (int)g_DrvKext.nDeviceCount;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        nLength = (int)strlen(g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames);
        pczTargetBase = sms_strnstr( pczFilePath, g_DrvKext.CtrlDeviceEx[nPos].cDevice.DeviceNames, nLength );
        if(pczTargetBase != NULL && (pczFilePath[nLength] == '/' || pczFilePath[nLength] == 0))
        {
            if(g_DrvKext.CtrlDeviceEx[nPos].cDevice.bDisableWrite && g_DrvKext.CtrlDeviceEx[nPos].cDevice.nReserved1 == -1)
            {
                bSafe = FALSE;
                bLog = (boolean_t)g_DrvKext.CtrlDeviceEx[nPos].cDevice.bLoggingOn;
                pLogParam->bLog = bLog;
                pLogParam->nPolicyType = g_DrvKext.CtrlDeviceEx[nPos].ulStorDevType;
            }
            break;
        }
    }

    if(FALSE == bSafe && IsAllowedFolderFileExt( (char*)pczFilePath, TRUE) == FALSE)
    {
        if(bLog) SmartDrv_LogAppend( pLogParam, VREG );
        LOG_MSG("[DLP][%s] ExistPolicy \n", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}


boolean_t
IsPolicyExist_Print_BlockWrite(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bLog = FALSE;

    if(!pczProcName || !pczFilePath || !pLogParam)
    {
        LOG_MSG("[DLP][%s] INVALID PARAMETER. \n", __FUNCTION__);
        return FALSE;
    }

    // Print Prevent Poliy Not Exist
    if(FALSE == g_DrvKext.PrintCtx.Policy.bPolicy)
    {
        LOG_MSG("[DLP][%s] NOT EXIST PRINTPREVENT. \n", __FUNCTION__);
        return FALSE;
    }

    if(g_DrvKext.PrintCtx.Policy.bDisableWrite && g_DrvKext.PrintCtx.Policy.nReserved1 == 1)
    {
        bLog = (boolean_t)g_DrvKext.PrintCtx.Policy.bLoggingOn;
        pLogParam->bLog = bLog;
        pLogParam->nPolicyType = g_DrvKext.PrintCtx.Policy.nPolicyType;
        if(bLog)
        {
            SmartDrv_LogAppend( pLogParam, VREG );
        }
        LOG_MSG( "[DLP][%s] pid=%d, proc=%s, FilePath=%s \n", __FUNCTION__, nPID, pczProcName, pczFilePath );
        return TRUE;
    }
    return FALSE;
}

boolean_t
IsPolicyExist_PrintPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bLog = FALSE;

    if(!pczProcName || !pczFilePath || !pLogParam)
    {
        LOG_MSG("[DLP][%s] INVALID PARAMETER \n", __FUNCTION__);
        return FALSE;
    }

    // Print Prevent Poliy Not Exist
    if(FALSE == g_DrvKext.PrintCtx.Policy.bPolicy)
    {
        LOG_MSG("[DLP][%s] NOT PRINT PREVENT. \n", __FUNCTION__);
        return FALSE;
    }

    if(g_DrvKext.PrintCtx.Policy.bDisableWrite && g_DrvKext.PrintCtx.Policy.nReserved1 == -1)
    {
        bLog = (boolean_t)g_DrvKext.PrintCtx.Policy.bLoggingOn;
        pLogParam->bLog = bLog;
        pLogParam->nPolicyType = g_DrvKext.PrintCtx.Policy.nPolicyType;
        if(bLog) SmartDrv_LogAppend( pLogParam, VREG );
        LOG_MSG("[DLP][%s] ExistPolicy, pLogParam->nPolicyType: %d\n", __FUNCTION__, pLogParam->nPolicyType);
        return TRUE;
    }
    return FALSE;
}

boolean_t
IsPolicyExist_UploadPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam )
{
    boolean_t bLog = FALSE;

    if(!pczProcName || !pczFilePath || !pLogParam)
    {
        LOG_MSG("[DLP][%s] INVALID PARAMETER \n", __FUNCTION__);
        return FALSE;
    }

    // Print Prevent Poliy Not Exist
    if(FALSE == g_DrvKext.UploadCtx.Policy.bPolicy)
    {
        LOG_MSG("[DLP][%s] NOT UPLOAD PREVENT. \n", __FUNCTION__);
        return FALSE;
    }

    if(g_DrvKext.UploadCtx.Policy.bDisableWrite && g_DrvKext.UploadCtx.Policy.nReserved1 == -1)
    {
        bLog = (boolean_t)g_DrvKext.UploadCtx.Policy.bLoggingOn;
        pLogParam->bLog = bLog;
        pLogParam->nPolicyType = g_DrvKext.UploadCtx.Policy.nPolicyType;
        if(bLog) SmartDrv_LogAppend( pLogParam, VREG );
        LOG_MSG("[DLP][%s] ExistPolicy, pLogParam->nPolicyType: %d\n", __FUNCTION__, pLogParam->nPolicyType);
        return TRUE;
    }
    return FALSE;
}

bool GetProcessName( int nPID, char* pczOutPath, uint32_t nMaxOutPath )
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

int
Kauth_Callback_FileOp(int nPID, kauth_cred_t pCred, void* pData, kauth_action_t Action, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    // 현재 프로세스의 pid, proc_name를 구함
    char     czProcName[MAX_PROC_NAME] = { 0, };
    //int      nPID=0;
    int      nFlag=0;
    vnode_t  pVnode    = NULL;
    char*    pczPath1  = NULL;
    char*    pczPath2  = NULL;
    LOG_PARAM  LogParam;
    memset( &LogParam, 0, sizeof(LogParam) );

    //nPID = GetCurrentProcessId();
    
    GetProcessName(nPID, czProcName, sizeof(czProcName) );
    if(TRUE == IsAllowedProcessId( nPID, czProcName ))
    {   // 허용 프로세스인지 확인
        return KAUTH_RESULT_DEFER;
    }

    // Listening 중인 쓰레드 카운트. Listener 종료 시에 필요.
    OSIncrementAtomic( &g_DrvKext.KauthCtx.nFileOpCount );

    LogParam.nLogType    = LOG_FILEOP;
    LogParam.nProcessId  = nPID;
    LogParam.pczProcName = czProcName;
    LogParam.nAction = Action;
    // printf("[DLP][%s] pid=%d, proc=%s Action=%08x \n", __FUNCTION__, nPID, czProcName, Action );
    // LOG_MSG("[DLP][%s] Kauth_Callback_FileOp() [%s][%s][%x]\n", __FUNCTION__,czProcName, (char*)arg1, Action);
    switch(Action)
    {
        case KAUTH_FILEOP_CLOSE:
        {
            pVnode   = (vnode_t)arg0;
            pczPath1 = (char*)arg1;
            nFlag    = (int)arg2;
            Kauth_FileOp_FileClose( nPID, czProcName, pVnode, pczPath1, nFlag, &LogParam );
            break;
        }
    }

    OSDecrementAtomic( &g_DrvKext.KauthCtx.nFileOpCount );
    return KAUTH_RESULT_DEFER;
}

kern_return_t
InstallKauthListener(void)
{
    LOG_MSG("[DLP][%s] Success \n", __FUNCTION__ );
    return KERN_SUCCESS;
}

kern_return_t
RemoveKauthListener(void)
{
    LOG_MSG("[DLP][%s] Success\n", __FUNCTION__ );
    return KERN_SUCCESS;
}

void UserModeTo_GetPrintSpoolPath()
{
    PCOMMAND_MESSAGE      pCmdNew = NULL;
    PSCANNER_NOTIFICATION pNotify = NULL;
    boolean_t             bReturn = FALSE;
    size_t                nTotalSize = 0, nDataSize=0;

    nDataSize  = sizeof(SCANNER_NOTIFICATION);
    nTotalSize = sizeof(COMMAND_MESSAGE) + nDataSize;

    //LOG_MSG("[DLP][%s] FilePath=%s \n", __FUNCTION__, pczFilePath );
    // pCmdNew = (PCOMMAND_MESSAGE)_MALLOC( nTotalSize, 1, M_ZERO );
    pCmdNew = (PCOMMAND_MESSAGE)MyMAlloc( nTotalSize );
    if(!pCmdNew)
    {
        LOG_MSG("[DLP]-[%s] Memory Allocate Failed.\n", __FUNCTION__);
        return;
    }

    pCmdNew->Size = (ULONG)nTotalSize;
    pCmdNew->Command  = (ULONG)GetPrintSpoolPath;
    pNotify = (PSCANNER_NOTIFICATION)pCmdNew->Data;
    if(pNotify)
    {
        pNotify->nSize    = (ULONG)nDataSize;
        pNotify->pWakeup  = (void*)&bReturn;
    }

    if(UserModeTo_SendCommand( pCmdNew, nTotalSize) != KERN_SUCCESS)
    {
        LOG_MSG("[DLP][%s] UserModeTo_SendCommand() Failed.\n", __FUNCTION__ );
        _FREE( pCmdNew, 1 );
        return;
    }
    
    _FREE( pCmdNew, 1 );
    return;
}

#ifndef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE  32*1024*1024
#endif

void* MyMAlloc(size_t nAllocSize)
{
    void *pReturn = NULL;
    if(nAllocSize > MAX_ALLOC_SIZE) return NULL;
    pReturn = (void*)malloc( nAllocSize);
    if (pReturn != NULL)
    {
        memset( pReturn, 0, nAllocSize );
    }
    return pReturn;
}


