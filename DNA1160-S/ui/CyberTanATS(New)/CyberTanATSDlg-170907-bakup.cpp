// CyberTanATSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "CyberTanATSDlg.h"
#include "Comm.h"
#include "iosocket.h"
#include "SFIS.h"
#include "FTP_Client.h"
#include "assert.h"
#include "AutoInputKey.h"
#include "CBT_Common.h"
#include "DB.h"
#include "SfisOffDlg.h"
#include "CInputNum.h"
#include "MyMessageBox.h"
#include "shlwapi.h"
#include "Winnetwk.h"
#include "DlgSetting.h"
#include "PasswordDLG.h"
#include "atlbase.h"
#include <vector>
#include "afxinet.h"
#include "CModelSel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/* 新加 */
#define	TEST_PASS					0x0001
#define	TEST_FAIL					0x0002
#define FIRST_RECEIVE_SFIS_DATA		1
#define SEND_PASS_INFO_TO_SFIS		2
#define SEND_FAIL_INFO_TO_SFIS		3

#define SFIS_OFF_Password			"disablesfis"
#define nNameLen					0
/* 新加 */
#define REG_KEY_LENGTH				10000
#define MAX_ERR_NUM                 200
//!!這裡是調整所有DLL Handle的個數
HINSTANCE hDLL = NULL;
//sfisData
SFIS_PM	SFIS;
SFIS_PM	secondSFIS;//20090428
CATSDlg *closeCATSDlg;//20090428
DEVICE_HANDLE *DeviceHandle = new DEVICE_HANDLE;
int retSFIS = -99;
ATSCONFIG AtsInfo;
LogST STATION_Log;
Log_Record Ftp_UpLog;
FtpLog_STRUCT LogRes[200];
int m_LogCount=0;
UINT nPassCount, nFailCount, nTotalCount;
CWinThread *pMainThread;
HWND UI_hWnd,m_SNWnd,m_MACWnd,m_SSN1Wnd;
SfisOffDlg SFISOFF;//20090622 tommy
BOOL bTestMode;
BOOL nRunningFlag = TRUE;
CString csTotalMes;

static int		Sfis_status = 0;

UINT ThreadSFISIN(LPVOID pParam);
UINT ThreadTester(LPVOID pParam);
int FuncTest(TEST_FUNC testFunction);
int SFISTestFunc(BOOL bTestResult,int nTestState);
///////////////////////////////////////////////20090429///////////////////////////////////////////////
////////////////////////////////
char receiveBufferThread[4096];
char BufferCheck[4096];
//CString	csGetSfisInfo;//Tommy debug
CString csLogPath;
HANDLE hIQFileTime;
SYSTEMTIME  sTime,sIQTime,sStartTime,sEndTime;
CString UiLogName, csUILog, strANT[3], csToMESLogPath;
int nUpFiles;
int nRetry=0;
static int nDlgSettingCounter = 0;
int nFileLength=0,nFileUadLen=0;
int nLinkRes=0;
static CWinThread *mytimethread = NULL;

extern bool bEfficencyCalc ;
extern bool bFirstStart ;
extern DWORD dwBeginTime;
extern DWORD dwTotalEfficiencyTime ;
extern DWORD dwEfficiencyBeginTime ;
extern DWORD dwEfficiencyEndTime ;
CDlgSetting *pDlgSetting = NULL; 
extern int nCancel;
void List(CFtpConnection *pConnection,CString csPath,CString csFileFind);
////////////////////////////////
UINT ThreadTesterGetSFIS(LPVOID pParam);
//UINT ThreadGetBarCode(LPVOID pParam);

DWORD WINAPI ThreadInput(LPVOID pParam)
{
	memset(&BufferCheck,0,sizeof(BufferCheck));
	Sfis_status=UserInput(AtsInfo.csUserName.GetBuffer(),AtsInfo.csPasswd.GetBuffer(),BufferCheck);
	return TRUE;
}
DWORD WINAPI ThreadTestEndSFIS(LPVOID pParam)
{
	memset(&BufferCheck, 0, sizeof(BufferCheck));
	Sfis_status=SendMsgMES(SFIS.czSN,SFIS.czMAC,SFIS.czSSN1,AtsInfo.csUserName.GetBuffer(),&SFIS,BufferCheck,0);
	return TRUE;
}
DWORD WINAPI ThreadTestGetSFIS(LPVOID pParam)
{
	memset(&receiveBufferThread, 0, sizeof(receiveBufferThread));
	//Sfis_status = DetSFISmsg(&SFIS,receiveBufferThread);
	Sfis_status=SendMsgMES(SFIS.czSN,SFIS.czMAC,SFIS.czSSN1,AtsInfo.csUserName.GetBuffer(),&SFIS,receiveBufferThread,1);
	return TRUE;
}
DWORD WINAPI ThreadLinkSFIS(LPVOID pParam)
{
	memset(&BufferCheck,0,sizeof(BufferCheck));
	Sfis_status=FinalCheckLink(SFIS.czSN,SFIS.czSSN1,SFIS.czMAC,BufferCheck,2);
	return TRUE;
}
DWORD WINAPI ThreadCheckSFIS(LPVOID pParam)
{
	memset(&BufferCheck,0,sizeof(BufferCheck));
	int nChoice=0;
	if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT"))
	{
		nChoice=1;
	}
	else if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RC"))
	{
		sprintf(SFIS.czSSN1,"%s",SFIS.czSN);
		nChoice=3;
	}
	else
	{
		nChoice=1;
	}
	Sfis_status=FinalCheckLink(SFIS.czSN,SFIS.czSSN1,SFIS.czMAC,BufferCheck,nChoice);
	return TRUE;
}
DWORD WINAPI ThreadGetBarCode(LPVOID pParam)
{
	CMacDlg dlg;
	dlg.Create(UI_hWnd);
	dlg.DoModal();
	while(1)
	{	
		//CString strMac = dlg.m_csMac;
		CString strSN = dlg.m_csSN;
		//CString strSSN1 = dlg.m_csSSN1;
		//CString strSSN2 = dlg.m_csSSN2;
		if(AtsInfo.m_InputNum==1)
		{
			if(dlg.CheckSN())
			{
				::SendMessage(dlg.m_hWnd,WM_COMMAND,IDOK,0);
				break;
			}else
			{
				continue;
			}
		}
	strcpy(SFIS.czSN, dlg.m_csSN);	
	}
	return TRUE;
}
int ThreadGetIQExpire()
{
	FILETIME lpFileCreateTime,fTime;
	SYSTEMTIME sLocalTime;
	::GetLocalTime(&sLocalTime);
	DWORD dwRegVal=0;
	hIQFileTime=CreateFile("C:\\Program Files\\MATLAB",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if(hIQFileTime==INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	GetFileTime(hIQFileTime,&lpFileCreateTime,NULL,NULL);
	CloseHandle(hIQFileTime);
	FileTimeToLocalFileTime(&lpFileCreateTime,&fTime);
	FileTimeToSystemTime(&fTime,&sIQTime);
	if(sLocalTime.wYear>sIQTime.wYear)
	{
		dwRegVal=0;
	}else
	{
		if((sLocalTime.wMonth-sIQTime.wMonth)<1)
			{
				if((sLocalTime.wMonth-sIQTime.wMonth)<0)
				{
					dwRegVal=-1;
				}else
				{
					dwRegVal=30+sIQTime.wDay-sLocalTime.wDay;
				}
			}else if((sLocalTime.wMonth-sIQTime.wMonth)==1)
			{
				if(sLocalTime.wDay+2>sIQTime.wDay)
				{
					dwRegVal=-1;
				}else
				{
					dwRegVal=sIQTime.wDay-sLocalTime.wDay;
				}
			}else
			{
				dwRegVal=-1;
			}
	}
	return dwRegVal;
}
///////////////////////////////////////////////20090429///////////////////////////////////////////////
void CallModel()//by sz for sfis  FT0028
{
	HANDLE		hRS232;
	DCB dcb;
	COMMTIMEOUTS timeOut;

	// Initial ComPort

    hRS232=CreateFile("COM4",
					  GENERIC_READ|GENERIC_WRITE,
					  0,
					  NULL,
					  OPEN_EXISTING,
					  0,
					  NULL);
	DeleteFile("COM4");
    if(!hRS232)
	{
		::AfxMessageBox("Open COM4 Port Fail!",0,0);
		CloseHandle(hRS232);
		exit(1);//20090428//return;
	}

	ZeroMemory(&dcb,sizeof(dcb));
	dcb.DCBlength= sizeof(dcb);
	dcb.BaudRate=9600;
	dcb.ByteSize=8;
	dcb.Parity=NOPARITY;
	dcb.StopBits=ONESTOPBIT;

	if(!SetCommState(hRS232,&dcb))
	{
		::AfxMessageBox("Open COM Port Fail!",0,0);
       //DisplayCustomize("Open COM Port Fail!", RGB(200,0,0), 20);
		CloseHandle(hRS232);
	    exit(1);//20090428//return;
	}
	timeOut.ReadIntervalTimeout=2;
	timeOut.ReadTotalTimeoutConstant=2;
	timeOut.ReadTotalTimeoutMultiplier=2;
	timeOut.WriteTotalTimeoutConstant=2;
	timeOut.WriteTotalTimeoutMultiplier=2;
	SetCommTimeouts(hRS232,&timeOut);
	///////////////////////////////////////////////////////////////////////////////////////////
	int nReTest;
	//MessageBox("████ 請依序掃入 SN、MAC 條碼 ████");

	//nReTest =::MessageBox(0,"████ 請掃入條碼 ████", "請用滑鼠選擇", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
	nReTest = ::MessageBox(0, "████ please scan the bar code ████", "please use mouse to select", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
	if ( nReTest == IDNO )
	{
		CloseHandle(hRS232);
		exit(1);//20090428//return;
	}

	int	 nType = 0,FileLen = 0;
	CString tmp, csTmp;
	DWORD byteCount = 0,byteTmp = 0;
	char receiveBuffer[255], rcvTmp[255], *pPoint;
	char tempModel[512];
	CString csBuffer,csBuffer1;
	DWORD	TimeStart=0, TimePass=0;
	memset(rcvTmp, 0, sizeof(rcvTmp));
	TimeStart = GetTickCount();
    int retryCount=0;
	while(1)
	{

			memset(receiveBuffer, 0, sizeof(receiveBuffer));
			if(!ReadFile(hRS232,receiveBuffer,sizeof(receiveBuffer),&byteCount,NULL))
			//if(!DetSFISmsg(&SFIS,receiveBuffer));
			{
				if(TimePass>2000000)
				{
					::AfxMessageBox(receiveBuffer,0,0);
					CloseHandle(hRS232);
					exit(1);//20090428//return;
				}
			}
			TimePass = GetTickCount() - TimeStart;
			if(byteCount != 0)
			{
					strcat(rcvTmp,receiveBuffer);
					byteTmp = strlen(rcvTmp);

					if(rcvTmp[byteTmp - 1] == '\r')
					{

							if(rcvTmp[25] == ' ')
							{
								memset(rcvTmp, 0, sizeof(rcvTmp));
									continue;
							}
							else
							{
								strcpy(receiveBuffer,rcvTmp);
								break;
							}
							memset(rcvTmp, 0, sizeof(rcvTmp));
					}
			}
			retryCount++;
			if(retryCount>5)//100 about 50s
			{
				retryCount=0;
				nReTest =::MessageBox(0,receiveBuffer, 0, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
				if ( nReTest == IDNO )
				{
					CloseHandle(hRS232);
					exit(1);//20090428//return;
				}
			}
	}

	FileLen = strlen(receiveBuffer);
	csBuffer1.Format("%d   |%s|",FileLen,receiveBuffer);
	//MessageBox(csBuffer1);
	memset(tempModel, 0, sizeof(tempModel));
	memset(tempModel, 32, sizeof(tempModel)-1);
	pPoint = receiveBuffer;
	memcpy(tempModel,pPoint+25,20);
	AtsInfo.TestFunc.SFIS.czModelPN.Format("%s",tempModel);
	AtsInfo.TestFunc.SFIS.czModelPN.Trim();
	//tmp.Format("ModelName:%s",FROM_SFIS.czModel);
	//nReTest =MessageBox(0,AtsInfo.TestFunc.SFIS.czModelPN, "請用滑鼠選擇", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
	nReTest = MessageBox(0, AtsInfo.TestFunc.SFIS.czModelPN, "please use mouse to selct", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
	//nReTest = ::AfxMessageBox(("測試結果失敗，是否重測?", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,0);
	//MessageBox(FROM_SFIS.czModel);
	if ( nReTest == IDNO )
	{
		CloseHandle(hRS232);
		exit(1);//20090428
	}
	CloseHandle(hRS232);
}
void Pause(int TimeMs)
{
	clock_t start,end;
	MSG msg;

	start = clock();
	do
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		end = clock();
	}
	while(((end-start) < TimeMs));
}

//----------------------------------------- SN  MAC  SSN1  SSN2 都在此处做了处理-----------------------------------------------------------------
UINT ThreadMainFlow(LPVOID pParam)
{
	int ret = 0, status = 0;
	int reTryTime = 0;
	CString text;
	BOOL nCheckInput = FALSE;

	::SendMessage(UI_hWnd, MSG_ENABLE_TEST_BUTTON, 0, 0);

	while( nRunningFlag )
	{
		/////////////////////////////////20090514///////////////////////////
		if(AtsInfo.m_ShowDect)//Tommy 20091124 DECT 顯示, 用ini 設定
		{
			status = STATUS_SHOW_DECT;
			::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
		}
	//	text.Format("請插入待測裝置...");
		text.Format("Please insert DUT...");
		GetLocalTime(&sTime);
//	    UiLogName.Format("%04d%02d%02d%02d%02d",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute);
		UiLogName.Format("%s", AtsInfo.csUserName);

		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&text,0);
		
            if(bEfficencyCalc)
			{
			if(bFirstStart)
				{
				bFirstStart = false;
				dwBeginTime = GetTickCount();
				dwTotalEfficiencyTime = 0;
				}
            if( (GetTickCount()-dwBeginTime) > 0 && (nPassCount+nFailCount) >0)
				{
				CString str;
				//str.Format("總測試次數:%d ,總測試時間:%d秒, 平均測試時間: %d秒,功能測試時間:%d秒,效率為:%d%%",\;
				str.Format("Total test:%d ,Total test time:%d seconds, Average test time: %d seconds,Function test time:%d seconds,Efficiency:%d%%", \
					nPassCount+nFailCount,\
					(GetTickCount()-dwBeginTime)/1000,\
					(GetTickCount()-dwBeginTime)/(1000*(nPassCount+nFailCount)),\
					dwTotalEfficiencyTime/1000,\
					(dwTotalEfficiencyTime*100)/(GetTickCount()-dwBeginTime));
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&str,0);
				
				}
			}
		/////////////////////////////////20090514///////////////////////////
		if( AtsInfo.m_bInputSN || AtsInfo.m_bInputMac ||
			AtsInfo.m_bInputSSN1 || AtsInfo.m_bInputSSN2 ) //手動輸入的情況
		{
			if( AtsInfo.m_bAutoMac )
			{
				CAutoInputKey AutoKey;
				strcpy(SFIS.czMAC, AutoKey.GetMac());
			}
			else
			{
				CMacDlg dlg;
				dlg.Create(UI_hWnd);
				dlg.DoModal();
				if(-1!=dlg.m_csMac.Find(":"))
				{
					dlg.m_csMac.Remove(':');
				}
				if (-1 != dlg.m_csSSN1.Find(":"))
				{
					dlg.m_csSSN1.Remove(':');
				}
				CString strMac = dlg.m_csMac;
				CString strSN = dlg.m_csSN;
				CString strSSN1 = dlg.m_csSSN1;
				CString strSSN2 = dlg.m_csSSN2;

				strcpy(SFIS.czSN, dlg.m_csSN);
				strcpy(SFIS.czMAC, dlg.m_csMac);
				strcpy(SFIS.czSSN1, dlg.m_csSSN1);
				strcpy(SFIS.czSSN2, dlg.m_csSSN2);

				if(!dlg.m_bMacLegal)
				{
					status = STATUS_SHOW_IDLE;
					::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
					::SendMessage(UI_hWnd, MSG_ENABLE_TEST_BUTTON, 1, 0);
					return 0;
				}
			}
		}
		nCheckInput = TRUE;
		int status = STATUS_SHOW_TEST;//20090514
		::SendMessage(UI_hWnd,MSG_SET_TEST_STATUS,(WPARAM)&status,0);//20090514
        
        if(!(AtsInfo.m_bAutoTest || AtsInfo.LoopTest.m_bLoopTest))
			{
			nRunningFlag = FALSE;
			}

		ThreadTester((LPVOID)1);
		
		while( AtsInfo.m_ipEmbedIP != "" && PingMe(AtsInfo.m_ipEmbedIP) &&nRunningFlag && nCheckInput )
		{
			//status = STATUS_SHOW_DECT;//20090514
			//::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);//20090514
		
			if( AtsInfo.m_ipEmbedIP != "" && !PingMe(AtsInfo.m_ipEmbedIP) && !AtsInfo.LoopTest.m_bLoopTest )
			{
				//text = "請插入待測裝置...";
				text = "Please insert DUT...";
				::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&text, 0);
			
			}
			else
			{
			
				status = STATUS_SHOW_TEST;
				::SendMessage(UI_hWnd,MSG_SET_TEST_STATUS,(WPARAM)&status,0);
				//ThreadTester((LPVOID)1);
				if((AtsInfo.m_bAutoTest || AtsInfo.LoopTest.m_bLoopTest))
					ThreadTester((LPVOID)1);
				nCheckInput = FALSE;
			}

			if( reTryTime++ >= AtsInfo.m_bootTime )
			{
				reTryTime = 1;
				//text = "待測裝置不良或不存在...";
				text = "DUT Fail or not exist...";
				::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&text, 0);
				strcpy(SFIS.czEC, "HW01");
				status = STATUS_SHOW_ERRCODE;
				::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
				if((AtsInfo.m_bAutoTest || AtsInfo.LoopTest.m_bLoopTest))
					break;
				else
				{
					::SendMessage(UI_hWnd, MSG_ENABLE_TEST_BUTTON, 1, 0);
					return 1;
				}
			}
			WinExec("arp -d", SW_HIDE);
			Pause(1500);
		}//while
		WinExec("arp -d", SW_HIDE);
		Pause(1000);
	}//while
	return 1;
}

UINT ThreadSFISIN(LPVOID pParam)
{
	char		errMsg[256];
	CString		showMsg;
	memset(errMsg,0,sizeof(errMsg));
	int status;
	while( nRunningFlag )
	{
		if(AtsInfo.m_ShowDect)//Tommy 20091124 DECT 顯示, 用ini 設定
		{
			status = STATUS_SHOW_DECT;
			::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
		}
		//showMsg.Format("請插入待測裝置......");
		showMsg.Format("Please insert DUT......");
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&showMsg,0);
		GetLocalTime(&sTime);
		if(bEfficencyCalc)
			{
			if(bFirstStart)
				{
				bFirstStart = false;
				dwBeginTime = GetTickCount();
				dwTotalEfficiencyTime = 0;
				}
            if( (GetTickCount()-dwBeginTime) > 0 && (nPassCount+nFailCount) >0)
				{
				CString str;
			//	str.Format("總測試次數:%d ,總測試時間:%d秒, 平均測試時間: %d秒,功能測試時間:%d秒,效率為:%d%%",\;
				str.Format("Total test:%d ,Total test time:%d seconds, Average test time: %d seconds,Function test time:%d seconds,Efficiency:%d%%",\
					nPassCount+nFailCount,\
					(GetTickCount()-dwBeginTime)/1000,\
					(GetTickCount()-dwBeginTime)/(1000*(nPassCount+nFailCount)),\
					dwTotalEfficiencyTime/1000,\
					(dwTotalEfficiencyTime*100)/(GetTickCount()-dwBeginTime));

				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&str,0);
			
				}
			}
//	    UiLogName.Format("%04d%02d%02d%02d%02d",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute);
		UiLogName.Format("%s", AtsInfo.csUserName);
		
		while( AtsInfo.m_ipEmbedIP != "" && !PingMe(AtsInfo.m_ipEmbedIP) && nRunningFlag )
		{
			WinExec("arp -d", SW_HIDE);
			Pause(1000);
		}

		if ( !nRunningFlag )
			return 0;

		retSFIS = SFISTestFunc(TRUE, FIRST_RECEIVE_SFIS_DATA);
		if (retSFIS == 0 || retSFIS == -9)
		{
			continue;
		}
		else if (retSFIS == -2)
		{
			::SendMessage(UI_hWnd, MSG_ENABLE_TEST_BUTTON, 1, 0);
			return 0;
		}

		m_LogCount=0;
		int status = STATUS_SHOW_TEST;
		::SendMessage(UI_hWnd,MSG_SET_TEST_STATUS,(WPARAM)&status,0);

		if(AtsInfo.m_startOnButton)
		{
			short key = 0 ;
		//	showMsg.Format("請押空白鍵開始測試...");
			showMsg.Format("Please press the space key to start test...");
			do
			{
 				//GetKeyboardState((LPBYTE)&keyState);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&showMsg,0);
   				key = GetAsyncKeyState(VK_SPACE);//偵測鍵盤狀態
				Sleep(500);
			}while(key == 0);
		}
		//AfxMessageBox("開始測試",MB_OK);
		for(int i=0;i<3;i++)
		{
			strANT[i].Empty();
		}
		ThreadTester((LPVOID)1);

		while( AtsInfo.m_ipEmbedIP != "" && PingMe(AtsInfo.m_ipEmbedIP) && nRunningFlag )
		{
			WinExec("arp -d", SW_HIDE);
			Pause(1500);
		}
		Pause(200);
	}

	return true;
}

