//shm_MultiList.h 共享内存List容器
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*
本文件定义共享内存List接口
*/

#include "shmstd.h"
#include "shmArray.h"

namespace ns_my_std
{
	template<typename T_DATA >
	struct T_LIST_NODE_STRUCT
	{
		T_SHM_SIZE hNext;//-1:无；0-N,指向下个地址
		T_DATA data;

		T_LIST_NODE_STRUCT() :hNext(-1) {}
		string& toString(string& str)const
		{
			char buf[2048];
			string tmp;
			sprintf(buf, "%8ld : %s", hNext, data.toString(tmp).c_str());
			return str = buf;
		}
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("NEXT", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("|");
			return T_DATA::AddTableColumns(table);
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(hNext);
			table.AddData("|");
			return data.AddTableData(table);
		}
	};
	template<typename T_DATA, int PI_N, typename T_HANDLE >
	struct T_LIST_NODE : public T_LIST_NODE_STRUCT<T_DATA >
	{
		static T_LIST_NODE& at(T_SHM_SIZE n)
		{
			if (n < 0)
			{
				G_SET_ERROR(My_ERR_INVALID_HANDLE);
				thelog << "at error " << n << ende;
			}
			T_HANDLE array_handle(n);
			//char buf[256];
			//sprintf(buf,"%ld %p",n,&*array_handle);
			//theLog<<buf<<endi;
			return *(T_LIST_NODE*)(void*)&*array_handle;
		}
		T_SHM_SIZE _me()const
		{
			return T_HANDLE::_me(this);
		}
	};
	template<typename T_USER_HEAD>
	struct T_LIST_HEAD
	{
		T_SHM_SIZE free_size;//节点个数
		T_SHM_SIZE free_head;//空闲地址头指针

		T_USER_HEAD user_head;//用户的特殊数据

		T_LIST_HEAD() :free_size(0), free_head(-1) {}

		string& toString(string& str)const
		{
			char buf[2048];
			string tmp;
			sprintf(buf, "free_size=%ld free_head=%ld ", free_size, free_head);
			str = buf + user_head.toString(tmp);
			return str;
		}
	};

	template<typename T_DATA, int PI_N, typename T_USER_HEAD = CDemoData, int PART = 0, typename T_COMP = less<T_DATA>, typename T_HANDLE = T_HANDLE_ARRAY<T_LIST_NODE_STRUCT<T_DATA>, PI_N > >
	class T_MULTI_LIST : private T_ARRAY<T_LIST_NODE_STRUCT<T_DATA>, PI_N, T_LIST_HEAD<T_USER_HEAD>, PART, T_HANDLE >
	{
	private:
		typedef T_ARRAY<T_LIST_NODE_STRUCT<T_DATA>, PI_N, T_LIST_HEAD<T_USER_HEAD>, PART, T_HANDLE > T_PARENT;
	public:
		typedef T_LIST_HEAD<T_USER_HEAD> LIST_HEAD;
		typedef T_LIST_NODE<T_DATA, PI_N, T_HANDLE > LIST_NODE;
		using T_PARENT::Add;
		using T_PARENT::capacity;
		using T_PARENT::size;
		using T_PARENT::m_FastRebuild_level;
		using T_PARENT::Get;
		using T_PARENT::Begin;
		using T_PARENT::End;
		using T_PARENT::IsOneBlock;
		using T_PARENT::Sort_slow;
		using T_PARENT::Sort_fast;
		using T_PARENT::SetSize;
		using T_PARENT::Size;
		using T_PARENT::Report;

		struct iterator
		{
			T_SHM_SIZE handle;

			iterator() :handle(-1) {}
			bool operator == (iterator const& tmp)const { return handle == tmp.handle; }
			bool operator != (iterator const& tmp)const { return !(*this == tmp); }
			T_DATA& operator * ()const
			{
				return LIST_NODE::at(handle).data;
			}
			T_DATA* operator -> ()const
			{
				return &(operator *());
			}
			iterator& operator ++ ()
			{
				handle = LIST_NODE::at(handle).hNext;
				return *this;
			}
			bool isEnd()const { return -1 == handle; }

