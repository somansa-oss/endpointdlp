#ifndef	_DEVICECONTROLLER_CPP
#define	_DEVICECONTROLLER_CPP

#ifdef LINUX
#include <unistd.h>     // getuid

#ifdef LSF_ENABLED
#include <lsf-api/media-api.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <libudev.h>
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

#ifdef LSF_ENABLED
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
#endif

	return result;
}

bool CPIDeviceControlLinux::controlDevice(const char *media_type, const char *pvi_media_type)
{
	bool result = false;

#ifdef LSF_ENABLED
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
#endif

	return result;
}

bool CPIDeviceControlLinux::controlWLAN(void)
{
#ifdef USE_HANCOMAPI	
	return controlDevice(LSF_MEDIA_WIRELESS, "WLAN");
#else
	bool result = false;

	std::string command, temp;
	command = "nmcli radio wifi ";
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

	if( "enabled" != token)
    {
		return false;
	}
			
	command = "nmcli radio wifi off ";
	DEBUG_LOG( "command:%s", command.c_str());
	system(command.c_str());

	result = true;
	deviceNameList.push_back("WLAN");
	return result;	
#endif	
}

bool CPIDeviceControlLinux::controlWWAN(void)
{
#ifdef LSF_ENABLED
	return controlDevice(LSF_MEDIA_USB_NETWORK, "WWLAN");
#else
	return controlDevice(0, "WWLAN");
#endif
}

bool CPIDeviceControlLinux::controlBluetooth(void)
{
#ifdef USE_HANCOMAPI	
	return controlDevice(LSF_MEDIA_BLUETOOTH, "Bluetooth");
#else
	bool result = false;

	std::string command, temp;
	command = "service bluetooth status | grep Status ";
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

	if( token.find("Status") == std::string::npos )
    {
		return false;
	}
			
	command = "service bluetooth stop ";
	DEBUG_LOG( "command:%s", command.c_str());
	system(command.c_str());

	result = true;
	deviceNameList.push_back("Bluetooth");
	return result;	
#endif		
}

#ifdef LINUX
int get_storage_type(struct udev* udev, char* mnt_fsname)
{
	int ret = BusTypeUnknown;

	struct udev_enumerate* enumerate = NULL;
	if (udev == NULL || mnt_fsname == NULL)
	return ret;

	enumerate = udev_enumerate_new(udev);
	if (enumerate == NULL)
	return ret;

	printf("DEVNAME = %s\n", mnt_fsname);

	udev_enumerate_add_match_property(enumerate, "DEVNAME", mnt_fsname);
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
	struct udev_list_entry *entry = NULL;

	udev_list_entry_foreach(entry, devices) 
	{
		const char* path = udev_list_entry_get_name(entry);
		if (path == NULL)
			continue;

		struct udev_device* parent = udev_device_new_from_syspath(udev, path);
		if (parent == NULL)
			continue;

		const char *id_cdrom = udev_device_get_property_value(parent, "ID_CDROM");
		//printf("\tproperty [ID_CDROM][%s]\n", id_cdrom);

		const char *devtype = udev_device_get_property_value(parent, "DEVTYPE");
		//printf("\tproperty [DEVTYPE][%s]\n", devtype);

		if (id_cdrom != NULL)
		{
			ret = BusTypeAtapi;
		}
		else if (devtype != NULL)
		{
			ret = BusTypeUsb;
		}

		udev_device_unref(parent);

		break;
	}

	udev_enumerate_unref(enumerate);

	if (ret == BusTypeUnknown)
	{
	#ifdef WSL
		ret = BusTypeUsb;
	#else
		ret = BusTypeSFolder;
	#endif      
	}

  return ret;
}
#endif

