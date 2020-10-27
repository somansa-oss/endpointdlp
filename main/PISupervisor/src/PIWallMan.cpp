#ifndef	_PIWALLMAN_CPP
#define	_PIWALLMAN_CPP

#include "PIWallMan.h"
#include "PIDocument.h"
#include "PIDiskSize.h"
#include <iostream>
#include <fstream>

////////////////////////////////////////
// class CPIWallMan

CPIWallMan::CPIWallMan(void) {
}

CPIWallMan::~CPIWallMan(void) {	
}

bool CPIWallMan::initialize(void) {
	DEBUG_LOG1("start");

	CPIObject::initialize();

	if (false == isReady()) {
		return false;
	}
    
	bool result = false;
	DEBUG_LOG1("open PWDisk - begin");
	result = pwDiskStub.initialize(CPIDeviceMan::smartdrvEventHandler);
	if( false == result ) {
		ERROR_LOG("open PWDisk - fail - result(%d)", pwDiskStub.result);
	}
	DEBUG_LOG("open PWDisk - %s - result(%d)", (result) ? "ok" : "fail", pwDiskStub.result);
	INFO_LOG("open PWDisk - %s", (result) ? "ok" : "fail");

	if( true == result ) {
		controller.wallControl.setBinPath(ConfigMan.path.binPath);
		controller.wallControl.setImageFilePath(ConfigMan.path.wallImagePath, ConfigMan.wall.imageName);
		start();
	}

	return true;
}

bool CPIWallMan::finalize(void) {
	DEBUG_LOG1("start");

	if( true == isReady() ) {
		stop();
		pwDiskStub.finalize();
	}

	CPIObject::finalize();
	return true;
}

bool CPIWallMan::unmountFileSystem(void) {
	DEBUG_LOG1("start");

	bool result = pwDiskStub.unmount();
	if( false == result ) {
		ERROR_LOG("unmount PWDisk - failed - result(%d)", pwDiskStub.result);
	}

	DEBUG_LOG("unmount PWDisk - %s - result(%d)", (result)?"ok":"fail", pwDiskStub.result);
	INFO_LOG("unmount PWDisk - %s", (result)?"ok":"fail");

	return result;
}

bool CPIWallMan::mountFileSystem(void) {
	DEBUG_LOG1("start");

	bool result = pwDiskStub.mount();
	if( false == result ) {
		ERROR_LOG("mount PWDisk - failed - result(%d)", pwDiskStub.result);
	}

	DEBUG_LOG("mount PWDisk - %s - result(%d)", (result)?"ok":"fail", pwDiskStub.result);
	INFO_LOG("mount PWDisk - %s", (result)?"ok":"fail");

	std::string volumePath;
	if( true == getDiskVolumePath(volumePath) ) {
		setVolumePath(volumePath);
	}

	return result;
}

bool CPIWallMan::start(void) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = mountFileSystem();	

	if( false == result ) {
		ERROR_LOG1("mount_file_system - fail");
	}

	DEBUG_LOG("mount_file_system - %s", (result) ? "ok" : "fail");
	INFO_LOG("mount_file_system - %s", (result) ? "ok" : "fail");

	return result;
}

bool CPIWallMan::stop(void) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = unmountFileSystem();	

	if( false == result ) {
		ERROR_LOG1("unmount_file_system - fail");
	}

	DEBUG_LOG("unmount_file_system - %s", (result) ? "ok" : "fail");
	INFO_LOG("unmount_file_system - %s", (result) ? "ok" : "fail");

	
	return result;
}

bool CPIWallMan::isReady(void) {
	bool result = ConfigMan.wall.usePWDisk;

	if( false == result ) {
		DEBUG_LOG1("pwdisk_in_active");
	}

	return result;
}

bool CPIWallMan::addPid(pid_t pid) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.addPid(pid);
	if( false == result ) {
		ERROR_LOG("add_pid - fail - result(%d)", pwDiskStub.result);
		return false;
	}

	DEBUG_LOG("add_pid - pid:%d - %s - result(%d)", pid, (result) ? "ok" : "fail", pwDiskStub.result);
	INFO_LOG("add_pid - %s", (result) ? "ok" : "fail");

	return true;
}

bool CPIWallMan::removePid(pid_t pid) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.removePid(pid);
	if( false == result ) {
		ERROR_LOG("remove_pid - fail - result(%d)", pwDiskStub.result);
		return false;
	}

	DEBUG_LOG("remove_pid - pid:%d - %s - result(%d)", pid, (result) ? "ok" : "fail", pwDiskStub.result);
	INFO_LOG("remove_pid - %s", (result) ? "ok" : "fail");

	return true;
}

bool CPIWallMan::removeAllPid(void) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.removeAllPid();
	if( false == result ) {
		ERROR_LOG("remove_all_pid - fail - result(%d)", pwDiskStub.result);
		return false;

	}
	DEBUG_LOG("remove_all_pid - %s - result(%d)", (result) ? "ok" : "fail", pwDiskStub.result);
	INFO_LOG("remove_all_pid - %s", (result) ? "ok" : "fail");

	return true;
}

