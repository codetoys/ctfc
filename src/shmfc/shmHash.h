//shm_Hash.h 共享内存HASH容器
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmArray.h"
#include "shmMultiList.h"

namespace ns_my_std
{
	//共享内存HASH容器，T_BUCKET_SIZE桶的数目类型
	template<typename T_DATA, int PI_HASH, int PI_DATA >
	class T_SHM_HASH : public CShmActiveObjects
	{
	private:
		typedef T_MULTI_LIST<T_DATA, PI_DATA, CDemoData > T_DATAS;
		struct HASH_HEAD
		{
			size_t old_bucket_count = 0;//原来的桶的数目
			size_t bucket_moved = 0;//已经移动的桶的数目
			size_t bucket_count = 4;//桶的数目

			friend ostream& operator<<(ostream& o, HASH_HEAD const& data)
			{
				string str;
				return o << data.toString(str);
			}
			string& toString(string& str)const
			{
				stringstream ss;
				ss << "bucket_count " << bucket_count << "（" << bucket_moved << " of " << old_bucket_count << "）";
				return str = ss.str();
			}
		};
		typedef T_ARRAY<typename T_DATAS::iterator, PI_HASH, HASH_HEAD > T_HASHS;

		T_HASHS m_hashs;
		T_DATAS m_datas;

		CManagedMutex m_mutex;//读写锁
		thread m_thread;//辅助线程
		bool m_ThreadExited = false;//辅助线程已停止
		bool m_StopThread = false;//命令辅助线程停止

	public:
		virtual char const* GetName()const { return m_hashs.GetName(); }
		T_SHM_HASH(char const* name, int version) :m_hashs(name, version), m_datas((string(name) + "_D").c_str(), version)
		{
			if (!m_mutex.AttachOrCreateMutexIfNeed(name))throw "同步对象初始化失败";
			if (!AddTable(&m_hashs))throw "内存不足";
			if (!AddTable(m_datas.getIShmActiveObject()))throw "内存不足";
		}
		~T_SHM_HASH()
		{
			if (m_thread.joinable())
			{
				time_t t1 = time(NULL);
				while (m_thread.joinable())
				{
					if (time(nullptr) - t1 > 10)
					{
						thelog << "辅助线程未能停止" << ende;
						return;
					}
					m_StopThread = true;
					this_thread::yield();
				}
			}
		}
		virtual bool ReportData()const
		{
			thelog << "ReportData =================================" << endi;
			//Report();
			typename T_HASHS::HANDLE h;
			string str;
			thelog << m_hashs.GetUserHead()->toString(str) << endi;
			size_t max_count = 20;
			size_t i = 0;
			bool ____ = false;
			for (h = m_hashs.Begin(); h < m_hashs.size(); ++h,++i)
			{
				if (i < max_count || (m_hashs.size() > max_count && i > m_hashs.size() - max_count))
				{
					thelog << "------------ " << h.handle << " " << *h << endi;
					typename T_DATAS::iterator it = *h;
					while (!it.isEnd())
					{
						thelog << "               " << it.handle << " " << *it << endi;
						++it;
					}
				}
				else
				{
					if (!____)
					{
						thelog << "......" << endi;
						____ = true;
					}
				}
			}
			return true;
		}

