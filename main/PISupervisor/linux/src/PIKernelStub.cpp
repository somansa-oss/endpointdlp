#ifndef	_PIKERNELSTUB_CPP
#define _PIKERNELSTUB_CPP

#ifdef LINUX
#include <string>
#endif

#include "PIKernelStub.h"

////////////////////////////////////////////////////////////////////////////////
//class IPIKernelStub
IPIKernelStub::IPIKernelStub() { 
	result = 0;
	initialized = false;
}

IPIKernelStub::~IPIKernelStub() {
}
#endif

