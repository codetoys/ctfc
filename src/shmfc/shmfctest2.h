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
