//shm_LRU.h 共享内存LRU算法
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmstd.h"
#include "shmSet.h"

namespace ns_my_std
{
#define LRU_HEAD_VIRTUAL_SET_HEAD_SIZE 1024

	struct T_USER_HEAD_LRU
	{
		T_SHM_SIZE head_FLRU;//访问过的节点的链表
		T_SHM_SIZE head_MLRU;//修改过的节点的链表
		char vitrual_set_head[LRU_HEAD_VIRTUAL_SET_HEAD_SIZE];//虚拟SET的HEAD

		T_USER_HEAD_LRU():head_FLRU(-1),head_MLRU(-1){}
		string & toString(string & str)const
		{
			char buf[2048];
			sprintf(buf,"head_FLRU %ld , head_MLRU %ld",head_FLRU,head_MLRU);
			return str=buf;
		}
	};

	template<typename T >
	struct T_DATA_LRU : public CActiveObjectBase
	{
		T_SHM_SIZE handle;
		T_SHM_SIZE next_FLRU; 
		T_SHM_SIZE next_MLRU; 
		T data;

		bool operator < (T_DATA_LRU const & tmp)const{return handle<tmp.handle;}
		string & toString(string & str)const
		{
			char buf[2048];
			sprintf(buf,"h=%ld next_FLRU=%ld data=",handle,next_FLRU);
			return str=buf+data.toString(str);
		}
	};
	
	template<typename T,int PI_N >
	struct T_HANDLE_LRU
	{
		typedef T_DATA_LRU<T > T_DATA_BUFFER;//缓存结构
		typedef T_SHMSET_NO_MUTEX<T_DATA_BUFFER,PI_N,T_USER_HEAD_LRU > T_SET_LRU_BUFFER;//缓存容器

		T_SHM_SIZE handle;
		T_HANDLE_LRU(T_SHM_SIZE h=-1):handle(h){}
		T_HANDLE_LRU & operator ++ (){++handle;return *this;}
		T_HANDLE_LRU & operator = (T_HANDLE_LRU const & tmp){handle=tmp.handle;return *this;}
		bool operator == (T_HANDLE_LRU const & tmp)const{return handle==tmp.handle;}
		bool operator != (T_HANDLE_LRU const & tmp)const{return !((*this)==tmp);}
		T & operator * ()const
		{
			return * operator ->();
		}
		T * operator -> ()const
		{
			if(0==PI_N)throw "SHM PI_N=0";

			//thelog<<"进入T_HANDLE_LRU -> handle="<<handle<<endi;

			T_SET_LRU_BUFFER * pLRUBuffer=(T_SET_LRU_BUFFER *)GET_PP_LRU(PI_N);
			typename T_SET_LRU_BUFFER::const_iterator it;
			T_DATA_BUFFER tmp;
			tmp.handle=handle;
			string str;

			//pLRUBuffer->Report();
			it=pLRUBuffer->find(tmp);
			if(it==pLRUBuffer->end())
			{
				if(pLRUBuffer->capacity()<=pLRUBuffer->size())
				{
					STATIC_G bool exchange=false;
					if(exchange)
					{
						thelog<<"LRU置换重入"<<endi;
						throw "LRU置换重入";
					}
					else exchange=true;

					//throw exception_my(__FILE__,__LINE__,"空间已满，需要LRU置换");
					//thelog<<"空间已满，需要LRU置换 "<<handle<<endi;
					//pLRUBuffer->Report();
					//找最久没有访问的并替换出去
					T_SHM_SIZE flru=pLRUBuffer->GetUserHead()->head_FLRU;
					it.handle=flru;
					while(it->next_FLRU>=0)
					{
						flru=it.handle;
						it.handle=it->next_FLRU;
					}
					//现在it指向最久没有访问的节点，flru指向前一个
					//thelog<<"最久没有被访问的是　"<<it.handle<<" ；逻辑节点 "<<it->handle<<endi;

					CStdOSFile f;
					string file;

					file=pLRUBuffer->GetName();
					file+=".lru";
					if(!f.OpenRW(file.c_str()))
					{
						thelog<<"打开文件失败 "<<file<<ende;
						throw exception_my(__FILE__,__LINE__,"打开文件失败");
					}
					f.SeekEnd();
					long f_length=f.Tell();
					long pos=LRU_HEAD_VIRTUAL_SET_HEAD_SIZE+it->handle*sizeof(T);
					if(pos>=f_length)
					{
						//补足文件长度
						for(long tmpi=f_length;tmpi<pos;++tmpi)f.Write("\0",1);
					}
					//将最久没有访问的那个写出去并从共享内存删除
					f.SeekBegin(pos);
					if(!f.Write(&it->data,sizeof(T)))throw exception_my(__FILE__,__LINE__,"写文件失败");
					//thelog<<it->data.toString(str)<<endi;
					pLRUBuffer->erase(it);

					f.Close();

					it.handle=flru;
					it->next_FLRU=-1;//pLRUBuffer->Report();

					exchange=false;
				}
				//已经确保有了一个可用空间，检查是否节点已经在文件中，若在则读入
				{
					CStdOSFile f;
					string file;

					file=pLRUBuffer->GetName();
					file+=".lru";
					if(!f.OpenRW(file.c_str()))
					{
						thelog<<"打开文件失败 "<<file<<ende;
						throw exception_my(__FILE__,__LINE__,"打开文件失败");
					}
					f.SeekEnd();
					long f_length=f.Tell();
					long pos=LRU_HEAD_VIRTUAL_SET_HEAD_SIZE+handle*sizeof(T);
					if(pos<f_length)
					{
						//将文件中的内容读入
						f.SeekBegin(pos);
						if(!f.Read((char *)&tmp.data,sizeof(T)))throw exception_my(__FILE__,__LINE__,"读文件失败");
						//thelog<<tmp.data.toString(str)<<endi;
					}

					f.Close();
					
					tmp.next_FLRU=pLRUBuffer->GetUserHead()->head_FLRU;
					it=pLRUBuffer->insert(tmp).first;
					pLRUBuffer->GetUserHead()->head_FLRU=it.handle;
					//pLRUBuffer->Report();
				}
			}
			if(it!=pLRUBuffer->end())
			{
				return &it->data;
			}
			else
			{
				return NULL;
			}
		}
		static T_SHM_SIZE _me(T const * p)
		{
			T_DATA_BUFFER tmp;
			T_DATA_BUFFER * plru;
			plru=(T_DATA_BUFFER *)((char *)p-((char *)&(tmp.data)-(char *)&tmp));
			return plru->handle;
		}
		static void ShowVMapPrivateData()
		{
			thelog<<"ShowVMapPrivateData未定义"<<ende;
		}
	};

