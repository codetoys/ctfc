//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <errno.h>
#ifdef _LINUXOS
#include <typeinfo>
#endif

#include "../function/mysocket.h"
#include "../function/htmldoc.h"
#include "../function/mimetype.h"

#define COPY_RIGHT "2004-2016"

namespace ns_my_std
{
	//HTTP时间
	class CHttpDate
	{
	public:
		//构造时间,t1+seconds
		static string HttpDate(time_t t1, long seconds)
		{
			char weekday[7][4] = { "Sun","Mon","Tue","Wes","Thu","Fri","Sat" };
			char month[12][4] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
			tm * t2 = gmtime(&t1);
			char buf[256];
			sprintf(buf, "%s, %02d %s %04d %02d:%02d:%02d GMT", weekday[t2->tm_wday], t2->tm_mday, month[t2->tm_mon], t2->tm_year + 1900, t2->tm_hour, t2->tm_min, t2->tm_sec);
			return buf;
		}
	};

	//HTTP请求
	class CHttpRequest
	{
	public:
		typedef vector<pair<string, string > > PARAMLIST_T;
		static bool AnalyzeParam(string const & params, PARAMLIST_T & ret)
		{
			ret.clear();
			if (0 == params.size())return true;

			string tmpparams = params + "&";
			string::size_type pos;
			while (tmpparams.npos != (pos = tmpparams.find("&")))
			{
				string str = tmpparams.substr(0, pos);
				string::size_type pos2;
				if (str.npos != (pos2 = str.find("=")))
				{
					ret.push_back(pair<string, string >(CHtmlDoc::URLDecode(str.substr(0, pos2)), CHtmlDoc::URLDecode(str.substr(pos2 + 1))));
				}
				tmpparams.erase(0, pos + 1);
			}
			return true;
		}
	private:
		bool m_isCgi;//是否是CGI
		CBuffer m_fullrequest;//完整请求,包含了状态行、头标
		long m_contentlength;//内容长度
		string m_method;//请求方法，GET、POST等
		string m_resource;//请求资源，/dir/file，不包括QueryString
		string m_querystring;//查询字符串
		PARAMLIST_T m_params;//参数表，QueryString，Form Data
		map<string, string > m_cookies;
		string m_content_type;//内容类型
		CBuffer m_content;//内容

