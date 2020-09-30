#ifndef	_PICLIENTSTUB_CPP
#define _PICLIENTSTUB_CPP

#include <map>

#include "PIClientStub.h"
#include "PIDocument.h"

CPIClientStub::CPIClientStub() { 
	initialize(); 
}

CPIClientStub::~CPIClientStub() {
	finalize(); 
}

bool CPIClientStub::initialize() {
	CPICommunicator::initialize();

#ifndef LINUX
	support.bigEndian = ConfigMan.socket.bigEndian;
	support.savePacket = ConfigMan.socket.savePacket;
	support.savePacketPath = ConfigMan.path.dlpLogPath;
#endif
    
	return true;
}

bool CPIClientStub::finalize() { 
	CPICommunicator::finalize();
	return true;
}
#endif
