#ifndef	_PIOBJECT_H
#define	_PIOBJECT_H

#include "include_system.h"
#include "include_commonlib.h"
#include "include_common.h"
#include "PIUtility.h"

////////////////////////////////////////////////////////////////////////////////
//class IPIObject
class IPIObject {
	public:
		IPIObject(){};
		virtual ~IPIObject(){};

	public:
		virtual bool initialize(void) = 0;
		virtual bool finalize(void) = 0;
		virtual void clear(void) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//class CPIObject
class CPIObject : public IPIObject {
	public:
		CPIObject();
		virtual ~CPIObject();

	public:
		CPIUtility util;
		bool initialized;

	protected:
		bool isInitialized(void);

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);
};

////////////////////////////////////////////////////////////////////////////////
//class CPIParam 
class CPIParam : public CPIObject {
	public:
		CPIParam();
		virtual ~CPIParam();

	public:
		typedef std::vector<std::string> VECTOR_PARAM;

	public:	
		std::string param;
		CPIParam::VECTOR_PARAM paramList;
	
	public:
		static const char DELIMETER;
	
	public:
		virtual void clear(void);
		
	public:
		void addParam(std::string param);
		void setParam(std::string param);
		std::string getParam(void) const;
		bool parse(void);
		std::string at(const int index);
};
#endif // #ifndef _PIOBJECT_H

