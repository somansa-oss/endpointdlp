#ifndef	_DEVICECONTROLLER_CPP
#define	_DEVICECONTROLLER_CPP

#ifdef LINUX
#include <unistd.h>     // getuid
#include <lsf-api/media-api.h>
#include <assert.h>
#include <stdio.h>
#endif

#include <sstream>
#include <map>

#include "PIDeviceController.h"
#include "PIDocument.h"
#include "PIWLanCheck.h"
#include "PIActionProcessDeviceLog.h"
#include "PIDeviceLog.h"

//class IPIDeviceControl
IPIDeviceControl::IPIDeviceControl(CPISecRule& rule)
{
	this->rule = rule;
}

IPIDeviceControl::~IPIDeviceControl()
{
}


//class CPIDeviceControlApple
CPIDeviceControlApple::CPIDeviceControlApple(CPISecRule& rule) : IPIDeviceControl(rule)
{
}

CPIDeviceControlApple::~CPIDeviceControlApple()
{
}

bool CPIDeviceControlApple::controlNdis(std::string name)
{
	// get list
	// compare keyword
	// set disable (by keyword)

	bool result = false;

	std::string command, temp;
	command = "networksetup listallnetworkservices";
	command += " 2>&1";
	temp = util.readCommandOutput(command);

	DEBUG_LOG( "command:%s", command.c_str());
	DEBUG_LOG( "[result] - %s", temp.c_str());

	if( true == temp.empty() ) {
		return false;
	}

	std::vector<std::string> vectorNetworkServiceEnabled;
	std::istringstream is(temp);
	std::string token;
	while(std::getline(is, token))
    {
		if( std::string::npos != token.find("*") )
        {
			continue;
		}

		vectorNetworkServiceEnabled.push_back(token);
		DEBUG_LOG( "network_service - enabled - %s", token.c_str());
	}

	DeviceMan.getMachedNetworkService(name, vectorNetworkServiceEnabled);

	if( 0 == vectorNetworkServiceEnabled.size() )
    {
		DEBUG_LOG1( "skip - not matched");
		return false;
	}

	for( int index = 0; index < vectorNetworkServiceEnabled.size(); ++index )
    {
			command = "networksetup setnetworkserviceenabled '";
			command += vectorNetworkServiceEnabled[index];
			command += "' off";
			system(command.c_str());
			result = true;

			deviceNameList.push_back(vectorNetworkServiceEnabled[index]);
			DEBUG_LOG( "command:%s", command.c_str());
	}

	return result;
}

bool CPIDeviceControlApple::controlWLANSSID( std::vector<std::string> vecWLanPermitList )
{
    bool   bBlock = false;
    string strBlockSSID, strIfaceName, strDeviceName;
    
#ifndef LINUX

    bBlock = g_SSID.SetWLanSSIDControl( vecWLanPermitList );
    if(true == bBlock)
    {
        strBlockSSID = g_SSID.GetBlockSSID();
        strDeviceName = "[SSID:" + strBlockSSID + "] WiFi";
        deviceNameList.push_back( strDeviceName );
    }
#endif
    
    return bBlock;
}

bool CPIDeviceControlApple::controlWLAN(void)
{
	return controlNdis("WLAN");
}

bool CPIDeviceControlApple::controlWWAN(void)
{
	return controlNdis("WWAN");
}

