// CPromptDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "x86test.h"
#include "CPromptDlg.h"


// CCPromptDlg 对话框

IMPLEMENT_DYNAMIC(CCPromptDlg, CDialog)

CCPromptDlg::CCPromptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCPromptDlg::IDD, pParent)
	, m_StaExpire(0)
	, m_Title(_T(""))
	, m_Pic(_T(""))
	, m_Prompt(_T(""))
	, nButton(0)
{

}
CCPromptDlg::CCPromptDlg(CString csTitle, CString csPrompt, CString csPic, int nTime, int nBtn,CWnd *pParent)
	: CDialog(CCPromptDlg::IDD,pParent)
	,m_Title(csTitle)
	,m_Prompt(csPrompt)
	,m_Pic(csPic)
	,m_StaExpire(nTime)
	,nButton(nBtn)
{
}
CCPromptDlg::~CCPromptDlg()
{
}

void CCPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_StaPrompt, m_StaPrompt);
	DDX_Control(pDX, IDC_PicCtrl, m_PicCtrl);
	DDX_Text(pDX, IDC_StaExpire, m_StaExpire);
}


BEGIN_MESSAGE_MAP(CCPromptDlg, CDialog)
	ON_BN_CLICKED(IDC_BTNYES, &CCPromptDlg::OnBnClickedBtnyes)
	ON_BN_CLICKED(IDC_BTNCONF, &CCPromptDlg::OnBnClickedBtnconf)
	ON_BN_CLICKED(IDC_BTNNO, &CCPromptDlg::OnBnClickedBtnno)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CCPromptDlg 消息处理程序

void CCPromptDlg::OnBnClickedBtnyes()
{
	// TODO: 在此添加控件通知?理程序代?
	nRes=BTN_YES;
	PostMessage(WM_CLOSE);
}

void CCPromptDlg::OnBnClickedBtnconf()
{
	// TODO: 在此添加控件通知?理程序代?
	nRes=BTN_YES;
	PostMessage(WM_CLOSE);
}

void CCPromptDlg::OnBnClickedBtnno()
{
	// TODO: 在此添加控件通知?理程序代?
	nRes=BTN_NO;
	PostMessage(WM_CLOSE);
}
int CCPromptDlg::DisplayDlg()
{
	DoModal();
	return nRes;
}
void CCPromptDlg::ShowDlgWindow(int nWindow)
{
	/*switch(nWindow)
	{
	case BUTTON_YESNO:
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		 GetDlgItem(IDC_BTNYES)->SetWindowText("YES");
		GetDlgItem(IDC_BTNYES)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_SHOW);
		 GetDlgItem(IDC_BTNNO)->SetWindowText("NO");
		GetDlgItem(IDC_BTNNO)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_SHOW);
		break;
	case BUTTON_YES:
		GetDlgItem(IDC_PicCtrl)->EnableWindow(FALSE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
		break;
	case BUTTON_PIC:
		cImage.Load(m_Pic);
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(TRUE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpire)->EnableWindow(TRUE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
		
		SetTimer(1,1000,NULL);
		break;
	case BUTTON_OLPIC:
		cImage.Load(m_Pic);
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_BTNYES)->SetWindowText("YES");
		GetDlgItem(IDC_BTNNO)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_BTNNO)->SetWindowText("NO");
		break;
	case BUTTON_YESPIC:
		cImage.Load(m_Pic);
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
        break;
    case BUTTON_COUNT:
        GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
        GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_StaExpPrompt)->EnableWindow(TRUE);
        GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_StaExpire)->EnableWindow(TRUE);
        GetDlgItem(IDC_StaExpire)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);

        SetTimer(1, 1000, NULL);
        break;
	}*/

	switch (nWindow)
	{
	case BUTTON_YESNO:
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNYES)->SetWindowText("YES");
		GetDlgItem(IDC_BTNNO)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNNO)->SetWindowText("NO");
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);
		break;
	case BUTTON_YES:
		GetDlgItem(IDC_PicCtrl)->EnableWindow(FALSE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNCONF)->SetWindowText("OK");
		break;
	case BUTTON_PIC:
		cImage.Load(m_Pic);
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(TRUE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpire)->EnableWindow(TRUE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);

		SetTimer(1, 1000, NULL);
		break;
	case BUTTON_OLPIC:
		cImage.Load(m_Pic);
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNYES)->SetWindowText("YES");
		GetDlgItem(IDC_BTNNO)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNNO)->SetWindowText("NO");
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);
		break;
	case BUTTON_YESPIC:
		cImage.Load(m_Pic);
		GetDlgItem(IDC_PicCtrl)->EnableWindow(TRUE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpire)->EnableWindow(FALSE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNCONF)->SetWindowText("OK");
		break;
	case BUTTON_COUNT:
		GetDlgItem(IDC_PicCtrl)->EnableWindow(FALSE);
		GetDlgItem(IDC_PicCtrl)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_StaExpPrompt)->EnableWindow(TRUE);
		GetDlgItem(IDC_StaExpPrompt)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_StaExpire)->EnableWindow(TRUE);
		GetDlgItem(IDC_StaExpire)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTNYES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNYES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNNO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTNCONF)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNCONF)->ShowWindow(SW_HIDE);

		SetTimer(1, 1000, NULL);
		break;
	}


}
BOOL CCPromptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加?外的初始化
	cFont.CreatePointFont(150,"Arial");
	GetDlgItem(IDC_StaPrompt)->SetFont(&cFont);
	GetDlgItem(IDC_StaPrompt)->SetWindowText(m_Prompt);
	GetDlgItem(IDC_StaExpPrompt)->SetFont(&cFont);
	GetDlgItem(IDC_StaExpire)->SetFont(&cFont);

	SetWindowText(m_Title);
	ShowDlgWindow(nButton);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX ?性??返回 FALSE
}

BOOL CCPromptDlg::DestroyWindow()
{
	// TODO: 在此添加?用代?和/或?用基?
	cFont.DeleteObject();
	return CDialog::DestroyWindow();
}
void CCPromptDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息?理程序代?和/或?用默?值
    if ((nButton == BUTTON_PIC) || (nButton == BUTTON_COUNT))
	{
		if(--m_StaExpire>0)
		{
			UpdateData(FALSE);
		}else
		{
			KillTimer(1);
			PostMessage(WM_CLOSE);
		}
	}
	CDialog::OnTimer(nIDEvent);
}
void CCPromptDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此?添加消息?理程序代?
	// 不???消息?用 CDialog::OnPaint()
	if (nButton == BUTTON_PIC || nButton == BUTTON_OLPIC || nButton == BUTTON_YESPIC)
	{
		CDC *pDC;
		CRect rect;
		pDC=m_PicCtrl.GetWindowDC();
		m_PicCtrl.GetClientRect(&rect);
		pDC->SetStretchBltMode(HALFTONE);
		cImage.StretchBlt(pDC->m_hDC,0,0,rect.Width(),rect.Height());
		ReleaseDC(pDC);
	}
}

HBRUSH CCPromptDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何?性

	// TODO:  如果默?的不是所需??，?返回另一???
		if(pWnd->m_hWnd==GetDlgItem(IDC_StaPrompt)->GetSafeHwnd())
	{
		pDC->SetTextColor(RGB(0,0,255));
	}
	if(pWnd->m_hWnd==GetDlgItem(IDC_StaExpire)->GetSafeHwnd())
	{
		pDC->SetTextColor(RGB(255,0,0));
	}
	return hbr;
}

