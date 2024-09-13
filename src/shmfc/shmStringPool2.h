//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmSet.h"

namespace ns_my_std
{
	//字符串池，模板参数T用于区分不同用途的池
	template<typename T, int PI_N, int PI_N2, typename T_USER_HEAD, int PART = 0 >
	class StringPool2x : public CShmActiveObjects
	{
	public:
		class StringPoolBuffer : public T_ARRAY<char,PI_N,T_USER_HEAD,PART >
		{
		public:
			typedef T_ARRAY<char,PI_N,T_USER_HEAD,PART > T_PARENT;
			typedef typename T_PARENT::HANDLE HANDLE;
			using T_PARENT::Get;
			using T_PARENT::Add;
			using T_PARENT::Adds_block;
		public:
			StringPoolBuffer(char const * name,int version=0):T_PARENT(name,version){}
			bool AddString(char const * str,HANDLE & h)
			{
				//加入字符串
				if(Adds_block(str,strlen(str)+1,h))
				{
					return true;
				}
				else return false;
			}
			char const * GetString(HANDLE const & h)const{return Get(h);}
		};
		struct struct_StringPoolIndex : public CActiveObjectBase
		{
			typename StringPoolBuffer::HANDLE hToStringPool;

			class Comp
			{
			public:
				bool operator () (struct_StringPoolIndex const & me,struct_StringPoolIndex const & tmp)const{return strcmp(&*me.hToStringPool,&*tmp.hToStringPool)<0;}
				bool operator () (struct_StringPoolIndex const & me,char const * str)const{return strcmp(&*me.hToStringPool,str)<0;}
				bool operator () (char const * str,struct_StringPoolIndex const & me)const{return strcmp(str,&*me.hToStringPool)<0;}
			};
			string & toString(string & str)const
			{
				char buf[2048];
				sprintf(buf,"%ld : ",hToStringPool.handle);
				str=buf;
				str+=&*hToStringPool;
				return str;
			}
		};
		typedef T_SHMSET_NO_MUTEX<struct_StringPoolIndex, PI_N2, CDemoData, PART, 0, typename struct_StringPoolIndex::Comp > StringPoolIndex;
		typedef typename StringPoolBuffer::HANDLE HANDLE;
	private:
		StringPoolBuffer m_StringPoolBuffer;
		StringPoolIndex m_StringPoolIndex;
	public:
		//IShmActiveObject接口
		virtual char const * GetName()const{return m_StringPoolBuffer.GetName();}
		StringPool2x(char const * name,int version=0):m_StringPoolBuffer(name,version),m_StringPoolIndex((string(name)+string("#")).c_str(),version)
		{
			if(!AddTable(&m_StringPoolBuffer))throw "内存不足";
			if(!AddTable(&m_StringPoolIndex))throw "内存不足";
		}
		long Capacity()const{return m_StringPoolBuffer.Capacity();}
		void RebuildCache(){}
		void ClearCache(){}
		virtual bool disableMutex()const{return true;}
		virtual bool Report()const
		{
			typename StringPoolIndex::const_iterator it;
			char buf[2048];
			
			StringPoolBuffer::HANDLE::ShowVMapPrivateData();
			StringPoolIndex::T_ARRAY_HANDLE::ShowVMapPrivateData();
			thelog<<endl<<m_StringPoolBuffer.GetName()<<" 开始报告字符串池"<<" ......"<<endl;
			sprintf(buf,"pHead=%p isPrivate=%d",m_StringPoolBuffer.GetHead(),(m_StringPoolBuffer.IsPrivate()?1:0));
			theLog<<buf<<endl;
			if(NULL!=m_StringPoolBuffer.GetHead())
			{
				sprintf(buf,"POOL name=%s capacity=%ld,size=%ld(%.2f%%)",m_StringPoolBuffer.GetName(),m_StringPoolBuffer.Capacity(),m_StringPoolBuffer.Size(),100.*m_StringPoolBuffer.Size()/m_StringPoolBuffer.Capacity());
				theLog<<buf<<endl;
				sprintf(buf,"INDEX name=%s capacity=%ld,size=%ld(%.2f%%)",m_StringPoolIndex.GetName(),(long)(m_StringPoolIndex.capacity()),(long)(m_StringPoolIndex.size()),100.*m_StringPoolIndex.size()/m_StringPoolIndex.capacity());
				theLog<<buf<<endl<<endl;
				long count=0;
				for(it=m_StringPoolIndex.begin();count<10 && it!=m_StringPoolIndex.end();++it,++count)
				{
					theLog<<&*(*it).hToStringPool<<endl;
				}
				if(m_StringPoolIndex.size()>11)theLog<<"......"<<endl;
				if(m_StringPoolIndex.size()>10)theLog<<"最后一个["<<&*(*m_StringPoolIndex.rbegin()).hToStringPool<<"]"<<endl;
			}
			theLog<<"报告完毕"<<endi;
			return true;
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			thelog<<"字符串池不需要导出文本"<<endi;
			return true;
		}
		//加入字符串，如果已经存在isExist返回true
		bool AddString(char const * str,HANDLE & h,long * pCountExist=NULL)
		{
			//先检查是否已经存在
			typename StringPoolIndex::iterator it=m_StringPoolIndex.find(str);

			if(it!=m_StringPoolIndex.end())
			{
				h=it->hToStringPool;
				if (NULL != pCountExist)++*pCountExist;
				return true;
			}

			if(m_StringPoolBuffer.AddString(str,h))
			{
				struct_StringPoolIndex tmpindex;
				tmpindex.hToStringPool=h;
				m_StringPoolIndex.insert(tmpindex);
				return true;
			}
			else
			{
				return false;
			}
		}
		//不使用索引直接加入，快速但是需要空间
		bool AddStringDirect(char const * str,HANDLE & h,long * pCountExist=NULL)
		{
			return m_StringPoolBuffer.AddString(str, h);
		}
		char const * GetString(HANDLE const & h)const{return m_StringPoolBuffer.GetString(h);}
		virtual bool ToDo(char const * what)
		{
			if(0==strcmp("StringPool2_CreatIndexFromBuffer",what))
			{
				if(!m_StringPoolBuffer.Attach(true))
				{
					thelog<<"连接缓冲池失败"<<ende;
					return false;
				}
				if(!m_StringPoolIndex.CreateShm())
				{
					thelog<<"创建共享内存失败"<<ende;
					return false;
				}
				if(!m_StringPoolIndex.Attach(false))
				{
					thelog<<"连接共享内存失败"<<ende;
					return false;
				}
				
				typename StringPoolBuffer::HANDLE h;
				struct_StringPoolIndex tmpindex;
				long count=0;
				long i;

				thelog<<"创建字符串索引......"<<endi;
				for(i=0;i<m_StringPoolBuffer.Size();i+=strlen(m_StringPoolBuffer.Get(i))+1)
				{
					++count;
					h.handle=i;
					tmpindex.hToStringPool=h;
					m_StringPoolIndex.insert(tmpindex);
				}
				thelog<<"创建字符串索引完成，共 "<<m_StringPoolIndex.size()<<" 个字符串（原始 "<<count<<" 个）"<<endi;
				if(!Detach())
				{
					thelog<<"断开共享内存失败"<<ende;
					return false;
				}
				return true;
			}
			else
			{
				return true;
			}
		}
		virtual bool FastRebuild_Start()
		{
			this->Detach();
			this->DestoryShm();
			this->CreateShm();
			return this->Attach(false);
		}
		virtual bool FastRebuild_Finish(char const* level, bool noDisk)
		{
			thelog << GetFullName() << " IShmActiveObject::FastRebuild_Finish 不需要操作" << endi;
			return true;
		}
		virtual bool repair(char const* level)
		{
			thelog << GetFullName() << " IShmActiveObject::repair 不需要操作" << endi;
			return true;
		}
	};
}
