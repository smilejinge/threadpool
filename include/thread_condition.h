#ifndef __THREAD_CONDITION_H__
#define __THREAD_CONDITION_H__


#include <pthread.h>

#include "noncopyable.h"


class CThreadMutex;


class CThreadCondition : public NonCopyable
{
public:
	CThreadCondition();
	CThreadCondition(const pthread_condattr_t *condattr);
	CThreadCondition(const pthread_condattr_t *condattr, CThreadMutex *thread_mutex);
	CThreadCondition(CThreadMutex *thread_mutex);
	~CThreadCondition();

public:
	void SetThreadMutex(CThreadMutex *thread_mutex);
	int Wait();
	int TimeWait(const struct timespec *timeout);
	int Signal();
	int Broadcast();
	
private:
	pthread_cond_t m_condition;
	CThreadMutex *m_thread_mutex;
};


#endif
