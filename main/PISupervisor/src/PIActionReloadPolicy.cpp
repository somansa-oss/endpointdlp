#ifndef	_PIACTIONRELOADPOLICY_CPP
#define	_PIACTIONRELOADPOLICY_CPP

#include <map>

#include "PIActionReloadPolicy.h"
#include "PIDocument.h"

////////////////////////////////////////
// CPIActionReloadPolicy

CPIActionReloadPolicy::CPIActionReloadPolicy() {
}

CPIActionReloadPolicy::~CPIActionReloadPolicy() {	
}

int CPIActionReloadPolicy::run(const std::string& param) {

	DEBUG_LOG1("reload_policy - begin");
	INFO_LOG1("reload policy");

	PolicyMan.load();
	PolicyMan.translatePolicy();
	PolicyMan.adjust();
	PolicyMan.reloadMediaList();
	DeviceMan.reloadUSBSerial();

	DEBUG_LOG1("reload_policy - end");
	return 0;
}

int CPIActionReloadPolicy::stop(void) {
	DEBUG_LOG1("reload_policy - begin");
	DEBUG_LOG1("reload_policy - end");
	return 0;
}

CPIActionReloadPolicy& CPIActionReloadPolicy::getInstance(void) {
	static CPIActionReloadPolicy instance;
	return instance;
}
#endif // #ifndef _PIACTIONRELOADPOLICY_CPP
