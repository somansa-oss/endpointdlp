#ifndef	_PIACTIONHEARTBEATCHECK_H
#define	_PIACTIONHEARTBEATCHECK_H

#include "PIAction.h"
#include "PIObject.h"

class CPIActionHeartbeatCheck : public IPIAction, public CPIObject {
	public:
		CPIActionHeartbeatCheck();
		virtual ~CPIActionHeartbeatCheck();

		// variable
	public:	
		pthread_t heartbeatThread;
		pthread_attr_t heartbeatThreadAttr;
		long ttlCycleCheck;
		bool isContinue;
		bool running;

	protected:	
		bool startThreads(void);
		void waitThreads(void);
		bool isRunning(void);
	
	public:
		virtual void clear(void);
	
	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionHeartbeatCheck& getInstance(void);
		static void* fnHeartbeatCheck(void* pzArg);
};
#endif // #ifndef _PIACTIONHEARTBEATCHECK_H
