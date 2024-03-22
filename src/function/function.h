//function.h 小功能类
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

//如果使用HP编译器请定义“_HP_ACC”

/*
class ____ 内存占位符，用于内存分析
class CMyStringStream 带有复制构造函数的stringstream
class CMyTools 一组小功能
	TimeToString_log 日志时间格式
	TimeToString_Date 年月日格式
	TimeToString_Time 年月日时分秒格式
	mystrncpy 类似strncpy但会设置最后一个字符为字符串结束符
	longtocommastr 将long型转换为逗号分隔的字符串，1234567转换为“1,234,567”
	make_px 构造缩进前缀，即重复level次
	ToString 数据类型转字符串
class CStringSplit 字符串分解
template SArray 定长数组
template smap 用于共享内存的map
class CEasyFile 简单文件读写，基于完整内容读写
*/

#pragma once

#include "config.h"
#include "BitSet.h"
#include "Buffer.h"
#include "sstring.h"
#include <sys/time.h>

#define sstring IShmActiveObject_sstring

namespace ns_my_std
{
	class CSystemParam
	{
	public:
		static char const* SystemParam_GetString(char const*, char const*, char const* defaultvalue) { return defaultvalue; }
		static long SystemParam_GetLong(char const*, char const*, long defaultvalue) { return defaultvalue; }
	};
	//这个类是一个占位符，用于分析内存时快速发现
	class ____
	{
	public:
		char line[8];
		____() { memset(line, '-', 8); }
	};
	//stringstream没有复制构造函数，导致含有stringstream的类不能用于大多数容器
	class CMyStringStream : public stringstream
	{
	public:
		CMyStringStream() {}
		CMyStringStream(CMyStringStream const & tmp)
		{
			str(tmp.str());
		}
		CMyStringStream & operator=(CMyStringStream const & tmp)
		{
			str(tmp.str());
			return *this;
		}
	};
	class CMyTools
	{
	public:
		//字符串转时间,YYYYMMDDHHmmSS
		static time_t TimeStringToTime(char const* timestr)
		{
			char buf[15] = "00000000000000";//14个‘0’
			if (strlen(buf) != 14)throw "TimeStringToTime 的 buf 错误";
			if (strlen(timestr) > 14)memcpy(buf, timestr, 14);
			else
			{
				memcpy(buf, timestr, strlen(timestr));
			}
			tm t2;
			memset(&t2, 0, sizeof(tm));
			t2.tm_year = (buf[0] - '0') * 1000 + (buf[1] - '0') * 100 + (buf[2] - '0') * 10 + (buf[3] - '0') - 1900;
			t2.tm_mon = (buf[4] - '0') * 10 + (buf[5] - '0') - 1;
			t2.tm_mday = (buf[6] - '0') * 10 + (buf[7] - '0');
			t2.tm_hour = (buf[8] - '0') * 10 + (buf[9] - '0');
			t2.tm_min = (buf[10] - '0') * 10 + (buf[11] - '0');
			t2.tm_sec = (buf[12] - '0') * 10 + (buf[13] - '0');

			return mktime(&t2);
		}
		//字符串转时间,YYYY-MM-DD HH:mm:SS
		static time_t TimeStringToTime2(char const* timestr)
		{
			char buf[20] = "0000-00-00 00:00:00";//20个字符
			if (strlen(buf) != 19)throw "TimeStringToTime2 的 buf 错误";
			if (strlen(timestr) > 19)memcpy(buf, timestr, 19);
			else
			{
				memcpy(buf, timestr, strlen(timestr));
			}
			tm t2;
			memset(&t2, 0, sizeof(tm));
			t2.tm_year = (buf[0] - '0') * 1000 + (buf[1] - '0') * 100 + (buf[2] - '0') * 10 + (buf[3] - '0') - 1900;
			t2.tm_mon = (buf[5] - '0') * 10 + (buf[6] - '0') - 1;
			t2.tm_mday = (buf[8] - '0') * 10 + (buf[9] - '0');
			t2.tm_hour = (buf[11] - '0') * 10 + (buf[12] - '0');
			t2.tm_min = (buf[14] - '0') * 10 + (buf[15] - '0');
			t2.tm_sec = (buf[17] - '0') * 10 + (buf[18] - '0');

			return mktime(&t2);
		}
		//年月日时分秒
		static string TimeToTimeString(time_t const& t1)
		{
			tm const* t2;
			char buf[256];
			t2 = localtime(&t1);
			sprintf(buf, "%04d%02d%02d%02d%02d%02d", t2->tm_year + 1900, t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);
			return buf;
		}
		//日志用格式
		static string TimeToString_log(time_t const & t1)
		{
			if (0 == t1)return "";

			tm const * t2;
			char buf[256];
			t2 = localtime(&t1);
			sprintf(buf, "%02d-%02d %02d:%02d:%02d", t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);
			return buf;
		}
		//年月日
		static string TimeToString_Date(time_t const & t1)
		{
			if (0 == t1)return "";

			tm const * t2;
			char buf[256];
			t2 = localtime(&t1);
			sprintf(buf, "%04d%02d%02d", t2->tm_year + 1900, t2->tm_mon + 1, t2->tm_mday);
			return buf;
		}
		//年月日 时分秒
		static string TimeToString_Time(time_t const & t1)
		{
			if (0 == t1)return "";

			tm const * t2;
			char buf[256];
			t2 = localtime(&t1);
			sprintf(buf, "%04d%02d%02d %02d%02d%02d", t2->tm_year + 1900, t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);
			return buf;
		}
		static char * mystrncpy(char * buf, char const * source, long bufsize)
		{
			buf[bufsize - 1] = '\0';
			long i = 0;
			for (; i < bufsize - 1; ++i)
			{
				buf[i] = source[i];
				if ('\0' == source[i])break;
			}
			for (; i < bufsize - 1; ++i)
			{
				buf[i] = '\0';
			}
			return buf;
		}

