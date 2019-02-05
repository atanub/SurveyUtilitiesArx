// FBSheetParamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FBSheetParamDlg.h"
#include "SurvUtilApp.h"

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CFBSheetParamDlg dialog

CFBSheetParamDlg::CFBSheetParamDlg(FBDWGPARAM& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CFBSheetParamDlg::IDD, pParent), m_RecDwgParam(Rec)
{
	//{{AFX_DATA_INIT(CFBSheetParamDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFBSheetParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFBSheetParamDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFBSheetParamDlg, CDialog)
	//{{AFX_MSG_MAP(CFBSheetParamDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFBSheetParamDlg message handlers

void CFBSheetParamDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp;
	
	GetDlgItem(IDC_EDIT1)->GetWindowText(strTmp);
	m_RecDwgParam.fTxtSize = atof(strTmp);
	if(m_RecDwgParam.fTxtSize <= 0.0)
	{
		AfxMessageBox("Invalid Text Size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT1)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(0, -1);
	}

	GetDlgItem(IDC_EDIT2)->GetWindowText(strTmp);
	m_RecDwgParam.fBlkScale = atof(strTmp);
	if(m_RecDwgParam.fBlkScale <= 0.0)
	{
		AfxMessageBox("Invalid Block Size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT2)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT2))->SetSel(0, -1);
	}

	GetDlgItem(IDC_EDIT3)->GetWindowText(strTmp);
	if(!CSurvUtilApp::IsValidACADSymName(strTmp))
	{
		strTmp.Format("Invalid layer name");
		GetDlgItem(IDC_EDIT3)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT3))->SetSel(0, -1);
	}
	strcpy(m_RecDwgParam.szLyr, (LPCSTR)strTmp);

	
	CDialog::OnOK();
}

BOOL CFBSheetParamDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;
	
	strTmp.Format("%f", m_RecDwgParam.fTxtSize);
	GetDlgItem(IDC_EDIT1)->SetWindowText(strTmp);

	strTmp.Format("%f", m_RecDwgParam.fBlkScale);
	GetDlgItem(IDC_EDIT2)->SetWindowText(strTmp);

	GetDlgItem(IDC_EDIT3)->SetWindowText(m_RecDwgParam.szLyr);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////
BOOL GetFBSheetDwgParam(FBDWGPARAM& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;

	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());
	CFBSheetParamDlg Dlg(Rec, pAcadWnd);
	
	if(Dlg.DoModal() == IDOK)
	{
		Dlg.GetAttrib(Rec);
		return TRUE;
	}
	return FALSE;
}