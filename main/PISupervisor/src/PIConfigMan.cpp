#ifndef	_PICONFIGMAN_CPP
#define	_PICONFIGMAN_CPP

#include <map>

#include "PIConfigMan.h"
#include "PIXmlParser.h"


////////////////////////////////////////////////////////////////////////////////
//CPISocketSupport 
CPISocketSupport::CPISocketSupport(void) {
	clear();
}

CPISocketSupport::~CPISocketSupport(void) {
}

void CPISocketSupport::clear(void) {
	unsignedType = true;
	bigEndian = false;
	savePacket = false;
	savePacketPath = "";
}

void CPISocketSupport::setUnsignedType(std::string unsignedType) {
	this->unsignedType = util.getValueAsBool(unsignedType);
}

void CPISocketSupport::setBigEndian(std::string bigEndian) {
	this->bigEndian = util.getValueAsBool(bigEndian);
}

void CPISocketSupport::setSavePacket(std::string savePacket) {
	this->savePacket = util.getValueAsBool(savePacket);
}


////////////////////////////////////////
// CPIConfigPath
CPIConfigPath::CPIConfigPath() {
	clear();
}

CPIConfigPath::~CPIConfigPath() {	
}

void CPIConfigPath::clear(void) {
	basePath = "";
	binPath = "";
	configPath = "";
	logPath = "";
	serveriPath = "";
	dlpConfigPath = "";
	dlpLogPath = "";
	wallImagePath = "";
	
	dlpPolicyFile = "";	
	policySetFile = "";
	configFile = "";	
	ndisFile = "";	
	usbmobileFile = "";	
	deviceLogFile = "";	
	wallFile = "";	
	processControlFile = "";
	mediaSerialFile = "";
}

bool CPIConfigPath::initialize(void) {

	binPath = "/usr/local/Privacy-i";
	configPath = "/usr/local/Privacy-i/discover/ProgramData/config";
	logPath = "/usr/local/Privacy-i/discover/ProgramData/log";
	serveriPath = "/usr/local/Privacy-i/discover";

	if( false == loadDirectoryInfo() ){
		printf("[warining] directory_info load error\n");
	}

	dlpConfigPath = configPath + "/security";
	dlpLogPath = logPath + "/pisec"; 
	wallImagePath = serveriPath + "/ProgramData/Data";

	char path[1024];
    memset(path,0x00,1024);
	sprintf( path, "%s/pb_dlp_info.xml", configPath.c_str());
	dlpPolicyFile = path;

    memset(path,0x00,1024);
	sprintf( path, "%s/cm_pwinterface.xml", configPath.c_str());
	wallFile = path;

    memset(path,0x00,1024);
	sprintf( path, "%s/pisec_policyset.xml", dlpConfigPath.c_str());
	policySetFile = path;	
	
    memset(path,0x00,1024);
	sprintf( path, "%s/pisec_supervisor.xml", dlpConfigPath.c_str());
	configFile = path;	
	
    memset(path,0x00,1024);
	sprintf( path, "%s/pisec_ndis_control.xml", dlpConfigPath.c_str());
	ndisFile = path;	
	
    memset(path,0x00,1024);
	sprintf( path, "%s/pisec_usbmobile_control.xml", dlpConfigPath.c_str());
	usbmobileFile = path;	
	
    memset(path,0x00,1024);
	sprintf( path, "%s/pisecmsg.pidb", logPath.c_str());
	deviceLogFile = path;	
	
    memset(path,0x00,1024);
	sprintf( path, "%s/pisec_process_control.xml", dlpConfigPath.c_str());
	processControlFile = path;	

	memset(path,0x00,1024);
	sprintf( path, "%s/pb_pisec_media_serial.xml", configPath.c_str());
	mediaSerialFile = path;
	
	return true;
}

std::string CPIConfigPath::getDLPPolicyFile(void) {
	return dlpPolicyFile; 
}

std::string CPIConfigPath::getWallFile(void) {
	return wallFile; 
}

std::string CPIConfigPath::getPolicySetFile(void) {
	return policySetFile; 
}

std::string CPIConfigPath::getConfigFile(void) {
	return configFile; 
}

std::string CPIConfigPath::getNdisFile(void) {
	return ndisFile; 
}

