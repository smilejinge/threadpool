#ifndef __THREAD_MANAGER_H__
#define __THREAD_MANAGER_H__


#include <stdint.h>

#include "config.h"
#include "thread_pool_static.h"


class CThreadJob;


class CThreadManager
{
public:
	CThreadManager(Config &config);
	virtual ~CThreadManager();

public:
	bool Initialize();
	CThreadPoolStatic *GetThreadPoolStatic();
	void Run(CThreadJob *job);
	void InterruptAllThread();

private:
	Config &m_config;
	CThreadPoolStatic *m_thread_pool_static;
};


#endif
