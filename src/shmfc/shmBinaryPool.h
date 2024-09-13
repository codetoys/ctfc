//shm_BinaryPool.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmArray.h"

#define BINARYPOOL_TRANCE (g_isShmPoolTrance)//输出调试信息
#define BINARYPOOL_Not_MakeShmData (!g_isMakeShmPoolData)//非构造共享内存数据，此时申请在私有内存进行

namespace ns_my_std
{
	//头信息，存储入口点
	class BinaryPoolEntry
	{
	public:
		long count_allocator;//分配的总数
		long count_deallocator;//释放的总数
		long entry_count;
		long max_entry_count;
		pair<sstring<32>, T_SHM_SIZE > name_handle[1024];//必须是max_entry_count
	public:
		BinaryPoolEntry() :count_allocator(0), count_deallocator(0), entry_count(0), max_entry_count(1024) {}//注意max_entry_count必须和定义一致
		long GetEntry(char const * name)const
		{
			long i;
			for (i = 0; i < entry_count; ++i)
			{
				if (name_handle[i].first == name)
				{
					return name_handle[i].second;
				}
			}
			return -1;
		}
		bool AddEntry(char const * name, long handle)
		{
			if (GetEntry(name) < 0)
			{
				if (entry_count == max_entry_count)
				{
					thelog << "入口点已满，无法存储" << ende;
					return false;
				}
				else
				{
					name_handle[entry_count].first = name;
					name_handle[entry_count].second = handle;
					++entry_count;
					return true;
				}
			}
			else
			{
				thelog << name << " 已经存在" << ende;
				return false;
			}
		}
		string & toString(string & ret)const
		{
			ret = "";

			char buf[256];
			long i;
			sprintf(buf, "总分配 %ld，总释放 %ld，最大入口点数 %ld，已用 %ld\n", count_allocator, count_deallocator, max_entry_count, entry_count);
			ret += buf;
			for (i = 0; i < entry_count; ++i)
			{
				sprintf(buf, "%ld : %32s %ld\n", i, name_handle[i].first.c_str(), name_handle[i].second);
				ret += buf;
			}
			return ret;
		}
	};
	//头信息，存储空闲链表
	class BinaryPoolFreeList
	{
		enum { FREE_LIST_SIZE = 100 };
		struct _FreeList
		{
			T_SHM_SIZE unit;//分配的单元大小，0为未用
			T_SHM_SIZE handle;
			long size;
			_FreeList() :unit(0), handle(-1), size(0) {}
		};
	public:
		_FreeList freelist[FREE_LIST_SIZE];//必须等于max_list
		long max_list;//必须等于freelist数组大小
		BinaryPoolFreeList() :max_list(FREE_LIST_SIZE) {}//必须等于freelist数组大小
		//计算空闲链表位置，第0个存储没有位置存的，其余存储8的倍数的
		void calcFreeHandle(long n, long & _freelist, long & newsize)
		{
			_freelist = 0;
			newsize = n;
			if (0 != newsize % 8)newsize = 8 * (newsize / 8 + 1);
			if (newsize < 8)newsize = 8;

			for (long i = 1; i < max_list; ++i)
			{
				if (freelist[i].unit == newsize)
				{
					_freelist = i;
					return;
				}
				if (0 == freelist[i].unit)
				{
					freelist[i].unit = newsize;
					_freelist = i;
					return;
				}
			}
		}
		string & toString(string & ret)const
		{
			ret = "";

			char buf[256];
			long i;
			sprintf(buf, "空闲链表数 %ld\n", max_list);
			ret += buf;
			for (i = 0; i < max_list; ++i)
			{
				if (0 != i && 0 == freelist[i].unit && -1 == freelist[i].handle)continue;
				sprintf(buf, "%ld : %ldB %ld 个，h=%ld\n", i, freelist[i].unit, freelist[i].size, freelist[i].handle);
				ret += buf;
			}
			return ret;
		}
	};
	class BinaryPoolHead
	{
	public:
		BinaryPoolEntry entry;
		BinaryPoolFreeList FreeList;
		string & toString(string & ret)const
		{
			string str;
			ret = "";
			ret += entry.toString(str);
			ret += "\n";
			ret += FreeList.toString(str);
			return ret;
		}
	};
	//共享内存池
	template<int PI_N>
	class BinaryPool : public T_ARRAY<char, PI_N, BinaryPoolHead >
	{
	private:
		using T_ARRAY<char, PI_N, BinaryPoolHead >::ReportHead;
		using T_ARRAY<char, PI_N, BinaryPoolHead >::GetHead;
		using T_ARRAY<char, PI_N, BinaryPoolHead >::size;
		using T_ARRAY<char, PI_N, BinaryPoolHead >::capacity;
		using T_ARRAY<char, PI_N, BinaryPoolHead >::Adds_block;
		using T_ARRAY<char, PI_N, BinaryPoolHead >::Add;
		using T_ARRAY<char, PI_N, BinaryPoolHead >::Reserve;
	public:
		using T_ARRAY<char, PI_N, BinaryPoolHead >::GetUserHead;
		typedef typename T_ARRAY<char, PI_N, BinaryPoolHead >::HANDLE HANDLE;
	public:
		BinaryPool(char const * name, int version = 0) :T_ARRAY<char, PI_N, BinaryPoolHead >(name, version) {}
		void Show(bool withhead = true, long startpos = 0, long maxcount = 32 * 8)const
		{
			string str;
			str = "";
			if (startpos < 0)
			{
				thelog << "错误的起始位置 " << startpos << " 默认为0" << ende;
				startpos = 0;
			}
			if (withhead)thelog << endl << ReportHead(str) << endl << " 开始报告共享内存池" << " ......" << endl;
			else thelog << endl;
			if (NULL != GetHead())
			{
				T_SHM_SIZE i;
				char const * last = NULL;
				HANDLE h;
				long tmplong;//记录一个long
				theLog << "起始位置 " << startpos << " 总大小 " << GetHead()->size << endl;
				for (i = startpos; i < GetHead()->size; ++i)
				{
					h.handle = i;
					last = this->Get(h);
					((char *)&tmplong)[i % 8] = *last;
					if (i < startpos + maxcount)
					{
						char buf[1024];
						if (0 == i % 8)
						{
							sprintf(buf, "%3ld :", i);
							theLog << buf;
						}
						char buf2[256];
						sprintf(buf2, "%02X", *last);
						if (strlen(buf2) > 2)memmove(buf2, buf2 + strlen(buf2) - 2, 3);//LINUX好好有趣哦
						sprintf(buf, " %s %4d %c", buf2, *last, (isprint(*last) ? *last : ' '));
						theLog << buf;
						if (0 == (i + 1) % 8)
						{
							str = "";
							for (long j = 0; j < 8; ++j)
							{
								unsigned char c = ((unsigned char *)&tmplong)[j];
								if (c >= ' ' && c < 127)
								{
									str += c;
								}
								else if (c > 127 && c < 255)
								{
									if (j < 7 && ((unsigned char *)&tmplong)[j + 1]>127)
									{
										str += c;
										str += ((unsigned char *)&tmplong)[j + 1];
										++j;
									}
									else
									{
										str += '.';
									}
								}
								else if (127 == c)
								{
									str += ".";
								}
								else
								{
									str += ".";
								}
							}
							theLog << " | " << str;
							sprintf(buf, " | %016lX %ld", tmplong, tmplong);
							theLog << buf << endl;
						}
					}
					else
					{
						break;
					}
				}
				theLog << "下一个位置 " << i << " 剩余字节数 " << GetHead()->size - i << endl;
			}
			theLog << endi;
		}
		void view()const
		{
			string cmd;
			long startpos = 0;
			long maxline = 32;
			while (true)
			{
				cmd = UIInput("共享内存显示 enter=继续显示 num=起始位置 max=设置每次显示的行数 b=break", "");
				if ("" == cmd)
				{
					if (startpos >= GetHead()->size)
					{
						thelog << "已经到达结尾" << endi;
						startpos = GetHead()->size;
					}
					Show((0 == startpos), startpos, maxline * 8);
					startpos += maxline * 8;
				}
				else if ("max" == cmd)
				{
					maxline = atol(UIInput("请输入行数", "").c_str());
					if (maxline < 8)
					{
						thelog << "输入不能小于8，已经设置为8" << ende;
						maxline = 8;
					}
					thelog << "maxline=" << maxline << endi;
				}
				else if ("b" == cmd)
				{
					break;
				}
				else
				{
					startpos = atol(cmd.c_str());
					if (0 != startpos % 8)
					{
						thelog << "起始位置必须是8的倍数" << ende;
						startpos = (startpos / 8) * 8;
						thelog << "已经重设为 " << startpos << endi;
					}
					Show((0 == startpos), startpos, maxline * 8);
					startpos += maxline * 8;
				}
			}
		}
		bool _Allocate(long n, HANDLE & h)
		{
			if (BINARYPOOL_Not_MakeShmData)
			{
				thelog << "仅可在构造共享内存池数据时调用" << ende;
				return false;
			}
			if (0 == size())
			{
				if (BINARYPOOL_TRANCE)thelog << "由于池为空，先申请8字节填充HANDLE为0的位置" << endi;
				HANDLE h;
				if (!Adds_block("NULLNULL", 8, h))
				{
					thelog << "内存不足" << ende;
					return false;
				}
			}
			else
			{
				if (BINARYPOOL_TRANCE)thelog << "池size " << size() << endi;
			}
			//检查剩余空间是否足够，若不够将剩余空间用0填充，避免一个字符串存储到两个块
			//共享内存增长大小应该确保大于最长的字符串
			T_SHM_SIZE i;
			HANDLE tmph;
			T_SHM_SIZE tmpn;//实际申请的大小
			long freelist;//空闲链表位置
			GetUserHead()->FreeList.calcFreeHandle(n, freelist, tmpn);
			if (BINARYPOOL_TRANCE)thelog << "n=" << n << " freelist=" << freelist << " newsize=" << tmpn << endi;
			//首先试图从空闲链表中分配
			BinaryPoolHead * pHead = GetUserHead();
			if (freelist != 0)
			{
				if (pHead->FreeList.freelist[freelist].size > 0)
				{
					h.handle = pHead->FreeList.freelist[freelist].handle;
					tmph.handle = pHead->FreeList.freelist[freelist].handle;
					pHead->FreeList.freelist[freelist].handle = *(long *)&*tmph;
					--pHead->FreeList.freelist[freelist].size;

					if (BINARYPOOL_TRANCE)thelog << "Allocate " << n << "(" << tmpn << ") " << h.handle << " from freelist" << endi;
					GetUserHead()->entry.count_allocator += n;
					return true;
				}
			}
			else
			{
			}

			//加入字符串--------------------------这一段有问题，考虑直接使用Adds_block
			if (static_cast<long>(capacity() - size()) < tmpn)
			{
				while (size() < capacity())Add('\0', tmph);
				if (!Reserve(size() + tmpn, tmpn))
				{
					thelog << "内存不足" << ende;
					return false;
				}
			}
			for (i = 0; i < tmpn; ++i)
			{
				if (!Add('\0', tmph))return false;
				if (0 == i)h = tmph;
			}
			if (BINARYPOOL_TRANCE)thelog << "Allocate " << n << "(" << tmpn << ") " << h.handle << endi;
			GetUserHead()->entry.count_allocator += n;
			return true;
		}
		bool Allocate(long n, HANDLE & h)
		{
			if (!_Allocate(n + sizeof(long), h))return false;
			*(long *)&*h = n;
			h.handle += sizeof(long);
			return true;
		}
		bool _Deallocate(HANDLE const & _h, long n)
		{
			HANDLE h = _h;
			BinaryPoolHead * pHead = GetUserHead();

			T_SHM_SIZE tmpn;//实际申请的大小
			long freelist;//空闲链表位置
			GetUserHead()->FreeList.calcFreeHandle(n, freelist, tmpn);
			if (BINARYPOOL_TRANCE)thelog << "Deallocate n=" << n << " freelist=" << freelist << " newsize=" << tmpn << " h=" << _h.handle << endi;
			for (long i = 0; i < tmpn; ++i, ++h)
			{
				*h = 0xE3;
			}

			if (0 != freelist)
			{
				*(long *)&*_h = pHead->FreeList.freelist[freelist].handle;
				pHead->FreeList.freelist[freelist].handle = _h.handle;
				++pHead->FreeList.freelist[freelist].size;
			}
			else
			{
				*(long *)&*_h = pHead->FreeList.freelist[freelist].handle;
				((long *)&*_h)[1] = tmpn;//下一个long存储长度
				pHead->FreeList.freelist[freelist].handle = _h.handle;
				++pHead->FreeList.freelist[freelist].size;
			}

			GetUserHead()->entry.count_deallocator += n;
			return true;
		}
		bool Deallocate(HANDLE const & _h, long n)
		{
			if (BINARYPOOL_Not_MakeShmData)
			{
				thelog << "仅可在构造共享内存池数据时调用" << ende;
				return false;
			}

			HANDLE h;
			h.handle = _h.handle - sizeof(long);
			if (n != *(long *)&*h)
			{
				throw "Deallocate n error";
			}
			return _Deallocate(h, *(long *)&*h + sizeof(long));
		}
		bool Deallocate(HANDLE const & _h)
		{
			if (BINARYPOOL_Not_MakeShmData)
			{
				thelog << "仅可在构造共享内存池数据时调用" << ende;
				return false;
			}

			HANDLE h;
			h.handle = _h.handle - sizeof(long);
			return _Deallocate(h, *(long *)&*h + sizeof(long));
		}
		long GetUserLen(HANDLE const & _h)
		{
			HANDLE h;
			h.handle = _h.handle - sizeof(long);
			return *(long *)&*h;
		}

