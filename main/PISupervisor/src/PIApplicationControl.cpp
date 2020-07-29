#ifndef	_PIAPPLICATIONCONTROL_CPP
#define	_PIAPPLICATIONCONTROL_CPP

#include <sstream>
#include <map>

#include "PIApplicationControl.h"
#include "PIDocument.h"
#include "DLPPolicy.h"

////////////////////////////////////////
// CPIApplicationControl

CPIApplicationControl::CPIApplicationControl() {
}

CPIApplicationControl::~CPIApplicationControl() {	
}

std::string CPIApplicationControl::getProcessControlAsString(void) {
	bool enableProcessControl = false;
	bool execControlBlockLog = false;
	size_t listCount = 0;
	PolicyMan.getProcessControl(enableProcessControl, execControlBlockLog, listCount);

	CMarkup markup;
	markup.AddElem("constrol_process");
	markup.SetAttrib("enable", util.getValueAsString(enableProcessControl));
	markup.SetAttrib("last_update_time", util.getCurrentDateTime());
	markup.SetAttrib("log_block", util.getValueAsString(execControlBlockLog));

	CDLPPolicyApplication* pDLPPolicy = dynamic_cast<CDLPPolicyApplication*>(PolicyMan.getCurrentDLPPolicy(nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeApplicationPreventPlus));
	if (NULL != pDLPPolicy) {
	CDLPApplication::Vector& list = pDLPPolicy->applicationList;
		for(int index = 0; index < list.size(); index++) {
			CDLPApplication& application = list[index];
			if( CDLPApplication::controltypeExecControl != application.controlType ) {
				continue;
			}

			markup.AddChildElem("process");
			markup.AddChildAttrib("guid", application.guid);
			markup.AddChildAttrib("type", application.type);
			markup.AddChildAttrib("name", application.fileName);
			markup.AddChildAttrib("check_binary_string", application.binData);
		}
	}

	return markup.GetDoc();
}

bool CPIApplicationControl::writeProcessControlXml(void) {
	DEBUG_LOG1("begin");
	std::string processControlXml = getProcessControlAsString();
	std::string processControlFile = ConfigMan.path.getProcessControlFile();
	
	DEBUG_LOG("xml:%s", processControlXml.c_str());
	DEBUG_LOG("file:%s", processControlFile.c_str());

	CMarkup markup;
	markup.SetDoc(processControlXml);
	markup.Save(processControlFile);
	DEBUG_LOG1("end");

	return true;
}

bool CPIApplicationControl::getProcessControl(bool& control, bool& log) {
	bool enableProcessControl = false;
	bool execControlBlockLog = false;
	size_t listCount = 0;
	PolicyMan.getProcessControl(enableProcessControl, execControlBlockLog, listCount);

	if (0 == listCount) {
		enableProcessControl = false;
		execControlBlockLog = false;
	}

	control = enableProcessControl;
	log = execControlBlockLog;
	return true;
}

int CPIApplicationControl::isProcessAllow(std::string ticket, int pid, std::string filePath, std::string processName, CResult& result) {
	
	CDLPPolicy* pDLPPolicy = PolicyMan.getCurrentDLPPolicy(nsPISecObject::dlptypePreventPlus, nsPISecObject::dlpsubtypeApplicationPreventPlus);
	CDLPPolicyApplication* pDLPPolicyApplication = dynamic_cast<CDLPPolicyApplication*>(pDLPPolicy);
	if (NULL == pDLPPolicyApplication) {
		//DEBUG_LOG("[ticket:%s] skip - application_control_policy - not found", ticket.c_str());
		return CResult::ALLOW;
	}
	
	result.value = CResult::ALLOW;
	do {
		if (true == ConfigMan.dlpPolicy.applicationControl.isSkipWindowsExecutable()) {
			if (true == pDLPPolicyApplication->isControlApplicationNonWindowsExecutableEmpty()) {
				//DEBUG_LOG("[ticket:%s] skip - non_windows_executable_process_keyword - empty", ticket.c_str());
				break; // allow
			}
		} else {
			if (true == pDLPPolicyApplication->isControlApplicationEmpty()) {
				//DEBUG_LOG("[ticket:%s] skip - process_keyword - empty", ticket.c_str());
				break; // allow
			}
		}
		
		if (true == isInAllowedList(ticket, filePath, processName)) {
			break; // allow
		}

		// 1) control by process name
		if (true == blockWithProcessName(pDLPPolicyApplication, processName, result)) {
			result.value = CResult::BLOCK_PROCESSNAME;
			break;
		}

		// 2) control by file path
		if (true == ConfigMan.dlpPolicy.applicationControl.isCheckFilePath()) {
			if (true == blockWithFilePath(pDLPPolicyApplication, filePath, result)) {
				result.value = CResult::BLOCK_FILEPATH;
				break;
			}
		}

		// 3) control by binary
		if (true == ConfigMan.dlpPolicy.applicationControl.isCheckBinary()) {
			if (true == blockWithBinary(pDLPPolicyApplication, filePath, result)) {
				result.value = CResult::BLOCK_BINARY;
				break;
			}
		}
	} while(false);

	return result.value;
}

bool CPIApplicationControl::blockWithFilePath(CDLPPolicyApplication* pDLPPolicy, std::string filePath, CResult& result) {
	if (0 == filePath.length()) {
		return false;
	}

	std::string processName = util.getFileNameExt(filePath);
	return blockWithProcessName(pDLPPolicy, processName, result);
}

