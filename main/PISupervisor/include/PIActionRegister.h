#ifndef	_PIACTIONREGISTER_H
#define	_PIACTIONREGISTER_H

#include "PIAction.h"
#include "PIObject.h"

class CPIActionRegister : public IPIAction, public CPIObject {
	public:
		CPIActionRegister();
		virtual ~CPIActionRegister();
	
	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionRegister& getInstance(void);
};
#endif // #ifndef _PIACTIONREGISTER_H
