#ifndef __THREAD_DEBUG_H__
#define __THREAD_DEBUG_H__


#include <stdio.h>
#include <stdlib.h>

#include <sys/syscall.h>

#include "thread_datetime.h"


#define DEBUG_TEST


inline int gettid()
{
	return syscall(__NR_gettid);
}

#if defined(DEBUG_TEST)
#define THREAD_DEBUG() fprintf(stdout, "tid[%d], threadid[%llu], file[%s], line[%d], func[%s] \n", gettid(), static_cast<long long unsigned int>(pthread_self()), __FILE__, __LINE__, __FUNCTION__)

#define INFO(format, ...) fprintf(stdout, "<INFO> [%s] pid[%d]:%s(%d)[%s]: "format"\n",DateTime::CurrentTime().c_str(), gettid(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ERROR(format, ...) fprintf(stdout, "<ERROR> [%s] pid[%d]:%s(%d)[%s]: "format"\n",DateTime::CurrentTime().c_str(), gettid(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define THREAD_DEBUG()

#define INFO(format, ...)
#define ERROR(format, ...)
#endif



#endif
