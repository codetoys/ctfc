//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../function/config.h"
#include "myUtil.h"
#include <sys/stat.h>
#ifdef _WINDOWS
#else
#include <sys/ioctl.h>
#include <termios.h>
#endif
