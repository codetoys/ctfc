//config.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//
#pragma once

// choose target platform: _WINDOWS | _SUNOS | _HPOS | _LINUXOS
// choose target compiler: _MS_VC | _SUN_CC | _HP_ACC | _LINUX_CC

//系统头文件
#ifdef _WINDOWS
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include <fcntl.h>

//C头文件<c*>==<*.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

//C++标准头文件
#include <string>
#include <list>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <algorithm>
#include <numeric>
#include <stdexcept>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <streambuf>
#ifndef _IBM_XLC
#define strstream stringstream
#define ostrstream ostringstream
#endif

//引入错误码定义文件
#include "std_error_code.h"

using namespace std;
using namespace ns_my_std_interface2;

//STATIC用于静态变量以便快速查找所有全局数据
#define STATIC static

//STATIC_C用于定义静态常量，静态常量不涉及线程安全（STATIC_C本身并未包含const）
#define STATIC_C static

//STATIC_G用于定义经过确认的作为全局变量使用的变量，跨线程使用（典型用作统计变量）
#define STATIC_G static

//需要补充的定义
#ifdef _LINUX_CC
#define sprintf_s snprintf
inline ostream& operator<<(ostream& os, const string& tmp)
{
	return os << tmp.c_str();
}
#endif
#ifdef _IBM_XLC
inline ostream& operator<<(ostream& os, const string& tmp)
{
	return os << tmp.c_str();
}
#endif

//All known versions of AIX lack WCOREDUMP but this works
#ifdef _IBMOS
#define WCOREDUMP(x) ((x) & 0x80)
#endif

#ifndef stricmp
#define stricmp(x,y) strcasecmp(x,y)
#endif
#ifndef strnicmp
#define strnicmp(x,y,n) strncasecmp(x,y,n)
#endif

#ifdef _WINDOWS
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define STDIN_FILENO _fileno(stdin)
#define popen _popen
#define pclose _pclose
#include <direct.h>
#include <process.h>
#define getpid _getpid
#define getcwd _getcwd
#else
#endif

//HP机内存问题，替换掉标准的vector
#define CMyVector vector
//#define CMyVector _CMyVector

//测试内存问题用的新set
#define CMySet set
//#define CMySet _CMySet

namespace ns_my_std
{
	//不要直接使用这个模板，应该使用CMyVector宏以便于统一切换
	template <typename T_DATA>
	class _CMyVector
	{
	public:
		typedef _CMyVector TYPE_ME;
		typedef T_DATA * iterator;
		typedef T_DATA const * const_iterator;
		typedef long size_type;
	private:
		T_DATA * pDataArray;
		long arraysize;
		long datacount;

