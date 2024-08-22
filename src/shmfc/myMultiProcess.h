//简单多进程服务器框架
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "../env/env.h"

namespace ns_my_std
{
	//简单多进程，将任务分给N个子进程并等待处理结束
	class CSimpleMultiProcess
	{
	public:
		class IChildProcess
		{
		public:
			bool isThread{false};
			virtual int doChildProcess(long max_process, long i_process) = 0;
		};
	private:
		static int doChildThread(IChildProcess* pMe, long max_process, long i_process)
		{
			return pMe->doChildProcess(max_process, i_process);
		}
	public:
		static int SimpleMultiProcess(IChildProcess * pChild,long max_process, char const* title)
		{
			pChild->isThread = false;
			thelog << "开始执行" << title << " 进程 " << max_process << endi;
			time_t t1 = time(NULL);
			long i_p;
			for (i_p = 0; i_p < max_process; ++i_p)
			{
				pid_t pid = fork();
				if (pid < 0)
				{
					thelog << "fork失败" << ende;
					return __LINE__;
				}
				else if (0 == pid)
				{
					exit(pChild->doChildProcess(max_process, i_p));
				}
			}
			int status;
			int ret = 0;
			stringstream msg;
			for (i_p = 0; i_p < max_process; ++i_p)
			{
				pid_t pid = waitpid(-1, &status, 0);
				if (-1 == pid)
				{
					thelog << "waitpid 出错 " << strerror(errno) << ende;
					return __LINE__;
				}
				else
				{
					if (WIFEXITED(status))
					{
						int tmpret = (0xFF00 & status) / 256;//WEXITSTATUS宏无法识别
						if (0 != tmpret)
						{
							msg << "进程 " << pid << " 出错,返回值 " << tmpret << " 状态码 " << status << endl;
							ret = tmpret;
						}
					}
					else
					{
						ret = 2;
						msg << "进程 " << pid << " 异常,状态码 " << status << endl;
					}
				}
			}
			int timespan = time(NULL) - t1;
			if (msg.str().size() != 0)thelog << msg.str() << ende;
			thelog << "执行" << title << "完成 进程 " << max_process << " 总用时 " << timespan << "/秒" << endi;

			return ret;
		}
		static int SimpleMultiThread(IChildProcess* pChild, long max_thread, char const* title)
		{
			pChild->isThread = true;
			thelog << "开始执行" << title << " 线程 " << max_thread << endi;
			time_t t1 = time(NULL);
			long i_p;
			vector<thread *> threads;
			for (i_p = 0; i_p < max_thread; ++i_p)
			{
				threads.push_back(new thread(doChildThread, pChild, max_thread, i_p));
			}
			for (auto & v:threads)
			{
				v->join();
			}
			int timespan = time(NULL) - t1;
			thelog << "执行" << title << "完成 进程 " << max_thread << " 总用时 " << timespan << "/秒" << endi;

			return 0;
		}
	};

	//压力测试，进程数逐渐加倍，批次任务数逐渐加倍，每进程执行的批次数不变
	//如果不需要批处理测试可将max_batch_process设置为1
	class CStressTesting
	{
	private://接口
		//初始化整个测试
		virtual bool InitAllTest() { return true; }
		//结束整个测试
		virtual bool FinishAllTest() { return true; }

		//初始化一个测试
		virtual bool InitOneTest(long process, long batch) { return true; }
		//结束一个测试
		virtual bool FinishOneTest(long process, long batch) { return true; }

		//初始化进程，参数：总进程数，本进程号，每批次任务数
		virtual bool InitChildProcess(long max_process, long i_process, long batch) { return true; }
		//清理进程
		virtual bool FimishChildProcess() { return true; }

		//执行，参数：总进程数，本进程号，每批次任务数，批次号，输出信息
		virtual bool doOneBatch(long max_process, long i_process, long max_batch_process, long batch, long i_batch, string & msg)
		{
			msg = "批次完成";
			return true;
		}
	private:
		struct _result
		{
			long process;
			long batch;
			long speed_process;//每秒执行的任务数
			long speed_total;//每秒执行的任务数
			long timespan;//用时
		};
		vector<_result> m_results;

