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
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}
