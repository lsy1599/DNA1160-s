// SfisOffDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "SfisOffDlg.h"
#include "CyberTanATSDlg.h"

extern ATSCONFIG AtsInfo;
// SfisOffDlg dialog

IMPLEMENT_DYNAMIC(SfisOffDlg, CDialog)

SfisOffDlg::SfisOffDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SfisOffDlg::IDD, pParent)
{

}

SfisOffDlg::~SfisOffDlg()
{
}

void SfisOffDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SfisOffDlg, CDialog)
	ON_BN_CLICKED(IDOK, &SfisOffDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_SFISOFF, &SfisOffDlg::OnEnChangeEditSfisoff)
END_MESSAGE_MAP()


// SfisOffDlg message handlers

void SfisOffDlg::OnBnClickedOk()
{
	CString csPassword;

	CEdit *GetPassword = (CEdit*)GetDlgItem(IDC_EDIT_SFISOFF);
	GetPassword->GetWindowTextA(AtsInfo.csSFISPassword);
	OnOK();
}


void SfisOffDlg::OnEnChangeEditSfisoff()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
