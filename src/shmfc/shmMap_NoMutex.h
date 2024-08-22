//shm_Map_NoMutex.h 共享内存Map容器 不带互斥的基本版本
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
#include "shmSet_NoMutex.h"

namespace ns_my_std
{
	template<typename T_KEY, typename T_VALUE >
	class T_MAP_PAIR
	{
	public:
		T_KEY const first;
		T_VALUE second;

		T_MAP_PAIR(){}
		T_MAP_PAIR(T_KEY const key) :first(key){}
		T_MAP_PAIR(T_MAP_PAIR const & tmp) :first(tmp.first), second(tmp.second) {}
		T_MAP_PAIR(T_KEY const key, T_VALUE const value) :first(key), second(value) {}
		bool operator <(T_MAP_PAIR const & tmp)const { return first < tmp.first; }
		T_MAP_PAIR & operator =(T_MAP_PAIR const tmp)
		{
			new((T_MAP_PAIR*)(void *)this)T_MAP_PAIR(tmp);
			return *this;
		}

		//用于输出数据的场合
		string & toString(string & str)const
		{
			str = "";
			string tmp;
			str += first.toString(tmp);
			str += " ";
			str += second.toString(tmp);
			return str;
		}

		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{

			return T_KEY::AddTableColumns(table) && T_VALUE::AddTableColumns(table);
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			return first.AddTableData(table) && second.AddTableData(table);
		}
	};
	template<typename T_KEY, typename T_VALUE, int PI_N, typename T_USER_HEAD = CDemoData >
	class T_SHMMAP_NO_MUTEX : public T_SHMSET_NO_MUTEX<T_MAP_PAIR<T_KEY const, T_VALUE >, PI_N, T_USER_HEAD >
	{
	public:
		typedef T_MAP_PAIR<T_KEY const, T_VALUE > value_type;
		typedef T_SHMSET_NO_MUTEX<T_MAP_PAIR<T_KEY const, T_VALUE >, PI_N, T_USER_HEAD > T_PARENT;
		using T_PARENT::find;
	public:
		T_SHMMAP_NO_MUTEX(char const * name, int version) :T_PARENT(name, version) {}
		T_SHMMAP_NO_MUTEX(char const * name) :T_PARENT(name, 0) {}

		typename T_PARENT::iterator find(T_KEY const & key)const
		{
			return find(value_type(key));
		}
	};

}
