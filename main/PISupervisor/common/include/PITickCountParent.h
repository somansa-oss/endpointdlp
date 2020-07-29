#include <map>
#include <string>
#include "PIUtility.h"

class CPITickCountParent
{
	public:
		CPITickCountParent();
		virtual ~CPITickCountParent();

	public:
		typedef enum _TYPE { START, END, STOP, INFO } TYPE;
		typedef std::map<unsigned int, std::string> MAP_EVENT;

	protected:
		std::string logFilePath;
		bool active;
		MAP_EVENT mapEvent;
		pthread_mutex_t mutexLog;
		CPIUtility util;

	public:
		virtual void defineEvent() = 0;

	protected:
		int writeFile(const std::string &tickCount);
		std::string getEventIDAsString(unsigned int eventID) const;
		std::string getStartTime(CPITickCountParent::TYPE type) const; 
		std::string getEndTime(CPITickCountParent::TYPE type) const;
		std::string getTypeAsString(CPITickCountParent::TYPE type) const;
		bool isActive() const;
		std::string readFile(const std::string &fileName) const;
		std::string writeFile(const std::string &fileName, const std::string &data);
		std::string getLogFilePath();
		unsigned int getTickCount();

	public:
		void setLogPath(std::string logPath);
		void setEnable(bool enable);
		int setEvent(unsigned int eventID, CPITickCountParent::TYPE type);
		int setInfo(std::string info);
		std::string getTickCountAsString(unsigned int tickCount) const;
};
