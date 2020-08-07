#ifndef	_PIACTIONSUPERVISORIPC_CPP
#define	_PIACTIONSUPERVISORIPC_CPP

#include "../../PISupervisor/PISupervisor/apple/include/KernelProtocol.h"
#include "SmartCmd.h"

#include "PIActionSupervisorIPC.h"

#include "PIDocument.h"
//#include "PIEventHandler.h"

////////////////////////////////////////
// CPIActionSupervisorIPC
CPIActionSupervisorIPC::CPIActionSupervisorIPC() {
	clear();
}

CPIActionSupervisorIPC::~CPIActionSupervisorIPC() {
}

void CPIActionSupervisorIPC::clear(void) {
	CPIObject::clear();
	
	serverFd = -1;
	//serverPort = nsPISupervisor::PISUPERVISOR_PORT;
    serverPort = nsPISupervisor::PIESF_PORT;
	isContinue = true;
	runningServerThread = false;
	runningClientThread = false;
}

int CPIActionSupervisorIPC::run(const std::string& param) {

	DEBUG_LOG1("open_client_ipc - begin");
	INFO_LOG1("client ipc start");

	initialize();

	if( false == startThreads() ) {
		ERROR_LOG1("open_client_ipc - create_thread failed");
	}

	DEBUG_LOG1("open_client_ipc - end");
	return 0;
}

