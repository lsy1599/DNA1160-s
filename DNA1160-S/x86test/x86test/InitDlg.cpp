// InitDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "x86test.h"
#include "InitDlg.h"
#include "afxdialogex.h"
#include "AtsComm\AtsComm.h"
InitInfo tmp_Init = {};

extern CString csModel;//--------------------------------
extern CString station_flag;//----------站位标志位，P表示 PT，T表示 FT-------------

//tree ADD 

//#define TESTITEMNUM 19

const int TestItems[TESTITEMNUM] = {
	IDC_ItemCk1, IDC_ItemCk2, IDC_ItemCk3,
	IDC_ItemCk4, IDC_ItemCk5, IDC_ItemCk6,
	IDC_ItemCk7, IDC_ItemCk8, IDC_ItemCk9,
	IDC_ItemCk10, IDC_ItemCk11, IDC_ItemCk12,
	IDC_ItemCk13, IDC_ItemCk14, IDC_ItemCk15,
	IDC_ItemCk16,IDC_ItemCk17,IDC_ItemCk18,IDC_ItemCk19,IDC_ItemCk20,IDC_ItemCk21,
	IDC_ItemCk22,IDC_ItemCk23,IDC_ItemCk24,IDC_ItemCk25
};

static int CheckState_PT1[TESTITEMNUM];//这些数据来自于 checkbox.index 文件
static int CheckState_PT2[TESTITEMNUM];
static int CheckState_PT[TESTITEMNUM];
static int CheckState_FT[TESTITEMNUM];

static int CheckState[TESTITEMNUM] = {0};

//tree ADD END

extern char Dataflg[TESTITEMNUM];    //tree MD

// CInitDlg 对话框

IMPLEMENT_DYNAMIC(CInitDlg, CDialogEx)

CInitDlg::CInitDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInitDlg::IDD, pParent)
	,strModel(_T(""))
{

}

CInitDlg::CInitDlg(CString csmodel, CWnd* pParent /*=NULL*/)
	: CDialogEx(CInitDlg::IDD, pParent)
	,strModel(csmodel)
{

}


CInitDlg::~CInitDlg()
{
}

void CInitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInitDlg, CDialogEx)
	ON_BN_CLICKED(IDC_Model1, &CInitDlg::OnBnClickedMode12)
	ON_BN_CLICKED(IDC_Model2, &CInitDlg::OnBnClickedModel24)
	ON_BN_CLICKED(IDC_Model3, &CInitDlg::OnBnClickedModel48)
	ON_BN_CLICKED(IDC_StationPT, &CInitDlg::OnBnClickedStationpt)
	ON_BN_CLICKED(IDC_StationFT, &CInitDlg::OnBnClickedStationft)
	ON_BN_CLICKED(IDC_StationTemp, &CInitDlg::OnBnClickedStationtemp)
	ON_BN_CLICKED(IDC_BtnLogPath, &CInitDlg::OnBnClickedBtnlogpath)
	ON_BN_CLICKED(IDC_ItemCk1, &CInitDlg::OnBnClickedItemck1)
	ON_BN_CLICKED(IDC_ItemCk2, &CInitDlg::OnBnClickedItemck2)
	ON_BN_CLICKED(IDC_ItemCk3, &CInitDlg::OnBnClickedItemck3)
	ON_BN_CLICKED(IDC_ItemCk4, &CInitDlg::OnBnClickedItemck4)
	ON_BN_CLICKED(IDC_ItemCk5, &CInitDlg::OnBnClickedItemck5)
	ON_BN_CLICKED(IDC_ItemCk6, &CInitDlg::OnBnClickedItemck6)
	ON_BN_CLICKED(IDC_ItemCk7, &CInitDlg::OnBnClickedItemck7)
	ON_BN_CLICKED(IDC_ItemCk8, &CInitDlg::OnBnClickedItemck8)
	ON_BN_CLICKED(IDC_ItemCk9, &CInitDlg::OnBnClickedItemck9)
	ON_BN_CLICKED(IDC_ItemCk10, &CInitDlg::OnBnClickedItemck10)
	ON_BN_CLICKED(IDC_ItemCk11, &CInitDlg::OnBnClickedItemck11)
	ON_BN_CLICKED(IDC_ItemCk12, &CInitDlg::OnBnClickedItemck12)
	ON_BN_CLICKED(IDC_ItemCk13, &CInitDlg::OnBnClickedItemck13)
	ON_BN_CLICKED(IDC_ItemCk14, &CInitDlg::OnBnClickedItemck14)
	ON_BN_CLICKED(IDC_ItemCk15, &CInitDlg::OnBnClickedItemck15)
	ON_BN_CLICKED(IDC_ItemCk16, &CInitDlg::OnBnClickedItemck16)
	ON_BN_CLICKED(IDC_ItemCk17, &CInitDlg::OnBnClickedItemck17)
	ON_BN_CLICKED(IDC_ItemCk18, &CInitDlg::OnBnClickedItemck18)
	ON_BN_CLICKED(IDC_BTNOK, &CInitDlg::OnBnClickedBtnok)
	ON_BN_CLICKED(IDC_ItemCk19, &CInitDlg::OnBnClickedItemck19)
	ON_BN_CLICKED(IDC_ItemCk20, &CInitDlg::OnBnClickedItemck20)
	ON_BN_CLICKED(IDC_ItemCk21, &CInitDlg::OnBnClickedItemck21)
	ON_BN_CLICKED(IDC_ItemCk22, &CInitDlg::OnBnClickedItemck22)
	ON_BN_CLICKED(IDC_ItemCk23, &CInitDlg::OnBnClickedItemck23)
	ON_BN_CLICKED(IDC_ItemCk24, &CInitDlg::OnBnClickedItemck24)
	ON_BN_CLICKED(IDC_ItemCk25, &CInitDlg::OnBnClickedItemck25)
	ON_BN_CLICKED(IDC_ItemCk26, &CInitDlg::OnBnClickedItemck26)
	ON_BN_CLICKED(IDC_ItemCk27, &CInitDlg::OnBnClickedItemck27)
	ON_BN_CLICKED(IDC_ItemCk28, &CInitDlg::OnBnClickedItemck28)
	ON_BN_CLICKED(IDC_ItemCk29, &CInitDlg::OnBnClickedItemck29)
	ON_BN_CLICKED(IDC_ItemCk30, &CInitDlg::OnBnClickedItemck30)
