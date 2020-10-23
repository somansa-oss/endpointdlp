//
//  ESFManager.hpp
//  PISupervisor
//
//  Created by Juno on 2020/07/07.
//  Copyright © 2020 somansa. All rights reserved.
//

#ifndef _ESFManager_H_
#define _ESFManager_H_

#include "ESFControl.h"
#include "SysControl.h"

#ifndef LINUX
#include <mach/boolean.h>
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>

#ifndef LINUX
#include <algorithm>
#endif

#define MAX_BUFFER 1024

typedef int (*LPCALLBACK)(void* param);

class CESFManager
{
public:
    CESFManager();
    ~CESFManager();
    
public:
    static int EventNotify_PostProcess(PEVT_PARAM pEvtInfo);
    
public:
    int EventNotify_FileIsRemove(PEVT_PARAM pEvtInfo);
    int EventNotify_FileScan(PEVT_PARAM pEvtInfo);
    int EventNotify_FileDelete(PEVT_PARAM pEvtInfo);
    int EventNotify_FileRename(PEVT_PARAM pEvtInfo);
    int EventNotify_FileExchangeData(PEVT_PARAM pEvtInfo);
    int EventNotify_FileEventDiskFull(PEVT_PARAM pEvtInfo);
    int EventNotify_SmartLogNotify(PEVT_PARAM pEvtInfo);
    int EventNotify_ProcessAccessCheck(PEVT_PARAM pEvtInfo);
    
public:
    boolean_t IsRemovable( char* pczDevice );
    void RegisterCallback( LPCALLBACK pCallback );
    
    int  ESFModule_Startup(LPCALLBACK pCallback, boolean_t bPWDisk );
    int  ESFModule_Cleanup( boolean_t bPWDisk );
    int  CommStartControl( boolean_t bPWDisk );
    int  CommStopControl( boolean_t bPWDisk );

public:
    int fnGetSmartLogEx();
    int fnGetDrivePolicy(void* pBuf, int nLength);
    int fnSetDrivePolicy(void* pBuf, int nBufSize);
    int fnSetExceptDrivePolicy(void* pBuf, int nBufSize);
    
    int fnSetProcessIdUnprotect(ULONG ProcessId);
    int fnSetPermitProcessName(void* pBuf, int nBufSize);
    int fnSetPermitFolderName(void* pBuf, int nBufSize);
    int fnSetPermitFileExtName(void* pBuf, int nBufSize);
    
    // Mobile
    int fnSetUSBMobilePermit( MB_PERMIT& PermitInfo );
    int fnSetUSBMobilePermitList( std::vector<MB_PERMIT>* pPermitList);
    int fnClrUSBMobilePermitList();
    
    int fnSetCtrlMobile(PMB_POLICY pPolicy);
    int fnMobileNodeInsert(PMB_NODE pNode);
    int fnMobileNodeRemoveAll();
    
    int fnMobileNodeInsert_SetMtp(const char* pczBasePath, const char* pczKeyword );
    int fnMobileNodeInsert_SetPtp(const char* pczBasePath, const char* pczKeyword );
    
    int fnSetQuarantineLimit(boolean_t bQtLimit);
    int fnSetQuarantinePathExt(PQT_CTX pQtCtx);

    //
    // PrintPrevent
    //
    int fnSetPrintPrevent_Off();
    int fnSetPrintPrevent_Allow();
    int fnSetPrintPrevent_Watermark();
    int fnSetPrintPrevent_DisableWrite();
    int fnSetPrintPrevent_CopyPrevent();
    // Real Command
    int fnSetPrintPrevent( void* pBuf, int nBufSize );
    
    // UploadPrevent
    int fnSetUploadPrevent(void* pBuf, int nBufSize );
    
    // SetSelfProtect
    int fnSetSelfProtect_Commit( boolean_t bProtect, ULONG nCommand );
    int fnSetSelfProtect_CommitComplete( void* pBuf, int nBufSize, ULONG nCommand );
    
    // ProcessAccessCheck
    int fnSetProcessAccessCheck( BOOLEAN bProcAC, BOOLEAN bLog );
    
    // SetControlCamera
    int fnSetControlCamera( bool bControl, bool bLog  );
    // SetControlRNDIS
    int fnSetControlRndis( bool bRNdisCtrl, bool RNdisLog );
    // SetControlAirDrop
    int fnSetControlAirDrop( bool bAirDropBlock, bool bAirDropLog );
};

extern LPCALLBACK   g_AppCallback;
extern LPCALLBACK   g_AppCallbackESF;
extern CESFManager g_AppESF;

#endif /* _ESFManager_H_ */
