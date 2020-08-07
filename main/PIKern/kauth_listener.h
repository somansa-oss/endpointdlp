#ifndef __KAUTH_LISTENER_H__
#define __KAUTH_LISTENER_H__

#include "DataType.h"
//#include "command.h"
#include "KernelCommand.h"
#include "PISecSmartDataType.h"
//#include "DACheck.h"

#ifdef __cplusplus
extern "C"
{
#endif



//
///***********************************************************************************/
//// 에이전트에 통보
///***********************************************************************************/
//
//// 채널(kernel control, system control)을 통해서 이벤트 패킷을 에이전트로 전송하는 함수.
//kern_return_t
//UserModeTo_SendCommand_WithEvent( kern_ctl_ref pEventRef, uint32_t nEventUnit, void* pData, size_t nLength );

kern_return_t
UserModeTo_SendCommand( void* pData, size_t nLength  );

//kern_return_t
//UserModeTo_SendCommandULONG( SMARTDRV_COMMAND nCommand, ULONG ulValue );
//
///***********************************************************************************/
///***********************************************************************************/
//
//errno_t
//IsExist_VnodeFile(char* pczPath);
//
//errno_t
//Vnode_CopyFile(char* pczPath, char* pczQtPath, uint64_t nFileSize);
//
//// Folder Create
//errno_t
//Vnode_MakeDir(char* pczDirPath);

boolean_t
QtFetchValidPath(char* pczPath, char* pczOutQtPath, size_t nMaxQtPath, int nPolicyType );

//boolean_t
//QtCopyFileKern(char* pczPath, char* pczQtPath, uint64_t nFileSize );
//
///***********************************************************************************/
///***********************************************************************************/
//
//boolean_t
//UserModeTo_FileIsRemove(int nPID, vnode_t pVnode, char* pczFilePath);


boolean_t
UserModeTo_SmartLogNotify( void* pLogBuf, ULONG nLogBufSize );

// 에이전트에 내용 기반 검색 요청
boolean_t
UserModeTo_FileScan( int nPID, vnode_t pVnode, char* pczFilePath, int64_t nFileSize, int nPolicyType );

// 에이전트에 파일 삭제 요청
boolean_t
UserModeTo_FileDelete(char* pczFilePath );

//// 에이전트에 두 파일에 대한 exchangedata() 함수 호출 요구.
//boolean_t
//UserModeTo_FileExchangeData(char*pczFilePath1, char* pczFilePath2 );
//
//boolean_t
//UserModeTo_FileRename(int nPID, vnode_t pVnode, char*pczFilePath);
//
//boolean_t
//UserModeTo_FileEventDiskFull(int nPID, vnode_t pVnode, char*pczFilePath);
//
//boolean_t
//UserModeTo_ProcessCallback(boolean_t bCreate, int nPID, char* pczProcName);
//
//
////
//// 에이전트에 프로세스 차단여부 검색요청
//boolean_t
//UserModeTo_ProcessAccessCheck( int nPID, char* pczProcessName, char* pczFilePath );
//
////
//// 에이전트에 프로세스 생성통지
////
//boolean_t
//UserModeTo_CreateProcessAccessCheck( int nPID, char* pczProcName, char* pczFilePath );
//
//
////
//// 에이전트에 PISupervisor FullDiskAccessCheck 체크요청
////
//boolean_t
//UserModeTo_FullDiskAccessCheck( int nPID, char* pczProcName, DACCESS_CHECK* pDACheck );

/*************************************************************************************/
// Policy Check Function
/*************************************************************************************/

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

//boolean_t
//IsPolicyExist_CopyPrevent_LogNone(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam );


//// Printer Control
//boolean_t
//IsPolicyExist_Print_BlockRead(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_Print_BlockWrite(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );

//boolean_t
//IsPolicyExist_Print_BlockAll(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );

boolean_t
IsPolicyExist_PrintPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );

//boolean_t
//IsPolicyExist_PrintPrevent_LogNone(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam );

boolean_t
IsPolicyExist_UploadPrevent(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLogParam );

//boolean_t
//IsPolicyExist_PrintWatermark(int nPID, char* pczProcName, const char* pczFilePath, LOG_PARAM* pLog );
//
//boolean_t
//IsProtect_AirDrop( int nPID, char* pczProcName, int nAction, char* pczVnodePath, LOG_PARAM* pLog );
//
//boolean_t
//IsProtect_AirDropEx( int nPID, char* pczProcName, int nAction, LOG_PARAM* pLog );
//
//boolean_t IsPolicyExist_AirDropBlock(void);
//boolean_t IsPolicyExist_AirDropLog(void);
//
//boolean_t
//CtrlCheck_Read(int nPID, char* pczProcName, int nVnodeType, char* pczVnodePath, LOG_PARAM* pLog );
//
//boolean_t
//CtrlCheck_Write(int nPID, char* pczProcName, int nVnodeType, char* pczVnodePath, LOG_PARAM* pLog );

boolean_t
CtrlCheck_Contents(int nPID, char* pczProcName, vnode_t pVnode, const char* pczPath, LOG_PARAM* pLog, DLP_POLICY_TYPE_ENUM emPolicyType);

//boolean_t
//CtrlCheck_ExchangeData( int nPID, char* pczProcName, const char* pczPath1, const char* pczPath2, LOG_PARAM* pLog );
//
/*********************************************************************************************/
/*********************************************************************************************/
    
// FileOperation scope listener 함수.
// 파일 close 과정에서 호출되는 KAUTH_FILEOP_CLOSE 액션에 반응.
// Dirty 플래그에 불이 들어와 있는(내용을 고친) removable 디바이스의 파일들만 모니터링 대상.
// 관련 정보를 에이전트에 전송하여 내용 기반 검색 결과를 기다리며 최대 10초 간의 잠에 들게 됨.
// 에이전트가 '접근 불가' 판정을 내리면 파일 내용을 전부 0으로 덮어 씌움.
// 만약 10초간의 기다림에도 에이전트로부터 응답이 없으면 '접근 불가' 판정을 받은 것처럼 행동함.
int
Kauth_Callback_FileOp( int nPID,
                       kauth_cred_t   credential,
                       void*          pData,
                       kauth_action_t action,
                       uintptr_t      arg0,
                       uintptr_t      arg1,
                       uintptr_t      arg2,
                       uintptr_t      arg3  );
    
//// Vnode scope listener 함수.
//// KAUTH_VNODE_READ_DATA, KAUTH_VNODE_WRITE_DATA 등 두 개의 액션에 반응함.
//// KAUTH_VNODE_READ_DATA는 policy 1에서 반응.
//// KAUTH_VNODE_WRITE_DATA는 policy 2에서 반응.
//// Removable 디바이스의 파일에 접근(읽기, 쓰기)하려 할 때 접근을 막고 에이전트에 정보를 통보함.
//int
//Kauth_Callback_Vnode( kauth_cred_t   credential,
//                      void *         pData,
//                      kauth_action_t action,
//                      uintptr_t      arg0,
//                      uintptr_t      arg1,
//                      uintptr_t      arg2,
//                      uintptr_t      arg3  );

/****************************************************************************************************************************/
/****************************************************************************************************************************/

// Install
kern_return_t
InstallKauthListener(void);

// Remove
kern_return_t
RemoveKauthListener(void);


/****************************************************************************************************************************/
/****************************************************************************************************************************/
void UserModeTo_GetPrintSpoolPath(void);

#ifdef __cplusplus
};
#endif
        
#endif // _KAUTH_LISTENER_H_
