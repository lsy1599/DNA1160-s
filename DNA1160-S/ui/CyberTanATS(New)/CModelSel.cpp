// CModelSel.cpp : 实现文件
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "CModelSel.h"
#include "afxdialogex.h"

#include <io.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
//tree ADD END
extern CString modelname;
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

// CCModelSel 对话框

IMPLEMENT_DYNAMIC(CCModelSel, CDialogEx)

CCModelSel::CCModelSel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_Model_Select, pParent)
{

}

CCModelSel::~CCModelSel()
{
}

void CCModelSel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_StaModel, m_StaModel);
	DDX_Control(pDX, IDC_ModelComb, m_ModelComb);
}


BEGIN_MESSAGE_MAP(CCModelSel, CDialogEx)
	ON_CBN_SELCHANGE(IDC_ModelComb, &CCModelSel::OnCbnSelchangeModelcomb)
	ON_BN_CLICKED(IDC_BtnOk, &CCModelSel::OnBnClickedBtnok)
END_MESSAGE_MAP()


// CCModelSel 消息处理程序


BOOL CCModelSel::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_Font.CreatePointFont(100, "Arial");
	m_StaModel.SetFont(&m_Font, false);

	CString csTemp, csPath;
	char PATH[MAX_PATH];

	//tree ADD
	string filePath = "D:";
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

		((CComboBox*)GetDlgItem(IDC_ModelComb))->AddString(cstr);
	}

	((CComboBox*)GetDlgItem(IDC_ModelComb))->SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CCModelSel::OnCbnSelchangeModelcomb()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ModelComb.GetLBText(m_ModelComb.GetCurSel(), m_csModel);
	
}
CString CCModelSel::DisplayDlg()
{
	DoModal();
	return m_csModel;
}


void CCModelSel::OnBnClickedBtnok()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ModelComb.GetLBText(m_ModelComb.GetCurSel(), m_csModel);
	modelname = m_csModel;
	OnOK();
}
