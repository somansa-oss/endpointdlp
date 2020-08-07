#ifndef	_PIDOCUMENT_H
#define	_PIDOCUMENT_H

#include "PIObject.h"

class CPIDocument : public CPIObject {
	public:
		CPIDocument();
		virtual ~CPIDocument();

	protected:
		bool stop;

	public:
//		CPIConfigMan configMan;
//		CPIPolicyMan policyMan;
//		CPIDeviceMan deviceMan;
//		CPIAgentStub piagentStub;
//		CPISQLiteStub pisqliteStub;
//		CPITickCount pitickCount;
//		CPIWallMan wallMan;

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);

	public:
		void setStop(void);
		bool getStop(void);

	public:
		static CPIDocument& getInstance(void);
};
#define Doc 		CPIDocument::getInstance()
//#define ConfigMan 	CPIDocument::getInstance().configMan
//#define PolicyMan 	CPIDocument::getInstance().policyMan
//#define DeviceMan 	CPIDocument::getInstance().deviceMan
//#define PIAgentStub CPIDocument::getInstance().piagentStub
//#define PISQLiteStub CPIDocument::getInstance().pisqliteStub
//#define PITickCount CPIDocument::getInstance().pitickCount
//#define WallMan		CPIDocument::getInstance().wallMan

#endif // #ifndef _PIDOCUMENT_H
