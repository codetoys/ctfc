//htmldoc.h HTML功能类
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
#include <ctype.h>
#include "config.h"
#include "ColumnData.h"
#include "IDataSource.h"

/////////////////////////////////////////////////////////////
//HTML相关功能
namespace ns_my_std
{
	class CHTMLEncode
	{
	public:
		string operator ()(string const & str, bool forPRE = true)const
		{
			return Encode(str, forPRE);
		}
		//默认标准处理,不转换空格和回车换行
		static string Encode(string const& str, bool forPRE = true)
		{
			string ret = "";
			ret.reserve(str.size() * 2);
			for (string::size_type i = 0; i < str.size(); ++i)
			{
				if ('\"' == str[i])ret += "&quot;";
				else if ('&' == str[i])ret += "&amp;";
				else if ('<' == str[i])ret += "&lt;";
				else if ('>' == str[i])ret += "&gt;";
				else if (!forPRE && ' ' == str[i])ret += "&ensp;";
				else if (!forPRE && '\n' == str[i])ret += "<BR>\r\n";
				else ret += str[i];
			}
			return ret;
		}
	};
	//编码为脚本
	class CScriptEncode
	{
	public:
		static string Encode(string const & str)
		{
			string ret = "";
			ret.reserve(str.size() * 2);
			for (string::size_type i = 0; i < str.size(); ++i)
			{
				if ('\"' == str[i])ret += "\\\"";
				else if ('\n' == str[i])ret += "\\n";
				else if ('\r' == str[i])ret += "\\r";
				else ret += str[i];
			}
			return ret;
		}
	};
	//命令接口
	class CWebCommandParam
	{
	public:
		string id;//唯一标识
		string name;//显示名称
		string note;//说明

		bool isCheckBox;//是否是选择框
		bool isChecked;//是否选中

		string defaultvalue;//默认值
		bool isNotNull;//是否必填
		bool isSelectOnly;//是否只能在选项表选择
		bool isHide;//是否隐藏
		bool isPassword;//是否是密码输入
		bool isReadOnly;//是否只读
		long size;//宽度
		long rows;//行数,改值不为0则显示为textaera
		vector<pair<string, string > > optionvalue;//选项集合

		CWebCommandParam() { clear(); }
		void clear()
		{
			isCheckBox = isChecked = false;
			isNotNull = isSelectOnly = isHide = isPassword = isReadOnly = false;
			id = name = note = defaultvalue = "";
			size = rows = 0;
			optionvalue.clear();
		}
		void SetFormatInput(char const * _id, char const * _name, long _size, long _rows = 0, char const * _default = "", char const * _note = "")
		{
			clear();
			id = _id;
			name = _name;
			size = _size;
			rows = _rows;
			defaultvalue = _default;
			note = _note;
		}
		void SetFormatPasswd(char const * _id, char const * _name, long _size, char const * _default = "", char const * _note = "")
		{
			clear();
			isPassword = true;
			id = _id;
			name = _name;
			size = _size;
			defaultvalue = _default;
			note = _note;
		}
		void ReadOnly() { isReadOnly = true; }
		string toHtmlInputOnly(char const* value, char const* desc, bool canEdit)
		{
			CWebCommandParam tmp = *this;
			tmp.isReadOnly = !canEdit;
			return tmp._toHtmlInput(false, false, value, desc, false);
		}
		string toHtmlInput(bool smallinput = false, bool showParamId = false, string NewDefault = "")
		{
			return _toHtmlInput(smallinput, showParamId, NewDefault);
		}
		string _toHtmlInput(bool smallinput = false, bool showParamId = false, string NewDefault = "", char const* desc = NULL, bool showName = true)
		{
			char buf[10240];
			string type;
			string ret;
			CHTMLEncode encode;

			char namestr[256];
			if (showName)sprintf(namestr, "%s&nbsp;%s%s", encode(name).c_str(), encode(note).c_str(), (isNotNull ? "*" : ""));
			else strcpy(namestr, "");

			//从请求更新参数的值
			if (0 != NewDefault.size())
			{
				if (isCheckBox && "1" == NewDefault)isChecked = true;
				else defaultvalue = NewDefault;
			}

			if (isHide)
			{
				sprintf(buf, "<INPUT NAME=\"%s\" TYPE=\"HIDDEN\" VALUE=\"%s\">"
					, encode(id).c_str(), encode(defaultvalue).c_str());
				ret += buf;
				return ret;
			}

			if (showParamId)
			{
				ret = id + "&nbsp;:&nbsp;";
			}
			if (this->isCheckBox)
			{
				sprintf(buf, "<input type=\"checkbox\" %s name=\"%s\" value=\"1\"><CODE><NOBR>%s</NOBR></CODE>"
					, (this->isChecked ? "checked" : ""), this->id.c_str(), this->name.c_str());
				ret += buf;
			}
			else
			{
				if (isReadOnly && !isPassword)
				{
					string _desc;
					if (NULL == desc || 0 == strlen(desc))_desc = defaultvalue;
					else _desc = desc;
					sprintf(buf, "<CODE>%s</CODE><INPUT NAME=\"%s\" TYPE=\"HIDDEN\" VALUE=\"%s\"><CODE>%s</CODE>"
						, namestr, encode(id).c_str(), encode(defaultvalue).c_str(), encode(_desc).c_str());
					ret += buf;
				}
				else
				{
					if (rows != 0)
					{
						sprintf(buf, "<LABEL><CODE>%s</CODE><textarea %s warp=\"soft\" cols=\"%ld\" rows=\"%ld\" NAME=\"%s\" >%s</textarea></LABEL>"
							, namestr, (isReadOnly ? "READONLY" : ""), (smallinput ? 16 : size), (smallinput ? 1 : rows), encode(id).c_str(), encode(defaultvalue).c_str());
						ret += buf;
					}
					else
					{
						if (optionvalue.size() != 0)
						{
							sprintf(buf, "<LABEL><CODE>%s</CODE><select %s NAME=\"%s\" >"
								, namestr, (isReadOnly ? "READONLY" : ""), encode(id).c_str());
							ret += buf;
							for (string::size_type i = 0; i < optionvalue.size(); ++i)
							{
								sprintf(buf, "\n<option value=\"%s\" %s>%s%s%s%s</option>", encode(optionvalue[i].first).c_str(), (optionvalue[i].first == defaultvalue ? "selected" : ""), encode(optionvalue[i].second).c_str()
									, (showParamId ? "[" : ""), (showParamId ? optionvalue[i].first.c_str() : ""), (showParamId ? "]" : ""));
								ret += buf;
							}
							sprintf(buf, "</select></LABEL>");
							ret += buf;
						}
						else
						{
							if (this->isPassword)type = "PASSWORD";
							else type = "INPUT";

							sprintf(buf, "<LABEL><CODE>%s<INPUT %s TYPE=\"%s\" SIZE=\"%ld\" NAME=\"%s\" value=\"%s\"></INPUT></CODE></LABEL>"
								, namestr, (isReadOnly ? "READONLY" : ""), type.c_str(), (smallinput ? 16 : size), encode(id).c_str(), encode(defaultvalue).c_str());
							ret += buf;
						}
					}
				}
			}

			return ret;
		}
	};
	class CHtmlFormSubmit
	{
	public:
		string value;
		string prompt;
		string action;
	};
	class CHtmlDoc
	{
	public:
		//数据类型
		enum DATACLASS
		{
			CHtmlDoc_DATACLASS_TEXT = 0,	//文本
			CHtmlDoc_DATACLASS_HTML,	//HTML代码
			CHtmlDoc_DATACLASS_PRE,		//HTML代码，输出时附加<PRE>
			CHtmlDoc_DATACLASS_LEFT,	//文本，左对齐,不可换行
			CHtmlDoc_DATACLASS_RIGHT,	//文本，右对齐,不可换行
			CHtmlDoc_DATACLASS_DATE,	//日期，右对齐,不可换行
			CHtmlDoc_DATACLASS_TIME,	//时间，右对齐,不可换行
			CHtmlDoc_DATACLASS_TIME_LOG	//时间，日志格式
		};
		//16进制字符转换为数值
		static long hexchartolong(char c)
		{
			if (c >= '0' && c <= '9')return c - '0';
			if (c >= 'a' && c <= 'f')return c - 'a' + 10;
			if (c >= 'A' && c <= 'F')return c - 'A' + 10;
			return 0;
		}
		static string XMLEncode(string const & origindata)
		{
			string data;
			for (string::size_type tmp = 0; tmp < origindata.size(); ++tmp)
			{
				if ('<' == origindata[tmp])data += "&lt;";
				else if ('>' == origindata[tmp])data += "&gt;";
				else if ('&' == origindata[tmp])data += "&amp;";
				else if ('\'' == origindata[tmp])data += "&apos;";
				else if ('\"' == origindata[tmp])data += "&quot;";
				else if (' ' == origindata[tmp])data += "&nbsp;";
				else data += origindata[tmp];
			}
			return data;
		}
		static string XMLDecode(string const & xmldata)
		{
			string data;
			string::size_type tmp = 0;
			while (tmp < xmldata.size())
			{
				if (tmp == xmldata.find("&lt;", tmp))
				{
					data += '<';
					tmp += 4;
				}
				else if (tmp == xmldata.find("&gt;", tmp))
				{
					data += '>';
					tmp += 4;
				}
				else if (tmp == xmldata.find("&amp;", tmp))
				{
					data += '&';
					tmp += 5;
				}
				else if (tmp == xmldata.find("&apos;", tmp))
				{
					data += '\'';
					tmp += 6;
				}
				else if (tmp == xmldata.find("&quot;", tmp))
				{
					data += '\"';
					tmp += 6;
				}
				else if (tmp == xmldata.find("&nbsp;", tmp))
				{
					data += ' ';
					tmp += 6;
				}
				else
				{
					data += xmldata[tmp];
					++tmp;
				}
			}
			return data;
		}
		//strong:强编码，否则弱编码
		static string URLEncode(string const & str, bool strong = true)
		{
			string ret = "";
			string::size_type i = 0;
			char buf[256];
			unsigned char c;
			while (i < str.size())
			{
				c = (unsigned char)str[i];
				if (
					(strong && isalnum(c))
					|| (!strong && (c >= 32 && c != '&' && c != '=' && c != '%' && c != '+'))
					)
				{
					ret += c;
				}
				else
				{
					sprintf(buf, "%%%02X", c);
					ret += buf;
				}
				++i;
			}
			return ret;
		}
		static string URLDecode(string const & str)
		{
			string ret = "";
			string::size_type i = 0;
			while (i < str.size())
			{
				if ('+' == str[i])
				{
					ret += ' ';
					++i;
					continue;
				}
				else if ('%' != str[i])
				{
					ret += str[i];
					++i;
					continue;
				}
				else
				{
					if (i + 2 < str.size())
					{
						char c = '\0';
						c += (char)hexchartolong(str[i + 1]) * 16;
						c += (char)hexchartolong(str[i + 2]);
						ret += c;
						i += 3;
						continue;
					}
					else
					{
						break;
					}
				}
			}
			return ret;
		}
		//为文本输出转换格式,默认附加对空格和回车换行的处理
		static string HTMLEncode(string const & str, bool forPRE = false)
		{
			CHTMLEncode encode;
			return encode(str, forPRE);
		}
		struct _thtd
		{
			bool hidden;
			bool nowrap;
			string bgcolor;
			string align;

