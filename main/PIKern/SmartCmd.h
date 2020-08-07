#ifndef __SMART_DRVCMD_H__
#define __SMART_DRVCMD_H__

#include "event_proto_esf.h"
#include "PISecSmartDrv.h"

#ifdef __cplusplus
extern "C"
{
#endif

long GetCurrentlongTime(void);

void GetCurrentTime( char* pczCurTime, int nTimeBufSize );
    
errno_t SmartCmd_Parser_CM( PCOMMAND_MESSAGE pCmdMsg, void* pParam );

errno_t
SmartCmd_SetDrivePolicy( PCOMMAND_MESSAGE pCmdMsg );

errno_t
SmartCmd_SetExceptDrivePolicy( PCOMMAND_MESSAGE pCmdMsg );

errno_t
SmartCmd_SetPermitProcessName( PCOMMAND_MESSAGE pCmdMsg );
errno_t
SmartCmd_SetPermitFolderName( PCOMMAND_MESSAGE pCmdMsg );
errno_t
SmartCmd_SetPermitFileExtName( PCOMMAND_MESSAGE pCmdMsg );

errno_t SmartCmd_SetQtLimit( PCOMMAND_MESSAGE pCmdMsg );
errno_t SmartCmd_SetQtPathExt( PCOMMAND_MESSAGE pCmdMsg );

errno_t SmartCmd_SetCtrlMobile( PCOMMAND_MESSAGE pCmdMsg );
errno_t SmartCmd_MobileNode_Insert( PCOMMAND_MESSAGE pCmdMsg );
errno_t SmartCmd_MobileNode_RemoveAll( PCOMMAND_MESSAGE pCmdMsg );

void MobileNode_Insert(PMB_NODE pNode);
void MobileNode_Remove(PMB_NODE pNode);
void MobileNode_RemoveAll(void);


bool USBMobilePermitList_Append( MB_PERMIT* pPermit );
void USBMobilePermitList_RemoveAll(void);

errno_t SmartCmd_SetMobilePermitList( PCOMMAND_MESSAGE pCmdMsg );
errno_t SmartCmd_ClrMobilePermitList( PCOMMAND_MESSAGE pCmdMsg );


// Printer Prevent
errno_t
SmartCmd_SetPrintPrevent( PCOMMAND_MESSAGE pCmdMsg );

// Upload Prevent
errno_t
SmartCmd_SetUploadPrevent( PCOMMAND_MESSAGE pCmdMsg );

// Self Protect
errno_t
SmartCmd_SetSelfProtect( PCOMMAND_MESSAGE pCmdMsg );

errno_t
SmartCmd_SetProcessAC( PCOMMAND_MESSAGE pCmdMsg );

errno_t
SmartCmd_SetControlCamera( PCOMMAND_MESSAGE pCmdMsg );

errno_t
SmartCmd_SetControlRNDIS( PCOMMAND_MESSAGE pCmdMsg );

errno_t
SmartCmd_SetControlAirDrop( PCOMMAND_MESSAGE pCmdMsg );

//
// PrintPrevent
//
kern_return_t DrvPrintCtx_Init(void);
kern_return_t DrvPrintCtx_Uninit(void);


// Upload Prevent
kern_return_t DrvUploadCtx_Init(void);
kern_return_t DrvUploadCtx_Uninit(void);

//
// Mobile
//
kern_return_t DrvMobile_Init(void);
kern_return_t DrvMobile_Uninit(void);

kern_return_t DrvCtx_Init(void);
kern_return_t DrvCtx_Uninit(void);

kern_return_t DrvCtx_Policy_Update_DriveName( PVOLUME_DEVICE pVolDevice );
kern_return_t DrvCtx_Policy_Update(void);


PATTACHED_DEVICE_EX DrvCtx_Except_Search( CHAR* pczVolume );

ULONG
GetBusTypeFromVolume( CHAR* pczVolume );

uint64_t
GetFileSize(const char *pPath);

BOOLEAN
IsAllowedProcessId( int nProcID, PCHAR pczProcName );

BOOLEAN
IsUnProtectId( ULONG nPID );

BOOLEAN
IsAllowedFolderFileExt(char* pczPath, BOOLEAN bOpen);

BOOLEAN
IsFileGetExtName(char* pczFilePath, char* pczExt);


boolean_t
SmartDrv_LogAppend( LOG_PARAM* pLogParam, int nVnodeType );


bool IsLogByPassProcess( LOG_PARAM* pLogParam );
bool IsExistParentLogCheck( LOG_PARAM* pParam );

    
#ifdef __cplusplus
};
#endif
 

#endif
