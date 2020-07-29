#ifndef	_PIPOLICYMAN_CPP
#define	_PIPOLICYMAN_CPP

#include <map>

#include"PIPolicyMan.h"
#include"PIDocument.h"
#include"PIXmlParser.h"

#include <time.h>
#include <cstdio>

using namespace nsPISecObject;

////////////////////////////////////////////////////////////////////////////////
//class CPIPolicySet 
CPIPolicySet::CPIPolicySet() {
	clear();
}

CPIPolicySet::~CPIPolicySet() {
}

void CPIPolicySet::clear(void) {

	CPIObject::clear();

	isOnline = false;
}

////////////////////////////////////////////////////////////////////////////////
//class CPIPolicyMan
CPIPolicyMan::CPIPolicyMan() {
	initialize();
}

CPIPolicyMan::~CPIPolicyMan() {	
	finalize();
}

bool CPIPolicyMan::initialize(void) {
	CPIObject::initialize();

	clear();
	pthread_mutex_init( &mutexPolicy, 0 );

	return true;
}

bool CPIPolicyMan::finalize(void) {

	pthread_mutex_unlock( &mutexPolicy); 
	pthread_mutex_destroy( &mutexPolicy); 
	
	return CPIObject::finalize();
}

void CPIPolicyMan::clear(void) {
	CPIObject::clear();
	
	ndisClassList.clear();
	usbmobileClassList.clear();

	ndisList.clear(); 
	usbmobileList.clear();
	
	clearDLPPolicy(onlinePolicyList);
	clearDLPPolicy(offlinePolicyList);
	clearDLPPolicy(commonPolicyList);

	enableMediaControlBlockLog = false;

}

bool CPIPolicyMan::load(void) {
	DEBUG_LOG1("policy");

	this->getLock();
	clear();
	loadNdisXml();
	loadUSBMobileXml();
	loadDLPPolicyXML();
	validateDLPPolicyXml();
	enableMediaControlBlockLog = isEnableBlockLog();
	this->releaseLock();

	return true;
}

bool CPIPolicyMan::translatePolicy(void) {
	DEBUG_LOG1("policy");

	makeDeviceRule();

	std::string policySet = getDLPPolicyForPISupervisor();
	if( 0 == policySet.length() ) {
		ERROR_LOG1("policy - invalid_policyset");
		return false;
	}

	writePolicySet(policySet);

	return true;
}

bool CPIPolicyMan::adjust(void) {
	DEBUG_LOG1("policy");
	
	// reserved
	return true;
}


bool CPIPolicyMan::loadDLPPolicyXML(void) {
	DEBUG_LOG1("policy");

	std::string policyFile= ConfigMan.path.getDLPPolicyFile();

	bool result = false;
	CPIXmlParser parser;
	CMarkup& markup = parser.markup;
	do {
		if( false == markup.Load(policyFile) ) {
			ERROR_LOG("load_fail - %s", policyFile.c_str());
			break;
		}

		if( false == parser.findPath("pisec_policy_info/policy_list/pisec_policy") ) {
			ERROR_LOG1("find_path_fail");
			break;
		}

		markup.IntoElem();
        if( true == markup.FindElem("policyset") ) {
			markup.IntoElem();
			while( true == markup.FindElem("policy") ) {
				std::string type = markup.GetAttrib("type");
				DEBUG_LOG("policy_type - %s", type.c_str());
				
				markup.IntoElem();

				while( true == markup.FindElem("dlppolicy") ) {
					std::string guid = markup.GetAttrib("dlppolicy_guid");
					DEBUG_LOG("policy_guid - %s", guid.c_str());
					
					if( true == isNullPolicy(guid) ){
						DEBUG_LOG("null_policy - guid(%s)", guid.c_str());
						continue;
					}

					int dlpType = atoi(markup.GetAttrib("dlppolicy_type").c_str());
					int dlpSubType = atoi(markup.GetAttrib("dlppolicy_subtype").c_str());

					CDLPPolicy* dlpPolicyPtr = addDLPPolicy(dlpType, dlpSubType, type);
					if( NULL == dlpPolicyPtr ) {
						ERROR_LOG("guid(%s), policy_type(%s), type(%d,%d)", 
								guid.c_str(), type.c_str(), dlpType, dlpSubType);
					}
					else {
						DEBUG_LOG1("parse policy - begin");
						dlpPolicyPtr->parsePolicyInfo(markup);
						DEBUG_LOG1("parse policy - end");
						
						DEBUG_LOG("name(%s), guid(%s), policy_type(%s), revision(%d), type(%d,%d)", 
								dlpPolicyPtr->name.c_str(), dlpPolicyPtr->guid.c_str(), type.c_str(),
								dlpPolicyPtr->revision, dlpPolicyPtr->dlpType, dlpPolicyPtr->dlpSubType);
					}
				}
				markup.OutOfElem();
			}
			markup.OutOfElem();
		}

		if( true == markup.FindElem("common") ) {
			markup.IntoElem();
			while( true == markup.FindElem("dlppolicy") ) {
				std::string guid = markup.GetAttrib("guid");
				if( true == isNullPolicy(guid) ){
					continue;
				}

				int dlpType = atoi(markup.GetAttrib("dlppolicy_type").c_str());
				int dlpSubType = atoi(markup.GetAttrib("dlppolicy_subtype").c_str());

				CDLPPolicy* dlpPolicyPtr = addDLPPolicy(dlpType, dlpSubType);
				dlpPolicyPtr->parsePolicyInfo(markup);
			}
			markup.OutOfElem();
		}

		if( true == markup.FindElem("ndis_class_list") ) {
			while(true == markup.FindChildElem("ndis_class")) {
				CNDISClass ndisClass;
				ndisClass.keyword = markup.GetChildAttrib("keyword");
				ndisClass.name = markup.GetChildAttrib("name");
				ndisClassList.push_back(ndisClass);
			}
		}

		if( true == markup.FindElem("usbmobile_class_list") ) {
			while(true == markup.FindChildElem("usbmobile_class")) {
				CUSBMobileClass usbmobileClass;
				usbmobileClass.keyword = markup.GetChildAttrib("keyword");
				usbmobileClass.name = markup.GetChildAttrib("name");
				usbmobileClassList.push_back(usbmobileClass);
			}
		}
        
		markup.OutOfElem();

		result = true;
	}while(false);

	return result;

}