	template<typename T_DATA,int PI_N,typename T_USER_HEAD=CDemoData,int VER=0 >
	class T_SHMLRU : public T_SHMSET<T_DATA,PI_N,T_USER_HEAD,0,VER,T_HANDLE_LRU<T_TREE_NODE_STRUCT<T_DATA > ,PI_N > >
	{
	public:
		typename T_HANDLE_LRU<T_TREE_NODE_STRUCT<T_DATA > ,PI_N >::T_SET_LRU_BUFFER m_LRUBuffer;
		typedef T_SHMSET<T_DATA,PI_N,T_USER_HEAD,0,VER,T_HANDLE_LRU<T_TREE_NODE_STRUCT<T_DATA > ,PI_N > > T_SET_PARENT;//父类型
		typedef typename T_SET_PARENT::iterator iterator;
	public:
		using T_SET_PARENT::capacity;
		using T_SET_PARENT::size;
		using T_SET_PARENT::GetName;

		T_SHMLRU(char const * name, int version) :T_SET_PARENT((string(name) + "_V").c_str(), version), m_LRUBuffer(name, version)
		{
			GET_PP_LRU(PI_N)=&m_LRUBuffer;
		}
		T_SHMLRU(char const * name):T_SET_PARENT((string(name)+"_V").c_str(),VER),m_LRUBuffer(name,VER)
		{
			GET_PP_LRU(PI_N)=&m_LRUBuffer;
		}
		pair<iterator, bool> insert(T_DATA const & data)
		{
			if(capacity()==size())
			{
				SetVirtualCapacity(capacity()+1);
			}
			return T_SET_PARENT::insert(data);
		}

	public://IShmActiveObject
		virtual bool CreateShm()
		{
			if(sizeof(typename T_SET_PARENT::TREE_HEAD)>LRU_HEAD_VIRTUAL_SET_HEAD_SIZE)
			{
				thelog<<"LRU_HEAD_VIRTUAL_SET_HEAD_SIZE "<<LRU_HEAD_VIRTUAL_SET_HEAD_SIZE<<" 不足，至少需要 "<<sizeof(typename T_SET_PARENT::TREE_HEAD)<<ende;
				return false;
			}
			bool ret=m_LRUBuffer.CreateShm() && m_LRUBuffer.Attach(false) && T_SET_PARENT::set_CreateShm_virtual(m_LRUBuffer.GetUserHead()->vitrual_set_head);
			if(!ret)return ret;

			//保存数据头
			CStdOSFile f;
			string file;

			file=m_LRUBuffer.GetName();
			file+=".lru";
			if(!f.OpenW(file.c_str()))
			{
				thelog<<"打开文件失败 "<<file<<ende;
				return false;
			}
			if(LRU_HEAD_VIRTUAL_SET_HEAD_SIZE!=f.Write((char *)T_SET_PARENT::GetTreeHead(),LRU_HEAD_VIRTUAL_SET_HEAD_SIZE))
			{
				thelog<<"写文件头失败 "<<file<<ende;
				return false;
			}

			f.Close();

			m_LRUBuffer.Detach();
			return true;
		}
		virtual bool _Attach(bool isReadOnly)
		{
			return m_LRUBuffer.Attach(isReadOnly) && T_SET_PARENT::set_AttachToShm_virtual(m_LRUBuffer.GetUserHead()->vitrual_set_head);
		}
		virtual bool Detach()
		{
			return m_LRUBuffer.Detach();
		}
		virtual bool LoadFromDir(char const * dir_name)
		{
			thelog<<GetName()<<" IShmActiveObject::LoadFromDir 尚未支持"<<ende;
			return false;
		}
		virtual bool SaveToDir(char const * dir_name)const
		{
			thelog<<GetName()<<" IShmActiveObject::SaveToDir 尚未支持"<<ende;
			return false;
		}
		virtual bool Report()const
		{
			string str;
			thelog<<T_SET_PARENT::Report_virtual(str)<<endi;
			return  m_LRUBuffer.Report();
		}
	};

}
