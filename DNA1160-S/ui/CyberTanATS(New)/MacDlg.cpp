// MacDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CyberTanATS.h"
#include "CyberTanATSDlg.h"
#include "MacDlg.h"
#include "Comm.h"
#include "CModelSel.h"
//#include "SFIS.h"
CString modelname;
extern CString PT_FT;
// CMacDlg dialog
extern ATSCONFIG AtsInfo;
//extern SFIS_PM	SFIS;//--------------赻撩氝樓腔測鎢

IMPLEMENT_DYNAMIC(CMacDlg, CDialog)

CMacDlg::CMacDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMacDlg::IDD, pParent)
{
	m_bMacLegal = false;
}

CMacDlg::~CMacDlg()
{

}

void CMacDlg::Create(HWND hWnd)
{
	m_NotifyhWnd = hWnd;
}

void CMacDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MAC, m_EDIT_mac);//mac begin
	DDX_Control(pDX, IDC_EDIT_SN, m_EDIT_sn);
	DDX_Control(pDX, IDC_EDIT_SSN1, m_EDIT_ssn1);//mac end
	DDX_Control(pDX, IDC_EDIT_SSN2, m_EDIT_ssn2);
}


BEGIN_MESSAGE_MAP(CMacDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CMacDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CMacDlg::OnBnClickedOk)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_EDIT_MAC, &CMacDlg::OnEnChangeEditMac)
	ON_EN_CHANGE(IDC_EDIT_SSN1, &CMacDlg::OnEnChangeEditSsn1)
	ON_EN_CHANGE(IDC_EDIT_SN, &CMacDlg::OnEnChangeEditSn)
END_MESSAGE_MAP()


// CMacDlg message handlers

void CMacDlg::PreInitDialog()
{
	CDialog::PreInitDialog();
}

BOOL CMacDlg::CheckMac()
{
	CString msg;
	m_bMacLegal = false;
	short len = m_csMac.GetLength();
	if (len == 12)
	{
		for (int i = 0; i < 12; i++)
		{
			if (iswxdigit(int(m_csMac[+i])) == 0)
			{
				msg.Format("Mac address:\"%s\", '%c' format is illegal.", m_csMac, m_csMac[i]);
				m_EDIT_mac.SetFocus();
				m_EDIT_mac.SetSel(0, m_csMac.GetLength());
				goto MAC_Err;
			}
			else
				m_bMacLegal = true;
		}
		return true;
	}
	else if (len == 17)
	{
		for (int i = 0; i < 17; i++)
		{
			//			AfxMessageBox((m_csMac.GetBuffer()) + i , MB_OK, 0);
			if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
			{
				if (m_csMac[i] == ':')
				{
					//					AfxMessageBox((m_csMac.GetBuffer()) + i, MB_OK, 0);
					continue;
				}
				else
				{
					//					msg.Format("Mac address:\"%s\", '%c' format is illegal.", m_csMac, m_csMac[i]);
					m_EDIT_mac.SetFocus();
					m_EDIT_mac.SetSel(0, m_csMac.GetLength());
					goto MAC_Err;
				}
			}
			if (iswxdigit(int(m_csMac[+i])) == 0)
			{
				//				msg.Format("Mac address:\"%s\", '%c' format is illegal.", m_csMac, m_csMac[i]);
				m_EDIT_mac.SetFocus();
				m_EDIT_mac.SetSel(0, m_csMac.GetLength());
				goto MAC_Err;
			}
			else
				m_bMacLegal = true;
		}
		return true;

	}
MAC_Err:

	//		msg.Format("Mac address:\"%s\", length is %d, <> 12.\r\n", m_csMac, len);
	m_EDIT_mac.SetFocus();
	m_EDIT_mac.SetSel(0, m_csMac.GetLength());
	msg.Format("MACBegㄩBarcode Wrong");
	ShowActionMessage(msg);
	m_bMacLegal = false;
	return false;



	//if ( m_csMac.CompareNoCase(AtsInfo.m_csMacStart) < 0 )
	//{		
	//	msg.Format("Mac overrange [%s ~ %s]",AtsInfo.m_csMacStart, AtsInfo.m_csMacEnd);
	//	//MessageBox(msg,"ERROR",MB_OK);
	//	m_EDIT_mac.SetFocus();
	//	m_EDIT_mac.SetSel(0,m_csMac.GetLength());
	//	return false;
	//}

	//if ( m_csMac.CompareNoCase(AtsInfo.m_csMacEnd) > 0 )
	//{		
	//	msg.Format("Mac overrange [%s ~ %s]",AtsInfo.m_csMacStart, AtsInfo.m_csMacEnd);		
	//	m_EDIT_mac.SetFocus();
	//	m_EDIT_mac.SetSel(0,m_csMac.GetLength());
	//	return false;
	//}

	//msg.Format("Input Mac Address : %s", m_csMac);
	//ShowMessage(msg);
	//return true;	
}

