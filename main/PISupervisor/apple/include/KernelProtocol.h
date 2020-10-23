#ifndef _KERNELPROTOCOL_H
#define _KERNELPROTOCOL_H

#ifdef LINUX

typedef int* vnode_t;

#else
#include <sys/kernel_types.h>
#include <sys/vnode.h>
#endif

#include "KernelCommand.h"
#include "KernelDataType.h"
#include <string.h>

#define kernctl_name_somansa "com.somansa.PISecSmartDrv"
#define sysctl_name_somansa  "som_somansa_PISecSmartDrv"

// printer with cups framework Start
#define PROC_CUPSD       "cupsd"
#define PROC_CUPS_EXEC   "cupsd-exec"
#define PROC_CUPS_FILTER "xdclfilter"
#define PROC_CUPS_LPD    "lpd"

#define DIR_CUPS       "/private/var/spool/cups/"
#define DIR_CUPS_CF    "/private/var/spool/cups/c0"
#define DIR_CUPS_DF    "/private/var/spool/cups/d0"
#define DIR_CUPS_TMP   "/private/var/spool/cups/tmp/"
#define DIR_CUPS_CACHE "/private/var/spool/cups/cache/"

// cupsd config file path
#define FILE_CUPSD_CONFIG "/private/etc/cups/cups-files.conf"

// printer with cups framework End

#define COMM_CHANNEL_NONE       0
#define COMM_CHANNEL_KERNCTL    1   // Kerenl control을 사용하는 채널
#define COMM_CHANNEL_SYSCTL     2   // System control을 사용하는 채널


#define POLICY_0                0   // 정책 없음
#define POLICY_1                1   // 읽기 통제
#define POLICY_2                2   // 쓰기 통제
#define POLICY_3                3   // 내용 기반 검색 정책

#define MAX_PROC           32

#ifndef MAX_PATH
    #define MAX_PATH          260
#endif


#define MAX_PROC_NAME     256  // Max ProcessName
#define MAX_DEVICE_NAME   256  // Max DeviceName
#define MAX_FOLDER_NAME   128
#define MAX_FILEEXT_NAME   64
#define MAX_ENTRY_LENGTH  512
#define MAX_FILE_LENGTH  1024
#define MAX_2K_BUFFER    2048
#define MAX_4K_BUFFER    4096
#define MAX_8K_BUFFER    8192
#define MAX_QT_COUNT     0x03
#define QT_TYPE_DEFAULT  0x00
#define QT_TYPE_EXTEND   0x01

#define NT_PROCNAME_LEN        16
#define NT_FILE_EXT_LEN         8
#define DEVICE_NAME_SZ        256

#define MAX_ALLOW_PROCNAME     64
#define MAX_ALLOW_PROCID       64
#define MAX_ALLOW_FOLDERNAME   64

#define MAX_DEVICE        36
#define MAX_BASE_SIZE    256
#define MAX_DEVICE_SIZE  260
#define MAX_DOS_SIZE     128
#define MAX_DRIVER_SIZE  128

#define EXCEPT_FLAG_LOG    0x10000
#define EXCEPT_FLAG_POLCY  0x20000

#pragma pack(1)

#ifndef _ACCESS_TYPE

typedef enum _ACCESS_TYPE
{
    accessUnknown  = 0x00,
    accessRead     = 0x01,
    accessWrite    = 0x02,
    accessRename   = 0x04,
    accessDelete   = 0x08,
    accessMove     = 0x10,
    accessExecute  = 0x20,
    accessDirWrite = 0x40
} ACCESS_TYPE;

#endif


#define RECORD_TYPE_NORMAL                       0x00000000
#define RECORD_TYPE_FILETAG                      0x00000004
#define RECORD_TYPE_FLAG_STATIC                  0x80000000
#define RECORD_TYPE_FLAG_EXCEED_MEMORY_ALLOWANCE 0x20000000
#define RECORD_TYPE_FLAG_OUT_OF_MEMORY           0x10000000
#define RECORD_TYPE_FLAG_MASK                    0xFFFF0000

#pragma pack()

#pragma pack(1)
struct event_proto
{
    int16_t size;
    int16_t command;    // 이벤트 처리 요청, 결과 통보 등을 정의하는 명령. command.h 참조.
    void*   param;        // command의 파라미터 1
    void*   param2;       // command의 파라미터 2
    void*   param3;       // command의 파라미터 3
    int     pid;            // 이벤트 발생한 프로세스의 pid
    vnode_t vnode;      // 이벤트 발생한 파일의 vnode.
    void*   wakeup_chan;  // msleep()으로 잠자고 있는 쓰레드를 깨울 때 씀. msleep 도움말 참조.
    char buf[1];        // 파일 경로, 디바이스 이름 등을 전송할 때 씀.
};
#pragma pack()

