#include "stdafx.h"
#include "IOSocket.h"

USHORT Checksum(USHORT* pBuffer, int nSize);

SOCKET ConnectRemote(CString remoteIP, unsigned short port,int protocol,int nettimeout)
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

SOCKET BindSockt(char * bindIP, unsigned short port,int protocal )
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
	//remoteSocket = INVALID_SOCKET;
}

bool Ping(CString pingIP,int timeOut) //s
{
	int i=0;
	for(i =1 ; i <= timeOut ; i++)
	{
		if(Ping(pingIP))
			return true;
		Sleep(800);
	}
	return false;
}
bool Ping(CString pingIP)
{
	SOCKET socket;
	char Sign[] = {"DrayTek "};
	char rcvbuffer[2000];
	int delay, timeout, packetsize, min, max;
	bool istimeout, istimeoutlasttime;
	ULONG IpAddr;
	CString temp;
	CTime Stime, Etime;
	LARGE_INTEGER TimeTick1,TimeTick3,Frequency;
	int rcvreturn = 0;
	unsigned short SquenceNumber;
	int i=0;

	::QueryPerformanceFrequency(&Frequency);
	
	delay = 50;
	timeout=100;
	max = 0, min = timeout+1;
	packetsize=1472;

	int IcmpSize = packetsize + 8;  // 8 = ICMP HEADER SIZE
	ICMP *IcmpPacket = (ICMP*)new char[IcmpSize];
	socket = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	IcmpPacket->Type = 8;
	IcmpPacket->Code = 0;
	IcmpPacket->CheckSum = 0;
	IcmpPacket->Identifier = NULL;	//-----------?
	int t_timeout = 100;

	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&t_timeout, sizeof(t_timeout));

	for(i=0;i<packetsize;i++)
	{
		*(IcmpPacket->Contain+i) = *(Sign+i%8);
	}
	
	istimeoutlasttime = false;
	
	IpAddr = inet_addr(pingIP);
	if(IpAddr == 0)
		istimeout = false;
	else
	{
		istimeout = false;

		sockaddr_in SockAddr;
		ZeroMemory((void*)&SockAddr, sizeof(SockAddr));
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_port = 0;
		SockAddr.sin_addr.S_un.S_addr = IpAddr;

		SquenceNumber = IcmpPacket->SquenceNumber;
		IcmpPacket->CheckSum = 0;
		IcmpPacket->CheckSum = Checksum((unsigned short*)IcmpPacket, IcmpSize);

		::QueryPerformanceCounter(&TimeTick1);

		int a = sendto(socket, (char*)IcmpPacket, IcmpSize, 0, (sockaddr*)&SockAddr, sizeof(SockAddr));

		int error = WSAGetLastError();
		//TotalPacketSend++;
		ZeroMemory((void*)rcvbuffer, 2000);
		do
		{
			rcvreturn = recvfrom(socket, rcvbuffer,sizeof(rcvbuffer), 0, 0, 0);
			::QueryPerformanceCounter(&TimeTick3);
			if((int)((TimeTick3.QuadPart - TimeTick1.QuadPart)/(Frequency.QuadPart/1000.0))>=timeout)
				break;
		}while(*((unsigned short*)(rcvbuffer+26)) != SquenceNumber );
	}

	closesocket(socket);
	if(rcvreturn==SOCKET_ERROR)
		return false;
	return true;
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