#ifndef	_NSPISECOBJECT_H
#define	_NSPISECOBJECT_H	1

namespace nsPISecObject
{
    typedef enum _EM_DEVICETYPE
    {
        deviceBegin			= 0x00,
        deviceNone			= 0x00,
        deviceRemovable		= 0x01,
        deviceEtcStorage	= 0x02,
        deviceCommDevice	= 0x04,
        deviceMedia			= (deviceEtcStorage | deviceCommDevice),
        deviceProcess		= 0x08,
        devicePrinter		= 0x10,
        deviceNetwork		= 0x20,
        deviceClipboard		= 0x40,
        deviceSceenCapture 	= 0x80,
        deviceSharedFolder	= 0x100,
        devicePCSecurity	= 0x200,
        deviceRemovableEx	= 0x400
    } EM_DEVICETYPE;

	typedef enum _EM_DLPSUBTYPE
    {
		dlpsubtypeNone					  = deviceNone,
        dlpsubtypeCopyPreventPlus		  = deviceRemovable,
        dlpsubtypeUploadPreventPlus		  = deviceNetwork,
        dlpsubtypePrintPreventPlus		  = devicePrinter,
        dlpsubtypeApplicationPreventPlus  = deviceProcess,
        dlpsubtypeClipboardPreventPlus	  = deviceClipboard,
        dlpsubtypeMediaControl			  = deviceMedia,
        dlpsubtypeScreenCaptureControl	  = deviceSceenCapture,
        dlpsubtypeSharedFolderPreventPlus = deviceSharedFolder,
        dlpsubtypePCSecurity			  = devicePCSecurity,
        dlpsubtypeCopyExPreventPlus		  = deviceRemovableEx
	} EM_DLPSUBTYPE;

	typedef enum _EM_DLPTYPE
	{
		dlptypeNone = 0,
        dlptypePreventPlus,	// subtype = EM_DEVICETYPE
        dlptypeControl,		// subtype = EM_DEVICETYPE
		dlptypeCommon		// subtype = EM_DLPCOMMON
	} EM_DLPTYPE;

	typedef enum _EM_DLPCOMMON
	{
		dlpcommonNone		= 0x00,
        dlpcommonSchedule	= 0x01,
        dlpcommonDecide	    = 0x02,
        dlpcommonDiscover   = 0x04
	} EM_DLPCOMMON;

	typedef enum _EM_DLPPOLICY_CONTROLMODE
	{	
		controlmodeNone		 = 0x00,
        controlmodeOffline	 = 0x01,
        controlmodeOnline	 = 0x02,
        controlmodeOnOffline = (controlmodeOffline | controlmodeOnline )
	} EM_DLPPOLICY_CONTROLMODE;

	typedef enum _EM_INSPECTOPTION
	{		
		inspectionNoInspect	 = 0, // "none"
		inspectionRunInspect = 1  // "inspect"
	}EM_INSPECTOPTION;

	typedef enum _EM_CONTROL_TARGETFILE
	{
		targetAll 		   = 0,
        targetPatternFound = 1
	}EM_CONTROL_TARGETFILE;	

	typedef enum _EM_CONTROL
	{		
		controlAllow	= 0, // "allow"
		controlBlock	= 1, // "block"
		controlApproval	= 2  // "approval"
	}EM_CONTROL;

	typedef enum _EM_NOTIFY
	{	
		notifyNone		= 0  // "none"
		, notifyAlways	= 1  // "always"
		, notifyBlock	= 2  // "block"
		, notifyAllow	= 3  // "allow"
	}EM_NOTIFY;
	
	typedef enum _EM_LOG_CONDITION
	{	
		logConditionNone		= 0x00
		, logConditionNotFound	= 0x01
		, logConditionFound		= 0x02
		, logConditionApproval	= 0x04		
		, logConditionAll	= ( logConditionNotFound | logConditionFound | logConditionApproval )
	}EM_LOG_CONDITION;

	typedef enum _EM_LOG
	{	
		logNone	= 0x00 // "none"
		, logAll = 0x01 // all"
		, logConditional = 0x02 // "conditional"
	}EM_LOG; 

	typedef enum _EM_MEDIA_PERMIT
	{
		mdPermitOwner		= 0x01,
		mdPermitOwnerDept	= 0x02,
		mdPermitDirect		= 0x04,
		mdPermitAll			= -1,
		mdPermitDefault		= mdPermitOwner
	} EM_MEDIA_PERMIT;

	typedef enum _EM_ETCSTORAGE_CONTROL
	{
		etcstorageBlockNone            	= 0x0000,
		etcstorageBlockNetDrive        	= 0x0001,
		etcstorageBlockCDDVDWrite    	= 0x0002,
		etcstorageBlockCDDVDRead    	= 0x0004, 
		etcstorageBlockFloppyWrite    	= 0x0008,
		etcstorageBlockFloppyRead    	= 0x0010, 
		etcstorageBlockRemovableWrite	= 0x0020,
		etcstorageBlockRemovableRead	= 0x0040,
	}EM_ETCSTORAGE_CONTROL;

	typedef enum _EM_COMMDEVICE_CONTROL
	{
		commdeviceNone 					= 0x0000,
		commdeviceBlockWLAN 			= 0x0001,
		commdeviceBlockWWAN 			= 0x0002,
		commdeviceBlockSerial 			= 0x0004,
		commdeviceBlockParallel 		= 0x0008,
		commdeviceBlockBluetooth 		= 0x0010,
		commdeviceBlockIRDA 			= 0x0020,
		commdeviceBlockIEEE1394 		= 0x0040,
		commdeviceBlockUSBMobile        = 0x0080,
        commdeviceBlockModem            = 0x0100, // 모뎀 (구 핸드폰 등) #145222 추가
        commdeviceBlockCamera           = 0x0200, // Camera Control 2018.08.23
        commdeviceBlockFileSharing      = 0x0400, // FileSharing      2019.02.07
        commdeviceBlockRemoteManagement = 0x0800, // RemoteManagement 2019.02.07
        commdeviceBlockAirDrop          = 0x1000  // AirDrop 2019.02.26
        
	}EM_COMMDEVICE_CONTROL;

	typedef enum _EM_WATERMARK_FILE
	{
		fileNone = -1,
		fileAll = 0,
		fileInspectFound = 1,
		fileInspectNotFound = 2
	}EM_WATERMARK_FILE;
}
#endif // #ifndef _NSPISECOBJECT_H