			bool isFormInput;//true则使用forminput，false则用其余的
			CWebCommandParam forminput;
			DATACLASS dataclass;//data格式
			long _dbtype;//数据在数据库的数据类型
			string _dbshowtype;//数据在数据库的show_type
			string _dbcomment;//数据在数据库的注释
			string value;

			_thtd()
			{
				hidden = false;
				nowrap = false;
				isFormInput = false;
				dataclass = CHtmlDoc_DATACLASS_HTML;
			}
			//根据dbshowtype显示
			string toShow(string const & data)const
			{
				if (CHtmlDoc_DATACLASS_TIME_LOG == dataclass || 0 == stricmp(_dbshowtype.c_str(), "TIME_LOG"))
				{
					time_t t1 = atol(data.c_str());
					return CMyTools::TimeToString_log(t1);
				}
				else if (CHtmlDoc_DATACLASS_DATE == dataclass || 0 == stricmp(_dbshowtype.c_str(), "DATE"))
				{
					time_t t1 = atol(data.c_str());
					return CMyTools::TimeToString_Date(t1);
				}
				else if (CHtmlDoc_DATACLASS_TIME == dataclass || 0 == stricmp(_dbshowtype.c_str(), "TIME"))
				{
					time_t t1 = atol(data.c_str());
					return CMyTools::TimeToString_Time(t1);
				}
				else
				{
					return data;
				}
			}
			string& toHtml(string const& data, char const* desc, string& id, string& str, bool isTH)
			{
				return toHtml2(data, desc, id, str, isTH);
			}
			string& toHtml2(string const & data, char const* desc, string& id, string& str, bool isTH, bool canEdit = true)
			{
				str = "";
				STATIC_C char const * th = "TH";
				STATIC_C char const * td = "TD";
				string thtd = (isTH ? th : td);
				DATACLASS tmpdataclass;
				if (isTH)tmpdataclass = CHtmlDoc_DATACLASS_TEXT;
				else tmpdataclass = dataclass;

				string prop = "";
				if (id.size() != 0)
				{
					prop += " id=\"" + id + "\"";
				}
				if (this->bgcolor.size() != 0)
				{
					prop += " bgcolor=\"";
					prop += bgcolor;
					prop += "\" ";
				}
				if (this->align.size() != 0)
				{
					prop += " align=\"";
					prop += align;
					prop += "\" ";
				}
				if (CHtmlDoc_DATACLASS_LEFT == tmpdataclass)
				{
					prop += " align=\"left\" ";
				}
				if (CHtmlDoc_DATACLASS_RIGHT == tmpdataclass)
				{
					prop += " align=\"right\" ";
				}
				if (this->nowrap)
				{
					prop += " nowrap ";
				}
				if (hidden)
				{
					prop += " style=\"display:none\" ";
				}
				if (!isTH && this->isFormInput)
				{
					str += "<" + thtd + " " + prop + ">" + forminput.toHtmlInputOnly(data.c_str(), desc, false) + "</" + thtd + ">\n";
				}
				else
				{
					if (data.size() == 0)
					{
						str += "<" + thtd + prop + ">&nbsp</" + thtd + ">\n";
					}
					else
					{
						string outputdata = (isTH ? data : toShow(data));//实际输出，可能经过了变换
						if (0 == outputdata.size())outputdata = " ";
						switch (tmpdataclass)
						{
						case CHtmlDoc_DATACLASS_TEXT:
							str += "<" + thtd + prop + "><CODE>" + HTMLEncode(outputdata) + "</CODE></" + thtd + ">\n";
							break;
						case CHtmlDoc_DATACLASS_PRE:
							str += "<" + thtd + prop + "><PRE>" + HTMLEncode(outputdata, true) + "</PRE></" + thtd + ">\n";
							break;
						case CHtmlDoc_DATACLASS_LEFT:
							str += "<" + thtd + prop + " nowrap ><CODE>" + HTMLEncode(outputdata, false) + "</CODE></" + thtd + ">\n";
							break;
						case CHtmlDoc_DATACLASS_RIGHT://直接下落
						case CHtmlDoc_DATACLASS_DATE://直接下落
						case CHtmlDoc_DATACLASS_TIME_LOG://直接下落
						case CHtmlDoc_DATACLASS_TIME:
							str += "<" + thtd + prop + " nowrap ><CODE>" + HTMLEncode(outputdata, false) + "</CODE></" + thtd + ">\n";
							break;
						default:
							str += "<" + thtd + prop + ">" + outputdata + "</" + thtd + ">\n";
							break;
						}
					}
				}
				return str;
			}
		};
		struct _tr
		{
			vector<_thtd > cells;
		};
		struct _cell
		{
			string value;
			string desc;//描述，作为form单元的时候显示desc，但提交的仍然是value
		};
		struct _record
		{
			string record_onclick;//HTML onclick事件处理程序
			string record_ondblclick;//HTML ondblclick事件处理程序
			string expand_html;//展开行，html格式，通过+号打开
			vector<_cell > cells;
		};
		class CHtmlTable2 :public CIDataSource_random
		{
		private:
			string m_dbcomment;//数据库的表注释
			string m_title;
			string m_id;
			_tr m_heads;
			_tr m_foots;
			vector<_record > m_bodys;

