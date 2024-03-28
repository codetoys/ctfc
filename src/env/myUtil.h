//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../function/config.h"
#include "myLog.h"

namespace ns_my_std
{

	//存放默认配置文件的环境变量
#define MySTD_ENV "MY_STD_ENV"

	//全局互斥对象
#define GLOBAL_MUTEX_NAME "GlobalMutex"

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//共享内存池的地址，共享内存池只能有一个并且连接到固定的地址，SHM_NAME_SHMPOOL，PI_SHMPOOL
//#define SHM_ALLOCATOR_USE_OLD_POINTER //使用旧式指针，同时使用固定连接地址
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
#define ADDR_SHM_POOL 0X700000010000000
#else
#define ADDR_SHM_POOL NULL
#endif

	/////////////////////////////////////////////////////////////////////////////////////
	//共享内存名称定义和信号量名称定义

#define SHM_NAME_SHMPOOL					"ShmPool"//共享内存池

#define SHM_NAME_SHMDB_TABLE_DEF			"SHM_DB_TABLE_DEF"
#define SHM_NAME_SHMDB_COLUMN_DEF			"SHM_DB_COLUMN_DEF"
#define SHM_NAME_SHMDB_INDEX_DEF			"SHM_DB_INDEX_DEF"
#define SHM_NAME_SHMDB_STRING_POOL			"SHM_DB_STRING_POOL"
#define SHM_NAME_SHMDB_DATA					"SHM_DB_DATA"
#define SHM_NAME_SHMDB_INDEX				"SHM_DB_INDEX"
#define SHM_NAME_SHMDB_REDO_0				"SHM_DB_REDO_0"
#define SHM_NAME_SHMDB_REDO_1				"SHM_DB_REDO_1"
#define SHM_NAME_SHMDB_REDO_2				"SHM_DB_REDO_2"
#define SHM_NAME_SHMDB_SEQUENCE				"SHM_DB_SEQUENCE"
#define SEM_NAME_SHMDB						"SHM_DB"

	//用于SET共享内存的指针编号，不重复即可
#define PI_NULL_					0//禁止使用

#define PI_TEST_1					1//测试用
#define PI_TEST_2					2//测试用
#define PI_TEST_3					3//测试用
#define PI_TEST_4					4//测试用
#define PI_TEST_5					5//测试用
#define PI_TEST_6					6//测试用
#define PI_TEST_7					7//测试用
#define PI_TEST_8					8//测试用
#define PI_TEST_9					9//测试用

#define PI_SHMPOOL					20//共享内存池

#define PI_HASHTREE					30//通用哈希树的哈希入口
#define PI_HASHTREE_DATA			31//通用哈希树的数据

#define PI_SHMDB_TABLE_DEF			100//表定义块
#define PI_SHMDB_COLUMN_DEF			101//列定义块
#define PI_SHMDB_INDEX_DEF			102//索引定义块
#define PI_SHMDB_STRING_POOL		103//字符串池块
#define PI_SHMDB_STRING_POOL_2		104//字符串池块2
#define PI_SHMDB_DATA				105//数据块
#define PI_SHMDB_INDEX				106//索引块
#define PI_SHMDB_REDO_0				107//REDO块
#define PI_SHMDB_REDO_1				108//REDO块
#define PI_SHMDB_REDO_2				109//REDO块
#define PI_SHMDB_SEQUENCE			110//序列块

#define MAX_PP				800//独立定义的限制，一定不要小于上一行的值
#define T_ARRAY_VMAP_MAX_SIZE		100//共享内存分块影射表的大小

#define GET_PP_LRU(n)				(g_pActiveAppEnv->shm_private_data_s[n].pLRU)//获得LRU对象指针
#define GET_PP_SET(n)				(g_pActiveAppEnv->shm_private_data_s[n].pSET)//获得SET对象指针
#define GET_PP_VMAP(n)				(g_pActiveAppEnv->shm_private_data_s[n].pVMAP)//获得句柄映射表
#define GET_SHM_PRIVATE_DATA(n)		(g_pActiveAppEnv->shm_private_data_s[n])//获得共享内存地址映射表

	////////////////////////////////////////////////////////////////////////
	class Log;