bool CPIWallMan::quarantineFile(const std::string& orgFilePath, const std::string& encryptedFile, std::string& quarantinedFilePath, int& errorCode) {
	DEBUG_LOG1("start");
	DEBUG_LOG("org:%s - encrypted:%s", orgFilePath.c_str(), encryptedFile.c_str());
	errorCode = 0;
	
	if( false == isReady() ) {
		return false;
	}

	if( 0 == orgFilePath.length() ) {
		DEBUG_LOG1("stop - invalid filepath");
		ERROR_LOG1("stop - invalid filepath");
		return false;
	}	

	bool result = false;
	if( 0 < encryptedFile.length() ) {
		result = pwDiskStub.quarantineFile(encryptedFile, quarantinedFilePath);
	} else {
		result = pwDiskStub.quarantineFile(orgFilePath, quarantinedFilePath);
	}

	if( false == result ) {
		ERROR_LOG("quarantine - fail - result(%d)", pwDiskStub.result);
		errorCode = 0;
	} else {
		errorCode = 1;
	}

	DEBUG_LOG("quarantine - %s - result(%d)", (result) ? "ok" : "fail", pwDiskStub.result);
	DEBUG_LOG("quarantine - quarantined:%s", quarantinedFilePath.c_str());
	INFO_LOG("quarantine - %s", (result) ? "ok" : "fail");
	DEBUG_LOG1("end");

	return result;
} 

bool CPIWallMan::releaseQuarantinedFile(const std::string& quarantinedFilePath, const std::string& targetPath, std::string& releasedFilePath, int& errorCode) {
	DEBUG_LOG1("start");
	errorCode = 0;
	
	if( false == isReady() ) {
		return false;
	}

	if( 0 == quarantinedFilePath.length() ) {
		DEBUG_LOG1("skip - invalid quarantine_file");
		ERROR_LOG1("skip - invalid quarantine_file");
		return false;
	}	

	if( 0 == targetPath.length() ) {
		DEBUG_LOG1("skip - invalid target_path");
		ERROR_LOG1("skip - invalid target_path");
		return false;
	}	

	DEBUG_LOG("check - qfile:%s - target:%s", quarantinedFilePath.c_str(), targetPath.c_str());

	bool result = pwDiskStub.releaseQuarantinedFile(quarantinedFilePath, targetPath, releasedFilePath);
	if( false == result ) {
		ERROR_LOG("release - fail - result(%d)", pwDiskStub.result);
		errorCode = 0;
	} else {
		errorCode = 1;
	}

	DEBUG_LOG("release - %s - result(%d)", (result) ? "ok" : "fail", pwDiskStub.result);
	INFO_LOG("release - %s", (result) ? "ok" : "fail");
	DEBUG_LOG1("end");

	return result;
} 

bool CPIWallMan::deleteQuarantinedFile(const std::string& quarantinedFilePath, int& errorCode) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.deleteQuarantinedFile(quarantinedFilePath);
	if( false == result ) {
		ERROR_LOG("delete - fail - result(%d)", pwDiskStub.result);
		errorCode = 0;
	} else {
		errorCode = 1;
	}

	DEBUG_LOG("delete - %s - result(%d)", (result) ? "ok" : "fail", pwDiskStub.result);
	INFO_LOG("delete - %s", (result) ? "ok" : "fail");
	DEBUG_LOG1("end");

	return result;
}

void CPIWallMan::setVolumePath(const std::string& volumePath) {
	this->volumePath = volumePath; // ex) /Volumes/PWDisk_D
	this->rootPath = this->volumePath + "/SMSDisk"; // ex) /Volumes/PWDisk_D/SMSDisk
	this->dataPath = this->rootPath + "/Data"; // ex) /Volumes/PWDisk_D/SMSDisk/Data
	this->drivePath = this->rootPath + "/Drive"; // ex) /Volumes/PWDisk_D/SMSDisk/Drive

	DEBUG_LOG("volumePath(%s)", this->volumePath.c_str());
	DEBUG_LOG("rootPath(%s)", this->rootPath.c_str());
	DEBUG_LOG("dataPath(%s)", this->dataPath.c_str());
	DEBUG_LOG("drivePath(%s)", this->drivePath.c_str());
}

std::string CPIWallMan::getRootPath(void) {
	return rootPath;
}

std::string CPIWallMan::getDrivePath(void) {
	return drivePath;
}

bool CPIWallMan::useSparse(void) {
	return ConfigMan.wall.useSparse;
}

bool CPIWallMan::getDiskSize(double& totalSize, double& freeSize) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.getDiskSize(totalSize, freeSize);
	if( false == result ) {
		ERROR_LOG("get_disksize PWDisk - failed - result(%d)", pwDiskStub.result);
	}

	DEBUG_LOG("get_disksize PWDisk - %s - result(%d) - total:%.0f, - free:%.0f", (result)?"ok":"fail", pwDiskStub.result, totalSize, freeSize);
	INFO_LOG("get_disksize PWDisk - %s", (result)?"ok":"fail");

	return result;
}

