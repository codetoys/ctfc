//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../function/function.h"
#include "mysocketserver.h"

namespace ns_my_std
{
#define G_pHttpServerData ((SocketServerControlBlock *)CPrivateShmMgr::getInstPtr()->getShmAddr()) //全局服务控制块
#define CHttpServerDatas_MAX (256)//共享内存能够记录的命令个数

	//用于每个子进程的数据结构
	class CHttpChildDataBase
	{
	public:
		//用于输出数据的场合
		string & toString(string & str)const{ str = "toString undefined"; return str; }

		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table){ return false; }
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const{ return false; }
	};

	class CPrivateShmMgr
	{
		DECLARE_SINGLETON(CPrivateShmMgr);
	private:
		char * m_pShm;
		long m_size;
	public:
		CPrivateShmMgr() :m_pShm(NULL){}
		bool CreateShm(long size)
		{
			m_size=size;
			m_pShm = new char[m_size];
			if (NULL == m_pShm)
			{
				LOG << "连接服务用共享内存失败" << ENDE;
				return false;
			}
			memset(m_pShm,0,m_size);
			return true;
		}
		bool DestoryShm()
		{
			delete[]m_pShm;
			m_pShm = NULL;
			return true;
		}
		char * getShmAddr(){ return m_pShm; }
		long getShmID(){ return -1; }
	};

	//HTTP服务器实现的子进程数据结构
	class CHttpChildData : public CHttpChildDataBase
	{
	public:
		long request_count;
		time_t last_time;//最后一次活动的时间
		sstring<24> peer_info;//客户端信息
		sstring<256 > info;

		CHttpChildData() :request_count(0), last_time(0) {}
	public:
		void SetHttpProcessInfo(char const * _info)
		{
			info = _info;
			last_time = time(NULL);
		}

		//用于输出数据的场合
		string & toString(string & str)const{ str = "toString undefined"; return str; }

		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{
			table.AddCol("request", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("last_time", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("peer_info");
			table.AddCol("http_info");
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddData(request_count, true);
			table.AddData(last_time ? CMyTools::TimeToString_log(last_time) : string(""));
			table.AddData(peer_info.c_str());
			table.AddData(info.c_str());
			return true;
		}
	};

	//HTTP服务器实现的整个服务器的数据结构
	class CHttpServerDatas
	{
	public:
		struct WebCommandData
		{
			sstring<256> command_id;
			long WebCommandType;
			long count;
			long count_err;

			WebCommandData() :command_id(""), count(0), count_err(0) {}
			//用于表格输出
			static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
			{
				table.AddCol("command_id");
				table.AddCol("type", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("count", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("count_err", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				return true;
			}
			bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
			{
				table.AddData(command_id.c_str());
				table.AddData(WebCommandType);
				table.AddData(count, true);
				table.AddData(count_err, true);
				return true;
			}
		};

		sstring<256 > m_sitename;//站点名称，用在内置的标准页面上
		sstring<256 > m_realm;//用于认证

		sstring<1024 > m_root;//根目录
		
		CMyRWMutex2::mySEM m_webcommand_sem;//互斥对象

		WebCommandData m_web_command_data_s[CHttpServerDatas_MAX];

	public:
		CHttpServerDatas()
		{
		}
		bool HSD_Output(map<long, CHtmlDoc::CHtmlTable2 > & oldtables, CHtmlDoc::CHtmlTable2::OUTPUT_TYPE type, string & ret)const
		{
			ret = "";

			long i;
			string str;

			{
				CHtmlDoc::CHtmlTable2 & oldtable = oldtables[__LINE__];
				CHtmlDoc::CHtmlTable2 table;
				table.SetID("HDS_1");

				table.SetTitle("HTTP信息");
				table.AddCol("SITE NAME", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("REALM", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("ROOT", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

				table.AddLine();

				table.AddData(m_sitename.c_str());
				table.AddData(m_realm.c_str());
				table.AddData(m_root.c_str());
				
				ret += table.MakeOutput(type, &oldtable);
				oldtable = table;
			}

			{
				CHtmlDoc::CHtmlTable2 & oldtable = oldtables[__LINE__];
				CHtmlDoc::CHtmlTable2 table;
				table.SetID("HDS_2");

				table.SetTitle("WebCommand信息");
				table.AddFoot(0, "统计");
				table.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				WebCommandData::AddTableColumns(table);
				for (i = 0; i < CHttpServerDatas_MAX; ++i)
				{
					WebCommandData const * p = &m_web_command_data_s[i];

					if (0 == p->command_id.size())break;

					table.AddLine();

					table.AddData(i);
					p->AddTableData(table);
				}
				if (CHtmlDoc::CHtmlTable2::OUTPUT_SCRIPT == type && table.GetRecordCount() > oldtable.GetRecordCount())
				{
					//行数增多，无法继续更新
					return false;
				}

				ret += table.MakeOutput(type, &oldtable);
				oldtable = table;
			}
			return true;
		}
	};
	struct SocketServerControlBlock : public SocketServerData
	{
		CHttpChildData child_datas[CSocketServer_MAX_CHILD];
		CHttpServerDatas server_data;

		//oldtables oldtables2 用于保存上次的结果，以便在后面输出变更脚本
		bool SSCB_Output(map<long, CHtmlDoc::CHtmlTable2 > & oldtables,map<long, CHtmlDoc::CHtmlTable2 > & oldtables2, CHtmlDoc::CHtmlTable2::OUTPUT_TYPE type, string & ret)const
		{
			ret = "";
			
			long i;
			string str;

			{
				CHtmlDoc::CHtmlTable2 & oldtable = oldtables[__LINE__];
				CHtmlDoc::CHtmlTable2 table;
				table.SetID("SSCB_1");

				str = server_name.c_str();
				str += " 的服务器信息";
				table.SetTitle(str.c_str());
				table.AddCol("isDebug", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("start_time", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("port", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("cmd_bShutDown", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("cmd_bPause", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("state_bPause", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("MAX", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("max_child", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("childs", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("time");//服务器时间
				table.AddLine();
				table.AddData(isDebug);
				table.AddData(CMyTools::TimeToString_log(server_start_time));
				table.AddData(port);
				table.AddData(cmd_bShutDown ? "1" : "0");
				table.AddData(cmd_bPause ? "1" : "0");
				table.AddData(state_bPause ? "暂停" : "运行");
				table.AddData(CSocketServer_MAX_CHILD);
				table.AddData(max_child);
				table.AddData(GetChildCount());
				table.AddData(CMyTools::TimeToString_log(time(NULL)));

				ret += table.MakeOutput(type, &oldtable);
				oldtable = table;
			}

			{
				CHtmlDoc::CHtmlTable2 & oldtable = oldtables[__LINE__];
				CHtmlDoc::CHtmlTable2 table;
				table.SetID("SSCB_MAIN_PROCESS");

				str = (CCurrentProcess::MultiProcessMode() ? "核心进程" : "核心线程");
				table.SetTitle(str.c_str());
				main_process_info::AddTableColumns(table);
				table.AddLine();
				admin_process.AddTableData(table);
				table.AddLine();
				httpd_main_process.AddTableData(table);
				
				ret += table.MakeOutput(type, &oldtable);
				oldtable = table;
			}

			if (!server_data.HSD_Output(oldtables2, type, str))
			{
				return false;
			}
			ret += str;

			{
				CHtmlDoc::CHtmlTable2 & oldtable = oldtables[__LINE__];
				CHtmlDoc::CHtmlTable2 table;
				table.SetID("SSCB_2");

				table.SetTitle(CCurrentProcess::MultiProcessMode() ? "子进程信息" : "子线程信息");
				table.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				child_info::AddTableColumns(table);
				table.AddCol("-");
				CHttpChildData::AddTableColumns(table);
				table.AddFoot(0, "统计");
				for (i = 0; i < CSocketServer_MAX_CHILD; ++i)
				{
					SocketServerData::child_info const * pChild = &childs[i];
					if (-1 == pChild->pid && 0 == pChild->start_time)
					{
						continue;//从没用过的记录
					}

					table.AddLine();

					table.AddData(i);
					pChild->AddTableData(table);
					table.AddData("-");
					child_datas[i].AddTableData(table);
				}
				if (CHtmlDoc::CHtmlTable2::OUTPUT_SCRIPT == type && table.GetRecordCount() > oldtable.GetRecordCount())
				{
					//行数增多，无法继续更新
					return false;
				}

				ret += table.MakeOutput(type, &oldtable);
				oldtable = table;
			}
			
			return true;
		}
	};

	//CHttpProcess的命令管理结构
	class CWebCommand;
	struct web_command_private_struce
	{
		long seq;//添加顺序，同时也是在共享内存的顺序
		bool isInited;//是否经过了初始化
		CWebCommand * pWebCommand;
	};
	typedef map<string, web_command_private_struce > TYPE_WEB_COMMANDS;
	typedef map<long, web_command_private_struce *> TYPE_QUEUE_WEB_COMMANDS;
}
