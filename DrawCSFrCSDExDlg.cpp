// DrawCSFrCSDExDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DrawCSFrCSDExDlg.h"
#include "RdXSecFrDWGDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"
#include "DrawUtil.h"
#include "Util.h"
#include "LyrSettingDlg.h"
#include "SurfaceParamDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrCSDExDlg dialog

CDrawCSFrCSDExDlg::CDrawCSFrCSDExDlg(CSDDWGPARAMEX& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CDrawCSFrCSDExDlg::IDD, pParent), m_Rec(Rec)
{
	//{{AFX_DATA_INIT(CDrawCSFrCSDExDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDrawCSFrCSDExDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDrawCSFrCSDExDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDrawCSFrCSDExDlg, CDialog)
	//{{AFX_MSG_MAP(CDrawCSFrCSDExDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_SHEET, OnSelChangeComboSheet)
	ON_BN_CLICKED(IDC_OPEN_CSD, OnOpenCSD)
	ON_BN_CLICKED(IDC_OPEN_CSD_DSGN, OnOpenCSDDsgn)
	ON_BN_CLICKED(IDC_OPEN_LOG, OnOpenLogFile)
	ON_BN_CLICKED(IDSETLYR, OnLyrSetting)
	ON_BN_CLICKED(IDSETANNTAGS, OnSetAnnTags)
	ON_BN_CLICKED(IDSET_SURF_MARKER, OnSurfMarker)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawCSFrCSDExDlg message handlers

BOOL CDrawCSFrCSDExDlg::OnInitDialog() 
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
	GetDlgItem(IDC_EDIT_FILENAME_DSGN)->SetWindowText(m_Rec.szDataFileDsgn);
	GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(m_Rec.szDataFile);
	GetDlgItem(IDC_EDIT_FILENAME_LOG)->SetWindowText(m_Rec.szDataFileOut);

	//DISABLED 
	GetDlgItem(IDC_EDIT_START_CHG)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_END_CHG)->ShowWindow(SW_HIDE);
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

	return TRUE;
}

void CDrawCSFrCSDExDlg::OnOK() 
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
	GetDlgItem(IDC_EDIT_FILENAME_DSGN)->GetWindowText(strTmp);
	if(strlen(strTmp) <= 0)
	{
		AfxMessageBox("Invalid design data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_CSD_DSGN)->SetFocus();
		return;
	}
	strcpy(m_Rec.szDataFileDsgn, strTmp);

	//
	GetDlgItem(IDC_EDIT_FILENAME_LOG)->GetWindowText(strTmp);
	//Check DISABLED temporarily
	//if(strlen(strTmp) <= 0)
	//{
	//	AfxMessageBox("Invalid output data file name", MB_ICONSTOP);
	//	GetDlgItem(IDC_EDIT_FILENAME_LOG)->SetFocus();
	//	return;
	//}
	strcpy(m_Rec.szDataFileOut, strTmp);
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

	CDialog::OnOK();
}

void CDrawCSFrCSDExDlg::OnSelChangeComboSheet() 
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
void CDrawCSFrCSDExDlg::OnOpenLogFile() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Calculation Log Files (*.TXT)|*.TXT||");

	dwFlags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME_LOG)->SetWindowText(Dlg.GetPathName());
	}
}
void CDrawCSFrCSDExDlg::OnOpenCSDDsgn() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("C/S Design Data Files (*.CSD)|*.CSD||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME_DSGN)->SetWindowText(Dlg.GetPathName());
	}
}

void CDrawCSFrCSDExDlg::OnOpenCSD() 
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
void CDrawCSFrCSDExDlg::OnSurfMarker() 
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
void CDrawCSFrCSDExDlg::OnLyrSetting() 
{
	const int iSize = 8;
	const char* pszTags[] = {"Existing Annotation", "Existing Surface", "Existing Orindates", "Axis", "Designed Annotation", "Designed Surface", "Designed Orindates", "Drawing Sheet"};
	const char* pszVals[] = {m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface, m_Rec.szLyrOrdinate, m_Rec.szLyrAxis, m_Rec.szLyrAnnotationDsgn, m_Rec.szLyrTopSurfaceDsgn, m_Rec.szLyrOrdinateDsgn, m_Rec.szDwgSheet};
	
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;
	char* pszValsDummy[] = {m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface, m_Rec.szLyrOrdinate, m_Rec.szLyrAxis, m_Rec.szLyrAnnotationDsgn, m_Rec.szLyrTopSurfaceDsgn, m_Rec.szLyrOrdinateDsgn, m_Rec.szDwgSheet};
	Dlg.GetValues(iSize, pszValsDummy);
}
void CDrawCSFrCSDExDlg::OnSetAnnTags() 
{
	const int iSize = 5;
	const char* pszTags[] = {"Offset", "Existing Elevation", "Proposed Elevation", "Datum", "C/S Label"};
	const char* pszVals[] = {m_Rec.szOffsetTag, m_Rec.szElevTag, m_Rec.szElevTagDsgn, m_Rec.szDatumTag, m_Rec.szXSecLabelTag};
	
	CLyrSettingDlg Dlg("Set Annotation Tags", FALSE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szOffsetTag, m_Rec.szElevTag, m_Rec.szElevTagDsgn, m_Rec.szDatumTag, m_Rec.szXSecLabelTag};
	Dlg.GetValues(iSize, pszValsDummy);
}

BOOL GetCSDExDwgParam(CSDDWGPARAMEX& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CDrawCSFrCSDExDlg Dlg(Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	return TRUE;
}

