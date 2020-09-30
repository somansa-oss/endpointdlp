#ifndef	_PIAGENTSTUB_CPP
#define _PIAGENTSTUB_CPP

#include <map>

#include"PIAgentStub.h"
#include"PICommunicator.h"
#include"PIDocument.h"
#include "PIEventHandler.h"

CPIAgentStub::CPIAgentStub() { 
	clear();
}

CPIAgentStub::~CPIAgentStub() {
}

bool CPIAgentStub::initialize(void ) {
	CPIObject::initialize();
	return true;
}

bool CPIAgentStub::finalize(void) { 
	CPIObject::finalize();
	return true;
}

void CPIAgentStub::clear(void) { 
	CPIObject::clear();
	
	name = "";
	pid = 0;
	memset(&timeStamp, 0x00, sizeof(struct tm));
	maxTimeSpan = 0;
}

void CPIAgentStub::setName(const std::string name) {
	this->name = name;
}

std::string CPIAgentStub::getName(void) const {
	return name;
}

void CPIAgentStub::setProcessId(const pid_t pid) {
	this->pid = pid;
}

pid_t CPIAgentStub::getProcessId(void) const {
	return pid;
}

void CPIAgentStub::setMaxTimeSpan(const int maxTimeSpan) {
	this->maxTimeSpan = maxTimeSpan;
}

int CPIAgentStub::getMaxTimeSpan(void) {
	return maxTimeSpan;
}

void CPIAgentStub::pressTimeStamp(void) {
	util.getCurrentDateTime(timeStamp);
	DEBUG_LOG("piagent - pid:%d - time:%s", 
			getProcessId(), util.getValueAsString(timeStamp).c_str());
}

bool CPIAgentStub::isOutDated(void) {

	if( (0 == pid ) || (0 == timeStamp.tm_year) ) {
		return true;
	}

	double seconds = util.diffTime(timeStamp);
	DEBUG_LOG("piagent - pid:%d - seconds:%f - max_time_span(%d)", 
			getProcessId(), seconds, getMaxTimeSpan());

	if( seconds > (double) getMaxTimeSpan()*3 ) {
		return true;
	}

	return false;
}

bool CPIAgentStub::isRegistered(const pid_t pid) const {

	bool result = false;

	do {
		if( getProcessId() != pid ) {
			break;
		}

		result = true;

	}while(false);

	return result;	
}

bool CPIAgentStub::isRegistered(void) const {
	return ( 0 == getProcessId() ) ? false : true;
}

void CPIAgentStub::set(std::string name, const pid_t pid, const int maxTimeSpan) {
	setName(name);
	setProcessId(pid);
	setMaxTimeSpan(maxTimeSpan);
	pressTimeStamp();
	DEBUG_LOG("piagent - registered - pid:%d - max_time_span:%d - time:%s", 
			getProcessId(), getMaxTimeSpan(), util.getValueAsString(timeStamp).c_str());

	EventHandler.addEvent(nsPISupervisor::EVENT_REGISTER, "");

}

bool CPIAgentStub::sendToPIAgent(CPIPacket& packet, int& response) {

	bool returnValue = false;
	CPICommunicator::COMM_RESULT_T result;
	CPICommunicator* comm = new CPICommunicator();
    
#ifndef LINUX
	comm->support.bigEndian = ConfigMan.socket.bigEndian;
	comm->support.savePacket = ConfigMan.socket.savePacket;
	comm->support.savePacketPath = ConfigMan.path.dlpLogPath;
#endif
	comm->ipString = "127.0.0.1";
	comm->tcpPort = nsPISupervisor::PIAGENT_PORT;

	do {
		if( false == comm->initializeTCPClient() ) {
			ERROR_LOG( "Unable to create socket - %s",  strerror(errno));
			break;
		}

		result = comm->connectTCP();
		if(  CPICommunicator::RESULT_OK != result ) {
			ERROR_LOG( "Unable to connect server - code(%d) - (%d)%s", result, errno, strerror(errno));
			break;
		}

		DEBUG_LOG( "connect piagent - ok - tcpState(%d)", comm->tcpState );

		comm->listSend.push_back(packet);

		result = comm->sendTCP();
		if(  CPICommunicator::RESULT_OK != result ) {
			ERROR_LOG( "Unable to send - code(%d) - tcpState(%d) - (%d)%s", result, comm->tcpState, errno, strerror(errno));
			break;
		}

		result = comm->recvTCP();
		if(  CPICommunicator::RESULT_OK != result ) {
			ERROR_LOG( "Unable to recv - code(%d) - (%d)%s", result, errno, strerror(errno));
			break;
		}

		returnValue = true; 
	}while(false);

	if( 0 < comm->listRecv.size()) {
		DEBUG_LOG( "recv - msg_count:%d", comm->listRecv.size());
		std::list<CPIPacket>::iterator itr = comm->listRecv.begin();
		for( ; itr != comm->listRecv.end(); ++itr) {
			DEBUG_LOG( "recv - command(%d) - param_count(%d)", itr->getCommand(), itr->getParamCount());
			response = itr->get_int(0);
		}
	}

	if( CPICommunicator::TCPSTATE_NOTCONNECTED != comm->tcpState ) {
		comm->terminateTCPClient();
	}

	delete comm;
	comm = NULL;
	return returnValue;
}

