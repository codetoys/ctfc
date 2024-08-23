//CodeTemplate_t.cpp 代码模板系统
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "CodeTemplate.h"
int main(int argc, char ** argv)
{
	if (!InitActiveApp("CodeTemplate", 1024 * 1024, argc, argv))exit(1);

	if (sizeof(long) != 8)
	{
		thelog << "非64位程序！" << ende;
	}

	//bool loop = true;
	G_IS_DEBUG = true;

	{
		CCodeTemplate ct;
		CCTObject O;
		CCTStack S;
		stringstream ss;

		O.SetObjectAddProperty("sys", "aaaaaaaaaaaaa\n");
		S.Push();

		if (!ct.ProcessTemplate("simplesample.ct", ss, O, S))
		{
			thelog << "执行失败" << ende;
		}
		thelog <<"========================="<< endl << ss.str() << endi;
	}

	thelog << "程序退出" << endi;
	return 0;
}
