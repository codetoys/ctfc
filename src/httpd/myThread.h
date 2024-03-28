//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//
#pragma once

#include "../env/myUtil.h" 
using namespace ns_my_std;

namespace ns_my_std
{
	//定义线程存储，不是每种编译器都支持的
#ifdef _LINUX_CC
#define MyAS_THREAD //定义这个以支持线程
#define My_THREAD __thread
#else
#define My_THREAD
#endif

	class CThread;

	//线程任务接口，由于线程并发运行，具体实现通常不应该包含具体数据，数据应该通过CThread的arg成员操作
	//应当在BeforeCreateThread创建线程相关数据并将总数据指针放在arg，该函数在线程(而不是CThread)被创建时调用
	//应当在OnDestoryThread销毁线程相关数据，该函数在CThread析构时才被调用
	//执行顺序：CThread构造-〉等待直到需要新的线程-〉BeforeCreateThread-〉线程被创建-〉thread_AfterCreateThread
	//                  -〉thread_Run->等待-〉thread_Run-〉等待-〉。。。-〉线程退出-〉线程数据保留直到最后-〉~CThread
	//IThreadJob对象完全由外部管理
	class IThreadJob
	{
	public:
		virtual ~IThreadJob(){}//虚析构函数
		virtual bool BeforeCreateThread(CThread * pThread) { return true; }//线程创建前调用的函数
		virtual bool thread_AfterCreateThread(CThread * pThread) { return true; }//线程创建后调用的函数
		virtual bool thread_Run(CThread * pThread);//线程函数
	};

	//线程启动函数，仅供CThread内部调用
	extern "C" void * CThread_static_thread_function(void * pMe);
	extern "C" void * CThread_static_thread_function_run_once(void * pMe);

	//线程
	class CThread
	{
		friend void * CThread_static_thread_function(void * pMe);
		friend void * CThread_static_thread_function_run_once(void * pMe);
	private:
		enum THREAD_CMD { CMD_NONE = 0, CMD_START, CMD_EXIT };
		enum THREAD_STATE { STATE_NONE = 0, STATE_IDLE, STATE_RUN, STATE_EXIT };
	private:
		bool m_isValid;//本线程是否已创建
		string m_errormsg;
		long m_runcount;//运行次数

		THREAD_CMD m_cmd;//主控线程传来的控制命令
		THREAD_STATE m_state;//线程状态

		pthread_t m_thread;//线程结构
		IThreadJob * m_pJob;//线程任务接口
		CPThreadMutex m_sync;//同步对象
		CPThreadMutex * m_pnotifysync;//任务完成通知对象

		bool _thread_function_init();
		void * thread_function();
		void * thread_function_run_once();
		string GetPThreadErrorMsg(int err)const;
	public:
		long id;//公共标识
		void * arg;//线程参数
		string logname;//日志名（前缀）
		long GetRunCount()const { return m_runcount; }
		string const & GetMsg()const { return m_errormsg; }
		string Report()const;
		CThread() :m_isValid(false), m_runcount(0), m_cmd(CMD_NONE), m_state(STATE_NONE), m_pJob(NULL), m_pnotifysync(NULL), id(0), arg(NULL)
		{
			m_sync.init();
		}
		~CThread()
		{
			m_sync.destory();
			if (isValid())
			{
				if (0 != pthread_join(m_thread, NULL))
				{
					thelog << "joining thread " << id << " error " << errno << " " << strerror(errno) << ende;
				}
				else
				{
					thelog << "joining thread " << id << " ok" << endi;
				}
			}
		}
		bool Lock();
		bool Unlock();
		bool isValid();
		bool isIdle();
		bool isExit();
		//创建但不运行
		bool Create(IThreadJob * job, CPThreadMutex * pNotify);
		//创建并运行然后退出，任务对象和自身都会被delete
		bool CreateAndRunOnce(IThreadJob * job, CPThreadMutex * pNotify);
		//运行
		bool Run();
		//停止
		bool PostExitCmd();
	};
	//线程池
	class CThreadPool
	{
	private:
		enum FIND_TYPE { FIND_TYPE_IDLE };
		vector<CThread *> m_datas;
		string m_errormsg;
		IThreadJob * m_pJob;
		CPThreadMutex m_NotifySync;
		long m_lastidle;//上一次找到的空闲ID

		CThread * __GetThread(long i, FIND_TYPE type);
		CThread * _GetThread(FIND_TYPE type);
		CThread * _GetThreadWait(FIND_TYPE type);
	public:
		CThreadPool() :m_lastidle(-1)
		{
			m_NotifySync.init();
		}
		~CThreadPool()
		{
			size_t i;
			for (i = 0; i < m_datas.size(); ++i)
			{
				if (NULL != m_datas[i])delete m_datas[i];
				m_datas[i] = NULL;
			}
			m_NotifySync.destory();
		}
		string const & GetMsg()const { return m_errormsg; }
		string Report()const;
		bool Init(long n, IThreadJob * job, char const * logname);
		long Stop();
		CThread * GetIdleThread() { return _GetThread(FIND_TYPE_IDLE); }
		CThread * GetIdleThreadWait() { return _GetThreadWait(FIND_TYPE_IDLE); }

		static int CThreadPool_test(int argc, char ** argv);
	};

}
