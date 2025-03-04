//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "myhttpbase.h"
#include "myhttpclient.h"
#include "mysocketserver.h"
#include "myhttpserver_struct.h"
#include "myhttpserver_admin_page.h"
#include "myhttpshell.h"
#include "myUserManager.h"
#include <errno.h>
#ifdef _LINUXOS
#include <typeinfo>
#endif
#include <dlfcn.h>
#include <sys/mman.h>
#include "mySSLTLS.h"

#define LAST_UPDATE "2016-7-12"

namespace ns_my_std
{
	//HTTP处理
	class CHttpProcess : public ISocketServerProcess
	{
	private:
		struct CConnectData
		{
			CMySocket m_s;//连接
			bool m_isMgrPort;//是否是管理端口
			clock_t m_clock;//时钟，记录处理页面花费的CPU时间
			time_t m_time;//时间，记录总处理用时
			CHttpRequest m_request;//请求
			CHttpRespond m_respond;//应答
			CHttpClient m_httpclient;//用于热备系统连接备机
			long m_i_child;//进程或线程的序号
			CHttpChildData * m_pThisChildData;

			CConnectData() :m_pThisChildData(NULL) {}
		};

		//是否是停机
		bool isShutDown()const{ return m_pServerControlBlock->cmd_bShutDown; }
		//是否子进程需要退出，条件为停机或暂停
		bool isChildNeedExit(bool isMgrPort)const{ return m_pServerControlBlock->cmd_bShutDown || (!isMgrPort && m_pServerControlBlock->cmd_bPause); }
		
		bool OnPageStart(CConnectData * pCD, char const * title, bool isStop = false)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答

			pCD->m_clock=clock();
			time(&pCD->m_time);
			m_respond.AddHeaderNoCache();
			m_respond.AppendBodyHtmlStart((string(title) + " - " + string(m_pServerDatas->m_sitename.c_str())).c_str());
			string str;
			str+="<noscript>\r\n注意：本站点使用JavaScript技术，但您的浏览器不支持或限制了JavaScript，本站点的功能可能无法正常使用。<BR>\r\n请更换或升级浏览器或修改安全设置，建议使用IE5以上浏览器。<P>\r\n</noscript>\r\n";
			time_t t = time(NULL);
			//tm * ptm = localtime(&t);
			str += "<A href=\"/\">首页</A>&nbsp;&nbsp;<A href=\"/admin.asp\" target=\"_blank\">管理</A>&nbsp;&nbsp;&nbsp;&nbsp;";
			str += "<A href=\"/functionlist.asp\" target=\"_blank\">命令清单</A>&nbsp;&nbsp;&nbsp;&nbsp;";
			//str += asctime(ptm);
			str += CMyTools::TimeToString_log(t);
			str += "&nbsp;&nbsp;&nbsp;&nbsp;";
			str += m_pServerDatas->m_sitename.c_str();
			str += "&nbsp;&nbsp;&nbsp;&nbsp;"; 
			str += m_request.GetHeader("Host");
			str += "&nbsp;&nbsp;&nbsp;&nbsp;";
			str += "G_IS_DEBUG(";
			str += (G_IS_DEBUG ? "on" : "off");
			str += ")";
			str += "\r\n<HR></HR>\r\n";
			m_respond.AppendBody(str);

			return true;
		}
		bool OnPageEnd(CConnectData * pCD, bool autoscroll=true)
		{
			//CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			CMySocket & m_s=pCD->m_s;//连接

			string str;

			if(autoscroll)m_respond.AppendBodyHtmlScroll();
			m_respond.AppendBody("<HR></HR>\n");
			m_respond.AppendBodyHtmlCopyright();

			char buf[256];
			sprintf(buf, "\n<BR><code>服务器最后更新：%s&nbsp;&nbsp;(页面处理用时 %ld 秒，其中CPU时间 %f 秒)</code>"
				, LAST_UPDATE, time(NULL) - pCD->m_time, (clock() - pCD->m_clock) / (float)CLOCKS_PER_SEC);
			str+=buf;

			m_respond.AppendBody(str);
			m_respond.AppendBodyHtmlEnd();
			m_respond.AddHeaderContentLength();
			if(m_s.IsConnected())
			{
				if(!m_respond.Flush(m_s))
				{
					LOG<<"send error"<<ENDE;
					m_s.Close();
					return false;
				}
			}
			return true;
		}
		void ShowFunctionList(CConnectData * pCD, bool showAll)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			//CMySocket & m_s = pCD->m_s;//连接

