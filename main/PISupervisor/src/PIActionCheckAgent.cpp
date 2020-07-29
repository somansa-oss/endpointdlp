#ifndef	_PIACTIONCHECKAGENT_CPP
#define	_PIACTIONCHECKAGENT_CPP

#include "PIActionCheckAgent.h"
#include "PIDocument.h"
#include "PIEventHandler.h"
#include "PIXmlParser.h"

////////////////////////////////////////
// CPIActionCheckAgent

CPIActionCheckAgent::CPIActionCheckAgent() {
	clear();
	pthread_mutex_init( &mutexLog, 0 );
}

CPIActionCheckAgent::~CPIActionCheckAgent() {	
	pthread_mutex_unlock( &mutexLog); 
	pthread_mutex_destroy( &mutexLog); 
}

int CPIActionCheckAgent::run(const std::string& param) {

	DEBUG_LOG1("checkagent_check - begin");

	if( "register" == param ) {
		DEBUG_LOG("checkagent_check - param:%s", param.c_str());
		onAgentRegistered();
	}
	else {
		if( true == isRunning() ) {
			DEBUG_LOG1("checkagent_check - skipped - already running");
			return 0;
		}

		clear();
		if( false == startThreads() ) {
			ERROR_LOG1("checkagent_check - create_thread failed");
		}
	}

	DEBUG_LOG1("checkagent_check - end");
	return 0;
}

bool CPIActionCheckAgent::startThreads(void) {

	DEBUG_LOG1("checkagent_check - begin");
	
	int result = 0;

	pthread_attr_init( &checkagentThreadAttr);
	pthread_attr_setscope( &checkagentThreadAttr, PTHREAD_SCOPE_SYSTEM );
	result = pthread_create( &checkagentThread, &checkagentThreadAttr, CPIActionCheckAgent::fnCheckAgent, (void*)this);
	if( result ) {
		ERROR_LOG1( "thread_create failed" );
		return false;
	}

	DEBUG_LOG1("checkagent_check - end");
	return true;
}

void CPIActionCheckAgent::waitThreads(void) {

	DEBUG_LOG1("checkagent_check - begin");

	pthread_join( checkagentThread, (void**)NULL );
	
	DEBUG_LOG1("checkagent_check - end");
}

int CPIActionCheckAgent::stop(void) {
	DEBUG_LOG1("checkagent_check - begin");

	isContinue = false;
	if( true == isRunning() ) {
		waitThreads();
	}

	DEBUG_LOG1("checkagent_check - end");
	return 0;
}

CPIActionCheckAgent& CPIActionCheckAgent::getInstance(void) {
	static CPIActionCheckAgent instance;
	return instance;
}

void* CPIActionCheckAgent::fnCheckAgent(void* pzArg) {
	DEBUG_LOG1("checkagent_check - begin");
	
	CPIActionCheckAgent* instance = reinterpret_cast<CPIActionCheckAgent*>(pzArg);
	instance->running = true;

	while(instance->isContinue && (false== Doc.getStop())) {
		//DEBUG_LOG1("loop");
		sleep(1);
		//todo: do something
		instance->checkAgent();
	}

	instance->running = false;
	DEBUG_LOG1("checkagent_check - end");
	return NULL;
}

void CPIActionCheckAgent::clear(void) {
	CPIObject::clear();

	isContinue = true;
	running = false;
}

bool CPIActionCheckAgent::isRunning(void) {
	return running;
}

bool CPIActionCheckAgent::checkAgent(void) {

	static bool working = false;
	if( true == working) {
		return true;
	}

	working = true;
	checkBegin();
	checkDriver(FILE_DRIVER);
	checkEnd();
	working = false;

	return true;
}

bool CPIActionCheckAgent::checkBegin(void) {
	return true; // do nothing
}

bool CPIActionCheckAgent::checkEnd(void) {

	if( 0 == log.size() ) {
		return true;
	}

	if( false == PIAgentStub.isRegistered() ) {
		return true;
	}

	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;
	markup.AddElem("LogList");

	pthread_mutex_lock( &mutexLog);
	for( size_t index = 0; index < log.size(); ++index) {
		markup.AddChildSubDoc(log[index]);
	}
	std::string eventList = markup.GetDoc();
	if( true == PIAgentStub.notifyEvent(nsPISupervisor::EVENT_CHANGED_AGENT_STATUS, eventList) ) {
		log.clear();
	}
	pthread_mutex_unlock( &mutexLog);

	return true;
}

bool CPIActionCheckAgent::checkDriver(EM_CATEGORY category) {

	bool isActive;

	// get status
	isActive = DeviceMan.smartDrvStub.initialized;
	
	// is status changed
	if( ( 0 < serviceStatus.size() ) && ( isActive == serviceStatus[category] ) ) {
		return true;
	}

	// set status
	serviceStatus[category] = isActive;
	EM_SUBCATEGORY subCategory = ( true == isActive ) ? STARTED : STOPPED;
	makeDriverEvent(category, subCategory);

	return true;
}

std::string CPIActionCheckAgent::getLog(EM_CATEGORY category, EM_SUBCATEGORY subCategory) {
	CPIXmlParser xmlParser;
	CMarkup & markup = xmlParser.markup; 
	markup.AddElem("Log");
	markup.AddAttrib("EventTime", util.getCurrentDateTime());
	markup.AddAttrib("Category", (int)category);
	markup.AddAttrib("SubCategory", (int)subCategory);

	return markup.GetDoc();
}

std::string CPIActionCheckAgent::getStatus(EM_CATEGORY category, EM_SUBCATEGORY subCategory) {
	CPIXmlParser xmlParser;
	CMarkup & markup = xmlParser.markup; 
	markup.AddElem("Status");
	markup.AddAttrib("Category", (int)category);
	markup.AddAttrib("SubCategory", (int)subCategory);

	return markup.GetDoc();
}

bool CPIActionCheckAgent::makeDriverEvent(EM_CATEGORY category, EM_SUBCATEGORY subCategory) {
	agentStatus[category] = subCategory;
	std::string log = getLog(category, subCategory);

	pthread_mutex_lock( &mutexLog);
	this->log.push_back(log);
	pthread_mutex_unlock( &mutexLog);
	return true;
}

bool CPIActionCheckAgent::onAgentRegistered(void) {
	DEBUG_LOG1("checkagent_check - begin");

	if( 0 == agentStatus.size() ) {
		DEBUG_LOG1("checkagent_check - skip - no_item");
		return true;
	}

	if( false == PIAgentStub.isRegistered() ) {
		DEBUG_LOG1("checkagent_check - skip - agent_not_registred");
		return true;
	}

	CPIXmlParser xmlParser;
	CMarkup& markup = xmlParser.markup;
	markup.AddElem("StatusList");

	pthread_mutex_lock( &mutexLog);
	MAP_AGENT_STATUS::iterator itr = agentStatus.begin();
	for( ; itr != agentStatus.end(); ++itr) {
		std::string event = getStatus(itr->first, itr->second);
		markup.AddChildSubDoc(event);
	}
	std::string eventList = markup.GetDoc();
	PIAgentStub.notifyEvent(nsPISupervisor::EVENT_POST_AGENT_STATUS, eventList);
	pthread_mutex_unlock( &mutexLog);

	DEBUG_LOG1("checkagent_check - end");
	return true;
}
#endif // #ifndef _PIACTIONCHECKAGENT_CPP
