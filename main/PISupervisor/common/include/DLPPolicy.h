#ifndef	_DLPPOLICY_H
#define	_DLPPOLICY_H

#include "PIObject.h"
#include "PIXmlParser.h"
#include "include_kernel.h"
#include <vector>
#include <string>
#include "DLPApplication.h"

#include "PCSecurity.h"

////////////////////////////////////////////////////////////////////////////////
//class CDeviceV6
class CDeviceV6 {
	public:
		CDeviceV6();
		CDeviceV6(unsigned short code, std::string name);
		virtual ~CDeviceV6();

	public:
		typedef enum _EM_TYPE{
			COPYPREVENT,
			CDDVD,
			FLOPPY,
			USB,
			NETDRIVE,
			SERIAL,
			PARALLEL,
			IEEE1394,
			MTP,
			PTP,
			ANDROID,
			APPLE,
			PRINTPREVENT,
            BLUETOOTH,
            PROCESS,
            CAMERA,
			COUNT
		}EM_TYPE;

		typedef std::map<CDeviceV6::EM_TYPE, CDeviceV6> Map;

	public:
		unsigned short code;
		std::string name;

	public:
		CDeviceV6::Map map;
	
	public:
		static CDeviceV6& getInstance(void);
};

////////////////////////////////////////////////////////////////////////////////
class CUSBMobilePermit;
//class CPISecRule
class CPISecRule : CPIObject
{
	public:
		CPISecRule();
		virtual ~CPISecRule();
	
	public:
		typedef std::list<CPISecRule> List;
		typedef std::vector<ATTACHED_DEVICE> VECTOR_ATTACHED_DEVICE;

	public:
		std::string virtualType;
		bool disableAll; 
		bool disableRead;
		bool disableWrite;
		bool enableLog;
		int reserved1;
        bool blockSelectedBluetooth;
	public:
		ATTACHED_DEVICE_EX deviceEx;

    public:
        std::vector<std::string> m_vecWLanPermitList;
        bool setWLanPermitList(std::vector<std::string> vecWLanPermitLis );
        bool appendWLanPermitList(std::string strWLanSSID);
        void clearWLanPermitList();
    public:
        std::vector<MB_PERMIT> m_vecUSBMobilePermitList;
        std::vector<MB_PERMIT> getUSBMobilePermitList() { return m_vecUSBMobilePermitList; }
        bool setUSBMobilePermitList(std::vector<MB_PERMIT> vecUSBMobilePermitList );
        void clearUSBMobilePermitList();
    
        std::vector<MB_PERMIT> m_vecRemovableDrivePermitList;
        int m_nRemovableDrivePermitAllowControlType;
        std::vector<MB_PERMIT> getRemovableDrivePermitList() { return m_vecRemovableDrivePermitList; }
        bool setRemovableDrivePermitList(std::vector<MB_PERMIT> vecRemovableDrivePermitList );
        void clearRemovableDrivePermitList();
    
	protected:
		void setDeviceRule(void);

	public:
		virtual void clear(void);

	public:
		void operator=(const CPISecRule& rule);
		PATTACHED_DEVICE_EX getData(void);
		static PATTACHED_DEVICE getAttachedDevice(CPISecRule::VECTOR_ATTACHED_DEVICE& list);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyCopyExObject 
class CDLPPolicyCopyExObject : public CPIObject {
	public:
		CDLPPolicyCopyExObject() {};
		virtual ~CDLPPolicyCopyExObject() {};

	public:
		virtual bool setXml(CMarkup& markup) = 0;
		//virtual bool getXml(CMarkup& markup) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPUSBDrive
class CDLPUSBDrive : public CDLPPolicyCopyExObject {
	public:
		CDLPUSBDrive() {};
		virtual ~CDLPUSBDrive() {};

	protected:
		std::string hwID;
		std::string usbSerial;
		std::string mediaGuid;
		int purpose;
	
