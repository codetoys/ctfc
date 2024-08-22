//semmgr.h 信号量管理
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "../function/config.h"
#include "../env/myUtil.h"
#include "shmEnv.h"

namespace ns_my_std
{
	class CSemMgr
	{
	private:
		vector<sem_reg_info > m_datas;
		CMyShmMutex* m_sems;
	public:
		CSemMgr():m_sems(NULL){}
		~CSemMgr()
		{
			clear();
		}
		bool clear()
		{
			if(NULL!=m_sems)
			{
				size_t i;
				for(i=0;i<m_datas.size();++i)
				{
					m_sems[i].Detach();
				}
				delete[] m_sems;
			}
			m_sems = NULL;
			m_datas.clear();
			return true;
		}
		bool Load()
		{
			clear();

			if(!CManagedMutex::GetAllSemInfo(m_datas))
			{
				return false;
			}
			
			m_sems = new CMyShmMutex[m_datas.size()];
			if(NULL==m_sems)
			{
				thelog<<"内存不足"<<ende;
				m_datas.clear();
				return false;
			}
			
			if(true)
			{
				size_t i;
				for(i=0;i<m_datas.size();++i)
				{
					m_sems[i].Attach(m_datas[i].pSemID);
				}
			}

			if(true)
			{
				CHtmlDoc::CHtmlTable2 htmltable;
				htmltable.AddCol("i", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("sys");
				htmltable.AddCol("name");
				htmltable.AddCol("PART", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				//htmltable.AddCol("locked", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("ctime", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
				htmltable.AddCol("info");
				size_t i;
				for(i=0;i<m_datas.size();++i)
				{
					htmltable.AddLine();
					htmltable.AddData(i);
					htmltable.AddData(m_datas[i].sys.c_str());
					htmltable.AddData(m_datas[i].name.c_str());
					htmltable.AddData(m_datas[i].part);
					//htmltable.AddData(m_datas[i].pSemID->isLocked());
					htmltable.AddData(CMyTools::TimeToString_log(m_datas[i].pSemID->ctime));
					htmltable.AddData(m_sems[i].Report().c_str());
				}
				thelog<<htmltable.MakeTextTable()<<endi;
			}

			return true;
		}
		bool Unlock()
		{
			if(0==m_datas.size())
			{
				if(!Load())
				{
					thelog << "查询失败 " << ende;
					return false;
				}
			}

			string str = UIInput("请输入信号量位置号：", "-1");
			long i = atol(str.c_str());
			if (i < 0 || (size_t)i >= m_datas.size())
			{
				thelog << "无效的输入 " << i << ende;
				return false;
			}

			thelog<<"当前操作的是 "<<m_datas[i].sys.c_str()<<" "<<m_datas[i].name.c_str()<<" "<<m_datas[i].part<<endi;
			thelog << m_sems[i].Report() << endi;

			long w, w_count, r_count, w_wait;
			if (!m_sems[i].GetCount2(w, w_count, r_count, w_wait))
			{
				thelog<<"获取信号量信息错误"<<ende;
				return false;
			}
			if(r_count>0)
			{
				if(UIInput("信号量处于读锁定中，执行一次解读锁？（y/n）","n")=="y")
				{
					if(!m_sems[i].RUnLock())
					{
						thelog<<"解读锁错误 "<<m_sems[i].Report() <<ende;
						return false;
					}
					else
					{
						thelog<<"解读锁完成 "<<m_sems[i].Report() <<endi;
						return true;
					}
				}
				else
				{
					thelog<<"用户取消"<<endi;
					return true;
				}
			}
			if(w_count>0)
			{
				if(UIInput("信号量处于写锁定中，执行一次解写锁？（y/n）","n")=="y")
				{
					if(!m_sems[i].WUnLock())
					{
						thelog<<"解写锁错误 "<<m_sems[i].Report() <<ende;
						return false;
					}
					else
					{
						thelog<<"解写锁完成 "<<m_sems[i].Report() <<endi;
						return true;
					}
				}
				else
				{
					thelog<<"用户取消"<<endi;
					return true;
				}
			}
			thelog<<"信号量不在锁定中"<<endi;
			return true;
		}
	};
}
