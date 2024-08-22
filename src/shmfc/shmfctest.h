//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmLRU.h"
#include "myMultiProcessServerGroup.h"
#include "myMultiProcess.h"
#include <sstream>
#include "shmSet.h"
#include "../function/mysocket.h"
#include "../function/mymutex.h"
#include <atomic>
#include <thread>

/////////////////////////////////////////////////////////////////////////////////////
//多进程服务器测试
struct CDemoTask
{
	enum { BUF_SIZE = 4096 };
	char m_buf[BUF_SIZE];
	long n;
	long n2;
	long n3;
	long n4;

	bool operator < (CDemoData const& tmp)const { return n < tmp.n; }
	string& toString(string& str)const
	{
		char buf[2048];
		sprintf(buf, "%d %ld %ld %ld %ld ", m_buf[0], n, n2, n3, n4);
		return str = buf;
	}
};

template<typename T_JOBINFO, typename T_TASKINFO>
class CDemoMPS : public IDistributeTask<T_JOBINFO, T_TASKINFO>, public IProcessTask<T_JOBINFO, T_TASKINFO>, public IRollupTask<T_JOBINFO, T_TASKINFO>
{
private:
	CStdOSFile infile;
	CStdOSFile outfile;
	long n_open;
	long n_task;
public:
	CDemoMPS() :n_open(0) {}
	//分发接口
	//打开job，若成功打开ret=true，否则没有job可做
	//virtual bool OpenJob(T_JOBINFO * pJobInfo, bool * ret)
	//{
	//	n_task = 0;
	//	++n_open;
	//	pJobInfo->n = n_open * 10000000;

	//	if (!infile.OpenR("myMultiProcessServer.h"))return false;

	//	*ret=(n_open<=2);//true;
	//	//*ret = true;
	//	return true;
	//}
	//分发一个task，放入pTaskInfo，如果发生数据性错误ret=false，仍继续处理
	virtual bool DistributeTask(T_JOBINFO* pJobInfo, T_TASKINFO* pTaskInfo, bool* ret)
	{
		++n_task;
		bool tmpret = true;
		//long tmp = pTaskInfo->n;
		//if(11!=tmp%100)
		//{
		//	thelog<<"错误："<<tmp<<" "<<pTaskInfo->n<<ende;
		//	tmpret=false;
		//}
		//if(pTaskInfo->n2 != pTaskInfo->n3 || pTaskInfo->n3 != pTaskInfo->n4)
		//{
		//	thelog<<"错误："<<pTaskInfo->n<<" "<<pTaskInfo->n2<<" "<<pTaskInfo->n3<<" "<<pTaskInfo->n4<<ende;
		//	tmpret=false;
		//}
		STATIC_G long n = 0;
		pTaskInfo->n = (++n) * 100;
		++pTaskInfo->n2;

		pTaskInfo->m_buf[1] = 'A';
		pTaskInfo->m_buf[1] = '\0';

		if (n_task >= 10)tmpret = false;
		*ret = tmpret;
		thelog << "DistributeTask " << n_task << " " << *ret << endi;
		return true;
	}
	//是否job结束
	//virtual bool isJobEnd(T_JOBINFO * pJobInfo, bool * ret)
	//{
	//	STATIC_G long i = 1;
	//	if (n_task >= i)
	//	{
	//		if (0 == i)thelog << "空JOB" << endi;
	//		*ret = true;
	//		i = rand() % 100;
	//		if (i <= 0)i = 1;
	//	}
	//	else
	//	{
	//		*ret = false;
	//	}
	//	return true;
	//}
	//结束job
	//virtual bool CloseJob_Distribute(T_JOBINFO * pJobInfo)
	//{
	//	pJobInfo->n += 1;