bool CPIPolicyMan::validateDLPPolicyXml(void) {
	DEBUG_LOG1("policy");

	// offline : approval --> block
	for(int index = 0; index < offlinePolicyList.size(); index++) {
		CDLPPolicyPreventPlus* policy = dynamic_cast<CDLPPolicyPreventPlus*>(offlinePolicyList[index]);
		if( NULL!= policy ) {
			policy->changeControlApprovalToBlock();
		}
	}

	return true;
}

bool CPIPolicyMan::isNullPolicy(const std::string & guid) {
const std::string STR_KEYWORD_NULLTHEME_GUID1 = "00000000-0000-0000-0000-000000000000";
const std::string STR_KEYWORD_NULLTHEME_GUID2 = "{00000000-0000-0000-0000-000000000000}";

	if( ( guid == STR_KEYWORD_NULLTHEME_GUID1 ) ||
			( guid == STR_KEYWORD_NULLTHEME_GUID2 ) ) {
		return true;
	}

	return false;
}
					
CDLPPolicy* CPIPolicyMan::addDLPPolicy(const int dlpType, const int dlpSubType, std::string type) {
	if(false == ConfigMan.dlpPolicy.isEnableWithSubType(dlpSubType)) {
		DEBUG_LOG("policy - skip - subtype:0x%08x", dlpSubType);
		return NULL;
	}

	CDLPPolicy* dlpPolicy = NULL;
	switch(dlpType) {
		case dlptypeNone : break;
		case dlptypePreventPlus : 
		case dlptypeControl : 
			switch(dlpSubType) {
			case dlpsubtypeCopyPreventPlus : 		dlpPolicy = new CDLPPolicyCopy; break;
			case dlpsubtypeUploadPreventPlus : 		dlpPolicy = new CDLPPolicyUpload; break;
			case dlpsubtypePrintPreventPlus : 		dlpPolicy = new CDLPPolicyPrint; break;
			case dlpsubtypeApplicationPreventPlus : 	dlpPolicy = new CDLPPolicyApplication; break;
			case dlpsubtypeClipboardPreventPlus : 	dlpPolicy = new CDLPPolicyClipboard; break;
			case dlpsubtypeMediaControl : 			dlpPolicy = new CDLPPolicyMedia; break;
			case dlpsubtypePCSecurity : 				dlpPolicy = new CDLPPolicyPCSecurity; break;
			case dlpsubtypeSharedFolderPreventPlus : dlpPolicy = new CDLPPolicySharedFolderCopy; break;
			case dlpsubtypeCopyExPreventPlus : 		dlpPolicy = new CDLPPolicyCopyEx; break;
			default : break;
			}
			break;
		case dlptypeCommon : break;
		default : break;
	}

	if( NULL != dlpPolicy ) {
		if( "online" == type) { 
			dlpPolicy->dlpControlMode = controlmodeOnline;
			onlinePolicyList.push_back(dlpPolicy); 
		} 
		else if( "offline" == type) { 
			dlpPolicy->dlpControlMode = controlmodeOffline;
			offlinePolicyList.push_back(dlpPolicy); 
		} 
		else { 
			dlpPolicy->dlpControlMode = controlmodeOnOffline;
			commonPolicyList.push_back(dlpPolicy); 
		}
	}

	return dlpPolicy;
}

