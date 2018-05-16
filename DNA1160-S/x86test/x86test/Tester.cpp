#include "stdafx.h"
#include "Tester.h"
#include "Ping.h"
#include "AtsComm\AtsComm.h"
#include "IOSocket.h"
#include "SocketDx.h"
#include "Comm.h"
#include "MfgErrorCode.h"
#include "ComCtrl.h"
#include "winbase.h"
#include <time.h>
#include "x86test.h"
#include "afx.h"
#include "Shlwapi.h"
#include "ShellApi.h"
#include "InitDlg.h"
#include "CPromptDlg.h"
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <iostream>
#include <cmath>

using namespace std;

CString station_flag = "PT";//----------站位标志位，0表示 PT，1表示 FT-------------
extern CString csDllVersion;
extern CWnd* pParent;
extern CString m_errCode;
extern ROUTER_TEST_INI TestIni;
extern SFIS_PM *sfisData;
extern HWND m_UINotifyWnd;
extern CString csMac, csSN;
extern ROUTE_TestList RouteValidItem;
extern HANDLE hCom;
extern InitInfo gInitInfo;
int bMode;
//tree MD
extern CString csModel;
//tree MD END
int TestResul = 0;
extern int nComPort;

int DutTestEntry()
{
	CString csMsg, csTemp, strTime;
	SYSTEMTIME sTime, sEndTime;
	char PATH[MAX_PATH];
	int nSuccess = 0;
	bMode = 0;

	//tree MD
	int flag = 0;
	CString csResult, csTime, csPath;
	char writeBuffer[4096];
	//tree MD END
	ShowUIActMessage("", 0xFFFFFF);
#if 0
	for (int i = 0;; i++)
	{
		if (PingMe(TestIni.csDutIP, 1000))
		{
			ShowUIMessage("Dut is Up");
			break;
		}
		else
		{
			Sleep(500);
		}
		if (i % 10 == 0)
		{
			csTemp.Format("Ping DutIP %s.......", TestIni.csDutIP);
			ShowUIMessage(csTemp);
		}
		if (i > 500)
		{
			ShowUIMessage("Ping Dut Debug Port Fail\r\nPlease Check board IP");
			return -1;
		}
	}
#endif
	ShowUIActMessage("Please confirm the dut powered on", 0xFF0000);
	//tree MD
	memset(writeBuffer, 0, sizeof(writeBuffer));
	//	sprintf(writeBuffer, "<<%s %s Test>>\r\n", TestIni.csDutModel, TestIni.csStation);
	GetLocalTime(&sTime);//获取PC的日期时间
	csTime.Format("%04d-%02d-%02d %02d:%02d:%02d", \
		sTime.wYear, sTime.wMonth, sTime.wDay, \
		sTime.wHour, sTime.wMinute, sTime.wSecond);
	sprintf(writeBuffer, "%sTest start at %s \r\n", writeBuffer, csTime);
	sprintf(writeBuffer, "%s=================================\r\n", writeBuffer);//将数据写入 writeBuffer
	//tree MD END
	for (int i = 0; i < RouteValidItem.TestCount; i++)
	{
		SetDutItemStatusTesting(RouteValidItem.TestItem[i].TestName);//将测试项的状态变为正在测试，就是将UI上的测试项名称变成黄绿闪烁
		if (!bMode)
		{
			for (int nEnter = 0; nEnter < 3; nEnter++)
			{
				if (!ShellMode())
				{
					bMode = 1;
					nSuccess = 1;
					break;
				}
			}
		}
		if (!nSuccess)
		{
			ShowUIMessage("Enter into board linux system mode fail");
			return -1;
		}

		//-----------------------------------------------------------------------------------------------------------------
		csTemp.Format("DllVersion:%s", csDllVersion);//打印 版本
		ShowUIMessage(csTemp);
		csTemp.Empty();
		//-----------------------------------------------------------------------------------------------------------------

		for (int j = 0;; j++)
		{
			if (j < RouteValidItem.TestItem[i].nRetryTimes)//判断测项测试次数是否小于设定的重复测试次数
			{
				m_errCode = "";
				GetLocalTime(&sTime);
				csTemp.Format("Date:%04d-%02d-%02d %02d:%02d:%02d", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
				ShowUIMessage(csTemp);
				csTemp.Format("Model:%s", csModel);//打印 机种.....
				ShowUIMessage(csTemp);
				csTemp.Format("=====TEST:%s=====", RouteValidItem.TestItem[i].TestName);
				ShowUIMessage(csTemp);
				if (!RouteValidItem.TestItem[i].TestFunc())//函数指针调用测项函数
				{
					csMsg.Format("RESULT: %s PASS", RouteValidItem.TestItem[i].TestName);
					ShowUIMessage(csMsg);
					csMsg.Format("%s:&Result=PASS&", RouteValidItem.TestItem[i].TestName);
					SendLogMessage(csMsg);
					SetDutItemStatusPass(RouteValidItem.TestItem[i].TestName);
					GetLocalTime(&sEndTime);

					csTemp.Format("Date:%04d-%02d-%02d %02d:%02d:%02d", sEndTime.wYear, sEndTime.wMonth, sEndTime.wDay, sEndTime.wHour, sEndTime.wMinute, sEndTime.wSecond);
					ShowUIMessage(csTemp);

					//tree MD
					csTemp.Format("%-24sPASS", RouteValidItem.TestItem[i].TestName);
					sprintf(writeBuffer, "%s%s\r\n", writeBuffer, csTemp);
					//tree MD END
					break;
				}
				else
				{
					continue;
				}
			}
			else
			{
				csMsg.Format("RESULT: %s FAIL", RouteValidItem.TestItem[i].TestName);
				ShowUIMessage(csMsg);
				csMsg.Format("%s:&Result=FAIL&", RouteValidItem.TestItem[i].TestName);
				SendLogMessage(csMsg);
				SetDutItemStatusFail(RouteValidItem.TestItem[i].TestName);
				GetLocalTime(&sTime);
				csTemp.Format("Date:%04d-%02d-%02d %02d:%02d:%02d", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
				ShowUIMessage(csTemp);
				if (m_errCode.IsEmpty())
				{
					m_errCode = RouteValidItem.TestItem[i].TestError;
				}

				//tree MD
				csTemp.Format("%-24sFAIL", RouteValidItem.TestItem[i].TestName);
				sprintf(writeBuffer, "%s%s\r\n", writeBuffer, csTemp);
				//tree MD END

				//tree MD
				sprintf(writeBuffer, "%s=================================\r\n", writeBuffer);
				sprintf(writeBuffer, "%sRESULT:%21s\r\n", writeBuffer, csResult);
				GetLocalTime(&sTime);
				csTime.Format("%04d-%02d-%02d %02d:%02d:%02d", \
					sTime.wYear, sTime.wMonth, sTime.wDay, \
					sTime.wHour, sTime.wMinute, sTime.wSecond);
				sprintf(writeBuffer, "%sTest end at %s \r\n", writeBuffer, csTime);

				GetLocalTime(&sTime);
				csTime.Format("%04d%02d%02d_%02d%02d%02d", \
					sTime.wYear, sTime.wMonth, sTime.wDay, \
					sTime.wHour, sTime.wMinute, sTime.wSecond);

				::GetCurrentDirectory(MAX_PATH, PATH);//GetCurrentDirectory 函数获取当前进程的当前目录。
				csPath.Format("%s\\Models\\%s\\%s", PATH, csModel, "worklog");
				::CreateDirectory(csPath, NULL); //创建文件夹

				csResult.Format("FAIL");
				csTemp.Format("%s\\%s_%s_%s.log", csPath, csSN, csResult, csTime);

				CFile LogFile(_T(csTemp), CFile::modeCreate | CFile::modeWrite);
				LogFile.Write(writeBuffer, strlen(writeBuffer));//将 writeBuffer 中的内容写入日志文件
				LogFile.Close();
				//tree MD END
				return -(i + 1);
			}
		}
	}
	//tree MD
	sprintf(writeBuffer, "%s=================================\r\n", writeBuffer);
	sprintf(writeBuffer, "%sRESULT:%21s\r\n", writeBuffer, csResult);
	GetLocalTime(&sTime);
	csTime.Format("%04d-%02d-%02d %02d:%02d:%02d", \
		sTime.wYear, sTime.wMonth, sTime.wDay, \
		sTime.wHour, sTime.wMinute, sTime.wSecond);
	sprintf(writeBuffer, "%sTest end at %s \r\n", writeBuffer, csTime);
	GetLocalTime(&sTime);
	csTime.Format("%04d%02d%02d_%02d%02d%02d", \
		sTime.wYear, sTime.wMonth, sTime.wDay, \
		sTime.wHour, sTime.wMinute, sTime.wSecond);
	::GetCurrentDirectory(MAX_PATH, PATH);
	csPath.Format("%s\\Models\\%s\\%s", PATH, csModel, "worklog");
	::CreateDirectory(csPath, NULL);
	csResult.Format("PASS");
	csTemp.Format("%s\\%s_%s_%s.log", csPath, csSN, csResult, csTime);
	TestResul = 1;
	CFile LogFile(_T(csTemp), CFile::modeCreate | CFile::modeWrite);
	LogFile.Write(writeBuffer, strlen(writeBuffer));
	LogFile.Close();
	return 0;
}

static DWORD WINAPI ButtonThread(int nButton)
{
	CString csTemp, str;
	switch (nButton)
	{
	case 1:
		str.Format("Please check if the network LED is flashing");
		break;
	case 2:
		str.Format("Please push the switch button to the other side");
		break;
	case 3:
		str.Format("Please push the power button and hold on for 7 seconds.");
	}
	//	CCPromptDlg pReset("Reset","请按下复位按钮5秒后松开","",0,BUTTON_YES,pParent);  0, BUTTON_OLPIC, pParent
	if (nButton == 1)
	{
		CCPromptDlg NetworkLED("NetworkLED", str, "Bin\\DNB1160S_NET3.jpg", 0, BUTTON_OLPIC, pParent);
		//NetworkLED.DisplayDlg();
		if (BTN_YES != NetworkLED.DisplayDlg())
		{
			m_errCode = LED_TEST_FAIL;
			ShowUIMessage("Network LED is off!");
			return -1;
		}
	}
	else if (nButton == 2)
	{
		CCPromptDlg pReset("Reset", str, "Bin\\Switch.jpg", TestIni.nCountDownSec, BUTTON_PIC, pParent);
		pReset.DisplayDlg();
	}
	else
	{
		CCPromptDlg pReset("power", str, "Bin\\Switch.jpg", TestIni.nCountDownSec, BUTTON_PIC, pParent);
		pReset.DisplayDlg();
	}
	return TRUE;
}

int TPM()//合格
{
	CString csTemp, csRecv, csSend;
	ComRecvCmd(hCom, "ifconfig lo 127.0.0.1 up", csRecv, "$");
	csRecv.Empty();
	if ("" == ComRecvCmd(hCom, "tcsd -f -c /usr/share/tcsd.conf &", csRecv, "TCSD up and running"))
	{
		ShowUIMessage("TCSD can not up");
		csRecv.Empty();
		ComRecvCmd(hCom, "killall -9 tcsd", csRecv, "$");
		return -1;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "tpm_selftest -l debug", csRecv, "$");
	if (-1 == csRecv.Find("tpm_selftest succeeded") && (-1 == csRecv.Find("800001ff")))
	{
		csRecv.Empty();
		ComRecvCmd(hCom, "killall -9 tcsd", csRecv, "$");
		return -1;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "killall -9 tcsd", csRecv, "$");
	return 0;
}

int Buzzer()//蜂鸣器  //合格
{
	ComSendCmd(hCom, "DNA1160_HWM 2");
	//ComSendCmd(hCom,"beep -f 600 -l 300 -r 5");//蜂鸣器发声命令
	//CCPromptDlg pPrompt("Buzzer","请确认是否听到蜂鸣器的声音","Bin\\BEEP.jpg",0, BUTTON_OLPIC,pParent);
	CCPromptDlg pPrompt("Buzzer", "Please check if you heard the sound of beeper", "Bin\\DNB1160S_BEEP.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pPrompt.DisplayDlg())
	{
		ShowUIMessage("Can not Hear Buzzer sound!");
		return -1;
	}
	return 0;
}

int HWMonitor()//进行温度、电压和风扇转速的范围检测  //合格
{
	CString csTemp, csRecv, csHWM;
	HWMSpec hwVal = {};
	csHWM.Format("DNA1160S_HWM");
	ComRecvCmd(hCom, csHWM, csRecv, "$");
	if (-1 == csRecv.Find("CPU Temperature"))
	{
		ShowUIMessage("Can not get CPU Temperature");//
		return -1;
	}
	GetCString(csRecv, "CPU Temperature", "=", "\n", csTemp);//,要先找到 CPU Temperature ，再从 CPU Temperature 起始位置开始 根据标志截取字符串
	csTemp.Remove('C');
	csTemp.Trim();
	hwVal.CPUTemp.fval = atof(csTemp);//atof 函数 把字符串转换成浮点数
	if (hwVal.CPUTemp.fval > TestIni.hwSpec.CPUTemp.maxval || hwVal.CPUTemp.fval < TestIni.hwSpec.CPUTemp.minval)
	{
		csTemp.Format("CPU Temp min spec:%f,max spec:%f", TestIni.hwSpec.CPUTemp.minval, TestIni.hwSpec.CPUTemp.maxval);
		ShowUIMessage(csTemp);
		ShowUIMessage("CPU Temperature out of range");//超出范围
		return -1;
	}
	GetCString(csRecv, "VCORE Voltage", "=", "\n", csTemp);
	csTemp.Remove('V');
	csTemp.Trim();
	hwVal.CPUVcore.fval = atof(csTemp);
	if (hwVal.CPUVcore.fval > TestIni.hwSpec.CPUVcore.maxval || hwVal.CPUVcore.fval < TestIni.hwSpec.CPUVcore.minval)
	{
		ShowUIMessage("CPU Vcore voltage out of range");
		return -1;
	}
	GetCString(csRecv, "3.3V Voltage", "=", "\n", csTemp);
	csTemp.Remove('V');
	csTemp.Trim();
	hwVal.Vol3P3.fval = atof(csTemp);
	if (hwVal.Vol3P3.fval > TestIni.hwSpec.Vol3P3.maxval || hwVal.Vol3P3.fval < TestIni.hwSpec.Vol3P3.minval)
	{
		ShowUIMessage("3.3V voltage out of range");
		return -1;
	}
	GetCString(csRecv, "DRAM Voltage", "=", "\n", csTemp);
	csTemp.Remove('V');
	csTemp.Trim();
	hwVal.VolDDR.fval = atof(csTemp);
	if (hwVal.VolDDR.fval > TestIni.hwSpec.VolDDR.maxval || hwVal.VolDDR.fval < TestIni.hwSpec.VolDDR.minval)
	{
		ShowUIMessage("DDR 1.35V voltage out of range");
		return -1;
	}
	GetCString(csRecv, "5V Voltage", "=", "\n", csTemp);
	csTemp.Remove('V');
	csTemp.Trim();
	hwVal.Vol5.fval = atof(csTemp);
	if (hwVal.Vol5.fval > TestIni.hwSpec.Vol5.maxval || hwVal.Vol5.fval < TestIni.hwSpec.Vol5.minval)
	{
		ShowUIMessage("5V voltage out of range");
		return -1;
	}
	GetCString(csRecv, "12V Voltage", "=", "\n", csTemp);
	csTemp.Remove('V');
	csTemp.Trim();
	hwVal.Vol12.fval = atof(csTemp);
	if (hwVal.Vol12.fval > TestIni.hwSpec.Vol12.maxval || hwVal.Vol12.fval < TestIni.hwSpec.Vol12.minval)
	{
		ShowUIMessage("12V voltage out of range");
		return -1;
	}
	GetCString(csRecv, "CPU FAN Speed", "=", "RPM", csTemp);
	csTemp.Trim();
	hwVal.CPUFAN.fval = atof(csTemp);
	if (hwVal.CPUFAN.fval > TestIni.hwSpec.CPUFAN.maxval || hwVal.CPUFAN.fval < TestIni.hwSpec.CPUFAN.minval)
	{
		ShowUIMessage("CPU FAN Speed out of range");
		return -1;
	}
	csTemp.Empty();
	csRecv.Empty();
	ComRecvCmd(hCom, "cat /proc/meminfo | grep MemTotal | awk '{ print int($2/1024000 +0.5)}'", csRecv, "$");
	GetCString(csRecv, "\r\n", "\r\n", csTemp);
	csTemp.Trim();
	if (station_flag == "FT")//FT
	{
		if (TestIni.MemSizeFT == atoi(csTemp))
		{
			csTemp.Format("Memory size:%sGB", csTemp);
			ShowUIMessage(csTemp);
			ShowUIMessage("Memory size meet a criterion");
		}
		else if (TestIni.MemSizeFT > atoi(csTemp))
		{
			csTemp.Format("Memory size:%sGB", csTemp);
			ShowUIMessage(csTemp);
			ShowUIMessage("Memory size is less than the standard value");
		}
		else if (TestIni.MemSizeFT < atoi(csTemp))
		{
			csTemp.Format("Memory size:%sGB", csTemp);
			ShowUIMessage(csTemp);
			ShowUIMessage("Memory size is is larger than the standard value");
		}
	}
	else
	{
		if (TestIni.MemSizePT == atoi(csTemp))
		{
			csTemp.Format("Memory size:%sGB", csTemp);
			ShowUIMessage(csTemp);
			ShowUIMessage("Memory size meet a criterion");
		}
		else if (TestIni.MemSizePT > atoi(csTemp))
		{
			csTemp.Format("Memory size:%sGB", csTemp);
			ShowUIMessage(csTemp);
			ShowUIMessage("Memory size is less than the standard value");
		}
		else if (TestIni.MemSizePT < atoi(csTemp))
		{
			csTemp.Format("Memory size:%sGB", csTemp);
			ShowUIMessage(csTemp);
			ShowUIMessage("Memory size is is larger than the standard value");
		}
	}
	if (TestIni.csCPUType != "skip")
	{
		csRecv.Empty();
		csTemp.Empty();
		ComRecvCmd(hCom, "cat /proc/cpuinfo | grep \"model name\"", csRecv, "$");
		if (-1 == csRecv.Find(TestIni.csCPUType))
		{
			ShowUIMessage("CPU model mismatch");
			return -1;
		}
		else
		{
			csTemp.Format("CPU model name:%s", TestIni.csCPUType);
			ShowUIMessage(csTemp);
		}
	}
	if (TestIni.csBIOSRev != "skip")
	{
		csRecv.Empty();
		csTemp.Empty();
		ComRecvCmd(hCom, "dmidecode -t bios | grep Version", csRecv, "$");
		if (-1 == csRecv.Find(TestIni.csBIOSRev))
		{
			ShowUIMessage("BIOS version mismatch");
			return -1;
		}
		else
		{
			csTemp.Format("BIOS version:%s", TestIni.csBIOSRev);
			ShowUIMessage(csTemp);
		}
	}
	return 0;
}

int VeryTime(CString csTimeVal)//将形参传入的时间按一定的格式进行转化，再与PC时间进行比较，再检查差值是否在一定的范围内
{
	CString strTime[7], csTemp;
	int nPos = 0;
	CTime cTimeSet = CTime::GetCurrentTime();

	if (csTimeVal.IsEmpty())
	{
		ShowUIMessage("Get system Time Error!");
		return -1;
	}
	for (int i = 0; i < 6; i++)
	{
		strTime[i] = csTimeVal.Tokenize(" ", nPos);//Tokenize函数将字符串 nPos 以空格进行分割
		strTime[i].Trim();
	}

	CString mon, month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	for (int i = 0; i < 12; ++i) {
		if (!strTime[1].Compare(month[i]))
		{
			mon.Format("%d", i + 1);
			strTime[1] = mon;
		}
	}

	CTime cTime(atoi(strTime[4]), atoi(strTime[1]), atoi(strTime[2]), atoi(strTime[3].Mid(0, 2)), atoi(strTime[3].Mid(3, 2)), atoi(strTime[3].Mid(6, 2)));
	CTimeSpan cts = cTime - cTimeSet;//过去减未来为负值，未来减过去为正值
	LONGLONG temp = cts.GetTotalSeconds();//将时间差转化为秒数
	temp += 28800;////--------东八区 时间 转换为 零时区 时间 要减去8小时----------------------------------------------
	csTemp.Format("Time Interval:%ld s", temp);
	ShowUIMessage(csTemp);
	if (temp > TestIni.RTCErrorRange || temp < (-(TestIni.RTCErrorRange)))//如果板子时间和PC时间的差值超过10秒，就判定 RTC Time out of range!
	{
		ShowUIMessage("RTC Time out of range!");
		return -1;
	}
	return 0;
}

int SetRTC() //合格
{
	CString csTemp, csRecv, csTime, strTime[7];
	int nPos = 0;
	time_t ltime;
	struct tm tml;
	char now[32];

	_tzset();//UNIX时间兼容函数
	time(&ltime);//time函数将自1970.1.1 00:00:00 开始的秒数保存在 ltime 中 
	ltime -= 28800;//--------东八区 时间 转换为 零时区 时间 要减去8小时----------------------------------------------
	if (_localtime64_s(&tml, &ltime))//将秒数转化为当前时间，并且保存在 tml 中 
	{
		return -1;
	}
	memset(now, 0, sizeof(now));
	strftime(now, sizeof(now), "%w %m %d %H:%M:%S %Y", &tml);//将 tml 保存的时间按一定格式存入 now
	csTime.Format("%s", now);
	csTemp.Format("System Time:%s", csTime);
	ShowUIMessage(csTemp);
	memset(now, 0, sizeof(now));
	strftime(now, sizeof(now), "%Y.%m.%d-%H:%M:%S", &tml);
	csTemp.Format("date -s %s", now);//修改linux系统时间的命令
	CTime cTimeSet = CTime::GetCurrentTime();//GetCurrentTime函数获取系统日期
	ComRecvCmd(hCom, csTemp, csRecv, "$");//发送修改linux系统时间的命令
	ComRecvCmd(hCom, "hwclock -w", csRecv, "$");//发送 hwclock -w 将系统时钟同步到硬件时钟 命令
	csRecv.Empty();
	ComRecvCmd(hCom, "hwclock", csRecv, "$");
	GetCString(csRecv, "\n", "\r", csTemp);
	regex re("^(Sun|Mon|Wed|Tue|Thu|Fri|Sat)(.*)(seconds)$");
	string str = (LPCSTR)(CString)(csTemp);
	if (regex_match(str, re))
	{
		return VeryTime(csTemp);
	}
	else
	{
		ShowUIMessage("hwclock: read time fail!");
		return -1;
	}
}

int CheckRTC()  //合格
{
	CString csTemp, csRecv;
	csRecv.Empty();
	ComRecvCmd(hCom, "hwclock", csRecv, "$");
	GetCString(csRecv, "\n", "\r", csTemp);
	regex re("^(Sun|Mon|Wed|Tue|Thu|Fri|Sat)(.*)(seconds)$");
	string str = (LPCSTR)(CString)(csTemp);
	if (regex_match(str, re))
	{
		return VeryTime(csTemp);
	}
	else
	{
		ShowUIMessage("hwclock: read time fail!");
		return -1;
	}
}

int WriteMAC()
{
	CString csTemp, csRecv, csMacBeg, csMacEnd, str, csAaa;
	BYTE nMACBeg[7], nMACEnd[7];
	int nReverse = -1, nPortNum = 0;
	ComRecvCmd(hCom, "eeupdate64e|grep I211|wc -l", csRecv, "$");
	if (-1 == csRecv.Find("4"))
	{
		ShowUIMessage("Not enough I211 ethernet port found");
		return -1;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "eeupdate64e|grep X553|wc -l", csRecv, "$");
	if (-1 == csRecv.Find("4"))            //这4个网口暂时没有驱动，先不管
	{
		ShowUIMessage("Not enough I354 ethernet port found");
		return -1;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "eeupdate64e", csRecv, "$");//新增加的网卡
	if (-1 == csRecv.Find("I210"))
	{
		ShowUIMessage("Not enough I210 ethernet port found");
		return -1;
	}
	csRecv.Empty();

	long long macnum_beg[1] = {0}, macnum_end[1] = {0};

	csTemp.Format("%c%c%c%c%c%c%c%c%c%c%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
	Hex2Long(macnum_beg, csTemp.GetBuffer());
	csTemp.Format("%c%c%c%c%c%c%c%c%c%c%c%c", sfisData->czSSN1[0], sfisData->czSSN1[1], sfisData->czSSN1[2], sfisData->czSSN1[3], sfisData->czSSN1[4], sfisData->czSSN1[5], sfisData->czSSN1[6], sfisData->czSSN1[7], sfisData->czSSN1[8], sfisData->czSSN1[9], sfisData->czSSN1[10], sfisData->czSSN1[11]);
	Hex2Long(macnum_end, csTemp.GetBuffer());
	
	long Length = abs(macnum_beg[0] - macnum_end[0]);
	if (Length != 8) 
	{
		ShowUIMessage("Length of MAC begin and MAC end must be 8");
		return -1;
	}
	
	if (macnum_beg[0] > macnum_end[0])
	{
		nReverse = 1;
	}
	if(macnum_beg[0] < macnum_end[0])
	{
		nReverse = 0;
	}

	if (-1 == nReverse)//若 -1==nReverse 则表示，nMACBeg[7]和nMACEnd[7] 两个数组中的内容完全一样
	{
		ShowUIMessage("Same Barcode for MAC Begin and MAC End");
		return -1;
	}
	if (nReverse == 1)//若 nMACBeg 大于 nMACEnd ，则将两个数组的内容交换存入如下
	{
		csMacEnd.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
		csMacBeg.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czSSN1[0], sfisData->czSSN1[1], sfisData->czSSN1[2], sfisData->czSSN1[3], sfisData->czSSN1[4], sfisData->czSSN1[5], sfisData->czSSN1[6], sfisData->czSSN1[7], sfisData->czSSN1[8], sfisData->czSSN1[9], sfisData->czSSN1[10], sfisData->czSSN1[11]);
	}
	else
	{
		csMacBeg.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
		csMacEnd.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czSSN1[0], sfisData->czSSN1[1], sfisData->czSSN1[2], sfisData->czSSN1[3], sfisData->czSSN1[4], sfisData->czSSN1[5], sfisData->czSSN1[6], sfisData->czSSN1[7], sfisData->czSSN1[8], sfisData->czSSN1[9], sfisData->czSSN1[10], sfisData->czSSN1[11]);
	}
	csTemp = csMacBeg;
	for (int i = 1; i < TestIni.nNetPort; i++)//此例中，nNetPort 暂时为9
	{
		if (!MACIncr(csTemp, csRecv))
		{
			ShowUIMessage("MAC Number Increased fail!");//后3个数都为0XFF 就会导致这个结果
			return -1;
		}
		csTemp = csRecv;
	}

	if (csTemp.CompareNoCase(csMacEnd))//经过 MACIncr 函数处理后的MAC 若与 csMacEnd 不同，就提示 MAC Label not right
	{
		csTemp.Format("Begin MAC Address:%s", csMacBeg);
		ShowUIMessage(csTemp);
		csTemp.Format("End MAC Address:%s", csMacEnd);
		ShowUIMessage(csTemp);
		ShowUIMessage("MAC Label not right");
		return -1;
	}
	csRecv.Empty();
	csMacBeg.Remove(':');
	csAaa = csMacBeg;
	str = csMacBeg;
	for (int i = 0; i < 4; i++)
	{
		if (!MACIncr(csAaa, str))
		{
			ShowUIMessage("MAC Address Add Failed");//后3个数都为0XFF 就会导致这个结果
			return -1;
		}
		csAaa = str;
		str.Remove(':');
	}
	for (int i = 0; i < 5; i++)//待定------------------------------------------------
	{
		csTemp.Format("eeupdate64e /NIC=%d /MAC=%s", i + 1, str);// NIC(1-5),MAC(5-9),eth(0-4)
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("Done"))
		{
			csTemp.Format("Write eth%d mac address:%s fail", i, str);
			ShowUIMessage(csTemp);
			return -1;
		}

		if (!MACIncr(csAaa, str))
		{
			ShowUIMessage("MAC Address Add Failed");//后3个数都为0XFF 就会导致这个结果
			return -1;
		}
		csAaa = str;
		str.Remove(':');
	}

	csAaa = csMacBeg;
	str = csMacBeg;
	for (int i = 6; i <= 9; i++)
	{
		csTemp.Format("eeupdate64e /NIC=%d /MAC=%s", i, str);// NIC(6-9),MAC(1-4),eth(5-8)
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("Done"))
		{
			csTemp.Format("Write eth%d mac address:%s fail", i - 1, str);
			ShowUIMessage(csTemp);
			return -1;
		}

		if (!MACIncr(csAaa, str))
		{
			ShowUIMessage("MAC Address Add Failed");//后3个数都为0XFF 就会导致这个结果
			return -1;
		}
		csAaa = str;
		str.Remove(':');
	}

	return 0;
}

