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
		static const size_t	npos = static_cast<size_t>(-1);
		size_t find(string const& str)const
		{
			return find(str, 0);
		}

		size_t find(string const& str,size_t pos_start)const
		{
			//cout << __FILE__ << " " << __LINE__ << " : --------------------------------------------" << endl;
			//cout << __FILE__ << " " << __LINE__ << " : " << this->data_size << " : " << p << endl;
			//cout << __FILE__ << " " << __LINE__ << " : " << pos_start << endl;
			//cout << __FILE__ << " " << __LINE__ << " : " << str.size() << " : " << str << endl;
			char* found = search(p + pos_start, p + data_size, str.c_str(), str.c_str() + str.size());
			if (found == p + data_size)
			{
				//cout << "没找到" << endl;
				return npos;
			}
			else
			{
				//cout << "找到 "<< found - p << endl;
				return found - p;
			}
		}
		string substr(size_t start_pos, size_t count)const
		{
			string str;
			for (size_t i = 0; i < count && start_pos + i < data_size; ++i)
			{
				if ('\0' == p[start_pos + i])break;
				str += p[start_pos + i];
			}
			return str;
		}
		void erase(size_t pos)
		{
			setSize(pos);
		}

		CBuffer() :p(NULL), buffer_size(0), data_size(0)
		{
			_init();
		}
		CBuffer(CBuffer const & tmp) :p(NULL), buffer_size(0), data_size(0)
		{
			_init();
			SetData(tmp.data(), (long)tmp.data_size);
		}
		CBuffer & operator=(CBuffer const & tmp)
		{
			_init();
			SetData(tmp.data(), (long)tmp.data_size);
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
			setSize((long)data_size + len);
			return true;
		}
		bool SetData(char const * sz)
		{
			setSize(0);
			return AddData((void *)sz, (long)strlen(sz));
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
	class CUnsignedBuffer
	{
	private:
		string name;
		unsigned char* p = NULL;
		size_t buffer_size = 0;
		size_t data_size = 0;
		bool bLockBuffer = false;

		void _init()
		{
			static int i = 0;
			char buf[64];
			sprintf_s(buf, 64, "CUnsignedBuffer%d", i++);
			name = buf;

			if (p)throw "p不为空指针";
			p = new unsigned char[1];
			if (!p)throw "内存不足";
			p[0] = '\0';
			buffer_size = 0;
		}
	public:
		CUnsignedBuffer()
		{
			_init();
		}
		CUnsignedBuffer(CUnsignedBuffer const& tmp)
		{
			_init();
			SetData(tmp.data(), (long)tmp.data_size);
		}
		CUnsignedBuffer& operator=(CUnsignedBuffer const& tmp)
		{
			_init();
			SetData(tmp.data(), (long)tmp.data_size);
			return *this;
		}
		~CUnsignedBuffer()
		{
			//cout << "~CUnsignedBuffer " << name <<" "<<(long)p<<" "<< buffer_size<<" "<<data_size << endl;
			if (p)delete[]p;
			data_size = 0;
			buffer_size = 0;
			p = NULL;
		}
		size_t capacity()const { return buffer_size; }
		size_t size()const { return data_size; }
		unsigned char const* data()const { return p; }
		unsigned char* lockBuffer()
		{
			bLockBuffer = true;
			return p;
		}
		void releaseBuffer() { bLockBuffer = false; }
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
				if (bLockBuffer)
				{
					cout << "缓冲区已锁定" << endl;
					throw "缓冲区已锁定";
				}
				//cout << name << " "<< "reserve "  << (long)p << " " << buffer_size << " " << data_size << endl;
				//cout << name<<"需要扩展 " << buffer_size << " -> " << n << endl;
				unsigned char* p2 = new unsigned char[n + 1];
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
					//cout << name<< "扩展成功 " << buffer_size << " -> " << n << endl;
					return true;
				}
				else
				{
					//cout << name<<"扩展失败 " << buffer_size << " -> " << n << endl;
					return false;
				}
			}
			return true;
		}
		bool AddData(void const* data, long len)
		{
			if (!reserve(data_size + len))return false;
			memmove(p + data_size, data, len);
			setSize((long)data_size + len);
			return true;
		}
		bool SetData(char const* sz)
		{
			setSize(0);
			return AddData((void*)sz, (long)strlen(sz));
		}
		bool SetData(unsigned char const* sz, long len)
		{
			return SetData((char const*)sz, len);
		}
		bool SetData(char const* sz, long len)
		{
			setSize(0);
			return AddData((void*)sz, len);
		}
		bool Compare(CUnsignedBuffer const& tmp)const
		{
			if (data_size != tmp.data_size)return false;
			if (data_size > 0)return 0 == memcmp(tmp.data(), data(), data_size);
			else return true;
		}
	};

}
