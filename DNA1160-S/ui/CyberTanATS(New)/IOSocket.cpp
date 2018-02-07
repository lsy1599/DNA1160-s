#include "stdafx.h"
#include "IOSocket.h"

USHORT Checksum(USHORT* pBuffer, int nSize);

SOCKET  ConnectRemote(CString remoteIP, unsigned short port,int protocol,int nettimeout)
{    
	LPHOSTENT	lpHostEntry;
	SOCKET	    hSocket;
	WORD		wVersionRequested = MAKEWORD(2,0);
	WSADATA		wsaData;
	int			nRet;
	int			socketType=SOCK_STREAM;
	
	// Initialize WinSock and check the version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if(nRet)	// Error
		return INVALID_SOCKET;

	if (wsaData.wVersion != wVersionRequested) //OD(" Wrong version");	
		return INVALID_SOCKET;

	// Find the server
	lpHostEntry = gethostbyname(remoteIP);
    if (lpHostEntry == NULL)
    {
        //OD("[CLIENT] gethostbyname() error");
		WSACleanup();
        return INVALID_SOCKET;
    }

	if(protocol==IPPROTO_UDP)
		socketType=SOCK_DGRAM;
	// Create a TCP/IP stream socket
	hSocket = socket(AF_INET,			// Address family
					   socketType,			// Socket type
					   protocol);		// Protocol IPPROTO_TCP(6) IPPROTO_UDP(17)
	if (hSocket == INVALID_SOCKET)
	{
		//OD("[CLIENT] invalid socket()");
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Fill in the address structure
	//
	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);	// Server's address
	saServer.sin_port = htons(port);	

	//
	// connect to the server
	//

	nRet = connect(hSocket,					// Socket
				   (LPSOCKADDR)&saServer,	// Server address
				   sizeof(struct sockaddr));// Length of server address structure
 	if (nRet == SOCKET_ERROR)
	{
		closesocket(hSocket);
		//OD("[CLIENT] connect error");
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Set the sending timeout
	setsockopt(hSocket,
			SOL_SOCKET,			               
			SO_SNDTIMEO,     
			(const char *)&nettimeout,  
			sizeof(DWORD)                
			);

	// Set the receiving timeout
	setsockopt(hSocket,
			SOL_SOCKET,			               
			SO_RCVTIMEO,     
			(const char *)&nettimeout,  
			sizeof(DWORD)                
			);

	//OD("[CLIENT] Connected..");

	return hSocket;     
}

SOCKET  BindListenUDP(unsigned short port)
{    
//	LPHOSTENT	lpHostEntry;
	WORD		wVersionRequested = MAKEWORD(2,0);
	WSADATA		wsaData;
	int			nRet;
	SOCKET		wSocket;


	// Initialize WinSock and check the version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if(nRet)	// Error
	{
		return INVALID_SOCKET;
	}

	if (wsaData.wVersion != wVersionRequested)
	{	
		//OD(" Wrong version");
		return INVALID_SOCKET;
	}
	// Create a TCP/IP stream socket
	wSocket = socket(AF_INET,			// Address family
					   SOCK_DGRAM,			// Socket type
					   IPPROTO_UDP);		// Protocol IPPROTO_TCP IPPROTO_UDP
	if (wSocket == INVALID_SOCKET)
	{
		//OD("[CLIENT] invalid socket()");
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKADDR_IN gServer;
	gServer.sin_family = AF_INET;
    gServer.sin_addr.s_addr = INADDR_ANY;
//	gServer.sin_addr.s_addr = inet_addr("127.0.0.1");

	gServer.sin_port=htons(port);

	nRet=bind(wSocket,					//socket
				(LPSOCKADDR)&gServer,		//Our address
				sizeof(struct sockaddr));	//Size of address structure
	if(nRet == SOCKET_ERROR)
		return INVALID_SOCKET;
	char szServer[_MAX_PATH];
	nRet = gethostname(szServer,sizeof(szServer));
	if(nRet == SOCKET_ERROR)
		return INVALID_SOCKET;

	return wSocket;
}

SOCKET  BindSockt(char * bindIP, unsigned short port,int protocal )
{    
//	LPHOSTENT	lpHostEntry;
	WORD		wVersionRequested = MAKEWORD(2,0);
	WSADATA		wsaData;
	int			nRet;
	SOCKET		wSocket;


	// Initialize WinSock and check the version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if(nRet)	// Error
	{
		return INVALID_SOCKET;
	}

	if (wsaData.wVersion != wVersionRequested)
	{	
		//OD(" Wrong version");
		return INVALID_SOCKET;
	}
	if(protocal==IPPROTO_UDP)
	{
		// Create a TCP/IP stream socket
		wSocket = socket(AF_INET,				// Address family
						SOCK_DGRAM,			// Socket type
						IPPROTO_UDP);		// Protocol IPPROTO_TCP IPPROTO_UDP
		if (wSocket == INVALID_SOCKET)
		{
			//OD("[CLIENT] invalid socket()");
			WSACleanup();
			return INVALID_SOCKET;
		}
	}
	else if(protocal==IPPROTO_TCP)
	{
		// Create a TCP/IP stream socket
		wSocket = socket(AF_INET,				// Address family
						SOCK_STREAM,			// Socket type
						IPPROTO_TCP);		// Protocol IPPROTO_TCP IPPROTO_UDP
		if (wSocket == INVALID_SOCKET)
		{
			//OD("[CLIENT] invalid socket()");
			WSACleanup();
			return INVALID_SOCKET;
		}
	}
	else
		return INVALID_SOCKET;

	SOCKADDR_IN gServer;
	gServer.sin_family = AF_INET;
//   gServer.sin_addr.s_addr = INADDR_ANY;
	gServer.sin_addr.s_addr = inet_addr(bindIP);

	gServer.sin_port=htons(port);

	nRet=bind(wSocket,						//socket
				(LPSOCKADDR)&gServer,		//Our address
				sizeof(struct sockaddr));	//Size of address structure
	if(nRet == SOCKET_ERROR)
		return INVALID_SOCKET;
	char szServer[_MAX_PATH];
	nRet = gethostname(szServer,sizeof(szServer));
	if(nRet == SOCKET_ERROR)
		return INVALID_SOCKET;

	return wSocket;
}



void  DisconnectRemote(SOCKET remoteSocket)
{
	closesocket(remoteSocket);
	remoteSocket = INVALID_SOCKET;
}

USHORT Checksum(USHORT* pBuffer, int nSize)
{
	unsigned long nCheckSum = 0;

	while(nSize >1) 
	{
		nCheckSum += *pBuffer++;
		nSize	  -=sizeof(USHORT);
	}
  
	if(nSize) 
		nCheckSum += *(UCHAR*)pBuffer;

	nCheckSum = (nCheckSum >> 16) + (nCheckSum & 0xffff);
	nCheckSum += (nCheckSum >>16);

	return (USHORT)(~nCheckSum);
}