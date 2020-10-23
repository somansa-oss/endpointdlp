//
//  PISecESFStub.cpp
//  PISupervisor
//
//  Created by Juno on 2020/08/05.
//  Copyright © 2020 somansa. All rights reserved.
//


#ifndef    _PISECSMARTDRVSTUB_CPP
#define _PISECSMARTDRVSTUB_CPP

#ifdef LINUX
#include <string>
#include <map>
#include <sstream>
#endif

#include "PIESFStub.h"
#include "ESFManager.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>

#ifndef LINUX
#include <algorithm>
#endif

////////////////////////////////////////////////////////////////////////////////
//class CUSBMobile
class CUSBMobile
{
    public:
        CUSBMobile(){};
        virtual ~CUSBMobile(){};

    public:
        typedef std::vector<CUSBMobile> Vector;

    public:
        std::string type;
        std::string basePath;
        std::string keyword;
};

////////////////////////////////////////////////////////////////////////////////

//class CPIESFStub
CPIESFStub::CPIESFStub() {
}

CPIESFStub::~CPIESFStub() {
}

CPIESFStub& CPIESFStub::getInstance()
{
    static CPIESFStub instance;
    return instance;
}

bool CPIESFStub::initialize(KERNEL_EVENTHANDLER kernel_eventhandler)
{
    
    while ( true )
    {
        result = g_AppESF.ESFModule_Startup( kernel_eventhandler, false);
        
        if (result >= 0)
            break;
        
        sleep(1);
    }
    
    //result = g_AppESF.ESFModule_Startup( kernel_eventhandler, false);
    
    if(result < 0)
    {
        return false;
    }
    
    initialized = true;
    return true;
}

bool CPIESFStub::finalize(void)
{
    result = g_AppESF.ESFModule_Cleanup( false );
    
    if( true == isActive() )
    {
        // unload();
    }
    return true;
}

void CPIESFStub::clear(void)
{
}

bool CPIESFStub::applyPolicy(unsigned long command, void* in, unsigned long size)
{
    if( false == isActive() )
    {
        return false;
    }

    bool emptyRule = false;
    switch(command)
    {
        case SetDrivePolicy :
            //printf("pisecsmartdrv - set_drive_policy - size:%ld\n", size);

            if( (NULL == in) && (0 == size) )
            {
                in = malloc(sizeof(ATTACHED_DEVICE_EX));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            
            result = g_AppESF.fnSetDrivePolicy( in, (int)size);
            
            break;
        case SetPermitProcessName :
            //printf("pisecsmartdrv - set_permit_process_name - size:%ld\n", size);

            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(ALLOWPROCESSNAME));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            
            result = g_AppESF.fnSetPermitProcessName( in, (int)size);

            break;
        case SetPermitFolderName :
            //printf("pisecsmartdrv - set_permit_folder_name - size:%ld\n", size);

            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(ALLOWFOLDERNAME));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            
            result = g_AppESF.fnSetPermitFolderName( in, (int)size);
            
            break;
        case SetPermitFileExtName :
            //printf("pisecsmartdrv - set_permit_file_ext_name - size:%ld\n", size);

            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(ALLOWFILEEXTNAME));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            
            result = g_AppESF.fnSetPermitFileExtName( in, (int)size);
            break;
        case SetExceptDrivePolicy :
            //printf("pisecsmartdrv - set_except_drive_policy - size:%ld\n", size);

            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(ATTACHED_DEVICE));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            
            result = g_AppESF.fnSetExceptDrivePolicy( in, (int)size);
            break;
        case SetQuarantinePathExt :
            //printf("pisecsmartdrv - set_quarantine_path_ext - size:%ld\n", size);
            result = g_AppESF.fnSetQuarantinePathExt( (PQT_CTX)in);
            break;
        case SetQuarantineLimit :
            //printf("pisecsmartdrv - set_quarantine_limit- size:%ld\n", size);
            result = g_AppESF.fnSetQuarantineLimit( *((boolean_t*)in));
            break;
        case SetPrintPrevent :
            //printf("pisecsmartdrv - set_print policy- size:%ld\n", size);

            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(PRINT_POLICY));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            
            result = g_AppESF.fnSetPrintPrevent( in, (int)size);
            break;
            
        case SetSelfProtect:
            result = g_AppESF.fnSetSelfProtect_Commit( *((boolean_t*)in), SetSelfProtect );
            break;
        case SetSelfProtectOnlyPermit:
            result = g_AppESF.fnSetSelfProtect_Commit( *((boolean_t*)in), SetSelfProtectOnlyPermit );
            break;
            
        case SetUploadPrevent:
            if( (NULL == in) && (0 == size) ) {
                in = malloc(sizeof(UPLOAD_POLICY));
                size = sizeof(ATTACHED_DEVICE);
                emptyRule = true;
            }
            result = g_AppESF.fnSetUploadPrevent( in, (int)size);
            break;
            
        default : break;
    }

    if (true == emptyRule) {
        free(in);
    }

    return (0 == result) ? true : false;
}


//
bool CPIESFStub::applyPolicyAirDrop( const bool bAirDropBlock, const bool bAirDropLog )
{
    bool bResult = false;
    if(false == isActive())
    {
        return false;
    }
    
    bResult = g_AppESF.fnSetControlAirDrop( bAirDropBlock, bAirDropLog );
    return bResult;
}


