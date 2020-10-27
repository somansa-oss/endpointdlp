
#include "PIDRDevice.h"
#include <errno.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOMediaBSDClient.h>
#include <IOKit/IOMessage.h>

#ifdef HAVE_IOKIT_STORAGE_IOBLOCKSTORAGEDEVICE_H
#include <IOKit/storage/IOBlockStorageDevice.h>
#endif

#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IOCDMediaBSDClient.h>
#include <IOKit/storage/IODVDMedia.h>
#include <IOKit/storage/IODVDMediaBSDClient.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread/pthread.h>
#include <mach/mach_port.h>
#include "PIDeviceController.h"
#include <IOKit/usb/IOUSBLib.h>

#define DRV_APPLE_CAMERA  "com.apple.driver.AppleCameraInterface"

#define MAX_UNAME 128
#define MAX_UID   128
#define MAX_BUF   260
#define NAME_SHAREDAEMON "com.apple.sharingd"
#define PATH_SHAREDAEMON "/System/Library/LaunchAgents/com.apple.sharingd.plist"

#define SLAGENTS_DIR  "/System/Library/LaunchAgents"
#define SLDAEMONS_DIR "/System/Library/LaunchDaemons"
#define SCREMOTE_DIR  "/System/Library/CoreServices/RemoteManagement/ARDAgent.app/Contents/Resources"


CPIDRDevice g_DRDevice;


CPIDRDevice::CPIDRDevice() : m_pMediaThread(NULL), m_pMediaRunLoop(NULL), m_pNotifyPort(NULL), m_bDenySharingDaemon(false)
{
    ClearDRDevicePolicy();
}

CPIDRDevice::~CPIDRDevice()
{
    ClearDRDevicePolicy();
    MediaDRDeviceStopThread();
}


bool CPIDRDevice::ShellExecute( std::string strCommand, std::string& strResult )
{
    FILE*  pPipe = NULL;
    char   czBuffer[1024];
    int  nLength=0;
    
    pPipe = popen( strCommand.c_str(), "r" );
    if(!pPipe) return false;
    
    // DEBUG_LOG( "Command=%s \n", strCommand.c_str() );
    while(!feof(pPipe))
    {
        memset(czBuffer, 0, sizeof(czBuffer) );
        if(fgets(czBuffer, 1024, pPipe) != NULL)
        {
            nLength = (int)strlen(czBuffer)-1;
            czBuffer[ nLength ] = '\0';
            strResult += czBuffer;
            // DEBUG_LOG( "Result=%s \n", strResult.c_str() );
        }
    }
    pclose( pPipe );
    return true;
}

bool CPIDRDevice::GetLoginUName( char* pczUName, uint32 nMaxUName )
{
    uint32 nLength = 0;
    std::string strCommand, strResult;
    
    if(!pczUName || nMaxUName <= 0)
    {
        return false;
    }
    strCommand = "/bin/ls -l /dev/console | /usr/bin/awk '{print $3}' ";
    ShellExecute( strCommand, strResult );
    
    nLength = (uint32)min(nMaxUName, strResult.length());
    strncpy( pczUName, strResult.c_str(), nLength );
    return true;
}


bool CPIDRDevice::GetLoginUID( char* pczUID, uint32 nMaxUID )
{
    uint32 nLength = 0;
    std::string strCommand, strResult;
    char czUName[MAX_UNAME];
    char czBuffer[MAX_BUF];
    
    if(!pczUID || nMaxUID <= 0)
    {
        return false;
    }
    memset( czUName, 0, sizeof(czUName) );
    GetLoginUName( czUName, sizeof(czUName) );
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "id -u %s", czUName );
    strCommand = czBuffer;
    ShellExecute( strCommand, strResult );
    
    nLength = (uint32)min(MAX_UNAME, strResult.length());
    strncpy( pczUID, strResult.c_str(), nLength );
    return true;
}

bool CPIDRDevice::IsActiveShareingd(void)
{
    uint32 nLength = 0, nPID=0;
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "sudo ps -ef | grep -v grep | grep sharingd | /usr/bin/awk '{print $2}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    nLength = (uint32)strResult.length();
    if(nLength <= 0)
    {
        return false;
    }
    
    nPID = atoi(strResult.c_str());
    if(nPID <= 1)
    {
        return false;
    }
    DEBUG_LOG("[%s] alive sharingd Length=%d nPID=%d, Result=%s \n", __FUNCTION__, nLength, nPID, strResult.c_str() );
    return true;
}


bool CPIDRDevice::IsActiveShareingDaemon(void)
{
    uint32 nLength = 0, nSvcSize=0;
    std::string strCommand, strResult;
    char czUID[MAX_UID];
    char czBuffer[MAX_BUF];
    
    if(false == IsActiveShareingd())
    {
        return false;
    }
    
    memset( czUID, 0, sizeof(czUID) );
    GetLoginUID( czUID, sizeof(czUID) );
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl asuser %s /bin/launchctl list | grep %s | awk '{print $3}' ", czUID, NAME_SHAREDAEMON );
    strCommand = czBuffer;
    
    ShellExecute( strCommand, strResult );
    nLength = (uint32)strResult.length();
    
    nSvcSize = strlen( NAME_SHAREDAEMON );
    if(nLength >= nSvcSize && 0 == strncasecmp( strResult.c_str(), NAME_SHAREDAEMON, nSvcSize ))
    {
        return true;
    }
    return false;
}


bool CPIDRDevice::SetProtectKillSharingd(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "sudo ps -ef | grep -v grep | grep sharingd | /usr/bin/awk '{print $2}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }

    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "sudo kill %s", strResult.c_str() );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    DEBUG_LOG("[%s] Command=%s \n", __FUNCTION__, strCommand.c_str() );
    return true;
}

bool CPIDRDevice::SetProtectSharingDaemon( bool bProtectAction )
{
    uint32 nLength = 0;
    std::string strCommand, strResult;
    char czUID[MAX_UID];
    char czBuffer[MAX_BUF];
    
    memset( czUID, 0, sizeof(czUID) );
    GetLoginUID( czUID, sizeof(czUID) );
    memset( czBuffer, 0, sizeof(czBuffer) );
    
    if(true == bProtectAction)
    {
        if(false == IsActiveShareingDaemon())
        {
            return false;
        }
        snprintf( czBuffer, sizeof(czBuffer), "sudo /bin/launchctl asuser %s /bin/launchctl unload -w %s ", czUID, PATH_SHAREDAEMON );
        strCommand = czBuffer;
        ShellExecute( strCommand, strResult );
        nLength = (uint32)strResult.length();
        
        SetProtectKillSharingd();
        
        m_bDenySharingDaemon = true;
        return true;
    }
    
    if(true == m_bDenySharingDaemon)
    {
        snprintf( czBuffer, sizeof(czBuffer), "sudo /bin/launchctl asuser %s /bin/launchctl load -w %s ", czUID, PATH_SHAREDAEMON );
        strCommand = czBuffer;
        ShellExecute( strCommand, strResult );
        nLength = (uint32)strResult.length();
        m_bDenySharingDaemon = false;
    }
    return false;
}


