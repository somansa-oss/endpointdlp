#ifndef	_PISCHEDULER_H
#define	_PISCHEDULER_H

#include "PIObject.h"

class CPIScheduler : public CPIObject {
	public:
		CPIScheduler();
		virtual ~CPIScheduler();
	
	public:
		pthread_t threadMain;
		pthread_attr_t threadMainAttr;
	
	public:
		bool startThreads(void);
		bool waitThreads(void);
		bool stop(void);

	public:
		static void* fnSchedulerMain(void* pzArg);
		static CPIScheduler& getInstance(void);
};
#endif // #ifndef _PISCHEDULER_H
