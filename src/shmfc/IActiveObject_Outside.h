//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <string>
#include "../function/sstring.h"

//共享内存公共接口
class IShmActiveObject_outside
{
public:
	virtual char const * GetName()const = 0;
	virtual bool isPrivateMem()const = 0;
	virtual bool isConnected()const = 0;
	virtual bool isReadOnly()const = 0;
	virtual bool CreateShm() = 0;
	virtual bool CreatePrivate() = 0;
	virtual bool Attach(bool isReadOnly) = 0;
	virtual bool Detach() = 0;
	virtual bool LoadFromDB() = 0;
	virtual bool SaveToDB() = 0;
	virtual bool LoadFromDir(char const * dir_name) = 0;
	virtual bool LoadPrivateFromDir(char const * dir_name) = 0;
	virtual bool DestoryShm() = 0;
	virtual bool DestoryPrivate() = 0;
	virtual bool SaveToDir(char const * dir_name)const = 0;
	virtual bool ExportTextToDir(char const * dir_name)const = 0;
	virtual bool Report()const = 0;
	virtual bool ReportData()const = 0;
	virtual bool check()const = 0;
	virtual bool repair(char const* level) = 0;
	virtual bool clear() = 0;
	virtual size_t size()const = 0;
	virtual size_t capacity()const = 0;
	virtual size_t record_length()const = 0;
	virtual size_t byte_size()const = 0;
	virtual size_t byte_capacity()const = 0;
	virtual size_t block_count()const = 0;
	virtual bool ToDo(char const * what) = 0;
	virtual void RunCmdUI() = 0;
	virtual std::string & ShowRet(std::string & ret, bool all = false)const = 0;
	virtual bool FastRebuild_Start() = 0;
	virtual bool FastRebuild_IncreToStatic() = 0;
	virtual bool FastRebuild_Finish(char const* level, bool noDisk) = 0;
};