UINT ThreadTester(LPVOID pParam)
{
	//MSG_SET_ITEM_STATUS

	CString temp;
	temp = "RESET";
	::SendMessage(UI_hWnd, MSG_SET_ITEM_STATUS, (WPARAM)&temp, 0);
	int nRet = 1;

	//::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	//表示 nRet == 1 時以3為參數代表寫PASS log
	//nRet == 0 時以4為參數寫FAIL log

	if(nRet)
	{
		nRet = FuncTest(AtsInfo.TestFunc.STATION);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	::SendMessage(UI_hWnd, MSG_TEST_END, (WPARAM)1, nRet ? TEST_PASS : TEST_FAIL);
	return nRet;
}

int SFISTestFunc(BOOL bTestResult,int nTestState)
{
	int		return_status = 1;
	CString		csInfo;
	CString 	ConvertMsg,showMsg;
	char* 	receiveBuffer = new char[4096];
	memset(receiveBuffer,0,sizeof(receiveBuffer));//
	COLORREF color1;

	Sfis_status = 0;


   

	if( nTestState == FIRST_RECEIVE_SFIS_DATA )
	{
		//showMsg.Format("████████ 請依序掃入 SN、MAC 條碼 ████████");
		showMsg.Format("████████ Please scan SN、MAC Barcode ████████");
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		/////////////////////////////////20090429/////////////////////////////////
		//Sfis_status = DetSFISmsg(&SFIS,receiveBuffer);
		DWORD  threadIdGetSFIS,theradIDGetInput,threadCheck;
		CMacDlg dlg;
		dlg.Create(UI_hWnd);
		if(AtsInfo.m_InputNum==1)
		{
			if(dlg.DoModal()==IDOK)
			{
				dlg.m_csSN.Trim();
				if(dlg.m_csSN.GetLength()!=AtsInfo.m_nSN_Length)
				{				
					::SendMessage(dlg.m_hWnd,WM_COMMAND,IDCANCEL,0);
					return FALSE;
				}
				strcpy(SFIS.czSN, dlg.m_csSN);
				//showMsg.Format("已輸入第一槍條碼:%s",SFIS.czSN);
				showMsg.Format("Input first Barcode:%s", SFIS.czSN);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			else
			{
					//showMsg.Format("!未掃入條碼信息!");
				    showMsg.Format("!Do not get any Barcde !");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					showMsg.Format("");
					COLORREF color0 = COLOR_GREEN;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color0);
					return FALSE;
			}
		}
		if(AtsInfo.m_InputNum==2)
		{
			if(dlg.DoModal()==IDOK)
			{
				showMsg.Format("SN Barcode:%s", dlg.m_csSN);
				::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
				strcpy(SFIS.czMAC, dlg.m_csMac);
				showMsg.Format("MAC Barcode:%s", dlg.m_csMac);
				::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
				dlg.m_csSN.Trim();
				if(dlg.m_csSN.GetLength()!=AtsInfo.m_nSN_Length)
				{				
					::SendMessage(dlg.m_hWnd,WM_COMMAND,IDCANCEL,0);
					return FALSE;
				}
				dlg.m_csMac.Remove(':');
				if(dlg.m_csMac.GetLength()!=AtsInfo.m_nMAC_Length)
				{				
					::SendMessage(dlg.m_hWnd,WM_COMMAND,IDCANCEL,0);
					return FALSE;
				}
				strcpy(SFIS.czSN, dlg.m_csSN);
				//if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RC"))
				//{
				//	strcpy(SFIS.czSSN1,SFIS.czSN);
				//}
				showMsg.Format("SFIS SN:%s", SFIS.czSN);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
				strcpy(SFIS.czMAC,dlg.m_csMac);
				showMsg.Format("SFIS MAC:%s",SFIS.czMAC);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			else
			{
					//showMsg.Format("!未掃入條碼信息!");
					showMsg.Format("!Do not get any Barcde !");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					showMsg.Format("");
					COLORREF color0 = COLOR_GREEN;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color0);
					return -2;
			}
		}
		if(AtsInfo.m_InputNum==3)
		{
			if(dlg.DoModal()==IDOK)
			{
				dlg.m_csSN.Trim();
				if(dlg.m_csSN.GetLength()!=AtsInfo.m_nSN_Length)
				{				
					::SendMessage(dlg.m_hWnd,WM_COMMAND,IDCANCEL,0);
					return FALSE;
				}
				if(dlg.m_csMac.GetLength()!=AtsInfo.m_nMAC_Length)
				{				
					::SendMessage(dlg.m_hWnd,WM_COMMAND,IDCANCEL,0);
					return FALSE;
				}
				if(dlg.m_csSSN1.GetLength()!=AtsInfo.m_nSSN1_Length)
				{
					::SendMessage(dlg.m_hWnd,WM_COMMAND,IDCANCEL,0);
					return FALSE;
				}
				strcpy(SFIS.czSN, dlg.m_csSN);
				//showMsg.Format("已輸入第一槍條碼:%s",SFIS.czSN);
				showMsg.Format("First Barcode:%s", SFIS.czSN);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
				strcpy(SFIS.czMAC,dlg.m_csMac);
				//showMsg.Format("已輸入第二槍條碼:%s",SFIS.czMAC);
				showMsg.Format("Second Barcode:%s", SFIS.czMAC);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
				strcpy(SFIS.czSSN1,dlg.m_csSSN1);
				//showMsg.Format("已輸入第三槍條碼:%s",SFIS.czSSN1);
				showMsg.Format("Third Barcode:%s", SFIS.czSSN1);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			else
			{
					//showMsg.Format("!未掃入條碼信息!");
					showMsg.Format("!Do not get any Barcde !");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					showMsg.Format("");
					COLORREF color0 = COLOR_GREEN;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color0);
					return FALSE;
			}
		}
/*		HANDLE hGetSFIS=CreateThread( NULL,
								0,
								ThreadTestGetSFIS,
								closeCATSDlg,
								0,
								&threadIdGetSFIS);
		if(!threadIdGetSFIS)
		{
			showMsg.Format("Call GetSFIS Function Fail");
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			return FALSE;
		}
		else
		{
			WaitForSingleObject(hGetSFIS,INFINITE) ;
		}
		sprintf(receiveBuffer,"%s",receiveBufferThread);
*/		/////////////////////////////////20090429/////////////////////////////////
		//做SFIS structure的去空白處理
		CString str;
		str=SFIS.czSN;
		str.Trim();
		strcpy(SFIS.czSN, str);

		str=SFIS.czMAC;
		str.Trim();
		strcpy(SFIS.czMAC, str);

		str=SFIS.czSSN1;
		str.Trim();
		strcpy(SFIS.czSSN1, str);

		str=SFIS.czSSN2;
		str.Trim();
		strcpy(SFIS.czSSN2, str);

/*		showMsg = "";
		COLORREF color1 = COLOR_GREEN;
		::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
		if(Sfis_status != 1)
		{
			CString rcvMsg;
			rcvMsg.Format("%s",receiveBuffer);
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&rcvMsg, 0);
			str=rcvMsg;
			rcvMsg=str.Right(str.GetLength()-str.ReverseFind(':'));
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&rcvMsg, 0);
			if(Sfis_status == 2)
			{
				showMsg.Format("! # 等待MES回應超時 #!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			if(Sfis_status == 3)
			{
				showMsg.Format("! #MES回應不可在本站測試#!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			if(Sfis_status == 5)
			{
				showMsg.Format("!MES回應無PASS,FAIL 信息!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			return FALSE;
		}

		if(!Sfis_status)
		{
			showMsg.Format("!Read MES Information Fail!!");
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		}
		csInfo.Format("(%d)%s",strlen(receiveBuffer),receiveBuffer);
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);
	///////////////////////////////////////////////////20090428////////////////////////////////////////
		//CString str;
		str.Format(SFIS.czPN);
		str.Trim();
		if(strlen(AtsInfo.TestFunc.SFIS.czModelPN)==0)
		{
			AtsInfo.TestFunc.SFIS.czModelPN=str;
		}
		if(strcmp(AtsInfo.TestFunc.SFIS.czModelPN,str) != 0)
		{
			showMsg.Format("!PN 比對不符合!:D_PN:%s,C_PN:%s",AtsInfo.TestFunc.SFIS.czModelPN,str);
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			return FALSE;
		}
*/		//if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT")||-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RC"))
		//{
		//	Sfis_status=0;
		//	HANDLE hCheckSFIS=CreateThread( NULL,
		//							0,
		//							ThreadCheckSFIS,
		//							closeCATSDlg,
		//							0,
		//							&threadCheck);
		//	if(!threadCheck)
		//	{
		//		showMsg.Format("Call GetSFIS Function Fail");
		//		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		//		return FALSE;
		//	}
		//	else
		//	{
		//		WaitForSingleObject(hCheckSFIS,INFINITE) ;
		//	}
		//	sprintf(receiveBuffer,"%s",BufferCheck);
		//	showMsg.Format("%s",receiveBuffer);
		//	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		//	if(Sfis_status != 1)
		//	{
		//		CString rcvMsg;
		//		color1=COLOR_RED;
		//		if(Sfis_status == 2)
		//		{
		//			showMsg.Format("! # 等待MES回應超時 #!");
		//			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		//			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&showMsg,(LPARAM)&color1);
		//		}
		//		if(Sfis_status == 6)
		//		{
		//			showMsg.Format("! #MES回應條碼不可在本站測試#!");
		//			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		//			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&showMsg,(LPARAM)&color1);
		//		}
		//		if(Sfis_status == 5)
		//		{
		//			showMsg.Format("!MES回應無PASS,FAIL 信息!");
		//			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		//			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&showMsg,(LPARAM)&color1);
		//		}
		//		return FALSE;
		//	}
		//}
		//////////////////////////////////////////////////20090428/////////////////////////////////////////////
	}
	else
	{
		//////////////////////////20090219/////////////////////////////////
		if(!bTestResult)
		{
			//::SetForegroundWindow(UI_hWnd);
			//Sleep(500);
			CRect rc;
			POINT orglpoinp,lpoinp;
			::GetWindowRect(UI_hWnd,rc);
			::SetForegroundWindow(UI_hWnd);
			::UpdateWindow(UI_hWnd);
			::SetFocus(UI_hWnd);
			::GetCursorPos(&orglpoinp);
			lpoinp.x = rc.left;
			lpoinp.y = rc.top;
			SetCursorPos(lpoinp.x+50,lpoinp.y+20);
			mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
			//::SetForegroundWindow(UI_hWnd);
			Sleep(500);
		}
		///////////////////////////////////////////////////////////////
		//////////////////////////////////////////////20090428////////////////////////////////////////////////
		else
		{
			if(AtsInfo.TestFunc.SFIS.nCheckSFIS_InfoFS)
			{
				memset(&secondSFIS, 0, sizeof(secondSFIS)); //清除這次測試的shopfloor資料
				//showMsg.Format("# 請種新掃入 SN、MAC 條碼 #");
				showMsg.Format("# Please rescan SN、MAC Barcode #");
				color1 = COLOR_YELLOW;
				::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
				//SFIS_PM	secondSFIS;
				//showMsg.Format("██ 請種新掃入 SN、MAC 條碼 ██");
				showMsg.Format("██ Please rescan SN、MAC Barcode ██");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
				Sleep(100);
				//Sfis_status = DetSFISmsg(&secondSFIS,receiveBuffer);
				/////////////////////////////////20090429/////////////////////////////////
				//Sfis_status = DetSFISmsg(&SFIS,receiveBuffer);
				DWORD  threadIdGetSFIS_S;
				HANDLE hGetSFIS_S=CreateThread(NULL,
										4096,
										ThreadTestGetSFIS,
										closeCATSDlg,
										0,
										&threadIdGetSFIS_S);
				if(!threadIdGetSFIS_S)
				{
					showMsg.Format("Call GetSFIS Function Fail");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					return FALSE;
				}
				else
				{
					WaitForSingleObject(hGetSFIS_S,INFINITE);
				}
				sprintf(receiveBuffer,"%s",receiveBufferThread);
				/////////////////////////////////20090429/////////////////////////////////
				//做SFIS structure的去空白處理
				CString str;
				str=secondSFIS.czSN;
				str.Trim();
				strcpy(secondSFIS.czSN, str);

				str=secondSFIS.czMAC;
				str.Trim();
				strcpy(secondSFIS.czMAC, str);

				str=secondSFIS.czSSN1;
				str.Trim();
				strcpy(secondSFIS.czSSN1, str);

				str=secondSFIS.czSSN2;
				str.Trim();
				strcpy(secondSFIS.czSSN2, str);

				showMsg = "";
				COLORREF color1 = COLOR_GREEN;
				::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
				if(Sfis_status != 1)
				{
					CString rcvMsg;
					rcvMsg.Format("%s",receiveBuffer);
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&rcvMsg, 0);
					if(Sfis_status == 2)
					{
						//showMsg.Format("! # 等待輸入超時 #!");
						showMsg.Format("! # Waiting for timeout #!");
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					}
					memset(&secondSFIS, 0, sizeof(secondSFIS)); //清除這次測試的shopfloor資料
					return FALSE;
				}

				if(!Sfis_status)
				{
					showMsg.Format("!Read ComPort Fail!!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					memset(&secondSFIS, 0, sizeof(secondSFIS)); //清除這次測試的shopfloor資料
					return FALSE;
				}
				csInfo.Format("(%d)%s",strlen(receiveBuffer),receiveBuffer);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);
				//Sleep(100);
				////////////////////////////////////////////////////////////////////////////////////////////////
				if(strcmp(secondSFIS.czSN,SFIS.czSN) != 0 || strcmp(secondSFIS.czMAC,SFIS.czMAC) != 0
					 || strcmp(secondSFIS.czSSN1,SFIS.czSSN1) != 0 || strcmp(secondSFIS.czSSN2,SFIS.czSSN2) != 0)
				{
					if(strcmp(SFIS.czSN,secondSFIS.czSN) != 0)
					{
						//showMsg.Format("!SN比對不符合!:F_SN:%s,S_SPN:%s",SFIS.czSN,secondSFIS.czSN);
						showMsg.Format("!SN not match!:F_SN:%s,S_SPN:%s", SFIS.czSN, secondSFIS.czSN);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						//Sleep(100);
					}
					if(strcmp(SFIS.czMAC,secondSFIS.czMAC) != 0)
					{
						//showMsg.Format("!MAC比對不符合!:F_MAC:%s,S_MAC:%s",SFIS.czMAC,secondSFIS.czMAC);
						showMsg.Format("!MAC not match!:F_MAC:%s,S_MAC:%s", SFIS.czMAC, secondSFIS.czMAC);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						Sleep(100);
					}
					if(strcmp(SFIS.czSSN1,secondSFIS.czSSN1) != 0)
					{
						//showMsg.Format("!SSN1比對不符合!:F_SSN1:%s,S_SSN1:%s",SFIS.czSSN1,secondSFIS.czSSN1);
						showMsg.Format("!SSN1 not match!:F_SSN1:%s,S_SSN1:%s", SFIS.czSSN1, secondSFIS.czSSN1);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						//Sleep(100);
					}
					if(strcmp(SFIS.czSSN2,secondSFIS.czSSN2) != 0)
					{
						//showMsg.Format("!SSN2比對不符合!:F_PN:%s,S_PN:%s",SFIS.czSSN2,secondSFIS.czSSN2);
						showMsg.Format("!SSN2 not match!:F_PN:%s,S_PN:%s", SFIS.czSSN2, secondSFIS.czSSN2);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						//Sleep(100);
					}
					//showMsg.Format("#測試前后SFIS回傳資料不符合#");
					showMsg.Format("#SFIS message not match#");
					color1 = COLOR_RED;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
					Sleep(100);
					memset(&secondSFIS, 0, sizeof(secondSFIS)); //清除這次測試的shopfloor資料
					return -9;
				}
			}
		}
		//傳2代表這裡是要進行FT站位的Link
	}
	

		///////////////////////////////////////////20090428///////////////////////////////////////////////////////
		////strcpy(SFIS.czEND, "END");
		////showMsg.Format("# 上傳DUT資訊至SFIS系統,等待SFIS回應... #");
		////::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////Sfis_status = SendMsgSFIS(bTestResult,SFIS.czFW,SFIS.czEC,&SFIS,receiveBuffer);
		////if(Sfis_status == 0 && strstr(receiveBuffer, "retest") != NULL )
		////{
		////	return_status = 5;
		////	showMsg.Format("重新測試，取消上傳DUT資訊");
		////	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////}
		////else if(Sfis_status == 0)
		////{
		////	showMsg.Format("!Send SFIS file error!!");
		////	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////}
		////else
		////{
		////	csInfo.Format("(%d)%s",strlen(receiveBuffer),receiveBuffer);
		////	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);

		////	Sfis_status = DetSFISmsg(&SFIS,receiveBuffer);


		////	csInfo.Format("(%d)%s",strlen(receiveBuffer),receiveBuffer);
		////	switch(Sfis_status)
		////	{
		////		case 0:
		////			showMsg.Format("!Read SFIS ComPort Fail!!");
		////			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////			color1 = COLOR_RED;
		////			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
		////			return_status = 0;
		////			break;
		////		case 1:
		////			Pause(1000);
		////			if( bTestResult == FALSE )
		////			{
		////				showMsg.Format("# SFIS回應[OK]");
		////				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);
		////				return_status = 4;
		////			}
		////			else
		////			{

		////				showMsg.Format("# SFIS回應[OK]");
		////				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);//Tommy debug
		////				return_status = 1;
		////			}
		////			color1 = COLOR_GREEN;
		////			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
		////			break;
		////		case 2:
		////			showMsg.Format("! SFIS系統無回應!");
		////			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);//Tommy debug
		////			color1 = COLOR_RED;
		////			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
		////			return_status = 2;
		////			break;
		////		case 3:
		////			Pause(1000);
		////			showMsg.Format("! SFIS回應DUT訊息不合規格!");
		////			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		////			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);
		////			color1 = COLOR_RED;
		////			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
		////			return_status = 3;
		////			break;
		////		case 5:
		////			return_status = 5;
		////			break;
		////		default:
		////			break;
		////	}
		////}
	//}
	delete [] receiveBuffer;
	return return_status;
}


int FuncTest(TEST_FUNC testFunction)
{
	bool testPass = 0;
	CString temp;
	typedef bool (WINAPI* lpInitDut)(SFIS_PM *);
	typedef bool (WINAPI* lpStartTest)(void );
	typedef bool (WINAPI* lpTestEnd)(void );
	lpInitDut InitDut;
	lpStartTest StartTest;
	lpTestEnd TestEnd;

	AtsInfo.dwTimeStart = 0;
	AtsInfo.dwTimeTotal = 0;
	
 
	//  monitor retest rate and if it is more than threshould , then will prompt engineer to check test environment
	if(!AtsInfo.m_RetestMSG.m_csAlarmMSG.IsEmpty() && (nPassCount+nFailCount) >0 )
	{
	if( (nFailCount+nPassCount) >= AtsInfo.m_RetestMSG.m_nTotalRetestCounter)
		{
		
		if( (nFailCount*100)/(nPassCount+nFailCount) > AtsInfo.m_RetestMSG.m_RetesRateThreshold)
			{
			// show Prompt diaglog 

			MyMessageBox dlgAlarm;
			CString str;
			//str.Format("當前重測率偏高:%%%2.1f, FAIL數量:%d 總測試數量:%d\n ",(float)(nFailCount*100)/(float)(nPassCount+nFailCount),nFailCount,(nPassCount+nFailCount));
			str.Format("Retest rate is high:%%%2.1f, FAIL:%d Total test:%d\n ", (float)(nFailCount * 100) / (float)(nPassCount + nFailCount), nFailCount, (nPassCount + nFailCount));
			if(str.GetLength() > AtsInfo.m_RetestMSG.m_Column)
				{
				AtsInfo.m_RetestMSG.m_Column = str.GetLength();
				}
			str += AtsInfo.m_RetestMSG.m_csAlarmMSG;
			dlgAlarm.vSetAlarmMessage(str,(AtsInfo.m_RetestMSG.m_num+1),AtsInfo.m_RetestMSG.m_Column);
			dlgAlarm.DoModal();

			}
		nFailCount = 0;
        nPassCount = 0;
		 
		}
	}	
     GetLocalTime(&sStartTime);
	InitDut = (lpInitDut)GetProcAddress(hDLL,"InitDut");
	StartTest = (lpStartTest)GetProcAddress(hDLL,"StartTest");
	TestEnd = (lpTestEnd)GetProcAddress(hDLL,"TestEnd");
	
	AtsInfo.dwTimeStart = ::GetTickCount();

    testPass = InitDut(&SFIS);
    if(bEfficencyCalc)
		{
		dwEfficiencyBeginTime = GetTickCount();
		}
	
	//此處獲得焦點

	//只有InitDut() Pass後才做StartTest()
	if ( testPass )
		{
		testPass = StartTest();
		}
	 if(bEfficencyCalc)
		{
		dwTotalEfficiencyTime +=(GetTickCount()-dwEfficiencyBeginTime);
		}
	//此處釋放焦點

	AtsInfo.dwTimeTotal = GetTickCount() - AtsInfo.dwTimeStart;

	temp.Format("Total test time : %d seconds.", AtsInfo.dwTimeTotal/1000);
	::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&temp, 0);
	GetLocalTime(&sEndTime);

	//不論是否PASS都要做TestEnd()
	TestEnd();
	
	HWND hWnd;
	hWnd = ::FindWindow(NULL,AtsInfo.TerminalNameSFIS.GetBuffer());

	if(hWnd)
		{
		::SetActiveWindow(hWnd);
		::BringWindowToTop(hWnd);
		::ShowWindow(hWnd,SW_SHOWNORMAL);
	    ::SetForegroundWindow(hWnd);
		::SetFocus(hWnd);
		}

	return testPass;
}

UINT HandlePreInit(LPVOID pParam)
{
	/* 8/6/08 Ryan:To add PreInit(); Call PreInit() of all enabled functions. */

	bool testPass = 0;
	typedef bool (WINAPI* lpPreInit)(HWND, DEVICE_HANDLE*);
	lpPreInit PreInit;
	CString temp1;

	if ( !AtsInfo.TestFunc.STATION.csStationName.IsEmpty())
	{
		if(0 == pParam)
			{
			hDLL = ::LoadLibrary(AtsInfo.TestFunc.STATION.m_DllName);
			}
		else
			{
			typedef void (WINAPI* lpTerminal)();
			lpTerminal Terminal;
			Terminal = (lpTerminal)GetProcAddress(hDLL,"Terminal");
			Terminal();
			Sleep(5000);
			}
		PreInit = (lpPreInit)GetProcAddress(hDLL,"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
 		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	//CString temp = "DLL初始作業完成.";
	CString temp = "DLL initialize complete.";
	::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&temp, 0);

	char errMsg[200];
	//判斷DLL是不是把UI設定為GOLDEN
	if ( !AtsInfo.m_bServe )
	{
/*		if( AtsInfo.TestFunc.SFIS.m_TestEnable )
		{
			if(0 == pParam)
			{
				if(!InitSFIS(errMsg))
				{
				temp.Format(errMsg);
				::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&temp, 0);
				return FALSE;
				}
			}
		}
*/		::SendMessage(UI_hWnd, MSG_ENABLE_TEST_BUTTON, 1, 0);
	}
	else //這邊是跑Golden流程
		::SendMessage(UI_hWnd, MSG_TEST_START, 0, 0);

	return 0;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CATSDlg dialog


CATSDlg::CATSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CATSDlg::IDD, pParent)
	, m_appVersion(_T(""))
{
	CATSApp *pApp = (CATSApp * )AfxGetApp();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_appVersion = pApp->m_AppVersionString;
	m_nTestingItem = 0;
	AtsInfo.LoopTest.m_iCounter = 1;
}


void CATSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_APP_VERSION, m_appVersion);
	DDX_Control(pDX, IDC_LAB_TEST_STATUS, m_testStatus);
	DDX_Control(pDX, IDC_SHOW_ACT_MESSAGE, m_ActMessage);
	DDX_Control(pDX, IDTEST, m_testbutton);
	DDX_Control(pDX, IDC_SFIS_MEG, m_SFISMessage);
	//Ryan:用回圈做DDX
	for ( int i = 0 ; i < 30 ; i++ )
	{
		DDX_Control(pDX, IDC_TESTITEM_1+i, m_testitem[i]);
	}
	DDX_Control(pDX, IDC_UI_MSG, m_list_msg);
	DDX_Control(pDX, IDC_TOTAL_COUNT, m_total_count);
	DDX_Control(pDX, IDC_FAIL_COUNT, m_fail_count);
	DDX_Control(pDX, IDC_PASS_COUNT, m_pass_count);
	DDX_Control(pDX, IDC_IQExpire, m_IQExpire);
	DDX_Control(pDX, IDC_IQFlash, m_IQFlash);
	//DDX_Control(pDX, IDC_EditSN, m_EditSN);
	//DDX_Control(pDX, IDC_EditMAC, m_EditMAC);
	//DDX_Control(pDX, IDC_EditSSN1, m_EditSSN1);
}

BEGIN_MESSAGE_MAP(CATSDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MSG_SHOW_MESSAGE, OnShowMessage)
	ON_MESSAGE(MSG_SHOW_ACT_MESSAGE, OnShowActMessage)
	ON_MESSAGE(MSG_TEST_START, OnTestStart)
	ON_MESSAGE(MSG_TEST_END, OnTestEnd)
	ON_MESSAGE(MSG_SHOW_GOLDEN, OnShowGolden)
	ON_MESSAGE(MSG_SET_ITEM_STATUS, OnSetDutItemStatus)
	ON_MESSAGE(MSG_SET_GOLDEN_ITEM, OnSetGoldenItemStatus)
	ON_MESSAGE(MSG_SEND_LOG_MESSAGE, OnLogMessage)
	ON_MESSAGE(MSG_ENABLE_TEST_BUTTON, OnEnableTestButton)
	ON_MESSAGE(MSG_SET_TEST_STATUS, OnSetTestStatus)
	ON_MESSAGE(MSG_DELETE_CLASS, OnDeleteObject)
	//ON_MESSAGE(MSG_ENABLE_LABLE_INPUT,OnSetLabelStatus)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDTEST, &CATSDlg::OnStartBtn)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
//	ON_BN_CLICKED(IDC_BUTTON_HELP, &CATSDlg::OnBnClickedButtonHelp)
//	ON_BN_DOUBLECLICKED(IDC_BUTTON_HELP, &CATSDlg::OnBnDoubleclickedButtonHelp)
//ON_BN_CLICKED(IDC_BUTTON_HELP, &CATSDlg::OnBnClickedButtonHelp)
ON_BN_CLICKED(IDC_BUTTON_HELP, &CATSDlg::OnBnClickedButtonHelp)
ON_WM_CTLCOLOR()
ON_STN_CLICKED(IDC_SFIS_MEG, &CATSDlg::OnStnClickedSfisMeg)
ON_EN_CHANGE(IDC_UI_MSG, &CATSDlg::OnEnChangeUiMsg)
ON_STN_CLICKED(IDC_PASS_COUNT, &CATSDlg::OnStnClickedPassCount)
END_MESSAGE_MAP()




// CATSDlg message handlers

