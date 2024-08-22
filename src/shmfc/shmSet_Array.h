//shm_Set_Array.h 共享内存Set容器 不带互斥的基本版本的数组版本
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*
本文件定义二叉树模板的数组版本，用于实现较小的空间需求
*/

#include "shmstd.h"
#include "shmArray.h"

namespace ns_my_std
{
	template<typename T_DATA, int PI_N, typename T_USER_HEAD = CDemoData, int PART = 0, int VER = 0, typename T_COMP = less<T_DATA>, typename T_HANDLE = T_HANDLE_ARRAY<T_DATA , PI_N > >
	class T_SHMSET_ARRAY : public T_ARRAY<T_DATA, PI_N, T_USER_HEAD, PART, T_HANDLE >
	{
	public:
		typedef T_ARRAY<T_DATA, PI_N, T_USER_HEAD, PART, T_HANDLE > T_PARENT;
		using T_PARENT::m_FastRebuild_level;
		using T_PARENT::Size;
		using T_PARENT::Report;
		using T_PARENT::Add;
		using T_PARENT::Get;
		using T_PARENT::Begin;
		using T_PARENT::End;
		using T_PARENT::IsOneBlock;
		using T_PARENT::Sort_slow;
		using T_PARENT::Sort_fast;
		using T_PARENT::SetSize;
		class iterator
		{
		public:
			T_SHM_SIZE handle;
			iterator() :handle(-1) {}
			bool operator == (iterator const& tmp)const { return handle == tmp.handle; }
			bool operator != (iterator const& tmp)const { return !(*this == tmp); }
			T_DATA& operator * ()const
			{
				T_HANDLE h;
				h.handle = handle;
				return *h;
			}
			T_DATA* operator -> ()const
			{
				return &(operator *());
			}
			iterator& operator ++ ()
			{
				++handle;
				return *this;
			}
			iterator& operator -- ()
			{
				--handle;
				return *this;
			}
		};
		typedef iterator const_iterator;
	public:
		T_SHMSET_ARRAY(char const* name, int version) :T_PARENT(name, version) {}
		const_iterator begin() const
		{
			const_iterator it;
			it.handle = 0;
			return it;
		}
		const_iterator end() const 
		{
			const_iterator it;
			it.handle = Size();
			return it;
		}
		void SetFlagInsert(T_SHM_SIZE&) {}
		void SetFlagUpdate(T_SHM_SIZE&) {}

		//开始快速重建
		virtual bool FastRebuild_Start()
		{
			this->Detach();
			this->DestoryShm();
			this->CreateShm();
			return this->Attach(false);
		}
		//快速添加数据
		bool FastRebuild_PushData(T_DATA const& data)
		{
			T_HANDLE h;

			return Add(data, h);
		}
		//设置数据大小，树结构的大小还没有设置
		bool FastRebuild_SetSize(long new_size)
		{
			return SetSize(new_size);
		}
		//获得数据大小，树结构的大小还没有设置
		long FastRebuild_GetSize()
		{
			return Size();
		}
		//结束快速重建
		virtual bool FastRebuild_Finish(char const* level, bool noDisk)
		{
			if (level != NULL && level != m_FastRebuild_level)
			{
				thelog << "level不符合，忽略" << endi;
				return true;
			}

			string str;
			thelog << "数据总数 " << Size() << endi;
			thelog << endl << Report(str, true) << endi;

			T_COMP comp;
			bool bNeedSort = false;
			thelog << "检查是否需要排序。。。。。。" << endi;
			for (long i = 1; i < Size(); ++i)
			{
				if (comp(*Get(i) , *Get(i-1)))
				{
					bNeedSort = true;
					thelog << "需要排序" << endi;
					break;
				}
			}
			if (bNeedSort)
			{
				if (!IsOneBlock() && !noDisk)
				{
					thelog << "非单一块，排序之前必须先搞成单一块" << endi;
					this->SaveToDir("./");
					this->Detach();
					this->DestoryShm();
					this->LoadFromDir("./");
				}
				thelog << "排序。。。。。。" << endi;
				bool slow = (!IsOneBlock());
				if (slow)
				{
					Sort_slow(comp);
					thelog << "慢速排序完成" << endi;
				}
				else
				{
					Sort_fast(comp);
					thelog << "快速排序完成" << endi;
				}
				thelog << endl << Report(str, true) << endi;
			}
			else
			{
				thelog << "不需要排序" << endi;
			}
			
			thelog << "检查是否存在重复数据。。。。。。" << endi;
			long newsize = 0;
			for (long i = 0; i < Size(); ++i)
			{
				if (0 == i)++newsize;
				else
				{
					T_DATA& data_top = *Get(newsize - 1);
					T_DATA& data_cur = *Get(i);
					if (!(comp(data_top, data_cur) || comp(data_cur, data_top)))
					{
						data_top = data_cur;//相同，后面的覆盖前面的
						continue;
					}
					if (i != newsize)
					{
						*Get(newsize) = data_cur;//往前移
					}
					++newsize;
				}
			}
			thelog << "发现 " << Size() - newsize << " 个重复数据，已经用后面的覆盖前面的" << endi;
			SetSize(newsize);

			thelog << endl << Report(str, true) << endi;
			thelog << "最终数据总数 " << Size() << endi;
			return true;
		}
		virtual bool repair(char const* level)
		{
			return FastRebuild_Finish(level, true);
		}
		
		//如果second为false则已经存在，发生了覆盖，用GetOldValue获得被覆盖的值
		pair<iterator, bool> insert(T_DATA const& data)
		{
			T_COMP comp;
			return insert(data, comp);
		}
		pair<iterator, bool> insert(T_DATA const& data, T_COMP& comp)
		{
			pair<iterator, bool> ret;

			ret.first = end();
			ret.second = false;

			return ret;
		}
		bool erase(const_iterator it)
		{
			return false;
		}
		bool erase(T_DATA const &)
		{
			return false;
		}
		template<typename T_FIND >
		const_iterator find(T_FIND const & tmp)const
		{
			T_COMP comp;
			return lower_bound(tmp, comp);
		}
		const_iterator find(T_DATA const & tmp, T_COMP & comp)const
		{
			const_iterator it = lower_bound(tmp, comp);
			if (it != end())
			{
				if (comp(*it, tmp) || comp(tmp, *it))return end();
			}
			return it;
		}
		//用部分比较函数（但必须是符合顺序的，否则结果不可预期）
		template<typename T_FIND, typename T_LESS_BOUND >
		const_iterator lower_bound(T_FIND const & tmp, T_LESS_BOUND comp)const
		{
			T_HANDLE h = ::lower_bound(Begin(), End(), tmp, comp);
			if (h != End())
			{
				if (comp(*h, tmp) || comp(tmp, *h))return end();
			}
			const_iterator it;
			it.handle = h.handle;
			return it;
		}
		//用部分比较函数（但必须是符合顺序的，否则结果不可预期）
		template<typename T_FIND, typename T_LESS_BOUND >
		const_iterator upper_bound(T_FIND const & tmp, T_LESS_BOUND comp)const
		{
			T_HANDLE h = ::upper_bound(Begin(), End(), tmp, comp);
			if (h != End())
			{
				if (comp(*h, tmp) || comp(tmp, *h))return end();
			}
			const_iterator it;
			it.handle = h.handle;
			return it;
		}
	public:
	};

}