std::string CPIConfigPath::getUSBMobileFile(void) {
	return usbmobileFile; 
}
		
std::string CPIConfigPath::getDeviceLogFile(void) {
	return deviceLogFile; 
}

std::string CPIConfigPath::getProcessControlFile(void) {
	return processControlFile; 
}

bool CPIConfigPath::loadDirectoryInfo(void) {

	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;

	// ----------
	printf("before set directory_file\n");
	// ----------

	std::string directoryFile = "directory_info.xml";
	if( false == basePath.empty() ) {
		directoryFile = basePath + "/directory_info.xml";
	}

	// ----------
	printf("base_file:%s\n", basePath.c_str());
	printf("directory_file:%s\n", directoryFile.c_str());
	// ----------
	
	if( false == markup.Load(directoryFile) ) {
		return false;
	}

	markup.FindElem("directory");
	markup.IntoElem();

	if( true == markup.FindElem("serveri") ){
		serveriPath = markup.GetData();
	}
	
	markup.ResetMainPos();	
	if( true == markup.FindElem("bin")) {
		binPath = markup.GetData();
	}

	markup.ResetMainPos();	
	if( true == markup.FindElem("config") ) {
		configPath = markup.GetData();
	}
	
	markup.ResetMainPos();	
	if( true == markup.FindElem("log") ) {
		logPath = markup.GetData();
	}
	
	markup.OutOfElem();
	
	return true;
}

std::string CPIConfigPath::getMediaSerialFile(void) {
	return mediaSerialFile; 
}

////////////////////////////////////////
// CPIConfigLog
CPIConfigLog::CPIConfigLog() {
	clear();
}

CPIConfigLog::~CPIConfigLog() {	
}

void CPIConfigLog::clear(void) {
	depth = 0;
	info = false;
	debug = false;
	error = true;
	useTickCount = false;
}

void CPIConfigLog::setDepth(std::string depth) {
	this->depth = atoi(depth.c_str());
}

bool CPIConfigLog::applyDepth(void) {
			
	info = false;
	debug = false;
	error = true;

	switch(depth) {
		case 0 : break;
		case 1 : info = true; break;
		case 2 : 
		default : info = true; debug = true; break;
	}

	return true;
}
		
void CPIConfigLog::setUseTickCount(std::string tickCount) {
	useTickCount = util.getValueAsBool(tickCount);
}

////////////////////////////////////////
// CPIConfigWall
CPIConfigWall::CPIConfigWall() : imageName("SmsDisk.dmg") {
	clear();
}

CPIConfigWall::~CPIConfigWall() {	
}

bool CPIConfigWall::initialize(void) {
	CPIObject::initialize();
	
	return true;
}

void CPIConfigWall::clear(void) {
	usePWDisk = false;
	useSparse = false;
	diskSizeGb = 1;
}

bool CPIConfigWall::loadConfig(std::string configFile) {
	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;

	bool result = false;
	do {
		if( false == markup.Load(configFile) ) {
			break;
		}

		if( false == markup.FindElem("PrivacyWall_Interface") ) {
			break;
		}

		if( false == markup.IntoElem() ) {
			break;
		}

		if( true == markup.FindElem("PWManager") )
        {
			if( true == markup.IntoElem() ) {
				markup.ResetMainPos();
				if( true == markup.FindElem("PWDISK") )
                {
					usePWDisk = ("1" == markup.GetAttrib("Enable"))?true:false;
				}
				markup.OutOfElem();
			}
		}

		if( true == markup.FindElem("PWDisk") ) {
			if( true == markup.IntoElem() ) {
				markup.ResetMainPos();
				if( true == markup.FindElem("Sparse") ) {
					useSparse = ("1" == markup.GetAttrib("Enable"))?true:false;
					diskSizeGb = atoi(markup.GetAttrib("DiskSize").c_str());
				}
				markup.OutOfElem();
			}
		}

		result = true;
	} while(false);

	return result;
}

////////////////////////////////////////
// CPIConfigDLPPolicy
CPIConfigDLPPolicy::CPIConfigDLPPolicy() {
	clear();
}

CPIConfigDLPPolicy::~CPIConfigDLPPolicy() {	
}

