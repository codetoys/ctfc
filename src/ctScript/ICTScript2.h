//ICTScript2.h 脚本解释器接口头文件
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

//脚本解释器 以简单C++语法为基础 做了少量扩展
//语法说明见下面的LanguageHelp函数
//内置函数说明运行时用PluginHelp函数获得

#include "../env/env.h"
#include "../env/myLog.h"
using namespace ns_my_std;

namespace ns_my_script2
{
	class CHelp
	{
	public:
		static string& LanguageHelp(string& ret)
		{
			ret = "脚本编译器 2.0 R1 2019-11-20 测试版\r\n"
				"ct 版权所有，保留所有权利\r\n"
				"\r\n"
				"带有预编译功能的脚本解释器 以简单C++语法为基础 做了少量扩展\r\n"
				"未支持的关键字、运算符全部保留，不可用做标识符\r\n"
				"\r\n"
				"已经支持的语法：\r\n"
				"语句：if else return {} for do while break continue;\r\n"
				"转义字符：C++标准\r\n"
				"注释：//到行尾\r\n"
				"\r\n"
				"增加的关键字：string\r\n"
				"\r\n"
				"强数据类型：\r\n"
				"整数：int long\r\n"
				"浮点数：float double\r\n"
				"字符串：string\r\n"
				"数值常量：C++标准\r\n"
				"字符串常量：双引号单引号均可\r\n"
				"不允许数值和字符串的自动化转换\r\n"
				"算术运算 + - * / % ++ --\r\n"
				"逻辑运算 ! > < >= <= == !=\r\n"
				"关系运算 && ||\r\n"
				"赋值运算 = += -= *= /= %=\r\n"
				"逗号运算 ,\r\n"
				"+可用作字符串连接\r\n"
				"++ --前缀每个变量只能用一次，后缀只能独立使用\r\n"
				"\r\n"
				"每个声明语句只能声明一个变量，可初始化\r\n"
				"for语句的初始化子句可以声明变量\r\n"
				"不允许变量覆盖（不同的同级块内变量可以同名）\r\n"
				"\r\n"
				"最后执行的表达式结果作为返回值，包括循环的判断语句\r\n"
				"空语句返回空\r\n"
				"if语句若无匹配语句返回空\r\n"
				"break continue不影响返回值\r\n"
				"声明语句返回变量值\r\n"
				"复杂逻辑下应使用return语句\r\n"
				"\r\n"
				"不打算支持的语法：\r\n"
				"源代码的三字母序列\r\n"
				"?: 位运算 * & sizeof [] -> . (类型)\r\n"
				"switch goto\r\n"
				"\r\n"
				"内部函数支持常用的数值转换和字符串操作\r\n"
				"\r\n"
				"支持函数，函数只能使用之前已经定义的全局变量和环境变量，参数都是值传递，不支持默认值\r\n"
				;
			return ret;
		}
		static string& Example(string& ret)
		{
			ret = "示例1 简单表达式：\r\n"
				"    123+4.5*3-max(1,2,3,0)\r\n"
				"\r\n"
				"示例2 简单逻辑：\r\n"
				"    if(12>5)12+5;else 12*5\r\n"
				"\r\n"
				"示例3 复杂逻辑，注意确保每个执行路径都返回有意义的结果：\r\n"
				"    int i;//int与long同义\r\n"
				"    double d=1.5+max(1,2,3);//float与double同义\r\n"
				"    string s=to_string(d);\r\n"
				"    \r\n"
				"    do       //do循环\r\n"
				"    {\r\n"
				"        ++i;\r\n"
				"    }while(i<10);\r\n"
				"    while(i>0)      //while循环\r\n"
				"    {\r\n"
				"        i-=d;\r\n"
				"    }\r\n"
				"    for(i=0;i<5;++i)   //for循环\r\n"
				"    {\r\n"
				"        if(strlen(s+'abc')>=3)  //if语句 if()...或if()...else...或if()...else if()...else...可以有多个else if\r\n"
				"        {\r\n"
				"            continue;\r\n"
				"        }\r\n"
				"        else\r\n"
				"        {\r\n"
				"            break;\r\n"
				"        }\r\n"
				"    }\r\n"
				"    return i+d;\r\n"
				;
			return ret;
		}
	};
	enum { TOKEN_BUF_LEN = 128 };//仅用于预定义的关键字、运算符，其它标识符任意长度
	//语法标记，去除空白之后的每个元素
	struct Token
	{
		enum types { DELIMITER = 0, OPERATOR, IDENTIFIER, NUMBER, KEYWORD, STRING };

		types type;//类型
		string text;//文本
		size_t pos;//在源代码中的位置

		Token(types _type, char const* _text, size_t _pos) :type(_type), text(_text), pos(_pos) {}

