//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//
#include "myhttpd.h"

//启动服务，如果带有“-stop”参数则关闭集群
int start_httpd(vector<CWebCommand* > const& ws, char const* server_name, int argc, char** argv)
{
	string str;

	long port = 10000;
	char cwd[1024];
	getcwd(cwd, 1024);
	string root = cwd;
	if (root[root.size() - 1] != '/')root += "/";
	root += "wwwroot/";

	if (GetCommandParam(argc, argv, "-port", str))
	{
		port = atol(str.c_str());
	}
	if (GetCommandParam(argc, argv, "-root", str))
	{
		root = str;
	}
	ShellEnvReplace(root);//这一句会产生子进程，因此会收到SIGCHLD信号

	if (port < 1 || port>65535)
	{
		thelog << "无效的端口号，端口号必须为1-65535" << ende;
		return __LINE__;
	}

	if (0 == root.size())root = "/";
	if ('/' != root[root.size() - 1])root += "/";
	if (root[0] != '/')
	{
		thelog << "根目录必须以'/'开始" << ende;
		return __LINE__;
	}

	if (!CUserManager::getInstPtr()->InitUserManager())
	{
		thelog << "初始化用户管理失败" << ende;
		return __LINE__;
	}

	pid_t pid;

	CHttpClient client;
	if (client.Connect("127.0.0.1", port + 1))
	{
		thelog << "端口被占用" << port << ende;
		client.Close();
		return My_ERR_PORT_OCCUPY;
	}
	client.Close();
	if ((pid = fork()) < 0)
	{
		thelog << "fork error" << ende;
		return 1;
	}
	else if (pid != 0)
	{
		thelog << "子进程启动成功 pid " << pid << endi;
		//父进程，等待端口启动即告成功
		time_t t1 = time(NULL);
		while (time(NULL) - t1 <= 60)
		{
			SleepSeconds(5);
			if (client.Connect("127.0.0.1", port + 1))
			{
				thelog << "端口成功连接，服务已启动 主进程" << getpid() << "退出" << port << endi;
				client.Close();
				exit(0);
			}
		}
		thelog << "服务启动超时，请检查 " << port << ende;
		exit(My_ERR_START_TIMEOUT);
	}
	else
	{
		thelog << "子进程pid " << getpid() << endi;
		//子进程
		thelog << "start server ..." << endi;
		CServer server;
		server.run(ws, server_name, port, root, true);
		thelog << "服务已结束" << endi;

		return 0;
	}

	return 0;//永远不会执行到这里
}

int _main(int argc, char** argv)
{
	CWebCommand demoasp;
	vector<CWebCommand* >  ws;
	ws.push_back(&demoasp);
	return start_httpd(ws, "ct嵌入式WEB服务器", argc, argv);
}
int main_SSLTLS();
int main(int argc, char** argv)
{
	if (!InitActiveApp("myhttpd", 1024 * 1024, argc, argv))exit(1);
	//return main_SSLTLS();
	thelog << "程序版本：2024.03.07 08:41" << endi;
	thelog << "pid " << getpid() << endi;
	ShowHttpdVersion();

	if (sizeof(long) != 8)
	{
		thelog << "非64位程序！" << ende;
		return 1;
	}

	G_IS_DEBUG = true;
	return __all_sig_catch(argc, argv, _main);
}
