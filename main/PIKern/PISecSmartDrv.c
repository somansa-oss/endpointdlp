#ifdef LINUX
#else
#import <Foundation/Foundation.h>

#import <IOKit/IOKitLib.h>
#import <IOKit/usb/IOUSBLib.h>
#import <IOKit/hid/IOHIDKeys.h>
#endif

#include <stdio.h>
#include <sys/mount.h>

#ifdef LINUX
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "SmartCmd.h"
#include "PISecSmartDrv.h"
#include "KextDeviceNotify.h"
#include "kauth_listener.h"
#include "kernel_control.h"
#include "system_control.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

DRV_KEXT  g_DrvKext;

char    g_czCupsSpoolPathDump[MAX_FILE_LENGTH] = {0,};
char    g_czCupsSpoolPath[MAX_FILE_LENGTH] = {0,};
char    g_czCupsTempPath[MAX_FILE_LENGTH] = {0,};

boolean_t g_bMoreRecentThanCatalina = false;

DEVICE_TYPE_V6 g_pDeviceType_V6_List[] =
{
    { 0x0001, "CopyPrevent\\Removable"}, // Copy Prevent+ 정책
    { 0x0002, "Media\\CD/DVD"   }, // CD/DVD 읽기 쓰기 차단
    { 0x0004, "Media\\Floppy"   }, // Floppy 읽기 쓰기 차단
    { 0x0008, "Media\\Removable"}, // USB 읽기 쓰기
    { 0x0010, "Media\\NetDrive" }, // 외부 공유폴더 및 네트워크
    { 0x0020, "Media\\Serial"   }, // 시리얼포트 차단
    { 0x0040, "Media\\Parallel" }, // 패러럴포트 차단
    { 0x0080, "Media\\1394"     }, // 1394 차단
};


kern_return_t
PISecSmartDrv_stop(kmod_info_t* pKmodInfo, void* pData)
{
    kern_return_t nResult = KERN_SUCCESS;
    
    g_DrvKext.SelfCtx.bProtect = FALSE;
    KextNotify_Uninit();
    RemoveKauthListener();
    StopSystemControl();
    StopKernelControl();
    PISecSmartDrv_Uninit();
    printf("[DLP][%s] Success. \n", __FUNCTION__ );
    return KERN_SUCCESS;
}

void PrintKmodInfo(char* moduleName, kmod_info_t *ki, void *d)
{
}

