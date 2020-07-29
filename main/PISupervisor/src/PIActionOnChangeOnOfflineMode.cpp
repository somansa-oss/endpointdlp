#ifndef	_PIACTIONONCHANGEONOFFLINEMODE_CPP
#define	_PIACTIONONCHANGEONOFFLINEMODE_CPP

#include <map>

#include "PIActionOnChangeOnOfflineMode.h"
#include "PIDocument.h"

////////////////////////////////////////
// CPIActionOnChangeOnOfflineMode

CPIActionOnChangeOnOfflineMode::CPIActionOnChangeOnOfflineMode() {
}

CPIActionOnChangeOnOfflineMode::~CPIActionOnChangeOnOfflineMode() {	
}

int CPIActionOnChangeOnOfflineMode::run(const std::string& param) {
	DEBUG_LOG1("on_change_onofline_mode - begin");
	DEBUG_LOG("on_change_onofline_mode - %s", param.c_str());
	INFO_LOG("policy mode changed to %s", param.c_str());

	if( "online" == param ) {
		PolicyMan.setOnlineMode();
	}
	else {
		PolicyMan.setOfflineMode();
	}

	PolicyMan.translatePolicy();

	DEBUG_LOG1("on_change_onofline_mode - end");
	return 0;
}

int CPIActionOnChangeOnOfflineMode::stop(void) {
	DEBUG_LOG1("on_change_onofline_mode - begin");
	DEBUG_LOG1("on_change_onofline_mode - end");
	return 0;
}

CPIActionOnChangeOnOfflineMode& CPIActionOnChangeOnOfflineMode::getInstance(void) {
	static CPIActionOnChangeOnOfflineMode instance;
	return instance;
}
#endif // #ifndef _PIACTIONONCHANGEONOFFLINEMODE_CPP
