//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "myUtil.h"
#ifdef _WINDOWS
#include <signal.h>
#else
#include <sys/wait.h>
#include <termios.h>
#endif
#include <errno.h>
#include<sys/stat.h>

namespace ns_my_std
{
	CActiveAppEnv * g_pActiveAppEnv = NULL;

	stringstream & G_ERROR_MESSAGE()
	{
		return g_pActiveAppEnv->ErrorMessage(); //返回stringstream的引用，用于输入或获取错误信息
	}

	int _std_main(int main_fun(int,char * []),int argc,char * argv[])
	{
		try
		{
			return main_fun(argc,argv);
		}
		catch(int e)
		{
			thelog<<"捕获到未处理的异常："<<e<<ende;
			return 1;
		}
		catch(long e)
		{
			thelog<<"捕获到未处理的异常："<<e<<ende;
			return 1;
		}
		catch(char const * e)
		{
			thelog<<"捕获到未处理的异常："<<e<<ende;
			return 1;
		}
		catch(string const & e)
		{
			thelog<<"捕获到未处理的异常："<<e<<ende;
			return 1;
		}
		catch(...)
		{
			thelog<<"捕获到未处理的异常"<<ende;
			return 1;
		}
	}

	char* get_date( char* t_date )
	{
		time_t tmCurr = time(NULL);

		tm* p = localtime(&tmCurr);
		sprintf(t_date, "%04d%02d%02d", 
			p->tm_year+1900, p->tm_mon+1, p->tm_mday  );
		return t_date;

	}

	bool isBlank(char c,char const * s)
	{
		STATIC_C char const blank[]=" \t\r\n";
		char const * p=(NULL!=s?s:blank);
		while(*p != '\0')
		{
			if(*p==c)return true;
			++p;
		}
		return false;
	}
	string& LTrim(string& strValue,char const * s)
	{
		while (strValue.begin() != strValue.end() && isBlank(*strValue.begin(),s))
		{
			strValue.erase(strValue.begin());
		}
		return strValue;
	}

	string& RTrim(string& strValue,char const * s)
	{
		while (strValue.begin() != strValue.end() && isBlank(*(strValue.end() - 1),s))
		{
			strValue.erase(strValue.end() - 1);
		}
		return strValue;
	}

	string& Trim(string& strValue,char const * s)
	{
		return LTrim(RTrim(strValue,s),s);
	}

	string FloatString(const long& num)
	{
		char tmp[20];
		sprintf(tmp, "%ld", num);
		return string(tmp);
	}

	string FloatString(const int& num)
	{
		char tmp[20];
		sprintf(tmp, "%d", num);
		return string(tmp);
	}

	string FloatString(const double& num)
	{
		char tmp[20];
		sprintf(tmp, "%.0f", num);
		return string(tmp);
	}


	string& ToUpper(string& strValue)
	{
		for (string::size_type i = 0; i < strValue.size(); i++)
		{
			strValue[i] = toupper(strValue[i]);
		}
		return strValue;
	}

