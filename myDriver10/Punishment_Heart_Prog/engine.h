#pragma once
#include "windows.h"
#include "stdio.h"
#include "tlhelp32.h"

bool ph_loadDriver(WCHAR* pDriverName, WCHAR* pDriverNamePath);
bool ph_unloadDriver(WCHAR* pDriverName);
bool ph_chkProcAlive(PWCHAR pProcName);


extern bool ph_status;
extern bool ph_init;