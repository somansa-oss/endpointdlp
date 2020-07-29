#ifndef __KAUTH_LISTENER_H__
#define __KAUTH_LISTENER_H__

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "DataType.h"
#include "command.h"
#include "PISecSmartDataType.h"

kern_return_t
UserModeTo_SendCommand( void* pData, size_t nLength );


boolean_t
QtFetchValidPath(char* pczPath, char* pczOutQtPath, size_t nMaxQtPath, int nPolicyType );


boolean_t
UserModeTo_SmartLogNotify( void* pLogBuf, ULONG nLogBufSize );

// 에이전트에 내용 기반 검색 요청
boolean_t
UserModeTo_FileScan( int nPID, vnode_t pVnode, char* pczFilePath, int64_t nFileSize, int nPolicyType );

// 에이전트에 파일 삭제 요청
boolean_t
UserModeTo_FileDelete(char* pczFilePath );


boolean_t
IsPolicyExist_BlockRead(int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_BlockWrite(int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_AllowWrite(int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_BlockAll(int nPID, char* pczProcName, int nVnodeType, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_CopyPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_PrintPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );


boolean_t
IsPolicyExist_UploadPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam );

boolean_t
CtrlCheck_Contents(int nPID, char* pczProcName, vnode_t pVnode, const char* pczPath, LOG_PARAM* pLog, DLP_POLICY_TYPE_ENUM emPolicyType);

int
Kauth_Callback_FileOp( int nPID,
                       kauth_cred_t   credential,
                       void*          pData,
                       kauth_action_t action,
                       uintptr_t      arg0,
                       uintptr_t      arg1,
                       uintptr_t      arg2,
                       uintptr_t      arg3  );
  
    
#ifdef __cplusplus
};
#endif
        
        
#endif // _KAUTH_LISTENER_H_
