#ifndef	_PIACTIONRELOADPOLICY_H
#define	_PIACTIONRELOADPOLICY_H

#include "PIAction.h"
#include "PIObject.h"

class CPIActionReloadPolicy : public IPIAction, public CPIObject {
	public:
		CPIActionReloadPolicy();
		virtual ~CPIActionReloadPolicy();

	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionReloadPolicy& getInstance(void);
};
#endif // #ifndef _PIACTIONRELOADPOLICY_H
