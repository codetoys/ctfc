//CommandSet.h 命令集接口
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#ifdef _WINDOWS
#define isatty _isatty
#else
#include <termios.h>
#endif
#include "../function/htmldoc.h"
#include "myUtil.h"

namespace ns_my_std
{

	class CCommandSet
	{
	public:
		//命令集命令对象
		struct command
		{
			string m_command_name;//命令名称
			string m_command_note;//命令说明
			bool use_inputfile;//可能使用输入文件
			bool use_outputfile;//可能使用输出文件

			//默认不使用输入输出文件
			command() :use_inputfile(false), use_outputfile(false) {}

			//获取命令行特定参数，如果silent=false则要求用户输入
			//不得自行处理--开头的参数，这代表命令集通用参数
			//检查单个参数是否存在
			bool csGetCommandParam(char const * key, char const *prompt, bool silent, bool isTry = false)
			{
				if (silent)
				{
					if(!ns_my_std::GetCommandParam(g_pActiveAppEnv->argc, g_pActiveAppEnv->argv, key))
					{
						if (!isTry)thelog << "未提供参数 " << key << " ，如果提供则表示对 \"" << prompt << "\" 确认" << endi;
						return false;
					}
					else
					{
						return true;
					}
				}
				else
				{
					stringstream ss;
					ss <<prompt<<"(参数 "<<key<<")\n";
					ss << "是否设置参数 " << key << " (回车=不设置 Y或y=设置)：";
					string tmp = UIInput(ss.str().c_str(), "");
					return "Y" == tmp || "y" == tmp;
				}
			}
			//获取双参数，如“-f a.txt”这样的，GetCommandParam("-f",str)，str返回“a.txt”
			bool csGetCommandParam(char const * key, char const *prompt, long & value, bool silent, bool isTry = false)
			{
				string tmp;
				bool ret = csGetCommandParam(key, prompt, tmp, silent, isTry);
				value = atol(tmp.c_str());
				return ret;
			}
			//获取双参数，如“-f a.txt”这样的，GetCommandParam("-f",str)，str返回“a.txt”
			bool csGetCommandParam(char const * key, char const *prompt, string & value, bool silent, bool isTry = false)
			{
				if (silent)
				{
					if(!ns_my_std::GetCommandParam(g_pActiveAppEnv->argc, g_pActiveAppEnv->argv, key, value))
					{
						if (!isTry)thelog << prompt << " 需要参数 " << key << " 和值" << ende;
						return false;
					}
					else
					{
						return true;
					}
				}
				else
				{
					stringstream ss;
					ss << prompt << "(参数 " << key << ")\n";
					ss << "请输入参数 " << key << " (回车=不输入)：";
					value = UIInput(ss.str().c_str(), "");
					return 0 != value.size();
				}
			}

			//命令执行入口,silent不能交互
			//不得处理公共参数
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				return 0;
			}

			//命令集外壳调用入口
			int doCommand(bool silent)
			{
				string input;
				string output;

				string filename;
				if (use_inputfile && csGetCommandParam("--inputfile", "请输入输入文件名", filename, silent))
				{
					CEasyFile file;
					if (!file.ReadFile(filename.c_str(), input))
					{
						return __LINE__;
					}
				}

				int ret = _doCommand(input, output, silent);

				if (use_outputfile && csGetCommandParam("--outputfile", "请输入输出文件名", filename, silent))
				{
					CEasyFile file;
					if (!file.WriteFile(filename.c_str(), output.c_str()))
					{
						return __LINE__;
					}
				}
				else
				{
					if (use_outputfile)thelog << "输出：" << endl << output << endi;
					thelog << endl << "命令 " << m_command_name << " 执行完毕 返回码 " << ret << endi;
				}

				return ret;
			}
		};
	private:
		struct COMMAND_ID
		{
			string command_id;
			string group;//仅作为过滤依据

			bool operator==(COMMAND_ID const & tmp)const { return command_id == tmp.command_id; }
		};
		typedef vector<pair<COMMAND_ID, command *> > T_DATAS;
		T_DATAS m_commands;

		string current_add_group;//当前添加组

