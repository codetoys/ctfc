//mymutex1.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <errno.h>
#include "function.h"
#ifndef USE_BOL
#include <sys/ipc.h>
#include <sys/sem.h>
#include <atomic>
#include <thread>

union semun
{
	int val;
	struct semid_ds* buf;
	unsigned short* array;
};
#else
#include "proc/bol_ipc.h"
#include "pub_def.h"
#define semget bol_ipc_semget
#define semctl bol_ipc_semctl
#define semop bol_ipc_semop
#define semctl bol_ipc_semctl
#endif

//CSemRWMutex
//读写互斥

namespace ns_my_std
{
	//对象实例不可复制不可移动，内部记录进程操作状态和线程操作状态
	class CSemRWMutex
	{
	private:
		int sem_id{ -1 };//信号量ID
		mutable bool isIngore{ false };//是否忽略，不锁定
		mutable bool isSafe{ false };//是否带有安全检查，确保操作序列正确

		//进程操作计数，防止操作顺序错误
		mutable atomic<int> count_WLock{ 0 };
		mutable atomic<int> count_RLock{ 0 };

		//线程操作记录，防止线程操作错误并可用于中途让出再重新锁定
		struct thread_data
		{
			bool _isLocked{ false };//是否已经锁定，若已经锁定则不重复锁定
			bool _isWLock{ false };//是否是写锁定，当isLocked时有效

			bool isLocked()const
			{
				return _isLocked;
			}
			bool isWLocked()const
			{
				return _isLocked && _isWLock;
			}
			bool isRLocked()const
			{
				return _isLocked && !_isWLock;
			}
			void thread_data_WLock()
			{
				_isLocked = true;
				_isWLock = true;
			}
			void thread_data_RLock()
			{
				_isLocked = true;
				_isWLock = false;
			}
			void thread_data_UnLock()
			{
				_isLocked = false;
				_isWLock = false;
			}
		};
	public:
		thread_data* getThreadData()const
		{
			thread_local map<CSemRWMutex const*, thread_data > d;//通过对象地址区分不同的对象
			return &d[this];
		}

		//禁止移动和复制（不能用于vector，因为vector会移动对象）
		CSemRWMutex() = default;
		CSemRWMutex(CSemRWMutex const&) = delete;
		CSemRWMutex& operator =(CSemRWMutex const&) = delete;
		CSemRWMutex(CSemRWMutex const&&) = delete;
		CSemRWMutex& operator =(CSemRWMutex const&&) = delete;

		~CSemRWMutex()
		{
			if (0 != count_WLock || 0 != count_RLock)
			{
				if (0 != count_WLock) cout << "警告：析构而未解锁：" << sem_id << " is w locked " << count_WLock << endl;
				if (0 != count_RLock) cout << "警告：析构而未解锁：" << sem_id << " is r locked " << count_RLock << endl;
			}
			sem_id = -1;
		}
	private:
		mutable int m_errid{ 0 };//最近的错误号
		mutable CMyStringStream m_errmsg;//最近的错误信息

		string errno2str()const
		{
			string s;
			switch (errno)
			{
			case EACCES: s = "EACCES"; break;
			case EINVAL: s = "EINVAL"; break;
			case EPERM: s = "EPERM"; break;
			case EOVERFLOW: s = "EOVERFLOW"; break;
			case ERANGE: s = "ERANGE"; break;
			case E2BIG: s = "E2BIG"; break;
			case EAGAIN: s = "EAGAIN"; break;
			case EFAULT: s = "EFAULT"; break;
			case EFBIG: s = "EFBIG"; break;
			case EIDRM: s = "EIDRM"; break;
			case EINTR: s = "EINTR"; break;
			case ENOSPC: s = "ENOSPC"; break;
			default: s = "semctl error";
			}
			return s;
		}
	public:
		string Report()const
		{
			char buf[1024];
			string ret;
			if (sem_id != -1)
			{
				sprintf(buf, "sem_id = %10d , W %d R %d (%s), %s %s", sem_id, count_WLock.load(), count_RLock.load()
					, (getThreadData()->isLocked() ? (getThreadData()->isWLocked() ? "W" : "R") : "-")
					, (isSafe ? "safe" : ""), (isIngore ? " , ingored" : ""));
				ret += buf;

				long w, w_count, r_count, w_wait;
				if (GetCount2(w, w_count, r_count, w_wait))
				{
					sprintf(buf, " 写锁 %ld 写计数 %ld 读计数 %ld 写等待 %ld", w, w_count, r_count, w_wait);
					ret += buf;
				}
				if (0 != m_errid)
				{
					sprintf(buf, " 错误：%d %s", m_errid, m_errmsg.str().c_str());
				}
				else
				{
					sprintf(buf, " 无错误");
				}
				ret += buf;
			}
			else
			{
				ret += "空信号量";
			}
			return ret;
		}
	private:
		bool _GetCTime(time_t& ctime)const
		{
			semid_ds ds;
			union semun sem;
			int ret;

			memset(&ds, 0, sizeof(semid_ds));
			sem.buf = &ds;
			if ((ret = semctl(sem_id, 0, IPC_STAT, sem)) < 0)
			{
				*(int*)&m_errid = __LINE__;
				(*(stringstream*)&m_errmsg).str("");
				*(stringstream*)&m_errmsg << "获得sem " << sem_id << " 状态出错(" << ret << ")：" << errno2str();
				return false;
			}
			ctime = ds.sem_ctime;
			return true;
		}

