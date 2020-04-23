#include "EventMaster.h"


#if 0
EventMaster::EventMaster() :m_base(NULL)
//, m_readCb(NULL)
//, m_writeCb(NULL)
//, m_errCb(NULL)
{
}


EventMaster::~EventMaster()
{
}


int EventMaster::Init()
{
	return Init("0.0.0.0", 6666);
}


int EventMaster::Init(const char* ip, int port)
{
	m_base = CreateSocketIO(FD_SETSIZE, SI_EPOLL);
	if (!m_base)
		return -1;

	if (0 != m_base->InitIO(ip, port, FD_SETSIZE))
		return -1;

	return 0;
}


int EventMaster::Loop()
{
	//while (true)
	//{
	//	m_base->WaitEvent();

	//	if (0 < m_base->GetEventSize())
	//		printf("wait event!event size:%d\n", m_base->GetEventSize());


	//	while (!m_base->EventEmpty())
	//	{
	//		m_base->HandleEvent(m_base->GetIOEvent());
	//		printf("handle event! event size:%d\n", m_base->GetEventSize());
	//	}
	//}


	while (true)
	{
		m_base->WaitEvent();
		ProcessEvenet();
	}

	return 0;
}


int EventMaster::Stop()
{
	return 0;
}


void EventMaster::RegisterEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userArg)
{
	m_base->AddEvent(fd, readCb, writeCb, listenCb, errCb, userArg);
}


void EventMaster::ProcessEvenet()
{
	/*
	while(!m_base->EventEmpty())
		m_base->HandleEvent(m_base->GetIOEvent());
	*/
	m_base->ProcessEvent();
}


int EventMaster::AddEvent(Event& ev)
{
	if (ev.ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
		m_ioEvents.insert(std::make_pair(ev.Ev.evSocket.fd, ev));

	else if (ev.ev & EV_TIMEOUT)
		m_timer.AddTimer(&ev, ev.Ev.evTimer.timeOut, ev.evCb);

	return 0;
}


void EventMaster::EraseEvent(Event& ev)
{
	if (ev.ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
	{
		auto end_it = m_ioEvents.upper_bound(ev.Ev.evSocket.fd);
		for (auto it = m_ioEvents.lower_bound(ev.Ev.evSocket.fd); it != end_it; ++it)
		{
			if (it->second.ev & ev.ev)
			{
				it = m_ioEvents.erase(it);
				break;
			}
		}
	}

	else if (ev.ev & EV_TIMEOUT)
	{
		m_timer.DelTimer(ev.Ev.evTimer.timerID);
	}
}

#endif 