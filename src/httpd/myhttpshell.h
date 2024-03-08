//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "myhttpbase.h"
#include "myhttpclient.h"
#include "mysocketserver.h"
#include "myhttpserver_struct.h"
#include <errno.h>
#ifdef _LINUXOS
#include <typeinfo>
#endif
#include <dlfcn.h>
#include <sys/mman.h>

namespace ns_my_std
{
	class CHttpShell
	{
	private:
		static CMySocket * AccessSocket(CMySocket * p=NULL)
		{
			STATIC_G CMySocket * s;
			if(NULL!=p)s=p;
			return s;
		}
		static void CallBackForLog(const string& strLog)
		{
			CMySocket * s=AccessSocket();
			STATIC_G long static_level = 0;
			if (s->IsConnected() && !s->Send(LogToHtml(&static_level, strLog, false, true)))
			{
				s->Close();
			}
		}
		//处理日志折叠和加色，static_level用于计算缩进级别，初始设置为0
		static string LogToHtml(long * static_level, const string& strLog, bool forPRE, bool addBR)
		{
			char const guid[]="{7DA5C0D6-31DC-40e3-92CF-5300B84BA162}";//DO NOT EDIT
			string str;
			string::size_type pos;

			if(str.npos!=(pos=strLog.find(guid)))
			{
				str=strLog.substr(pos);
				char buf[2048];
				bool isStart=('S'==str[strlen(guid)]);
				bool isEnd=('E'==str[strlen(guid)]);
				long id=atol(str.c_str()+strlen(guid)+1);
				long & level = *static_level;
				string title;
				{
					title=str.substr(str.find("#")+1);
					string::size_type tmppos;
					if(title.npos!=(tmppos=title.find("\r")))title.erase(tmppos);
					if(title.npos!=(tmppos=title.find("\n")))title.erase(tmppos);
				}
				if(isStart)
				{
					++level;
					sprintf(buf,"<hr></hr><CODE><NOBR><div onclick=\"if(''==document.getElementById('%ld').style.display){document.getElementById('%ld').style.display='none';this.innerHTML='[+]%s';};else {document.getElementById('%ld').style.display='';this.innerHTML='[-]%s';}\">[-]%s level %ld</div></NOBR></CODE>\r\n"
						,id,id,title.c_str(),id,title.c_str(),title.c_str(),level);
					str=buf;
					sprintf(buf,"<div id=\"%ld\" style=\"margin-left:%ldpx;\">\r\n",id,32L);
					str+=buf;
				}
				if(isEnd)
				{
					--level;
					sprintf(buf,"</div>\r\n");
					str=buf;
				}
			}
			else
			{
				str="<CODE><NOBR>"+CHtmlDoc::HTMLEncode(strLog,forPRE)+"</NOBR></CODE>";
				if(str.npos!=str.find("[出错]") || str.npos!=str.find("FATAL") || str.npos!=str.find("ERROR") || str.npos!=str.find("Error") || str.npos!=str.find("error"))
				{
					str="<FONT color=\"red\">"+str+"</FONT>";
				}
				else if(str.npos!=str.find("[警告]") || str.npos!=str.find("WARN") || str.npos!=str.find("Warning") || str.npos!=str.find("warning"))
				{
					str="<FONT color=\"blue\">"+str+"</FONT>";
				}
				else if(str.npos!=str.find("[归档]"))
				{
					str="<B>"+str+"</B>";
				}
				if(addBR)str+="<BR>\r\n";
				else str+="\r\n";
			}
			return str;
		}
	private:
		CMySocket & m_s;//连接
		CHttpRequest & m_request;//请求
		CHttpRespond & m_respond;//应答
	public:
		CHttpShell(CMySocket & _s, CHttpRequest & _request, CHttpRespond & _respond) :m_s(_s), m_request(_request), m_respond(_respond) {}
		bool doPageViewFile()
		{
			ifstream f;
			char * buf;
			long bufsize=1024*1024;
			long count,len,outcount=0;
			string file=m_request.GetParam("file");
			string autorefresh=m_request.GetParam("autorefresh");
			long start=atol(m_request.GetParam("start").c_str());
			long end=atol(m_request.GetParam("end").c_str());
			long origin_start=start;//原始start
			long maxcount=atol(m_request.GetParam("maxcount").c_str());
			if(0==maxcount)maxcount=500;//默认值
			if(maxcount<0 || maxcount>50000)maxcount=50000;//最大行数限制

			while(true)
			{
				if(NULL==(buf=new char[bufsize]))
				{
					m_respond.AppendBody("内存不足<P>");
					break;
				}
				if(file.size()==0)
				{
					m_respond.AppendBody("没有文件名<P>");
					break;
				}
				m_respond.AppendBody("文件名： ");
				m_respond.AppendBody(file);
				m_respond.AppendBody("<BR>");
				//打开文件
				f.open(file.c_str(),ios::in);
				if(!f.good())
				{
					m_respond.AppendBody("打开文件错误<P>");
					break;
				}
				//获取长度
				f.seekg(0,ios::end);
				len=f.tellg();
				if(end>0)
				{
					sprintf(buf,"文件长度： %ld，开始位置：%ld，结束位置： %ld，请求的总长度：%ld<BR>------------------------------------------<P>\n",len,start,end,end-start);
					len=end;
				}
				else
				{
					sprintf(buf,"文件长度： %ld，开始位置：%ld，请求的总长度：%ld<BR>------------------------------------------<P>\n",len,start,len-start);
				}
				m_respond.AppendBody(buf);
				if(!m_respond.Flush(m_s))return true;
				//读取内容
				if(start<0)start=len+start;//负值代表从结束开始往前
				if(start<0)start=0;//如果还是负的说明参数错误
				f.seekg(start,ios::beg);
				outcount=0;
				count=0;
				long static_level = 0;
				while(f.good())
				{
					if(count>=maxcount)break;//超过输出量限制
					f.getline(buf,bufsize-1);
					buf[bufsize-1]='\0';
					m_respond.AppendBody(LogToHtml(&static_level, buf, false, true));
					++count;
					if(count%100==0)
					{
						m_respond.AppendBodyHtmlScroll();
						if(!m_respond.Flush(m_s))break;
					}
					outcount=(long)f.tellg()-start;
					if(f.tellg()>=len)
					{
						break;
					}
				}
				m_respond.Flush(m_s);
				f.close();
				m_respond.AppendBody("<P>------------------------------------------<P>");
				sprintf(buf,"剩余字节数： %ld",len-start-outcount);
				m_respond.AppendBody(buf);

				break;
			}
			if(!m_respond.Flush(m_s))return true;

			char buf2[10240];
			sprintf(buf2,
				"<FORM METHOD=\"GET\" name=\"form_view\">\n"
				"<INPUT TYPE=\"hidden\" NAME=\"file\" VALUE=\"%s\" >\n"
				"<INPUT TYPE=\"hidden\" NAME=\"autorefresh\" VALUE=\"%s\" >\n"
				"起点（从0开始，负值代表从文件尾倒数）：<INPUT TYPE=\"text\" SIZE=\"30\" NAME=\"start\" VALUE=\"%ld\"><BR>\n"
				"终点（从0开始，负值代表从文件尾倒数）：<INPUT TYPE=\"text\" SIZE=\"30\" NAME=\"end\" VALUE=\"%ld\"><BR>\n"
				"最大输出行数： <INPUT TYPE=\"text\" SIZE=\"30\" NAME=\"maxcount\" VALUE=\"%ld\">\n"
				"<INPUT TYPE=SUBMIT VALUE=\"执行\" >\n"
				"</FORM>\n"
				,file.c_str(),autorefresh.c_str(),(origin_start<0?origin_start:start+outcount),end,maxcount);
			m_respond.AppendBody(buf2);
			bool isAutoRefresh=(m_request.GetParam("autorefresh")=="true");
			sprintf(buf2,
				"<BUTTON NAME=\"autorefresh\" onclick=\"timer=setTimeout('window.form_view.submit()',10000);window.form_view.autorefresh.value='true';window.autorefresh.disabled=true;window.stoprefresh.disabled=false;\">自动刷新</BUTTON>\n"
				"<button NAME=\"stoprefresh\" disabled onclick=\"clearTimeout(timer);window.form_view.autorefresh.value='';window.autorefresh.disabled=false;window.stoprefresh.disabled=true;\">停止刷新</BUTTON><BR>\n"
				"%s<BR>\n"
				,(isAutoRefresh?"<script language=\"JavaScript\">window.autorefresh.onclick()</script>":""));
			m_respond.AppendBody(buf2);
			if(!m_respond.Flush(m_s))return true;

			delete[] buf;
			return true;
		}
		bool doPageSSH()
		{
			return _doPageShell(true);
		}
		bool doPageShell()
		{
			return _doPageShell(false);
		}
		bool _doPageShell(bool isSSH)
		{
			FILE * fp;
			string host = m_request.GetParam("host");//远程执行
			string user = m_request.GetParam("user");//远程执行
			string changedir = m_request.GetParam("changedir");
			string curdir=m_request.GetParam("curdir");
			string cmd=m_request.GetParam("command");
			bool noform=(m_request.GetParam("noform")=="true");
			bool term = (m_request.GetParam("term") == "true");//如果连接断开则中止执行

			StringTokenizer st(host, "@");
			if (st.size() >= 2)
			{
				user = st[0];
				host = st[1];
			}
			if (st.size() >= 3)
			{//更多参数，附加在cmd之后
				cmd += " " + st[2];
			}
			
			long bufsize=1024*1024;
			char * buf=new char[bufsize];
			if(NULL==buf)
			{
				m_respond.AppendBody("<P><FONT color=RED>内存不足</FONT><P>");
				return true;
			}

			bool isError=false;

			//切换路径
			if(0!=curdir.size() && !isSSH)
			{
				if(0!=chdir(curdir.c_str()))
				{
					m_respond.AppendBody("<P><FONT color=RED>设置初始路径出错</FONT><P>"+curdir+"<P>");
					isError = true;
				}
			}
			if(0!=changedir.size() && !isSSH)
			{
				if(0!=chdir(changedir.c_str()))
				{
					m_respond.AppendBody("<P><FONT color=RED>切换工作路径出错</FONT><P>"+changedir+"<P>");
					isError = true;
				}
			}

			Trim(host);
			Trim(user);
			//执行命令
			if(0==cmd.size() || (isSSH && 0==host.size()))
			{
				isError = true;
				if(0 == cmd.size())m_respond.AppendBody("<P>空命令<P>");
				if (isSSH && 0 == host.size())m_respond.AppendBody("<P>未输入远程主机<P>");
			}
			if (!isError)
			{
				//对SSH要构造复杂的指令串
				string sshcmd;
				if(isSSH)
				{
					sshcmd = "ssh '" + user + "@" + host + "' '. ./.bash_profile;";
					if (0 != changedir.size())sshcmd += "cd " + changedir + ";";
					sshcmd += cmd + "'";
				}
				
				//if(0!=setpgid(getpid(),getpid()))
				//{
				//	m_respond.AppendBody("设置进程组ID出错<P>");
				//}
				if (NULL == (fp = popen(((isSSH ? sshcmd : cmd) + " 2>&1").c_str(), "r")))
				{
					m_respond.AppendBody("<P><FONT color=RED>无法执行，原因：popen error</FONT><P>");
					if(!m_respond.Flush(m_s))return true;
				}
				else
				{
					int fd=fileno(fp);
					int flags;
					fd_set fdset;
					struct timeval tv;

					tv.tv_sec=300;
					tv.tv_usec=0;
					flags = fcntl(fd, F_GETFL, 0);
					flags |= O_NONBLOCK;
					fcntl(fd, F_SETFL, flags);

					char * tmpp;
					m_respond.AppendBody("开始执行&nbsp;");
					m_respond.AppendBody(CHtmlDoc::HTMLEncode(isSSH ? sshcmd : cmd));
					m_respond.Flush(m_s);
					m_respond.AppendBody("<HR></HR><CODE>");
					while(true)
					{
						FD_ZERO(&fdset);
						FD_SET(fd,&fdset);
#ifdef _HPOS
						int selectret=select(fd+1,(int *)&fdset,NULL,NULL,&tv);
#else
						int selectret=select(fd+1,&fdset,NULL,NULL,&tv);
#endif
						if(selectret<0)
						{
							LOG<<"select error"<<ENDE;
						}
						else if(0==selectret)
						{//超时没有数据
							m_respond.AppendBody("注意,长时间没有收到输出.");
							if(!m_respond.Flush(m_s))
							{
								LOG<<"发送失败,客户端已断开,直接退出"<<ENDI;
								if(term)
								{
									//if(0!=kill(0,SIGTERM))
									//{
									//	LOG<<"发送停止信号出错，shell会持续执行到命令结束"<<ENDE;
									//}
								}
								return true;
							}
							continue;
						}
						else
						{
						}

						bool fileend=false;
						long static_level = 0;
						while(true)
						{
							tmpp=fgets(buf,int(bufsize-1),fp);
							if(NULL==tmpp)
							{
								//正常结束
								if(0!=feof(fp))
								{
									fileend=true;
									break;
								}

								if(EWOULDBLOCK==errno || EAGAIN==errno)
								{//无数据
									break;
								}
								else
								{//出错结束
									m_respond.AppendBody("<P><FONT color=RED>执行出错，原因：read error</FONT><P>");
									m_respond.AppendBody(strerror(errno));
									m_respond.Flush(m_s);
									fileend=true;
									break;
								}
							}
							else
							{
								m_respond.AppendBody(LogToHtml(&static_level, buf, false, false));
								m_respond.AppendBodyHtmlScroll();
								if(!m_respond.Flush(m_s))
								{
									LOG<<"发送失败,客户端已断开,直接退出"<<ENDI;
									if(term)
									{
										//if(0!=kill(0,SIGTERM))
										//{
										//	LOG<<"发送停止信号出错，shell会持续执行到命令结束"<<ENDE;
										//}
									}
									return true;
								}
							}
						}
						if(fileend)break;
					}
					m_respond.AppendBody("</CODE><HR></HR>");
					int ret=pclose(fp);
					if(0!=ret)
					{
						if(WIFEXITED(ret))
						{
							sprintf(buf,"<FONT color=RED>执行完毕，返回代码 %d 。</FONT><BR>",WEXITSTATUS(ret));//(0xFF00&ret)/256);
						}
						else if(WIFSIGNALED(ret))
						{
							sprintf(buf,"<FONT color=RED>被信号终止，信号 %d 。</FONT><BR>",WTERMSIG(ret));
						}
						else if(WCOREDUMP(ret))
						{
							sprintf(buf,"<FONT color=RED>执行失败，COREDUMP。</FONT><BR>");
						}
						else
						{
							sprintf(buf,"<FONT color=RED>未知的返回值：%d。</FONT><BR>",ret);
						}
					}
					else sprintf(buf,"执行完毕，返回代码 %d 。<BR>",ret);
					m_respond.AppendBody(buf);
				}
			}

			if(!noform)
			{
				char cwd[1024];
				if(NULL!=getcwd(cwd,1024))
				{
					if(isSSH)
					{
						sprintf(buf,
							"<FORM ACTION=\"/ssh.asp\" METHOD=\"GET\" >\n"
							"主机：<INPUT TYPE=\"text\" SIZE=\"30\" NAME=\"host\" VALUE=\"%s\">\n"
							"用户：<INPUT TYPE=\"text\" SIZE=\"30\" NAME=\"user\" VALUE=\"%s\"><BR>\n"
							"切换路径到：<INPUT TYPE=\"text\" SIZE=\"100\" NAME=\"changedir\" VALUE=\"%s\"><BR>\n"
							"Shell命令： <INPUT TYPE=\"text\" SIZE=\"100\" NAME=\"command\" VALUE=\"%s\">\n"
							"<INPUT TYPE=SUBMIT VALUE=\"执行\" >\n"
							"</FORM>\n"
							, host.c_str(), user.c_str(), changedir.c_str(), cmd.c_str());
					}
					else
					{
						sprintf(buf,
							"<FORM ACTION=\"/shell.asp\" METHOD=\"GET\" >\n"
							"当前路径：%s<BR>"
							"<INPUT TYPE=\"hidden\" NAME=\"curdir\" VALUE=\"%s\" >\n"
							"切换路径到：<INPUT TYPE=\"text\" SIZE=\"100\" NAME=\"changedir\" ><BR>\n"
							"Shell命令： <INPUT TYPE=\"text\" SIZE=\"100\" NAME=\"command\" VALUE=\"%s\">\n"
							"<INPUT TYPE=SUBMIT VALUE=\"执行\" >\n"
							"</FORM>\n"
							, cwd, cwd, cmd.c_str());
					}
				}
				else
				{
					sprintf(buf,"获取当前工作路径出错");
				}
				m_respond.AppendBody(buf);
			}

			delete[] buf;
			return true;
		}

	};

}
