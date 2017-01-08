// MonitorShakeFixer.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "MonitorShakeFixer.h"
#include <process.h>
#include <vector>
#include <WindowsX.h>
#include "Monitor.h"

// ȫ�ֱ���:

//�����Ƿ��Ǵ�ϵͳ������
BOOL g_bAuto = FALSE;

//ϵͳ�е���ʾ�豸�б�
std::vector<Monitor *> g_Monitors;

/*
*��������Ĵ�����
*/
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	//��������
	if (wcslen(lpCmdLine) > 0)
	{
		g_bAuto = TRUE;
		if (GetDisplayDeviceFromParam(&g_Monitors, lpCmdLine) == FALSE)
		{
			MessageBox(NULL, L"�����������ʧ��", L"����", MB_OK);
			return -1;
		}
	}
	else
	{
		if (GetDisplayDeviceFromSystem(&g_Monitors) == FALSE || g_Monitors.size() < 1)
		{
			MessageBox(NULL, L"ϵͳ��δ��⵽��ʾ���������޷�ִ��", L"����", MB_OK | MB_ICONERROR);
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
		SetDlgItemText(hDlg, IDC_BTNSAVE, HasDisplayDeviceSaved() ? L"ɾ��������" : L"д��������");
		//�������
		for (size_t i = 0; i < g_Monitors.size(); i++)
		{
			ComboBox_AddString(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), g_Monitors[i]->Description);
			ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), i, g_Monitors[i]);
		}
		ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), 0);
		//������Զ������Զ�ִ���޸�
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
				MessageBox(hDlg, L"��ѡ����Ӧ����ʾ�豸", L"��ʾ", MB_OK);
				return NULL;
			}
			Monitor * monitor = (Monitor *)ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES), index);
			if (monitor == NULL)
			{
				return (INT_PTR)TRUE;
			}
			BOOL ret = HasDisplayDeviceSaved() ? DeleteDisplayDevice() : SaveDisplayDevice(monitor);
			SetDlgItemText(hDlg, IDC_BTNSAVE, HasDisplayDeviceSaved() ? L"ɾ��������" : L"д��������");
			MessageBox(hDlg, ret ? L"�ɹ�" : L"����ʧ�ܣ����Թ���ԱȨ�����г���", L"��ʾ", MB_OK);
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_BTNFIX)
		{
			int index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CBBDISPLAYDEVICES));
			wchar_t msgBuf[1024] = { 0 };
			if (index == CB_ERR)
			{
				MessageBox(hDlg, L"��ѡ����Ӧ����ʾ�豸", L"��ʾ", MB_OK);
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
						MessageBox(hDlg, ret ? L"��ɲ���" : msgBuf, L"��ʾ", MB_OK);
					}
				}
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}