	private:
		virtual bool setXml(CMarkup& markup) { return true; };
		//virtual bool getXml(CMarkup& markup) { return true; };
	public:
		void setMediaGuid(const std::string& value) { mediaGuid = value; };
		std::string getMediaGuid(void) const { return mediaGuid; };
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPUSBDriveControl
class CDLPUSBDriveControl : public CDLPPolicyCopyExObject {
	public:
		CDLPUSBDriveControl();
		virtual ~CDLPUSBDriveControl();

	public:
		enum SELECT_TYPE { ALL = -1, NOT_SELECTED, SELECTED };

	private:
		int selectType;
		bool readPermit;

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		int getSelectType(void) const { return selectType; };
		bool isReadPermit(void) const { return readPermit; };
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPNotify
class CDLPNotify : public CDLPPolicyCopyExObject {
	public:
		CDLPNotify();
		virtual ~CDLPNotify();

	protected:
		int option;

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		std::string getNotify(const int value) const;
		int getNotify(const std::string& value);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPAdvancedInfo
class CDLPAdvancedInfo : public CDLPPolicyCopyExObject {
	public:
		CDLPAdvancedInfo();
		virtual ~CDLPAdvancedInfo();

	protected:
		int warningOnFileCopy;

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPMediaInfo
class CDLPMediaInfo : public CDLPPolicyCopyExObject {
	public:
		CDLPMediaInfo();
		virtual ~CDLPMediaInfo();

	protected:
		int permitType;
		std::vector<std::string> guids;

	//protected:
	//	std::string getPermitType(const int value);

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		int getPermitType(void) const { return permitType; };
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPControlCount
class CDLPControlCount : public CDLPPolicyCopyExObject {
	public:
		CDLPControlCount();
		virtual ~CDLPControlCount();

	public:
		int block;
		int allow;
		int approval;

	public:
		virtual bool setXml(CMarkup& markup) { return true; };
		int getBlock(void) const { return block; };
		int getAllow(void) const { return allow; };
		int getApproval(void) const { return approval; };
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPLogBase
class CDLPLogBase : public CDLPPolicyCopyExObject {
	public:
		CDLPLogBase();
		virtual ~CDLPLogBase();

	protected:
		bool use;
		int type;
		int condition;

	protected:
		int getType(const std::string& value);
		//std::string getType(const int value);
		int getCondition(const std::string& value);
		//std::string getCondition(const int value);
		//bool getXmlWithCondition(CMarkup& markup, const int condition);

	public:
		virtual bool setXml(CMarkup& markup) = 0;
		//virtual bool getXml(CMarkup& markup) = 0;
		void setUse(const bool use){ this->use = use; };
		bool isUse(void) const { return use; };
		void setType(const int type){ this->type = type; };
		int getType(void) const { return type; };
		int getCondition(void) const { return condition; };
		void setCondition(const int condition) { this->condition = condition; };
		void addCondition(const int flag) { condition |= flag; };
		void removeCondition(const int flag) { condition &= ~flag; };
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPFileCopy
class CDLPFileCopy : public CDLPLogBase {
	public:
		CDLPFileCopy();
		virtual ~CDLPFileCopy();

	private:
		bool maxUse;
		int maxLimit;

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		void setMaxUse(const bool maxUse) { this->maxUse = maxUse; };
		void setMaxLimit(const int maxLimit) { this->maxLimit = maxLimit; };
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPLog
class CDLPLog : public CDLPLogBase {
	public:
		CDLPLog();
		virtual ~CDLPLog();

	public:
		CDLPFileCopy fileCopy;

	public:
		virtual bool setXml(CMarkup& markup) { return true; };
		//virtual bool getXml(CMarkup& markup) { return true; };
		bool setXml(CMarkup& markup, const std::string& elem);
		//bool getXml(CMarkup& markup, const std::string& elem);
};


////////////////////////////////////////////////////////////////////////////////
//class CDLPLogs
class CDLPLogs : public CDLPPolicyCopyExObject {
	public:
		CDLPLogs();
		virtual ~CDLPLogs();

