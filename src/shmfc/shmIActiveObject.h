//shm_IActiveObject.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmstd.h"
#include <thread>
#include <string>
#include "IActiveObject_Outside.h"
using namespace std;

namespace ns_my_std
{
	//共享内存对象管理接口
	class IShmActiveObject : public IShmActiveObject_outside
	{
	public:
		string GetFullName()const
		{
			char buf[256];
			if (GetPart() > 0)
			{
				sprintf(buf, "%s part %02d", GetName(), GetPart());
				return buf;
			}
			else return GetName();
		}
		char * _size_to_str(long size, char * buf)const
		{
			long k = 1024;
			if (size >= k * k * k)
			{
				sprintf(buf, "%.1fG", (double)size / (double)(k * k * k));
			}
			else if (size >= k * k)
			{
				sprintf(buf, "%.1fM", (double)size / (double)(k * k));
			}
			else if (size >= k)
			{
				sprintf(buf, "%.1fK", (double)size / (double)(k));
			}
			else
			{
				sprintf(buf, "%ld ", size);
			}
			return buf;
		}
		string & perfix(long level, string & ret)const
		{
			ret = "";
			long i;
			for (i = 0; i < level; ++i)
			{
				ret += "    ";
			}
			return ret;
		}
		//通用数据，子类也用
		string m_process_type;
		long m_process_seconds;//操作所用的时间
		bool m_process_ret;//每次操作后的结果
		bool m_undefined;//操作未定义

