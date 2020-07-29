#ifndef	_PISECSMARTDRVSTUB_CPP
#define _PISECSMARTDRVSTUB_CPP

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include "PISecSmartDrvStub.h"
#include "KextManager.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////////
//class CUSBMobile 
class CUSBMobile
{
	public:
		CUSBMobile(){};
		virtual ~CUSBMobile(){};

	public:
		typedef std::vector<CUSBMobile> Vector;

	public:
		std::string type;
		std::string basePath;
		std::string keyword;
};

////////////////////////////////////////////////////////////////////////////////
//class CPISecSmartDrvStub
CPISecSmartDrvStub::CPISecSmartDrvStub() { 
}

CPISecSmartDrvStub::~CPISecSmartDrvStub() {
}

bool CPISecSmartDrvStub::initialize(KERNEL_EVENTHANDLER kernel_eventhandler)
{
    for(int i=0; i<3; i++)
    {
        load();
        if(true == isActive())
        {
            break;
        }
    }
    
	if( false == isActive() )
    {
        return false;
	}

	result = g_AppKext.KextModule_Startup( kernel_eventhandler, false );
	if(result < 0)
    {
		return false;
	}
	initialized = true;
	return true;
}

bool CPISecSmartDrvStub::finalize(void)
{
	result = g_AppKext.KextModule_Cleanup( false );
	if( true == isActive() )
    {
		// unload();
	}
	return true;
}

void CPISecSmartDrvStub::clear(void)
{
}

bool CPISecSmartDrvStub::applyPolicy(unsigned long command, void* in, unsigned long size)
{
	if( false == isActive() )
    {
		return false;
	}

	bool emptyRule = false;
	switch(command)
    {
		case SetDrivePolicy : 
			//printf("pisecsmartdrv - set_drive_policy - size:%ld\n", size);

			if( (NULL == in) && (0 == size) )
            {
				in = malloc(sizeof(ATTACHED_DEVICE_EX));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
			}

			result = g_AppKext.fnSetDrivePolicy( in, (int)size);
            
			break;
		case SetPermitProcessName : 
			//printf("pisecsmartdrv - set_permit_process_name - size:%ld\n", size);

			if( (NULL == in) && (0 == size) ) {
				in = malloc(sizeof(ALLOWPROCESSNAME));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
			}

			result = g_AppKext.fnSetPermitProcessName( in, (int)size);

			break;
		case SetPermitFolderName : 
			//printf("pisecsmartdrv - set_permit_folder_name - size:%ld\n", size);

			if( (NULL == in) && (0 == size) ) {
				in = malloc(sizeof(ALLOWFOLDERNAME));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
			}

			result = g_AppKext.fnSetPermitFolderName( in, (int)size);
			break;
		case SetPermitFileExtName : 
			//printf("pisecsmartdrv - set_permit_file_ext_name - size:%ld\n", size);

			if( (NULL == in) && (0 == size) ) {
				in = malloc(sizeof(ALLOWFILEEXTNAME));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
			}

			result = g_AppKext.fnSetPermitFileExtName( in, (int)size);
			break;
		case SetExceptDrivePolicy : 
			//printf("pisecsmartdrv - set_except_drive_policy - size:%ld\n", size);

			if( (NULL == in) && (0 == size) ) {
				in = malloc(sizeof(ATTACHED_DEVICE));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
			}

			result = g_AppKext.fnSetExceptDrivePolicy( in, (int)size);
			break;
		case SetQuarantinePathExt :
			//printf("pisecsmartdrv - set_quarantine_path_ext - size:%ld\n", size);
			result = g_AppKext.fnSetQuarantinePathExt( (PQT_CTX)in);
			break;
		case SetQuarantineLimit :
			//printf("pisecsmartdrv - set_quarantine_limit- size:%ld\n", size);
			result = g_AppKext.fnSetQuarantineLimit( *((boolean_t*)in));
			break;
		case SetPrintPrevent : 
			//printf("pisecsmartdrv - set_print policy- size:%ld\n", size);

			if( (NULL == in) && (0 == size) ) {
				in = malloc(sizeof(PRINT_POLICY));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
			}

			result = g_AppKext.fnSetPrintPrevent( in, (int)size);
			break;
            
        case SetSelfProtect:
            result = g_AppKext.fnSetSelfProtect_Commit( *((boolean_t*)in), SetSelfProtect );
            break;
        case SetSelfProtectOnlyPermit:
            result = g_AppKext.fnSetSelfProtect_Commit( *((boolean_t*)in), SetSelfProtectOnlyPermit );
            break;
            
        case SetUploadPrevent:
            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(UPLOAD_POLICY));
				size = sizeof(ATTACHED_DEVICE);
				emptyRule = true;
            }
            result = g_AppKext.fnSetUploadPrevent( in, (int)size);
            break;
            
		default : break;
	}

	if (true == emptyRule) {
		free(in);
	}

	return (0 == result) ? true : false;
}


//
bool CPISecSmartDrvStub::applyPolicyAirDrop( const bool bAirDropBlock, const bool bAirDropLog )
{
    bool bResult = false;
    if(false == isActive())
    {
        return false;
    }
    bResult = g_AppKext.fnSetControlAirDrop( bAirDropBlock, bAirDropLog );
    return bResult;
}


