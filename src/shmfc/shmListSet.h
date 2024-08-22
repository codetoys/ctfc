//shm_ListSet.h 共享内存ListSet容器 带有列表的set
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmSet_NoMutex.h"
#include "shmMultiList.h"

namespace ns_my_std
{
	//set接口，用于分为N个子树的set
	template<typename T_DATA, typename T_DATA_SECOND>
	class IListSet
	{
	public:
		//IListSet遍历接口
		struct IListSetForEach
		{
			virtual bool doOneSub(T_DATA * pData,T_DATA_SECOND * pSecond)=0;
		};
		//IListSet遍历接口
		struct IListSetForEachShuffle
		{
			long iSet;//非group对象为-1
			IListSetForEachShuffle():iSet(-1){}
			virtual bool doOne(long handle, T_DATA const * pData, vector<T_DATA_SECOND> & Seconds) { return false; }
		};

		//IListSet遍历子数据接口
		struct IListSetForEachSub
		{
			virtual bool doOneSub(T_DATA_SECOND * pSecond)=0;
		};
		//清理控制接口
		struct IlsgClear
		{
			//是否需要删除主数据和列表（全部删除）
			virtual bool isMainDataDeleteAll(T_DATA const &data)
			{
				string str;
				//thelog << data.toString(str) << endi;
				return false;
			}
			//是否需要跳过主数据和列表（全部保留）
			virtual bool isMainDataSkip(T_DATA const &data)
			{
				string str;
				//thelog << data.toString(str) << endi;
				return false;
			}
			//是否需要删除一条列表
			virtual bool isListDataDelete(T_DATA const &data, T_DATA_SECOND const & listdata)
			{
				string str;
				thelog << listdata.toString(str) << endi;
				return UIInput("删除？（y/n）","n")=="y";
			}
		};
	public:
		virtual T_DATA * isetGet(long h)const
		{
			thelog<<" ISet::isetGet 未实现"<<ende;
			return NULL;
		}
		virtual long isetMoveNext(long & h)const
		{
			thelog<<" ISet::isetMoveNext 未实现"<<ende;
			return -1;
		}
		virtual long isetBegin()const
		{
			thelog<<" ISet::isetBegin 未实现"<<ende;
			return -1;
		}
		virtual pair<long,bool> isetInsert(T_DATA const & value)
		{
			pair<long,bool> tmp;
			tmp.first=-1;
			tmp.second=false;
			thelog<<" ISet::isetInsert 未实现"<<ende;
			return tmp;
		}
		virtual long isetFind(T_DATA const & value)const
		{
			thelog<<" ISet::isetFind 未实现"<<ende;
			return -1;
		}
		virtual bool isetErase(long h)
		{
			thelog<<" ISet::isetErase 未实现"<<ende;
			return -1;
		}
		virtual bool ilistsetSubAdd(long h,T_DATA_SECOND const & sub)
		{
			thelog<<" IListSet::ilistsetSubAdd 未实现"<<ende;
			return false;
		}
		virtual T_DATA_SECOND * ilistsetSubGet(long h,T_DATA_SECOND const & sub)
		{
			thelog<<" IListSet::ilistsetSubGet 未实现"<<ende;
			return NULL;
		}
		virtual bool ilistsetSubGetAll(long h,vector<T_DATA_SECOND >  & subs)const
		{
			thelog<<" IListSet::ilistsetSubGetAll 未实现"<<ende;
			return true;
		}
		virtual bool ilistsetSubGetSetAll(long h,set<T_DATA_SECOND >  & subs)const
		{
			thelog<<" IListSet::ilistsetSubGetSetAll 未实现"<<ende;
			return true;
		}
		virtual bool ilistsetForEach(IListSetForEach * pForEach)const
		{
			thelog<<" IListSet::ilistsetForEach 未实现"<<ende;
			return true;
		}
		virtual bool ilistsetForEachSub(long h,IListSetForEachSub * pForEachSub)const
		{
			thelog<<" IListSet::ilistsetForEachSub 未实现"<<ende;
			return true;
		}
		virtual bool ilistsetForEachShuffle(long handle, IListSetForEachShuffle * pForEachShiffle)const
		{
			thelog<<" IListSet::ilistsetForEachShuffle 未实现"<<ende;
			return true;
		}
		virtual bool ilistsetClear(IlsgClear * fun, long & count_main, long & count_second)
		{
			thelog << " IListSet::ilistsetClear 未实现" << ende;
			return true;
		}
	};

	template <typename T_DATA_FIRST, int PI_N, typename T_DATA_SECOND, int PI_N2, int PART = 0 >
	class T_SHM_LIST_SET : public CShmActiveObjects, public IListSet<T_DATA_FIRST, T_DATA_SECOND >
	{
	private:
		//second容器类型
		typedef T_MULTI_LIST<T_DATA_SECOND, PI_N2, CDemoData, PART > T_SECOND_S;
		//first节点，增加了指针
		struct first_node
		{
			T_DATA_FIRST m_data;
			typename T_SECOND_S::iterator m_handle;
			long child_count;

