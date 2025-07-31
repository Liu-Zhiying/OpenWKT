#ifndef THREAD_H
#define THREAD_H

#include "AllocMemory.h"
#include "ArgPack.h"
#include "Basic.h"

constexpr UINT32 threadTag = 'THRE';

class KernelThread
{
	HANDLE hThread;

	template<typename ArgPackT, typename Func>
	struct Pack
	{
		ArgPackT argPack;
		Func func;

	public:
		Pack(ArgPackT&& _argPack, Func&& _func) : argPack(static_cast<ArgPackT&&>(_argPack)), func(static_cast<Func&&>(_func)) {}
	};

	template<typename Pack>
	static void ThreadEntry(PVOID StartContext)
	{
		Pack* pack = (Pack*)StartContext;
		UnpackArgsAndCall::UnpackAndCall(pack->func, pack->argPack);
		CallDestroyer<Pack>(pack);
		FreeNonPagedMem(pack, threadTag);
	}

	KernelThread(const KernelThread&) = delete;
	KernelThread& operator=(const KernelThread&) = delete;

	KernelThread(KernelThread&& other)
	{
		hThread = other.hThread;
		other.hThread = NULL;
	}

	KernelThread& operator=(KernelThread&& other)
	{
		if (this == &other)
			return *this;
		hThread = other.hThread;
		other.hThread = NULL;
		return *this;
	}

public:

	KernelThread() : hThread(NULL) {}

	template<typename ArgPackT, typename Func>
	KernelThread(ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PCLIENT_ID ClientId, Func&& func, ArgPackT&& argPack) : hThread(NULL)
	{
		Pack<ArgPackT, Func>* pack = (Pack<ArgPackT, Func>*)AllocNonPagedMem(sizeof(Pack<ArgPackT, Func>), threadTag);
		if (pack == NULL)
			KeBugCheck(MEMORY_MANAGEMENT);
		new (pack) Pack<ArgPackT, Func> (static_cast<ArgPackT&&>(argPack), static_cast<Func&&>(func));
		PsCreateSystemThread(&hThread, DesiredAccess, ObjectAttributes, ProcessHandle, ClientId, ThreadEntry<Pack<ArgPackT, Func>>, pack);
	}

	HANDLE GetHandle() { return hThread; }
};

class KernelThreadFactory
{
	template<typename OriginType>
	class GetRReferenceType
	{
	public:
		using RReferenceType = OriginType&&;
	};

	template<typename OriginType>
	class GetPointerType
	{
	public:
		using PointerType = OriginType*;
	};

public:
	template<typename Func, typename ...Args>
	static auto CreateKernelThread(ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PCLIENT_ID ClientId, Func&& func, Args... args)
	{
		auto argPack = PackArgs::pack(args...);
		return KernelThread(DesiredAccess, ObjectAttributes, ProcessHandle, ClientId, 
			static_cast<GetRReferenceType<decltype(func)>::RReferenceType>(func),
			static_cast<GetRReferenceType<decltype(argPack)>::RReferenceType>(argPack));
	}
};

#endif
