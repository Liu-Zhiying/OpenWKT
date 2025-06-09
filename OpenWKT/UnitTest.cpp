#include "Library/PlacementNew.h"
#include "Library/HelperFunction.h"
#include "Library/KernelVector.h"
#include "Library/ArgPack.h"
#include "Library/Traits.h"
#include <ntddk.h>

// 测试 Traits.h 中的类型特征模板
void TestTraits() {
	// 模拟一个类用于测试
	class TestClass {};
    // 测试 IsClassType
    static_assert(IsClassType<TestClass>::Value, "IsClassType test failed");
    static_assert(!IsClassType<int>::Value, "IsClassType test failed");

    // 测试 IsPointerType
    static_assert(IsPointerType<int*>::Value, "IsPointerType test failed");
    static_assert(!IsPointerType<int>::Value, "IsPointerType test failed");

    // 测试 IsExtendFrom
    class Base {};
    class Derived : public Base {};
    static_assert(IsExtendFrom<Base, Derived>::Value, "IsExtendFrom test failed");
    static_assert(!IsExtendFrom<Base, int>::Value, "IsExtendFrom test failed");
}

// 测试 ArgPack.h 中的参数打包和解包功能
void TestArgPack() {
    // 测试1个参数
    auto argPack1 = PackArgs::pack(42);
    auto result1 = 0;
    auto func1 = [&](int a) {
        result1 = a;
    };
    UnpackArgsAndCall::UnpackAndCall(func1, argPack1);
    if (result1 != 42) {
        DbgPrint("ArgPack 1 parameter test failed\n");
    }

    // 测试2个参数
    auto argPack2 = PackArgs::pack(1, 2);
    auto result2 = 0;
    auto func2 = [&](int a, int b) {
        result2 = static_cast<int>(a + b);
    };
    UnpackArgsAndCall::UnpackAndCall(func2, argPack2);
    if (result2 != 3) {
        DbgPrint("ArgPack 2 parameters test failed\n");
    }

    // 测试3个参数
    auto argPack3 = PackArgs::pack(1, 2, "three");
    auto result3 = 0;
    auto func3 = [&](int a, int b, const char* c) {
        result3 = a + b;
        UNREFERENCED_PARAMETER(c);
    };
    UnpackArgsAndCall::UnpackAndCall(func3, argPack3);
    if (result3 != 3) {
        DbgPrint("ArgPack 3 parameters test failed\n");
    }

    char strBuf[] = "aaa";

    // 测试4个参数
    auto argPack4 = PackArgs::pack(1, 2, 3, strBuf);
    auto result4 = 0;
    auto func4 = [&](int a, int b, int c, char* d) {
        result4 = a + b + c;
        d[0] = d[1] = d[2] = 'b';
    };
    UnpackArgsAndCall::UnpackAndCall(func4, argPack4);
    if (result4 != 6 || strcmp(strBuf, "bbb")) {
        DbgPrint("ArgPack 4 parameters test failed\n");
    }
}

// 测试 HelperFunction.h 中的 RunOnEachCore 函数
void TestHelperFunction() {
    auto func1 = [](UINT32 cpuIdx) {
        UNREFERENCED_PARAMETER(cpuIdx);
        return STATUS_SUCCESS;
    };
    NTSTATUS status = RunOnEachCore(0, 1, func1);
    if (!NT_SUCCESS(status)) {
        DbgPrint("RunOnEachCore test failed\n");
    }

    auto func2 = [](UINT32 cpuIdx, const char* str1, const char* str2) {
        DbgPrint("Cpuidx = %u, str1 = %s, str2 = %s\n", cpuIdx, str1, str2);
        return STATUS_SUCCESS;
    };

    status = RunOnEachCore(0, 1, func2, "aaa", "bbb");
    if (!NT_SUCCESS(status)) {
        DbgPrint("RunOnEachCore test failed\n");
    }
}

// 测试 KernelVector.h 的功能
void TestKernelVector() {

    constexpr UINT32 Tag = 'Test';

    // 使用自定义placement new在预分配的内存上创建KernelVector
    constexpr size_t bufferSize = 1024;
    unsigned char buffer[bufferSize];
    
    // 使用自定义的placement new操作
	CallConstructor((KernelVector<int, Tag>*)buffer);

    KernelVector<int, Tag>* pVec = (KernelVector<int, Tag>*)buffer;
    
    // 测试基本功能
    pVec->PushBack(1);
    pVec->PushBack(2);
    pVec->PushBack(3);
    
    if (pVec->Length() != 3) {
        DbgPrint("KernelVector size test failed\n");
    }
    
    if ((*pVec)[0] != 1 || (*pVec)[1] != 2 || (*pVec)[2] != 3) {
        DbgPrint("KernelVector element access test failed\n");
    }

	// 测试复制构造
    KernelVector<int, Tag> copiedVec(*pVec);
    if (copiedVec.Length() != 3 || copiedVec[0] != 1 || copiedVec[1] != 2 || copiedVec[2] != 3) {
        DbgPrint("KernelVector copy constructor test failed\n");
    }

    // 测试移动构造
    KernelVector<int, Tag> movedVec(static_cast<KernelVector<int, Tag>&&>(copiedVec));
    if (movedVec.Length() != 3 || movedVec[0] != 1 || movedVec[1] != 2 || movedVec[2] != 3 || copiedVec.Length() != 0) {
        DbgPrint("KernelVector move constructor test failed\n");
    }

    // 测试删除操作
    movedVec.PopBack();
    if (movedVec.Length() != 2 || movedVec[0] != 1 || movedVec[1] != 2) {
        DbgPrint("KernelVector erase test failed\n");
    }

    movedVec.Remove(0);
    if (movedVec.Length() != 1 || movedVec[0] != 2) {
        DbgPrint("KernelVector erase test failed\n");
    }
    
    //测试插入操作
    movedVec.Insert(20, 0);
    if (movedVec.Length() != 2 || movedVec[0] != 20 || movedVec[0] !=2) {
        DbgPrint("KernelVector erase test failed\n");
    }

    // 测试clear操作
    movedVec.Clear();
    if (movedVec.Length() != 0) {
        DbgPrint("KernelVector clear test failed\n");
    }
    
    // 显式调用析构函数
    CallDestroyer(pVec);
}

void DriverUnload(
    _In_ struct _DRIVER_OBJECT* pDriverObject
)
{
    UNREFERENCED_PARAMETER(pDriverObject);

    return;
}

extern "C" NTSTATUS DriverEntry(PUNICODE_STRING pRegisterPath, PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pRegisterPath);
    
    pDriverObject->DriverUnload = DriverUnload;

    // 运行所有测试用例
    TestTraits();
    TestArgPack();
    TestHelperFunction();
	TestKernelVector();

    return STATUS_SUCCESS;
}