int Network()  //合格
{
	CString csTemp, csRecv, csEthMac[MAX_NET_PORT], csIP[MAX_NET_PORT], csPacketBef[MAX_NET_PORT], csPacketBak[MAX_NET_PORT];
	int nTemp;
	ComRecvCmd(hCom, "ifconfig -a|grep eth", csRecv, "$");
	for (int i = 0; i < TestIni.nNetPort + 1; i++)//
	{
		csTemp.Format("eth%d", i);
		if (-1 == csRecv.Find(csTemp) && i != 4)
		{
			csTemp.Format("eth%d not found", i);//8个网口都被找到才不会失败
			ShowUIMessage(csTemp);
			return -1;
		}
	}
	for (int i = 0; i < TestIni.nNetPort + 1; i++)
	{
		csTemp.Format("eth%d", i);
		GetCString(csRecv, csTemp, "HWaddr", "\n", csEthMac[i]);
		csEthMac[i].Trim();
		if (csEthMac[i].IsEmpty() && i != 4)
		{
			csTemp.Format("eth%d Mac Address not found", i);//若有任一个网口的硬件地址为空都会导致失败
			ShowUIMessage(csTemp);
			return -1;
		}
	}
	csRecv = csModel;
	csRecv.MakeUpper();//将 csRecv 中内容转化为大写字符串
	CCPromptDlg pLEDFront1("LED", "Please check network LED is on", "Bin\\DNB1160S_NET1.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pLEDFront1.DisplayDlg())
	{
		m_errCode = NETWORK_LED_FAIL;
		ShowUIMessage("Network LED not lighted!");
		return -1;
	}
	CCPromptDlg pLEDFront2("LED", "Please check network LED is on", "Bin\\DNB1160S_NET2.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pLEDFront2.DisplayDlg())
	{
		m_errCode = NETWORK_LED_FAIL;
		ShowUIMessage("Network LED not lighted!");
		return -1;
	}
	for (int i = 0; i < TestIni.nNetPort / 2; i++)////--------- 0到4 --- 1到5 ----2到6----3到7-----------
	{
		csRecv.Empty();
		csTemp.Format("ifconfig eth%d|grep \"TX packets\"", i);//寻找发送包
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("TX packets:"))
		{
			csTemp.Format("eth%d TX packets not found", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		GetCString(csRecv, "TX packets:", "errors", csPacketBef[i]);//寻找发送包的数量
		csPacketBef[i].Trim();
		if (csPacketBef[i].IsEmpty())
		{
			ShowUIMessage(csPacketBef[i]);
			return -1;
		}
		csRecv.Empty();
		nTemp = i + 1 + (TestIni.nNetPort / 2);//--------- 0到4 --- 1到5 ----2到6----3到7-----------
		//nTemp = i + 1;//0到1 ，2到3 ，4到5 ，6到7
		csTemp.Format("ifconfig eth%d|grep \"RX packets\"", nTemp);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("RX packets:"))
		{
			csTemp.Format("eth%d RX packets not found", nTemp);
			ShowUIMessage(csTemp);
			return -1;
		}
		GetCString(csRecv, "RX packets:", "errors", csPacketBef[nTemp]);
		csPacketBef[nTemp].Trim();
		if (csPacketBef[nTemp].IsEmpty())//IsEmpty 函数返回值为1时，表示判断的对象为空
		{
			ShowUIMessage(csPacketBef[nTemp]);
			return -1;
		}
	}

	ComRecvCmd(hCom, "echo \"rem_device_all\" > /proc/net/pktgen/kpktgend_0", csRecv, "$");//将echo的输出重定向到 /proc/net/pktgen/kpktgend_0
	csRecv.Empty();																		//rem_device_all 删除绑定在某个线程的所有端口
	Sleep(10);
	for (int i = 0; i < TestIni.nNetPort / 2; i++)
	{
		csTemp.Format("echo \"add_device eth%d\" > /proc/net/pktgen/kpktgend_0", i);//add_device 添加某个端口到某个线程
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();
		Sleep(10);
	}
	ComRecvCmd(hCom, "echo \"max_before_softirq 10000\" > /proc/net/pktgen/kpktgend_0", csRecv, "$");//max_before_softirq 在最多发送多少个数据包后,执行do_softirq()，此处为 10000
	csRecv.Empty();
	Sleep(10);
	for (int i = 0; i < TestIni.nNetPort / 2; i++)
	{
		csTemp.Format("echo \"clone_skb 10000\" > /proc/net/pktgen/eth%d", i);//clone_skb 对每个skb进行多少个复制（此处为10000），0表示不复制。对于Dos等测试必须置零
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();
		Sleep(10);
	}
	for (int i = 0; i < TestIni.nNetPort / 2; i++)//--------- 0到4 --- 1到5 ----2到6----3到7-----------
	{
		csTemp.Format("echo \"pkt_size 60\" > /proc/net/pktgen/eth%d", i);//pkt_size 链路包的大小（减去CRC的值），此处为 60
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();
		Sleep(10);
		csTemp.Format("echo \"src_mac %s\" > /proc/net/pktgen/eth%d", csEthMac[i], i);//src_mac 源mac
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();
		Sleep(10);
		csTemp.Format("echo \"dst_mac %s\" > /proc/net/pktgen/eth%d", csEthMac[i + 1 + (TestIni.nNetPort / 2)], i);//dst_mac 目的MAC地址，此处为广播地址
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();

		csTemp.Format("echo \"count 500000\" > /proc/net/pktgen/eth%d", i);//count 发送数据包的个数，0 表示一直发送。此处为 100000
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();
		csTemp.Format("echo \"delay 10000\" > /proc/net/pktgen/eth%d", i);//delay 发送两个数据包之间的延时
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csRecv.Empty();
	}
	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPOWER1("NetworkLED", "Please pay attention to Network LED.Click YES when you finished. ", "Bin\\DNB1160S_NET3.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPOWER1.DisplayDlg())
		{
			if (i == 2)
			{
				m_errCode = LED_TEST_FAIL;
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please pay attention to Network LED.");
		}
		else
		{
			break;
		}
	}
	ComRecvCmd(hCom, "echo \"start\" > /proc/net/pktgen/pgctrl", csRecv, "$");//start 所有的线程开始发送
	csRecv.Empty();
	if (-1 == ButtonThread(1))
	{
		ShowUIMessage("Network LED is not flashing!");
		return -1;
	}

	for (int i = 0; i < TestIni.nNetPort / 2; i++)//--------- 0到4 --- 1到5 --------2到6-----3到7------
	{
		csRecv.Empty();
		csTemp.Format("ifconfig eth%d|grep \"TX packets\"", i);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("TX packets:"))
		{
			csTemp.Format("eth%d TX packets not found", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		GetCString(csRecv, "TX packets:", "errors", csPacketBak[i]);
		csPacketBak[i].Trim();

		if (csPacketBak[i].IsEmpty())
		{
			ShowUIMessage(csPacketBak[i]);
			return -1;
		}

		nTemp = i + 1 + (TestIni.nNetPort / 2);//--------- 0到4 --- 1到5 --------2到6-----3到7------
		//nTemp = i + 1;//--------- 0到1 --- 2到3 --------4到5-----6到7------
		csRecv.Empty();
		csTemp.Format("ifconfig eth%d|grep \"RX packets\"", nTemp);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("RX packets:"))
		{
			csTemp.Format("eth%d RX packets not found", nTemp);
			ShowUIMessage(csTemp);
			return -1;
		}
		GetCString(csRecv, "RX packets:", "errors", csPacketBak[nTemp]);
		csPacketBak[nTemp].Trim();
		if (csPacketBak[nTemp].IsEmpty())
		{
			ShowUIMessage(csPacketBak[nTemp]);
			return -1;
		}
	}

	for (int i = 0; i < TestIni.nNetPort; i++)
	{
		if (i == 4)
		{
			continue;
		}
		int nBef = 0, nBak = 0;
		nBef = atoi(csPacketBef[i]);
		nBak = atoi(csPacketBak[i]);
		if (nBak - nBef < 500000)
		{
			//if(i%2==0)
			if (i <= 3)
			{
				csTemp.Format("eth%d Tx Packets not enough", i);
			}
			else
			{
				csTemp.Format("eth%d Rx Packets not enough", i);
			}
			ShowUIMessage(csTemp);
			return -1;
		}
	}
	return 0;
}

int USB() //合格
{
	CString csSend,csTemp, csRecv, str, csUSB[6], strUSB,s;
	int nPos = 0, nUSB = 0, hu = 0;
	int USB_quantity = 0; 
	ComRecvCmd(hCom, "fdisk -l |grep -v Disk | grep /dev | awk -F ' ' '{print($1)}'", csRecv, "$");

	csTemp = csRecv.Tokenize("\n", nPos); 
	while (!csTemp.IsEmpty())
	{
		if (-1 != csTemp.Find("sd"))
		{
			csTemp.Remove('\r');
			csTemp.Remove('\n');
			csUSB[nUSB] = csTemp;
			nUSB++;
		}
		str.Empty();
		csTemp = csRecv.Tokenize("\n", nPos);
	}
	/*s.Format("%d", TestIni.nUSBNum);
	ShowUIMessage("usb num are:"+ s);*/

	if (nUSB != TestIni.nUSBNum)//此处判断有几个USB 通过了测试,在DNA1160上应该为 2 ,请看配置文件或 LoadIni函数中是否已经更改为 2
	{
		ShowUIMessage("Not Enough USB test pass");
		return -1;
	}

	for (int i = 0; i < nUSB; i++)
	{
		csTemp.Format("mount \"%s\" \/mnt\/usb", csUSB[i]);/////////////////////////////////////////////////////----------------------
		ComRecvCmd(hCom, csTemp, str, "$");
		str.Empty();
		ComRecvCmd(hCom, "mount", str, "$");
		ComRecvCmd(hCom, "rm -rf /tmp/test", str, "$");//rm-rf表示删除文件，而且可以删除非空目录。-rf参数表示递归强制删除。
		str.Empty();
		ComRecvCmd(hCom, "rm -rf /mnt/usb/test", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "test -e /mnt/usb/test", str, "$");//test -e 如果文件存在则返回 0
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 != str.Find("0"))//如果在返回信息中找到了 0 ，就说明文件没有成功删除,此处的 $? 是 test 的返回值
		{
			ShowUIMessage("Can not delete test file");
			return -1;
		}
		str.Empty();
		ComRecvCmd(hCom, "dd if=/dev/zero of=/tmp/test count=1M bs=1", str, "$");//把 /dev/zero 内容拷到 /tmp/test 
		str.Empty();
		ComRecvCmd(hCom, "cp -arf /tmp/test /mnt/usb", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "test -e /mnt/usb/test", str, "$");//检查是否有 /mnt/usb/test 文件存在
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 == str.Find("0"))//若此处找不到 0 ，就表示没有在 /mnt/usb 目录下创建 test 文件，则表示 test 文件没有复制成功
		{
			ShowUIMessage("Can not copy test file to USB");
			return -1;
		}
		str.Empty();
		ComRecvCmd(hCom, "cp -arf /mnt/usb/test /tmp/test1", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "test -e /tmp/test1", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 == str.Find("0"))
		{
			ShowUIMessage("Can not read file from USB");
			return -1;
		}
		str.Empty();
		ComRecvCmd(hCom, "cmp -l /tmp/test /tmp/test1", str, "$");//比较两个被复制的文件
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 == str.Find("0"))
		{
			ShowUIMessage("File copied not same with orginal file");
			return -1;
		}

		//-------------此处应该卸载USB，准备下个USB的挂载-----------------------------------------------
		csTemp.Empty();
		csTemp.Format("umount \/mnt\/usb");
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		csTemp.Empty();
		csTemp.Format("mount");
		ComRecvCmd(hCom, csTemp, csRecv, "$");
	}
	return 0;
}

