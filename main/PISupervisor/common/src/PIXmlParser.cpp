#ifndef	_PIXMLPARSER_CPP
#define	_PIXMLPARSER_CPP

#include <sstream>

#include "PIXmlParser.h"

CPIXmlParser::CPIXmlParser() {
}

CPIXmlParser::~CPIXmlParser() {
}

bool CPIXmlParser::initialize(void) {
	DEBUG_LOG1("xml_parser");	
	return true;
}

bool CPIXmlParser::finalize(void) {
	DEBUG_LOG1("xml_parser");	
	return true;
}

bool CPIXmlParser::findPath(std::string path) {
	DEBUG_LOG1("xml_parser");	

	std::vector<std::string> pathList;
	std::istringstream is(path);
	std::string token;
	while(std::getline(is, token, '/')) {
		pathList.push_back(token);
	}

	if( 0 == pathList.size() ) {
		ERROR_LOG("xml_parser - %s", path.c_str());	
		return false;
	}
	
	DEBUG_LOG("xml_parser - node(%d) - path(%s)", pathList.size(), path.c_str());	

	bool result = true;
	markup.ResetPos();
	for(int index = 0; index < pathList.size(); index++) {

		if( false == markup.FindElem(pathList[index].c_str()) ) {
			ERROR_LOG("xml_parser - elem_notfound - %s", pathList[index].c_str());
			result = false;
			break;
		}

		if( (index+1 ) < pathList.size() ) {
			if( false == markup.IntoElem() ) {
				result = false;
				break;
			}
		}
	}

	return result;
}
#endif // #ifndef _PIXMLPARSER_CPP
