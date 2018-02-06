#ifndef __TIMER_THREAD_H__
#define __TIMER_THREAD_H__


#include "thread_base.h"
#include "thread_job.h"
#include "thread_mutex.h"


class CTimerThread : public CThreadBase
{
public:
	CTimerThread();
	CTimerThread(const std::string &threadname, bool detach = false);
	virtual ~CTimerThread();

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
	void RunningThread();

private:
	CThreadJob *m_thread_job;
	CThreadMutex m_thread_mutex;
};


#endif
