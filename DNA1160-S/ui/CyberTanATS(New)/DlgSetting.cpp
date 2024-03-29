// DlgSetting.cpp : 实现文件
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "DlgSetting.h"
extern ATSCONFIG AtsInfo;
extern BOOL ReInitSystem(void);
extern BOOL bTestMode;
bool bEfficencyCalc = false;
bool bFirstStart = false;
extern UINT nPassCount, nFailCount;
DWORD dwBeginTime = 0;
DWORD dwTotalEfficiencyTime = 0;
DWORD dwEfficiencyBeginTime = 0;
DWORD dwEfficiencyEndTime = 0;

// CDlgSetting 对话框

IMPLEMENT_DYNAMIC(CDlgSetting, CDialog)

CDlgSetting::CDlgSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetting::IDD, pParent)
{
}

CDlgSetting::CDlgSetting(CWnd* pParent /*=NULL*/, class CATSDlg* pClass, BOOL bPriv)
	: CDialog(CDlgSetting::IDD, pParent)
{
	pCATSDlg = pClass;
	bPrivilege = bPriv;
}
CDlgSetting::~CDlgSetting()
{
}

void CDlgSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSetting, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CDlgSetting::OnBnClickedButtonHelp)
	ON_BN_CLICKED(IDC_BUTTON_ATSCFG, &CDlgSetting::OnBnClickedButtonAtscfg)
	ON_BN_CLICKED(IDC_BUTTON_SETTING, &CDlgSetting::OnBnClickedButtonSetting)
	ON_BN_CLICKED(IDC_BUTTON_ERROR, &CDlgSetting::OnBnClickedButtonError)
	ON_BN_CLICKED(IDC_BUTTON_RESTART, &CDlgSetting::OnBnClickedButtonRestart)
	ON_BN_CLICKED(IDC_BUTTON_UI, &CDlgSetting::OnBnClickedButtonUi)
	ON_BN_CLICKED(IDOK, &CDlgSetting::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgSetting::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_EFFICIENCY, &CDlgSetting::OnBnClickedButtonEfficiency)
	//	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSetting::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BTNTestMode, &CDlgSetting::OnBnClickedBtntestmode)
END_MESSAGE_MAP()
// CDlgSetting 消息处理程序

void CDlgSetting::OnBnClickedButtonHelp()
{
	// TODO: 在此添加控件通知?理程序代?
	if (AtsInfo.HelpFile.IsEmpty())
	{
		AfxMessageBox("Help FIle not exist");
	}

	ShellExecute(NULL, "open", AtsInfo.HelpFile.GetBuffer(), NULL, NULL, SW_SHOW);
	AtsInfo.HelpFile.ReleaseBuffer();
}

void CDlgSetting::OnBnClickedButtonAtscfg()
{
	// TODO: 在此添加控件通知?理程序代?
	WinExec("notepad ATSCfg.ini", SW_SHOW);
}

void CDlgSetting::OnBnClickedButtonSetting()
{
	// TODO: 在此添加控件通知?理程序代?
	char chBuffer[400] = { 0 };
	strcat(chBuffer, "notepad ");
	strcat(chBuffer, AtsInfo.TestFunc.STATION.m_DllName);
	strcat(chBuffer, ".ini");
	WinExec(chBuffer, SW_SHOW);
}

BOOL CDlgSetting::IsFileInUse(CString FileName)
{
	BOOL Result;
	HANDLE Handle = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == Handle)
	{
		return true;
	}
	else
	{
		CloseHandle(Handle);
		return false;
	}

}
void CDlgSetting::OnBnClickedButtonError()
{
	// TODO: 在此添加控件通知?理程序代?
	if (AtsInfo.ErrorCodeFile.IsEmpty())
	{
		AfxMessageBox("Errcode File not exist!");
	}

	ShellExecute(NULL, "open", AtsInfo.ErrorCodeFile.GetBuffer(), NULL, NULL, SW_SHOW);
	AtsInfo.HelpFile.ReleaseBuffer();
}

void CDlgSetting::OnBnClickedButtonRestart()
{
	// TODO: 在此添加控件通知?理程序代?
	pCATSDlg->SetFocus();
	pCATSDlg->ReInitSystem();
}