		static string longtocommastr(long n)
		{
			char buf[256];
			string ret;
			int len;
			if (-1 == (len = sprintf(buf, "%ld", n)))return "";
			for (int i = 0; i < len; ++i)
			{
				ret += buf[i];
				if (i != len - 1 && 1 == (len - i) % 3)ret += ",";
			}
			return ret;
		}

		//构造前缀
		static string make_px(char const * px, long level)
		{
			string ret;
			for (long i = 0; i < level; ++i)
			{
				ret += px;
			}
			return ret;
		}

		//数据类型转字符串
		static string & ToString(long data, string & ret)
		{
			char buf[256];
			sprintf(buf, "%ld", data);
			return ret = buf;
		}
		//数据类型转字符串
		static string & ToString(string const & data, string & ret)
		{
			return ret = data;
		}
		//数据类型转字符串
		static string & ToString(char const * data, string & ret)
		{
			return ret = data;
		}
		static string ToString(long data)
		{
			char buf[256];
			sprintf(buf, "%ld", data);
			return buf;
		}
		static long to_long(string const & data)
		{
			return atol(data.c_str());
		}
		static long to_long(long const data)
		{
			return data;
		}
		static string const & to_string(string const & data)
		{
			return data;
		}
		static string to_string(long data)
		{
			char buf[256];
			sprintf(buf, "%ld", data);
			return buf;
		}
		template<long N>
		static string to_string(sstring<N> data)
		{
			return data.c_str();
		}
		template<long N>
		static string to_string(myBitSet<N>const & bm)
		{
			return bm.to_string();
		}

		//删除字符串两端空白
		static string& TrimAll(string& in)
		{
			string space = " \t\r\n";
			while (in.size() > 0 && space.find(in[0]) != string::npos)
			{
				in.erase(0, 1);
			}
			while (in.size() > 0 && space.find(in[in.size() - 1]) != string::npos)
			{
				in.erase(in.size() - 1, 1);
			}
			return in;
		}

