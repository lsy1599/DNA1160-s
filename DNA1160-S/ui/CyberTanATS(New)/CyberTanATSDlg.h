// CyberTanATSDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Label.h"
#include "UI.h"
#include "MacDlg.h"
#include "comm.h"
#include "afxcmn.h"


#define STATUS_SHOW_DECT		1
#define STATUS_SHOW_TEST		2
#define STATUS_SHOW_FAIL		3
#define STATUS_SHOW_PASS		4
#define STATUS_SHOW_GOLD		5
#define STATUS_SHOW_IDLE		6
#define STATUS_SHOW_ERRCODE		7

#define UPLOAD_LOG_FAIL			"LOGF"

#define STATUS_ITEM_PASS		1
#define STATUS_ITEM_FAIL		2
#define STATUS_ITEM_TESTING		3
#define STATUS_ITEM_NORMAL		4

#define COLOR_NORMAL	GetSysColor(COLOR_3DFACE)
#define COLOR_BLUE		RGB(50,50,255)
#define COLOR_YELLOW	RGB(255,100,50)
#define COLOR_RED		RGB(230,30,30)
#define COLOR_GREEN		RGB(100,255,50)
#define COLOR_BLACK		RGB(0,0,0)
#define COLOR_WHITE		RGB(255,255,255)
/*_____________for Log Upload_________*/
#define TOTAL_RESULT			1
#define PASS_ONLY				2
#define FAIL_ONLY				3
#define UPLOAD_RECORD			4
#define UPLOAD_UILOG			5

/* ________ for timer ________ */
#define TEST_START					200
#define TEST_END					300
#define SFIS_TEST					400
#define AUTOTEST_PING_DUT_UNLOAD	500
#define FLASH_SIGNAL				800
#define FLASH_IQSIGNAL				1000
/* ======== for timer ======== */

#define TEST_MODE			0
#define DEBUG_MODE          1 

/* ======== for Input Label ======= */
#define STATUS_LABEL_INPUT1    1
#define STATUS_LABEL_INPUT2	   2
#define STATUS_LABEL_INPUT3    3
#define STATUS_LABEL_SHOW	   4
#define STATUS_LABEL_HIDEALL   5
#define STATUS_LABEL_HIDEMAC   6
#define STATUS_LABEL_HIDESSN1  7


typedef struct _LOOPTEST_STRUCT
{
	UINT	m_bLoopTest;
	unsigned m_iCounter;
	unsigned m_iCountLimite;
}	LOOPTEST_STRUCT;
typedef struct _FtpLog_Res
{
	CString csLogTitle;
	CString csLogRes;
	CString csHightLimit;
	CString csLowLimit;
} FtpLog_STRUCT;
typedef struct _TEST_FUNC
{
	UINT m_TestEnable;
	CString csStationName;
	CString csStep;
	char m_DllName[MAX_PATH];
	char m_DllVersion[MAX_PATH];
	int     nCALL_PN_Enable; //20090421
	CString czModelPN;//20090421
	int     nCheckSFIS_InfoFS; //2009042
	CString csDllLoadIniName;//20090421
	int    nIQExpire_Enable;
}	TEST_FUNC;
typedef struct _TEST_DATA
{
	TEST_FUNC  SFIS,SFIS_Retry,STATION;
}	TEST_DATA;
typedef struct _Log_Record
{
	int m_nLogItem[200];
	CString csTestName[200];
	CString HighLimit[200];
	CString LowLimit[200];
	CString	csRead[200];
	CString csRes[200];
	CString Unit[200];
} Log_Record;
typedef struct 
{
	bool m_bLogByResult;
	bool m_bLogAll;
	bool m_bUpLoad;
	UINT nWaitTime;
	CString csUserNum;
	CString csLogFilePath;//For log file path
	CString csLogFilePathUI;
	CString csLogUploadTime;
	CString csLogUploadTime1;
	CString csLogMappingDriver;
	CString csLogServerPath;
	CString csUser;
	CString csPassWord;
	CString csStationNum;
	CString csComputerName;
} _LOG_CONFIG;

