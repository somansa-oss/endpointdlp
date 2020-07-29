#ifndef	_DLPAPPLICATION_H
#define	_DLPAPPLICATION_H

#include "PIObject.h"

////////////////////////////////////////////////////////////////////////////////
//class CDLPApplication
class CDLPApplication : public CPIObject {
	public:
		CDLPApplication();
		virtual ~CDLPApplication();

	public:
		typedef enum _EM_TYPE {
			typeNone = 0
				,typeDefault = 1
				,typeCustom = 2
		}EM_TYPE;

		typedef enum _EM_CONTROL_TYPE {
			controltypeNone = -1
				,controltypeExecControl = 0
				,controltypeFileOpenControl = 1
		}EM_CONTROL_TYPE;

		typedef enum _EM_FILE_OPEN_CONTROL_TYPE {
			fileopencontroltypeNone = 0
				,fileopencontroltypeApplicationHooking = 2
				,fileopencontroltypeFileAccessHooking = 3
		}EM_FILE_OPEN_CONTROL_TYPE;

		typedef std::vector<CDLPApplication> Vector;

		class CBufferType {
			public:
				CBufferType() { 
					data = NULL; 
					length = 0;
					allocSize = 0;
				};
				~CBufferType() {
					if (NULL != data) {
						delete [] data;
						data = NULL;
					}
				};
		
			public:
				char* data;
				size_t length;
				size_t allocSize;
		
			public:
				bool initialize(const size_t size) {
					if (0 == size) {
						return false;
					}
					
					try {
					data = new char[size];
					} catch (std::bad_alloc& ba) {
						return false;
					}

					if (NULL == data) {
						return false;
					}

					allocSize = size;
					reset();

					return true;
				};

				bool isValid(void) {
					if ((NULL == data) || (0 >= allocSize)) {
						return false;	
					}
					return true;
				};

				void reset(void) {
					memset(data, 0x00, allocSize);
					length = 0;
				};
		};

	public:
		CDLPApplication::EM_TYPE type;
		CDLPApplication::EM_CONTROL_TYPE controlType;
		CDLPApplication::EM_FILE_OPEN_CONTROL_TYPE fileOpenControlType;
		std::string guid;
		std::string display;
		std::string fileName;
		std::string binData;
		bool useBinData;
		std::string lowerFileNameExt;
		std::string lowerFileName;
		std::string lowerFileExt;

	private:
		size_t hexToBin(CBufferType& pattern, const std::string& binData) const;
		unsigned char x2c(unsigned char c) const ;
		bool containBinary(int fd, const CBufferType& buffer, const CBufferType& pattern, void(*logCallbak)(std::string& log)) const;

	public:
		virtual void clear(void);
		void setType(std::string type);
		bool isSameProcessName(std::string processName) const;
		bool containBinary(int fd, void(*logCallbak)(std::string& log)) const;
		void setFileName(std::string fileName);
		bool isWindowsExecutable(void) const;
		std::string getFileName(void) const { return fileName; };
};
#endif // #ifndef _DLPAPPLICATION_H
