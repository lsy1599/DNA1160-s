#ifdef CYGPIB_EXPORTS
#define CYSFIS_API __declspec(dllexport)
#else
#define CYSFIS_API __declspec(dllimport)
#endif


typedef struct _SFIS_PM
{
	char				czSN[26];
	char				czPN[21];
	char				czMAC[13];
	char				czSSN1[26];
	char				czSSN2[36];     //pSFIS��^��
	char				czFW[13];
	char				czSta[13];
	char				czEC[5];
	char				czEND[4];
	char				czResult[5];
} SFIS_PM;



#pragma comment(lib,"SFIS")
	CYSFIS_API int			InitSFIS(char *errMsg); // return(0=error    1=OK    returnMsg=�����~��^�H���^
	CYSFIS_API int			DetSFISmsg(struct _SFIS_PM* pSFIS,char* recvBuffer); // return (0=error   1=OK   2= SFISTimeOut   3=SFISERROR  4=SFISPASS   recvBuffer=��^�H���^
	CYSFIS_API int			SendMsgSFIS(BOOL nResult,char FW[13],char EC[5],struct _SFIS_PM* pSFIS,char* recvBuffer); //return (0=error  1=ok    EC=error code    nResult=TRUE/FALSE(��Dut���յ��GPASS=TRUE/FAIL=FALSE)  recvBuffer= ��^�H��)
	CYSFIS_API int			SendMsgMES(char SN[26],char MAC[13],char SSN1[26],char OPNum[26],struct _SFIS_PM* pSFIS,char* recvBuffer,int nFlag);
	CYSFIS_API int			FinalCheckLink(char SN[26],char SSN1[26],char MAC[13],char *Buff,int Choice);
	CYSFIS_API bool			UserInput(char User[100],char Passwd[100],char *pBuff);			