#ifndef	_PISECSMARTDRVSTUB_H
#define	_PISECSMARTDRVSTUB_H

#include "PIKernelStub.h"

//class CPISecSmartDrvStub 
struct _MB_PERMIT;

class CPISecSmartDrvStub : public IPIKernelStub
{
	public:
		CPISecSmartDrvStub();
		virtual ~CPISecSmartDrvStub();
	
	public:
		virtual bool initialize(KERNEL_EVENTHANDLER kernel_eventhandler);
		virtual bool finalize(void);
		virtual void clear(void);
		virtual bool applyPolicy(unsigned long command, void* in, unsigned long size);
		virtual void load(void);
		virtual void unload(void);
		virtual bool isActive(void);

	public:
		bool applyPolicyUSBMobile(const bool control, const bool log, void* param, std::vector<struct _MB_PERMIT>* pPermitList = NULL);
		bool applyPolicySelectedBluetooth(const bool control, const bool log, void* param);
		bool applyPolicyProcess(const bool control, const bool log, void* param);
        bool applyPolicyCamera(const bool bControl, const bool bLog );
        bool applyPolicyRndis(const bool bRNdisCtrl, const bool bRNdisLog );
        bool applyPolicyAirDrop(const bool bAirDropBlock, const bool bAirDropLog );
    
};
#endif // #ifndef _PISECSMARTDRVSTUB_H
