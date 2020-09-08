#ifndef	_SCHEDULER_CPP
#define	_SCHEDULER_CPP

#ifdef LINUX
#include <string>
#endif

#include"PIScheduler.h"
#include"PIEventHandler.h"

void* CPIScheduler::fnSchedulerMain( void* pzArg ) {

	DEBUG_LOG1("begin");

	//CPIScheduler* pScheduler = (CPIScheduler*) pzArg;
	CPIEventHandler::getInstance().run();

	DEBUG_LOG1("end");

	return NULL;
}

////////////////////////////////////////
// CPIScheduler

CPIScheduler::CPIScheduler() {
}

CPIScheduler::~CPIScheduler() {	
}

bool CPIScheduler::startThreads(void) {

	DEBUG_LOG1("scheduler - begin");

	int nRet = 0;
	bool bRet = true;

	pthread_attr_init( &threadMainAttr);
	pthread_attr_setscope( &threadMainAttr, PTHREAD_SCOPE_SYSTEM );
	nRet= pthread_create( &threadMain, &threadMainAttr, CPIScheduler::fnSchedulerMain, (void*)this );
	if( nRet ) {
		bRet = false;
		ERROR_LOG1("scheduler - thread - failed");
	}

	DEBUG_LOG1("scheduler - end");
	return bRet;
}

bool CPIScheduler::waitThreads(void) {

	DEBUG_LOG1("scheduler - begin");

	pthread_join( threadMain, (void**)NULL );

	DEBUG_LOG1("scheduler - end");
	return true;
}

bool CPIScheduler::stop(void) {

	DEBUG_LOG1("scheduler - begin");
	
	CPIEventHandler::getInstance().stop();
	
	DEBUG_LOG1("scheduler - end");
	return true;
}

CPIScheduler& CPIScheduler::getInstance(void) {
	static CPIScheduler	instance;
	return instance;
}
#endif // #ifndef _SCHEDULER_CPP