bool CPIWallMan::convertDiskSize(void) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.convertDiskSize();
	if( false == result ) {
		ERROR_LOG("convert_disksize PWDisk - failed - result(%d)", pwDiskStub.result);
	}

	DEBUG_LOG("convert_disksize PWDisk - %s - result(%d)", (result)?"ok":"fail", pwDiskStub.result);
	INFO_LOG("convert_disksize PWDisk - %s", (result)?"ok":"fail");

	return result;
}

bool CPIWallMan::getDiskVolumePath(std::string& volumePath) {
	DEBUG_LOG1("start");
	
	if( false == isReady() ) {
		return false;
	}

	bool result = pwDiskStub.getDiskVolumePath(volumePath);
	if( false == result ) {
		ERROR_LOG("get_diskvolumepath PWDisk - failed - result(%d)", pwDiskStub.result);
	}

	DEBUG_LOG("get_diskvolumepath PWDisk - %s - result(%d) - path:%s", (result)?"ok":"fail", pwDiskStub.result, volumePath.c_str());
	INFO_LOG("get_diskvolumepath PWDisk - %s", (result)?"ok":"fail");
	DEBUG_LOG1("end");

	return result;
}

std::string CPIWallMan::getDiskSize(void) {
	DEBUG_LOG1("start");

	CPIDiskSize quarantineDiskSize;
	CPIDiskSize systemDiskSize;
	if( false == systemDiskSize.getSystemDiskSize() ) {
		ERROR_LOG1("get_system_disk_size - failed");
		return "";
	}

	quarantineDiskSize.clear();
	if( false == getDiskSize(quarantineDiskSize.totalBytes, quarantineDiskSize.freeBytes) ) {
		ERROR_LOG1("get_pwdisk_size - failed");
		return "";
	}
	quarantineDiskSize.getUsedBytes();

#define SIZE_KB 1000
#define SIZE_MB (SIZE_KB * 1000)
#define SIZE_GB (SIZE_MB * 1000)

	CMarkup markup;
	markup.InsertNode(CMarkup::MNT_PROCESSING_INSTRUCTION, "xml");
	markup.SetAttrib("version", "1.0");
	markup.SetAttrib("encoding", "utf-8");

	markup.AddElem("DiskSize");
	markup.IntoElem();
	markup.AddElem("QuarantineDisk");
	markup.IntoElem();
	markup.AddElem("Total");
	markup.AddAttrib("Bytes", util.getValueAsString(quarantineDiskSize.totalBytes));
	markup.AddAttrib("MB", util.getValueAsString(quarantineDiskSize.totalBytes/SIZE_MB));
	markup.AddAttrib("GB", util.getValueAsString(quarantineDiskSize.totalBytes/SIZE_GB));
	markup.AddElem("Free");
	markup.AddAttrib("Bytes", util.getValueAsString(quarantineDiskSize.freeBytes));
	markup.AddAttrib("MB", util.getValueAsString(quarantineDiskSize.freeBytes/SIZE_MB));
	markup.AddAttrib("GB", util.getValueAsString(quarantineDiskSize.freeBytes/SIZE_GB));
	markup.AddElem("Used");
	markup.AddAttrib("Bytes", util.getValueAsString(quarantineDiskSize.usedBytes));
	markup.AddAttrib("MB", util.getValueAsString(quarantineDiskSize.usedBytes/SIZE_MB));
	markup.AddAttrib("GB", util.getValueAsString(quarantineDiskSize.usedBytes/SIZE_GB));
	markup.OutOfElem();
	markup.AddElem("SystemDisk");
	markup.IntoElem();
	markup.AddElem("Total");
	markup.AddAttrib("Bytes", util.getValueAsString(systemDiskSize.totalBytes));
	markup.AddAttrib("MB", util.getValueAsString(systemDiskSize.totalBytes/SIZE_MB));
	markup.AddAttrib("GB", util.getValueAsString(systemDiskSize.totalBytes/SIZE_GB));
	markup.AddElem("Free");
	markup.AddAttrib("Bytes", util.getValueAsString(systemDiskSize.freeBytes));
	markup.AddAttrib("MB", util.getValueAsString(systemDiskSize.freeBytes/SIZE_MB));
	markup.AddAttrib("GB", util.getValueAsString(systemDiskSize.freeBytes/SIZE_GB));
	markup.AddElem("Used");
	markup.AddAttrib("Bytes", util.getValueAsString(systemDiskSize.usedBytes));
	markup.AddAttrib("MB", util.getValueAsString(systemDiskSize.usedBytes/SIZE_MB));
	markup.AddAttrib("GB", util.getValueAsString(systemDiskSize.usedBytes/SIZE_GB));
	std::string result = markup.GetDoc();

	DEBUG_LOG("get_pwdisk_size - result:\n%s", result.c_str());
	INFO_LOG1("get_pwdisk_size - ok");

	return result;
}
#endif