// WLan Tethering
bool CPIESFStub::applyPolicyRndis( const bool bRNdisCtrl, const bool bRNdisLog )
{
    bool bResult = false;
    if(false == isActive())
    {
        return false;
    }
    bResult = g_AppESF.fnSetControlRndis( bRNdisCtrl, bRNdisLog );
    return bResult;
}


bool CPIESFStub::applyPolicyCamera(const bool bControl, const bool bLog )
{
    bool bResult = false;
    if(false == isActive())
    {
        return false;
    }
    bResult = g_AppESF.fnSetControlCamera( bControl, bLog );
    return bResult;
}


bool CPIESFStub::applyPolicyUSBMobile(const bool control, const bool log, void* param, std::vector<struct _MB_PERMIT>* pPermitList /*=NULL*/ )
{
    if(false == isActive())
    {
        return false;
    }

    MB_POLICY Policy;
    memset( &Policy, 0, sizeof(Policy) );
    Policy.bMtpLog   = (true == log) ? TRUE : FALSE;
    Policy.bMtpBlock = (true == control) ? TRUE : FALSE;
    Policy.bPtpLog   = (true == log) ? TRUE : FALSE;
    Policy.bPtpBlock = (true == control) ? TRUE : FALSE;

    result = g_AppESF.fnClrUSBMobilePermitList();
    
    if(pPermitList)
    {
        result = g_AppESF.fnSetUSBMobilePermitList( pPermitList );
    }

    result = g_AppESF.fnSetCtrlMobile( &Policy );
    result = g_AppESF.fnMobileNodeRemoveAll();
    
    if((NULL == param) || (false == control))
    {
        return (0 == result) ? true : false;
    }

    CUSBMobile::Vector* list = reinterpret_cast<CUSBMobile::Vector*>(param);
    if( 0 == list->size() )
    {
        return (0 == result) ? true : false;
    }

    CUSBMobile::Vector::iterator itr = list->begin();
    for(; itr != list->end(); itr++)
    {
        CUSBMobile& usbmobile = *itr;
        if("Mtp" == usbmobile.type )
        {
            result = g_AppESF.fnMobileNodeInsert_SetMtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
        }
        else if("Ptp" == usbmobile.type )
        {
            result = g_AppESF.fnMobileNodeInsert_SetPtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
        }
    }

    return (0 == result) ? true : false;
}


bool CPIESFStub::applyPolicySelectedBluetooth(const bool control, const bool log, void* param) {
    
    if( false == isActive() ) {
        return false;
    }
    
    MB_POLICY Policy;
    memset( &Policy, 0, sizeof(Policy) );
    
    Policy.bMtpLog   = (true == log) ? TRUE : FALSE;
    Policy.bMtpBlock = (true == control) ? TRUE : FALSE;
    Policy.bPtpLog   = (true == log) ? TRUE : FALSE;
    Policy.bPtpBlock = (true == control) ? TRUE : FALSE;
    
    result = g_AppESF.fnSetCtrlMobile( &Policy );
    result = g_AppESF.fnMobileNodeRemoveAll();
    
    if( (NULL == param) || (false == control) ) {
        return (0 == result) ? true : false;
    }
    
    CUSBMobile::Vector* list = reinterpret_cast<CUSBMobile::Vector*>(param);
    if( 0 == list->size() ) {
        return (0 == result) ? true : false;
    }
    
    CUSBMobile::Vector::iterator itr = list->begin();
    for(; itr != list->end(); itr++) {
        CUSBMobile& usbmobile = *itr;
        if( "Mtp" == usbmobile.type ) {
            result = g_AppESF.fnMobileNodeInsert_SetMtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
        }
        else if( "Ptp" == usbmobile.type ) {
            result = g_AppESF.fnMobileNodeInsert_SetPtp( usbmobile.basePath.c_str(), usbmobile.keyword.c_str());
        }
    }
    
    return (0 == result) ? true : false;
}

void CPIESFStub::load(void)
{
    system("kextload -v 6 /Library/Extensions/PISecSmartDrv.kext");}

void CPIESFStub::unload(void)
{
    // system("kextunload -v 6 /Library/Extensions/PISecSmartDrv.kext");
}

bool CPIESFStub::isActive(void)
{
    std::string command, temp;
#ifdef LINUX
    command = "ps -eaf | grep PIKern";
#else    
    command = "sudo launchctl list | grep piesf";
#endif    
    temp = util.readCommandOutput(command);
    
    if( true == temp.empty() )
    {
        return false;
    }
    
    std::istringstream is(temp);
    std::string token;
    std::getline(is, token);
#ifdef LINUX
    if( std::string::npos != token.find("PIKern") )
#else    
    if( std::string::npos != token.find("piesf") )
#endif    
    {
        return true;
    }
    
    return false;
}
        
bool CPIESFStub::applyPolicyProcess(const bool control, const bool log, void* param)
{
    if( false == isActive() )
    {
        return false;
    }

    BOOLEAN bProcAC = (true == control) ? TRUE : FALSE;
    BOOLEAN bLog = (true == log) ? TRUE : FALSE;
    
    result = g_AppESF.fnSetProcessAccessCheck( bProcAC, bLog);
    
    return (0 == result) ? true : false;
}
#endif

