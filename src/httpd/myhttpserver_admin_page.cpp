//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "myhttpserver_admin_page.h"

namespace ns_my_std
{
	int CWebCommand_ShowDir::doDirBegin(char const * dirname, long deep)
	{
		string px;
		for (long i = 0; i < deep * 8; ++i)px += "&nbsp;";

		string tmp = dirname;
		tmp.erase(0, strlen(getenv("HOME")));
		if (tmp.size() > 0 && '/' == tmp[tmp.size() - 1])tmp.erase(tmp.size() - 1);
		string::size_type pos = tmp.find_last_of("/");
		if (pos == tmp.npos)return 0;//没有父目录了

		tmp.erase(pos);
		char buf[2048];
		sprintf(buf, "<DIV><A href=\"/admin/ShowDir.asp?dir=%s\">%s</A><BR/>\n", tmp.c_str(), "..");
		output += px + buf;
		return 0;
	}
	int CWebCommand_ShowDir::doDirEnd(char const * dirname, long deep)
	{
		string px;
		for (long i = 0; i < deep * 8; ++i)px += "&nbsp;";

		char buf[2048];
		sprintf(buf, "</DIV>\n");
		output += px + buf;
		return 0;
	}
	int CWebCommand_ShowDir::doOneFile(char const * filename, bool isDir, long deep)
	{
		string px;
		for (long i = 0; i < deep * 8; ++i)px += "&nbsp;";

		string tmp = filename;
		tmp.erase(0, strlen(getenv("HOME")));

		string file;
		string::size_type pos = tmp.find_last_of("/");
		if (pos != tmp.npos)file = tmp.substr(pos + 1);
		else file = tmp;


		char buf[2048];
		if (isDir)
		{
			sprintf(buf, "<B><A href=\"/admin/ShowDir.asp?dir=%s\">%s</A></B><BR/>\n", tmp.c_str(), file.c_str());
		}
		else
		{
			sprintf(buf, "<A target=\"_blank\" href=\"/ViewFile.asp?file=%s\">%s</A><BR/>\n", filename, file.c_str());
		}
		output += px + buf;
		return 0;
	}
	bool CWebCommand_ShowDir::doWebFunction(CHttpRequest const * pRequest, CMySocket & s, CHttpRespond * pRespond)
	{
		dir = pRequest->GetParam("dir");
		filter = pRequest->GetParam("filter");
		r = (0 != pRequest->GetParam("r").size());
		pRespond->AppendBody(dir);
		pRespond->AppendBody("<BR/>");
		pRespond->AppendBody(filter);
		pRespond->AppendBody("<BR/>");
		pRespond->AppendBody(r?"递归":"不递归");
		pRespond->AppendBody("<BR/>");

		string _dir = getenv("HOME");
		if (_dir.size() > 0 && _dir[_dir.size() - 1] != '/')_dir += "/";
		if (dir.size() > 0 && '/' == dir[0])_dir.erase(_dir.size() - 1);
		_dir += dir;
		if (_dir != "/" && '/' == _dir[_dir.size() - 1])_dir.erase(_dir.size() - 1);//除了“/”之外不带末尾的'/'

		output="";
		long count=0;
		long count_err=0;
		doForEachDir(_dir.c_str(),false,r,count,count_err);
		pRespond->AppendBody(output);
		
		return true;
	}
}
