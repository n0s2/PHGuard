#pragma once
#include <ntddk.h>

ULONG64 FindSignature(ULONG64 Address, ULONG64 size_t, UCHAR* sig);

ULONG64 _getRemoteEProcessById(ULONG ulPid);
ULONG64 _getRemoteEProcessByName(PUCHAR FileImageName);
ULONG64 _getRemoteKernelAddrByName(PUCHAR DriverImageName);
ULONG _getRemoteProcessId(UCHAR* FileImageName);
VOID KeSleep(LONG lSeccond);