		void after_WLock()const
		{
			++count_WLock;
			getThreadData()->thread_data_WLock();
		}
		void after_RLock()const
		{
			++count_RLock;
			getThreadData()->thread_data_RLock();
		}
		void after_WUnLock()const
		{
			--count_WLock;
			getThreadData()->thread_data_UnLock();
		}
		void after_RUnLock()const
		{
			--count_RLock;
			getThreadData()->thread_data_UnLock();
		}
	public:
		//忽略锁定调用，不执行锁定
		void ingore()const { isIngore = true; }
		//恢复功能
		void enable()const { isIngore = false; }
		//启用安全检查
		void safe(bool _safe)const { isSafe = _safe; }
		int GetID()const
		{
			return sem_id;
		}
		int GetID(time_t& ctime)const
		{
			if (!_GetCTime(ctime))return -1;
			return sem_id;
		}
		bool Attach(int id, time_t const& ctime)
		{
			if (isSafe)
			{
				if (0 != count_WLock || 0 != count_RLock)
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 已经锁定，需要先解锁";
					return false;
				}
			}
			sem_id = id;

			//检查
			time_t tmp_ctime;
			if (!_GetCTime(tmp_ctime))
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				sem_id = -1;
				return false;
			}
			else
			{
				if (tmp_ctime != ctime)
				{
					m_errid = __LINE__;
					m_errmsg.str("");
					m_errmsg << "ctime不匹配";
					sem_id = -1;
					return false;
				}
			}

			return -1 != sem_id;
		}
		bool Detach()
		{
			if (isSafe)
			{
				if (0 != count_WLock || 0 != count_RLock)
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 已经锁定，需要先解锁";
					return false;
				}
			}
			sem_id = -1;
			return true;
		}

		//创建新信号量
		bool Create()
		{
			if (isSafe)
			{
				if (0 != count_WLock || 0 != count_RLock)
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 已经锁定，需要先解锁";
					return false;
				}
			}
#ifdef _LINUXOS
			sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | IPC_EXCL | 0666);
#else
			sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | IPC_EXCL | SEM_R | (SEM_R >> 3) | (SEM_R >> 6) | SEM_A | (SEM_A >> 3) | (SEM_A >> 6));
