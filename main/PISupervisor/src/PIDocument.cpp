#ifndef	_PIDOCUMENT_CPP
#define	_PIDOCUMENT_CPP

#include <map>
#include"PIDocument.h"

////////////////////////////////////////
// CPIDocument

CPIDocument::CPIDocument() {
	clear();
}

CPIDocument::~CPIDocument() {	
}

bool CPIDocument::initialize(void) {
	return true;
}

bool CPIDocument::finalize(void) {
	configMan.finalize();
	policyMan.finalize();
	piagentStub.finalize();
	pisqliteStub.finalize();

#ifndef LINUX	
	wallMan.finalize();
#endif
	
	deviceMan.finalize();

	return true;
}

void CPIDocument::clear(void) {
	stop = false;
}

CPIDocument& CPIDocument::getInstance(void) {
	static CPIDocument instance;
	return instance;
}

void CPIDocument::setStop(void) {
	this->stop = true;
}

bool CPIDocument::getStop(void) {
	return stop;
}
#endif
