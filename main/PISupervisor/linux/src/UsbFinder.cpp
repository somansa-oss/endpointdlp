#include "UsbFinder.h"
#include "Markup.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <sys/param.h>

#include <iostream>
//#include "LogWriter.h"

using namespace std;

USBInfo::USBInfo()
{
    USBInfoInit();
}

USBInfo::~USBInfo()
{
}

void USBInfo::USBInfoInit()
{
    m_nVendorID  = 0;
    m_nProductID = 0;
    m_nDeviceType = DEVICE_TYPE_UNKNOWN;
    memset( m_czVendor,  0, sizeof(m_czVendor)  );
    memset( m_czProduct, 0, sizeof(m_czProduct) );
    memset( m_czSerial,  0, sizeof(m_czSerial)  );
    memset( m_czVolumePath,  0, sizeof(m_czVolumePath) );
    memset( m_czMediaID, 0, sizeof(m_czMediaID) );
    
    m_nIfaceCount = 0;
    memset( &m_Iface, 0, sizeof(m_Iface) );
}

///////////////////////////////////////////////////////////////////////////////////////////

UsbFinder::UsbFinder()
{
    UsbDeviceInit();
}

UsbFinder::~UsbFinder()
{
}

void UsbFinder::UsbDeviceInit()
{
    m_nCount = 0;
    memset( m_UsbInfo, 0, sizeof(m_UsbInfo) );
    
    EnumerateUsbDevice();

}

bool
UsbFinder::EnumerateUsbDevice()
{
#ifdef _FIXME_

    kern_return_t nRet = 0;
    io_iterator_t IterAttach = 0;
    mach_port_t   MasterPort = 0;
    CFMutableDictionaryRef pMatchDict  = NULL;
    IONotificationPortRef  pNotifyPort = NULL;
    CFRunLoopSourceRef     pRunLoopSource = NULL;
    
    pMatchDict = IOServiceMatching( "IOUSBDevice" );
    if(!pMatchDict) return false;
    
    IOMasterPort(MACH_PORT_NULL, &MasterPort);
    if(!MasterPort)
    {
        CFRelease( pMatchDict );
        return false;
    }
    
    pNotifyPort = IONotificationPortCreate( MasterPort );
    pRunLoopSource = IONotificationPortGetRunLoopSource( pNotifyPort );
    CFRunLoopAddSource( CFRunLoopGetCurrent(), pRunLoopSource, kCFRunLoopDefaultMode );
    
    pMatchDict = (CFMutableDictionaryRef)CFRetain( pMatchDict );
    pMatchDict = (CFMutableDictionaryRef)CFRetain( pMatchDict );
    pMatchDict = (CFMutableDictionaryRef)CFRetain( pMatchDict );
    
    nRet = IOServiceAddMatchingNotification( pNotifyPort, kIOFirstMatchNotification,
                                             pMatchDict, UsbFinder::UsbDeviceEnumerate, NULL, &IterAttach );
    if(nRet)
    {
        mach_port_deallocate(mach_task_self(), MasterPort);
        return false;
    }
    
    UsbDeviceEnumerate( this, IterAttach );
    
    mach_port_deallocate(mach_task_self(), MasterPort);

#endif

    return true;
}

#ifdef _FIXME_
char*
UsbFinder::CFURLRefToCString(CFURLRef pCFUrl)
{
    static char czFilePath[1024];
    char* pczPos = NULL;
    char* pczEnd = NULL;
    int   nLength = 0, nPos=0;
    
    if(!pCFUrl) return NULL;
    
    memset( czFilePath, 0, sizeof(czFilePath) );
    CFStringGetCString( CFURLGetString( pCFUrl ), czFilePath, MAXPATHLEN, kCFStringEncodingUTF8 );
    
    // End '/' eliminate
    nLength = (int)strlen( czFilePath );
    pczEnd = strrchr( czFilePath, '/' );
    if(pczEnd)
    {
        nPos = (int)(pczEnd - czFilePath);
        if(nPos == (nLength-1))
        {
            *pczEnd = '\0';
        }
    }
    
    pczPos = strstr( czFilePath, "/Volumes");
    if(pczPos)
    {
        return pczPos;
    }
    return czFilePath;
}