			friend ostream& operator << (ostream& o, iterator const& d)
			{
				return o << d.handle;
			}
		};
		typedef iterator const_iterator;
	public:
		T_MULTI_LIST(char const* name, int version) :T_PARENT(name, version) {}
		T_USER_HEAD* GetUserHead() { return &T_PARENT::GetUserHead()->user_head; }
		LIST_HEAD* GetListHead()const { return T_PARENT::GetUserHead(); }
		bool Reserve(T_SHM_SIZE _n) { return T_PARENT::Reserve(_n); }
		//添加data到head的尾部，如果head是isEnd则添加到新列表并放置位置在head
		bool AddTail(iterator& head, T_DATA const& data)
		{
			T_HANDLE h;
			if (-1 != GetListHead()->free_head)
			{
				h = GetListHead()->free_head;
				GetListHead()->free_head = h->hNext;
				--GetListHead()->free_size;

				h->data = data;
				h->hNext = -1;
			}
			else
			{
				LIST_NODE tmp;
				tmp.data = data;
				if (!Add(tmp, h))
				{
					thelog << "mutlilist add error" << ende;
					return false;
				}
			}
			if (head.isEnd())
			{
				head.handle = h.handle;
			}
			else
			{
				T_HANDLE tmph;
				tmph.handle = head.handle;
				while (-1 != tmph->hNext)
				{
					tmph = tmph->hNext;
				}
				tmph->hNext = h.handle;
			}
			return true;
		}
		//添加data到head的头部，如果head是isEnd则添加到新列表并放置位置在head
		bool AddHead(iterator& head, T_DATA const& data)
		{
			T_HANDLE h;
			if (-1 != GetListHead()->free_head)
			{
				h = GetListHead()->free_head;
				GetListHead()->free_head = h->hNext;
				--GetListHead()->free_size;

				h->data = data;
				h->hNext = -1;
			}
			else
			{
				LIST_NODE tmp;
				tmp.data = data;
				if (!Add(tmp, h))
				{
					thelog << "mutlilist add error" << ende;
					return false;
				}
			}
			if (head.isEnd())
			{
				head.handle = h.handle;
			}
			else
			{
				h->hNext = head.handle;
				head.handle = h.handle;
			}
			return true;
		}
		//删除链表
		bool DeleteList(iterator const& head)
		{
			if (head.isEnd())return true;
			T_HANDLE tmph;
			long count = 0;
			tmph.handle = head.handle;
			++count;
			while (-1 != tmph->hNext)
			{
				tmph = tmph->hNext;
				++count;
			}
			tmph->hNext = GetListHead()->free_head;
			GetListHead()->free_head = head.handle;
			GetListHead()->free_size += count;
			return true;
		}
		//删除链表的一个项
		bool DeleteListOne(iterator& head, iterator const& toDelete)
		{
			//thelog<<head.handle<<" "<<toDelete.handle<<endi;
			if (head.isEnd())
			{
				thelog << "无效的头" << ende;
				return false;
			}
			T_HANDLE tmph, tmpDelete;
			tmph.handle = head.handle;
			tmpDelete.handle = toDelete.handle;
			if (toDelete == head)
			{
				//thelog<<"删除头"<<endi;
				head.handle = tmpDelete->hNext;//修改头
				tmpDelete->hNext = GetListHead()->free_head;//放在空闲头处
				GetListHead()->free_head = tmpDelete.handle;
				++GetListHead()->free_size;
				return true;
			}
			else
			{
				while (-1 != tmph->hNext)
				{
					//thelog << tmpDelete.handle << " " << tmph->hNext << endi;
					if (tmpDelete.handle == tmph->hNext)
					{
						//thelog << "删除非头" << endi;
						tmph->hNext = tmpDelete->hNext;//卸下要删除的节点
						tmpDelete->hNext = GetListHead()->free_head;//放在空闲头处
						GetListHead()->free_head = tmpDelete.handle;
						++GetListHead()->free_size;
						return true;
					}
					else
					{
						tmph = tmph->hNext;
					}
				}
			}
			thelog << "没找到" << ende;
			return false;
		}
	public:
		IShmActiveObject* getIShmActiveObject() { return this; }
		//IShmActiveObject接口
		virtual bool Report()const
		{
			string str;
			thelog << endl << T_PARENT::Report(str, true) << endi;
			return true;
		}
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
			iterator it;
			return AddTail(it, data);
		}
		//获得数据大小，树结构的大小还没有设置
		long FastRebuild_GetSize()
		{
			return size();
		}
		struct multilist_comp
		{
			bool operator()(T_LIST_NODE_STRUCT<T_DATA> const& a, T_LIST_NODE_STRUCT<T_DATA> const& b)const
			{
				T_COMP comp;
				return comp(a.data, b.data);
			}
		};
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
				if (comp(Get(i)->data, Get(i - 1)->data))
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
				multilist_comp ml_comp;
				if (slow)
				{
					Sort_slow(ml_comp);
					thelog << "慢速排序完成" << endi;
				}
				else
				{
					Sort_fast(ml_comp);
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
					T_DATA& data_top = Get(newsize - 1)->data;
					T_DATA& data_cur = Get(i)->data;
					if (!(comp(data_top, data_cur) || comp(data_cur, data_top)))
					{
						data_top = data_cur;//相同，后面的覆盖前面的
						continue;
					}
					if (i != newsize)
					{
						Get(newsize)->data = data_cur;//往前移
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
			thelog << "T_MULTI_LIST无需处理" << endi;
			return true;
		}
	};

}
