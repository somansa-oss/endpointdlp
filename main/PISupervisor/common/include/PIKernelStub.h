#ifndef	_PIKERNELSTUB_H
#define	_PIKERNELSTUB_H

#include "include_system.h"
#include "include_commonlib.h"
#include "include_common.h"
#include "PIUtility.h"

////////////////////////////////////////
//class IPIKernelStub
class IPIKernelStub {
	public:
		IPIKernelStub();
		virtual ~IPIKernelStub();

	public:
		typedef int (*KERNEL_EVENTHANDLER)(void* param);

	protected:
		CPIUtility util;

	public:
		int result;
		bool initialized;

	public:
		virtual bool initialize(KERNEL_EVENTHANDLER kernel_eventhandler) = 0;
		virtual bool finalize(void) = 0;
		virtual void clear(void) = 0;
		virtual bool applyPolicy(unsigned long command, void* in, unsigned long size) = 0;
		virtual void load(void) = 0;
		virtual void unload(void) = 0;
		virtual bool isActive(void) = 0;
};
#endif // #ifndef _PIKERNELSTUB_H
