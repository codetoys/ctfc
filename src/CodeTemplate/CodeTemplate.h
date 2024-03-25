//CreateTemplate.h 代码模板系统
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
using namespace ns_my_std;

//objname脱壳
string _makeobjname(string objname)
{
	string ret = objname;
	if (ret.substr(0, 2) == "${")ret = ret.substr(2);
	if (ret.size() > 0 && ret[ret.size() - 1] == '}')ret.erase(ret.size() - 1);

	//thelog << objname << " " << ret << endi;
	return ret;
}
//计算行号
long _getLineNumber(char const* str, long pos)
{
	char const* p = str;
	long ret = 1;
	char last = '\0';
	while (*p != '\0' && p != str + pos)
	{
		if ('\r' == *p)
		{
			++ret;
		}
		if ('\n' == *p && last != '\r')
		{
			++ret;
		}
		last = *p;
		++p;
	}
	return ret;
}
//移动到行首（如果只有空白字符），如果失败返回-1
long _moveToHead(char const* str, long pos)
{
	long ret = pos;
	while (ret > 0)
	{
		if (' ' == str[ret - 1] || '\t' == str[ret - 1])
		{
			//thelog << "_moveToHead 空白 "<< ret-1 << endi;
			--ret;
		}
		else
		{
			break;
		}
	}
	//thelog << "_moveToHead " << ret << " [" << (int)(str[ret]) << "]" << endi;
	if (0 == ret || '\n' == str[ret - 1])return ret;
	else return -1;
}
//移动到下一行（如果只有空白字符），如果失败返回-1
long _moveToNextLine(char const* str, long pos)
{
	long ret = pos;
	while (str[ret] != '\0')
	{
		if (' ' == str[ret] || '\t' == str[ret])
		{
			//thelog << "_moveToNextLine 空白 " <<ret<< endi;
			++ret;
		}
		else
		{
			break;
		}
	}
	if ('\r' == str[ret])
	{
		//thelog << "_moveToNextLine \\r "<<ret << endi;
		++ret;
	}
	if ('\n' == str[ret])
	{
		//thelog << "_moveToNextLine \\n "<<ret << endi;
		++ret;
	}
	//thelog << "_moveToNextLine " << ret << " [" << (int)(str[ret]) << "]" << endi;
	if ('\0' == str[ret] || '\r' == str[ret - 1] || '\n' == str[ret - 1])return ret;
	else return -1;
}
void _extendToLine(char const* str, size_t& head, size_t& tail)
{
	long tmp = _moveToHead(str, head);
	long tmp2 = _moveToNextLine(str, tail);
	if (tmp != -1 && tmp2 != -1)
	{//必须前后都是独占
		head = tmp;
		tail = tmp2;
	}
	return;
}
struct CCTObject
{
	enum TYPE { OBJECT, POINTER, PROPERTY, ARRAY };
	TYPE m_type;//根据type决定哪个成员有效
	CCTObject* m_Pointer;//指针
	string m_Property;//属性（值）
	map<string, CCTObject > m_Object;//对象
	vector<CCTObject >  m_Array;//数组

	CCTObject() :m_type(OBJECT), m_Pointer(NULL) {}