	//	infile.Close();
	//	return true;
	//}
	//获得下一个TASK的hash，当前没有任务可返回负值
	virtual long IDistributeTask_GetNextTaskHash(bool* hasTask)
	{
		*hasTask = (0 == n_task % 2);
		long hash = n_task % 5;
		SleepSeconds(1);
		thelog << "IDistributeTask_GetNextTaskHash " << n_task << " " << *hasTask << " " << hash << endi;
		return hash;
	}
	//处理接口
	virtual bool Process(T_JOBINFO* pJobInfo, T_TASKINFO* pTaskInfo, bool* ret)
	{
		bool tmpret = true;
		//long tmp = pTaskInfo->n;
		long n2 = pTaskInfo->n2;

		//if(0!=tmp%100)
		//{
		//	thelog<<"错误："<<tmp<<" "<<pTaskInfo->n<<ende;
		//	tmpret=false;
		//}
		//if(pTaskInfo->n2 != pTaskInfo->n3+1 || pTaskInfo->n3 != pTaskInfo->n4)
		//{
		//	thelog<<"错误："<<pTaskInfo->n<<" "<<pTaskInfo->n2<<" "<<pTaskInfo->n3<<" "<<pTaskInfo->n4<<ende;
		//	tmpret=false;
		//}
		for (long i = 0; i < 1000; ++i)
		{
			char buf[256];
			sprintf(buf, "%ld\n", pTaskInfo->n);
		}

		pTaskInfo->n += 1;
		pTaskInfo->n3 = n2;
		*ret = tmpret;
		return true;
	}
	virtual bool UnProcess(T_JOBINFO* pJobInfo, T_TASKINFO* pTaskInfo, bool* ret) { return true; }
	//汇总接口
	//打开job
	virtual bool OpenJob_Rollup(T_JOBINFO* pJobInfo)
	{
		//if(!outfile.OpenW("tmp.dat"))return false;

		pJobInfo->n += 10;
		return true;
	}
	virtual bool Rollup(T_JOBINFO* pJobInfo, T_TASKINFO* pTaskInfo, bool* ret)
	{
		bool tmpret = true;
		//long tmp = pTaskInfo->n;
		long n3 = pTaskInfo->n3;

		STATIC_G long k = 0;
		if (++k >= 1000)
		{
			*ret = false;
			return true;
		}
		//if(1!=tmp%100)
		//{
		//	thelog<<"错误："<<tmp<<" "<<pTaskInfo->n<<ende;
		//	tmpret=false;
		//}
		//if(pTaskInfo->n2 != pTaskInfo->n3 || pTaskInfo->n3 != pTaskInfo->n4+1)
		//{
		//	thelog<<"错误："<<pTaskInfo->n<<" "<<pTaskInfo->n2<<" "<<pTaskInfo->n3<<" "<<pTaskInfo->n4<<ende;
		//	tmpret=false;
		//}
		char buf[256];
		sprintf(buf, "%ld\n", pTaskInfo->n);
		//outfile.SeekBegin();
		//outfile.Write(buf,strlen(buf));

		pTaskInfo->n += 10;
		pTaskInfo->n4 = n3;
		*ret = tmpret;
		return true;
	}
	//结束job
	virtual bool CloseJob_Rollup(T_JOBINFO* pJobInfo)
	{
		//outfile.Close();

		pJobInfo->n += 100;
		return true;
	}
};
//////////////////////////////////////////////////////
//简单多进程框架测试
class CTestCSimpleMultiProcess_mutex : public CSimpleMultiProcess::IChildProcess
{
public:
	enum { N = 100 };
private:
	struct struct_data
	{
		long t;
		long a;
		long _[N];
		long b;

		void zero()
		{
			memset(this, 0, sizeof(struct_data));
		}
		bool compliance()const
		{
			for (long i = 0; i < N; ++i)
			{
				if (a != _[i])return false;
			}
			return a == _[0] && a == b;
		}
		friend ostream& operator<<(ostream& o, struct_data const& tmp)
		{
			map<long, int> _s;
			for (long i = 0; i < N; ++i)
			{
				++_s[tmp._[i]];
			}
			o << tmp.t << " " << tmp.a;
			for (auto& v : _s)o << " " << v.first << "(" << v.second << ")";
			o << " " << tmp.b;
			return o;
		}
		//stringstream& toString(stringstream& ss)const
		//{
		//}
	};
	struct D
	{
		long a;
		volatile long volatile_a;

		atomic_flag atomicflag;
		atomic_long atomic_a;
		atomic_long atomic_b;
		atomic_long atomic_c;

		atomic<struct_data > atomic_d;
		struct_data no_atomic_d;

