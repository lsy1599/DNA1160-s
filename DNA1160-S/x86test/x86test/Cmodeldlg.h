#pragma once





// Cmodeldlg �Ի���

class Cmodeldlg : public CDialogEx
{
	DECLARE_DYNAMIC(Cmodeldlg)

public:
	Cmodeldlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~Cmodeldlg();
	

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();       //tree MD
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComportcb();
	//tree MD
	CString cstr;
	CString DisplayDlg();
	//tree MD END

};
