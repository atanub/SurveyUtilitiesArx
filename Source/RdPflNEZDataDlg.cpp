// RdPflNEZDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RdPflNEZDataDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"
#include "LyrSettingDlg.h"
#include "SurfaceParamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CRdPflNEZDataDlg dialog


CRdPflNEZDataDlg::CRdPflNEZDataDlg(const RDPFLNEZDATA& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CRdPflNEZDataDlg::IDD, pParent), m_RdPflNEZDataRec(Rec), m_RdPflNEZDataRecNew(Rec)
{
	//{{AFX_DATA_INIT(CRdPflNEZDataDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRdPflNEZDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRdPflNEZDataDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRdPflNEZDataDlg, CDialog)
	//{{AFX_MSG_MAP(CRdPflNEZDataDlg)
	ON_BN_CLICKED(IDLYRSETTING,		OnLyrSetting)
	ON_BN_CLICKED(IDANNTAGSSETTING, OnSetAnnTags)
	ON_BN_CLICKED(IDSET_SURF_MARKER, OnSurfMarker)
	ON_BN_CLICKED(IDSET_OUTFILE, OnOutFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRdPflNEZDataDlg message handlers

BOOL CRdPflNEZDataDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;
	BOOL bFlag;

	strTmp.Format("%f", m_RdPflNEZDataRec.dXScale); GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetWindowText(strTmp);
	strTmp.Format("%f", m_RdPflNEZDataRec.dYScale); GetDlgItem(IDC_EDIT_EXAG_VERT)->SetWindowText(strTmp);
	strTmp.Format("%f", m_RdPflNEZDataRec.dDatum); GetDlgItem(IDC_EDIT_DATUM)->SetWindowText(strTmp);
	strTmp.Format("%f", m_RdPflNEZDataRec.dNumAnnSize); GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_RdPflNEZDataRec.dRowAnnSize); GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_RdPflNEZDataRec.dRowHt); GetDlgItem(IDC_EDIT_ROW_HT)->SetWindowText(strTmp);
	
	bFlag = (m_RdPflNEZDataRec.dOrdInterval >= 0.0);
	strTmp.Format("%f", m_RdPflNEZDataRec.dOrdInterval); GetDlgItem(IDC_EDIT_ORD_INTERVAL)->SetWindowText(strTmp);
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->EnableWindow(bFlag);
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->EnableWindow(bFlag);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);
	GetDlgItem(IDSET_OUTFILE)->EnableWindow(bFlag);
	GetDlgItem(IDSET_OUTFILE)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);
	GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(bFlag);
	GetDlgItem(IDC_EDIT_FILENAME)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);
	GetDlgItem(IDC_CHECK_DRAW_GEN_PTS)->EnableWindow(bFlag);
	GetDlgItem(IDC_CHECK_DRAW_GEN_PTS)->ShowWindow(bFlag ? SW_SHOWNORMAL:SW_HIDE);
	((CButton*)GetDlgItem(IDC_CHECK_DRAW_GEN_PTS))->SetCheck(m_RdPflNEZDataRec.bDrawGeneratedPoints);
	

	((CButton*)GetDlgItem(IDC_CHECK))->SetCheck((m_RdPflNEZDataRec.bConsider3DPts) ? 1:0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CRdPflNEZDataDlg::OnSurfMarker() 
{
	CStringArray strArrayBlocks;

	GetBlockNames(strArrayBlocks);
	if(strArrayBlocks.GetSize() <= 0)
	{
		AfxMessageBox("No block definition found in current drawing\nCreate block first & then try again", MB_ICONSTOP);
		strcpy(m_RdPflNEZDataRecNew.szXistTopSurfAnnBlk, "");
		m_RdPflNEZDataRecNew.dXistTopSurfAnnBlkScale = -1.0;//Invalid
		m_RdPflNEZDataRecNew.dXistTopSurfAnnBlkSpaceMin = -1.0;//Invalid
		return;
	}
	
	CSurfaceParamDlg Dlg(strArrayBlocks, m_RdPflNEZDataRecNew.dXistTopSurfAnnBlkScale, m_RdPflNEZDataRecNew.dXistTopSurfAnnBlkSpaceMin, this);

	if(Dlg.DoModal() == IDOK)
	{
		CString strSelBlk;

		Dlg.GetAttrib(strSelBlk, m_RdPflNEZDataRecNew.dXistTopSurfAnnBlkScale, m_RdPflNEZDataRecNew.dXistTopSurfAnnBlkSpaceMin);
		strcpy(m_RdPflNEZDataRecNew.szXistTopSurfAnnBlk, strSelBlk);
	}
	
}
void CRdPflNEZDataDlg::OnLyrSetting() 
{
	const int iSize = 4;
	const char* pszTags[] = {"Annotation", "Top Surface", "Orindates", "Axis"};
	const char* pszVals[] = {m_RdPflNEZDataRecNew.szLyrAnnotation, m_RdPflNEZDataRecNew.szLyrTopSurface, m_RdPflNEZDataRecNew.szLyrOrdinate, m_RdPflNEZDataRecNew.szLyrAxis};
	
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_RdPflNEZDataRecNew.szLyrAnnotation, m_RdPflNEZDataRecNew.szLyrTopSurface, m_RdPflNEZDataRecNew.szLyrOrdinate, m_RdPflNEZDataRecNew.szLyrAxis};
	Dlg.GetValues(iSize, pszValsDummy);
}
void CRdPflNEZDataDlg::OnSetAnnTags() 
{
	const int iSize = 6;
	const char* pszTags[] = {"Elevation", "Northing", "Easting", "Cumulative Distance", "Distance", "Datum"};
	const char* pszVals[] = {m_RdPflNEZDataRecNew.szElevTag, m_RdPflNEZDataRecNew.szNorthingTag, m_RdPflNEZDataRecNew.szEastingTag, m_RdPflNEZDataRecNew.szCumDistTag, m_RdPflNEZDataRecNew.szDistTag, m_RdPflNEZDataRecNew.szDatumTag};
	
	CLyrSettingDlg Dlg("Set Annotation Tags", FALSE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_RdPflNEZDataRecNew.szElevTag, m_RdPflNEZDataRecNew.szNorthingTag, m_RdPflNEZDataRecNew.szEastingTag, m_RdPflNEZDataRecNew.szCumDistTag, m_RdPflNEZDataRecNew.szDistTag, m_RdPflNEZDataRecNew.szDatumTag};
	Dlg.GetValues(iSize, pszValsDummy);
}