char*
UsbFinder::CFTypeRefToCString(CFTypeRef pCFTypeStr)
{
    char*  pczPos = NULL;
    static char czFilePath[1024];
    
    if(!pCFTypeStr) return NULL;
    
    memset( czFilePath, 0, sizeof(czFilePath) );
    CFStringGetCString( CFCopyDescription( pCFTypeStr ), czFilePath, MAXPATHLEN, kCFStringEncodingUTF8 );
    
    pczPos = czFilePath;
    return pczPos;
}

char*
UsbFinder::CFStringRefToCString(CFStringRef pCFString)
{
    static char czBuffer[1024];
    
    if(!pCFString)
    {
        return NULL;
    }
    
    memset( czBuffer, 0, sizeof(czBuffer) );
    CFStringGetCString( pCFString, czBuffer, MAXPATHLEN, kCFStringEncodingUTF8 );
    return &czBuffer[0];
}


bool UsbFinder::UsbDevice_Parsing(char* pczBsdNamePos, io_service_t UsbDevice, UsbFinder* pUsbInfo)
{
    CFDictionaryRef pDict       = NULL;
    DASessionRef    pDASession  = NULL;
    DADiskRef       pDADisk     = NULL;
    CFURLRef        pCFUrl      = NULL;
    CFStringRef     pCFString   = NULL;
    char*           pczVolume   = NULL;
    char*           pczVolPath  = NULL;
    
    if(!pczBsdNamePos || !pUsbInfo)
    {
        return false;
    }

    pDASession = DASessionCreate( kCFAllocatorDefault );
    if(!pDASession) return false;
    
    pDADisk = DADiskCreateFromBSDName( kCFAllocatorDefault, pDASession, pczBsdNamePos );
    if(!pDADisk)
    {
        if(pDASession) CFRelease( pDASession );
        return false;
    }
    
    pDict = DADiskCopyDescription( pDADisk );
    if(!pDict)
    {
        if(pDADisk) CFRelease( pDADisk );
        if(pDASession) CFRelease( pDASession );
        return false;
    }
    
    pCFUrl = (CFURLRef)CFDictionaryGetValue( pDict, kDADiskDescriptionVolumePathKey );
    if(pCFUrl)
    {
        pczVolPath = CFURLRefToCString( pCFUrl );
        if(pUsbInfo && pczVolPath && strlen(pczVolPath))
        {
            pUsbInfo->SetVolumePath( pczVolPath );
            FetchUsbDeviceInfo( UsbDevice, pUsbInfo );
            
            if(pDict) CFRelease( pDict );
            if(pDADisk) CFRelease( pDADisk );
            if(pDASession) CFRelease( pDASession );
            return true;
        }
    }
    
    pCFString = (CFStringRef)CFDictionaryGetValue( pDict, kDADiskDescriptionVolumeNameKey );
    if(pCFString)
    {
        pczVolume = CFStringRefToCString( pCFString );
        if(pUsbInfo && pczVolume && strlen(pczVolume))
        {
            pUsbInfo->SetVolume( pczVolume );
            FetchUsbDeviceInfo( UsbDevice, pUsbInfo );
            
            if(pDict) CFRelease( pDict );
            if(pDADisk) CFRelease( pDADisk );
            if(pDASession) CFRelease( pDASession );
            return true;
        }
    }
    
    if(pDict) CFRelease( pDict );
    if(pDADisk) CFRelease( pDADisk );
    if(pDASession) CFRelease( pDASession );
    return false;
}

