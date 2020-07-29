#ifndef	_PISQLITESTUB_CPP
#define _PISQLITESTUB_CPP

#include <map>

#include "PIDocument.h"
#include "PISQLiteStub.h"
#include "sqlite3.h"

CPISQLiteStub::CPISQLiteStub() { 
	initialize();
}

CPISQLiteStub::~CPISQLiteStub() {
	finalize();
}

bool CPISQLiteStub::initialize(void) {
	CPIObject::initialize();
	return true;
}

bool CPISQLiteStub::finalize(void) { 
	return CPIObject::finalize();
}

void CPISQLiteStub::clear(void) {
	CPIObject::clear();
}

bool CPISQLiteStub::createTableIfNotExist(void* void_conn) {

	if( NULL == void_conn ) {
		return false;
	}

	sqlite3* conn;
	sqlite3_stmt* stmt;
	int rc;
	std::string sql;
	sql = "CREATE TABLE IF NOT EXISTS PISecCommand( "
		"CommandSeq INTEGER, "
		"CreateTime TEXT, "
		"CommandData TEXT);";

	conn = reinterpret_cast<sqlite3*>(void_conn);
	rc = sqlite3_prepare_v2(conn, sql.c_str(), -1,&stmt, NULL);
	if( SQLITE_OK != rc ) {
		std::string errmsg(sqlite3_errmsg(conn));
		//DEBUG_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
		ERROR_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
		sqlite3_finalize(stmt);
		return false;
	}

	rc = sqlite3_step(stmt);
	if( SQLITE_DONE != rc ) {
		std::string errmsg(sqlite3_errmsg(conn));
		//DEBUG_LOG("sql step error: %d:%s", rc, errmsg.c_str());
		ERROR_LOG("sql step error: %d:%s", rc, errmsg.c_str());
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);

	//DEBUG_LOG1("sql ok");
	return true;
}


bool CPISQLiteStub::open(void*& void_conn) {

	sqlite3* conn;
	std::string deviceLogFile = ConfigMan.path.getDeviceLogFile();
	int rc = sqlite3_open(deviceLogFile.c_str(), &conn);
	if( rc ) {
		ERROR_LOG1("open device_log_file failed");
		DEBUG_LOG1("stop - open device_log_file failed");
		return false;
	}
	
	createTableIfNotExist((void*) conn);

	void_conn = reinterpret_cast<void*>(conn);
	return true;
}

bool CPISQLiteStub::close(void* void_conn) {
	if( NULL == void_conn ) {
		return false;
	}

	sqlite3* conn = reinterpret_cast<sqlite3*>(void_conn);
	sqlite3_close(conn);

	return true;
}

bool CPISQLiteStub::push_back(void* void_conn, CPIDeviceLog& deviceLog) {

	if( NULL == void_conn ) {
		return false;
	}

	static int seq = 0;
	std::string createTime;
	std::string deviceLogXml;

	++seq;
	createTime = deviceLog.getLogTime();
	deviceLogXml = deviceLog.string();
	
	sqlite3* conn;
	sqlite3_stmt* stmt;
	int rc;
	std::string sql;
	
	sql = "INSERT INTO PISecCommand(CommandSeq, CreateTime, CommandData) VALUES (?, ?, ?);";
	
	conn = reinterpret_cast<sqlite3*>(void_conn);
	rc = sqlite3_prepare_v2(conn, sql.c_str(), -1,&stmt, NULL);
	if( SQLITE_OK != rc ) {
		std::string errmsg(sqlite3_errmsg(conn));
		DEBUG_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
		ERROR_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
		sqlite3_finalize(stmt);
		return false;
	}

	do {
		rc = sqlite3_bind_int(stmt, 1, seq);
		if( SQLITE_OK != rc ) {
			break;
		}

		rc = sqlite3_bind_text(stmt, 2, createTime.c_str(), (int)createTime.length(), 0);
		if( SQLITE_OK != rc ) {
			break;
		}

		rc = sqlite3_bind_text(stmt, 3, deviceLogXml.c_str(), (int)deviceLogXml.length(), 0);
		if( SQLITE_OK != rc ) {
			break;
		}
	}while(false);

	if( SQLITE_OK != rc ) {
		std::string errmsg(sqlite3_errmsg(conn));
		DEBUG_LOG("sql bind error: %d:%s", rc, errmsg.c_str());
		ERROR_LOG("sql bind error: %d:%s", rc, errmsg.c_str());
		sqlite3_finalize(stmt);
		return false;
	}

	rc = sqlite3_step(stmt);
	if( SQLITE_DONE != rc ) {
		std::string errmsg(sqlite3_errmsg(conn));
		DEBUG_LOG("sql step error: %d:%s", rc, errmsg.c_str());
		ERROR_LOG("sql step error: %d:%s", rc, errmsg.c_str());
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);

	DEBUG_LOG1("sql ok");
	
	return true;
}

