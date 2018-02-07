// SocketDx.h: interface for the CSocketDx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETDX_H__DFEE9DED_9AF1_11D2_A6CE_204C4F4F5020__INCLUDED_)
#define AFX_SOCKETDX_H__DFEE9DED_9AF1_11D2_A6CE_204C4F4F5020__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#include "atlstr.h"
class CSocketDx  
{
public:
	CSocketDx();
	~CSocketDx();
	int Create();
	int Connect();
	void OnSend(char *sendMsg);
	void OnClose();
	CString OnReceive(void);
	CString OnRecv(int nTimeout=3);
	int TelnetConnect(CString csIP);
	bool RecvString(CString csFind, CString &csRecv, int nTime = 30);
	int OnCmd(CString csCmd, CString &csRecv, int nTimeout=5, CString csPrompt="#");
	bool RecvResult(CString csPrompt,int nTime=5);
	CString Recv(CString &csRecv,int nTime=3);

	sockaddr_in m_sockaddr_in;
	SOCKET m_hSocket;
};

#endif // !defined(AFX_SOCKETDX_H__DFEE9DED_9AF1_11D2_A6CE_204C4F4F5020__INCLUDED_)
