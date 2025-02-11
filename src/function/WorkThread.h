﻿//工作线程
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "myUtil.h" 
using namespace ns_my_std;
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ns_my_std
{
	class IWorkThread
	{
	public:
		virtual ~IWorkThread() {}
		//工作线程的任务
		virtual void worker_job() = 0;
		//定时器线程的任务，会在激活工作线程之前执行（并不代表工作线程此时一定不在工作中）
		virtual void timer_job() = 0;
	};
	class CWorkerThread
	{
	public:
	private:
		string m_name;
		thread* m_work_thread = nullptr;
		mutex m_work_mutex;//互斥锁
		condition_variable m_work_cv;//条件变量
		bool m_manual_active = false;//此标志位确保进程已经在处理时不会错过激活信号（如果线程不在等待中则信号会被忽略，导致处理被推迟到下次定时激活）
		unsigned long m_interval_ms;//间隔，毫秒
		IWorkThread* m_pIWorkThread;//实际的工作任务

		thread* m_timer_thread = nullptr;//定时循环

		void _WorkThread()
		{
			DEBUG_LOG << "WorkThread" << endi;
			while (true)
			{
				m_manual_active = false;
				//处理
				m_pIWorkThread->worker_job();

				if (!m_manual_active)
				{
					//等待数据
					unique_lock<mutex> lck(m_work_mutex);
					m_work_cv.wait(lck);
					DEBUG_LOG << m_name << " WorkThread被激活" << endi;
				}
			}
		}
		void _TimerThread()
		{
			DEBUG_LOG << "TimerThread" << endi;
			while (true)
			{
				m_pIWorkThread->timer_job();
				ActiveWorkerThread();
				//定时激活工作线程，避免工作线程错过信号而无限期等待
				SleepUSeconds(m_interval_ms * 1000);
			}
		}
		static void WorkThread(CWorkerThread* pMe)
		{
			pMe->_WorkThread();
		}
		static void TimerThread(CWorkerThread* pMe)
		{
			pMe->_TimerThread();
		}
	public:
		bool StartWorkerThread(IWorkThread* pIWorkThread, char const* name, unsigned long ms)
		{
			m_name = name;
			m_interval_ms = ms;
			m_pIWorkThread = pIWorkThread;

			//启动子线程
			m_work_thread = new thread(WorkThread, this);
			m_timer_thread = new thread(TimerThread, this);

			return true;
		}
		bool StopWorkerThread()
		{
			//需要首先停止子线程，不过一般设备都是直接断电重启的，所以无所谓

			return true;
		}
		bool ActiveWorkerThread()
		{
			m_work_cv.notify_one();
			m_manual_active = true;
			return true;
		}
	};
}
