//CTScript2.h 脚本解释器
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

//脚本解释器 以简单C++语法为基础 做了少量扩展
//语法说明见下面的LanguageHelp函数
//内置函数说明运行时用PluginHelp函数获得

#include "ICTScript2.h"
#include "CTScriptFun2.h"

namespace ns_my_script2
{
	class CCTSException
	{
	public:
		static void Throw(char const* file, long line, string const& source, size_t pos, char const* _msg)
		{
			return Throw(file, line, source.c_str(), pos, _msg);
		}
		static void Throw(char const* file, long line, char const* source, size_t pos, char const* _msg)
		{
			string msg;
			char buf[256];
			msg += "来自文件";
			msg += file;
			sprintf(buf, "的行%ld的出错信息：", line);
			msg += buf;
			msg += "出错脚本：\r\n";
			msg += source;
			sprintf(buf, "\r\n出错位置%ld\r\n", (long)pos);
			msg += buf;
			string str;
			str = source;
			str.replace(0, pos, pos, '.');
			msg += str + "\r\n";
			msg += "错误信息：\r\n";
			msg += _msg;
			msg += "\r\n";
			msg += "----------------------------\r\n";
			throw msg;
		}
	};

	////////////////////////////////////////////////////////////////////////////////
	//CPluginMap
	template<typename T>
	void CPluginMap::addplugin(map<string, CPlugin*>& mapPlugins)
	{
		CPlugin* p;
		p = new T;
		if (NULL == p)throw "内存不足";
		mapPlugins[p->plugin_name] = p;
	}
	map<string, CPlugin*>& CPluginMap::GetPluginMap()
	{
		STATIC_G map<string, CPlugin*> mapPlugins;
		if (0 == mapPlugins.size())
		{
			addplugin<CNullFun >(mapPlugins);
			addplugin<CMax >(mapPlugins);
			addplugin<CMin >(mapPlugins);
			addplugin<CFloor >(mapPlugins);
			addplugin<CCeil >(mapPlugins);
			addplugin<CRound >(mapPlugins);
			addplugin<CAbs >(mapPlugins);
			addplugin<CStrlen >(mapPlugins);
			addplugin<CStrcmp >(mapPlugins);
			addplugin<CStricmp >(mapPlugins);
			addplugin<CStrncmp >(mapPlugins);
			addplugin<CStrnicmp >(mapPlugins);
			addplugin<CStrcat >(mapPlugins);
			addplugin<CStrstr >(mapPlugins);
			addplugin<CSubstr >(mapPlugins);
			addplugin<CToLong >(mapPlugins);
			addplugin<CToDouble >(mapPlugins);
			addplugin<CToString >(mapPlugins);
		}
		return mapPlugins;
	}
	bool CPluginMap::AddPlugin(char const* name, Variable::types type, CPlugin* p)
	{
		map<string, CPlugin*>& mapPlugins = GetPluginMap();
		if (mapPlugins.find(name) != mapPlugins.end())return false;
		else
		{
			p->plugin_name = name;
			p->plugin_return_type = type;
			mapPlugins[name] = p;
			return true;
		}
	}
	CPlugin* CPluginMap::GetPlugin(string const& fun_name)
	{
		map<string, CPlugin*>& funMap = CPluginMap::GetPluginMap();
		map<string, CPlugin*>::iterator it = funMap.find(fun_name);
		if (it != funMap.end())return it->second;
		else return NULL;
	}
	CPlugin* CPluginMap::GetPlugin(HANDLE const& h)
	{
		return GetPlugin(h.plugin_name);
	}
	string& CPluginMap::PluginHelp(string& ret)
	{
		ret = "";
		map<string, CPlugin*>& funMap = GetPluginMap();
		string str;

		for (map<string, CPlugin*>::iterator it = funMap.begin(); it != funMap.end(); ++it)
		{
			ret += it->second->help(str);
			ret += "\r\n";
		}

		return ret;
	}
	/////////////////////////////////////////////////////////////////////////////////////
	class CCTScript
	{
	private:
		//变量表
		struct T_VARIABLE_BLOCK : public vector<pair<string, Variable > >
		{
			bool AddVariable(string const& var_name, Variable const& var)
			{
				for (const_iterator it = begin(); it != end(); ++it)
				{
					if (it->first == var_name)
					{
						return false;
					}
				}

				push_back(pair<string, Variable >(var_name, var));
				return true;
			}
			Variable* FindVariable(string const& var_name)const
			{
				return FindVariable(var_name.c_str());
			}
			Variable* FindVariable(char const* var_name)const
			{
				for (const_reverse_iterator it = rbegin(); it != rend(); ++it)
				{
					if (it->first != var_name)
					{
						continue;
					}
					return (Variable*)&it->second;
				}
				return NULL;
			}
		};
		class T_VARIABLE_S
		{
		private:
			typedef vector<T_VARIABLE_BLOCK > T_DATA;

