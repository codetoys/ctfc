//shm_MultiListTree.h 共享内存ListTree容器
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*
本文件定义共享内存ListTree接口
*/

#include "shmstd.h"
#include "shmArray.h"

namespace ns_my_std
{
	template<typename T_DATA >
	struct T_LISTTREE_NODE_STRUCT
	{
		T_SHM_SIZE hNext;//-1:无；0-N,指向下个地址
		T_SHM_SIZE hChild;//-1:无；0-N,指向子数据
		T_DATA data;
		
		T_LISTTREE_NODE_STRUCT():hNext(-1), hChild(-1){}
		string & toString(string & str)const
		{
			char buf[2048];
			string tmp;
			sprintf(buf, "%8ld %8ld: %s", hNext, hChild, data.toString(tmp).c_str());
			return str=buf;
		}
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{
			table.AddCol("NEXT", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("CHILD", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("|");
			return T_DATA::AddTableColumns(table);
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddData(hNext);
			table.AddData(hChild);
			table.AddData("|");
			return data.AddTableData(table);
		}
	};
	template<typename T_DATA,int PI_N,typename T_HANDLE >
	struct T_LISTTREE_NODE : public T_LISTTREE_NODE_STRUCT<T_DATA >
	{
		static T_LISTTREE_NODE & at(T_SHM_SIZE n)
		{
			if(n<0)
			{
				G_SET_ERROR(My_ERR_INVALID_HANDLE);
				thelog<<"at error "<<n<<ende;
			}
			T_HANDLE array_handle(n);
			//char buf[256];
			//sprintf(buf,"%ld %p",n,&*array_handle);
			//theLog<<buf<<endi;
			return *(T_LISTTREE_NODE *)(void *)&*array_handle;
		}
		T_SHM_SIZE _me()const
		{
			return T_HANDLE::_me(this);
		}
	};
	template<typename T_USER_HEAD>
	struct T_LISTTREE_HEAD
	{
		T_SHM_SIZE free_size;//节点个数
		T_SHM_SIZE free_head;//空闲地址头指针

		T_USER_HEAD user_head;//用户的特殊数据

		T_LISTTREE_HEAD() :free_size(0), free_head(-1){}
		
		string & toString(string & str)const
		{
			char buf[2048];
			string tmp;
			sprintf(buf,"free_size=%ld free_head=%ld ",free_size,free_head);
			str = buf + user_head.toString(tmp);
			return str;
		}
	};

	template<typename T_DATA,int PI_N,typename T_USER_HEAD = CDemoData,int PART=0,typename T_HANDLE=T_HANDLE_ARRAY<T_LISTTREE_NODE_STRUCT<T_DATA>,PI_N > >
	class T_MULTI_LISTTREE : public T_ARRAY<T_LISTTREE_NODE_STRUCT<T_DATA>, PI_N, T_LISTTREE_HEAD<T_USER_HEAD>, PART, T_HANDLE >
	{
	private:
		typedef T_ARRAY<T_LISTTREE_NODE_STRUCT<T_DATA>, PI_N, T_LISTTREE_HEAD<T_USER_HEAD>, PART, T_HANDLE > T_PARENT;
	public:
		typedef T_LISTTREE_HEAD<T_USER_HEAD> LIST_HEAD;
		typedef T_LISTTREE_NODE<T_DATA,PI_N,T_HANDLE > LIST_NODE;
		using T_PARENT::Add;
		struct iterator
		{
			T_SHM_SIZE handle;

			iterator():handle(-1){}
			bool operator == (iterator const & tmp)const{return handle==tmp.handle;}
			bool operator != (iterator const & tmp)const{return !(*this==tmp);}
			T_DATA & operator * ()const
			{
				return LIST_NODE::at(handle).data;
			}
			T_DATA * operator -> ()const
			{
				return &(operator *());
			}
			iterator & operator ++ ()
			{
				handle=LIST_NODE::at(handle).hNext;
				return *this;
			}
			bool isEnd()const { return -1 == handle; }
			bool hasNext()const { return -1 != LIST_NODE::at(handle).hNext; }
			long getNext()const { return LIST_NODE::at(handle).hNext; }
			void setNext(T_SHM_SIZE h)const { LIST_NODE::at(handle).hNext = h; }
			void setChild(T_SHM_SIZE h)const { LIST_NODE::at(handle).hChild = h; }
			long getChild()const { return LIST_NODE::at(handle).hChild; }
		};
		typedef iterator const_iterator;
	public:
		T_MULTI_LISTTREE(char const * name, int version) :T_PARENT(name, version){}
		T_USER_HEAD * GetUserHead(){return &T_PARENT::GetUserHead()->user_head;}
		LIST_HEAD * GetListHead()const{return T_PARENT::GetUserHead();}
		bool Reserve(T_SHM_SIZE _n){ return T_PARENT::Reserve(_n); }
		//查找一个项，没找到返回false，h返回找到的位置或列表最后一个节点
		bool FindInList(iterator const & head, T_DATA const & data, iterator & it)const
		{
			it = head;
			while (!it.isEnd())
			{
				if (!(data < *it) && !(*it < data))return true;
				if (!it.hasNext())break;
				++it;
			}
			return false;
		}
		//添加data到head，如果head是isEnd则添加到新列表并放置位置在h，如果已经存在则覆盖，如果head是isEnd则会被修改
		bool AddList(iterator & head, T_DATA const & data, iterator & it)
		{
			if (FindInList(head, data, it))
			{
				*it = data;
				return true;
			}
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
			//仅当head是isEnd此时it才会是isEnd
			if (it.isEnd())
			{
				it.handle = h.handle;
				head.handle = h.handle;
			}
			else
			{
				it.setNext(h.handle);
				++it;
			}
			return true;
		}
		//添加data到head，如果head是isEnd则添加到新列表并放置位置在h，如果已经存在则覆盖，如果head是isEnd则会被修改
		bool AddChild(iterator const & node, T_DATA const & data, iterator & it)
		{
			if (node.isEnd())return false;
			iterator head;
			head.handle = node.getChild();
			if (AddList(head, data, it))
			{
				node.setChild(head.handle);
				return true;
			}
			return false;
		}
		//添加data到head的尾部，如果head是isEnd则添加到新列表并放置位置在head，不检查是否存在
		bool AddTail(iterator & head,T_DATA const & data)
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
			if(head.isEnd())
			{
				head.handle=h.handle;
			}
			else
			{
				T_HANDLE tmph;
				tmph.handle=head.handle;
				while(-1!=tmph->hNext)
				{
					tmph=tmph->hNext;
				}
				tmph->hNext=h.handle;
			}
			return true;
		}
		//删除子项
		bool DeleteChild(iterator const & node)
		{
			if (node.isEnd())return true;

			iterator it;
			it.handle = node.getChild();
			if (it.isEnd())return true;
			if (DeleteList(it))
			{
				node.setChild(-1);
				return true;
			}
			return false;
		}
		//删除链表
		bool DeleteList(iterator const & head)
		{
			if (head.isEnd())return true;
			T_HANDLE tmph;
			long count = 0;
			tmph.handle = head.handle;
			while(-1!=tmph.handle)
			{
				++count;
				iterator node;
				node.handle = tmph.handle;
				DeleteChild(node);
				if (-1 == tmph->hNext)break;
				tmph = tmph->hNext;
			};
			tmph->hNext = GetListHead()->free_head;
			GetListHead()->free_head = head.handle;
			GetListHead()->free_size += count;
			return true;
		}
		//删除链表的一个项
		bool DeleteListOne(iterator & head,iterator const & toDelete)
		{
			//thelog<<head.handle<<" "<<toDelete.handle<<endi;
			if (head.isEnd())
			{
				thelog<<"无效的头"<<ende;
				return false;
			}
			T_HANDLE tmph,tmpDelete;
			tmph.handle = head.handle;
			tmpDelete.handle = toDelete.handle;
			if (toDelete == head)
			{
				DeleteChild(toDelete);
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
					if (tmpDelete.handle == tmph->hNext)
					{
						DeleteChild(toDelete);
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
			thelog<<"没找到"<<ende;
			return false;
		}
		string & _ReportList(long h, string & ret, long level)const
		{
			string px;
			long i;
			for (i = 0; i < level; ++i)px += "    ";

			iterator it;
			it.handle = h;
			while (!it.isEnd())
			{
				string str;
				ret += px + it->toString(str) + "\n";
				_ReportList(it.getChild(), ret, level + 1);
				++it;
			}
			return ret;
		}
		string & ReportList(long h, string & ret)const
		{
			return _ReportList(h, ret, 0);
		}
	public:
		IShmActiveObject * getIShmActiveObject(){ return this; }
		//IShmActiveObject接口
		virtual bool Report()const
		{
			string str;
			thelog << endl << T_PARENT::Report(str, true) << endi;
			return true;
		}
	};

}
