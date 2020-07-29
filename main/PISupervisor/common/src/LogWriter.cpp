#ifndef	_LOGWRITER_CPP
#define	_LOGWRITER_CPP

#include "LogWriter.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "PIUtility.h"

CLogWriter::CLogWriter() { 
	clear(); 
	pthread_mutex_init( &mutexLog, 0 );
}

CLogWriter::~CLogWriter() {
	pthread_mutex_unlock( &mutexLog); 
	pthread_mutex_destroy( &mutexLog); 
}

void CLogWriter::clear(void) {
	pid = 0;
	fd = -1;
	path.clear();
	header.clear();
	bActive = true;
	bInitialized = false;
}

bool CLogWriter::writeRaw( const char* message ) {

	pthread_mutex_lock( &mutexLog);

	initialize();

	if( NULL == message ) {
		pthread_mutex_unlock( &mutexLog);
		return false;
	}

	long msgLen = strlen(message);
	if(!bInitialized) {
		fprintf( stderr, "%s\n", message );
		pthread_mutex_unlock( &mutexLog);
		return false;
	}

	if( write( fd, message, msgLen)<0) {
		fprintf( stderr, "[LogWriter] %s\n", strerror(errno));
		pthread_mutex_unlock( &mutexLog);
		return false;
	}

	finalize();

	pthread_mutex_unlock( &mutexLog);
	return true;
}

bool CLogWriter::isActive()	{ 
	return bActive; 
}

void CLogWriter::setActive(bool active)	{ 
	bActive = active;
}

bool CLogWriter::isInitialized() { 
	return bInitialized; 
}

bool CLogWriter::setHeader( std::string header ) {
	this->header.clear();
	this->header = header;

	return ( 0 == this->header.length() ) ? false : true;
}

std::string CLogWriter::getHeader() { 
	return header; 
}

bool CLogWriter::setPath( std::string path ) {
	this->path.clear();
	this->path = path;

	return ( 0 == this->path.length() ) ? false : true;
}

std::string CLogWriter::getPath() { 
	return path; 
}

bool CLogWriter::initialize() { 
	pid = getpid();
	
	return initializeEx( path ); 
}
	
void CLogWriter::closeFile( std::string path ) {

	if( 0 == path.length() ) {
		return;
	}

	if( ( STRING_STDOUT != path ) && ( STRING_STDERR != path ) && (0 < fd) ) {
		close( fd );
		fd = -1;
	}
}

bool CLogWriter::initializeEx( std::string path ) {

	//	Close opened logfile.
	closeFile(path);

	if( 0 == path.length() ) {
		fd = fileno(stdout);
		bInitialized = true;
		return true;
	}

	// ----------
	time_t t1;
	struct tm localTime;
	time( &t1 );
	if (NULL == localtime_r(&t1, &localTime)) {
		fprintf( stderr, "[LogWriter] localtime_r() failed\n");
		fd = fileno(stdout);
		bInitialized = true;
		return true;
	}

	char pathDateTime[1024] = {0x00,};
	sprintf( pathDateTime, "%s/%04d%02d%02d-pisupervisor-%s.log", 
			path.c_str(), 1900+localTime.tm_year, 1+localTime.tm_mon, localTime.tm_mday, header.c_str());
	// ----------

	CPIUtility util;
	if( false == util.makeDir(path, 0755) ) {
		return false;
	}

	if((fd = open( pathDateTime, O_CREAT|O_APPEND|O_RDWR|O_SYNC, S_IRUSR|S_IWUSR ))<0 ) {
		fprintf( stderr, "[LogWriter] %s\n", strerror(errno));
		bInitialized = false;
		return false;
	}

	bInitialized = true;
	return true;
}

bool CLogWriter::finalize() { 
	return finalizeEx( path ); 
}

bool CLogWriter::finalizeEx( std::string path ) {
	bInitialized = false;
	//	Close opened logfile.
	closeFile(path);
	return true;
}
	
std::string CLogWriter::getHeaderString(void) {
	
	time_t t1;
	struct tm localTime;
	time( &t1 );
	if (NULL == localtime_r(&t1, &localTime)) {
		fprintf( stderr, "[LogWriter] localtime_r() failed\n");
		return "";
	}

	char* headerString;
	int size = asprintf(&headerString, "%04d-%02d-%02d %02d:%02d:%02d\t[pid:%d]\t[%s]\t", 
			1900+localTime.tm_year, 1+localTime.tm_mon, localTime.tm_mday,
			localTime.tm_hour, localTime.tm_min, localTime.tm_sec,
			pid, header.c_str());

	if( 0 > size ) {
		return "";
	}

	std::string result;
	result = headerString;

	free(headerString);
	headerString = NULL;

	return result;
}

void CLogWriter::logEx( const char* msg  ) {

	if( false == isActive() ) {
		return;
	}
	
	if( NULL == msg ) {
		fprintf( stderr, "[LogWriter] empty msg\n");
		return;
	}

	std::string headerString = getHeaderString();
	if( 0 == headerString.length() ) {
		writeRaw(msg);
	}
	else {
		long headerLen = headerString.length();
		long msgLen = strlen(msg);
		long msgNewLen = headerLen + msgLen;

		char* msgNew = (char*)malloc(msgNewLen + 1);
		strcpy(msgNew, headerString.c_str());
		strcat(msgNew, msg);
		msgNew[msgNewLen] = '\0';

		writeRaw(msgNew);
		free((void*)msgNew);
	}
}

void CLogWriter::log( const char* format, ... ) {

	if( false == isActive() ) {
		return;
	}

	char * logString = NULL;
	long length = 0;
	{
		va_list ap;
		va_start( ap, format );
		length = vsnprintf( 0, 0, format, ap );
		va_end( ap );

		if( 0 < length ) {
			logString = (char*)malloc( length+2);
			memset(logString, 0x00, length+2);
		}
	}

	if( NULL != logString ) {
		va_list ap;
		va_start( ap, format );
		vsprintf( logString, format, ap );
		va_end( ap );

		logString[length] = '\n';
		logEx( logString );

		free(logString);
		logString = NULL;
	}
}

CLogWriter& CLogWriter::getInfo(void) {
	static CLogWriter instance;
	return instance;
}

CLogWriter& CLogWriter::getDebug(void) {
	static CLogWriter instance;
	return instance;
}

CLogWriter& CLogWriter::getError(void) {
	static CLogWriter instance;
	return instance;
}
#endif // #define _LOGWRITER_CPP
