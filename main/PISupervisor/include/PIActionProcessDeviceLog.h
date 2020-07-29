#ifndef	_PIACTIONPROCESSDEVICELOG_H
#define	_PIACTIONPROCESSDEVICELOG_H

#include "PIAction.h"
#include "PIObject.h"
#include "PIDeviceLog.h"

class CPIActionProcessDeviceLog : public IPIAction, public CPIObject {
	public:
		CPIActionProcessDeviceLog();
		virtual ~CPIActionProcessDeviceLog();

		//typedef
	public:
		typedef enum { STATE_WAIT, STATE_SIG } STATE;

		// variables
	protected:
		bool isContinue;
		bool running;

		pthread_t logThread;
		pthread_attr_t logThreadAttr;
		CPIActionProcessDeviceLog::STATE stateDeviceLog;
		pthread_mutex_t mutexDeviceLog;
		pthread_cond_t condDeviceLog;
		CPIDeviceLog::VECTOR deviceLog;
		
		pthread_t savedLogThread;
		pthread_attr_t savedLogThreadAttr;
		CPIActionProcessDeviceLog::STATE stateSavedDeviceLog;
		pthread_mutex_t mutexSavedDeviceLog;
		pthread_cond_t condSavedDeviceLog;

		pthread_mutex_t mutexPISQLite;

		int savedDeviceLogCount;

		// method
	protected:	
		bool startThreads(void);
		void waitThreads(void);
		bool isRunning(void);

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionProcessDeviceLog& getInstance(void);
		static void* fnLog(void* pzArg);
		static void* fnSavedLog(void* pzArg);

	public:
		void addDeviceLog(CPIDeviceLog& deviceLog);
		bool saveDeviceLog(size_t index, CPIDeviceLog::VECTOR& deviceLog);
		bool sendSavedDeviceLog(CPIDeviceLog::VECTOR& deviceLog);
};
#endif // #ifndef _PIACTIONPROCESSDEVICELOG_H
