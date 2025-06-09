#ifndef BASIC_H
#define BASIC_H

#include <ntddk.h>

//������64λWindows
#define WINDOWS_X64

//����Windows 2004���ϰ汾build
#define _BUILD_WIN_2004

//һЩ���ͱ���
typedef unsigned char UINT8;
typedef char SINT8;
typedef unsigned short UINT16;
typedef short SINT16;
typedef unsigned int UINT32;
typedef int SINT32;
typedef unsigned long long UINT64;
typedef long long SINT64;
#if defined(WINDOWS_X64)
typedef UINT64 SIZE_TYPE;
typedef UINT64 PTR_TYPE;
#elif defined(WINDOWS_X86)
typedef UINT32 SIZE_TYPE;
typedef UINT32 PTR_TYPE;
#endif

#endif // !BASIC_H
