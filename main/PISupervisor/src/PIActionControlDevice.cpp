#ifndef	_PIACTIONCONTROLDEVICE_CPP
#define	_PIACTIONCONTROLDEVICE_CPP

#include <map>

#ifdef LINUX
#include <string>
#endif

#include "PIActionControlDevice.h"
#include "PIDocument.h"

#ifndef LINUX
#include "PIDRDevice.h"
#endif

////////////////////////////////////////
// CPIActionControlDevice

CPIActionControlDevice::CPIActionControlDevice() {
	clear();
}

CPIActionControlDevice::~CPIActionControlDevice() {	
}

void CPIActionControlDevice::clear(void) {
	CPIObject::clear();
	
	isContinue = true;
	running = false;
}

bool CPIActionControlDevice::initialize(void) {
	DEBUG_LOG1("control device - begin");
	
	CPIObject::initialize();

	DEBUG_LOG1("control device - end");
	
	return true;
}

bool CPIActionControlDevice::finalize(void) {
	DEBUG_LOG1("control device - begin");
	
	DEBUG_LOG1("control device - end");
	
	return CPIObject::finalize();
}

int CPIActionControlDevice::run(const std::string& param) {

	DEBUG_LOG1("control device - begin");
	INFO_LOG1("connect to driver");
    
    if( true == isRunning() ) {
        DEBUG_LOG1("control device - skip - already running");
        return 0;
    }

    initialize();
	if( false == startThreads() ) {
		ERROR_LOG1("control device - create_thread failed");
	}

	DEBUG_LOG1("control device - end");
	return 0;
}

bool CPIActionControlDevice::startThreads(void) {

	DEBUG_LOG1("control device - begin");
	
	int result = 0;

	if( false == isRunning() ) {
		pthread_attr_init( &deviceThreadAttr);
		pthread_attr_setscope( &deviceThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &deviceThread, &deviceThreadAttr, CPIActionControlDevice::fnControlDevice, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for log" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("control device - skip - already running");
	}

	DEBUG_LOG1("control device - end");
	return true;
}

void CPIActionControlDevice::waitThreads(void) {

	DEBUG_LOG1("control device - begin");

	if( true == isRunning() ) {
		pthread_join( deviceThread, (void**)NULL );
	}
	
	DEBUG_LOG1("control device - end");
}

int CPIActionControlDevice::stop(void) {
	DEBUG_LOG1("control device - begin");
	
	isContinue = false;

	waitThreads();
    finalize();

	DEBUG_LOG1("control device - end");
	return 0;
}

CPIActionControlDevice& CPIActionControlDevice::getInstance(void) {
	static CPIActionControlDevice instance; 
	return instance;
}

void* CPIActionControlDevice::fnControlDevice(void* pzArg)
{
	DEBUG_LOG1("control device - begin");
    
	CPIActionControlDevice* instance = reinterpret_cast<CPIActionControlDevice*>(pzArg);
	instance->running = true;

#ifndef LINUX
    g_DRDevice.MediaDRDeviceStartThread();
#endif

	while(instance->isContinue && (false == Doc.getStop()))
    {
		sleep(3);
		DeviceMan.controlDevice();
		DeviceMan.monitorVolums();
	}
    
	instance->running = false;
	DEBUG_LOG1("control device - end");
	return NULL;
}

bool CPIActionControlDevice::isRunning(void) {
	return running;
}
#endif
