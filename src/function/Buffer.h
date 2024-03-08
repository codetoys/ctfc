//Buffer.h 缓冲区
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"

namespace ns_my_std
{
	//内部隐含保持一个结束符，不计算在容量和大小之内，确保数据可以被当作字符串使用
	class CBuffer
	{
	private:
		char * p;
		size_t buffer_size;
		size_t data_size;

		void _init()
		{
			p = new char[1];
			if (!p)throw "内存不足";
			p[0] = '\0';
			buffer_size = 0;
		}
	public:
		CBuffer() :p(NULL), buffer_size(0), data_size(0)
		{
			_init();
		}
		CBuffer(CBuffer const & tmp) :p(NULL), buffer_size(0), data_size(0)
		{
			_init();
			SetData(tmp.data(), tmp.data_size);
		}
		CBuffer & operator=(CBuffer const & tmp)
		{
			_init();
			SetData(tmp.data(), tmp.data_size);
			return *this;
		}
		~CBuffer()
		{
			if (p)delete[]p;
			data_size = 0;
			buffer_size = 0;
			p = NULL;
		}
		size_t capacity()const { return buffer_size; }
		size_t size()const { return data_size; }
		char const * data()const { return p; }
		char * getbuffer() { return p; }
		void setSize(long s)
		{
			reserve(s);
			p[s] = '\0';
			data_size = s;
		}
		bool reserve(size_t n)
		{
			if (n > buffer_size)
			{
				char * p2 = new char[n + 1];
				if (p2)
				{
					if (p)
					{
						memmove(p2, p, data_size);
						delete[]p;
					}
					buffer_size = n;
					p2[data_size] = '\0';
					p = p2;
					return true;
				}
				else
				{
					return false;
				}
			}
			return true;
		}
		bool AddData(void const * data, long len)
		{
			if (!reserve(data_size + len))return false;
			memmove(p + data_size, data, len);
			setSize(data_size + len);
			return true;
		}
		bool SetData(char const * sz)
		{
			setSize(0);
			return AddData((void *)sz, strlen(sz));
		}
		bool SetData(char const * sz, long len)
		{
			setSize(0);
			return AddData((void *)sz, len);
		}
		bool Compare(CBuffer const & tmp)const
		{
			if (data_size != tmp.data_size)return false;
			if (data_size > 0)return 0 == memcmp(tmp.data(), data(), data_size);
			else return true;
		}
	};
}