void CRdPflNEZDataDlg::OnOK() 
{
	CString strTmp;

	//
	GetDlgItem(IDC_EDIT_EXAG_HORZ)->GetWindowText(strTmp);
	m_RdPflNEZDataRecNew.dXScale = atof(strTmp);
	if(m_RdPflNEZDataRecNew.dXScale <= 0.0)
	{
		AfxMessageBox("Invalid horizontal exagaration", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_EXAG_HORZ))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_EXAG_VERT)->GetWindowText(strTmp);
	m_RdPflNEZDataRecNew.dYScale = atof(strTmp);
	if(m_RdPflNEZDataRecNew.dYScale <= 0.0)
	{
		AfxMessageBox("Invalid vertical exagaration", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_EXAG_VERT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_EXAG_VERT))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_DATUM)->GetWindowText(strTmp);
	m_RdPflNEZDataRecNew.dDatum = atof(strTmp);
	//
	GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->GetWindowText(strTmp);
	m_RdPflNEZDataRecNew.dNumAnnSize = atof(strTmp);
	if(m_RdPflNEZDataRecNew.dNumAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NUM_TXT_SIZE))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->GetWindowText(strTmp);
	m_RdPflNEZDataRecNew.dRowAnnSize = atof(strTmp);
	if(m_RdPflNEZDataRecNew.dRowAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_LAB_TXT_SIZE))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_ROW_HT)->GetWindowText(strTmp);
	m_RdPflNEZDataRecNew.dRowHt = atof(strTmp);
	if(m_RdPflNEZDataRecNew.dRowHt <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_ROW_HT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_ROW_HT))->SetSel(0, -1);
		return;
	}
	//
	m_RdPflNEZDataRecNew.bConsider3DPts = (((CButton*)GetDlgItem(IDC_CHECK))->GetCheck() != 0);
	//
	if(m_RdPflNEZDataRec.dOrdInterval >= 0.0)
	{
		double fTmp;

		GetDlgItem(IDC_EDIT_ORD_INTERVAL)->GetWindowText(strTmp);
		fTmp = atof(strTmp);
		if(fTmp <= 0.0)
		{
			AfxMessageBox("Invalid ordinate annotation interval", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_ORD_INTERVAL)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_ORD_INTERVAL))->SetSel(0, -1);
			return;
		}
		m_RdPflNEZDataRecNew.dOrdInterval = fTmp;
		m_RdPflNEZDataRecNew.bDrawGeneratedPoints = ((CButton*)GetDlgItem(IDC_CHECK_DRAW_GEN_PTS))->GetCheck();
	}
	if(m_RdPflNEZDataRecNew.bConsider3DPts && (m_RdPflNEZDataRec.dOrdInterval > 0.0))
	{
		AfxMessageBox("This facility considering 3D coordinates is yet to be implemented!", MB_ICONSTOP);
		return;
	}
	//
	CDialog::OnOK();
}

void CRdPflNEZDataDlg::OnOutFile() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Output Files (*.TXT)|*.TXT||");

	dwFlags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(Dlg.GetFileName());
		strcpy(m_RdPflNEZDataRecNew.szOutFile, Dlg.GetPathName());
	}
}
void CRdPflNEZDataDlg::GetAttribs(RDPFLNEZDATA& Rec) 
{
	Rec = m_RdPflNEZDataRecNew;
}
//////////////////////////////////////////////////
BOOL CRdPflNEZDataDlg::GetRdPflNEZData(RDPFLNEZDATA& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CRdPflNEZDataDlg Dlg(Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(Rec);
	return TRUE;
}
