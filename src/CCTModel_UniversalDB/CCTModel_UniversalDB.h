//CCTModel_UniversalDB.h 通用数据库对象模型
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
using namespace ns_my_std;

class CCTModel_UniversalDB
{
private:
	//成员变量定义
	struct member
	{
		//group为空则为列定义，使用下面一组成员
		string member_name;
		string member_comment;//单行
		string member_type;//long time double string
		string member_default;//默认值
		string member_show_type;//显示方式

		//group不为空则为列组访问，使用下面一组成员
		string group;//组名，
		string var_name;//变量名

		//生成数据库类型
		string makeDBType()const
		{
			stringstream ss;
			if ("string" == member_type)ss << "COLUMN_TYPE_STRING_POOL";
			else if ("long" == member_type)ss << "COLUMN_TYPE_LONG";
			else if ("time" == member_type)ss << "COLUMN_TYPE_LONG";
			else if ("double" == member_type)ss << "COLUMN_TYPE_DOUBLE";
			else ss << "错误的类型 " << member_type;
			return ss.str();
		}
		//生成参数类型
		string makeParamType()const
		{
			stringstream ss;
			if ("string" == member_type)ss << "char const *";
			else if ("long" == member_type)ss << "long const";
			else if ("time" == member_type)ss << "long const";
			else if ("double" == member_type)ss << "double const";
			else ss << "错误的类型 " << member_type;
			return ss.str();
		}
		//生成参数值
		string makeParamValue()const
		{
			stringstream ss;
			if ("string" == member_type)ss << member_name << ".c_str()";
			else if ("long" == member_type)ss << member_name;
			else if ("time" == member_type)ss << member_name;
			else if ("double" == member_type)ss << member_name;
			else ss << "错误的类型 " << member_type;
			return ss.str();
		}
		//生成printf类型
		string makePrintfType()const
		{
			stringstream ss;
			if ("string" == member_type)ss << "%s";
			else if ("long" == member_type)ss << "%ld";
			else if ("time" == member_type)ss << "%ld";
			//else if ("double" == member_type)ss << "%f";
			else ss << "错误的类型 " << member_type;
			return ss.str();
		}
		//生成HTML输出类型
		string makeHtmlType()const
		{
			stringstream ss;
			if ("string" == member_type)ss << "CHtmlDoc::CHtmlDoc_DATACLASS_LEFT";
			else if ("time" == member_type)ss << "CHtmlDoc::CHtmlDoc_DATACLASS_TIME";
			else ss << "CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT";
			return ss.str();
		}

		//输出到CCTObject
		CCTObject toCCTObject()
		{
			CCTObject O;
			if (0 == group.size())
			{
				O.SetObjectAddProperty("name", member_name);
				O.SetObjectAddProperty("GetName", member_name);
				O.SetObjectAddProperty("GroupIndexVar", "");
				O.SetObjectAddProperty("comment", member_comment);
				O.SetObjectAddProperty("type", ("time" == member_type ? "long" : member_type));
				O.SetObjectAddProperty("default", member_default);
				O.SetObjectAddProperty("show_type", member_show_type);
				O.SetObjectAddProperty("DBType", makeDBType());
				O.SetObjectAddProperty("ParamType", makeParamType());
				O.SetObjectAddProperty("ParamName", member_name);
				O.SetObjectAddProperty("PrintfType", makePrintfType());
				O.SetObjectAddProperty("ParamValue", makeParamValue());
				O.SetObjectAddProperty("HtmlType", makeHtmlType());
			}
			else
			{
				O.SetObjectAddProperty("name", var_name);
				O.SetObjectAddProperty("GetName", "\" + GetGroup" + group + "ColName(" + var_name + ") + \"");
				O.SetObjectAddProperty("GroupIndexVar", " " + var_name + ",");
				O.SetObjectAddProperty("ParamType", "long " + var_name + ", long");
				O.SetObjectAddProperty("ParamName", var_name);
				O.SetObjectAddProperty("PrintfType", "%ld");
			}
			return O;
		}
	};
	//索引定义
	struct index
	{
		string index_name;
		string index_comment;
		bool unique{ false };
		string index_members;

