#ifndef	_PIACTIONPROCESSTTL_CPP
#define	_PIACTIONPROCESSTTL_CPP

#include <map>

#include "PIActionProcessTTL.h"
#include "PIDocument.h"
#include "PIAgentStub.h"
#include "PIEvent.h"
#include "PIEventHandler.h"

////////////////////////////////////////
// CPIActionProcessTTL

CPIActionProcessTTL::CPIActionProcessTTL() {
}

CPIActionProcessTTL::~CPIActionProcessTTL() {	
}

int CPIActionProcessTTL::run(const std::string& param) {

	DEBUG_LOG1("process ttl - begin");

	CPIEvent event;
	event.setParam(param);
	event.parse();

	std::string processId = event.at(0);
	std::string onOffLine = event.at(1);

	pid_t pid = atoi(processId.c_str());
	if( 0 == pid ) {
		ERROR_LOG1("process ttl - invalid pid");
	}
	else {
		if( PIAgentStub.getProcessId() == pid ) {
			PIAgentStub.pressTimeStamp();
		}
		else {
			PIAgentStub.set("PIAgent", pid, ConfigMan.ttlCycle);
		}
	}

	const bool isOnline = ("online" == onOffLine) ? true : false;
	if( isOnline != PolicyMan.policySet.isOnline ) {
		EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, onOffLine);
	}

	DEBUG_LOG1("process ttl - end");
	return 0;
}

int CPIActionProcessTTL::stop(void) {
	DEBUG_LOG1("process ttl - begin");
	DEBUG_LOG1("process ttl - end");
	return 0;
}

CPIActionProcessTTL& CPIActionProcessTTL::getInstance(void) {
	static CPIActionProcessTTL instance;
	return instance;
}
#endif // #ifndef _PIACTIONPROCESSTTL_CPP