bool CPIAgentStub::notifyEvent(nsPISupervisor::EM_EVENTID eventId, std::string param) {

	DEBUG_LOG( "piagent - begin - event:%d - param:%s", eventId, param.c_str());
	
	if( false == isRegistered() ) {
		DEBUG_LOG1( "piagent - stop - agent_not_registered");
		INFO_LOG1( "notify event to piagent : stop - agent_not_registered");
		return false;
	}
	
	CPIPacket packet;
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
	packet.setCommand(nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_ASYNC_EVENT_N);
	packet.add_string(util.getValueAsString((int)eventId));
	packet.add_string(param);

	int response = 0;
	bool result = sendToPIAgent(packet, response);

	DEBUG_LOG( "piagent - end - event:%d - result:%s - response:%d", eventId, util.getValueAsString(result).c_str(), response); 
	INFO_LOG( "notify event to piagent : event:%d - result:%s, response:%d", eventId, util.getValueAsString(result).c_str(), response); 

	return result;
}

bool CPIAgentStub::analyzeFileForCopyPreventPlus(std::string filePath, std::string tempFilePath, 
		unsigned int deviceCategory, unsigned int processId, int& response) {

	DEBUG_LOG( "piagent - begin - %s", filePath.c_str()); 
	
	if( false == isRegistered() ) {
		DEBUG_LOG1( "piagent - stop - agent_not_registered");
		INFO_LOG1( "request analyze file to piagent (copy prevent+) : stop - agent_not_registered");
		return false;
	}
	
	CPIPacket packet;
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
	packet.setCommand(nsPISupervisor::PIPE_COMMAND_WRITE_REMOVAL_DRIVE_N);
	packet.add_string(filePath);
	packet.add_string(tempFilePath);
	packet.add_uint(deviceCategory);
	packet.add_uint(processId);

	bool result = sendToPIAgent(packet, response);
	
	DEBUG_LOG( "piagent - end - result:%s - response:%d", util.getValueAsString(result).c_str(), response); 
	INFO_LOG( "request analyze file to piagent (copy prevent+) : result:%s, response:%d", util.getValueAsString(result).c_str(), response); 

	return result;
}

bool CPIAgentStub::notifyFileDelete(std::string filePath, unsigned int processId, int& response) {

	DEBUG_LOG( "piagent - begin - %s", filePath.c_str()); 
	
	if( false == isRegistered() ) {
		DEBUG_LOG1( "piagent - stop - agent_not_registered");
		INFO_LOG1( "notify disk delete to piagent : stop - agent_not_registered");
		return false;
	}

	CPIPacket packet;
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
	packet.setCommand(nsPISupervisor::PIPE_COMMAND_DELETE_FILE_N);
	packet.add_string(filePath);
	packet.add_uint(processId);

	bool result = sendToPIAgent(packet, response);

	DEBUG_LOG( "piagent - end - result:%s - response:%d", util.getValueAsString(result).c_str(), response); 
	INFO_LOG( "notify file delete to piagent : result:%s, response:%d", util.getValueAsString(result).c_str(), response); 

	return result;
}

