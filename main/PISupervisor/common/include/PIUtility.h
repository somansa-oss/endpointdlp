#ifndef	_PIUTILITY_H
#define	_PIUTILITY_H

#include "include_system.h"
#include "include_commonlib.h"
#include "include_common.h"

class CPIUtility {
	public:
		CPIUtility();
		virtual ~CPIUtility();

	private:
		static const std::string WHITE_SPACE;

	public:
		bool getValueAsBool(std::string value);
		bool getValueAsBool(int value);
		std::string getValueAsString(int value);
		std::string getValueAsString(bool value);
		std::string getValueAsString(struct tm& localTime);
		std::string getValueAsString(double value);
		bool getValueAsTime(std::string value, struct tm& localTime);
		std::string getCurrentDateTime(void);
		bool getCurrentDateTime(struct tm& currentTime);
		double diffTime(struct tm& localTime);
		void toUpper(std::string& value) const;
		void toLower(std::string& value) const;
		bool replace(std::string& source, std::string tokenOld, std::string tokenNew);
		bool isExistFile(std::string filePath);
		bool makeDir(std::string path, mode_t mode);
		bool ensurePath(std::string path, mode_t mode);
		int getInt(unsigned int value);
		long getLong(unsigned long value);
		long long getLongLong(unsigned long long value);
		int getInt32(unsigned int value);
		long long getInt64(unsigned long long value);
		std::string readCommandOutput(std::string command);
		unsigned int getTickCount() const;
		void removeSubString(std::string &source, const std::string &keyword);
		std::string getCurrentDate(void);
		std::string getCurrentTime(void);
		std::string getFileExt(std::string filePath);
		std::string getFileName(std::string filePath);
		std::string getFileNameExt(std::string filePath);
		std::string getPath(std::string filePath);
		int toInt(const std::string& value);
		std::string ltrim(const std::string value);
		std::string rtrim(const std::string value);
		std::string trim(const std::string value);
};
#endif // #ifndef _PIUTILITY_H

