#ifndef	_PIDOCUMENT_H
#define	_PIDOCUMENT_H

#include "PIObject.h"
#include "PIConfigMan.h"
#include "PIPolicyMan.h"
#include "PIDeviceMan.h"
#include "PIAgentStub.h"
#include "PISQLiteStub.h"
#include "PITickCount.h"
#include "PIWallMan.h"

class CPIDocument : public CPIObject {
	public:
		CPIDocument();
		virtual ~CPIDocument();

	protected:
		bool stop;

	public:
		CPIConfigMan configMan;
		CPIPolicyMan policyMan;
		CPIDeviceMan deviceMan;
		CPIAgentStub piagentStub;
		CPISQLiteStub pisqliteStub;
		CPITickCount pitickCount;

#ifndef LINUX		
		CPIWallMan wallMan;
#endif

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
#define ConfigMan 	CPIDocument::getInstance().configMan
#define PolicyMan 	CPIDocument::getInstance().policyMan
#define DeviceMan 	CPIDocument::getInstance().deviceMan
#define PIAgentStub CPIDocument::getInstance().piagentStub
#define PISQLiteStub CPIDocument::getInstance().pisqliteStub
#define PITickCount CPIDocument::getInstance().pitickCount

#ifndef LINUX
#define WallMan		CPIDocument::getInstance().wallMan
#endif

#endif // #ifndef _PIDOCUMENT_H
