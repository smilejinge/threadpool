#include "timer_thread.h"

#include "log.h"


CTimerThread::CTimerThread()
{
	m_thread_job = NULL;
}

CTimerThread::CTimerThread(const std::string &threadname, bool detach /* = false */) : CThreadBase(threadname, detach)
{
	m_thread_job = NULL;
}

CTimerThread::~CTimerThread()
{
	//此处的任务由任务源delete
	m_thread_job = NULL;
}

bool CTimerThread::Initialize()
{
	m_thread_mutex.Lock();
	return true;
}

void CTimerThread::Prepare()
{
	
}

void *CTimerThread::Run()
{
	do
	{
		m_thread_mutex.Lock();
		/*
		if(NULL == m_thread_job)
		{
			log_error("timer thread job is null.");
			SetExitMsg("thread job is null.");
			break;
		}
		*/
		while(!IsNeedStop())
		{
			m_thread_job->Run();
		}

		m_thread_mutex.UnLock();
		SetExitMsg("CTimerThread stop.");
		log_error("thread [%s] normal stopped.", GetThreadName().c_str());
	} while(0);

	return static_cast<void *>(const_cast<char *>(GetExitMsg().c_str()));
}

void CTimerThread::Destroy()
{
	
}

bool CTimerThread::Interrupt(void **msg)
{
	if(NULL == this || m_stop || !GetThreadID())
	{
		return true;
	}

	SetStopFlag(1);
	m_thread_job->Destroy();
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

void CTimerThread::SetJob(CThreadJob *job)
{
	m_thread_job = job;
}

CThreadJob *CTimerThread::GetJob()
{
	return m_thread_job;
}

void CTimerThread::RunningThread()
{
	m_thread_mutex.UnLock();
}
