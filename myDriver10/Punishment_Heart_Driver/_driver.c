#include "_driver.h"
#include "utils.h"

/*扫描进程 禁止加速*/

NTSTATUS d_processHide(ULONG ulPid) {
	/*进程隐藏
		1.将目标进程从activeProcessLinks中摘除。
		2.伪造父进程句柄，欺骗便利pid获取进程的程序
		3.将EPROCESS信息从PspCidTable中抹去。
		4.程序关闭时恢复上述操作，不然会导致蓝屏。
	*/
	ULONG64 pEPROCESS = 0;
	ULONG ulProcessId = 0;
	ULONG64 ulProtectedFlag = 0;

	pEPROCESS = (ULONG64)PsGetCurrentProcess();
	if (!pEPROCESS) return STATUS_UNSUCCESSFUL;

	PLIST_ENTRY headerNode = (PLIST_ENTRY)(pEPROCESS + 0x448);
	if (!headerNode)return STATUS_ACCESS_DENIED;

	PLIST_ENTRY nextNode = headerNode->Flink;
	if (!nextNode)return STATUS_ACCESS_DENIED;

	while (nextNode != headerNode) {
		pEPROCESS = ((ULONG64)nextNode - 0x448);
		if (!pEPROCESS) return STATUS_ACCESS_DENIED;

		ulProcessId = *((ULONG*)(pEPROCESS + 0x440));
		if (!ulProcessId) return STATUS_ACCESS_DENIED;

		if (ulProcessId == ulPid) {
			//_getRemoteEProcess();
			nextNode->Flink->Blink = nextNode->Blink;
			nextNode->Blink->Flink = nextNode->Flink;
			DbgPrint("[PH INFO]Hiding process successed!\n");
			return STATUS_SUCCESS;
		}
		nextNode = nextNode->Flink;
	}  

	return STATUS_SUCCESS;
}

NTSTATUS d_processHideByEP(ULONG64 ulEProcess) {
	PLIST_ENTRY nextNode = (PLIST_ENTRY)(ulEProcess + 0x448);
	nextNode->Flink->Blink = nextNode->Blink;
	nextNode->Blink->Flink = nextNode->Flink;
	DbgPrint("[PH INFO]Hiding process successed!\n");
	return STATUS_SUCCESS;
}

OB_PREOP_CALLBACK_STATUS PreOperationCallback(
	__in PVOID RegistrationContext,
	__in POB_PRE_OPERATION_INFORMATION OperationInformation
) {
	PEPROCESS pEPROCESS = NULL;
	PUCHAR ImgName = NULL;
#define PROCESS_TERMINATE 0x1

	pEPROCESS = OperationInformation->Object;
	if (!pEPROCESS) {
		//EPROCESS获取失败
		return STATUS_UNSUCCESSFUL;
	}
	ImgName = PsGetProcessImageFileName(pEPROCESS);
	//DbgPrint(ImgName);
	if (strstr(ImgName, "Calc") != NULL) {
		//字串判断，如果回调的进程是目标进程，则开始创建保护。
		if (OperationInformation->Operation = OB_OPERATION_HANDLE_CREATE) {
			if ((OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE) {
				//说明进程的访问访问权限
				OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = ~PROCESS_TERMINATE;
			}
		}
	}
	return OB_PREOP_SUCCESS;
}

void PostOperationCallback(
	__in PVOID RegistrationContext,
	__in POB_POST_OPERATION_INFORMATION OperationInformation
)
{
}

NTSTATUS d_setProcessCallbacks() {

	OB_CALLBACK_REGISTRATION obReg;
	OB_OPERATION_REGISTRATION obOper;

	//ob_operation_registeration initial.
	obOper.ObjectType = PsProcessType;
	obOper.Operations = OB_OPERATION_HANDLE_CREATE;
	obOper.PostOperation = NULL;
	obOper.PreOperation = NULL;

	obReg.Version = OB_FLT_REGISTRATION_VERSION;
	obReg.OperationRegistrationCount = 1;
	RtlInitUnicodeString(&obReg.Altitude, L"321000");
	obReg.RegistrationContext = NULL;
	obOper.PostOperation = (POB_POST_OPERATION_CALLBACK)&PostOperationCallback;
	obOper.PreOperation = (POB_PRE_OPERATION_CALLBACK)&PreOperationCallback;
	//process call back func.
	obReg.OperationRegistration = &obOper;

	//DbgPrint("obReg->Version:%d,addr:%llx\n", obReg.Version,&obReg.Version);
	if (NT_SUCCESS(ObRegisterCallbacks(&obReg, &g_pRegistrationHandle))) {
		KdPrint(("Protect Success!!!"));
	}
	else {
		KdPrint(("Protect Failure!!!"));
	}

	return STATUS_SUCCESS;
}

NTSTATUS d_clearDbgPort(ULONG64 ulpEPROCESS) {
	ULONG64* ulDbgPort = (ULONG64*)(ulpEPROCESS + 0x578);
	if (*ulDbgPort) {
		//target is debugging.
		DbgPrint("clear,\n");
		*ulDbgPort = NULL;
	}
	else {
		//target isnt debugging.
	}
	return STATUS_SUCCESS;
}

NTSTATUS d_setProtection(ULONG64 ulpEPROCESS) {
	PPS_PROTECTION _ppsProtection = (PPS_PROTECTION)(ulpEPROCESS + 0x87a);
	_ppsProtection->un.Type = 5;
	_ppsProtection->un.Audit = 0;
	_ppsProtection->un.Signer = 6;
	//https://learn.microsoft.com/en-us/windows/win32/procthread/zwqueryinformationprocess
	//http://www.hackdig.com/04/hack-324382.htm different combination.
	return STATUS_SUCCESS;
}

NTSTATUS d_getSSDTBase() {
	ULONG64 BASE = __readmsr(0xC0000082);//msr -> KiSystemCall64Shadow -> KiSystemServiceRepeat -> KeServiceDescriptorTable
	//获取SSDT基址
}

BOOLEAN d_VMEnvironment() {
	BOOLEAN returnValue = FALSE;
	//https://bbs.pediy.com/thread-266522.htm 
	{
		//Judge if the process 'vmtoolsd.exe' exists.
		ULONG64 tmp = _getRemoteEProcessByName("vmtoolsd.exe");
		if (tmp) {
			returnValue = TRUE;
			goto _rtn;
		}
	}

	{
		//redpill

	}
	
	{
		//NoPill
	}

	{
		//rookit travel kernel 'VMMemCtl'||'VMMouse'||'vmrawdsk'
	}

	{
		//Use privileged instruction to judge if machine could resolve.
		__try {
			returnValue = d_jdVM();
		} // Set ExceptionProt
		__except (1)//
		{
			returnValue = TRUE;
			goto _rtn;
		}
		
	}
	_rtn:
		return returnValue;
}