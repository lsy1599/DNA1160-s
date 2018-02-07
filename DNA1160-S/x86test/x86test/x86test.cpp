// x86test.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "x86test.h"
#include "AtsComm\AtsComm.h"
#include "Comm.h"
#include "IoSocket.h"
#include "SocketDx.h"
#include "Afxsock.h"
#include "MfgErrorCode.h"
#include "ComCtrl.h"
#include "Tester.h"
#include "InitDlg.h"
#include "CPromptDlg.h"
#include "Cmodeldlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ROUTER_TEST_INI TestIni;
extern SFIS_PM *sfisData;
extern CString m_errCode;
extern HWND m_UINotifyWnd;
extern int TestResul;
CWnd *pParent;
CString csMac, csSN;
CString csModel;
HANDLE hCom;
InitInfo gInitInfo;
CString csDllVersion;
int nComPort = 0;

char Dataflg[TESTITEMNUM];  //tree MD
//char Dataflg[20];  //tree MD

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// Cx86testApp

BEGIN_MESSAGE_MAP(Cx86testApp, CWinApp)
END_MESSAGE_MAP()
// Cx86testApp 构造
Cx86testApp::Cx86testApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}
// 唯一的一个 Cx86testApp 对象
Cx86testApp theApp;
// Cx86testApp 初始化
BOOL Cx86testApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

TestList TestItemList[MAXTESTCOUNT] = {
									  {"Buzzer",Buzzer,BUZZER_TEST_FAIL,TRUE,2},
									  {"FAN",FAN,FAN_TEST_FAIL,TRUE,2},
									  {"HWMonitor",HWMonitor,HWMONITOR_TEST_FAIL,TRUE,2},
									  {"SetRTC",SetRTC,RTC_TEST_FAIL,TRUE,2},
									  {"CheckRTC",CheckRTC,CHECK_RTC_FAIL,TRUE,2},
									  {"WriteMAC",WriteMAC,WRITE_MAC_FAIL,TRUE,2},
									  {"Network",Network,ETHERNET_TEST_FAIL,TRUE,2},
									  {"USB",USB,USB_PORT_FAIL,TRUE,2},
									  {"CFCard",CFCard,CFCARD_TEST_FAIL,TRUE,2},
									  {"SATA",SATA,SATA_DISK_FAIL,TRUE,2},
									  {"Memory",Memory,MEM_TEST_FAIL,TRUE,2},
									  {"CheckMAC",CheckMAC,CHECK_MAC_FAIL,TRUE,2},
									  {"VGA",VGA,VGA_DISPLAY_FAIL,TRUE,2},
									  {"LED",LED,LED_FAIL,TRUE,2 },
									  {"NetSpeed",NetSpeed,NET_SPEED_FAIL,TRUE,2 },
									  {"CFDetail",CFDetail,CFCARD_TEST_FAIL,TRUE,2 },
									  {"HDDDetail",HDDDetail ,SATA_DISK_FAIL,TRUE,2},
									  {"ResetButton",ResetButton ,RESET_BUTTON_FAIL,TRUE,2 },
									  {"PowerButton",PowerButton,POWER_BUTTON_FALL,TRUE,2},
									  {"Wifi",Wifi,WIFI_FALL,TRUE,2},
									  {"Wifi5G",Wifi5G,WIFI5G_FALL,TRUE,2 },
									  {"CPUStress",CPUStress,CPUStress_FALL,TRUE,2 },
									  {"LTE_4G",LTE_4G,LTE_4G_FALL,TRUE,2 },
									  {"TPM",TPM,TPM_FALL,TRUE,2 },
									  {"SFP",SFP,SFP_FALL,TRUE,2 },
									  {"HDMI",HDMI,HDMI_FALL,TRUE,2 },
									  {"MicroUSB",MicroUSB,MICRO_USB_FALL,TRUE,2 },
									  {"WriteI211",WriteI211,TEMP_TEST_FAIL,TRUE,2 },
									  {"PCIE",PCIE,PCIE_FAIL,TRUE,2 },
									  {"M2_ssd",M2_ssd,SATA_DISK_FAIL,TRUE,2 },
									  {"CheckCPLD",CheckCPLD ,CPLDVER_TEST_FAIL ,TRUE ,2},
									  {"CheckDMI",CheckDMI ,TEMP_TEST_FAIL ,TRUE ,2}

};/*名称   指针     错误码            是否 重复测试次数									                                         测试
									 允许
									 重复
									 测试     */
