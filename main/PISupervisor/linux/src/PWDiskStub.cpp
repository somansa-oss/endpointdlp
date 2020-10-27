#ifndef	_PWDISKSTUB_CPP
#define _PWDISKSTUB_CPP

#include "PWDiskStub.h"
#include "KextManager.h"
#include <stdlib.h>

CPWDiskStub::CPWDiskStub()
{
}

CPWDiskStub::~CPWDiskStub()
{
}

bool CPWDiskStub::initialize(KERNEL_EVENTHANDLER kernel_eventhandler)
{
	return true;
}

bool CPWDiskStub::finalize(void)
{
	return true;
}

void CPWDiskStub::clear(void)
{
}

bool CPWDiskStub::applyPolicy(unsigned long command, void* in, unsigned long size)
{
	return true;
}

bool CPWDiskStub::addPid(pid_t pid) {
	return false;
}

bool CPWDiskStub::removePid(pid_t pid) {
	return false;
}

bool CPWDiskStub::removeAllPid(void) {
	return false;
}

void CPWDiskStub::load(void)
{
}

void CPWDiskStub::unload(void)
{
}

bool CPWDiskStub::isActive(void) {
	return false;
}

bool CPWDiskStub::mount(void) {
	return false;
}

bool CPWDiskStub::unmount(void) {
	return false;
}

bool CPWDiskStub::getDiskSize(double& totalSize, double& freeSize) {
	return false;
}

bool CPWDiskStub::convertDiskSize(void) {	
	return false;
}

bool CPWDiskStub::getDiskVolumePath(std::string& volumePath) {
	return false;
}

bool CPWDiskStub::quarantineFile(std::string filePath, std::string& qtFilePath) {
	return false;
}

bool CPWDiskStub::releaseQuarantinedFile(std::string qtFilePath, std::string dstPath, std::string& relFilePath) {
	return false;
}

bool CPWDiskStub::deleteQuarantinedFile(std::string qtFilePath) {
	bool bSuc = false;
	int nErrCode = 0;
	char czQtFilePath[MAX_PATH];

	memset( czQtFilePath, 0, sizeof(czQtFilePath) );
	strcpy( czQtFilePath, qtFilePath.c_str());
	
	bSuc = g_AppKext.PWDisk_ProcessQuarantineFile_Delete( czQtFilePath, nErrCode );

	if( true == bSuc ) {
		result = 0;
	} else {
		result = nErrCode;
	}

	return bSuc;
}
#endif
