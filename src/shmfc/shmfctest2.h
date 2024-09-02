//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once


#include <sstream>
#include "../env/env.h"
#include "shmArray.h"

class CTestT_ARRAY
{
public:
	struct DemoData : public CActiveObjectBase
	{
		long n = 0;
		sstring<8> s;

		//用于需要排序的场合
		bool operator < (CDemoData const& tmp)const { return n < tmp.n; }
		//某些场合也需要等于
		bool operator == (CDemoData const& tmp)const { return n == tmp.n; }

		friend ostream& operator << (ostream& o, DemoData const& d)
		{
			return o << d.n << " " << d.s.c_str();
		}
		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return n; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "%ld %s", n, s.c_str());
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("N", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("S", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(n);
			table.AddData(s.c_str());
			return true;
		}
	};

	static int test_T_ARRAY(int argc, char** argv)
	{
		T_ARRAY<DemoData, PI_TEST_1, CDemoData > a("test", 1);
		a.DestoryShm();
		if (!a.CreateShm())return __LINE__;
		if (!a.Attach(false))return __LINE__;
		for (int i = 0; i < 100; ++i)
		{
			DemoData tmp;
			tmp.n = 10000 + i;
			tmp.s = "abc";
			T_ARRAY<DemoData, PI_TEST_1, CDemoData >::HANDLE h;
			a.Add(tmp, h);
			thelog << h.handle << endi;
		}
		for (int i = 0; i < a.Size(); ++i)
		{
			string str;
			thelog << "第" << i << "个数据：" << a.Get(i)->toString(str) << endi;
		}
		a.RunCmdUI();

		return 0;
	}
};

#include "shmSet_NoMutex.h"
class CTest_T_SHMSET_NO_MUTEX
{
public:
	struct DemoData : public CActiveObjectBase
	{
		long n = 0;
		sstring<8> s;

		//用于需要排序的场合
		bool operator < (DemoData const& tmp)const { return n < tmp.n; }
		//某些场合也需要等于
		bool operator == (DemoData const& tmp)const { return n == tmp.n; }

		friend ostream& operator << (ostream& o, DemoData const& d)
		{
			return o << d.n << " " << d.s.c_str();
		}
		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return n; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "%ld %s", n, s.c_str());
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("N", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("S", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(n);
			table.AddData(s.c_str());
			return true;
		}
	};

	static int test_T_SHMSET_NO_MUTEX(int argc, char** argv)
	{
		T_SHMSET_NO_MUTEX<DemoData, PI_TEST_1, CDemoData > a("test", 1);
		a.DestoryShm();
		if (!a.CreateShm())return __LINE__;
		if (!a.Attach(false))return __LINE__;
		for (int i = 0; i < 10; ++i)
		{
			DemoData tmp;
			tmp.n = rand() % 10;
			tmp.s = "abc";
			thelog << i << " n:" << tmp.n << " handle:" << a.insert(tmp).first.handle << endi;
		}
		for (T_SHMSET_NO_MUTEX<DemoData, PI_TEST_1, CDemoData >::const_iterator it = a.begin(); it!=a.end(); ++it)
		{
			string str;
			thelog << it->toString(str) << endi;
		}
		for (int i = 0; i < 10; ++i)
		{
			DemoData tmp;
			tmp.n = i;
			T_SHMSET_NO_MUTEX<DemoData, PI_TEST_1, CDemoData >::const_iterator it = a.find(tmp);
			string str;
			if(it!=a.end())thelog << i << " 找到 handle:" << it.handle <<" "<<it->toString(str) << endi;
			else thelog << i << " 没找到 handle:" << it.handle << endi;
		}
		a.RunCmdUI();

		return 0;
	}
};

#include "shmSet_Mutex.h"
class CTest_T_SHMSET
{
public:
	struct DemoData : public CActiveObjectBase
	{
		typedef DemoData T_ME;
		long n = 0;
		sstring<8> s;

		//用于需要排序的场合
		bool operator < (T_ME const& tmp)const { return n < tmp.n; }
		//某些场合也需要等于
		bool operator == (T_ME const& tmp)const { return n == tmp.n; }

		friend ostream& operator << (ostream& o, T_ME const& d)
		{
			return o << d.n << " " << d.s.c_str();
		}
		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return n; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "%ld %s", n, s.c_str());
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("N", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("S", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(n);
			table.AddData(s.c_str());
			return true;
		}
	};
	typedef T_SHMSET<DemoData, PI_TEST_1, CDemoData > T_CONTINER;//迭代器和数据操作都是互斥的，但连接断开等管理类操作不是互斥的（通常也不应该并发操作）
	//typedef T_SHMSET_MUTEX<DemoData, PI_TEST_1, CDemoData > T_CONTINER;//用这个也可以，但只有M开头的操作才是互斥的，定义在shmSet.h，名字有点混乱
	static int test_T_SHMSET(int argc, char** argv)
	{
		T_CONTINER a("test", 1);
		a.DestoryShm();
		if (!a.CreateShm())return __LINE__;
		thelog << endi;
		if (!a.Attach(false))return __LINE__;
		thelog << endi;
		for (int i = 0; i < 10; ++i)
		{
			DemoData tmp;
			tmp.n = rand() % 10;
			tmp.s = "abc";
			thelog << i << " n:" << tmp.n << " handle:" << a.insert(tmp).first.handle << endi;
		}
		for (T_CONTINER::const_iterator it = a.begin(); it != a.end(); ++it)
		{
			string str;
			thelog << it->toString(str) << endi;
		}
		for (int i = 0; i < 10; ++i)
		{
			DemoData tmp;
			tmp.n = i;
			T_CONTINER::const_iterator it = a.find(tmp);
			string str;
			if (it != a.end())thelog << i << " 找到 handle:" << it.handle << " " << it->toString(str) << endi;
			else thelog << i << " 没找到 handle:" << it.handle << endi;
		}
		a.RunCmdUI();

		return 0;
	}
};

