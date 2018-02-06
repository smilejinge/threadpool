#ifndef __THREAD_JOB_H__
#define __THREAD_JOB_H__


#include <string>


class CThreadBase;


class CThreadJob
{
public:
	CThreadJob() {}
	virtual ~CThreadJob() {}
	virtual void Destroy() {};
	virtual void Run() = 0;

public:
	void SetJobName(const std::string &jobname)
	{
		m_jobname = jobname;
	}
	std::string GetJobName()
	{
		return m_jobname;
	}
	void SetWorkThread(CThreadBase *thread)
	{
		m_workthread = thread;
	}
	CThreadBase *GetWorkThread()
	{
		return m_workthread;
	}
	void SetJobData(void *jobdata)
	{
		m_jobdata = jobdata;
	}
	void *GetJobData()
	{
		return m_jobdata;
	}

private:
	std::string m_jobname;
	CThreadBase *m_workthread;
	void *m_jobdata;
};


#endif