			CHtmlDoc::CHtmlTable2 table;
			table.SetTitle("功能目录");
			table.AddCol("命令说明");
			table.AddCol("参数",CHtmlDoc::CHtmlDoc_DATACLASS_HTML);
			table.AddCol("执行命令",CHtmlDoc::CHtmlDoc_DATACLASS_HTML);
			if(showAll)
			{
				table.AddCol("名称",CHtmlDoc::CHtmlDoc_DATACLASS_TEXT);
				table.AddCol("隐藏",CHtmlDoc::CHtmlDoc_DATACLASS_TEXT);
				table.AddCol("标准",CHtmlDoc::CHtmlDoc_DATACLASS_TEXT);
				table.AddCol("队列",CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			}

			map<long, vector<CWebCommand *> > list_by_seq;
			for (TYPE_WEB_COMMANDS::const_iterator it = m_commands.begin(); it != m_commands.end(); ++it)
			{
				list_by_seq[it->second.seq].push_back(it->second.pWebCommand);
			}
			for(map<long, vector<CWebCommand *> >::const_iterator it_list=list_by_seq.begin();it_list!=list_by_seq.end();++it_list)
			{
				for (vector<CWebCommand *>::const_iterator it = it_list->second.begin(); it != it_list->second.end(); ++it)
				{
					CWebCommand * p = *it;
					if (!showAll && p->notonhomepage)continue;

					long line = table.AddLine();
					table.SetData(line, 0, p->name + "\n" + p->note);
					table.SetData(line, 1, p->toHtmlFormInput(true, (showAll ? true : false), &m_request));
					table.SetData(line, 2, p->toHtmlFormSubmit(pCD->m_isMgrPort && !p->isAdmin, false));
					if (showAll)
					{
						string str;

						table.AddData(p->command_id + ".asp");
						table.AddData(p->notonhomepage ? "主页隐藏" : "");
						table.AddData(p->NotStdPage ? "自定义" : "");
						if (p->queue < 0) table.AddData("");
						else table.AddData(p->queue);
					}
				}
			}
			m_respond.AppendBody(table.MakeHtmlTable());
			m_respond.AppendBody("<P>\n");
		}
		//独立检查管理员密码
		bool CheckAdminByForm(CConnectData * pCD)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			//CHttpRespond & m_respond = pCD->m_respond;//应答
			//CMySocket & m_s = pCD->m_s;//连接
			