bool CPIApplicationControl::blockWithProcessName(CDLPPolicyApplication* pDLPPolicy, std::string processName, CResult& result) {
	if (NULL == pDLPPolicy) {
		return false;
	}

	bool found = false;
	CDLPApplication::Vector& list = pDLPPolicy->applicationList;
	CDLPApplication::Vector::const_iterator itr = list.begin();
	for ( ; ((itr != list.end()) && (false == found)); ++itr) {
		const CDLPApplication& application = *itr;
		if( CDLPApplication::controltypeExecControl != application.controlType ) {
			continue;
		}

		if (true == ConfigMan.dlpPolicy.applicationControl.isSkipWindowsExecutable()) {
			if (true == application.isWindowsExecutable()) {
				continue;
			}
		}

		//if (true == ConfigMan.dlpPolicy.applicationControl.isSkipWindowsExecutable()) {
		//	DEBUG_LOG("[test] - process_keyword:%s", application.getFileName().c_str());
		//}

		found = application.isSameProcessName(processName);
		if (true == found) {
			result.guid = application.guid;
			result.processName = processName;
		}
	}

	return found;
}

bool CPIApplicationControl::blockWithBinary(CDLPPolicyApplication* pDLPPolicy, std::string filePath, CResult& result) {
	if (NULL == pDLPPolicy) {
		return false;
	}

	int fd = -1;
	if ((fd = open(filePath.c_str(), O_RDONLY))<0) {
		return false;
	}

	bool found = false;
	CDLPApplication::Vector& list = pDLPPolicy->applicationList;
	CDLPApplication::Vector::const_iterator itr = list.begin();
	for ( ; ((itr != list.end()) && (false == found)); ++itr) {
		const CDLPApplication& application = *itr;
		if( CDLPApplication::controltypeExecControl != application.controlType ) {
			continue;
		}
		
		if (true == ConfigMan.dlpPolicy.applicationControl.isSkipWindowsExecutable()) {
			if (true == application.isWindowsExecutable()) {
				continue;
			}
		}
	
		//DEBUG_LOG("begin - fd:%d - file:%s", fd, filePath.c_str()); // test
		found = application.containBinary(fd, CPIApplicationControl::logCallback);
		//DEBUG_LOG("end:%d - fd:%d", found?1:0, fd); // test
		if (true == found) {
			result.guid = application.guid;
			result.processName = util.getFileNameExt(filePath);
		}
	}

	if (0 < fd) {
		close(fd);
		fd = -1;
	}
	
	return found;
}

bool CPIApplicationControl::blockWithBinary_experimental(CDLPPolicyApplication* pDLPPolicy, std::string filePath, CResult& result) {
	if (NULL == pDLPPolicy) {
		return false;
	}

	std::string command, temp;
	command = ConfigMan.path.binPath + "/PIBinCmp \"" + filePath + "\"";
	command += " 2>&1";

	DEBUG_LOG( "command:%s", command.c_str());
	temp = util.readCommandOutput(command);
	DEBUG_LOG( "[result] - %s", temp.c_str());

	bool found = false;
	if (false == temp.empty()) {
		std::istringstream is(temp);
		std::string token;
		if (std::getline(is, token)) {
			if (("not_found" != token) && ("open_error" != token)) {
				found = true;
			}
		}
	}

	if (true == found) {
		result.guid = temp;
		result.processName = util.getFileNameExt(filePath);
	}

	return found;
}

bool CPIApplicationControl::isInAllowedList(std::string ticket, std::string filePath, std::string processName) {

	// ----------
	// process
	// ----------
	if (0 < ConfigMan.dlpPolicy.applicationControl.getAllowedProcessList()->size()) {
		std::set<std::string>* list = ConfigMan.dlpPolicy.applicationControl.getAllowedProcessList();
		std::set<std::string>::iterator itr = list->find(processName);
		if (itr != list->end()) {
			//DEBUG_LOG("[ticket:%s] found - process:%s", ticket.c_str(), itr->c_str()); 
			return true;
		}
	}

	// ----------
	// file
	// ----------
	if (0 < ConfigMan.dlpPolicy.applicationControl.getAllowedFileList()->size()) {
		std::set<std::string>* list = ConfigMan.dlpPolicy.applicationControl.getAllowedFileList();
		std::set<std::string>::iterator itr = list->begin();
		for (; itr != list->end(); ++itr) {
			if ( std::string::npos != filePath.find(*itr)) {
				//DEBUG_LOG("[ticket:%s] found - file:%s", ticket.c_str(), itr->c_str());
				return true;
			}
		}
	}

	//DEBUG_LOG("[ticket:%s] not found", ticket.c_str());
	return false;
}

void CPIApplicationControl::logCallback(std::string& log)
{
	DEBUG_LOG("%s", log.c_str());
}

//
// RequestProcessTerminate
//
bool CPIApplicationControl::ReqeustProcessTerminate( pid_t nPID, char* pczFilePath )
{
    int nReturn = 0;
    
    nReturn = kill( nPID, SIGTERM);
    if(0 == nReturn)
    {
        printf("[pisupervisor][%s] Success. SIGTERM, pid=%d, FilePath=%s \n", __FUNCTION__, nPID, pczFilePath );
        DEBUG_LOG("Success. pid=%d, FilePath=%s", nPID, pczFilePath );
        return true;
    }
    
    nReturn = kill(nPID, SIGKILL);
    if(0 == nReturn)
    {
        printf("[pisupervisor][%s] Success. SIGKILL, pid=%d, FilePath=%s \n", __FUNCTION__, nPID, pczFilePath );
        DEBUG_LOG("Success. pid=%d, FilePath=%s", nPID, pczFilePath );
        return true;
    }
    
    printf("[pisupervisor][%s] failed. pid=%d, FilePath=%s \n", __FUNCTION__, nPID, pczFilePath );
    DEBUG_LOG("failed. pid=%d, FilePath=%s", nPID, pczFilePath );
    return false;
}




#endif // #ifndef _PIAPPLICATIONCONTROL_CPP
