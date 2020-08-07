
#ifndef __KERNELCOMMAND_H__
#define __KERNELCOMMAND_H__


///*
#define SET_POLICY              6       // 정책(1,2,3)을 설정하라는 명령 (agent -> kext)
#define SET_DEBUG               7       // 디버그 메시지를 출력할지 말지를 설정하는 명령 (agent -> kext)
#define GET_FILEDATA            8       // Vnode를 키로 사용하여 파일 내용을 읽을 때 사용 (agent -> kext)
#define NOTIFY_DIRTY            9       // 정책 3번 사용 시, 내용 기반 검색을 요청하는 명령 (kext -> agent)
#define REPORT_DIRTY            10      // 정책 3번에 대한 내용 기반 검색 완료를 통보하는 명령 (agent -> kext)
#define NOTIFY_NO_READ          11      // 정책 1번 사용 시, 읽기 통제 하였음을 통보하는 명령 (kext -> agent)
#define NOTIFY_NO_WRITE         12      // 정책 2번 사용 시, 쓰기 통제 하였음을 통보하는 명령 (kext -> agent)
#define IS_REMOVABLE            13      // Removable 디바이스 확인을 요청하는 명령 (kext -> agent)
#define REPORT_REMOVABLE        14      // Removable 디바이스 확인 결과를 통보하는 명령 (agent -> kext)
#define DELETE_FILE             15      // 파일 삭제를 요청하는 명령 (kext -> agent)
#define REPORT_DELETE_FILE      16      // 파일 삭제 결과를 통보하는 명령 (agent -> kext)
#define EXCHANGE_DATA           17      // exchangedata() 호출을 요청하는 명령 (kext -> agent)
#define REPORT_EXCHANGE_DATA    18      // exchangedata() 호출 결과를 통보하는 명령 (agent -> kext)
#define INSERT_ANDROID_NODE     19      // 안드로이드 디바이스 노드 경로를 추가하는 명령 (agent -> kext)
#define REMOVE_ANDROID_NODE     20      // 안드로이드 디바이스 노드 경로를 제거하는 명령 (agent -> kext)

#define BEGIN_TEST              30      // 스트레스 테스트 등 용도
//*/

typedef enum _SMARTDRV_COMMAND
{
/**************************************************************************************************/
    NO_COMMAND            = 1000, // Kernel control 채널 사용 시, 이벤트 큐 시작 명령 (agent -> kext)
    BEGIN_EVENT_QUEUE     = 1001, // Kernel control 채널 사용 시, 이벤트 큐 닫기 명령 (agent <-> kext), 응답 값 사용
    DESTROY_EVENT_QUEUE   = 1002, // System control 채널 사용 시, 쓰레드 풀에 참여한다는 명령 (agent -> kext)
    BEGIN_WAIT_THREAD     = 1003, // System control 채널 사용 시, 쓰레드 풀에 참여한다는 명령 (agent -> kext)
    ACTIVATE_THREAD_POOL  = 1004, // System control 채널 사용 시, 쓰레드 풀 시작 명령 (agent -> kext)
    DESTROY_THREAD_POOL   = 1005, // System control 채널 사용 시, 쓰레드 풀 닫기 명령 (agent -> kext)
/**************************************************************************************************/
    GetFileData           = 1009,
/**************************************************************************************************/
    GetDrivePolicy        = 1011,
    SetDrivePolicy        = 1012,
    SetExceptDrivePolicy  = 1013,
    SetQuarantinePathExt  = 1014,
    SetQuarantineLimit    = 1015,
/**************************************************************************************************/
    
    SetMobilePermitList   = 1021,
    ClrMobilePermitList   = 1022,
    
    SetCtrlMobile         = 1026,
    MobileNodeInsert      = 1027,
    MobileNodeRemoveAll   = 1028,

/**************************************************************************************************/
    SetPermitProcessName  = 1031,
    SetPermitFolderName   = 1032,
    SetPermitFileExtName  = 1033,
    SetProcessIdUnprotect = 1034,
/**************************************************************************************************/
    GetSmartDrvLogEx      = 1041,
    GetSmartDrvLog        = 1042,
    GetSmartDrvVersion    = 1043,
/**************************************************************************************************/
    SetStopSmartDrv       = 1044, //  [3/27/2015 sally]
    SetStartSmartDrv	  = 1045, //  [3/27/2015 sally]
    IsUseSmartDrv	      = 1046, //
    GetEnumDriveList      = 1047,
    GetSmartBusType       = 1048,
    GetSmartVolumeSize    = 1049,
/**************************************************************************************************/
    SetPrintPrevent       = 1071,
/**************************************************************************************************/
    SetSelfProtect           = 1081,
    SetSelfProtectOnlyPermit = 1082,
/**************************************************************************************************/
    SetUploadPrevent      = 1091,
    SetProcessAccessCheck = 1101,
/**************************************************************************************************/
    SetControlCamera      = 1201,
    SetControlRNDIS       = 1301,
/**************************************************************************************************/
    SetControlAirDrop     = 1305
    
} SMARTDRV_COMMAND;


