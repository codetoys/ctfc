//myshm.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "mymutex.h"
#include "function.h"
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>

namespace ns_my_std
{
	typedef char * PSHM;

	class CShmMan
	{
	public:
		static PSHM ConnectByID(int id,bool isreadonly,void * addr=NULL)
		{
			PSHM pshm;
			if((pshm=(PSHM)shmat(id,addr,(isreadonly?SHM_RDONLY:0)))==(PSHM)(-1L))
			{
				return NULL;
			}
			return pshm;
		}
		static PSHM ConnectByKey(key_t key,bool isreadonly)
		{
			int shmid;
			if(0>(shmid=shmget(key,0,SHM_R|(SHM_R>>3)|(SHM_R>>6)|SHM_W|(SHM_W>>3)|(SHM_W>>6))))
			{//必须是已经存在的
				return NULL;
			}
			PSHM pshm;
			if((pshm=(PSHM)shmat(shmid,0,(isreadonly?SHM_RDONLY:0)))==(PSHM)(-1L))
			{
				return NULL;
			}
			return pshm;
		}
		static int CreatePrivateSHM(long size){return CreateSHMByKey(IPC_PRIVATE,size);}
		static int CreateSHMByKey(key_t key,long size)
		{
			int shmid;
			if(0>(shmid=shmget(key,size,SHM_R|(SHM_R>>3)|(SHM_R>>6)|SHM_W|(SHM_W>>3)|(SHM_W>>6)|IPC_CREAT|IPC_EXCL)))
			{//必须不是已经存在的
				//LOG<<"shmid "<<shmid<<ENDE;
				return -1;
			}
			return shmid;
		}

		//删除共享内存，删除后不可再使用
		static bool Destory(int shmid)
		{
			if(shmctl(shmid,IPC_RMID,0)<0)
			{
				return false;
			}
			return true;
		}
		static bool Disconnect(PSHM p)
		{
			if(0!=shmdt(p))
			{
				return false;
			}
			return true;
		}

		static bool GetState(int shmid,size_t & size,time_t & ctime)
		{
			shmid_ds ds;

			if(0!=shmctl(shmid,IPC_STAT,&ds))
			{
				return false;
			}

			size=ds.shm_segsz;
			ctime=ds.shm_ctime;

			return true;
		}
	};

}//ns_my_std