		void _constructdata(long from, long n)
		{
			long i;
			for (i = 0; i < n; ++i)new(&pDataArray[from + i]) T_DATA();
		}
		void _remove(long pos)
		{
			long i;
			for (i = pos + 1; i < datacount; ++i)pDataArray[i - 1] = pDataArray[i];
			pDataArray[datacount - 1].~T_DATA();
			--datacount;
		}
		void _reserve(size_type _Newsize, bool _more = true)
		{
			if (_Newsize <= arraysize)return;

			long n;
			T_DATA * tmpP = pDataArray;

			if (_more)
			{
				if (1 == _Newsize)n = _Newsize;
				else if (_Newsize < 20)n = _Newsize * 2;
				else n = _Newsize*1.5;
			}
			else
			{
				n = _Newsize;
			}

			pDataArray = (T_DATA *) new char[sizeof(T_DATA)*n];
			if (NULL == pDataArray)
			{
				throw "内存不足";
				return;
			}
			if (NULL != tmpP)
			{
				int i;
				for (i = 0; i < datacount; ++i)new (&pDataArray[i]) T_DATA(tmpP[i]);
				delete[](char *)tmpP;
			}
			arraysize = n;
			return;
		}
	public:
		_CMyVector() :pDataArray(NULL), arraysize(0), datacount(0){}
		_CMyVector(TYPE_ME const & tmp) :pDataArray(NULL), arraysize(0), datacount(0){ *this = tmp; }
		_CMyVector(size_type n, T_DATA const & tmp) :pDataArray(NULL), arraysize(0), datacount(0)
		{
			size_type i;
			for (i = 0; i < n; ++i)
			{
				push_back(tmp);
			}
		}
		~_CMyVector()
		{
			clear();
			if (NULL != pDataArray)delete[](char *)pDataArray;
			pDataArray = NULL;
			arraysize = 0;
			datacount = 0;
		}
		void clear()
		{
			long i;
			for (i = 0; i < datacount; ++i)pDataArray[i].~T_DATA();
			datacount = 0;
		}
		T_DATA & operator[](long index){ return *(pDataArray + index); }
		T_DATA const & operator[](long index)const{ return *(pDataArray + index); }
		long size()const{ return datacount; }
		long capacity()const{ return arraysize; }
		iterator begin(){ return pDataArray; }
		iterator end(){ return pDataArray + datacount; }
		const_iterator begin()const{ return pDataArray; }
		const_iterator end()const{ return pDataArray + datacount; }
		void insert(iterator it, size_type n, T_DATA const & tmp)
		{
			long pos = it - begin();
			long i;
			_reserve(datacount + n);
			for (i = datacount - 1; i >= pos; --i)
			{
				new (&pDataArray[i + n]) T_DATA(pDataArray[i]);
				pDataArray[i].~T_DATA();
			}
			for (i = 0; i < n; ++i)
			{
				new (&pDataArray[pos + i]) T_DATA(tmp);
			}
			datacount += n;
		}
		void insert(iterator it, iterator it_begin, iterator it_end)
		{
			long pos = it - begin();
			long n = it_end - it_begin;
			long i;
			_reserve(datacount + n);
			for (i = datacount - 1; i >= pos; --i)
			{
				new (&pDataArray[i + n]) T_DATA(pDataArray[i]);
				pDataArray[i].~T_DATA();
			}
			for (i = 0; i < n; ++i)
			{
				new (&pDataArray[pos + i]) T_DATA(*(it_begin + i));
			}
			datacount += n;
		}
		void insert(iterator it, T_DATA const & val)
		{
			insert(it, 1, val);
		}
		void resize(size_type _Newsize)
		{
			_reserve(_Newsize, false);
			if (_Newsize > datacount)
			{
				_constructdata(datacount, _Newsize - datacount);
				datacount = _Newsize;
			}
			else if (_Newsize < datacount)
			{
				long i;
				for (i = datacount - 1; i >= _Newsize; --i)
				{
					_remove(i);
				}
				datacount = _Newsize;
			}
			else
			{
			}
		}
		void reserve(size_type _Newsize)
		{
			_reserve(_Newsize, false);
		}
		void push_back(T_DATA const & val)
		{
			_reserve(datacount + 1);
			++datacount;
			new(&pDataArray[datacount - 1]) T_DATA(val);
			return;
		}
		void erase(iterator it)
		{
			_remove(it - begin());
		}
		void erase(T_DATA const & key)
		{
			long i = 0;
			long count = 0;
			while (i < datacount)
			{
				if ((pDataArray)[i] == key)
				{
					_remove(i);
					++count;
					continue;//不可++i
				}
				++i;
			}
			//return count;
		}
		TYPE_ME & operator=(TYPE_ME const & tmp)
		{
			if (this == &tmp)return *this;

			int i;
			clear();
			_reserve(tmp.datacount, false);
			for (i = 0; i < tmp.datacount; ++i)
			{
				new (&pDataArray[i]) T_DATA(tmp.pDataArray[i]);
			}
			datacount = tmp.datacount;
			return *this;
		}
	};

	//set模板
	//template <typename T_DATA>
	class _CMySet
	{
	public:
		typedef long T_DATA;
		typedef _CMySet TYPE_ME;
		typedef long size_type;
		struct NODE
		{
			NODE * pParent;
			NODE * pLeft;
			T_DATA data;
			NODE * pRight;