BOOL CATSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	//Tommy 偵測是否有相同的程式正在執行, 避免開啟兩個
	//if(FindWindow("#32770", "NexComATS MFG Utility"))
	//{
	//	AfxMessageBox("系統中已經有重複的程式正在執行", MB_OK, 0);
	//	exit(0);
	//}



	//===============Add Pass and Fail Status Show============Jane/2013/03/12
	bTestMode=DEBUG_MODE;
	if(!bTestMode)
	{
		GetDlgItem(IDC_UI_MSG)->ShowWindow(SW_HIDE);
	}
	CRect rc;
	::GetWindowRect(GetDlgItem(IDC_UI_MSG)->GetSafeHwnd(),&rc);
	ScreenToClient(&rc);
	cFont.CreatePointFont(200,"宋體");
	//m_TestStateCtrl.Create("請掃條碼開始測試！",WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE|SS_CENTER,rc,AfxGetApp()->GetMainWnd(),IDC_SHOWSTATE);
	m_TestStateCtrl.Create("Please scan the Barcode and start test！", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER, rc, AfxGetApp()->GetMainWnd(), IDC_SHOWSTATE);
	m_TestStateCtrl.SetFontName("宋體");
	m_TestStateCtrl.SetFontSize(40);
	if(bTestMode)
	{
		m_TestStateCtrl.ShowWindow(SW_SHOW);
	}
	else
	{
		m_TestStateCtrl.ShowWindow(SW_HIDE);
	}

	//===============Add Pass and Fail Status Show============Jane/2013/03/12
	//end
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	

	char errMsg[MAX_PATH];
	SetTestStatus(STATUS_SHOW_IDLE);
	UI_hWnd = m_hWnd;
	LoadATSini(); //讀INI檔
	//CCModelSel cModel;//模式选择对话框，此处要生成模式选择对话框
	//AtsInfo.m_csModelName=cModel.DisplayDlg();  //John add for select models
	char iniFileName[MAX_PATH];
	CString csLogPathTemp;
	::GetCurrentDirectory(256, errMsg);
	sprintf_s(iniFileName, "%s\\ATSCfg.ini", errMsg);
	::WritePrivateProfileString("DutConfig", "ProductName", AtsInfo.m_csModelName, iniFileName);

	DWORD NameSize = 128;
	TCHAR infoBuf[128];
	GetComputerName(infoBuf,&NameSize);
	AtsInfo.Log.csComputerName.Format("%s",infoBuf);
	ShowMessage("Computer Name:"+AtsInfo.Log.csComputerName);
	DeviceHandle->dev0 = NULL;
	DeviceHandle->dev1 = NULL;
	DeviceHandle->dev2 = NULL;
	DeviceHandle->dev3 = NULL;
	DeviceHandle->dev4 = NULL;
	DeviceHandle->dev5 = NULL;
	DeviceHandle->dev6 = NULL;
	DeviceHandle->dev7 = NULL;
	DeviceHandle->dev8 = NULL;
	DeviceHandle->dev9 = NULL;
	DeviceHandle->dev10 = NULL;
	DeviceHandle->dev11 = NULL;
	DeviceHandle->dev12 = NULL;
	DeviceHandle->dev13 = NULL;

	//------ 查檢Server上是否有更新的DLL檔案可更新 ------
	//------ 未完成 ------
	//if(AtsInfo.m_FTP)
	//{
	//	CString		Loadfile;
	//	CFTP_Client		pFTP;
	//	if(!pFTP.ConnectFTPServer("10.132.48.71","nsgdcc","nsgdcc168!",AtsInfo.m_csModelName))
	//		exit(1);
	//}
	//------

	memset(&SFIS, 0, sizeof(SFIS));
	if(!LoadTestItem())
	{
		nInitialLoadIsOk = FALSE;
		ShowMessage("Please close application and check library is exist or version is correct.");
		SetTestStatus(STATUS_SHOW_FAIL);
		m_testbutton.EnableWindow(1);

		return FALSE;
	}

	CString csTmp, csTmp2;
	char tmp[10];
	//------ 初始化一些參數 ------
	nPassCount = 0;
	nFailCount = 0;
	nTotalCount = 0;

	m_pass_count.SetWindowTextA(itoa(nPassCount, tmp, 10));
	m_fail_count.SetWindowTextA(itoa(nFailCount, tmp, 10));
	m_total_count.SetWindowTextA(itoa(nTotalCount, tmp, 10));

	//建立Log目錄
	
//	csTmp.Format("%s",AtsInfo.Log.csLogFilePath);
//	::CreateDirectory(csTmp, NULL);
	csTmp.Format("%sModels\\%s\\Log",AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);

//	::CreateDirectory("Log", NULL);     //tree0316
	::CreateDirectory(csTmp, NULL);
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		::SetFileAttributesA(csTmp,FILE_ATTRIBUTE_HIDDEN);
	}

	//tree MD
//	csTmp.Format("%sLog\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
//	::CreateDirectory(csTmp, NULL);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	if(AtsInfo.TestFunc.STATION.csStationName.IsEmpty())
	{
		ShowMessage("Please close application and check station name is exist or version is correct.");
	}
	else
	{
		//csTmp.Format("%sLog\\%s\\BACKUP", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		csTmp.Format("%sModels\\%s\\Log\\BACKUP", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		::CreateDirectory(csTmp, NULL);
		csTmp2 = csTmp + "\\PASS";
		::CreateDirectory(csTmp2, NULL);
		csTmp2 = csTmp + "\\FAIL";
		::CreateDirectory(csTmp2, NULL);
		csTmp2 = csTmp + "\\UI_Log";
		::CreateDirectory(csTmp2, NULL);
	//	csTmp.Format("%sLog\\%s\\Log_Upload_Info", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		csTmp.Format("%sModels\\%s\\Log\\Log_Upload_Info", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		::CreateDirectory(csTmp, NULL);
	//	csTmp.Format("%sLog\\%s\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,AtsInfo.TestFunc.STATION.csStationName);
		csTmp.Format("%sModels\\%s\\Log\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName, AtsInfo.TestFunc.STATION.csStationName);
		::CreateDirectory(csTmp, NULL);
		/////////////////////////////////20090427///////////////////////////////////////////
		if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
		{
			csTmp.Format("Log\\%s\\%s\\%s", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN,AtsInfo.TestFunc.STATION.csStationName);
			::CreateDirectory(csTmp, NULL);
		}
		////////////////////////////////20090427/////////////////////////////////////////////
		csTmp2 = csTmp + "\\PASS";
		::CreateDirectory(csTmp2, NULL);
		csTmp2 = csTmp + "\\FAIL";
		::CreateDirectory(csTmp2, NULL);
	}
	csLogPath = csTmp;
	ShowMessage("Local Log Path:"+csLogPath);
	csTmp.Format("DATAOUT_BACKUP");
	::CreateDirectory(csTmp, NULL);
	GetDlgItem(IDC_Computer_Name1)->SetWindowTextA(AtsInfo.Log.csComputerName);
	GetDlgItem(IDC_DLL_INFO)->SetWindowTextA(m_dllInfo);
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		csTmp.Empty();
//		GetDlgItem(IDC_SFIS_MEG)->ShowWindow(SW_HIDE);
		//=========for JiangYang,Send Log to MES============Jane//
		GetDlgItem(IDC_Log_Server)->SetWindowTextA("Log Server Address: "+AtsInfo.Log.csLogMappingDriver);
//		GetDlgItem(IDC_Log_Time)->SetWindowTextA("Upload Time: "+AtsInfo.Log.csLogUploadTime+" "+AtsInfo.Log.csLogUploadTime1);
		GetDlgItem(IDC_UPLOAD_STATUS)->SetWindowTextA("Upload Status: Connecting...");
		ShowMessage(AtsInfo.Log.csLogServerPath);
//		mytimethread = ::AfxBeginThread(HandleLogUpload, (LPVOID)0);
		DWORD rslt;
		exec("net start \"computer browser\"");
		OnUnMountNetDriver();
		rslt = OnMountNetDriver();
		if (rslt)
		{
			//AfxMessageBox("無法連接Log Server，請聯繫架設.", MB_OK, 0);
			AfxMessageBox("can not connect Log Server，please contact engineer.", MB_OK, 0);
			exit(0);
		}
		else
		{
		//	csTmp.Format("Upload Status: 已與伺服器正常連接,可以正常上傳");
			csTmp.Format("Upload Status: connected to server,can upload normally");
			GetDlgItem(IDC_UPLOAD_STATUS)->SetWindowTextA(csTmp);
		}

	}
	else
	{
		GetDlgItem(IDC_Log_Server)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_Log_Time)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_STATUS)->ShowWindow(SW_HIDE);
		m_SFISMessage.SetFontBold(TRUE);
		m_SFISMessage.SetFontName("Tahoma");
		m_SFISMessage.SetFontSize(26);
		m_SFISMessage.SetTextColor(RGB(255,0,0));
		m_SFISMessage.SetText("SFIS OFF");
	}
	//清除並初始化log structure
	STATION_Log.ResetAll();

	nInitialLoadIsOk = TRUE;

	m_testbutton.EnableWindow(FALSE);
	//ShowMessage("正在處理DLL初始程序.");
	ShowMessage("Initializing DLL....");
	//處理所有DLL的PreInit程序，完成後才會把Start button enable
	CWinThread *mythread = ::AfxBeginThread(HandlePreInit, (LPVOID)0);
	
	//IQ Expire Prompt
	m_IQExpire.EnableWindow(FALSE);
	m_IQExpire.ShowWindow(SW_HIDE);
	m_IQFlash.EnableWindow(FALSE);
	m_IQFlash.ShowWindow(SW_HIDE);
	if(AtsInfo.TestFunc.SFIS.nIQExpire_Enable)
	{
		nInitIQ=ThreadGetIQExpire();
		m_IQExpire.EnableWindow(TRUE);
		m_IQExpire.ShowWindow(SW_SHOWNORMAL);
		if(nInitIQ>1)
		{
			//csTmp.Format("IQ到期剩餘:%d天",nInitIQ);
			csTmp.Format("IQ:%ddays", nInitIQ);
			m_IQExpire.SetWindowText(csTmp);
			m_IQExpire.SetBkColor(RGB(0,255,0),0,CLabel::BackFillMode::Normal);
		}
		else
		{
			m_IQFlash.EnableWindow(TRUE);
			m_IQFlash.ShowWindow(SW_SHOWNORMAL);
			SetTimer(FLASH_IQSIGNAL,1000,NULL);
			//csTmp.Format("IQ即將到期");
			csTmp.Format("IQ are going timeout");
			m_IQExpire.SetWindowText(csTmp);
			m_IQExpire.SetBkColor(RGB(255,0,0),0,CLabel::BackFillMode::Normal);
		}
	}
	//SJ要求, SFIS disable測試, 需要輸入password
	/*if ( !AtsInfo.m_bServe && AtsInfo.TestFunc.SFIS.m_TestEnable != 1 )
	{
		do
		{
			AtsInfo.csSFISPassword = "";
			SFISOFF.DoModal();
			if(AtsInfo.csSFISPassword == "")
				exit(0);

			if(!AtsInfo.csSFISPassword.Compare(SFIS_OFF_Password))
			{
				break;
			}
			else
				continue;

		}while(1);
		
	}*/
		csTotalMes.Empty();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CATSDlg::ReInitSystem(void)
{
    char errMsg[256];
	CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
	SetTestStatus(STATUS_SHOW_IDLE);
	p->SetWindowTextA("");
	LoadATSini(); //讀INI檔
	CCModelSel cModel;
	AtsInfo.m_csModelName = cModel.DisplayDlg();
	DWORD NameSize = 128;
	TCHAR infoBuf[128];
	GetComputerName(infoBuf,&NameSize);
	AtsInfo.Log.csComputerName.Format("%s",infoBuf);
	ShowMessage("Computer Name:"+AtsInfo.Log.csComputerName);
	DeviceHandle->dev0 = NULL;
	DeviceHandle->dev1 = NULL;
	DeviceHandle->dev2 = NULL;
	DeviceHandle->dev3 = NULL;
	DeviceHandle->dev4 = NULL;
	DeviceHandle->dev5 = NULL;
	DeviceHandle->dev6 = NULL;
	DeviceHandle->dev7 = NULL;
	DeviceHandle->dev8 = NULL;
	DeviceHandle->dev9 = NULL;
	DeviceHandle->dev10 = NULL;
	DeviceHandle->dev11 = NULL;
	DeviceHandle->dev12 = NULL;
	DeviceHandle->dev13 = NULL;

	//------ 查檢Server上是否有更新的DLL檔案可更新 ------
	//------ 未完成 ------
	//if(AtsInfo.m_FTP)
	//{
	//	CString		Loadfile;
	//	CFTP_Client		pFTP;
	//	if(!pFTP.ConnectFTPServer("10.132.48.71","nsgdcc","nsgdcc168!",AtsInfo.m_csModelName))
	//		exit(1);
	//}
	//------
    if(mytimethread != NULL)
		{
		::TerminateThread(mytimethread->m_hThread,0);
		mytimethread = NULL;
		Sleep(1000);
		}
	memset(&SFIS, 0, sizeof(SFIS));
	if(!LoadTestItem())
	{
		nInitialLoadIsOk = FALSE;
		ShowMessage("Please close application and check library is exist or version is correct.");
		SetTestStatus(STATUS_SHOW_FAIL);
		m_testbutton.EnableWindow(1);

		return FALSE;
	}

	CString csTmp, csTmp2;
	char tmp[10];
	//------ 初始化一些參數 ------
	nPassCount = 0;
	nFailCount = 0;
	nTotalCount = 0;

	m_pass_count.SetWindowTextA(itoa(nPassCount, tmp, 10));
	m_fail_count.SetWindowTextA(itoa(nFailCount, tmp, 10));
	m_total_count.SetWindowTextA(itoa(nTotalCount, tmp, 10));

	GetDlgItem(IDC_Computer_Name1)->SetWindowTextA(AtsInfo.Log.csComputerName);
	GetDlgItem(IDC_DLL_INFO)->SetWindowTextA(m_dllInfo);
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		csTmp.Empty();
		GetDlgItem(IDC_SFIS_MEG)->ShowWindow(SW_HIDE);
		//=========For JiangYang MES===========Jane//
		//GetDlgItem(IDC_Log_Server)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_Log_Time)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_UPLOAD_STATUS)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_Log_Server)->SetWindowTextA("Log Server Address: "+AtsInfo.Log.csLogMappingDriver);
		//GetDlgItem(IDC_Log_Time)->SetWindowTextA("Upload Time: "+AtsInfo.Log.csLogUploadTime+" "+AtsInfo.Log.csLogUploadTime1);
		//GetDlgItem(IDC_UPLOAD_STATUS)->SetWindowTextA("Upload Status: Connecting...");
		//ShowMessage(AtsInfo.Log.csLogServerPath);
		//mytimethread = ::AfxBeginThread(HandleLogUpload, (LPVOID)0);
	}
	else
	{
		GetDlgItem(IDC_Log_Server)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_Log_Time)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_STATUS)->ShowWindow(SW_HIDE);
		m_SFISMessage.SetFontBold(TRUE);
		m_SFISMessage.SetFontName("Tahoma");
		m_SFISMessage.SetFontSize(26);
		m_SFISMessage.SetTextColor(RGB(255,0,0));
		m_SFISMessage.SetText("SFIS OFF");
	}
	//清除並初始化log structure
	STATION_Log.ResetAll();

	nInitialLoadIsOk = TRUE;

	m_testbutton.EnableWindow(FALSE);
	//ShowMessage("正在處理DLL初始程序.");
	ShowMessage("Initializing DLL ...");
	//處理所有DLL的PreInit程序，完成後才會把Start button enable
	CWinThread *mythread = ::AfxBeginThread(HandlePreInit, (LPVOID)1);
	csTotalMes.Empty();

    //ShowMessage("======測試系統重新啟動完成======");
	ShowMessage("======Test System restart OK======");
	return TRUE;
}
BOOL CATSDlg::PreTranslateMessage(MSG* pMsg)
{	
	if(pMsg->message==WM_KEYDOWN&&pMsg->wParam==VK_F7)
	{
		CRect rect;
		::GetWindowRect(GetDlgItem(IDC_UI_MSG)->m_hWnd,&rect);
		::SetCursorPos(rect.CenterPoint().x-(rect.Width()/2)+10,rect.CenterPoint().y-(rect.Height()/2)+5);
		//
		//::GetWindowRect(GetDlgItem(IDC_BUTTON_HELP)->m_hWnd,&rect);
		//GetDlgItem(IDC_BUTTON_HELP)->SetFocus();
		//::SetCursorPos(rect.CenterPoint().x,rect.CenterPoint().y);
		::mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
		::mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
		OnBnDoubleclickedButtonHelp();
			return TRUE;
	}
	if(pMsg->message==WM_KEYDOWN&&pMsg->hwnd==GetDlgItem(IDC_BUTTON_HELP)->m_hWnd)
	{
		if(pMsg->wParam==VK_RETURN)
		{
			return TRUE;
		}

	}
	return CDialog::PreTranslateMessage(pMsg);
}

CString CATSDlg::GetFileVersion(char *filename)
{
	CString sText;
	CString m_fileVersionString;
	m_fileVersionString = "Unknown";

		DWORD dwType;
		DWORD dwDataLen = GetFileVersionInfoSize(filename, &dwType);

		void *pszBuffer = malloc(dwDataLen + 1);

		if (pszBuffer == NULL)
			return m_fileVersionString;

		if (GetFileVersionInfo(filename, 0L, dwDataLen, pszBuffer) == TRUE)
		{
			VS_FIXEDFILEINFO *pFileInfo;

			if (VerQueryValue(pszBuffer, _T( "\\" ),
								 (void **)&pFileInfo, (UINT *)&dwDataLen) == TRUE)
			{
				char	szScratch[ 16 ];

				sprintf_s( szScratch, "%d.%d.%d.%d",
					HIWORD( pFileInfo->dwFileVersionMS ),
					LOWORD( pFileInfo->dwFileVersionMS ),
					HIWORD( pFileInfo->dwFileVersionLS ),
					LOWORD( pFileInfo->dwFileVersionLS )
					);

				m_fileVersionString = szScratch;
			}
		}
		free(pszBuffer);

	sText.ReleaseBuffer();
	return m_fileVersionString;
}

