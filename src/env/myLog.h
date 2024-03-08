//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../function/config.h"
#include "../function/function.h"
#include "../function/mymutex.h"

namespace ns_my_std
{
	//调试开关
#define G_IS_DEBUG (*g_pActiveAppEnv->pIsDebug)

	//错误码，仅供原始出错位置设置，仅当函数使用此错误码方可使用
	//除非使用了G_SET_ERROR或G_CLEAR_ERROR，错误信息将在一定范围内累加
#define G_IS_ERROR (g_pActiveAppEnv->is_error())
#define G_SET_ERROR(n) g_pActiveAppEnv->set_error((n));thelog //先设置错误代码，然后输出错误信息，注意必须用ende结尾。该操作清除原有错误信息
	stringstream & G_ERROR_MESSAGE();//返回stringstream的引用，用于输入错误信息，但一般不需要使用，错误信息用thelog/ende即可
#define G_GET_ERROR (g_pActiveAppEnv->get_error())
#define G_CLEAR_ERROR g_pActiveAppEnv->clear_error();//该操作将清除原有错误信息

	//输出信息所在位置的日志宏
#define thelog ((g_pActiveAppEnv->GetLog()).LogPos(__FILE__,__LINE__,__func__))
#define theLog (g_pActiveAppEnv->GetLog())

//输出调试信息的宏
#define DEBUG_LOG if(G_IS_DEBUG)thelog

	//////////////////////////////////////////////////
	//日志

	struct LogEnd
	{
		enum { fENDE = 1, fENDW, fENDI, fENDF, fENDD };
		char end;
		LogEnd(int ,int n):end(n){}
	};
	ostream & operator<<(ostream & s, LogEnd const & end);
	LogEnd const ende(0, LogEnd::fENDE); // error
	LogEnd const endw(0, LogEnd::fENDW); // warning
	LogEnd const endi(0, LogEnd::fENDI); // information
	LogEnd const endf(0, LogEnd::fENDF); // fatal
	LogEnd const endd(0, LogEnd::fENDD); // debug

	class Log
	{
	private:
		//线程数据
		struct _ThreadSpec
		{
			long m_thread_id;
			stringstream m_buf;
			string m_strSource;
			bool m_bOutputThis;//仅控制当前一行输出
			string m__file;
			long m__line;
			string m__func;
			_ThreadSpec() :m_thread_id(0), m_bOutputThis(true), m__line(0) {}
		};
		static void _CloseThreadSpec(void * _p)
		{
			_ThreadSpec * p=(_ThreadSpec *)_p;
			delete p;
		}
		_ThreadSpec * _getThreadSpec()
		{
			_ThreadSpec * p = (_ThreadSpec *)pthread_getspecific(tsd_key);
			if (p)return p;
			else
			{
				p = new _ThreadSpec;
				if (!p)
				{
					throw "new _ThreadSpec return NULL";
				}
				m_mutex.lock();
				p->m_thread_id = m_thread_count;
				++m_thread_count;
				m_mutex.unlock();
				if (0 == pthread_setspecific(tsd_key, p))
				{
					return p;
				}
				else
				{
					throw "pthread_setspecific error";
				}
			}
		}
	private:
		long m_thread_count;//线程计数
		pthread_key_t tsd_key;//线程存储key	
		CPThreadMutex m_mutex;
		string m_appname;//用来构造m_filename，用于定时切换文件，若未设置则直接使用m_filename
		string m_filename;//当前的日志文件名
		ofstream m_ofs;
		bool m_bOutput;//控制所有输出
		bool m_bCache;//当!m_bOutput时缓存输出结果
		string::size_type m_maxcache;//最大缓存的数目
		string m_cache;//缓存的输出
		long max_log_size = 0;//最大日志文件大小，用于存储有限的设备，0为不限制，负值为按日期命名

		void(*m_pUD)(const string& strLog);

		//错误和警告统计
		long countw;
		long counte;
		long countall;
		//正常记录统计
		long countn;
	public:
		void SetUserFunction(void(*pUD)(const string& strLog)) { m_pUD = pUD; }
	public:
		Log()
		{
			m_thread_count = 0;
			pthread_key_create(&tsd_key, _CloseThreadSpec);
			m_mutex.init();
			SetSource("应用");
			m_bOutput = true;
			m_bCache = false;
			m_maxcache = 0;
			m_pUD = NULL;

			countw = 0;
			counte = 0;
			countn = 0;
			countall = 0;
		}
		int ActiveClone(Log const & old)
		{
			m_bOutput = old.m_bOutput;
			m_bCache = old.m_bCache;
			m_maxcache = old.m_maxcache;
			m_pUD = old.m_pUD;

			return 0;
		}

