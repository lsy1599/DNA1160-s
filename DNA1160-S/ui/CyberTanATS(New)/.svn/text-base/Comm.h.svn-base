#ifndef COMM_H
#define COMM_H

//ONTIMER ID DEFINE



//BOOL PingIP(LPCTSTR ip, UINT durSec = 2000);
BOOL CutCString(CString &sourceStr, CString startTargetStr);
BOOL GetCString(CString sourceStr, CString startTargetStr, CString startSepStr, CString endSepStr, CString &resultStr);
BOOL PingMe(CString csIP, CString csSourceIP = "any", float fTimeOut = 10.0);//BOOL PingMe(CString csIP, CString csSourceIP = "any", float fTimeOut = 2.0);
BOOL PingMe(char strIP[16], CString csSourceIP = "any", float fTimeOut = 10.0);	//BOOL PingMe(char strIP[16], CString csSourceIP = "any", float fTimeOut = 2.0);	


void InsertStr(char *beInsertBuf,int insertLocal,CString strBuf);
//void GetString(CString sourceStr,PSTR targetStr,char startChar,char endChar,CString strVal);
char *GetString(CString sourceStr,PSTR targetStr,char startChar,char endChar);

int GetIniInt(PSTR iniFileName,PSTR iniSection,PSTR iniKey,int defVal);
void GetIniString(PSTR iniFileName,PSTR iniSection,PSTR iniKey,PSTR defVal,PSTR strBuf);
void SetStrtoArry(char *strBuf,int *array);
void SetStrtoArry(char *strBuf,double *array);
void Str2MAC(unsigned char *MAC, char str[12]);

int HexStr2Int(char* hexstr);
int ConvertStrToInt( char* s );

#endif