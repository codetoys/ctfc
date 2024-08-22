//shmEnv.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../function/config.h"
#include "../function/meta.h"
#include "../function/mymutex.h"
#include "../function/myshm.h"
#include "../function/htmldoc.h"
#include "../env/env.h"
using namespace ns_my_std;

namespace ns_my_std
{
	//当前的共享内存环境，仅可在连接时改变，并且不能嵌套
	extern string g_ShmEnvRoot;

	//共享内存系统
	struct shm_sys
	{
		long sys;
		long sys2;

		set<string > nosys;//不区分sys的共享内存的名字

		shm_sys() :sys(-1), sys2(-1) {}
	};
	extern shm_sys g_sys;//共享内存系统，0为默认

	//获取共享内存系统名称，有些名字区别共享内存系统，有些名字不区分
	string GetShmSysOfName(char const* name);

	struct shm_private_data
	{
		void* pLRU;//指向LRU对象
		void* pSET;//指向带有互斥的T_SHM_SET
		void* pVMAP;//指向句柄映射表（指向共享内存）
		long addr_map_size;//已经连接的共享内存块个数
		pair<int, char* > shm_addr_map[T_ARRAY_VMAP_MAX_SIZE];//共享内存块对应的连接地址
		CPThreadMutex thread_mutex;
		shm_private_data()
		{
			clearShmPrivateData();
			pLRU = NULL;
			pSET = NULL;
			thread_mutex.init();
		}
		void clearShmPrivateData()
		{
			pVMAP = NULL;
			addr_map_size = 0;

			//下面这些是私有对象地址，不能清理
			//pLRU=NULL;
			//pSET=NULL;
		}
		bool AddShmMap(int _shm_id, char* _addr)
		{
			if (addr_map_size + 1 >= T_ARRAY_VMAP_MAX_SIZE)return false;
			shm_addr_map[addr_map_size].first = _shm_id;
			shm_addr_map[addr_map_size].second = _addr;
			++addr_map_size;

			return true;
		}
	};
	extern shm_private_data g_shm_private_data_s[MAX_PP];//用于共享内存模板

	extern bool g_isMakeShmPoolData;//是否正在进行共享内存池数据构造，非构造时的内存申请在私有内存进行
	extern bool g_isShmPoolTrance;//是否跟踪共享内存池

	typedef long T_SHM_SIZE;//共享内存相关大小类型
	signed char const GUID_T_ARRAY[] = "8AA20E2E-4891-4df1-A87E-E5A23CB8D076";//全球唯一的标识，作为元数据的一部分识别物理结构

	//信号量信息
	struct sem_reg
	{
		sstring<32> sys;
		sstring<32> name;
		long part;

		CMyShmMutex::mySEM semid;

		sem_reg() :part(0)
		{
			semid.init();
		}
	};
	struct sem_reg_info
	{
		sstring<32> sys;
		sstring<32> name;
		long part;

		CMyShmMutex::mySEM * pSemID;

		sem_reg_info(sem_reg const& tmp)
		{
			sys = tmp.sys;
			name = tmp.name;
			part = tmp.part;
			pSemID = const_cast<CMyShmMutex::mySEM*>(&tmp.semid);
		}
	};
	class CShmEnv_Impl;
	//主共享内存块，存储共享内存和信号灯的注册信息，shm_id登记在$(EFC_ETC_ROOT)/.active_shmid.txt
	class CShmEnv
	{
		DECLARE_SINGLETON(CShmEnv)
	private:
		CShmEnv_Impl * m_pImpl;
	public:
		CShmEnv();
		~CShmEnv();
		bool ShmEnvCreate();
		size_t GetShmEnvSize()const;
		//root为NULL则使用默认（从环境变量获取）
		bool ShmEnvConnect(char const * root = NULL);
		bool Disconect();
		bool isConnected()const;
		bool DestoryShm();
		bool Report()const;
		bool ReportShmEnv(PSHM p)const;
		bool MakeShmTable(CHtmlDoc::CHtmlTable2 & htmltable)const;
		string HtmlReport()const;
		long size()const;
		long capacity()const;
		long byte_size()const;
		long byte_capacity()const;
	public://共享内存和信号量信息
		bool GetShmConfig(string const & sys, string const & name, long & r_size, bool silent = false);
		bool GetShmConfig(string const & sys, string const & name, string & str, bool silent = false);
		bool GetShmReg(string const & sys, string const & name, long part, int & _shmid, size_t & _segsz, time_t & _ctime);
		bool DeleteShmReg(string const & sys, string const & name, long part);
		bool SaveShmReg(string const & sys, string const & name, long part, int _shmid, long _segsz, time_t _ctime);
		