bool CPIDRDevice::IsActiveFileSharingSmbd(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl print-disabled system | grep com.apple.smbd | /usr/bin/awk '{print $3}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    if(strResult == "false")
    {
        DEBUG_LOG1( "Active=true.\n");
        return true;
    }
    DEBUG_LOG1( "Active=false.\n");
    return false;
}

bool CPIDRDevice::IsActiveFileSharingAppleFileServer(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl print-disabled system | grep com.apple.AppleFileServer | /usr/bin/awk '{print $3}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    if(strResult == "false")
    {
        DEBUG_LOG1("Active=true.\n");
        return true;
    }
    DEBUG_LOG1("Active=false.\n");
    return false;
}

bool CPIDRDevice::SetProtectFileSharing(bool bProtectAction, std::vector<std::string>& vecDenyList)
{
    bool bResult = false;
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    if(true == bProtectAction)
    {
        if(true == IsActiveFileSharingSmbd())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl unload -w %s/com.apple.smbd.plist", SLDAEMONS_DIR );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            vecDenyList.push_back("com.apple.smbd");
            bResult = true;
        }
        
        if(true == IsActiveFileSharingAppleFileServer())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl unload -w %s/com.apple.AppleFileServer.plist", SLDAEMONS_DIR );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            vecDenyList.push_back("com.apple.AppleFileServer");
            bResult = true;
        }
        return bResult;
    }
    else
    {
        if(false == IsActiveFileSharingSmbd())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl load -w %s/com.apple.smbd.plist", SLDAEMONS_DIR );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
        }
        
        if(false == IsActiveFileSharingAppleFileServer())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl load -w %s/com.apple.AppleFileServer.plist", SLDAEMONS_DIR );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
        }
    }
    return bResult;
}


bool CPIDRDevice::IsActiveScreenSharing(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "/bin/launchctl print-disabled system | grep com.apple.screensharing | /usr/bin/awk '{print $3}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    if(strResult == "false")
    {
        DEBUG_LOG1( "Active=true\n");
        return true;
    }
    DEBUG_LOG1("Active=false\n");
    return false;
}

bool CPIDRDevice::IsActiveRemoteLogin(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "sudo systemsetup -getremotelogin | /usr/bin/awk '{print $3}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    if(strResult == "On" || strResult == "on")
    {
        DEBUG_LOG1("Active=true.\n");
        return true;
    }
    DEBUG_LOG1( "Active=false.\n");
    return false;
}

bool CPIDRDevice::IsActiveRemoteAppleEvents(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "sudo systemsetup -getremoteappleevents | /usr/bin/awk '{print $4}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    if(strResult == "On" || strResult == "On")
    {
        DEBUG_LOG1("Active=true.\n");
        return true;
    }
    DEBUG_LOG1("Active=false.\n");
    return false;
}

bool CPIDRDevice::IsActiveRemoteManagement(void)
{
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    snprintf( czBuffer, sizeof(czBuffer), "sudo ps -ef | grep -v grep | grep ARDAgent | /usr/bin/awk '{print $2}' " );
    strCommand = czBuffer;
    if(false == ShellExecute(strCommand, strResult))
    {
        return false;
    }
    
    if(strResult.length() > 0)
    {
        DEBUG_LOG1( "ARDAgent-Active=true.\n");
        return true;
    }
    DEBUG_LOG1("ARDAgent-Active=false.\n");
    return false;
}


bool CPIDRDevice::SetProtectRemoteManagement(bool bProtectAction, std::vector<std::string>& vecDenyList)
{
    bool bResult =false;
    std::string strCommand, strResult;
    char czBuffer[MAX_BUF];
    
    if(true == bProtectAction)
    {
        if(true == IsActiveRemoteManagement() )
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "%s/kickstart -deactivate -configure -access -off -restart -agent", SCREMOTE_DIR );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            vecDenyList.push_back("com.apple.RemoteDesktopAgent");
            bResult = true;
        }
        
        if(true == IsActiveScreenSharing())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "sudo defaults write /var/db/launchd.db/com.apple.launchd/overrides.plist com.apple.screensharing -dict Disabled -bool true" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "sudo launchctl unload -w /System/Library/LaunchDaemons/com.apple.screensharing.plist" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            vecDenyList.push_back("com.apple.screensharing");
            bResult = true;
        }
        
        if(true == IsActiveRemoteLogin())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "sudo systemsetup -f -setremotelogin off" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            vecDenyList.push_back("RemoteLogin");
            bResult = true;
        }
        
        if(true == IsActiveRemoteAppleEvents())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "sudo systemsetup -f -setremoteappleevents off" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            vecDenyList.push_back("RemoteAppleEvents");
            bResult = true;
        }
        return bResult;
    }
    else
    {
        if(false == IsActiveRemoteManagement() )
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "%s/kickstart -activate -configure -access -on -privs -all -restart -agent", SCREMOTE_DIR );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
        }
        
        if(false == IsActiveScreenSharing())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "defaults write /var/db/launchd.db/com.apple.launchd/overrides.plist com.apple.screensharing -dict Disabled -bool false" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
            
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "launchctl load -w /System/Library/LaunchDaemons/com.apple.screensharing.plist" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
        }
        
        if(false == IsActiveRemoteLogin())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "systemsetup -f -setremotelogin on" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
        }
        
        if(false == IsActiveRemoteAppleEvents())
        {
            memset( czBuffer, 0, sizeof(czBuffer) );
            snprintf( czBuffer, sizeof(czBuffer), "systemsetup -f -setremoteappleevents on" );
            strCommand = czBuffer;
            ShellExecute( strCommand, strResult );
        }
    }
    return bResult;
}


void CPIDRDevice::FetchAppleCameraDeviceName( char* pczOutDeviceName, int nMaxOutDeviceName )
{
    if(!pczOutDeviceName) return;
}

bool CPIDRDevice::IsActiveAppleCamera()
{
    FILE* pDrvPipe = NULL;
    char czBuffer[NAME_MAX];
    char czSHCommand[NAME_MAX];
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    memset( czSHCommand, 0, sizeof(czSHCommand) );
    snprintf( czSHCommand, sizeof(czSHCommand), "/usr/sbin/kextstat -l -b %s", DRV_APPLE_CAMERA );
    pDrvPipe = popen( czSHCommand, "r" );
    if(!pDrvPipe) return false;
    
    if(fgets( czBuffer, sizeof(czBuffer), pDrvPipe ))
    {
        do
        {
            if(strstr( czBuffer, DRV_APPLE_CAMERA ))
            {
                pclose( pDrvPipe );
                return true;
            }
        } while( fgets(czBuffer, sizeof(czBuffer), pDrvPipe) );
    }
    pclose( pDrvPipe );
    return false;
}


