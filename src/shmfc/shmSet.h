//shm_Set.h 共享内存Set容器 扩展接口
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*
T_SHMSET_MUTEX 带互斥的二叉树
T_SHM_MUTEX_SET 带互斥的二叉树
*/

#include "shmSet_NoMutex.h"
#include "shmSet_Mutex.h"

namespace ns_my_std
{
	//扩展了互斥接口的
	template<typename T_DATA,int PI_N,typename T_USER_HEAD=CDemoData,int VER=0>
	class T_SHMSET_MUTEX : public T_SHMSET_NO_MUTEX<T_DATA,PI_N,T_USER_HEAD,0,VER>
	{
	public:
		typedef T_SHMSET_NO_MUTEX<T_DATA,PI_N,T_USER_HEAD,0,VER> T_SET;
		typedef typename T_SET::iterator iterator;
		typedef typename T_SET::const_iterator const_iterator;
	public:
		using T_SET::end;
		using T_SET::clear;
		using T_SET::GetName;
		using T_SET::Report;
		using T_SET::find;
	private:
		CManagedMutex m_mutex;
		bool _get(T_DATA & value)const
		{
			iterator it=find(value);
			if(it!=end())
			{
				value=*it;
				return true;
			}
			else
			{
				return false;
			}
		}
		bool _update(T_DATA const & value)
		{
			iterator it = find(value);
			if(it!=end())
			{
				*it=value;
				return true;
			}
			else
			{
				pair<iterator, bool> tmp=insert(value);
				return tmp.first!=end();
			}
		}
		bool _erase(T_DATA const & value)
		{
			iterator it=find(value);
			if(it!=end())
			{
				erase(it);
				return true;
			}
			else
			{
				return false;
			}
		}
	public:
		T_SHMSET_MUTEX(char const * name,int version):T_SET(name,version),m_mutex(name){}
		T_SHMSET_MUTEX(char const * name):T_SET(name,VER),m_mutex(name){}
		//IShmActiveObject接口
		virtual bool CreateShm()
		{
			return T_SET::CreateShm() && m_mutex.AttachOrCreateMutexIfNeed();
		}
		virtual bool _Attach(bool isReadOnly)
		{
			return T_SET::_Attach(isReadOnly) && m_mutex.AttachOrCreateMutexIfNeed();
		}
	public://互斥操作，全部以M为前缀
		//清理全部数据
		bool MClear()
		{
			bool ret=true;
			if(m_mutex.WLock())
			{
				clear();
			}
			else
			{
				thelog<<GetName()<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<GetName()<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		//报告内容，调试用
		string & MReport(string & str)const
		{
			if(m_mutex.RLock())
			{
				Report(str);
			}
			else
			{
				str=m_mutex.GetErrorMessage();
			}

			if(!m_mutex.RUnLock())
			{
				str+=m_mutex.GetErrorMessage();
			}
			return str;
		}
		//直接存取
		bool MGet(T_DATA & value)const
		{
			bool ret=true;
			if(m_mutex.RLock())
			{
				ret=_get(value);
			}
			else
			{
				thelog<<GetName()<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.RUnLock())
			{
				thelog<<GetName()<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		bool MUpdate(T_DATA const & value)
		{
			bool ret=true;
			if(m_mutex.WLock())
			{
				ret=_update(value);
			}
			else
			{
				thelog<<GetName()<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<GetName()<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		bool MErase(T_DATA const & value)
		{
			bool ret=true;
			if(m_mutex.WLock())
			{
				ret=_erase(value);
			}
			else
			{
				thelog<<GetName()<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<GetName()<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
	};

	//简化的带有互斥的SET，非标准接口
	template<typename T_DATA,int PI_N>
	class T_SHM_MUTEX_SET
	{
	public:
		typedef T_SHMSET_NO_MUTEX<T_DATA,PI_N> T_SET;
		typedef typename T_SET::iterator iterator;
		typedef iterator const_iterator;
	private:
		T_SET m_shmset;
		CManagedMutex m_mutex;

		bool _get(T_DATA & value)const
		{
			iterator it=m_shmset.find(value);
			if(it!=end())
			{
				value=*it;
				return true;
			}
			else
			{
				return false;
			}
		}
		bool _update(T_DATA const & value)
		{
			iterator it=m_shmset.find(value);
			if(it!=end())
			{
				*it=value;
				return true;
			}
			else
			{
				pair<iterator, bool> tmp=m_shmset.insert(value);
				return tmp.first!=m_shmset.end();
			}
		}
		template<typename T_FUN_B1>
		bool _Clear(T_FUN_B1 fun)
		{
			iterator it,old_it;
			long count_all=0;
			long count=0;

			thelog<<"根据规则清理......"<<endi;
			it=begin();
			while(it!=end())
			{
				old_it=it;
				++it;
				if(fun(*old_it))
				{
					m_shmset.erase(old_it);
					++count;
				}
				++count_all;
				if(0==count_all%10000)thelog<<count_all<<" "<<count<<endi;
			}
			thelog<<"清理完成，清理["<<count<<"]个。"<<endi;
			return true;
		}
	public:
		T_SHM_MUTEX_SET(char const * name,int version):m_shmset(name,version),m_mutex(name){}
		//根据配置的大小创建共享内存，仅供管理员使用
		bool adminRatableCreateShm()
		{
			if(!m_shmset.set_CreateShm(0))return false;

			return m_mutex.AttachOrCreateMutexIfNeed();
		}
		//保存到文件
		bool SaveToDir(char const * dir_name)
		{
			return m_shmset.SaveToDir(dir_name);
		}
		//从文件中加载
		bool LoadFromDir(char const * dir_name)
		{
			return m_shmset.LoadFromDir(dir_name);
		}
		//连接和断开，管理员和一般使用者使用
		bool init(bool isReadOnly)
		{
			if(!m_shmset.set_AttachToShm(isReadOnly))
			{
				thelog<<"连接到共享内存失败"<<ende;
				return false;
			}
			return m_mutex.AttachOrCreateMutexIfNeed();
		}
		bool uninit()
		{
			return m_shmset.set_DetachFromShm();
		}
		//遍历，无互斥
		iterator begin()const{return m_shmset.begin();}
		iterator end()const{return m_shmset.end();}
		//有效数据个数和容量
		long size()const{return m_shmset.size();}
		long capacity()const{return m_shmset.capacity();}
		//清理全部数据
		bool Destroy()
		{
			bool ret=true;
			if(m_mutex.WLock())
			{
				m_shmset.clear();
			}
			else
			{
				thelog<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		//报告内容，调试用
		string & Report(string & str)const
		{
			if(m_mutex.RLock())
			{
				m_shmset.Report(str);
			}
			else
			{
				str=m_mutex.GetErrorMessage();
			}

			if(!m_mutex.RUnLock())
			{
				str+=m_mutex.GetErrorMessage();
			}
			return str;
		}
		//直接存取
		bool get(T_DATA * value)const{return get(*value);}
		bool get(T_DATA & value)const
		{
			bool ret=true;
			if(m_mutex.RLock())
			{
				ret=_get(value);
			}
			else
			{
				thelog<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.RUnLock())
			{
				thelog<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		bool update(T_DATA const & value)
		{
			bool ret=true;
			if(m_mutex.WLock())
			{
				ret=_update(value);
			}
			else
			{
				thelog<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		bool get_and_update(T_DATA & oldvalue, T_DATA const & newvalue)
		{
			bool ret=true;
			if(m_mutex.WLock())
			{
				_get(oldvalue);
				ret=_update(newvalue);
			}
			else
			{
				thelog<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
		template<typename T_FUN_B1>
		bool Clear(T_FUN_B1 fun)
		{
			bool ret=false;
			if(m_mutex.WLock())
			{
				ret=_Clear(fun);
			}
			else
			{
				thelog<<"互斥失败： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}

			if(!m_mutex.WUnLock())
			{
				thelog<<"解锁失败 ： "<<m_mutex.GetErrorMessage()<<ende;
				return false;
			}
			return ret;
		}
	};

	//互斥对象
	template <typename T_DATA>
	class CMutexGroup
	{
	private:
		long m_count;//个数
		CManagedMutex * m_pMutexes;
	public:
		CMutexGroup():m_count(0),m_pMutexes(NULL){}
		~CMutexGroup()
		{
			Clear();
		}
		void Clear()
		{
			if(NULL!=m_pMutexes)delete[] m_pMutexes;
			m_count=0;
			m_pMutexes=NULL;
		}
		bool initCMutexGroup(char const * name)
		{
			Clear();

			m_count = 10;
			if (NULL == (m_pMutexes = new CManagedMutex[m_count]))
			{
				thelog << "内存不足" << ende;
				return false;
			}
			for (long i = 0; i < m_count; ++i)
			{
				new(&m_pMutexes[i]) CManagedMutex(name, i+1);
			}
			return true;
		}
		bool AttachOrCreateMutexIfNeed()
		{
			for (long i = 0; i < m_count; ++i)
			{
				if (!m_pMutexes[i].AttachOrCreateMutexIfNeed())
				{
					return false;
				}
			}
			return true;
		}
		void ingore()
		{
			for (long i = 0; i < m_count; ++i)
			{
				m_pMutexes[i].ingore();
			}
		}
		void safe()
		{
			for (long i = 0; i < m_count; ++i)
			{
				m_pMutexes[i].safe(true);
			}
		}
		
		//锁定分为 读/写/双读/双写全读/全写/

		bool RLock(T_DATA const & data)const
		{
 			return m_pMutexes[data.keyhash()%m_count].RLock();
		}
		bool RUnLock(T_DATA const & data)const
		{
 			return m_pMutexes[data.keyhash()%m_count].RUnLock();
		}
		bool WLock(T_DATA const & data)const
		{
 			return m_pMutexes[data.keyhash()%m_count].WLock();
		}
		bool WUnLock(T_DATA const & data)const
		{
 			return m_pMutexes[data.keyhash()%m_count].WUnLock();
		}

		//双锁定，要么两个都锁定要么都不锁定
		//总是按照a<b的顺序锁定可以避免死锁
		long _getKey(T_DATA const & data)const
		{
			STATIC_G long c=0;
			if(c++<200)cout<<data.keyhash() % m_count<<endl;
			return data.keyhash() % m_count;
		}
		bool RLock2(T_DATA const & data, T_DATA const & data2)const
		{
			int _ = _getKey(data) - _getKey(data2);
			bool ret;
			if(_<=0)
			{
				ret = m_pMutexes[_getKey(data)].RLock();
			}
			else
			{
				ret = m_pMutexes[_getKey(data2)].RLock();
			}
			if (0 == _ || !ret)
			{//两个对象分区相同或锁定出错
				return ret;
			}
			if(_<=0)
			{
				ret = m_pMutexes[_getKey(data2)].RLock();
			}
			else
			{
				ret = m_pMutexes[_getKey(data)].RLock();
			}
			if (!ret)
			{//第二个锁定失败，取消第一个锁定
				if (_ <= 0)
				{
					ret = m_pMutexes[_getKey(data)].RUnLock();
				}
				else
				{
					ret = m_pMutexes[_getKey(data2)].RUnLock();
				}
			}
			return ret;
		}
		bool RUnLock2(T_DATA const & data, T_DATA const & data2)const
		{
			int _ = _getKey(data) - _getKey(data2);
			if (0 == _)return m_pMutexes[_getKey(data)].RUnLock();
			else return m_pMutexes[_getKey(data)].RUnLock() && m_pMutexes[_getKey(data2)].RUnLock();
		}
		bool WLock2(T_DATA const & data, T_DATA const & data2)const
		{
			int _ = _getKey(data) - _getKey(data2);
			bool ret;
			if(_<=0)
			{
				ret = m_pMutexes[_getKey(data)].WLock();
			}
			else
			{
				ret = m_pMutexes[_getKey(data2)].WLock();
			}
			if (0 == _ || !ret)
			{//两个对象分区相同或锁定出错
				return ret;
			}
			if(_<=0)
			{
				ret = m_pMutexes[_getKey(data2)].WLock();
			}
			else
			{
				ret = m_pMutexes[_getKey(data)].WLock();
			}
			if (!ret)
			{//第二个锁定失败，取消第一个锁定
				if (_ <= 0)
				{
					ret = m_pMutexes[_getKey(data)].WUnLock();
				}
				else
				{
					ret = m_pMutexes[_getKey(data2)].WUnLock();
				}
			}
			return ret;
		}
		bool WUnLock2(T_DATA const & data, T_DATA const & data2)const
		{
			int _ = _getKey(data) - _getKey(data2);
			if (0 == _)return m_pMutexes[_getKey(data)].WUnLock();
			else return m_pMutexes[_getKey(data)].WUnLock() && m_pMutexes[_getKey(data2)].WUnLock();
		}

		bool AllRLock() const
		{
			bool ret;
			for(int i=0;i<m_count;++i)
			{
				ret=m_pMutexes[i].RLock();
				if(!ret) return false;
			}
			return true;
		}
		bool AllRUnLock() const
		{
			bool ret;
			for(int i=0;i<m_count;++i)
			{
				ret=m_pMutexes[i].RUnLock();
				if(!ret) return false;
			}
			return true;
		}
		bool AllWLock() const
		{
			bool ret;
			for(int i=0;i<m_count;++i)
			{
				ret=m_pMutexes[i].WLock();
				if(!ret) return false;
			}
			return true;
		}
		bool AllWUnLock() const
		{
			bool ret;
			for(int i=0;i<m_count;++i)
			{
				ret=m_pMutexes[i].WUnLock();
				if(!ret) return false;
			}
			return true;
		}
		string Report()const
		{
			stringstream ret;
			for (long i = 0; i < m_count; ++i)
			{
				ret << i << " " << m_pMutexes[i].Report() << "\n";
			}
			return ret.str();
		}
	};

	//分为N个子树的SET，非标准接口
	template<typename T_DATA,int PI_N >
	class T_SHM_SET_GROUP : public CShmActiveObjects
	{
	private:
		vector<ISet<T_DATA > *> vISet;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N  ,CDemoData,1> m_shmset;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+1,CDemoData,2> m_shmset1;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+2,CDemoData,3> m_shmset2;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+3,CDemoData,4> m_shmset3;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+4,CDemoData,5> m_shmset4;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+5,CDemoData,6> m_shmset5;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+6,CDemoData,7> m_shmset6;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+7,CDemoData,8> m_shmset7;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+8,CDemoData,9> m_shmset8;
		T_SHMSET_NO_MUTEX_ISET<T_DATA,PI_N+9,CDemoData,10> m_shmset9;
		enum {N_SUBTREE=10};
		long KeyHashToIndex(long kh)const
		{
			//thelog<<kh<<" "<<kh%N_SUBTREE<<endi;
			if(kh>=0)return kh%N_SUBTREE;
			else return (-kh)%N_SUBTREE;;
		}
		ISet<T_DATA > * GetISet(long n)const
		{
			//thelog<<"GetISet "<<n<<endi;
			return vISet[n];
		}
	public:
		struct iterator
		{
			long set_index;
			T_SHM_SIZE handle;
			iterator():set_index(-1),handle(-1){}
			bool operator == (iterator const & tmp)const{return set_index==tmp.set_index && handle==tmp.handle;}
			bool operator != (iterator const & tmp)const{return !(*this==tmp);}
		};
		typedef iterator const_iterator;
	private:
		iterator _find(T_DATA const & value)const
		{
			iterator it;
			it.set_index=KeyHashToIndex(value.keyhash());
			it.handle=GetISet(it.set_index)->isetFind(value);
			if(it.handle<0)new(&it) iterator;
			return it;
		}
		iterator _find_lower_bound(T_DATA const & value, bool(*less)(T_DATA const &, T_DATA const &))const
		{
			iterator it;
			it.set_index=KeyHashToIndex(value.keyhash());
			it.handle=GetISet(it.set_index)->isetFindLowerBound(value,less);
			if(it.handle<0)new(&it) iterator;
			return it;
		}
		pair<iterator, bool> _insert(T_DATA const & value)
		{
			pair<iterator, bool> ret;
			pair<long,bool> tmp;
			ret.first.set_index=KeyHashToIndex(value.keyhash());
			tmp=GetISet(ret.first.set_index)->isetInsert(value);
			ret.first.handle=tmp.first;
			ret.second=tmp.second;
			if(ret.first.handle<0)new(&ret.first) iterator;
			return ret;
		}
		bool _get(T_DATA & value)const
		{
			iterator it=_find(value);
			if(it!=end())
			{
				value=GetByHandle(it);		
				return true;
			}
			else
			{
				return false;
			}
		}
		bool _update(T_DATA const & value)
		{
			iterator it=_find(value);
			if(it!=end())
			{
				GetByHandle(it)=value;
				return true;
			}
			else
			{
				pair<iterator, bool> tmp=_insert(value);
				return tmp.first!=end();
			}
		}
		bool _erase(T_DATA const & value)
		{
			iterator it=_find(value);
			if(it!=end())
			{
				GetISet(it.set_index)->isetErase(it.handle);
				return true;
			}
			else
			{
				return false;
			}
		}
		template<typename T_FUN_B1>
		bool _Clear(T_FUN_B1 fun)
		{
			iterator it,old_it;
			long count_all=0;
			long count=0;

			thelog<<"根据规则清理......"<<endi;
			it=begin();
			while(it!=end())
			{
				old_it=it;
				MoveNext(&it);
				if(fun(GetByHandle(old_it)))
				{
					GetISet(old_it.set_index)->isetErase(old_it.handle);
					++count;
				}
				++count_all;
				if(0==count_all%10000)thelog<<count_all<<" "<<count<<endi;
			}
			thelog<<"清理完成，清理["<<count<<"]个。"<<endi;
			return true;
		}

		//从给定位置开始清理一段数据
		template<typename T_FUN_B1>
		bool _Clear(T_FUN_B1 fun,iterator itBegin)
		{
			iterator it,old_it;
			long count=0;

			thelog<<"根据规则清理......"<<endi;
			it=itBegin;
			while(it!=end())
			{
				old_it=it;
				MoveNext(&it);
				if(fun(GetByHandle(old_it)))
				{
					GetISet(old_it.set_index)->isetErase(old_it.handle);
					++count;
				}
				else
				{
					break; //第一个不符合的跳出
				}
				if(0==count%100000)thelog<<"已清理记录:"<<count<<"条"<<endi;
			}
			thelog<<"清理完成，清理["<<count<<"]个。"<<endi;
			return true;
		}

		template<typename T_FUN_B1>
		bool _ForEach(T_FUN_B1 fun)
		{
			iterator it;
			long count_all=0;
			long count=0;

			thelog << "开始处理。。。" << endi;
			for(it=begin();it!=end();MoveNext(&it))
			{
				if(fun(&GetByHandle(it)))
				{
					++count;
				}
				++count_all;
				if(0==count_all%10000)thelog<<count_all<<" "<<count<<endi;
			}
			thelog << "处理完成，共[" << count_all << "]个，处理[" << count << "]个。" << endi;
			return true;
		}
		bool _ForEachShuffle(long iSet, long handle, typename ISet<T_DATA >::ISetForEach * fun)
		{
			typename vector<ISet<T_DATA > *>::iterator it;
			if (iSet >= 0 && static_cast<size_t>(iSet) < vISet.size())
			{
				thelog << "续传模式 " << iSet << " " << handle << endi;
				it = vISet.begin() + iSet;
			}
			else
			{
				thelog << "全量模式 " << iSet << " " << handle << endi;
				it = vISet.begin();
			}
			for (; it != vISet.end(); ++it)
			{
				fun->iSet = it - vISet.begin();
				if (iSet == fun->iSet)
				{
					if (!(*it)->isetForEachShuffle(handle, fun))return false;
				}
				else
				{
					if (!(*it)->isetForEachShuffle(-1, fun))return false;
				}
			}
			return true;
		}
	public:
		virtual char const * GetName()const{return m_shmset.GetName();}

		virtual bool ReportData()const
		{
			CHtmlDoc::CHtmlTable2 table;
			ReportHtmlTable(table, false, 0, 20);
			thelog << endl << table.MakeTextTable() << endi;
			return true;
		}
		T_SHM_SET_GROUP(char const * name,int version):
			m_shmset(name,version)
			,m_shmset1(name,version)
			,m_shmset2(name,version)
			,m_shmset3(name,version)
			,m_shmset4(name,version)
			,m_shmset5(name,version)
			,m_shmset6(name,version)
			,m_shmset7(name,version)
			,m_shmset8(name,version)
			,m_shmset9(name,version)
		{
			vISet.reserve(N_SUBTREE);
			vISet.push_back(&m_shmset);
			vISet.push_back(&m_shmset1);
			vISet.push_back(&m_shmset2);
			vISet.push_back(&m_shmset3);
			vISet.push_back(&m_shmset4);
			vISet.push_back(&m_shmset5);
			vISet.push_back(&m_shmset6);
			vISet.push_back(&m_shmset7);
			vISet.push_back(&m_shmset8);
			vISet.push_back(&m_shmset9);
			if(vISet.size()!=N_SUBTREE)throw "内存不足";
	
			if(!AddTable(&m_shmset))throw "内存不足";
			if(!AddTable(&m_shmset1))throw "内存不足";
			if(!AddTable(&m_shmset2))throw "内存不足";
			if(!AddTable(&m_shmset3))throw "内存不足";
			if(!AddTable(&m_shmset4))throw "内存不足";
			if(!AddTable(&m_shmset5))throw "内存不足";
			if(!AddTable(&m_shmset6))throw "内存不足";
			if(!AddTable(&m_shmset7))throw "内存不足";
			if(!AddTable(&m_shmset8))throw "内存不足";
			if(!AddTable(&m_shmset9))throw "内存不足";
		}
		//根据配置的大小创建共享内存，仅供管理员使用
		bool adminRatableCreateShm()
		{
			return CreateShm();
		}
		bool adminCreateShm()
		{
			return CreateShm();
		}
		//清理全部数据
		bool Destroy()
		{
			return clear();
		}
		//连接和断开，管理员和一般使用者使用
		bool init(bool isReadOnly)
		{
			if(!Attach(isReadOnly))
			{
				thelog<<"连接到共享内存失败"<<ende;
				return false;
			}
			return true;
		}
		bool uninit()
		{
			return Detach();
		}
		//遍历
		iterator begin()const
		{
			iterator it;
			it.set_index=0;
			it.handle=GetISet(it.set_index)->isetBegin();
			if(it.handle<0)MoveNext(&it);
			return it;
		}
		iterator end()const
		{
			iterator it;
			new(&it) iterator;
			return it;
		}
		T_DATA & GetByHandle(iterator const & it)const
		{
			return *GetISet(it.set_index)->isetGet(it.handle);
		}
		iterator & MoveNext(iterator * p)const
		{
			while(p->set_index<N_SUBTREE)
			{			
				//thelog<<"当前位置："<<p->set_index<<" "<<p->handle<<endi;
				//先++
				if(p->handle>=0)
				{
					GetISet(p->set_index)->isetMoveNext(p->handle);
					//thelog<<"++后位置："<<p->set_index<<" "<<p->handle<<endi;
				}
				//到了end
				if(p->handle<0)
				{
					//thelog<<"子树结尾，指向下一个子树数begin"<<endi;
					++(p->set_index);
					//thelog<<"新子树位置："<<p->set_index<<" "<<p->handle<<endi;
					if(p->set_index<N_SUBTREE)
					{
						p->handle=GetISet(p->set_index)->isetBegin();
						//thelog<<"新子树begin位置："<<p->set_index<<" "<<p->handle<<endi;
						if(p->handle>=0)
						{
							break;
						}
					}
					else
					{
						//thelog<<"到达所有的结尾"<<endi;
					}
				}
				else
				{
					break;
				}
			}
			//仍然是end
			if(p->handle<0)new(p) iterator;
			return *p;
		}
		//报告内容，调试用
		string & Report(string & str)const
		{
			string tmp;
			str="";
			//str+=m_shmset.Report(tmp);
			//str+=m_shmset1.Report(tmp);
			//str+=m_shmset2.Report(tmp);
			//str+=m_shmset3.Report(tmp);
			//str+=m_shmset4.Report(tmp);
			//str+=m_shmset5.Report(tmp);
			//str+=m_shmset6.Report(tmp);
			//str+=m_shmset7.Report(tmp);
			//str+=m_shmset8.Report(tmp);
			//str+=m_shmset9.Report(tmp);

			char buf[2048];
			sprintf(buf,"\n总容量 %ld 总大小 %ld (%ld%%)",capacity(),size(),size()*100/(0==capacity()?1:capacity()));
			str+=buf;

			iterator it;
			long count;
			for (it = begin(), count = 0; it != end(); MoveNext(&it), ++count)
			{
				if(count>100)
				{
					str+="\n......";
					break;
				}
				sprintf(buf, "\n%ld ", it.set_index);
				str += buf;
				str += GetByHandle(it).toString(tmp);
			}

			return str;
		}
		//输出数据到表格，formEnd倒序，start_pos起始位置，倒序时最后一个为0，返回输出的行数
		long ReportHtmlTable(CHtmlDoc::CHtmlTable2 & table,bool fromEnd,long start_pos,long max_count)const
		{
			table.Clear();
			table.AddCol("PART",CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("i",CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			T_DATA::AddTableColumns(table);

			const_iterator it;
			long i;
			long count=0;
			if(fromEnd)
			{
				table.AddLine();
				table.AddData("不支持倒序显示");
				return 0;
			}
			for (i = 0, it = begin(); it != end() && count < max_count; ++i, MoveNext(&it))
			{
				if(i<start_pos)continue;
				if(count>=max_count)break;
				++count;
				table.AddLine();
				table.AddData(it.set_index);
				table.AddData(i);
				GetByHandle(it).AddTableData(table);
			}
			return count;
		}
		//直接存取
		bool get(T_DATA * value)const{return get(*value);}
		bool get(T_DATA & value)const
		{
			return _get(value);
		}
		//直接查找
		iterator find(T_DATA const & value) const
		{
			return _find(value);
		}
		iterator lower_bound(T_DATA * value, bool(*less)(T_DATA const &, T_DATA const &))const{return lower_bound(*value,less);}
		iterator lower_bound(T_DATA & value, bool(*less)(T_DATA const &, T_DATA const &))const
		{
			return _find_lower_bound(value,less);
		}
		bool update(T_DATA const & value)
		{
			return _update(value);
		}
		bool erase(T_DATA const & value)
		{
			return _erase(value);
		}
		pair<iterator, bool> insert(T_DATA const & value)
		{
			return _insert(value);
		}
		template<typename T_FUN_B1>
		bool Clear(T_FUN_B1 fun)
		{
			return _Clear(fun);
		}
		template<typename T_FUN_B1>
		bool Clear(T_FUN_B1 fun,iterator itBegin)
		{
			return _Clear(fun,itBegin);
		}
		template<typename T_FUN_B1>
		bool ForEach(T_FUN_B1 fun)
		{
			return _ForEach(fun);
		}
		typedef typename ISet<T_DATA >::ISetForEach T_FOR_EACH_SHUFFLE;
		bool ForEachShuffle(long iSet, long handle, typename ISet<T_DATA >::ISetForEach * fun)
		{
			return _ForEachShuffle(iSet, handle, fun);
		}
	};
}
