#ifndef	_PIACTIONPROCESSDEVICELOG_CPP
#define	_PIACTIONPROCESSDEVICELOG_CPP

#include <map>

#ifdef LINUX
#include <string>
#endif

#include "PIActionProcessDeviceLog.h"
#include "PIDocument.h"
#include <sys/time.h>
#include "sqlite3.h"

////////////////////////////////////////
// CPIActionProcessDeviceLog

CPIActionProcessDeviceLog::CPIActionProcessDeviceLog() {
	clear();
}

CPIActionProcessDeviceLog::~CPIActionProcessDeviceLog() {	
}

void CPIActionProcessDeviceLog::clear(void) {
	CPIObject::clear();
	
	isContinue = true;
	running = false;
	deviceLog.clear();
}

bool CPIActionProcessDeviceLog::initialize(void) {
	DEBUG_LOG1("process device log - begin");
	
	CPIObject::initialize();

	pthread_mutex_init( &mutexDeviceLog, 0 );
	pthread_cond_init( &condDeviceLog, 0 );
	stateDeviceLog = CPIActionProcessDeviceLog::STATE_WAIT;
	
	pthread_mutex_init( &mutexSavedDeviceLog, 0 );
	pthread_cond_init( &condSavedDeviceLog, 0 );
	stateSavedDeviceLog = CPIActionProcessDeviceLog::STATE_WAIT;
	
	pthread_mutex_init( &mutexPISQLite, 0 );

	DEBUG_LOG1("process device log - end");
	
	return true;
}

bool CPIActionProcessDeviceLog::finalize(void) {
	DEBUG_LOG1("process device log - begin");

	pthread_mutex_unlock( &mutexDeviceLog); 
	pthread_mutex_destroy( &mutexDeviceLog); 
	pthread_cond_destroy( &condDeviceLog); 
	
	pthread_mutex_unlock( &mutexSavedDeviceLog); 
	pthread_mutex_destroy( &mutexSavedDeviceLog); 
	pthread_cond_destroy( &condSavedDeviceLog); 
	
	pthread_mutex_unlock( &mutexPISQLite); 
	pthread_mutex_destroy( &mutexPISQLite); 
	
	DEBUG_LOG1("process device log - end");
	
	return CPIObject::finalize();
}

int CPIActionProcessDeviceLog::run(const std::string& param) {

	DEBUG_LOG1("process device log - begin");
	INFO_LOG1("connect to driver");
    
    if( true == isRunning() ) {
        DEBUG_LOG1("process device log - skip - already running");
        return 0;
    }

    initialize();
	if( false == startThreads() ) {
		ERROR_LOG1("process device log - create_thread failed");
	}

	DEBUG_LOG1("process device log - end");
	return 0;
}

