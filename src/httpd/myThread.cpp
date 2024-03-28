//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//
#include "myThread.h"

namespace ns_my_std
{
	bool IThreadJob::thread_Run(CThread * pThread)
	{
		thelog << &errno << endi;
		return true;
	}

	void * CThread_static_thread_function(void * pMe)
	{
		return ((CThread *)pMe)->thread_function();
	}
	void * CThread_static_thread_function_run_once(void * pMe)
	{
		return ((CThread *)pMe)->thread_function_run_once();
	}

	//////////////////////////////////////////////////////////////////////////
	//class CThread
	bool CThread::_thread_function_init()
	{
		return true;
	}
	void * CThread::thread_function()
	{
		if (!_thread_function_init())return NULL;

		//线程初始化
		if (!m_pJob->thread_AfterCreateThread(this))
		{
			m_state = STATE_EXIT;
			return NULL;
		}

		//线程循环
		bool bExit = false;
		while (!bExit)
		{
			bool bRun = false;
			Lock();
			//thelog<<"thread "<<pThis->id<<" cmd "<<pThis->m_cmd<<endi;
			switch (m_cmd)
			{
			case CMD_START:
				m_state = STATE_RUN;
				m_cmd = CMD_NONE;
				bRun = true;
				break;
			case CMD_NONE:
				m_state = STATE_IDLE;
				m_sync.wait();
				break;
			case CMD_EXIT:
				m_state = STATE_EXIT;
				bExit = true;
				break;
			default:
				m_state = STATE_EXIT;
				m_errormsg += "未知的命令\n";
				bExit = true;
				break;
			}
			Unlock();
			if (bRun)
			{
				++(m_runcount);
				m_pJob->thread_Run(this);
				//发送任务完成通知
				if (NULL != m_pnotifysync)
				{
					m_pnotifysync->lock();
					m_pnotifysync->signal();
					m_pnotifysync->unlock();
				}
			}
		}
		return NULL;
	}
	void * CThread::thread_function_run_once()
	{
		m_isValid=true;
		if (!_thread_function_init())return NULL;

		//线程初始化
		if (!m_pJob->thread_AfterCreateThread(this))
		{
			m_state = STATE_EXIT;
			return NULL;
		}

		m_pJob->thread_Run(this);
		//发送任务完成通知
		if (NULL != m_pnotifysync)
		{
			m_pnotifysync->lock();
			m_pnotifysync->signal();
			m_pnotifysync->unlock();
		}

		//清理自身
		if (0 != pthread_detach(pthread_self()))
		{
			thelog << "分离线程出错 " << errno << " " << strerror(errno) << ende;
		}
		m_isValid = false;
		delete m_pJob;
		delete this;
		return NULL;
	}

