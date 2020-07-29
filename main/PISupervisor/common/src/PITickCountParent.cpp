#include "PITickCountParent.h"

CPITickCountParent::CPITickCountParent() {
	logFilePath = "";
	active = false;
	pthread_mutex_init( &mutexLog, 0 );
}

CPITickCountParent::~CPITickCountParent() {
	pthread_mutex_unlock( &mutexLog); 
	pthread_mutex_destroy( &mutexLog); 
}

int CPITickCountParent::writeFile(const std::string &tickCount) {

	if( true == logFilePath.empty() ) {
		return -1;
	}

	pthread_mutex_lock( &mutexLog);
	int fd = -1;
	int result = -1;
	do {
		if((fd = open( getLogFilePath().c_str(), O_CREAT|O_APPEND|O_RDWR|O_SYNC, S_IRUSR|S_IWUSR ))<0 ) {
			fprintf( stderr, "[PITickCount] %s\n", strerror(errno));
			break;
		}

		if( write( fd, tickCount.c_str(), tickCount.length())<0) {
			fprintf( stderr, "[PITickCount] %s\n", strerror(errno));
			break;
		}

		result = 0;
	}while(false);

	if( -1 < fd ) {
		close( fd );
		fd = -1;
	}
	pthread_mutex_unlock( &mutexLog);

	return result;
}

std::string CPITickCountParent::getEventIDAsString(unsigned int eventID) const {

	if( 0 == mapEvent.size() ) {
		return "";
	}

	CPITickCountParent::MAP_EVENT::const_iterator itr = mapEvent.find(eventID);
	if( itr != mapEvent.end() ) {
		return itr->second;
	}

	return "";
}

std::string CPITickCountParent::getStartTime(CPITickCountParent::TYPE type) const {
	if( CPITickCountParent::START != type ) {
		return "";
	}

	return getTickCountAsString(util.getTickCount());
}

std::string CPITickCountParent::getEndTime(CPITickCountParent::TYPE type) const {
	if( ( CPITickCountParent::END != type ) && ( CPITickCountParent::STOP != type ) ) {
		return "";
	}

	return getTickCountAsString(util.getTickCount());
}

std::string CPITickCountParent::getTypeAsString(CPITickCountParent::TYPE type) const {

	std::string result;

	switch(type) {
		case CPITickCountParent::START : result = "start"; break;
		case CPITickCountParent::END: result = "end"; break;
		case CPITickCountParent::STOP: result = "stop"; break;
		default : break;
	}

	return result;
}

bool CPITickCountParent::isActive() const {
	return active;
}

std::string CPITickCountParent::readFile(const std::string & fileName) const {
	struct stat st;
	memset(&st, 0x00, sizeof(st));

	if( 0 != stat(fileName.c_str(), &st) ) {
		return "";
	}

	if( 0 == st.st_size ) {
		return "";
	}

	int fd = -1;
	if( (fd = open(fileName.c_str(), O_RDONLY, S_IRUSR)) < 0 ) {
		return "";
	}

	char* buf = (char*) malloc(st.st_size+1);
	memset(buf, 0x00, st.st_size+1);
	read(fd, buf, st.st_size);
	close(fd);
	fd = -1;

	std::string result = buf;
	free(buf);

	return result;
}

std::string CPITickCountParent::writeFile(const std::string &fileName, const std::string &data) {

	if( true == logFilePath.empty() ) {
		return "";
	}

	int fd = -1;
	int result = -1;
	do {
		if((fd = open( fileName.c_str(), O_CREAT|O_RDWR|O_SYNC, S_IRUSR|S_IWUSR ))<0 ) {
			fprintf( stderr, "[PITickCount] %s\n", strerror(errno));
			break;
		}

		if( write( fd, data.c_str(), data.length())<0) {
			fprintf( stderr, "[PITickCount] %s\n", strerror(errno));
			break;
		}

		result = 0;
	}while(false);

	if( -1 < fd ) {
		close( fd );
		fd = -1;
	}

	return (-1 < result) ? fileName : "";
}

std::string CPITickCountParent::getLogFilePath() {

	char* temp;
	int size = asprintf(&temp, "%s/pitick_%s.csv", 
			logFilePath.c_str(),
			util.getCurrentDate().c_str()
			);

	if( 0 > size ) {
		return "";
	}

	std::string result;
	result = temp;

	if( NULL != temp) {
		free(temp);
		temp= NULL;
	}

	return result;


}

void CPITickCountParent::setLogPath(std::string logPath) {
	logFilePath = logPath;
}

void CPITickCountParent::setEnable(bool enable) {
	active = enable;
}

int CPITickCountParent::setEvent(unsigned int eventID, CPITickCountParent::TYPE type) {
	if( false == isActive() ) {
		return -1;
	}

	char* temp;
	int size = asprintf(&temp, "%s,%s,%s,%s,%s\n", 
			getTypeAsString(type).c_str(),
			util.getCurrentDateTime().c_str(),
			getEventIDAsString(eventID).c_str(),
			getStartTime(type).c_str(),
			getEndTime(type).c_str()
			);

	if( 0 > size ) {
		return -1;
	}

	std::string log;
	log = temp;

	if( NULL != temp) {
		free(temp);
		temp= NULL;
	}

	return this->writeFile(log);
}

int CPITickCountParent::setInfo(std::string info) {

	if( false == isActive() ) {
		return -1;
	}

	char* temp;
	int size = asprintf(&temp, "info,%s,%s,,\n", 
			util.getCurrentDateTime().c_str(),
			info.c_str()
			);

	if( 0 > size ) {
		return -1;
	}

	std::string log;
	log = temp;

	if( NULL != temp) {
		free(temp);
		temp= NULL;
	}

	return this->writeFile(log);
}

std::string CPITickCountParent::getTickCountAsString(unsigned int tickCount) const {

	char* temp;
	int size = asprintf(&temp, "%0.3f", tickCount/1000.0);

	if( 0 > size ) {
		return "";
	}

	std::string result;
	result = temp;

	if( NULL != temp) {
		free(temp);
		temp= NULL;
	}

	return result;
}

