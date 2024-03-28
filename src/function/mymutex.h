//mymutex.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#ifdef _WINDOWS
#else
#include "mymutex1.h"
#endif
#include "mymutex2.h"
#include "mymutex3.h"

namespace ns_my_std
{
#ifdef _WINDOWS
#else
	using CMyRWMutex = CSemRWMutex;
#endif
	using CMyShmMutex = CAtomicRWMutex;

#ifdef _WINDOWS
#else
	//线程同步对象
	class CPThreadMutex
	{
	private:
		pthread_mutex_t m_mutex;//互斥对象
		pthread_cond_t m_cond;//条件变量
	public:
		void init()
		{
			pthread_mutex_init(&m_mutex, NULL);
			pthread_cond_init(&m_cond, NULL);
		}
		void destory()
		{
			pthread_cond_destroy(&m_cond);
			pthread_mutex_destroy(&m_mutex);
		}
		int lock()const { return pthread_mutex_lock((pthread_mutex_t*)&m_mutex); }
		int unlock()const { return pthread_mutex_unlock((pthread_mutex_t*)&m_mutex); }
		int wait()const { return pthread_cond_wait((pthread_cond_t*)&m_cond, (pthread_mutex_t*)&m_mutex); }
		int reltimedwait()const
		{
			timespec to;
			to.tv_sec = time(NULL) + 1;
			to.tv_nsec = 0;
			int ret;

			ret = pthread_cond_timedwait((pthread_cond_t*)&m_cond, (pthread_mutex_t*)&m_mutex, &to);
			if (0 == ret)return true;
			else if (ETIMEDOUT == ret)return false;
			else throw ret;
			return false;
		}
		int signal()const { return pthread_cond_signal((pthread_cond_t*)&m_cond); }
	};
#endif
}
