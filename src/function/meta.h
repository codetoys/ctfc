//meta.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

//定义元数据协议

/********************************************************************

元数据协议规定共享的数据结构的元信息的表述方式
对于诸如共享内存这样的数据在访问数据前检查其数据格式是有必要的
共享的数据结构的定义的改变会导致无法正确访问共享数据
编译时的对齐方式的不同也会导致无法正确访问共享数据
因此在共享数据的头部放置以字节方式规定的数据元信息
由于元信息以字节方式定义，因此不存在格式混乱问题

元信息定义如下：

所有字节都是有符号的
整个元信息大小256字节
从偏移量0开始依次为：
 0 GUID 36字节 包含一个GUID字符串，注册表格式，例如“0AEFC2F4-A411-4c26-897E-F8C6E4DFCCD9”
			意味着一种数据的唯一标识
36 SIZEOFSHORT 1字节 short的字节数
37 SIZEOFINT 1字节 int的字节数
38 SIZEOFLONG 1字节 long的字节数
39 BYTEORDER 1字节 主机字节序，低位放在低地址为1，否则为0
40+n*4 USERINT数组 每个4字节 用户定义的整数，主机字节序，共54个，若未用则应置为0

********************************************************************/

#pragma once

namespace ns_my_std
{
	//元数据辅助类
	class CMeta
	{
		signed char data[256];//不要试图用int来冒充4个字节
	public:
		bool Set(signed char const * szGuid,int const * pInt,int nIntCount)
		{
			if(sizeof(int)!=4)
			{
				cout<<"fatal error : sizeof(int)!=4 , class Meta can not work . "<<endl;
				return false;
			}
			if(NULL==szGuid || NULL==pInt || strlen((char const *)szGuid)>36 || nIntCount>54)return false;
			memset(data,0,256);
			strcpy((char *)(data+0),(char const *)szGuid);
			data[36]=(signed char)sizeof(short);
			data[37]=(signed char)sizeof(int);
			data[38]=(signed char)sizeof(long);
			int tempint=1;
			data[39]=(signed char)*(signed char *)&tempint;
			memcpy(data+40,pInt,4L*nIntCount);
			return true;
		}
		string & toString(string & s)const
		{
			s="";
			if(sizeof(int)!=4)
			{
				s="fatal error : sizeof(int)!=4 , class Meta can not work . \n";
				return s;
			}
			char buf[1024];
			memcpy(buf,data,36);
			buf[36]='\0';
			s+="GUID = ";
			s+=buf;
			s+=" \n";
			sprintf(buf,"sizeof short= %d int= %d long= %d    byteorder= %d\n",data[36],data[37],data[38],data[39]);
			s+=buf;
			int i,k;
			k=0;
			for(i=0;i<54;++i)
			{
				int temp;
				memcpy(&temp,data+40+4*i,4);//注意，由于对齐问题，直接转换成int*可能会导致core dump！所以这样复制一次
				if(0!=temp)k=i;//找到最大的i
			}
			for(i=0;i<=k;++i)
			{
				int temp;
				memcpy(&temp,data+40+4*i,4);//注意，由于对齐问题，直接转换成int*可能会导致core dump！所以这样复制一次
				sprintf(buf,"%2d = %6d ",i,temp);
				s+=buf;
				if(0==(i+1)%6)s+="\n";
			}
			return s;
		}
		bool Compare(CMeta const& tmp, string & msg)const
		{
			if (0 != memcmp(data, tmp.data, 256))
			{
				stringstream ss;
				for (long i = 0; i < 256; ++i)
				{
					if (data[i] != tmp.data[i])
					{
						ss << "字节 " << i << " " << (int)(data[i]) << " " << (int)(tmp.data[i]) << endl;
					}
				}
				msg = ss.str();
				return false;
			}
			else
			{
				return true;
			}
		}
		bool CheckGuid(signed char const * szGuid)const
		{
			return 0 == memcmp(data, szGuid, 36);
		}
		bool CheckSys()const
		{
			if(data[36]!=(signed char)sizeof(short))return false;
			if(data[37]!=(signed char)sizeof(int))return false;
			if(data[38]!=(signed char)sizeof(long))return false;
			int tempint=1;
			if(data[39]!=(signed char)*(signed char *)&tempint)return false;
			return true;
		}
		int GetInt(int i)const
		{
			int ret;
			memcpy(&ret, data + 40 + i * sizeof(int), sizeof(int));
			return ret;
		}
		int GetIntCount()const
		{
			int i, k;
			k = -1;
			for (i = 0; i < 54; ++i)
			{
				int temp;
				memcpy(&temp, data + 40 + 4 * i, 4);//注意，由于对齐问题，直接转换成int*可能会导致core dump！所以这样复制一次
				if (0 != temp)k = i;//找到最大的i
			}
			return k + 1;
		}
		bool CopyTo(signed char * pTo)const
		{
			memcpy(pTo,data,256);
			return true;
		}
		bool CopyFrom(signed char const * pFrom)
		{
			memcpy(data,pFrom,256);
			return true;
		}
	};
}
