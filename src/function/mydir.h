//mydir.h 文件系统功能
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

namespace ns_my_std
{
	class CDir
	{
	public:
		static bool IsDir(char const * strPathName)
		{
			struct stat statBuf;

			if (stat(strPathName, &statBuf) == -1)
			{
				cout << "stat error : " << strPathName << endl;
				return false;
			}

			//return(statBuf.st_mode & S_IFDIR);
			return 0 != S_ISDIR(statBuf.st_mode);
		}
		static bool CreateDir(char const * filename)
		{
			string dirname;
			char const * p = filename;
			for (; '\0' != *p; ++p)
			{
				dirname += *p;
				if ('/' == *p)
				{
					//thelog<<dirname<<endi;
					mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
				}
			}
			return true;
		}
		static int ClearDir(char const * dirname, bool isKeepDir, long & count, long & count_err)
		{
			struct dirent *drip;
			DIR *dp;
			string fullname;
			string inputdir = dirname;
			if ((dp = opendir(inputdir.c_str())) == NULL)
			{
				++count_err;
				thelog << "Error open dir " << inputdir << " : " << strerror(errno) << ende;
				return __LINE__;
			}

			while ((drip = readdir(dp)) != NULL)
			{
				if (strcmp(drip->d_name, ".") == 0 || strcmp(drip->d_name, "..") == 0)
					continue;

				fullname = inputdir + "/" + drip->d_name;
				//thelog<<fullname<<endi;
				if (IsDir(fullname.c_str()))
				{
					ClearDir(fullname.c_str(), isKeepDir, count, count_err);
				}
				if (!isKeepDir || !IsDir(fullname.c_str()))
				{
					if (0 == unlink(fullname.c_str()))++count;
					else ++count_err;
				}

				if (count + count_err > 0 && 0 == (count + count_err) % 10000)
				{
					cout << "已处理 " << count + count_err << " 删除 " << count << " 个，出错 " << count_err << " 个" << endl;
				}
			}
			closedir(dp);

			return 0;
		}
	};
	class CForEachDir
	{
	private:
		//默认动作的数据
		int model;//内置的动作，0为显示名称
		long more_skip;//只处理这么多
		long doOneFile_count;//doOneFile用的计数
	private://接口
		//进入目录时调用一次
		virtual int doDirBegin(char const * dirname, long deep) { return 0; }
		virtual int doDirEnd(char const * dirname, long deep) { return 0; }
		//对每个文件调用一次
		virtual int doOneFile(char const * filename, bool isDir, long deep)
		{
			if (0 == model)
			{
				if (doOneFile_count < more_skip)cout << filename << endl;
				++doOneFile_count;
			}
			else
			{
				cout << "未知的模式 " << model << endl;
			}
			return 0;
		}
	private://内部函数
		int _doForEachDir(char const * dirname, bool isNoDir, bool r, long & count, long & count_err, long & _, long deep)
		{
			struct dirent *drip;
			DIR *dp;
			string fullname;
			string inputdir = dirname;
			if ((dp = opendir(inputdir.c_str())) == NULL)
			{
				++count_err;
				DEBUG_LOG << "Error open dir " << inputdir << " : " << strerror(errno) << ende;
				return __LINE__;
			}

			doDirBegin(inputdir.c_str(), deep);
			while ((drip = readdir(dp)) != NULL)
			{
				if (strcmp(drip->d_name, ".") == 0 || strcmp(drip->d_name, "..") == 0)
					continue;

				fullname = inputdir + "/" + drip->d_name;
				//thelog<<fullname<<endi;
				bool isdir = CDir::IsDir(fullname.c_str());
				if (isdir && r)
				{
					_doForEachDir(fullname.c_str(), isNoDir, r, count, count_err, _, deep + 1);
				}
				if (!isNoDir || !isdir)
				{
					if (0 == doOneFile(fullname.c_str(), isdir, deep))++count;
					else ++count_err;
				}

				if (_ != count + count_err)
				{
					_ = count + count_err;
					if (count + count_err > 0 && 0 == (count + count_err) % 10000)
					{
						cout << "已处理 " << count + count_err << " 成功 " << count << " 个，出错 " << count_err << " 个" << endl;
					}
				}
			}
			doDirEnd(inputdir.c_str(), deep);
			closedir(dp);

			return 0;
		}
	public:
		CForEachDir():model(-1){}
		int doForEachDir(char const * dirname, bool isNoDir, bool r, long & count, long & count_err)
		{
			count = 0;
			count_err = 0;
			long tmp = 0;
			return _doForEachDir(dirname, isNoDir, r, count, count_err, tmp, 0);
		}
		int showAllFileName(char const * dirname, long _more_skip = -1)
		{
			long count;
			long count_err;

			model = 0;
			more_skip = _more_skip;
			doOneFile_count = 0;
			return doForEachDir(dirname, true, true, count, count_err);
		}
	};
}
