#ifndef __LOCK_OBJECT_H__
#define __LOCK_OBJECT_H__


class CLockObject
{
public:
	virtual bool Lock() = 0;
	virtual bool TryLock() = 0;
	virtual bool UnLock() = 0;
};


#endif