		//出错或需要退出返回false，seconds为0不超时
		bool _Recv(CMySocket & s, long seconds, char * buf, int buflen, long * pReadCount, bool(*pfNeedBrek)() = NULL)
		{
			bool isReady = false;
			if (!s.IsSocketReadReady2(seconds, isReady, pfNeedBrek))
			{
				LOG << "socket error" << ENDE;
				return false;
			}
			if (!isReady)
			{
				DEBUG_LOG << "socket timeout" << ENDI;
				return false;
			}
			else
			{
				DEBUG_LOG << "socket ready" << ENDI;
			}

			return s.Recv(buf, buflen, pReadCount);
		}
		bool AnalyzeParam(string const & params)
		{
			return AnalyzeParam(params, m_params);
		}
		void AnalyzeCookie(string const & request)
		{
			string COOKIE = "Cookie: ";
			string::size_type pos_start;
			string::size_type pos_end;

			pos_start = request.find(COOKIE);
			if (request.npos == pos_start)return;
			pos_end = request.find("\r\n", pos_start);
			if (request.npos == pos_end)return;
			string str = request.substr(pos_start + COOKIE.size(), pos_end - (pos_start + COOKIE.size()));
			CStringSplit st(str.c_str(), ";");
			for (CStringSplit::iterator it = st.begin(); it != st.end(); ++it)
			{
				CStringSplit st2(it->c_str(), "=");
				if (st2.size() != 2)continue;
				m_cookies[st2[0]] = st2[1];
			}
		}
	public:
		void Clear()
		{
			m_isCgi = false;
			m_fullrequest.setSize(0);
			m_contentlength = 0;
			m_method = "";
			m_resource = "";
			m_querystring = "";
			m_params.clear();
			m_cookies.clear();
			m_content_type = "";
			m_content.setSize(0);
		}
		map<string, string > const & GetCookies()const { return m_cookies; }
		string const & GetResource()const { return m_resource; }//获得请求资源，/dir/file，不包括QueryString
		string const & GetQueryString()const { return m_querystring; }//获得QueryString
		string const & GetMethod()const { return m_method; }//获得请求方法
		string const & GetContentType()const { return m_content_type; }//获得内容类型
		CBuffer const & GetContent()const { return m_content; }//获得内容
		//获得请求资源的后缀名
		string GetResourceType()const
		{
			long pos = m_resource.size() - 1;
			while (pos >= 0 && m_resource[pos] != '.' && m_resource[pos] != '/')--pos;
			if (pos < 0 || m_resource[pos] == '/')return "";
			return m_resource.substr(pos + 1);
		}
		//获得请求资源的文件名
		string GetResourceFileTitle()const
		{
			size_t start = m_resource.find_last_of('/');
			size_t end = m_resource.find_last_of('.');
			if (start != m_resource.npos && end != m_resource.npos)
			{
				if (end > start + 1)return m_resource.substr(start + 1, end - start - 1);
			}
			return "";
		}
		string GetFullRequest()const { return m_fullrequest.data(); }//获得完整请求
		long GetContentLength()const { return m_contentlength; }//获得contentlength
		string const & GetParam(string const & param)const//获得参数，如果参数有重复则只能取得第一个
		{
			STATIC_C string const static_str;
			string const * p = GetParam(param, m_params);
			if (NULL != p)return *p;
			else return static_str;
		}
		static string const * GetParam(string const & param, PARAMLIST_T const & params)//获得参数，如果参数有重复则只能取得第一个
		{
			string _param = param;
			for (string::size_type i = 0; i < params.size(); ++i)
			{
				string _first = params[i].first;
				if (0 == stricmp(_param.c_str(), _first.c_str()))return &params[i].second;
			}
			return NULL;
		}
		string GetCookie(string const & cookie)const
		{
			map<string, string >::const_iterator it = this->m_cookies.find(cookie);
			if (it != this->m_cookies.end())return it->second;
			else return "";
		}
		string GetHeader(char const * _header)const
		{
			if (m_isCgi)
			{
				//获得HTTP头标，实际上是从环境变量中直接获取
				char const * penv = getenv(_header);
				if (NULL != penv)return penv;
				else return "";
			}
			else
			{
				string header = _header;
				header += ": ";
				string::size_type pos_start;
				string::size_type pos_end;

				if (m_fullrequest.npos == (pos_start = m_fullrequest.find(header)))return "";
				if (m_fullrequest.npos == (pos_end = m_fullrequest.find("\r\n", pos_start)))return "";
				string ret = m_fullrequest.substr(pos_start + header.size(), pos_end - pos_start - header.size());
				return Trim(ret);
			}
		}
		//替换Header
		static string & ReplaceHeader(string const & oldrequest, char const * _header, long _value, string & newrequest)
		{
			char buf[256];
			sprintf(buf, "%ld", _value);
			return ReplaceHeader(oldrequest, _header, buf, newrequest);
		}
		//替换Header
		static string & ReplaceHeader(string const & oldrequest, char const * _header, char const * _value, string & newrequest)
		{
			newrequest = oldrequest;
			string header = _header;
			header += ": ";
			string::size_type pos_start;
			string::size_type pos_end;

			if (newrequest.npos != (pos_start = newrequest.find(header)))
			{
				if (newrequest.npos != (pos_end = newrequest.find("\r\n", pos_start)))
				{
					newrequest.replace(pos_start + header.size(), pos_end - pos_start - header.size(), _value);
					return newrequest;
				}
				else
				{
					LOG << "格式错误，没有行结束符" << ENDE;
					return newrequest = "";
				}
			}
			else
			{
				if (newrequest.npos != (pos_start = newrequest.find("\r\n")))
				{
					newrequest.insert(pos_start + strlen("\r\n"), header + _value + "\r\n");
					return newrequest;
				}
				else
				{
					LOG << "格式错误，没有行结束符" << ENDE;
					return newrequest = "";
				}
			}
		}
		bool GetAuthorization(string & user, string & password)const
		{
			string AUTHORIZATION = "Authorization: Basic ";
			string::size_type pos_start;
			string::size_type pos_end;
			string base64;

			if (m_fullrequest.npos == (pos_start = m_fullrequest.find(AUTHORIZATION)))return false;
			if (m_fullrequest.npos == (pos_end = m_fullrequest.find("\r\n", pos_start)))return false;
			base64 = m_fullrequest.substr(pos_start + AUTHORIZATION.size(), pos_end - pos_start - AUTHORIZATION.size());

			char buf[2048];
			int len;
			if (0 > (len = CBase64::Base64Dec(buf, base64.c_str(), (int)base64.size())))
			{
				LOG << "base64解码错误" << ENDE;
			}
			buf[len] = '\0';
			DEBUG_LOG << buf << ENDI;
			CStringSplit st(buf, ":");
			if (st.size() != 2)return false;
			user = st[0];
			password = st[1];
			//thelog << pos_start << " " << pos_end << " " << base64 << endi;
			return true;
		}
		PARAMLIST_T GetParamList(PARAMLIST_T & params)const { return params = m_params; }//获得参数表，<参数名，参数值>数组
		//接收请求，此调用成功后才能调用GetXXXX
		//pFullRequest不为空直接从pFullRequest中分析而不需要从s接收
		bool RecvRequest(CMySocket & s, CBuffer * pFullRequest = NULL, bool(*pfNeedBrek)() = NULL)
		{
			Clear();
			m_isCgi = false;
			long timeout = 300;//接收超时

			string::size_type headlen = 0;

			char buf[1024];
			long count;
			string::size_type pos;

			if (NULL != pFullRequest)m_fullrequest.AddData(pFullRequest->data(), pFullRequest->size());

			//先接收请求头
			while (m_fullrequest.npos == (headlen = m_fullrequest.find("\r\n\r\n")))
			{
				if (!_Recv(s, timeout, buf, 1023, &count, pfNeedBrek))
				{
					//LOG<<"recv error"<<ENDE;
					s.Close();
					return false;
				}
				if (0 == count)
				{
					//LOG<<"client closed"<<ENDE;
					s.Close();
					return false;
				}
				buf[count] = '\0';
				//thelog << endl << buf << endi;
				m_fullrequest.AddData(buf, count);
			}
			//获得第一行
			pos = m_fullrequest.find("\r\n");
			string str;
			str = m_fullrequest.substr(0, pos);
			//获得method
			pos = str.find(" ");
			if (str.npos == pos)
			{
				return false;
			}
			this->m_method = str.substr(0, pos);
			str.erase(0, pos + 1);
			//去掉http版本
			pos = str.find_last_of(" ");
			if (str.npos == pos)
			{
				return false;
			}
			str.erase(pos);
			//剩下的是URL，问号之后的是GET参数
			pos = str.find_first_of("?");
			if (str.npos != pos)
			{
				this->m_resource = str.substr(0, pos);
				str.erase(0, pos + 1);
				m_querystring = str;
			}
			else
			{
				this->m_resource = str;
			}

			//分析Cookie
			AnalyzeCookie(m_fullrequest.data());

			//如果存在请求内容则接收请求内容
			pos = m_fullrequest.find("Content-Length:");
			if (m_fullrequest.npos == pos || pos >= headlen)
			{
				m_contentlength = 0;
				m_content.setSize(0);
			}
			else
			{
				m_contentlength = atol(m_fullrequest.data() + pos + strlen("Content-Length:"));
				//thelog << "m_contentlength " << m_contentlength << endi;
				//thelog << "headlen " << headlen << endi;
				m_content.AddData(m_fullrequest.data() + headlen + strlen("\r\n\r\n"), m_fullrequest.size() - headlen - strlen("\r\n\r\n"));
				m_fullrequest.erase(headlen + strlen("\r\n\r\n"));
				while ((long)m_content.size() < m_contentlength)
				{
					if (!_Recv(s, timeout, buf, 1023, &count, pfNeedBrek))
					{
						LOG << "recv error" << ENDE;
						s.Close();
						return false;
					}
					if (0 == count)
					{
						LOG << "client closed" << ENDE;
						s.Close();
						return false;
					}
					buf[count] = '\0';
					m_content.AddData(buf, count);
				}
			}

			//根据内容类型处理数据
			m_content_type = GetHeader("Content-Type");
			Trim(m_content_type);
			if ("application/x-www-form-urlencoded" == m_content_type)
			{
				if (m_querystring.size() != 0)m_querystring += "&";
				m_querystring += m_content.data();
			}
			else if (isContentTypeXml(m_content_type))
			{
				DEBUG_LOG << "content is xml" << ENDI;
			}
			else
			{
			}
			this->AnalyzeParam(m_querystring);
			return true;
		}
		static bool isContentTypeXml(string const & content_type)
		{
			return "Text/xml" == content_type;
		}
		bool InitCgiRequest(int argc, char ** argv)
		{
			Clear();
			m_isCgi = true;
			string tmp;

			thelog << "argc " << argc << endi;
			if (argc >= 2)
			{
				thelog << m_resource << endi;
				m_resource = argv[1];
			}
			if (argc >= 3)
			{
				tmp = argv[2];
				thelog << tmp << endi;
				m_querystring = tmp;
				AnalyzeParam(m_querystring);
			}

			char const * penv;
			if (NULL != (penv = getenv("REQUEST_METHOD")))
			{
				m_method = penv;
			}
			if (NULL != (penv = getenv("QUERY_STRING")))
			{
				tmp = penv;
				AnalyzeParam(tmp);
			}
			//如果是POST，内容从标准输入获取，因为没有文件结束符，长度必须从头标获取
			if (NULL != (penv = getenv("CONTENT_LENGTH")))
			{
				char * p = NULL;
				int content_length = atol(penv);
				p = new char[content_length + 1];
				if (NULL == p)return false;
#ifdef _WINDOWS
				if (content_length != _read(STDIN_FILENO, p, content_length))return false;
#else
				if (content_length != read(STDIN_FILENO, p, content_length))return false;
#endif
				p[content_length] = '\0';
				tmp = p;
				AnalyzeParam(tmp);
				delete[] p;
			}
			if (NULL != (penv = getenv("REQUEST_URI")))
			{
				tmp = penv;
				string::size_type pos = tmp.find_first_of("?");
				if (tmp.npos != pos)
				{
					m_resource = tmp.substr(0, pos);
				}
				else
				{
					m_resource = tmp;
				}
			}
			return true;
		}
		string RequestHtmlReport()const//报告请求信息，HTML格式
		{
			string ret = "HttpRequest:<HR/>";
			ret += m_method;
			ret += "&nbsp;";
			ret += m_resource;
			ret += "<BR>\n";
			for (PARAMLIST_T::size_type i = 0; i < m_params.size(); ++i)
			{
				ret += CHtmlDoc::HTMLEncode(m_params[i].first);
				ret += " = ";
				ret += CHtmlDoc::HTMLEncode(m_params[i].second);
				ret += "<BR>\n";
			}
			map<string, string >::const_iterator it;
			for (it = m_cookies.begin(); it != m_cookies.end(); ++it)
			{
				ret += "Cookie: ";
				ret += it->first + "=" + it->second + "<BR>\n";
			}
			ret += "<HR/>";
			time_t t1 = time(NULL);
			ret += asctime(localtime(&t1));
			return ret;
		}
	};

