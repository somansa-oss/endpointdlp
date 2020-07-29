#ifndef	_PIXMLPARSER_H
#define	_PIXMLPARSER_H

#include "include_system.h"
#include "include_commonlib.h"
#include "include_common.h"

// ----------
#define MAKRUP_STL
#include "Markup.h"
// ----------

class CPIXmlParser {
	public:
		CPIXmlParser( );
		virtual ~CPIXmlParser();

	public:
		CMarkup markup;

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);

	public:
		bool findPath(std::string path);
};
#endif // #ifndef _PIXMLPARSER_H