	string CThread::GetPThreadErrorMsg(int err)const
	{
		char buf[1024];
		switch (err)
		{
		case 0:return "OK";
		case EAGAIN:return "EAGAIN";
		case EINVAL:return "EINVAL";
		case EPERM:return "EPERM";
		default:
			sprintf(buf, "%d %s", err, strerror(err));
			return buf;
		}
	}
	string CThread::Report()const
	{
		char buf[256];
		sprintf(buf, "id[%ld] valid[%d] cmd[%d] state[%d] runcount[%ld]", id, (m_isValid ? 1 : 0), m_cmd, m_state, m_runcount);
		return buf + m_errormsg;
	}
	bool CThread::Lock() { return 0 == m_sync.lock(); }
	bool CThread::Unlock() { return 0 == m_sync.unlock(); }
	bool CThread::isValid()
	{
		return m_isValid;
	}
	bool CThread::isIdle()
	{
		bool ret;
		Lock();
		ret = (STATE_IDLE == m_state && CMD_NONE == m_cmd);
		Unlock();
		return ret;
	}
	bool CThread::isExit()
	{
		bool ret;
		Lock();
		ret = (STATE_EXIT == m_state);
		Unlock();
		return ret;
	}
	//创建但不运行
	bool CThread::Create(IThreadJob * job, CPThreadMutex * pNotify)
	{
		if (m_isValid)
		{
			m_errormsg += "线程已经存在，不能创建\n";
			return false;
		}
		m_errormsg = "";
		m_runcount = 0;
		m_cmd = CMD_NONE;
		m_state = STATE_NONE;
		m_pnotifysync = pNotify;
		if (!job->BeforeCreateThread(this))
		{
			m_errormsg += "BeforeCreateThread错误\n";
			return false;
		}
		m_pJob = job;
		int ret;
		if (0 != (ret = pthread_create(&m_thread, NULL, CThread_static_thread_function, this)))
		{
			char buf[256];
			sprintf(buf, "%d", ret);
			m_errormsg += "创建线程失败 返回码 ";
			m_errormsg += buf;
			m_errormsg += " ";
			m_errormsg += GetPThreadErrorMsg(ret);
			m_errormsg += "\n";
			return false;
		}
		m_isValid = true;
		return true;
	}
	//创建然后运行一次就退出
	bool CThread::CreateAndRunOnce(IThreadJob * job, CPThreadMutex * pNotify)
	{
		if (m_isValid)
		{
			m_errormsg += "线程已经存在，不能创建\n";
			return false;
		}
		m_errormsg = "";
		m_runcount = 0;
		m_cmd = CMD_NONE;
		m_state = STATE_NONE;
		m_pnotifysync = pNotify;
		if (!job->BeforeCreateThread(this))
		{
			m_errormsg += "BeforeCreateThread错误\n";
			return false;
		}
		m_pJob = job;
		int ret;
		if (0 != (ret = pthread_create(&m_thread, NULL, CThread_static_thread_function_run_once, this)))
		{
			char buf[256];
			sprintf(buf, "%d", ret);
			m_errormsg += "创建线程失败 返回码 ";
			m_errormsg += buf;
			m_errormsg += " ";
			m_errormsg += GetPThreadErrorMsg(ret);
			m_errormsg += "\n";
			return false;
		}
		return true;
	}
	//运行
	bool CThread::Run()
	{
		bool ret = true;
		m_errormsg = "";
		Lock();
		do
		{
			//thelog<<"thread "<<id<<" state "<<m_state<<endi;
			if (CMD_NONE != m_cmd)
			{
				ret = false;
				thelog << Report() << ende;
				break;
			}
			m_cmd = CMD_START;
			if (0 != m_sync.signal())
			{
				ret = false;
				thelog << Report() << ende;
				break;
			}
		} while (false);
		Unlock();
		return ret;
	}
	//停止
	bool CThread::PostExitCmd()
	{
		bool ret = true;
		Lock();
		do
		{
			//thelog<<"thread "<<id<<" state "<<m_state<<endi;
			m_cmd = CMD_EXIT;
			if (0 != m_sync.signal())
			{
				ret = false;
				break;
			}
		} while (false);
		Unlock();
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	//class CThreadPool
	CThread * CThreadPool::__GetThread(long i, FIND_TYPE type)
	{
		switch (type)
		{
		case FIND_TYPE_IDLE:
			if (!m_datas[i]->isValid())
			{
				//thelog<<"创建新线程"<<endi;
				if (!m_datas[i]->Create(m_pJob, &m_NotifySync))
				{
					thelog << "创建新线程失败：" << m_datas[i]->GetMsg() << ende;
					return NULL;
				}
				m_lastidle = i;
				return m_datas[i];
			}
			else if (m_datas[i]->isIdle())
			{
				//thelog<<"找到空闲线程"<<endi;
				m_lastidle = i;
				return m_datas[i];
			}
			else
			{
				return NULL;
			}
			break;
		default:
			break;
		}
		return NULL;
	}
	CThread * CThreadPool::_GetThread(FIND_TYPE type)
	{
		size_t i;
		CThread * ret = NULL;
		for (i = m_lastidle + 1; i < m_datas.size() && NULL == ret; ++i)
		{
			ret = __GetThread(i, type);
		}
		for (i = 0; (long)i <= m_lastidle && NULL == ret; ++i)
		{
			ret = __GetThread(i, type);
		}
		return ret;
	}
	CThread * CThreadPool::_GetThreadWait(FIND_TYPE type)
	{
		CThread * ret = _GetThread(type);
		if (NULL == ret)
		{
			m_NotifySync.lock();
			ret = _GetThread(type);
			if (NULL == ret)
			{
				m_NotifySync.wait();
				ret = _GetThread(type);
			}
			m_NotifySync.unlock();
		}
		return ret;
	}
	string CThreadPool::Report()const
	{
		string ret;
		char buf[256];
		long runcount = 0;
		size_t i = 0;
		sprintf(buf, "thread pool size[%ld]\n", (long)m_datas.size());
		ret += buf;
		for (i = 0; i < m_datas.size(); ++i)
		{
			ret += m_datas[i]->Report() + "\n";
			runcount += m_datas[i]->GetRunCount();
		}
		sprintf(buf, "total run count %ld", runcount);
		ret += buf;
		return ret;
	}
	bool CThreadPool::Init(long n, IThreadJob * job, char const * logname)
	{
		long i;
		m_datas.reserve(n);
		m_datas.resize(n);
		for (i = 0; i < n; ++i)
		{
			m_datas[i] = new CThread;
			if (NULL == m_datas[i])
			{
				m_errormsg += "内存不足\n";
				return false;
			}
			m_datas[i]->id = i + 1;
			m_datas[i]->logname = logname;
		}
		m_pJob = job;
		m_lastidle = -1;
		return true;
	}
	long CThreadPool::Stop()
	{
		long ret = 0;
		size_t i;
		for (i = 0; i < m_datas.size(); ++i)
		{
			if (m_datas[i]->isValid() && !m_datas[i]->isExit())
			{
				++ret;
				if (!m_datas[i]->PostExitCmd())
				{
					m_errormsg += m_datas[i]->GetMsg();
					return -1;
				}
			}
		}
		return ret;
	}

	int CThreadPool::CThreadPool_test(int argc, char ** argv)
	{
		IThreadJob job;
		CThreadPool pool;
		if (!pool.Init(2, &job, "test"))return __LINE__;
		thelog << pool.Report() << endi;
		for (long i = 0; i < 10; ++i)
		{
			SleepSeconds(1);
			CThread * p = pool.GetIdleThread();
			if (NULL != p)
			{
				p->Run();
			}
		}
		thelog << pool.Report() << endi;
		pool.Stop();
		thelog << pool.Report() << endi;
		return 0;
	}

}