void CPIPolicyMan::clearDLPPolicy(CDLPPolicy::Vector & vector) {
	if( 0 == vector.size()) {
		return;
	}

	for(int index = 0; index < vector.size(); index++) {
		delete vector[index];
	}
	vector.clear();
}

std::string CPIPolicyMan::getDLPPolicyForPISupervisor(void) {
	DEBUG_LOG1("policy");
	
	std::string result;

	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;

	markup.AddElem("policyset");
	markup.IntoElem();
	markup.AddElem("info");
	markup.SetAttrib("type", (policySet.isOnline) ? "online" : "offline");
	markup.SetAttrib("timestamp", util.getCurrentDateTime());
	markup.SetAttrib("enable_block_log", util.getValueAsString(enableMediaControlBlockLog));
	markup.IntoElem();
	markup.AddElem("ndis_class_list");
	if( 0 < ndisClassList.size() ) {
		for(int index = 0; index <  ndisClassList.size(); index++ ) {
			markup.AddChildElem("ndis_class");
			markup.SetChildAttrib("keyword", ndisClassList[index].keyword);
			markup.SetChildAttrib("name", ndisClassList[index].name);
		}
	}
	markup.AddElem("usbmobile_class_list");
	if( 0 < usbmobileClassList.size() ) {
		for(int index = 0; index <  usbmobileClassList.size(); index++ ) {
			markup.AddChildElem("usbmobile_class");
			markup.SetChildAttrib("keyword", usbmobileClassList[index].keyword);
			markup.SetChildAttrib("name", usbmobileClassList[index].name);
		}
	}
	markup.OutOfElem();
	markup.AddElem("policy_list");
	markup.IntoElem();
	getDLPPolicyForPISupervisor(markup, controlmodeOnline);
	getDLPPolicyForPISupervisor(markup, controlmodeOffline);
	markup.OutOfElem();

	return markup.GetDoc();
}

bool CPIPolicyMan::writePolicySet(const std::string& policySet) {
	DEBUG_LOG1("policy");

	std::string policySetFile = ConfigMan.path.getPolicySetFile();
	
	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;
	markup.SetDoc(policySet);
	markup.Save(policySetFile);
	
	return true;
}

bool CPIPolicyMan::isEnableBlockLog(void) { 
	
	bool result = false;
	CDLPPolicy::Vector& policyList = (policySet.isOnline) ? onlinePolicyList : offlinePolicyList;
	for(int index = 0; index < policyList.size(); index++) {
		CDLPPolicy* policy = policyList[index];
		if( (NULL == policy ) || ( false == policy->use ) ) {
			continue;
		}

		if(dlpsubtypeMediaControl != policy->dlpSubType) {
			continue;
		}

		if( true == policy->isEnableBlockLog() ) {
			result = true;
			break;
		}
	}

	return result;
}

bool CPIPolicyMan::getDLPPolicyForPISupervisor(CMarkup& markup, const int controlMode) {

	markup.AddElem("policy");
	markup.SetAttrib("type", (controlmodeOnline == controlMode ) ? "online" : "offline"  );
	markup.IntoElem();
	markup.AddElem("rule_list");
	markup.IntoElem();
	makeDeviceRuleXml(markup, controlMode);
	markup.OutOfElem();
	markup.AddElem("info");
	markup.IntoElem();
	makeInformationXml(markup, controlMode);
	markup.OutOfElem();
	markup.OutOfElem();

	return true;
}

void CPIPolicyMan::getDLPPolicyList(const int controlMode, CDLPPolicy::Vector*& policyList) {
	
	policyList = NULL;
	switch(controlMode) {
		case controlmodeOnline : policyList = &onlinePolicyList; break;
		case controlmodeOffline : policyList = &offlinePolicyList; break;
		case controlmodeOnOffline: policyList = &commonPolicyList; break;
		default : break;
	}
}

