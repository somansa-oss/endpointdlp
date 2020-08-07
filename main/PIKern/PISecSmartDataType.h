#ifndef _PISecSmart_DataType_H_
#define _PISecSmart_DataType_H_

#ifndef lck_grp_t

#define lck_grp_t           pthread_mutex_t
#define lck_mtx_t           pthread_mutex_t
#define lck_rw_t            pthread_mutex_t

#define lck_mtx_lock        pthread_mutex_lock
#define lck_mtx_unlock      pthread_mutex_unlock
#define lck_rw_lock_exclusive        pthread_mutex_lock
#define lck_rw_unlock_exclusive      pthread_mutex_unlock


#define kern_ctl_ref        int

#define kauth_listener_t    int
#endif

#ifndef LOG_MSG
#define LOG_MSG printf
#endif

#ifndef _FREE
#define _FREE(x,y)  free((void*)x)
#endif

#ifndef OSIncrementAtomic
#define OSIncrementAtomic(x)    x
#define OSDecrementAtomic(x)    x
#endif

#ifndef EINVAL

#define ENOENT          2               /* No such file or directory */
#define EINVAL          22              /* Invalid argument */

#endif

#ifndef KAUTH_RESULT_DEFER

#define KAUTH_RESULT_ALLOW      (1)
#define KAUTH_RESULT_DENY       (2)
#define KAUTH_RESULT_DEFER      (3)

#define KAUTH_FILEOP_OPEN                       1
#define KAUTH_FILEOP_CLOSE                      2
#define KAUTH_FILEOP_RENAME                     3
#define KAUTH_FILEOP_EXCHANGE                   4
#define KAUTH_FILEOP_LINK                       5
#define KAUTH_FILEOP_EXEC                       6
#define KAUTH_FILEOP_DELETE                     7
#define KAUTH_FILEOP_WILL_RENAME                8

/* Flag values returned to close listeners. */
#define KAUTH_FILEOP_CLOSE_MODIFIED                     (1<<1)

#endif

#include <stdlib.h>
#include <mach/mach_types.h>
//#include <libkern/libkern.h>
#include <libkern/OSTypes.h>
//#include <sys/systm.h>
//#include <sys/sysctl.h>
//#include <sys/types.h>
//#include <sys/kauth.h>
//#include <sys/kern_control.h>
//#include <sys/kpi_mbuf.h>
#include <pthread.h>


//#include <KernelProtocol.h>


#include "DataType.h"
#include "event_proto_esf.h"
#include "command.h"
#include "KernelCommand.h"

//////////////////////////////////////////////////////////////////////////////////
//            Need to convert
//////////////////////////////////////////////////////////////////////////////////
//#ifndef _STORAGE_BUS_TYPE
//
//typedef enum _STORAGE_BUS_TYPE
//{
//    BusTypeUnknown            = 0x00,
//    BusTypeScsi               = 0x1,
//    BusTypeAtapi              = 0x2,
//    BusTypeAta                = 0x3,
//    BusType1394               = 0x4,
//    BusTypeSsa                = 0x5,
//    BusTypeFibre              = 0x6,
//    BusTypeUsb                = 0x7,
//    BusTypeRAID               = 0x8,
//    BusTypeiScsi              = 0x9,
//    BusTypeSas                = 0xA,
//    BusTypeSata               = 0xB,
//    BusTypeSd                 = 0xC,
//    BusTypeMmc                = 0xD,
//    BusTypeVirtual            = 0xE,
//    BusTypeFileBackedVirtual  = 0xF,
//    BusTypeMax                = 0x10,
//
//    BusTypeSFolder            = 0x21,
//    BusTypeThunderBolt        = 0x31,
//
//    BusTypeMaxReserved        = 0x7F
//} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;
//
//#endif
//
//#ifndef _ACTION_TYPE
//
//typedef enum _ACTION_TYPE
//{
//    ActionTypePrint           = 0x80,
//    ActionTypeUpload,
//} ACTION_TYPE, *PACTION_TYPE;
//
//#endif

#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DFS_FILE_SYSTEM     0x00000035
#define FILE_DEVICE_DFS_VOLUME          0x00000036
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_DVD                 0x00000033
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_FIPS                0x0000003a
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_KSEC                0x00000039
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SERENUM             0x00000037
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TERMSRV             0x00000038
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026

