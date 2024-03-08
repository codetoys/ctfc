//FastFileIndex.h 快速文件索引
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include "mydir.h"
#include "function.h"
#include "Buffer.h"

namespace ns_my_std
{
	//快速文件索引，此结构本身可以放在共享内存
	class CFastFileInex
	{
	private:
		sstring<256> m_dir;//根目录
		long section;//几个字节一层目录，同一个目录下文件超过一定数量打开文件会很慢
	public:
		char const * GetRoot()const { return m_dir.c_str(); }
		string & toString(string & str)const
		{
			stringstream ss;
			ss << "dir[" << m_dir.c_str() << "]" << section;
			return str = ss.str();
		}
		void SetFastFileInex(char const * dir, long _section)
		{
			m_dir = dir;
			section = _section;
		}
		string & ffiGetFileName(unsigned long key, string & filename)const
		{
			char buf[256];
			sprintf(buf, "%lu", key);
			return ffiGetFileName(buf, filename);
		}
		string & ffiGetFileName(char const * key, string & filename)const
		{
			filename = m_dir.c_str();

			int len = strlen(key);
			long k = len / section;
			if (0 == len % section)--k;
			for (long i = 0; i < k; ++i)
			{
				for (long j = 0; j < section; ++j)	filename += key[i*section + j];
				filename += '/';
			}
			filename += key;
			filename += ".cache";
			return filename;
		}
		bool ffiReadFile(unsigned long key, CBuffer & buffer)const
		{
			char buf[256];
			sprintf(buf, "%lu", key);
			return ffiReadFile(buf, buffer);
		}
		bool ffiReadFile(char const * key, CBuffer & buffer)const
		{
			string filename;
			ffiGetFileName(key, filename);

			//打开失败再创建目录，再次打开失败才算失败
			CEasyFile file;
			if (!file.ReadFile(filename.c_str(), buffer))
			{
				CDir::CreateDir(filename.c_str());
				if (!file.ReadFile(filename.c_str(), buffer))
				{
					DEBUG_LOG << "打开文件出错 " << filename << " " << strerror(errno) << ende;
					return false;
				}
			}
			return true;
		}
		//读不到足够数据则失败
		bool ffiReadFile(long key, char * buffer, long buffersize)const
		{
			char buf[256];
			sprintf(buf, "%lu", key);
			return ffiReadFile(buf, buffer, buffersize);
		}
		//读不到足够数据则失败
		bool ffiReadFile(char const * key, char * buffer, long buffersize)const
		{
			string filename;
			ffiGetFileName(key, filename);

			//打开失败再创建目录，再次打开失败才算失败
			CEasyFile file;
			if (!file.ReadFile(filename.c_str(), buffer, buffersize))
			{
				CDir::CreateDir(filename.c_str());
				if (!file.ReadFile(filename.c_str(), buffer, buffersize))
				{
					DEBUG_LOG << "打开文件出错 " << filename << " " << strerror(errno) << ende;
					return false;
				}
			}
			return true;
		}
		bool ffiWriteFile(long key, char const * buffer, long buffersize)const
		{
			char buf[256];
			sprintf(buf, "%lu", key);
			return ffiWriteFile(buf, buffer, buffersize);
		}
		bool ffiWriteFile(char const * key, char const * buffer, long buffersize)const
		{
			string filename;
			ffiGetFileName(key, filename);

			//打开失败再创建目录，再次打开失败才算失败
			CEasyFile file;
			if (!file.WriteFile(filename.c_str(), buffer, buffersize))
			{
				CDir::CreateDir(filename.c_str());
				if (!file.WriteFile(filename.c_str(), buffer, buffersize))
				{
					DEBUG_LOG << "写入文件出错 " << filename << " " << strerror(errno) << ende;
					return false;
				}
			}
			return true;
		}
		bool ffiDeleteFile(long key)const
		{
			char buf[256];
			sprintf(buf, "%lu", key);
			return ffiDeleteFile(buf);
		}
		bool ffiDeleteFile(char const * key)const
		{
			string filename;
			ffiGetFileName(key, filename);

			//打开失败再创建目录，再次打开失败才算失败
			CEasyFile file;
			if (!file.DeleteFile(filename.c_str()))
			{
				DEBUG_LOG << "删除文件出错 " << filename << " " << strerror(errno) << ende;
				return false;
			}
			return true;
		}
	};
}

