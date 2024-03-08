//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/myUtil.h" 
using namespace ns_my_std;
#include "myhttpbase.h"

void ShowHttpdVersion();

//启动服务，如果带有“-stop”参数则关闭集群
int start_httpd(char const * ini_section, vector<CWebCommand * > const & ws, char const * server_name, int argc, char ** argv);
