//shm_Env.cpp
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "shmEnv.h"

namespace ns_my_std
{
	string g_ShmEnvRoot;
	shm_sys g_sys;
	shm_private_data g_shm_private_data_s[MAX_PP];
	bool g_isMakeShmPoolData = false;
	bool g_isShmPoolTrance = false;

	string GetShmSysOfName(char const* name)
	{
		if (0 == strcmp(name, GLOBAL_MUTEX_NAME))return "GLOBAL";

		if (g_sys.nosys.end() != g_sys.nosys.find(name))return "GLOBAL";

		if (g_sys.sys < 0 && g_sys.sys2 < 0)return "default";
		else
		{
			char buf[256];
			if (g_sys.sys2 < 0)sprintf(buf, "sys_%ld", g_sys.sys);
			else sprintf(buf, "sys_%ld_sys2_%ld", g_sys.sys, g_sys.sys2);
			return buf;
		}
	}

	class CShmEnv_Impl
	{
	private:
		struct shm_reg
		{
			sstring<32> sys;
			sstring<64> name;
			long part;

			int shmid;
			long segsz;
			time_t ctime;

			shm_reg() :part(0), shmid(-1), segsz(0), ctime(0)
			{
			}
		};
		struct param_record
		{
			sstring<32> section_name;
			sstring<64> param_name;
			sstring<64> param_origin_value;//来自配置文件的原始值
			sstring<64> param_current_value;//运行过程中设置的值

			void SetOrigin(char const * v) { param_origin_value = v; }
			void SetCurrent(char const * v) { param_current_value = v; }
			void Set(bool isOrigin, char const * v)
			{
				if (isOrigin) SetOrigin(v);
				else SetCurrent(v);
			}
			char const * Get()const
			{
				if ('\0' != param_current_value[0])return param_current_value.c_str();
				else return param_origin_value.c_str();
			}
		};
		struct runtime_record
		{
			sstring<32> model_name;//模块名
			sstring<64> runtime_name;//运行时信息名
			long runtime_value;//值
		};
		struct statistics_record
		{
			sstring<32> model_name;//模块名
			sstring<64> statistics_name;//统计信息名
			long statistics_value;//值
		};
		enum { SHM_REG_SIZE = 10240, SHM_PARAM_CAPACITY = 1024, SHM_RUNTIME_CAPACITY = 1025, SHM_STATISTICS_CAPACITY = 1026 };
	public:
		struct KeyReg
		{
			CMeta m_meta;
			int m_mutex_id;//内置互斥
			time_t m_mutex_ctime;//内置互斥
			shm_reg m_shm_reg[SHM_REG_SIZE];
			sem_reg m_sem_reg[SHM_REG_SIZE];
			param_record m_params[SHM_PARAM_CAPACITY];//参数
			runtime_record m_runtimes[SHM_RUNTIME_CAPACITY];
			statistics_record m_statisticses[SHM_STATISTICS_CAPACITY];

