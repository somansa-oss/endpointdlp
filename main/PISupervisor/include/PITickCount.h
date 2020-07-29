#include "PITickCountParent.h"

class CPITickCount : public CPITickCountParent {
	public:
		CPITickCount(void);
		virtual ~CPITickCount(void);

	public:
		typedef enum _EVENT_ID {
			PISUPERVISOR_ROOT
		} EVENT_ID;

	public:
		virtual void defineEvent();
};