ROUTE_TestList RouteValidItem;

//IDC_ComportCb
//该函数先读取 AtsCfg.ini 文件获得模式信息（如在本例中为 CE100），再根据模式信息找到相应的 x86test.ini 文件并读取，获得相关信息（就是需要测试的测项名）。
//再把读取到的测项名与上面代码中给出的 TestList TestItemList[MAXTESTCOUNT] 数组中的测项名比较，来填充 ROUTE_TestList RouteValidItem 结构体
//此处的填充结构体操作，若在 x86test.ini 文件中没有给出的测项名，则该测项的相关信息就不填充. 该函数也创建并且显示了选择测项配置的那个对话框

bool TestPreInit(void)
{
	CString csMsg, csAaa;
	char strModel[100];
	char DllVersion[100];
	memset(strModel, 0x0, sizeof(strModel));
	//Cmodeldlg modeldlg;  //tree MD
	//csModel=modeldlg.DisplayDlg();  //tree MD
	char inifile[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, inifile);
	csMsg.Format("%s\\AtsCfg.ini", inifile);
	::GetPrivateProfileString("DutConfig", "ProductName", "", strModel, 100, csMsg);//将 csMsg 指定的配置文件中的相关条目的值放入 strModel ，100为指定的 strModel 的大小
	csModel.Format("%s", strModel);
	::GetPrivateProfileString("DLL", "DllVersion", "", DllVersion, 100, csMsg);//
	csDllVersion.Format("%s", DllVersion);

	if (csModel.IsEmpty())
	{
		::AfxMessageBox("No Model Selected!", MB_OK);
		return false;
	}
	CInitDlg InitDlg(csModel, pParent);//创建对话框对象
	gInitInfo = InitDlg.DisplayItem();//DisplayItem函数调用DoModal()函数来创建显示对话框
	//------------打印机种名------------------------------------------------------
	csAaa.Format("Model:%s", strModel);
	//ShowModel("WWWW");
	ShowUIMessage(csAaa);/////////////////-------
	//------------------------------------------------------------------
	csModel = InitDlg.strModel;
	LoadIni();
	CreateLogTitle();//
	if (!RegisterTestItem())
	{
		return false;
	}

	//hCom = InitCom(gInitInfo.Com, 115200);   //tree MD
	//if (NULL == hCom)
	//{
	//	::AfxMessageBox("Can not open selected comport,please check comport", MB_OK);
	//	return false;
	//}
	return true;
}

int get_com_port_num(char* cPID) {

	CString csMsg;
	nComPort = GetComNum(cPID);
	if (nComPort == -1) {
		//csMsg.Format("Can't find comport!");
		//ShowUIMessage(csMsg);
		return -1;
	}

	hCom = InitCom(nComPort, 115200);
	if (NULL != hCom)
	{
		return nComPort;
	}
	/*csMsg.Format("Can't init selected comport:%d", nComPort);
	ShowUIMessage(csMsg);*/
	return -1;
}

int shell_mode()
{
	CString csTemp, csRecv;
	int nFind = 0;
	DWORD TimeStart = ::GetTickCount();// GetTickCount 函数返回自操作系统启动所经过的毫秒数
	while (::GetTickCount() - TimeStart < 30000)
	{
		if (::GetTickCount() - TimeStart > 2000)//10000&&!nEnter=1
		{
			ComSendCmd(hCom, "\r");
		}
		if (!ComFind(hCom, csRecv, "$"))//找提示符
		{
			nFind = 1;
			break;
		}
	}
	if (!nFind)
	{
		return -1; //在正常启动时间内没找到$
	}
	return 0;//在正常启动时间内找到$
}