class CTest_T_SHMSET_lower_bound
{
public:
	struct DemoData : public CActiveObjectBase
	{
		typedef DemoData T_ME;
		long a = 0;
		long b = 0;
		long c = 0;
		sstring<8> s;

		//用于需要排序的场合
		bool operator < (T_ME const& tmp)const 
		{
			return a == tmp.a ? (b == tmp.b ? c < tmp.c : b < tmp.b) : a < tmp.a;
		}
		//部分比较函数
		static bool Less_A(T_ME const& tmp1, T_ME const& tmp2)
		{
			return tmp1.a < tmp2.a;
		}
		//某些场合也需要等于
		bool operator == (T_ME const& tmp)const { return !(*this<tmp) && !(tmp<*this); }

		friend ostream& operator << (ostream& o, T_ME const& d)
		{
			return o << d.a << " " << d.b << " " << d.c << " " << d.s.c_str();
		}
		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return a; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "%ld %ld %ld %s", a, b, c, s.c_str());
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("A", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("B", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("C", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("S", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(a);
			table.AddData(b);
			table.AddData(c);
			table.AddData(s.c_str());
			return true;
		}
	};
	typedef T_SHMSET<DemoData, PI_TEST_1, CDemoData > T_CONTINER;//迭代器和数据操作都是互斥的，但连接断开等管理类操作不是互斥的（通常也不应该并发操作）
	//typedef T_SHMSET_MUTEX<DemoData, PI_TEST_1, CDemoData > T_CONTINER;//用这个也可以，但只有M开头的操作才是互斥的，定义在shmSet.h，名字有点混乱
	static int test_T_SHMSET_lower_bound(int argc, char** argv)
	{
		T_CONTINER a("test", 1);
		a.DestoryShm();
		if (!a.CreateShm())return __LINE__;
		thelog << endi;
		if (!a.Attach(false))return __LINE__;
		thelog << endi;
		for (int i = 0; i < 10; ++i)
		{
			DemoData tmp;
			tmp.a = rand() % 2;
			tmp.b = rand() % 2;
			tmp.c = i;
			tmp.s = "abc";
			thelog << i << " n:" << tmp << " handle:" << a.insert(tmp).first.handle << endi;
		}
		for (T_CONTINER::const_iterator it = a.begin(); it != a.end(); ++it)
		{
			string str;
			thelog << it->toString(str) << endi;
		}
		for (int i = 0; i < 10; ++i)
		{
			DemoData tmp;
			tmp.a = i;
			T_CONTINER::const_iterator it_from = a.lower_bound(tmp, DemoData::Less_A);
			T_CONTINER::const_iterator it_to = a.upper_bound(tmp, DemoData::Less_A);
			string str;
			if (it_from != a.end())
			{
				thelog << "-----------------" << i << " 找到" << endi;
				for (T_CONTINER::const_iterator it = it_from; it != it_to; ++it)
				{
					thelog << it->toString(str) << endi;
				}
			}
			else thelog << "-----------------" << i << " 没找到" << endi;
		}
		a.RunCmdUI();

		return 0;
	}
};

#include "shmStringPool2.h"
class CTest_StringPool2x
{
public:
	typedef StringPool2x<CTest_StringPool2x, PI_TEST_1, PI_TEST_2, CDemoData > T_CONTINER;
	static int test_StringPool2x(int argc, char** argv)
	{
		T_CONTINER a("test", 1);
		a.DestoryShm();
		if (!a.CreateShm())return __LINE__;
		thelog << endi;
		if (!a.Attach(false))return __LINE__;
		thelog << endi;
		vector<T_CONTINER::HANDLE > v_handles;
		for (int i = 0; i < 10; ++i)
		{
			T_CONTINER::HANDLE h;
			char buf[256];
			sprintf(buf, "abc%d", (5 == i ? 6 : i));
			if (!a.AddString(buf, h))thelog << "添加失败 " << i << " : " << buf << ende;
			else
			{
				thelog << "添加成功 " << i << " : " << buf << ende;
				v_handles.push_back(h);
			}
		}
		for (vector<T_CONTINER::HANDLE >::const_iterator it = v_handles.begin(); it != v_handles.end(); ++it)
		{
			string str;
			thelog << (*it).handle << " : " << a.GetString(*it) << endi;
		}

		a.RunCmdUI();

		return 0;
	}
};
