
#ifndef _PW_PROTOCOL_H_
#define _PW_PROTOCOL_H_

#include "KernelCommand.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define KERNCTL_PWDISK_NAME "com.somansa.PWDisk"


#define PATH_SMSDISK_DATA  "/SMSDisk/Data"
#define PATH_SMSDISK_DRIVE "/SMSDisk/Drive"

#define PWDISK_VOLUMENAME  "PWDisk_D"
#define PWDISK_VOLUMEPATH  "/Volumes/PWDisk_D"
//
// #define PWDISK_DIRPATH    "/usr/local/Privacy-i/Quarantine/PWDisk"
// #define PWDISK_IMAGEPATH  "/usr/local/Privacy-i/Quarantine/PWDisk.dmg"
//
#define PWDISK_DIRPATH     "/usr/local/PWDisk/Quarantine/PWDisk"
#define PWDISK_IMAGEPATH   "/usr/local/PWDisk/Quarantine/PWDisk.dmg"



#define TMP_PATH_RELEASE "/tmp/ReleasePath"
#define TMP_PATH_CREATE  "/tmp/CreateFile.txt"
#define TMP_PATH_COPY    "/tmp/CopyFile.txt"


	typedef char             CHAR;
	typedef unsigned char  UCHAR;
	typedef char*            PCHAR;
	typedef unsigned char* PUCHAR;
	typedef uint32_t      ULONG;
	typedef uint32_t*     PULONG;
	//typedef __uint32_t      ULONG;
	//typedef __uint32_t*     PULONG;
	typedef  unsigned long BOOLEAN;
	typedef void*           PVOID;
	typedef __int64_t       LARGE_INTEGER;
	typedef __uint64_t       ULONGLONG;



	//
	// #define SIZE_KB 1024
	// #define SIZE_MB (SIZE_KB * 1024)
	// #define SIZE_GB (SIZE_MB * 1024)
	//

#define SIZE_KB 1000
#define SIZE_MB (SIZE_KB * 1000)
#define SIZE_GB (SIZE_MB * 1000)


#ifndef MAX_PATH
    #define MAX_PATH         1024 // 260
#endif
    
#define MAX_RETRY        256
#define MAX_FILE_NAME    260

#define LOG_TIME_LEN      32
#define LOG_PROC_LEN     260
#define LOG_FILE_LEN     260

