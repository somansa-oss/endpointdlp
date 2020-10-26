#ifndef _PISUPERVISORMAIN_CPP
#define _PISUPERVISORMAIN_CPP

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <map>

#include"include_common.h"
#include"include_commonlib.h"

#include "PIScheduler.h"
#include "PIDocument.h"
#include "PIUtility.h"

bool is_noservice = false;
std::string basePath;

void get_option(int argc, char **argv);
void set_daemon(const bool bValue);
void set_noservice(const bool bValue);
bool get_noservice(void);
void set_base_path(char* path);
void print_help(void);
void print_version(void);
void install(void);
void uninstall(void);
void signalHandler(int piSignal);
void initialize();

/*
void error(const char* msg) {
	perror(msg);
	exit(1);
}
*/

int main(int argc, char **argv) {

	// ----------
	// signal handler
	// ----------
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_handler = signalHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if( sigaction(SIGTERM, &act, 0) < 0) { // == -1) {
		printf("Unable to set SIGTERM...\n");
	}

	// ----------
	// check option
	// ----------
	get_option(argc, argv);

#ifndef __APPLE__
#ifndef LINUX
	// ----------
	// run daemon
	// ----------
	if( false == get_noservice() ) {
		if( daemon(0,0) == -1 ) {
			printf("daemon error: %s", strerror(errno));	
			exit(EXIT_FAILURE);
		}
	}
#endif	
#endif 
	
	// initialize
	initialize();

	INFO_LOG1("begin");

	PITickCount.setEvent(CPITickCount::PISUPERVISOR_ROOT, CPITickCountParent::START); // test

	// run scheduler
	CPIScheduler& scheduler = CPIScheduler::getInstance();
	scheduler.startThreads();
	scheduler.waitThreads();

	PITickCount.setEvent(CPITickCount::PISUPERVISOR_ROOT, CPITickCountParent::END); // test

	INFO_LOG1("end");

	exit(EXIT_SUCCESS);
}

void get_option(int argc, char **argv) {

	int opt;

	while(-1 != (opt = getopt(argc, argv, "d:hinuv"))) {
		switch(opt) {
			case 'd' : set_base_path(optarg); break;
			case 'h' : print_help(); break;
			case 'i' : install(); break;
			case 'n' : set_noservice(true); break;
			case 'u' : uninstall(); break;
			case 'v' : print_version(); break;
			default: break;
		}
	}
}

void set_noservice(const bool bValue) {
	is_noservice = bValue;
}

bool get_noservice(void) {
	return is_noservice;
}

void set_base_path(char* path) {
	if( NULL != path ) {
		basePath = path;
	}

	// ----------
	printf("basePath:%s,%s\n", basePath.c_str(), path);
	// ----------
}

void print_help(void) {

	printf("print help...\n");
	exit(EXIT_SUCCESS);
}

void print_version(void) {

	printf("print version...\n");
	exit(EXIT_SUCCESS);
}

void install(void) {
	
	printf("install...\n");
	exit(EXIT_SUCCESS);
}

void uninstall(void) {
	
	printf("uninstall...\n");
	exit(EXIT_SUCCESS);
}

void signalHandler(int piSignal) {

	switch(piSignal) {
		case SIGTERM : 
			printf("signal received - SIGTERM(%d)", piSignal);
			INFO_LOG("signal received - SIGTERM(%d)", piSignal);
			CPIScheduler::getInstance().stop(); 
			break;
		default : break;
	}
}

void initialize(void) {

	// load config
	ConfigMan.path.basePath = basePath;
	if( false == ConfigMan.initialize() ) {
		printf("config init error\n");
		exit(EXIT_FAILURE);
	}

	setlocale(LC_ALL, "ko_KR.UTF-8");
	
	if( false == ConfigMan.load() ) {
		ConfigMan.save();
	}

	// tickcount
	PITickCount.setLogPath(ConfigMan.path.dlpLogPath);
	PITickCount.setEnable(ConfigMan.log.useTickCount);
	PITickCount.defineEvent();
	
	// apply config

	// set log
	CLogWriter& logInfo = CLogWriter::getInfo();
	logInfo.setPath(ConfigMan.path.dlpLogPath.c_str());
	logInfo.setHeader( "info" );
	logInfo.setActive(ConfigMan.log.info);
	
	CLogWriter& logDebug = CLogWriter::getDebug();
	logDebug.setPath(ConfigMan.path.dlpLogPath.c_str());
	logDebug.setHeader( "debug" );
	logDebug.setActive(ConfigMan.log.debug);

	CLogWriter& logError = CLogWriter::getError();
	logError.setPath(ConfigMan.path.dlpLogPath.c_str());
	logError.setHeader( "error" );
	logError.setActive(ConfigMan.log.error);

	// ensure path
	CPIUtility util;
	util.ensurePath(ConfigMan.path.dlpLogPath, 0755);
	util.ensurePath(ConfigMan.path.dlpConfigPath, 0755);
}
#endif // #ifndef _PISUPERVISORMAIN_CPP