void UsbFinder::UsbDevice_Callback(void* pRefCon, io_iterator_t Iter)
{
    int  nIndex=0;
    bool bSuc = false;
    io_service_t    UsbDevice;
    io_name_t       ClassName;
    io_name_t       DeviceName;
    CFStringRef     pBsdName   = NULL;
    char*           pczBsdNamePos = NULL;
    char            czBsdNameBuf[260];
    UsbFinder*      pUsbInfo = NULL;
    
    pUsbInfo = (UsbFinder*)pRefCon;

    while(( UsbDevice = IOIteratorNext(Iter) ))
    {
        pBsdName = NULL;
        memset( ClassName, 0, sizeof(ClassName) );
        memset( DeviceName, 0, sizeof(DeviceName) );
        
        if(!IOObjectConformsTo( UsbDevice, kIOUSBDeviceClassName ))
        {
            continue;
        }
       
        IOObjectGetClass( UsbDevice, ClassName );
        if(strcmp(ClassName, "IOUSBDevice"))
        {
            continue;
        }
        
        IORegistryEntryGetName( UsbDevice, DeviceName );
        pBsdName = (CFStringRef)IORegistryEntrySearchCFProperty(UsbDevice,
                                                                kIOServicePlane,
                                                                CFSTR(kIOBSDNameKey),
                                                                kCFAllocatorDefault,
                                                                kIORegistryIterateRecursively );
        if(!pBsdName)
        {
            continue;
        }
        
        for(nIndex=0; nIndex<5; nIndex++)
        {
            memset( czBsdNameBuf, 0, sizeof(czBsdNameBuf) );
            // sprintf( czBsdNameBuf, "/dev/%ss1", CFStringRefToCString(pBsdName) );
            if(!nIndex)
            {
                sprintf( czBsdNameBuf, "/dev/%s", CFStringRefToCString(pBsdName) );
            }
            else
            {
                sprintf( czBsdNameBuf, "/dev/%ss%d", CFStringRefToCString(pBsdName), nIndex );
            }
            pczBsdNamePos = &czBsdNameBuf[0];
        
            bSuc = UsbDevice_Parsing( pczBsdNamePos, UsbDevice, pUsbInfo );
            if(bSuc == true)
            {
                break;
            }
        }
        
    }
    
}


char*
UsbFinder::FetchUsbStringDescriptor(IOUSBDeviceInterface182** ppUsbDevice, BYTE bIndex)
{
    UInt16  wBuffer[64];
    IOUSBDevRequest Request;
    kern_return_t   nRet = 0;
    char*  pczNewBuf = NULL;
    int    nPos=0, nMaxPos=0;
    
    if(!ppUsbDevice || !(*ppUsbDevice))
    {
        return NULL;
    }
    
    Request.bmRequestType = USBmakebmRequestType( kUSBIn, kUSBStandard, kUSBDevice );
    Request.bRequest = kUSBRqGetDescriptor;
    Request.wValue   = (kUSBStringDesc << 8) | bIndex;
    Request.wIndex   = 0x409; // english
    Request.wLength  = sizeof(wBuffer);
    Request.pData    = wBuffer;
    
    nRet = (*ppUsbDevice)->DeviceRequest( ppUsbDevice, &Request );
    if(nRet != 0)
    {
        return NULL;
    }
    
    pczNewBuf = (char*)malloc( 128 );
    if(!pczNewBuf) return NULL;
    
    nMaxPos = (Request.wLenDone - 1)/2;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        pczNewBuf[nPos] = wBuffer[nPos+1];
    }
    pczNewBuf[nPos] = '\0';
    return pczNewBuf;
}


void UsbFinder::FetchUsbDeviceInfo(io_service_t UsbDevice, UsbFinder* pUsbInfo )
{
    SInt32                     nScore  = 0;
    kern_return_t              nReturn = 0;
    IOCFPlugInInterface**      ppPlugInIFace = NULL;
    IOUSBDeviceInterface182**  ppUsbIFace182 = NULL;
    char*  pczBuffer  = NULL;
    UInt8  bVIDIndex  = 0;
    UInt8  bPIDIndex  = 0;
    UInt8  bSNIndex   = 0;
    UInt16 wVendorID  = 0;
    UInt16 wProductID = 0;
    
    if(!pUsbInfo)
    {
        return;
    }
    
    nReturn = IOCreatePlugInInterfaceForService( UsbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
                                                 &ppPlugInIFace, &nScore );
    if(nReturn != 0)
    {
        return;
    }
    
    nReturn = (*ppPlugInIFace)->QueryInterface( ppPlugInIFace,
                                                CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID182),
                                               (void**)&ppUsbIFace182 );
    IODestroyPlugInInterface( ppPlugInIFace );
    if(nReturn != 0 || !ppUsbIFace182 || !(*ppUsbIFace182))
    {
        return;
    }
    
    (*ppUsbIFace182)->GetDeviceVendor( ppUsbIFace182,  (UInt16*)&wVendorID  );
    (*ppUsbIFace182)->GetDeviceProduct( ppUsbIFace182, (UInt16*)&wProductID );
    
    pUsbInfo->SetVendorID( wVendorID );
    pUsbInfo->SetProductID( wProductID );
    
    (*ppUsbIFace182)->USBGetProductStringIndex( ppUsbIFace182, &bPIDIndex );
    (*ppUsbIFace182)->USBGetManufacturerStringIndex( ppUsbIFace182, &bVIDIndex );
    (*ppUsbIFace182)->USBGetSerialNumberStringIndex( ppUsbIFace182, &bSNIndex  );
  
    pczBuffer = FetchUsbStringDescriptor( ppUsbIFace182, bSNIndex );
    if(pczBuffer)
    {
        pUsbInfo->SetSerial( pczBuffer );
        free( pczBuffer );
    }

    pczBuffer = FetchUsbStringDescriptor( ppUsbIFace182, bVIDIndex );
    if(pczBuffer)
    {
        pUsbInfo->SetVendor( pczBuffer );
        free( pczBuffer );
    }
    
    pczBuffer = FetchUsbStringDescriptor( ppUsbIFace182, bPIDIndex );
    if(pczBuffer)
    {
        pUsbInfo->SetProduct( pczBuffer );
        free( pczBuffer );
    }
    
    pUsbInfo->m_nCount++;
    
}
#endif