bool CPIPolicyMan::makeDeviceRuleXml_copyPrevent(CMarkup& markup, CDLPPolicy::Vector* policyList, bool& copyPreventFound) {
	if( NULL == policyList) {
		return false;
	}
	
	for(int index = 0; index < policyList->size(); index++) {
		CDLPPolicy* policy = (*policyList)[index];
	
		if( 
				(NULL == policy) || 
				(false == policy->use) ||
				(false == policy->isEnableDeviceRule())
		  ) {
			continue;
		}

		if (dlpsubtypeCopyExPreventPlus == policy->dlpSubType) {
			policy->addDeviceRule(markup);
			copyPreventFound = true;
			DEBUG_LOG1("policyman - copy_ex - call:addDeviceRule(xml) (copy_ex - found)");
			break;
		}
	}

	return true;
}

bool CPIPolicyMan::makeDeviceRuleXml(CMarkup& markup, const int controlMode) {
	DEBUG_LOG1("policyman - begin");

	CDLPPolicy::Vector* policyList = NULL;
	getDLPPolicyList(controlMode, policyList);
	if( NULL == policyList) {
		ERROR_LOG1("policyman - invalid policy_list");
		return false;
	}

	DEBUG_LOG("policyman - check - policy_count:%d", policyList->size());
	
	bool copyPreventFound = false;
	makeDeviceRuleXml_copyPrevent(markup, policyList, copyPreventFound);

	for(int index = 0; index < policyList->size(); index++) {
		CDLPPolicy* policy = (*policyList)[index];
		
		if( 
				(NULL == policy) || 
				(false == policy->use) ||
				(false == policy->isEnableDeviceRule())
		  ) {
			continue;
		}

		DEBUG_LOG("policyman - mode:%d - type:%d - subtype:%d", policy->dlpControlMode, policy->dlpType, policy->dlpSubType);

		switch(policy->dlpSubType)
		{
			case dlpsubtypeCopyPreventPlus : 
				{
					if (false == copyPreventFound) {
						policy->addDeviceRule(markup);
						DEBUG_LOG1("policyman - copy - call:addDeviceRule(xml) - (copy_ex - not_found)");
					}
				}
				break;
			case dlpsubtypeCopyExPreventPlus :
				{
					DEBUG_LOG1("policyman - skip - copy_ex - found");
				}
				break;
			default : 
				{
					policy->addDeviceRule(markup); 
					DEBUG_LOG("policyman - subtype:%d - call:add_device_rule(xml)", policy->dlpSubType);
				}
				break;
		}
	}
	
	DEBUG_LOG1("policyman - end");
	return true;
}

bool CPIPolicyMan::makeInformationXml(CMarkup& markup, const int controlMode) {
	
	CDLPPolicy::Vector* policyList = NULL;
	getDLPPolicyList(controlMode, policyList);
	if( NULL == policyList) {
		ERROR_LOG1("policyman - invalid policy_list");
		return false;
	}

	for(int index = 0; index < policyList->size(); index++) {
		CDLPPolicy* policy = (*policyList)[index];
		if( (NULL == policy ) || ( false == policy->use ) ) {
			continue;
		}
		policy->makeInformationXml(markup);
	}
	
	return true;
}

bool CPIPolicyMan::makeDeviceRule(const int controlMode) {

	CDLPPolicy::Vector* policyList = NULL;
	getDLPPolicyList(controlMode, policyList);
	if( NULL == policyList) {
		ERROR_LOG1("policyman - invalid policy_list");
		return false;
	}
	
	makeDeviceRule(policyList);
	validateDeviceRule(policyList);

	return true;
}

bool CPIPolicyMan::makeDeviceRule(CDLPPolicy::Vector* policyList) {
	
	if( NULL == policyList ) {
		return false;
	}

	for(int index = 0; index < policyList->size(); index++) {
		CDLPPolicy* policy = (*policyList)[index];
		if( (NULL == policy ) || ( false == policy->use ) ) {
			continue;
		}

		DEBUG_LOG("policyman - type:%d - subtype:%d - call:add_device_rule", policy->dlpType, policy->dlpSubType);
		policy->addDeviceRule(ConfigMan.isBlockWriteAll());
	}

	return true;
}