		CMyShmMutex::mySEM sem2;
		CMyRWMutex3::mySEM sem3;

		void init()
		{
			sem2.init();
			sem3.init();
			reinit();
		}
		void reinit()
		{
			a = 0; volatile_a = 0;
			atomicflag.clear();
			atomic_a = 0; atomic_b = 0; atomic_c = 0;
			struct_data tmp;
			tmp.zero();
			atomic_d.store(tmp);

			no_atomic_d.zero();
		}
		string to_string()const
		{
			stringstream ss;
			struct_data tmp = atomic_d.load();
			ss << a << " " << volatile_a << " " << atomic_a << " " << atomic_b << " " << atomic_c << " atomic_d[" << tmp << "] no_atomic_d[" << no_atomic_d << " ]";
			return ss.str();
		}
	};
	int mode;
	long count;
	int shm_id;
	CMyRWMutex mutex;
	CMyShmMutex mutex2;
	CMyRWMutex3 mutex3;
	CPThreadMutex threadmutex;

	D* main_p;

	CTestCSimpleMultiProcess_mutex() :mode(-1), shm_id(-1), main_p(nullptr) {}

	int doChildProcess(long max_process, long i_process)override
	{
		thelog << "子进程/线程 " << i_process << endi;
		D* p;
		if (isThread)
		{
			p = main_p;
		}
		else
		{
			CShmMan::Disconnect((PSHM)main_p);//断开
			int tmp_id = CShmMan::CreatePrivateSHM((i_process + 1) * 4096);//临时块，占个位置，确保每个进程使用的地址都不相同
			if (tmp_id < 0)
			{
				thelog << "创建占位共享内存失败" << ende;
				return false;
			}
			p = (D*)CShmMan::ConnectByID(tmp_id, false);
			if (nullptr == p)
			{
				thelog << "连接占位共享内存失败" << ende;
				return false;
			}
			CShmMan::Destory(tmp_id);//仍然是连接的
			p = (D*)CShmMan::ConnectByID(shm_id, false);
			if (nullptr == p)
			{
				thelog << "连接共享内存失败" << ende;
				return false;
			}
			thelog << i_process << " p=" << p << endi;
		}

		if (!mutex2.Attach(&p->sem2))
		{
			thelog << "mutex2.Attach error " << mutex2.GetErrorMessage() << ende;
			return false;
		}
		if (!mutex3.Attach(&p->sem3))
		{
			thelog << "mutex2.Attach error " << mutex2.GetErrorMessage() << ende;
			return false;
		}

		switch (mode)
		{
		case 0:
			for (long i = 0; i < count; ++i)
			{
				++p->a;
			}
			break;
		case 1:
			for (long i = 0; i < count; ++i)
			{
				++p->volatile_a;
			}
			break;
		case 2:
			for (long i = 0; i < count; ++i)
			{
				++(p->atomic_a);
			}
			break;
		case 3:
			for (long i = 0; i < count; ++i)
			{
				long tmp = p->atomic_b.fetch_sub(1);//先扣减
				if (tmp <= 0)
				{
					++p->atomic_b;//如果不够，吐回去
					++p->atomic_c;
				}
				++(p->atomic_a);
			}
			break;
		case 4:
		{
			struct_data tmp;
			long no_atomic_d_error{ 0 };
			long no_atomic_d_error2{ 0 };
			for (long i = 0; i < count; ++i)
			{
				if (!p->no_atomic_d.compliance())++no_atomic_d_error2;
				++p->no_atomic_d.a;
				for (long _i = 0; _i < N; ++_i)++p->no_atomic_d._[_i];
				++p->no_atomic_d.b;
				if (!p->no_atomic_d.compliance())++no_atomic_d_error;

				tmp = p->atomic_d.load();
				if (!tmp.compliance())
				{
					thelog << "子进程 " << i_process << " 不一致 " << sizeof(tmp) << " " << sizeof(p->atomic_d) << " : " << tmp << ende;
					exit(0);
				}
				++tmp.a;
				for (long _i = 0; _i < N; ++_i)++tmp._[_i];
				++tmp.b;
				p->atomic_d.store(tmp);
			}
			thelog << "no_atomic_d_error " << no_atomic_d_error << " " << no_atomic_d_error2 << " " << tmp << endi;
		}
		break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		{
			//struct_data tmp;
			long count_idle{ 0 };
			long no_atomic_d_error{ 0 };
			long no_atomic_d_error2{ 0 };
			for (long i = 0; i < count; ++i)
			{
				while (true)
				{
					long want{ 0 };
					if (5 == mode)
					{
						if (p->a % max_process == i_process)break;
					}
					else if (6 == mode)
					{
						if (p->atomic_a % max_process == i_process)break;
					}
					else if (7 == mode)
					{
						if (p->atomic_a.compare_exchange_weak(want, 1))break;
					}
					else if (8 == mode)
					{
						if (!p->atomicflag.test_and_set()) break;
					}
					else if (9 == mode)
					{
						if (mutex2.TryWLock())break;
					}
					else if (10 == mode)
					{
						if (mutex3.TryWLock())break;
					}
					else if (11 == mode)
					{
						if (0 == threadmutex.lock())break;
					}
					else if (12 == mode)
					{
						if (mutex.TryWLock())break;
					}
					else if (13 == mode)
					{
						if (mutex.WLock())break;
					}
					else
					{
						thelog << "未知模式" << ende;
						return __LINE__;
					}

					this_thread::yield();
					++count_idle;
					if (0 == count_idle % 1000000000)thelog << i_process << " " << i << " " << p->a << " " << p->atomic_a << " " << count_idle << endi;
				}

				if (!p->no_atomic_d.compliance())++no_atomic_d_error;
				++p->no_atomic_d.a;
				for (long _i = 0; _i < N; ++_i)++p->no_atomic_d._[_i];
				++p->no_atomic_d.b;
				if (!p->no_atomic_d.compliance())++no_atomic_d_error2;

				if (5 == mode)++p->a;
				else if (6 == mode)++p->atomic_a;
				else if (7 == mode)p->atomic_a = 0;
				else if (8 == mode)p->atomicflag.clear();
				else if (9 == mode)mutex2.WUnLock();
				else if (10 == mode)mutex3.WUnLock();
				else if (11 == mode)threadmutex.unlock();
				else if (12 == mode)mutex.WUnLock();
				else if (13 == mode)mutex.WUnLock();
				else
				{
					thelog << "未知模式" << ende;
					return __LINE__;
				}

			}
			thelog << "no_atomic_d_error " << no_atomic_d_error << " " << no_atomic_d_error2 << " count_idle " << count_idle << endi;
		}
		break;
		case 14:
			for (long i = 0; i < count; ++i)
			{
				mutex2.WLock();
				++p->a;
				mutex2.WUnLock();
			}
			break;
		case 15:
			for (long i = 0; i < count; ++i)
			{
				mutex.WLock();
				++p->a;
				mutex.WUnLock();
			}
			break;
		default:
			thelog << "未知的mode " << mode << ende;
			return 1;
		}
		return 0;
	}
	bool init()
	{
		shm_id = CShmMan::CreatePrivateSHM(sizeof(D));
		if (shm_id < 0)
		{
			thelog << "创建私有共享内存失败" << ende;
			return false;
		}
		main_p = (D*)CShmMan::ConnectByID(shm_id, false);
		if (nullptr == main_p)
		{
			thelog << "连接私有共享内存失败" << ende;
			return false;
		}

		if (!mutex.Create())
		{
			thelog << "创建信号量失败" << ende;
			return false;
		}

		if (!mutex2.Create(&main_p->sem2))
		{
			thelog << "mutex2.Create" << ende;
			return false;
		}
		if (!mutex2.Detach())
		{
			thelog << "mutex2.Detach" << ende;
			return false;
		}

		if (!mutex3.Create(&main_p->sem3))
		{
			thelog << "mutex3.Create" << ende;
			return false;
		}
		if (!mutex3.Detach())
		{
			thelog << "mutex3.Detach" << ende;
			return false;
		}

		threadmutex.init();

		return true;
	}
	bool uninit()
	{
		if (shm_id >= 0)CShmMan::Destory(shm_id);
		main_p = nullptr;

		mutex.Destory();
		return true;
	}
public:
	static int doTest(int argc, char** argv)
	{
		thelog << "long " << sizeof(long) << endi;
		thelog << "atomic_long " << sizeof(atomic_long) << endi;
		thelog << "atomic_flag " << sizeof(atomic_flag) << endi;
		CTestCSimpleMultiProcess_mutex t;

		if (!t.init())return __LINE__;

		thelog << t.main_p->atomic_a.is_lock_free() << endi;
		thelog << t.main_p->atomic_d.is_lock_free() << endi;

		UIInput("press any key to continue", "");

		int ret = 0;
		for (int m = 0; m < 100; ++m)
		{
			t.mode = m;
			t.count = 1000000;

			t.main_p->reinit();
			t.main_p->atomic_b = t.count * 9;
			if (0 != CSimpleMultiProcess::SimpleMultiProcess(&t, 10, "多进程测试"))break;
			thelog << "模式 " << m << " 执行完成，结果为 " << t.main_p->to_string() << endi;

			t.main_p->reinit();
			t.main_p->atomic_b = t.count * 9;
			ret = CSimpleMultiProcess::SimpleMultiThread(&t, 10, "多线程测试");
			thelog << "模式 " << m << " 执行完成，结果为 " << t.main_p->to_string() << endi;
			SleepSeconds(5);
			thelog << "模式 " << m << " 执行完成，结果为 " << t.main_p->to_string() << endi;
		}
		if (!t.uninit())return __LINE__;
		return ret;
	}
};

