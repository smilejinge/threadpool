#include "worker_thread.h"

#include "log.h"
#include "thread_debug.h"
#include "thread_pool_static.h"


CWorkerThread::CWorkerThread(Config &config)
	: m_config(config)
	, m_job_mutex()
	, m_job_condition(&m_job_mutex)
{
	m_thread_pool_static = NULL;
	m_thread_job = NULL;
}

CWorkerThread::CWorkerThread(Config &config, const std::string &threadname, bool detach /* = false */)
	: CThreadBase(threadname, detach)
	, m_config(config)
	, m_job_mutex()
	, m_job_condition(&m_job_mutex)
{
	m_thread_pool_static = NULL;
	m_thread_job = NULL;
}

CWorkerThread::~CWorkerThread()
{
	m_thread_pool_static = NULL;
	
	if(NULL != m_thread_job)
	{
		delete m_thread_job;
		m_thread_job = NULL;
	}
}

bool CWorkerThread::Initialize()
{
	return true;
}

void CWorkerThread::Prepare()
{
	
}

void *CWorkerThread::Run()
{
	m_job_mutex.Lock();
	
	while(!IsNeedStop() || (NULL != m_thread_job))
	{
		if(NULL == m_thread_job)
		{
			m_job_condition.Wait();
			continue;
		}

		m_thread_job->Run();
		m_thread_job->SetWorkThread(NULL);
		delete m_thread_job;
		m_thread_job = NULL;
		m_thread_pool_static->GiveBackIdleThread(this);
	}

	SetExitMsg("CWorkerThread Stop.");
	m_job_mutex.UnLock();
	
	return static_cast<void *>(const_cast<char *>(GetExitMsg().c_str()));
}

void CWorkerThread::Destroy()
{
	
}

bool CWorkerThread::Interrupt(void **msg)
{
	if(NULL == this || m_stop || !GetThreadID())
	{
		return true;
	}
	SetStopFlag(1);
	m_job_condition.Signal();
	bool ret = Join(msg);
	do
	{
		if(false == ret)
		{
			log_error("join failed, when interrupt.");
			break;
		}

		log_error("thread [%s] normal stopped.", GetThreadName().c_str());
	} while(0);

	return ret;
}

void CWorkerThread::SetJob(CThreadJob *job)
{
	m_job_mutex.Lock();
	job->SetWorkThread(this);
	m_thread_job = job;
	m_job_condition.Signal();
	m_job_mutex.UnLock();
}

CThreadJob *CWorkerThread::GetJob()
{
	return m_thread_job;
}

void CWorkerThread::SetThreadPoolStatic(CThreadPoolStatic *threadpoolstatic)
{
	m_job_mutex.Lock();
	m_thread_pool_static = threadpoolstatic;
	m_job_mutex.UnLock();
}

CThreadPoolStatic *CWorkerThread::GetThreadPoolStatic()
{
	return m_thread_pool_static;
}
