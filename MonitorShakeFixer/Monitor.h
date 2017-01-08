#include "stdafx.h"
#include <process.h>
#include <vector>
#include <WindowsX.h>

typedef struct _Monitor
{
	DISPLAY_DEVICEW Device;
	wchar_t Description[256];
}Monitor;

BOOL GetDisplayDeviceFromParam(std::vector<Monitor *> * monitors, const wchar_t *param);

BOOL GetDisplayDeviceFromSystem(std::vector<Monitor *> * monitors);

BOOL FreeDisplayDevice(std::vector<Monitor *> * monitors);

BOOL HasDisplayDeviceSaved();

BOOL SaveDisplayDevice(Monitor *monitor);

BOOL DeleteDisplayDevice();

BOOL MonitorShakeFix(Monitor * monitor,wchar_t errorMsg[1024]);
