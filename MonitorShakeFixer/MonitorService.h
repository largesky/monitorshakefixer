#include "stdafx.h"

BOOL InstallService(const wchar_t * displayDevice);

BOOL UninstallService();

BOOL HasServiceInstalled();

void WINAPI MonitorServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);