//	ON_CBN_SELCHANGE(IDC_ComportCb2, &CInitDlg::OnCbnSelchangeComportcb2)
	ON_CBN_SELCHANGE(IDC_ComportCb, &CInitDlg::OnCbnSelchangeComportcb)
	ON_EN_CHANGE(IDC_LogPathEdit, &CInitDlg::OnEnChangeLogpathedit)
	ON_BN_CLICKED(IDC_StaCom, &CInitDlg::OnBnClickedStacom)
END_MESSAGE_MAP()

//tree ADD
int ReadCheckboxCsvData(CString strModel) //读取 checkbox.index 文件  ,,需要将 checkbox.index 文件中的测项排列顺序与 x86test.ini 文件中的测项排列顺序 保持一致
{
	FILE *fp;
	char sBuffer[32], strBuf[MAX_PATH], iniFileName[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, strBuf);
	sprintf_s(iniFileName, "%s\\%s\\%s\\checkbox.index", strBuf,"Models",strModel);
	fp = fopen(iniFileName, "r");
	fseek(fp, 0L, SEEK_SET);
	for (int i = 0; i < TESTITEMNUM; i++)
	{
		fscanf(fp, "%d,", &CheckState_PT1[i]);
		fscanf(fp, "%d,", &CheckState_PT2[i]);
		fscanf(fp, "%d,", &CheckState_PT[i]);
		fscanf(fp, "%d,", &CheckState_FT[i]);
		if (fscanf(fp, "%s\n", &sBuffer) == EOF)//测项名称
		{
			break;
		}
	}
	fclose(fp);
	return 0;
}

//tree ADD END

// CInitDlg 消息处理程序

BOOL CInitDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Init Comport Combox
	CString csTemp,str,strTemp,csPath;
	int nPos=0,num=0;
	char PATH[MAX_PATH],buf[100];

	// 1/17/2018 disable combox for selecting COM port automatically
	((CComboBox*)GetDlgItem(IDC_ComportCb))->EnableWindow(FALSE);

	/*for(int i=1;i<20;i++)
	{
		if(i<10)
			csTemp.Format("COM0%d",i);
		else
			csTemp.Format("COM%d",i);
		((CComboBox*)GetDlgItem(IDC_ComportCb))->AddString(csTemp);
	}*/

//tree MD
//	AfxMessageBox("tree");
//	AfxMessageBox(strModel);
	ReadCheckboxCsvData(strModel); //选择读取 strModel 代表的模式下的 checkbox.index 文件

	//---------------------------------------------------------------------------------------------------
	for (int i = 0;i < TESTITEMNUM;i++)
	{
		m_PTItemCheck[0][i] = CheckState_PT[i];
	}
	for (int i = 0;i < TESTITEMNUM;i++)
	{
		m_FTItemCheck[0][i] = CheckState_FT[i];
	}
	//---------------------------------------------------------------------------------------------------

	char key[20], strBuf[MAX_PATH], iniFileName[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, strBuf);
	//sprintf_s(iniFileName, "%s\\x86test.ini", strBuf);
	
	sprintf_s(iniFileName, "%s\\%s\\%s\\x86test.ini", strBuf, "Models",strModel);

	
	for (int i = 1; i <= TESTITEMNUM; i++)
	{
		sprintf_s(key, "TEST%d", i);
		GetPrivateProfileString("TEST_ITEM", key, "", strBuf, sizeof(strBuf), iniFileName);//在配置文件 x86test.ini 中读取需要测试的项
		if (strlen(strBuf) == 0)
			continue;

		sprintf_s(strBuf, "%s_Test", strBuf);//在此处加上 _Test

		((CButton*)GetDlgItem(TestItems[i - 1]))->SetWindowText(_T(strBuf));//这里在测项复选框旁边显示测项名称,表示要测哪些项，如 LED_Test
	}

	//tree MD END