			T_VARIABLE_BLOCK* m_pEnvs;//指向环境变量
			T_VARIABLE_BLOCK* m_pGlobals;//指向全局变量
			long m_Globals_count;//全局变量个数
			T_VARIABLE_BLOCK* m_pParams;//指向参数
			T_DATA m_local_variables;//局部变量
		public:
			T_VARIABLE_S() :m_pEnvs(NULL), m_pGlobals(NULL), m_Globals_count(-1), m_pParams(NULL) {}
			long getGlobalCount()const { return m_pGlobals->size(); }
			void T_VARIABLE_S_init(T_VARIABLE_BLOCK* pEnv, T_VARIABLE_BLOCK* pG, long Globals_count, T_VARIABLE_BLOCK* pP)
			{
				m_pEnvs = pEnv;
				m_pGlobals = pG;
				m_Globals_count = Globals_count;
				m_pParams = pP;
				m_local_variables.clear();
			}
			void T_VARIABLE_S_init(T_VARIABLE_BLOCK const* pEnv, T_VARIABLE_BLOCK const* pG, long Globals_count, T_VARIABLE_BLOCK const* pP)
			{
				m_pEnvs = (T_VARIABLE_BLOCK*)pEnv;
				m_pGlobals = (T_VARIABLE_BLOCK*)pG;
				m_Globals_count = Globals_count;
				m_pParams = (T_VARIABLE_BLOCK*)pP;
				m_local_variables.clear();
			}
			void FromParentVars(T_VARIABLE_S& vars, long Globals_count, T_VARIABLE_BLOCK const* pP)
			{
				T_VARIABLE_S_init(vars.m_pEnvs, vars.m_pGlobals, Globals_count, pP);
			}
			void FromParentVars(T_VARIABLE_S const& vars, long Globals_count, T_VARIABLE_BLOCK const* pP)
			{
				T_VARIABLE_S_init(vars.m_pEnvs, vars.m_pGlobals, Globals_count, pP);
			}
			void PushLevel()
			{
				m_local_variables.resize(m_local_variables.size() + 1);
				//thelog << "PushLevel " << m_datas.size() << endi;
			}
			void PopLevel()
			{
				if (m_local_variables.size() > 0)m_local_variables.resize(m_local_variables.size() - 1);
				//thelog << "PopLevel " << m_datas.size() << endi;
			}
			bool AddVariable(string const& var_name, Variable const& var)
			{
				if (NULL != FindVariable(var_name))return false;
				if (0 == m_local_variables.size())
				{//全局变量
					m_pGlobals->push_back(pair<string, Variable >(var_name, var));
				}
				else
				{//局部变量
					m_local_variables.rbegin()->push_back(pair<string, Variable >(var_name, var));
				}
				return true;
			}
			Variable* FindVariable(string const& var_name)const
			{
				return FindVariable(var_name.c_str());
			}
			Variable* FindVariable(char const* var_name)const
			{
				long count;

				//从顶层向底层查找
				for (T_DATA::const_reverse_iterator it = m_local_variables.rbegin(); it != m_local_variables.rend(); ++it)
				{
					for (T_VARIABLE_BLOCK::const_iterator it_block = it->begin(); it_block != it->end(); ++it_block)
					{
						if (it_block->first != var_name)
						{
							continue;
						}
						return (Variable*)&it_block->second;
					}
				}

				//再找参数
				count = m_pParams->size();
				for (--count; count >= 0; --count)
				{
					if ((*m_pParams)[count].first != var_name)
					{
						continue;
					}
					return (Variable*) & (*m_pParams)[count].second;
				}

				//再找全局变量
				count = (m_Globals_count < 0 ? m_pGlobals->size() : m_Globals_count);
				if (m_Globals_count >= 0 && static_cast<size_t>(m_Globals_count) > m_pGlobals->size())
				{
					thelog << "错误的有效全局变量个数 " << m_Globals_count << " 最大 " << m_pGlobals->size() << ende;
					return NULL;
				}
				for (--count; count >= 0; --count)
				{
					if ((*m_pGlobals)[count].first != var_name)
					{
						continue;
					}
					return (Variable*) & (*m_pGlobals)[count].second;
				}

				//最后找环境变量
				for (T_VARIABLE_BLOCK::const_reverse_iterator it = m_pEnvs->rbegin(); it != m_pEnvs->rend(); ++it)
				{
					if (it->first != var_name)
					{
						continue;
					}
					return (Variable*)&it->second;
				}

				return NULL;
			}
			string ToString(long level = 0)const
			{
				stringstream ret;
				ret << this << endl;
				long n = 0;
				ret << "环境变量：" << endl;
				for (T_VARIABLE_BLOCK::const_iterator it = m_pEnvs->begin(); it != m_pEnvs->end(); ++it)
				{
					ret << n << " : " << it->first << " : " << it->second.ToString() << endl;
					++n;
				}
				ret << "全局变量：有效数" << this->m_Globals_count << endl;
				for (T_VARIABLE_BLOCK::const_iterator it = m_pGlobals->begin(); it != m_pGlobals->end(); ++it)
				{
					ret << n << " : " << it->first << " : " << it->second.ToString() << endl;
					++n;
				}
				ret << "参数：" << endl;
				for (T_VARIABLE_BLOCK::const_iterator it = m_pParams->begin(); it != m_pParams->end(); ++it)
				{
					ret << n << " : " << it->first << " : " << it->second.ToString() << endl;
					++n;
				}
				ret << "局部变量：" << endl;
				for (T_DATA::const_iterator it = m_local_variables.begin(); it != m_local_variables.end(); ++it)
				{
					for (T_VARIABLE_BLOCK::const_iterator it_block = it->begin(); it_block != it->end(); ++it_block)
					{
						//if(it_block->second->isconst)continue;
						ret << n << " : " << it_block->first << " : " << it_block->second.ToString() << endl;
					}
					++n;
				}
				return ret.str();
			}
		};
		class CCTTokens
		{
		public:
			CMyVector<Token > m_tokens;//解析出的语法元素
			//返回开始位置，如果超出有效范围，则返回0或越过最后一个
			size_t TokenStart(size_t pos)const
			{
				if (0 == m_tokens.size())return 0;
				if (pos < 0)return 0;
				if (pos < m_tokens.size())return m_tokens[pos].pos;
				else
				{
					return m_tokens[m_tokens.size() - 1].pos + m_tokens[m_tokens.size() - 1].text.size();
				}
			}
			bool IsDelimiter(size_t pos, char const* delimiter)const
			{
				return pos < m_tokens.size() && m_tokens[pos].type == Token::DELIMITER && m_tokens[pos].text == delimiter;
			}
			bool IsOperator(size_t pos, char const* op)const
			{
				return pos < m_tokens.size() && m_tokens[pos].type == Token::OPERATOR && m_tokens[pos].text == op;
			}
			bool GetOperatorLevel(char const* op, long& ret_level)const
			{
				ret_level = -1;
				if (0 == strcmp(op, "["))ret_level = 1;
				if (0 == strcmp(op, "."))ret_level = 1;
				if (0 == strcmp(op, "!"))ret_level = 2;
				if (0 == strcmp(op, "++"))ret_level = 2;
				if (0 == strcmp(op, "--"))ret_level = 2;
				if (0 == strcmp(op, "*"))ret_level = 3;
				if (0 == strcmp(op, "/"))ret_level = 3;
				if (0 == strcmp(op, "%"))ret_level = 3;
				if (0 == strcmp(op, "+"))ret_level = 4;
				if (0 == strcmp(op, "-"))ret_level = 4;
				if (0 == strcmp(op, "<"))ret_level = 6;
				if (0 == strcmp(op, "<="))ret_level = 6;
				if (0 == strcmp(op, ">"))ret_level = 6;
				if (0 == strcmp(op, ">="))ret_level = 6;
				if (0 == strcmp(op, "=="))ret_level = 7;
				if (0 == strcmp(op, "!="))ret_level = 7;
				if (0 == strcmp(op, "&&"))ret_level = 11;
				if (0 == strcmp(op, "||"))ret_level = 12;
				if (0 == strcmp(op, "="))ret_level = 14;
				if (0 == strcmp(op, "+="))ret_level = 14;
				if (0 == strcmp(op, "-="))ret_level = 14;
				if (0 == strcmp(op, "*="))ret_level = 14;
				if (0 == strcmp(op, "/="))ret_level = 14;
				if (0 == strcmp(op, "%="))ret_level = 14;
				if (0 == strcmp(op, ","))ret_level = 15;
				return ret_level != -1;
			}
			bool IsOperatorLeftFirst(long level)const
			{
				return level != 2 && level != 13 && level != 14;
			}
			bool IsCharIn(char c, char const* charset)const
			{
				long i = 0;
				while (charset[i] != '\0')
				{
					if (charset[i] == c)return true;
					++i;
				}
				return false;
			}
			//headset最后一个必须是空串
			bool IsStartWith(char const* str, char const (*headset)[TOKEN_BUF_LEN], string& ret)const
			{
				long i = 0;
				ret = "";
				while (headset[i][0] != '\0')
				{
					size_t keylen = strlen(headset[i]);
					if (0 == strncmp(headset[i], str, keylen))
					{
						if (ret.size() < strlen(headset[i]))ret = headset[i];
					}
					++i;
				}
				return ret.size() != 0;
			}
			bool IsBlank(char c)const
			{
				return IsCharIn(c, " \t\r\n");
			}
			bool IsLineEnd(char c)const
			{
				return '\0' == c || IsCharIn(c, "\r\n");
			}
			bool IsDelimiter(char c)const
			{
				return IsCharIn(c, "{};");
			}
			bool IsKeyword(char const* str, string& key)const
			{
				STATIC_C char const buf[][TOKEN_BUF_LEN] = {
					"asm","default","float","operator","static_cast","union",
					"auto","delete","for","private","struct","unsigned",
					"bool","do","friend","protected","switch","using",
					"break","double","goto","public","template","virtual",
					"case","dynamic_cast","if","register","this","void",
					"catch","else","inline","reinterpret_cast","throw","volatile",
					"char","enum","int","return","true","wchar_t",
					"class","explicit","long","short","try","while",
					"const","export","mutable","signed","typedef",
					"const_cast","extern","namespace","sizeof","typeid",
					"continue","false","new","static","typename","string",
					""
				};//必须以空串结尾
				return IsStartWith(str, buf, key);
			}
			bool IsOperator(char const* str, string& key)const
			{
				STATIC_C char const buf[][TOKEN_BUF_LEN] = {
					">>=","<<=","->","++","--","<<",">>",
					"<=",">=","==","!=","&&","||",
					"+=","-=","*=","/=","%=","&=",
					"^=","|=","!","~","+","-","*","/","%","&",">","<","=","|","^","?","::",":",".",
					"(",")","[","]",",",
					""
				};//必须以空串结尾
				return IsStartWith(str, buf, key);
			}
			bool TryGetDelimiter(char const* source, string::size_type& pos, string& ret)
			{
				if (IsDelimiter(source[pos]))
				{
					ret = source[pos];
					++pos;
					return true;
				}
				return false;
			}
			bool TryGetKeyword(char const* source, string::size_type& pos, string& ret)
			{
				string key;
				string nextkey;
				size_t keylen;
				if (IsKeyword(source + pos, key))
				{
					keylen = key.size();
					if ('\0' == source[pos + keylen] || IsBlank(source[pos + keylen]) || IsDelimiter(source[pos + keylen]) || IsOperator(source + pos + keylen, nextkey))
					{
						ret = key;
						pos += keylen;
						return true;
					}
				}
				return false;
			}
			bool TryGetOperator(char const* source, string::size_type& pos, string& ret)
			{
				string key;
				size_t keylen;
				if (IsOperator(source + pos, key))
				{
					keylen = key.size();
					ret = key;
					pos += keylen;
					return true;
				}
				return false;
			}
			bool NumberToVariable(char const* source, Variable& var)
			{
				char* endptr;
				if (IsCharIn('.', source) || IsCharIn('e', source) || IsCharIn('E', source))
				{
					var.type = Variable::DOUBLE;
					var.dValue = strtod(source, &endptr);
				}
				else
				{
					var.type = Variable::LONG;
					long prefix = 0;
					long radix = 10;
					if (strlen(source) >= 1 && '0' == source[0])
					{
						if (strlen(source) >= 2 && ('x' == source[1] || 'X' == source[1]))
						{
							radix = 16;
							prefix = 2;
						}
						else
						{
							radix = 8;
							prefix = 1;
						}
					}
					var.lValue = strtol(source + prefix, &endptr, radix);
				}
				if (strlen(endptr) != 0)
				{
					if ((Variable::DOUBLE == var.type && (0 == stricmp(endptr, "f") || 0 == stricmp(endptr, "l")))
						|| (Variable::LONG == var.type && (0 == stricmp(endptr, "u") || 0 == stricmp(endptr, "l") || 0 == stricmp(endptr, "i64"))))
					{
						return true;
					}
					string str;
					str = "数值常量格式错误 ";
					str += endptr;
					CCTSException::Throw(__FILE__, __LINE__, source, endptr - source, str.c_str());
					return false;
				}
				return true;
			}
			//以数字或点开头的串
			bool TryGetNumber(char const* source, string::size_type& pos, string& ret)
			{
				ret = "";
				char c = source[pos];
				if ((c >= '0' && c <= '9') || (c == '.' && source[pos + 1] >= '0' && source[pos + 1] <= '9'))
				{
					while ((c = source[pos]) != '\0')
					{
						if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || '.' == c || '_' == c)
						{
						}
						else
						{
							break;
						}
						ret += c;
						++pos;
					}
				}
				return ret.size() != 0;
			}
			string TokenToString(char const* source)
			{
				string ret;
				size_t pos;
				char c;

				pos = 0;
				while ((c = source[pos]) != '\0')
				{
					//处理转义序列
					if ('\\' == c)
					{
						++pos;
						c = source[pos];
						if ('\0' == c)CCTSException::Throw(__FILE__, __LINE__, source, pos, "未期待的字符串结束，字符转义序列不完整");
						if ('x' == c)
						{//16进制
							++pos;
							c = source[pos];
							if ('\0' == c)CCTSException::Throw(__FILE__, __LINE__, source, pos, "未期待的字符串结束，字符转义序列不完整，至少需要一个十六进制数字");
							if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))CCTSException::Throw(__FILE__, __LINE__, source, pos, "字符转义序列不完整，至少需要一个十六进制数字");
							char tmp = 0;
							do
							{
								if (c >= '0' && c <= '9')tmp = tmp * 16 + (c - '0');
								else if (c >= 'a' && c <= 'f')tmp = tmp * 16 + (c - 'a' + 10);
								else if (c >= 'A' && c <= 'F')tmp = tmp * 16 + (c - 'A' + 10);
								else CCTSException::Throw(__FILE__, __LINE__, source, pos, "程序内部错误");
								++pos;
								c = source[pos];
							} while ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
							c = tmp;
							--pos;
						}
						else if (c >= '0' && c <= '9')
						{//8进制
							char tmp = 0;
							long count = 0;
							do
							{
								tmp = tmp * 8 + (c - '0');
								++pos;
								c = source[pos];
							} while (c >= '0' && c <= '9' && (++count) < 3);
							c = tmp;
							--pos;
						}
						else
						{
							switch (c)
							{
							case 'n':c = 0x0a; break;
							case 't':c = 0x09; break;
							case 'v':c = 0x0b; break;
							case 'b':c = 0x08; break;
							case 'r':c = 0x0d; break;
							case 'f':c = 0x0c; break;
							case 'a':c = 0x07; break;
							default:break;
							}
						}
					}

