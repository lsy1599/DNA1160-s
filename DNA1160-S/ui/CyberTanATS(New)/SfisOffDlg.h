#pragma once


// SfisOffDlg dialog

class SfisOffDlg : public CDialog
{
	DECLARE_DYNAMIC(SfisOffDlg)

public:
	SfisOffDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SfisOffDlg();

// Dialog Data
	enum { IDD = IDD_DLG_SFISOFF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditSfisoff();
};
