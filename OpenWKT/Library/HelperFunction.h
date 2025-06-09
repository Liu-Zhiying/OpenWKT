#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include "Basic.h"

//��ÿһ��CPU�����϶�����һ��ָ���Ŀ�ִ�ж���
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

//����������ȡ�����Ԫ����Ŀ
#pragma code_seg()
template<typename T, SIZE_TYPE n>
SIZE_TYPE GetArrayElementCnt(T(&)[n])
{
	return n;
}

#endif
