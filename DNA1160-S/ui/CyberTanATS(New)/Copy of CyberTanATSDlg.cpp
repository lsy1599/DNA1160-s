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
/* 新加 */

enum TESTING_STATION { 
	STA_RF = 0x01,
	STA_PER = 0x02,
	STA_TXRX = 0x03,
	STA_FW = 0x04,
	STA_FT = 0x05,
	STA_CURR = 0x06,
	STA_PT = 0x07,
	STA_RC = 0x08
};

//!!這裡是調整所有DLL Handle的個數
HINSTANCE hDLL[15] = {0};

SFIS_PM	SFIS;
SFIS_PM	secondSFIS;//20090428
CATSDlg *closeCATSDlg;//20090428
DEVICE_HANDLE *DeviceHandle = new DEVICE_HANDLE;
int retSFIS = -99;
ATSCONFIG AtsInfo;
LogST RF_Log, PER_Log, TXRX_Log, FW_Log, FT_Log, Temp_Log, CURR_Log, PT_Log, RC_Log;
TESTING_STATION	gTestingStation;
UINT nPassCount, nFailCount, nTotalCount;
CWinThread *pMainThread;
HWND UI_hWnd;
SfisOffDlg SFISOFF;//20090622 tommy

BOOL nRunningFlag = TRUE;

static int		Sfis_status = 0;

UINT ThreadSFISIN(LPVOID pParam);
UINT ThreadTester(LPVOID pParam);
int FuncTest(TEST_FUNC testFunction, int r_flag);
int SFISTestFunc(BOOL bTestResult,int nTestState);
///////////////////////////////////////////////20090429///////////////////////////////////////////////
////////////////////////////////
char receiveBufferThread[4096];
//CString	csGetSfisInfo;//Tommy debug

////////////////////////////////
UINT ThreadTesterGetSFIS(LPVOID pParam);