bool CPIPolicyMan::validateDeviceRule(CDLPPolicy::Vector* policyList) {

	if( NULL == policyList ) {
		DEBUG_LOG1("policyman - policy_list_empty");
		return false;
	}

	// ----------
	// Disable PreventPlus By MediaControl
	// ----------

	disableDeviceRuleByMediaControl(policyList, dlpsubtypeSharedFolderPreventPlus, deviceEtcStorage, etcstorageBlockNetDrive);
	disableDeviceRuleByMediaControl(policyList, dlpsubtypeCopyPreventPlus, deviceEtcStorage, etcstorageBlockRemovableRead);
	disableDeviceRuleByMediaControl(policyList, dlpsubtypeCopyPreventPlus, deviceEtcStorage, etcstorageBlockRemovableWrite);

	// ----------
	// Disable MediaControl By PreventPlus
	// ----------

	disableDeviceRuleByPreventPlus(policyList);

	return true;
}

bool CPIPolicyMan::disableDeviceRuleByMediaControl(CDLPPolicy::Vector* policyList, const int dlpSubType, const int deviceType, const int mediaControlType) {
	if (NULL == policyList) {
		return false;
	}

	bool mediaControlFound = false;
	for (int index = 0; index < policyList->size(); ++index) {
		CDLPPolicy* policy = (*policyList)[index];
		if (
				(NULL == policy) ||
				(false == policy->use) ||
				(dlpsubtypeMediaControl != policy->dlpSubType)
		   ) {
			continue;
		}

		CDLPPolicyMedia* mediaControl = dynamic_cast<CDLPPolicyMedia*>(policy);
		if (NULL != mediaControl) {
			mediaControlFound = mediaControl->getControl(static_cast<EM_DEVICETYPE>(deviceType), mediaControlType);
			break;
		}
	}

	if (false == mediaControlFound) {
		DEBUG_LOG("policyman - skip - media_control(%d/%d) - not_found", deviceType, mediaControlType);
		return true;
	}

	bool stop = false;
	for (int index = 0; (index < policyList->size()) && (false == stop) ; ++index) {
		CDLPPolicy* policy = (*policyList)[index];
		if (
				(NULL == policy) ||
				(false == policy->use)
		   ) {
			continue;
		}

		setEnableDeviceRule(policy, dlpSubType);

		switch(dlpSubType) {
			case dlpsubtypeCopyPreventPlus :
			case dlpsubtypeCopyExPreventPlus :
				{
					// do nothing
				}
				break;
			default : 
				{
					if (dlpSubType == policy->dlpSubType) {
						stop = true;
					}
				}
				break;
		}
	}

	return true;
}

bool CPIPolicyMan::setEnableDeviceRule(CDLPPolicy* policy, const int dlpSubType) {
	if (NULL == policy) {
		return false;
	}

	switch(dlpSubType) {
		case dlpsubtypeCopyPreventPlus : 
		case dlpsubtypeCopyExPreventPlus : 
			{
				if (
						(dlpsubtypeCopyPreventPlus == policy->dlpSubType) ||
						(dlpsubtypeCopyExPreventPlus == policy->dlpSubType)
				   ) {
					policy->setEnableDeviceRule(false); 
				}
			}
			break;
		default : 
			{
				if (dlpSubType == policy->dlpSubType) {
					policy->setEnableDeviceRule(false); 
				} 
			}
			break;
	}

	return true;
}

bool CPIPolicyMan::makeDeviceRule(void) {
	DEBUG_LOG1("policy");
	
	makeDeviceRule(controlmodeOffline);
	makeDeviceRule(controlmodeOnline);
	
	return true;
}

bool CPIPolicyMan::isOnline(void) {
	return policySet.isOnline;
}