#define RECORD_TYPE_NORMAL                       0x00000000
#define RECORD_TYPE_FILETAG                      0x00000004
#define RECORD_TYPE_FLAG_STATIC                  0x80000000
#define RECORD_TYPE_FLAG_EXCEED_MEMORY_ALLOWANCE 0x20000000
#define RECORD_TYPE_FLAG_OUT_OF_MEMORY           0x10000000
#define RECORD_TYPE_FLAG_MASK                    0xFFFF0000


	/***********************************************************************************************/

	typedef enum _EM_PWAPI_RETURNCODE
	{
		PWAPI_FAIL = -1,					//   FAIL
		PWAPI_OK = 0,						//   SUCCESS
		////////
		PWAPI_INVALIDARG_EINVAL		= 1,		// destinaion or source is NULL, memcpy_s 참조
		PWAPI_INVALIDARG_ERANGE		= 2,
		PWAPI_COPY_STRING_FAILED	= 3,		// String Copy Failed
		PWAPI_INVALID_RESPONSE		= 4,
		PWAPI_INVALID_BUFFERSIZE	= 5,
		PWAPI_IPC_FAILED			= 6,
		PWAPI_COPY_FILE_FAILED		= 7,
		PWAPI_DELETE_FILE_FAILED	= 8,
		PWAPI_CREATE_DIR_FAILED		= 9,
		PWAPI_NORMAL_PRG_RUNNING	= 10,
		PWAPI_VIRTUAL_PRG_RUNNING	= 11,
		PWAPI_ALREADY_OPENED		= 12,
		////////
		PWAPI_RETURNCODE_END				// RETURNCODE
	} EM_PWAPI_RETURNCODE;


	typedef enum _PWDISK_ACCESS_TYPE
	{
		PWAccessUnknown  = 0x00,
		PWAccessRead     = 0x01,
		PWAccessWrite    = 0x02,
		PWAccessRename   = 0x04,
		PWAccessDelete   = 0x08,
		PWAccessMove     = 0x10,
		PWAccessExecute  = 0x20,
		PWAccessDirWrite = 0x40
	} PWDISK_ACCESS_TYPE;

	typedef enum _PWDISK_COMMAND
	{
		/**************************************************************************************************/
		PWEventQueue_BEGIN     = 2001, // Kernel control 채널 사용 시, 이벤트 큐 닫기 명령 (agent <-> kext), 응답 값 사용
		PWEventQueue_DESTROY   = 2002, // System control 채널 사용 시, 쓰레드 풀에 참여한다는 명령 (agent -> kext)
		/**************************************************************************************************/
		PWGetDrvVersion        = 2011,
		/**************************************************************************************************/

		PWSetFilter            = 2021,
		PWUnsetFilter          = 2022,

		PWSetDiskVolumePath    = 2031,
		PWClrDiskVolumePath    = 2032,

		PWProcessID_Add        = 2041,
		PWProcessID_Remove     = 2042,
		PWProcessID_RemoveAll  = 2043

			/***********************************************************************************************/
	} PWDISK_COMMAND;

	typedef enum _PWDISK_NOTIFY_COMMAND
	{
		PWNotify_ProcessNotify   = 3101,
		PWNotify_PWDiskLogNotify = 3201,
		PWFileEventDiskFull      = 3301,
		PWFileExchangeDataResult = 3302

	} PWDISK_NOTIFY_COMMAND;

	typedef struct _MSG_COMMAND
	{
		ULONG  Size;
		ULONG  Command;
		UCHAR  Data[];
	} MSG_COMMAND, *PMSG_COMMAND;

	typedef struct _MSG_NOTIFY
	{
		ULONG  nSize;
		ULONG  nAction;
		ULONG  nResult; // 0: Deny  1: Allow, 2: Cancel
		ULONG  nPID;
		void*  pVnode;
		void*  pWakeup;
		void*  pParam;
		void*  pParam02;
		void*  pParam03;
		char   czFilePath[MAX_PATH];
		char   czFilePathDst[MAX_PATH];
	} MSG_NOTIFY, *PMSG_NOTIFY;


	typedef struct _LOG_DATA_PW
	{
		char    LogTime[LOG_TIME_LEN];
		UCHAR   LogType;
		UCHAR   LogAction;
		ULONG   ThreadId;
		ULONG   Flags;
		ULONG   Status;
		void*   pVnode;
		BOOLEAN  bDisableResult;
		ULONG   nPID;
		char    ProcName[LOG_PROC_LEN];
		char    FileName[LOG_FILE_LEN];
	} LOG_DATA_PW, *PLOG_DATA_PW;

	typedef struct _LOG_RECORD_PW
	{
		ULONG     Size;
		ULONG     AccessType;
		ULONG     RecordType;
		ULONG     SeqNumber;
		LOG_DATA_PW Data;
	} LOG_RECORD_PW, *PLOG_RECORD_PW;

	typedef struct _LOG_RECORD_PW_EX
	{
		ULONG             Size;
		ULONG             PolicyType;
		LOG_RECORD_PW     Log;
	} LOG_RECORD_PW_EX, *PLOG_RECORD_PW_EX;



	typedef struct _UKP_AGENT
	{
		int    nPID;           // 이벤트 발생한 프로세스의 pid
		void*  pVnode;         // 이벤트 발생한 파일의 vnode.
		void*  pWakeupChannel; // msleep()으로 잠자고 있는 쓰레드를 깨울 때 씀. msleep 도움말 참조.
		char   pBuf[1];        // 파일 경로,
	} UKP_AGENT;

	typedef struct _UKP_FILTER
	{
		boolean_t bFilter;
	} UKP_FILTER;


	typedef struct _UKP_DISK
	{
		BOOLEAN bDiskDeny;
		BOOLEAN bDiskLog;
		char     czVolumePath[MAX_PATH];
	} UKP_DISK;


	typedef struct _UKP_PROC
	{
		ULONG nPID;
		char  czDocPath[MAX_PATH];
	} UKP_PROC;


	typedef struct _PWDISK_SIZE
	{
		boolean_t bMount;
		double     nTotalSize;
		double     nFreeSize;
		char        czVolumeName[MAX_PATH]; // PWDisk_D
		char        czVolumePath[MAX_PATH]; // /Volumes/PWDisk_D
		char        czImagePath[MAX_PATH];  // /usr/local/Privacy-i/Quarantine/PWDisk.dmg
		char        czDirPath[MAX_PATH];    // /usr/local/Privacy-i/Quarantine/PWDisk
	} PWDISK_SIZE;



#ifdef __cplusplus
};
#endif

#endif /* PWProtocol_h */