bool DutInit()//读取用户输入的 SN 和 MAC
{
	pParent = CWnd::FromHandle(m_UINotifyWnd);//
	CString csMsg, csRecv;
	DWORD TimeStart;
	char cPID_232_RJ45[16];
	sprintf(cPID_232_RJ45, "*%s*", TestIni.strRS232RJ45);
	char cPID_USB_RJ45[16];
	sprintf(cPID_USB_RJ45, "*%s*", TestIni.strUSB2RJ45PID);
	char cPID_MicroUSB[16];
	sprintf(cPID_MicroUSB, "*%s*", TestIni.strMicroUSBPID);

	//////////////////////////////////////////////////////
	//	cPID_232_RJ45
	nComPort = get_com_port_num(cPID_232_RJ45);// PNP0501
	Sleep(500);
	if (nComPort != -1)
	{
		if (!shell_mode())// 0: enter system success
		{
			csMsg.Format("Current selected comport:%d", nComPort);
			ShowUIMessage(csMsg);
			goto there;
		}
	}
	////////////////////////////////////////////////////////////////
	//   cPID_USB_RJ45
	nComPort = get_com_port_num(cPID_USB_RJ45);// PID_2303
	Sleep(500);
	if (nComPort != -1)
	{
		if (!shell_mode())// 0: enter system success
		{
			csMsg.Format("Current selected comport:%d", nComPort);
			ShowUIMessage(csMsg);
			goto there;
		}
	}
	////////////////////////////////////////////////////////////////
	//   cPID_MicroUSB
	nComPort = get_com_port_num(cPID_MicroUSB);// PID_2303
	Sleep(500);
	if (nComPort != -1)
	{
		if (!shell_mode())// 0: enter system success
		{
			csMsg.Format("Current selected comport:%d", nComPort);
			ShowUIMessage(csMsg);
			goto there;
		}
	}
there:
	if (NULL == hCom)
	{
		ShowUIMessage("COM Open Fail!");
		::AfxMessageBox("Can not open selected comport,please check comport", MB_OK);
		return -1;
	}

	csSN.Format("%s", sfisData->czSN);
	csMac.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
	return true;
}

bool StartTestItem()//调用 DutTestEntry() 函数，开始执行具体测项
{
	CString strTmp, csRecv;
	int nFind = 0, i = 0;
	strTmp.Format("Cmd.exe /C arp -d");//删除ARP缓存 的命令
	WinExec(strTmp, SW_HIDE);//向windows命令行输入 arp -d
	if (DutTestEntry())
	{
		Beep(1000, 300);//蜂鸣器发声
		return false;
	}
	Beep(900, 100);
	return true;
}

bool DutTestEnd()//关闭串口，关机
{
	if (NULL != hCom)
	{
		if (1 == TestResul)//若 TestResul=1 则表示 DutTestEntry() 函数测项都执行通过了 
		{
			ComSendCmd(hCom, "poweroff");    //tree MD  发送命令关机//----------------------------------------------------------------------------------------
		}
		Sleep(2000);
		// CCPromptDlg pCF("Test","请拔掉电源线\r\n移除U盘、CF卡、硬盘以及网线等测试设备","",0,BUTTON_YES,pParent);
		// CCPromptDlg pCF("Test", "Please unplug the power cord\r\n unplug U disk  CF card  hard disk and network cables", "", 0, BUTTON_YES, pParent);
		// pCF.DisplayDlg();
		CloseCom(hCom); //关闭串口
		hCom = NULL;
	}
	return true;
}

void TestTerminal()//检查串口是否关闭，若没关闭则关闭
{
	if (NULL != hCom)
	{
		CloseCom(hCom);
		hCom = NULL;
	}
}

