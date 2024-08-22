//shm_Set_Mutex.h 共享内存Set容器 带互斥的版本
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmstd.h"
#include "shmSet_NoMutex.h"

namespace ns_my_std
{
	//完整互斥的
#define _T_SHM_SET_R_LOCK_BEGIN 	\
			if(m_mutex.RLock())\
			{

#define _T_SHM_SET_R_LOCK_END \
			}\
			else\
			{\
				thelog<<GetName()<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;\
				return ret;\
			}\
\
			if(!m_mutex.RUnLock())\
			{\
				thelog<<GetName()<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;\
				return ret;\
			}\
			return ret;

#define _T_SHM_SET_W_LOCK_BEGIN 	\
			if(m_mutex.WLock())\
			{

#define _T_SHM_SET_W_LOCK_END \
			}\
			else\
			{\
				thelog<<GetName()<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;\
				return ret;\
			}\
\
			if(!m_mutex.WUnLock())\
			{\
				thelog<<GetName()<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;\
				return ret;\
			}\
			return ret;

//#define T_SHM_SET_R_LOCK_BEGIN thelog<<m_mutex.Report()<<endi;_T_SHM_SET_R_LOCK_BEGIN;
//#define T_SHM_SET_R_LOCK_END thelog<<m_mutex.Report()<<endi;_T_SHM_SET_R_LOCK_END;
//#define T_SHM_SET_W_LOCK_BEGIN thelog<<m_mutex.Report()<<endi;_T_SHM_SET_W_LOCK_BEGIN;
//#define T_SHM_SET_W_LOCK_END sleep(10);thelog<<m_mutex.Report()<<endi;_T_SHM_SET_W_LOCK_END;
#define T_SHM_SET_R_LOCK_BEGIN _T_SHM_SET_R_LOCK_BEGIN
#define T_SHM_SET_R_LOCK_END _T_SHM_SET_R_LOCK_END
#define T_SHM_SET_W_LOCK_BEGIN _T_SHM_SET_W_LOCK_BEGIN
#define T_SHM_SET_W_LOCK_END _T_SHM_SET_W_LOCK_END

	template<typename T_DATA,int PI_N,typename T_USER_HEAD=CDemoData,int PART=0,int VER=0,typename T_HANDLE=T_HANDLE_ARRAY<T_TREE_NODE_STRUCT<T_DATA > ,PI_N > >
	class T_SHMSET : public T_SHMSET_NO_MUTEX<T_DATA, PI_N, T_USER_HEAD, PART, VER, less<T_DATA>, T_HANDLE >, public ISet<T_DATA>
	{
	public:
		typedef T_SHMSET_NO_MUTEX<T_DATA, PI_N, T_USER_HEAD, PART, VER, less<T_DATA>, T_HANDLE> T_SET;

		using T_SET::end;
		using T_SET::GetName;
	public:
		struct iterator : public T_SET::iterator
		{
			using T_SET::iterator::handle;

