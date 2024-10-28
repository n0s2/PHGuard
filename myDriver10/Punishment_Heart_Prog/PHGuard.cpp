#include "PHGuard.h"
#include "engine.h"

WCHAR* targetProc = (WCHAR*)L"Target.exe";

#define CTL_SYS \
    CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_BUFFERED,FILE_ANY_ACCESS)
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

int main() {
	{
		ph_status = false;
		ph_init = false;
	}

	WCHAR lpPath[256], lpFileName[256];
	{
		if (!_tgetcwd(lpPath, 256)) return 0;
		wcscpy_s(lpFileName, lpPath);
		wcscat_s(lpFileName, L"\\PHGuard.sys");
		ph_unloadDriver((WCHAR*)L"PHGuard.sys");
	}

	{/*
		HANDLE DeviceHandle = CreateFile(L"\\\\.\\PHGuard",
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (DeviceHandle == INVALID_HANDLE_VALUE)
		{
			printf("[PH Console]:Init -- CreateFile failed, error code:%d\n",GetLastError());
		}
		else {
			char BufferData = NULL;
			DWORD ReturnLength = 0;
			BOOL IsOk = DeviceIoControl(DeviceHandle, CTL_SYS,
				targetProc,
				sizeof(targetProc) + 1,
				(LPVOID)BufferData,
				MAX_PATH,
				&ReturnLength,
				NULL);
			if (IsOk) {
				printf("%s\r\n", BufferData);
			}
			else {
				printf("%d\r\n", GetLastError());
			}
		}
		*/
	}

	{
		WCHAR* pProcName = (WCHAR*)L"test.exe";
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ph_chkProcAlive, pProcName, 0, NULL);
		if (!hThread) {
			printf("[PH Console]:Thread -- Create thread failed, error code:%d\n", GetLastError());
			return 0;
		}
		while (!ph_init) Sleep(1000);
		if (!ph_loadDriver((WCHAR*)L"PHGuard.sys", lpFileName)) ExitProcess(0);
	}

	//п╤ть
	while (!ph_status) Sleep(1000);
	ph_unloadDriver((WCHAR*)L"PHGuard.sys");
	return 0;
}