/*
	//((CButton *)GetDlgItem(IDC_Model2))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_Model2))->ShowWindow(SW_HIDE);
	//((CButton *)GetDlgItem(IDC_Model3))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_Model3))->ShowWindow(SW_HIDE);
	////Set Test Item Name
	//::GetCurrentDirectory(MAX_PATH,PATH);
	//csTemp.Format("%s\\x86test.ini",PATH);
	//csPath.Format("%s",csTemp);

	//for(int i=0;i<TEST_ITEM_NUM;i++)
	//{
	//	str.Format("TEST%d",i+1);
	//	::GetPrivateProfileString("TEST_ITEM",str.GetBuffer(),"",buf,sizeof(buf),csTemp.GetBuffer());

	//	if(strlen(buf)==0)
	//		continue;
	//	str.Format("%s",buf);

	//	((CButton *)GetDlgItem(IDC_ItemCk1+i))->SetWindowTextA(str.GetBuffer());

	//	((CButton *)GetDlgItem(IDC_ItemCk1+i))->SetCheck(0);
	//	for(int j=0;j<TOTAL_MODEL;j++)
	//	{
	//		m_PTItemCheck[j][i]=1;
	//		m_FTItemCheck[j][i]=1;
	//	}
	//}
	//for(int i=0;i<TOTAL_MODEL;i++)
	//{
	//	switch(i)
	//	{
	//	case 0:
	//		strTemp="NSB125CNoneItem";
	//		break;
	//	case 1:
	//		strTemp="NSB125DNoneItem";
	//		break;
	//	case 2:
	//		strTemp="NSB125ENoneItem";
	//		break;
	//	default:
	//		break;
	//	}

	//for(int j=0;j<2;j++)
	//{
	//	
	//	if(j==0)
	//	{
	//		str="ItemNumPT";
	//	}else
	//	{
	//		str="ItemNumFT";
	//	}
	//	memset(buf,0x0,sizeof(buf));
	//::GetPrivateProfileString(strTemp,str,"",buf,sizeof(buf),csPath.GetBuffer());
	//str.Format("%s",buf);

	//if(str.GetLength()<1)
	//	continue;
	//nPos=0;
	//if(-1!=str.Find(","))
	//{
	//	csTemp=str.Tokenize(",",nPos);

	//	while(csTemp!="")
	//	{
	//		csTemp.Trim();
	//		num=atoi(csTemp.GetBuffer());
	//		if(num>TEST_ITEM_NUM)
	//			continue;
	//		if(j==0)
	//			m_PTItemCheck[i][num-1]=0;
	//		else
	//			m_FTItemCheck[i][num-1]=0;
	//
	//		csTemp=str.Tokenize(",",nPos);
	//	}

	//}else
	//{
	//	str.Trim();
	//	num=atoi(str.GetBuffer());
	//	if(j==0)
	//			m_PTItemCheck[i][num-1]=0;
	//	else
	//			m_FTItemCheck[i][num-1]=0;
	//}
	//}
	//}
	//str.ReleaseBuffer();
	//csTemp.ReleaseBuffer();

	//((CButton *)GetDlgItem(IDC_Model1))->SetCheck(0);
	//((CButton *)GetDlgItem(IDC_Model2))->SetCheck(0);
	//((CButton *)GetDlgItem(IDC_Model3))->SetCheck(0);

	//((CButton *)GetDlgItem(IDC_StationPT))->SetCheck(0);
	//((CButton *)GetDlgItem(IDC_StationFT))->SetCheck(0);
	//((CButton *)GetDlgItem(IDC_StationTemp))->SetCheck(0);

	//((CButton *)GetDlgItem(IDC_Model1))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_Model1))->ShowWindow(SW_HIDE);
	//((CButton *)GetDlgItem(IDC_Model2))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_Model2))->ShowWindow(SW_HIDE);
	//((CButton *)GetDlgItem(IDC_Model3))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_Model3))->ShowWindow(SW_HIDE);


	//((CButton *)GetDlgItem(IDC_StationPT))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_StationPT))->ShowWindow(SW_HIDE);
	//((CButton *)GetDlgItem(IDC_StationFT))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_StationFT))->ShowWindow(SW_HIDE);
	//((CButton *)GetDlgItem(IDC_StationTemp))->EnableWindow(FALSE);
	//((CButton *)GetDlgItem(IDC_StationTemp))->ShowWindow(SW_HIDE);*/

	((CComboBox*)GetDlgItem(IDC_ComportCb))->SetCurSel(0);//设置串口列表默认选择第一项
	((CComboBox*)GetDlgItem(IDC_ComportCb2))->SetCurSel(0);//设置串口列表默认选择第一项
													   //Set Log Path folder
	((CEdit *)GetDlgItem(IDC_LogPathEdit))->SetWindowTextA("worklog");

	UpdCheckboxState();//根据在 x86test.ini 文件中读取的各个测项在不同站点的状态（默认选中或未选中）来进行设置

	((CStatic*)GetDlgItem(IDC_Model_hu))->SetWindowTextA(strModel);//---------------

	return TRUE;  // return TRUE unless you set the focus to a control
}
void CInitDlg::CreateFolder(CString str)
{
	CFileFind finder;
	if(!finder.FindFile(str))
	{
		CreateDirectory(str,NULL);
	}
	else
	{
		CString filefullname = str;

		HANDLE hFile1 = CreateFile(filefullname, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		_BY_HANDLE_FILE_INFORMATION fImfor;
		BOOL bf = GetFileInformationByHandle(hFile1, &fImfor);

		if(!bf)
		{
			AfxMessageBox(_T("File Folder Already Exist"));
		}else
		{
			CreateDirectory(str,NULL);
		}
		finder.Close();
	}
}

void CInitDlg::OnBnClickedMode12()
{
	// TODO: 在此添加控件通知处理程序代码

	CString ControlText;//-----控件字符串------
	//((CButton *)GetDlgItem(IDC_Model2))->SetCheck(0);
	//((CButton *)GetDlgItem(IDC_Model3))->SetCheck(0);

	//---------------------------------------------------------------------------------------------------------------------------------------------------
	GetDlgItemText(IDC_Model1, ControlText);
	SetModel(ControlText);
	ReadCheckboxCsvData(strModel); //选择读取 strModel 代表的模式下的 checkbox.index 文件
	for (int i = 0;i < TESTITEMNUM;i++)
	{
		m_PTItemCheck[0][i] = CheckState_PT[i];
	}
	for (int i = 0;i < TESTITEMNUM;i++)
	{
		m_FTItemCheck[0][i] = CheckState_FT[i];
	}
	char key[20], strBuf[MAX_PATH], iniFileName[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, strBuf);
	//sprintf_s(iniFileName, "%s\\x86test.ini", strBuf);

	sprintf_s(iniFileName, "%s\\%s\\%s\\x86test.ini", strBuf, "Models", strModel);


	for (int i = 1; i <= TESTITEMNUM; i++)
	{
		sprintf_s(key, "TEST%d", i);
		GetPrivateProfileString("TEST_ITEM", key, "", strBuf, sizeof(strBuf), iniFileName);//在配置文件 x86test.ini 中读取需要测试的项
		if (strlen(strBuf) == 0)
			continue;

		sprintf_s(strBuf, "%s_Test", strBuf);//在此处加上 _Test

		((CButton*)GetDlgItem(TestItems[i - 1]))->SetWindowText(_T(strBuf));//这里在测项复选框旁边显示测项名称,表示要测哪些项，如 LED_Test
	}
	UpdCheckboxState();

	((CButton *)GetDlgItem(IDC_Model2))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_Model3))->SetCheck(0);
	//---------------------------------------------------------------------------------------------------------------------------------------------------

	if(((CButton *)GetDlgItem(IDC_StationPT))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationpt();
	}
	if(((CButton *)GetDlgItem(IDC_StationFT))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationft();
	}
	if(((CButton *)GetDlgItem(IDC_StationTemp))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationtemp();
	}

}


