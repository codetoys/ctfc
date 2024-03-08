//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "myhttpd.h"
#include "myhttpserver.h"
#include "myUserManager.h"

char const * INI_SECITON = "not set";

void ShowHttpdVersion()
{
	thelog << endl
		<< endi;
}
class CServer
{
public:
	void run(vector<CWebCommand * > const & ws, char const * _server_name, unsigned short portnum, string const & root)
	{
		if (!CPrivateShmMgr::getInstPtr()->CreateShm(sizeof(SocketServerControlBlock)))
		{
			thelog << "未能创建服务控制块" << ende;
			return;
		}
		new(CPrivateShmMgr::getInstPtr()->getShmAddr())SocketServerControlBlock;
		SocketServerData * pSSD = (SocketServerData*)CPrivateShmMgr::getInstPtr()->getShmAddr();
		CHttpServerDatas * pHSD = &((SocketServerControlBlock*)CPrivateShmMgr::getInstPtr()->getShmAddr())->server_data;

		CMyShmMutex mutex;//用于HTTP服务的互斥对象，必须在CHttpProcessInit之前创建被记录到共享内存

		if (!mutex.Create(&pHSD->m_webcommand_sem))
		{
			LOG << "创建信号量失败" << ende;
			return;
		}

		string server_name = _server_name;
		string realm = server_name;

		thelog << "站点标识 " << realm << endi;

		CHttpProcess process;
		if (!process.CHttpProcessInit(realm + " 请登录"))
		{
			thelog << "CHttpProcess初始化失败" << ende;
			return;
		}

		process.SetRoot(root.c_str());
		for (vector<CWebCommand * >::const_iterator it = ws.begin(); it != ws.end(); ++it)
		{
			if (process.AddWebCommand(*it))
			{
				thelog << "页面:" << typeid(*it).name() << " " << (*it)->command_id << " 安装成功!" << endi;
			}
			else
			{
				thelog << "页面:" << typeid(*it).name() << " " << (*it)->command_id << " 安装失败!" << ende;
				return;
			}
		}
		process.SetCheckUser(CUserManager::CheckUser);
		process.SetCheckAdmin(CUserManager::CheckAdminPassword);
		process.SetSiteName(server_name.c_str());

		CSocketServer svr;
		if (svr.Start(pSSD, server_name.c_str(), portnum, &process, 0, &process))
		{
			//控制转入服务器
		}
		else
		{
			thelog << "服务启动失败" << ende;
		}
		mutex.Destory();
		CPrivateShmMgr::getInstPtr()->DestoryShm();
		thelog << "服务结束" << endi;
		return;
	}

};

int start_httpd(char const * ini_section, vector<CWebCommand * > const & ws, char const * server_name, int argc, char ** argv)
{
	//必须首先设置INI节
	INI_SECITON=ini_section;

	string str;

	long port= CSystemParam::SystemParam_GetLong(INI_SECITON, "port", 10000);
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
		root=str;
	}
	ShellEnvReplace(root);

	if(port<1 || port>65535)
	{
		thelog<<"无效的端口号，端口号必须为1-65535"<<ende;
		return __LINE__;
	}

	if(0==root.size())root="/";
	if('/'!=root[root.size()-1])root+="/";
	if(root[0]!='/')
	{
		thelog<<"根目录必须以'/'开始"<<ende;
		return __LINE__;
	}

	if (!CUserManager::getInstPtr()->InitUserManager())
	{
		thelog<<"初始化用户管理失败"<<ende;
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
		//父进程，等待端口启动即告成功
		time_t t1 = time(NULL);
		while (time(NULL) - t1 <= 60)
		{
			SleepSeconds(5);
			if (client.Connect("127.0.0.1", port + 1))
			{
				thelog << "端口成功连接，服务已启动 " << port << endi;
				client.Close();
				exit(0);
			}
		}
		thelog << "服务启动超时，请检查 " << port << ende;
		exit(My_ERR_START_TIMEOUT);
	}
	else
	{
		//子进程
		thelog << "start server ..." << endi;
		CServer server;
		server.run(ws, server_name, port, root);
		thelog << "服务已结束" << endi;

		return 0;
	}

	return 0;//永远不会执行到这里
}