	//应用程序启动函数，每个应用程序开始必须调用，注意多数公共机制尚不能工作
	bool InitActiveApp(char const * appname, long max_log_size, int argc, char ** argv) { return _InitActiveApp(appname, max_log_size, false, argc, argv); }
	//cgi程序启动函数，每个cgi开始必须调用
	bool InitActiveCgi(char const * appname, long max_log_size, int argc, char ** argv) { return _InitActiveApp(appname, max_log_size, true, argc, argv); }
	//初始化应用环境，注意多数公共机制尚不能工作
	bool _InitActiveApp(char const* appname, long max_log_size, bool isCgi, int argc, char** argv)
	{
		if (NULL != g_pActiveAppEnv)
		{
			cout << "g_pActiveAppEnv is not NULL" << endl;
			return true;
		}
		g_pActiveAppEnv = new CActiveAppEnv;
		if (NULL == g_pActiveAppEnv)
		{
			cout << "can not create ActiveAppEnv,no memony" << endl;
			return false;
		}
		if (isCgi)theLog.setOutput(false);//禁用输出到控制台,必须在任何输出之前设置

		g_pActiveAppEnv->appname = appname;
		g_pActiveAppEnv->argc=argc;
		g_pActiveAppEnv->argv=argv;

		//登记程序运行，已停用
		if(false)if (!isCgi)
		{
			char buf[1024];
			FILE * pf;
			long pid = getpid();
			string psinfo;

			tm const * t2;
			time_t t1;
			time(&t1);
			t2 = localtime(&t1);
			sprintf(buf, "%02d-%02d %02d:%02d:%02d %s ps信息 ", t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec, appname);
			psinfo += buf;
			sprintf(buf, "ps -f -p %ld | grep %ld", pid, pid);
			string cmd = buf;
			if (NULL == (pf = popen(cmd.c_str(), "r")))
			{
				thelog << "popen error,can not execute : " << strerror(errno) << ende;
				return false;
			}
			while (NULL != fgets(buf, 1024, pf))
			{
				psinfo += buf;
				if (psinfo.size() > 0 && '\n' == psinfo[psinfo.size() - 1])psinfo.erase(psinfo.size() - 1, 1);
			}
			pclose(pf);
			psinfo += " 工作目录 ";
			psinfo += getcwd(buf, 1024);
			psinfo += " 编译时间 ";
			psinfo += __DATE__;
			psinfo += " ";
			psinfo += __TIME__;
			//if (!isCgi)cout << "进程信息：" << endl << psinfo << endl;
			psinfo += "\n";

			ofstream f;
			sprintf(buf, "%s/.active_app_log.txt", getenv("HOME"));
			f.open(buf, ios::out | ios::app);
			if (!f.good())
			{
				cout << "can not open activeapplog : " << buf << endl;
			}
			else
			{
				f.write(psinfo.c_str(), psinfo.size());
				if (!f.good())
				{
					cout << "can not output activeapplog : " << buf << endl;
				}
				f.close();
			}
		}
		
		theLog.ActiveOpen(appname, max_log_size);
		
		if (GetCommandParam(argc, argv, "--debug"))
		{
			G_IS_DEBUG = true;
			thelog << "全局调试开关已通过命令行参数打开" << endi;
		}
		else G_IS_DEBUG = false;
		
		return true;
	}
	void ExitActiveApp(int exit_code) 
	{
		exit(exit_code); 
	}

	////////////////////////////
	//CActiveAppEnv
	CActiveAppEnv::CActiveAppEnv()
	{
		_isDebug = false;
		pIsDebug = &_isDebug;
		error_code=0;
		signal = -1;

		mapSingleton.clear();

	}
	void CActiveAppEnv::Report()
	{
		thelog<<endl;
		thelog<<"CActiveAppEnv 状态报告:"<<endl;
		thelog<<"isDebug = "<<(pIsDebug?"true":"false")<<endl;
		for(T_SINGLETONS::iterator it=mapSingleton.begin();it!=mapSingleton.end();++it)
		{
			thelog << "classname = " << it->first << " ptr = " << it->second << endl;
		}
		thelog<<"CActiveAppEnv 状态报告结束"<<endi;
	}

	void CpuBusy()
	{
		time_t t=time(NULL);
		while(time(NULL)-t<30);
	}

	void ShowPS()
	{
		char buf[1024];
		FILE * pf;

		sprintf(buf, "ps aux | head -1 ; ps aux | grep %ld | sort -rn +4 | head -1", (long)getpid());
		string cmd = buf;
		if (NULL == (pf = popen(cmd.c_str(), "r")))
		{
			thelog << "popen失败，无法执行 " << strerror(errno) << ende;
			return;
		}
		thelog << endl;
		while (NULL != fgets(buf, 1024, pf))
		{
			theLog << buf;
		}
		theLog << endi;
		pclose(pf);
		return;
	}

#ifdef _IBMOS
#define ShowPS_command "ps -p %d -o pid,pcpu,pmem,vsz,rssize,stat,start,time"
#else
#define ShowPS_command "ps -p %d -o pid,pcpu,pmem,vsz,rssize,stat,start,time"
#endif
	string & ShowPS_head(string & ret)
	{
		ret = "";
		char buf[1024];
		FILE * pf;

		sprintf(buf, ShowPS_command " | head -1", 1);
		string cmd = buf;
		if (NULL == (pf = popen(cmd.c_str(), "r")))
		{
			thelog << "popen失败，无法执行 " << strerror(errno) << ende;
			return ret;
		}
		while (NULL != fgets(buf, 1024, pf))
		{
			string tmp = buf;
			ret += Trim(tmp);
		}
		pclose(pf);
		return ret;
	}
#ifdef _WINDOWS
#else
	string & ShowPS_pid(pid_t pid, string & ret)
	{
		ret = "";
		char buf[1024];
		FILE * pf;

		sprintf(buf, ShowPS_command " | grep %d", pid, pid);
		string cmd = buf;
		if (NULL == (pf = popen(cmd.c_str(), "r")))
		{
			thelog << "popen失败，无法执行 " << strerror(errno) << ende;
			return ret;
		}
		while (NULL != fgets(buf, 1024, pf))
		{
			ret += buf;
		}
		pclose(pf);
		return ret;
	}
#endif
	bool GetCommandParam(int argc, char **argv,char const * key)
	{
		int i;
		for(i=1;i<argc;++i)
		{
			if(0==strcmp(key,argv[i]))return true;
		}
		return false;
	}