typedef struct
{
	bool	m_bDB_Enable;
	CString m_csLineNumber;
	CString m_csGroupName;
	CString m_csPC_IP;
	
} _DB_SETTING;
typedef struct
{
	CString m_ftpPath;
	CString csUserName;
	CString csPasswd;
	UINT nPort;
}_FTP_SETTING;
typedef struct _RETEST_MSG
{
  UINT m_nTotalRetestCounter;
  UINT m_RetesRateThreshold;
  CString m_csAlarmMSG;
  UINT m_num;
  UINT m_Column;
  CString m_csPassword;
} RETEST_MSG;

typedef struct _ATSCONFIG
{	
	UINT	m_FTP, m_bAutoTest, m_bServe, m_bootTime, m_startOnButton, m_ShowDect, m_UiLog;
	bool    m_bInputSN, m_bInputMac,m_bAutoMac, m_bInputSSN1, m_bInputSSN2;
	bool	m_bCheckSN, m_bCheckMac,m_bWriteMac,m_bCheckSSN1,m_bSSN1ToMac,m_bCheckSSN2;
	
	UINT	m_nSN_Length,m_nMAC_Length,m_nSSN1_Length,m_nSSN2_Length;
	CString m_csMacStart, m_csMacEnd;

	LOOPTEST_STRUCT LoopTest;
	CString m_ipEmbedIP;
	CString m_csModelName;;
	TEST_DATA TestFunc;
	_LOG_CONFIG Log;
	_DB_SETTING DB_Setting;
	_FTP_SETTING ftp_Setting;


	DWORD dwTimeStart, dwTimeTotal;
	CString csSFISPassword;//It is necessary For SFIS disable 
	
	RETEST_MSG m_RetestMSG;

	CString HelpFile;
	CString ErrorCodeFile;

	CString TerminalNameSFIS;
	
	CString csStationName;
	CString csUserName;
	CString csPasswd;
	UINT m_InputNum;
	bool m_ModelSelEnable;
} ATSCONFIG;

typedef struct _DEVICE_HANDLE_ST
{
	HANDLE dev0;
	HANDLE dev1;
	HANDLE dev2;
	HANDLE dev3;
	HANDLE dev4;
	HANDLE dev5;
	HANDLE dev6;
	HANDLE dev7;
	HANDLE dev8;
	HANDLE dev9;	
	HANDLE dev10;	
	HANDLE dev11;	
	HANDLE dev12;	
	HANDLE dev13;	
} DEVICE_HANDLE;

typedef struct _LogST
{
	CString title[2000];
	CString value[2000];
	CString filename;
	UINT	nCount; //記錄CreateLog之後，總共有幾個欄位，不含前面的固定欄位
	UINT	nIndex; //用來記錄每次抓到的欄位index
	bool	bCreated; //記錄此log是否被Create過

	void ResetAll()
	{
		nCount = 0;
		nIndex = 0;
		bCreated = false;

		for ( int i = 0 ; i < 2000 ; i++ )
		{
			title[i] = "";
			value[i] = "";
		}
	}

	void ResetValue()
	{		
		nIndex = 0;

		for ( int i = 0 ; i < 2000 ; i++ )
		{			
			value[i] = "";
		}
	}

} LogST;



// CATSDlg dialog
class CATSDlg : public CDialog
{
// Construction
public:
	CATSDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CYBERTANATS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	int	 m_nFlashFlag;
	int	m_nIQFlash;
	int	 m_nTestingItem;
	void SetTestStatus(int nStatus);
	void LoadATSini(void);
	bool LoadTestItem(void);
	void ShowMessage(CString msg);
	CString CATSDlg::WriteLog(bool nType);
	void DebugMessage(CString msg);
	void SetTestItemStatus(int nID, int nStatus);	
	bool StoreTestItem(TEST_FUNC testFunction);
	bool CATSDlg::CheckOKNG(int nWaitTime);
	void GetUiMsg();
	bool MakeMESLog(bool nType,CString &csLogMessage);
	CString MakeFTPInfo(bool nType,CString csMESInfo);
	CString MakeNetLog(bool nType);
	bool UpLoadLog(const CString csLogFile);
	CString GetFileVersion(char *filename);
	LRESULT OnShowMessage(WPARAM, LPARAM);
	LRESULT OnShowActMessage(WPARAM a, LPARAM b);	
	LRESULT OnTestEnd(WPARAM Func, LPARAM Result);	
	LRESULT OnShowGolden(WPARAM, LPARAM);
	LRESULT OnTestStart(WPARAM, LPARAM);
	//PT	FT
	LRESULT OnSelectPT_FT_Status(WPARAM wParam, LPARAM lParam);

