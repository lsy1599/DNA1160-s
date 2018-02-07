#ifndef IOSOCKET_H
#define IOSOCKET_H

#include "stdafx.h"
#include "winsock2.h"

//Other define
#define HTTP_PORT			(80)
#define LISTEN_PORT			(4120)	
#define TIME_OUT_SEC		(2)
#define TIME_OUT_USEC		(0)

struct ICMP
{
	unsigned char Type;
	unsigned char Code;
	unsigned short CheckSum;
	unsigned short Identifier;
	unsigned short SquenceNumber;
	unsigned char Contain[1];
};

extern SOCKET listenSocket;

SOCKET	ConnectRemote(CString remoteIP, unsigned short port,int protocol, int timeout); // Protocol IPPROTO_TCP(6) IPPROTO_UDP(17)
void	DisconnectRemote(SOCKET remoteSocket);
SOCKET  BindListenUDP(unsigned short port);
SOCKET  BindSockt(char * bindIP, unsigned short port,int protocal );

#endif