void CInitDlg::OnBnClickedModel24()
{
	// TODO: 在此添加控件通知处理程序代码

	CString ControlText;//-----控件字符串------

	//((CButton *)GetDlgItem(IDC_Model1))->SetCheck(0);
	//((CButton *)GetDlgItem(IDC_Model3))->SetCheck(0);

	//---------------------------------------------------------------------------------------------------------------------------------------------------
	GetDlgItemText(IDC_Model2, ControlText);
	SetModel(ControlText);
	ReadCheckboxCsvData(strModel); //选择读取 strModel 代表的模式下的 checkbox.index 文件
	for (int i = 0;i < TESTITEMNUM;i++)
	{
		m_PTItemCheck[1][i] = CheckState_PT[i];
	}
	for (int i = 0;i < TESTITEMNUM;i++)
	{
		m_FTItemCheck[1][i] = CheckState_FT[i];
	}
	char key[20], strBuf[MAX_PATH], iniFileName[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, strBuf);
	//sprintf_s(iniFileName, "%s\\x86test.ini", strBuf);

	sprintf_s(iniFileName, "%s\\%s\\%s\\x86test.ini", strBuf, "Models", strModel);


	for (int i = 1; i <= TESTITEMNUM; i++)
	{
		sprintf_s(key, "TEST%d", i);
		GetPrivateProfileString("TEST_ITEM", key, "", strBuf, sizeof(strBuf), iniFileName);//在配置文件 x86test.ini 中读取需要测试的项
		if (strlen(strBuf) == 0)
			continue;

		sprintf_s(strBuf, "%s_Test", strBuf);//在此处加上 _Test

		((CButton*)GetDlgItem(TestItems[i - 1]))->SetWindowText(_T(strBuf));//这里在测项复选框旁边显示测项名称,表示要测哪些项，如 LED_Test
	}
	UpdCheckboxState();

	((CButton *)GetDlgItem(IDC_Model1))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_Model3))->SetCheck(0);
	//---------------------------------------------------------------------------------------------------------------------------------------------------

	if(((CButton *)GetDlgItem(IDC_StationPT))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationpt();
	}
	if(((CButton *)GetDlgItem(IDC_StationFT))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationft();
	}
	if(((CButton *)GetDlgItem(IDC_StationTemp))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationtemp();
	}
}


