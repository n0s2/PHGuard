#include "_thread.h"

NTSTATUS t_antiDebug() {
	while (1) {
		if (t_antiDebug_ExitCode) {
			KeSetEvent(&t_antiDebug_event, 0, FALSE);
			PsTerminateSystemThread(STATUS_SUCCESS);
		}

		d_clearDbgPort(g_ulEProcess);
		KeSleep(100);
	}
	return STATUS_SUCCESS;
}

NTSTATUS t_chkDbgEvr() {
	{

	}
}

NTSTATUS t_chkProc() {
	//ȷ�������Ƿ���ڣ�������������˳��߳�
}