bool CPIConfigDLPPolicy::initialize(void) {
	CPIObject::initialize();

	applicationControl.initialize();
	
	return true;
}

void CPIConfigDLPPolicy::clear(void) {
}

void CPIConfigDLPPolicy::set(int dlpSubType, std::string enable) {
	this->enableList[dlpSubType] = util.getValueAsBool(enable);
}

bool CPIConfigDLPPolicy::isEnableWithSubType(int dlpSubType) {
	bool result = true;
	if (0 == enableList.size()) {
		return true;
	}

	std::map<int, bool>::iterator itr = enableList.find(dlpSubType);
	if (itr != enableList.end()) {
		result = itr->second;
	}

	return result;
}

////////////////////////////////////////
// CPIConfigDLPPolicy::CApplicationControl
CPIConfigDLPPolicy::CApplicationControl::CApplicationControl() {
	clear();
}

CPIConfigDLPPolicy::CApplicationControl::~CApplicationControl() {
}

bool CPIConfigDLPPolicy::CApplicationControl::initialize(void) {
	CPIObject::initialize();

	//loadDefault();

	return true;
}

void CPIConfigDLPPolicy::CApplicationControl::clear(void) {
	checkBinary = false;
	checkFilePath = true;
	skipWindowsExecutable = true;
	allowedFileList.clear();
	allowedProcessList.clear();
}

void CPIConfigDLPPolicy::CApplicationControl::setCheckFilePath(std::string value) {
	if (0 == value.length()) {
		return;
	}
	
	checkFilePath = util.getValueAsBool(value);
}

bool CPIConfigDLPPolicy::CApplicationControl::isCheckFilePath(void) const {
	return checkFilePath;
}

void CPIConfigDLPPolicy::CApplicationControl::setCheckBinary(std::string value) {
	if (0 == value.length()) {
		return;
	}
	
	checkBinary = util.getValueAsBool(value);
}

bool CPIConfigDLPPolicy::CApplicationControl::isCheckBinary(void) const {
	return checkBinary;
}

void CPIConfigDLPPolicy::CApplicationControl::setSkipWindowsExecutable(std::string value) {
	if (0 == value.length()) {
		return;
	}
	
	skipWindowsExecutable = util.getValueAsBool(value);
}

bool CPIConfigDLPPolicy::CApplicationControl::isSkipWindowsExecutable(void) const {
	return skipWindowsExecutable;
}

bool CPIConfigDLPPolicy::CApplicationControl::set(void* pObject) {
	CMarkup* markup = reinterpret_cast<CMarkup*>(pObject);
	//allowedFileList.clear();
	//allowedProcessList.clear();

	setCheckBinary(markup->GetAttrib("check_binary"));
	setCheckFilePath(markup->GetAttrib("check_filepath"));
	setSkipWindowsExecutable(markup->GetAttrib("skip_windows_exe"));

	if (true == markup->IntoElem()) {
		if (true == markup->FindElem("allow_list") ) {
			if (true == markup->IntoElem()) {
				if (true == markup->FindElem("file_list") ) {
					while (true == markup->FindChildElem("file") ) {
						allowedFileList.insert(markup->GetChildData());
					}
				}
				
				if (true == markup->FindElem("process_list") ) {
					while (true == markup->FindChildElem("process") ) {
						allowedProcessList.insert(markup->GetChildData());
					}
				}
				markup->OutOfElem();
			}
		}
		markup->OutOfElem();
	}

	return true;
}

