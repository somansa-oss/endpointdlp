#ifndef	_PIDEVICELOG_CPP
#define	_PIDEVICELOG_CPP

#include <map>

#include "PIDeviceLog.h"
#include "PIXmlParser.h"
#include"DLPPolicy.h"

////////////////////////////////////////
//class CPIDeviceLog 

CPIDeviceLog::CPIDeviceLog() {
	clear();
}

CPIDeviceLog::~CPIDeviceLog() {	
}

void CPIDeviceLog::clear(void) {
	CPIObject::clear();

	// SMART_LOG_RECORD_EX
	policyType = 0x00;

	// SMART_LOG_RECORD
	accessType = accessUnknown;
	recordType = RECORD_TYPE_NORMAL;

	// LOG_DATA
	logTime.clear();
	logType = LOG_VNODE;
	logAction = ACTION_NONE;
	disableResult = false;
	processId = 0;
	processName.clear();
	deviceName.clear();
	fileName.clear();
	deviceLogXml.clear();
	
	seq = 0;
	deviceType = CPIDeviceLog::typeNone;
	memset((void*)&tmLogTime, 0x00, sizeof(struct tm));

	virtualType.clear();
	guid.clear();
}

std::string CPIDeviceLog::string(void) {

	if( false == deviceLogXml.empty() ) {
		return deviceLogXml;
	}

	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;

	markup.AddElem("monitor");
	markup.AddAttrib("create_time", getLogTime().c_str());
	markup.AddAttrib("processname", processName.c_str());

	std::string tempType = getDeviceType();
	markup.AddAttrib("type", tempType);

	if( "device" == tempType ) {
		if( false == deviceName.empty() ) {
			markup.AddAttrib("device", deviceName.c_str());
		} else {
			markup.AddAttrib("device", fileName.c_str());
		}
	}
	else if( "drive" == tempType ) {
		markup.AddAttrib("filepath", fileName.c_str());
	}
	else if( "ndis" == tempType ) {
		markup.AddAttrib("device", deviceName.c_str());
	}
	else if( "process" == tempType ) {
		markup.AddAttrib("filepath", fileName.c_str());
	}

	markup.AddAttrib("virtualtype", this->getVirtualType().c_str());
	markup.AddAttrib("operation", this->getOperation().c_str());
	markup.AddAttrib("operationcode", this->getOperationCode().c_str());
	markup.AddAttrib("access", (int)this->getAccess());
	markup.AddAttrib("disabletype", this->getDisableType());
	markup.AddAttrib("adapterbustype", this->getAdapterBusType().c_str());
	markup.AddAttrib("guid", this->guid.c_str());

	deviceLogXml = (markup.IsWellFormed()) ? markup.GetDoc() : "<warning>not well formed xml</warning>";

	return deviceLogXml;
}

void CPIDeviceLog::setLogTime(std::string logTime) {
	this->logTime = logTime;
	util.getValueAsTime(logTime, this->tmLogTime);

	DEBUG_LOG("%s", util.getValueAsString(this->tmLogTime).c_str());
}

std::string CPIDeviceLog::getLogTime(void) {
	return logTime;
}

std::string CPIDeviceLog::getVirtualType(void) {

	std::string result = "";

	if( false == virtualType.empty() ) {
		return virtualType;
	}
	else {
		CDeviceV6& deviceV6 = CDeviceV6::getInstance();
		unsigned long ulStorDeviceType = GET_STORDEVICETYPE(policyType);

		for(int  index = 0; index < CDeviceV6::COUNT; ++index) {

			CDeviceV6::EM_TYPE type = static_cast<CDeviceV6::EM_TYPE>(index);
			if( ulStorDeviceType == deviceV6.map[type].code ) {

				switch(index) {
					case CDeviceV6::COPYPREVENT : break;
					case CDeviceV6::CDDVD		: result = "Drive\\CD/DVD"; break;
					case CDeviceV6::FLOPPY		: result = "Drive\\Floppy"; break;
					case CDeviceV6::USB		: result = "Drive\\RemovableMedia"; break;
					case CDeviceV6::NETDRIVE	: result = "Drive\\NetDrive"; break;
					case CDeviceV6::SERIAL		: result = "Device\\Serial"; break;
					case CDeviceV6::PARALLEL	: result = "Device\\Parallel"; break;
					case CDeviceV6::IEEE1394	: result = "Device\\IEEE1394"; break;
					case CDeviceV6::MTP		:
					case CDeviceV6::PTP		:
					case CDeviceV6::ANDROID		: 
					case CDeviceV6::APPLE		: result = "Device\\USBMobile"; break;
                    case CDeviceV6::BLUETOOTH	: result = "Device\\Bluetooth"; break;
					case CDeviceV6::PROCESS		: result = "Process\\Control"; break;
					default : break;
				}

				break; // end of loop condition
			}
		}
	}

	return result;
}

