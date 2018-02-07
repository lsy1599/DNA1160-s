#pragma once


// CPasswordDLG 对话框

class CPasswordDLG : public CDialog
{
	DECLARE_DYNAMIC(CPasswordDLG)

public:
	CPasswordDLG();   // 标准构造函数
    CPasswordDLG(CWnd* pParent = NULL,CString password="");
	virtual ~CPasswordDLG();

// 对话框数据
	enum { IDD = IDD_DLG_PASSWORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_csPassword;
	bool m_bExit;

public:
	afx_msg void OnClose();
public:
	bool IsOK(void);
public:
	afx_msg void OnBnClickedOk();
private:
	CString m_csPrePassword;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtncancel();
};