#define FSD_GLOBAL_DOS  "\\??\\"
#define FSD_GLOBAL      "\\GLOBAL??\\"
#define FSD_DOSDEVICES  "\\DosDevices\\"
#define DRIVER_FLPY     "\\Driver\\FlpyDisk"  // Floopy--RealDevice
#define DRIVER_FIXED    "\\Driver\\FtDisk"    // Fixed/CDRom--RealDevice
#define DRIVER_DISK     "\\Driver\\Disk"     //  Removable--RealDevice
#define DRIVER_CDROM    "\\Driver\\Cdrom"    // CD--RealDevice
#define VOLUME_VIPUSB   "\\Device\\VIPUSB"
#define VOLUME_NICSUSB  "\\Device\\SUSB\\SUSB"


#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field) ((ULONG)&(((type *)0)->field))
#endif

#ifndef FIELD_SIZE
#define FIELD_SIZE(type, field) (sizeof(((type *)0)->field))
#endif

// 0x%08x%08x
#define P64(X)   (unsigned int)(X>>32)&0xFFFFFFFF,(unsigned int)X&0xFFFFFFFF

/* ********************************************************************************************************** */
    
#define MAX_DRIVE          26
#define MAX_DEVICE         36

//
// #define MAX_BASE_SIZE     256
//

#define MAX_DEVICE_SIZE   260
#define MAX_DOS_SIZE      128
#define MAX_DRIVER_SIZE   128
    
#define NT_PROCNAMELEN     16
#define NT_FILE_EXT_LEN     8
#define DEVICE_NAME_SZ    256
        
#define MB_NODE_COUNT      30
#define USBMOBILE_PERMIT_COUNT 256

/* ********************************************************************************************************** */
    
typedef enum _DEVICE_TYPE_V6_ENUM
{
    DEVICE_V6_COPY,
    DEVICE_V6_CDDVD,
    DEVICE_V6_FLOPPY,
    DEVICE_V6_USB,
    DEVICE_V6_NETDRIVE,
    DEVICE_V6_SERIAL,
    DEVICE_V6_PARALLEL,
    DEVICE_V6_IEEE1394,
    DEVICE_V6_BLUETOOTH,
    DEVICE_V6_MAX,
} DEVICE_TYPE_V6_ENUM;

typedef struct _DEVICE_TYPE_V6
{
    ULONG   nCode;
    CHAR*   pczName;
} DEVICE_TYPE_V6;

    
#define MAX_POLICY  36


typedef struct _DRV_EXCEPT
{
    ULONG              nCount;
    ATTACHED_DEVICE_EX DriveEx[MAX_DEVICE];
} DRV_EXCEPT, *PDRV_EXCEPT;

typedef struct _DRV_POLICY
{
     ULONG              nCount;
    ATTACHED_DEVICE_EX DeviceEx[ MAX_POLICY ];
} DRV_POLICY, *PDRV_POLICY;

typedef struct _DRV_CTX
{
    DRV_POLICY  Policy; // ?úÏñ¥?ïÏ±Ö
    DRV_EXCEPT  Except; // ?àÏô∏?ïÏ±Ö
    lck_grp_t*  DrvLockGroup;
    lck_mtx_t*  DrvLock; // LOCK;
    
    lck_grp_t   _DrvLockGroup;
    lck_mtx_t   _DrvLock; // LOCK;
    
} DRV_CTX, *PDRV_CTX;


#if 0
typedef struct _ALLOWPROCESSNAME
{
    char  czAllowedProcessName[NT_PROCNAMELEN + 1];	 // ?ÄÎ¨∏ÏûêÎ°? ( ?? EXECEL.EXE )
} ALLOWPROCESSNAME, *PALLOWPROCESSNAME;

typedef struct _ALLOWFOLDERNAME
{
    char  czAllowedFolderName[NT_PROCNAMELEN + 1];	 // ?ÄÎ¨∏ÏûêÎ°? ( ?? EXECEL.EXE )
} ALLOWFOLDERNAME, *PALLOWFOLDERNAME;


typedef struct _ALLOWFILEEXTNAME
{
    char   czAllowedExtName[NT_FILE_EXT_LEN];	 // ?ÄÎ¨∏ÏûêÎ°? ( ?? .EXE )
} ALLOWFILEEXTNAME, *PALLOWFILEEXTNAME;


typedef struct _ALLOWPROCESSID
{
    ULONG  nAllowedProcessId;	 // ProcessID. ( ?? 350)
} ALLOWPROCESSID, *PALLOWPROCESSID;

typedef struct _ALLOW_DEVICE
{
    WCHAR   DeviceNames[DEVICE_NAME_SZ];
    int     nReserved1;
} ALLOW_DEVICE, *PALLOW_DEVICE;


typedef struct _COMMAND_MESSAGE
{
    SMARTDRV_COMMAND Command;
    ULONG  Reserved;  // Alignment on IA64, DataSize
    UCHAR  Data[];
} COMMAND_MESSAGE, *PCOMMAND_MESSAGE;

#endif