void CInitDlg::OnBnClickedModel48()
{
	// TODO: 在此添加控件通知处理程序代码

		((CButton *)GetDlgItem(IDC_Model1))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_Model2))->SetCheck(0);

	if(((CButton *)GetDlgItem(IDC_StationPT))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationpt();
	}
	if(((CButton *)GetDlgItem(IDC_StationFT))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationft();
	}
	if(((CButton *)GetDlgItem(IDC_StationTemp))->GetCheck()==BST_CHECKED)
	{
		OnBnClickedStationtemp();
	}
}


void CInitDlg::OnBnClickedStationpt()
{
	// TODO: 在此添加控件通知处理程序代码
	
	
	((CButton *)GetDlgItem(IDC_StationFT))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_StationTemp))->SetCheck(0);

	for(int i=0;i<TEST_ITEM_NUM;i++)
	{
		((CButton *)GetDlgItem(IDC_ItemCk1+i))->SetCheck(1);
	}

	/*if (csModel== "SG125rev3")
	{

	}
	else if (csModel == "SG125Wrev3")
	{

	}
	else if (csModel == "SG135rev3")
	{

	}
	else if (csModel == "SG135Wrev3")
	{

	}
	else if (csModel == "XG125rev3")
	{

	}
	else if (csModel == "XG125Wrev3")
	{

	}
	else if (csModel == "XG135rev3")
	{

	}
	else if (csModel == "XG135Wrev3")
	{

	}*/
	if(((CButton *)GetDlgItem(IDC_Model1))->GetCheck()==BST_CHECKED)
	{
		for(int i=0;i<TEST_ITEM_NUM;i++)
		{
			if(m_PTItemCheck[0][i]==0)
			{
				((CButton *)GetDlgItem(IDC_ItemCk1+i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
			}
		}
	}
	else if(((CButton *)GetDlgItem(IDC_Model2))->GetCheck()==BST_CHECKED)
	{
		for(int i=0;i<TEST_ITEM_NUM;i++)
		{
			if (m_PTItemCheck[1][i] == 0)
			{
				((CButton *)GetDlgItem(IDC_ItemCk1 + i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
			}
		}
	}
	else
	//if(((CButton *)GetDlgItem(IDC_Model48))->GetCheck()==BST_CHECKED)
	{
		for(int i=0;i<TEST_ITEM_NUM;i++)
		{
			if (m_PTItemCheck[2][i] == 0)
			{
				((CButton *)GetDlgItem(IDC_ItemCk1 + i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
			}
		}
	}
	//for(int i=0;i<TEST_ITEM_NUM;i++)
	//	//((CButton *)GetDlgItem(IDC_ItemCk1+i))->EnableWindow(FALSE);
	//	((CButton *)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
}


void CInitDlg::OnBnClickedStationft()
{
	// TODO: 在此添加控件通知处理程序代码
	((CButton *)GetDlgItem(IDC_StationPT))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_StationTemp))->SetCheck(0);
	station_flag = "FT";
	for(int i=0;i<TEST_ITEM_NUM;i++)
	{
		((CButton *)GetDlgItem(IDC_ItemCk1+i))->SetCheck(1);
	}
	if(((CButton *)GetDlgItem(IDC_Model1))->GetCheck()==BST_CHECKED)
	{
		for(int i=0;i<TEST_ITEM_NUM;i++)
		{
			if (m_FTItemCheck[0][i] == 0)
			{
				((CButton *)GetDlgItem(IDC_ItemCk1 + i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
			}
		}
	}
	if(((CButton *)GetDlgItem(IDC_Model2))->GetCheck()==BST_CHECKED)
	{
		for(int i=0;i<TEST_ITEM_NUM;i++)
		{
			if (m_FTItemCheck[1][i] == 0)
			{
				((CButton *)GetDlgItem(IDC_ItemCk1 + i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
			}
		}
	}
	if(((CButton *)GetDlgItem(IDC_Model3))->GetCheck()==BST_CHECKED)
	{
		for(int i=0;i<TEST_ITEM_NUM;i++)
		{
			if (m_FTItemCheck[2][i] == 0)
			{
				((CButton *)GetDlgItem(IDC_ItemCk1 + i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
			}
		}
	}
	//for(int i=0;i<TEST_ITEM_NUM;i++)
	//	//((CButton *)GetDlgItem(IDC_ItemCk1+i))->EnableWindow(FALSE);
	//	((CButton *)GetDlgItem(IDC_ItemCk1 + i))->EnableWindow(TRUE);
}


void CInitDlg::OnBnClickedStationtemp()
{
	// TODO: 在此添加控件通知处理程序代码
	((CButton *)GetDlgItem(IDC_StationFT))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_StationPT))->SetCheck(0);
	for(int i=0;i<TEST_ITEM_NUM;i++)
	{
		((CButton *)GetDlgItem(IDC_ItemCk1+i))->SetCheck(1);
	}
		for(int i=0;i<TEST_ITEM_NUM;i++)
		((CButton *)GetDlgItem(IDC_ItemCk1+i))->EnableWindow(TRUE);
}


void CInitDlg::OnBnClickedBtnlogpath()
{
	// TODO: 在此添加控件通知处理程序代码
	((CButton *)GetDlgItem(IDC_LogPathEdit))->SetWindowTextA("worklog");
}


void CInitDlg::OnBnClickedItemck1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk1))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk1))->SetCheck(0);
		m_TotalItem[0] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk1))->SetCheck(1);
		m_TotalItem[0] = 1;
	}
}


