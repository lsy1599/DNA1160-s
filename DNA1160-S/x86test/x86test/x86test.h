// x86test.h : x86test DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

#define MAX_NET_PORT 128


// Cx86testApp
// �йش���ʵ�ֵ���Ϣ������� x86test.cpp
//

class Cx86testApp : public CWinApp
{
public:
	Cx86testApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
#define	 NOITEM		"N/A"
#define MAXTESTCOUNT 33
typedef int (*pFunc)(void);

typedef struct _Test_List_
{
	CString TestName;
	pFunc TestFunc;
	CString TestError;
	int TestRetry;
	int nRetryTimes;
}TestList, *pTestList;
typedef struct Range
{
	double maxval;
	double minval;
	double fval;
}volRange;

typedef struct SMART
{
	double Power_On_Hours;
	double Reallocated_Sector_Ct;
	double Reallocated_Event_Count;
	double Current_Pending_Sector;
	double Offline_Uncorrectable;
	double UDMA_CRC_Error_Count;
	
}Smart;
typedef struct HWM
{
	volRange SYSTemp2;
	volRange CPUTemp;
	volRange SYSTemp;
	volRange FANRpm;
	volRange CPUVcore;
	//volRange Vol1P35;
	volRange VolDDR;
	volRange Vol3P3;
	volRange Vol5;
	volRange Vol12;
	volRange VolVSB;
	volRange SystemFAN;
	volRange CPUFAN;
	volRange SystemFANFullSpeed;
	volRange MemSize;
	Smart	 smart;
}HWMSpec;

typedef struct _TEST_ITEM_LIST
{
	int TestCount;
	TestList TestItem[MAXTESTCOUNT];
}ROUTE_TestList;
typedef struct _IPERF_WIRE_IPERF
{
	CString csIperfPacket;
	double dIperfTotalTime;
	double dIperfCutTime;
	double dIperfCatTime;
	int nIperfWay;
	double dIperfSpec;
	int nRetryCount;
	unsigned long uIperfPktNum;
}IPERF;

typedef struct _LOT_TEST_INI
{
	CString csWifiCard1ID, csLTECardID,csWifiCard2ID,csIMEI;
	CString csDutIP,csNetIP;
	CString csFWVersion,csLogin,csPasswd,csUbootVer,csPcbVer,csCPLDVer,csCPU,csCPUVer,csMCUVer;
	int nCom, nRetryTimes, nPacketNum, nPOEPow, nUSBNum, nNetPort, nSATANum;
	int nCountDownSec;//tree MD
	HWMSpec hwSpec;
	CString csSSID, csHostIP, csClientIP,csWifiHostIP,csWifiClientIP;
	int nBufferLengthKB, nTransmitSec, nBandwidthLowLimit, nBandwidthLowLimit5G, nBandwidthLowLimitSFP;
	double nPeriodSec;
	CString strMicroUSBPID, strUSB2RJ45PID, strRS232RJ45;
	CString csCPUType,csBIOSRev,csCPLDRev;
	CString nMemSize;
	CString Manufacturer, ProductName, Version, SerialNumber;
	int MemSizePT, MemSizeFT;
	int RTCErrorRange;
	int wifiLEDEnable;
	CString SSDModel;
}ROUTER_TEST_INI,*PROUTER_TEST_INI;

void LoadIni(void);
void CreateLogTitle(void);
bool RegisterTestItem(void);