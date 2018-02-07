#include "stdafx.h"
#include "Comm.h"
#include "IOSocket.h"
#include "Ping.h"
#include ".\AtsComm\AtsComm.h"

//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------

BOOL CutCString(CString &sourceStr, CString startTargetStr)
{
	CString str1;
	int pos1;
	str1 = sourceStr;

	if ( ( pos1 = str1.Find(startTargetStr) ) == -1 )
		return FALSE;

	sourceStr = str1.Mid(pos1,str1.GetLength()-pos1);

	return TRUE;
}

BOOL GetCString(CString sourceStr, CString startTargetStr, CString startSepStr, CString endSepStr, CString &resultStr)
{                                         //此函数首先要再源字符串中找到 startTargetStr ，再从这个 startTargetStr 起始位置开始根据标志截取字符串
	CString str1,str2;
	int pos1, pos2;
	CString str3; //Winnie add

	resultStr.Empty();

	str1 = str2 = sourceStr;

	if ( ( pos1 = str1.Find(startTargetStr) ) == -1 )
		return FALSE;

	str1 = str1.Mid(pos1,str1.GetLength()-pos1);
	if ( ( pos1 = str1.Find(startSepStr)) == -1 )
		return FALSE;

	if ( ( pos2 = str1.Find(endSepStr, pos1+startSepStr.GetLength()) ) == -1 )
	{
		resultStr = "";
		return FALSE;
	}

	if ( pos1 >= pos2-1 ) 
		return FALSE;

	resultStr = str1.Mid( pos1+startSepStr.GetLength() , pos2-(pos1+startSepStr.GetLength()));

	if ( resultStr.GetLength() < 1 )
		return FALSE;

	return TRUE;
}

BOOL GetCString(CString sourceStr, CString startSepStr, CString endSepStr, CString &resultStr)
{
	CString str1;
	int pos1, pos2;

	str1 = sourceStr;

	if ( ( pos1 = str1.Find(startSepStr)) == -1 )
		return FALSE;

	if ( ( pos2 = str1.Find(endSepStr, pos1+startSepStr.GetLength()) ) == -1 )
	{
		resultStr = "";
		return FALSE;
	}

	if ( pos1 >= pos2-1 ) 
		return FALSE;

	resultStr = str1.Mid( pos1+startSepStr.GetLength() , pos2-(pos1+startSepStr.GetLength()));

	if ( resultStr.GetLength() < 1 )
		return FALSE;

	return TRUE;
}
BOOL PingIP(LPCTSTR ip, UINT durSec)
{
	int nLimit = durSec/200;
	int nRetPingPass = 0, nRetPingFail = 0;

	while ( nRetPingPass < nLimit && nRetPingFail < nLimit )
	{
		if ( PingMe(ip, 200) )
		{
			nRetPingPass++;
			nRetPingFail = 0;
		}
		else
		{
            nRetPingFail++;
			nRetPingPass = 0;
		}

		Sleep(10);
	}

	if ( nRetPingPass < nLimit )
		return FALSE;
	else
		return TRUE;
}

BOOL PingMe(CString csIP, UINT nTimeOut)
{
	CPing		PingFoo;
	CPingReply	pr;
	char		strIP[16] = {0};

	sprintf_s(strIP,sizeof(strIP),"%s",csIP);

	//sprintf(strIP, "%s", csIP.GetBuffer());
    
	return PingFoo.Ping1(strIP, pr, 10, nTimeOut, 32);
}
BOOL PingMeSize(CString csIP, UINT nTimeOut, WORD nPacketSize)
{
	CPing		PingFoo;
	CPingReply	pr;
	char		strIP[16] = {0};

	sprintf_s(strIP, "%s", csIP);
    
	return PingFoo.Ping1(strIP, pr, 128, nTimeOut, nPacketSize);
}

BOOL PingMe(char strIP[16], UINT nTimeOut)
{
	CPing		PingFoo;
	CPingReply	pr;
	
    return PingFoo.Ping1(strIP, pr, 10, nTimeOut, 32);	
}

void GetIniString(CString csModel,char * iniFileName, char * iniSection, char * iniKey, char * defVal, char * strBuf)
{
	char path[_MAX_PATH];
	char strTemp[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, strTemp);

	sprintf_s(path,"%s\\%s\\%s\\%s",strTemp,"Models",csModel,iniFileName);
	GetPrivateProfileString(iniSection,iniKey,defVal,strBuf,1500,path);
}

void WriteIniString(char * iniFileName, char * iniSection, char * iniKey, char * strBuf)
{
	char path[_MAX_PATH];
	char strTemp[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, strTemp);

	sprintf_s(path,"%s\\%s",strTemp,iniFileName);
	WritePrivateProfileString( iniSection, iniKey, strBuf, path);
}