kern_return_t
PISecSmartDrv_start(kmod_info_t* pKmodInfo, void* pData)
{
    int version = 0;
    int version_major = 0;
    int version_minor = 0;
    
    printf("[DLP][%s] Kernel Version Info: [%d] [%d] [%d]",__FUNCTION__, version, version_major, version_minor);
    
    if (version_major < 19)
        g_bMoreRecentThanCatalina = false;
    else
        g_bMoreRecentThanCatalina = true;

    PrintKmodInfo( "PIESF", pKmodInfo, pData );
    if(PISecSmartDrv_Init() != KERN_SUCCESS)
    {
        PISecSmartDrv_stop( pKmodInfo, pData );
        printf("[DLP][%s] PISecSmartDrv_Init failed. \n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
    if(StartKernelControl() != KERN_SUCCESS)
    {
        PISecSmartDrv_stop( pKmodInfo, pData);
        printf("[DLP][%s] failed to start kernel control\n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
    if(StartSystemControl() != KERN_SUCCESS)
    {
        PISecSmartDrv_stop( pKmodInfo, pData );
        printf("[DLP][%s] failed to start system control\n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
    if(InstallKauthListener() != KERN_SUCCESS)
    {
        PISecSmartDrv_stop(pKmodInfo, pData);
        printf("[DLP][%s] failed to register kauth listner\n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
    if(FALSE == KextNotify_Init())
    {
        PISecSmartDrv_stop(pKmodInfo, pData);
        printf("[DLP][%s] failed. KextNotify_Init. \n", __FUNCTION__);
        return KERN_FAILURE;
    }
    
#ifdef FIXME_UPLOAD
    if(KERN_SUCCESS != InstallNetworkFilter() )
    {
        PISecSmartDrv_stop(pKmodInfo, pData);
        printf("[DLP][%s] failed. InstallNetworkFilter. \n", __FUNCTION__);
        return KERN_FAILURE;
    }
#endif
    
    FetchVolumes();
    
    printf("[DLP][%s] Success. \n", __FUNCTION__ );
    return KERN_SUCCESS;
}


kern_return_t
PISecSmartDrv_Init(void)
{
    kern_return_t nResult = KERN_SUCCESS;
    printf("[DLP][%s] start.\n", __FUNCTION__ );
    nResult = DrvKext_Init();
    if(nResult != KERN_SUCCESS)
    {
        printf("[DLP][%s] DrvKext_Init() failed(%d)\n", __FUNCTION__, nResult );
    }
    printf("[DLP][%s] End. return=%d \n", __FUNCTION__, nResult );
    return nResult;
}


kern_return_t
PISecSmartDrv_Uninit(void)
{
    kern_return_t nResult = KERN_SUCCESS;
    printf("[DLP][%s] start. \n", __FUNCTION__ );
    
    nResult = DrvKext_Uninit();
    if(nResult != KERN_SUCCESS)
    {
        printf("[DLP][%s] DrvKext_Uninit() failed(%d)\n", __FUNCTION__, nResult );
    }
    printf("[DLP][%s] End. Result=%d \n", __FUNCTION__, nResult );
    return nResult;
}


kern_return_t
DrvKext_Init(void)
{
    printf("[DLP][%s] Start. \n", __FUNCTION__ );
    memset( &g_DrvKext, 0, sizeof(g_DrvKext) );

    DrvCtx_Init();
    
    // Mobile Init
    DrvMobile_Init();
    
    // Print init
    DrvPrintCtx_Init();
    
    // Upload init
    DrvUploadCtx_Init();
    
    g_DrvKext.bDebugMsg = TRUE;
    g_DrvKext.QtCtx.bQtLimit = TRUE;
    g_DrvKext.CommCtx.nChannel = COMM_CHANNEL_NONE;
    
    // Process AC
    g_DrvKext.ProcCtx.Policy.bLog = FALSE;
    g_DrvKext.ProcCtx.Policy.bProcAC = FALSE;
    
    // MediaControl
    memset( &g_DrvKext.MediaCtx, 0, sizeof(g_DrvKext.MediaCtx) );
    printf("[DLP][%s] End. Success. \n", __FUNCTION__ );
    
    return KERN_SUCCESS;
}


kern_return_t
DrvKext_Uninit(void)
{
    printf("[DLP][%s] Start. \n", __FUNCTION__ );
    // MediaControl
    memset( &g_DrvKext.MediaCtx, 0, sizeof(g_DrvKext.MediaCtx) );
    // Process Access Check
    g_DrvKext.ProcCtx.Policy.bLog = FALSE;
    g_DrvKext.ProcCtx.Policy.bProcAC = FALSE;
    
    // Print Uninit
    DrvPrintCtx_Uninit();
    
    // Mobile UnInit
    DrvMobile_Uninit();
    
    DrvCtx_Uninit();
    
    memset( &g_DrvKext, 0, sizeof(g_DrvKext) );
    g_DrvKext.bDebugMsg = TRUE;
    g_DrvKext.QtCtx.bQtLimit = TRUE;
    g_DrvKext.CommCtx.nChannel = COMM_CHANNEL_NONE;
    printf("[DLP][%s] End. Success. \n", __FUNCTION__ );
    return KERN_SUCCESS;
}

void GetMountPath(const char* pczPath, char* pczDevPath, int nBufferSize1, char* pczBSDPath, int nBufferSize2)
{
    if (pczPath == NULL || pczDevPath == NULL)
        return;

#ifdef LINUX
//FIXME_MUST
#else
    // e.g. /Volumes/tmp/1.doc -> /Volumes/tmp
    
    NSString *temp = @(pczPath);
    NSArray *list = NULL;
    
    if ([temp length] == 1 && [temp characterAtIndex:0] == '/')
    {
        //int     snprintf(char * __restrict __str, size_t __size, const char * __restrict __format, ...) __printflike(3, 4);
        snprintf(pczDevPath, nBufferSize1, "/");
    }
    else
    {
        list = [temp componentsSeparatedByString:@"/"];
        if (list == NULL || list.count < 3) {
            return;
        }

        snprintf(pczDevPath, nBufferSize1, "/%s/%s", [list[1] UTF8String], [list[2] UTF8String]);
    }
    
    DASessionRef dasess = DASessionCreate(nil);
    CFURLRef cfurl = CFURLCreateFromFileSystemRepresentation(nil, (const uint8*)pczDevPath, strlen(pczDevPath), TRUE);
    DADiskRef dadisk = DADiskCreateFromVolumePath(nil, dasess, cfurl);
    CFDictionaryRef cfdict = DADiskCopyDescription(dadisk);
    if (cfdict == NULL)
        return;

    //Shared Folder
    //@"DAVolumeNetwork" : YES
    //NSString *t1 = @"DAVolumeNetwork";
    CFBooleanRef cfbool = (CFBooleanRef)CFDictionaryGetValue(cfdict, @"DAVolumeNetwork");
    if (CFBooleanGetValue(cfbool)) {
        // do nothing!!!
        return;
    }
    
    //NSString *t2 = @"DAMediaBSDName";
    CFStringRef cfstr = (CFStringRef)CFDictionaryGetValue(cfdict, @"DAMediaBSDName");
    if(cfstr) {
        snprintf(pczBSDPath, nBufferSize2, "%s", [(__bridge NSString *)cfstr UTF8String]);
    }
#endif    
}

int
VolCtx_Search_BusType(const char* pczBasePath)
{
    ULONG nPos=0, nMaxPos=0;
    ULONG nCtxLength=0, nLength=0, nCmpLength=0;
    char  czCmpBuf[MAX_BASE_SIZE];
    
    if(!pczBasePath) return -1;
    
    nLength = (ULONG)strlen( pczBasePath );
    nMaxPos = g_DrvKext.VolCtx.nCount;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        if(nPos >= MAX_DEVICE) return -1;
        
        nCtxLength = (ULONG)strlen(g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath);
        if(nLength > nCtxLength)
        {
            memset( czCmpBuf, 0, sizeof(czCmpBuf) );
            strncpy( czCmpBuf, g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath, nCtxLength );
            strncat( czCmpBuf, "/", strlen("/") );
            nCmpLength = (ULONG)strlen(czCmpBuf);
            if(0 == strncasecmp( pczBasePath, czCmpBuf, nCmpLength ))
            {
                return g_DrvKext.VolCtx.VolumeDevice[ nPos ].ulBusType;
            }
        }
        else
        {
            if(0 == strncasecmp( pczBasePath, g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath, nCtxLength ))
            {
                return g_DrvKext.VolCtx.VolumeDevice[ nPos ].ulBusType;
            }
        }
    }
    return -1;
}

boolean_t
VolCtx_Update(char* pczDeviceName, char* pczBasePath, ULONG ulBusType)
{
    ULONG nPos=0, nMaxPos=0, nLength=0;
    
    if(!pczDeviceName || !pczBasePath) return FALSE;
    
    lck_mtx_lock( g_DrvKext.VolCtx.VolLock );
    nMaxPos = g_DrvKext.VolCtx.nCount;
    for(nPos=0; nPos<nMaxPos; nPos++)
    {
        if(nPos >= MAX_DEVICE)
        {
            lck_mtx_unlock( g_DrvKext.VolCtx.VolLock );
            return FALSE;
        }
        
        if(0 == strncmp(g_DrvKext.VolCtx.VolumeDevice[nPos].czDeviceName, pczDeviceName, strlen(pczDeviceName)))
        {
            if(ulBusType == g_DrvKext.VolCtx.VolumeDevice[nPos].ulBusType)
            {
                lck_mtx_unlock( g_DrvKext.VolCtx.VolLock );
                return TRUE;
            }
            
            nLength = (ULONG)strlen(pczBasePath)+1;
            nLength = min(nLength, MAX_DEVICE_SIZE );
            
            memset( g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath, 0, MAX_DEVICE_SIZE );
            strncpy( g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath, pczBasePath, nLength );
            g_DrvKext.VolCtx.VolumeDevice[nPos].ulBusType = ulBusType;
            DrvCtx_Policy_Update_DriveName( &g_DrvKext.VolCtx.VolumeDevice[nPos] );
            
            printf( "[DLP][%s] Exist, BusType=%d, Device=%s, BasePath=%s. Count=%d \n", __FUNCTION__,
                    ulBusType, pczDeviceName, pczBasePath, g_DrvKext.VolCtx.nCount );

            lck_mtx_unlock( g_DrvKext.VolCtx.VolLock );
            return TRUE;
        }
    }
    
    nLength = (ULONG)strlen(pczDeviceName)+1;
    nLength = min(nLength, MAX_DEVICE_SIZE);
    memset( &g_DrvKext.VolCtx.VolumeDevice[nPos], 0, sizeof(VOLUME_DEVICE) );
    strncpy( g_DrvKext.VolCtx.VolumeDevice[nPos].czDeviceName, pczDeviceName, nLength );
    
    nLength = (ULONG)strlen(pczBasePath)+1;
    nLength = min(nLength, MAX_DEVICE_SIZE );
    memset( g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath, 0, MAX_DEVICE_SIZE );
    strncpy( g_DrvKext.VolCtx.VolumeDevice[nPos].czBasePath, pczBasePath, nLength );
    
    g_DrvKext.VolCtx.VolumeDevice[nPos].ulBusType = ulBusType;
    g_DrvKext.VolCtx.nCount++;
    DrvCtx_Policy_Update_DriveName( &g_DrvKext.VolCtx.VolumeDevice[nPos] );
    
    // e.g.
    // [VolCtx_Update] NewAdd BusType=7, Device=/dev/disk2s2, BasePath=/Volumes/새 볼륨. Count=2
    //
    printf( "[DLP][%s] NewAdd BusType=%d, Device=%s, BasePath=%s. Count=%d \n", __FUNCTION__,
            ulBusType, pczDeviceName, pczBasePath, g_DrvKext.VolCtx.nCount );

    lck_mtx_unlock( g_DrvKext.VolCtx.VolLock );
    return TRUE;
}

void VolCtx_Clear(void)
{
    lck_mtx_lock( g_DrvKext.VolCtx.VolLock );
    g_DrvKext.VolCtx.nCount = 0;
    memset( &g_DrvKext.VolCtx.VolumeDevice, 0, sizeof(g_DrvKext.VolCtx.VolumeDevice) );
    lck_mtx_unlock( g_DrvKext.VolCtx.VolLock );
}

boolean_t
IsMediaPath_SFolder(const char* pczDevice, const char* pczBasePath, const char* pczFsTypeName)
{
    boolean_t        bSFolder       = false;
    
    if (pczBasePath == NULL)
        return bSFolder;
    
#ifdef LINUX
//FIXME_MUST
#else    
    NSString *volume = @(pczBasePath);
    DASessionRef dasess = DASessionCreate(nil);
    CFURLRef cfurl = CFURLCreateFromFileSystemRepresentation(nil, (const uint8*)[volume UTF8String], strlen([volume UTF8String]), TRUE);
    DADiskRef dadisk = DADiskCreateFromVolumePath(nil, dasess, cfurl);
    CFDictionaryRef cfdict = DADiskCopyDescription(dadisk);
    if (cfdict == NULL)
        return bSFolder;
    
    //Shared Folder
    //@"DAVolumeNetwork" : YES
    //NSString *t1 = @"DAVolumeNetwork";
    //CFBooleanRef cfbool = (CFBooleanRef)CFDictionaryGetValue(cfdict, [t1 UTF8String]);
    CFBooleanRef cfbool = (CFBooleanRef)CFDictionaryGetValue(cfdict, @"DAVolumeNetwork");
    if (cfbool) {
        if (kCFBooleanTrue == cfbool) {
            bSFolder = true;
        }
    }
#endif

    return bSFolder;
}

//
// 파라미터(device)로 주어진 디바이스가 removable 디바이스인지를 알아내는 함수.
// e.g. /Volumes/temp
boolean_t
IsMediaPath_UsbStor( const char* pczPath )
{
    boolean_t        bUsbStor   = false;
    
    if (pczPath == NULL)
        return bUsbStor;
    
#ifdef LINUX
//FIXME_MUST
#else    
    NSString *volume = @(pczPath);
    DASessionRef dasess = DASessionCreate(nil);
    CFURLRef cfurl = CFURLCreateFromFileSystemRepresentation(nil, (const uint8*)[volume UTF8String], strlen([volume UTF8String]), TRUE);
    DADiskRef dadisk = DADiskCreateFromVolumePath(nil, dasess, cfurl);
    CFDictionaryRef cfdict = DADiskCopyDescription(dadisk);
    if (cfdict == NULL)
        return bUsbStor;
    
    // USB
    // @"DAMediaKind" : @"IOMedia"
    //NSString *t1 = @"DAMediaKind";
    //CFStringRef cfstr = (CFStringRef)CFDictionaryGetValue(cfdict, [t1 UTF8String]);
    CFStringRef cfstr = (CFStringRef)CFDictionaryGetValue(cfdict, @"DAMediaKind");
    if(cfstr) {
        if([(__bridge NSString *)cfstr isEqualToString:@"IOMedia"]) {
            //NSString *t2 = @"DADeviceInternal";
            //CFBooleanRef cfbool = (CFBooleanRef)CFDictionaryGetValue(cfdict, [t2 UTF8String]);
            CFBooleanRef cfbool = (CFBooleanRef)CFDictionaryGetValue(cfdict, @"DADeviceInternal");
            if (cfbool) {
                if (kCFBooleanFalse == cfbool) {
                    bUsbStor = true;
                }
            }
        }
    }
#endif

    return bUsbStor;
}


boolean_t
IsMediaPath_CDStor(const char* pczPath)
{
    boolean_t        bCDStor       = FALSE;

    if (pczPath == NULL)
        return bCDStor;
    
#ifdef LINUX
//FIXME_MUST
#else    
    NSString *volume = @(pczPath);
    DASessionRef dasess = DASessionCreate(nil);
    CFURLRef cfurl = CFURLCreateFromFileSystemRepresentation(nil, (const uint8*)[volume UTF8String], strlen([volume UTF8String]), TRUE);
    DADiskRef dadisk = DADiskCreateFromVolumePath(nil, dasess, cfurl);
    CFDictionaryRef cfdict = DADiskCopyDescription(dadisk);
    if (cfdict == NULL)
        return bCDStor;
    	
    // CD/DVD/BD
    //@"DAMediaKind" : @"IODVDMedia"    // IOCDMedia, IOBDMedia
    //NSString *t1 = @"DAMediaKind";
    //CFStringRef cfstr = (CFStringRef)CFDictionaryGetValue(cfdict, [t1 UTF8String]);
    CFStringRef cfstr = (CFStringRef)CFDictionaryGetValue(cfdict, @"DAMediaKind");
    if(cfstr) {
        if([(__bridge NSString *)cfstr isEqualToString:@"IODVDMedia"]
           || [(__bridge NSString *)cfstr isEqualToString:@"IOCDMedia"]
           || [(__bridge NSString *)cfstr isEqualToString:@"IOBDMedia"]) {
            bCDStor = true;
        }
    }
#endif

    return bCDStor;
}