		//输出到CCTObject
		CCTObject toCCTObject()
		{
			CCTObject O;
			O.SetObjectAddProperty("name", index_name);
			O.SetObjectAddProperty("comment", index_comment);
			O.SetObjectAddProperty("unique", (unique ? "true" : "false"));
			O.SetObjectAddProperty("members", index_members);
			return O;
		}
	};
	//列组定义（可以用下标标识的多个列）
	struct group
	{
		string group_name;
		string group_comment;
		vector<member > group_members;

		//输出到CCTObject
		CCTObject toCCTObject()
		{
			CCTObject O;
			O.SetObjectAddProperty("name", group_name);
			O.SetObjectAddProperty("comment", group_comment);
			for (vector<member >::iterator it = group_members.begin(); it != group_members.end(); ++it)
			{
				O.SetObjectArrayPushBack("members", it->toCCTObject());
			}
			return O;
		}
	};
	//DML定义
	struct dml
	{
		string dml_name;
		string dml_type;//select insert update delete
		string dml_comment;
		vector<member > op_members;//操作的列
		bool hasWhere;//是否有where，当where_members和other_where均为空则不添加此属性
		vector<member > where_members;//条件列，仅支持 and =
		string other_where;//直接写死的条件部分

		//输出到CCTObject
		CCTObject toCCTObject()
		{
			hasWhere = (where_members.size() != 0 || other_where.size() != 0);
			CCTObject O;
			O.SetObjectAddProperty("name", dml_name);
			O.SetObjectAddProperty("type", dml_type);
			O.SetObjectAddProperty("comment", dml_comment);
			for (vector<member >::iterator it = op_members.begin(); it != op_members.end(); ++it)
			{
				O.SetObjectArrayPushBack("op_members", it->toCCTObject());
			}
			if (hasWhere)O.SetObjectAddProperty("hasWhere", hasWhere);
			for (vector<member >::iterator it = where_members.begin(); it != where_members.end(); ++it)
			{
				O.SetObjectArrayPushBack("where_members", it->toCCTObject());
			}
			if(other_where.size() != 0)O.SetObjectAddProperty("other_where", other_where);
			return O;
		}
	};
public:
	class table
	{
	public:
		string table_name;
		string table_comment;
		vector<member > table_members;
		string PK_cols;//主键的列
		vector<index > table_indexs;//主键名称为PK
		vector<group > table_groups;//可以用下标访问的列组
		vector<dml > table_dmls;//数据操作，对应一个sql