void UsbFinder::SetVolumePath( char* pczVolumePath )
{
    if(!pczVolumePath)
    {
        return;
    }
    memset( m_UsbInfo[m_nCount].m_czVolumePath, 0, sizeof(m_UsbInfo[m_nCount].m_czVolumePath) );
    sprintf( m_UsbInfo[m_nCount].m_czVolumePath, "%s", pczVolumePath );
}

void UsbFinder::SetVolume( char* pczVolume )
{
    if(!pczVolume)
    {
        return;
    }
    
    memset( m_UsbInfo[m_nCount].m_czVolumePath, 0, sizeof(m_UsbInfo[m_nCount].m_czVolumePath) );
    sprintf( m_UsbInfo[m_nCount].m_czVolumePath, "/Volumes/%s", pczVolume );
}

void UsbFinder::SetVendorID(int nVendorID)
{
    m_UsbInfo[m_nCount].m_nVendorID = nVendorID;
}

void UsbFinder::SetProductID(int nProductID)
{
    m_UsbInfo[m_nCount].m_nProductID = nProductID;
}

void UsbFinder::SetVendor(char* pczVendor)
{
    if(!pczVendor)
    {
        return;
    }
    
    memset( m_UsbInfo[m_nCount].m_czVendor, 0, sizeof(m_UsbInfo[m_nCount].m_czVendor) );
    strncpy( m_UsbInfo[m_nCount].m_czVendor, pczVendor, strlen(pczVendor) );
}


void UsbFinder::SetProduct(char* pczProduct)
{
    if(!pczProduct)
    {
        return;
    }
    memset( m_UsbInfo[m_nCount].m_czProduct,  0, sizeof(m_UsbInfo[m_nCount].m_czProduct) );
    strncpy( m_UsbInfo[m_nCount].m_czProduct, pczProduct, strlen(pczProduct) );
    
}

void UsbFinder::SetSerial(char* pczSerial)
{
    if(!pczSerial)
    {
        return;
    }
    
    memset( m_UsbInfo[m_nCount].m_czSerial, 0, sizeof(m_UsbInfo[m_nCount].m_czSerial) );
    strncpy( m_UsbInfo[m_nCount].m_czSerial, pczSerial, strlen(pczSerial) );
}

void UsbFinder::SetInterface( int nPos, USBIface& UsbIface )
{
    memcpy( &m_UsbInfo[m_nCount].m_Iface[nPos], &UsbIface, sizeof(UsbIface) );
}

void UsbFinder::SetInterface(int nNumber, char* pczIfaceName, int nClass, int nSubClass, int nProto )
{
    if(!pczIfaceName)
        return;

    memset( &m_UsbInfo[m_nCount].m_Iface[nNumber], 0, sizeof(m_UsbInfo[m_nCount].m_Iface[nNumber]) );
    strncpy( m_UsbInfo[m_nCount].m_Iface[nNumber].m_czIfaceName, pczIfaceName, strlen(pczIfaceName) );
    m_UsbInfo[m_nCount].m_Iface[nNumber].m_nIndex    = nNumber;
    m_UsbInfo[m_nCount].m_Iface[nNumber].m_nClass    = nClass;
    m_UsbInfo[m_nCount].m_Iface[nNumber].m_nSubClass = nSubClass;
    m_UsbInfo[m_nCount].m_Iface[nNumber].m_nProtocol = nProto;
}