	bool GetCommandParam(int argc, char **argv,char const * key,string & value)
	{
		int i;
		for(i=1;i<argc;++i)
		{
			if(0==strcmp(key,argv[i]))
			{
				if(i+1<argc && argv[i+1][0]!='-')
				{
					value=argv[i+1];
				}
				else
				{
					value = "";
				}
				return true;
			}
		}
		return false;
	}

#ifdef _WINDOWS
#else
	int myShellExecute(char const * cmd, char const * param)
	{
		string shellstr=cmd;
		shellstr+=" ";
		shellstr+=param;

		int ret=1;

		thelog<<"执行命令："<<shellstr<<endi;
		int status=system(shellstr.c_str());
		if(WIFEXITED(status))
		{
			ret=(0xFF00&status)/256;//WEXITSTATUS宏无法识别
			if(0!=ret)theLog<<"执行 "<<shellstr<<" 出错,返回值 "<<ret<<" 状态码 "<<status<<ende;
			else thelog<<"执行成功"<<endi;
		}
		else
		{
			ret= status;
			thelog<<"执行 "<<shellstr<<" 异常,状态码 "<<status<<ende;
		}

		return ret;
	}
#endif
	int GetShellOutput(char const * cmd, string & output)
	{
		char buf[1024];
		FILE * pf;

		output = "";
		//DEBUG_LOG << cmd << endi;
		if (NULL == (pf = popen(cmd, "r")))
		{
			thelog << "popen失败，无法执行环境变量替换 " << strerror(errno) << ende;
			return errno;
		}
		while (NULL != fgets(buf, 1024, pf))
		{
			output += buf;
		}
		//DEBUG_LOG << output << endi;
		pclose(pf);
		return 0;
	}
	string & ShellEnvReplace(string & str)
	{
		string cmd = "echo " + str;
		bool hasNewLine = (str.size() > 0 && '\n' == str[str.size() - 1]);
		GetShellOutput(cmd.c_str(), str);
		if (!hasNewLine)str.erase(str.size() - 1);
		return str;
	}
	string & Replace(string & str, string const & old_char, string const & new_char)
	{
		string::size_type pos;
		while (str.npos != (pos = str.find(old_char)))str.replace(pos, old_char.size(), new_char);
		return str;
	}

	void ShowArg(int argc,char ** argv)
	{
		int i;
		thelog<<"命令行:"<<endl;
		for(i=0;i<argc;++i)
		{
			theLog<<argv[i]<<" ";
		}
		theLog<<endi;
	}
	void ShowEnv()
	{
		char ** penv=environ;
		string str;
		thelog<<"环境变量:"<<endl;
		while(*penv)
		{
			str=*penv;
			theLog<<str<<endl;
			++penv;
		}
		theLog<<endi;
	}

