#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__


#include <stdint.h>
#include <pthread.h>

#include <string>

#include "thread_constant.h"


class CThreadBase
{
public:
	CThreadBase();
	CThreadBase(const std::string &threadname);
	CThreadBase(bool detach);
	CThreadBase(const std::string &threadname, bool detach);
	virtual ~CThreadBase();

public:
	/* 线程创建前初始化函数 */
	virtual bool Initialize();
	/* 线程创建后，运行前的准备函数 */
	virtual void Prepare();
	/* 线程执行函数 */
	virtual void *Run();
	/* 线程清理函数 */
	virtual void Destroy();
	/* 中断线程函数 */
	virtual bool Interrupt(void **msg);

public:
	/* 设置线程并行数量 */
	static int SetThreadConcurrency(const int concurrency);
	/* 获取线程并行数量 */
	static int GetThreadConcurrency();

public:
	/* 线程错误码设置 */
	void SetErrorCode(int64_t errorcode);
	/* 获取线程最后的错误码 */
	int64_t GetLastError();
	/* 设置线程名称 */
	bool SetThreadName(const std::string name);
	/* 获取线程名称 */
	std::string GetThreadName();
	/* 设置退出信息 */
	bool SetExitMsg(const std::string msg);
	/* 获取退出信息 */
	std::string GetExitMsg();
	/* 设置线程状态 */
	bool SetThreadState(ThreadState state);
	/* 获取线程状态 */
	ThreadState GetThreadState();
	/* 获取tid */
	pid_t GetTID();
	/* 获取线程ID */
	pthread_t GetThreadID();
	/* 设置detach标志 */
	void SetDetachFlag(bool detach);
	/* 设置线程interrupt标志 */
	void SetStopFlag(int stop);
	/* 获取线程停止运行标志 */
	int GetStopFlag();
	/* 判断线程是否需要停止运行 */
	bool IsNeedStop();
	
	/* 设置线程继承属性，即attr继承父线程，或者使用自定义的attr*/
	bool SetThreadAttrInheritSched(const int inheritsched);
	/* 获取线程继承属性，即attr继承父线程，或者使用自定义的attr*/
	bool GetThreadAttrInheritSched(int &inheritsched);
	/* 设置线程分离状态属性 */
	bool SetThreadAttrDetachState(const int detachstate);
	/* 获取线程分离状态属性 */
	bool GetThreadAttrDetachState(int &detachstate);
	/* 设置线程调度策略属性 SCHED_FIFO、SCHED_RR、SCHED_OTHER(此调度策略不支持优先级) */
	bool SetThreadAttrPolicy(const int policy);
	/* 获取线程调度策略属性 */
	bool GetThreadAttrPolicy(int &policy);
	/* 设置线程优先级属性 */
	bool SetThreadAttrPriority(const int priority);
	/* 获取线程优先级属性 */
	bool GetThreadAttrPriority(int &priority);
	/* 获取线程优先级最大值属性 */
	bool GetThreadAttrPriorityMax(int &priority);
	bool GetThreadAttrPriorityMax(const int policy, int &priority);
	/* 获取线程优先级最小值属性 */
	bool GetThreadAttrPriorityMin(int &priority);
	bool GetThreadAttrPriorityMin(const int policy, int &priority);
	/* 绑定CPU */
	bool SetThreadBindCpu(const int64_t cpu);
	bool SetThreadBindCpu(const cpu_set_t &mask);
	/* 获取线程绑定的CPU */
	bool GetThreadBindCpu(cpu_set_t &mask);
	/* 线程创建后，运行前设置是否可以cancel线程 */
	bool SetThreadCancelState(const int state, int *oldstate);
	/* 线程取消功能处于启用状态，设置线程取消类型 */
	bool SetThreadCancelType(const int type, int *oldtype);
	/* 建立线程取消点 */
	void SetThreadCancelPoint();
	/* 线程创建后，动态设置线程优先级 */
	bool SetThreadRunPriority(const int priority);
	/* 线程创建后，动态设置线程调度策略、优先级 */
	bool SetThreadRunPolicyAndPriority(const int policy, const int priority);
	/* 线程创建后，获取线程调度策略、优先级 */
	bool GetThreadRunPolicyAndPriority(int &policy, int &priority);
	
	/* 分离线程 */
	bool Detach();
	/* 等待线程退出 */
	bool Join(void **msg);
	/* 线程让出CPU */
	bool Yield();
	/* 获取POSIX线程ID */
	pthread_t Self();
	/* 开始线程函数 */
	bool Start();
	/* 唤醒线程函数 */
	bool Wakeup();
	/* 终止线程函数 */
	bool Terminate();
	/* 退出线程函数 */
    void Exit(void *msg = NULL);
protected:
	/* 线程入口函数 */
	static void *ThreadFunction(void *thread);

private:
	void CleanData(const std::string threadname = "", bool detach = false);
	bool InitializeBase();
	void PrepareBase();
	void DestroyBase();
	bool IsCanSetPriority(const int policy);

protected:
	/* 线程是否interrupt标志 */
	volatile int m_stop;
	
private:
	/* 错误码 */
	int64_t m_errorcode;
	/* 线程名称 */
	std::string m_threadname;
	/* 线程结束后返回信息 */
	std::string m_exitmsg;
	/* 线程状态，空闲、忙碌、挂起(Linux线程库不支持挂起操作)、终止 */
	ThreadState m_threadstate;
	/* 线程属性 */
	pthread_attr_t m_attr;
	/* 内核真实线程ID */
	pid_t m_tid;
	/* POSIX定义的线程ID */
	pthread_t m_threadid;
	/* 创建时，是否直接设置为分离线程 */
	bool m_isdetach;
};


#endif
