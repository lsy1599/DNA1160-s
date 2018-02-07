
#ifndef _ATSCOM_H_
#define _ATSCOM_H_

/* ________ for General functions ________ */
#define MSG_SHOW_MESSAGE		(WM_USER + 3)
#define MSG_TEST_END			(WM_USER + 4)	
#define MSG_SHOW_GOLDEN			(WM_USER + 5)
#define MSG_SET_ITEM_STATUS		(WM_USER + 6)
#define MSG_SET_GOLDEN_ITEM		(WM_USER + 7)
#define MSG_SHOW_ACT_MESSAGE	(WM_USER + 8)
#define MSG_SEND_LOG_MESSAGE		(WM_USER + 9)

#define MSG_FT_PT_MESSAGE			(WM_USER + 12)
#define MSG_SHOW_MAC_BEGIN			(WM_USER + 13)
#define MSG_SHOW_MAC_END			(WM_USER + 14)

#define MSG_SHOW_MODEL          (WM_USER+30)
/* ======== for General functions ======== */

/* ________ for Item status ________ */
#define STATUS_ITEM_PASS		1
#define STATUS_ITEM_FAIL		2
#define STATUS_ITEM_TESTING		3
#define STATUS_ITEM_NORMAL		4
/* ======== for Item status ======== */

/* ________ for Color reference ________ */
#define COLOR_NORMAL	GetSysColor(COLOR_3DFACE)
#define COLOR_BLUE		RGB(50,50,255)
#define COLOR_YELLOW	RGB(255,100,50)
#define COLOR_RED		RGB(230,30,30)
#define COLOR_GREEN		RGB(100,255,50)
#define COLOR_BLACK		RGB(0,0,0)
#define COLOR_WHITE		RGB(255,255,255)
/* ======== for Color reference ======== */

typedef struct _SFIS_PM
{
	char	czSN[26];
	char	czPN[21];
	char	czMAC[13];
	char	czSSN1[26];
	char	czSSN2[36];     //pSFIS
	char	czFW[13];
	char	czSta[13];
	char	czEC[5];
	char	czEND[4];
	char	czResult[5];
} SFIS_PM;

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
} DEVICE_HANDLE;

extern DEVICE_HANDLE *DevHndl;
extern SFIS_PM *sfisData;


//Export API
bool WINAPI PreInit(HWND notifyWnd, DEVICE_HANDLE *DevHndl);
bool WINAPI InitDut(SFIS_PM *pSFIS);
bool WINAPI StartTest(void);
bool WINAPI TestEnd(void);
void WINAPI Terminal(void);

//Windows message interface

void ShowUIGolden();

void Show_PF_or_FT(CString msg);
void ShowMAC_BEGIN(CString msg);
void ShowMAC_END(CString msg);

void ShowUIMessage(CString msg);

void SetDutItemStatusPass(CString csItemTitle);

void SetDutItemStatusNormal(CString csItemTitle);

void SetDutItemStatusTesting(CString csItemTitle);

void SetDutItemStatusFail(CString csItemTitle);

void SetGoldenItemStatusTesting(CString csItemTitle);

void ShowUIActMessage(CString msg, COLORREF color);

void CreateLogMessage(CString msg);

void SendLogMessage(CString msg);

void CreateLogMessageEnd();

void ShowModel(CString msg);//在UI上显示机种

bool TestPreInit(void);
bool DutInit(void);
bool StartTestItem(void);
bool DutTestEnd(void);
void TestTerminal(void);

#endif