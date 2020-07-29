#ifndef	_PIACTIONOPENCLIENTIPC_H
#define	_PIACTIONOPENCLIENTIPC_H

#include "PIAction.h"
#include "PIObject.h"
#include "PIClientStub.h"


class CPIActionOpenClientIPC : public IPIAction, public CPIObject {
	public:
		CPIActionOpenClientIPC();
		virtual ~CPIActionOpenClientIPC();

		// variables
	protected:
		int serverFd;
		struct sockaddr_in serverAddr;
		unsigned int serverPort;
		pthread_t serverThread;
		pthread_attr_t serverThreadAttr;
		pthread_mutex_t mutexClient;
		pthread_cond_t condClient;
		enum { STATE_WAIT, STATE_SIG } stateClient;
		pthread_t clientThread;
		pthread_attr_t clientThreadAttr;
		bool isContinue;
		bool runningServerThread;
		bool runningClientThread;

	public:
		CPIClientStub::LIST client;

		// method
	protected:	
		bool initializeTCP(void);
		bool startThreads(void);
		void waitThreads(void);
		bool isRunningServerThread(void);
		bool isRunningClientThread(void);
		void clearClient(void);
		bool parseClientPacket( CPIClientStub* pClient, CPIPacket& packet);
		bool sendResponse(CPIClientStub* pClient, const int command, const int resultCode, const std::string& resultValue);
		bool processPacket(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncRegister(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onAsyncUnresiter(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onAsyncOnChangeOnOffline(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onAsyncOnChangePolicy(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onAsyncTTL(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onTest(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncQuarantineFile(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncDeleteQuarantinedFile(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncReleaseQuarantinedFile(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncQuarantineAddPid(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncQuarantineRemovePid(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncGetQuarantineRootPath(CPIPacket& packet, int& resultCode, std::string& resultValue);
		bool onSyncGetQuarantineDiskSize(CPIPacket& packet, int& resultCode, std::string& resultValue);

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);

	public:
		virtual int run(const std::string& param);
		virtual int stop(void);
	
	public:
		static CPIActionOpenClientIPC& getInstance(void);
		static void* fnCommunicateClient(void* pzArg);
		static void* fnWaitClient(void* pzArg);
};
#endif // #ifndef _PIACTIONOPENCLIENTIPC_H
