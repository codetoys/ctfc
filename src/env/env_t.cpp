﻿//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "env.h"
#include "CommandSet.h"

using namespace ns_my_std;

int main(int argc,char ** argv)
{
	if (!InitActiveApp("myenv", 0, argc, argv))return __LINE__;

	G_IS_DEBUG=true;
	//theLog.Open("myenv.log");
	//abort();
	//string a,b,c;
	//GetDBConnectInfo(a,b,c);
	
	thelog<<"测试"<<endi;
	DEBUG_LOG<<"测试"<<endi;
	DEBUG_LOG<<"测试"<<endi;
	DEBUG_LOG<<"测试"<<endi;
	DEBUG_LOG<<"测试"<<endi;
	DEBUG_LOG<<"测试"<<endi;

	thelog << "__cplusplus " << __cplusplus << endi;

	G_IS_DEBUG = true;

	int ret = 0;

	CCommandSet commandset;
	do
	{
		//if (!CShmEnvMgr::AddToCommandSet(commandset))
		//{
		//	ret = __LINE__;
		//	break;
		//}
		//if (!CEnvTools::AddToCommandSet(commandset))
		//{
		//	ret = __LINE__;
		//	break;
		//}

		ret=commandset.doCommandSet("201");
	}while(false);

	ExitActiveApp(ret);
}
