#ifndef	_PIEVENT_H
#define	_PIEVENT_H

#include "PIObject.h"

class CPIEvent : public CPIParam {
	public:
		CPIEvent();
		virtual ~CPIEvent();

	protected:
		nsPISupervisor::EM_EVENTID eventId;

	protected:
		virtual void clear(void);

	public:
		void setId(const nsPISupervisor::EM_EVENTID eventId);
		nsPISupervisor::EM_EVENTID getId(void) const;
};
#endif // #ifndef _PIEVENT_H
