// InsBlkFrFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InsBlkFrFileDlg.h"
#include "SurvUtilApp.h"
#include "ResourceHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// CInsBlkFrFileDlg dialog


CInsBlkFrFileDlg::CInsBlkFrFileDlg(const CStringArray& BlkNameArray, const INSBLKFRFILEPARAMS& Rec, CWnd* pParent /*=NULL*/)
	: CDialog(CInsBlkFrFileDlg::IDD, pParent), m_BlkNameArray(BlkNameArray), m_RecParam(Rec)
{
	//{{AFX_DATA_INIT(CInsBlkFrFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInsBlkFrFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsBlkFrFileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsBlkFrFileDlg, CDialog)
	//{{AFX_MSG_MAP(CInsBlkFrFileDlg)
	ON_BN_CLICKED(IDC_CHECK, OnCheck)
	ON_BN_CLICKED(IDC_OPEN_FILE, OnOpenFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsBlkFrFileDlg message handlers
void CInsBlkFrFileDlg::OnOpenFile() 
{
	DWORD dwFlags;
	const CString strConstFOpenFilter = CString("Block Insertion Data Files (*.TXT)|*.TXT||");

	dwFlags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	CFileDialog Dlg(TRUE, 0L, 0L, dwFlags, strConstFOpenFilter);
	if(Dlg.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(Dlg.GetPathName());
	}
}

BOOL CInsBlkFrFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTmp;
	
	m_bDrawOnCurLyr = (strlen(m_RecParam.szLyrAnnotation) == 0);
	((CButton*)GetDlgItem(IDC_CHECK))->SetCheck(m_bDrawOnCurLyr);
	((CButton*)GetDlgItem(IDC_HAS_Z_DATA))->SetCheck(m_RecParam.bHasDataZValue);
	((CButton*)GetDlgItem(IDC_CHECK_ZFLAG))->SetCheck(m_RecParam.bInsConsideringZValue);

	GetDlgItem(IDC_EDIT_LYR)->EnableWindow(!m_bDrawOnCurLyr);

	strTmp.Format("%.3f", m_RecParam.dBlkScaleX);			GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetWindowText(strTmp);
	strTmp.Format("%.3f", m_RecParam.dBlkScaleY);			GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->SetWindowText(strTmp);
	strTmp.Format("%.3f", RTOD(m_RecParam.dBlkRotAngle));	GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE)->SetWindowText(strTmp);
	strTmp.Format("%.3f", m_RecParam.dTxtSize);				GetDlgItem(IDC_EDIT_TXT_SIZE)->SetWindowText(strTmp);
	strTmp.Format("%.3f", RTOD(m_RecParam.dTxtRotation));	GetDlgItem(IDC_EDIT_TXT_ROT_ANGLE)->SetWindowText(strTmp);
	
	{//Block Name Combo...
		int i;
		for(i = 0; i < m_BlkNameArray.GetSize(); i++)
		{
			((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->AddString(m_BlkNameArray[i]);
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInsBlkFrFileDlg::OnOK() 
{
	// TODO: Add extra validation here
	int iIndex;
	BOOL bDrawOnCurLyr;
	CString strTemp;

	bDrawOnCurLyr = ((CButton*)GetDlgItem(IDC_CHECK))->GetCheck();
	m_RecParam.bInsConsideringZValue = ((CButton*)GetDlgItem(IDC_CHECK_ZFLAG))->GetCheck();
	m_RecParam.bHasDataZValue = ((CButton*)GetDlgItem(IDC_HAS_Z_DATA))->GetCheck();

	strcpy(m_RecParam.szLyrAnnotation, "");
	GetDlgItem(IDC_EDIT_LYR)->GetWindowText(strTemp);
	if(!bDrawOnCurLyr)
	{
		if(!CSurvUtilApp::IsValidACADSymName(strTemp))
		{
			AfxMessageBox("Invalid Layer name", MB_ICONSTOP);
			GetDlgItem(IDC_EDIT_LYR)->SetFocus();
			((CEdit*)GetDlgItem(IDC_EDIT_LYR))->SetSel(0, -1);
			return;
		}
		strcpy(m_RecParam.szLyrAnnotation, strTemp);
	}

	iIndex = ((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetCurSel();
	if(iIndex == CB_ERR)
	{
		AfxMessageBox("No block selected", MB_ICONSTOP);
		GetDlgItem(IDC_COMBO_BLK_NAME)->SetFocus();
		return;
	}
	((CComboBox*)GetDlgItem(IDC_COMBO_BLK_NAME))->GetLBText(iIndex, strTemp);
	strcpy(m_RecParam.szBlkName, strTemp);

	GetDlgItem(IDC_EDIT_BLK_SCALE_X)->GetWindowText(strTemp);
	m_RecParam.dBlkScaleX = atof((LPCSTR)strTemp);
	if(m_RecParam.dBlkScaleX <= 0.0)
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_X)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_X))->SetSel(0, -1);
		return;
	}
	
	GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->GetWindowText(strTemp);
	m_RecParam.dBlkScaleY = atof((LPCSTR)strTemp);
	if(m_RecParam.dBlkScaleY <= 0.0)
	{
		AfxMessageBox("Invalid Block Scale", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_SCALE_Y)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_SCALE_Y))->SetSel(0, -1);
		return;
	}

	GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE)->GetWindowText(strTemp);
	m_RecParam.dBlkRotAngle = atof((LPCSTR)strTemp);
	if((m_RecParam.dBlkRotAngle < -360.0) || (m_RecParam.dBlkRotAngle > 360.0))
	{
		AfxMessageBox("Invalid Block Rotation Angle", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_BLK_ROT_ANGLE))->SetSel(0, -1);
		return;
	}
	m_RecParam.dBlkRotAngle = DTOR(m_RecParam.dBlkRotAngle);

	GetDlgItem(IDC_EDIT_TXT_SIZE)->GetWindowText(strTemp);
	m_RecParam.dTxtSize = atof((LPCSTR)strTemp);
	if(m_RecParam.dTxtSize <= 0.0)
	{
		AfxMessageBox("Invalid Text Size", MB_ICONSTOP);
		GetDlgItem(IDC_EDIT_TXT_SIZE)->SetFocus();
		((CEdit*)GetDlgItem(IDC_EDIT_TXT_SIZE))->SetSel(0, -1);
		return;
	}
	GetDlgItem(IDC_EDIT_TXT_ROT_ANGLE)->GetWindowText(strTemp);
	m_RecParam.dTxtRotation = atof((LPCSTR)strTemp);
	m_RecParam.dTxtRotation = DTOR(m_RecParam.dTxtRotation);
	//if(m_RecParam.dTxtRotation <= 0.0)
	//{
	//	AfxMessageBox("Invalid Text Rotation Angle", MB_ICONSTOP);
	//	GetDlgItem(IDC_EDIT_TXT_ROT_ANGLE)->SetFocus();
	//	((CEdit*)GetDlgItem(IDC_EDIT_TXT_ROT_ANGLE))->SetSel(0, -1);
	//	return;
	//}

	GetDlgItem(IDC_EDIT_FILENAME)->GetWindowText(strTemp);
	strcpy(m_RecParam.szDataFile,  strTemp);
	if(strlen(m_RecParam.szDataFile) <= 0)
	{
		AfxMessageBox("Invalid data file name", MB_ICONSTOP);
		GetDlgItem(IDC_OPEN_FILE)->SetFocus();
		return;
	}
	CDialog::OnOK();
}
void CInsBlkFrFileDlg::OnCheck() 
{
	m_bDrawOnCurLyr = !m_bDrawOnCurLyr;
	GetDlgItem(IDC_EDIT_LYR)->EnableWindow(!m_bDrawOnCurLyr);
}
void CInsBlkFrFileDlg::GetAttribs(INSBLKFRFILEPARAMS& Rec)
{
	Rec = m_RecParam;
}


BOOL GetInsBlkFrFileParams(const CStringArray& BlkNameArray, INSBLKFRFILEPARAMS& Rec)
{
	CWnd* pAcadWnd;
	CTemporaryResourceOverride ResOverride;
	
	pAcadWnd = CWnd::FromHandle(adsw_acadMainWnd());

	CInsBlkFrFileDlg Dlg(BlkNameArray, Rec, pAcadWnd);
	if(Dlg.DoModal() != IDOK)
		return FALSE;

	Dlg.GetAttribs(Rec);
	return TRUE;
}