void LoadIni()
{
	PSTR IniFileName = "x86test.ini";  //需要将 checkbox.index 文件中的测项排列顺序与 x86test.ini 文件中的测项排列顺序 保持一致
	PSTR pBuf = new char[MAX_PATH];
	PSTR tmp = new char[20];
	CString csTemp, str[10];
	RouteValidItem.TestCount = 0;
	int n = 0;
	for (int i = 0; i < MAXTESTCOUNT; i++)
	{
		sprintf_s(tmp, 20, "TEST%d", i + 1);
		GetIniString(csModel, IniFileName, "TEST_ITEM", tmp, NOITEM, pBuf);//读取 x86test.ini 文件中的相关键值存入 pBuf
		csTemp.Format("%s", pBuf);
		//Dataflg[i]
		//if(strcmp(pBuf,NOITEM))
		if (strcmp(pBuf, NOITEM) && Dataflg[i])           //tree MD
		{
			RouteValidItem.TestItem[RouteValidItem.TestCount++].TestName.Format("%s", pBuf);//将从 x86test.ini 文件读取的测项名称存入 TestName
		}
		//此处在 x86test.ini 文件中每读取一个测项名，RouteValidItem.TestCount 就 +1 ，最后读取到几个测项名，RouteValidItem.TestCount 的值就为几
	}

	GetIniString(csModel, IniFileName, "DutConfig", "SSDModel", "IM2S3134N", pBuf);
	TestIni.SSDModel.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "wifiLEDEnable", "1", pBuf);
	TestIni.wifiLEDEnable = atoi(pBuf);

	//------------------------------------------------------------------------------------------------------------------
	GetIniString(csModel, IniFileName, "DutConfig", "MemSizePT", "8", pBuf);
	TestIni.MemSizePT = atoi(pBuf);
	GetIniString(csModel, IniFileName, "DutConfig", "MemSizeFT", "8", pBuf);
	TestIni.MemSizeFT = atoi(pBuf);
	//------------------------------------------------------------------------------------------------------------------

	GetIniString(csModel, IniFileName, "DutConfig", "RTCErrorRange", "90", pBuf);
	TestIni.RTCErrorRange = atoi(pBuf);

	GetIniString(csModel, IniFileName, "TestConfig", "MicroUSBPID", "PID_6015", pBuf);
	TestIni.strMicroUSBPID.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "TestConfig", "USB2RJ45PID", "PID_2303", pBuf);
	TestIni.strUSB2RJ45PID.Format("%s", pBuf);
	//RS232RJ45=PID_0501
	GetIniString(csModel, IniFileName, "TestConfig", "RS232RJ45", "PNP0501", pBuf);
	TestIni.strRS232RJ45.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "Manufacturer", "Sophos", pBuf);
	TestIni.Manufacturer.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "ProductName", "SG", pBuf);
	TestIni.ProductName.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "Version", "125Wr3", pBuf);
	TestIni.Version.Format("%s", pBuf);

	/*GetIniString(csModel, IniFileName, "DutConfig", "SerialNumber", "TBBH06075341", pBuf);
	TestIni.SerialNumber.Format("%s", pBuf);*/

	GetIniString(csModel, IniFileName, "DutConfig", "CPUType", "C3538", pBuf);
	TestIni.csCPUType.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "BIOSRev", "Z161-006", pBuf);
	TestIni.csBIOSRev.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "CPLDRev", "0x05", pBuf);
	TestIni.csCPLDRev.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "WifiHostIP", "192.168.55.11", pBuf);
	TestIni.csWifiHostIP.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "WifiClientIP", "192.168.55.10", pBuf);
	TestIni.csWifiClientIP.Format("%s", pBuf);
	//-------------------------------------------------------------------------------------------------------------------------
	GetIniString(csModel, IniFileName, "DutConfig", "IMEI", "460020371282595", pBuf);
	TestIni.csIMEI.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "WifiCard1ID", "168c:0033", pBuf);
	TestIni.csWifiCard1ID.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "WifiCard2ID", "168c:0033", pBuf);
	TestIni.csWifiCard2ID.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "LTECardID", "05c6:9215", pBuf);
	TestIni.csLTECardID.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "SSID", "DNA1160", pBuf);
	TestIni.csSSID.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "HostIP", "192.168.55.11", pBuf);
	TestIni.csHostIP.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "ClientIP", "192.168.55.10", pBuf);
	TestIni.csClientIP.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "BufferLengthKB", "1024", pBuf);
	TestIni.nBufferLengthKB = atoi(pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "TransmitSec", "5", pBuf);
	TestIni.nTransmitSec = atoi(pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "PeriodSec", "0.5", pBuf);
	TestIni.nPeriodSec = atof(pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "BandwidthLowLimit", "3", pBuf);
	TestIni.nBandwidthLowLimit = atoi(pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "BandwidthLowLimit5G", "3", pBuf);
	TestIni.nBandwidthLowLimit5G = atoi(pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "BandwidthLowLimitSFP", "3", pBuf);
	TestIni.nBandwidthLowLimitSFP = atoi(pBuf);
	//-------------------------------------------------------------------------------------------------------------------------

	GetIniString(csModel, IniFileName, "DutConfig", "DutIP", "192.168.10.9", pBuf);
	TestIni.csDutIP.Format("%s", pBuf);

	GetIniString(csModel, IniFileName, "DutConfig", "NetIP", "192.168.2.100", pBuf);
	TestIni.csNetIP.Format("%s", pBuf);
	TestIni.nCom = GetIniInt(csModel, IniFileName, "DutConfig", "ComPort", 1);
	TestIni.nRetryTimes = GetIniInt(csModel, IniFileName, "DutConfig", "RetryTime", 3);
	GetIniString(csModel, IniFileName, "DutConfig", "LoginName", "root", pBuf);
	TestIni.csLogin.Format("%s", pBuf);
	GetIniString(csModel, IniFileName, "DutConfig", "Passwd", "admin", pBuf);
	TestIni.csPasswd.Format("%s", pBuf);
	GetIniString(csModel, IniFileName, "DutConfig", "HWVer", "10", pBuf);
	TestIni.csMCUVer.Format("%s", pBuf);
	TestIni.nUSBNum = GetIniInt(csModel, IniFileName, "DutConfig", "USBNum", 2);

	//TestIni.nMemSize = GetIniInt(csModel,IniFileName,"DutConfig","Memtester",1);//--------------------------------------
	GetIniString(csModel, IniFileName, "DutConfig", "Memtester", "1M", pBuf);
	TestIni.nMemSize.Format("%s", pBuf);

	TestIni.nCountDownSec = GetIniInt(csModel, IniFileName, "DutConfig", "CountDownSec", 20);

	TestIni.nNetPort = GetIniInt(csModel, IniFileName, "DutConfig", "PortNum", 8);//由于暂时只有4个网卡的驱动，所以此处暂时先改为 4
	TestIni.nSATANum = GetIniInt(csModel, IniFileName, "DutConfig", "SATANum", 2);

	GetIniString(csModel, IniFileName, "HWSpec", "CPUTempMin", "0.001", pBuf);
	TestIni.hwSpec.CPUTemp.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "CPUTempMax", "65.0", pBuf);
	TestIni.hwSpec.CPUTemp.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "CPUVCoreMin", "0.7", pBuf);
	TestIni.hwSpec.CPUVcore.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "CPUVCoreMax", "1.1", pBuf);
	TestIni.hwSpec.CPUVcore.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "CPUFanRpmMin", "1000.0", pBuf);
	TestIni.hwSpec.FANRpm.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "CPUFanRpmMax", "5000.0", pBuf);
	TestIni.hwSpec.FANRpm.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "SysTempMin", "0.0001", pBuf);
	TestIni.hwSpec.SYSTemp.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "SysTempMax", "45.0", pBuf);
	TestIni.hwSpec.SYSTemp.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "SysTemp2Min", "0.0001", pBuf);
	TestIni.hwSpec.SYSTemp2.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "SysTemp2Max", "45.0", pBuf);
	TestIni.hwSpec.SYSTemp2.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "Vol12Min", "11.0", pBuf);
	TestIni.hwSpec.Vol12.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "Vol12Max", "13.0", pBuf);
	TestIni.hwSpec.Vol12.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "VolDDRMin", "1.3825", pBuf);
	TestIni.hwSpec.VolDDR.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "VolDDRMax", "1.5575", pBuf);
	TestIni.hwSpec.VolDDR.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "Vol3P3Min", "3.135", pBuf);
	TestIni.hwSpec.Vol3P3.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "Vol3P3Max", "3.465", pBuf);
	TestIni.hwSpec.Vol3P3.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "Vol5Min", "4.65", pBuf);
	TestIni.hwSpec.Vol5.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "Vol5Max", "5.35", pBuf);
	TestIni.hwSpec.Vol5.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "VolVSBMin", "3.135", pBuf);
	TestIni.hwSpec.VolVSB.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "VolVSBMax", "3.465", pBuf);
	TestIni.hwSpec.VolVSB.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "SFSMin", "1500", pBuf);
	TestIni.hwSpec.SystemFAN.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "SFSMax", "2500", pBuf);
	TestIni.hwSpec.SystemFAN.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "CFSMin", "1500", pBuf);
	TestIni.hwSpec.CPUFAN.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "CFSMax", "2500", pBuf);
	TestIni.hwSpec.CPUFAN.maxval = atof(pBuf);

	GetIniString(csModel, IniFileName, "HWSpec", "SFFullSpeedMin", "1500", pBuf);
	TestIni.hwSpec.SystemFANFullSpeed.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "SFFullSpeedMax", "2500", pBuf);
	TestIni.hwSpec.SystemFANFullSpeed.maxval = atof(pBuf);

	/*GetIniString(csModel, IniFileName, "HWSpec", "MemSizeMin", "16", pBuf);
	TestIni.hwSpec.MemSize.minval = atof(pBuf);
	GetIniString(csModel, IniFileName, "HWSpec", "MemSizeMax", "16", pBuf);
	TestIni.hwSpec.MemSize.maxval = atof(pBuf);*/

	delete[] pBuf;
	delete[] tmp;
}

