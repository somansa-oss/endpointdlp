#ifndef	_PIDEVICEMAN_CPP
#define	_PIDEVICEMAN_CPP

#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "PIDeviceMan.h"
#include "PIDocument.h"
#include "include_kernel.h"
#include "PIDeviceLog.h"
#include "PIActionProcessDeviceLog.h"
#include "PIDeviceController.h"
#include "PIDeviceManTest.h"

#ifndef LINUX
#include "PIDRDevice.h"
#endif

#include "PIEventHandler.h"

using namespace nsPISecObject;

bool IsOverBigSur()
{
#ifndef LINUX
    FILE* fpOut = popen("sw_vers -productVersion", "r");
    char cBuf[8] = { 0, };
    DEBUG_LOG("IsOverBigSur %d", 1);
    if (fpOut)
    {
        fgets(cBuf, sizeof(cBuf), fpOut);
        pclose(fpOut);
        
        std::string sVersionInfo = cBuf;
        
        size_t nFirst = sVersionInfo.find_first_of('.');
        size_t nLast  = sVersionInfo.find_last_of('.');
        
        if (nFirst == std::string::npos || nLast == std::string::npos)
            return false;
        
        int nMajor = atoi(sVersionInfo.substr(0, nFirst).c_str());
        int nMinor = 0;
        
        if (nFirst != nLast)
            nMinor = atoi(sVersionInfo.substr(nFirst + 1, nLast).c_str());
        else
            nMinor = atoi(sVersionInfo.substr(nFirst + 1, sVersionInfo.length() - 1).c_str());
        
        printf("Version Info nMajor: %d / nMinor: %d", nMajor, nMinor);
        
        if (nMajor >= 11 || nMinor >= 16)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
#endif    
    return false;
}

////////////////////////////////////////
// class CPIDeviceMan
CPIDeviceMan::CPIDeviceMan() : m_bProtect(false)
{
	//pthread_mutex_init(&mutexProcessCheck, 0);
}

CPIDeviceMan::~CPIDeviceMan() {	
	//pthread_mutex_unlock(&mutexProcessCheck);
	//pthread_mutex_destroy(&mutexProcessCheck);
}

bool CPIDeviceMan::initialize(void) {
	DEBUG_LOG1("device");

	CPIObject::initialize();
	
	bool result = false;

	DEBUG_LOG1("open PISecSmartDrv - begin");
	result = smartDrvStub.initialize(CPIDeviceMan::smartdrvEventHandler);
	if( false == result )
    {
		ERROR_LOG("open PISecSmartDrv - fail - result(%d)", smartDrvStub.result);
	}
	DEBUG_LOG("open PISecSmartDrv - %s - result(%d)", (result) ? "ok" : "fail", smartDrvStub.result);
	INFO_LOG("open PISecSmartDrv - %s", (result) ? "ok" : "fail");

	setQuarantineLimit();
    
    m_bProtect = true;
    setSelfProtect();
	return true;
}

bool CPIDeviceMan::finalize(void)
{
	DEBUG_LOG1("device");
    
    m_bProtect = false;
    setSelfProtect();
	
	smartDrvStub.finalize();
	deviceLogCache.clear();

	CPIObject::finalize();
	return true;
}

bool CPIDeviceMan::applyPolicy(void)
{
	DEBUG_LOG1("device");
	CPISecRule::List ruleList;
	if( false == PolicyMan.getCurrentRule(ruleList) )
    {
		DEBUG_LOG1("device - no_rule");
        applyPolicyDrive(ruleList);
        applyExceptPolicyDrive();
        applyPolicyCamera(ruleList);
        applyPolicyRndis(ruleList);
        applyPolicyAirDrop(ruleList);
		return false;
	}
	// ----------
	// application control
	// ----------
	applyPolicyProcess();
	// ----------
	// media control - usb_mobile 
	// ----------
	applyPolicyUSBMobile(ruleList);
    // Camera Control
    applyPolicyCamera(ruleList);
    // WWAN
    applyPolicyRndis(ruleList);
    // AirDrop
    applyPolicyAirDrop(ruleList);
    
	// ----------
	// copy prevent+
	// ----------
	setQuarantineFilePath();
	setAllowedProcess();
	setAllowedFolder();
	//applyPolicyDevice(ruleList);
	applyPolicyDrive(ruleList);	
	applyExceptPolicyDrive();
	// ----------
	// print prevent+
	// ----------
	applyPolicyPrintPrevent();
    // upload prevent+
    applyPolicyUploadPrevent();
    // PC Security
    applyPolicyPCSecurity();
	return true;
}


int CPIDeviceMan::GetDriveRuleCount(CPISecRule::List& ruleList)
{
    int nCount = 0;
    CPISecRule rule;
    CPISecRule::List::iterator itr;
    
    for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
    {
        CPISecRule& rule = *itr;
        if(false == isDriveRule(rule))
        {
            continue;
        }
        nCount++;
    }
    return nCount;
}

bool CPIDeviceMan::SetClearPolicyDrive(CPISecRule::List& ruleList)
{
    int nCount=0;
    CPISecRule rule;
    CPISecRule::List::iterator itr;
    
    nCount = GetDriveRuleCount( ruleList );
    if(nCount > 0)
    {
        DEBUG_LOG( "DriveRule - count - %d, failed.", ruleList.size() );
        return false;
    }
    
    rule.clear();
    rule.virtualType  = "Drive\\Floppy";
    rule.disableAll   = false;
    rule.disableRead  = false;
    rule.disableWrite = false;
    rule.enableLog    = false;
    rule.blockSelectedBluetooth = false;
    ruleList.push_back( rule );
    return true;
}

bool CPIDeviceMan::applyPolicyDrive(CPISecRule::List& ruleList)
{
	CPISecRule::List::iterator itr;
	unsigned long count = 0;
	PATTACHED_DEVICE_EX attached_device_ex_list = NULL;
	size_t struct_size = sizeof(ATTACHED_DEVICE_EX);

	if( 0 < ruleList.size() )
    {
		for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
        {
			CPISecRule& rule = *itr;
			if( false == isDriveRule(rule) )
            {
				continue;
			}
			count++;
		}
	}
    
    if(count <= 0)
    {   // count <= 0 --> Drive Policy Not Exist.
        DEBUG_LOG("DriveRule=%d, SetClearPolicyDrive. Start", count );
        if(true == SetClearPolicyDrive( ruleList ))
        {
            count = GetDriveRuleCount( ruleList );
        }
        DEBUG_LOG("DriveRule=%d, SetClearPolicyDrive. End. \n\n\n", count );
    }

	if(count > 0)
    {
		unsigned short index = 0;
		attached_device_ex_list = new ATTACHED_DEVICE_EX[count];
		for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
        {
			CPISecRule& rule = *itr;
			if( false == isDriveRule(rule) )
            {
				continue;
			}

			ATTACHED_DEVICE_EX& deviceRule = attached_device_ex_list[index++];
			memcpy(&deviceRule, rule.getData(), struct_size);
			deviceRule.cDevice.bDisableAll = FALSE; 

			// ----------
			DEBUG_LOG("[rule] type:0x%04x - log:%d - all:%d - read:%d - write:%d - reserved1:%d - name:%s", 
					deviceRule.ulStorDevType,
					deviceRule.cDevice.bLoggingOn,
					deviceRule.cDevice.bDisableAll,
					deviceRule.cDevice.bDisableRead,
					deviceRule.cDevice.bDisableWrite,
					deviceRule.cDevice.nReserved1,
					deviceRule.cDevice.DeviceNames
					);
			// ----------
		}
	}

	DEBUG_LOG("count - %d", count);
	bool result = smartDrvStub.applyPolicy(SetDrivePolicy, (void*) attached_device_ex_list, struct_size*count);
	INFO_LOG("apply drive policy - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("apply drive policy - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result )
    {
		ERROR_LOG("apply drive policy - failed - result(%d)", smartDrvStub.result);
	}

	if( NULL != attached_device_ex_list )
    {
		delete [] attached_device_ex_list;
		attached_device_ex_list = NULL;
	}
    
    if(count > 0)
    {
#ifndef LINUX		
        m_cFDA.FDAJobRequest();
#endif		
    }
	return true;
}


bool CPIDeviceMan::applyExceptPolicyDrive()
{
	CPISecRule::VECTOR_ATTACHED_DEVICE vectorAllow;
	CPISecRule::VECTOR_ATTACHED_DEVICE vectorBlock;
	
	CPIUSBDrive usbDrive;
	usbDrive.reload();

	// debug ----
	if (0 == usbDrive.list.size()) {
		DEBUG_LOG1("usb_drive_empty");
	} else {
		DEBUG_LOG("usb_drive_count:%d", usbDrive.list.size());
		for (int index = 0; index < usbDrive.list.size(); ++index) {
			CPIUSBDrive& temp = usbDrive.list[index];
			DEBUG_LOG("usb_drive - mount:%s - encrypted_serial:%s - UsbSerialPlain:%s", temp.getMountPoint().c_str(), temp.getEncryptedSerial().c_str(), temp.getUsbSerialPlain().c_str());
		}
	}
	// debug ----

	applyExceptPolicyDriveAllow(usbDrive, vectorAllow);
	applyExceptPolicyDriveBlock(usbDrive, vectorBlock);

	const size_t struct_size = sizeof(ATTACHED_DEVICE);
	size_t allow_count = vectorAllow.size();
	size_t block_count = vectorBlock.size();
	const unsigned long count = allow_count + block_count;
	PATTACHED_DEVICE exception_list = NULL;

	if( 0 < count ) {
		exception_list = new ATTACHED_DEVICE[count];
		PATTACHED_DEVICE temp = exception_list;

		PATTACHED_DEVICE allow_list = CPISecRule::getAttachedDevice(vectorAllow);
		if( (0 < allow_count) && (NULL != allow_list) ) {
			size_t allow_size = struct_size*allow_count;
			memcpy(temp, allow_list, allow_size);
			temp += allow_count;
		}
		
		PATTACHED_DEVICE block_list = CPISecRule::getAttachedDevice(vectorBlock);
		if( (0 < block_count) && (NULL != block_list) ) {
			size_t block_size = struct_size*block_count;
			memcpy(temp, block_list, block_size);
		}

		if( NULL != allow_list ) {
			delete [] allow_list;
			allow_list = NULL;
		}

		if( NULL != block_list ) {
			delete [] block_list;
			block_list = NULL;
		}
	}

	DEBUG_LOG("count - %d", count);
	bool result = smartDrvStub.applyPolicy(SetExceptDrivePolicy, (void*) exception_list, struct_size*count);
	INFO_LOG("set exception policy - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("set exception policy - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("set exception policy - failed - result(%d)", smartDrvStub.result);
	}

	if( NULL != exception_list ) {
		delete [] exception_list;
		exception_list = NULL;
	}

	return result;
}


//IsPolicyExistCopyPrevent

bool CPIDeviceMan::applyExceptPolicyDriveAllow(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorAllow)
{
    CPISecRule::List ruleList;
    if( false == PolicyMan.getCurrentRule(ruleList) )
    {
        return false;
    }
    
    bool log = false;
    bool found = false;
    bool bRead = false;
    bool bWrite = false;
    int nRemovableDrivePermitAllowControlType = 1;
    std::vector<MB_PERMIT> PermitList;
    CPISecRule::List::iterator itr;
    if( 0 < ruleList.size() ) {
        for(itr = ruleList.begin(); itr != ruleList.end(); itr++) {
            CPISecRule& rule = *itr;
            if( "Drive\\Removable" == rule.virtualType ) {
                log = (rule.getData()->cDevice.bLoggingOn) ? true : false;
                bRead = (rule.getData()->cDevice.bDisableRead) ? true : false;
                bWrite = (rule.getData()->cDevice.bDisableWrite) ? true : false;
                found = true;
                
                PermitList = rule.m_vecRemovableDrivePermitList;
                nRemovableDrivePermitAllowControlType =  rule.m_nRemovableDrivePermitAllowControlType;
                break;
            }
        }
    }
    
    DEBUG_LOG("control:%s - log:%s - count:%d, bRead:%d, bWrite:%d",
            util.getValueAsString(found).c_str(), util.getValueAsString(log).c_str(), PolicyMan.usbmobileList.size(),
              bRead, bWrite);
    
	size_t count = PermitList.size();
	if( 0 < count && bRead && bWrite) {
		//unsigned short index = 0;
        
		std::vector<MB_PERMIT>::iterator itr;
		for(itr = PermitList.begin(); itr != PermitList.end(); itr++) {
			std::string name((*itr).czMediaID);

            DEBUG_LOG("RemovableDrivePermitList name:%s", name.c_str());
            
            for (int index = 0; index < usbDrive.list.size(); ++index) {
                
                CPIUSBDrive& temp = usbDrive.list[index];
                DEBUG_LOG("usb_drive - mount:%s - UsbSerialPlain:%s", temp.getMountPoint().c_str(), temp.getUsbSerialPlain().c_str());
                
                if(name.length() > 0 && 0 == strncasecmp(temp.getUsbSerialPlain().c_str(),name.c_str(),name.length()))
                {
                    ATTACHED_DEVICE exception;
                    size_t size = temp.getMountPoint().length()*sizeof(char);
                    
                    memcpy(&(exception.DeviceNames), temp.getMountPoint().c_str(), (MAX_DEVICE_NAME > size) ? size : MAX_DEVICE_NAME-1);
                    exception.bLoggingOn = log;
                    exception.bDisableAll = 0;
                    
                    if(1 == nRemovableDrivePermitAllowControlType)
                    {
                        if(PolicyMan.getCurrentDLPPolicy( nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeCopyPreventPlus )
                           || PolicyMan.getCurrentDLPPolicy( nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeCopyExPreventPlus ))
                        {
                            // CopyPrevent+
                            exception.bDisableRead = 0;
                            exception.bDisableWrite = 1;
                            exception.nReserved1 = -1;
                        }
                        else
                        {
                            // read : allow
                            // write: allow
                            exception.bDisableRead = 0;
                            exception.bDisableWrite = 0;
                            exception.nReserved1 = 1;
                        }
                    }
                    else if(2 == nRemovableDrivePermitAllowControlType)
                    {
                        // read : allow
                        // write: block
                        exception.bDisableRead = 0;
                        exception.bDisableWrite = 1;
                        exception.nReserved1 = 1;
                    }
                    else
                    {
                        // read : allow
                        // write: allow
                        exception.bDisableRead = 0;
                        exception.bDisableWrite = 0;
                        exception.nReserved1 = 1;
                    }

                    vectorAllow.push_back(exception);

                    // ----------
                    DEBUG_LOG("[rule] log:%d - all:%d - read:%d - write:%d - reserved1:%d - name:%s",
                            exception.bLoggingOn,
                            exception.bDisableAll,
                            exception.bDisableRead,
                            exception.bDisableWrite,
                            exception.nReserved1,
                            exception.DeviceNames
                            );
                    // ----------
                }
            }
		}
	}

	DEBUG_LOG("count - %d", count);
	return true;
}

bool CPIDeviceMan::applyExceptPolicyDriveBlock(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorBlock) {
	applyExceptPolicyUSBReadBlockByUnregister(usbDrive, vectorBlock);
	applyExceptPolicyUSBReadBlockByCopyPrevent(usbDrive, vectorBlock);

	return true;
}
	

bool CPIDeviceMan::applyExceptPolicyUSBReadBlockByUnregister(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorBlock) {
	if (false == ConfigMan.unRegisterUSBReadBlockUse) {
		DEBUG_LOG1("skip - option_inactive");
		return true;
	}

	CDLPPolicyCopy* policyCopy = dynamic_cast<CDLPPolicyCopy*>(PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeCopyPreventPlus));
	if ((NULL != policyCopy) && (true == policyCopy->isBlockReadAll())) {
		DEBUG_LOG1("skip - policy_copy - use_block_raad_all");
		return true;
	}

	CDLPPolicyMedia* policyMedia = dynamic_cast<CDLPPolicyMedia*>(PolicyMan.getCurrentDLPPolicy(dlptypeControl, dlpsubtypeMediaControl));
	if ((NULL != policyMedia) && (true == policyMedia->getControl(deviceEtcStorage, etcstorageBlockRemovableRead))) {
		DEBUG_LOG1("skip - policy_media - block_removable_read");
		return true;
	}

	std::vector<std::string> foundList;
	if (0 < usbDrive.list.size()) {
		CPIUSBDrive::VECTOR::iterator itr = usbDrive.list.begin();
		for (; itr != usbDrive.list.end(); ++itr) {
			if (false == PolicyMan.isRegisteredUSB(itr->getEncryptedSerial())) {
				foundList.push_back(itr->getMountPoint());
				DEBUG_LOG("found - mount_point:%s", itr->getMountPoint().c_str());
			}
		}
	}

	size_t count = foundList.size();
	if (0 < count) {
		for (int index = 0; index < count; ++index) {
			std::string& name = foundList[index];
			size_t size = name.length()*sizeof(char);
			ATTACHED_DEVICE exception;
			memcpy(&(exception.DeviceNames), name.c_str(), (MAX_DEVICE_NAME > size) ? size : MAX_DEVICE_NAME-1);
			exception.bLoggingOn = 1;
			exception.bDisableAll = 0;
			exception.bDisableRead = 1;
			exception.bDisableWrite = 1;
			exception.nReserved1 = -1;
			vectorBlock.push_back(exception);
			
			// ----------
			DEBUG_LOG("[rule] log:%d - all:%d - read:%d - write:%d - reserved1:%d - name:%s", 
					exception.bLoggingOn,
					exception.bDisableAll,
					exception.bDisableRead, 
					exception.bDisableWrite, 
					exception.nReserved1,
					exception.DeviceNames
					);
			// ----------
		}
	}

	DEBUG_LOG("count - %d", count);
	return true;
}

