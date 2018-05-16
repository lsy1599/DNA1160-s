#include "stdafx.h"
#include "ComCtrl.h"
#include "Ping.h"
#include "AtsComm\AtsComm.h"

int nShow=1;
CString HuComName[20];
CString* FindComm()//检索注册表，寻找PC上存在的串口
{
	HKEY hKey = NULL;
	DWORD dwIndex = 0;
	DWORD dwName = 30;
	DWORD Type;
	DWORD ComNameSize = 10;
	UCHAR ComName[10] = { 0 };
	CHAR name[30] = { 0 };
	int status = 0, i = 0;
	//CWindow CWndGet;
	RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", &hKey);
	while (1)
	{
		status = RegEnumValue(hKey, dwIndex++, name, &dwName, 0, &Type, ComName, &ComNameSize);
		if (status == ERROR_NO_MORE_ITEMS) break;
		HuComName[i].Format("%c%c%c%c%c", ComName[0], ComName[1], ComName[2], ComName[3], ComName[4]);
		i++;//AfxMessageBox(csTemp[i]);//-------临时显示用------
		if (dwIndex >= 19) break;//保险，防止陷入死循环
	}
	RegCloseKey(hKey);
	return HuComName;//AfxMessageBox("注册表操作已执行");
}

HANDLE InitCom(int nCom,  DWORD nBaudRate)
{
	CString csCom;
	HANDLE hCom;
	DCB dCom;
	COMMTIMEOUTS cmto;
	CString csErrMsg;
	if(nCom>0)
	{
		if (nCom > 9) 
			csCom.Format("\\\\.\\COM%d",nCom);
		else 
			csCom.Format("COM%d", nCom);
	}
	else
	{
		return NULL;
	}
	hCom = CreateFile(csCom.GetString(), GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if(hCom==INVALID_HANDLE_VALUE)
	{
		hCom=NULL;
		return NULL;
	}
	if(!GetCommState(hCom, &dCom))
	{
		//csErrMsg.Format("GetCommState failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return NULL;
	}
	dCom.BaudRate=nBaudRate;
	dCom.ByteSize=8;
	dCom.Parity=NOPARITY;
	dCom.StopBits=ONESTOPBIT;
	dCom.fBinary=TRUE;
	dCom.fParity=FALSE;
	dCom.fDtrControl=DTR_CONTROL_ENABLE;
	dCom.fRtsControl=RTS_CONTROL_ENABLE;

	if(!SetCommState(hCom, &dCom))
	{
		//csErrMsg.Format("SetCommState failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return NULL;
	}
	if(!SetupComm(hCom, 4096, 512))
	{
		//csErrMsg.Format("SetupComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return NULL;
	}
	cmto.ReadTotalTimeoutConstant=100;
	cmto.WriteTotalTimeoutConstant=100;
	cmto.ReadIntervalTimeout=0;
	cmto.ReadTotalTimeoutMultiplier=0;
	cmto.WriteTotalTimeoutMultiplier=0;
	if(!SetCommTimeouts(hCom, &cmto))
	{
		//csErrMsg.Format("SetCommTimeouts failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return NULL;
	}
	if(!PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return NULL;
	}
	if(!ClearCommError(hCom, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return NULL;
	}
	return hCom;
}

int ComSend(HANDLE handle, CString csSend)
{
	DWORD dwWrite;
	CString csErrMsg,csCmd;
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	if(!PurgeComm(handle,PURGE_TXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -2;
	}
	csCmd.Format("%s\r",csSend.GetString());
	if(!WriteFile(handle, csCmd, csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -3;
	}
	return 0;
}

int ComSendCmdEx(HANDLE handle, CString csSend, CString &csRecv)
{
	DWORD dwWrite;
	CString csTemp;
	csRecv.Empty();
	CString csErrMsg, csCmd;
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	if(!PurgeComm(handle,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -2;
	}
	csCmd.Format("%s\r",csSend.GetString());
	if(!WriteFile(handle, csCmd.GetString(), csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -3;
	}
	for(int i=0; ; i++)
	{
		if(i<600)
		{
			ComRecv(handle, csTemp, TRUE);
			csRecv+=csTemp;
			if(-1 != csRecv.Find("BCM.0>"))
			{
				if(-1 !=csRecv.Find("Error")) return -5;
				break;
			}
		}
		else
		{
			return -4;
		}
	}
	return 0;
}

int ComGetDRICmdRlt(HANDLE handle, CString csSend, CString csRecv, CString csPrompt)
{
	DWORD dwWrite;
	CString csTemp;
	CString csErrMsg, csCmd;
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	if(!PurgeComm(handle,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -2;
	}
	csCmd.Format("%s\r",csSend.GetString());
	if(!WriteFile(handle, csCmd.GetString(), csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -3;
	}
	csRecv.Empty();
	for(int i=0; ; i++)
	{
		if(i<600)
		{
			ComRecv(handle, csTemp, TRUE);
			csRecv+=csTemp;
			if(-1 != csTemp.Find(csPrompt)) break;
		}
		else
		{
			return -4;
		}
	}
	return 0;
}

int ComSendCmd(HANDLE handle, CString csSend)
{
	DWORD dwWrite;
	CString csErrMsg, csCmd;
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	if(!PurgeComm(handle,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -2;
	}
	csCmd.Format("%s\r",csSend.GetString());
	if(!WriteFile(handle, csCmd.GetString(), csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -3;
	}
	return 0;
}

int ComRecv(HANDLE handle, CString &csRecv, bool showMsg)//从串口接受数据
{
	char chRecv;
	DWORD dwRead=0;
	CString csErrMsg, csBuff;
	csRecv.Empty();
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	while(true)
	{
		memset(&chRecv,0,1);
		if(!ReadFile(handle, &chRecv, 1, &dwRead, NULL))//将 handle 中的内容读取到 chRecv 中，1 表示读取一个字节，dwRead 存放实际读取的字节数
		{
			//csErrMsg.Format("ReadFile Error: %d.\n", GetLastError());
			//ShowUIMessage(csErrMsg);
			return -2;
		}
		if(dwRead)
		{
			csRecv.AppendChar(chRecv);
			csBuff.AppendChar(chRecv);
			if(-1!=csBuff.Find("\n"))
			{
				csBuff.Trim();
				if(csBuff.IsEmpty()) continue;
				if(showMsg)	ShowUIMessage(csBuff);
				csBuff.Empty();
			}
		}
		else
		{
			if(!csBuff.IsEmpty())//IsEmpty判断为空就返回 1
				//ShowUIMessage(csBuff);
			break;
		}
	}
	return 0;
}

int ComFind(HANDLE handle, CString &csRecv, CString csFind) //把从串口 handle 接受的数据保存在 csRecv 中，并在其中寻找 csFind ，若找到就返回 0
{
	char chRecv;
	DWORD dwRead=0;
	CString csErrMsg, csBuff;
	csRecv.Empty();
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	do
	{
		memset(&chRecv,0,1);
		if(!ReadFile(handle, &chRecv, 1, &dwRead, NULL))
		{
			return -4;
		}
		else
		{
			csBuff.AppendChar(chRecv);
			csRecv.AppendChar(chRecv);
			if((-1!=csBuff.Find("\n"))||(-1!=csBuff.Find(csFind)))
			{
				csBuff.Trim();
				if(csBuff.IsEmpty())
				{
					continue;
				}
				if(-1!=csBuff.Find(csFind))
				{
					return 0;
				}
				else
				{
					csBuff.Empty();
				}
			}
		}
	}
	while(dwRead);
	return -5;
}

int CloseCom(HANDLE handle)
{
	CString csErrMsg;
	if(!CloseHandle(handle))
	{
		//csErrMsg.Format("CloseHandle failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return -1;
	}
	return 0;
}

CString ComRecvCmd(HANDLE handle,CString csSend,CString &csRecv,CString csPrompt)//向串口发送命令，在接受串口返回的数据，在其中寻找 csPrompt 代表的字符串，若找到就返回接受到的数据，若找不到就返回空字符
{
	DWORD dwWrite;
	CString csTemp;
	nShow=1;
	csRecv.Empty();
	CString csErrMsg, csCmd;
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	if(!PurgeComm(handle,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	csCmd.Format("%s\r",csSend.GetString());
	if(!WriteFile(handle, csCmd.GetString(), csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}

	for(int i=0; ; i++)
	{
		if(i<600)
		{
			//CString s;
			//s.Format("%d", i);
			//ShowUIMessage(s);
			ComRecv(handle, csTemp, TRUE);
			csRecv+=csTemp;
			if(-1 != csRecv.Find(csPrompt)) break;
		}
		else return "";
	}
	return csRecv;
}

CString ComRecvOnTime(HANDLE handle,CString csSend,CString &csRecv,CString csPrompt,int nTime)
{
	DWORD dwWrite;
	CString csTemp;
	nShow=0;
	csRecv.Empty();
	CString csErrMsg, csCmd;
	if(!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	if(!PurgeComm(handle,PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	csCmd.Format("%s\r",csSend.GetString());
	if(!WriteFile(handle, csCmd.GetString(), csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	for(int i=0; ; i++)
	{
		if(i<nTime)
		{
			ComRecv(handle, csTemp, TRUE);
			csRecv+=csTemp;
			if(-1 != csRecv.Find(csPrompt)) break;
		}
		else return "";
	}
	return csRecv;
}

CString ComRecvCmdLong(HANDLE handle, CString csSend, CString &csRecv, CString csPrompt)
{
	DWORD dwWrite;
	CString csTemp;
	nShow = 1;
	csRecv.Empty();
	CString csErrMsg, csCmd;

	if (!ClearCommError(handle, NULL, NULL))
	{
		//csErrMsg.Format("ClearCommError failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	if (!PurgeComm(handle, PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		//csErrMsg.Format("PurgeComm failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}
	csCmd.Format("%s\r", csSend.GetString());
	if (!WriteFile(handle, csCmd.GetString(), csCmd.GetLength(), &dwWrite, NULL))
	{
		//csErrMsg.Format("WriteFile failed: %d.\n", GetLastError());
		//ShowUIMessage(csErrMsg);
		return "";
	}

	for (int i = 0; ; i++)
	{
		if (i<10000)
		{
			ComRecv(handle, csTemp, TRUE);
			csRecv += csTemp;
			if (-1 != csRecv.Find(csPrompt)) break;
		}
		else return "";
	}
	return csRecv;
}

