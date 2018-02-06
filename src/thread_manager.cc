#include "thread_manager.h"

#include "thread_job.h"


CThreadManager::CThreadManager(Config &config) : m_config(config)
{
	
}

CThreadManager::~CThreadManager()
{
	if(NULL != m_thread_pool_static)
	{
		delete m_thread_pool_static;
		m_thread_pool_static = NULL;
	}
}

bool CThreadManager::Initialize()
{
	m_thread_pool_static = new CThreadPoolStatic();
	if(NULL == m_thread_pool_static)
	{
		return false;
	}

	if(false == m_thread_pool_static->Init(m_config))
	{
		return false;
	}

	return true;
}

CThreadPoolStatic *CThreadManager::GetThreadPoolStatic()
{
	return m_thread_pool_static;
}

void CThreadManager::Run(CThreadJob *job)
{
	m_thread_pool_static->Run(job);
}

void CThreadManager::InterruptAllThread()
{
	m_thread_pool_static->InterruptAllThread();
}