	public:
		CDLPLog block;
		CDLPLog allow;

	private:
		bool validateWithNoInspect(const CDLPControlCount& before, const CDLPControlCount& after);
		bool validateWithFound(const CDLPControlCount& before, const CDLPControlCount& after);
		bool validateWithNotFound(const CDLPControlCount& before, const CDLPControlCount& after);

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		bool hasLogEnable(void) const;
		bool validate(const int inspectOption,
				const CDLPControlCount& foundBefore,
				const CDLPControlCount& notFoundBefore,
				const CDLPControlCount& noInspectBefore,
				const CDLPControlCount& foundAfter,
				const CDLPControlCount& notFoundAfter,
				const CDLPControlCount& noInspectAfter);
		bool isSaveLogAny(void);
};


////////////////////////////////////////////////////////////////////////////////
//class CDLPControl
class CDLPControl : public CDLPPolicyCopyExObject {
	public:
		CDLPControl();
		virtual ~CDLPControl();
	
	public:
		typedef std::vector<CDLPControl> VECTOR;

	public:
		int inspectOption;
		int targetFile;
		int noInspect;
		int found;
		int notFound;
		int purpose;

	//public:
		//static const int DEFAULT_PURPOSE;

	private:
		bool getDLPControlCount(const int controlType, CDLPControlCount& controlCount);

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		bool hasBlockOrApproval() const;
		//std::string getInspectOption(const int value) const;
		int getInspectOption(void) const { return inspectOption; };
		int getInspectOption(const std::string& value) const;
		int getTargetFile(const std::string& value) const;
		void setControlType(const std::string& inspect, const std::string& type);
		bool changeControlApprovalToBlock(void);
		//std::string getControlType(const int value) const;
		bool getDLPControlCount(CDLPControlCount& found, CDLPControlCount& notFound, CDLPControlCount& noInspect);
		bool isBlockReadAll(void);
		bool isSaveLogAny(void);
};



////////////////////////////////////////////////////////////////////////////////
//class CDLPCopyControl
class CDLPCopyControl : public CDLPPolicyCopyExObject {
	public:
		CDLPCopyControl();
		virtual ~CDLPCopyControl();

	public:
		typedef std::vector<CDLPCopyControl> VECTOR;

	public:
		CDLPUSBDriveControl usbDriveControl;
		CDLPControl::VECTOR controls;
		CDLPLogs logs;