					ret += c;
					++pos;
				}
				return ret;
			}
			//以单双引号开始的串
			bool TryGetString(char const* source, string::size_type& pos, string& ret)
			{
				ret = "";
				char c = source[pos];
				char head;
				if (c == '\'' || c == '\"')
				{
					bool isSpe = false;
					head = c;
					++pos;
					c = source[pos];
					while (!IsLineEnd(c) && (isSpe || c != head))
					{
						//处理转义序列
						if (!isSpe)
						{
							if ('\\' == c)
							{
								isSpe = true;
							}
						}
						else
						{
							isSpe = false;
						}

						ret += c;
						++pos;
						c = source[pos];
					}
					if (isSpe)CCTSException::Throw(__FILE__, __LINE__, source, pos, "不完整的转义序列");
					if (source[pos] == '\0')CCTSException::Throw(__FILE__, __LINE__, source, pos, "未期待的脚本或行结束，不匹配的字符串单引号或双引号");
					++pos;
					return true;
				}
				return false;
			}
			//以字母或下划线开头的子母数字下划线串
			bool TryGetIdentifier(char const* source, string::size_type& pos, string& ret)
			{
				ret = "";
				char c;
				while ((c = source[pos]) != '\0' && (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
				{
					if (ret.size() == 0 && c >= '0' && c <= '9')break;
					ret += c;
					++pos;
				}
				return ret.size() != 0;
			}
			bool GetToken(string& source, string::size_type& pos)
			{
				Token::types type;
				string token;
				char c;
				bool isInComment = false;
				while (pos < source.size())
				{
					c = source[pos];
					if (isInComment)
					{
						if ('\n' == c)
						{
							isInComment = false;
						}
						++pos;
						continue;
					}
					if ('/' == c && pos + 1 < source.size() && '/' == source[pos + 1])
					{
						isInComment = true;
						pos += 2;
						continue;
					}
					if (!IsBlank(c))break;
					++pos;
				}
				if (source.size() == pos)return false;
				if (TryGetKeyword(source.c_str(), pos, token))
				{
					type = Token::KEYWORD;
				}
				else if (TryGetNumber(source.c_str(), pos, token))
				{
					type = Token::NUMBER;
				}
				else if (TryGetString(source.c_str(), pos, token))
				{
					type = Token::STRING;
				}
				else if (TryGetDelimiter(source.c_str(), pos, token))
				{
					type = Token::DELIMITER;
				}
				else if (TryGetOperator(source.c_str(), pos, token))
				{
					type = Token::OPERATOR;
				}
				else if (TryGetIdentifier(source.c_str(), pos, token))
				{
					type = Token::IDENTIFIER;
				}
				else
				{
					CCTSException::Throw(__FILE__, __LINE__, source.c_str(), pos, "无法识别的符号");
					return false;
				}
				m_tokens.push_back(Token(type, token.c_str(), pos - token.size()));
				return true;
			}
			bool ToTokens(string& source)
			{
				string::size_type pos = 0;
				while (GetToken(source, pos));
				return true;
			}
			bool MoveCurrentToken(Token*& pToken, size_t pos)
			{
				if (IsPosNotToken(pos))pToken = NULL;
				else pToken = &m_tokens[pos];
				return NULL != pToken;
			}
			bool MoveNextToken(Token*& pToken, size_t& pos)
			{
				++pos;
				if (IsPosNotToken(pos))pToken = NULL;
				else pToken = &m_tokens[pos];
				return true;
			}
			bool IsPosNotToken(size_t pos)
			{
				return pos >= m_tokens.size();
			}
		};
	public:
		//表达式，单目算符有两个操作数则为后缀(第一个不使用)，函数参数个数不限
		//VARIABLE:pVariable 包含常量
		//PLUGIN:pPlugin(operands)
		//OPERATION:op operands
		struct Expression
		{
			enum types { NULLEXPRESSION, CONSTANT, DEFINE, VARIABLE, FUNCTION, PLUGIN, OPERATION };
			types type;
			Variable::types result_type;//编译用的结果类型，执行时实际上并不需要这个
			size_t source_start;//起始位置
			size_t source_end;//结束位置

			//变量
			string VariableName;
			Variable m_variable;//用于常量或运行时创建变量

			//函数
			CCTScript * pFunction;//函数索引

			//插件
			CPluginMap::HANDLE pPlugin;

			//插件或函数的参数
			CMyVector<Expression > ParamList;

			//操作
			string op;
			vector<Expression > Operands;//操作数

			//函数和属性
			void* user_p;//用户自行使用的指针，编译时可设置，执行时不可修改（指向的内容由用户决定）

			Expression() :type(NULLEXPRESSION), result_type(Variable::NULLVARIABLE), source_start(0), source_end(0), pFunction(NULL), user_p(NULL) {}
			Expression* pLeftOperand() { return (Operands.size() >= 1 && Operands[0].type != NULLEXPRESSION ? &Operands[0] : NULL); }//左操作数
			Expression const* pLeftOperand()const { return (Operands.size() >= 1 && Operands[0].type != NULLEXPRESSION ? &Operands[0] : NULL); }//左操作数
			Expression* pRightOperand() { return (Operands.size() >= 2 && Operands[1].type != NULLEXPRESSION ? &Operands[1] : NULL); }//右操作数
			Expression const* pRightOperand()const { return (Operands.size() >= 2 && Operands[1].type != NULLEXPRESSION ? &Operands[1] : NULL); }//右操作数
			bool AddLeftOperand(Expression const& exp)
			{
				if (NULL != pLeftOperand())return false;
				if (Operands.size() < 1)Operands.resize(1);
				Operands[0] = exp;
				return true;
			}
			bool AddRightOperand(Expression const& exp)
			{
				if (NULL != pRightOperand())return false;
				if (Operands.size() < 2)Operands.resize(2);
				Operands[1] = exp;
				return true;
			}

			Variable::types GetResultType(string& source, T_VARIABLE_S& vars)
			{
				Variable::types _resulttype = Variable::NULLVARIABLE;
				bool eva;
				switch (type)
				{
				case NULLEXPRESSION:
					_resulttype = Variable::NULLVARIABLE;
					break;
				case DEFINE:
					_resulttype = m_variable.type;
					break;
				case CONSTANT:
					_resulttype = m_variable.type;
					break;
				case VARIABLE:
					if (0 == VariableName.size())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的变量");
					if (NULL == vars.FindVariable(VariableName))
					{
						thelog << VariableName << endl << vars.ToString() << endi;
						CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的变量");
					}
					_resulttype = vars.FindVariable(VariableName)->type;
					break;
				case FUNCTION:
					if (NULL== pFunction)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的函数");
					_resulttype = pFunction->return_type;
					break;
				case PLUGIN:
					if (pPlugin.isNULL())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的插件");
					_resulttype = CPluginMap::GetPlugin(pPlugin)->plugin_return_type;
					break;
				case OPERATION:
					if (NULL == pLeftOperand() && NULL != pRightOperand())_resulttype = pRightOperand()->GetResultType(source, vars);
					else if (NULL != pLeftOperand() && NULL == pRightOperand())_resulttype = pLeftOperand()->GetResultType(source, vars);
					else if (NULL != pLeftOperand() && NULL != pRightOperand())
					{
						if ("=" == op || "+=" == op || "-=" == op || "*=" == op || "/=" == op || "%=" == op)eva = true;
						else eva = false;
						_resulttype = Variable::typeUpgrade(pLeftOperand()->GetResultType(source, vars), pRightOperand()->GetResultType(source, vars), eva);
						if (">" == op || "<" == op || ">=" == op || "<=" == op || "==" == op || "!=" == op || "||" == op || "&&" == op)
						{
							if (Variable::NULLVARIABLE != _resulttype)_resulttype = Variable::LONG;
						}
						if ("," == op)_resulttype = pRightOperand()->GetResultType(source, vars);
					}
					else CCTSException::Throw(__FILE__, __LINE__, source, source_start, "左右操作数不能都没有");
					break;
				default:
					CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未知的表达式类型");
					break;
				}
				return _resulttype;
			}
			//setpp记录已经做过的++ --
			bool CheckExpression(string& source, T_VARIABLE_S& vars, long level, set<string >& setpp)
			{
				size_t i;
				string msg;
				Variable var;
				CMyVector<Variable > _paramvars;//函数调用的参数

				if (DEFINE == type)
				{
					if (!vars.AddVariable(VariableName, m_variable))CCTSException::Throw(__FILE__, __LINE__, source, source_start, "添加变量失败");
					//thelog <<"添加变量 "<< VariableName << endl << vars.ToString() << endi;
				}

				result_type = GetResultType(source, vars);
				switch (type)
				{
				case NULLEXPRESSION:
					break;
				case DEFINE:
					if (0 == VariableName.size())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的变量");
					if (Variable::NULLVARIABLE == m_variable.type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "变量类型不能为NULLVARIABLE");
					if (Operands.size() > 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "变量定义最多有一个操作数");
					break;
				case CONSTANT:
					if (0 != VariableName.size())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "常量不应该有名字");
					if (Variable::NULLVARIABLE == m_variable.type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "常量类型不能为NULLVARIABLE");
					break;
				case VARIABLE:
					if (0 == VariableName.size())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的变量");
					break;
				case FUNCTION:
					if (NULL==pFunction)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的函数");
					if (!pFunction->IsCompiled())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "子函数未能成功编译");
					if (pFunction->m_EnvVariables.size() != ParamList.size())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "函数参数个数错误");
					for (i = 0; i < ParamList.size(); ++i)
					{
						ParamList[i].CheckExpression(source, vars, 0, setpp);
					}
					break;
				case PLUGIN:
					if (pPlugin.isNULL())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "未声明的插件");
					_paramvars.reserve(ParamList.size());
					for (i = 0; i < ParamList.size(); ++i)
					{
						ParamList[i].CheckExpression(source, vars, 0, setpp);
						if (Expression::VARIABLE == ParamList[i].type && vars.FindVariable(ParamList[i].VariableName)->isconst)
						{
							_paramvars.push_back(*vars.FindVariable(ParamList[i].VariableName));
						}
						else
						{
							var.type = ParamList[i].result_type;
							_paramvars.push_back(var);
						}
					}
					if (!CPluginMap::GetPlugin(pPlugin)->CheckPlugin(_paramvars, user_p, msg))CCTSException::Throw(__FILE__, __LINE__, source, source_start, msg.c_str());
					break;
				case OPERATION:
					//先检查左右表达式
					if (NULL != pLeftOperand())
					{
						pLeftOperand()->CheckExpression(source, vars, level + 1, setpp);
					}
					if (NULL != pRightOperand())
					{
						pRightOperand()->CheckExpression(source, vars, level + 1, setpp);
					}
					//检查操作是否合理
					if (NULL == pRightOperand())
					{//后缀
						if ("++" == op || "--" == op)
						{
							if (result_type != Variable::LONG)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "后缀++ --操作只能用于整数");
							if (Expression::VARIABLE != pLeftOperand()->type || vars.FindVariable(pLeftOperand()->VariableName)->isconst)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "后缀++ --操作只能用于变量");
							if (0 != level)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "后缀++ --操作只能作为独立语句使用，如'i++;'或'for(...;...;i++)'，不能用于赋值或组合运算");
						}
						else
						{
							CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不支持的后缀操作");
						}
					}
					else if (NULL == pLeftOperand())
					{//前缀
						if ("-" == op || "+" == op)
						{
							if (result_type != Variable::DOUBLE && result_type != Variable::LONG)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "前缀+-操作只能用于数值");
						}
						else if ("!" == op)
						{
							if (result_type != Variable::LONG)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "前缀!操作只能用于整数");
						}
						else if ("++" == op || "--" == op)
						{
							if (result_type != Variable::LONG)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "前缀++ --操作只能用于整数");
							if (Expression::VARIABLE != pRightOperand()->type || vars.FindVariable(pRightOperand()->VariableName)->isconst)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "前缀++ --操作只能用于变量");
							if (setpp.find(pRightOperand()->VariableName) != setpp.end())
								CCTSException::Throw(__FILE__, __LINE__, source, source_start, "前缀++ --操作每个语句对每个变量仅限使用一次（不区分++ --）");
							setpp.insert(pRightOperand()->VariableName);
						}
						else
						{
							CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不支持的前缀操作");
						}
					}
					else
					{//二元
						if ("+" == op || ">" == op || "<" == op || ">=" == op || "<=" == op || "==" == op || "!=" == op || "," == op)
						{
						}
						else if ("-" == op || "*" == op || "/" == op)
						{
							if (Variable::STRING == result_type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不能对字符串做-*/");
						}
						else if ("-=" == op || "*=" == op || "/=" == op)
						{
							if (Variable::STRING == result_type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不能对字符串做-*/");
							if (pLeftOperand()->type != Expression::VARIABLE || vars.FindVariable(pLeftOperand()->VariableName)->isconst)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不能对常量赋值");
						}
						else if ("&&" == op || "||" == op)
						{
							if (Variable::STRING == result_type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不能对字符串做&& ||");
						}
						else if ("%" == op)
						{
							if (Variable::LONG != result_type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "只能对整数做%");
						}
						else if ("%=" == op)
						{
							if (Variable::LONG != result_type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "只能对整数做%");
							if (pLeftOperand()->type != Expression::VARIABLE || vars.FindVariable(pLeftOperand()->VariableName)->isconst)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不能对常量赋值");
						}
						else if ("=" == op || "+=" == op)
						{
							if (pLeftOperand()->type != Expression::VARIABLE || vars.FindVariable(pLeftOperand()->VariableName)->isconst)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "不能对常量赋值");
						}
						else
						{
							CCTSException::Throw(__FILE__, __LINE__, source, source_start, (op + " 不支持的操作").c_str());
						}

						if ("," != op)
						{
							if (Variable::NULLVARIABLE == result_type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "操作符两边类型不匹配，不允许数值和字符串的自动转换");
						}
					}
					break;
				}
				return true;
			}
			bool ExecExpression(CCTScript const& script, T_VARIABLE_S& vars, Variable& ret, void* pe)const
			{
				size_t i;
				string msg;
				CMyVector<Variable > _paramvars;//插件调用的参数
				Variable ret_left;
				Variable ret_right;
				ret.type = result_type;
				switch (type)
				{
				case NULLEXPRESSION:
					ret.type = Variable::NULLVARIABLE;
					break;
				case DEFINE:
					ret.type = Variable::NULLVARIABLE;
					vars.AddVariable(VariableName, m_variable);
					if (NULL != pLeftOperand())
					{
						pLeftOperand()->ExecExpression(script, vars, ret_left, pe);
						if (ret_left.isNull())script.Throw(__FILE__, __LINE__, "左操作数计算出错");
						ret = *(vars.FindVariable(VariableName)) = ret_left;
					}
					break;
				case CONSTANT:
					ret = m_variable;
					break;
				case VARIABLE:
					ret = *vars.FindVariable(VariableName);
					break;
				case FUNCTION:
					{
						T_VARIABLE_BLOCK tmpparams;
						tmpparams = pFunction->m_EnvVariables;
						if(tmpparams.size()!= ParamList.size())script.Throw(__FILE__, __LINE__, "函数参数个数错误");
						for (i = 0; i < ParamList.size(); ++i)
						{
							ParamList[i].ExecExpression(script, vars, ret, pe);
							tmpparams[i].second = ret;
						}
						
						T_VARIABLE_S tmpvars;
						tmpvars.FromParentVars(vars, pFunction->count_global_variable, &tmpparams);
						
						ret.type = result_type;
						if (!pFunction->ExeSentences(tmpvars, ret, pe))script.Throw(__FILE__, __LINE__, "函数执行出错");
					}
					break;
				case PLUGIN:
					_paramvars.reserve(ParamList.size());
					for (i = 0; i < ParamList.size(); ++i)
					{
						ParamList[i].ExecExpression(script, vars, ret, pe);
						_paramvars.push_back(ret);
					}
					ret.type = result_type;
					if (!CPluginMap::GetPlugin(pPlugin)->ExecFunction(_paramvars, user_p, ret, msg, pe))script.Throw(__FILE__, __LINE__, "插件执行出错");
					break;
				case OPERATION:
					//计算
					if (NULL == pRightOperand())
					{
						if ("++" == op)
						{
							ret = *vars.FindVariable(pLeftOperand()->VariableName);
							++vars.FindVariable(pLeftOperand()->VariableName)->lValue;
						}
						else if ("--" == op)
						{
							ret = *vars.FindVariable(pLeftOperand()->VariableName);
							--vars.FindVariable(pLeftOperand()->VariableName)->lValue;
						}
						else
						{
							script.Throw(__FILE__, __LINE__, "不支持的前缀操作");
						}
					}
					else if (NULL == pLeftOperand())
					{//前缀
						pRightOperand()->ExecExpression(script, vars, ret_right, pe);
						if (ret_right.isNull())script.Throw(__FILE__, __LINE__, "右操作数计算出错");
						if ("-" == op)
						{
							ret = -ret_right;
						}
						else if ("+" == op)
						{
							ret = ret_right;
						}
						else if ("++" == op)
						{
							++vars.FindVariable(pRightOperand()->VariableName)->lValue;
							ret = *vars.FindVariable(pRightOperand()->VariableName);
						}
						else if ("--" == op)
						{
							--vars.FindVariable(pRightOperand()->VariableName)->lValue;
							ret = *vars.FindVariable(pRightOperand()->VariableName);
						}
						else if ("!" == op)
						{
							ret = ret_right;
							ret.lValue = (0 == ret.lValue ? 1 : 0);
						}
						else
						{
							script.Throw(__FILE__, __LINE__, "不支持的前缀操作");
						}
					}
					else
					{//二元
						if ("||" == op)
						{//若前面为真则不计算后面
							pLeftOperand()->ExecExpression(script, vars, ret_left, pe);
							if (ret_left.isNull())script.Throw(__FILE__, __LINE__, "左操作数计算出错");
							if (ret_left.GetBool())ret = ret_left;
							else
							{
								pRightOperand()->ExecExpression(script, vars, ret_right, pe);
								if (ret_right.isNull())script.Throw(__FILE__, __LINE__, "右操作数计算出错");
								ret = ret_right;
							}
						}
						else
						{
							{
								//先计算左右表达式
								pLeftOperand()->ExecExpression(script, vars, ret_left, pe);
								if (ret_left.isNull())script.Throw(__FILE__, __LINE__, "左操作数计算出错");
								pRightOperand()->ExecExpression(script, vars, ret_right, pe);
								if (ret_right.isNull())script.Throw(__FILE__, __LINE__, "右操作数计算出错");
								else if ("+" == op)ret = ret_left + ret_right;
								else if ("-" == op)ret = ret_left - ret_right;
								else if ("*" == op)ret = ret_left * ret_right;
								else if ("/" == op)ret = ret_left / ret_right;
								else if ("%" == op)ret = ret_left % ret_right;
								else if (">" == op)ret = ret_left > ret_right;
								else if ("<" == op)ret = ret_left < ret_right;
								else if (">=" == op)ret = ret_left >= ret_right;
								else if ("<=" == op)ret = ret_left <= ret_right;
								else if ("==" == op)ret = ret_left == ret_right;
								else if ("!=" == op)ret = ret_left != ret_right;
								else if ("&&" == op)ret = ret_left && ret_right;
								else if ("=" == op)ret = *(vars.FindVariable(pLeftOperand()->VariableName)) = ret_right;
								else if ("+=" == op)ret = *(vars.FindVariable(pLeftOperand()->VariableName)) = ret_left + ret_right;
								else if ("-=" == op)ret = *(vars.FindVariable(pLeftOperand()->VariableName)) = ret_left - ret_right;
								else if ("*=" == op)ret = *(vars.FindVariable(pLeftOperand()->VariableName)) = ret_left * ret_right;
								else if ("/=" == op)ret = *(vars.FindVariable(pLeftOperand()->VariableName)) = ret_left / ret_right;
								else if ("%=" == op)ret = *(vars.FindVariable(pLeftOperand()->VariableName)) = ret_left % ret_right;
								else if ("," == op)
								{
									ret = ret_right;
								}
								else
								{
									script.Throw(__FILE__, __LINE__, "不支持的操作");
								}
							}
						}
						if (Variable::NULLVARIABLE == ret.type)script.Throw(__FILE__, __LINE__, "操作符两边类型不匹配");
					}
					break;
				}
				return true;
			}
			string ToString(string const& source, T_VARIABLE_S const& vars, long level = 0)const
			{
				STATIC_C const char typestr[][TOKEN_BUF_LEN] = { "NULLEXPRESSION","DEFINE","CONSTANT","VARIABLE","FUNCTION","PLUGIN","OPERATION" };//必须与types对应
				char buf[256];
				string ret;
				size_t i;
				string prefix;
				prefix.assign(level * 4, ' ');

				ret = prefix.c_str();
				sprintf(buf, "%03ld %03ld 表达式类型 %-12s 结果类型 %-12s: ", source_start, source_end - 1, typestr[type], Variable::TypeStr(result_type));
				ret += buf;
				ret += source.substr(source_start, source_end - source_start);
				ret += "\r\n";
				switch (type)
				{
				case DEFINE:
					ret += VariableName + " " + m_variable.ToString(level + 1);
					ret += "\r\n";
					break;
				case CONSTANT:
					ret += m_variable.ToString(level + 1);
					ret += "\r\n";
					break;
				case VARIABLE:
					if (NULL == vars.FindVariable(VariableName))
					{
						ret += VariableName;
					}
					else ret += vars.FindVariable(VariableName)->ToString(level + 1);
					ret += "\r\n";
					break;
				case FUNCTION:
					ret += prefix + "函数名： " + pFunction->script_name;
					if (ParamList.size() != 0)
					{
						ret += prefix + "参数：\r\n";
						for (i = 0; i < ParamList.size(); ++i)
						{
							ret += ParamList[i].ToString(source, vars, level + 1);
						}
					}
					else
					{
						ret += "\r\n";
					}
					break;
				case PLUGIN:
					ret += prefix + "插件名： " + CPluginMap::GetPlugin(pPlugin)->plugin_name;
					if (ParamList.size() != 0)
					{
						ret += prefix + "参数：\r\n";
						for (i = 0; i < ParamList.size(); ++i)
						{
							ret += ParamList[i].ToString(source, vars, level + 1);
						}
					}
					else
					{
						ret += "\r\n";
					}
					break;
				case OPERATION:
					if (NULL != pLeftOperand())
					{
						ret += pLeftOperand()->ToString(source, vars, level + 1);
					}
					ret += prefix + "    " + op;
					ret += "\r\n";
					if (NULL != pRightOperand())
					{
						ret += pRightOperand()->ToString(source, vars, level + 1);
					}
					break;
				default:
					break;
				}
				return ret;
			}
		};
		//语句
		//EXPRESSION RETURN:expressions[0]
		//BLOCK:senetnces
		//IF:if(expressions[0])sentences[0] else sentences[1]
		//FOR:for(expressions[0];expressions[1];expressions[2])sentences[0]
		//DO:do sentences[0] while(expressions[0]);
		//WHILE:while(expressions[0])sentences[0]
		struct Sentence
		{
			enum types { NULLSENTENCE = 0, DECLARE, EXPRESSION, BLOCK, RETURN, IF, FOR, BREAK, CONTINUE, DO, WHILE };
			types type;
			size_t source_start;//起始位置
			size_t source_end;//结束位置

			CMyVector<Expression > expressions;
			CMyVector<Sentence > sentences;

			Sentence() { clear(); }
			void clear()
			{
				type = NULLSENTENCE;
				source_start = 0;
				source_end = 0;
				expressions.clear();
				sentences.clear();
			}
			bool isLoop()const { return FOR == type || DO == type || WHILE == type; }
			//inloop=true表示处于循环体内，可能从上层带入
			bool CheckSentence(string& source, T_VARIABLE_S& vars, bool _inloop)
			{
				bool inloop = (_inloop || isLoop());
				size_t i;

				if (FOR == type)
				{
					vars.PushLevel();//只有for语句的表达式可以定义变量
					if (sentences.size() != 2)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "for语句子语句必须有2个");
					if (!sentences[0].CheckSentence(source, vars, inloop))return false;
				}
				for (i = 0; i < expressions.size(); ++i)
				{
					set<string > setpp;
					if (!expressions[i].CheckExpression(source, vars, 0, setpp))return false;
				}
				switch (type)
				{
				case NULLSENTENCE:
					if (expressions.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "空语句不能有表达式");
					if (sentences.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "空语句不能有子语句");
					break;
				case DECLARE:
					if (expressions.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "变量定义语句有且只能有一个表达式");
					if (sentences.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "变量定义语句语句不能有子语句");
					if (expressions[0].type != Expression::DEFINE)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "变量定义语句表达式必须是DEFINE类型");
					if (0 == expressions[0].VariableName.size())CCTSException::Throw(__FILE__, __LINE__, source, source_start, "变量定义语句表达式必须有变量名");
					break;
				case EXPRESSION:
					if (expressions.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "简单语句表达式只能有一个");
					if (sentences.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "简单语句不能有子语句");
					break;
				case BLOCK:
					if (expressions.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "块语句不能有表达式");
					break;
				case RETURN:
					if (expressions.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "return语句表达式只能有一个");
					if (sentences.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "return语句不能有子语句");
					break;
				case IF:
					if (expressions.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "if语句表达式只能有一个");
					if (sentences.size() != 1 && sentences.size() != 2)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "if语句子语句只能有1或2个");
					if (expressions[0].result_type != Variable::LONG && expressions[0].result_type != Variable::DOUBLE)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "if语句判断表达式必须是数值类型");
					break;
				case DO:
					if (expressions.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "do语句表达式只能有一个");
					if (sentences.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "do语句子语句只能有1个");
					if (expressions[0].result_type != Variable::LONG && expressions[0].result_type != Variable::DOUBLE)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "do语句判断表达式必须是数值类型");
					break;
				case WHILE:
					if (expressions.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "while语句表达式只能有一个");
					if (sentences.size() != 1)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "while语句子语句只能有1个");
					if (expressions[0].result_type != Variable::LONG && expressions[0].result_type != Variable::DOUBLE)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "while语句判断表达式必须是数值类型");
					break;
				case FOR:
					if (expressions.size() != 2)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "for语句表达式必须有2个");
					if (sentences.size() != 2)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "for语句子语句必须有2个");
					if (expressions[1].result_type != Variable::LONG && expressions[1].result_type != Variable::DOUBLE)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "for语句判断表达式必须是数值类型");
					break;
				case BREAK:
					if (expressions.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "break语句不能有表达式");
					if (sentences.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "break语句不能有子语句");
					break;
				case CONTINUE:
					if (expressions.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "break语句不能有表达式");
					if (sentences.size() != 0)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "break语句不能有子语句");
					break;
				default:
					CCTSException::Throw(__FILE__, __LINE__, source, source_start, "暂不支持");
					return false;
					break;
				}
				if (BLOCK == type)vars.PushLevel();
				for (i = 0; i < sentences.size(); ++i)
				{
					if (BLOCK != type)vars.PushLevel();//每个子语句都是一个层次，if有1或2个子语句，do\while\for有1个子语句(for的第一个语句已经提前检查)
					if ((FOR == type && 0 != i) || FOR != type)if (!sentences[i].CheckSentence(source, vars, inloop))return false;
					if (!inloop)
					{
						if (BREAK == sentences[i].type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "非期待的break语句");
						if (CONTINUE == sentences[i].type)CCTSException::Throw(__FILE__, __LINE__, source, source_start, "非期待的continue语句");
					}
					if (BLOCK != type)vars.PopLevel();
				}
				if (BLOCK == type)vars.PopLevel();
				if (FOR == type)vars.PopLevel();
				return true;
			}
			//若为return语句则设置bFinish
			bool ExecSentence(CCTScript const& script, T_VARIABLE_S& vars, bool& bFinish, bool& bBreak, bool& bContinue, Variable& ret, void* pe)const
			{
				size_t i;
				if (bFinish)return true;
				if (bBreak)return true;
				if (bContinue)return true;
				ret.type = Variable::NULLVARIABLE;
				switch (type)
				{
				case NULLSENTENCE:
					ret.type = Variable::NULLVARIABLE;
					break;
				case DECLARE:
					expressions[0].ExecExpression(script, vars, ret, pe);
					break;
				case EXPRESSION:
					expressions[0].ExecExpression(script, vars, ret, pe);
					break;
				case BLOCK:
					vars.PushLevel();
					for (i = 0; i < sentences.size(); ++i)
					{
						sentences[i].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
						if (bFinish)break;
						if (bBreak)break;
						if (bContinue)break;
					}
					vars.PopLevel();
					break;
				case RETURN:
					expressions[0].ExecExpression(script, vars, ret, pe);
					bFinish = true;
					break;
				case IF:
					expressions[0].ExecExpression(script, vars, ret, pe);
					vars.PushLevel();
					if (ret.GetBool())
					{
						sentences[0].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
					}
					else
					{
						if (sentences.size() >= 2)sentences[1].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
						else ret.type = Variable::NULLVARIABLE;
					}
					vars.PopLevel();
					break;
				case FOR:
					vars.PushLevel();
					sentences[0].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
					while (true)
					{
						expressions[0].ExecExpression(script, vars, ret, pe);
						if (!ret.GetBool())break;
						vars.PushLevel();
						sentences[1].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
						vars.PopLevel();
						if (bFinish)break;
						if (bBreak)
						{
							bBreak = false;
							break;
						}
						if (bContinue)bContinue = false;
						expressions[1].ExecExpression(script, vars, ret, pe);
					}
					vars.PopLevel();
					//ret.type=Variable::NULLVARIABLE;
					break;
				case DO:
					while (true)
					{
						vars.PushLevel();
						sentences[0].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
						vars.PopLevel();
						if (bFinish)break;
						if (bBreak)
						{
							bBreak = false;
							break;
						}
						if (bContinue)bContinue = false;

						expressions[0].ExecExpression(script, vars, ret, pe);
						if (!ret.GetBool())break;
					}
					//ret.type=Variable::NULLVARIABLE;
					break;
				case WHILE:
					while (true)
					{
						expressions[0].ExecExpression(script, vars, ret, pe);
						if (!ret.GetBool())break;

						vars.PushLevel();
						sentences[0].ExecSentence(script, vars, bFinish, bBreak, bContinue, ret, pe);
						vars.PopLevel();
						if (bFinish)break;
						if (bBreak)
						{
							bBreak = false;
							break;
						}
						if (bContinue)bContinue = false;
					}
					//ret.type=Variable::NULLVARIABLE;
					break;
				case BREAK:
					bBreak = true;
					break;
				case CONTINUE:
					bContinue = true;
					break;
				default:
					script.ThrowN(__FILE__, __LINE__, "暂不支持的语句类型", type);
					return false;
					break;
				}
				return true;
			}
			string ToString(string const& source, T_VARIABLE_S const& vars, long level = 0)const
			{
				STATIC_C const char typestr[][TOKEN_BUF_LEN] = { "NULLSENTENCE","DECLARE","EXPRESSION","BLOCK","RETURN","IF","FOR","BREAK","CONTINUE","DO","WHILE" };//必须与types对应
				char buf[256];
				string ret;
				size_t i;
				string prefix;
				prefix.assign(level * 4, ' ');
				ret = prefix;
				sprintf(buf, "%03ld %03ld 语句类型 %-12s : ", source_start, source_end - 1, typestr[type]);
				ret += buf;
				ret += source.substr(source_start, source_end - source_start);
				ret += "\r\n";
				if (0 != expressions.size())
				{
					ret += prefix;
					ret += "    表达式：\r\n";
					for (i = 0; i < expressions.size(); ++i)
					{
						ret += expressions[i].ToString(source, vars, level + 1);
					}
				}
				if (0 != sentences.size())
				{
					ret += prefix;
					ret += "    子语句：\r\n";
					for (i = 0; i < sentences.size(); ++i)
					{
						ret += sentences[i].ToString(source, vars, level + 1);
					}
				}
				return ret;
			}
		};
	public:
		string m_source;
		Variable::types return_type;
		string script_name;//如果是子函数，这是函数名
		long count_global_variable;//如果是子函数，这是全局变量个数（只能使用之前定义的全局变量）
	private:
		bool m_compiled;
		bool m_isFunction;//是否是子函数
		long m_execcount;//执行次数
		string m_msg;
		T_VARIABLE_BLOCK m_EnvVariables;//环境变量表，对于子函数，是参数
		CMyVector<Sentence > m_sentences;//语句组
		CMyVector<CCTScript > m_functions;//子函数

		vector<Expression*> m_NewExpressions;//所有new出来的表达式，析构时要清理

		void Init(char const* _source)
		{
			m_compiled = false;
			m_isFunction = false;
			script_name = "";
			count_global_variable = -1;
			m_msg = "";
			m_source = _source;
			return_type = Variable::types::NULLVARIABLE;
			m_EnvVariables.clear();
			m_sentences.clear();
			m_functions.clear();

			for (vector<Expression*>::iterator it = m_NewExpressions.begin(); it != m_NewExpressions.end(); ++it)
			{
				delete(*it);
			}
			m_NewExpressions.clear();
		}
		//子函数从父脚本获得已经基础数据
		void FromParent(CCTScript& parent,string const & name)
		{
			Init(parent.m_source.c_str());
			m_isFunction = true;
			script_name = name;
		}
		Expression* NewExpression()
		{
			Expression* p = new Expression;
			if (NULL != p)m_NewExpressions.push_back(p);
			return p;
		}
		CCTScript * FindFunction(string const& name)
		{
			for (size_t i = 0; i < m_functions.size(); ++i)
			{
				if (name == m_functions[i].script_name)return &m_functions[i];
			}
			return NULL;
		}
		bool GetOperand(CCTTokens& tokens, T_VARIABLE_S& vars, Expression*& pOperand, size_t& pos)
		{
			Token* pToken;
			string variable_name;
			Variable variable;
			tokens.MoveCurrentToken(pToken, pos);
			if (tokens.IsPosNotToken(pos))
			{
				pOperand = NULL;
			}
			else
			{
				switch (pToken->type)
				{
				case Token::IDENTIFIER:
					if (!tokens.IsPosNotToken(pos + 1) && tokens.IsOperator(pos + 1, "("))
					{
						CCTScript* tmp_pFunction = FindFunction(pToken->text);
						if (NULL!= tmp_pFunction)
						{
							pOperand = NewExpression();
							if (NULL == pOperand)throw "内存不足";
							pOperand->type = Expression::FUNCTION;
							pOperand->pFunction = tmp_pFunction;
						}
						else
						{
							if (NULL == CPluginMap::GetPlugin(pToken->text))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未定义的函数或插件");

							pOperand = NewExpression();
							if (NULL == pOperand)throw "内存不足";
							pOperand->type = Expression::PLUGIN;
							pOperand->pPlugin.plugin_name = pToken->text;
						}
						pOperand->source_start = tokens.TokenStart(pos);

						tokens.MoveNextToken(pToken, pos);
						tokens.MoveNextToken(pToken, pos);
						while (true)
						{
							if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待右括号");
							if (tokens.IsOperator(pos, ")"))
							{
								tokens.MoveNextToken(pToken, pos);
								break;
							}
							Expression* pParam = GetExpression(tokens, vars, NULL, pos, ",)");
							pOperand->ParamList.reserve(pOperand->ParamList.size() + 1);
							pOperand->ParamList.push_back(*pParam);
							if (tokens.IsOperator(pos, ","))
							{
								tokens.MoveNextToken(pToken, pos);
								if (tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的右括号");
							}
						}
						pOperand->source_end = tokens.TokenStart(pos);
					}
					else
					{
						pOperand = NewExpression();
						if (NULL == pOperand)throw "内存不足";

						pOperand->VariableName = pToken->text;
						pOperand->type = Expression::VARIABLE;
						pOperand->source_start = tokens.TokenStart(pos);
						++pos;
						pOperand->source_end = tokens.TokenStart(pos);
					}
					break;
				case Token::NUMBER:
					pOperand = NewExpression();
					if (NULL == pOperand)throw "内存不足";
					if (!tokens.NumberToVariable(pToken->text.c_str(), variable))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "NumberToVariable错误");

					pOperand->m_variable = variable;
					pOperand->m_variable.isconst = true;
					pOperand->type = Expression::CONSTANT;
					pOperand->source_start = tokens.TokenStart(pos);
					++pos;
					pOperand->source_end = tokens.TokenStart(pos);
					break;
				case Token::STRING:
					pOperand = new Expression;
					if (NULL == pOperand)throw "内存不足";
					variable = tokens.TokenToString(pToken->text.c_str());
					variable.isconst = true;

					pOperand->m_variable = variable;
					pOperand->m_variable.isconst = true;
					pOperand->type = Expression::CONSTANT;
					pOperand->source_start = tokens.TokenStart(pos);
					++pos;
					pOperand->source_end = tokens.TokenStart(pos);
					break;
				case Token::OPERATOR:
					if (tokens.IsOperator(pos, "("))
					{
						tokens.MoveNextToken(pToken, pos);
						pOperand = GetExpression(tokens, vars, NULL, pos, ")");
						tokens.MoveCurrentToken(pToken, pos);
						if (!tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待右括号");
						tokens.MoveNextToken(pToken, pos);
					}
					break;
				case Token::KEYWORD:
					CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "此处不期待关键字");
					break;
				case Token::DELIMITER:
					break;
				default:
					CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未支持的标记类型");
					break;
				}
			}
			return NULL != pOperand;
		}
		bool GetOperator(CCTTokens& tokens, Token*& pOperator, size_t& pos)
		{
			Token* pToken;
			tokens.MoveCurrentToken(pToken, pos);
			if (!tokens.IsPosNotToken(pos) && pToken->type == Token::OPERATOR)
			{
				pOperator = pToken;
				++pos;
			}
			return NULL != pOperator;
		}
		//);,]
		bool isExpressionEnd(CCTTokens& tokens, size_t pos, char const* endch)
		{
			if (pos >= tokens.m_tokens.size())return true;

			string str = endch;
			if (str == ";")return tokens.IsDelimiter(pos, str.c_str());
			else
			{
				if (tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的分号");
				for (size_t i = 0; i < str.size(); ++i)
				{
					string tmp;
					tmp += str[i];
					if (tokens.IsOperator(pos, tmp.c_str()))return true;
				}
			}
			return false;
		}
		Expression* GetExpression(CCTTokens& tokens, T_VARIABLE_S& vars, Expression* pExpression, size_t& pos, char const* endch, bool half = false)
		{
			size_t start_pos = pos;
			size_t op_pos = 0;
			Token* pToken;
			Expression* pOperand = NULL;//下一个操作数
			Token* pOperator = NULL;//指向下一个操作符

			while (true)
			{
				if (NULL == pOperand && NULL == pOperator)
				{
					tokens.MoveCurrentToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))
					{
						return pExpression;
					}

					if (isExpressionEnd(tokens, pos, endch))
					{//空语句
						if (NULL == pExpression)
						{
							pExpression = NewExpression();
							pExpression->source_start = tokens.TokenStart(pos);
							pExpression->source_end = tokens.TokenStart(pos);
							if (NULL == pExpression)throw "内存不足";
						}
						return pExpression;
					}
					//取下一个操作数，可能没有（当下一个标记是操作符）
					if (NULL != pOperator)thelog << pOperator->text << endi;
					GetOperand(tokens, vars, pOperand, pos);
					//取下一个操作符
					if (GetOperator(tokens, pOperator, pos))op_pos = pos - 1;
				}

				if (NULL == pOperand && NULL == pOperator)
				{//什么都没取到，结束
					return pExpression;
				}

				if (NULL == pExpression)
				{//新建表达式
					if (NULL == pOperator || isExpressionEnd(tokens, pos - 1, endch))
					{
						if (isExpressionEnd(tokens, pos - 1, endch))--pos;
						else
						{
							string str;
							if (!isExpressionEnd(tokens, pos, endch))
							{
								thelog << tokens.m_tokens[pos - 1].text << " " << endch << endi;
								thelog << pOperand->ToString(m_source, vars) << endi;
								CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待操作符");
							}
						}
						pExpression = pOperand;
						return pExpression;
					}
					else
					{
						pExpression = NewExpression();
						if (NULL == pExpression)throw "内存不足";
						pExpression->source_start = tokens.TokenStart(start_pos);
						pExpression->source_end = tokens.TokenStart(op_pos);
						pExpression->type = Expression::OPERATION;
						pExpression->op = pOperator->text;
						if (NULL != pOperand)pExpression->AddLeftOperand(*pOperand);
					}
					pOperand = NULL;
					pOperator = NULL;
				}
				else
				{//追加表达式
					if (NULL == pOperator || isExpressionEnd(tokens, pos - 1, endch))
					{//操作符为空
						if (isExpressionEnd(tokens, pos - 1, endch))--pos;
						pExpression->AddRightOperand(*pOperand);
						pExpression->source_end = tokens.TokenStart(pos);
						pOperand = NULL;
						return pExpression;
					}
					else
					{
						long level1, level2;
						if (NULL == pExpression->pLeftOperand())level1 = 2;//单目算符
						else if (!tokens.GetOperatorLevel(pExpression->op.c_str(), level1))Throw(__FILE__, __LINE__, pExpression->op + " 操作符优先级未知或此处不应该出现此操作符");
						if (NULL == pOperand)level2 = 2;//单目算符
						else if (!tokens.GetOperatorLevel(pOperator->text.c_str(), level2))Throw(__FILE__, __LINE__, pOperator->text + " 操作符优先级未知或此处不应该出现此操作符");
						if (level1 < level2 || (level1 == level2 && tokens.IsOperatorLeftFirst(level1)))
						{
							pExpression->AddRightOperand(*pOperand);
							pExpression->source_end = tokens.TokenStart(pos - 1);
							if (half)
							{
								--pos;//退回最后一个操作符
								return pExpression;
							}
							Expression* tmp = NewExpression();
							if (NULL == tmp)throw "内存不足";
							tmp->type = Expression::OPERATION;
							tmp->source_start = pExpression->source_start;
							tmp->source_end = op_pos;
							tmp->op = pOperator->text;
							tmp->AddLeftOperand(*pExpression);
							pExpression = tmp;
							pOperand = NULL;
							pOperator = NULL;
						}
						else
						{
							Expression* tmp = NewExpression();
							if (NULL == tmp)throw "内存不足";
							tmp->type = Expression::OPERATION;
							if (NULL == pOperand)tmp->source_start = tokens.TokenStart(op_pos);
							else tmp->source_start = pOperand->source_start;
							tmp->source_end = tokens.TokenStart(op_pos);
							tmp->op = pOperator->text;
							tmp->AddLeftOperand(*pOperand);
							pOperand = NULL;
							pOperator = NULL;

							pOperand = GetExpression(tokens, vars, tmp, pos, endch, true);
							if (GetOperator(tokens, pOperator, pos))op_pos = pos - 1;
						}
					}
				}
			}
			//return pExpression;
		}
		bool GetSentence(CCTTokens& tokens, T_VARIABLE_S& vars, Sentence& sentence, Token*& pToken, size_t& pos)
		{
			sentence.clear();
			Expression* pExpression;

			if (tokens.IsPosNotToken(pos))return false;
			sentence.source_start = tokens.TokenStart(pos);
			sentence.source_end = tokens.TokenStart(pos);
			tokens.MoveCurrentToken(pToken, pos);
			switch (pToken->type)
			{
			case Token::IDENTIFIER:
			case Token::NUMBER:
			case Token::STRING:
			case Token::OPERATOR:
				sentence.type = Sentence::EXPRESSION;
				pExpression = GetExpression(tokens, vars, NULL, pos, ";");
				if (!tokens.IsPosNotToken(pos) && tokens.IsDelimiter(pos, ";"))
				{
					tokens.MoveNextToken(pToken, pos);
				}
				sentence.expressions.reserve(1);
				sentence.expressions.push_back(*pExpression);
				break;
			case Token::DELIMITER:
				if (pToken->text == "{")
				{
					sentence.type = Sentence::BLOCK;
					tokens.MoveNextToken(pToken, pos);
					while (true)
					{
						if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，块语句未结束");
						if (tokens.IsDelimiter(pos, "}"))
						{
							tokens.MoveNextToken(pToken, pos);
							break;
						}
						Sentence tmpsentence;
						if (!GetSentence(tokens, vars, tmpsentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "获取块语句的语句出错");
						sentence.sentences.reserve(sentence.sentences.size() + 1);
						sentence.sentences.push_back(tmpsentence);
					}
				}
				else if (pToken->text == "}")
				{
					CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的块语句结束标记");
				}
				else if (pToken->text == ";")
				{
					sentence.type = Sentence::NULLSENTENCE;
					tokens.MoveNextToken(pToken, pos);
				}
				else
				{
					CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "尚未支持的分隔符");
				}
				break;
			case Token::KEYWORD:
				if (pToken->text == "return")
				{
					sentence.type = Sentence::RETURN;
					tokens.MoveNextToken(pToken, pos);
					pExpression = GetExpression(tokens, vars, NULL, pos, ";");
					Expression null_expression;
					if (NULL == pExpression)
					{
						pExpression = &null_expression;
					}
					if (!tokens.IsPosNotToken(pos) && tokens.IsDelimiter(pos, ";"))
					{
						tokens.MoveNextToken(pToken, pos);
					}
					sentence.expressions.reserve(1);
					sentence.expressions.push_back(*pExpression);
				}
				else if (pToken->text == "if")
				{
					sentence.type = Sentence::IF;
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待左括号");
					if (!tokens.IsOperator(pos, "("))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待左括号");
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待条件表达式");
					if (NULL == (pExpression = GetExpression(tokens, vars, NULL, pos, ")")))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "条件表达式为空");
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待右括号");
					if (!tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待右括号");
					sentence.expressions.reserve(1);
					sentence.expressions.push_back(*pExpression);
					tokens.MoveNextToken(pToken, pos);
					sentence.sentences.reserve(2);
					Sentence if_sentence;
					if (!GetSentence(tokens, vars, if_sentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "分析if块的执行语句出错");
					sentence.sentences.push_back(if_sentence);
					if (!tokens.IsPosNotToken(pos))
					{
						if (pToken->type == Token::KEYWORD && pToken->text == "else")
						{
							tokens.MoveNextToken(pToken, pos);
							Sentence else_sentence;
							if (!GetSentence(tokens, vars, else_sentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "分析else块的执行语句出错");
							sentence.sentences.push_back(else_sentence);
						}
					}
				}
				else if (pToken->text == "do")
				{
					sentence.type = Sentence::DO;
					tokens.MoveNextToken(pToken, pos);
					sentence.sentences.reserve(1);
					Sentence if_sentence;
					if (!GetSentence(tokens, vars, if_sentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "分析do块的执行语句出错");
					sentence.sentences.push_back(if_sentence);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待while");
					if (pToken->type != Token::KEYWORD || pToken->text != "while")CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待while");
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待左括号");
					if (!tokens.IsOperator(pos, "("))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待左括号");
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待条件表达式");
					if (NULL == (pExpression = GetExpression(tokens, vars, NULL, pos, ")")))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "条件表达式为空");
					tokens.MoveCurrentToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待右括号");
					if (!tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待右括号");
					sentence.expressions.reserve(1);
					sentence.expressions.push_back(*pExpression);
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待分号");
					if (!tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号");
					tokens.MoveNextToken(pToken, pos);
				}
				else if (pToken->text == "while")
				{
					sentence.type = Sentence::WHILE;
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待左括号");
					if (!tokens.IsOperator(pos, "("))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待左括号");
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待条件表达式");
					if (NULL == (pExpression = GetExpression(tokens, vars, NULL, pos, ")")))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "条件表达式为空");
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待右括号");
					if (!tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待右括号");
					sentence.expressions.reserve(1);
					sentence.expressions.push_back(*pExpression);
					tokens.MoveNextToken(pToken, pos);
					sentence.sentences.reserve(1);
					Sentence if_sentence;
					if (!GetSentence(tokens, vars, if_sentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "分析while块的执行语句出错");
					sentence.sentences.push_back(if_sentence);
				}
				else if (pToken->text == "for")
				{
					sentence.type = Sentence::FOR;//第一个分号前面是一个语句
					sentence.expressions.reserve(2);
					sentence.sentences.reserve(2);

					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待左括号");
					if (!tokens.IsOperator(pos, "("))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待左括号");

					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待初始化表达式");
					Sentence init_sentence;
					if (!GetSentence(tokens, vars, init_sentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "分析for块的初始化语句出错");
					sentence.sentences.push_back(init_sentence);

					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待条件表达式");
					if (NULL == (pExpression = GetExpression(tokens, vars, NULL, pos, ";")))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "获取条件表达式出错");
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待分号");
					if (!tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号");
					sentence.expressions.push_back(*pExpression);

					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待循环递增表达式");
					if (NULL == (pExpression = GetExpression(tokens, vars, NULL, pos, ")")))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "获取循环递增表达式出错");
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待右括号");
					if (!tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待右括号");
					sentence.expressions.push_back(*pExpression);

					tokens.MoveNextToken(pToken, pos);
					Sentence for_sentence;
					if (!GetSentence(tokens, vars, for_sentence, pToken, pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "分析for块的执行语句出错");
					sentence.sentences.push_back(for_sentence);
				}
				else if ("break" == pToken->text)
				{
					sentence.type = Sentence::BREAK;
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待分号");
					if (!tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号");
					tokens.MoveNextToken(pToken, pos);
				}
				else if ("continue" == pToken->text)
				{
					sentence.type = Sentence::CONTINUE;
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待分号");
					if (!tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号");
					tokens.MoveNextToken(pToken, pos);
				}
				else if ("long" == pToken->text || "double" == pToken->text || "int" == pToken->text || "float" == pToken->text || "string" == pToken->text)
				{//声明语句
					sentence.type = Sentence::DECLARE;
					Variable var;
					if ("long" == pToken->text || "int" == pToken->text)var.type = Variable::LONG;
					else if ("double" == pToken->text || "float" == pToken->text)var.type = Variable::DOUBLE;
					else var.type = Variable::STRING;

					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待变量名");
					if (tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的分号，期待变量名");
					if (pToken->type != Token::IDENTIFIER)CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待标识符");
					sentence.expressions.resize(1);
					sentence.expressions[0].type = Expression::DEFINE;
					sentence.expressions[0].VariableName = pToken->text;
					sentence.expressions[0].m_variable = var;
					sentence.expressions[0].source_start = sentence.source_start;
					sentence.expressions[0].source_end = tokens.TokenStart(pos + 1);
					tokens.MoveNextToken(pToken, pos);
					if (tokens.IsPosNotToken(pos))
					{
					}
					else if (tokens.IsOperator(pos, "="))
					{//带有初始值
						tokens.MoveNextToken(pToken, pos);
						if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待初始化表达式");
						if (NULL == (pExpression = GetExpression(tokens, vars, NULL, pos, ";")))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "获取初始化表达式出错");
						if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待分号");
						if (!tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号");
						tokens.MoveNextToken(pToken, pos);
						sentence.expressions[0].AddLeftOperand(*pExpression);
					}
					else if (tokens.IsOperator(pos, "("))
					{//函数
						for (size_t i = 0; i < m_functions.size(); ++i)
						{
							if (m_functions[i].script_name == sentence.expressions[0].VariableName)CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "函数重名");
						}
						m_functions.resize(m_functions.size() + 1);
						CCTScript* pFunction = &m_functions[m_functions.size() - 1];
						pFunction->FromParent(*this, sentence.expressions[0].VariableName);
						pFunction->return_type = var.type;
						pFunction->count_global_variable = vars.getGlobalCount();
					
						tokens.MoveNextToken(pToken, pos);
						while (!tokens.IsOperator(pos, ")"))
						{
							Variable param;
							if ("long" == pToken->text || "int" == pToken->text)param.type = Variable::LONG;
							else if ("double" == pToken->text || "float" == pToken->text)param.type = Variable::DOUBLE;
							else if("string"== pToken->text)param.type = Variable::STRING;
							else CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待参数类型");
							tokens.MoveNextToken(pToken, pos);
							if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待参数名");
							if (pToken->type != Token::IDENTIFIER)CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待标识符");
							if (!pFunction->m_EnvVariables.AddVariable(pToken->text, param))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "函数参数重名");
							if (NULL!=vars.FindVariable(pToken->text))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "函数参数与全局变量或环境变量重名");
							tokens.MoveNextToken(pToken, pos);
							if (tokens.IsPosNotToken(pos))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未期待的脚本结束，期待)");
							if (tokens.IsOperator(pos, ","))tokens.MoveNextToken(pToken, pos);
							else break;
						}
						if (!tokens.IsOperator(pos, ")"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待)");
						tokens.MoveNextToken(pToken, pos);
						if (!tokens.IsDelimiter(pos, "{"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待{");
						tokens.MoveNextToken(pToken, pos);
						
						T_VARIABLE_S tmpvars;
						tmpvars.FromParentVars(vars, pFunction->count_global_variable, &pFunction->m_EnvVariables);
						if (!pFunction->Build(tokens, tmpvars, pos))
						{
							m_msg += "编译失败\r\n";
							return false;
						}
						
						if (!tokens.IsDelimiter(pos, "}"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待}");
						tokens.MoveNextToken(pToken, pos);
						if (!tokens.IsDelimiter(pos, ";"))CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号");
						tokens.MoveNextToken(pToken, pos);
						
						return GetSentence(tokens, vars, sentence, pToken, pos);
					}
					else if (!tokens.IsDelimiter(pos, ";"))
					{
						CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "期待分号，一个语句只能定义一个变量");
					}
				}
				else
				{
					CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "非期待的的关键字，是否前面缺少分号？");
				}
				break;
			default:
				CCTSException::Throw(__FILE__, __LINE__, m_source, tokens.TokenStart(pos), "未支持的标记类型");
				break;
			}
			sentence.source_end = tokens.TokenStart(pos);
			return sentence.source_end != sentence.source_start;
		}
		bool Build(CCTTokens& tokens, T_VARIABLE_S& vars, size_t& pos)
		{
			bool isFunction = (0 != pos);
			while (pos < tokens.m_tokens.size())
			{
				if (isFunction)
				{
					if (tokens.IsDelimiter(pos, "}"))
					{
						break;
					}
				}
				m_sentences.resize(m_sentences.size() + 1);
				Token* pToken;
				if (!GetSentence(tokens, vars, m_sentences[m_sentences.size() - 1], pToken, pos))return false;
				if (!m_sentences[m_sentences.size() - 1].CheckSentence(m_source, vars, false))return false;
			}
			m_compiled = true;
			return true;
		}
		bool ExeSentences(T_VARIABLE_S& vars, Variable& ret, void* pe)const
		{
			size_t i;
			bool bFinish = false;
			bool bBreak = false;
			bool bContinue = false;

			if (!m_compiled)Throw(__FILE__, __LINE__, "尚未编译");

			//执行
			for (i = 0; i < m_sentences.size(); ++i)
			{
				m_sentences[i].ExecSentence(*this, vars, bFinish, bBreak, bContinue, ret, pe);
				if (bFinish)break;
			}
			return true;
		}
		bool AddEnvVariableDelcare(CMyVector<pair<string, Variable > >* pVars)
		{
			if (NULL == pVars)return true;

			for (CMyVector<pair<string, Variable > >::iterator it = pVars->begin(); it != pVars->end(); ++it)
			{
				if (Variable::NULLVARIABLE == it->second.type)
				{
					Throw(__FILE__, __LINE__, (it->first + " 环境变量必需有类型").c_str());
					return false;
				}
				if (0 == it->first.size())
				{
					Throw(__FILE__, __LINE__, "环境变量必需有名称");
					return false;
				}
				if (!(it->first[0] == '_' || (it->first[0] >= 'a' && it->first[0] <= 'z') || (it->first[0] >= 'A' && it->first[0] <= 'Z')))
				{
					Throw(__FILE__, __LINE__, (it->first + "：变量名错误，环境变量必须以字符或下划线开始").c_str());
					return false;
				}
				if (NULL != m_EnvVariables.FindVariable(it->first))
				{
					Throw(__FILE__, __LINE__, (it->first + " 环境变量已经存在").c_str());
					return false;
				}
				m_EnvVariables.AddVariable(it->first, it->second);
			}
			return true;
		}
		bool SetVariableValue(T_VARIABLE_S* pOldVars, CMyVector<pair<string, Variable > >* pNewVars)const
		{
			if (NULL == pNewVars)return true;

			for (CMyVector<pair<string, Variable > >::iterator it = pNewVars->begin(); it != pNewVars->end(); ++it)
			{
				Variable* it_v = pOldVars->FindVariable(it->first);
				if (NULL != it_v)
				{
					*it_v = it->second;
				}
				else
				{
					Throw(__FILE__, __LINE__, (it->first + " 变量未定义").c_str());
					return false;
				}
			}
			return true;
		}
		bool Execute_const(T_VARIABLE_S& curentVars, long* execcount, string& msg, Variable& ret, CMyVector<pair<string, Variable > >* pEnvs = NULL, void* pe = NULL)const
		{
			++* execcount;
			try
			{
				if (!SetVariableValue(&curentVars, pEnvs))
				{
					msg += "设置环境变量失败\r\n";
					return false;
				}
				if (!ExeSentences(curentVars, ret, pe))
				{
					msg += "执行失败\r\n";
					return false;
				}
			}
			catch (string const& e)
			{
				msg += e;
				return false;
			}
			catch (char const* s)
			{
				msg += s;
				return false;
			}
			return true;
		}
	public:
		CCTScript(void)
		{
			m_compiled = false;
			m_execcount = 0;
			m_sentences.reserve(1);
		}
		~CCTScript(void) { Init(""); }
		string const& GetSource()const { return m_source; }
		bool IsCompiled()const { return m_compiled; }
		string GetMessage()const { return m_msg; }
		void ThrowN(char const* file, long line, char const* _msg, long n)const
		{
			string msg;
			char buf[256];
			msg += "来自文件";
			msg += file;
			sprintf(buf, "的行%ld的出错信息：", line);
			msg += buf;
			sprintf(buf, " %ld", n);
			msg += buf;
			msg += _msg;
			throw msg;
		}
		void Throw(char const* file, long line, string const& _msg)const
		{
			Throw(file, line, _msg.c_str());
		}
		void Throw(char const* file, long line, char const* _msg)const
		{
			string msg;
			char buf[256];
			msg += "来自文件";
			msg += file;
			sprintf(buf, "的行%ld的出错信息：", line);
			msg += buf;
			msg += _msg;
			throw msg;
		}

		bool Compile(char const* _source, CMyVector<pair<string, Variable > >* pEnvs = NULL)
		{
			Init(_source);
			string str;
			CCTTokens tokens;
			T_VARIABLE_BLOCK currentGlobals;
			T_VARIABLE_BLOCK currentParams;
			T_VARIABLE_S curentVars;
			curentVars.T_VARIABLE_S_init(&m_EnvVariables, &currentGlobals, -1, &currentParams);//当前环境变量和全局变量在子函数中同样有效，必须独立存储

			try
			{
				if (!AddEnvVariableDelcare(pEnvs))
				{
					m_msg += "添加环境变量失败\r\n";
					return false;
				}
				if (!tokens.ToTokens(m_source))
				{
					m_msg += "编译失败\r\n";
					return false;
				}
				size_t pos = 0;
				if (!Build(tokens, curentVars, pos))
				{
					m_msg += "编译失败\r\n";
					return false;
				}
			}
			catch (string const& e)
			{
				m_msg += e;
				Report(&tokens, curentVars, str);
				m_msg += str;
				return false;
			}
			catch (char const* s)
			{
				m_msg += s;
				Report(&tokens, curentVars, str);
				m_msg += str;
				return false;
			}
			return true;
		}
		bool Execute(string& msg, Variable& ret, CMyVector<pair<string, Variable > >* pEnvs = NULL, void* pe = NULL)
		{
			T_VARIABLE_BLOCK currentGlobals;
			T_VARIABLE_S curentVars;
			T_VARIABLE_BLOCK currentParams;
			curentVars.T_VARIABLE_S_init(&m_EnvVariables, &currentGlobals, -1, &currentParams);//当前环境变量和全局变量在子函数中同样有效，必须独立存储
			long execcount;
			return Execute_const(curentVars, &execcount, msg, ret, pEnvs, pe);
		}

		string& Report(string& ret)const
		{
			T_VARIABLE_BLOCK currentGlobals;
			T_VARIABLE_S curentVars;
			T_VARIABLE_BLOCK currentParams;
			curentVars.T_VARIABLE_S_init(&m_EnvVariables, &currentGlobals, -1, &currentParams);//当前环境变量和全局变量在子函数中同样有效，必须独立存储
			return Report(NULL, curentVars, ret);
		}
		string& Report(CCTTokens const* pTokens, T_VARIABLE_S const& vars, string& ret)const
		{
			ret = "";
			size_t i;
			char buf[256];
			string hr;
			hr.assign(80, '-');

			if (0 == m_source.size())
			{
				ret += "无脚本\r\n";
				return ret;
			}

			if (!m_isFunction)
			{
				ret += "脚本：\r\n";
				ret += m_source;
				ret += "\r\n" + hr + "\r\n";

				if (pTokens)
				{
					ret += "单词(编号 起始字符位置)：\r\n";
					ret += hr + "\r\n";
					for (i = 0; i < pTokens->m_tokens.size(); ++i)
					{
						sprintf(buf, "%03ld ", i);
						ret += buf;
						ret += pTokens->m_tokens[i].ToString();
						ret += "\r\n";
					}
					ret += hr + "\r\n";
				}

				ret += "函数：\r\n";
				ret += hr + "\r\n";
				for (i = 0; i < m_functions.size(); ++i)
				{
					string str;
					T_VARIABLE_S tmpvars;
					tmpvars.FromParentVars(vars, m_functions[i].count_global_variable, &m_functions[i].m_EnvVariables);
					ret += m_functions[i].Report(pTokens, tmpvars, str);
					ret += hr + "\r\n";
				}
			}
			else
			{
				ret += hr + "函数开始 " + script_name + " 全局变量个数 ";
				sprintf(buf, "%ld", count_global_variable);
				ret += buf;
				ret += " 返回类型 ";
				ret += Variable::TypeStr(return_type);
				ret += "\r\n";
			}

			ret += "语句(起始单词 结束单词)：\r\n";
			ret += hr + "\r\n";
			for (i = 0; i < m_sentences.size(); ++i)
			{
				ret += m_sentences[i].ToString(m_source, vars);
				ret += hr + "\r\n";
			}

			ret += "变量：\r\n";
			ret += hr + "\r\n";
			ret += vars.ToString();

			if (m_msg.size() != 0)
			{
				ret += hr + "\r\n";
				ret += "信息：\r\n";
				ret += m_msg;
			}

			if (m_isFunction)
			{
				ret += hr + "函数结束\r\n";
			}
			return ret;
		}
		long GetExecCount()const { return m_execcount; }
	};
	///////////////////////////////////////////////////////////////////////
	//ICTScript
	bool ICTScript::AttachScript(CCTScript* p)
	{
		m_p = p;
		return NULL != m_p;
	}
	bool ICTScript::Compile(char const* _source, CMyVector<pair<string, Variable > >* pEnvs)
	{
		return m_p->Compile(_source, pEnvs);
	}
	string ICTScript::GetMessage()const
	{
		return m_p->GetMessage() + m_msg;
	}
	string const& ICTScript::GetSource()const
	{
		return m_p->GetSource();
	}
	bool ICTScript::IsCompiled()const
	{
		return m_p->IsCompiled();
	}
	string& ICTScript::Report(string& ret)const
	{
		return m_p->Report(ret);
	}
	bool ICTScript::Execute(Variable& ret, CMyVector<pair<string, Variable > >* pEnvs, void* pe)
	{
		return m_p->Execute(m_msg, ret, pEnvs, pe);
	}
	long ICTScript::GetExecCount()const
	{
		return m_p->GetExecCount();
	}
}