bool CPIActionSupervisorIPC::startThreads(void) {

	DEBUG_LOG1("open_client_ipc_thread - begin");

	int result = 0;

	if( false == isRunningServerThread() ) {
		pthread_attr_init( &serverThreadAttr);
		pthread_attr_setscope( &serverThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &serverThread, &serverThreadAttr, CPIActionSupervisorIPC::fnWaitClient, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for TCP/IP server" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("open_client_ipc_thread - skip - already running - server thread");
	}

	if( false == isRunningClientThread() ) {
		pthread_attr_init( &clientThreadAttr);
		pthread_attr_setscope( &clientThreadAttr, PTHREAD_SCOPE_SYSTEM );
		result = pthread_create( &clientThread, &clientThreadAttr, CPIActionSupervisorIPC::fnCommunicateClient, (void*)this);
		if( result ) {
			ERROR_LOG1( "Unable to start thread for handling client" );
			return false;
		}
	}
	else {
		DEBUG_LOG1("open_client_ipc_thread - skip - already running - client thread");
	}

	DEBUG_LOG1("open_client_ipc_thread - end");
	return true;
}

void CPIActionSupervisorIPC::waitThreads(void) {

	DEBUG_LOG1("open_client_ipc - begin");

	if( true == isRunningServerThread() ) {
		pthread_join( serverThread, (void**)NULL );
	}

	if( true == isRunningClientThread() ) {
		pthread_join( clientThread, (void**)NULL );
	}
	
	DEBUG_LOG1("open_client_ipc - end");
}

void* CPIActionSupervisorIPC::fnCommunicateClient(void* pzArg) {
	DEBUG_LOG1("open_client_ipc - begin");
	CPIActionSupervisorIPC* instance;
	CPIClientStub* pClient;
    size_t recvCount;

	instance = reinterpret_cast<CPIActionSupervisorIPC*>(pzArg);

	instance->runningClientThread = true;

	while(instance->isContinue && (false== Doc.getStop())) {
		
		// ----------
		pthread_mutex_lock( &instance->mutexClient);
		//DEBUG_LOG1("open_client_ipc - client wait...");
		while(STATE_SIG != instance->stateClient) {
			/* int err = */ pthread_cond_wait(&instance->condClient, &instance->mutexClient);
			//DEBUG_LOG("open_client_ipc - pthread_cond_wait - %d", err);
		}
		if(false == instance->isContinue || (true == Doc.getStop())) {
			break;
		}
		pthread_mutex_unlock( &instance->mutexClient);
		// ----------

		usleep(1000*100);

		CPIClientStub::LIST::iterator itr = instance->client.begin();
		while(itr != instance->client.end()) {
			pClient = *itr;
			pClient->selectTCP();
			switch(pClient->tcpState) {
				case CPICommunicator::TCPSTATE_NOTCONNECTED:
					close( pClient->tcpFd );
					pClient->state = CPICommunicator::STATE_INACTIVE;
					//DEBUG_LOG( "Client peer(%s) disconnected - fd(%d)", pClient->ipString.c_str(), pClient->tcpFd );

					pthread_mutex_lock( &instance->mutexClient);
					delete pClient;
					pClient = NULL;
					itr = instance->client.erase(itr);
					if( 0 == instance->client.size() ) {
						instance->stateClient = STATE_WAIT;
					}
					pthread_mutex_unlock( &instance->mutexClient);

					break;
				case CPICommunicator::TCPSTATE_ONRECV: 
					pClient->recvTCP();
					pthread_mutex_lock( &pClient->mutexRecv );
					recvCount = pClient->listRecv.size();
					if( 0 < recvCount ) {
						std::list<CPIPacket>::iterator itr = pClient->listRecv.begin();
						while(itr != pClient->listRecv.end()) {
							instance->parseClientPacket( pClient, *itr);
							itr = pClient->listRecv.erase(itr);
						}
					}
					pthread_mutex_unlock( &pClient->mutexRecv );
				default: ++itr; break;
			}
		}
	}

	instance->runningClientThread = false;
	DEBUG_LOG1("open_client_ipc - end");

	return NULL;
}

void* CPIActionSupervisorIPC::fnWaitClient(void* pzArg) {
	DEBUG_LOG1("open_client_ipc - begin");

	CPIActionSupervisorIPC* instance;
	CPIClientStub* pClient;
	struct sockaddr_in clientAddr;
	int clientFd;
	int sockAddrLen;

	instance = reinterpret_cast<CPIActionSupervisorIPC*>(pzArg);
	instance->runningServerThread = true;
	sockAddrLen= sizeof(clientAddr);
	
	DEBUG_LOG("open_client_ipc - sererfd(%d)", instance->serverFd);

	fd_set set;
	struct timeval timeout;
	int rv;

	while(instance->isContinue && (false == Doc.getStop())) {
		FD_ZERO(&set);
		FD_SET(instance->serverFd, &set);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
    retry:
		rv = select(instance->serverFd+1, &set, NULL, NULL, &timeout);
		if( -1 == rv ) {
            if (errno == EINTR)
            {
                goto retry;
            }
            
			if(instance->isContinue && (false == Doc.getStop())) {
				ERROR_LOG( "server socket error: %s", strerror(errno) );
			}
			break; // error
		}
		else if( 0 == rv )  {
			//DEBUG_LOG1("open_client_ipc - timeout");
			continue; // timeout
		}
		else {
			clientFd = accept( instance->serverFd, (struct sockaddr*)&clientAddr, (socklen_t*)&sockAddrLen );
			if( clientFd  < 0 ) {
				ERROR_LOG( "Unable to accept : %s", strerror(errno) );
				break;
			}
			//DEBUG_LOG1("open_client_ipc - accept");
		}

		//	Create new client
		pClient = new CPIClientStub();
		pClient->tcpFd = clientFd;
		pClient->sockAddr = clientAddr;
		pClient->sockLen = sockAddrLen;
		pClient->state = CPICommunicator::STATE_ACTIVE;
		pClient->tcpState = CPICommunicator::TCPSTATE_CONNECTED;

		pthread_mutex_lock( &instance->mutexClient);
		instance->stateClient = STATE_SIG;
		instance->client.push_back(pClient);
		pthread_mutex_unlock( &instance->mutexClient);
		pthread_cond_signal(&instance->condClient);

		int result = getpeername( clientFd, (struct sockaddr*)&clientAddr, (socklen_t*)&sockAddrLen );
		pClient->ipString = (result<0)?"Unknown":inet_ntoa(clientAddr.sin_addr);
		//DEBUG_LOG( "peer( %s ) connected - fd(%d)", pClient->ipString.c_str(), clientFd);
	}
	
	instance->runningServerThread = false;
	DEBUG_LOG1("open_client_ipc - end");
	return NULL;
}

bool CPIActionSupervisorIPC::initialize(void) {

	DEBUG_LOG1("open_client_ipc - begin");
	if( true == isInitialized() ) {
		return true;
	}

	pthread_mutex_init( &mutexClient, 0 );
	pthread_cond_init( &condClient, 0 );
	stateClient = STATE_WAIT;
	isContinue = true;

	initializeTCP();

	DEBUG_LOG1("open_client_ipc - end");

	return CPIObject::initialize();
}

bool CPIActionSupervisorIPC::initializeTCP(void) {
	
	close(serverFd);
	serverFd = -1;
	serverFd = socket( AF_INET, SOCK_STREAM, 0 );
	if( serverFd < 0 ) {
		ERROR_LOG( "Unable to create TCP/IP server socket : %s", strerror(errno) );
		return false;
	}

	memset( &serverAddr, 0x00, sizeof( serverAddr ));
	serverAddr.sin_family= AF_INET;
	serverAddr.sin_addr.s_addr= htonl(INADDR_ANY);
	serverAddr.sin_port = htons( serverPort );

	int optval=1;
	if( setsockopt( serverFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int))) {
		ERROR_LOG( "Unable to set socket option(SO_REUSEADDR): %s", strerror(errno) );
		return false;
	}

	optval= 262144;
	if( setsockopt( serverFd, SOL_SOCKET, SO_RCVBUF, (const void*)&optval, sizeof(int))) {
		ERROR_LOG( "Unable to set socket option(SO_RCVBUF): %s", strerror(errno));
		return false;
	}

	optval= 262144;
	if( setsockopt( serverFd, SOL_SOCKET, SO_SNDBUF, (const void*)&optval, sizeof(int))) {
		ERROR_LOG( "Unable to set socket option(SO_SNDBUF): %s", strerror(errno));
		return false;
	}	

	if( bind( serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))<0 ) {
		ERROR_LOG( "Unable to bind TCP/IP server socket : %s", strerror(errno) );
		return false;
	}

