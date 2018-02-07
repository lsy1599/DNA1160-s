#pragma once





// Cmodeldlg 对话框

class Cmodeldlg : public CDialogEx
{
	DECLARE_DYNAMIC(Cmodeldlg)

public:
	Cmodeldlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~Cmodeldlg();
	

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
