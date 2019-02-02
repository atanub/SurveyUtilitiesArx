// RdXSecFrDWGDlg.cpp : implementation file
//

#include "Stdafx.h"
#include "RdXSecFrDWGDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"
#include "DrawUtil.h"
#include "LyrSettingDlg.h"
#include "Util.h"
#include "SurfaceParamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();

static BOOL IsValueDefined(const double&, const AcGePoint3dArray&);
static int GetInsertionIndex(const double&, const AcGePoint3dArray&);
static void SmoothDatumRangeArray(AcGePoint3dArray&);

const char	CRdXSecFrDWGDlg::m_chShAttrSeparetor = '\t';
const char* CRdXSecFrDWGDlg::m_pszArrayOfPIDPathNames[] = {"Don't Draw", "Draw PI - Min Elev", "Draw PI - Max Elev"};
const int	CRdXSecFrDWGDlg::m_iArrayOfPIDPathVals[] = {0, 1, 4};
/////////////////////////////////////////////////////////////////////////////
// CRdXSecFrDWGDlg dialog

CRdXSecFrDWGDlg::CRdXSecFrDWGDlg(RDXSECOFFELEVDATA& Rec, AcGePoint3dArray& Array, CWnd* pParent /*=NULL*/)
	: CDialog(CRdXSecFrDWGDlg::IDD, pParent), m_Rec(Rec), m_DatumRangeArray(Array)
{
	//{{AFX_DATA_INIT(CRdXSecFrDWGDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CRdXSecFrDWGDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRdXSecFrDWGDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRdXSecFrDWGDlg, CDialog)
	//{{AFX_MSG_MAP(CRdXSecFrDWGDlg)
	ON_BN_CLICKED(IDC_BUTTON_ADD_RANGE, OnButtonAddRange)
	ON_BN_CLICKED(IDC_BUTTON_DEL_RANGE, OnButtonDelRange)
	ON_LBN_DBLCLK(IDC_LIST_LEV_RANGE,	OnDblClkListLevRange)
	ON_LBN_SELCHANGE(IDC_LIST_LEV_RANGE, OnSelChangeListLevRange)
	ON_CBN_SELCHANGE(IDC_COMBO_SHEET,	OnSelChangeComboSheet)
	ON_BN_CLICKED(IDSETLYR,				OnLyrSetting)
	ON_BN_CLICKED(IDSETANNTAGS,			OnSetAnnTags)
	ON_BN_CLICKED(IDC_CHECK_AUTO_DATUM,	OnCheckAutoDatum)
	ON_BN_CLICKED(IDSET_SURF_MARKER,	OnSurfMarker)
	ON_BN_CLICKED(IDC_OPEN_CSD_FILE,	OnOpenCSDOutFile)
	ON_BN_CLICKED(IDC_OPEN_PID_FILE,	OnOpenPIDOutFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRdXSecFrDWGDlg message handlers
void CRdXSecFrDWGDlg::OnCheckAutoDatum()
{
	const BOOL bIsForAutoDatumCalc = ((CButton*)GetDlgItem(IDC_CHECK_AUTO_DATUM))->GetCheck();

	GetDlgItem(IDC_EDIT_MIN_ORD_HT)->EnableWindow(bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->EnableWindow(bIsForAutoDatumCalc);
	
	GetDlgItem(IDC_EDIT_DATUM_START)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_END)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_ELEV)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_BUTTON_ADD_RANGE)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_BUTTON_DEL_RANGE)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_LIST_LEV_RANGE)->EnableWindow(!bIsForAutoDatumCalc);
}

