#ifndef	_PICONFIGMAN_H
#define	_PICONFIGMAN_H

#include "PIObject.h"
#include <set>

////////////////////////////////////////////////////////////////////////////////
//CPISocketSupport 
class CPISocketSupport : public CPIObject {
	public:
		CPISocketSupport(void);
		virtual ~CPISocketSupport(void);

	public:
		bool unsignedType;
		bool bigEndian;
		bool savePacket;
		std::string savePacketPath;

	public:
		virtual void clear(void);
		void setUnsignedType(std::string unsignedType);
		void setBigEndian(std::string bigEndian);
		void setSavePacket(std::string savePacket);
};

////////////////////////////////////////////////////////////////////////////////
//CPIConfigPath
class CPIConfigPath : public CPIObject {
	public:
		CPIConfigPath();
		virtual ~CPIConfigPath();

	public:
		std::string basePath;
		std::string binPath;
		std::string configPath;
		std::string logPath;
		std::string serveriPath;
		std::string dlpLogPath;
		std::string dlpConfigPath;
		std::string wallImagePath;

		std::string dlpPolicyFile;
		std::string policySetFile;
		std::string configFile;
		std::string ndisFile;
		std::string usbmobileFile;
		std::string deviceLogFile;
		std::string wallFile;
		std::string processControlFile;
		std::string mediaSerialFile;

	public:
		virtual bool initialize(void);
		virtual void clear(void);
	
	public:
		std::string getDLPPolicyFile(void);
		std::string getWallFile(void);
		std::string getPolicySetFile(void);
		std::string getConfigFile(void);
		std::string getNdisFile(void);
		std::string getUSBMobileFile(void);
		std::string getDeviceLogFile(void);
		std::string getProcessControlFile(void);
		bool loadDirectoryInfo(void);
		std::string getMediaSerialFile(void);
};

////////////////////////////////////////////////////////////////////////////////
//CPIConfigLog
class CPIConfigLog : public CPIObject {
	public:
		CPIConfigLog();
		virtual ~CPIConfigLog();

	public:
		int depth;
		bool info;
		bool debug;
		bool error;
		bool useTickCount;

	public:
		virtual void clear(void);
	
	public:
		void setDepth(std::string depth);
		void setUseTickCount(std::string tickCount);
		bool applyDepth(void);
};

////////////////////////////////////////////////////////////////////////////////
//CPIConfigWall
class CPIConfigWall : public CPIObject {
	public:
		CPIConfigWall();
		virtual ~CPIConfigWall();

	public:
		const std::string imageName;
		bool usePWDisk;
		bool useSparse;
		int diskSizeGb;

	public:
		virtual bool initialize(void);
		virtual void clear(void);

	public:
		bool loadConfig(std::string configFile);
};

////////////////////////////////////////////////////////////////////////////////
//CPIConfigDLPPolicy
class CPIConfigDLPPolicy : public CPIObject {
	public:
		CPIConfigDLPPolicy();
		virtual ~CPIConfigDLPPolicy();

	private:
		////////////////////////////////////////////////////////////////////////////////
		//CApplicationControl
		class CApplicationControl : public CPIObject {
			public:
				CApplicationControl();
				virtual ~CApplicationControl();

			private:
				bool checkBinary;
				bool checkFilePath;
				bool skipWindowsExecutable;
				std::set<std::string> allowedFileList;
				std::set<std::string> allowedProcessList;

			private:
				bool loadDefault(void);

			public:
				virtual bool initialize(void);
				virtual void clear(void);

			public:
				void setCheckBinary(std::string value);
				bool isCheckBinary(void) const;
				void setCheckFilePath(std::string value);
				bool isCheckFilePath(void) const;
				void setSkipWindowsExecutable(std::string value);
				bool isSkipWindowsExecutable(void) const;
				bool set(void* pObject);
				std::set<std::string>* getAllowedFileList(void) { return &allowedFileList; };
				std::set<std::string>* getAllowedProcessList(void) { return &allowedProcessList; };
		};


	private:
		std::map<int, bool> enableList;

	public:
		CApplicationControl applicationControl;

	public:
		virtual bool initialize(void);
		virtual void clear(void);

	public:
		void set(int dlpSubType, std::string enable);
		bool isEnableWithSubType(int dlpSubType);
};


////////////////////////////////////////////////////////////////////////////////
//CPIConfigMan
class CPIConfigMan : public CPIObject {
	public:
		CPIConfigMan();
		virtual ~CPIConfigMan();
		
	public:
		CPIConfigPath path;
		CPIConfigLog log;
		CPISocketSupport socket;
		CPIConfigWall wall;
		CPIConfigDLPPolicy dlpPolicy;

	public:
		int ttlCycle;
		bool agentAutoRestartUse;
		bool fileMonitorAutoRestartUse;
		std::string quarantinePath;
		bool copyPreventBlockWriteAll;
		bool blockHidUse;
		bool blockHidCCSpec;
		bool cdsuAutoRestartUse;
		bool driveManagementUse;
		bool driveDefaultBlockUse;
		bool unRegisterUSBReadBlockUse;
		std::vector<std::string> allowProcessList;
		std::vector<std::string> allowFolderList;
		std::vector<std::string> allowFileExtList;
		std::vector<std::string> allowUSBIDList;
		std::vector<std::string> blockDeviceIDList;
		std::vector<std::string> accessBlockRegistryList;
		bool qtLimitUse;
        bool m_bAirDropProtectUse;
    
        // FullDiskAccess
        bool m_bFDAEnable;
        int  m_nFDACheckInterval;
    
    public:
        bool getAirDropProtectUse(void) { return m_bAirDropProtectUse; }
    
	protected:
        void setAirDropProtectUse(std::string qtAirDropProtectUse);
		void setTTLCycle(std::string ttlCycle);
		void setAgentAutoRestartUse(std::string agentAutoRestartUse);
		void setFileMonitorAutoRestartUse(std::string fileMonitorAutoRestartUse);
		void setQuarantinePath(std::string quarantinePath);
		void setCopyPreventBlockWriteAll(std::string copyPreventBlockWriteAll);
		void setBlockHidUse(std::string blockHidUse);
		void setBlockHidCCSpec(std::string blockHidCCSpec);
		void setCdsuAutoRestartUse(std::string cdsuAutoRestartUse);
		void setDriveManagementUse(std::string driveManagementUse);
		void setDriveDefaultBlockUse(std::string DriveDefaultBlockUse);
		void setUnRegisterUSBReadBlockUse(std::string UnRegisterUSBReadBlockUse);
		void setQtLimitUse(std::string qtLimitUse);
		bool loadConfig(void);
		bool loadExceptionDeviceList(void);
	
	public:
		virtual bool initialize(void);
		virtual void clear(void);
		
	public:
		bool load(void);
		bool save(void);
		bool isBlockWriteAll(void);
};
#endif // #ifndef _PICONFIGMAN_H
