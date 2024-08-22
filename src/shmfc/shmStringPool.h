//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmArray.h"

namespace ns_my_std
{
	//字符串池，模板参数T用于区分不同用途的池
	template<typename T,int PI_N,typename T_USER_HEAD>
	class StringPool : public T_ARRAY<char,PI_N,T_USER_HEAD >
	{
	public:
		using T_ARRAY<char, PI_N, T_USER_HEAD >::GetHead;
		using T_ARRAY<char, PI_N, T_USER_HEAD >::Get;
		using T_ARRAY<char, PI_N, T_USER_HEAD >::IsPrivate;
		using T_ARRAY<char, PI_N, T_USER_HEAD >::Capacity;
		using T_ARRAY<char, PI_N, T_USER_HEAD >::Size;
	public:
		typedef typename T_ARRAY<char,PI_N,T_USER_HEAD >::HANDLE HANDLE;
	private:
		map<string,HANDLE> mapStringHandle;//缓存，用来

		HANDLE & FindString(char const * str,HANDLE & h)const
		{
			T_SHM_SIZE i;
			for(i=0,h.handle=0;i<GetHead()->size;i+=strlen(Get(i))+1)
			{
				if(0==strcmp(str,Get(i)))
				{
					h.handle=i;
					return h;
				}
			}
			h.handle=-1;
			return h;
		}
	public:
		StringPool(char const * name,int version=0):T_ARRAY<char,PI_N,T_USER_HEAD >(name,version){}
		void RebuildCache()
		{
			HANDLE h;
			long count=0;
			long i;

			thelog<<"重构字符串缓存......"<<endi;
			ClearCache();
			for(i=0;i<GetHead()->size;i+=strlen(Get(i))+1)
			{
				++count;
				h.handle=i;
				mapStringHandle[Get(i)]=h;
			}
			thelog<<"重构缓存完成，共 "<<mapStringHandle.size()<<" 个字符串"<<endi;
		}
		void ClearCache(){mapStringHandle.clear();}
		virtual bool disableMutex()const{return true;}
		virtual bool Report()const
		{
			string str;
			str="";
			char buf[2048];
			thelog<<endl<<GetHead()->name.c_str()<<" 开始报告字符串池"<<" ......"<<endl;
			sprintf(buf,"pHead=%p isPrivate=%d",GetHead(),(IsPrivate()?1:0));
			theLog<<buf<<endl;
			if(NULL!=GetHead())
			{
				sprintf(buf,"name=%s capacity=%ld,size=%ld(%.2f%%)\n",GetHead()->name.c_str(),Capacity(),Size(),100.*Size()/Capacity());
				theLog<<buf<<endl;
				T_SHM_SIZE i;
				long count=0;
				char const * last=NULL;
				HANDLE h;
				for(i=0;i<GetHead()->size;i+=strlen(Get(i))+1)
				{
					++count;
					h.handle=i;
					last=Get(h);
					if(count<10)theLog<<last<<endl;
					else if(10==count)theLog<<"......"<<endl;
					else continue;
				}
				if(NULL!=last)theLog<<"最后一个["<<last<<"]"<<endl;
				theLog<<"字符串总数 "<<count<<endl;
			}
			theLog<<"报告完毕"<<endi;
			return true;
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			thelog<<"字符串池不需要导出文本"<<endi;
			return true;
		}
		bool AddString(char const * str,HANDLE & h)
		{
			//先检查是否已经存在
			//FindString(str,h);
			typename map<string,HANDLE>::iterator it=mapStringHandle.find(str);
			if(it!=mapStringHandle.end())
			{
				h=it->second;
				return true;
			}

			//检查剩余空间是否足够，若不够将剩余空间用0填充，避免一个字符串存储到两个块
			//共享内存增长大小应该确保大于最长的字符串
			T_ARRAY<char,PI_N,T_USER_HEAD > * pArray=this;
			if(pArray->Capacity()-pArray->Size()<strlen(str)+1)
			{
				for(T_SHM_SIZE i=pArray->Size();i<pArray->Capacity();++i)
				{
					this->Add('\0',h);
				}
			}
			//加入字符串
			if(this->Adds(str,strlen(str)+1,h))
			{
				mapStringHandle[str]=h;
				return true;
			}
			else return false;
		}
		char const * GetString(HANDLE const & h)const{return Get(h);}
	};
}
