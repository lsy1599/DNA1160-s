// CInputNum.cpp : 实现文件
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "CyberTanATSDlg.h"
#include "CInputNum.h"

extern ATSCONFIG AtsInfo;

// CCInputNum 对话框

IMPLEMENT_DYNAMIC(CCInputNum, CDialog)

CCInputNum::CCInputNum(CWnd* pParent /*=NULL*/)
	: CDialog(CCInputNum::IDD, pParent)
{

}

CCInputNum::~CCInputNum()
{
}

void CCInputNum::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCInputNum, CDialog)
	ON_BN_CLICKED(IDOK, &CCInputNum::OnBnClicked)
	ON_BN_CLICKED(IDCANCEL, &CCInputNum::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EditOPNum, &CCInputNum::OnEnChangeEditopnum)
	ON_EN_CHANGE(IDC_EditPasswd, &CCInputNum::OnEnChangeEditpasswd)
END_MESSAGE_MAP()


void CCInputNum::OnBnClicked()
{
	((CEdit *)GetDlgItem(IDC_EditOPNum))->GetWindowText(AtsInfo.csUserName);
	((CEdit *)GetDlgItem(IDC_EditPasswd))->GetWindowText(AtsInfo.csPasswd);
	OnOK();
}
void CCInputNum::OnBnClickedCancel()
{
	AtsInfo.csUserName.Empty();
	AtsInfo.csPasswd.Empty();
	OnCancel();
}
void CCInputNum::Create(HWND hWnd)
{
	m_MyhWnd = hWnd;
}
// CCInputNum 消息处理程序

BOOL CCInputNum::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加?外的初始化
	((CEdit *)GetDlgItem(IDC_EditOPNum))->SetFocus();
	cFont.CreatePointFont(100, "Arial");
	GetDlgItem(IDC_Prompt)->SetFont(&cFont);
	GetDlgItem(IDC_EditOPNum)->SetFont(&cFont);
	GetDlgItem(IDC_StaUser)->SetFont(&cFont);
	GetDlgItem(IDC_StaPasswd)->SetFont(&cFont);
	GetDlgItem(IDC_EditPasswd)->SetFont(&cFont);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX ?性??返回 FALSE
}

BOOL CCInputNum::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加?用代?和/或?用基?
	//if(pMsg->message==WM_KEYDOWN)
	//{
	//	if(pMsg->wParam==VK_RETURN&&pMsg->hwnd==GetDlgItem(IDC_EditOPNum)->m_hWnd)
	//	{
	//		if(AtsInfo.csUserName.IsEmpty())
	//		{
	//			return TRUE;
	//		}
	//	}
	//}
	GetDlgItem(IDC_EditOPNum)->GetWindowText(AtsInfo.csUserName);
	if (AtsInfo.csUserName.IsEmpty())
	{
		GetDlgItem(IDC_EditPasswd)->SetWindowText("");
		GetDlgItem(IDC_EditOPNum)->SetFocus();
	}
	else
	{
		GetDlgItem(IDC_EditPasswd)->EnableWindow(TRUE);
	}
	if (pMsg->message == WM_KEYDOWN&&pMsg->hwnd == GetDlgItem(IDC_EditOPNum)->m_hWnd)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			GetDlgItem(IDC_EditPasswd)->EnableWindow(TRUE);
			GetDlgItem(IDC_EditPasswd)->SetFocus();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CCInputNum::OnEnChangeEditopnum()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CCInputNum::OnEnChangeEditpasswd()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
