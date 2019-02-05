// Protection.cpp : implementation file
//

#include "stdafx.h"           // MFC stuff
#include "Protection.h"

const CString	CProtection::m_strFileName = CString("SurvUtilSys.DLL");
const int		CProtection::m_iNoOfAllowableAxessDays = 365;

CProtection::CProtection()
{
}
CProtection::~CProtection()
{
}

//Creates the file with default data...
BOOL CProtection::_ReadFile(const CString& strFile)
{
	CFile File;
	PROTECTIONDATA Rec;

	if(!File.Open(strFile, CFile::modeRead|CFile::typeBinary))
		return FALSE;

	if(sizeof(PROTECTIONDATA) != File.Read(&Rec, sizeof(PROTECTIONDATA)))
		return FALSE;
	
	//Set Member Vars....
	m_InstallationDate = (DATE)(Rec.fInstallDate);
	m_LastAccessDate = (DATE)(Rec.fLastAccessDate);
	m_iNoOfDaysAccessed = Rec.iNoOfDaysAccessed;

	return TRUE;
}
BOOL CProtection::_WriteFile(const CString& strFile, const BOOL& bWithDefaultData /*= FALSE*/)
{
	CFile File;
	PROTECTIONDATA Rec;

	if(bWithDefaultData)
	{
		COleDateTime CurDateTime;
		
		CurDateTime = COleDateTime::GetCurrentTime();
		Rec.fInstallDate = (DATE)CurDateTime;
		Rec.fLastAccessDate = (DATE)CurDateTime;
		Rec.iNoOfDaysAccessed = 0;
	}
	else
	{
		Rec.fInstallDate = (DATE)(m_InstallationDate);
		Rec.fLastAccessDate = (DATE)(m_LastAccessDate);
		Rec.iNoOfDaysAccessed = m_iNoOfDaysAccessed;
	}
	if(!File.Open(strFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary))
		return FALSE;
	
	File.Write(&Rec, sizeof(PROTECTIONDATA));
	return TRUE;
}

BOOL CProtection::_GetFileName(CString& strPath)
{
	char szSysDir[_MAX_PATH + 1];

	if(GetSystemDirectory(szSysDir, _MAX_PATH) == 0)
		return FALSE; 
 
	strPath = CString(szSysDir) + CString("\\") + CProtection::m_strFileName;
	return TRUE;
}
BOOL CProtection::_IsFileExists(const CString& strFile)
{
	CFileFind FileFind;

	return FileFind.FindFile(strFile);
}

BOOL CProtection::Initialize(const BOOL& bIsFirstRun /*= FALSE*/)
{
	return TRUE;//TEMPORARY

	COleDateTime CurDateTime;
	COleDateTimeSpan TimeElapsed;
	CString strDataFile;

	
	CurDateTime = COleDateTime::GetCurrentTime();
	m_bCanRunApp = FALSE;
	if(!CProtection::_GetFileName(strDataFile))
		return FALSE;

	if(bIsFirstRun)
	{//Create BIN file in Sys Directory...if doesn't exists
		if(!CProtection::_IsFileExists(strDataFile))
		{
			m_bCanRunApp = _WriteFile(strDataFile, TRUE);
			return m_bCanRunApp;
		}
	}
	{//File exists...so check different criteria....
		//Read the file from Disk....
		if(!CProtection::_IsFileExists(strDataFile))
			return FALSE;

		if(!_ReadFile(strDataFile))
			return FALSE;

		//Check the values......
		if(m_InstallationDate > CurDateTime)
			return FALSE;
		if(m_LastAccessDate > CurDateTime)
			return FALSE;

		if(! ((m_LastAccessDate.GetYear() == CurDateTime.GetYear()) && (m_LastAccessDate.GetMonth() == CurDateTime.GetMonth()) && (m_LastAccessDate.GetDay() == CurDateTime.GetDay())))
		{
			m_LastAccessDate = CurDateTime;
		}
		TimeElapsed = CurDateTime - m_InstallationDate;
		m_iNoOfDaysAccessed = TimeElapsed.GetDays();

#ifdef _DEBUG
		{
			CString strMsg;
			
			strMsg.Format("Accessed %d days, allowable %d days", m_iNoOfDaysAccessed, m_iNoOfAllowableAxessDays);
			AfxMessageBox(strMsg, MB_ICONINFORMATION);
		}
#endif //_DEBUG
		if(m_iNoOfDaysAccessed > m_iNoOfAllowableAxessDays)
			return FALSE;
		
		//Save the file....
		m_bCanRunApp = _WriteFile(strDataFile);
		return m_bCanRunApp;
	}
	return FALSE;
}
