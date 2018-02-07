
extern CString HuComName[20];
CString* FindComm();
HANDLE InitCom(int nCom, DWORD nBaudRate=9600);
int ComSendCmd(HANDLE handle, CString csSend);
int ComSendCmdEx(HANDLE handle, CString csSend, CString &csRecv);
int ComGetDRICmdRlt(HANDLE handle, CString csSend, CString csRecv, CString csPrompt="->");
int ComSend(HANDLE handle, CString csSend);
int ComRecv(HANDLE handle, CString &csRecv, bool showMsg = TRUE);
int ComFind(HANDLE handle, CString &csRecv, CString csFind);
int CloseCom(HANDLE handle);
CString ComRecvCmd(HANDLE handle,CString csSend,CString &csRecv,CString csPrompt);
CString ComRecvOnTime(HANDLE handle,CString csSend,CString &csRecv,CString csPrompt,int nTime);
CString ComRecvCmdLong(HANDLE handle, CString csSend, CString &csRecv, CString csPrompt);