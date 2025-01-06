//CTScript2.h 脚本解释器
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "ICTScript2.h"
#include <math.h>

namespace ns_my_script2
{
	//这个函数用来测试
	struct CNullFun : public CPlugin
	{
		CNullFun() :CPlugin("nullfun", Variable::NULLVARIABLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "测试用函数，用户不可调用\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			for (size_t i = 0; i < params.size(); ++i)
			{
				if (!params[i].isconst)msg += "参数必须是常数\r\n";
			}
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe) { return true; }
	};
	struct CMax : public CPlugin
	{
		CMax() :CPlugin("max", Variable::DOUBLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "取最大值，1-N个参数，参数必须是数值\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() < 1)msg += "参数不足\r\n";
			for (size_t i = 0; i < params.size(); ++i)
			{
				if (!params[i].isNumber())msg += "参数必须是数值\r\n";
			}
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			size_t _max = 0;
			for (size_t i = 1; i < params.size(); ++i)
			{
				if (params[i].GetDouble() > params[_max].GetDouble())_max = i;
			}
			ret = params[_max];
			return true;
		}
	};
	struct CMin : public CPlugin
	{
		CMin() :CPlugin("min", Variable::DOUBLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "取最小值，1-N个参数，参数必须是数值\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() < 1)msg += "参数不足\r\n";
			for (size_t i = 0; i < params.size(); ++i)
			{
				if (!params[i].isNumber())msg += "参数必须是数值\r\n";
			}
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			size_t _max = 0;
			for (size_t i = 1; i < params.size(); ++i)
			{
				if (params[i].GetDouble() < params[_max].GetDouble())_max = i;
			}
			ret = params[_max];
			return true;
		}
	};
	struct CFloor : public CPlugin
	{
		CFloor() :CPlugin("floor", Variable::DOUBLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "向下取整，1个参数，参数必须是数值\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是一个\r\n";
			if (params.size() >= 1 && !params[0].isNumber())msg += "参数1必须是数值\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = floor(params[0].GetDouble());
			return true;
		}
	};
	struct CCeil : public CPlugin
	{
		CCeil() :CPlugin("ceil", Variable::DOUBLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "向上取整，1个参数，参数必须是数值\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是一个\r\n";
			if (params.size() >= 1 && !params[0].isNumber())msg += "参数1必须是数值\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = ceil(params[0].GetDouble());
			return true;
		}
	};
	struct CRound : public CPlugin
	{
		CRound() :CPlugin("round", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "四舍五入，1个参数，参数必须是数值，负数向0舍入，-1.2返回-1，-1.8返回-2\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是一个\r\n";
			if (params.size() >= 1 && !params[0].isNumber())msg += "参数1必须是数值\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			double tmp = params[0].GetDouble();
			if (tmp >= 0)ret = tmp + 0.5;
			else ret = -(-tmp + 0.5);
			return true;
		}
	};
	struct CAbs : public CPlugin
	{
		CAbs() :CPlugin("abs", Variable::DOUBLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "绝对值，1个参数，参数必须是数值\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是一个\r\n";
			if (params.size() >= 1 && !params[0].isNumber())msg += "参数1必须是数值\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			double tmp = params[0].GetDouble();
			if (tmp >= 0)ret = tmp;
			else ret = -tmp;
			return true;
		}
	};
	struct CStrlen : public CPlugin
	{
		CStrlen() :CPlugin("strlen", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "字符串长度，1个参数，参数必须是字符串\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是一个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = (long)params[0].strValue.size();
			return true;
		}
	};
	struct CStrcmp : public CPlugin
	{
		CStrcmp() :CPlugin("strcmp", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "字符串比较，2个参数，参数必须是字符串\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 2)msg += "参数不是二个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			if (params.size() >= 2 && !params[1].isString())msg += "参数2必须是字符串\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = (long)strcmp(params[0].strValue.c_str(), params[1].strValue.c_str());
			return true;
		}
	};
	struct CStricmp : public CPlugin
	{
		CStricmp() :CPlugin("stricmp", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "字符串忽略大小写比较，2个参数，参数必须是字符串\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 2)msg += "参数不是二个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			if (params.size() >= 2 && !params[1].isString())msg += "参数2必须是字符串\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = (long)stricmp(params[0].strValue.c_str(), params[1].strValue.c_str());
			return true;
		}
	};
	struct CStrncmp : public CPlugin
	{
		CStrncmp() :CPlugin("strncmp", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "字符串部分比较，3个参数\r\n";
			ret += "参数1 字符串\r\n";
			ret += "参数2 字符串\r\n";
			ret += "参数3 整数\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 3)msg += "参数不是3个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			if (params.size() >= 2 && !params[1].isString())msg += "参数2必须是字符串\r\n";
			if (params.size() >= 3 && !params[2].isNumber())msg += "参数3必须是整数\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = (long)strncmp(params[0].strValue.c_str(), params[1].strValue.c_str(), params[2].GetLong());
			return true;
		}
	};
	struct CStrnicmp : public CPlugin
	{
		CStrnicmp() :CPlugin("strnicmp", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "字符串忽略大小写部分比较，3个参数\r\n";
			ret += "参数1 字符串\r\n";
			ret += "参数2 字符串\r\n";
			ret += "参数3 整数\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 3)msg += "参数不是3个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			if (params.size() >= 2 && !params[1].isString())msg += "参数2必须是字符串\r\n";
			if (params.size() >= 3 && !params[2].isNumber())msg += "参数3必须是整数\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = (long)strnicmp(params[0].strValue.c_str(), params[1].strValue.c_str(), params[2].GetLong());
			return true;
		}
	};
	struct CStrcat : public CPlugin
	{
		CStrcat() :CPlugin("strcat", Variable::STRING) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "字符串连接，2个参数，参数必须是字符串\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 2)msg += "参数不是二个";
			if (params.size() >= 1 && !params[0].isString())msg += "参数必须是字符串";
			if (params.size() >= 2 && !params[1].isString())msg += "参数必须是字符串";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = params[0].strValue + params[1].strValue;
			return true;
		}
	};
	struct CStrstr : public CPlugin
	{
		CStrstr() :CPlugin("strstr", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "查找子串，2个参数，参数必须是字符串，返回参数2在参数1中出现的位置，若没找到则返回-1\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 2)msg += "参数不是二个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			if (params.size() >= 2 && !params[1].isString())msg += "参数2必须是字符串\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			char const* tmp = strstr(params[0].strValue.c_str(), params[1].strValue.c_str());
			if (NULL != tmp)ret = (long)(tmp - params[0].strValue.c_str());
			else ret = -1L;
			return true;
		}
	};
	struct CSubstr : public CPlugin
	{
		CSubstr() :CPlugin("substr", Variable::STRING) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "获取子串，2-3个参数，参数必须是数值，参数2为开始位置，参数3为字符数，省略参数3则一直取到字符串结束\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 2 && params.size() != 3)msg += "参数不是2-3个\r\n";
			if (params.size() >= 1 && !params[0].isString())msg += "参数1必须是字符串\r\n";
			if (params.size() >= 2 && !params[1].isNumber())msg += "参数2必须是数值\r\n";
			if (params.size() >= 3 && !params[2].isNumber())msg += "参数2必须是数值\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			if (params.size() >= 3)ret = params[0].strValue.substr(params[1].GetLong(), params[2].GetLong());
			else ret = params[0].strValue.substr(params[1].GetLong());
			return true;
		}
	};
	struct CToLong : public CPlugin
	{
		CToLong() :CPlugin("to_long", Variable::LONG) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "转换为整数，一个参数，任意类型\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是1个\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = params[0].GetLong();
			return true;
		}
	};
	struct CToDouble : public CPlugin
	{
		CToDouble() :CPlugin("to_double", Variable::DOUBLE) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "转换为浮点数，一个参数，任意类型\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是1个\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = params[0].GetDouble();
			return true;
		}
	};
	struct CToString : public CPlugin
	{
		CToString() :CPlugin("to_string", Variable::STRING) {}
		virtual string& help(string& ret)
		{
			ret = CPlugin::help(ret);
			ret += "转换为字符串，一个参数，任意类型\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg)
		{
			msg = "";
			if (params.size() != 1)msg += "参数不是1个\r\n";
			return 0 == msg.size();
		}
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe)
		{
			ret = params[0].GetString();
			return true;
		}
	};
}
