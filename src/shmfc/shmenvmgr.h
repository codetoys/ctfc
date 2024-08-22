//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmEnv.h"
#include "semmgr.h"
#include "../env/CommandSet.h"

namespace ns_my_std
{
	class CShmEnvMgr
	{
	public:
		class ShmEnvShow : public CCommandSet::command
		{
		public:
			ShmEnvShow()
			{
				m_command_name = "ShmEnvShow";
				m_command_note = "显示主共享内存";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				return (CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->Report() ? 0 : 1);
			}
		};
		class ShmEnvCreate : public CCommandSet::command
		{
		public:
			ShmEnvCreate()
			{
				m_command_name = "ShmEnvCreate";
				m_command_note = "创建主共享内存";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				if (CShmEnv::getInstPtr()->ShmEnvCreate())
				{
					CGlobalMutex mutex;
					mutex.CGlobalMutex_Lock();
					mutex.CGlobalMutex_UnLock();
					return 0;
				}
				else return 1;
			}
		};
		class SystemParamLoad : public CCommandSet::command
		{
		public:
			SystemParamLoad()
			{
				m_command_name = "SystemParamLoad";
				m_command_note = "加载系统参数";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				return (CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->SystemParam_Load() ? 0 : 1);
			}
		};
		class SystemParamSaveAs : public CCommandSet::command
		{
		public:
			SystemParamSaveAs()
			{
				m_command_name = "SystemParamSaveAs";
				m_command_note = "系统参数另存为";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				return (CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->SystemParam_SaveAs() ? 0 : 1);
			}
		};
		class SystemParamClear : public CCommandSet::command
		{
		public:
			SystemParamClear()
			{
				m_command_name = "SystemParamClear";
				m_command_note = "清空系统参数";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				return (CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->SystemParam_Clear() ? 0 : 1);
			}
		};
		class SystemParamDelete : public CCommandSet::command
		{
		public:
			SystemParamDelete()
			{
				m_command_name = "SystemParamDelete";
				m_command_note = "删除系统参数";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				string section = UIInput("请输入参数节：", "");
				if (0 == section.size())
				{
					thelog << "未输入参数节" << ende;
					return 1;
				}
				string name = UIInput("请输入参数名：", "");
				if (0 == name.size())
				{
					thelog << "未输入参数名" << ende;
					return 1;
				}
				return (CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->SystemParam_Delete(section.c_str(), name.c_str()) ? 0 : 1);
			}
		};
		class SystemParamSet : public CCommandSet::command
		{
		public:
			SystemParamSet()
			{
				m_command_name = "SystemParamSet";
				m_command_note = "设置系统参数";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				string section;
				if (!this->csGetCommandParam("-section", "请输入参数节", section, silent) || 0 == section.size())
				{
					thelog << "未输入参数节" << ende;
					return 1;
				}
				string name;
				if (!this->csGetCommandParam("-name", "请输入参数名", name, silent) || 0 == name.size())
				{
					thelog << "未输入参数名" << ende;
					return 1;
				}
				string value;
				if (!this->csGetCommandParam("-value", "请输入参数值", value, silent) || 0 == value.size())
				{
					thelog << "未输入参数值" << ende;
					return 1;
				}
				return (CShmEnv::getInstPtr()->ShmEnvConnect() && CShmEnv::getInstPtr()->SystemParam_SetCurrent(section.c_str(), name.c_str(), value.c_str()) ? 0 : 1);
			}
		};
		class SemShow : public CCommandSet::command
		{
		public:
			SemShow()
			{
				m_command_name = "SemShow";
				m_command_note = "显示信号量";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				CSemMgr semmgr;
				return (semmgr.Load() ? 0 : 1);
			}
		};
		class SemUnlock : public CCommandSet::command
		{
		public:
			SemUnlock()
			{
				m_command_name = "SemUnlock";
				m_command_note = "解锁信号量";
			}
			virtual int _doCommand(string const & input, string & output, bool silent)
			{
				CSemMgr semmgr;
				semmgr.Load();
				return (semmgr.Unlock() ? 0 : 1);
			}
		};
	public:
		static bool AddToCommandSet(CCommandSet & commandset)
		{
			if(!commandset.AddGroup("系统管理"))return false;
			if(!commandset.AddCommand("0", new ShmEnvShow()))return false;
			if(!commandset.AddCommand("1", new ShmEnvCreate()))return false;
			if(!commandset.AddCommand("2", new SystemParamLoad()))return false;
			if (!commandset.AddCommand("3", new SystemParamSaveAs()))return false;
			if (!commandset.AddCommand("4", new SystemParamClear()))return false;
			if (!commandset.AddCommand("5", new SystemParamDelete()))return false;
			if (!commandset.AddCommand("6", new SystemParamSet()))return false;
			if(!commandset.AddGroup("信号量管理"))return false;
			if(!commandset.AddCommand("10", new SemShow()))return false;
			if(!commandset.AddCommand("11", new SemUnlock()))return false;

			return true;
		}
	};
}