#define DEFAULT_BACKLOG	5
	if( listen( serverFd, DEFAULT_BACKLOG )<0 ) {
		ERROR_LOG( "Unable to listen to TCP/IP server socket : %s", strerror(errno) );
		return false;
	}
#undef	DEFAULT_BACKLOG

	return true;
}

bool CPIActionSupervisorIPC::finalize(void) {
	DEBUG_LOG1("open_client_ipc - begin");

	clear();
	close(serverFd);
	serverFd = -1;
	pthread_mutex_unlock( &mutexClient); 
	clearClient();
	pthread_mutex_destroy( &mutexClient); 
	pthread_cond_destroy( &condClient); 
	
	DEBUG_LOG1("open_client_ipc - end");

	return CPIObject::finalize();
}

void CPIActionSupervisorIPC::clearClient(void) {
	pthread_mutex_lock( &mutexClient); 
	size_t size = client.size();
	if( 0 < size ) {
		CPIClientStub::LIST::iterator itr = client.begin();
		while(itr != client.end()) {
			CPIClientStub* pClient = *itr;
			if( NULL != pClient) {
				delete pClient;
				pClient = NULL;
			}
		}
		client.clear();
	}
	pthread_mutex_unlock( &mutexClient); 
}

int CPIActionSupervisorIPC::stop(void) {
	DEBUG_LOG1("open_client_ipc - begin");
	
	isContinue = false;
	close(serverFd);
	serverFd = -1;
	stateClient = STATE_SIG;
	pthread_cond_signal(&condClient);
	waitThreads();
	finalize();

	DEBUG_LOG1("open_client_ipc - end");
	return 0;
}