bool CPIDRDevice::SetAppleCameraStatus( bool bEnable )
{
    FILE* pDrvPipe = NULL;
    char czSHCommand[NAME_MAX];
    
    memset( czSHCommand, 0, sizeof(czSHCommand) );
    if(bEnable)
    {
        snprintf( czSHCommand, sizeof(czSHCommand), "/sbin/kextload -q -b %s", DRV_APPLE_CAMERA );
    }
    else
    {
        snprintf( czSHCommand, sizeof(czSHCommand), "/sbin/kextunload -q -b %s", DRV_APPLE_CAMERA );
    }
    
    pDrvPipe = popen( czSHCommand, "r" );
    if(!pDrvPipe)
    {
        return false;
    }
    pclose( pDrvPipe );
    return true;
}


bool CPIDRDevice::MediaDRDeviceStopThread()
{
    if(m_pMediaRunLoop)
    {
        while(!CFRunLoopIsWaiting( m_pMediaRunLoop ))
        {
            usleep( 0 );
        }
        CFRunLoopStop( m_pMediaRunLoop );
        m_pMediaRunLoop = NULL;
    }
    
    if(m_pMediaThread)
    {
        pthread_join( m_pMediaThread, NULL );
        m_pMediaThread = NULL;
    }
    
    if(m_pNotifyPort)
    {
        IONotificationPortDestroy( m_pNotifyPort );
        m_pNotifyPort = NULL;
    }
    DEBUG_LOG1("End.");
    return true;
}

bool CPIDRDevice::MediaDRDeviceStartThread()
{
    if(0 != pthread_create( &m_pMediaThread, NULL, CPIDRDevice::ThreadMediaDRDevice, this ))
    {
        return false;
    }
    
    DEBUG_LOG1("Start.");
    return true;
}

void CPIDRDevice::ClearDRDevicePolicy()
{
    m_Rule.clear();
}

void CPIDRDevice::SetDRDevicePolicy(CPISecRule& rule)
{
    kern_return_t nResult = KERN_SUCCESS;
    mach_port_t   TempPort = 0;
    io_iterator_t MediaIter;
    IONotificationPortRef pNotifyPort = NULL;
    
    m_Rule = rule;
    
    DEBUG_LOG( "Type=%s, All=%d, Read=%d, Write=%d, Log=%d",
                m_Rule.virtualType.c_str(), IsDisableAll(), IsDisableRead(), IsDisableWrite(), IsEnableLog() );
    if(false == IsDisableAll() && false == IsDisableWrite())
    {
        DEBUG_LOG1( "ByPass." );
        return;
    }
    
    nResult = IOMasterPort( MACH_PORT_NULL, &TempPort );
    if(!TempPort) return;
    
    pNotifyPort = IONotificationPortCreate( kIOMasterPortDefault );
    if(!pNotifyPort) return;
    
    IOServiceAddMatchingNotification( pNotifyPort,
                                      kIOFirstMatchNotification,
                                      IOServiceMatching( kIOMediaClass ),
                                      (IOServiceMatchingCallback)MediaDRDeviceEnumerate,
                                      NULL, &MediaIter );
    MediaDRDeviceEnumerate( NULL, MediaIter );
    
    IOObjectRelease( MediaIter );
    IONotificationPortDestroy( pNotifyPort );
    mach_port_deallocate( mach_task_self(), TempPort );
}


void CPIDRDevice::MediaDRDeviceEnumerate( void* pRefCon, io_iterator_t Iter )
{
    kern_return_t nResult=0;
    io_object_t   Object;
    io_service_t  Service;
    CFStringRef   pClassName = NULL;
    io_name_t     czClassName;
    io_name_t     czDeviceName;
    io_string_t   czDevicePath;
    io_string_t   czRegEntryPath;
    
    DEBUG_LOG1( "Start." );
    while(( Object = IOIteratorNext(Iter) ))
    {
        memset( czClassName,  0, sizeof(czClassName) );
        memset( czDeviceName, 0, sizeof(czDeviceName) );
        memset( czDevicePath, 0, sizeof(czDevicePath) );
        memset( czRegEntryPath, 0, sizeof(czRegEntryPath) );
        
        Service = (io_service_t)Object;
        pClassName = IOObjectCopyClass( Object );
        CFStringGetCString( pClassName, czClassName, sizeof(czClassName), kCFStringEncodingUTF8 );
        IORegistryEntryGetName( Service, czDeviceName );
        nResult = GetDevicePath( Object, czDevicePath, sizeof(czDevicePath) );
        IORegistryEntryGetPath( Service, kIOServicePlane, czRegEntryPath  );
        
        if(true == IOObjectConformsTo(Object, kIOCDMediaClass) || true == IOObjectConformsTo(Object, kIODVDMediaClass))
        {
            bool  bDenyMedia = false;
            DRDeviceRef pDevice = NULL;
            DEBUG_LOG( "Class=%s, Name=%s, DevicePath=%s", czClassName, czDeviceName, czDevicePath );
            pDevice = GetDRDeviceFromRegistyEntryPath( czRegEntryPath );
            if(pDevice)
            {
                //
                // bDenyMedia = Request_AnyMedia_DRDeviceEjectMedia( pDevice, czDeviceName );
                //
                bDenyMedia = Request_WriteMedia_DRDeviceEjectMedia( pDevice, czDeviceName );
                DEBUG_LOG( "bDenyMedia == %d", bDenyMedia );
            }
        }
        if(pClassName) CFRelease( pClassName );
        if(Object) IOObjectRelease( Object );
    }
    DEBUG_LOG1( "End." );
}


bool CPIDRDevice::MediaDRDeviceExecuteRunLoop()
{
    mach_port_t   MasterPort = 0;
    io_iterator_t MediaAttachIter;
    CFRunLoopSourceRef pLoopSource = NULL;
    
    g_DRDevice.m_pMediaRunLoop = CFRunLoopGetCurrent();
    IOMasterPort( MACH_PORT_NULL, &MasterPort );
    if(!MasterPort) return false;
    
    g_DRDevice.m_pNotifyPort = IONotificationPortCreate( kIOMasterPortDefault );
    pLoopSource = IONotificationPortGetRunLoopSource( g_DRDevice.m_pNotifyPort );
    CFRunLoopAddSource( g_DRDevice.m_pMediaRunLoop, pLoopSource, kCFRunLoopDefaultMode );

    IOServiceAddMatchingNotification( g_DRDevice.m_pNotifyPort,
                                      kIOMatchedNotification,
                                      IOServiceMatching(kIOMediaClass),
                                      (IOServiceMatchingCallback)MediaDRDeviceAttach,
                                      NULL, &MediaAttachIter );
    MediaDRDeviceAttach( NULL, MediaAttachIter );
    
    CFRunLoopRun();
    
    IOObjectRelease( MediaAttachIter );
    mach_port_deallocate( mach_task_self(), MasterPort );
    return false;
}


