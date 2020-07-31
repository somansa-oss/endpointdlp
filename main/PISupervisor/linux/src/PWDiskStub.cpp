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
    for(int i=0; i<3; i++)
    {
        load();
        if(true == isActive())
        {
            break;
        }
    }
	
	if( false == isActive() )
    {
		return false;
	}
    
    result = g_AppKext.KextModule_Startup( kernel_eventhandler, true );
	if(result < 0)
    {
		return false;
	}
	initialized = true;
	return true;
}

bool CPWDiskStub::finalize(void)
{
	result = g_AppKext.KextModule_Cleanup( true );
	if( true == isActive() )
    {
		// unload();
	}
	return true;
}

void CPWDiskStub::clear(void)
{
	//not used
}

bool CPWDiskStub::applyPolicy(unsigned long command, void* in, unsigned long size)
{
	//not used
	return true;
}

bool CPWDiskStub::addPid(pid_t pid) {
	result = g_AppKext.PWDisk_PWProcessID_Add(pid);
	return (0 == result)?true:false;
}

bool CPWDiskStub::removePid(pid_t pid) {
	result = g_AppKext.PWDisk_PWProcessID_Remove(pid);
	return (0 == result)?true:false;
}

bool CPWDiskStub::removeAllPid(void) {
	result = g_AppKext.PWDisk_PWProcessID_RemoveAll();
	return (0 == result)?true:false;
}

void CPWDiskStub::load(void)
{
    system("kextload -v 6 /Library/Extensions/PWDisk.kext");
}

void CPWDiskStub::unload(void)
{
    // system("kextunload -v 6 /Library/Extensions/PWDisk.kext");
}

bool CPWDiskStub::isActive(void) {
	std::string command, temp;
	command = "kextstat | grep com.somansa.PWDisk";
	temp = util.readCommandOutput(command);

	if( true == temp.empty() ) {
		return false;
	}

	std::istringstream is(temp);
	std::string token;
	std::getline(is, token);

	if( std::string::npos != token.find("PWDisk") ) {
		return true;
	}

	return false;
}

bool CPWDiskStub::mount(void) {
	result = g_AppKext.PWDisk_PWProcess_load();
	return (0 == result)?true:false;
}

bool CPWDiskStub::unmount(void) {
	result = g_AppKext.PWDisk_PWProcess_unload();
	return (0 == result)?true:false;
}

bool CPWDiskStub::getDiskSize(double& totalSize, double& freeSize) {
	PWDISK_SIZE DiskSize;
	memset( &DiskSize, 0, sizeof(DiskSize) );
	bool bSuc = g_AppKext.PWdisk_GetDiskVolumePath( DiskSize.czVolumePath, sizeof(DiskSize.czVolumePath) );
	if(true == bSuc ) {
		bSuc = g_AppKext.PWDisk_GetDiskSize( &DiskSize );
	}

	if( true == bSuc ) {
		result = 0;
		totalSize = DiskSize.nTotalSize;
		freeSize = DiskSize.nFreeSize;
	} else {
		result = -1;
		totalSize = 0;
		freeSize = 0;
	}
	
	return bSuc;
}

bool CPWDiskStub::convertDiskSize(void) {
	PWDISK_SIZE DiskSize;
	memset( &DiskSize, 0, sizeof(DiskSize) );
	bool bSuc = g_AppKext.PWDisk_DuplicateDiskSize(&DiskSize);
	if(true == bSuc ) {
		bSuc = g_AppKext.PWDisk_ConvertDiskSize(&DiskSize, 1); // increase by 1GB
	}

	result = (true==bSuc)?0:-1;
	return bSuc;
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
