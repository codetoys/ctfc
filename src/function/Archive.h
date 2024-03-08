//Archive.h 二进制序列化
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include "function.h"
#include "Buffer.h"

namespace ns_my_std
{
	class CMyArchive
	{
	public:
		CBuffer m_buf;

		void StartArchive(char const * name)
		{
			m_buf.setSize(0);
			m_buf.AddData((void *)name,strlen(name)+1);
		}
		//对各种类型的操作
		template<typename T> 
		CMyArchive & operator<<(T const & data)
		{
			data.MySerialize(*this);
			return *this;
		}
		template<typename T> 
		CMyArchive & Add(T const & data)
		{
			m_buf.AddData(&data,sizeof(T));
			return *this;
		}
		//对基本类型的特化，如果用到其它基本类型会导致“Serialize不支持”错误，需要在此增加
		template <long BUFSIZE >
		CMyArchive & operator<<(sstring<BUFSIZE> const & data) { m_buf.AddData(data.c_str(), data.size() + 1); return *this; }
		CMyArchive & operator<<(char const * data) { m_buf.AddData(data, strlen(data) + 1); return *this; }
		CMyArchive & operator<<(char * data) { m_buf.AddData(data, strlen(data) + 1); return *this; }
		CMyArchive & operator<<(string const & data){ m_buf.AddData((void *)data.c_str(), data.size() + 1); return *this; }
		CMyArchive & operator<<(long data) { return Add(data); }
		CMyArchive & operator<<(int data) { return Add(data); }
		CMyArchive & operator<<(unsigned long data) { return Add(data); }
		CMyArchive & operator<<(unsigned int data) { return Add(data); }
		CMyArchive & operator<<(bool data) { return Add(data); }
	};
	class CMyUnArchive
	{
	private:
		CBuffer const * m_pBuf;
		string::size_type m_pos;
		bool m_isOK;
	public:
		operator bool ()const{return m_isOK;}
		long getPos()const { return m_pos; }
		bool StartUnArchive(char const * name, CBuffer const * source)
		{
			m_pBuf=source;
			m_pos=0;
			m_isOK=true;

			if(0!=strcmp(name,m_pBuf->data()))
			{
				m_isOK=false;
			}
			m_pos+=strlen(m_pBuf->data())+1;
			return operator bool ();
		}
		
		//对各种类型的操作
		template<typename T>
		CMyUnArchive & operator>>(T & data)
		{
			if (!m_isOK)return *this;
			data.MyUnSerialize(*this);
			return *this;
		}
		template<typename T> 
		CMyUnArchive & Get(T & data)
		{
			if (m_pos >= m_pBuf->size())
			{
				thelog << "溢出 " << m_pBuf->size() << " " << m_pos << ende;
				m_isOK = false;
				return *this;
			}
			memcpy(&data, m_pBuf->data() + m_pos, sizeof(T));
			m_pos += sizeof(T);
			return *this;
		}
		//对基本类型的特化，如果用到其它基本类型会导致“UnSerialize不支持”错误，需要在此增加
		template <long BUFSIZE >
		CMyUnArchive & operator>>(sstring<BUFSIZE> & data)
		{
			if (m_pos >= m_pBuf->size())
			{
				thelog << "溢出 " << m_pBuf->size() << " " << m_pos << ende;
				m_isOK = false;
				return *this;
			}
			data = m_pBuf->data() + m_pos;
			m_pos += strlen(m_pBuf->data() + m_pos) + 1;//必须用原始数据计算长度
			return *this;
		}
		CMyUnArchive & operator>>(string & data) 
		{
			if (m_pos >= m_pBuf->size())
			{
				thelog << "溢出 " << m_pBuf->size() << " " << m_pos << ende;
				m_isOK = false;
				return *this;
			}
			data = m_pBuf->data() + m_pos;
			m_pos += strlen(m_pBuf->data() + m_pos) + 1;//必须用原始数据计算长度
			return *this;
		}
		CMyUnArchive & operator>>(long & data) { return Get(data); }
		CMyUnArchive & operator>>(int & data) { return Get(data); }
		CMyUnArchive & operator>>(unsigned long & data) { return Get(data); }
		CMyUnArchive & operator>>(unsigned int & data) { return Get(data); }
		CMyUnArchive & operator>>(bool & data) { return Get(data); }
	};

	class CMyArchiveTest
	{
	public:
		struct A
		{
			bool a;
			int b;
			long c;
			string d;
			sstring<16> f;
			void MySerialize(CMyArchive & ar)const
			{
				ar << a << b << c << d << f;
			}
			void MyUnSerialize(CMyUnArchive & unar)
			{
				unar >> a >> b >> c >> d >> f;
			}
		};
		static int CMyArchiveTest_doTest(int argc,char ** argv)
		{
			CMyArchive ar;
			A tmp;
			tmp.a=true;
			tmp.b=1;
			tmp.c=2;
			tmp.d="3";
			tmp.f="4";
			ar.StartArchive("a");
			ar<<tmp;

			CMyUnArchive unar;
			if(!unar.StartUnArchive("a",&ar.m_buf))return __LINE__;
			A tmp2;
			unar>>tmp2;
			thelog<<tmp2.a<<endi;
			thelog<<tmp2.b<<endi;
			thelog<<tmp2.c<<endi;
			thelog<<tmp2.d<<endi;
			thelog<<tmp2.f.c_str()<<endi;
			return 0;
		}
	};
}

