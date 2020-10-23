#ifndef	_DLPPOLICY_CPP
#define	_DLPPOLICY_CPP

#include <map>

#include "DLPPolicy.h"

using namespace nsPISecObject;

////////////////////////////////////////////////////////////////////////////////
//class CDeviceV6

CDeviceV6::CDeviceV6()  {
	this->code = 0;
	this->name = "";
}

CDeviceV6::CDeviceV6(unsigned short code, std::string name)  {
	this->code = code;
	this->name = name;
}

CDeviceV6::~CDeviceV6() {
}

CDeviceV6& CDeviceV6::getInstance(void) {

	static CDeviceV6 instance(0, "");

	if( 0 == instance.map.size() )
    {
		instance.map[COPYPREVENT] = CDeviceV6(MEDIA_COPYPREVENT, "CopyPrevent\\Removable");
		instance.map[CDDVD] = CDeviceV6(MEDIA_CD_DVD, "Media\\CDDVD");
		instance.map[FLOPPY] = CDeviceV6(MEDIA_FLOPPY, "Media\\Floppy");
		instance.map[USB] = CDeviceV6(MEDIA_USB, "Media\\Removable");
		instance.map[NETDRIVE] = CDeviceV6(MEDIA_NET_DRIVE, "Media\\NetDrive");
		instance.map[SERIAL] = CDeviceV6(MEDIA_SERIAL, "Media\\Serial");
		instance.map[PARALLEL] = CDeviceV6(MEDIA_PARALLEL, "Media\\Parallel");
		instance.map[IEEE1394] = CDeviceV6(MEDIA_1394, "Media\\1394");
		instance.map[MTP] = CDeviceV6(MEDIA_MTP, "Media\\MTP");
		instance.map[PTP] = CDeviceV6(MEDIA_PTP, "Media\\PTP");
		instance.map[ANDROID] = CDeviceV6(MEDIA_ANDROID, "Media\\Android");
		instance.map[APPLE] = CDeviceV6(MEDIA_APPLE, "Media\\Apple");
		instance.map[PRINTPREVENT] = CDeviceV6(MEDIA_PRINTPREVENT, "PrintPrevent\\Print");
        instance.map[BLUETOOTH] = CDeviceV6(MEDIA_BLUETOOTH, "Media\\Bluetooth");
        instance.map[PROCESS] = CDeviceV6(MEDIA_PROCESS, "Process\\Control");
        instance.map[CAMERA] = CDeviceV6(MEDIA_CAMERA, "Media\\Camera");
	}

	return instance;
}


////////////////////////////////////////////////////////////////////////////////
//class CPISecRule
CPISecRule::CPISecRule()
{
	clear();
}

CPISecRule::~CPISecRule() {
}

void CPISecRule::clear(void)
{
	CPIObject::clear();

	virtualType = "";
	disableAll = false; 
	disableRead = false;
	disableWrite = false;
	enableLog = false;
	reserved1 = -1;
    clearWLanPermitList();
    clearUSBMobilePermitList();
}

void CPISecRule::operator=(const CPISecRule& rule) {
	this->clear();	
	this->virtualType = rule.virtualType;
	this->disableAll = rule.disableAll; 
	this->disableRead = rule.disableRead;
	this->disableWrite = rule.disableWrite;
	this->enableLog = rule.enableLog;
	this->reserved1 = rule.reserved1;
    this->blockSelectedBluetooth = rule.blockSelectedBluetooth;
    setWLanPermitList( rule.m_vecWLanPermitList );
    setUSBMobilePermitList( rule.m_vecUSBMobilePermitList );
    setRemovableDrivePermitList( rule.m_vecRemovableDrivePermitList );
    this->m_nRemovableDrivePermitAllowControlType = rule.m_nRemovableDrivePermitAllowControlType;
}

bool CPISecRule::setWLanPermitList(std::vector<std::string> vecWLanPermitList)
{
    if(vecWLanPermitList.size() > 0)
    {
        int nPos=0;
        std::string strPermitSSID;
        
        clearWLanPermitList();
        for(nPos=0; nPos<vecWLanPermitList.size(); nPos++)
        {
            strPermitSSID = vecWLanPermitList[ nPos ];
            appendWLanPermitList( strPermitSSID );
        }
        return true;
    }
    return false;
}

bool CPISecRule::appendWLanPermitList( std::string strPermitSSID )
{
    m_vecWLanPermitList.push_back( strPermitSSID );
    return true;
}

void CPISecRule::clearWLanPermitList()
{
    m_vecWLanPermitList.clear();
}


void CPISecRule::clearUSBMobilePermitList()
{
    m_vecUSBMobilePermitList.clear();
}

bool CPISecRule::setUSBMobilePermitList(std::vector<MB_PERMIT> vecUSBMobilePermitList)
{
    int nPos=0;
    MB_PERMIT PermitInfo;
    
    if(vecUSBMobilePermitList.size() <= 0) return false;
    
    m_vecUSBMobilePermitList.clear();
    for(nPos=0; nPos<vecUSBMobilePermitList.size(); nPos++)
    {
        PermitInfo = vecUSBMobilePermitList[nPos];
        m_vecUSBMobilePermitList.push_back( PermitInfo );
    }
    return true;
}

void CPISecRule::clearRemovableDrivePermitList()
{
    m_vecRemovableDrivePermitList.clear();
}

bool CPISecRule::setRemovableDrivePermitList(std::vector<MB_PERMIT> vecRemovableDrivePermitList )
{
    int nPos=0;
    MB_PERMIT PermitInfo;
    
    if(vecRemovableDrivePermitList.size() <= 0) return false;
    
    m_vecRemovableDrivePermitList.clear();
    for(nPos=0; nPos<vecRemovableDrivePermitList.size(); nPos++)
    {
        PermitInfo = vecRemovableDrivePermitList[nPos];
        m_vecRemovableDrivePermitList.push_back( PermitInfo );
    }
    return true;
}

PATTACHED_DEVICE_EX CPISecRule::getData(void) {

	CDeviceV6& deviceV6 = CDeviceV6::getInstance();
	setDeviceRule();

	if( "Drive\\Removable" == virtualType ) {
		if( (false == disableRead) && (1 != reserved1) ) {
			strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::COPYPREVENT].name.c_str());
			deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::COPYPREVENT].code;
		}
		else {
			strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::USB].name.c_str());
			deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::USB].code;

			if( true == disableWrite ) {
				deviceEx.cDevice.nReserved1 = 1;
			}
		}
	}
	else if ( "Drive\\CD/DVD" == virtualType ) {
		if( true == disableWrite ) {
			deviceEx.cDevice.nReserved1 = 1;
		}
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::CDDVD].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::CDDVD].code;
	}
	else if ( "Drive\\Floppy" == virtualType ) {
		if( true == disableWrite ) {
			deviceEx.cDevice.nReserved1 = 1;
		}
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::FLOPPY].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::FLOPPY].code;
	}
	else if ( "Drive\\NetDrive" == virtualType ) {
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::NETDRIVE].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::NETDRIVE].code;

		if( (true == disableRead) || (1 == reserved1) ) {
			if( true == disableWrite ) {
				deviceEx.cDevice.nReserved1 = 1;
			}
		}
	}
	else if ( "Device\\Serial" == virtualType ) {
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::SERIAL].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::SERIAL].code;
	}
	else if ( "Device\\Parallel" == virtualType ) {
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::PARALLEL].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::PARALLEL].code;
	}
	else if ( "Device\\IEEE1394" == virtualType ) {
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::IEEE1394].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::IEEE1394].code;
	}
	else if ( "Device\\CD/DVD" == virtualType ) {
		strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::CDDVD].name.c_str());
		deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::CDDVD].code;
	}
    else if ( "Device\\Bluetooth" == virtualType ) {
        strcpy(deviceEx.cDevice.DeviceNames, deviceV6.map[CDeviceV6::BLUETOOTH].name.c_str());
        deviceEx.ulStorDevType = deviceV6.map[CDeviceV6::BLUETOOTH].code;
        if(blockSelectedBluetooth)
            deviceEx.cDevice.nReserved1 = 1;
        else
            deviceEx.cDevice.nReserved1 = -1;
    }
	else {
	}

	return &deviceEx;
}

void CPISecRule::setDeviceRule(void) {
    deviceEx.cDevice.bLoggingOn 	= (true == this->enableLog) ? TRUE : FALSE;
    deviceEx.cDevice.bDisableAll 	= (true == this->disableAll) ? TRUE : FALSE;
    deviceEx.cDevice.bDisableRead 	= (true == this->disableRead) ? TRUE : FALSE;
    deviceEx.cDevice.bDisableWrite 	= (true == this->disableWrite) ? TRUE : FALSE;
}