		void GetCount(long & c_w, long & c_e, long & c_all)const { c_w = countw; c_e = counte; c_all = countall; }
		string const &GetFileName()const { return m_filename; }
		Log& LogPos(char const * file, long line,char const * func)
		{
			string tmp = file;
			string::size_type pos = tmp.find_last_of("/");
			if (pos != tmp.npos)_getThreadSpec()->m__file = tmp.substr(pos + 1);
			else _getThreadSpec()->m__file = file;

			_getThreadSpec()->m__line = line;
			_getThreadSpec()->m__func = func;
			return *this;
		}

		string const & _makesource(string const & source, string & ret);
		template <typename T>
		Log& operator<<(T const& t)
		{
			_getThreadSpec()->m_buf << (t);
			return *this;
		}
		Log& operator<<(stringstream const& ss)
		{
			_getThreadSpec()->m_buf << ss.str();
			return *this;
		}
		Log & operator<<(ostream &(*p)(ostream &))
		{
			_getThreadSpec()->m_buf <<(p);
			return *this;
		}
		Log& operator<<(LogEnd const& end);

		Log& SetSource(const string& strSource)
		{
			_getThreadSpec()->m_strSource = "[" + strSource + "]";
			return *this;
		}
		void setMaxFileSize(long n)
		{
			max_log_size = n;
		}
		void setCache(string::size_type maxcache)
		{
			m_maxcache = maxcache;
			m_bCache = (maxcache > 0);
		}
		bool getCache()const { return m_bCache; }
		string & GetCachedLog(string & ret)//获得缓存的日志并清理缓存
		{
			ret = m_cache;
			m_cache.clear();
			return ret;
		}
		void ClearCache()//结束缓存，丢弃缓存的东西
		{
			m_cache.clear();
			setCache(0);
		}

		void setOutput(bool bEnable = true) { m_bOutput = bEnable; }
		bool getOutput() { return m_bOutput; }
		Log & setOutputThis(bool bEnable = false) { _getThreadSpec()->m_bOutputThis = bEnable; return *this; }
#ifdef _LINUXOS
		int _Open(const string& strFile, std::_Ios_Openmode nMode = ios::out | ios::app)
#else
		int _Open(const string& strFile, int nMode = ios::out | ios::app)
#endif
		{
			m_ofs.close();
			m_ofs.clear();
			m_ofs.open(strFile.c_str(), nMode);
			if (!m_ofs.good())
			{
				cout << "打开文件出错 " << strFile << " " << strerror(errno) << endl;
				return -1;
			}
			m_filename = strFile;

			return 1;
		}
		//以固定文件方式打开日志，不会根据日期切换
		int Open(char const * filename)
		{
			cout << "theLog.Open(filename) 此操作将取消按日期生成日志文件的功能，若要使用按日期分文件请取消此调用" << endl;
			m_appname = "";
			return _Open(filename);
		}
		//根据当前年月日构造日志文件名，若未设置m_appname则为m_filename
		string makelogfilename()
		{
			if (0 == m_appname.size())return m_filename;
			if (max_log_size >= 0)return m_appname + ".log";
			time_t t;
			tm const * t2;
			char buf[256];
			time(&t);
			t2 = localtime(&t);
			sprintf(buf, "%s.%04d%02d%02d.log", m_appname.c_str(), t2->tm_year + 1900, t2->tm_mon + 1, t2->tm_mday);
			return buf;
		}
		//按日期打开日志，日期改变日志文件自动切换，格式为“appname.YYYYMMDD.log”
		int ActiveOpen(char const * appname, long _max_log_size)
		{
			m_appname = appname;
			max_log_size = _max_log_size;
			return _Open(makelogfilename());
		}
		//获得当前日志位置
		long tellEndP()
		{
			m_ofs.seekp(0, ios::end);
			return m_ofs.tellp();
		}
		//返回当前日志记录数
		long getCountN()
		{
			return countn;
		}
		long getCountW()
		{
			return countw;
		}
		long getCountE()
		{
			return counte;
		}
	};
	//日志接口
#define LOG (thelog)
#define ENDI (endi)
#define ENDW (endw)
#define ENDE (ende)
}