void CInitDlg::OnBnClickedItemck2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk2))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk2))->SetCheck(0);
		m_TotalItem[1] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk2))->SetCheck(1);
		m_TotalItem[1] = 1;
	}
}


void CInitDlg::OnBnClickedItemck3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk3))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk3))->SetCheck(0);
		m_TotalItem[2] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk3))->SetCheck(1);
		m_TotalItem[2] = 1;
	}
}


void CInitDlg::OnBnClickedItemck4()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk4))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk4))->SetCheck(0);
		m_TotalItem[3] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk4))->SetCheck(1);
		m_TotalItem[3] = 1;
	}
}


void CInitDlg::OnBnClickedItemck5()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk5))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk5))->SetCheck(0);
		m_TotalItem[4] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk5))->SetCheck(1);
		m_TotalItem[4] = 1;
	}
}


void CInitDlg::OnBnClickedItemck6()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk6))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk6))->SetCheck(0);
		m_TotalItem[5] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk6))->SetCheck(1);
		m_TotalItem[5] = 1;
	}
}


void CInitDlg::OnBnClickedItemck7()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk7))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk7))->SetCheck(0);
		m_TotalItem[6] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk7))->SetCheck(1);
		m_TotalItem[6] = 1;
	}
}


void CInitDlg::OnBnClickedItemck8()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk8))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk8))->SetCheck(0);
		m_TotalItem[7] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk8))->SetCheck(1);
		m_TotalItem[7] = 1;
	}
}


void CInitDlg::OnBnClickedItemck9()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk9))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk9))->SetCheck(0);
		m_TotalItem[8] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk9))->SetCheck(1);
		m_TotalItem[8] = 1;
	}
}


void CInitDlg::OnBnClickedItemck10()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk10))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk10))->SetCheck(0);
		m_TotalItem[9] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk10))->SetCheck(1);
		m_TotalItem[9] = 1;
	}
}


void CInitDlg::OnBnClickedItemck11()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk11))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk11))->SetCheck(0);
		m_TotalItem[10] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk11))->SetCheck(1);
		m_TotalItem[10] = 1;
	}
}


void CInitDlg::OnBnClickedItemck12()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk12))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk12))->SetCheck(0);
		m_TotalItem[11] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk12))->SetCheck(1);
		m_TotalItem[11] = 1;
	}
}


void CInitDlg::OnBnClickedItemck13()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk13))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk13))->SetCheck(0);
		m_TotalItem[12] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk13))->SetCheck(1);
		m_TotalItem[12] = 1;
	}
}


void CInitDlg::OnBnClickedItemck14()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk14))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk14))->SetCheck(0);
		m_TotalItem[13] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk14))->SetCheck(1);
		m_TotalItem[13] = 1;
	}
}


void CInitDlg::OnBnClickedItemck15()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk15))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk15))->SetCheck(0);
		m_TotalItem[14] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk15))->SetCheck(1);
		m_TotalItem[14] = 1;
	}
}


void CInitDlg::OnBnClickedItemck16()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk16))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk16))->SetCheck(0);
		m_TotalItem[15] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk16))->SetCheck(1);
		m_TotalItem[15] = 1;
	}
}


void CInitDlg::OnBnClickedItemck17()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk17))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk17))->SetCheck(0);
		m_TotalItem[16] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk17))->SetCheck(1);
		m_TotalItem[16] = 1;
	}
}


void CInitDlg::OnBnClickedItemck18()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk18))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk18))->SetCheck(0);
		m_TotalItem[17] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk18))->SetCheck(1);
		m_TotalItem[17] = 1;
	}
}
int CInitDlg::GetCurStation()
{
	if(((CButton *)GetDlgItem(IDC_StationPT))->GetCheck()==BST_CHECKED)
	{
		return PT;
	}
	if(((CButton *)GetDlgItem(IDC_StationFT))->GetCheck()==BST_CHECKED)
	{
		return FT;
	}
	if(((CButton *)GetDlgItem(IDC_StationTemp))->GetCheck()==BST_CHECKED)
	{
		return TEMP;
	}
	return 0;
}
int CInitDlg::GetCurModel()
{
	if(((CButton *)GetDlgItem(IDC_Model1))->GetCheck()==BST_CHECKED)
	{
		return NSB125C;
	}
	if(((CButton *)GetDlgItem(IDC_Model2))->GetCheck()==BST_CHECKED)
	{
		return NSB125D;
	}
	if(((CButton *)GetDlgItem(IDC_Model3))->GetCheck()==BST_CHECKED)
	{
		return NSB125E;
	}
	return 0;
}
InitInfo CInitDlg::DisplayItem()
{	
	DoModal();	
	return m_InitInfo;
}
extern HWND m_UINotifyWnd;
extern CString station_flag;

