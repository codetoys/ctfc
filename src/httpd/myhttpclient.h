//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "myhttpbase.h"


namespace ns_my_std
{
	class CHttpClient
	{
	public:
		struct Headers
		{
			vector<pair<string, string> > m_headers;

			void clear() { m_headers.clear(); }
			bool push_back(char const * header, char const * value)
			{
				m_headers.push_back(pair<string, string >(header, value));
				return true;
			}
			bool push_back(char const * header, long value)
			{
				char buf[256];
				sprintf(buf, "%ld", value);
				return push_back(header, buf);
			}
		};
	private:
		CMySocket m_s;
		string m_host;
		int m_port;
		string m_user;
		string m_pass;

		string m_fullrequest;
		long m_status;
		long m_contentlength;
		string m_content;

		long m_timeout;

	private:

		bool RecvRequest(CMySocket & s)
		{
			if (!m_s.IsConnected()) //处理客户端有时候接收响应数据失败情况
			{
				thelog << "接收数据时连接断开，尝试重新连接..." << endi;
				if (!Connect(m_host, m_port))
				{
					thelog << "重新连接失败，无法接收数据" << ende;
					return false;
				}
			}

			m_fullrequest="";
			m_status=0;
			m_contentlength=0;

			string::size_type headlen=0;

			char buf[1024];
			long count;
			string::size_type pos;

			//先接收请求头
			while(m_fullrequest.npos==(headlen=m_fullrequest.find("\r\n\r\n")))
			{
				bool ready = false;
				if (!s.IsSocketReadReady2(m_timeout, ready) || !ready)
				{
					thelog << "select error or timeout" << ende;
					return false;
				}
				if(!s.Recv(buf,1023,&count))
				{
					thelog << "recv head error" << ende;
					s.Close();
					return false;
				}
				if(0==count)
				{
					thelog << "client closed when recv head" << ende;
					s.Close();
					return false;
				}
				buf[count]='\0';
				m_fullrequest+=buf;
			}

			//获得第一行
			pos=m_fullrequest.find("\r\n");
			string str;
			str=m_fullrequest.substr(0,pos);
			//获得status code
			pos=str.find(" ");
			if(str.npos==pos)
			{
				return false;
			}
			m_status=atol(str.c_str()+pos+1);

			//如果存在请求内容则接收请求内容
			pos=m_fullrequest.find("Content-Length:");
			if(m_fullrequest.npos==pos || pos >=headlen)
			{
				return true;
			}
			m_contentlength=atol(m_fullrequest.c_str()+pos+strlen("Content-Length:"));
			while(m_fullrequest.size()<headlen+strlen("\r\n\r\n")+m_contentlength)
			{
				if(!s.Recv(buf,1023,&count))
				{
					thelog << "recv content error" << ende;
					return false;
				}
				if(0==count)
				{
					thelog << "client closed when recv content" << ende;
					return false;
				}
				buf[count]='\0';
				m_fullrequest+=buf;
			}
			m_content=m_fullrequest.substr(headlen+strlen("\r\n\r\n"),m_contentlength);
			return true;
		}

		bool _Open(string method, string resource, Headers const *request_headers, string querystring
			, string content_type, string const & content, bool keep_alive = false, bool debug = false)
		{
			if (!__Open(method, resource, request_headers, querystring, content_type, content, keep_alive, debug))
			{
				if (!Connect(m_host, m_port))
				{
					thelog << "重新连接失败 " << m_host << " " << m_port << ende;
					return false;
				}
				else
				{
					return __Open(method, resource, request_headers, querystring, content_type, content, keep_alive, debug);
				}
			}

			return true;
		}
		bool __Open(string method, string resource, Headers const *request_headers, string querystring
			, string content_type, string const& content, bool keep_alive = false, bool debug = false)
		{
			if(!m_s.IsConnected())return false;
			m_s.isDebug = debug;

			ToUpper(method);

			string request_data;
			request_data+=method+" ";
			
			if(resource.size()<1 || resource[0]!='/')
			{
				thelog<<"resource error : "<<resource<<" (must start with '/')"<<ende;
				return false;
			}
			request_data+=resource;
			
			if(querystring.size()!=0)
			{
				request_data+="?"+querystring;
			}

			request_data+=" HTTP/1.1\r\n";
			if(m_user.size()!=0)
			{
				request_data+="Authorization: Basic ";
				
				char buf[2048];
				string str=m_user+":"+m_pass;
				int len=CBase64::Base64Enc((unsigned char *)buf, (unsigned char*)str.c_str(),str.size());
				buf[len]='\0';

				request_data+=buf;
				request_data+="\r\n";
			}
			if (keep_alive) //是否保持连接
			{
				request_data += "Connection: Keep-Alive";
				request_data += "\r\n";
			}

			//内容类型
			request_data += "Content-Type: ";
			request_data += CMIMEType::GetMIMEType(content_type.c_str());
			request_data += "\r\n";
		
			//内容长度
			char buf[2048];
			sprintf(buf, "Content-Length: %ld", (long)content.size());
			request_data += buf;
			request_data += "\r\n";

			if(NULL!=request_headers)
			{
				vector<pair<string,string> >::const_iterator it;
				for(it=request_headers->m_headers.begin();it!=request_headers->m_headers.end();++it)
				{
					char buf[2048];
					sprintf(buf, "%s: %s", it->first.c_str(), it->second.c_str());
					request_data += buf;
					request_data += "\r\n";
				}
			}
			request_data+="\r\n";//请求头结束

			request_data+=content;//内容

			DEBUG_LOG<<"request data :"<<endl<<"==============================================="<<endl
				<<request_data<<endl
				<<"==============================================="<<endi;

			if(!m_s.Send(request_data))
			{
				thelog<<"Send error"<<ende;
				m_s.Close();
				return false;
			}
			if(!RecvRequest(m_s))
			{
				thelog<<"RecvRequest error"<<ende;
				m_s.Close();
				return false;
			}
			DEBUG_LOG << "recv :" << m_status << " " << m_contentlength << endl
				<<"======================================================================="<<endl
				<<m_content<<endl
				<<"======================================================================="<<endi;
			if (!keep_alive)
			{
				m_s.Close();
			}

			if (GetHeader("Location") != "")  //重定向
			{
				if (!Open(method, m_host, m_port, GetHeader("Location"), "", "", m_user, m_pass, keep_alive))
				{
					thelog << "CHttpClient Open error" << ende;
					return false;
				}
				if (GetStatus() != 200)
				{
					thelog << " error : " << GetStatus() << ende;
					return false;
				}
			}

			return true;
		}
	public:
		CHttpClient()
		{
			m_timeout = 300;
		}
		~CHttpClient()
		{
			//不能自动关闭，对于vector，会遇到扩展时执行析构
			//Close();
		}
		CMySocket & GetSocket(){return m_s;}
		bool Connect(string host, int port, string user, string pass)
		{
			m_user=user;
			m_pass=pass;
			return Connect(host, port);
		}
		bool Connect(string host,int port)
		{
			DEBUG_LOG << "Connect to " << host << " " << port << " ......" << endi;
			if(m_s.IsConnected())
			{
				if(m_host!=host || m_port!=port)
				{
					DEBUG_LOG << "diffrence target , need reconnect" << endi;
					m_s.Close();
				}
				else
				{
					DEBUG_LOG << "same target" << endi;
					return true;
				}
			}

			if(port<0 || port>=65535)
			{
				thelog<<"port error : "<<port<<" (0-65534)"<<ende;
				return false;
			}
			if(!m_s.Connect(host,(0==port?80:port)))
			{
				thelog<<"Connect to "<<host<<" "<<port<<" error"<<ende;
				return false;
			}
			m_host=host;
			m_port=port;
			return true;
		}
		bool Close()
		{
			m_host="";
			m_port=0;
			m_user="";
			m_pass="";
			return m_s.Close();
		}