	//应用程序启动函数，每个应用程序开始必须调用
	bool InitActiveApp(char const * appname, long max_log_size, int argc, char ** argv);
	//cgi程序启动函数，每个cgi开始必须调用
	bool InitActiveCgi(char const * appname, long max_log_size, int argc, char ** argv);
	//应用退出函数
	void ExitActiveApp(int exit_code);
	//实际的启动函数
	bool _InitActiveApp(char const * appname, long max_log_size, bool isCgi, int argc, char ** argv);

	class CActiveAppEnv;
	extern CActiveAppEnv * g_pActiveAppEnv;//全局应用程序环境指针
	//应用程序环境
	class CActiveAppEnv
	{
	private:
		bool _isDebug;
		//错误码支持，仅当函数明确指出使用此错误码支持时才有意义
		long error_code;
		stringstream error_message;
		Log _theLog;//日志对象

	public:
		typedef map<string, void *> T_SINGLETONS;

		string appname;
		//调试和跟踪，实际使用的是指针，以便可以根据需要把实际控制变量指向其他地方（比如共享内存）
		bool * pIsDebug;
		

		long signal;//接收到的信号，由sig_default设置

		T_SINGLETONS mapSingleton;

		int argc;
		char ** argv;

		CActiveAppEnv();
		void Report();

		bool is_error()
		{
			return 0 != error_code;
		}
		stringstream & set_error(long n)
		{
			error_code = n;
			error_message.str("");
			return error_message;
		}
		long get_error()const
		{
			return error_code;
		}
		string get_error_message()const
		{
			return error_message.str();
		}
		void clear_error()
		{
			error_code = 0;
			error_message.str("");
		}

		Log & GetLog()
		{
			return _theLog;
		}
		stringstream & ErrorMessage()
		{
			return error_message;
		}
	};

	//捕获异常的main函数，改变原有的main函数的名字，然后使用此宏即可获得标准的异常捕获功能
#define DELCARE_MAIN(main_fun) int main(int argc,char * argv[]){return _std_main(main_fun,argc,argv);}
	int _std_main(int main_fun(int, char *[]), int argc, char * argv[]);//捕获异常的函数

	//取当前日期 yyyymmdd
	char* get_date(char* t_date);

	bool isBlank(char c, char const * s = NULL);
	string& LTrim(string& strValue, char const * s = NULL);
	string& RTrim(string& strValue, char const * s = NULL);
	string& Trim(string& strValue, char const * s = NULL);
	string FloatString(const double&);
	string FloatString(const long&);
	string FloatString(const int&);
	string& ToUpper(string& strValue);

	class StringTokenizer
	{
	private:
		typedef vector<string> STRING_LIST;
		STRING_LIST m_list;

