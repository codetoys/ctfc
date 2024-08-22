//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once
//
//多进程服务器框架
//服务模式：单进程分发任务-多进程处理任务-单进程汇总
//

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "../function/mymutex.h"

bool const DEBUG_ENABLE_BINDCPU = false;//绑定CPU
#define CHAR_0xE3 (0xE3)

namespace ns_my_std
{
	//接口返回值说明：一般均定义为 bool fun(...,ret)的形式，函数返回值表达是否执行出错（系统错误，不再继续运行），ret表达处理结果
	
	//分发接口
	template<typename T_JOBINFO,typename T_TASKINFO>
	class IDistributeTask
	{
	public:
		//打开job，若成功打开hasJob=true，否则没有job可做
		virtual bool OpenJob(T_JOBINFO * pJobInfo, bool * hasJob)
		{
			*hasJob = *_in_auto_job;
			//thelog << "OpenJob " << *hasJob << endi;
			return true;
		}
		//分发一个task，放入pTaskBlock，如果发生数据性错误ret=false，仍继续处理
		virtual bool DistributeTask(T_JOBINFO * pJobInfo, T_TASKINFO * pTaskBlock, bool * ret) = 0;
		//是否job结束
		virtual bool isJobEnd(T_JOBINFO * pJobInfo, bool * _isJobEnd)
		{
			*_isJobEnd = !*_in_auto_job;
			//thelog << "isJobEnd " << *_isJobEnd << " " << *_in_auto_job << endi;
			return true;
		}
		//结束job
		virtual bool CloseJob_Distribute(T_JOBINFO * pJobInfo) { return true; }
	public:
		//获得下一个TASK的hash，当前没有任务可返回负值，负值不使用指定流水线，随机分配，如果采用此机制则只能在暂停时改变进程数
		virtual long IDistributeTask_GetNextTaskHash(bool * hasTask) { *hasTask = true; return -1; }
	public:
		//标志，消息模式时用于内部隐含的JOB开始和结束
		bool * _in_auto_job;//true OpenJob默认打开JOB false 则isJobEnd默认关闭JOB
		IDistributeTask() :_in_auto_job(NULL) {}
		bool IDistributeTask_DistributeTask(timeval * pTimeVal, T_JOBINFO * pJobInfo, T_TASKINFO * pTaskBlock, bool * ret)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret=DistributeTask(pJobInfo,pTaskBlock,ret);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
		bool IDistributeTask_isJobEnd(timeval * pTimeVal, T_JOBINFO * pJobInfo, bool * ret)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret = isJobEnd(pJobInfo, ret);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
		bool IDistributeTask_OpenJob(timeval * pTimeVal, T_JOBINFO * pJobInfo, bool * ret)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret = OpenJob(pJobInfo, ret);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
		bool IDistributeTask_CloseJob_Distribute(timeval * pTimeVal, T_JOBINFO * pJobInfo)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret = CloseJob_Distribute(pJobInfo);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
	};
	//处理接口
	template<typename T_JOBINFO,typename T_TASKINFO>
	class IProcessTask
	{
	private:
		virtual bool Process(T_JOBINFO * pJobInfo,T_TASKINFO * pTaskBlock,bool * ret)=0;
		virtual bool UnProcess(T_JOBINFO * pJobInfo,T_TASKINFO * pTaskBlock,bool * ret)=0;
	public:
		bool IProcessTask_Process(timeval * pTimeVal,T_JOBINFO * pJobInfo,T_TASKINFO * pTaskBlock,bool * ret)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret=Process(pJobInfo,pTaskBlock,ret);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
		bool IProcessTask_UnProcess(timeval * pTimeVal,T_JOBINFO * pJobInfo,T_TASKINFO * pTaskBlock,bool * ret)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret=UnProcess(pJobInfo,pTaskBlock,ret);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
	};
	//汇总接口
	template<typename T_JOBINFO,typename T_TASKINFO>
	class IRollupTask
	{
	public:
		//打开job
		virtual bool OpenJob_Rollup(T_JOBINFO * pJobInfo)=0;
		virtual bool Rollup(T_JOBINFO * pJobInfo,T_TASKINFO * pTaskBlock,bool * ret)=0;
		//结束job
		virtual bool CloseJob_Rollup(T_JOBINFO * pJobInfo)=0;
	public:
		bool IRollupTask_Rollup(timeval * pTimeVal, T_JOBINFO * pJobInfo, T_TASKINFO * pTaskBlock, bool * ret)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret=Rollup(pJobInfo,pTaskBlock,ret);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
		bool IRollupTask_OpenJob_Rollup(timeval * pTimeVal, T_JOBINFO * pJobInfo)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret=OpenJob_Rollup(pJobInfo);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
		bool IRollupTask_CloseJob_Rollup(timeval * pTimeVal, T_JOBINFO * pJobInfo)
		{
			timeval t3;
			gettimeofday(&t3,NULL);

			bool _ret=CloseJob_Rollup(pJobInfo);

			timeval t4;
			gettimeofday(&t4, NULL);

			pTimeVal->tv_sec += t4.tv_sec - t3.tv_sec;
			pTimeVal->tv_usec += t4.tv_usec - t3.tv_usec;

			return _ret;
		}
	};

	//多进程服务器框架
	//T_SERVERINFO 服务信息，内容完全由用户管理，放在共享内存中，用getServerInfo获得
	//T_TASKINFO 任务信息，内容完全由用户管理，作为接口传递的参数
	template<typename T_JOBINFO, typename T_SERVERINFO, typename T_TASKINFO, long MAX_JOB, long MAX_PROCESS, long MAX_PROCESS_TASK_BLOCK, long MAX_BLOCK_TASK>
	class CMultiProcessServer
	{
	public:
		enum STATE_T{STATE_NULL=0,STATE_CREATE,STATE_RUN,STATE_PAUSE,STATE_EXITED,STATE_COREDUMP};//进程状态
		enum COMMAND_T{CMD_NULL=0,CMD_EXIT,CMD_PAUSE};//进程命令
		enum TASK_STATE_T{TASK_STATE_NULL=100,TASK_STATE_NEW,TASK_STATE_DONE,TASK_STATE_UNDO,TASK_STATE_CANCLE};//任务状态

