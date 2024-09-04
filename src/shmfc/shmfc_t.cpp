
#include "shmfctest.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "shmBinaryPool.h"
#include "shmListSet.h"
#include <algorithm>
#include "../function/BitSet.h"
#include "shmHash.h"
#include "../env/CommandSet.h"
#include "shmenvmgr.h"
#include "shmtools.h"
#include "shmfctest2.h"

using namespace ns_my_std;

typedef CMultiProcessServerGroup<CDemoData, CDemoData, CDemoTask, 10L, 20L, 5L, 3000L > MPS_T;
key_t key = 0x10000;

CDemoMPS<CDemoData, CDemoTask > demoMPS;

int test_CMultiProcessServer(int argc, char** argv)
{
	G_IS_DEBUG = false;
	thelog << "多进程服务器测试" << endi;
	char* pshm = CShmMan::ConnectByKey(key, false);
	if (NULL == pshm)
	{
		int shmid = CShmMan::CreateSHMByKey(key, MPS_T::calcBufSise());
		thelog << "shmid=" << shmid << endi;
		if (shmid < 0)return __LINE__;

		pshm = CShmMan::ConnectByKey(key, false);
		if (NULL == pshm)
		{
			CShmMan::Destory(shmid);
			return __LINE__;
		}
	}

	string str;

	MPS_T mps(&demoMPS, &demoMPS, &demoMPS, pshm);
	mps.clear();
	mps.SetMaxProcess(20);
	mps.SetAutoHighLow(false);
	mps.SetExitIfNoJob(false);
	mps.SetNotAutoSleep(false);
	mps.SetExitIfProcessCoredump(false);

	//thelog<<endl<<mps.toString(str)<<endi;
	int ret = mps.run();
	//thelog<<endl<<mps.toString(str)<<endi;

	CShmMan::Disconnect(pshm);
	//CShmMan::Destory(shmid);
	return ret;
}
int test_CMultiProcessServer_view(int argc, char** argv)
{
	thelog << "多进程服务器查看" << endi;
	char* pshm = CShmMan::ConnectByKey(key, false);

	MPS_T mps(&demoMPS, &demoMPS, &demoMPS, pshm);

	mps.view();

	CShmMan::Disconnect(pshm);
	return 0;
}
int test_CMultiProcessServer_speed(int argc, char** argv)
{
	thelog << "多进程服务器速度监测" << endi;
	char* pshm = CShmMan::ConnectByKey(key, false);

	MPS_T mps(&demoMPS, &demoMPS, &demoMPS, pshm);

	mps.speed(0, true);

	CShmMan::Disconnect(pshm);
	return 0;
}
int test_CMultiProcessServer_direct(int argc, char** argv)
{
	CStdOSFile outfile;
	CStdOSFile infile;
	long i;
	char const* filename = "test.tmp";
	char const* outfilename = "testout.tmp";
	long n = 1000 * 10000;
	long count;
	string line;
	clock_t tmpclock;
	timeval t3;
	timeval t4;

	for (i = 0; i < 300; ++i)
	{
		line += 'a' + i % 26;
	}
	line += "\n";

	if (!outfile.OpenW(filename))
	{
		return __LINE__;
	}
	count = 0;
	gettimeofday(&t3, NULL);
	tmpclock = clock();
	thelog << "开始简单写入。。。" << endi;
	for (i = 0; i < n; ++i)
	{
		if (!outfile.Write(line.c_str(), line.size()))return __LINE__;
		++count;
	}
	gettimeofday(&t4, NULL);
	thelog << "写入完成 count=" << count
		<< " timeval " << MPS_T::timespan_usec(&t4, &t3) / 1000000 << " sec"
		<< " clock " << MPS_T::clock_percent(clock(), tmpclock, MPS_T::timespan_usec(&t4, &t3)) << "%" << endi;
	outfile.Close();

	if (!outfile.OpenW(filename))
	{
		return __LINE__;
	}
	count = 0;
	gettimeofday(&t3, NULL);
	tmpclock = clock();
	thelog << "开始复杂写入。。。" << endi;
	for (i = 0; i < n; ++i)
	{
		STATIC_G char buf[301];
		for (long j = 0; j < 300; ++j)
		{
			buf[j] = 'a' + i % 26;
		}
		buf[300] = '\n';
		sprintf(buf, "%-20ld", i);
		buf[20] = ' ';
		if (!outfile.Write(buf, 301))return __LINE__;
		++count;
	}
	gettimeofday(&t4, NULL);
	thelog << "写入完成 count=" << count
		<< " timeval " << MPS_T::timespan_usec(&t4, &t3) / 1000000 << " sec"
		<< " clock " << MPS_T::clock_percent(clock(), tmpclock, MPS_T::timespan_usec(&t4, &t3)) << "%" << endi;
	outfile.Close();

	if (!infile.OpenR(filename))
	{
		return __LINE__;
	}
	char buf[10240];
	count = 0;
	gettimeofday(&t3, NULL);
	tmpclock = clock();
	thelog << "开始简单读取。。。" << endi;
	for (i = 0; i < n; ++i)
	{
		if (0 > infile.Read(buf, line.size()))return __LINE__;
		++count;
	}
	gettimeofday(&t4, NULL);
	thelog << "读取完成 count=" << count
		<< " timeval " << MPS_T::timespan_usec(&t4, &t3) / 1000000 << " sec"
		<< " clock " << MPS_T::clock_percent(clock(), tmpclock, MPS_T::timespan_usec(&t4, &t3)) << "%" << endi;
	infile.Close();

	return 0;

	CDemoData demodata;
	CDemoTask demotask;
	bool ret;

	count = 0;
	gettimeofday(&t3, NULL);
	tmpclock = clock();
	thelog << "开始demo分发MPS。。。" << endi;
	if (!demoMPS.OpenJob(&demodata, &ret))return __LINE__;
	while (true)
	{
		if (!demoMPS.DistributeTask(&demodata, &demotask, &ret))break;
		if (!demoMPS.isJobEnd(&demodata, &ret))break;
		if (ret)break;
		++count;
	}
	demoMPS.CloseJob_Distribute(&demodata);
	gettimeofday(&t4, NULL);
	thelog << "demoMPS分发完成 count=" << count
		<< " timeval " << MPS_T::timespan_usec(&t4, &t3) / 1000000 << " sec"
		<< " clock " << MPS_T::clock_percent(clock(), tmpclock, MPS_T::timespan_usec(&t4, &t3)) << "%" << endi;

	count = 0;
	gettimeofday(&t3, NULL);
	tmpclock = clock();
	thelog << "开始demoMPS全流程。。。" << endi;
	if (!demoMPS.OpenJob(&demodata, &ret))return __LINE__;
	if (!demoMPS.OpenJob_Rollup(&demodata))return __LINE__;
	while (true)
	{
		if (!demoMPS.DistributeTask(&demodata, &demotask, &ret))break;
		//if(!demoMPS.Process(&demodata,&demotask,&ret))break;
		if (!demoMPS.Rollup(&demodata, &demotask, &ret))break;
		if (!demoMPS.isJobEnd(&demodata, &ret))break;
		if (ret)break;
		++count;
	}
	demoMPS.CloseJob_Distribute(&demodata);
	demoMPS.CloseJob_Rollup(&demodata);
	gettimeofday(&t4, NULL);
	thelog << "demoMPS全流程完成 count=" << count
		<< " timeval " << MPS_T::timespan_usec(&t4, &t3) / 1000000 << " sec"
		<< " clock " << MPS_T::clock_percent(clock(), tmpclock, MPS_T::timespan_usec(&t4, &t3)) << "%" << endi;

	if (!infile.OpenR(filename))
	{
		return __LINE__;
	}
	if (!outfile.OpenW(outfilename))
	{
		return __LINE__;
	}
	unsigned short port = 60000;
	CMySocket ss;
	CMySocket cs_array[100];
	if (!ss.Listen(port))return __LINE__;
	for (long loop = 1; loop < 20; loop *= 2)
	{
		long const child_count = loop;
		int child_i;
		for (child_i = 0; child_i < child_count; ++child_i)
		{
			pid_t pid = fork();
			if (pid > 0)
			{
				thelog << "子进程 " << child_i << " " << pid << endi;
				cs_array[child_i] = ss.Accept();
			}
			else if (0 == pid)
			{
				CMySocket cs;
				if (!cs.Connect("127.0.0.1", port))
				{
					thelog << child_i << " connect error" << ende;
					exit(0);
				}
				char buf[10240];
				long totalcount = 0;
				long readcount;
				long count = 0;
				while (true)
				{
					if (!cs.Recv(buf, 10240, &readcount))
					{
						thelog << child_i << " recv error" << ende;
						break;
					}
					if (0 == readcount)
					{
						break;
					}
					//if (readcount != line.size())thelog << line.size() << " " << readcount << endi;
					totalcount += readcount;

					if (!cs.Send(buf, readcount))
					{
						thelog << child_i << " send error" << ende;
						break;
					}
					++count;
				}
				thelog << child_i << " " << count << " total recv " << totalcount << endi;
				cs.Close();
				exit(0);
			}
			else return __LINE__;
		}
		count = 0;
		long readcount;
		gettimeofday(&t3, NULL);
		tmpclock = clock();
		thelog << "开始读写，用socket传输。。。" << endi;
		for (i = 0; i < n; ++i)
		{
			if (i >= loop)
			{
				if (!cs_array[i % child_count].Recv(buf, 10240, &readcount))return __LINE__;
				outfile.Write(buf, readcount);
			}
			if (0 > infile.Read(buf, line.size()))return __LINE__;
			cs_array[i % child_count].Send(buf, line.size());
			++count;
		}
		gettimeofday(&t4, NULL);
		thelog << loop << " 读写完成 count=" << count
			<< " timeval " << MPS_T::timespan_usec(&t4, &t3) / 1000000 << " sec"
			<< " clock " << MPS_T::clock_percent(clock(), tmpclock, MPS_T::timespan_usec(&t4, &t3)) << "%" << endi;
		infile.Close();
		outfile.Close();
		for (child_i = 0; child_i < child_count; ++child_i)
		{
			cs_array[child_i].Close();
		}
	}
	ss.Close();

	return 0;
}
int test_BinaryPool(int argc, char** argv)
{
	g_isShmPoolTrance = true;
	g_isMakeShmPoolData = true;
	string str;
	long i;

	ShmPool& shmpool = *ShmPool::getInstPtr();
	ShmPool::HANDLE h, h2;

	thelog << "=====================================" << endi;
	if (shmpool.IsConnected())
	{
		if (shmpool.Detach())thelog << "成功断开" << endi;
		else thelog << "断开失败" << ende;
	}

	if (shmpool.DestoryShm())thelog << "成功删除" << endi;
	else thelog << "删除失败" << ende;

	if (shmpool.Attach(false))
	{
		thelog << "连接成功，显示" << endi;
		shmpool.view();
	}
	else
	{
		thelog << "连接失败，重新创建" << endi;
		shmpool.DestoryShm();
		shmpool.CreateShm();
		if (!shmpool.Attach(false))return __LINE__;
		thelog << "创建完成，显示" << endi;
		shmpool.Report();
		thelog << endi;
		if (!shmpool.Allocate(10, h))
		{
			thelog << "Allocate error" << ende;
			return __LINE__;
		}
		thelog << endi;
		strcpy(&*h, "abc");
		thelog << endi;
		if (!shmpool.Allocate(10, h2))
		{
			thelog << "Allocate error" << ende;
			return __LINE__;
		}
		thelog << endi;
		strcpy(&*h2, "def");
		thelog << endi;
		if (!shmpool.Deallocate(h))
		{
			thelog << "Deallocate error" << ende;
			return __LINE__;
		}
		thelog << endi;
		shmpool.view();
		thelog << "-------------------------------" << endi;
		if (!shmpool.Allocate(sizeof(shm_vector<long >), h))
		{
			thelog << "申请数组头失败" << ende;
			return __LINE__;
		}
		new(&*h) shm_vector<long >;
		shmpool.AddEntry("v", h);
		shmpool.Report();
		thelog << "-------------------------------" << endi;
		if (!shmpool.Allocate(sizeof(shm_set<long >), h))
		{
			thelog << "申请set头失败" << ende;
			return __LINE__;
		}
		new(&*h) shm_set<long >;
		shmpool.AddEntry("s", h);
		thelog << "-------------------------------" << endi;
		if (!shmpool.Allocate(sizeof(shm_map<long, long >), h))
		{
			thelog << "申请map头失败" << ende;
			return __LINE__;
		}
		new(&*h) shm_map<long, long >;
		shmpool.AddEntry("m", h);
		thelog << "-------------------------------" << endi;
	}
	h = shmpool.GetEntry("v");
	shm_vector<long >& v = *(shm_vector<long >*) & *h;
	shm_vector<long >::const_iterator it_v;
	h = shmpool.GetEntry("s");
	shm_set<long >& s = *(shm_set<long >*) & *h;
	shm_set<long >::const_iterator it_s;
	h = shmpool.GetEntry("m");
	shm_map<long, long >& m = *(shm_map<long, long >*) & *h;
	shm_map<long, long >::const_iterator it_m;
	thelog << "-------------------------------" << endi;
	shmpool.Report();

	thelog << "-------------------------------" << endi;
	//v.clear();
	thelog << "-------------------------------" << endi;
	thelog << v.capacity() << endi;
	thelog << "-------------------------------" << endi;
	thelog << "v.size=" << v.size() << endi;
	thelog << "-------------------------------" << endi;
	v.reserve(5);
	thelog << "-------------------------------" << endi;
	shmpool.Report();
	thelog << "v.capacity=" << v.capacity() << endi;
	thelog << "-------------------------------" << endi;
	thelog << "v.size=" << v.size() << endi;
	thelog << "-------------------------------" << endi;
	v.push_back('a');
	thelog << "-------------------------------" << endi;
	v.push_back('a');
	thelog << "-------------------------------" << endi;
	v.push_back(0x0123456789abcdef);
	thelog << "-------------------------------" << endi;
	thelog << v[0] << endi;
	thelog << "-------------------------------" << endi;
	thelog << v[1] << endi;
	thelog << "-------------------------------" << endi;
	thelog << v[2] << endi;
	thelog << "-------------------------------" << endi;
	thelog << v.capacity() << endi;
	thelog << "-------------------------------" << endi;
	thelog << "v.size=" << v.size() << endi;
	shmpool.Report();
	thelog << "-------------------------------" << endi;
	v.push_back('a');
	v.push_back('b');
	v.push_back('c');
	v.push_back('d');
	v.push_back('e');
	v.push_back('f');
	v.push_back('g');
	v.push_back('h');
	v.push_back('i');
	v.push_back('j');
	v.push_back('k');
	v.push_back('E');
	thelog << "-------------------------------" << endi;
	thelog << v.capacity() << endi;
	thelog << "-------------------------------" << endi;
	thelog << "v.size=" << v.size() << endi;
	thelog << "-------------------------------" << endi;
	shmpool.Report();
	for (it_v = v.begin(); it_v != v.end(); ++it_v)
	{
		thelog << *it_v << endi;
	}
	thelog << "-------------------------------" << endi;
	shmpool.Report();
	thelog << "-------------------------------" << endi;
	for (i = 0; i < 100; ++i)s.insert(i);
	for (i = -1; i < 101; ++i)
	{
		it_s = s.find(i);
		if (s.end() != it_s)thelog << "找到 " << *it_s << endi;
		else thelog << "没找到 " << i << endi;
	}
	thelog << "s.size()=" << s.size() << endi;
	for (it_s = s.begin(); it_s != s.end(); ++it_s)
	{
		thelog << *it_s << endi;
	}
	shmpool.Report();
	thelog << "-------------------------------" << endi;
	m[0] = 100;
	m[1] = 101;
	thelog << "m.size()=" << m.size() << endi;
	for (it_m = m.begin(); it_m != m.end(); ++it_m)
	{
		thelog << (*it_m).first << " - " << (*it_m).second << endi;
	}
	shmpool.Report();
	if (false)
	{
		//这一段在ubuntu下异常，当然shm_string放在私有内存可能也是问题（或许之前在centos下是没问题的）
		BINARYPOOL_TRANCE = true;
		thelog << "================================" << endi;
		shm_string bs;
		thelog << "================================" << endi;
		shm_string bs2;
		thelog << "================================" << endi;
		bs.reserve(51);
		thelog << "================================" << endi;
		bs = 'c';
		thelog << "================================" << endi;
		bs = "basic";
		bs2 = "basic_string";
		if (bs < bs2)thelog << "" << endi;
		if (bs.find(""))thelog << "" << endi;
		if (bs.find_first_of(""))thelog << "" << endi;
		thelog << bs.capacity() << endi;
		thelog << bs.size() << endi;
		thelog << "=====================================" << endi;
	}
	i = 3;
	shmpool.Allocate(i, h);
	shmpool.Deallocate(h, i);
	i = 1024 * 1024;
	shmpool.Allocate(i, h);
	shmpool.Deallocate(h, i);
	shmpool.Allocate(i, h);
	shmpool.Deallocate(h, i);
	i = 32;
	shmpool.Allocate(i, h);
	i = 32;
	shmpool.Allocate(i, h);
	shmpool.Allocate(i, h2);
	shmpool.Deallocate(h, i);
	shmpool.Deallocate(h2, i);
	shmpool.view();

	return 0;
}
int test_ParseFromXml(int argc, char** argv)
{
	CHtmlDoc::CHtmlTable2 table;
	table.AddCol("col_a");
	table.AddCol("col_b");
	table.AddCol("col_c");

	long i;
	for (i = 0; i < 3; ++i)
	{
		table.AddLine();

		table.AddData(i);
		table.AddData(i + 10);
		table.AddData("12<>&\"\'34");
	}

	string str;

	thelog << endl << table.MakeTextTable() << endi;
	thelog << endl << table.MakeXML("__root__", str) << endi;

	vector<vector<pair<string, string> > > vmss_head;
	vector<vector<pair<string, string> > > vmss_record;
	table.ParseFromXml(str, "", vmss_head, vmss_record);

	vector<vector<pair<string, string> > >* p;
	for (long x = 0; x < 2; ++x)
	{
		if (0 == x)p = &vmss_head;
		else p = &vmss_record;
		table.Clear();
		for (i = 0; i < (long)p->size(); ++i)
		{
			long line = table.AddLine();
			vector<pair<string, string> >::const_iterator it;
			long col = 0;
			for (it = (*p)[i].begin(); it != (*p)[i].end(); ++it)
			{
				table.SetData(line, col++, it->first);
				table.SetData(line, col++, it->second);
			}
		}
		thelog << endl << table.MakeTextTable() << endi;
	}
	return 0;
}