CPIActionSupervisorIPC& CPIActionSupervisorIPC::getInstance(void) {
	static CPIActionSupervisorIPC instance;
	return instance;
}

								
bool CPIActionSupervisorIPC::parseClientPacket( CPIClientStub* pClient, CPIPacket& packet) {

	// ----------
	//DEBUG_LOG("client(%s:%d) - command(%d) - param_count(%d)", pClient->ipString.c_str(), pClient->tcpPort, packet.getCommand(), packet.getParamCount());
	// ----------

	int resultCode = 0;
	std::string resultValue = "";
	processPacket(packet, resultCode, resultValue);
	sendResponse(pClient, packet.getCommand(), resultCode, resultValue);

	return true;
}

bool CPIActionSupervisorIPC::sendResponse(CPIClientStub* pClient, const int command, const int resultCode, const std::string& resultValue) {

	CPIPacket packet;
    
#ifdef FIXME
	packet.support.bigEndian = ConfigMan.socket.bigEndian;
	packet.support.unsignedType = ConfigMan.socket.unsignedType;
#endif
    
	packet.support.unsignedType = false;
	packet.setCommand(command);

	packet.add_int(resultCode);
	if( 0 < resultValue.length() ) {
		packet.add_string(resultValue);
	}
	
	pthread_mutex_lock( &pClient->mutexSend);
	pClient->listSend.push_back(packet);
	pthread_mutex_unlock( &pClient->mutexSend);
	pClient->sendTCP();
	
	return true;
}

/*
 // 에이전트에서 send() 함수 호출하면 이 함수가 응대함.
 static errno_t
 ctl_send( kern_ctl_ref pKctlRef, unsigned int nUnit, void* pUnitInfo, mbuf_t pMsgBuf, int nFlags )
 {
     PCOMMAND_MESSAGE pCmdMsg   = NULL;
     errno_t          nError    = 0;
     size_t           nMsgSize  = 0;
     PARAM_KCTL       ParamKctl;
 
     memset( &ParamKctl, 0, sizeof(ParamKctl) );
     ParamKctl.pKctlRef  = pKctlRef;
     ParamKctl.pMsgBuf   = pMsgBuf;
     ParamKctl.pUnitInfo = pUnitInfo;
     ParamKctl.nUnit      = nUnit;
     ParamKctl.nFlags     = nFlags;
     
     nMsgSize = mbuf_len( pMsgBuf );
     pCmdMsg  = (PCOMMAND_MESSAGE)mbuf_data( pMsgBuf );
     if(pCmdMsg == NULL || nMsgSize < (FIELD_OFFSET(COMMAND_MESSAGE, Command) + sizeof(SMARTDRV_COMMAND)))
     {
         LOG_MSG("[DLP][%s] parameter error(0x%08x%08x, %lu). \n", __FUNCTION__, P64((int64_t)pCmdMsg), nMsgSize);
         return EINVAL;
     }
     
     nError = SmartCmd_Parser_CM( pCmdMsg, &ParamKctl );
     return nError;
 }

 */

//extern int _SmartCmd_SetDrivePolicy(void* pCmdMsg);

