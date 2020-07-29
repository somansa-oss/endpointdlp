#ifndef	_PIACTIONCONTROLDEVICE_H
#define	_PIACTIONCONTROLDEVICE_H

#include "PIAction.h"
#include "PIObject.h"

class CPIActionControlDevice : public IPIAction, public CPIObject {
	public:
		CPIActionControlDevice();
		virtual ~CPIActionControlDevice();

		// variables
	protected:
		bool isContinue;
		bool running;

		pthread_t deviceThread;
		pthread_attr_t deviceThreadAttr;

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
		static CPIActionControlDevice& getInstance(void);
		static void* fnControlDevice(void* pzArg);
};
#endif // #ifndef _PIACTIONCONTROLDEVICE_H
