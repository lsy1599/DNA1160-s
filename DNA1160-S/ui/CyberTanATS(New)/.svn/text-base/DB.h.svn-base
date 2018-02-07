#pragma once

// CMyDB by Ryan last modified 2008/12/31
#include "ado2.h"

class CMyDB
{
private:
	CADODatabase		m_ADODB;
	CADORecordset		m_ADORS;
	_bstr_t				m_vSQL; //for SQL operation
	CString				tempString;
		

public:
	CMyDB();
	CMyDB(LPCTSTR id, LPCTSTR password, LPCTSTR data_source);
	virtual ~CMyDB();

	void SetConnectionParam(LPCTSTR password, LPCTSTR id, LPCTSTR data_source);	
	int  OpenDBConnection(CString *err_msg);	
	int  OpenRSConnection();
	void SetSQLText(UINT action, LPCSTR sql_command);

protected: 

	CString		m_Id;
	CString		m_Password;
	CString		m_DataSource;

	CString		m_strConnection; //³s±µ¥Î¦r¦ê
	CString		m_strExecution; 
};







