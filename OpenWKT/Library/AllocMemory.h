#ifndef ALLOCMEMORY_H
#define ALLOCMEMORY_H

#include "Basic.h"
#include <ntddk.h>

//封装一下内存分配函数
//封装一下Windows内核内存分配函数
#pragma code_seg()
inline PVOID AllocNonPagedMem(SIZE_TYPE byteCnt, ULONG tag)
{
#ifdef _BUILD_WIN_2004
	return ExAllocatePool2(POOL_FLAG_NON_PAGED, byteCnt, tag);
#else
	return ExAllocatePoolWithTag(POOL_TYPE::NonPagedPool, byteCnt, tag);
#endif
}

#pragma code_seg()
inline void FreeNonPagedMem(PVOID pMem, ULONG tag)
{
	ExFreePoolWithTag(pMem, tag);
}

#pragma code_seg()
inline PVOID AllocPagedMem(SIZE_TYPE byteCnt, ULONG tag)
{
#ifdef _BUILD_WIN_2004
	return ExAllocatePool2(POOL_FLAG_PAGED, byteCnt, tag);
#else
	return ExAllocatePoolWithTag(POOL_TYPE::PagedPool, byteCnt, tag);
#endif
}

#pragma code_seg()
inline void FreePagedMem(PVOID pMem, ULONG tag)
{
	ExFreePoolWithTag(pMem, tag);
}

#pragma code_seg()
inline PVOID AllocContiguousMem(SIZE_TYPE byteCnt, ULONG tag)
{
	UNREFERENCED_PARAMETER(tag);
	constexpr LARGE_INTEGER highestPhyAddr = { (ULONG)-1,-1 };
	return MmAllocateContiguousMemory(byteCnt, highestPhyAddr);
}

#pragma code_seg()
inline void FreeContigousMem(PVOID pMem, ULONG tag)
{
	UNREFERENCED_PARAMETER(tag);
	return MmFreeContiguousMemory(pMem);
}

#pragma code_seg()
inline PVOID AllocExecutableNonPagedMem(SIZE_TYPE byteCnt, ULONG tag)
{
#ifdef _BUILD_WIN_2004
	return ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, byteCnt, tag);
#else
	return ExAllocatePoolWithTag(POOL_TYPE::NonPagedPoolExecute, byteCnt, tag);
#endif
}

#pragma code_seg()
inline void FreeExecutableNonPagedMem(PVOID pMem, ULONG tag)
{
	ExFreePoolWithTag(pMem, tag);
}

#endif