	//HTTP应答
	//AddHeaderXXXX系列函数添加应答头，必须在Flush之前使用
	class CHttpRespond
	{
	private:
		string m_status_line;//状态行
		vector<pair<string, string > > m_headers;//应答头
		map<string, string > m_cookies;
		string m_body;//消息体
		long m_content_length;//设置的内容长度，未设置为-1
		long m_content_length_sended;//实际发送的内容长度
		bool m_isCgi;
	public:
		//判断是否可以保持连接，依据是发送了内容长度并且实际发送长度匹配
		bool isCanKeepAlive()const
		{
			return m_content_length >= 0 && m_content_length == m_content_length_sended;
		}
		string GetBody()const { return m_body; }
		//返回无内容的成功
		bool Send200(CMySocket & s)
		{
			m_status_line = "HTTP/1.1 200 OK";
			AddHeaderContentLength();
			return Flush(s);
		}
		//重定向
		bool Send302(CMySocket & s, string const & location)
		{
			m_status_line = "HTTP/1.1 302 Found";
			AddHeader("Location", location);
			AddHeaderContentLength();
			return Flush(s);
		}
		//错误的请求
		bool Send400(CMySocket & s)
		{
			m_status_line = "HTTP/1.1 400 BadRequest";
			AddHeaderContentLength();
			return Flush(s);
		}
		//需要认证
		bool Send401(CMySocket & s, string const & realm)
		{
			m_status_line = "HTTP/1.1 401 Unauthorized";
			string str = "Basic realm=\"" + realm + "\"";
			AddHeader("WWW-Authenticate", str);
			AddHeaderContentLength();
			return Flush(s);
		}
		//禁止访问
		bool Send403(CMySocket & s)
		{
			m_status_line = "HTTP/1.1 403 Forbidden";
			AddHeaderContentLength();
			return Flush(s);
		}
		//没有找到
		bool Send404(CMySocket & s)
		{
			m_status_line = "HTTP/1.1 404 NotFound";
			AddHeaderContentLength();
			return Flush(s);
		}
		//内部错误
		bool Send500(CMySocket & s)
		{
			m_status_line = "HTTP/1.1 500 Internal Server Error";
			AddHeaderContentLength();
			return Flush(s);
		}
		//内部错误，返回第一行为错误码然后是错误信息，错误信息为全局错误信息
		bool Send500WithMessage(CMySocket & s)
		{
			m_status_line = "HTTP/1.1 200 Internal Server Error";
			AddHeaderContentTypeByFilename("*.txt");

			char buf[256];
			sprintf(buf, "%ld\n", G_GET_ERROR);
			AppendBody(buf);

			AppendBody(G_ERROR_MESSAGE().str());
			G_CLEAR_ERROR;

			AddHeaderContentLength();
			return Flush(s);
		}
		void Clear()
		{
			m_status_line = "";
			m_body = "";
			m_content_length = -1;
			m_content_length_sended = 0;
			m_headers.clear();
			m_cookies.clear();
			m_isCgi = false;
		}
		void Init(bool _cgi = false)//初始化应答，设置通用部分
		{
			Clear();
			m_isCgi = _cgi;
			m_status_line = "HTTP/1.1 200 OK";
			AddHeader("Server", "WWW-Server/1.0 by ct");
			//AddCookie("user","unknown");
			//AddCookie("id","unknown");
		}
		void AddHeader(string const & header, string const & value)//添加应答头，除了"Server"其它都可以用，但有一些可以更方便地使用AddHeaderXXXX
		{
			if ("Server" != header)m_headers.push_back(pair<string, string >(header, value));
		}
		void AddHeader(string const & header, long value)
		{
			char buf[256];
			sprintf(buf, "%ld", value);
			AddHeader(header, buf);
		}
		void AddCookie(string const & name, string const & value)
		{
			m_cookies[name] = value;
		}
		void AppendBody(long n)//添加消息体
		{
			char buf[64];
			sprintf(buf, "%ld", n);
			m_body += buf;
		}
		void AppendBody(string const & str)//添加消息体
		{
			m_body += str;
			//LOG<<"HTML 输出："<<endl<<str<<ENDI;
		}
		void AppendBody(char const * str)//添加消息体
		{
			m_body += str;
			//LOG<<"HTML 输出："<<endl<<str<<ENDI;
		}
		void AppendBodyHtmlStart(char const * title = NULL)//添加HTML文档头部，直到<body>
		{
			m_body += "<!DOCTYPE HTML>\r\n"
				"<html>\r\n"
				"<head>\r\n"
				"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\r\n"
				"<link href=\"/css/css.css\" rel=\"stylesheet\" type=\"text/css\"/>\r\n"
				;
			if (NULL != title)
			{
				m_body += "<title>";
				m_body += title;
				m_body += "</title>\r\n";
			}
			m_body += "</head>\r\n<body>\r\n";
		}
		void AppendBodyHtmlEnd()//添加HTML文档结束，从</body>开始
		{
			m_body += "</body>\r\n</html>\r\n";
		}
		void AppendBodyHtmlScroll()//添加HTML滚动
		{
			m_body += "<script language=\"JavaScript\">window.scrollBy(0,document.body.scrollHeight);</script>\r\n";
		}
		void AppendBodyHtmlCopyright()//添加HTML版权
		{
			m_body += "\r\n<BR><CODE>C++ WEB服务类库 包含嵌入式WEB服务器、CGI、HTML、XML支持</CODE>";
			m_body += "\r\n<BR><CODE>Powered by Embeded Web Server , Copyright &copy; " COPY_RIGHT " ct ,All rights reserved.</CODE>";
			m_body += "\r\n<BR><CODE>本站点由嵌入式WEB服务器支持 , 版权所有 &copy; " COPY_RIGHT " ct ,保留所有权利.</CODE>";
		}
		bool Flush(CMySocket & s)
		{
			if (m_isCgi)return CgiFlush();
			string str;
			vector<pair<string, string > >::size_type i;
			//仅在状态行不为空的情况下发送应答头
			if (m_status_line.size() != 0)
			{
				str = m_status_line + "\r\n";
				for (i = 0; i < m_headers.size(); ++i)
				{
					str += m_headers[i].first + ": " + m_headers[i].second + "\r\n";
				}
				map<string, string >::const_iterator it;
				for (it = m_cookies.begin(); it != m_cookies.end(); ++it)
				{
					str += "Set-Cookie: ";
					str += it->first + "=" + it->second + "\r\n";
				}
				str += "\r\n";
			}
			str += m_body;
			m_content_length_sended += m_body.size();
			m_status_line = "";
			m_headers.clear();
			m_body = "";
			DEBUG_LOG << "发送应答" << endl << str << ENDI;
			if (!s.Send(str))
			{
				s.Close();
				return false;
			}
			return true;
		}
		bool CgiFlush()
		{
			string str;
			string::size_type i;
			if (m_status_line.size() != 0)
			{
				//cgi不发送状态行，直接发送头标
				m_status_line = "";
				for (i = 0; i < m_headers.size(); ++i)
				{
					str += m_headers[i].first + ": " + m_headers[i].second + "\r\n";
				}
				m_headers.clear();
				map<string, string >::const_iterator it;
				for (it = m_cookies.begin(); it != m_cookies.end(); ++it)
				{
					str += "Set-Cookie: ";
					str += it->first + "=" + it->second + "\r\n";
				}
				m_cookies.clear();
				str += "\r\n";//应答头结束标记
			}
			str += m_body;
			m_body = "";
			thelog << "发送应答" << endl << str << endi;
			cout << str << flush;
			return true;
		}