		//SQL字符串转换
		static string SQLEncode(string const& in)
		{
			string out;
			for (size_t i = 0; i < in.size(); ++i)
			{
				if ('\'' == in[i])out += '\'';
				out += in[i];
			}
			return out;
		}
		static string ToHex(string const& data)
		{
			return ToHex(data.c_str(), data.size());
		}
		static string ToHex(char const* data, int len)
		{
			string ret;
			for (int i = 0; i < len; ++i)
			{
				char buf[16];
				sprintf(buf, "%02X", (unsigned char)data[i]);
				ret += buf;
			}
			return ret;
		}
		static int HexToBuffer(unsigned char const* hex, int hex_len, unsigned char* buf)
		{
			unsigned char const* src = hex;
			unsigned char* dst = buf;
			bool first = true;
			int outlen = 0;
			for (; *src; ++src)
			{
				//cout << src - hex << " " << dst - buf << " " << *src << endl;
				if (*src >= (unsigned char)'a' && *src <= (unsigned char)'z')
				{
					*dst = (*dst) * 16 + (*src - 'a' + 10);
				}
				else if (*src >= (unsigned char)'A' && *src <= (unsigned char)'Z')
				{
					*dst = (*dst) * 16 + (*src - 'A' + 10);
				}
				else if (*src >= (unsigned char)'0' && *src <= (unsigned char)'9')
				{
					*dst = (*dst) * 16 + (*src - '0');
				}
				else
				{
					continue;
				}
				if (!first)
				{
					++dst;
					*dst = '\0';
				}
				else
				{
					++outlen;
				}
				first = !first;
			}
			return outlen;
		}
		
		template<typename T>
		static string NumberToHex(T data)
		{
			string ret;
			for (size_t i = 0; i < sizeof(T); ++i)
			{
				char buf[16];
				sprintf(buf, "%02X", ((unsigned char*)((void*)&data))[sizeof(T) - 1 - i]);
				ret += buf;
			}
			return ret;
		}
		static long HexToLong(char const* hexstr)
		{
			long ret = 0;
			char const * p = hexstr;
			while (*p != '\0')
			{
				int x = 0;
				if (*p >= '0' && *p <= '9')x = *p - '0';
				else if (*p >= 'a' && *p <= 'z')x = *p - 'a' + 10;
				else if (*p >= 'A' && *p <= 'Z')x = *p - 'A' + 10;
				else;

				ret *= 16;
				ret += x;

				++p;
			}
			return ret;
		}
	};

	class CMyAssign
	{
	public:
		template <typename T1, typename T2>
		static T1& assign(T1& a, T2 const& b)
		{
			return a = b;
		}
		static long& assign(long& a, string const& b)
		{
			return a = atol(b.c_str());
		}
		static int& assign(int& a, string const& b)
		{
			return a = atol(b.c_str());
		}
		static short& assign(short& a, string const& b)
		{
			return a = atol(b.c_str());
		}
		static char& assign(char& a, string const& b)
		{
			return a = atol(b.c_str());
		}
		static unsigned long& assign(unsigned long& a, string const& b)
		{
			return a = strtoul(b.c_str(), nullptr, 0);
		}
		static unsigned int& assign(unsigned int& a, string const& b)
		{
			return a = strtoul(b.c_str(), nullptr, 0);
		}
		static unsigned short& assign(unsigned short& a, string const& b)
		{
			return a = strtoul(b.c_str(), nullptr, 0);
		}
		static unsigned char& assign(unsigned char& a, string const& b)
		{
			return a = strtoul(b.c_str(), nullptr, 0);
		}
	};

	class CStringSplit : public vector<string >
	{
	private:
		void add(string const & _str)
		{
			string str = _str;
			string blank;
			if (m_trim)blank += " \t";
			if (m_trimcrlf)blank += "\r\n";
			if (blank.size() != 0)
			{
				while (str.size() != 0 && blank.npos != blank.find(str[0]))str.erase(0, 1);
				while (str.size() != 0 && blank.npos != blank.find(str[str.size() - 1]))str.erase(str.size() - 1, 1);
			}
			if (str.size() != 0)push_back(str);
		}
		bool m_trim;
		bool m_trimcrlf;
	public:
		CStringSplit() :m_trim(true), m_trimcrlf(false)
		{
		}
		CStringSplit(char const* str, char const* sep, bool trim = true, bool trimcrlf = false) :m_trim(trim), m_trimcrlf(trimcrlf)
		{
			Set(str, sep, trim, trimcrlf);
		}
		void Set(char const* str, char const* sep, bool trim = true, bool trimcrlf = false)
		{
			clear();
			string tmp = str;
			string::size_type pos;
			while (tmp.npos != (pos = tmp.find(sep)))
			{
				add(tmp.substr(0, pos));
				tmp.erase(0, pos + strlen(sep));
			}
			add(tmp);
		}
	};

