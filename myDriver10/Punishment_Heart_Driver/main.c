#include <ntddk.h>
#include "_driver.h"
#include "_thread.h"
#include "utils.h"

PVOID g_pRegistrationHandle;
ULONG g_ulProcessId;
ULONG64 g_ulEProcess;
ULONG64 l_ulEProcess;

//====================================================
//=        Control flags                             =
//====================================================

BOOLEAN bProcessHidden = FALSE;

//====================================================
//=        Thread variables define area              =
//====================================================

HANDLE _threadHandle = NULL;
KEVENT t_antiDebug_event;
BOOLEAN t_antiDebug_ExitCode = FALSE;
PVOID p_antiDebug_ThreadObject = NULL;

//====================================================

//��ǲ����
NTSTATUS PassThroughDispatch(PDEVICE_OBJECT  DeviceObject, PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;     //LastError()
	Irp->IoStatus.Information = 0;             //ReturnLength
	IoCompleteRequest(Irp, IO_NO_INCREMENT);   //��Irp���ظ�Io������
	return STATUS_SUCCESS;
}

NTSTATUS ControlThroughDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS Status;
	ULONG_PTR Informaiton = 0;
	PVOID InputData = NULL;
	ULONG InputDataLength = 0;
	PVOID OutputData = NULL;
	ULONG OutputDataLength = 0;
	ULONG IoControlCode = 0;
	PIO_STACK_LOCATION  IoStackLocation = IoGetCurrentIrpStackLocation(Irp);  //Irp��ջ  
	IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	InputData = Irp->AssociatedIrp.SystemBuffer;
	OutputData = Irp->AssociatedIrp.SystemBuffer;
	InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	switch (IoControlCode)
	{
	case CTL_SYS:
	{
		if (InputData != NULL && InputDataLength > 0)
		{
			DbgPrint("%s\r\n", InputData);
		}
		if (OutputData != NULL && OutputDataLength >= strlen("Ring0->Ring3") + 1)
		{
			memcpy(OutputData, "Ring0->Ring3", strlen("Ring0->Ring3") + 1);
			Status = STATUS_SUCCESS;
			Informaiton = strlen("Ring0->Ring3") + 1;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;   //�ڴ治��
			Informaiton = 0;
		}
		break;
	}
	default:
		break;
	}
	Irp->IoStatus.Status = Status;             //Ring3 GetLastError();
	Irp->IoStatus.Information = Informaiton;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);  //��Irp���ظ�Io������
	return Status;                            //Ring3 DeviceIoControl()����ֵ
}

/*   Driver load & unload   */

