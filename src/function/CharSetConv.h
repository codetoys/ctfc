//CharSetConv.h 字符集转换
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include <iconv.h> 
#include <string> 

namespace ns_my_std
{

#define OUTLEN 255 

	using namespace std;

	// 代码转换操作类 
	class CodeConverter 
	{
	private:
		iconv_t cd;
		string from_charset;
		string to_charset;
	public:
		bool isInited;
		// 构造 
		CodeConverter() :isInited(false) {}
		// 析构 
		~CodeConverter() 
		{
			if (isInited)closeCodeConverter();
		}
		
		bool initCodeConverter(const char* _from_charset, const char* _to_charset)
		{
			if (isInited)closeCodeConverter();

			from_charset = _from_charset;
			to_charset = _to_charset;

			cd = iconv_open(to_charset.c_str(), from_charset.c_str());
			isInited = ((iconv_t)-1 != cd);
			if (!isInited)cout << "iconv_open失败，将使用命令转换" << endl;

			return true;
		}
		void closeCodeConverter()
		{
			if (isInited)
			{
				iconv_close(cd);
				isInited = false;
			}
		}

		bool convert(string & str)
		{
			if (isInited)
			{
				int bufsize = str.size() * 2 + 20;
				char* buffer = new char[bufsize];

				string msg;
				if (convert((char*)str.c_str(), str.size(), buffer, bufsize, msg))
				{
					str = buffer;
					delete[] buffer;
					return true;
				}
				else
				{
					cout << str << endl;
					cout << msg << endl;
					delete[] buffer;
					return false;
				}
			}
			else
			{
				char buf[1024];
				FILE* pf;

				CEasyFile file;
				if (!file.WriteFile("iconv.tmp", str.c_str()))
				{
					cout << "WriteFile失败，无法转换" << endl;
					return false;
				}
				
				string cmd = string("./iconv -f ") + from_charset + " -t " + to_charset + " iconv.tmp";
				if (NULL == (pf = popen(cmd.c_str(), "r")))
				{
					cout << "popen失败，无法执行：" << errno << " : " << strerror(errno) << endl;
					return false;
				}

				//获取输出
				str = "";
				while (NULL != fgets(buf, 1024, pf))
				{
					str += buf;
				}
				
				pclose(pf);
				return true;
			}
		}

		// 转换输出，注意类型必须是size_t，不能是&int转换成size_t*
		bool convert(char* inbuf, size_t inlen, char* outbuf, size_t outlen,string & msg)
		{
			if (!isInited)
			{
				return false;
			}
			char** pin = &inbuf;
			char** pout = &outbuf;

			memset(outbuf, 0, outlen);
			--outlen;//确保最后有个结束符
			if ((size_t)-1 != iconv(cd, pin, & inlen, pout, & outlen))
			{
				return true;
			}
			else
			{
				switch (errno)
				{
				case EILSEQ:
					msg = "The conversion stopped because of an invalid byte sequence in the input. After the call, *inbuf points at the first byte of the invalid byte sequence. ";
					break;
				case E2BIG:
					msg = "The conversion stopped because it ran out of space in the output buffer. ";
					break;
				case EINVAL:
					msg = "The conversion stopped because of an incomplete byte sequence at the end of the input buffer. ";
					break;
				case EBADF:
					msg = "";
					break;
				default:
					msg = strerror(errno);
					break;
				}
				return false;
			}
		}
	};
}

