// ConvertRDPlanDataFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConvertRDPlanDataFileDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CConvertRDPlanDataFileDlg dialog

CConvertRDPlanDataFileDlg::CConvertRDPlanDataFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConvertRDPlanDataFileDlg::IDD, pParent)
{
	m_fValIncr = 0.0;
	//{{AFX_DATA_INIT(CConvertRDPlanDataFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConvertRDPlanDataFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConvertRDPlanDataFileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConvertRDPlanDataFileDlg, CDialog)
	//{{AFX_MSG_MAP(CConvertRDPlanDataFileDlg)

	ON_BN_CLICKED(IDC_OPEN_STN_DATA,	OnOpenSTN)
	ON_BN_CLICKED(IDC_OPEN_OUT_FILE,	OnOpenOUT)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvertRDPlanDataFileDlg message handlers

BOOL CConvertRDPlanDataFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	// TODO: Add extra initialization here
	CString strTmp;

	strTmp.Format("%f", m_fValIncr); GetDlgItem(IDC_EDIT_VAL)->SetWindowText(strTmp);
	return TRUE;
}

void CConvertRDPlanDataFileDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp;
	
	//
	GetDlgItem(IDC_EDIT_VAL)->GetWindowText(strTmp);
	m_fValIncr = atof(strTmp);
	if(m_fValIncr == 0.0)
	{
		AfxMessageBox("Invalid value provided", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_VAL)->SetFocus();
		return;
	}
	//
	GetDlgItem(IDC_EDIT_FILENAME_STN)->GetWindowText(m_strInpFile);
	if(strlen(m_strInpFile) <= 0)
	{
		AfxMessageBox("Invalid Station data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_STN_DATA)->SetFocus();
		return;
	}
	//
	GetDlgItem(IDC_EDIT_FILENAME_OUT)->GetWindowText(m_strOutFile);
	if(strlen(m_strOutFile) <= 0)
	{
		AfxMessageBox("Invalid output data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_OUT_FILE)->SetFocus();
		return;
	}

	CDialog::OnOK();
}

void CConvertRDPlanDataFileDlg::OnOpenOUT() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Output Files (*.TXT)|*.TXT||");

	dwFlags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME_OUT)->SetWindowText(Dlg.GetPathName());
	}
}
void CConvertRDPlanDataFileDlg::OnOpenSTN() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Station Data Files (*.STN)|*.STN||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME_STN)->SetWindowText(Dlg.GetPathName());
	}
}
BOOL GetRDPlanDataFileConversionData(CString& strInpFile, CString& strOutFile, double& fValIncr)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;

	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());
	CConvertRDPlanDataFileDlg Dlg(pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(strInpFile, strOutFile, fValIncr);
	return TRUE;
}
