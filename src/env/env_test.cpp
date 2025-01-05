
/*
	//构造完整文件路径名，文件名为"~/appname.start.log"，修改实现以适应特别需要
	CAppUtil::MakeFileName(char const* appname, char const* px);
	//注册程序启动，格式大致为：日期时间 名称 ps信息（含PID） 工作目录 编译时间
	CAppUtil::RegistAppStart(appname);
	//pid文件读写和删除，文件名为"~/appname.pid"
	CAppUtil::WritePidFile(appname);
	CAppUtil::ReadPidFile(childappname, child);
	CAppUtil::DeletePidFile(childappname);

	完整示例见main函数
*/

#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

class CMyProcess
{
public:
	static bool isProcessLive(pid_t pid)
	{
		//waitpid返回0表明进程在运行
		if (0 != waitpid(pid, NULL, WNOHANG))
		{
			//如果进程已经停止，则kill 0会失败，由于已经做过waitpid，僵尸进程已经回收
			if (0 != kill(pid, 0))
			{
				return false;
			}
		}

		return true;
	}
	//如果已经停止返回true，如果是被信号终止则bTerm为true
	static bool WaitProcessFinish(pid_t pid, bool& bTerm, int& ret, int& termSig)
	{
		int status;
		waitpid(pid, &status, 0);//返回时一定已经结束
		bTerm = false;
		if (WIFEXITED(status))
		{
			ret = WEXITSTATUS(status);
		}
		if (WIFSIGNALED(status))
		{
			bTerm = true;
			termSig = WTERMSIG(status);
		}
		return true;
	}
};

class CEasyFile
{
private:
	string m_msg;
public:
	string const& getMsg()const { return m_msg; }
	static long GetFileSize(char const* filename)
	{
		FILE* file = fopen(filename, "rb");
		if (NULL == file)return -1;

		long file_size = -1;
		if (fseek(file, 0, SEEK_END) != 0)return -1;
		file_size = ftell(file);	// 获取此时偏移值，即文件大小
		fclose(file);
		return file_size;
	}
	bool DeleteFile(char const* filename)
	{
		if (0 != remove(filename) && IsFileExist(filename))
		{
			m_msg = "删除错误";
			return false;
		}
		return true;
	}
	bool IsFileExist(char const* filename)
	{
		if (0 != access(filename, F_OK))
		{
			m_msg = "access错误";
			return false;
		}
		return true;
	}
	bool RenameFile(char const* from, char const* to)
	{
		if (0 != rename(from, to))
		{
			m_msg = "重命名错误";
			return false;
		}
		return true;
	}
	//读取文件所有数据，不建议用于大文件
	bool ReadFile(char const* filename, vector<char>& filedata)
	{
		FILE* file;
		file = fopen(filename, "rb");
		if (NULL == file)
		{
			m_msg = "未能打开文件";
			return false;
		}
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		if (size > 1024 * 1024 * 1024)
		{
			m_msg = "文件太大";
			fclose(file);
			return false;
		}
		fseek(file, 0, SEEK_SET);//回到开始位置
		size_t tmp;
		filedata.resize(size);
		if (size != (tmp = fread(&filedata[0], 1, size, file)))
		{
			m_msg = "读取错误";
			fclose(file);
			return false;
		}
		if (0 != fclose(file))
		{
			m_msg = "关闭文件错误错误";
			return false;
		}
		return true;
	}
	//读不到足够数据则失败
	bool ReadFile(char const* filename, char* buffer, long buffersize)
	{
		FILE* file;
		file = fopen(filename, "rb");
		if (NULL == file)
		{
			m_msg = "未能打开文件";
			return false;
		}
		if (1 != fread(buffer, buffersize, 1, file))
		{
			m_msg = "读取错误";
			fclose(file);
			return false;
		}
		if (0 != fclose(file))
		{
			m_msg = "关闭文件错误错误";
			return false;
		}
		return true;
	}
	bool WriteFile(char const* filename, long filedata)
	{
		char buf[256];
		snprintf(buf, 256, "%ld", filedata);
		return WriteFile(filename, buf);
	}
	bool WriteFile(char const* filename, char const* filedata)
	{
		return WriteFile(filename, filedata, (long)strlen(filedata));
	}
	bool WriteFile(char const* filename, char const* filedata, long datasize)
	{
		return WriteFile(filename, 0, filedata, datasize, true);
	}
	bool WriteFile(char const* filename, long seek, char const* filedata, long datasize, bool trunc)
	{
		FILE* file;
		string mode = "wb";
		if (!trunc)mode = "ab";

		file = fopen(filename, mode.c_str());
		if (NULL == file)
		{
			m_msg = "未能打开文件";
			return false;
		}
		size_t size = datasize;
		fseek(file, seek, SEEK_SET);
		if (ftell(file) != seek)
		{
			char buf[256];
			snprintf(buf, 256, "定位错 %ld %ld", seek, ftell(file));
			m_msg = buf;
			return false;
		}
		if (size != fwrite(filedata, 1, size, file))
		{
			m_msg = "写入错误";
			return false;
		}
		if (0 != fclose(file))
		{
			m_msg = "关闭文件错误错误";
			return false;
		}
		return true;
	}
};

