#ifndef COMM_H
#define COMM_H
//#pragma once
//#include "atlstr.h"
BOOL	PingIP(LPCTSTR ip, UINT durSec = 2000);
BOOL	PingMe(CString csIP, UINT nTimeOut = 200);
BOOL	PingMeSize(CString csIP, UINT nTimeOut = 200, WORD nPacketSize=32);
BOOL	PingMe(char strIP[16], UINT nTimeOut = 200);
BOOL	CutCString(CString &sourceStr, CString startTargetStr);
BOOL	GetCString(CString sourceStr, CString startTargetStr, CString startSepStr, CString endSepStr, CString &resultStr);
BOOL	GetCString(CString sourceStr,CString startSepStr, CString endSepStr, CString &resultStr);
	
CString GetString(CString sourceStr,PSTR targetStr,char startChar,char endChar);
int		GetIniInt(CString csModle, PSTR iniFileName,PSTR iniSection,PSTR iniKey,int defVal);
double	GetIniDouble(CString csModle, PSTR iniFileName,PSTR iniSection,PSTR iniKey,double defVal);
void	GetIniString(CString csModel, char * iniFileName, char * iniSection, char * iniKey, char * defVal, char * strBuf);
void	WriteIniString(char * iniFileName, char * iniSection, char * iniKey, char * strBuf);
BOOL GetCStrings(CString sourceStr, CString startSepStr, CString endSepStr, CString &resultStr, int num);
bool Str2MAC(unsigned char *MAC, char *str);
void MAC2Str(unsigned char MAC[6], char *str);
bool MACIncr(CString MACIn,CString &MACOut);
bool MACDecr(CString MACIn,CString &MACOut);
bool Hex2Long(long long *macnum, char *str);

#endif