void UsbFinder::SetCopyUSBInfo( USBInfo& UsbInfo )
{
    memset( &m_UsbInfo[m_nCount], 0, sizeof(USBInfo) );
    memcpy( &m_UsbInfo[m_nCount], &UsbInfo, sizeof(USBInfo) );
}



void UsbFinder::SetMediaID(USBInfo& usbInfo)
{
    memset( usbInfo.m_czMediaID, 0, sizeof(usbInfo.m_czMediaID) );
    snprintf( usbInfo.m_czMediaID, sizeof(usbInfo.m_czMediaID),
             "USB\\VID_%04X&PID_%04X\\%s", usbInfo.m_nVendorID, usbInfo.m_nProductID, usbInfo.m_czSerial );
}


uint32_t UsbFinder::SetDeviceType(USBInfo& usbInfo)
{
    if(true == IsUsbDeviceUsbMobile( usbInfo ))
    {
        usbInfo.m_nDeviceType |= DEVICE_TYPE_USBMOBILE;
    }
    else if(true == IsUsbDeviceUsbStor( usbInfo ))
    {
        usbInfo.m_nDeviceType |= DEVICE_TYPE_USBSTOR;
        
    }
    return usbInfo.m_nDeviceType;
}


bool UsbFinder::IsUsbDeviceUsbStor( USBInfo& usbInfo )
{
    uint32_t nLength = 0;
    nLength = (uint32_t)strlen(usbInfo.m_czVolumePath);
    if(nLength <= 0) return false;
    return true;
}


bool UsbFinder::IsUsbDeviceUsbMobile( USBInfo& usbInfo )
{
    uint32_t nPos=0, nMaxPos=0;
    USBIface* pIface = NULL;
    
    nMaxPos = usbInfo.m_nIfaceCount;
    for(nPos=0; nPos<=nMaxPos; nPos++)
    {
        pIface = &usbInfo.m_Iface[nPos];
        if(!pIface) continue;
        
        if(0 == strncasecmp( pIface->m_czIfaceName, "MTP", strlen("MTP") ))
        {   // MTP
            return true;
        }
        else if(0x06 == pIface->m_nClass && 0x01 == pIface->m_nSubClass && 0x01 == pIface->m_nProtocol)
        {  // PTP
            return true;
        }
        else if(0xFF == pIface->m_nClass && 0x42 == pIface->m_nSubClass && 0x01 == pIface->m_nProtocol)
        {  // ADB
            return true;
        }
    }
    
    if( 0 == strncasecmp(usbInfo.m_czProduct, "iPad", strlen("iPad")) ||
        0 == strncasecmp(usbInfo.m_czProduct, "iPod", strlen("iPod")) ||
        0 == strncasecmp(usbInfo.m_czProduct, "iPhone", strlen("iPhone")) )
    {
        return true;
    }
    return false;
}

#ifdef _FIXME_

