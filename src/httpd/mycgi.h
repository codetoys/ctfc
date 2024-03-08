//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "myhttpbase.h"
using namespace ns_my_std;

/*
说明：
实现一个cgi应实现IHttpPluginAsmx
IHttpPluginAsmx的_asmx_exec返回一个table或者xmlsource，取决于rettype
获得查询参数和/或form内容的方式类似：string _sub_query_=pRequest->GetParam("_sub_query_");
table支持输出为html表格或xml
cgi程序所有的控制台输出都将输出到浏览器，所以不可使用控制台输出（InitActiveCgi已经预先禁用了日志的控制台输出）

runCGI自动跟据_asmx_exec的返回数据和请求的资源类型确定输出方式
	_asmx_exec返回RET_XMLSOURCE则返回xml类型，数据为xmlsource
	aspx资源返回html表格
	asmx资源返回xml
*/

class IHttpPluginAsmx
{
public:
	enum RET_TYPE { RET_STD, RET_XMLSOURCE };
public:
	//重载此函数以实现业务功能，RET_XMLSOURCE用xmlsource返回结果
	virtual void _asmx_exec(CHtmlDoc::CHtmlTable2& table, CHttpRequest const* pRequest, RET_TYPE* rettype, string* xmlsource) = 0;
private:
	void ShowArg(int argc, char** argv)
	{
		int i;
		thelog << "命令行:" << endl;
		for (i = 0; i < argc; ++i)
		{
			theLog << argv[i] << " ";
		}
		theLog << endi;
	}
	void ShowEnv()
	{
		char** penv = environ;
		string str;
		thelog << "环境变量:" << endl;
		while (*penv)
		{
			str = *penv;
			theLog << str << endl;
			++penv;
		}
		theLog << endi;
	}
public:
	bool runCGI(int argc, char** argv)
	{
		CHttpRequest request;
		CHttpRespond respond;
		CHttpRequest* pRequest = &request;
		CHttpRespond* pRespond = &respond;

		ShowArg(argc, argv);
		ShowEnv();
		pRequest->InitCgiRequest(argc, argv);
		thelog << "请求信息：" << endl << pRequest->RequestHtmlReport() << endi;
		pRespond->Init();
		pRespond->AddHeaderNoCache();

		CHtmlDoc::CHtmlTable2 table;
		RET_TYPE rettype = RET_STD;
		string xmlsource;
		//theDS;
		_asmx_exec(table, pRequest, &rettype, &xmlsource);
		//theConn.close();

		string str;
		if (RET_XMLSOURCE == rettype)
		{
			pRespond - AddHeaderContentTypeByFilename("*.xml");
			pRespond->AppendBody(xmlsource);
		}
		else
		{
			if (pRequest->GetResourceType() == "aspx")
			{
				pRespond->AddHeaderContentTypeByFilename("*.html");
				pRespond->AppendBody("<!DOCTYPE HTML>\r\n<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /></head><body>");
				pRespond->AppendBody(table.MakeHtmlTable());
				pRespond->AppendBody("</body></html>");
			}
			else
			{
				pRespond->AddHeaderContentTypeByFilename("*.xml");
				table.MakeXML("___xml_root___", str);
				pRespond->AppendBody(str);
			}
		}
		//thelog<<str<<endi;
		pRespond->AddHeaderContentLength();
		pRespond->CgiFlush();
		return true;
	}
};
