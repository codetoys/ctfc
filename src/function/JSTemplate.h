//JSTemplate.h JS输出
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
	class CHtml_Check
	{
	public:
		//多选框组合操作，全选，全不选，反选
		static string ChecksOperate(char const* name)
		{
			stringstream ss;
			ss<<"<script type = \"text/javascript\">\n"
				"	function f()\n"
				"{\n"
				"	var check_s = document.getElementsByName(\""<<name<<"\");\n"
				"	check_s.forEach(function(e) { e.checked = true; });\n"
				"}\n"
				"function f2()\n"
				"{\n"
				"	var check_s = document.getElementsByName(\""<<name<<"\");\n"
				"	check_s.forEach(function(e) { e.checked = false; });\n"
				"}\n"
				"function f3()\n"
				"{\n"
				"	var check_s = document.getElementsByName(\"" << name << "\");\n"
				"	check_s.forEach(function(e) { if(e.checked == true)e.checked = false;else e.checked = true; });\n"
				"}\n"

				"</script>\n"
				"<button type = \"button\" onclick = \"f();\">全选</button>\n"
				"<button type = \"button\" onclick = \"f2();\">全不选</button>\n"
				"<button type = \"button\" onclick = \"f3();\">反选</button>\n"
				;
			return ss.str();
		}
	};
}