		HANDLE GetEntry(char const * name)const
		{
			HANDLE h;
			h.handle = GetUserHead()->entry.GetEntry(name);
			return h;
		}
		bool AddEntry(char const * name, HANDLE handle)
		{
			if (BINARYPOOL_Not_MakeShmData)
			{
				thelog << "仅可在构造共享内存池数据时调用" << ende;
				return false;
			}
			return GetUserHead()->entry.AddEntry(name, handle.handle);
		}
		template<typename Y >
		Y * NEW()
		{
			HANDLE h;
			if (!Allocate(sizeof(Y), h))return NULL;
			else
			{
				new((Y*)&*h) Y;
				return (Y*)&*h;
			}
		}
		template<typename Y >
		Y * GetEntryWithNewIfNeed(char const * name)
		{
			HANDLE h = GetEntry(name);
			if (h.handle < 0)
			{
				thelog << "入口点 " << name << " 不存在，需要添加" << endi;
				if (!Allocate(sizeof(Y), h))return NULL;
				else
				{
					if (!AddEntry(name, h))
					{
						thelog << "添加入口点 " << name << " 出错" << ende;
						Deallocate(h, sizeof(Y));
						return NULL;
					}
					new((Y*)&*h) Y;
					return (Y*)&*h;
				}
			}
			else
			{
				return (Y*)&*h;
			}
		}
	public:
		virtual bool disableMutex()const { return true; }
		virtual bool Report()const
		{
			Show();
			return true;
		}
		virtual bool ExportTextToDir(char const * dir_name)const
		{
			thelog << "共享内存池不需要导出文本" << endi;
			return true;
		}
		virtual bool ToDo(char const * what)
		{
			if (NULL == what || '\0' == what[0])
			{
			}
			else if (0 == strcmp(what, "view"))
			{
				view();
			}
			else
			{
			}
			thelog << "ToDo: view" << endi;
			return true;
		}
	};

