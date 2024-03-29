#pragma once
#include "afxwin.h"


// MyMessageBox 对话框
#define RET_OK 1
#define RET_RETRY 2
#define RET_CANCEL 3
#define WM_UPLOAD_UPDATE		(WM_USER + 101)
class MyMessageBox : public CDialog
{
	DECLARE_DYNAMIC(MyMessageBox)

public:
	MyMessageBox(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~MyMessageBox();
	CStatic	m_SHOW_TEXT;

// 对话框数据
	enum { IDD = IDD_MessageBox };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual	BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonRetry();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void MessageFlag(int nMFlag);
	void SetFontSize(int nFontSize);
	BOOL DestroyWindow();
	int DisplayMessageBox(void);
	LRESULT OnUploadUpdate(WPARAM wParam, LPARAM lParam);
	void vSetAlarmMessage(CString csMessage,UINT num,UINT column);
private:
	int nRet;
	CFont m_font;
	int nMessageFlag;
	int m_nFontSize;
	CString m_csShowMSG; 
	UINT m_MSGLines;
	UINT m_column;
	afx_msg void OnClose();
};
