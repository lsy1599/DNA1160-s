#ifndef CBT_COMM_HEADER
#define CBT_COMM_HEADER

extern CString m_HTTP_Dest_IP;
extern CString m_HTTP_Header;
extern CString m_HTTP_Cmd;

#ifdef _CBT_COMMON_API_
	#define CBT_COMMON_API __declspec(dllexport)
#else
	#define CBT_COMMON_API __declspec(dllimport)
#endif

#pragma comment(lib, "CBT_COMMON.lib")

class CBT_COMMON_API CBT_COMM
{
public:
	//________ ping ________
	BOOL Ping(CString csIP, float fTimeOut = 3000, CString csSourceIP = "any");
	BOOL Ping(char strIP[16], float fTimeOut = 3000, CString csSourceIP = "any");
	//=========================================================================

	//________ MAC function ________
	void MAC_Increase(BYTE *MAC);
	void MAC_Decrease(BYTE *MAC);
	void MAC2Str(BYTE MAC[6], char *str);
	void Str2MAC(BYTE *MAC, char *str);
	//=========================================================================

	//________ string function ________
	void	InsertStr(char *beInsertBuf,int insertLocal,CString strBuf);
	BOOL	CutCString(CString &sourceStr, CString startTargetStr);
	BOOL	GetCString(CString sourceStr, CString startTargetStr, CString startSepStr, CString endSepStr, CString &resultStr);
	CString GetString(CString sourceStr,PSTR targetStr,char startChar,char endChar);	
	void	WINAPI GenerateRandomString(char string[13]);
	void	SetStrtoArryi(char *strBuf,int *array);
	void	SetStrtoArryd(char *strBuf,double *array);
	int		Get_DUT_Mac(CString szSrcIP,CString szDstIP,char *MAC);
	//=========================================================================

	//________ INI function ________
	void	GetIniString(char * iniFileName, char * iniSection, char * iniKey, char * defVal, CString &strBack);
	void	GetIniString(char * iniFileName, char * iniSection, char * iniKey, char * defVal, char * strBuf);
	int		GetIniInt(PSTR iniFileName, PSTR iniSection, PSTR iniKey, int defVal);
	//=========================================================================
};

class CBT_COMMON_API CBT_SOCK
{	
public:
	//_______________Socket ¾Þ§@_______________
	int		ReceiveSocket(SOCKET sd, char *buffer, int nLength, struct sockaddr_in &serv_addr, int nFlags = 0);
	int		OpenSocket(SOCKET &sd, int type);
	void	CloseSocket(SOCKET sd);
	int		SetSocketOption(SOCKET sd, int option, int optval);
	int		BindSocket(SOCKET sd, struct sockaddr_in &serv_addr, LPCTSTR szHost, int nDestPort, int nSrcPort);
	int		Host2Addr(CString strHost, UINT uPort, struct sockaddr_in* pAddr);
	int		SendSocket(SOCKET sd, LPCTSTR buffer, int nLength, struct sockaddr_in &serv_addr);	
	int		WaitSocket(SOCKET sd, DWORD delay);	
	int		SetBlocking(SOCKET sd, BOOL bBlock);
	int		DisconnectRemote(SOCKET remoteSocket);
	void	HexIP2String(unsigned __int8* szHex, LPSTR szIP);
	SOCKET	ConnectRemote(CString remoteIP, unsigned short port,int protocol,int nettimeout);
	SOCKET	BindListenUDP(unsigned short port);
	SOCKET	BindSockt(char * bindIP, unsigned short port,int protocal );	
	//=========================================================================

	int		TFTP_upload_file(CString csIP, CString csFileName, CString csFileWholePath, CString csPassword);
	CString Send_TCP_Packet(int rplyTimeOut, CString cmd, int port, CString destIP);
	LPCSTR	Send_TCP_Packet(int rplyTimeOut, LPCSTR cmd, int port, LPCSTR destIP);
	void	Init_HTTP_Packet(CString csHTTP_Header, CString DestIP);
	//void	Init_HTTP_Packet(char *sHTTP_Header);
	CString	Send_HTTP_Packet(CString cmd,UINT timeout);
	//LPCSTR		Send_HTTP_Packet(UINT timeout);
};


#endif