	private://非互斥层，必须以下划线开头
		size_t _KeyHashToBucket(size_t kh, size_t buckets)const
		{
			return kh % buckets;
		}
		//扩展但并不移动数据，返回true表示扩展成功或不需要扩展
		bool _Extend()
		{
			DEBUG_LOG << "_Extend " << *m_hashs.GetUserHead() << endi;
			
			if (m_hashs.GetUserHead()->old_bucket_count != m_hashs.GetUserHead()->bucket_moved)
			{
				DEBUG_LOG << "bucket_moved不是old_bucket_count" << endi;;
				return false;
			}
			if (m_hashs.GetUserHead()->bucket_count >= m_datas.size() / 2)
			{
				return true;//不需要扩展
			}
			
			m_hashs.GetUserHead()->old_bucket_count = m_hashs.GetUserHead()->bucket_count;
			m_hashs.GetUserHead()->bucket_moved = 0;
			while (m_hashs.GetUserHead()->bucket_count < m_datas.size() / 2)
			{
				DEBUG_LOG << "_Extend " << *m_hashs.GetUserHead() << endi;
				m_hashs.GetUserHead()->bucket_count *= 2;
			}
			m_hashs.Reserve(m_hashs.GetUserHead()->bucket_count);
			
			DEBUG_LOG << "_Extend " << *m_hashs.GetUserHead() << endi;
			
			_ActiveThread();//激活辅助线程
			return true;
		}
		//激活辅助线程
		void _ActiveThread()
		{
			DEBUG_LOG << "joinable " << m_thread.joinable() << " m_ThreadExited " << m_ThreadExited << endi;
			if (m_thread.joinable() && m_ThreadExited)
			{
				m_thread.join();
			}
			DEBUG_LOG << "joinable " << m_thread.joinable() << " m_ThreadExited " << m_ThreadExited << endi;
			if (!m_thread.joinable())
			{
				m_ThreadExited = false;
				m_thread = thread(Thread_MoveData, this);
				DEBUG_LOG << "辅助线程已激活" << endi;
			}
		}
		//添加入口
		bool _AddData(T_DATA const& data, T_DATA*& pData)
		{
			DEBUG_LOG << "_AddData" << endi;
			
			_Extend();
			
			size_t buckets = m_hashs.GetUserHead()->bucket_count;
			size_t hash = _KeyHashToBucket(data.keyhash(), buckets);

			if (m_hashs.capacity() < buckets)
			{
				m_hashs.Reserve(buckets);
			}
			while (hash >= m_hashs.size())
			{
				static typename T_HASHS::HANDLE h;
				static typename T_DATAS::iterator null_it;
				m_hashs.Add(null_it, h);
			}

			if (!m_datas.AddHead(*m_hashs.Get(hash), data))
			{
				pData = nullptr;
				thelog << ende;
				return false;
			}
			pData = &**m_hashs.Get(hash);
			return true;
		}
		//获得一个入口，返回入口所在的列表头位置和入口位置，入口头位置将用于删除
		bool __GetData(T_DATA const& data, T_DATA*& pData, size_t buckets)
		{
			size_t hash = _KeyHashToBucket(data.keyhash(), buckets);
			if (hash >= m_hashs.size())
			{
				pData = nullptr;
				return false;
			}

			typename T_DATAS::iterator it = *m_hashs.Get(hash);
			for (; !it.isEnd(); ++it)
			{
				if (*it == data)
				{
					pData = &*it;
					return true;
				}
			}
			pData = nullptr;
			return false;
		}
		bool _GetData(T_DATA const& data, T_DATA*& pData)
		{
			size_t buckets = m_hashs.GetUserHead()->bucket_count;
			size_t old_buckets = m_hashs.GetUserHead()->old_bucket_count;
			if (m_hashs.GetUserHead()->bucket_moved != old_buckets)
			{
				_ActiveThread();//激活辅助线程
				size_t hash = _KeyHashToBucket(data.keyhash(), old_buckets);
				if (hash >= m_hashs.GetUserHead()->bucket_moved)
				{
					if (__GetData(data, pData, old_buckets))
					{
						return true;
					}
				}
			}
			return __GetData(data, pData, buckets);
		}
		//返回0表示已经处理完
		size_t _Thread_MoveOneData()
		{
			size_t buckets = m_hashs.GetUserHead()->bucket_count;
			size_t old_buckets = m_hashs.GetUserHead()->old_bucket_count;
			DEBUG_LOG << "_Thread_MoveOneData " << *m_hashs.GetUserHead() << endi;

			for (size_t i = m_hashs.GetUserHead()->bucket_moved; i < old_buckets; ++i)
			{
				typename T_DATAS::iterator& head = *m_hashs.Get(i);

				if (head.isEnd())continue;

				typename T_DATAS::iterator  it = head;
				while (!it.isEnd())
				{
					size_t hash = _KeyHashToBucket(it->keyhash(), buckets);
					DEBUG_LOG << *it << " hash " << hash << " buckets " << buckets << endi;
					if (hash != i)
					{
						DEBUG_LOG << "i " << i << " 需要移动 head " << head << " it " << it << endi;
						T_DATA tmp = *it;
						typename T_DATAS::iterator it_to_delete = it;
						++it;
						m_datas.DeleteListOne(head, it_to_delete);
						T_DATA* pTmp;
						_AddData(tmp, pTmp);
					}
					else
					{
						DEBUG_LOG << "i " << i << " 不需要移动" << endi;
						++it;
					}

				}
				++m_hashs.GetUserHead()->bucket_moved;
				return 1;
			}
			return 0;
		}
	public:
		//辅助线程
		static void Thread_MoveData(T_SHM_HASH* o)
		{
			thelog << "辅助线程已启动" << endi;
			while (true)
			{
				o->m_mutex.WLock();
				if (0 == o->_Thread_MoveOneData())
				{
					o->m_ThreadExited = true;
					o->m_mutex.WUnLock();
					thelog << "辅助线程已停止" << endi;
					return;
				}
				o->m_mutex.WUnLock();
			}
		}
		//手工执行一次辅助线程
		void Extend()
		{
			m_mutex.WLock();
			_Extend();
			m_mutex.WUnLock();
			
			m_thread.join();
		}
		bool AddData(T_DATA const& data, T_DATA*& pData)
		{
			m_mutex.WLock();
			bool ret = _AddData(data, pData);
			m_mutex.WUnLock();
			return ret;
		}
		bool GetData(T_DATA const& data, T_DATA*& pData)
		{
			m_mutex.WLock();
			bool ret = _GetData(data, pData);
			m_mutex.WUnLock();
			return ret;
		}
		static int T_SHM_HASH_test(int argc, char** argv, char const* name, int version)
		{
			G_IS_DEBUG = false;
			try
			{
				T_SHM_HASH<T_DATA, PI_HASH, PI_DATA> a(name, version);
				if (a.Attach(false) || (a.CreateShm() && a.Attach(false) && a.m_mutex.Reset()))
				{
					a.RunCmdUI();
					a.clear();

					long count = 100000;
					CDemoData data;
					CDemoData* data_from_shm;
					for (int i = 0; i < count; ++i)
					{
						DEBUG_LOG << i << endi;
						data.n = i + 100;
						if (!a.AddData(data, data_from_shm))return __LINE__;
					}
					for (int i = 0; i < count; ++i)
					{
						data.n = i + 100;
						if (!a.GetData(data, data_from_shm))return __LINE__;
					}
					a.ReportData();
					a.Extend();
					a.ReportData();
					a.RunCmdUI();
					a.Detach();
					//a.DestoryShm();
					return 0;
				}
				else
				{
					return __LINE__;
				}
			}
			catch (char const* e)
			{
				thelog << e << ende;
				abort();
			}
		}
	};
}