//hash测试
class CTest_hash
{
private:
	struct D
	{
		long a;
		long b;
		long c;

		void init() { a = 0; b = 0; c = 0; }
		string to_string()const
		{
			stringstream ss;
			ss << a << " " << b << " " << c;
			return ss.str();
		}
	};
	typedef unsigned int T_HASH_TYPE;
	typedef bitset<4 * 1024 * 1024 * 1024UL > T_BITMAP;
	int mode;
	long count;
	T_BITMAP* pBitMap;
	T_BITMAP* pBitMap2;
	T_BITMAP* pBitMap3;
	long count_err;//冲突的个数

	CTest_hash() :mode(-1)
	{
		pBitMap = new T_BITMAP;
		pBitMap2 = new T_BITMAP;
		pBitMap3 = new T_BITMAP;
		thelog << "sizeof(T_BITMAP) " << sizeof(T_BITMAP) << endi;
	}
	~CTest_hash()
	{
		delete pBitMap;
		delete pBitMap2;
		delete pBitMap3;
	}
	int doProcess()
	{
		thelog << "mode " << mode << endi;
		//D tmp;
		hash<unsigned long> hash_obj;//hash对象
		hash<string > hash_string;//hash对象
		char buf[256];

		switch (mode)
		{
		case 0:
			for (long i = 0; i < count; ++i)
			{
				T_HASH_TYPE tmp = hash_obj(2 == i ? i + 1 : i);
				if (pBitMap->test(tmp))
				{
					++count_err;
				}
				else
				{
					pBitMap->set(tmp);
				}
				if (0 == (i + 1) % 10000000)
				{
					thelog << i + 1 << " " << pBitMap->size() << " " << count_err << " " << count_err * 10000 / (i + 1) << "%%" << endi;
				}
			}
			break;
		case 1:
			for (long i = 0; i < count; ++i)
			{
				T_HASH_TYPE tmp = hash_obj(rand());

				if (pBitMap->test(tmp))
				{
					++count_err;
				}
				else
				{
					pBitMap->set(tmp);
				}
				if (0 == (i + 1) % 10000000)
				{
					thelog << i + 1 << " " << pBitMap->size() << " " << count_err << " " << count_err * 10000 / (i + 1) << "%%" << endi;
				}
			}
			break;
		case 2:
			for (long i = 0; i < count; ++i)
			{
				int tmp_d = rand();
				T_HASH_TYPE tmp = hash_obj(tmp_d);
				sprintf(buf, "%d", tmp_d);
				T_HASH_TYPE tmp2 = hash_string(buf);
				if (pBitMap->test(tmp) && pBitMap2->test(tmp2))
				{
					++count_err;
				}
				else
				{
					pBitMap->set(tmp);
					pBitMap2->set(tmp2);
				}
				if (0 == (i + 1) % 10000000)
				{
					thelog << i + 1 << " " << pBitMap->size() << " " << count_err << " " << count_err * 10000 / (i + 1) << "%%" << endi;
				}
			}
			break;
		case 3:
			for (long i = 0; i < count; ++i)
			{
				int tmp_d = rand();
				T_HASH_TYPE tmp = hash_obj(tmp_d);
				sprintf(buf, "%d", tmp_d);
				T_HASH_TYPE tmp2 = hash_string(buf);
				sprintf(buf, "%X", tmp_d);
				T_HASH_TYPE tmp3 = hash_string(buf);
				if (pBitMap->test(tmp) && pBitMap2->test(tmp2) && pBitMap3->test(tmp3))
				{
					++count_err;
				}
				else
				{
					pBitMap->set(tmp);
					pBitMap2->set(tmp2);
					pBitMap3->set(tmp3);
				}
				if (0 == (i + 1) % 10000000)
				{
					thelog << i + 1 << " " << pBitMap->size() << " " << count_err << " " << count_err * 10000 / (i + 1) << "%%" << endi;
				}
			}
			break;
		case 4:
			for (long i = 0; i < count; ++i)
			{
				srand(i);
				int tmp_d = rand();
				T_HASH_TYPE tmp = hash_obj(tmp_d);
				sprintf(buf, "%d", tmp_d);
				T_HASH_TYPE tmp2 = hash_string(buf);
				sprintf(buf, "%X", tmp_d);
				T_HASH_TYPE tmp3 = hash_string(buf);
				if (pBitMap->test(tmp) && pBitMap2->test(tmp2) && pBitMap3->test(tmp3))
				{
					++count_err;
				}
				else
				{
					pBitMap->set(tmp);
					pBitMap2->set(tmp2);
					pBitMap3->set(tmp3);
				}
				if (0 == (i + 1) % 10000000)
				{
					thelog << i + 1 << " " << pBitMap->size() << " " << count_err << " " << count_err * 10000 / (i + 1) << "%%" << endi;
				}
			}
			break;
		default:
			thelog << "未知的mode " << mode << ende;
		}
		return 0;
	}
public:
	static int doTest(int argc, char** argv)
	{
		CTest_hash t;
		int ret = 0;
		stringstream ss;
		for (int m = 0; m < 5; ++m)
		{
			t.mode = m;
			t.count = 500 * 1000 * 1000;
			t.pBitMap->reset();
			t.pBitMap2->reset();
			t.pBitMap3->reset();
			t.count_err = 0;
			srand(0);

			time_t t1 = time(NULL);
			ret = t.doProcess();
			ss << "模式 " << m << " 用时 " << time(NULL) - t1 << " 结果集 " << t.pBitMap->size() << " 冲突 " << t.count_err << " " << t.count_err * 10000 / t.count << "%%" << endl;
		}
		thelog << endl << ss.str() << endi;
		return ret;
	}
};

