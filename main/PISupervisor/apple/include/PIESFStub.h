//
//  PISecESFStub.hpp
//  PISupervisor
//
//  Created by Juno on 2020/08/05.
//  Copyright © 2020 somansa. All rights reserved.
//

#ifndef _PIESFStub_h_
#define _PIESFStub_h_

#include "PIKernelStub.h"
#include "PISecSmartDrvStub.h"

class CPIESFStub : public IPIKernelStub
{
public:
    CPIESFStub();
    ~CPIESFStub();

public:
    static CPIESFStub& getInstance();
    
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
#endif
