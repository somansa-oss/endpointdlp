#ifndef	_PIACTIONUNREGISTER_CPP
#define	_PIACTIONUNREGISTER_CPP

#include <map>

#ifdef LINUX
#include <string>
#endif

#include "PIActionUnregister.h"
#include "PIDocument.h"
#include "PIEventHandler.h"

////////////////////////////////////////
// CPIActionUnregister

CPIActionUnregister::CPIActionUnregister() {
}

CPIActionUnregister::~CPIActionUnregister() {	
}

int CPIActionUnregister::run(const std::string& param) {

	DEBUG_LOG1("unregister - begin");

	//pid_t pid = atoi(param.c_str());

	PIAgentStub.clear();

	if( true == PolicyMan.policySet.isOnline ) {
		EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, "offline");
	}

	DEBUG_LOG1("unregister - end");
	return 0;
}

int CPIActionUnregister::stop(void) {
	DEBUG_LOG1("unregister - begin");
	DEBUG_LOG1("unregister - end");
	return 0;
}

CPIActionUnregister& CPIActionUnregister::getInstance(void) {
	static CPIActionUnregister instance;
	return instance;
}
#endif