void* CPIDRDevice::ThreadMediaDRDevice(void* pParam)
{
    CPIDRDevice* pDRDevice = NULL;
    pDRDevice = (CPIDRDevice*)pParam;
    if(pDRDevice)
    {
        pDRDevice->MediaDRDeviceExecuteRunLoop();
    }
    return NULL;
}


DRDeviceRef
CPIDRDevice::GetDRDeviceFromDevicePath( char* pczDevicePath )
{
    DRDeviceRef pDevice = NULL;
    CFStringRef pCFDevicePath = NULL;
    
    if(!pczDevicePath) return NULL;
    
    pCFDevicePath = CFStringCreateWithCString( kCFAllocatorDefault, pczDevicePath, kCFStringEncodingMacRoman );
    if(!pCFDevicePath) return NULL;
    
    pDevice = DRDeviceCopyDeviceForBSDName( pCFDevicePath );
    CFRelease( pCFDevicePath );
    return pDevice;
}


DRDeviceRef
CPIDRDevice::GetDRDeviceFromRegistyEntryPath( char* pczEntryPath )
{
    DRDeviceRef pDevice = NULL;
    CFStringRef pCFRegEntryPath = NULL;
    
    if(!pczEntryPath) return NULL;
    
    pCFRegEntryPath  = CFStringCreateWithCString( kCFAllocatorDefault, pczEntryPath, kCFStringEncodingMacRoman );
    if(!pCFRegEntryPath) return NULL;
    
    pDevice = DRDeviceCopyDeviceForIORegistryEntryPath( pCFRegEntryPath );
    CFRelease( pCFRegEntryPath );
    return pDevice;
}


bool CPIDRDevice::RequestDRDeviceTrayOpen( DRDeviceRef pDevice )
{
    OSStatus Status =0;
    bool bResult = false;
    if(!pDevice || false == DRDeviceIsValid( pDevice ))
    {
        return false;
    }
    Status = DRDeviceOpenTray( pDevice );
    return bResult;
}

bool CPIDRDevice::RequestDRDeviceTrayClose( DRDeviceRef pDevice )
{
    OSStatus Status =0;
    bool bResult = false;
    if(!pDevice || false == DRDeviceIsValid( pDevice ))
    {
        return false;
    }
    Status = DRDeviceCloseTray( pDevice );
    return bResult;
}


bool CPIDRDevice::Request_AnyMedia_DRDeviceEjectMedia( DRDeviceRef pDevice, std::string strDeviceName )
{
    bool bStateTrans = false;
    bool bBlankMedia = false;
    bool bEraseMedia = false;
    bool bAnyBurner = false;
    bool bAnyEraser = false;
    bool bCDBurner = false;
    bool bDVDBurner = false;
    OSStatus Status = 0;
    
    if(!pDevice) return false;
    if(false == g_DRDevice.IsDisableWrite())
    {
        DEBUG_LOG1( "IsDisableWrite() == false, Skipped." );
        return false;
    }
    
    bAnyBurner  = IsDRDeviceAnyBurner( pDevice );
    bAnyEraser  = IsDRDeviceAnyEraser( pDevice );
    bCDBurner   = IsDRDeviceCDBurner( pDevice );
    bDVDBurner  = IsDRDeviceDVDBurner( pDevice );
    if(bAnyBurner || bAnyEraser || bCDBurner || bDVDBurner)
    {
        bStateTrans = IsDeviceMediaStateTransition( pDevice );
        bBlankMedia = IsDeviceContainsBlankMedia( pDevice );
        bEraseMedia = IsDeviceContainsEraseMedia( pDevice );
        DEBUG_LOG( "bStateTrans=%d, bBlankMedia=%d, EraseMedia=%d", bStateTrans, bBlankMedia, bEraseMedia );
        
        DRDeviceAcquireExclusiveAccess( pDevice );
        Status = DRDeviceEjectMedia( pDevice );
        DRDeviceReleaseExclusiveAccess( pDevice );
        if(Status == 0 && true == g_DRDevice.IsEnableLog())
        {
            CPIDeviceController controller( g_DRDevice.m_Rule );
            if(strDeviceName.empty())
            {
                controller.appendDeviceLog( "CD/DVD"  );
            }
            else
            {
                controller.appendDeviceLog( strDeviceName );
            }
        }
        DEBUG_LOG( "return true, Status=%d", Status );
        return true;
    }
    DEBUG_LOG( "return false, Status=%d", Status );
    return false;
}


bool CPIDRDevice::Request_WriteMedia_DRDeviceEjectMedia( DRDeviceRef pDevice, std::string strDeviceName )
{
    bool bStateTrans = false;
    bool bBlankMedia = false;
    bool bEraseMedia = false;
    bool bAnyBurner = false;
    bool bAnyEraser = false;
    bool bCDBurner = false;
    bool bDVDBurner = false;
    OSStatus Status = 0;
    
    if(!pDevice) return false;
    if(false == g_DRDevice.IsDisableWrite())
    {
        DEBUG_LOG1( "IsDisableWrite() == false, Skipped." );
        return false;
    }

    bAnyBurner  = IsDRDeviceAnyBurner( pDevice );
    bAnyEraser  = IsDRDeviceAnyEraser( pDevice );
    bCDBurner   = IsDRDeviceCDBurner( pDevice );
    bDVDBurner  = IsDRDeviceDVDBurner( pDevice );
   
    DEBUG_LOG( "bBurner=%d, bEraser=%d, bCDBurner=%d, bDVDBurner=%d", bAnyBurner, bAnyEraser, bCDBurner, bDVDBurner );
    if(bAnyBurner || bAnyEraser || bCDBurner || bDVDBurner)
    {
        bStateTrans = IsDeviceMediaStateTransition( pDevice );
        DEBUG_LOG( "bStateTrans=%d \n", bStateTrans );
        
        bBlankMedia = IsDeviceContainsBlankMedia( pDevice );
        bEraseMedia = IsDeviceContainsEraseMedia( pDevice );
        DEBUG_LOG( "bBlank=%d, bErase=%d", bBlankMedia, bEraseMedia );
        if(true == bBlankMedia || true == bEraseMedia)
        {
            DRDeviceAcquireExclusiveAccess( pDevice );
            Status = DRDeviceEjectMedia( pDevice );
            DRDeviceReleaseExclusiveAccess( pDevice );
            if(Status == 0 && true == g_DRDevice.IsEnableLog())
            {
                CPIDeviceController controller( g_DRDevice.m_Rule );
                if(strDeviceName.empty())
                {
                    controller.appendDeviceLog( "CD/DVD"  );
                }
                else
                {
                    controller.appendDeviceLog( strDeviceName );
                }
            }
            DEBUG_LOG( "return true, Status=%d", Status );
            return true;
        }
    }
    DEBUG_LOG( "return false, Status=%d", Status );
    return false;
}


