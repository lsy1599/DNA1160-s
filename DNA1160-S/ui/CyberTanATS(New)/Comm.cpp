#include "stdafx.h"
#include "Comm.h"
#include "ping.h"
#include "string.h"

unsigned char HEX_2_INT_TABLE[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 
            6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


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
{
	CString str1,str2;
	int pos1, pos2;

	str1 = str2 = sourceStr;

	if ( ( pos1 = str1.Find(startTargetStr) ) == -1 )
		return FALSE;

	str1 = str1.Mid(pos1,str1.GetLength()-pos1);

	if ( ( pos1 = str1.Find(startSepStr) ) == -1 )
		return FALSE;

	if ( ( pos2 = str1.Find(endSepStr, pos1+1) ) == -1 )
		return FALSE;

	if ( pos1 >= pos2-1 ) 
		return FALSE;

	resultStr = str1.Mid( pos1+1 , pos2-(pos1+1));

	if ( resultStr.GetLength() < 1 )
		return FALSE;

	return TRUE;
}


void InsertStr(char *beInsertBuf,int insertLocal,CString strBuf)
{
	int originalCount;
	int strLength;
	char HttpCommand[1500]={0};

	strLength=strBuf.GetLength();
	for(originalCount=0;originalCount<insertLocal;originalCount++)
		HttpCommand[originalCount]=beInsertBuf[originalCount];

	while(beInsertBuf[originalCount]!= '=')
		HttpCommand[originalCount++]=beInsertBuf[originalCount];
	int count=0;
	if(beInsertBuf[originalCount+1]!= '&')
	{
		HttpCommand[originalCount]=beInsertBuf[originalCount++];
		while(beInsertBuf[originalCount+(count++)+1]!= '&');
	}
	else
		HttpCommand[originalCount]=beInsertBuf[originalCount++];
	sprintf_s(HttpCommand,"%s%s",	HttpCommand,strBuf);

	for(originalCount=originalCount;originalCount<((int)(strlen(beInsertBuf)+strLength));originalCount++)
		HttpCommand[originalCount+strLength]=beInsertBuf[originalCount+count];
	strcpy_s(beInsertBuf,sizeof(HttpCommand),HttpCommand);
}

void GetIniString(char * iniFileName, char * iniSection, char * iniKey, char * defVal, char * strBuf)
{
	char path[_MAX_PATH];
	char strTemp[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, strTemp);

	sprintf_s(path,"%s\\%s",strTemp,iniFileName);
	GetPrivateProfileString(iniSection,iniKey,defVal,strBuf,1500,path);
}

int GetIniInt(PSTR iniFileName, PSTR iniSection, PSTR iniKey, int defVal)
{
	char path[_MAX_PATH];
	char strTemp[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, strTemp);
	sprintf_s(path,"%s\\%s",strTemp,iniFileName);
	return GetPrivateProfileInt(iniSection,iniKey,defVal,path);

}

char *GetString(CString sourceStr,PSTR targetStr,char startChar,char endChar)
{
	char *pdest;
	char strBuf[1000];
	int  localAddr=0;
	int strIndex=0;


	//if(sourceStr.Find()==-1)
	//	return "-999";
	pdest=(char *)strstr(sourceStr,targetStr);
	while( (pdest[localAddr++]!= startChar) && (localAddr <= sourceStr.GetLength()) );
	//	localAddr++;
	while((pdest[localAddr]!= endChar) && (localAddr <= sourceStr.GetLength()) )
		strBuf[strIndex++]=pdest[localAddr++];

	strBuf[strIndex] = NULL;
	pdest = strBuf;
	if(strlen(pdest))
		return pdest;
	return("-999");
}

void SetStrtoArry(char *strBuf,int *array)
{
	int i,index,j;
	char temp[10];

	i=0;index=0;
	while(strBuf[0]!=NULL && strBuf[0]!='0')
	{
		j=0;
		memset(temp,0,10);
		while(strBuf[i]!=',' && strBuf[i]!=NULL && strBuf[i]!=';' && strBuf[i]!='#' && strBuf[i]!='/' &&  strBuf[i]!=NULL)
		{
			if(strBuf[i]==' ' || strBuf[i] == 9)
			{
				i++;
				continue;
			}
			temp[j++]=strBuf[i++];
		}
		array[index++]=atoi(temp);
		if((strBuf[i]==NULL)||strBuf[i]==';'|| strBuf[i]=='#')
			break;
		i++;		
	}
}

void SetStrtoArry(char *strBuf,double *array)
{
	int i,index,j;
	char temp[10];

	i=0;index=0;
	while(strBuf[0]!=NULL && strBuf[0]!='0')
	{
		j=0;
		memset(temp,0,10);
		while(strBuf[i]!=',' && strBuf[i]!=NULL && strBuf[i]!=';' && strBuf[i]!='#')
		{
			if(strBuf[i]==' ' || strBuf[i] == 9)
			{
				i++;
				continue;
			}
			temp[j++]=strBuf[i++];
		}
		array[index++]=atof(temp);
		if((strBuf[i]==NULL)||strBuf[i]==';'|| strBuf[i]=='#')
			break;
		i++;		
	}
}


int HexStr2Int(char* hexstr)
{
    register unsigned int length, i, value, shift;
    for (length = 0; length < 9; length++) if (!hexstr[length]) break;
    shift = (length - 1) * 4;
    for (i = value = 0; i < length; i++, shift -= 4)
        value += HEX_2_INT_TABLE[(unsigned int)hexstr[i] & 127] << shift;
    return value;
}

int ConvertStrToInt( char* s )
{
    int  n = 0;
    char c = *s++;

    while( c >= '0' && c <= '9' )
    {
        n = n * 10 + (c - '0');    
        c = *s++;
    }

    return n;
}

void Str2MAC(unsigned char *MAC, char *str)
{	
	int i;

	for ( i = 0; i < 12; i++ )
	{
		if (( str[i] >= '0' ) && ( str[i] <= '9' ))  str[i] -= '0';
                if (( str[i] >= 'a' ) && ( str[i] <= 'f' ))  str[i] -= 87;
                if (( str[i] >= 'A' ) && ( str[i] <= 'F' ))  str[i] -= 55;
	}
	for ( i = 0; i < 6; i++ )            //change MAC to Number from char
                MAC[i] = 16 * str[2 * i] + str[2 * i + 1];
}

BOOL PingMe(CString csIP, CString csSourceIP, float fTimeOut)
{
	CPing		PingFoo;
	CPingReply	pr;
	char		strIP[16] = {0};

	sprintf(strIP, "%s", csIP);
   
	return PingFoo.Ping3(strIP, pr, 10, (DWORD)(fTimeOut*1000), 32, csSourceIP);
}

BOOL PingMe(char strIP[16], CString csSourceIP, float fTimeOut)
{
	CPing		PingFoo;
	CPingReply	pr;
	
    return PingFoo.Ping3(strIP, pr, 10, (DWORD)(fTimeOut*1000), 32, csSourceIP);	
}