DWORD WINAPI ThreadTestGetSFIS(LPVOID pParam)
{
	memset(&receiveBufferThread, 0, sizeof(receiveBufferThread));
	Sfis_status = DetSFISmsg(&SFIS,receiveBufferThread);
	//Tommy debug
	//csGetSfisInfo.Format("(%d)%s",strlen(receiveBufferThread),receiveBufferThread);
	//::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csGetSfisInfo, 0);
	return TRUE;
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
					  NULL);DeleteFile("COM4");
    if(!hRS232)
	{
		::AfxMessageBox("Open COM Port Fail!",0,0);
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

	nReTest =::MessageBox(0,"████ 請掃入條碼 ████", "請用滑鼠選擇", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);	
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
	nReTest =MessageBox(0,AtsInfo.TestFunc.SFIS.czModelPN, "請用滑鼠選擇", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
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
		text.Format("請插入待測裝置...");
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&text,0);
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
			   nRunningFlag = FALSE;
					ThreadTester((LPVOID)1);
		while( AtsInfo.m_ipEmbedIP != "" && PingMe(AtsInfo.m_ipEmbedIP) &&nRunningFlag && nCheckInput )
		{
			//status = STATUS_SHOW_DECT;//20090514
			//::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);//20090514					

			if( AtsInfo.m_ipEmbedIP != "" && !PingMe(AtsInfo.m_ipEmbedIP) && !AtsInfo.LoopTest.m_bLoopTest )
			{
				text = "請插入待測裝置...";
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
				text = "待測裝置不良或不存在...";
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
		showMsg.Format("請插入待測裝置...");
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&showMsg,0);

		while( AtsInfo.m_ipEmbedIP != "" && !PingMe(AtsInfo.m_ipEmbedIP) && nRunningFlag )
		{
			WinExec("arp -d", SW_HIDE);	
			Pause(1000);
		}

		if ( !nRunningFlag )
			return 0;

		if( (retSFIS = SFISTestFunc( TRUE, FIRST_RECEIVE_SFIS_DATA )) !=1 )	
		{
			continue;
		}
		
		int status = STATUS_SHOW_TEST;
		::SendMessage(UI_hWnd,MSG_SET_TEST_STATUS,(WPARAM)&status,0);

		if(AtsInfo.m_startOnButton)
		{
			short key = 0 ;
			showMsg.Format("請押空白鍵開始測試...");
			do
			{
 				//GetKeyboardState((LPBYTE)&keyState);
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE,(WPARAM)&showMsg,0);
   				key = GetAsyncKeyState(VK_SPACE);//偵測鍵盤狀態
				Sleep(500);
			}while(key == 0);
		}
		//AfxMessageBox("開始測試",MB_OK);
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

	if(nRet && AtsInfo.TestFunc.RF.m_TestEnable)
	{
		gTestingStation = STA_RF;
		nRet = FuncTest(AtsInfo.TestFunc.RF, 0);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(nRet && AtsInfo.TestFunc.PER.m_TestEnable)
	{
		gTestingStation = STA_PER;
		nRet = FuncTest(AtsInfo.TestFunc.PER, 1);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(nRet && AtsInfo.TestFunc.TXRX.m_TestEnable)
	{
		gTestingStation = STA_TXRX;
		nRet = FuncTest(AtsInfo.TestFunc.TXRX, 2);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(nRet && AtsInfo.TestFunc.FW_UPGRADE.m_TestEnable)
	{
		gTestingStation = STA_FW;
		nRet = FuncTest(AtsInfo.TestFunc.FW_UPGRADE, 3);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(AtsInfo.TestFunc.FINAL_TEST.m_TestEnable)
	{
		gTestingStation = STA_FT;
		nRet = FuncTest(AtsInfo.TestFunc.FINAL_TEST, 4);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(AtsInfo.TestFunc.Current.m_TestEnable)
	{
		gTestingStation = STA_CURR;
		nRet = FuncTest(AtsInfo.TestFunc.Current, 5);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(nRet && AtsInfo.TestFunc.PT.m_TestEnable)
	{
		gTestingStation = STA_PT;
		nRet = FuncTest(AtsInfo.TestFunc.PT, 6);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, nRet ? 3 : 4, 0);
	}

	if(nRet && AtsInfo.TestFunc.RC.m_TestEnable)
	{
		gTestingStation = STA_RC;
		nRet = FuncTest(AtsInfo.TestFunc.RC, 7);
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
		showMsg.Format("████████ 請依序掃入 SN、MAC 條碼 ████████");
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		/////////////////////////////////20090429/////////////////////////////////
		//Sfis_status = DetSFISmsg(&SFIS,receiveBuffer);
		DWORD  threadIdGetSFIS;
		HANDLE hGetSFIS=CreateThread( NULL,
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
		/////////////////////////////////20090429/////////////////////////////////
		//做SFIS structure的去空白處理
		CString str;
		str.Format(SFIS.czSN);
		str.Trim();
		strcpy(SFIS.czSN, str);
		
		str.Format(SFIS.czMAC);
		str.Trim();
		strcpy(SFIS.czMAC, str);

		str.Format(SFIS.czSSN1);
		str.Trim();
		strcpy(SFIS.czSSN1, str);

		str.Format(SFIS.czSSN2);
		str.Trim();
		strcpy(SFIS.czSSN2, str);

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
				showMsg.Format("! # 等待輸入超時 #!");
				::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
			}
			return FALSE;
		}		

		if(!Sfis_status)
		{
			showMsg.Format("!Read ComPort Fail!!");
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
				showMsg.Format("# 請種新掃入 SN、MAC 條碼 #");			
				color1 = COLOR_YELLOW;
				::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);				
				//SFIS_PM	secondSFIS;
				showMsg.Format("██ 請種新掃入 SN、MAC 條碼 ██");
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
					WaitForSingleObject(hGetSFIS_S,INFINITE) ;	
				}
				sprintf(receiveBuffer,"%s",receiveBufferThread);
				/////////////////////////////////20090429/////////////////////////////////
				//做SFIS structure的去空白處理
				CString str;
				str.Format(secondSFIS.czSN);
				str.Trim();
				strcpy(secondSFIS.czSN, str);
				
				str.Format(secondSFIS.czMAC);
				str.Trim();
				strcpy(secondSFIS.czMAC, str);

				str.Format(secondSFIS.czSSN1);
				str.Trim();
				strcpy(secondSFIS.czSSN1, str);

				str.Format(secondSFIS.czSSN2);
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
						showMsg.Format("! # 等待輸入超時 #!");
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
						showMsg.Format("!SN比對不符合!:F_SN:%s,S_SPN:%s",SFIS.czSN,secondSFIS.czSN);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);	
						//Sleep(100);
					}
					if(strcmp(SFIS.czMAC,secondSFIS.czMAC) != 0)
					{
						showMsg.Format("!MAC比對不符合!:F_MAC:%s,S_MAC:%s",SFIS.czMAC,secondSFIS.czMAC);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);	
						Sleep(100);
					}
					if(strcmp(SFIS.czSSN1,secondSFIS.czSSN1) != 0)
					{
						showMsg.Format("!SSN1比對不符合!:F_SSN1:%s,S_SSN1:%s",SFIS.czSSN1,secondSFIS.czSSN1);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);	
						//Sleep(100);
					}
					if(strcmp(SFIS.czSSN2,secondSFIS.czSSN2) != 0)
					{
						showMsg.Format("!SSN2比對不符合!:F_PN:%s,S_PN:%s",SFIS.czSSN2,secondSFIS.czSSN2);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						//Sleep(100);
					}
					showMsg.Format("#測試前后SFIS回傳資料不符合#");			
					color1 = COLOR_RED;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
					Sleep(100);
					memset(&secondSFIS, 0, sizeof(secondSFIS)); //清除這次測試的shopfloor資料
					return -9;
				}
			}
		}
		///////////////////////////////////////////20090428///////////////////////////////////////////////////////
		strcpy(SFIS.czEND, "END");
		showMsg.Format("# 上傳DUT資訊至SFIS系統,等待SFIS回應... #");
		::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		//Sfis_status = SendMsgSFIS(bTestResult,"            ",SFIS.czEC,&SFIS,receiveBuffer);	
		Sfis_status = SendMsgSFIS(bTestResult,SFIS.czFW,SFIS.czEC,&SFIS,receiveBuffer);//20090514
		///////////////////////////////////20090429/////////////////////////////////
		////Sfis_status = SendMsgSFIS(bTestResult,"            ",SFIS.czEC,&SFIS,receiveBuffer);	//
		//DWORD  threadIdSendSFIS;
		//HANDLE hSendSFIS=CreateThread( NULL,
		//						0,
		//						ThreadTestSendSFIS,
		//						this,
		//						0,
		//						&threadIdSendSFIS);				
		//if(!threadIdSendSFIS)
		//{
		//	pMfgDlg->AddListMsg("Call GetSFIS_S Function Fail");
		//	nResult=FALSE;				
		//}
		//else	
		//{					
		//	WaitForSingleObject(hSendSFIS,INFINITE) ;					
		//}
		///////////////////////////////////20090429/////////////////////////////////
		if(Sfis_status == 0 && strstr(receiveBuffer, "retest") != NULL )
		{
			return_status = 5;
			showMsg.Format("重新測試，取消上傳DUT資訊");
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		}
		else if(Sfis_status == 0)
		{
			showMsg.Format("!Send SFIS file error!!");
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
		}
		else
		{
			csInfo.Format("(%d)%s",strlen(receiveBuffer),receiveBuffer);
			::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);

			Sfis_status = DetSFISmsg(&SFIS,receiveBuffer);

			
			csInfo.Format("(%d)%s",strlen(receiveBuffer),receiveBuffer);
			switch(Sfis_status)
			{
				case 0:
					showMsg.Format("!Read SFIS ComPort Fail!!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					color1 = COLOR_RED;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
					return_status = 0;
					break;
				case 1:
					Pause(1000);
					if( bTestResult == FALSE )
					{
						showMsg.Format("# SFIS回應[OK]");
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);
						return_status = 4;
					}
					else
					{
						
						showMsg.Format("# SFIS回應[OK]");					
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);//Tommy debug
						return_status = 1;
					}
					color1 = COLOR_GREEN;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
					break;
				case 2:
					showMsg.Format("! SFIS系統無回應!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);//Tommy debug
					color1 = COLOR_RED;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
					return_status = 2;
					break;
				case 3:
					Pause(1000);
					showMsg.Format("! SFIS回應DUT訊息不合規格!");
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsg, 0);					
					::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&csInfo, 0);
					color1 = COLOR_RED;
					::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsg, (LPARAM)&color1);
					return_status = 3;
					break;
				case 5:
					return_status = 5;
					break;
				default:
					break;
			}
		}
	}
	delete [] receiveBuffer;
	return return_status;
}