void CPIDRDevice::MediaDRDeviceStatus( void* pRefCon, io_service_t service, natural_t nMsgType, void* pMsgArg )
{
    kern_return_t nResult=0;
    CFStringRef   pClassName = NULL;
    io_name_t     czClassName;
    io_name_t     czDeviceName;
    io_string_t   czDevicePath;
    io_string_t   czRegEntryPath;
    MediaData*    pMediaData = NULL;
    
    pMediaData = (MediaData*)pRefCon;
    memset( czClassName,  0, sizeof(czClassName) );
    memset( czDeviceName, 0, sizeof(czDeviceName) );
    memset( czDevicePath, 0, sizeof(czDevicePath) );
    memset( czRegEntryPath, 0, sizeof(czRegEntryPath) );
    
    pClassName = IOObjectCopyClass( service );
    if(pClassName)
    {
        CFStringGetCString( pClassName, czClassName, sizeof(czClassName), kCFStringEncodingUTF8 );
        CFRelease(pClassName);
    }
    
    IORegistryEntryGetName( service, czDeviceName );
    nResult = GetDevicePath( service, czDevicePath, sizeof(czDevicePath) );
    IORegistryEntryGetPath( service, kIOServicePlane, czRegEntryPath  );
    
    if(nMsgType == kIOMessageServiceIsTerminated)
    {
        DEBUG_LOG( "kIOMessageServiceIsTerminated, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
        if(pMediaData)
        {
            IOObjectRelease( pMediaData->Notification );
            IOObjectRelease( pMediaData->Service );
            free( pMediaData );
        }
    }
    else if(nMsgType == kIOMessageServiceBusyStateChange)
    {
        DEBUG_LOG( "kIOMessageServiceBusyStateChange, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageServiceIsSuspended)
    {
        DEBUG_LOG("kIOMessageServiceIsSuspended, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageServiceIsResumed)
    {
        DEBUG_LOG( "kIOMessageServiceIsResumed, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageServiceIsRequestingClose)
    {
        DEBUG_LOG( "kIOMessageServiceIsRequestingClose, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageServiceIsAttemptingOpen)
    {
        DEBUG_LOG( "kIOMessageServiceIsAttemptingOpen, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageServiceWasClosed)
    {
        DEBUG_LOG( "kIOMessageServiceWasClosed, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageConsoleSecurityChange)
    {
        DEBUG_LOG( "kIOMessageConsoleSecurityChange, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageServicePropertyChange)
    {
        DEBUG_LOG( "kIOMessageServicePropertyChange, Type=%08x, Class=%s, Name=%s \n", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageCopyClientID)
    {
        DEBUG_LOG( "kIOMessageCopyClientID, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageSystemCapabilityChange)
    {
        DEBUG_LOG( "kIOMessageSystemCapabilityChange, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else if(nMsgType == kIOMessageDeviceSignaledWakeup)
    {
        DEBUG_LOG( "kIOMessageDeviceSignaledWakeup, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
    else
    {
        DEBUG_LOG( "Etc, Type=%08x, Class=%s, Name=%s", nMsgType, czClassName, czDeviceName );
    }
}


void CPIDRDevice::MediaDRDeviceAttach( void* pRefCon, io_iterator_t Iter )
{
    kern_return_t nResult=0;
    io_object_t   object;
    io_service_t  service;
    CFStringRef   pClassName = NULL;
    io_name_t     czClassName;
    io_name_t     czDeviceName;
    io_string_t   czDevicePath;
    io_string_t   czRegEntryPath;
    
    DEBUG_LOG1( "Start." );
    while(( object = IOIteratorNext(Iter) ))
    {
        memset( czClassName,  0, sizeof(czClassName) );
        memset( czDeviceName, 0, sizeof(czDeviceName) );
        memset( czDevicePath, 0, sizeof(czDevicePath) );
        memset( czRegEntryPath, 0, sizeof(czRegEntryPath) );
       
        service = (io_service_t)object;
        pClassName = IOObjectCopyClass( object );
        if(pClassName)
        {
            CFStringGetCString( pClassName, czClassName, sizeof(czClassName), kCFStringEncodingUTF8 );
        }
        IORegistryEntryGetName( service, czDeviceName );
        nResult = GetDevicePath( object, czDevicePath, sizeof(czDevicePath) );
        IORegistryEntryGetPath( service, kIOServicePlane, czRegEntryPath  );

        if(true == IOObjectConformsTo(object, kIOCDMediaClass) || true == IOObjectConformsTo(object, kIODVDMediaClass))
        {
            bool bDenyDRDevice = false;
            DRDeviceRef pDevice = NULL;
            MediaData* pMediaGeneral = NULL;
            
            pMediaGeneral = (MediaData*)malloc( sizeof(MediaData) );
            if(pMediaGeneral)
            {
                memset( pMediaGeneral, 0, sizeof(MediaData) );
                pMediaGeneral->Service = service;
                IOServiceAddInterestNotification( g_DRDevice.m_pNotifyPort,
                                                  service,
                                                  kIOGeneralInterest,
                                                  MediaDRDeviceStatus,
                                                  pMediaGeneral,
                                                  &pMediaGeneral->Notification );
            }
            
            DEBUG_LOG( "Class=%s, Name=%s, DevicePath=%s", czClassName, czDeviceName, czDevicePath );
            pDevice = GetDRDeviceFromRegistyEntryPath( czRegEntryPath );
            if(pDevice)
            {
                // bDenyDRDevice = Request_AnyMedia_DRDeviceEjectMedia( pDevice, czDeviceName );
                bDenyDRDevice = Request_WriteMedia_DRDeviceEjectMedia( pDevice, czDeviceName );
                DEBUG_LOG( "bDenyDRDevice == %d", bDenyDRDevice );
            }
            
            if(pClassName)
            {
                CFRelease( pClassName );
                pClassName = NULL;
            }
            continue;
        }
        
        if(pClassName) CFRelease( pClassName );
        if(object) IOObjectRelease( object );
    }
    DEBUG_LOG1( "End." );
}


kern_return_t
CPIDRDevice::GetDevicePath( io_object_t object, char* pczPath, size_t nMaxLength )
{
    kern_return_t nResult = KERN_SUCCESS;
    CFTypeRef pPathCFString = NULL;
    
    if(!pczPath) return KERN_INVALID_VALUE;
    
    pPathCFString = IORegistryEntryCreateCFProperty( object, CFSTR(kIOBSDNameKey), kCFAllocatorDefault, 0 );
    if(pPathCFString)
    {
        strcpy( pczPath, "/dev/" );
        size_t nPathLength = strlen( pczPath );
        
        if(CFStringGetCString( (const __CFString *)pPathCFString, pczPath + nPathLength, nMaxLength - nPathLength, kCFStringEncodingASCII))
        {
            nResult = KERN_SUCCESS;
        }
        CFRelease( pPathCFString );
    }
    return nResult;
}


bool CPIDRDevice::IsDRDeviceAnyBurner(DRDeviceRef pDevice)
{
    bool bResult = 0;
    CFDictionaryRef pCopyInfo = DRDeviceCopyInfo(pDevice);
    CFDictionaryRef pCaps = (CFDictionaryRef)CFDictionaryGetValue( pCopyInfo, kDRDeviceWriteCapabilitiesKey );
    bResult = ((CFDictionaryGetValue(pCaps, kDRDeviceCanWriteCDRKey  ) == kCFBooleanTrue) ||
               (CFDictionaryGetValue(pCaps, kDRDeviceCanWriteCDRWKey ) == kCFBooleanTrue) ||
               (CFDictionaryGetValue(pCaps, kDRDeviceCanWriteDVDRKey ) == kCFBooleanTrue) ||
               (CFDictionaryGetValue(pCaps, kDRDeviceCanWriteDVDRWKey) == kCFBooleanTrue) );
    CFRelease(pCopyInfo);
    return bResult;
}


bool CPIDRDevice::IsDRDeviceAnyEraser(DRDeviceRef pDevice)
{
    bool bResult = 0;
    CFDictionaryRef pCopyInfo = DRDeviceCopyInfo(pDevice);
    CFDictionaryRef pCaps = (CFDictionaryRef)CFDictionaryGetValue( pCopyInfo, kDRDeviceWriteCapabilitiesKey );
    bResult = ((CFDictionaryGetValue( pCaps, kDRDeviceCanWriteCDRWKey ) == kCFBooleanTrue) ||
               (CFDictionaryGetValue( pCaps, kDRDeviceCanWriteDVDRWKey) == kCFBooleanTrue) );
    
    CFRelease(pCopyInfo);
    return bResult;
}


bool CPIDRDevice::IsDRDeviceCDBurner(DRDeviceRef pDevice)
{
    bool bResult = 0;
    CFDictionaryRef pCopyInfo = DRDeviceCopyInfo(pDevice);
    CFDictionaryRef pCaps = (CFDictionaryRef)CFDictionaryGetValue( pCopyInfo, kDRDeviceWriteCapabilitiesKey );
    bResult = ((CFDictionaryGetValue( pCaps, kDRDeviceCanWriteCDRKey) == kCFBooleanTrue) ||
               (CFDictionaryGetValue( pCaps, kDRDeviceCanWriteCDRWKey) == kCFBooleanTrue) );
    CFRelease(pCopyInfo);
    return bResult;
}


bool CPIDRDevice::IsDRDeviceDVDBurner(DRDeviceRef pDevice)
{
    bool bResult = 0;
    CFDictionaryRef pCopyInfo = DRDeviceCopyInfo(pDevice);
    CFDictionaryRef pCaps = (CFDictionaryRef)CFDictionaryGetValue( pCopyInfo, kDRDeviceWriteCapabilitiesKey);
    bResult = ((CFDictionaryGetValue( pCaps, kDRDeviceCanWriteDVDRKey) == kCFBooleanTrue) ||
               (CFDictionaryGetValue( pCaps, kDRDeviceCanWriteDVDRWKey) == kCFBooleanTrue) );
    CFRelease(pCopyInfo);
    return bResult;
}

bool CPIDRDevice::IsDeviceBecomingReady( DRDeviceRef pDevice )
{
    bool  bResult = false;
    CFDictionaryRef pDeviceStatus = DRDeviceCopyStatus( pDevice );
    CFStringRef pMediaState = (CFStringRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaStateKey );
    
    bResult = (pMediaState && CFEqual( pMediaState, kDRDeviceMediaStateInTransition )) ? true:false;
    CFRelease( pDeviceStatus );
    return bResult;
}


bool CPIDRDevice::IsDeviceMediaStateTransition( DRDeviceRef pDevice )
{
    bool  bResult = false;
    CFDictionaryRef pDeviceStatus = DRDeviceCopyStatus( pDevice );
    CFStringRef pMediaState = (CFStringRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaStateKey );
    
    bResult = (pMediaState && CFEqual( pMediaState, kDRDeviceMediaStateInTransition )) ? true:false;
    CFRelease( pDeviceStatus );
    DEBUG_LOG( "bStateTrans == %d", bResult );
    if(true == bResult)
    {
        WritableMediaCheckInDevice( pDevice );
    }
    return bResult;
}


bool CPIDRDevice::IsMediaIsReserved( DRDeviceRef pDevice )
{
    bool bResult = false;
    CFDictionaryRef pDeviceStatus = DRDeviceCopyStatus( pDevice );
    CFStringRef  pMediaState = (CFStringRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaStateKey );
    
    // Check to see if there's media in the device
    if(pMediaState && CFEqual( pMediaState, kDRDeviceMediaStateMediaPresent ))
    {
        CFDictionaryRef pMediaInfo = (CFDictionaryRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaInfoKey );
        CFBooleanRef  pReserved = (CFBooleanRef)CFDictionaryGetValue( pMediaInfo, kDRDeviceMediaIsReservedKey );
        
        // There's media, but do we have the reservation?
        if(pReserved && CFBooleanGetValue( pReserved ))
        {
            bResult = true;
        }
    }
    CFRelease( pDeviceStatus );
    return bResult;
}

bool CPIDRDevice::IsDeviceContainsEraseMedia(DRDeviceRef pDevice)
{
    bool bResult = false;
    CFDictionaryRef pDeviceStatus = DRDeviceCopyStatus( pDevice );
    CFStringRef pMediaState = (CFStringRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaStateKey );
    
    // Check to see if there's media in the device
    if(pMediaState && CFEqual( pMediaState, kDRDeviceMediaStateMediaPresent ))
    {
        CFDictionaryRef pMediaInfo = (CFDictionaryRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaInfoKey );
        CFBooleanRef pErasable = (CFBooleanRef)CFDictionaryGetValue( pMediaInfo, kDRDeviceMediaIsErasableKey );
        
        // There's media, but is it blank and writable?
        if(pErasable && CFBooleanGetValue( pErasable ))
        {
            bResult = true;
        }
        DEBUG_LOG( "kDRDeviceMediaIsErasableKey == %d", CFBooleanGetValue( pErasable ) );
    }
    else
    {
        DEBUG_LOG( "bResult == %d", bResult );
    }
    CFRelease( pDeviceStatus );
    return bResult;
}

bool CPIDRDevice::IsDeviceContainsBlankMedia( DRDeviceRef pDevice )
{
    bool bReturn = false;
    CFDictionaryRef pDeviceStatus = DRDeviceCopyStatus( pDevice );
    CFStringRef  pMediaState = (CFStringRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaStateKey );
    
    // Check to see if there's media in the device
    if(pMediaState && CFEqual( pMediaState, kDRDeviceMediaStateMediaPresent ))
    {
        CFDictionaryRef pMediaInfo = (CFDictionaryRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaInfoKey );
        CFBooleanRef  pBlank = (CFBooleanRef)CFDictionaryGetValue( pMediaInfo, kDRDeviceMediaIsBlankKey );
        CFBooleanRef  pAppendable = (CFBooleanRef)CFDictionaryGetValue( pMediaInfo, kDRDeviceMediaIsAppendableKey );
        
        // There's media, but is it blank and writable ?
        if(pBlank && CFBooleanGetValue(pBlank) && pAppendable && CFBooleanGetValue(pAppendable))
        {
            bReturn = true;
        }
        DEBUG_LOG( "kDRDeviceMediaIsBlankKey == %d, kDRDeviceMediaIsAppendableKey == %d",
                   CFBooleanGetValue(pBlank), CFBooleanGetValue(pAppendable) );
    }
    else
    {
        DEBUG_LOG( "Result == %d", bReturn );
    }
    CFRelease( pDeviceStatus );
    return bReturn;
}


bool CPIDRDevice::IsDeviceMediaStatePresent( DRDeviceRef pDevice )
{
    CFDictionaryRef pDeviceStatus = DRDeviceCopyStatus( pDevice );
    CFStringRef  pMediaState = (CFStringRef)CFDictionaryGetValue( pDeviceStatus, kDRDeviceMediaStateKey );
    
    // Check to see if there's media in the device
    if(pMediaState && CFEqual( pMediaState, kDRDeviceMediaStateMediaPresent ))
    {
        CFRelease( pDeviceStatus );
        DEBUG_LOG1( "true == kDRDeviceMediaStateMediaPresent." );
        return true;
    }
    CFRelease( pDeviceStatus );
    DEBUG_LOG1( "false == kDRDeviceMediaStateMediaPresent." );
    return false;
}

//
// WaitForWritableMedia
//
void CPIDRDevice::WaitForWritableMedia( DRNotificationCenterRef pCenter, void* pObserver, CFStringRef pStrName, DRTypeRef pObject, CFDictionaryRef pInfo )
{
    DRDeviceRef pDevice = NULL;
    pDevice = (DRDeviceRef)pObject;
    if(!pDevice || !pStrName) return;
    
    if(CFEqual( pStrName, kDRDeviceDisappearedNotification ) || !DRDeviceIsValid( pDevice ))
    {
        DEBUG_LOG1( "[Failed] kDRDeviceDisappearedNotification, false == DRDeviceIsValid(), CFRunLoopStop()." );
        CFRunLoopStop(CFRunLoopGetCurrent());
        return;
    }
    
    if(CFEqual( pStrName, kDRDeviceStatusChangedNotification) && IsDeviceMediaStatePresent( pDevice ))
    {
        DEBUG_LOG1( "[Success] kDRDeviceStatusChangedNotification, true == IsDeviceMediaStatePresent, CFRunLoopStop()");
        CFRunLoopStop(CFRunLoopGetCurrent());
    }
}


void CPIDRDevice::WritableMediaCheckInDevice( DRDeviceRef pDevice )
{
    CFRunLoopSourceRef  pSource = NULL;
    DRNotificationCenterRef pCenter = NULL;
    
    if(true == IsDeviceContainsBlankMedia(pDevice) || true == IsDeviceContainsEraseMedia(pDevice))
    {
        DEBUG_LOG1( "true == IsDeviceContainsBlankMedia(pDevice) || true == IsDeviceContainsEraseMedia(pDevice)" );
        return;
    }
    
    if(!IsDeviceBecomingReady( pDevice ))
    {
        DEBUG_LOG1( "false == IsDeviceBecomingReady(pDevice), DRDeviceEjectMedia(pDevice)" );
        DRDeviceEjectMedia( pDevice );
    }
    
    pCenter = DRNotificationCenterCreate();
    pSource = DRNotificationCenterCreateRunLoopSource( pCenter );
    CFRunLoopAddSource(CFRunLoopGetCurrent(), pSource, kCFRunLoopCommonModes);
    DRNotificationCenterAddObserver( pCenter, NULL, WaitForWritableMedia, NULL, pDevice );
    
    DEBUG_LOG1( "CFRunLoopRun() Start." );
    CFRunLoopRun();
    CFRunLoopSourceInvalidate( pSource );
    DEBUG_LOG1( "CFRunLoopRun() End." );
    
    if(pCenter) CFRelease( pCenter );
    if(pSource) CFRelease( pSource );
}


bool CPIDRDevice::IsRNDIS_DeviceName( char* pServiceName )
{
    bool bRemoteNDIS = false;
    kern_return_t nResult = KERN_SUCCESS;
    mach_port_t   TempPort = 0;
    io_iterator_t MediaIter;
    IONotificationPortRef pNotifyPort = NULL;
    
    nResult = IOMasterPort( MACH_PORT_NULL, &TempPort );
    if(!TempPort) return false;
    
    pNotifyPort = IONotificationPortCreate( kIOMasterPortDefault );
    if(!pNotifyPort) return false;
    
    // kIOUSBDeviceClassName
    IOServiceAddMatchingNotification( pNotifyPort,
                                     kIOFirstMatchNotification,
                                     IOServiceMatching( kIOUSBInterfaceClassName ),
                                     (IOServiceMatchingCallback)RNDISDeviceEnumerate_Dummy,
                                     NULL, &MediaIter );
    bRemoteNDIS = RNDISDeviceEnumerate( NULL, MediaIter, pServiceName );
    
    IOObjectRelease( MediaIter );
    IONotificationPortDestroy( pNotifyPort );
    mach_port_deallocate( mach_task_self(), TempPort );
    
    return bRemoteNDIS;
}


void CPIDRDevice::RNDISDeviceEnumerate_Dummy( void* pRefCon, io_iterator_t Iter )
{
}


bool CPIDRDevice::RNDISDeviceEnumerate( void* pRefCon, io_iterator_t Iter, char* pServiceName )
{
    kern_return_t nResult=0;
    io_object_t   Object;
    CFStringRef   pClassName = NULL;
    io_name_t     czClassName;
    io_name_t     czDeviceName;
    io_string_t   czDevicePath;
    io_string_t   czRegEntryPath;
    io_name_t     czBuffer;
    uint32_t      nLength=0;
    uint32_t      nIfClass=0, nIfSubClass = 0, nIfProto=0;
    io_name_t     czChildName;
    io_registry_entry_t Parent;
    
    if( !pServiceName ) return false;
    
    while(( Object = IOIteratorNext(Iter) ))
    {
        memset( czClassName,  0, sizeof(czClassName) );
        memset( czDeviceName, 0, sizeof(czDeviceName) );
        memset( czDevicePath, 0, sizeof(czDevicePath) );
        memset( czRegEntryPath, 0, sizeof(czRegEntryPath) );
        
        nResult = IORegistryEntryGetParentEntry( Object, kIOServicePlane, &Parent );
        
        pClassName = IOObjectCopyClass( Parent );
        CFStringGetCString( pClassName, czClassName, sizeof(czClassName), kCFStringEncodingUTF8 );
        IORegistryEntryGetName( Parent, czDeviceName );
        nResult = GetDevicePath( Parent, czDevicePath, sizeof(czDevicePath) );
        IORegistryEntryGetPath( Parent, kIOServicePlane, czRegEntryPath  );
        
        CFTypeRef cTypeRef;
        memset( czBuffer, 0, sizeof(czBuffer) );
        nLength = sizeof(czBuffer);
        uint32_t nClass=0, nSubClass=0, nProto=0;
        
        memset( czChildName, 0, sizeof(czChildName) );
        IORegistryEntryGetName( Object, czChildName );
        
        cTypeRef = IORegistryEntryCreateCFProperty( Object, CFSTR("bInterfaceClass"), kCFAllocatorDefault, 0 );
        if(cTypeRef)
        {
            CFNumberGetValue( (CFNumberRef)cTypeRef, kCFNumberLongType, &nIfClass );
            CFRelease( cTypeRef );
        }
        nClass = nIfClass;
        
        cTypeRef = IORegistryEntryCreateCFProperty( Object, CFSTR("bInterfaceSubClass"), kCFAllocatorDefault, 0 );
        if(cTypeRef)
        {
            CFNumberGetValue( (CFNumberRef)cTypeRef, kCFNumberLongType, &nIfSubClass );
            CFRelease( cTypeRef );
        }
        nSubClass = nIfSubClass;
        
        cTypeRef = IORegistryEntryCreateCFProperty( Object, CFSTR("bInterfaceProtocol"), kCFAllocatorDefault, 0 );
        if(cTypeRef)
        {
            CFNumberGetValue( (CFNumberRef)cTypeRef, kCFNumberLongType, &nIfProto );
            CFRelease( cTypeRef );
        }
        nProto = nIfProto;
        
        // DEBUG_LOG( "[%s] DeviceName=\"%s\", czChildName=\"%s\", Class=0x%02x, SubClass=0x%02x, Proto=0x%02x \n", __FUNCTION__, czDeviceName, czChildName, nClass, nSubClass, nProto );
        
        if(0 == strncasecmp( czDeviceName, pServiceName, strlen(pServiceName) ))
        {
            if(0xE0 == nClass && 0x01 == nSubClass && 0x03 == nProto)
            {   // RNDIS
                return true;
            }
        }
        
        if(pClassName) CFRelease( pClassName );
        if(Object) IOObjectRelease( Object );
    }
    return false;
}



bool CPIDRDevice::FetchWWAN_SCNetworkService( std::vector<std::string>& VectorNetworkService )
{
    Boolean bEnabled =false;
    CFIndex nPos=0, nMaxPos=0;
    CFArrayRef pServiceArray = NULL;
    SCPreferencesRef pPreference = NULL;
    SCNetworkServiceRef pNetworkService = NULL;
    CFStringRef pCFServiceName = NULL;
    SCNetworkInterfaceRef pIFace = NULL;
    CFStringRef pCFType = NULL;
    char   czServiceName[ 260 ];
    char   czTypeName[260];
    std::string strCommand;
    std::string strResult;
    
    pPreference = SCPreferencesCreate( kCFAllocatorDefault, CFSTR("PRG"), NULL );
    if(!pPreference) return false;
    
    pServiceArray = SCNetworkServiceCopyAll( pPreference );
    nMaxPos = CFArrayGetCount( pServiceArray );
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        pNetworkService = (SCNetworkServiceRef)CFArrayGetValueAtIndex( pServiceArray, nPos );
        if( !pNetworkService ) continue;
    
        pCFServiceName = SCNetworkServiceGetName( pNetworkService );
        memset( czServiceName, 0, sizeof(czServiceName) );
        CFStringGetCString( pCFServiceName, czServiceName, sizeof(czServiceName), kCFStringEncodingMacRoman );
        pIFace = (SCNetworkInterfaceRef)SCNetworkServiceGetInterface( pNetworkService );
        if(!pIFace) continue;
        
        pCFType = SCNetworkInterfaceGetInterfaceType( pIFace );
        CFStringGetCString( pCFType, czTypeName, sizeof(czTypeName), kCFStringEncodingMacRoman );
        
        bEnabled = SCNetworkServiceGetEnabled( pNetworkService );
        DEBUG_LOG( "[%s] %02ld. bEnabled=%d, Type=%s, Service=%s \n", __FUNCTION__, nPos, bEnabled, czTypeName, czServiceName );
        if(false == bEnabled)
        {
            continue;
        }
        
        if(CFEqual( pCFType, kSCNetworkInterfaceTypeEthernet ))
        {
            if(strstr(czServiceName, "Bluetooth") || true == g_DRDevice.IsRNDIS_DeviceName( czServiceName ))
            {
                std::string strServiceName;
                // DEBUG_LOG( "[%s] nPos=%ld --> RNDIS --> ServiceName=%s \n", __FUNCTION__, nPos, czServiceName );
                strServiceName = czServiceName;
                VectorNetworkService.push_back( strServiceName );
            }
        }
        else if(CFEqual( pCFType, kSCNetworkInterfaceTypeIEEE80211 ))
        {
        }
        else if(CFEqual( pCFType, kSCNetworkInterfaceTypeBluetooth ))
        {
        }
    }
    CFRelease( pServiceArray );
    return true;
}




