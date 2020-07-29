#ifndef	_PIACTIONPROCESSTTL_H
#define	_PIACTIONPROCESSTTL_H

#include "PIAction.h"
#include "PIObject.h"

class CPIActionProcessTTL : public IPIAction, public CPIObject {
	public:
		CPIActionProcessTTL();
		virtual ~CPIActionProcessTTL();

	public:
		virtual int run(const std::string& param);
		virtual int stop(void);

	public:
		static CPIActionProcessTTL& getInstance(void);
};
#endif // #ifndef _PIACTIONPROCESSTTL_H
