#ifndef __KextManager_H__
#define __KextManager_H__

//#include <string>
#include <stdio.h>
#include <vector>
#include <cerrno>

#include "KernControl.h"
#include "SysControl.h"
#include "PWProtocol.h"

/*
#include <stdlib.h>
#include <string>
#include <algorithm>
*/

#define MAX_BUFFER 1024


//typedef int (*LPCALLBACK)( PEVT_PARAM pParam );
typedef int (*LPCALLBACK)(void* param);


class CKextManager
{
public:
    CKextManager();
    ~CKextManager();
    
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
    
    // PWDisk.kext
    int PWDisk_EN_Process(PEVT_PARAM pEvtInfo);
    int PWDisk_EN_Log(PEVT_PARAM pEvtInfo);

public:
    boolean_t  m_bKctl; // true: KernelContrl false: Syscontrol
    
public:
    boolean_t IsRemovable( char* pczDevice );
    void RegisterCallback( LPCALLBACK pCallback );
    
    int  SelectKextMenu();
    int  SelectDisplayMenu_PWDisk();
    int  SelectDisplayMenu_PISecSmartDrv();
    
    int  KextModule_Startup(LPCALLBACK pCallback, boolean_t bPWDisk );
    int  KextModule_Cleanup( boolean_t bPWDisk );
    int  CommStartControl( boolean_t bPWDisk );
    int  CommStopControl( boolean_t bPWDisk );
    
public:
    int Test_SetExceptUsbPolicy(int nCmd, void* pBuf, const char* pczVolumePath );
    int Test_SetExceptCDPolicy(int nCmd, void* pBuf, const char* pczVolumePath );
    int Test_SetExceptSFolderPolicy( int nCmd, void* pBuf, const char* pczVolumePath );
    
    int Test_SetUsbPolicy(int nCmd, void* pBuf );
    int Test_SetCDPolicy(int nCmd, void* pBuf );
    int Test_SetSFolderPolicy( int nCmd, void* pBuf );
    
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
    
// PWDisk.kext
private:
    PWDISK_SIZE m_DiskSize;
    
public:
    int PWDisk_PWProcess_load(void);
    // int PWDisk_PWProcess_load_edisk(void);
    int PWDisk_PWProcess_unload(void);
    int PWDisk_PWSetFilter(void);
    int PWDisk_PWUnsetFilter(void);
    int PWDisk_PWSetDiskVolumePath( BOOLEAN bDeny, BOOLEAN bLog, char* pczVolumePath );
    int PWDisk_PWClrDiskVolumePath(void);    
    int PWDisk_PWProcessID_Add( ULONG nPID, char* pczDocPath=(char*)"" );
    int PWDisk_PWProcessID_Remove( ULONG nPID, char* pczDocPath=(char*)"" );
    int PWDisk_PWProcessID_RemoveAll(void);
    
public:
    boolean_t PWDisk_DefaultCreateDir(void);
    boolean_t PWDisk_SplitFilePath( char* pczFilePath, char* pczOutDirPath, char* pczOutFileName );
    boolean_t PWDisk_FetchQuarantineFilePath( char* pczFilePath, char* pczOutQtFilePath, int& nOutQtFilePath, int& nErrCode );
    boolean_t PWDisk_FetchReleaseFilePath( char* pczQtFilePath, char* pczDstFilePath, char* pczOutRelFilePath, int& nOutRelFilePath, int& nErrCode );
    
    boolean_t PWDisk_ProcessIsQuarantineFile( char* pczFilePath, int& nErrCode );
    boolean_t PWDisk_ProcessQuarantineFile( char* pczFilePath, char* pczOutQtFilePath, int& nOutQtFilePath, int& nErrCode );
    boolean_t PWDisk_ProcessQuarantineFile_Release( char* pczQtFilePath, char* pczDstDirPath, char* pczOutRelFilePath, int& nOutRelFilePath, int& nErrCode );
    boolean_t PWDisk_ProcessQuarantineFile_Delete( char* pczQtFilePath, int& nErrCode );
    
public:
    
    boolean_t PWdisk_GetDiskVolumePath( char* pczOutVolumePath, int nMaxOutVolumePath );
    boolean_t PWDisk_GetDiskVolumeInfo( PWDISK_SIZE* pDiskInfo );
    
    int PWDisk_mkdirs( const char* pczDirPath, mode_t nMode );
    boolean_t PWDisk_SyCall_CreateDir( char* pczDirPath );

    boolean_t PWDisk_SyCall_CreateFile( const char* pczNewFilePath );
    boolean_t PWDisk_SyCall_CopyFile( const char* pczOrgFilePath, const char* pczDstFilePath );
    boolean_t PWDisk_SyCall_DeleteFile( const char* pczFilePath );
    boolean_t PWDisk_SyCall_DeleteDir( char* pczDirPath );
    
    boolean_t PWDisk_rmdirs(char* pczDirPath, boolean_t bIsErrStop);
    boolean_t PWDisk_IsEmptyDir(char* pczDirPath);
    boolean_t PWDisk_DeleteEmptyDir(char* pczDirPath);
    boolean_t PWDisk_DeleteEmptyDir_Recur(char* pczDirPath);

    boolean_t PWDisk_GetDiskSize( PWDISK_SIZE* pDiskSize );
    boolean_t PWDisk_ConvertDiskSize( PWDISK_SIZE* pDiskSize, int nNewSize );
    boolean_t PWDisk_ProcessConvertDiskSize(void);
    boolean_t PWDisk_PWDefaultAllowProcess(void);
    
public:
    boolean_t ShellCommand_Execute( std::string strCommand, std::string& strResult );
    boolean_t PWDisk_GetFreeDiskPath(PWDISK_SIZE* pDiskSize);
    boolean_t PWDisk_QuarantineDisk_mount(PWDISK_SIZE* pDiskSize);
    boolean_t PWDisk_QuarantineDisk_unmount(PWDISK_SIZE* pDiskSize);
    boolean_t PWDisk_QuarantineDisk_resize(PWDISK_SIZE* pDiskSize, int nNewSize );
   
public:
	boolean_t PWDisk_DuplicateDiskSize( PWDISK_SIZE* pDiskSize );
};

extern LPCALLBACK   g_AppCallback;
extern CKextManager g_AppKext;

#endif /* KextManager_h */