void Show_PF_or_FT(CString msg) {
	::SendMessage(m_UINotifyWnd, MSG_FT_PT_MESSAGE, (WPARAM)&msg, 0);
}
void CInitDlg::OnBnClickedBtnok()
{
	// TODO: 在此添加控件通知处理程序代码

	Show_PF_or_FT(station_flag);
	CString csTemp;
	((CEdit*)GetDlgItem(IDC_LogPathEdit))->GetWindowTextA(csTemp);

	int strLength = csTemp.GetLength() + 1;
	char *pValue = new char[strLength];
	char *ch = csTemp.GetBuffer(0);
	
	strncpy(pValue, ch, strLength);

	CString csNew;
	CString csSource;
	char delims[]="\\";
	char *result = NULL;
	result = strtok(pValue, delims);
	while(result != NULL)
	{
		csNew.Empty();
		csNew.Format(result);
		csSource += csNew;
		csSource += "\\";
		CreateFolder(csSource);
		result = strtok(NULL,delims);
	}
	strLength = csSource.GetLength() + 1;
	ch = csSource.GetBuffer(0);
	csLogFolder.Format("%s", ch);
	
	char strCurrentDir[LOG_PATH_MAX],strPath[LOG_PATH_MAX];
	::GetCurrentDirectory(LOG_PATH_MAX,strCurrentDir);
	sprintf(strPath,"%s\\%s",strCurrentDir, csLogFolder);

	if(!PathIsDirectory(strPath))
	{
		::CreateDirectory(strPath,NULL);
	}	
	csLogFolder = (LPSTR)(LPCTSTR)strPath;

	int Model=GetCurModel();
	int station=GetCurStation();

	m_InitInfo.Com=((CComboBox*)GetDlgItem(IDC_ComportCb))->GetCurSel()+1;//将选择的串口号存入 m_InitInfo.Com
	
	m_InitInfo.Model=Model;
	m_InitInfo.Station=station;
	memset(m_InitInfo.LogPath,0x0,sizeof(tmp_Init.LogPath));
	strcpy(m_InitInfo.LogPath,csLogFolder.GetBuffer());

	// tree MD

	memset(Dataflg, 0, sizeof(Dataflg));//----------------------------------------------------------------------------------------

	for (int i = 0; i < TESTITEMNUM; i++)//检查测项选择对话框上的各个测项复选框是否被选中，若选中，则将数组 Dataflg 中相应元素置 1，
	{                                    //在 LoadIni 函数中有对 Dataflg 数组中的元素进行检查，若某元素为 1 ，则，相应的测项结构体中的 TestName 元素 将被赋值，则该测项函数将会被执行
		if (FALSE != ((CButton*)GetDlgItem(TestItems[i]))->GetState())
		{
			if (BST_CHECKED == ((CButton*)GetDlgItem(TestItems[i]))->GetCheck())
			{
				Dataflg[i] = 1;
			}
		}
	}

	//tree MD

	/*if(Model==NSB125C)
	{
		if(station==PT)
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_PTItemCheck[0][i];
		else if(station==FT)
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_FTItemCheck[0][i];
		else
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_TotalItem[i];
	}
	if(Model==NSB125D)
			{
			if(station==PT)
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_PTItemCheck[1][i];
		else if(station==FT)
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_FTItemCheck[1][i];
		else
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_TotalItem[i];
	}
	if(Model==NSB125E)
			{
		if(station==PT)
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_PTItemCheck[2][i];
		else if(station==FT)
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_FTItemCheck[2][i];
		else
			for(int i=0;i<TEST_ITEM_NUM;i++)
				m_InitInfo.Item[i]= m_TotalItem[i];
	}*/

	PostMessage(WM_CLOSE);
}


void CInitDlg::OnBnClickedItemck19()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk19))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk19))->SetCheck(0);
		m_TotalItem[18] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk19))->SetCheck(1);
		m_TotalItem[18] = 1;
	}
}


void CInitDlg::OnBnClickedItemck20()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk20))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk20))->SetCheck(0);
		m_TotalItem[19] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk20))->SetCheck(1);
		m_TotalItem[19] = 1;
	}
}


void CInitDlg::OnBnClickedItemck21()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk21))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk21))->SetCheck(0);
		m_TotalItem[20] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk21))->SetCheck(1);
		m_TotalItem[20] = 1;
	}
}


void CInitDlg::OnBnClickedItemck22()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk22))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk22))->SetCheck(0);
		m_TotalItem[21] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk22))->SetCheck(1);
		m_TotalItem[21] = 1;
	}
}


void CInitDlg::OnBnClickedItemck23()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk23))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk23))->SetCheck(0);
		m_TotalItem[22] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk23))->SetCheck(1);
		m_TotalItem[22] = 1;
	}
}


void CInitDlg::OnBnClickedItemck24()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk24))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk24))->SetCheck(0);
		m_TotalItem[23] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk24))->SetCheck(1);
		m_TotalItem[23] = 1;
	}
}


void CInitDlg::OnBnClickedItemck25()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk25))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk25))->SetCheck(0);
		m_TotalItem[24] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk25))->SetCheck(1);
		m_TotalItem[24] = 1;
	}
}


void CInitDlg::OnBnClickedItemck26()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk26))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk26))->SetCheck(0);
		m_TotalItem[25] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk26))->SetCheck(1);
		m_TotalItem[25] = 1;
	}
}


void CInitDlg::OnBnClickedItemck27()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk27))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk27))->SetCheck(0);
		m_TotalItem[26] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk27))->SetCheck(1);
		m_TotalItem[26] = 1;
	}
}