void CATSDlg::LoadATSini(void)
{
	char iniFileName[MAX_PATH];
	char strBuf[MAX_PATH];
	CString csLogPathTemp;
	::GetCurrentDirectory(256,strBuf);
	sprintf_s(iniFileName,"%s\\ATSCfg.ini",strBuf);
	csLogPathTemp.Format(strBuf);
	csLogPathTemp=csLogPathTemp.Left(3).Trim();
	if(csLogPathTemp.Compare("C:\\")==0){
		csLogPathTemp="D:\\";
	}
	
	/////////////////////////////////////// 20090421/////////////////////////////////////
	AtsInfo.TestFunc.SFIS.czModelPN="";//20090421
	AtsInfo.TestFunc.SFIS.nCALL_PN_Enable = GetPrivateProfileInt("SFIS","CALL_PN_Enable",0,iniFileName);//20090421
	GetPrivateProfileString("SFIS","DllLoadIniName","MFGATS_FTFC.ini",strBuf,sizeof(strBuf),iniFileName);
	AtsInfo.TestFunc.SFIS.csDllLoadIniName = strBuf;

	GetPrivateProfileString("SFIS","TerminalName","Super Terminal as DCN --> 1.0.2.8",strBuf,sizeof(strBuf),iniFileName);
	AtsInfo.TerminalNameSFIS =" ";
    AtsInfo.TerminalNameSFIS += strBuf;
	
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		CallModel();
		if(AtsInfo.TestFunc.SFIS.czModelPN=="")
		{
			::AfxMessageBox("PN為空", 0, 0);
			exit(1);//20090428//exit(1);
		}
		else
		{
			//FILE	*fp;
			char	filepath[512], filename[512],pn_filename[512],backup_filename[512];
			CString	newname, strTmp;
			BOOL	nResult;
			memset(filepath,0,sizeof(filepath));
			memset(filename,0,sizeof(filename));
			memset(pn_filename,0,sizeof(pn_filename));
			memset(backup_filename,0,sizeof(backup_filename));
			::GetCurrentDirectory(512, filepath);
			sprintf(filename,"%s\\%s", filepath,AtsInfo.TestFunc.SFIS.csDllLoadIniName);
			sprintf(pn_filename, "%s\\IniFile\\%s.ini", filepath,AtsInfo.TestFunc.SFIS.czModelPN);
			sprintf(backup_filename, "%s\\IniFile\\%s", filepath,AtsInfo.TestFunc.SFIS.csDllLoadIniName);
				//DeleteFile(new_filename);
				///////////////////////////
				CString csTFileName;
				CFileStatus TFileStatus;
				///////////////////////////
				//csTFileName.Format ("%s",backup_filename);
					//if ( !CFile::GetStatus(backup_filename, TFileStatus))
					//{
					//	nResult = CopyFile(filename,backup_filename,false);
					//}
					////csTFileName.Format ("%s",filename);
					//if ( !CFile::GetStatus(filename, TFileStatus))
					//{
					//	nResult = CopyFile(pn_filename,filename,false);
					//	if(!nResult)
					//	{
					//		nResult = CopyFile(pn_filename,filename,false);
					//		if(!nResult)
					//		{
					//			strTmp.Format("error or %s not found...!",pn_filename);
					//			::AfxMessageBox(strTmp, 0, 0);
					//			exit(1);
					//		}
					//	}
					//}
			nResult = CopyFile(filename,backup_filename,false);
			nResult = CopyFile(pn_filename,filename,false);
			if(!nResult)
			{
				nResult = CopyFile(pn_filename,filename,false);
				if(!nResult)
				{
					strTmp.Format("error or %s not found...!",pn_filename);
					::AfxMessageBox(strTmp, 0, 0);
					exit(1);
				}
			}
			Sleep(200);
			//WriteIniString(f, "Configuration", "BuildGDEnable", "0");
			WritePrivateProfileString("Configuration", "BuildGDEnable", "0",pn_filename);//20050515
		}
	}
	AtsInfo.TestFunc.SFIS.nCheckSFIS_InfoFS = GetPrivateProfileInt("SFIS","CheckSFIS_InfoFS",0,iniFileName);//20090421
	//////////////////////////////////// 20090421//////////////////////////////////////////
	GetPrivateProfileString("DutConfig","ProductName","",strBuf,sizeof(strBuf),iniFileName);
	AtsInfo.m_csModelName = strBuf;

	m_debug_message = GetPrivateProfileInt("TECONFIG","DEBUG_MSG",0,iniFileName);

	GetPrivateProfileString("DutConfig","EmbedIP","",strBuf,sizeof(strBuf),iniFileName);
	AtsInfo.m_ipEmbedIP = strBuf;
	AtsInfo.m_bootTime = GetPrivateProfileInt("DutConfig","BootTime",0,iniFileName);
	
	AtsInfo.m_nMAC_Length=GetPrivateProfileInt("InputKey","MAC_Length",12,iniFileName);
	AtsInfo.m_nSSN1_Length=GetPrivateProfileInt("InputKey","SSN1_Length",25,iniFileName);
	AtsInfo.TestFunc.SFIS.nIQExpire_Enable=GetPrivateProfileInt("SFIS","IQExpireEnable",0,iniFileName);
	AtsInfo.TestFunc.SFIS.m_TestEnable = GetPrivateProfileInt("SFIS","SFISEab",0,iniFileName);
	AtsInfo.TestFunc.SFIS_Retry.m_TestEnable =GetPrivateProfileInt("SFIS","SFIS_Retry_Eab",0,iniFileName);
	GetPrivateProfileString("SFIS","DllName","",AtsInfo.TestFunc.SFIS.m_DllName,sizeof(AtsInfo.TestFunc.SFIS.m_DllName),iniFileName);
	GetPrivateProfileString("SFIS","DllVersion","",AtsInfo.TestFunc.SFIS.m_DllVersion,sizeof(AtsInfo.TestFunc.SFIS.m_DllVersion),iniFileName);

	AtsInfo.m_bInputMac = GetPrivateProfileInt("InputKey", "InputMac", 0, iniFileName);
	AtsInfo.m_bAutoMac = GetPrivateProfileInt("InputKey", "AutoMac", 0, iniFileName);
	AtsInfo.m_bInputSN = GetPrivateProfileInt("InputKey", "InputSN", 0, iniFileName);
	AtsInfo.m_bInputSSN1 = GetPrivateProfileInt("InputKey", "InputSSN1", 0, iniFileName);
	AtsInfo.m_bInputSSN2 = GetPrivateProfileInt("InputKey", "InputSSN2", 0, iniFileName);
	AtsInfo.m_bCheckMac = GetPrivateProfileInt("InputKey", "CheckMac", 0, iniFileName);
	AtsInfo.m_bCheckSN = GetPrivateProfileInt("InputKey", "CheckSN", 0, iniFileName);
	AtsInfo.m_bCheckSSN1=GetPrivateProfileInt("InputKey","CheckSSN1",0,iniFileName);
	AtsInfo.m_bCheckSSN2=GetPrivateProfileInt("InputKey","CheckSSN2",0,iniFileName);
	AtsInfo.m_nSN_Length = GetPrivateProfileInt("InputKey", "SN_Length", 0, iniFileName);
	AtsInfo.m_nSSN1_Length=GetPrivateProfileInt("InputKey","SSN1_Length",0,iniFileName);
	AtsInfo.m_nSSN2_Length=GetPrivateProfileInt("InputKey","SSN2_Length",0,iniFileName);
	GetPrivateProfileString("InputKey", "MacStart", "000000000000", strBuf, 12, iniFileName);
	AtsInfo.m_csMacStart = strBuf;
	GetPrivateProfileString("InputKey", "MacEnd", "FFFFFFFFFFFF", strBuf, 12, iniFileName);
	AtsInfo.m_csMacEnd = strBuf;

	GetPrivateProfileString("DLL","StationName","",strBuf,255,iniFileName);
	AtsInfo.TestFunc.STATION.csStationName.Format(strBuf);
	GetPrivateProfileString("DLL","DllName","",AtsInfo.TestFunc.STATION.m_DllName,sizeof(AtsInfo.TestFunc.STATION.m_DllName),iniFileName);
	GetPrivateProfileString("DLL","DllVersion","",AtsInfo.TestFunc.STATION.m_DllVersion,sizeof(AtsInfo.TestFunc.STATION.m_DllVersion),iniFileName);

	AtsInfo.m_bAutoTest = GetPrivateProfileInt("TECONFIG","AutoTest",0,iniFileName);
	AtsInfo.m_FTP = GetPrivateProfileInt("TECONFIG","FTPEab",0,iniFileName);
	AtsInfo.LoopTest.m_bLoopTest = GetPrivateProfileInt("TECONFIG","LoopCountEab",0,iniFileName);
	AtsInfo.LoopTest.m_iCountLimite = GetPrivateProfileInt("TECONFIG","LoopCounts",0,iniFileName);

	///////////////////////////////////////////////////////////////////////
	//m_bLogByResult指的是Log是否依照PASS or FAIL分開擺放
	//m_bLogAll就是一般的合在一起的Log
	AtsInfo.Log.m_bLogByResult = GetPrivateProfileInt("TECONFIG","LogByResult",1,iniFileName);
	AtsInfo.Log.m_bLogAll = GetPrivateProfileInt("TECONFIG","LogAll",1,iniFileName);

	AtsInfo.LoopTest.m_bLoopTest = GetPrivateProfileInt("TECONFIG","LoopCountEab",0,iniFileName);
	AtsInfo.LoopTest.m_iCountLimite = GetPrivateProfileInt("TECONFIG","LoopCounts",0,iniFileName);

	// Add buttons to open Help file and Error code file Hillson 20120823 
	GetPrivateProfileString("TECONFIG","HelpFile","", strBuf, 255, iniFileName);
    AtsInfo.HelpFile = strBuf;
	GetPrivateProfileString("TECONFIG","ErrorCodeFile","", strBuf, 255, iniFileName);
    AtsInfo.ErrorCodeFile = strBuf;

	GetPrivateProfileString("FTP","FtpServerPath","\\\\192.168.126.245",strBuf,125,iniFileName);
	AtsInfo.ftp_Setting.m_ftpPath.Format(strBuf);
	GetPrivateProfileString("FTP","FtpUser","anonymous",strBuf,125,iniFileName);
	AtsInfo.ftp_Setting.csUserName.Format(strBuf);
	GetPrivateProfileString("FTP","FtpPasswd","",strBuf,125,iniFileName);
	AtsInfo.ftp_Setting.csPasswd.Format(strBuf);
	AtsInfo.ftp_Setting.nPort=GetPrivateProfileInt("FTP","FtpPort",21,iniFileName);
	//For SJ log file 路徑 20090625
	GetPrivateProfileString("TECONFIG","LogMappingDriver","Z:\\", strBuf, 255, iniFileName);
	AtsInfo.Log.csLogMappingDriver.Format(strBuf);
	GetPrivateProfileString("TECONFIG","LogServerPath","\\\\192.168.0.1\\result", strBuf, 255, iniFileName);
	AtsInfo.Log.csLogServerPath.Format(strBuf);
	GetPrivateProfileString("TECONFIG","User","", strBuf, 255, iniFileName);
	AtsInfo.Log.csUser.Format(strBuf);
	GetPrivateProfileString("TECONFIG","PassWord","", strBuf, 255, iniFileName);
	AtsInfo.Log.csPassWord.Format(strBuf);
	GetPrivateProfileString("TECONFIG","LogLocalFilePath","", strBuf, 255, iniFileName);
	AtsInfo.Log.csLogFilePath.Format(strBuf);
	GetPrivateProfileString("TECONFIG","LogUploadTime","", strBuf, 255, iniFileName);
	CString csTemp;
	csTemp.Format(strBuf);
	csTemp.Trim();
	AtsInfo.Log.csLogUploadTime=csTemp.Left(csTemp.Find(','));
	AtsInfo.Log.csLogUploadTime.Trim();
	AtsInfo.Log.csLogUploadTime1=csTemp.Right(csTemp.GetLength()-csTemp.Find(',')-1);
	AtsInfo.Log.csLogUploadTime1.Trim();
	if(AtsInfo.Log.csLogUploadTime.IsEmpty())
	{
		AtsInfo.Log.csLogUploadTime = AtsInfo.Log.csLogUploadTime1;
		AtsInfo.Log.csLogUploadTime1.Empty();
	}

	//For SJ
	AtsInfo.m_startOnButton = GetPrivateProfileInt("TECONFIG","StartTestOnButton",0,iniFileName);
	AtsInfo.m_ShowDect = GetPrivateProfileInt("TECONFIG","ShowDect",0,iniFileName);
	AtsInfo.m_UiLog = GetPrivateProfileInt("TECONFIG","UiLog",1,iniFileName);
	bEfficencyCalc = GetPrivateProfileInt("TECONFIG","TestEfficiency",1,iniFileName);
	if(bEfficencyCalc)
		{
		bFirstStart = true;
		}
	else
		{
		bFirstStart = false;
		}

	//DB_Setting
	AtsInfo.DB_Setting.m_bDB_Enable = GetPrivateProfileInt("DB_Setting","DB_Enable",0,iniFileName);
	GetPrivateProfileString("DB_Setting","Group_Name","TEST_GROUP", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csGroupName.Format(strBuf);
	GetPrivateProfileString("DB_Setting","Line_Number","TEST_Line", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csLineNumber.Format(strBuf);
	GetPrivateProfileString("DB_Setting","PC_IP","127.0.0.1", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csPC_IP.Format(strBuf);

	// Retest Setting
	
	GetPrivateProfileString("RetestAlarm","AlarmMessage","",strBuf,sizeof(strBuf),iniFileName);
	
	AtsInfo.m_RetestMSG.m_csAlarmMSG.Format("%s",strBuf);
	UINT num=0,iStart=0,nMaxLength=0;

	while(1)
		{
		num = AtsInfo.m_RetestMSG.m_csAlarmMSG.Find("\\n",iStart);
		if(num == -1)
			{
			break;
			}
		else
			{
				
			if( nMaxLength < (num-iStart))
				{
				nMaxLength = num-iStart;
				}
			iStart = num+strlen("\\n");
			}
		}
    AtsInfo.m_RetestMSG.m_Column = nMaxLength;
	AtsInfo.m_RetestMSG.m_num=AtsInfo.m_RetestMSG.m_csAlarmMSG.Replace("\\n","\n");

    GetPrivateProfileString("RetestAlarm","ConfirmPassword","123456",strBuf,sizeof(strBuf),iniFileName);
	AtsInfo.m_RetestMSG.m_csPassword.Format("%s",strBuf);

	AtsInfo.m_RetestMSG.m_nTotalRetestCounter = GetPrivateProfileInt("RetestAlarm","TotalRetestCounter",100,iniFileName);

    GetPrivateProfileString("RetestAlarm","RetestRateThreshold","10%",strBuf,sizeof(strBuf),iniFileName);
	strtok(strBuf,"%");
	AtsInfo.m_RetestMSG.m_RetesRateThreshold = (atoi(strBuf)*AtsInfo.m_RetestMSG.m_nTotalRetestCounter)/100;
	
	::GetCurrentDirectory(256,strBuf);
	sprintf_s(iniFileName,"%s\\SFIS.ini",strBuf);
	GetPrivateProfileString("SFIS","Step","",strBuf,255,iniFileName);
	AtsInfo.TestFunc.SFIS.csStep.Format(strBuf);
	AtsInfo.TestFunc.SFIS.csStep.Trim();
	GetPrivateProfileString("SFIS","Station Number","", strBuf, 255, iniFileName);
	AtsInfo.csStationName.Format(strBuf);
	AtsInfo.csStationName.Trim();
	AtsInfo.Log.csStationNum.Format(strBuf);
	AtsInfo.Log.csStationNum.Remove('-');
	AtsInfo.Log.csStationNum.Trim();
	AtsInfo.m_InputNum=GetPrivateProfileInt("Dynamic_Item","nInputTimes",1,iniFileName);
	//GetPrivateProfileString("Dynamic_Item","OP","jiangyang",strBuf,255,iniFileName);
	//AtsInfo.csUserName.Format(strBuf);
	//AtsInfo.csUserName.Trim();
	

//DB_Enable 	 = 0
//Line_Number  = L6
//Group_Name   = RF_TEST
//PC_IP 		 = 192.168.4.100
//Station_Name = RF_TEST_1

	if(AtsInfo.Log.csLogFilePath.IsEmpty()){
		AtsInfo.Log.csLogFilePath=csLogPathTemp;
	}

	if(!AtsInfo.TestFunc.SFIS.m_TestEnable){
		CString sPath;
		GetModuleFileName(NULL,sPath.GetBufferSetLength(201),200);
		sPath.ReleaseBuffer();
		int nPos = sPath.ReverseFind('\\');
		sPath= sPath.Left(nPos);
        AtsInfo.Log.csLogFilePath = sPath+"\\";
		//AtsInfo.Log.csLogFilePath.Empty();
	}

	//有用Shopfloor的時後，LoopTest自動設定為0
	if ( AtsInfo.TestFunc.SFIS.m_TestEnable )
		AtsInfo.LoopTest.m_bLoopTest = 0;
}

bool CATSDlg::StoreTestItem(TEST_FUNC testFunction)
{
	char FileName[MAX_PATH];
	char strBuf[MAX_PATH];
	char key[20];
	int	retPress=0;
	CString curtFileVerson;
	CString LoadFileVerson;

/*

	//Show CyberTanATS Product
	char Utility_Title[260];
	memset(Utility_Title,0,sizeof(Utility_Title));
	sprintf_s(Utility_Title,"CyberTanATS %s-V%s Test Utility",AtsInfo.m_csModelName,testFunction.m_DllVersion);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		memset(Utility_Title,0,sizeof(Utility_Title));
		sprintf_s(Utility_Title,"CyberTanATS %s-%s-V%s Test Utility",AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN,testFunction.m_DllVersion);
	}
	////////////////////////////////20090427/////////////////////////////////////////////

	SetWindowText(Utility_Title);

*/

	SetWindowText("NexComATS MFG Utility");	// for 1.0.0.23

	::GetCurrentDirectory(MAX_PATH,strBuf);
	curtFileVerson = GetFileVersion(testFunction.m_DllName);
	if(curtFileVerson!=testFunction.m_DllVersion)
	{
		sprintf_s(strBuf,"%s.Dll(ver %s) not found, or version(%s) is not mached.",testFunction.m_DllName,curtFileVerson,testFunction.m_DllVersion);
		ShowMessage(strBuf);
		return false;
	}

	if(AtsInfo.m_FTP)
	{
		char Loadfile[260];
		char Newfile[260];
		CFileStatus TFileStatus;
		sprintf(Loadfile,"%s\\%s.dll",AtsInfo.m_csModelName,testFunction.m_DllName);
		sprintf(Newfile,"%s.dll",testFunction.m_DllName);

		if(CFile::GetStatus(Loadfile,TFileStatus))
		{
			LoadFileVerson = GetFileVersion(Loadfile);
			if(LoadFileVerson != curtFileVerson)
			{
				//retPress = MessageBox("程式檢查到不同版本的程式文件，確認是否更新？","發現更新",MB_YESNO| MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
				retPress = MessageBox("Find diff Version File，are you going to update？", "Discovery Update", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
				if(retPress == IDYES)
				{
					if(CopyFile(Loadfile,Newfile,FALSE))
					//	ShowMessage("下載文件成功.");
						ShowMessage("Download OK.");
					else
					{
						//MessageBox("下載文件失敗.!","ERROR",MB_OK | MB_SETFOREGROUND);
						MessageBox("Download Fail.!", "ERROR", MB_OK | MB_SETFOREGROUND);
						exit(1);
					}
				}
			}
		}

		if(!LoadTestItem())
		{
			ShowMessage("Please close application and check library is exist or version is correct.");
			//ShowMessage("文件已更新至最新版本,請變更InI檔設置...!");
			ShowMessage("Have updated to the latest version,please update InI config file...!");
			SetTestStatus(STATUS_SHOW_FAIL);
		}
	}

	
	//sprintf_s(FileName, "%s\\%s.ini", strBuf, testFunction.m_DllName);
	//tree MD to get init test items from modelname path
	sprintf_s(FileName,"%s\\%s\\%s\\%s.ini",strBuf,"Models", AtsInfo.m_csModelName,testFunction.m_DllName);
	for(int i=1;i<200;i++)
	{
		sprintf_s(key,"Test%d",i);
		GetPrivateProfileString("TEST_ITEM",key,"",strBuf,sizeof(strBuf),FileName);
		if(strlen(strBuf)==0)
			continue; // 8/6/08 Ryan:遇到空白item也不會停止，繼續讀完全部的Test items
		sprintf_s(m_TestItemList[m_TestItemCount++],"%s_Test", strBuf);
	}
	return true;
}
bool CATSDlg::LoadTestItem(void)
{
	m_TestItemCount = 0;
	CString csTemp;
	//Initial test item list array
	for(int i=0; i < 200 ;i++)
	{
		memset(m_TestItemList[i],0,sizeof(m_TestItemList[i]));
		SetDlgItemText(IDC_TESTITEM_1+i,m_TestItemList[i]);
	}


	m_dllInfo = "";
	if(	!StoreTestItem(AtsInfo.TestFunc.STATION))
			return false;
	csTemp.Format("[%s]-%s-v%s   ",AtsInfo.TestFunc.STATION.csStationName, AtsInfo.TestFunc.STATION.m_DllName, AtsInfo.TestFunc.STATION.m_DllVersion);
	m_dllInfo.Append(csTemp);
	int j=0;
	for(int i=0; i < 30 ;i++)
	{
		j = (int)strlen(m_TestItemList[i]);
		SetDlgItemText(IDC_TESTITEM_1+i,m_TestItemList[i]);
	}

	return true;

}

void CATSDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CATSDlg::OnPaint()
{
	if(bTestMode==DEBUG_MODE)
	{
		GetDlgItem(IDC_UI_MSG)->ShowWindow(SW_SHOWNORMAL);
		m_TestStateCtrl.ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_UI_MSG)->ShowWindow(SW_HIDE);
		m_TestStateCtrl.ShowWindow(SW_SHOWNORMAL);
	}
	if (IsIconic())
	{
		 // device context for painting
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CATSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CATSDlg::DebugMessage(CString msg)
{
	if ( m_debug_message )
	{
		msg = "[Debug]" + msg;
		ShowMessage(msg);
	}
}

void CATSDlg::ShowMessage(CString text)
{
	CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
	CString History;
	int len;

	if(	p->GetLineCount()>200)
	{
		GetUiMsg();
		p->SetWindowText(text+"\r\n");
	}
	else
	{
		p->GetWindowText(History);
		len = History.GetLength();
		p->SetSel(len, len);
		p->ReplaceSel(text+"\r\n", false);
		csTotalMes+=History;
	}
}

void CATSDlg::SetTestItemStatus(int nID, int nStatus)
{
	switch(nStatus)
	{
	case STATUS_ITEM_PASS:
		m_testitem[nID].SetBkColor(RGB(100,255,50), GetSysColor(COLOR_3DFACE), CLabel::BackFillMode::Gradient);
		m_testitem[nID].SetTextColor(RGB(0,0,200));
		KillTimer(FLASH_SIGNAL);
		//Ftp_UpLog.m_nLogItem[nID]=nID;
		//m_testitem[nID].GetWindowText(Ftp_UpLog.csTestName[nID]);
		Ftp_UpLog.csRes[nID].Format("Pass");
		Ftp_UpLog.csRead[nID].Format("NA");
		Ftp_UpLog.Unit[nID].Format("NA");
		break;
	case STATUS_ITEM_FAIL:
		m_testitem[nID].SetBkColor(RGB(230,30,30));
		m_testitem[nID].SetTextColor(RGB(0,0,0));
		KillTimer(FLASH_SIGNAL);
		//Ftp_UpLog.m_nLogItem[nID]=nID;
		//m_testitem[nID].GetWindowText(Ftp_UpLog.csTestName[nID]);
		Ftp_UpLog.csRes[nID].Format("Fail");
		Ftp_UpLog.csRead[nID].Format("NA");
		Ftp_UpLog.Unit[nID].Format("NA");
		break;
	case STATUS_ITEM_TESTING:
		m_testitem[nID].SetBkColor(RGB(255,255,0), RGB(200,245,100), CLabel::BackFillMode::Gradient);
		m_testitem[nID].SetTextColor(RGB(95,125,5));
		Ftp_UpLog.m_nLogItem[nID]=nID;
		m_testitem[nID].GetWindowText(Ftp_UpLog.csTestName[nID]);
		m_nTestingItem = nID;
		m_nFlashFlag = 1;
		SetTimer(FLASH_SIGNAL, 600, NULL);
		break;
	case STATUS_ITEM_NORMAL:
		m_testitem[nID].SetBkColor(GetSysColor(COLOR_3DFACE));
		m_testitem[nID].SetTextColor(RGB(0,0,0));
		KillTimer(FLASH_SIGNAL);
		break;
	}


	return;
}

void CATSDlg::SetTestStatus(int nStatus)
{
	if ( AtsInfo.m_bServe )
	{
		nStatus = STATUS_SHOW_GOLD;
		return;
	}
	
	switch(nStatus)
	{
	case STATUS_SHOW_DECT:
		m_testStatus.SetTextColor( RGB(0,100,255) );
		m_testStatus.SetFontSize(55);
		m_testStatus.SetText((CString)"DECT");
		break;
	case STATUS_SHOW_IDLE:
		m_testStatus.SetTextColor( RGB(0,100,255) );
		m_testStatus.SetFontSize(55);
		m_testStatus.SetText((CString)"IDLE");
		break;
	case STATUS_SHOW_TEST:
		m_testStatus.SetTextColor( RGB(0,0,100) );
		m_testStatus.SetFontSize(60);
		m_testStatus.SetText((CString)"TEST");
		break;
	case STATUS_SHOW_FAIL:
		m_testStatus.SetTextColor( RGB(255,0,0) );
		m_testStatus.SetFontSize(60);
		m_testStatus.SetText( "FAIL");
		break;
	case STATUS_SHOW_ERRCODE:
		m_testStatus.SetTextColor( RGB(255,0,0) );
		m_testStatus.SetFontSize(55);
		m_testStatus.SetText(SFIS.czEC);
		break;
	case STATUS_SHOW_PASS:
		m_testStatus.SetTextColor( RGB(0,255,0) );
		m_testStatus.SetFontSize(58);
		m_testStatus.SetText((CString)"PASS");
		break;
	case STATUS_SHOW_GOLD:
		m_testStatus.SetTextColor( RGB(255,255,0) );
		m_testStatus.SetFontSize(53);
		m_testStatus.SetText((CString)"GOLD");
		break;
	}
	if(bTestMode==TEST_MODE)
	{
		ReDrawWindow();
	}
}

//開始測試函式
void CATSDlg::OnStartBtn()
{
	if(AtsInfo.m_UiLog)
	{
		CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
		p->SetWindowText("");//清除Dialog 所有內容
	}

	(CButton*)GetDlgItem(IDC_BUTTON_HELP)->EnableWindow(TRUE);
	m_testbutton.EnableWindow(FALSE);
	
	nRunningFlag = TRUE;

	//走Shopfloor的流程
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		AtsInfo.csUserName.Empty();
		CCInputNum cInput;
		COLORREF color1;
		color1=COLOR_RED;
		cInput.Create(UI_hWnd);
		CString showmessage;
		showmessage.Empty();
		::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&showmessage,(LPARAM)&color1);

		if(cInput.DoModal()==IDOK)
		{
			AtsInfo.csUserName.Trim();
			if(AtsInfo.csUserName.IsEmpty()/*||AtsInfo.csPasswd.IsEmpty()*/)
			{
				m_testbutton.EnableWindow(TRUE);
				return;
			}
		}else
		{
				m_testbutton.EnableWindow(TRUE);
				return;
		}

/*		Sfis_status=0;
		DWORD userID;
		HANDLE hThread=CreateThread(NULL,0,ThreadInput,closeCATSDlg,0,&userID);
		if(!userID)
		{
			m_testbutton.EnableWindow(TRUE);
			return;
		}
		WaitForSingleObject(hThread,INFINITE);
		TerminateThread(hThread,0);
		
		
		showmessage.Format("%s",BufferCheck);
		//::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&showmessage,(LPARAM)&color1);
		if(1!=Sfis_status)
		{
			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&showmessage,(LPARAM)&color1);
			m_testbutton.EnableWindow(TRUE);
			return;
		}
*/		CWinThread *pThread = AfxBeginThread(ThreadSFISIN, (LPVOID)1);
		return;
	}

	//走一般流程
	pMainThread = ::AfxBeginThread(ThreadMainFlow, NULL);
	return;
}

//OnShowMessage處理字串顯示
LRESULT CATSDlg::OnShowMessage(WPARAM a, LPARAM b)
{
	ShowMessage(*((CString*) a));
	return 1;
}

//OnLogMessage專門處理接收到DLL過來的Log訊息,
//type = 0時表示建立log title
//type = 1時表示建立log title結束
//type = 2時表示設定log欄位的值
//type = 3 or4時表示測試結束寫log, 3 for PASS, 4 for FAIL
//LRESULT CATSDlg::OnMESLogMessage(WPARAM wParam,LPARAM lParam)
//{
//
//}
LRESULT CATSDlg::OnLogMessage(WPARAM type, LPARAM msg)
{
	CString csFileName = "", csLogDate = "", csMsg = "", csTmp;
	CString csStation, csItem, csCh, csRate, csAnt;
	LogST *logST = new LogST;
	LogST *tmplog = new LogST;
	int	nPos = 0, nPos2;

	//0跟2的時後把傳過來的字串存起來預備做處理
	if ( type == 0 || type == 2 )
		csMsg = *((CString*)msg);


	//csTmp.Format("[Log]接收到訊息:%s", csMsg);
	csTmp.Format("[Log]receive message:%s", csMsg);
	DebugMessage(csTmp);

	GetLocalTime(&sTime);
	csLogDate.Format("%04d%02d%02d", sTime.wYear, sTime.wMonth, sTime.wDay);

	

	//做出Model Name路徑
	//csTmp.Format("%sLog\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		csTmp.Format("%sModels\\%s\\Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);

	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
	//	csTmp.Format("Log\\%s\\%s", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		csTmp.Format("%sModels\\%s\\Log\\%s", AtsInfo.m_csModelName, AtsInfo.TestFunc.SFIS.czModelPN);
	}
	////////////////////////////////20090427/////////////////////////////////////////////

	//依照不同的站別，先建出不同的log file檔案
	logST = &STATION_Log;
	logST->filename.Format("%s\\%s\\%s_%s.xls", csTmp, AtsInfo.TestFunc.STATION.csStationName, AtsInfo.TestFunc.STATION.csStationName, csLogDate);

	//Type 0:設定Log title
	//範例： TestItem001:CH=6&RATE=54&MCS=15&CHAIN=1&ANT=1&Field1=0&Field2=0&
	//會做出 TestItem001_CH6_RATE54_MSC15_CHAIN1_ANT1
	//實際上使用RATE和MCS不要同用，CHAIN和ANT不要同用

	if ( type == 0 )
	{
	//	DebugMessage("[Log]訊息分類：設定log title");
		DebugMessage("[Log]message ：set log title");
		//找出最前面的站別設定
		nPos = csMsg.Find(":");

		if ( nPos == -1 )
		{
			ShowMessage("[Log]找不到TestItem, 請檢查訊息格式!");
			return 0;
		}

		csItem = csMsg.Left(nPos);

		//再來找有無 CH 字串
		CutCString(csMsg, csItem);

		nPos = csMsg.Find("CH");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有CH");
		else
		{
			GetCString(csMsg, "CH=", "=", "&", csCh);
			csItem = csItem + "_Ch";
			csItem = csItem + csCh;

			CutCString(csMsg, "CH=" + csCh);
		}

		//再來找 RATE 字串
		nPos = csMsg.Find("RATE");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有RATE");
		else
		{
			GetCString(csMsg, "RATE=", "=", "&", csRate);
			csItem = csItem + "_Rate";
			csItem = csItem + csRate;

			CutCString(csMsg, "RATE=" + csRate);
		}

		//再來找 MCS 字串
		nPos = csMsg.Find("MCS");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有MCS");
		else
		{
			GetCString(csMsg, "MCS=", "=", "&", csRate);
			csItem = csItem + "_MCS";
			csItem = csItem + csRate;

			CutCString(csMsg, "MCS=" + csRate);
		}

		//再來找 CHAIN 字串
		nPos = csMsg.Find("CHAIN");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有CHAIN");
		else
		{
			GetCString(csMsg, "CHAIN=", "=", "&", csAnt);
			csItem = csItem + "_Chain";
			csItem = csItem + csAnt;

			CutCString(csMsg, "CHAIN=" + csAnt);
		}

		//再來找 ANT 字串
		nPos = csMsg.Find("ANT");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有ANT");
		else
		{
			GetCString(csMsg, "ANT=", "=", "&", csAnt);
			csItem = csItem + "_Ant";
			csItem = csItem + csAnt;

			CutCString(csMsg, "ANT=" + csAnt);
		}

		//下一步驟
		//1. 先找 &
		//2. 找到 =
		//3. 抓出中間字串
		//4. 不抓值因為不是設log值
		do
		{
			nPos = csMsg.Find("&");
			nPos2 = csMsg.Find("=", nPos);

			if ( nPos < 0 || nPos2 < 0 || nPos2 < nPos )
				break;

			//下面是抓&xxxx= 中間的xxxx
			csTmp = csMsg.Mid(nPos+1, nPos2-(nPos+1)); //抓到項目

			logST->title[logST->nIndex] = csItem + "_";
			logST->title[logST->nIndex] += csTmp;

			DebugMessage(logST->title[logST->nIndex]);

			logST->nIndex++;
			logST->nCount++;

			//下面這一行表示讓原字串再把目前找到的Field item切掉
			csMsg = csMsg.Mid(nPos2+1, csMsg.GetLength() - (nPos2+1) );
			nPos = csMsg.Find("&");
		} while ( nPos < csMsg.GetLength()-1 );

	}
	else if ( type == 1 ) //type 1:log title結束
	{
		CStdioFile flogfile;
		CFileStatus fs;
		CString tmp = "";

		if ( logST->nCount <= 0 )
			return 0;

		DebugMessage("[Log]訊息分類：結束log title");


		if ( flogfile.GetStatus(logST->filename, fs) )
		{
			if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
			{
				ShowMessage("open log file error(Type=1)! Try again.");
				//kill excel task
				WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
				Pause(1000);
				if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					//AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					AfxMessageBox("Log File path error, Please reset it", MB_OK, 0);
					exit(0);
					return FALSE;
				}
			}
			else
				flogfile.SeekToEnd();

		}
		else
		{
			if ( !flogfile.Open(logST->filename, CFile::modeCreate | CFile::modeReadWrite ) )
			{
				ShowMessage("open log file error(Type=1)! Try again.");
				//kill excel task
				WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
				Pause(1000);
				if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					//AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					AfxMessageBox("Log File path error, please reset it", MB_OK, 0);
					exit(0);
					return FALSE;
				}
			}
		}

		//先加入DATE_TIME, SN, MAC, RESULT
		tmp.Format("DATE_TIME\tDUT_SN\tDUT_MAC\tSpecialKey1\tSpecialKey2\tERROR_CODE\tRESULT\t");
		flogfile.WriteString(tmp);

	//	tmp.Format("寫入檔案 %s, 寫入欄位共 %d 欄", logST->filename, logST->nCount);
		tmp.Format("write file %s, write all %d fields", logST->filename, logST->nCount);
		DebugMessage(tmp);
		//CreateLog完成，寫title
		for ( int i = 0 ; i < logST->nCount ; i++ )
		{
			tmp.Format("%s\t", logST->title[i]);
			flogfile.WriteString(tmp);
		}

		tmp = "\n";
		flogfile.WriteString(tmp);
		flogfile.Close();
		logST->bCreated = true;
	}
	else if ( type == 2 ) //type 2:給log值
	{
		CString tmpTitle;
		DebugMessage("訊息分類：設定log value");
		if ( !logST->bCreated )
		{
			DebugMessage("[Log]此Log沒有create過，無法賦值！");
		}

		//先找到Test Item title
		nPos = csMsg.Find(":");

		if ( nPos == -1 )
		{
			ShowMessage("[Log]找不到TestItem, 請檢查訊息格式!");
			return 0;
		}

		csItem = csMsg.Left(nPos);

		//再來找有無 CH 字串
		CutCString(csMsg, csItem);

		nPos = csMsg.Find("CH");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有CH");
		else
		{
			GetCString(csMsg, "CH=", "=", "&", csCh);
			csItem = csItem + "_Ch";
			csItem = csItem + csCh;

			CutCString(csMsg, "CH=" + csCh);
		}

		//再來找 RATE 字串
		nPos = csMsg.Find("RATE");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有RATE");
		else
		{
			GetCString(csMsg, "RATE=", "=", "&", csRate);
			csItem = csItem + "_Rate";
			csItem = csItem + csRate;

			CutCString(csMsg, "RATE=" + csRate);
		}

		//再來找 MCS 字串
		nPos = csMsg.Find("MCS");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有MCS");
		else
		{
			GetCString(csMsg, "MCS=", "=", "&", csRate);
			csItem = csItem + "_MCS";
			csItem = csItem + csRate;

			CutCString(csMsg, "MCS=" + csRate);
		}

		//再來找 CHAIN 字串
		nPos = csMsg.Find("CHAIN");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有CHAIN");
		else
		{
			GetCString(csMsg, "CHAIN=", "=", "&", csAnt);
			csItem = csItem + "_Chain";
			csItem = csItem + csAnt;

			CutCString(csMsg, "CHAIN=" + csAnt);
		}

		//再來找 ANT 字串
		nPos = csMsg.Find("ANT");
		if ( nPos == -1 )
			DebugMessage("[Log]訊息中沒有ANT");
		else
		{
			GetCString(csMsg, "ANT=", "=", "&", csAnt);
			csItem = csItem + "_Ant";
			csItem = csItem + csAnt;

			CutCString(csMsg, "ANT=" + csAnt);
		}

		//下一步驟
		//1. 先找 &
		//2. 找到 =
		//3. 抓出中間字串, 找到欄位名
		//4. 再抓值出來
		int nValue=0;
		do
		{
			nPos = csMsg.Find("&"); //找&和=
			nPos2 = csMsg.Find("=", nPos);

			if ( nPos < 0 || nPos2 < 0 || nPos2 < nPos )
				break;

			//下面是抓&xxxx= 中間的xxxx
			csTmp = csMsg.Mid(nPos+1, nPos2-(nPos+1)); //抓到項目

			tmpTitle = csItem + "_";
			tmpTitle += csTmp; //組合起要填值的field

			int i = 0;
			//這裡搜尋是否有同樣欄位可以賦值
			for ( i = 0 ; i < logST->nCount ; i ++ )
			{
				if ( logST->title[i] == tmpTitle )
					logST->nIndex = i;
			}
			
			//Log的欄位是0 base，nIndex和nCount相同時表示全部的欄位都沒有找到
			if ( logST->nIndex == logST->nCount )
			{
				ShowMessage("[Log]沒找到相同欄位!");
				return 0;
			}

			nPos = nPos2;
			nPos2 = csMsg.Find("&", nPos); //找到=之後的&(下一項的開頭)
			csTmp = csMsg.Mid(nPos+1, nPos2-(nPos+1));
			logST->value[logST->nIndex] = csTmp;

			csTmp.Format("[Log]給值%s=%s", logST->title[logST->nIndex], logST->value[logST->nIndex]);
			DebugMessage(csTmp);
			//::AfxMessageBox(logST->value[logST->nIndex],MB_OK,0);
			if(-1!=logST->title[logST->nIndex].Find("ANT"))
			{
				if(-1!=logST->value[logST->nIndex].Find("null"))
				{
					strANT[nValue].Format("0");
				}else
				{
				strANT[nValue].Format("%s",logST->value[logST->nIndex]);
				}
				nValue++;	
			}
			//logST->nIndex++;
			//logST->nCount++;

			//下面這一行表示讓原字串再把目前找到的Field item切掉
			csMsg = csMsg.Mid(nPos2, csMsg.GetLength() - nPos2 );
			nPos = csMsg.Find("&");
		} while ( nPos < csMsg.GetLength()-1 );

	}
	else if ( type == 3 || type == 4 ) //寫Log值
	{
		CStdioFile flogfile, fPassfile, fFailfile;
		CFileStatus fs, fs1, fs2;
		CString tmp = "";
		DebugMessage("[Log]訊息分類：測試結束寫log");

		DebugMessage("[Log]寫入Log");

		if ( !logST->bCreated )
		{
			DebugMessage("[Log]此Log沒有create過，無法寫入！");
			DebugMessage("[Warning]注意Log沒有被寫入！");
			return 0;
		}

		//一般Log的情況
		if ( AtsInfo.Log.m_bLogAll )
		{
			if ( flogfile.GetStatus(logST->filename, fs) )
			{
				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error(Type=3 or 4)! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
					{
						ShowMessage("open log file error!");
						//AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
						AfxMessageBox("Log File path error, please reset it", MB_OK, 0);
						exit(0);
						return FALSE;
					}
				}

				flogfile.SeekToEnd();

			}
			else //這是檔案不存在的時後
			{
				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !flogfile.Open(logST->filename, CFile::modeCreate | CFile::modeReadWrite ) )
				{
					ShowMessage("Open log file error(Type=3 or 4)! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
					{
						ShowMessage("Open log file error!");
						//AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
						AfxMessageBox("Log File path error, please reset it", MB_OK, 0);
						exit(0);
						return FALSE;
					}
				}

				flogfile.SeekToEnd();

				//做寫入Log Title的動作
				//先加入DATE_TIME, SN, MAC, RESULT
				tmp.Format("DATE_TIME\tDUT_SN\tDUT_MAC\tSpecialKey1\tSpecialKey2\tERROR_CODE\tRESULT\t");
				flogfile.WriteString(tmp);

				//tmp.Format("寫入檔案 %s, 寫入欄位共 %d 欄", logST->filename, logST->nCount);
				//DebugMessage(tmp);
				//寫title
				for ( int i = 0 ; i < logST->nCount ; i++ )
				{
					//tmp.Format("%s,", logST->title[i]); //不用逗號分隔，改用xls
					tmp.Format("%s\t", logST->title[i]);
					flogfile.WriteString(tmp);
				}

				tmp = "\n";
				flogfile.WriteString(tmp);
			} //檔案不存在情況結尾
		}

		//假如有enable PASS/FAIL Log分開的選項
		if ( AtsInfo.Log.m_bLogByResult )
		{
			CString PassFileName, FailFileName, Dir, OriName;
			int pos = logST->filename.ReverseFind('\\');
			Dir = logST->filename.Left(pos+1);
			OriName = logST->filename.Mid(pos+1, logST->filename.GetLength() - pos );

			FailFileName = PassFileName = OriName.Left( OriName.GetLength()-4 );
			PassFileName.Append("_PASS_Only.xls");
			FailFileName.Append("_FAIL_Only.xls");
			PassFileName.Insert(0, "PASS\\");
			FailFileName.Insert(0, "FAIL\\");
			PassFileName.Insert(0, Dir);
			FailFileName.Insert(0, Dir);

			if ( fPassfile.GetStatus(PassFileName, fs1) &&
				 fFailfile.GetStatus(FailFileName, fs2) )
			{
				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !fPassfile.Open(PassFileName, CFile::modeReadWrite ) )
				{
					ShowMessage("Open xls log file error! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !fPassfile.Open(PassFileName, CFile::modeReadWrite) )
					{
						ShowMessage("Open log file error!");
						//AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
						AfxMessageBox("Log File path error, please reset it", MB_OK, 0);
						exit(0);
						return FALSE;
					}
				}

				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !fFailfile.Open(FailFileName, CFile::modeReadWrite ) )
				{
					ShowMessage("Open xls log file error! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !fFailfile.Open(FailFileName, CFile::modeReadWrite) )
					{
						ShowMessage("Open log file error!");
						//AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
						AfxMessageBox("Log File path error, please reset it", MB_OK, 0);
						exit(0);
						return FALSE;
					}
				}

				//把2個開啟的檔案都移到最後寫入
				fPassfile.SeekToEnd();
				fFailfile.SeekToEnd();
			}
			else //這是檔案不存在的時後
			{
				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !fPassfile.Open(PassFileName, CFile::modeCreate | CFile::modeReadWrite ) )
				{
					ShowMessage("Open xls log file error! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !fPassfile.Open(PassFileName, CFile::modeReadWrite) )
					{
						ShowMessage("Open log file error!");
						AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
						exit(0);
						return FALSE;
					}
				}

				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !fFailfile.Open(FailFileName, CFile::modeCreate | CFile::modeReadWrite ) )
				{
					ShowMessage("Open xls log file error! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !fFailfile.Open(FailFileName, CFile::modeReadWrite) )
					{
						ShowMessage("Open log file error!");
						AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
						exit(0);
						return FALSE;
					}
				}

				//把2個開啟的檔案都移到最後寫入
				fPassfile.SeekToEnd();
				fFailfile.SeekToEnd();

				//做寫入Log Title的動作
				//先加入DATE_TIME, SN, MAC, RESULT
				tmp.Format("DATE_TIME\tDUT_SN\tDUT_MAC\tSpecialKey1\tSpecialKey2\tERROR_CODE\tRESULT\t");
				fPassfile.WriteString(tmp);
				fFailfile.WriteString(tmp);

				tmp.Format("[Log]寫入檔案 %s, 寫入欄位共 %d 欄", logST->filename, logST->nCount);
				DebugMessage(tmp);
				//寫title
				for ( int i = 0 ; i < logST->nCount ; i++ )
				{
					//tmp.Format("%s,", logST->title[i]); //不用逗號分隔，改用xls
					tmp.Format("%s\t", logST->title[i]);
					fPassfile.WriteString(tmp);
					fFailfile.WriteString(tmp);
				}

				tmp = "\n";
				fPassfile.WriteString(tmp);
				fFailfile.WriteString(tmp);
			}
		}

		//================================================================================
		//接下來這段寫內容值
		//固定欄位DATE_TIME, SN, MAC, RESULT, SN, MAC, SSN1, SSN2, ERROR_CODE
		if ( AtsInfo.Log.m_bLogByResult )
		{
			tmp.Format("%4d/%02d/%02d-%02d:%02d:%02d\t", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
			tmp.AppendFormat("\'%s\'\t\'%s\'\t\'%s\'\t\'%s\'\t%s\t%s\t", SFIS.czSN, SFIS.czMAC, SFIS.czSSN1, SFIS.czSSN2, SFIS.czEC, type == 3 ? "PASS" : "FAIL");
			if ( type == 3 ) //PASS condition
				fPassfile.WriteString(tmp);
			else if ( type == 4 )
				fFailfile.WriteString(tmp);

			tmp.Format("[Log]寫入檔案 %s, 寫入欄位共 %d 欄", logST->filename, logST->nCount);
			DebugMessage(tmp);
			//寫value
			for ( int i = 0 ; i < logST->nCount ; i++ )
			{
				tmp.Format("%s\t", logST->value[i]);
				if ( type == 3 ) //PASS condition
					fPassfile.WriteString(tmp);
				else if ( type == 4 )
					fFailfile.WriteString(tmp);
			}

			tmp = "\n";
			//if ( type == 3 ) //PASS condition
			fPassfile.WriteString(tmp);
			//else if ( type == 4 )
			fFailfile.WriteString(tmp);

			fPassfile.Close();
			fFailfile.Close();
		}

		if ( AtsInfo.Log.m_bLogAll )
		{
			tmp.Format("%4d/%02d/%02d-%02d:%02d:%02d\t", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
			tmp.AppendFormat("\'%s\'\t\'%s\'\t\'%s\'\t\'%s\'\t%s\t%s\t", SFIS.czSN, SFIS.czMAC, SFIS.czSSN1, SFIS.czSSN2, SFIS.czEC, type == 3 ? "PASS" : "FAIL");
			flogfile.WriteString(tmp);

			tmp.Format("[Log]寫入檔案 %s, 寫入欄位共 %d 欄", logST->filename, logST->nCount);
			DebugMessage(tmp);
			//寫value
			for ( int i = 0 ; i < logST->nCount ; i++ )
			{
				if(!logST->value[i].Trim().IsEmpty())
				{					
					LogRes[m_LogCount].csLogTitle=logST->title[i];
					LogRes[m_LogCount].csLogRes=logST->value[i];
					m_LogCount++;
				}
				tmp.Format("%s\t", logST->value[i]);
				flogfile.WriteString(tmp);
			}
			
			tmp = "\n";
			flogfile.WriteString(tmp);
			flogfile.Close();
		}
		//===========================================================================
	}
	return 1;
}