#pragma pack(1)
typedef struct _COMMAND_MESSAGE
{
    ULONG  Size;
    ULONG  Command;
    UCHAR  Data[];
} COMMAND_MESSAGE, *PCOMMAND_MESSAGE;
#pragma pack()

#pragma pack(1)
typedef struct _ATTACHED_DEVICE
{
    BOOLEAN  bLoggingOn;
    BOOLEAN  bDisableAll;
    BOOLEAN  bDisableRead;
    BOOLEAN  bDisableWrite;
    int32_t  nReserved1;
    char     DeviceNames[ MAX_DEVICE_NAME ];

#ifdef __cplusplus
	_ATTACHED_DEVICE() {
		bLoggingOn = 0;
		bDisableAll = 0;
		bDisableRead = 0;
		bDisableWrite = 0;
		nReserved1 = -1;
		memset(DeviceNames, 0x00, MAX_DEVICE_NAME);
	};
#endif
} ATTACHED_DEVICE, *PATTACHED_DEVICE;

typedef struct _ATTACHED_DEVICE_EX
{
    ATTACHED_DEVICE cDevice;
    ULONG           ulStorDevType;

#ifdef __cplusplus
	_ATTACHED_DEVICE_EX() {
		ulStorDevType = 0;
	};
#endif
} ATTACHED_DEVICE_EX, *PATTACHED_DEVICE_EX;

typedef struct _PRINT_POLICY
{
	BOOLEAN bPolicy; // 1: Policy-On 0: Policy-Off,  IsPolicyExist

	ULONG   nPolicyType;

	// Watermark
	BOOLEAN bWatermark;

	// PrintPrevent Policy
	BOOLEAN bLoggingOn;
	BOOLEAN bDisableAll;
	BOOLEAN bDisableRead;
	BOOLEAN bDisableWrite;
	int32_t nReserved1;
	char    czDeviceName[ MAX_DEVICE_NAME ];

#ifdef __cplusplus
	_PRINT_POLICY() {
		bPolicy = 0;
		nPolicyType = 0;
		bWatermark = 0;
		bLoggingOn = 0;
		bDisableAll = 0;
		bDisableRead = 0;
		bDisableWrite = 0;
		nReserved1 = -1;
		memset(czDeviceName, 0x00, MAX_DEVICE_NAME);
	};
#endif
} PRINT_POLICY, *PPRINT_POLICY;

typedef struct _UPLOAD_POLICY
{
    BOOLEAN bPolicy; // 1: Policy-On 0: Policy-Off,  IsPolicyExist
    
    ULONG   nPolicyType;
    
    // Watermark
    //BOOLEAN bWatermark;
    
    // PrintPrevent Policy
    BOOLEAN bLoggingOn;
    BOOLEAN bDisableAll;
    BOOLEAN bDisableRead;
    BOOLEAN bDisableWrite;
    int32_t nReserved1;
    char    czDeviceName[ MAX_DEVICE_NAME ];
    unsigned long lControlListSize;
    char * szControlList;

#ifdef __cplusplus
    _UPLOAD_POLICY() {
        bPolicy = 0;
        nPolicyType = 0;
        //bWatermark = 0;
        bLoggingOn = 0;
        bDisableAll = 0;
        bDisableRead = 0;
        bDisableWrite = 0;
        nReserved1 = -1;
        memset(czDeviceName, 0x00, MAX_DEVICE_NAME);
        lControlListSize = 0;
        szControlList = NULL;
    };
#endif
} UPLOAD_POLICY, *PUPLOAD_POLICY;

typedef struct _ALLOWPROCESSID
{
    ULONG  nAllowedProcID;	 // ProcessID. ( 예) 350)
} ALLOWPROCESSID, *PALLOWPROCESSID;

typedef struct _ALLOWPROCESSNAME
{
    char czAllowProcName[ MAX_PROC_NAME ];
} ALLOWPROCESSNAME, *PALLOWPROCESSNAME;

typedef struct _ALLOWFOLDERNAME
{
    char czAllowFolder[MAX_FOLDER_NAME];
} ALLOWFOLDERNAME, *PALLOWFOLDERNAME;

typedef struct _ALLOWFILEEXTNAME
{
    char czAllowFileExt[MAX_FILEEXT_NAME];
} ALLOWFILEEXTNAME, *PALLOWFILEEXTNAME;

