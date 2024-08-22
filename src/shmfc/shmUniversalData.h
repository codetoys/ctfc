//shm_UniversalData.h 共享内存通用数据结构
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmstd.h"

namespace ns_my_std
{
	struct T_UNI_KEY
	{
		long type;
		long key;

		T_UNI_KEY() :type(0), key(0) {}
		//用于需要排序的场合
		bool operator < (T_UNI_KEY const & tmp)const
		{
			if (type != tmp.type)return type < tmp.type;
			else return key < tmp.key;
		}
		bool operator == (T_UNI_KEY const & tmp)const
		{
			return type == tmp.type && key == tmp.key;
		}
		bool operator != (T_UNI_KEY const & tmp)const
		{
			return type != tmp.type || key != tmp.key;
		}
		//用于输出数据的场合
		string & toString(string & str)const
		{
			char buf[2048];
			sprintf(buf, "%ld %ld", type, key);
			return str = buf;
		}

		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{
			table.AddCol("type", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("key", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddData(type);
			table.AddData(key);
			return true;
		}
	};
	struct T_UNI_DATA
	{
		//32字节的数据，可以当作四个long或者一个字符串或者任意32个字节
		union
		{
			char data_str[sizeof(long) * 4];
			long data_long[4];
		}data;

		T_UNI_DATA()
		{
			data.data_long[0] = 0;
			data.data_long[1] = 0;
			data.data_long[2] = 0;
			data.data_long[3] = 0;
		}

		//用于输出数据的场合
		string & toString(string & str)const
		{
			char buf[2048];
			sprintf(buf, "%ld,%ld,%ld,%ld[%s][%s][%s][%s]", data.data_long[0], data.data_long[1], data.data_long[2], data.data_long[3]
				, _tryGetString(0).c_str(), _tryGetString(1).c_str(), _tryGetString(2).c_str(), _tryGetString(3).c_str());
			return str = buf;
		}
		//尝试获得字符串，如果长度超出则返回空串
		string _tryGetString(long pos)const
		{
			for (size_t i = pos * sizeof(long); i < sizeof(long) * 4; ++i)
			{
				if ('\0' == data.data_str[i])return data.data_str + pos * sizeof(long);
			}
			return "";
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2 & table)
		{
			table.AddCol("data0", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data1", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data2", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data3", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data0s", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data1s", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data2s", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("data3s", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddData(data.data_long[0]);
			table.AddData(data.data_long[1]);
			table.AddData(data.data_long[2]);
			table.AddData(data.data_long[3]);
			table.AddData(_tryGetString(0));
			table.AddData(_tryGetString(1));
			table.AddData(_tryGetString(2));
			table.AddData(_tryGetString(3));
			return true;
		}
	};
}
