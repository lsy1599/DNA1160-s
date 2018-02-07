
#include "stdafx.h"
#include "DB.h"
#include "ado2.h"

CMyDB::CMyDB()
{   

}

CMyDB::~CMyDB()
{	

}

//建構子設定DB連線的參數：如帳號、密碼、Data source
CMyDB::CMyDB(LPCTSTR id, LPCTSTR password, LPCTSTR data_source)
{   
	CString str;
	str.Format("Provider=OraOLEDB.Oracle.1;User ID=%s;Password=%s;", id, password);
	str.AppendFormat("Persist Security Info=True;Data Source=%s", data_source);
	
	m_Id = id;
	m_Password = password;
	m_DataSource = data_source;
	m_strConnection = str;
}

//設定DB連線的參數：如帳號、密碼、Data source
void CMyDB::SetConnectionParam(LPCTSTR id, LPCTSTR password, LPCTSTR data_source)
{
	CString str;
	str.Format("Provider=OraOLEDB.Oracle.1;User ID=%s;Password=%s;", id, password);
	str.AppendFormat("Persist Security Info=True;Data Source=%s", data_source);
	
	m_Id = id;
	m_Password = password;
	m_DataSource = data_source;
	m_strConnection = str;

	return;
}

//開啟資料庫
int CMyDB::OpenDBConnection(CString *err_msg)
{
	int ret;
	try
	{
		ret = m_ADODB.Open(m_strConnection, m_Id, m_Password);
		if ( !ret )
		{
			m_ADODB.Close();
			return FALSE;
		}
	}
	catch ( CADOException &e )
	{
		*err_msg = e.GetErrorMessage();
		return 0;
	}
	
	return 1;
}

//對Recordset做開啟並做操作
int CMyDB::OpenRSConnection()
{
	int ret;
	_ConnectionPtr m_pConnection;
	m_pConnection = m_ADODB.GetActiveConnection();
	ret = m_ADORS.Open(m_pConnection,m_vSQL);

	m_ADORS.Close();
	m_ADODB.Close();

	return 1;
}

//設定SQL要用什麼指令
void CMyDB::SetSQLText(UINT action, LPCSTR sql_command )
{
	switch (action)
	{
	case 1: //INSERT
		strcpy(m_vSQL, "hihi");
		break;
	case 2: //SELECT
		break;
	case 3: //DELETE
		break;

	}

	//vSQL = "INSERT INTO APPL.A_PRE_STATION_T (SN, MAC, MODEL_NAME, GROUP_NAME, STATION_NAME, LINE_NAME, PC_IP, TEST_TIME, ERROR_CODE, TEST_RESULT, SFIS_RESULT, USB_SPEED) VALUES ( '" +
	//			csInputSN + "','" + csInputMAC + "','" + AppIni.csModelName + "','" +
	//			AppIni.csGroup_Name + "','" + AppIni.csStation_Name + "','" + 
	//			AppIni.csLine_Number + "','" + AppIni.csPC_IP + "',sysdate,'" +
	//			gECode + "','" + m_csRet + "','" + sfisresult + "'," + data_queue +")";
}


//void CMyDB::DbCBT_RecordLogBT(_bstr_t vSQL)
//{
	
		////_bstr_t vSQL;	
		//CString strConnect = "Provider=OraOLEDB.Oracle.1;Password=tsd123;Persist Security Info=True;User ID=tsd;Data Source=tedb";

		////ShowListMsg("預備開啟數據庫...1003");
		//if(!m_ADODatabase.Open(strConnect,"tsd","tsd123"))
		//{
		//	MessageBox("ERROR : 數據開啟失敗！1004");
		//	exit(1);
		//}
		////ShowListMsg("開啟數據庫成功1005");
		//_ConnectionPtr m_pConnection;
		////ShowListMsg("取得連線狀態1006");
		//m_pConnection = m_ADODatabase.GetActiveConnection();
		////ShowListMsg("連線狀態OK 1007");

		////ShowListMsg("開啟資料表並執行SQL語法1008");
		//m_ADORecordset.Open(m_pConnection,vSQL);
		////ShowListMsg("開啟資料表並執行SQL語法OK 1009");
		////ShowListMsg("關閉資料表和資料庫1010");
		//m_ADORecordset.Close();
		//m_ADODatabase.Close();
		////ShowListMsg("關閉資料表和資料庫OK 1011");	
	//return;
//}




