#ifndef	_PIACTIONUNREGISTER_H
#define	_PIACTIONUNREGISTER_H

#include "PIAction.h"
#include "PIObject.h"

class CPIActionUnregister : public IPIAction, public CPIObject {
	public:
		CPIActionUnregister();
		virtual ~CPIActionUnregister();
	
	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionUnregister& getInstance(void);
};
#endif // #ifndef _PIACTIONUNREGISTER_H
