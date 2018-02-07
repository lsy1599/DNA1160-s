#include "StdAfx.h"
#include "FTP_Client.h"

CFTP_Client::CFTP_Client(void)
{
	m_Folder	= "TSD_PR";
}

CFTP_Client::~CFTP_Client(void)
{
}

BOOL CFTP_Client::ConnectFTPServer(CString m_StrFTPServer,CString	m_StrUserName,CString m_StrPassword,CString csFolder) 
{
	m_SecFolder = csFolder;
	m_pIntSession=new CInternetSession(AfxGetAppName(),1,PRE_CONFIG_INTERNET_ACCESS);
	try 
	{ 
		m_pFTPConnection=m_pIntSession->GetFtpConnection(m_StrFTPServer,m_StrUserName,m_StrPassword,21);
	} 
	catch(CInternetException *pEx) 
	{
		TCHAR szError[1024];
		if(pEx->GetErrorMessage(szError,1024))
			AfxMessageBox(szError);
		else  
			AfxMessageBox("異常錯誤");
		pEx->Delete();
		m_pFTPConnection=NULL;
		return FALSE;
	}
	m_pRemoteFinder = new CFtpFileFind(m_pFTPConnection);
	BrowseFolder("",m_pRemoteFinder,&m_ArrayRemoteFiles);
	return TRUE;
}

void CFTP_Client::BrowseFolder(CString strDir,CFileFind* pFinder,CArray<FILEITEM,FILEITEM&>* pFilesArr)
{
	CString cpath;
	FILEITEM fileitem;
	int nIndex = 1;	

	CString szDir = strDir;
	if(strDir == "")
		szDir += "*.*";

	BOOL res = pFinder->FindFile(szDir);
	while(res)
	{
		res = pFinder->FindNextFile();
		if(pFinder->IsDirectory() && !pFinder->IsDots())
		{
			fileitem.nItem = nIndex;
			fileitem.bDir = TRUE;
			fileitem.StrFileName = pFinder->GetFileName();
			pFilesArr->Add(fileitem);
			if(fileitem.StrFileName == m_Folder)
			{ 
				strRemoteFile.Format("%s",fileitem.StrFileName);
				temppath = strRemoteFile + "/";
				BrowseFolder(strRemoteFile,m_pRemoteFinder,&m_ArrayRemoteFiles);
			}
			if(strcmp(fileitem.StrFileName,m_SecFolder) == 0)
			{
				CreateDirectory(m_SecFolder,NULL);

				strRemoteFile.Format("%s/%s",m_Folder,fileitem.StrFileName);
				temppath = strRemoteFile + "/";
				BrowseFolder(strRemoteFile,m_pRemoteFinder,&m_ArrayRemoteFiles);
			}
		}
		else if(!pFinder->IsDirectory() && !pFinder->IsDots())
		{
			fileitem.nItem = nIndex;
			fileitem.bDir = FALSE;
			fileitem.StrFileName = pFinder->GetFileName();
			filepath.Format("%s%s",temppath,fileitem.StrFileName);
			pFilesArr->Add(fileitem);
			cpath = pFinder->GetFilePath();
			if(strstr(cpath,m_Folder))
				DownLoadFile(filepath,fileitem,nIndex);
		}
		nIndex++;
	}
}
void CFTP_Client::DownLoadFile(CString Remote_file,FILEITEM fileItem,int fileindex)
{
	if(m_pFTPConnection==NULL)
	{
		MessageBox(NULL,"程式不支持文件夾下載","ERROR",MB_OK);
		return;
	}
	if (fileindex == NULL)
		AfxMessageBox("沒有要下載的文件");
	else
	{
		if(fileItem.bDir == TRUE)
		{
			AfxMessageBox("文件下載錯誤...!");
		}
		else
		{
			if(strstr(Remote_file,m_SecFolder))
			{	
				strLocalFile.Format("%s\\%s",m_SecFolder,fileItem.StrFileName);
				m_pFTPConnection->GetFile(Remote_file,strLocalFile);
			}
			else
			{
				strLocalFile.Format("TSD\\%s",fileItem.StrFileName);
				m_pFTPConnection->GetFile(Remote_file,strLocalFile);
			}
		}
	}
}