BOOL CMacDlg::CheckSN()
{
	//  Add your control notification handler code here
	CString msg;
	m_bMacLegal = false;
	int len = m_csSN.GetLength();
	if (m_csSN.GetLength() != AtsInfo.m_nSN_Length)
	{
		msg.Format("SN:\"%s\", length is %d, <> %d.\r\n", m_csSN, m_csSN.GetLength(), AtsInfo.m_nSN_Length);
		//MessageBox(msg,"ERROR",MB_OK);
		m_EDIT_sn.SetFocus();
		m_EDIT_sn.SetSel(0, m_csSN.GetLength());
		msg.Format("SN:Barcode Wrong");
		m_bMacLegal = false;
		ShowActionMessage(msg);
		return false;
	}
	m_bMacLegal = true;
	//msg.Format("Input SN : %s",m_csSN);
	//ShowMessage(msg);
	return true;
}

BOOL CMacDlg::CheckSSN1()
{
	//Here for NSB125C
	CString msg;
	m_bMacLegal = false;
	short len = m_csSSN1.GetLength();
	if (len == 12)
	{
		for (int i = 0; i < 12; i++)
		{
			if (iswxdigit(int(m_csSSN1[+i])) == 0)
			{
				msg.Format("Mac address:\"%s\", '%c' format is illegal.", m_csSSN1, m_csSSN1[i]);
				m_EDIT_ssn1.SetFocus();
				m_EDIT_ssn1.SetSel(0, m_csSSN1.GetLength());
				goto MAC_Err;
			}
			else
				m_bMacLegal = true;
		}
		return true;
	}
	else if (len == 17)
	{
		for (int i = 0; i < 17; i++)
		{
			//			AfxMessageBox((m_csMac.GetBuffer()) + i , MB_OK, 0);
			if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
			{
				if (m_csSSN1[i] == ':')
				{
					//					AfxMessageBox((m_csMac.GetBuffer()) + i, MB_OK, 0);
					continue;
				}
				else
				{
					//					msg.Format("Mac address:\"%s\", '%c' format is illegal.", m_csMac, m_csMac[i]);
					m_EDIT_ssn1.SetFocus();
					m_EDIT_ssn1.SetSel(0, m_csSSN1.GetLength());
					goto MAC_Err;
				}
			}
			if (iswxdigit(int(m_csMac[+i])) == 0)
			{
				//				msg.Format("Mac address:\"%s\", '%c' format is illegal.", m_csMac, m_csMac[i]);
				m_EDIT_ssn1.SetFocus();
				m_EDIT_ssn1.SetSel(0, m_csSSN1.GetLength());
				goto MAC_Err;
			}
			else
				m_bMacLegal = true;
		}
		return true;

	}
MAC_Err:

	//		msg.Format("Mac address:\"%s\", length is %d, <> 12.\r\n", m_csMac, len);
	m_EDIT_ssn1.SetFocus();
	m_EDIT_ssn1.SetSel(0, m_csSSN1.GetLength());
	msg.Format("MACEndㄩBarcode Wrong");
	ShowActionMessage(msg);
	m_bMacLegal = false;
	return false;
# if 0
	//  Add your control notification handler code here
	CString msg;

	m_bMacLegal = false;

	if (m_csSSN1.GetLength() != AtsInfo.m_nSSN1_Length)
	{
		msg.Format("SSN1:\"%s\", length is %d, <> 12.\r\n", m_csSSN1, m_csSSN1.GetLength());
		m_EDIT_ssn1.SetFocus();
		m_EDIT_ssn1.SetSel(0, m_csSSN1.GetLength());
		msg.Format("SSN1:Barcode Wrong");
		m_bMacLegal = false;
		ShowActionMessage(msg);
		return false;
	}
	//for ( int i = 0 ; i < 12 ; i ++ )
	//{		
	//	if ( iswxdigit(int(m_csSSN1[+i])) == 0 )
	//	{
	//		msg.Format("SSN1:\"%s\", '%c' format is illegal.",m_csSSN1,m_csSSN1[i]);
	//		m_EDIT_ssn1.SetFocus();
	//		m_EDIT_ssn1.SetSel(0,m_csSSN1.GetLength());
	//		return false;
	//	}
	//	else
	m_bMacLegal = true;
	//}

	//msg.Format("Input SSN1 : %s",m_csSSN1);
	//ShowMessage(msg);
	return true;
#endif
}

