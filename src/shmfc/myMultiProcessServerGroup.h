//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once
//
//多进程服务器框架组，可同时运行多份以实现2-多-2模式
//

#include "myMultiProcess.h"
#include "myMultiProcessServer.h"

namespace ns_my_std
{
	//如需增加服务组数量，只需修改enum N即可
	template<typename T_JOBINFO, typename T_SERVERINFO, typename T_TASKINFO, long MAX_JOB, long MAX_PROCESS, long MAX_PROCESS_TASK_BLOCK, long MAX_BLOCK_TASK>
	class CMultiProcessServerGroup : public CSimpleMultiProcess::IChildProcess
	{
	private:
		enum { N = 1 };
		typedef CMultiProcessServer<T_JOBINFO, T_SERVERINFO, T_TASKINFO, MAX_JOB, MAX_PROCESS, MAX_PROCESS_TASK_BLOCK, MAX_BLOCK_TASK> T_SERVER;
		typedef vector<T_SERVER > T_DATAS;
		T_DATAS m_datas;
	public:
		CMultiProcessServerGroup(IDistributeTask<T_JOBINFO,T_TASKINFO > * d,IProcessTask<T_JOBINFO,T_TASKINFO > * p,IRollupTask<T_JOBINFO,T_TASKINFO > * r,char * shmbuf)
		{
			m_datas.reserve(N);
			for (size_t i = 0; i < m_datas.capacity(); ++i)
			{
				m_datas.push_back(T_SERVER(d, p, r, shmbuf + T_SERVER::calcBufSise() * i));
			}
		}
		static long calcBufSise() { return T_SERVER::calcBufSise() * N; }//获得所需的内存大小
		string & report(string & ret, bool process = true, bool process_detail = true, bool taskinfo = false)const
		{
			ret = "";
			for (typename T_DATAS::const_iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				string str;
				ret += it->report(str, process, process_detail, taskinfo);
			}
			return ret;
		}
		string & toString(string & ret)const
		{
			ret = "";
			for (typename T_DATAS::const_iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				string str;
				ret += it->toString(str);
			}
			return ret;
		}
		void SetCommondPause()
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetCommondPause();
			}
			thelog<<"暂停命令已设置"<<endi;
		}
		void SetCommondExit()
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetCommondExit();
			}
			thelog<<"退出命令已设置"<<endi;
		}
		void SetCommondContiue()
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetCommondContiue();
			}
			thelog<<"继续命令已设置"<<endi;
		}
		void SetAutoHighLow(bool autoHighLow)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetAutoHighLow(autoHighLow);
			}
		}
		void SetExitIfNoJob(bool exitIfNoJob)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetExitIfNoJob(exitIfNoJob);
			}
		}
		void SetExitIfProcessCoredump(bool exitIfProcessCoredump)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetExitIfProcessCoredump(exitIfProcessCoredump);
			}
		}
		void SetNotAutoSleep(bool notAutoSleep)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetNotAutoSleep(notAutoSleep);
			}
		}
		void SetNotAutoSleep_distribute(bool notAutoSleep)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetNotAutoSleep_distribute(notAutoSleep);
			}
		}
		void SetNotAutoSleep_rollup(bool notAutoSleep)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetNotAutoSleep_rollup(notAutoSleep);
			}
		}
		void SetMaxProcess(long max_process)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SetMaxProcess(max_process);
			}
		}
		void CheckSumEnable(bool chenksumEnable)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->CheckSumEnable(chenksumEnable);
			}
		}
		void SyncEnable(bool syncEnable)
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->SyncEnable(syncEnable);
			}
		}
		void clear()
		{
			for (typename T_DATAS::iterator it = m_datas.begin(); it != m_datas.end(); ++it)
			{
				it->clear();
			}
		}
		int doChildProcess(long max_process, long i_process)override
		{
			return	m_datas[i_process].run(i_process);
		}
		int run()
		{
			return CSimpleMultiProcess::SimpleMultiProcess(this, m_datas.size(), "多进程服务组");
		}
		int view()
		{
			thelog<<"多进程服务器控制，显示模式推荐使用2，只显示第一个进程的状态\n"
				"多进程服务器控制 ctrl-c to break"<<endi;
			int view_level=0;//查看模式
			string str;

			signal(SIGINT,sig_default);
			while(true)
			{
				if(check_signal(SIGINT))break;
				string cmd=UIInput("p=暂停 s=停止 c=继续 a=睡眠 na=不睡眠 checksum-enable checksum-disable sync-enable sync-disable"
					"\n d-a=分发睡眠 d-na=分发不睡眠 r-a=汇总睡眠 r-na=汇总不睡眠"
					"\n hl=高低水 nhl=不高低水 sp=最大进程数"
					"\n 显示模式设置：0=显示全部 1=不显示任务细节 2=不显示处理进程 3=显示用户任务数据"
					"\n other=根据模式显示","");
				if("p"==cmd)SetCommondPause();
				else if("s"==cmd)SetCommondExit();
				else if("c"==cmd)SetCommondContiue();
				else if("a"==cmd)SetNotAutoSleep(false);
				else if("na"==cmd)SetNotAutoSleep(true);
				else if ("checksum-enable" == cmd)CheckSumEnable(true);
				else if ("checksum-disable" == cmd)CheckSumEnable(false);
				else if ("sync-enable" == cmd)SyncEnable(true);
				else if ("sync-disable" == cmd)SyncEnable(false);
				else if("d-a"==cmd)SetNotAutoSleep_distribute(false);
				else if("d-na"==cmd)SetNotAutoSleep_distribute(true);
				else if("r-a"==cmd)SetNotAutoSleep_rollup(false);
				else if("r-na"==cmd)SetNotAutoSleep_rollup(true);
				else if("hl"==cmd)SetAutoHighLow(true);
				else if("nhl"==cmd)SetAutoHighLow(false);
				else if("sp"==cmd)SetMaxProcess(atol(UIInput("intput 1-MAX:","").c_str()));
				else if("0"==cmd)view_level=0;
				else if("1"==cmd)view_level=1;
				else if("2"==cmd)view_level=2;
				else if("3"==cmd)view_level=3;
				else
				{
					if(1==view_level)thelog<<"显示模式："<<view_level<<endl<<report(str,true,false)<<endi;
					else if(2==view_level)thelog<<"显示模式："<<view_level<<endl<<report(str,false,false)<<endi;
					else if(3==view_level)thelog<<"显示模式："<<view_level<<endl<<report(str,true,true,true)<<endi;
					else thelog<<"显示模式："<<view_level<<endl<<toString(str)<<endi;
				}
			}
			return 0;
		}
		static long timeval_percent(timeval const * current, timeval const * last, long span_usec)
		{
			return T_SERVER::timeval_percent(current, last, span_usec);
		}
		static long timespan_usec(timeval const * current, timeval const * last)
		{
			return T_SERVER::timespan_usec(current, last);
		}
		static long clock_percent(clock_t current, clock_t last, long span_usec)
		{
			return T_SERVER::clock_percent(current, last, span_usec);
		}
		int speed(size_t iServer, bool AutomaticTransmission)
		{
			if (iServer >= m_datas.size())
			{
				thelog << "无效的服务索引 " << iServer << endi;
				return __LINE__;
			}

			return m_datas[iServer].speed(AutomaticTransmission);
		}
	};

}
