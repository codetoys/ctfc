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
