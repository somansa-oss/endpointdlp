#ifndef	_PIACTIONAPPLYPOLICY_CPP
#define	_PIACTIONAPPLYPOLICY_CPP

#include <map>

#include "PIActionApplyPolicy.h"
#include "PIActionOpenClientIPC.h"
#include "PIDocument.h"

////////////////////////////////////////
// CPIActionApplyPolicy

CPIActionApplyPolicy::CPIActionApplyPolicy()
{
	clear();
    pthread_mutex_init( &mutexApplyPolicy, 0 );
}

CPIActionApplyPolicy::~CPIActionApplyPolicy()
{
    pthread_mutex_destroy( &mutexApplyPolicy);
}


int CPIActionApplyPolicy::run(const std::string& param)
{
    DEBUG_LOG1("apply_policy - begin");
	INFO_LOG1("apply_policy");
    
    //
    // Timing 이슈때문에 주석처리하여 아래와 같이 수정합니다. 2019.02.11
    //
/* 	if( true == isRunning() )
		DEBUG_LOG1("apply_policy - skip");
	else {
		if(false == startThreads()) {
			ERROR_LOG1("apply_policy - create_thread failed");
		}
	} */
     
    if( false == startThreads() )
    {
        ERROR_LOG1("apply_policy - create_thread failed");
    }

	DEBUG_LOG1("apply_policy - end");
	return 0;
}


bool CPIActionApplyPolicy::startThreads(void)
{
	DEBUG_LOG1("apply_policy - begin");
	
	int result = 0;
	pthread_attr_init( &policyThreadAttr);
	pthread_attr_setscope( &policyThreadAttr, PTHREAD_SCOPE_SYSTEM );
	result = pthread_create( &policyThread, &policyThreadAttr, CPIActionApplyPolicy::fnApplyPolicy, (void*)this);
	if( result )
    {
		ERROR_LOG1( "thread_create failed" );
		return false;
	}
	DEBUG_LOG1("apply_policy - end");
	return true;
}

void CPIActionApplyPolicy::waitThreads(void) {

	DEBUG_LOG1("apply_policy - begin");

	pthread_join( policyThread, (void**)NULL );
	
	DEBUG_LOG1("apply_policy - end");
}

int CPIActionApplyPolicy::stop(void) {
	DEBUG_LOG1("apply_policy - begin");

	if( true == isRunning() ) {
		waitThreads();
	}

	DEBUG_LOG1("apply_policy - end");
	return 0;
}

CPIActionApplyPolicy& CPIActionApplyPolicy::getInstance(void)
{
	static CPIActionApplyPolicy instance;
	return instance;
}

void* CPIActionApplyPolicy::fnApplyPolicy(void* pzArg)
{
	DEBUG_LOG1("apply_policy - begin");
	
	CPIActionApplyPolicy* instance = reinterpret_cast<CPIActionApplyPolicy*>(pzArg);
    
    pthread_mutex_lock( &getInstance().mutexApplyPolicy );
    DEBUG_LOG1("apply_policy - lock");
	
    instance->running = true;
	DeviceMan.applyPolicy();
	PIAgentStub.notifyEvent( nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, PIAgentStub.util.getValueAsString(PolicyMan.isOnline()) );
	instance->running = false;
    
    DEBUG_LOG1("apply_policy - unlock");
    pthread_mutex_unlock( &getInstance().mutexApplyPolicy );
    
	DEBUG_LOG1("apply_policy - end");
	return NULL;
}

bool CPIActionApplyPolicy::isRunning(void) {
	return this->running;
}

void CPIActionApplyPolicy::clear(void) {
	CPIObject::clear();
	
	running = false;
}
#endif // #ifndef _PIACTIONAPPLYPOLICY_CPP