		static bool less_by_length_desc(string const & a, string const & b)
		{
			return a.size() > b.size();
		}
	public:
		typedef STRING_LIST::iterator iterator;
		typedef STRING_LIST::const_iterator const_iterator;
		iterator begin() { return m_list.begin(); }
		iterator end() { return m_list.end(); }
		const_iterator begin()const { return m_list.begin(); }
		const_iterator end()const { return m_list.end(); }
		STRING_LIST::size_type size() { return m_list.size(); }
		void SortByLengthDesc()
		{
			sort(begin(), end(), less_by_length_desc);
		}
		StringTokenizer& operator()(const string& str, const string& delim, bool bTrim = true, bool quote = false)
		{
			m_list.clear();
			string::size_type nPos1 = 0, nPos2 = 0, nFindBase = 0;
			if (delim.size() == 0) return *this;

			string tmp;
			while ((nPos2 = str.find(delim, nFindBase)) != string::npos)
			{
				if (quote)
				{
					bool in_quote{ false };
					char c;
					for (size_t x = nPos1; x < nPos2; ++x)
					{
						if (str[x] == '"' || str[x] == '\'')
						{
							if (!in_quote)
							{
								c = str[x];
							}
							if (str[x] == c)
							{
								in_quote = !in_quote;
							}
						}
					}
					if (in_quote)
					{
						nFindBase = nPos2 + delim.size();
						continue;
					}
				}
				tmp.assign(str, nPos1, nPos2 - nPos1);
				if (bTrim) Trim(tmp);
				m_list.push_back(tmp);
				nPos1 = nPos2 + delim.size();
				nFindBase = nPos1;
			}
			tmp.assign(str, nPos1, str.size() - nPos1);
			if (bTrim) Trim(tmp);
			m_list.push_back(tmp);
			return *this;
		}
		StringTokenizer(const string &str, const string &delim, bool bTrim = true, bool quote = false)
		{
			operator ()(str, delim, bTrim, quote);
		}
		StringTokenizer() {}
		string& operator[] (unsigned int nPos)
		{
			return m_list[nPos];
		}
		void TrimAll(char const * s)
		{
			STRING_LIST::size_type i;
			for (i = 0; i < size(); ++i)
			{
				Trim(m_list[i], s);
			}
		}
	};
	class FastStringTokenizer
	{
	public:
		FastStringTokenizer() :m_buf(NULL), m_bufsize(0) {};
		FastStringTokenizer(const string& str, string const & delim, bool bTrim = true) :m_buf(NULL), m_bufsize(0)
		{
			operator () (str, delim, bTrim);
		}
		~FastStringTokenizer()
		{
			if (NULL != m_buf)delete[] m_buf;
		}
	private:
		class CIsSpace
		{
		public:
			bool isSpace[256];
			void set(char c)
			{
				long i = (unsigned long)(unsigned char)c;
				if (i < 0)
				{
					//thelog << "字符[" << c << "](" << (long)c << ")被解释为 " << i << endi;
					return;
				}
				isSpace[i] = true;
			}
			CIsSpace()
			{
				for (long i = 0; i < 256; ++i)
				{
					isSpace[i] = false;
				}
				set(' ');
				set('\t');
				set('\f');
				set('\v');
				set('\r');
				set('\n');
			}
		};
		typedef vector<char const * > STRING_LIST;
		STRING_LIST m_list;
		char * m_buf;
		size_t m_bufsize;
	public:
		typedef STRING_LIST::iterator iterator;
		iterator	begin()
		{
			return m_list.begin();
		}

		iterator  end()
		{
			return m_list.end();
		}

		long count()
		{
			return (long)m_list.size();
		}

		string operator[](unsigned int nPos)
		{
			return m_list[nPos];
		}
		char const * at(unsigned int nPos)
		{
			return m_list[nPos];
		}

		FastStringTokenizer&  operator()(const string& str, string const & delim, bool bTrim = true)
		{
			STATIC_C CIsSpace const isSpace;//判断空白用

			m_list.clear();
			if (0 == str.size())return *this;

			if (NULL == m_buf || m_bufsize <= str.size())
			{
				if (NULL != m_buf)
				{
					delete[] m_buf;
				}
				m_buf = new char[str.size() + 1];
				m_bufsize = str.size() + 1;
			}
			strcpy(m_buf, str.c_str());
			//Loginfo<<m_buf<<" "<<delim<<std::endl;

			char * p_cur_start = NULL;//当前处理串的开始位置
			bool isInstr = false;//是否处于字符串中
			bool isFinish = false;//处理结束
			for (char * p = &m_buf[0]; !isFinish; ++p)
			{
				//Loginfo<<(NULL!=p?*p:'X')<<std::endl;
				isFinish = ('\0' == *p);
				//Loginfo<<(!isFinish?"":"isFinish")<<std::endl;
				//Loginfo<<"strcmp(p, delim.c_str())="<<strncmp(p, delim.c_str(),delim.size())<<std::endl;
				if (isFinish || (delim[0] == *p && 0 == strncmp(p, delim.c_str(), delim.size())))
				{//结束
					*p = '\0';
					if (isInstr)
					{//非空串，反向擦除空字符
						isInstr = false;
						m_list.push_back(p_cur_start);
						for (char * rp = p - 1; rp >= p_cur_start; --rp)
						{
							if (isSpace.isSpace[(unsigned char)*rp])
							{
								*rp = '\0';
							}
							else
							{
								break;
							}
						}
					}
					else
					{//空串
						if (!isFinish)m_list.push_back(p);
					}

					if (!isFinish)
					{
						p += delim.size() - 1;
					}
					continue;
				}
				if (!isSpace.isSpace[(unsigned char)*p])
				{//非空字符
					if (!isInstr)
					{//字符串开始
						p_cur_start = p;
						isInstr = true;
					}
				}
			}

			//跟原来的类核对
			//if (true)
			//{
			//	_MyStringTokenizer st(str, delim, bTrim);
			//	if (this->count() != st.count())
			//	{
			//		Loginfo << "错误：个数不同 " << count() << " " << st.count() << std::endl;
			//	}
			//	for (long i = 0; i < count(); ++i)
			//	{
			//		if (0 != strcmp(at(i), st.at(i)))
			//		{
			//			Loginfo << "错误：不一致 " << i << " [" << (*this)[i] << "][ " << st[i] << "]" << std::endl;
			//		}
			//	}
			//	static long count=0;
			//	if(0==(++count)%100000)Loginfo<<"FastStringTokenizer "<<count<<std::endl;
			//}
			return *this;
		}
	};