void CRdXSecFrDWGDlg::OnOpenCSDOutFile() 
{
	DWORD dwFlags;
	CString strFile;
	const CString strConstFileType = CString("Text Files (*.TXT)|*.TXT|All Files (*.*)|*.*||");

	dwFlags = OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_EXPLORER|OFN_LONGNAMES|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFileType);
	Dlg.m_ofn.lpstrTitle = "Select CSD output file name";
	if(Dlg.DoModal() != IDOK)
		return;
	GetDlgItem(IDC_EDIT_FILENAME_CSD)->SetWindowText(Dlg.GetPathName());
}
void CRdXSecFrDWGDlg::OnOpenPIDOutFile() 
{
	DWORD dwFlags;
	CString strFile;
	const CString strConstFileType = CString("Text Files (*.TXT)|*.TXT|All Files (*.*)|*.*||");
	
	dwFlags = OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_EXPLORER|OFN_LONGNAMES|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(FALSE, 0L, 0L, dwFlags, strConstFileType);
	Dlg.m_ofn.lpstrTitle = "Select PID output file name";
	if(Dlg.DoModal() != IDOK)
		return;
	GetDlgItem(IDC_EDIT_FILENAME_PID)->SetWindowText(Dlg.GetPathName());
}
BOOL CRdXSecFrDWGDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;
	{//Set Font...
		HFONT hFont;
		CListBox* pCtrl;

		pCtrl = (CListBox*)GetDlgItem(IDC_LIST_LEV_RANGE);
		hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		ASSERT(hFont != 0L);
		pCtrl->SendMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}
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
	GetDlgItem(IDC_CHECK_DRAW_GEN_PTS)->ShowWindow((m_Rec.fOrdInterval < 0.0) ? SW_HIDE:SW_SHOW);
	GetDlgItem(IDC_STATIC_X)->ShowWindow((m_Rec.fOrdInterval < 0.0) ? SW_HIDE:SW_SHOW);

	GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->EnableWindow((m_Rec.fOrdInterval >= 0.0));
	GetDlgItem(IDC_CHECK_DRAW_GEN_PTS)->EnableWindow((m_Rec.fOrdInterval >= 0.0));
	GetDlgItem(IDC_STATIC_X)->EnableWindow(m_Rec.fOrdInterval >= 0.0);
	strTmp.Format("%f", m_Rec.fOrdInterval); GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->SetWindowText(strTmp);
	((CButton*)GetDlgItem(IDC_CHECK_DRAW_GEN_PTS))->SetCheck(m_Rec.bDrawGeneratedPoints);

	strTmp.Format("%f", m_Rec.dXScale); GetDlgItem(IDC_EDIT_EXAG_HORZ)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dYScale); GetDlgItem(IDC_EDIT_EXAG_VERT)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dNumAnnSize);		GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dRowAnnSize);		GetDlgItem(IDC_EDIT_LAB_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dRowHt);			GetDlgItem(IDC_EDIT_ROW_HT)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dCSLabelAnnSize); GetDlgItem(IDC_EDIT_CS_LABEL_TXT)->SetWindowText(strTmp);
	
	strTmp.Format("%f", 0.0); GetDlgItem(IDC_EDIT_DATUM_START)->SetWindowText(strTmp);
	strTmp.Format("%f", 0.0); GetDlgItem(IDC_EDIT_DATUM_END)->SetWindowText(strTmp);
	strTmp.Format("%f", 0.0); GetDlgItem(IDC_EDIT_DATUM_ELEV)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dSheetWid);		GetDlgItem(IDC_EDIT_SHEET_WIDTH)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dSheetHeight);	GetDlgItem(IDC_EDIT_SHEET_HEIGHT)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dNPlateWid);		GetDlgItem(IDC_EDIT_NAMEPLATE_WIDTH)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dNPlateHeight);	GetDlgItem(IDC_EDIT_NAMEPLATE_HEIGHT)->SetWindowText(strTmp);

	strTmp.Format("%f", m_Rec.dStartChg);	GetDlgItem(IDC_EDIT_START_CHG)->SetWindowText(strTmp);
	
	GetDlgItem(IDC_EDIT_FILENAME_CSD)->SetWindowText(m_Rec.szOutFileCSD);
	GetDlgItem(IDC_EDIT_FILENAME_PID)->SetWindowText(m_Rec.szOutFilePID);

	m_Rec.bDrawGeneratedPoints = ((CButton*)GetDlgItem(IDC_CHECK_DRAW_GEN_PTS))->GetCheck();

	_IntializeDatumRange();
	const BOOL bIsForAutoDatumCalc = (m_DatumRangeArray.length() > 0);
	((CButton*)GetDlgItem(IDC_CHECK_AUTO_DATUM))->SetCheck(bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_MIN_ORD_HT)->EnableWindow(bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->EnableWindow(bIsForAutoDatumCalc);
	strTmp.Format("%f", m_Rec.fDatumDenominator);	GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.fMinOrdinateHt);		GetDlgItem(IDC_EDIT_MIN_ORD_HT)->SetWindowText(strTmp);
	//
	GetDlgItem(IDC_EDIT_DATUM_START)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_END)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_EDIT_DATUM_ELEV)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_BUTTON_ADD_RANGE)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_BUTTON_DEL_RANGE)->EnableWindow(!bIsForAutoDatumCalc);
	GetDlgItem(IDC_LIST_LEV_RANGE)->EnableWindow(!bIsForAutoDatumCalc);

