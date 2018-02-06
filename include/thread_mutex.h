#ifndef __THREAD_MUTEX_H__
#define __THREAD_MUTEX_H__


#include <pthread.h>

#include "lock_object.h"


class CThreadCondition;


class CThreadMutex : public CLockObject
{
	friend class CThreadCondition;
public:
	CThreadMutex();
	CThreadMutex(const pthread_mutexattr_t *mutexattr);
	virtual ~CThreadMutex();
	
public:
	virtual bool Lock();
	virtual bool TryLock();
	virtual bool UnLock();

private:
	pthread_mutex_t m_mutex;
};


#endif
