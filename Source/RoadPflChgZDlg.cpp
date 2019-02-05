// RoadPflChgZDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RoadPflChgZDlg.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"
#include "LyrSettingDlg.h"
#include "SurfaceParamDlg.h"

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CRoadPflChgZDlg dialog


CRoadPflChgZDlg::CRoadPflChgZDlg(const RDPFL_CHG_Z_DWG_PARAM& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CRoadPflChgZDlg::IDD, pParent), m_Rec(Rec)
{
	//{{AFX_DATA_INIT(CRoadPflChgZDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRoadPflChgZDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoadPflChgZDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoadPflChgZDlg, CDialog)
	//{{AFX_MSG_MAP(CRoadPflChgZDlg)
	ON_BN_CLICKED(IDC_BUTTON_LYR,				OnLyrSetting)
	ON_BN_CLICKED(IDC_BUTTON_ANN_TAGS,			OnSetAnnTags)
	ON_BN_CLICKED(IDSET_SURF_MARKER, OnSurfMarker)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoadPflChgZDlg message handlers

BOOL CRoadPflChgZDlg::OnInitDialog() 
{
	CString strTmp;
	
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	strTmp.Format("%f", m_Rec.dXScale); GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dYScale); GetDlgItem(IDC_EDIT_EXAG_VERT)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dDatum);	GetDlgItem(IDC_EDIT_DATUM)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dLabTxtSize);		GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dNumAnnTxtSize);	GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dRowHt);			GetDlgItem(IDC_EDIT_ROW_HT)->SetWindowText(strTmp);
	
	strTmp.Format("%f", m_Rec.fOrdInterval);	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.fTxtAnnotationAngleInDeg);	GetDlgItem(IDC_EDIT_TXT_ANGLE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.fTxtAnnotationMargin);		GetDlgItem(IDC_EDIT_TXT_MARGIN)->SetWindowText(strTmp);
	/*
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->ShowWindow((m_Rec.fOrdInterval > 0.0) ? SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->ShowWindow((m_Rec.fOrdInterval > 0.0) ? SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->EnableWindow((m_Rec.fOrdInterval > 0.0));
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->EnableWindow((m_Rec.fOrdInterval > 0.0));
	*/
	GetDlgItem(IDC_EDIT_ORD_INTERVAL)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ORD_INTERVAL)->ShowWindow(SW_SHOW);
	
	return TRUE;
}
void CRoadPflChgZDlg::OnSurfMarker() 
{
	CStringArray strArrayBlocks;

	GetBlockNames(strArrayBlocks);
	if(strArrayBlocks.GetSize() <= 0)
	{
		AfxMessageBox("No block definition found in current drawing\nCreate block first & then try again", MB_ICONSTOP);
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
void CRoadPflChgZDlg::OnLyrSetting() 
{
	const char* pszTags[] = 
	{
		"Axis", 
		"Orindates", 
		"Annotation", 
		"Existing Top Surface", 
		"Proposed Top Surface", 
		"Existing Designed Top Surface", 
		"Top Surface Annotation"
	};
	const char* pszVals[] = 
	{
		m_Rec.szLyrAxis, 
		m_Rec.szLyrOrdinate, 
		m_Rec.szLyrAnnotation, 
		m_Rec.szLyrTopSurfaceOrg, 
		m_Rec.szLyrTopSurfaceDsgn, 
		m_Rec.szLyrTopSurfaceDsgnExisting, 
		m_Rec.szLyrAnnotationEx
	};
	
	const int iSize = sizeof(pszTags)/sizeof(char*);
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szLyrAxis, m_Rec.szLyrOrdinate, m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurfaceOrg, m_Rec.szLyrTopSurfaceDsgn, m_Rec.szLyrAnnotationEx};
	Dlg.GetValues(iSize, pszValsDummy);
}
void CRoadPflChgZDlg::OnSetAnnTags() 
{
	const int iSize = 5;
	const char* pszTags[] = 
	{
		"Existing Elevation", 
		"Proposed Elevation", 
		"Existing Design Elevation", 
		"Cut/Fill", 
		"Design Elevation Difference", 
		"Chainage", 
		"Datum"
	};
	const char* pszVals[] = 
	{
		m_Rec.szXElevTag, 
		m_Rec.szDsgnElevTag, 
		m_Rec.szExistingDsgnElevTag, 
		m_Rec.szCutFillTag, 
		m_Rec.szDsgnElevDifferenceTag, 
		m_Rec.szChgTag, 
		m_Rec.szDatum
	};

	CLyrSettingDlg Dlg("Set Annotation Tags", FALSE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szXElevTag, m_Rec.szDsgnElevTag, m_Rec.szCutFillTag, m_Rec.szChgTag, m_Rec.szDatum};
	Dlg.GetValues(iSize, pszValsDummy);
}

void CRoadPflChgZDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTemp;
	UINT iID;
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
	m_Rec.dLabTxtSize = atof(strTemp);
	if(m_Rec.dLabTxtSize <= 0.0)
	{
		AfxMessageBox("Invalid value found", MB_ICONSTOP);
		GetDlgItem(iID)->SetFocus();
		((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
		return;
	}
	//
	iID = IDC_EDIT_NUM_TXT_SIZE;
	GetDlgItem(iID)->GetWindowText(strTemp);
	m_Rec.dNumAnnTxtSize = atof(strTemp);
	if(m_Rec.dNumAnnTxtSize <= 0.0)
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
	iID = IDC_EDIT_TXT_ANGLE;
	GetDlgItem(iID)->GetWindowText(strTemp);
	{//
		m_Rec.fTxtAnnotationAngleInDeg = atof(strTemp);
		/*
		if(m_Rec.fTxtAnnotationAngleInDeg <= 0.0)
		{
			AfxMessageBox("Invalid value found", MB_ICONSTOP);
			GetDlgItem(iID)->SetFocus();
			((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
			return;
		}
		*/
	}
	//
	iID = IDC_EDIT_TXT_MARGIN;
	GetDlgItem(iID)->GetWindowText(strTemp);
	{//
		m_Rec.fTxtAnnotationMargin = atof(strTemp);
		if(m_Rec.fTxtAnnotationMargin <= 0.0)
		{
			AfxMessageBox("Invalid value found", MB_ICONSTOP);
			GetDlgItem(iID)->SetFocus();
			((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
			return;
		}
	}
	//
	iID = IDC_EDIT_ORD_INTERVAL;
	GetDlgItem(iID)->GetWindowText(strTemp);
	{//
		m_Rec.fOrdInterval = atof(strTemp);
		if(m_Rec.fOrdInterval <= 0.0)
		{
			AfxMessageBox("Invalid value found", MB_ICONSTOP);
			GetDlgItem(iID)->SetFocus();
			((CEdit*)GetDlgItem(iID))->SetSel(0, -1);
			return;
		}
	}

	CDialog::OnOK();
}
void CRoadPflChgZDlg::GetAttribs(RDPFL_CHG_Z_DWG_PARAM& Rec)
{
	Rec = m_Rec;
}

BOOL GetXLSRoadPflChgZParam(RDPFL_CHG_Z_DWG_PARAM& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CRoadPflChgZDlg Dlg(Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(Rec);
	return TRUE;
}

