//myNestedTable.h 嵌套表格
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
#include "htmldoc.h"
#include <ctype.h>

namespace ns_my_std
{
	//嵌套表格
	class CNestedTable : public CIDataSource_random
	{
	public:
		struct rule
		{
			vector<string> groupcols;
			string othername;
			bool nohead;//不显示列头
			bool horizontal;//横向

			rule() :nohead(false), horizontal(false) {}
			void clear()
			{
				groupcols.clear();
				othername.clear();
				nohead = false;
				horizontal = false;
			}
		};

	private:
		struct column
		{
			long datasource_index;//数据源索引
			long column_index;//数据源中的索引，为负则为嵌套对象
			string column_name;//未设置则从数据源取得名字
		};
		struct record
		{
			struct _record_part
			{
				long datasource_index;
				long line_index;
			};
			vector<_record_part > datasource_line_s;//一组数据源和行号，数量和pDataSources相同
			map<long, pair<CNestedTable, CNestedTable> > col_object_s;//列-对象

			string show()const
			{
				stringstream ss;
				for (size_t i = 0; i < datasource_line_s.size(); ++i)
				{
					ss << "[" << datasource_line_s[i].datasource_index << "," << datasource_line_s[i].line_index << "] ";
				}
				return ss.str();
			}
		};

		struct data_source_s : public vector<CIDataSource_random* >
		{
		};
		data_source_s pDataSources;//指向数据源
		rule m_rule;//规则

		vector<column > m_columns;//所有列
		vector<record > m_records;//所有行

		//CIDataSource_random
		long m_current_record;//当前记录

		void Clear()
		{
			pDataSources.clear();
			m_rule.clear();
			m_columns.clear();
			m_records.clear();

			m_current_record = -1;
		}

		CIDataSource_random* SOURCE(long col)const { return pDataSources[m_columns[col].datasource_index]; }
		long SOURCE_LINE(long line, long col)const { return m_records[line].datasource_line_s[m_columns[col].datasource_index].line_index; }
		long SOURCE_LINE(record const & line, long col)const { return line.datasource_line_s[m_columns[col].datasource_index].line_index; }
		long SOURCE_COL(long col)const { return m_columns[col].column_index; }
	public:
		string GetDataByRecord(record const& r, long col)const
		{
			if (SOURCE_COL(col) < 0)return "";
			return SOURCE(col)->IDS_GetDataByID(SOURCE_LINE(r, col), SOURCE_COL(col));
		}
	public:
		virtual long IDS_GetColCount()const { return m_columns.size(); }
		virtual string IDS_GetColName(long vcol_index)const 
		{
			//对象列或设置了列名的直接返回，否则从数据源获得列名
			if (m_columns[vcol_index].column_index < 0 || m_columns[vcol_index].column_name.size() != 0)return m_columns[vcol_index].column_name;
			return SOURCE(vcol_index)->IDS_GetColName(m_columns[vcol_index].column_index); 
		}
		virtual bool IDS_GetColDbInfo(long vcol_index, int& dataclass, long& dbtype, string& dbshowtype, string& dbcomment)const
		{
			if (m_columns[vcol_index].column_index < 0)return false;
			return SOURCE(vcol_index)->IDS_GetColDbInfo(m_columns[vcol_index].column_index, dataclass, dbtype, dbshowtype, dbcomment);
		}
		virtual long IDS_GetColIndex(char const* col_name)const
		{
			for (vector<column >::const_iterator it = m_columns.begin(); it != m_columns.end(); ++it)
			{
				if (it->column_index < 0)continue;
				if (pDataSources[it->datasource_index]->IDS_GetColName(it->column_index) == col_name)return it - m_columns.begin();
			}
			return -1;
		}
		virtual bool IDS_MoveFirst() { m_current_record = 0; return !IDS_isEnd(); }
		virtual bool IDS_MoveNext() { ++m_current_record; return !IDS_isEnd(); }
		virtual bool IDS_Move(long id) { m_current_record = id; return !IDS_isEnd(); }
		virtual bool IDS_isEnd()const { return m_current_record < 0 || static_cast<size_t>(m_current_record) >= m_records.size(); }
		virtual long IDS_GetCurrentID()const { return m_current_record; }
		virtual string IDS_GetCurrentData(long vcol)const
		{
			return IDS_GetDataByID(m_current_record, vcol);
		}
		virtual string IDS_GetCurrent_OnClick()const
		{
			return IDS_GetOnClickByID(m_current_record);
		}
		virtual string IDS_GetCurrentDataToShow(long col)const
		{
			return IDS_GetDataToShowByID(m_current_record, col);
		}
		virtual string IDS_OutputCurrentRecord()const
		{
			stringstream ss;

			for (long i = 0; i < IDS_GetColCount(); ++i)
			{
				ss << "[" << this->IDS_GetColName(i) << "](" << IDS_GetCurrentData(i) << ") ";
			}

			return ss.str();
		}
		virtual string IDS_GetDataByID(long id, long col)const
		{
			if (SOURCE_COL(col) < 0)return "";
			if (id < 0)return "";
			return SOURCE(col)->IDS_GetDataByID(SOURCE_LINE(id, col), SOURCE_COL(col));
		}
		virtual string IDS_GetOnClickByID(long id)const
		{
			if (id < 0)return "";
			return SOURCE(0)->IDS_GetOnClickByID(SOURCE_LINE(id, 0));
		}
		virtual string IDS_GetDataToShowByID(long id, long col)const
		{
			if (SOURCE_COL(col) < 0)return "";
			if (id < 0)return "";
			return SOURCE(col)->IDS_GetDataToShowByID(SOURCE_LINE(id, col), SOURCE_COL(col));
		}