bool UsbFinder::UsbDeviceSetVolumePath( char* pczBsdNamePos, io_service_t UsbDevice, USBInfo& usbInfo )
{
    CFDictionaryRef pDict       = NULL;
    DASessionRef    pDASession  = NULL;
    DADiskRef       pDADisk     = NULL;
    CFURLRef        pCFUrl      = NULL;
    CFStringRef     pCFString   = NULL;
    char*           pczVolume   = NULL;
    char*           pczVolPath  = NULL;
    
    if(!pczBsdNamePos)
    {
        return false;
    }
    
    pDASession = DASessionCreate( kCFAllocatorDefault );
    if(!pDASession) return false;
    
    pDADisk = DADiskCreateFromBSDName( kCFAllocatorDefault, pDASession, pczBsdNamePos );
    if(!pDADisk)
    {
        if(pDASession) CFRelease( pDASession );
        return false;
    }
    
    pDict = DADiskCopyDescription( pDADisk );
    if(!pDict)
    {
        if(pDADisk) CFRelease( pDADisk );
        if(pDASession) CFRelease( pDASession );
        return false;
    }
    
    pCFUrl = (CFURLRef)CFDictionaryGetValue( pDict, kDADiskDescriptionVolumePathKey );
    if(pCFUrl)
    {
		pczVolPath = CFURLRefToCString( pCFUrl );
		
		CFStringRef value = CFStringCreateWithCString(kCFAllocatorDefault, pczVolPath, kCFStringEncodingUTF8);
		CFStringRef temp = CFURLCreateStringByReplacingPercentEscapes(kCFAllocatorDefault, value, CFSTR(""));
		pczVolPath = CFStringRefToCString(temp);
		//DEBUG_LOG("test - volume:%s", pczVolPath);
	
		if(pczVolPath && strlen(pczVolPath))
        {
            memset( usbInfo.m_czVolumePath, 0, sizeof(usbInfo.m_czVolumePath) );
            sprintf( usbInfo.m_czVolumePath, "%s", pczVolPath );
            usbInfo.m_nDeviceType |= DEVICE_TYPE_USBSTOR;
            
            if(pDict) CFRelease( pDict );
            if(pDADisk) CFRelease( pDADisk );
            if(pDASession) CFRelease( pDASession );
            return true;
        }
    }
    
    pCFString = (CFStringRef)CFDictionaryGetValue( pDict, kDADiskDescriptionVolumeNameKey );
    if(pCFString)
    {
		CFStringRef temp = CFURLCreateStringByReplacingPercentEscapes(kCFAllocatorDefault, pCFString, CFSTR(""));
		pczVolume = CFStringRefToCString(temp);
		//DEBUG_LOG("test - volume:%s", pczVolume);

        //pczVolume = CFStringRefToCString( pCFString );
        if(pczVolume && strlen(pczVolume))
        {
            memset( usbInfo.m_czVolumePath, 0, sizeof(usbInfo.m_czVolumePath) );
            sprintf( usbInfo.m_czVolumePath, "%s", pczVolume);
            usbInfo.m_nDeviceType |= DEVICE_TYPE_USBSTOR;
            
            if(pDict) CFRelease( pDict );
            if(pDADisk) CFRelease( pDADisk );
            if(pDASession) CFRelease( pDASession );
            return true;
        }
    }
    
    if(pDict) CFRelease( pDict );
    if(pDADisk) CFRelease( pDADisk );
    if(pDASession) CFRelease( pDASession );
    return false;
}


bool UsbFinder::FetchUsbDeviceBsdName( io_service_t UsbDevice, USBInfo& usbInfo )
{
    uint32_t    nIndex = 0;
    CFStringRef pBsdName = NULL;
    char*       pczBsdNamePos = NULL;
    char        czBsdNameBuf[MAX_PATH];
    
    pBsdName = (CFStringRef)IORegistryEntrySearchCFProperty( UsbDevice,
                                                             kIOServicePlane, CFSTR(kIOBSDNameKey),
                                                             kCFAllocatorDefault, kIORegistryIterateRecursively );
    if(!pBsdName) return false;
    
    for(nIndex=0; nIndex<5; nIndex++)
    {
        memset( czBsdNameBuf, 0, sizeof(czBsdNameBuf) );
        if(!nIndex)
        {
            sprintf( czBsdNameBuf, "/dev/%s", CFStringRefToCString(pBsdName) );
        }
        else
        {
            sprintf( czBsdNameBuf, "/dev/%ss%d", CFStringRefToCString(pBsdName), nIndex );
        }
        pczBsdNamePos = &czBsdNameBuf[0];
        
        if(true == UsbDeviceSetVolumePath( pczBsdNamePos, UsbDevice, usbInfo ))
        {
            break;
        }
    }
    CFRelease( pBsdName );
    return true;
}


