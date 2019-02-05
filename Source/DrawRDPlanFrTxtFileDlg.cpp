// DrawCSFrCSDExDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DrawRDPlanFrTxtFileDlg.h"
#include "ResourceHelper.h"
#include "SurvUtilApp.h"
#include "DrawUtil.h"
#include "Util.h"
#include "LyrSettingDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();
/////////////////////////////////////////////////////////////////////////////
// CDrawRDPlanFrTxtFileDlg dialog

CDrawRDPlanFrTxtFileDlg::CDrawRDPlanFrTxtFileDlg(const BOOL& bIsNEZ, const CStringArray& BlkNameArray, DRAWPARAMRDPLANFRTXTFILE& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CDrawRDPlanFrTxtFileDlg::IDD, pParent), m_Rec(Rec), m_BlkNameArray(BlkNameArray), m_bIsNEZ(bIsNEZ)
{
	//{{AFX_DATA_INIT(CDrawRDPlanFrTxtFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDrawRDPlanFrTxtFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDrawRDPlanFrTxtFileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDrawRDPlanFrTxtFileDlg, CDialog)
	//{{AFX_MSG_MAP(CDrawRDPlanFrTxtFileDlg)

	ON_BN_CLICKED(IDC_OPEN_STN_DATA,	OnOpenSTN)
	ON_BN_CLICKED(IDC_OPEN_PID,			OnOpenPID)
	ON_BN_CLICKED(IDC_OPEN_OUT_FILE,	OnOpenOUT)
	ON_BN_CLICKED(IDSETLYR,				OnLyrSetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawRDPlanFrTxtFileDlg message handlers

BOOL CDrawRDPlanFrTxtFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	// TODO: Add extra initialization here
	CString strTmp;

	GetDlgItem(IDC_EDIT_FILENAME_OUT)->SetWindowText(m_Rec.szDataFileOut);
	GetDlgItem(IDC_EDIT_FILENAME_OUT)->ShowWindow(m_bIsNEZ?SW_HIDE:SW_SHOW);
	GetDlgItem(IDC_OPEN_OUT_FILE)->ShowWindow(m_bIsNEZ?SW_HIDE:SW_SHOW);

	GetDlgItem(IDC_EDIT_FILENAME_PID)->SetWindowText(m_Rec.szDataFilePID);
	GetDlgItem(IDC_EDIT_FILENAME_STN)->SetWindowText(m_Rec.szDataFileSTN);

	((CButton*)GetDlgItem(IDC_CHECK_ZFLAG))->SetCheck(m_Rec.bConsiderZ);

	strTmp.Format("%f", m_Rec.dNumAnnSize); GetDlgItem(IDC_EDIT_NUM_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dBlkScaleX);	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetWindowText(strTmp);
	strTmp.Format("%f", m_Rec.dBlkScaleY);	GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->SetWindowText(strTmp);
	{//Block Name Combo...
		int i;
		for(i = 0; i < m_BlkNameArray.GetSize(); i++)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->AddString(m_BlkNameArray[i]);
		}
	}
	return TRUE;
}

void CDrawRDPlanFrTxtFileDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strTmp;
	int iIndex;
	
	m_Rec.bConsiderZ = ((CButton*)GetDlgItem(IDC_CHECK_ZFLAG))->GetCheck();
	//
	GetDlgItem(IDC_EDIT_FILENAME_PID)->GetWindowText(strTmp);
	if(strlen(strTmp) <= 0)
	{
		AfxMessageBox("Invalid PID file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_PID)->SetFocus();
		return;
	}
	strcpy(m_Rec.szDataFilePID, strTmp);
	//
	GetDlgItem(IDC_EDIT_FILENAME_STN)->GetWindowText(strTmp);
	if(strlen(strTmp) <= 0)
	{
		AfxMessageBox("Invalid Station data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_STN_DATA)->SetFocus();
		return;
	}
	strcpy(m_Rec.szDataFileSTN, strTmp);

	//
	GetDlgItem(IDC_EDIT_FILENAME_OUT)->GetWindowText(strTmp);
	if(m_bIsNEZ)
	{
		strcpy(m_Rec.szDataFileOut, "");
	}
	else
	{
		//if(strlen(strTmp) <= 0)
		//{
		//	AfxMessageBox("Invalid output data file name", MB_ICONSTOP);
		//	GetDlgItem(IDC_OPEN_OUT_FILE)->SetFocus();
		//	return;
		//}
		strcpy(m_Rec.szDataFileOut, strTmp);
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
	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->GetWindowText(strTmp);
	m_Rec.dBlkScaleX = atof(strTmp);
	if((m_Rec.dBlkScaleX <= 0.0) || (m_Rec.dBlkScaleX > 1.0))
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_X))->SetSel(0, -1);
		return;
	}
	//
	GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->GetWindowText(strTmp);
	m_Rec.dBlkScaleY = atof(strTmp);
	if((m_Rec.dBlkScaleY <= 0.0) || (m_Rec.dBlkScaleY > 1.0))
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_Y))->SetSel(0, -1);
		return;
	}
	iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetCurSel();
	if(iIndex == CB_ERR)
	{
		AfxMessageBox("No block selected", MB_ICONSTOP);
		GetDlgItem(IDC_COMBO_BLK_NAME)->SetFocus();
		return;
	}
	((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetLBText(iIndex, strTmp);
	strcpy(m_Rec.szBlkName, strTmp);

	CDialog::OnOK();
}

void CDrawRDPlanFrTxtFileDlg::OnOpenOUT() 
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
void CDrawRDPlanFrTxtFileDlg::OnOpenPID() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("PID Files (*.PID)|*.PID||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME_PID)->SetWindowText(Dlg.GetPathName());
	}
}

void CDrawRDPlanFrTxtFileDlg::OnOpenSTN() 
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
void CDrawRDPlanFrTxtFileDlg::OnLyrSetting() 
{
	const int iSize = 2;
	const char* pszTags[] = {"Road C/L", "Annotation"};
	const char* pszVals[] = {m_Rec.szLyrPID, m_Rec.szLyrAnnotation};
	
	CLyrSettingDlg Dlg("Layer Setting", TRUE, iSize, pszTags, pszVals, this);
	
	if(Dlg.DoModal() != IDOK)
		return;
	char* pszValsDummy[] = {m_Rec.szLyrPID, m_Rec.szLyrAnnotation};
	Dlg.GetValues(iSize, pszValsDummy);
}
BOOL GetDrawParamRDPlanFrTxtFile(const BOOL& bIsNEZ, DRAWPARAMRDPLANFRTXTFILE& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	CStringArray BlkNameArray;
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	{//Initialize default data...
		Rec.bConsiderZ = TRUE;
		Rec.dBlkScaleX = 1.0;
		Rec.dBlkScaleY = 1.0;

		Rec.dNumAnnSize = 2.0;

		strcpy(Rec.szBlkName,				"");	
		strcpy(Rec.szLyrAnnotation,			"RD_PLAN_ANNOTATION");	
		strcpy(Rec.szLyrPID,				"RD_PLAN_CENTRE_LINE");	

		strcpy(Rec.szDataFileOut,			"");	
		strcpy(Rec.szDataFilePID,			"");	
		strcpy(Rec.szDataFileSTN,			"");	
	}
	GetBlockNames(BlkNameArray);
	if(BlkNameArray.GetSize() <= 0)
	{
		AfxMessageBox("No block definition found in current drawing\nCreate block first & then try again", MB_ICONSTOP);
		return FALSE;
	}

	CDrawRDPlanFrTxtFileDlg Dlg(bIsNEZ, BlkNameArray, Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	return TRUE;
}