int CFCard()
{
	CString csTemp, csRecv, str, csCF, csSend;
	int nPos = 0;
	ComRecvCmd(hCom, "sfdisk -s|grep /dev/|grep -v lv|awk -F':' '{print $1}'", csRecv, "$");//sfdisk -s 命令显示分区的大小
	csTemp = csRecv.Tokenize("\n", nPos);
	while (!csTemp.IsEmpty())
	{
		if (-1 != csTemp.Find("sd"))
		{
			csCF.Format("smartctl -i %s", csTemp);//smartctl -i 显示指定的硬盘设备的参数信息
			ComRecvCmd(hCom, csCF, str, "$");
			if (-1 != str.Find("ATA Version") && -1 != str.Find("Solid State Device"))  //信息显示为固态设备
			{
				csCF = csTemp;
				csCF.Remove('\r');
				csCF.Remove('\n');
				break;
			}
		}
		csTemp = csRecv.Tokenize("\n", nPos);
	}
	if (csCF.IsEmpty() || -1 == csCF.Find("sd"))
	{
		ShowUIMessage("No CF Card Found!");
		return -1;
	}
	csRecv.Empty();
	csSend.Empty();
	csSend.Format("mkfs.vfat %s", csCF);
	ComRecvCmd(hCom, csSend, csRecv, "$");
	csSend.Empty();
	csRecv.Empty();
	ComRecvCmd(hCom, "ls /mnt", csRecv, "$");
	if (-1 == csRecv.Find("CF"))
	{
		csRecv.Empty();
		ComRecvCmd(hCom, "mkdir /mnt/CF", csRecv, "$");
	}
	csRecv.Empty();
	csTemp.Format("mount \"%s\" \/mnt\/CF", csCF);
	ComRecvCmd(hCom, csTemp, str, "$");
	str.Empty();
	ComRecvCmd(hCom, "mount", str, "$");
	if (-1 != str.Find(csCF))
	{
		ComRecvCmd(hCom, "rm -rf /mnt/CF/test", str, "$");//rm-rf表示删除文件，而且可以删除非空目录。-rf参数表示递归强制删除。
		str.Empty();
		ComRecvCmd(hCom, "test -e /mnt/CF/test", str, "$");//test -e 如果文件存在则返回 0
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 != str.Find("0"))
		{
			ShowUIMessage("Can not delete test file");
			return -1;
		}
		str.Empty();
		ComRecvCmd(hCom, "dd if=/dev/zero of=/tmp/test count=1M bs=1", str, "$");//将 /dev/zero 拷到  /tmp/test
		str.Empty();
		ComRecvCmd(hCom, "cp -arf /tmp/test /mnt/CF", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "test -e /mnt/CF/test", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 == str.Find("0"))
		{
			ShowUIMessage("Can not copy test file to CF Card");
			return -1;
		}
		str.Empty();
		ComRecvCmd(hCom, "cp -arf /mnt/CF/test /tmp/test1", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "test -e /tmp/test1", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 == str.Find("0"))
		{
			ShowUIMessage("Can not read file from CF Card");
			return -1;
		}
		str.Empty();
		ComRecvCmd(hCom, "cmp -l /tmp/test /tmp/test1", str, "$");
		str.Empty();
		ComRecvCmd(hCom, "echo $?", str, "$");
		if (-1 == str.Find("0"))
		{
			ShowUIMessage("File copied not same with orginal file");
			return -1;
		}
		return 0;
	}
	return -1;
}