typedef enum _SCANNER_COMMAND
{
/**************************************************************************************************/
    FileScan               = 1101,
    FileScanResult         = 1102,
    FileDelete             = 1103,
    FileDeleteResult       = 1104,
    FileRename             = 1105,
    FileRenameResult       = 1106,
    FileExchangeData       = 1107,
    FileExchangeDataResult = 1108,
/**************************************************************************************************/
    FileEventDiskFull      = 1201,
    FileEventNotify        = 1203,
    
    FileIsRemove           = 1205,
    FileIsRemoveResult     = 1206,
/**************************************************************************************************/
    
    SmartLogNotify         = 1301,
    GetPrintSpoolPath      = 1302,
    GetPrintSpoolPathResult= 1303,
    
/**************************************************************************************************/
    
    ProcessCallback        	 = 1401,
    ProcessAccessCheck       = 1403,
    ProcessAccessCheckResult = 1404,
    
/**************************************************************************************************/
    
    FullDiskAccessCheck      = 1501,
    // Volume Notify
    EnumMountVolumeNotify    = 1505
    
/**************************************************************************************************/
} SCANNER_COMMAND;


// Policy StorDeviceType
#define MEDIA_COPYPREVENT  	0x0001
#define MEDIA_CD_DVD       	0x0002
#define MEDIA_FLOPPY       	0x0004
#define MEDIA_USB          	0x0008
#define MEDIA_NET_DRIVE    	0x0010
#define MEDIA_SERIAL       	0x0020
#define MEDIA_PARALLEL     	0x0040
#define MEDIA_1394         	0x0080
#define MEDIA_MTP          	0x0100
#define MEDIA_PTP          	0x0200
#define MEDIA_ANDROID      	0x0400
#define MEDIA_APPLE        	0x0800
#define MEDIA_PRINTPREVENT 	0x1000
#define MEDIA_UPLOADPREVENT 0x2000
#define MEDIA_BLUETOOTH     0x4000
#define MEDIA_PROCESS       0x8000
#define MEDIA_CAMERA        0xA000
#define MEDIA_RNDIS         0xB000
#define MEDIA_AIRDROP       0xC000


#define LOG_VNODE          0x0000
#define LOG_FILEOP         0x0001
#define LOG_MTP            0x0002
#define LOG_PTP            0x0004
#define LOG_ANDROID        0x0008
#define LOG_APPLE          0x0010
#define LOG_PRINTPREVENT   0x0020
#define LOG_UPLOADPREVENT  0x0040
#define LOG_BLUETOOTH      0x0080
#define LOG_PROCESS        0x0100
#define LOG_CAMERA         0x0200
#define LOG_RNDIS          0x0400
#define LOG_AIRDROP        0x0800

#define ACTION_NONE        0x0000
#define ACTION_OPEN        0x0001
#define ACTION_CONNECT     0x0002
#define ACTION_UNLINK      0x0004
#define ACTION_KILL        0x0008
#define ACTION_READ        0x0010
#define ACTION_WRITE       0x0020
#define ACTION_APPEND      0x0040
#define ACTION_EXCHANGE    0x0080
#define ACTION_CLOSE       0x0100
#define ACTION_EXECUTE     0x0200


#define NOTIFY_BLOCK_READ   0x00
#define NOTIFY_BLOCK_WRITE  0x01
#define NOTIFY_LOG_RECORD   0x02

#define GET_STORDEVICETYPE(x) ((x & 0x0000FFFF))
#define GET_EXCEPTION_FLAG(x) ((x & 0x000F0000))
#define MAKE_LOGEX_POLICYTYPE(exception_flag, stor_devicetype) ((stor_devicetype & 0xFFFF) | ((exception_flag & 0xFFFF) << 16))

#endif // __KERNELCOMMAND_H__
