//CCTModel_UniversalDB_t.cpp 通用数据库对象模型
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "../CodeTemplate/CodeTemplate.h"
#include "CCTModel_UniversalDB.h"

bool Rooster_AddAllTable(CCTModel_UniversalDB* pCTM)
{
	CCTModel_UniversalDB::table* p;

	//资源类型，预定义的数值
	p = pCTM->AddNewTable("RESOURCE_TYPE_D", "资源类型");
	if (NULL == p)return false;
	p->AddMember("RESOURCE_TYPE_ID", "long", "资源类型");
	p->AddMember("COMMENT", "string", "备注");
	p->SetPK("RESOURCE_TYPE_ID");
	p->AddDML("InsertNewResourceType", "insert", "RESOURCE_TYPE_ID,COMMENT", "", "插入新资源类型");

	//资源使用类型，预定义的数值
	p = pCTM->AddNewTable("RESOURCE_USE_TYPE_D", "资源使用类型");
	if (NULL == p)return false;
	p->AddMember("RESOURCE_USE_TYPE_ID", "long", "资源使用类型ID");
	p->AddMember("COMMENT", "string", "备注");
	p->SetPK("RESOURCE_USE_TYPE_ID");
	p->AddDML("InsertNewResourceUseType", "insert", "RESOURCE_USE_TYPE_ID,COMMENT", "", "插入新资源使用类型");

	//资源
	p = pCTM->AddNewTable("RESOURCE_D", "资源");
	if (NULL == p)return false;
	p->AddMember("RESOURCE_ID", "long", "资源ID");
	p->AddMember("RESOURCE_NAME", "string", "资源名称");
	p->AddMember("RESOURCE_TYPE_ID", "long", "资源类型");
	p->AddMember("RESOURCE_WITH_SYS", "long", "资源是否区分sys/sys2");
	p->AddMember("COMMENT", "string", "备注");
	p->SetPK("RESOURCE_ID");
	p->AddDML("InsertNewResource", "insert", "*", "", "插入新资源");
	p->AddDML("GetAllResource", "select", "*", "", "获得全部");

	//任务定义
	p = pCTM->AddNewTable("TASK_D", "任务");
	if (NULL == p)return false;
	p->AddMember("TASK_ID", "long", "任务ID");
	p->AddMember("TASK_NAME", "string", "任务名称");
	p->AddMember("TASK_MULTIPLE", "long", "0非多开1多开");
	p->AddMember("TASK_WITH_SYS", "long", "是否区分sys/sys2");
	p->AddMember("TASK_PRIORITY", "long", "优先级，小的优先");
	p->AddMember("TASK_PLAN_TIME", "string", "执行时间，‘2,8-16’，时间点或时间段");
	p->AddMember("TASK_PLAN_INTERVAL", "long", "执行时间段的执行间隔（分钟0-59）");
	p->AddMember("TASK_START_CMD", "string", "启动命令");
	p->AddMember("COMMENT", "string", "备注");
	p->SetPK("TASK_ID");
	p->AddDML("InsertNewTaskDefine", "insert", "*", "", "插入新任务定义");
	p->AddDML("GetAllTask", "select", "*", "", "获得全部");

	//任务资源使用定义
	p = pCTM->AddNewTable("TASK_RESOURCE_USE_D", "任务资源使用");
	if (NULL == p)return false;
	p->AddMember("TASK_ID", "long", "任务ID");
	p->AddMember("RESOURCE_ID", "long", "资源ID");
	p->AddMember("RESOURCE_USE_TYPE_ID", "long", "资源使用类型ID");
	p->AddMember("COMMENT", "string", "备注");
	p->SetPK("TASK_ID,RESOURCE_ID");
	p->AddDML("InsertNewTaskResurceUseDefine", "insert", "TASK_ID,RESOURCE_ID,RESOURCE_USE_TYPE_ID,COMMENT", "", "插入新任务资源使用定义");
	p->AddDML("GetAllTaskResourceUse", "select", "*", "", "获得全部");

	//任务队列表
	p = pCTM->AddNewTable("TASK_QUEUE", "待执行的任务队列");
	if (NULL == p)return false;
	p->AddMember("TASK_ID", "long", "任务ID");
	p->AddMember("SYS", "long", "维度1");
	p->AddMember("SYS2", "long", "维度2");
	p->AddMember("TASK_NAME", "string", "任务名称（参考）");
	p->AddMember("TASK_PRIORITY", "long", "优先级，小的优先（参考）");
	p->AddMember("COMMENT", "string", "备注（参考）");
	p->AddMember("INSERT_TIME", "time", "开始时间", "", "TIME_LOG");
	p->SetPK("TASK_ID,SYS,SYS2");
	p->AddDML("InsertNewTaskQueue", "insert", "TASK_ID,SYS,SYS2,TASK_NAME,TASK_PRIORITY,COMMENT,INSERT_TIME", "", "插入新任务队列");
	p->AddDML("DeleteTaskQueue", "delete", "", "TASK_ID,SYS,SYS2", "删除任务队列");
	p->AddDML("GetAllTaskQueue", "select", "*", "", "获取全部任务队列");
	p->AddDML("GetTaskQueue", "select", "*", "TASK_ID,SYS,SYS2", "获取任务队列");

	//任务执行表
	p = pCTM->AddNewTable("TASK_LOG", "任务日志");
	if (NULL == p)return false;
	p->AddMember("TASK_LOG_ID", "long", "任务执行序列号SEQ_TASK_LOG");
	p->AddMember("TASK_ID", "long", "任务ID");
	p->AddMember("SYS", "long", "维度1");
	p->AddMember("SYS2", "long", "维度2");
	p->AddMember("PID", "long", "任务所在的进程，客户方写入");
	p->AddMember("START_TIME", "time", "开始时间", "", "TIME_LOG");
	p->AddMember("FINISH_TIME", "time", "结束时间", "", "TIME_LOG");
	p->AddMember("RETURN_VALUE", "long", "返回值 0成功 <0Rooster错误 >0任务错误");
	p->AddMember("UPDATE_TIME", "time", "记录更新时间", "", "TIME_LOG");
	p->SetPK("TASK_LOG_ID");
	p->AddDML("InsertNewTaskLog", "insert", "TASK_LOG_ID,TASK_ID,SYS,SYS2,START_TIME,UPDATE_TIME", "", "插入新任务日志");
	p->AddDML("UpdateTaskLog", "update", "PID,UPDATE_TIME", "TASK_LOG_ID", "更新任务日志", "finish_time=0");
	p->AddDML("FinishTaskLog", "update", "FINISH_TIME,RETURN_VALUE,UPDATE_TIME", "TASK_LOG_ID", "更新任务日志", "finish_time=0");
	p->AddDML("GetCurrentTask", "select", "*", "", "获取正在运行的任务", "finish_time=0");

	//序列
	if (NULL == pCTM->AddNewSequence("SEQ_TASK_LOG", "TASK日志序列", 0))return false;

	return true;
}

int main(int argc, char** argv)
{
	if (!InitActiveApp("CCTModel_UniversalDB", 1024 * 1024, argc, argv))exit(1);

	if (sizeof(long) != 8)
	{
		thelog << "非64位程序！" << ende;
	}

	G_IS_DEBUG = true;

	CCTModel_UniversalDB ctm;
	if (!Rooster_AddAllTable(&ctm))
	{
		thelog << "执行失败" << ende;
		return __LINE__;
	}
	if (!ctm.CreateCode("ns_rooster_struct", "RoosterStruct", "."))
	{
		thelog << "执行失败" << ende;
		return __LINE__;
	}

	thelog << "程序退出" << endi;
	return 0;
}
