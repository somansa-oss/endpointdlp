#ifndef	_PISECFILTERDEVICESTUB_CPP
#define _PISECFILTERDEVICESTUB_CPP

#include"PISecFilterDeviceStub.h"

CPISecFilterDeviceStub::CPISecFilterDeviceStub() { 
}

CPISecFilterDeviceStub::~CPISecFilterDeviceStub() {
}

bool CPISecFilterDeviceStub::initialize(KERNEL_EVENTHANDLER kernel_eventhandler) {
	return true;
}

bool CPISecFilterDeviceStub::finalize(void) {
	return true;
}

void CPISecFilterDeviceStub::clear(void) {
}

bool CPISecFilterDeviceStub::applyPolicy(unsigned long command, void* in, unsigned long size) {
	return true;
}

void CPISecFilterDeviceStub::load(void) {
}

void CPISecFilterDeviceStub::unload(void) {
}

bool CPISecFilterDeviceStub::isActive(void) {
	return false;
}
#endif
