#pragma once
#include<ntifs.h>
#include<ntddk.h>
#pragma intrinsic(__readmsr)

//SSDT�Ľṹ
typedef struct _SYSTEM_SERVICE_TABLE {
	PLONG  		ServiceTableBase;
	PVOID  		ServiceCounterTableBase;
	ULONGLONG  	NumberOfServices;
	PVOID  		ParamTableBase;
} SYSTEM_SERVICE_TABLE, * PSYSTEM_SERVICE_TABLE;


/*ȫ�ֱ���*/
extern PSYSTEM_SERVICE_TABLE SSDT��ַ;

//��������
UINT64 ��ȡSSDT��ַ();
UINT64 ��ȡ������ַ(ULONG dwIndex);
KIRQL �ر�д����();
VOID ����д����(KIRQL irql);
VOID ��ʼHOOK(UINT64 HOOK������ַ, UINT64 ��������ַ, USHORT ��д�ĳ���, PVOID* ԭ����);
VOID �ָ�HOOK(UINT64 HOOK������ַ, USHORT ��д�ĳ���, PVOID ԭ����);