//IBM不支持
//int test_Sort(int argc, char ** argv)
//{
//	long count=5000*10000;
//	vector<long> v;
//
//	v.reserve(count);
//
//	long i,i_sort;
//	time_t t1;
//
//	for (i_sort = 0; i_sort < 2; ++i_sort)
//	{
//		thelog << "填充初始数据。。。" << endi;
//		v.clear();
//		thelog << v.capacity() << endi;
//		srand(0);
//		for (i = 0; i < count; ++i)
//		{
//			v.push_back(rand());
//		}
//		thelog << "填充初始数据完成" << endi;
//		t1 = time(NULL);
//		if (0 == i_sort)
//		{
//			sort(v.begin(), v.end());
//		}
//		else if (1 == i_sort)
//		{
//			make_heap(v.begin(), v.end());
//			for (i = count - 1; i > 0; --i)
//			{
//				pop_heap(&v[0], &v[i + 1]);
//			}
//		}
//		thelog << "sort完成 " << time(NULL) - t1 << endi;
//		long last = v[0];
//		bool isSorted = true;
//		for (i = 1; i < count; ++i)
//		{
//			if (0 == i_sort)
//			{
//				if (v[i] < last)
//				{
//					isSorted = false;
//					break;
//				}
//			}
//			else if(1==i_sort)
//			{
//				if (v[i] < last)
//				{
//					isSorted = false;
//					break;
//				}
//			}
//			last = v[i];
//		}
//		thelog << (isSorted ? "有序" : "无序") << endi;
//	}
//	set<long> s;
//	srand(0);
//	t1 = time(NULL);
//	for (i = 0; i < count; ++i)
//	{
//		s.insert(rand());
//	}
//	thelog << "insert完成 " << time(NULL) - t1 << endi;
//	return 0;
//}
int test_ShmMultiMap(int argc, char** argv)
{
	T_SHM_LIST_SET<CDemoData, 1, CDemoData, 2> datas("multimap", "multimap2", 0);

	if (!datas.Attach(false))
	{
		if (!datas.CreateShm())return __LINE__;
		if (!datas.Attach(false))return __LINE__;
	}
	datas.Report();

	T_SHM_LIST_SET<CDemoData, 1, CDemoData, 2>::iterator it;
	CDemoData tmp;
	tmp.n = time(NULL) % 2;
	it = datas.find(tmp);
	thelog << (it != datas.end() ? "找到" : "没找到") << endi;
	datas.insert(tmp);
	datas.ReportData();
	return 0;
}
int test_CMyRWMutex(int argc, char** argv)
{
	CMyRWMutex m;
	if (!m.Create())return __LINE__;
	thelog << m.Report() << endi;
	pid_t pid = fork();
	if (pid < 0)
	{
		thelog << "fork出错" << ende;
	}
	else if (0 == pid)
	{
		//子进程
		theLog.SetSource("子进程");
		thelog << m.Report() << endi;
		if (!m.WLock())
		{
			thelog << "锁定失败" << ende;
		}
		thelog << "锁定成功" << endi;
		thelog << m.Report() << endi;
		SleepSeconds(10);
		thelog << "未锁定退出" << endi;
		exit(0);
	}
	else
	{
		thelog << m.Report() << endi;
		SleepSeconds(5);
		thelog << "等待锁定" << endi;
		if (!m.WLock())
		{
			thelog << "锁定失败" << ende;
		}
		thelog << "锁定成功" << endi;
		thelog << m.Report() << endi;
		m.WUnLock();
		thelog << m.Report() << endi;
	}
	if (!m.Destory())return __LINE__;
	return 0;
}
class A
{
public:
	string s;
	string& toString(string& ret)const
	{
		return ret = s;
	}
};
int test(int argc, char** argv)
{
	CCommandSet commandset;
	bool ret;
	do
	{
		if (!CShmEnvMgr::AddToCommandSet(commandset))
		{
			ret = __LINE__;
			break;
		}
		if (!CEnvTools::AddToCommandSet(commandset))
		{
			ret = __LINE__;
			break;
		}

		ret = commandset.doCommandSet("201");
	} while (false);

	if (ret)return 0;
	else return 1;
}

