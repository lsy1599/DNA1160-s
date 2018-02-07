#pragma once
#include "afxwin.h"
#include "UI.h"
#include "winbase.h"


// CMacDlg dialog

class CMacDlg : public CDialog
{
	DECLARE_DYNAMIC(CMacDlg)

public:
	CMacDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMacDlg();

// Dialog Data
	enum { IDD = IDD_DLG_MAC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PreInitDialog();
	DECLARE_MESSAGE_MAP()	
public:
	CString  m_csMac;
	CString	 m_csSN;
	CString  m_csSSN1;
	CString  m_csSSN2;	
	HWND m_NotifyhWnd;
	bool m_bMacLegal;
	CEdit m_EDIT_mac;
	CEdit m_EDIT_sn;
	CEdit m_EDIT_ssn1;
	CEdit m_EDIT_ssn2;

public:
	void Create(HWND hWnd);
	BOOL	CheckMac();
	BOOL	CheckSN();
	BOOL	CheckSSN1();
	BOOL	CheckSSN2();
	void ShowMessage(CString msg);
	void ShowActionMessage(CString msg);
	//void MakeBinFile();
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();	
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeEditMac();
	afx_msg void OnEnChangeEditSsn1();
	afx_msg void OnEnChangeEditSn();
};