	LRESULT OnSelectModel(WPARAM wParam, LPARAM lParam);
	LRESULT OnSelectMAC_BEGIN(WPARAM wParam, LPARAM lParam);
	LRESULT OnSelectMAC_END(WPARAM wParam, LPARAM lParam);
	//OnSelectMAC_BEGIN
	LRESULT OnSetDutItemStatus(WPARAM nID, LPARAM nStatus);
	LRESULT OnSetGoldenItemStatus(WPARAM wParam, LPARAM lParam);
	LRESULT OnLogMessage(WPARAM type, LPARAM msg);
	//LRESULT OnMESLogMessage(WPARAM wParam,LPARAM lParam);
	LRESULT OnEnableTestButton(WPARAM enable, LPARAM lParam);
	LRESULT OnSetTestStatus(WPARAM enable, LPARAM lParam);
	LRESULT OnDeleteObject(WPARAM wParam, LPARAM lParam);
	//LRESULT OnSetLabelStatus(WPARAM enable, LPARAM lParam);

	afx_msg void OnStartBtn();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
		
	
	CString m_appVersion;
	CString m_dllInfo;
	
	char m_TestItemList[200][50]; //可存放200個test items
	UINT m_TestItemCount;	//記錄總共有多少個test items
	UINT m_nCurrentTestItemPage; //記錄目前的item page
	BOOL m_debug_message;

public:		
	BOOL		PreTranslateMessage(MSG* pMsg);
	CEdit		m_list_msg;
	CStatic		m_total_count;	//顯示總測試次數
	CStatic		m_fail_count;	//顯示總fail數
	CStatic		m_pass_count;	//顯示pass數
	CLabel		m_testitem[30]; //共有30個Label以供顯示test items
	CLabel		m_testStatus; //Show test status
	CLabel		m_ActMessage; //Show action messageノ
	CLabel		m_SFISMessage; //Show SFIS messageノ
	CButton		m_testbutton;
	int			nInitialLoadIsOk; //用來記錄初始化時是否正確執行
	int			nInitIQ;//用來記錄IQ的到期
	CStatic		m_TestStateShow;
	CLabel      m_TestStateCtrl;
	
	CFont cFont;
	
	afx_msg void OnClose();
	
public:
//	afx_msg void OnBnClickedButtonHelp();
	BOOL ReInitSystem();
	void ReDrawWindow();
public:
	CLabel m_IQExpire;
public:
	CLabel m_IQFlash;
public:
	void OnBnDoubleclickedButtonHelp();
public:
	afx_msg void OnBnClickedButtoncancel();
public:
	afx_msg void OnBnClickedButtonHelp();
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedSfisMeg();
	afx_msg void OnEnChangeUiMsg();
	afx_msg void OnStnClickedPassCount();
	afx_msg void OnBnClickedButtonHelp2();
	afx_msg void OnBnClickedButtonHelp3();
};
void WriteLogUploadInfo(CString csFileName,CString Result);
int GetLogUploadInfo();
int CopyFileMode(CString csSrcPath,CString csFileName,int nFileMode);
int  MyCopyFile(CString srcPath, CString destPath);
UINT HandleLogUpload(LPVOID pParam);
bool ListFolder(CString sPath,int nUpFlag);
bool LogServerDirectoryIni();
DWORD CreateUICloseUploadThread(LPVOID lpParam);
DWORD OnMountNetDriver();
DWORD OnUnMountNetDriver();
bool ftpLog();
void WriteLogNetDriverInfo(CString csOper,DWORD ErrorCode);
static void exec(char * pCmd);
int ThreadGetIQExpire();
char* LengthAlign(char Buffer[],int nLenth);
UINT MountDriver();
//void TestFailCal(CString &csErrorCode);