void UsbFinder::UsbDeviceEnumerate(void* pRefCon, io_iterator_t Iter)
{
    io_service_t  UsbDevice;
    io_name_t     ClassName;
    io_name_t     DeviceName;
    UsbFinder*    pUsbFinder = NULL;
    USBInfo       usbInfo;
   
    pUsbFinder = (UsbFinder*)pRefCon;
    if(!pUsbFinder) return;

    while(( UsbDevice = IOIteratorNext(Iter) ))
    {
        memset( &usbInfo, 0, sizeof(usbInfo) );
        memset( ClassName, 0, sizeof(ClassName) );
        memset( DeviceName, 0, sizeof(DeviceName) );
        
        if(!IOObjectConformsTo( UsbDevice, kIOUSBDeviceClassName )) continue;
        
        IOObjectGetClass( UsbDevice, ClassName );
        IORegistryEntryGetName( UsbDevice, DeviceName );
        if(strcmp(ClassName, "IOUSBDevice")) continue;
        
        FetchUsbDevice( UsbDevice, pUsbFinder, usbInfo );
        FetchUsbDeviceBsdName( UsbDevice, usbInfo );
        FetchUsbInterface( UsbDevice, pUsbFinder, usbInfo );
        pUsbFinder->SetDeviceType( usbInfo );
        
        pUsbFinder->SetCopyUSBInfo( usbInfo );
        pUsbFinder->m_nCount++;
    }
}


bool UsbFinder::FetchUsbDevice( io_service_t UsbDevice, UsbFinder* pUsbFinder, USBInfo& usbInfo )
{
    CFStringRef   pEntryCFString = NULL;
    CFNumberRef   pEntryNumber = NULL;
    
    if(!pUsbFinder)
    {
        return false;
    }
    
    pEntryNumber = (CFNumberRef)IORegistryEntryCreateCFProperty( UsbDevice, CFSTR(kUSBVendorID), kCFAllocatorDefault, 0 );
    if(pEntryNumber)
    {
        CFNumberGetValue( pEntryNumber, kCFNumberSInt32Type, &usbInfo.m_nVendorID );
        pUsbFinder->SetVendorID( usbInfo.m_nVendorID );
        CFRelease( pEntryNumber );
    }
    
    pEntryNumber = (CFNumberRef)IORegistryEntryCreateCFProperty( UsbDevice, CFSTR(kUSBProductID), kCFAllocatorDefault, 0 );
    if(pEntryNumber)
    {
        CFNumberGetValue( pEntryNumber, kCFNumberSInt32Type, &usbInfo.m_nProductID );
        pUsbFinder->SetProductID( usbInfo.m_nProductID );
        CFRelease( pEntryNumber );
    }
    
    pEntryCFString = (CFStringRef)IORegistryEntryCreateCFProperty( UsbDevice, CFSTR(kUSBProductString), kCFAllocatorDefault, 0 );
    if(pEntryCFString)
    {
        CFStringGetCString( (const __CFString *)pEntryCFString, usbInfo.m_czProduct, sizeof(usbInfo.m_czProduct), kCFStringEncodingASCII );
        pUsbFinder->SetVendor( usbInfo.m_czProduct );
        CFRelease( pEntryCFString );
    }
    
    pEntryCFString = (CFStringRef)IORegistryEntryCreateCFProperty( UsbDevice, CFSTR(kUSBVendorString), kCFAllocatorDefault, 0 );
    if(pEntryCFString)
    {
        CFStringGetCString( (const __CFString *)pEntryCFString, usbInfo.m_czVendor, sizeof(usbInfo.m_czVendor), kCFStringEncodingASCII );
        pUsbFinder->SetProduct( usbInfo.m_czVendor );
        CFRelease( pEntryCFString );
    }
    
    pEntryCFString = (CFStringRef)IORegistryEntryCreateCFProperty( UsbDevice, CFSTR(kUSBSerialNumberString), kCFAllocatorDefault, 0 );
    if(pEntryCFString)
    {
        CFStringGetCString( (const __CFString *)pEntryCFString, usbInfo.m_czSerial, sizeof(usbInfo.m_czSerial), kCFStringEncodingASCII );
        pUsbFinder->SetSerial( usbInfo.m_czSerial );
        CFRelease( pEntryCFString );
    }
    pUsbFinder->SetMediaID( usbInfo );
    return true;
}


