#include "engine.h"

bool ph_status;
bool ph_init;

SC_HANDLE _gSCMng;
SC_HANDLE _gSCService;

bool ph_loadDriver(WCHAR* pDriverName, WCHAR* pDriverNamePath) {
	//正常加载驱动
	_gSCMng = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (!_gSCMng) {
		printf("[PH Console]:Load -- Opening manager failed, error code:%d\n", GetLastError());
		return false;
	}

	_gSCService = CreateService(
		_gSCMng,
		pDriverName,
		pDriverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		pDriverNamePath,
		NULL,
		NULL,
		NULL,
		NULL,//username
		NULL);
	if (!_gSCService) {
		printf("[PH Console]:Load -- Creating service failed, error code:%d\n", GetLastError());
		CloseServiceHandle(_gSCMng);
		return false;
	}

	SC_HANDLE tSCService = OpenService(_gSCMng, pDriverName, SERVICE_ALL_ACCESS);
	if (!tSCService) {
		CloseServiceHandle(_gSCMng);
		CloseServiceHandle(_gSCService);
		printf("[PH Console]:Load -- Opening service failed, error code:%d\n", GetLastError());
		return false;
	}
	_gSCService = tSCService;

	if (!StartService(_gSCService, NULL, NULL)) {
		CloseServiceHandle(_gSCMng);
		CloseServiceHandle(_gSCService);
		printf("<service path:%ls,%ls>\n", pDriverName, pDriverNamePath);
		printf("[PH Console]:Load -- Starting service failed, error code:%d\n", GetLastError());
		return false;
	}

	return true;
}

bool ph_unloadDriver(WCHAR* pDriverName) {
	SERVICE_STATUS svcsta = {0};

	_gSCMng = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (!_gSCMng) {
		if(ph_status) printf("[PH Console]:Unload -- Opening manager failed, error code:%d\n", GetLastError());
		return false;
	}

	_gSCService = OpenService(_gSCMng, pDriverName, SERVICE_ALL_ACCESS);
	if (!_gSCService) {
		if(ph_status) printf("[PH Console]:Unload -- Opening service failed, error code:%d\n", GetLastError());
		return false;
	}

	if (!ControlService(_gSCService, SERVICE_CONTROL_STOP, &svcsta)) {
		if (ph_status) printf("[PH Console]:Unload -- Stopping service failed, error code:%d\n", GetLastError());
		//可能会出现暂停失败的情况，暂停失败直接删除驱动。
	}

	if (!DeleteService(_gSCService)) {
		if (ph_status) printf("[PH Console]:Unload -- Deleting service failed, error code:%d\n", GetLastError());
		return false;
	}

	if (!CloseServiceHandle(_gSCService) ||
		!CloseServiceHandle(_gSCMng)) {
		if (ph_status) printf("[PH Console]:Unload -- Closing handle failed, error code:%d\n", GetLastError());
		return false;
	}
	return true;
}

bool ph_chkProcAlive(PWCHAR pProcName) {
	ULONG ulProcessId = 0;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process = { sizeof(PROCESSENTRY32) };
	while (Process32Next(hProcessSnap, &process)) {
		if (wcsstr(process.szExeFile, pProcName)) {
			ulProcessId = process.th32ProcessID;
			break;
		}
	}
	if (!ulProcessId) {
		printf("[PH Console]:Thread -- Get process ID failed, process does not exist or access denied;\n");
		ExitProcess(0);
		return false;
	}
	CloseHandle(hProcessSnap);
	HANDLE hTarget = OpenProcess(SYNCHRONIZE, false, ulProcessId);
	ph_init = true;
	WaitForSingleObject(hTarget, INFINITE);
	ph_status = true;
	CloseHandle(hTarget);
}

