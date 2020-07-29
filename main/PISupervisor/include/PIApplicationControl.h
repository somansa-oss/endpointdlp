#ifndef	_PIAPPLICATIONCONTROL_H
#define	_PIAPPLICATIONCONTROL_H

#include "PIObject.h"
class CDLPPolicyApplication;

class CPIApplicationControl : public CPIObject {
	public:
		CPIApplicationControl();
		virtual ~CPIApplicationControl();

	public:
		class CResult {
			public:
				CResult() { 
					pid = 0;
					value = ALLOW;
				};
				~CResult() {};

			public:
				typedef enum _CONTROL_RESULT { 
					ALLOW = 0, 
					BLOCK_FILEPATH, 
					BLOCK_PROCESSNAME, 
					BLOCK_BINARY
				} CONTROL_RESULT;

			public:
				std::string guid;
				std::string processName;
				int pid;
				int value;

			public:
				std::string valueAsString(void) {
					std::string result = "";
					switch(value) {
						case ALLOW : result = "ALLOW"; break;
						case BLOCK_FILEPATH : result = "BLOCK_FILEPATH"; break;
						case BLOCK_PROCESSNAME : result = "BLOCK_PROCESSNAME"; break;
						case BLOCK_BINARY : result = "BLOCK_BINARY"; break;
						default : break;	
					}
					return result;
				};
		};

	private:
		std::string getProcessControlAsString(void);
		bool blockWithFilePath(CDLPPolicyApplication* pDLPPolicy, std::string filePath, CResult& result);
		bool blockWithProcessName(CDLPPolicyApplication* pDLPPolicy, std::string processName, CResult& result);
		bool blockWithBinary(CDLPPolicyApplication* pDLPPolicy, std::string filePath, CResult& result);
		bool blockWithBinary_experimental(CDLPPolicyApplication* pDLPPolicy, std::string filePath, CResult& result);
		bool isInAllowedList(std::string ticket, std::string filePath, std::string processName);

	public:
		bool writeProcessControlXml(void);
		int  isProcessAllow(std::string ticket, int pid, std::string filePath, std::string processName, CResult& result);
		bool getProcessControl(bool& control, bool& log);
        bool ReqeustProcessTerminate( pid_t nPID, char* pczFilePath );

	public:
		static void logCallback(std::string& log);
};


#endif // #ifndef _PIAPPLICATIONCONTROL_H
