/************C++ Header File****************
#
#	Filename: Poller.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:39:58
#	Last Modified: 2018-08-11 17:39:58	
*******************************************/

#pragma once
#include "Socket.h"
#include <list>
#include <set>

namespace chaos
{

	class Event;
	class EventCentre;

	const int NET_TICK = 1;	// 1000 / 60;		//һ��60֡

	class Poller : public NonCopyable
	{
	public:
		friend EventCentre;

		typedef std::map<socket_t, Event*> NetEventMap;

		Poller(EventCentre* pCentre);

		virtual ~Poller();

	public:
		virtual int Init() = 0;

		virtual int Launch(int timeoutMs) = 0;

		int AddEvent(Event* pEvent);

		int DelEvent(Event* pEvent);

		int DelEvent(socket_t fd);

		Event* GetEvent(socket_t);

		EventCentre& GetCentre() const { return *m_pCentre; }

		void Clear();

		static Poller* AdapterNetDrive(EventCentre* pCentre);


	protected:
		virtual int RegistFd(socket_t, short) { return 0; }

		virtual int RegistFd(const Event*) { return 0; }

		virtual int CancelFd(socket_t) { return 0; }

		int PushActiveEvent(socket_t fd, short ev);

		const NetEventMap& GetAllEvents() const { return m_events; }

		NetEventMap& GetAllEvents() { return m_events; }

	private:
		Poller(const Poller&);

	protected:
		EventCentre* m_pCentre;
		NetEventMap m_events;
	};

}
