#ifndef	_WALLCONTROLLER_CPP
#define	_WALLCONTROLLER_CPP

#include "PIWallController.h"

////////////////////////////////////////////////////////////////////////////////
//class IPIWallControl
IPIWallControl::IPIWallControl() {
}

IPIWallControl::~IPIWallControl() {
}

std::string IPIWallControl::getImageFileName(void) const {
	return "";
}

bool IPIWallControl::setBinPath(std::string binPath) {
	return true;
}


bool IPIWallControl::setImageFilePath(std::string imagePath, std::string imageName) {
	return true;
}

bool IPIWallControl::isMounted(void) {
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//class CPIWallControlApple
CPIWallControlApple::CPIWallControlApple() {
}

CPIWallControlApple::~CPIWallControlApple() {
}

bool CPIWallControlApple::createIMG(void) {
	return false;
}

bool CPIWallControlApple::mountIMG(void) {
	//hdiutil mount ~/SmsDisk.dmg 
	//or
	//hdiutil attach ~/SmsDisk.dmg
	
	if( 0 == getImageFilePath().length() ) {
		DEBUG_LOG1("invalid image_file_path");
		ERROR_LOG1("invalid image_file_path");
		return false;
	}

	if( 0 != access(getImageFilePath().c_str(), 0x00) ) {
		DEBUG_LOG1("image_file not found");
		ERROR_LOG1("image_file not found");
		return false;
	}

	std::string command, temp;

	// attach
	//command = "hdiutil attach ";
	//command += getImageFilePath();
	//command += " 2>&1";
	
	command = runScriptFile;
	command += " mount ";
	command += getImageFilePath();

	temp = util.readCommandOutput(command);

	DEBUG_LOG( "command:%s", command.c_str());
	DEBUG_LOG( "[result] - %s", temp.c_str());

	if( true == temp.empty() ) {
		return false;
	}

	if( std::string::npos != temp.find(getImageFileName()) ) {
		return true;
	}

	return false;
}

bool CPIWallControlApple::unmountIMG(void) {
	//hdiutil unmount  /Volumes/SmsDisk
	//or
	//hdiutil detach /Volumes/SmsDisk

	std::string command, temp;

	// detach 
	//command = "hdiutil detach /Volumes/";
	//command += getImageFileName();
	//command += " 2>&1";
	
	command = runScriptFile;
	command += " unmount ";
	command += getVolumeName();
	
	temp = util.readCommandOutput(command);

	DEBUG_LOG( "command:%s", command.c_str());
	DEBUG_LOG( "[result] - %s", temp.c_str());

	if( true == temp.empty() ) {
		return false;
	}

	std::string lower = temp;
	util.toLower(lower);

	if( std::string::npos != lower.find("fail") ) {
		return false;
	}
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////
//class CPIWallController 
CPIWallController::CPIWallController() {
}

CPIWallController::~CPIWallController() {
}
#endif // #ifndef _WALLCONTROLLER_CPP
