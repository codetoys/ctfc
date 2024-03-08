//sstring.h 用于共享内存的定长字符串缓冲区
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <cstring>
#include <string>

//定长字符数组模板，共享内存不能用string，char[]又不安全
template <long BUFSIZE >
class IShmActiveObject_sstring
{
private:
	char data[BUFSIZE];
	static void _copydata(char * buf, char const * source)
	{
		buf[BUFSIZE - 1] = '\0';
		long i = 0;
		for (; i < BUFSIZE - 1; ++i)
		{
			buf[i] = source[i];
			if ('\0' == source[i])break;
		}
		for (; i < BUFSIZE - 1; ++i)
		{
			buf[i] = '\0';
		}
	}
public:
	IShmActiveObject_sstring() { memset(data, 0, BUFSIZE); }
	~IShmActiveObject_sstring() { memset(data, 0, BUFSIZE); }
	IShmActiveObject_sstring(IShmActiveObject_sstring const & tmp) { _copydata(data, tmp.data); }
	IShmActiveObject_sstring(char const * psz) { _copydata(data, psz); }
	IShmActiveObject_sstring(std::string const & str) { _copydata(data, str.c_str()); }
	IShmActiveObject_sstring & operator=(IShmActiveObject_sstring const & tmp) { _copydata(data, tmp.data); return *this; }
	IShmActiveObject_sstring & operator=(char const * psz) { _copydata(data, psz); return *this; }
	IShmActiveObject_sstring & operator=(std::string const & tmp) { _copydata(data, tmp.c_str()); return *this; }

	long size()const { return strlen(data); }
	char const * c_str()const { return data; }
	char const & operator [](long off)const { return data[off]; }

	bool operator == (char const * tmp)const { return 0 == strcmp(data, tmp); }
	bool operator == (IShmActiveObject_sstring const & tmp)const { return 0 == strcmp(data, tmp.data); }
	bool operator == (std::string const & tmp)const { return 0 == strcmp(data, tmp.c_str()); }
	bool operator != (char const * tmp)const { return 0 != strcmp(data, tmp); }
	bool operator != (IShmActiveObject_sstring const & tmp)const { return 0 != strcmp(data, tmp.data); }
	bool operator != (std::string const & tmp)const { return 0 != strcmp(data, tmp.c_str()); }
	bool operator < (char const * tmp)const { return 0 > strcmp(data, tmp); }
	bool operator < (IShmActiveObject_sstring const & tmp)const { return 0 > strcmp(data, tmp.data); }
	bool operator < (std::string const & tmp)const { return 0 > strcmp(data, tmp.c_str()); }
	bool operator <= (char const * tmp)const { return 0 >= strcmp(data, tmp); }
	bool operator <= (IShmActiveObject_sstring const & tmp)const { return 0 >= strcmp(data, tmp.data); }
	bool operator <= (std::string const & tmp)const { return 0 >= strcmp(data, tmp.c_str()); }
	bool operator > (char const * tmp)const { return 0 < strcmp(data, tmp); }
	bool operator > (IShmActiveObject_sstring const & tmp)const { return 0 < strcmp(data, tmp.data); }
	bool operator > (std::string const & tmp)const { return 0 < strcmp(data, tmp.c_str()); }
	bool operator >= (char const * tmp)const { return 0 <= strcmp(data, tmp); }
	bool operator >= (IShmActiveObject_sstring const & tmp)const { return 0 <= strcmp(data, tmp.data); }
	bool operator >= (std::string const & tmp)const { return 0 <= strcmp(data, tmp.c_str()); }
};

template <long BUFSIZE >
inline std::ostream& operator <<(std::ostream & out, IShmActiveObject_sstring<BUFSIZE> const & data)
{
	out << data.c_str();
	return out;
}

template <long BUFSIZE >
inline std::istream & operator >>(std::istream & in, IShmActiveObject_sstring<BUFSIZE> & data)
{
	std::string tmp;

	in >> tmp;
	data = tmp;

	return in;
}
