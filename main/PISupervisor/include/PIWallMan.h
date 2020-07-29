#ifndef	_PIWALLMAN_H
#define	_PIWALLMAN_H

#include "PIObject.h"
#include "PWDiskStub.h"
#include "PIWallController.h"

class CPIWallMan : public CPIObject {
	public:
		CPIWallMan();
		virtual ~CPIWallMan();

	public:
		CPWDiskStub pwDiskStub;
		CPIWallController controller;
		std::string volumePath;
		std::string rootPath;
		std::string dataPath;
		std::string drivePath;

	protected:
		bool unmountFileSystem(void);
		bool mountFileSystem(void);
		bool useSparse(void);
		void setVolumePath(const std::string& volumePath);

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);

	public:
		bool start(void);
		bool stop(void);
		bool isReady(void);
		bool addPid(pid_t pid);
		bool removePid(pid_t pid);
		bool removeAllPid(void);
		bool quarantineFile(const std::string& orgFilePath, const std::string& encryptedFile, std::string& quarantinedFilePath, int& errorCode);
		bool releaseQuarantinedFile(const std::string& quarantinedFilePath, const std::string& targetPath, std::string& releasedFilePath, int& errorCode);
		bool deleteQuarantinedFile(const std::string& quarantinedFilePath, int& errorCode);
		bool getDiskSize(double& totalSize, double& freeSize); 
		bool convertDiskSize(void); 
		bool getDiskVolumePath(std::string& volumePath);
		std::string getRootPath(void);
		std::string getDrivePath(void);
		std::string getDiskSize(void); 
};
#endif // #ifndef _PIWALLMAN_H