	//信号的描述
	char const * sigstr(long sig)
	{
		switch(sig)
		{
		case SIGABRT    :    return "SIGABRT    进程调用abort函数，进程非正常退出";
#ifdef _WINDOWS
#else
		case SIGALRM    :    return "SIGALRM    用alarm函数设置的timer超时或setitimer函数设置的interval timer超时";
		case SIGBUS     :    return "SIGBUS     某种特定的硬件异常，通常由内存访问引起";
		case SIGCHLD    :    return "SIGCHLD    子进程Terminate或Stop";
		case SIGCONT    :    return "SIGCONT    从stop中恢复运行";
		case SIGHUP     :    return "SIGHUP     终端断开";
		case SIGIO      :    return "SIGIO      异步IO事件";
		case SIGKILL    :    return "SIGKILL    强制中止";
		case SIGPIPE    :    return "SIGPIPE    在reader中止之后写Pipe的时候发送";
		case SIGPROF    :    return "SIGPROF    Setitimer指定的Profiling Interval Timer所产生";
		case SIGPWR     :    return "SIGPWR     和系统相关。和UPS相关。";
		case SIGQUIT    :    return "SIGQUIT    输入Quit Key（CTRL+\\）";
		case SIGSTOP    :    return "SIGSTOP    中止进程";
		case SIGSYS     :    return "SIGSYS     非法系统调用";
		case SIGTRAP    :    return "SIGTRAP    实现相关的硬件异常。一般是调试异常";
		case SIGTSTP    :    return "SIGTSTP    Suspend Key，一般是Ctrl+Z";
		case SIGTTIN    :    return "SIGTTIN    当Background Group的进程尝试读取Terminal的时候发送";
		case SIGTTOU    :    return "SIGTTOU    当Background Group的进程尝试写Terminal的时候发送";
		case SIGURG     :    return "SIGURG     当out-of-band data接收的时候可能发送";
		case SIGUSR1    :    return "SIGUSR1    用户自定义signal 1";
		case SIGUSR2    :    return "SIGUSR2    用户自定义signal 2";
		case SIGVTALRM  :    return "SIGVTALRM  setitimer函数设置的Virtual Interval Timer超时的时候";
		case SIGWINCH   :    return "SIGWINCH   当Terminal的窗口大小改变的时候，发送给Foreground Group的所有进程";
		case SIGXCPU    :    return "SIGXCPU    当CPU时间限制超时的时候";
		case SIGXFSZ    :    return "SIGXFSZ    进程超过文件大小限制";
#endif
#ifndef _LINUXOS
		//case SIGEMT     :    return "SIGEMT     和实现相关的硬件异常";这个linux和windows都不支持
#endif
		case SIGFPE     :    return "SIGFPE     数学相关的异常，如被0除，浮点溢出，等等";
		case SIGILL     :    return "SIGILL     非法指令异常";
			//case SIGINFO    :    return "SIGINFO    BSD signal。由Status Key产生，通常是CTRL+T。发送给所有Foreground Group的进程     ";
		case SIGINT     :    return "SIGINT     由Interrupt Key产生，通常是CTRL+C或者DELETE";
			//case SIGIOT     :    return "SIGIOT     实现相关的硬件异常，一般对应SIGABRT                                              ";
			//case SIGPOLL    :    return "SIGPOLL    当某个事件发送给Pollable Device的时候发送                                        ";
		case SIGSEGV    :    return "SIGSEGV    非法内存访问";
		case SIGTERM    :    return "SIGTERM    请求中止进程，kill命令缺省发送";
		default:			 return "未知的信号";
		}
	}

	extern "C" void sig_default(int sig)
	{
		g_pActiveAppEnv->signal = sig;
		signal(sig, sig_default);
		cout << "pid=" << getpid() << " " << sigstr(sig) << endl;
	}
	bool check_signal(int sig)
	{
		if(sig==g_pActiveAppEnv->signal)
		{
			g_pActiveAppEnv->signal=-1;
			return true;
		}
		return false;
	}
	string UIInput(char const* prompt, char const* defaultvalue, char const* mutli_line_end)
	{
		char line[10240];
		string cmd;
		cout << prompt << "(q=exit " << (strlen(defaultvalue) != 0 ? "default=" : "") << defaultvalue << ")：" << endl;
		while (true)
		{
			cin.getline(line, 10240);
			if (0 == strcmp("q", line))exit(0);
			if (NULL == mutli_line_end)
			{
				cmd += line;
				break;
			}
			else
			{
				if (0 == strcmp(line, mutli_line_end))break;
				else
				{
					if (cmd.size() != 0)cmd += "\n";
					cmd += line;
				}
			}
		}
		if (0 == cmd.size())cmd = defaultvalue;
		theLog << " 用户输入的是:" << cmd << endi;
		return cmd;
	}
	string UIInput(char const * prompt,long defaultvalue)
	{
		char buf[256];
		sprintf(buf,"%ld",defaultvalue);
		return UIInput(prompt,buf);
	}
	bool GetTopCmd(string const & cmdline, string & cmd, string & param)
	{
		string::size_type pos = cmdline.find_first_of(' ');
		if (cmdline.npos != pos)
		{
			cmd = cmdline.substr(0, pos);
			param = cmdline.substr(pos + 1);
		}
		else
		{
			cmd = cmdline;
			param = "";
		}
		Trim(cmd);
		Trim(param);
		return true;
	}

#ifdef _WINDOWS
#else
	bool setEcho(int fd,bool option)
	{
		int err;
		struct termios term;

		if(tcgetattr(fd,&term)==-1)
		{
			thelog<<"获得终端属性失败"<<ende;
			return false;
		}

		if(option)
		{
			term.c_lflag|=ECHO;
		}
		else
		{
			term.c_lflag &=~ECHO;
		}
		err=tcsetattr(fd,TCSAFLUSH,&term);
		if(err==-1 && err==EINTR)
		{
			thelog<<"设置终端属性失败"<<ende;
			return false;
		}

		return true;
	}
	string inputPassword(bool repeat)
	{
		string ret;
		char buf[256];
	
		//关回显
		setEcho(STDIN_FILENO,false);
		
		while(true)
		{
			cout<<"Please input password >"<<endl;
			cin.getline(buf,256);
			buf[255]='\0';
			ret=buf;
			if(repeat)
			{
				cout<<"Please repeat password >"<<endl;
				cin.getline(buf,256);
				buf[255]='\0';
				if(ret==buf)break;
				else cout<<"two inputs are diffrent"<<endl;
			}
			else break;
		}
		
		//开回显
		setEcho(STDIN_FILENO,true);
		return ret;
	}
#endif
	string to_db_style(string const & struct_style)
	{
		string db_style;

		for(long i=0;i<(long)struct_style.size();++i)
		{
			if(0!=i)
			{
				if(struct_style[i]>='A' && struct_style[i]<='Z')
				{
					if(struct_style[i-1]>='A' && struct_style[i-1]<='Z')
					{
					}
					else
					{
						db_style+='_';
					}
				}
			}
			db_style+=struct_style[i];
		}

		return ToUpper(db_style);
	}

