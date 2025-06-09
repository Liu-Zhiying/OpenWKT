#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include "Basic.h"

//在每一个CPU核心上都运行一次指定的可执行对象
#pragma code_seg("PAGE")
template<typename Func, typename ...Args>
NTSTATUS RunOnEachCore(UINT32 startCoreIdx, UINT32 endCoreIdx, Func&& func, Args&& ...args)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	PROCESSOR_NUMBER processorNum = {};
	GROUP_AFFINITY affinity = {}, oldAffinity = {};

	for (UINT32 cpuIdx = startCoreIdx; cpuIdx < endCoreIdx; ++cpuIdx)
	{
		status = KeGetProcessorNumberFromIndex(cpuIdx, &processorNum);
		if (!NT_SUCCESS(status))
			break;

		affinity = {};
		affinity.Group = processorNum.Group;
		affinity.Mask = 1ULL << processorNum.Number;

		KeSetSystemGroupAffinityThread(&affinity, &oldAffinity);

		status = func(cpuIdx, args...);

		KeRevertToUserGroupAffinityThread(&oldAffinity);

		if (!NT_SUCCESS(status))
			break;
	}

	return status;
}

//帮助函数，取数组的元素数目
#pragma code_seg()
template<typename T, SIZE_TYPE n>
SIZE_TYPE GetArrayElementCnt(T(&)[n])
{
	return n;
}

#endif
