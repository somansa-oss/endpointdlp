#ifndef	_PIDEVICEMAN_H
#define	_PIDEVICEMAN_H

#include "PIObject.h"
#include "DLPPolicy.h"
#include "PISecSmartDrvStub.h"
#include "PIDeviceLog.h"
#include "PIApplicationControl.h"
#include "PCSecurity.h"
#include "PIUSBDrive.h"

#ifndef LINUX
#include "PIFullDiskAccess.h"
#endif

typedef struct _EXIST_POLICY
{
    bool bExistWLan;
    bool bExistWwan;
    bool bExistBth;
    bool bExistCamera;
    bool bExistCDPolicy;
    bool bExistAirDrop;
    bool bExistFileSharing;
    bool bExistRemoteManagement;
    
} EXIST_POLICY, *PEXIST_POLICY;


class CPIDeviceMan : public CPIObject
{
	public:
		CPIDeviceMan();
		virtual ~CPIDeviceMan();

	protected:
        bool m_bProtect;
		CPIDeviceLog::VECTOR deviceLogCache;
		CPIApplicationControl applicationControl;
		//pthread_mutex_t mutexProcessCheck;
		std::string lastVolumes;

	public:
		CPISecSmartDrvStub smartDrvStub;
        CPCSecurity m_cPCSecurity;
#ifndef LINUX		
        CPIFullDiskAccess m_cFDA;
#endif

	protected:
        int  GetDriveRuleCount(CPISecRule::List& ruleList);
        bool SetClearPolicyDrive(CPISecRule::List& ruleList);
		bool applyPolicyDrive(CPISecRule::List& ruleList);
		bool applyExceptPolicyDrive();
		bool applyExceptPolicyDriveAllow(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorAllow);
		bool applyExceptPolicyDriveBlock(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorBlock);
		bool isDeviceRule(CPISecRule& rule);
		bool isDriveRule(CPISecRule& rule);
		bool applyPolicyUSBMobile(CPISecRule::List& ruleList);
        bool applyPolicySelectedBluetooth(CPISecRule::List& ruleList);
        bool applyPolicyCamera( CPISecRule::List& ruleList );
        bool applyPolicyRndis( CPISecRule::List& ruleList );
        bool applyPolicyAirDrop( CPISecRule::List& ruleList );
		bool setAllowedProcess(void);
		bool setAllowedFolder(void);
		bool setAllowedFolderName(void);
		bool setAllowedFileExtName(void);
		bool setQuarantineFilePath(void);
		bool setQuarantineLimit(void);
		int getMaxTimeSpan(void) const;
		bool isSharedFolder(const unsigned int busType) const;
		unsigned int getDeviceCategory(const bool sharedFolder) const;
		bool applyPolicyPrintPrevent(void);
		bool applyPolicyUploadPrevent(void);
		bool applyPolicyProcess(void);
        // PC Security
		bool applyPolicyPCSecurity(void);
		bool applyExceptPolicyUSBReadBlockByUnregister(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorBlock);
		bool applyExceptPolicyUSBReadBlockByCopyPrevent(CPIUSBDrive& usbDrive, CPISecRule::VECTOR_ATTACHED_DEVICE& vectorBlock);
		bool isMediaControlLog(int storDeviceType);
		bool adjustSmartLog(void*& param);
    
	public:
        bool setSelfProtect(void);
        bool setSelfProtectOnlyPermit(void);
		virtual bool initialize(void);
		virtual bool finalize(void);
		bool applyPolicy(void);
		int smartdrvFileIsRemove(void* param);
		int smartdrvFileScan(void* param);
		int smartdrvFileDelete(void* param);
		int smartdrvFileRename(void* param);
		int smartdrvFileExchangeData(void* param);
		int smartdrvFileEventDiskFull(void* param);
		int smartdrvSmartLogNotify(void* param);
		int smartdrvProcessCallback(void* param);
		int smartdrvProcessAccessCheck(void* param);
		bool isMediaControlUSBReadAllowWriteBlock(void);
		void test(void);
		bool isDuplicatedDeviceLog(const CPIDeviceLog& deviceLog);
        bool controlDevice(void);
        bool MediaPolicyExistSet(CPISecRule& rule, EXIST_POLICY& Policy);
        bool MediaPolicyExistPost(EXIST_POLICY& Policy);
		bool getMachedNetworkService(std::string name, std::vector<std::string>& vectorNetworkService);
		bool addProcessBlockLog(CPIApplicationControl::CResult& application);
		std::string getTicket(int pid);
		static int smartdrvEventHandler(void* param);
		bool reloadUSBSerial(void);
		bool monitorVolums(void);
    
    public:
        bool RequestProcessTerminate( PSCANNER_NOTIFICATION pNotify );
    
};

#endif // #ifndef _PIDEVICEMAN_H