	//使进程繁忙以便观察进程状态
	void CpuBusy();

	//输出进程占用内存情况，包括子进程
	void ShowPS();
	//仅输出头
	string & ShowPS_head(string & ret);

#ifdef _WINDOWS
#else
	//仅输出头和本PID
	string & ShowPS_pid(pid_t pid, string & ret);
	//跟踪进程时间和内存
	class CTrance
	{
	private:
		time_t t1;
		string title;
	public:
		//开始跟踪，初始化计时器
		void StartTrance(char const * _title)
		{
			t1 = time(NULL);
			title = _title;

			string str;
			thelog << title << "开始......" << endl << ShowPS_pid(getpid(), str) << endi;
		}
		//显示自从上次StartTrance或ShowTrance
		void ShowTrance()
		{
			time_t t = time(NULL) - t1;//耗时
			string str;
			thelog << title << "耗时 " << t / 60 << " 分 " << t % 60 << " 秒" << endl << ShowPS_pid(getpid(), str) << endi;

			t1 = time(NULL);//重置计时
		}
	};
#endif

	//获取命令行特定参数
	//检查单个参数是否存在
	bool GetCommandParam(int argc, char **argv, char const * key);
	//获取双参数，如“-f a.txt”这样的，GetCommandParam("-f",str)，str返回“a.txt”
	bool GetCommandParam(int argc, char **argv, char const * key, string & value);

	//调用system，返回0为正常
	int myShellExecute(char const * cmd, char const * param);

	//获得命令输出
	int GetShellOutput(char const * cmd,string & output);

	//环境变量替换，利用shell功能
	string & ShellEnvReplace(string & str);

	//字符串的字符替换
	string & Replace(string & str, string const & old_char, string const & new_char);

	//依赖操作系统的文件类
	class CStdOSFile
	{
	private:
		FILE * m_file;//文件
		char * m_buf;//内部缓存
		long m_bufsize;//内部缓存大小
		long m_bufcount;//内部缓存存储的数据量
		long m_bufpos;//内部缓存当前位置
		string m_filename;
	public:
		CStdOSFile() :m_file(NULL), m_buf(NULL) {}
		~CStdOSFile() { if (NULL != m_buf)delete[] m_buf; }
		string const & GetFileName()const { return m_filename; }
		string & Report(string & ret)const//将此对象中所有的数据成员输出
		{
			ret = "";
			ret += "file name : [" + m_filename + "]\n";
			char buf[1024];
			sprintf(buf, "m_buf=[%p] m_bufsize=[%ld] m_bufcount=[%ld] m_bufpos=[%ld]\n", m_buf, m_bufsize, m_bufcount, m_bufpos);
			ret += buf;
			if (NULL != m_buf)
			{
				ret += "buf=[";
				ret += m_buf;
				ret += "]\n";
				ret += "pos=[";
				ret += (m_buf + m_bufpos);
				ret += "]\n";
			}
			return ret;
		}
		//为读打开
		bool OpenR(char const * filename)
		{
			m_filename = filename;
			return NULL != (m_file = fopen(m_filename.c_str(), "r"));
		}
		//为写打开，截断或创建
		bool OpenW(char const * filename)
		{
			m_filename = filename;
			return NULL != (m_file = fopen(m_filename.c_str(), "w"));
		}
		//为写打开，创建或追加
		bool OpenA(char const * filename)
		{
			m_filename = filename;
			return NULL != (m_file = fopen(m_filename.c_str(), "a"));
		}
		//为读写打开，不截断
		bool OpenRW(char const * filename)
		{
			m_filename = filename;
			return NULL != (m_file = fopen(m_filename.c_str(), "r+"));
		}
		//关闭
		bool Close()
		{
			m_filename = "";
			m_bufcount = 0;
			m_bufpos = 0;
			return 0 == fclose(m_file);
		}
		bool isEnd()
		{
			if (NULL != m_buf && m_bufpos < m_bufcount)return false;
			return 0 != feof(m_file);
		}
		bool isError() { return 0 != ferror(m_file); }
		//标准读
		size_t Read(void * p, size_t size)
		{
			return fread(p, 1, size, m_file);
		}
		//标准行读
		char * ReadLine(char * p, size_t size)
		{
			char * ret = fgets(p, (int)size, m_file);
			if (NULL != ret)
			{
				long len = (long)strlen(ret);
				if (len > 0)
				{
					if ('\n' == ret[len - 1])
					{
						ret[len - 1] = '\0';
					}
				}
			}
			return ret;
		}
		//标准写
		size_t Write(void const * p, size_t size)
		{
			return fwrite(p, 1, size, m_file);
		}
		long Tell()const { return ftell(m_file); }
		bool SeekBegin(long offset = 0) { return 0 == fseek(m_file, offset, SEEK_SET); }
		bool SeekEnd(long offset = 0) { return 0 == fseek(m_file, offset, SEEK_END); }
		bool SeekCur(long offset) { return 0 == fseek(m_file, offset, SEEK_CUR); }

