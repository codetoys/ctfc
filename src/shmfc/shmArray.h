//shmarray.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "shmstd.h"
#include "shmIActiveObject.h"
/*
此文件定义作为共享内存系统根基的数组和信号灯
T_ARRAY 共享内存数组，注册在数据库，实现IShmActiveObject接口
CManagedMutex 受管理的信号灯，注册在数据库
“#”开头的不需要配置
*/

namespace ns_my_std
{
	struct CActiveObjectBase
	{
		//用于需要排序的场合
		bool operator < (CActiveObjectBase const& tmp)const { throw "operator < undefined"; return false; }

		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { throw "keyhash undefined"; return 0; }

		//用于输出数据的场合
		string& toString(string& str)const { throw "toString undefined"; return str; }

		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table) { return false; }
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const { return false; }
	};
	//T_DATA范例
	struct CDemoData : public CActiveObjectBase
	{
		long n = 0;

		//用于需要排序的场合
		bool operator < (CDemoData const& tmp)const { return n < tmp.n; }
		//某些场合也需要等于
		bool operator == (CDemoData const& tmp)const { return n == tmp.n; }

		friend ostream& operator << (ostream& o, CDemoData const& d)
		{
			return o << d.n;
		}
		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return n; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "%ld", n);
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("N", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			table.AddData(n);
			return true;
		}
	};
	struct CShmChar : public CActiveObjectBase
	{
		char c;

		//用于需要排序的场合
		bool operator < (CShmChar const& tmp)const { return c < tmp.c; }

		//关键字的hash值，用于分块场合，应保证hash值的最后一部分仍然是平均分布的
		long keyhash()const { return c; }

		//用于输出数据的场合
		string& toString(string& str)const
		{
			char buf[2048];
			sprintf(buf, "%c", c);
			return str = buf;
		}
		//用于表格输出
		static bool AddTableColumns(CHtmlDoc::CHtmlTable2& table)
		{
			table.AddCol("N", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);
			return true;
		}
		bool AddTableData(CHtmlDoc::CHtmlTable2& table)const
		{
			string str;
			table.AddData(toString(str));
			return true;
		}
	};
	class CToString
	{
	public:
		string& operator()(long data, string& ret)
		{
			char buf[256];
			sprintf(buf, "%ld", data);
			return ret = buf;
		}
		template <typename T>
		string& operator()(T const& data, string& ret)
		{
			return data.toString(ret);
		}
	};

	//共享内存数组模板，所有共享内存类型的基础结构
	//T 元素类型
	//PI_N 预定义的整数，用于支持直接指针操作，若此参数为PI_NULL则不可以使用直接指针功能（此类及子类型的句柄或迭代器的*和->）
	//T_USER_HEAD 用户定义的数组头类型
	//PART 分块号1-10，非分块为0
	struct struct_T_ARRAY_VMAP
	{
		int shm_id;//共享内存ID，第一个总是和array_head在一起，其余块只有数据
		T_SHM_SIZE handle_begin;//此块对应的handle范围[handle_begin,handle_end）
		T_SHM_SIZE handle_end;
	};
	struct struct_T_ARRAY_VMAP_S
	{
		long size;//使用的个数
		struct_T_ARRAY_VMAP m_vmaps[T_ARRAY_VMAP_MAX_SIZE];

		void clearVMAP()
		{
			memset(this, 0, sizeof(struct_T_ARRAY_VMAP_S));
		}
		bool AddVMAP(int _shm_id, T_SHM_SIZE oldCapacity, T_SHM_SIZE blocksize)
		{
			if (size + 1 >= T_ARRAY_VMAP_MAX_SIZE)return false;
			m_vmaps[size].shm_id = _shm_id;
			m_vmaps[size].handle_begin = oldCapacity;
			m_vmaps[size].handle_end = oldCapacity + blocksize;
			++size;

			return true;
		}
	};
	template<typename T, int PI_N >
	struct T_HANDLE_ARRAY
	{
		T_SHM_SIZE handle;
		T_HANDLE_ARRAY(T_SHM_SIZE h = -1) :handle(h) {}
		T_HANDLE_ARRAY(T_HANDLE_ARRAY const& tmp) :handle(tmp.handle) {}

		typedef random_access_iterator_tag iterator_category;
		typedef T* pointer;
		typedef T element_type;
		typedef T value_type;
		typedef long difference_type;
		typedef long offset_type;
		typedef T& reference;

		bool operator<(T_HANDLE_ARRAY const& tmp)const { return handle < tmp.handle; }
		T_HANDLE_ARRAY operator + (long n)const
		{
			T_HANDLE_ARRAY tmp;
			tmp.handle = handle + n;
			return tmp;
		}
		T_HANDLE_ARRAY operator - (long n)const
		{
			T_HANDLE_ARRAY tmp;
			tmp.handle = handle - n;
			return tmp;
		}
		T_SHM_SIZE operator - (T_HANDLE_ARRAY const& tmp)const { return handle - tmp.handle; }
		T_HANDLE_ARRAY& operator += (T_SHM_SIZE n) { handle += n; return *this; }
		T_HANDLE_ARRAY& operator ++ () { ++handle; return *this; }
		T_HANDLE_ARRAY& operator -- () { --handle; return *this; }
		T_HANDLE_ARRAY& operator = (T_HANDLE_ARRAY const& tmp) { handle = tmp.handle; return *this; }
		bool operator == (T_HANDLE_ARRAY const& tmp)const { return handle == tmp.handle; }
		bool operator != (T_HANDLE_ARRAY const& tmp)const { return !((*this) == tmp); }
		T& operator * ()const
		{
			return *operator ->();
		}
		T* operator -> ()const
		{
			if (0 == PI_N)throw "SHM PI_N=0";

			struct_T_ARRAY_VMAP_S* pvmap = (struct_T_ARRAY_VMAP_S*)GET_PP_VMAP(PI_N);
			shm_private_data* paddrmap = &GET_SHM_PRIVATE_DATA(PI_N);
			for (long i = 0; i < pvmap->size; ++i)
			{
				if (pvmap->m_vmaps[i].handle_begin <= handle && handle < pvmap->m_vmaps[i].handle_end)
				{
					if (pvmap->size > paddrmap->addr_map_size)
					{
						if (0 != paddrmap->thread_mutex.lock())throw "paddrmap->thread_mutex.lock error";
						if (pvmap->size > paddrmap->addr_map_size)
						{
							for (long j = paddrmap->addr_map_size; j < pvmap->size; ++j)
							{
								char* p = CShmMan::ConnectByID(pvmap->m_vmaps[j].shm_id, false);
								if (NULL == p)
								{
									thelog << "连接共享内存失败 shmid = " << pvmap->m_vmaps[j].shm_id << " 错误信息：" << strerror(errno) << ende;
									throw "连接共享内存失败";
								}
								if (((unsigned long)p) % 8 != 0)
								{
									thelog << "地址对齐错误" << ende;
									throw "地址对齐错误";
								}
								char buf[256];
								sprintf(buf, "连接共享内存 %d %ld shm_id %d addr %p", PI_N, j, pvmap->m_vmaps[j].shm_id, p);
								thelog << buf << endi;
								paddrmap->AddShmMap(pvmap->m_vmaps[j].shm_id, p);
							}
						}
						else
						{
							thelog << "已经被其它线程处理" << endi;
						}
						if (0 != paddrmap->thread_mutex.unlock())throw "paddrmap->thread_mutex.unlock error";
					}
					return (T*)paddrmap->shm_addr_map[i].second + handle - pvmap->m_vmaps[i].handle_begin;
				}
				if (i == pvmap->size - 1 && handle == pvmap->m_vmaps[i].handle_end)
				{
					thelog << "刚好越过最后一个 " << handle << " " << (T*)paddrmap->shm_addr_map[i].second + handle - pvmap->m_vmaps[i].handle_begin << endi;
					//return (T *)paddrmap->shm_addr_map[i].second+handle-pvmap->m_vmaps[i].handle_begin;
				}
			}
			char buf[2048];
			sprintf(buf, "->无效的句柄 PI_N=%d handle=%ld", PI_N, handle);
			theLog << "shmArray.h T_HANDLE_ARRAY" << buf << ende;
			ShowVMapPrivateData();
			abort();
			return NULL;
		}
		static T_SHM_SIZE _me(T const* p, bool not_throw = false)
		{
			char buf[256];
			struct_T_ARRAY_VMAP_S* pvmap = (struct_T_ARRAY_VMAP_S*)GET_PP_VMAP(PI_N);
			shm_private_data* paddrmap = &GET_SHM_PRIVATE_DATA(PI_N);
			for (long i = 0; i < pvmap->size; ++i)
			{
				if ((T*)paddrmap->shm_addr_map[i].second <= p && p < (T*)paddrmap->shm_addr_map[i].second + pvmap->m_vmaps[i].handle_end - pvmap->m_vmaps[i].handle_begin)
				{
					return p - (T*)paddrmap->shm_addr_map[i].second + pvmap->m_vmaps[i].handle_begin;
				}
				if (i == pvmap->size - 1 && p == (T*)paddrmap->shm_addr_map[i].second + pvmap->m_vmaps[i].handle_end - pvmap->m_vmaps[i].handle_begin)
				{
					thelog << "刚好越过最后一个 " << p << " " << p - (T*)paddrmap->shm_addr_map[i].second + pvmap->m_vmaps[i].handle_begin << endi;
					//return p-(T *)paddrmap->shm_addr_map[i].second+pvmap->m_vmaps[i].handle_begin;
				}
			}
			if (not_throw)return -1;
			else
			{
				sprintf(buf, "_me无效的地址 %p", p);
				throw buf;
			}
		}
		static void ShowVMapPrivateData()
		{
			struct_T_ARRAY_VMAP_S* pvmap = (struct_T_ARRAY_VMAP_S*)GET_PP_VMAP(PI_N);
			shm_private_data* paddrmap = &GET_SHM_PRIVATE_DATA(PI_N);
			long i, j;
			theLog << endl;
			theLog << "pvmap->size=" << pvmap->size << endl;
			for (i = 0; i < pvmap->size; ++i)
			{
				theLog << i << " : begin=" << pvmap->m_vmaps[i].handle_begin << " end=" << pvmap->m_vmaps[i].handle_end << endl;
			}
			theLog << "paddrmap->addr_map_size=" << paddrmap->addr_map_size << endl;
			for (j = 0; j < paddrmap->addr_map_size; ++j)
			{
				theLog << j << " : " << paddrmap->shm_addr_map[j].first << " - " << (long)paddrmap->shm_addr_map[j].second << endl;
			}
			theLog << endi;
		}
	};
	template<typename T, int PI_N, typename T_USER_HEAD, int PART = 0, typename T_HANDLE = T_HANDLE_ARRAY<T, PI_N >  >
	class T_ARRAY : public IShmActiveObject
	{
		//子类型
	public:
		typedef T_HANDLE HANDLE;
		typedef T value_type;
	public:
		//旧版数组头
		struct array_head_old
		{
			CMeta meta;
			sstring<32> name;
			T_SHM_SIZE capacity;
			T_SHM_SIZE size;
			T_USER_HEAD userhead;
			struct_T_ARRAY_VMAP_S vmaps;//分块影射表
			long ____;//用来确保array_head用8字节对齐
		};
		//新版数组头
		struct array_head_new
		{
			CMeta meta;
			sstring<64> name;
			T_SHM_SIZE capacity;
			T_SHM_SIZE size;
			struct_T_ARRAY_VMAP_S vmaps;//分块影射表
			long ____;//用来确保array_head用8字节对齐
			T_USER_HEAD userhead;
		};
		//当前使用的数组头
		typedef array_head_new array_head;

	private:
		array_head* pHead;
		T* pData;
		bool isPrivate;
		bool m_isReadOnly;
		int shmid;
		string name;
		long version;

		//计算共享内存大小，比实际容量多申请一个单元
		static T_SHM_SIZE _CalcShmSize(T_SHM_SIZE size)
		{
			T_SHM_SIZE ret = sizeof(array_head) + size * sizeof(T);
			if (sizeof(T) > 1024)ret += 1024;
			else ret += sizeof(T);
			//thelog<<"计算出的大小 "<<ret<<endi;
			return ret;
		}
		void _makemeta(CMeta& meta, int version)const
		{
			int Tsize[7] = { sizeof(array_head),sizeof(T_USER_HEAD),sizeof(T),version,998,999,1000 };
			meta.Set(GUID_T_ARRAY, Tsize, 7);
		}
		bool _CheckMeta()const
		{
			if (NULL == pHead)return false;

			CMeta meta;
			string msg;
			_makemeta(meta, version);
			if (!pHead->meta.Compare(meta, msg))
			{
				string str;
				thelog << msg << ende;
				thelog << "数据格式不匹配" << endl << "数据的元数据信息" << endl << pHead->meta.toString(str) << ende;
				thelog << "应用的元数据信息" << endl << meta.toString(str) << ende;
				return false;
			}
			return true;
		}
		void _makefilename(string& file, char const* _dir)const
		{
			file = _dir;
			if (file.size() > 0 && file[file.size() - 1] != '/')file += "/";
			file += this->name;

			char buf[256];
			if (PART > 0)
			{
				sprintf(buf, "_%02d", PART);
				file += buf;
			}
			file += ".dat";
		}

		//初始化第一个共享内存分块影射表（存储于head）和地址映射表（存储于私有数据）
		void _InitFirstSPD()
		{
			this->pHead->vmaps.clearVMAP();
			this->pHead->vmaps.AddVMAP(shmid, 0, this->pHead->capacity);
			GET_SHM_PRIVATE_DATA(PI_N).clearShmPrivateData();
			GET_SHM_PRIVATE_DATA(PI_N).AddShmMap(shmid, (char*)(this->pHead) + sizeof(array_head));
			GET_PP_VMAP(PI_N) = &pHead->vmaps;
		}

		char* _CreateShmIfNeed(long len, bool& isNewshm)
		{
			char* p = NULL;
			shmid = CShmMan::CreatePrivateSHM(len);
			if (shmid < 0)
			{
				thelog << "创建共享内存失败 申请的大小 " << len
					<< " 通常是因为系统内存中没有足够的连续物理内存 或者超过了系统限制" << ende;
				return NULL;
			}
			else
			{
				thelog << "创建新共享内存成功，id = " << shmid << endi;
			}
			p = CShmMan::ConnectByID(shmid, false, (SHM_NAME_SHMPOOL == name ? (void*)ADDR_SHM_POOL : NULL));
			if (NULL == p)
			{
				thelog << "连接新共享内存失败 错误信息：" << strerror(errno) << ende;
				return NULL;
			}
			isNewshm = true;
			return p;
		}
		bool _LoadFromFile(bool toShm, char const* file)
		{
			if (NULL != this->pHead)
			{
				thelog << "数据指针不为空,不能用已经含有数据的对象做数据加载" << ende;
				return false;
			}
			if (toShm)
			{
				_DestoryShm();
			}

			bool isNewshm = true;//是否创建了新共享内存，若存在的空间够大就不创建
			ifstream f;

			f.open(file);
			if (!f.good())
			{
				thelog << "打开文件失败 " << file << ende;
				return false;
			}
			f.seekg(0, ios::end);
			long len = f.tellg();
			thelog << "文件长度 " << len << endi;

			//先读取元数据分析，再读取文件头，最后读取数据区

			long file_headsize;//文件实际的头长度，版本不同可能不同
			CMeta file_meta;//文件的meta
			f.seekg(0, ios::beg);
			f.read((char*)&file_meta, sizeof(CMeta));
			if (!f.good() || f.gcount() != sizeof(CMeta))
			{
				thelog << "读文件失败 " << file << ende;
				f.close();
				return false;
			}
			file_headsize = file_meta.GetInt(0);

			CMeta tmp_meta;
			string msg;
			_makemeta(tmp_meta, version);
			if (!tmp_meta.Compare(file_meta, msg))
			{
				thelog << "元数据格式不匹配" << endi;
				if (!file_meta.CheckGuid((signed char const*)&tmp_meta))
				{
					thelog << "文件的元数据GUID错误" << ende;
					f.close();
					return false;
				}
				if (!file_meta.CheckSys())
				{
					thelog << "文件的元数据格式信息错误，可能不是64位系统或字节序不同" << ende;
					f.close();
					return false;
				}

				if (file_headsize == sizeof(array_head))
				{
				}
				else if (file_headsize == sizeof(array_head_old))
				{
					thelog << "识别为旧版本数据" << endi;
				}
				else if (file_headsize == sizeof(array_head_new))
				{
					thelog << "识别为新版本数据" << endi;
				}
				else
				{
					thelog << "文件的元数据文件头长度错误，" << file_headsize << " 预期 " << sizeof(array_head_new) << " 或" << sizeof(array_head_old) << ende;
					f.close();
					return false;
				}

				if (file_meta.GetInt(1) != sizeof(T_USER_HEAD))
				{
					thelog << "文件的元数据USER_HEAD长度错误，" << file_meta.GetInt(1) << " 预期 " << sizeof(T_USER_HEAD) << ende;
					f.close();
					return false;
				}

				if (file_meta.GetInt(2) != sizeof(T))
				{
					thelog << "文件的元数据T长度错误，" << file_meta.GetInt(2) << " 预期 " << sizeof(T) << ende;
					f.close();
					return false;
				}

				if (file_meta.GetInt(3) != version)
				{
					thelog << "文件的元数据version错误，" << file_meta.GetInt(3) << " 预期 " << version << ende;
					f.close();
					return false;
				}
			}

			array_head new_head;//实际使用的头结构，定义为新结构或旧结构，若长度不相同，必然是与文件不同的另一个结构
			char file_head[file_headsize];//文件的头结构
			f.seekg(0, ios::beg);
			f.read(file_head, file_headsize);
			if (!f.good() || f.gcount() != file_headsize)
			{
				thelog << "读文件失败 " << file << ende;
				f.close();
				return false;
			}
			if (file_headsize == sizeof(array_head))
			{
				memcpy((void*)&new_head, file_head, file_headsize);
			}
			else if (file_headsize == sizeof(array_head_old))
			{
				array_head_old* p = (array_head_old*)file_head;
				_makemeta(new_head.meta, version);
				new_head.name = p->name.c_str();
				new_head.capacity = p->capacity;
				new_head.size = p->size;
				new_head.vmaps = p->vmaps;
				new_head.____ = p->____;
				new_head.userhead = p->userhead;
			}
			else if (file_headsize == sizeof(array_head_new))
			{
				array_head_new* p = (array_head_new*)file_head;
				_makemeta(new_head.meta, version);
				new_head.name = p->name.c_str();
				new_head.capacity = p->capacity;
				new_head.size = p->size;
				new_head.vmaps = p->vmaps;
				new_head.____ = p->____;
				new_head.userhead = p->userhead;
			}
			else
			{
				thelog << "未识别的头长度" << ende;
				f.close();
				return false;
			}

			T_SHM_SIZE r_size = -1;//数据库配置的大小

			//获取配置的大小
			ShmRegInfo tmpreg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
			if (tmpreg.GetConfigSize(r_size))
			{
				thelog << GetShmSysOfName(name.c_str()) << " " << name << " 配置的最大记录数 " << r_size << endi;
				//如果数据库配置了大小并且比文件大则使用数据库配置的大小
				if (_CalcShmSize(r_size) > len)
				{
					thelog << "配置的大小比文件大，使用配置的大小" << endi;
				}
				else
				{
					thelog << "配置的大小比文件小，使用文件的大小" << endi;
				}
			}
			else
			{
				r_size = -1;
				thelog << GetShmSysOfName(name.c_str()) << " " << name << " 未配置的最大记录数，使用文件的大小" << endi;
			}

			//创建
			if (toShm)
			{
				char* p = _CreateShmIfNeed((len > _CalcShmSize(r_size) ? len : _CalcShmSize(r_size)), isNewshm);
				if (NULL == p)
				{
					thelog << "无法获得共享内存" << ende;
					return false;
				}
				this->pHead = (array_head*)p;
			}
			else
			{
				this->pHead = (array_head*)new char[len > _CalcShmSize(r_size) ? len : _CalcShmSize(r_size)];
				if (NULL == this->pHead)
				{
					thelog << "内存不足" << ende;
					f.close();
					return false;
				}
			}
			memcpy((void*)this->pHead, &new_head, sizeof(array_head));
			this->pData = (T*)(((char*)this->pHead) + sizeof(array_head));
			this->isPrivate = (!toShm);

			long datalen = len - file_headsize;
			long count = 100 * 1024 * 1024;
			long i = 0;
			f.seekg(file_headsize, ios::beg);
			for (; i < datalen; i += count)
			{
				count = (i + count <= datalen ? count : datalen - i);
				f.read(((char*)this->pData) + i, count);
				if (!f.good() || f.gcount() != count)
				{
					thelog << "读文件失败 " << file << ende;
					f.close();
					return false;
				}
			}
			f.close();
			thelog << "读取完成 " << i << endi;

			//检查内容
			if (!_CheckMeta() || pHead->name != name)
			{
				thelog << "数据格式或名称不匹配" << ende;
				return false;
			}
			if (datalen != (long)(pHead->capacity * sizeof(T)))
			{
				thelog << "数据大小 " << datalen << " 与容量计算 " << pHead->capacity * sizeof(T) + sizeof(array_head) << " 不匹配" << ende;
				return false;
			}

			if (r_size > 0 && pHead->capacity < r_size)
			{
				pHead->capacity = r_size;//修正实际的容量
			}
			if (toShm)
			{
				if (isNewshm)
				{
					//注册共享内存，注册的是实际大小，可能大于申请的大小
					ShmRegInfo tmpreg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
					tmpreg.shmid = shmid;
					if (!CShmMan::GetState(shmid, tmpreg.segsz, tmpreg.ctime))
					{
						thelog << "获取共享内存失败" << ende;
						return false;
					}
					if (!tmpreg.SaveRegToDb())return false;
				}
			}

			_InitFirstSPD();
			return true;
		}
		bool _SaveToFile(char const* file)const
		{
			if (NULL == pHead)return false;

			ofstream f;

			f.open(file);
			if (!f.good())
			{
				thelog << "打开文件失败 " << file << ende;
				return false;
			}
			array_head tmphead;
			memcpy((void*)&tmphead, pHead, sizeof(array_head));
			tmphead.capacity = tmphead.size;
			tmphead.vmaps.clearVMAP();

			f.write((char*)&tmphead, sizeof(array_head));
			if (!f.good())
			{
				thelog << "写文件头失败 " << file << ende;
				return false;
			}

			long totlewrite = 0;
			thelog << "总数据字节数 " << tmphead.capacity * sizeof(T) << endi;
			HANDLE h;
			for (long vi = 0; vi < pHead->vmaps.size; ++vi)
			{
				struct_T_ARRAY_VMAP* pvmap = &pHead->vmaps.m_vmaps[vi];
				long totlebytes;
				long count = 100 * 1024 * 1024;
				long i;
				if (pvmap->handle_begin >= tmphead.capacity)break;
				if (pvmap->handle_end >= tmphead.capacity)totlebytes = (tmphead.capacity - pvmap->handle_begin) * sizeof(T);
				else totlebytes = (pvmap->handle_end - pvmap->handle_begin) * sizeof(T);
				for (i = 0; i < totlebytes; i += count)
				{
					count = (i + count <= totlebytes ? count : totlebytes - i);
					h.handle = pvmap->handle_begin;
					f.write(((char*)&*h) + i, count);
					if (!f.good())
					{
						thelog << "写文件失败 " << file << ende;
						return false;
					}
				}
				thelog << "写数据入字节数 " << i << endi;
				totlewrite += i;
			}
			thelog << "总写入数据字节数 " << totlewrite << endi;

			f.close();
			return true;
		}
	public:
		T_ARRAY(char const* _name, int _version) :pHead(NULL), pData(NULL), isPrivate(true), shmid(-1), name(_name), version(_version) {}
		array_head const* GetHead()const { return pHead; }
		T_USER_HEAD* GetUserHead()const { return &pHead->userhead; }
		T const* GetData()const
		{
			if (IsOneBlock())return pData;
			else
			{
				thelog << "不是单一块，不能获取pData" << ende;
				return NULL;
			}
		}
		bool IsPrivate()const { return isPrivate; }
		bool IsConnected()const { return NULL != this->pHead; }
		bool IsReadOnly()const { return m_isReadOnly; }
		long Size()const { return (pHead != NULL ? pHead->size : 0); }
		long Capacity()const { return (pHead != NULL ? pHead->capacity : 0); }
		void SetVirtualCapacity(long n) { pHead->capacity = n; }
		HANDLE Begin()const { return HANDLE(0); }
		HANDLE End()const { return HANDLE(pHead->size); }

		string& ReportHead(string& str, bool withVMAP = true)const
		{
			str = "";
			char buf[2048];
			str += "\n";
			str += pHead->name.c_str();
			str += " 开始报告T_ARRAY......\n";
			sprintf(buf, "pHead=%p pData=%p isPrivate=%ld shmid=%ld sizeof(T)=%ld\n", pHead, pData, (isPrivate ? 1L : 0L), (long)shmid, (long)sizeof(T));
			str += buf;
			if (NULL != pHead)
			{
				string tmp;
				str += pHead->meta.toString(tmp);
				str += "\n";
				sprintf(buf, "name=%s part=%d capacity=%ld,size=%ld(%.2f%%) bytes=%ld\n", pHead->name.c_str(), PART, (long)pHead->capacity, (long)pHead->size, 100. * pHead->size / pHead->capacity, (long)sizeof(array_head) + pHead->capacity * sizeof(T));
				str += buf;
				//地址映射表
				if (withVMAP)
				{
					long i;
					shm_private_data const* pSPD = &GET_SHM_PRIVATE_DATA(PI_N);
					sprintf(buf, "地址映射表： GET_PP_SET=%p\n", GET_PP_SET(PI_N));
					str += buf;
					sprintf(buf, "分块数 %ld\n", pHead->vmaps.size);
					str += buf;
					for (i = 0; i < pHead->vmaps.size; ++i)
					{
						sprintf(buf, "%ld : shm_id=%d handle[%ld,%ld) 容量=%ld ", i, pHead->vmaps.m_vmaps[i].shm_id, pHead->vmaps.m_vmaps[i].handle_begin, pHead->vmaps.m_vmaps[i].handle_end, pHead->vmaps.m_vmaps[i].handle_end - pHead->vmaps.m_vmaps[i].handle_begin);
						str += buf;
						if (i < pSPD->addr_map_size)
						{
							sprintf(buf, "连接信息： shm_id=%d addr=%p", pSPD->shm_addr_map[i].first, pSPD->shm_addr_map[i].second);
							str += buf;
						}
						else
						{
							str += "未连接";
						}
						str += "\n";
					}
				}
				CToString tostring;
				str += "--------------------------------------------\n";
				str += tostring(pHead->userhead, tmp);
			}
			return str;
		}
		string& Report(string& str, bool withDetail)const
		{
			long maxsize = 10;
			bool withVMAP = true;
			str = ReportHead(str, withVMAP);

			char buf[2048];
			CHtmlDoc::CHtmlTable2 table;
			table.AddCol("h", CHtmlDoc::CHtmlDoc_DATACLASS_RIGHT);

			if (NULL != pHead)
			{
				if (withDetail)
				{
					string tmp;
					T_SHM_SIZE i;
					HANDLE h;
					bool isTable = T::AddTableColumns(table);
					for (i = 0; i < pHead->size; ++i)
					{
						if (maxsize <= 0 || i < maxsize || i >= pHead->size - maxsize)
						{
							h.handle = i;
							if (!isTable)
							{
								sprintf(buf, "%2ld %p: ", i, &*h);
								str += buf;
								str += h->toString(tmp);
								str += "\n";
							}
							else
							{
								table.AddLine();
								table.AddData(i);
								h->AddTableData(table);
							}
						}
						else
						{
							if (!isTable)
							{
								str += "......\n";
							}
							else
							{
								table.AddLine();
								table.AddData("...");
							}
							i = pHead->size - maxsize - 1;
						}
					}
					if (isTable)
					{
						str += table.MakeTextTable();
					}
				}
			}
			return str;
		}
		//只显示数据，不显示结构信息的版本
		string& Report3(string& str, long maxsize = 10)const
		{
			str = "\n";
			char buf[2048];
			if (NULL != pHead)
			{
				sprintf(buf, "pHead=%p pData=%p isPrivate=%ld shmid=%ld sizeof(T)=%ld\n", pHead, pData, (isPrivate ? 1L : 0L), (long)shmid, (long)sizeof(T));
				str += buf;
				sprintf(buf, "name=%s part=%d capacity=%ld,size=%ld(%.2f%%) bytes=%ld\n", pHead->name.c_str(), PART, (long)pHead->capacity, (long)pHead->size, 100. * pHead->size / pHead->capacity, (long)sizeof(array_head) + pHead->capacity * sizeof(T));
				str += buf;
				bool ed = false;
				string tmp;
				T_SHM_SIZE i;
				HANDLE h;
				for (i = 0; i < pHead->size; ++i)
				{
					if (maxsize <= 0 || i < maxsize || i >= pHead->size - maxsize)
					{
						h.handle = i;
						sprintf(buf, "%2ld : ", i);
						str += buf;
						str += h->toString(tmp);
						str += "\n";
					}
					else
					{
						if (!ed)str += "......\n";
						ed = true;
					}
				}
			}
			else
			{
				str += "未连接\n";
			}
			return str;
		}
		//若bSetSize则直接设置数据大小，但不执行构造函数，用于获得一块固定大小的内存
		//defaultsize为0则从数据库获取大小
		bool _CreatePrivate(T_SHM_SIZE defaultsize = 16, bool bSetSize = false)
		{
			if (0 == defaultsize)
			{
				T_SHM_SIZE r_size;
				ShmRegInfo tmpreg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
				if (tmpreg.GetConfigSize(r_size))
				{
					thelog << GetShmSysOfName(name.c_str()) << " " << name << " " << PART << " 配置的最大记录数 " << r_size << endi;
				}
				else
				{
					thelog << GetShmSysOfName(name.c_str()) << " " << name << " " << PART << " 未配置的最大记录数" << ende;
					return false;
				}
				defaultsize = r_size;
			}

			if (defaultsize < 1)
			{
				thelog << "尺寸错误：" << defaultsize << ende;
				return false;
			}
			char* tmp = new char[_CalcShmSize(defaultsize)];
			if (NULL == tmp)return false;
			pHead = (array_head*)tmp;
			new(pHead) array_head;
			pData = (T*)(tmp + sizeof(array_head));

			_makemeta(pHead->meta, version);
			pHead->name = name;
			pHead->capacity = defaultsize;
			pHead->size = 0;
			isPrivate = true;

			if (bSetSize)
			{
				pHead->size = defaultsize;
			}

			_InitFirstSPD();
			return true;
		}
		bool _DestoryPrivate()
		{
			thelog << name << " " << PART << " 删除私有内存" << endi;
			if (isPrivate && NULL != pHead)
			{
				char* tmp = (char*)pHead;
				delete[] tmp;
				pHead = NULL;
				pData = NULL;

				GET_PP_VMAP(PI_N) = NULL;
				return true;
			}
			else
			{
				return false;
			}
		}
		bool AttachToShm_virtual(char* head)
		{
			this->pHead = (array_head*)head;
			this->pData = NULL;
			this->isPrivate = false;

			string str;
			thelog << ReportHead(str, false) << endi;
			return true;
		}
		bool AttachToShm(bool _isReadOnly)
		{
			if (IsConnected())
			{
				if (!DetachFromShm())return false;
			}
			//获取注册信息
			ShmRegInfo reg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
			if (!reg.GetRegFromDb())return false;
			shmid = reg.shmid;

			m_isReadOnly = _isReadOnly;
			char* p = CShmMan::ConnectByID(shmid, m_isReadOnly, (SHM_NAME_SHMPOOL == name ? (void*)ADDR_SHM_POOL : NULL));
			if (NULL == p)
			{
				thelog << "连接共享内存失败 shmid = " << shmid << " 错误信息：" << strerror(errno) << ende;
				return false;
			}
			thelog << name << " 连接共享内存成功 PI_N " << PI_N << " PART " << PART << " shmid = " << shmid << " p " << (void*)p << endi;
			if (((unsigned long)p) % 8 != 0)
			{
				thelog << "地址对齐错误" << ende;
				DetachFromShm();
				return false;
			}
			this->pHead = (array_head*)p;
			this->pData = (T*)(p + sizeof(array_head));
			this->isPrivate = false;

			if (!_CheckMeta())
			{
				DetachFromShm();
				return false;
			}
			if (pHead->name != name)
			{
				thelog << "数据名称不匹配 " << pHead->name.c_str() << " " << name << ende;
				DetachFromShm();
				return false;
			}

			GET_SHM_PRIVATE_DATA(PI_N).clearShmPrivateData();
			GET_SHM_PRIVATE_DATA(PI_N).AddShmMap(shmid, (char*)(this->pHead) + sizeof(array_head));
			GET_PP_VMAP(PI_N) = &pHead->vmaps;

			//string str;
			//thelog<<ReportHead(str)<<endi;
			return true;
		}
		bool DetachFromShm()
		{
			if (this->isPrivate)return false;

			//断开附加块（主块首地址不是这个结构登记的）
			shm_private_data* spd = &GET_SHM_PRIVATE_DATA(PI_N);
			long addri;
			for (addri = 1; addri < spd->addr_map_size; ++addri)
			{
				if (!CShmMan::Disconnect(spd->shm_addr_map[addri].second))
				{
					thelog << "断开共享内存失败" << ende;
					return false;
				}
			}
			//断开主块
			if (!CShmMan::Disconnect((PSHM)this->pHead))
			{
				thelog << "断开共享内存失败" << ende;
				return false;
			}
			this->isPrivate = true;
			this->pHead = NULL;
			this->pData = NULL;
			this->shmid = -1;

			GET_PP_VMAP(PI_N) = NULL;
			return true;
		}
		bool _DestoryShm()
		{
			if (!AttachToShm(false))
			{
				thelog << name << " " << PART << " 连接到共享内存失败" << endi;
				return false;
			}
			long vi;
			for (vi = 0; vi < pHead->vmaps.size; ++vi)
			{
				if (!CShmMan::Destory(pHead->vmaps.m_vmaps[vi].shm_id))
				{
					thelog << "删除共享内存出错 " << vi << " id=" << pHead->vmaps.m_vmaps[vi].shm_id << ende;
					return false;
				}
			}
			DetachFromShm();

			//删除主块注册信息
			ShmRegInfo reg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
			//if(!reg.GetRegFromDb())return false;
			if (!reg.DeleteRegFromDb())return false;

			return true;
		}
		//创建虚拟共享内存（用于在已经提供的地址上构造）
		bool _CreateShm_virtual(char* head)
		{
			pHead = (array_head*)head;
			new(pHead) array_head;
			pData = NULL;

			_makemeta(pHead->meta, version);
			pHead->name = name;
			pHead->capacity = 1;//不能为0
			pHead->size = 0;
			isPrivate = false;

			thelog << "虚拟基础共享内存创建完成" << endi;
			return true;
		}
		//创建共享内存
		bool _CreateShm(T_SHM_SIZE r_size)
		{
			_DestoryShm();
			if (0 == r_size)
			{
				ShmRegInfo tmpreg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
				if (tmpreg.GetConfigSize(r_size))
				{
					thelog << GetShmSysOfName(name.c_str()) << " " << name << " " << PART << " 配置的最大记录数 " << r_size << endi;
				}
				else
				{
					thelog << GetShmSysOfName(name.c_str()) << " " << name << " " << PART << " 未配置的最大记录数" << endi;
					r_size = 1024;
					thelog << "使用默认值 " << r_size << endi;
					//return false;
				}
			}

			if (r_size < 1)
			{
				thelog << "尺寸错误：" << r_size << ende;
				return false;
			}
			if (1 == sizeof(T) && r_size > 8 && 0 != r_size % 8)
			{
				r_size = (r_size / 8 + 1) * 8;
				thelog << "T=char记录数不为8的倍数，需修正为" << r_size << endi;
			}

			bool isNewshm = true;
			char* tmp = _CreateShmIfNeed(_CalcShmSize(r_size), isNewshm);
			if (NULL == tmp)
			{
				thelog << "无法获取共享内存" << ende;
				return false;
			}
			pHead = (array_head*)tmp;
			new(pHead) array_head;
			pData = (T*)(tmp + sizeof(array_head));

			_makemeta(pHead->meta, version);
			pHead->name = name;
			pHead->capacity = r_size;
			pHead->size = 0;
			isPrivate = false;
			_InitFirstSPD();

			//注册共享内存
			if (isNewshm)
			{
				ShmRegInfo tmpreg(GetShmSysOfName(name.c_str()), name.c_str(), PART);
				tmpreg.shmid = shmid;
				if (!CShmMan::GetState(shmid, tmpreg.segsz, tmpreg.ctime))
				{
					thelog << "获取共享内存失败" << ende;
					return false;
				}
				if (!tmpreg.SaveRegToDb())return false;
			}

			if (!DetachFromShm())
			{
				thelog << "断开共享内存失败" << ende;
				return false;
			}

			thelog << "基础共享内存创建完成" << endi;
			return true;
		}
		bool Reserve(T_SHM_SIZE _n, T_SHM_SIZE min_block_n = 0)
		{
			if (pHead->capacity >= _n)return true;
			T_SHM_SIZE new_size = (pHead->capacity >= 1000 * 1000 * 10 ? pHead->capacity / 5 : pHead->capacity);//增加的容量，先设置为默认的比例
			if (new_size < _n - pHead->capacity)new_size = _n - pHead->capacity;
			if (new_size < 1)new_size = 1;
			if (new_size < min_block_n)new_size = min_block_n;
			if (1 == sizeof(T) && new_size > 8 && 0 != new_size % 8)
			{
				new_size = (new_size / 8 + 1) * 8;
				thelog << "T=char记录数不为8的倍数，需修正为" << new_size << endi;
			}

			thelog << PI_N << " 容量 " << pHead->capacity << " 申请容量 " << _n << " 最小新块 " << min_block_n << " 实际申请 " << new_size << " 扩展后容量 " << pHead->capacity + new_size << endi;
			if (!isPrivate)
			{//共享内存扩展,附加一个共享内存块,不连续
				if (NULL != GET_PP_LRU(PI_N) || name.substr(1, 3) == "LOG")
				{
					thelog << "LRU缓存和LOG*命名的共享内存" << ende;
					return false;
				}
#ifdef SHM_ALLOCATOR_USE_OLD_POINTER
				if (SHM_NAME_SHMPOOL == name))
				{
				thelog << "老式共享内存池不可扩展" << ende;
				return false;
				}
#endif
				if (pHead->vmaps.size >= T_ARRAY_VMAP_MAX_SIZE)
				{
					thelog << "共享内存扩展次数已经达到最大,不能扩展" << ende;
					return false;
				}
				if (pHead->vmaps.size >= T_ARRAY_VMAP_MAX_SIZE - 5)
				{
					thelog << "注意:共享内存扩展次数已经达到 " << pHead->vmaps.size << " ,最大扩展次数为 " << T_ARRAY_VMAP_MAX_SIZE << " ,请整理共享内存并修正初始大小设置" << endw;
				}
				int new_shmid;
				while ((new_shmid = CShmMan::CreatePrivateSHM(sizeof(T) * new_size)) < 0)
				{
					thelog << "创建共享内存失败 申请的大小 " << sizeof(T) * new_size << " 通常是因为系统内存中没有足够的连续物理内存 或者超过了系统限制" << endw;
					new_size /= 2;
					if (new_size > 1 && new_size >= min_block_n)
					{
						thelog << "尝试申请更小的大小 " << sizeof(T) * new_size << endi;
						continue;
					}
					else
					{
						thelog << "创建共享内存失败.不能申请所需的最小共享内存" << ende;
						return false;
					}
				}
				thelog << "创建新共享内存成功，id = " << new_shmid << endi;
				char* p = CShmMan::ConnectByID(new_shmid, false);
				if (NULL == p)
				{
					thelog << PI_N << " 连接新共享内存失败 错误信息：" << strerror(errno) << ende;
					return false;
				}
				//设置地址映射表
				GET_SHM_PRIVATE_DATA(PI_N).shm_addr_map[pHead->vmaps.size].first = new_shmid;
				GET_SHM_PRIVATE_DATA(PI_N).shm_addr_map[pHead->vmaps.size].second = p;
				++GET_SHM_PRIVATE_DATA(PI_N).addr_map_size;
				//设置分块影射表
				struct_T_ARRAY_VMAP* pvmap = &pHead->vmaps.m_vmaps[pHead->vmaps.size];
				pvmap->shm_id = new_shmid;
				pvmap->handle_begin = pHead->capacity;
				pvmap->handle_end = pvmap->handle_begin + new_size;
				++pHead->vmaps.size;
				//设置新的总容量
				pHead->capacity += new_size;
			}
			else
			{//私有内存扩展,保持为一个连续的块
				char* tmp = new char[_CalcShmSize(pHead->capacity + new_size)];
				if (NULL == tmp)
				{
					return false;
				}
				memcpy(tmp, pHead, _CalcShmSize(pHead->capacity));
				delete[](char*)pHead;
				pHead = (array_head*)tmp;
				pData = (T*)(tmp + sizeof(array_head));
				pHead->capacity = pHead->capacity + new_size;

				_InitFirstSPD();
			}
			string str;
			thelog << ReportHead(str) << endi;
			return true;
		}
		bool Clear()
		{
			new(&pHead->userhead) T_USER_HEAD;
			//无需析构，共享内存里面不应该有需要特别析构的数据
			pHead->size = 0;
			return true;
		}
		//设置大小，只能减小
		bool SetSize(long new_size)
		{
			if (new_size > pHead->size)return false;
			pHead->size = new_size;
			return true;
		}
		bool Add(T const& data, HANDLE& h)
		{
			if (pHead->capacity > pHead->size)
			{
				h.handle = pHead->size;
				*h = data;
				++(pHead->size);
			}
			else
			{
				thelog << "容量 " << pHead->capacity << " 大小 " << pHead->size << " 需要扩展" << endi;
				if (!Reserve(pHead->size + 1))
				{
					h.handle = -1;
					return false;
				}
				return Add(data, h);
			}
			return true;
		}
		//添加一组数据，保证存放在同一个块(通常用于字符串)
		bool Adds_block(T const* pDatas, T_SHM_SIZE count, HANDLE& h)
		{
			HANDLE h1, h2;
			h1.handle = pHead->size;
			h2.handle = h1.handle + count - 1;
			if (pHead->capacity < pHead->size + count)
			{
				pHead->size = pHead->capacity;//空间不足，直接跳过剩余空间
				if (!Reserve(pHead->size + count, count))
				{
					h.handle = -1;
					return false;
				}
				return Adds_block(pDatas, count, h);
			}
			else if (&*h2 - &*h1 != count - 1)
			{
				Add(pDatas[0], h);//空间足够但不是在单一块，填充一个无意义数据后再次尝试，直到进入一个新块或扩充了空间
				return Adds_block(pDatas, count, h);
			}
			else
			{
				T_SHM_SIZE i;
				for (i = 0; i < count; ++i)
				{
					if (!Add(pDatas[i], h))
					{
						return false;
					}
				}
				h.handle -= (count - 1);
				return true;
			}
		}
		bool IsOneBlock()const
		{
			return 1 == pHead->vmaps.size;
		}
		//排序，速度比较快，但只能是单一块
		template <typename T_COMP >
		void Sort_fast(T_COMP comp)
		{
			if (!IsOneBlock())
			{
				thelog << "共享内存 " << name << " 不是单一块，不能调用此功能" << endi;
				return;
			}
			sort(pData, pData + pHead->size, comp);
		}
		//排序，速度比较慢，但不必是单一块
		template <typename T_COMP>
		void Sort_slow(T_COMP comp)
		{
			sort(Begin(), End(), comp);
		}
		HANDLE& LowerBound(T const& data, HANDLE& h)const
		{
			if (!IsOneBlock())
			{
				thelog << "共享内存 " << name << " 不是单一块，不能调用此功能" << endi;
				return h;
			}
			T* it = lower_bound(pData, pData + pHead->size, data);
			h.handle = it - pData;
			return h;
		}
		HANDLE& LowerBound(T const& data, HANDLE& h, bool (*less)(T const&, T const&))const
		{
			if (!IsOneBlock())
			{
				thelog << "共享内存 " << name << " 不是单一块，不能调用此功能" << endi;
				return h;
			}
			T* it = lower_bound(pData, pData + pHead->size, data, less);
			h.handle = it - pData;
			return h;
		}
		T* Get(HANDLE const& h)const { return &*h; }
		T* Get(T_SHM_SIZE i)const
		{
			HANDLE h;
			h.handle = i;
			return (T*)(void*)&*h;
		}
	public:
		//IShmActiveObject接口
		virtual char const* GetName()const { return name.c_str(); }
		virtual int GetPart()const { return PART; }//获得共享内存的模板参数PART
		virtual int GetPI()const { return PI_N; };//获得共享内存的模板参数PI_N
		virtual bool isPrivateMem()const { return IsPrivate(); }
		virtual bool isConnected()const { return IsConnected(); }
		virtual bool isReadOnly()const { return IsReadOnly(); }
		virtual bool CreateShm()
		{
			return _CreateShm(0);
		}
		virtual bool CreatePrivate()
		{
			return _CreatePrivate();
		}
		virtual bool _Attach(bool isReadOnly)
		{
			return AttachToShm(isReadOnly);
		}
		virtual bool Detach()
		{
			return DetachFromShm();
		}
		virtual bool SaveToDir(char const* _dir)const
		{
			string file;
			_makefilename(file, _dir);
			return _SaveToFile(file.c_str());
		}
		virtual bool LoadFromDir(char const* _dir)
		{
			string file;
			_makefilename(file, _dir);
			return _LoadFromFile(true, file.c_str());
		}
		virtual bool LoadPrivateFromDir(char const* _dir)
		{
			string file;
			_makefilename(file, _dir);
			return _LoadFromFile(false, file.c_str());
		}
		virtual bool DestoryShm()
		{
			return _DestoryShm();
		}
		virtual bool DestoryPrivate()
		{
			return _DestoryPrivate();
		}
		virtual bool ExportTextToDir(char const* dir_name)const
		{
			thelog << "T_ARRAY 尚未支持ExportTextToDir" << endi;
			return true;
		}
		virtual bool Report()const
		{
			string str;
			thelog << ReportHead(str) << endi;
			return true;
		}
		virtual bool clear()
		{
			return Clear();
		}
		size_t size()const override
		{
			return Size();
		}
		size_t capacity()const override
		{
			return Capacity();
		}
		size_t record_length()const override
		{
			return sizeof(T);
		}
		size_t byte_size()const override
		{
			return _CalcShmSize(capacity());
		}
		size_t byte_capacity()const override
		{
			return capacity() * sizeof(T);
		}
		size_t block_count()const override
		{
			return pHead->vmaps.size;
		}
	};

}
