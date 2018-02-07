#pragma once
#include "afxwin.h"
#include "atlimage.h"
#include "Resource.h"

#define BTN_YES 1
#define BTN_NO 2

#define BUTTON_YESNO    1
#define BUTTON_YES      2
#define BUTTON_PIC      3
#define BUTTON_OLPIC    4
#define BUTTON_YESPIC   5
#define BUTTON_COUNT    6


// CCPromptDlg 对话框

class CCPromptDlg : public CDialog
{
	DECLARE_DYNAMIC(CCPromptDlg)

public:
	CCPromptDlg(CWnd* pParent = NULL);   // 标准构造函数
	CCPromptDlg(CString csTitle,CString csPrompt,CString csPic,int nTime,int nBtn,CWnd* pParent=NULL);
	virtual ~CCPromptDlg();

// 对话框数据
	enum { IDD = IDD_PromptDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_StaPrompt;
	CStatic m_PicCtrl;
	int m_StaExpire;
	CString m_Title;
	CString m_Prompt;
	CString m_Pic;
	int nButton;
	int nRes;

	CFont cFont;
	CImage cImage;
private:
	void ShowDlgWindow(int nWindow);
public:
	afx_msg void OnBnClickedBtnyes();
	afx_msg void OnBnClickedBtnconf();
	afx_msg void OnBnClickedBtnno();
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int DisplayDlg();
};


