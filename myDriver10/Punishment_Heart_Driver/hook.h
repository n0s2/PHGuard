#pragma once
#include<ntifs.h>
#include<ntddk.h>
#pragma intrinsic(__readmsr)

//SSDT的结构
typedef struct _SYSTEM_SERVICE_TABLE {
	PLONG  		ServiceTableBase;
	PVOID  		ServiceCounterTableBase;
	ULONGLONG  	NumberOfServices;
	PVOID  		ParamTableBase;
} SYSTEM_SERVICE_TABLE, * PSYSTEM_SERVICE_TABLE;


/*全局变量*/
extern PSYSTEM_SERVICE_TABLE SSDT地址;

//函数声明
UINT64 获取SSDT地址();
UINT64 获取函数地址(ULONG dwIndex);
KIRQL 关闭写保护();
VOID 开启写保护(KIRQL irql);
VOID 开始HOOK(UINT64 HOOK函数地址, UINT64 代理函数地址, USHORT 改写的长度, PVOID* 原函数);
VOID 恢复HOOK(UINT64 HOOK函数地址, USHORT 改写的长度, PVOID 原函数);
