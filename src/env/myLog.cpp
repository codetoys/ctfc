//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "stdafx.h"
#include "myLog.h"
#include "myUtil.h"

namespace ns_my_std
{
	ostream & operator<<(ostream & s, LogEnd const & end)
	{
		switch (end.end)
		{
		case LogEnd::fENDI:
			s << "[信息]";
			break;
		case LogEnd::fENDW:
			s << "[警告]";
			break;
		case LogEnd::fENDE:
			s << "[出错]";
			break;
		case LogEnd::fENDF:
			s << "[致命]";
			break;
		case LogEnd::fENDD:
			s << "[调试]";
			break;
		}
		s << endl;
		return s;
	}

	string const & Log::_makesource(string const & source, string & ret)
	{
		if (m_thread_count < 2)return source;

		_ThreadSpec * ThreadSpec = _getThreadSpec();
		char buf[256];
		if (0 == ThreadSpec->m_thread_id)
		{
			sprintf(buf, "[%lu]", (unsigned long)getpid());
		}
		else
		{
			sprintf(buf, "[%lu-%2ld]", (unsigned long)getpid(), ThreadSpec->m_thread_id);
		}
		return ret = source + buf;
	}
	Log& Log::operator<<(LogEnd const& end)
	{
		m_mutex.WLock();
		_ThreadSpec* ThreadSpec = _getThreadSpec();
		char nCh = end.end;
		bool isImportant = false;

		string strType;

		switch (nCh)
		{
		case LogEnd::fENDI:
			strType = "[信息]";
			++countn;
			break;
		case LogEnd::fENDW:
			strType = "[警告]";
			isImportant = true;
			++countw;
			break;
		case LogEnd::fENDE:
			strType = "[出错]";
			isImportant = true;
			++counte;
			break;
		case LogEnd::fENDF:
			strType = "[致命]";
			isImportant = true;
			break;
		case LogEnd::fENDD:
			strType = "[调试]";
			break;
		}
		if (isImportant)++countall;

		time_t t;
		tm const* t2;
		char buf[2048];
		time(&t);
		t2 = localtime(&t);
		sprintf(buf, "%02d-%02d %02d:%02d:%02d", t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);

		string strTime = buf;

		strTime = "[" + strTime + "]";

		//若未设置则不输出文件名和行号
		if (0 != ThreadSpec->m__file.size())
		{
			sprintf(buf, "[%-24s:%4ld(%s)][%6.2f]", ThreadSpec->m__file.c_str()
				, ThreadSpec->m__line, ThreadSpec->m__func.c_str(), (clock() / (float)CLOCKS_PER_SEC));
		}
		else buf[0] = '\0';

		string tmpSource;
		string strMsg;
		strMsg = strTime + _makesource(ThreadSpec->m_strSource, tmpSource) + strType + buf + ThreadSpec->m_buf.str();

		if (LogEnd::fENDE == nCh)
		{
			if (G_ERROR_MESSAGE().str().size() > 1024 * 1024)G_ERROR_MESSAGE().str(G_ERROR_MESSAGE().str().substr(512 * 1024));
			G_ERROR_MESSAGE() << strMsg << endl;
		}

		if (!m_bCache && m_filename.size() != 0)
		{
			if (max_log_size >= 0)
			{
				if (max_log_size > 0)
				{
					//压缩比高达98%，备份文件所占的空间可以忽略不计
					if (m_ofs.tellp() > max_log_size)
					{
						m_ofs.close();
						string bakfile = m_filename + ".log";
						remove(bakfile.c_str());//删除旧的备份文件
						rename(m_filename.c_str(), bakfile.c_str());//改名
						string zipcmd = "bzip2 -fz " + bakfile;//强制覆盖
						system(zipcmd.c_str());
						_Open(m_filename);//重新打开日志文件（此时会创建）
					}
				}
			}
			else
			{
				string newfile = makelogfilename();
				if (m_filename != newfile)
				{
					m_ofs.close();
					_Open(newfile);
					cout << "文件切换，原文件 " << m_filename << " 新文件 " << newfile << endl;
					m_ofs << "文件切换，原文件 " << m_filename << " 新文件 " << newfile << endl;
				}
			}
			m_ofs << strMsg.c_str() << endl;
			m_ofs.flush();
			if (m_ofs.bad())
			{
				m_ofs.close();
				_Open(m_filename);
				cout << "写文件错误，关闭后重新打开文件" << endl;
				m_ofs << "写文件错误，关闭后重新打开文件" << endl;
				m_ofs << strMsg.c_str() << endl;
				m_ofs.flush();
			}
		}

		if (m_bOutput && ThreadSpec->m_bOutputThis)
		{
			cout << strMsg.c_str() << endl;
		}
		else
		{
			if (m_bCache)
			{
				if (m_cache.size() > m_maxcache)
				{
					m_cache.erase(0, m_cache.size() / 2);//超长时删去前半部分
				}
				m_cache += strMsg + "\n";
			}
		}
		ThreadSpec->m_bOutputThis = true;

		if (m_pUD) (*m_pUD)(strMsg); // 用户定义功能

		ThreadSpec->m__file = "";
		ThreadSpec->m__line = 0;
		ThreadSpec->m_buf.str("");

		m_mutex.WUnLock();
		return *this;
	}

}
