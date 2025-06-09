#include "Library/PlacementNew.h"
#include "Library/HelperFunction.h"
#include "Library/KernelVector.h"
#include "Library/ArgPack.h"
#include "Library/Traits.h"
#include <ntddk.h>

// ���� Traits.h �е���������ģ��
void TestTraits() {
	// ģ��һ�������ڲ���
	class TestClass {};
    // ���� IsClassType
    static_assert(IsClassType<TestClass>::Value, "IsClassType test failed");
    static_assert(!IsClassType<int>::Value, "IsClassType test failed");

    // ���� IsPointerType
    static_assert(IsPointerType<int*>::Value, "IsPointerType test failed");
    static_assert(!IsPointerType<int>::Value, "IsPointerType test failed");

    // ���� IsExtendFrom
    class Base {};
    class Derived : public Base {};
    static_assert(IsExtendFrom<Base, Derived>::Value, "IsExtendFrom test failed");
    static_assert(!IsExtendFrom<Base, int>::Value, "IsExtendFrom test failed");
}

// ���� ArgPack.h �еĲ�������ͽ������
void TestArgPack() {
    // ����1������
    auto argPack1 = PackArgs::pack(42);
    auto result1 = 0;
    auto func1 = [&](int a) {
        result1 = a;
    };
    UnpackArgsAndCall::UnpackAndCall(func1, argPack1);
    if (result1 != 42) {
        DbgPrint("ArgPack 1 parameter test failed\n");
    }

    // ����2������
    auto argPack2 = PackArgs::pack(1, 2);
    auto result2 = 0;
    auto func2 = [&](int a, int b) {
        result2 = static_cast<int>(a + b);
    };
    UnpackArgsAndCall::UnpackAndCall(func2, argPack2);
    if (result2 != 3) {
        DbgPrint("ArgPack 2 parameters test failed\n");
    }

    // ����3������
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

    // ����4������
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

// ���� HelperFunction.h �е� RunOnEachCore ����
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

// ���� KernelVector.h �Ĺ���
void TestKernelVector() {

    constexpr UINT32 Tag = 'Test';

    // ʹ���Զ���placement new��Ԥ������ڴ��ϴ���KernelVector
    constexpr size_t bufferSize = 1024;
    unsigned char buffer[bufferSize];
    
    // ʹ���Զ����placement new����
	CallConstructor((KernelVector<int, Tag>*)buffer);

    KernelVector<int, Tag>* pVec = (KernelVector<int, Tag>*)buffer;
    
    // ���Ի�������
    pVec->PushBack(1);
    pVec->PushBack(2);
    pVec->PushBack(3);
    
    if (pVec->Length() != 3) {
        DbgPrint("KernelVector size test failed\n");
    }
    
    if ((*pVec)[0] != 1 || (*pVec)[1] != 2 || (*pVec)[2] != 3) {
        DbgPrint("KernelVector element access test failed\n");
    }

	// ���Ը��ƹ���
    KernelVector<int, Tag> copiedVec(*pVec);
    if (copiedVec.Length() != 3 || copiedVec[0] != 1 || copiedVec[1] != 2 || copiedVec[2] != 3) {
        DbgPrint("KernelVector copy constructor test failed\n");
    }

    // �����ƶ�����
    KernelVector<int, Tag> movedVec(static_cast<KernelVector<int, Tag>&&>(copiedVec));
    if (movedVec.Length() != 3 || movedVec[0] != 1 || movedVec[1] != 2 || movedVec[2] != 3 || copiedVec.Length() != 0) {
        DbgPrint("KernelVector move constructor test failed\n");
    }

    // ����ɾ������
    movedVec.PopBack();
    if (movedVec.Length() != 2 || movedVec[0] != 1 || movedVec[1] != 2) {
        DbgPrint("KernelVector erase test failed\n");
    }

    movedVec.Remove(0);
    if (movedVec.Length() != 1 || movedVec[0] != 2) {
        DbgPrint("KernelVector erase test failed\n");
    }
    
    //���Բ������
    movedVec.Insert(20, 0);
    if (movedVec.Length() != 2 || movedVec[0] != 20 || movedVec[0] !=2) {
        DbgPrint("KernelVector erase test failed\n");
    }

    // ����clear����
    movedVec.Clear();
    if (movedVec.Length() != 0) {
        DbgPrint("KernelVector clear test failed\n");
    }
    
    // ��ʽ������������
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

    // �������в�������
    TestTraits();
    TestArgPack();
    TestHelperFunction();
	TestKernelVector();

    return STATUS_SUCCESS;
}