//////////////////////////////////////////////////////
//压力测试框架测试
class CTestCStressTesting : public CStressTesting
{
private:
	//初始化进程，参数：总进程数，本进程号，每批次任务数
	virtual bool InitChildProcess(long max_process, long i_process, long batch)
	{
		return true;
	}
	//清理进程
	virtual bool FimishChildProcess()
	{
		return true;
	}
	//执行，参数：总进程数，本进程号，每批次任务数，批次号，输出信息
	virtual bool doOneBatch(long max_process, long i_process, long max_batch_process, long batch, long i_batch, string& msg)
	{
		SleepSeconds(1);
		msg = "批次完成";
		return true;
	}
public:
	static int doTest(int argc, char** argv)
	{
		CTestCStressTesting t;
		return t.doStressTesting(false, 128, 128, 5);
	}
};

////////////////////////////////////////////////////////
//UDP服务测试
class CUDPServer
{
public://消息结构
	struct structMessage
	{
		int nId;
		char szId[32];
		char szMsg[256];
	};
private:
	int m_Socket;
	sockaddr_in m_udpsa_peer;
	structMessage m_structmessage;
	bool UDPRecvFrom(sockaddr_in& udpsa, structMessage& structmessage)
	{
		socklen_t fromlen = sizeof(sockaddr_in);
		int recvcount;
		//char str[256];
		//int ii;
		STATIC_G unsigned long ulAuto = 0;//自动增量，记录消息接收顺序

		recvcount = recvfrom(this->m_Socket, (char*)&structmessage, sizeof(structMessage), 0, (sockaddr*)&udpsa, &fromlen);
		if (0 == recvcount || -1 == recvcount || recvcount != sizeof(structMessage))
		{
			if (-1 == recvcount)
			{
			}
			else
			{
				thelog << "接收出错" << ende;
				return false;
			}
		}
		else
		{
			ulAuto++;
			if (1 == ulAuto || 0 == ulAuto % 10000)
			{
				thelog << "接收成功 " << ulAuto << " : " << inet_ntoa(udpsa.sin_addr) << " " << ntohs(udpsa.sin_port) << " : " << structmessage.nId << " " << structmessage.szId << " " << structmessage.szMsg << endi;
			}
		}

		return true;
	}
	bool SendUdp(sockaddr_in* sa, const int nId, const char* szId, const char* szMsg)
	{
		structMessage structmessage;

		structmessage.nId = nId;
		strncpy(structmessage.szId, szId, 32);
		structmessage.szId[31] = '\0';
		strncpy(structmessage.szMsg, szMsg, 256);
		structmessage.szMsg[255] = '\0';
		//send
		if (-1 == sendto(m_Socket, (char*)&structmessage, sizeof(structmessage), 0, (sockaddr*)sa, sizeof(sockaddr_in)))
		{
			close(m_Socket);
			thelog << "发送出错" << ende;
			return false;
		}

		return true;
	}
	bool SendUdp(const char* toAddr, const unsigned short toPort,
		const int nId, const char* szId, const char* szMsg)
	{
		sockaddr_in udpsa;

		//设置发送目标
		udpsa.sin_family = AF_INET;
		udpsa.sin_addr.s_addr = inet_addr(toAddr);
		udpsa.sin_port = htons(toPort);

		//send
		if (!SendUdp(&udpsa, nId, szId, szMsg))
		{
			thelog << "发送出错" << ende;
			return false;
		}

		return true;
	}
public:
	bool UDPServerStart(int port)
	{
		sockaddr_in udpsa;
		//int fromlen = sizeof(sockaddr_in);

		//建立 UDP SOCKET
		if (-1 == (this->m_Socket = socket(AF_INET, SOCK_DGRAM, 0)))
		{
			thelog << "create socket error!" << ende;
			return false;
		}
		//bind
		udpsa.sin_family = AF_INET;
		udpsa.sin_addr.s_addr = 0;
		udpsa.sin_port = htons(port);
		if (-1 == bind(this->m_Socket, (sockaddr*)&udpsa, sizeof(sockaddr_in)))
		{
			thelog << "bind socket error!" << ende;
			close(this->m_Socket);
			return false;
		}
		return true;
	}