typedef struct _SMART_TASKINFO
{
    ULONG nPID;
    ULONG nTask;
    ULONG nReserved;
} SMART_TASKINFO, *PSMART_TASKINFO;

// Quarantine Folder
typedef struct _QT_ENTRY
{
    ULONG  ulQtType;
    char   czBuffer[ MAX_ENTRY_LENGTH ];
} QT_ENTRY, *PQT_ENTRY;

typedef struct _QT_CTX
{
    BOOLEAN  bQtLimit;
    ULONG    ulCount;
    QT_ENTRY Entry[ MAX_QT_COUNT ];
} QT_CTX, *PQT_CTX;


#define RESULT_DENY    0x00  // Deny
#define RESULT_ALLOW   0x01  // Allow
#define RESULT_CANCEL  0x02  // Cancel

typedef struct _APP_PROCINFO
{
    int  nUID;
    int  nRUID;
    int  nPID;
    int  nPPID;
} APP_PROCINFO, *PAPP_PROCINFO;

// Kernel --> User
typedef struct _SCANNER_NOTIFICATION
{
    ULONG    nSize;
    ULONG    nAction;
    ULONG    nResult; // 0: Deny  1: Allow, 2: Cancel
    ULONG    nPID;
    vnode_t  pVnode;
    void*    pWakeup;
    //
    void*    pParam;
    void*    pParam02;
    void*    pParam03;
    //
    char     czFilePath  [MAX_FILE_LENGTH];
    char     czQtFilePath[MAX_FILE_LENGTH];
    
} SCANNER_NOTIFICATION, *PSCANNER_NOTIFICATION;


#define MAX_MOUNT 16
typedef struct _MOUNT_VOLUME
{
    ULONG  nBusType;
    char   czBasePath[MAX_BASE_SIZE];
    char   czDeviceName[MAX_DEVICE_SIZE]; 
} MOUNT_VOLUME, *PMOUNT_VOLUME;

// Kernel --> User
typedef struct _MOUNT_NOTIFICATION
{
    MOUNT_VOLUME  Mount;
} MOUNT_NOTIFICATION, *PMOUNT_NOTIFICATION;


#define LOG_TIME_LEN  32
#define LOG_PROC_LEN  260
#define LOG_FILE_LEN  260

// 작업후 정의필요함 의미있는 데이터를 현재는 모르겠음( 2016.03.18 )
typedef struct _LOG_DATA
{
    char    LogTime[LOG_TIME_LEN];
    UCHAR   LogType;
    UCHAR   LogAction;
    ULONG   ThreadId;
    ULONG   Flags;
    ULONG   Status;
    void*   pVnode;
    
    BOOLEAN bDisableResult;
    
    ULONG   ProcessId;
    char    ProcName[LOG_PROC_LEN];
    char    FileName[LOG_FILE_LEN];
} LOG_DATA, *PLOG_DATA;


typedef struct _SMART_LOG_RECORD
{
    ULONG     Size;
    ULONG     AccessType;
    ULONG     RecordType;
    ULONG     SeqNumber;
    LOG_DATA  Data;
} SMART_LOG_RECORD, *PSMART_LOG_RECORD;


typedef struct _SMART_LOG_RECORD_EX
{
    ULONG             Size;
    ULONG             PolicyType;
    SMART_LOG_RECORD  Log;
} SMART_LOG_RECORD_EX, *PSMART_LOG_RECORD_EX;


//
// Protocol Data Definition
//

typedef struct _COMM_AGENT_INFO
{
    int      nPID;           // 이벤트 발생한 프로세스의 pid
    vnode_t  pVnode;         // 이벤트 발생한 파일의 vnode.
    void*    pWakeupChannel; // msleep()으로 잠자고 있는 쓰레드를 깨울 때 씀. msleep 도움말 참조.
    char     pBuf[1];        // 파일 경로,
} COMM_AGENT_INFO;

typedef struct _MB_POLICY
{
    // MTP
    BOOLEAN  bMtpLog;
    BOOLEAN  bMtpBlock;
    // PTP
    BOOLEAN  bPtpLog;
    BOOLEAN  bPtpBlock;
    
    // Not Used
    // Android
    BOOLEAN  bAndroidLog;
    BOOLEAN  bAndroidBlock;
    // Apple
    BOOLEAN  bAppleLog;
    BOOLEAN  bAppleBlock;
    
} MB_POLICY, *PMB_POLICY;



#define SPT_PLIST_LENGTH  128
#define SPT_SERVICE_COUNT 32

typedef struct _SPT_SERVICE
{
    ULONG nPID;
    char  czPList[SPT_PLIST_LENGTH];
} SPT_SERVICE, *PSPT_SERVICE;

