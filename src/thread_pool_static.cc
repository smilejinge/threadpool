#include "thread_pool_static.h"

#include <algorithm>
#include <sstream>

#include "log.h"
#include "thread_debug.h"
#include "worker_thread.h"


CThreadPoolStatic::CThreadPoolStatic()
	: m_busymutex()
	, m_idlemutex()
	, m_jobmutex()
	, m_busycond(&m_busymutex)
	, m_idlecond(&m_idlemutex)
	, m_jobcond(&m_jobmutex)
{
	m_initnum = 10;
	m_busylist.clear();
	m_idlelist.clear();
}

CThreadPoolStatic::CThreadPoolStatic(uint32_t initnum)
	: m_busymutex()
	, m_idlemutex()
	, m_jobmutex()
	, m_busycond(&m_busymutex)
	, m_idlecond(&m_idlemutex)
	, m_jobcond(&m_jobmutex)
{
	m_initnum = initnum;
	m_busylist.clear();
	m_idlelist.clear();
}

CThreadPoolStatic::~CThreadPoolStatic()
{
	InterruptAllThread();
}

bool CThreadPoolStatic::Init(Config &config)
{
	if(config.isMember("init_thread_num") && config["init_thread_num"].isUInt())
	{
		m_initnum = config["init_thread_num"].asUInt();
	}
	
	for(uint32_t idle_num = 0; idle_num < m_initnum; ++ idle_num)
	{
		std::ostringstream oss;
		oss << "worker_thread_";
		oss << (idle_num + 1);
		CWorkerThread *workthread = new CWorkerThread(config, oss.str());
		if(NULL == workthread)
		{
			log_error("no enough memory for new CWorkerThread.");
			return false;
		}
		if(false == workthread->Start())
		{
			log_error("start thread failed.");
			delete workthread;
			workthread = NULL;
			return false;
		}

		AppendToIdleList(workthread);
		workthread->SetThreadPoolStatic(this);
	}

	log_debug("create thread success.");
	return true;
}

uint32_t CThreadPoolStatic::GetBusyListNum()
{
	return m_busylist.size();
}

uint32_t CThreadPoolStatic::GetIdleListNum()
{
	return m_idlelist.size();
}

void CThreadPoolStatic::GiveBackIdleThread(CWorkerThread *workthread)
{
	m_idlemutex.Lock();
	m_idlelist.push_back(workthread);
	workthread->SetThreadState(THREAD_IDLE);
	m_idlecond.Signal();
	m_idlemutex.UnLock();

	m_busymutex.Lock();
	std::vector<CWorkerThread *>::iterator pos;
	pos = find(m_busylist.begin(), m_busylist.end(), workthread);
	if(m_busylist.end() != pos)
	{
		m_busylist.erase(pos);
	}
	m_busycond.Signal();
	m_busymutex.UnLock();
}

bool CThreadPoolStatic::InterruptThread(CWorkerThread *workthread, void **msg)
{
	if(NULL != workthread)
	{
		return workthread->Interrupt(msg);
	}

	return true;
}

void CThreadPoolStatic::InterruptAllThread()
{
	m_busymutex.Lock();
	while(GetBusyListNum() > 0)
	{
		log_debug("busy list num [%u].", GetBusyListNum());
		m_busycond.Wait();
		continue;
	}
	m_busymutex.UnLock();

	CWorkerThread *workthread = NULL;
	while(GetIdleListNum() > 0)
	{
		m_idlemutex.Lock();
		if(GetIdleListNum() > 0)
		{
			workthread = m_idlelist.front();
		}
		else
		{
			m_idlemutex.UnLock();
			continue;
		}
		InterruptThread(workthread, NULL);
		std::vector<CWorkerThread *>::iterator pos;
		pos = find(m_idlelist.begin(), m_idlelist.end(), workthread);
		if(m_idlelist.end() != pos)
		{
			m_idlelist.erase(pos);
		}
		delete workthread;
		workthread = NULL;
		m_idlemutex.UnLock();
	}
}

void CThreadPoolStatic::Run(CThreadJob *threadjob)
{
	if(NULL == threadjob)
	{
		return;
	}
	CWorkerThread *workthread = GetIdleThread();
	workthread->SetJob(threadjob);
}

CWorkerThread *CThreadPoolStatic::GetIdleThread()
{
	m_idlemutex.Lock();
	while(0 == GetIdleListNum())
	{
		m_idlecond.Wait();
		continue;
	}

	CWorkerThread *workthread = m_idlelist.front();
	std::vector<CWorkerThread *>::iterator pos;
	pos = find(m_idlelist.begin(), m_idlelist.end(), workthread);
	if(m_idlelist.end() != pos)
	{
		m_idlelist.erase(pos);
	}

	m_idlemutex.UnLock();

	m_busymutex.Lock();
	m_busylist.push_back(workthread);
	workthread->SetThreadState(THREAD_RUNING);
	m_busymutex.UnLock();
	
	return workthread;
}

void CThreadPoolStatic::AppendToIdleList(CWorkerThread *workthread)
{
	m_idlemutex.Lock();
	m_idlelist.push_back(workthread);
	m_idlemutex.UnLock();
}
