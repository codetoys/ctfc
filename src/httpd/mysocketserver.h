//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#ifdef _WINDOWS
#else
#include <sys/wait.h>
#endif
#include "../function/myprocess.h"
#include "myThread.h"

namespace ns_my_std
{
#define CSocketServer_MAX_CHILD (1024)//最大子进程数
#define G_PROCESS_INFO CCurrentProcess::GetProcessInfo()
#define G_DOUBLE_LOG(msg) G_PROCESS_INFO->SetInfo(msg);thelog<<(msg)

	class CHttpProcess;
	class main_process_info;

	class CCurrentProcess
	{
	public:
		static main_process_info * GetProcessInfo(main_process_info * _p = NULL)
		{
			STATIC_G thread_local main_process_info * p = NULL;
			if (NULL != _p)
			{
				p = _p;
			}
			return p;
		}
		static bool MultiProcessMode(bool * _mode = NULL)
		{
			STATIC_G bool mode = true;
			if (NULL != _mode)
			{
				mode = *_mode;
			}
			return mode;
		}
	};

	enum PROCESS_STATE { PROCESS_STATE_NULL = 0, PROCESS_STATE_INIT, PROCESS_STATE_RUN, PROCESS_STATE_PAUSE, PROCESS_STATE_EXITED };
	//主进程状态
	struct main_process_info
	{
		pid_t pid;//多进程模式为pid，多线程模式为线程序号，-1代表无效
		sstring<32> name;
		time_t start_time;//进程开始时间
		time_t end_time;//进程结束时间
		time_t last_time;//进程最后一次活动的时间
		sstring<256 > info;
		PROCESS_STATE state;

