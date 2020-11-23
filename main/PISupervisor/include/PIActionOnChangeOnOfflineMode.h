#ifndef	_PIACTIONONCHANGEONOFFLINEMODE_H
#define	_PIACTIONONCHANGEONOFFLINEMODE_H

#ifdef LINUX
#include <string>
#endif

#include "PIAction.h"
#include "PIObject.h"

class CPIActionOnChangeOnOfflineMode : public IPIAction, public CPIObject {
	public:
		CPIActionOnChangeOnOfflineMode();
		virtual ~CPIActionOnChangeOnOfflineMode();

	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionOnChangeOnOfflineMode& getInstance(void);
		static void* fnOnChangeOnOfflineMode(void* pzArg);
};
#endif // #ifndef _PIACTIONONCHANGEONOFFLINEMODE_H
