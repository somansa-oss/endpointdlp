#ifndef	_PIACTIONHEARTBEATCHECK_CPP
#define	_PIACTIONHEARTBEATCHECK_CPP

#include <map>

#ifdef LINUX
#include <string>
#endif

#include "PIActionHeartbeatCheck.h"
#include "PIDocument.h"
#include "PIEventHandler.h"

////////////////////////////////////////
// CPIActionHeartbeatCheck

CPIActionHeartbeatCheck::CPIActionHeartbeatCheck() {
	clear();
}

CPIActionHeartbeatCheck::~CPIActionHeartbeatCheck() {	
}

int CPIActionHeartbeatCheck::run(const std::string& param) {

	DEBUG_LOG1("heartbeat_check - begin");

	if( true == isRunning() ) {
		DEBUG_LOG1("heartbeat_check - skipped - already running");
		return 0;
	}

	clear();
	if( false == startThreads() ) {
		ERROR_LOG1("heartbeat_check - create_thread failed");
	}

	DEBUG_LOG1("heartbeat_check - end");
	return 0;
}

bool CPIActionHeartbeatCheck::startThreads(void) {

	DEBUG_LOG1("heartbeat_check - begin");
	
	int result = 0;

	pthread_attr_init( &heartbeatThreadAttr);
	pthread_attr_setscope( &heartbeatThreadAttr, PTHREAD_SCOPE_SYSTEM );
	result = pthread_create( &heartbeatThread, &heartbeatThreadAttr, CPIActionHeartbeatCheck::fnHeartbeatCheck, (void*)this);
	if( result ) {
		ERROR_LOG1( "thread_create failed" );
		return false;
	}

	DEBUG_LOG1("heartbeat_check - end");
	return true;
}

void CPIActionHeartbeatCheck::waitThreads(void) {

	DEBUG_LOG1("heartbeat_check - begin");

	pthread_join( heartbeatThread, (void**)NULL );
	
	DEBUG_LOG1("heartbeat_check - end");
}

int CPIActionHeartbeatCheck::stop(void) {
	DEBUG_LOG1("heartbeat_check - begin");

	isContinue = false;
	if( true == isRunning() ) {
		waitThreads();
	}

	DEBUG_LOG1("heartbeat_check - end");
	return 0;
}

CPIActionHeartbeatCheck& CPIActionHeartbeatCheck::getInstance(void) {
	static CPIActionHeartbeatCheck instance;
	return instance;
}

void* CPIActionHeartbeatCheck::fnHeartbeatCheck(void* pzArg) {
	DEBUG_LOG1("heartbeat_check - begin");
	
	CPIActionHeartbeatCheck* instance = reinterpret_cast<CPIActionHeartbeatCheck*>(pzArg);
	instance->running = true;

	while(instance->isContinue && (false== Doc.getStop())) {
			//DEBUG_LOG1("loop");
		sleep(1);

		int ttlCycle = ConfigMan.ttlCycle;
		if( 0 >= ttlCycle ) {
			continue;
		}
		if( ++(instance->ttlCycleCheck) < ttlCycle ) {
			continue;
		}

		instance->ttlCycleCheck = 0;

		bool needOfflinePolicy = true;
		if( false == PIAgentStub.isRegistered() ) {
			//DEBUG_LOG1("heartbeat_check - piagent not registered");
			//INFO_LOG1("piagent not registered");
		}
		else if( true == PIAgentStub.isOutDated() ) {
			DEBUG_LOG("heartbeat_check - piagent(%d) is outdated", PIAgentStub.getProcessId());
			INFO_LOG("piagent(%d) is outdated", PIAgentStub.getProcessId());

			PIAgentStub.clear();
			//DEBUG_LOG("piagent(%d) - clear ", PIAgentStub.getProcessId());
		}
		else {
			needOfflinePolicy = false;
		}

		if( ( true == needOfflinePolicy ) && ( true == PolicyMan.policySet.isOnline ) ) {
			EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, "offline");
		}
	}

	instance->running = false;
	DEBUG_LOG1("heartbeat_check - end");
	return NULL;
}

void CPIActionHeartbeatCheck::clear(void) {
	CPIObject::clear();

	ttlCycleCheck = 0;
	isContinue = true;
	running = false;
}

bool CPIActionHeartbeatCheck::isRunning(void) {
	return running;
}
#endif // #ifndef _PIACTIONHEARTBEATCHECK_CPP
