//std_error_code.h 错误码
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

/*
CArchive和CUnArchive实现XML文档化
通过operator()(name,data)来实现输入输出
已经对基本数据类型做了实现，用户数据类型需要通过定义Serialize（name,ar）和UnSerialize(name,unar)来实现
具体例子：
Test_Serialize 简单结构
CSerializeSet 容器模板的再包装
CTest_Serialize 复杂的容器
*/

#pragma once

#include <string>
#include <map>

namespace ns_my_std_interface2
{
	using namespace std;

	enum
	{
		My_ERR_SUCESS=0
		
		//笼统的出错
		, My_ERR_ERROR=1//出错返回

		//共享内存底层
		, My_ERR_INVALID_HANDLE//无效的句柄
		, My_ERR_INVALID_BALANCE//无效的平衡因子

		//内存数据库
		, My_ERR_PK_CONFLICT//PK冲突

		//网络和命令集接口
		, My_ERR_NOT_TTY//不是终端
		, My_ERR_PORT_OCCUPY//端口使用中
		, My_ERR_START_TIMEOUT//启动超时
	};
}
