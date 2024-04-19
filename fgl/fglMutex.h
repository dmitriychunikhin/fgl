#ifndef _FGLMUTEX_H_
#define _FGLMUTEX_H_

#include "common.h"

class fglMutex //windows mutex is slow, so use critical section instead
{
public:
	fglMutex(void) = default;
	//bad for perf virtual ~fglMutex(void);
	
	bool tryLock(void);
	void lock(void);
	void unlock(void);

protected:
	CRITICAL_SECTION hMutex;
};

#endif