LRESULT CATSDlg::OnEnableTestButton(WPARAM enable, LPARAM lParam)
{
	m_testbutton.EnableWindow((int)enable);

	return 1;
}

LRESULT CATSDlg::OnShowActMessage(WPARAM a, LPARAM b)
{
	CString *text = (CString*)a;
	//char *msg = new char[255];
	//strcpy(msg, (char*)a);
	//CString text(msg);
	if((*text).Find("9317-11000ML0R")!=-1)
	{
		(*text).Insert(2,"-KR");
	}
	if((*text).Find("931G-11000ML0R")!=-1)
	{
		(*text).Insert(2,"-CH");
	}
	COLORREF *color = (COLORREF*)b;
	m_ActMessage.SetFontBold(TRUE);
	m_ActMessage.SetFontName("Tahoma");
	m_ActMessage.SetFontSize(26);
	m_ActMessage.SetTextColor(*color);
	m_ActMessage.SetText(*text);

	return 1;
}

//------ 這個函式會使Status顯示GOLD字樣 ------
LRESULT CATSDlg::OnShowGolden(WPARAM wMsg, LPARAM lMsg)
{
	SetTestStatus(STATUS_SHOW_GOLD);
	AtsInfo.m_bServe = 1;

	//ShowGolden時順便把所有的TestItem都隱藏
	for ( int i = 0 ; i < 30 ; i++ )
		m_testitem[i].ShowWindow(0);

	m_testbutton.EnableWindow(FALSE);

	return true;
}
LRESULT CATSDlg::OnDeleteObject(WPARAM wParam, LPARAM lParam)
{
    delete (CDlgSetting *)lParam;
	nDlgSettingCounter --;
	return 1;
}

LRESULT CATSDlg::OnSetTestStatus(WPARAM wParam, LPARAM lParam)
{
	int status = *((int*)wParam);
	SetTestStatus(status);

	return 1;
}

//這個function讓GOLDEN端show出正在測試的Item並做變色動作
LRESULT CATSDlg::OnSetGoldenItemStatus(WPARAM wParam, LPARAM lParam)
{
	CString *csTmp = (CString *)wParam;

	for ( int i = 0 ; i < 30 ; i++ )
		m_testitem[i].ShowWindow(0);

	m_testitem[0].ShowWindow(1);
	m_testitem[0].SetWindowText(csTmp->GetBuffer());

	m_nTestingItem = 0; //Ryan:因為GOLDEN端只會SHOW出第1個item label
	strcpy(m_TestItemList[0], csTmp->GetBuffer()); //把顯示的文字設成List中的第一個

	//讓第1個label變色
	::SendMessage(this->m_hWnd, MSG_SET_ITEM_STATUS, (WPARAM)csTmp, (LPARAM)STATUS_ITEM_TESTING);

	return 1;
}

//這個Function是要找出符合的Item Title然後對它的標籤做變色動作
LRESULT CATSDlg::OnSetDutItemStatus(WPARAM wParam, LPARAM lParam)
{
	int count = 0;
	CString *csTmp = (CString *)wParam;
	int	status = lParam;

	if ( *csTmp == "RESET" )
	{
		for ( int i = 0 ; i < 30 ; i ++ )
			SetTestItemStatus(i, STATUS_ITEM_NORMAL);
		return 0;
	}

	csTmp->Append("_Test");

	for ( count = 0 ; count < 30 ; count++)
	{
		if ( strstr(m_TestItemList[count], csTmp->GetBuffer()) != NULL )
			break;
	}

	//count等於30的話，表示沒有找到任何符合的Item Title所以不會做任何變色的動作
	if ( count == 30 )
		return false;

	SetTestItemStatus(count, status);

	return true;
}
//測試開始，呼叫Thread
LRESULT CATSDlg::OnTestStart(WPARAM, LPARAM)
{
	// Clean Message Window when start to test Hillson 20120824
    CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
    p->SetWindowText("");
		csTotalMes.Empty();
	if ( !AtsInfo.m_bServe )
		{
		SetTestStatus(STATUS_SHOW_TEST);
		}
   
/*	for(int i=0;i<200;i++)
	{
		Ftp_UpLog.csTestName[i].Format("");
	}
*/	CWinThread *pThread = AfxBeginThread(ThreadTester, (LPVOID)1);
	return true;
}



