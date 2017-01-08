// MonitorShakeFixer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MonitorShakeFixer.h"
#include <process.h>
#include <vector>
#include <WindowsX.h>
#include "Monitor.h"

// 全局变量:

//程序是否是从系统中启动
BOOL g_bAuto = FALSE;

//系统中的显示设备列表
std::vector<Monitor *> g_Monitors;

/*
*主程序窗体的处理方法
*/
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	//参数处理
	if (wcslen(lpCmdLine) > 0)
	{
		g_bAuto = TRUE;
		if (GetDisplayDeviceFromParam(&g_Monitors, lpCmdLine) == FALSE)
		{
			MessageBox(NULL, L"解析程序参数失败", L"错误", MB_OK);
			return -1;
		}
	}
	else
	{
		if (GetDisplayDeviceFromSystem(&g_Monitors) == FALSE || g_Monitors.size() < 1)
		{
			MessageBox(NULL, L"系统中未检测到显示器，程序无法执行", L"错误", MB_OK | MB_ICONERROR);
			return -2;
		}
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOGMAIN), NULL, MainDlgProc);
	FreeDisplayDevice(&g_Monitors);

	return 0;
}

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_BTNSAVE, HasDisplayDeviceSaved() ? L"删除启动项" : L"写入启动项");
		//添加数据
		for (size_t i = 0; i < g_Monitors.size(); i++)
		{
			ComboBox_AddString(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), g_Monitors[i]->Description);
			ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), i, g_Monitors[i]);
		}
		ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), 0);
		//如果是自动，则自动执行修复
		if (g_bAuto)
			PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTNFIX, 0), MAKELPARAM(0, 0));
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_BTNSAVE)
		{
			int index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES));
			if (index == CB_ERR)
			{
				MessageBox(hDlg, L"请选择相应的显示设备", L"提示", MB_OK);
				return NULL;
			}
			Monitor * monitor = (Monitor *)ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), index);
			if (monitor == NULL)
			{
				return (INT_PTR)TRUE;
			}
			BOOL ret = HasDisplayDeviceSaved() ? DeleteDisplayDevice() : SaveDisplayDevice(monitor);
			SetDlgItemText(hDlg, IDC_BTNSAVE, HasDisplayDeviceSaved() ? L"删除启动项" : L"写入启动项");
			MessageBox(hDlg, ret ? L"成功" : L"操作失败，请以管理员权限运行程序", L"提示", MB_OK);
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_BTNFIX)
		{
			int index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES));
			wchar_t msgBuf[1024] = { 0 };
			if (index == CB_ERR)
			{
				MessageBox(hDlg, L"请选择相应的显示设备", L"提示", MB_OK);
			}
			else
			{
				Monitor * monitor = (Monitor *)ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), index);
				if (monitor != NULL)
				{
					EnableWindow(GetDlgItem(hDlg, IDC_BTNFIX), FALSE);
					BOOL ret = MonitorShakeFix(monitor, msgBuf);
					EnableWindow(GetDlgItem(hDlg, IDC_BTNFIX), TRUE);
					if (g_bAuto&&ret)
					{
						EndDialog(hDlg, IDCANCEL);
					}
					else
					{
						MessageBox(hDlg, ret ? L"完成操作" : msgBuf, L"提示", MB_OK);
					}
				}
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}