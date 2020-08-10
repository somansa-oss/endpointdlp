#include <stdio.h>
#include <string.h>

#ifdef LINUX
#else
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/usb/IOUSBLib.h>
#import <IOKit/hid/IOHIDKeys.h>
#endif

#ifdef LINUX
    #include "../../PISupervisor/apple/include/KernelProtocol.h"
#else
    #include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#endif

#include "KextDeviceNotify.h"
#include "PISecSmartDrv.h"


#ifndef VFS_RETURNED
#define VFS_RETURNED    0
#endif

#ifdef LINUX
int EnumMountCallback(void* pMount, void* pParam);
#else
int EnumMountCallback(mount_t pMount, void* pParam);
#endif

void FetchVolumes()
{
    VolCtx_Clear();
    EnumMountCallback(NULL, NULL);
}

#ifdef LINUX
int EnumMountCallback(void *pMount, void* pParam)
#else
int EnumMountCallback(mount_t pMount, void* pParam)
#endif
{
    int       nBusType = 0;
    boolean_t bUsbStor = FALSE;
    boolean_t bCDStor  = FALSE;
    boolean_t bSFolder = FALSE;
    boolean_t bTBStor  = FALSE;

#ifdef LINUX
//FIXME_MUST
    return 0;
#else
    // Get all mounted path
    NSArray *mounted = [[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:nil options:0];
    
    // Get mounted USB information
    for(NSURL *url in mounted) {
        char   f_mntfromname[MAX_PATH] = {0};
        char   f_mntonname[MAX_PATH] = {0};
        char   f_fstypename[] = "";

        GetMountPath([url.path UTF8String], f_mntonname, sizeof(f_mntonname), f_mntfromname, sizeof(f_mntfromname));
        if (f_mntfromname[0] == 0 || f_mntonname[0] == 0)
            continue;
        
        // 1. Volume Context Search.
        nBusType = VolCtx_Search_BusType( f_mntonname );
        if(nBusType == BusTypeUsb ||
           nBusType == BusType1394 ||
           nBusType == BusTypeThunderBolt ||
           nBusType == BusTypeAtapi ||
           nBusType == BusTypeSFolder)
        {
            switch(nBusType)
            {
                case BusType1394:
                case BusTypeUsb:
                    bUsbStor = TRUE;
                    break;
                case BusTypeThunderBolt:
                    bTBStor  = TRUE;
                    break;
                case BusTypeAtapi:
                    bCDStor  = TRUE;
                    break;
                case BusTypeSFolder:
                    bSFolder = TRUE;
                    break;
                default: break;
            }
        }
        else
        {  // 2. First Acess Check.
            bUsbStor = IsMediaPath_UsbStor( f_mntonname );
            if(bUsbStor)
            {
                nBusType = BusTypeUsb;
                VolCtx_Update( f_mntfromname, f_mntonname, BusTypeUsb );
            }

            bCDStor = IsMediaPath_CDStor( f_mntonname );
            if(bCDStor)
            {
                nBusType = BusTypeAtapi;
                VolCtx_Update( f_mntfromname, f_mntonname, BusTypeAtapi );
            }
            
            bSFolder = IsMediaPath_SFolder( f_mntfromname, f_mntonname, f_fstypename );
            if(bSFolder)
            {
                nBusType = BusTypeSFolder;
                VolCtx_Update( f_mntfromname, f_mntonname, BusTypeSFolder );
            }
        }
    }

    return (VFS_RETURNED);  //VFS_RETURENED_DONE
#endif    
}


boolean_t
IsControlDeviceType( const vnode_t pVnode, const char* pczPath )
{
    boolean_t bSFolder    = FALSE;
    boolean_t bUsbStor    = FALSE;
    boolean_t bCDStor     = FALSE;
    boolean_t bTBStor     = FALSE;
    boolean_t bVoulmesDir = FALSE;
    boolean_t bCups  = FALSE;

    //if(!pVnode) return FALSE;

    if(pczPath)
    {
        bCups = IsCupsDirectory( pczPath );
        if(TRUE == bCups) return TRUE;
    }
    
    if(pczPath)
    {
        bVoulmesDir = IsVolumesDirectory( pczPath );
        // printf("[DLP][%s] pczPath=%s \n", __FUNCTION__, pczPath );
    }

    char   f_mntonname[MAX_PATH] = {0};
    char   f_mntfromname[MAX_PATH] = {0};
    char   f_fstypename[] = "";
    if(bVoulmesDir)
    {
        int               nBusType = 0;
       
        // e.g.
        // f_mntonname     /Volumes/새 볼륨
        // f_mntfromname   disk2s2
        GetMountPath(pczPath, f_mntonname, sizeof(f_mntonname), f_mntfromname, sizeof(f_mntfromname));

        // 1. Volume Context Search.
        //nBusType = VolCtx_Search_BusType( Stat.f_mntonname );
        nBusType = VolCtx_Search_BusType( f_mntonname );
        
        if(nBusType == BusTypeUsb || nBusType == BusType1394 ||
           nBusType == BusTypeThunderBolt || nBusType == BusTypeAtapi || nBusType == BusTypeSFolder)
        {
            return TRUE;
        }

        // 2. First Acess Check.
        //bUsbStor = IsMediaPath_UsbStor( f_mntfromname );
        bUsbStor = IsMediaPath_UsbStor( f_mntonname );
        if(bUsbStor)
        {
            printf("[DLP][%s] UsbStor=1, fs=%s, from=%s, name=%s \n", __FUNCTION__, f_fstypename, f_mntfromname, f_mntonname );
            VolCtx_Update( f_mntfromname, f_mntonname, BusTypeUsb );
            return bUsbStor;
        }

        //bCDStor = IsMediaPath_CDStor( f_mntfromname );
        bCDStor = IsMediaPath_CDStor( f_mntonname );
        if(bCDStor)
        {
            printf("[DLP][%s] CDStor=1, fs=%s, from=%s, name=%s \n", __FUNCTION__, f_fstypename, f_mntfromname, f_mntonname );
            VolCtx_Update( f_mntfromname, f_mntonname, BusTypeAtapi );
            return bCDStor;
        }

        bSFolder = IsMediaPath_SFolder( f_mntfromname, f_mntonname, f_fstypename );
        if(bSFolder)
        {
            // '/Volumes/shared2/.DS_Store' skip
            printf("[DLP][%s] SFolder=1, fs=%s, from=%s, name=%s, pczPath=%s \n", __FUNCTION__, f_fstypename, f_mntfromname, f_mntonname, pczPath );
            if(NULL == sms_strnstr( pczPath, "/.DS_Store", strlen("/.DS_Store") ))
            {
                VolCtx_Update( f_mntfromname, f_mntonname, BusTypeSFolder );
                return bSFolder;
            }
            else
                return FALSE;
        }
    }

    return FALSE;
}

boolean_t
IsCupsdConfigFile( const char* pczPath )
{
    boolean_t bCups = FALSE;
    size_t nlen = 0;

    if(!pczPath)
        return FALSE;

    if(pczPath)
    {
        nlen = strlen(FILE_CUPSD_CONFIG);
        if(nlen > 0 && 0 == strncasecmp( pczPath, FILE_CUPSD_CONFIG, nlen))
        {
            bCups = TRUE;
        }
    }
    return bCups;
}

boolean_t
IsCupsDirectory(const char* pczPath)
{
    size_t nLength = 0;
    size_t nCups   = 0;

    if(!pczPath) return FALSE;

    nLength = strlen(pczPath);
    nCups   = strlen(g_czCupsSpoolPath);

    if(0 == nCups || nLength < nCups) return FALSE;

    if(0 == strncasecmp( pczPath, g_czCupsSpoolPath, nCups ))
    {
        size_t nLenght = (int)strlen(pczPath);
        int nAdd = ('/' == g_czCupsSpoolPath[nCups-1])?0:1;

        for(size_t i=nCups+nAdd; i<nLenght; i++)
        {
            // is file??
            if('/' == pczPath[i])
            {
                printf("[DLP][%s] Detected Path is not file=%s \n", __FUNCTION__, pczPath );
                return FALSE;
            }
        }
        printf("[DLP][%s] Detect Path=%s \n", __FUNCTION__, pczPath );
        return TRUE;
    }
    return FALSE;
}


boolean_t IsVolumesDirectory(const char* path)
{
    boolean_t volumesDir = FALSE;

    if (path != NULL &&
        strlen(path) >= strlen("/Volumes/") &&
        *(path+0) == '/' &&
        *(path+1) == 'V' &&
        *(path+2) == 'o' &&
        *(path+3) == 'l' &&
        *(path+4) == 'u' &&
        *(path+5) == 'm' &&
        *(path+6) == 'e' &&
        *(path+7) == 's' &&
        *(path+8) == '/')
    {
        volumesDir = TRUE;
    }
    return volumesDir;
}

void SetProtectUsbMobileNotify(void)
{
}

void SetProtect_Camera_UsbDeviceNotify(void)
{
}

void SetProtect_RNDIS_UsbDeviceNotify(void)
{
}

void SetProtect_RNDIS_BthDeviceNotify(void)
{
}


boolean_t
KextNotify_Init()
{
    return true;
}


boolean_t
KextNotify_Uninit()
{
    return true;
}
