//csv格式文件处理
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
#include "htmldoc.h"
#include <sys/stat.h>

namespace ns_my_std
{
	//基本csv功能
	class CCSV
	{
	private:
		CStdOSFile m_file;
		vector<string > m_Heads;//头标，如果no_head则没有头标
	public:
		//根据路径名创建所有目录，不包括最后的文件名
		static bool CreateDir(char const* filename)
		{
			string dirname;
			char const* p = filename;
			for (; '\0' != *p; ++p)
			{
				dirname += *p;
				if ('/' == *p)
				{
					//thelog<<dirname<<endi;
					mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
				}
			}
			return true;
		}
		//csv格式，双引号内为普通文本，文本内双引号要用两个双引号表达，简单起见全部加上了双引号
		static string& CSVEncode(char const* in, string& out)
		{
			out = "";
			out += '\"';
			for (char const* p = in; *p != '\0'; ++p)
			{
				if ('\"' == *p)out += '\"';
				out += *p;
			}
			out += '\"';
			return out;
		}
		//csv格式，双引号内为普通文本，文本内双引号要用两个双引号表达，简单起见全部加上了双引号
		static string CSVEncode(string const& in)
		{
			string out;
			return CSVEncode(in.c_str(), out);
		}
		//打开文件
		bool OpenCSV(char const* filepatnname, bool no_head = false)
		{
			m_Heads.clear();

			//if (!CreateDir(filepatnname))return false;
			if (!m_file.OpenR(filepatnname))return false;

			if (!no_head)
			{
				bool isLineEnd;
				string str;
				while (_ReadField(isLineEnd, str))
				{
					if (isLineEnd)
					{
						break;
					}
					if (0 == m_Heads.size())
					{//对第一个消除可能存在的UTF-8签名（EF BB BF）
						if (0 == strncmp("\xEF\xBB\xBF", str.c_str(), 3))
						{
							str.erase(0, 3);
						}
					}
					//thelog << str << endi;
					m_Heads.push_back(str);
				}
				thelog << "获得列头 " << m_Heads.size() << " 个" << endi;
			}
			return true;
		}
		//获得字段序号
		long GetFieldIndex(char const* field)const
		{
			vector<string >::const_iterator it;
			for (it = m_Heads.begin(); it != m_Heads.end(); ++it)
			{
				if (*it == field)return it - m_Heads.begin();
			}
			return -1;
		}
		//获得字段数
		long GetFieldCount()const
		{
			return m_Heads.size();
		}
		//获得字段名
		string GetFieldName(long field)const
		{
			return m_Heads[field];
		}
		//读取字段，或者是一个字段，或者是行结束
		bool _ReadField(bool& isLineEnd, string& ret)
		{
			bool tmp = __ReadField(isLineEnd, ret);
			CMyTools::TrimAll(ret);
			return tmp;
		}
		bool __ReadField(bool& isLineEnd, string& ret)
		{
			isLineEnd = false;
			ret = "";

			char m_c;
			string str;
			bool isFirstChar = true;
			bool isInStr = false;
			bool isInStrQuote = false;//是否在字符串内出现了一个双引号，这意味着字符串结束，或者一个内嵌双引号的开始
			while (1 == m_file.Read(&m_c, 1))
			{
				if (isFirstChar && ('\r' == m_c || '\n' == m_c))
				{//仅当第一个字符就是行结束才认为是行结束而没有获得数据
					isLineEnd = true;
					if ('\r' == m_c)
					{
						if (1 == m_file.Read(&m_c, 1))
						{
							if ('\n' != m_c)
							{
								m_file.SeekCur(-1);//不是连续\r\n，吐回去
							}
						}
					}
					return true;
				}
				else
				{
					isFirstChar = false;
				}

				if (isInStr && isInStrQuote && '\"' == m_c)
				{//内嵌双引号
					ret = +m_c;
					isInStrQuote = false;
					continue;
				}
				if (isInStr && isInStrQuote && ',' == m_c)
				{//字符串结束，逗号标记字段结束
					return true;
				}
				if (isInStr && isInStrQuote && ('\r' == m_c || '\n' == m_c))
				{//字符串结束，行结束
					m_file.SeekCur(-1);//吐回行结束，否则永远不能发现单独的行结束
					return true;
				}
				if (isInStr && isInStrQuote)
				{//字符串结束
					isInStr = false;
					continue;
				}
				if (isInStr && !isInStrQuote && '\"' == m_c)
				{//遇到内部第一个双引号
					isInStrQuote = true;
					continue;
				}
				if (isInStr && !isInStrQuote)
				{//字符串内容，除了双引号都不用特殊判断
					ret += m_c;
					continue;
				}

				if (!isInStr && '\"' == m_c)
				{//字符串开始
					isInStr = true;
					continue;
				}
				if (!isInStr && ',' == m_c)
				{//字段结束
					return true;
				}
				if (!isInStr && ('\r' == m_c || '\n' == m_c))
				{//行结束
					m_file.SeekCur(-1);//吐回行结束，否则永远不能发现单独的行结束
					return true;
				}
				if (!isInStr)
				{//非字符串的其余
					ret += m_c;
					continue;
				}
			}

			return false;
		}
		bool ReadField(string& ret)
		{
			bool isLineEnd;
			while (_ReadField(isLineEnd, ret))
			{
				if (!isLineEnd)return true;
			}
			return false;
		}
		//关闭文件
		bool CloseCSV()
		{
			return m_file.Close();
		}
		//文件到表格
		bool LoadFromFile(char const* file, CHtmlDoc::CHtmlTable2 & table)
		{
			table.Clear();

			CCSV csv;
			if (!csv.OpenCSV(file))
			{
				return false;
			}
			long i;
			for (i = 0; i < csv.GetFieldCount(); ++i)
			{
				table.AddCol(csv.GetFieldName(i));
			}
			bool isLineEnd;
			string str;
			bool need_new_line = false;
			table.AddLine();
			while (csv._ReadField(isLineEnd, str))
			{
				if (isLineEnd)
				{
					need_new_line = true;
				}
				else
				{
					if (need_new_line)
					{
						table.AddLine();
						need_new_line = false;
					}
					table.AddData(str);
				}
			}
			table.Fill();
			return csv.CloseCSV();
		}
	};
	class CText
	{
	public:
		//删除两端的空白，连续回车换行缩减为一个换行
		static string FormatText(char const* text)
		{
			string ret = text;
			CMyTools::TrimAll(ret);

			string::size_type pos = 0;
			bool isInNewLine = false;
			while (pos < ret.size())
			{
				if ('\r' == ret[pos] || '\n' == ret[pos])
				{
					if (!isInNewLine)
					{
						isInNewLine = true;
						ret[pos] = '\n';
						++pos;
					}
					else
					{//丢弃连续的换行，pos不变
						ret.erase(pos, 1);
					}
				}
				else
				{
					isInNewLine = false;
					++pos;
				}
			}
			return ret;
		}

	};
}