		void AddHeaderContentTypeByFilename(char const * filename)//添加内容类型到应答头，如果已经存在则替换，根据文件名后缀判断
		{
			for (size_t i = 0; i < m_headers.size(); ++i)
			{
				if (m_headers[i].first != "Content-Type")continue;
				m_headers[i].second = CMIMEType::GetMIMEType(filename);
				return;
			}
			m_headers.push_back(pair<string, string >("Content-Type", CMIMEType::GetMIMEType(filename)));
		}
		bool AddHeaderContentLength()//添加消息体长度到应答头，若要使用此功能必须在唯一的Flush之前调用（因为第一个Flush将发出应答头，以后无法再更改应答头并且无法获得正确的消息体长度）
		{
			if (m_content_length < 0)
			{
				m_content_length = m_body.size();
				char buf[256];
				sprintf(buf, "%ld", m_content_length);
				m_headers.push_back(pair<string, string >("Content-Length", buf));
				return true;
			}
			else
			{
				LOG << "错误：已经设置过Content-Length" << ENDE;
				LOG << this->m_status_line << ENDE;
				LOG << this->m_content_length << ENDE;
				LOG << this->m_body << ENDE;
				return false;
			}
		}
		bool AddHeaderContentLength(long len)//添加消息体长度到应答头
		{
			if (m_content_length < 0)
			{
				m_content_length = len;
				char buf[256];
				sprintf(buf, "%ld", m_content_length);
				m_headers.push_back(pair<string, string >("Content-Length", buf));
				return true;
			}
			else
			{
				LOG << "错误：已经设置过Content-Length" << ENDE;
				return false;
			}
		}
		void AddHeaderNoCache()//添加禁止缓存到应答头
		{
			AddHeader("Pragma", "no-cache");
			AddHeader("Cache-Control", "no-cache, must-revalidate");
			AddHeader("Expires", "0");
		}
		void AddHeaderExpires(time_t t1, long seconds)//添加缓存控制到应答头
		{
			AddHeader("Expires", CHttpDate::HttpDate(t1, seconds));
		}
	};

