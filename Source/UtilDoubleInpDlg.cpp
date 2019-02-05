// UtilDoubleInpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UtilDoubleInpDlg.h"
#include "ResourceHelper.h"

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CUtilDoubleInpDlg dialog


CUtilDoubleInpDlg::CUtilDoubleInpDlg(const double& fVal, const ValidationType iValidationType, const char* pszPmt, CWnd* pParent /*=NULL*/)
	: CDialog(CUtilDoubleInpDlg::IDD, pParent), m_fVal(fVal), m_pszPmt(pszPmt), m_iValidationType(iValidationType)
{
	//{{AFX_DATA_INIT(CUtilDoubleInpDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUtilDoubleInpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUtilDoubleInpDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUtilDoubleInpDlg, CDialog)
	//{{AFX_MSG_MAP(CUtilDoubleInpDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUtilDoubleInpDlg message handlers

void CUtilDoubleInpDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTemp;
	
	GetDlgItem(IDC_EDIT)->GetWindowText(strTemp);
	m_fVal = atof((LPCSTR)strTemp);

	//
	if(m_iValidationType & ValidationType::ZeroNotAllowed)
	{
		if(m_fVal == 0.0)
		{
			AfxMessageBox("Invalid value specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT))->SetSel(0, -1);
			return;
		}
	}
	//
	if(m_iValidationType & ValidationType::NegNotAllowed)
	{
		if(m_fVal < 0.0)
		{
			AfxMessageBox("Invalid value specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT))->SetSel(0, -1);
			return;
		}
	}
	//
	CDialog::OnOK();
}

BOOL CUtilDoubleInpDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;
	
	strTmp.Format("%f", m_fVal);
	GetDlgItem(IDC_EDIT)->SetWindowText(strTmp);
	GetDlgItem(IDC_STATIC_PROMPT)->SetWindowText(m_pszPmt);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////
BOOL GetPositiveDoubleVal(const char* pszPmt, const CUtilDoubleInpDlg::ValidationType iValidationType, double& fVal)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;

	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CUtilDoubleInpDlg Dlg(fVal, iValidationType, pszPmt, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(fVal);
	return TRUE;
}

