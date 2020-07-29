#ifndef	_PIUTILITY_CPP
#define	_PIUTILITY_CPP

#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include "PIUtility.h"

const std::string CPIUtility::WHITE_SPACE = " \n\r\t\f\v";

CPIUtility::CPIUtility() {
}

CPIUtility::~CPIUtility() {
}

bool CPIUtility::getValueAsBool(std::string value) {
	return ("true" == value) ? true : false;
}

bool CPIUtility::getValueAsBool(int value) {
	return (0 == value) ? false : true;
}

std::string CPIUtility::getValueAsString(int value) {
	char* buffer;
	if( 0 > asprintf(&buffer, "%d", value) ) {
		return "error";
	}

	std::string result = buffer;
	free(buffer);
	return result; 
}

std::string CPIUtility::getValueAsString(double value) {
	char* buffer;
	if( 0 > asprintf(&buffer, "%.2f", value) ) {
		return "error";
	}

	std::string result = buffer;
	free(buffer);
	return result; 
}

std::string CPIUtility::getValueAsString(bool value) {
	return (true == value) ? "true" : "false";
}

std::string CPIUtility::getValueAsString(struct tm& localTime) {
	
	char dateTime[1024] = {0x00,};
	sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", 
			1900+localTime.tm_year, 1+localTime.tm_mon, localTime.tm_mday,
			localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

	return std::string(dateTime);
}

std::string CPIUtility::getCurrentDateTime(void) {

	struct tm localTime;
	getCurrentDateTime(localTime);

	return getValueAsString(localTime);
}

bool CPIUtility::getCurrentDateTime(struct tm& currentTime) {

	memset(&currentTime, 0x00, sizeof(struct tm));

	time_t t1;
	time( &t1 );
	currentTime = *localtime_r( &t1, &currentTime );

	return true;
}

double CPIUtility::diffTime(struct tm& localTime) {
	
	time_t now;
	time(&now);

	double diff = difftime(now, mktime(&localTime));

	return diff;
}

void CPIUtility::toUpper(std::string& value) const {
	std::transform(value.begin(), value.end(), value.begin(), toupper);
}

void CPIUtility::toLower(std::string& value) const {
	std::transform(value.begin(), value.end(), value.begin(), tolower);
}

bool CPIUtility::replace(std::string& source, std::string tokenOld, std::string tokenNew) {

	bool result = false;
	size_t pos = 0;
	while( std::string::npos != (pos = source.find(tokenOld, pos)) ) {
		source.replace(pos, tokenOld.length(), tokenNew); 
		result = true;
	}
	return result;
}

bool CPIUtility::isExistFile(std::string filePath) {
	struct stat st;
	memset(&st, 0x00, sizeof(st));

	bool result = false;
	if( 0 == stat(filePath.c_str(), &st) ) {
		result = true;
	}

	return result;
}

bool CPIUtility::makeDir(std::string path, mode_t mode) {
	struct stat st;
	memset(&st, 0x00, sizeof(st));
	int status = 0;
	if( 0 != stat(path.c_str(), &st) ) {
		mode_t umask_old = umask(0);
		if( (0 != mkdir(path.c_str(), mode)) && EEXIST != errno ) {
			status = -1;
		}
		umask(umask_old);
	}

	return (0 == status) ? true : false;
}

bool CPIUtility::ensurePath(std::string path, mode_t mode) {

	std::vector<std::string> pathList;
	std::istringstream is(path);
	std::string token;
	while(std::getline(is, token, '/')) {
		pathList.push_back(token);
	}

	if( 0 == pathList.size() ) {
		ERROR_LOG("util - %s", path.c_str());	
		return false;
	}

	DEBUG_LOG("util - node:%d - path:%s", pathList.size(), path.c_str());	

	std::string path_temp;
	bool result = true;
	for(int index = 0; index < pathList.size(); index++) {

		path_temp += pathList[index] + "/";
		if( false == makeDir(path_temp, mode) ) {
			result = false;
			break;
		}
	}

	return result;
}

int CPIUtility::getInt(unsigned int value) {
	if( INT_MAX < value ) {
		return -1;
	}

	return value;
}

long CPIUtility::getLong(unsigned long value) {
	if( LONG_MAX < value ) {
		return -1L;
	}

	return value;
}

long long CPIUtility::getLongLong(unsigned long long value) {
	if( LLONG_MAX < value ) {
		return -1LL;
	}

	return value;
}