VOID DriverUnload(PDRIVER_OBJECT driver)
{
	//Unload driver, each step should obey rule that from top to base.

	//***Resume hidden process.***
	if (bProcessHidden) {
		((PLIST_ENTRY)(l_ulEProcess + 0x448))->Flink->Blink = (PLIST_ENTRY)(l_ulEProcess + 0x448);
		((PLIST_ENTRY)(l_ulEProcess + 0x448))->Blink->Flink = (PLIST_ENTRY)(l_ulEProcess + 0x448);
	}

	//***Close device object.***
	{
		PDEVICE_OBJECT pIoDev;
		UNICODE_STRING IoSymLinkName;

		//ɾ�������豸
		pIoDev = driver->DeviceObject;
		IoDeleteDevice(pIoDev);                                       // ����IoDeleteDevice����ɾ���豸
		RtlInitUnicodeString(&IoSymLinkName, L"\\??\\PHGuard");     // ��ʼ���ַ�����symLinkName�������Ҫɾ���ķ�����������
		IoDeleteSymbolicLink(&IoSymLinkName);                         // ����IoDeleteSymbolicLinkɾ����������
	}

	//***Unregister callbacks if exists;***
	if (g_pRegistrationHandle) {
		ObUnRegisterCallbacks(g_pRegistrationHandle);
	}

	if(p_antiDebug_ThreadObject){
		//***Thread exit***
		t_antiDebug_ExitCode = TRUE;
		KeWaitForSingleObject(&t_antiDebug_event , Executive, KernelMode, FALSE, NULL);
	}
	DbgPrint("[PH INFO]driver unload success.\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
	driver->DriverUnload = DriverUnload;

	{
		//***Initialize driver params.***
		g_ulEProcess = _getRemoteEProcessByName("phmsvc.exe");
		if (!g_ulEProcess) {
			DbgPrint("[PH INFO]Process not exist:%llx\n", g_ulEProcess);
			return STATUS_SUCCESS;
		}
		d_processHideByEP(g_ulEProcess);
		l_ulEProcess = g_ulEProcess;
		bProcessHidden = TRUE;
		
		g_ulEProcess = _getRemoteEProcessByName("test.exe");
		DbgPrint("[PH INFO]EPROCESS:%llx\n", g_ulEProcess);
	}

	{
		//***Creating Io Device for Ring3***
		NTSTATUS dIoStatus;               // ������������ķ���״̬
		PDEVICE_OBJECT pIoDevObj;        // ���ڷ��ش����豸
		UNICODE_STRING IoDriverName;     // ���ڴ���豸������
		UNICODE_STRING IoSymLinkName;    // ���ڴ�ŷ�����������

		RtlInitUnicodeString(&IoDriverName, L"\\Device\\PHGuard");  // ��DrvierName���Ϊ\\Device\\My_Device
		dIoStatus = IoCreateDevice(driver, 0, &IoDriverName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pIoDevObj);
		if(!NT_SUCCESS(dIoStatus)) {
			if (dIoStatus == STATUS_OBJECT_NAME_COLLISION)
			{
				DbgPrint("[PH INFO]Device name existed.\n");
			}
			DbgPrint("[PH INFO]IoCreateDevice failed.\n");
		}
		else {
			pIoDevObj->Flags |= DO_BUFFERED_IO;
			RtlInitUnicodeString(&IoSymLinkName, L"\\??\\PHGuard");
			dIoStatus = IoCreateSymbolicLink(&IoSymLinkName, &IoDriverName);
			if (!NT_SUCCESS(dIoStatus)) // ���status������0 ��ִ��
			{
				IoDeleteDevice(pIoDevObj);
				DbgPrint("[PH INFO]IoCreateSysbolicLink failed.");
			}
			else {
				for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
				{
					driver->MajorFunction[i] = PassThroughDispatch;   //����ָ��
				}
				driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlThroughDispatch;
			}
		}
	}

	{
		//***Creating thread***
		KeInitializeEvent(&t_antiDebug_event, SynchronizationEvent, FALSE);

		NTSTATUS nsCreateThreadCode;
		nsCreateThreadCode = PsCreateSystemThread(&_threadHandle, THREAD_ALL_ACCESS, NULL, NtCurrentProcess(), NULL, (PKSTART_ROUTINE)t_antiDebug, NULL);

		if (!NT_SUCCESS(nsCreateThreadCode)) {
			DbgPrint("[PH INFO]Failed to create system thread.\n");
		}

		nsCreateThreadCode = ObReferenceObjectByHandle(_threadHandle, THREAD_ALL_ACCESS, NULL, KernelMode, &p_antiDebug_ThreadObject, NULL);

		if(NT_SUCCESS(nsCreateThreadCode)) ZwClose(_threadHandle);
		else {
			t_antiDebug_ExitCode = TRUE;
			p_antiDebug_ThreadObject = NULL;
			DbgPrint("[PH INFO]Failed to get object handle.\n");
		}
		
	}

	{
		//***Register function callbacks***
		//bypass signature-checking func 'MmVerifyCallbackFunction'
		PLDR_DATA_TABLE_ENTRY pldte;
		pldte = (PLDR_DATA_TABLE_ENTRY)driver->DriverSection;
		pldte->Flags |= 0x20;
	}

	{
		//*** ***
		if (d_VMEnvironment()) DbgPrint("[PH INFO]Running inside VM.\n");
	}

	return STATUS_SUCCESS;
}