bool CPIDeviceMan::applyExceptPolicyUSBReadBlockByCopyPrevent(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorBlock) {
	CDLPPolicyCopy* policyCopy = dynamic_cast<CDLPPolicyCopy*>(PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeCopyPreventPlus));
	if (NULL == policyCopy) {
		DEBUG_LOG1("stop - policy_copy_not_found"); 
		return true;
	}

	if (true == policyCopy->isBlockReadAll()) {
		DEBUG_LOG1("skip - block_read_all");
		return true;
	}

	if (0 == usbDrive.list.size()) {
		DEBUG_LOG1("skip - usb_drive_empty");
		return true;
	}

	std::vector<std::string> foundList;
	CPIUSBDrive::VECTOR::iterator itr = usbDrive.list.begin();
	for (; itr != usbDrive.list.end(); ++itr) {
		CDLPUSBDrive dlpUsbDrive;
		PolicyMan.getDLPUSBDrive(itr->getMountPoint(), itr->getEncryptedSerial(), dlpUsbDrive);
	
		CDLPCopyControl copyControl;
		if (false == policyCopy->getDLPCopyControl(&dlpUsbDrive, copyControl)) {
			DEBUG_LOG1("copy_control - not_found");
			continue;
		}

		// ----------
		DEBUG_LOG("copy_control - found - select_type:%d - is_read_permit:%s", 
				copyControl.usbDriveControl.getSelectType(), 
				(copyControl.usbDriveControl.isReadPermit())?"true":"false");
		// ----------

		if (false == copyControl.usbDriveControl.isReadPermit()) {
			foundList.push_back(itr->getMountPoint());
			DEBUG_LOG("found - mount_point:%s", itr->getMountPoint().c_str());
		}
	}

	size_t count = foundList.size();
	if (0 < count) {
		for (int index = 0; index < count; ++index) {
			std::string& name = foundList[index];
			size_t size = name.length()*sizeof(char);
			ATTACHED_DEVICE exception;
			memcpy(&(exception.DeviceNames), name.c_str(), (MAX_DEVICE_NAME > size) ? size : MAX_DEVICE_NAME-1);
			exception.bLoggingOn = 1;
			exception.bDisableAll = 0;
			exception.bDisableRead = 1;
			exception.bDisableWrite = 1;
			exception.nReserved1 = -1;
			vectorBlock.push_back(exception);

			// ----------
			DEBUG_LOG("[rule] log:%d - all:%d - read:%d - write:%d - reserved1:%d - name:%s", 
					exception.bLoggingOn,
					exception.bDisableAll,
					exception.bDisableRead, 
					exception.bDisableWrite, 
					exception.nReserved1,
					exception.DeviceNames
					);
			// ----------
		}
	}

	DEBUG_LOG("count - %d", count);
	return true;
}

