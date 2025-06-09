#ifndef KERNELVECTOR_H
#define KERNELVECTOR_H

#include "Basic.h"
#include "AllocMemory.h"

enum MemType
{
	NonPaged,
	Paged,
	ContigousMem,
};

//类似C++11中的std::vector，内核模式的变长数组
template<typename ElementType, UINT32 allocTag, MemType memType = NonPaged>
class KernelVector
{
	ElementType* pData;
	SIZE_TYPE length;
	SIZE_TYPE capacity;

	PVOID(*pMemAlloc)(SIZE_TYPE byteCnt, ULONG tag);
	void (*pMemFree)(PVOID pMem, ULONG tag);

public:
	KernelVector();
	~KernelVector();

	KernelVector(KernelVector&& container);
	KernelVector& operator=(KernelVector&& container);

	KernelVector(const KernelVector& container);
	KernelVector& operator=(const KernelVector& container);

	void PushBack(ElementType e);
	void EmplaceBack(ElementType&& e);
	ElementType PopBack();
	const ElementType& operator[](SIZE_TYPE idx) const;
	ElementType& operator[](SIZE_TYPE idx);
	void Insert(ElementType e, SIZE_TYPE idx);
	void Remove(SIZE_TYPE idx);
	SIZE_TYPE Length() const;
	SIZE_TYPE Capacity() const;
	void SetCapacity(SIZE_TYPE newCapacity);
	void Clear();
};

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline KernelVector<ElementType, allocTag, memType>::KernelVector() : pData(NULL), length(0), capacity(0)
{
	//根据内存类型，选择不同的内存分配释放函数

	if constexpr (memType == MemType::NonPaged)
	{
		pMemAlloc = AllocNonPagedMem;
		pMemFree = FreeNonPagedMem;
	}
	else if constexpr (memType == MemType::Paged)
	{
		pMemAlloc = AllocPagedMem;
		pMemFree = FreePagedMem;
	}
	else if constexpr (memType = MemType::ContigousMem)
	{
		pMemAlloc = AllocContiguousMem;
		pMemFree = FreeContigousMem;
	}
	else
	{
		__debugbreak();
		KeBugCheck(DRIVER_INVALID_CRUNTIME_PARAMETER);
	}
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline KernelVector<ElementType, allocTag, memType>::~KernelVector()
{
	if (pData != NULL)
	{
		for (SIZE_TYPE idx = 0; idx < length; ++idx)
			CallDestroyer(pData + idx);

		pMemFree(pData, allocTag);

		pData = NULL;
	}
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline KernelVector<ElementType, allocTag, memType>::KernelVector(KernelVector&& container) : KernelVector()
{
	*this = static_cast<KernelVector<ElementType, allocTag, memType>&&>(container);
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline KernelVector<ElementType, allocTag, memType>& KernelVector<ElementType, allocTag, memType>::operator=(KernelVector&& container)
{
	if (&container == this)
		return *this;

	this->~KernelVector();

	pData = container.pData;
	length = container.length;
	capacity = container.capacity;
	container.pData = NULL;
	container.length = 0;
	container.capacity = 0;

	return *this;
}

template<typename ElementType, UINT32 allocTag, MemType memType>
inline KernelVector<ElementType, allocTag, memType>::KernelVector(const KernelVector& container) : KernelVector()
{
	*this = container;
}

template<typename ElementType, UINT32 allocTag, MemType memType>
inline KernelVector<ElementType, allocTag, memType>& KernelVector<ElementType, allocTag, memType>::operator=(const KernelVector& container)
{
	if (&container == this)
		return *this;

	Clear();

	SetCapacity(container.Capacity());

	for (SIZE_TYPE idx = 0; idx < container.Length(); ++idx)
		CallConstructor(pData + idx, container[idx]);

	length = container.Length();

	return *this;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline void KernelVector<ElementType, allocTag, memType>::PushBack(ElementType e)
{
	if (length == capacity)
		SetCapacity(!length ? 50 : length * 2);
	pData[length++] = e;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline void KernelVector<ElementType, allocTag, memType>::EmplaceBack(ElementType&& e)
{
	if (length == capacity)
		SetCapacity(!length ? 50 : length * 2);
	CallConstructor(&pData[length]);
	pData[length++] = static_cast<ElementType&&>(e);
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline ElementType KernelVector<ElementType, allocTag, memType>::PopBack()
{
	ElementType result = static_cast<ElementType&&>(pData[--length]);
	return result;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline const ElementType& KernelVector<ElementType, allocTag, memType>::operator[](SIZE_TYPE idx) const
{
	if (idx < Length())
	{
		return pData[idx];
	}
	else
	{
		__debugbreak();
		KeBugCheck(MEMORY_MANAGEMENT);
	}
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline ElementType& KernelVector<ElementType, allocTag, memType>::operator[](SIZE_TYPE idx)
{
	if (idx < Length())
	{
		return pData[idx];
	}
	else
	{
		__debugbreak();
		KeBugCheck(MEMORY_MANAGEMENT);
	}
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline void KernelVector<ElementType, allocTag, memType>::Insert(ElementType e, SIZE_TYPE idx)
{
	if (idx >= Length())
	{
		__debugbreak();
		KeBugCheck(MEMORY_MANAGEMENT);
	}

	if (length == capacity)
		SetCapacity(length + 50);

	for (SIZE_T idx2 = length; idx2 > idx; --idx2)
		pData[idx2] = static_cast<ElementType&&>(pData[idx2 - 1]);

	pData[idx] = e;

	++length;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline void KernelVector<ElementType, allocTag, memType>::Remove(SIZE_TYPE idx)
{
	if (idx >= Length())
	{
		__debugbreak();
		KeBugCheck(MEMORY_MANAGEMENT);
	}

	for (SIZE_TYPE idx2 = idx; idx2 < Length() - 1; ++idx2)
		pData[idx2] = static_cast<ElementType&&>(pData[idx2 + 1]);

	--length;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline SIZE_TYPE KernelVector<ElementType, allocTag, memType>::Length() const
{
	return length;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline SIZE_TYPE KernelVector<ElementType, allocTag, memType>::Capacity() const
{
	return capacity;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline void KernelVector<ElementType, allocTag, memType>::SetCapacity(SIZE_TYPE newCapacity)
{
	SIZE_TYPE copyLength = newCapacity > capacity ? capacity : newCapacity;

	ElementType* pNewData = (ElementType*)pMemAlloc(newCapacity * sizeof(ElementType), allocTag);

	if (pNewData == NULL)
	{
		__debugbreak();
		KeBugCheck(MEMORY_MANAGEMENT);
	}

	for (SIZE_TYPE idx = 0; idx < copyLength; ++idx)
		pNewData[idx] = static_cast<ElementType&&>(pData[idx]);

	if (copyLength < capacity)
	{
		for (SIZE_T idx = 0; idx < capacity; ++idx)
			CallDestroyer(pNewData + idx);
	}

	if (pData != NULL)
		pMemFree(pData, allocTag);

	pData = pNewData;
	capacity = newCapacity;
	length = copyLength;
}

#pragma code_seg()
template<typename ElementType, UINT32 allocTag, MemType memType>
inline void KernelVector<ElementType, allocTag, memType>::Clear()
{
	for (SIZE_TYPE idx = 0; idx < length; ++idx)
		CallDestroyer(pData + idx);

	length = 0;
}

#endif
