bool Create_Com(CString comPort,unsigned baudRate);
void OnSend(CString sendBuf);
void OnReceive(char *GetMsg);
void OnClose(void);
void GetResult(CString Strbuf);

