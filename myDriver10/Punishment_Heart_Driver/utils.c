#include "utils.h"

ULONG64 FindSignature(ULONG64 Address, ULONG64 size_t, UCHAR* sig) {

};

ULONG64 _getRemoteEProcessById(ULONG ulPid) {
	ULONG ulProcessId = 0;
	ULONG64 pEPROCESS = PsGetCurrentProcess();
	if (!pEPROCESS) return 0;

	PLIST_ENTRY headerNode = (PLIST_ENTRY)(pEPROCESS + 0x448);
	if (!headerNode) return 0;

	PLIST_ENTRY nextNode = headerNode->Flink;
	if (!nextNode)return 0;

	while (nextNode != headerNode) {
		pEPROCESS = ((ULONG64)nextNode - 0x448);
		if (!pEPROCESS) return 0;

		ulProcessId = *((ULONG*)(pEPROCESS + 0x440));
		if (!ulProcessId) return 0;

		if (ulProcessId == ulPid) {
			return pEPROCESS;
		}
		nextNode = nextNode->Flink;
	}
}

ULONG64 _getRemoteEProcessByName(PUCHAR FileImageName) {
	PUCHAR pucImageName;
	ULONG64 pEPROCESS = PsGetCurrentProcess();
	if (!pEPROCESS) return 0;

	PLIST_ENTRY headerNode = (PLIST_ENTRY)(pEPROCESS + 0x448);
	if (!headerNode)return 0;

	PLIST_ENTRY nextNode = headerNode->Flink;
	if (!nextNode)return 0;

	while (nextNode != headerNode) {
		pEPROCESS = ((ULONG64)nextNode - 0x448);
		if (!pEPROCESS) continue;

		pucImageName = (UCHAR*)(pEPROCESS + 0x5a8);
		if (!pucImageName) continue;

		if (strstr(FileImageName, pucImageName) != NULL) {
			//FileImageName长度可能大于pucImgName,在EPROCESS结构中对应的成员位UCHAR[15]
			return pEPROCESS;
		}
		nextNode = nextNode->Flink;
	}
	return 0;
}

ULONG64 _getRemoteKernelAddrByName(PUCHAR DriverImageName) {

}

ULONG _getRemoteProcessId(UCHAR* FileImageName) {
	
}

VOID KeSleep(LONG lSeccond)
{
	LARGE_INTEGER my_interval;
	my_interval.QuadPart = -10000;
	my_interval.QuadPart *= lSeccond;
	KeDelayExecutionThread(KernelMode, FALSE, &my_interval);
}