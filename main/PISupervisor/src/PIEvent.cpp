#ifndef	_PIEVENT_CPP
#define	_PIEVENT_CPP

#ifdef LINUX
#include <string>
#endif

#include"PIEvent.h"

////////////////////////////////////////
// CPIEvent
CPIEvent::CPIEvent() {
	clear();
}

CPIEvent::~CPIEvent() {	
}

void CPIEvent::setId(const nsPISupervisor::EM_EVENTID eventId) {
	this->eventId = eventId;
}

nsPISupervisor::EM_EVENTID CPIEvent::getId(void) const {
	return eventId;
}

void CPIEvent::clear(void) {
	CPIParam::clear();	

	eventId = nsPISupervisor::EVENT_NONE;
}
#endif // #ifndef _PIEVENT_CPP