		//直接提供完整的请求数据
		bool Open(string host, int port, string const & full_request, bool keep_alive = false, bool debug = false)
		{
			if(!Connect(host,port))return false;
			m_s.isDebug = debug;

			if(!m_s.Send(full_request))
			{
				thelog<<"Send error"<<ende;
				m_s.Close();
				return false;
			}
			if(!RecvRequest(m_s))
			{
				thelog<<"RecvRequest error"<<ende;
				m_s.Close();
				return false;
			}
			DEBUG_LOG << "recv :" << m_status << " " << m_contentlength << endl
				<<"======================================================================="<<endl
				<<m_content<<endl
				<<"======================================================================="<<endi;
			if (!keep_alive)m_s.Close();  //短连接需关闭连接，否则重用这个连接时将接收错误
			return true;
		}
		//最后一个参数keep_alive默认为false表示短连接，要使用长连接则需将keep_alive赋值为true
		bool Open(string method, string host, int port, string resource, string querystring
			, string xml_content, string name, string pass, bool keep_alive = false, bool debug = false)
		{
			if (!Connect(host, port, name, pass))return false;
			return _Open(method, resource, NULL, querystring, "*.xml", xml_content, keep_alive, debug);
		}
		bool Open(string method, string host, int port, string resource, Headers const *request_headers, string querystring
			, string xml_content, string name, string pass, bool keep_alive = false, bool debug = false)
		{
			if (!Connect(host, port, name, pass))return false;
			return _Open(method, resource, request_headers, querystring, "*.xml", xml_content, keep_alive, debug);
		}
		bool Open(string method, string resource, string querystring
			, string xml_content, bool keep_alive = false, bool debug = false)
		{
			return _Open(method, resource, NULL, querystring, "*.xml", xml_content, keep_alive, debug);
		}
		bool Open(string method, string resource, Headers const *request_headers, string querystring
			, string xml_content, bool keep_alive = false, bool debug = false)
		{
			return _Open(method, resource, request_headers, querystring, "*.xml", xml_content, keep_alive, debug);
		}
		bool Open1(string method, string host, int port, string resource, string querystring
			, string xml_content, string name, string pass, bool keep_alive = false, bool debug = false)
		{
			if (!Connect(host, port, name, pass))return false;
			return _Open(method, resource, NULL, querystring, "*.xml", xml_content, keep_alive, debug);
		}
		//支持不同的内容类型
		bool Open2(string method, string resource, Headers const *request_headers, string querystring
			,string content_type, string const& content, bool keep_alive = false, bool debug = false)
		{
			return _Open(method, resource, request_headers, querystring, content_type, content, keep_alive, debug);
		}
		long GetStatus()const{return m_status;}
		long GetContentLength()const{ return m_contentlength; }
		string const & GetContent()const{return m_content;}
		string GetHeader(char const * _header)const
		{
			string header = _header;
			header += ": ";
			string::size_type pos_start;
			string::size_type pos_end;

			if (m_fullrequest.npos == (pos_start = m_fullrequest.find(header)))return "";
			if (m_fullrequest.npos == (pos_end = m_fullrequest.find("\r\n", pos_start)))return "";
			string ret=m_fullrequest.substr(pos_start + header.size(), pos_end - pos_start - header.size());
			return Trim(ret);
		}
	};
}