		string ToString()const
		{
			STATIC_C const char typestr[][TOKEN_BUF_LEN] = { "DELIMITER","OPERATOR","IDENTIFIER","NUMBER","KEYWORD","STRING" };//必须与types对应
			char buf[TOKEN_BUF_LEN * 2];
			string ret;
			sprintf(buf, "%03ld %-12s ", pos, typestr[type]);
			ret = buf;
			ret += text.c_str();
			return ret;
		}
	};
	//变量
	struct Variable
	{
		enum types { NULLVARIABLE = 0, LONG, DOUBLE, STRING };
		types type;
		bool isconst;
		long lValue;
		double dValue;
		string strValue;

		Variable() :type(NULLVARIABLE), isconst(false), lValue(0), dValue(0.) {}
		bool isNull() { return type == NULLVARIABLE; }
		bool isNumber() { return type == LONG || type == DOUBLE; }
		bool isString() { return type == STRING; }
		void clear()
		{
			type = NULLVARIABLE;
			isconst = false;
			lValue = 0;
			dValue = 0;
			strValue = "";
		}
		void initvalue()
		{
			lValue = 0;
			dValue = 0;
			strValue = "";
		}
		Variable& operator = (long v)
		{
			char buf[256];
			if (NULLVARIABLE == type)type = LONG;
			switch (type)
			{
			case LONG:lValue = v; break;
			case DOUBLE:dValue = v; break;
			case STRING:
				sprintf(buf, "%ld", v);
				strValue = buf;
				break;
			default:break;
			}
			return *this;
		}
		Variable& operator = (double v)
		{
			char buf[256];
			if (NULLVARIABLE == type)type = DOUBLE;
			switch (type)
			{
			case LONG:lValue = (long)v; break;
			case DOUBLE:dValue = v; break;
			case STRING:
				gcvt(v, 200, buf);
				strValue = buf;
				break;
			default:break;
			}
			return *this;
		}
		Variable& operator = (string const& v)
		{
			if (NULLVARIABLE == type)type = STRING;
			switch (type)
			{
			case LONG:lValue = atol(v.c_str()); break;
			case DOUBLE:dValue = atof(v.c_str()); break;
			case STRING:strValue = v; break;
			default:break;
			}
			return *this;
		}
		Variable& operator = (Variable const& v)
		{
			if (NULLVARIABLE == type)type = v.type;
			switch (type)
			{
			case LONG:lValue = v.GetLong(); break;
			case DOUBLE:dValue = v.GetDouble(); break;
			case STRING:strValue = v.GetString(); break;
			default:break;
			}
			return *this;
		}
		Variable operator-()const
		{
			Variable tmp = *this;
			switch (type)
			{
			case LONG:tmp.lValue = -lValue; break;
			case DOUBLE:tmp.dValue = -dValue; break;
			default:break;
			}
			return tmp;
		}
		//eva=true则是为赋值提升，结果以左边为准
		static types typeUpgrade(types a, types b, bool eva = false)
		{
			if (NULLVARIABLE == a || NULLVARIABLE == b)return NULLVARIABLE;
			if (LONG == a && LONG == b)return LONG;
			if (STRING == a && STRING == b)return STRING;
			if (DOUBLE == a && DOUBLE == b)return DOUBLE;
			if (!eva)
			{
				if (DOUBLE == a && LONG == b)return DOUBLE;
				if (LONG == a && DOUBLE == b)return DOUBLE;
			}
			else
			{
				if (DOUBLE == a && LONG == b)return DOUBLE;
				if (LONG == a && DOUBLE == b)return LONG;
			}
			return NULLVARIABLE;
		}
		long GetLong()const
		{
			string tmp;
			switch (type)
			{
			case LONG: return lValue;
			case DOUBLE: return (long)dValue;
			case STRING: tmp = strValue; return atol(tmp.c_str());
			default:return 0;
			}
		}
		double GetDouble()const
		{
			string tmp;
			switch (type)
			{
			case LONG: return lValue;
			case DOUBLE: return dValue;
			case STRING: tmp = strValue; return atof(tmp.c_str());
			default:return 0.;
			}
		}
		bool GetBool()const
		{
			switch (type)
			{
			case LONG: return 0 != lValue;
			case DOUBLE: return 0 != dValue;
			default:return false;
			}
		}
		string GetString()const
		{
			char buf[256];
			switch (type)
			{
			case LONG: sprintf(buf, "%ld", lValue); return buf;
			case DOUBLE: gcvt(dValue, 200, buf); return buf;
			case STRING: return strValue;
			default:return "";
			}
		}
		Variable operator+(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = typeUpgrade(type, b.type);
			switch (tmp.type)
			{
			case LONG:tmp.lValue = GetLong() + b.GetLong(); break;
			case DOUBLE:tmp.dValue = GetDouble() + b.GetDouble(); break;
			case STRING:tmp.strValue = GetString() + b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator-(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = typeUpgrade(type, b.type);
			switch (tmp.type)
			{
			case LONG:tmp.lValue = GetLong() - b.GetLong(); break;
			case DOUBLE:tmp.dValue = GetDouble() - b.GetDouble(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator*(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = typeUpgrade(type, b.type);
			switch (tmp.type)
			{
			case LONG:tmp.lValue = GetLong() * b.GetLong(); break;
			case DOUBLE:tmp.dValue = GetDouble() * b.GetDouble(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator/(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = typeUpgrade(type, b.type);
			switch (tmp.type)
			{
			case LONG:
			{
				if (0 == b.GetLong())throw "div zero";
				tmp.lValue = GetLong() / b.GetLong(); break;
			}
			case DOUBLE:
			{
				if (0 == b.GetDouble())throw "div zero";
				tmp.dValue = GetDouble() / b.GetDouble(); break;
			}
			default:break;
			}
			return tmp;
		}
		Variable operator%(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = typeUpgrade(type, b.type);
			switch (tmp.type)
			{
			case LONG:
			{
				if (0 == b.GetLong())throw "mod zero";
				tmp.lValue = GetLong() % b.GetLong(); break;
			}
			default:break;
			}
			return tmp;
		}
		Variable operator>(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() > b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() > b.GetDouble(); break;
			case STRING:tmp.lValue = GetString() > b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator<(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() < b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() < b.GetDouble(); break;
			case STRING:tmp.lValue = GetString() < b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator>=(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() >= b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() >= b.GetDouble(); break;
			case STRING:tmp.lValue = GetString() >= b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator<=(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() <= b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() <= b.GetDouble(); break;
			case STRING:tmp.lValue = GetString() <= b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator==(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() == b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() == b.GetDouble(); break;
			case STRING:tmp.lValue = GetString() == b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator!=(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() != b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() != b.GetDouble(); break;
			case STRING:tmp.lValue = GetString() != b.GetString(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator&&(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() && b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() && b.GetDouble(); break;
			default:break;
			}
			return tmp;
		}
		Variable operator||(Variable const& b)const
		{
			Variable tmp = *this;
			tmp.type = LONG;
			switch (typeUpgrade(type, b.type))
			{
			case LONG:tmp.lValue = GetLong() || b.GetLong(); break;
			case DOUBLE:tmp.lValue = GetDouble() || b.GetDouble(); break;
			default:break;
			}
			return tmp;
		}
		static char const* TypeStr(types type)
		{
			STATIC_C const char typestr[][TOKEN_BUF_LEN] = { "NULLVARIABLE","LONG","DOUBLE","STRING" };//必须与types对应
			if(type>=0 && type<4)return typestr[type];
			else
			{
				static char buf[256];
				sprintf(buf, "错误的类型 %d", type);
				thelog << buf << ende; exit(0);
				return buf;
			}
		}
		string ToString(long level = 0)const
		{
			string ret;
			char buf[256];
			string prefix;
			prefix.assign(level * 4, ' ');
			switch (type)
			{
			case LONG:sprintf(buf, "%ld", lValue); break;
			case DOUBLE:gcvt(dValue, 200, buf); break;
			case STRING:strcpy(buf, strValue.c_str()); break;
			default:sprintf(buf, "NULL"); break;
			}
			ret = prefix + " ";
			ret += (isconst ? "常量" : "变量");
			ret += "类型 ";
			ret += TypeStr(type);
			ret += " : ";
			ret += buf;
			return ret;
		}
	};
	//插件
	struct CPlugin
	{
		string plugin_name;
		Variable::types plugin_return_type;
		CPlugin() :plugin_name(""), plugin_return_type(Variable::NULLVARIABLE) {}
		CPlugin(char const* _name, Variable::types _type) :plugin_name(_name), plugin_return_type(_type) {}
		virtual string& help(string& ret)
		{
			ret = plugin_name + " ： 返回值 " + Variable::TypeStr(plugin_return_type) + "\r\n";
			return ret;
		}
		virtual bool CheckPlugin(CMyVector<Variable >& params, void*& pc, string& msg) = 0;
		virtual bool ExecFunction(CMyVector<Variable >& params, void* const& pc, Variable& ret, string& msg, void* pe) = 0;
	};
	//插件表
	class CPluginMap
	{
	public:
		struct HANDLE
		{
			string plugin_name;

			bool isNULL()const { return 0 == plugin_name.size(); }
		};
	private:
		static map<string, CPlugin*>& GetPluginMap();
	public:
		template<typename T>
		static void addplugin(map<string, CPlugin*>& mapPlugins);
		static bool AddPlugin(char const* name, Variable::types type, CPlugin* p);
		static CPlugin* GetPlugin(string const& fun_name);
		static CPlugin* GetPlugin(HANDLE const& h);
		static string& PluginHelp(string& ret);
	};
	//脚本解释器接口
	class CCTScript;
	class ICTScript
	{
	private:
		CCTScript* m_p;
		string m_msg;
	public:
		ICTScript() :m_p(NULL) {}
		string GetMessage()const;

		//连接
		bool AttachScript(CCTScript* p);

		//编译
		bool Compile(char const* _source, CMyVector<pair<string, Variable > >* pEnvs = NULL);

		//执行
		string const& GetSource()const;
		bool IsCompiled()const;
		string& Report(string& ret)const;
		bool Execute(Variable& ret, CMyVector<pair<string, Variable > >* pEnvs = NULL, void* pe = NULL);
		long GetExecCount()const;
	};
}