	protected:
		bool getDLPControlCount(CDLPControlCount& found, CDLPControlCount& notFound, CDLPControlCount& noInspect);

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		bool hasBlockOrApproval(void) const;
		bool isBlockReadAll(void);
		bool isSaveLogAny(void);
		bool changeControlApprovalToBlock(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPCopyControls
class CDLPCopyControls : public CDLPPolicyCopyExObject {
	public:
		CDLPCopyControls();
		virtual ~CDLPCopyControls();

	private:
		CDLPCopyControl::VECTOR copyControls;

	public:
		virtual bool setXml(CMarkup& markup);
		//virtual bool getXml(CMarkup& markup);
		bool hasBlockOrApproval() const;
		bool isBlockReadAll(void);
		bool isSaveLogAny(void);
		bool changeControlApprovalToBlock(void);
		bool getDLPCopyControl(const CDLPUSBDrive* usbDrive, CDLPCopyControl& copyControl);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicy 
class CDLPPolicy : public CPIObject {
	public:
		CDLPPolicy();
		virtual ~CDLPPolicy();

	public:
		typedef std::vector<CDLPPolicy*> Vector;

	public:
		CPIXmlParser xmlParser;

	public:
		nsPISecObject::EM_DLPTYPE dlpType;
		nsPISecObject::EM_DLPSUBTYPE dlpSubType;
		std::string guid;
		std::string name;
		bool use;
		std::string datetime;
		int revision;
		nsPISecObject::EM_DLPPOLICY_CONTROLMODE dlpControlMode;
		std::string virtualType;
		bool enableDeviceRule;

	public:
		CPISecRule rule;

	protected:
		void addDeviceRule( std::string virtualType,
				bool disableAll, bool disableRead, bool disableWrite,
				bool enableLog, bool disbleSmartInspect = false, bool blockSelectedBluetooth = false, int reserved1 = -1 );

	public:
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool addDeviceRule(const bool blockWriteAll);
		virtual bool addDeviceRule(CMarkup& markup);
		virtual bool makeInformationXml(CMarkup& markup);
		virtual bool isEnableBlockLog(void);
		void setEnableDeviceRule(const bool enableDeviceRule);
		bool isEnableDeviceRule(void) { return enableDeviceRule; };
};

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusLogParent
class CPreventPlusLogParent : public CPIObject {
	public:
		CPreventPlusLogParent();
		virtual ~CPreventPlusLogParent();
	
	public:
		bool use;
		bool isConditional;
		int condition;

	public:	
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup) = 0;

	public:
		void setType(std::string type);
		void setUse(std::string use);
		void addCondition(std::string use);
};

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusSaveFileCopy
class CPreventPlusSaveFileCopy: public CPreventPlusLogParent {
	public:
		CPreventPlusSaveFileCopy();
		virtual ~CPreventPlusSaveFileCopy();

	public:
		int maxLimit;
		bool maxLimitUse;

	public:	
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);

	public:
		void setMaxLimitUse(std::string use);
};

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusLogParent 
class CPreventPlusLog : public CPreventPlusLogParent {
	public:
		CPreventPlusLog();
		virtual ~CPreventPlusLog();
	
	public:
		CPreventPlusSaveFileCopy saveFileCopy;

	public:	
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
};

////////////////////////////////////////////////////////////////////////////////
//class CPreventPlusControls
class CPreventPlusControls : public CPIObject {
	public:
		CPreventPlusControls();
		virtual ~CPreventPlusControls();

	public:
		nsPISecObject::EM_INSPECTOPTION type;
		nsPISecObject::EM_CONTROL_TARGETFILE targetFile;
		nsPISecObject::EM_CONTROL noInspect;
		nsPISecObject::EM_CONTROL found;
		nsPISecObject::EM_CONTROL notFound;

	protected:
		void setControl(std::string type, nsPISecObject::EM_CONTROL & control);

	public:	
		virtual void clear(void);

	public:
		void setType(std::string type);
		void setTargetFile(std::string targetFile);
		void setControl(std::string type, std::string inspect);
		bool parsePolicyInfo(CMarkup& markup);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyPreventPlus 
class CDLPPolicyPreventPlus : public CDLPPolicy {
	public:
		CDLPPolicyPreventPlus();
		virtual ~CDLPPolicyPreventPlus();

	public:
		CPreventPlusControls controls;
		CPreventPlusLog blockLog;
		CPreventPlusLog allowLog;
		nsPISecObject::EM_NOTIFY notify;
	
	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool addDeviceRule(const bool blockWriteAll);
		virtual bool isEnableBlockLog(void);

