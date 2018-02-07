// x86test.cpp : ���� DLL �ĳ�ʼ�����̡�
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
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// Cx86testApp

BEGIN_MESSAGE_MAP(Cx86testApp, CWinApp)
END_MESSAGE_MAP()
// Cx86testApp ����
Cx86testApp::Cx86testApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}
// Ψһ��һ�� Cx86testApp ����
Cx86testApp theApp;
// Cx86testApp ��ʼ��
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

};/*����   ָ��     ������            �Ƿ� �ظ����Դ���									                                         ����
									 ����
									 �ظ�
									 ����     */
ROUTE_TestList RouteValidItem;

//IDC_ComportCb
//�ú����ȶ�ȡ AtsCfg.ini �ļ����ģʽ��Ϣ�����ڱ�����Ϊ CE100�����ٸ���ģʽ��Ϣ�ҵ���Ӧ�� x86test.ini �ļ�����ȡ����������Ϣ��������Ҫ���ԵĲ���������
//�ٰѶ�ȡ���Ĳ���������������и����� TestList TestItemList[MAXTESTCOUNT] �����еĲ������Ƚϣ������ ROUTE_TestList RouteValidItem �ṹ��
//�˴������ṹ����������� x86test.ini �ļ���û�и����Ĳ���������ò���������Ϣ�Ͳ����. �ú���Ҳ����������ʾ��ѡ��������õ��Ǹ��Ի���

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
	::GetPrivateProfileString("DutConfig", "ProductName", "", strModel, 100, csMsg);//�� csMsg ָ���������ļ��е������Ŀ��ֵ���� strModel ��100Ϊָ���� strModel �Ĵ�С
	csModel.Format("%s", strModel);
	::GetPrivateProfileString("DLL", "DllVersion", "", DllVersion, 100, csMsg);//
	csDllVersion.Format("%s", DllVersion);

	if (csModel.IsEmpty())
	{
		::AfxMessageBox("No Model Selected!", MB_OK);
		return false;
	}
	CInitDlg InitDlg(csModel, pParent);//�����Ի������
	gInitInfo = InitDlg.DisplayItem();//DisplayItem��������DoModal()������������ʾ�Ի���
	//------------��ӡ������------------------------------------------------------
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
	DWORD TimeStart = ::GetTickCount();// GetTickCount ���������Բ���ϵͳ�����������ĺ�����
	while (::GetTickCount() - TimeStart < 30000)
	{
		if (::GetTickCount() - TimeStart > 2000)//10000&&!nEnter=1
		{
			ComSendCmd(hCom, "\r");
		}
		if (!ComFind(hCom, csRecv, "$"))//����ʾ��
		{
			nFind = 1;
			break;
		}
	}
	if (!nFind)
	{
		return -1; //����������ʱ����û�ҵ�$
	}
	return 0;//����������ʱ�����ҵ�$
}

bool DutInit()//��ȡ�û������ SN �� MAC
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

bool StartTestItem()//���� DutTestEntry() ��������ʼִ�о������
{
	CString strTmp, csRecv;
	int nFind = 0, i = 0;
	strTmp.Format("Cmd.exe /C arp -d");//ɾ��ARP���� ������
	WinExec(strTmp, SW_HIDE);//��windows���������� arp -d
	if (DutTestEntry())
	{
		Beep(1000, 300);//����������
		return false;
	}
	Beep(900, 100);
	return true;
}

bool DutTestEnd()//�رմ��ڣ��ػ�
{
	if (NULL != hCom)
	{
		if (1 == TestResul)//�� TestResul=1 ���ʾ DutTestEntry() �������ִ��ͨ���� 
		{
			ComSendCmd(hCom, "poweroff");    //tree MD  ��������ػ�//----------------------------------------------------------------------------------------
		}
		Sleep(2000);
		// CCPromptDlg pCF("Test","��ε���Դ��\r\n�Ƴ�U�̡�CF����Ӳ���Լ����ߵȲ����豸","",0,BUTTON_YES,pParent);
		// CCPromptDlg pCF("Test", "Please unplug the power cord\r\n unplug U disk  CF card  hard disk and network cables", "", 0, BUTTON_YES, pParent);
		// pCF.DisplayDlg();
		CloseCom(hCom); //�رմ���
		hCom = NULL;
	}
	return true;
}

void TestTerminal()//��鴮���Ƿ�رգ���û�ر���ر�
{
	if (NULL != hCom)
	{
		CloseCom(hCom);
		hCom = NULL;
	}
}

void LoadIni()
{
	PSTR IniFileName = "x86test.ini";  //��Ҫ�� checkbox.index �ļ��еĲ�������˳���� x86test.ini �ļ��еĲ�������˳�� ����һ��
	PSTR pBuf = new char[MAX_PATH];
	PSTR tmp = new char[20];
	CString csTemp, str[10];
	RouteValidItem.TestCount = 0;
	int n = 0;
	for (int i = 0; i < MAXTESTCOUNT; i++)
	{
		sprintf_s(tmp, 20, "TEST%d", i + 1);
		GetIniString(csModel, IniFileName, "TEST_ITEM", tmp, NOITEM, pBuf);//��ȡ x86test.ini �ļ��е���ؼ�ֵ���� pBuf
		csTemp.Format("%s", pBuf);
		//Dataflg[i]
		//if(strcmp(pBuf,NOITEM))
		if (strcmp(pBuf, NOITEM) && Dataflg[i])           //tree MD
		{
			RouteValidItem.TestItem[RouteValidItem.TestCount++].TestName.Format("%s", pBuf);//���� x86test.ini �ļ���ȡ�Ĳ������ƴ��� TestName
		}
		//�˴��� x86test.ini �ļ���ÿ��ȡһ����������RouteValidItem.TestCount �� +1 ������ȡ��������������RouteValidItem.TestCount ��ֵ��Ϊ��
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

	TestIni.nNetPort = GetIniInt(csModel, IniFileName, "DutConfig", "PortNum", 8);//������ʱֻ��4�����������������Դ˴���ʱ�ȸ�Ϊ 4
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
		CreateLogMessage(csTemp);//�ڽ�������ʾ�ַ���
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
				if (!RouteValidItem.TestItem[i].TestName.Compare(TestItemList[j].TestName))//���������ļ� x86test.ini �ж�ȡ��TestName ������и����� TestName �Ƚϣ�����ͬ���ͽ������и����ĺ���ָ�롢��������ظ����Ե������Ϣ����ֵ�� RouteValidItem
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
