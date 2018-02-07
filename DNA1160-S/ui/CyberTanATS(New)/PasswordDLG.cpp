// PasswordDLG.cpp : 实现文件
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "PasswordDLG.h"
#include "CyberTanATSDlg.h"
extern ATSCONFIG AtsInfo;
int nCancel;

// CPasswordDLG 对话框

IMPLEMENT_DYNAMIC(CPasswordDLG, CDialog)

CPasswordDLG::CPasswordDLG(void)
{
 	m_bExit = false;
}
CPasswordDLG::CPasswordDLG(CWnd* pParent /*=NULL*/,CString password)
	: CDialog(CPasswordDLG::IDD, pParent)
	, m_csPassword(_T(""))
{
 	m_bExit = false;
	nCancel=false;
	m_csPrePassword = password;
}
CPasswordDLG::~CPasswordDLG()
{
}

void CPasswordDLG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RETEST_PASSWORD, m_csPassword);
}


BEGIN_MESSAGE_MAP(CPasswordDLG, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CPasswordDLG::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTNCancel, &CPasswordDLG::OnBnClickedBtncancel)
END_MESSAGE_MAP()


// CPasswordDLG 消息处理程序



void CPasswordDLG::OnClose()
{
	// TODO: 在此添加消息?理程序代?和/或?用默?值
    CDialog::OnClose();
		
}

bool CPasswordDLG::IsOK(void)
{
	    m_csPassword.Trim();
		AtsInfo.m_RetestMSG.m_csPassword.Trim();
		if(!m_csPrePassword.IsEmpty())
			{
			if(m_csPassword.Find(m_csPrePassword) >= 0)
			//if(m_csPassword.Compare(m_csPrePassword) == 0)
				{
				return true;
				}
			else
				{
				return false;
				}
			}
		else
			{
			if(m_csPassword.Compare(AtsInfo.m_RetestMSG.m_csPassword) == 0)
				{
				return true;
				}
			else if(m_csPassword.Compare("888888") == 0)
				{
				AtsInfo.m_RetestMSG.m_csAlarmMSG="";
				return true;
				}
			else
				{
				return false;
				}
			}
		
}

void CPasswordDLG::OnBnClickedOk()
{
	// TODO: 在此添加控件通知?理程序代?
	UpdateData(true);
	OnOK();
}

BOOL CPasswordDLG::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加?外的初始化
	if(!m_csPrePassword.IsEmpty())
		{
		GetDlgItem(IDC_DISPLAY_PASSWORD)->SetWindowText("General User no need to Input Password:");
		GetDlgItem(IDC_DISPLAY_PROMPT)->SetWindowText("Super User Please Input Password:");
		}
	
	GetDlgItem(IDC_DISPLAY_PASSWORD)->SetFocus();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX ?性??返回 FALSE
}

void CPasswordDLG::OnBnClickedBtncancel()
{
	// TODO: 在此添加控件通知?理程序代?
	nCancel=TRUE;
	OnCancel();
	PostMessage(WM_CLOSE);
}