bool CPIDeviceMan::isDeviceRule(CPISecRule& rule) {

	bool result = false;
	
	if( 
			( "Device\\Serial" == rule.virtualType ) ||
			( "Device\\Parallel" == rule.virtualType ) ||
			( "Device\\IEEE1394" == rule.virtualType ) ||
			( "Device\\CD/DVD" == rule.virtualType )
	  ) {
		result = true;
	}

	return result;
}

bool CPIDeviceMan::isDriveRule(CPISecRule& rule) {

	bool result = false;
	
	if( 
			( "Drive\\Removable" == rule.virtualType ) ||
			( "Drive\\Floppy" == rule.virtualType ) ||
			( "Drive\\CD/DVD" == rule.virtualType ) ||
            ( "Device\\Bluetooth" == rule.virtualType ) ||
			( "Drive\\NetDrive" == rule.virtualType )
	  ) {
		result = true;
	}

	return result;
}

bool CPIDeviceMan::applyPolicyUSBMobile(CPISecRule::List& ruleList) {

    bool log = false;
	bool found = false;
    std::vector<MB_PERMIT> PermitList;
	CPISecRule::List::iterator itr;
	if( 0 < ruleList.size() ) {
		for(itr = ruleList.begin(); itr != ruleList.end(); itr++) {
			CPISecRule& rule = *itr;
			if( "Device\\USBMobile" == rule.virtualType ) {
                log = (rule.getData()->cDevice.bLoggingOn) ? true : false;
				if (rule.getData()->cDevice.bDisableAll) {
					found = true; 
					PermitList = rule.m_vecUSBMobilePermitList;
				}
				break;
			}
		}
	}

	if( true == found ) {
		PolicyMan.debugPrintUSBMobile();
	}
	
	DEBUG_LOG("control:%s - log:%s - count:%d", 
			util.getValueAsString(found).c_str(), util.getValueAsString(log).c_str(), PolicyMan.usbmobileList.size());

	bool result = smartDrvStub.applyPolicyUSBMobile(found, log, (void*)&PolicyMan.usbmobileList, &PermitList );
	INFO_LOG("apply policy usbmobile - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("apply policy usbmobile - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("apply policy usbmobile - failed - result(%d)", smartDrvStub.result);
	}

	return result;
}

