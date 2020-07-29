
#ifndef _PI_DRDEVICE_H_
#define _PI_DRDEVICE_H_

#ifdef MACOS

#include <DiscRecording/DiscRecording.h>
#include <SystemConfiguration/SystemConfiguration.h>

#endif

#include "DLPPolicy.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))
    
typedef struct _MediaData
{
    io_service_t Service;
    io_object_t  Notification;
    CFStringRef  pCFDeviceName;
    UInt32       nLocationID;
} MediaData, *PMediaData;
    
class CPIDRDevice
{
public:
    CPIDRDevice();
    virtual ~CPIDRDevice();
    
public:
    bool ShellExecute( std::string strCommand, std::string& strResult );
    bool GetLoginUID( char* pczUID, uint32 nMaxUID );
    bool GetLoginUName( char* pczUName, uint32 nMaxUName );
    
    bool IsActiveShareingd(void);
    bool IsActiveShareingDaemon(void);
    bool SetProtectSharingDaemon(bool bProtectAction);
    bool SetProtectKillSharingd(void);
    
    bool IsActiveFileSharingSmbd(void);
    bool IsActiveFileSharingAppleFileServer(void);
    bool IsActiveRemoteLogin(void);
    bool IsActiveRemoteAppleEvents(void);
    bool IsActiveScreenSharing(void);
    bool IsActiveRemoteManagement(void);
    
    bool SetProtectFileSharing(bool bProtectAction, std::vector<std::string>& vecDenyList);
    bool SetProtectRemoteManagement(bool bProtectAction, std::vector<std::string>& vecDenyList);
    
public:
    bool IsActiveAppleCamera();
    bool SetAppleCameraStatus(bool bEnable);
    void FetchAppleCameraDeviceName( char* pczOutDeviceName, int nMaxOutDeviceName );
    
public:
    bool MediaDRDeviceStartThread();
    bool MediaDRDeviceStopThread();
    bool MediaDRDeviceExecuteRunLoop();
    void ClearDRDevicePolicy(void);
    void SetDRDevicePolicy(CPISecRule& rule);

    static kern_return_t GetDevicePath( io_object_t object, char* pczPath, size_t nMaxLength );
    static DRDeviceRef GetDRDeviceFromDevicePath( char* pczDevicePath );
    static DRDeviceRef GetDRDeviceFromRegistyEntryPath( char* pczEntryPath );
    static void* ThreadMediaDRDevice( void* pParam );
    static void  MediaDRDeviceEnumerate( void* pRefCon, io_iterator_t Iter );
    static void  MediaDRDeviceAttach( void* pRefCon, io_iterator_t Iter );
    static void  MediaDRDeviceStatus( void*  pRefcon, io_service_t service, natural_t nMsgType, void* pMsgArgument );
    static bool IsDeviceBecomingReady( DRDeviceRef pDevice );
    static bool IsMediaIsReserved( DRDeviceRef pDevice );
    static bool IsDeviceMediaStateTransition( DRDeviceRef pDevice );
    static bool IsDeviceMediaStatePresent( DRDeviceRef pDevice );
    static bool IsDeviceContainsBlankMedia( DRDeviceRef pDevice );
    static bool IsDeviceContainsEraseMedia( DRDeviceRef pDevice );
    static void WritableMediaCheckInDevice( DRDeviceRef pDevice );
    static void WaitForWritableMedia( DRNotificationCenterRef pCenter, void* pObserver, CFStringRef pStrName, DRTypeRef pObject, CFDictionaryRef pInfo );
    
protected:
    static bool RequestDRDeviceTrayOpen( DRDeviceRef pDevice );
    static bool RequestDRDeviceTrayClose( DRDeviceRef pDevice );
    static bool Request_WriteMedia_DRDeviceEjectMedia( DRDeviceRef pDevice, std::string strDeviceName );
    static bool Request_AnyMedia_DRDeviceEjectMedia( DRDeviceRef pDevice, std::string strDeviceName );
    
    static bool IsDRDeviceAnyBurner(DRDeviceRef pDevice);
    static bool IsDRDeviceAnyEraser(DRDeviceRef pDevice);
    static bool IsDRDeviceCDBurner (DRDeviceRef pDevice);
    static bool IsDRDeviceDVDBurner(DRDeviceRef pDevice);
    
    bool IsDisableAll() { return m_Rule.disableAll; }
    bool IsDisableRead() { return m_Rule.disableRead; }
    bool IsDisableWrite() { return m_Rule.disableWrite; }
    bool IsEnableLog() { return m_Rule.enableLog; }
    
public:
    bool FetchWWAN_SCNetworkService( std::vector<std::string>& VectorNetworkServiceEnabled  );
    bool IsRNDIS_DeviceName( char* pServiceName );
    bool RNDISDeviceEnumerate(void* pRefCon, io_iterator_t Iter, char* pServiceName );
    bool ShellCommand_Execute( std::string strCommand, std::string& strResult );
    static void RNDISDeviceEnumerate_Dummy( void* pRefCon, io_iterator_t Iter );
    
public:
    bool m_bDenySharingDaemon;
    IONotificationPortRef m_pNotifyPort;
    pthread_t    m_pMediaThread;
    CFRunLoopRef m_pMediaRunLoop;
    CPISecRule   m_Rule;
    
};


extern CPIDRDevice g_DRDevice;
    
    

#ifdef __cplusplus
};
#endif

#endif /* _PI_DRDEVICE_H_ */