		static char const * StateStr(STATE_T state)
		{
			STATIC_G char buf[64];
			switch(state)
			{
			case STATE_NULL:return "未用";
			case STATE_CREATE:return "创建";
			case STATE_RUN:return "运行";
			case STATE_PAUSE:return "暂停";
			case STATE_EXITED:return "退出";
			case STATE_COREDUMP:return "COREDUMP";
			default:
				sprintf(buf,"未识别的进程状态码 %ld",(long)state);
				return buf;
			}
		}
		static char const * TaskStateStr(TASK_STATE_T state)
		{
			STATIC_G char buf[64];
			switch(state)
			{
			case TASK_STATE_NULL:return "NULL";
			case TASK_STATE_NEW:return "NEW";
			case TASK_STATE_DONE:return "DONE";
			case TASK_STATE_UNDO:return "UNDO";
			case TASK_STATE_CANCLE:return "CANCLE";
			default:
				sprintf(buf,"未识别的进程状态码 %ld",(long)state);
				return buf;
			}
		}
		static char const * CmdStr(COMMAND_T cmd)
		{
			STATIC_G char buf[64];
			switch(cmd)
			{
			case CMD_NULL:return "无";
			case CMD_EXIT:return "退出";
			case CMD_PAUSE:return "暂停";
			default:
				sprintf(buf,"未识别的命令码 %ld",(long)cmd);
				return buf;
			}
		}
		static string maketimestr(time_t t)
		{
			tm const * t2;
			char buf[256];
			t2=localtime(&t);
			if(0==t)buf[0]='\0';
			else sprintf(buf,"%02d-%02d%02d%02d",t2->tm_mday,t2->tm_hour,t2->tm_min,t2->tm_sec);
			return buf;
		}
		class PID_DATA
		{
		public:
			pid_t pid;
			long create_seq;//创建顺序号，创建了多少次
			long create_key;//创建关键字，用于阻止重复进程
			clock_t cpu_clock;//clock()
		private:
			STATE_T state;
			time_t state_time;
			COMMAND_T cmd;
			time_t cmd_time;
			long cmd_seq;//命令序列号，每次设置命令+1
			int exit_code;//退出码
			long sleep_count;//睡眠次数
			time_t sig_time;//发信号的时间，用于避免短时间内快速发送大量信号给进程
		public:
			string & toString(string & ret)const
			{
				char buf[2048];
				sprintf(buf,"pid[%ld] state %s %s cmd(%ld) %s %s 睡眠[%ld] sig(%s) create(%ld)(%ld) exit_code[%d] clock[%ld]"
					,(long)pid,StateStr(state),maketimestr(state_time).c_str()
					,cmd_seq,CmdStr(cmd),maketimestr(cmd_time).c_str(),sleep_count,maketimestr(sig_time).c_str(),create_seq,create_key,exit_code,cpu_clock);
				return ret=buf;
			}
			long getSleepCount()const{return sleep_count;}
			STATE_T getState(CMyRWMutex & mutex)const
			{
				mutex.RLock();
				STATE_T ret = state;
				mutex.RUnLock();
				return ret;
			}
			COMMAND_T getCmd()const{return cmd;}
			long getCmdSeq()const{return cmd_seq;}
			void setState(CMyRWMutex & mutex,STATE_T s)
			{
				mutex.RLock();
				state=s;
				state_time=time(NULL);
				mutex.RUnLock();
			}
			void setCmd(CMyRWMutex & mutex,COMMAND_T c)
			{
				mutex.RLock();
				cmd=c;
				++cmd_seq;
				cmd_time=time(NULL);
				mutex.RUnLock();
			}
			int setExit(CMyRWMutex & mutex,int ret)
			{
				mutex.RLock();
				state=STATE_EXITED;
				state_time=time(NULL);
				exit_code=ret;
				mutex.RUnLock();
				return ret;
			}
			bool isOK()
			{
				//需要避免并发冲突，仅当进程不存在并且pid不变的情况下认为进程异常
				pid_t tmppid=pid;
				if(tmppid>0)
				{
					if(isRuning())
					{
						if(0!=kill(tmppid,0) && tmppid==pid && isRuning())
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				return true;
			}
			bool isRuning()
			{
				//需要避免并发冲突，提前保存state，否则判断会因为时序而出错
				STATE_T tmpstate=state;
				return (STATE_RUN==tmpstate || STATE_PAUSE==tmpstate);
			}
			//睡眠
			int Sleep(int sec)
			{
				int ret;
				state=STATE_PAUSE;
				state_time=time(NULL);
				signal(SIGCONT,sig_default);
				++sleep_count;
				ret=sleep(sec);
				sig_time=0;//把处理过的信号时间清掉
				state=STATE_RUN;
				state_time=time(NULL);
				return ret;
			}
			//唤醒，同一秒最多发一次信号，如果信号丢失，下一秒可以再次发送信号
			void wakeup()
			{
				if(pid>0 && STATE_PAUSE==state)
				{
					if(time(NULL)!=sig_time)
					{
						sig_time=time(NULL);
						kill(pid,SIGCONT);
						DEBUG_LOG<<"发送信号"<<endi;
					}
					else
					{
						DEBUG_LOG<<"上次信号尚未处理"<<endi;
					}
				}
				else
				{
					DEBUG_LOG<<"进程不存在或不是暂停中"<<endi;
				}
			}
			//命令处理
			void process_cmd(long seq,long sec,bool notAutoSleep)
			{
				if(seq!=cmd_seq)
				{
					//thelog<<"命令序列号已更改，需要重新处理"<<endi;
					return;
				}
				switch(cmd)
				{
				case CMD_NULL:
					if(!notAutoSleep)Sleep(sec);
					break;
				case CMD_EXIT:
					thelog<<"收到退出命令，进程退出"<<endi;
					state=STATE_EXITED;
					state_time=time(NULL);
					exit_code=0;
					exit(0);
					break;
				case CMD_PAUSE:
					//thelog<<"pause..."<<endi;
					Sleep(15);
					break;
				default:
					thelog<<"未知的命令 "<<CmdStr(cmd)<<ende;
					Sleep(sec);
					break;
				}
			}
			//绑定CPU
			void bindprocessor(long cpu)
			{
				if(DEBUG_ENABLE_BINDCPU)
				{
					char buf[256];
					sprintf(buf,"bindprocessor %ld %ld",(long)pid,cpu);
					myShellExecute(buf,"");
				}
			}
		};
		struct JOBINFOS_T
		{
		private:
			struct jobinfo_t
			{
				long count_set_used;//计数
				long count_set_idle;//计数

				bool isUsed;
				bool isDistributeEnd;//分发结束
				long c_distribute;//分发数
				long c_rollup;//汇总数
				time_t start_time;//开始时间
				time_t end_time;//结束时间
				T_JOBINFO jobinfo;
			};

			jobinfo_t jobinfos[MAX_JOB];
			long size;

		public:
			string & toString(string & ret)const
			{
				ret="";
				char buf[2048];
				string str;

				sprintf(buf,"MAX_JOB[%ld] 已用[%ld]",MAX_JOB,size);
				ret+=buf;
				for(long i=0;i<size;++i)
				{
					long speed;//每分钟处理的任务数
					if(0==jobinfos[i].start_time)speed=0;
					else
					{
						time_t tmptime;
						if(0==jobinfos[i].end_time)tmptime=time(NULL);
						else tmptime=jobinfos[i].end_time;
						tmptime-=jobinfos[i].start_time;
						if(0==tmptime)tmptime=1;
						speed=jobinfos[i].c_rollup*60/tmptime;
					}
					sprintf(buf,"\n%ld %s(%ld %ld) %s 分发[%ld] 汇总[%ld] %ld(Task/min) start[%s] end[%s]: "
						,i,(jobinfos[i].isUsed?"在用":"空闲"),jobinfos[i].count_set_used,jobinfos[i].count_set_idle
						,(jobinfos[i].isUsed?(jobinfos[i].isDistributeEnd?"分发完毕":"正在分发"):"----"),jobinfos[i].c_distribute,jobinfos[i].c_rollup
						,speed,maketimestr(jobinfos[i].start_time).c_str(),maketimestr(jobinfos[i].end_time).c_str());
					ret+=buf;
					ret+=jobinfos[i].jobinfo.toString(str);
				}
				return ret;
			}
			long GetCountSetUesd()const
			{
				long ret = 0;
				for (long i = 0; i < size; ++i)
				{
					ret += jobinfos[i].count_set_used;
				}
				return ret;
			}
			long GetCountSetIdle()const
			{
				long ret = 0;
				for (long i = 0; i < size; ++i)
				{
					ret += jobinfos[i].count_set_idle;
				}
				return ret;
			}
			T_JOBINFO * GetJobInfo(long i_job){return &jobinfos[i_job].jobinfo;}
			long GetIdleJobInfo()
			{
				STATIC_G long i=0;
				if(size<MAX_JOB)
				{
					++size;
					return size-1;
				}
				if(i<MAX_JOB && !jobinfos[i].isUsed)
				{
					return i++;
				}
				for(i=0;i<size;++i)
				{
					if(!jobinfos[i].isUsed)return i++;
				}
				return -1;
			}
			void SetUsed(CMyRWMutex & mutex,long i_job)
			{
				mutex.RLock();
				jobinfo_t * p=&jobinfos[i_job];
				p->isDistributeEnd=false;
				p->c_distribute=0;
				p->c_rollup=0;
				p->start_time=time(NULL);
				p->end_time=0;
				++p->count_set_used;
				p->isUsed=true;
				mutex.RUnLock();
				//thelog<<"JOB "<<i_job<<" 开始使用"<<endi;
			}
			bool isIdle(long i_job)const{return !jobinfos[i_job].isUsed;}
			void SetIdle(CMyRWMutex & mutex,long i_job)
			{
				mutex.RLock();
				jobinfos[i_job].end_time=time(NULL);
				jobinfos[i_job].isUsed=false;
				++jobinfos[i_job].count_set_idle;
				AddRollupCount(i_job,true);
				mutex.RUnLock();
			}
			void SetDistributeEnd(long i_job){jobinfos[i_job].isDistributeEnd=true;}
			void AddDistributeCount(long i_job){++jobinfos[i_job].c_distribute;}
			void AddRollupCount(long i_job, bool clearcache)
			{
				STATIC_G long count[MAX_JOB];
				if(clearcache)
				{
					count[i_job]=0;
					//thelog<<"JOB "<<i_job<<" 缓存清理"<<endi;
				}
				else
				{
					if(count[i_job]!=jobinfos[i_job].c_rollup)
					{
						thelog<<"汇总数出错 "<<i_job<<" "<<count[i_job]<<" "<<jobinfos[i_job].c_rollup<<ende;
						string tmp;
						thelog<<toString(tmp)<<ende;
						SleepSeconds(5);
						thelog<<toString(tmp)<<ende;
						exit(__LINE__);
					}
					++count[i_job];
					++jobinfos[i_job].c_rollup;
					if(count[i_job]!=jobinfos[i_job].c_rollup)
					{
						thelog<<"汇总数出错 "<<i_job<<" "<<count[i_job]<<" "<<jobinfos[i_job].c_rollup<<ende;
						exit(__LINE__);
					}
				}
			}
			bool isNewRollup(long i_job){return 0==jobinfos[i_job].c_rollup;}
			bool isRollupEnd(long i_job)
			{
				jobinfo_t * p=&jobinfos[i_job];
				return p->isDistributeEnd && p->c_distribute==p->c_rollup;
			}
		};
		struct TASK_WARP
		{
			unsigned char ____1;
			T_TASKINFO task;
			unsigned char ____2;
			TASK_WARP() :____1(CHAR_0xE3), ____2(CHAR_0xE3){}
		};
		struct TASK_BLOCK_T
		{
		public:
			TASK_BLOCK_T():error_task_index(0){};
		public:
			TASK_STATE_T task_state;//volatile
			long i_job;//所属的job索引
			long checksum;//task_info校验和
			long count_ti_distribute_task;
			long count_ti_process_task;
			long count_ti_rollup_task;
			long tasks;//本块中有效的任务个数
			long error_task_index; //出现错误时的task下标
			long hash;
			TASK_WARP task_info[MAX_BLOCK_TASK];
			
		private:
			//校验和结构：seq尾数+step+校验和
			//step:0分发 1处理 2汇总
			long makeCheckSum(long seq,long step)
			{
				long ret = 0;
				unsigned char sum=0;
				for(long i_task=0;i_task<tasks;++i_task)
				{
					for (size_t i = 0; i < sizeof(T_TASKINFO); ++i)
					{
						sum += ((unsigned char *)&task_info[i_task].task)[i];
						if (i>64)i += 30;//抽样校验，否则速度很慢
					}
				}
				ret=seq*10000+step*1000+sum;
				return ret;
			}
		public:
			void setCheckSum(long seq,long step)
			{
				checksum=makeCheckSum(seq,step);
			}
			bool CheckSum(long seq,long step)
			{
				long shmsum=checksum;
				long sum=makeCheckSum(seq,step);
				bool ret=true;
				if(0!=shmsum)
				{
					if(0!=seq)
					{
						if(sum!=shmsum)
						{
							ret=false;
						}
					}
					else
					{
						if(sum%10000!=shmsum%10000)
						{
							ret=false;
						}
					}
				}
				if(!ret)thelog<<"校验出错，内存尚未同步 "<<seq<<" "<<step<<" "<<sum<<" "<<shmsum<<endi;
				return ret;
			}
			void clear()
			{
				task_state=TASK_STATE_NULL;
				checksum=0;
			}
			string & toString(string & ret)const
			{
				return report(ret);
			}
			string & report(string & ret,bool taskinfo=false)const
			{
				ret="";
				char buf[2048];
				sprintf(buf, "%d:%ld:%ld:%ld(%ld/%ld/%ld):%ld ", task_state, i_job, checksum, hash
					, count_ti_distribute_task, count_ti_process_task, count_ti_rollup_task, tasks);
				ret+=buf;
				string str;
				if(taskinfo)ret+=task_info[0].task.toString(str);
				return ret;
			}
		};
		struct PROCESS_DATA
		{
			PID_DATA pid_data;
			long count_all_distribute_task;
			long count_all_process_task;
			long count_all_rollup_task;
			long count_route_all;//所有循环
			long count_route_nothing;//空循环
			timeval timeval_process;//处理所用的时间
			long nextDistribute;//下一个分发位置
			long nextProcess;//下一个处理位置
			long nextRollup;//下一个汇总位置
			
			TASK_BLOCK_T m_taskblocks[MAX_PROCESS_TASK_BLOCK];
			
			string & report(string & ret,bool detail=true,bool taskinfo=false)const
			{
				ret="";
				string str;
				char buf[2048];

				ret+=pid_data.toString(str)+"\n";
				sprintf(buf,"    Total 分发 %ld 处理 %ld 汇总%ld Process循环 %ld 其中空循环 %ld(%ld%%) next(%ld %ld %ld)有效时间 %ld %ld\n"
					,count_all_distribute_task,count_all_process_task,count_all_rollup_task,count_route_all,count_route_nothing,count_route_nothing*100/(0==count_route_all?1:count_route_all)
					, nextDistribute, nextProcess, nextRollup
					,timeval_process.tv_sec,timeval_process.tv_usec);
				ret+=buf;
				long c_null=0;
				long c_new=0;
				long c_done=0;
				long c_unknown=0;
				ret+="    ";
				for(long i=0;i<MAX_PROCESS_TASK_BLOCK;++i)
				{
					switch(m_taskblocks[i].task_state)
					{
					case TASK_STATE_NULL:
						++c_null;
						break;
					case TASK_STATE_NEW:
						++c_new;
						break;
					case TASK_STATE_DONE:
						++c_done;
						break;
					default:
						++c_unknown;
						break;
					}
					if(detail)
					{
						ret+=m_taskblocks[i].report(str,taskinfo);
					}
				}
				ret+="\n";
				sprintf(buf, "    NULL: %ld NEW: %ld DONE: %ld", c_null, c_new, c_done);
				ret+=buf;
				if(0!=c_unknown)
				{
					sprintf(buf," ???: %ld",c_unknown);
					ret+=buf;
				}
				return ret;
			}
			string & toString(string & ret)const
			{
				return report(ret);
			}
			void clear()
			{
				for(long i=0;i<MAX_PROCESS_TASK_BLOCK;++i)
				{
					m_taskblocks[i].clear();
				}
			}
		};
		//位于共享内存的存储结构
		struct MPS_DATA
		{
			CMyRWMutex m_sync;

			long iGroup;//服务组的服务索引
			sstring<256> msg;//重要信息

			bool DEBUG_ENABLE_SYNC;//使用同步信号量
			bool DEBUG_ENABLE_CHECKSUM;//使用校验和
			bool exitIfNoJob;//退出控制：无job就退出
			bool notAutoSleep;//禁用自动睡眠，可以提高吞吐率，但多占用CPU
			bool notAutoSleep_distribute;//分发进程禁用自动睡眠，可以提高吞吐率，但多占用CPU
			bool notAutoSleep_rollup;//汇总进程禁用自动睡眠，可以提高吞吐率，但多占用CPU
			long max_process;//允许使用的最大处理进程数
			bool exitIfProcessCoredump;//处理进程coredump时退出，否则自动重起
			bool autoHighLow;//自动高低水控制
			
			PID_DATA m_distribute_data;//分发进程的进程控制信息
			long count_job_open;//job计数
			long count_all_task;//所有task计数
			long count_current_task;//当前job的task计数
			long count_no_idle;//没有空闲次数统计
			long max_used_process;//一段时间内使用的最大进程号，该值可由外部清除
			timeval timeval_distribute;//分发所用的时间
			timeval timeval_distribute_OpenClose;//分发所用的时间
			bool _in_auto_job;

			PID_DATA m_rollup_data;//汇总进程的进程控制信息
			long count_job_close;//job计数
			long count_all_rollup;
			long count_rollup_route_all;//所有循环
			long count_rollup_route_nothing;//空循环
			timeval timeval_rollup;//汇总所用的时间
			timeval timeval_rollup_OpenClose;//汇总所用的时间

			long count_taskinfo_new;//待处理/处理中
			long count_taskinfo_done_err;//待汇总/汇总中/处理出错

			JOBINFOS_T m_jobs;//job信息
			T_SERVERINFO m_server_info;
			PROCESS_DATA m_process_datas[MAX_PROCESS];//处理进程的数据
		
			string & report(string & ret,bool process=true,bool process_detail=true,bool taskinfo=false)const
			{
				ret="";

				char line[]="--------------------------------------------------------\n";
				char buf[2048];
				string str;

				ret+=line;
				sprintf(buf, "服务组服务号 %ld", iGroup);
				ret += buf;
				ret+="\n";
				ret+="重要信息：";
				ret += msg.c_str();
				ret += "\n";
				sprintf(buf,"运行模式：%s %s[D%d P%d R%d] %s %s\n",(exitIfNoJob?"批处理，无JOB退出":"服务，无JOB等待")
					,"循环处理，不放弃CPU/自动睡眠，不浪费CPU",notAutoSleep_distribute,notAutoSleep,notAutoSleep_rollup
					,(exitIfProcessCoredump?"处理进程coredump全部退出":"处理进程coredump重起")
					,(autoHighLow?"自动高低水":"平均分配任务"));
				ret+=buf;
				sprintf(buf,"调试开关：%s %s %s\n",(DEBUG_ENABLE_BINDCPU?"绑定CPU":"不绑定CPU"),(DEBUG_ENABLE_SYNC?"使用同步":"不使用同步"),(DEBUG_ENABLE_CHECKSUM?"校验":"不校验"));
				ret+=buf;
				ret+="分发进程："+m_distribute_data.toString(str);
				ret+="\n";
				sprintf(buf,"    job_open[%ld] all_task[%ld] 当前job[%ld] 分发失败[%ld](%ld%%)"
					,count_job_open,count_all_task,count_current_task,count_no_idle,count_no_idle*100/(0==count_all_task?1:count_all_task));
				ret+=buf;
				ret+="\n";
				ret+="汇总进程："+m_rollup_data.toString(str);
				ret+="\n";
				sprintf(buf,"    job_close[%ld] 汇总[%ld] 循环[%ld] 其中空循环[%ld](%ld%%)"
					,count_job_close,count_all_rollup,count_rollup_route_all,count_rollup_route_nothing,count_rollup_route_nothing*100/(0==count_rollup_route_all?1:count_rollup_route_all));
				ret+=buf;
				ret+="\n";
				sprintf(buf,"当前TASK分发失败时 new[%ld] down/err[%ld]\n",count_taskinfo_new,count_taskinfo_done_err);
				ret+=buf;
				ret+="job数据：\n";
				ret+=m_jobs.toString(str)+"\n";
				ret+="用户服务数据："+m_server_info.toString(str);
				ret+="\n";
				ret+=line;
				sprintf(buf,"进程空间 %ld 允许使用 %ld",MAX_PROCESS,max_process);
				ret+=buf;
				if(process)
				{
					for(long i=0;i<MAX_PROCESS;++i)
					{
						sprintf(buf,"\nProcess %ld : ",i);
						ret+=buf;
						ret+=m_process_datas[i].report(str,process_detail,taskinfo);
					}
				}
				else
				{//只显示第一个
					for(long i=0;i<MAX_PROCESS && i<1;++i)
					{
						sprintf(buf,"\nProcess %ld : ",i);
						ret+=buf;
						ret+=m_process_datas[i].report(str,process_detail,taskinfo);
					}
				}
				ret+="\n";
				ret+=line;
				return ret;
			}
			string & toString(string & ret)const
			{
				return report(ret);
			}
			void clear()
			{
				DEBUG_ENABLE_SYNC = true;
				for(long i=0;i<MAX_PROCESS;++i)
				{
					m_process_datas[i].clear();
				}
			}
			bool init_sync()
			{
				if (!m_sync.Create())
				{
					thelog << "创建信号量失败" << ende;
					return false;
				}
				return true;
			}
			bool sync(bool isEnd)
			{
				if (!DEBUG_ENABLE_SYNC)return true;

				if(!isEnd)m_sync.RLock();
				else m_sync.RUnLock();
				return true;
			}
		};
	private:
		IDistributeTask<T_JOBINFO,T_TASKINFO > * pDistribute;
		IProcessTask<T_JOBINFO,T_TASKINFO > * pProcess;
		IRollupTask<T_JOBINFO,T_TASKINFO > * pRollup;
		MPS_DATA * pMPSData;//指向共享内存
		bool isInPause;//已经处于暂停命令中
		long m_taskinfo_seq[MAX_PROCESS][MAX_PROCESS_TASK_BLOCK];//私有的task顺序号信息，用于确认数据
	private:
		pid_t myfork()
		{
			pid_t ret=fork();
			return ret;
		}
		void reset_private_data(long step)
		{
			for(long i=0;i<MAX_PROCESS;++i)
			{
				for(long j=0;j<MAX_PROCESS_TASK_BLOCK;++j)
				{
					long count;
					switch(step)
					{
					case 0:
						count=pMPSData->m_process_datas[i].m_taskblocks[j].count_ti_distribute_task;
						break;
					case 1:
						count=pMPSData->m_process_datas[i].m_taskblocks[j].count_ti_process_task;
						break;
					case 2:
						count=pMPSData->m_process_datas[i].m_taskblocks[j].count_ti_rollup_task;
						break;
					default:
						count=0;
						break;

					}
					m_taskinfo_seq[i][j]=count;
				}
			}
		}
		bool CheckMainPID()
		{
			if(!pMPSData->m_distribute_data.isRuning())
			{
				thelog<<"分发进程已经异常退出"<<ende;
				return false;
			}
			if(!pMPSData->m_distribute_data.isOK())
			{
				pMPSData->m_distribute_data.setState(pMPSData->m_sync,STATE_COREDUMP);
				thelog<<"检测到分发进程异常退出"<<ende;
				return false;
			}
			return true;
		}
		//检查状态并返回是否运行失败需要退出（与模式设定有关）
		bool CheckChild()
		{
			bool ret=true;
			int stat;
			while(waitpid(-1,&stat,WNOHANG)>0){}//获取所有已经结束的子进程的状态
			//if(waitpid(-1,&stat,WNOHANG)>0) { 
			//	thelog<<"有子进程结束"<<endi;
			//}
			
			long i,j;
			if(!pMPSData->m_rollup_data.isOK())
			{
				pMPSData->m_rollup_data.setState(pMPSData->m_sync,STATE_COREDUMP);
				thelog<<"汇总进程异常退出"<<ende;
				ret=false;
			}

			for(i=0;i<MAX_PROCESS;++i)
			{
				bool needexit=false;

				for(j=0;j<MAX_PROCESS_TASK_BLOCK;++j)
				{
					//有任务的必须处于正常状态
					if(TASK_STATE_NEW==pMPSData->m_process_datas[i].m_taskblocks[j].task_state)
					{
						if(!pMPSData->m_process_datas[i].pid_data.isOK() || !pMPSData->m_process_datas[i].pid_data.isRuning())
						{
							thelog<<"处理进程 "<<i<<" 异常，因为有任务 "<<j<<" 需要处理但进程不在运行中"<<ende;
							needexit=true;
							ret=false;
						}
					}
				}
				if(!pMPSData->m_process_datas[i].pid_data.isOK())
				{
					pMPSData->m_process_datas[i].pid_data.setState(pMPSData->m_sync,STATE_COREDUMP);
					thelog<<"处理进程"<<i<<"异常退出，pid="<<pMPSData->m_process_datas[i].pid_data.pid<<ende;
					if(pMPSData->exitIfProcessCoredump)
					{
						needexit=true;
						ret=false;
					}
					else
					{
						thelog<<"将自动重起"<<endi;
						if(!CreateOrWakeupProcessIfNeed(i))
						{
							ret=false;
						}
					}
				}
				if(needexit)
				{
					for(j=0;j<MAX_PROCESS_TASK_BLOCK;++j)
					{
						//必须清理掉coredump进程的任务，否则汇总进程不能退出
						pMPSData->m_process_datas[i].m_taskblocks[j].task_state=TASK_STATE_NULL;
					}
				}
			}
			return ret;
		}
		void SetAllChildCmd(COMMAND_T cmd)
		{
			long i;
			if(pMPSData->m_rollup_data.getCmd()!=cmd)
			{
				pMPSData->m_rollup_data.setCmd(pMPSData->m_sync,cmd);
			}

			for(i=0;i<MAX_PROCESS;++i)
			{
				if(pMPSData->m_process_datas[i].pid_data.getCmd()!=cmd)
				{
					pMPSData->m_process_datas[i].pid_data.setCmd(pMPSData->m_sync,cmd);
				}
			}
		}
		void wakeupAllChild()
		{
			long i;
			pMPSData->m_rollup_data.wakeup();

			for(i=0;i<MAX_PROCESS;++i)
			{
				pMPSData->m_process_datas[i].pid_data.wakeup();
			}
		}
		int countTaskNotFinish()
		{
			long i,j;
			long count=0;
			for(i=0;i<MAX_PROCESS;++i)
			{
				for(j=0;j<MAX_PROCESS_TASK_BLOCK;++j)
				{
					TASK_BLOCK_T * pTaskBlock=&pMPSData->m_process_datas[i].m_taskblocks[j];

					if(pTaskBlock->task_state!=TASK_STATE_NULL)
					{
						++count;
					}
				}
			}
			return count;
		}
		//失败则系统退出
		bool waitAllTaskFinish()
		{
			long countRun;
			string str;
			thelog<<"检查工作是否都已完成..."<<endi;
			while(0!=(countRun=countTaskNotFinish()))
			{
				thelog<<"还有 "<<countRun<<" 个task未完成"<<endi;
				if(!CheckChild())return false;
				wakeupAllChild();
				pMPSData->m_distribute_data.Sleep(2);
			}
			return true;
		}
		bool StopAllChild()
		{
			string str;
			long i;

			SetAllChildCmd(CMD_EXIT);
			wakeupAllChild();

			bool isAllExited=false;
			while(!isAllExited)
			{
				CheckChild();
				thelog<<"等待全部子进程退出..."<<endi;
				pMPSData->m_distribute_data.Sleep(3);
				isAllExited=true;
				if(pMPSData->m_rollup_data.isRuning())
				{
					isAllExited=false;
					thelog<<"汇总进程尚未退出"<<endi;
				}

				for(i=0;i<MAX_PROCESS;++i)
				{
					if(pMPSData->m_process_datas[i].pid_data.isRuning())
					{
						isAllExited=false;
						thelog<<"进程 "<<i<<" 尚未退出"<<endi;
					}
				}
				//thelog<<endl<<toString(str)<<endi;
			}
			thelog<<"全部子进程已退出"<<endi;
			return true;
		}
		//如果进程未创建则创建处理进程
		bool CreateOrWakeupProcessIfNeed(long iProcess)
		{
			pMPSData->m_rollup_data.wakeup();//首先唤醒汇总进程

			PROCESS_DATA * pProcess=&pMPSData->m_process_datas[iProcess];
			char buf[256];
			pid_t pid;
			STATIC_G long create_key=0;//子进程的识别码
			if(0==create_key)create_key=getpid();

			switch(pProcess->pid_data.getState(pMPSData->m_sync))
			{
			case STATE_PAUSE:
				pProcess->pid_data.wakeup();
				break;
			case STATE_EXITED:
				thelog<<"子进程已退出，需要重起"<<endi;
				//falling down
			case STATE_COREDUMP:
				thelog<<"子进程coredump，需要重起"<<endi;
				//falling down
			case STATE_NULL:
				++create_key;
				pProcess->pid_data.pid=0;
				pProcess->pid_data.setState(pMPSData->m_sync,STATE_CREATE);
				pProcess->pid_data.setCmd(pMPSData->m_sync,CMD_NULL);
				++pProcess->pid_data.create_seq;
				++pProcess->pid_data.create_key=create_key;
				if((pid=myfork())<0)
				{
					thelog<<"创建子进程失败"<<ende;
					return false;
				}
				else if(0==pid)
				{//子进程
					pProcess->pid_data.pid=getpid();
					pProcess->pid_data.setState(pMPSData->m_sync,STATE_RUN);
					
					sprintf(buf, "%ld-%2ld", pMPSData->iGroup, iProcess);
					theLog.SetSource(buf);
					thelog<<"子进程 "<<iProcess<<" 启动 pid="<<pProcess->pid_data.pid<<endi;

					pProcess->pid_data.bindprocessor(iProcess+2);
					exit(doProcess(iProcess,create_key));//转入子进程处理
				}
				else
				{//父进程
					while(STATE_CREATE==pProcess->pid_data.getState(pMPSData->m_sync))
					{
					}
					thelog<<"子进程 "<<iProcess<<" 创建成功 pid="<<pid<<endi;
				}
				break;
			default:
				break;
			}
			return true;
		}
		//查找空闲位置，p_iProcess返回进程索引，p_iTaskInfo返回taskinfo索引
		TASK_BLOCK_T * _FindIdelTaskInfo(long iHash, long * p_iProcess, long * p_iTaskInfo)
		{
			STATIC_G long i_process=0;
			TASK_BLOCK_T * ret=NULL;

			long c_new=0;
			long c_done_err=0;
			for(long i=0;i<pMPSData->max_process;++i)
			{
				if(pMPSData->autoHighLow)
				{
					i_process=i;//自动高低水总是从第一个进程开始分配，最后的进程得不到任务就睡眠
				}
				if (iHash >= 0)
				{
					i = pMPSData->max_process - 1;//将只循环一次
					i_process = iHash;//只考虑给定的进程
				}
				//for(long j=0;j<MAX_PROCESS_TASK_BLOCK;++j)
				long j = pMPSData->m_process_datas[i_process].nextDistribute;
				if (j >= MAX_PROCESS_TASK_BLOCK)j = 0;
				if (j < 0)j = 0;
				{
					long tmpstate=pMPSData->m_process_datas[i_process].m_taskblocks[j].task_state;
					switch(tmpstate)
					{
					case TASK_STATE_NULL:
						pMPSData->m_process_datas[i_process].nextDistribute = j + 1;
						//cout << MAX_PROCESS_TASK_BLOCK << " " << i_process << " " << pMPSData->m_process_datas[i_process].nextDistribute << endl;
						ret=&pMPSData->m_process_datas[i_process].m_taskblocks[j];
						if (pMPSData->DEBUG_ENABLE_CHECKSUM)if (!ret->CheckSum(m_taskinfo_seq[i_process][j], 2))return NULL;//校验汇总结果出错，内存尚未同步
						*p_iProcess=i_process;
						*p_iTaskInfo=j;
						if(i_process>pMPSData->max_used_process)pMPSData->max_used_process=i_process;
						++i_process;
						i_process = i_process % pMPSData->max_process;
						return ret;
						break;
					case TASK_STATE_NEW:
						++c_new;
						break;
					case TASK_STATE_DONE:
						++c_done_err;
						break;
					default:
						break;
					}
				}

				++i_process;
				i_process = i_process % pMPSData->max_process;
			}
			pMPSData->count_taskinfo_new=c_new;
			pMPSData->count_taskinfo_done_err=c_done_err;
			return NULL;
		}
		TASK_BLOCK_T * FindIdelTaskInfo(long iHash, long * p_iProcess, long * p_iTaskInfo)
		{
			STATIC_G long c_checkchild=0;
			TASK_BLOCK_T * pTaskBlock;
			bool isIdleCounted=false;//每个task分发失败只计一次
			while (NULL == (pTaskBlock = _FindIdelTaskInfo(iHash, p_iProcess, p_iTaskInfo)))
			{
				if(0==c_checkchild++%10000)
				{
					if(!CheckChild())return NULL;
				}
				if(!isIdleCounted)
				{
					++pMPSData->count_no_idle;
					isIdleCounted=true;
				}
				if(!pMPSData->notAutoSleep_distribute)
				{
					pMPSData->m_distribute_data.Sleep(0);
				}
			}
			return pTaskBlock;
		}
		bool rullup_closejob_if_need(long i_job)
		{
			if(!pMPSData->m_jobs.isIdle(i_job) && pMPSData->m_jobs.isRollupEnd(i_job))
			{
				if(!pRollup->IRollupTask_CloseJob_Rollup(&pMPSData->timeval_distribute_OpenClose,pMPSData->m_jobs.GetJobInfo(i_job)))
				{
					thelog<<"CloseJob_Rollup出错"<<ende;
					return false;
				}
				pMPSData->m_jobs.SetIdle(pMPSData->m_sync,i_job);
				++pMPSData->count_job_close;

				//校验job计数是否正常
				if(pMPSData->count_job_close!=pMPSData->m_jobs.GetCountSetIdle())
				{
					char buf[2048];
					sprintf(buf,"出错：job关闭计数不正常 汇总记载 %ld job记载 %ld",pMPSData->count_job_close,pMPSData->m_jobs.GetCountSetIdle());
					pMPSData->msg=buf;
					thelog<<buf<<ende;
				}
			}
			return true;
		}
		int doRollup()
		{
			string str;
			bool tmpbool;
			bool hadRollupError=false; //是否已有汇总出错
			//TASK_BLOCK_T tmptaskinfo;

			int countFlag=0;
			int allCountFlag=pMPSData->max_process*MAX_PROCESS_TASK_BLOCK;

			while(true)
			{
				pMPSData->m_rollup_data.cpu_clock = clock();
				++pMPSData->count_rollup_route_all;
				bool isTaskRun=false;//是否有正在处理不能汇总的任务（即有数据正在处理，进程不能退出）
				long cmdseq=pMPSData->m_rollup_data.getCmdSeq();

				TASK_BLOCK_T * pTaskBlock;
				bool needCheckJob;//是否需要处理TASK对应的JOB
				long i_job;
				long i;
				bool hadDoNothing=true;//一个循环下来什么也没做
				
				for(i=0;i<MAX_PROCESS;++i)
				{
					PROCESS_DATA * process = &pMPSData->m_process_datas[i];
					for (long ii = 0, j = process->nextRollup % MAX_PROCESS_TASK_BLOCK; ii < MAX_PROCESS_TASK_BLOCK; ++ii)
					{
						pMPSData->m_rollup_data.cpu_clock = clock();
						pTaskBlock=&pMPSData->m_process_datas[i].m_taskblocks[j];

						needCheckJob=false;
						pMPSData->sync(false);
						TASK_STATE_T taskstate=pTaskBlock->task_state;
						i_job = pTaskBlock->i_job;//因为这个在设置pTaskBlock状态之后就无效，所以要预先保存
						pMPSData->sync(true);
						long i_task;//BLOCK内的任务循环变量
						switch(taskstate)
						{
						case TASK_STATE_NULL:
							break;
						case TASK_STATE_NEW:
							isTaskRun=true;
							break;
						case TASK_STATE_DONE:
							hadDoNothing=false;
							process->nextRollup = j + 1;
							if(hadRollupError) 
							{//已发生汇总错误，不再进行汇总输出，转到处理进程回退
								//thelog<<"进程["<<i<<"],出错block["<<j<<"]设置为需要回退状态"<<endi;
								countFlag++;
								pMPSData->sync(false);
								pTaskBlock->task_state = TASK_STATE_UNDO;
								pMPSData->sync(true);
								break;
							}
							if (pMPSData->DEBUG_ENABLE_CHECKSUM)if (!pTaskBlock->CheckSum(m_taskinfo_seq[i][j] + 1, 1))break;//校验处理结果出错，内存尚未同步
							hadDoNothing=false;
							needCheckJob=true;
						
							if(pMPSData->m_jobs.isNewRollup(i_job))
							{
								if(!pRollup->IRollupTask_OpenJob_Rollup(&pMPSData->timeval_distribute_OpenClose,pMPSData->m_jobs.GetJobInfo(i_job)))
								{
									thelog<<"OpenJob_Rollup出错"<<ende;
									return pMPSData->m_rollup_data.setExit(pMPSData->m_sync,__LINE__);
								}
							}
							for (i_task = 0; i_task < pTaskBlock->tasks; ++i_task)
							{
								if (pTaskBlock->task_info[i_task].____1 != CHAR_0xE3 || pTaskBlock->task_info[i_task].____2 != CHAR_0xE3)
								{
									thelog<<"TASK数据溢出！"<<ende;
								}
								if (!pRollup->IRollupTask_Rollup(&pMPSData->timeval_rollup, pMPSData->m_jobs.GetJobInfo(i_job), &pTaskBlock->task_info[i_task].task, &tmpbool))
								{
									pMPSData->sync(false);
									pTaskBlock->error_task_index=i_task;
									pTaskBlock->task_state = TASK_STATE_UNDO;
									pMPSData->sync(true);
									hadRollupError=true;
									countFlag++;
									needCheckJob=false; //一旦出错，不需要再检查
									thelog << "Rollup出错,出错进程["<<i<<"],出错block["<<j<<"],出错task["<<i_task<<"]"<<ende;
									break;
									//return pMPSData->m_rollup_data.setExit(pMPSData->m_sync,__LINE__);
								}
								if (pTaskBlock->task_info[i_task].____1 != CHAR_0xE3 || pTaskBlock->task_info[i_task].____2 != CHAR_0xE3)
								{
									thelog<<"TASK数据溢出！"<<ende;
								}
								++pMPSData->m_process_datas[i].count_all_rollup_task;
								++pTaskBlock->count_ti_rollup_task;
								++pMPSData->count_all_rollup;
								pMPSData->m_jobs.AddRollupCount(i_job, false);
							}
							if(!hadRollupError) 
							{//没发生汇总错误时
								++m_taskinfo_seq[i][j];//汇总序列号，与共享内存作校验
								//if (pMPSData->DEBUG_ENABLE_CHECKSUM)pTaskBlock->setCheckSum(m_taskinfo_seq[i][j], 2);//设置汇总校验和
								pMPSData->sync(false);
								pTaskBlock->task_state = TASK_STATE_NULL;
								pMPSData->sync(true);
							}
							break;
						case TASK_STATE_UNDO:
							break;
						case TASK_STATE_CANCLE:
							break;
						default:
							thelog<<"process "<<i<<" task "<<j<<" 未知的task状态码 "<<TaskStateStr(taskstate)<<ende;
							break;
						}
						if(needCheckJob)
						{
							if(!rullup_closejob_if_need(i_job))
							{
								thelog<<"CloseJob_Rollup出错"<<ende;
								return pMPSData->m_rollup_data.setExit(pMPSData->m_sync,__LINE__);
							}
						}

						++j;
						j = j % MAX_PROCESS_TASK_BLOCK;
					}
				}
				if(hadDoNothing)
				{
					++pMPSData->count_rollup_route_nothing;
					if(!CheckMainPID())
					{
						thelog<<"汇总进程因为分发进程异常退出而退出"<<ende;
						return pMPSData->m_rollup_data.setExit(pMPSData->m_sync,__LINE__);
					}
					//检查空job
					for(i=0;i<MAX_JOB;++i)
					{
						if(!rullup_closejob_if_need(i))
						{
							thelog<<"CloseJob_Rollup出错"<<ende;
							return pMPSData->m_rollup_data.setExit(pMPSData->m_sync,__LINE__);
						}
					}
					if(hadRollupError && countFlag==allCountFlag) 
					{//当已发生汇总错误，并已全部设置block状态时
						thelog<<"任务已全部回退完毕，汇总进程退出"<<endi;
						return pMPSData->m_rollup_data.setExit(pMPSData->m_sync,__LINE__);
					}
				}
				//thelog<<endl<<toString(str)<<endi;
				//命令处理
				if(!isTaskRun)
				{
					pMPSData->m_rollup_data.process_cmd(cmdseq, 1, pMPSData->notAutoSleep_rollup);
				}
			}
			return 0;
		}
		int doProcess(long iProcess,long create_key)
		{
			reset_private_data(1);
			PROCESS_DATA * process=&pMPSData->m_process_datas[iProcess];
		
			string str;
			bool tmpbool;

			process->timeval_process.tv_sec=0;
			process->timeval_process.tv_usec=0;
			while(true)
			{
				if(process->pid_data.create_key!=create_key)
				{
					thelog<<"创建码 "<<create_key<<" 与共享内存不一致 "<<process->pid_data.create_key<<ende;
					return __LINE__;
				}
				++process->count_route_all;
				long cmdseq=process->pid_data.getCmdSeq();

				bool hadDoNothing=true;//一个循环下来什么也没做
				for (long ii = 0, i = process->nextProcess % MAX_PROCESS_TASK_BLOCK; ii < MAX_PROCESS_TASK_BLOCK; ++ii)
				{
					process->pid_data.cpu_clock = clock();
					TASK_BLOCK_T * pTaskBlock=&process->m_taskblocks[i];
					if(TASK_STATE_NEW==pTaskBlock->task_state)
					{
						if (pMPSData->DEBUG_ENABLE_CHECKSUM)if (!pTaskBlock->CheckSum(m_taskinfo_seq[iProcess][i] + 1, 0))continue;//校验分发结果出错，内存尚未同步
						hadDoNothing=false;

						for(long i_task=0;i_task<pTaskBlock->tasks;++i_task)
						{
							process->pid_data.cpu_clock = clock();
							++process->count_all_process_task;
							++pTaskBlock->count_ti_process_task;
							if (pTaskBlock->task_info[i_task].____1 != CHAR_0xE3 || pTaskBlock->task_info[i_task].____2 != CHAR_0xE3)
							{
								thelog << "TASK数据溢出！" << (unsigned int)pTaskBlock->task_info[i_task].____1 << " " << (unsigned int)pTaskBlock->task_info[i_task].____2 << ende;
							}
							if (!pProcess->IProcessTask_Process(&process->timeval_process, pMPSData->m_jobs.GetJobInfo(pTaskBlock->i_job), &pTaskBlock->task_info[i_task].task, &tmpbool))
							{
								thelog << "Process出错" << ende;
								return process->pid_data.setExit(pMPSData->m_sync,__LINE__);
							}
							if (pTaskBlock->task_info[i_task].____1 != CHAR_0xE3 || pTaskBlock->task_info[i_task].____2 != CHAR_0xE3)
							{
								thelog << "TASK数据溢出！" << (unsigned int)pTaskBlock->task_info[i_task].____1 << " " << (unsigned int)pTaskBlock->task_info[i_task].____2 << ende;
							}
						}
						++m_taskinfo_seq[iProcess][i];//处理序列号，与共享内存作校验
						if (pMPSData->DEBUG_ENABLE_CHECKSUM)pTaskBlock->setCheckSum(m_taskinfo_seq[iProcess][i], 1);//设置处理校验和
						pMPSData->sync(false);
						pTaskBlock->task_state=TASK_STATE_DONE;
						process->nextProcess = i + 1;
						pMPSData->sync(true);
					}
					else if(TASK_STATE_UNDO==pTaskBlock->task_state) 
					{//如果当前状态为回退
						hadDoNothing=false;
						for(long i_task=pTaskBlock->error_task_index;i_task<pTaskBlock->tasks;++i_task)
						{
							//thelog<<"回退当前记录，block["<<i<<"],task["<<i_task<<"]"<<endi;
							process->pid_data.cpu_clock = clock();
							++process->count_all_process_task;
							++pTaskBlock->count_ti_process_task;
							if (!pProcess->IProcessTask_UnProcess(&process->timeval_process, pMPSData->m_jobs.GetJobInfo(pTaskBlock->i_job), &pTaskBlock->task_info[i_task].task, &tmpbool))
							{
								thelog << "Process出错" << ende;
								return process->pid_data.setExit(pMPSData->m_sync,__LINE__);
							}
						}
						pMPSData->sync(false);
						pTaskBlock->task_state=TASK_STATE_CANCLE;
						pMPSData->sync(true);
					}

					++i;
					i = i % MAX_PROCESS_TASK_BLOCK;
				}
				//命令处理
				if(hadDoNothing)
				{
					++process->count_route_nothing;
					if(!CheckMainPID())
					{
						thelog<<"处理进程因为分发进程异常退出而退出"<<ende;
						return process->pid_data.setExit(pMPSData->m_sync,__LINE__);
					}
					process->pid_data.process_cmd(cmdseq,1,pMPSData->notAutoSleep);
				}
			}
			return 0;
		}
		void afterDistributeOpenJob(long i_job)
		{
			pMPSData->m_jobs.SetUsed(pMPSData->m_sync,i_job);
			++pMPSData->count_job_open;
			pMPSData->count_current_task=0;
			SetAllChildCmd(CMD_NULL);
			wakeupAllChild();

			//校验job计数是否正常
			if(pMPSData->count_job_open!=pMPSData->m_jobs.GetCountSetUesd())
			{
				char buf[2048];
				sprintf(buf,"出错：job打开计数不正常 分发记载 %ld job记载 %ld",pMPSData->count_job_open,pMPSData->m_jobs.GetCountSetUesd());
				pMPSData->msg = buf;
				thelog << buf << ende;
			}
		}
		void afterDistribueCloseJob(long i_job)
		{
			pMPSData->m_jobs.SetDistributeEnd(i_job);
			wakeupAllChild();
		}
		long HashToProcessIndex(long hash) 
		{
			if(hash<0)return -1;
			else return hash % pMPSData->max_process; 
		}
		int doDistribute_OneJob()
		{
			STATIC_G long c_checkchild=0;
			string str;
			bool tmpbool;

			long i_job=pMPSData->m_jobs.GetIdleJobInfo();
			if(i_job<0)
			{
				//thelog<<"没有空闲job位置可用"<<endi;
				//pMPSData->m_distribute_data.Sleep(1);
				return 0;
			}
			//打开一个job
			if(!pDistribute->IDistributeTask_OpenJob(&pMPSData->timeval_distribute_OpenClose,pMPSData->m_jobs.GetJobInfo(i_job),&tmpbool))
			{
				thelog<<"OpenJob出错，系统退出"<<ende;
				return __LINE__;
			}
			if(!tmpbool)
			{
				if(pMPSData->exitIfNoJob)
				{
					thelog<<"跟据运行模式，无JOB退出，设置退出命令"<<endi;
					pMPSData->m_distribute_data.setCmd(pMPSData->m_sync,CMD_EXIT);
				}
				//thelog<<"没有job可做"<<endi;
				pMPSData->m_distribute_data.Sleep(5);
				return 0;
			}
			afterDistributeOpenJob(i_job);
			DEBUG_LOG<<"打开一个job"<<endi;
			bool isJobEnd=false;//独立记录是否job结束，后面要保证isJobEnd调用和分发次数一致
			//处理job
			while (!isJobEnd)
			{
				pMPSData->m_distribute_data.cpu_clock = clock();
				if (0 == c_checkchild++ % 10000)
				{
					if(!CheckChild())return __LINE__;
				}
				if (!pDistribute->IDistributeTask_isJobEnd(&pMPSData->timeval_distribute, pMPSData->m_jobs.GetJobInfo(i_job), &tmpbool))
				{
					thelog << "isJobEnd出错，系统退出" << ende;
					return __LINE__;
				}
				if (tmpbool)
				{
					isJobEnd = true;
					DEBUG_LOG << "job分发结束" << endi;
					break;
				}
			
				TASK_BLOCK_T * pTaskBlock;
				long iHash = HashToProcessIndex(pDistribute->IDistributeTask_GetNextTaskHash(&tmpbool));
				long iProcess;
				long iTaskInfo;

				if(!tmpbool)
				{
					//thelog<<"无任务，继续等待"<<endi;
					continue;
				}
				
				if (NULL == (pTaskBlock = FindIdelTaskInfo(iHash, &iProcess, &iTaskInfo)))
				{//这个函数内部会循环等待
					return __LINE__;
				}
				if(TASK_STATE_NULL!=pTaskBlock->task_state)
				{
					thelog<<"出错 "<<iProcess<<" "<<iTaskInfo<<" "<<pTaskBlock->task_state<<ende;
				}
				pTaskBlock->tasks = 0;
				for(long i_task=0;i_task<MAX_BLOCK_TASK;++i_task)
				{
					pMPSData->m_distribute_data.cpu_clock = clock();
					
					if (!pDistribute->IDistributeTask_isJobEnd(&pMPSData->timeval_distribute, pMPSData->m_jobs.GetJobInfo(i_job), &tmpbool))
					{
						thelog << "isJobEnd出错，系统退出" << ende;
						return __LINE__;
					}
					if (tmpbool)
					{
						isJobEnd = true;
						DEBUG_LOG << "job分发结束" << endi;
						break;
					}
					
					long tmpHash = HashToProcessIndex(pDistribute->IDistributeTask_GetNextTaskHash(&tmpbool));
					if (!tmpbool)
					{
						//thelog << "无任务，跳出" << endi;
						break;
					}
					if (tmpHash != iHash)
					{
						DEBUG_LOG << "hash不一致，需要重新分配" << endi;
						break;
					}
					
					if (pTaskBlock->task_info[i_task].____1 != CHAR_0xE3 || pTaskBlock->task_info[i_task].____2 != CHAR_0xE3)
					{
						thelog << "TASK数据溢出！" << ende;
					}
					if (!pDistribute->IDistributeTask_DistributeTask(&pMPSData->timeval_distribute, pMPSData->m_jobs.GetJobInfo(i_job), &pTaskBlock->task_info[i_task].task, &tmpbool))
					{
						thelog << "DistributeTask出错，系统退出" << ende;
						return __LINE__;
					}
					if (pTaskBlock->task_info[i_task].____1 != CHAR_0xE3 || pTaskBlock->task_info[i_task].____2 != CHAR_0xE3)
					{
						thelog << "TASK数据溢出！" << ende;
					}
					if (!tmpbool)
					{
						thelog<<"分发失败，可能是出错或者JOB结束"<<endw;
						break;//分发失败，可能是job结束
					}
					++pTaskBlock->tasks;
			
					++pMPSData->m_process_datas[iProcess].count_all_distribute_task;
					++pTaskBlock->count_ti_distribute_task;
					pTaskBlock->hash = iHash;
					pMPSData->m_jobs.AddDistributeCount(i_job);
					++pMPSData->count_all_task;
					++pMPSData->count_current_task;
				}
				if(0==pTaskBlock->tasks)
				{
					continue;//job结束
				}
				if(TASK_STATE_NULL!=pTaskBlock->task_state)
				{
					thelog<<"出错 "<<iProcess<<" "<<iTaskInfo<<" "<<pTaskBlock->task_state<<ende;
				}
				pTaskBlock->i_job = i_job;
				++m_taskinfo_seq[iProcess][iTaskInfo];//分发序列号，与共享内存作校验
				if (pMPSData->DEBUG_ENABLE_CHECKSUM)pTaskBlock->setCheckSum(m_taskinfo_seq[iProcess][iTaskInfo], 0);//设置分发校验和
				pMPSData->sync(false);
				pTaskBlock->task_state = TASK_STATE_NEW;
				pMPSData->sync(true);
				if (!CreateOrWakeupProcessIfNeed(iProcess))
				{
					thelog << "CreateProcessIfNeed出错，系统退出" << ende;
					return __LINE__;
				}
				//thelog<<endl<<toString(str)<<endi;
			}
			//关闭job
			if(!pDistribute->IDistributeTask_CloseJob_Distribute(&pMPSData->timeval_distribute_OpenClose,pMPSData->m_jobs.GetJobInfo(i_job)))
			{
				thelog<<"CloseJob_Distribute出错，系统退出"<<ende;
				return __LINE__;
			}
			afterDistribueCloseJob(i_job);
			SetAllChildCmd(CMD_PAUSE);
			DEBUG_LOG<<"关闭job"<<endi;
			//thelog<<endl<<toString(str)<<endi;

			return 0;
		}
		int doDistribute()
		{
			string str;
			while(true)
			{
				int ret;
				
				if(!CheckChild())
				{
					StopAllChild();
					thelog<<"系统因为子进程异常退出而退出"<<endi;
					return pMPSData->m_distribute_data.setExit(pMPSData->m_sync,__LINE__);
				}

				switch(pMPSData->m_distribute_data.getCmd())
				{
				case CMD_NULL:
					isInPause=false;
					ret=doDistribute_OneJob();
					if(0!=ret)
					{
						StopAllChild();
						thelog<<"系统因为子进程异常退出而退出"<<endi;
						return pMPSData->m_distribute_data.setExit(pMPSData->m_sync,ret);
					}
					break;
				case CMD_PAUSE:
					if(!isInPause)
					{
						if(!waitAllTaskFinish())
						{
							StopAllChild();
							thelog<<"系统因为子进程异常退出而退出"<<endi;
							return pMPSData->m_distribute_data.setExit(pMPSData->m_sync,__LINE__);
						}
						SetAllChildCmd(CMD_PAUSE);
						isInPause=true;
						thelog<<"系统暂停"<<endi;
					}
					pMPSData->m_distribute_data.Sleep(1);
					break;
				case CMD_EXIT:
					isInPause=false;
					if(!waitAllTaskFinish())
					{
						StopAllChild();
						thelog<<"系统因为子进程异常退出而退出"<<endi;
						return pMPSData->m_distribute_data.setExit(pMPSData->m_sync,__LINE__);
					}
					StopAllChild();
					thelog<<"系统退出"<<endi;
					return pMPSData->m_distribute_data.setExit(pMPSData->m_sync,0);
					break;
				default:
					isInPause=false;
					//pMPSData->m_distribute_data.Sleep(1);
					break;
				}
			}
			//这里不应该被执行到
			StopAllChild();
			return 0;
		}
		bool init_distribute_and_rollup()
		{
			//设置分发进程
			thelog<<"分发进程 pid="<<getpid()<<endi;
			pMPSData->m_distribute_data.pid=getpid();
			pMPSData->m_distribute_data.setState(pMPSData->m_sync,STATE_RUN);
			pMPSData->m_distribute_data.setCmd(pMPSData->m_sync,CMD_NULL);
			pMPSData->m_distribute_data.bindprocessor(0L);
			pDistribute->_in_auto_job = &pMPSData->_in_auto_job;

			//创建汇总进程
			thelog<<"创建汇总进程..."<<endi;
			pid_t pid;
			char buf[256];

			pMPSData->m_rollup_data.setState(pMPSData->m_sync,STATE_CREATE);
			pMPSData->m_rollup_data.setCmd(pMPSData->m_sync,CMD_PAUSE);
			if((pid=myfork())<0)
			{
				thelog<<"创建子进程失败"<<ende;
				return false;
			}
			else if(0==pid)
			{//子进程
				sprintf(buf, "%ld-汇总", pMPSData->iGroup);
				theLog.SetSource(buf);
				thelog<<"汇总进程启动 "<<getpid()<<endi;

				pMPSData->m_rollup_data.pid=getpid();
				pMPSData->m_rollup_data.setState(pMPSData->m_sync,STATE_RUN);
				pMPSData->m_rollup_data.bindprocessor(1L);
				
				reset_private_data(2);
				exit(doRollup());
			}
			else
			{//父进程
				thelog << "汇总进程 pid=" << pid << endi;
				while (STATE_CREATE == pMPSData->m_rollup_data.getState(pMPSData->m_sync))
				{
				}
				thelog << "汇总进程 pid=" << pid << "创建完成" << endi;
				reset_private_data(0);
			}
			return true;
		}
	public:
		CMultiProcessServer(IDistributeTask<T_JOBINFO,T_TASKINFO > * d,IProcessTask<T_JOBINFO,T_TASKINFO > * p,IRollupTask<T_JOBINFO,T_TASKINFO > * r,char * shmbuf)
			:pDistribute(d),pProcess(p),pRollup(r),pMPSData((MPS_DATA *)shmbuf),isInPause(false)
		{
		}
		static long calcBufSise(){return sizeof(MPS_DATA);}//获得所需的内存大小
		T_SERVERINFO * getServerInfo(){return &pMPSData->m_server_info;}//返回用户服务数据存储空间
		string & report(string & ret,bool process=true,bool process_detail=true,bool taskinfo=false)const
		{
			return pMPSData->report(ret,process,process_detail,taskinfo);
		}
		string & toString(string & ret)const
		{
			return pMPSData->toString(ret);
		}
		long getTotalRollupCount()const{return pMPSData->count_all_rollup;}//返回总汇总量，通过这个可以计算吞吐量
		void SetCommondPause()
		{
			pMPSData->_in_auto_job = false;
			pMPSData->m_distribute_data.setCmd(pMPSData->m_sync,CMD_PAUSE);
			thelog<<"暂停命令已设置"<<endi;
		}
		void SetCommondExit()
		{
			pMPSData->_in_auto_job = false;
			pMPSData->m_distribute_data.setCmd(pMPSData->m_sync,CMD_EXIT);
			thelog<<"退出命令已设置"<<endi;
		}
		void SetCommondContiue()
		{
			pMPSData->_in_auto_job = true;
			pMPSData->m_distribute_data.setCmd(pMPSData->m_sync,CMD_NULL);
			thelog<<"继续命令已设置"<<endi;
		}
		void SetAutoHighLow(bool autoHighLow)
		{
			pMPSData->autoHighLow=autoHighLow;
			if(pMPSData->autoHighLow)thelog<<"已设置为自动高低水"<<endi;
			else thelog<<"已设置为平均分配任务"<<endi;
		}
		void SetExitIfNoJob(bool exitIfNoJob)
		{
			pMPSData->exitIfNoJob=exitIfNoJob;
			if(pMPSData->exitIfNoJob)thelog<<"已设置为批处理模式，无job退出"<<endi;
			else thelog<<"已设置为服务模式"<<endi;
		}
		void SetExitIfProcessCoredump(bool exitIfProcessCoredump)
		{
			//pMPSData->exitIfProcessCoredump=exitIfProcessCoredump;
			//if(pMPSData->exitIfProcessCoredump)thelog<<"已设置为处理进程coredump则全部退出"<<endi;
			//else thelog<<"已设置为处理进程coredump自动重起"<<endi;
			pMPSData->exitIfProcessCoredump=true;
			thelog<<"暂时不能coredump自动重起"<<endw;
		}
		void SetNotAutoSleep(bool notAutoSleep)
		{
			pMPSData->notAutoSleep=notAutoSleep;
			if(pMPSData->notAutoSleep)thelog<<"已设置为禁止自动睡眠"<<endi;
			else thelog<<"已设置为自动睡眠"<<endi;
		}
		void SetNotAutoSleep_distribute(bool notAutoSleep)
		{
			pMPSData->notAutoSleep_distribute=notAutoSleep;
			if(pMPSData->notAutoSleep_distribute)thelog<<"分发进程已设置为禁止自动睡眠"<<endi;
			else thelog<<"分发进程已设置为自动睡眠"<<endi;
		}
		void SetNotAutoSleep_rollup(bool notAutoSleep)
		{
			pMPSData->notAutoSleep_rollup=notAutoSleep;
			if(pMPSData->notAutoSleep_rollup)thelog<<"汇总进程已设置为禁止自动睡眠"<<endi;
			else thelog<<"汇总进程已设置为自动睡眠"<<endi;
		}
		void SetMaxProcess(long max_process)
		{
			if(max_process>0 && max_process<=MAX_PROCESS)pMPSData->max_process=max_process;
			else
			{
				if(pMPSData->max_process<1)
				{
					pMPSData->max_process=1;
				}
				if(pMPSData->max_process>MAX_PROCESS)
				{
					pMPSData->max_process=MAX_PROCESS;
				}
			}
		}
		void CheckSumEnable(bool chenksumEnable) { pMPSData->DEBUG_ENABLE_CHECKSUM = chenksumEnable; }
		void SyncEnable(bool syncEnable) { pMPSData->DEBUG_ENABLE_SYNC = syncEnable; }
		void clear()
		{
			thelog<<"初始化共享内存..."<<endi;
			memset((void*)pMPSData,0,sizeof(MPS_DATA));
			new(pMPSData)MPS_DATA;
			pMPSData->clear();
			SetNotAutoSleep_distribute(true);
			SetNotAutoSleep(false);
			SetNotAutoSleep_rollup(true);
			SetExitIfProcessCoredump(true);
			SetMaxProcess(MAX_PROCESS);//设置为最大值
			pMPSData->init_sync();
		}
		int run(long iGroup = 0)
		{
			pMPSData->iGroup = iGroup;

			char buf[2048];
			sprintf(buf, "%ld-分发", pMPSData->iGroup);
			theLog.SetSource(buf);
	
			if(!init_distribute_and_rollup())
			{
				return __LINE__;
			}
			thelog<<"进入分发处理..."<<endi;
			int ret=doDistribute();
			pMPSData->m_distribute_data.setState(pMPSData->m_sync,STATE_EXITED);
			pMPSData->m_sync.Destory();//必须删除信号量
			return ret;
		}
		int view()
		{
			thelog<<"多进程服务器控制，显示模式推荐使用2，只显示第一个进程的状态\n"
				"多进程服务器控制 ctrl-c to break"<<endi;
			int view_level=0;//查看模式
			string str;

			signal(SIGINT,sig_default);
			while(true)
			{
				if(check_signal(SIGINT))break;
				string cmd=UIInput("p=暂停 s=停止 c=继续 a=睡眠 na=不睡眠 checksum-enable checksum-disable sync-enable sync-disable"
					"\n d-a=分发睡眠 d-na=分发不睡眠 r-a=汇总睡眠 r-na=汇总不睡眠"
					"\n hl=高低水 nhl=不高低水 sp=最大进程数"
					"\n 显示模式设置：0=显示全部 1=不显示任务细节 2=不显示处理进程 3=显示用户任务数据"
					"\n other=根据模式显示","");
				if("p"==cmd)SetCommondPause();
				else if("s"==cmd)SetCommondExit();
				else if("c"==cmd)SetCommondContiue();
				else if("a"==cmd)SetNotAutoSleep(false);
				else if("na"==cmd)SetNotAutoSleep(true);
				else if ("checksum-enable" == cmd)CheckSumEnable(true);
				else if ("checksum-disable" == cmd)CheckSumEnable(false);
				else if ("sync-enable" == cmd)SyncEnable(true);
				else if ("sync-disable" == cmd)SyncEnable(false);
				else if("d-a"==cmd)SetNotAutoSleep_distribute(false);
				else if("d-na"==cmd)SetNotAutoSleep_distribute(true);
				else if("r-a"==cmd)SetNotAutoSleep_rollup(false);
				else if("r-na"==cmd)SetNotAutoSleep_rollup(true);
				else if("hl"==cmd)SetAutoHighLow(true);
				else if("nhl"==cmd)SetAutoHighLow(false);
				else if("sp"==cmd)SetMaxProcess(atol(UIInput("intput 1-MAX:","").c_str()));
				else if("0"==cmd)view_level=0;
				else if("1"==cmd)view_level=1;
				else if("2"==cmd)view_level=2;
				else if("3"==cmd)view_level=3;
				else
				{
					if(1==view_level)thelog<<"显示模式："<<view_level<<endl<<report(str,true,false)<<endi;
					else if(2==view_level)thelog<<"显示模式："<<view_level<<endl<<report(str,false,false)<<endi;
					else if(3==view_level)thelog<<"显示模式："<<view_level<<endl<<report(str,true,true,true)<<endi;
					else thelog<<"显示模式："<<view_level<<endl<<toString(str)<<endi;
				}
			}
			return 0;
		}
		static long timeval_percent(timeval const * current, timeval const * last, long span_usec)
		{
			return ((long)(current->tv_sec - last->tv_sec) * 1000000 + (current->tv_usec - last->tv_usec)) * 100 / span_usec;
		}
		static long timespan_usec(timeval const * current, timeval const * last)
		{
			return (long)(current->tv_sec - last->tv_sec) * 1000000 + (current->tv_usec - last->tv_usec);
		}
		static long clock_percent(clock_t current, clock_t last, long span_usec)
		{
			return ((long)(current - last) * 100 * 1000000 / CLOCKS_PER_SEC) / span_usec;
		}
		int speed(bool AutomaticTransmission)
		{
			thelog<<"多进程服务器速度监测 ctrl-c to break"<<endi;

			string str;

			if(AutomaticTransmission)
			{
				thelog<<"自动调速"<<endi;
			}
			long last=getTotalRollupCount();
			long last_distribute=pMPSData->count_all_task;
			long last_no_idle=pMPSData->count_no_idle;
			long last_rollup_route=pMPSData->count_rollup_route_all;
			long last_rollup_route_nothing=pMPSData->count_rollup_route_nothing;
			//long last_distribute_sleep=pMPSData->m_distribute_data.getSleepCount();
			//long last_rollup_sleep=pMPSData->m_rollup_data.getSleepCount();
			pMPSData->max_used_process=0;
			signal(SIGINT,sig_default);
			long oldspeed=0;
			long newspeed=0;
			bool isAddProcess=true;
			long process=pMPSData->max_process;
			long speedchange=0;
			long sill = 0;
			long timespan;//循环间隔，微秒
			STATIC_G long n=0;
			while(true)
			{
				if(check_signal(SIGINT))break;

				timeval tmp_timeval_process = pMPSData->m_process_datas[0].timeval_process;
				timeval tmp_timeval_distribute = pMPSData->timeval_distribute;
				timeval tmp_timeval_distribute_OpenClose = pMPSData->timeval_distribute_OpenClose;
				timeval tmp_timeval_rollup = pMPSData->timeval_rollup;
				timeval tmp_timeval_rollup_OpenClose = pMPSData->timeval_rollup_OpenClose;
				clock_t tmp_clock_process=pMPSData->m_process_datas[0].pid_data.cpu_clock;
				clock_t tmp_clock_distribute=pMPSData->m_distribute_data.cpu_clock;
				clock_t tmp_clock_rollup=pMPSData->m_rollup_data.cpu_clock;
				pMPSData->max_used_process=0;
				long max_process=pMPSData->max_process;//这个可能中途被外部手工改变
				timeval lasttime;
				gettimeofday(&lasttime, NULL);
				
				sleep(5);
				
				long current=getTotalRollupCount();
				long current_distribute=pMPSData->count_all_task;
				long current_no_idle=pMPSData->count_no_idle;
				long current_rollup_route=pMPSData->count_rollup_route_all;
				long current_rollup_route_nothing=pMPSData->count_rollup_route_nothing;
				//long current_distribute_sleep=pMPSData->m_distribute_data.getSleepCount();
				//long current_rollup_sleep=pMPSData->m_rollup_data.getSleepCount();
				timeval currenttime;
				gettimeofday(&currenttime, NULL);

				timespan = timespan_usec(&currenttime, &lasttime);

				if(process!=pMPSData->max_process)
				{
					if(0!=process)thelog<<"已经被手工修改，重新测速 "<<process<<" "<<pMPSData->max_process<<endi;
					else thelog<<"进程数不能更低，重新测速 "<<process<<" "<<pMPSData->max_process<<endi;
					process=pMPSData->max_process;
					oldspeed=0;
					newspeed=0;
					n=0;
					last=current;
					last_distribute=current_distribute;
					last_no_idle=current_no_idle;
					last_rollup_route=current_rollup_route;
					last_rollup_route_nothing=current_rollup_route_nothing;
					//last_distribute_sleep=current_distribute_sleep;
					//last_rollup_sleep=current_rollup_sleep;
					continue;
				}
				newspeed = (current - last) * 60 * 1000000 / timespan;//换算为秒
				if(0==oldspeed)speedchange=0;
				else speedchange=(newspeed-oldspeed)*100/oldspeed;
	
				if(current!=last)
				{
					cout << maketimestr(time(NULL))
						<< " S:" << !pMPSData->notAutoSleep_distribute << !pMPSData->notAutoSleep << !pMPSData->notAutoSleep_rollup
						<< " H:" << pMPSData->autoHighLow
						<< " MP:" << max_process
						<< " UP:" << pMPSData->max_used_process + 1
						<< " 分发失败：" << (current_no_idle - last_no_idle) * 100 / (0 == (current_distribute - last_distribute) ? 1 : (current_distribute - last_distribute)) << "%"
						<< " 有效率 " << timeval_percent(&pMPSData->timeval_distribute, &tmp_timeval_distribute, timespan) << "%"
						<< " OC " << timeval_percent(&pMPSData->timeval_distribute_OpenClose, &tmp_timeval_distribute_OpenClose, timespan) << "%"
						<< " C " << clock_percent(pMPSData->m_distribute_data.cpu_clock, tmp_clock_distribute, timespan) << "%"
						//<<" S/m:"<<setw(5)<<(current_distribute_sleep-last_distribute_sleep)*timespan/60
						<< " 汇总空闲：" << (current_rollup_route_nothing - last_rollup_route_nothing) * 100 / (0 == (current_rollup_route - last_rollup_route) ? 1 : (current_rollup_route - last_rollup_route)) << "%"
						<< " 有效率 " << timeval_percent(&pMPSData->timeval_rollup, &tmp_timeval_rollup, timespan) << "%"
						<< " OC " << timeval_percent(&pMPSData->timeval_rollup_OpenClose,&tmp_timeval_rollup_OpenClose,timespan)<<"%"
						<< " C " << clock_percent(pMPSData->m_rollup_data.cpu_clock, tmp_clock_rollup, timespan) << "%"
						//<<" S/m:"<<setw(5)<<(current_rollup_sleep-last_rollup_sleep)*timespan/60
						<< " 即时速度 tasks/minute : " << newspeed << " " << speedchange << "% sill=" << sill << "%"
						<< " P0 " << timeval_percent(&pMPSData->m_process_datas[0].timeval_process, &tmp_timeval_process, timespan) << "%"
						<< " C " << clock_percent(pMPSData->m_process_datas[0].pid_data.cpu_clock, tmp_clock_process, timespan) << "%"
						<<endl;
					last=current;
					last_distribute=current_distribute;
					last_no_idle=current_no_idle;
					last_rollup_route=current_rollup_route;
					last_rollup_route_nothing=current_rollup_route_nothing;
					//last_distribute_sleep=current_distribute_sleep;
					//last_rollup_sleep=current_rollup_sleep;
				}
				else
				{
					time_t t1=time(NULL);
					cout << "\r" << asctime(localtime(&t1));
					cout.flush();
				}
				//调速
				if(AutomaticTransmission)
				{
					sill=100/(process*3);
					if(sill>10)sill=10;
					if(0==(++n)%3)
					{
						if(0==oldspeed)
						{
							if(max_process!=MAX_PROCESS)
							{
								++process;
								SetMaxProcess(process);
								isAddProcess=true;
							}
							else
							{
								--process;
								SetMaxProcess(process);
								isAddProcess=false;
							}
							oldspeed = newspeed;
						}
						else if(speedchange>=sill)
						{
							if(isAddProcess)
							{
								++process;
								SetMaxProcess(process);
								isAddProcess=true;
							}
							else
							{
								--process;
								SetMaxProcess(process);
								isAddProcess=false;
							}
							oldspeed = newspeed;
						}
						else if(speedchange<=-sill)
						{
							if(!isAddProcess)
							{
								++process;
								SetMaxProcess(process);
								isAddProcess=true;
							}
							else
							{
								--process;
								SetMaxProcess(process);
								isAddProcess=false;
							}
							oldspeed = newspeed;
						}
						else
						{
							--process;
							SetMaxProcess(process);
							isAddProcess=false;
							oldspeed = newspeed;
						}
					}
				}
			}
			return 0;
		}
	};

}
