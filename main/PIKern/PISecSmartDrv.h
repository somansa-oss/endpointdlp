
#ifndef _PISecSmartDrv_H_
#define _PISecSmartDrv_H_

#include "PISecSmartDataType.h"

extern DRV_KEXT       g_DrvKext;
extern DEVICE_TYPE_V6 g_pDeviceType_V6_List[];
extern char g_czCupsSpoolPathDump[MAX_FILE_LENGTH];
extern char g_czCupsSpoolPath[MAX_FILE_LENGTH];
extern char g_czCupsTempPath[MAX_FILE_LENGTH];


#ifdef __cplusplus
extern "C"
{
#endif

	kern_return_t PISecSmartDrv_start(kmod_info_t* pKmodInfo, void* d);
	kern_return_t PISecSmartDrv_stop(kmod_info_t* pKmodInfo, void* d);

	kern_return_t PISecSmartDrv_Init(void);
	kern_return_t PISecSmartDrv_Uninit(void);

	kern_return_t DrvKext_Init(void);
	kern_return_t DrvKext_Uninit(void);


    void
    GetMountPath(const char* pczPath, char* pczDevPath, int nBufferSize1, char* pczBSDPath, int nBufferSize2);

    int
    VolCtx_Search_BusType(const char* pczBasePath);

    boolean_t
    VolCtx_Update(char* pczDeviceName, char* pczBasePath, ULONG ulBusType );

    void
    VolCtx_Clear(void);

    int SendEventDataToPISupervisor(  void* pData, size_t nLength );

#ifdef __cplusplus
};
#endif

#endif