			KeyReg()
			{
				m_mutex_id = -1;
				m_mutex_ctime = 0;
				memset(m_params, 0, sizeof(param_record)*SHM_PARAM_CAPACITY);
				memset(m_runtimes, 0, sizeof(runtime_record)*SHM_RUNTIME_CAPACITY);
				memset(m_statisticses, 0, sizeof(statistics_record)*SHM_STATISTICS_CAPACITY);
			}
			bool Report()const
			{
				thelog << endl << _Report(true) << endi;
				return true;
			}
			string HtmlReport()const
			{
				return _Report(false);
			}
			bool MakeShmTable(CHtmlDoc::CHtmlTable2 & htmltable)const
			{
				htmltable.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("sys");
				htmltable.AddCol("name");
				htmltable.AddCol("part", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("shmid", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("segsz", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("ctime", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("ctime", CHtmlDoc::CHtmlDoc_DATACLASS_TIME);

				shm_reg const * pReg;
				long count = 0;
				for (long i = 0; i < SHM_REG_SIZE; ++i)
				{
					pReg = &m_shm_reg[i];
					//thelog<<i<<" "<<pReg->sys.c_str()<<endi;
					if (0 == pReg->sys.size())continue;

					htmltable.AddLine();
					htmltable.AddData(count);
					htmltable.AddData(pReg->sys.c_str());
					htmltable.AddData(pReg->name.c_str());
					htmltable.AddData(pReg->part);
					htmltable.AddData(pReg->shmid);
					htmltable.AddData(pReg->segsz, true);
					htmltable.AddData(pReg->ctime);
					htmltable.AddData(pReg->ctime);
					//theLog<<count<<" "<<pReg->sys.c_str()<<" "<<pReg->name.c_str()<<" "<<pReg->part
					//	<<" shmid="<<pReg->shmid<<" segsz="<<pReg->segsz<<" ctime="<<pReg->ctime<<endl;
					++count;
				}
				return true;
			}
			string _Report(bool isText)const
			{
				stringstream ss;
				char const hr[] = "\n-------------------------------------------\n";
				long i;
				long count;
				long line;
				ss << "sem id " << m_mutex_id << " ctime " << CMyTools::TimeToString_Time(m_mutex_ctime) << "\n";
				if (true)
				{
					CHtmlDoc::CHtmlTable2 htmltable;
					ss << hr << "共享内存信息：" << endl;
					MakeShmTable(htmltable);
					ss << (isText ? htmltable.MakeTextTable() : htmltable.MakeHtmlTable()) << endl << "共计 " << htmltable.GetRecordCount() << " 个" << endl;
				}
				if (true)
				{
					CHtmlDoc::CHtmlTable2 htmltable;
					htmltable.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("sys");
					htmltable.AddCol("name");
					htmltable.AddCol("part", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					//htmltable.AddCol("semid", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("ctime", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("ctime", CHtmlDoc::CHtmlDoc_DATACLASS_TIME);
					htmltable.AddCol("写锁", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("写计数", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("读计数", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("写等待", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

					sem_reg const * pReg;
					ss << hr << "信号量信息：" << endl;
					count = 0;
					for (i = 0; i < SHM_REG_SIZE; ++i)
					{
						pReg = &m_sem_reg[i];
						if (0 == pReg->sys.size())continue;

						line = htmltable.AddLine();
						long col_i = 0;
						htmltable.SetData(line, col_i++, count);
						htmltable.SetData(line, col_i++, pReg->sys.c_str());
						htmltable.SetData(line, col_i++, pReg->name.c_str());
						htmltable.SetData(line, col_i++, pReg->part);
						//htmltable.SetData(line, col_i++, pReg->semid);
						htmltable.SetData(line, col_i++, pReg->semid.ctime);
						htmltable.SetData(line, col_i++, pReg->semid.ctime);
						CMyShmMutex tmpmutex;
						if (tmpmutex.Attach(const_cast<CMyShmMutex::mySEM*>(&pReg->semid)))
						{
							long w, w_count, r_count, w_wait;
							if (tmpmutex.GetCount2(w, w_count, r_count, w_wait))
							{
								htmltable.AddData(w);
								htmltable.AddData(w_count);
								htmltable.AddData(r_count);
								htmltable.AddData(w_wait);
							}
							tmpmutex.Detach();
						}
						//theLog<<count<<" "<<pReg->sys.c_str()<<" "<<pReg->name.c_str()<<" "<<pReg->part
						//	<<" semid="<<pReg->semid<<" ctime="<<pReg->ctime<<endl;
						++count;
					}
					ss << (isText ? htmltable.MakeTextTable() : htmltable.MakeHtmlTable()) << endl << "共计 " << count << " 个" << endl;
				}
				if (true)
				{
					CHtmlDoc::CHtmlTable2 htmltable;
					htmltable.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("section");
					htmltable.AddCol("name");
					htmltable.AddCol("param");
					htmltable.AddCol("current");

					param_record const * pParam;
					ss << hr << "参数信息：" << endl;
					count = 0;
					for (i = 0; i < SHM_PARAM_CAPACITY; ++i)
					{
						pParam = &m_params[i];
						if (0 == pParam->param_name.size())continue;

						line = htmltable.AddLine();
						htmltable.AddData(i);
						htmltable.AddData(pParam->section_name.c_str());
						htmltable.AddData(pParam->param_name.c_str());
						htmltable.AddData(pParam->param_origin_value.c_str());
						htmltable.AddData(pParam->param_current_value.c_str());
						++count;
					}
					ss << (isText ? htmltable.MakeTextTable() : htmltable.MakeHtmlTable()) << endl << "共计 " << count << " 个" << endl;
				}
				if (true)
				{
					CHtmlDoc::CHtmlTable2 htmltable;
					htmltable.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("model");
					htmltable.AddCol("name");
					htmltable.AddCol("runtime", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

					runtime_record const * pRuntime;
					ss << hr << "运行时信息：" << endl;
					count = 0;
					for (i = 0; i < SHM_RUNTIME_CAPACITY; ++i)
					{
						pRuntime = &m_runtimes[i];
						if (0 == pRuntime->runtime_name.size())continue;

						line = htmltable.AddLine();
						htmltable.AddData(i);
						htmltable.AddData(pRuntime->model_name.c_str());
						htmltable.AddData(pRuntime->runtime_name.c_str());
						htmltable.AddData(pRuntime->runtime_value);
						++count;
					}
					ss << (isText ? htmltable.MakeTextTable() : htmltable.MakeHtmlTable()) << endl << "共计 " << count << " 个" << endl;
				}
				if (true)
				{
					CHtmlDoc::CHtmlTable2 htmltable;
					htmltable.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
					htmltable.AddCol("model");
					htmltable.AddCol("name");
					htmltable.AddCol("statistics", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

					statistics_record const * pStatistics;
					ss << hr << "统计信息：" << endl;
					count = 0;
					for (i = 0; i < SHM_STATISTICS_CAPACITY; ++i)
					{
						pStatistics = &m_statisticses[i];
						if (0 == pStatistics->statistics_name.size())continue;

						line = htmltable.AddLine();
						htmltable.AddData(i);
						htmltable.AddData(pStatistics->model_name.c_str());
						htmltable.AddData(pStatistics->statistics_name.c_str());
						htmltable.AddData(pStatistics->statistics_value);
						++count;
					}
					ss << (isText ? htmltable.MakeTextTable() : htmltable.MakeHtmlTable()) << endl << "共计 " << count << " 个" << endl;
				}
				return ss.str();
			}
		};
	private:
		KeyReg * m_pShm;//指向共享内存
		CMyRWMutex m_mutex;//内置互斥

		//主SHM_ID保存的文件名
		static string getMainShmIDFileName(char const * root)
		{
			char buf[2048];
			if (NULL != root)
			{
				string _root = root;
				ShellEnvReplace(_root);
				sprintf(buf, "%s/shmenv_shmid.runtime", _root.c_str());
			}
			else sprintf(buf, "%s/shmenv_shmid.runtime", getenv(MySTD_ENV));
			return buf;
		}
		//获得主SHM_ID
		static int getMainShmID(char const * root)
		{
			CEasyFile file;
			string str;
			if (!file.ReadFile(getMainShmIDFileName(root).c_str(), str))
			{
				cout << "读取文件出错 " << getMainShmIDFileName(root) << " " << file.getMsg() << endl;
				return -1;
			}
			return atol(str.c_str());
		}
		//设置元数据
		void makeMeta(CMeta & meta)
		{
			vector<int> intarray;
			intarray.push_back(0);//version
			intarray.push_back(sizeof(KeyReg));
			intarray.push_back(SHM_REG_SIZE);
			meta.Set(GUID_T_ARRAY, &intarray[0], intarray.size());

			//string str;
			//thelog << endl << meta.toString(str) << endi;
		}
		bool _Connect(char const * root)
		{
			if (NULL == m_pShm)
			{
				m_pShm = (KeyReg *)CShmMan::ConnectByID(getMainShmID(root), false);
				if (NULL == m_pShm)
				{
					thelog << "连接主共享内存失败" << ende;
					return false;
				}
				DEBUG_LOG << "连接主共享内存成功 shm_id= " << getMainShmID(root) << " size= " << sizeof(KeyReg) << endi;
			}
			return true;
		}
	public:
		CShmEnv_Impl() :m_pShm(NULL) {}
		bool ShmEnvLock()
		{
			if (!m_mutex.WLock())
			{
				thelog << "shmenv lock error" << ende;
				return false;
			}
			else
			{
				//thelog << "shmenv lock ok" << endi;
				return true;
			}
		}
		bool ShmEnvUnLock()
		{
			if (!m_mutex.WUnLock())
			{
				thelog << "shmenv unlock error" << ende;
				return false;
			}
			else
			{
				//thelog << "shmenv unlock ok" << endi;
				return true;
			}
		}
		bool ShmEnvCreate()
		{
			if (NULL != m_pShm)
			{
				thelog << "已经连接，不能创建。创建已忽略。" << endi;
				return true;
			}
			if (ShmEnvConnect())
			{
				thelog << "已经存在。创建已忽略。" << endi;
				return true;
			}

			int shm_id = CShmMan::CreatePrivateSHM(sizeof(KeyReg));
			if (shm_id < 0)
			{
				thelog << "创建主共享内存失败 shm_id= " << shm_id << " size= " << sizeof(KeyReg) << ende;
				return false;
			}
			thelog << "创建主共享内存成功 shm_id= " << shm_id << " size= " << sizeof(KeyReg) << endi;

			//保存到文件
			CEasyFile file;
			if (!file.WriteFile(getMainShmIDFileName(NULL).c_str(), shm_id))
			{
				thelog << "写文件失败 " << getMainShmIDFileName(NULL) << endl;
				CShmMan::Destory(shm_id);
				return false;
			}

			if (!_Connect(NULL))
			{
				thelog << "连接失败" << ende;
				return false;
			}
			new(m_pShm)KeyReg;

			//设置元数据
			makeMeta(m_pShm->m_meta);
			string str;
			thelog << endl << m_pShm->m_meta.toString(str) << endi;

			//创建内置互斥
			if (!m_mutex.Create())
			{
				thelog << "create mutex error" << ende;
				Disconect();
				return false;
			}
			m_pShm->m_mutex_id = m_mutex.GetID(m_pShm->m_mutex_ctime);
			thelog << "创建主共享内存成功 shm_id= " << shm_id << " size= " << sizeof(KeyReg) << endi;

			long * tmp;
			if (!this->_SystemRuntime_Get("测试", "test", tmp, 0))
			{
				return false;
			}
			*tmp = 1;
			if (!this->SystemStatistics_Get("测试s", "tests", tmp))
			{
				return false;
			}
			*tmp = 2;

			return true;
		}
		long GetShmEnvSize()const { return sizeof(KeyReg); }
		//root为NULL则使用默认（从环境变量获取）
		bool ShmEnvConnect(char const * root = NULL)
		{
			if (0 != g_ShmEnvRoot.size())
			{
				thelog << endl << "=============================================="
					<< endl << "正在连接到另外的共享内存环境 " << root
					<< endl << "==============================================" << endi;
			}
			if (!_Connect(root))
			{
				return false;
			}

			//检查元数据
			CMeta meta;
			string msg;
			makeMeta(meta);
			if (!meta.Compare(m_pShm->m_meta, msg))
			{
				thelog << "元数据不匹配" << endl << msg << ende;
				string str;
				thelog << endl << meta.toString(str) << endi;
				thelog << endl << m_pShm->m_meta.toString(str) << endi;

				Disconect();
				return false;
			}

			if (!m_mutex.Attach(m_pShm->m_mutex_id, m_pShm->m_mutex_ctime))
			{
				thelog << "attach mutex error" << ende;

				Disconect();
				return false;
			}

			return true;
		}
		bool Disconect()
		{
			if (0 != g_ShmEnvRoot.size())
			{
				thelog << endl << "=============================================="
					<< endl << "正在从另外的共享内存环境断开 " << g_ShmEnvRoot
					<< endl << "==============================================" << endi;
			}
			if (CShmMan::Disconnect((PSHM)m_pShm))
			{
				m_pShm = NULL;
				return true;
			}
			else
			{
				return false;
			}
		}
		bool isConnected()const { return NULL != m_pShm; }
		bool DestoryShm()
		{
			long i;
			sem_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_sem_reg[i];
				if (0 == pReg->sys.size())continue;

				//CMyRWMutex mutex;
				//if (mutex.Attach(pReg->semid, pReg->ctime))
				//{
				//	if (mutex.Destory())
				//	{
				//		thelog << "destory " << pReg->sys.c_str() << " " << pReg->name.c_str() << " " << pReg->semid << " ok" << endi;
				//	}
				//	else
				//	{
				//		thelog << "destory " << pReg->sys.c_str() << " " << pReg->name.c_str() << " " << pReg->semid << " error" << ende;
				//	}
				//}
				new(pReg)sem_reg;
			}
			return true;
		}
		bool Report()const
		{
			if (NULL == m_pShm)
			{
				thelog << "未连接" << ende;
				return false;
			}

			return m_pShm->Report();
		}
		bool ReportShmEnv(PSHM p)const
		{
			KeyReg * pKeyReg = (KeyReg*)p;

			return pKeyReg->Report();
		}
		bool MakeShmTable(CHtmlDoc::CHtmlTable2 & htmltable)const
		{
			if (NULL == m_pShm)
			{
				thelog << "未连接" << ende;
				return false;
			}

			return m_pShm->MakeShmTable(htmltable);
		}
		string HtmlReport()const
		{
			if (NULL == m_pShm)
			{
				return "未连接";
			}

			return m_pShm->HtmlReport();
		}
		long size()const
		{
			return SHM_REG_SIZE;
		}
		long capacity()const
		{
			return SHM_REG_SIZE;
		}
		long byte_size()const
		{
			return sizeof(KeyReg);
		}
		long byte_capacity()const
		{
			return sizeof(KeyReg);
		}
	public://共享内存和信号量信息
		bool GetShmConfig(string const & sys, string const & name, long & r_size, bool silent = false)
		{
			string str;
			if (GetShmConfig(sys, name, str, silent))
			{
				r_size = atol(str.c_str());
				return true;
			}
			else
			{
				return false;
			}
		}
		bool GetShmConfig(string const & sys, string const & name, string & str, bool silent = false)
		{
			string tmp;//参数名，有多种选择，可以忽略sys，但是最严格的优先，这可能导致意外的匹配
			StringTokenizer st(name, ".", false);
			tmp = "";
			for (size_t i = 0; i < st.size(); ++i)
			{
				if (tmp.size() != 0)tmp += ".";
				tmp += st[i];
				if (SystemParam_Get("SHM_CONFIG", tmp.c_str(), str))
				{
					DEBUG_LOG << "发现配置 " << tmp << " " << str << endi;
				}
			}
			tmp = sys;
			for (size_t i = 0; i < st.size(); ++i)
			{
				tmp += "." + st[i];
				if (SystemParam_Get("SHM_CONFIG", tmp.c_str(), str))
				{
					DEBUG_LOG << "发现配置 " << tmp << " " << str << endi;
				}
			}
			if (0 == str.size())
			{
				if (!silent)thelog << "未配置 " << "SHM_CONFIG" << " " << tmp << endi;
				return false;
			}
			return true;
		}
		bool GetShmReg(string const & sys, string const & name, long part, int & _shmid, size_t & _segsz, time_t & _ctime)
		{
			if (NULL == m_pShm && !ShmEnvConnect())return false;
			long i;
			shm_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_shm_reg[i];
				//thelog<<i<<" "<<pReg->sys.c_str()<<endi;
				if (sys == pReg->sys.c_str() && name == pReg->name.c_str() && part == pReg->part)
				{
					_shmid = pReg->shmid;
					_segsz = pReg->segsz;
					_ctime = pReg->ctime;
					return true;
				}
			}
			DEBUG_LOG << "共享内存注册信息不存在 " << sys << " " << name << " " << part << endi;
			return false;
		}
		bool DeleteShmReg(string const & sys, string const & name, long part)
		{
			if (NULL == m_pShm && !ShmEnvConnect())return false;
			long i;
			shm_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_shm_reg[i];
				if (sys == pReg->sys.c_str() && name == pReg->name.c_str() && part == pReg->part)
				{
					new(pReg)shm_reg;
				}
			}
			return true;
		}
		bool SaveShmReg(string const & sys, string const & name, long part, int _shmid, long _segsz, time_t _ctime)
		{
			if (NULL == m_pShm && !ShmEnvConnect())return false;
			DeleteShmReg(sys, name, part);

			long i;
			shm_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_shm_reg[i];
				if (0 == pReg->sys.size() && 0 == pReg->name.size())
				{
					pReg->sys = sys;
					pReg->name = name;
					pReg->part = part;
					pReg->shmid = _shmid;
					pReg->segsz = _segsz;
					pReg->ctime = _ctime;
					return true;
				}
			}
			thelog << "没有位置供存储shm注册信息 " << sys << " " << name << " " << part << ende;
			return false;
		}
		bool GetAllSemInfo(vector<sem_reg_info > & ret)
		{
			ret.clear();
			if (NULL == m_pShm && !ShmEnvConnect())return false;
			long i;
			sem_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_sem_reg[i];
				if (0 == pReg->sys.size())continue;
				ret.push_back(*pReg);
			}
			DEBUG_LOG << "获得信号量 " << ret.size() << " 个" << endi;
			return true;
		}
		bool GetSemReg(string const& sys, string const& name, long part, CMyShmMutex::mySEM*& _semid, bool newIfNotExist)
		{
			if (NULL == m_pShm && !ShmEnvConnect())return false;
			long i;
			sem_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_sem_reg[i];
				if (sys == pReg->sys.c_str() && name == pReg->name.c_str() && part == pReg->part)
				{
					_semid = &pReg->semid;
					//thelog<<"获得sem注册信息 "<<sys<<" "<<name<<" "<<part<<" id="<<_semid<<endi;
					return true;
				}
			}
			DEBUG_LOG << "没有找到sem注册信息 " << sys << " " << name << " " << part << endi;
			if (newIfNotExist)
			{
				for (i = 0; i < SHM_REG_SIZE; ++i)
				{
					pReg = &m_pShm->m_sem_reg[i];
					if (0 == pReg->sys.size())
					{
						pReg->sys = sys;
						pReg->name = name;
						pReg->part = part;
						_semid = &pReg->semid;
						//thelog<<"获得sem注册信息 "<<sys<<" "<<name<<" "<<part<<" id="<<_semid<<endi;
						return true;
					}
				}
				DEBUG_LOG << "没有空闲位置 sem注册信息 " << sys << " " << name << " " << part << endi;
			}
			return false;
		}
		bool DeleteSemReg(string const & sys, string const & name, long part)
		{
			if (NULL == m_pShm && !ShmEnvConnect())return false;
			long i;
			sem_reg * pReg;
			for (i = 0; i < SHM_REG_SIZE; ++i)
			{
				pReg = &m_pShm->m_sem_reg[i];
				if (sys == pReg->sys.c_str() && name == pReg->name.c_str() && part == pReg->part)
				{
					new(pReg)sem_reg;
				}
			}
			return true;
		}
	public://系统参数
		bool SystemParam_SaveAs()
		{
			CIniFile::T_DATA_S datas;
			long i;
			for (i = 0; i < SHM_PARAM_CAPACITY; ++i)
			{
				if (0 == m_pShm->m_params[i].section_name.size())continue;
				datas[m_pShm->m_params[i].section_name.c_str()].push_back(pair<string, string>(m_pShm->m_params[i].param_name.c_str(), m_pShm->m_params[i].Get()));
			}
			CIniFile ini;
			return ini.SaveDefaultIni(&datas);
		}
		bool SystemParam_Clear()
		{
			memset(m_pShm->m_params, 0, sizeof(param_record)*SHM_PARAM_CAPACITY);
			return true;
		}
		bool SystemParam_Load()
		{
			vector<string > inis;
			inis.push_back("ctfc.ini");
			for (size_t i = 0; i < inis.size(); ++i)
			{
				CIniFile ini;
				if (!ini.LoadDefaultIni(inis[i].c_str()))
				{
					thelog << "加载配置文件出错" << inis[i] << ende;
					return false;
				}

				CIniFile::T_DATA_S const * pIniDatas = ini.GetAllData();
				CIniFile::T_DATA_S::const_iterator it;
				for (it = pIniDatas->begin(); it != pIniDatas->end(); ++it)
				{
					CIniFile::T_SECTION::const_iterator it2;
					for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
					{
						if (!SystemParam_SetOrigin(it->first.c_str(), it2->first.c_str(), it2->second.c_str()))
						{
							return false;
						}
					}
				}
			}
			return true;
		}
		bool SystemParam_SetOriginLong(char const * section, char const * name, long value)
		{
			return _SystemParam_SetLong(true, section, name, value);
		}
		bool SystemParam_SetCurrentLong(char const * section, char const * name, long value)
		{
			return _SystemParam_SetLong(false, section, name, value);
		}
		bool _SystemParam_SetLong(bool isOrigin, char const * section, char const * name, long value)
		{
			char buf[256];
			sprintf(buf, "%ld", value);
			return _SystemParam_Set(isOrigin, section, name, buf);
		}
		bool SystemParam_SetOrigin(char const * section, char const * name, char const * value)
		{
			return _SystemParam_Set(true, section, name, value);
		}
		bool SystemParam_SetCurrent(char const * section, char const * name, char const * value)
		{
			return _SystemParam_Set(false, section, name, value);
		}
		bool _SystemParam_Set(bool isOrigin, char const * section, char const * name, char const * value)
		{
			long empty = -1;//第一个空闲位置
			long i;
			for (i = 0; i < SHM_PARAM_CAPACITY; ++i)
			{
				if (m_pShm->m_params[i].section_name == section && m_pShm->m_params[i].param_name == name)
				{
					m_pShm->m_params[i].Set(isOrigin, value);
					return true;
				}
				if (0 == m_pShm->m_params[i].section_name.size())
				{
					if (-1 == empty)empty = i;
				}
			}
			if (-1 == empty)
			{
				thelog << "参数空间已满" << ende;
				return false;
			}
			m_pShm->m_params[empty].section_name = section;
			m_pShm->m_params[empty].param_name = name;
			m_pShm->m_params[empty].Set(isOrigin, value);
			return true;
		}
		bool SystemParam_Delete(char const * section, char const * name)const
		{
			long i;
			for (i = 0; i < SHM_PARAM_CAPACITY; ++i)
			{
				if (m_pShm->m_params[i].section_name == section && m_pShm->m_params[i].param_name == name)
				{
					m_pShm->m_params[i].section_name = "";
					m_pShm->m_params[i].param_name = "";
					m_pShm->m_params[i].param_origin_value = "";
					m_pShm->m_params[i].param_current_value = "";
					return true;
				}
			}
			return false;
		}
		bool SystemParam_Get(char const * section, char const * name, char const *& value)const
		{
			long i;
			for (i = 0; i < SHM_PARAM_CAPACITY; ++i)
			{
				if (m_pShm->m_params[i].section_name == section && m_pShm->m_params[i].param_name == name)
				{
					value = m_pShm->m_params[i].Get();
					return true;
				}
			}
			value = NULL;
			return false;
		}
		bool SystemParam_Get(char const * section, char const * name, string & value)const
		{
			char const * p;
			if (!SystemParam_Get(section, name, p))
			{
				return false;
			}
			value = p;
			return true;
		}
		long SystemParam_GetLong(char const * section, char const * name, long defaultvalue)const
		{
			char const * p;
			if (!SystemParam_Get(section, name, p))
			{
				DEBUG_LOG << section << " " << name << " 未配置" << endi;
				return defaultvalue;
			}
			return atol(p);
		}
		string SystemParam_GetString(char const * section, char const * name, string defaultvalue)const
		{
			char const * p;
			if (!SystemParam_Get(section, name, p))
			{
				DEBUG_LOG << section << " " << name << " 未配置" << endi;
				return defaultvalue;
			}
			return p;
		}
	public://系统运行时信息
		//获得运行时信息的位置
		bool _SystemRuntime_Get(char const * model, char const * name, long *& pValue, long defaultvalue)const
		{
			long i;
			long idel = -1;
			for (i = 0; i < SHM_RUNTIME_CAPACITY; ++i)
			{
				if (0 == m_pShm->m_runtimes[i].runtime_name.size())
				{
					if (idel < 0)idel = i;
					continue;
				}
				if (m_pShm->m_runtimes[i].model_name == model && m_pShm->m_runtimes[i].runtime_name == name)
				{
					pValue = &(m_pShm->m_runtimes[i].runtime_value);
					return true;
				}
			}
			if (idel < 0)
			{
				thelog << "运行时信息空间已满" << ende;
				return false;
			}
			m_pShm->m_runtimes[idel].model_name = model;
			m_pShm->m_runtimes[idel].runtime_name = name;
			m_pShm->m_runtimes[idel].runtime_value = defaultvalue;
			pValue = &(m_pShm->m_runtimes[idel].runtime_value);
			return true;
		}
	public://系统统计信息
		//获得统计信息的位置
		bool SystemStatistics_Get(char const * model, char const * name, long *& pValue)
		{
			long i;
			long idel = -1;
			for (i = 0; i < SHM_STATISTICS_CAPACITY; ++i)
			{
				if (0 == m_pShm->m_statisticses[i].statistics_name.size())
				{
					if (idel < 0)idel = i;
					continue;
				}
				if (m_pShm->m_statisticses[i].model_name == model && m_pShm->m_statisticses[i].statistics_name == name)
				{
					pValue = &(m_pShm->m_statisticses[i].statistics_value);
					return true;
				}
			}
			if (idel < 0)
			{
				thelog << "统计信息空间已满" << ende;
				return false;
			}
			m_pShm->m_statisticses[idel].model_name = model;
			m_pShm->m_statisticses[idel].statistics_name = name;
			m_pShm->m_statisticses[idel].statistics_value = 0;
			pValue = &(m_pShm->m_statisticses[idel].statistics_value);
			return true;
		}
	};

	CShmEnv::CShmEnv() :m_pImpl(NULL)
	{
		m_pImpl = new CShmEnv_Impl;
	}
	CShmEnv::~CShmEnv()
	{
		if (m_pImpl)
		{
			delete m_pImpl;
			m_pImpl = NULL;
		}
	}
	bool CShmEnv::ShmEnvCreate()
	{
		return m_pImpl->ShmEnvCreate();
	}
	size_t CShmEnv::GetShmEnvSize()const
	{
		return m_pImpl->GetShmEnvSize();
	}

	bool CShmEnv::ShmEnvConnect(char const * root)
	{
		return m_pImpl->ShmEnvConnect();
	}

	bool CShmEnv::Disconect()
	{
		return m_pImpl->Disconect();
	}

	bool CShmEnv::isConnected() const
	{
		return m_pImpl->isConnected();
	}

	bool CShmEnv::DestoryShm()
	{
		return m_pImpl->DestoryShm();
	}

	bool CShmEnv::Report() const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->Report();

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}
	bool CShmEnv::ReportShmEnv(PSHM p) const
	{
		return m_pImpl->ReportShmEnv(p);
	}

	bool CShmEnv::MakeShmTable(CHtmlDoc::CHtmlTable2 & htmltable) const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->MakeShmTable(htmltable);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	string CShmEnv::HtmlReport() const
	{
		if (!m_pImpl->ShmEnvLock())return "lock error";

		string ret = m_pImpl->HtmlReport();

		m_pImpl->ShmEnvUnLock();
		return ret;
	}

	long CShmEnv::size() const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		long ret = m_pImpl->size();

		m_pImpl->ShmEnvUnLock();
		return ret;
	}

	long CShmEnv::capacity() const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		long ret = m_pImpl->capacity();

		m_pImpl->ShmEnvUnLock();
		return ret;
	}