	typedef BinaryPool<PI_SHMPOOL > _ShmPool;

	class ShmPool : public _ShmPool
	{
		DECLARE_SINGLETON(ShmPool);
	public:
		ShmPool() :_ShmPool(SHM_NAME_SHMPOOL, 0) {}
	};

	template<typename T>
	class ShmAllocator
	{
	private:
	public:
		typedef T value_type;
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
		typedef T * pointer;
		typedef T const * const_pointer;
#else
		class shm_pointer
		{
		public:
#ifdef _LINUX_CC
			typedef random_access_iterator_tag iterator_category;
#endif

			typedef T * pointer;
			typedef T element_type;
			typedef T value_type;
			typedef long difference_type;
			typedef long offset_type;
			typedef T & reference;
			template<typename U>
			struct rebind : public ShmAllocator<U>::shm_pointer {};
		public:
			T_SHM_SIZE handle;
			shm_pointer() :handle(0) {}
			shm_pointer(ShmPool::HANDLE const & h) :handle(h.handle) {}
			shm_pointer(T * p)
			{
				if (BINARYPOOL_TRANCE)thelog << "shm_pointer " <<(unsigned long)p<< endi;
				if (NULL == p)handle = 0;
				else handle = ShmPool::HANDLE::_me((char *)p, true);
				if (handle < 0)throw "非法句柄";
			}
			//static shm_pointer pointer_to(T & tmp) { return const_shm_pointer(); }
			static shm_pointer pointer_to(T & tmp) { return shm_pointer(tmp); }