BOOL CMacDlg::CheckSSN2()
{
	//  Add your control notification handler code here
	CString msg;

	m_bMacLegal = false;

	if (m_csSSN2.GetLength() != AtsInfo.m_nSSN2_Length)
	{
		msg.Format("SSN2:\"%s\", length is %d, <> 12.\r\n", m_csSSN2, m_csSSN2.GetLength());
		//MessageBox(msg,"ERROR",MB_OK);
		m_EDIT_ssn2.SetFocus();
		m_EDIT_ssn2.SetSel(0, m_csSSN2.GetLength());
		msg.Format("SSN2: Barcode Wrong");
		m_bMacLegal = false;
		ShowActionMessage(msg);
		return false;
	}
	//msg.Format("Input SSN2 : %s",m_csSSN2);
	//ShowMessage(msg);
	m_bMacLegal = true;
	return true;
}

void CMacDlg::ShowMessage(CString msg)
{
	::SendMessage(m_NotifyhWnd, MSG_SHOW_MESSAGE, (WPARAM)&msg, 0);
}
void CMacDlg::ShowActionMessage(CString msg)
{
	COLORREF color = COLOR_RED;
	::SendMessage(m_NotifyhWnd, MSG_SHOW_ACT_MESSAGE, (WPARAM)&msg, (LPARAM)&color);
}
void CMacDlg::OnBnClickedCancel()
{
	m_bMacLegal = false;
	OnCancel();
}

void CMacDlg::OnBnClickedOk()
{
	m_EDIT_sn.GetWindowText(m_csSN);
	m_EDIT_ssn2.GetWindowText(m_csSSN2);

	if (-1 != m_csSN.Find(":"))
	{
		int nPos = m_csSN.Find(":");
		CString csTemp = m_csSN.Right(m_csSN.GetLength() - nPos);
		m_csSN = csTemp;
		m_csSN.Remove(':');
		m_csSN.Trim();
	}

	if (AtsInfo.m_bCheckSN)
		if (!CheckSN())
			return;


	if (PT_FT == "PT" && (modelname != "NSK3300_MPCIE_LTE7430APAC_KIT" && modelname != "NSK3300_MPCIE_LTE7455NA" && modelname != "NSK3300_MPCIE_S")) {
		m_EDIT_mac.GetWindowText(m_csMac);//mac begin
		m_EDIT_ssn1.GetWindowText(m_csSSN1);// mac end
		if (AtsInfo.m_bCheckMac)
		{
			if (!CheckMac())
				return;
		}
		if (AtsInfo.m_bCheckSSN1)
		{
			/*for NSB125C Here Input two MAC*/
			if (!CheckSSN1())
				return;

			BYTE nMACBeg[7], nMACEnd[7];
			CString csMacBeg, csMacEnd;
			int nReverse = -1;


			csMacBeg = m_csMac;
			csMacBeg.Remove(':');
			Str2MAC(nMACBeg, csMacBeg.GetBuffer());
			//czSSN1
			csMacEnd = m_csSSN1;
			csMacEnd.Remove(':');
			Str2MAC(nMACEnd, csMacEnd.GetBuffer());

			//SFIS.czSSN1[0]=

			for (int i = 0; i < 6; i++)
			{
				if (nMACBeg[i] == nMACEnd[i])
					continue;
				if (nMACBeg[i] > nMACEnd[i])
				{
					nReverse = 1;
					break;
				}
				else
				{
					nReverse = 0;
					break;
				}
			}
			if (-1 == nReverse)
			{
				ShowActionMessage("Same Barcode for TWO MAC");
				return;
			}
		}
	}

	if (AtsInfo.m_bCheckSSN2)
	{
		if (!CheckSSN2())
			return;
	}

	m_bMacLegal = true;

	//::AfxMessageBox(m_csMac);
	//::AfxMessageBox(m_csSSN1);
	OnOK();
}

