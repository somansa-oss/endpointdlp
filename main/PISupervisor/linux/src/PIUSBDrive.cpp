#ifndef	_PIUSBDRIVE_CPP 
#define _PIUSBDRIVE_CPP

#ifdef LINUX
#include <string>
#endif

#include "PIUSBDrive.h"
#include "UsbFinder.h"

#ifdef _FIXME

#include "nsMICrypto.h"

#endif

#include "Markup.h"
#include "LogWriter.h"

CPIUSBDrive::CPIUSBDrive() {
}

CPIUSBDrive::~CPIUSBDrive() {
}

int CPIUSBDrive::reload(void) {
	std::vector<USBInfo> usbinfoList;
	UsbFinder usbFinder;
	if (false == usbFinder.Reload(usbinfoList)) {
		return 0;
	}

	for(int index = 0; index < usbinfoList.size(); ++index) {
		USBInfo info = usbinfoList[index];
		CPIUSBDrive usbDrive;
		usbDrive.setMountPoint(info.m_czVolumePath);
		usbDrive.setUsbSerialPlain(info.m_czMediaID);
		list.push_back(usbDrive);
		DEBUG_LOG("usb_volume - mount:%s - usbserial(plain):%s", usbDrive.getMountPoint().c_str(), usbDrive.getUsbSerialPlain().c_str());
	}
	return list.size();
}

std::string CPIUSBDrive::getEncryptedSerial(void) {
	if (0 < usbSerialEncrypted.length()) {
		return usbSerialEncrypted;
	}

	if (0 == usbSerialPlain.length()) {
		return "";
	}

	std::string temp = usbSerialPlain;
	util.replace(temp, "USB\\VID_", "");

	usbSerialEncrypted = encrypt(temp);
	return usbSerialEncrypted;
}

std::string CPIUSBDrive::encrypt(const std::string& plainText) {
	std::string result = "";

#ifdef _FIXME_    

	nsMICrypto::CRC4 rc4;
	rc4.Init();

	const size_t bufferSize = plainText.length()+1;
	char* temp = new char[bufferSize];
	memset(temp, 0x00, bufferSize);
	memcpy(temp, plainText.c_str(), bufferSize);
	int bufferLen = strlen(temp);

	rc4.EnDecrypt((unsigned char*)temp, bufferLen);

	std::string hexEncrypted;
	nsMICrypto::BinToHex((unsigned char*)temp, bufferLen, hexEncrypted);

	delete [] temp;
	temp = NULL;

	return hexEncrypted;
#else
	return "";
#endif	
}

void CPIUSBDrive::setMountPoint(std::string value) {
	if (0 == value.length()) {
		mountPoint = "";
		return;
	}

	util.replace(value, "%20", " ");

	if (std::string::npos == value.find("/Volumes/", 0)) {
		value = "/Volumes/" + value;
	}

	mountPoint = value;
}

void CPIUSBDrive::setUsbSerialPlain(std::string value) {
	if (0 == value.length()) {
		usbSerialPlain = "";
		return;
	}

	usbSerialPlain = CMarkup::UnescapeText(value);
}
#endif
