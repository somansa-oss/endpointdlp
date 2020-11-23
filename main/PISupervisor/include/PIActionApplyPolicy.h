#ifndef	_PIACTIONAPPLYPOLICY_H
#define	_PIACTIONAPPLYPOLICY_H

#ifdef LINUX
#include <string>
#endif

#include "PIAction.h"
#include "PIObject.h"

class CPIActionApplyPolicy : public IPIAction, public CPIObject
{
	public:
		CPIActionApplyPolicy();
		virtual ~CPIActionApplyPolicy();

		// variable
	protected:	
		pthread_t policyThread;
		pthread_attr_t policyThreadAttr;
		bool running;
        pthread_mutex_t mutexApplyPolicy;

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
		static CPIActionApplyPolicy& getInstance(void);
		static void* fnApplyPolicy(void* pzArg);
};
#endif // #ifndef _PIACTIONAPPLYPOLICY_H