std::string CPIDeviceLog::getOperation(void) {

	std::string result;
	switch(accessType) {
		case accessRead 	: result = "read"; break;
		case accessWrite 	: result = "write"; break;
		default 			: result = "access"; break;
	}

	return result;
}

std::string CPIDeviceLog::getOperationCode(void) {
	//do nothing here
	return "";
}

unsigned long CPIDeviceLog::getAccess(void) {
    return accessType;	
}

std::string CPIDeviceLog::getDisableType(void) {
	return util.getValueAsString(disableResult);
}

std::string CPIDeviceLog::getAdapterBusType(void) {
	//do nothing here
	return "";
}

std::string CPIDeviceLog::getDeviceType(void) {
	std::string result;

	switch(deviceType) {
		case CPIDeviceLog::typeDevice : result = "device"; break;
		case CPIDeviceLog::typeDrive : result = "drive"; break;
		case CPIDeviceLog::typeNdis : result = "ndis"; break;
		case CPIDeviceLog::typeProcess : result = "process"; break;
		default : break;
	}

	return result;
}

bool CPIDeviceLog::isOutDated(const int maxTimeSpan) {

	double seconds = util.diffTime(this->tmLogTime);
	DEBUG_LOG("devicelog - seconds:%f - max_time_span(%d)", seconds, maxTimeSpan);

	if( seconds > (double) maxTimeSpan ) {
		return true;
	}
	
	return false;
}

bool CPIDeviceLog::isDuplicated(const CPIDeviceLog& deviceLog) {
	bool result = false;
	do {
		if( deviceLog.policyType != policyType ) {
			break;
		}

		if( deviceLog.accessType != accessType ) {
			break;
		}

		if( deviceLog.recordType != recordType  ) {
			break;
		}

		if( deviceLog.logType != logType ) {
			break;
		}
		
		if( deviceLog.logAction != logAction ) {
			break;
		}
	
		if( deviceLog.disableResult != disableResult ) {
			break;
		}
	
		if( deviceLog.processId != processId ) {
			break;
		}
	
		if( deviceLog.processName != processName ) {
			break;
		}
		
		if( deviceLog.deviceName != deviceName ) {
			break;
		}

		if( deviceLog.fileName != fileName ) {
			break;
		}

		result = true;
	} while(false);

	return result;
}

void CPIDeviceLog::setDeviceTypeByPolicyType(const unsigned long policyType)
{
	CDeviceV6& deviceV6 = CDeviceV6::getInstance();
	unsigned long ulStorDeviceType = GET_STORDEVICETYPE(policyType);

	for(int  index = 0; index < CDeviceV6::COUNT; ++index)
    {
		CDeviceV6::EM_TYPE type = static_cast<CDeviceV6::EM_TYPE>(index);
		if( ulStorDeviceType == deviceV6.map[type].code )
        {
			switch(index)
            {
				case CDeviceV6::COPYPREVENT : break;
				case CDeviceV6::CDDVD : 
				case CDeviceV6::FLOPPY : 
				case CDeviceV6::USB : 
				case CDeviceV6::NETDRIVE : deviceType = CPIDeviceLog::typeDrive; break;
				case CDeviceV6::PROCESS : deviceType = CPIDeviceLog::typeProcess; break;
				default : deviceType = CPIDeviceLog::typeDevice; break;
			}

			break; // end of loop condition
		}
	}
}

void CPIDeviceLog::setDeviceName(std::string value) {
	if (EXCEPT_FLAG_POLCY == policyType) {
		virtualType = "Drive\\Removable";
		deviceName = fileName;
	} else {
		const int POLICY_TYPE = GET_STORDEVICETYPE(policyType);
		switch(POLICY_TYPE) {
			case MEDIA_CAMERA : 
				{
					accessType = accessWrite;
					processName = "kernel_task";
					virtualType = "Device\\Camera";
					deviceName = fileName;
				}
				break;
			case MEDIA_RNDIS : 
				{
					accessType = accessWrite;
					processName = "kernel_task";
					virtualType = "NDIS\\WWAN";
					deviceName = fileName;
				}
				break;
			case MEDIA_AIRDROP : 
				{
					virtualType = "Share\\AirDrop";
					deviceName = "AirDrop";
				} 
				break;
			case MEDIA_COPYPREVENT : 
				{
					virtualType = "Drive\\Removable";
					deviceName = fileName;
				}
				break;
			default : break;
		}
	}
}
#endif // #ifndef _PIDEVICELOG_CPP
