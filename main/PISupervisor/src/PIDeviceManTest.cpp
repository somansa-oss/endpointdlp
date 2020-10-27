#ifndef	_PIDEVICEMANTEST_CPP
#define	_PIDEVICEMANTEST_CPP

#include <map>

#include "PIDeviceManTest.h"
#include "PIDocument.h"
#include "include_kernel.h"
#include "PIDeviceLog.h"
#include "PIActionProcessDeviceLog.h"

////////////////////////////////////////
// CPIDeviceManTest 

CPIDeviceManTest::CPIDeviceManTest()
{
}

CPIDeviceManTest::~CPIDeviceManTest()
{
}

void CPIDeviceManTest::copyPreventPlusDeviceLog(void)
{
#ifndef LINUX
	CPIDeviceLog deviceLog;
	deviceLog.deviceType 	= CPIDeviceLog::typeDrive;
	deviceLog.policyType 	= MEDIA_CD_DVD;
	deviceLog.accessType 	= accessRead;
	deviceLog.recordType 	= 0;
	deviceLog.setLogTime( 	  util.getCurrentDateTime());
	deviceLog.logType 		= LOG_VNODE;
	deviceLog.logAction 	= ACTION_READ;
	deviceLog.disableResult = true;
	deviceLog.processId 	= 0;
	deviceLog.processName 	= "testProc";
	deviceLog.fileName 		= "testFile";

	if(MEDIA_COPYPREVENT & deviceLog.policyType)
    {
		DEBUG_LOG1("skip - copy_prevent_log");
		return;
	}

	if( false == PolicyMan.enableMediaControlBlockLog )
    {
		DEBUG_LOG1("skip - enable_media_control_block_log:false");
		return;
	}
	
	CPIActionProcessDeviceLog::getInstance().addDeviceLog(deviceLog);
#endif
}

void CPIDeviceManTest::printPreventPlus(void) {
#ifndef LINUX
	std::string pdfPath = "/tmp/test_print.pdf";
	unsigned int processId = 0;
	
	int result = 0;
    
	PIAgentStub.analyzeFileForPreventPlus(pdfPath, pdfPath, processId, ActionTypePrint, result);
	
	DEBUG_LOG("result:%s(%d)", (1 == result) ? "block" : "allow", result);
#endif
}

void CPIDeviceManTest::copyPreventPlus(void) {
#ifndef LINUX
	std::string filePath = "/tmp/test_data.txt";
	std::string tempFilePath = "Volumes/TESTUSB/test_data.txt";
	unsigned int processId = 0;
	unsigned int deviceCategory = 0x0008;
	
	int result = 0;
	PIAgentStub.analyzeFileForCopyPreventPlus(filePath, tempFilePath, deviceCategory, processId, result);
	
	DEBUG_LOG("result:%s(%d)", (1 == result) ? "block" : "allow", result);
#endif
}
#endif