			first_node():child_count(0){}
			//用于需要排序的场合
			bool operator < (first_node const & tmp)const { return m_data < tmp.m_data; }

			//用于输出数据的场合
			string & toString(string & str)const
			{
				string tmp;
				m_data.toString(tmp);
				char buf[2048];
				sprintf(buf, "%s %ld", tmp.c_str(), m_handle.handle);
				return str = buf;
			}
			//用于表格输出
			static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
			{
				if(T_DATA_FIRST::AddTableColumns(table))
				{
					table.AddCol("H", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					table.AddCol("N", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					return true;
				}
				else
				{
					return false;
				}
			}
			bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
			{
				m_data.AddTableData(table);
				table.AddData(m_handle.handle);
				table.AddData(child_count);
				return true;
			}
		};
		//用于set的比较对象
		struct less_first_node
		{
			bool operator()(first_node const & a,first_node const & b)const
			{
				return a<b;
			}
			bool operator()(first_node const & a,T_DATA_FIRST const & b)const
			{
				return a.m_data<b;
			}
			bool operator()(T_DATA_FIRST const & a,first_node const & b)const
			{
				return a<b.m_data;
			}
		};
		//first容器类型
		typedef T_SHMSET_NO_MUTEX_ISET<first_node, PI_N, CDemoData, PART, 0, T_HANDLE_ARRAY<T_TREE_NODE_STRUCT<first_node >, PI_N >, less_first_node > T_FIRST_S;
		T_SECOND_S m_seconds;
		T_FIRST_S m_firsts;
	public:
		typedef typename T_FIRST_S::iterator iterator;
		typedef typename T_FIRST_S::const_iterator const_iterator;
		virtual char const * GetName()const{return m_firsts.GetName();}
		virtual bool ReportData()const
		{
			CHtmlDoc::CHtmlTable2 table;
			ReportHtmlTable(table, false, 0, 20);
			thelog << endl << table.MakeTextTable() << endi;
			return true;
		}
		T_SHM_LIST_SET(char const * name,char const * name2,int version):
			m_seconds(name2,version),m_firsts(name,version)
		{
			if(!AddTable(&m_firsts))throw "内存不足";
			if(!AddTable(m_seconds.getIShmActiveObject()))throw "内存不足";
		}
		//输出数据到表格，formEnd倒序，start_pos起始位置，倒序时最后一个为0，返回输出的行数
		long ReportHtmlTable(CHtmlDoc::CHtmlTable2 & table,bool fromEnd,long start_pos,long max_count)const
		{
			table.Clear();
			table.AddCol("i",CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			first_node::AddTableColumns(table);

			const_iterator it;
			long i;
			long count=0;
			if(fromEnd)
			{
				table.AddLine();
				table.AddData("不支持倒序显示");
				return 0;
			}
			for (i = 0, it = m_firsts.begin(); it != m_firsts.end() && count < max_count; ++i, ++it)
			{
				if(i<start_pos)continue;
				if(count>=max_count)break;
				++count;
				table.AddLine();
				table.AddData(i);
				it->AddTableData(table);
			}
			return count;
		}
		iterator begin(){return m_firsts.begin();}
		const_iterator begin()const{return m_firsts.begin();}
		iterator end(){return m_firsts.end();}
		const_iterator end()const{return m_firsts.end();}
		iterator find(T_DATA_FIRST const & first)const
		{
			return m_firsts.find(first);
		}
		pair<iterator, bool> insert(T_DATA_FIRST const & first)
		{
			first_node tmp;
			tmp.m_data=first;
			return m_firsts.insert(tmp);
		}
	public:
		virtual T_DATA_FIRST * isetGet(long h)const
		{
			iterator it;
			it.handle=h;
			return &(it->m_data);
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
		virtual pair<long,bool> isetInsert(T_DATA_FIRST const & value)
		{
			pair<long,bool> ret;
			pair<iterator, bool> tmppair=insert(value);
			ret.first=tmppair.first.handle;
			ret.second=tmppair.second;
			return ret;
		}
		virtual long isetFind(T_DATA_FIRST const & value)const
		{
			return find(value).handle;
		}
		virtual bool isetErase(long h)
		{
			iterator it;
			it.handle=h;
			m_seconds.DeleteList(it->m_handle);
			return m_firsts.erase(it);
		}
		virtual bool ilistsetSubAdd(long h,T_DATA_SECOND const & sub)
		{
			iterator it;
			it.handle=h;
			if(end()==it)return false;
			if(m_seconds.AddTail(it->m_handle,sub))
			{
				++it->child_count;
				return true;
			}
			else
			{
				return false;
			}
		}
		virtual T_DATA_SECOND * ilistsetSubGet(long h,T_DATA_SECOND const & sub)
		{
			iterator it;
			it.handle=h;
			if(end()==it)return NULL;
			typename T_SECOND_S::iterator sub_it;
			sub_it=it->m_handle;
			for(;!sub_it.isEnd();++sub_it)
			{
				if(sub==*sub_it)
				{
					return &*sub_it;
				}
			}
			return NULL;
		}
		virtual bool ilistsetSubGetAll(long h,vector<T_DATA_SECOND >  & subs)const
		{
			subs.clear();
			iterator it;
			it.handle=h;
			if(end()==it)return false;
			typename T_SECOND_S::iterator sub_it;
			sub_it=it->m_handle;
			for(;!sub_it.isEnd();++sub_it)
			{
				subs.push_back(*sub_it);
			}
			return true;
		}
		virtual bool ilistsetSubGetSetAll(long h,set<T_DATA_SECOND >  & subs)const
		{
			subs.clear();
			iterator it;
			it.handle=h;
			if(end()==it)return false;
			typename T_SECOND_S::iterator sub_it;
			sub_it=it->m_handle;
			for(;!sub_it.isEnd();++sub_it)
			{
				subs.insert(*sub_it);
			}
			return true;
		}
		virtual bool ilistsetForEach(typename IListSet<T_DATA_FIRST, T_DATA_SECOND >::IListSetForEach * pForEach)const
		{
			iterator it;
			for(it=begin();it!=end();++it)
			{
				typename T_SECOND_S::iterator sub_it;
				sub_it = it->m_handle;
				for (; !sub_it.isEnd(); ++sub_it)
				{
					if(!pForEach->doOneSub(&it->m_data,&*sub_it))
					{
						return false;
					}
				}
			}
			return true;
		}
		struct funForEachShuffle :public ISet<first_node >::ISetForEach
		{
			typename IListSet<T_DATA_FIRST, T_DATA_SECOND >::IListSetForEachShuffle * pForEach;
			virtual bool doOneData(long handle, first_node const * pData)
			{
				vector<T_DATA_SECOND> seconds;
				typename T_SECOND_S::iterator sub_it;
				sub_it = pData->m_handle;
				for (; !sub_it.isEnd(); ++sub_it)
				{
					seconds.push_back(*sub_it);
				}

				if (!pForEach->doOne(handle, &pData->m_data, seconds))
				{
					return false;
				}

				return true;
			}
		};
		virtual bool ilistsetForEachShuffle(long handle, typename IListSet<T_DATA_FIRST, T_DATA_SECOND >::IListSetForEachShuffle * pForEachShiffle)const
		{
			funForEachShuffle fun;
			fun.pForEach=pForEachShiffle;

			return m_firsts.isetForEachShuffle(handle, &fun);
		}

		virtual bool ilistsetForEachSub(long h,typename IListSet<T_DATA_FIRST, T_DATA_SECOND >::IListSetForEachSub * pForEachSub)const
		{
			iterator it;
			it.handle=h;
			if(end()==it)return false;
			typename T_SECOND_S::iterator sub_it;
			sub_it=it->m_handle;
			for(;!sub_it.isEnd();++sub_it)
			{
				if(!pForEachSub->doOneSub(&*sub_it))
				{
					return false;
				}
			}
			return true;
		}

		virtual bool ilistsetClear(typename IListSet<T_DATA_FIRST, T_DATA_SECOND >::IlsgClear * fun, long & count_main, long & count_second)
		{
			iterator it, old_it;

			it = begin();
			long count_all = 0;
			while (it != end())
			{
				old_it = it;
				++it;
				thelog<< old_it->child_count<<endi;
				if (fun->isMainDataSkip(old_it->m_data))
				{
					continue;
				}
				if (fun->isMainDataDeleteAll(old_it->m_data))
				{
					++count_main;
					count_second+=old_it->child_count;
					isetErase(old_it.handle);
				}
				else
				{
					typename T_SECOND_S::iterator sub_it;
					typename T_SECOND_S::iterator old_sub_it;
					sub_it = old_it->m_handle;
					while (!sub_it.isEnd())
					{
						old_sub_it=sub_it;
						++sub_it;
						if (fun->isListDataDelete(old_it->m_data, *old_sub_it))
						{
							thelog << "需要删除 " << old_it.handle << " " << old_sub_it.handle << endi;
							if (!m_seconds.DeleteListOne(old_it->m_handle, old_sub_it))
							{
								thelog << "DeleteListOne出错" << ende;
							}
							else
							{
								++count_second;
							}
						}
					}
				}
				++count_all;
				if (0 == count_all % 10000)thelog << count_all << " " << count_main << " " << count_second<< endi;
			}
			thelog << "清理完成 " << count_main << " " << count_second << endi;
			//m_seconds.Report();
			return true;
		}
		//编译测试
		void test()
		{
			T_DATA_FIRST a;
			T_DATA_SECOND b;
			long n=-1;
			isetGet(0);
			isetMoveNext(n);
			isetBegin();
			isetInsert(a);
			isetFind(a);
			isetErase(n);
			ilistsetSubGet(n,b);
			vector<T_DATA_SECOND > c;
			ilistsetSubGetAll(n,c);
			typename IListSet<T_DATA_FIRST, T_DATA_SECOND>::IListSetForEachShuffle funForEach;
			ilistsetForEachShuffle(-1, &funForEach);
			ilistsetClear(NULL);
		}
	};
}
