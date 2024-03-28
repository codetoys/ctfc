//ColumnData.h 列数据
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <vector>
#include <string>
using std::vector;
using std::string;

namespace ns_my_std
{
	//与内存数据库和SQL有关的基础结构
	enum COLUMN_TYPE { COLUMN_TYPE_LONG = 0, COLUMN_TYPE_DOUBLE, COLUMN_TYPE_STRING_POOL };
	struct sColumnDefine
	{
		string name;
		COLUMN_TYPE type;
		string show_type;
		string comment;

		string GetTypeString()const
		{
			switch (type)
			{
			case COLUMN_TYPE_LONG:return "COLUMN_TYPE_LONG"; break;
			case COLUMN_TYPE_DOUBLE:return "COLUMN_TYPE_DOUBLE"; break;
			case COLUMN_TYPE_STRING_POOL:return "COLUMN_TYPE_STRING_POOL"; break;
			default:return "unknown"; break;
			}
		}
	};
	class CColumnDefines : public vector<sColumnDefine>
	{
	public:
		void push_back(char const* _name, COLUMN_TYPE _type, char const* _comment, char const* _show_type = "")
		{
			sColumnDefine tmp;
			tmp.name = _name;
			ToUpper(tmp.name);
			tmp.type = _type;
			tmp.show_type = _show_type;
			tmp.comment = _comment;
			vector<sColumnDefine>::push_back(tmp);
		}
	};
	class CColumnData
	{
	private:
		COLUMN_TYPE type;
		long long_data;
		double double_data;
		string string_data;

	public:
		CColumnData() { Set(""); }
		void Set(long value)
		{
			type = COLUMN_TYPE_LONG;
			long_data = value;
		}
		void Set(double value)
		{
			type = COLUMN_TYPE_DOUBLE;
			double_data = value;
		}
		void Set(string const& value)
		{
			type = COLUMN_TYPE_STRING_POOL;
			string_data = value;
		}
		void Set(char const* value)
		{
			type = COLUMN_TYPE_STRING_POOL;
			string_data = value;
		}
		long GetLong()const
		{
			if (COLUMN_TYPE_STRING_POOL == type)return atol(string_data.c_str());
			else if (COLUMN_TYPE_LONG == type)	return long_data;
			else return (long)double_data;
		}
		double GetDouble()const
		{
			if (COLUMN_TYPE_STRING_POOL == type)return atof(string_data.c_str());
			else if (COLUMN_TYPE_LONG == type)	return long_data;
			else return double_data;
		}
		string GetString()const
		{
			string str;
			return toString(str);
		}
		void OutputTo(string& output)const
		{
			output = GetString();
		}
		void OutputTo(long& output)const
		{
			output = GetLong();
		}
		void OutputTo(double& output)const
		{
			output = GetDouble();
		}
		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];

			if (COLUMN_TYPE_STRING_POOL == type)return str = string_data;
			else if (COLUMN_TYPE_LONG == type)	sprintf(buf, "%ld", long_data);
			else sprintf(buf, "%f", double_data);
			return str = buf;
		}
	};
	class CColumnDataS : public vector<CColumnData >
	{
	public:
		void push_back(char const* data)
		{
			CColumnData tmp;
			tmp.Set(data);
			vector<CColumnData >::push_back(tmp);
		}
		void push_back(long data)
		{
			CColumnData tmp;
			tmp.Set(data);
			vector<CColumnData >::push_back(tmp);
		}
		void push_back(double data)
		{
			CColumnData tmp;
			tmp.Set(data);
			vector<CColumnData >::push_back(tmp);
		}
		void push_back(CColumnData const& data)
		{
			vector<CColumnData >::push_back(data);
		}
	};
}
