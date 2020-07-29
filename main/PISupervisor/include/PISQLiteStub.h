#ifndef	_PISQLITESTUB_H
#define	_PISQLITESTUB_H

#include "PIObject.h"
#include "PIDeviceLog.h"

class CPISQLiteStub : public CPIObject {
	public:
		CPISQLiteStub();
		virtual ~CPISQLiteStub();

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);
	
	protected:
		bool createTableIfNotExist(void* void_conn);

	public:
		bool open(void*& void_conn);	
		bool close(void* void_conn);	
		bool push_back(void* void_conn, CPIDeviceLog& deviceLog);
		bool push_back(CPIDeviceLog& deviceLog);
		bool getAll(CPIDeviceLog::VECTOR& deviceLogList);
		bool erase(void* void_conn, int seq, std::string createTime);
};
#endif