//測試結束時做的事
LRESULT CATSDlg::OnTestEnd(WPARAM Func, LPARAM Result)
{
	SFIS_PM *SFIS_Recv;
	int nSfis_result;
	int labelsta;
	char tmp[100];

 	if ( Result == 3 ) //傳3回來表示設定Shopfloor的值
	{
		SFIS_Recv = (SFIS_PM*)Func;
		memcpy(&SFIS, SFIS_Recv, sizeof(SFIS_PM));
		return 0;
	}

	if ( Result == TEST_FAIL )
	{
		if ( strlen(SFIS.czEC) < 4 )  //STATUS_SHOW_FAIL = 3
			SetTestStatus(STATUS_SHOW_FAIL);
		else //STATUS_SHOW_ERRCODE = 7
			SetTestStatus(STATUS_SHOW_ERRCODE);
	}
	else if ( Result == TEST_PASS )
	{	//STATUS_SHOW_PASS = 4
		SetTestStatus(STATUS_SHOW_PASS);
	}

	//計算測試count and show
	Result == TEST_FAIL ? nFailCount++ : nPassCount++;
	nTotalCount++;

	m_pass_count.SetWindowTextA(_itoa(nPassCount, tmp, 10));
	m_fail_count.SetWindowTextA(_itoa(nFailCount, tmp, 10));
	m_total_count.SetWindowTextA(_itoa(nTotalCount, tmp, 10));

	if(0==(nPassCount+nFailCount)%50)
	{
	if(AtsInfo.TestFunc.SFIS.nIQExpire_Enable)
	{
		CString csTemp;
		nInitIQ=ThreadGetIQExpire();
		m_IQExpire.EnableWindow(TRUE);
		m_IQExpire.ShowWindow(SW_SHOWNORMAL);
		if(nInitIQ>1)
		{
			m_IQFlash.EnableWindow(FALSE);
			m_IQFlash.ShowWindow(SW_HIDE);
			csTemp.Format("IQ到期剩餘:%d天",nInitIQ);
			m_IQExpire.SetWindowText(csTemp);
			m_IQExpire.SetBkColor(RGB(0,255,0),0,CLabel::BackFillMode::Normal);
		}
		else
		{
			m_IQFlash.EnableWindow(TRUE);
			m_IQFlash.ShowWindow(SW_SHOWNORMAL);
			SetTimer(FLASH_IQSIGNAL,1000,NULL);
			csTemp.Format("IQ即將到期");
			m_IQExpire.SetWindowText(csTemp);
			m_IQExpire.SetBkColor(RGB(255,0,0),0,CLabel::BackFillMode::Normal);
		}
	}	
	}
	//Golden's flow
	if ( AtsInfo.m_bServe )
	{
		::SendMessage(UI_hWnd, MSG_SHOW_GOLDEN, 0, 0);
		return 0;
	}
	MakeNetLog(Result == TEST_PASS ? 1:0 );
	//清除所有Log資料
	STATION_Log.ResetValue();
	//////////////////////////////////////////////////////////////////////////////

	if( Result == TEST_PASS && AtsInfo.m_bAutoMac )// Add Auto mac counter, Brand 2008/10/7
	{
		CAutoInputKey AutoKey;
		AutoKey.GetMac();
		AutoKey.MacCounter();
	}
		csTotalMes.Empty();
	//Dut's flow [Shopfloor enable]
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		int i;
		CString		tempstr;
		CString		csTmp;
		int			status;
		nLinkRes=0;
		Sfis_status=0;

		DWORD  threadIdLink,ID;
		//====For  JiangYang MES,call routestation another time===Jane//
/*		HANDLE hEndSFIS=CreateThread(NULL,
									4096,
									ThreadTestEndSFIS,
									closeCATSDlg,
										0,
									&ID);
		if(!ID)
		{
			csTmp.Format("Send MES Test End Information fail!");
			ShowMessage(csTmp);
		}else
		{
			WaitForSingleObject(hEndSFIS,INFINITE);
		}
		csTmp.Format("%s",BufferCheck);
		ShowMessage(csTmp);
		Sfis_status=0;							
*/		//====For  JiangYang MES,Begin Link for FT Station====Jane//
		/*if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT"))
		{
			if(Result==TEST_PASS)
			{		
				char* 	receiveBuffer = new char[4096];
				memset(receiveBuffer,0,sizeof(receiveBuffer));
				HANDLE hLinkSFIS=CreateThread(NULL,
											4096,
											ThreadLinkSFIS,
											closeCATSDlg,
												0,
											&threadIdLink);
				if(!threadIdLink)
				{
					csTmp.Format("Call GetMES Function Fail");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csTmp, 0);
					strcpy(SFIS.czEC, "SF00");
					status = STATUS_SHOW_ERRCODE;
					::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
				}
				else
				{
					WaitForSingleObject(hLinkSFIS,INFINITE) ;
				}
				sprintf(receiveBuffer,"%s",BufferCheck);
				csTmp.Format("%s",receiveBuffer);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTmp,0);
				if(Sfis_status!=1)
				{
					nLinkRes=1;
					COLORREF color1;
					color1=COLOR_RED;
					
					if(Sfis_status==2)
					{
						csTmp.Format("!#MES條碼綁定超時無回應#");
						::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&csTmp,(LPARAM)&color1);
						strcpy(SFIS.czEC, "SF00");
						status = STATUS_SHOW_ERRCODE;
						::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
					}
					if(Sfis_status==5)
					{
						csTmp.Format("!#MES條碼綁定無PASS或者FAIL信息#");
						::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&csTmp,(LPARAM)&color1);
						strcpy(SFIS.czEC, "SF10");
						status = STATUS_SHOW_ERRCODE;
						::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
					}
					if(Sfis_status==6||Sfis_status==0)
					{
						csTmp.Format("!#MES條碼綁定失敗#");
						::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&csTmp,(LPARAM)&color1);
						strcpy(SFIS.czEC, "SF11");
						status = STATUS_SHOW_ERRCODE;
						::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
					}
				}
				else
				{
					strcpy(SFIS.czEC,"");
				}
				delete [] receiveBuffer;
			}
		}*/
				//csTmp.Format(">> 第 %d次 上傳綁定訊息至MES ...",1);
				//ShowMessage(csTmp);
				//nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );
				//if(nSfis_result!=1)
				//{
				//	if(-2==nSfis_result)
				//	{
				//		strcpy(SFIS.czEC, "SF00");
				//		nLinkRes=1;
				//		status = STATUS_SHOW_ERRCODE;
				//		::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
				//	}
				//	else
				//	{
				//		Sleep(1000);
				//		csTmp.Format(">> 第 %d次 上傳綁定訊息至MES ...",2);
				//		ShowMessage(csTmp);
				//		nLinkRes=0;
				//		nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );
				//		if(nSfis_result!=1)
				//		{
				//		if(-1==nSfis_result)
				//		{
				//			nLinkRes=1;
				//			strcpy(SFIS.czEC, "SF10");
				//			status = STATUS_SHOW_ERRCODE;
				//			::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
				//		}
				//		else
				//		{
				//			nLinkRes=1;
				//			strcpy(SFIS.czEC, "SF00");
				//			status = STATUS_SHOW_ERRCODE;
				//			::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
				//		}
				//		}
				//	}
				//}
				
			

		//=====For Jiang MES,Begin Upload Log=====Jane//
		bool nValue=((Result==TEST_PASS)?1:0);
		nSfis_result=strlen(SFIS.czEC);
		if(nSfis_result)
		{
			nValue=0;
			nLinkRes=1;
		}else
		{
			nValue=1;
			nLinkRes=0;
		}
/*			tempstr=MakeFTPInfo(nValue,csTmp);
			if(tempstr.IsEmpty())
			{
				csTmp.Format("!#未找到相應的MES測試信息!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTmp,0);
				COLORREF color19;
				color19=COLOR_RED;
				::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&csTmp,0);
			}
			else
			{
				if(!MakeMESLog(nValue,tempstr))
				{
					csTmp.Format("!#未能在本地生成測試LOG!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTmp,0);
					COLORREF color19;
					color19=COLOR_RED;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&csTmp,0);
				}
				else
				{
					for(int i=0;;i++)
					{
						if(i<3)
						{
							if(ftpLog())
							{
								break;
							}
							Sleep(1000);
						}
						else
						{
							csTmp.Format("!#上傳FTP Log失敗!");
							::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTmp,0);
							COLORREF color19;
							color19=COLOR_RED;
							::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE,(WPARAM)&csTmp,(LPARAM)&color19);
							if(Result==TEST_PASS)
							{
								strcpy(SFIS.czEC, "SF10");
								status = STATUS_SHOW_ERRCODE;
								::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
							}
							break;
						}
					}
				}
			}
	*/}
		//=====For Jiang MES,Begin Upload Log=====Jane//
			//if ( Result == TEST_PASS ) //upload directly while pass condition
			//{
			//	csTmp.Format(">> 第 %d次 上傳DUT TEST PASS訊息至SFIS ...",1);
			//	ShowMessage(csTmp);
			//	nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );
			//	if(nSfis_result !=1)
			//	{
			//		if(nSfis_result==-9)
			//		{
			//			CString showMsgTemp;
			//			showMsgTemp.Format("!#測試前后SFIS回傳資料不符合#!");
			//			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsgTemp, 0);
			//			COLORREF color19;
			//			color19 = COLOR_RED;
			//			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsgTemp, (LPARAM)&color19);
			//		}
			//		else
			//		{
			//			Sleep(100);
			//			csTmp.Format(">> 第 %d次 上傳DUT TEST PASS訊息至SFIS ...",2);
			//			ShowMessage(csTmp);
			//			nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );
			//		}

			//		if(nSfis_result == 2)//T0mmy 20091124
			//		{
			//			//Tommy 20091118 SFIS error 時, 顯示SFIS errorcode
			//			strcpy(SFIS.czEC, "SF00");
			//			status = STATUS_SHOW_ERRCODE;
			//			::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);

			//		}
			//		else if(nSfis_result == 3)
			//		{
			//			//Tommy 20091118 SFIS error 時, 顯示SFIS errorcode
			//			strcpy(SFIS.czEC, "SF10");
			//			status = STATUS_SHOW_ERRCODE;
			//			::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
			//		}
			//	}
			//}
			//else if ( Result == TEST_FAIL )
			//{
			//	SFISTestFunc(FALSE,SEND_FAIL_INFO_TO_SFIS );
			//}
	
	
	
	//ShowMessage("測試已完成，請更換待測裝置!!");
	ShowMessage("Test complete，please change a new DUT!!");

	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		switch(AtsInfo.m_InputNum)
		{
		case 1:
			labelsta=STATUS_LABEL_INPUT1;
			break;
		case 2:
			labelsta=STATUS_LABEL_INPUT2;
			break;
		case 3:
			labelsta=STATUS_LABEL_INPUT3;
			break;
		}
		::SendMessage(UI_hWnd,MSG_ENABLE_LABLE_INPUT,(WPARAM)&labelsta,0);
	}
	if(AtsInfo.m_UiLog)
	{
		GetUiMsg();
	}
	
	CString csfinalName,cslogserverpath,csSN,csMac;
	csSN.Format("%s", SFIS.czSN);
	csSN.Trim();
	csMac.Format("%s", SFIS.czMAC);
	csMac.Trim();
	if(Result == TEST_PASS)
	{
		csfinalName.Format("%s_P.txt",csUILog);
		cslogserverpath.Format("%s\%s_%s_%s_%s_P.txt", AtsInfo.Log.csLogMappingDriver, csSN, csMac, UiLogName, AtsInfo.Log.csStationNum);
	}
	else if(Result == TEST_FAIL)
	{
		csfinalName.Format("%s_F.txt",csUILog);
		cslogserverpath.Format("%s\%s_%s_%s_%s_F.txt", AtsInfo.Log.csLogMappingDriver, csSN, csMac, UiLogName, AtsInfo.Log.csStationNum);
	}
	rename(csUILog+".txt",csfinalName);
    AtsInfo.Log.csLogFilePathUI =csfinalName;
//	ShowMessage(AtsInfo.Log.csLogFilePathUI);
//	ShowMessage(cslogserverpath);

	if (AtsInfo.TestFunc.SFIS.m_TestEnable)
		MyCopyFile(AtsInfo.Log.csLogFilePathUI, cslogserverpath);
	//有Loop的情況
	if (AtsInfo.m_UiLog)
	{
		CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
		p->SetWindowText("");//清除Dialog 所有內容
	}
	memset(&SFIS, 0, sizeof(SFIS)); //清除這次測試的shopfloor資料
	if (AtsInfo.LoopTest.m_bLoopTest)
	{
		//loop次數還沒結束的情況
		if( AtsInfo.LoopTest.m_iCounter++ < AtsInfo.LoopTest.m_iCountLimite )
		{
			if ( ( AtsInfo.m_ipEmbedIP != "" && AtsInfo.m_ipEmbedIP.GetLength() > 7 ) )
			{
				int count = 0;
				while ( count < 3 && nRunningFlag )
				{
					if ( !PingMe(AtsInfo.m_ipEmbedIP, "any", 2.0) )
						count++;
					else
					{
						Pause(500);
						//ShowMessage("測試已完成，請更換待測裝置!!");
						ShowMessage("Test complete，please change a new DUT!!");
						count = 0;
					}
					Pause(200);
				}

				COLORREF color1 = COLOR_GREEN;
				CString temp = "";
				::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&temp, (LPARAM)&color1);
				SetTestStatus(STATUS_SHOW_IDLE);
			}
			else
			{
				Pause(200);
				SetTestStatus(STATUS_SHOW_IDLE);
			}
		}
		else
		{
			//loop次數結束的情況
			//GetDlgItem(IDTEST)->EnableWindow(true);
			AtsInfo.LoopTest.m_iCounter = 1;
			nRunningFlag = FALSE;
			return true;
		}
	}
	else if ( AtsInfo.m_bAutoTest ) //沒Loop但是有AutoTest的情況, 有偵測到Ping就自動開始
	{

		//check if Dut is removed while AutoTest condition
		if ( ( AtsInfo.m_ipEmbedIP != "" && AtsInfo.m_ipEmbedIP.GetLength() > 7 ) )
		{
			int count = 0;
			while ( count < 3 && nRunningFlag )
			{
				if ( !PingMe(AtsInfo.m_ipEmbedIP, "any", 2.0) )
					count++;
				else
				{
					//ShowMessage("測試已完成，請更換待測裝置!!");
					ShowMessage("Test complete，please change a new DUT!!");
					count = 0;
					Pause(1000);
				}
				Sleep(500);
			}

			COLORREF color1 = COLOR_GREEN;
			CString temp = "";
			::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&temp, (LPARAM)&color1);
			SetTestStatus(STATUS_SHOW_IDLE);
		}
		else
		{
			//GetDlgItem(IDTEST)->EnableWindow(true);
			nRunningFlag = FALSE;
		}
	}
	else if ( !AtsInfo.TestFunc.SFIS.m_TestEnable ) //這是假如什麼都沒設定的結果
	{
		//if((AtsInfo.m_bAutoTest || AtsInfo.LoopTest.m_bLoopTest))//20090506
		//	Sleep(100);//20090506
		//else//20090506
		nRunningFlag = FALSE;
			GetDlgItem(IDTEST)->EnableWindow(true);

	}

	return true;
}

void CATSDlg::OnTimer(UINT_PTR nIDEvent)
{
	static UINT reTryTime=1;
	switch(nIDEvent)
	{
	case FLASH_SIGNAL: //變色閃爍用的Timer
		m_nFlashFlag = !m_nFlashFlag;
		if ( m_nFlashFlag )
			m_testitem[m_nTestingItem].SetBkColor(RGB(200,245,100), RGB(255,255,0), CLabel::BackFillMode::Gradient);
		else
			m_testitem[m_nTestingItem].SetBkColor(RGB(255,255,0), RGB(200,245,100), CLabel::BackFillMode::Gradient);

		break;
	case FLASH_IQSIGNAL:
		m_nIQFlash=!m_nIQFlash;
		if(m_nIQFlash)
			m_IQFlash.SetBkColor(RGB(255,0,0),0,CLabel::BackFillMode::Normal);
		else
			m_IQFlash.SetBkColor(RGB(128,0,255),0,CLabel::BackFillMode::Normal);
		break;
	}
	CDialog::OnTimer(nIDEvent);
}

//UI結束時呼叫函式
void CATSDlg::OnDestroy()
{
	
	CDialog::OnDestroy();
	//判斷若是初始化正常的時後，要做釋放DLL的動作
	if ( nInitialLoadIsOk )
	{
		/* 8/06/08 Ryan:To add Terminal(); */
		//HINSTANCE hDLL;
		bool testPass = 0;
		typedef void (WINAPI* lpTerminal)();
		lpTerminal Terminal;

		if ( hDLL != NULL )
		{
			Terminal = (lpTerminal)GetProcAddress(hDLL,"Terminal");
			Terminal();
			FreeLibrary(hDLL);
		}
	}
	WinExec("taskkill /F /IM CyberTanATS.exe /T", SW_HIDE);

}