		member * _getMember(string const & name)
		{
			for (size_t i = 0; i < table_members.size(); ++i)
			{
				if (name == table_members[i].member_name)return &table_members[i];
			}
			return NULL;
		}
		bool SetTable(char const * name, char const * comment)
		{
			table_name = name;
			table_comment = comment;
			return true;
		}
		bool AddMember(char const * name, char const * type, char const * comment, char const * _default = "", char const * _show_type="")
		{
			member m;
			m.member_name = name;
			m.member_type = type;
			m.member_comment = comment;
			m.member_default = _default;
			m.member_show_type = _show_type;
			if ("string" == m.member_type)m.member_default = "\"" + m.member_default + "\"";
			else
			{
				if (0 == m.member_default.size())m.member_default = "0";
			}

			table_members.push_back(m);
			return true;
		}
		//设置主键
		bool SetPK(char const * PK)
		{
			PK_cols = PK;

			StringTokenizer st(PK_cols, ",");
			for (size_t i = 0; i < st.size(); ++i)
			{
				member * p = _getMember(st[i].c_str());
				if (NULL == p)
				{
					thelog << "主键指定的列不存在 " << st[i] << ende;
					exit(0);
				}
			}
			return true;
		}
		bool AddIndex(char const * name, bool unique,char const * members, char const * comment)
		{
			StringTokenizer st(members, ",");
			index tmp;
			tmp.index_name = name;
			tmp.unique = unique;
			tmp.index_comment = comment;
			tmp.index_members = members;

			//检查
			for (size_t i = 0; i < st.size(); ++i)
			{
				member* p = _getMember(st[i].c_str());
				if (NULL == p)
				{
					thelog << "索引指定的列不存在 " << st[i] << ende;
					exit(0);
				}
			}

			table_indexs.push_back(tmp);
			return true;
		}
		bool AddGroup(char const * name, char const * members, char const * comment)
		{
			StringTokenizer st(members, ",");
			group tmp;
			tmp.group_name = name;
			tmp.group_comment = comment;
			for (size_t i = 0; i < st.size(); ++i)
			{
				member * p = _getMember(st[i].c_str());
				if (NULL == p)
				{
					thelog << "列组指定的列不存在 " << st[i] << ende;
					exit(0);
				}
				tmp.group_members.push_back(*p);
			}

			table_groups.push_back(tmp);
			return true;
		}
		bool AddDML(char const * name, char const * type, char const * op_members, char const * where_members, char const * comment, char const * other_where = "")
		{
			dml tmp;
			tmp.dml_name = name;
			tmp.dml_type = type;
			tmp.dml_comment = comment;
			tmp.other_where = other_where;

			if (0 != strlen(op_members))
			{
				StringTokenizer st(op_members, ",");
				for (size_t i = 0; i < st.size(); ++i)
				{
					if ("*" == st[i])
					{//全部
						for (vector<member >::const_iterator it = table_members.begin(); it != table_members.end(); ++it)
						{
							tmp.op_members.push_back(*it);
						}
					}
					else
					{
						StringTokenizer st2(st[i], ":");
						if (2 == st2.size())
						{//列组
							bool isGroupFound = false;
							for (vector<group >::const_iterator it = table_groups.begin(); it != table_groups.end(); ++it)
							{
								if (it->group_name == st2[0])
								{
									isGroupFound = true;
								}
							}
							if (!isGroupFound)
							{
								thelog << "op_members指定的列组不存在 " << st2[0] << ende;
								exit(0);
							}
							member tmpmember;
							tmpmember.group = st2[0];
							tmpmember.var_name = st2[1];
							tmp.op_members.push_back(tmpmember);
						}
						else
						{//简单列
							member * p = _getMember(st[i].c_str());
							if (NULL == p)
							{
								thelog << "op_members指定的列不存在 " << st[i] << ende;
								exit(0);
							}
							tmp.op_members.push_back(*p);
						}
					}
				}
			}
			if (0 != strlen(where_members))
			{
				StringTokenizer st(where_members, ",");
				for (size_t i = 0; i < st.size(); ++i)
				{
					member * p = _getMember(st[i].c_str());
					if (NULL == p)
					{
						thelog << "where_members指定的列不存在 " << st[i] << ende;
						exit(0);
					}
					tmp.where_members.push_back(*p);
				}
			}

			table_dmls.push_back(tmp);
			return true;
		}
		//输出到CCTObject
		CCTObject toCCTObject()
		{
			CCTObject O;
			O.SetObjectAddProperty("name", table_name);
			O.SetObjectAddProperty("comment", table_comment);
			O.SetObjectAddProperty("PK_cols", PK_cols);
			for (vector<member >::iterator it = table_members.begin(); it != table_members.end(); ++it)
			{
				O.SetObjectArrayPushBack("members", it->toCCTObject());
			}
			for (vector<index >::iterator it = table_indexs.begin(); it != table_indexs.end(); ++it)
			{
				O.SetObjectArrayPushBack("indexs", it->toCCTObject());
			}
			for (vector<group >::iterator it = table_groups.begin(); it != table_groups.end(); ++it)
			{
				O.SetObjectArrayPushBack("groups", it->toCCTObject());
			}
			for (vector<dml >::iterator it = table_dmls.begin(); it != table_dmls.end(); ++it)
			{
				O.SetObjectArrayPushBack("dmls", it->toCCTObject());
			}
			return O;
		}
	};
	class sequence
	{
	public:
		string sequence_name;
		string sequence_init;
		string sequence_comment;