BOOL CDlgSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加?外的初始化
	if (bTestMode)
	{
		GetDlgItem(IDC_BTNTestMode)->SetWindowText("Debug Mode");
	}
	else
	{
		GetDlgItem(IDC_BTNTestMode)->SetWindowText("Test Mode");
	}
	if (!bPrivilege)
	{
		GetDlgItem(IDC_BUTTON_ATSCFG)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_SETTING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_RESTART)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNTestMode)->ShowWindow(SW_HIDE);
	}
	if (bEfficencyCalc)
	{
		GetDlgItem(IDC_BUTTON_EFFICIENCY)->SetWindowTextA("Effieiency Statisticing..");
	}
	else
	{
		GetDlgItem(IDC_BUTTON_EFFICIENCY)->SetWindowTextA("Begin Effieiency Statistic");
	}
	GetDlgItem(IDC_BUTTON_UI)->SetFocus();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX ?性??返回 FALSE
}

void CDlgSetting::OnBnClickedButtonUi()
{
	// TODO: 在此添加控件通知?理程序代?

	char chBuffer[400] = { 0 };
	strcat(chBuffer, "notepad ");
	strcat(chBuffer, AtsInfo.Log.csLogFilePathUI.GetBuffer());
	AtsInfo.Log.csLogFilePathUI.ReleaseBuffer();
	WinExec(chBuffer, SW_SHOW);
}

void CDlgSetting::OnBnClickedOk()
{
	// TODO: 在此添加控件通知?理程序代?
//	::PostMessage(pCATSDlg->m_hWnd,MSG_DELETE_CLASS,0,LPARAM(this));
	::PostMessage(pCATSDlg->m_hWnd, MSG_DELETE_CLASS, 0, LPARAM(this));
	OnOK();
}

void CDlgSetting::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知?理程序代?
	::PostMessage(pCATSDlg->m_hWnd, MSG_DELETE_CLASS, 0, LPARAM(this));
	OnCancel();
}



void CDlgSetting::OnClose()
{
	// TODO: 在此添加消息?理程序代?和/或?用默?值
	::PostMessage(pCATSDlg->m_hWnd, MSG_DELETE_CLASS, 0, LPARAM(this));
	CDialog::OnClose();
}

void CDlgSetting::OnBnClickedButtonEfficiency()
{
	// TODO: 在此添加控件通知?理程序代?
	char Buffer[80] = { 0 };
	GetDlgItem(IDC_BUTTON_EFFICIENCY)->GetWindowTextA(Buffer, 79);
	if (strcmp(Buffer, "效能統計中...") == 0)
	{
		GetDlgItem(IDC_BUTTON_EFFICIENCY)->SetWindowTextA("Begin Effieiency Statistic");
		bEfficencyCalc = false;
		bFirstStart = false;
	}
	else
	{
		GetDlgItem(IDC_BUTTON_EFFICIENCY)->SetWindowTextA("Effieiency Statisticing..");
		bEfficencyCalc = true;
		bFirstStart = true;
		nPassCount = 0;
		nFailCount = 0;
		dwEfficiencyBeginTime = 0;
		DWORD dwBeginTime = 0;
		dwTotalEfficiencyTime = 0;
		dwEfficiencyBeginTime = 0;
		dwEfficiencyEndTime = 0;
	}
}

//void CDlgSetting::OnBnClickedButton1()
//{
//	// TODO: 在此添加控件通知?理程序代?
//	bTestMode=!bTestMode;
//	if(bTestMode)
//	{
//		GetDlgItem(IDC_
//	}
//}

void CDlgSetting::OnBnClickedBtntestmode()
{
	// TODO: 在此添加控件通知?理程序代?
	bTestMode = !bTestMode;
	if (bTestMode)
	{
		GetDlgItem(IDC_BTNTestMode)->SetWindowText("Debug Mode");
	}
	else
	{
		GetDlgItem(IDC_BTNTestMode)->SetWindowText("Test Mode");
	}
	pCATSDlg->SetFocus();
	pCATSDlg->ReDrawWindow();
}
