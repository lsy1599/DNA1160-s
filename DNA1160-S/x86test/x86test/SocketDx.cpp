// SocketDx.cpp: implementation of the CSocketDx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "winsock2.h"
#include "SocketDx.h"
#include "AtsComm\AtsComm.h"
#include "Comm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketDx::CSocketDx()
{
	m_hSocket=NULL;
}

CSocketDx::~CSocketDx()
{

}

int CSocketDx::Create()
{
	//WSADATA wsd;
	unsigned long val=1;

	//if(WSAStartup(MAKEWORD(2,0),&wsd)) return 0;
	m_hSocket = socket(PF_INET,SOCK_STREAM,0);
	if(m_hSocket == INVALID_SOCKET)
	{
		m_hSocket=NULL;
		return -1;
	}
	if(ioctlsocket(m_hSocket,FIONBIO,&val)==SOCKET_ERROR) return -1;
	return 0;
}

int CSocketDx::Connect()
{
	int nRet;
	fd_set fdreadd;
	struct timeval tv;
//	unsigned long val = 0;
	
	connect(m_hSocket,(sockaddr*)&m_sockaddr_in,sizeof(sockaddr));
	FD_ZERO(&fdreadd);
	FD_SET(m_hSocket,&fdreadd);
	tv.tv_sec=1;
	tv.tv_usec= 0;
	
	nRet = select(0,0,&fdreadd,0,&tv);
	if ( nRet <= 0 ) return false;

//	ioctlsocket(m_hSocket,FIONBIO,&val);
	return true;
}

int CSocketDx::TelnetConnect(CString csIP)
{
	int nRet;
	fd_set fdRead;
	timeval tv;
	tv.tv_sec=5;
	tv.tv_usec=0;
	char pBuf[64];
	memset(pBuf, 0, sizeof(pBuf));
	
	m_sockaddr_in.sin_family = AF_INET;
	m_sockaddr_in.sin_port = htons(23);
	m_sockaddr_in.sin_addr.s_addr = inet_addr(csIP.GetString());
	
	nRet = Create();
	if ( nRet < 0 ) 
		return -1;

	nRet = Connect();
	if ( nRet < 0 ) 
	{
		return -2;
	}

	FD_ZERO(&fdRead);
	FD_SET(m_hSocket, &fdRead);
	select(0, &fdRead, NULL, NULL, &tv);
	if(!FD_ISSET(m_hSocket, &fdRead))
	{
		return -3;
	}
	return 0;
}

void CSocketDx::OnSend(char *msg)
{
	int dwLen;
	char ch;
	int nRet;
	int i=0;
	dwLen = 1;
	while(msg[i] !=NULL)
	{
		ch = msg[i++];
		nRet = send(m_hSocket,&ch,dwLen,0);
	}
	ch = '\r';
	nRet = send(m_hSocket,&ch,dwLen,0);
}

CString CSocketDx::OnReceive(void)
{
	char pBuf[5000]; //Griffin 081007 2000->5000
	int nRet;
	memset(pBuf , 0 , sizeof(pBuf));
	nRet = recv(m_hSocket,pBuf,sizeof(pBuf),0);

	CString buf;
	buf.Format("%s", pBuf);
	int pos = buf.Find("# ÌÌ"); //°£¥h¶Ã½X¦r¤¸

	if ( pos != -1 )
		buf = buf.Left(pos);
	
	return buf;
}

CString CSocketDx::OnRecv(int nTimeout)
{
	char pBuf[1024];
	timeval tv;
	fd_set fdRead;

	tv.tv_sec=nTimeout;
	tv.tv_usec=0;
	memset(pBuf , 0 , sizeof(pBuf));

	FD_ZERO(&fdRead);
	FD_SET(m_hSocket, &fdRead);
	select(0, &fdRead, NULL, NULL, &tv);
	if(FD_ISSET(m_hSocket, &fdRead))
	{
		recv(m_hSocket,pBuf,sizeof(pBuf),0);
	}
	return CString(pBuf);
}

void CSocketDx::OnClose(void)
{
	closesocket(m_hSocket);
}

bool CSocketDx::RecvString(CString csFind, CString &csRecv, int nTime)
{
	fd_set fdRead;
	timeval tv;
	tv.tv_sec=nTime;
	tv.tv_usec=0;

	csRecv.Empty();

	while(-1 ==csRecv.Find(csFind))
	{
		FD_ZERO(&fdRead);
		FD_SET(m_hSocket, &fdRead);
		select(0, &fdRead, NULL, NULL, &tv);
		if(FD_ISSET(m_hSocket, &fdRead))
		{
			csRecv+=OnReceive();
		}
		else
		{
			return false;
		}
	}
	ShowUIMessage(csRecv);
	return true;
}

int CSocketDx::OnCmd(CString csCmd, CString &csRecv, int nTimeout, CString csPrompt)
{
	fd_set fdRead;
	timeval tv;
	tv.tv_sec=nTimeout;
	tv.tv_usec=0;

	csRecv.Empty();
	OnSend(csCmd.GetBuffer());
	while(-1 ==csRecv.Find(csPrompt))
	{
		FD_ZERO(&fdRead);
		FD_SET(m_hSocket, &fdRead);
		select(0, &fdRead, NULL, NULL, &tv);
		if(FD_ISSET(m_hSocket, &fdRead))
		{
			csRecv += OnReceive();
		}
		else
		{
			if(!csRecv.IsEmpty())
			{
				//if(-1==csRecv.Find("unrecognized command"))
				//{
					ShowUIMessage(csRecv);
				//}
			}
			return -1;
		}
	}
	ShowUIMessage(csRecv);
	return 0;
}
bool CSocketDx::RecvResult(CString csPrompt,int nTime)
{
	CString csRecv;
	for (int i=0; i<(2*nTime); i++)
	{
		Sleep(500);
		csRecv += OnReceive();
		if(!csRecv.IsEmpty())
		{
			ShowUIMessage(csRecv);
		}
		if(-1 != csRecv.Find(csPrompt))
		{
			return true;
		}
	}
	return false;
}

CString CSocketDx::Recv(CString &csRecv,int nTime)
{
	for(int i=0;i<nTime;i++)
	{
		Sleep(500);
		csRecv +=OnReceive();
		if(!csRecv.IsEmpty())
		{
			//ShowUIMessage(csRecv);
		}
	}
	return csRecv;
}