void CATSDlg::OnClose()
{
	nRunningFlag = FALSE;
	//=======For JiangYang MES==========Jane//

	//if(AtsInfo.TestFunc.SFIS.m_TestEnable&&nInitialLoadIsOk)
	//	{
	//	MyMessageBox MyMB;
	//	int BoxResult = MyMB.DisplayMessageBox();
	//	}
	CDialog::OnClose();
}
CString CATSDlg::MakeFTPInfo(bool nType,CString csMESInfo)
{
	CString csTemp,str;
	int BuffLen=0,nStaTime=0,nEndTime=0;
	char Buff[20];
	int nHour=0,nMinute=0,nSec=0;
	csTemp.Format("%s",receiveBufferThread);
	//if(csTemp.IsEmpty())
	//{
	//	csMESInfo.Format("");
	//	return csMESInfo;
	//}
	csMESInfo.Format("[START_LOG]\r\n\r\n");			
	memset(Buff,0,sizeof(Buff));
	sprintf(Buff,"SerialNumber:");
	LengthAlign(Buff,20);
	str.Format("%s",Buff);
	//if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT"))
	//{
	//	if(nLinkRes)
	//	{
	//		csMESInfo.AppendFormat("%s%s\r\n",str,SFIS.czSN);
	//	}else
	//	{
	//		csMESInfo.AppendFormat("%s%s\r\n",str,SFIS.czSSN1);
	//	}
	//}
	//else if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RC"))
	//{
	//	csMESInfo.AppendFormat("%s%s\r\n",str,SFIS.czSSN1);
	//}
	//else
	//{
	csMESInfo.AppendFormat("%s%s\r\n",str,SFIS.czSN);
	//}
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Station:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,AtsInfo.csStationName.Trim());
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Step:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,AtsInfo.TestFunc.SFIS.csStep);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Start Time:");
	LengthAlign(Buff,20);
	str.Format("%04d-%02d-%02d %02d:%02d:%02d",sStartTime.wYear,sStartTime.wMonth,sStartTime.wDay,sStartTime.wHour,sStartTime.wMinute,sStartTime.wSecond);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"EndTime:");
	LengthAlign(Buff,20);
	str.Format("%04d-%02d-%02d %02d:%02d:%02d",sEndTime.wYear,sEndTime.wMonth,sEndTime.wDay,sEndTime.wHour,sEndTime.wMinute,sEndTime.wSecond);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Result:");
	LengthAlign(Buff,20);
	str.Format("%c",nType?'P':'F');
	//if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT"))
	//{
	//	if(nLinkRes)
	//	{
	//		str.Format("F");
	//	}
	//}
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"ErrorCode:");
	LengthAlign(Buff,20);
	if(nType)
	{
		csMESInfo.AppendFormat("%sNA\r\n",Buff);
	}else
	{
		if(strlen(SFIS.czEC)<4)
		{
			strcpy(SFIS.czEC,"FAIL");
		}
		csMESInfo.AppendFormat("%s%s[ErrorCode(%s%s)]\r\n",Buff,SFIS.czSN,AtsInfo.TestFunc.STATION.csStationName.Left(1),SFIS.czEC);
	}
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"UserName:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,AtsInfo.csUserName);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"WO:");
	LengthAlign(Buff,20);
	GetCString(csTemp,"Order",":","Error",str);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str.Trim());
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Model:");
	LengthAlign(Buff,20);
	GetCString(csTemp,"Model No",":","Revision",str);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str.Trim());
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Model Rev:");
	LengthAlign(Buff,20);
	GetCString(csTemp,"Revision",":","Work",str);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str.Trim());
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"FailItemNumber:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%sNA\r\n",Buff);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"TestTime:");
	LengthAlign(Buff,20);
	if(sEndTime.wHour<sStartTime.wHour)
	{
		nHour=23-((int)sStartTime.wHour)+((int)sEndTime.wHour);
		nMinute=59-((int)sStartTime.wMinute)+((int)sEndTime.wMinute);
		nSec=60-((int)sStartTime.wSecond)+((int)sEndTime.wSecond);
		str.Format("%d",3600*nHour+60*nMinute+nSec);
	}else
	{
		nHour=((int)sEndTime.wHour)-((int)sStartTime.wHour);
		nMinute=((int)sEndTime.wMinute)-((int)sStartTime.wMinute);
		nSec=((int)sEndTime.wSecond)-((int)sStartTime.wSecond);
		str.Format("%d",3600*nHour+60*nMinute+nSec);
	}
	csMESInfo.AppendFormat("%s%s\r\n",Buff,str);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"TestType:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%sNormalPrd\r\n",Buff);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Test SoftwareRev:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%s%s\r\n",Buff,AtsInfo.TestFunc.STATION.m_DllVersion);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Test DataSheetRev:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%sA01\r\n",Buff);
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,"Test Environment:");
	LengthAlign(Buff,20);
	csMESInfo.AppendFormat("%sROOM\r\n",Buff);
	csMESInfo.Append("==========================================================================\r\n[START_TEST_LIST]\r\n");
	csMESInfo.Append("Item\tTestName\tHightLimit\tLowLimit\tReading\tResult\tUnit\r\n");
	//if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RC"))
	//{
	//	for(int i=0;i<4;i++)
	//	{
	//		if(i==3)
	//		{
	//			if(nType)
	//			{
	//				LogRes[i].csLogRes="Pass";
	//			}else
	//			{
	//				LogRes[i].csLogRes="Fail";
	//			}
	//		}else
	//		{
	//			LogRes[i].csLogRes="Pass";
	//		}
	//	}
	//	csMESInfo.AppendFormat("0\tUSBVoltageTest\tNA\tNA\tNA\t%s\tNA\r\n",LogRes[0].csLogRes);
	//	csMESInfo.AppendFormat("1\tDUT_SysInfo_Compare\tNA\tNA\tNA\t%s\tNA\r\n",LogRes[1].csLogRes);
	//	csMESInfo.AppendFormat("2\Wireless_Security_Mode_Check\tNA\tNA\tNA\t%s\tNA\r\n",LogRes[2].csLogRes);
	//	csMESInfo.AppendFormat("3\Wireless_Key_Check\tNA\tNA\tNA\t%s\tNA\r\n",LogRes[3].csLogRes);
	//}
	//else
	//{
	//for(int i=0;i<m_LogCount;i++)
	//{
	//	if(i==m_LogCount-1)
	//	{
	//		if(nType)
	//		{
	//			str="Pass";
	//		}else
	//		{
	//			str="Fail";
	//		}
	//	}else
	//	{
	//		str="Pass";
	//	}
	//	if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RF"))
	//	{
	//		switch(i)
	//		{
	//		case 0:
	//			LogRes[i].csHightLimit="20";
	//			LogRes[i].csLowLimit="-20";
	//			break;
	//		case 1:
	//			LogRes[i].csHightLimit="-25";
	//			LogRes[i].csLowLimit="-100";
	//			break;
	//		case 2:
	//			LogRes[i].csHightLimit="15";
	//			LogRes[i].csLowLimit="12";
	//			break;
	//		case 3:
	//			LogRes[i].csHightLimit="NA";
	//			LogRes[i].csLowLimit="NA";
	//			break;
	//		case 4:
	//			LogRes[i].csHightLimit="10";
	//			LogRes[i].csLowLimit="0";
	//			break;
	//		case 5:
	//			LogRes[i].csHightLimit="-15";
	//			LogRes[i].csLowLimit="-100";
	//			break;
	//		case 6:
	//			LogRes[i].csHightLimit="20";
	//			LogRes[i].csLowLimit="-20";
	//			break;
	//		case 7:
	//			LogRes[i].csHightLimit="-25";
	//			LogRes[i].csLowLimit="-100";
	//			break;
	//		case 8:
	//			LogRes[i].csHightLimit="15";
	//			LogRes[i].csLowLimit="12";
	//			break;
	//		case 9:
	//			LogRes[i].csHightLimit="NA";
	//			LogRes[i].csLowLimit="NA";
	//			break;
	//		case 10:
	//			LogRes[i].csHightLimit="10";
	//			LogRes[i].csLowLimit="0";
	//			break;
	//		case 11:
	//			LogRes[i].csHightLimit="-15";
	//			LogRes[i].csLowLimit="-100";
	//			break;
	//		}
	//		csMESInfo.AppendFormat("%d\t%s\t%s\t%s\t%s\t%s\tdB\r\n",i,LogRes[i].csLogTitle,LogRes[i].csHightLimit,LogRes[i].csLowLimit,LogRes[i].csLogRes,str);
	//	}
	//	if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("TXRX"))
	//	{
	//		if(-1!=LogRes[i].csLogTitle.Find("Spec"))
	//		{
	//			LogRes[0].csLowLimit=LogRes[i].csLogRes;
	//			LogRes[0].csHightLimit="100";
	//		}
	//		if(-1!=LogRes[i].csLogTitle.Find("Throughput"))
	//		{
	//			csMESInfo.AppendFormat("0\t%s\t%s\t%s\t%s\t%s\tMbps\r\n",LogRes[i].csLogTitle,LogRes[0].csHightLimit,LogRes[0].csLowLimit,LogRes[i].csLogRes,str);
	//		}
	//		if(-1!=LogRes[i].csLogTitle.Find("ANT0"))
	//		{
	//			csMESInfo.AppendFormat("1\t%s\t-20\t-60\t%s\t%s\tdB\r\n",LogRes[i].csLogTitle,LogRes[i].csLogRes,str);
	//		}
	//		if(-1!=LogRes[i].csLogTitle.Find("ANT1"))
	//		{
	//			csMESInfo.AppendFormat("2\t%s\t-20\t-60\t%s\t%s\tdB\r\n",LogRes[i].csLogTitle,LogRes[i].csLogRes,str);
	//		}
	//	}
	//	if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT"))
	//	{
	//		if(-1!=LogRes[i].csLogRes.Find("PASS"))
	//		{
	//			LogRes[i].csHightLimit="NA";
	//			LogRes[i].csLowLimit="NA";
	//			LogRes[i].csLogRes="NA";
	//		}
	//		if(-1!=LogRes[i].csLogTitle.Find("LANMAC"))
	//		{
	//			LogRes[i].csHightLimit="NA";
	//			LogRes[i].csLowLimit="NA";
	//		}
	//		if(-1!=LogRes[i].csLogTitle.Find("Throughput"))
	//		{
	//			LogRes[i].csHightLimit="100";
	//			LogRes[i].csLowLimit="80";
	//		}
	//		csMESInfo.AppendFormat("%d\t%s\t%s\t%s\t%s\t%s\tNA\r\n",i,LogRes[i].csLogTitle,LogRes[i].csHightLimit,LogRes[i].csLowLimit,LogRes[i].csLogRes,str);
	//	}
	//}
	//}

	csMESInfo.Append("==========================================================================\r\n[END_TEST_LIST]\r\n");
	csMESInfo.Append("==========================================================================\r\n[END_LOG]");
	csMESInfo.Remove('昍');
	return csMESInfo;
}
bool CATSDlg::MakeMESLog(bool nType,CString &csLogMessage)
{
	CStdioFile LogFile;
	CFileStatus FileSta;
	CString csSN,csTemp,csLogName,str;
	GetLocalTime(&sTime);
	csSN.Format("%s",SFIS.czSN);
	if(nType)
	{
		str.Format("%04d",sTime.wYear);
		str.Trim();
		//if(-1!=AtsInfo.TestFunc.SFIS.csStep.Find("FT")||-1!=AtsInfo.TestFunc.SFIS.csStep.Find("RC"))
		//{
		//	csSN.Format("%s",SFIS.czSSN1);
		//	csLogName.Format("PASS_%s_%s%02d%02d%02d%02d%02d.log",csSN,str.Right(2),sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond);
		//}else
		//{
			csLogName.Format("PASS_%s_%s%02d%02d%02d%02d%02d.log",csSN,str.Right(2),sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond);
		//}
	}
	else
	{
		str.Format("%04d",sTime.wYear);
		str.Trim();
		csLogName.Format("FAIL_%s_%s%02d%02d%02d%02d%02d.log",csSN,str.Right(2),sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond);
	}
	
	//csTemp.Format("%sLog\\%s\\FTP_Log",AtsInfo.Log.csLogFilePath,AtsInfo.m_csModelName);
		csTemp.Format("%sModels\\%s\\Log\\FTP_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	::CreateDirectory(csTemp,NULL);
	str.Format("%s\\%s",csTemp,csLogName);
	csToMESLogPath.Format("%s",str);
	if(LogFile.GetStatus(str,FileSta))
	{
		if(!LogFile.Open(str,CFile::modeReadWrite))
		{
			LogFile.Close();
			csTemp.Format("Open Ftp Log File Fail!");
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
			csToMESLogPath.Empty();
			return FALSE;
		}else
		{
			LogFile.SeekToEnd();
		}
	}
	else
	{
		if(!LogFile.Open(str,CFile::modeCreate|CFile::modeReadWrite))
		{
			WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
			Pause(1000);
			if(!LogFile.Open(str,CFile::modeReadWrite))
			{
				csTemp.Format("Create Ftp Log File Fail!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
				LogFile.Close();
				csToMESLogPath.Empty();
				return FALSE;
			}
		}
	}
	LogFile.WriteString(csLogMessage);
	LogFile.Close();
	return TRUE;
}
CString CATSDlg::MakeNetLog(bool nType)
{
	CStdioFile LogFile;
	CFileStatus FileSta;
	CString csTemp,csSN,csLogName,str;
	csSN.Format("%s",SFIS.czSN);
	if(nType)
	{
		csLogName.Format("%s_%04d%02d%02d%02d%02d%02d_PASS.txt",csSN,sEndTime.wYear,sStartTime.wMonth,sStartTime.wDay,sStartTime.wHour,sStartTime.wMinute,sStartTime.wSecond);
	}else
	{
		csLogName.Format("%s_%04d%02d%02d%02d%02d%02d_FAIL.txt",csSN,sEndTime.wYear,sStartTime.wMonth,sStartTime.wDay,sStartTime.wHour,sStartTime.wMinute,sStartTime.wSecond);
	}
	
	//csTemp.Format("%sLog\\%s\\UPLoad_Log",AtsInfo.Log.csLogFilePath,AtsInfo.m_csModelName);
		csTemp.Format("%sModels\\%s\\Log\\UPLoad_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	::CreateDirectory(csTemp,NULL);

	str.Format("%s\\%s",csTemp,csLogName);
	csLogName=str;
	if(LogFile.GetStatus(str,FileSta))
	{
		if(!LogFile.Open(str,CFile::modeReadWrite))
		{
			LogFile.Close();
			csTemp.Format("Open Upload Log File Fail!");
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
			return "";
		}else
		{
			LogFile.SeekToEnd();
		}
	}
	else
	{
		if(!LogFile.Open(str,CFile::modeCreate|CFile::modeReadWrite))
		{
			WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
			Pause(1000);
			if(!LogFile.Open(str,CFile::modeReadWrite))
			{
				csTemp.Format("Create UpLoad Log File Fail!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
				LogFile.Close();
				return "";
			}
		}
	}
	csTemp.Format("%s\n",AtsInfo.TestFunc.STATION.csStationName);
	csTemp.AppendFormat("%04d%02d%02d%02d%02d%02d\n",sStartTime.wYear,sStartTime.wMonth,sStartTime.wDay,sStartTime.wHour,sStartTime.wMinute,sStartTime.wSecond);
	csTemp.AppendFormat("%04d%02d%02d%02d%02d%02d\n",sEndTime.wYear,sEndTime.wMonth,sEndTime.wDay,sEndTime.wHour,sEndTime.wMinute,sEndTime.wSecond);
	csTemp.AppendFormat("%s\n",nType?"PASS":"FAIL");
	csTemp.AppendFormat("%s\n",SFIS.czEC);
	csTemp.AppendFormat("%s\n",SFIS.czMAC);
	//CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
	//str.Empty();
	//p->GetWindowTextA(str);
	str=csTotalMes;
	csTotalMes.Empty();
	csTemp.AppendFormat("DETAIL\n%s",str);
	LogFile.WriteString(csTemp);
	LogFile.Close();
	return csLogName;
}
void CATSDlg::GetUiMsg()//把UI上的內容儲存為LOG
{
	CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
	CString GetResult;

	CStdioFile fuifile;
	CFileStatus fstatus;
	CString csTmp,csLogName,csSN,csMac;
	int nLen,nLen1;

	csSN.Format("%s",SFIS.czSN);
	csSN.Trim();
	csMac.Format("%s",SFIS.czMAC);
	csMac.Trim();
	csLogName.Format("%s_%s_%s_%s",csSN,csMac,UiLogName,AtsInfo.Log.csStationNum);
   
	//csTmp.Format("%sLog\\%s\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	csTmp.Format("%sModels\\%s\\Log\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	::CreateDirectory(csTmp, NULL);
	//csTmp.Format("%sLog\\%s\\UI_Log\\%s.txt", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName, csLogName);
	csTmp.Format("%sModels\\%s\\Log\\UI_Log\\%s.txt", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName, csLogName);
	
	//csUILog.Format("%sLog\\%s\\UI_Log\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName, csLogName);
	csUILog.Format("%sModels\\%s\\Log\\UI_Log\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,csLogName);


	if ( fuifile.GetStatus(csTmp, fstatus) )
		{
		
			if ( !fuifile.Open(csTmp, CFile::modeReadWrite) )
			{
				
				ShowMessage("open ui log file error! Try again.");
				if ( !fuifile.Open(csTmp, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					fuifile.Close();
					exit(0);
					//return FALSE;
				}
			}
			else
				{
				fuifile.SeekToEnd();
				}

		}
	else
		{
			
			if ( !fuifile.Open(csTmp, CFile::modeCreate | CFile::modeReadWrite ) )
			{
				
				ShowMessage("open ui log file error! Try again.");
				//kill excel task
				WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
				Pause(1000);
				if ( !fuifile.Open(csTmp, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					fuifile.Close();
					exit(0);
					//return FALSE;
				}
			}
		}
	p->GetWindowText(GetResult);
	fuifile.WriteString(GetResult);
	fuifile.Close();
	p->SetWindowText("");

}
int GetLogUploadInfo()
{
	CStdioFile flogfile;
	CFileStatus flogstatus;
	CString csTmp,csTemp,tmp;
	//Download record file from server
//	csTmp.Format("%sLog\\%s\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
    csTmp.Format("%sModels\\%s\\Log\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	csTemp.Format("%sLog\\%s\\%s\\Log_Upload_Info\\LogUploadInfo.log",AtsInfo.Log.csLogMappingDriver.GetBuffer(),AtsInfo.m_csModelName.GetBuffer(),AtsInfo.Log.csComputerName.GetBuffer());

	CopyFile(csTemp.GetBuffer(),csTmp.GetBuffer(),FALSE);
	return GetLastError();
}
DWORD OnMountNetDriver()
{
	CString csTmp;
	DWORD rslt;
	csTmp = AtsInfo.Log.csLogMappingDriver;
	csTmp.Remove('\\');
	NETRESOURCE netres;
	netres.dwScope=RESOURCE_GLOBALNET;
	netres.dwType=RESOURCETYPE_ANY;
	netres.dwDisplayType=RESOURCEDISPLAYTYPE_GENERIC;
	netres.dwUsage=RESOURCEUSAGE_CONNECTABLE;
	netres.lpLocalName=csTmp.GetBuffer();
	netres.lpRemoteName=AtsInfo.Log.csLogServerPath.GetBuffer();
	netres.lpComment=NULL;
	netres.lpProvider=NULL;
	if (AtsInfo.Log.csUser.IsEmpty())
		rslt = WNetAddConnection2(&netres, NULL, NULL, CONNECT_REDIRECT);
	else
		rslt = WNetAddConnection2(&netres, AtsInfo.Log.csPassWord.GetBuffer(), AtsInfo.Log.csUser.GetBuffer(), CONNECT_REDIRECT);
//	rslt = WNetAddConnection2(&netres, NULL, NULL, CONNECT_UPDATE_PROFILE);
	return rslt;
}
DWORD OnUnMountNetDriver()
{
	CString csTmp;
	csTmp = AtsInfo.Log.csLogMappingDriver;
	csTmp.Remove('\\');
	DWORD rslt=WNetCancelConnection2(csTmp.GetBuffer(), CONNECT_UPDATE_PROFILE, TRUE);
	return rslt;
}
void WriteLogNetDriverInfo(CString csOper, DWORD ErrorCode)
{
	CStdioFile flogfile;
	CFileStatus flogstatus;
	CString csTmp,csTemp;
	
	GetLocalTime(&sTime);
	
//	csTmp.Format("%sLog\\%s\\Log_Upload_Info\\NetDriver.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
    csTmp.Format("%sModels\\%s\\Log\\Log_Upload_Info\\NetDriver.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
//	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTmp,0);

	if ( flogfile.GetStatus(csTmp, flogstatus) )
		{
			if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
			{
				csTemp.Format("open net driver record file error! Try again.");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
				if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
				{
					csTemp.Format("open net driver record file error!Check the file path, please!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
//					AfxMessageBox("Log Upload Record File 路徑錯誤, 請重新設定!", MB_OK, 0);
//					flogfile.Close();
//					exit(0);
					//return FALSE;
				}
			}
			flogfile.SeekToEnd();

		}
	else
		{
			if ( !flogfile.Open(csTmp, CFile::modeCreate | CFile::modeReadWrite ) )
			{
				csTemp.Format("open net driver record file error! Try again...");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
				if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
				{
					csTemp.Format("open net driver record file error!Check the file path, please!!!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTemp,0);
//					AfxMessageBox("Log Upload Record File 路徑錯誤, 請重新設定?", MB_OK, 0);
//					flogfile.Close();
//					exit(0);
					//return FALSE;
				}
			}
		}
	csTemp.Format("[%04d%02d%02d-%02d:%02d:%02d]%s:%d\r\n",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond,csOper,ErrorCode);
	flogfile.WriteString(csTemp);
	flogfile.Close();

}

void WriteLogUploadInfo(CString csFileName,CString csResult)
{
	CStdioFile flogfile;
	CFileStatus flogstatus;
	CString csTmp,csTemp;
	
//	csTmp.Format("%sLog\\%s\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	csTmp.Format("%sModels\\%s\\Log\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);

	if ( flogfile.GetStatus(csTmp, flogstatus) )
		{
			if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
			{
				if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
				{
					AfxMessageBox("無法打開日誌文件，請架設檢查本地log路逕設置是否正確！", MB_OK, 0);
//					exit(0);
				}
			}
			flogfile.SeekToEnd();
		}
	else
		{
			if(GetLogUploadInfo()==53)	
			{
				AfxMessageBox("下載日誌文件失敗，請檢查網絡...",MB_OK,0);
//				exit(0);
			}
			else
			{
				if ( flogfile.GetStatus(csTmp, flogstatus) )
				{
					if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
					{
						if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
						{
							AfxMessageBox("無法打開日誌文件，請架設檢查本地log路逕設置是否正確！", MB_OK, 0);
//							exit(0);
						}
					}
					flogfile.SeekToEnd();
				}
				else
				{
					if ( !flogfile.Open(csTmp, CFile::modeCreate | CFile::modeReadWrite ) )
					{
						if ( !flogfile.Open(csTmp, CFile::modeReadWrite) )
						{
							AfxMessageBox("無法生成日誌文件，請架設檢查本地log路逕設置是否正確！", MB_OK, 0);
//							exit(0);
						}
					}
					GetLocalTime(&sTime);
					csTemp.Format("[%04d%02d%02d-%02d:%02d:%02d]Initializing...\r\n",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond);
					flogfile.WriteString(csTemp.GetBuffer());
				}
			}
		}
	csTemp.Format("[%04d%02d%02d-%02d:%02d:%02d] %s %s\r\n",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond,csFileName,csResult);
	flogfile.WriteString(csTemp);
	flogfile.Close();

}
bool LogServerDirectoryIni()
{
	CString csDestPath;
	csDestPath.Format("%sLog",AtsInfo.Log.csLogMappingDriver.GetBuffer());
	::CreateDirectory(csDestPath, NULL);
	csDestPath.Format("%sLog\\%s",AtsInfo.Log.csLogMappingDriver.GetBuffer(),AtsInfo.m_csModelName.GetBuffer());
    ::CreateDirectory(csDestPath, NULL);
	csDestPath += "\\";
    csDestPath += AtsInfo.TestFunc.STATION.csStationName;
	::CreateDirectory(csDestPath, NULL);

	csDestPath += "\\";
    csDestPath += AtsInfo.Log.csComputerName;
	::CreateDirectory(csDestPath, NULL);

	//csDestPath.Format("%sLog\\%s\\%s",AtsInfo.Log.csLogMappingDriver.GetBuffer(),AtsInfo.m_csModelName.GetBuffer(),AtsInfo.Log.csComputerName.GetBuffer());
	//::CreateDirectory(csDestPath, NULL);
	
	::CreateDirectory(csDestPath+"\\PASS", NULL);
	::CreateDirectory(csDestPath+"\\FAIL", NULL);
	::CreateDirectory(csDestPath+"\\Log_Upload_Info", NULL);
//	::CreateDirectory(csDestPath+"\\UI_Log", NULL);
	if(::PathFileExists(csDestPath))
	{
		WriteLogUploadInfo(csDestPath," Succeed to create Directory"); 
	}
	else
	{
		WriteLogUploadInfo(csDestPath," Fail to create Directory"); 
		return false;	
	}

	return true;

}
int CopyFileMode(CString csSrcPath,CString csFileName,int nFileMode)
{
	CString csDestPath,csBackupPath;
	switch(nFileMode)
	{
	case TOTAL_RESULT:
				csDestPath.Format("%sLog\\%s\\%s\\%s\\%s",AtsInfo.Log.csLogMappingDriver,AtsInfo.m_csModelName,AtsInfo.TestFunc.STATION.csStationName,AtsInfo.Log.csComputerName,csFileName);
				csBackupPath.Format("%sLog\\%s\\BACKUP\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,csFileName);
				break;
	case PASS_ONLY:	
				csDestPath.Format("%sLog\\%s\\%s\\%s\\PASS\\%s",AtsInfo.Log.csLogMappingDriver,AtsInfo.m_csModelName,AtsInfo.TestFunc.STATION.csStationName,AtsInfo.Log.csComputerName,csFileName);
				csBackupPath.Format("%sLog\\%s\\BACKUP\\PASS\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,csFileName);
				break;
	case FAIL_ONLY:		
				csDestPath.Format("%sLog\\%s\\%s\\%s\\FAIL\\%s",AtsInfo.Log.csLogMappingDriver,AtsInfo.m_csModelName,AtsInfo.TestFunc.STATION.csStationName,AtsInfo.Log.csComputerName,csFileName);
				csBackupPath.Format("%sLog\\%s\\BACKUP\\FAIL\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,csFileName);
				break;
	case UPLOAD_RECORD:
				csDestPath.Format("%sLog\\%s\\%s\\%s\\Log_Upload_Info\\%s",AtsInfo.Log.csLogMappingDriver,AtsInfo.m_csModelName,AtsInfo.TestFunc.STATION.csStationName,AtsInfo.Log.csComputerName,csFileName);
				csBackupPath.Format("%sLog\\%s\\BACKUP\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,csFileName);
				break;
	case UPLOAD_UILOG:
				csDestPath.Format("%sLog\\%s\\%s\\%s\\UI_Log\\%s",AtsInfo.Log.csLogMappingDriver,AtsInfo.m_csModelName,AtsInfo.TestFunc.STATION.csStationName,AtsInfo.Log.csComputerName,csFileName);
				csBackupPath.Format("%sLog\\%s\\BACKUP\\UI_Log\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName,csFileName);
				break;
	}
//	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csDestPath,0);
//	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csSrcPath,0);
//	::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csBackupPath,0);
/*	int nCopyFlag = MyCopyFile(csSrcPath,csDestPath,csBackupPath);
	if(nCopyFlag == -1)
	{
		WriteLogUploadInfo(csFileName,"UpLoadAndBackUp   FAIL");
//		AfxMessageBox("Fail to open local file! Check the file path, please!", MB_OK, 0);
		return -1;
	}
	else if(nCopyFlag == -2)
	{
		WriteLogUploadInfo(csFileName,"UpLoadAndBackUp   FAIL");
//		AfxMessageBox("Fail to open remote file! Check the file path, please!", MB_OK, 0);
		return -2;
	}
	else
	{
		if(nFileMode!=4&&csFileName.Find("Only")==-1)
		{
		WriteLogUploadInfo(csFileName,"UpLoadAndBackUp   PASS");
		}
		return 0;
	}*/
	return 0;
}
/**********************************************************************************
FUNCTION_NAME             CopyFile
DESC :  copy a file from local  to remote 
INPUT:
   srcPath:  path of source file
   destPath: path of destination file
OUTPUT:
    N/A
RESULT
   0 : OK
   -1 : fail to open local file 
   -2 ：fail to open remote file    
**********************************************************************************/
//int MyCopyFile(CString srcPath,CString destPath,CString backupPath)
int MyCopyFile(CString srcPath,CString destPath)
{
	CFile destFile;
	CFile srcFile;
	CFile backupFile;
	char readBuffer[2048] = {0}; 
#define LEN_EACH_READ 1400
	if(srcFile.Open(srcPath.GetBuffer(),CFile::modeRead|CFile::typeBinary,NULL))
	{
		int len = srcFile.GetLength();
        if( len > 0 )
		{
				if(destFile.Open(destPath.GetBuffer(),CFile::modeCreate|CFile::modeReadWrite|CFile::typeBinary,NULL)/*&&
					backupFile.Open(backupPath.GetBuffer(),CFile::modeCreate|CFile::modeReadWrite|CFile::typeBinary,NULL)*/)
				{
					for(int i = 0 ; i < len/LEN_EACH_READ ; i++)
					{
					memset(readBuffer,0,sizeof(readBuffer));
                    srcFile.Read(readBuffer,LEN_EACH_READ);
					destFile.Write(readBuffer,LEN_EACH_READ);
//					backupFile.Write(readBuffer,LEN_EACH_READ);	
					}
					srcFile.Read(readBuffer,len%LEN_EACH_READ);
					destFile.Write(readBuffer,len%LEN_EACH_READ);
//					backupFile.Write(readBuffer,len%LEN_EACH_READ);
				}
				else
				{
					srcFile.Close();
					return -2;
				}

		}
	}
	else
	{
			return -1;
	}
	destFile.Close();
	srcFile.Close();
//	backupFile.Close();
	return 0;
}

UINT HandleLogUpload(LPVOID pParam)
{
	CString csHour,csMin,csHour1,csMin1,csTmp, csTmp2;
	DWORD rslt;
	BOOL bUploadStatus = false;
	BOOL bCreateDirectory = false;
//	csHour = AtsInfo.Log.csLogUploadTime.Left(AtsInfo.Log.csLogUploadTime.Find(':'));
//	csMin = AtsInfo.Log.csLogUploadTime.Right(AtsInfo.Log.csLogUploadTime.GetLength()-AtsInfo.Log.csLogUploadTime.Find(":")-1);
//	csHour1 = AtsInfo.Log.csLogUploadTime1.Left(AtsInfo.Log.csLogUploadTime1.Find(':'));
//	csMin1 = AtsInfo.Log.csLogUploadTime1.Right(AtsInfo.Log.csLogUploadTime1.GetLength()-AtsInfo.Log.csLogUploadTime1.Find(":")-1);
	exec("net start \"computer browser\"");
	OnUnMountNetDriver();
	rslt = OnMountNetDriver();
	bool bFirstTransfer = true;
	if(!rslt)
	{
		csTmp.Format("Upload Status: 已與伺服器正常連接,可以正常上傳");
		::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
	}
	else
	{
		AfxMessageBox("LOG無法上傳,請聯繫架設工程師,確認網路連接");
		csTmp.Format("Upload Status: 無法上傳,網路連接有問題");
		::SetDlgItemTextA(UI_hWnd, IDC_UPLOAD_STATUS, csTmp.GetBuffer());
	}
/*	if (!LogServerDirectoryIni())
	{
		AfxMessageBox("LOG無法上傳,請聯繫架設工程師,確認網路連接");
		csTmp.Format("Upload Status: 無法上傳,網路連接有問題");
		::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
	}
	else
	{
		bCreateDirectory = true;
		csTmp.Format("Upload Status: 已與伺服器正常連接,可以正常上傳");
		::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
	}
*/	while(1)
	{
		GetLocalTime(&sTime);
		CString csTime;
		csTime.Format("%04d%02d%02d-%02d:%02d:%02d",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond);
//		ShowMessage(csTime);
//		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&csTime,0);
//		if( (bFirstTransfer) ||(sTime.wHour==atoi(csHour)&&sTime.wMinute==atoi(csMin))||(sTime.wHour==atoi(csHour1)&&sTime.wMinute==atoi(csMin1)))
		if ((bFirstTransfer) || (sTime.wMinute%3==0) )
		{
			
		    bFirstTransfer =false;
						
			nUpFiles = 0;
			
			bUploadStatus = true;
//			AfxMessageBox("系統時間:"+csTime,MB_OK,0);
			exec("net start \"computer browser\"");
/*			if(!bCreateDirectory)
			{
				OnUnMountNetDriver();
				rslt = OnMountNetDriver();
				if(!rslt)
				{
					csTmp.Format("Upload Status: 已與伺服器正常連接,可以正常上傳");
					::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
				}
				if(LogServerDirectoryIni())
				{
					bCreateDirectory = true;
				}
				else
				{
					AfxMessageBox("LOG無法上傳,請聯繫架設工程師,確認網路連接");
					csTmp.Format("Upload Status: [%s] 無法上傳,網路連接有問題",csTime);
					::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
					Sleep(40000);
					continue;
				}
			}
*/			csTmp.Format("Upload Status: 開始上傳");
			::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
			for(int i=0;i<3;i++)
			{
				if(!ListFolder(csLogPath,2))
				{
					OnUnMountNetDriver();
					rslt = OnMountNetDriver();
					if(!rslt)
					{
						csTmp.Format("Upload Status: 已與伺服器正常連接,可以正常上傳");
						::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
						WriteLogNetDriverInfo("Add NetWork Disk OK",rslt);
						continue;
					}
					else
					{					
						AfxMessageBox("LOG無法上傳,請聯繫架設工程師,確認網路連接");
						csTmp.Format("Upload Status: 上傳失敗");
						::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
						WriteLogNetDriverInfo("Add NetWork Disk Fail",rslt);
					}
				}
				else
				{
					
				//	csTmp.Format("%sLog\\%s\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
					csTmp.Format("%sModels\\%s\\Log\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
					ListFolder(csTmp,1);
					csTmp.Format("Upload Status: [%s] 上傳成功 上傳%d個文件",csTime,nUpFiles-1);
					::SetDlgItemTextA(UI_hWnd,IDC_UPLOAD_STATUS,csTmp.GetBuffer());
					WriteLogUploadInfo(csTmp,"");
				//	csTmp.Format("%sLog\\%s\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
					csTmp.Format("%sModels\\%s\\Log\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
					CopyFileMode(csTmp,"LogUploadInfo.log",4);
					break;
				}
			}
		}
		Sleep(40000);
	}
	return TRUE;

}
bool ListFolder(CString sPath,int nUpFlag)  //nUpFlag=1---upload all log files; nUpFlag=2---upload all past log files
{
    CFileFind filefind;
    BOOL bFound;
	CString sFileName,csTemp;
	UINT UploadCounter = 0;
	int Flag;
	CWnd* pWnd = NULL;
	pWnd = CWnd::FindWindow(NULL,"MyMessageBox");
	
//	AfxMessageBox(sPath,MB_OK,0);
    bFound = filefind.FindFile(sPath + "\\*.*");    //找第一個文件
	while(bFound)    //如果找到,繼續
    {
        bFound = filefind.FindNextFile();
		CString sFilePath = filefind.GetFilePath();

        if(filefind.IsDirectory())   //如果是目錄,注意任何一個目錄都包括.和..目錄
        { 
			if(!filefind.IsDots())    //去除.和..目錄_
			{
				if(!ListFolder(sFilePath,nUpFlag))				//遞迴下一層目錄
				{
					return false;
				}
			}
        } 
		else 
		{ 
			nUpFiles++;
			sFileName = filefind.GetFileName();
			if(sFileName.Find("PASS_Only")!=-1)
			{
			Flag = 2;
			}
			else if(sFileName.Find("FAIL_Only")!=-1)
			{
			Flag = 3;
			}
			else if(sFileName.Find(".txt")!=-1)
			{
			
			Flag = 5;
			}
			else
			{
			Flag = 1;
			}
			csTemp.Format("%04d%02d%02d",sTime.wYear,sTime.wMonth,sTime.wDay);
			if(nUpFlag == 1)
			{
				if(CopyFileMode(sFilePath,sFileName,Flag)!=0)
					{
						return false;
					}
				UploadCounter ++;
				if(UploadCounter >= 5)
					{
					UploadCounter = 0;
					if(pWnd)
						{
						pWnd->PostMessage(WM_UPLOAD_UPDATE,nUpFiles,0);
						}
					}
//				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&sFilePath,0);    //枚舉到的檔案名字
//				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&sFileName,0);    //枚舉到的檔案名字
			}
			if(nUpFlag == 2)
			{
				if(sFileName.Find(csTemp)==-1)
				{
					if(CopyFileMode(sFilePath,sFileName,Flag)!=0)
					{
						return false;
					}
				 UploadCounter ++;
				if(UploadCounter >= 5)
					{
					UploadCounter = 0;
					if(pWnd)
						{
						pWnd->PostMessage(WM_UPLOAD_UPDATE,nUpFiles,0);
						}
					}
//				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&sFilePath,0);    //枚舉到的檔案名字
//				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&sFileName,0);    //枚舉到的檔案名字
				}
			}
			if(sFileName.Find(csTemp)==-1)
			{
			DeleteFile(sFilePath.GetBuffer());
			}
		}
    }
    filefind.Close();
	return true;
}

char csErrMsgBuffer[100];
DWORD CreateUICloseUploadThread(LPVOID lpParam)
{
	CWnd* pWnd = NULL;
	CString csTmp,csTime;
	DWORD rslt;
	nUpFiles = 0;
	exec("net start \"computer browser\"");
	if(!LogServerDirectoryIni())
	{
		for(int j = 0; j<3; j++)
		{
			OnUnMountNetDriver();
		    rslt = OnMountNetDriver();
			if(!rslt)
			{
				break;
			}
			else
			{
				WriteLogNetDriverInfo("Add",rslt);
			}
		}
		if(!LogServerDirectoryIni())
		{
			pWnd = CWnd::FindWindow(NULL,"MyMessageBox");
			if(pWnd)
			{
				strcpy(csErrMsgBuffer,AtsInfo.Log.csLogMappingDriver.GetBuffer());
			    csTmp.ReleaseBuffer();
				pWnd->PostMessage(WM_UPLOAD_UPDATE,0,1);
				//pWnd->SendMessage(WM_COMMAND,IDC_BUTTON_RETRY,0);
			}
			return TRUE;
		}
	}
	GetLocalTime(&sTime);
	if(!ListFolder(csLogPath,1))
	{
		pWnd = CWnd::FindWindow(NULL,"MyMessageBox");
		if(pWnd)
		{
//			AfxMessageBox("Fail Find",MB_OK,0);
			strcpy(csErrMsgBuffer,csLogPath.GetBuffer());
			csTmp.ReleaseBuffer();
			pWnd->PostMessage(WM_UPLOAD_UPDATE,0,1);
			//pWnd->SendMessage(WM_COMMAND,IDC_BUTTON_RETRY,0);
		}
	}
	else
	{
		
		//csTmp.Format("%sLog\\%s\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		csTmp.Format("%sModels\\%s\\Log\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		ListFolder(csTmp,1);
		csTime.Format("%04d%02d%02d-%02d:%02d:%02d",sTime.wYear,sTime.wMonth,sTime.wDay,sTime.wHour,sTime.wMinute,sTime.wSecond);
		csTmp.Format("Upload Status: [%s] 上傳成功 上傳%d個文件",csTime,nUpFiles);
		WriteLogUploadInfo(csTmp,"");
		//csTmp.Format("%sLog\\%s\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		csTmp.Format("%sModels\\%s\\Log\\Log_Upload_Info\\LogUploadInfo.log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
		CopyFileMode(csTmp,"LogUploadInfo.log",4);
		pWnd = CWnd::FindWindow(NULL,"MyMessageBox");
		if(pWnd)
		{
//			AfxMessageBox("PASS Find",MB_OK,0);
			pWnd->PostMessage(WM_UPLOAD_UPDATE,0,2);
			//pWnd->SendMessage(WM_COMMAND,IDC_BUTTON_OK,0);
		}

	}

	return TRUE;

}
static void exec(char * pCmd) // 用於保證命令執行完成后,才開始執行餘下代碼.確保順序執行.
{
	STARTUPINFO  si;
	ZeroMemory(&si,  sizeof(si));
	si.cb  =  sizeof(STARTUPINFO);

	PROCESS_INFORMATION  pi;
	BOOL  res  =  CreateProcess(NULL,
	pCmd,  
	NULL,
	NULL,
	NULL,
	NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
	NULL,
	NULL,
	&si,
	&pi);

	if  (TRUE  ==  res)
	{
		WaitForSingleObject(pi.hProcess,INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}


void CATSDlg::OnBnDoubleclickedButtonHelp()
{
	// TODO: 在此添加控件通知?理程序代?
		BOOL bPriv;
	CPasswordDLG dlg1(NULL,"janejeff");
	dlg1.DoModal();
	bPriv = dlg1.IsOK();
		if(nCancel)
	{
		return;
	}
	nDlgSettingCounter ++;
	if(nDlgSettingCounter > 1)
		{
		AfxMessageBox("Help and Setting Dialog Opened!");
		return;
		}
	pDlgSetting = new CDlgSetting(NULL,this,bPriv);
	pDlgSetting->Create(IDD_DIALOG_SETTING,NULL);
	
	pDlgSetting->ShowWindow(SW_SHOW);
	RECT  rect,rect1;
	this->GetWindowRect(&rect1);
	pDlgSetting->GetWindowRect(&rect);
	rect.left = rect1.right-(rect.right-rect.left);
	rect.right = rect1.right;	
	pDlgSetting->MoveWindow(&rect);
	pDlgSetting->SetFocus();
	//CDlgSetting dlg(NULL,this,bPriv);
	//dlg.DoModal();

}


void CATSDlg::OnBnClickedButtonHelp()
{
	// TODO: 在此添加控件通知?理程序代?
	OnBnDoubleclickedButtonHelp();
}

HBRUSH CATSDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何?性

	// TODO:  如果默?的不是所需??，?返回另一???
	//if(pWnd->GetDlgCtrlID()==m_TestStateShow.GetDlgCtrlID())
	//{
	//	pDC->SetTextColor(RGB(0,0,255));
	//	
	//	//CRect rc;
	//	//::GetClientRect(m_TestStateShow.m_hWnd,&rc);
	//	//CDC *pDC=m_TestStateShow.GetWindowDC();
	//	//pDC->FillSolidRect(&rc,RGB(0,255,0));
	//	//ReleaseDC(pDC);
	//	
	//}
	return hbr;
}
void CATSDlg::ReDrawWindow()
{
	CRect rect;
	::GetWindowRect(GetDlgItem(IDC_UI_MSG)->m_hWnd,&rect);
	ScreenToClient(&rect);
	CString csTemp;
	m_testStatus.GetWindowText(csTemp);
	if(0==csTemp.CompareNoCase("TEST"))
	{
		//m_TestStateCtrl.SetWindowText("測試中......");
		m_TestStateCtrl.SetWindowText("Testing......");
		m_TestStateCtrl.SetBkColor(RGB(255,255,0),0,CLabel::BackFillMode::Normal);
		m_TestStateCtrl.SetTextColor(RGB(255,0,0));
	}
	if(0==csTemp.CompareNoCase("PASS"))
	{
		//m_TestStateCtrl.SetWindowText("PASS 請掃條碼測試下一片");
		m_TestStateCtrl.SetWindowText("PASS Scan Barcode to test next one");
		m_TestStateCtrl.SetBkColor(RGB(0,255,0),0,CLabel::BackFillMode::Normal);
		m_TestStateCtrl.SetTextColor(RGB(0,0,255));
	}
	if(0==csTemp.CompareNoCase("FAIL"))
	{
		//m_TestStateCtrl.SetWindowText("FAIL 請掃條碼測試下一片");
		m_TestStateCtrl.SetWindowText("FAIL Scan Barcode to test next one");
		m_TestStateCtrl.SetBkColor(RGB(255,0,0),0,CLabel::BackFillMode::Normal);
		m_TestStateCtrl.SetTextColor(RGB(0,0,0));
	}
	if(0==csTemp.CompareNoCase("IDLE")||0==csTemp.CompareNoCase("DECT"))
	{
		//m_TestStateCtrl.SetWindowText("請掃條碼開始測試");
		m_TestStateCtrl.SetWindowText("Please scan Barcode to start test");
		m_TestStateCtrl.SetBkColor(RGB(255,255,255),0,CLabel::BackFillMode::Normal);
		m_TestStateCtrl.SetTextColor(RGB(0,0,0));
	}
	AfxGetApp()->GetMainWnd()->InvalidateRect(&rect,TRUE);
}
bool ftpLog()
{
	CInternetSession* pSession;
	CFtpConnection* pConnection;
	char ServerPath[MAX_PATH];
	sprintf(ServerPath,"%s\\\\%s",AtsInfo.ftp_Setting.m_ftpPath,AtsInfo.TestFunc.SFIS.csStep);
	CString csAtsSerPath,csUadPath,str,csTemp;
	int nPos=0;
	nPos=csToMESLogPath.ReverseFind('\\');
	if(-1==nPos)
	{
		return FALSE;
	}
	str=csToMESLogPath.Right(csToMESLogPath.GetLength()-nPos);
	str.Remove('\\');
	str.Trim();
	csAtsSerPath.Format("./ATSLOG/%s/%s",AtsInfo.TestFunc.SFIS.csStep,str);
	csUadPath.Format("./UADCLOG/%s/%s",AtsInfo.TestFunc.SFIS.csStep,str);
	pConnection=NULL;
	pSession=new CInternetSession(AfxGetAppName(),1,PRE_CONFIG_INTERNET_ACCESS);
	try
	{
		pConnection=pSession->GetFtpConnection(AtsInfo.ftp_Setting.m_ftpPath,AtsInfo.ftp_Setting.csUserName,AtsInfo.ftp_Setting.csPasswd,AtsInfo.ftp_Setting.nPort,TRUE);
	}
	catch(CInternetException* e)
	{
		e->Delete();
		pConnection=NULL;
		return FALSE;
	}	
	if(pConnection!=NULL)
	{
		if(!pConnection->PutFile(csToMESLogPath,csAtsSerPath))
		{
			DWORD nError;
			nError=::GetLastError();
			str.Format("上傳文件失敗,錯誤代碼:%d",nError);
			::AfxMessageBox(str,MB_OK,0);
			pConnection->Close();
			delete pConnection;
			delete pSession;
			return FALSE;
		}
		Sleep(500);
		if(!pConnection->PutFile(csToMESLogPath,csUadPath))
		{
			DWORD nError;
			nError=::GetLastError();
			str.Format("上傳文件失敗,錯誤代碼:%d",nError);
			::AfxMessageBox(str,MB_OK,0);
			pConnection->Close();
			delete pConnection;
			delete pSession;
			return FALSE;
		}	
	}
	//nFileLength=0;
	//nFileUadLen=0;
	//if(pConnection!=NULL)
	//{
	//	List(pConnection,"*",str);
	//}
	//if(nFileLength==0||nFileUadLen==0)
	//{
	//	return FALSE;
	//}
	pConnection->Close();
	delete pSession;
	return TRUE;
}
char* LengthAlign(char Buffer[],int nLenth)
{
	int dwLen=0;
	dwLen=strlen(Buffer);
	if(dwLen<nLenth)
	{
		for(dwLen;dwLen<nLenth;dwLen++)
		{
			Buffer[dwLen]=' ';
		}
	}
	return Buffer;
}
void List(CFtpConnection *pConnection,CString csPath,CString csFileFind)
{
	CString csFilePath;
	CStringArray m_Dir;
	CFtpFileFind finder(pConnection);
	BOOL bWorking=finder.FindFile(_T("*"));
	while(bWorking)
	{
		bWorking=finder.FindNextFile();
		if(finder.IsDots())
		{
			continue;
		}
		if(finder.IsDirectory())
		{
			m_Dir.Add(finder.GetFileName());
		}else
		{
			csFilePath=finder.GetFileName();
			if(-1!=csFilePath.Find(csFileFind))
			{
				if(-1!=finder.GetFilePath().Find("ATSLOG"))
				{
				nFileLength=finder.GetLength();
				}
				if(-1!=finder.GetFilePath().Find("UADCLOG"))
				{
					nFileUadLen=finder.GetLength();
				}
				if(nFileLength&&nFileUadLen)
				{
					finder.Close();
					return;
				}
			}
		}
	}
	finder.Close();
	for(int i=0;i<m_Dir.GetSize();i++)
	{
		nRetry++;
		BOOL m_Find=0;
		while(!m_Find)
		{
		m_Find=pConnection->SetCurrentDirectory(m_Dir.GetAt(i));
		}
		if(-1!=m_Dir.GetAt(i).Find(""))
		{
		List(pConnection,csPath,csFileFind);
		}
		BOOL m_Find1=0;
		while(!m_Find1)
		{
			m_Find1=pConnection->SetCurrentDirectory("..");
		}
		nRetry--;
	}
	return;
}
/*
{
	CString csFileName,csFilePath;	
	CFtpFileFind finder(pConnection);
	CStringList list;
	
	pConnection->SetCurrentDirectory(csPath);
	BOOL bWorking=finder.FindFile(_T("*"));
	while(bWorking)
	{
		bWorking=finder.FindNextFile();
		csFileName=finder.GetFileName();
		csFilePath=finder.GetFilePath();
		::AfxMessageBox(csFilePath,MB_OK,0);
		if(-1!=csFileName.Find(csFileFind))
		{			
			csFilePath=finder.GetFilePath();		
			nFileLength=finder.GetLength();
			finder.Close();
			return;
		}
		if(csFileName=="."||csFileName=="..")
		{
			continue;
		}
		if(finder.IsDirectory())
		{
			list.AddTail(csFileName);
		}
	}
	finder.Close();
	while(!list.IsEmpty())
	{
		csPath=list.RemoveHead();
		List(pConnection,csPath,csFileFind);
	}
}*/
//void List(CFtpConnection *pConnection,CString csPath,CString csFileFind)
//{
//	CString m_i;
//　 CString m_ii;//用来保存一些临时信息
//　 CStringArray m_Dir;//用数组来保存目录
//　 CFtpFileFind　 finder(pConnection);
//　 BOOL bWorking=finder.FindFile(_T("*"));
//	CString m_ftpinfo;
//　 while (bWorking)
//　　{
//　　　　bWorking = finder.FindNextFile();
//　　	if ( finder.IsDots() ) continue;
//　　	if (finder.IsDirectory())
//　　　　 {　
//　　		m_Dir.Add( finder.GetFileName());//如果是目录的话,就保存在数组里
//　　　　 }
//　　	else
//　　　　　{
//　　		 m_i=finder.GetFileName()+"rn";//如果不是目录的话,就先显示出来
//　　　　	 for (int j=0;j<n;j++)　　　//这里就是来控制层次的,深的目录前面就多空一些
//　　			{
//　　　				m_ii="t";
//　　　　			m_ftpinfo=m_ftpinfo+m_ii;
//　　			}
//　　　			m_ftpinfo +=m_i;
//　　　　　}
//　　}
//　 finder.Close();　　　　　　　　//连接关闭
//　 for(int i=0;i<m_Dir.GetSize();i++)　　　//开始遍历目录里的文件
//　 {
//　　	n++;　　　　　　　　
//　　	m_i="["+m_Dir.GetAt(i)+"]"+"rn"; //在目录的名字外面加上[]
//　　
//　　	for (int j=1;j<n;j++)
//　　	{
//　　　		m_ii="t";
//　　　　	m_ftpinfo=m_ftpinfo+m_ii;
//　　	}
//　　　 m_ftpinfo +=m_i;　　　　　//先显示目录的名字
//　
//　　	BOOL m_suc=0;
//　　	while (!m_suc)　　 //进入目录里面
//　		{
//　　　　　m_suc=m_pFtpConnection->SetCurrentDirectory(m_Dir.GetAt(i));
//　　	}
//　　　　
//　　	List();　　　　 //递归调用!
//　　	BOOL m_suc1=0;
//　　	while(!m_suc1)
//　　	{
//　　　　m_suc1= m_pFtpConnection->SetCurrentDirectory("..");
//　　	}
//　　n--;　 //控制层次的
//　 }
//}
//void TestFailCal(CString &csErrorCode)
//{
//	////////First Read Reg
//	//////CRegKey cRegKey;
//	//////HKEY hRoot=HKEY_LOCAL_MACHINE;
//	//////CString csSubKey,csValue,csTemp;
//	//////int nPos=0;
//	//////DWORD Size=REG_KEY_LENGTH;
//	//////csSubKey.Format("Software\\Microsoft\\ErrorCode");
//	//////LONG lRetOpen,lRetCrt;
//	//////lRetOpen=cRegKey.Open(hRoot,csSubKey.GetBuffer(),KEY_READ);
//	//////if(lRetOpen!=ERROR_SUCCESS)
//	//////{
//	//////	lRetCrt=cRegKey.Create(HKEY_LOCAL_MACHINE,csSubKey.GetBuffer(),REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,NULL);
//	//////	csTemp.Format("MAX:%s,1%MID:%s,0%MIN:%s,0%%s:1&",csErrorCode,"NONE","NONE",csErrorCode);
//	//////	cRegKey.SetStringValue("ErrorNum",csTemp,REG_MULTI_SZ);
//	//////}
//	//////cRegKey.QueryMultiStringValue("ErrorNum",csValue.GetBuffer(REG_KEY_LENGTH),&Size);
//	////////Test Fail ErrorCode Calculate
//	//////nPos=csValue.Find(csErrorCode);
//	////////Need to Set Max,Mid,Third FAIL code 
//
//	//No Need to Write to Register,write the errorcode to local file
//	HANDLE hFile;
//	CFile ErrFile(hFile),OldFile;
//	CString csErrPath,csTemp,str,strTemp;
//	int ErrNum=0,ErrMax=0,ErrMin=0,ErrMid=0;
//	char CurPath[MAX_PATH],ErrPath[MAX_PATH];
//	::GetCurrentDirectory(MAX_PATH,CurPath);
//	sprintf(ErrPath,"%s\\ErrCode.ini",CurPath);
//	csErrPath.Format("%s",ErrPath);
//	
//	WIN32_FIND_DATA FindData;
//	
//	if(INVALID_HANDLE_VALUE==::FindFirstFile(csErrPath,&FindData))
//	{
//		hFile=CreateFile("ErrCode.ini",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
//		if(hFile==INVALID_HANDLE_VALUE)
//		{
//			::AfxMessageBox("無法創建錯誤代碼記錄文件",MB_OK,0);
//			return;
//		}
//		csTemp.Format("MAX:%s,1\r\nMID:%s,0\r\nMIN:%s,0\r\n*%s:1\r\n",csErrorCode,"NONE","NONE",csErrorCode);
//		ErrFile.Write(csTemp.GetString(),csTemp.GetLength());
//		ErrFile.Close();
//		return;
//	}else
//	{
//		if(NULL==OldFile.Open("ErrCode.ini",CFile::modeRead|CFile::modeWrite))
//		{
//			::AfxMessageBox("無法打開錯誤代碼記錄文件",MB_OK,0);
//			return;
//		}
//		OldFile.Read(csTemp.GetBuffer(),OldFile.GetLength());
//		csTemp.Trim();
//		////First,Need to Check the ErrorCode not or in the three Nums
//		CString csErr[3];
//		int nErrNum[3],nFlag=0,nMax,nMin,nTemp=0;
//		GetCString(csTemp,"MAX",":",",",csErr[0]);
//		GetCString(csTemp,"MID",":",",",csErr[1]);
//		GetCString(csTemp,"MIN",":",",",csErr[2]);
//		GetCString(csTemp,"MAX",",","\r",str);
//		str.Trim();
//		nErrNum[0]=atoi(str);
//		GetCString(csTemp,"MID",",","\r",str);
//		str.Trim();
//		nErrNum[1]=atoi(str);
//		GetCString(csTemp,"MIN",",","\r",str);
//		str.Trim();
//		nErrNum[2]=atoi(str);
//		str.Format("*%s",csErrorCode);
//		for(int i=0;i<3;i++)
//		{
//			if(csErr[i].Compare(str)==0)
//			{
//				nErrNum[i]++;
//				nFlag=1;
//				break;
//			}
//		}
//		if(nFlag)
//		{
//			nMax=nErrNum[0];
//			nMax=(nErrNum[0]>nErrNum[1])?nErrNum[0]:nErrNum[1];
//			nMax=(nMax>nErrNum[2])?nMax:nErrNum[2];
//			if(nMax==nErrNum[1])
//			{
//				nTemp=nErrNum[0];
//				nErrNum[0]=nMax;
//				nErrNum[1]=nTemp;
//				str=csErr[0];
//				csErr[0]=csErr[1];
//				csErr[1]=str;
//			}
//			else if(nMax==nErrNum[2])
//			{
//				nTemp=nErrNum[0];
//				nErrNum[0]=nMax;
//				nErrNum[2]=nTemp;
//				str=csErr[0];
//				csErr[0]=csErr[2];
//				csErr[2]=str;
//			}
//			else
//			{
//				;
//			}
//			if(nErrNum[1]<nErrNum[2])
//			{
//				nMin=nErrNum[1];
//				nErrNum[1]=nErrNum[2];
//				nErrNum[2]=nMin;
//				str=csErr[1];
//				csErr[1]=csErr[2];
//				csErr[2]=str;
//			}
//			strTemp.Format("*%s",csErrorCode);
//			GetCString(csTemp,strTemp,":","\r",str);
//			str.Trim();
//			nTemp=atoi(str);
//			strTemp.Format("*%s:%d\r\n",csErrorCode,nTemp);
//			str.Format("*%s:%d\r\n",csErrorCode,nTemp+1);
//			csTemp.Replace(strTemp,str);
//		}
//		else
//		{
//			str.Format("*%s",csErrorCode);
//			if(-1==csTemp.Find(str))
//			{
//				csTemp.AppendFormat("*%s:%d\r\n",csErrorCode,1);
//			}
//			else
//			{
//				GetCString(csTemp,str,",","\r",strTemp);
//				strTemp.Trim();
//				nMax=atoi(strTemp);
//				nMax++;
//				do
//				{
//				if(nMax>nErrNum[0])
//				{
//					nTemp=nErrNum[0];
//					nErrNum[0]=nMax;
//					str=csErr[0];
//					csErr[0]=csErrorCode;
//					if(nTemp>nErrNum[1])
//					{
//						nErrNum[1]=nTemp;
//						nTemp=nErrNum[1];
//						strTemp=csErr[1];
//						csErr[1]=str;
//						if(nTemp>nErrNum[2])
//						{
//							nErrNum[2]=nTemp;
//							csErr[2]=strTemp;
//						}
//					}
//					break;
//				}
//				if(nMax>nErrNum[1])
//				{
//					nTemp=nErrNum[1];
//					nErrNum[1]=nMax;
//					strTemp=csErr[1];
//					csErr[1]=csErrorCode;
//					if(nTemp>nErrNum[2])
//					{
//						nErrNum[2]=nTemp;
//						csErr[2]=strTemp;
//					}
//					break;
//				}
//				if(nMax>nErrNum[2])
//				{
//					nErrNum[2]=nMax;
//					csErr[2]=csErrorCode;
//				}
//				break;
//				}while(1);
//			}
//		}
//	
//	}
//
//}

void CATSDlg::OnStnClickedSfisMeg()
{
	// TODO:  在此添加控件通知处理程序代码
}


void CATSDlg::OnEnChangeUiMsg()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CATSDlg::OnStnClickedPassCount()
{
	// TODO:  在此添加控件通知处理程序代码
}