int SATA()  //合格
{
	CString csTemp, csRecv, str, csSATA, csSend, csAaa;
	int nPos = 0, nUSB = 0;
	int nSata = 0;
	ComRecvCmd(hCom, "\r", csRecv, "$");
	Sleep(500);
	ComRecvCmd(hCom, "sfdisk -s|grep /dev/|grep -v lv|awk -F':' '{print $1}'", csRecv, "$");
	csTemp = csRecv.Tokenize("\n", nPos);
	nSata = 0;
	while (!csTemp.IsEmpty())
	{
		if (-1 != csTemp.Find("sd"))
		{
			csTemp.Remove('\r');
			csTemp.Remove('\n');
			csSATA.Format("smartctl -i %s", csTemp);//smartctl -i 显示指定的硬盘设备的参数信息
			ComRecvCmd(hCom, csSATA, str, "$");
			if (-1 != str.Find("ATA Version") && (-1 != str.Find("SATA Version")))
			{
				csSATA = csTemp;
				//--------------------------- 自己添加的代码 ------------------------------------------------------------------------------------------
				csTemp.Format("ls \/mnt");//
				ComRecvCmd(hCom, csTemp, csAaa, "$");
				if (-1 == csAaa.Find("sata"))
				{
					csAaa.Empty();
					csTemp.Format("mkdir \/mnt\/sata");//创建 /mnt/sata 目录
					ComRecvCmd(hCom, csTemp, csAaa, "$");
				}
				csAaa.Empty();
				//-------------------------------------------------------------------------------------------------------------------------------------
				for (int i = 0; i < 2; i++)
				{
					csTemp.Format("mount \"%s\" \/mnt\/sata", csSATA, 1);//挂载
					ComRecvCmd(hCom, csTemp, str, "$");
					str.Empty();
					ComRecvCmd(hCom, "mount", str, "$");//查看挂载信息
					if (-1 == str.Find(csSATA))
					{
						csAaa.Empty();
						csSend.Format("mkfs.vfat %s", csSATA);
						ComRecvCmd(hCom, csSend, csAaa, "$");
					}
					else
					{
						break;
					}
				}
				ComRecvCmd(hCom, "rm -rf /tmp/test", str, "$");//删除 /tmp/test 
				str.Empty();
				ComRecvCmd(hCom, "dd if=/dev/zero of=/tmp/test bs=1M count=1", str, "$");//将 /dev/zero 拷到 /tmp/test
				str.Empty();
				ComRecvCmd(hCom, "test -e /tmp/test", str, "$");//test -e 如果文件存在则返回 0
				str.Empty();
				ComRecvCmd(hCom, "echo $?", str, "$");
				if (-1 == str.Find("0"))
				{
					ShowUIMessage("Can not touch file");//表明没有成功创建这个 /tmp/test
					return -1;
				}
				ComRecvCmd(hCom, "rm -rf /tmp/test1", str, "$");
				str.Empty();

				ComRecvCmd(hCom, "cp -arf /tmp/test /mnt/sata", str, "$");
				str.Empty();
				ComRecvCmd(hCom, "cp -f /mnt/sata/test /tmp/test1", str, "$");
				str.Empty();
				ComRecvCmd(hCom, "cmp -l /tmp/test /tmp/test1", str, "$");
				str.Empty();
				ComRecvCmd(hCom, "echo $?", str, "$");
				if (-1 == str.Find("0"))
				{
					ShowUIMessage("Copy file failed for SATA Disk");
					return -1;
				}
				nSata++;
				ComRecvCmd(hCom, "umount /mnt/sata", str, "$");
				Sleep(10);
				ComRecvCmd(hCom, "mount", str, "$");
				if (-1 != str.Find(csSATA))
				{
					ShowUIMessage("Can not umount SATA");
					return -1;
				}
				if (nSata == TestIni.nSATANum)//nSATANum 在本例 DNB1160-S 中为 2
				{
					break;
				}
			}
		}
		str.Empty();
		csTemp = csRecv.Tokenize("\n", nPos);
	}
	if (csSATA.IsEmpty())
	{
		ShowUIMessage("No SATA Disk found");
		return -1;
	}
	if (nSata == TestIni.nSATANum)
	{
		return 0;
	}
	else
	{
		ShowUIMessage("Can not find all satas");
		return -1;
	}
}

int Memory()  //合格
{
	CString csTemp, csRecv, csMemMsg;
	int pass_num = 0;
	int nPos = 0;
	csRecv.Empty();
	csTemp.Format("free");
	ComRecvCmd(hCom, csTemp, csRecv, "$");
	csTemp.Empty();
	csRecv.Empty();
	csTemp = "rm -f /tmp/log_mem";
	ComSendCmd(hCom, csTemp);
	csRecv.Empty();
	csTemp.Format("memtester %s 1 > /tmp/log_Mem &", TestIni.nMemSize);//测试1M内存，将 memtester 命令结果重定向输出到 /tmp/log_Mem
	ShowUIMessage(csTemp);//--------------------------------------
	ComRecvOnTime(hCom, csTemp, csRecv, "$", 600);//这个函数在这里会寻找600次提示符 $
	while (1)
	{
		Sleep(5000);
		csRecv.Empty();
		ComRecvCmd(hCom, "ps", csRecv, "$");//ps 显示当前运行的进程
		if (-1 == csRecv.Find("memtester"))//寻找 memtester 进程
		{
			break;
		}
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "cat /tmp/log_Mem|grep Done", csRecv, "$");
	if (-1 == csRecv.Find("Done"))
	{
		ShowUIMessage("Memory Test not done");
		return -1;
	}
	csRecv.Empty();
	Sleep(1000);
	ComRecvCmd(hCom, "cat /tmp/log_Mem | grep ok|wc -l", csRecv, "$");
	if (-1 == csRecv.Find("18"))
	{
		ShowUIMessage("Memory Test Item lost");
		return -1;
	}
	return 0;
}

int PowerButton()  //合格
{
	CString csTemp, csRecv, csSend;
	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPrompt("PowerButton", "Please push the power button and hold on for 5 seconds.If you have finished, click the YES button", "Bin\\DNB1160S_POWER.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPrompt.DisplayDlg())
		{
			if (i == 2)
			{
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please push the power button and hold on for 5 seconds");
		}
		else
		{
			break;
		}
	}
	Sleep(2000);
	ComSendCmd(hCom, "\r");
	if (!ComFind(hCom, csRecv, "$"))
	{
		ShowUIMessage("Unable to complete shutdown!");
		return -1;
	}
	return 0;
}

int CheckMAC()
{
	CString csTemp, csRecv, csMacBeg, csMacEnd, str, csAaa;
	BYTE nMACBeg[6], nMACEnd[6];
	int nReverse = -1;
	long long macnum_beg[1] = { 0 }, macnum_end[1] = { 0 };

	csTemp.Format("%c%c%c%c%c%c%c%c%c%c%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
	Hex2Long(macnum_beg, csTemp.GetBuffer());
	csTemp.Format("%c%c%c%c%c%c%c%c%c%c%c%c", sfisData->czSSN1[0], sfisData->czSSN1[1], sfisData->czSSN1[2], sfisData->czSSN1[3], sfisData->czSSN1[4], sfisData->czSSN1[5], sfisData->czSSN1[6], sfisData->czSSN1[7], sfisData->czSSN1[8], sfisData->czSSN1[9], sfisData->czSSN1[10], sfisData->czSSN1[11]);
	Hex2Long(macnum_end, csTemp.GetBuffer());

	long Length = abs(macnum_beg[0] - macnum_end[0]);
	if (Length != 8)
	{
		ShowUIMessage("Length of MAC begin and MAC end must be 8");
		return -1;
	}

	if (macnum_beg[0] > macnum_end[0])
	{
		nReverse = 1;
	}
	if (macnum_beg[0] < macnum_end[0])
	{
		nReverse = 0;
	}

	if (-1 == nReverse)
	{
		ShowUIMessage("Same Barcode for MAC Begin and MAC End");
		return -1;
	}
	if (nReverse == 1)
	{
		csMacEnd.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
		csMacBeg.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czSSN1[0], sfisData->czSSN1[1], sfisData->czSSN1[2], sfisData->czSSN1[3], sfisData->czSSN1[4], sfisData->czSSN1[5], sfisData->czSSN1[6], sfisData->czSSN1[7], sfisData->czSSN1[8], sfisData->czSSN1[9], sfisData->czSSN1[10], sfisData->czSSN1[11]);
	}
	else
	{
		csMacBeg.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czMAC[0], sfisData->czMAC[1], sfisData->czMAC[2], sfisData->czMAC[3], sfisData->czMAC[4], sfisData->czMAC[5], sfisData->czMAC[6], sfisData->czMAC[7], sfisData->czMAC[8], sfisData->czMAC[9], sfisData->czMAC[10], sfisData->czMAC[11]);
		csMacEnd.Format("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", sfisData->czSSN1[0], sfisData->czSSN1[1], sfisData->czSSN1[2], sfisData->czSSN1[3], sfisData->czSSN1[4], sfisData->czSSN1[5], sfisData->czSSN1[6], sfisData->czSSN1[7], sfisData->czSSN1[8], sfisData->czSSN1[9], sfisData->czSSN1[10], sfisData->czSSN1[11]);
	}
	csTemp = csMacBeg;
	for (int i = 1; i < TestIni.nNetPort; i++)//此处将 TestIni.nNetPort 暂时改为 4 ，是因为还有 4 个网卡没驱动
	{
		if (!MACIncr(csTemp, csRecv))//MACIncr 函数将 csTemp 经过一系列处理后以十六进制形式存放在 csRecv 中
		{
			ShowUIMessage("MAC Number Increased fail!");
		}
		csTemp = csRecv;
	}
	if (csTemp.CompareNoCase(csMacEnd))//如果经过处理后的 csMacBeg 与 csMacEnd 不相等，则报错，测试失败
	{
		csTemp.Format("Begin MAC Address:%s", csMacBeg);
		ShowUIMessage(csTemp);
		csTemp.Format("End MAC Address:%s", csMacEnd);
		ShowUIMessage(csTemp);
		ShowUIMessage("MAC Label not right");
		return -1;
	}
	str = csMacBeg;
	csAaa = csMacBeg;
	for (int i = 0; i < 4; i++)
	{
		if (!MACIncr(csAaa, str))
		{
			ShowUIMessage("MAC Address Add Failed");//后3个数都为0XFF 就会导致这个结果
			return -1;
		}
		csAaa = str;
	}
	for (int i = 0; i < 5; i++)
	{
		csTemp.Format("ifconfig eth%d|grep HWaddr", i);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("HWaddr") || -1 == csRecv.Find(str))
		{
			csTemp.Format("eth%d mac address %s not found", i, str);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
		if (!MACIncr(csAaa, str))
		{
			ShowUIMessage("MAC Address Add Failed");
			return -1;
		}
		csAaa = str;
	}
	str = csMacBeg;
	csAaa = csMacBeg;
	for (int i = 5; i < 9; i++)
	{
		csTemp.Format("ifconfig eth%d|grep HWaddr", i);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("HWaddr") || -1 == csRecv.Find(str))
		{
			csTemp.Format("eth%d mac address %s not found", i, str);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
		if (!MACIncr(csAaa, str))
		{
			ShowUIMessage("MAC Address Add Failed");
			return -1;
		}
		csAaa = str;
	}
	return 0;
}