			shm_pointer operator + (long n)const { return shm_pointer(handle + n * sizeof(T)); }
			shm_pointer operator - (long n)const { return shm_pointer(handle - n * sizeof(T)); }
			bool operator < (shm_pointer const & tmp)const { return handle < tmp.handle; }
			T_SHM_SIZE operator - (shm_pointer const & tmp)const { return (handle - tmp.handle) / sizeof(T); }
			shm_pointer & operator += (T_SHM_SIZE n) { handle += n * sizeof(T); return *this; }
			shm_pointer & operator ++ () { operator +=(1); return *this; }
			shm_pointer & operator -- () { operator +=(-1); return *this; }
			shm_pointer & operator ++ (int) { shm_pointer tmp = *this; operator +=(1); return tmp; }
			shm_pointer & operator -- (int) { shm_pointer tmp = *this; operator +=(-1); return tmp; }
			shm_pointer & operator = (shm_pointer const & tmp) { handle = tmp.handle; return *this; }
			bool operator == (T const * p)const { return *this == shm_pointer((T *)p); }
			bool operator != (T const * p)const { return *this != shm_pointer((T *)p); }
			bool operator == (shm_pointer const & tmp)const { return handle == tmp.handle; }
			bool operator != (shm_pointer const & tmp)const { return !((*this) == tmp); }

