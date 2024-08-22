//shmSet_NoMutex.h 共享内存Set容器 不带互斥的基本版本
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*
本文件定义二叉树模板和带互斥的二叉树模版
T_SHMSET 二叉树，IShmActiveObject接口
*/

#include "shmstd.h"
#include "shmArray.h"

namespace ns_my_std
{
	//单字节标志组
	struct T_FLAG
	{
		unsigned char _flag;
		T_FLAG() :_flag(0) {}
		bool getflag(int flag)const
		{
			if (flag < 0 || flag >= 8)
			{
				thelog << "无效的位 " << flag << ende;
				return false;
			}
			return _flag & (1 << flag);
		}
		bool setflag(int flag, bool value)
		{
			if (flag < 0 || flag >= 8)
			{
				thelog << "无效的位 " << flag << ende;
				return false;
			}
			_flag &= (0 << flag);//清位
			_flag |= ((value ? 1 : 0) << flag);
			return true;
		}
	};
	//遍历标志的接口
	template<typename T_DATA>
	class IForEachFlag
	{
	protected:
		bool isDeleted;//是否是删除的，这个不是标志，表明数据是否已经删除
		T_FLAG * flag;//内置标志 位0 添加 位1修改（包括因为不存在而添加） 位2 删除
		T_FLAG * uflag;//用户标志
	public:
		void set(bool d, T_FLAG * f, T_FLAG * uf)
		{
			isDeleted = d;
			flag = f;
			uflag = uf;
		}
	protected:
		//检查一个节点的标志（即使节点已经删除，标志位仍然可能有用）
		bool GetFlagInsert()const
		{
			return flag->getflag(0);
		}
		bool GetFlagUpdate()const
		{
			return flag->getflag(1);
		}
		bool GetFlagDelete()const
		{
			return flag->getflag(2);
		}
		//设置一个节点的标志（即使节点已经删除，标志位仍然可能有用）
		bool ClearFlagInsert()const
		{
			return flag->setflag(0, false);
		}
		bool ClearFlagUpdate()const
		{
			return flag->setflag(1, false);
		}
		bool ClearFlagDelete()const
		{
			return flag->setflag(2, false);
		}
		bool SetFlagInsert()const
		{
			return flag->setflag(0, true);
		}
		bool SetFlagUpdate()const
		{
			return flag->setflag(1, true);
		}
		bool SetFlagDelete()const
		{
			return flag->setflag(2, true);
		}
		//检查一个节点的标志（即使节点已经删除，标志位仍然可能有用）flag 0~7
		bool GetUserFlag(int flag)const
		{
			return uflag->getflag(flag);
		}
		//设置一个节点的标志（即使节点已经删除，标志位仍然可能有用）flag 0~7
		bool SetUserFlag(int flag, bool value)const
		{
			return uflag->setflag(flag, value);
		}
	public:
		virtual bool doOneData(long handle, T_DATA const * pData) = 0;
	};

	//set接口，用于分为N个子树的set
	template<typename T_DATA>
	class ISet
	{
	public:
		//IListSet遍历接口
		struct ISetForEach
		{
			long iSet;//非group对象为-1
			ISetForEach() :iSet(-1) {}
			virtual bool doOneData(long handle, T_DATA const * pData) = 0;
		};
	public:
		virtual T_DATA * isetGet(long h)const
		{
			thelog << " ISet::isetGet 未实现" << ende;
			return NULL;
		}
		virtual long isetMoveNext(long & h)const
		{
			thelog << " ISet::isetMoveNext 未实现" << ende;
			return -1;
		}
		virtual long isetBegin()const
		{
			thelog << " ISet::isetBegin 未实现" << ende;
			return -1;
		}
		virtual pair<long, bool> isetInsert(T_DATA const & value)
		{
			pair<long, bool> tmp;
			tmp.first = -1;
			tmp.second = false;
			thelog << " ISet::isetInsert 未实现" << ende;
			return tmp;
		}
		virtual long isetFind(T_DATA const & value)const
		{
			thelog << " ISet::isetFind 未实现" << ende;
			return -1;
		}
		virtual long isetFindLowerBound(T_DATA const & value, bool(*less)(T_DATA const &, T_DATA const &))const
		{
			thelog << " ISet::isetFindLowerBound 未实现" << ende;
			return -1;
		}
		virtual bool isetErase(long h)
		{
			thelog << " ISet::isetErase 未实现" << ende;
			return -1;
		}
		virtual bool isetForEachShuffle(long handle, ISetForEach * pForEach)const
		{
			thelog << " IListSet::isetForEachShuffle 未实现" << ende;
			return true;
		}
	};

	int const LH = 1; //左高
	int const EH = 0; //平衡
	int const RH = -1; //右高

	//平衡二叉树模版，T_DATA 数据类型，PI_N 一个全局的指针变量的索引，客户代码只能提供而不能操作此变量，该变量用于简化模版内部实现

	template<typename T_DATA >
	struct T_TREE_NODE_STRUCT
	{
		T_SHM_SIZE hParent;//-1:无，根节点；0-N,子节点，或指向下个空闲地址
		T_SHM_SIZE hLeft;//-1表示无子节点
		T_SHM_SIZE hRight;//-1表示无子节点
		//平衡因子
		signed char bf;//balance_factor 0;平衡 1： 左子节点高 -1：右子节点高
		//删除标志
		signed char deleted;//0：有效，1：删除
		//额外标志
		T_FLAG flag;//内置标志 位0 添加 位1修改（包括因为不存在而添加） 位2 删除
		T_FLAG uflag;//用户标志
		//用户数据结构
		T_DATA data;

