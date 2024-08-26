//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "othertest.h"

using namespace ns_my_std;

int main(int argc,char ** argv)
{
	if (!InitActiveApp("othertest", 0, argc, argv))return __LINE__;

	G_IS_DEBUG = true;

	int ret = 0;

	CCommandSet commandset;
	TestGmSSL testgmssl;
	do
	{
		commandset.AddCommand("a", &testgmssl);

		ret=commandset.doCommandSet("a");
	}while(false);

	ExitActiveApp(ret);
}
