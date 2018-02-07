// CyberTanATS.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CATSApp:
// See CyberTanATS.cpp for the implementation of this class
//

class CATSApp : public CWinApp
{
public:
	CATSApp();

// Overrides
	public:
	virtual BOOL InitInstance();
	VOID GetAppVersion();

	CString				m_AppVersionString;
	VS_FIXEDFILEINFO	m_AppVersionInfo;

// Implementation
private:
	
	DECLARE_MESSAGE_MAP()
};

extern CATSApp theApp;