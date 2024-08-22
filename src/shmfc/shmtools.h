//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../env/env.h"
#include "semmgr.h"
#include "../function/myProcPseudo.h"
#include "../env/CommandSet.h"

namespace ns_my_std
{
	class CEnvTools
	{
	public:
		struct struct_T_ARRAY_VMAP_tmp
		{
			int shm_id;//共享内存ID，第一个总是和array_head在一起，其余块只有数据
			T_SHM_SIZE handle_begin;//此块对应的handle范围[handle_begin,handle_end）
			T_SHM_SIZE handle_end;
		};
		struct struct_T_ARRAY_VMAP_S_tmp
		{
			long size;//使用的个数
			struct_T_ARRAY_VMAP_tmp m_vmaps[T_ARRAY_VMAP_MAX_SIZE];
		};
		//数组头的前半部分，后半部分不同版本顺序不同
		struct array_head_head_tmp
		{
			CMeta meta;
			sstring<64> name;
			T_SHM_SIZE capacity;
			T_SHM_SIZE size;
			//struct_T_ARRAY_VMAP_S vmaps;//分块影射表
		};
		class FindAllShm : public CCommandSet::command
		{
		protected:
			CHtmlDoc::CHtmlTable2 table;
		public:
			FindAllShm()
			{
				m_command_name = "FindAllShm";
				m_command_note = "查找全部共享内存";
			}
			//获得系统命令输出
			int GetIPCS(long & col_shmid)
			{
				table.AddCol("key");
				col_shmid = table.AddCol("shmid", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("owner");
				table.AddCol("perms");
				table.AddCol("bytes", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				table.AddCol("nattch", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

				string str;
				GetShellOutput("ipcs -m", str);//获得命令输出
				Replace(str, "\t", " ");//替换掉跳格
				Replace(str, "  ", " ");//连续空格压缩为一个

				StringTokenizer st_line(str, "\n");
				for (size_t i = 0; i < st_line.size(); ++i)
				{
					//忽略空行
					Trim(st_line[i]);
					if (0 == st_line[i].size())continue;

					StringTokenizer st_col(st_line[i], " ");
					if (0==i)
					{
						//if (st_col.size() >= table.GetColCount())
						//{
						//	bool _ok = true;
						//	for (long j = 0; j < table.GetColCount(); ++j)
						//	{
						//		thelog << table.GetColName(j) << " " << st_col[j] << endi;
						//		if (table.GetColName(j) != st_col[j])
						//		{
						//			_ok = false;
						//		}
						//	}
						//	if (_ok)titleOK = true;
						//}
					}
					else
					{
						if (st_col.size() < table.GetColCount())
						{
							thelog << "输出不足 " << table.GetColCount() << " " << st_col.size() << ende;
						}
						else
						{
							table.AddLine();
							for (size_t j = 0; j < table.GetColCount(); ++j)
							{
								table.AddData(st_col[j], (4 == j ? true : false));
							}
						}
					}
				}
				thelog << table.MakeTextTable() << endi;
				//if (!titleOK)
				//{
				//	thelog << "命令输出分析失败" << ende;
				//	return __LINE__;
				//}
				return 0;
			}
			//检查meta
			int CheckMeta(long col_shmid, long & col_result_type, long & col_int2, long & col_msg)
			{
				long col_size = table.AddCol("size", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_ctime = table.AddCol("ctime", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_ismeta = table.AddCol("meta", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_isconnect = table.AddCol("conn", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_issys = table.AddCol("sys", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_intcount = table.AddCol("IC", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_int1 = table.AddCol("v/head", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				col_int2 = table.AddCol("size/user", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_int3 = table.AddCol("regmax/T", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_int4 = table.AddCol("v", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_int5 = table.AddCol("998", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_int6 = table.AddCol("999", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_int7 = table.AddCol("1000", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_shmid2 = table.AddCol("shmid2", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				col_result_type = table.AddCol("type", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);//0未知 1env块 2主块 3附加块
				col_msg = table.AddCol("result");

				for (size_t i = 0; i < table.GetRecordCount(); ++i)
				{
					string str;

					int shm_id = atol(table.GetData(i, col_shmid).c_str());
					table.SetData(i, col_shmid2, shm_id);
					table.SetData(i, col_result_type, 0);
					size_t size;
					time_t ctime;
					if (!CShmMan::GetState(shm_id, size, ctime))
					{
						table.SetData(i, col_msg, "无法获取");
						continue;
					}
					table.SetData(i, col_size, size);
					table.SetData(i, col_ctime, CMyTools::TimeToString_Time(ctime));

					if (size < sizeof(CMeta))
					{
						table.SetData(i, col_ismeta, "X");
						table.SetData(i, col_msg, "无法识别");
						continue;
					}

					CMeta meta;
					PSHM p = CShmMan::ConnectByID(shm_id, true);
					if (NULL == p)
					{
						table.SetData(i, col_isconnect, "X");
						table.SetData(i, col_msg, "无法连接");
						continue;
					}
					table.SetData(i, col_isconnect, "V");

					meta.CopyFrom((signed char *)(void *)p);
					if (!meta.CheckGuid(GUID_T_ARRAY))
					{
						table.SetData(i, col_ismeta, "X");
						table.SetData(i, col_msg, "没有匹配到GUID标识");
						CShmMan::Disconnect(p);
						continue;
					}
					table.SetData(i, col_ismeta, "V");

					if (!meta.CheckSys())
					{
						table.SetData(i, col_issys, "X");
						table.SetData(i, col_msg, "机型或32位/64位不匹配");
						thelog << endl << meta.toString(str) << endi;
						CShmMan::Disconnect(p);
						continue;
					}
					table.SetData(i, col_issys, "V");

					table.SetData(i, col_intcount, meta.GetIntCount());
					table.SetData(i, col_int1, meta.GetInt(0));
					table.SetData(i, col_int2, meta.GetInt(1));
					table.SetData(i, col_int3, meta.GetInt(2));
					table.SetData(i, col_int4, meta.GetInt(3));
					table.SetData(i, col_int5, meta.GetInt(4));
					table.SetData(i, col_int6, meta.GetInt(5));
					table.SetData(i, col_int7, meta.GetInt(6));

					if (3 == meta.GetIntCount())
					{
						table.SetData(i, col_msg, "可能是ShmENV主控块，请用命令查看");
						table.SetData(i, col_result_type, 1);
					}
					if (7 == meta.GetIntCount() && 998 == meta.GetInt(4) && 999 == meta.GetInt(5) && 1000 == meta.GetInt(6))
					{
						table.SetData(i, col_msg, "可能是主共享内存");
						table.SetData(i, col_result_type, 2);
					}
					CShmMan::Disconnect(p);
				}
				return 0;
			}
			int CheckMain(long col_shmid, long col_result_type, int col_int2, long col_msg,bool checkAdditionalBlock)
			{
				long col_name = table.AddCol("name");
				long col_capacity = table.AddCol("capacity", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_size = table.AddCol("size", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_format = table.AddCol("格式", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				long col_block = table.AddCol("block", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

				for (size_t i = 0; i < table.GetRecordCount(); ++i)
				{
					long result_type = atol(table.GetData(i, col_result_type).c_str());
					if (2 != result_type)continue;

					int shm_id = atol(table.GetData(i, col_shmid).c_str());
					PSHM p = CShmMan::ConnectByID(shm_id, true);
					if (NULL == p)
					{
						thelog << "无法连接？？？？？" << ende;
						return __LINE__;
					}
					array_head_head_tmp * pHead = (array_head_head_tmp*)p;
					table.SetData(i, col_name, pHead->name.c_str());
					table.SetData(i, col_capacity, pHead->capacity);
					table.SetData(i, col_size, pHead->size);

					struct_T_ARRAY_VMAP_S_tmp * pVMap = (struct_T_ARRAY_VMAP_S_tmp *)(p + sizeof(array_head_head_tmp));
					if (shm_id != pVMap->m_vmaps[0].shm_id)
					{
						thelog << "不是当前版本格式" << endi;
						pVMap = (struct_T_ARRAY_VMAP_S_tmp *)(p + sizeof(array_head_head_tmp) + atol(table.GetData(i, col_int2).c_str()));
						if (shm_id != pVMap->m_vmaps[0].shm_id)
						{
							thelog << "也不是旧版本格式" << ende;
							CShmMan::Disconnect(p);
							continue;
						}
						else
						{
							table.SetData(i, col_format, "old");
						}
					}
					else
					{
						table.SetData(i, col_format, "current");
					}

					table.SetData(i, col_block, pVMap->size);
					bool isOK = true;
					for (long i_v = 1; i_v < pVMap->size && checkAdditionalBlock; ++i_v)
					{
						if (!FindAdditionalBlock(pVMap->m_vmaps[i_v].shm_id, pHead->name.c_str(), col_shmid, col_result_type, col_msg))
						{
							isOK = false;
							table.SetData(i, col_msg, "附加块没有找到");
							thelog << shm_id << " " << pHead->name.c_str() << " 的附加块 " << pVMap->m_vmaps[i_v].shm_id << " 没有找到" << ende;
						}
					}
					if (isOK)table.SetData(i, col_msg, "主共享内存");

					CShmMan::Disconnect(p);
				}
				return 0;
			}
			bool FindAdditionalBlock(long id, char const * name, long col_shmid, long col_result_type, long col_msg)
			{
				for (size_t i = 0; i < table.GetRecordCount(); ++i)
				{
					long shm_id = atol(table.GetData(i, col_shmid).c_str());
					long result_type = atol(table.GetData(i, col_result_type).c_str());
					if (shm_id == id)
					{
						if (0 == result_type)
						{
							table.SetData(i, col_result_type, 3);
							table.SetData(i, col_msg, name);
							return true;
						}
						else
						{
							thelog << "此块看起来不是附加块 " << shm_id << ende;
						}
					}
				}
				return false;
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				table.Clear();
				long col_shmid;

				int ret;

				ret = GetIPCS(col_shmid);
				if (0 != ret)return ret;

				long col_result_type;
				long col_int2;
				long col_msg;
				ret = CheckMeta(col_shmid, col_result_type, col_int2, col_msg);
				if (0 != ret)return ret;

				ret = CheckMain(col_shmid, col_result_type, col_int2, col_msg, true);
				if (0 != ret)return ret;

				thelog << table.MakeTextTable() << endi;
				return 0;
			}
		};
		class FindCurrentShm : public FindAllShm
		{
		public:
			FindCurrentShm()
			{
				m_command_name = "FindCurrentShm";
				m_command_note = "查找当前环境注册的共享内存";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				table.Clear();

				int ret;

				if (!CShmEnv::getInstPtr()->MakeShmTable(table))return __LINE__;
				long col_shmid=table.GetColIndex("shmid");
				if (col_shmid < 0)
				{
					thelog<<"格式错误，未发现shmid列"<<ende;
					return __LINE__;
				}

				long col_result_type;
				long col_int2;
				long col_msg;
				ret = CheckMeta(col_shmid, col_result_type, col_int2, col_msg);
				if (0 != ret)return ret;

				ret = CheckMain(col_shmid, col_result_type, col_int2, col_msg, false);
				if (0 != ret)return ret;

				thelog << table.MakeTextTable() << endi;
				return 0;
			}
		};
		class ShowEnvShm : public CCommandSet::command
		{
		public:
			ShowEnvShm()
			{
				m_command_name = "ShowEnvShm";
				m_command_note = "显示指定的ShmEnv";
			}
			virtual int _doCommand(string const& input, string& output, bool silent)
			{
				string id;
				if (!csGetCommandParam("-shmid", "请输入共享内存ID", id, silent))
				{
					thelog << "需要参数 -shmid" << ende;
					return __LINE__;
				}
				int shm_id = atol(id.c_str());
				size_t size;
				time_t ctime;
				if (!CShmMan::GetState(shm_id, size, ctime))
				{
					thelog << "无法获取" << ende;
					return __LINE__;
				}
				if (size < CShmEnv::getInstPtr()->GetShmEnvSize())
				{
					thelog << "长度不足，肯定不是" << ende;
					return __LINE__;
				}
				PSHM p = CShmMan::ConnectByID(shm_id, true);
				if (NULL == p)
				{
					thelog << "无法连接" << ende;
					return __LINE__;
				}
				CShmEnv::getInstPtr()->ReportShmEnv(p);

				CShmMan::Disconnect(p);
				return 0;
			}
		};
		class ShowSysState : public CCommandSet::command
		{
		public:
			ShowSysState()
			{
				m_command_name = "ShowSysState";
				m_command_note = "显示系统状态";
			}
			virtual int _doCommand(string const& input, string& output, bool silent)
			{
				CProcPseudo pp;
				stringstream ss;
				pp.ShowSysState(ss);
				thelog << endl << ss.str() << endi;
				return 0;
			}
		};
	public:
		static bool AddToCommandSet(CCommandSet & commandset)
		{
			if (!commandset.AddGroup("工具"))return false;
			if (!commandset.AddCommand("101", new FindAllShm()))return false;
			if (!commandset.AddCommand("102", new FindCurrentShm()))return false;
			if (!commandset.AddCommand("103", new ShowEnvShm()))return false;
			if (!commandset.AddCommand("201", new ShowSysState()))return false;
			return true;
		}
	};
}
