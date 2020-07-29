#ifndef	_LOGWRITER_H
#define	_LOGWRITER_H	1

#include"include_system.h"
#include"include_commonlib.h"

class CLogWriter {
	public:
		CLogWriter();
		~CLogWriter();

	protected:
		int fd;
		int pid;
		std::string path;
		std::string header;

		bool bActive;
		bool bInitialized;

		pthread_mutex_t mutexLog;

	protected:
		bool writeRaw( const char* message );
		void closeFile( std::string path );
		std::string getHeaderString(void);
	
	public:
		bool isActive();
		bool isInitialized();

		bool setHeader( std::string header );
		std::string getHeader();
		bool setPath( std::string path );
		std::string getPath();
		void setActive(bool active);

		void clear(void);
		bool initialize();
		bool initializeEx( std::string path );
		bool finalize();
		bool finalizeEx( std::string path );

		void log( const char* format, ... );
		void logEx( const char* msg  );

	public:
		static CLogWriter& getInfo(void);
		static CLogWriter& getDebug(void);
		static CLogWriter& getError(void);
};
#define INFO_LOG1(x) CLogWriter::getInfo().log(x)
#define INFO_LOG(x,y...) CLogWriter::getInfo().log(x,y)
#define DEBUG_LOG1(x) CLogWriter::getDebug().log("%s(%d) - "#x, __FUNCTION__, __LINE__)
#define DEBUG_LOG(x,y...) CLogWriter::getDebug().log("%s(%d) - "#x, __FUNCTION__, __LINE__, y)
#define ERROR_LOG1(x) CLogWriter::getError().log("%s(%d) - "#x, __PRETTY_FUNCTION__, __LINE__)
#define ERROR_LOG(x,y...) CLogWriter::getError().log("%s(%d) - "#x, __PRETTY_FUNCTION__, __LINE__, y)
#endif
