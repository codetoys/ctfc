//mymutex3.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <errno.h>
#include "function.h"
#include <atomic>
#include <thread>

namespace ns_my_std
{
	//对象实例不可复制不可移动，内部记录进程操作状态和线程操作状态
	class CMyRWMutex3
	{
	public:
		struct mySEM
		{
		public:
			mutable atomic_flag flag{false};

		private:
			bool _Lock(bool no_wait)
			{
				while (flag.test_and_set())
				{
					if (no_wait)return false;
					this_thread::yield();
				}
				return true;
			}
			bool _UnLock()
			{
				flag.clear();
				return true;
			}
		public:
			bool isLocked()const
			{
				if (flag.test_and_set())return true;
				else
				{
					flag.clear();
					return false;
				}
			}
			void init()
			{
				flag.clear();
			}
			bool RLock(bool no_wait)
			{
				return _Lock(no_wait);
			}
			bool RUnLock()
			{
				return _UnLock();
			}
			bool WLock(bool no_wait)
			{
				return _Lock(no_wait);
			}
			bool WUnLock()
			{
				return _UnLock();
			}
		};
	private:
		mutable mySEM  * sem_id;//信号量ID
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
			thread_local map<CMyRWMutex3 const*, thread_data > d;//通过对象地址区分不同的对象
			return &d[this];
		}

		//禁止移动和复制（不能用于vector，因为vector会移动对象）
		CMyRWMutex3() = default;
		CMyRWMutex3(CMyRWMutex3 const&) = delete;
		CMyRWMutex3& operator =(CMyRWMutex3 const&) = delete;
		CMyRWMutex3(CMyRWMutex3 const&&) = delete;
		CMyRWMutex3& operator =(CMyRWMutex3 const&&) = delete;

		~CMyRWMutex3()
		{
			if (0 != count_WLock || 0 != count_RLock)
			{
				if (0 != count_WLock) cout << "警告：析构而未解锁：" << sem_id << " is w locked " << count_WLock << endl;
				if (0 != count_RLock) cout << "警告：析构而未解锁：" << sem_id << " is r locked " << count_RLock << endl;
			}
			sem_id = nullptr;
		}
	private:
		mutable int m_errid{ 0 };//最近的错误号
		mutable CMyStringStream m_errmsg;//最近的错误信息

	public:
		string Report()const
		{
			char buf[1024];
			string ret;
			if (nullptr != sem_id)
			{
				sprintf(buf, "sem_id = %p , W %d R %d (%s), %s %s", sem_id, count_WLock.load(), count_RLock.load()
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
		bool isConnected()const { return nullptr != sem_id; }
		bool Attach(mySEM* id)
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

			return nullptr != sem_id;
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
			sem_id = nullptr;
			return true;
		}

		//创建新信号量
		bool Create(mySEM * id)
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
			if (nullptr == sem_id)
			{
				m_errid = __LINE__;
				m_errmsg.str("");
				m_errmsg << "id is nullptr";
				return false;
			}
			sem_id->init();
			return true;
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
			if (sem_id->RLock(no_wait))
			{
				after_RLock();
				return true;
			}
			else
			{
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

			if (sem_id->WLock(no_wait))
			{
				after_WLock();
				return true;
			}
			else
			{
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
			if (sem_id->RUnLock())
			{
				after_RUnLock();
				return true;
			}
			else
			{
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
			if (sem_id->WUnLock())
			{
				after_WUnLock();
				return true;
			}
			else
			{
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
			if (nullptr != sem_id)
			{
				w = sem_id->isLocked();
				w_count = 0;
				r_count = 0;
				w_wait = 0;
			}
			return true;
		}
	};

}
