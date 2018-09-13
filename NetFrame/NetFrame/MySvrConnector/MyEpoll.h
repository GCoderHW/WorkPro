/************C++ Header File****************
#
#	Filename: MyEpoll.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:31
#	Last Modified: 2018-08-10 10:12:31
*******************************************/

#ifndef _WIN32

#pragma once

#include "MySocketIO.h"
#include <sys/epoll.h>


class MyEpoll : public MySocketIO
{
public:

	static MyEpoll& Instance();

	virtual ~MyEpoll() {}

	virtual int InitIO(const char* ip, int port, uint32 max_fd);

	virtual void WaitEvent();

	virtual void HandleEvent(const IOEvent& ioEvent);

	int AddSocket(const MySocket& s);

protected:
	MyEpoll();

	virtual void delScoket(const uint32 fd);

private:
	int m_max_fd;
	int m_epfd;
	epoll_event* m_events;
};

#endif // !_WIN32
