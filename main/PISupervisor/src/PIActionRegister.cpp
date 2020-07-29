#ifndef	_PIACTIONREGISTER_CPP
#define	_PIACTIONREGISTER_CPP

#include <map>

#include "PIActionRegister.h"
#include "PIDocument.h"
#include "PIEventHandler.h"

////////////////////////////////////////
// CPIActionRegister

CPIActionRegister::CPIActionRegister() {
}

CPIActionRegister::~CPIActionRegister() {	
}

int CPIActionRegister::run(const std::string& param) {

	DEBUG_LOG1("register - begin");

	pid_t pid = atoi(param.c_str());

	PIAgentStub.set("PIAgent", pid, ConfigMan.ttlCycle);

	if( false == PolicyMan.policySet.isOnline ) {
		EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, "online");
	}
	
	DEBUG_LOG1("register - end");
	return 0;
}

int CPIActionRegister::stop(void) {
	DEBUG_LOG1("register - begin");
	DEBUG_LOG1("register - end");
	return 0;
}

CPIActionRegister& CPIActionRegister::getInstance(void) {
	static CPIActionRegister instance;
	return instance;
}
#endif // #ifndef _PIACTIONREGISTER_CPP
