#include "stdafx.h"
//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
#include "ArtComPort.h"
#include "Comm.h"

OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
HANDLE m_hIDComDev;

bool Create_Com(CString comPort,unsigned baudRate)
{
	DCB dcb;

	memset(&m_OverlappedRead, 0, sizeof (OVERLAPPED));
	memset(&m_OverlappedWrite, 0, sizeof (OVERLAPPED));
	m_hIDComDev = NULL;
	m_hIDComDev = CreateFile(comPort,
							 GENERIC_READ|GENERIC_WRITE,
							 0,
							 NULL,
							 OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
							 NULL);
	if (m_hIDComDev == NULL)
	{
		return false;
	}

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout=20;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 20;
	CommTimeOuts.ReadTotalTimeoutConstant = 20;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 20;
	CommTimeOuts.WriteTotalTimeoutConstant = 20;
	SetCommTimeouts(m_hIDComDev, &CommTimeOuts);

	m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_OverlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	dcb.DCBlength = sizeof(DCB);

	//Set Comm state
	GetCommState(m_hIDComDev, &dcb);
	dcb.BaudRate = baudRate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	
	SetCommTimeouts(m_hIDComDev,&CommTimeOuts);

	if (!SetCommState(m_hIDComDev, &dcb) ||
		!SetupComm(m_hIDComDev,1024,1024) ||
		(m_OverlappedRead.hEvent ==NULL) ||
		(m_OverlappedWrite.hEvent ==NULL)
		)
	{

		DWORD dwError = GetLastError();

		if (m_OverlappedRead.hEvent != NULL) 
			CloseHandle(m_OverlappedRead.hEvent);

		if (m_OverlappedWrite.hEvent != NULL) 
			CloseHandle(m_OverlappedWrite.hEvent);

		CloseHandle(m_hIDComDev);
		
		return false;
	}
	return true;

}

void OnSend(CString sendBuf)
{
	DWORD dwBytesWritte=0,dwBytesWritten=0;

	dwBytesWritte=0;
	dwBytesWritten=0;
	dwBytesWritte=sendBuf.GetLength();
	WriteFile(m_hIDComDev, sendBuf, dwBytesWritte, &dwBytesWritten, &m_OverlappedWrite);

	//#MAC=123123123123&SN=123123123123&
}

void OnReceive(char *GetMsg)
{
	char szMessage[1024];
	CString RecBuf;
	DWORD dwBytesRead=0,dwBytesReaden=0;

	dwBytesRead=sizeof(szMessage);
	while(1)
	{
		memset(szMessage,0,sizeof(szMessage));
		RecBuf.Empty();
		Sleep(666);
		ReadFile(m_hIDComDev, szMessage, dwBytesRead, &dwBytesReaden, &m_OverlappedRead);
	
		sprintf(GetMsg,"%s",szMessage);
		RecBuf = szMessage;
		GetResult(RecBuf);
	}
	//return (CString) szMessage;
}

void OnClose(void)
{
	if (m_OverlappedRead.hEvent != NULL) 
			CloseHandle(m_OverlappedRead.hEvent);

	if (m_OverlappedWrite.hEvent != NULL) 
			CloseHandle(m_OverlappedWrite.hEvent);
	CloseHandle(m_hIDComDev);
}

void GetResult(CString Strbuf)
{
	CString ErrCode;

	if(Strbuf.Find(_T("#SendMessage")) != -1)
	{
		//ShowMessage(Strbuf);
	}
	if(Strbuf.Find(_T("#TestFAIL")) != -1)
	{
		//ShowMessage(Strbuf);
		GetCString(Strbuf, "ErrorCode:", ":", ",", ErrCode);
	}
	if(Strbuf.Find(_T("#TestPASS")) != -1)
	{
		//ShowMessage(Strbuf);
	}
	

}