			NODE(NODE * parent, T_DATA const & tmp) :pParent(parent), pLeft(NULL), data(tmp), pRight(NULL){}
			NODE(NODE * parent, NODE const & tmp) :pParent(parent), data(tmp.data)
			{
				if (NULL == tmp.pLeft)
				{
					pLeft = NULL;
				}
				else
				{
					pLeft = new NODE(this, *tmp.pLeft);
					if (NULL == pLeft)throw "内存不足";
				}
				if (NULL == tmp.pRight)
				{
					pRight = NULL;
				}
				else
				{
					pRight = new NODE(this, *tmp.pRight);
					if (NULL == pRight)throw "内存不足";
				}
			}
			~NODE()
			{
				if (NULL != pLeft)
				{
					delete pLeft;
					pLeft = NULL;
				}
				if (NULL != pRight)
				{
					delete pRight;
					pRight = NULL;
				}
			}
			NODE * _begin()
			{
				if (NULL == pLeft)return this;
				return pLeft->_begin();
			}
			bool isRight()const
			{
				return NULL != pParent && this == pParent->pRight;
			}
		};
		struct iterator
		{
			NODE * p;

			T_DATA const & operator * ()
			{
				return p->data;
			}
			iterator & operator ++ ()
			{
				if (NULL != p->pRight)
				{//存在右子树，取右子树的begin
					p = p->pRight->_begin();
				}
				else if (NULL != p->pParent)
				{//存在父节点
					if (p->isRight())
					{//是父节点的右子树，向上找到是左子树的节点，取这个节点的父节点
						while ((p = p->pParent) != NULL && p->isRight());
						if (NULL != p->pParent)p = p->pParent;
					}
					else
					{//是父节点的左子树，取父节点
						p = p->pParent;
					}
				}
				else
				{//根节点且没有右子树，结束
					p = NULL;
				}
				return *this;
			}
		};
		typedef iterator const_iterator;
	private:
		NODE * _pHead;
		size_type _size;
		long _deep;
	public:
		_CMySet() :_pHead(NULL), _size(0), _deep(0){}
		~_CMySet(){ clear(); }
		long size()const{ return _size; }
		const_iterator begin()const
		{
			const_iterator it;
			if (NULL == _pHead)it.p = NULL;
			else it.p = _pHead->_begin();
			return it;
		}
		const_iterator end()const
		{
			const_iterator it;
			it.p = NULL;
			return it;
		}
		void clear()
		{
			if (NULL != _pHead)delete _pHead;
			_pHead = NULL;
			_size = 0;
			_deep = 0;
		}
		const_iterator find(T_DATA const & tmp)const
		{
			const_iterator it;

			NODE * p = _pHead;
			while (NULL != p)
			{
				if (tmp < p->data)
				{
					p = p->pLeft;
				}
				else if (p->data < tmp)
				{
					p = p->pRight;
				}
				else
				{
					break;
				}
			}

			it.p = p;
			return it;
		}
		pair<iterator, bool> insert(T_DATA const & tmp)
		{
			pair<iterator, bool> tmppair;
			NODE * parent = NULL;
			NODE ** pp = &_pHead;
			long deep = 0;

			while (NULL != *pp)
			{
				++deep;
				if (tmp < (*pp)->data)
				{
					parent = *pp;
					pp = &(*pp)->pLeft;
				}
				else if ((*pp)->data < tmp)
				{
					parent = *pp;
					pp = &(*pp)->pRight;
				}
				else
				{
					tmppair.first.p = *pp;
					tmppair.second = false;
					return tmppair;
				}
			}
			(*pp) = new NODE(parent, tmp);
			if (NULL == (*pp))throw "内存不足";
			tmppair.first.p = *pp;
			tmppair.second = true;

			if (deep > _deep)_deep = deep;
			++_size;
			return tmppair;
		}
	};
}

//实现唯一实例，用classname::getInstPtr()获得唯一实例指针
//该实现利用了CActiveAppEnv类
#define DECLARE_SINGLETON(ClassName) \
public: \
	char const * GetClassName()const{return #ClassName;}\
    static ClassName* getInstPtr() { \
		STATIC ClassName * p=NULL;\
		if(NULL==p)\
		{\
			CActiveAppEnv::T_SINGLETONS::iterator it=g_pActiveAppEnv->mapSingleton.find(#ClassName);\
			if(it==g_pActiveAppEnv->mapSingleton.end())\
			{\
				g_pActiveAppEnv->mapSingleton[#ClassName]=new ClassName;\
			}\
			p=(ClassName *)g_pActiveAppEnv->mapSingleton[#ClassName];\
		}\
        return p; \
    } \
private: \
    static void operator delete(void *, size_t) { \
         throw -1; \
    }//End of define DECLARE_SINGLETON(ClassName);

