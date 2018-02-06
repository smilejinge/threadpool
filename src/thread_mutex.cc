#include "thread_mutex.h"


CThreadMutex::CThreadMutex()
{
	pthread_mutex_init(&m_mutex, NULL);
}

CThreadMutex::CThreadMutex(const pthread_mutexattr_t *mutexattr)
{
	pthread_mutex_init(&m_mutex, mutexattr);
}

CThreadMutex::~CThreadMutex()
{
	pthread_mutex_destroy(&m_mutex);
}

bool CThreadMutex::Lock()
{
	pthread_mutex_lock(&m_mutex);
	return true;
}

bool CThreadMutex::TryLock()
{
	pthread_mutex_trylock(&m_mutex);
	return true;
}

bool CThreadMutex::UnLock()
{
	pthread_mutex_unlock(&m_mutex);
	return true;
}
