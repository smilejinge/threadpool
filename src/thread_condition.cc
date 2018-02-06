#include "thread_condition.h"

#include "thread_mutex.h"


CThreadCondition::CThreadCondition()
{
	m_thread_mutex = NULL;
	pthread_cond_init(&m_condition, NULL);
}

CThreadCondition::CThreadCondition(const pthread_condattr_t *condattr)
{
	m_thread_mutex = NULL;
	pthread_cond_init(&m_condition, condattr);
}

CThreadCondition::CThreadCondition(CThreadMutex *thread_mutex)
{
	m_thread_mutex = thread_mutex;
	pthread_cond_init(&m_condition, NULL);
}

CThreadCondition::CThreadCondition(const pthread_condattr_t *condattr, CThreadMutex *thread_mutex)
{
	m_thread_mutex = thread_mutex;
	pthread_cond_init(&m_condition, condattr);
}

CThreadCondition::~CThreadCondition()
{
	pthread_cond_destroy(&m_condition);
}

void CThreadCondition::SetThreadMutex(CThreadMutex *thread_mutex)
{
	m_thread_mutex = thread_mutex;
}

int CThreadCondition::Wait()
{
	return pthread_cond_wait(&m_condition, &(m_thread_mutex->m_mutex));
}

int CThreadCondition::TimeWait(const struct timespec *timeout)
{
	return pthread_cond_timedwait(&m_condition, &(m_thread_mutex->m_mutex), timeout);
}

int CThreadCondition::Signal()
{
	return pthread_cond_signal(&m_condition);
}

int CThreadCondition::Broadcast()
{
	return pthread_cond_broadcast(&m_condition);
}
