//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/myUtil.h"
#include "../env/CommandSet.h"
#include "../function/myGmSSL.h"

namespace ns_my_std
{
	class TestGmSSL : public CCommandSet::command
	{
	public:
		TestGmSSL()
		{
			m_command_name = "TestGmSSL";
			m_command_note = "国密测试";
		}
		virtual int _doCommand(string const& input, string& output, bool silent)
		{
			if (!CMyGmSSL::aes_test())return 1;
			else return 0;
		}
	};
}
