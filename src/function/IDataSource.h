//IDataSource.h 数据源接口
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
#include <ctype.h>

/////////////////////////////////////////////////////////////
//HTML相关功能
namespace ns_my_std
{
	class CIDataSource_forward
	{
	public:
		virtual long IDS_GetColCount()const = 0;
		virtual string IDS_GetColName(long col_index)const = 0;
		//获得附加属性，若不支持则返回false，不支持的属性返回""
		virtual bool IDS_GetColDbInfo(long col_index, int& dataclass, long& dbtype, string& dbshowtype, string& dbcomment)const = 0;
		virtual long IDS_GetColIndex(char const* col_name)const = 0;
		virtual bool IDS_MoveFirst() = 0;
		virtual bool IDS_MoveNext() = 0;
		virtual bool IDS_isEnd()const = 0;
		virtual string IDS_GetCurrentData(long col)const = 0;
		virtual string IDS_GetCurrentDataToShow(long col)const = 0;
		virtual string IDS_OutputCurrentRecord()const = 0;
	};
	class CIDataSource_random : public CIDataSource_forward
	{
	public:
		virtual bool IDS_Move(long id) = 0;//移动到指定行
		virtual long IDS_GetCurrentID()const = 0;//当前记录的ID，可能是行号也可能是行指针之类
		virtual string IDS_GetCurrent_OnClick()const = 0;
		virtual string IDS_GetDataByID(long id, long col)const = 0;
		virtual string IDS_GetOnClickByID(long id)const = 0;
		virtual string IDS_GetDataToShowByID(long id, long col)const = 0;
	};
}
