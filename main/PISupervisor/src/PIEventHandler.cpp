#ifndef	_PIEVENTHANDLER_CPP
#define	_PIEVENTHANDLER_CPP

#ifdef LINUX
#include <string>
#endif

#include "PIEventHandler.h"
#include "PIDocument.h"
#include "PIActionInitializeData.h"
#include "PIActionOpenClientIPC.h"
#include "PIActionProcessDeviceLog.h"
#include "PIActionReloadPolicy.h"
#include "PIActionApplyPolicy.h"
#include "PIActionOnChangeOnOfflineMode.h"
#include "PIActionRegister.h"
#include "PIActionUnregister.h"
#include "PIActionHeartbeatCheck.h"
#include "PIActionProcessTTL.h"
#include "PIActionCheckAgent.h"
#include "PIActionControlDevice.h"

////////////////////////////////////////
// CPIEventHandler

CPIEventHandler::CPIEventHandler() {
	initialize();
}

CPIEventHandler::~CPIEventHandler() {	
	finalize();
}

void CPIEventHandler::run(void) {	

	DEBUG_LOG1("event_handler");

	ADD_PISEC_EVENT(nsPISupervisor::EVENT_INIT_PISUPERVISOR, "" );

	CPIEventHandler::VECTOR_EVENT vectorTemp;
	while(isContinue) {
	
		pthread_mutex_lock( &mutexEvent);
		DEBUG_LOG1("event_handler - event wait...");
		
		while(STATE_SIG != stateEvent) {
			/* int err = */ pthread_cond_wait(&condEvent, &mutexEvent);
            //DEBUG_LOG("event_handler - pthread_cond_wait - %d", err);
		}

		if(false == isContinue) {
			break;
		}
		
		DEBUG_LOG1("event_handler - event received...");

		size_t eventCount = vectorEvent.size();
		if( 0 < eventCount ) {
			vectorTemp.clear();
			vectorTemp.resize(eventCount);
			std::copy(vectorEvent.begin(), vectorEvent.end(), vectorTemp.begin());
			vectorEvent.clear();
		}
		stateEvent = STATE_WAIT;
		pthread_mutex_unlock( &mutexEvent);

		if( 0 == eventCount ) {
			continue;
		}

		for( size_t index = 0; index < eventCount; index++) {
			const CPIEvent & event = vectorTemp[index];

			MAP_HANDLER::iterator itr = mapHandler.find(event.getId());
			if( itr != mapHandler.end() ) {
				itr->second(&event);
			}
		}
		vectorTemp.clear();
	}

	DEBUG_LOG1("event_handler - end");
}

void CPIEventHandler::stopAllAction(void) {
	DEBUG_LOG1("event_handler");
	
	CPIDocument::getInstance().setStop();

	STOP_PISEC_EVENT(CPIActionInitializeData);
	STOP_PISEC_EVENT(CPIActionOpenClientIPC);
	STOP_PISEC_EVENT(CPIActionProcessDeviceLog);
	STOP_PISEC_EVENT(CPIActionReloadPolicy);
	STOP_PISEC_EVENT(CPIActionApplyPolicy);
	STOP_PISEC_EVENT(CPIActionOnChangeOnOfflineMode);
	STOP_PISEC_EVENT(CPIActionRegister);
	STOP_PISEC_EVENT(CPIActionUnregister);
	STOP_PISEC_EVENT(CPIActionHeartbeatCheck);
	STOP_PISEC_EVENT(CPIActionProcessTTL);
	STOP_PISEC_EVENT(CPIActionCheckAgent);
	STOP_PISEC_EVENT(CPIActionControlDevice);
	
	CPIDocument::getInstance().finalize();
}

void CPIEventHandler::stop(void) {
	DEBUG_LOG1("event_handler");
	
	stopAllAction();
	isContinue = false;
	stateEvent = STATE_SIG;
	pthread_cond_signal(&condEvent);
	
	DEBUG_LOG1("event_handler - end");
}

bool CPIEventHandler::initialize(void) {
	DEBUG_LOG1("event_handler");
	CPIObject::initialize();

	pthread_mutex_init( &mutexEvent, 0 );
	pthread_cond_init( &condEvent, 0 );
	stateEvent = STATE_WAIT;
	isContinue = true;

	// ----------
	ON_PISEC_EVENT(nsPISupervisor::EVENT_INIT_PISUPERVISOR, &CPIEventHandler::onInitPISupervisor);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_UNINIT_PISUPERVISOR, &CPIEventHandler::onUninitPISupervisor);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_UNREGISTER, &CPIEventHandler::onUnregsiter);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, &CPIEventHandler::onChangedOnOfflineMode);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_CHANGED_DLPPOLICY, &CPIEventHandler::onChangedDLPPolicy);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_CHANGED_DEVICE, &CPIEventHandler::onChangedDevice);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_EXECUTE, &CPIEventHandler::onExecute);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_TTL, &CPIEventHandler::onTTL);
	ON_PISEC_EVENT(nsPISupervisor::EVENT_REGISTER, &CPIEventHandler::onRegsiter);
	// ----------

	return true;
}