	//发送UDP
	bool UDPClientStart()
	{
		//建立客户端 UDP SOCKET
		if (-1 == (m_Socket = socket(AF_INET, SOCK_DGRAM, 0)))
		{
			thelog << "创建socket出错" << ende;
			return false;
		}

		return true;
	}

	int _testClient(int maxcount)
	{
		if (!UDPClientStart())return __LINE__;

		long i;
		char buf[256];
		for (i = 0; i < maxcount; ++i)
		{
			sprintf(buf, "%ld", i);
			if (!SendUdp("132.126.3.246", 10000, i, buf, "abc"))
			{
				thelog << "发送失败" << ende;
				return __LINE__;
			}
			if (!UDPRecvFrom(m_udpsa_peer, m_structmessage))
			{
				thelog << "接收失败" << ende;
				return __LINE__;
			}
		}
		return 0;
	}
	int _testServer(int argc, char** argv)
	{
		if (!UDPServerStart(10000))return __LINE__;

		long i = 0;
		while (UDPRecvFrom(m_udpsa_peer, m_structmessage))
		{
			if (!SendUdp(&m_udpsa_peer, i++, "server", "abc"))
			{
				thelog << "发送失败" << ende;
				return __LINE__;
			}
		}
		return 0;
	}
	static int testClient(int argc, char** argv)
	{
		CUDPServer client;
		return client._testClient(1000000);
	}
	static int testServer(int argc, char** argv)
	{
		CUDPServer server;
		return server._testServer(argc, argv);
	}
};