	void SleepSeconds(long seconds)
	{
#ifdef _WINDOWS
		Sleep(seconds * 1000);
#else
		sleep(seconds);
#endif
	}
	void SleepUSeconds(unsigned long useconds)
	{
#ifdef _WINDOWS
		Sleep(useconds / 1000);
#else
		usleep(useconds);
#endif
	}

#ifdef _WINDOWS
#else
	//启动精灵进程，注意这个函数在两个驻留进程中分别执行，否则客户端不能看到启动日志
	bool start_demon()
	{
		setsid();
		//chdir("/");
		umask(0);
		signal(SIGCONT, sig_default);
		signal(SIGTERM, sig_default);
		signal(SIGALRM, sig_default);
		signal(SIGHUP, sig_default);

		int fd = open("/dev/null", O_RDWR);

		if (fd < 0)
		{
			LOG << "无法打开NULL设备" << ENDE;
			return false;
		}

		if (dup2(fd, STDIN_FILENO) < 0 ||
			dup2(fd, STDOUT_FILENO) < 0 ||
			dup2(fd, STDERR_FILENO) < 0)
		{
			close(fd);
			LOG << "重定向到NULL设备出错" << ENDE;
			return false;
		}
		return true;
	}
#endif
	int __all_sig_catch(int argc, char ** argv, int fun(int, char **))
	{
		signal(SIGABRT, sig_default);
#ifdef _WINDOWS
#else
		signal(SIGALRM, sig_default);
		signal(SIGBUS, sig_default);
		signal(SIGCHLD, sig_default);
		signal(SIGCONT, sig_default);
		signal(SIGHUP, sig_default);
		signal(SIGIO, sig_default);
		signal(SIGIOT, sig_default);
		signal(SIGKILL, sig_default);
		signal(SIGPIPE, sig_default);
		signal(SIGPOLL, sig_default);
		signal(SIGPROF, sig_default);
		signal(SIGPWR, sig_default);
		signal(SIGQUIT, sig_default);
		signal(SIGSTOP, sig_default);
		signal(SIGSYS, sig_default);
		signal(SIGTRAP, sig_default);
		signal(SIGTSTP, sig_default);
		signal(SIGTTIN, sig_default);
		signal(SIGTTOU, sig_default);
		signal(SIGURG, sig_default);
		signal(SIGUSR1, sig_default);
		signal(SIGUSR2, sig_default);
		signal(SIGVTALRM, sig_default);
		signal(SIGWINCH, sig_default);
		signal(SIGXCPU, sig_default);
		signal(SIGXFSZ, sig_default);
#endif
		signal(SIGFPE, sig_default);
		signal(SIGILL, sig_default);
		//signal(SIGINT, sig_default);//ctrl-c
		signal(SIGSEGV, sig_default);
		signal(SIGTERM, sig_default);

		int ret;
		try
		{
			ret = fun(argc, argv);
		}
		catch (...)
		{
			thelog << "未处理的异常发生" << ende;
			return __LINE__;
		}
		return ret;
	}
}
