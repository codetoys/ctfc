
#include "ctScripttest.h"

int test_CTScript2(int argc, char** argv)
{
	string str;

	ns_my_script2::CCTScript script;
	ns_my_script2::ICTScript Script;
	ns_my_script2::ICTScript* pScript = &Script;
	char const* source = "int i=2;i+=1,i+=1;return max(i+7,5);";

	if (!pScript->AttachScript(&script))
	{
		return __LINE__;
	}

	if (!pScript->Compile(source))
	{
		thelog << pScript->Report(str) << endi;
		return __LINE__;
	}
	thelog << pScript->Report(str) << endi;

	ns_my_script2::Variable var;
	if (!pScript->Execute(var))
	{
		thelog << pScript->GetMessage() << ende;
		return __LINE__;
	}
	thelog << "脚本执行结果：" << var.GetString() << endi;

	return 0;
}
int test_CTScript2_case(int argc, char** argv)
{
	//执行所有测试用例
	ns_my_script2::CTestCase testcase;
	if (!testcase.doTestCase())
	{
		return __LINE__;
	}

	return 0;
}

//int main_fun(int argc,char ** argv)
int main(int argc, char** argv)
{
	if (!InitActiveApp("CTScripttest", 1024 * 1024 * 10, argc, argv))exit(1);

	if (0 == isatty(STDIN_FILENO))
	{
		thelog << "not tty" << endi;
	}
	else
	{
		winsize sz;
		if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&sz) < 0)
		{
			thelog << "get tty size error" << endi;
		}
		else
		{
			thelog << "控制台窗口尺寸 ws_row=" << sz.ws_row << " ws_col=" << sz.ws_col << endi;
		}
	}
	if (sizeof(long) != 8)
	{
		thelog << "非64位程序！" << ende;
		return 1;
	}

	{
		string str;
		int n = 1;
		n = htonl(n);
		if (1 == n)
		{
			thelog << "主机字节序与网络字节序相同" << endi;
		}
		else
		{
			thelog << "主机字节序与网络字节序不相同" << endi;
		}
	}

	bool loop = true;
	G_IS_DEBUG = true;

	while (loop)
	{
		int ret = 0;
		string cmd;

		if (GetCommandParam(argc, argv, "-cmd", cmd))
		{
			loop = false;
		}
		else
		{
			thelog << endl << "----------------------------------------" << endl << "命令表：（q=exit）" << endl
				<< "1 test_CTScript2" << endl
				<< "2 test_CTScript2_case" << endl
				<< "........................................" << endl
				<< "----------------------------------------" << endi;
			cmd = UIInput("请选择命令：", "2");
			if (cmd == "q")break;
		}
		long nCmd = atol(cmd.c_str());
		switch (nCmd)
		{
		case 1:
			ret = test_CTScript2(argc, argv);
			break;
		case 2:
			ret = test_CTScript2_case(argc, argv);
			break;
		default:
			thelog << "无效的命令：" << cmd << "(" << nCmd << ")" << ende;
			break;
		}
		if (0 != ret)
		{
			thelog << "命令 " << nCmd << " 返回 " << ret << ende;
		}
		else
		{
			thelog << "命令 " << nCmd << " 返回 " << ret << endi;
		}
		if (loop)UIInput("Press any key to continue ...", "y");
	}
	//thelog<<"请观察内存..."<<endi;
	//CpuBusy();
	//thelog<<"程序退出"<<endi;
	return 0;
}
//DELCARE_MAIN(main_fun)