#endif
			if (-1 == sem_id)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				return false;
			}
			union semun sem;
			sem.val = 1;
			if (semctl(sem_id, 0, SETVAL, sem) < 0)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				return false;
			}
			sem.val = 0;
			if (semctl(sem_id, 1, SETVAL, sem) < 0)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				return false;
			}
			sem.val = 0;
			if (semctl(sem_id, 2, SETVAL, sem) < 0)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				return false;
			}
			sem.val = 0;
			if (semctl(sem_id, 3, SETVAL, sem) < 0)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				return false;
			}
			return true;
		}
		//删除信号量
		bool Destory()
		{
			if (isSafe)
			{
				if (0 != count_WLock || 0 != count_RLock)
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 已经锁定，需要先解锁";
					return false;
				}
			}
			union semun sem;
			sem.val = 0;
			if (semctl(sem_id, 0, IPC_RMID, sem) < 0)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << errno2str();
				return false;
			}
			else
			{
				sem_id = -1;
				return true;
			}
		}
		//锁定，等待
		bool RLock()const { return _RLock(false); }
		bool TryRLock()const { return _RLock(true); }
		bool _RLock(bool no_wait)const
		{
			if (isSafe)
			{
				if (getThreadData()->isLocked())
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 已经锁定，不能重复锁定";
					return false;
				}
			}
			if (isIngore)
			{
				after_RLock();
				return true;//忽略锁定
			}
			constexpr short int flag = SEM_UNDO;
			constexpr short int flag_nowait = SEM_UNDO | IPC_NOWAIT;

			struct sembuf sops_wait[] = { {1,0,flag},{2,1,flag},{3,0,flag} };
			struct sembuf sops_nowait[] = { {1,0,flag_nowait},{2,1,flag_nowait},{3,0,flag_nowait} };

			struct sembuf* sops = sops_wait;
			if (no_wait)sops = sops_nowait;

			if (-1 != semop(sem_id, sops, 3))
			{
				after_RLock();
				return true;
			}
			else
			{
				*(int*)&m_errid = __LINE__;
				(*(stringstream*)&m_errmsg).str("");
				*(stringstream*)&m_errmsg << errno2str();
				return false;
			}
		}
		bool WLock()const { return _WLock(false); }
		bool TryWLock()const { return _WLock(true); }
		bool _WLock(bool no_wait)const
		{
			if (isSafe)
			{
				if (getThreadData()->isLocked())
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 已经锁定，需要先解锁";
					return false;
				}
			}
			if (isIngore)
			{
				after_WLock();
				return true;//忽略锁定
			}

			STATIC_C short int const flag = SEM_UNDO;
			STATIC_C short int const flag_nowait = SEM_UNDO | IPC_NOWAIT;

			struct sembuf sops_wait[] = { {0,-1,flag},{1,1,flag},{2,0,flag},{3,1,flag} };
			struct sembuf sops_nowait[] = { {0,-1,flag_nowait},{1,1,flag_nowait},{2,0,flag_nowait},{3,1,flag_nowait},{3,-1,flag_nowait} };

			struct sembuf* sops = sops_wait;
			if (no_wait)sops = sops_nowait;

			//先增加写等待数，阻止读请求被激活
			if (-1 == semop(sem_id, &sops[3], 1))
			{
				*(int*)&m_errid = __LINE__;
				(*(stringstream*)&m_errmsg).str("");
				*(stringstream*)&m_errmsg << errno2str();
				return false;
			}
			if (-1 != semop(sem_id, sops, 3))
			{
				after_WLock();
				return true;
			}
			else if (no_wait && EAGAIN == errno)
			{//撤销写等待
				if (-1 == semop(sem_id, &sops[4], 1))
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << errno2str();
				}
				return false;
			}
			else
			{
				*(int*)&m_errid = __LINE__;
				(*(stringstream*)&m_errmsg).str("");
				*(stringstream*)&m_errmsg << errno2str();
				return false;
			}
		}
		//解除锁定
		bool RUnLock()const
		{
			if (isSafe)
			{
				if (!getThreadData()->isRLocked())
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 未锁定或不是读锁定";
					return false;
				}
			}
			if (isIngore)
			{
				after_RUnLock();
				return true;//忽略锁定
			}
			struct sembuf sops[] = { {2,-1,SEM_UNDO} };
			if (-1 != semop(sem_id, sops, 1))
			{
				after_RUnLock();
				return true;
			}
			else
			{
				*(int*)&m_errid = __LINE__;
				(*(stringstream*)&m_errmsg).str("");
				*(stringstream*)&m_errmsg << errno2str();
				return false;
			}
		}
		bool WUnLock()const
		{
			if (isSafe)
			{
				if (!getThreadData()->isWLocked())
				{
					*(int*)&m_errid = __LINE__;
					(*(stringstream*)&m_errmsg).str("");
					*(stringstream*)&m_errmsg << "sem " << sem_id << " 未锁定或不是写锁定";
					return false;
				}
			}
			if (isIngore)
			{
				after_WUnLock();
				return true;//忽略锁定
			}
			struct sembuf sops[] = { {0,1,SEM_UNDO},{1,-1,SEM_UNDO},{3,-1,SEM_UNDO} };
			if (-1 != semop(sem_id, sops, 3))
			{
				after_WUnLock();
				return true;
			}
			else
			{
				*(int*)&m_errid = __LINE__;
				(*(stringstream*)&m_errmsg).str("");
				*(stringstream*)&m_errmsg << errno2str();
				return false;
			}
		}

		string GetErrorMessage()const { return m_errmsg.str(); }
		int GetErrorID()const { return m_errid; }//获得最新的错误ID
		bool isFree()const
		{
			bool ignored;
			long w_count;
			long r_count;
			long w_wait;
			if (GetCount(ignored, w_count, r_count, w_wait))
			{
				return 0 == w_count + r_count + w_wait;
			}
			return false;
		}
		bool GetCount(bool& ignored, long& w_count, long& r_count, long& w_wait)const
		{
			long w;
			ignored = isIngore;
			return GetCount2(w, w_count, r_count, w_wait);
		}
		bool GetCount2(long& w, long& w_count, long& r_count, long& w_wait)const
		{
			w = 0;
			w_count = 0;
			r_count = 0;
			w_wait = 0;
			if (sem_id != -1)
			{
				union semun sem;
				int val;
				sem.val = 0;
				if ((val = semctl(sem_id, 0, GETVAL, sem)) < 0)
				{
					return false;
				}
				else
				{
					w = val;
				}
				if ((val = semctl(sem_id, 1, GETVAL, sem)) < 0)
				{
					return false;
				}
				else
				{
					w_count = val;
				}
				if ((val = semctl(sem_id, 2, GETVAL, sem)) < 0)
				{
					return false;
				}
				else
				{
					r_count = val;
				}
				if ((val = semctl(sem_id, 3, GETVAL, sem)) < 0)
				{
					return false;
				}
				else
				{
					w_wait = val;
				}
			}
			return true;
		}
		bool GetCTime(time_t& ctime)const
		{
			return _GetCTime(ctime);
		}
	};

}
