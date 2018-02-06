#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__


class NonCopyable
{
protected:
	NonCopyable() {};
	~NonCopyable() {};

private:
	NonCopyable(const NonCopyable &);
	const NonCopyable &operator=(const NonCopyable &);
};

#endif
