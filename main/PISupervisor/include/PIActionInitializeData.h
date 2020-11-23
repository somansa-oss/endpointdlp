#ifndef	_PIACTIONINITIALIZEDATA_H
#define	_PIACTIONINITIALIZEDATA_H

#ifdef LINUX
#include <string>
#endif

#include "PIAction.h"
#include "PIObject.h"

class CPIActionInitializeData : public IPIAction, public CPIObject {
	public:
		CPIActionInitializeData();
		virtual ~CPIActionInitializeData();
	
	protected:
		pthread_t wallManThread;
		pthread_attr_t wallManThreadAttr;
		bool runningWallManThread;

	protected:
		bool startThreads(void);
		void waitThreads(void);
		bool isRunningWallManThread(void);

	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionInitializeData& getInstance(void);
		static void* fnInitializeWallMan(void* pzArg);
};
#endif // #ifndef _PIACTIONINITIALIZEDATA_H