		//输出到CCTObject
		CCTObject toCCTObject()
		{
			CCTObject O;
			O.SetObjectAddProperty("name", sequence_name);
			O.SetObjectAddProperty("comment", sequence_comment);
			O.SetObjectAddProperty("init", sequence_init);
			return O;
		}
	};
private:
	vector<table > m_tables;
	vector<sequence > m_sequences;

	set<string > _table_names;//用来检查是否已经存在
	set<string > _sequence_names;//用来检查是否已经存在

public:
	CCTObject toCCTObject()
	{
		CCTObject O;
		O.SetObjectAddProperty("name", "CodeTemplate");
		for (vector<table >::iterator it = m_tables.begin(); it != m_tables.end(); ++it)
		{
			O.SetObjectArrayPushBack("tables", it->toCCTObject());
		}
		for (vector<sequence >::iterator it = m_sequences.begin(); it != m_sequences.end(); ++it)
		{
			O.SetObjectArrayPushBack("sequences", it->toCCTObject());
		}
		return O;
	}

	sequence * AddNewSequence(char const * sequencename, char const * comment, long init)
	{
		if (_sequence_names.end() != _sequence_names.find(sequencename))
		{
			thelog << sequencename << " 已经存在" << ende;
			return NULL;
		}
		else
		{
			_sequence_names.insert(sequencename);
		}
		m_sequences.resize(m_sequences.size() + 1);
		sequence * p = &m_sequences[m_sequences.size() - 1];
		p->sequence_name = sequencename;
		p->sequence_comment = comment;
		char buf[256];
		sprintf(buf, "%ld", init);
		p->sequence_init = buf;
		return p;
	}
	table * AddNewTable(char const * tablename, char const * comment)
	{
		if (_table_names.end() != _table_names.find(tablename))
		{
			thelog << tablename << " 已经存在" << ende;
			return NULL;
		}
		else
		{
			_table_names.insert(tablename);
		}

		m_tables.resize(m_tables.size() + 1);
		table * p = &m_tables[m_tables.size() - 1];
		p->SetTable(tablename, comment);

		return p;
	}
public:
	//_ns 名字空间，sys 总类的名称的一部分
	bool CreateCode(char const * _ns, char const * sys, char const * output_dir)
	{
		CCTObject O;
		O = toCCTObject();

		O.SetObjectAddProperty("NAMESPACE", _ns);
		O.SetObjectAddProperty("SYS", sys);

		CCTStack S;

		CCodeTemplate ct;

		string str;
		//thelog << endl << O.toString(str) << endi;

		S.clear();
		if (!ct.ProcessFile("_t_UniversalDB.h.ct", (string(output_dir) + "/_cc_" + sys + ".h").c_str(), O, S))return false;

		vector<CCTObject > * p;
		if (NULL != O.FindObject("tables"))
		{
			p = &O.FindObject("tables")->m_Array;
			for (vector<CCTObject >::iterator it = p->begin(); it != p->end(); ++it)
			{
				S.clear();
				S.Push();
				S.Add("table", *it);
				//thelog << endl << S.toString(str) << endi;
				if (!ct.ProcessFile("_t_UniversalDB_table.h.ct", (string(output_dir) + "/_cc_" + sys + "_" + it->GetDefaultValue() + ".h").c_str(), O, S))return false;
			}
		}
		if (NULL != O.FindObject("sequences"))
		{
			p = &O.FindObject("sequences")->m_Array;
			for (vector<CCTObject >::iterator it = p->begin(); it != p->end(); ++it)
			{
				S.clear();
				S.Push();
				S.Add("sequence", *it);
				thelog << endl << S.toString(str) << endi;
				if (!ct.ProcessFile("_t_UniversalDB_sequence.h.ct", (string(output_dir) + "/_cc_" + sys + "_" + it->GetDefaultValue() + ".h").c_str(), O, S))return false;
			}
		}

		return true;
	}

};