		bool GetAllSemInfo(vector<sem_reg_info > & datas);
		bool GetSemReg(string const& sys, string const& name, long part, CMyShmMutex::mySEM*& _semid, bool newIfNotExist);
		bool DeleteSemReg(string const & sys, string const & name, long part);
	public://系统参数
		bool SystemParam_SaveAs();
		bool SystemParam_Clear();
		bool SystemParam_Load();
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
			return _SystemParam_Set(isOrigin,section, name, buf);
		}
		bool SystemParam_SetOrigin(char const * section, char const * name, char const * value)
		{
			return _SystemParam_Set(true, section, name, value);
		}
		bool SystemParam_SetCurrent(char const * section, char const * name, char const * value)
		{
			return _SystemParam_Set(false, section, name, value);
		}
		bool _SystemParam_Set(bool isOrigin, char const * section, char const * name, char const * value);
		bool SystemParam_Delete(char const * section, char const * name)const;
		bool SystemParam_Get(char const * section, char const * name, char const *& value)const;
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
		bool SystemRuntime_Get(char const* model, char const* name, long*& pValue, long defaultvalue)
		{
			return _SystemRuntime_Get(model, name, pValue, defaultvalue);
		}
		bool _SystemRuntime_Get(char const* model, char const* name, long*& pValue, long defaultvalue)const;
		long SystemRuntime_GetLong(char const * model, char const * name, long defaultvalue)const
		{
			long * p;
			if (!_SystemRuntime_Get(model, name, p, defaultvalue))
			{
				DEBUG_LOG << model << " " << name << " 不存在" << endi;
				return defaultvalue;
			}
			return *p;
		}
		bool SystemRuntime_SetLong(char const * model, char const * name, long value)
		{
			long * p;
			if (!_SystemRuntime_Get(model, name, p, value))
			{
				DEBUG_LOG << model << " " << name << " 不存在" << endi;
				return false;
			}
			*p = value;
			return true;
		}
	public://系统统计信息
		//获得统计信息的位置
		bool SystemStatistics_Get(char const * model, char const * name, long *& pValue);
	};

	//共享内存注册信息
	struct ShmRegInfo
	{
	public:
		string sys;
		string name;
		long PART;

		//配置信息
		long r_size;

		//注册信息
		int shmid;
		size_t segsz;
		time_t ctime;

		ShmRegInfo(char const * _sys, char const * _name, long _part) :sys(_sys), name(_name), PART(_part) {}
		ShmRegInfo(string const & _sys, char const * _name, long _part) :sys(_sys), name(_name), PART(_part) {}
		bool _GetCinfigFromSHM()
		{
			if (!CShmEnv::getInstPtr()->GetShmConfig(sys, name, r_size))
			{
				r_size = 0;
				return false;
			}
			return true;
		}
		bool GetCinfigFromDb()
		{
			return _GetCinfigFromSHM();
		}
		bool _CompareShmRegInfo(ShmRegInfo const & tmp)const
		{
			bool ret = true;
			char buf[1024];
			if (segsz != tmp.segsz)
			{
				sprintf(buf, "物理共享内存尺寸(%ld)与数据库注册尺寸(%ld)不符合", (long)tmp.segsz, (long)segsz);
				thelog << buf << ende;
				ret = false;
			}
			if (ctime != tmp.ctime)
			{
				string strtime1 = asctime(localtime(&tmp.ctime));
				string strtime2 = asctime(localtime(&ctime));
				sprintf(buf, "物理共享内存创建时间(%ld %s)与数据库注册时间(%ld %s)不符合", (long)tmp.ctime, strtime1.c_str(), (long)ctime, strtime2.c_str());
				thelog << buf << ende;
				ret = false;
			}
			return ret;
		}
		//获取注册信息
		bool _GetRegFromSHM()
		{
			if (!CShmEnv::getInstPtr()->GetShmReg(sys, name, PART, shmid, segsz, ctime))
			{
				return false;
			}
			return true;
		}
		bool GetRegFromDb()
		{
			//if (!GetCinfigFromDb())return false;
			if (!_GetRegFromSHM())return false;
			//检查与实际共享内存是否一致
			ShmRegInfo tmpreg(sys.c_str(), name.c_str(), PART);
			if (!CShmMan::GetState(shmid, tmpreg.segsz, tmpreg.ctime))
			{
				thelog << "共享内存 " << sys << " " << name << " " << PART << " 不存在，可能的原因：主机重启或手工删除" << ende;
				return false;
			}
			if (!_CompareShmRegInfo(tmpreg))return false;
			return true;
		}
		//保存注册信息
		bool _SaveRegToSHM()
		{
			if (!CShmEnv::getInstPtr()->SaveShmReg(sys, name, PART, shmid, segsz, ctime))
			{
				return false;
			}
			return true;
		}
		bool SaveRegToDb()
		{
			//if (!GetCinfigFromDb())return false;
			if (!_SaveRegToSHM())return false;
			return true;
		}
		//删除注册信息
		bool _DeleteRegFromSHM()
		{
			if (!CShmEnv::getInstPtr()->DeleteShmReg(sys, name, PART))
			{
				return false;
			}
			return true;
		}
		bool DeleteRegFromDb()
		{
			if (!GetCinfigFromDb())return false;
			if (!_DeleteRegFromSHM())return false;
			return true;
		}
		bool GetConfigSize(long & _r_size)
		{
			if (!GetCinfigFromDb())return false;
			_r_size = r_size;
			return true;
		}
	};
	//受管理的互斥对象
	class CManagedMutex : public CMyShmMutex
	{
	private:
		string m_name;
		int m_part;
		bool __GetSemInfoFromSHM(CMyShmMutex::mySEM*& _sem_id)
		{
			if (!CShmEnv::getInstPtr()->GetSemReg(GetShmSysOfName(m_name.c_str()), m_name, m_part, _sem_id, true))
			{
				return false;
			}
			return true;
		}
		bool _GetSemInfo(CMyShmMutex::mySEM*& _sem_id)
		{
			if (!__GetSemInfoFromSHM(_sem_id))return false;
			return true;
		}
	public:
		CManagedMutex(char const * name = "", int part = 0) :m_name(name), m_part(part) {}
		bool AttachOrCreateMutexIfNeed(char const * name, int part = 0)
		{
			m_name = name;
			m_part = part;
			return AttachOrCreateMutexIfNeed();
		}
		bool AttachOrCreateMutexIfNeed()
		{
			if (0 == m_name.size())
			{
				thelog << "未设置信号灯名称" << ende;
				return false;
			}
			if (!CShmEnv::getInstPtr()->isConnected())
			{
				thelog << "主共享内存尚未连接" << ende;
				return false;
			}

			CMyShmMutex::mySEM* _sem_id{nullptr};
			if (!_GetSemInfo(_sem_id) || !Attach(_sem_id))
			{
				DEBUG_LOG << m_name << " " << m_part << " 信号灯信息不存在或已过期，重新创建信号灯" << endi;
				if (!Create(_sem_id))
				{
					thelog << m_name << " " << m_part << "创建信号灯出错" << ende;
					return false;
				}
			}
			return true;
		}
		static bool GetAllSemInfo(vector<sem_reg_info > & ret)
		{
			return CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->GetAllSemInfo(ret);
		}
	};
	//全局互斥对象
	class CGlobalMutex
	{
	private:
		CManagedMutex mutex;
	public:
		CGlobalMutex() :mutex(GLOBAL_MUTEX_NAME)
		{
			mutex.AttachOrCreateMutexIfNeed();
		}
		bool CGlobalMutex_Lock()
		{
			thelog << "CGlobalMutex_Lock" << endi;
			return  mutex.WLock();
		}
		bool CGlobalMutex_UnLock()
		{
			thelog << "CGlobalMutex_UnLock" << endi;
			return  mutex.WUnLock();
		}
	};
}