		command * FindCommand(string const & cmd)
		{
			COMMAND_ID tmp;
			tmp.command_id = cmd;

			T_DATAS::iterator it;
			for(it=m_commands.begin();it!=m_commands.end();++it)
			{
				if(tmp==it->first)
				{
					return it->second;
				}
			}
			return NULL;
		}
	public:
		//id不能为空或""
		bool AddCommand(char const * id, command * pCommand)
		{
			if (0 == current_add_group.size())
			{
				AddGroup("未命名命令组");
			}

			COMMAND_ID tmp;
			tmp.command_id = id;
			tmp.group = current_add_group;
			command * p;
			if (tmp.command_id.size() != 0 && NULL != (p = FindCommand(tmp.command_id)))
			{
				thelog << "错误：命令已经存在 " << tmp.command_id << " " << pCommand->m_command_name
					<< " 已存在的是 " << tmp.group << " " << p->m_command_name << ende;
				return false;
			}
			m_commands.push_back(pair<COMMAND_ID, command *>(tmp,pCommand));
			return true;
		}
		bool AddGroup(char const * group)
		{
			current_add_group = group;
			return true;
		}
		//带有--cmd则执行参数给定的命令，否则请求控制台输入并循环，default_command为控制台默认命令
		int doCommandSet(char const * default_command)
		{
			//如果是直接命令行调用则会重置这两个参数
			bool loop = true;
			bool silent = false;

			int ret = 0;

			string select_group;//用户选择的命令组，空为全部
			while (loop)
			{
				string cmd;

				if (GetCommandParam(g_pActiveAppEnv->argc, g_pActiveAppEnv->argv, "--cmd", cmd))
				{
					loop = false;
					silent = true;
				}
				else
				{
					CHtmlDoc::CHtmlTable2 table;
					table.SetTitle("命令表");
					table.AddCol("命令");
					table.AddCol("组");
					table.AddCol("名称");
					table.AddCol("命令");
					table.AddCol("说明");
			
					T_DATAS::const_iterator it;
					string last_group;
					for (it = m_commands.begin(); it != m_commands.end(); ++it)
					{
						if (select_group.size() != 0 && select_group != it->first.group)continue;
						if (last_group != it->first.group && last_group.size() != 0)
						{
							table.AddLine();//额外的空行
						}
						last_group = it->first.group;
						table.AddLine();
						table.AddData(it->first.command_id);
						table.AddData(it->first.group);
						table.AddData(it->second->m_command_name);
						table.AddData(it->first.command_id);
						table.AddData(it->second->m_command_note);
					}
					thelog << endl << table.MakeTextTable() << endi;
					if (0 == isatty(STDIN_FILENO))
					{
						G_SET_ERROR(My_ERR_NOT_TTY) << "not tty,want --cmd" << ende;
						return My_ERR_NOT_TTY;
					}
					else
					{
						cmd = UIInput("请选择命令：（--group选择命令组 b=break）", default_command);
						if (cmd == "b")break;
						if (cmd == "--group")
						{
							select_group = UIInput("请输入命令组：（默认为全部）", "");
							continue;
						}
					}
				}
				command * pCommand = FindCommand(cmd);
				if (NULL == pCommand)
				{
					thelog << "无效的命令 " << cmd << ende;
				}
				else
				{
					ret = pCommand->doCommand(silent);
				}
				if (!silent)UIInput("press any key to continue ... ", "");
			}
			return ret;
		}
	};
	class CCommandEnableDebug : public CCommandSet::command
	{
	public:
		CCommandEnableDebug()
		{
			m_command_name = "CCommandEnableDebug";
			m_command_note = "打开调试";
		}
		virtual int _doCommand(string const & input, string & output, bool silent)
		{
			G_IS_DEBUG = true;
			return 0;
		}
	};
	class CCommandDisableDebug : public CCommandSet::command
	{
	public:
		CCommandDisableDebug()
		{
			m_command_name = "CCommandDisableDebug";
			m_command_note = "关闭调试";
		}
		virtual int _doCommand(string const & input, string & output, bool silent)
		{
			G_IS_DEBUG = false;
			return 0;
		}
	};
}