class CTestShmIActiveObject : public CShmActiveObjects
{
public:
	class A : public IShmActiveObject
	{
		virtual char const* GetName()const { return "A"; }
		virtual bool _Attach(bool isReadOnly) { return true; }
		virtual bool isConnected()const { return true; }
		size_t size()const override
		{
			STATIC_G long i = 0;
			return i++;
		}
		size_t capacity()const override
		{
			STATIC_G long i = 0;
			return (++i) * 10;
		}
	};
	class B : public CShmActiveObjects
	{
	public:
		A a;
		A b;
		A c;
		B()
		{
			AddTable(&a);
			AddTable(&b);
			AddTable(&c);
		}
		virtual char const* GetName()const { return "B"; }
		virtual bool isConnected()const { return true; }
	};
public:
	A a;
	B b;
	A c;
	CTestShmIActiveObject()
	{
		AddTable(&a);
		AddTable(&b);
		AddTable(&c);
	}
	virtual char const* GetName()const { return "CTestShmIActiveObject"; }
	virtual bool isConnected()const { return true; }
	static int test(int argc, char** argv)
	{
		CTestShmIActiveObject me;
		string str;
		me.Attach(true);
		thelog << endl << me.ShowRet(str, true) << endi;
		me.RunCmdUI();
		return 0;
	}
};

