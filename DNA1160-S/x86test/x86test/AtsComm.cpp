#include "stdafx.h"
#include "AtsComm\AtsComm.h"

HWND m_UINotifyWnd;
DEVICE_HANDLE *DevHandle;
SFIS_PM *sfisData;
CString m_errCode;
CString m_FW;

extern CString m_errCode;
extern CString m_FW;

void ShowUIGolden()
{
	::SendMessage(m_UINotifyWnd, MSG_SHOW_GOLDEN, 0,0);
}

void SetDutItemStatusPass(CString csItemTitle)
{
	::SendMessage(m_UINotifyWnd, MSG_SET_ITEM_STATUS, (WPARAM)&csItemTitle, (LPARAM)STATUS_ITEM_PASS);	
}

void SetDutItemStatusTesting(CString csItemTitle)
{
	::SendMessage(m_UINotifyWnd, MSG_SET_ITEM_STATUS, (WPARAM)&csItemTitle, (LPARAM)STATUS_ITEM_TESTING);
}

void SetDutItemStatusFail(CString csItemTitle)
{
	::SendMessage(m_UINotifyWnd, MSG_SET_ITEM_STATUS, (WPARAM)&csItemTitle, (LPARAM)STATUS_ITEM_FAIL);
}

void SetDutItemStatusNormal(CString csItemTitle)
{
	::SendMessage(m_UINotifyWnd, MSG_SET_ITEM_STATUS, (WPARAM)&csItemTitle, (LPARAM)STATUS_ITEM_NORMAL);
}

void SetGoldenItemStatusTesting(CString csItemTitle)
{
	::SendMessage(m_UINotifyWnd, MSG_SET_GOLDEN_ITEM, (WPARAM)&csItemTitle, 0);
}

void ShowUIMessage(CString msg)
{
	::SendMessage(m_UINotifyWnd, MSG_SHOW_MESSAGE, (WPARAM)&msg, 0);
}

void ShowModel(CString msg)//在UI上显示机种
{
	::SendMessage(m_UINotifyWnd, MSG_SHOW_MODEL, (WPARAM)&msg, 0);
}

void ShowMAC_BEGIN(CString msg)//在UI上显示 MAC_BEGIN
{
	::SendMessage(m_UINotifyWnd, MSG_SHOW_MAC_BEGIN, (WPARAM)&msg, 0);
}

void ShowMAC_END(CString msg)//在UI上显示机种 MAC_END
{
	::SendMessage(m_UINotifyWnd, MSG_SHOW_MAC_END, (WPARAM)&msg, 0);
}

void ShowUIActMessage(CString msg, COLORREF color)
{
	::SendMessage(m_UINotifyWnd, MSG_SHOW_ACT_MESSAGE, (WPARAM)&msg, (LPARAM)&color);
}

void CreateLogMessage(CString msg)
{
	::SendMessage(m_UINotifyWnd, MSG_SEND_LOG_MESSAGE, 0, (LPARAM)&msg);
}

void SendLogMessage(CString msg)
{
	::SendMessage(m_UINotifyWnd, MSG_SEND_LOG_MESSAGE, 2, (LPARAM)&msg);
}

void CreateLogMessageEnd()
{
	::PostMessage(m_UINotifyWnd, MSG_SEND_LOG_MESSAGE, 1, 0);
}

bool WINAPI PreInit(HWND notifyWnd, DEVICE_HANDLE *DevHndl)
{
	m_UINotifyWnd = notifyWnd;
	DevHandle = DevHndl;
	if(TestPreInit())	
		return true;
	return false;
}




bool WINAPI InitDut(SFIS_PM *pSFIS)
{
	
	m_errCode = "";
	sfisData = pSFIS;
	return DutInit();
}
	
bool WINAPI StartTest(void)
{
	
	return StartTestItem();
}

bool WINAPI TestEnd(void)
{
	DutTestEnd();
	sprintf_s(sfisData->czEC,"%s", m_errCode);
	//strcpy(sfisData->czFW, m_FW);
	sprintf_s(sfisData->czFW,"%s",m_FW);
	::SendMessage(m_UINotifyWnd, MSG_TEST_END, (WPARAM)sfisData, (LPARAM)3);
	return true;
}

void WINAPI Terminal(void)
{
	TestTerminal();
}