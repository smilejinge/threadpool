#include <signal.h>

#include "log.h"
#include "thread_base.h"
#include "thread_debug.h"
#include "thread_error.h"


CThreadBase::CThreadBase()
{
	CleanData();
}

CThreadBase::CThreadBase(const std::string &threadname)
{
	CleanData(threadname);
}

CThreadBase::CThreadBase(bool detach)
{
	CleanData("", detach);
}

CThreadBase::CThreadBase(const std::string &threadname, bool detach)
{
	CleanData(threadname, detach);
	log_debug("threadname [%s], detach flag [%d].", threadname.c_str(), detach);
}

CThreadBase::~CThreadBase()
{
	CleanData();

	int ret = pthread_attr_destroy(&m_attr);
	if(0 != ret)
	{
		log_error("thread attr destroy failed, errcode [%d].", ret);
		SetErrorCode(ret);
	}
}

bool CThreadBase::Initialize()
{
	return true;
}

void CThreadBase::Prepare()
{

}

void *CThreadBase::Run()
{
	while(!IsNeedStop())
	{
		pause();
	}

	return NULL;
}

void CThreadBase::Destroy()
{
	
}

bool CThreadBase::Interrupt(void **msg)
{
	if(NULL == this || m_stop || !GetThreadID())
	{
		return true;
	}
	pthread_t threadid = m_threadid;
	pid_t tid = m_tid;
	SetStopFlag(1);
	pthread_kill(m_threadid, SIGINT);
	bool res = Join(msg);
	if(false == res)
	{
		log_error("join failed, when interrupt.");
		return false;
	}

	log_debug("thread [%s] stopped, threadid [%llu], tid [%d].", m_threadname.c_str(), static_cast<unsigned long long int>(threadid), tid);
	return true;
}

int CThreadBase::SetThreadConcurrency(const int concurrency)
{
	return pthread_setconcurrency(concurrency);
}

/**
 *Get之前若未调用过Set，则返回0
 **/
int CThreadBase::GetThreadConcurrency()
{
	return pthread_getconcurrency();
}

void CThreadBase::SetErrorCode(int64_t errorcode)
{
	m_errorcode = errorcode;
}

int64_t CThreadBase::GetLastError()
{
	return m_errorcode;
}

bool CThreadBase::SetThreadName(const std::string name)
{
	m_threadname = name;
	return true;
}

std::string CThreadBase::GetThreadName()
{
	return m_threadname;
}

bool CThreadBase::SetExitMsg(const std::string msg)
{
	m_exitmsg = msg;
	return true;
}

std::string CThreadBase::GetExitMsg()
{
	return m_exitmsg;
}

bool CThreadBase::SetThreadState(ThreadState state)
{
	m_threadstate = state;
	return true;
}

ThreadState CThreadBase::GetThreadState()
{
	return m_threadstate;
}

pid_t CThreadBase::GetTID()
{
	return (THREAD_EXIT == GetThreadState()) ? 0 : m_tid;
}

pthread_t CThreadBase::GetThreadID()
{
	return (THREAD_EXIT == GetThreadState()) ? 0 : m_threadid;
}

void CThreadBase::SetDetachFlag(bool detach)
{
	m_isdetach = detach;
}

void CThreadBase::SetStopFlag(int stop)
{
	m_stop = stop;
}

int CThreadBase::GetStopFlag()
{
	return m_stop;
}

bool CThreadBase::IsNeedStop()
{
	return (0 != m_stop);
}