class CTestT_SHM_SET_GROUP
{
private:
	struct CData : public CActiveObjectBase
	{
		long a;
		long b;
		long c;
		long d;
		long x;

		//用于需要排序的场合
		bool operator < (CData const& tmp)const
		{
			if (a != tmp.a)return a < tmp.a;
			else if (b != tmp.b)return b < tmp.b;
			else if (c != tmp.c)return c < tmp.c;
			else return d < tmp.d;
		}
		//某些场合也需要等于
		bool operator == (CData const& tmp)const { return !(*this < tmp || tmp < *this); }

		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return b; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "CDemoData=%ld %ld %ld %ld %ld", a, b, c, d, x);
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("A", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("B", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("C", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("D", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("X", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(a);
			table.AddData(b);
			table.AddData(c);
			table.AddData(d);
			table.AddData(x);
			return true;
		}
	};
	typedef T_SHM_SET_GROUP<CData, 1> T_DATAS;
	T_DATAS m_datas;

	CTestT_SHM_SET_GROUP() :m_datas("set_group", 0) {}
	bool doTest()
	{
		if (!CShmEnv::getInstPtr()->ShmEnvConnect())return false;
		if (!m_datas.DestoryShm())return false;
		if (!m_datas.adminCreateShm())return false;
		if (!m_datas.Attach(false))return false;

		long max_b = 1000 * 10000;
		time_t t1 = time(NULL);
		CData data;
		long b, d;
		data.a = 1;
		data.c = 2;
		data.x = 0;

		thelog << "开始插入。。。 " << m_datas.size() << endi;
		for (b = 1; b <= max_b; ++b)
		{
			data.b = b;
			for (d = 1; d <= 5; ++d)
			{
				data.d = d;
				m_datas.update(data);
			}
		}
		thelog << "插入结束 " << time(NULL) - t1 << " 秒 " << max_b / (time(NULL) - t1) << "/秒 " << m_datas.size() << endi;
		m_datas.ReportData();
		thelog << "开始更新。。。 " << m_datas.size() << endi;
		for (b = 1; b <= max_b; ++b)
		{
			data.b = b;
			for (d = 1; d <= 5; ++d)
			{
				data.d = d;
				m_datas.get(data);
				data.x += 1;
				m_datas.update(data);
			}
		}
		thelog << "更新结束 " << time(NULL) - t1 << " 秒 " << max_b / (time(NULL) - t1) << "/秒 " << m_datas.size() << endi;
		m_datas.ReportData();
		return true;
	}
public:
	static int test(int argc, char** argv)
	{
		CTestT_SHM_SET_GROUP tmp;
		tmp.doTest();
		return 0;
	}
};