		bool _ClearRet()
		{
			m_process_ret = false;
			m_process_seconds = 0;
			m_process_type = "";
			m_undefined = false;
			return true;
		}
		void ShowRet_AddColumns(CHtmlDoc::CHtmlTable2 & table)const
		{
			table.AddCol("SHM NAME");
			table.AddCol("PI", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

			table.AddCol("CSW");//连接/未连接 共享内存/私有内存 只读/可写
			table.AddCol("TOTAL", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("BLOCK", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("R_LENGTH", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("CAPACITY", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("CAPACITY", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("SIZE", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("SIZE", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("%", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

			table.AddCol("OPERATION");
			table.AddCol("DUAL", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("RESULT");
			table.AddCol("OPERATION");
			table.AddCol("DUAL", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			table.AddCol("RESULT");
		}
		bool _ShowRet_AddDatas(CHtmlDoc::CHtmlTable2 & table, bool all = false, long level = 0)const
		{
			string str;
			char buf[2048];

			if (0 == level)
			{
				str = GetFullName();
				str += " " + m_process_type;
				table.SetTitle(str.c_str());
			}
			table.AddLine();
			table.AddData(perfix(level, str) + GetFullName());
			table.AddData(GetPI());
			if (isConnected())
			{
				str = "C";
				if (isPrivateMem())str += "P";
				else str += "S";
				if (isReadOnly())str += "R";
				else str += "W";
				table.AddData(str);
				table.AddData(_size_to_str(byte_size(), buf));
				table.AddData(block_count());
				if (0 != record_length())
				{
					table.AddData(record_length());
					table.AddData(capacity());
					table.AddData(_size_to_str(capacity(), buf));
					table.AddData(size());
					table.AddData(_size_to_str(size(), buf));
					double percent = (size() * 100 / (0 == capacity() ? 1 : capacity()));
					sprintf(buf, "%.1f%%", percent);
					table.AddData(buf);
				}
				else
				{
					long i;
					for (i = 0; i < 6; ++i)table.AddData(".");
				}
			}
			else
			{
				table.AddData("---");
				long i;
				for (i = 0; i < 8; ++i)table.AddData(".");
			}

			if (GetChildsName(str))
			{//集合对象的结果后退三个位置
				table.AddData("");
				table.AddData("");
				table.AddData("");
			}
			if (0 != m_process_type.size())
			{
				table.AddData(m_process_type);
				table.AddData(m_process_seconds);
				if (m_undefined)
				{
					str = "---";
				}
				else if (m_process_ret)
				{
					str = "OK";
				}
				else
				{
					str = "ERROR";
				}
				table.AddData(str);
			}
			return true;
		}
		virtual string & ShowRet(string & ret, bool all = false)const
		{
			CHtmlDoc::CHtmlTable2 table;
			ShowRet_AddColumns(table);
			ShowRet_AddDatas(table, all);
			ret = table.MakeTextTable();
			return ret;
		}
		string RunCmdUI_GetCommondList()
		{
			return "1:创建 2:连接(只读) 3:连接（可写） 4:断开 5:禁用互斥 6:清除数据 7:创建私有 8:删除共享内存 9:显示 10:数据\n"
				"11:从数据库加载 12:保存到数据库\n"
				"21:从目录加载 22:保存到目录 23:导出为文本文件\n"
				"31:从目录加载到私有内存 32:销毁私有内存\n"
				"97:repair 98:check 99:ToDo 100:shell";
		}
		bool RunCmdUI_DoCommond(string const & cmd)
		{
			string str;
			time_t t1 = time(NULL);

			if ("1" == cmd) { if (UIInput("创建共享内存将失去现有数据，继续吗？(y/n)", "n") == "y")m_process_ret = CreateShm(); }
			else if ("2" == cmd) { if (UIInput("以只读方式连接，继续吗？(y/n)", "n") == "y")m_process_ret = Attach(true); }
			else if ("3" == cmd) { if (UIInput("以可写方式连接，继续吗？(y/n)", "n") == "y")m_process_ret = Attach(false); }
			else if ("4" == cmd) { if (UIInput("断开，继续吗？(y/n)", "n") == "y")m_process_ret = Detach(); }
			else if ("5" == cmd) { if (UIInput("禁用互斥，继续吗？(y/n)", "n") == "y")m_process_ret = disableMutex(); }
			else if ("6" == cmd) { if (UIInput("清除数据，继续吗？(y/n)", "n") == "y")m_process_ret = clear(); }
			else if ("7" == cmd) { if (UIInput("创建私有内存，继续吗？(y/n)", "n") == "y")m_process_ret = CreatePrivate(); }
			else if ("8" == cmd) { if (UIInput("删除共享内存，继续吗？(y/n)", "n") == "y")m_process_ret = DestoryShm(); }
			else if ("9" == cmd) { m_process_ret = Report(); }
			else if ("10" == cmd) { m_process_ret = ReportData(); }
			else if ("11" == cmd) { if (UIInput("从数据库加载可能失去现有数据（取决于实际加载方式），继续吗？(y/n)", "n") == "y")m_process_ret = LoadFromDB(); }
			else if ("12" == cmd) { if (UIInput("保存到数据库可能失去现有数据库数据（取决于实际保存方式），继续吗？(y/n)", "n") == "y")m_process_ret = SaveToDB(); }
			else if ("21" == cmd) { if (UIInput("从目录加载将失去现有数据，继续吗？(y/n)", "n") == "y")m_process_ret = LoadFromDir(""); }
			else if ("22" == cmd) { if (UIInput("保存到目录将失去目录下现有数据，继续吗？(y/n)", "n") == "y")m_process_ret = SaveToDir(""); }
			else if ("23" == cmd) { if (UIInput("导出为文本将失去目录下现有数据，继续吗？(y/n)", "n") == "y")m_process_ret = ExportTextToDir(""); }
			else if ("31" == cmd) { if (UIInput("从目录加载到私有内存，继续吗？(y/n)", "n") == "y")m_process_ret = LoadPrivateFromDir(""); }
			else if ("32" == cmd) { if (UIInput("销毁私有内存数据，继续吗？(y/n)", "n") == "y")m_process_ret = DestoryPrivate(); }
			else if ("97" == cmd) { if (UIInput("修复数据，可能需要大量时间，继续吗？(y/n)", "n") == "y")m_process_ret = repair("data") && repair("index"); }
			else if ("98" == cmd) { if (UIInput("检查数据，可能需要大量时间，继续吗？(y/n)", "n") == "y")m_process_ret = check(); }
			else if ("99" == cmd)
			{
				str = UIInput("请输入ToDo操作", "");
				if (str.size() != 0)
				{
					m_process_ret = ToDo(str.c_str());
				}
			}
			else if ("100" == cmd)
			{
				str = UIInput("请输入shell命令", "");
				if (str.size() != 0)myShellExecute(str.c_str(), "");
			}
			else
			{
				return false;
			}

			m_process_type = cmd;
			m_process_seconds = time(NULL) - t1;
			return true;
		}
	private:
		bool _undefined()const
		{
			((IShmActiveObject *)this)->m_undefined = true;
			return false;
		}
	public:
		virtual char const * GetName()const = 0;
		virtual bool GetChildsName(string & ret)const { return false; }//获得子项名称列表
		virtual int GetPart()const { return 0; };//获得共享内存的模板参数PART
		virtual int GetPI()const { return 0; };//获得共享内存的模板参数PI
		virtual bool disableMutex()const
		{
			thelog << GetFullName() << " IShmActiveObject::disableMutex 未实现" << ende;
			return false;
		}
		virtual bool isPrivateMem()const { return false; }
		virtual bool isConnected()const { return false; }
		virtual bool isReadOnly()const { return false; }
		virtual bool CreateShm()
		{
			thelog << GetFullName() << " IShmActiveObject::CreateShm 未实现" << ende;
			return _undefined();
		}
		virtual bool CreatePrivate()
		{
			thelog << GetFullName() << " IShmActiveObject::CreatePrivate 未实现" << ende;
			return _undefined();
		}
		virtual bool _Attach(bool isReadOnly) = 0;
		//不可继承此接口，子类应当继承_Attach接口
		virtual bool Attach(bool isReadOnly)
		{
			bool ret;
			string str;
			string::size_type pos;
			if (CShmEnv::getInstPtr()->GetShmConfig(GetShmSysOfName(GetName()), GetName(), str, true))
			{
				string link = "link:";
				if (0 == (pos = str.find(link)))
				{
					thelog << GetFullName() << " 来自其他用户 " << str << endi;
					if (0 != g_ShmEnvRoot.size())
					{
						thelog << GetFullName() << "无法连接，原因：不能嵌套处理重定向" << ende;
						return false;
					}
					if (!CShmEnv::getInstPtr()->Disconect())
					{
						thelog << GetFullName() << "无法连接，原因：断开当前共享内存环境失败" << ende;
						return false;
					}
					g_ShmEnvRoot = str.substr(link.size());
					if (!CShmEnv::getInstPtr()->ShmEnvConnect(g_ShmEnvRoot.c_str()))
					{
						thelog << GetFullName() << "无法连接，原因：目标位置不存在或无法访问" << ende;
						g_ShmEnvRoot = "";
						if (!CShmEnv::getInstPtr()->ShmEnvConnect())
						{
							thelog << GetFullName() << "还原共享内存环境居然失败" << ende;
							return false;
						}
						return false;
					}
					else
					{
						ret = _Attach(isReadOnly);
					}
					if (!CShmEnv::getInstPtr()->Disconect())
					{
						thelog << GetFullName() << "连接后断开共享内存环境失败" << ende;
						return false;
					}
					g_ShmEnvRoot = "";
					if (!CShmEnv::getInstPtr()->ShmEnvConnect())
					{
						thelog << GetFullName() << "连接后还原共享内存环境居然失败" << ende;
						return false;
					}
				}
				else
				{
					ret = _Attach(isReadOnly);
				}
			}
			else
			{
				ret = _Attach(isReadOnly);
			}
			return ret;
		}
		virtual bool Detach()
		{
			thelog << GetFullName() << " IShmActiveObject::Detach 未实现" << ende;
			return _undefined();
		}
		virtual bool LoadFromDB()
		{
			thelog << GetFullName() << " IShmActiveObject::LoadFromDB 未实现" << ende;
			return _undefined();
		}
		virtual bool SaveToDB()
		{
			thelog << GetFullName() << " IShmActiveObject::SaveToDB 未实现" << ende;
			return _undefined();
		}
		virtual bool LoadFromDir(char const * dir_name)
		{
			thelog << GetFullName() << " IShmActiveObject::LoadFromDir 未实现" << ende;
			return _undefined();
		}
		virtual bool LoadPrivateFromDir(char const * dir_name)
		{
			thelog << GetFullName() << " IShmActiveObject::LoadPrivateFromDir 未实现" << ende;
			return _undefined();
		}
		virtual bool DestoryShm()
		{
			thelog << GetFullName() << " IShmActiveObject::DestoryShm 未实现" << ende;
			return _undefined();
		}
		virtual bool DestoryPrivate()
		{
			thelog << GetFullName() << " IShmActiveObject::DestoryPrivate 未实现" << ende;
			return _undefined();
		}
		virtual bool SaveToDir(char const * dir_name)const
		{
			thelog << GetFullName() << " IShmActiveObject::SaveToDir 未实现" << ende;
			return _undefined();
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			thelog << GetFullName() << " IShmActiveObject::ExportTextToDir 未实现" << ende;
			return _undefined();
		}
		virtual bool Report()const
		{
			thelog << GetFullName() << " IShmActiveObject::Report 未实现" << ende;
			return _undefined();
		}
		virtual bool ReportData()const
		{
			thelog << GetFullName() << " IShmActiveObject::ReportData 未实现" << ende;
			return _undefined();
		}
		virtual bool check()const
		{
			thelog << GetFullName() << " IShmActiveObject::check 未实现" << ende;
			return _undefined();
		}
		virtual bool repair(char const* level)
		{
			thelog << GetFullName() << " IShmActiveObject::repair 未实现" << ende;
			return _undefined();
		}
		virtual bool clear()
		{
			thelog << GetFullName() << " IShmActiveObject::clear 未实现" << ende;
			return _undefined();
		}
		size_t size()const override
		{
			thelog << GetFullName() << " IShmActiveObject::size 未实现" << ende;
			_undefined();
			return 0;
		}
		size_t capacity()const override
		{
			thelog << GetFullName() << " IShmActiveObject::capacity 未实现" << ende;
			_undefined();
			return 0;
		}
		size_t record_length()const override
		{
			thelog << GetFullName() << "IShmActiveObject::record_length 未实现" << ende;
			_undefined();
			return 1;
		}
		size_t byte_size()const override
		{
			thelog << GetFullName() << " IShmActiveObject::byte_size 未实现" << ende;
			_undefined();
			return 0;
		}
		size_t byte_capacity()const override
		{
			thelog << GetFullName() << " IShmActiveObject::byte_capacity 未实现" << ende;
			_undefined();
			return 0;
		}
		size_t block_count()const override
		{
			_undefined();
			return 1;
		}
		virtual bool ToDo(char const * what)
		{
			thelog << GetFullName() << " IShmActiveObject::ToDo 未实现" << ende;
			return true;
		}
		virtual bool ShowRet_AddDatas(CHtmlDoc::CHtmlTable2 & table, bool all = false, long level = 0)const
		{
			return _ShowRet_AddDatas(table, all, level);
		}
		virtual bool ClearRet()
		{
			return _ClearRet();
		}
		virtual bool AddNoSys()
		{
			g_sys.nosys.insert(GetName());
			DEBUG_LOG << GetName() << "AddNoSys " << g_sys.nosys.size() << endi;
			return true;
		}
		virtual void RunCmdUI()
		{
			string str;
			string cmd = "";
			string prompt = "\nb:返回上一层\n" + RunCmdUI_GetCommondList();

			while (true)
			{
				thelog << endl << GetName() << endi;
				if (GetPart() > 0)
				{
					thelog << "这是分块共享内存的第 " << GetPart() << " 块，不能单独处理" << endi;
					break;
				}
				cmd = UIInput(prompt.c_str(), "");
				if ("b" == cmd)break;
				if (!RunCmdUI_DoCommond(cmd))
				{
					thelog << endl << ShowRet(str) << endi;
					thelog << "未输入有效的选项" << endi;
				}
			}
		}
		string m_FastRebuild_level;
		virtual bool FastRebuild_Start()
		{
			thelog << GetFullName() << " IShmActiveObject::FastRebuild_Start 未实现" << ende;
			return false;
		}
		virtual bool FastRebuild_IncreToStatic()
		{
			thelog << GetFullName() << " IShmActiveObject::FastRebuild_IncreToStatic 未实现" << ende;
			return false;
		}
		virtual bool FastRebuild_Finish(char const* level, bool noDisk)
		{
			thelog << GetFullName() << " IShmActiveObject::FastRebuild_Finish 未实现" << ende;
			return false;
		}
	};

	//复合共享内存对象
	class CShmActiveObjects : public IShmActiveObject
	{
	public:
	protected:
		enum PROCESS_TYPE
		{
			TODO, DISABLE_MUTEX, CREATE_SHM, CREATE_PRIVATE, ATTACH, DETACH
			, LOAD_FROM_DB, SAVE_TO_DB, LOAD_FROM_DIR, LOAD_PRIVATE_FROM_DIR, DESTORY_SHM, DESTORY_PRIVATE
			, SAVE_TO_DIR, SEND_TO, EXPORT_TEXT_TO_DIR, REPORT, REPORT_DATA, CHECK, REPAIR, CLEAR
			, FAST_REBUILD_START, FAST_REBUILD_INCRE_TO_STATIC, FAST_REBUILD_FINISH
		};
		void set_PROCESS_TYPE_str(PROCESS_TYPE process_type, char const * what)
		{
			switch (process_type)
			{
			case TODO:
				m_process_type = what;
				break;
			case DISABLE_MUTEX:
				m_process_type = "禁用互斥";
				break;
			case ATTACH:
				m_process_type = "连接";
				break;
			case DETACH:
				m_process_type = "断开";
				break;
			case CREATE_SHM:
				m_process_type = "创建";
				break;
			case CREATE_PRIVATE:
				m_process_type = "创建私有";
				break;
			case LOAD_FROM_DB:
				m_process_type = "从数据库加载";
				break;
			case SAVE_TO_DB:
				m_process_type = "保存到数据库";
				break;
			case LOAD_FROM_DIR:
				m_process_type = "从目录恢复";
				break;
			case LOAD_PRIVATE_FROM_DIR:
				m_process_type = "从目录恢复到私有内存";
				break;
			case DESTORY_SHM:
				m_process_type = "删除共享内存";
				break;
			case DESTORY_PRIVATE:
				m_process_type = "清除私有内存";
				break;
			case SAVE_TO_DIR:
				m_process_type = "备份到目录";
				break;
			case SEND_TO:
				m_process_type = "网络发送";
				break;
			case EXPORT_TEXT_TO_DIR:
				m_process_type = "导出文本到目录";
				break;
			case REPORT:
				m_process_type = "报告";
				break;
			case REPORT_DATA:
				m_process_type = "数据";
				break;
			case CHECK:
				m_process_type = "检查数据";
				break;
			case REPAIR:
				m_process_type = "修复数据";
				break;
			case CLEAR:
				m_process_type = "清除数据";
				break;
			case FAST_REBUILD_START:
				m_process_type = "开始快速重建";
				break; 
			case FAST_REBUILD_INCRE_TO_STATIC:
				m_process_type = "增量到静态";
				break;
			case FAST_REBUILD_FINISH:
				m_process_type = "结束快速重建";
				break;
			default:
				m_process_type = "错误的操作类型";
				break;
			}
		}
		struct thread_data
		{
			pthread_t m_thread;//线程对象

			CShmActiveObjects * pMe;
			IShmActiveObject * pObj;
			PROCESS_TYPE process_type;
			bool bParam;
			char const * what;
		};
		struct _data_t
		{
			IShmActiveObject * pIShmActiveObject;
			thread_data m_thread_data;

			_data_t() :pIShmActiveObject(NULL) {}
		};
		typedef vector<_data_t > T_DATAS;

		T_DATAS m_pTables;

		static void * _ProcessThread(void * p)
		{
			thread_data * pThreadData = (thread_data *)p;
			CShmActiveObjects * pMe = pThreadData->pMe;
			IShmActiveObject * pObj = pThreadData->pObj;
			PROCESS_TYPE process_type = pThreadData->process_type;
			bool bParam = pThreadData->bParam;
			char const * what = pThreadData->what;

			//thelog<<"开始处理["<<it->first<<"]......"<<endi;
			time_t t = time(NULL);
			switch (process_type)
			{
			case TODO:
				pObj->m_process_ret = pObj->ToDo(what);
				break;
			case DISABLE_MUTEX:
				pObj->m_process_ret = pObj->disableMutex();
				break;
			case ATTACH:
				pObj->m_process_ret = pObj->Attach(bParam);
				break;
			case DETACH:
				pObj->m_process_ret = pObj->Detach();
				break;
			case CREATE_SHM:
				thelog << "创建[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]..." << endi;
				pObj->m_process_ret = pObj->CreateShm();
				break;
			case CREATE_PRIVATE:
				thelog << "创建私有[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]..." << endi;
				pObj->m_process_ret = pObj->CreatePrivate();
				break;
			case LOAD_FROM_DB:
				thelog << "从数据库加载[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]..." << endi;
				pObj->m_process_ret = pObj->LoadFromDB();
				break;
			case SAVE_TO_DB:
				thelog << "保存[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]到数据库..." << endi;
				pObj->m_process_ret = pObj->SaveToDB();
				break;
			case LOAD_FROM_DIR:
				thelog << "从目录恢复[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]..." << endi;
				pObj->m_process_ret = pObj->LoadFromDir(what);
				break;
			case LOAD_PRIVATE_FROM_DIR:
				thelog << "从目录恢复[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]到私有内存..." << endi;
				pObj->m_process_ret = pObj->LoadPrivateFromDir(what);
				break;
			case DESTORY_SHM:
				thelog << "删除[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的共享内存..." << endi;
				pObj->m_process_ret = pObj->DestoryShm();
				break;
			case DESTORY_PRIVATE:
				thelog << "清除[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的私有内存..." << endi;
				pObj->m_process_ret = pObj->DestoryPrivate();
				break;
			case SAVE_TO_DIR:
				thelog << "备份[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]到目录..." << endi;
				pObj->m_process_ret = pObj->SaveToDir(what);
				break;
			case EXPORT_TEXT_TO_DIR:
				thelog << "导出[" << pMe->GetFullName() << "]的文本[" << pObj->GetFullName() << "]到目录..." << endi;
				pObj->m_process_ret = pObj->ExportTextToDir(what);
				break;
			case REPORT:
				thelog << "报告[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]..." << endi;
				pObj->m_process_ret = pObj->Report();
				break;
			case REPORT_DATA:
				thelog << "数据[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]..." << endi;
				pObj->m_process_ret = pObj->ReportData();
				break;
			case CHECK:
				thelog << "检查[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的数据..." << endi;
				pObj->m_process_ret = pObj->check();
				break;
			case REPAIR:
				thelog << "修复[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的数据..." << endi;
				pObj->m_process_ret = pObj->repair(what);
				break;
			case CLEAR:
				thelog << "清除[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的数据..." << endi;
				pObj->m_process_ret = pObj->clear();
				break;
			case FAST_REBUILD_START:
				thelog << "开始快速重建[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的数据..." << endi;
				pObj->m_process_ret = pObj->FastRebuild_Start();
				break;
			case FAST_REBUILD_INCRE_TO_STATIC:
				thelog << "增量到静态[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的数据..." << endi;
				pObj->m_process_ret = pObj->FastRebuild_IncreToStatic();
				break;
			case FAST_REBUILD_FINISH:
				thelog << "结束快速重建[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]的数据..." << endi;
				pObj->m_process_ret = pObj->FastRebuild_Finish(what, bParam);
				break;
			default:
				pObj->m_process_ret = false;
				break;
			}
			pObj->m_process_seconds = time(NULL) - t;
			pObj->m_process_type = pMe->m_process_type;
			if (ATTACH != process_type && DETACH != process_type)
			{
				thelog << "[" << pMe->GetFullName() << "]的[" << pObj->GetFullName() << "]" << pObj->m_process_type << " 用时 " << pObj->m_process_seconds << " 秒" << endi;
			}

			return nullptr;
		}
		bool _Process(PROCESS_TYPE process_type, bool bParam = true, char const * what = NULL)
		{
			ClearRet();
			T_DATAS::iterator it;
			m_process_type = "unknow operator";
			long count_ok = 0;
			long count_err = 0;
			long count_undefined = 0;
			time_t t_all = time(NULL);
			set_PROCESS_TYPE_str(process_type, what);
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				//thelog<<"开始处理["<<it->first<<"]......"<<endi;
				it->m_thread_data.pMe = this;
				it->m_thread_data.pObj = it->pIShmActiveObject;
				it->m_thread_data.process_type = process_type;
				it->m_thread_data.bParam = bParam;
				it->m_thread_data.what = what;
				if (0 != pthread_create(&it->m_thread_data.m_thread, NULL, _ProcessThread, &it->m_thread_data))
				{
					thelog << "创建子线程失败" << ende;
					return false;
				}
			}

			//等待线程结束并汇总结果
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				pthread_join(it->m_thread_data.m_thread, NULL);

				if (it->pIShmActiveObject->m_undefined)
				{
					++count_undefined;
				}
				else if (it->pIShmActiveObject->m_process_ret)
				{
					++count_ok;
					//thelog<<it->first<<" 处理成功"<<endi;
				}
				else
				{
					++count_err;
					DEBUG_LOG << "[" << GetFullName() << "]的[" << (*it).pIShmActiveObject->GetFullName() << "]" << it->pIShmActiveObject->m_process_type << "处理失败" << endi;
				}
			}
			m_process_seconds = time(NULL) - t_all;
			m_process_ret = true;
			if (0 != count_err)
			{
				m_process_ret = false;
				DEBUG_LOG << "总共用时 " << m_process_seconds << " 秒" << endi;
				DEBUG_LOG << "[" << GetFullName() << "]共 " << m_pTables.size() << " 个，成功" << m_process_type << " " << count_ok << " 个，失败 " << count_err << " 个，未定义 " << count_undefined << " 个" << endi;
				string str;
				DEBUG_LOG << endl << ShowRet(str, true) << endi;
			}
			return 0 == count_err;
		}
	public:
		//CShmActiveObjects
		bool AddTable(IShmActiveObject * p)
		{
			if (NULL == p)return false;
			_data_t tmp;
			tmp.pIShmActiveObject = p;
			m_pTables.push_back(tmp);
			return true;
		}
		//IShmActiveObject
		virtual bool GetChildsName(string & ret)const
		{
			T_DATAS::const_iterator it;
			string str;
			ret = "";
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				ret += it->pIShmActiveObject->GetName();
				if (it->pIShmActiveObject->GetChildsName(str))
				{
					ret += "(组) ";
				}
				else
				{
					ret += " ";
				}
			}
			return true;
		}
		virtual bool disableMutex()const
		{
			return ((CShmActiveObjects*)this)->_Process(DISABLE_MUTEX);
		}
		virtual bool isPrivateMem()const
		{
			bool ret = false;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				if (it->pIShmActiveObject->isPrivateMem())
				{
					ret = true;
					break;
				}
			}
			return ret;
		}
		virtual bool isConnected()const
		{
			bool ret = true;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				if (!it->pIShmActiveObject->isConnected())
				{
					ret = false;
					break;
				}
			}
			return ret;
		}
		virtual bool isReadOnly()const
		{
			bool ret = false;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				if (it->pIShmActiveObject->isReadOnly())
				{
					ret = true;
					break;
				}
			}
			return ret;
		}
		virtual bool CreateShm()
		{
			return _Process(CREATE_SHM);
		}
		virtual bool CreatePrivate()
		{
			return _Process(CREATE_PRIVATE);
		}
		virtual bool _Attach(bool isReadOnly)
		{
			//thelog<<GetFullName()<<" 连接..."<<endi;
			return _Process(ATTACH, isReadOnly, NULL);
		}
		virtual bool Detach()
		{
			//thelog<<GetFullName()<<" 断开..."<<endi;
			return _Process(DETACH);
		}
		virtual bool LoadFromDB()
		{
			return _Process(LOAD_FROM_DB);
		}
		virtual bool SaveToDB()
		{
			return _Process(SAVE_TO_DB);
		}
		virtual bool LoadFromDir(char const * dir_name)
		{
			return _Process(LOAD_FROM_DIR, true, dir_name);
		}
		virtual bool LoadPrivateFromDir(char const * dir_name)
		{
			return _Process(LOAD_PRIVATE_FROM_DIR, true, dir_name);
		}
		virtual bool DestoryShm()
		{
			return _Process(DESTORY_SHM);
		}
		virtual bool DestoryPrivate()
		{
			return _Process(DESTORY_PRIVATE);
		}
		virtual bool SaveToDir(char const * dir_name)const
		{
			return ((CShmActiveObjects*)this)->_Process(SAVE_TO_DIR, true, dir_name);
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			return ((CShmActiveObjects*)this)->_Process(EXPORT_TEXT_TO_DIR, true, dir_name);
		}
		virtual bool Report()const
		{
			//return (*((CShmActiveObjects*)this))._Process(REPORT);
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				it->pIShmActiveObject->Report();
			}
			return true;
		}
		virtual bool ReportData()const
		{
			//return (*((CShmActiveObjects*)this))._Process(REPORT_DATA);
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				it->pIShmActiveObject->ReportData();
			}
			return true;
		}
		virtual bool check()const
		{
			return (*((CShmActiveObjects*)this))._Process(CHECK);
		}
		virtual bool repair(char const* level)
		{
			return (*((CShmActiveObjects*)this))._Process(REPAIR, false, level);
		}
		virtual bool clear()
		{
			return _Process(CLEAR);
		}
		size_t size()const override
		{
			long n = 0;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				n += it->pIShmActiveObject->size();
			}
			return n;
		}
		size_t capacity()const override
		{
			long n = 0;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				n += it->pIShmActiveObject->capacity();
			}
			return n;
		}
		size_t record_length()const override
		{
			return 0;
		}
		size_t byte_size()const override
		{
			long n = 0;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				n += it->pIShmActiveObject->byte_size();
			}
			return n;
		}
		size_t byte_capacity()const override
		{
			long n = 0;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				n += it->pIShmActiveObject->byte_capacity();
			}
			return n;
		}
		size_t block_count()const override
		{
			long n = 0;
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				n += it->pIShmActiveObject->block_count();
			}
			return n;
		}
		virtual bool ToDo(char const * what)
		{
			return (*((CShmActiveObjects*)this))._Process(TODO, false, what);
		}
		virtual bool ShowRet_AddDatas(CHtmlDoc::CHtmlTable2 & table, bool all = false, long level = 0)const
		{
			T_DATAS::const_iterator it;
			long count_ok = 0;
			long count_err = 0;
			long count_undefined = 0;
			long count_ingroed = 0;//因为父类重载而没有执行到的
			char buf[2048];
			string str;

			_ShowRet_AddDatas(table, all, level);
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				if (all)it->pIShmActiveObject->ShowRet_AddDatas(table, all, level + 1);
				if (0 != it->pIShmActiveObject->m_process_type.size())
				{
					if (it->pIShmActiveObject->m_undefined)
					{
						++count_undefined;
					}
					else if (it->pIShmActiveObject->m_process_ret)
					{
						++count_ok;
					}
					else
					{
						++count_err;
					}
				}
				else
				{
					++count_ingroed;
				}
			}
			if (0 != m_process_type.size())
			{
				table.AddLine();
				if (0 != count_err)sprintf(buf, "成功 %ld 个 失败 %ld 个 未定义 %ld 个 忽略 %ld 个 用时 %ld 秒", count_ok, count_err, count_undefined, count_ingroed, m_process_seconds);
				else sprintf(buf, "共 %ld 个 全部成功 未定义 %ld 个 忽略 %ld 个 用时 %ld 秒", count_ok, count_undefined, count_ingroed, m_process_seconds);
				table.AddData(perfix(level, str) + buf, false, 2);
			}

			return true;
		}
		virtual bool ClearRet()
		{
			_ClearRet();
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				it->pIShmActiveObject->ClearRet();
			}

			return true;
		}
		virtual bool AddNoSys()
		{
			T_DATAS::const_iterator it;
			for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
			{
				it->pIShmActiveObject->AddNoSys();
			}

			return true;
		}
		//命令行操作
		virtual void RunCmdUI()
		{
			string str;
			string cmd = "";
			string prompt = "\nb:返回上一层 child:选择子项操作 默认显示子项列表和最后操作状态\n" + RunCmdUI_GetCommondList();

			while (true)
			{
				GetChildsName(str);
				thelog << endl << "当前对象：" << GetName() << " 子项[" << str << "]" << endi;
				if (GetPart() > 0)
				{
					thelog << "这是分块共享内存的第 " << GetPart() << " 块，不能单独处理" << endi;
					break;
				}
				cmd = UIInput(prompt.c_str(), "");
				if ("b" == cmd)break;
				else if ("child" == cmd)
				{
					{
						string childsname;
						if (GetChildsName(childsname))
						{
							thelog << endl << "子项列表：" << childsname << endi;
						}
						theLog << endi;
					}
					str = UIInput("请输入子项名称", "");
					if (str.size() > 0)
					{
						T_DATAS::const_iterator it;
						for (it = m_pTables.begin(); it != m_pTables.end(); ++it)
						{
							if (it->pIShmActiveObject->GetName() == str)
							{
								it->pIShmActiveObject->RunCmdUI();
							}
						}
					}
				}
				else if (!RunCmdUI_DoCommond(cmd))
				{
					thelog << endl << ShowRet(str, true) << endi;
					thelog << "未输入有效的选项" << endi;
				}
			}
		}
		virtual bool FastRebuild_Start()
		{
			return (*((CShmActiveObjects*)this))._Process(FAST_REBUILD_START);
		}
		virtual bool FastRebuild_IncreToStatic()
		{
			return (*((CShmActiveObjects*)this))._Process(FAST_REBUILD_INCRE_TO_STATIC);
		}
		virtual bool FastRebuild_Finish(char const* level, bool noDisk)
		{
			return (*((CShmActiveObjects*)this))._Process(FAST_REBUILD_FINISH, noDisk, level);
		}
	};
}