			bool m_isForm;
			bool m_canAdd;
			bool m_canDelete;
			bool m_canEdit;//是否允许对现有数据进行编辑
			string m_action;
			vector<CHtmlFormSubmit > m_FormSubmits;
			bool WithExpand;//添加展开行

			//用于CIDataSource_random
			size_t m_current_line;

			vector<_tr >::size_type _GetColCount(vector<_record > const & trs)
			{
				vector<_record >::const_iterator it;
				vector<_tr >::size_type maxcount = 0;
				for (it = trs.begin(); it != trs.end(); ++it)
				{
					if (it->cells.size() > maxcount)maxcount = it->cells.size();
				}
				return maxcount;
			}
			size_t GetActualColCount()
			{
				//这个值是定义的列的数目和每个行的列数目之最大者
				vector<_thtd >::size_type maxcount = 0;
				if (m_heads.cells.size() > maxcount)maxcount = m_heads.cells.size();
				if (m_foots.cells.size() > maxcount)maxcount = m_foots.cells.size();
				if (_GetColCount(m_bodys) > maxcount)maxcount = _GetColCount(m_bodys);
				return maxcount;
			}
			string _MakeID(char const * DataOrHead, long line, long col)const
			{
				char buf[256];
				sprintf(buf, "_%s_%ld_%ld", DataOrHead, line, col);
				return m_id + buf;
			}
			string MakeHeadID(long line, long col)const { return _MakeID("head", line, col); }
			string MakeBodyID(long line, long col)const { return _MakeID("data", line, col); }
			string MakeFootID(long line, long col)const { return _MakeID("foot", line, col); }
			string UpdateScript(string const & id, string const & data)
			{
				return "<script language=\"JavaScript\">window." + id + ".innerHTML=\"" + CScriptEncode::Encode(data) + "\";</script>\r\n";
			}
		public:
			CHtmlTable2()
			{
				Clear();
			}
			//清空列配置和数据
			bool Clear()
			{
				m_dbcomment = "";
				m_title = "";
				m_id = "";
				m_isForm = false;
				m_canAdd = false;
				m_canDelete = false;
				m_canEdit = false;
				m_heads.cells.clear();
				m_foots.cells.clear();
				m_bodys.clear();
				m_FormSubmits.clear();
				WithExpand = false;

				m_current_line = -1;

				return true;
			}
			//清空Body数据，保留其它配置
			bool ClearBody()
			{
				m_bodys.clear();

				return true;
			}
			//用于排序的比较对象
			class TR_Less
			{
			public:
				struct less_rule
				{
					long col_index;
					bool asc;
					COLUMN_TYPE col_type;;
					less_rule() :col_index(0), asc(true), col_type(COLUMN_TYPE_STRING_POOL) {}
					bool less(_record const& a, _record const& b)const
					{
						if (asc)
						{
							if (COLUMN_TYPE_LONG == col_type)return atol(a.cells[col_index].value.c_str()) < atol(b.cells[col_index].value.c_str());
							if (COLUMN_TYPE_DOUBLE == col_type)return atof(a.cells[col_index].value.c_str()) < atof(b.cells[col_index].value.c_str());
							else return a.cells[col_index].value < b.cells[col_index].value;
						}
						else
						{
							if (COLUMN_TYPE_LONG == col_type)return atol(a.cells[col_index].value.c_str()) > atol(b.cells[col_index].value.c_str());
							if (COLUMN_TYPE_DOUBLE == col_type)return atof(a.cells[col_index].value.c_str()) > atof(b.cells[col_index].value.c_str());
							else return a.cells[col_index].value > b.cells[col_index].value;
						}
					}
				};

				vector<less_rule > m_less_rule_s;
				bool operator()(_record const& a, _record const& b)const
				{
					for (vector<less_rule >::const_iterator it = m_less_rule_s.begin(); it != m_less_rule_s.end(); ++it)
					{
						if (it->less(b, a))return false;
						if (it->less(a, b))return true;
					}
					return false;
				}
			};

