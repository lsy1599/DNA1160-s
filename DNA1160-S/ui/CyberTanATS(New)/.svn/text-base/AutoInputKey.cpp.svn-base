#include "StdAfx.h"
#include "AutoInputKey.h"

CAutoInputKey::CAutoInputKey(void)
{
	::GetCurrentDirectory(256,m_strBuf);
	sprintf_s(m_iniFileName,"%s\\ATSCfg.ini",m_strBuf);
}

CAutoInputKey::~CAutoInputKey(void)
{
}

CString CAutoInputKey::GetMac(void)
{	
	GetPrivateProfileString("InputKey", "MacStart", "000000000000", m_MacAddr, sizeof(m_MacAddr), m_iniFileName);
	return m_MacAddr;
}

void CAutoInputKey::MacCounter(void)
{
    int i,j;
	BYTE byteMac[7];

	char temp[3];

	for(i=0,j=0; i<12; i+=2,j+=1)
		byteMac[j]=(ASC2Value(m_MacAddr[i])<<4) | (ASC2Value(m_MacAddr[i+1])&0x0f);  
    if (byteMac[5] < 255 )
        byteMac[5] = byteMac[5] + 1;
    else
	{
        byteMac[5] = 0;
        if( byteMac[4] < 255)
            byteMac[4] = byteMac[4] + 1;
        else
		{
            byteMac[4] = 0;
            if( byteMac[3] < 255)
                byteMac[3] = byteMac[3] + 1;
            else
                byteMac[3] = 0;
        }
	}
	_itoa(byteMac[3],temp,16);

	if (temp[1]==0)
	{
		m_MacAddr[6]='0';
		m_MacAddr[7]=temp[0];
	}
	else
	{
		m_MacAddr[6]=temp[0];
		m_MacAddr[7]=temp[1];	
	}
	itoa(byteMac[4],temp,16);
	if (temp[1]==0)
	{
		m_MacAddr[8]='0';
		m_MacAddr[9]=temp[0];
	}
	else
	{
		m_MacAddr[8]=temp[0];
		m_MacAddr[9]=temp[1];
	}
	_itoa(byteMac[5],temp,16);
	if (temp[1]==0)
	{
		m_MacAddr[10]='0';
		m_MacAddr[11]=temp[0];
	}
	else
	{
		m_MacAddr[10]=temp[0];
		m_MacAddr[11]=temp[1];
	}
	m_MacAddr[12]=NULL;
	WritePrivateProfileString( "InputKey", "MacStart",m_MacAddr, m_iniFileName);

}

void CAutoInputKey::ByteToAscii( BYTE *instr,int len)
{
	int i,j=0;
	char temp[10];
	char strtemp[20];

	strtemp[j]='\0';
	for( i=0 ;i<len;i++)
	{
		if(  instr[i] <= 15)
		{
			strtemp[j++]= '0';	
			_itoa( instr[i],temp,16);
			strtemp[j++]=temp[0];
		}
		else
		{
			_itoa( instr[i],temp,16);
			strcat(strtemp,temp);
			j+=2;
		}
		strtemp[j]='\0';
	}

}

BYTE CAutoInputKey::ASC2Value(char ASC)
{
	BYTE val=0;
	if (ASC >= '0' && ASC <= '9')
		val=ASC-'0';
	if (ASC >= 'A' && ASC <= 'F')
		val = ASC - 'A' + 10;
	if (ASC >= 'a' && ASC <= 'f')
		val = ASC - 'a' + 10;
    return val;
}