bool CThreadBase::SetThreadAttrInheritSched(const int inheritsched)
{
	int ret = pthread_attr_setinheritsched(&m_attr, inheritsched);
	if(0 != ret)
	{
		log_error("set thread attr inherit sched failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::GetThreadAttrInheritSched(int &inheritsched)
{
	int ret = pthread_attr_getinheritsched(&m_attr, &inheritsched);
	if(0 != ret)
	{
		log_error("get thread attr inherit sched failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::SetThreadAttrDetachState(const int detachstate)
{
	int ret = pthread_attr_setdetachstate(&m_attr, detachstate);
	if(0 != ret)
	{
		log_error("set thread attr detach state failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::GetThreadAttrDetachState(int &detachstate)
{
	int ret = pthread_attr_getdetachstate(&m_attr, &detachstate);
	if(0 != ret)
	{
		log_error("get thread attr detach state failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::SetThreadAttrPolicy(const int policy)
{
	int ret = pthread_attr_setschedpolicy(&m_attr, policy);
	if(0 != ret)
	{
		log_error("set thread attr policy failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}
	
	return true;
}

bool CThreadBase::GetThreadAttrPolicy(int &policy)
{
	int ret = pthread_attr_getschedpolicy(&m_attr, &policy);
	if(0 != ret)
	{
		log_error("get thread attr policy failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::SetThreadAttrPriority(const int priority)
{
	int policy = 0;
	bool get_res = GetThreadAttrPolicy(policy);
	if(false == get_res)
	{
		log_error("GetThreadAttrPolicy failed.");
		return false;
	}
	if(false == IsCanSetPriority(policy))
	{
		log_error("IsCanSetPriority failed.");
		SetErrorCode(-TE_PRIORITY_UNSUPPORT);
		return false;
	}
	
	int max_priority = 0;
	int min_priority = 0;
	get_res = GetThreadAttrPriorityMax(policy, max_priority);
	if(false == get_res)
	{
		log_error("GetThreadAttrPriorityMax failed.");
		return false;
	}
	get_res = GetThreadAttrPriorityMin(policy, min_priority);
	if(false == get_res)
	{
		log_error("GetThreadAttrPriorityMin failed.");
		return false;
	}
	if(priority > max_priority || priority < min_priority)
	{
		log_error("priority [%d] is invalid.", priority);
		SetErrorCode(TE_PARAM_INVALID);
		return false;
	}

	struct sched_param scparam;
	scparam.sched_priority = priority;
	int ret = pthread_attr_setschedparam(&m_attr, &scparam);
	if(0 != ret)
	{
		log_error("set thread attr policy failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}
	
	return true;
}

bool CThreadBase::GetThreadAttrPriority(int &priority)
{
	struct sched_param scparam;
	int ret = pthread_attr_getschedparam(&m_attr, &scparam);
	if(0 != ret)
	{
		log_error("get thread attr priority failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	priority = scparam.sched_priority;
	return true;
}

bool CThreadBase::GetThreadAttrPriorityMax(int &priority)
{
	int policy = 0;
	bool get_res = GetThreadAttrPolicy(policy);
	if(false == get_res)
	{
		log_error("GetThreadAttrPolicy failed.");
		return false;
	}
	
	get_res = GetThreadAttrPriorityMax(policy, priority);
	return get_res;
}

bool CThreadBase::GetThreadAttrPriorityMax(const int policy, int &priority)
{
	if(false == IsCanSetPriority(policy))
	{
		return false;
	}

	int max_priority = sched_get_priority_max(policy);
	if(-1 == max_priority)
	{
		log_error("get thread attr priority max failed.");
		SetErrorCode(-TE_SERVER_ERROR);
		return false;
	}

	priority = max_priority;
	return true;
}

bool CThreadBase::GetThreadAttrPriorityMin(int &priority)
{
	int policy = 0;
	bool get_res = GetThreadAttrPolicy(policy);
	if(false == get_res)
	{
		log_error("GetThreadAttrPolicy failed.");
		return false;
	}
	
	get_res = GetThreadAttrPriorityMin(policy, priority);
	return get_res;
}

bool CThreadBase::GetThreadAttrPriorityMin(const int policy, int &priority)
{
	if(false == IsCanSetPriority(policy))
	{
		log_error("IsCanSetPriority failed.");
		SetErrorCode(-TE_PRIORITY_UNSUPPORT);
		return false;
	}

	int min_priority = sched_get_priority_min(policy);
	if(-1 == min_priority)
	{
		log_error("get thread attr priority min failed.");
		SetErrorCode(-TE_SERVER_ERROR);
		return false;
	}

	priority = min_priority;
	return true;
}

bool CThreadBase::SetThreadBindCpu(const int64_t cpu)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	bool set_res = SetThreadBindCpu(mask);
	return set_res;
}

bool CThreadBase::SetThreadBindCpu(const cpu_set_t &mask)
{
	int ret = sched_setaffinity(m_tid, sizeof(mask), &mask);
	if(ret < 0)
	{
		log_error("set thread bind cpu failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::GetThreadBindCpu(cpu_set_t &mask)
{
	CPU_ZERO(&mask);
	int ret = sched_getaffinity(m_tid, sizeof(mask), &mask);
	if(ret < 0)
	{
		log_error("get thread bind cpu failed, errcode [%d].", ret);
		return false;
	}

	return true;
}

bool CThreadBase::SetThreadCancelState(const int state, int *oldstate)
{
	int ret = pthread_setcancelstate(state, oldstate);
	if(0 != ret)
	{
		log_error("set thread cancel state failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::SetThreadCancelType(const int type, int *oldtype)
{
	int ret = pthread_setcanceltype(type, oldtype);
	if(0 != ret)
	{
		log_error("set thread cancel type failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

void CThreadBase::SetThreadCancelPoint()
{
	pthread_testcancel();
}

bool CThreadBase::SetThreadRunPriority(const int priority)
{
	int ret = pthread_setschedprio(m_threadid, priority);
	if(0 != ret)
	{
		log_error("set thread run priority failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::SetThreadRunPolicyAndPriority(const int policy, const int priority)
{
	if(false == IsCanSetPriority(policy))
	{
		log_error("IsCanSetPriority failed.");
		SetErrorCode(-TE_PRIORITY_UNSUPPORT);
		return false;
	}
	
	struct sched_param scparam;
	scparam.sched_priority = priority;

	int ret = pthread_setschedparam(m_threadid, policy, &scparam);
	if(0 != ret)
	{
		log_error("set thread run policy and priority failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	return true;
}

bool CThreadBase::GetThreadRunPolicyAndPriority(int &policy, int &priority)
{
	int get_policy = 0;
	struct sched_param scparam;
	int ret = pthread_getschedparam(m_threadid, &get_policy, &scparam);
	if(0 != ret)
	{
		log_error("get thread run policy and priority failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	policy = get_policy;
	priority = scparam.sched_priority;
	return true;
}

bool CThreadBase::Detach()
{
	int ret = pthread_detach(m_threadid);
	if(0 != ret)
	{
		log_error("detach thread [%llu] failed, tid [%d], threadname [%s], errcode [%d].", static_cast<long long unsigned int>(m_threadid), m_tid, m_threadname.c_str(), ret);
		SetErrorCode(ret);
		return false;
	}
	
	return true;
}

bool CThreadBase::Join(void **msg)
{
	if(m_threadid <= 0)
	{
		log_debug("this thread had been join.");
		return true;
	}
	int ret = 0;
	pthread_join(m_threadid, msg);
	if(0 != ret)
	{
		log_error("join thread [%llu] failed, tid [%d], threadname [%s], errcode [%d].", static_cast<long long unsigned int>(m_threadid), m_tid, m_threadname.c_str(), ret);
		SetErrorCode(ret);
		return false;
	}

	log_debug("join thread [%llu] success, tid [%d], threadname [%s].", static_cast<long long unsigned int>(m_threadid), m_tid, m_threadname.c_str());
	m_threadid = 0;
	return true;
}

bool CThreadBase::Yield()
{
	int ret = pthread_yield();
	if(0 != ret)
	{
		log_error("yield thread [%llu] failed, tid [%d], threadname [%s], errcode [%d].", static_cast<long long unsigned int>(m_threadid), m_tid, m_threadname.c_str(), ret);
		SetErrorCode(ret);
		return false;
	}
	
	return true;
}

pthread_t CThreadBase::Self()
{
	return pthread_self();
}

bool CThreadBase::Start()
{
	THREAD_DEBUG();
	if(false == InitializeBase())
	{
		log_error("InitializeBase failed.");
		return false;
	}
	
	int ret = pthread_create(&m_threadid, &m_attr, CThreadBase::ThreadFunction, this);
	if(0 != ret)
	{
		log_error("create thread [%s] failed, errcode [%d].", m_threadname.c_str(), ret);
		SetErrorCode(ret);
		return false;
	}
	THREAD_DEBUG();

	return true;
}

/**
 *Linux下没有挂起唤醒操作
 **/
bool CThreadBase::Wakeup()
{
	return true;
}

bool CThreadBase::Terminate()
{
	int ret = pthread_cancel(m_threadid);
	if(0 != ret)
	{
		log_error("cancel thread [%llu] failed, tid [%d], threadname [%s], errcode [%d].", static_cast<unsigned long long int>(m_threadid), m_tid, m_threadname.c_str(), ret);
		SetErrorCode(ret);
		return false;
	}
	
	return true;
}

void CThreadBase::Exit(void *msg /* = NULL */)
{
	if(NULL == msg)
	{
		pthread_exit(msg);
	}
	else
	{
		pthread_exit(static_cast<void *>(const_cast<char *>(m_threadname.c_str())));
	}
}

void *CThreadBase::ThreadFunction(void *thread)
{
	CThreadBase *myself = static_cast<CThreadBase *>(thread);
	myself->PrepareBase();
	void *ret = myself->Run();
	myself->DestroyBase();
	return ret;
}

void CThreadBase::CleanData(const std::string threadname /* = "" */, bool detach /* = false */)
{
	m_errorcode = 0;
	m_tid = 0;
	m_threadid = 0;
	m_threadname = threadname;
	m_threadstate = THREAD_EXIT;
	m_isdetach = detach;
	m_stop = 0;
}

bool CThreadBase::InitializeBase()
{
	int ret = pthread_attr_init(&m_attr);
	if(0 != ret)
	{
		log_error("thread attr init failed, errcode [%d].", ret);
		SetErrorCode(ret);
		return false;
	}

	bool init_res = Initialize();
	if(true != init_res)
	{
		return false;
	}

	return true;
}

void CThreadBase::PrepareBase()
{
	if(true == m_isdetach)
	{
		pthread_detach(pthread_self());
	}
	
	m_tid = gettid();
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &sset, &sset);

	Prepare();

	SetThreadState(THREAD_IDLE);

	log_error("the thread [%s], threadid [%llu], tid [%d] runing...", m_threadname.c_str(), static_cast<unsigned long long int>(m_threadid), m_tid);
}

void CThreadBase::DestroyBase()
{
	SetThreadState(THREAD_EXIT);
	//m_tid = 0;
	//m_threadid = 0;
	
	Destroy();

	log_error("the thread [%s], threadid [%llu], tid [%d] destroy.", m_threadname.c_str(), static_cast<unsigned long long int>(m_threadid), m_tid);
}

bool CThreadBase::IsCanSetPriority(const int policy)
{
	if(SCHED_FIFO != policy || SCHED_RR != policy)
	{
		log_error("policy [%d] do not support priority.", policy);
		return false;
	}

	return true;
}
