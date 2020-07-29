#ifndef	_PIUSBDRIVE_H
#define _PIUSBDRIVE_H

#include "PIObject.h"

////////////////////////////////////////////////////////////////////////////////
//class CPIUSBDrive 
class CPIUSBDrive : public CPIObject {
	public:
		CPIUSBDrive();
		virtual ~CPIUSBDrive();

	public:
		typedef std::vector<CPIUSBDrive> VECTOR;

	private:	
		std::string mountPoint;
		std::string usbSerialPlain;
		std::string usbSerialEncrypted;
	public:
		CPIUSBDrive::VECTOR list;

	private:
		std::string encrypt(const std::string& plainText);

	public:
		int reload(void);
		std::string getEncryptedSerial(void);
		void setMountPoint(std::string value);
		std::string getMountPoint(void) { return mountPoint; };
		void setUsbSerialPlain(std::string value);
		std::string getUsbSerialPlain(void) { return usbSerialPlain; };
};
#endif // _PIUSBDRIVE_H

