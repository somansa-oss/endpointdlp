#ifndef	_PIDOCUMENT_H
#define	_PIDOCUMENT_H

#include "PIObject.h"

class CPIDocument : public CPIObject {
	public:
		CPIDocument();
		virtual ~CPIDocument();

	protected:
		bool stop;

	public:
		virtual bool initialize(void);
		virtual bool finalize(void);
		virtual void clear(void);

	public:
		void setStop(void);
		bool getStop(void);

	public:
		static CPIDocument& getInstance(void);
};
#define Doc 		CPIDocument::getInstance()

#endif // #ifndef _PIDOCUMENT_H