bool UsbFinder::FetchUsbInterface( io_service_t UsbDevice, UsbFinder* pUsbFinder, USBInfo& usbInfo )
{
    uint32_t  nPos = 0;
    io_iterator_t IfaceIter;
    io_service_t  UsbIface;
    CFNumberRef   pEntryNumber = NULL;
    
    if(!pUsbFinder) return false;
    
    IORegistryEntryGetChildIterator( UsbDevice, kIOServicePlane, &IfaceIter );
    while(( UsbIface  = IOIteratorNext(IfaceIter) ))
    {
        IORegistryEntryGetName( UsbIface, usbInfo.m_Iface[nPos].m_czIfaceName );
        pEntryNumber = (CFNumberRef)IORegistryEntryCreateCFProperty( UsbIface, CFSTR(kUSBInterfaceNumber), kCFAllocatorDefault, 0 );
        if(pEntryNumber)
        {
            CFNumberGetValue( pEntryNumber, kCFNumberSInt32Type, &usbInfo.m_Iface[nPos].m_nIndex );
            CFRelease( pEntryNumber );
        }
        
        pEntryNumber = (CFNumberRef)IORegistryEntryCreateCFProperty( UsbIface, CFSTR(kUSBInterfaceClass), kCFAllocatorDefault, 0 );
        if(pEntryNumber)
        {
            CFNumberGetValue( pEntryNumber, kCFNumberSInt32Type, &usbInfo.m_Iface[nPos].m_nClass );
            CFRelease( pEntryNumber );
        }
        pEntryNumber = (CFNumberRef)IORegistryEntryCreateCFProperty( UsbIface, CFSTR(kUSBInterfaceSubClass), kCFAllocatorDefault, 0 );
        if(pEntryNumber)
        {
            CFNumberGetValue( pEntryNumber, kCFNumberSInt32Type, &usbInfo.m_Iface[nPos].m_nSubClass );
            CFRelease( pEntryNumber );
        }
        pEntryNumber = (CFNumberRef)IORegistryEntryCreateCFProperty( UsbIface, CFSTR(kUSBInterfaceProtocol), kCFAllocatorDefault, 0 );
        if(pEntryNumber)
        {
            CFNumberGetValue( pEntryNumber, kCFNumberSInt32Type, &usbInfo.m_Iface[nPos].m_nProtocol );
            CFRelease( pEntryNumber );
        }
        
        pUsbFinder->SetInterface( nPos, usbInfo.m_Iface[nPos] );
        usbInfo.m_nIfaceCount++;
        nPos++;
    }
    return true;
}

#endif

uint32_t UsbFinder::GetUsbDeviceCount()
{
    return m_nCount;
}

bool UsbFinder::GetUsbDevice(int nPos, char* pczOutBuf, int nMaxOutBuf)
{
    char czMediaID[MAX_PATH];
    if(!pczOutBuf || nMaxOutBuf <= 0 || nPos < 0 || nPos >= m_nCount)
    {
        return false;
    }
    memset( czMediaID, 0, sizeof(czMediaID) );
    snprintf( czMediaID, sizeof(czMediaID),
              "MediaID=USB\\VID_%04X&PID_%04X\\%s",
              m_UsbInfo[nPos].m_nVendorID, m_UsbInfo[nPos].m_nProductID, m_UsbInfo[nPos].m_czSerial );
    snprintf( pczOutBuf, nMaxOutBuf,
             "%s,"
             "VID=%d, "
             "PID=%d, "
             "Vendor=%s, "
             "Product=%s, "
             "Serial=%s",
             czMediaID,
             m_UsbInfo[nPos].m_nVendorID,
             m_UsbInfo[nPos].m_nProductID,
             m_UsbInfo[nPos].m_czVendor,
             m_UsbInfo[nPos].m_czProduct,
             m_UsbInfo[nPos].m_czSerial    );
    return true;
}

bool UsbFinder::GetUsbDeviceInfo( int nPos, USBInfo* pOutUsbInfo )
{
    if(!pOutUsbInfo || nPos < 0 || nPos >= m_nCount)
    {
        return false;
    }
    memcpy( pOutUsbInfo, &m_UsbInfo[nPos], sizeof(USBInfo) );
    return true;
}

bool UsbFinder::Reload(std::vector<USBInfo>& list) {
	UsbDeviceInit();
	uint32_t nTotalCount = GetUsbDeviceCount();
	if(0 == nTotalCount) {
		return false;
	}

	for(int i=0; i<nTotalCount; i++) {
		USBInfo usbInfo;
		GetUsbDeviceInfo( i, &usbInfo );
		if (DEVICE_TYPE_USBSTOR != usbInfo.m_nDeviceType) {
			continue;
		}

		list.push_back(usbInfo);
	}

	return (0 < list.size())?true:false;
}
