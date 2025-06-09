#ifndef PLACEMENTNEW_H
#define PLACEMENTNEW_H

#include "Basic.h"

void* operator new(UINT64, void* pObj)
{
	return pObj;
}

void operator delete(void*, UINT64)
{
	return;
}

#pragma code_seg()
template<typename T, typename ...Args>
inline void CallConstructor(T* pObj, Args&& ...args)
{
	new (pObj) T(args...);
}

#pragma code_seg()
template<typename T>
inline void CallDestroyer(T* pObj)
{
	delete (0, pObj);
}

#endif
