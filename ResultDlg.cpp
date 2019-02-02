// ResultDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ResultDlg.h"
#include "ResourceHelper.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CResultDlg dialog



CResultDlg::CResultDlg(const CStringArray& strArray, CWnd* pParent /*=NULL*/)
	: CDialog(CResultDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResultDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	{//Copy 'CStringArray' Param
		int i;
		for(i = 0; i < strArray.GetSize(); i++)
			m_ResultStrArray.Add(strArray[i]);
	}
}
void CResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResultDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CResultDlg, CDialog)
	//{{AFX_MSG_MAP(CResultDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers

BOOL CResultDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	{//Copy 'CStringArray' Param
		const CString strConstRowSep("\r\n");
		int i;
		CString strToShow;
		for(i = 0; i < m_ResultStrArray.GetSize(); i++)
		{
			if(i == 0)
				strToShow = m_ResultStrArray[i];
			else
				strToShow += strConstRowSep + m_ResultStrArray[i];
		}
		GetDlgItem(IDC_EDIT_RESULT)->SetWindowText(strToShow);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ShowResultDlg(const CStringArray& Array)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CResultDlg Dlg(Array, pAcadWnd);
	Dlg.DoModal();
}

void ShowResultDlg(const CString& strInpFile)
{
	CWnd* pAcadWnd;
	CStringArray strLines;
	CString strLine;
	CTemporaryResourceOverride ResOverride;
	
	{//Read file from which data to show
		CStdioFile	FileInp;

		if(!FileInp.Open(strInpFile, CFile::modeRead|CFile::typeText))
		{
			strLine.Format("ERROR: Unable to open input file %s", (LPCSTR)strInpFile);
			AfxMessageBox(strLine, MB_ICONSTOP);
			return;
		}
		for(;;)
		{
			if(!FileInp.ReadString(strLine))
			{
				break;
			}
			strLines.Add(strLine);
		}
	}
	if(strLines.GetSize() <= 0)
		return;

	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());
	CResultDlg Dlg(strLines, pAcadWnd);
	Dlg.DoModal();
}