bool CPIPolicyMan::getCurrentRule(CPISecRule::List& ruleList)
{
	//DEBUG_LOG1("policy - begin");

	this->getLock();
	ruleList.clear();
	CDLPPolicy::Vector& policyList = (policySet.isOnline) ? onlinePolicyList : offlinePolicyList;
	
	//DEBUG_LOG("policy - count(%d)", policyList.size());
	
	bool copyPreventFound = false;
	for(int index = 0; index < policyList.size(); index++) {
		CDLPPolicy* policy = policyList[index];

		if( 
				(NULL == policy) || 
				(false == policy->use) ||
				(false == policy->isEnableDeviceRule())
		  ) {
			continue;
		}

		if ((dlptypePreventPlus == policy->dlpType) && (dlpsubtypeCopyExPreventPlus == policy->dlpSubType)) {
			ruleList.push_back(policy->rule);
			//DEBUG_LOG("policy - virtualtype(%s)", policy->rule.virtualType.c_str());
			copyPreventFound = true;
			break;
		}
	}
	
	
	for(int index = 0; index < policyList.size(); index++) {
		CDLPPolicy* policy = policyList[index];

		if( 
				(NULL == policy) || 
				(false == policy->use) ||
				(false == policy->isEnableDeviceRule())
		  ) {
			continue;
		}

		if ((dlptypeControl == policy->dlpType) && (dlpsubtypeMediaControl == policy->dlpSubType)) {
			CDLPPolicyMedia * policyMedia = dynamic_cast<CDLPPolicyMedia*>(policy);
			if( 0 < policyMedia->ruleList.size() ) {
				CPISecRule::List::iterator itr = policyMedia->ruleList.begin();
				while(itr != policyMedia->ruleList.end() ) {
					ruleList.push_back(*itr);
					//DEBUG_LOG("policy - virtualtype(%s)", itr->virtualType.c_str());
					++itr;
				}
			}
		}
		else if ((dlptypePreventPlus == policy->dlpType) && (dlpsubtypeCopyExPreventPlus == policy->dlpSubType)) {
			continue;
		}
		else if ((dlptypePreventPlus == policy->dlpType) && (dlpsubtypeCopyPreventPlus == policy->dlpSubType)) {
			if (false == copyPreventFound) {
			ruleList.push_back(policy->rule);
			//DEBUG_LOG("policy - virtualtype(%s)", policy->rule.virtualType.c_str());
			}
		}
		else {
			ruleList.push_back(policy->rule);
			//DEBUG_LOG("policy - virtualtype(%s)", policy->rule.virtualType.c_str());
		}
	}
	this->releaseLock();

	//DEBUG_LOG1("policy - end");
	return (0 == ruleList.size()) ? false : true;
}

bool CPIPolicyMan::setOnlineMode(void) {
	policySet.isOnline = true;
	return true;
}

bool CPIPolicyMan::setOfflineMode(void) {
	policySet.isOnline = false;
	return true;
}

bool CPIPolicyMan::getDLPPolicy(CDLPPolicy::Vector& policyList, int dlpType, int dlpSubType, CDLPPolicy*& policy) {
	//DEBUG_LOG("policy - begin - policy_count:%d", policyList.size());

	if (0 == policyList.size()) {
		DEBUG_LOG1("policy - policy_list_empty");
		return false;
	}

	for(int index = 0; index < policyList.size(); index++) {
		CDLPPolicy* temp = policyList[index];
		
		if( (NULL == temp) || ( false == temp->use ) ) {
			//DEBUG_LOG1("policy - invalid");
			continue;
		}
		
		//DEBUG_LOG("policy - type:%d - sub_type:%d", temp->dlpType, temp->dlpSubType);

		if( ( dlpType == temp->dlpType) && ( dlpSubType == temp->dlpSubType) ) {
			policy = temp;
			//DEBUG_LOG1("policy - end - found");
			return true;
		}
	}
	
	//DEBUG_LOG1("policy - end - not_found");
	return false;
}

CDLPPolicy* CPIPolicyMan::getCurrentDLPPolicy(int dlpType, int dlpSubType) {
	
	CDLPPolicy::Vector& policyList = (policySet.isOnline) ? onlinePolicyList : offlinePolicyList;

	CDLPPolicy* result = NULL;
	switch(dlpSubType) 
	{
		case dlpsubtypeCopyPreventPlus : 
		case dlpsubtypeCopyExPreventPlus : 
			{
				if (false == getDLPPolicy(policyList, dlpType, dlpsubtypeCopyExPreventPlus, result)) {
					if (true == getDLPPolicy(policyList, dlpType, dlpsubtypeCopyPreventPlus, result)) {
						DEBUG_LOG1("policyman - policy_copy - found");
					}
				} else {
					DEBUG_LOG1("policyman - policy_copyex - found");
				}
			}
			break;
		default : 
			{
				getDLPPolicy(policyList, dlpType, dlpSubType, result);
			}
			break;
	}


	return result;
}

void CPIPolicyMan::getLock(void) {
	pthread_mutex_lock( &mutexPolicy);
}

void CPIPolicyMan::releaseLock(void) {
	pthread_mutex_unlock( &mutexPolicy);
}

bool CPIPolicyMan::loadNdisXml(void) {
	DEBUG_LOG1("policy");

	std::string ndisFile = ConfigMan.path.getNdisFile();

	bool result = false;
	CPIXmlParser parser;
	CMarkup& markup = parser.markup;
	do {
		if( false == markup.Load(ndisFile) ) {
			ERROR_LOG("load_fail - %s", ndisFile.c_str());
			break;
		}
		
		if( true == markup.FindElem("ndis_class_list") ) {
			while(true == markup.FindChildElem("ndis_class")) {
				CNDISClass ndisClass;
				ndisClass.keyword = markup.GetChildAttrib("keyword");
				ndisClass.name = markup.GetChildAttrib("name");
				ndisList.push_back(ndisClass);
			}
		}

		markup.OutOfElem();

		result = true;
	}while(false);

	return result;
}