	public:
		class SORT_Less
		{
		private:
			CNestedTable* p;
		public:
			struct _less_col
			{
				string col_name;
				bool asc;
				COLUMN_TYPE col_type;
			
				long col_index;
				_less_col() :asc(true), col_type(COLUMN_TYPE_STRING_POOL), col_index(0) {}
				bool less(CNestedTable* p, record const& a, record const& b)const
				{
					bool ret = _less(p,a,b);
					//thelog << "结果 " << ret << endi;
					return ret;
				}
				bool _less(CNestedTable* p, record const& a, record const& b)const
				{
					string data_a = p->GetDataByRecord(a, col_index);
					string data_b = p->GetDataByRecord(b, col_index);
					//thelog << (unsigned long)p << " " << col_name << " " << col_index << " " << &a - &p->m_records[0] << " " << &b - &p->m_records[0] << " " << data_a << " " << data_b << endi;
					if (asc)
					{
						switch (col_type)
						{
						case COLUMN_TYPE_LONG:return atol(data_a.c_str()) < atol(data_b.c_str());
						case COLUMN_TYPE_DOUBLE:return atof(data_a.c_str()) < atof(data_b.c_str());
						default:return data_a < data_b;
						}
					}
					else
					{
						switch (col_type)
						{
						case COLUMN_TYPE_LONG:return atol(data_a.c_str()) > atol(data_b.c_str());
						case COLUMN_TYPE_DOUBLE:return atof(data_a.c_str()) > atof(data_b.c_str());
						default:return data_a > data_b;
						}
					}
				}
			};
			vector<_less_col > cols;
			SORT_Less():p(NULL){}
			CNestedTable* getP() { return p; }
			string to_string()const
			{
				stringstream ss;
				ss << (unsigned long)p << " " << cols.size();
				for (vector<_less_col >::const_iterator it = cols.begin(); it != cols.end(); ++it)
				{
					ss << endl << it->col_name << "[" << it->col_index << "] asc " << it->asc << " type " << it->col_type;
				}
				return ss.str();
			}
			bool AddCol(char const* colname, bool asc, COLUMN_TYPE type)
			{
				_less_col tmp;
				tmp.col_name = colname;
				tmp.asc = asc;
				tmp.col_type = type;
				cols.push_back(tmp);
				return true;
			}
			bool FindCol(char const* colname)const
			{
				for (vector<_less_col >::const_iterator it = cols.begin(); it != cols.end(); ++it)
				{
					if (it->col_name == colname)return true;
				}
				return false;
			}
			bool Prepare(CNestedTable* _p)
			{
				p = _p;

				if (NULL == p)return false;
				for (vector<_less_col >::iterator it = cols.begin(); it != cols.end(); ++it)
				{
					it->col_index = p->IDS_GetColIndex(it->col_name.c_str());
					if (it->col_index < 0)
					{
						thelog << "无效的列 " << it->col_name << ende;
						return false;
					}
					//thelog << it->col_name << " " << it->col_index << endi;
				}
				return true;
			}
			bool operator()(record const& a, record const& b)const
			{
				for (vector<_less_col >::const_iterator it = cols.begin(); it != cols.end(); ++it)
				{
					if (it->less(p, b, a))return false;
					if (it->less(p, a, b))return true;
				}
				return false;
			}
		};
		class JOIN_Less
		{
		public:
			SORT_Less left;
			SORT_Less right;
			string to_string()const
			{
				return left.to_string() + "\n" + right.to_string();
			}
			bool AddCol(char const* colname, bool asc, COLUMN_TYPE type)
			{
				return left.AddCol(colname, asc, type) && right.AddCol(colname, asc, type);
			}
			bool AddCol2(char const* leftcol, char const* rightcol, bool asc, COLUMN_TYPE type)
			{
				return left.AddCol(leftcol, asc, type) && right.AddCol(rightcol, asc, type);
			}
			bool Prepare(CNestedTable* _left, CNestedTable* _right)
			{
				if (left.Prepare(_left) && right.Prepare(_right))
				{
					_left->Sort(left);
					_right->Sort(right);
					return true;
				}
				else
				{
					return false;
				}
			}
			//比较当前行
			int comp()
			{
				for (size_t i = 0; i < left.cols.size(); ++i)
				{
					string data_left = left.getP()->IDS_GetCurrentData(left.cols[i].col_index);
					string data_right = right.getP()->IDS_GetCurrentData(right.cols[i].col_index);
					int ret = strcmp(data_left.c_str(), data_right.c_str());
					if (0 != ret)
					{//仅在不相同时需要进行数值比较
						switch(left.cols[i].col_type)
						{
						default:
							thelog << "错误的类型 " << left.cols[i].col_type << ende;
							break;
						case COLUMN_TYPE_STRING_POOL:
							break;
						case COLUMN_TYPE_LONG:
							ret = atol(data_left.c_str()) - atol(data_right.c_str());
							break;
						case COLUMN_TYPE_DOUBLE:
							double tmp = atof(data_left.c_str()) - atof(data_right.c_str());
							ret = (tmp < 0 ? -1 : 1);
							break;
						}
						return (left.cols[i].asc ? ret : -ret);
					}
				}
				return 0;
			}
		};

