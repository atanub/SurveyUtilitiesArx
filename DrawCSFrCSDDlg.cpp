// DrawCSFrCSDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DrawCSFrCSDDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"
#include "DrawUtil.h"
#include "Util.h"
#include "RdXSecFrDWGDlg.h"
#include "LyrSettingDlg.h"
#include "SurfaceParamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrCSDDlg dialog


CDrawCSFrCSDDlg::CDrawCSFrCSDDlg(CSDDWGPARAM& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CDrawCSFrCSDDlg::IDD, pParent), m_Rec(Rec)
{
	//{{AFX_DATA_INIT(CDrawCSFrCSDDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDrawCSFrCSDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDrawCSFrCSDDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDrawCSFrCSDDlg, CDialog)
	//{{AFX_MSG_MAP(CDrawCSFrCSDDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_SHEET, OnSelChangeComboSheet)
	ON_BN_CLICKED(IDC_OPEN_CSD, OnOpenCSD)
	ON_BN_CLICKED(IDSETLYR, OnLyrSetting)
	ON_BN_CLICKED(IDSETANNTAGS, OnSetAnnTags)
	ON_BN_CLICKED(IDSET_SURF_MARKER, OnSurfMarker)
	ON_BN_CLICKED(IDC_CHECK_AUTO_DATUM,	OnCheckAutoDatum)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrCSDDlg message handlers

BOOL CDrawCSFrCSDDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	// TODO: Add extra initialization here
	CString strTmp;
	{//Set Font...
		HFONT hFont;
		CComboBox* pCtrl;

		pCtrl = (CComboBox*)GetDlgItem(IDC_COMBO_SHEET);
		hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		ASSERT(hFont != 0L);
		pCtrl->SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

		CRdXSecFrDWGDlg::InitSheetArray(m_SheetSizeArray);
		CRdXSecFrDWGDlg::FillCombo(pCtrl, m_SheetSizeArray);
	}
	GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->ShowWindow((m_Rec.fOrdInterval < 0.0) ? SW_HIDE:SW_SHOW);
	GetDlgItem(IDC_STATIC_X)->ShowWindow((m_Rec.fOrdInterval < 0.0) ? SW_HIDE:SW_SHOW);
	//
	GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->EnableWindow((m_Rec.fOrdInterval >= 0.0));
	GetDlgItem(IDC_STATIC_X)->EnableWindow(m_Rec.fOrdInterval >= 0.0);
	strTmp.Format("%f", m_Rec.fOrdInterval); GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->SetWindowText(strTmp);
	//
	GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(m_Rec.szDataFile);
	
	strTmp.Format("%f", m_Rec.dStartChg);	GetDlgItem(IDC_EDIT_START_CHG)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dEndChg);		GetDlgItem(IDC_EDIT_END_CHG)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dXScale); GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dYScale); GetDlgItem(IDC_EDIT_EXAG_VERT)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dNumAnnSize);		GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dRowAnnSize);		GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dCSLabelAnnSize); GetDlgItem(IDC_EDIT_CS_LABEL_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dRowHt);			GetDlgItem(IDC_EDIT_ROW_HT)->SetWindowText(strTmp);
	
	strTmp.Format("%f", m_Rec.dSheetWid);		GetDlgItem(IDC_EDIT_SHEET_WIDTH)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dSheetHeight);	GetDlgItem(IDC_EDIT_SHEET_HEIGHT)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dNPlateWid);		GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dNPlateHeight);	GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT)->SetWindowText(strTmp);

	const BOOL bIsForAutoDatumCalc = ((m_Rec.fMinOrdinateHt > 0.0) && (m_Rec.fDatumDenominator > 0.0));
	((CButton*)GetDlgItem(IDC_CHECK_AUTO_DATUM))->SetCheck(bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_MIN_ORD_HT)->EnableWindow(bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->EnableWindow(bIsForAutoDatumCalc);
	strTmp.Format("%.3f", m_Rec.fDatumDenominator);	GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->SetWindowText(strTmp);
	strTmp.Format("%.3f", m_Rec.fMinOrdinateHt);	GetDlgItem(IDC_EDIT_MIN_ORD_HT)->SetWindowText(strTmp);

	return TRUE;
}

void CDrawCSFrCSDDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp;
	
	//
	GetDlgItem(IDC_EDIT_FILENAME)->GetWindowText(strTmp);
	if(strlen(strTmp) <= 0)
	{
		AfxMessageBox("Invalid data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_CSD)->SetFocus();
		return;
	}
	strcpy(m_Rec.szDataFile, strTmp);
	//
	if(strlen(m_Rec.szElevTag) <= 0)
	{
		AfxMessageBox("Invalid cross section label prefix", MB_ICONSTOP);
		GetDlgItem(IDSETANNTAGS)->SetFocus();
		return;
	}
	//
	if(strlen(m_Rec.szElevTag) <= 0)
	{
		AfxMessageBox("Invalid Elevation tag", MB_ICONSTOP);
		GetDlgItem(IDSETANNTAGS)->SetFocus();
		return;
	}
	//
	if(strlen(m_Rec.szOffsetTag) <= 0)
	{
		AfxMessageBox("Invalid Offset tag", MB_ICONSTOP);
		GetDlgItem(IDSETANNTAGS)->SetFocus();
		return;
	}
	//
	if(strlen(m_Rec.szDatumTag) <= 0)
	{
		AfxMessageBox("Invalid Offset tag", MB_ICONSTOP);
		GetDlgItem(IDSETANNTAGS)->SetFocus();
		return;
	}
	//
	GetDlgItem(IDC_EDIT_EXAG_VERT)->GetWindowText(strTmp);
	m_Rec.dYScale = atof(strTmp);
	if(m_Rec.dYScale <= 0.0)
	{
		AfxMessageBox("Invalid vertical exagaration", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_EXAG_VERT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_EXAG_VERT))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_EXAG_HORZ)->GetWindowText(strTmp);
	m_Rec.dXScale = atof(strTmp);
	if(m_Rec.dXScale <= 0.0)
	{
		AfxMessageBox("Invalid horizontal exagaration", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_EXAG_HORZ))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->GetWindowText(strTmp);
	m_Rec.dNumAnnSize = atof(strTmp);
	if(m_Rec.dNumAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NUM_TXT_SIZE))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->GetWindowText(strTmp);
	m_Rec.dRowAnnSize = atof(strTmp);
	if(m_Rec.dRowAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_LAB_TXT_SIZE))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_ROW_HT)->GetWindowText(strTmp);
	m_Rec.dRowHt = atof(strTmp);
	if(m_Rec.dRowHt <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_ROW_HT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_ROW_HT))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_CS_LABEL_SIZE)->GetWindowText(strTmp);
	m_Rec.dCSLabelAnnSize = atof(strTmp);
	if(m_Rec.dCSLabelAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_CS_LABEL_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_CS_LABEL_SIZE))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_START_CHG)->GetWindowText(strTmp);
	m_Rec.dStartChg = atof(strTmp);
	//
	GetDlgItem(IDC_EDIT_END_CHG)->GetWindowText(strTmp);
	m_Rec.dEndChg = atof(strTmp);

	//
	GetDlgItem(IDC_EDIT_SHEET_WIDTH)->GetWindowText(strTmp);
	m_Rec.dSheetWid = atof(strTmp);
	if(m_Rec.dSheetWid <= 0.0)
	{
		AfxMessageBox("Invalid sheet width", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_SHEET_WIDTH)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_SHEET_WIDTH))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_SHEET_HEIGHT)->GetWindowText(strTmp);
	m_Rec.dSheetHeight = atof(strTmp);
	if(m_Rec.dSheetHeight <= 0.0)
	{
		AfxMessageBox("Invalid sheet height", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_SHEET_HEIGHT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_SHEET_HEIGHT))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH)->GetWindowText(strTmp);
	m_Rec.dNPlateWid = atof(strTmp);
	if(m_Rec.dNPlateWid <= 0.0)
	{
		AfxMessageBox("Invalid name plate width", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT)->GetWindowText(strTmp);
	m_Rec.dNPlateHeight = atof(strTmp);
	if(m_Rec.dNPlateHeight <= 0.0)
	{
		AfxMessageBox("Invalid name plate height", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT))->SetSel(0, -1);
		return;
	}
	if(m_Rec.dNPlateHeight >= m_Rec.dSheetHeight)
	{
		AfxMessageBox("Invalid name plate height", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT))->SetSel(0, -1);
		return;
	}
	if(m_Rec.dNPlateWid >= m_Rec.dSheetWid)
	{
		AfxMessageBox("Invalid name plate width", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->GetWindowText(strTmp);
	m_Rec.fOrdInterval = atof(strTmp);
	//
	const BOOL bIsForAutoDatumCalc = ((CButton*)GetDlgItem(IDC_CHECK_AUTO_DATUM))->GetCheck();
	if(bIsForAutoDatumCalc)
	{//
		GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->GetWindowText(strTmp);
		m_Rec.fDatumDenominator = atof(strTmp);	
		if(m_Rec.fDatumDenominator <= 0.0)
		{
			AfxMessageBox("Invalid Datum Denominator", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR))->SetSel(0, -1);
			return;
		}
		//
		GetDlgItem(IDC_EDIT_MIN_ORD_HT)->GetWindowText(strTmp);
		m_Rec.fMinOrdinateHt = atof(strTmp);	
		if(m_Rec.fMinOrdinateHt <= 0.0)
		{
			AfxMessageBox("Invalid Minimum Ordinate Height", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_MIN_ORD_HT)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_MIN_ORD_HT))->SetSel(0, -1);
			return;
		}
		if((m_Rec.fMinOrdinateHt > 0.0) || (m_Rec.fDatumDenominator > 0.0))
		{
			AfxMessageBox("Automatic Datum calculation is yet to be implemented!", MB_ICONSTOP);
			return;
		}
	}
	else
	{
		m_Rec.fMinOrdinateHt = 0.0;
		m_Rec.fDatumDenominator = 0.0;
	}

	CDialog::OnOK();
}
void CDrawCSFrCSDDlg::OnCheckAutoDatum()
{
	const BOOL bIsForAutoDatumCalc = ((CButton*)GetDlgItem(IDC_CHECK_AUTO_DATUM))->GetCheck();

	GetDlgItem(IDC_EDIT_MIN_ORD_HT)->EnableWindow(bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->EnableWindow(bIsForAutoDatumCalc);
}

void CDrawCSFrCSDDlg::OnSelChangeComboSheet() 
{
	CComboBox* pCtrl;
	int iData, iIndex;
	CString strTmp;
	double fWid, fHt;

	pCtrl = (CComboBox*)GetDlgItem(IDC_COMBO_SHEET);
	iIndex = pCtrl->GetCurSel();
	if(iIndex == CB_ERR)
		return;
	iData = pCtrl->GetItemData(iIndex);
	strTmp = m_SheetSizeArray[iData];
	CRdXSecFrDWGDlg::ParseSheetSizeDataRec(strTmp, fWid, fHt);

	strTmp.Format("%f", fWid);	GetDlgItem(IDC_EDIT_SHEET_WIDTH)->SetWindowText(strTmp);
	strTmp.Format("%f", fHt);	GetDlgItem(IDC_EDIT_SHEET_HEIGHT)->SetWindowText(strTmp);
}
void CDrawCSFrCSDDlg::OnOpenCSD() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("C/S Data Files (*.CSD)|*.CSD||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(Dlg.GetPathName());
	}
}
void CDrawCSFrCSDDlg::OnSetAnnTags() 
{
	const int iSize = 4;
	const char* pszTags[] = {"Offset", "Elevation", "Datum", "C/S Label"};
	const char* pszVals[] = {m_Rec.szOffsetTag, m_Rec.szElevTag, m_Rec.szDatumTag, m_Rec.szXSecLabelTag};
	
	CLyrSettingDlg Dlg("Set Annotation Tags", FALSE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szOffsetTag, m_Rec.szElevTag, m_Rec.szDatumTag, m_Rec.szXSecLabelTag};
	Dlg.GetValues(iSize, pszValsDummy);
}
void CDrawCSFrCSDDlg::OnSurfMarker() 
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
void CDrawCSFrCSDDlg::OnLyrSetting() 
{
	const int iSize = 4;
	const char* pszTags[] = {"Existing Annotation", "Existing Surface", "Existing Orindates", "Axis"};
	const char* pszVals[] = {m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface, m_Rec.szLyrOrdinate, m_Rec.szLyrAxis};
	
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;
	char* pszValsDummy[] = {m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface, m_Rec.szLyrOrdinate, m_Rec.szLyrAxis};
	Dlg.GetValues(iSize, pszValsDummy);
}

BOOL GetCSDDwgParam(CSDDWGPARAM& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CDrawCSFrCSDDlg Dlg(Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;


	return TRUE;
}