bool CPIPolicyMan::loadUSBMobileXml(void) {
	DEBUG_LOG1("policy");

	std::string usbmobileFile = ConfigMan.path.getUSBMobileFile();

	bool result = false;
	CPIXmlParser parser;
	CMarkup& markup = parser.markup;
	do {
		if( false == markup.Load(usbmobileFile) ) {
			ERROR_LOG("load_fail - %s", usbmobileFile.c_str());
			break;
		}
		
		if( true == markup.FindElem("usbmobile_list") ) {
			while(true == markup.FindChildElem("usbmobile")) {
				CUSBMobile usbmobile;
				usbmobile.type = markup.GetChildAttrib("type");
				usbmobile.basePath = markup.GetChildAttrib("base_path");
				usbmobile.keyword = markup.GetChildAttrib("keyword");
				usbmobileList.push_back(usbmobile);
			}
		}

		markup.OutOfElem();

		result = true;
	}while(false);

	return result;
}

void CPIPolicyMan::debugPrintUSBMobile(void) {

	size_t count = usbmobileList.size();
	DEBUG_LOG("usbmobile - count:%d", count);

	if( 0 == count ) {
		return;
	}

	CUSBMobile::Vector::iterator itr = usbmobileList.begin();
	for(; itr != usbmobileList.end(); itr++) {
		CUSBMobile& usbmobile = *itr;
		DEBUG_LOG("usbmobile - type:%s - base_path:%s - keyword:%s", usbmobile.type.c_str(), usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
	}
}

bool CPIPolicyMan::getProcessControl(bool& control, bool& log, size_t& listCount) {
	control = false;
	log = false;
	CDLPPolicyApplication* pDLPPolicy = dynamic_cast<CDLPPolicyApplication*>(getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeApplicationPreventPlus));
	if (NULL != pDLPPolicy) {
		control = true;
		log = pDLPPolicy->execControlBlockLog;
		listCount = pDLPPolicy->applicationList.size();
	}

	return true;
}

void CPIPolicyMan::clearMediaList(void) {
	mediaList.clear();
}

bool CPIPolicyMan::reloadMediaList(void) {
	DEBUG_LOG1("begin");

	clearMediaList();

	CDLPPolicyCopy* policyCopy = dynamic_cast<CDLPPolicyCopy*>(getCurrentDLPPolicy(dlptypePreventPlus, dlpsubtypeCopyPreventPlus));
	if (NULL != policyCopy) {
		if (mdPermitAll == policyCopy->getMediaPermitType()) {
			DEBUG_LOG1("skip - policy_copy - media_permit_type: permit_all");
			return true;
		}
	}

	std::string xmlFile = ConfigMan.path.getMediaSerialFile();
	DEBUG_LOG("file:%s", xmlFile.c_str());

	bool result = false;
	CPIXmlParser parser;
	CMarkup& markup = parser.markup;
	do {
		if (false == markup.Load(xmlFile)) {
			DEBUG_LOG("load_fail - %s", xmlFile.c_str());
			break;
		}

		if (false == markup.FindElem("pisec_media_info")) {
			break;
		}

		if (false == markup.IntoElem()) {
			break;
		}

		if (true == markup.FindElem("media_list")) {
			if (false == markup.IntoElem()) {
				break;
			}
	
			while (true == markup.FindElem("MediaData")) {
				std::string serial = markup.GetAttrib("SerialNumber");
				if (0 == serial.length()) {
					continue;
				}

				CPIMediaData media;
				media.serialNumber = serial;		
				media.expireDate = markup.GetAttrib("ExpireDate");
				media.purpose = util.toInt(markup.GetAttrib("Purpose"));
				media.type = util.toInt(markup.GetAttrib("MediaType"));
                
                const std::string& sExpireDate = markup.GetAttrib("MediaGuid");
                if (isExpiredUSB(sExpireDate)){
                    DEBUG_LOG("usb drive expired - serial:%s", media.serialNumber.c_str());
                    media.guid = "";
                   
                }
                else{
                    media.guid = markup.GetAttrib("MediaGuid");
                }
                   
				mediaList[serial] = media;
	
				DEBUG_LOG("add_media - pupose:%d - type:%d - guid:%s - serial:%s", media.purpose, media.type, media.guid.c_str(), media.serialNumber.c_str());
			}	

			markup.OutOfElem();
		}
			
		markup.OutOfElem();

		result = true;
	}while(false);

	DEBUG_LOG1("end");
	return result;
}

