#pragma once


// CCInputNum 对话框

class CCInputNum : public CDialog
{
	DECLARE_DYNAMIC(CCInputNum)

public:
	CCInputNum(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCInputNum();

// 对话框数据
	enum { IDD = IDD_OPNumber };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClicked();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	void Create(HWND hWnd);
public:
	CFont cFont;
	HWND m_MyhWnd;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeEditopnum();
	afx_msg void OnEnChangeEditpasswd();
	afx_msg void OnStnClickedStapasswd();
};
