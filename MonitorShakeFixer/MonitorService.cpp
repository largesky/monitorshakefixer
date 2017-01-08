#include "Monitor.h"

BOOL InstallService(const wchar_t * displayDevice)
{
	SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

	if (hSC == NULL)
	{
		return FALSE;
	}

	wchar_t szAppPath[MAX_PATH] = { 0 };

	szAppPath[0] = L'"';
	GetModuleFileName(NULL, szAppPath + 1, MAX_PATH - 1);
	wsprintf(szAppPath + wcslen(szAppPath), L" %s\"", displayDevice);
	SC_HANDLE hService = CreateService(hSC, _T("MonitorService"), _T("Largesky's monitor shake fixer"), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_CRITICAL, szAppPath, NULL, NULL, NULL, NULL, NULL);

	if (hService != NULL)
	{
		SERVICE_DESCRIPTION descrption = { 0 };
		descrption.lpDescription = L"一款由Largesky开发，能够自动切换显示器分辨率，从而达到修复显示器抖动的程序";
		ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &descrption);
	}
	CloseServiceHandle(hService);
	CloseServiceHandle(hSC);
	return hService != NULL;
}

BOOL UninstallService()
{
	SC_HANDLE hSC = NULL;
	SC_HANDLE hService = NULL;
	SERVICE_STATUS_PROCESS  status = { 0 };
	DWORD bytesNeed = 0;
	int tryCount = 0;

	__try
	{
		hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (hSC == NULL)
		{
			return FALSE;
		}

		hService = OpenService(hSC, _T("MonitorService"), SERVICE_ALL_ACCESS);
		if (hService == NULL)
		{
			return FALSE;
		}

		if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeed) == FALSE)
		{
			return FALSE;
		}

		if (status.dwCurrentState != SERVICE_CONTROL_STOP)
		{
			if (ControlService(hService, SERVICE_CONTROL_STOP, NULL) == FALSE)
			{
				return FALSE;
			}
		}

		while (status.dwCurrentState != SERVICE_CONTROL_STOP)
		{
			Sleep(500);
			tryCount++;
			if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(SERVICE_STATUS_PROCESS), &bytesNeed) == FALSE)
			{
				return FALSE;
			}
			if (tryCount >= 10)
			{
				break;
			}
		}

		if (tryCount >= 10)
		{
			return FALSE;
		}

		return DeleteService(hService);
	}
	__finally
	{
		if (hService != NULL)
		{
			CloseServiceHandle(hService);
		}
		if (hSC != NULL)
		{
			CloseServiceHandle(hSC);
		}
	}
}

BOOL HasServiceInstalled()
{
	SC_HANDLE hSC = NULL;
	SC_HANDLE hService = NULL;

	__try
	{
		hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (hSC == NULL)
		{
			return FALSE;
		}

		hService = OpenService(hSC, _T("MonitorService"), SERVICE_ALL_ACCESS);
		return hService != NULL;
	}
	__finally
	{
		if (hService != NULL)
		{
			CloseServiceHandle(hService);
		}
		if (hSC != NULL)
		{
			CloseServiceHandle(hSC);
		}
	}
}

void WINAPI MonitorServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{

}