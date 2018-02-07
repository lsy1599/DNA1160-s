// MyMessageBox.cpp : 实现文件
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "MyMessageBox.h"
#include "PasswordDLG.h"

extern int nUpFiles;
extern char csErrMsgBuffer[100]; 
extern DWORD CreateUICloseUploadThread(LPVOID lpParam);
// MyMessageBox 对话框
static bool  bCloseDiag = false; 
IMPLEMENT_DYNAMIC(MyMessageBox, CDialog)

MyMessageBox::MyMessageBox(CWnd* pParent /*=NULL*/)
	: CDialog(MyMessageBox::IDD, pParent)
{
	m_csShowMSG.Empty();
}

MyMessageBox::~MyMessageBox()
{
}

void MyMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SHOWTEST ,m_SHOW_TEXT);
}


BEGIN_MESSAGE_MAP(MyMessageBox, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, &MyMessageBox::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_RETRY, &MyMessageBox::OnBnClickedButtonRetry)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &MyMessageBox::OnBnClickedButtonCancel)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_UPLOAD_UPDATE,OnUploadUpdate)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// MyMessageBox 消息处理程序

void MyMessageBox::OnBnClickedButtonRetry()
{
	// TODO: 睰北ン硄?瞶祘?
	HANDLE hUI_Close_Upload;
	CString str;
	if(m_csShowMSG.IsEmpty())
		{
		str = "上傳LOG開始,請稍侯...";
		m_SHOW_TEXT.SetWindowTextA(str);
	    ((CButton*)GetDlgItem(IDC_BUTTON_OK))->EnableWindow(FALSE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_OK))->ShowWindow(SW_HIDE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->EnableWindow(FALSE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->ShowWindow(SW_HIDE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->ShowWindow(SW_HIDE);
        hUI_Close_Upload = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CreateUICloseUploadThread,NULL,0,NULL);
		}
	else
		{
		bCloseDiag =true;
		str = m_csShowMSG;
		m_SHOW_TEXT.SetWindowTextA(str);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->EnableWindow(FALSE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->ShowWindow(SW_HIDE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);
	    ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->ShowWindow(SW_HIDE);
		}
	
}
void MyMessageBox::OnBnClickedButtonCancel()
{
	// TODO: 睰北ン硄?瞶祘?
	    bCloseDiag = true;
		nRet = RET_CANCEL;
		PostMessage(WM_CLOSE);
}

void MyMessageBox::OnBnClickedButtonOk()
{
	// TODO: 睰北ン硄?瞶祘?
	bCloseDiag = true;
	nRet = RET_OK;
	PostMessage(WM_CLOSE);
	
}

BOOL MyMessageBox::OnInitDialog()
{
	
	CDialog::OnInitDialog();
	bCloseDiag = false;
    HANDLE hUI_Close_Upload;
	// TODO:  睰?﹍て
	CString str;
	m_nFontSize = 200;
	m_font.CreatePointFont(m_nFontSize,"Arial");
	m_SHOW_TEXT.SetFont(&m_font,1);
	if(m_csShowMSG.IsEmpty())
		{
		str = "上傳LOG開始,請稍侯...";
		m_SHOW_TEXT.SetWindowTextA(str);
		((CButton*)GetDlgItem(IDC_BUTTON_OK))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_OK))->ShowWindow(SW_HIDE);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->ShowWindow(SW_HIDE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->ShowWindow(SW_HIDE);
		hUI_Close_Upload = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CreateUICloseUploadThread,NULL,0,NULL);
		}
	else
		{
		CRect rect,rect1;
		GetWindowRect(&rect);
		SetWindowText("測試環境異常確認");
		SetWindowPos(&wndTop,rect.left,rect.top,m_column*30/2,m_MSGLines*40+40,SWP_NOMOVE);
		GetWindowRect(&rect);
		GetDlgItem(IDC_BUTTON_OK)->MoveWindow(rect.Width()-60,rect.Height()-60,50,20);
		//GetDlgItem(IDC_BUTTON_OK)->SetWindowPos(NULL,rect1.left,rect1.top,rect1.Width(),rect1.Height(),SWP_NOSIZE);
		str = m_csShowMSG;
		m_SHOW_TEXT.MoveWindow(0,0,rect.Width(),rect.Height()-60);
		m_SHOW_TEXT.SetWindowTextA((TCHAR*)str.GetBuffer());
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->ShowWindow(SW_HIDE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->ShowWindow(SW_HIDE);
		}
	return TRUE;  // return TRUE unless you set the focus to a control
	// 奢盽: OCX ?┦?? FALSE
}

// MyMessageBox 消息处理程序
HBRUSH MyMessageBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	pDC->SetTextColor(RGB(0,0,255));
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
void MyMessageBox::MessageFlag(int nMFlag)
{
	nMessageFlag = nMFlag;
}

void MyMessageBox::SetFontSize(int nFontSize)
{
  m_nFontSize = nFontSize;
}
BOOL MyMessageBox::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	m_font.DeleteObject();

	return CDialog::DestroyWindow();
}
int MyMessageBox::DisplayMessageBox(void)
{
	DoModal();
	return nRet;
}
LRESULT MyMessageBox::OnUploadUpdate(WPARAM wParam, LPARAM lParam)
{
	CString str;

	if(lParam == 0)
		{
	   str.Format("正在上傳LOG,請稍侯...！\n    已上傳%d個文件",nUpFiles);
	   m_SHOW_TEXT.SetWindowTextA(str);
		}
	else if(lParam == 1)
		{
		Sleep(1000);
		int len = strlen(csErrMsgBuffer);
		CString str1 = csErrMsgBuffer;
		for(int i = 0 ; i < len/25; i++)
			{
				str1.Insert((i+1)*25,'\n');
			}

		str.Format("上傳失敗\n請檢查本地和伺服器是否可達\n %s",str1);
		m_SHOW_TEXT.SetWindowTextA(str+",點擊Retry按鈕!");
		((CButton*)GetDlgItem(IDC_BUTTON_OK))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_OK))->ShowWindow(SW_HIDE);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->ShowWindow(SW_SHOW);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->SetFocus();
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->ShowWindow(SW_SHOW);
		}

	else if(lParam == 2)
	{
		str.Format("上傳成功！\n  共上傳%d個文件",nUpFiles);
		m_SHOW_TEXT.SetWindowTextA(str);
		((CButton*)GetDlgItem(IDC_BUTTON_OK))->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_BUTTON_OK))->ShowWindow(SW_SHOW);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_RETRY))->ShowWindow(SW_HIDE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->ShowWindow(SW_HIDE);
	}
	else
	{
		;
	}
	return 0;
}

void MyMessageBox::OnClose()
{
	// TODO: 在此添加消息?理程序代?和/或?用默?值
  
	if(m_csShowMSG.IsEmpty())
	{
	
    if(bCloseDiag)
	   {
		CDialog::OnClose();
		}
	}
	else
	{
	CPasswordDLG dlg(NULL);
	dlg.DoModal();
	if(dlg.IsOK())
		{
		CDialog::OnClose();
		}
	}
}
void MyMessageBox::vSetAlarmMessage(CString csMessage, UINT num,UINT column)
{
   m_csShowMSG = csMessage;
   m_MSGLines = num;
   m_column = column;
}