void CInitDlg::OnBnClickedItemck28()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk28))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk28))->SetCheck(0);
		m_TotalItem[27] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk28))->SetCheck(1);
		m_TotalItem[27] = 1;
	}
}


void CInitDlg::OnBnClickedItemck29()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk29))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk29))->SetCheck(0);
		m_TotalItem[28] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk29))->SetCheck(1);
		m_TotalItem[28] = 1;
	}
}


void CInitDlg::OnBnClickedItemck30()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!((CButton *)GetDlgItem(IDC_ItemCk30))->GetCheck())
	{
		((CButton *)GetDlgItem(IDC_ItemCk30))->SetCheck(0);
		m_TotalItem[29] = 0;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_ItemCk30))->SetCheck(1);
		m_TotalItem[29] = 1;
	}
}



//void CInitDlg::OnCbnSelchangeComportcb2()
//{
//	// TODO: 在此添加控件通知处理程序代码
//}


void CInitDlg::OnCbnSelchangeComportcb()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CInitDlg::OnEnChangeLogpathedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CInitDlg::OnBnClickedStacom()
{
	// TODO: 在此添加控件通知处理程序代码
}



//tree ADD 

void CInitDlg::UpdCheckboxState()//根据在 x86test.ini 文件中读取的各个测项在不同站点的状态（默认选中或未选中）来进行设置
{
	int CheckState_Null[TESTITEMNUM];
	memset(CheckState_Null, 0, TESTITEMNUM);

	int * pCheckState_Station= nullptr;
	/*if (0 == TestIni.csStation.Compare("PT1"))
	{
		((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(1);
		pCheckState_Station = CheckState_PT1;

		((CButton*)GetDlgItem(IDC_CHECK_TestFwUpd))->SetCheck(0);
	}
	else if (0 == TestIni.csStation.Compare("PT2"))
	{
		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(1);
		pCheckState_Station = CheckState_PT2;

		((CButton*)GetDlgItem(IDC_CHECK_TestFwUpd))->SetCheck(0);
	}
	else  if (0 == TestIni.csStation.Compare("PT"))
	{
		((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck(1);
		pCheckState_Station = CheckState_PT;

		((CButton*)GetDlgItem(IDC_CHECK_TestFwUpd))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_CHECK_ForOBA))->SetCheck(0);
	}
	else if (0 == TestIni.csStation.Compare("FT"))
	{
		((CButton*)GetDlgItem(IDC_RADIO4))->SetCheck(1);
		pCheckState_Station = CheckState_FT;

		((CButton*)GetDlgItem(IDC_CHECK_TestFwUpd))->SetCheck(0);

		if (TestIni.nForOBA == 1)
		{
			((CButton*)GetDlgItem(IDC_CHECK_ForOBA))->SetCheck(1);
		}
	}
	else
	{
		pCheckState_Station = CheckState_Null;

		((CButton*)GetDlgItem(IDC_CHECK_TestFwUpd))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_CHECK_ForOBA))->SetCheck(0);
	}

	((CButton*)GetDlgItem(IDC_CHECK_TestFwUpd))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_CHECK_ForOBA))->EnableWindow(FALSE);*/
	//应该可以在此处来初始化设置站点 和 模式，用来为初始化时设置默认哪些测项被选中,,,,--------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	if (strModel == "NSK3300_MPCIE_LTE7455NA" || strModel == "NSK3300_MPCIE_S")
	{
		((CButton *)GetDlgItem(IDC_StationFT))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_StationPT))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_StationTemp))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_StationPT))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_StationTemp))->EnableWindow(FALSE);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_StationPT))->SetCheck(1);
	}
	//-----------------------------------------------------------------------------------------------
	((CButton *)GetDlgItem(IDC_Model1))->SetCheck(1);
	if (PT == GetCurStation())
	{
		station_flag = "PT";
		pCheckState_Station = CheckState_PT;
	}
	else if (FT == GetCurStation())
	{
		station_flag = "FT";
		pCheckState_Station = CheckState_FT;
	}
	else
	{
		for (int i = 0;i<TEST_ITEM_NUM;i++)
		{
			((CButton *)GetDlgItem(IDC_ItemCk1 + i))->SetCheck(1);
		}
	}
	//---------------------------================================----------------------------=====================--------------------------------

	//pCheckState_Station = CheckState_PT;

	// ---
	for (int i = 0; i < TESTITEMNUM; i++)
	{
		CheckState[i] = pCheckState_Station[i];

		if (CheckState[i])
		{
			((CButton*)GetDlgItem(TestItems[i]))->SetCheck(1);//设置该控件的状态为被选中
			((CButton*)GetDlgItem(TestItems[i]))->EnableWindow(TRUE);//允许该控件接受点击
		}
		else
		{
			((CButton*)GetDlgItem(TestItems[i]))->SetCheck(0);
			((CButton*)GetDlgItem(TestItems[i]))->EnableWindow(FALSE);
		}
	}


	/*const int Radio[4] = {
		IDC_RADIO1, IDC_RADIO2, IDC_RADIO3, IDC_RADIO4 };
	for (int i = 0; i < 4; i++)
	{
		((CButton*)GetDlgItem(Radio[i]))->EnableWindow(FALSE);
	}*/
}


//tree ADD END