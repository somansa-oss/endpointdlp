#ifndef	_PISECFILTERDEVICESTUB_H
#define	_PISECFILTERDEVICESTUB_H

#include "PIKernelStub.h"

////////////////////////////////////////
//class CPISecFilterDeviceStub 
class CPISecFilterDeviceStub : public IPIKernelStub {
	public:
		CPISecFilterDeviceStub();
		virtual ~CPISecFilterDeviceStub();
	
	public:
		virtual bool initialize(KERNEL_EVENTHANDLER kernel_eventhandler);
		virtual bool finalize(void);
		virtual void clear(void);
		virtual bool applyPolicy(unsigned long command, void* in, unsigned long size);
		virtual void load(void);
		virtual void unload(void);
		virtual bool isActive(void);
};
#endif // #ifndef _PISECFILTERDEVICESTUB_H