			iterator(){handle=-1;}
			iterator(typename T_SET::iterator it){handle=it.handle;}
			iterator & operator=(typename T_SET::iterator const & it){handle=it.handle;return *this;}
			iterator & operator ++ ()
			{
				T_SHMSET * pSet=(T_SHMSET*)GET_PP_SET(PI_N);
				if(pSet->m_mutex.RLock())
				{//thelog<<"锁定"<<endi;
					iterator old=*this;
					*this=pSet->T_SET::lower_bound(**this);
					if(*this!=pSet->end())
					{
						if(**this<*old || *old<**this)
						{
						}
						else
						{
							T_SET::iterator::operator ++();
						}
					}
				}
				else
				{
					thelog<<pSet->GetName()<<"互斥失败： "<<pSet->m_mutex.GetErrorMessage()<<ende;
					handle=-1;
					return *this;
				}

				if(!pSet->m_mutex.RUnLock())
				{
					thelog<<pSet->GetName()<<"解锁失败 ： "<<pSet->m_mutex.GetErrorMessage()<<ende;
					handle=-1;
					return *this;
				}//thelog<<"解锁"<<endi;
				return *this;
			}
			iterator & operator -- ()
			{
				T_SHMSET * pSet=(T_SHMSET*)GET_PP_SET(PI_N);
				if(pSet->m_mutex.RLock())
				{//thelog<<"锁定"<<endi;
					iterator old=*this;
					*this=pSet->T_SET::lower_bound(**this);
					if(*this!=pSet->end())
					{
						T_SET::iterator::operator --();
					}
				}
				else
				{
					thelog<<pSet->GetName()<<"互斥失败： "<<pSet->m_mutex.GetErrorMessage()<<ende;
					handle=-1;
					return *this;
				}

				if(!pSet->m_mutex.RUnLock())
				{
					thelog<<pSet->GetName()<<"解锁失败 ： "<<pSet->m_mutex.GetErrorMessage()<<ende;
					handle=-1;
					return *this;
				}//thelog<<"解锁"<<endi;
				return *this;
			}
		};
		typedef iterator const_iterator;
		friend struct iterator;
		//friend struct const_iterator;
	private:
		CManagedMutex m_mutex;
	public:
		T_SHMSET(char const * name,int version):T_SET(name,version),m_mutex(name,PART)
		{
			GET_PP_SET(PI_N)=this;
			if(!m_mutex.AttachOrCreateMutexIfNeed())throw string(name)+" : AttachOrCreateMutexIfNeed出错";
		}
		T_SHMSET(char const * name):T_SET(name,VER),m_mutex(name,PART)
		{
			GET_PP_SET(PI_N)=this;
			if(!m_mutex.AttachOrCreateMutexIfNeed())throw string(name)+" : AttachOrCreateMutexIfNeed出错";
		}
		const_iterator begin()const
		{
			const_iterator ret=end();
			T_SHM_SET_R_LOCK_BEGIN;
			ret=T_SET::begin();
			T_SHM_SET_R_LOCK_END;
		}
		//const_iterator end()const//不需要互斥
		const_iterator rbegin()const
		{
			const_iterator ret=end();
			T_SHM_SET_R_LOCK_BEGIN;
			ret=T_SET::rbegin();
			T_SHM_SET_R_LOCK_END;
		}
		//const_iterator rend()const//不需要互斥
		bool clear()
		{
			bool ret=true;
			T_SHM_SET_W_LOCK_BEGIN;
			T_SET::clear();
			T_SHM_SET_W_LOCK_END;
		}
		pair<iterator, bool> insert(T_DATA const & data)
		{
			pair<iterator, bool>  ret;
			pair<typename T_SET::iterator, bool>  tmp;
			ret.first=end();
			ret.second=false;
			T_SHM_SET_W_LOCK_BEGIN;
			tmp=T_SET::insert(data);
			ret.first=tmp.first;
			ret.second=tmp.second;
			T_SHM_SET_W_LOCK_END;
		}
		template<typename T_FIND >
		const_iterator find(T_FIND const & tmp)const
		{
			const_iterator ret=end();
			T_SHM_SET_R_LOCK_BEGIN;
			ret=T_SET::find(tmp);
			T_SHM_SET_R_LOCK_END;
		}
		//删除并指向下一个
		iterator & DeleteAndMoveNext(iterator & ret)
		{
			T_SHM_SET_W_LOCK_BEGIN;
			T_SET::DeleteAndMoveNext(ret);
			T_SHM_SET_W_LOCK_END;
		}
		bool erase(const_iterator it)
		{
			bool ret=true;
			T_SHM_SET_W_LOCK_BEGIN;
			T_SET::erase(it);
			T_SHM_SET_W_LOCK_END;
		}
		bool erase(T_DATA const & data)
		{
			bool ret=true;
			T_SHM_SET_W_LOCK_BEGIN;
			T_SET::erase(data);
			T_SHM_SET_W_LOCK_END;
		}
		//用全比较函数，实际是find的功能
		template<typename T_FIND >
		const_iterator lower_bound(T_FIND const & tmp)const
		{
			const_iterator ret=end();
			T_SHM_SET_R_LOCK_BEGIN;
			ret=T_SET::lower_bound(tmp);
			T_SHM_SET_R_LOCK_END;
		}
		//用部分比较函数（但必须是符合顺序的，否则结果不可预期）
		template<typename T_FIND,typename T_LESS_BOUND >
		const_iterator lower_bound(T_FIND const & tmp,T_LESS_BOUND comp)const
		{
			const_iterator ret=end();
			T_SHM_SET_R_LOCK_BEGIN;
			ret=T_SET::lower_bound(tmp,comp);
			T_SHM_SET_R_LOCK_END;
		}
		//用部分比较函数（但必须是符合顺序的，否则结果不可预期）
		template<typename T_FIND,typename T_LESS_BOUND >
		const_iterator upper_bound(T_FIND const & tmp,T_LESS_BOUND comp)const
		{
			const_iterator ret=end();
			T_SHM_SET_R_LOCK_BEGIN;
			ret=T_SET::upper_bound(tmp,comp);
			T_SHM_SET_R_LOCK_END;
		}
		//受保护的操作（但必须是针对非key字段的，否则结果不可预期）
		template<typename T_PROCESS >
		T_DATA * safe_process(T_DATA * p, long d, T_PROCESS process)
		{
			T_DATA * ret = p;
			T_SHM_SET_R_LOCK_BEGIN;
			ret = process(p, d);
			T_SHM_SET_R_LOCK_END;
		}
	public:
		virtual bool disableMutex()const{m_mutex.ingore();return true;}
	public://ISet接口
		virtual T_DATA * isetGet(long h)const
		{
			iterator it;
			it.handle=h;
			return &*it;
		}
		virtual long isetMoveNext(long & h)const
		{
			iterator it;
			it.handle=h;
			++it;
			return h=it.handle;
		}
		virtual long isetBegin()const
		{
			return begin().handle;
		}
		virtual pair<long,bool> isetInsert(T_DATA const & value)
		{
			pair<long,bool> ret;
			pair<iterator, bool> tmppair=insert(value);
			ret.first=tmppair.first.handle;
			ret.second=tmppair.second;
			return ret;
		}
		virtual long isetFind(T_DATA const & value)const
		{
			return find(value).handle;
		}
		virtual long isetFindLowerBound(T_DATA const & value, bool(*less)(T_DATA const &, T_DATA const &))const
		{
			return lower_bound(value,less).handle;
		}
		virtual bool isetErase(long h)
		{
			iterator it;
			it.handle=h;
			return erase(it);
		}
	};

}