		//子进程不输出信息，总进程数，本进程号，每批次任务数，要执行的批次数
		int doStressTesting_ChileProcess(bool silent, long max_process, long i_process, long batch, long max_batch_process)
		{
			if (!InitChildProcess(max_process, i_process, batch))return __LINE__;

			long i;
			//long readcount;
			string msg;
			time_t t1 = time(NULL);
			for (i = 0; i < max_batch_process; ++i)
			{
				if (!doOneBatch(max_process, i_process, max_batch_process, batch, i, msg))
				{
					thelog << "任务处理出错 " << msg << ende;
					FimishChildProcess();
					return __LINE__;
				}
				if (!silent)
				{
					if (max_batch_process == i + 1)
					{
						thelog << msg << endi;
						int timespan = time(NULL) - t1;
						thelog << (i + 1)*batch << " " << (i + 1)*batch / (0 == timespan ? 1 : timespan) << "/秒" << endi;
					}
				}
			}
			int timespan = time(NULL) - t1;
			if (!silent)thelog << "子进程完成 " << i_process << " " << i*batch << " " << i*batch / (0 == timespan ? 1 : timespan) << "/秒" << endi;

			FimishChildProcess();
			return 0;
		}
		void Report()
		{
			CHtmlDoc::CHtmlTable2 table;
			table.AddCol("进程数", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("每批次", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("每进程/s", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("总/s", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("用时", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

			for (size_t i = 0; i < m_results.size(); ++i)
			{
				table.AddLine();
				table.AddData(m_results[i].process);
				table.AddData(m_results[i].batch);
				table.AddData(m_results[i].speed_process);
				table.AddData(m_results[i].speed_total);
				table.AddData(m_results[i].timespan);
			}

			thelog << endl << table.MakeTextTable() << endi;
		}
	public:
		//参数：不输出信息，最大进程数 最大批次任务数 每个子进程执行的批次数
		int doStressTesting(bool silent, long max_process, long max_batch, long max_batch_process)
		{
			return doStressTesting2(silent, 1, max_process, 1, max_batch, max_batch_process, 120);
		}
		//参数：不输出信息，进程数 批次任务数 每个子进程执行的批次数(相当于只执行给定的参数)
		int doStressTestingOne(bool silent, long process, long batch, long max_batch_process)
		{
			return doStressTesting2(silent, process, process, batch, batch, max_batch_process, 120);
		}
		//参数：不输出信息，最小进程数，最大进程数 最小批次任务数，最大批次任务数 每个子进程执行的批次数，时间控制（如果测试时间太长则减少任务数）
		int doStressTesting2(bool silent, long base_process, long max_process, long base_batch, long max_batch, long max_batch_process,long timespan_limit)
		{
			int ret=0;
			if (!InitAllTest())
			{
				thelog << "初始化测试失败" << ende;
				return __LINE__;
			}
			m_results.clear();
			G_IS_DEBUG = false;
			long batch, process;
			for (process = base_process; process <= max_process; process *= 2)
			{
				int timespan = 0;
				long current_batch_process = max_batch_process;
				for (batch = base_batch; batch <= max_batch; batch *= 2)
				{
					if(timespan>timespan_limit)
					{
						current_batch_process = current_batch_process / (timespan / 60);
					}
					if (!InitOneTest(process, batch))
					{
						thelog << "初始化测试失败 单元 " << batch << " 进程 " << process << ende;
						break;
					}
					thelog << "开始测试......" << " 单元 " << batch << " 进程 " << process << " 每进程 " << current_batch_process << endi;
					time_t t1 = time(NULL);
					long i_p;
					for (i_p = 0; i_p < process; ++i_p)
					{
						pid_t pid = fork();
						if (pid < 0)
						{
							thelog << "fork失败" << ende;
							return __LINE__;
						}
						else if (0 == pid)
						{
							exit(doStressTesting_ChileProcess(silent, max_process, i_p, batch, current_batch_process));
						}
					}
					int status;
					for (i_p = 0; i_p < process; ++i_p)
					{
						pid_t pid = waitpid(-1, &status, 0);
						if (-1 == pid)
						{
							thelog << "waitpid 出错 " << strerror(errno) << ende;
						}
						else
						{
							if (WIFEXITED(status))
							{
								int tmpret = (0xFF00 & status) / 256;//WEXITSTATUS宏无法识别
								if (0 != tmpret)
								{
									theLog << "进程 " << pid << " 出错,返回值 " << tmpret << " 状态码 " << status << ende;
									ret = tmpret;
								}
							}
							else
							{
								ret = 2;
								thelog << "进程 " << pid << " 异常,状态码 " << status << ende;
							}
						}
					}
					timespan = time(NULL) - t1;
					_result tmp;
					tmp.process = process;
					tmp.batch = batch;
					tmp.speed_process = current_batch_process*batch / (0 == timespan ? 1 : timespan);
					tmp.speed_total = current_batch_process*batch*process / (0 == timespan ? 1 : timespan);
					tmp.timespan = timespan;
					m_results.push_back(tmp);
					thelog << "测试完成 " << " 单元 " << batch << " 进程 " << process << " 每进程 " << current_batch_process << " " << tmp.speed_process << "/秒" << " 总 " << tmp.speed_total << "/秒" << endi;
					if (!FinishOneTest(process, batch))
					{
						thelog << "结束测试失败 单元 " << batch << " 进程 " << process << ende;
						break;
					}
				}
			}

			if (!FinishAllTest())
			{
				thelog << "结束测试失败（测试结果已经获得）" << ende;
			}
			Report();
			return ret;
		}
	};
}