bool CPIActionSupervisorIPC::processPacket(CPIPacket& packet, int& resultCode, std::string& resultValue) {

    PCOMMAND_MESSAGE pCmdMsg = (PCOMMAND_MESSAGE)packet.getBinary(0);
    
    if (pCmdMsg != NULL) {
        resultCode = SmartCmd_Parser_CM( pCmdMsg, NULL );
    }

//    int nError = 0;
//
//	switch(packet.getCommand()) {
//        case SetDrivePolicy:
//            pCmdMsg = (PCOMMAND_MESSAGE)packet.getBinary(0);
//            nError = SmartCmd_SetDrivePolicy(pCmdMsg);
//            break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_TEST_N:
//			onTest(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_REGISTER_N :
//			onSyncRegister(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_ASYNC_UNREGISTER_N :
//			onAsyncUnresiter(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_ASYNC_ONCHANGE_ONOFFLINE_N :
//			onAsyncOnChangeOnOffline(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_ASYNC_ONCHANGE_POLICY_N :
//			onAsyncOnChangePolicy(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_ASYNC_TTL_N :
//			onAsyncTTL(packet, resultCode, resultValue);
//			break;
//		//start: Redmine #148385 ----------
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_QUARANTINEFILE_N :
//			onSyncQuarantineFile(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_DELETE_QUARANTINEDFILE_N :
//			onSyncDeleteQuarantinedFile(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor:: PIPE_COMMAND_PISUPERVISOR_SYNC_RELEASE_QUARANTINEDFILE_N :
//			onSyncReleaseQuarantinedFile(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_QUARANTINE_ADD_PID_N :
//			onSyncQuarantineAddPid(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_QUARANTINE_REMOVE_PID_N :
//			onSyncQuarantineRemovePid(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_GET_QUARANTINE_ROOTPATH_N :
//			onSyncGetQuarantineRootPath(packet, resultCode, resultValue);
//			break;
//		case nsPISupervisor::PIPE_COMMAND_PISUPERVISOR_SYNC_GET_QUARANTINE_DISKSIZE_N :
//			onSyncGetQuarantineDiskSize(packet, resultCode, resultValue);
//			break;
//		//end: Redmine #148385 ----------
//		default : break;
//	}
	
	return true;
}