int CPIUtility::getInt32(unsigned int value) {
	return getInt(value);
}

long long CPIUtility::getInt64(unsigned long long value) {
	return getLongLong(value);
}

bool CPIUtility::getValueAsTime(std::string value, struct tm& localTime) {
	memset((void*)&localTime, 0x00, sizeof(struct tm));
	std::string format = "YYYY-MM-DD HH:mm:ss";
	if( value.length() != format.length() ) {
		return false;
	}

	localTime.tm_year = atoi(value.substr(0,4).c_str()) - 1900;
	localTime.tm_mon = atoi(value.substr(5,2).c_str()) - 1;
	localTime.tm_mday = atoi(value.substr(8,2).c_str());
	localTime.tm_hour = atoi(value.substr(11,2).c_str());
	localTime.tm_min = atoi(value.substr(14,2).c_str());
	localTime.tm_sec = atoi(value.substr(17,2).c_str());

	return true;
}

std::string CPIUtility::readCommandOutput(std::string command) {
#define MAX_COMMAND_LENGTH 1024
#define MAX_OUTPUT_LENGTH 1024

	char buffer[MAX_OUTPUT_LENGTH];
	std::string result = "";
    
	FILE* pipe = popen(command.c_str(), "r");
	if (NULL != pipe)
    {
		while(!feof(pipe))
        {
			if(fgets(buffer, MAX_OUTPUT_LENGTH, pipe) != NULL)
            {
				result += buffer;
			}
		}
		pclose(pipe);
	}

	return result;
}

unsigned int CPIUtility::getTickCount() const {
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

void CPIUtility::removeSubString(std::string &source, const std::string &keyword) {
	std::string::size_type n = keyword.length();
	for( std::string::size_type i = source.find(keyword);
			i != std::string::npos;
			i = source.find(keyword) ) {
		source.erase(i,n);
	}
}

std::string CPIUtility::getCurrentDate(void) {

	struct tm localTime;
	getCurrentDateTime(localTime);

	char dateTime[1024] = {0x00,};
	sprintf(dateTime, "%04d-%02d-%02d", 
			1900+localTime.tm_year, 1+localTime.tm_mon, localTime.tm_mday);

	return std::string(dateTime);
}

std::string CPIUtility::getCurrentTime(void) {

	struct tm localTime;
	getCurrentDateTime(localTime);
	
	char dateTime[1024] = {0x00,};
	sprintf(dateTime, "%02d:%02d:%02d", 
			localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

	return std::string(dateTime);
}

std::string CPIUtility::getFileExt(std::string filePath) {
	if( 0 == filePath.length() ) {
		return "";
	}

	size_t found = filePath.rfind(".");
	if( std::string::npos == found ) {
		return "";
	}

	return filePath.substr(found + 1);
}

std::string CPIUtility::getFileNameExt(std::string filePath) {
	if( 0 == filePath.length() ) {
		return "";
	}

	size_t found = filePath.find_last_of("/");
	if( std::string::npos == found ) {
		return filePath;
	}

	return filePath.substr(found + 1);
}

std::string CPIUtility::getPath(std::string filePath) {
	if( 0 == filePath.length() ) {
		return "";
	}

	size_t found = filePath.find_last_of("/");
	if( std::string::npos == found ) {
		return filePath;
	}

	return filePath.substr(0, found);
}

std::string CPIUtility::getFileName(std::string filePath) {
	std::string fileNameExt = getFileNameExt(filePath);
	if( 0 == fileNameExt.length() ) {
		return "";
	}

	size_t found = fileNameExt.find(".");
	if( std::string::npos == found ) {
		return fileNameExt;
	}

	return fileNameExt.substr(0, found);
}

int CPIUtility::toInt(const std::string& value) {
	return atoi(value.c_str());
}

std::string CPIUtility::ltrim(const std::string value) {
	size_t start = value.find_first_not_of(WHITE_SPACE);
	return (start == std::string::npos)?"":value.substr(start);
}

std::string CPIUtility::rtrim(const std::string value) {
	size_t end = value.find_last_not_of(WHITE_SPACE);
	return (end == std::string::npos)?"":value.substr(0, end + 1);
}

std::string CPIUtility::trim(const std::string value) {
	return rtrim(ltrim(value));
}
#endif // #ifndef _PIUTILITY_CPP