BOOL CMacDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_EDIT_sn.GetWindowTextLength())
	{
		if (PT_FT == "PT" && (modelname != "NSK3300_MPCIE_LTE7430APAC_KIT" && modelname != "NSK3300_MPCIE_LTE7455NA" && modelname != "NSK3300_MPCIE_S"))
		{
			m_EDIT_mac.EnableWindow(AtsInfo.m_bInputMac);
		}
		
	}
	else
	{
		if (PT_FT == "PT" && (modelname != "NSK3300_MPCIE_LTE7430APAC_KIT" && modelname != "NSK3300_MPCIE_LTE7455NA" && modelname != "NSK3300_MPCIE_S"))
		{
			m_EDIT_mac.SetWindowTextA("");
		}

		m_EDIT_sn.SetFocus();
		m_EDIT_sn.SetSel(0, m_EDIT_sn.GetWindowTextLength());
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->hwnd == m_EDIT_sn.m_hWnd)
	{
		//在SN EDIT按enter時，對MAC EDIT做focus
		if (pMsg->wParam == VK_RETURN && AtsInfo.m_bInputMac)
		{
			if (PT_FT == "PT" && (modelname != "NSK3300_MPCIE_LTE7430APAC_KIT" && modelname != "NSK3300_MPCIE_LTE7455NA" && modelname != "NSK3300_MPCIE_S"))
			{
				m_EDIT_mac.EnableWindow(1);
				m_EDIT_mac.SetFocus();
				m_EDIT_mac.SetSel(0, m_EDIT_mac.GetWindowTextLength());
			}
			else {
				CMacDlg::OnBnClickedOk();
			}
			return TRUE;
		}
	}
	if (m_EDIT_mac.GetWindowTextLength())
	{
		m_EDIT_ssn1.EnableWindow(AtsInfo.m_bInputSSN1);
	}
	else
	{
		if (PT_FT == "PT" && (modelname != "NSK3300_MPCIE_LTE7430APAC_KIT" && modelname != "NSK3300_MPCIE_LTE7455NA" && modelname != "NSK3300_MPCIE_S"))
		{
			m_EDIT_ssn1.SetWindowTextA("");
		}
	}
	if (pMsg->message == WM_KEYDOWN && pMsg->hwnd == m_EDIT_mac.m_hWnd)
	{
		//在mac按enter時，對SSN1 EDIT做focus
		if (pMsg->wParam == VK_RETURN && AtsInfo.m_bInputSSN1 && PT_FT == "PT" && (modelname != "NSK3300_MPCIE_LTE7430APAC_KIT" && modelname != "NSK3300_MPCIE_LTE7455NA" && modelname != "NSK3300_MPCIE_S"))
		{
			m_EDIT_ssn1.EnableWindow(1);
			m_EDIT_ssn1.SetFocus();
			m_EDIT_ssn1.SetSel(0, m_EDIT_ssn1.GetWindowTextLength());
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->hwnd == m_EDIT_ssn1.m_hWnd)
	{
		//在ssn1按enter時，對SSN2 EDIT做focus
		if (pMsg->wParam == VK_RETURN && AtsInfo.m_bInputSSN2)
		{
			m_EDIT_ssn2.EnableWindow(1);
			m_EDIT_ssn2.SetFocus();
			m_EDIT_ssn2.SetSel(0, m_EDIT_ssn2.GetWindowTextLength());
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CMacDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	m_EDIT_sn.EnableWindow(AtsInfo.m_bInputSN);
	m_EDIT_mac.EnableWindow(0);
	m_EDIT_ssn1.EnableWindow(0);
	m_EDIT_ssn2.EnableWindow(0);
	/*m_EDIT_mac.EnableWindow(AtsInfo.m_bInputMac);
	m_EDIT_ssn1.EnableWindow(AtsInfo.m_bInputSSN1);
	m_EDIT_ssn2.EnableWindow(AtsInfo.m_bInputSSN2);*/
	if (AtsInfo.m_bInputSN)
		m_EDIT_sn.SetFocus();
}

void CMacDlg::OnEnChangeEditMac()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CMacDlg::OnEnChangeEditSsn1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CMacDlg::OnEnChangeEditSn()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
