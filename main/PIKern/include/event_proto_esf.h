
#ifndef _event_proto_h
#define _event_proto_h

#ifdef LINUX
#else
#include <sys/kernel_types.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#endif

#include "KernelCommand.h"
#include "DataType.h"


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

#define DAEMON_SHARINGD "/usr/libexec/sharingd"
#define DAEMON_AIRDROP  "/System/Library/PrivateFrameworks/FinderKit.framework/Versions/A/PlugIns/AirDrop.appex/Contents/MacOS/AirDrop"


#define COMM_CHANNEL_NONE       0
#define COMM_CHANNEL_KERNCTL    1   // Kerenl control을 사용하는 채널
#define COMM_CHANNEL_SYSCTL     2   // System control을 사용하는 채널


#define POLICY_0                0   // 정책 없음
#define POLICY_1                1   // 읽기 통제
#define POLICY_2                2   // 쓰기 통제
#define POLICY_3                3   // 내용 기반 검색 정책

#define MAX_PROC             32
#define MAX_PATH            260
#define MAX_SOCKBUF         260

#define MAX_PROC_NAME       256  // Max ProcessName
#define MAX_DEVICE_NAME     256  // Max DeviceName
#define MAX_FOLDER_NAME     128
#define MAX_FILEEXT_NAME     64
#define MAX_ENTRY_LENGTH    512
#define MAX_FILE_LENGTH    1024
#define MAX_2K_BUFFER      2048
#define MAX_4K_BUFFER      4096
#define MAX_8K_BUFFER      8192
#define MAX_QT_COUNT       0x03
#define QT_TYPE_DEFAULT    0x00
#define QT_TYPE_EXTEND     0x01

#define NT_PROCNAME_LEN      16
#define NT_FILE_EXT_LEN       8
#define DEVICE_NAME_SZ      256

#define MAX_ALLOW_PROCNAME   64
#define MAX_ALLOW_PROCID     64
#define MAX_ALLOW_FOLDERNAME 64

#define MAX_DEVICE           36
#define MAX_BASE_SIZE       256
#define MAX_DEVICE_SIZE     260
#define MAX_DOS_SIZE        128
#define MAX_DRIVER_SIZE     128

#define EXCEPT_FLAG_LOG    0x10000
#define EXCEPT_FLAG_POLCY  0x20000

#pragma pack(1)

#define RECORD_TYPE_NORMAL                       0x00000000
#define RECORD_TYPE_FILETAG                      0x00000004
#define RECORD_TYPE_FLAG_STATIC                  0x80000000
#define RECORD_TYPE_FLAG_EXCEED_MEMORY_ALLOWANCE 0x20000000
#define RECORD_TYPE_FLAG_OUT_OF_MEMORY           0x10000000
#define RECORD_TYPE_FLAG_MASK                    0xFFFF0000

#pragma pack()

#pragma pack()

#endif // _event_proto_h
