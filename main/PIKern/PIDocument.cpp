#ifndef	_PIDOCUMENT_CPP
#define	_PIDOCUMENT_CPP

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
//	configMan.finalize();
//	policyMan.finalize();
//	piagentStub.finalize();
//	pisqliteStub.finalize();
//	wallMan.finalize();
//	deviceMan.finalize();

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
#endif // #ifndef _PIDOCUMENT_CPP