	string GetDefaultValue()
	{
		if (PROPERTY == m_type)
		{
			return m_Property;
		}
		else if (OBJECT == m_type)
		{
			return m_Object["name"].GetDefaultValue();
		}
		else if (POINTER == m_type)
		{
			return m_Pointer->GetDefaultValue();
		}
		else if (ARRAY == m_type)
		{
			return "ARRAY";
		}
		else
		{
			return "unknown type";
		}
	}
	string SetDefaultValue(char const* value)
	{
		if (PROPERTY == m_type)
		{
			return m_Property = value;
		}
		else if (OBJECT == m_type)
		{
			SetObjectAddProperty("name", value);
			return value;
		}
		else if (POINTER == m_type)
		{
			return m_Pointer->SetDefaultValue(value);
		}
		else if (ARRAY == m_type)
		{
			return "ARRAY";
		}
		else
		{
			return "unknown type";
		}
	}
	void SetObjectAddProperty(string name, long value)
	{
		char buf[64];
		sprintf(buf, "%ld", value);
		SetObjectAddProperty(name, buf);
	}
	void SetObjectAddProperty(string name, string value)
	{
		CCTObject p;
		p.m_type = PROPERTY;
		p.m_Property = value;
		m_Object[name] = p;
	}
	void SetProperty(string value)
	{
		m_type = PROPERTY;
		m_Property = value;
	}
	void SetProperty(int value)
	{
		m_type = PROPERTY;
		char buf[256];
		sprintf(buf, "%d", value);
		m_Property = buf;
	}
	void SetPointer(CCTObject* value)
	{
		m_type = POINTER;
		m_Pointer = value;
	}
	void SetArrayPushBack(CCTObject value)
	{
		m_type = ARRAY;
		m_Array.push_back(value);
	}
	//给对象初始化一个拥有的数组
	void SetObjectArrayInit(string name)
	{
		m_type = OBJECT;
		CCTObject tmp;
		tmp.m_type = ARRAY;
		m_Object[name] = tmp;
	}
	//给对象拥有的对象数组添加对象
	void SetObjectArrayPushBack(string name, CCTObject value)
	{
		m_type = OBJECT;
		m_Object[name].SetArrayPushBack(value);
	}
	//给对象拥有的属性数组添加属性
	void SetObjectArrayPushBack(string name, string value)
	{
		m_type = OBJECT;
		CCTObject tmp;
		tmp.SetProperty(value);
		m_Object[name].SetArrayPushBack(tmp);
	}
	void SetObjectAdd(string const& name, CCTObject const& value)
	{
		m_type = OBJECT;
		m_Object[name] = value;
	}
	void SetObjectAddRef(string const& name, CCTObject * value)
	{
		m_type = OBJECT;
		CCTObject tmp;
		tmp.SetPointer(value);
		m_Object[name] = tmp;
	}
	//查找对象
	CCTObject* FindObject(string const& objname)
	{
		string str;
		long index = -1;
		StringTokenizer st(objname, ".");
		CCTObject* ret = this;
		DEBUG_LOG << objname << " st.size " << st.size() << endi;
		for (size_t i = 0; i < st.size(); ++i)
		{
			DEBUG_LOG << st[i] << endi;
			while (POINTER == ret->m_type)ret = ret->m_Pointer;

			size_t pos = st[i].find("[");
			if (pos != string::npos)
			{
				index = atol(st[i].substr(pos + 1).c_str());
				str = st[i].substr(0, pos);
			}
			else
			{
				index = -1;
				str = st[i];
			}
			if (PROPERTY == ret->m_type)
			{
				thelog << str << " 不是对象" << ende;
				return NULL;
			}
			else if (OBJECT == ret->m_type)
			{
				if (ret->m_Object.find(str) != ret->m_Object.end())
				{
					ret = &ret->m_Object.find(str)->second;
					if (index >= 0)
					{
						if (ARRAY != ret->m_type)
						{
							thelog << str << " 不是数组" << ende;
							return NULL;
						}
						ret = &ret->m_Array[index];
					}
				}
				else
				{
					DEBUG_LOG << str << " 不存在 " << objname << ende;
					return NULL;
				}
			}
			else
			{
				thelog << str << " 未知的类型 " << ret->m_type << ende;
				return NULL;
			}
		}
		return ret;
	}
	//deep=-1不限深度，自身deep=0
	string tostring(string& ret, long level, long deep)const
	{
		string px;
		long n;
		for (long i = 0; i < level; ++i)px += "\t";
		stringstream ss;

		if (!(deep < 0 || level < deep))
		{
			return "";
		}

		if (POINTER == m_type)
		{
			ss << px << "---->>>> :" << endl;
			string str;
			ss << tostring(str, level + 2, deep);
		}
		else if (PROPERTY == m_type)
		{
			ss << px << "PROPERTY " << m_Property << endl;
		}
		else if (ARRAY == m_type)
		{
			ss << px << "ARRAY size " << m_Array.size() << endl;
			n = 1;
			for (vector<CCTObject >::const_iterator it = m_Array.begin(); it != m_Array.end(); ++it)
			{
				ss << px << "\t" << n << endl;
				string str;
				ss << it->tostring(str, level + 2, deep);
				++n;
			}
		}
		else if (OBJECT == m_type)
		{
			if (0 == m_Object.size())
			{
				ss << px << "empty" << endl;
			}
			for (long i = 0; i < 3; ++i)
			{
				n = 1;
				for (map<string, CCTObject >::const_iterator it = m_Object.begin(); it != m_Object.end(); ++it)
				{
					if (0 == i && PROPERTY != it->second.m_type)continue;
					if (1 == i && ARRAY != it->second.m_type)continue;
					if (2 == i && OBJECT != it->second.m_type)continue;
					string str;
					if (PROPERTY != it->second.m_type)
					{
						ss << px << n << "\t" << it->first << endl;
						ss << it->second.tostring(str, level + 2, deep);
					}
					else
					{
						ss << px << n << "\t" << it->first << " \t PROPERTY \t" << it->second.m_Property << endl;
					}

					++n;
				}
			}
		}
		else
		{
			ss << px << "此类型不能输出 " << m_type << endl;
		}

		return ss.str();
	}
	string& toString(string& ret)const
	{
		return ret = tostring(ret, 0, -1);
	}
};
struct CCTStack : public vector<CCTObject >
{
	void Push()
	{
		CCTObject o;
		this->push_back(o);
	}
	void Pop()
	{
		this->pop_back();
	}
	bool AddRef(string const& objname, CCTObject * pObj)
	{
		if (0 == this->size())return false;
		this->rbegin()->SetObjectAddRef(objname, pObj);
		return true;
	}
	bool Add(string const& objname, CCTObject const& obj)
	{
		if (0 == this->size())return false;
		this->rbegin()->SetObjectAdd(objname, obj);
		return true;
	}
	bool AddProperty(string const& objname, string const& obj)
	{
		if (0 == this->size())return false;
		this->rbegin()->SetObjectAddProperty(objname, obj);
		return true;
	}

