#ifndef	_DLPAPPLICATION_CPP
#define	_DLPAPPLICATION_CPP

#ifdef LINUX
#include <string>
#endif

#include "DLPApplication.h"

////////////////////////////////////////////////////////////////////////////////
//class CDLPApplication
CDLPApplication::CDLPApplication() {
	clear();
}

CDLPApplication::~CDLPApplication() {
}

void CDLPApplication::clear(void) {
	CPIObject::clear();

	type = CDLPApplication::typeCustom;
	controlType = CDLPApplication::controltypeNone;
	fileOpenControlType = CDLPApplication::fileopencontroltypeApplicationHooking;
	guid = "";
	display = "";
	fileName = "";
	binData = "";
	useBinData = false;
}

void CDLPApplication::setType(std::string type) {
	this->type = static_cast<CDLPApplication::EM_TYPE>(atoi(type.c_str()));
}
		
bool CDLPApplication::isSameProcessName(std::string processName) const {
	if (0 == processName.length()) {
		return false;
	}
	
	std::string lowerProcessName = processName;
	util.toLower(lowerProcessName);
	
	if (lowerProcessName == lowerFileName) {
		return true;
	}

	return false;
}

bool CDLPApplication::containBinary(int fd, void(*logCallback)(std::string& log)) const {
	const unsigned int BUFFER_SIZE = 1024*8;
	
	if (0 > fd) {
		return false;
	}

	CBufferType buffer;
	CBufferType pattern;
	bool result = false;
	do {
		hexToBin(pattern, binData);
		if (false == pattern.isValid()) {
			break;
		}
		
		if (false == buffer.initialize(BUFFER_SIZE)) {
			break;
		}
		
		const off_t fileLength = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		int readSum = 0;
		do {
			buffer.length = read(fd, buffer.data, BUFFER_SIZE);

			if (0 >= buffer.length ) {
				break;
			}
			
			readSum += buffer.length;
/*
			if (NULL != logCallback) {
				char szLog[1024] = {0x00,}; sprintf(szLog, "[fd:%d] read:%d - sum:%d - file_len:%lld", fd, (int) buffer.length, readSum, fileLength );
				std::string log = szLog;
				logCallback(log);
			}
*/
			result = containBinary(fd, buffer, pattern, logCallback);
/*			
			if (NULL != logCallback) {
				char szLog[1024] = {0x00,}; sprintf(szLog, "[fd:%d] result:%s", fd, (result)?"true":"false");
				std::string log = szLog;
				logCallback(log);
			}
*/
		} while ((fileLength > readSum) && (false == result));
	} while (false);

	return result;
}

size_t CDLPApplication::hexToBin(CBufferType& buffer, const std::string& binHex) const {
	size_t hexLength = binHex.length();
	if (2 > hexLength) {
		return false;
	}

	buffer.initialize(hexLength/2);
	for ( size_t index = 0; index < buffer.allocSize; ++index) {
		unsigned char c1 = x2c(binHex.at(index * 2));
		unsigned char c2 = x2c(binHex.at(index * 2 + 1));
		buffer.data[index] = (c1 << 4) | c2;
	}
	buffer.length = buffer.allocSize;
	return buffer.length;
}

unsigned char CDLPApplication::x2c(unsigned char c) const { /*0~9, A~F, a~f*/
	if ((c >= '0') && (c <= '9')) {
		c -= '0';
	} else if ((c >= 'a') && (c <= 'f')) {
		c = c - 'a' + 10;
	} else if ((c >= 'A') && (c <= 'F')) {
		c = c - 'A' + 10;
	} else {
		c = 16;
	}

	return c;
}

bool CDLPApplication::containBinary(int fd, const CBufferType& buffer, const CBufferType& pattern, void(*logCallback)(std::string& log)) const {
	int curSize = 0;
	char* source = buffer.data;
	/*	
	if (NULL != logCallback) {
			char szLog[1024] = {0x00,}; sprintf(szLog, "[fd:%d] before - curSize:%d - buf_len:%d - pattern_len:%d", fd, curSize, (int) buffer.length, (int) pattern.length);
			std::string log = szLog;
			logCallback(log);
		}
*/
	while (curSize < buffer.length) {
		char* dest = pattern.data;	

		int tempPos = 0;


		while (true) {
		/*	
			if (NULL != logCallback) {
				char szLog[1024] = {0x00,}; sprintf(szLog, "%s(%d) [fd:%d] curSize:%d - buf_len:%d - tempPos:%d - pattern_len:%d", __FUNCTION__, __LINE__, fd, curSize, (int) buffer.length, tempPos, (int) pattern.length);
				std::string log = szLog;
				logCallback(log);
			}
	*/	
			if (curSize >= buffer.length) {
				break;
			}

			if (tempPos >= pattern.length) {
				break;
			}


			if (*source != *dest) {
				break;
			}

			source++;
			curSize++;
			dest++;
			tempPos++;

		}	


		if (tempPos == pattern.length) {
			return true; // source - pattern.length;
		}

		source = source - tempPos + 1;
		curSize = curSize - tempPos + 1;
	}
/*	
	if (NULL != logCallback) {
			char szLog[1024] = {0x00,}; sprintf(szLog, "[fd:%d] after - curSize:%d - buf_len:%d - pattern_len:%d", fd, curSize, (int) buffer.length, (int) pattern.length);
			std::string log = szLog;
			logCallback(log);
		}
*/		

	return false;
}

void CDLPApplication::setFileName(std::string fileName) {
	this->fileName = fileName;

	if (0 < fileName.length()) {
		lowerFileNameExt = fileName;
		util.toLower(lowerFileNameExt);
		lowerFileName = util.getFileName(lowerFileNameExt);
		lowerFileExt = util.getFileExt(lowerFileNameExt);
	}
}

bool CDLPApplication::isWindowsExecutable(void) const {
	if (0 == lowerFileExt.length()) {
		return false;
	}

	return ("exe" == lowerFileExt)?true:false;
}
#endif // #ifndef _DLPAPPLICATION_CPP