typedef struct _VOLUME_DEVICE
{
    ULONG ulBusType;
    ULONG ulDeviceType;
    CHAR  czBasePath  [MAX_BASE_SIZE];
    CHAR  czDriverName[MAX_DRIVER_SIZE];
    CHAR  czDeviceName[MAX_DEVICE_SIZE]; // Í∏∞Ï?Í∞?
    
    LARGE_INTEGER  ullTotalSize;
    LARGE_INTEGER  ullFreeSize;
    
} VOLUME_DEVICE, *PVOLUME_DEVICE;

    
typedef struct _VOLUME_CTX
{
    ULONG         nCount;
    VOLUME_DEVICE VolumeDevice[ MAX_DEVICE ];
    lck_grp_t*    VolLockGroup;
    lck_mtx_t*    VolLock; // LOCK;
    
    lck_grp_t     _VolLockGroup;
    lck_mtx_t     _VolLock; // LOCK;
    
    ULONG         nCheckCount;
} VOLUME_CTX, *PVOLUME_CTX;

/* ********************************************************************************************************** */
    
typedef struct _MOBILE_CTX
{
    // Android, Syscallhook
    lck_grp_t* RwLockGrp;
    lck_rw_t*  RwLock;
    
    lck_grp_t _RwLockGrp;
    lck_rw_t  _RwLock;

    MB_POLICY  Policy;
    MB_NODE*   pNewMBNodes[MB_NODE_COUNT];
    MB_PERMIT* pNewPermitList[USBMOBILE_PERMIT_COUNT];
} MOBILE_CTX;
    
typedef struct _PROC_AC_CTX
{
    PROC_AC_POLICY Policy;
} PROC_AC_CTX, *PPROC_AC_CTX;
    
/* ********************************************************************************************************** */
    
// Commiunication KernelControl
typedef struct _COMM_CTX_KCTL
{
    boolean_t      bRegistered;
    kern_ctl_ref  pKctlRef;
    kern_ctl_ref  pEvtRef;
    uint32_t       nEvtUnit;
    int64_t        nllKctlThread;
} COMM_CTX_KCTL;
    
    
    
// Commiunication KernelControl
typedef struct _COMM_CTX_KCTL_EX
{

    boolean_t     bRegistered;
    kern_ctl_ref  KctlRef;
    kern_ctl_ref  EvtRef;
    uint32_t      nEvtUnit;
    int64_t       nKctlThread;
 
} COMM_CTX_KCTL_EX;
    
    
    
// Commiunication SystemControl
typedef struct _COMM_CTX_SCTL
{
    boolean_t   bRegisteredOID;
    lck_grp_t*  LockMutexGrp;
    lck_mtx_t*  LockMutex;
    
    lck_grp_t   _LockMutexGrp;
    lck_mtx_t   _LockMutex;
    
    int64_t     nCmdMsg;
    SInt32      nListenerWait;
    SInt32      nListenerCount;
    void*       pEventProto;
    
} COMM_CTX_SCTL;
    
// Commiunication
typedef struct _COMM_CTX
{
    int  nChannel;   // COMM_CHANNEL_NONE
    COMM_CTX_KCTL Kctl;
    COMM_CTX_SCTL Sctl;
} COMM_CTX;
    
typedef struct _KAUTH_CTX
{
    int              nFileOpCount;
    kauth_listener_t pFileOpListener;
    int              nVnodeCount;
    kauth_listener_t pVnodeListener;
    
} KAUTH_CTX;
    
// Agent info
typedef struct _AGENT_CTX
{
    int      nAgentPID;   // ?êÏù¥?ÑÌä∏ ?ÑÎ°ú?∏Ïä§??pid
    vnode_t  pAgentVnode; // ?êÏù¥?ÑÌä∏ ?§Ìñâ ?åÏùº??vnode
    
} AGENT_CTX;
    
    
typedef struct _LOG_PARAM
{
    boolean_t  bLog;
    int    nPolicyType;
    int    nLogType;
    int    nAction;
    int    nProcessId;
    char*  pczProcName;
    void*  pVnode;
    char*  pczPath1;
    char*  pczPath2;
    int    nVnodeType;
    boolean_t  bAllow;
    
} LOG_PARAM, *PLOG_PARAM;
    
typedef struct _NOTIFY_CTX
{
    void* pMediaAttach;
    void* pMediaDetach;
    void* pCDAttach;
    void* pCDDetach;
    void* pPANFirst;
    void* pPANAttach;
    void* pPANDetach;
    void* pRFCOMMFirst;
    void* pRFCOMMAttach;
    void* pRFCOMMDetach;
} NOTIFY_CTX, *PDEVICE_CTX;

    
typedef struct _NODE_TASK
{
    
    ULONG nPID;
    char  czProcName[MAX_PROC_NAME];
    
} NODE_TASK, *PNODE_TASK;
    
