#ifndef _KEXT_DEVICE_NOTIFY_H_
#define _KEXT_DEVICE_NOTIFY_H_

#ifdef LINUX
#else
#include <mach/mach_types.h>
#include <sys/vnode.h>
#endif

#include "PISecSmartDataType.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void FetchVolumes(void);

    boolean_t IsControlDeviceType(const vnode_t pVnode, const char* pczPath);

    boolean_t IsCupsDirectory(const char* pczPath);

    boolean_t IsCupsdConfigFile(const char* pczPath);

    boolean_t IsPrintControlType( const char* pczName, const char* pczPath );

    boolean_t IsVolumesDirectory(const char* path);

    // UsbStor
    boolean_t IsMediaPath_UsbStor(const char* pczPath);


	// CDStor
    boolean_t IsMediaPath_CDStor(const char* pczPath);


	// SFolder
    boolean_t IsMediaPath_SFolder(const char* pczDevice, const char* pczBasePath, const char* pczFsTypeName);

    void SetProtectUsbMobileNotify(void);
    void SetProtect_Camera_UsbDeviceNotify(void);
    void SetProtect_RNDIS_UsbDeviceNotify(void);
    void SetProtect_RNDIS_BthDeviceNotify(void);

    boolean_t SetProtect_RFCOMM(void* pService);

    boolean_t KextNotify_Init(void);
    boolean_t KextNotify_Uninit(void);
    
#ifdef __cplusplus
};
#endif

#endif 
