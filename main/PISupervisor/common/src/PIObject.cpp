#ifndef	_PIOBJECT_CPP
#define	_PIOBJECT_CPP

#include <sstream>

#include"PIObject.h"

////////////////////////////////////////////////////////////////////////////////
//class CPIObject
CPIObject::CPIObject() {
	clear();
}

CPIObject::~CPIObject() {
}

bool CPIObject::initialize(void) {
	initialized = true;
	return true;
}

bool CPIObject::finalize(void) {
	initialized = false;
	return true;
}

void CPIObject::clear(void) {
	initialized = false;
}

bool CPIObject::isInitialized(void) {
	return initialized;
}

////////////////////////////////////////////////////////////////////////////////
//class CPIParam 
const char CPIParam::DELIMETER = '^';
CPIParam::CPIParam() {
	clear();
}

CPIParam::~CPIParam() {
}

void CPIParam::addParam(std::string param) {
	if( 0 < this->param.length() ) {
		this->param += CPIParam::DELIMETER;
	}

	this->param += param;
}

void CPIParam::setParam(std::string param) {
	this->param = param;
}

std::string CPIParam::getParam(void) const {
	return param;
}

void CPIParam::clear(void) {
	CPIObject::clear();	

	param = "";
	paramList.clear();
}

bool CPIParam::parse(void) {
	paramList.clear();
	std::istringstream is(param);
	std::string token; // , processId, onOffLine;
	while(std::getline(is, token, CPIParam::DELIMETER)) {
		paramList.push_back(token);
	}

	return (0 == paramList.size()) ? false : true;
}

std::string CPIParam::at(const int index) {
	if( index >= paramList.size() ) {
		return "";
	}

	return paramList[index];
}
#endif // #ifndef _PIOBJECT_CPP
