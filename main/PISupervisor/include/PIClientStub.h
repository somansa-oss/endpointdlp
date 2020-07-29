#ifndef	_PICLIENTSTUB_H
#define	_PICLIENTSTUB_H

#include"PICommunicator.h"

class CPIClientStub : public CPICommunicator {
	public:
		CPIClientStub();
		virtual ~CPIClientStub();

	public:
		typedef std::list<CPIClientStub*> LIST;

	public:
		bool bSession;

	protected:
		virtual bool initialize(void);
		virtual bool finalize(void);
};
#endif
