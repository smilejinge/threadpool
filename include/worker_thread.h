#ifndef __WORKER_THREAD_H__
#define __WORKER_THREAD_H__


#include <stdint.h>

#include "config.h"
#include "thread_base.h"
#include "thread_condition.h"
#include "thread_job.h"
#include "thread_mutex.h"


class CThreadPoolStatic;


class CWorkerThread : public CThreadBase
{
public:
	CWorkerThread(Config &config);
	CWorkerThread(Config &config, const std::string &threadname, bool detach = false);
	virtual ~CWorkerThread();

public:
	/* 线程创建前，设置一些线程属性 */
	virtual bool Initialize();
	/* 线程创建后，设置一些运行状态 */
	virtual void Prepare();
	/* 线程运行 */
	virtual void *Run();
	/* 线程退出前，做一些清理操作 */
	virtual void Destroy();
	/* 中断退出线程 */
	virtual bool Interrupt(void **msg);

public:
	void SetJob(CThreadJob *job);
	CThreadJob *GetJob();
	void SetThreadPoolStatic(CThreadPoolStatic *threadpoolstatic);
	CThreadPoolStatic *GetThreadPoolStatic();

private:
	Config &m_config;
	CThreadMutex m_job_mutex;
	CThreadCondition m_job_condition;
	CThreadPoolStatic *m_thread_pool_static;
	CThreadJob *m_thread_job;
};


#endif
