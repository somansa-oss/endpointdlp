#ifndef	_PIDEVICEMANTEST_H 
#define	_PIDEVICEMANTEST_H 

#include "PIObject.h"

class CPIDeviceManTest : public CPIObject {
	public:
		CPIDeviceManTest();
		virtual ~CPIDeviceManTest();

	public:
		void copyPreventPlusDeviceLog(void);
		void copyPreventPlus(void);
		void printPreventPlus(void);
};
#endif // #ifndef	_PIDEVICEMANTEST_H 
