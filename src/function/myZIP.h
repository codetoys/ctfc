//myZIP.h 数据压缩
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include "function.h"
#include "Buffer.h"
#include "zlib/zlib.h"

namespace ns_my_std
{
	//数据压缩，压缩后数据自带解压缩以后长度
	class CMyZip
	{
	public:
		typedef unsigned long T_LEN;
		//默认压缩级别
		static bool Compress(char const * src, long srclen, CBuffer & output)
		{
			return Compress2(src, srclen, output, Z_DEFAULT_COMPRESSION);
		}
		//level 0-9 越大压缩率越高耗时越长
		static bool Compress2(char const * src, T_LEN srclen, CBuffer & output, int level)
		{
			if (Z_DEFAULT_COMPRESSION == level);
			else if (level < 0)level = 0;
			else if (level > 9)level = 9;
			else;

			if (!output.reserve(compressBound(srclen) + sizeof(T_LEN)))
			{
				thelog << "内存不足" << ende;
				return false;
			}
			T_LEN len = output.capacity() - sizeof(T_LEN);
			if (0 != compress2((unsigned char *)output.getbuffer() + sizeof(T_LEN), &len, (unsigned char *)src, srclen, level))
			{
				thelog << "压缩失败" << ende;
				return false;
			}
			T_LEN tmp = srclen;
			memmove(output.getbuffer(), &tmp, sizeof(T_LEN));
			output.setSize(sizeof(T_LEN) + len);
			return true;
		}
		static bool UnCompress(char const * src, long srclen, CBuffer & output)
		{
			return _UnCompress(src, srclen, output, 1);
		}
		static bool _UnCompress(char const * src, long srclen, CBuffer & output, long buf_override)
		{
			T_LEN outsize;//压缩数据记录的解压缩后长度
			memmove(&outsize, src, sizeof(T_LEN));
			//thelog << "记录的长度 " << outsize << endi;
			if (0 == outsize)
			{
				output.setSize(0);
				return true;
			}
			if (!output.reserve(outsize*buf_override))
			{
				thelog << "内存不足" << ende;
				return false;
			}
			unsigned long len = output.capacity();
			int ret = uncompress((unsigned char *)output.getbuffer(), &len, (unsigned char *)src + sizeof(T_LEN), srclen - sizeof(T_LEN));
			if (0 != ret)
			{
				if (Z_MEM_ERROR == ret)
				{
					thelog << "解压缩失败 内存不足" << ende;
					return false;
				}
				else if (Z_STREAM_ERROR == ret)
				{
					thelog << "解压缩失败 level错误" << ende;
					return false;
				}
				else if (Z_BUF_ERROR)
				{
					thelog << "预设缓冲区不足  " << buf_override << " " << output.capacity() << " " << len << ende;
					return _UnCompress(src, srclen, output, buf_override + 1);
				}
				else
				{
					thelog << "解压缩失败 " << ret << ende;
					return false;
				}
			}
			if (len != outsize)
			{
				thelog << "解压缩后长度与预期不一致 " << len << " " << outsize << ende;
				return false;
			}
			output.setSize(len);
			return true;
		}
	public:
		static int CZip_test(int argc, char ** argv)
		{
			CEasyFile file;
			CBuffer input;
			CBuffer output;
			CBuffer output2;
			if (!file.ReadFile("../function/myZIP.h", input))
			{
				thelog << "读文件失败" << ende;
				return __LINE__;
			}
			if (!Compress(input.data(), input.size(), output))return __LINE__;
			thelog << input.size() << " " << output.size() << " " << (100 - output.size() * 100 / (input.size() ? input.size() : 1)) << "%" << endi;
			if (!UnCompress(output.data(), output.size(), output2))return __LINE__;
			thelog << input.size() << " 解压缩后 " << output2.size() << endi;
			if (!input.Compare(output2))return __LINE__;
			for (long i = 0; i < 10; ++i)
			{
				if (!Compress2(input.data(), input.size(), output, i))return __LINE__;
				thelog << i << " " << input.size() << " " << output.size() << " " << (100 - output.size() * 100 / (input.size() ? input.size() : 1)) << "%" << endi;
			}
			return 0;
		}
	};
}