bool CPIConfigDLPPolicy::CApplicationControl::loadDefault(void) {
	allowedFileList.insert("/Desktop/.DS_Store");
	allowedFileList.insert("/Library/Application");
	allowedFileList.insert("/Library/Keychains");
	allowedFileList.insert("/Library/Logs/DiagnosticReports/");
	allowedFileList.insert("/System/Library/Assets");
	allowedFileList.insert("/System/Library/CoreServices");
	allowedFileList.insert("/System/Library/Fonts");
	allowedFileList.insert("/System/Library/Frameworks");
	allowedFileList.insert("/System/Library/PrivateFrameworks");
	allowedFileList.insert("/Users/somansa/Desktop/VMware");
	allowedFileList.insert("/bin/bash");
	allowedFileList.insert("/bin/launchctl");
	allowedFileList.insert("/bin/ln");
	allowedFileList.insert("/bin/ls");
	allowedFileList.insert("/bin/mkdir");
	allowedFileList.insert("/bin/ps");
	allowedFileList.insert("/bin/rm");
	allowedFileList.insert("/bin/rmdir");
	allowedFileList.insert("/bin/sh");
	allowedFileList.insert("/bin/sleep");
	allowedFileList.insert("/private/var/log/");
	allowedFileList.insert("/sbin/kextload");
	allowedFileList.insert("/sbin/kextunload");
	allowedFileList.insert("/sbin/mount");
	allowedFileList.insert("/sbin/umount");
	allowedFileList.insert("/usr/bin/awk");
	allowedFileList.insert("/usr/bin/clang");
	allowedFileList.insert("/usr/bin/clear");
	allowedFileList.insert("/usr/bin/curl");
	allowedFileList.insert("/usr/bin/cut");
	allowedFileList.insert("/usr/bin/dscl");
	allowedFileList.insert("/usr/bin/g++");
	allowedFileList.insert("/usr/bin/gcc");
	allowedFileList.insert("/usr/bin/grep");
	allowedFileList.insert("/usr/bin/logger");
	allowedFileList.insert("/usr/bin/make");
	allowedFileList.insert("/usr/bin/pluginkit");
	allowedFileList.insert("/usr/bin/scp");
	allowedFileList.insert("/usr/bin/sed");
	allowedFileList.insert("/usr/bin/ssh");
	allowedFileList.insert("/usr/bin/sudo");
	allowedFileList.insert("/usr/bin/uname");
	allowedFileList.insert("/usr/bin/xcodebuild");
	allowedFileList.insert("/usr/bin/xcrun");
	allowedFileList.insert("/usr/lib/dyld");
	allowedFileList.insert("/usr/libexec/");
	allowedFileList.insert("/usr/local/Privacy-i/");
	allowedFileList.insert("/usr/sbin/kextstat");
	allowedFileList.insert("/usr/sbin/sysctl");
	allowedFileList.insert("PISupervisor");
	allowedFileList.insert("nsurlsessiond"); // com.apple.nsurlsessiond

	return true;
}

////////////////////////////////////////
// CPIConfigMan
CPIConfigMan::CPIConfigMan() {
	clear();
}

CPIConfigMan::~CPIConfigMan() {	
}

bool CPIConfigMan::initialize(void) {
	CPIObject::initialize();

	path.initialize();
	log.initialize();
	socket.initialize();
	wall.initialize();
	dlpPolicy.initialize();

	return true;
}

void CPIConfigMan::clear(void) {
	CPIObject::clear();

	path.clear();
	log.clear();
	socket.clear();
	wall.clear();
	dlpPolicy.clear();

	ttlCycle = 10;
	agentAutoRestartUse = false;
	fileMonitorAutoRestartUse = false;
	quarantinePath = "";
	copyPreventBlockWriteAll = false;
	blockHidUse = false;
	blockHidCCSpec = false;
	allowProcessList.clear();
	allowFolderList.clear();
	allowFileExtList.clear();
	cdsuAutoRestartUse = false;
	driveManagementUse = false;
	driveDefaultBlockUse = false;
	unRegisterUSBReadBlockUse = false;
	blockDeviceIDList.clear();
	accessBlockRegistryList.clear();
	qtLimitUse = false;
	allowUSBIDList.clear();
    m_bAirDropProtectUse = false;
    
    // FullDiskAccess
     m_bFDAEnable = false;
     m_nFDACheckInterval = 0;
}

bool CPIConfigMan::isBlockWriteAll(void) {
	return copyPreventBlockWriteAll;
}

bool CPIConfigMan::load(void) {
	if( false == loadConfig() ) {
		return false;
	}

	log.applyDepth();
	loadExceptionDeviceList();

	wall.loadConfig(path.wallFile);
	
	return true;
}

