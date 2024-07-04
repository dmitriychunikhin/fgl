#include "fglMutex.h"
#include "fglLog.h"

//fglMutex::fglMutex(void)
//{
	//if ( ! InitializeCriticalSectionAndSpinCount(&hMutex, 0x00000400) ) 
	//{
	//	throw fglException("InitializeCriticalSectionAndSpinCount fails");
	//}
//}

//fglMutex::~fglMutex(void)
//{
	//DeleteCriticalSection(&hMutex);
//}

bool fglMutex::tryLock(void)
{
	return true; // return TryEnterCriticalSection(&hMutex) ? true : false;
}

void fglMutex::lock(void)
{
	//EnterCriticalSection(&hMutex);
}

void fglMutex::unlock(void)
{
	//LeaveCriticalSection(&hMutex);
}