bool CPIPolicyMan::isExpiredUSB(const std::string& sExpireDate){
    
    bool bExpired = false;
    
    if (sExpireDate.length() == 0)
        return bExpired;
    
    int hh, mm, ss, dd, mth, yy;
    struct tm when = {0};
    memset( &when, 0, sizeof(tm) );
    sscanf(sExpireDate.c_str(), "%d-%d-%d %d:%d:%d", &yy,&mth,&dd,&hh,&mm,&ss);
    
    when.tm_mday = dd;
    when.tm_mon = mth -1;
    when.tm_year = yy - 1900;
    when.tm_hour = hh;
    when.tm_min = mm;
    when.tm_sec = ss;
    
    time_t expireDate_t;
    expireDate_t = mktime(&when);
    
    time_t now = time(NULL);
    
    if (now > expireDate_t)
        bExpired = true;
    
    return bExpired;
}


bool CPIPolicyMan::isRegisteredUSB(const std::string& encryptedSerial) {
	DEBUG_LOG1("policyman - begin");
	if (0 == encryptedSerial.length()) {
		DEBUG_LOG1("policyman - stop - encrypted_serial_empty");
		return false;
	}
		
	DEBUG_LOG("policyman - serial:%s", encryptedSerial.c_str());

	if (0 == mediaList.size()) {
		DEBUG_LOG1("policyman - stop - media_list_empty");
		return false;
	}

	CPIMediaData::MAP::const_iterator itr = mediaList.find(encryptedSerial);
	if (itr != mediaList.end()) {
		DEBUG_LOG1("policyman - end - true(found)");
		return true;
	}

	DEBUG_LOG1("policyman - end - false(not_found)");
	return false;
}

bool CPIPolicyMan::getDLPUSBDrive(const std::string& mouuntPoint, const std::string& encryptedSerial, CDLPUSBDrive& dlpUsbDrive) {
	DEBUG_LOG1("policyman - begin");

	//dlpUsbDrive.setHwID("");
	//dlpUsbDrive.setUsbSerial(encryptedSerial);

	if (0 == mediaList.size()) {
		DEBUG_LOG1("policyman - skip - media_list_empty");
		return true;
	}

	CPIMediaData::MAP::const_iterator itr = mediaList.find(encryptedSerial);
	if (itr != mediaList.end()) {
		dlpUsbDrive.setMediaGuid(itr->second.guid);
		//dlpUsbDrive.setPurpose(itr->second.purpose);
		DEBUG_LOG1("policyman - end - found");
		return true;
	}

	DEBUG_LOG1("policyman - end - not_found");
	return true;
}

bool CPIPolicyMan::disableDeviceRuleByPreventPlus(CDLPPolicy::Vector* policyList) {
	if (NULL == policyList) {
		return false;
	}

	bool copyFound = false;
	bool sharedFolderFound = false;
	for (int index = 0;
			(index < policyList->size()) && (false == copyFound);
			++index)
	{
		CDLPPolicy* policy = (*policyList)[index];
		if( 
			(NULL == policy) || 
			(false == policy->use)
			)
		{
			continue;
		}

		switch(policy->dlpSubType)
		{
		case dlpsubtypeCopyPreventPlus : 
		case dlpsubtypeCopyExPreventPlus : copyFound = true; break;
		case dlpsubtypeSharedFolderPreventPlus : sharedFolderFound = true; break;
		default : break;
		}
	}

	if (
		(false == copyFound) && 
		(false == sharedFolderFound)
		)
	{
		return true;
	}

	for(int index = 0;
		index < policyList->size();
		index++)
	{
		CDLPPolicy* policy = (*policyList)[index];
		if( 
			(NULL == policy) || 
			(false == policy->use)  || 
			(dlpsubtypeMediaControl != policy->dlpSubType) 
			)
		{
			continue;
		}

		CDLPPolicyMedia* mediaControl = dynamic_cast<CDLPPolicyMedia*>(policy);
		if (NULL != mediaControl) {
			if (
				(false == mediaControl->getControl(deviceEtcStorage, etcstorageBlockRemovableRead)) &&
				(false == mediaControl->getControl(deviceEtcStorage, etcstorageBlockRemovableWrite))
				)
			{
				mediaControl->validateDeviceRule(copyFound, sharedFolderFound);
			}

			break;
		}
	}
	
	return true;
}
#endif // #ifndef _PIPOLICYMAN_CPP