		//缓存读，不能与标准读写混合使用。不能使用Seek函数
		//实际总是多申请一个字节，但不记录在缓存大小中
		bool SetBufSize(long size)
		{
			DEBUG_LOG << "设置缓存大小为 " << size << endi;
			size = size * 2;
			DEBUG_LOG << "实际设置缓存大小为 " << size << endi;
			if (NULL != m_buf)
			{
				setbuf(m_file, NULL);//关闭系统缓存
				char * p = m_buf;
				m_buf = new char[size + 1];
				if (NULL == m_buf)return false;
				memcpy(m_buf, p + m_bufpos, m_bufcount - m_bufpos);//读是从当前位置读入到文件尾的所有数据，当前位置前的文件数据不理会
				m_bufcount -= m_bufpos;
				m_bufpos = 0;
				m_bufsize = size;
				m_buf[m_bufcount] = '\0';
				delete[] p;
			}
			else
			{
				m_buf = new char[size + 1];
				if (NULL == m_buf)return false;
				m_bufsize = size;
				m_bufcount = 0;
				m_bufpos = 0;
				m_buf[m_bufcount] = '\0';
			}
			return true;
		}
		//读取数据并可以设置最后一个字符为NULL，返回值为指向内部缓存的指针，返回值在下一次调用时失效
		//使用此函数则不应该再使用Seek
		char const * Read(long size, bool bSetLastNull)
		{
			char * ret = NULL;
			if (NULL == m_buf)
			{
				thelog << "仅可用于缓存模式" << ende;
				return NULL;
			}
			if (m_bufcount - m_bufpos < size)
			{
				//比缓存大，必须先扩大缓存
				if (size > m_bufsize)
				{
					if (!SetBufSize(size))
					{
						thelog << "设置缓存出错" << ende;
						return NULL;
					}
				}
				//移动剩余部分
				memmove(m_buf, m_buf + m_bufpos, m_bufcount - m_bufpos);
				m_bufcount = m_bufcount - m_bufpos;
				m_bufpos = 0;
				//读取填充缓存
				long readcount = (long)fread(m_buf + m_bufcount, 1, m_bufsize - m_bufcount, m_file);
				//if(readcount<0)
				//{
				//	thelog<<"读文件错误"<<ende;
				//	return NULL;
				//}
				//else 
				if (0 == readcount)
				{
					return NULL;
				}
				else if (readcount < size - (m_bufcount - m_bufpos))
				{
					thelog << "读文件错误 未读到 " << size - (m_bufcount - m_bufpos) << " 个字节" << ende;
					m_bufcount += readcount;
					return NULL;
				}
				else
				{
				}
				m_bufcount += readcount;
			}
			ret = m_buf + m_bufpos;
			if (bSetLastNull)
			{
				ret[size - 1] = '\0';
			}
			m_bufpos += size;
			m_buf[m_bufcount] = '\0';

			return ret;
		}
	};