	//定长数组模板
	template <typename T, long CAPACITY >
	class SArray
	{
	private:
		long m_size;//数据长度
		bool isString;//数据是否是字符串，此时m_size包含了结束符
		T m_data[CAPACITY];
	public:
		SArray() :m_size(0), isString(false) {}
		~SArray() { m_size = 0; }

		long size()const { return m_size; }
		char const & operator [](long off)const { return m_data[off]; }
		void SetData(T const * source, long count)
		{
			m_size = 0;
			isString = false;

			long i;
			for (i = 0; i < count && m_size < CAPACITY; ++i, ++m_size)
			{
				m_data[m_size] = source[i];
			}
		}
		void SetString(char const * source)
		{
			if (sizeof(char) != sizeof(T))return;

			m_size = 0;
			isString = true;

			//不包括结束符，预留一个结束符位置
			char *p = (char *)m_data;
			long i;
			for (i = 0; source[i] != '\0' && m_size < CAPACITY - 1; ++i, ++m_size)
			{
				p[m_size] = source[i];
			}
			//补一个结束符
			p[m_size] = '\0';
			++m_size;
		}
		string GetString()const
		{
			return (char const *)m_data;
		}

		string & toString(string & ret)const
		{
			if (isString)
			{
				ret = (char *)m_data;
			}
			else
			{
				char buf[256];
				sprintf(buf, "size:%ld", m_size);
				ret = buf;
			}
			return ret;
		}
	};

	//用于平面内存的map，注意最后多一个以供存放溢出的，因为很多代码不检查溢出
	template <long MAX_COUNT, typename KEY, typename VALUE >
	class smap
	{
	private:
		struct ITEM
		{
			KEY first;
			VALUE second;
		};
		ITEM m_datas[MAX_COUNT + 1];
		long count;
	public:
		smap() :count(0) {}
		typedef ITEM * iterator;
		typedef ITEM const * const_iterator;

		iterator begin()
		{
			return &m_datas[0];
		}
		iterator end()
		{
			return &m_datas[count];
		}
		const_iterator begin()const
		{
			return &m_datas[0];
		}
		const_iterator end()const
		{
			return &m_datas[count];
		}
		iterator find(KEY const & key)
		{
			iterator it;
			for (it = begin(); it != end(); ++it)
			{
				if (key == it->first)
				{
					return it;
				}
			}
			return end();
		}
		iterator insert(KEY const & key, VALUE const & value)
		{
			iterator it = find(key);
			if (it != end())
			{
				it->second = value;
				return it;
			}
			else
			{
				//如果是溢出，返回的值任然可以暂时使用，但是会被下一个溢出冲掉
				it->first = key;
				it->second = value;
				if (MAX_COUNT == count)
				{
					return end();
				}
				else
				{
					++count;
					return it;
				}
			}
		}
	};

