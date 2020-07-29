#ifndef	_PIPOLICYMAN_H
#define	_PIPOLICYMAN_H

#include "PIObject.h"
#include "DLPPolicy.h"

////////////////////////////////////////////////////////////////////////////////
//class CPIPolicySet 
class CPIPolicySet : public CPIObject {
	public:
		CPIPolicySet();
		virtual ~CPIPolicySet();

	public:
		bool isOnline;

	public:
		virtual void clear(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CPIPolicyMan
class CPIPolicyMan : public CPIObject {
	public:
		CPIPolicyMan();
		virtual ~CPIPolicyMan();

	protected:
		pthread_mutex_t mutexPolicy;

	public:
		CDLPPolicy::Vector onlinePolicyList;
		CDLPPolicy::Vector offlinePolicyList;
		CDLPPolicy::Vector commonPolicyList;
		CNDISClass::Vector ndisClassList; // not used
		CUSBMobileClass::Vector usbmobileClassList; // not used
		CPIPolicySet policySet;
		CNDISClass::Vector ndisList; // macos only
		CUSBMobile::Vector usbmobileList;  // macos only
		bool enableMediaControlBlockLog;
		CPIMediaData::MAP mediaList;

	protected:
		bool loadDLPPolicyXML(void);
		bool validateDLPPolicyXml(void);
		bool isNullPolicy(const std::string & guid);
		CDLPPolicy* addDLPPolicy(const int dlpType, const int dlpSubType, std::string type = "");
		void clearDLPPolicy(CDLPPolicy::Vector & vector);
		//std::string setDLPPolicyForPISupervisor(void);
		bool writePolicySet(const std::string& policySet);
		bool isEnableBlockLog(void);
		std::string getDLPPolicyForPISupervisor(void);
		bool makeInformationXml(CMarkup& markup, const int controlMode);
		void getDLPPolicyList(const int controlMode, CDLPPolicy::Vector*& policyList);
		bool getDLPPolicyForPISupervisor( CMarkup& markup, const int controlMode);
		bool makeDeviceRuleXml_copyPrevent(CMarkup& markup, CDLPPolicy::Vector* policyList, bool& copyPreventFound);
		bool makeDeviceRule(const int controlMode);
		bool makeDeviceRuleXml(CMarkup& markup, const int controlMode);
		bool makeDeviceRule(void);
		bool makeDeviceRule(CDLPPolicy::Vector* policyList);
		bool validateDeviceRule(CDLPPolicy::Vector* policyList);
		bool loadNdisXml(void);
		bool loadUSBMobileXml(void);
		void clearMediaList(void);
		bool getDLPPolicy(CDLPPolicy::Vector& policyList, int dlpType, int dlpSubType, CDLPPolicy*& policy);
		bool disableDeviceRuleByMediaControl(CDLPPolicy::Vector* policyList, const int dlpSubType, const int deviceType, const int mediaControlType);
		bool setEnableDeviceRule(CDLPPolicy* policy, const int dlpSubType);
		bool disableDeviceRuleByPreventPlus(CDLPPolicy::Vector* policyList);

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);
		bool load(void);
		bool translatePolicy(void);
		bool adjust(void);
		bool isOnline(void);
		bool getCurrentRule(CPISecRule::List& ruleList);
		bool setOnlineMode(void);
		bool setOfflineMode(void);
		CDLPPolicy* getCurrentDLPPolicy(int dlpType, int dlpSubType);
		void getLock(void);
		void releaseLock(void);
		void debugPrintUSBMobile(void);
		bool getProcessControl(bool& control, bool& log, size_t& listCount);
		bool reloadMediaList(void);
        bool isExpiredUSB(const std::string& sExpireDate);
		bool isRegisteredUSB(const std::string& encryptedSerial);
		bool getDLPUSBDrive(const std::string& mouuntPoint, const std::string& encryptedSerial, CDLPUSBDrive& dlpUsbDrive);
};
#endif // #ifndef _PIPOLICYMAN_H