	string& toString(string& ret)const
	{
		string str;
		stringstream ss;
		for (const_iterator it = begin(); it != end(); ++it)
		{
			ss << "===============================" << endl;
			ss << it->toString(str) << endl;
		}
		return ret = ss.str();
	}
};

class CCodeTemplate
{
private:
	map<string, string > m_functions;//定义的函数

	CCTObject* _FindObject(string _objname, CCTObject& O, CCTStack& S)
	{
		return __FindObject(_objname, O, S, false);
	}
	CCTObject* _tryFindObject(string _objname, CCTObject& O, CCTStack& S)
	{
		return __FindObject(_objname, O, S, true);
	}
	CCTObject* __FindObject(string _objname, CCTObject& O, CCTStack& S, bool isTry)
	{
		//G_IS_DEBUG = true;
		string objname = _makeobjname(_objname);//脱壳
		CCTObject* ret = NULL;
		DEBUG_LOG << "查找 " << objname << " " << S.size() << endi;
		for (long i = S.size() - 1; i >= 0; --i)
		{
			DEBUG_LOG << "S... " << endi;
			ret = S[i].FindObject(objname);
			if (ret != NULL)
			{
				DEBUG_LOG << "找到 " << endi;
				return ret;
			}
		}
		DEBUG_LOG << "O... " << endi;
		ret = O.FindObject(objname);
		if (NULL == ret)
		{
			if (!isTry)thelog << "对象不存在 " << objname << endw;
			//string str;
			//thelog << endl << O.toString(str) << endi;
			//for (CCTStack::reverse_iterator it = S.rbegin(); it != S.rend(); ++it)
			//{
			//	thelog << endl << it->toString(str) << endi;
			//}
		}
		return ret;
	}
	//获得对象值，可能是对象的默认值，也可能是文本
	bool _GetObjectValue(string _objname, CCTObject& O, CCTStack& S, bool isTry, string& ret)
	{
		if (0 == _objname.size())return "";

		if ('\"' == _objname[0])
		{
			if (_objname.size() < 2)
			{
				return false;
			}
			if ('\"' != _objname[_objname.size() - 1])
			{
				return false;
			}
			ret = _objname.substr(1, _objname.size() - 2);
			return true;
		}
		else if (_objname[0] >= '0' && _objname[0] <= '9')
		{
			ret = _objname;
			return true;
		}
		else
		{
			CCTObject* p = __FindObject(_objname, O, S, isTry);
			if (NULL == p)
			{
				return false;
			}
			else
			{
				ret = p->GetDefaultValue();
				return true;
			}
		}
	}
	//处理没有控制块的纯替换
	bool _ProcessNoBlock(string const& _source, long start, long end, stringstream& ss, CCTObject& O, CCTStack& S)
	{
		string source = _source.substr(start, end - start);
		DEBUG_LOG << "---------------------------------- " << start << " " << end << endl << source << endi;
		size_t pos = 0;//标记开始处
		size_t pos2 = 0;//指向未处理部分
		while (string::npos != (pos = source.find("${", pos2)))
		{
			ss << source.substr(pos2, pos - pos2);
			pos2 = source.find("}", pos + 2);
			if (source.npos == pos2)
			{
				thelog << "没找到匹配的 } ：" << source.substr(pos) << ende;
				return false;
			}

			string objname = source.substr(pos, pos2 + 1 - pos);
			bool INCafterUsed = false;//带有后++
			objname = _makeobjname(objname);//脱壳
			if (objname.size() > 2 && "++" == objname.substr(objname.size() - 2))
			{
				INCafterUsed = true;
				objname.erase(objname.size() - 2);
			}

			if ("__LOG__" == objname)
			{
				ss << "thelog << " << _getLineNumber(_source.c_str(), start + pos) << " << \" \"";
			}
			else if ("__DEBUG_LOG__" == objname)
			{
				ss << "DEBUG_LOG << " << _getLineNumber(_source.c_str(), start + pos) << " << \" \"";
			}
			else
			{
				CCTObject* p = _FindObject(objname, O, S);
				if (NULL == p)
				{
					thelog << "对象不存在 [" << objname << "] line : " << _getLineNumber(_source.c_str(), pos + start) << ende;
					return false;
				}
				DEBUG_LOG << "line : " << _getLineNumber(_source.c_str(), pos + start) << " " << objname << " " << p->GetDefaultValue() << endi;
				ss << p->GetDefaultValue();

				if (INCafterUsed)
				{
					char buf[64];
					sprintf(buf, "%ld", atol(p->GetDefaultValue().c_str()) + 1);
					p->SetDefaultValue(buf);
				}
			}

			pos2 = pos2 + 1;
		}
		ss << source.substr(pos2);
		DEBUG_LOG << "-------------------" << endi;
		return true;
	}
	//寻找块结束，end1 end2返回扩展后的结束标签
	bool _findBlockEnd(string const& source, size_t start, size_t end, size_t& end1, size_t& end2, char const* labelBegin, char const* labelEnd)
	{
		return __findBlockEnd(source, start, end, end1, end2, labelBegin, labelEnd, false);
	}
	bool _TryFindBlockEnd(string const& source, size_t start, size_t end, size_t& end1, size_t& end2, char const* labelBegin, char const* labelEnd)
	{
		return __findBlockEnd(source, start, end, end1, end2, labelBegin, labelEnd, true);
	}
	bool __findBlockEnd(string const& source, size_t start, size_t end, size_t& end1, size_t& end2, char const* labelBegin, char const* labelEnd, bool isTry)
	{
		long level = 1;
		string matchlable;
		end1 = start;
		end2 = start;
		while (level > 0)
		{
			end1 = source.find("<%", end2);
			if (string::npos == end1 || end1 >= end)
			{
				if (!isTry)thelog << "控制块不匹配 level " << level << " " << labelBegin << " " << labelEnd << " 行 " << _getLineNumber(source.c_str(), start) << ende;
				//exit(0);
				return false;
			}
			end2 = source.find("%>", end1 + 2);
			if (string::npos == end2 || end2 >= end)
			{
				thelog << "没找到匹配的 %> : " << source.substr(end1) << ende;
				return false;
			}
			string blockcode = source.substr(end1 + 2, end2 - (end1 + 2));
			StringTokenizer st(blockcode, " ");
			DEBUG_LOG << "level " << level << " " << st[0] << " 行：" << _getLineNumber(source.c_str(), end1) << endi;
			if (st.size() > 0)
			{
				if ((string)"else" == labelEnd)
				{
					if ((string)"else" == st[0] && 1 == level)
					{//找else只能在最后一层时降级
						DEBUG_LOG << "level=1时找到else" << endi;
						matchlable = st[0];
						--level;
					}
					if ((string)"endif" == st[0])
					{
						DEBUG_LOG << "找else时遇到endif" << endi;
						matchlable = st[0];
						--level;
					}
				}
				else if (labelEnd == st[0])
				{
					DEBUG_LOG << "找到结束标记" << endi;
					matchlable = st[0];
					--level;
				}
				if (labelBegin == st[0])
				{
					DEBUG_LOG << "找到开始标记" << endi;
					matchlable = st[0];
					++level;
				}
			}
			end2 += 2;
		}
		if (0 == level && labelEnd == matchlable)
		{
			_extendToLine(source.c_str(), end1, end2);
			DEBUG_LOG << "找到块 " << labelBegin << " " << labelEnd << " 行 " << _getLineNumber(source.c_str(), end1) << " " << source.substr(end1, end2 - end1) << endi;
			return true;
		}
		else
		{
			if (labelEnd != (string)"else")thelog << "控制块不匹配" << labelBegin << " " << labelEnd << " 行 " << _getLineNumber(source.c_str(), start) << ende;
			return false;
		}
	}
	//处理表达式，双引号引导为纯文本，单引号引导为脚本，没有引号先判断是不是对象再当作脚本，如果是对象则p有效
	string _ProcessExpression(CCTObject& O, CCTStack& S, string const& expression, CCTObject*& p)
	{
		//thelog << expression << endi;
		p = nullptr;
		if (expression.size() >= 2 && '\"' == expression[0] && '\"' == expression[expression.size() - 1])
		{
			//thelog << varname<<" "<< objname.substr(1, objname.size() - 2) <<endi;
			return expression.substr(1, expression.size() - 2);
		}
		stringstream tmp_ss;
		if (expression.size() >= 2 && '\'' == expression[0] && '\'' == expression[expression.size() - 1])
		{
			//thelog << expression << endi;
			if (_ProcessBlock(expression.substr(1, expression.size() - 2), 0, expression.size() - 2, tmp_ss, O, S))
			{
				//thelog << tmp_ss.str() << endi;
				return tmp_ss.str();
			}
			else
			{
				return expression + " 对象不存在或脚本出错 ";
			}
		}
		else
		{
			p = _tryFindObject(expression, O, S);
			if (NULL != p)
			{
				return "";
			}
			if (_ProcessBlock(expression, 0, expression.size(), tmp_ss, O, S))
			{
				return tmp_ss.str();
			}
			else
			{
				return expression + " 对象不存在或脚本出错 ";
			}
		}
	}
	//处理控制块
	bool _ProcessBlock(string const& source, size_t start, size_t const end, stringstream& ss, CCTObject& O, CCTStack& S)
	{
		string str;
		DEBUG_LOG << "==================================================== " << start << " " << end << endi;
		DEBUG_LOG << "line : " << _getLineNumber(source.c_str(), start) << " - " << _getLineNumber(source.c_str(), end) << endi;
		DEBUG_LOG << endl << source.substr(start, end - start) << endi;

		size_t pos = source.find("<%", start);
		DEBUG_LOG << "line : " << _getLineNumber(source.c_str(), pos) << endi;

		if (string::npos == pos || pos >= end)
		{//没有控制块
			return _ProcessNoBlock(source, start, end, ss, O, S);
		}
		size_t pos2 = source.find("%>", pos + 2);
		if (string::npos == pos2 || pos2 >= end)
		{
			thelog << "没找到匹配的 %> : " << source.substr(pos) << " line : " << _getLineNumber(source.c_str(), pos) << ende;
			return false;
		}
		string blockcode = source.substr(pos + 2, pos2 - (pos + 2));
		DEBUG_LOG << blockcode << endi;

		pos2 = pos2 + 2;
		_extendToLine(source.c_str(), pos, pos2);

		DEBUG_LOG << " line : " << _getLineNumber(source.c_str(), pos) << " : " << source.substr(pos, pos2 - pos) << endi;

		//首先处理控制块之前的NoBlock
		if (!_ProcessNoBlock(source, start, pos, ss, O, S))return false;

		//再处理控制部分
		StringTokenizer st(blockcode, " ", true, true);
		if ("replace" == st[0])
		{
			//语法：replace 删除整个块
			//语法：replace object 用对象替换整个块
			size_t end1, end2;
			if (!_findBlockEnd(source, pos2, end, end1, end2, "replace", "endreplace"))
			{
				return false;
			}
			if (st.size() > 1)
			{
				string objname = st[1];
				string value;
				if (!_GetObjectValue(objname, O, S, false, value))
				{
					thelog << "对象值不存在" << objname << " line : " << _getLineNumber(source.c_str(), pos) << ende;
					return false;
				}
				ss << value;
			}
			//处理end之后的部分
			return _ProcessBlock(source, end2, end, ss, O, S);
		}
		else if ("function" == st[0])
		{
			//语法：function name 定义函数
			size_t end1, end2;
			if (!_findBlockEnd(source, pos2, end, end1, end2, "function", "endfunction"))
			{
				return false;
			}
			if (st.size() < 2)
			{
				thelog << "缺少函数名" << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			string name = st[1];
			string body = source.substr(pos2, end1 - pos2);
			if (m_functions.find(name) != m_functions.end())
			{
				thelog << "函数已经定义 " << name << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			m_functions[name] = body;

			//处理end之后的部分
			return _ProcessBlock(source, end2, end, ss, O, S);
		}
		else if ("call" == st[0])
		{
			//语法：call 函数名 (参数应该添加到S中)
			if (st.size() < 2)
			{
				thelog << "缺少函数名" << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			string name = st[1];
			auto it = m_functions.find(name);
			if (it == m_functions.end())
			{
				thelog << "函数未定义 " << name << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			string body = it->second;

			//处理函数
			S.Push();
			if (!_ProcessBlock(body, 0, body.size(), ss, O, S))
			{
				thelog << "函数处理出错 " << name << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			S.Pop();

			//处理函数之后的部分
			return _ProcessBlock(source, pos2, end, ss, O, S);
		}
		else if ("foreach" == st[0])
		{
			//语法 foreach 对象名（是个引用） in 数组对象 [连接符，追加在输出中，仅用于中间]
			//语法 foreach 对象名（是个引用） in1 数组对象(从下标1开始，跳过第一个对象) [连接符，追加在输出中，仅用于中间]
			//语法 foreach 对象名（是临时对象） stepto 数组对象 [name连接符，给循环对象生成name时用的连接符]
			size_t end1, end2;
			if (!_findBlockEnd(source, pos2, end, end1, end2, "foreach", "endforeach"))
			{
				return false;
			}
			if (st.size() < 4)
			{
				thelog << "语法错误 " << blockcode << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			string varname = st[1];
			string objname = st[3];
			CCTObject* p = _tryFindObject(objname, O, S);
			if (NULL != p && CCTObject::POINTER == p->m_type)
			{
				p = p->m_Pointer;
			}
			if (NULL == p)
			{
				//thelog << "对象不存在 " << objname << " 跳过此块" << " line : " << _getLineNumber(source.c_str(), pos) << endi;
			}
			else if (CCTObject::ARRAY != p->m_type)
			{
				//thelog << "对象不是数组" << objname << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				//thelog << endl << p->toString(str) << ende;
				//return false;
			}
			else if (0 == p->m_Array.size())
			{
				thelog << "数组没有成员 " << objname << " line : " << _getLineNumber(source.c_str(), pos) << ende;
			}
			else
			{
				CCTObject & P = *p;
				if ("in" == st[2] || "in1" == st[2])
				{
					string _;
					if (st.size() >= 5)_ = st[4];
					vector<CCTObject >::iterator it = P.m_Array.begin();
					if ("in1" == st[2] && it != P.m_Array.end())++it;//in1跳过第一个
					for (; it != P.m_Array.end(); ++it)
					{
						S.Push();
						DEBUG_LOG << "varname " << varname << endi;
						S.AddRef(varname, &*it);
						//处理块
						if (!_ProcessBlock(source, pos2, end1, ss, O, S))
						{
							//thelog << "处理出错" << ende;
							return false;
						}
						if (it + 1 != P.m_Array.end())ss << _;
						S.Pop();
					}
				}
				else if ("stepto" == st[2])
				{
					for (size_t stepend = 0; stepend < P.m_Array.size(); ++stepend)
					{
						CCTObject o_step;
						StringTokenizer st_step(objname, ".");
						string stepname = st_step[st_step.size() - 1];//构建为一个包含数组的对象，不能直接构建为一个数组对象是因为还需要拼一个名字出来
						string name;
						string _;
						if (st.size() >= 5)_ = st[4];
						for (size_t step = 0; step <= stepend; ++step)
						{
							o_step.SetObjectArrayPushBack(stepname, P.m_Array[step]);
							if (0 != step)name += _;
							name += P.m_Array[step].GetDefaultValue();
						}
						o_step.SetObjectAddProperty("name", name);

						S.Push();
						S.Add(varname, o_step);
						//处理块
						if (!_ProcessBlock(source, pos2, end1, ss, O, S))
						{
							thelog << "处理出错" << ende;
							return false;
						}
						S.Pop();
					}
				}
				else
				{
					thelog << "未知的 " << st[2] << " line : " << _getLineNumber(source.c_str(), pos) << ende;
					return false;
				}
			}
			//处理end之后的部分
			return _ProcessBlock(source, end2, end, ss, O, S);
		}
		else if ("for" == st[0])
		{
			//语法：for 变量名 from 起点 less 终点
			size_t end1, end2;
			if (!_findBlockEnd(source, pos2, end, end1, end2, "for", "endfor"))
			{
				return false;
			}
			if (st.size() < 4)
			{
				thelog << "语法错误 " << blockcode << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			string varname = st[1];
			string objname = st[5];
			string value;
			if (!_GetObjectValue(objname, O, S, false, value))
			{
				thelog << "对象值不存在" << objname << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			if ("less" == st[4])
			{
				for (int i = atol(st[3].c_str()); i < atol(value.c_str()); ++i)
				{
					S.Push();
					DEBUG_LOG << "varname " << varname << endi;
					CCTObject tmp;
					tmp.SetProperty(i);
					S.Add(varname, tmp);
					//处理块
					if (!_ProcessBlock(source, pos2, end1, ss, O, S))
					{
						thelog << "处理出错" << ende;
						return false;
					}
					S.Pop();
				}
			}
			else
			{
				thelog << "未知的 " << st[2] << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			//处理end之后的部分
			return _ProcessBlock(source, end2, end, ss, O, S);
		}
		else if ("if" == st[0])
		{
			//语法：if 对象 exist
			//语法：if 对象 equal 对象或文本
			//语法：if 对象 in 对象或文本 对象或文本 ......
			if (st.size() < 3)
			{
				thelog << "语法错误 " << blockcode << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			string objname = st[1];
			string iftype = st[2];
			CCTObject* p = _tryFindObject(objname, O, S);
			bool ifOK = false;//是否符合条件
			if ("exist" == iftype)
			{
				if (NULL == p)
				{
					DEBUG_LOG << "对象不存在 " << objname << " 跳过此块" << " line : " << _getLineNumber(source.c_str(), pos) << endi;
					ifOK = false;
				}
				else
				{
					ifOK = true;
				}
			}
			else if ("equal" == iftype)
			{
				if (NULL == p)
				{
					DEBUG_LOG << "对象不存在 " << objname << " 跳过此块" << " line : " << _getLineNumber(source.c_str(), pos) << endi;
					ifOK = false;
				}
				else
				{
					if (st.size() < 4)
					{
						thelog << "语法错误 " << blockcode << " line : " << _getLineNumber(source.c_str(), pos) << ende;
						return false;
					}
					//thelog << st[0] << " " << st[1] << " " << st[2] << " " << st[3] << endi;
					string refobj;
					if (!_GetObjectValue(st[3], O, S, false, refobj))
					{
						thelog << "对象不存在 " << st[3] << " line : " << _getLineNumber(source.c_str(), pos) << ende;
						return false;
					}
					if (p->GetDefaultValue() != refobj)
					{
						DEBUG_LOG << "不相等 " << objname << " [" << p->GetDefaultValue() << "] [" << refobj << "] 跳过此块"
							<< " line : " << _getLineNumber(source.c_str(), pos) << endi;
						ifOK = false;
					}
					else
					{
						ifOK = true;
					}
				}
			}
			else if ("in" == iftype)
			{
				if (NULL == p)
				{
					DEBUG_LOG << "对象不存在 " << objname << " 跳过此块" << " line : " << _getLineNumber(source.c_str(), pos) << endi;
					ifOK = false;
				}
				else
				{
					if (st.size() < 4)
					{
						thelog << "语法错误 " << blockcode << " line : " << _getLineNumber(source.c_str(), pos) << ende;
						return false;
					}
					ifOK = false;
					for (size_t i_in = 3; i_in < st.size(); ++i_in)
					{
						string refobj;
						if (!_GetObjectValue(st[i_in], O, S, false, refobj))
						{
							thelog << "对象不存在 " << st[i_in] << " line : " << _getLineNumber(source.c_str(), pos) << ende;
							return false;
						}
						if (p->GetDefaultValue() != refobj)
						{
							DEBUG_LOG << "不相等 " << objname << " [" << p->GetDefaultValue() << "] [" << refobj << "] 跳过此块"
								<< " line : " << _getLineNumber(source.c_str(), pos) << endi;
						}
						else
						{
							ifOK = true;
							break;
						}
					}
				}
			}
			else
			{
				thelog << "未知的 " << st[2] << " line : " << _getLineNumber(source.c_str(), pos) << ende;
				return false;
			}
			size_t end1, end2;
			S.Push();
			if (ifOK)
			{
				if (_TryFindBlockEnd(source, pos2, end, end1, end2, "if", "else"))
				{
					DEBUG_LOG << "找到else" << endi;
					if (!_ProcessBlock(source, pos2, end1, ss, O, S))return false;//处理else之前的
					if (!_findBlockEnd(source, end2, end, end1, end2, "if", "endif"))return false;//找到endif
				}
				else
				{
					DEBUG_LOG << "没有else" << endi;
					if (!_findBlockEnd(source, pos2, end, end1, end2, "if", "endif"))return false;
					if (!_ProcessBlock(source, pos2, end1, ss, O, S))return false;//处理endif之前的
				}
			}
			else
			{
				if (_TryFindBlockEnd(source, pos2, end, end1, end2, "if", "else"))
				{//找到else
					DEBUG_LOG << "找到else" << endi;
					size_t end3, end4;
					if (!_findBlockEnd(source, pos2, end, end3, end4, "if", "endif"))return false;
					if (!_ProcessBlock(source, end2, end3, ss, O, S))return false;//处理else之后的
					end1 = end3;
					end2 = end4;
				}
				else
				{//没有else
					DEBUG_LOG << "没有else" << endi;
					if (!_findBlockEnd(source, pos2, end, end1, end2, "if", "endif"))return false;
				}
			}
			S.Pop();
			return _ProcessBlock(source, end2, end, ss, O, S);//处理endif之后的
		}
		else if ("dim" == st[0])
		{
			//语法：dim 对象名=表达式 [ref]
			if (st.size() < 2)
			{
				thelog << "语法错误 " << blockcode << ende;
			}
			size_t dimpos = st[1].find("=");
			if (string::npos == dimpos)
			{
				thelog << "语法错误 " << st[1] << ende;
				return false;
			}
			string varname = st[1].substr(0, dimpos);
			string objname = st[1].substr(dimpos + 1);
			bool ref = (st.size() >= 3 && "ref" == st[2]);
			CCTObject* p{ nullptr };
			string value = _ProcessExpression(O, S, objname, p);
			if (p != nullptr)
			{
				if (ref)
				{
					S.AddRef(varname, p);
				}
				else
				{
					S.Add(varname, *p);
				}
			}
			else
			{
				if (!S.AddProperty(varname, value))
				{
					thelog << "AddProperty出错" << ende;
				}
			}
			//处理dim之后的部分
			return _ProcessBlock(source, pos2, end, ss, O, S);
		}
		else if ("set" == st[0])
		{
			//语法：set 对象 = 表达式 [ref]
			//语法：set 对象 += 表达式 （执行整数加）
			if (st.size() < 4)
			{
				thelog << "语法错误 " << blockcode << ende;
			}
			//thelog << st[0]<<" "<< st[1] << " " << st[2] << " " << st[3] << endi;
			string varname = st[1];
			string op = st[2];
			CCTObject* p{ nullptr };
			string value = _ProcessExpression(O, S, st[3], p);
			CCTObject* obj = _tryFindObject(varname, O, S);
			bool ref = (st.size() >= 5 && "ref" == st[4]);
			if (NULL == obj)
			{
				thelog << blockcode << " 对象不存在 " << varname << endi;
				return false;
			}
			else
			{
				if ("=" == op)
				{
					if (nullptr == p)
					{
						obj->SetProperty(value);
					}
					else
					{
						if (ref)
						{
							obj->SetPointer(p);
						}
						else
						{
							*obj = *p;
						}
					}
				}
				else if ("+=" == op)
				{
					if (nullptr == p)
					{
						obj->SetProperty(atol(obj->GetDefaultValue().c_str()) + atol(value.c_str()));
					}
					else
					{
						obj->SetProperty(atol(obj->GetDefaultValue().c_str()) + atol(p->GetDefaultValue().c_str()));
					}
				}
				else
				{
					thelog << "未知的 set 动作 " << op << ende;
					return false;
				}
			}
			//处理set之后的部分
			return _ProcessBlock(source, pos2, end, ss, O, S);
		}
		else if ("backspace" == st[0])
		{
			//语法：backspace 对象或文本
			if (st.size() < 2)
			{
				thelog << "语法错误 " << blockcode << ende;
			}
			string objname = st[1];
			if (objname.size() >= 2 && '\"' == objname[0] && '\"' == objname[objname.size() - 1])
			{
				objname = objname.substr(1, objname.size() - 2);
			}
			else
			{
				CCTObject* p = _tryFindObject(objname, O, S);
				if (NULL == p)
				{
					thelog << blockcode << " 对象不存在 " << objname << endi;
					return false;
				}
				else
				{
					objname = p->GetDefaultValue();
				}
			}
			string tmp = ss.str();
			size_t n = tmp.find_last_of(objname);
			//thelog << "消除 [" << objname << "] 找到位置 " << n << " 总长度 " << tmp.size() << endi;
			if (n != tmp.npos)
			{
				bool all_blank{ true };
				auto is_space = [](char c) -> bool
				{
					return ' ' == c || '\t' == c || '\r' == c || '\n' == c;
				};
				for (size_t x = n + objname.size(); x < tmp.size(); ++x)
				{
					if (!is_space(tmp.at(x)))
					{
						all_blank = false;
						break;
					}
				}
				if (all_blank)
				{
					ss.str("");
					ss << tmp.substr(0, n);
					//thelog << "成功消除 [" << objname << "] 新长度 " << ss.str().size() << endi;
				}
				else
				{
					//thelog << "不是尾巴 " << ss.str().size() << endi;
				}
			}
			//处理backspace之后的部分
			return _ProcessBlock(source, pos2, end, ss, O, S);
		}
		else if ("line_continuation" == st[0])
		{
			//语法：line_continuation 连接行并去掉了空白
			char const* _blanks = " \t\r\n";
			string tmp = ss.str();
			RTrim(tmp, _blanks);
			ss.str("");
			ss << tmp;
			for (; pos2 < end && isBlank(source[pos2], _blanks); ++pos2)
			{
			}
			//处理line_continuation之后的部分
			return _ProcessBlock(source, pos2, end, ss, O, S);
		 }
		else if ("error" == st[0])
		{
		thelog << "模板报错，位于行：" << _getLineNumber(source.c_str(), pos) << " 信息：" << endl << blockcode << ende;
		return false;
		}
		else
		{
		thelog << "未知的 " << st[0] << " line : " << _getLineNumber(source.c_str(), pos) << ende;
		return false;
		}
	}

public:
	//处理模板文件，输出到ss中
	bool ProcessTemplate(char const* templatefile, stringstream& ss, CCTObject& O, CCTStack& S)
	{
		m_functions.clear();

		CEasyFile file;
		string filedata;
		if (!file.ReadFile(templatefile, filedata))
		{
			thelog << "未能打开模板文件 " << templatefile << ende;
			return false;
		}
		long end = filedata.size();
		bool ret = _ProcessBlock(filedata, 0, end, ss, O, S);
		if (!ret)
		{
			thelog << "操作失败 " << templatefile << " 输出文件在出错处中止 " << ende;
		}

		//thelog << endl << ss.str() << endi;
		return ret;
	}
	bool ProcessFile(char const* infile, char const* outfile, CCTObject& O, CCTStack& S)
	{
		thelog << "开始处理 " << infile << " 输出到 " << outfile << endi;
		stringstream ss;
		bool ret = ProcessTemplate(infile, ss, O, S);
		//thelog << endl << ss.str() << endi;

		CEasyFile file;
		if (!file.WriteFile(outfile, ss.str().c_str()))
		{
			thelog << "写文件出错 " << outfile << ende;
			return false;
		}

		return ret;
	}
};