	//简单文件读写，用于将简单数据保存至特定的文件
	class CEasyFile
	{
	private:
		string m_msg;
	public:
		string const & getMsg()const { return m_msg; }
		static long GetFileSize(char const* filename)
		{
			FILE* file = fopen(filename, "r");
			if (NULL == file)return -1;

			long file_size = -1;
			if (fseek(file, 0, SEEK_END) != 0)return -1;
			file_size = ftell(file);	// 获取此时偏移值，即文件大小
			fclose(file);
			return file_size;
		}
		bool DeleteFile(char const* filename)
		{
			if (0 != remove(filename))
			{
				m_msg = "删除错误";
				return false;
			}
			return true;
		}
		bool IsFileExist(char const* filename)
		{
			if (0 != access(filename, F_OK))
			{
				m_msg = "删除错误";
				return false;
			}
			return true;
		}
		bool RenameFile(char const* from,char const* to)
		{
			if (0 != rename(from, to))
			{
				m_msg = "重命名错误";
				return false;
			}
			return true;
		}
		bool ReadFile(char const * filename, string & filedata)
		{
			CBuffer buffer;

			if (ReadFile(filename, buffer))
			{
				filedata = buffer.data();
				return true;
			}
			else
			{
				return false;
			}
		}
		//读取文件所有数据，不建议用于大文件
		bool ReadFile(char const * filename, CBuffer & filedata)
		{
			FILE * file;
			file = fopen(filename, "r");
			if (NULL == file)
			{
				m_msg = "未能打开文件";
				return false;
			}
			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			if (size > 1024 * 1024 * 1024)
			{
				m_msg = "文件太大";
				fclose(file);
				return false;
			}
			fseek(file, 0, SEEK_SET);//回到开始位置
			size_t tmp;
			filedata.reserve(size);
			if (size != (tmp = fread(filedata.getbuffer(), 1, size, file)))
			{
				m_msg = "读取错误";
				fclose(file);
				return false;
			}
			filedata.setSize(size);
			if (0 != fclose(file))
			{
				m_msg = "关闭文件错误错误";
				return false;
			}
			return true;
		}
		//读不到足够数据则失败
		bool ReadFile(char const * filename, char * buffer, long buffersize)
		{
			FILE * file;
			file = fopen(filename, "r");
			if (NULL == file)
			{
				m_msg = "未能打开文件";
				return false;
			}
			if (1 != fread(buffer, buffersize, 1, file))
			{
				m_msg = "读取错误";
				fclose(file);
				return false;
			}
			if (0 != fclose(file))
			{
				m_msg = "关闭文件错误错误";
				return false;
			}
			return true;
		}
		bool WriteFile(char const * filename, long filedata)
		{
			char buf[256];
			sprintf(buf, "%ld", filedata);
			return WriteFile(filename, buf);
		}
		bool WriteFile(char const * filename, char const * filedata)
		{
			return WriteFile(filename, filedata, strlen(filedata));
		}
		bool WriteFile(char const* filename, char const* filedata, long datasize)
		{
			return WriteFile(filename, 0, filedata, strlen(filedata), true);
		}
		bool WriteFile(char const* filename, long seek, char const* filedata, long datasize, bool trunc)
		{
			FILE* file;
			char const* mode = "w";
			if (!trunc)mode = "a";

			file = fopen(filename, mode);
			if (NULL == file)
			{
				m_msg = "未能打开文件";
				return false;
			}
			size_t size = datasize;
			fseek(file, seek, SEEK_SET);
			if (ftell(file) != seek)
			{
				char buf[256];
				sprintf(buf, "定位错 %ld %ld", seek, ftell(file));
				m_msg = buf;
				return false;
			}
			if (size != fwrite(filedata, 1, size, file))
			{
				m_msg = "写入错误";
				return false;
			}
			if (0 != fclose(file))
			{
				m_msg = "关闭文件错误错误";
				return false;
			}
			return true;
		}
	};

	class CMyTime
	{
	private:
		timeval t;
	public:
		CMyTime()
		{
			clear();
		}
		void clear() { t.tv_sec = 0; t.tv_usec=0; }
		bool hasTime()const { return 0 != t.tv_sec || 0 != t.tv_usec; }
		//设置为当前时间
		void SetCurrentTime()
		{
			gettimeofday(&t, NULL);
		}
		//获取经过的微秒数
		int64_t GetTimeSpanUS()const
		{
			timeval now;
			gettimeofday(&now, NULL);
			return ((int64_t)now.tv_sec - t.tv_sec) * 1000000 + ((int64_t)now.tv_usec - t.tv_usec);
		}
		//获取经过的毫秒数
		int64_t GetTimeSpanMS()const
		{
			return GetTimeSpanUS() / 1000;
		}
		//获取经过的秒数
		int64_t GetTimeSpanS()const
		{
			return GetTimeSpanMS() / 1000;
		}
	
		int64_t GetTS_s()const
		{
			return GetTS_ms() / 1000;
		}
		int64_t GetTS_ms()const
		{
			return GetTS_us() / 1000;
		}
		int64_t GetTS_us()const
		{
			return (int64_t)t.tv_sec * 1000*1000 + t.tv_usec;
		}
	};
}

