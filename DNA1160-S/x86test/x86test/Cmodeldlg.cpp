// Cmodeldlg.cpp : 实现文件
//

#include "stdafx.h"
#include "x86test.h"
#include "Cmodeldlg.h"
#include "afxdialogex.h"


//tree ADD 
#include <io.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

//tree ADD END

//tree ADD
void GetAllFolders(string path, vector<string>& files)
{
	long hFile = 0;
	//文件信息      
	struct _finddata_t fileinfo;//用来存储文件信息的结构体      
	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) //第一次查找    
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR)) //如果查找到的是文件夹    
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) //进入文件夹查找    
				{
					files.push_back(p.assign(fileinfo.name));
				}
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);//结束查找    
	}

}

//tree ADD END

// Cmodeldlg 对话框

IMPLEMENT_DYNAMIC(Cmodeldlg, CDialogEx)

Cmodeldlg::Cmodeldlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
}


Cmodeldlg::~Cmodeldlg()
{
}

void Cmodeldlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Cmodeldlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &Cmodeldlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_ComportCb, &Cmodeldlg::OnCbnSelchangeComportcb)
END_MESSAGE_MAP()
// Cmodeldlg 消息处理程序
void Cmodeldlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csTemp;

	((CEdit*)GetDlgItem(IDC_ComportCb))->GetWindowTextA(csTemp);  //tree MD

//	AfxMessageBox(csTemp);    //tree MD
	cstr = csTemp;
	CDialogEx::OnOK();
}


void Cmodeldlg::OnCbnSelchangeComportcb()
{
	// TODO: 在此添加控件通知处理程序代码
}

//tree MD
CString Cmodeldlg::DisplayDlg()
{
	DoModal();
	return cstr;
}

BOOL Cmodeldlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString csTemp, csPath;
	char PATH[MAX_PATH];

    //tree ADD
	string filePath = "D:\\bufflo";
	vector<string> files;
	CString cstr;
	string st;

	::GetCurrentDirectory(MAX_PATH, PATH);
	csTemp.Format("%s\\Models", PATH);
	csPath.Format("%s", csTemp);
	filePath = csPath.GetBuffer(0);

	//st = filePath;
	//cstr = st.c_str();
	//AfxMessageBox(cstr);

	//read all folders    
	GetAllFolders(filePath, files);
	int size = files.size();
	for (int i = 0; i<size; i++)
	{
		st = files[i];
		cstr = st.c_str();

		((CComboBox*)GetDlgItem(IDC_ComportCb))->AddString(cstr);
	}

	((CComboBox*)GetDlgItem(IDC_ComportCb))->SetCurSel(0);

	return true;
}

	


//tree MD END