			T & operator * ()const { return *operator ->(); }
			T * operator -> ()const { if (0 == handle)return NULL; else return (T *)&* ShmPool::HANDLE(handle); }
			operator T *() { return operator ->(); }
		};
		class const_shm_pointer
		{
		public:
#ifdef _LINUX_CC
			typedef random_access_iterator_tag iterator_category;
#endif

			typedef T const * pointer;
			typedef T const element_type;
			typedef T const value_type;
			typedef long difference_type;
			typedef long offset_type;
			typedef T const & reference;
			template<typename U>
			struct rebind : public ShmAllocator<U>::const_shm_pointer {};

		public:
			T_SHM_SIZE handle;
			const_shm_pointer() :handle(0) {}
			const_shm_pointer(ShmPool::HANDLE const & h) :handle(h.handle) {}
			const_shm_pointer(const_shm_pointer const & tmp) :handle(tmp.handle) {}
			const_shm_pointer(shm_pointer const & tmp) :handle(tmp.handle) {}
			const_shm_pointer(T const * p)
			{
				if (BINARYPOOL_TRANCE)thelog << "shm_pointer " << (unsigned long)p << endi;
				if (NULL == p)handle = 0;
				else handle = ShmPool::HANDLE::_me((char const *)p, true);
				if (handle < 0)throw "非法句柄";
			}
			//static pointer pointer_to(T const & tmp) { return pointer((T*)(void*)&tmp); }
			static const_shm_pointer pointer_to(T const & tmp) { return const_shm_pointer(tmp); }

			const_shm_pointer operator + (long n)const { return const_shm_pointer(handle + n * sizeof(T)); }
			const_shm_pointer operator - (long n)const { return const_shm_pointer(handle - n * sizeof(T)); }
			bool operator < (const_shm_pointer const & tmp)const { return handle < tmp.handle; }
			T_SHM_SIZE operator - (const_shm_pointer const & tmp)const { return (handle - tmp.handle) / sizeof(T); }
			const_shm_pointer & operator += (T_SHM_SIZE n) { handle += n * sizeof(T); return *this; }
			const_shm_pointer & operator ++ () { operator +=(1); return *this; }
			const_shm_pointer & operator -- () { operator +=(-1); return *this; }
			const_shm_pointer & operator ++ (int) { const_shm_pointer tmp = *this; operator +=(1); return tmp; }
			const_shm_pointer & operator -- (int) { const_shm_pointer tmp = *this; operator +=(-1); return tmp; }
			const_shm_pointer & operator = (const_shm_pointer const & tmp) { handle = tmp.handle; return *this; }
			bool operator == (T const * p)const { return *this == const_shm_pointer((T *)p); }
			bool operator != (T const * p)const { return *this != const_shm_pointer((T *)p); }
			bool operator == (const_shm_pointer const & tmp)const { return handle == tmp.handle; }
			bool operator != (const_shm_pointer const & tmp)const { return !((*this) == tmp); }

