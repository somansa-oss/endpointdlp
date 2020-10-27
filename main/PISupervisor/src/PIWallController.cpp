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