class CAppUtil
{
private:
	//构造文件名
	static string MakeFileName(char const* appname, char const* px)
	{
		char buf[1024];
		sprintf(buf, "%s/%s.%s", getenv("HOME"), appname, px);
		return buf;
	}
public:
	//注册程序启动，格式大致为：日期时间 名称 ps信息（含PID） 工作目录 编译时间
	static bool RegistAppStart(char const* appname)
	{
		//登记程序运行
		string psinfo;
		{
			char buf[1024];
			FILE* pf;
			long pid = getpid();

			tm const* t2;
			time_t t1;
			time(&t1);
			t2 = localtime(&t1);
			sprintf(buf, "%02d-%02d %02d:%02d:%02d %s ps信息 ", t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec, appname);
			psinfo += buf;
			sprintf(buf, "ps -f -p %ld | grep %ld", pid, pid);
			string cmd = buf;
			if (NULL == (pf = popen(cmd.c_str(), "r")))
			{
				cout << "popen error,can not execute : " << strerror(errno) << endl;
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
			psinfo += "\n";
		}

		ofstream f;
		string filename = MakeFileName(appname, "start.log");
		f.open(filename.c_str(), ios::out | ios::app);
		if (!f.good())
		{
			cout << "can not open activeapplog : " << filename << endl;
		}
		else
		{
			f.write(psinfo.c_str(), psinfo.size());
			if (!f.good())
			{
				cout << "can not output activeapplog : " << filename << endl;
			}
			f.close();
		}

		return true;
	}

	//删除pid文件
	static bool DeletePidFile(char const* appname)
	{
		CEasyFile file;
		return file.DeleteFile(MakeFileName(appname, "pid").c_str());
	}
	
	//写入pid文件
	static bool WritePidFile(char const* appname)
	{
		CEasyFile file;
		char buf[256];
		sprintf(buf, "%d", (int)getpid());
		return file.WriteFile(MakeFileName(appname, "pid").c_str(), buf, strlen(buf));
	}

	//读取pid文件
	static bool ReadPidFile(char const* appname,pid_t & pid)
	{
		CEasyFile file;
		vector<char > buf;
		if (file.ReadFile(MakeFileName(appname, "pid").c_str(), buf))
		{
			buf.push_back('\0');
			pid = atol(&buf[0]);
			return true;
		}
		return false;
	}
};

int main(int argc, char** argv)
{
	cout << "独立测试程序" << endl;

	char const* appname = "test";
	char const* childappname = "child";
	CAppUtil::RegistAppStart(appname);
	CAppUtil::WritePidFile(appname);

	CAppUtil::DeletePidFile(childappname);
	pid_t pid = fork();
	if (pid < 0)
	{
		char buf[256];
		sprintf(buf, "fork error : %d : %s", errno, strerror(errno));
		cout << buf << endl;
		return 1;
	}
	else if (0 == pid)
	{
		CAppUtil::WritePidFile(childappname);
		cout << "子进程 pid=" << getpid() << endl;

		sleep(10);

		cout << "子进程退出" << endl;
		exit(2);
	}
	else
	{
		while (true)
		{
			pid_t child;
			if (!CAppUtil::ReadPidFile(childappname, child))
			{
				sleep(1);
				continue;
			}

			bool bTerm = false;
			int termSig = 0;
			int last_ret = 0;
			if (!CMyProcess::WaitProcessFinish(child, bTerm, last_ret, termSig))break;
			char buf[256];
			if(bTerm)sprintf(buf, "%d 被信号终止 %d", child, termSig);
			else sprintf(buf, "%d 返回 %d", child, last_ret);
			cout << buf << endl;
			break;
		}
	}

	return 0;
}