#ifdef _DEBUG
	GetDlgItem(IDC_EDIT_SHEET_WIDTH)->SetWindowText("10000.0");
	GetDlgItem(IDC_EDIT_SHEET_HEIGHT)->SetWindowText("10000.0");
	GetDlgItem(IDC_EDIT_EXAG_VERT)->SetWindowText("5.0");
#endif// _DEBUG

	{//Populate the PID Path selector combo
		int i, iIndex;
		CComboBox* pCombo;

		pCombo = ((CComboBox*)GetDlgItem(IDC_COMBO_DRAW_MINMAX_ELEV_PATH));
		pCombo->ResetContent();
		for(i = 0; i < sizeof(CRdXSecFrDWGDlg::m_pszArrayOfPIDPathNames)/sizeof(char*); i++)
		{
			iIndex = pCombo->AddString(CRdXSecFrDWGDlg::m_pszArrayOfPIDPathNames[i]);
			pCombo->SetItemData(iIndex, CRdXSecFrDWGDlg::m_iArrayOfPIDPathVals[i]);
		}
		for(i = 0; i < pCombo->GetCount(); i++)
		{
			iIndex = pCombo->GetItemData(i);
			if(iIndex == m_Rec.iDrawPathWIthMinMaxElev)
			{
				pCombo->SetCurSel(i);
				break;
			}
		}
	}
	((CButton*)GetDlgItem(IDC_CHECK_NE_ANN_SECTION))->SetCheck(m_Rec.bDrawNorthingEastingAnnSection);
	return TRUE;
}

void CRdXSecFrDWGDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp;
	
	//
	GetDlgItem(IDC_EDIT_EXAG_VERT)->GetWindowText(strTmp);
	m_Rec.dYScale = atof(strTmp);
	if(m_Rec.dYScale <= 0.0)
	{
		AfxMessageBox("Invalid vertical exaggeration", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_EXAG_VERT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_EXAG_VERT))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_EXAG_HORZ)->GetWindowText(strTmp);
	m_Rec.dXScale = atof(strTmp);
	if(m_Rec.dXScale <= 0.0)
	{
		AfxMessageBox("Invalid horizontal exaggeration", MB_ICONSTOP);
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
	GetDlgItem(IDC_EDIT_CS_LABEL_TXT)->GetWindowText(strTmp);
	m_Rec.dCSLabelAnnSize = atof(strTmp);
	if(m_Rec.dCSLabelAnnSize <= 0.0)
	{
		AfxMessageBox("Invalid text size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_CS_LABEL_TXT)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_CS_LABEL_TXT))->SetSel(0, -1);
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
	//Sheet Detail
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
	//
	GetDlgItem(IDC_EDIT_START_CHG)->GetWindowText(strTmp);
	m_Rec.dStartChg = atof(strTmp);
	//
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
	GetDlgItem(IDC_EDIT_FILENAME_CSD)->GetWindowText(strTmp);
	strcpy(m_Rec.szOutFileCSD, strTmp);
	//
	GetDlgItem(IDC_EDIT_FILENAME_PID)->GetWindowText(strTmp);
	strcpy(m_Rec.szOutFilePID, strTmp);
	//
	if((strlen(m_Rec.szOutFileCSD) <= 0) || (strlen(m_Rec.szOutFilePID) <= 0))
	{
		if((strlen(m_Rec.szOutFileCSD) <= 0) && (strlen(m_Rec.szOutFilePID) <= 0))
		{
		}
		else
		{
			AfxMessageBox("Invalid output file name specified", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_FILENAME_CSD)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_FILENAME_CSD))->SetSel(0, -1);
			return;
		}
	}
	//
	m_Rec.iDrawPathWIthMinMaxElev = ((CComboBox*)GetDlgItem(IDC_COMBO_DRAW_MINMAX_ELEV_PATH))->GetCurSel();
	if(m_Rec.iDrawPathWIthMinMaxElev == CB_ERR)
	{
		return;
	}
	m_Rec.iDrawPathWIthMinMaxElev = ((CComboBox*)GetDlgItem(IDC_COMBO_DRAW_MINMAX_ELEV_PATH))->GetItemData(m_Rec.iDrawPathWIthMinMaxElev);
	//
	GetDlgItem(IDC_EDIT_ORDINATE_INTERVAL)->GetWindowText(strTmp);
	m_Rec.fOrdInterval = atof(strTmp);
	//Make sure following validation code should be executed at the LAST
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
		if(m_Rec.fDatumDenominator >= m_Rec.fMinOrdinateHt)
		{
			AfxMessageBox("Invalid Datum Denominator", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_DATUM_DENOMINATOR))->SetSel(0, -1);
			return;
		}
		while(m_DatumRangeArray.length() > 0)
		{
			m_DatumRangeArray.removeFirst();
		}
	}
	else if(m_DatumRangeArray.length() <= 0)
	{
		AfxMessageBox("Datum range not defined", MB_ICONSTOP);
		GetDlgItem(IDC_LIST_LEV_RANGE)->SetFocus();
		return;
	}
	SmoothDatumRangeArray(m_DatumRangeArray);
	m_Rec.bDrawNorthingEastingAnnSection = ((CButton*)GetDlgItem(IDC_CHECK_NE_ANN_SECTION))->GetCheck();

	CDialog::OnOK();
}
void CRdXSecFrDWGDlg::OnSelChangeComboSheet() 
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