		main_process_info() :pid(-1), start_time(0), end_time(0), last_time(0), state(PROCESS_STATE_NULL) {}
		void SetInfo(char const * msg)
		{
			last_time = time(NULL);
			info = msg;
		}
		void SetInfo2(char const * msg, long n)
		{
			last_time = time(NULL);
			char buf[256];
			sprintf(buf, " %ld", n);
			info = (string)msg + buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{
			table.AddCol((CCurrentProcess::MultiProcessMode() ? "pid" : "tid"), CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("name");
			table.AddCol("start_time", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("end_time", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("last_time", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("state", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("info");
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddData(pid);
			table.AddData(name.c_str());
			table.AddData(start_time ? CMyTools::TimeToString_log(start_time) : string(""));
			table.AddData(end_time ? CMyTools::TimeToString_log(end_time) : string(""));
			table.AddData(last_time ? CMyTools::TimeToString_log(last_time) : string(""));
			if (PROCESS_STATE_NULL == state)table.AddData("NULL");
			else if (PROCESS_STATE_INIT == state)table.AddData("INIT");
			else if (PROCESS_STATE_RUN == state)table.AddData("RUN");
			else if (PROCESS_STATE_PAUSE == state)table.AddData("PAUSE");
			else if (PROCESS_STATE_EXITED == state)table.AddData("EXITED");
			else
			{
				char buf[256];
				sprintf(buf, "unknown : %d", state);
				table.AddData(buf);
			}
			table.AddData(info.c_str());
			return true;
		}
	};

	struct SocketServerData
	{
	public:
		struct child_info :public main_process_info
		{
			long seq;//此位置的重用次数
			long connect_count;//连接计数
			bool isMgrPort;//是否是管理服务

			child_info() :seq(0), connect_count(0), isMgrPort(false) {}
			void SetChildProcessInfo(char const * _info)
			{
				info = _info;
				last_time = time(NULL);
			}
			bool isChildAlive()const
			{
				return (-1 != pid);
			}
			bool PostExitCmd()const
			{
				if (CCurrentProcess::MultiProcessMode())
				{
					kill(pid, SIGTERM);
					return true;
				}
				else
				{
					return true;
				}
			}
			//用于表格输出
			static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
			{
				main_process_info::AddTableColumns(table);
				table.AddCol("seq", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("connect", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("isMgr", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				string str;
				table.AddCol(ShowPS_head(str), CHtmlDoc::CHtmlDoc_DATACLASS_PRE);
				return true;
			}
			bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
			{
				if (!main_process_info::AddTableData(table))return false;
				table.AddData(seq, true);
				table.AddData(connect_count, true);
				table.AddData(isMgrPort);
				string str;
				table.AddData(pid > 0 ? ShowPS_pid(pid, str) : "");
				return true;
			}
		};
	public:
		bool isDebug;
		sstring<32> server_name;
		time_t server_start_time;
		int port;
		bool cmd_bShutDown;//命令：关机
		bool cmd_bPause;//命令：暂停，管理服务保持，普通服务停止，初始为true，初始化完成后设置为false
		bool state_bPause;//状态：普通服务已暂停，该值设置表明HTTPD已经实施了暂停
		long max_child;
		CAtomicRWMutex::mySEM m_sem;//互斥对象

		main_process_info admin_process;//后台监控进程
		main_process_info httpd_main_process;//AS服务主进程
		child_info childs[CSocketServer_MAX_CHILD];

		SocketServerData() :server_start_time(0), port(0), cmd_bShutDown(false), cmd_bPause(false)
			, state_bPause(false)
		{
			//将全局调试开关指向共享内存
			isDebug = G_IS_DEBUG;
			g_pActiveAppEnv->pIsDebug = &isDebug;
		}

		//判断是否用户服务处于暂停中，必须state_bPause且没有用户端口的子进程存在
		bool isPaused()const
		{
			if (!state_bPause)return false;
			long i;
			bool ret = true;
			for (i = 0; i < CSocketServer_MAX_CHILD; ++i)
			{
				if (-1 != childs[i].pid && !childs[i].isMgrPort)
				{
					childs[i].PostExitCmd();
					ret = false;

				}
			}
			if (!ret)
			{
				//激活一下端口
				CMySocket tmps;
				tmps.Connect("127.0.0.1", port);
				tmps.Close();
				tmps.Connect("127.0.0.1", port + 1);
				tmps.Close();
			}
			return ret;
		}

		void ClearChild(pid_t pid)
		{
			long i;
			for (i = 0; i < CSocketServer_MAX_CHILD; ++i)
			{
				if (pid == childs[i].pid)childs[i].pid = -1;
			}
		}
		long _AddChild(bool isSecond)
		{
			child_info * p = NULL;
			long i;
			for (i = 0; i < CSocketServer_MAX_CHILD; ++i)
			{
				if (isSecond)
				{
					if (-1 == childs[i].pid)
					{
						p = &childs[i];
						break;
					}
				}
				else
				{
					if (-1 == childs[i].pid && 0 == childs[i].start_time)
					{
						p = &childs[i];
						break;
					}
				}
			}
			if (NULL != p)
			{
				p->start_time = time(NULL);
				p->end_time = 0;
				p->last_time = 0;
				p->state = PROCESS_STATE_NULL;
				return i;
			}
			else return -1;
		}
		long AddChild()
		{
			//long i = _AddChild(false);
			//if (i >= 0)return i;
			//else 
			{
				return _AddChild(true);
			}
		}
		long GetChildCount()const
		{
			long ret = 0;
			long i;
			for (i = 0; i < CSocketServer_MAX_CHILD; ++i)
			{
				if (-1 != childs[i].pid)
				{
					++ret;
				}
			}
			return ret;
		}
	};

	//服务处理接口
	class ISocketServerProcess
	{
	public:
		//服务开始时调用（主进程），对于多线程模式，不调用OnStartChildSocketProcess
		virtual bool OnStartServer() = 0;
		//开始新的子进程时调用
		virtual bool OnStartChildSocketProcess() = 0;
		//处理一个已经建立的连接
		virtual bool SocketProcess(bool isMgrPort, CMySocket & s, long * pRet, long i_child) = 0;
		//结束新的子进程时调用
		virtual bool OnStopChildSocketProcess() = 0;
		//服务结束时调用（主进程）
		virtual bool OnStopServer() = 0;
	};

	//SOCKET服务器
	class CSocketServer
	{
	private:
		CMySocket m_s_user;//主socket
		CMySocket m_s_admin;//主socket，管理端口
		SocketServerData * m_pShmSocketServerData;
		map<pid_t, CMySocket > m_child_socket;//记录子进程的socket，子进程结束时关闭不用的socket，创建子进程后立即关闭可能出问题

		bool m_demon = true;
	private:
		void clearStoppedChild()
		{
			int stat;
			pid_t tmp;
			//获取所有已经结束的子进程的状态
			while (true)
			{
				tmp = waitpid(-1, &stat, WNOHANG);
				if (tmp > 0)
				{
					if (0 != kill(tmp, 0))
					{
						m_pShmSocketServerData->ClearChild(tmp);
						DEBUG_LOG << "子进程 " << tmp << " 已退出" << ENDI;
						map<pid_t, CMySocket >::iterator it = m_child_socket.find(tmp);
						if (it != m_child_socket.end())
						{
							it->second.Close();
							m_child_socket.erase(it);
						}
					}
				}
				else break;
			}
		}
		//出错或需要退出返回false，seconds为0立即返回，seconds<0永不超时,no_lock不互斥
		bool _Accept(bool isMgrPort, CMySocket & listen_s, CMySocket & s, long seconds, bool no_lock, bool & isTimeout)
		{
			CMyShmMutex m;

			if (!no_lock)
			{
				if (m.Attach(&m_pShmSocketServerData->m_sem))
				{
					if (0 == seconds)
					{
						if (!m.TryWLock())
						{
							//LOG << "TryWLock 失败，立即返回" << ENDI;
							isTimeout = true;
							return true;
						}
					}
					else
					{
						m.WLock();
					}
				}
			}
			bool ret = __Accept(isMgrPort, listen_s, s, seconds, isTimeout);
			if (m.getThreadData()->isWLocked())m.WUnLock();
			return ret;
		}
		bool __Accept(bool isMgrPort, CMySocket & listen_s, CMySocket & s, long seconds, bool & isTimeout)
		{
			time_t t1 = time(NULL);
			isTimeout = false;
			long step_seconds = (0 == seconds ? 0 : 1);//步长
			do
			{
				if (m_pShmSocketServerData->cmd_bShutDown)
				{
					LOG << "收到退出命令......" << ENDI;
					return false;
				}
				else
				{
					G_PROCESS_INFO->state = PROCESS_STATE_RUN;
					G_PROCESS_INFO->SetInfo2("IsSocketReadReady", listen_s.GetMyPort());
				}

				bool isReady = false;
				if (!listen_s.IsSocketReadReady2(step_seconds, isReady))
				{
					LOG << "socket error" << ENDE;
					return false;
				}
				if (isReady)
				{
					DEBUG_LOG << seconds << " socket ready " << listen_s.debuginfo() << ENDI;
					s = listen_s.Accept();
					if (!s.IsConnected())
					{
						LOG << "accept error : " << strerror(errno) << ENDE;
						return false;
					}
					else
					{
						return true;
					}
				}
				//DEBUG_LOG << seconds << " socket not ready " << listen_s.debuginfo() << ENDI;
			} while (seconds < 0 || time(NULL) - t1 < seconds);

			isTimeout = true;
			return true;
		}
		void _ChildProcess(bool isMgrPort, CMySocket & s, ISocketServerProcess * pProcess, long i_child)
		{
			SocketServerData::child_info * pChildInfo = &m_pShmSocketServerData->childs[i_child];
			DEBUG_LOG << "子处理启动" << ENDI;
			while (true)
			{
				DEBUG_LOG << endl << s.debuginfo() << ENDI;
				++pChildInfo->connect_count;
				pChildInfo->SetChildProcessInfo("SocketProcess...");
				long ret;
				pProcess->SocketProcess(isMgrPort, s, &ret, i_child);
				pChildInfo->SetChildProcessInfo("SocketProcess end");
				if (m_pShmSocketServerData->cmd_bShutDown || (!isMgrPort && m_pShmSocketServerData->cmd_bPause))
				{
					LOG << "收到退出命令，子处理退出......" << ENDI;
					return;
				}

				if (CCurrentProcess::MultiProcessMode())
				{
					//与父进程争夺accept
					bool isTimeout;
					pChildInfo->SetChildProcessInfo("Accept...");
					if (pChildInfo->isMgrPort)return;//管理服务只能由主进程accept
					if (!_Accept(isMgrPort, m_s_user, s, 300, false, isTimeout))
					{//暂停时虽然得到一个连接但SocketProcess会检测到暂停而立即返回
						LOG << "超时或收到退出命令" << ENDI;
						return;
					}
				}
				else
				{//多线程模式不能争夺
					break;
				}
			}
		}
		void ChildProcess(bool isMgrPort, CMySocket & s, ISocketServerProcess * pProcess, long i_child)
		{
			SocketServerData::child_info * pChildInfo = &m_pShmSocketServerData->childs[i_child];

			if (CCurrentProcess::MultiProcessMode())
			{//多进程模式
				char buf[256];
				sprintf(buf, "%ld", (long)getpid());
				theLog.SetSource(buf);
				pChildInfo->name = buf;

				pChildInfo->SetChildProcessInfo("OnStartChildSocketProcess...");
				if (!pProcess->OnStartChildSocketProcess())
				{
					LOG << "OnStartChildSocketProcess失败" << ENDE;
					pProcess->OnStopChildSocketProcess();
					pChildInfo->SetChildProcessInfo("OnStartChildSocketProcess失败");
					pChildInfo->end_time = time(NULL);
					G_PROCESS_INFO->state = PROCESS_STATE_EXITED;
					exit(0);
				}

				_ChildProcess(isMgrPort, s, pProcess, i_child);

				pChildInfo->SetChildProcessInfo("OnStopChildSocketProcess...");
				if (!pProcess->OnStopChildSocketProcess())
				{
					LOG << "OnStopChildSocketProcess失败" << ENDE;
					LOG << "子进程结束" << ENDE;
					pChildInfo->SetChildProcessInfo("OnStopChildSocketProcess失败,子进程结束");
					pChildInfo->end_time = time(NULL);
					G_PROCESS_INFO->state = PROCESS_STATE_EXITED;
					exit(0);
				}
				pChildInfo->SetChildProcessInfo("子进程结束");
				pChildInfo->end_time = time(NULL);
				DEBUG_LOG << "子进程结束" << ENDI;
				G_PROCESS_INFO->state = PROCESS_STATE_EXITED;
				exit(0);
			}
			else
			{//多线程模式

				_ChildProcess(isMgrPort, s, pProcess, i_child);

				pChildInfo->SetChildProcessInfo("子线程结束");
				pChildInfo->end_time = time(NULL);
				pChildInfo->pid = -1;
				LOG << "子线程结束" << ENDI;
			}
		}
		class ThreadJob : public IThreadJob
		{
		private:
			CSocketServer * pMe;
			bool isMgrPort;
			CMySocket s;
			ISocketServerProcess * pProcess;
			long i_child;
		public:
			ThreadJob(CSocketServer * p, bool _isMgrPort, CMySocket & _s, ISocketServerProcess * _pProcess, long _i_child)
				: pMe(p), isMgrPort(_isMgrPort), s(_s), pProcess(_pProcess), i_child(_i_child)
			{
			}
			virtual ~ThreadJob()override
			{
				thelog << "~ThreadJob被执行" << endi;
			}
			virtual bool thread_AfterCreateThread(CThread * pThread)
			{
				CCurrentProcess::GetProcessInfo(&pMe->m_pShmSocketServerData->childs[i_child]);
				return true;
			}
			virtual bool thread_Run(CThread * pThread)//线程函数
			{
				pMe->ChildProcess(isMgrPort, s, pProcess, i_child);
				return true;
			}
		};
		//出错或需要退出返回false，static_bM用于控制两个端口轮换，初始设置为false
		bool SvrAccept(ISocketServerProcess * pProcess, bool * static_bM)
		{
			G_PROCESS_INFO->SetInfo("Accept");
			clearStoppedChild();

			long child_count = m_pShmSocketServerData->GetChildCount();
			if (child_count >= m_pShmSocketServerData->max_child || child_count >= CSocketServer_MAX_CHILD)
			{
				//LOG << "子进程数 " << m_pShmSocketServerData->GetChildCount() << " 达到 " << m_pShmSocketServerData->max_child << " 主进程睡眠" << ENDI;
				G_PROCESS_INFO->SetInfo("too many childs");
				SleepSeconds(1);
				return true;
			}

			CMySocket s;//子socket
			bool isMgrPort = false;
			bool & bM = *static_bM;
			bM = !bM;

			//接受连接，不阻塞
			bool isTimeout;
			//LOG << "m_s_admin" << ENDI;
			if (bM)
			{
				if (!_Accept(true, m_s_admin, s, 1, true, isTimeout))
				{
					LOG << "收到退出命令或出错" << ENDI;
					return true;
				}
				if (isTimeout)
				{
					return true;
				}
				else
				{
					isMgrPort = true;
				}
			}
			else
			{
				if (child_count >= m_pShmSocketServerData->max_child - 1)
				{
					//必须保留一个给管理服务
					return true;
				}
				//LOG << "m_s_user" << ENDI;
				if (!_Accept(false, m_s_user, s, 0, false, isTimeout))
				{
					LOG << "收到退出命令或出错" << ENDI;
					return true;
				}
				else
				{
					if (isTimeout)
					{
						return true;
					}
					else
					{
						if (m_pShmSocketServerData->state_bPause)
						{
							LOG << "暂停" << ENDI;
							string str503 = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
							s.Send(str503);
							s.Close();
							return true;
						}
						isMgrPort = false;
					}
				}
			}
			G_PROCESS_INFO->SetInfo("get a new connect");

			pid_t pid;
			long i_child = m_pShmSocketServerData->AddChild();
			if (i_child < 0)
			{
				LOG << "不可思议的错误" << ENDE;
				s.Close();
				return true;
			}

			m_pShmSocketServerData->childs[i_child].isMgrPort = isMgrPort;

			if (CCurrentProcess::MultiProcessMode())
			{//多进程模式
				if ((pid = fork()) < 0)
				{
					LOG << "fork error" << ENDE;
					return false;
				}
				else if (pid != 0)
				{//父进程
					DEBUG_LOG << "fork ok pid=" << pid << ENDI;
					m_pShmSocketServerData->childs[i_child].pid = pid;
					++m_pShmSocketServerData->childs[i_child].seq;
					m_child_socket[pid] = s;//不能立即关闭子连接，要等到子进程结束

					//等待子进程启动
					time_t t1 = time(NULL);
					while (PROCESS_STATE_NULL == m_pShmSocketServerData->childs[i_child].state)
					{
						DEBUG_LOG << "子进程仍未启动" << ENDI;
						SleepSeconds(1);
						if (time(NULL) - t1 > 30)
						{
							LOG << "子进程超时未启动，可能是个问题" << ENDE;
							break;
						}
					}
					return true;
				}
				else
				{//子进程，内部会exit
					//改变进程信息指针
					CCurrentProcess::GetProcessInfo(&m_pShmSocketServerData->childs[i_child]);

					G_PROCESS_INFO->state = PROCESS_STATE_RUN;
					ChildProcess(isMgrPort, s, pProcess, i_child);
				}
			}
			else
			{//多线程模式
				SocketServerData::child_info * pChildInfo = &m_pShmSocketServerData->childs[i_child];
				STATIC_G long thread_count = 0;
				++thread_count;
				pChildInfo->pid = thread_count;
				++pChildInfo->seq;
				ThreadJob * threadjob = new ThreadJob(this, isMgrPort, s, pProcess, i_child);
				CThread * thread = new CThread;

				pChildInfo->SetChildProcessInfo("StartChildThread...");
				thread->id = thread_count;
				thread->logname = "thread";
				thread->CreateAndRunOnce(threadjob, NULL);
				//子线程并发处理，主线程继续
			}
			return true;
		}
		void _WaitAllChildStop()
		{
			while (true)
			{
				long i;

				clearStoppedChild();
				for (i = 0; i < CSocketServer_MAX_CHILD; ++i)
				{
					if (m_pShmSocketServerData->childs[i].isChildAlive())
					{
						LOG << "子进程 " << m_pShmSocketServerData->childs[i].pid << "仍未结束" << ENDI;
						m_pShmSocketServerData->childs[i].PostExitCmd();
					}
				}
				if (0 == m_pShmSocketServerData->GetChildCount())
				{
					LOG << "所有子进程都已退出" << ENDI;
					return;
				}
				else
				{
					SleepSeconds(1);
				}
			}
		}
		bool _Start_Listen(unsigned short portnum, ISocketServerProcess * pProcess)
		{
			G_PROCESS_INFO->SetInfo("Listen");
			if (m_s_admin.Listen(portnum + 1))
			{
				LOG << "管理服务启动 端口号 " << portnum + 1 << ENDI;

				if (m_s_user.Listen(portnum))
				{
					LOG << "WWW服务启动 端口号 " << portnum << ENDI;

					G_PROCESS_INFO->state = PROCESS_STATE_RUN;

					if (m_demon)start_demon();//进入精灵进程模式

					bool static_bM = false;
					while (SvrAccept(pProcess, &static_bM))
					{
						if (m_pShmSocketServerData->cmd_bPause)
						{
							m_pShmSocketServerData->state_bPause = true;
							//LOG << "服务暂停" << ENDI;
						}
						else
						{
							m_pShmSocketServerData->state_bPause = false;
						}

						if (m_pShmSocketServerData->cmd_bShutDown)
						{
							thelog << "用户停止服务,正在等待子进程结束......" << endi;
							_WaitAllChildStop();
							G_PROCESS_INFO->state = PROCESS_STATE_EXITED;
							thelog << "用户停止服务,服务结束" << endi;
							return true;
						}

						//检查admin进程是否存在
						if (!CMyProcess::isProcessLive(m_pShmSocketServerData->admin_process.pid))
						{
							m_pShmSocketServerData->cmd_bShutDown = true;
							G_DOUBLE_LOG("admin process not exist 服务结束") << ende;
							return true;
						}
					}
					thelog << "accept 出错 服务结束" << ende;
					return false;
				}
				else
				{
					LOG << "WWW服务启动失败 端口号 " << portnum << ENDE;
					LOG << "提示：如果有通过控制台启动的程序在运行,即使kill服务进程端口也仍然会在占用中" << ENDE;
					return false;
				}
			}
			else
			{
				LOG << "管理服务启动失败 端口号 " << portnum + 1 << ENDE;
				LOG << "提示：如果有通过控制台启动的程序在运行,即使kill服务进程端口也仍然会在占用中" << ENDE;
				return false;
			}
		}
		bool _Start_StartStop(unsigned short portnum, ISocketServerProcess * pProcess)
		{
			G_PROCESS_INFO->SetInfo("OnStartServer");
			if (!pProcess->OnStartServer())
			{
				LOG << "服务启动失败" << ENDE;
				return false;
			}
			G_PROCESS_INFO->SetInfo("_Start_Listen");
			if (!_Start_Listen(portnum, pProcess))
			{
				LOG << "服务启动失败" << ENDE;
				return false;
			}
			G_PROCESS_INFO->SetInfo("OnStopServer");
			if (!pProcess->OnStopServer())
			{
				LOG << "服务退出处理失败" << ENDE;
				return false;
			}
			return true;
		}
		bool _Start_Mutex(unsigned short portnum, ISocketServerProcess * pProcess)
		{
			CMyShmMutex mutex;

			if (!mutex.Create(&m_pShmSocketServerData->m_sem))
			{
				LOG << "创建信号量失败，可能发生不能快速退出的问题" << ende;
			}

			bool ret = _Start_StartStop(portnum, pProcess);
			mutex.Destory();
			return ret;
		}
	public:
		//启动服务循环，除非服务结束，否则不会返回。maxchild小于等于0或大于最大值都等同于最大值。理论上pSocketServerData应该来自共享内存
		bool Start(SocketServerData * pSocketServerData, char const * server_name, unsigned short portnum, ISocketServerProcess * pProcess, long maxchild, CHttpProcess * pHttpProcess,bool demon)
		{
			m_demon = demon;

			bool mode = false;//是否使用多进程模式
			CCurrentProcess::MultiProcessMode(&mode);

			m_pShmSocketServerData = pSocketServerData;
			if (NULL == m_pShmSocketServerData)
			{
				LOG << "服务控制块未创建" << ENDE;
				return false;
			}

			theLog.SetSource("**ADMIN**");
			CCurrentProcess::GetProcessInfo(&m_pShmSocketServerData->admin_process);
			G_PROCESS_INFO->name = "ADMIN";
			G_PROCESS_INFO->pid = getpid();
			G_PROCESS_INFO->state = PROCESS_STATE_INIT;
			G_PROCESS_INFO->start_time = time(NULL);
			G_PROCESS_INFO->SetInfo("启动");

			m_pShmSocketServerData->server_name = server_name;
			m_pShmSocketServerData->server_start_time = time(NULL);
			m_pShmSocketServerData->port = portnum;
			m_pShmSocketServerData->max_child = ((maxchild <= CSocketServer_MAX_CHILD && maxchild > 0) ? maxchild : CSocketServer_MAX_CHILD);

			theLog.SetSource("**HTTPD**");
			CCurrentProcess::GetProcessInfo(&m_pShmSocketServerData->httpd_main_process);
			G_PROCESS_INFO->name = "HTTPD";
			G_PROCESS_INFO->pid = getpid();
			G_PROCESS_INFO->state = PROCESS_STATE_INIT;
			G_PROCESS_INFO->start_time = time(NULL);
			G_PROCESS_INFO->SetInfo("启动");

			_Start_Mutex(portnum, pProcess);

			G_PROCESS_INFO->SetInfo("exited");
			G_PROCESS_INFO->state = PROCESS_STATE_EXITED;
			thelog << "服务结束" << endi;
		
			return true;
		}
	};

}