#ifdef LINUX
bool CPIDeviceControlLinux::enableDevice(const char * media_type)
{
	bool result = false;

	lsf_media_error_t *error = NULL;
	LSF_MEDIA_STATE state = LSF_MEDIA_NONE; 
	LSF_MEDIA_RESULT r = LSF_MEDIA_FAIL;

	if (media_type == NULL)
	{
		return result;
	}

	r = lsf_media_get_state( media_type, &state, &error);
	if (r == LSF_MEDIA_SUCCESS)
	{
		if (state == LSF_MEDIA_ALLOW)
		{
			if (error != NULL)
			{
				DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
				lsf_media_free_error(error);
			}

			return false;
		}
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	r = lsf_media_set_state( media_type, LSF_MEDIA_ALLOW, &error );
	if (r == LSF_MEDIA_SUCCESS)
	{
		result = true;
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_set_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	return result;
}

bool CPIDeviceControlLinux::controlDevice(const char *media_type, const char *pvi_media_type)
{
	bool result = false;

	lsf_media_error_t *error = NULL;
	LSF_MEDIA_STATE state = LSF_MEDIA_NONE; 
	LSF_MEDIA_RESULT r = LSF_MEDIA_FAIL;

	if (media_type == NULL || pvi_media_type == NULL)
	{
		return result;
	}	

	r = lsf_media_get_state( media_type, &state, &error);
	if (r == LSF_MEDIA_SUCCESS)
	{
		//DEBUG_LOG( "command:lsf_media_get_state(%d),result-success,state-%d", LSF_MEDIA_BLUETOOTH, state);
		if (state != LSF_MEDIA_ALLOW)
		{
			if (error != NULL)
			{
				DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
				lsf_media_free_error(error);
			}

			return false;
		}
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_get_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	r = lsf_media_set_state( media_type, LSF_MEDIA_DISALLOW, &error );
	if (r == LSF_MEDIA_SUCCESS)
	{
		result = true;
		deviceNameList.push_back( pvi_media_type );
	}
	if (error != NULL)
	{
		DEBUG_LOG( "command:lsf_media_set_state(%d),result-error[%s]", media_type, error->message);
		lsf_media_free_error(error);
	}

	return result;
}

bool CPIDeviceControlLinux::controlWLAN(void)
{
	return controlDevice(LSF_MEDIA_WIRELESS, "WLAN");
}

bool CPIDeviceControlLinux::controlWWAN(void)
{
	return controlDevice(LSF_MEDIA_USB_NETWORK, "WWLAN");
}

bool CPIDeviceControlLinux::controlBluetooth(void)
{
	return controlDevice(LSF_MEDIA_BLUETOOTH, "Bluetooth");
}

bool CPIDeviceControlLinux::controlWriteCDDVD(void)
{
	return controlDevice(LSF_MEDIA_CD_DVD, "CD/DVD");
}

bool CPIDeviceControlLinux::enableWLAN(void)
{
	return enableDevice(LSF_MEDIA_WIRELESS);
}

bool CPIDeviceControlLinux::enableWWLAN(void)
{
	return enableDevice(LSF_MEDIA_USB_NETWORK);
}

bool CPIDeviceControlLinux::enableBluetooth()
{
	return enableDevice(LSF_MEDIA_BLUETOOTH);
}

bool CPIDeviceControlLinux::enableWriteCDDVD()
{
	return enableDevice(LSF_MEDIA_CD_DVD);
}

#endif

bool CPIDeviceControlApple::controlBluetooth(void)
{
	bool result = false;

	std::string command, temp;
	command = "/usr/bin/defaults read /Library/Preferences/com.apple.Bluetooth ControllerPowerState";
	command += " 2>&1";
	temp = util.readCommandOutput(command);

	DEBUG_LOG( "command:%s", command.c_str());
	DEBUG_LOG( "[result] - %s", temp.c_str());
	
	if( true == temp.empty() ) {
		return false;
	}

	std::istringstream is(temp);
	std::string token;
	std::getline(is, token);

	if( "1" != token)
    {
		return false;
	}
			
	command = "/usr/bin/defaults write /Library/Preferences/com.apple.Bluetooth ControllerPowerState -int 0"
		" | /usr/bin/killall -SIGHUP blued";
	DEBUG_LOG( "command:%s", command.c_str());
	system(command.c_str());

	// ----------
	// remark: 
	// macOS High Sierra (version 10.13)
	// How to find out macOS version from terminal: "sw_vers -productVersion"
	// High Sierra : "/usr/bin/killall -SIGHUP bluetoothd"
	// ----------

    command = "/usr/bin/killall -SIGHUP bluetoothd";
    system(command.c_str());
    
	result = true;
	deviceNameList.push_back("Bluetooth");
	return result;
}

bool CPIDeviceControlApple::controlCamera(void)
{
    bool bResult = false;
#ifndef LINUX    
    if(true == g_DRDevice.IsActiveAppleCamera())
    {
        bResult = g_DRDevice.SetAppleCameraStatus( false );
    }
#endif    
    if(true == bResult)
    {
        deviceNameList.push_back("AppleCamIn");
    }
    DEBUG_LOG("bResult == %d", bResult );
    return bResult;
}


bool CPIDeviceControlApple::controlWriteCDDVD(void)
{
#ifndef LINUX      
    g_DRDevice.SetDRDevicePolicy( rule );
    DEBUG_LOG1("Start." );
#endif

    return false;
}

bool CPIDeviceControlApple::controlFileSharing(void)
{
    bool bResult = false;
    std::vector<std::string> vecDenyList;
    
#ifndef LINUX      
    bResult = g_DRDevice.SetProtectFileSharing(true, vecDenyList);
#endif    
    if(true == bResult)
    {
        std::vector<std::string>::iterator Iter;
        for(Iter=vecDenyList.begin(); Iter != vecDenyList.end(); ++Iter)
        {   
            deviceNameList.push_back( *Iter );
        }
    }
    return bResult;
}


bool CPIDeviceControlApple::controlRemoteManagement(void)
{
    bool bResult = false;
    std::vector<std::string> vecDenyList;
    
#ifndef LINUX      
    bResult = g_DRDevice.SetProtectRemoteManagement(true, vecDenyList);
#endif    
    if(true == bResult)
    {
        std::vector<std::string>::iterator Iter;
        for(Iter=vecDenyList.begin(); Iter != vecDenyList.end(); ++Iter)
        {
            deviceNameList.push_back( *Iter );
        }
    }
    return bResult;
}

bool CPIDeviceControlApple::controlAirDrop(void)
{
    bool bResult = false;
#ifndef LINUX      
    bResult = g_DRDevice.SetProtectSharingDaemon( true );
#endif    
    if(true == bResult)
    {
        deviceNameList.push_back("AirDrop");
    }
    return bResult;
}


//class CPIDeviceController 
CPIDeviceController::CPIDeviceController(CPISecRule& rule) : deviceControl(rule)
{
    this->rule = rule;
}

CPIDeviceController::~CPIDeviceController()
{
}

bool CPIDeviceController::control(void)
{
	DEBUG_LOG("rule - %s", rule.virtualType.c_str());

	bool result = false;

	if (false == rule.disableAll && "Device\\CD/DVD" != rule.virtualType) {
		return result;
	}

	if("NDIS\\WLAN" == rule.virtualType)
    {
        if(rule.m_vecWLanPermitList.size() > 0)
        {
            result = deviceControl.controlWLANSSID( rule.m_vecWLanPermitList );
        }
        else
        {
            result = deviceControl.controlWLAN();
        }
	}
    else if("NDIS\\WWAN" == rule.virtualType )
    {
		result = deviceControl.controlWWAN();
	}
    else if("Device\\Bluetooth" == rule.virtualType )
    {
		result = deviceControl.controlBluetooth();
	}    
    else if("Device\\Camera" == rule.virtualType)
    {
        result = deviceControl.controlCamera();
    }
    else if("Device\\CD/DVD" == rule.virtualType)
    {
        result = deviceControl.controlWriteCDDVD();
    }
    else if("Share\\FileSharing" == rule.virtualType)
    {
        result = deviceControl.controlFileSharing();
    }
    else if("Share\\RemoteManagement" == rule.virtualType)
    {
        result = deviceControl.controlRemoteManagement();
    }
    
	return result;
}


bool CPIDeviceController::getDeviceLog(CPIDeviceLog::VECTOR& deviceLogList)
{
	CPIDeviceLog deviceLog;
	if( ("NDIS\\WLAN" == rule.virtualType) || ("NDIS\\WWAN" == rule.virtualType) )
    {
		deviceLog.deviceType = CPIDeviceLog::typeNdis;
	}
	else if( ("Device\\Bluetooth" == rule.virtualType) ||
             ("Device\\Camera" == rule.virtualType) ||
             // ("Share\\AirDrop" == rule.virtualType) ||
             ("Share\\FileSharing" == rule.virtualType) ||
             ("Share\\RemoteManagement" == rule.virtualType) )
    {
		deviceLog.deviceType = CPIDeviceLog::typeDevice;
	}
    
    deviceLog.accessType     = accessRead;
	deviceLog.disableResult = true;
	deviceLog.setLogTime( 	  util.getCurrentDateTime());
	deviceLog.processName 	= "kernel_task";
	deviceLog.virtualType 	= rule.virtualType;
    if("Device\\CD/DVD" == rule.virtualType)
    {
        deviceLog.deviceType = CPIDeviceLog::typeDevice;
        deviceLog.accessType = accessWrite;
    }
    
	if(0 < deviceControl.deviceNameList.size())
    {
		std::vector<std::string>::iterator itr = deviceControl.deviceNameList.begin();
		for( ; itr != deviceControl.deviceNameList.end(); ++itr )
        {
            deviceLog.deviceName = *itr;
			deviceLogList.push_back(deviceLog);
		}

		deviceControl.deviceNameList.clear();
	}
	return true;
}


void CPIDeviceController::appendDeviceName( std::string strDeviceName )
{
    deviceControl.deviceNameList.push_back( strDeviceName );
}

void CPIDeviceController::appendDeviceLog( std::string strDeviceName )
{
    CPIDeviceLog::VECTOR deviceLogList;
    
    appendDeviceName( strDeviceName );
    
    getDeviceLog(deviceLogList);
    if(0 < deviceLogList.size())
    {
        CPIDeviceLog::VECTOR::iterator itrLog = deviceLogList.begin();
        for( ; itrLog != deviceLogList.end(); ++itrLog)
        {
            CPIActionProcessDeviceLog::getInstance().addDeviceLog(*itrLog);
        }
    }
}

#endif // #ifndef _DEVICECONTROLLER_CPP