int ShellMode()//向板子发送回车符，检测是否会出现提示符，若出现返回 0 ，表示进入系统成功
{
	CString csTemp, csRecv;
	int nFind = 0, nEnter = 0;
	if (!bMode)
	{
		if (NULL == hCom)
		{
			hCom = InitCom(nComPort, 115200);  //tree MD
			if (NULL == hCom)
			{
				//	CCPromptDlg pCom("COM","请确认串口是否被占用并关闭被占用的串口","",0,BUTTON_YES,pParent);
				CCPromptDlg pCom("COM", "Please check console port is occupied by other program", "", 0, BUTTON_YES, pParent);
				pCom.DisplayDlg();
				return -1;
			}
		}
		if (hCom != NULL)
		{
			for (int i = 0; i < 20; i++)
			{
				ComRecv(hCom, csRecv, FALSE);
				if (csRecv.IsEmpty())
				{
					Sleep(500);
					ComSendCmd(hCom, "\r");
					continue;
				}
				else
				{
					nFind = 1;
					break;
				}
			}
			if (!nFind)
			{
				CloseCom(hCom);
				hCom = NULL;
				return -1;
			}
		}
	}
	nFind = 0;
	DWORD TimeStart = ::GetTickCount();// GetTickCount 函数返回自操作系统启动所经过的毫秒数
	while (::GetTickCount() - TimeStart < 180000)
	{
		if (::GetTickCount() - TimeStart > 10000 && !nEnter)//10000&&!nEnter=1
		{
			nEnter = 1;
			ComSendCmd(hCom, "\r");
		}
		if (!ComFind(hCom, csRecv, "$"))//找提示符
		{
			ShowUIActMessage("", 0xFF);
			nFind = 1;
			break;
		}
	}
	if (!nFind)
	{
		bMode = 0;
		ShowUIMessage("Enter system fail");
		return -1;
	}
	return 0;
}

int VGA()//看终端2屏幕显示是否正确
{
	CString csTemp, csRecv;
	ComRecvCmd(hCom, "chvt 2", csRecv, "$");//chvt 命令切换终端
	CCPromptDlg pVGA("VGA", "Please confirm whether the product display is shown in the picture", "Bin\\VGA.jpg", 0, BUTTON_OLPIC, pParent);
	if (pVGA.DisplayDlg() != BTN_YES)
	{
		ComSendCmd(hCom, "chvt 1");
		return -1;
	}
	ComRecvCmd(hCom, "chvt 1", csRecv, "$");
	return 0;
}

int CFDetail()
{
	CString csTemp, csRecv, str, csCF;
	int nPos = 0;
	ComRecvCmd(hCom, "sfdisk -s|grep /dev/|grep -v lv|awk -F':' '{print $1}'", csRecv, "$");
	csTemp = csRecv.Tokenize("\n", nPos);//Tokenize 函数以 \n 为分隔符分割字符串
	while (!csTemp.IsEmpty())
	{
		if (-1 != csTemp.Find("sd"))
		{
			csCF.Format("smartctl -i %s", csTemp);//smartctl -i 显示磁盘设备的信息
			ComRecvCmd(hCom, csCF, str, "$");
			if (-1 != str.Find("ATA Version") && -1 != str.Find("Solid State Device"))
			{
				csCF = csTemp;
				csCF.Remove('\r');
				csCF.Remove('\n');
				break;
			}
		}
		csTemp = csRecv.Tokenize("\n", nPos);
	}
	if (csCF.IsEmpty() || -1 == csCF.Find("sd"))
	{
		ShowUIMessage("No CF Card Found!");
		return -1;
	}
	Sleep(10);
	csTemp.Format("sfdisk -s %s", csCF);//csCF 中为磁盘设备的名称
	ComRecvCmd(hCom, csTemp, csRecv, "$");

	GetCString(csRecv, "\n", "$", csTemp);
	if (csTemp.IsEmpty())
	{
		ShowUIMessage(csTemp);
		ShowUIMessage("No blocks found!\n");
		return -1;
	}

	csTemp.Remove('\n');
	csTemp.Remove('\\');
	csTemp.Trim();
	csRecv.Empty();
	csTemp.Format("CFSmart %s > /tmp/cfblock", csCF);
	ComRecvCmd(hCom, csTemp, csRecv, "$");

	csRecv.Empty();
	ComRecvCmd(hCom, "cat /tmp/cfblock|tail -n 3", csRecv, "$");
	if (-1 == csRecv.Find("OK"))
	{
		ShowUIMessage("Error CF Card badblock result");
		return -1;
	}
}

int HDDDetail()//硬盘  //合格
{
	CString csTemp, csRecv, str, csSATA;
	int nPos = 0, nUSB = 0, nTemp = 0;
	int nSata = 0;
	CString csItem[5] = { "Reallocated_Sector_Ct","Reallocated_Event_Count","Current_Pending_Sector","Offline_Uncorrectable","UDMA_CRC_Error_Count" };
	//重映射扇区数          //重新配置事件的数量       //当前待映射扇区计数     //脱机无法校正的扇区计数 //DMA通道CRC检验错误次数
	ComRecvCmd(hCom, "sfdisk -s|grep /dev/|grep -v lv|awk -F':' '{print $1}'", csRecv, "$");
	csTemp = csRecv.Tokenize("\n", nPos);
	nSata = 0;
	while (!csTemp.IsEmpty())
	{
		csSATA.Format("smartctl -i %s", csTemp);//smartctl -i 显示指定的磁盘的信息
		ComRecvCmd(hCom, csSATA, str, "$");
		csSATA.Empty();
		if (-1 != str.Find("ATA Version") && (-1 != str.Find("Rotation Rate")) && -1 == str.Find("Solid State Device"))//此处寻找标志字符应注意，还不敢确定
		{
			csSATA = csTemp;
			csSATA.Remove('\r');
			csSATA.Remove('\n');
			str.Empty();
			csTemp.Format("smartctl --all %s|grep \"Power_On_Hours\" |cut -d'-' -f 2", csSATA);
			ComRecvCmd(hCom, csTemp, str, "$");
			GetCString(str, "\n", "$", csTemp);
			csTemp.Remove('\\');
			csTemp.Remove('$');
			csTemp.Trim();
			if (csTemp.IsEmpty())
			{
				ShowUIMessage("Can not get smartctl info");
				return -1;
			}
			nTemp = atoi(csTemp);
			if (nTemp > 1500)
			{
				ShowUIMessage("Can not get power on hours");
				return -1;
			}
			for (int i = 0; i < 5; i++)
			{
				str.Empty();
				csTemp.Format("smartctl --all %s|grep \"%s\" |cut -d'-' -f 2", csSATA, csItem[i]);
				ComRecvCmd(hCom, csTemp, str, "$");
				GetCString(str, "\n", "$", csTemp);
				csTemp.Remove('\\');
				csTemp.Remove('$');
				csTemp.Trim();
				if (csTemp.IsEmpty())
				{
					ShowUIMessage("Can not get smartctl info");
					return -1;
				}
				nTemp = atoi(csTemp);
				if (nTemp > 0)
				{
					csTemp.Format("%s value:%d is out of range", csItem[i], nTemp);
					ShowUIMessage(csTemp);
					return -1;
				}
			}
			nSata++;
			if (nSata == TestIni.nSATANum)
			{
				break;
			}
		}
		str.Empty();
		csTemp = csRecv.Tokenize("\n", nPos);
	}
	if (csSATA.IsEmpty())
	{
		ShowUIMessage("No SATA Disk found");
		return -1;
	}
	if (nSata == TestIni.nSATANum)
	{
		return 0;
	}
	else
	{
		ShowUIMessage("Can not find all satas");
		return -1;
	}
}

