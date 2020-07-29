#ifndef	_PIACTIONINITIALIZEDATA_CPP
#define	_PIACTIONINITIALIZEDATA_CPP

#include <map>

#include "PIActionInitializeData.h"
#include "PIDocument.h"

////////////////////////////////////////
// CPIActionInitializeData

CPIActionInitializeData::CPIActionInitializeData() {
}

CPIActionInitializeData::~CPIActionInitializeData() {	
}

int CPIActionInitializeData::run(const std::string& param) {
	DEBUG_LOG1("initialize_data - begin");
	INFO_LOG1("initialize data");
	
	if( false == startThreads() ) {
		ERROR_LOG1("initialize_data - create_thread failed");
	}
	
	DeviceMan.initialize();

	DEBUG_LOG1("initialize_data - end");

	return 0;
}

int CPIActionInitializeData::stop(void) {
	DEBUG_LOG1("initialize_data - begin");

	waitThreads();
	finalize();

	DEBUG_LOG1("initialize_data - end");
	return 0;
}

CPIActionInitializeData& CPIActionInitializeData::getInstance(void) {
	static CPIActionInitializeData instance;
	return instance;
}

bool CPIActionInitializeData::startThreads(void) {

	DEBUG_LOG1("initialize_data_thread - begin");

	int result = 0;

	if( false == isRunningWallManThread() ) {
		pthread_attr_init( &wallManThreadAttr);
		pthread_attr_setscope( &wallManThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &wallManThread, &wallManThreadAttr, CPIActionInitializeData::fnInitializeWallMan, (void*)this);
		if( result ) {
			ERROR_LOG1( "unable to initialize wall_man thread" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("initialize_data_thread - skip - already running - client thread");
	}

	DEBUG_LOG1("initialize_data_thread - end");
	return true;
}

void CPIActionInitializeData::waitThreads(void) {

	DEBUG_LOG1("initialize_data_thread - begin");

	if( true == isRunningWallManThread() ) {
		pthread_join( wallManThread, (void**)NULL );
	}
	
	DEBUG_LOG1("initialize_data_thread - end");
}

void* CPIActionInitializeData::fnInitializeWallMan(void* pzArg) {
	DEBUG_LOG1("initialize_data_thread - begin");
	CPIActionInitializeData* instance = reinterpret_cast<CPIActionInitializeData*>(pzArg);
	instance->runningWallManThread = true;

#ifndef LINUX
	WallMan.initialize();
#endif
	
	instance->runningWallManThread = false;
	DEBUG_LOG1("initialize_data_thread - end");
	return NULL;
}

bool CPIActionInitializeData::isRunningWallManThread(void) {
	return runningWallManThread;
}
#endif // #ifndef _PIACTIONINITIALIZEDATA_CPP
