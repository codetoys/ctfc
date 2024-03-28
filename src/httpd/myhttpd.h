//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/myUtil.h" 
using namespace ns_my_std;
#include "myhttpbase.h"
#include "myhttpserver.h"
#include "myUserManager.h"

void ShowHttpdVersion();

class CServer
{
public:
	void run(vector<CWebCommand* > const& ws, char const* _server_name, unsigned short portnum, string const& root, bool demon)
	{
		if (!CPrivateShmMgr::getInstPtr()->CreateShm(sizeof(SocketServerControlBlock)))
		{
			thelog << "未能创建服务控制块" << ende;
			return;
		}
		new(CPrivateShmMgr::getInstPtr()->getShmAddr())SocketServerControlBlock;
		SocketServerData* pSSD = (SocketServerData*)CPrivateShmMgr::getInstPtr()->getShmAddr();
		CHttpServerDatas* pHSD = &((SocketServerControlBlock*)CPrivateShmMgr::getInstPtr()->getShmAddr())->server_data;

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
		for (vector<CWebCommand* >::const_iterator it = ws.begin(); it != ws.end(); ++it)
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
		if (svr.Start(pSSD, server_name.c_str(), portnum, &process, 0, &process, demon))
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
