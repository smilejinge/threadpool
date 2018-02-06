#ifndef __THREAD_POOL_STATIC_H__
#define __THREAD_POOL_STATIC_H__


#include <stdint.h>

#include <vector>

#include "config.h"
#include "thread_condition.h"
#include "thread_job.h"
#include "thread_mutex.h"


class CWorkerThread;


class CThreadPoolStatic
{
public:
	CThreadPoolStatic();
	CThreadPoolStatic(uint32_t initnum);
	virtual ~CThreadPoolStatic();

public:
	void SetInitNum(uint32_t initnum)
	{
		m_initnum = initnum;
	}
	uint32_t GetInitNum()
	{
		return m_initnum;
	}

public:
	bool Init(Config &config);
	uint32_t GetBusyListNum();
	uint32_t GetIdleListNum();
	void GiveBackIdleThread(CWorkerThread *workthread);
	bool InterruptThread(CWorkerThread *workthread, void **msg);
	void InterruptAllThread();
	void Run(CThreadJob *threadjob);

protected:
	CWorkerThread *GetIdleThread();
	void AppendToIdleList(CWorkerThread *workthread);
	
private:
	/* 初始创建时线程池中的线程的个数 */
	uint32_t m_initnum;
	/* 忙碌线程列表 */
	std::vector<CWorkerThread *> m_busylist;
	/* 空闲线程列表 */
	std::vector<CWorkerThread *> m_idlelist;
	
	CThreadMutex m_busymutex;
	CThreadMutex m_idlemutex;
	CThreadMutex m_jobmutex;
	CThreadCondition m_busycond;
	CThreadCondition m_idlecond;
	CThreadCondition m_jobcond;
};


#endif
