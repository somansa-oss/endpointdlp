#ifndef	_DEVICECONTROLLER_H
#define	_DEVICECONTROLLER_H

#include "PIObject.h"
#include "DLPPolicy.h"
#include "PIDeviceLog.h"

#ifndef LINUX
#include "PIDRDevice.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//class IPIDeviceControl
class IPIDeviceControl : public CPIObject
{
	public:
		IPIDeviceControl(CPISecRule& rule);
		virtual ~IPIDeviceControl();

	public:
		CPISecRule rule;
		std::vector<std::string> deviceNameList;

	public:
        virtual bool controlWLANSSID( std::vector<std::string> vecWLanPermitList ) = 0;
		virtual bool controlWLAN(void) = 0;
		virtual bool controlWWAN(void) = 0;
		virtual bool controlBluetooth(void) = 0;
        virtual bool controlCamera(void) = 0;
        virtual bool controlWriteCDDVD(void) = 0;
        virtual bool controlFileSharing(void) = 0;
        virtual bool controlRemoteManagement(void)=0;
        virtual bool controlAirDrop(void)=0;
    
};

////////////////////////////////////////////////////////////////////////////////
//class CPIDeviceControlApple
class CPIDeviceControlApple : public IPIDeviceControl
{
	public:
		CPIDeviceControlApple(CPISecRule& rule);
		virtual ~CPIDeviceControlApple();

	protected:
		bool controlNdis(std::string name);
    
	public:
        virtual bool controlWLANSSID( std::vector<std::string> vecWLanPermitList );
		virtual bool controlWLAN(void);
		virtual bool controlWWAN(void);
		virtual bool controlBluetooth(void);
        virtual bool controlCamera(void);
        virtual bool controlWriteCDDVD(void);
        virtual bool controlFileSharing(void);
        virtual bool controlRemoteManagement(void);
        virtual bool controlAirDrop(void);
    
};

////////////////////////////////////////////////////////////////////////////////
//class CPIDeviceControlLinux
class CPIDeviceControlLinux : public IPIDeviceControl
{
	public:
		CPIDeviceControlLinux(CPISecRule& rule): IPIDeviceControl(rule) {};
		virtual ~CPIDeviceControlLinux() {};

	public:
        virtual bool controlWLANSSID( std::vector<std::string> vecWLanPermitList ) { return false; }
		virtual bool controlWLAN(void) { return false; };
		virtual bool controlWWAN(void) { return false; };
		virtual bool controlBluetooth(void) { return false; };
        virtual bool controlCamera(void) { return false; };
        virtual bool controlWriteCDDVD(void) { return false; }
        virtual bool controlFileSharing(void) { return false; }
        virtual bool controlRemoteManagement(void) { return false; }
        virtual bool controlAirDrop(void) { return false; }
};

////////////////////////////////////////////////////////////////////////////////
//class CPIDeviceController 
class CPIDeviceController : public CPIObject
{
public:
    CPIDeviceController(CPISecRule& rule);
    virtual ~CPIDeviceController();
    
public:
    CPISecRule rule;
    
public:
#ifdef __APPLE__
    CPIDeviceControlApple deviceControl;
#else
    CPIDeviceControlLinux  deviceControl;
#endif // #ifdef __APPLE__
    
public:
    bool control(void);
    bool getDeviceLog(CPIDeviceLog::VECTOR& deviceLogList);
    void appendDeviceName(std::string strDeviceName );
    void appendDeviceLog( std::string strDeviceName );
};
#endif // #ifndef _DEVICECONTROLLER_H
