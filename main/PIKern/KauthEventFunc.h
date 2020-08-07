#ifndef _KAUTH_EVENT_FUNC_H_
#define _KAUTH_EVENT_FUNC_H_

#include "PISecSmartDataType.h"
#include <mach/mach_types.h>
#include <sys/vnode.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
    boolean_t Kauth_FileOp_FileClose( int nPID, char* pczProcName, vnode_t pVnode, char* pczFilePath, int nFlags, LOG_PARAM* pLog );
    boolean_t Kauth_FileOp_FileOpen(  int nPID, char* pczProcName, vnode_t pVnode, char* pczFilePath, LOG_PARAM* pLog );
    boolean_t Kauth_FileOp_FileExchange( int nPID, char* pczProcName, char* pczFilePath1, char* pczFilePath2, LOG_PARAM* pLog );
    
    boolean_t Kauth_FileOp_FileRename( int nPID, char* pczProcName, char* pczPrevPath, char* pczNextPath, LOG_PARAM* pLog );
    boolean_t Kauth_FileOp_FileDelete( int nPID, char* pczProcName, char* pczFilePath, LOG_PARAM* pLog);
    
    // Process Callback.
    boolean_t Kauth_FileOp_ProcessExecute( kauth_cred_t pCred, int nPID, char* pczProcName, vnode_t pVnode, char* pczFilePath, LOG_PARAM* pLog );
    
    boolean_t IsProtect_HideProc( char* pczProcName );
    boolean_t IsProtect_DefProc( char* pczProcName );
    boolean_t IsProtect_KillProc( char* pczProcName, __uint32_t nPID );
    boolean_t IsProtect_FilePath( char* pczFilePath );
    boolean_t IsProtect_Service( char* pczService );
    boolean_t IsProtect_KextUnload(int nPID, char* pczProcName, char* pczVnodePath);
    
#ifdef __cplusplus
};
#endif

#endif /* _KEXT_MAC_POLICY_H_ */
