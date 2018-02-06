#ifndef __THREAD_CONSTANT_H__
#define __THREAD_CONSTANT_H__


enum ThreadState
{
	THREAD_IDLE = 1,	//空闲
	THREAD_RUNING,		//运行中
	THREAD_PAUSE,		//挂起(暂停，Linux线程库不支持挂起操作)
	THREAD_EXIT,		//退出(包括正常和非正常退出)
};


#endif