void CRdXSecFrDWGDlg::OnLyrSetting() 
{
	const char* pszTags[] = {"Annotation", "Top Surface", "Orindates", "Axis", "Min/Max PID Path"};
	const char* pszVals[] = {m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface, m_Rec.szLyrOrdinate, m_Rec.szLyrAxis, m_Rec.szLyrMinMaxPIDPath};
	const int iSize = sizeof(pszVals)/sizeof(char*);
	
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szLyrAnnotation, m_Rec.szLyrTopSurface, m_Rec.szLyrOrdinate, m_Rec.szLyrAxis, m_Rec.szLyrMinMaxPIDPath};
	Dlg.GetValues(iSize, pszValsDummy);
}
void CRdXSecFrDWGDlg::OnSetAnnTags() 
{
	const int iSize = 6;
	const char* pszTags[] = {"Elevation", "Northing", "Easting", "Offset", "Datum Prefix", "Datum Suffix", "C/S Label"};
	const char* pszVals[] = {m_Rec.szElevTag, m_Rec.szNorthingTag, m_Rec.szEastingTag, m_Rec.szOffsetTag, m_Rec.szDatumTagPrefix, m_Rec.szDatumTagSuffix, m_Rec.szXSecLabelTag};
	
	CLyrSettingDlg Dlg("Set Annotation Tags", FALSE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;

	char* pszValsDummy[] = {m_Rec.szElevTag, m_Rec.szNorthingTag, m_Rec.szEastingTag, m_Rec.szOffsetTag, m_Rec.szDatumTagPrefix, m_Rec.szDatumTagSuffix, m_Rec.szXSecLabelTag};
	Dlg.GetValues(iSize, pszValsDummy);
}

void CRdXSecFrDWGDlg::_IntializeDatumRange() 
{
	const CString strRangeDispFmt = "%12.3f - %12.3f";
	CString strTmp;
	CListBox* pCtrl;
	int i, iIndex;

	pCtrl = (CListBox*)GetDlgItem(IDC_LIST_LEV_RANGE);
	pCtrl->ResetContent();
	for(i = 0; i < m_DatumRangeArray.length(); i++)
	{
		strTmp.Format(strRangeDispFmt, m_DatumRangeArray[i].x, m_DatumRangeArray[i].y);
		iIndex = pCtrl->AddString(strTmp);
		pCtrl->SetItemData(iIndex, i);
	}
}
void CRdXSecFrDWGDlg::OnButtonAddRange() 
{
	const CString strRangeDispFmt = "%12.3f - %12.3f";
	CString strTmp;
	double fStart, fEnd, fElev;
	CListBox* pCtrl;
	int iIndex, iIndexNew;

	GetDlgItem(IDC_EDIT_DATUM_START)->GetWindowText(strTmp);
	fStart = atof(strTmp);
	GetDlgItem(IDC_EDIT_DATUM_END)->GetWindowText(strTmp);
	fEnd = atof(strTmp);
	GetDlgItem(IDC_EDIT_DATUM_ELEV)->GetWindowText(strTmp);
	fElev = atof(strTmp);

	if(fStart >= fEnd)
	{
		AfxMessageBox("ERROR: Invalid range data", MB_ICONSTOP);
		return;
	}
	pCtrl = (CListBox*)GetDlgItem(IDC_LIST_LEV_RANGE);
	if(m_DatumRangeArray.length() <= 0)
	{
		if(pCtrl->GetCount() != 0)
		{
			AfxMessageBox("FATAL ERROR: Contact Developer!!", MB_ICONSTOP);
			return;
		}
		strTmp.Format(strRangeDispFmt, fStart, fEnd);
		iIndex = pCtrl->AddString(strTmp);
		m_DatumRangeArray.append(AcGePoint3d(fStart, fEnd, fElev));
		pCtrl->SetItemData(iIndex, m_DatumRangeArray.length() - 1);

		return;
	}
	if(IsValueDefined(fStart, m_DatumRangeArray) || IsValueDefined(fEnd, m_DatumRangeArray))
	{
		AfxMessageBox("ERROR: Range already defined !", MB_ICONSTOP);
		return;
	}

	iIndexNew = GetInsertionIndex(fStart, m_DatumRangeArray);

	strTmp.Format(strRangeDispFmt, fStart, fEnd);
	iIndex = pCtrl->InsertString(iIndexNew, strTmp);
    m_DatumRangeArray.insertAt(iIndexNew, AcGePoint3d(fStart, fEnd, fElev));
	pCtrl->SetItemData(iIndex, iIndexNew);
}


void CRdXSecFrDWGDlg::OnButtonDelRange() 
{
	CListBox* pCtrl;
	int iIndex, iArrayIndex;

	pCtrl = (CListBox*)GetDlgItem(IDC_LIST_LEV_RANGE);
	iIndex = pCtrl->GetCurSel();
	if(iIndex == CB_ERR)
	{
		AfxMessageBox("ERROR: Nothing selected for deletion", MB_ICONSTOP);
		return;
	}
	iArrayIndex = pCtrl->GetItemData(iIndex);
	iIndex = pCtrl->DeleteString(iIndex);

    m_DatumRangeArray.removeAt(iArrayIndex);
}
void CRdXSecFrDWGDlg::OnDblClkListLevRange() 
{
	OnButtonDelRange();
}
void CRdXSecFrDWGDlg::OnSurfMarker() 
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
void CRdXSecFrDWGDlg::OnSelChangeListLevRange() 
{
	CString strTmp;
	CListBox* pCtrl;
	int iIndex, iArrayIndex;

	pCtrl = (CListBox*)GetDlgItem(IDC_LIST_LEV_RANGE);
	iIndex = pCtrl->GetCurSel();
	if(iIndex == CB_ERR)
	{
		return;
	}
	iArrayIndex = pCtrl->GetItemData(iIndex);

	strTmp.Format("%f", m_DatumRangeArray[iArrayIndex].x); GetDlgItem(IDC_EDIT_DATUM_START)->SetWindowText(strTmp);
	strTmp.Format("%f", m_DatumRangeArray[iArrayIndex].y); GetDlgItem(IDC_EDIT_DATUM_END)->SetWindowText(strTmp);
	strTmp.Format("%f", m_DatumRangeArray[iArrayIndex].z); GetDlgItem(IDC_EDIT_DATUM_ELEV)->SetWindowText(strTmp);
}

static BOOL IsValueDefined(const double& fVal, const AcGePoint3dArray& Array)
{
	int i;
	BOOL bIsDefined;

	for(i = 0; i < Array.length(); i++)
	{
		bIsDefined = (fVal > Array[i].x) && (fVal < Array[i].y);
		if(bIsDefined)
			return TRUE;
		if((fVal == Array[i].x) && (fVal == Array[i].y))
			return TRUE;
	}
	return FALSE;
	//Old
	//	if((fVal <= Array[i].y) && (fVal >= Array[i].x))
	//		return TRUE;
}
static int GetInsertionIndex(const double& fVal, const AcGePoint3dArray& Array)
{
	int i;

	for(i = Array.length() - 1; i >= 0; i--)
	{
		if((i == Array.length() - 1) && (fVal >= Array[i].y))
			return Array.length();
		
		if((fVal > Array[i].x) && (fVal >= Array[i].y))
		{
			return i+1;
		}
	}
	return 0;
}
static void SmoothDatumRangeArray(AcGePoint3dArray& Array)
{
	int i;

	for(i = 1; i < Array.length(); i++)
	{
		if(Array[i].x != Array[i-1].y)
		{
			AcGePoint3d& Pt = Array[i];
			Pt.x = Array[i-1].y;
		}
	}
}
void CRdXSecFrDWGDlg::FillCombo(CComboBox* pCtrl, const CStringArray& SheetSizeArray)
{
	int i, iIndex;
	CString strGUIFmt;
	
	pCtrl->ResetContent();
	for(i = 0; i < SheetSizeArray.GetSize(); i++)
	{
		strGUIFmt = ConvertInternalFmt2GUIFmt(SheetSizeArray[i]);
		iIndex = pCtrl->AddString(strGUIFmt);
		pCtrl->SetItemData(iIndex, i);
	}
}
void CRdXSecFrDWGDlg::InitSheetArray(CStringArray& SheetSizeArray)
{
	if(SheetSizeArray.GetSize() != 0)
	{
		AfxMessageBox("Some error occured...Control should not reach here !", MB_ICONSTOP);
		return;
	}
	CString strTmp;

	strTmp.Format("%s%c%-4.1f%c%-4.1f", "A0", m_chShAttrSeparetor, 1189.0, m_chShAttrSeparetor, 841.0);SheetSizeArray.Add(strTmp);
	strTmp.Format("%s%c%-4.1f%c%-4.1f", "A1", m_chShAttrSeparetor,  841.0, m_chShAttrSeparetor, 595.0);SheetSizeArray.Add(strTmp);
	strTmp.Format("%s%c%-4.1f%c%-4.1f", "A2", m_chShAttrSeparetor,  595.0, m_chShAttrSeparetor, 421.0);SheetSizeArray.Add(strTmp);
	strTmp.Format("%s%c%-4.1f%c%-4.1f", "A3", m_chShAttrSeparetor,  421.0, m_chShAttrSeparetor, 297.0);SheetSizeArray.Add(strTmp);
	strTmp.Format("%s%c%-4.1f%c%-4.1f", "A4", m_chShAttrSeparetor,  297.0, m_chShAttrSeparetor, 421.0);SheetSizeArray.Add(strTmp);
}
void CRdXSecFrDWGDlg::ParseSheetSizeDataRec(const CString& strData, double& fWid, double& fHt)
{
	CStringArray strArray;

	fWid = fHt = 0.0;
	if(!XTractWordsFromLine(strData, strArray))
		return;
	
	if(strArray.GetSize() != 3)
		return;

	fWid = atof(strArray[1]);
	fHt = atof(strArray[2]);
}
CString CRdXSecFrDWGDlg::ConvertInternalFmt2GUIFmt(const CString& strParam)
{
	CString strToRet;
	int i, iSepCount;

	for(i = 0, iSepCount = 0; i < (int)strlen(strParam); i++)
	{
		if(strParam[i] == m_chShAttrSeparetor)
		{
			switch(iSepCount)
			{
			case 0:
				strToRet += "  ";
				break;
			case 1:
				strToRet += " X ";
				break;
			default:
				AfxMessageBox("Some error occured...Control should not reach here !", MB_ICONSTOP);
				return strToRet;
			}
			iSepCount++;
		}
		else
		{
			strToRet += strParam[i];
		}
	}
	return strToRet;
}

//////////////////////////////////////////////////
BOOL CRdXSecFrDWGDlg::GetRoadXSecFrDwgData(RDXSECOFFELEVDATA& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CRdXSecFrDWGDlg Dlg(Rec, Rec.arrayDatumRanges, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	return TRUE;
}

