#ifndef	_PIACTIONCHECKAGENT_H
#define	_PIACTIONCHECKAGENT_H

#include "PIAction.h"
#include "PIObject.h"
#include <map>

class CPIActionCheckAgent : public IPIAction, public CPIObject {
	public:
		CPIActionCheckAgent();
		virtual ~CPIActionCheckAgent();

		// typedef
	public:
		typedef enum _EM_CATEGORY
		{
			CATEGORY_NONE			= 0
				, AGENT_SERVICE 		= 0x0001 // 1
				, PROTECTOR_SERVICE 	= 0x0002 // 2
				, SUPERVISOR_SERVICE 	= 0x0004 // 4
				, FILE_DRIVER 			= 0x0008 // 8
				, MEDIA_DRIVER 			= 0x0010 // 16
				, NETWORK_DRIVER 		= 0x0020 // 32
				, OUTLOOK_PLUGIN 		= 0x0040 // 64
				, CHROME_PLUGIN 		= 0x0080 // 128
				, FIREFOX_PLUGIN 		= 0x0100 // 256
				, PWMANAGER_SERVICE		= 0x0200 // 512
				, PWDISK_DRIVER			= 0x0400 // 1024
				, PWSMART_DRIVER		= 0x0800 // 2048
				, CDS_SERVICE			= 0x1000 // 4096
		}EM_CATEGORY;

		typedef enum _EM_SUBCATEGORY
		{
			SUBCATEGORY_NONE		= 0
				, STARTED 				= 1
				, STARTED_BY_WATCHDOG 	= 2
				, STOPPED 				= 3
				, STOPPED_BY_ADMINTOOL 	= 4
				, UNEXPECTED_ERROR 		= 5
				, NOT_INSTALLED 		= 6
				, TAMPERED 				= 7
				, DISK_USEDSPACE_CHANGED= 8
		}EM_SUBCATEGORY;
		
		typedef std::map<EM_CATEGORY, bool> MAP_SERVICE_STATUS;
		typedef std::map<EM_CATEGORY, EM_SUBCATEGORY> MAP_AGENT_STATUS;

		// variable
	public:	
		pthread_t checkagentThread;
		pthread_attr_t checkagentThreadAttr;
		bool isContinue;
		bool running;
		MAP_SERVICE_STATUS serviceStatus;
		MAP_AGENT_STATUS agentStatus;
		std::vector<std::string> log;
		pthread_mutex_t mutexLog;

	protected:
		bool startThreads(void);
		void waitThreads(void);
		bool isRunning(void);
		bool checkBegin(void);
		bool checkEnd(void);
		bool checkDriver(EM_CATEGORY category);
		bool makeDriverEvent(EM_CATEGORY category, EM_SUBCATEGORY subCategory);
		std::string getLog(EM_CATEGORY category, EM_SUBCATEGORY subCategory);
		std::string getStatus(EM_CATEGORY category, EM_SUBCATEGORY subCategory);
		bool onAgentRegistered(void);
	
	public:
		virtual void clear(void);
		virtual int run(const std::string& param);
		virtual int stop(void);
		bool checkAgent(void);

	public:
		static CPIActionCheckAgent& getInstance(void);
		static void* fnCheckAgent(void* pzArg);
};
#endif // #ifndef _PIACTIONCHECKAGENT_H