	long CShmEnv::byte_size() const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		long ret = m_pImpl->byte_size();

		m_pImpl->ShmEnvUnLock();
		return ret;
	}

	long CShmEnv::byte_capacity() const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		long ret = m_pImpl->byte_capacity();

		m_pImpl->ShmEnvUnLock();
		return ret;
	}

	bool CShmEnv::GetShmConfig(string const & sys, string const & name, long & r_size, bool silent)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->GetShmConfig(sys, name, r_size, silent);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::GetShmConfig(string const & sys, string const & name, string & str, bool silent)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->GetShmConfig(sys, name, str, silent);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::GetShmReg(string const & sys, string const & name, long part, int & _shmid, size_t & _segsz, time_t & _ctime)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->GetShmReg(sys, name, part, _shmid, _segsz, _ctime);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::DeleteShmReg(string const & sys, string const & name, long part)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->DeleteShmReg(sys, name, part);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SaveShmReg(string const & sys, string const & name, long part, int _shmid, long _segsz, time_t _ctime)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SaveShmReg(sys, name, part, _shmid, _segsz, _ctime);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::GetAllSemInfo(vector<sem_reg_info>& datas)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->GetAllSemInfo(datas);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::GetSemReg(string const & sys, string const & name, long part, CMyShmMutex::mySEM*& _semid, bool newIfNotExist)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->GetSemReg(sys, name, part, _semid, newIfNotExist);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::DeleteSemReg(string const & sys, string const & name, long part)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->DeleteSemReg(sys, name, part);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SystemParam_SaveAs()
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SystemParam_SaveAs();

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SystemParam_Clear()
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SystemParam_Clear();

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SystemParam_Load()
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SystemParam_Load();

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::_SystemParam_Set(bool isOrigin, char const * section, char const * name, char const * value)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->_SystemParam_Set(isOrigin, section, name, value);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SystemParam_Delete(char const * section, char const * name) const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SystemParam_Delete(section, name);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SystemParam_Get(char const * section, char const * name, char const *& value) const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SystemParam_Get(section, name, value);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::_SystemRuntime_Get(char const * model, char const * name, long *& pValue,long defaultvalue) const
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->_SystemRuntime_Get(model, name, pValue, defaultvalue);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

	bool CShmEnv::SystemStatistics_Get(char const * model, char const * name, long *& pValue)
	{
		if (!m_pImpl->ShmEnvLock())return false;

		bool ret = m_pImpl->SystemStatistics_Get(model, name, pValue);

		if (!m_pImpl->ShmEnvUnLock())return false;
		return ret;
	}

}