void CreateLogTitle()
{
	CString csTemp;
	for (int i = 0; i < RouteValidItem.TestCount; i++)
	{
		csTemp.Format("%s:&Result=0&", RouteValidItem.TestItem[i].TestName);
		CreateLogMessage(csTemp);//在界面上显示字符串
	}
}

bool RegisterTestItem(void)
{
	CString csTemp;
	for (int i = 0; i < RouteValidItem.TestCount; i++)
	{
		for (int j = 0;; j++)
		{
			if (j < MAXTESTCOUNT)
			{
				if (!RouteValidItem.TestItem[i].TestName.Compare(TestItemList[j].TestName))//将从配置文件 x86test.ini 中读取的TestName 与代码中给出的 TestName 比较，若相同，就将代码中给出的函数指针、错误码和重复测试的相关信息都赋值给 RouteValidItem
				{
					RouteValidItem.TestItem[i].TestFunc = TestItemList[j].TestFunc;
					RouteValidItem.TestItem[i].TestError = TestItemList[j].TestError;
					if (TestItemList[j].TestRetry)
					{
						RouteValidItem.TestItem[i].nRetryTimes = TestItemList[j].nRetryTimes;
					}
					else
					{
						RouteValidItem.TestItem[i].nRetryTimes = 1;
					}
					break;
				}
				else
				{
					continue;
				}
			}
			else
			{
				csTemp.Format("%s Item not found!", RouteValidItem.TestItem[i].TestName);
				ShowUIMessage(csTemp);
				return false;
			}
		}
	}
	return true;
}