PATTACHED_DEVICE CPISecRule::getAttachedDevice(CPISecRule::VECTOR_ATTACHED_DEVICE& list) {
	const size_t count = list.size();
	if (0 == count) {
		return NULL;
	}

	PATTACHED_DEVICE result = new ATTACHED_DEVICE[count];
	for (size_t index = 0; index < count; ++index) {
		memcpy(&(result[index]), &(list[index]), sizeof(ATTACHED_DEVICE));
	}
	
	return result;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicy 
CDLPPolicy::CDLPPolicy() {
	clear();
}

CDLPPolicy::~CDLPPolicy() {
}

void CDLPPolicy::clear(void) {
	CPIObject::clear();

	dlpType = dlptypeNone;
	dlpSubType = dlpsubtypeNone;
	guid = "";
	name = "";
	use = true;
	datetime = "";
	revision = 0;
	dlpControlMode = controlmodeNone;
	rule.clear();
	enableDeviceRule = true;
}

bool CDLPPolicy::parsePolicyInfo(CMarkup& markup) {
	
	std::string dlpType = markup.GetAttrib("dlppolicy_type");
	this->dlpType = static_cast<EM_DLPTYPE>(atoi(dlpType.c_str()));
	std::string dlpSubType = markup.GetAttrib("dlppolicy_subtype");
	this->dlpSubType = static_cast<EM_DLPSUBTYPE>(atoi(dlpSubType.c_str()));
	std::string use = markup.GetAttrib("use");
	this->use = util.getValueAsBool(use);
	this->name = markup.GetAttrib("dlppolicy_name");
	this->guid = markup.GetAttrib("dlppolicy_guid");
	this->revision = atoi(markup.GetAttrib("dlppolicy_revision").c_str());
	
	return true;
}

void CDLPPolicy::addDeviceRule( std::string virtualType,
                                bool disableAll, bool disableRead, bool disableWrite,
		                        bool enableLog, bool disbleSmartInspect, bool blockSelectedBluetooth, int reserved1 )
{
	rule.clear();
	rule.virtualType = virtualType;
	rule.disableAll = disableAll;
	rule.disableRead = disableRead;
	rule.disableWrite = disableWrite;
	rule.enableLog = enableLog;
    rule.blockSelectedBluetooth = blockSelectedBluetooth;
	rule.reserved1 = reserved1;
    
	if( true == disbleSmartInspect )
    {
		rule.reserved1 = 1;
	}
}

bool CDLPPolicy::addDeviceRule(CMarkup& markup)
{
	markup.AddElem("rule");
	markup.AddAttrib("virtualtype", rule.virtualType); 
	markup.AddAttrib("block_all", util.getValueAsString(rule.disableAll)); 
	markup.AddAttrib("block_read", util.getValueAsString(rule.disableRead));
	markup.AddAttrib("block_write", util.getValueAsString(rule.disableWrite));
	markup.AddAttrib("enable_log", util.getValueAsString(rule.enableLog));
	markup.AddAttrib("reserved1", util.getValueAsString(rule.reserved1));

	if ((1 == rule.reserved1) && (true == rule.disableWrite)) {
		markup.AddAttrib("block_write", "all");
	}
	
	return true;
}

bool CDLPPolicy::addDeviceRule(const bool blockWriteAll) {
	return true;
}

bool CDLPPolicy::makeInformationXml(CMarkup& markup) {
	return true;
}
		
bool CDLPPolicy::isEnableBlockLog(void) {
	return false;
}

void CDLPPolicy::setEnableDeviceRule(const bool enableDeviceRule) { 
	this->enableDeviceRule = enableDeviceRule; 
	DEBUG_LOG("mode:%d - type:%d - subtype(%d) - enable:%s", dlpControlMode, dlpType, dlpSubType, (this->enableDeviceRule)?"true":"false");
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyPreventPlus 
CDLPPolicyPreventPlus::CDLPPolicyPreventPlus() {
	clear();
	initialize();
}

CDLPPolicyPreventPlus::~CDLPPolicyPreventPlus() {
}

bool CDLPPolicyPreventPlus::initialize(void) {
	dlpType = dlptypePreventPlus;
	
	return 	CDLPPolicy::initialize();
}

void CDLPPolicyPreventPlus::clear(void) {
	CDLPPolicy::clear();

	controls.clear();
	blockLog.clear();
	allowLog.clear();
	notify = notifyNone;
}

bool CDLPPolicyPreventPlus::parsePolicyInfo(CMarkup& markup) {
	CDLPPolicy::parsePolicyInfo(markup);

	markup.IntoElem();

	this->controls.clear();
	if( true == markup.FindElem("controls") ) {
		this->controls.parsePolicyInfo(markup);
	}
	
	this->allowLog.clear();
	this->blockLog.clear();
	if( true == markup.FindElem("log") ) {
		markup.IntoElem();
		if( true == markup.FindElem("block") ) {
			this->blockLog.parsePolicyInfo(markup);
		}
		markup.ResetMainPos();
		if( true == markup.FindElem("allow") ) {
			this->allowLog.parsePolicyInfo(markup);
		}
		markup.OutOfElem();
	}

	if( true == markup.FindElem("notify") ) {
		this->setNotify(markup.GetAttrib("type"));
	}
	
	markup.OutOfElem();
	
	return true;
}

void CDLPPolicyPreventPlus::setNotify(std::string notify) {

	if( "none" == notify ) {
		this->notify = notifyNone;
	}
	else if( "always" == notify ) {
		this->notify = notifyAlways;
	}
	else if( "block" == notify ) {
		this->notify = notifyBlock;
	}
	else {
		this->notify = notifyNone;
	}
}

bool CDLPPolicyPreventPlus::changeControlApprovalToBlock(void) {

	switch(controls.type) {
		case inspectionNoInspect : 
		
			if( controlApproval == controls.noInspect ) {
				controls.noInspect = controlBlock;
				if( logConditionAll & allowLog.condition) {
					allowLog.condition &= ~logConditionAll;
					blockLog.condition |= logConditionAll;
					allowLog.saveFileCopy.condition &= ~logConditionAll;
				}
			}
		
			break;
		case inspectionRunInspect : 
			
			if( controlApproval == controls.found ) {
				controls.found = controlBlock;
				if( logConditionFound & allowLog.condition) {
					allowLog.condition &= ~logConditionFound;
					blockLog.condition |= logConditionFound;
					allowLog.saveFileCopy.condition &= ~logConditionFound;
				}
			}
			
			if( controlApproval == controls.notFound ) {
				controls.notFound = controlBlock;
				if( logConditionNotFound & allowLog.condition) {
					allowLog.condition &= ~logConditionNotFound;
					blockLog.condition |= logConditionNotFound;
					allowLog.saveFileCopy.condition &= ~logConditionNotFound;
				}
			}
		
			break;
		default : break;
	}
	
	return true;
}

bool CDLPPolicyPreventPlus::addDeviceRule(const bool blockWriteAll)
{
	CDLPPolicy::addDeviceRule(virtualType, false, false, true, blockLog.use);
	return true;
}

bool CDLPPolicyPreventPlus::isEnableBlockLog(void) {
	return blockLog.use;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyCopy 
CDLPPolicyCopy::CDLPPolicyCopy()
{
	clear();
	initialize();
}

CDLPPolicyCopy::~CDLPPolicyCopy()
{
}

bool CDLPPolicyCopy::initialize(void) {
	dlpSubType = dlpsubtypeCopyPreventPlus;
	virtualType = "Drive\\Removable";
	
	return CDLPPolicyPreventPlus::initialize();
}

void CDLPPolicyCopy::clear(void) {
	CDLPPolicyPreventPlus::clear();

	warningOnFileCopy = false;
	mediaPermitType = mdPermitOwner;
	mediaGuidList.clear();
}

bool CDLPPolicyCopy::parsePolicyInfo(CMarkup& markup) {
	CDLPPolicyPreventPlus::parsePolicyInfo(markup);
	
	markup.IntoElem();
	if( true == markup.FindElem("advanced") ) {
		setWarningOnFileCopy(markup.GetAttrib("warning_onfilecopy"));
	}

	if( true == markup.FindElem("media_list") ) {
		setMediaPermitType(markup.GetAttrib("type"));
		while(true == markup.FindChildElem("media")) {
			mediaGuidList.push_back(markup.GetChildAttrib("guid"));
		}
	}
	markup.OutOfElem();
	
	return true;
}
		
void CDLPPolicyCopy::setWarningOnFileCopy(std::string warningOnFileCopy) {
	this->warningOnFileCopy = util.getValueAsBool(warningOnFileCopy);
}

void CDLPPolicyCopy::setMediaPermitType(std::string mediaPermitType) {
	this->mediaPermitType = static_cast<EM_MEDIA_PERMIT>(atoi(mediaPermitType.c_str()));
}

bool CDLPPolicyCopy::addDeviceRule(const bool blockWriteAll) {

	if( true == blockWriteAll ) {
		bool blockWrite = false;
		if( inspectionRunInspect == this->controls.type ) {
			if( 
					(controlAllow != this->controls.found ) ||
					(controlAllow != this->controls.notFound )
			  ) {
				blockWrite = true;
			}
		}
		else {
			if( controlAllow != this->controls.noInspect ) {
				blockWrite = true;
			}
		}

		CDLPPolicy::addDeviceRule(virtualType, false, false, blockWrite, blockLog.use, blockWrite);
	}
	else {
		CDLPPolicy::addDeviceRule(virtualType, false, false, true, blockLog.use);
	}
	
	return true;
}

bool CDLPPolicyCopy::isBlockReadAll(void) {
	return false;
}

bool CDLPPolicyCopy::getDLPCopyControl(const CDLPUSBDrive* usbDrive, CDLPCopyControl& copyControl) {
	copyControl.logs.block.setUse(blockLog.use);
	copyControl.logs.block.setType((blockLog.isConditional)?logConditional:logAll);
	copyControl.logs.block.setCondition(blockLog.condition);
	copyControl.logs.block.fileCopy.setUse(blockLog.saveFileCopy.use);
	copyControl.logs.block.fileCopy.setType((blockLog.saveFileCopy.isConditional)?logConditional:logAll);
	copyControl.logs.block.fileCopy.setCondition(blockLog.saveFileCopy.condition);
	copyControl.logs.block.fileCopy.setMaxUse(blockLog.saveFileCopy.maxLimitUse);
	copyControl.logs.block.fileCopy.setMaxLimit(blockLog.saveFileCopy.maxLimit);

	copyControl.logs.allow.setUse(allowLog.use);
	copyControl.logs.allow.setType((allowLog.isConditional)?logConditional:logAll);
	copyControl.logs.allow.setCondition(allowLog.condition);
	copyControl.logs.allow.fileCopy.setUse(allowLog.saveFileCopy.use);
	copyControl.logs.allow.fileCopy.setType((allowLog.saveFileCopy.isConditional)?logConditional:logAll);
	copyControl.logs.allow.fileCopy.setCondition(allowLog.saveFileCopy.condition);
	copyControl.logs.allow.fileCopy.setMaxUse(allowLog.saveFileCopy.maxLimitUse);
	copyControl.logs.allow.fileCopy.setMaxLimit(allowLog.saveFileCopy.maxLimit);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyCopyEx 
CDLPPolicyCopyEx::CDLPPolicyCopyEx()
{
	clear();
	initialize();
}

CDLPPolicyCopyEx::~CDLPPolicyCopyEx()
{
}

bool CDLPPolicyCopyEx::initialize(void) {
	dlpSubType = dlpsubtypeCopyExPreventPlus;
	virtualType = "Drive\\Removable";
	
	return CDLPPolicy::initialize();
}

void CDLPPolicyCopyEx::clear(void) {
	CDLPPolicyCopy::clear();
		
	dlpCopyControls.clear();
	notify.clear();
	advancedInfo.clear();
	media.clear();
}

bool CDLPPolicyCopyEx::parsePolicyInfo(CMarkup& markup) {
	CDLPPolicy::parsePolicyInfo(markup);

	DEBUG_LOG1("copy_ex - begin");

	if (true == markup.IntoElem()) {
		DEBUG_LOG1("copy_ex - into_elem");
		dlpCopyControls.setXml(markup);
		notify.setXml(markup);
		advancedInfo.setXml(markup);
		media.setXml(markup);
		markup.OutOfElem();
		DEBUG_LOG1("copy_ex - outof_elem");
	}
	
	DEBUG_LOG1("copy_ex - end");
	
	return true;
}

bool CDLPPolicyCopyEx::addDeviceRule(const bool blockWriteAll) {
	bool blockRead = isBlockReadAll();
	if (true == blockWriteAll) {
		bool blockWrite = dlpCopyControls.hasBlockOrApproval();
		CDLPPolicy::addDeviceRule(virtualType, false, blockRead, blockWrite, isSaveLogAny(), blockWriteAll);
	} else {
		CDLPPolicy::addDeviceRule(virtualType, false, blockRead, true, isSaveLogAny());
	}

	return true;
}

bool CDLPPolicyCopyEx::isBlockReadAll(void) {
	return dlpCopyControls.isBlockReadAll();
}

bool CDLPPolicyCopyEx::isSaveLogAny(void) {
	return dlpCopyControls.isSaveLogAny();
}

bool CDLPPolicyCopyEx::changeControlApprovalToBlock(void) {
	return dlpCopyControls.changeControlApprovalToBlock();
}
		
bool CDLPPolicyCopyEx::getDLPCopyControl(const CDLPUSBDrive* usbDrive, CDLPCopyControl& copyControl) {
	return dlpCopyControls.getDLPCopyControl(usbDrive, copyControl);
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyUpload 
CDLPPolicyUpload::CDLPPolicyUpload() {
	initialize();
}

CDLPPolicyUpload::~CDLPPolicyUpload() {
}

bool CDLPPolicyUpload::initialize(void) {
	virtualType = "Network\\Control";
	dlpSubType = dlpsubtypeUploadPreventPlus;
    
    LoadControlList(m_sControlList);
	
	return CDLPPolicyPreventPlus::initialize();
}

void CDLPPolicyUpload::LoadControlList(std::string & sControlList)
{
    CMarkup cXml;
    
    sControlList.clear();
    while(TRUE)
    {
        if (true != cXml.Load("/usr/local/Privacy-i/discover/ProgramData/config/security/pisec_upload.xml"))
            break;
        
        if (true != cXml.FindElem("pisecurity_upload"))
            break;
        
        if (true == cXml.FindChildElem("hook_upload"))
        {
            if (true != cXml.IntoElem())
                break;
            
            if (true == cXml.FindChildElem("injection_list"))
            {
                if (true != cXml.IntoElem())
                    break;
                
                while (true == cXml.FindChildElem("process"))
                {
                    if (true != cXml.IntoElem())
                        break;
                    
                    MCD_STR sProcess = cXml.GetAttrib("name");
                    //sControlList += sProcess + ":";
                    sControlList += sProcess + char(0x0d);
                    
                    MCD_STR sTargetProcess = cXml.GetAttrib("target_name");
                    sControlList += sTargetProcess + char(0x0d);
                    
                    while (true == cXml.FindChildElem("path"))
                    {
                        MCD_STR sPath = cXml.GetChildAttrib("name");
                        //sControlList += sPath + ":";
                        sControlList += sPath  + char(0x0d);
                    }
                    
                    sControlList +=  + char(0x0a);
                    
                    if (true != cXml.OutOfElem())
                        break;
                }
                
                if (true != cXml.OutOfElem())
                    break;
            }
            
            if (true != cXml.OutOfElem())
                break;
        }
        break;
    }
}
////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyPrint 
CDLPPolicyPrint::CDLPPolicyPrint() {
	clear();
	initialize();
}

CDLPPolicyPrint::~CDLPPolicyPrint() {
}

bool CDLPPolicyPrint::initialize(void) {
	dlpSubType = dlpsubtypePrintPreventPlus;
	virtualType = "Printer\\Control";


	return CDLPPolicyPreventPlus::initialize();
}

void CDLPPolicyPrint::clear(void) {
	CDLPPolicyPreventPlus::clear();

	warningOnPrint = false;
	showSerialNumber = false;
	saveCopyFoundPage = false;
	patternMaskingUse = false;
	useWatermark = false;
	watermarkFile = fileNone;
	differentWatermark = false;

}

bool CDLPPolicyPrint::parsePolicyInfo(CMarkup& markup) {
	CDLPPolicyPreventPlus::parsePolicyInfo(markup);

	DEBUG_LOG1("start");

	markup.IntoElem();
	if( true == markup.FindElem("advanced") ) {
		DEBUG_LOG1("found advanced");

		warningOnPrint = util.getValueAsBool(markup.GetAttrib("warning_onprint"));
		showSerialNumber = util.getValueAsBool(markup.GetAttrib("show_serialnumber"));
		saveCopyFoundPage = util.getValueAsBool(markup.GetAttrib("option_savecopy_foundpage"));
		patternMaskingUse = util.getValueAsBool(markup.GetAttrib("option_patternmaskinguse"));
		useWatermark = util.getValueAsBool(markup.GetAttrib("use_watermark"));
		setPrintWatermarkFileOptionAsString(markup.GetAttrib("watermark_file"));
		differentWatermark = util.getValueAsBool(markup.GetAttrib("different_watermark"));

		DEBUG_LOG("use_watermark, %s, %d", markup.GetAttrib("use_watermark").c_str(), useWatermark);
	}
	markup.OutOfElem();

	DEBUG_LOG1("end");

	return true;
}
		
void CDLPPolicyPrint::setPrintWatermarkFileOptionAsString(std::string watermarkFile) {

	this->watermarkFile = fileAll;

	std::string lower = watermarkFile;
	util.toLower(lower);

	if( "none" == lower ) {
		this->watermarkFile = fileNone;
	} else if( "all" == lower ) {
		this->watermarkFile = fileAll;
	} else if( "inspect_found" == lower ) {
		this->watermarkFile = fileInspectFound;
	} else if( "inspect_notfound" == lower ) {
		this->watermarkFile = fileInspectNotFound;
	}

	if( false == useWatermark ) {
		this->watermarkFile = fileNone;
	}
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyApplication 
CDLPPolicyApplication::CDLPPolicyApplication() {
	clear();
	initialize();
}

CDLPPolicyApplication::~CDLPPolicyApplication() {
}

bool CDLPPolicyApplication::initialize(void) {
	dlpSubType = dlpsubtypeApplicationPreventPlus;
	virtualType = "Process\\Control";

	return CDLPPolicyPreventPlus::initialize();
}

void CDLPPolicyApplication::clear(void) {
	CDLPPolicyPreventPlus::clear();
	
	execControlBlockLog = false;
	applicationList.clear();
	controlCount = 0;
	controlWindowsExecutableCount = 0;
	preventCount = 0;
	preventWindowsExecutableCount = 0;
}

void CDLPPolicyApplication::setExecControlBlockLog(std::string execControlBlockLog) {
	this->execControlBlockLog = util.getValueAsBool(execControlBlockLog);
}

bool CDLPPolicyApplication::parsePolicyInfo(CMarkup& markup) {
	CDLPPolicyPreventPlus::parsePolicyInfo(markup);

	execControlBlockLog = false;
	applicationList.clear();
	controlCount = 0;
	controlWindowsExecutableCount = 0;
	preventCount = 0;
	preventWindowsExecutableCount = 0;

	markup.IntoElem();

	if( true == markup.FindElem("control_process") ) {
		setExecControlBlockLog(markup.GetAttrib("log_block"));

		while(true == markup.FindChildElem("process") ) {
			CDLPApplication application;
			application.guid = markup.GetChildAttrib("guid");
			application.setType(markup.GetChildAttrib("type"));
			application.controlType = CDLPApplication::controltypeExecControl;
			application.display = markup.GetChildAttrib("display");
			application.setFileName(markup.GetChildAttrib("name"));
			application.binData = markup.GetChildAttrib("check_binary_string");
			applicationList.push_back(application);

			++controlCount;
			if (true == application.isWindowsExecutable()) {
				++controlWindowsExecutableCount;
			}
		}
	}

	if( true == markup.FindElem("PIFileAccessControl") ) {
		while(true == markup.FindChildElem("process") ) {
			CDLPApplication application;
			application.type = CDLPApplication::typeCustom;
			application.controlType = CDLPApplication::controltypeFileOpenControl;
			application.display = markup.GetChildAttrib("display");
			application.setFileName(markup.GetChildAttrib("name"));
			applicationList.push_back(application);
			
			++preventCount;
			if (true == application.isWindowsExecutable()) {
				++preventWindowsExecutableCount;
			}
		}
	}
	
	markup.OutOfElem();

	return true;
}

bool CDLPPolicyApplication::makeInformationXml(CMarkup& markup) {

	// execute control
	if( true == markup.AddElem("control_process") ) {
		markup.SetAttrib("log_block", util.getValueAsString(execControlBlockLog));

		for(int index = 0; index < applicationList.size(); index++) {
			CDLPApplication& application = applicationList[index];
			if( CDLPApplication::controltypeExecControl != application.controlType ) {
				continue;
			}
			markup.AddChildElem("process");
			markup.AddChildAttrib("guid", application.guid);
			markup.AddChildAttrib("type", application.type);
			markup.AddChildAttrib("display", application.display);
			markup.AddChildAttrib("name", application.fileName);
			markup.AddChildAttrib("check_binary_string", application.binData);
		}
	}

	// file access control
	if( true == markup.AddElem("PIFileAccessControl") ) {
		markup.SetAttrib("Enable", "true");

		for(int index = 0; index < applicationList.size(); index++) {
			CDLPApplication& application = applicationList[index];
			if( CDLPApplication::controltypeFileOpenControl != application.controlType ) {
				continue;
			}
			markup.AddChildElem("process");
			markup.AddChildAttrib("display", application.display);
			markup.AddChildAttrib("name", application.fileName);
			markup.AddChildAttrib("type", application.fileOpenControlType);
		}
	}
	
	return true;
}

bool CDLPPolicyApplication::addDeviceRule(const bool blockWriteAll) {
	CDLPPolicy::addDeviceRule(virtualType, true, true, true, execControlBlockLog);
	return true;
}

bool CDLPPolicyApplication::isEnableBlockLog(void) {
	bool result = false;
	if( (true == execControlBlockLog) || (true == blockLog.use )) {
		result = true;
	}
	return result;
}

bool CDLPPolicyApplication::isControlApplicationNonWindowsExecutableEmpty(void) {
	if (true == isControlApplicationEmpty()) {
		return true;
	}

	return (0 < (controlCount - controlWindowsExecutableCount))?false:true;
}

bool CDLPPolicyApplication::isControlApplicationEmpty(void) {
	return (0 == controlCount)?true:false;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyClipboard 
CDLPPolicyClipboard::CDLPPolicyClipboard() {
	clear();
	initialize();
}

CDLPPolicyClipboard::~CDLPPolicyClipboard() {
}

bool CDLPPolicyClipboard::initialize(void) {
	dlpSubType = dlpsubtypeClipboardPreventPlus;
	virtualType = "Process\\PasteControl";

	return CDLPPolicyPreventPlus::initialize();
}

void CDLPPolicyClipboard::clear(void) {
	CDLPPolicyPreventPlus::clear();

	applicationList.clear();
}

bool CDLPPolicyClipboard::parsePolicyInfo(CMarkup& markup) {
	CDLPPolicyPreventPlus::parsePolicyInfo(markup);

	markup.IntoElem();

	if( true == markup.FindElem("control_paste_by_process") ) {

		while(true == markup.FindChildElem("process") ) {
			CDLPApplication application;
			application.setType(markup.GetChildAttrib("type"));
			application.display = markup.GetChildAttrib("display");
			application.fileName = markup.GetChildAttrib("name");
			applicationList.push_back(application);
		}
	}

	markup.OutOfElem();
	
	return true;
}

bool CDLPPolicyClipboard::makeInformationXml(CMarkup& markup) {

	if( true == markup.AddElem("control_paste_by_process") ) {

		for(int index = 0; index < applicationList.size(); index++) {
			CDLPApplication& application = applicationList[index];
			markup.AddChildElem("process");
			markup.AddChildAttrib("type", application.type);
			markup.AddChildAttrib("display", application.display);
			markup.AddChildAttrib("name", application.fileName);
		}
	}
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyControl 
CDLPPolicyControl::CDLPPolicyControl() {
	initialize();
}

CDLPPolicyControl::~CDLPPolicyControl() {
}

bool CDLPPolicyControl::initialize(void) {
	dlpType = dlptypeControl;
	
	return CDLPPolicy::initialize();
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyMedia 
CDLPPolicyMedia::CDLPPolicyMedia()
{
	clear();
	initialize();
}

CDLPPolicyMedia::~CDLPPolicyMedia()
{
    clrUSBMobilePermitList();
}

bool CDLPPolicyMedia::initialize(void) {
	dlpSubType = dlpsubtypeMediaControl;
    m_bBlockSelectedBluetooth = false;
	return CDLPPolicyControl::initialize();
}

void CDLPPolicyMedia::clear(void) {
	CDLPPolicyControl::clear();

	controlEtcStorage = 0;
	controlCommDevice = 0;
	blockLog = false;
    m_bBlockSelectedBluetooth = false;
	ruleList.clear();
    
    m_vecWlanPermitList.clear();
    clrUSBMobilePermitList();
}

bool CDLPPolicyMedia::parsePolicyInfo(CMarkup& markup)
{
	CDLPPolicyControl::parsePolicyInfo(markup);

	markup.IntoElem();

	if(true == markup.FindElem("control"))
    {
		controlEtcStorage = atoi(markup.GetAttrib("etcstorage_control_type").c_str());
		controlCommDevice = atoi(markup.GetAttrib("commdevice_control_type").c_str());
		setBlockLog(markup.GetAttrib("log_block"));
        
        if(true == markup.IntoElem())
        {
            markup.ResetMainPos();
            if(true == markup.FindElem("bluetooth"))
            {
                if(0 != (controlCommDevice & commdeviceBlockBluetooth))
                    m_bBlockSelectedBluetooth = ("true" == markup.GetAttrib("block_selected_bluetooth") ? true : false);
            }
            markup.OutOfElem();
        }

        if(controlCommDevice & commdeviceBlockWLAN)
        {
            if(true == markup.IntoElem())
            {
                markup.ResetMainPos();
                if(true == markup.FindElem("wlan_permit_list"))
                {
                    std::string strPermitSSID;
                    m_vecWlanPermitList.clear();
                    
                    markup.IntoElem();
                    while(true == markup.FindElem( "wlan" ))
                    {
                        strPermitSSID = markup.GetAttrib( "ssid" );
                        m_vecWlanPermitList.push_back( strPermitSSID );
                    }
                    markup.OutOfElem();
                }
                markup.OutOfElem();
            }
        }
        
        if(controlCommDevice & commdeviceBlockUSBMobile)
        {
            if(true == markup.IntoElem())
            {
                markup.ResetMainPos();
                if(true == markup.FindElem( "usbmobile_permit_list" ))
                {
                    std::string strMediaID;
                    std::string strMediaName;
                    if(true == markup.FindChildElem( "usbmobile" ))
                    {
                        clrUSBMobilePermitList();
                    }
                    
                    markup.ResetChildPos();
                    while(true == markup.FindChildElem( "usbmobile" ))
                    {
                        strMediaName = markup.GetChildAttrib( "medianame" );
                        strMediaID = markup.GetChildAttrib( "mediaid" );
                        
                        MB_PERMIT PermitInfo;
                        memset( &PermitInfo, 0, sizeof(PermitInfo) );
                        strcpy( PermitInfo.czMediaName, strMediaName.c_str() );
                        strcpy( PermitInfo.czMediaID, strMediaID.c_str() );
                        m_vecUSBMobilePermitList.push_back( PermitInfo );
                    }
                }
                markup.OutOfElem();
            }
        }
        
        int nBlock = (etcstorageBlockRemovableRead | etcstorageBlockRemovableWrite);
        if(controlEtcStorage & nBlock)
        {
            if(true == markup.IntoElem())
            {
                markup.ResetMainPos();
                if(true == markup.FindElem( "removabledrive_permit_list" ))
                {
                    std::string strMediaID;
                    std::string strMediaName;
                    if(true == markup.FindChildElem( "removabledrive" ))
                    {
                        clrRemovableDrivePermitList();
                    }
                    
                    markup.ResetChildPos();
                    while(true == markup.FindChildElem( "removabledrive" ))
                    {
                        strMediaName = markup.GetChildAttrib( "medianame" );
                        strMediaID = markup.GetChildAttrib( "mediaid" );
                        
                        MB_PERMIT PermitInfo;
                        memset( &PermitInfo, 0, sizeof(PermitInfo) );
                        strcpy( PermitInfo.czMediaName, strMediaName.c_str() );
                        strcpy( PermitInfo.czMediaID, strMediaID.c_str() );
                        m_vecRemovableDrivePermitList.push_back( PermitInfo );
                    }
                }
                markup.OutOfElem();
            }
            
            if(true == markup.IntoElem())
            {
                markup.ResetMainPos();
                if(true == markup.FindElem( "removabledrive" ))
                {
                    m_nRemovableDrivePermitAllowControlType = atoi(markup.GetAttrib("permit_allow_control_type").c_str());
                }
                markup.OutOfElem();
            }
        }
	}
	
	markup.OutOfElem();

	return true;
}

void CDLPPolicyMedia::setBlockLog(std::string blockLog) {
	this->blockLog = util.getValueAsBool(blockLog);
}


bool CDLPPolicyMedia::setWLanPermitList( std::vector<std::string> vecWLanPermitList )
{
    if(vecWLanPermitList.size() > 0)
    {
        int nPos=0;
        std::string strPermitSSID;
        
        rule.clearWLanPermitList();
        for(nPos=0; nPos<vecWLanPermitList.size(); nPos++)
        {
            strPermitSSID = vecWLanPermitList[ nPos ];
            rule.appendWLanPermitList( strPermitSSID );
        }
        return true;
    }
    
    return false;
}


bool CDLPPolicyMedia::setUSBMobilePermitList( std::vector<MB_PERMIT> vecUSBMobilePermitList )
{
    return rule.setUSBMobilePermitList( vecUSBMobilePermitList );
}

void CDLPPolicyMedia::clrUSBMobilePermitList()
{
    m_vecUSBMobilePermitList.clear();
}


bool CDLPPolicyMedia::setRemovableDrivePermitList( std::vector<MB_PERMIT> vecRemovableDrivePermitList )
{
    return rule.setRemovableDrivePermitList( vecRemovableDrivePermitList );
}

void CDLPPolicyMedia::clrRemovableDrivePermitList()
{
    m_vecRemovableDrivePermitList.clear();
}

void CDLPPolicyMedia::addDeviceRule(const int block, std::string virtualType)
{
    bool blockAll, blockRead, blockWrite;
    blockAll = blockRead = blockWrite = (block)?true:false;
    CDLPPolicy::addDeviceRule(virtualType, blockAll, blockRead, blockWrite, blockLog);
    ruleList.push_back(rule);
}

bool CDLPPolicyMedia::addDeviceRule(const bool blockWriteAll)
{
	ruleList.clear();
	bool blockAll, blockRead, blockWrite;

	//----------
	// etc storage
	//----------
	{
		// Network Drive
		addDeviceRule((etcstorageBlockNetDrive & controlEtcStorage), "Drive\\NetDrive");

		// CD/DVD
		{ 
			blockAll = false;
			blockRead = (etcstorageBlockCDDVDRead & controlEtcStorage) ? true : false;
			bool blockWrite = (etcstorageBlockCDDVDWrite & controlEtcStorage) ? true : false;

			CDLPPolicy::addDeviceRule("Drive\\CD/DVD", blockAll, blockRead, blockWrite, blockLog);
			ruleList.push_back(rule);

			if( true == blockWrite)
			{
				CDLPPolicy::addDeviceRule("Device\\CD/DVD", false, false, true, blockLog);
				ruleList.push_back(rule);
			}
		}

        // USB
        {
            blockAll = false;
            blockRead = (etcstorageBlockRemovableRead & controlEtcStorage) ? true : false;
            blockWrite = ( etcstorageBlockRemovableWrite & controlEtcStorage) ? true : false;
            CDLPPolicy::addDeviceRule("Drive\\Removable", blockAll, blockRead, blockWrite, blockLog, blockWrite, false, 1);
            setRemovableDrivePermitList(m_vecRemovableDrivePermitList);
            rule.m_nRemovableDrivePermitAllowControlType = m_nRemovableDrivePermitAllowControlType;
            ruleList.push_back(rule);
        }

        // floppy drive
        {
            blockAll = false;
            blockRead = (etcstorageBlockFloppyRead & controlEtcStorage) ? true : false;
            blockWrite = (etcstorageBlockFloppyWrite & controlEtcStorage) ? true : false;
            
            CDLPPolicy::addDeviceRule("Drive\\Floppy", blockAll, blockRead, blockWrite, blockLog);
            ruleList.push_back(rule);
        }
    }

	//----------
	// comm device
	//----------

	{
		// WLAN
		{
			addDeviceRule((commdeviceBlockWLAN & controlCommDevice), "NDIS\\WLAN");
			setWLanPermitList( m_vecWlanPermitList );
		}

		// WWAN
		addDeviceRule((commdeviceBlockWWAN & controlCommDevice), "NDIS\\WWAN");
	
		// Bluetooth
		if( commdeviceBlockBluetooth & controlCommDevice)
        {
			blockAll = blockRead = blockWrite = (commdeviceBlockBluetooth & controlCommDevice) ? true : false;
			CDLPPolicy::addDeviceRule("Device\\Bluetooth", blockAll, blockRead, blockWrite, blockLog, m_bBlockSelectedBluetooth, m_bBlockSelectedBluetooth);     
			ruleList.push_back(rule);
		}

		// Infrared
		addDeviceRule((commdeviceBlockIRDA & controlCommDevice), "Device\\Infrared");
	
		// Serial
		addDeviceRule((commdeviceBlockSerial & controlCommDevice), "Device\\Serial");
	
		// Parallel 
		{
			addDeviceRule((commdeviceBlockParallel & controlCommDevice), "Device\\Parallel");
			addDeviceRule((commdeviceBlockParallel & controlCommDevice), "Device\\Printer");
		}

		// IEEE1394
		addDeviceRule((commdeviceBlockIEEE1394 & controlCommDevice), "Device\\IEEE1394");

		// USBMobile
		{
			addDeviceRule((commdeviceBlockUSBMobile& controlCommDevice), "Device\\USBMobile");
			setUSBMobilePermitList( m_vecUSBMobilePermitList );
		}

		// Camera
		addDeviceRule((commdeviceBlockCamera & controlCommDevice), "Device\\Camera");

		// FileSharing
		addDeviceRule((commdeviceBlockFileSharing & controlCommDevice), "Share\\FileSharing");

		// RemoteManagement
		addDeviceRule((commdeviceBlockRemoteManagement & controlCommDevice), "Share\\RemoteManagement");

		// AirDrop
		addDeviceRule((commdeviceBlockAirDrop & controlCommDevice), "Share\\AirDrop");
	}

	return true;
}

bool CDLPPolicyMedia::addDeviceRule(CMarkup& markup)
{
	if( 0 == ruleList.size() )
    {
		return true;
	}
		
	CPISecRule::List::iterator itr = ruleList.begin();
	while(itr != ruleList.end())
    {
		rule = *itr++;
		CDLPPolicy::addDeviceRule(markup);
        
        if(rule.virtualType == "Device\\Bluetooth" && rule.disableAll == TRUE)
        {
            if(m_bBlockSelectedBluetooth)
            {
                if(true == markup.IntoElem())
                {
                    markup.AddElem( "bluetooth" );
                    markup.AddAttrib( "block_selected_bluetooth", "true" );
                    markup.OutOfElem();
                }
            }
        }

        if((rule.virtualType == "NDIS\\WLAN") && (controlCommDevice & commdeviceBlockWLAN))
        {
            std::vector<std::string>::iterator ItPos;
            if(m_vecWlanPermitList.size() > 0)
            {
                if(true == markup.IntoElem())
                {
                    std::string strSSID;
                    markup.AddElem( "wlan_permit_list" );
                    for(ItPos = m_vecWlanPermitList.begin(); ItPos != m_vecWlanPermitList.end(); ItPos++)
                    {
                        strSSID = (*ItPos);
                        markup.AddChildElem( "wlan" );
                        markup.SetChildAttrib( "ssid", strSSID );
                    }
                    markup.OutOfElem();
                }
            }
        }
        
        if((rule.virtualType == "Device\\USBMobile") && (controlCommDevice & commdeviceBlockUSBMobile))
        {
            std::vector<MB_PERMIT>::iterator ItPos;
            if(m_vecUSBMobilePermitList.size() > 0)
            {
                if(true == markup.IntoElem())
                {
                    MB_PERMIT PermitInfo;
                    markup.AddElem( "usbmobile_permit_list" );
                    for(ItPos = m_vecUSBMobilePermitList.begin(); ItPos != m_vecUSBMobilePermitList.end(); ItPos++)
                    {
                        PermitInfo = (*ItPos);
                        markup.AddChildElem( "usbmobile" );
                        markup.SetChildAttrib( "medianame", PermitInfo.czMediaName );
                        markup.SetChildAttrib( "mediaid", PermitInfo.czMediaID );
                    }
                    markup.OutOfElem();
                }
            }
        }
        
        int nBlock = (etcstorageBlockRemovableRead | etcstorageBlockRemovableWrite);
        if((rule.virtualType == "Drive\\Removable") && (controlEtcStorage & nBlock))
        {
            std::vector<MB_PERMIT>::iterator ItPos;
            if(m_vecRemovableDrivePermitList.size() > 0)
            {
                if(true == markup.IntoElem())
                {
                    MB_PERMIT PermitInfo;
                    markup.AddElem( "removabledrive_permit_list" );
                    for(ItPos = m_vecRemovableDrivePermitList.begin(); ItPos != m_vecRemovableDrivePermitList.end(); ItPos++)
                    {
                        PermitInfo = (*ItPos);
                        markup.AddChildElem( "removabledrive" );
                        markup.SetChildAttrib( "medianame", PermitInfo.czMediaName );
                        markup.SetChildAttrib( "mediaid", PermitInfo.czMediaID );
                    }
                    markup.OutOfElem();
                }
                
                if(true == markup.IntoElem())
                {
                    markup.AddElem( "removabledrive" );
                    markup.SetAttrib( "permit_allow_control_type", m_nRemovableDrivePermitAllowControlType );
                    markup.OutOfElem();
                }
            }
        }
	}
	return true;
}

bool CDLPPolicyMedia::isEnableBlockLog(void) {
	return blockLog;
}

bool CDLPPolicyMedia::validateDeviceRule(const bool copy, const bool sharedFolder) {

	if( 0 == ruleList.size() ) {
		return true;
	}

	CPISecRule::List::iterator itr = ruleList.begin();
	while(itr != ruleList.end()) {
		if( (true == copy ) && ("Drive\\Removable" == itr->virtualType ) ) {
			itr = ruleList.erase(itr);
			DEBUG_LOG("policy_media(%d) - remove - usb_read_blcok", dlpControlMode);
		} else if( (true == sharedFolder) && ("Drive\\NetDrive" == itr->virtualType ) ) {
			itr = ruleList.erase(itr);
			DEBUG_LOG("policy_media(%d) - remove - netdrive_read_blcok", dlpControlMode);
		} else {
			++itr;
		}
	}

	return true;
}

bool CDLPPolicyMedia::getControl(const EM_DEVICETYPE deviceType, const int control) const {
	bool result = false;
	switch(deviceType) {
		case deviceEtcStorage : result = (controlEtcStorage & control)?true:false; break;
		case deviceCommDevice : result = (controlCommDevice & control)?true:false; break;
		default : break;
	}

	return result;
}


////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyScreenCapture 
CDLPPolicyScreenCapture::CDLPPolicyScreenCapture() {
	initialize();
}

CDLPPolicyScreenCapture::~CDLPPolicyScreenCapture() {
}

bool CDLPPolicyScreenCapture::initialize(void) {
	dlpSubType = dlpsubtypeScreenCaptureControl;
	
	return CDLPPolicyControl::initialize();
}

////////////////////////////////////////////////////////////////////////////////

//class CDLPPolicyPCSecurity
CDLPPolicyPCSecurity::CDLPPolicyPCSecurity()
{
	initialize();
}

CDLPPolicyPCSecurity::~CDLPPolicyPCSecurity()
{
}

bool CDLPPolicyPCSecurity::initialize(void)
{
	dlpSubType = dlpsubtypePCSecurity;
	return CDLPPolicyControl::initialize();
}

void CDLPPolicyPCSecurity::clear(void)
{
}

void CDLPPolicyPCSecurity::Copy( const CDLPPolicy* pDLPPolicy )
{
    CDLPPolicyPCSecurity* pSecurity = NULL;
    
    pSecurity = (CDLPPolicyPCSecurity*)(pDLPPolicy);
    if(!pSecurity)
    {
        return;
    }
    
    m_ShareFolder.setBlockAll( pSecurity->m_ShareFolder.getBlockAll());
    m_ShareFolder.setBlockEveryOne( pSecurity->m_ShareFolder.getBlockEveryOne());
    m_ShareFolder.setBlockDefault( pSecurity->m_ShareFolder.getBlockDefault());
    m_ShareFolder.setCheckInterval( pSecurity->m_ShareFolder.getCheckInterval() );
}


bool CDLPPolicyPCSecurity::makePolicyInfo(CMarkup& cXml)
{
    cXml.IntoElem();
    cXml.AddElem( "shared_folder" );
    cXml.AddAttrib( "block", util.getValueAsString(m_ShareFolder.getBlockAll()) );
    cXml.AddAttrib( "block_everyone", util.getValueAsString(m_ShareFolder.getBlockEveryOne()) );
    cXml.AddAttrib( "block_default_drive", util.getValueAsString(m_ShareFolder.getBlockDefault()) );
    cXml.AddAttrib( "check_interval", util.getValueAsString(m_ShareFolder.getCheckInterval()) );
    cXml.OutOfElem();
    return true;
}


bool CDLPPolicyPCSecurity::parsePolicyInfo(CMarkup& cXml)
{
    std::string strValue;
    
    CDLPPolicyControl::parsePolicyInfo(cXml);
    
    cXml.IntoElem();
    if(true == cXml.FindElem( "shared_folder" ))
    {
        strValue = cXml.GetAttrib( "block" );
        if(!strValue.empty()) m_ShareFolder.setBlockAll( util.getValueAsBool(strValue) );
    
        strValue = cXml.GetAttrib( "block_default_drive" );
        if(!strValue.empty()) m_ShareFolder.setBlockDefault( util.getValueAsBool(strValue) );
        
        strValue = cXml.GetAttrib( "block_everyone" );
        if(!strValue.empty()) m_ShareFolder.setBlockEveryOne( util.getValueAsBool(strValue) );
        
        strValue = cXml.GetAttrib( "check_interval" );
        if(!strValue.empty()) m_ShareFolder.setCheckInterval( atoi(strValue.c_str()) );
    }
    cXml.OutOfElem();
    return true;
}


bool CDLPPolicyPCSecurity::AddDeviceRule(const bool bBlockWriteAll/*=false*/)
{
    return true;
}



////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicySharedFolderCopy 
CDLPPolicySharedFolderCopy::CDLPPolicySharedFolderCopy() {
	initialize();
}

CDLPPolicySharedFolderCopy::~CDLPPolicySharedFolderCopy() {
}

bool CDLPPolicySharedFolderCopy::initialize(void) {
	dlpSubType = dlpsubtypeSharedFolderPreventPlus;	
	virtualType = "Drive\\NetDrive";
	
	return CDLPPolicyPreventPlus::initialize();
}

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusControls
CPreventPlusControls::CPreventPlusControls() {
	clear();
}

CPreventPlusControls::~CPreventPlusControls() {
}

void CPreventPlusControls::clear(void) {
	CPIObject::clear();
	
	type 		= inspectionNoInspect;
	targetFile 	= targetAll;
	noInspect 	= controlAllow;	
	found 		= controlAllow;	
	notFound 	= controlAllow;	
}

void CPreventPlusControls::setType(std::string type) {
	if( "inspect" == type ) {
		this->type = inspectionRunInspect;
	}
	else {
		this->type = inspectionNoInspect;
	}
}

void CPreventPlusControls::setTargetFile(std::string targetFile) {
	if( "all" == targetFile) {
		this->targetFile = targetAll;
	}
	else {
		this->targetFile = targetPatternFound;
	}
}

void CPreventPlusControls::setControl(std::string type, EM_CONTROL & control) {
	if( "allow" == type ) {
		control = controlAllow;
	}
	else if( "block" == type ) { 
		control = controlBlock;
	}
	else if( "approval" == type ) {
		control = controlApproval;
	}
	else {
		control = controlAllow;
	}
}

void CPreventPlusControls::setControl(std::string type, std::string inspect) {
	if( "none" == inspect ) {
		setControl(type, noInspect);
	}
	else if( "found" == inspect ) {
		setControl(type, found);
	}
	else if( "not_found" == inspect ) {
		setControl(type, notFound);
	}
	else {
		//error
	}
}

bool CPreventPlusControls::parsePolicyInfo(CMarkup& markup) {
	this->setType(markup.GetAttrib("type"));
	this->setTargetFile(markup.GetAttrib("target_file"));
	while(true == markup.FindChildElem("control") ) {
		this->setControl( 
				markup.GetChildAttrib("type"),
				markup.GetChildAttrib("inspect")
				);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusLogParent
CPreventPlusLogParent::CPreventPlusLogParent() {
	clear();
}

CPreventPlusLogParent::~CPreventPlusLogParent() {
}

void CPreventPlusLogParent::clear(void) {
	CPIObject::clear();
	
	use = false;
	isConditional = false;
	condition = logConditionNone;
}

void CPreventPlusLogParent::setType(std::string type) {
	if( "all" == type) {
		this->isConditional = false;
	}
	else if ( "conditional" == type) {
		this->isConditional = true;
	}
	else {
		this->isConditional = false;
	}
}

void CPreventPlusLogParent::setUse(std::string use) {
	this->use = util.getValueAsBool(use);
}

void CPreventPlusLogParent::addCondition(std::string type) {
	if( "inspect_notfound" == type) {
		condition |= logConditionNotFound;
	}
	else if( "inspect_found" == type) {
		condition |= logConditionFound;
	}
	else if( "approval_allow" == type) {
		condition |= logConditionApproval;
	}
	else {
		condition = logConditionNone;
	}
}

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusLog
CPreventPlusLog::CPreventPlusLog() {
	clear();
}

CPreventPlusLog::~CPreventPlusLog() {
}

void CPreventPlusLog::clear(void) {
	CPreventPlusLogParent::clear();

	saveFileCopy.clear();
}

bool CPreventPlusLog::parsePolicyInfo(CMarkup& markup) {
	setType(markup.GetAttrib("type"));
	setUse(markup.GetAttrib("use"));
	while(true == markup.FindChildElem("condition")) {
		addCondition(markup.GetChildAttrib("type"));
	}

	markup.IntoElem();
	while(true == markup.FindElem("save_filecopy")) {
		saveFileCopy.parsePolicyInfo(markup);
	}
	markup.OutOfElem();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusSaveFileCopy
CPreventPlusSaveFileCopy::CPreventPlusSaveFileCopy() {
	clear();
}

CPreventPlusSaveFileCopy::~CPreventPlusSaveFileCopy() {
}

void CPreventPlusSaveFileCopy::clear(void) {
	CPreventPlusLogParent::clear();

	maxLimit = 0;
	maxLimitUse = false;
}

void CPreventPlusSaveFileCopy::setMaxLimitUse(std::string use) {
	this->maxLimitUse = util.getValueAsBool(use);
}

bool CPreventPlusSaveFileCopy::parsePolicyInfo(CMarkup& markup) {
	setType(markup.GetAttrib("type"));
	setUse(markup.GetAttrib("use"));
	maxLimit = atoi(markup.GetAttrib("max_limit").c_str());
	setMaxLimitUse(markup.GetAttrib("max_limit_use"));
	while(true == markup.FindChildElem("condition")) {
		addCondition(markup.GetChildAttrib("type"));
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CPIControlItem
CPIControlItem::CPIControlItem() {
	clear();
}

CPIControlItem::~CPIControlItem() {
}

void CPIControlItem::clear(void) {
	CPIObject::clear();
	
	keyword = "";
	name = "";
}

////////////////////////////////////////////////////////////////////////////////
//class CNDISClass
CNDISClass::CNDISClass() {
}

CNDISClass::~CNDISClass() {
}

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobileClass
CUSBMobileClass::CUSBMobileClass() {
}

CUSBMobileClass::~CUSBMobileClass() {
}

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobile
CUSBMobile::CUSBMobile()
{
}

CUSBMobile::~CUSBMobile()
{
}

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobilePermit
CUSBMobilePermit::CUSBMobilePermit()
{    
}

CUSBMobilePermit::~CUSBMobilePermit()
{
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPControl
//const int DEFAULT_PURPOSE = 0;
CDLPControl::CDLPControl() {
	inspectOption = inspectionNoInspect;
	targetFile = targetAll;
	noInspect = controlAllow;
	found = controlAllow;
	notFound = controlAllow;
	purpose = 0;
}

CDLPControl::~CDLPControl() {
}

bool CDLPControl::setXml(CMarkup& markup) { 
	inspectOption = getInspectOption(markup.GetAttrib("type"));
	targetFile = getTargetFile(markup.GetAttrib("target_file"));
	purpose = atoi(markup.GetAttrib("purpose").c_str());

	while (true == markup.FindChildElem("control")) {
		std::string inspect = markup.GetChildAttrib("inspect");
		std::string type = markup.GetChildAttrib("type");
		setControlType(inspect, type);
	}	

	return true;
}	

void CDLPControl::setControlType(const std::string& inspect, const std::string& type) {
	EM_CONTROL control = controlAllow;

	if ("allow" == type) {
		control = controlAllow;
	} else if ("block" == type) {
		control = controlBlock;
	} else if ("approval" == type) {
		control = controlBlock;
	}

	if ("allow" == inspect) {
		noInspect = control;
	} else if ("block" == inspect) {
		found = control;
	} else if ("approval" == inspect) {
		notFound = control;
	}
}

int CDLPControl::getTargetFile(const std::string& value) const {
	int result = targetAll;

	if ("all" == value) {
		result = targetAll;
	} else if ("found" == value) {
		result = targetPatternFound;
	}

	return result;
}

int CDLPControl::getInspectOption(const std::string& value) const {
	int result = inspectionNoInspect;

	if ("none" == value) {
		result = inspectionNoInspect;
	} else if ("inspect" == value) {
		result = inspectionRunInspect;
	}

	return result;
}

bool CDLPControl::hasBlockOrApproval() const {
	bool result = false;

	switch(inspectOption) {
		case inspectionNoInspect :
			{
				if (controlAllow != noInspect) {
					result = true;
				}
			}
			break;
		case inspectionRunInspect :
			{
				if (controlAllow != found) {
					result = true;
				}

				if (controlAllow != notFound) {
					result = true;
				}
			}
			break;
		default: break;
	}

	return result;
}

bool CDLPControl::changeControlApprovalToBlock(void) {
	switch(inspectOption) {
		case inspectionNoInspect :
			{
				if (controlApproval == noInspect) {
					noInspect = controlBlock;
				}
			}
			break;
		case inspectionRunInspect :
			{
				if (controlApproval == found) {
					found= controlBlock;
				}
				
				if (controlApproval == found) {
					found= controlBlock;
				}
			}
			break;
		default : break;
	}

	return true;
}

bool CDLPControl::getDLPControlCount(CDLPControlCount& found, CDLPControlCount& notFound, CDLPControlCount& noInspect) {
	switch(inspectOption) {
		case inspectionNoInspect :
			{
				getDLPControlCount(this->noInspect, noInspect);
			}
			break;
		case inspectionRunInspect :
			{
				getDLPControlCount(this->found, found);
				getDLPControlCount(this->notFound, notFound);
			}
			break;
		default : break;
	}

	return true;
}

bool CDLPControl::getDLPControlCount(const int controlType, CDLPControlCount& controlCount) {
	switch(controlType) {
		case controlBlock : ++controlCount.block; break;
		case controlAllow : ++controlCount.allow; break;
		case controlApproval : ++controlCount.approval; break;
		default : break;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPUSBDriveControl
CDLPUSBDriveControl::CDLPUSBDriveControl() {
	selectType = ALL;
	readPermit = true;
}

CDLPUSBDriveControl::~CDLPUSBDriveControl() {
}

bool CDLPUSBDriveControl::setXml(CMarkup& markup) { 
	selectType  = atoi(markup.GetAttrib("select_type").c_str());
	readPermit = util.getValueAsBool(markup.GetAttrib("read_permit"));

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPLobBase
CDLPLogBase::CDLPLogBase() {
	use = false;
	type = logNone;
	condition = 0;
}

CDLPLogBase::~CDLPLogBase() {
}

int CDLPLogBase::getType(const std::string& value) {
	int result = logNone;
	if ("all" == value) {
		result = logAll;
	} else if ("conditional" == value) {
		result = logConditional;
	}

	return result;
}

int CDLPLogBase::getCondition(const std::string& value) {
	int result = logConditionNotFound;
	if ("inspect_notfound" == value) {
		result = logConditionNotFound;
	} else if ("inspect_found" == value) {
		result = logConditionFound;
	} else if ("approval_allow" == value) {
		result = logConditionApproval;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPFileCopy
CDLPFileCopy::CDLPFileCopy() {
	maxUse = false;
	maxLimit = 0;
}

CDLPFileCopy::~CDLPFileCopy() {
}

bool CDLPFileCopy::setXml(CMarkup& markup) { 
	if (false == markup.FindElem("save_filecopy")) {
		return false;
	}

	use = util.getValueAsBool(markup.GetAttrib("use"));
	type = getType(markup.GetAttrib("type"));
	maxUse = util.getValueAsBool(markup.GetAttrib("max_limit_use"));
	maxLimit = atoi(markup.GetAttrib("max_limit").c_str());

	if (logConditional == type) {
		condition = 0;
		while (true == markup.FindChildElem("condition")) {
			condition |= getCondition(markup.GetChildAttrib("type"));
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPLog
CDLPLog::CDLPLog() {
}

CDLPLog::~CDLPLog() {
}

bool CDLPLog::setXml(CMarkup& markup, const std::string& elem) {
	if (true == markup.FindElem(elem)) {
		use = util.getValueAsBool(markup.GetAttrib("use"));
		type = getType(markup.GetAttrib("type"));
	}

	if (logConditional == type) {
		condition = 0;
		while (true == markup.FindChildElem("condition")) {
			condition = getCondition(markup.GetChildAttrib("type"));
		}
	}

	if (true == markup.IntoElem()) {
		fileCopy.setXml(markup);
		markup.OutOfElem();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPLogs
CDLPLogs::CDLPLogs() {
}

CDLPLogs::~CDLPLogs() {
}

bool CDLPLogs::setXml(CMarkup& markup) { 
	if (false == markup.FindElem("log")) {
		return false;
	}

	if (true == markup.IntoElem()) {
		block.setXml(markup, "block"); 
		allow.setXml(markup, "allow"); 
		markup.OutOfElem();
	}

	return true;
}


bool CDLPLogs::hasLogEnable(void) const {
	if (
			(true == allow.isUse()) ||
			(true == block.isUse())
	   ) {
		return true;
	}

	return false;
}

bool CDLPLogs::validate(const int inspectOption,
		const CDLPControlCount& foundBefore,
		const CDLPControlCount& notFoundBefore,
		const CDLPControlCount& noInspectBefore,
		const CDLPControlCount& foundAfter,
		const CDLPControlCount& notFoundAfter,
		const CDLPControlCount& noInspectAfter) {
	switch(inspectOption) {
		case inspectionNoInspect : 
			{
				validateWithNoInspect(noInspectBefore, noInspectAfter);

			}
			break;
		case inspectionRunInspect : 
			{
				validateWithFound(foundBefore, foundAfter);
				validateWithNotFound(notFoundBefore, notFoundAfter);
			}
			break;
		default : break;
	}

	return true;
}

bool CDLPLogs::validateWithNoInspect(const CDLPControlCount& before, const CDLPControlCount& after) {
	if (logNone == allow.getType()) {
		return true;
	}

	if ((0 < before.getApproval()) && (0 == after.getApproval())) {
		if (logNone == block.getType()) {
			block.setType(logAll);
			block.setUse(true);
		}

		if (0 == after.getAllow()) {
			allow.setType(logNone);
			allow.setUse(false);
			allow.fileCopy.setType(logNone);
			allow.fileCopy.setUse(false);
		}
	}

	return true;	
}

bool CDLPLogs::validateWithFound(const CDLPControlCount& before, const CDLPControlCount& after) {
	do {
		if (logConditional != allow.getType()) {
			break;
		}

		if ((0 >= before.getApproval()) || (0 != after.getApproval())) {
			break;
		}

		if (0 == (logConditionFound & allow.getCondition())) {
			break;
		}

		switch(block.getType()) {
			case logConditional : 
				{
					if (0 == (logConditionFound & block.getCondition())) {
						block.addCondition(logConditionFound);
					}
				}
				break;
			case logNone : 
				{
					block.addCondition(logConditionFound);
					block.setType(logConditional);
					block.setUse(true);
				}
				break;
			default : break;
		}

		if (0 != after.getAllow()) {
			break;
		}

		allow.removeCondition(logConditionFound);
		allow.fileCopy.removeCondition(logConditionFound);

		if (0 != allow.getCondition()) {
			break;
		}

		allow.setType(logNone);
		allow.setUse(false);
		allow.fileCopy.setType(logNone);
		allow.fileCopy.setUse(false);
	}while(false);

	return true;
}

bool CDLPLogs::validateWithNotFound(const CDLPControlCount& before, const CDLPControlCount& after) {
	do {
		if (logConditional != allow.getType()) {
			break;
		}

		if ((0 >= before.getApproval()) || (0 != after.getApproval())) {
			break;
		}

		if (0 == (logConditionNotFound & allow.getCondition())) {
			break;
		}

		switch(block.getType()) {
			case logConditional : 
				{
					if (0 == (logConditionNotFound & block.getCondition())) {
						block.addCondition(logConditionNotFound);
					}
				}
				break;
			case logNone : 
				{
					block.addCondition(logConditionNotFound);
					block.setType(logConditional);
					block.setUse(true);
				}
				break;
			default : break;
		}

		if (0 != after.getAllow()) {
			break;
		}

		allow.removeCondition(logConditionNotFound);
		allow.fileCopy.removeCondition(logConditionNotFound);

		if (0 != allow.getCondition()) {
			break;
		}

		allow.setType(logNone);
		allow.setUse(false);
		allow.fileCopy.setType(logNone);
		allow.fileCopy.setUse(false);
	}while(false);

	return true;

}

bool CDLPLogs::isSaveLogAny(void) {
	bool result = true;
	do {
		if (true == block.isUse()) {
			break;
		}

		if (true == allow.isUse()) {
			break;
		}

		result = false;
	}while(false);

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPControlCount
CDLPControlCount::CDLPControlCount() {
	block = 0;
	allow = 0;
	approval = 0;
}

CDLPControlCount::~CDLPControlCount() {
};


////////////////////////////////////////////////////////////////////////////////
//class CDLPCopyControl
CDLPCopyControl::CDLPCopyControl() {
}

CDLPCopyControl::~CDLPCopyControl() {
}

bool CDLPCopyControl::setXml(CMarkup& markup) { 
	usbDriveControl.setXml(markup);

	if (true == markup.IntoElem()) {
		while (true == markup.FindElem("controls")) {
			CDLPControl control;
			if (true == control.setXml(markup)) {
				controls.push_back(control);
			}
		}

		logs.setXml(markup);
		markup.OutOfElem();
	}

	DEBUG_LOG("copy_control - control_count:%d", controls.size());

	return true;
}

//bool CDLPCopyControl::getXml(CMarkup& markup) { 
//	usbDriveControl.getXml(markup);
//
//	if (true == markup.IntoElem()) {
//		if (0 < controls.size()) {
//			CDLPControl::VECTOR::const_iterator itr = controls.begin();
//			for (; itr != controls.end(); ++itr) {
//				CDLPControl& control = (CDLPControl&)*itr;
//				control.getXml(markup);
//			}
//		}
//
//		logs.getXml(markup);
//		markup.OutOfElem();
//	}
//
//	return true; 
//}

bool CDLPCopyControl::hasBlockOrApproval() const {
	if (0 == controls.size()) {
		return false;
	}

	CDLPControl::VECTOR::const_iterator itr = controls.begin();
	for (; itr != controls.end(); ++itr) {
		CDLPControl& control = (CDLPControl&)*itr;
		if( true == control.hasBlockOrApproval()){
			return true;
		}
	}
	
	return false; 
}

bool CDLPCopyControl::changeControlApprovalToBlock(void) {
	if (0 == controls.size()) {
		return false;
	}

	CDLPControlCount foundBefore, notFoundBefore, noInspectBefore;
	CDLPControlCount foundAfter, notFoundAfter, noInspectAfter;

	if (true == logs.hasLogEnable()) {
		getDLPControlCount(foundBefore, notFoundBefore, noInspectBefore);
	}

	int inspectOption = 0;
	CDLPControl::VECTOR::const_iterator itr = controls.begin();
	for (; itr != controls.end(); ++itr) {
		CDLPControl& control = (CDLPControl&)*itr;
		control.changeControlApprovalToBlock();
		inspectOption = control.getInspectOption();
	}

	if (true == logs.hasLogEnable()) {
		getDLPControlCount(foundAfter, notFoundAfter, noInspectAfter);
		logs.validate(inspectOption, foundBefore, notFoundBefore, noInspectBefore,
				foundAfter, notFoundAfter, noInspectAfter);
	}

	return true;
}

bool CDLPCopyControl::getDLPControlCount(CDLPControlCount& found, CDLPControlCount& notFound, CDLPControlCount& noInspect) {
	if (0 == controls.size()) {
		return false;
	}

	CDLPControl::VECTOR::const_iterator itr = controls.begin();
	for (; itr != controls.end(); ++itr) {
		CDLPControl& control = (CDLPControl&)*itr;
		control.getDLPControlCount(found, notFound, noInspect);
	}

	return true;
}

bool CDLPCopyControl::isSaveLogAny(void) {
	return logs.isSaveLogAny();
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPCopyControls
CDLPCopyControls::CDLPCopyControls() {
}

CDLPCopyControls::~CDLPCopyControls() {
}

bool CDLPCopyControls::setXml(CMarkup& markup) { 
	DEBUG_LOG1("copy_controls - begin");

	while (true == markup.FindElem("usb")) {
		CDLPCopyControl copyControl;
		if (true == copyControl.setXml(markup)) {
			copyControls.push_back(copyControl);
		}
	}

	DEBUG_LOG("copy_controls - end -  list_count:%d", copyControls.size());

	return true;
}

//bool CDLPCopyControls::getXml(CMarkup& markup) { 
//	if (0 == copyControls.size()) {
//		return false;
//	}
//
//	CDLPCopyControl::VECTOR::const_iterator itr = copyControls.begin();
//	for (; itr != copyControls.end(); ++itr) {
//		markup.AddElem("usb");
//		CDLPCopyControl& copyControl = (CDLPCopyControl&)*itr;
//		copyControl.getXml(markup);
//	}
//
//	return true; 
//}

bool CDLPCopyControls::hasBlockOrApproval() const {
	if (0 == copyControls.size()) {
		return false;
	}

	CDLPCopyControl::VECTOR::const_iterator itr = copyControls.begin();
	for (; itr != copyControls.end(); ++itr) {
		CDLPCopyControl& copyControl = (CDLPCopyControl&)*itr;
		if( true == copyControl.hasBlockOrApproval()){
			return true;
		}
	}
	
	return false; 
}

bool CDLPCopyControls::changeControlApprovalToBlock(void) {
	if (0 == copyControls.size()) {
		return false;
	}

	CDLPCopyControl::VECTOR::const_iterator itr = copyControls.begin();
	for (; itr != copyControls.end(); ++itr) {
		CDLPCopyControl& copyControl = (CDLPCopyControl&)*itr;
		copyControl.changeControlApprovalToBlock();
	}
	
	return true;
}

bool CDLPCopyControls::isBlockReadAll(void) {
	if (1 != copyControls.size()) {
		return false;
	}

	CDLPCopyControl::VECTOR::const_iterator itr = copyControls.begin();
	for (; itr != copyControls.end(); ++itr) {
		CDLPCopyControl& copyControl = (CDLPCopyControl&)*itr;
		if (CDLPUSBDriveControl::ALL == copyControl.usbDriveControl.getSelectType()) {
			return !(copyControl.usbDriveControl.isReadPermit());
		}
	}

	return false;
}

bool CDLPCopyControls::isSaveLogAny(void) {
	if (0 == copyControls.size()) {
		return false;
	}

	CDLPCopyControl::VECTOR::const_iterator itr = copyControls.begin();
	for (; itr != copyControls.end(); ++itr) {
		CDLPCopyControl& copyControl = (CDLPCopyControl&)*itr;
		if (true == copyControl.isSaveLogAny()) {
			return true;
		}
	}

	return false;
}

bool CDLPCopyControls::getDLPCopyControl(const CDLPUSBDrive* usbDrive, CDLPCopyControl& copyControl) {
	DEBUG_LOG1("copy_controls - begin");
	if (0 == copyControls.size())
    {
		DEBUG_LOG1("copy_controls - stop - list_empty");
		return false;
	}

	bool selected = false;
	if (NULL != usbDrive)
    {
		if (0 < usbDrive->getMediaGuid().length())
        {
			selected = true;
		}
        DEBUG_LOG("copy_controls - selcted:%s - guid:%s", (selected)?"true":"false", usbDrive->getMediaGuid().c_str());
	}
    
	CDLPCopyControl::VECTOR::const_iterator itr = copyControls.begin();
	for (; itr != copyControls.end(); ++itr)
    {
		CDLPCopyControl& temp= (CDLPCopyControl&)*itr;
		if (true == selected)
        {
			if (CDLPUSBDriveControl::SELECTED == temp.usbDriveControl.getSelectType())
            {
				copyControl = (CDLPCopyControl&)*itr;
				DEBUG_LOG1("copy_controls - selected - found");
				return true;
			}
		}
        else
        {
			if (CDLPUSBDriveControl::SELECTED != temp.usbDriveControl.getSelectType())
            {
				copyControl = (CDLPCopyControl&)*itr;
				DEBUG_LOG1("copy_controls - not_selected - found");
				return true;
			}
		}
	}

	DEBUG_LOG1("copy_controls - end - not_found");
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPNotify
CDLPNotify::CDLPNotify() {
	option = 0;
}

CDLPNotify::~CDLPNotify() {
}

bool CDLPNotify::setXml(CMarkup& markup) { 
	if (false == markup.FindElem("notify")) {
		return false;
	}

	option = getNotify(markup.GetAttrib("type"));

	return true;
}

//bool CDLPNotify::getXml(CMarkup& markup) { 
//	markup.AddElem("notify");
//	markup.SetAttrib("type", getNotify(option));
//
//	return true; 
//}

std::string CDLPNotify::getNotify(const int value) const {
	std::string result = "";
	switch(value) {
		case notifyNone : result = "none"; break;
		case notifyAlways: result = "always"; break;
		case notifyBlock : result = "block"; break;
		default : break;
	}

	return result;
}

int CDLPNotify::getNotify(const std::string& value) {
	int result = notifyNone;
	if ("none" == value) {
		result = notifyNone;
	} else if ("always" == value) {
		result = notifyAlways;
	} else if ("block" == value) {
		result = notifyBlock;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//class CDLPAdvancedInfo
CDLPAdvancedInfo::CDLPAdvancedInfo() {
	warningOnFileCopy = 0;
}

CDLPAdvancedInfo::~CDLPAdvancedInfo() {
}

bool CDLPAdvancedInfo::setXml(CMarkup& markup) { 
	if (false == markup.FindElem("advanced")) {
		return false;
	}

	warningOnFileCopy = util.getValueAsBool(markup.GetAttrib("warning_on_filecopy"));

	return true;
}

//bool CDLPAdvancedInfo::getXml(CMarkup& markup) { 
//	markup.AddElem("advaned");
//	markup.SetAttrib("warning_on_filecopy", util.getValueAsBool(warningOnFileCopy ));
//
//	return true; 
//}


////////////////////////////////////////////////////////////////////////////////
//class CDLPMediaInfo
CDLPMediaInfo::CDLPMediaInfo() {
	permitType = 0;
}

CDLPMediaInfo::~CDLPMediaInfo() {
}

bool CDLPMediaInfo::setXml(CMarkup& markup) { 
	guids.clear();

	if (false == markup.FindElem("media_list")) {
		return false;
	}

	permitType = atoi(markup.GetAttrib("type").c_str());
	while (true == markup.FindChildElem("media")) {
		guids.push_back(markup.GetChildAttrib("guid"));
	}

	return true;
}

//bool CDLPMediaInfo::getXml(CMarkup& markup) { 
//	markup.AddElem("media_list");
//	markup.SetAttrib("type", permitType);
//	markup.SetAttrib("type_desc", getPermitType(permitType));
//
//	if (0 < guids.size()) {
//		std::vector<std::string>::iterator itr = guids.begin();
//		for (; itr != guids.end(); ++itr) {
//			markup.AddChildElem("media");
//			markup.SetChildAttrib("guid", *itr);
//		}
//	}
//
//	return true; 
//}

//std::string CDLPMediaInfo::getPermitType(const int value) {
//	std::string result = "";
//	switch(value) {
//		case mdPermitOwner : result = "owner"; break;
//		case mdPermitOwnerDept : result = "owner_dept"; break;
//		case mdPermitDirect : result = "direct"; break;
//		case mdPermitAll: result = "all"; break;
//		default : break;
//	}
//
//	return result;
//}
//

////////////////////////////////////////////////////////////////////////////////
//class CPIMediaData 
CPIMediaData::CPIMediaData() {
}

CPIMediaData::~CPIMediaData() {
}

void CPIMediaData::clear(void) {
	CPIControlItem::clear();
	serialNumber = "";
	expireDate = "";
	purpose = 0;
	type = 0;
	guid = "";
}
#endif // #ifndef _DLPPOLICY_CPP
