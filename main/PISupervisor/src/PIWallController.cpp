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
	return true;
}


////////////////////////////////////////////////////////////////////////////////
//class CPIWallController 
CPIWallController::CPIWallController() {
}

CPIWallController::~CPIWallController() {
}
#endif // #ifndef _WALLCONTROLLER_CPP
