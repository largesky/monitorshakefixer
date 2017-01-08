#include "Monitor.h"

wchar_t * MONITOR_KEY_NAME = _T("Largesky_Monitor");

BOOL GetDisplayDeviceFromParam(std::vector<Monitor *> * monitors, const wchar_t *param)
{
	if (param == NULL)
	{
		return FALSE;
	}
	const wchar_t *deviceName = wcsstr(param, L"\\\\.\\DISPLAY");
	if (deviceName == NULL)
	{
		return FALSE;
	}

	Monitor * monitor = (Monitor *)malloc(sizeof(Monitor));
	if (monitor == NULL)
	{
		return FALSE;
	}

	wcscpy_s(monitor->Description, 256, deviceName);
	wcscpy_s(monitor->Device.DeviceName, 32, deviceName);
	monitors->push_back(monitor);

	return TRUE;
}

BOOL GetDisplayDeviceFromSystem(std::vector<Monitor *> * monitors)
{
	if (monitors == NULL)
	{
		return FALSE;
	}

	DWORD displayNum = 0;
	DISPLAY_DEVICE device = { sizeof(DISPLAY_DEVICE) };

	//ö��ϵͳ�е���ʾ�豸
	while (EnumDisplayDevices(NULL, displayNum++, &device, 0))
	{
		//����ϵͳ��������Կ��豸
		if (wcslen(device.DeviceID) < 1)
			continue;

		if (_wcsnicmp(L"\\\\.\\DISPLAY", device.DeviceName, wcslen(L"\\\\.\\DISPLAY")) != 0)
			continue;

		Monitor *m = (Monitor *)malloc(sizeof(Monitor));
		m->Device = device;
		wsprintf(m->Description, L"%s %s", device.DeviceString, device.DeviceName);
		monitors->push_back(m);
	}

	return TRUE;
}

BOOL HasDisplayDeviceSaved()
{
	HKEY hRunKey = 0;
	wchar_t buf[1024] = { 0 };
	DWORD dataType = 0;
	DWORD dataSizeOfCB = 1024 * 2;

	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &hRunKey);
	if (ret != ERROR_SUCCESS){ return FALSE; }

	ret = RegQueryValueEx(hRunKey, MONITOR_KEY_NAME, NULL, &dataType, (BYTE *)buf, &dataSizeOfCB);
	RegCloseKey(hRunKey);

	return ret == ERROR_SUCCESS;
}

BOOL SaveDisplayDevice(Monitor *monitor)
{
	HKEY hRunKey = 0;
	wchar_t buf[1024 * 2] = { 0 };
	wchar_t exePath[1024] = { 0 };
	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hRunKey);

	if (ret != ERROR_SUCCESS)
	{
		return FALSE;
	}

	GetModuleFileName(NULL, exePath, 1024);
	wsprintf(buf, L"\"%s\" %s", exePath, monitor->Device.DeviceName);

	ret = RegSetValueEx(hRunKey, MONITOR_KEY_NAME, 0, REG_SZ, (BYTE*)buf, (DWORD)(wcslen(buf) + 1) * 2);
	RegCloseKey(hRunKey);

	return ret == ERROR_SUCCESS;
}

BOOL DeleteDisplayDevice()
{
	HKEY hRunKey = 0;
	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hRunKey);

	if (ret != ERROR_SUCCESS)
	{
		return FALSE;
	}

	ret = RegDeleteValue(hRunKey, MONITOR_KEY_NAME);
	RegCloseKey(hRunKey);

	return ret == ERROR_SUCCESS;
}

BOOL FreeDisplayDevice(std::vector<Monitor *> * monitors)
{
	if (monitors == NULL)
	{
		return FALSE;
	}

	for (size_t i = 0; i < monitors->size(); i++)
	{
		free(monitors->at(i));
	}
	monitors->clear();

	return TRUE;
}

void OnMessage(wchar_t errorMsg[1024], const wchar_t *pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);
	vswprintf_s(errorMsg, 1024, pszFormat, argList);
	va_end(argList);
}

BOOL MonitorShakeFix(Monitor *monitor, wchar_t errorMsg[1024])
{
	DEVMODE currentMode;
	DEVMODE defaultMode;
	DEVMODE targetMode;

	ZeroMemory(&currentMode, sizeof(DEVMODE));
	currentMode.dmSize = sizeof(DEVMODE);
	ZeroMemory(&defaultMode, sizeof(DEVMODE));
	defaultMode.dmSize = sizeof(DEVMODE);
	ZeroMemory(&targetMode, sizeof(DEVMODE));
	targetMode.dmSize = sizeof(DEVMODE);

	//��ȡ��ǰ����ʾģʽ
	if (EnumDisplaySettingsEx(monitor->Device.DeviceName, ENUM_CURRENT_SETTINGS, &currentMode, 0) != TRUE)
	{
		OnMessage(errorMsg, L"��ȡ�豸��%s ��ǰ��ʾģʽ����\n������룺%d", monitor->Description, GetLastError());
		return FALSE;
	}

	//��ȡ���õ���ʾģ��
	DWORD displayMode = 0;
	while (EnumDisplaySettingsEx(monitor->Device.DeviceName, displayMode, &defaultMode, 0))
	{
		if (defaultMode.dmPelsHeight != currentMode.dmPelsHeight || defaultMode.dmPelsWidth != currentMode.dmPelsWidth)
		{
			//��windows 8���棬ֻ������32bit perģʽ
			if (defaultMode.dmBitsPerPel == 32)
			{
				targetMode = defaultMode;
			}
		}
		displayMode++;
	}

	if (targetMode.dmBitsPerPel==0 ||( targetMode.dmPelsWidth == currentMode.dmPelsWidth && targetMode.dmPelsHeight == currentMode.dmPelsHeight))
	{
		OnMessage(errorMsg, L"��ȡ�豸��%s ������ʾģʽ����\n������룺%d", monitor->Device.DeviceName, GetLastError());
		return FALSE;
	}

	//����п��õ���ʾģʽ�������л���ĳһ�����õ���ʾģ�ͣ����л�������
	Sleep(1 * 1000);
	targetMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY | DM_POSITION;
	LONG ret = ChangeDisplaySettingsEx(monitor->Device.DeviceName, &targetMode, NULL, CDS_GLOBAL | CDS_UPDATEREGISTRY, NULL);
	if (ret != DISP_CHANGE_SUCCESSFUL)
	{
		OnMessage(errorMsg, L"�л��豸��%s  ��ʾģʽ����\n������룺%d", monitor->Device.DeviceName, GetLastError());
		return FALSE;
	}

	Sleep(2 * 1000);

	currentMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY | DM_POSITION;
	ret = ChangeDisplaySettingsEx(monitor->Device.DeviceName, &currentMode, NULL, CDS_GLOBAL | CDS_UPDATEREGISTRY, NULL);
	if (ret != DISP_CHANGE_SUCCESSFUL)
	{
		OnMessage(errorMsg, L"��ԭ�豸��%s ��ʾģʽ����\n������룺%d", monitor->Description, GetLastError());
	}
	else
	{
		OnMessage(errorMsg, L"��ԭ�豸��%s ��ʾģʽ�ɹ�\n������룺%d", monitor->Description, GetLastError());
	}

	return ret == DISP_CHANGE_SUCCESSFUL;
}