int GetIniInt(CString csModel, PSTR iniFileName, PSTR iniSection, PSTR iniKey, int defVal)
{
	char path[_MAX_PATH];
	char strTemp[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, strTemp);
	sprintf_s(path,"%s\\%s\\%s\\%s",strTemp,"Models",csModel,iniFileName);
	return GetPrivateProfileInt(iniSection,iniKey,defVal,path);
}

double GetIniDouble(CString csModel, PSTR iniFileName, PSTR iniSection, PSTR iniKey, double defVal)
{
	char path[_MAX_PATH];
	char strTemp[_MAX_PATH];
	char deftmp[255];
	char strBuf[255];
	double dRetValue = 0.0f;
	GetCurrentDirectory(_MAX_PATH, strTemp);
	sprintf_s(deftmp, "%d", defVal);
	sprintf_s(path,"%s\\%s\\%s\\%s",strTemp,"Models",csModel,iniFileName);
	GetPrivateProfileString(iniSection,iniKey,deftmp,strBuf,1500,path);
	dRetValue = atof(strBuf);
	return dRetValue;
}

CString GetString(CString sourceStr,PSTR targetStr,char startChar,char endChar)
{
	char *pdest;
	char strBuf[1000], temp[255];
	int  localAddr=0;
	int strIndex=0;

	sprintf_s(temp, "%s", sourceStr);
	pdest=(char *)strstr(temp,targetStr);
	while(pdest[localAddr++]!= startChar);
	while(pdest[localAddr]!= endChar)
		strBuf[strIndex++]=pdest[localAddr++];

	strBuf[strIndex]=NULL;
	return strBuf;
}

bool Str2MAC(unsigned char *MAC, char *str)
{	
	for (int i = 0; i < 12; i++ )//将代表十六进制数的字符转化为数值
	{
		if (( str[i] >= '0' ) && ( str[i] <= '9' ))
		{
			str[i] -= '0';
		}
		else if (( str[i] >= 'a' ) && ( str[i] <= 'f' ))
		{
			str[i] -= ('a'-10);
		}
		else if (( str[i] >= 'A' ) && ( str[i] <= 'F' ))
		{
			str[i] -= ('A'-10);
		}
		else
		{
			return false;
		}
	}
	for (int i = 0; i < 6; i++ )            //change MAC to Number from char
	{
		MAC[i] = 16 * str[2 * i] + str[2 * i + 1];//将16进制转化为10进制
	}
	return true;
}

void MAC2Str(unsigned char MAC[6], char *str)
{
	sprintf_s(str,sizeof(MAC)*3-1, "%02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
}

bool MACIncr(CString MACIn,CString &MACOut)//若MAC 的后3段全为 0xff，则该函数返回 0 ，否则返回 1
{
	BYTE BMac[6];
	memset(BMac, 0, sizeof(BMac));
	char czTemp[25];
	memset(czTemp,0,sizeof(czTemp));
//	ShowUIMessage("MACIncr");

	MACIn.Remove(':');
	MACOut.Empty();
	Str2MAC(BMac, MACIn.GetBuffer());
	
	for(int i=5; i>2; i--)
	{
		if(BMac[i]==0xff)
		{
			BMac[i]=0x00;
			if(i==3)
			{
				return false;
			}
		}
		else
		{
			BMac[i]++;
			break;
		}
	}
	//MAC2Str(BMac,czTemp);
	//MACOut=czTemp;
	MACOut.Format("%02X:%02X:%02X:%02X:%02X:%02X", BMac[0], BMac[1], BMac[2], BMac[3], BMac[4], BMac[5]);
	return true;
}

bool MACDecr(CString MACIn,CString &MACOut)
{
	BYTE BMac[7];
	char czTemp[18];
	memset(BMac, 0, sizeof(BMac));
	memset(czTemp,0,sizeof(czTemp));
//	ShowUIMessage("MACDecr");

	MACIn.Remove(':');
	MACOut.Empty();
	Str2MAC(BMac, MACIn.GetBuffer());
//	CString csTemp;
//	csTemp.Format("%02X%02X%02X%02X%02X%02X",BMac[0],BMac[1],BMac[2],BMac[3],BMac[4],BMac[5]);
//	ShowUIMessage(csTemp);

	for(int i=5; i>2; i--)
	{
		if(BMac[i]==0x00)
		{
			BMac[i]=0xff;
			if(i==3)
			{
				return false;
			}
		}
		else
		{
			BMac[i]--;
			break;
		}
	}
//	csTemp.Format("%02X%02X%02X%02X%02X%02X",BMac[0],BMac[1],BMac[2],BMac[3],BMac[4],BMac[5]);
//	ShowUIMessage(csTemp);

	MAC2Str(BMac,czTemp);
	MACOut=czTemp;
	return true;
}