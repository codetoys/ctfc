//shm_ListSetGroup.h 共享内存ListSetGroup容器
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*
T_SHMSET_MUTEX 带互斥的二叉树
T_SHM_MUTEX_SET 带互斥的二叉树
*/

#include "shmListSet.h"
#include "shmSet.h"

namespace ns_my_std
{
	//分为N个子树的SET，非标准接口
	template <typename T_DATA, int PI_N, typename T_DATA_LIST, int PI_N2 >
	class T_SHM_LIST_SET_GROUP : public CShmActiveObjects
	{
	private:
		vector<IListSet<T_DATA, T_DATA_LIST > *> vISet;
		T_SHM_LIST_SET<T_DATA,PI_N  ,T_DATA_LIST,PI_N2,1> m_shmset;
		T_SHM_LIST_SET<T_DATA,PI_N+1,T_DATA_LIST,PI_N2+1,2> m_shmset1;
		T_SHM_LIST_SET<T_DATA,PI_N+2,T_DATA_LIST,PI_N2+2,3> m_shmset2;
		T_SHM_LIST_SET<T_DATA,PI_N+3,T_DATA_LIST,PI_N2+3,4> m_shmset3;
		T_SHM_LIST_SET<T_DATA,PI_N+4,T_DATA_LIST,PI_N2+4,5> m_shmset4;
		T_SHM_LIST_SET<T_DATA,PI_N+5,T_DATA_LIST,PI_N2+5,6> m_shmset5;
		T_SHM_LIST_SET<T_DATA,PI_N+6,T_DATA_LIST,PI_N2+6,7> m_shmset6;
		T_SHM_LIST_SET<T_DATA,PI_N+7,T_DATA_LIST,PI_N2+7,8> m_shmset7;
		T_SHM_LIST_SET<T_DATA,PI_N+8,T_DATA_LIST,PI_N2+8,9> m_shmset8;
		T_SHM_LIST_SET<T_DATA,PI_N+9,T_DATA_LIST,PI_N2+9,10> m_shmset9;
		enum {N_SUBTREE=10};
		long KeyHashToIndex(long kh)const
		{
			//thelog<<kh<<" "<<kh%N_SUBTREE<<endi;
			if(kh>=0)return kh%N_SUBTREE;
			else return (-kh)%N_SUBTREE;;
		}
		IListSet<T_DATA, T_DATA_LIST > * GetISet(long n)const
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
		struct sub_iterator
		{
			long set_index;
			T_SHM_SIZE handle;
			sub_iterator():set_index(-1),handle(-1){}
			bool operator == (iterator const & tmp)const{return set_index==tmp.set_index && handle==tmp.handle;}
			bool operator != (iterator const & tmp)const{return !(*this==tmp);}
		};
		typedef sub_iterator const_sub_iterator;
	private:
		iterator _find(T_DATA const & value)const
		{
			iterator it;
			it.set_index=KeyHashToIndex(value.keyhash());
			it.handle=GetISet(it.set_index)->isetFind(value);
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
		bool _getSub(iterator const & it,T_DATA_LIST & sub)const
		{
			if (it != end())
			{
				IListSet<T_DATA, T_DATA_LIST > * pSet = GetISet(it.set_index);
				T_DATA_LIST * p= pSet->ilistsetSubGet(it.handle,sub);
				if(NULL==p)
				{
					return false;
				}
				else
				{
					sub=*p;
					return true;
				}
			}
			else
			{
				return false;
			}
		}
		bool _getSubAll(iterator const & it,vector<T_DATA_LIST > & vsub)const
		{
			if (it != end())
			{
				IListSet<T_DATA, T_DATA_LIST > * pSet = GetISet(it.set_index);
				return pSet->ilistsetSubGetAll(it.handle,vsub);
			}
			else
			{
				return false;
			}
		}
		bool _getSubSetAll(iterator const & it,set<T_DATA_LIST > & ssub)const
		{
			if (it != end())
			{
				IListSet<T_DATA, T_DATA_LIST > * pSet = GetISet(it.set_index);
				return pSet->ilistsetSubGetSetAll(it.handle,ssub);
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
		bool _updateSub(iterator const & it,T_DATA_LIST const & sub)
		{
			if (it != end())
			{
				IListSet<T_DATA, T_DATA_LIST > * pSet = GetISet(it.set_index);
				T_DATA_LIST * p= pSet->ilistsetSubGet(it.handle,sub);
				if(NULL==p)
				{
					pSet->ilistsetSubAdd(it.handle,sub);
					return true;
				}
				else
				{
					*p=sub;
					return true;
				}
			}
			else
			{
				return false;
			}
		}
		bool _addSub(iterator const & it,T_DATA_LIST const & sub)
		{
			if (it != end())
			{
				IListSet<T_DATA, T_DATA_LIST > * pSet = GetISet(it.set_index);
				T_DATA_LIST * p= pSet->ilistsetSubGet(it.handle,sub);
				if(NULL==p)
				{
					pSet->ilistsetSubAdd(it.handle,sub);
					return true;
				}
				else
				{
					*p=*p+sub;
					return true;
				}
			}
			else
			{
				return false;
			}
		}
		//bool _erase(T_DATA const & value)
		//{
		//	iterator it=_find(value);
		//	if(it!=end())
		//	{
		//		GetISet(it.set_index)->isetErase(it.handle);
		//		return true;
		//	}
		//	else
		//	{
		//		return false;
		//	}
		//}
		bool _lsgClear(typename IListSet<T_DATA, T_DATA_LIST >::IlsgClear * fun)
		{
			typename vector<IListSet<T_DATA, T_DATA_LIST > *>::iterator it;
			long count_main = 0;
			long count_second = 0;
			thelog << "根据规则清理......" << endi;
			for (it = vISet.begin(); it != vISet.end(); ++it)
			{
				if (!(*it)->ilistsetClear(fun, count_main, count_second))return false;
			}
			thelog << "清理完成 清理主数据 " << count_main << " 个 子数据 " << count_second << " 个" << endi;
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

		bool _ForEach(typename IListSet<T_DATA,T_DATA_LIST >::IListSetForEach * fun)
		{
			typename vector<IListSet<T_DATA, T_DATA_LIST > *>::iterator it;
			for(it=vISet.begin();it!=vISet.end();++it)
			{
				if(!(*it)->ilistsetForEach(fun))return false;
			}
			return true;
		}
		bool _ForEachSub(iterator const & it,typename IListSet<T_DATA,T_DATA_LIST >::IListSetForEachSub * fun)
		{
			if (it != end())
			{
				IListSet<T_DATA, T_DATA_LIST > * pSet = GetISet(it.set_index);
				return pSet->ilistsetForEachSub(it.handle,fun);
			}
			else
			{
				return false;
			}
		}
		bool _ForEachShuffle(long iSet, long handle, typename IListSet<T_DATA,T_DATA_LIST >::IListSetForEachShuffle * fun)
		{
			typename vector<IListSet<T_DATA, T_DATA_LIST > *>::iterator it;
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
			for(;it!=vISet.end();++it)
			{
				fun->iSet = it - vISet.begin();//设置iSet
				if (iSet == fun->iSet)
				{
					if (!(*it)->ilistsetForEachShuffle(handle, fun))return false;
				}
				else
				{
					if (!(*it)->ilistsetForEachShuffle(-1, fun))return false;
				}
			}
			return true;
		}
	public:
		virtual char const * GetName()const{return m_shmset.GetName();}

		virtual bool ReportData()const
		{
			//m_shmset.Report();
			//m_shmset1.Report();
			//m_shmset2.Report();
			//m_shmset3.Report();
			//m_shmset4.Report();
			//m_shmset5.Report();
			//m_shmset6.Report();
			//m_shmset7.Report();
			//m_shmset8.Report();
			//m_shmset9.Report();

			CHtmlDoc::CHtmlTable2 table;
			ReportHtmlTable(table, false, 0, 20);
			thelog << endl << table.MakeTextTable() << endi;
			return true;
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			string file = dir_name;
			if (file.size()>0 && file[file.size() - 1] != '/')file += "/";
			file += this->GetName();
			file += ".txt";

			ofstream f;

			f.open(file.c_str());
			if (!f.good())
			{
				thelog << "打开文件失败 " << file << ende;
				return false;
			}

			string str;
			long count = 0;
			thelog << "开始导出..." << endi;
			for (const_iterator it = begin(); it != end(); MoveNext(&it))
			{
				vector<T_DATA_LIST > subs;
				getSubAll(it, subs);
				if (0 == subs.size())
				{
				}
				else
				{
					for (typename vector<T_DATA_LIST >::iterator sub_it = subs.begin(); sub_it != subs.end(); ++sub_it)
					{
						++count;
						GetByHandle(it).toString(str);
						str += " ";
						f.write(str.c_str(), str.size());
						(*sub_it).toString(str);
						str += "\n";
						f.write(str.c_str(), str.size());
						if (0 == count % 100000)thelog << "导出 " << count << " 条" << endi;
					}
				}
			}
			thelog << "导出完成，共 " << count << " 条" << endi;

			f.close();
			return true;
		}
		T_SHM_LIST_SET_GROUP(char const * name,char const * name2,int version):
			m_shmset(name,name2,version)
			,m_shmset1(name,name2,version)
			,m_shmset2(name,name2,version)
			,m_shmset3(name,name2,version)
			,m_shmset4(name,name2,version)
			,m_shmset5(name,name2,version)
			,m_shmset6(name,name2,version)
			,m_shmset7(name,name2,version)
			,m_shmset8(name,name2,version)
			,m_shmset9(name,name2,version)
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
			T_DATA_LIST::AddTableColumns(table);

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
				vector<T_DATA_LIST > subs;
				getSubAll(it,subs);
				if(0==subs.size())
				{
					++count;
					table.AddLine();
					table.AddData(it.set_index);
					table.AddData(i);
					GetByHandle(it).AddTableData(table);
				}
				else
				{
					for (typename vector<T_DATA_LIST >::iterator sub_it = subs.begin(); sub_it != subs.end(); ++sub_it)
					{
						++count;
						table.AddLine();
						table.AddData(it.set_index);
						table.AddData(i);
						GetByHandle(it).AddTableData(table);
						sub_it->AddTableData(table);
					}
				}
			}
			return count;
		}
		//直接存取
		bool get(T_DATA * value)const{return get(*value);}
		bool get(T_DATA & value)const
		{
			return _get(value);
		}
		bool getSub(iterator const & it,T_DATA_LIST & sub)const
		{
			return _getSub(it,sub);
		}
		bool getSubAll(iterator const & it,vector<T_DATA_LIST > & vsub)const
		{
			return _getSubAll(it,vsub);
		}
		bool getSubSetAll(iterator const & it,set<T_DATA_LIST > & ssub)const
		{
			return _getSubSetAll(it,ssub);
		}
		//直接查找
		iterator find(T_DATA const & value) const
		{
			return _find(value);
		}
		bool update(T_DATA const & value)
		{
			return _update(value);
		}
		bool updateSub(iterator const & it,T_DATA_LIST const & sub)
		{
			return _updateSub(it,sub);
		}
		bool addSub(iterator const & it,T_DATA_LIST const & sub)
		{
			return _addSub(it,sub);
		}

		//bool erase(T_DATA const & value)
		//{
		//	return _erase(value);
		//}
		pair<iterator, bool> insert(T_DATA const & value)
		{
			return _insert(value);
		}
		bool lsgClear(typename IListSet<T_DATA, T_DATA_LIST >::IlsgClear * fun)
		{
			return _lsgClear(fun);
		}
		template<typename T_FUN_B1>
		bool Clear(T_FUN_B1 fun,iterator itBegin)
		{
			return _Clear(fun,itBegin);
		}
		bool ForEach(typename IListSet<T_DATA,T_DATA_LIST >::IListSetForEach * fun)
		{
			return _ForEach(fun);
		}
		bool ForEachSub(iterator const & it,typename IListSet<T_DATA,T_DATA_LIST >::IListSetForEachSub * fun)
		{
			return _ForEachSub(it,fun);
		}
		typedef typename IListSet<T_DATA,T_DATA_LIST >::IListSetForEachShuffle T_FOR_EACH_SHUFFLE;
		bool ForEachShuffle(long iSet, long handle, T_FOR_EACH_SHUFFLE * fun)
		{
			return _ForEachShuffle(iSet, handle, fun);
		}
		//编译测试
		void test()
		{
			m_shmset.test();
		}
	};
}