			T const & operator * ()const { return *operator ->(); }
			T const * operator -> ()const { if (0 == handle)return NULL; else return (T const *)&* ShmPool::HANDLE(handle); }
			operator T const *() { return operator ->(); }
		};
#endif
		typedef T & reference;
		typedef T const & const_reference;
		typedef ptrdiff_t difference_type;
		typedef shm_pointer pointer;
		typedef const_shm_pointer const_pointer;
		typedef long size_type;
		/*重绑定函数*/
		template<typename U>
		struct rebind
		{
			typedef ShmAllocator < U > other;
		};
		ShmAllocator() throw () {}
		ShmAllocator(ShmAllocator const &) throw () {}
		template<typename U>
		ShmAllocator(ShmAllocator < U > const &) throw () {}
		//若一个分配的空间可由另一个返还则相等
		template<typename U>
		bool operator == (ShmAllocator<U> const &) throw ()
		{
			thelog << "operator ==" << endi;
			return true;
		}
		/* 申请内存*/
		pointer allocate(size_type num, void * hint = 0)
		{
			if (BINARYPOOL_TRANCE)thelog << "申请 " << num << "  个内存块，sizeof(T)=" << sizeof(T) << endi;
			if (BINARYPOOL_Not_MakeShmData)
			{
				if (BINARYPOOL_TRANCE)thelog << "非构造共享内存池数据，由私有内存分配" << endi;
				std::allocator<T> stdallocator;
				return stdallocator.allocate(num, hint);
			}
			ShmPool::HANDLE h;
			if (!ShmPool::getInstPtr()->Allocate(num * sizeof(T), h))
			{
				thelog << "内存不足" << ende;
				return pointer(0);
			}
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
			pointer ptr((T *)(void *)&*h);
			if (BINARYPOOL_TRANCE)thelog << ptr << endi;
#else
			pointer ptr(h);
			if (BINARYPOOL_TRANCE)thelog << ptr.handle << endi;
#endif
			return ptr;
		}
		/* 释放内存 */
		void deallocate(pointer p, size_type num)
		{
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
			if (BINARYPOOL_TRANCE)thelog << "在 " << (long)p << " 释放 " << num << "  个内存块，sizeof(T)=" << sizeof(T) << endi;
#else
			if (BINARYPOOL_TRANCE)thelog << "在 " << p.handle << " 释放 " << num << "  个内存块，sizeof(T)=" << sizeof(T) << endi;
#endif
			if (BINARYPOOL_Not_MakeShmData)
			{
				if (BINARYPOOL_TRANCE)thelog << "非构造共享内存池数据，由私有内存释放" << endi;
				std::allocator<T> stdallocator;
				return stdallocator.deallocate(p, num);
			}
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
			if (NULL == p || 0 == num)
#else
			if (0 == p.handle || 0 == num)
#endif
			{
				//thelog<<"在 "<<(long)p<<" 释放 "<<num<<"  个内存块，sizeof(T)="<<sizeof(T)<<" 已忽略"<<endi;
				return;
			}
			ShmPool::HANDLE h;
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
			h.handle = ShmPool::HANDLE::_me((char const *)p, true);
#else
			h.handle = p.handle;
#endif
			if (!ShmPool::getInstPtr()->Deallocate(h, num * sizeof(T)))throw "释放内存失败";
		}
		size_type max_size() const throw()
		{
			if (BINARYPOOL_TRANCE)thelog << "max_size" << endi;
			return SIZE_MAX;
		}
		/* 使用placement new创建对象 */
		void construct(pointer p, const_reference value)
		{
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
			if (BINARYPOOL_TRANCE)thelog << "construct " << (long)p << endi;
#else
			if (BINARYPOOL_TRANCE)thelog << "construct " << p.handle << endi;
#endif
			new((void *)&*p) T(value); //placement new
		}
		/* 销毁对象 */
		void destroy(pointer p)
		{
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
			if (BINARYPOOL_TRANCE)thelog << "destroy " << (long)p << endi;
#else
			if (BINARYPOOL_TRANCE)thelog << "destroy " << p.handle << endi;
#endif
			((T*)(void *)&*p)->~T();
		}
		pointer address(reference value) const
		{
			if (BINARYPOOL_TRANCE)thelog << "address" << endi;
			return &value;
		}
		const_pointer address(const_reference value) const
		{
			if (BINARYPOOL_TRANCE)thelog << "address" << endi;
			return &value;
		}
		~ShmAllocator() throw () {}
	};

	typedef std::basic_string<char, std::char_traits<char>, ShmAllocator<char> > shm_string;
	template <typename T>
	class shm_vector : public vector<T, ShmAllocator<T > > {};
	template <typename T>
	class shm_set : public set<T, less<T >, ShmAllocator<T > > {};
	template <typename K, typename D>
	class shm_map : public map < K, D, less<K >, ShmAllocator<pair<const K, D> > > {};

	//inline ostream& operator<<(ostream& os, const shm_string& tmp)
	//{
	//	string str;
	//	str = tmp.c_str();
	//	return os << str;
	//}
}