			return pfCheckAdmin(m_request.GetParam("username").c_str(), m_request.GetParam("password").c_str());
		}
		//独立检查管理员密码的INPUT
		string _FormInputAdminPass()
		{
			return "用户名<INPUT TYPE=\"input\" SIZE=\"30\" NAME=\"username\" >\n"
				"口令<INPUT TYPE=\"password\" SIZE=\"30\" NAME=\"password\" >\n";
		}
		bool ShowStopServer(CConnectData * pCD)
		{
			//CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			//CMySocket & m_s = pCD->m_s;//连接

			string str;

			//str="<BR><BR><A href=\"/shell.asp?curdir="+m_root+"\" target=\"_blank\">Shell</A><P>\n";
			//m_respond.AppendBody(str);

			str="<BR><BR><FORM ACTION=\"/StopServer.asp\" METHOD=\"POST\" target=\"_blank\">\n"
				"<CODE>关闭服务器需要口令：</CODE><BR>" + _FormInputAdminPass() +
				"<INPUT TYPE=SUBMIT VALUE=\"关闭服务器\" >\n"
				"</FORM>\n";
			m_respond.AppendBody(str);

			return true;
		}
		bool doPageDefault(CConnectData * pCD)
		{
			ShowFunctionList(pCD,false);
			ShowStopServer(pCD);
			return true;
		}
		bool doPageFile(CConnectData * pCD, char const * file = NULL)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			CMySocket & m_s = pCD->m_s;//连接

			fstream f;
			long bufsize=1024*1024;
			char * buf;
			long count,len;

			buf=new char[bufsize];
			if(NULL==buf)
			{
				LOG<<"内存不足"<<ENDE;
				return true;
			}

			m_respond.AddHeaderExpires(time(NULL),60);//60秒过期

			string filename;
			if(NULL==file || strlen(file)==0)
			{
				filename=m_request.GetResource();
				DEBUG_LOG<<m_pServerDatas->m_root.c_str()<<ENDI;
				if('/'==filename[filename.size()-1])
				{//对于目录要打开默认页
					filename+="default.htm";
				}
				if(0==m_pServerDatas->m_root.size())
				{
					delete[] buf;
					return false;
				}
				if('/'==filename[0])filename.erase(0,1);
				filename=m_pServerDatas->m_root.c_str()+filename;
				//检查是否是目录
#ifndef _MS_VC
				struct stat statbuf;
				if(0==stat(filename.c_str(),&statbuf))
				{
					if(S_ISDIR(statbuf.st_mode))
					{
						OnPageStart(pCD, "404 NOT FOUND");
						m_respond.AppendBody("请求的资源是目录,请在资源最后增加\"/\"");
						OnPageEnd(pCD);
						delete[] buf;
						return true;
					}
				}
#endif
			}
			else
			{
				filename=file;

				string filetitle;
				filetitle=filename.substr(filename.npos==filename.find_last_of("/")?0:filename.find_last_of("/")+1);
				m_respond.AddHeader("Content-disposition","attachment; filename="+filetitle);
			}
			DEBUG_LOG<<filename.c_str()<<ENDI;

			f.open(filename.c_str(),ios::in|ios::binary);
			if(!f.good())
			{
				OnPageStart(pCD, "404 NOT FOUND");
				m_respond.AppendBody("<P>404 NOT FOUND<P>请求的资源不存在<P>");
				OnPageEnd(pCD);
				delete[] buf;
				return false;
			}
			f.seekg(0,ios::end);
			len=f.tellg();
			m_respond.AddHeaderContentTypeByFilename(filename.c_str());
			m_respond.AddHeaderContentLength(len);
			f.seekg(0,ios::beg);
			if(!m_respond.Flush(m_s))
			{
				delete[] buf;
				return false;
			}
			while(f.good())
			{
				if(len-f.tellg()>bufsize-1)count=bufsize-1;
				else count=len-f.tellg();
				if(0==count)break;
				f.read(buf,count);
				if(!m_s.Send(buf,count))
				{
					m_s.Close();
					break;
				}
			}
			f.close();

			delete[] buf;
			return true;
		}
		bool doPageUpload(CConnectData* pCD)
		{
			CHttpRequest& m_request = pCD->m_request;//请求
			CHttpRespond& m_respond = pCD->m_respond;//应答
			CMySocket& m_s = pCD->m_s;//连接

			m_respond.Init();
			m_respond.AddHeaderNoCache();
			m_respond.AddHeaderContentTypeByFilename("*.html");
			m_respond.AppendBodyHtmlStart("Uplaod");
			m_respond.AppendBody(m_request.RequestHtmlReport());
			m_respond.AppendBody("<HR/>");
			m_respond.AppendBody(m_request.GetFullRequest());
			m_respond.AppendBody("<HR/>");
			m_respond.AppendBody(m_request.GetContent().data());
			m_respond.AppendBody("<HR/>");

			string content_type = m_request.GetContentType();
			string a = "multipart/form-data; boundary=";
			string boundary;
			size_t pos = content_type.find(a);
			if (0 != pos)
			{
				m_respond.AppendBody("未识别的内容类型，仅支持 multipart/form-data<P>");
			}
			else
			{
				boundary = content_type.substr(a.size());
				string boundary_begin = "--" + boundary + "\r\n";
				string boundary_end = "--" + boundary + "--\r\n";
				size_t part_pos;
				size_t pos_next_find = 0;
				while (CBuffer::npos != (part_pos = m_request.GetContent().find(boundary_begin, pos_next_find)))
				{
					size_t part_end = m_request.GetContent().find(boundary_end, part_pos + boundary_begin.size());
					if (CBuffer::npos == part_end)
					{
						m_respond.AppendBody("数据内容不完整<P>");
						break;
					}

					size_t part_header_end = m_request.GetContent().find("\r\n\r\n");
					string part_header = m_request.GetContent().substr(part_pos + boundary_begin.size(), part_header_end - part_pos - boundary_begin.size());
					size_t pos_file_name_begin;
					string filename_head="filename=\"";
					pos_file_name_begin = part_header.find(filename_head);
					if (string::npos == pos_file_name_begin)
					{
						m_respond.AppendBody("没有文件名<P>");
					}
					else
					{
						size_t pos_file_name_end = part_header.find("\"", pos_file_name_begin + filename_head.size());
						if (string::npos == pos_file_name_end)
						{
							m_respond.AppendBody("文件名格式问题<P>");
						}
						else
						{
							//thelog << pos_file_name_begin << " " << pos_file_name_end << " " << pos_file_name_end - pos_file_name_begin - filename_head.size() << endi;
							string filename = part_header.substr(pos_file_name_begin + filename_head.size(), pos_file_name_end - pos_file_name_begin - filename_head.size());
							if (filename.size() == 0)
							{
								m_respond.AppendBody("没有文件名<P>");
							}
							else
							{
								m_respond.AppendBody(filename);
								CEasyFile file;
								size_t pos_filedata = part_header_end + 4;
								long filesize = part_end - pos_filedata - 2;//前面还有\r\n
								if (!file.WriteFile(filename.c_str(), m_request.GetContent().data() + pos_filedata, filesize))
								{
									m_respond.AppendBody("写入失败<P>");
								}
								else
								{
									char buf[256];
									sprintf(buf, "写入成功 字节数%ld<P>", filesize);
									m_respond.AppendBody(buf);
									char path[1024];
									m_respond.AppendBody(getcwd(path, 1024));
								}
							}
						}
					}

					pos_next_find = part_end + boundary_end.size();
					m_respond.AppendBody("<HR />");
				}
			}

			string form = "<form id=\"upload-form\" action=\"/Upload.asp\" method=\"post\" enctype=\"multipart/form-data\" >\
				<input type=\"file\" id=\"upload\" name=\"upload\" /> <br />\
				<input type=\"submit\" value=\"Upload\" />\
				</form>";
			m_respond.AppendBody(form);
	
			m_respond.AppendBodyHtmlEnd();
			m_respond.Flush(m_s);
			return true;
		}
		//无限页面，需要判断是否需要结束
		bool doPageAdmin(CConnectData * pCD)
		{
			//CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			CMySocket & m_s = pCD->m_s;//连接

			m_respond.Init();
			m_respond.AddHeaderNoCache();
			m_respond.AddHeaderContentTypeByFilename("*.html");
			m_respond.AppendBodyHtmlStart("Admin");

			string str;

			map<long, CHtmlDoc::CHtmlTable2 >  oldtables;
			map<long, CHtmlDoc::CHtmlTable2 >  oldtables2;

			//第一次输出表格
			m_pServerControlBlock->SSCB_Output(oldtables, oldtables2, CHtmlDoc::CHtmlTable2::OUTPUT_HTML, str);
			m_respond.AppendBody(str);
			ShowStopServer(pCD);
		
			long count;
			for (count = 0; (count) < 60; ++count)
			{
				//输出更新脚本
				if (!m_pServerControlBlock->SSCB_Output(oldtables, oldtables2, CHtmlDoc::CHtmlTable2::OUTPUT_SCRIPT, str))
				{
					//生成更新脚本失败，通常是行数增加
					break;
				}
				m_respond.AppendBody(str);

				if (!m_respond.Flush(m_s))
				{
					m_s.Close();
					break;
				}

				if (isShutDown() && m_pServerControlBlock->GetChildCount() <= 1)
				{
					m_respond.AppendBody("<script language=\"JavaScript\">alert(\"服务已停止\");</script>");
					m_respond.AppendBodyHtmlEnd();
					m_respond.Flush(m_s);
					m_s.Close();
					return true;
				}
				SleepSeconds(1);
			}
			m_respond.AppendBody("<script language=\"JavaScript\">window.location.reload();</script>");
			m_respond.AppendBodyHtmlEnd();
			m_respond.Flush(m_s);
			m_s.Close();
			return true;
		}
		bool doPageFunctionList(CConnectData * pCD)
		{
			//CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			//CMySocket & m_s = pCD->m_s;//连接

			m_respond.AppendBody("全部功能清单（比主页的全）");
			m_respond.AppendBody("<P>\n");
			ShowFunctionList(pCD,true);
			return true;
		}
		bool doPageBedRequest(CConnectData * pCD)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			//CMySocket & m_s = pCD->m_s;//连接

			string str;
			str=m_request.GetFullRequest();
			str+="<P>以上请求无法理解<P>";
			m_respond.AppendBody(str);
			return true;
		}
		//查找命令，引用参数i返回命令序号
		web_command_private_struce * FindCommand(CConnectData * pCD)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			//CHttpRespond & m_respond = pCD->m_respond;//应答
			//CMySocket & m_s = pCD->m_s;//连接

			string page=m_request.GetResource();
			string type=m_request.GetResourceType();
			if (0!=type.size())page.erase(page.size()-type.size()-1);

			TYPE_WEB_COMMANDS::iterator it = m_commands.find(page);
			if (it != m_commands.end())
			{
				return &it->second;
			}
			else return NULL;
		}
		bool _doPageFunction(CConnectData * pCD, web_command_private_struce * pCmdInfo)
		{
			CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			CMySocket & m_s = pCD->m_s;//连接

			long i_cmd = pCmdInfo->seq;
			CWebCommand * pCmd = pCmdInfo->pWebCommand;
			//处理用户系统内部检查
			string err;

			++m_pServerDatas->m_web_command_data_s[i_cmd].count;//执行计数

			if (!pCmdInfo->isInited)
			{
				if (!pCmd->InitWebFunction())
				{
					LOG << pCmd->command_id << " 初始化失败" << ENDE;
					OnPageStart(pCD,"500 Internal Server Error");
					m_respond.AppendBody(CHtmlDoc::HTMLEncode(err));
					OnPageEnd(pCD);
					return false;
				}
				else
				{
					DEBUG_LOG << pCmd->command_id << " 初始化成功" << ENDI;
					pCmdInfo->isInited = true;
				}
			}

			if (pCmd->NotStdPage)
			{
				if (!pCmd->doWebFunction(&m_request, m_s, &m_respond))
				{
					++m_pServerDatas->m_web_command_data_s[i_cmd].count_err;//出错计数
				}
			}
			else
			{
				//文件头
				OnPageStart(pCD,pCmd->name.c_str());
				DEBUG_LOG << "发送应答头" << ENDI;
				if (!m_respond.Flush(m_s))return true;

				if (!pCmd->doWebFunction(&m_request,m_s,&m_respond))
				{
					++m_pServerDatas->m_web_command_data_s[i_cmd].count_err;//出错计数
				}

				//文件尾
				m_respond.AppendBody("<P><HR></HR>");
				m_respond.AppendBody(pCmd->name);
				m_respond.AppendBody("<BR>");
				m_respond.AppendBody(pCmd->note);
				m_respond.AppendBody("<BR>");
				if (!pCmd->hide)
				{
					pCmd->SetParamValueFromRequest(m_request);
					m_respond.AppendBody(pCmd->toHtmlFormInput(false, false, &m_request));
					m_respond.AppendBody("<INPUT TYPE=\"hidden\" NAME=\"autorefresh\" >\n");
					m_respond.AppendBody(pCmd->toHtmlFormSubmit(false, true));

					if (pCmd->AutoRefresh)
					{
						char buf2[2048];
						bool isAutoRefresh = (m_request.GetParam("autorefresh") == "true");
						long refreshtimeout = atol(m_request.GetParam("refreshtimeout").c_str());
						if (refreshtimeout <= 0)refreshtimeout = 10;
						sprintf(buf2,
							"<BUTTON NAME=\"autorefresh\" onclick=\"timer=setTimeout('window.%s.submit()',%ld);window.%s.autorefresh.value='true';window.autorefresh.disabled=true;window.stoprefresh.disabled=false;\">自动刷新</BUTTON>\n"
							"<button NAME=\"stoprefresh\" disabled onclick=\"clearTimeout(timer);window.%s.autorefresh.value='';window.autorefresh.disabled=false;window.stoprefresh.disabled=true;\">停止刷新</BUTTON><BR>\n"
							"%s<BR>\n"
							, pCmd->GetFormName().c_str(), refreshtimeout * 1000, pCmd->GetFormName().c_str(), pCmd->GetFormName().c_str(), (isAutoRefresh ? "<script language=\"JavaScript\">window.autorefresh.onclick()</script>" : ""));
						m_respond.AppendBody(buf2);
					}
				}
				OnPageEnd(pCD);
			}
			return true;
		}
		bool doPageFunction(CConnectData * pCD)
		{
			//CHttpRequest & m_request = pCD->m_request;//请求
			CHttpRespond & m_respond = pCD->m_respond;//应答
			CMySocket & m_s = pCD->m_s;//连接

			web_command_private_struce * pCmdInfo = FindCommand(pCD);
			if(NULL==pCmdInfo)
			{
				m_respond.Clear();
				m_respond.Send404(m_s);
				return true;
			}

			_doPageFunction(pCD,pCmdInfo);
			return true;
		}
		bool _AddWebCommand(CWebCommand * web_cmd)
		{
			if(!web_cmd->isVirtual)
			{
				if (web_cmd->isAdmin)web_cmd->command_id = "/admin/" + web_cmd->command_id;
				else web_cmd->command_id = "/bin/" + web_cmd->command_id;
			}
			if (m_commands.end() != m_commands.find(web_cmd->command_id))
			{
				LOG << web_cmd->command_id << " 已经存在" << ENDE;
				return false;
			}
			if (web_cmd->queue >= 0)
			{
				if (m_queue_command.end() != m_queue_command.find(web_cmd->queue))
				{
					LOG << web_cmd->command_id << " 的队列号已经被使用 " << web_cmd->queue << ENDE;
					return false;
				}
			}

			long i = m_commands.size();
			if (i >= CHttpServerDatas_MAX)
			{
				LOG << m_commands.size() << " 已经达到支持的最大数量：" << CHttpServerDatas_MAX << ENDE;
				return false;
			}
			web_command_private_struce tmp;
			tmp.seq = i;
			tmp.isInited = false;
			tmp.pWebCommand = web_cmd;
			m_commands[web_cmd->command_id] = tmp;
			if (web_cmd->queue >= 0)m_queue_command[web_cmd->queue] = &m_commands[web_cmd->command_id];
			return true;
		}
		bool AddWebCommandAdmin(CWebCommand * web_cmd)
		{
			web_cmd->isVirtual = false;
			web_cmd->isAdmin = true;
			return _AddWebCommand(web_cmd);
		}
		bool AddWebCommandVirtual(CWebCommand * web_cmd)
		{
			web_cmd->isVirtual = true;
			web_cmd->isAdmin = true;
			return _AddWebCommand(web_cmd);
		}
		static bool isNeedBreak()
		{
			return G_pHttpServerData->cmd_bShutDown;
		}
	private:
		SocketServerControlBlock * m_pServerControlBlock;//总共享内存入口
		CHttpServerDatas * m_pServerDatas;//服务自定义块的入口
		TYPE_WEB_COMMANDS m_commands;//命令组
		TYPE_QUEUE_WEB_COMMANDS m_queue_command;//消息队列-命令映射关系，仅包含queue>0的，0为默认测试
		bool (*pfCheckUser)(char const * _user,char const * _pass);//检查普通用户口令
		bool (*pfCheckAdmin)(char const * _user,char const * _pass);//检查管理员密码
	
		CMyShmMutex m_WebCommandMutex;//用于多进程页面间的互斥，主要用于实现统计数据的准确性
		CWebCommand_SetDebug cmd_SetDebug;
		CWebCommand_SetMax cmd_SetMax;
		CWebCommand_Pause cmd_Pause;
		CWebCommand_ShowDir cmd_ShowDir;
		//这一组是虚拟命令，只用来提供页面入口
		CWebCommand_vShell cmd_shell;
		CWebCommand_vSSH cmd_ssh;
		CWebCommand_vViewFile cmd_ViewFile;
		CWebCommand_vDownFile cmd_Downfile;
	public:
		bool CHttpProcessInit(string const & realm)
		{
			pfCheckUser = (NULL);
			pfCheckAdmin = (NULL);
			m_pServerControlBlock = (NULL);
			m_pServerDatas = (NULL);
			//m_pThisChildData = (NULL);

			m_pServerControlBlock = G_pHttpServerData;
			if (NULL == m_pServerControlBlock)
			{
				LOG << "服务控制块未创建" << ENDE;
				return false;
			}
			m_pServerDatas = &m_pServerControlBlock->server_data;
			if(!m_WebCommandMutex.Attach(&m_pServerDatas->m_webcommand_sem))
			{
				LOG << "信号量连接失败" << ENDE;
				return false;
			}
			if (0 == m_pServerDatas->m_realm.size())
			{//非克隆
				m_pServerDatas->m_realm = realm;
			}

			if (!AddWebCommandAdmin(&cmd_SetDebug)) { thelog << "命令已经存在" << ende; return false; }
			if (!AddWebCommandAdmin(&cmd_SetMax)) { thelog << "命令已经存在" << ende; return false; }
			if (!AddWebCommandAdmin(&cmd_Pause)) { thelog << "命令已经存在" << ende; return false; }
			if (!AddWebCommandAdmin(&cmd_ShowDir)) { thelog << "命令已经存在" << ende; return false; }

			if (!AddWebCommandVirtual(&cmd_shell)) { thelog << "命令已经存在" << ende; return false; }
			if (!AddWebCommandVirtual(&cmd_ssh)) { thelog << "命令已经存在" << ende; return false; }
			if (!AddWebCommandVirtual(&cmd_ViewFile)) { thelog << "命令已经存在" << ende; return false; }
			if (!AddWebCommandVirtual(&cmd_Downfile)) { thelog << "命令已经存在" << ende; return false; }

			return true;
		}
		void SetRoot(char const * root)
		{
			string str = root;
			if(str.size()!=0 && str[str.size()-1]!='/')str+="/";
			m_pServerDatas->m_root=str;
		}
		//添加命令，queue不为0的是消息中转
		bool AddWebCommand(CWebCommand * web_cmd)
		{
			web_cmd->isVirtual = false;
			web_cmd->isAdmin = false;
			return _AddWebCommand(web_cmd);
		}
		void SetCheckUser(bool(*CheckUser)(char const * _user, char const * _pass)) { pfCheckUser = CheckUser; }
		void SetCheckAdmin(bool(*CheckAdmin)(char const * _user, char const * _pass)) { pfCheckAdmin = CheckAdmin; }
		void SetSiteName(char const * name){m_pServerDatas->m_sitename=name;}
		TYPE_QUEUE_WEB_COMMANDS * getTYPE_QUEUE_WEB_COMMANDS() { return &m_queue_command; }
		bool _InitAllCommands()
		{
			TYPE_WEB_COMMANDS::iterator it;
			for (it = m_commands.begin(); it != m_commands.end(); ++it)
			{
				CWebCommand * p = it->second.pWebCommand;
				if (!p->InitWebFunction())
				{
					LOG << p->command_id << " 初始化失败" << ENDE;
					return false;
				}
				else
				{
					LOG << p->command_id << " 初始化成功" << ENDI;
				}
			}
			return true;
		}
	public://ISocketServerProcess