	//外接页面，类似于asp页面，后缀名不作为页面名称的一部分而是保留给能够自动跟后缀名输出不同格式用
	//命令访问方式：/bin/command_id.asp(.aspx .asmx)
	class CWebCommand
	{
	private:
		string ReplaceDot(string const & str)
		{
			string ret = str;
			string::size_type pos;
			while (str.npos != (pos = ret.find('.')))ret.replace(pos, 1, "_");
			while (str.npos != (pos = ret.find('/')))ret.replace(pos, 1, "_");
			return ret;
		}
	public:
		bool isVirtual;//是否是虚拟命令，仅出现在命令表中但并非使用命令接口来实现
		bool isAdmin;//是否是内置页面
		bool notonhomepage;//是否不出现在首页
		bool hide;//是否隐藏，只能通过直接命令调用
		bool AutoRefresh;//是否允许自动刷新
		bool NotStdPage;//非标准页面，不生成标准数据，所有数据，包括应答头都由命令自己生成
		string command_id;//命令ID，也作为页面名称，但不包括后缀名
		string name;//显示名称
		string note;

		long queue;//仅用于消息模式
		bool demon;//是否配有后台进程

		vector<CWebCommandParam > params;//命令参数

		//asp页面的接口函数
		virtual bool InitWebFunction() { return true; }
		virtual bool UnInitWebFunction() { return true; }
		virtual bool doWebFunction(CHttpRequest const * pRequest, CMySocket & s, CHttpRespond * pRespond)
		{
			pRespond->AppendBody(pRequest->RequestHtmlReport());
			return true;
		}

