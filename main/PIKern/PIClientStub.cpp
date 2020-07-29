#ifndef	_PICLIENTSTUB_CPP
#define _PICLIENTSTUB_CPP

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

	return true;
}

bool CPIClientStub::finalize() { 
	CPICommunicator::finalize();
	return true;
}
#endif