#ifdef ENABLE_HTTPS //定义ssl的ctx
		CmySSLTLS ctx;
#endif
		//服务开始时调用（主进程）
		virtual bool OnStartServer()
		{
#ifdef ENABLE_HTTPS //初始化ssl的ctx
			if (!ctx.Init_SSL_CTX())
			{
				return false;
			}
#endif
			TYPE_WEB_COMMANDS::iterator it;
			for (it = m_commands.begin(); it != m_commands.end(); ++it)
			{
				m_pServerDatas->m_web_command_data_s[it->second.seq].command_id = it->second.pWebCommand->command_id;
			}

			if(!CCurrentProcess::MultiProcessMode())
			{//多线程模式需要预先初始化，否则并发初始化会引发错误，多进程模式无此问题
				return _InitAllCommands();
			}
			return true;
		}
		//服务结束时调用（主进程）
		virtual bool OnStopServer()
		{
#ifdef ENABLE_HTTPS //释放ssl的ctx
			ctx.free_SSL_CTX();
#endif
			if (!CCurrentProcess::MultiProcessMode())
			{//多线程模式需要在此卸载
				return OnStopChildSocketProcess();
			}
			return true;
		}
		//开始新的子进程时调用
		virtual bool OnStartChildSocketProcess()
		{
			//if(!_InitAllCommands())return false;
			return true;
		}
		//结束新的子进程时调用
		virtual bool OnStopChildSocketProcess()
		{
			TYPE_WEB_COMMANDS::iterator it;
			for (it = m_commands.begin(); it != m_commands.end(); ++it)
			{
				CWebCommand * p = it->second.pWebCommand;
				if (!it->second.isInited)
				{
					DEBUG_LOG << p->command_id << " 未曾初始化" << ENDI;
					continue;
				}
				if (!p->UnInitWebFunction())
				{
					LOG << p->command_id << " 卸载失败" << ENDE;
					return false;
				}
				else
				{
					DEBUG_LOG << p->command_id << " 卸载成功" << ENDI;
				}
			}
			return true;
		}
		//处理一个已经建立的连接
		virtual bool SocketProcess(bool _isMgrPort, CMySocket & _s, long * pRet, long i_child)
		{
			CConnectData connectdata;

			CMySocket & m_s = connectdata.m_s;//连接
			bool & m_isMgrPort = connectdata.m_isMgrPort;//是否是管理端口
			//clock_t & m_clock = connectdata.m_clock;//时钟，记录处理页面花费的CPU时间
			//time_t & m_time = connectdata.m_time;//时间，记录总处理用时
			CHttpRequest & m_request = connectdata.m_request;//请求
			CHttpRespond & m_respond = connectdata.m_respond;//应答
			//CHttpClient & m_httpclient = connectdata.m_httpclient;//用于热备系统连接备机
			long & m_i_child = connectdata.m_i_child;//进程或线程的序号
			CHttpChildData * & m_pThisChildData = connectdata.m_pThisChildData;

			m_s=_s;
			m_isMgrPort= _isMgrPort;
			m_i_child = i_child;
			m_pThisChildData = &m_pServerControlBlock->child_datas[i_child];

			m_pThisChildData->peer_info=_s.GetPeerInfo();

#ifdef ENABLE_HTTPS //获得SSL
			m_s.ssl = this->ctx.getSSL(m_s.GetFD());
#endif
			//支持HTTP1.1，一个连接处理多个请求
			while (m_s.IsConnected())
			{
				G_CLEAR_ERROR;
				bool isReady = false;
				m_pThisChildData->SetHttpProcessInfo("wait...");
				if (!m_s.IsSocketReadReady2(1, isReady) || isChildNeedExit(m_isMgrPort))
				{
					DEBUG_LOG<<"socket error or need exit"<<ENDE;
					m_s.Close();
					return true;
				}
				if (!isReady)
				{
					//DEBUG_LOG << "socekt not ready On HTTP Process" << ENDI;
					continue;
				}

				bool isKeepAlive=false;
				m_request.Clear();
				m_respond.Init();

				m_pThisChildData->SetHttpProcessInfo("RecvRequest...");
				if (!m_request.RecvRequest(m_s, NULL, isNeedBreak))
				{
					if(m_s.IsConnected())
					{
						m_pThisChildData->SetHttpProcessInfo("错误的请求");
						LOG << getpid() << "错误的请求：" << m_request.GetFullRequest() << ENDE;
						doPageBedRequest(&connectdata);
						m_s.Close();
					}
					else
					{
						m_pThisChildData->SetHttpProcessInfo("连接已关闭");
						DEBUG_LOG << getpid() << "客户关闭连接" << ENDI;
					}
					break;
				}
			
				++m_pThisChildData->request_count;
				DEBUG_LOG<<getpid()<<"接收到请求，接连信息：\n"<<m_s.debuginfo()<<ENDI;
				DEBUG_LOG<<getpid()<<"接收到请求，请求信息：\n"<<m_request.GetFullRequest()<<ENDI;
				m_pThisChildData->SetHttpProcessInfo(m_request.GetResource().c_str());

				//检查是否需要保持连接
				if(m_request.GetHeader("Connection")=="Keep-Alive")
				{
					DEBUG_LOG<<getpid()<<"保持连接"<<ENDI;
					isKeepAlive=true;
				}
				else
				{
					DEBUG_LOG<<"不保持连接"<<ENDI;
					isKeepAlive=false;
				}

				if ("/" == m_request.GetResource())
				{
					m_respond.Send302(m_s, "/login/login.htm");
					//m_respond.Send302(m_s,"/default.asp");
					if (isKeepAlive)continue;
					else
					{
						m_s.Close();
						break;
					}
				}

				//处理用户认证，登录用的目录不需要认证，登录目录可以用XMLHttp来实现整合登录
				//login目录下的内容无需认证，这要求login下引用的图片也必须在login目录下
				if(NULL!=this->pfCheckUser)
				{
					char const * logondir = "/login/";
					char const * cssdir = "/css/";
					if (0 != strncmp(logondir, m_request.GetResource().c_str(), strlen(logondir))
						&& 0 != strncmp(cssdir, m_request.GetResource().c_str(), strlen(cssdir)))
					{
						string user;
						string password;
						//thelog << endi;
						if(m_request.GetAuthorization(user,password))
						{
							//thelog << endi;
							if(!pfCheckUser(user.c_str(),password.c_str()))
							{
								m_respond.Send401(m_s,m_pServerDatas->m_realm.c_str());
								if(isKeepAlive)continue;
								else
								{
									m_s.Close();
									break;
								}
							}
							string cookie="logon_user";
							if(m_request.GetCookie(cookie)!=user)m_respond.AddCookie(cookie,user);
						}
						else
						{
							m_respond.Send401(m_s,m_pServerDatas->m_realm.c_str());
							if(isKeepAlive)continue;
							else
							{
								m_s.Close();
								break;
							}
						}
					}
				}

				if((!m_isMgrPort && m_request.GetResource().substr(0,5)=="/bin/") || m_request.GetResource().substr(0,7)=="/admin/")
				{
					//执行用户功能
					doPageFunction(&connectdata);//内置页面
				}
				else if("/default.asp"==m_request.GetResource() || "/default.htm"==m_request.GetResource())
				{
					OnPageStart(&connectdata, "default");
					doPageDefault(&connectdata);
					OnPageEnd(&connectdata,false);
				}
				else if("/functionlist.asp"==m_request.GetResource())
				{
					OnPageStart(&connectdata, "Function List");
					doPageFunctionList(&connectdata);
					OnPageEnd(&connectdata,false);
				}
				else if("/admin.asp"==m_request.GetResource())
				{
					doPageAdmin(&connectdata);
				}
				else if ("/StopServer.asp" == m_request.GetResource())
				{//关闭服务器
					OnPageStart(&connectdata,"exit cluster", true);
					if (NULL != pfCheckAdmin && !CheckAdminByForm(&connectdata))
					{
						m_respond.AppendBody("口令错误");
						OnPageEnd(&connectdata);
					}
					else
					{
						m_pServerControlBlock->cmd_bShutDown = true;
						m_respond.AppendBody("收到命令，服务正在停止......");
						m_respond.Flush(m_s);
						OnPageEnd(&connectdata);
					}
					isKeepAlive = false;
					m_s.Close();
				}
				else if ("/shell.asp" == m_request.GetResource())
				{
					OnPageStart(&connectdata,"shell");
					CHttpShell shell(m_s, m_request, m_respond);
					shell.doPageShell();
					OnPageEnd(&connectdata);
					m_s.Close();//所有此类页面都可能无法预先确定输出长度
					isKeepAlive = false;
				}
				else if ("/ssh.asp" == m_request.GetResource())
				{
					OnPageStart(&connectdata,"ssh");
					CHttpShell shell(m_s, m_request, m_respond);
					shell.doPageSSH();
					OnPageEnd(&connectdata);
					m_s.Close();//所有此类页面都可能无法预先确定输出长度
					isKeepAlive = false;
				}
				else if ("/ViewFile.asp" == m_request.GetResource())
				{
					char buf[2048];
					sprintf(buf, "查看文件 %s ", m_request.GetParam("file").c_str());
					OnPageStart(&connectdata,buf);
					CHttpShell shell(m_s, m_request, m_respond);
					shell.doPageViewFile();
					OnPageEnd(&connectdata);
					m_s.Close();//所有此类页面都可能无法预先确定输出长度
					isKeepAlive = false;
				}
				else if ("/DownFile.asp" == m_request.GetResource())
				{
					if (doPageFile(&connectdata, m_request.GetParam("file").c_str()))
					{
						m_respond.Flush(m_s);
					}
					else
					{
						m_respond.Flush(m_s);
						m_s.Close();//所有此类页面都可能无法预先确定输出长度
						isKeepAlive = false;
					}
				}
				else if ("/Upload.asp" == m_request.GetResource())
				{
					if (doPageUpload(&connectdata))
					{
						m_respond.Flush(m_s);
					}
					else
					{
						m_respond.Flush(m_s);
						m_s.Close();//所有此类页面都可能无法预先确定输出长度
						isKeepAlive = false;
					}
				}
				else
				{
					doPageFile(&connectdata);
					m_respond.Flush(m_s);
				}

				//客户指定不保持连接或应答不支持保持连接则关闭连接
				if (!isKeepAlive || !m_respond.isCanKeepAlive() || isChildNeedExit(m_isMgrPort))
				{
					m_s.Close();
					break;
				}
			}

#ifdef ENABLE_HTTPS //释放SSL
			this->ctx.freeSSL(m_s.ssl);
			m_s.ssl = NULL;
#endif
			return true;
		}
	};

}
