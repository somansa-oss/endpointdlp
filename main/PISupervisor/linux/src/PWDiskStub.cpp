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
    char  czVolumePath[MAX_PATH];
	memset( czVolumePath, 0, sizeof(czVolumePath) );
	bool bSuc = g_AppKext.PWdisk_GetDiskVolumePath( czVolumePath, sizeof(czVolumePath) );
	
	if( true == bSuc ) {
		volumePath = czVolumePath;
		result = 0;
	} else {
		result = -1;
	}
	
	return bSuc;
}

bool CPWDiskStub::quarantineFile(std::string filePath, std::string& qtFilePath) {

	bool bSuc = false;
	int  nOutQtFilePath=0;
	char czFilePath[MAX_PATH];
	char czOutQtFilePath[MAX_PATH];
	int nErrCode = 0;

	memset( czFilePath, 0, sizeof(czFilePath) );
	memset( czOutQtFilePath, 0, sizeof(czOutQtFilePath) );

	strcpy( czFilePath, filePath.c_str());
	nOutQtFilePath = (int)sizeof(czOutQtFilePath);

	bSuc = g_AppKext.PWDisk_ProcessQuarantineFile( czFilePath, czOutQtFilePath, nOutQtFilePath, nErrCode );

	if( true == bSuc ) {
		qtFilePath = czOutQtFilePath;
		result = 0;
	} else {
		result = nErrCode;
	}

	return bSuc;
}

bool CPWDiskStub::releaseQuarantinedFile(std::string qtFilePath, std::string dstPath, std::string& relFilePath) {
	bool bSuc = false;
	int  nRetFilePath = 0;
	char czQtFilePath[MAX_PATH];
	char czDstFilePath[MAX_PATH];
	char czRetFilePath[MAX_PATH];
	int nErrCode = 0;

	nRetFilePath = sizeof(czRetFilePath);
	memset( czQtFilePath, 0, sizeof(czQtFilePath) );
	memset( czDstFilePath, 0, sizeof(czDstFilePath) );
	memset( czRetFilePath, 0, sizeof(czRetFilePath) );

	strcpy( czQtFilePath, qtFilePath.c_str());
	strcpy( czDstFilePath, dstPath.c_str());
	
	bSuc = g_AppKext.PWDisk_ProcessQuarantineFile_Release( czQtFilePath, czDstFilePath, czRetFilePath, nRetFilePath, nErrCode );

	if( true == bSuc ) {
		relFilePath = czRetFilePath;
		result = 0;
	} else {
		result = nErrCode;
	}

	return bSuc;
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
