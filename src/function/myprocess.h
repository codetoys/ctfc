//myprcoess.h 进程相关
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#ifdef _WINDOWS
#else
#include <sys/wait.h>
#endif

namespace ns_my_std
{
	class CMyProcess
	{
	public:
		static bool isProcessLive(pid_t pid)
		{
			//waitpid返回0表明进程在运行
			if (0 != waitpid(pid, NULL, WNOHANG))
			{
				//如果进程已经停止，则kill 0会失败，由于已经做过waitpid，僵尸进程已经回收
				if (0 != kill(pid, 0))
				{
					return false;
				}
			}

			return true;
		}
		//如果已经停止返回true，如果是被信号终止则bTerm为true
		static bool WaitProcessFinish(pid_t pid, bool & bTerm, int& ret, int& termSig)
		{
			int status;
			waitpid(pid, &status, 0);//返回时一定已经结束
			bTerm = false;
			if (WIFEXITED(status))
			{
				ret = WEXITSTATUS(status);
			}
			if (WIFSIGNALED(status))
			{
				bTerm = true;
				termSig = WTERMSIG(status);
			}
			return true;
		}
	};
}
