#pragma once


#include "../../common/stdafx.h"
#include "Mutex.h"

class Condition :public NonCopyable
{
public:
	Condition(Mutex& mutex):
		m_mutex(mutex)
	{
#ifdef _WIN32
		m_cond = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
		pthread_cond_init(&m_cond, NULL);
#endif // _WIN32
		
	}


	~Condition()
	{
#ifdef _WIN32
		CloseHandle(m_cond);
#else
		pthread_cond_destroy(&m_cond);
#endif // _WIN32

	}


	int CondWait(int timeOut = -1)
	{
#ifdef _WIN32
		if (0 > timeOut)
			timeOut = INFINITE;

		//����,��UnLock,��Lock,����ģ��pthread_cond_wait
		m_mutex.UnLock();
		int ret = WaitForSingleObject(m_cond, timeOut * 1000);
		if (0 == ret)
		{
			//���ڴ���Eventʱ���ֶ�ģʽ,��Ҫ�ֶ������ź�, 
			//���������̵߳�WaitForSingleObject��һֱ�յ��ź�
			ResetEvent(m_cond);
		}

		m_mutex.Lock();

		return ret;
#else
		if (0 <= timeOut)
		{
			struct timespec timeSpec;
			timeSpec.tv_sec = timeOut;
			timeSpec.tv_nsec = 0;
			
			return pthread_cond_timedwait(&m_cond, m_mutex.GetMutex(), &timeSpec);
		}
		return pthread_cond_wait(&m_cond, m_mutex.GetMutex());
#endif // _WIN32

	}


	int CondSignal()
	{
#ifdef _WIN32
		SetEvent(m_cond);
		return ResetEvent(m_cond);
		//return SetEvent(m_cond);
#else
		return pthread_cond_signal(&m_cond);
#endif // _WIN32
	}


	int CondBroadCast()
	{
#ifdef _WIN32
		return PulseEvent(m_cond);
#else
		return pthread_cond_broadcast(&m_cond);
#endif // _WIN32

	}

private:
	cond_t m_cond;
	Mutex& m_mutex;
};