	public:
		void setNotify(std::string notify);
		virtual bool changeControlApprovalToBlock(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyCopy
class CDLPPolicyCopy: public CDLPPolicyPreventPlus {
	public:
		CDLPPolicyCopy();
		virtual ~CDLPPolicyCopy();

	public:
		typedef std::vector<std::string> VectorMediaGuid;

	protected:
		nsPISecObject::EM_MEDIA_PERMIT mediaPermitType;

	public:	
		bool warningOnFileCopy;
		CDLPPolicyCopy::VectorMediaGuid mediaGuidList;

	public:
		void setWarningOnFileCopy(std::string warningOnFileCopy);
		void setMediaPermitType(std::string mediaPermitType);

	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool addDeviceRule(const bool blockWriteAll);
		virtual bool isBlockReadAll(void);
		virtual int getMediaPermitType(void) const { return mediaPermitType; };
		virtual bool getDLPCopyControl(const CDLPUSBDrive* usbDrive, CDLPCopyControl& copyControl);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyCopyEx
class CDLPPolicyCopyEx: public CDLPPolicyCopy {
	public:
		CDLPPolicyCopyEx();
		virtual ~CDLPPolicyCopyEx();
	
	protected:
		CDLPCopyControls dlpCopyControls;
		CDLPNotify notify;
		CDLPAdvancedInfo advancedInfo;
		CDLPMediaInfo media;

	private:
		bool isSaveLogAny(void);

	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool addDeviceRule(const bool blockWriteAll);
		virtual bool changeControlApprovalToBlock(void);
		virtual bool isBlockReadAll(void);
		virtual int getMediaPermitType(void) const { return media.getPermitType(); };
		virtual bool getDLPCopyControl(const CDLPUSBDrive* usbDrive, CDLPCopyControl& copyControl);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyUpload
class CDLPPolicyUpload: public CDLPPolicyPreventPlus {
	public:
		CDLPPolicyUpload();
		virtual ~CDLPPolicyUpload();
    
        std::string m_sControlList;
	public:
		virtual bool initialize(void);
    
        void LoadControlList(std::string & sControlList);
//        virtual void clear(void);
//        virtual bool parsePolicyInfo(CMarkup& markup);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyPrint
class CDLPPolicyPrint: public CDLPPolicyPreventPlus {
	public:
		CDLPPolicyPrint();
		virtual ~CDLPPolicyPrint();
	
	public:
		bool warningOnPrint;
		bool showSerialNumber;
		bool saveCopyFoundPage;
		bool patternMaskingUse;
		bool useWatermark;
		nsPISecObject::EM_WATERMARK_FILE watermarkFile;
		bool differentWatermark;

	protected:
		void setPrintWatermarkFileOptionAsString(std::string watermarkFile);

	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyClipboard
class CDLPPolicyClipboard: public CDLPPolicyPreventPlus {
	public:
		CDLPPolicyClipboard();
		virtual ~CDLPPolicyClipboard();

	public:
		CDLPApplication::Vector applicationList;
	
	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool makeInformationXml(CMarkup& markup);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyApplication
class CDLPPolicyApplication: public CDLPPolicyPreventPlus {
	public:
		CDLPPolicyApplication();
		virtual ~CDLPPolicyApplication();

	public:
		bool execControlBlockLog;
		CDLPApplication::Vector applicationList;
		int controlCount;
		int controlWindowsExecutableCount;
		int preventCount;
		int preventWindowsExecutableCount;

	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool addDeviceRule(const bool blockWriteAll);
		virtual bool makeInformationXml(CMarkup& markup);
		virtual bool isEnableBlockLog(void);
	
	public:
		void setExecControlBlockLog(std::string execControlBlockLog);
		bool isControlApplicationNonWindowsExecutableEmpty(void);
		bool isControlApplicationEmpty(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyControl 
class CDLPPolicyControl : public CDLPPolicy {
	public:
		CDLPPolicyControl();
		virtual ~CDLPPolicyControl();
	public:
		virtual bool initialize(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyMedia 
class CDLPPolicyMedia : public CDLPPolicyControl
{
	public:
		CDLPPolicyMedia();
		virtual ~CDLPPolicyMedia();
		
	public:
		int controlEtcStorage;
		int controlCommDevice;
		bool blockLog;
        std::vector<std::string> m_vecWlanPermitList;
        std::vector<MB_PERMIT>  m_vecUSBMobilePermitList;
        std::vector<MB_PERMIT> m_vecRemovableDrivePermitList;
        int m_nRemovableDrivePermitAllowControlType;
    
	public:
		CPISecRule::List ruleList;
        bool m_bBlockSelectedBluetooth;
    
	public:
		virtual bool initialize(void);
		virtual void clear(void);
		virtual bool parsePolicyInfo(CMarkup& markup);
		virtual bool addDeviceRule(const bool blockWriteAll);
		virtual bool addDeviceRule(CMarkup& markup);
		virtual bool isEnableBlockLog(void);
		void addDeviceRule(const int block, std::string virtualType);

	public:
		void setBlockLog(std::string blockLog);
		bool validateDeviceRule(const bool copy, const bool sharedFolder);
        bool setWLanPermitList( std::vector<std::string> vecWLanPermitList );
    
        bool setUSBMobilePermitList( std::vector<MB_PERMIT> vecUSBMobilePermitList );
        void clrUSBMobilePermitList();
    
        bool setRemovableDrivePermitList( std::vector<MB_PERMIT> vecRemovableDrivePermitList );
        void clrRemovableDrivePermitList();

		bool getControl(const nsPISecObject::EM_DEVICETYPE deviceType, const int control) const;
    
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyScreenCapture 
class CDLPPolicyScreenCapture : public CDLPPolicyControl {
	public:
		CDLPPolicyScreenCapture();
		virtual ~CDLPPolicyScreenCapture();
	public:
		virtual bool initialize(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicyPCSecurity
class CDLPPolicyPCSecurity : public CDLPPolicyControl
{
public:
    CDLPPolicyPCSecurity();
    virtual ~CDLPPolicyPCSecurity();
    
public:
    CShareFolder m_ShareFolder;
    
public:
    virtual void Copy( const CDLPPolicy* pDLPPolicy );
    virtual bool initialize(void);
    virtual bool makePolicyInfo(CMarkup& cXml);
    virtual bool parsePolicyInfo(CMarkup& cMarkup);
    virtual bool AddDeviceRule(const bool bBlockWriteAll = false );
    virtual void clear(void);
    
    
};

////////////////////////////////////////////////////////////////////////////////
//class CDLPPolicySharedFolderCopy 
class CDLPPolicySharedFolderCopy : public CDLPPolicyPreventPlus {
	public:
		CDLPPolicySharedFolderCopy();
		virtual ~CDLPPolicySharedFolderCopy();
	public:
		virtual bool initialize(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CPIControlItem 
class CPIControlItem : public CPIObject {
	public:
		CPIControlItem();
		virtual ~CPIControlItem();

	public:
		std::string keyword;
		std::string name;

	public:	
		virtual void clear(void);
};


////////////////////////////////////////////////////////////////////////////////
//class CNDISClass
class CNDISClass : public CPIControlItem {
	public:
		CNDISClass();
		virtual ~CNDISClass();

	public:
		typedef std::vector<CNDISClass> Vector;
};

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobileClass
class CUSBMobileClass : public CPIControlItem {
	public:
		CUSBMobileClass();
		virtual ~CUSBMobileClass();

	public:
		typedef std::vector<CUSBMobileClass> Vector;
};

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobile
class CUSBMobile{
	public:
		CUSBMobile();
		virtual ~CUSBMobile();

	public:
		typedef std::vector<CUSBMobile> Vector;

	public:
		std::string type;
		std::string basePath;
		std::string keyword;
};

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobilePermit
class CUSBMobilePermit
{
public:
    CUSBMobilePermit();
    ~CUSBMobilePermit();
public:
    typedef std::vector<CUSBMobilePermit> Vector;
public:
    std::string m_strProduct;
    std::string m_strMediaID;
};


////////////////////////////////////////////////////////////////////////////////
//class CPIMediaData 
class CPIMediaData : public CPIControlItem {
	public:
		CPIMediaData();
		virtual ~CPIMediaData();

	public:
		typedef std::map<std::string, CPIMediaData> MAP;

	public:
		std::string serialNumber;
		std::string expireDate;
		int purpose;
		int type;
		std::string guid;

	public:
		virtual void clear(void);
};
#endif // #ifndef _DLPPOLICY_H
