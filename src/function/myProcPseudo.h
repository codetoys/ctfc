//myProcPseudo.h Linux proc pseudo File System
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include "function.h"
#include "mydir.h"
#include <errno.h>

namespace ns_my_std
{
	class CProcPseudo
	{
	public:
		class CTextFile
		{
		private:
			bool GetShellOutput(char const* cmd, string& output)
			{
				char buf[1024];
				FILE* pf;

				output = "";
				//DEBUG_LOG << cmd << endi;
				if (NULL == (pf = popen(cmd, "r")))
				{
					output = "popen失败，无法执行 ";
					output += strerror(errno);
					return false;
				}
				while (NULL != fgets(buf, 1024, pf))
				{
					output += buf;
				}
				//DEBUG_LOG << output << endi;
				pclose(pf);
				return true;
			}
			//Tab替换为空格
			void TabToSpace()
			{
				for (size_t i = 0; i < m_filedata.size(); ++i)
				{
					if ('\t' == m_filedata[i])m_filedata[i] = ' ';
				}
			}
		public:
			string m_filedata;
			vector<CStringSplit > m_formatted_data;
			bool ReadInfo(char const* cmd, stringstream& msg)
			{
				if (!GetShellOutput(cmd, m_filedata))
				{
					msg << m_filedata;
					return false;
				}
				TabToSpace();
				m_formatted_data.clear();
				CStringSplit split1(m_filedata.c_str(), "\n");
				for (size_t i = 0; i < split1.size(); ++i)
				{
					CStringSplit split2(split1[i].c_str(), " ");
					m_formatted_data.push_back(split2);
				}
				return true;
			}
			string const& GetFileData()const { return m_filedata; }
			//获得指定开头的行的指定数据，i从0开始
			long GetDataLong(char const* data0, long index)
			{
				return atol(GetDataStr(data0, index).c_str());
			}
			string GetDataStr(char const* data0, size_t index)
			{
				for (size_t i = 0; i < m_formatted_data.size(); ++i)
				{
					CStringSplit& line = m_formatted_data[i];
					if (line.size() != 0 && line[0] == data0)
					{
						if (line.size() > index)return line[index].c_str();
						else return 0;
					}
				}
				return 0;
			}
			//获得指定开头的行的指定数据，i从0开始
			long GetDataLong(long line, long index)
			{
				return atol(GetDataStr(line, index).c_str());
			}
			string GetDataStr(size_t line, size_t index)
			{
				if (line < m_formatted_data.size())
				{
					if (index < m_formatted_data[line].size())
					{
						//cout << "line " << line << " col " << index << " " << m_formatted_data[line][index] << endl;
						return m_formatted_data[line][index].c_str();
					}
				}
				return 0;
			}
			//获得指定开头的行的索引
			long GetLine(char const* data0)
			{
				for (size_t i = 0; i < m_formatted_data.size(); ++i)
				{
					CStringSplit& line = m_formatted_data[i];
					if (line.size() != 0 && line[0] == data0)
					{
						return i;
					}
				}
				return -1;
			}
			//统计指定开头的行的数量
			long CountData(char const* data0)
			{
				long ret = 0;
				for (size_t i = 0; i < m_formatted_data.size(); ++i)
				{
					CStringSplit& line = m_formatted_data[i];
					//cout << i << " [" << line[0] << "]" << endl;
					if (line.size() != 0 && line[0] == data0)
					{
						++ret;
					}
				}
				return ret;
			}
		};

		class CpuUsing
		{
		public:
			string cpu;
			long user;
			long nice;
			long system;
			long idle;

			//从新旧stat计算
			void Make(char const * _cpu,CTextFile & stat_old, CTextFile & stat_new)
			{
				//cout << _cpu << endl;
				//cout << stat_old.GetFileData() << endl;
				//cout << stat_new.GetFileData() << endl;
				
				long line_old = stat_old.GetLine(_cpu);
				long line_new = stat_new.GetLine(_cpu);
				//cout << line_old << endl;
				//cout << line_new << endl;
				cpu = _cpu;
				user = stat_new.GetDataLong(line_new, 1) - stat_old.GetDataLong(line_old, 1);
				nice = stat_new.GetDataLong(line_new, 2) - stat_old.GetDataLong(line_old, 2);
				system = stat_new.GetDataLong(line_new, 3) - stat_old.GetDataLong(line_old, 3);
				idle = stat_new.GetDataLong(line_new, 4) - stat_old.GetDataLong(line_old, 4);
			}
			//输出
			stringstream & show(stringstream & ss)const
			{
				long total = user + nice + system + idle;
				ss << cpu << " user " << user * 100 / total << "% nice " << nice * 100 / total << "% system " << system * 100 / total << "% idle " << idle * 100 / total << "%" << endl;
				return ss;
			}
		};

		bool ShowSysState(stringstream & ret)
		{
			CTextFile file, file2;

			string cmd_meminfo = "cat /proc/meminfo";
			if (!file.ReadInfo(cmd_meminfo.c_str(), ret))
			{
				ret << cmd_meminfo << " 失败";
				return false;
			}
			//ret << "文件数据：" << endl << file.GetFileData() << endl;
			//ret << "总行数：" << file.m_formatted_data.size() << endl;
			for (size_t i = 0; i < file.m_formatted_data.size(); ++i)
			{
				//ret << i << " " << file.m_formatted_data[i].size() << endl;
			}
			ret << "总内存 " << file.GetDataLong("MemTotal:", 1) << " " << file.GetDataStr("MemTotal:", 2) << endl;
			ret << "未用 " << file.GetDataLong("MemFree:", 1) << " " << file.GetDataStr("MemTotal:", 2) << endl;
			ret << "可用 " << file.GetDataLong("MemAvailable:", 1) << " " << file.GetDataStr("MemTotal:", 2) << endl;
			ret << "使用率 " << (file.GetDataLong("MemTotal:", 1) - file.GetDataLong("MemFree:", 1)) * 100 / file.GetDataLong("MemTotal:", 1) << "%" << endl;
			ret << "可用率 " << file.GetDataLong("MemFree:", 1) * 100 / file.GetDataLong("MemTotal:", 1) << "%" << endl;
		
			long cpu_num = 0;

			string cmd_cpuinfo = "cat /proc/cpuinfo";
			if (!file.ReadInfo(cmd_cpuinfo.c_str(), ret))
			{
				ret << cmd_cpuinfo << " 失败";
				return false;
			}
			//ret << "文件数据：" << endl << file.GetFileData() << endl;
			//ret << "总行数：" << file.m_formatted_data.size() << endl;
			cpu_num = file.CountData("processor");
			ret << "CPU数：" << cpu_num << endl;
			
			string cmd_stat = "cat /proc/stat";
			if (!file.ReadInfo(cmd_stat.c_str(), ret))
			{
				ret << cmd_stat << " 失败";
				return false;
			}
			SleepSeconds(1);
			if (!file2.ReadInfo(cmd_stat.c_str(), ret))
			{
				ret << cmd_stat << " 失败";
				return false;
			}
			CpuUsing cpu;
			{
				cpu.Make("cpu", file, file2);
				cpu.show(ret);
			}
			for (long i = 0; i < cpu_num; ++i)
			{
				char buf[256];
				sprintf(buf, "cpu%ld", i);
				cpu.Make(buf, file, file2);
				cpu.show(ret);
			}
		
			return true;
		}
	};
}
