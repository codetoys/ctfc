//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//
#include "myhttpd.h"

int _main(int argc, char** argv)
{
	CWebCommand demoasp;
	vector<CWebCommand* >  ws;
	ws.push_back(&demoasp);
	return start_httpd("myhttpd", ws, "ct嵌入式WEB服务器", argc, argv);
}
int main(int argc, char** argv)
{
	if (!InitActiveApp("myhttpd", 1024 * 1024, argc, argv))exit(1);
	//theLog.Open("myhttpd.log");
	thelog << "程序版本：2024.03.07 08:41" << endi;
	ShowHttpdVersion();

	if (sizeof(long) != 8)
	{
		thelog << "非64位程序！" << ende;
		return 1;
	}

	G_IS_DEBUG = true;
	return __all_sig_catch(argc, argv, _main);
}