	public:
		CNestedTable() { Clear(); }

		//从表格加载
		struct load_other_match
		{
			virtual bool is_load_other_match(CIDataSource_forward* _pSource, bool& isMatch) = 0;
		};

		bool LoadFromTableByFilter(CIDataSource_random& data_source, load_other_match * pFilter)
		{
			return _LoadFromTable(data_source, pFilter);
		}
		bool LoadFromTable(CIDataSource_random& data_source)
		{
			return _LoadFromTable(data_source, NULL);
		}
		bool _LoadFromTable(CIDataSource_random& data_source, load_other_match* pFilter)
		{
			pDataSources.clear();
			pDataSources.push_back(&data_source);

			column tmpcolumn;
			tmpcolumn.datasource_index = 0;
			for (long col = 0; col < data_source.IDS_GetColCount(); ++col)
			{
				tmpcolumn.column_index = col;
				m_columns.push_back(tmpcolumn);
			}
			record tmprecord;
			tmprecord.datasource_line_s.resize(1);
			tmprecord.datasource_line_s[0].datasource_index = 0;
			for (data_source.IDS_MoveFirst(); !data_source.IDS_isEnd(); data_source.IDS_MoveNext())
			{
				if (pFilter != NULL)
				{
					bool isMatch;
					if (!pFilter->is_load_other_match(&data_source, isMatch))
					{
						thelog << "is_load_other_match 出错" << ende;
						return false;
					}
					if (!isMatch)continue;
				}
				
				tmprecord.datasource_line_s[0].line_index = data_source.IDS_GetCurrentID();
				m_records.push_back(tmprecord);
				//thelog << tmprecord.show() << endi;
			}

			return true;
		}
		//排序
		void Sort(SORT_Less& comp)
		{
			//thelog << "开始排序..." << endi;
			if (!comp.Prepare(this))
			{
				thelog << "比较对象初始化错误" << ende;
				return;
			}
			sort(m_records.begin(), m_records.end(), comp);
			//thelog << "排序完成" << endi;
		}
		//连接表
		struct join_other_match
		{
			//_pRight可能为NULL，表示右表未匹配
			virtual bool is_join_other_match(CNestedTable* _pLeft, CNestedTable* _pRight,bool & isMatch) = 0;
		};
		bool LoadFromInnerJoin(CNestedTable& left_source, CNestedTable& right_source, JOIN_Less& less, join_other_match* other_match)
		{
			return _LoadFromJoin(false, left_source, right_source, less, other_match);
		}
		bool LoadFromLeftJoin(CNestedTable& left_source, CNestedTable& right_source, JOIN_Less& less, join_other_match* other_match)
		{
			return _LoadFromJoin(true, left_source, right_source, less, other_match);
		}
		bool _LoadFromJoin(bool isLeft, CNestedTable& left_source, CNestedTable& right_source, JOIN_Less& less, join_other_match* other_match)
		{
			if (!less.Prepare(&left_source, &right_source))return false;

			pDataSources.push_back(&left_source);
			pDataSources.push_back(&right_source);

			column tmpcolumn;
			tmpcolumn.column_name = "";
			
			//添加左表所有列
			tmpcolumn.datasource_index = 0;
			for (long scol = 0; scol < left_source.IDS_GetColCount(); ++scol)
			{
				//thelog << left_source.IDS_GetColName(scol) << " " << scol << endi;

				tmpcolumn.column_index = scol;
				//thelog << scol << " " << tmpcolumn.column_index << " " << left_source.IDS_GetColName(scol) << " " << left_source.IDS_GetColName(tmpcolumn.column_index) << endi;
				m_columns.push_back(tmpcolumn);
			}
			//添加右表所有非连接条件列
			tmpcolumn.datasource_index = 1;
			for (long scol = 0; scol < right_source.IDS_GetColCount(); ++scol)
			{
				//thelog << right_source.IDS_GetColName(scol) << " " << scol << endi;
				//if (less.right.FindCol(right_source.IDS_GetColName(scol).c_str()))continue;

				tmpcolumn.column_index = scol;
				//thelog << scol << " " << tmpcolumn.column_index << " " << right_source.IDS_GetColName(scol) << " " << right_source.IDS_GetColName(tmpcolumn.column_index) << endi;
				m_columns.push_back(tmpcolumn);
			}

			//连接行
			right_source.IDS_MoveFirst();
			long right_begin = right_source.IDS_GetCurrentID();
			for (left_source.IDS_MoveFirst(); !left_source.IDS_isEnd(); left_source.IDS_MoveNext())
			{
				bool matched = false;
				bool full_matched = false;
				//thelog << "======左 " << left_source.IDS_GetCurrentID() << " : " << left_source.IDS_OutputCurrentRecord() <<" 右起始位置 "<< right_begin << endi;
				for (right_source.IDS_Move(right_begin); !right_source.IDS_isEnd(); right_source.IDS_MoveNext())
				{
					int comp = less.comp();
					//thelog << "右 " << right_source.IDS_GetCurrentID() << " : " << right_source.IDS_OutputCurrentRecord() << " 比较结果 " << comp << endi;
					if (comp < 0)
					{
						break;//左边小，跳出，左边往下走
					}
					else if (comp > 0)
					{
						continue;//左边大，右边往下走
					}
					else
					{
						if (!matched)
						{
							right_begin = right_source.IDS_GetCurrentID();//首次匹配到，更新右边起始位置
							matched = true;
						}
						bool isMatch = true;
						if (NULL != other_match && !other_match->is_join_other_match(&left_source, &right_source, isMatch))
						{
							thelog << "join_other_match 出错" << ende;
							return false;
						}
						if (isMatch)
						{
							full_matched = true;
							record tmprecord;
							tmprecord.datasource_line_s.resize(2);
							tmprecord.datasource_line_s[0].datasource_index = 0;
							tmprecord.datasource_line_s[0].line_index = left_source.IDS_GetCurrentID();
							tmprecord.datasource_line_s[1].datasource_index = 1;
							tmprecord.datasource_line_s[1].line_index = right_source.IDS_GetCurrentID();
							m_records.push_back(tmprecord);
						}
					}
				}
				if (isLeft && !full_matched)
				{
					record tmprecord;
					tmprecord.datasource_line_s.resize(2);
					tmprecord.datasource_line_s[0].datasource_index = 0;
					tmprecord.datasource_line_s[0].line_index = left_source.IDS_GetCurrentID();
					tmprecord.datasource_line_s[1].datasource_index = 1;
					tmprecord.datasource_line_s[1].line_index = -1;
					m_records.push_back(tmprecord);
				}
			}

			return true;
		}
		//从表格加载
		bool LoadFromTable(CIDataSource_random& source, vector<rule > rule)
		{
			return LoadFromTable(source, rule, 0);
		}
		bool LoadFromTable(CIDataSource_random& source, vector<rule > rules, long level)
		{
			thelog << "-------------------------------------" <<" level "<<level<< endi;
			pDataSources.clear();
			pDataSources.push_back(&source);
			m_rule = rules[level];

			vector<string>& groupcols = rules[level].groupcols;
			string othername = rules[level].othername;

			column tmpcolumn;
			tmpcolumn.datasource_index = 0;

			//设置列
			for (vector<string>::const_iterator it = groupcols.begin(); it != groupcols.end(); ++it)
			{
				tmpcolumn.column_index = source.IDS_GetColIndex(it->c_str());
				if (tmpcolumn.column_index < 0)
				{
					thelog << "无效的列 " << *it << ende;
					return false;
				}
				m_columns.push_back(tmpcolumn);
			}
			tmpcolumn.column_index = -1;
			tmpcolumn.column_name = othername;
			m_columns.push_back(tmpcolumn);

			//子表
			CNestedTable childtable;
			childtable.pDataSources = pDataSources;

			//设置子表的列
			tmpcolumn.column_name = "";
			for (long scol = 0; scol < source.IDS_GetColCount(); ++scol)
			{
				//thelog <<"level "<< level << " " << source.IDS_GetColName(scol)<<" "<<scol << endi;
				bool isGroupCol = false;
				for (vector<string>::const_iterator it = groupcols.begin(); it != groupcols.end(); ++it)
				{
					if (source.IDS_GetColName(scol) == *it)
					{
						isGroupCol = true; thelog << *it << " 匹配" << endi;
						break;
					}
				}
				if (isGroupCol)continue;

				tmpcolumn.column_index = scol;
				//thelog <<"添加对象列 源列 "<< scol << " " << tmpcolumn.column_index << " " << source.IDS_GetColName(scol) << " " << source.IDS_GetColName(tmpcolumn.column_index) << endi;
				childtable.m_columns.push_back(tmpcolumn);
			}

			//处理行
			for (source.IDS_MoveFirst(); !source.IDS_isEnd(); source.IDS_MoveNext())
			{
				//thelog <<"source 行 "<<source.IDS_GetCurrentID()<<" : "<< source.IDS_OutputCurrentRecord() << endi;
				long found_line = -1;//已经存在的虚拟行和最终的虚拟行
				for (this->IDS_MoveFirst(); !this->IDS_isEnd(); this->IDS_MoveNext())
				{
					//thelog << this->IDS_OutputCurrentRecord() << endi;
					bool isSame = true;
					for (size_t col = 0; col < m_columns.size(); ++col)
					{
						if (m_columns[col].column_index < 0)continue;//跳过对象列
						//thelog <<"源：" << source.IDS_GetCurrentID() << " " << m_columns[col].column_index << " 存在的：" << this->IDS_GetCurrentID() << " " << col << endi;
						//thelog << source.IDS_GetCurrentData(m_columns[col].column_index) << " " << this->IDS_GetCurrentData(col) << endi;
						if (this->IDS_GetCurrentData( col) != source.IDS_GetCurrentData( m_columns[col].column_index))
						{
							isSame = false;
							break;
						}
					}
					if (isSame)
					{
						found_line = this->IDS_GetCurrentID();
						break;
					}
				}
				if (!this->IDS_Move(found_line))
				{
					//thelog << level << " 发现新行 " << source.IDS_GetCurrentID() << endi;
					record tmprecord;
					tmprecord.datasource_line_s.resize(1);
					tmprecord.datasource_line_s[0].datasource_index = 0;
					tmprecord.datasource_line_s[0].line_index = source.IDS_GetCurrentID();
					tmprecord.col_object_s[m_columns.size() - 1].second = childtable;
					m_records.push_back(tmprecord);
					found_line = m_records.size() - 1;

					this->IDS_Move(found_line);
				}
				//填充子对象
				//thelog << level << " 填充子对象 " << m_records[found_line].show() << endi;
				record tmprecord;
				tmprecord.datasource_line_s.resize(1);
				tmprecord.datasource_line_s[0].datasource_index = 0;
				tmprecord.datasource_line_s[0].line_index = source.IDS_GetCurrentID();
				m_records[found_line].col_object_s[m_columns.size() - 1].second.m_records.push_back(tmprecord);
			}

			//处理子对象
			if (static_cast<size_t>(level + 1) < rules.size())
			{
				for (size_t vline = 0; vline < m_records.size(); ++vline)
				{
					m_records[vline].col_object_s[m_columns.size() - 1].first = m_records[vline].col_object_s[m_columns.size() - 1].second;
					m_records[vline].col_object_s[m_columns.size() - 1].second.Clear();
					if (!m_records[vline].col_object_s[m_columns.size() - 1].second.LoadFromTable(m_records[vline].col_object_s[m_columns.size() - 1].first, rules, level + 1))
					{
						return false;
					}
				}
			}

			return true;
		}
		stringstream& MakeHtmlTable(stringstream& ss_out, long level = 0)
		{
			string px;
			for (long _ = 0; _ < level; ++_)px += "\t";
			ss_out << px << "<table class=\"table1\">\r\n";

			if (this->m_rule.horizontal)
			{
				ss_out << px << "<thead><tr>\r\n";
				for (IDS_MoveFirst(); !IDS_isEnd(); IDS_MoveNext())
				{
					ss_out << px << "<th><code>";
					if (m_columns[0].column_index < 0)m_records[IDS_GetCurrentID()].col_object_s[0].second.MakeHtmlTable(ss_out, level + 1);
					else ss_out << IDS_GetCurrentDataToShow(0);
					ss_out << px << "</code></th>\r\n";
				}
				ss_out << px << "</tr></thead>\r\n";
			}
			else if (!this->m_rule.nohead)
			{
				ss_out << px << "<thead><tr>\r\n";
				for (vector<column >::const_iterator it = m_columns.begin(); it != m_columns.end(); ++it)
				{
					ss_out << px << "<th><code>";
					ss_out << this->IDS_GetColName(it - m_columns.begin());
					ss_out << px << "</code></th>\r\n";
				}
				ss_out << px << "</tr></thead>\r\n";
			}
			else
			{
			}

			ss_out << px << "<tbody>\r\n";
			if (this->m_rule.horizontal)
			{
				for (size_t vcol = 1; vcol < m_columns.size(); ++vcol)
				{
					ss_out << px << "<tr>\r\n";
					for (IDS_MoveFirst(); !IDS_isEnd(); IDS_MoveNext())
					{
						ss_out << px << "<td><code>";
						if (m_columns[vcol].column_index < 0)m_records[IDS_GetCurrentID()].col_object_s[vcol].second.MakeHtmlTable(ss_out, level + 1);
						else ss_out << this->IDS_GetCurrentDataToShow(vcol);
						ss_out << px << "</code></td>\r\n";
					}
					ss_out << px << "</tr>\r\n";
				}
			}
			else
			{
				for (IDS_MoveFirst(); !IDS_isEnd(); IDS_MoveNext())
				{
					if (0 == this->m_records[IDS_GetCurrentID()].col_object_s.size())
					{//没有子对象说明是最后一级，需要输出脚本
						ss_out << px << "<tr onclick=\"" << this->IDS_GetCurrent_OnClick() << "\">\r\n";
					}
					else
					{
						ss_out << px << "<tr>\r\n";
					}
					for (size_t vcol = 0; vcol < m_columns.size(); ++vcol)
					{
						ss_out << px << "<td><code>";
						if (m_columns[vcol].column_index < 0)m_records[IDS_GetCurrentID()].col_object_s[vcol].second.MakeHtmlTable(ss_out, level + 1);
						else ss_out << this->IDS_GetCurrentDataToShow(vcol);
						ss_out << px << "</code></td>\r\n";
					}
					ss_out << px << "</tr>\r\n";
				}
			}
			ss_out << px << "</tbody>";
			ss_out << px << "</table>\r\n";
			return ss_out;
		}
		//导出表格，不包括嵌套对象
		bool ExportToTable(CHtmlDoc::CHtmlTable2 * pTable)
		{
			pTable->Clear();

			for (vector<column >::const_iterator it = m_columns.begin(); it != m_columns.end(); ++it)
			{
				pTable->AddCol(this->IDS_GetColName(it - m_columns.begin()));
			}

			for (IDS_MoveFirst(); !IDS_isEnd(); IDS_MoveNext())
			{
				pTable->AddLine();
				for (size_t vcol = 0; vcol < m_columns.size(); ++vcol)
				{
					pTable->AddData(this->IDS_GetCurrentDataToShow(vcol));
				}
			}
			return true;
		}
	};
}
