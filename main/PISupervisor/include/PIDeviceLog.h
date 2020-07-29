#ifndef	_PIDEVICELOG_H
#define	_PIDEVICELOG_H

#include "PIObject.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
//class CPIDeviceLog
class CPIDeviceLog : public CPIObject {
	public:
		CPIDeviceLog();
		virtual ~CPIDeviceLog();

	public:
		typedef std::vector<CPIDeviceLog> VECTOR;

		typedef enum _EM_DEVICETYPE{ 
			typeNone 	= 1, 
			typeDevice 	= 1,
			typeDrive 	= 2,
			typeNdis 	= 3,
			typeProcess = 4
		} EM_DEVICETYPE;

	public:
		// SMART_LOG_RECORD_EX
		unsigned long policyType;

	public:
		// SMART_LOG_RECORD
		unsigned long accessType;
		unsigned long recordType;

	public:
		// LOG_DATA
		std::string logTime;
		unsigned int logType;
		unsigned int logAction;
		bool disableResult;
		unsigned long processId;
		std::string processName;
		std::string deviceName;
		std::string fileName;
		std::string deviceLogXml;
		int seq;
		std::string virtualType;
		std::string guid;
    
        std::string strMajorString;
        std::string strMinorString;
        std::string strOperationString;

	public:
		CPIDeviceLog::EM_DEVICETYPE deviceType;
		struct tm tmLogTime;

	public:
		virtual void clear(void);
		std::string string(void);
		void setLogTime(std::string logTime);
		std::string getLogTime(void);
		std::string getVirtualType(void);
		std::string getOperation(void);
		std::string getOperationCode(void);
		unsigned long getAccess(void);
		std::string getDisableType(void);
		std::string getAdapterBusType(void);
		std::string getDeviceType(void);
		bool isOutDated(const int maxTimeSpan);
		bool isDuplicated(const CPIDeviceLog& deviceLog);
		void setDeviceTypeByPolicyType(const unsigned long policyType);
		void setDeviceName(std::string value);
};
#endif // #ifndef _PIDEVICELOG_H
