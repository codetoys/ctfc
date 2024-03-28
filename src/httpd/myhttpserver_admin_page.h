//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/myUtil.h" 
using namespace ns_my_std;
#include "myhttpbase.h"
#include "myhttpserver_struct.h"
#include "../function/mydir.h"

namespace ns_my_std
{
	//HTTP处理
	class CWebCommand_SetDebug : public CWebCommand
	{
	public:
		CWebCommand_SetDebug()
		{
			clear();
			AddWebCommandParam("SetDebug", "SetDebug", "1：开启调试/0：关闭调试", "");
			SetWebCommand("SetDebug", "设置调试开关", "开启G_IS_DEBUG，注意，开启后产生海量日志");
		}

		virtual bool doWebFunction(CHttpRequest const * pRequest, CMySocket & s, CHttpRespond * pRespond)
		{
			string param = pRequest->GetParam("SetDebug");

			if ("1" == param)
			{
				G_IS_DEBUG = true;
				pRespond->AppendBody("打开调试成功");
			}
			else if ("0" == param)
			{
				G_IS_DEBUG = false;
				pRespond->AppendBody("关闭调试成功");
			}
			else
			{
				pRespond->AppendBody("非法参数，超出限制");
			}
			return true;
		}
	};
	class CWebCommand_SetMax : public CWebCommand
	{
	public:
		CWebCommand_SetMax()
		{
			clear();
			AddWebCommandParam("max", "max", "最大值", "");
			SetWebCommand("SetMax", "设置最大并发数", "可以将进程数限制为较小的数值");
		}

		virtual bool doWebFunction(CHttpRequest const * pRequest, CMySocket & s, CHttpRespond * pRespond)
		{
			string param = pRequest->GetParam("max");

			long newvalue = atol(param.c_str());
			if (newvalue > 0 && newvalue <= CSocketServer_MAX_CHILD)
			{
				G_pHttpServerData->max_child = newvalue;
				pRespond->AppendBody("设置成功");
			}
			else
			{
				pRespond->AppendBody("非法参数，超出限制");
			}
			return true;
		}
	};
	class CWebCommand_Pause : public CWebCommand
	{
	public:
		CWebCommand_Pause()
		{
			clear();
			SetWebCommand("Pause", "暂停", "停止业务端口（管理端口仍然开启）");
		}

		virtual bool doWebFunction(CHttpRequest const * pRequest, CMySocket & s, CHttpRespond * pRespond)
		{
			G_pHttpServerData->cmd_bPause = 1;
			pRespond->AppendBody("设置成功");
			return true;
		}
	};

	class CWebCommand_vShell : public CWebCommand
	{
	public:
		CWebCommand_vShell()
		{
			clear();
			AddWebCommandParam("command", "SHELL命令", "", "");
			SetWebCommand("shell", "执行SHELL命令", "类似telnet的功能");
		}
	};
	class CWebCommand_vSSH : public CWebCommand
	{
	public:
		CWebCommand_vSSH()
		{
			clear();
			AddWebCommandParam("host", "主机", "", "");
			AddWebCommandParam("user", "用户", "", "");
			AddWebCommandParam("changedir", "切换工作路径", "", "");
			AddWebCommandParam("command", "SHELL命令", "", "");
			SetWebCommand("ssh", "执行ssh命令", "远程ssh命令");
		}
	};
	class CWebCommand_vViewFile : public CWebCommand
	{
	public:
		CWebCommand_vViewFile()
		{
			clear();
			AddWebCommandParam("file", "文件名", "可带路径", "");
			SetWebCommand("ViewFile", "查看文件", "具有分页功能");
		}
	};
	class CWebCommand_vDownFile : public CWebCommand
	{
	public:
		CWebCommand_vDownFile()
		{
			clear();
			AddWebCommandParam("file", "文件名", "可带路径", "");
			SetWebCommand("DownFile", "下载文件", "文件另存为");
		}
	};
	class CWebCommand_ShowDir : public CWebCommand,private ns_my_std::CForEachDir
	{
	private:
		string output;
		string dir;
		string filter;
		bool r;
		virtual int doDirBegin(char const * dirname, long deep);
		virtual int doDirEnd(char const * dirname, long deep);
		virtual int doOneFile(char const * filename, bool isDir, long deep);
	public:
		CWebCommand_ShowDir()
		{
			clear();
			AddWebCommandParam("dir", "目录", "", "");
			AddWebCommandParam("filter", "过滤(未实现)", "", "*.*");
			AddWebCommandParamCheckBox("r", "递归", "", false);
			SetWebCommand("ShowDir", "目录浏览", "");
		}

		virtual bool doWebFunction(CHttpRequest const * pRequest, CMySocket & s, CHttpRespond * pRespond);
	};
}