bool CPIConfigMan::loadConfig(void)
{
	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;

	bool result = false;
	do
    {
		if( false == markup.Load(path.configFile) ) {
			break;
		}

		if( false == markup.FindElem("pisecurity") ) {
			break;
		}

		markup.IntoElem();

		markup.ResetMainPos();
		if( true == markup.FindElem("filelog") ) {
			log.setDepth(markup.GetAttrib("depth"));
			log.setUseTickCount(markup.GetAttrib("use_tickcount"));
		}
		
		markup.ResetMainPos();
		if( true == markup.FindElem("socket") ) {
			socket.setUnsignedType(markup.GetAttrib("use_unsigned_type"));
			socket.setBigEndian(markup.GetAttrib("use_big_endian"));
			socket.setSavePacket(markup.GetAttrib("save_packet"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("client") ) {
			setTTLCycle(markup.GetAttrib("ttlcycle"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("agent_auto_restart") ) {
			setAgentAutoRestartUse(markup.GetAttrib("use"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("filemonitor_auto_restart") ) {
			setFileMonitorAutoRestartUse(markup.GetAttrib("use"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("copy_prevent") ) {
			setQuarantinePath(markup.GetAttrib("save_path"));
			setCopyPreventBlockWriteAll(markup.GetAttrib("block_write_all"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("block_hid") ) {
			setBlockHidUse(markup.GetAttrib("use"));
			setBlockHidCCSpec(markup.GetAttrib("cc_spec"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("allow_program_list") ) {
			while(true == markup.FindChildElem("program")) {
				std::string processName = markup.GetChildAttrib("name");
				if(false == processName.empty() ) {
					util.toUpper(processName);
					allowProcessList.push_back(processName);
				}
			}
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("allow_paths_for_ext") ) {
			while(true == markup.FindChildElem("path")) {
				std::string folderName = markup.GetChildAttrib("name");
				if(false == folderName.empty() ) {
					util.toUpper(folderName);
					allowFolderList.push_back(folderName);
				}
			}
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("allow_file_ext") ) {
			while(true == markup.FindChildElem("ext")) {
				std::string extName = markup.GetChildAttrib("name");
				if(false == extName.empty() ) {
					util.toUpper(extName);
					allowFileExtList.push_back(extName);
				}
			}
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("cdsu_auto_restart") ) {
			setCdsuAutoRestartUse(markup.GetAttrib("use"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("drive_management") ) {
			setDriveManagementUse(markup.GetAttrib("use"));
		}
		
		markup.ResetMainPos();
		if( true == markup.FindElem("drive_policy_default_block") ) {
			setDriveDefaultBlockUse(markup.GetAttrib("use"));
		}
		
		markup.ResetMainPos();
		if( true == markup.FindElem("nonregister_usbdisk_read_block") ) {
			setUnRegisterUSBReadBlockUse(markup.GetAttrib("use"));
		}

		markup.ResetMainPos();
		if( true == markup.FindElem("block_device_list") ) {
			while(true == markup.FindChildElem("device")) {
				std::string deviceName = markup.GetChildAttrib("InstanceID");
				if(false == deviceName.empty() ) {
					util.toUpper(deviceName);
					blockDeviceIDList.push_back(deviceName);
				}
			}
		}
		
		markup.ResetMainPos();
		if( true == markup.FindElem("access_block_registry_list") ) {
			while(true == markup.FindChildElem("registry")) {
				std::string registryName = markup.GetChildAttrib("name");
				if(false == registryName.empty() ) {
					util.toUpper(registryName);
					accessBlockRegistryList.push_back(registryName);
				}
			}
		}
		
		markup.ResetMainPos();
		if( true == markup.FindElem("QuarantineLimit") ) {
			setQtLimitUse(markup.GetAttrib("use"));
		}
		
		markup.ResetMainPos();
		if( true == markup.FindElem("application_control") ) {
			std::string value = markup.GetAttrib("enable");
			if (0 < value.length()) {
				dlpPolicy.set(nsPISecObject::dlpsubtypeApplicationPreventPlus, value);
			}
			dlpPolicy.applicationControl.set(&markup);
		}
        
        markup.ResetMainPos();
        if( true == markup.FindElem("airdrop_protect") )
        {
            setAirDropProtectUse( markup.GetAttrib("use") );
        }
        
        markup.ResetMainPos();
        if(true == markup.FindElem("FullDiskAccessCheck"))
        {
            m_bFDAEnable = util.getValueAsBool(markup.GetAttrib("enable"));
            if(true == m_bFDAEnable)
            {
                m_nFDACheckInterval = util.toInt(markup.GetAttrib("CheckInterval"));
            }
        }
        
		markup.OutOfElem();

		result = true;
	} while(false);

	return result;
}

bool CPIConfigMan::loadExceptionDeviceList(void) {

	allowUSBIDList.clear();
	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;
	std::string file = path.dlpConfigPath + "/pisec_exception_device.xml";

	bool result = false;
	do {
		if( false == markup.Load(file) ) {
			break;
		}

		if( false == markup.FindElem("allow_device_list") ) {
			break;
		}

		while( true == markup.FindChildElem("usbdevice") ) {
			std::string usbId = markup.GetChildAttrib("instanceid");
			if( (true == usbId.empty()) || (usbId.length() < 15) ) {
				continue;
			}

			util.replace(usbId, "$", "&");
			allowUSBIDList.push_back(usbId);
		}

		result = true;
	} while(false);

	return result;
}

bool CPIConfigMan::save(void) {

	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;

	markup.AddElem("pisecurity");
	markup.IntoElem();

	if( true == markup.AddElem("filelog") ) {
		markup.SetAttrib("depth", 0);
	}
		
	if( true == markup.AddElem("socket") ) {
		markup.SetAttrib("use_unsigned_type", "true");
		markup.SetAttrib("use_big_endian", "false");
		markup.SetAttrib("save_packet", "false");
	}
	
	if( true == markup.AddElem("client") ) {
		markup.SetAttrib("ttlcycle", 10);
	}

	markup.OutOfElem();

	return markup.Save(path.configFile);
}

void CPIConfigMan::setTTLCycle(std::string ttlCycle) {
	this->ttlCycle = atoi(ttlCycle.c_str());

	if( this->ttlCycle < 2 ) {
		this->ttlCycle = 2;
	}

	if( this->ttlCycle > 60 ) {
		this->ttlCycle = 60;
	}
}

void CPIConfigMan::setAgentAutoRestartUse(std::string agentAutoRestartUse) {
	this->agentAutoRestartUse = util.getValueAsBool(agentAutoRestartUse);
}

void CPIConfigMan::setFileMonitorAutoRestartUse(std::string fileMonitorAutoRestartUse) {
	this->fileMonitorAutoRestartUse = util.getValueAsBool(fileMonitorAutoRestartUse);
}

void CPIConfigMan::setQuarantinePath(std::string quarantinePath) {
	this->quarantinePath = quarantinePath;
}

void CPIConfigMan::setCopyPreventBlockWriteAll(std::string copyPreventBlockWriteAll) {
	this->copyPreventBlockWriteAll = util.getValueAsBool(copyPreventBlockWriteAll);
}

void CPIConfigMan::setBlockHidUse(std::string blockHidUse) {
	this->blockHidUse = util.getValueAsBool(blockHidUse);
}

void CPIConfigMan::setBlockHidCCSpec(std::string blockHidCCSpec) {
	this->blockHidCCSpec = util.getValueAsBool(blockHidCCSpec);
}

void CPIConfigMan::setCdsuAutoRestartUse(std::string cdsuAutoRestartUse) {
	this->cdsuAutoRestartUse = util.getValueAsBool(cdsuAutoRestartUse);
}

void CPIConfigMan::setDriveManagementUse(std::string driveManagementUse) {
	this->driveManagementUse = util.getValueAsBool(driveManagementUse);
}

void CPIConfigMan::setDriveDefaultBlockUse(std::string driveDefaultBlockUse) {
	this->driveDefaultBlockUse = util.getValueAsBool(driveDefaultBlockUse);
}

void CPIConfigMan::setUnRegisterUSBReadBlockUse(std::string unRegisterUSBReadBlockUse) {
	this->unRegisterUSBReadBlockUse = util.getValueAsBool(unRegisterUSBReadBlockUse);
}

void CPIConfigMan::setQtLimitUse(std::string qtLimitUse) {
	this->qtLimitUse = util.getValueAsBool(qtLimitUse);
}

void CPIConfigMan::setAirDropProtectUse(std::string qtAirDropProtectUse)
{
    this->m_bAirDropProtectUse = util.getValueAsBool( qtAirDropProtectUse );
}


#endif
