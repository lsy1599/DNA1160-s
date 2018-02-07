#pragma once
#include "afxwin.h"


// CCModelSel �Ի���

class CCModelSel : public CDialogEx
{
	DECLARE_DYNAMIC(CCModelSel)

public:
	CCModelSel(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCModelSel();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_Model_Select };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeModelcomb();

	CStatic m_StaModel;
	CFont m_Font;
	CString m_csModel;
	CString DisplayDlg();

	CComboBox m_ModelComb;
	afx_msg void OnBnClickedBtnok();
};
