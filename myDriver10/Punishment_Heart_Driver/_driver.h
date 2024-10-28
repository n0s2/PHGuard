#pragma once
#include <ntddk.h>

#define CTL_SYS \
    CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_BUFFERED,FILE_ANY_ACCESS)

EXTERN_C BOOLEAN d_jdVM();

/*   struct declaration   */
#ifdef _WIN64
/*   here is to distinguish Windows Kits  */
typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	ULONG64 DllBase;
	ULONG64 EntryPoint;
	ULONG64 SizeOfImage;
	UNICODE_STRING FullDllName;// "\??\C:\Users\Lenovo\Desktop\myDriver1.sys"
	UNICODE_STRING BaseDllName;// "myDriver1.sys";
	ULONG Flags;
}LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PS_PROTECTION {
	union {
		UCHAR Level;
		struct {
			UCHAR Type : 3;
			UCHAR Audit : 1;
			UCHAR Signer : 4;
		};
	}un;
}PS_PROTECTION, * PPS_PROTECTION;

#else
typedef struct _LDR_DATA_TABLE_ENTRY {

};
#endif
typedef struct _INITIAL_TEB {
	PVOID                StackBase;
	PVOID                StackLimit;
	PVOID                StackCommit;
	PVOID                StackCommitMax;
	PVOID                StackReserved;
} INITIAL_TEB, * PINITIAL_TEB;
//本文中所有非公开函数定义均来自 http://undocumented.ntinternals.net/


/*   function declaration   */
NTKERNELAPI
UCHAR* PsGetProcessImageFileName(__in PEPROCESS Process);//PsGetProcessImageFileName

NTSYSAPI
NTSTATUS NTAPI NtCreateThread(
	OUT PHANDLE             ThreadHandle,
	IN ACCESS_MASK          DesiredAccess,
	IN POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
	IN HANDLE               ProcessHandle,
	OUT PCLIENT_ID          ClientId,
	IN PCONTEXT             ThreadContext,
	IN PINITIAL_TEB         InitialTeb,
	IN BOOLEAN              CreateSuspended);


NTSTATUS d_processHide(ULONG ulPid);
NTSTATUS d_processHideByEP(ULONG64 ulEProcess);
NTSTATUS d_clearDbgPort(ULONG64 ulpEPROCESS);
NTSTATUS d_setProcessCallbacks();
NTSTATUS d_setProtection(ULONG64 ulpEPROCESS);
NTSTATUS d_getSSDTBase();
BOOLEAN d_VMEnvironment();

extern PVOID g_pRegistrationHandle;
extern ULONG g_ulProcessId;
extern ULONG64 g_ulEProcess;