		CWebCommand() { clear();  SetWebCommand("CWebCommand", "CWebCommand", "原始CWebCommand"); }
		void clear()
		{
			isVirtual = false;
			isAdmin = false;
			command_id = "";
			notonhomepage = hide = AutoRefresh = NotStdPage = false;
			name = note = "";
			queue = -1;
			demon = false;
			params.clear();
		}
		void SetNotOnHomePage() { notonhomepage = true; }
		void SetHide() { notonhomepage = hide = true; }
		void SetAutoRefresh() { AutoRefresh = true; }
		void SetNotStdPage() { NotStdPage = true; }
		void SetWebCommand(char const * _id, char const * _name, char const * _note)
		{
			command_id = _id;
			name = _name;
			note = _note;
		}
		//rows不为0显示为teataera,选项数目不为0则显示为select
		void AddWebCommandParam(char const * _id, char const * _name, char const * _note, char const * _default, bool isPassword = false, bool isNotNull = false, long size = 16, long rows = 0, vector<pair<string, string > > * _optionvalue = NULL)
		{
			CWebCommandParam tmp;
			tmp.id = _id;
			tmp.name = _name;
			tmp.note = _note;
			tmp.defaultvalue = _default;
			tmp.isPassword = isPassword;
			tmp.isNotNull = isNotNull;
			tmp.size = size;
			tmp.rows = rows;
			if (NULL != _optionvalue)tmp.optionvalue = *_optionvalue;
			else tmp.optionvalue.clear();
			this->params.push_back(tmp);
		}
		void AddWebCommandParamCheckBox(char const * _id, char const * _name, char const * _note, bool isChecked)
		{
			CWebCommandParam tmp;
			tmp.id = _id;
			tmp.name = _name;
			tmp.note = _note;
			tmp.isCheckBox = true;
			tmp.isChecked = isChecked;
			this->params.push_back(tmp);
		}
		void SetParamValueFromRequest(CHttpRequest const & request)
		{
			for (vector<CWebCommandParam >::size_type i = 0; i < params.size(); ++i)
			{
				if (params[i].isCheckBox)params[i].isChecked = (0 != request.GetParam(params[i].id).size());
				else params[i].defaultvalue = request.GetParam(params[i].id);
			}
		}
		string GetFormName()
		{
			return "form_" + ReplaceDot(command_id);
		}
		string toHtmlFormInput(bool smallinput, bool showParamId, CHttpRequest const * pRequest)
		{
			string str;

			if (command_id.size() != 0)
			{
				str += "<FORM name=\"" + GetFormName() + "\" target=\"_self\" ACTION=\"";
				str += command_id;
				str += ".asp\" METHOD=\"GET\" >\r\n";
			}
			else
			{
				str += "<FORM name=\"" + GetFormName() + "\" target=\"_self\" METHOD=\"GET\" >\r\n";
			}

			//添加命令ID
			str += "<input type=\"hidden\" name=\"___COMMAND_ID___\" value=\"";
			str += command_id;
			str += "\"></input>";
			//添加每个参数
			if (0 != params.size())
			{
				for (vector<CWebCommandParam >::size_type i = 0; i < params.size(); ++i)
				{
					str += params[i].toHtmlInput(smallinput, showParamId, pRequest->GetParam(params[i].id));
					str += "<BR>\r\n";
				}
			}
			else
			{
				str += "&nbsp;";
			}
			return str;
		}
		string toHtmlFormSubmit(bool disable, bool both)
		{
			char buf[1024];

			if (disable)
			{
				sprintf(buf, "<BR>不可用\r\n</FORM>\r\n");
			}
			else
			{
				if (both)
				{
					sprintf(buf, "<INPUT TYPE=SUBMIT VALUE=\"%s\" onclick=\"document.%s.target='_self'\">"
						"<INPUT TYPE=SUBMIT VALUE=\"%s(新窗口)\" onclick=\"document.%s.target='_blank'\">\r\n</FORM>\r\n"
						, name.c_str(), GetFormName().c_str(), name.c_str(), GetFormName().c_str());
				}
				else
				{
					sprintf(buf, "<BR><INPUT TYPE=SUBMIT VALUE=\"%s\" onclick=\"document.%s.target='_blank'\">\r\n</FORM>\r\n"
						, name.c_str(), GetFormName().c_str());
				}
			}

			return buf;
		}
	};
}