bool CPIDeviceControlLinux::controlRemovableStorage(void)
{
#ifdef LINUX
	bool result = false;
	{
		int       nBusType = 0;
		boolean_t bUsbStor = FALSE;
		boolean_t bCDStor  = FALSE;
		boolean_t bSFolder = FALSE;
		boolean_t bTBStor  = FALSE;

		struct mntent *ent = NULL;
		FILE *aFile = NULL;
		struct udev* udev = NULL;

		aFile = setmntent("/proc/mounts", "r");
		if (aFile == NULL) 
		{
			printf("[DLP][%s] setmntent() failed \n", __FUNCTION__);
			return -1;
		}

		udev = udev_new();
		if (udev == NULL)
		{
			printf("[DLP][%s] udev_new() failed \n", __FUNCTION__);
			endmntent(aFile);
			return -1;
		}

		while (NULL != (ent = getmntent(aFile))) 
		{
			// e.g.
			// f_mntonname     /media/somansa/PRIVACY-I
			// f_mntfromname   /dev/sdb1
			//        
			// ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, ent->mnt_opts
			//  /dev/sdb1  /media/somansa/PRIVACY-I  vfat  rw,nosuid,nodev,relatime,uid=1001,gid=1001,fmask=0022,dmask=0022,codepage=437,iocharset=ascii,shortname=mixed,showexec,utf8,flush,errors=remount-ro
			//  /dev/sda3 / ext4  rw,relatime,errors=remount-ro 
	#ifdef WSL
			if (strstr(ent->mnt_dir, "/dev/") == NULL && strstr(ent->mnt_dir, "/mnt/") == NULL)
	#else
			if (strstr(ent->mnt_fsname, "/dev/") == NULL && strstr(ent->mnt_fsname, "/mnt/") == NULL)
	#endif
			{
	#ifdef WSL
				if (ent->mnt_fsname[0] == '/' && ent->mnt_fsname[1] == '/')
	#else            
				if (ent->mnt_dir[0] == '/' && ent->mnt_dir[1] == '/')
	#endif            
				{
					// //192.168.181.1/tmp
				}
				else
				{
					continue;
				}
			}
			else
			{
	#ifdef WSL
	#else
				// /dev/sda3  /  
				if (ent->mnt_dir[0] == '/' && ent->mnt_dir[1] == 0)
				{
					continue;
				}
	#endif            
			}

	#ifdef WSL
			if (strlen(ent->mnt_dir) <= 1)
	#else        
			if (strlen(ent->mnt_fsname) <= 1)
	#endif        
			{
				continue;
			}
			int type = 0;

			type = get_storage_type(udev, ent->mnt_fsname);
			if (BusTypeUsb == type)
			{
				printf("Removable Storage: %s %s %s %s \n", ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, ent->mnt_opts);

				std::string command;
				command = "eject ";
				command += ent->mnt_fsname;
				DEBUG_LOG( "command:%s", command.c_str());
				system(command.c_str());

				result = true;
				deviceNameList.push_back( ent->mnt_dir );
				break;
			}
			else
			{
				//printf("Not Removable Storage: %s %s %s %s \n", ent->mnt_fsname, ent->mnt_dir, ent->mnt_type, ent->mnt_opts);
			}
		}
		endmntent(aFile);
		udev_unref(udev);		
	}
	return result;	

#endif
}

bool CPIDeviceControlLinux::controlWriteCDDVD(void)
{
#ifdef USE_HANCOMAPI	
	return controlDevice(LSF_MEDIA_CD_DVD, "CD/DVD");
#else
	bool result = false;

	std::string command, temp;
	command = "blkid -i /dev/dvd";
	command += " 2>&1";
	system(command.c_str());

	temp = util.readCommandOutput(command);

	DEBUG_LOG( "command:%s", command.c_str());
	DEBUG_LOG( "[result] - %s", temp.c_str());

	if( false == temp.empty()
		&& std::string::npos == temp.find("error:") ) 
	{
		command = "eject -v -s /dev/dvd ";
		system(command.c_str());

		result = true;
		deviceNameList.push_back("CD/DVD");
	}
	else
	{
		command = "blkid -i /dev/cdrom";
		command += " 2>&1";
		system(command.c_str());

		temp = util.readCommandOutput(command);

		DEBUG_LOG( "command:%s", command.c_str());
		DEBUG_LOG( "[result] - %s", temp.c_str());

		if( false == temp.empty()
			&& std::string::npos == temp.find("error:") ) 
		{
			command = "eject -v -s /dev/cdrom ";
			system(command.c_str());

			result = true;
			deviceNameList.push_back("CD/DVD");
		}
	}

	return result;	
#endif	
}

bool CPIDeviceControlLinux::enableWLAN(void)
{
#ifdef USE_HANCOMAPI		
	return enableDevice(LSF_MEDIA_WIRELESS);
#else
	return true;
#endif	
}

bool CPIDeviceControlLinux::enableWWLAN(void)
{
#ifdef LSF_ENABLED
	return enableDevice(LSF_MEDIA_USB_NETWORK);
#else
	return enableDevice(0);
#endif
}

bool CPIDeviceControlLinux::enableBluetooth()
{
#ifdef USE_HANCOMAPI	
	return enableDevice(LSF_MEDIA_BLUETOOTH);
#else
	return true;
#endif	
}

bool CPIDeviceControlLinux::enableWriteCDDVD()
{
#ifdef USE_HANCOMAPI		
	return enableDevice(LSF_MEDIA_CD_DVD);
#else
	return true;
#endif	
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

#ifdef LINUX
	if (false == rule.disableAll && "Device\\CD/DVD" != rule.virtualType && "Drive\\Removable" != rule.virtualType) {
#else
	if (false == rule.disableAll && "Device\\CD/DVD" != rule.virtualType) {
#endif		
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
#ifdef LINUX
	else if ("Drive\\Removable" == rule.virtualType)
	{
		result = deviceControl.controlRemovableStorage();
	}
#endif

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
#ifdef LINUX	
    if("Drive\\Removable" == rule.virtualType)
    {
        deviceLog.deviceType = CPIDeviceLog::typeDrive;
        deviceLog.accessType = accessRead;
		deviceLog.virtualType = "Drive\\RemovableMedia";
    }
#endif

	if(0 < deviceControl.deviceNameList.size())
    {
		std::vector<std::string>::iterator itr = deviceControl.deviceNameList.begin();
		for( ; itr != deviceControl.deviceNameList.end(); ++itr )
        {
            deviceLog.deviceName = *itr;
#ifdef LINUX	
			if("Drive\\Removable" == rule.virtualType)
			{			
				deviceLog.fileName = deviceLog.deviceName;
			}
#endif			
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