bool CPIAgentStub::notifyDiskFull(std::string jobStatus, std::string filePath, int& response) {

	DEBUG_LOG( "piagent - begin - %s", filePath.c_str()); 
	
	if( false == isRegistered() ) {
		DEBUG_LOG1( "piagent - stop - agent_not_registered");
		INFO_LOG1( "notify disk full to piagent : stop - agent_not_registered");
		return false;
	}
	CPIPacket packet;
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
	packet.setCommand(nsPISupervisor::PIPE_COMMAND_PIAGENT_QT_DISKFULL_MSG_N);
	packet.add_string(jobStatus);
	packet.add_string(filePath);
	
	bool result = sendToPIAgent(packet, response);
	
	DEBUG_LOG( "piagent - end - result:%s - response:%d", util.getValueAsString(result).c_str(), response); 
	INFO_LOG( "notify disk full to piagent : result:%s, response:%d", util.getValueAsString(result).c_str(), response); 

	return result;
}

bool CPIAgentStub::notifyNeedFullDiskAccessAuth(int& response)
{
    if( false == isRegistered() ) {
        DEBUG_LOG1( "piagent - stop - agent_not_registered");
        INFO_LOG1( "notify disk full to piagent : stop - agent_not_registered");
        return false;
    }
    CPIPacket packet;
    packet.support.bigEndian = ConfigMan.socket.bigEndian;
    packet.support.unsignedType = ConfigMan.socket.unsignedType;
    packet.setCommand(nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_NEED_DISK_AUTH_N);
    
    bool result = sendToPIAgent(packet, response);
    
    DEBUG_LOG( "piagent - end - result:%s - response:%d", util.getValueAsString(result).c_str(), response);
    INFO_LOG( "notify disk full to piagent : result:%s, response:%d", util.getValueAsString(result).c_str(), response);

    return result;
}

bool CPIAgentStub::setDeviceLog(const std::string& deviceLog) {

	DEBUG_LOG( "piagent - begin - %s", deviceLog.c_str()); 

	if( false == isRegistered() ) {
		DEBUG_LOG1( "piagent - stop - agent_not_registered");
		INFO_LOG1( "send device_log to piagent : stop - agent_not_registered");
		return false;
	}

	CPIPacket packet;
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
	packet.setCommand(nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_ASYNC_DEVICELOG_N);
	packet.add_string(deviceLog);
	
	int response = 0;
	bool result = sendToPIAgent(packet, response);
	
	DEBUG_LOG( "piagent - end - result:%s - response:%d", util.getValueAsString(result).c_str(), response); 
	INFO_LOG( "send device_log to piagent : result:%s, response:%d", util.getValueAsString(result).c_str(), response); 

	return result;
}

bool CPIAgentStub::test(void) {

	int response = 0;
	analyzeFileForCopyPreventPlus("/Test/FilePath", "", 0x08, 2000, response);

	return true;
}

bool CPIAgentStub::analyzeFileForPreventPlus(std::string filePath, std::string orgfilePath, unsigned int processId, ACTION_TYPE emActionType, int& response) {

	ERROR_LOG( "piagent - begin - %s, %s", filePath.c_str(), orgfilePath.c_str());
	
	if( false == isRegistered() ) {
		DEBUG_LOG1( "piagent - stop - agent_not_registered");
		INFO_LOG1( "request analyze file to piagent (print prevent+) : stop - agent_not_registered");
		return false;
	}
	
	CPIPacket packet;
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
    
    if(ActionTypePrint == emActionType)
        packet.setCommand(nsPISupervisor::PIPE_COMMAND_PROCESS_CONTENT_N);
    else if(ActionTypeUpload == emActionType)
        packet.setCommand(nsPISupervisor::PIPE_COMMAND_PROCESS_NETWORK_CONTENT_N);
    
	packet.add_string(filePath);
    packet.add_string(orgfilePath);
	packet.add_uint(processId);

	bool result = sendToPIAgent(packet, response);
	
	ERROR_LOG( "piagent - end - result:%s - response:%d", util.getValueAsString(result).c_str(), response);
	ERROR_LOG( "request analyze file to piagent (print prevent+) : result:%s, response:%d", util.getValueAsString(result).c_str(), response);

	return result;
}
#endif