	void ShowArg(int argc, char ** argv);
	void ShowEnv();

	//信号的描述
	char const * sigstr(long sig);
	//信号处理器，收到的信号放入g_pActiveAppEnv->signal
	extern "C" void sig_default(int sig);
	//检测收到的信号，如果是则清楚信号标志g_pActiveAppEnv->signal
	bool check_signal(int sig);

	//获取用户控制台输入的函数，可用于主程序与用户交互
	string UIInput(char const * prompt, char const * defaultvalue, char const* mutli_line_end = NULL);
	string UIInput(char const * prompt, long defaultvalue);

	//分解命令行，返回第一个命令和后面的参数
	bool GetTopCmd(string const & cmdline, string & cmd, string & param);

	//配置文件处理类
	class CIniFile
	{
	public:
		typedef vector<pair<string, string> > T_SECTION;
		typedef map<string, T_SECTION > T_DATA_S;
	private:
		T_DATA_S m_datas;//section-key-value

		string::size_type findString(string const & str, char const * tofind)const
		{
			string::size_type i;
			bool isInText = false;//是否处于引号中
			i = 0;
			for (; i < str.size(); ++i)
			{
				if ('\"' == str[i])
				{
					isInText = !isInText;
				}
				else
				{
					if (!isInText)
					{
						if (i == str.find(tofind, i))
						{
							break;
						}
					}
				}
			}
			if (i < str.size())return i;
			else return str.npos;
		}
		//去掉用来包裹字符串的引号
		string eraseQuotes(string const & str)const
		{
			if (str.size() >= 2 && '\"' == str[0] && '\"' == str[str.size() - 1])
			{
				return str.substr(1, str.size() - 2);
			}
			return str;
		}
		string & eraseComment(string & str)const
		{
			string::size_type i = findString(str, "//");
			if (i != str.npos)str.erase(i);
			return str;
		}
	public:
		static bool getDefaultIni(string & ini)
		{
			char const * p;
			p = getenv(MySTD_ENV);
			if (NULL == p)
			{
				thelog << "未设置环境变量 " << MySTD_ENV << ende;
				return false;
			}
			ini = p;
			return true;
		}
		bool LoadDefaultIni(char const * ini_name)
		{
			string env;
		
			if(!getDefaultIni(env))
			{
				return false;
			}

			thelog << MySTD_ENV << " = " << env << endi;
			if (env.size() > 0 && env[env.size() - 1] != '/')env += '/';
			env += ini_name;
			thelog <<"默认配置文件 = " << env << endi;
			return LoadIni(env.c_str());
		}
		static bool SaveDefaultIni(T_DATA_S const * datas)
		{
			string env;
		
			if(!getDefaultIni(env))
			{
				return false;
			}

			string filename=env;
			filename+=".save.ini";
			return SaveIni(datas, filename.c_str());
		}
		static bool SaveIni(T_DATA_S const * datas, char const * filename)
		{
			string str;
			T_DATA_S::const_iterator it;
			for (it = datas->begin(); it != datas->end(); ++it)
			{
				str += "[" + it->first + "]\n";
				T_DATA_S::value_type::second_type::const_iterator it2;
				for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
				{
					str += it2->first + "=" + it2->second + "\n";
				}
			}
			CEasyFile file;
			if (!file.WriteFile(filename, str.c_str()))
			{
				thelog << "写文件出错 " << filename << ende;
				return false;
			}
			thelog << "写文件成功 " << filename << endi;
			return true;
		}
		bool LoadIni(char const * filename)
		{
			m_datas.clear();

			ifstream f;
			char buf[1024];
			string str;

			f.open(filename);
			if (!f.good())
			{
				thelog << "打开文件失败 " << filename << ende;
				return false;
			}

			string section;
			string key;
			string value;
			while (f.good())
			{
				f.getline(buf, 1024);
				buf[1024 - 1] = '\0';
				str = buf;
				eraseComment(str);//删除注释
				Trim(str);
				if (str.size() == 0)
				{
					continue;
				}
				if (str[0] == '[')
				{
					section = str.substr(1, str.size() - 2);
				}
				else
				{
					string::size_type n = findString(str, "=");
					if (n != str.npos)
					{
						key = str.substr(0, n);
						Trim(key);
						value = str.substr(n + 1);
						Trim(value);
						value = eraseQuotes(value);
						thelog << "[" << section << "] " << key << " = " << value << endi;
						m_datas[section].push_back(pair<string, string>(key, value));
					}
				}
			}
			f.close();

			return true;
		}
		T_DATA_S const * GetAllData()const { return &m_datas; }
		string GetIniItem(string const & _section, string const & _key)const
		{
			string value;
			if (GetIniItem(_section, _key, value))return value;
			else return "";
		}
		bool GetIniItem(string const & _section, string const & _key, string & value)const
		{
			map<string, vector<pair<string, string> > >::const_iterator it_section = m_datas.find(_section);
			if (it_section != m_datas.end())
			{
				vector<pair<string, string> >::const_iterator it;
				for (it = it_section->second.begin(); it != it_section->second.end(); ++it)
				{
					if (it->first == _key)
					{
						value = it->second;
						return true;
					}
				}
			}
			value = "";
			return false;
		}
		T_SECTION const * GetIniSection(string const & _section)const
		{
			map<string, vector<pair<string, string> > >::const_iterator it_section = m_datas.find(_section);
			if (it_section != m_datas.end())
			{
				return &it_section->second;
			}
			return NULL;
		}
		static bool makeSubValues(string const & value, vector<string > & values)
		{
			values.clear();

			string tmp;
			bool isInText = false;
			string::size_type i;
			char c;
			for (i = 0; i < value.size(); ++i)
			{
				c = value[i];
				if ('\"' == c)
				{
					isInText = !isInText;
				}
				else if (isInText)
				{
					tmp += c;
				}
				else if (',' == c)
				{
					thelog << values.size() << " : " << Trim(tmp) << endi;
					values.push_back(Trim(tmp));
					tmp = "";
				}
				else
				{
					tmp += c;
				}
			}
			if (isInText)
			{
				thelog << "双引号不匹配" << ende;
				return false;
			}
			thelog << values.size() << " : " << Trim(tmp) << endi;
			values.push_back(Trim(tmp));//没有逗号的或者最后一个逗号之后的
			return true;
		}
	};

