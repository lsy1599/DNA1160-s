#pragma once
#include "Resource.h"
#define TEST_ITEM_NUM 30
#define TOTAL_MODEL 3
#define NSB125C 0
#define NSB125D 1
#define NSB125E 2
#define PT 0
#define FT 1
#define TEMP 2
#define LOG_PATH_MAX 300
#define TESTITEMNUM 25



typedef struct InitInfo
{
	int Com;
	int Model;
	int Station;
	int Item[TEST_ITEM_NUM];
	char LogPath[LOG_PATH_MAX];
}Init_Info;
// CInitDlg 对话框

class CInitDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInitDlg)

public:
	CInitDlg(CWnd* pParent = NULL);   // 标准构造函数
	CInitDlg(CString csmodel,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInitDlg();

// 对话框数据
	enum { IDD = IDD_InitDialog };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMode12();
	afx_msg void OnBnClickedModel24();
	afx_msg void OnBnClickedModel48();
	afx_msg void OnBnClickedStationpt();
	afx_msg void OnBnClickedStationft();
	afx_msg void OnBnClickedStationtemp();
	afx_msg void OnBnClickedBtnlogpath();
	afx_msg void UpdCheckboxState();                //tree MD
	void CreateFolder(CString str);
	int GetCurStation(void );
	int GetCurModel(void );
	struct InitInfo DisplayItem();
	int get_m_TotalItem(int i)
	{
		return m_TotalItem[i];
	}
private:
	int m_PTItemCheck[TOTAL_MODEL][TEST_ITEM_NUM];
	int m_FTItemCheck[TOTAL_MODEL][TEST_ITEM_NUM];
	int m_TotalItem[TEST_ITEM_NUM];
	CString csLogFolder;
	InitInfo m_InitInfo;
public:
	afx_msg void OnBnClickedItemck1();
	afx_msg void OnBnClickedItemck2();
	afx_msg void OnBnClickedItemck3();
	afx_msg void OnBnClickedItemck4();
	afx_msg void OnBnClickedItemck5();
	afx_msg void OnBnClickedItemck6();
	afx_msg void OnBnClickedItemck7();
	afx_msg void OnBnClickedItemck8();
	afx_msg void OnBnClickedItemck9();
	afx_msg void OnBnClickedItemck10();
	afx_msg void OnBnClickedItemck11();
	afx_msg void OnBnClickedItemck12();
	afx_msg void OnBnClickedItemck13();
	afx_msg void OnBnClickedItemck14();
	afx_msg void OnBnClickedItemck15();
	afx_msg void OnBnClickedItemck16();
	afx_msg void OnBnClickedItemck17();
	afx_msg void OnBnClickedItemck18();
	afx_msg void OnBnClickedBtnok();
	afx_msg void OnBnClickedItemck19();
	afx_msg void OnBnClickedItemck20();
	afx_msg void OnBnClickedItemck21();
	afx_msg void OnBnClickedItemck22();
	afx_msg void OnBnClickedItemck23();
	afx_msg void OnBnClickedItemck24();
	afx_msg void OnBnClickedItemck25();
	afx_msg void OnBnClickedItemck26();
	afx_msg void OnBnClickedItemck27();
	afx_msg void OnBnClickedItemck28();
	afx_msg void OnBnClickedItemck29();
	afx_msg void OnBnClickedItemck30();
//	afx_msg void OnCbnSelchangeComportcb2();///////////////////////////////
	afx_msg void OnCbnSelchangeComportcb();
	afx_msg void OnEnChangeLogpathedit();
	afx_msg void OnBnClickedStacom();
	CString strModel;
public:
	void SetModel(CString Model)
	{
		strModel = Model;
	}
};