// WLan Tethering
bool CPISecSmartDrvStub::applyPolicyRndis( const bool bRNdisCtrl, const bool bRNdisLog )
{
    bool bResult = false;
    if(false == isActive())
    {
        return false;
    }
    bResult = g_AppKext.fnSetControlRndis( bRNdisCtrl, bRNdisLog );
    return bResult;
}


bool CPISecSmartDrvStub::applyPolicyCamera(const bool bControl, const bool bLog )
{
    bool bResult = false;
    if(false == isActive())
    {
        return false;
    }
    
    bResult = g_AppKext.fnSetControlCamera( bControl, bLog );
    return bResult;
}


bool CPISecSmartDrvStub::applyPolicyUSBMobile(const bool control, const bool log, void* param, std::vector<struct _MB_PERMIT>* pPermitList /*=NULL*/ )
{
	if(false == isActive())
    {
		return false;
	}

	MB_POLICY Policy;
	memset( &Policy, 0, sizeof(Policy) );
	Policy.bMtpLog   = (true == log) ? TRUE : FALSE;
	Policy.bMtpBlock = (true == control) ? TRUE : FALSE;
	Policy.bPtpLog   = (true == log) ? TRUE : FALSE;
	Policy.bPtpBlock = (true == control) ? TRUE : FALSE;

    result = g_AppKext.fnClrUSBMobilePermitList();
    if(pPermitList)
    {
        result = g_AppKext.fnSetUSBMobilePermitList( pPermitList );
    }

	result = g_AppKext.fnSetCtrlMobile( &Policy );
	result = g_AppKext.fnMobileNodeRemoveAll();
    
	if((NULL == param) || (false == control))
    {
		return (0 == result) ? true : false;
	}

	CUSBMobile::Vector* list = reinterpret_cast<CUSBMobile::Vector*>(param);
	if( 0 == list->size() )
    {
		return (0 == result) ? true : false;
	}

	CUSBMobile::Vector::iterator itr = list->begin();
	for(; itr != list->end(); itr++)
    {
		CUSBMobile& usbmobile = *itr;
		if("Mtp" == usbmobile.type )
        {
			result = g_AppKext.fnMobileNodeInsert_SetMtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
		}
		else if("Ptp" == usbmobile.type )
        {
			result = g_AppKext.fnMobileNodeInsert_SetPtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
		}
	}

	return (0 == result) ? true : false;
}


bool CPISecSmartDrvStub::applyPolicySelectedBluetooth(const bool control, const bool log, void* param) {
    
    if( false == isActive() ) {
        return false;
    }
    
    MB_POLICY Policy;
    memset( &Policy, 0, sizeof(Policy) );
    
    Policy.bMtpLog   = (true == log) ? TRUE : FALSE;
    Policy.bMtpBlock = (true == control) ? TRUE : FALSE;
    Policy.bPtpLog   = (true == log) ? TRUE : FALSE;
    Policy.bPtpBlock = (true == control) ? TRUE : FALSE;
    
    result = g_AppKext.fnSetCtrlMobile( &Policy );
    result = g_AppKext.fnMobileNodeRemoveAll();
    
    if( (NULL == param) || (false == control) ) {
        return (0 == result) ? true : false;
    }
    
    CUSBMobile::Vector* list = reinterpret_cast<CUSBMobile::Vector*>(param);
    if( 0 == list->size() ) {
        return (0 == result) ? true : false;
    }
    
    CUSBMobile::Vector::iterator itr = list->begin();
    for(; itr != list->end(); itr++) {
        CUSBMobile& usbmobile = *itr;
        if( "Mtp" == usbmobile.type ) {
            result = g_AppKext.fnMobileNodeInsert_SetMtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
        }
        else if( "Ptp" == usbmobile.type ) {
            result = g_AppKext.fnMobileNodeInsert_SetPtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
        }
    }
    
    return (0 == result) ? true : false;
}

void CPISecSmartDrvStub::load(void)
{
    system("kextload -v 6 /Library/Extensions/PISecSmartDrv.kext");
}

void CPISecSmartDrvStub::unload(void)
{
	// system("kextunload -v 6 /Library/Extensions/PISecSmartDrv.kext");
}

bool CPISecSmartDrvStub::isActive(void)
{
	std::string command, temp;
	command = "kextstat | grep com.somansa.PISecSmartDrv";
	temp = util.readCommandOutput(command);

	if( true == temp.empty() )
    {
		return false;
	}

	std::istringstream is(temp);
	std::string token;
	std::getline(is, token);

	if( std::string::npos != token.find("PISecSmartDrv") )
    {
		return true;
	}

	return false;
}
		
bool CPISecSmartDrvStub::applyPolicyProcess(const bool control, const bool log, void* param)
{
	if( false == isActive() )
    {
		return false;
	}

    BOOLEAN bProcAC = (true == control) ? TRUE : FALSE;
    BOOLEAN bLog = (true == log) ? TRUE : FALSE;
    result = g_AppKext.fnSetProcessAccessCheck( bProcAC, bLog );

	return (0 == result) ? true : false;
}
#endif

