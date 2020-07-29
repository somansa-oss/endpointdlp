#ifndef	_EVENTHANDLER_H
#define	_EVENTHANDLER_H

#include "PIObject.h"
#include "PIEvent.h"
#include <vector>
#include <map>

class CPIEventHandler : public CPIObject {
	public:
		CPIEventHandler();
		virtual ~CPIEventHandler();

	public:
		typedef void (*EVENT_HANDLER)(const CPIEvent* pEvent);
		typedef std::map<unsigned int, EVENT_HANDLER> MAP_HANDLER;
		typedef std::vector<CPIEvent> VECTOR_EVENT;

	protected:
		pthread_mutex_t mutexEvent;
		pthread_cond_t condEvent;
		MAP_HANDLER mapHandler;
		VECTOR_EVENT vectorEvent;
		bool isContinue;
		enum { STATE_WAIT, STATE_SIG } stateEvent;

	protected:
		virtual bool initialize(void);
		virtual bool finalize(void);
		void setEventHandler(nsPISupervisor::EM_EVENTID eventId, EVENT_HANDLER fnEventHandler);
		void stopAllAction(void);

	public:
		void run(void);
		void stop(void);
		void addEvent(const CPIEvent& event);
		void addEvent(const nsPISupervisor::EM_EVENTID eventId, const std::string& param);
		bool onSyncRegister(const std::string& param);	

	public:
		static CPIEventHandler & getInstance(void);
		static void onInitPISupervisor(const CPIEvent* pEvent);
		static void onUninitPISupervisor(const CPIEvent* pEvent);
		static void onUnregsiter(const CPIEvent* pEvent);
		static void onChangedOnOfflineMode(const CPIEvent* pEvent);
		static void onChangedDLPPolicy(const CPIEvent* pEvent);
		static void onChangedDevice(const CPIEvent* pEvent);
		static void onExecute(const CPIEvent* pEvent);
		static void onTTL(const CPIEvent* pEvent);
		static void onRegsiter(const CPIEvent* pEvent);
};

#define ON_PISEC_EVENT(x,y) setEventHandler(x,y);
#define RUN_PISEC_EVENT(x,y) x::getInstance().run(y)
#define STOP_PISEC_EVENT(x) x::getInstance().stop()
#define ADD_PISEC_EVENT(x,y) CPIEventHandler::getInstance().addEvent(x,y);
#define EventHandler CPIEventHandler::getInstance()
#endif // #ifndef _EVENTHANDLER_H
