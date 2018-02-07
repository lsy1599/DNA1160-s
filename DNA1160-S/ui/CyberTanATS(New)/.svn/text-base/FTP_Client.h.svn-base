#include "afxinet.h"
#include "afxtempl.h"

typedef struct FileItem
{
	int nItem;	
	BOOL bDir;
	CString StrFileName;
}FILEITEM;

#pragma once

class CFTP_Client
{
public:
	CString		m_Folder;
	CString		m_SecFolder;
	CString		strLocalFile,strRemoteFile,temppath,filepath;
	CInternetSession* m_pIntSession;
	CFtpConnection* m_pFTPConnection;
	CFtpFileFind* m_pRemoteFinder;	
	CFileFind m_LocalFinder;
	CString m_StrLocalParentRoot;
	CString m_StrRemoteParentRoot;
	CArray<FILEITEM,FILEITEM&>m_ArrayLocalFiles;
	CArray<FILEITEM,FILEITEM&>m_ArrayRemoteFiles;
	void BrowseFolder(CString strDir,CFileFind* pFinder,CArray<FILEITEM,FILEITEM&>* pFilesArr);
	void DownLoadFile(CString Remote_file,FILEITEM fileItem,int fileindex);
	BOOL ConnectFTPServer(CString m_StrFTPServer,CString	m_StrUserName,CString m_StrPassword,CString csFolder);
public:
	CFTP_Client(void);
public:
	~CFTP_Client(void);
};