			//排序
			bool Sort(long col, bool ASC, COLUMN_TYPE type)
			{
				TR_Less lesses;
				TR_Less::less_rule less;
				less.col_index = col;
				less.asc = ASC;
				less.col_type = type;
				lesses.m_less_rule_s.push_back(less);
				sort(m_bodys.begin(), m_bodys.end(), lesses);
				return true;
			}
			bool Sort(TR_Less const & lesses)
			{
				sort(m_bodys.begin(), m_bodys.end(), lesses);
				return true;
			}

			//返回列个数，不包括数据中设置的超出范围的
			size_t GetColCount()const { return m_heads.cells.size(); }
			string GetColName(long i)const { return m_heads.cells[i].value; }
			long GetColIndex(char const * col_name)const
			{
				size_t j;
				for (j = 0; j < m_heads.cells.size(); ++j)
				{
					if (GetColName((long)j) == col_name)return (long)j;
				}
				return -1;
			}
			virtual long IDS_GetColCount()const { return (long)GetColCount(); }
			virtual long IDS_GetColIndex(char const* col_name)const 
			{
				return GetColIndex(col_name);
			}
			virtual string IDS_GetColName(long i)const { return GetColName(i); }
			virtual bool IDS_GetColDbInfo(long col_index, int& dataclass, long& dbtype, string& dbshowtype, string& dbcomment)const
			{
				dataclass = m_heads.cells[col_index].dataclass;
				dbtype = m_heads.cells[col_index]._dbtype;
				dbshowtype = m_heads.cells[col_index]._dbshowtype;
				dbcomment = m_heads.cells[col_index]._dbcomment;
				//thelog << "m_heads.cells[col_index]._dbtype=" << m_heads.cells[col_index]._dbtype << endi;
				return true;
			}
			
			virtual bool IDS_MoveFirst() { m_current_line = 0; return !IDS_isEnd(); }
			virtual bool IDS_MoveNext() { ++m_current_line; return !IDS_isEnd(); }
			virtual bool IDS_Move(long id) { m_current_line = id; return !IDS_isEnd(); }
			virtual bool IDS_isEnd()const { return m_current_line < 0 || m_current_line >= m_bodys.size(); }
			virtual long IDS_GetCurrentID()const {return (long)m_current_line;}

			virtual string IDS_GetCurrentData(long col)const { return IDS_GetDataByID((long)m_current_line, col); }
			//获取行onclick（HTML）
			virtual string IDS_GetCurrent_OnClick()const{return IDS_GetOnClickByID((long)m_current_line);}
			virtual string IDS_GetCurrentDataToShow(long col)const { return IDS_GetDataToShowByID((long)m_current_line,col); }
			virtual string IDS_OutputCurrentRecord()const { return OutputRecord(m_current_line); }
			virtual string IDS_GetDataByID(long id, long col)const { return GetData(id, col); }
			virtual string IDS_GetOnClickByID(long _id)const 
			{
				STATIC_G string null_str;
				if (_id < 0)return null_str;
				size_t id = _id;
				if (id >= m_bodys.size())return null_str;
				return m_bodys[id].record_onclick;
			}
			virtual string IDS_GetDataToShowByID(long _id, long col)const 
			{
				STATIC_G string null_str;
				if (_id < 0)return null_str;
				size_t id = _id;
				if (id >= m_bodys.size())return null_str;
				return m_heads.cells[col].toShow(m_bodys[id].cells[col].value);
			}

			size_t GetRecordCount()const { return m_bodys.size(); }
			string GetData(size_t line, long col)const
			{ 
				STATIC_G string null_str;
				if (line >= m_bodys.size())return null_str;
				return m_bodys[line].cells[col].value;
			}
			string OutputRecord(size_t line)const
			{
				stringstream ss;

				for (size_t i = 0; i < m_bodys[line].cells.size(); ++i)
				{
					ss << "[" << this->GetColName((long)i) << "](" << m_bodys[line].cells[i].value << ") ";
				}

				return ss.str();
			}
			void PushData(CHtmlTable2 const& datas)
			{
				for (size_t i = 0; i < datas.m_bodys.size(); ++i)m_bodys.push_back(datas.m_bodys[i]);
			}