bool CPIDeviceMan::applyPolicySelectedBluetooth(CPISecRule::List& ruleList) {
    
    bool log = false;
    bool found = false;
    CPISecRule::List::iterator itr;
    if( 0 < ruleList.size() ) {
        for(itr = ruleList.begin(); itr != ruleList.end(); itr++) {
            CPISecRule& rule = *itr;
            if( "Device\\USBMobile" == rule.virtualType ) {
                log = (rule.getData()->cDevice.bLoggingOn) ? true : false;
                found = true;
                break;
            }
        }
    }
    
    if( true == found ) {
        PolicyMan.debugPrintUSBMobile();
    }
    
    DEBUG_LOG("control:%s - log:%s - count:%d",
              util.getValueAsString(found).c_str(), util.getValueAsString(log).c_str(), PolicyMan.usbmobileList.size());
    
    bool result = smartDrvStub.applyPolicyUSBMobile(found, log, (void*)&PolicyMan.usbmobileList);
    INFO_LOG("apply policy bluetooth - %s", (true == result) ? "ok" : "failed");
    DEBUG_LOG("apply policy bluetooth - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
    if( false == result ) {
        ERROR_LOG("apply policy bluetooth - failed - result(%d)", smartDrvStub.result);
    }
    
    return result;
}


bool CPIDeviceMan::applyPolicyCamera( CPISecRule::List& ruleList )
{
    bool bLog = false;
    bool bControl = false;
    bool bResult = false;
    
    CPISecRule::List::iterator itr;
    if( 0 < ruleList.size() )
    {
        for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
        {
            CPISecRule& rule = *itr;
            if( "Device\\Camera" == rule.virtualType )
            {
                bLog = (rule.getData()->cDevice.bLoggingOn) ? true : false;
				if (rule.getData()->cDevice.bDisableAll) {
					bControl = true;
				}
                break;
            }
        }
    }
    
    DEBUG_LOG("control:%s - log:%s - count:%d", util.getValueAsString(bControl).c_str(), util.getValueAsString(bLog).c_str() );
    
    bResult = smartDrvStub.applyPolicyCamera( bControl, bLog );
    
    INFO_LOG("apply policy Camera - %s", (true == bResult) ? "ok" : "failed");
    DEBUG_LOG("apply policy Camera - %s - result(%d)", (true == bResult) ? "ok" : "failed", smartDrvStub.result);
    if(false == bResult )
    {
        ERROR_LOG("apply policy Camera - failed - result(%d)", smartDrvStub.result);
    }
    return bResult;
}


bool CPIDeviceMan::applyPolicyAirDrop( CPISecRule::List& ruleList )
{
    bool bLog = false;
    bool bControl = false;
    bool bResult = false;
    
    if(true == ConfigMan.getAirDropProtectUse())
    {
        DEBUG_LOG1("true == getAirDropProtectUse()");
        CPISecRule::List::iterator itr;
        if( 0 < ruleList.size() )
        {
            for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
            {
                CPISecRule& rule = *itr;
                if( "Share\\AirDrop" == rule.virtualType )
                {
                    bLog = (rule.getData()->cDevice.bLoggingOn) ? true : false;
					if (rule.getData()->cDevice.bDisableAll) {
						bControl = true;
					}
					break;
                }
            }
        }
    }
    else
    {
        DEBUG_LOG1("false == getAirDropProtectUse()");
    }
    
    DEBUG_LOG("control:%s - log:%s - count:%d", util.getValueAsString(bControl).c_str(), util.getValueAsString(bLog).c_str() );
    
    bResult = smartDrvStub.applyPolicyAirDrop( bControl, bLog );
    
    INFO_LOG("apply policy AirDrop - %s", (true == bResult) ? "ok" : "failed");
    DEBUG_LOG("apply policy AirDrop - %s - result(%d)", (true == bResult) ? "ok" : "failed", smartDrvStub.result);
    if(false == bResult )
    {
        ERROR_LOG("apply policy AirDrop - failed - result(%d)", smartDrvStub.result);
    }
    return bResult;
}


// WLan Tethering
bool CPIDeviceMan::applyPolicyRndis( CPISecRule::List& ruleList )
{
    bool bLog = false;
    bool bControl = false;
    bool bResult = false;
    
    CPISecRule::List::iterator itr;
    if(ruleList.size() > 0)
    {
        for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
        {
            CPISecRule& rule = *itr;
            if("NDIS\\WWAN" == rule.virtualType)
            {
                bLog = (rule.getData()->cDevice.bLoggingOn) ? true : false;
				if (rule.getData()->cDevice.bDisableAll) {
					bControl = true;
				}
                break;
            }
        }
    }
    
    DEBUG_LOG("control:%s - log:%s - count:%d", util.getValueAsString(bControl).c_str(), util.getValueAsString(bLog).c_str() );
    
    bResult = smartDrvStub.applyPolicyRndis( bControl, bLog );
    
    INFO_LOG("apply policy RNDIS - %s", (true == bResult) ? "ok" : "failed");
    DEBUG_LOG("apply policy RNDIS - %s - result(%d)", (true == bResult) ? "ok" : "failed", smartDrvStub.result);
    if(false == bResult )
    {
        ERROR_LOG("apply policy RNDIS - failed - result(%d)", smartDrvStub.result);
    }
    return bResult;
}


bool CPIDeviceMan::applyPolicyPCSecurity(void)
{
    INFO_LOG1( "Begin" );
    
    CDLPPolicyPCSecurity* pDLPPCSecurity=NULL;
    
    pDLPPCSecurity = (CDLPPolicyPCSecurity*)PolicyMan.getCurrentDLPPolicy( dlptypeControl, dlpsubtypePCSecurity );
    if(pDLPPCSecurity)
    {
        m_cPCSecurity.Load( pDLPPCSecurity->m_ShareFolder );
        if(m_cPCSecurity.getSFThread())
        {
            m_cPCSecurity.PCSecurity_SFThread_Stop();
        }
        m_cPCSecurity.PCSecurity_SFThread_Start();
    }
    else
    {
        if(m_cPCSecurity.getSFThread())
        {
            m_cPCSecurity.PCSecurity_SFThread_Stop();
            sleep( 1 );
        }
    }
    
    INFO_LOG1( "Finish" );
    return true;
}

int CPIDeviceMan::smartdrvEventHandler(void* param)
{
	//DEBUG_LOG1( "begin");
    
	if(NULL == param) {
		DEBUG_LOG1( "skip - invalid param");
		return 0;
	}
	
	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
	//DEBUG_LOG( "begin - command:%d", pEvtInfo->Command);
    
	int result = 0;
    switch(pEvtInfo->Command)
    {
        case FileIsRemove:
            result = DeviceMan.smartdrvFileIsRemove(param);
            break;
        case FileScan:
            result = DeviceMan.smartdrvFileScan(param);
            break;
        case FileDelete:
            result = DeviceMan.smartdrvFileDelete(param);
            break;
        case FileExchangeData:
            result = DeviceMan.smartdrvFileExchangeData(param);
            break;
        case FileRename:
            result = DeviceMan.smartdrvFileRename(param);
            break;
        case FileEventDiskFull:
            result = DeviceMan.smartdrvFileEventDiskFull(param);
            break;
        case SmartLogNotify:
            result = DeviceMan.smartdrvSmartLogNotify(param);
            break;
		case ProcessCallback:
			result = DeviceMan.smartdrvProcessCallback(param);
			break;
		case ProcessAccessCheck:
			result = DeviceMan.smartdrvProcessAccessCheck(param);
            break;
		default: 
			DEBUG_LOG( "unknown command:%d", pEvtInfo->Command);
			break;
	}

	//DEBUG_LOG( "end - result:%d - command:%d", result, pEvtInfo->Command);
    return result;
}

int CPIDeviceMan::smartdrvFileIsRemove(void* param) {
	if( NULL == param ) {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
    DEBUG_LOG( "pid=%d, path=%s ", pEvtInfo->ProcessId, pEvtInfo->pFilePath );

	// do nothing
    return 0;
}


int CPIDeviceMan::smartdrvFileScan(void* param) {
	if( NULL == param ) {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
	DEBUG_LOG( "pid=%d, filepath=%s, qt_filepath=%s, bus_type=0x%02x", pEvtInfo->ProcessId, pEvtInfo->pFilePath, pEvtInfo->pQtFilePath, pEvtInfo->ullReserved);

	bool allow = true;
	PolicyMan.getLock();
	if( ActionTypePrint == pEvtInfo->ullReserved
       || ActionTypeUpload == pEvtInfo->ullReserved){
		// case) Prevent Prevent+
		std::string pdfPath = pEvtInfo->pQtFilePath;
        std::string pdfOrgPath = pEvtInfo->pFilePath;
		if( true == pdfPath.empty() ) {
			pdfPath = pEvtInfo->pFilePath;
		}
		unsigned int processId = pEvtInfo->ProcessId;
        
        ERROR_LOG( "smartdrvFileScan - ProcessId: [%d],", pEvtInfo->ProcessId);
		
		if( pdfPath.empty() ) {
			//do nothing
			DEBUG_LOG1("inspect_file - skipped - empty filepath");
		}
        else if(ActionTypeUpload == pEvtInfo->ullReserved && 0 != access( pdfPath.c_str(), 0))
        {
            DEBUG_LOG( "[DLP][%s] File-NotExist., Path=%s \n", __FUNCTION__, pdfPath.c_str() );
            allow = true;
        }
		else {
			// send message
			int result = 0;
			PIAgentStub.analyzeFileForPreventPlus(pdfPath, pdfOrgPath, processId, (ACTION_TYPE)pEvtInfo->ullReserved, result);

			if( 1 == result ) {
				allow = false;
			}
		}
	}
	else
    {
		// case) copy prevent+ or sharedfolder prevent+
		bool sharedFolder = isSharedFolder(pEvtInfo->ullReserved);
		
		if( 
				(false == sharedFolder) &&
				( NULL == PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeCopyPreventPlus) ) 
		  ) {
			// case) copy prevent+ and policy not found
			DEBUG_LOG1("inspect_file - skipped - dlppolicy not found");

			if( true == DeviceMan.isMediaControlUSBReadAllowWriteBlock()) {
				// case) media control : usb - read:allow - write:block
				allow = false;
			}
		}
		else
        {
			// case 1) sharedfolder prevent+
			// case 2) copy prevent+ and policy exist
			std::string filePath = pEvtInfo->pFilePath;
			std::string tempFilePath = pEvtInfo->pQtFilePath;
			unsigned int processId = pEvtInfo->ProcessId;
			unsigned int deviceCategory = getDeviceCategory(sharedFolder);
			if( filePath.empty() && tempFilePath.empty() ) {
				//do nothing
				DEBUG_LOG1("inspect_file - skipped - empty filepath");
			}
			else {
				// send message
				int result = 0;
				PIAgentStub.analyzeFileForCopyPreventPlus(filePath, tempFilePath, deviceCategory, processId, result);

				if( 1 == result ) {
					allow = false;
				}
			}
		}
	}
	PolicyMan.releaseLock();

	pEvtInfo->bAccess = (true == allow) ? 1 : 0;

	return 0;
}

int CPIDeviceMan::smartdrvFileDelete(void* param) {
	if( NULL == param ) {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
    DEBUG_LOG( "pid=%d, path=%s ", pEvtInfo->ProcessId, pEvtInfo->pFilePath );

	PolicyMan.getLock();
	bool allow = true;
	CDLPPolicy* policy = PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeCopyPreventPlus);
	if( NULL == policy ) {
		DEBUG_LOG1("inspect_file - skipped - dlppolicy not found");
		if( true == DeviceMan.isMediaControlUSBReadAllowWriteBlock()) {
			// media control case : usb - read:allow - write:block
			allow = false;
		}
	}
	else {

		std::string filePath = pEvtInfo->pFilePath;
		std::string tempFilePath = pEvtInfo->pQtFilePath;
		unsigned int processId = pEvtInfo->ProcessId;
		if( filePath.empty() ) {
			//do nothing
		}
		else {
			// send message
			int result = 0;
			PIAgentStub.notifyFileDelete(filePath, processId, result);
			if( 1 == result) {
				allow = false;
			}
		}
	}
	PolicyMan.releaseLock();
	
	pEvtInfo->bAccess = (true == allow) ? 1 : 0;

	return 0;
}

int CPIDeviceMan::smartdrvFileRename(void* param) {
	if( NULL == param ) {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
    DEBUG_LOG( "pid=%d, path=%s ", pEvtInfo->ProcessId, pEvtInfo->pFilePath );
    
	//do nothing
	
	return 0;
}

int CPIDeviceMan::smartdrvFileExchangeData(void* param) {
	if( NULL == param ) {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
    DEBUG_LOG( "pid=%d, path=%s ", pEvtInfo->ProcessId, pEvtInfo->pFilePath );

	//do nothing
   
	return 0;
}

int CPIDeviceMan::smartdrvFileEventDiskFull(void* param) {
	if( NULL == param ) {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
    DEBUG_LOG( "pid=%d, path=%s ", pEvtInfo->ProcessId, pEvtInfo->pFilePath );

	std::string jobStatus = "0x00000000";
	std::string filePath = pEvtInfo->pFilePath;

	// send message
	int result = 0;
	PIAgentStub.notifyDiskFull(jobStatus, filePath, result);
	return 0;
}


int CPIDeviceMan::smartdrvSmartLogNotify(void* param )
{
	if( NULL == param )
    {
		return 0;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
	PSMART_LOG_RECORD_EX pLogEx = NULL;

	pLogEx = (PSMART_LOG_RECORD_EX)pEvtInfo->pEvtCtx;
	if(!pLogEx) return 0;
	
	// ----------
	DEBUG_LOG( "log_time=%s, policy_type=0x%08x, access_type=0x%04x, record_type=0x%04x, log_type=0x%04x, log_action=0x%04x, disable_result=%d, pid=%d, proc_name=%s, file_name=%s",
			pLogEx->Log.Data.LogTime,
			pLogEx->PolicyType,
			pLogEx->Log.AccessType,
			pLogEx->Log.RecordType,
			pLogEx->Log.Data.LogType,
			pLogEx->Log.Data.LogAction,
			(int)pLogEx->Log.Data.bDisableResult,
			pLogEx->Log.Data.ProcessId,
			pLogEx->Log.Data.ProcName,
			pLogEx->Log.Data.FileName  );
	// ----------

	adjustSmartLog(param);

	if( LOG_FILEOP == pLogEx->Log.Data.LogType && TRUE == pLogEx->Log.Data.bDisableResult)
    {
		DEBUG_LOG1("skip - prevent_log");
		return 0; 
	}

	const int POLICY_TYPE = GET_STORDEVICETYPE(pLogEx->PolicyType);

	if (true == isMediaControlLog(POLICY_TYPE)) {
		if( false == PolicyMan.enableMediaControlBlockLog ) {
			DEBUG_LOG1("skip - enable_media_control_block_log:false");
			return 0;
		}
	}

	if (MEDIA_PROCESS == POLICY_TYPE) {
		DEBUG_LOG1("skip - media_process_log ");
		return 0;
	}

	CPIDeviceLog deviceLog;
	deviceLog.setDeviceTypeByPolicyType(pLogEx->PolicyType);
	deviceLog.policyType 	= pLogEx->PolicyType;
	deviceLog.accessType 	= pLogEx->Log.AccessType;
	deviceLog.recordType 	= pLogEx->Log.RecordType;
	deviceLog.setLogTime( 	  pLogEx->Log.Data.LogTime);
	deviceLog.logType 	= pLogEx->Log.Data.LogType;
	deviceLog.logAction 	= pLogEx->Log.Data.LogAction;
	deviceLog.disableResult = util.getValueAsBool((int)pLogEx->Log.Data.bDisableResult);
	deviceLog.processId 	= pLogEx->Log.Data.ProcessId;
	deviceLog.processName 	= pLogEx->Log.Data.ProcName;
	deviceLog.fileName 		= pLogEx->Log.Data.FileName;
	deviceLog.setDeviceName(std::string(pLogEx->Log.Data.FileName));
	CPIActionProcessDeviceLog::getInstance().addDeviceLog(deviceLog);

	return 0;
}

bool CPIDeviceMan::isMediaControlLog(int storDeviceType)  {
	switch(storDeviceType) 
	{
		case 0x0000 :
		case MEDIA_COPYPREVENT :
		case MEDIA_MTP :
		case MEDIA_PTP :
		case MEDIA_PRINTPREVENT :
		case MEDIA_UPLOADPREVENT :
		case MEDIA_PROCESS : return false; break;
		default: break;
	}

	return true;
}

bool CPIDeviceMan::isMediaControlUSBReadAllowWriteBlock(void)
{
	CPISecRule::List ruleList;
	PolicyMan.getCurrentRule(ruleList);
	
	if( 0 == ruleList.size() ) {
		return false;
	}

	bool result = false;
	CPISecRule::List::iterator itr = ruleList.begin();
	for( ; itr != ruleList.end(); ++itr) {
		if( "Drive\\Removable" == itr->virtualType) {
			if( (false == itr->disableRead) && (true == itr->disableWrite) ) {
				result = true;
			}
			break;
		}
	}

	return result;
}

bool CPIDeviceMan::setAllowedProcess(void) {
	
	std::vector<std::string>::iterator itr;
	unsigned long count = 0;
	PALLOWPROCESSNAME process_list = NULL;
	size_t struct_size = sizeof(ALLOWPROCESSNAME);

	count = ConfigMan.allowProcessList.size();
	if( 0 < count ) {

		unsigned short index = 0;
		process_list = new ALLOWPROCESSNAME[count];
		memset(process_list, 0x00, struct_size*count);

		for(itr = ConfigMan.allowProcessList.begin(); itr != ConfigMan.allowProcessList.end(); itr++) {
			std::string& name = *itr;
			size_t size = name.length()*sizeof(char);

			ALLOWPROCESSNAME& process = process_list[index++];
			memcpy(&process.czAllowProcName, name.c_str(), (MAX_PROC_NAME > size) ? size : MAX_PROC_NAME-1);

			// ----------
			DEBUG_LOG("[process_name] - %s", process.czAllowProcName);
			// ----------
		}
	}

	DEBUG_LOG("count - %d", count);
	bool result = smartDrvStub.applyPolicy(SetPermitProcessName, (void*) process_list, struct_size*count);
	INFO_LOG("set process name - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("set process name - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("set process name - failed - result(%d)", smartDrvStub.result);
	}

	if( NULL != process_list ) {
		delete [] process_list;
		process_list = NULL;
	}

	return true;
}

bool CPIDeviceMan::setAllowedFolderName(void) {
	
	std::vector<std::string>::iterator itr;
	unsigned long count = 0;
	PALLOWFOLDERNAME folder_list = NULL;
	size_t struct_size = sizeof(ALLOWFOLDERNAME);

	count = ConfigMan.allowFolderList.size();
	if( 0 < count ) {

		unsigned short index = 0;
		folder_list = new ALLOWFOLDERNAME[count];
		memset(folder_list, 0x00, struct_size*count);

		for(itr = ConfigMan.allowFolderList.begin(); itr != ConfigMan.allowFolderList.end(); itr++) {
			std::string& name = *itr;
			size_t size = name.length()*sizeof(char);

			ALLOWFOLDERNAME& folder = folder_list[index++];
			memcpy(&folder.czAllowFolder, name.c_str(), (MAX_FOLDER_NAME > size) ? size : MAX_FOLDER_NAME-1);

			// ----------
			DEBUG_LOG("[folder_name] - %s", folder.czAllowFolder);
			// ----------
		}
	}

	DEBUG_LOG("count - %d", count);
	bool result = smartDrvStub.applyPolicy(SetPermitFolderName, (void*) folder_list, struct_size*count);
	INFO_LOG("set folder name - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("set folder name - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("set folder name - failed - result(%d)", smartDrvStub.result);
	}

	if( NULL != folder_list ) {
		delete [] folder_list;
		folder_list = NULL;
	}

	return true;
}

bool CPIDeviceMan::setAllowedFileExtName(void) {
	
	std::vector<std::string>::iterator itr;
	unsigned long count = 0;
	PALLOWFILEEXTNAME file_ext_list = NULL;
	size_t struct_size = sizeof(ALLOWFILEEXTNAME);

	count = ConfigMan.allowFileExtList.size();
	if( 0 < count ) {

		unsigned short index = 0;
		file_ext_list = new ALLOWFILEEXTNAME[count];
		memset(file_ext_list, 0x00, struct_size*count);

		for(itr = ConfigMan.allowFileExtList.begin(); itr != ConfigMan.allowFileExtList.end(); itr++) {
			std::string& name = *itr;
			size_t size = name.length()*sizeof(char);

			ALLOWFILEEXTNAME& file_ext = file_ext_list[index++];
			memcpy(&file_ext.czAllowFileExt, name.c_str(), (MAX_FILEEXT_NAME > size) ? size : MAX_FILEEXT_NAME-1);

			// ----------
			DEBUG_LOG("[file_ext_name] - %s", file_ext.czAllowFileExt);
			// ----------
		}
	}

	DEBUG_LOG("count - %d", count);
	bool result = smartDrvStub.applyPolicy(SetPermitFileExtName, (void*) file_ext_list, struct_size*count);
	INFO_LOG("set file_ext name - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("set file_ext name - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("set file_ext name - failed - result(%d)", smartDrvStub.result);
	}

	if( NULL != file_ext_list ) {
		delete [] file_ext_list;
		file_ext_list = NULL;
	}

	return true;
}

bool CPIDeviceMan::setAllowedFolder(void) {

	setAllowedFolderName();
	setAllowedFileExtName();

	return true;
}

void CPIDeviceMan::test(void) {
	
	CPIDeviceManTest test;
	//test.copyPreventPlusDeviceLog();
	//test.copyPreventPlus();
	test.printPreventPlus();
}

bool CPIDeviceMan::setQuarantineFilePath(void) {

	std::string quarantinePath = ConfigMan.quarantinePath;
	if( true == quarantinePath.empty() ) {
#ifdef __APPLE__
		quarantinePath = "/Users/Shared/Privacy-i/Quarantine";
#else
		quarantinePath = "/usr/local/Privacy-i/Quarantine";
#endif // #ifdef __APPLE__
	}

	util.ensurePath(quarantinePath, 0777);

	CPISecRule::List::iterator itr;
	unsigned long count = 1;
	QT_CTX  qt_ctx;
	memset( &qt_ctx, 0, sizeof(qt_ctx) );
	const size_t struct_size = sizeof(QT_CTX);
	const size_t buffer_size = sizeof(qt_ctx.Entry[0].czBuffer);
	const size_t quarantinePathSize = (quarantinePath.length()+1)*sizeof(char);

	qt_ctx.ulCount = 1;
	qt_ctx.Entry[0].ulQtType = QT_TYPE_DEFAULT;
	memcpy((void*)qt_ctx.Entry[0].czBuffer, quarantinePath.c_str(), (quarantinePathSize > buffer_size) ? buffer_size : quarantinePathSize );

	// ----------
	DEBUG_LOG("[qt_ctx] %d - %d - %s", 
			qt_ctx.ulCount,
			qt_ctx.Entry[0].ulQtType,
			qt_ctx.Entry[0].czBuffer);
	// ----------

	DEBUG_LOG("count - %d", count);
	bool result = smartDrvStub.applyPolicy(SetQuarantinePathExt, (void*) &qt_ctx, struct_size*count);
	INFO_LOG("apply drive policy - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("result:%s(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("failed - result:%s(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	}

	return true;
} 

bool CPIDeviceMan::setQuarantineLimit(void) {

	boolean_t qtLimit = FALSE;
	qtLimit = (true == ConfigMan.qtLimitUse) ? TRUE : FALSE;
	
	// ----------
	DEBUG_LOG("[qt_limit] %d", qtLimit);
	// ----------

	bool result = smartDrvStub.applyPolicy(SetQuarantineLimit, (void*) &qtLimit, sizeof(qtLimit));
	INFO_LOG("set quarantine limit - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("result:%s(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("failed - result:%s(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	}

	return true;
}

bool CPIDeviceMan::isDuplicatedDeviceLog(const CPIDeviceLog& deviceLog)
{
	bool result = false;
	size_t count = deviceLogCache.size();
	if( 0 < count )
    {
		int last_outdated_index = -1;
		for( size_t index = 0; index < deviceLogCache.size(); ++index)
        {
			CPIDeviceLog& cache = deviceLogCache[index];
			if( true == cache.isOutDated(getMaxTimeSpan()) )
            {
				last_outdated_index = (int)index;
				continue;
			}

			if( true == cache.isDuplicated(deviceLog) )
            {
				result = true;
				break;
			}
		}

		if( -1 < last_outdated_index )
        {
			if( (deviceLogCache.size()-1) == last_outdated_index )
            {
				DEBUG_LOG("clear all cache - count:%d - last_outdated_index:%d", deviceLogCache.size(), last_outdated_index );
				deviceLogCache.clear();
			}
			else
            {
				DEBUG_LOG("clear cache - count:%d - last_outdated_index:%d", deviceLogCache.size(), last_outdated_index );
				deviceLogCache.erase(deviceLogCache.begin(), deviceLogCache.begin()+(last_outdated_index+1));
				DEBUG_LOG("clear cache - left - count:%d", deviceLogCache.size());
			}
		}
	}

	if( false == result )
    {
		deviceLogCache.push_back(deviceLog);
	}

	return result;
}

int CPIDeviceMan::getMaxTimeSpan(void) const
{
	return 60; // seconds
}

bool CPIDeviceMan::MediaPolicyExistSet(CPISecRule& rule, EXIST_POLICY& Policy)
{
	if (false == rule.disableAll && "Device\\CD/DVD" != rule.virtualType) {
		return false;
	}

    if("Device\\CD/DVD" == rule.virtualType)
    {
        Policy.bExistCDPolicy = true;
    }
    else if("Device\\Camera" == rule.virtualType)
    {
        Policy.bExistCamera = true;
    }
    else if("NDIS\\WLAN" == rule.virtualType)
    {
        Policy.bExistWLan = true;
    }
    else if("NDIS\\WWAN" == rule.virtualType)
    {
        Policy.bExistWwan = true;
    }
    else if(("Device\\Bluetooth" == rule.virtualType && false == rule.blockSelectedBluetooth))
    {
        Policy.bExistBth = true;
    }
    else if(("Share\\AirDrop" == rule.virtualType))
    {
        Policy.bExistAirDrop = true;
    }
    else if(("Share\\FileSharing" == rule.virtualType))
    {
        Policy.bExistFileSharing = true;
    }
    else if(("Share\\RemoteManagement" == rule.virtualType))
    {
        Policy.bExistRemoteManagement = true;
    }
    return true;
}

bool CPIDeviceMan::MediaPolicyExistPost(EXIST_POLICY& Policy)
{
    if(false == Policy.bExistCDPolicy)
    {
#ifndef LINUX		
        g_DRDevice.ClearDRDevicePolicy();
#else
		CPISecRule rule;
		CPIDeviceController controller(rule);
		controller.deviceControl.enableWriteCDDVD();
#endif		
    }
    
    if(false == Policy.bExistWLan)
	{
#ifndef LINUX		
#else
		CPISecRule rule;
		CPIDeviceController controller(rule);
		controller.deviceControl.enableWLAN();
#endif	
	}
    if(false == Policy.bExistWwan) 
	{
#ifndef LINUX		
#else
		CPISecRule rule;
		CPIDeviceController controller(rule);
		controller.deviceControl.enableWWLAN();
#endif	
	}
    if(false == Policy.bExistCamera) {}
    
    if(false == Policy.bExistAirDrop)
    {
#ifndef LINUX		
        g_DRDevice.SetProtectSharingDaemon( false );
#endif		
    }
    
    if(false == Policy.bExistFileSharing) {}
    if(false == Policy.bExistRemoteManagement) {}

	if (false == Policy.bExistBth)
	{
		CPISecRule rule;
		CPIDeviceController controller(rule);
		controller.deviceControl.enableBluetooth();
	}
    return true;
}

bool CPIDeviceMan::controlDevice(void)
{
	//DEBUG_LOG1("device - begin");
    EXIST_POLICY Policy;
	CPISecRule::List ruleList;
	if( false == PolicyMan.getCurrentRule(ruleList) )
    {
#ifndef LINUX		
        g_DRDevice.ClearDRDevicePolicy();
        g_DRDevice.SetProtectSharingDaemon( false );
#endif		
		DEBUG_LOG1("device - no_rule");
		return false;
	}

	if(0 == ruleList.size())
    {
#ifndef LINUX		
        g_DRDevice.ClearDRDevicePolicy();
        g_DRDevice.SetProtectSharingDaemon( false );
#endif		
		DEBUG_LOG1("device - no_rule");
		return false;
	}

    memset( &Policy, 0, sizeof(Policy) );
	CPISecRule::List::iterator itr;
	for(itr = ruleList.begin(); itr != ruleList.end(); itr++)
    {
		CPISecRule& rule = *itr;
        if( ("Device\\CD/DVD" == rule.virtualType) ||
            ("Device\\Camera" == rule.virtualType) ||
            ("NDIS\\WLAN" == rule.virtualType) ||
            ("NDIS\\WWAN" == rule.virtualType) ||
            // ("Share\\AirDrop" == rule.virtualType) ||
            ("Share\\FileSharing" == rule.virtualType) ||
            ("Share\\RemoteManagement" == rule.virtualType) ||
#ifdef LINUX
			("Drive\\Removable" == rule.virtualType && true == rule.disableRead) ||
#endif
            ("Device\\Bluetooth" == rule.virtualType && false == rule.blockSelectedBluetooth) )
        {
			DEBUG_LOG("device - control - %s", rule.virtualType.c_str() );
            MediaPolicyExistSet(rule, Policy);
            
            CPIDeviceController controller(rule);
			bool resultBlock = controller.control();
			if((true == resultBlock) && (true == rule.enableLog))
            {
				CPIDeviceLog::VECTOR deviceLogList;
				controller.getDeviceLog(deviceLogList);
				if(deviceLogList.size() > 0)
                {
					CPIDeviceLog::VECTOR::iterator itrLog = deviceLogList.begin();
					for( ; itrLog != deviceLogList.end(); ++itrLog)
                    {
						CPIActionProcessDeviceLog::getInstance().addDeviceLog(*itrLog);
					}
				}
			}
		}
	}
	ruleList.clear();
    
    MediaPolicyExistPost(Policy);
    //DEBUG_LOG1("device - end");
	return true;
}

struct ndis_compare : public std::unary_function<std::string, bool> {
	explicit ndis_compare (const std::string& argName, const std::string& argNetworkService) : name(argName), networkService(argNetworkService){}
	bool operator() (const CNDISClass& ndisClass) {
		return compareLike(ndisClass, name, networkService); 
	}

	std::string name;
	std::string networkService;

	bool compareLike(const CNDISClass& ndisClass, const std::string& name, const std::string& networkService) {

		DEBUG_LOG("device - ndisClass.name(%s) - ndisClass.keyword(%s) - name(%s) - networkService(%s)", 
				ndisClass.name.c_str(), ndisClass.keyword.c_str(), name.c_str(), networkService.c_str());

		if( ndisClass.name != name ) {
			return false;
		}

		return ( std::string::npos == networkService.find(ndisClass.keyword) ) ? false : true;
	}
};

bool CPIDeviceMan::getMachedNetworkService(std::string name, std::vector<std::string>& vectorNetworkService)
{
	if( 0 == vectorNetworkService.size() )
    {
		DEBUG_LOG1("device - skip - empty - network_service");
		return false;
	}

	if( 0 == PolicyMan.ndisList.size() )
    {
		vectorNetworkService.clear();
		DEBUG_LOG1("device - skip - empty - ndis_class_list");
		return false;
	}
    
	std::vector<std::string> result;
	result.reserve(vectorNetworkService.size());
	std::vector<std::string>::iterator itr = vectorNetworkService.begin();
	for( ; itr != vectorNetworkService.end() ; ++itr)
    {
		CNDISClass::Vector::iterator itrFound = std::find_if( 
				PolicyMan.ndisList.begin(), 
				PolicyMan.ndisList.end(), 
				ndis_compare(name, *itr));
		
		if( itrFound != PolicyMan.ndisList.end() )
        {
			result.push_back(*itr);
			DEBUG_LOG("device - found - %s", itr->c_str());
		}
	}
    
    if(name == "WWAN")
    {
#ifndef LINUX		
        g_DRDevice.FetchWWAN_SCNetworkService( result );
#endif		
    }

	vectorNetworkService.clear();
	vectorNetworkService.resize(result.size());
	std::copy(result.begin(), result.end(), vectorNetworkService.begin());
	return true;
}

bool CPIDeviceMan::isSharedFolder(const unsigned int busType) const {
	return (BusTypeSFolder == busType) ? true : false;
}

unsigned int CPIDeviceMan::getDeviceCategory(const bool sharedFolder) const {
	return (true == sharedFolder) ? 0x0010 : 0x0008;
}

bool CPIDeviceMan::applyPolicyPrintPrevent(void) {

    ERROR_LOG( "applyPolicyPrintPrevent: %s", "");
    
	PPRINT_POLICY print_policy = new PRINT_POLICY;
	memset(print_policy, 0x00, sizeof(PRINT_POLICY));

	print_policy->nReserved1 = -1;
	print_policy->nPolicyType= MEDIA_PRINTPREVENT;

	PolicyMan.getLock();
	CDLPPolicy* policy = PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypePrintPreventPlus);
	if( NULL == policy ) {
		print_policy->bPolicy = FALSE;
	}
	else {
		CDLPPolicyPrint *pDLPPolicyPrint = dynamic_cast<CDLPPolicyPrint*>(policy);

		if( NULL == pDLPPolicyPrint ) {
			print_policy->bPolicy = FALSE;
		}
		else {
			print_policy->bPolicy       = TRUE;
			print_policy->bWatermark    = (true == pDLPPolicyPrint->useWatermark) ? TRUE : FALSE;
			print_policy->bDisableWrite = TRUE;
			print_policy->nReserved1    = -1;}
	}
	PolicyMan.releaseLock();

	DEBUG_LOG("print policy - policy(%d) - policy_type(0x%04x) - watermark(%d) - disable_write(%d) - reserved1(%d)"
			, print_policy->bPolicy
			, print_policy->nPolicyType
			, print_policy->bWatermark
			, print_policy->bDisableWrite
			, print_policy->nReserved1);

	bool result = smartDrvStub.applyPolicy(SetPrintPrevent, (void*) print_policy, sizeof(PRINT_POLICY));
	INFO_LOG("apply print policy - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("apply print policy - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result ) {
		ERROR_LOG("apply print policy - failed - result(%d)", smartDrvStub.result);
	}

	if( NULL != print_policy ) {
		delete print_policy;
		print_policy = NULL;
	}
	
	return true;
}


bool CPIDeviceMan::applyPolicyUploadPrevent(void) {
    PUPLOAD_POLICY upload_policy = NULL;
    unsigned long nTotalSize = 0;
    
    CDLPPolicy* policy = PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeUploadPreventPlus);

    PolicyMan.getLock();
    
    if(policy)
    {
        
        CDLPPolicyUpload *pDLPPolicyUpload = dynamic_cast<CDLPPolicyUpload*>(policy);
        if(pDLPPolicyUpload)
        {
            nTotalSize = sizeof(UPLOAD_POLICY) + pDLPPolicyUpload->m_sControlList.size();
            
            upload_policy = (PUPLOAD_POLICY)malloc(nTotalSize);
            memset(upload_policy, 0x00, nTotalSize);
            memcpy(&upload_policy->szControlList, pDLPPolicyUpload->m_sControlList.c_str(), pDLPPolicyUpload->m_sControlList.size());
            
            upload_policy->bPolicy       = TRUE;
            upload_policy->bDisableWrite = TRUE;
            upload_policy->nReserved1    = -1;
            upload_policy->lControlListSize = pDLPPolicyUpload->m_sControlList.size();
        }
    }
    
    // no policy
    if(0 == nTotalSize)
    {
        nTotalSize = sizeof(UPLOAD_POLICY);
        upload_policy = (PUPLOAD_POLICY)malloc(nTotalSize);
        memset(upload_policy, 0x00, sizeof(UPLOAD_POLICY));
        upload_policy->bPolicy = FALSE;
    }
    
    upload_policy->nReserved1 = -1;
    upload_policy->nPolicyType= MEDIA_UPLOADPREVENT;
    
    PolicyMan.releaseLock();
    
    // ----------
    DEBUG_LOG("upload policy - policy(%d) - policy_type(0x%04x) - disable_write(%d) - reserved1(%d)"
              , upload_policy->bPolicy
              , upload_policy->nPolicyType
              //, upload_policy->bWatermark
              , upload_policy->bDisableWrite
              , upload_policy->nReserved1);
    // ----------
    
    bool result = smartDrvStub.applyPolicy(SetUploadPrevent, (void*) upload_policy, nTotalSize);
    
    INFO_LOG("apply upload policy - %s", (true == result) ? "ok" : "failed");
    DEBUG_LOG("apply upload policy - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
    if( false == result ) {
        ERROR_LOG("apply upload policy - failed - result(%d)", smartDrvStub.result);
    }
    
    if( NULL != upload_policy ) {
        free(upload_policy);
        upload_policy = NULL;
    }
    
    return true;
}

bool CPIDeviceMan::setSelfProtect()
{
    bool        bResult = false;
    boolean_t  bProtect = m_bProtect;
    
    // ----------
    DEBUG_LOG("[setSelfProtect] %d", bProtect );
    // ----------
    
    bResult = smartDrvStub.applyPolicy( SetSelfProtect, (void*)&bProtect, sizeof(bProtect) );
    
    INFO_LOG("setSelfProtect - %s", (true == bResult) ? "ok" : "failed");
    DEBUG_LOG("result:%s", (true == bResult) ? "ok" : "failed" );
    if( false == bResult )
    {
        ERROR_LOG("failed - result:%s", (true == bResult) ? "ok" : "failed" );
    }
    
    return bResult;
    
}

bool CPIDeviceMan::setSelfProtectOnlyPermit()
{
    bool  bResult = false;
    bool  bProtect = false;
    
    bResult = smartDrvStub.applyPolicy( SetSelfProtectOnlyPermit, (void*)&bProtect, sizeof(bProtect) );
    
    INFO_LOG("SetSelfProtectOnlyPermit - %s", (true == bResult) ? "ok" : "failed");
    DEBUG_LOG("result:%s", (true == bResult) ? "ok" : "failed" );
    if( false == bResult )
    {
        ERROR_LOG("failed - result:%s", (true == bResult) ? "ok" : "failed" );
    }
    return bResult;
}

int CPIDeviceMan::smartdrvProcessCallback(void* param) {
	DEBUG_LOG1("device - begin");

	setSelfProtectOnlyPermit();

	DEBUG_LOG1("device - end");
	return 0;
}

bool CPIDeviceMan::applyPolicyProcess(void) {
	DEBUG_LOG1("begin");

	applicationControl.writeProcessControlXml();

	bool control = false;
	bool log = false;  
	applicationControl.getProcessControl(control, log);

	DEBUG_LOG("apply process control - control:%s - log:%s", util.getValueAsString(control).c_str(), util.getValueAsString(log).c_str());
	bool result = smartDrvStub.applyPolicyProcess(control, log, NULL);
	INFO_LOG("apply process control - %s", (true == result) ? "ok" : "failed");
	DEBUG_LOG("apply process control - %s - result(%d)", (true == result) ? "ok" : "failed", smartDrvStub.result);
	if( false == result )
    {
		ERROR_LOG("apply process control - failed - result(%d)", smartDrvStub.result);
	}
	return true;
}

int CPIDeviceMan::smartdrvProcessAccessCheck(void* param)
{
	//DEBUG_LOG1("device - begin");
	
	if( NULL == param )
    {
		DEBUG_LOG1("device - stop - invalid param");
		return 0;
	}

	//pthread_mutex_lock(&mutexProcessCheck);
	//DEBUG_LOG1("locked");

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
	std::string ticket = getTicket(pEvtInfo->ProcessId);
	//DEBUG_LOG( "[ticket:%s] device - before_check -  pid=%d, file_path=%s, process_name=%s", ticket.c_str(), pEvtInfo->ProcessId, pEvtInfo->pFilePath, pEvtInfo->pQtFilePath);

	CPIApplicationControl::CResult result;
	applicationControl.isProcessAllow(ticket, pEvtInfo->ProcessId, pEvtInfo->pFilePath, pEvtInfo->pQtFilePath, result);
	bool allow = (CPIApplicationControl::CResult::ALLOW == result.value) ? true : false;
	pEvtInfo->bAccess = (true == allow) ? 1 : 0;
	
	//DEBUG_LOG( "[ticket:%s] device - after_check - result:%s", ticket.c_str(), result.valueAsString().c_str());

	if (false == allow)
    {
		result.pid = pEvtInfo->ProcessId;
		addProcessBlockLog(result);
		DEBUG_LOG( "[ticket:%s] device - result:%s -  pid=%d, file_path=%s, process_name=%s", ticket.c_str(), result.valueAsString().c_str(), pEvtInfo->ProcessId, pEvtInfo->pFilePath, pEvtInfo->pQtFilePath);
	}
	
	//DEBUG_LOG1("unlocked");
	//pthread_mutex_unlock(&mutexProcessCheck);
	
	//DEBUG_LOG1("device - end");
	return 0;
}

bool CPIDeviceMan::addProcessBlockLog(CPIApplicationControl::CResult& application)
{

	CPIDeviceLog deviceLog;
	deviceLog.setDeviceTypeByPolicyType(MEDIA_PROCESS);
	deviceLog.policyType 	= MEDIA_PROCESS;
	deviceLog.accessType 	= accessRead;
	deviceLog.recordType 	= RECORD_TYPE_NORMAL;
	deviceLog.setLogTime(	  util.getCurrentDateTime());
	deviceLog.logType 		= LOG_VNODE;
	deviceLog.logAction 	= ACTION_NONE;
	deviceLog.disableResult = true;
	deviceLog.processId 	= application.pid;
	deviceLog.processName 	= application.processName; 
	deviceLog.fileName 		= "";
	deviceLog.guid 			= application.guid;

	CPIActionProcessDeviceLog::getInstance().addDeviceLog(deviceLog);

	return true;
}

std::string CPIDeviceMan::getTicket(int pid) {
	char* buffer;
	if( 0 > asprintf(&buffer, "%d-%d", util.getTickCount(), pid) ) {
		return "null";
	}

	std::string result = buffer;
	free(buffer);
	return result; 
}

bool CPIDeviceMan::reloadUSBSerial(void) {
	if (0 == PolicyMan.mediaList.size()) {
		return true;
	}

	return true;
}

bool CPIDeviceMan::monitorVolums(void) {
#ifndef LINUX	
	std::string command, temp;
	command = "ls /Volumes";
	temp = util.readCommandOutput(command);

	if( true == temp.empty() ) {
		return false;
	}

	if (lastVolumes != temp) {
		EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_DEVICE, "");
	}

	lastVolumes = temp;
#endif	
	return true;
}

bool CPIDeviceMan::adjustSmartLog(void*& param) {
	if( NULL == param ) {
		return false;
	}

	PEVT_PARAM pEvtInfo = reinterpret_cast<PEVT_PARAM>(param);
	PSMART_LOG_RECORD_EX pLogEx = NULL;

	pLogEx = (PSMART_LOG_RECORD_EX)pEvtInfo->pEvtCtx;
	if(!pLogEx) {
		return false;
	}

	if (MEDIA_USB == GET_STORDEVICETYPE(pLogEx->PolicyType)) {
		CDLPPolicyCopy* policyCopy = dynamic_cast<CDLPPolicyCopy*>(PolicyMan.getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeCopyPreventPlus));
		if (NULL == policyCopy) {
			return true;
		}

		if (true == policyCopy->isBlockReadAll()) {
			const int EXCEPTION_FLAG = GET_EXCEPTION_FLAG(pLogEx->PolicyType);
			const ULONG POLICYTYPE_OLD = pLogEx->PolicyType;
			pLogEx->PolicyType = MAKE_LOGEX_POLICYTYPE(EXCEPTION_FLAG, MEDIA_COPYPREVENT);
			DEBUG_LOG("policy_type - old:0x%08x - new:0x%08x", POLICYTYPE_OLD, pLogEx->PolicyType);
			return true;
		}
	}

	return true;
}


bool CPIDeviceMan::RequestProcessTerminate( PSCANNER_NOTIFICATION pNotify )
{
    bool bTerminate = false;    
    if(!pNotify)
    {
        DEBUG_LOG1("Invalid Parameters.");
        return false;
    }
    
    bTerminate = applicationControl.ReqeustProcessTerminate( (pid_t)pNotify->nPID, pNotify->czFilePath );
    return bTerminate;
}


#endif