typedef struct _SELF_PROTECT
{
    BOOLEAN      bProtect;
    ULONG         nService;
    char          FilePath[MAX_PATH];
    SPT_SERVICE Services[SPT_SERVICE_COUNT];
} SELF_PROTECT, *PSELF_PROTECT;


// ProcessAccessCheck Policy
typedef struct _PROC_AC_POLICY
{
    BOOLEAN bLog;
    BOOLEAN bProcAC;
} PROC_AC_POLICY, *PPROC_AC_POLICY;

// SetControlCamera
typedef struct _CAMERA_POLICY
{
    BOOLEAN bCameraLog;
    BOOLEAN bCameraBlock;
    ULONG   nReserved;
    
} CAMERA_POLICY, *PCAMERA_POLICY;

// SetControlRNDIS
typedef struct _RNDIS_POLICY
{
    BOOLEAN  bRNdisLog;
    BOOLEAN  bRNdisBlock;
} RNDIS_POLICY, *PRNDIS_POLICY;

// SetControlAirDrop
typedef struct _AIRDROP_POLICY
{
    BOOLEAN  bAirDropLog;
    BOOLEAN  bAirDropBlock;
    ULONG    nReserved0;
    ULONG    nReserved1;
    ULONG    nReserved2;
    ULONG    nReserved3;
} AIRDROP_POLICY, *PAIRDROP_POLICY;


typedef enum _MB_TYPE
{
    MB_TYPE_NONE    = 0,
    MB_TYPE_MTP     = 1,
    MB_TYPE_PTP     = 2,
    MB_TYPE_ANDROID = 3,
    MB_TYPE_APPLE   = 4
} MB_TYPE;


#define MAX_BASEPATH 260
#define MAX_KEYWORD  128

typedef struct _MB_NODE
{
    MB_TYPE MbType;
    char    czBasePath[ MAX_BASEPATH ];
    char    czKeyword [ MAX_KEYWORD ];
} MB_NODE, *PMB_NODE;

typedef struct _COMM_MB_NODE
{
    MB_NODE Node;
} COMM_MB_NODE, *PCOMM_MB_NODE;


#define MAX_PRODUCT 128
#define MAX_MEDIAID 260

typedef struct _MB_PERMIT
{
    char czMediaName[MAX_PRODUCT];
    char czMediaID[MAX_MEDIAID];
} MB_PERMIT, *PMB_PERMIT;

typedef struct _COMM_MB_PERMIT
{
    MB_PERMIT Permit;
} COMM_MB_PERMIT, *PCOMM_MB_PERMIT;



typedef struct _COMM_MB_POLICY
{
    MB_POLICY Policy;
    
} COMM_MB_POLICY, *PCOMM_MB_POLICY;

typedef struct _COMM_QT_INFO
{
    QT_CTX QtCtx;
} COMM_QT_INFO, *PCOMM_QT_INFO;

typedef struct _EVT_PARAM
{
    // Result
    boolean_t bAccess;
    void*     pEvtCtx;
    int       Command;
    int       ProcessId;
    size_t    FileSize;
    char*     pFilePath;
    char*     pQtFilePath;
    // FileScan --> BusType
    ULONG     ullReserved;
    ULONG     ullReserved1;
    ULONG     ullReserved2;
    
} EVT_PARAM, *PEVT_PARAM;

#pragma pack()

typedef enum _STORAGE_BUS_TYPE
{
    BusTypeUnknown            = 0x00,
    BusTypeScsi               = 0x1,
    BusTypeAtapi              = 0x2,
    BusTypeAta                = 0x3,
    BusType1394               = 0x4,
    BusTypeSsa                = 0x5,
    BusTypeFibre              = 0x6,
    BusTypeUsb                = 0x7,
    BusTypeRAID               = 0x8,
    BusTypeiScsi              = 0x9,
    BusTypeSas                = 0xA,
    BusTypeSata               = 0xB,
    BusTypeSd                 = 0xC,
    BusTypeMmc                = 0xD,
    BusTypeVirtual            = 0xE,
    BusTypeFileBackedVirtual  = 0xF,
    BusTypeMax                = 0x10,
    
    BusTypeSFolder            = 0x21,
    BusTypeThunderBolt        = 0x31,
    
    BusTypeMaxReserved        = 0x7F
} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;

typedef enum _ACTION_TYPE
{
    ActionTypePrint           = 0x80,
    ActionTypeUpload,
} ACTION_TYPE, *PACTION_TYPE;

#endif // _KERNELPROTOCOL_H