int FuncTest(TEST_FUNC testFunction,int r_flag)
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

	InitDut = (lpInitDut)GetProcAddress(hDLL[r_flag],"InitDut");	
	StartTest = (lpStartTest)GetProcAddress(hDLL[r_flag],"StartTest");
	TestEnd = (lpTestEnd)GetProcAddress(hDLL[r_flag],"TestEnd");
	
	testPass = InitDut(&SFIS);

	AtsInfo.dwTimeStart = ::GetTickCount();

	//只有InitDut() Pass後才做StartTest()
	if ( testPass )
		testPass = StartTest();

	//不論是否PASS都要做TestEnd()
	TestEnd();
	AtsInfo.dwTimeTotal = GetTickCount() - AtsInfo.dwTimeStart;
	
	temp.Format("Total test time : %d seconds.", AtsInfo.dwTimeTotal/1000);
	::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&temp, 0);

	//FreeLibrary(hDLL[r_flag]);

	return testPass;
}

UINT HandlePreInit(LPVOID pParam)
{
	/* 8/6/08 Ryan:To add PreInit(); Call PreInit() of all enabled functions. */
	
	bool testPass = 0;
	typedef bool (WINAPI* lpPreInit)(HWND, DEVICE_HANDLE*);
	lpPreInit PreInit;
	CString temp1;	

	if ( AtsInfo.TestFunc.RF.m_TestEnable )
	{
		gTestingStation = STA_RF;
		hDLL[0] = ::LoadLibrary(AtsInfo.TestFunc.RF.m_DllName);		
		PreInit = (lpPreInit)GetProcAddress(hDLL[0],"PreInit");		
		PreInit(UI_hWnd, DeviceHandle);		
 		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.PER.m_TestEnable )
	{
		gTestingStation = STA_PER;
		hDLL[1] = ::LoadLibrary(AtsInfo.TestFunc.PER.m_DllName);
		PreInit = (lpPreInit)GetProcAddress(hDLL[1],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.TXRX.m_TestEnable )
	{
		gTestingStation = STA_TXRX;
		hDLL[2] = ::LoadLibrary(AtsInfo.TestFunc.TXRX.m_DllName);
		PreInit = (lpPreInit)GetProcAddress(hDLL[2],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.FW_UPGRADE.m_TestEnable )
	{
		gTestingStation = STA_FW;
		hDLL[3] = ::LoadLibrary(AtsInfo.TestFunc.FW_UPGRADE.m_DllName);		
		PreInit = (lpPreInit)GetProcAddress(hDLL[3],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.FINAL_TEST.m_TestEnable )
	{
		gTestingStation = STA_FT;
		hDLL[4] = ::LoadLibrary(AtsInfo.TestFunc.FINAL_TEST.m_DllName);		
		PreInit = (lpPreInit)GetProcAddress(hDLL[4],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.Current.m_TestEnable )
	{
		gTestingStation = STA_CURR;
		hDLL[5] = ::LoadLibrary(AtsInfo.TestFunc.Current.m_DllName);		
		PreInit = (lpPreInit)GetProcAddress(hDLL[5],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.PT.m_TestEnable )
	{
		gTestingStation = STA_PT;
		hDLL[2] = ::LoadLibrary(AtsInfo.TestFunc.PT.m_DllName);
		PreInit = (lpPreInit)GetProcAddress(hDLL[6],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	if ( AtsInfo.TestFunc.RC.m_TestEnable )
	{
		gTestingStation = STA_RC;
		hDLL[2] = ::LoadLibrary(AtsInfo.TestFunc.RC.m_DllName);
		PreInit = (lpPreInit)GetProcAddress(hDLL[7],"PreInit");
		PreInit(UI_hWnd, DeviceHandle);
		::SendMessage(UI_hWnd, MSG_SEND_LOG_MESSAGE, (WPARAM)1, (LPARAM)0);
	}

	CString temp = "DLL初始作業完成.";
	::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&temp, 0);

	char errMsg[200];
	//判斷DLL是不是把UI設定為GOLDEN
	if ( !AtsInfo.m_bServe )
	{
		if( AtsInfo.TestFunc.SFIS.m_TestEnable )
		{
			if(!InitSFIS(errMsg))
			{				
				temp.Format(errMsg);
				::SendMessage(UI_hWnd, MSG_SHOW_MESSAGE, (WPARAM)&temp, 0);
				return FALSE;
			}			
		}
		::SendMessage(UI_hWnd, MSG_ENABLE_TEST_BUTTON, 1, 0);
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

	//Ryan:用回圈做DDX
	for ( int i = 0 ; i < 30 ; i++ )
	{
		DDX_Control(pDX, IDC_TESTITEM_1+i, m_testitem[i]);
	}
	DDX_Control(pDX, IDC_UI_MSG, m_list_msg);
	DDX_Control(pDX, IDC_TOTAL_COUNT, m_total_count);
	DDX_Control(pDX, IDC_FAIL_COUNT, m_fail_count);
	DDX_Control(pDX, IDC_PASS_COUNT, m_pass_count);
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
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDTEST, &CATSDlg::OnStartBtn)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
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
	if(FindWindow("#32770", "CyberTanATS MFG Utility"))
	{
		AfxMessageBox("系統中已經有重複的程式正在執行", MB_OK, 0);
		exit(0);
	}
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
			
	char errMsg[256];
	SetTestStatus(STATUS_SHOW_IDLE);
	UI_hWnd = m_hWnd;
	LoadATSini(); //讀INI檔


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
	csTmp.Format("%s",AtsInfo.Log.csLogFilePath);
	::CreateDirectory(csTmp, NULL);
	csTmp.Format("%sLog",AtsInfo.Log.csLogFilePath);
	//::CreateDirectory("Log", NULL);
	::CreateDirectory(csTmp, NULL);
	csTmp.Format("%sLog\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	::CreateDirectory(csTmp, NULL);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	csTmp.Format("%sLog\\%s\\RF", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\RF", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("%sLog\\%s\\PER", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\PER", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("%sLog\\%s\\TXRX", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\TXRX", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("%sLog\\%s\\FT", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\FT", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);
	
	csTmp.Format("%sLog\\%s\\FW", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\FW", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("%sLog\\%s\\CURR", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\CURR", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("%sLog\\%s\\PT", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\PT", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("%sLog\\%s\\RC", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{
		csTmp.Format("Log\\%s\\%s\\RC", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);
		::CreateDirectory(csTmp, NULL);
	}
	////////////////////////////////20090427/////////////////////////////////////////////
	::CreateDirectory(csTmp, NULL);
	csTmp2 = csTmp + "\\PASS";
	::CreateDirectory(csTmp2, NULL);
	csTmp2 = csTmp + "\\FAIL";
	::CreateDirectory(csTmp2, NULL);

	csTmp.Format("DATAOUT_BACKUP");
	::CreateDirectory(csTmp, NULL);

	//清除並初始化log structure
	RF_Log.ResetAll();
	PER_Log.ResetAll();
	TXRX_Log.ResetAll();
	FW_Log.ResetAll();
	FT_Log.ResetAll();
	CURR_Log.ResetAll();
	PT_Log.ResetAll();
	RC_Log.ResetAll();

	GetDlgItem(IDC_DLL_INFO)->SetWindowTextA(m_dllInfo);

	if(!LoadTestItem())
	{
		ShowMessage("Please close application and check library is exist or version is correct.");
		SetTestStatus(STATUS_SHOW_FAIL);
		return 0;
	}

	nInitialLoadIsOk = TRUE;

	m_testbutton.EnableWindow(FALSE);
	ShowMessage("正在處理DLL初始程序.");
	//處理所有DLL的PreInit程序，完成後才會把Start button enable
	CWinThread *mythread = ::AfxBeginThread(HandlePreInit, (LPVOID)0);
/*	
	//SJ要求, SFIS disable測試, 需要輸入password 
	if ( !AtsInfo.m_bServe && AtsInfo.TestFunc.SFIS.m_TestEnable != 1 )
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

	}
*/
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CATSDlg::PreTranslateMessage(MSG* pMsg)
{	
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

	::GetCurrentDirectory(256,strBuf);
	sprintf_s(iniFileName,"%s\\ATSCfg.ini",strBuf);
	/////////////////////////////////////// 20090421/////////////////////////////////////
	AtsInfo.TestFunc.SFIS.czModelPN="";//20090421
	AtsInfo.TestFunc.SFIS.nCALL_PN_Enable = GetPrivateProfileInt("SFIS","CALL_PN_Enable",0,iniFileName);//20090421	
	GetPrivateProfileString("SFIS","DllLoadIniName","MFGATS_FTFC.ini",strBuf,sizeof(strBuf),iniFileName);
	AtsInfo.TestFunc.SFIS.csDllLoadIniName = strBuf;	
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

	AtsInfo.TestFunc.SFIS.m_TestEnable = GetPrivateProfileInt("SFIS","SFISEab",0,iniFileName);
	AtsInfo.TestFunc.SFIS_Retry.m_TestEnable =GetPrivateProfileInt("SFIS","SFIS_Retry_Eab",0,iniFileName);
	GetPrivateProfileString("SFIS","DllName","",AtsInfo.TestFunc.SFIS.m_DllName,sizeof(AtsInfo.TestFunc.SFIS.m_DllName),iniFileName);
	GetPrivateProfileString("SFIS","DllVersion","",AtsInfo.TestFunc.SFIS.m_DllVersion,sizeof(AtsInfo.TestFunc.SFIS.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.RF.m_TestEnable= GetPrivateProfileInt("RF","TestEab",0,iniFileName);
	GetPrivateProfileString("RF","DllName","",AtsInfo.TestFunc.RF.m_DllName,sizeof(AtsInfo.TestFunc.RF.m_DllName),iniFileName);
	GetPrivateProfileString("RF","DllVersion","",AtsInfo.TestFunc.RF.m_DllVersion,sizeof(AtsInfo.TestFunc.RF.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.PER.m_TestEnable= GetPrivateProfileInt("PER","TestEab",0,iniFileName);
	GetPrivateProfileString("PER","DllName","",AtsInfo.TestFunc.PER.m_DllName,sizeof(AtsInfo.TestFunc.PER.m_DllName),iniFileName);
	GetPrivateProfileString("PER","DllVersion","",AtsInfo.TestFunc.PER.m_DllVersion,sizeof(AtsInfo.TestFunc.PER.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.TXRX.m_TestEnable = GetPrivateProfileInt("TxRx","TestEab",0,iniFileName);
	GetPrivateProfileString("TxRx","DllName","",AtsInfo.TestFunc.TXRX.m_DllName,sizeof(AtsInfo.TestFunc.TXRX.m_DllName),iniFileName);
	GetPrivateProfileString("TxRx","DllVersion","",AtsInfo.TestFunc.TXRX.m_DllVersion,sizeof(AtsInfo.TestFunc.TXRX.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.FINAL_TEST.m_TestEnable = GetPrivateProfileInt("FINAL&CHECK","TestEab",0,iniFileName);
	GetPrivateProfileString("FINAL&CHECK","DllName","",AtsInfo.TestFunc.FINAL_TEST.m_DllName,sizeof(AtsInfo.TestFunc.FINAL_TEST.m_DllName),iniFileName);
	GetPrivateProfileString("FINAL&CHECK","DllVersion","",AtsInfo.TestFunc.FINAL_TEST.m_DllVersion,sizeof(AtsInfo.TestFunc.FINAL_TEST.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.FW_UPGRADE.m_TestEnable = GetPrivateProfileInt("FW_UPGRADE","TestEab",0,iniFileName);
	GetPrivateProfileString("FW_UPGRADE","DllName","",AtsInfo.TestFunc.FW_UPGRADE.m_DllName,sizeof(AtsInfo.TestFunc.FW_UPGRADE.m_DllName),iniFileName);
	GetPrivateProfileString("FW_UPGRADE","DllVersion","",AtsInfo.TestFunc.FW_UPGRADE.m_DllVersion,sizeof(AtsInfo.TestFunc.FW_UPGRADE.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.Current.m_TestEnable = GetPrivateProfileInt("CURRENT","TestEab",0,iniFileName);
	GetPrivateProfileString("CURRENT","DllName","",AtsInfo.TestFunc.Current.m_DllName,sizeof(AtsInfo.TestFunc.Current.m_DllName),iniFileName);
	GetPrivateProfileString("CURRENT","DllVersion","",AtsInfo.TestFunc.Current.m_DllVersion,sizeof(AtsInfo.TestFunc.Current.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.PT.m_TestEnable = GetPrivateProfileInt("PT","TestEab",0,iniFileName);
	GetPrivateProfileString("PT","DllName","",AtsInfo.TestFunc.PT.m_DllName,sizeof(AtsInfo.TestFunc.PT.m_DllName),iniFileName);
	GetPrivateProfileString("PT","DllVersion","",AtsInfo.TestFunc.PT.m_DllVersion,sizeof(AtsInfo.TestFunc.PT.m_DllVersion),iniFileName);

	AtsInfo.TestFunc.RC.m_TestEnable = GetPrivateProfileInt("RC","TestEab",0,iniFileName);
	GetPrivateProfileString("RC","DllName","",AtsInfo.TestFunc.RC.m_DllName,sizeof(AtsInfo.TestFunc.RC.m_DllName),iniFileName);
	GetPrivateProfileString("RC","DllVersion","",AtsInfo.TestFunc.RC.m_DllVersion,sizeof(AtsInfo.TestFunc.RC.m_DllVersion),iniFileName);

	//InputKey
	AtsInfo.m_bInputMac = GetPrivateProfileInt("InputKey", "InputMac", 0, iniFileName);
	AtsInfo.m_bAutoMac = GetPrivateProfileInt("InputKey", "AutoMac", 0, iniFileName);
	AtsInfo.m_bInputSN = GetPrivateProfileInt("InputKey", "InputSN", 0, iniFileName);
	AtsInfo.m_bInputSSN1 = GetPrivateProfileInt("InputKey", "InputSSN1", 0, iniFileName);
	AtsInfo.m_bInputSSN2 = GetPrivateProfileInt("InputKey", "InputSSN2", 0, iniFileName);
	AtsInfo.m_bCheckMac = GetPrivateProfileInt("InputKey", "CheckMac", 0, iniFileName);
	AtsInfo.m_bCheckSN = GetPrivateProfileInt("InputKey", "CheckSN", 0, iniFileName);
	AtsInfo.m_nSN_Length = GetPrivateProfileInt("InputKey", "SN_Length", 0, iniFileName);
	GetPrivateProfileString("InputKey", "MacStart", "000000000000", strBuf, 12, iniFileName);
	AtsInfo.m_csMacStart = strBuf;
	GetPrivateProfileString("InputKey", "MacEnd", "FFFFFFFFFFFF", strBuf, 12, iniFileName);
	AtsInfo.m_csMacEnd = strBuf;

	AtsInfo.m_bAutoTest = GetPrivateProfileInt("TECONFIG","AutoTest",0,iniFileName);
	AtsInfo.m_FTP = GetPrivateProfileInt("TECONFIG","FTPEab",0,iniFileName);
	AtsInfo.LoopTest.m_bLoopTest = GetPrivateProfileInt("TECONFIG","LoopCountEab",0,iniFileName);
	AtsInfo.LoopTest.m_iCountLimite = GetPrivateProfileInt("TECONFIG","LoopCounts",0,iniFileName);

	///////////////////////////////////////////////////////////////////////
	//m_bLogByResult指的是Log是否依照PASS or FAIL分開擺放
	//m_bLogAll就是一般的合在一起的Log
	AtsInfo.Log.m_bLogByResult = GetPrivateProfileInt("TECONFIG","LogByResult",0,iniFileName);
	AtsInfo.Log.m_bLogAll = GetPrivateProfileInt("TECONFIG","LogAll",1,iniFileName);

	AtsInfo.LoopTest.m_bLoopTest = GetPrivateProfileInt("TECONFIG","LoopCountEab",0,iniFileName);
	AtsInfo.LoopTest.m_iCountLimite = GetPrivateProfileInt("TECONFIG","LoopCounts",0,iniFileName);

	//For SJ log file 路徑 20090625
	GetPrivateProfileString("TECONFIG","LogFilePath","", strBuf, 255, iniFileName);
	AtsInfo.Log.csLogFilePath.Format(strBuf);
	
	//For SJ 
	AtsInfo.m_startOnButton = GetPrivateProfileInt("TECONFIG","StartTestOnButton",0,iniFileName);
	AtsInfo.m_ShowDect = GetPrivateProfileInt("TECONFIG","ShowDect",0,iniFileName);
	AtsInfo.m_UiLog = GetPrivateProfileInt("TECONFIG","UiLog",0,iniFileName);
	
	//DB_Setting
	AtsInfo.DB_Setting.m_bDB_Enable = GetPrivateProfileInt("DB_Setting","DB_Enable",0,iniFileName);
	GetPrivateProfileString("DB_Setting","Group_Name","TEST_GROUP", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csGroupName.Format(strBuf);
	GetPrivateProfileString("DB_Setting","Line_Number","TEST_Line", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csLineNumber.Format(strBuf);
	GetPrivateProfileString("DB_Setting","PC_IP","127.0.0.1", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csPC_IP.Format(strBuf);
	GetPrivateProfileString("DB_Setting","Station_Name","TEST_STATION", strBuf, 255, iniFileName);
	AtsInfo.DB_Setting.m_csStationName.Format(strBuf);

	
	
//DB_Enable 	 = 0
//Line_Number  = L6
//Group_Name   = RF_TEST
//PC_IP 		 = 192.168.4.100
//Station_Name = RF_TEST_1

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

	SetWindowText("CyberTanATS MFG Utility");	// for 1.0.0.23

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
				retPress = MessageBox("程式檢查到不同版本的程式文件，確認是否更新？","發現更新",MB_YESNO| MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
				if(retPress == IDYES)
				{
					if(CopyFile(Loadfile,Newfile,FALSE))
						ShowMessage("下載文件成功.");
					else
					{
						MessageBox("下載文件失敗.!","ERROR",MB_OK | MB_SETFOREGROUND);
						exit(1);
					}
				}
			}
		}

		if(!LoadTestItem())
		{
			ShowMessage("Please close application and check library is exist or version is correct.");
			ShowMessage("文件已更新至最新版本,請變更InI檔設置...!");
			SetTestStatus(STATUS_SHOW_FAIL);
		}	
	}
	
	sprintf_s(FileName,"%s\\%s.ini",strBuf,testFunction.m_DllName);
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
	if(AtsInfo.TestFunc.RF.m_TestEnable)
	{
		if(	!StoreTestItem(AtsInfo.TestFunc.RF))
			return false;
		csTemp.Format("[RF]-%s-v%s   ", AtsInfo.TestFunc.RF.m_DllName, AtsInfo.TestFunc.RF.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.PER.m_TestEnable)
	{
		if(	!StoreTestItem(AtsInfo.TestFunc.PER))
			return false;
		csTemp.Format("[PER]-%s-v%s   ", AtsInfo.TestFunc.PER.m_DllName, AtsInfo.TestFunc.PER.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.TXRX.m_TestEnable)
	{
		if(!StoreTestItem(AtsInfo.TestFunc.TXRX))
			return false;
		csTemp.Format("[TXRX]-%s-v%s   ", AtsInfo.TestFunc.TXRX.m_DllName, AtsInfo.TestFunc.TXRX.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.FINAL_TEST.m_TestEnable)
	{
		if(!StoreTestItem(AtsInfo.TestFunc.FINAL_TEST))
			return false;
		csTemp.Format("[FT]-%s-v%s   ", AtsInfo.TestFunc.FINAL_TEST.m_DllName, AtsInfo.TestFunc.FINAL_TEST.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.FW_UPGRADE.m_TestEnable)
	{
		if(!StoreTestItem(AtsInfo.TestFunc.FW_UPGRADE))
			return false;
		csTemp.Format("[FW]-%s-v%s   ", AtsInfo.TestFunc.FW_UPGRADE.m_DllName, AtsInfo.TestFunc.FW_UPGRADE.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.Current.m_TestEnable)
	{
		if(!StoreTestItem(AtsInfo.TestFunc.Current))
			return false;
		csTemp.Format("[CURR]-%s-v%s   ", AtsInfo.TestFunc.Current.m_DllName, AtsInfo.TestFunc.Current.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.PT.m_TestEnable)
	{
		if(!StoreTestItem(AtsInfo.TestFunc.PT))
			return false;
		csTemp.Format("[PT]-%s-v%s   ", AtsInfo.TestFunc.PT.m_DllName, AtsInfo.TestFunc.PT.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}
	if(AtsInfo.TestFunc.RC.m_TestEnable)
	{
		if(!StoreTestItem(AtsInfo.TestFunc.RC))
			return false;
		csTemp.Format("[RC]-%s-v%s   ", AtsInfo.TestFunc.RC.m_DllName, AtsInfo.TestFunc.RC.m_DllVersion);
		m_dllInfo.Append(csTemp);
	}


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
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

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

	p->GetWindowText(History);
	len = History.GetLength();
	if(len>=30000)
	{
		History = History.Right(5000);
		p->SetWindowText(History);
		len = History.GetLength();
	}

	p->SetSel(len, len);
	p->ReplaceSel(text+"\r\n", false);
}

void CATSDlg::SetTestItemStatus(int nID, int nStatus)
{
	switch(nStatus)
	{
	case STATUS_ITEM_PASS:
		m_testitem[nID].SetBkColor(RGB(100,255,50), GetSysColor(COLOR_3DFACE), CLabel::BackFillMode::Gradient);
		m_testitem[nID].SetTextColor(RGB(0,0,200));		
		KillTimer(FLASH_SIGNAL);
		break;
	case STATUS_ITEM_FAIL:
		m_testitem[nID].SetBkColor(RGB(230,30,30));
		m_testitem[nID].SetTextColor(RGB(0,0,0));		
		KillTimer(FLASH_SIGNAL);
		break;
	case STATUS_ITEM_TESTING:
		m_testitem[nID].SetBkColor(RGB(255,255,0), RGB(200,245,100), CLabel::BackFillMode::Gradient);
		m_testitem[nID].SetTextColor(RGB(95,125,5));
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
}

//開始測試函式
void CATSDlg::OnStartBtn()
{
	if(AtsInfo.m_UiLog)
	{
		CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
		p->SetWindowText("");//清除Dialog 所有內容
	}

	m_testbutton.EnableWindow(FALSE);

	nRunningFlag = TRUE;

	//走Shopfloor的流程
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		CWinThread *pThread = AfxBeginThread(ThreadSFISIN, (LPVOID)1);
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
LRESULT CATSDlg::OnLogMessage(WPARAM type, LPARAM msg)
{
	CString csFileName = "", csLogDate = "", csMsg = "", csTmp;
	CString csStation, csItem, csCh, csRate, csAnt;
	SYSTEMTIME  sTime;
	LogST *logST = new LogST;
	LogST *tmplog = new LogST;
	int	nPos = 0, nPos2;

	//0跟2的時後把傳過來的字串存起來預備做處理
	if ( type == 0 || type == 2 )
		csMsg = *((CString*)msg);


	csTmp.Format("[Log]接收到訊息:%s", csMsg);
	DebugMessage(csTmp);

	GetLocalTime(&sTime);
	csLogDate.Format("%04d%02d%02d", sTime.wYear, sTime.wMonth, sTime.wDay);

	//做出Model Name路徑
	csTmp.Format("%sLog\\%s", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	/////////////////////////////////20090427///////////////////////////////////////////
	if(AtsInfo.TestFunc.SFIS.nCALL_PN_Enable)
	{		
		csTmp.Format("Log\\%s\\%s", AtsInfo.m_csModelName,AtsInfo.TestFunc.SFIS.czModelPN);		
	}
	////////////////////////////////20090427/////////////////////////////////////////////

	//依照不同的站別，先建出不同的log file檔案
	switch ( gTestingStation )
	{
	case STA_RF :
		logST = &RF_Log;
		logST->filename.Format("%s\\RF\\RF_%s.xls", csTmp, csLogDate);
		break;
	case STA_PER :
		logST = &PER_Log;
		logST->filename.Format("%s\\PER\\PER_%s.xls", csTmp, csLogDate);
		break;
	case STA_TXRX :
		logST = &TXRX_Log;
		logST->filename.Format("%s\\TXRX\\TXRX_%s.xls", csTmp, csLogDate);
		break;
	case STA_FW :
		logST = &FW_Log;
		logST->filename.Format("%s\\FW\\FW_%s.xls", csTmp, csLogDate);
		break;
	case STA_FT :
		logST = &FT_Log;
		logST->filename.Format("%s\\FT\\FT_%s.xls", csTmp, csLogDate);
		break;
	case STA_CURR :
		logST = &CURR_Log;
		logST->filename.Format("%s\\CURR\\CURR_%s.xls", csTmp, csLogDate);
		break;
	case STA_PT :
		logST = &PT_Log;
		logST->filename.Format("%s\\PT\\PT_%s.xls", csTmp, csLogDate);
		break;
	case STA_RC :
		logST = &RC_Log;
		logST->filename.Format("%s\\RC\\RC_%s.xls", csTmp, csLogDate);
		break;
	}

	//Type 0:設定Log title
	//範例： TestItem001:CH=6&RATE=54&MCS=15&CHAIN=1&ANT=1&Field1=0&Field2=0& 
	//會做出 TestItem001_CH6_RATE54_MSC15_CHAIN1_ANT1
	//實際上使用RATE和MCS不要同用，CHAIN和ANT不要同用
	
	if ( type == 0 ) 
	{
		DebugMessage("[Log]訊息分類：設定log title");
	
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
				ShowMessage("open log file error! Try again.");
				//kill excel task
				WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
				Pause(1000);
				if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
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
				ShowMessage("open log file error! Try again.");
				//kill excel task
				WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
				Pause(1000);
				if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					exit(0);
					return FALSE;
				}
			}
		}
		
		//先加入DATE_TIME, SN, MAC, RESULT
		tmp.Format("DATE_TIME\tDUT_SN\tDUT_MAC\tSpecialKey1\tSpecialKey2\tERROR_CODE\tRESULT\t");
		flogfile.WriteString(tmp);

		tmp.Format("寫入檔案 %s, 寫入欄位共 %d 欄", logST->filename, logST->nCount);
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
					ShowMessage("open log file error! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
					{
						ShowMessage("open log file error!");
						AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
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
					ShowMessage("Open log file error! Try again.");
					//kill excel task
					WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
					Pause(1000);
					if ( !flogfile.Open(logST->filename, CFile::modeReadWrite) )
					{
						ShowMessage("Open log file error!");
						AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
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
					ShowMessage("Open log file error! Try again.");
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
				if ( !fFailfile.Open(FailFileName, CFile::modeReadWrite ) )
				{
					ShowMessage("Open log file error! Try again.");
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
			}
			else //這是檔案不存在的時後
			{
				//這一段是避免EXCEL開啟Log檔案造成開啟失敗
				if ( !fPassfile.Open(PassFileName, CFile::modeCreate | CFile::modeReadWrite ) )
				{
					ShowMessage("Open log file error! Try again.");
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
					ShowMessage("Open log file error! Try again.");
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
	if ( !AtsInfo.m_bServe )
		SetTestStatus(STATUS_SHOW_TEST);
	
	CWinThread *pThread = AfxBeginThread(ThreadTester, (LPVOID)1);
	return true;
}



//測試結束時做的事
LRESULT CATSDlg::OnTestEnd(WPARAM Func, LPARAM Result)
{	
	SFIS_PM *SFIS_Recv;
	int nSfis_result;
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

	//Golden's flow
	if ( AtsInfo.m_bServe )
	{		
		::SendMessage(UI_hWnd, MSG_SHOW_GOLDEN, 0, 0);		
		return 0;
	}

	//清除所有Log資料
	RF_Log.ResetValue();
	PER_Log.ResetValue();
	TXRX_Log.ResetValue();
	FW_Log.ResetValue();
	FT_Log.ResetValue();
	CURR_Log.ResetValue();
	PT_Log.ResetValue();
	RC_Log.ResetValue();
	//////////////////////////////////////////////////////////////////////////////

	if( Result == TEST_PASS && AtsInfo.m_bAutoMac )// Add Auto mac counter, Brand 2008/10/7
	{
		CAutoInputKey AutoKey;
		AutoKey.GetMac();
		AutoKey.MacCounter();
	}

	//Dut's flow [Shopfloor enable]
	if(AtsInfo.TestFunc.SFIS.m_TestEnable)
	{
		int i;
		CString		tempstr;
		CString		csTmp;
		int			status;
		//for( i = 0 ; i < 2 ; i++ )
		//{
		//	//ShowMessage(csTmp);			
		//	if ( Result == TEST_PASS ) //upload directly while pass condition
		//	{
		//		csTmp.Format(">> 第 %d次 上傳DUT TEST PASS訊息至SFIS ...",(i+1));
		//		ShowMessage(csTmp);					
		//		nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );
		//		if(nSfis_result ==1)
		//			break;							
		//	}
		//	else if ( Result == TEST_FAIL )
		//	{
		//		SFISTestFunc(FALSE,SEND_FAIL_INFO_TO_SFIS );
		//		break;				
		//	}			
		//}
					
			if ( Result == TEST_PASS ) //upload directly while pass condition
			{
				csTmp.Format(">> 第 %d次 上傳DUT TEST PASS訊息至SFIS ...",1);
				ShowMessage(csTmp);					
				nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );
				if(nSfis_result !=1)
				{
					if(nSfis_result==-9)
					{
						CString showMsgTemp;
						showMsgTemp.Format("!#測試前后SFIS回傳資料不符合#!");
						::SendMessage(UI_hWnd,MSG_SHOW_MESSAGE, (WPARAM)&showMsgTemp, 0);	
						COLORREF color19;
						color19 = COLOR_RED;
						::SendMessage(UI_hWnd,MSG_SHOW_ACT_MESSAGE, (WPARAM)&showMsgTemp, (LPARAM)&color19);
					}
					else
					{
						Sleep(100);
						csTmp.Format(">> 第 %d次 上傳DUT TEST PASS訊息至SFIS ...",2);
						ShowMessage(csTmp);					
						nSfis_result = SFISTestFunc( TRUE,SEND_PASS_INFO_TO_SFIS );	
					}

					if(nSfis_result == 2)//T0mmy 20091124
					{
						//Tommy 20091118 SFIS error 時, 顯示SFIS errorcode
						strcpy(SFIS.czEC, "SF00");
						status = STATUS_SHOW_ERRCODE;
						::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);

					}
					else if(nSfis_result == 3)
					{
						//Tommy 20091118 SFIS error 時, 顯示SFIS errorcode
						strcpy(SFIS.czEC, "SF10");
						status = STATUS_SHOW_ERRCODE;
						::SendMessage(UI_hWnd, MSG_SET_TEST_STATUS, (WPARAM)&status, 0);
					}
				}
			}
			else if ( Result == TEST_FAIL )
			{
				SFISTestFunc(FALSE,SEND_FAIL_INFO_TO_SFIS );								
			}			
		}	
	

	ShowMessage("測試已完成，請更換待測裝置!!");
	if(AtsInfo.m_UiLog)
	{
		GetUiMsg();
	}
	memset(&SFIS, 0, sizeof(SFIS)); //清除這次測試的shopfloor資料
			
	//有Loop的情況
	if( AtsInfo.LoopTest.m_bLoopTest )
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
						ShowMessage("測試已完成，請更換待測裝置!!");
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
					ShowMessage("測試已完成，請更換待測裝置!!");
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

		if ( AtsInfo.TestFunc.RF.m_TestEnable )
		{			
			if ( hDLL[0] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[0],"Terminal");
				Terminal();
				FreeLibrary(hDLL[0]);
			}
		}
		
		if ( AtsInfo.TestFunc.PER.m_TestEnable )
		{
			if ( hDLL[1] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[1],"Terminal");
				Terminal();
				FreeLibrary(hDLL[1]);
			}
		}

		if ( AtsInfo.TestFunc.TXRX.m_TestEnable )
		{			
			if ( hDLL[2] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[2],"Terminal");
				Terminal();
				FreeLibrary(hDLL[2]);
			}
		}

		if ( AtsInfo.TestFunc.FW_UPGRADE.m_TestEnable )
		{
			if ( hDLL[3] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[3],"Terminal");
				Terminal();
				FreeLibrary(hDLL[3]);
			}
		}

		if ( AtsInfo.TestFunc.FINAL_TEST.m_TestEnable )
		{
			if ( hDLL[4] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[4],"Terminal");
				Terminal();
				FreeLibrary(hDLL[4]);
			}
		}


		if ( AtsInfo.TestFunc.Current.m_TestEnable )
		{
			if ( hDLL[5] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[5],"Terminal");
				Terminal();
				FreeLibrary(hDLL[5]);
			}
		}

		if ( AtsInfo.TestFunc.PT.m_TestEnable )
		{			
			if ( hDLL[6] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[6],"Terminal");
				Terminal();
				FreeLibrary(hDLL[6]);
			}
		}

		if ( AtsInfo.TestFunc.RC.m_TestEnable )
		{			
			if ( hDLL[7] != NULL )
			{
				Terminal = (lpTerminal)GetProcAddress(hDLL[7],"Terminal");
				Terminal();
				FreeLibrary(hDLL[7]);
			}
		}

	}
	WinExec("taskill -f -IM cybertan.exe;", SW_HIDE);

}

void CATSDlg::OnClose()
{	
	nRunningFlag = FALSE;	
	CDialog::OnClose();
}

void CATSDlg::GetUiMsg()//把UI上的內容儲存為LOG
{
	CEdit *p = (CEdit *)GetDlgItem(IDC_UI_MSG);
	CString GetResult;

	CStdioFile fuifile;
	CFileStatus fstatus;
	CString csTmp;
	int nLen;

	nLen = strlen(SFIS.czMAC);

	csTmp.Format("%sLog\\%s\\UI_Log", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName);
	::CreateDirectory(csTmp, NULL);
	if(nLen == 0)
	{
		csTmp.Format("%sLog\\%s\\UI_Log\\%s.txt", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName, "log");
	}
	else
	{
		csTmp.Format("%sLog\\%s\\UI_Log\\%s.txt", AtsInfo.Log.csLogFilePath, AtsInfo.m_csModelName, SFIS.czMAC);
	}
	
	if ( fuifile.GetStatus(csTmp, fstatus) )
		{
			if ( !fuifile.Open(csTmp, CFile::modeReadWrite) )
			{
				ShowMessage("open log file error! Try again.");
				if ( !fuifile.Open(csTmp, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					exit(0);
					//return FALSE;
				}
			}
			else
				fuifile.SeekToEnd();

		}
		else
		{
			if ( !fuifile.Open(csTmp, CFile::modeCreate | CFile::modeReadWrite ) )
			{
				ShowMessage("open log file error! Try again.");
				//kill excel task
				WinExec("taskkill /F /IM EXCEL.EXE /T", SW_HIDE);
				Pause(1000);
				if ( !fuifile.Open(csTmp, CFile::modeReadWrite) )
				{
					ShowMessage("open log file error!");
					AfxMessageBox("Log File 路徑錯誤, 請重新設定", MB_OK, 0);
					exit(0);
					//return FALSE;
				}
			}
		}
	
	p->GetWindowText(GetResult);
	fuifile.WriteString(GetResult);
	fuifile.Close();

}