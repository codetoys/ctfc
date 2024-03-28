//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"

#define ADMIN_NAME (CUserManager::getInstPtr()->admin_name)
#define ADMIN_PASS (CUserManager::getInstPtr()->admin_password)
#define APPUSER_NAME (CUserManager::getInstPtr()->appuser_name)
#define APPUSER_PASS (CUserManager::getInstPtr()->appuser_password)
#define USER_NAME (CUserManager::getInstPtr()->user_name)
#define USER_PASS (CUserManager::getInstPtr()->user_password)

namespace ns_my_std
{
	class CUserManager
	{
		DECLARE_SINGLETON(CUserManager);
	public:
		//管理员，用于内部连接和管理服务
		string admin_name;
		string admin_password;
		//普通用户，用于观察
		string user_name;
		string user_password;
		//数据用户，用于数据客户端（APP）
		string appuser_name;
		string appuser_password;

	public:
		bool InitUserManager()
		{
			CUserManager* pUM = CUserManager::getInstPtr();
			pUM->admin_name = "admin";
			pUM->admin_password = "iloveyou";
			pUM->user_name = "user";
			pUM->user_password = "user";
			pUM->appuser_name = "appuser";
			pUM->appuser_password = "apppass";

			thelog << "管理员 " << pUM->admin_name << "/" << pUM->admin_password << endi;
			thelog << "用户 " << pUM->user_name << "/" << pUM->user_password << endi;
			thelog << "APP客户 " << pUM->appuser_name << "/" << pUM->appuser_password << endi;

			return true;
		}
		//管理员身份验证
		static bool CheckAdminPassword(char const* _user, char const* _pass)
		{
			CUserManager const* pUM = CUserManager::getInstPtr();
			if (pUM->admin_name == _user && pUM->admin_password == _pass)
			{
				return true;
			}
			else
			{
				DEBUG_LOG << "管理员身份认证失败" << ende;
				return false;
			}
		}
		//APP客户身份验证
		static bool CheckAppUserPassword(char const* _user, char const* _pass)
		{
			CUserManager const* pUM = CUserManager::getInstPtr();
			if (pUM->appuser_name == _user && pUM->appuser_password == _pass)
			{
				return true;
			}
			else
			{
				DEBUG_LOG << "APP客户身份认证失败" << ende;
				return false;
			}
		}
		//任何用户身份验证
		static bool CheckUser(char const* _user, char const* _pass)
		{
			DEBUG_LOG << "[" << _user << "]" << "[" << _pass << "]" << endi;
			if (CheckAdminPassword(_user, _pass))
			{
				return true;
			}
			if (CheckAppUserPassword(_user, _pass))
			{
				return true;
			}
			CUserManager const* pUM = CUserManager::getInstPtr();
			if (pUM->user_name == _user && pUM->user_password == _pass)
			{
				return true;
			}
			DEBUG_LOG << "用户身份认证失败" << ende;
			return false;
		}

	};
}