bool CPISQLiteStub::push_back(CPIDeviceLog& deviceLog) {

	sqlite3* conn;
	if( false == open((void*&)conn) ) {
		return false;
	}

	push_back((void*)conn, deviceLog);
	close((void*)conn);

	return true;
}

bool CPISQLiteStub::getAll(CPIDeviceLog::VECTOR& deviceLogList) {

	sqlite3* conn;
	sqlite3_stmt* stmt;
	int rc;
	int column_count;
	std::string sql;
	
	if( false == open((void*&) conn) ) {
		return false;
	}
	
	sql = "SELECT CommandSeq, CreateTime, CommandData "
		"FROM PISecCommand "
		"ORDER BY CreateTime ASC ";

	do {
		rc = sqlite3_prepare_v2(conn, sql.c_str(), -1,&stmt, NULL);
		if( SQLITE_OK != rc ) {
			std::string errmsg(sqlite3_errmsg(conn));
			//DEBUG_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
			ERROR_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
			break;
		}

		if( SQLITE_OK != rc ) {
			std::string errmsg(sqlite3_errmsg(conn));
			//DEBUG_LOG("sql bind error: %d:%s", rc, errmsg.c_str());
			ERROR_LOG("sql bind error: %d:%s", rc, errmsg.c_str());
			break;
		}

		column_count = sqlite3_column_count(stmt);
		while(true) {
			rc = sqlite3_step(stmt);
			if (SQLITE_ROW == rc ) {
				CPIDeviceLog deviceLog;
				for(int column = 0; column < column_count; ++column) {
					switch(column) {
						case 0 : deviceLog.seq = sqlite3_column_int(stmt, column); break;
						case 1 : deviceLog.logTime = (const char*) sqlite3_column_text(stmt, column); break;
						case 2 : deviceLog.deviceLogXml = (const char*) sqlite3_column_text(stmt, column); break;
						default : break;
					}
				}

				if( false == deviceLog.deviceLogXml.empty()) {
					deviceLogList.push_back(deviceLog);
				}
			}
			else if( SQLITE_DONE == rc ) {
				//DEBUG_LOG1("sql - no_record");
				break;
			}
			else {
				std::string errmsg(sqlite3_errmsg(conn));
				//DEBUG_LOG("sql step error: %d:%s", rc, errmsg.c_str());
				ERROR_LOG("sql step error: %d:%s", rc, errmsg.c_str());
				break;
			}
		}
	}while(false);

	sqlite3_finalize(stmt);
	close(conn);

	//DEBUG_LOG1("sql ok");
	
	return true;
}

bool CPISQLiteStub::erase(void* void_conn, int seq, std::string createTime) {

	if( NULL == void_conn ) {
		return false;
	}

	sqlite3* conn;
	sqlite3_stmt* stmt;
	int rc;
	std::string sql;
	sql = "DELETE FROM PISecCommand WHERE CreateTime = ? AND CommandSeq = ?";

	bool result = false;
	do {
		conn = reinterpret_cast<sqlite3*>(void_conn);
		rc = sqlite3_prepare_v2(conn, sql.c_str(), -1,&stmt, NULL);
		if( SQLITE_OK != rc ) {
			std::string errmsg(sqlite3_errmsg(conn));
			DEBUG_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
			ERROR_LOG("sql prepare error: %d:%s", rc, errmsg.c_str());
			break;
		}

		do {
			rc = sqlite3_bind_text(stmt, 1, createTime.c_str(), (int)createTime.length(), 0);
			if( SQLITE_OK != rc ) {
				break;
			}

			rc = sqlite3_bind_int(stmt, 2, seq);
			if( SQLITE_OK != rc ) {
				break;
			}
		}while(false);

		if( SQLITE_OK != rc ) {
			std::string errmsg(sqlite3_errmsg(conn));
			DEBUG_LOG("sql bind error: %d:%s", rc, errmsg.c_str());
			ERROR_LOG("sql bind error: %d:%s", rc, errmsg.c_str());
			break;	
		}

		rc = sqlite3_step(stmt);
		if( SQLITE_DONE != rc ) {
			std::string errmsg(sqlite3_errmsg(conn));
			DEBUG_LOG("sql step error: %d:%s", rc, errmsg.c_str());
			ERROR_LOG("sql step error: %d:%s", rc, errmsg.c_str());
			break;	
		}

		result = true;
	}while(false);

	sqlite3_finalize(stmt);

	DEBUG_LOG1("sql ok");
	return result;
}
#endif