typedef struct _PRINT_CTX
{
    PRINT_POLICY Policy;
    NODE_TASK    Node;
    lck_grp_t*   pPrtMtxGrp;
    lck_mtx_t*   pPrtMtx;
    
    lck_grp_t    _pPrtMtxGrp;
    lck_mtx_t    _pPrtMtx;
    
} PRINT_CTX, *PPRINT_CTX;
    
typedef struct _UPLOAD_CTX
{
    UPLOAD_POLICY Policy;
    NODE_TASK     Node;
    lck_grp_t*    pPrtMtxGrp;
    lck_mtx_t*    pPrtMtx;
    lck_mtx_t*    pPrtMtxFileAccessTime;
    lck_mtx_t*    pPrtMtxFileAccessInfo;
    
    lck_grp_t     _pPrtMtxGrp;
    lck_mtx_t     _pPrtMtx;
    lck_mtx_t     _pPrtMtxFileAccessTime;
    lck_mtx_t     _pPrtMtxFileAccessInfo;
} UPLOAD_CTX, *PUPLOAD_CTX;


// From PISecSmartDrv/inc/event_proto.h
//
typedef struct _BLUETOOTH_POLICY
{
    // MTP
    BOOLEAN  bBluetoothLog;
    BOOLEAN  bBluetoothBlock;
    BOOLEAN  bSelectedBluetoothBlock;
    
} BLUETOOTH_POLICY, *PBLUETOOTH_POLICY;
    
typedef struct _MEDIA_CTX
{
    BLUETOOTH_POLICY Bth;
    CAMERA_POLICY    Camera;
    RNDIS_POLICY     Rndis;
    AIRDROP_POLICY   AirDrop;
} MEDIA_CTX, *PMEDIA_CTX;
    

/************************************************************************************************************/
    
    
typedef struct _DRV_KEXT
{
/**********************************************************************/
    // Control Apply
    unsigned int       nDeviceCount;
    ATTACHED_DEVICE_EX CtrlDeviceEx[MAX_DEVICE];
    // Allow Apply
    ULONG             nAllowProcName;
    ALLOWPROCESSNAME  AllowProcName[ MAX_ALLOW_PROCNAME ];
    ULONG             nAllowProcID;
    ALLOWPROCESSID    AllowProcID[ MAX_ALLOW_PROCID ];
    ULONG             nAllowFolder;
    ALLOWFOLDERNAME   AllowFolder[ MAX_ALLOW_FOLDERNAME ];
    ULONG             nAllowFileExt;
    ALLOWFILEEXTNAME  AllowFileExt[ MAX_ALLOW_FOLDERNAME ];
/**********************************************************************/
    // Policy
    VOLUME_CTX    VolCtx;
    QT_CTX        QtCtx;
    DRV_CTX       DrvCtx;
    MOBILE_CTX    MobileCtx;
    // MediaControl
    MEDIA_CTX     MediaCtx;
    COMM_CTX      CommCtx;  // Comm
    KAUTH_CTX     KauthCtx; // Listener
    AGENT_CTX     AgentCtx; // AgentInfo
    // Flag
    BOOLEAN       bIsControlRunning;
    BOOLEAN       bDebugMsg;
    // Previous Version
    int             gPolicy;
    NOTIFY_CTX    NotifyCtx;
    // Printer Control
    PRINT_CTX      PrintCtx;
    UPLOAD_CTX     UploadCtx;
    SELF_PROTECT  SelfCtx;
    // Process AC(Access Check)
    PROC_AC_CTX   ProcCtx;
/**********************************************************************/
} DRV_KEXT, *PDRV_KEXT;
    
extern DRV_KEXT g_DrvKext;
extern char g_czCupsSpoolPathDump[MAX_FILE_LENGTH];
extern char g_czCupsSpoolPath[MAX_FILE_LENGTH];
extern char g_czCupsTempPath[MAX_FILE_LENGTH];

typedef enum _DLP_POLICY_TYPE_ENUM
{
    POLICY_COPY = 0,
    POLICY_PRINT,
    POLICY_UPLOAD,
} DLP_POLICY_TYPE_ENUM;

/* ********************************************************************************************************** */

#ifdef __cplusplus
extern "C"
{
#endif


char*
sms_strstr2(const char* pczStr, const char* pczToken);
    
char*
sms_strstr(char* pczStr, char* pczToken);
    
char*
sms_strnstr(const char *s, const char *find, size_t slen);

char*
sms_strrchr(char* pczStr, int ch);

#ifdef __cplusplus
};
#endif

#endif /* PISecSmart_DataType_H */
