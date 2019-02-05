// TraverseCorrectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TraverseCorrectionDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CTraverseCorrectionDlg dialog


CTraverseCorrectionDlg::CTraverseCorrectionDlg(const TRAVDWGPARAM& Rec, const BOOL& bIsInpFromXLS, CWnd* pParent /*=NULL*/)
	: CDialog(CTraverseCorrectionDlg::IDD, pParent), m_Rec(Rec), m_bIsInpFromXLS(bIsInpFromXLS)
{
	//{{AFX_DATA_INIT(CTraverseCorrectionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTraverseCorrectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTraverseCorrectionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTraverseCorrectionDlg, CDialog)
	//{{AFX_MSG_MAP(CTraverseCorrectionDlg)
	ON_BN_CLICKED(IDC_BUTTON_INP_FILE, OnButtonInpFile)
	ON_BN_CLICKED(IDC_BUTTON_OUT_FILE, OnButtonOutFile)
	ON_BN_CLICKED(IDDRAW, OnDraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTraverseCorrectionDlg message handlers

BOOL CTraverseCorrectionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CString strTmp;

	GetDlgItem(IDC_EDIT_FILE_IN)->SetWindowText(m_Rec.szInFile);
	GetDlgItem(IDC_EDIT_FILE_OUT)->SetWindowText(m_Rec.szOutFile);

	strTmp.Format("%f", m_Rec.fStnMarkTxtSize); GetDlgItem(IDC_EDIT_STN_TXT)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.fTableTxtSize); GetDlgItem(IDC_EDIT_TAB_TXT)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.fTableMargin); GetDlgItem(IDC_EDIT_TAB_MARGIN)->SetWindowText(strTmp);

	((CButton*)GetDlgItem(IDDRAW))->SetCheck((m_Rec.fStnMarkTxtSize > 0.0)?1:0);
	if(m_bIsInpFromXLS)
	{
		SetWindowText("Open Traverse Adjustment");
	}
	else
	{
		SetWindowText("Close Traverse Adjustment");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTraverseCorrectionDlg::OnButtonInpFile() 
{
	const DWORD dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CString strFile, strConstFileOpenFilter;

	if(!m_bIsInpFromXLS)
	{
		strConstFileOpenFilter = CString("Traverse Data(*.TXT)|*.TXT||");
	}
	else
	{
		strConstFileOpenFilter = CSurvUtilApp::m_strConstFileOpenFilter;
	}
	{//
		CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFileOpenFilter);
		if(Dlg.DoModal() == IDOK)
		{
			strFile = Dlg.GetPathName();
			GetDlgItem(IDC_EDIT_FILE_IN)->SetWindowText(strFile);
		}
	}
}

void CTraverseCorrectionDlg::OnButtonOutFile() 
{
	const CString strConstFileOpenFilter = CString("Traverse Adjustment Result Data(*.TXT)|*.TXT||");
	DWORD dwFlags;
	CString strFile;

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFileOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		strFile = Dlg.GetPathName();
		GetDlgItem(IDC_EDIT_FILE_OUT)->SetWindowText(strFile);
	}
}

void CTraverseCorrectionDlg::OnDraw() 
{
	BOOL bFlag;
	bFlag = (0 != ((CButton*)GetDlgItem(IDDRAW))->GetCheck());

	GetDlgItem(IDC_EDIT_STN_TXT)->EnableWindow(bFlag);
	GetDlgItem(IDC_EDIT_TAB_TXT)->EnableWindow(bFlag);
	GetDlgItem(IDC_EDIT_TAB_MARGIN)->EnableWindow(bFlag);
}

void CTraverseCorrectionDlg::OnOK() 
{
	CString strTmp;

	GetDlgItem(IDC_EDIT_FILE_IN)->GetWindowText(strTmp);
	if(strlen((LPCSTR)strTmp) <= 0)
	{
		AfxMessageBox("Invalid Input file name specified", MB_ICONSTOP);
		GetDlgItem(IDC_BUTTON_INP_FILE)->SetFocus();
		return;
	}
	if(strlen((LPCSTR)strTmp) >= _MAX_PATH)
	{
		AfxMessageBox("Input file path is too long", MB_ICONSTOP);
		GetDlgItem(IDC_BUTTON_INP_FILE)->SetFocus();
		return;
	}
	strcpy(m_Rec.szInFile, (LPCSTR)strTmp);
	//
	GetDlgItem(IDC_EDIT_FILE_OUT)->GetWindowText(strTmp);
	if(strlen((LPCSTR)strTmp) <= 0)
	{
		AfxMessageBox("Invalid Output file name specified", MB_ICONSTOP);
		GetDlgItem(IDC_BUTTON_OUT_FILE)->SetFocus();
		return;
	}
	if(strlen((LPCSTR)strTmp) >= _MAX_PATH)
	{
		AfxMessageBox("Output file path is too long", MB_ICONSTOP);
		GetDlgItem(IDC_BUTTON_OUT_FILE)->SetFocus();
		return;
	}
	strcpy(m_Rec.szOutFile, (LPCSTR)strTmp);
	if(strTmp.CompareNoCase(m_Rec.szInFile) == 0)
	{
		AfxMessageBox("Input file & Output file can't be same", MB_ICONSTOP);
		GetDlgItem(IDC_BUTTON_INP_FILE)->SetFocus();
		return;
	}

	if(0 == ((CButton*)GetDlgItem(IDDRAW))->GetCheck())
	{//Draw check button not checked
		m_Rec.fStnMarkTxtSize = 0.0;
		m_Rec.ptTravBase		= AcGePoint3d(0.0, 0.0, 0.0);
		m_Rec.fTableTxtSize	= 0.0;
		m_Rec.fTableMargin	= 0.0;
		m_Rec.ptTableBase		= AcGePoint3d(0.0, 0.0, 0.0);
	}
	else
	{//Check values..........
		//
		GetDlgItem(IDC_EDIT_STN_TXT)->GetWindowText(strTmp);
		m_Rec.fStnMarkTxtSize = atof(strTmp);

		if(m_Rec.fStnMarkTxtSize <= 0.0)
		{
			AfxMessageBox("Station Mark Text Size specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_STN_TXT)->SetFocus();
			return;
		}
		//
		GetDlgItem(IDC_EDIT_TAB_TXT)->GetWindowText(strTmp);
		m_Rec.fTableTxtSize = atof(strTmp);

		if(m_Rec.fTableTxtSize <= 0.0)
		{
			AfxMessageBox("Calculation Table Text Size specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_TAB_TXT)->SetFocus();
			return;
		}
		//
		GetDlgItem(IDC_EDIT_TAB_MARGIN)->GetWindowText(strTmp);
		m_Rec.fTableMargin = atof(strTmp);

		if(m_Rec.fTableMargin <= 0.0)
		{
			AfxMessageBox("Calculation Table Margin specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_TAB_MARGIN)->SetFocus();
			return;
		}
	}

	CDialog::OnOK();
}

//
////////////////////////////////////////////////////////////////////////////
//
BOOL GetTraverseParam(TRAVDWGPARAM& Rec, const BOOL& bIsInpFromXLS)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CTraverseCorrectionDlg Dlg(Rec, bIsInpFromXLS, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttrib(Rec);
	return TRUE;
}
