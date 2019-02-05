// PflFromChgZDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"
#include "LyrSettingDlg.h"
#include "SurfaceParamDlg.h"
#include "PflFromChgZDlg.h"

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CPflFromChgZDlg dialog


CPflFromChgZDlg::CPflFromChgZDlg(const PFLOFFZDWGPARAM& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CPflFromChgZDlg::IDD, pParent), m_Rec(Rec)
{
	//{{AFX_DATA_INIT(CPflFromChgZDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPflFromChgZDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPflFromChgZDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPflFromChgZDlg, CDialog)
	//{{AFX_MSG_MAP(CPflFromChgZDlg)
	ON_BN_CLICKED(IDC_BUTTON_LYR,				OnLyrSetting)
	ON_BN_CLICKED(IDC_BUTTON_ANN_TAGS,			OnSetAnnTags)
	ON_BN_CLICKED(IDSET_SURF_MARKER, OnSurfMarker)
	ON_BN_CLICKED(IDC_OPEN_DAT_FILE, OnOpenDataFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPflFromChgZDlg message handlers
void CPflFromChgZDlg::OnOpenDataFile() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Profile Data Files (*.PRD)|*.PRD||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(Dlg.GetPathName());
	}
}

BOOL CPflFromChgZDlg::OnInitDialog() 
{
	CString strTmp;
	
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	const BOOL bFlag = (m_Rec.fOrdAnnInterval >= 0.0);
	strTmp.Format("%f", m_Rec.fOrdAnnInterval); GetDlgItem(IDC_EDIT_ORD_INTERVAL)->SetWindowText(strTmp);
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->EnableWindow(bFlag);
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->EnableWindow(bFlag);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);

	SetWindowText("Set Profile Parameters");
	strTmp.Format("%f", m_Rec.dXScale); GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dYScale); GetDlgItem(IDC_EDIT_EXAG_VERT)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dDatum);	GetDlgItem(IDC_EDIT_DATUM)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dRowAnnSize);		GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dNumAnnSize);		GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dRowHt);			GetDlgItem(IDC_EDIT_ROW_HT)->SetWindowText(strTmp);
	
	GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(m_Rec.szDataFile);
	return TRUE;
}
void CPflFromChgZDlg::OnSurfMarker() 
{
	CStringArray strArrayBlocks;

	GetBlockNames(strArrayBlocks);
	if(strArrayBlocks.GetSize() <= 0)
	{
		AfxMessageBox("No block definition found in current drawing/nCreate block first & then try again", MB_ICONSTOP);
		strcpy(m_Rec.szXistTopSurfAnnBlk, "");
		m_Rec.dXistTopSurfAnnBlkScale = -1.0;//Invalid
		m_Rec.dXistTopSurfAnnBlkSpaceMin = -1.0;//Invalid
		return;
	}
	
	CSurfaceParamDlg Dlg(strArrayBlocks, m_Rec.dXistTopSurfAnnBlkScale, m_Rec.dXistTopSurfAnnBlkSpaceMin, this);

	if(Dlg.DoModal() == IDOK)
	{
		CString strSelBlk;

		Dlg.GetAttrib(strSelBlk, m_Rec.dXistTopSurfAnnBlkScale, m_Rec.dXistTopSurfAnnBlkSpaceMin);
		strcpy(m_Rec.szXistTopSurfAnnBlk, strSelBlk);
	}
	
}
void CPflFromChgZDlg::OnLyrSetting() 
{
	const int iSize = 4;
	const char* pszTags[] = {"Axis", "Orindates", "Annotation", "Existing Top Surface"};
	const char* pszVals[] = {m_Rec.szLyrAxis, m_Rec.szLyrOrdinate, m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface};
	
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szLyrAxis, m_Rec.szLyrOrdinate, m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface};
	Dlg.GetValues(iSize, pszValsDummy);
}
void CPflFromChgZDlg::OnSetAnnTags() 
{
	const int iSize = 5;
	const char* pszTags[] = {"Profile Label", "Elevation", "Chainage", "Dist. from Last Station", "Datum"};
	const char* pszVals[] = {m_Rec.szProfileTag, m_Rec.szElevTag, m_Rec.szDistTag, m_Rec.szCumDistTag, m_Rec.szDatumTag};
	
	CLyrSettingDlg Dlg("Set Annotation Tags", FALSE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szProfileTag, m_Rec.szElevTag, m_Rec.szDistTag, m_Rec.szCumDistTag, m_Rec.szDatumTag};
	Dlg.GetValues(iSize, pszValsDummy);
}

void CPflFromChgZDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTemp;
	UINT iID;
	//
	GetDlgItem(IDC_EDIT_FILENAME)->GetWindowText(strTemp);
	if(strlen(strTemp) <= 0)
	{
		AfxMessageBox("Invalid data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_DAT_FILE)->SetFocus();
		return;
	}
	strcpy(m_Rec.szDataFile, strTemp);
	//
	iID = IDC_EDIT_EXAG_HORZ;
	GetDlgItem(iID)->GetWindowText(strTemp);
	m_Rec.dXScale = atof(strTemp);
	if(m_Rec.dXScale <= 0.0)
	{
		AfxMessageBox("Invalid value found", MB_ICONSTOP);
		GetDlgItem(iID)->SetFocus();
		((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
		return;
	}
	//
	iID = IDC_EDIT_EXAG_VERT;
	GetDlgItem(iID)->GetWindowText(strTemp);
	m_Rec.dYScale = atof(strTemp);
	if(m_Rec.dYScale <= 0.0)
	{
		AfxMessageBox("Invalid value found", MB_ICONSTOP);
		GetDlgItem(iID)->SetFocus();
		((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
		return;
	}
	//
	iID = IDC_EDIT_LAB_TXT_SIZE;
	GetDlgItem(iID)->GetWindowText(strTemp);
	m_Rec.dRowAnnSize = atof(strTemp);
	if(m_Rec.dRowAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid value found", MB_ICONSTOP);
		GetDlgItem(iID)->SetFocus();
		((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
		return;
	}
	//
	iID = IDC_EDIT_NUM_TXT_SIZE;
	GetDlgItem(iID)->GetWindowText(strTemp);
	m_Rec.dNumAnnSize = atof(strTemp);
	if(m_Rec.dNumAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid value found", MB_ICONSTOP);
		GetDlgItem(iID)->SetFocus();
		((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
		return;
	}
	//
	iID = IDC_EDIT_ROW_HT;
	GetDlgItem(iID)->GetWindowText(strTemp);
	m_Rec.dRowHt = atof(strTemp);
	if(m_Rec.dRowHt <= 0.0)
	{
		AfxMessageBox("Invalid value found", MB_ICONSTOP);
		GetDlgItem(iID)->SetFocus();
		((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_DATUM)->GetWindowText(strTemp);
	m_Rec.dDatum = atof(strTemp);
	//
	if(m_Rec.fOrdAnnInterval >= 0.0)
	{
		double fTmp;

		GetDlgItem(IDC_EDIT_ORD_INTERVAL)->GetWindowText(strTemp);
		fTmp = atof(strTemp);
		if(fTmp <= 0.0)
		{
			AfxMessageBox("Invalid ordinate annotation interval", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_ORD_INTERVAL)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_ORD_INTERVAL))->SetSel(0, -1);
			return;
		}
		m_Rec.fOrdAnnInterval = fTmp;
	}

	CDialog::OnOK();
}
void CPflFromChgZDlg::GetAttribs(PFLOFFZDWGPARAM& Rec)
{
	Rec = m_Rec;
}

BOOL GetPflFromChgZParam(PFLOFFZDWGPARAM& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CPflFromChgZDlg Dlg(Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(Rec);
	return TRUE;
}

