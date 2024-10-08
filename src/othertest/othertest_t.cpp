﻿//
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
	TestGmSSL_aes testgmssl_aes;
	TestGmSSL_sm3 testgmssl_sm3;
	TestGmSSL_sm4 testgmssl_sm4;
	do
	{
		commandset.AddCommand("a", &testgmssl);
		commandset.AddCommand("aes", &testgmssl_aes);
		commandset.AddCommand("3", &testgmssl_sm3);
		commandset.AddCommand("4", &testgmssl_sm4);

		ret = commandset.doCommandSet("a");
	}while(false);

	ExitActiveApp(ret);
}