int NetSpeed()//对网口的速度和工作模式作了检测。对10M,100M,1000M,和 半双工 ，全双工 之间的任意组合都作了测试.。。我发现DNA1160只支持 1000/full 和 100/full
{
	CString csTemp, csRecv;
	int fps = 0;//-----自己定义的变量，用来控制 while 循环的退出-------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------------
	for (int i = 0; i < TestIni.nNetPort / 2; i++)
	{
		csTemp.Format("ethtool -s eth%d speed 100 duplex full", i);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		Sleep(5000);
		fps = 1000000;
		while (-1 == csRecv.Find("NIC Link is Up"))
		{
			fps--;
			if (fps == 0)
			{
				break;
			}
		}
		ComRecvCmd(hCom, "\r", csRecv, "$");
		csRecv.Empty();
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------

	for (int i = 1; i < TestIni.nNetPort + 1; i++)
	{
		if (i == TestIni.nNetPort)//检查网口速度
		{
			csTemp.Format("ethtool eth%d|grep Speed", 0);
		}
		else
		{
			csTemp.Format("ethtool eth%d|grep Speed", i);
		}
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("100"))
		{
			csTemp.Format("eth%d can not change to 100M", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
		if (i == TestIni.nNetPort)//检查网口工作模式
		{
			csTemp.Format("ethtool eth%d|grep Duplex", 0);
		}
		else
		{
			csTemp.Format("ethtool eth%d|grep Duplex", i);
		}
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("Full"))
		{
			csTemp.Format("eth%d can not change to 100M duplex mode:full", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
		if (i == TestIni.nNetPort)
		{
			csTemp.Format("ethtool eth%d|grep Link", 0);////检查网口是否已经链接
		}
		else
		{
			csTemp.Format("ethtool eth%d|grep Link", i);
		}
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("yes"))
		{
			csTemp.Format("eth%d can not link to 100M duplex mode:full ", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------
	for (int i = 0; i < TestIni.nNetPort / 2; i++)
	{
		csTemp.Format("ethtool -s eth%d speed 1000 duplex full", i);
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		Sleep(5000);
		fps = 1000000;
		while (-1 == csRecv.Find("NIC Link is Up"))
		{
			fps--;
			if (fps == 0)
			{
				break;
			}
		}
		ComRecvCmd(hCom, "\r", csRecv, "$");
		csRecv.Empty();
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------
	for (int i = 1; i < TestIni.nNetPort + 1; i++)
	{
		if (i == TestIni.nNetPort)
		{
			csTemp.Format("ethtool eth%d|grep Speed", 0);
		}
		else
		{
			csTemp.Format("ethtool eth%d|grep Speed", i);
		}
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("1000"))
		{
			csTemp.Format("eth%d can not change to 1000M", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
		if (i == TestIni.nNetPort)
		{
			csTemp.Format("ethtool eth%d|grep Duplex", 0);
		}
		else
		{
			csTemp.Format("ethtool eth%d|grep Duplex", i);
		}
		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("Full"))
		{
			csTemp.Format("eth%d can not change to 1000M duplex mode:full", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
		if (i == TestIni.nNetPort)
		{
			csTemp.Format("ethtool eth%d|grep Link", 0);
		}
		else
		{
			csTemp.Format("ethtool eth%d|grep Link", i);
		}

		ComRecvCmd(hCom, csTemp, csRecv, "$");
		if (-1 == csRecv.Find("yes"))
		{
			csTemp.Format("eth%d can not link to 1000M duplex mode:full ", i);
			ShowUIMessage(csTemp);
			return -1;
		}
		csRecv.Empty();
	}
	return 0;
}

int Wifi()
{
	CString csSend, csRecv, csTemp, csSend0, csSend1;
	double Bandwidth = 0;
	csRecv.Empty();
	GetCString(csModel, "G", "r", csTemp);

	if (csTemp == "125W") {
		csSend.Format("ifconfig wlan0 down");
	}
	else {
		csSend.Format("ifconfig wlan1 down");
	}
	ComSendCmd(hCom, csSend);

	if (csTemp == "125W") {
		csSend.Format("wifi_setup 1-a 2.4G n20 1 %s", TestIni.csSSID);
	}
	else {
		csSend.Format("wifi_setup 2-a 2.4G n20 1 %s", TestIni.csSSID);
	}
	ComSendCmd(hCom, csSend);
	Sleep(100);
	/*CCPromptDlg WIFIDialog("Please config the wireless network of host PC", \
		csTemp, NULL, 40, BUTTON_COUNT, pParent);*/
	for (int i = 0; i < 3; i++)
	{
		CString wifi;
		wifi.Format("1. Please connect the WIFI named \"%s_2.4G\"; \n 2. Then config the IP address to \"192.168.55.11\"; \n 3. Click YES when you finished.", TestIni.csSSID);
		CCPromptDlg pPrompt_wifi("wifi", wifi, "Bin\\WIFI.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPrompt_wifi.DisplayDlg())
		{
			if (i == 2)
			{
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please config the wireless network of host PC");
		}
		else
		{
			break;
		}
	}
	csSend0.Format("ifconfig wlan0 %s", TestIni.csWifiHostIP);
	csSend1.Format("ifconfig wlan1 %s", TestIni.csWifiHostIP);

	GetCString(csModel, "G", "r", csTemp);
	if (csTemp == "125W") {
		ComRecvCmd(hCom, csSend0, csRecv, "$");//--------------设置wifi 的IP地址------------
	}
	else {
		ComRecvCmd(hCom, csSend1, csRecv, "$");//--------------设置wifi 的IP地址------------
	}
	
	Sleep(100);
	for (int i = 0; i < 3; i++)
	{
		if (TRUE == PingIP(TestIni.csWifiClientIP, 2000))
		{
			break;
		}
		if (i == 2)
		{
			ShowUIMessage("Ping failed");
			return -1;
		}
		Sleep(1000);
	}
	int i = 1;
	do {
		WinExec("Bin\\iperf.exe -s -P 10", SW_HIDE);
		Sleep(500);
		csRecv.Empty();
		csSend.Format("iperf -c %s -i %.1f -P 10 -t %d | tail -n 1", \
			TestIni.csWifiClientIP, \
			TestIni.nPeriodSec, \
			TestIni.nTransmitSec);

		ComRecvCmd(hCom, csSend, csRecv, "$");
		Sleep(500);
		WinExec("Taskkill /F /IM iperf.exe", SW_HIDE);
		GetCString(csRecv, "MBytes  ", " Mbits/sec", csTemp);
		Bandwidth = atof(csTemp);
		if (Bandwidth >= TestIni.nBandwidthLowLimit)
		{
			csTemp.Format("2.4G WIFI Bandwidth Test Value: %.1f Mbits/sec, Spec(Min: %d Mbits/sec)", Bandwidth, TestIni.nBandwidthLowLimit);
			ShowUIMessage(csTemp);
			return 0;
		}
		else
		{
			csTemp.Format("2.4G WIFI Bandwidth Test Value: %.1f Mbits/sec, out of Spec(Min: %d Mbits/sec)", Bandwidth, TestIni.nBandwidthLowLimit);
			ShowUIMessage(csTemp);
			if (i == 1)
				continue;
			else
				return -1;
		}
	} while (i--);
}

int Wifi5G()//wifi卡的IP地址要重新设置，主要是为了避免与SFP的网段冲突。然后在配置文件中，要单独设置wifi 的主机IP和客户机IP ，不能用原来已有的项，因为原来的已经被SFP占用了
{
	CString csSend, csRecv, csTemp;
	double Bandwidth = 0;

	csRecv.Empty();
	GetCString(csModel, "G", "r", csTemp);

	if (csTemp == "125W") {
		csSend.Format("ifconfig wlan0 down");
	}
	else {
		csSend.Format("ifconfig wlan1 down");
	}
	
	ComSendCmd(hCom, csSend);
	Sleep(100);
	GetCString(csModel, "G", "r", csTemp);
	if (csTemp == "125W") {
		csSend.Format("wifi_setup 1-a 5G n20 149 %s", TestIni.csSSID);
	}
	else {
		csSend.Format("wifi_setup 2-a 5G n20 149 %s", TestIni.csSSID);
	}
	
	ComSendCmd(hCom, csSend);
	Sleep(100);

	for (int i = 0; i < 3; i++)
	{
		CString wifi;
		wifi.Format("1. Please connect the WIFI named \"%s_5G\"; \n 2. Then config the IP address to \"192.168.55.11\"; \n 3. Click YES when you finished.", TestIni.csSSID);
		CCPromptDlg pPrompt_wifi("wifi", wifi, "Bin\\WIFI.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPrompt_wifi.DisplayDlg())
		{
			if (i == 2)
			{
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please config the wireless network of host PC");
		}
		else
		{
			break;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		if (TRUE == PingIP(TestIni.csWifiClientIP, 2000))
		{
			break;
		}
		if (i == 2)
		{
			ShowUIMessage("Ping failed");
			return -1;
		}
		Sleep(1000);
	}

	int i = 1;
	do {
		WinExec("Bin\\iperf.exe -s -P 10", SW_HIDE);
		Sleep(500);

		csRecv.Empty();
		//csHostIP=192.168.30.2  nPeriodSec=0.5  nBufferLengthKB=1024  nTransmitSec=5
		/*csSend.Format("iperf -c %s -i %.1f -l %dK -t %d -f m | tail -n 1", \
			TestIni.csWifiHostIP, \
			TestIni.nPeriodSec, \
			TestIni.nBufferLengthKB, \
			TestIni.nTransmitSec);*/

		csSend.Format("iperf -c %s -i %.1f -P 10 -t %d | tail -n 1", \
			TestIni.csWifiClientIP, \
			TestIni.nPeriodSec, \
			TestIni.nTransmitSec);

		ComRecvCmd(hCom, csSend, csRecv, "$");
		Sleep(500);

		WinExec("Taskkill /F /IM iperf.exe", SW_HIDE);
		GetCString(csRecv, "MBytes  ", " Mbits/sec", csTemp);

		Bandwidth = atof(csTemp);

		if (Bandwidth >= TestIni.nBandwidthLowLimit5G)
		{
			csTemp.Format("5G WIFI Bandwidth Test Value: %.1f Mbits/sec, Spec(Min: %d Mbits/sec)", Bandwidth, TestIni.nBandwidthLowLimit5G);
			ShowUIMessage(csTemp);
			return 0;
		}
		else
		{
			csTemp.Format("5G WIFI Bandwidth Test Value: %.1f Mbits/sec, out of Spec(Min: %d Mbits/sec)", Bandwidth, TestIni.nBandwidthLowLimit5G);
			ShowUIMessage(csTemp);
			if (i == 1)
				continue;
			else
				return -1;
		}
	} while (i--);

}

int CPUStress() //合格
{
	CString csSend, csRecv, csTemp;
	HWMSpec hwTemp;
	csSend.Format("stress -c 16 -i 8 -t 15");
	ComRecvCmd(hCom, csSend, csRecv, "$");
	if (-1 == csRecv.Find("successful run completed in 15s"))
	{
		ShowUIMessage("CPU can not successful run completed in 15s!");
		return -1;
	}
	csSend.Empty();
	csRecv.Empty();
	csSend.Format("DNA1160_HWM");
	ComRecvCmd(hCom, csSend, csRecv, "$");
	GetCString(csRecv, "CPU Temperature", "=", "\n", csTemp);//,要先找到 CPU Temperature ，再从 CPU Temperature 起始位置开始 根据标志截取字符串
	csTemp.Trim();
	hwTemp.CPUTemp.fval = atof(csTemp);//atof 函数 把字符串转换成浮点数
	if (hwTemp.CPUTemp.fval > TestIni.hwSpec.CPUTemp.maxval || hwTemp.CPUTemp.fval < TestIni.hwSpec.CPUTemp.minval)
	{
		csTemp.Format("CPU Temp min spec:%f,max spec:%f", TestIni.hwSpec.CPUTemp.minval, TestIni.hwSpec.CPUTemp.maxval);
		ShowUIMessage(csTemp);
		ShowUIMessage("CPU Temperature out of range");//超出范围
		return -1;
	}
	return 0;
}

int LTE_4G()
{
	CString csSend, csRecv, csTemp, csAaa;
	CString csPackets = "5";
	if ("" == ComRecvCmd(hCom, "pppd call quectel-ppp &", csRecv, "local  IP address"))
	{
		ShowUIMessage("pppd call quectel-ppp failed!");
		csRecv.Empty();
		ComRecvCmd(hCom, "killall -9 pppd", csRecv, "$");
		return -1;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "ifconfig -a|grep ppp0", csRecv, "$");
	if (-1 == csRecv.Find("Link encap:Point-to-Point Protocol"))
	{
		ShowUIMessage("pppd call quectel-ppp failed!");
		csRecv.Empty();
		ComRecvCmd(hCom, "killall -9 pppd", csRecv, "$");
		return -1;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "ifconfig ppp0|grep \"inet addr\"", csRecv, "$");
	if (-1 == csRecv.Find("inet addr:") || -1 == csRecv.Find("P-t-P:") || -1 == csRecv.Find("Mask:"))
	{
		ShowUIMessage("pppd call quectel-ppp failed!");
		csRecv.Empty();
		ComRecvCmd(hCom, "killall -9 pppd", csRecv, "$");
		return -1;
	}

	csRecv.Empty();
	ComRecvCmd(hCom, "cp -arf /etc/ppp/resolv.conf /etc", csRecv, "$");
	Sleep(500);

	csTemp.Format("ping www.google.com.tw -w %s", csPackets);
	csRecv.Empty();
	ComRecvCmd(hCom, csTemp, csRecv, "$");
	csTemp.Empty();
	//Sleep(10000);
	//csAaa = csRecv;
	csTemp.Format("%s packets transmitted, %s packets received, 0%% packet loss", csPackets, csPackets);
	//DWORD TimeStart = ::GetTickCount();// GetTickCount 函数返回自操作系统启动所经过的毫秒数
	//while (::GetTickCount() - TimeStart < 10000)
	//{
	//	
	//	csAaa += csRecv;
	//	if (-1 != csAaa.Find(csTemp))
	//	{
	//		csRecv.Empty();
	//		ComRecvCmd(hCom, "killall -9 pppd", csRecv, "$");
	//		return 0;
	//	}
	//	ComRecv(hCom, csRecv);
	//	
	//}
	if (-1 != csRecv.Find(csTemp))
	{
		csRecv.Empty();
		ComRecvCmd(hCom, "killall -9 pppd", csRecv, "$");
		return 0;
	}
	csRecv.Empty();
	ComRecvCmd(hCom, "killall -9 pppd", csRecv, "$");
	return -1;
}

int ResetButton()  //合格
{
	CString csTemp, csRecv, csSend;
	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPrompt("ResetButton", "Please push the reset button.If you have finished, click the YES button", "Bin\\DNB1160S_RESET.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPrompt.DisplayDlg())
		{
			if (i == 2)
			{
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please push the reset button");
		}
		else
		{
			break;
		}
	}
	csRecv.Empty();
	DWORD TimeStart = ::GetTickCount();// GetTickCount 函数返回自操作系统启动所经过的毫秒数
	while (ComFind(hCom, csRecv, "pci"))
	{
		ComSendCmd(hCom, "\r");
		if (::GetTickCount() - TimeStart > 80000)
		{
			ShowUIMessage("启动超时");
			return -1;
		}
	}
	DWORD TimeStart1 = ::GetTickCount();// GetTickCount 函数返回自操作系统启动所经过的毫秒数
	int i = 0;
	while (i < 60)
	{
		csRecv.Empty();
		ComSendCmd(hCom, "\r");
		if (ComFind(hCom, csRecv, "$") == 0 ) {
			return 0;
		}
		i++;
		Sleep(2000);
	}
	ShowUIMessage("找不到提示符");
	return -1;
}

int SFP() //光纤接口  //合格
{
	CString csSend, csRecv, csTemp;
	char Ctrl_C = 0x03;
	double Bandwidth = 0;
	//for (int i = 0; i < 3; i++)
	//{
	//	CCPromptDlg pPrompt("SFP", "Please insert the plug.If you have finished, click the YES button", "Bin\\DNB1160S_SFP.jpg", 0, BUTTON_OLPIC, pParent);
	//	if (BTN_YES != pPrompt.DisplayDlg())
	//	{
	//		if (i == 2)
	//		{
	//			ShowUIMessage("Operator errors!");
	//			return -1;
	//		}
	//		ShowUIMessage("Please insert the plug");
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}

	//for (int i = 0; i < 3; i++)
	//{
	//	CCPromptDlg pPrompt("SFP", "Set the IP of the host according to the file x86test.ini .If you have finished, click the YES button", "Bin\\DNB1160S_IP.jpg", 0, BUTTON_OLPIC, pParent);
	//	if (BTN_YES != pPrompt.DisplayDlg())
	//	{
	//		if (i == 2)
	//		{
	//			ShowUIMessage("Operator errors!");
	//			return -1;
	//		}
	//		ShowUIMessage("Please set HostIP");
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}

	CCPromptDlg pSFPLED1("wifiCard", "Please check if the LED is on", "Bin\\SFPLED1.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSFPLED1.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("SFP LED is error");
		return -1;
	}

	CCPromptDlg pSFPLED3("wifiLED", "Please check if the LED is on", "Bin\\SFPLED3.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSFPLED3.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("SFP LED is error!");
		return -1;
	}
	csSend.Format("ifconfig eth4 %s", TestIni.csClientIP);
	ComRecvCmd(hCom, csSend, csRecv, "$");
	csSend.Empty();
	csRecv.Empty();

	csSend.Format("ping %s  -I %s -c 6| grep packet | awk -F' ' '{print $7}'|cut -d'%%' -f1", TestIni.csHostIP, TestIni.csClientIP);
	CString lostPackets;
	ComRecvCmd(hCom, csSend, csRecv, "$");
	GetCStrings(csRecv, "\n", "\r", lostPackets, 1);
	if (lostPackets != "0")
	{
		csTemp.Format("Packet lost: %s%%.", lostPackets);
		ShowUIMessage(csTemp + "Ping failed");
		return -1;
	}
	//-----------------------------------------------------------------------------------------------------------
	csSend.Empty();
	csRecv.Empty();
	csSend.Format("ping %s  -I %s", TestIni.csHostIP, TestIni.csClientIP);
	//ComRecvCmd(hCom, csSend, csRecv, "$");
	ComSend(hCom, csSend);
	CCPromptDlg pSFPLED2("wifiLED", "Please check if the SFP LED is flashing", "Bin\\SFPLED2.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSFPLED2.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("SFP LED is error!");
		ComSend(hCom, (CString)Ctrl_C);
		return -1;
	}
	ComSend(hCom, (CString)Ctrl_C);
	//-----------------------------------------------------------------------------------------------------------
	return 0;
}

int HDMI() //合格
{
	CString csTemp, csRecv;
	ComRecvCmd(hCom, "chvt 2", csRecv, "$");//chvt 命令切换终端
	CCPromptDlg pVGA("VGA", "Please confirm whether the product display is shown in the picture", "Bin\\VGA.jpg", 0, BUTTON_OLPIC, pParent);
	if (pVGA.DisplayDlg() != BTN_YES)
	{
		ComSendCmd(hCom, "chvt 1");
		return -1;
	}
	ComRecvCmd(hCom, "chvt 1", csRecv, "$");
	return 0;
}

int MicroUSB()
{
	CString csTemp, csRecv, csSend;
	CloseCom(hCom); //关闭串口 (此处关闭的是RJ45)
	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPrompt("MicroUSB", "Please insert the plug.If you have finished, click the YES button", "Bin\\DNB1160S_MicroUSB.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPrompt.DisplayDlg())
		{
			if (i == 2)
			{
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please insert the plug");
		}
		else
		{
			break;
		}
	}
	Sleep(1000);

	char cPID[16];
	sprintf(cPID, "*%s*", TestIni.strMicroUSBPID);
	for (int i = 0; i < 3; i++)
	{
		nComPort = GetComNum(cPID);
		csTemp.Format("nComPort=%d", nComPort);
		ShowUIMessage(csTemp);
		hCom = InitCom(nComPort, 115200);
		Sleep(500);
		if (NULL == hCom)
		{
			if (i == 2)
			{
				ShowUIMessage("COM Open Fail!");
				return -1;
			}
		}
		else
		{
			break;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		csRecv.Empty();
		if ("" != ComRecvCmd(hCom, "\r", csRecv, "$"))//
		{
			return 0;
		}
		Sleep(500);
	}
	return -1;
}

int GetComNum(const char* ProductID)
{
	//AfxMessageBox("无法获取串口，请确认是否安装并连接串口!");
	CString Aaa;
	int SysBits = 0;
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_AMD_X8664 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		SysBits = 64;
	}
	else
	{
		SysBits = 32;
	}

	HKEY hKey = NULL;
	long ret = 0;

	LPCTSTR data = LPCTSTR("SYSTEM\\CurrentControlSet\\Control\\COM Name Arbiter");
	if (64 == SysBits)
	{
		ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, data, 0, KEY_WOW64_64KEY | KEY_ALL_ACCESS, &hKey);
	}
	else if (32 == SysBits)
	{
		ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, data, 0, KEY_ALL_ACCESS, &hKey);
	}

	if (ret != ERROR_SUCCESS)
	{
		//--------------------------------------------------------------------------------
		Aaa.Format("ret=%d", ret);
		ShowUIMessage(Aaa);
		//--------------------------------------------------------------------------------
		return -1;
	}
	ret = ::RegDeleteValue(hKey, "ComDB");
	::RegCloseKey(hKey);

	CString csTemp;
	CFile fp;
	char buffer[1024];
	if (64 == SysBits)
	{
		csTemp.Format("CMD /C \"Bin\\devcon_x64.exe find %s >Log\\dutdetected.txt\"", ProductID);
	}
	else if (32 == SysBits)
	{
		csTemp.Format("CMD /C \"Bin\\devcon_x86.exe find %s >Log\\dutdetected.txt\"", ProductID);//启动设备管理器
	}
	WinExec(csTemp.GetBuffer(), SW_HIDE);
	Sleep(1000);
	if (fp.Open("Log\\dutdetected.txt", CFile::modeRead) != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		fp.Read(buffer, sizeof(buffer));
		fp.Close();
		csTemp.Format("%s", buffer);
		if (-1 == csTemp.Find("matching device(s) found."))
		{
			ShowUIMessage("matching device(s) not found.");//---------
			return -1;
		}
	}

	int nPos = 0;
	CString csCom[3];
	int nComPort = 0;
	for (int i = 0; i < 3; i++)
	{
		csCom[i].Empty();
		if (nPos < 0)
		{
			break;
		}
		csCom[i] = csTemp.Tokenize("()", nPos);
		csCom[i].Trim();
	}
	//csTemp.Format("Com port %s founded!\n", csCom[1]);
	//ShowUIMessage(csTemp);

	csCom[1].Delete(0, 3);
	nComPort = atoi(csCom[1]);
	return nComPort;
}

int WriteI211()
{
	CString csSend, csRecv, csTemp;
	for (int i = 1; i < 5; i++)
	{
		csSend.Format("eeupdate64e /NIC=%d /INVMUPDATE /FILE=/usr/share/I211_DNB1160.txt", i);
		ComRecvCmd(hCom, csSend, csRecv, "$");
		if (-1 == csRecv.Find("... done."))
		{
			csTemp.Format("NIC%d fail", i);
			ShowUIMessage(csTemp);
			return -1;
		}
	}
	return 0;
}

int PCIE()
{
	CString csSend, csRecv, csTemp;
	char Ctrl_C = 0x03;
	ComRecvCmd(hCom, "lspci -s 09:00.0", csRecv, "$");

	// wifi
	if (station_flag == "FT") {
		if (csModel == "NSK3300_MPCIE_LTE7455NA" || csModel == "NSK3300_MPCIE_LTE7430APAC_KIT") {
			// LTE 4G
			csRecv.Empty();
			if (TestIni.csLTECardID != "skip")
			{
				ComRecvCmd(hCom, "lsusb", csRecv, "$");
				if (-1 == csRecv.Find(TestIni.csLTECardID))
				{
					ShowUIMessage("Can not find LTECardID!");
					return -1;
				}
				else
				{
					csTemp.Format("LTECardID:%s", TestIni.csLTECardID);
					ShowUIMessage(csTemp);
				}
			}
		}
		else {
			if (TestIni.csWifiCard1ID != "skip")
			{
				if (-1 == csRecv.Find(TestIni.csWifiCard1ID))
				{
					ShowUIMessage("Can not find WifiCard1ID!");
					return -1;
				}
				else
				{
					csTemp.Format("WifiCard1ID:%s", TestIni.csWifiCard1ID);
					ShowUIMessage(csTemp);
				}
			}
			if (TestIni.csWifiCard2ID != "skip")
			{
				csRecv.Empty();
				ComRecvCmd(hCom, "lspci -s 08:00.0", csRecv, "$");
				if (-1 == csRecv.Find(TestIni.csWifiCard2ID))
				{
					ShowUIMessage("Can not find WifiCard2ID!");
					return -1;
				}
				else
				{
					csTemp.Format("WifiCard2ID:%s", TestIni.csWifiCard2ID);
					ShowUIMessage(csTemp);
				}
			}
		}
	}
	else {// PT
		csRecv.Empty();
		if (csModel == "NSK3300_MPCIE_S") {//if NSK3300_MPCIE_S wifi
			if (TestIni.csWifiCard1ID != "skip")
			{
				if (-1 == csRecv.Find(TestIni.csWifiCard1ID))
				{
					ShowUIMessage("Can not find WifiCard1ID!");
					return -1;
				}
				else
				{
					csTemp.Format("WifiCard1ID:%s", TestIni.csWifiCard1ID);
					ShowUIMessage(csTemp);
				}
			}
			if (TestIni.csWifiCard2ID != "skip")
			{
				csRecv.Empty();
				ComRecvCmd(hCom, "lspci -s 08:00.0", csRecv, "$");
				if (-1 == csRecv.Find(TestIni.csWifiCard2ID))
				{
					ShowUIMessage("Can not find WifiCard2ID!");
					return -1;
				}
				else
				{
					csTemp.Format("WifiCard2ID:%s", TestIni.csWifiCard2ID);
					ShowUIMessage(csTemp);
				}
			}
		}
		else {//LTE
			if (TestIni.csLTECardID != "skip")
			{
				ComRecvCmd(hCom, "lsusb", csRecv, "$");
				if (-1 == csRecv.Find(TestIni.csLTECardID))
				{
					ShowUIMessage("Can not find LTECardID!");
					return -1;
				}
				else
				{
					csTemp.Format("LTECardID:%s", TestIni.csLTECardID);
					ShowUIMessage(csTemp);
				}
			}
		}
	}
	csRecv.Empty();
	if (TestIni.csIMEI != "skip")
	{
		ComRecvCmd(hCom, "insmod /usr/share/GobiNet.ko", csRecv, "$");
		Sleep(500);
		ComRecvCmd(hCom, "insmod /usr/share/GobiSerial.ko", csRecv, "$");
		Sleep(500);
		
		//ComRecvCmd(hCom, "echo AT+CIMI > /dev/ttyUSB2;cat /dev/ttyUSB2 &\r", csRecv, "$");
		/*ComSendCmd(hCom, "echo AT+CIMI > /dev/ttyUSB2");
		ComRecvCmd(hCom, "cat /dev/ttyUSB2 &\r", csRecv, "$");*/
		
		ComRecvCmd(hCom, "echo AT+CIMI > /dev/ttyUSB2", csRecv, "$");
		ComSendCmd(hCom, "cat /dev/ttyUSB2 > /tmp/lte.log &");
		ComRecvCmd(hCom, "cat /tmp/lte.log", csRecv, "$");

		ComSendCmd(hCom, "\r");
		ComSendCmd(hCom, "killall -9 cat");
		ComSendCmd(hCom, "\r");
		if (-1 == csRecv.Find(TestIni.csIMEI))
		{
			ShowUIMessage("Can not find IMEI!");
			return -1;
		}
		else
		{
			csTemp.Format("IMEI:%s", TestIni.csIMEI);
			ShowUIMessage(csTemp);
		}
	}
	return 0;
}

int M2_ssd()
{
	CString csTemp, csRecv, str, csCF, csSend;
	//int nPos = 0;
	//ComRecvCmd(hCom, "sfdisk -s|grep /dev/|grep -v lv|awk -F':' '{print $1}'", csRecv, "$");//sfdisk -s 命令显示分区的大小
	//csTemp = csRecv.Tokenize("\n", nPos);
	//while (!csTemp.IsEmpty())
	//{
	//	if (-1 != csTemp.Find("sd"))
	//	{
	//		csCF.Format("smartctl -i %s", csTemp);//smartctl -i 显示指定的硬盘设备的参数信息
	//		ComRecvCmd(hCom, csCF, str, "$");
	//		if (-1 != str.Find("ATA Version") && -1 != str.Find("Solid State Device"))  //信息显示为固态设备
	//		{
	//			csCF = csTemp;
	//			csCF.Remove('\r');
	//			csCF.Remove('\n');
	//			break;
	//		}
	//	}
	//	csTemp = csRecv.Tokenize("\n", nPos);
	//}
	//if (csCF.IsEmpty() || -1 == csCF.Find("sd"))
	//{
	//	ShowUIMessage("No M2 SSD Found!");
	//	return -1;
	//}
	//csRecv.Empty();
	//csSend.Empty();
	//csSend.Format("mkfs.vfat %s", csCF);
	//ComRecvCmd(hCom, csSend, csRecv, "$");
	//csSend.Empty();
	//csRecv.Empty();
	//ComRecvCmd(hCom, "ls /mnt", csRecv, "$");
	//if (-1 == csRecv.Find("ssd"))
	//{
	//	csRecv.Empty();
	//	ComRecvCmd(hCom, "mkdir /mnt/ssd", csRecv, "$");
	//}
	//csRecv.Empty();
	//csTemp.Format("mount \"%s\" \/mnt\/ssd", csCF);
	//ComRecvCmd(hCom, csTemp, str, "$");
	//str.Empty();
	//ComRecvCmd(hCom, "mount", str, "$");
	//if (-1 != str.Find(csCF))
	//{
	//	ComRecvCmd(hCom, "rm -rf /mnt/ssd/test", str, "$");//rm-rf表示删除文件，而且可以删除非空目录。-rf参数表示递归强制删除。
	//	str.Empty();
	//	ComRecvCmd(hCom, "test -e /mnt/ssd/test", str, "$");//test -e 如果文件存在则返回 0
	//	str.Empty();
	//	ComRecvCmd(hCom, "echo $?", str, "$");
	//	if (-1 != str.Find("0"))
	//	{
	//		ShowUIMessage("Can not delete test file");
	//		return -1;
	//	}
	//	str.Empty();
	//	ComRecvCmd(hCom, "dd if=/dev/zero of=/tmp/test count=1M bs=1", str, "$");//将 /dev/zero 拷到  /tmp/test
	//	str.Empty();
	//	ComRecvCmd(hCom, "cp -arf /tmp/test /mnt/ssd", str, "$");
	//	str.Empty();
	//	ComRecvCmd(hCom, "test -e /mnt/ssd/test", str, "$");
	//	str.Empty();
	//	ComRecvCmd(hCom, "echo $?", str, "$");
	//	if (-1 == str.Find("0"))
	//	{
	//		ShowUIMessage("Can not copy test file to M2 SSD");
	//		return -1;
	//	}
	//	str.Empty();
	//	ComRecvCmd(hCom, "cp -arf /mnt/ssd/test /tmp/test1", str, "$");
	//	str.Empty();
	//	ComRecvCmd(hCom, "test -e /tmp/test1", str, "$");
	//	str.Empty();
	//	ComRecvCmd(hCom, "echo $?", str, "$");
	//	if (-1 == str.Find("0"))
	//	{
	//		ShowUIMessage("Can not read file from M2 SSD");
	//		return -1;
	//	}
	//	str.Empty();
	//	ComRecvCmd(hCom, "cmp -l /tmp/test /tmp/test1", str, "$");
	//	str.Empty();
	//	ComRecvCmd(hCom, "echo $?", str, "$");
	//	if (-1 == str.Find("0"))
	//	{
	//		ShowUIMessage("File copied not same with orginal file");
	//		return -1;
	//	}
	//	return 0;
	//}
	ComRecvCmd(hCom, "smartctl -i /dev/sda | awk -F':' '/Device Model:/ {print $2}'", csRecv, "$");
	CString res;
	int i = 1;
	while (GetCStrings(TestIni.SSDModel, "<", ">", res, i++))
	{
		if (-1 != csRecv.Find(res))
		{
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			csTemp.Format("SSD model:%s", csRecv);
			ShowUIMessage(csTemp);

			// TODO smart
			ComRecvCmd(hCom, "smartctl -a /dev/sda | awk '/Power_On_Hours/ {print $NF}'", csRecv, "$");
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			if (TestIni.hwSpec.smart.Power_On_Hours < atof(csRecv))
			{
				ShowUIMessage("SMART FAIL");
				return -1;
			}
			ComRecvCmd(hCom, "smartctl -a /dev/sda | awk '/Reallocated_Sector_Ct/ {print $NF}'", csRecv, "$");
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			if (TestIni.hwSpec.smart.Reallocated_Sector_Ct < atof(csRecv))
			{
				ShowUIMessage("SMART FAIL");
				return -1;
			}
			ComRecvCmd(hCom, "smartctl -a /dev/sda | awk '/Reallocated_Event_Count/ {print $NF}'", csRecv, "$");
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			if (TestIni.hwSpec.smart.Reallocated_Event_Count < atof(csRecv))
			{
				ShowUIMessage("SMART FAIL");
				return -1;
			}
			ComRecvCmd(hCom, "smartctl -a /dev/sda | awk '/Current_Pending_Sector/ {print $NF}'", csRecv, "$");
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			if (TestIni.hwSpec.smart.Current_Pending_Sector < atof(csRecv))
			{
				ShowUIMessage("SMART FAIL");
				return -1;
			}
			ComRecvCmd(hCom, "smartctl -a /dev/sda | awk '/Offline_Uncorrectable/ {print $NF}'", csRecv, "$");
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			if (TestIni.hwSpec.smart.Offline_Uncorrectable < atof(csRecv))
			{
				ShowUIMessage("SMART FAIL");
				return -1;
			}
			ComRecvCmd(hCom, "smartctl -a /dev/sda | awk '/UDMA_CRC_Error_Count/ {print $NF}'", csRecv, "$");
			GetCString(csRecv, "\r\n", "\r\n", csRecv);
			if (TestIni.hwSpec.smart.UDMA_CRC_Error_Count < atof(csRecv))
			{
				ShowUIMessage("SMART FAIL");
				return -1;
			}
			return 0;
		}
	}
	ShowUIMessage("SSD model error");
	return -1;
}

int FAN()
{
	CString csTemp, csRecv, csHWM, csSend, csAaa, csBbb;
	HWMSpec hwVal = {};
	//---------------------------------------------------------------------------------------------------------------------
	int nPos = 0, nUSB = 0;
	int USB_quantity = 0;//该变量表示检测到的USB 的数目
	CString csUSB[6], str;
	ComRecvCmd(hCom, "sfdisk -s|grep /dev/|grep -v lv|awk -F':' '{print $1}'", csRecv, "$");//sfdisk -s 命令显示分区的大小
	csTemp = csRecv.Tokenize("\n", nPos);  //grep -v 显示没有搜寻的字符的那一行   awk -F 以后面的字符作为分隔符
	while (!csTemp.IsEmpty())
	{
		if (-1 != csTemp.Find("sd"))
		{
			csTemp.Remove('\r');
			csTemp.Remove('\n');
			csTemp += "1";//-----------------------------------------------------------------------------------------
			csUSB[nUSB].Format("smartctl -i %s", csTemp);//smartctl -i 显示指定的硬盘设备的参数信息
			ComRecvCmd(hCom, csUSB[nUSB], str, "$");
			if (((-1 != str.Find("Device type:          disk")) && (-1 == str.Find("SATA"))) || ((-1 != str.Find("USB")) && (-1 == str.Find("SATA"))) || ((-1 != str.Find("Generic")) && (-1 == str.Find("SATA"))))
			{
				csUSB[nUSB] = csTemp;
				nUSB++;
			}
			else
			{
				csUSB[nUSB] = "";
			}
		}
		str.Empty();
		csTemp = csRecv.Tokenize("\n", nPos);
	}
	for (int i = 0; i < 6; i++)
	{
		if (!csUSB[i].IsEmpty())
		{
			ComRecvCmd(hCom, "ls /mnt", csBbb, "$");
			if (-1 == csBbb.Find("usb"))
			{
				ComRecvCmd(hCom, "mkdir \/mnt\/usb", csBbb, "$");
			}
			csSend.Empty();
			csBbb.Empty();
			csRecv.Format("mount %s /mnt/usb", csUSB[i]);
			ComRecvCmd(hCom, csRecv, csBbb, "$");
			csBbb.Empty();
			ComRecvCmd(hCom, "mount", csBbb, "$");
			if (-1 == csBbb.Find(csUSB[i]))
			{
				ShowUIMessage("挂载U盘失败");
				return -1;
			}
			csSend.Empty();
			csBbb.Empty();
			csSend.Format("ls /mnt/usb");
			ComRecvCmd(hCom, csSend, csBbb, "$");
			if (-1 == csBbb.Find("DNA1160_HWM"))
			{
				continue;;
			}
			else
			{
				csBbb.Empty();
				ComRecvCmd(hCom, "cd /mnt/usb", csBbb, "$");
				break;
			}
		}
	}
	csHWM.Format("./DNA1160_HWM 1 100");
	ComRecvCmd(hCom, csHWM, csRecv, "$");
	csHWM.Format("./DNA1160_HWM");
	ComRecvCmd(hCom, csHWM, csRecv, "$");
	GetCString(csRecv, "System FAN Speed", "=", "RPM", csTemp);
	csTemp.Trim();
	csAaa.Format("System FAN full Speed = %s RPM", csTemp);
	ShowUIMessage(csAaa);
	hwVal.SystemFANFullSpeed.fval = atof(csTemp);
	csBbb.Empty();
	ComRecvCmd(hCom, "cd /", csBbb, "$");
	csBbb.Empty();
	ComRecvCmd(hCom, "umount /mnt/usb", csBbb, "$");
	csBbb.Empty();
	ComRecvCmd(hCom, "mount", csBbb, "$");
	if (hwVal.SystemFANFullSpeed.fval > TestIni.hwSpec.SystemFANFullSpeed.maxval || hwVal.SystemFANFullSpeed.fval < TestIni.hwSpec.SystemFANFullSpeed.minval)
	{
		ShowUIMessage("System FAN full Speed out of range");
		return -1;
	}
	return 0;
}

int CheckCPLD()
{
	CString csTemp, csRecv, csSend, csAaa, csBbb, csCcc;
	ComRecvCmd(hCom, "cpldget 0xfd", csRecv, "$");
	GetCString(csRecv, ":", ":", "\r", csTemp);
	csAaa = csTemp.Mid(3, 1);
	csRecv.Empty();
	csTemp.Empty();
	ComRecvCmd(hCom, "cpldget 0xf1", csRecv, "$");
	GetCString(csRecv, ":", ":", "\r", csTemp);
	csBbb = csTemp.Mid(3, 1);
	csCcc = "0x" + csAaa + csBbb;
	if (csCcc.Compare(TestIni.csCPLDRev))
	{
		ShowUIMessage("CPLD Version error");
		return -1;
	}
	return 0;
}

int LED()
{
	CString csTemp, csRecv, csSend, img, cmd0, cmd1, cmd2, cmd3;
	int speed;

	//LAN LED----------------------------------------------------------------------------------------------------------------------------------
	/*for (int i = 0; i < 12; ++i) {
	cmd.Format("ethtool -s eth%d speed %d autoneg on duplex full", i / 3, (int)pow(10, ((i % 3) + 1)));
	ComSendCmd(hCom, cmd);
	Sleep(500);
	csTemp.Format("Plesase check the speed of eth%d is %dM", i / 3, (int)pow(10, ((i % 3) + 1)));
	img.Format("Bin\\LAN_LED%d.jpg", i);
	CCPromptDlg LAN_LED("LED", csTemp, img, 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != LAN_LED.DisplayDlg())
	{
	m_errCode = LED_FAIL;
	ShowUIMessage("POWER_GRE_LED FAIL!");
	return -1;
	}
	}*/
	if (csModel != "NSK3300_MPCIE_LTE7455NA" || csModel != "NSK3300_MPCIE_LTE7430APAC_KIT" || csModel != "NSK3300_MPCIE_S") {
		for (int i = 0; i < 3; ++i) {
			speed = (int)pow(10, ((i % 3) + 1));
			cmd0.Format("ethtool -s eth%d speed %d autoneg on duplex full&", 0, speed);
			cmd1.Format("ethtool -s eth%d speed %d autoneg on duplex full&", 1, speed);
			cmd2.Format("ethtool -s eth%d speed %d autoneg on duplex full&", 2, speed);
			cmd3.Format("ethtool -s eth%d speed %d autoneg on duplex full&", 3, speed);
			ComSendCmd(hCom, cmd0);
			ComSendCmd(hCom, cmd1);
			ComSendCmd(hCom, cmd2);
			ComSendCmd(hCom, cmd3);
			csTemp.Format("Plesase check the speed of eth%d ~ eth%d is %dM", 0, 3, speed);

			img.Format("Bin\\LANLED%d.jpg", i);
			CCPromptDlg LAN_LED("LED", csTemp, img, 0, BUTTON_OLPIC, pParent);
			if (BTN_YES != LAN_LED.DisplayDlg())
			{
				m_errCode = LED_FAIL;
				ShowUIMessage("POWER_GRE_LED FAIL!");
				return -1;
			}
		}
	}
	
	//ethtool - s eth0 speed 1000 autoneg on duplex full
	//	ethtool - s eth0 speed 100 autoneg on duplex full
	//	ethtool - s eth0 speed 10 autoneg on duplex full

	//Power LED--------------------------------------------------------------------------------------------------------------------------
	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPOWER1("power", "Please plug in the two power cords.Click YES when you finished.", "Bin\\power0.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPOWER1.DisplayDlg())
		{
			if (i == 2)
			{
				m_errCode = LED_TEST_FAIL;
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please plug in the two power cords.");
		}
		else
		{
			break;
		}
	}
	//------插上两个电源------
	CCPromptDlg pPOWERLED1("powerLED", "Please check if the two LED are all green", "Bin\\power0LED.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pPOWERLED1.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("Power LED is error!");
		return -1;
	}

	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPOWER2("power", "Please unplug one of the power supplies.Click YES when you finished.", "Bin\\power1.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPOWER2.DisplayDlg())
		{
			if (i == 2)
			{
				m_errCode = LED_TEST_FAIL;
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please unplug one of the power supplies.");
		}
		else
		{
			break;
		}
	}//------插上1号电源------内侧为1号电源------
	CCPromptDlg pPOWERLED2("powerLED", "Please check if one LED is green and the other LED is red", "Bin\\power1LED.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pPOWERLED2.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("Power LED is error!");
		return -1;
	}

	for (int i = 0; i < 3; i++)
	{
		CCPromptDlg pPOWER3("power", "Please plug in the power supply first and then unplug the other one power..Click YES when you finished.", "Bin\\power2.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pPOWER3.DisplayDlg())
		{
			if (i == 2)
			{
				m_errCode = LED_TEST_FAIL;
				ShowUIMessage("Operator errors!");
				return -1;
			}
			ShowUIMessage("Please plug in the power supply first and then unplug the other one power.");
		}
		else
		{
			break;
		}
	}//------插上2号电源------外侧为2号电源------
	CCPromptDlg pPOWERLED3("powerLED", "Please check if one LED is green and the other LED is red", "Bin\\power2LED.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pPOWERLED3.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("Power LED is error!");
		return -1;
	}

	//wifi LED-----------------------------------------------------------------------------------------------------------------------------
	if (TestIni.wifiLEDEnable)
	{
		CCPromptDlg pWIFICARD("wifiCard", "Please check if there is such a wifi card ", "Bin\\wifiCard.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pWIFICARD.DisplayDlg())
		{
			m_errCode = LED_TEST_FAIL;
			ShowUIMessage("There is no one wifi card");
			return -1;
		}
		CCPromptDlg pWIFILED("wifiLED", "Please check if the LED is on", "Bin\\wifiLED.jpg", 0, BUTTON_OLPIC, pParent);
		if (BTN_YES != pWIFILED.DisplayDlg())
		{
			m_errCode = LED_TEST_FAIL;
			ShowUIMessage("WIFI LED is error!");
			return -1;
		}
	}

	//SSD LED----------------------------------------------------------------------------------------------------------------------------------
	CCPromptDlg pSSDCARD("ssdCard", "Please check if there is such a SSD ", "Bin\\ssdCard.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSSDCARD.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("There is no one SSD");
		return -1;
	}
	csRecv.Empty();
	ComSend(hCom, "fio -filename=/dev/sda -direct=1 -iodepth=1 -thread -rw=randread -bs=512k -numjobs=8 -runtime=600 -group_reporting -name=nxtest &");//发送读写SSD的指令

	CCPromptDlg pSSDLED("ssdLED", "Please check if the LED is on", "Bin\\ssdLED.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSSDLED.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("SSD LED is error!");
		ComRecvCmd(hCom, "killall -s 9 fio", csRecv, "$");//
		return -1;
	}

	ComRecvCmd(hCom, "killall -s 9 fio", csRecv, "$");//

	//status LED------------------------------------------------------------------------------------------------------------------------------
	ComRecvCmd(hCom, "DNA130S_LED 2", csRecv, "$");
	CCPromptDlg pSTATUSLED1("StatusLED", "Please check if the LED is red", "Bin\\statusLED2.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSTATUSLED1.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("Status LED is error!");
		return -1;
	}

	ComRecvCmd(hCom, "DNA130S_LED 3", csRecv, "$");
	CCPromptDlg pSTATUSLED2("StatusLED", "Please check if the LED is green", "Bin\\statusLED1.jpg", 0, BUTTON_OLPIC, pParent);
	if (BTN_YES != pSTATUSLED2.DisplayDlg())
	{
		m_errCode = LED_TEST_FAIL;
		ShowUIMessage("Status LED is error!");
		return -1;
	}

	
	return 0;
}

int CheckDMI()
{
	CString csTemp, csRecv, csSend;
	CString csManufacturer, csProductName, csVersion, csSerialNumber;
	ComRecvCmd(hCom, "dmidecode -t 1", csRecv, "$");
	GetCString(csRecv, "Manufacturer:", "\n", csTemp);//Manufacturer----------------
	csManufacturer = csTemp.Trim();
	if (csManufacturer != TestIni.Manufacturer)
	{
		ShowUIMessage("Manufacturer error");
		return -1;
	}
	else
	{
		csTemp.Format("Manufacturer:%s", csManufacturer);
		ShowUIMessage(csTemp);
	}

	csTemp.Empty();
	GetCString(csRecv, "Product Name:", "\n", csTemp);//ProductName-------------
	csProductName = csTemp.Trim();
	if (csProductName != TestIni.ProductName)
	{
		ShowUIMessage("Product Name error");
		return -1;
	}
	else
	{
		csTemp.Format("Product Name:%s", csProductName);
		ShowUIMessage(csTemp);
	}

	csTemp.Empty();
	GetCString(csRecv, "Version:", "\n", csTemp);//Version--------------------
	csVersion = csTemp.Trim();
	if (csVersion != TestIni.Version)
	{
		ShowUIMessage("Version error");
		return -1;
	}
	else
	{
		csTemp.Format("Version:%s", csVersion);
		ShowUIMessage(csTemp);
	}

	//csSN
	csTemp.Empty();
	GetCString(csRecv, "Serial Number:", "\n", csTemp);//Serial Number--------------------
	csSerialNumber = csTemp.Trim();
	if (csSerialNumber != csSN)
	{
		ShowUIMessage("Serial Number error");
		return -1;
	}
	else
	{
		csTemp.Format("Serial Number:%s", csSerialNumber);
		ShowUIMessage(csTemp);
	}

	return 0;
}