bool CPIEventHandler::finalize(void) {
	DEBUG_LOG1("event_handler");
	
	mapHandler.clear();
	pthread_mutex_unlock( &mutexEvent); 
	pthread_mutex_destroy( &mutexEvent); 
	pthread_cond_destroy( &condEvent); 
	
	CPIObject::finalize();

	return true;
}

CPIEventHandler & CPIEventHandler::getInstance(void) {
	static CPIEventHandler instance;
	return instance;
}

void CPIEventHandler::addEvent(const CPIEvent& event) {
	DEBUG_LOG("event_id - %d", event.getId());

    pthread_mutex_lock(&mutexEvent);
	stateEvent = STATE_SIG;
	this->vectorEvent.push_back(event);
	pthread_mutex_unlock(&mutexEvent);
    pthread_cond_signal(&condEvent);

}

void CPIEventHandler::addEvent(const nsPISupervisor::EM_EVENTID eventId, const std::string& param) {
	CPIEvent event;
	event.setId(eventId);
	event.setParam(param);
	addEvent(event);
}

void CPIEventHandler::setEventHandler(const nsPISupervisor::EM_EVENTID eventId, EVENT_HANDLER fnEventHandler) {
	mapHandler[eventId] = fnEventHandler;
}

void CPIEventHandler::onInitPISupervisor(const CPIEvent* pEvent) {

	DEBUG_LOG1("begin");
	INFO_LOG1("PISupervisor initialize event received");

	RUN_PISEC_EVENT(CPIActionInitializeData, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionOpenClientIPC, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionProcessDeviceLog, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionReloadPolicy, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionApplyPolicy, pEvent->getParam());
	//RUN_PISEC_EVENT(CPIActionStopRDPService, pEvent->getParam());
	//RUN_PISEC_EVENT(CPIActionDriveManagement, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionCheckAgent, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionControlDevice, pEvent->getParam());

	DEBUG_LOG1("end");

}

void CPIEventHandler::onUninitPISupervisor(const CPIEvent* pEvent) {

	INFO_LOG1("PISupervisor uninitialize event received");

	//RUN_PISEC_EVENT(CPIActionUninitializeData, pEvent->getParam());

}

void CPIEventHandler::onUnregsiter(const CPIEvent* pEvent) {

	INFO_LOG1("PIAgent unregister event received");

	RUN_PISEC_EVENT(CPIActionUnregister, pEvent->getParam());
	STOP_PISEC_EVENT(CPIActionHeartbeatCheck);
}

void CPIEventHandler::onChangedOnOfflineMode(const CPIEvent* pEvent) {

	INFO_LOG1("policy on/offline changed event received");
	RUN_PISEC_EVENT(CPIActionOnChangeOnOfflineMode, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionApplyPolicy, pEvent->getParam());
}

void CPIEventHandler::onChangedDLPPolicy(const CPIEvent* pEvent) {

	INFO_LOG1("dlp policy changed event received");
	RUN_PISEC_EVENT(CPIActionReloadPolicy, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionApplyPolicy, pEvent->getParam());
}

void CPIEventHandler::onChangedDevice(const CPIEvent* pEvent) {
	INFO_LOG1("device changed event received");

	//RUN_PISEC_EVENT(CPIActionOnChangedDevice, pEvent->getParam());
	//RUN_PISEC_EVENT(CPIActionDrivemanagement, pEvent->getParam());
	RUN_PISEC_EVENT(CPIActionApplyPolicy, pEvent->getParam());
}

void CPIEventHandler::onExecute(const CPIEvent* pEvent) {
	INFO_LOG1("execute event received");
}

void CPIEventHandler::onTTL(const CPIEvent* pEvent) {
	INFO_LOG1("PIAgent TTL event received");

	RUN_PISEC_EVENT(CPIActionProcessTTL, pEvent->getParam());
}

bool CPIEventHandler::onSyncRegister(const std::string& param) {
	INFO_LOG1("PIAgent register msg received");

	RUN_PISEC_EVENT(CPIActionRegister, param);
	RUN_PISEC_EVENT(CPIActionHeartbeatCheck, param);

	pid_t pid = atoi(param.c_str());
	return PIAgentStub.isRegistered(pid);
}

void CPIEventHandler::onRegsiter(const CPIEvent* pEvent) {

	INFO_LOG1("PIAgent register event received");

	RUN_PISEC_EVENT(CPIActionCheckAgent, "register");
}
#endif // #ifndef _PIEVENTHANDLER_CPP