bool CPIActionProcessDeviceLog::startThreads(void) {

	DEBUG_LOG1("process device log - begin");
	
	int result = 0;

	if( false == isRunning() ) {
		pthread_attr_init( &logThreadAttr);
		pthread_attr_setscope( &logThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &logThread, &logThreadAttr, CPIActionProcessDeviceLog::fnLog, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for log" );
			return false;
		}
		
		pthread_attr_init( &savedLogThreadAttr);
		pthread_attr_setscope( &savedLogThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &savedLogThread, &savedLogThreadAttr, CPIActionProcessDeviceLog::fnSavedLog, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for saved log" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("process device log - skip - already running");
	}

	DEBUG_LOG1("process device log - end");
	return true;
}

void CPIActionProcessDeviceLog::waitThreads(void) {

	DEBUG_LOG1("process device log - begin");

	if( true == isRunning() ) {
		pthread_join( logThread, (void**)NULL );
		pthread_join( savedLogThread, (void**)NULL );
	}
	
	DEBUG_LOG1("process device log - end");
}

int CPIActionProcessDeviceLog::stop(void) {
	DEBUG_LOG1("process device log - begin");
	
	isContinue = false;
	stateDeviceLog = CPIActionProcessDeviceLog::STATE_SIG;
	pthread_cond_signal(&condDeviceLog);
	stateSavedDeviceLog = CPIActionProcessDeviceLog::STATE_SIG;
	pthread_cond_signal(&condSavedDeviceLog);

	waitThreads();
    finalize();

	DEBUG_LOG1("process device log - end");
	return 0;
}

CPIActionProcessDeviceLog& CPIActionProcessDeviceLog::getInstance(void) {
	static CPIActionProcessDeviceLog instance; 
	return instance;
}

void* CPIActionProcessDeviceLog::fnLog(void* pzArg)
{
	DEBUG_LOG1("process device log - begin");
    
	CPIActionProcessDeviceLog* instance = reinterpret_cast<CPIActionProcessDeviceLog*>(pzArg);
	instance->running = true;

	CPIDeviceLog::VECTOR deviceLog;
	while(instance->isContinue && (false == Doc.getStop()))
    {
        pthread_mutex_lock( &instance->mutexDeviceLog);
		DEBUG_LOG1("process device log - driver log wait...");

		while(CPIActionProcessDeviceLog::STATE_SIG != instance->stateDeviceLog)
        {
			/* int err = */ pthread_cond_wait(&instance->condDeviceLog, &instance->mutexDeviceLog);
            //DEBUG_LOG("process device log - pthread_cond_wait - %d", err);
		}

		if(false == instance->isContinue || (true == Doc.getStop()))
        {
			break;
		}

		DEBUG_LOG1("process device log - driver log received...");
		if( 0 < instance->deviceLog.size() ) {
			deviceLog.clear();
			deviceLog.resize(instance->deviceLog.size());
			std::copy(instance->deviceLog.begin(), instance->deviceLog.end(), deviceLog.begin());
			instance->deviceLog.clear();
		}
		instance->stateDeviceLog = CPIActionProcessDeviceLog::STATE_WAIT;
		pthread_mutex_unlock( &instance->mutexDeviceLog);

		if( 0 == deviceLog.size() ) {
			continue;
		}
	
		// ----------
		bool result = true;
		for(size_t index = 0; index < deviceLog.size(); ++index)
        {
			DEBUG_LOG1("process device log - process driver log ...");
			//DEBUG_LOG("process device log - sample : %s", deviceLog[index].string().c_str());
        
            if((deviceLog[index].policyType != MEDIA_AIRDROP) && true == DeviceMan.isDuplicatedDeviceLog(deviceLog[index]) )
            {
                DEBUG_LOG("process device log - skip - duplicated:%s", deviceLog[index].string().c_str());
                continue;
            }
            
			result = PIAgentStub.setDeviceLog(deviceLog[index].string());
			if( false == result ) {
				DEBUG_LOG1("process device log - sending_device_log - failed");
				ERROR_LOG1("process device log - sending_device_log - failed");
				instance->saveDeviceLog(index, deviceLog);
				break;
			}
		}
		deviceLog.clear();

		if( ( true == result ) && ( 0 < instance->savedDeviceLogCount ) ) {
			pthread_mutex_lock(&instance->mutexSavedDeviceLog);
			instance->stateSavedDeviceLog = CPIActionProcessDeviceLog::STATE_SIG;
			pthread_mutex_unlock(&instance->mutexSavedDeviceLog);
			pthread_cond_signal(&instance->condSavedDeviceLog);
		}
		// ----------
	}

	instance->running = false;
	DEBUG_LOG1("process device log - end");
	return NULL;
}


void* CPIActionProcessDeviceLog::fnSavedLog(void* pzArg) {
	DEBUG_LOG1("process device log - begin");

#define WAIT_TIME_SECONDS 10

	CPIActionProcessDeviceLog* instance = reinterpret_cast<CPIActionProcessDeviceLog*>(pzArg);
	instance->running = true;

	struct timespec ts;
	struct timeval tv;
	CPIDeviceLog::VECTOR deviceLog;
	while(instance->isContinue && (false == Doc.getStop())) {
        
        pthread_mutex_lock( &instance->mutexSavedDeviceLog);
		//DEBUG_LOG1("process device log - driver log wait...");

		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = tv.tv_usec * 1000;
		ts.tv_sec += WAIT_TIME_SECONDS; 

		while(CPIActionProcessDeviceLog::STATE_SIG != instance->stateSavedDeviceLog) {
			int rc = pthread_cond_timedwait(&instance->condSavedDeviceLog, &instance->mutexSavedDeviceLog, &ts);
            //DEBUG_LOG("process device log - pthread_cond_timedwait - %d", rc);
			if( ETIMEDOUT == rc ) {
				//DEBUG_LOG1("process device log - pthread_cond_timedwait - timeout");
				break;
			}
		}

		if(false == instance->isContinue || (true == Doc.getStop())) {
			break;
		}

		if( true == PIAgentStub.isRegistered() ) {
			pthread_mutex_lock( &instance->mutexPISQLite);
			PISQLiteStub.getAll(deviceLog);
			pthread_mutex_unlock( &instance->mutexPISQLite);
		}
		
		instance->stateSavedDeviceLog = CPIActionProcessDeviceLog::STATE_WAIT;
		pthread_mutex_unlock( &instance->mutexSavedDeviceLog);

		instance->sendSavedDeviceLog(deviceLog);
		deviceLog.clear();
	}

#undef WAIT_TIME_SECONDS

	instance->running = false;
	DEBUG_LOG1("process device log - end");
	return NULL;
}


bool CPIActionProcessDeviceLog::isRunning(void) {
	return running;
}

void CPIActionProcessDeviceLog::addDeviceLog(CPIDeviceLog& deviceLog) {
	DEBUG_LOG1("process device log");
    
    pthread_mutex_lock(&mutexDeviceLog);
	stateDeviceLog = CPIActionProcessDeviceLog::STATE_SIG;
	this->deviceLog.push_back(deviceLog);
	pthread_mutex_unlock(&mutexDeviceLog);
    pthread_cond_signal(&condDeviceLog);
}

bool CPIActionProcessDeviceLog::saveDeviceLog(size_t index, CPIDeviceLog::VECTOR& deviceLog) {

	if( index >= deviceLog.size() ) {
		return false;
	}

	void* conn;
	PISQLiteStub.open(conn);
	for(; index < deviceLog.size(); ++index) {
		//todo: log save and retry again
		DEBUG_LOG1("process device log - save device_log");
		pthread_mutex_lock( &mutexPISQLite);
		PISQLiteStub.push_back(conn, deviceLog[index]);
		pthread_mutex_unlock( &mutexPISQLite);
	}
	PISQLiteStub.close(conn);

	return true;
}

bool CPIActionProcessDeviceLog::sendSavedDeviceLog(CPIDeviceLog::VECTOR& deviceLog) {

	if( 0 == deviceLog.size() ) {
		return true;
	}

	void* conn;
	PISQLiteStub.open(conn);
	for(size_t index = 0; index < deviceLog.size(); ++index) {
		DEBUG_LOG1("process device log - process...");
		bool result = PIAgentStub.setDeviceLog(deviceLog[index].string());
		if( false == result ) {
			DEBUG_LOG1("process device log - sending_device_log - failed");
			ERROR_LOG1("process device log - sending_device_log - failed");
			break;
		}
		pthread_mutex_lock( &mutexPISQLite);
		PISQLiteStub.erase(conn, deviceLog[index].seq, deviceLog[index].getLogTime());
		pthread_mutex_unlock( &mutexPISQLite);
	}
	PISQLiteStub.close(conn);
	deviceLog.clear();

	return true;
}
#endif
