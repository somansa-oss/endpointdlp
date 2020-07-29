#ifndef	_PIAGENTSTUB_H
#define	_PIAGENTSTUB_H

#include "PIObject.h"
#include "PIPacket.h"
#include "include_kernel.h"

class CPIAgentStub : public CPIObject {
	public:
		CPIAgentStub();
		virtual ~CPIAgentStub();

	public:
		std::string name;
		pid_t pid;
		struct tm timeStamp;
		int maxTimeSpan;

	protected:
		bool sendToPIAgent(CPIPacket& packet, int& response);
	
	public:
		virtual bool initialize(void);
		virtual bool finalize(void);

	public:
		virtual void clear(void);
		void setName(const std::string name);
		std::string getName(void) const;
		void setProcessId(const pid_t pid);
		pid_t getProcessId(void) const;
		void pressTimeStamp(void);
		bool isOutDated(void);
		bool isRegistered(const pid_t pid) const;
		bool isRegistered(void) const;
		void setMaxTimeSpan(const int maxTimeSpan);
		int getMaxTimeSpan(void);
		void set(std::string name, const pid_t pid, const int maxTimeSpan);

	public:
		bool notifyEvent(nsPISupervisor::EM_EVENTID eventId, std::string param);
		bool notifyFileDelete(std::string filePath, unsigned int processId, int& result);
		bool notifyDiskFull(std::string jobStatus, std::string filePath, int& result);
        bool notifyNeedFullDiskAccessAuth(int& response);
		bool setDeviceLog(const std::string& deviceLog);
		bool test(void);
		bool analyzeFileForCopyPreventPlus(std::string filePath, std::string tempFilePath, 
				unsigned int deviceCategory, unsigned int processId, int& result);
		bool analyzeFileForPreventPlus(std::string filePath, std::string orgfilePath,  unsigned int processId, ACTION_TYPE emActionType, int& result);
};
#endif
