#ifndef __COMMAND_H__
#define __COMMAND_H__

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

// Policy StorDeviceType
#define MEDIA_COPYPREVENT   0x0001
#define MEDIA_CD_DVD        0x0002
#define MEDIA_FLOOPY        0x0004
#define MEDIA_USB           0x0008
#define MEDIA_NET_DRIVE     0x0010
#define MEDIA_SERIAL        0x0020
#define MEDIA_PARALLEL      0x0040
#define MEDIA_1394          0x0080
#define MEDIA_MTP           0x0100
#define MEDIA_PTP           0x0200
#define MEDIA_ANDROID       0x0400
#define MEDIA_APPLE         0x0800
#define MEDIA_PRINTPREVENT  0x1000
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

#define ACCESS_CHECK_NONE     0
#define ACCESS_CHECK_COPY     1
#define ACCESS_CHECK_PRINT    2
#define ACCESS_CHECK_UPLOAD   3

#endif // __COMMAND_H__