//
//bool CPIActionSupervisorIPC::onSyncRegister(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//
//	DEBUG_LOG1("begin");
//
//	std::string param = packet.get_string(0);
//	bool result = EventHandler.onSyncRegister(param);
//	resultCode = (true == result) ? 1 : 0;
//
//	DEBUG_LOG("end - param:%s - result_code:%d", param.c_str(), resultCode);
//	INFO_LOG("ipc recv - register - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onAsyncUnresiter(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//
//	DEBUG_LOG1("begin");
//
//	std::string param = packet.get_string(0);
//	EventHandler.addEvent(nsPISupervisor::EVENT_UNREGISTER, param);
//
//	resultCode = 1;
//
//	DEBUG_LOG("end - param:%s - result_code:%d", param.c_str(), resultCode);
//	INFO_LOG("ipc recv - unregister - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onAsyncOnChangeOnOffline(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//
//	DEBUG_LOG1("begin");
//
//	std::string param = packet.get_string(0);
//	EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_ONOFFLINEMODE, param);
//
//	resultCode = 1;
//
//	DEBUG_LOG("end - param:%s - result_code:%d", param.c_str(), resultCode);
//	INFO_LOG("ipc recv - on_change_onffline - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onAsyncOnChangePolicy(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//
//	DEBUG_LOG1("begin");
//
//	std::string param;
//	EventHandler.addEvent(nsPISupervisor::EVENT_CHANGED_DLPPOLICY, param);
//
//	resultCode = 1;
//
//	DEBUG_LOG("end - result_code:%d", resultCode);
//	INFO_LOG("ipc recv - on_change_policy - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onAsyncTTL(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//
//	DEBUG_LOG1("begin");
//
//	CPIEvent event;
//	event.setId(nsPISupervisor::EVENT_TTL);
//	event.addParam(packet.get_string(0));
//	event.addParam(packet.get_string(1));
//	EventHandler.addEvent(event);
//
//	resultCode = 1;
//
//	DEBUG_LOG("end - param_1:%s - param_2:%s - result_code:%d",
//			packet.get_string(0).c_str(), packet.get_string(1).c_str(), resultCode);
//	INFO_LOG("ipc recv - ttl - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onTest(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//
//	DEBUG_LOG1("begin");
//
//	//----------
//	//do test here
//	DeviceMan.test();
//	//PIAgentStub.test();
//	//----------
//
//	resultCode = 1;
//
//	DEBUG_LOG("end - result_code:%d", resultCode);
//	INFO_LOG("ipc recv - test - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncQuarantineFile(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	std::string orgFilePath = packet.get_string(0);
//	std::string encryptedFile = packet.get_string(1);
//	std::string returnFilePath = "";
//	int errorCode = 0;
//
//	bool result = WallMan.quarantineFile(orgFilePath, encryptedFile, returnFilePath, errorCode);
//
//	resultCode = (true == result)?1:errorCode;
//	resultValue = returnFilePath;
//
//	DEBUG_LOG("end - param1:%s - param2:%s - result_code:%d - result_value:%s", orgFilePath.c_str(), encryptedFile.c_str(), resultCode, resultValue.c_str());
//	INFO_LOG("ipc recv - on_quarantine_file - result_code:%d, result_value:%s", resultCode, resultValue.c_str());
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncDeleteQuarantinedFile(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	std::string filePath = packet.get_string(0);
//	int errorCode = 0;
//
//	bool result = WallMan.deleteQuarantinedFile(filePath, errorCode);
//
//	resultCode = (true == result)?1:errorCode;
//
//	DEBUG_LOG("end - param:%s - result_code:%d", filePath.c_str(), resultCode);
//	INFO_LOG("ipc recv - on_delete_quarantined_file - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncReleaseQuarantinedFile(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	std::string quarantinedFilePath = packet.get_string(0);
//	std::string targetPath = packet.get_string(1);
//	std::string orgFilePath = packet.get_string(2);
//	std::string returnFilePath = orgFilePath;
//	int errorCode = 0;
//
//	bool result = WallMan.releaseQuarantinedFile(quarantinedFilePath, targetPath, returnFilePath, errorCode);
//
//	resultCode = (true == result)?1:errorCode;
//	resultValue = returnFilePath;
//
//	DEBUG_LOG("end - param_1:%s - param_2:%s - result_code:%d - result_value:%s",
//			quarantinedFilePath.c_str(), targetPath.c_str(), resultCode, resultValue.c_str());
//	INFO_LOG("ipc recv - on_release_quarantined_file - result_code:%d, result_value:%s", resultCode, resultValue.c_str());
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncQuarantineAddPid(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	pid_t pid = (pid_t) packet.get_int(0);
//
//	bool result = WallMan.addPid(pid);
//
//	resultCode = (true == result)?1:0;
//
//	DEBUG_LOG("end - param:%d - result_code:%d", pid, resultCode);
//	INFO_LOG("ipc recv - on_quarantine_add_pid - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncQuarantineRemovePid(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	pid_t pid = (pid_t) packet.get_int(0);
//
//	bool result = WallMan.removePid(pid);
//
//	resultCode = (true == result)?1:0;
//
//	DEBUG_LOG("end - param:%d - result_code:%d", pid, resultCode);
//	INFO_LOG("ipc recv - on_quarantine_remove_pid - result_code:%d", resultCode);
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncGetQuarantineRootPath(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	std::string result;
//	int errorCode = 0;
//	result = WallMan.getRootPath();
//
//	resultCode = (0 < result.length())?1:errorCode;
//	resultValue = result;
//
//	DEBUG_LOG("end - result_code:%d - result_value:%s", resultCode, resultValue.c_str());
//	INFO_LOG("ipc recv - on_get_quarantine_rootpath - result_code:%d, result_value:%s", resultCode, resultValue.c_str());
//
//	return true;
//}
//
//bool CPIActionSupervisorIPC::onSyncGetQuarantineDiskSize(CPIPacket& packet, int& resultCode, std::string& resultValue) {
//	DEBUG_LOG1("begin");
//
//	std::string result;
//	int errorCode = 0;
//	result = WallMan.getDiskSize();
//
//	resultCode = (0 < result.length())?1:errorCode;
//	resultValue = result;
//
//	DEBUG_LOG("end - result_code:%d - result_value:%s", resultCode, resultValue.c_str());
//	INFO_LOG("ipc recv - on_get_quarantine_disksize - result_code:%d, result_value:%s", resultCode, resultValue.c_str());
//
//	return true;
//}


bool CPIActionSupervisorIPC::isRunningServerThread(void) {
    return runningServerThread;
}

bool CPIActionSupervisorIPC::isRunningClientThread(void) {
    return runningClientThread;
}

#endif // #ifndef _PIACTIONSUPERVISORIPC_CPP
