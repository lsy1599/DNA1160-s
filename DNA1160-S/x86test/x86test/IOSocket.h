#ifndef IOSOCKET_H
#define IOSOCKET_H

#include "stdafx.h"
#include "winsock2.h"
//#pragma once
//#include "atlstr.h"
//#include "afx.h"
//Other define
#define HTTP_PORT			(80)
#define HTTP_PORT_SPC		(62207)
#define LISTEN_PORT			(4120)
#define LISTEN_FWPORT		(9998)
#define LISTEN_FWRSTPORT	(9999)
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
extern SOCKET listenSocket2;

SOCKET	ConnectRemote(CString remoteIP, unsigned short port,int protocol, int timeout); // Protocol IPPROTO_TCP(6) IPPROTO_UDP(17)
void	DisconnectRemote(SOCKET remoteSocket);
SOCKET  BindListenUDP(unsigned short port);
SOCKET  BindSockt(char * bindIP, unsigned short port,int protocal );
bool	Ping(CString pingIP);
bool	Ping(CString pingIP,int timeOut); //ms

#endif