int test_RebuildSet(int argc, char** argv)
{
	T_SHMSET_NO_MUTEX< CDemoData, 1> datas("tmp");
	CDemoData data;
	time_t t0 = time(NULL);
	long t1;
	long t2;
	long count = 1000 * 10000;

	srand(0);
	if (!datas.FastRebuild_Start())return __LINE__;
	for (long i = 0; i < count; ++i)
	{
		data.n = rand();
		if (!datas.FastRebuild_PushData(data))return __LINE__;
	}
	if (!datas.FastRebuild_Finish(NULL, true))return __LINE__;
	t1 = time(NULL) - t0;

	t0 = time(NULL);
	srand(0);
	if (!datas.FastRebuild_Start())return __LINE__;
	for (long i = 0; i < count; ++i)
	{
		data.n = rand();
		datas.insert(data);
	}
	datas.Report();
	t2 = time(NULL) - t0;

	thelog << "快速重建用时 " << t1 << " 秒，insert用时 " << t2 << " 秒" << endi;

	datas.RunCmdUI();

	return 0;
}
//int main_fun(int argc,char ** argv)
int main(int argc, char** argv)
{
	if (!InitActiveApp("shmfc", 1024 * 1024 * 10, argc, argv))exit(1);

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
	if (sizeof(long long) != 8)
	{
		thelog << "非64位程序！" << ende;
		return 1;
	}

	{
		string str;
		CMeta meta;
		int a[5] = { 24,68,998,999,1000 };
		meta.Set(GUID_T_ARRAY, a, 5);
		meta.toString(str);
		thelog << endl << str << endi;
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

	myBitSet<8> bm;
	bm = "1";
	thelog << bm.to_string() << endi;

	if (!CShmEnv::getInstPtr()->isConnected() && !CShmEnv::getInstPtr()->ShmEnvConnect())
	{
		thelog << "未能连接到主共享内存，请检查信息（初次运行请用命令 0 - 1 创建）" << ende;
		UIInput("Press any key to continue", "");
	}

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
				<< "0 管理" << endl
				<< "1 test_BinaryPool" << endl
				<< "3 test_CMultiProcessServer" << endl
				<< "4 test_CMultiProcessServer_view" << endl
				<< "5 test_CMultiProcessServer_speed" << endl
				<< "6 test_CMultiProcessServer_direct" << endl
				<< "7 test_RebuildSet" << endl
				<< "12 UDP测试客户端" << endl
				<< "13 UDP测试服务端" << endl
				<< "14 CStressTesting" << endl
				<< "15 CTestCSimpleMultiProcess_mutex atomic" << endl
				<< "16 CTest_hash" << endl
				<< "17 T_SHM_HASH" << endl
				<< "20 T_ARRAY" << endl
				<< "21 T_SHMSET_NO_MUTEX" << endl
				<< "22 T_SHMSET（带互斥）" << endl
				<< "23 T_SHMSET_lower_bound（带互斥）" << endl
				<< "24 StringPool2x" << endl
				<< "25 T_MULTI_LIST" << endl
				<< "26 T_MULTI_LISTTREE" << endl
				<< "88 test_CMyRWMutex" << endl
				<< "89 test_T_SHM_SET_GROUP" << endl
				<< "90 test_shm_IActiveObject" << endl
				<< "91 test_ShmMultiMap" << endl
				//<< "95 test_Sort" << endl
				<< "99 test_ParseFromXml" << endl
				<< "........................................" << endl
				<< "----------------------------------------" << endi;
			cmd = UIInput("请选择命令：", "26");
			if (cmd == "q")break;
		}
		long nCmd = atol(cmd.c_str());
		switch (nCmd)
		{
		case 0:
			ret = test(argc, argv);
			break;
		case 1:
			ret = test_BinaryPool(argc, argv);
			break;
		case 3:
			ret = test_CMultiProcessServer(argc, argv);
			break;
		case 4:
			ret = test_CMultiProcessServer_view(argc, argv);
			break;
		case 5:
			ret = test_CMultiProcessServer_speed(argc, argv);
			break;
		case 6:
			ret = test_CMultiProcessServer_direct(argc, argv);
			break;
		case 7:
			ret = test_RebuildSet(argc, argv);
			break;
		case 12:
			ret = CUDPServer::testClient(argc, argv);
			break;
		case 13:
			ret = CUDPServer::testServer(argc, argv);
			break;
		case 14:
			ret = CTestCStressTesting::doTest(argc, argv);
			break;
		case 15:
			ret = CTestCSimpleMultiProcess_mutex::doTest(argc, argv);
			break;
		case 16:
			ret = CTest_hash::doTest(argc, argv);
			break;
		case 17:
			ret = T_SHM_HASH<CDemoData, PI_TEST_1, PI_TEST_2>::T_SHM_HASH_test(argc, argv, "test", 0);
			break;
		case 20:
			ret = CTestT_ARRAY::test_T_ARRAY(argc, argv);
			break;
		case 21:
			ret = CTest_T_SHMSET_NO_MUTEX::test_T_SHMSET_NO_MUTEX(argc, argv);
			break;
		case 22:
			ret = CTest_T_SHMSET::test_T_SHMSET(argc, argv);
			break;
		case 23:
			ret = CTest_T_SHMSET_lower_bound::test_T_SHMSET_lower_bound(argc, argv);
			break;
		case 24:
			ret = CTest_StringPool2x::test_StringPool2x(argc, argv);
			break;
		case 25:
			ret = CTest_T_MULTI_LIST::test_T_MULTI_LIST(argc, argv);
			break;
		case 26:
			ret = CTest_T_MULTI_LISTTREE::test_T_MULTI_LISTTREE(argc, argv);
			break;
		case 88:
			ret = test_CMyRWMutex(argc, argv);
			break;
		case 89:
			ret = CTestT_SHM_SET_GROUP::test(argc, argv);
			break;
		case 90:
			ret = CTestShmIActiveObject::test(argc, argv);
			break;
		case 91:
			ret = test_ShmMultiMap(argc, argv);
			break;
		//case 95:
		//	ret = test_Sort(argc, argv);
		//	break;
		case 99:
			ret = test_ParseFromXml(argc, argv);
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
