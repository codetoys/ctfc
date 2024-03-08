//myexception.h 小功能类
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

//如果使用HP编译器请定义“_HP_ACC”

///////////////////////////////////////////////////////////////////////
//exception_my 定义异常类，继承自exception
//构造函数：exception_my(char const * file,long line,long errid,char const * msg)
//一般用法：throw exception_my(__FILE__,__LINE__,1,"内存不足");

#pragma once

namespace ns_my_std
{
	class exception_my
	{
	private:
		enum { BUF_SIZE = 10240 };
		char m_szMessage[BUF_SIZE];
	public:
		exception_my(char const* file, long line, char const* msg)
		{
			snprintf(m_szMessage, BUF_SIZE, "异常发生，位于文件 %s 的第 %ld 行\n错误描述：%s\n", file, line, msg);
		}
		exception_my(char const* file, long line, long errid, char const* msg)
		{
			snprintf(m_szMessage, BUF_SIZE, "异常发生，位于文件 %s 的第 %ld 行\n错误号：%ld\n错误描述：%s\n", file, line, errid, msg);
		}
#ifdef _HP_ACC
		virtual const char * what()const __THROWSPEC_NULL{return m_szMessage;}
#else
		virtual const char * what()const throw(){return m_szMessage;}
#endif
	};
}
