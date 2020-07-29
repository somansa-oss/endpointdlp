#ifndef	_PIACTION_H
#define	_PIACTION_H

#include <string>

class IPIAction {
	public:
		IPIAction();
		virtual ~IPIAction();
	public:
		virtual int run(const std::string& param) = 0;
		virtual int stop(void) = 0;
};
#endif // #ifndef _PIACTION_H