	class CCrypt
	{
	private:
		static string __crypt(char const * _salt, char const * text, bool isUn)
		{
			string str;
			char buf[16];
			char c;
			long i, j;
			//thelog<<"salt "<<_salt<<endi;
			for (i = 0, j = 0; '\0' != text[i] && '\0' != _salt[j]; ++i)
			{
				if (isUn)
				{
					char buf[16];
					int c;
					if ('\0' != text[i] && '\0' != text[i + 1])
					{
						buf[0] = text[i];
						buf[1] = text[i + 1];
						buf[2] = '\0';
						sscanf(buf, "%X", &c);
					}
					else
					{
						thelog << "数据错误" << ende;
						return "";
					}
					//thelog<<"还原的 "<<c<<endi;
					c ^= _salt[j];
					str += c;
					++i;
				}
				else
				{
					c = text[i];
					c ^= _salt[j];
					//thelog<<"加密的 "<<(int)c<<endi;
					sprintf(buf, "%02X", c);
					str += buf;
				}
				++j;
				if ('\0' == _salt[j])j = 0;
			}
			return str;
		}
		static string _crypt(char const * _salt, char const * text, bool isUn)
		{
			string salt = __crypt(_salt, "mystdmystdmystd", false);
			return __crypt(salt.c_str(), text, isUn);
		}
	public:
		static string crypt(char const * salt, char const * text)
		{
			return _crypt(salt, text, false);
		}
		static string uncrypt(char const * salt, char const * crypted)
		{
			return _crypt(salt, crypted, true);
		}

	};
#ifdef _WINDOWS
#else
	//控制台输入密码，不回显
	string inputPassword(bool repeat = false);
#endif

	string to_db_style(string const & struct_style);

	//按秒睡眠
	void SleepSeconds(long seconds);
	//按微秒睡眠
	void SleepUSeconds(unsigned long useconds);

#ifdef _WINDOWS
#else
	//启动精灵进程，注意这个函数在两个驻留进程中分别执行，否则客户端不能看到启动日志
	bool start_demon();
#endif

	//所有信号和异常捕获
	int __all_sig_catch(int argc,char ** argv, int fun(int, char **));
}
