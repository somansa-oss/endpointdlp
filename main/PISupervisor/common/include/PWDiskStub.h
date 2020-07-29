#ifndef	_PWDISKSTUB_H
#define	_PWDISKSTUB_H

#include "PIKernelStub.h"

////////////////////////////////////////
//class CPWDiskStub
class CPWDiskStub : public IPIKernelStub {
	public:
		CPWDiskStub();
		virtual ~CPWDiskStub();
	
	public:
		virtual bool initialize(KERNEL_EVENTHANDLER kernel_eventhandler);
		virtual bool finalize(void);
		virtual void clear(void);
		virtual bool applyPolicy(unsigned long command, void* in, unsigned long size);
		virtual void load(void);
		virtual void unload(void);
		virtual bool isActive(void);
	
	public:
		bool addPid(pid_t pid);
		bool removePid(pid_t pid);
		bool removeAllPid(void);
		bool mount(void);
		bool unmount(void);
		bool getDiskSize(double& totalSize, double& freeSize); 
		bool convertDiskSize(void); 
		bool getDiskVolumePath(std::string& volumePath);
		bool quarantineFile(std::string filePath, std::string& qtFilePath);
		bool releaseQuarantinedFile(std::string qtFilePath, std::string dstPath, std::string& relFilePath);
		bool deleteQuarantinedFile(std::string qtFilePath);
};
#endif // #ifndef _PWDISKSTUB_H