		T_TREE_NODE_STRUCT() :hParent(-1), hLeft(-1), hRight(-1), bf(EH), deleted(0) {}
		T_TREE_NODE_STRUCT(T_SHM_SIZE parent, T_DATA const & tmp) :hParent(parent), hLeft(-1), hRight(-1), bf(EH), deleted(0), data(tmp) {}
		string & toString(string & str, void * = NULL)const
		{
			char buf[2048];
			string tmp;
			sprintf(buf, "%8ld %8ld %8ld %1d %1d %3d %3d: %s", hParent, hLeft, hRight, bf, deleted
				, (unsigned int)flag._flag, (unsigned int)uflag._flag, data.toString(tmp).c_str());
			return str = buf;
		}
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{
			table.AddCol("P", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("L", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("R", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("b", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("d", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("F", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("uF", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("|");
			return T_DATA::AddTableColumns(table);
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddData(hParent);
			table.AddData(hLeft);
			table.AddData(hRight);
			table.AddData(bf);
			table.AddData(deleted);
			table.AddData((unsigned int)flag._flag);
			table.AddData((unsigned int)uflag._flag);
			table.AddData("|");
			return data.AddTableData(table);
		}
	};
	template<typename T_DATA, int PI_N, typename T_HANDLE, typename T_COMP >
	struct T_TREE_NODE : public T_TREE_NODE_STRUCT<T_DATA >
	{
		using T_TREE_NODE_STRUCT<T_DATA >::data;
		using T_TREE_NODE_STRUCT<T_DATA >::hLeft;
		using T_TREE_NODE_STRUCT<T_DATA >::hRight;
		using T_TREE_NODE_STRUCT<T_DATA >::hParent;

		T_TREE_NODE() {}
		T_TREE_NODE(T_SHM_SIZE parent, T_DATA const& tmp) :T_TREE_NODE_STRUCT<T_DATA >(parent, tmp) {}
		bool operator < (T_TREE_NODE const & tmp)const
		{
			T_COMP comp;
			return comp(data, tmp.data);
		}

		static T_TREE_NODE & at(T_SHM_SIZE n)
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
			return *(T_TREE_NODE *)(void *)&*array_handle;
		}
		T_SHM_SIZE _me()const
		{
			return T_HANDLE::_me(this);
		}
		T_SHM_SIZE _begin()const
		{
			if (-1 == hLeft)return _me();
			return at(hLeft)._begin();
		}
		T_SHM_SIZE _end()const
		{
			if (-1 == hRight)return _me();
			return at(hRight)._end();
		}
		bool isRight()const
		{
			return -1 != hParent && _me() == at(hParent).hRight;
		}
		bool isLeft()const
		{
			return !isRight();
		}
	};

	template<typename T_DATA, int PI_N >
	struct T_HANDLE_SET :public	T_HANDLE_ARRAY<T_TREE_NODE_STRUCT<T_DATA >, PI_N >
	{
		typedef T_TREE_NODE_STRUCT<T_DATA > T;
		typedef T_HANDLE_ARRAY<T, PI_N > T_PARENT;
		using T_PARENT::handle;
		T_HANDLE_SET(T_SHM_SIZE h = -1) :T_PARENT(h) {}
		T_HANDLE_SET(T_HANDLE_SET const& tmp) :T_PARENT(tmp.handle) {}
		
		using T_PARENT::iterator_category;
		using T_PARENT::pointer;
		using T_PARENT::element_type;
		using T_PARENT::value_type;
		using T_PARENT::difference_type;
		using T_PARENT::offset_type;
		using T_PARENT::reference;
		
		bool operator<(T_HANDLE_SET const& tmp)const { return handle < tmp.handle; }
		T_HANDLE_SET operator + (long n)const
		{
			T_HANDLE_SET tmp;
			tmp.handle = handle + n;
			return tmp;
		}
		T_HANDLE_SET operator - (long n)const
		{
			T_HANDLE_SET tmp;
			tmp.handle = handle - n;
			return tmp;
		}
		T_SHM_SIZE operator - (T_HANDLE_SET const& tmp)const { return handle - tmp.handle; }
		T_HANDLE_SET& operator += (T_SHM_SIZE n) { handle += n; return *this; }
		T_HANDLE_SET& operator ++ () { ++handle; return *this; }
		T_HANDLE_SET& operator -- () { --handle; return *this; }
		T_HANDLE_SET& operator = (T_HANDLE_SET const& tmp) { handle = tmp.handle; return *this; }
		bool operator == (T_HANDLE_SET const& tmp)const { return handle == tmp.handle; }
		bool operator != (T_HANDLE_SET const& tmp)const { return !((*this) == tmp); }
		T& operator * ()const
		{
			return *operator ->();
		}
		T* operator -> ()const
		{
			return T_PARENT::operator->();
		}
		static T_SHM_SIZE _me(T const* p, bool not_throw = false)
		{
			return T_PARENT::_me(p, not_throw);
		}
		static void ShowVMapPrivateData()
		{
			return T_PARENT::ShowVMapPrivateData();
		}
	};

	template<typename T_DATA, int PI_N, typename T_USER_HEAD = CDemoData, int PART = 0, int VER = 0, typename T_COMP = less<T_DATA>, typename T_HANDLE = T_HANDLE_SET<T_DATA , PI_N > >
	class T_SHMSET_NO_MUTEX : public IShmActiveObject
	{
	public:
		typedef T_HANDLE T_ARRAY_HANDLE;
		typedef T_TREE_NODE<T_DATA, PI_N, T_HANDLE, T_COMP > TREE_NODE;
		struct iterator
		{
			T_SHM_SIZE handle;

			iterator() :handle(-1) {}
			bool operator == (iterator const & tmp)const { return handle == tmp.handle; }
			bool operator != (iterator const & tmp)const { return !(*this == tmp); }
			T_DATA & operator * ()const
			{
				return TREE_NODE::at(handle).data;
			}
			T_DATA * operator -> ()const
			{
				return &(operator *());
			}
			iterator & operator ++ ()
			{
				if (-1 != TREE_NODE::at(handle).hRight)
				{//存在右子树，取右子树的begin
					handle = TREE_NODE::at(handle).hRight;
					handle = TREE_NODE::at(handle)._begin();
				}
				else if (-1 != TREE_NODE::at(handle).hParent)
				{//存在父节点
					if (TREE_NODE::at(handle).isRight())
					{//是父节点的右子树，向上找到是左子树的节点，取这个节点的父节点
						while ((handle = TREE_NODE::at(handle).hParent) != -1 && TREE_NODE::at(handle).isRight()) {}
						if (-1 != handle && !TREE_NODE::at(handle).isRight())handle = TREE_NODE::at(handle).hParent;
					}
					else
					{//是父节点的左子树，取父节点
						handle = TREE_NODE::at(handle).hParent;
					}
				}
				else
				{//根节点且没有右子树，结束
					handle = -1;
				}
				return *this;
			}
			iterator & operator -- ()
			{
				if (-1 != TREE_NODE::at(handle).hLeft)
				{//存在左子树，取左子树的end
					handle = TREE_NODE::at(handle).hLeft;
					handle = TREE_NODE::at(handle)._end();
				}
				else if (-1 != TREE_NODE::at(handle).hParent)
				{//存在父节点
					if (TREE_NODE::at(handle).isLeft())
					{//是父节点的左子树，向上找到是右子树的节点，取这个节点的父节点
						while ((handle = TREE_NODE::at(handle).hParent) != -1 && TREE_NODE::at(handle).isLeft()) {}
						if (-1 != handle && !TREE_NODE::at(handle).isLeft())handle = TREE_NODE::at(handle).hParent;
					}
					else
					{//是父节点的右子树，取父节点
						handle = TREE_NODE::at(handle).hParent;
					}
				}
				else
				{//根节点且没有右子树，结束
					handle = -1;
				}
				return *this;
			}
		};
		typedef iterator const_iterator;
		struct TREE_HEAD
		{
			T_SHM_SIZE hHead;
			T_SHM_SIZE size;
			//int deep;
			//add
			T_SHM_SIZE free_head;//空闲地址头指针

			T_USER_HEAD user_head;//用户的特殊数据

			TREE_HEAD() :hHead(-1), size(0), free_head(-1) {}

			//用于输出数据的场合
			string & toString(string & str)const
			{
				char buf[2048];
				sprintf(buf, "head %ld size %ld", hHead, size);
				return str;
			}

			//用于表格输出
			static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
			{
				return false;
			}
			bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
			{
				return false;
			}
		};
	protected:
		typedef T_ARRAY<TREE_NODE, PI_N, TREE_HEAD, PART, T_HANDLE > T_SETARRAY;
		T_SETARRAY m_array;//内置数组对象，存储实际数据
		TREE_HEAD * tree_head;//指向树的头

		bool _check_handle(T_SHM_SIZE h)const
		{
			return h >= -1 && h < m_array.GetHead()->size;
		}
		//检查数据结构是否正确
		bool _check()const
		{
			thelog << "检查树结构，如果检查过程中发生数据修改则检查可能会出错" << endi;

			{
				size_t count_data_array = 0;//数组中的有效数据个数
				T_SHM_SIZE h;
				for (h = 0; h < static_cast<T_SHM_SIZE>(m_array.size()); ++h)
				{
					if (!TREE_NODE::at(h).deleted)++count_data_array;
				}
				thelog << "数组容量 " << m_array.capacity() << " 个数 " << m_array.size() << " 有效数据 " << count_data_array << endi;
				thelog << "树结构容量 " << capacity() << " 个数 " << size() << endi;
				if (count_data_array != size())
				{
					thelog << "树结构大小与数组统计不符 " << size() << " " << count_data_array << ende;
					return false;
				}
			}

			long max_handle = m_array.GetHead()->size - 1;//整个已分配空间的最大句柄
			size_t count_free = 0;//未用节点数（删除的）
			//获取自由节点数
			{
				if (!_check_handle(tree_head->free_head))
				{
					thelog << "free_head error " << tree_head->free_head << ende;
					return false;
				}
				T_SHM_SIZE h = tree_head->free_head;
				while (h >= 0)
				{
					if (!TREE_NODE::at(h).deleted)
					{
						thelog << "此节点未被标记为删除 " << h << ende;
						return false;
					}
					++count_free;
					if (TREE_NODE::at(h).hParent<-1 || TREE_NODE::at(h).hParent>max_handle)
					{
						thelog << "TREE_NODE::at(h).hParent error " << TREE_NODE::at(h).hParent << ende;
						return false;
					}
					h = TREE_NODE::at(h).hParent;
				}
			}
			if (count_free != m_array.size() - size())
			{
				thelog << "删除链表总数不正确 " << count_free << " " << m_array.size() - size() << ende;
				return false;
			}
			thelog << "删除链表节点总数 " << count_free << endi;
			size_t count_used = 0;//已用节点数

			//获取已用节点数
			{
				T_COMP comp;
				iterator it = begin();
				iterator it_old = end();
				while (it != end())
				{
					if (!_check_handle(it.handle))
					{
						thelog << "无效的节点 " << it.handle << ende;
						return false;
					}
					if (TREE_NODE::at(it.handle).deleted)
					{
						thelog << "此节点被标记为删除 " << it.handle << ende;
						return false;
					}
					if (it_old == it)
					{
						thelog << "指针循环 [" << it_old.handle << "][" << it.handle << "]" << ende;
						return false;
					}
					if (it_old != end() && !comp(*it_old, *it))
					{
						string str1, str2;
						thelog << "节点数据比较错误 [" << it_old->toString(str1) << "][" << it->toString(str2) << "]" << ende;
					}
					++count_used;
					it_old = it;
					++it;
				}
			}
			if (count_used != static_cast<size_t>(tree_head->size))
			{
				thelog << "begin->end ！= size " << count_used << " " << tree_head->size << ende;
				return false;
			}
			if (count_used != size())
			{
				thelog << "遍历获得节点数不正确 " << count_used << " " << size() << ende;
				return false;
			}

			thelog << "检查完成，没有错误" << endi;
			return true;
		}
		//修复数据结构
		bool _repair(char const* level)
		{
			thelog << "修复树结构，如果修复过程中发生数据修改则修复结果可能仍然是错的" << endi;
			return FastRebuild_Finish(level, true);
		}
	private:
		//修改头指针指向 src 的改为指向des
		void _changeRoot(T_SHM_SIZE src, T_SHM_SIZE des)
		{
			TREE_NODE::at(des).hParent = TREE_NODE::at(src).hParent;
			if (TREE_NODE::at(des).hParent != -1)
			{
				//旋转后是左节点
				if (TREE_NODE::at(TREE_NODE::at(des).hParent).hLeft == src)
					TREE_NODE::at(TREE_NODE::at(des).hParent).hLeft = des;
				//旋转后是右节点
				else
					TREE_NODE::at(TREE_NODE::at(des).hParent).hRight = des;
			}
			else
			{
				tree_head->hHead = des;
			}
		}
		//右旋转
		void _RRotate(T_SHM_SIZE p)
		{
			//assert(p>=0&&p<tree_head->size);

			//DEBUG_LOG<<"当前位置"<<p <<"大小"<< tree_head->size <<endi;
			T_SHM_SIZE t_lchild = TREE_NODE::at(p).hLeft;
			TREE_NODE::at(p).hLeft = TREE_NODE::at(t_lchild).hRight;

			//右节点非空
			if (TREE_NODE::at(t_lchild).hRight != -1)
				TREE_NODE::at(TREE_NODE::at(t_lchild).hRight).hParent = p;

			//修改根节点父指针
			_changeRoot(p, t_lchild);

			TREE_NODE::at(p).hParent = t_lchild;
			TREE_NODE::at(t_lchild).hRight = p;

		}
		void RRotate(iterator it)
		{
			_RRotate(it.handle);
		}
		//左旋转
		void _LRotate(T_SHM_SIZE p)
		{
			//assert(p>=0&&p<tree_head->size);
			//DEBUG_LOG<<"当前位置"<<p <<"大小"<< tree_head->size <<endi;

			//DEBUG_LOG<<"当前左旋转节点" <<p<<"p's value"<<show(p)<<endi;
			T_SHM_SIZE t_rchild = TREE_NODE::at(p).hRight;

			TREE_NODE::at(p).hRight = TREE_NODE::at(t_rchild).hLeft;

			//右节点非空
			if (TREE_NODE::at(t_rchild).hLeft != -1)
				TREE_NODE::at(TREE_NODE::at(t_rchild).hLeft).hParent = p;
			//非根节点
			//DEBUG_LOG << "_LRotate t_rchild's parent" <<TREE_NODE::at(t_rchild).hParent <<endi;
			//修改根节点父指针
			_changeRoot(p, t_rchild);

			TREE_NODE::at(p).hParent = t_rchild;
			TREE_NODE::at(t_rchild).hLeft = p;
		}
		void LRotate(iterator it)
		{
			_LRotate(it.handle);
		}
		//平衡右边
		void _RBalance(T_SHM_SIZE p)
		{
			//DEBUG_LOG<<"右平衡"<<p<<"value"<<show(p)<<endi;
			T_SHM_SIZE t_rchild = TREE_NODE::at(p).hRight;
			//DEBUG_LOG<<"右平衡右子树的情况"<<t_rchild<<"value"<<show(t_rchild)<<endi; 
			T_SHM_SIZE t_lchild = -1;
			switch (TREE_NODE::at(t_rchild).bf)
			{
			case EH:
				TREE_NODE::at(p).bf = RH;
				TREE_NODE::at(t_rchild).bf = LH;
				_LRotate(p);
				break;
			case RH:
				TREE_NODE::at(p).bf = EH;
				TREE_NODE::at(t_rchild).bf = EH;
				_LRotate(p);
				break;
			case LH:
				t_lchild = TREE_NODE::at(t_rchild).hLeft;
				switch (TREE_NODE::at(t_lchild).bf)
				{
				case LH:
					TREE_NODE::at(p).bf = EH;
					TREE_NODE::at(t_rchild).bf = RH;
					break;
				case EH:
					TREE_NODE::at(p).bf = EH;
					TREE_NODE::at(t_rchild).bf = EH;
					break;
				case RH:
					TREE_NODE::at(p).bf = LH;
					TREE_NODE::at(t_rchild).bf = EH;
					break;
				default:
					thelog << __FILE__ << __LINE__ << "RBalance bf error" << TREE_NODE::at(t_lchild).bf << ende;
					break;
				}
				//DEBUG_LOG<<"右平衡右旋转左节点"<<t_lchild<<"value"<<show(t_lchild)<<endi;
				TREE_NODE::at(t_lchild).bf = EH;
				_RRotate(t_rchild);
				_LRotate(p);
				//DEBUG_LOG<<"右平衡左旋转后根节点情况"<<t_lchild<<"value"<<show(t_lchild)<<endi;
				break;
			default:
				thelog << "右平衡右子节点平衡因子错误 t_rchild " << t_rchild << " : " << show(t_rchild) << ende;
				thelog << "右平衡右子节点平衡因子错误 p " << p << " : " << show(p) << ende;
				G_SET_ERROR(My_ERR_INVALID_BALANCE);
				break;
			}


			//DEBUG_LOG<<"右平衡后右节点的情况"<<t_rchild<<"value"<<show(t_rchild)<<endi; 

		}
		void RBalance(iterator it)
		{
			_RBalance(it.handle);
		}

		//平衡左边
		void _LBalance(T_SHM_SIZE p)
		{
			//左平衡
			//DEBUG_LOG<<"左平衡" << show(p)<<endi;
			T_SHM_SIZE t_lchild = TREE_NODE::at(p).hLeft;
			//DEBUG_LOG<<"左平衡左子树的情况"<<show(t_lchild)<<endi; 
			T_SHM_SIZE t_rchild = -1;
			switch (TREE_NODE::at(t_lchild).bf)
			{
			case EH:
				TREE_NODE::at(p).bf = LH;
				TREE_NODE::at(t_lchild).bf = RH;
				_RRotate(p);
				break;
			case LH:
				TREE_NODE::at(p).bf = TREE_NODE::at(t_lchild).bf = EH;
				_RRotate(p);
				break;
			case RH:
				t_rchild = TREE_NODE::at(t_lchild).hRight;
				switch (TREE_NODE::at(t_rchild).bf)
				{
				case LH:
					TREE_NODE::at(p).bf = RH;
					TREE_NODE::at(t_lchild).bf = EH;
					break;
				case EH:
					TREE_NODE::at(p).bf = EH;
					TREE_NODE::at(t_lchild).bf = EH;
					break;
				case RH:
					TREE_NODE::at(p).bf = EH;
					TREE_NODE::at(t_lchild).bf = LH;
					break;
				default:
					thelog << "LBalance bf error " << t_rchild << " " << TREE_NODE::at(t_rchild).bf << ende;
					break;
				}
				TREE_NODE::at(t_rchild).bf = EH;
				_LRotate(t_lchild);
				_RRotate(p);
				break;
			default:
				thelog << "左平衡左子节点平衡因子错误" << show(t_lchild) << ende;
				break;
			}
			//DEBUG_LOG<<"左平衡旋转后的情况"<<show(t_lchild)<<endi; 
		}
		void LBalance(iterator it)
		{
			_LBalance(it.handle);

		}
		//taller 判断插入数据后节点深度是否增加
		pair<iterator, bool> __insert(T_DATA const & tmp, T_COMP & comp)
		{

			pair<iterator, bool> tmppair;
			T_SHM_SIZE hParent = -1;
			T_SHM_SIZE insert_position = -1;
			bool taller = false;
			bool isLeft = true;//插入节点方向 默认左插入
			bool isInsert = false;
			//need more work
			//从父节点开始查
			if ((insert_position = ___insert(tmp, tree_head->hHead, taller, hParent, isLeft, isInsert, comp)) >= 0)
			{
				tmppair.first.handle = insert_position;
				tmppair.second = isInsert;
				//DEBUG_LOG<<"插入节点"<<insert_position<<"数据 "<<show(insert_position) <<endi;
			}
			else
			{
				//插入失败或者空间已满
				thelog << "插入节点失败" << ende;
				tmppair.first.handle = -1;
				tmppair.second = false;
			}
			return tmppair;

		}
		//插入节点返回-1不成功
		T_SHM_SIZE ___insert(T_DATA const & tmp, T_SHM_SIZE vp, bool& taller, T_SHM_SIZE& parent, bool isLeft, bool& isInsert, T_COMP & comp)
		{
			T_SHM_SIZE insert_position = -1;
			if (vp == -1)
			{
				insert_position = tree_head->free_head;
				____insert(tmp, insert_position, parent, isLeft, isInsert);
				//DEBUG_LOG<<"insert_position="<<insert_position<<endi;
				if (isInsert)
					taller = true;
				else
					taller = false;
			}
			else
			{
				if (comp(TREE_NODE::at(vp).data, tmp))
				{
					if ((insert_position = ___insert(tmp, TREE_NODE::at(vp).hRight, taller, vp, false, isInsert, comp)) == -1)
					{
						taller = false;
						return -1;
					}
					if (taller)
					{
						switch (TREE_NODE::at(vp).bf)
						{
						case LH:
							TREE_NODE::at(vp).bf = EH;
							taller = false;
							break;
						case EH:
							TREE_NODE::at(vp).bf = RH;
							taller = true;
							break;
						case RH:
							_RBalance(vp);
							taller = false;
							break;
						default:
							thelog << "右插入数据平衡因子错误" << show(vp) << ende;
							taller = false;
							break;
						}
					}
				}
				else if (comp(tmp, TREE_NODE::at(vp).data))
				{
					if ((insert_position = ___insert(tmp, TREE_NODE::at(vp).hLeft, taller, vp, true, isInsert, comp)) == -1)
					{
						taller = false;
						return -1;
					}
					if (taller)
					{
						switch (TREE_NODE::at(vp).bf)
						{
						case LH:
							_LBalance(vp);
							taller = false;
							break;
						case EH:
							TREE_NODE::at(vp).bf = LH;
							taller = true;
							break;
						case RH:
							TREE_NODE::at(vp).bf = EH;
							taller = false;
							break;
						default:
							thelog << "左插入数据平衡因子错误" << show(vp) << ende;
							taller = false;
							break;
						}
					}
				}
				else
				{
					//thelog<<"插入数据已经存在"<<endw;
					isInsert = false;
					taller = false;
					m_OldValue = TREE_NODE::at(vp).data;//保存被覆盖的值
					m_OldValueSeted = true;//设置被覆盖的对象有效
					update(vp, tmp);
					return vp;
				}

			}
			return insert_position;

		}
		//插入节点
		void ____insert(T_DATA const & data, T_SHM_SIZE & position, T_SHM_SIZE parent, bool isLeft, bool& isInsert)
		{
			if (position < 0)
			{
				//动态扩展长度
				if (m_array.Capacity() > m_array.Size())
				{
					TREE_NODE tmp;
					typename T_SETARRAY::HANDLE h;
					if (!m_array.Add(tmp, h))
					{
						thelog << "扩展数组出错" << ende;
						isInsert = false;
						return;
					}
					else
					{
						tree_head = m_array.GetUserHead();
						position = h.handle;
					}
				}
				else
				{
					thelog << "空间已满，请申请更大空间!!!!!!!!!!!!!!!!!!" << ende;
					isInsert = false;
					return;
				}
			}
			else
			{
				//thelog<<"exist position="<<position<<endi;
			}
			tree_head->free_head = TREE_NODE::at(position).hParent;
			//char buf[256];
			//sprintf(buf,"%ld %p",position,&TREE_NODE::at(position));
			//thelog<<buf<<endi;
			new(&TREE_NODE::at(position)) TREE_NODE(parent, data);
			//DEBUG_LOG<<show(position)<<endi;
			TREE_NODE::at(position).deleted = 0;
			if (parent == -1)
				tree_head->hHead = position;
			else
			{
				//修改父节点
				//DEBUG_LOG<<show(parent)<<endi;
				if (isLeft)
				{
					TREE_NODE::at(parent).hLeft = position;
					//DEBUG_LOG<<show(parent)<<endi;
				}
				else
				{
					TREE_NODE::at(parent).hRight = position;
					//DEBUG_LOG<<show(parent)<<endi;
				}
			}
			isInsert = true;
			//DEBUG_LOG<<"新增节点"<<position<<"["<<show(position)<<"]"<<endi;
			++tree_head->size;
			if (tree_head->size % 200000 == 0)
			{
				thelog << GetName() << "树结构新增数据" << tree_head->size << endi;
			}
		}

		//重新平衡最右边节点移动节点后的树
		T_SHM_SIZE _MoveBalance(T_SHM_SIZE root, bool & lower, T_SHM_SIZE src_root)
		{
			T_SHM_SIZE t_l_rchild = -1;
			if (TREE_NODE::at(root).hRight == -1)
			{
				TREE_NODE::at(TREE_NODE::at(root).hParent).hRight = TREE_NODE::at(root).hLeft;
				if (TREE_NODE::at(root).hLeft != -1)
					TREE_NODE::at(TREE_NODE::at(root).hLeft).hParent = TREE_NODE::at(root).hParent;

				_changeRoot(src_root, root);

				TREE_NODE::at(root).bf = TREE_NODE::at(src_root).bf;
				TREE_NODE::at(root).hRight = TREE_NODE::at(src_root).hRight;
				TREE_NODE::at(root).hLeft = TREE_NODE::at(src_root).hLeft;

				if (TREE_NODE::at(root).hLeft != -1)
					TREE_NODE::at(TREE_NODE::at(root).hLeft).hParent = root;
				if (TREE_NODE::at(root).hRight != -1)
					TREE_NODE::at(TREE_NODE::at(root).hRight).hParent = root;

				lower = true;
				t_l_rchild = root;
				return root;
			}
			else
			{
				if ((t_l_rchild = _MoveBalance(TREE_NODE::at(root).hRight, lower, src_root)) < 0)
				{
					lower = false;
					return -1;
				}
				else
				{
					if (lower)
					{
						switch (TREE_NODE::at(root).bf)
						{
						case LH:
							if (TREE_NODE::at(TREE_NODE::at(root).hLeft).bf != EH)
								lower = true;
							else
								lower = false;
							_LBalance(root);
							break;
						case EH:
							TREE_NODE::at(root).bf = LH;
							lower = false;
							break;
						case RH:
							TREE_NODE::at(root).bf = EH;
							lower = true;
							break;
						default:
							thelog << "_MoveBalance平衡因子出错" << show(root) << ende;
							return -1;

						}
					}
				}
			}
			return t_l_rchild;
		}
		//根节点左边最右节点移动到根节点的平衡旋转
		T_SHM_SIZE MoveBalance(T_SHM_SIZE root, bool& lower)
		{
			//DEBUG_LOG<<"平衡修改" << show(root) <<endi;
			T_SHM_SIZE t_lchild = TREE_NODE::at(root).hLeft;
			T_SHM_SIZE t_l_rchild;
			//DEBUG_LOG<<"查看左边节点"<<show(t_l_rchild)<<endi;
			if (TREE_NODE::at(t_lchild).hRight == -1)
			{

				_changeRoot(root, t_lchild);
				//TREE_NODE::at(t_l_rchild).hParent=TREE_NODE::at(root).hParent;
				//TREE_NODE::at(t_l_rchild).hLeft=-1;
				TREE_NODE::at(t_lchild).hRight = TREE_NODE::at(root).hRight;
				TREE_NODE::at(t_lchild).bf = TREE_NODE::at(root).bf;

				TREE_NODE::at(TREE_NODE::at(t_lchild).hRight).hParent = t_lchild;
				lower = true;
				//switch(TREE_NODE::at(t_lchild).bf)
				//{
				//case LH:
				//	TREE_NODE::at(t_lchild).bf=EH;
				//	lower=true;
				//	break;
				//case EH:
				//	TREE_NODE::at(t_lchild).bf=RH;
				//	lower=false;
				//	break;
				//case RH:
				//	if(TREE_NODE::at(TREE_NODE::at(t_lchild).hRight).bf!=EH)
				//		lower=true;
				//	else
				//		lower=false;
				//	_RBalance(t_lchild);
				//	break;
				//default:
				//	thelog<<"move_balance 平衡因子错误"<<show(t_lchild)<<ende;
				//	return -1;
				//	break;					
				//}
				//
				return t_lchild;
			}
			else
			{

				if ((t_l_rchild = _MoveBalance(TREE_NODE::at(t_lchild).hRight, lower, root)) < 0)
				{
					lower = false;
					return -1;
				}
				else
				{
					//删除原父节点指向
					//DEBUG_LOG<<"查询左子树的最右节点" <<show(t_l_rchild)<<endi;

					if (lower)
					{
						switch (TREE_NODE::at(t_lchild).bf)
						{
						case RH:
							TREE_NODE::at(t_lchild).bf = EH;
							lower = true;
							break;
						case EH:
							TREE_NODE::at(t_lchild).bf = LH;
							lower = false;
							break;
						case LH:
							if (TREE_NODE::at(TREE_NODE::at(t_lchild).hLeft).bf != EH)
								lower = true;
							else
								lower = false;
							_LBalance(t_lchild);
							break;
						default:
							thelog << "_MoveBalance后平衡因子出错" << show(t_lchild) << ende;
							return -1;
						}
					}
				}
			}
			return t_l_rchild;
		}

		//删除指定节点
		void ___erase(T_SHM_SIZE position, bool & lower)
		{
			//DEBUG_LOG<<"删除" << position <<"节点" << show(position)<<endi;
			//修改空闲头指针


			//如果为叶子节点直接删除
			if (TREE_NODE::at(position).hLeft == -1 && TREE_NODE::at(position).hRight == -1)
			{
				//DEBUG_LOG<<"删除节点为叶子节点" <<endi;
				//判断是否为根节点,唯一节点删除
				if (TREE_NODE::at(position).hParent == -1)
				{
					//DEBUG_LOG<<"删除节点"<<position<<"为根节点" <<show(position)<<endi;
					tree_head->hHead = -1;
				}
				else
				{
					if (TREE_NODE::at(TREE_NODE::at(position).hParent).hLeft == position)
					{
						//DEBUG_LOG<<"删除节点"<<position<<"为父节点"<<TREE_NODE::at(position).hParent<<"的左节点"<<endi;
						TREE_NODE::at(TREE_NODE::at(position).hParent).hLeft = -1;
					}
					else
					{
						//DEBUG_LOG<<"删除节点"<<position<<"为父节点"<<TREE_NODE::at(position).hParent<<"的右节点"<<endi;
						TREE_NODE::at(TREE_NODE::at(position).hParent).hRight = -1;
					}
				}

				lower = true;
			}
			//有左子节点，无右子节点
			else if (TREE_NODE::at(position).hLeft != -1 && TREE_NODE::at(position).hRight == -1)
			{
				//DEBUG_LOG<<"删除节点有左无右"<<show(position)<<endi;
				T_SHM_SIZE t_lchild = TREE_NODE::at(position).hLeft;
				_changeRoot(position, t_lchild);
				//DEBUG_LOG<<"删除有左无右"<<show(t_lchild)<<endi;
				lower = true;
			}
			//有右子节点，无左节点
			else if (TREE_NODE::at(position).hLeft == -1 && TREE_NODE::at(position).hRight != -1)
			{
				//DEBUG_LOG<<"删除节点有右无左"<<show(position)<<endi;
				T_SHM_SIZE t_rchild = TREE_NODE::at(position).hRight;
				_changeRoot(position, t_rchild);
				//DEBUG_LOG<<"删除有右无左"<<show(t_rchild)<<endi;
				lower = true;
			}
			//左右子节点都存在
			else
			{
				//DEBUG_LOG<<"删除节点有右有左"<<show(position)<<endi;

				T_SHM_SIZE t_l_rchild = MoveBalance(position, lower);

				if (lower)
				{
					switch (TREE_NODE::at(t_l_rchild).bf)
					{
					case RH:
						if (TREE_NODE::at(TREE_NODE::at(t_l_rchild).hRight).bf != EH)
							lower = true;
						else
							lower = false;
						_RBalance(t_l_rchild);
						break;
					case EH:
						TREE_NODE::at(t_l_rchild).bf = RH;
						lower = false;
						break;
					case LH:
						TREE_NODE::at(t_l_rchild).bf = EH;
						lower = true;
						break;
					default:
						thelog << "_MoveBalance平衡因子出错" << show(t_l_rchild) << ende;
						break;
					}
				}
				//DEBUG_LOG<<show(t_l_rchild)<<endi;
				//DEBUG_LOG<<show(TREE_NODE::at(position).hRight)<<endi;

			}

			//TREE_NODE::at(position).hParent=-1;
			//TREE_NODE::at(position).hParent=tree_head->free_head
			//tree_head->free_head=position;
			TREE_NODE::at(position).hParent = tree_head->free_head;
			tree_head->free_head = position;
			TREE_NODE::at(position).hLeft = -1;
			TREE_NODE::at(position).hRight = -1;
			TREE_NODE::at(position).bf = EH;
			TREE_NODE::at(position).deleted = 1;

			--tree_head->size;
		}
		//删除节点 返回-1不成功 
		T_SHM_SIZE __erase(T_DATA const & tmp, T_SHM_SIZE vp, bool & lower, T_COMP & comp)
		{
			T_SHM_SIZE delete_position = -1;
			//没有对应节点
			if (vp < 0)
			{
				lower = false;
				return -1;
			}
			else
			{
				if (comp(TREE_NODE::at(vp).data, tmp))
				{
					//DEBUG_LOG<<"查右边"<<show(vp) <<endi;
					if ((delete_position = __erase(tmp, TREE_NODE::at(vp).hRight, lower, comp)) < 0)
					{
						lower = false;
						return -1;
					}
					if (lower)
					{
						switch (TREE_NODE::at(vp).bf)
						{
						case LH:
							//DEBUG_LOG<<"左边需要平衡" <<show(vp)<<endi;
							if (TREE_NODE::at(TREE_NODE::at(vp).hLeft).bf != EH)
								lower = true;
							else
								lower = false;
							_LBalance(vp);
							break;
						case EH:
							TREE_NODE::at(vp).bf = LH;
							lower = false;
							break;
						case RH:
							TREE_NODE::at(vp).bf = EH;
							lower = true;
							break;
						default:
							thelog << "erase 右边出错" << show(vp) << ende;
							break;
						}
					}
				}
				else if (comp(tmp, TREE_NODE::at(vp).data))
				{
					//DEBUG_LOG<<"查左边" << show(vp) <<endi;
					if ((delete_position = __erase(tmp, TREE_NODE::at(vp).hLeft, lower, comp)) < 0)
					{
						lower = false;
						return -1;
					}
					if (lower)
					{
						//DEBUG_LOG<<"删除节点后平衡修改" <<endi;
						switch (TREE_NODE::at(vp).bf)
						{
						case LH:
							TREE_NODE::at(vp).bf = EH;
							lower = true;
							break;
						case EH:
							TREE_NODE::at(vp).bf = RH;
							lower = false;
							break;
						case RH:
							//DEBUG_LOG<<"右边需要平衡" <<show(vp)<<endi;
							if (TREE_NODE::at(TREE_NODE::at(vp).hRight).bf != EH)
								lower = true;
							else
								lower = false;
							_RBalance(vp);
							break;
						default:
							thelog << "删除左边出错" << show(vp) << ende;
							break;
						}
					}
				}
				else
				{
					//DEBUG_LOG<<"找到删除节点"<<show(vp)<<endi;
					delete_position = vp;
					___erase(vp, lower);
					//lower=true;
				}
			}
			return delete_position;
		}
		//删除从根节点开始查起
		bool _erase(T_DATA const & tmp, T_COMP & comp)
		{
			string  report;
			//DEBUG_LOG<<"开始删除...."<<Report(report) <<endi;
			bool lower = false;
			T_SHM_SIZE delete_position = -1;
			if ((delete_position = __erase(tmp, tree_head->hHead, lower, comp)) < 0)
			{
				return false;
			}
			else
			{
				//DEBUG_LOG<<show(delete_position) <<endi;
				return true;
			}
		}
		//existH:数据已经放在节点数组中，只需要设置指针
		pair<iterator, bool> _insert(T_COMP & comp, T_DATA const & tmp, T_SHM_SIZE existH = -1)
		{
			//string  report;
			//thelog<<"插入...."<<Report(report) <<endi;
			//sleep(2);
			return __insert(tmp, comp);
		}
		string show(T_SHM_SIZE position)
		{
			char buf[2048];
			string tmp;
			TREE_NODE::at(position).toString(tmp);
			sprintf(buf, "%d %ld : %s", PI_N, position, tmp.c_str());
			return buf;
		}
		void update(T_SHM_SIZE position, T_DATA const & tmp)
		{
			TREE_NODE::at(position).data = tmp;
		}
	private:
		//insert时如果已经存在保存被覆盖的数据
		bool m_OldValueSeted;
		T_DATA m_OldValue;
	public:
		T_SHMSET_NO_MUTEX(char const * name, int version) :m_array(name, version) {}
		T_SHMSET_NO_MUTEX(char const * name) :m_array(name, VER) {}
		T_USER_HEAD * GetUserHead() { return &tree_head->user_head; }
		T_USER_HEAD const * GetUserHead()const { return &tree_head->user_head; }
		TREE_HEAD const * GetTreeHead()const { return tree_head; }
		string & Report(string & str)const
		{
			string tmpstr;
			str = "";
			char buf[2048];
			string tmp;
			str += "\n此结构为SET，使用率见下面信息（而不是后面的T_ARRAY信息）";
			sprintf(buf, "\nSET信息：%p head %ld,size %ld(%.2f%%),free_head %ld,begin %ld,sizeof(T) %ld", tree_head, tree_head->hHead, tree_head->size, 100.*tree_head->size / m_array.Capacity(), tree_head->free_head, begin().handle, sizeof(T_DATA));
			str += buf;
			sprintf(buf, "\nUSER_HEAD信息：%s", tree_head->user_head.toString(tmp).c_str());
			str += buf;
			str += m_array.ReportHead(tmp, true);

			CHtmlDoc::CHtmlTable2 table;
			table.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("h", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			const_iterator it;
			bool isTable = TREE_NODE::AddTableColumns(table);
			long maxsize = 10;
			long i;
			bool ed = false;
			for (i = 0, it = begin(); it != end(); ++i, ++it)
			{
				if (maxsize <= 0 || i < maxsize || i >= tree_head->size - maxsize)
				{
					if (!isTable)
					{
						sprintf(buf, "%2ld %p: ", i, &*it);
						str += buf;
						str += TREE_NODE::at(it.handle).toString(tmpstr);
						str += "\n";
					}
					else
					{
						table.AddLine();
						table.AddData(i);
						table.AddData(it.handle);
						TREE_NODE::at(it.handle).AddTableData(table);
					}
				}
				else
				{
					if (!ed)
					{
						if (!isTable)
						{
							str += "......\n";
						}
						else
						{
							table.AddLine();
							table.AddData("...");
						}
					}
					ed = true;

					//修正位置，直接跳到最后
					const_iterator tmp_rit = rbegin();
					i = tree_head->size - 1;
					while (i >= tree_head->size - maxsize)
					{
						--i;
						--tmp_rit;
					}
					it = tmp_rit;
				}
			}
			if (isTable)
			{
				str += table.MakeTextTable();
			}

			return str;
		}
		string & Report_virtual(string & str)const
		{
			str = "";
			char buf[2048];
			string tmp;
			str += "\n此结构为SET，使用率见下面信息（而不是后面的T_ARRAY信息）";
			sprintf(buf, "\nSET信息：%p head %ld,size %ld(%.2f%%),free_head %ld,begin %ld,sizeof(T) %ld", tree_head, tree_head->hHead, tree_head->size, 100.*tree_head->size / m_array.Capacity(), tree_head->free_head, begin().handle, sizeof(T_DATA));
			str += buf;
			sprintf(buf, "\nUSER_HEAD信息：%s", tree_head->user_head.toString(tmp).c_str());
			str += buf;
			str += m_array.Report3(tmp);
			return str;
		}
		//创建虚拟共享内存（用于在已经提供的地址上构造）
		bool set_CreateShm_virtual(char * head)
		{
			if (!m_array._CreateShm_virtual(head))return false;
			if (!set_AttachToShm_virtual(head)) return false;
			new(m_array.GetUserHead()) TREE_HEAD;
			//清空数组所有数据构建空闲列表
			clear();
			thelog << "创建虚拟set成功" << endi;
			return true;
		}
		bool set_CreateShm(long n)
		{
			if (!m_array._CreateShm(n))return false;
			if (!set_AttachToShm(false)) return false;
			new(m_array.GetUserHead()) TREE_HEAD;
			//清空数组所有数据构建空闲列表
			clear();
			thelog << "数据结构构造成功" << endi;
			if (!set_DetachFromShm()) return false;
			thelog << "创建成功" << endi;
			return true;
		}
		bool set_AttachToShm_virtual(char * head)
		{
			if (!m_array.AttachToShm_virtual(head))return false;
			tree_head = m_array.GetUserHead();

			return true;
		}
		bool set_AttachToShm(bool isReadOnly)
		{
			if (!m_array.AttachToShm(isReadOnly))return false;
			tree_head = m_array.GetUserHead();

			return true;
		}
		bool set_DetachFromShm()
		{
			if (!m_array.DetachFromShm())
			{
				return false;
			}

			tree_head = NULL;

			return true;
		}
		long Size()const { return tree_head->size; }
		long Capacity()const { return m_array.GetHead()->capacity; }
		void SetVirtualCapacity(long n) { m_array.SetVirtualCapacity(n); }
		const_iterator begin()const
		{
			const_iterator it;
			if (-1 == tree_head->hHead)it.handle = -1;
			else it.handle = TREE_NODE::at(tree_head->hHead)._begin();
			return it;
		}
		const_iterator end()const
		{
			const_iterator it;
			it.handle = -1;
			return it;
		}
		const_iterator rbegin()const
		{
			const_iterator it;
			if (-1 == tree_head->hHead)it.handle = -1;
			else it.handle = TREE_NODE::at(tree_head->hHead)._end();
			return it;
		}
		const_iterator rend()const
		{
			const_iterator it;
			it.handle = -1;
			return it;
		}
		bool Clear()
		{
			//new(tree_head) TREE_HEAD;
			m_array.Clear();
			return true;
		}
		bool Reserve(T_SHM_SIZE _n, T_SHM_SIZE min_block_n = 0)
		{
			if (!m_array.Reserve(_n, min_block_n))
			{
				thelog << (m_array.GetHead()->name).c_str() << " 空间已满，无法添加" << ende;
				return false;
			}
			tree_head = m_array.GetUserHead();
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
			TREE_NODE node;
			T_HANDLE h;

			node.data = data;
			
			return m_array.Add(node, h);
		}
		//设置数据大小，树结构的大小还没有设置
		bool FastRebuild_SetSize(long new_size)
		{
			return m_array.SetSize(new_size);
		}
		//获得数据大小，树结构的大小还没有设置
		long FastRebuild_GetSize()
		{
			return m_array.Size();
		}
		//计算n个子节点的树的层
		long FastRebuild__Layer(long n)
		{
			long layer = 0;
			long tmp = n;
			for (long i = 1; tmp > 0; i *= 2)
			{
				tmp -= i;
				++layer;
			}
			return layer;
		}
		//递归设置[from,to)
		void FastRebuild__SetNode(long from, long to, long hParent, T_SHM_SIZE* pParentLorR, long level = 0)
		{
			if (to - from <= 0)
			{
				*pParentLorR = -1;
				return;
			}

			//string str;
			//str.assign("    ", level);
			//cout << str << from << " " << to << " " << hParent << endl;
			long mid = (from + to) / 2;
			TREE_NODE * pMidNode = m_array.Get(mid);
			*pParentLorR = mid;
			pMidNode->hParent = hParent;
			//cout << str << "---------------- " << mid << endl;
			
			pMidNode->bf = FastRebuild__Layer(mid - from) - FastRebuild__Layer(to - (mid + 1));
			
			FastRebuild__SetNode(from, mid, mid, &pMidNode->hLeft, level + 1);
			FastRebuild__SetNode(mid + 1, to, mid, &pMidNode->hRight, level + 1);
		}
		struct less_Array
		{
			bool operator()(T_TREE_NODE_STRUCT<T_DATA > const& a, T_TREE_NODE_STRUCT<T_DATA > const& b)
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
				thelog << "level "<< m_FastRebuild_level <<"不符合，忽略 " << level << endi;
				return true;
			}

			string str;
			thelog << "数据总数 " << m_array.Size() << endi;
			thelog << endl << m_array.Report(str, true) << endi;

			bool bNeedSort = false;
			thelog << "检查是否需要排序。。。。。。" << endi;
			for (long i = 1; i < m_array.Size(); ++i)
			{
				if (*m_array.Get(i) < *m_array.Get(i-1))
				{
					bNeedSort = true;
					thelog << "需要排序" << endi;
					break;
				}
			}
			if (bNeedSort)
			{
				if (!m_array.IsOneBlock() && !noDisk)
				{
					thelog << "非单一块，排序之前必须先搞成单一块" << endi;
					this->SaveToDir("./");
					this->Detach();
					this->DestoryShm();
					this->LoadFromDir("./");
				}
				thelog << "排序。。。。。。" << endi;
				bool slow = (!m_array.IsOneBlock());
				less_Array less_array;
				if (slow)
				{
					m_array.Sort_slow(less_array);
					thelog << "慢速排序完成" << endi;
				}
				else
				{
					m_array.Sort_fast(less_array);
					thelog << "快速排序完成" << endi;
				}
				thelog << endl << m_array.Report(str, true) << endi;
			}
			else
			{
				thelog << "不需要排序" << endi;
			}
			
			thelog << "检查是否存在重复数据。。。。。。" << endi;
			long newsize = 0;
			long count_dup = 0;
			long count_del = 0;
			for (long i = 0; i < m_array.Size(); ++i)
			{
				TREE_NODE& data_cur = *m_array.Get(i);
				if (TREE_NODE::at(i).deleted)
				{
					++count_del;
					continue;
				}
				else
				{
					if (newsize > 0)
					{
						TREE_NODE& data_top = *m_array.Get(newsize - 1);
						if (!(data_top < data_cur || data_cur < data_top))
						{
							data_top = data_cur;//相同，后面的覆盖前面的
							++count_dup;
							continue;
						}
					}
					if (i != newsize)
					{
						*m_array.Get(newsize) = data_cur;//往前移
					}
					++newsize;
				}
			}
			thelog << "发现 " << count_del << " 个删除数据，已经用后面的覆盖前面的" << endi;
			thelog << "发现 " << count_dup << " 个重复数据，已经用后面的覆盖前面的" << endi;
			thelog << "原数据 " << m_array.Size() << " 个，现数据 " << newsize << " 个" << endi;
			FastRebuild_SetSize(newsize);

			thelog << "建立树结构。。。。。。" << endi;
			thelog << "总层数 " << FastRebuild__Layer(m_array.Size()) << endi;
			if (0 == m_array.Size())
			{
				thelog << "没有数据" << endi;
				return true;
			}
			FastRebuild__SetNode(0, m_array.Size(), -1, &this->tree_head->hHead);

			this->tree_head->free_head = -1;
			this->tree_head->size = m_array.Size();

			thelog << endl << m_array.Report(str, true) << endi;
			thelog << "最终数据总数 " << Size() << endi;
			
			//this->Report();
			return true;
		}

		//如果second为false则已经存在，发生了覆盖，用GetOldValue获得被覆盖的值
		pair<iterator, bool> insert(T_DATA const & data)
		{
			T_COMP comp;
			return insert(data, comp);
		}
		pair<iterator, bool> insert(T_DATA const & data, T_COMP & comp)
		{
			m_OldValueSeted = false;//清除被覆盖对象的有效标志

			pair<iterator, bool> ret;

			ret.first = end();
			ret.second = false;
			if (tree_head->free_head < 0 && m_array.Capacity() <= m_array.Size())
			{
				if (!Reserve(tree_head->size + 1))
				{
					return ret;
				}
			}

			try
			{
				ret = _insert(comp, data);
			}
			catch (exception_my & e)
			{
				thelog << e.what() << ende;
			}
			bool tmp = (ret.first != end());
			if (tmp && ret.second)
			{
				STATIC_G long last = 0;
				long percent = 100 * size() / capacity();
				if (percent >= 95 && percent != last)
				{
					thelog << "共享内存容量警告：" << (m_array.GetHead()->name).c_str() << " " << size() << "/" << capacity() << " (" << percent << "%)" << endw;
					last = percent;
				}
			}
			//thelog<<"insert ret "<<ret.first.handle<<" "<<ret.second<<endi;
			return ret;
		}
		//返回被覆盖的值，如果最近的操作没有发生覆盖则false
		bool GetOldValue(T_DATA & ret)const
		{
			if (m_OldValueSeted)
			{
				ret = m_OldValue;
				return true;
			}
			else
			{
				return false;
			}
		}
		template<typename T_FIND >
		const_iterator find(T_FIND const & tmp)const
		{
			T_COMP comp;
			const_iterator it = lower_bound<T_FIND >(tmp);
			if (it != end())
			{
				if (comp(*it, tmp) || comp(tmp, *it))return end();
			}
			return it;
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
		bool erase(const_iterator it)
		{
			T_COMP comp;
			return erase(it, comp);
		}
		//删除并指向下一个
		iterator & DeleteAndMoveNext(iterator & it)
		{
			if (end() == it)return it;

			iterator & ret = it;
			iterator tmp = ret;
			++ret;
			erase(tmp);
			return ret;
		}
		bool erase(const_iterator it, T_COMP & comp)
		{
			if (it.handle < 0)return true;
			if (end() == find(TREE_NODE::at(it.handle).data, comp))return true;
			//DEBUG_LOG<<"要删除节点"<<show(it.handle)<<endi;
			_erase(TREE_NODE::at(it.handle).data, comp);
			//if(!_check())
			//{
			//	G_SET_ERROR(My_ERR_ERROR);
			//	Report();
			//}
			return true;
		}
		bool erase(T_DATA const & data)
		{
			T_COMP comp;
			return erase(data, comp);
		}
		bool erase(T_DATA const & data, T_COMP & comp)
		{
			if (end() == find(data, comp))return true;
			_erase(data, comp);
			return true;
		}
		//用全比较函数，实际是find的功能
		template<typename T_FIND >
		const_iterator lower_bound(T_FIND const & tmp)const
		{
			T_COMP comp;
			const_iterator it;

			T_SHM_SIZE hNode = tree_head->hHead;
			it.handle = -1;
			while (-1 != hNode)
			{
				if (comp(tmp, TREE_NODE::at(hNode).data))
				{
					it.handle = hNode;
					hNode = TREE_NODE::at(hNode).hLeft;
				}
				else if (comp(TREE_NODE::at(hNode).data, tmp))
				{
					hNode = TREE_NODE::at(hNode).hRight;
				}
				else
				{
					it.handle = hNode;
					break;
				}
			}

			return it;
		}
		//用部分比较函数（但必须是符合顺序的，否则结果不可预期）
		template<typename T_FIND, typename T_LESS_BOUND >
		const_iterator lower_bound(T_FIND const & tmp, T_LESS_BOUND comp)const
		{
			const_iterator it;

			T_SHM_SIZE hNode = tree_head->hHead;
			it.handle = -1;
			while (-1 != hNode)
			{
				if (comp(tmp, TREE_NODE::at(hNode).data))
				{
					it.handle = hNode;
					hNode = TREE_NODE::at(hNode).hLeft;
				}
				else if (comp(TREE_NODE::at(hNode).data, tmp))
				{
					hNode = TREE_NODE::at(hNode).hRight;
				}
				else
				{
					it.handle = hNode;
					hNode = TREE_NODE::at(hNode).hLeft;
				}
			}

			return it;
		}
		//用部分比较函数（但必须是符合顺序的，否则结果不可预期）
		template<typename T_FIND, typename T_LESS_BOUND >
		const_iterator upper_bound(T_FIND const & tmp, T_LESS_BOUND comp)const
		{
			const_iterator it;

			T_SHM_SIZE hNode = tree_head->hHead;
			it.handle = -1;
			while (-1 != hNode)
			{
				if (comp(tmp, TREE_NODE::at(hNode).data))
				{
					it.handle = hNode;
					hNode = TREE_NODE::at(hNode).hLeft;
				}
				else if (comp(TREE_NODE::at(hNode).data, tmp))
				{
					hNode = TREE_NODE::at(hNode).hRight;
				}
				else
				{
					hNode = TREE_NODE::at(hNode).hRight;
				}
			}

			return it;
		}
		//检查一个节点是否已经删除，必须是有效参数
		bool IsDeleted(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).deleted;
		}
		//检查一个节点的标志（即使节点已经删除，标志位仍然可能有用）
		bool GetFlagInsert(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.getflag(0);
		}
		bool GetFlagUpdate(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.getflag(1);
		}
		bool GetFlagDelete(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.getflag(2);
		}
		//设置一个节点的标志（即使节点已经删除，标志位仍然可能有用）
		bool ClearFlagInsert(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.setflag(0, false);
		}
		bool ClearFlagUpdate(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.setflag(1, false);
		}
		bool ClearFlagDelete(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.setflag(2, false);
		}
		bool SetFlagInsert(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.setflag(0, true);
		}
		bool SetFlagUpdate(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.setflag(1, true);
		}
		bool SetFlagDelete(T_SHM_SIZE handle)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).flag.setflag(2, true);
		}
		//检查一个节点的标志（即使节点已经删除，标志位仍然可能有用）flag 0~7
		bool GetUserFlag(T_SHM_SIZE handle, int flag)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).uflag.getflag(flag);
		}
		//设置一个节点的标志（即使节点已经删除，标志位仍然可能有用）flag 0~7
		bool SetUserFlag(T_SHM_SIZE handle, int flag, bool value)const
		{
			if (handle < 0 || static_cast<size_t>(handle) >= m_array.capacity())return false;
			return TREE_NODE::at(handle).uflag.setflag(flag, value);
		}
		//注意，数据可能是已经被删除的，此时需要慎重考虑data的可用性，比如指向的别的数据可能已经不存在了
		bool ForEachFlag(long & handle, IForEachFlag<T_DATA > * pForEach)const
		{
			typename T_SETARRAY::HANDLE h;
			if (handle >= 0)
			{
				thelog << "续传模式 " << handle << endi;
				h.handle = handle;//续传位置
			}
			else
			{
				thelog << "全量模式 " << handle << endi;
				h = m_array.Begin();
			}
			for (; h.handle < m_array.Size(); ++h)//不加锁用Size比End安全
			{
				handle = h.handle;
				pForEach->set(h->deleted, &h->flag, &h->uflag);
				if (!pForEach->doOneData(h.handle, &h->data))
				{
					return false;
				}
			}
			return true;
		}
	public:
		//IShmActiveObject接口
		virtual char const * GetName()const { return m_array.GetName(); }
		virtual int GetPart()const { return PART; }//获得共享内存的模板参数PART
		virtual int GetPI()const { return PI_N; };//获得共享内存的模板参数PI_N
		virtual bool isPrivateMem()const { return m_array.IsPrivate(); }
		virtual bool isConnected()const { return m_array.IsConnected(); }
		virtual bool isReadOnly()const { return m_array.IsReadOnly(); }
		virtual bool CreateShm()
		{
			return set_CreateShm(0);
		}
		virtual bool CreatePrivate()
		{
			if (!m_array.CreatePrivate())return false;
			new(m_array.GetUserHead()) TREE_HEAD;
			tree_head = m_array.GetUserHead();
			//清空数组所有数据构建空闲列表
			clear();
			thelog << "数据结构构造成功" << endi;
			thelog << "创建成功" << endi;
			return true;
		}
		virtual bool _Attach(bool isReadOnly)
		{
			return set_AttachToShm(isReadOnly);
		}
		virtual bool Detach()
		{
			return set_DetachFromShm();
		}
		virtual bool LoadFromDir(char const * dir_name)
		{
			if (m_array.LoadFromDir(dir_name))
			{
				tree_head = m_array.GetUserHead();
				return true;
			}
			else
			{
				return false;
			}
		}
		virtual bool LoadPrivateFromDir(char const * dir_name)
		{
			if (m_array.LoadPrivateFromDir(dir_name))
			{
				tree_head = m_array.GetUserHead();
				return true;
			}
			else
			{
				return false;
			}
		}
		virtual bool DestoryShm()
		{
			if (m_array.DestoryShm())
			{
				tree_head = NULL;
				return true;
			}
			else
			{
				return false;
			}
		}
		virtual bool DestoryPrivate()
		{
			if (m_array.DestoryPrivate())
			{
				tree_head = NULL;
				return true;
			}
			else
			{
				return false;
			}
		}
		virtual bool SaveToDir(char const * dir_name)const
		{
			return m_array.SaveToDir(dir_name);
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			string file = dir_name;
			if (file.size() > 0 && file[file.size() - 1] != '/')file += "/";
			file += this->GetName();
			char buf[256];
			if (PART > 0)
			{
				sprintf(buf, "_%02d", PART);
				file += buf;
			}
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
			thelog << "共 " << this->size() << " 条" << endi;
			for (const_iterator it = begin(); it != end(); ++it)
			{
				(*it).toString(str);
				str += "\n";
				f.write(str.c_str(), str.size());
				if (!f.good())
				{
					thelog << "写文件失败 " << file << ende;
					return false;
				}
				++count;
				if (0 == count % 100000)thelog << "导出 " << count << " 条" << endi;
			}
			thelog << "导出完成，共 " << count << " 条" << endi;

			f.close();
			return true;
		}
		virtual bool Report()const
		{
			if (!m_array.IsConnected())
			{
				thelog << "未连接" << ende;
				return false;
			}

			string str;
			thelog << Report(str) << endi;
			return true;
		}
		virtual bool clear()
		{
			return Clear();
		}
		virtual bool check()const
		{
			return _check();
		}
		virtual bool repair(char const* level)
		{
			return _repair(level);
		}
		size_t size()const override
		{
			return Size();
		}
		size_t capacity()const override
		{
			return Capacity();
		}
		size_t byte_capacity()const override
		{
			return m_array.byte_capacity();
		}
		size_t record_length()const override
		{
			return m_array.record_length();
		}
		size_t byte_size()const override
		{
			return m_array.byte_size();
		}
		size_t block_count()const override
		{
			return m_array.block_count();
		}
	};
	//简化定义的版本
	template<typename T_DATA, int PI_N, typename T_COMP >
	class T_SHMSET_NO_MUTEX_2 : public T_SHMSET_NO_MUTEX<T_DATA, PI_N, CDemoData, 0, 0, T_COMP, T_HANDLE_ARRAY<T_TREE_NODE_STRUCT<T_DATA >, PI_N > >
	{
	private:
		typedef T_SHMSET_NO_MUTEX<T_DATA, PI_N, CDemoData, 0, 0, T_COMP, T_HANDLE_ARRAY<T_TREE_NODE_STRUCT<T_DATA >, PI_N > > T_PARENT;
	public:
		T_SHMSET_NO_MUTEX_2(char const * name, int version) :T_PARENT(name, version) {}
		T_SHMSET_NO_MUTEX_2(char const * name) :T_PARENT(name, 0) {}
	};

	//带有ISet接口的
	template<typename T_DATA, int PI_N, typename T_USER_HEAD = CDemoData, int PART = 0, int VER = 0, typename T_HANDLE = T_HANDLE_ARRAY<T_TREE_NODE_STRUCT<T_DATA >, PI_N >, typename T_COMP = less<T_DATA> >
	class T_SHMSET_NO_MUTEX_ISET : public T_SHMSET_NO_MUTEX<T_DATA, PI_N, T_USER_HEAD, PART, VER, T_COMP, T_HANDLE >, public ISet<T_DATA>
	{
	private:
		typedef T_SHMSET_NO_MUTEX<T_DATA, PI_N, T_USER_HEAD, PART, VER, T_COMP, T_HANDLE > T_PARENT;
	public:
		typedef typename T_PARENT::iterator iterator;
		typedef typename T_PARENT::T_SETARRAY T_SETARRAY;
		using T_PARENT::begin;
		using T_PARENT::find;
		using T_PARENT::erase;
		using T_PARENT::insert;
		using T_PARENT::lower_bound;
		using T_PARENT::m_array;
		T_SHMSET_NO_MUTEX_ISET(char const * name, int version) :T_PARENT(name, version) {}
		T_SHMSET_NO_MUTEX_ISET(char const * name) :T_PARENT(name, 0) {}
	public://ISet接口
		virtual T_DATA * isetGet(long h)const
		{
			iterator it;
			it.handle = h;
			return &*it;
		}
		virtual long isetMoveNext(long & h)const
		{
			iterator it;
			it.handle = h;
			++it;
			return h = it.handle;
		}
		virtual long isetBegin()const
		{
			return begin().handle;
		}
		virtual pair<long, bool> isetInsert(T_DATA const & value)
		{
			pair<long, bool> ret;
			pair<iterator, bool> tmppair = insert(value);
			ret.first = tmppair.first.handle;
			ret.second = tmppair.second;
			return ret;
		}
		virtual long isetFind(T_DATA const & value)const
		{
			return find(value).handle;
		}
		virtual long isetFindLowerBound(T_DATA const & value, bool(*less)(T_DATA const &, T_DATA const &))const
		{
			return lower_bound(value, less).handle;
		}
		virtual bool isetErase(long h)
		{
			iterator it;
			it.handle = h;
			return erase(it);
		}
		virtual bool isetForEachShuffle(long handle, typename ISet<T_DATA >::ISetForEach * pForEach)const
		{
			typename T_SETARRAY::HANDLE h;
			if (handle >= 0)
			{
				thelog << "续传模式 " << handle << endi;
				h.handle = handle;//续传位置
			}
			else
			{
				thelog << "全量模式 " << handle << endi;
				h = m_array.Begin();
			}
			for (; h.handle < m_array.Size(); ++h)//不加锁用Size比End安全
			{
				if (h->deleted)continue;
				if (!pForEach->doOneData(h.handle, &h->data))
				{
					return false;
				}
			}
			return true;
		}
	};

}
