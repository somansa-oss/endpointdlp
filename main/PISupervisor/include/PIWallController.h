#ifndef	_WALLCONTROLLER_H
#define	_WALLCONTROLLER_H

#include "PIObject.h"

////////////////////////////////////////////////////////////////////////////////
//class IPIWallControl
class IPIWallControl : public CPIObject {
	public:
		IPIWallControl();
		virtual ~IPIWallControl();
	
	public:
		std::string imageName;
		std::string imagePath;
		std::string imageFilePath;
		std::string runScriptFile;
		std::string volumeName;

	public:
		virtual bool mountIMG(void) = 0;
		virtual bool createIMG(void) = 0;
		virtual bool unmountIMG(void) = 0;
	
	public:
		bool setBinPath(std::string binPath);
		bool setImageFilePath(std::string imagePath, std::string imageName);
		std::string getImageFileName(void) const;
		std::string getImageFilePath(void) const { return imageFilePath; };
		std::string getImageName(void) const { return imageName; };
		std::string getImagePath(void) const { return imagePath; };
		std::string getVolumeName(void) const { return volumeName; };
		bool isMounted(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CPIWallControlApple
class CPIWallControlApple : public IPIWallControl{
	public:
		CPIWallControlApple();
		virtual ~CPIWallControlApple();

	public:		
		bool mountIMG(void);
		bool createIMG(void);
		bool unmountIMG(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CPIWallControlLinux
class CPIWallControlLinux : public IPIWallControl{
	public:
		CPIWallControlLinux(): IPIWallControl() {};
		virtual ~CPIWallControlLinux() {};
	
	public:		
		bool mountIMG(void) { return true; };
		bool createIMG(void) { return true; };
		bool unmountIMG(void) { return true; };
};

////////////////////////////////////////////////////////////////////////////////
//class CPIWallController 
class CPIWallController : public CPIObject {
	public:
		CPIWallController();
		virtual ~CPIWallController();
	
	public:
#ifdef __APPLE__
		CPIWallControlApple wallControl;
#else
		CPIWallControlLinux wallControl;
#endif // #ifdef __APPLE__
};
#endif // #ifndef _WALLCONTROLLER_H