			long GetColDbType(long i)const { return m_heads.cells[i]._dbtype; }
			char const * GetColDbShowType(long i)const { return m_heads.cells[i]._dbshowtype.c_str(); }
			string GetColDbComment(long i)const { return m_heads.cells[i]._dbcomment; }
			string GetData(size_t i, char const * col_name)const
			{
				size_t j;
				for (j = 0; j < m_heads.cells.size(); ++j)
				{
					if (GetColName((long)j) == col_name)return m_bodys[i].cells[j].value;
				}
				return "";
			}
			//查找行，根据某一列数据，只能找到的第一个
			long findLineByData(size_t col, size_t startline, char const* data)
			{
				for (size_t i = startline; i < m_bodys.size(); ++i)
				{
					if (col < m_bodys[i].cells.size() && m_bodys[i].cells[col].value == data)
					{
						return (long)i;
					}
				}
				return -1;
			}
			//删除行
			bool DeleteRecord(size_t i)
			{
				if (i>m_bodys.size())return false;
				m_bodys.erase(m_bodys.begin() + i);
				return true;
			}
			//启用扩展行
			void EnableExpand() { WithExpand = true; }
			//启用form，默认为不是form，所有提交按钮都会称之为“_op”，因此op不能用于一般列数据
			void EnableForm(bool canAdd, bool canDelete, bool canEdit, char const* action)
			{
				m_isForm = true;
				m_canAdd = canAdd;
				m_canDelete = canDelete;
				m_canEdit = canEdit;
				if (action != NULL)m_action = action;
			}
			//添加Form操作，不需要包括delete和add，如果canAddDelete则这两个会自动添加
			void AddFormSubmit(string const & value, string const & prompt = "", string const & action = "")
			{
				m_FormSubmits.reserve(5);
				m_FormSubmits.resize(m_FormSubmits.size() + 1);
				m_FormSubmits[m_FormSubmits.size() - 1].value = value;
				m_FormSubmits[m_FormSubmits.size() - 1].prompt = prompt;
				m_FormSubmits[m_FormSubmits.size() - 1].action = action;
			}
			void SetDbComment(char const * comment) { if (NULL != comment)m_dbcomment = comment; }
			string const & GetDbComment()const { return m_dbcomment; }
			//设置标题
			void SetTitle(char const * title) { m_title = title; }
			//设置ID
			void SetID(char const * id) { m_id = id; }
			//添加列，返回列索引，基于0
			long AddCol_db(string const & str, DATACLASS dataclass, long dbtype, char const * dbcomment, char const * dbshowtype)
			{
				//thelog << str << " : " << dbshowtype << endi;
				return AddCol(str, dataclass, NULL, false,dbtype, dbcomment, dbshowtype);
			}
			//添加列，返回列索引，基于0
			long AddCol(string const & str, DATACLASS dataclass = CHtmlDoc_DATACLASS_TEXT, CWebCommandParam const * pCommandParam = NULL, bool hidden = false, long dbtype = -1, char const * dbcomment = NULL, char const * dbshowtype = NULL)
			{
				m_heads.cells.resize(m_heads.cells.size() + 1);
				_thtd & th = m_heads.cells[m_heads.cells.size() - 1];
				th.value = str;
				th.dataclass = dataclass;
				th.hidden = hidden;
				th._dbtype = dbtype;
				if (NULL != dbcomment)th._dbcomment = dbcomment;
				if (NULL != dbshowtype)th._dbshowtype = dbshowtype;
				if (NULL != pCommandParam)
				{
					th.isFormInput = true;
					th.forminput = *pCommandParam;
				}
				return (long)m_heads.cells.size() - 1;
			}
			//设置列，增加列名前缀
			void SetCol_AddPX(char const* px)
			{
				for (size_t i = 0; i < m_heads.cells.size(); ++i)
				{
					m_heads.cells[i].value = px + m_heads.cells[i].value;
				}
			}
			//设置列
			bool SetCol(size_t col, string const & str, DATACLASS dataclass, char const * align)
			{
				if (col>m_heads.cells.size())return false;
				_thtd & th = m_heads.cells[col];
				th.value = str;
				th.dataclass = dataclass;
				th.align = align;
				return true;
			}
			//设置列，不包括列名
			bool SetCol(size_t col, DATACLASS dataclass, char const * align)
			{
				if (col>m_heads.cells.size())return false;
				_thtd & th = m_heads.cells[col];
				th.dataclass = dataclass;
				th.align = align;
				return true;
			}
			//设置列的显示类型
			bool SetCol(char const * colname, DATACLASS dataclass)
			{
				size_t col = GetColIndex(colname);
				if (col<0 || col>m_heads.cells.size())return false;
				_thtd & th = m_heads.cells[col];
				th.dataclass = dataclass;
				return true;
			}
			//设置列为Form的Input，文本输入，宽度为size
			bool SetColFormInput(char const* colname, long size, bool _hidden)
			{
				long col = GetColIndex(colname);
				return SetColFormInput(col, size, _hidden);
			}
			bool SetColFormInput(size_t col, long size, bool _hidden)
			{
				if (col>m_heads.cells.size())return false;
				_thtd & th = m_heads.cells[col];
				th.isFormInput = true;
				th.forminput.SetFormatInput(GetColName((long)col).c_str(), GetColName((long)col).c_str(), size);
				th.hidden = _hidden;
				return true;
			}
			//添加foot
			void AddFoot(size_t i, string const & data)
			{
				if (i >= m_foots.cells.size())m_foots.cells.resize(i + 1);
				m_foots.cells[i].value = data;
			}
			//添加行，返回行索引，基于0
			long AddLine()
			{
				m_bodys.resize(m_bodys.size() + 1);
				return (long)m_bodys.size() - 1;
			}
			//设置行onclick（HTML）
			bool SetLine_OnClick(size_t line, char const* js)
			{
				if (line < 0 || line >= m_bodys.size())return false;
				m_bodys[line].record_onclick = js;
				return true;
			}
			//设置行ondblclick（HTML）
			bool SetLine_OnDblclick(size_t line, char const* js)
			{
				if (line < 0 || line >= m_bodys.size())return false;
				m_bodys[line].record_ondblclick = js;
				return true;
			}
			//设置行展开数据（HTML）
			bool SetLine_ExpandData(size_t line, char const* data)
			{
				if (line < 0 || line >= m_bodys.size())return false;
				m_bodys[line].expand_html = data;
				return true;
			}
			//添加数据，最后一行
			void AddData(string const& data, bool CountToFoot = false, long span = 1)
			{
				if (0 == m_bodys.size())AddLine();
				long line = (long)m_bodys.size() - 1;
				
				AddDataToLine(line, data, CountToFoot);
			}
			void AddData(long data, bool CountToFoot = false, long span = 1)
			{
				char buf[256];
				sprintf(buf, "%ld", data);
				AddData(buf, CountToFoot, span);
			}
			void AddData(double data, long decimal, bool CountToFoot = false, long span = 1)
			{
				char format[256];
				char buf[256];
				sprintf(format, "%%.%ldf", decimal);
				sprintf(buf, format, data);
				AddData(buf, CountToFoot, span);
			}
			//添加到指定行末尾
			bool AddDataToLine(size_t line,string const& data, bool CountToFoot = false)
			{
				if (line >= m_bodys.size())return false;
				size_t col = m_bodys[line].cells.size();
				if (col >= m_heads.cells.size())m_heads.cells.resize(col + 1);
				m_bodys[line].cells.resize(col + 1);
				m_bodys[line].cells[col].value = data;

				//统计到foot
				if (CountToFoot)
				{
					if (col >= m_foots.cells.size())m_foots.cells.resize(col + 1);
					long count = atol(m_foots.cells[col].value.c_str()) + atol(data.c_str());
					char buf[256];
					sprintf(buf, "%ld", count);
					m_foots.cells[col].value = buf;
				}

				return true;
			}
			bool AddDataToLine(long line,long data, bool CountToFoot = false)
			{
				char buf[256];
				sprintf(buf, "%ld", data);
				return AddDataToLine(line, buf, CountToFoot);
			}
			//设置数据，行必须有效，但可随意跳过一些列
			void SetData(size_t lineindex, size_t colindex, string const& data, char const* desc = NULL)
			{
				if (lineindex >= m_bodys.size())m_bodys.resize(lineindex + 1);
				if (colindex >= m_bodys[lineindex].cells.size())m_bodys[lineindex].cells.resize(colindex + 1);
				if (colindex >= m_heads.cells.size())m_heads.cells.resize(colindex + 1);
				m_bodys[lineindex].cells[colindex].value = data;
				if (desc)m_bodys[lineindex].cells[colindex].desc = desc;
			}
			void SetData(size_t lineindex, size_t colindex, long data, char const* desc = NULL)
			{
				char buf[256];
				sprintf(buf, "%ld", data);
				SetData(lineindex, colindex, buf, desc);
			}
			//填充缺失的格子，构造成完整表格
			void Fill()
			{
				long cols = (long)GetActualColCount();
				m_heads.cells.resize(cols);
				for (size_t i = 0; i < m_bodys.size(); ++i)
				{
					m_bodys[i].cells.resize(cols);
				}
				if (0 != m_foots.cells.size())m_foots.cells.resize(cols);
			}
			string MakeHtmlTable()
			{
				string ret;
				char buf[2048];
				size_t i, j;
				string tmpstr;
				string id;
				size_t actualcount = GetActualColCount();

				string expand_name;//展开数据的名字
				static long _expand_name = 0;
				sprintf(buf, "_expand_%ld", _expand_name++);
				expand_name = buf;

				sprintf(buf, "<table id=\"%s\" class=\"table1\" border=1 cols=\"%ld\" rules=\"all\">\n", m_id.c_str(), (m_isForm ? (long)actualcount + 1 : (long)actualcount));
				ret += buf;
				if (m_title.size() != 0)
				{
					ret += "<CAPTION>";
					ret += m_title;
					ret += "</CAPTION>";
				}
				ret += "<THEAD>\n";
				//head
				{
					ret += "<TR>";
					if (WithExpand)
					{
						ret += "<th>&nbsp;</th>\n";
					}
					for (i = 0; i < m_heads.cells.size(); ++i)
					{
						id = MakeHeadID(0, (long)i);
						ret += m_heads.cells[i].toHtml(m_heads.cells[i].value, NULL, id, tmpstr, true);
					}
					if (this->m_isForm)
					{
						ret += "<TH>操作</TH>\n";
					}
					ret += "</TR>\n";
				}
				ret += "</THEAD>\n";
				ret += "<TBODY>\n";
				for (j = 0; j < m_bodys.size(); ++j)
				{
					if (0 != m_bodys[j].record_onclick.size())ret += "<TR onclick=\"" + m_bodys[j].record_onclick + "\">";
					if (0 != m_bodys[j].record_ondblclick.size())ret += "<TR ondblclick=\"" + m_bodys[j].record_ondblclick + "\">";
					else ret += "<TR>\n";

					if (WithExpand)
					{
						ret += "<td onclick=\"var obj=document.getElementsByName('" + expand_name + "'); obj.forEach(function(e){e.hidden=!e.hidden;});\">+</td>\n";
					}

					if (this->m_isForm)
					{
						if (0 != m_action.size())ret += "<FORM target=\"_blank\" method=\"POST\" action=\"" + m_action + "\">\n";
						else ret += "<FORM target=\"_blank\" method=\"POST\">\n";
					}

					for (i = 0; i < m_bodys[j].cells.size(); ++i)
					{
						id = MakeBodyID((long)j, (long)i);
						if (i >= m_heads.cells.size())
						{
							_thtd tmp;
							ret += tmp.toHtml(m_bodys[j].cells[i].value, m_bodys[j].cells[i].desc.c_str(), id, tmpstr, false);
						}
						else
						{

							ret += m_heads.cells[i].toHtml2(m_bodys[j].cells[i].value, m_bodys[j].cells[i].desc.c_str(), id, tmpstr, false, this->m_canEdit);
						}
					}

					if (this->m_isForm)
					{
						ret += "<TD>";
						char buf2[10240];
						vector<CHtmlFormSubmit >::const_iterator it;
						for (it = m_FormSubmits.begin(); it != m_FormSubmits.end(); ++it)
						{
							char onclick[2048];
							if (it->prompt.size() != 0)
							{
								sprintf(onclick, " onclick=\"return confirm('%s')\" ", it->prompt.c_str());
							}
							char formaction[2048];
							if (it->action.size() != 0)
							{
								sprintf(formaction, " formaction=\"%s\" ", it->action.c_str());
							}
							sprintf(buf2, "<button type=\"submit\" name=\"_op\" value=\"%s\" %s %s>%s</button>"
								, it->value.c_str(), onclick, formaction, it->value.c_str());
							ret += buf2;
						}
						if (this->m_canEdit)ret += "<input type=\"submit\" name=\"_op\" value=\"编辑\">";
						if (this->m_canDelete)ret += "<input type=\"submit\" name=\"_op\" value=\"删除\" onclick=\"return confirm('删除数据，确定吗？')\">";
						ret += "</TD></FORM>\n";
					}

					ret += "</TR>\n";

					if (WithExpand)
					{
						char buf[256];
						sprintf(buf, " colspan=\"%ld\" ", (long)actualcount);
						ret += string("<TR hidden=\"hidden\" name=\"" + expand_name + "\"><td></td><td ") + buf + ">" + m_bodys[j].expand_html + "</td></TR>\n";
					}
				}
				ret += "<TFOOT>\n";
				if (0 != m_foots.cells.size())
				{
					ret += "<TR>";
					if (m_foots.cells.size() < actualcount)m_foots.cells.resize(actualcount);
					if (WithExpand)
					{
						ret += "<th></th>\n";
					}
					for (i = 0; i < m_foots.cells.size(); ++i)
					{
						id = MakeFootID(0, (long)i);
						if (i >= m_heads.cells.size())
						{
							ret += m_foots.cells[i].toHtml(m_foots.cells[i].value, NULL, id, tmpstr, true);
						}
						else
						{
							ret += m_heads.cells[i].toHtml(m_foots.cells[i].value, NULL, id, tmpstr, true);
						}
					}
					if (this->m_isForm)
					{
						ret += "<TH></TH>\n";
					}
					ret += "</TR>\n";
				}
				if (this->m_isForm && this->m_canAdd)
				{
					ret += "<TR>";
					
					if (WithExpand)
					{
						ret += "<TD>&nbsp;</TD>\n";
					}
					
					if (0 != m_action.size())ret += "<FORM target=\"_blank\" method=\"POST\" action=\"" + m_action + "\">\n";
					else ret += "<FORM target=\"_blank\" method=\"POST\">\n";
					for (i = 0; i < m_heads.cells.size(); ++i)
					{
						id = MakeFootID((long)m_bodys.size(), (long)i);
						m_heads.cells[i].forminput.isReadOnly = false;
						if (m_heads.cells[i].isFormInput)ret += m_heads.cells[i].toHtml("", NULL, id, tmpstr, false);
						else ret += "<TD></TD>\n";
					}
					ret += "<TD><input type=\"submit\" name=\"_op\" value=\"添加\">";
					ret += "</TD></FORM>\n";
					
					ret += "</TR>\n";
				}
				ret += "</TFOOT>\n";
				ret += "</TBODY>\n";
				ret += "</table>\n";
				return ret;
			}
			string MakeUpdateScript(CHtmlTable2 * oldtable = NULL)
			{
				string ret;
				size_t i, j;
				string tmpstr;
				string id;

				//head
				{
					for (i = 0; i < m_heads.cells.size(); ++i)
					{
						if (NULL != oldtable && i < oldtable->m_heads.cells.size())
						{
							if (m_heads.cells[i].value == oldtable->m_heads.cells[i].value)continue;//相同，不用生成脚本
						}
						id = MakeHeadID(0, (long)i);
						ret += UpdateScript(id, m_heads.cells[i].toHtml(m_heads.cells[i].value, NULL, id, tmpstr, true));
					}
				}
				//foot
				{
					for (i = 0; i < m_foots.cells.size(); ++i)
					{
						if (NULL != oldtable && i < oldtable->m_foots.cells.size())
						{
							if (m_foots.cells[i].value == oldtable->m_foots.cells[i].value)continue;//相同，不用生成脚本
						}
						id = MakeFootID(0, (long)i);
						ret += UpdateScript(id, m_foots.cells[i].toHtml(m_foots.cells[i].value, NULL, id, tmpstr, true));
					}
				}
				for (j = 0; j < m_bodys.size(); ++j)
				{
					for (i = 0; i < m_bodys[j].cells.size(); ++i)
					{
						if (NULL != oldtable && j < oldtable->m_bodys.size() && i < oldtable->m_bodys[j].cells.size())
						{
							if (m_bodys[j].cells[i].value == oldtable->m_bodys[j].cells[i].value)continue;//相同，不用生成脚本
						}
						id = MakeBodyID((long)j, (long)i);
						if (i >= m_heads.cells.size())
						{
							_thtd tmp;
							ret += UpdateScript(id, tmp.toHtml(m_bodys[j].cells[i].value, NULL, id, tmpstr, false));
						}
						else
						{
							ret += UpdateScript(id, m_heads.cells[i].toHtml(m_bodys[j].cells[i].value, NULL, id, tmpstr, false));
						}
					}
				}
				return ret;
			}
			//生成XML输出头，标准XML文件格式(多表格输出)
			static string & StartXML(char const * _root, string & ret)
			{
				string root = _root;
				ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
				ret += "<" + root + ">\r\n";
				return ret;
			}
			//生成XML节，标准XML文件格式(多表格输出)
			string & MakeXMLSection(char const * _section, string & ret)
			{
				string section = _section;
				size_t i, j;
				ret += "<" + section + ">\r\n";
				ret += "<__HEADS__>\r\n";
				for (i = 0; i < m_heads.cells.size(); ++i)
				{
					ret += "\t<__head__>\r\n";
					ret += "\t\t<_dbtype>" + CMyTools::ToString(m_heads.cells[i]._dbtype) + "</_dbtype>\r\n";
					ret += "\t\t<_dbcomment>" + XMLEncode(m_heads.cells[i]._dbcomment) + "</_dbcomment>\r\n";
					ret += "\t\t<_dbshowtype>" + m_heads.cells[i]._dbshowtype + "</_dbshowtype>\r\n";
					ret += "\t</__head__>\r\n";
				}
				ret += "</__HEADS__>\r\n";
				ret += "<__RECORDS__>\r\n";
				for (i = 0; i < m_bodys.size(); ++i)
				{
					ret += "\t<__record__>\r\n";
					for (j = 0; j < m_heads.cells.size(); ++j)
					{
						if (j < m_bodys[i].cells.size())
						{
							string origindata = m_bodys[i].cells[j].value;
							string data = XMLEncode(origindata);
							string lebel = XMLEncode(m_heads.cells[j].value);
							ret += "\t\t<" + lebel + ">" + data + "</" + lebel + ">\r\n";
						}
					}
					ret += "\t</__record__>\r\n";
				}
				ret += "</__RECORDS__>\r\n";
				ret += "</" + section + ">\r\n";
				return ret;
			}
			//生成XML输出尾，标准XML文件格式(多表格输出)
			static string & FinishXML(char const * _root, string & ret)
			{
				string root = _root;
				ret += "</" + root + ">\r\n";
				return ret;
			}
			//生成XML输出，标准XML文件格式(单一表格输出)
			string & MakeXML(char const * _root, string & ret)
			{
				string root = _root;
				ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
				return MakeXMLSection(_root, ret);
			}
			//计算输出长度
			size_t outputlen(string const & s)
			{
				//cout << s << " " << s.size() << " " << strlen(s.c_str()) << " outputlen " << utf8outputlen(s.c_str()) << endl;
				return utf8outputlen(s.c_str());
			}
			//utf-8中文通常为3个字节，输出仍为两个字符宽度
			size_t utf8outputlen(char const * s)
			{
				signed char const * p = (signed char const* )s;
				long count_ansi = 0;
				long count_other = 0;
				while (*p)
				{
					if (*p < 0)++count_other;
					else ++count_ansi;

					++p;
				}
				return count_ansi + count_other * 2 / 3;
			}
			//head_tail_n==0显示全部，否则只显示前后head_tail_n
			string MakeTextTable(size_t head_tail_n = 0)
			{
				string ret;
				size_t i, j;
				size_t actualcount = this->GetActualColCount();
				vector<size_t > v_colmaxlen;//列的最大宽度
				v_colmaxlen.reserve(actualcount);
				for (i = 0; i < actualcount; ++i)
				{
					v_colmaxlen.push_back(0);
					if (i < m_heads.cells.size())
					{
						if (outputlen(m_heads.cells[i].value) > v_colmaxlen[i])v_colmaxlen[i] = outputlen(m_heads.cells[i].value);
					}
					if (i < m_foots.cells.size())
					{
						if (outputlen(m_foots.cells[i].value) > v_colmaxlen[i])v_colmaxlen[i] = outputlen(m_foots.cells[i].value);
					}
					for (j = 0; j < m_bodys.size(); ++j)
					{
						if (i < m_bodys[j].cells.size())
						{
								string data = m_heads.cells[i].toShow(m_bodys[j].cells[i].value);
								if (outputlen(data) > v_colmaxlen[i])v_colmaxlen[i] = outputlen(data);
						}
					}
				}

				ret = "";
				string str;

				ret += m_title;
				ret += "\n";
				for (i = 0; i < actualcount; ++i)
				{
					if (i < m_heads.cells.size())
					{
						str.assign(v_colmaxlen[i] - outputlen(m_heads.cells[i].value), ' ');
						if (CHtmlDoc_DATACLASS_RIGHT == m_heads.cells[i].dataclass)
						{
							ret += str;
							ret += m_heads.cells[i].value;
						}
						else
						{
							ret += m_heads.cells[i].value;
							ret += str;
						}
					}
					ret += " ";
				}
				ret += "\n";
				for (i = 0; i < actualcount; ++i)
				{
					str.assign(v_colmaxlen[i], '-');
					ret += str;
					ret += " ";
				}
				ret += "\n";
				bool first_skip = true;
				for (j = 0; j < m_bodys.size(); ++j)
				{
					if (head_tail_n > 0 && j >= head_tail_n && j < m_bodys.size() - head_tail_n)
					{
						if (first_skip)
						{
							first_skip = false;
							ret += "......\n";
						}
						else continue;
					}
					else
					{
						for (i = 0; i < m_bodys[j].cells.size(); ++i)
						{
							string data = m_heads.cells[i].toShow(m_bodys[j].cells[i].value);
							str.assign(v_colmaxlen[i] - outputlen(data), ' ');
							if (CHtmlDoc_DATACLASS_RIGHT == m_heads.cells[i].dataclass)
							{
								ret += str;
								ret += data;
							}
							else
							{
								ret += data;
								ret += str;
							}
							ret += " ";
						}
						ret += "\n";
					}
				}
				for (i = 0; i < actualcount; ++i)
				{
					str.assign(v_colmaxlen[i], '-');
					ret += str;
					ret += " ";
				}
				ret += "\n";
				for (i = 0; i < actualcount; ++i)
				{
					if (i < m_foots.cells.size())
					{
						str.assign(v_colmaxlen[i] - outputlen(m_foots.cells[i].value), ' ');
						if (CHtmlDoc_DATACLASS_RIGHT == m_foots.cells[i].dataclass)
						{
							ret += str;
							ret += m_foots.cells[i].value;
						}
						else
						{
							ret += m_foots.cells[i].value;
							ret += str;
						}
					}
					ret += " ";
				}
				ret += "\n";

				return ret;
			}
			bool ParseFromXml_GetLabel(string const & xml, string::size_type & start_pos, string::size_type & end_pos, string & label, bool & IDS_isEnd)
			{
				start_pos = xml.find("<", start_pos);
				if (xml.npos == start_pos)return false;
				end_pos = xml.find(">", start_pos + 1);
				if (xml.npos == end_pos)
				{
					thelog << "缺少>" << ende;
					return false;
				}
				label = xml.substr(start_pos + 1, end_pos - start_pos - 1);
				if (0 == label.size())
				{
					thelog << "空标签" << ende;
					return false;
				}
				if ('/' == label[0])
				{
					IDS_isEnd = true;
					label.erase(0, 1);
				}
				else
				{
					IDS_isEnd = false;
				}
				end_pos += 1;
				//thelog<<"label:"<<label<<" "<<(IDS_isEnd?"结束":"开始")<<endi;
				return true;
			}
			//从xml分析，单一表格，section为空则根节点之下就是，否则位于section下
			bool ParseFromXml(string const & xml, string const & section, vector<vector<pair<string, string> > > & vmss_head, vector<vector<pair<string, string> > > & vmss_record)
			{
				return ParseFromXml2(xml, section, "__HEADS__", vmss_head) && ParseFromXml2(xml, section, "__RECORDS__", vmss_record);
			}
			//从xml分析，多表格，需要指定表格的标签,section表格入口，part表格的部分（head or record）
			bool ParseFromXml2(string const & xml, string const & section, string const & part, vector<vector<pair<string, string> > > & vmss)
			{
				vmss.clear();

				string::size_type start_pos;
				string::size_type end_pos;

				start_pos = xml.find("<?");
				if (start_pos != xml.npos)
				{
					end_pos = xml.find("?>", start_pos + 2);
					end_pos += 1;
					start_pos = end_pos;
				}
				else
				{
					start_pos = 0;
					end_pos = 0;
				}

				bool IDS_isEnd;
				string root;

				do
				{
					start_pos = end_pos;
					if (!ParseFromXml_GetLabel(xml, start_pos, end_pos, root, IDS_isEnd))
					{
						thelog << "缺少根标签 " << section << ende;
						return false;
					}
					if (IDS_isEnd)continue;
				} while (0 != section.size() && root != section);

				do
				{
					start_pos = end_pos;
					if (!ParseFromXml_GetLabel(xml, start_pos, end_pos, root, IDS_isEnd))
					{
						thelog << "缺少标签 " << part << ende;
						return false;
					}
					if (IDS_isEnd)continue;
				} while (root != part);

				while (true)
				{
					string record;
					start_pos = end_pos;
					if (!ParseFromXml_GetLabel(xml, start_pos, end_pos, record, IDS_isEnd))
					{
						thelog << "缺少标签" << ende;
						return false;
					}
					if (record == root)
					{
						if (IDS_isEnd)break;
						else
						{
							thelog << "缺少开始标签" << ende;
							return false;
						}
					}
					else
					{
						vmss.resize(vmss.size() + 1);
					}

					while (true)
					{
						string field;
						start_pos = end_pos;
						if (!ParseFromXml_GetLabel(xml, start_pos, end_pos, field, IDS_isEnd))
						{
							thelog << "缺少标签" << ende;
							return false;
						}
						if (field == record)
						{
							if (IDS_isEnd)break;
							else
							{
								thelog << "缺少开始标签" << ende;
								return false;
							}
						}

						string tmp;
						string::size_type old_pos = end_pos;
						start_pos = end_pos;
						if (!ParseFromXml_GetLabel(xml, start_pos, end_pos, tmp, IDS_isEnd))
						{
							thelog << "缺少标签" << ende;
							return false;
						}
						if (tmp == field && IDS_isEnd)
						{
							string value;
							value = XMLDecode(xml.substr(old_pos, start_pos - old_pos));

							//thelog<<field<<" "<<value<<endi;
							vmss[vmss.size() - 1].push_back(pair<string, string>(XMLDecode(field), value));
						}
						else
						{
							thelog << "缺少结束标签" << ende;
							return false;
						}
					}

				}

				return true;
			}
			bool LoadFromXml(string const & xml)
			{
				return LoadFromXml2(xml, "");
			}
			bool LoadFromXml2(string const & xml, string const & section)
			{
				this->Clear();
				vector<vector<pair<string, string> > > vmss_head;
				vector<vector<pair<string, string> > > vmss_record;
				if (!ParseFromXml(xml, section, vmss_head, vmss_record))return false;

				vector<map<string, string> > heads;//转换为map
				heads.resize(vmss_head.size());
				for (size_t m = 0; m < vmss_head.size(); ++m)
				{
					for (size_t n = 0; n < vmss_head[m].size(); ++n)
					{
						heads[m][vmss_head[m][n].first] = vmss_head[m][n].second;
					}
				}

				for (long i = 0; i < (long)vmss_record.size(); ++i)
				{
					AddLine();
					vector<pair<string, string> >::const_iterator it;
					for (size_t j = 0; j < vmss_record[i].size(); ++j)
					{
						if (0 == i)
						{
							//thelog << heads[j]["_dbtype"] << " : " << heads[j]["_dbshowtype"] << endi;
							DATACLASS dataclass = (COLUMN_TYPE_STRING_POOL == atol(heads[j]["_dbtype"].c_str()) ? CHtmlDoc_DATACLASS_TEXT : CHtmlDoc_DATACLASS_RIGHT);
							AddCol_db(vmss_record[i][j].first, dataclass
								, atol(heads[j]["_dbtype"].c_str())
								, heads[j]["_dbcomment"].c_str()
								, heads[j]["_dbshowtype"].c_str());
						}
						AddData(vmss_record[i][j].second);
					}
				}
				return true;
			}
			enum OUTPUT_TYPE { OUTPUT_HTML = 0, OUTPUT_SCRIPT, OUTPUT_TXT, OUTPUT_XML, OUTPUT_XMLSECTION };
			string MakeOutput(OUTPUT_TYPE type, CHtmlTable2 * oldtable)
			{
				string ret;
				char const * _section = (0 == m_id.size() ? "__table__" : m_id.c_str());

				if (OUTPUT_HTML == type)
				{
					return  MakeHtmlTable();
				}
				else if (OUTPUT_SCRIPT == type)
				{
					return MakeUpdateScript(oldtable);
				}
				else if (OUTPUT_TXT == type)
				{
					return MakeTextTable();
				}
				else if (OUTPUT_XML == type)
				{
					return MakeXML(_section, ret);
				}
				else if (OUTPUT_XMLSECTION == type)
				{
					return MakeXMLSection(_section, ret);
				}
				else
				{
					return ret = "unknown output type";
				}
			}
		};
	};
}

